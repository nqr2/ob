#include "Serial.h"
#include "Array.h"
#include "Assert.h"
#include "Context.h"
#include "Object.h"
#include "Table.h"

#include <stdbit.h>
#include <string.h>

void srl_init(Serial *srl, Context ctx) {
  srl->ctx = ctx;
  arr_init(&srl->output, ctx->allocator);
  tbl_init(&srl->identifiers, ctx->allocator);
}

void srl_free(Serial *srl) {
  arr_free(&srl->output);
  tbl_free(&srl->identifiers);
}

static bool write_obj(Serial *srl, uint64_t *ident, Obj object) {
  tbl_set(&srl->identifiers, (uint64_t)object, (void *)&object);

  *ident = srl->output.size;

  if (tbl_get(&srl->identifiers, (uint64_t)object, (void **)ident)) {
    return true;
  }

  // write data for object here?

  return false;
}

static void write_int(Serial *srl, uint64_t n) {
  auto len = stdc_bit_width(n);
  ASSERT(len <= 63, "cannot encode a num > 64 bits.");

  do {
    uint8_t byte = n & 0x7f;
    n >>= 7;

    if (n > 0) {
      byte |= 0x80;
    }

    arr_push(&srl->output, sizeof(uint8_t), &byte);
  } while (n != 0);
}

static uint64_t read_int(Serial *srl, size_t off) {
  uint8_t *bytes = srl->output.data;
  bytes += off;

  auto shift = 0;
  uint64_t result = 0;

  do {
    result |= (*bytes & 0x7f) << (shift * 7);
    shift++;
  } while ((*bytes & 0x80) != 0);

  return result;
}

void srl_write(Serial *srl, Obj object) {
  tbl_clear(&srl->identifiers);

  arr_push(&srl->output, sizeof(SERIAL_HEADER), SERIAL_HEADER);

  switch (obj_get_tag(object)) {
  case OT_NIL: // no need to do anything here
    break;
  case OT_SYMBOL: // <length> <characters>
  case OT_STRING: // same
    break;
  case OT_SLOTS: // <prototype> <slots>?
    break;
  case OT_NUMBER: // the number
    break;
  case OT_ARRAY: // <length> <items>
    break;
  case OT_METHOD: // <arguments> <bytecode>
    break;

  case OT_ACTIVATION: // no idea if this is OK
  case OT_CMETHOD:    // cannot serialize opaque data
  case OT_CDATA:      // same here
  case OT_Ra:
  case OT_Rb:
  case OT_Rc:
  case OT_Rd:
  case OT_Re:
  case OT_Rf:

  default:
    ASSERT(false, "cannot serialize this object");
    break;
  }
}

Obj srl_read(Serial *srl) {
  tbl_clear(&srl->identifiers);
  return NULL;
}

void srl_store(const Serial *srl, size_t len, uint8_t *data) {
  memcpy(data, srl->output.data, len);
}

void srl_load(Serial *srl, size_t len, const uint8_t *data) {
  arr_reserve(&srl->output, len);
  memcpy(srl->output.data, data, len);
  srl->output.size = len;
}
