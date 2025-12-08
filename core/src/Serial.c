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
#include <string.h>

#define A_FULL 0x00
#define A_REF 0x01

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

void srl_init(Serial *srl, Context ctx) {
  srl->ctx = ctx;
  arr_init(&srl->output, ctx->allocator);
  tbl_init(&srl->identifiers, ctx->allocator);
}

void srl_free(Serial *srl) {
  arr_free(&srl->output);
  tbl_free(&srl->identifiers);
}

struct udata {
  Serial *srl;
  uint64_t *ident;
};

static void write_obj(Obj object, void *userdata) {
  struct udata *udata = userdata;
  auto srl = udata->srl;
  auto ident = udata->ident;

  char kind = A_FULL;

  if (tbl_get(&srl->identifiers, (uint64_t)object, (void **)ident)) {
    kind = A_REF;
    arr_push(&srl->output, sizeof(char), &kind);
    write_int(srl, *ident);
    return;
  }

  arr_push(&srl->output, sizeof(char), &kind);
  auto tag = obj_get_tag(object);

  *ident = srl->output.size;
  tbl_set(&srl->identifiers, (uint64_t)object, ident);

  arr_push(&srl->output, sizeof(tag), &tag);

  // data
  switch (tag) {
  case OT_SYMBOL: // <length> <characters>
  case OT_STRING: // same
  {
    ObjString *str = obj_get_data(object);
    auto length = str_get_length(str->inner);
    auto data = str_get_data(srl->ctx, str->inner);
    arr_push(&srl->output, length, data);
  } break;
  case OT_NUMBER: // the number
  {
    ObjNumber *num = obj_get_data(object);
    // TODO: something actually portable
    arr_push(&srl->output, sizeof(Number), &num->number);
  } break;

  default:
    ASSERT(false, "cannot serialize this object");
    break;
  }
}

void srl_write(Serial *srl, Obj object) {
  tbl_clear(&srl->identifiers);

  arr_push(&srl->output, sizeof(SERIAL_HEADER), SERIAL_HEADER);

  auto sink = (uint64_t)0;

  auto udata = (struct udata){srl, &sink};
  obj_visit_after(object, write_obj, &udata);

  (void)sink;
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
