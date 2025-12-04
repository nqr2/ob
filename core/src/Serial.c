#include "Serial.h"
#include "Context.h"

#include <string.h>

void srl_init(Serial *srl, Context ctx) {
  srl->ctx = ctx;
  arr_init(&srl->output, ctx->allocator);
}

void srl_free(Serial *srl) {
  arr_free(&srl->output);
}

void srl_write(Serial *srl, Obj object) {
}

Obj srl_read(const Serial *srl) {
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
