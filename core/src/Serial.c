#include <ob/Array.h>
#include <ob/Assert.h>
#include <ob/Context.h>
#include <ob/Number.h>
#include <ob/Object.h>
#include <ob/Serial.h>
#include <ob/String.h>
#include <ob/Table.h>

#include <stdbit.h>
#include <stdint.h>
#include <string.h>

static void write_int(ob_Serial *srl, uint64_t n) {
  auto len = stdc_bit_width(n);
  ASSERT(len <= 63, "cannot encode a num > 64 bits.");

  do {
    uint8_t byte = n & 0x7f;
    n >>= 7;

    if (n != 0) {
      byte |= 0x80;
    }
    ql_array_push(&srl->buffer, sizeof(uint8_t), &byte);
  } while (n != 0);
}

static uint8_t *read_int(uint8_t *bytes, uint64_t *result) {
  auto shift = 0;
  auto byte = *bytes;

  *result = 0;

  do {
    byte = *bytes;
    *result |= (byte & 0x7f) << (shift * 7);
    shift++;
    bytes++;
  } while ((byte & 0x80) != 0);

  return bytes;
}

void obsrl_init(ob_Serial *srl, ob_Context ctx) {
  srl->ctx = ctx;
  ql_array_init(&srl->buffer, ctx->allocator);
  ql_table_init(&srl->identifiers, ctx->allocator);
}

void obsrl_free(ob_Serial *srl) {
  ql_array_free(&srl->buffer);
  ql_table_free(&srl->identifiers);
}

static void write_ref(ob_Obj object, ob_Serial *srl) {
  uint64_t ident = 0;

  // The bc header should be @ offset 0, so this is always 1 byte
  if (object == NULL) {
    write_int(srl, 0);
    return;
  }

  if (ql_table_get(&srl->identifiers, (uint64_t)object, (void **)&ident)) {
    write_int(srl, ident);
    return;
  }

  ASSERT(false, "object %p was not yet written", (void *)object);
}

static void write_obj(ob_Obj object, void *userdata) {
  ob_Serial *srl = userdata;
  uint64_t ident = 0;

  ident = srl->buffer.size;
  ql_table_set(&srl->identifiers, (uint64_t)object, (void *)ident);

  uint8_t tag = ob_get_tag(object);

  if (tag != OB_NIL) {
    ql_array_push(&srl->buffer, sizeof(tag), &tag);
  }

  // data
  switch (tag) {
  case OB_NIL: // nothing here
    break;

  case OB_SYMBOL: // <length> <characters>
  {
    auto str = ob_cast_symbol(object);
    auto length = obstr_get_length(*str);
    auto data = obstr_get_data(srl->ctx, *str);

    write_int(srl, length);
    ql_array_push(&srl->buffer, length, data);
  } break;
  case OB_STRING: // same
  {
    auto str = ob_cast_string(object);
    auto length = obstr_get_length(*str);
    auto data = obstr_get_data(srl->ctx, *str);

    write_int(srl, length);
    ql_array_push(&srl->buffer, length, data);
  } break;
  case OB_NUMBER: // the number
  {
    auto num = ob_cast_number(object);
    // TODO: something actually portable
    ql_array_push(&srl->buffer, sizeof(ob_Number), num);
  } break;

  case OB_METHOD: {
    auto data = ob_cast_method(object);

    write_ref(data->env, srl);

    auto len = data->literals.size / sizeof(ob_Obj);
    write_int(srl, len);

    for (size_t i = 0; i < len; i++) {
      ob_Obj item = ((ob_Obj *)data->literals.data)[i];

      write_ref(item, srl);
    }

    write_int(srl, data->bytecode.size);
    ql_array_push(&srl->buffer, data->bytecode.size, data->bytecode.data);
  }; break;

  default:
    ASSERT(false, "cannot serialize this object of tag %d", tag);
    break;
  }
}

static bool write_pred(ob_Obj obj, void *udata) {
  ob_Serial *srl = udata;
  return ql_table_get(&srl->identifiers, (uint64_t)obj, NULL);
}

void obsrl_write(ob_Serial *srl, ob_Obj object) {
  ql_table_clear(&srl->identifiers);

  ql_array_push(&srl->buffer, sizeof(OB_SERIAL_HEADER), OB_SERIAL_HEADER);

  ob_visit(object, OB_VISIT_AFTER, write_obj, write_pred, srl);
}

static bool string_equal(size_t n, void *left, void *right) {
  return strncmp(left, right, n) == 0;
}

ob_Obj read_ref(ob_Serial *srl, uint64_t ident) {
  ob_Obj res = NULL;

  if (ident == 0) {
    return NULL;
  }

  if (ql_table_get(&srl->identifiers, ident, (void **)&res)) {
    return res;
  }

  ASSERT(false, "ref %ld doesn't exist", ident);
  return NULL;
}

ob_Obj obsrl_read(ob_Serial *srl) {
  ql_table_clear(&srl->identifiers);

  ob_Obj result = NULL;
  uint8_t *head = srl->buffer.data;
  auto remaining = srl->buffer.size - sizeof(OB_SERIAL_HEADER);

  ASSERT(string_equal(sizeof(OB_SERIAL_HEADER), head, OB_SERIAL_HEADER),
         "invalid header");

  head += sizeof(OB_SERIAL_HEADER);

  while (remaining > 0) {
    auto offset = head - (uint8_t *)srl->buffer.data;

    auto here = head;
    auto tag = *head;
    head++;

    switch (tag) {
    case OB_NIL:
      result = NULL;
      break;

    case OB_SYMBOL: {
      uint64_t length = 0;
      head = read_int(head, &length);

      auto str = obstr_create(srl->ctx, length, (const char *)head);
      head += length;

      result = ob_create_symbol(srl->ctx, str);
    } break;

    case OB_STRING: {
      uint64_t length = 0;
      head = read_int(head, &length);

      auto str = obstr_create(srl->ctx, length, (const char *)head);
      head += length;

      result = ob_create_string(srl->ctx, str);
    } break;

    case OB_NUMBER: {
      auto num = (ob_Number){};
      memcpy(&num, head, sizeof(ob_Number));
      head += sizeof(ob_Number);

      result = ob_create_number(srl->ctx, num);
    } break;

    case OB_METHOD: {
      uint64_t ident = 0;
      uint64_t length = 0;

      result = ob_create_method(srl->ctx);
      auto method = ob_cast_method(result);

      // method->env
      head = read_int(head, &ident);
      method->env = read_ref(srl, ident);

      // method->literals
      head = read_int(head, &length);
      for (uint64_t i = 0; i < length; i++) {
        head = read_int(head, &ident);

        auto item = read_ref(srl, ident);

        ql_array_push(&method->literals, sizeof(ob_Obj), (void *)&item);
      }

      // method->bytecode
      head = read_int(head, &length);
      ql_array_push(&method->bytecode, length, head);
      head += length;
    } break;

    default:
      ASSERT(false, "unsupported object type when read: %d at offset %d", tag,
             offset);
    }

    ql_table_set(&srl->identifiers, offset, result);

    remaining -= head - here;
  }

  return result;
}

void obsrl_store(const ob_Serial *srl, size_t len, uint8_t *data) {
  memcpy(data, srl->buffer.data, len);
}

void obsrl_load(ob_Serial *srl, size_t len, const uint8_t *data) {
  ql_array_reserve(&srl->buffer, len);
  memcpy(srl->buffer.data, data, len);
  srl->buffer.size = len;
}
