#ifndef SERIAL_H_INCLUDED
#define SERIAL_H_INCLUDED

#include "Object.h"

#define SERIAL_HEADER "obS"

typedef struct {
  Context ctx;
  Array output;
  Table identifiers;
} Serial;

void srl_init(Serial *srl, Context ctx);
void srl_free(Serial *srl);

void srl_write(Serial *srl, Obj object);
Obj srl_read(Serial *srl);

void srl_store(const Serial *srl, size_t len, uint8_t *data);
void srl_load(Serial *srl, size_t len, const uint8_t *data);

#endif
