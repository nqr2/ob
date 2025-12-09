#include "Serial.h"
#include "Array.h"
#include "Assert.h"
#include "Context.h"
#include "Number.h"
#include "Object.h"
#include "String.h"
#include "Table.h"

#include <stdbit.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void write_int(Serial *srl, uint64_t n) {
  auto len = stdc_bit_width(n);
  ASSERT(len <= 63, "cannot encode a num > 64 bits.");

  do {
    uint8_t byte = n & 0x7f;
    n >>= 7;

    if (n > 0) {
      byte |= 0x80;
    }

    arr_push(&srl->buffer, sizeof(uint8_t), &byte);
  } while (n != 0);
}

static uint8_t *read_int(uint8_t *bytes, uint64_t *result) {
  auto shift = 0;
  auto byte = *bytes;

  do {
    byte = *bytes;
    *result |= (byte & 0x7f) << (shift * 7);
    shift++;
    bytes++;
  } while ((byte & 0x80) != 0);

  return bytes;
}

void srl_init(Serial *srl, Context ctx) {
  srl->ctx = ctx;
  arr_init(&srl->buffer, ctx->allocator);
  tbl_init(&srl->identifiers, ctx->allocator);
}

void srl_free(Serial *srl) {
  arr_free(&srl->buffer);
  tbl_free(&srl->identifiers);
}

static void write_ref(Obj object, Serial *srl) {
  uint64_t ident = 0;

  // The bc header should be @ offset 0, so this is always 1 byte
  if (object == NULL) {
    write_int(srl, 0);
    return;
  }

  if (tbl_get(&srl->identifiers, (uint64_t)object, (void **)&ident)) {
    write_int(srl, ident);
    return;
  }

  ASSERT(false, "object %p was not yet written", (void *)object);
}

static void write_obj(Obj object, void *userdata) {
  Serial *srl = userdata;
  uint64_t ident = 0;

  if (tbl_get(&srl->identifiers, (uint64_t)object, (void **)&ident)) {
    fprintf(stderr, "already: %p\n", (void *)object);
    return;
  }

  ident = srl->buffer.size;
  fprintf(stderr, "add label: %p -> %ld\n", (void *)object, ident);
  tbl_set(&srl->identifiers, (uint64_t)object, (void *)ident);

  uint8_t tag = obj_get_tag(object);
  arr_push(&srl->buffer, sizeof(tag), &tag);

  // data
  switch (tag) {
  case OT_NIL: // nothing here
    break;

  case OT_SYMBOL: // <length> <characters>
  case OT_STRING: // same
  {
    ObjString *str = obj_get_data(object);
    auto length = str_get_length(str->inner);
    auto data = str_get_data(srl->ctx, str->inner);

    write_int(srl, length);
    arr_push(&srl->buffer, length, data);
  } break;
  case OT_NUMBER: // the number
  {
    ObjNumber *num = obj_get_data(object);
    // TODO: something actually portable
    arr_push(&srl->buffer, sizeof(Number), &num->number);
  } break;

  case OT_METHOD: {
    ObjMethod *data = obj_get_data(object);

    write_ref(data->env, srl);

    auto len = data->literals.size / sizeof(Obj);
    write_int(srl, len);

    for (size_t i = 0; i < len; i++) {
      Obj item = ((Obj *)data->literals.data)[i];

      write_ref(item, srl);
    }

    write_int(srl, data->bytecode.size);
    arr_push(&srl->buffer, data->bytecode.size, data->bytecode.data);
  }; break;

  default:
    ASSERT(false, "cannot serialize this object of tag %d", tag);
    break;
  }
}

void srl_write(Serial *srl, Obj object) {
  tbl_clear(&srl->identifiers);

  arr_push(&srl->buffer, sizeof(SERIAL_HEADER), SERIAL_HEADER);

  obj_visit_after(object, write_obj, srl);
}

static bool string_equal(size_t n, void *left, void *right) {
  return strncmp(left, right, n) == 0;
}

Obj read_ref(Serial *srl, uint64_t ident) {
  Obj res = NULL;

  if (ident == 0) {
    return NULL;
  }

  if (tbl_get(&srl->identifiers, ident, (void **)&res)) {
    return res;
  }

  ASSERT(false, "ref %ld doesn't exist", ident);
  return NULL;
}

Obj srl_read(Serial *srl) {
  tbl_clear(&srl->identifiers);

  Obj result = NULL;
  uint8_t *head = srl->buffer.data;
  auto remaining = srl->buffer.size - sizeof(SERIAL_HEADER);

  ASSERT(string_equal(sizeof(SERIAL_HEADER), head, SERIAL_HEADER),
         "invalid header");

  head += sizeof(SERIAL_HEADER);

  while (remaining > 0) {
    auto offset = head - (uint8_t *)srl->buffer.data;

    auto here = head;
    auto tag = *head;
    head++;

    switch (tag) {
    case OT_NIL:
      result = NULL;
      break;

    case OT_SYMBOL:
    case OT_STRING: {
      uint64_t length = 0;
      head = read_int(head, &length);

      auto str = str_create(srl->ctx, length, (const char *)head);
      head += length;

      result = ctx_alloc_string(srl->ctx, str);
    } break;

    case OT_NUMBER: {
      auto num = (Number){};
      memcpy(&num, head, sizeof(Number));
      head += sizeof(Number);

      result = ctx_alloc_number(srl->ctx, num);
    } break;

    case OT_METHOD: {
      uint64_t ident = 0;
      uint64_t length = 0;

      result = ctx_alloc_method(srl->ctx);
      ObjMethod *method = obj_get_data(result);

      // method->env
      head = read_int(head, &ident);
      method->env = read_ref(srl, ident);

      // method->literals
      head = read_int(head, &length);
      for (uint64_t i = 0; i < length; i++) {
        head = read_int(head, &ident);
        auto item = read_ref(srl, ident);

        arr_push(&method->literals, sizeof(Obj), (void *)&item);
      }

      // method->bytecode
      head = read_int(head, &ident);
      arr_push(&method->bytecode, ident, head);
      head += ident;
    } break;

    default:
      ASSERT(false, "unsupported object type when read: %d at offset %d", tag,
             offset);
    }

    printf("add key: %ld -> %p\n", offset, (void *)result);
    tbl_set(&srl->identifiers, offset, result);

    remaining -= head - here;
  }

  return result;
}

void srl_store(const Serial *srl, size_t len, uint8_t *data) {
  memcpy(data, srl->buffer.data, len);
}

void srl_load(Serial *srl, size_t len, const uint8_t *data) {
  arr_reserve(&srl->buffer, len);
  memcpy(srl->buffer.data, data, len);
  srl->buffer.size = len;
}
