#include "Serial.h"
#include "Array.h"
#include "Context.h"
#include "Table.h"

#include <string.h>

/*
 * So, the idea would be that this whole thing is done in "packets", where the
 * first one just describes the object to return, so the format would be:
 *
 *   <length> <header> <content...>
 *
 * where length also includes the size of the header.
 *
 * I am considering two packets to begin with:
 *   STRING, which would contain string data, and
 *   OBJECT, which would contain an object, serialized.
 *
 * STRING
 *   <id:number>
 *   <string data>
 *
 * OBJECT
 *   <varied contents>
 *
 * There could be more, of course ;)
 */

void srl_init(Serial *srl, Context ctx) {
  srl->ctx = ctx;
  arr_init(&srl->output, ctx->allocator);
  tbl_init(&srl->identifiers, ctx->allocator);
}

void srl_free(Serial *srl) {
  arr_free(&srl->output);
  tbl_free(&srl->identifiers);
}

void srl_write(Serial *srl, Obj object) {
  (void)object;

  tbl_clear(&srl->identifiers);

  arr_push(&srl->output, sizeof(SERIAL_HEADER), SERIAL_HEADER);
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
