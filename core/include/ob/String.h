#ifndef OB_CORE_STRING_H_INCLUDED
#define OB_CORE_STRING_H_INCLUDED

#include <stddef.h>
#include <stdint.h>

#include "ContextFwd.h"

typedef struct String {
  uint64_t length;
  size_t offset;
  struct String *next;
} ob_String;

typedef ob_String *ob_Str;

ob_Str obstr_create(ob_Context ctx, size_t len, const char *data);

#define obstr_create_literal(Context, Literal)                                 \
  obstr_create((Context), sizeof(Literal) - 1, "" Literal "")

size_t obstr_get_length(ob_Str str);
const char *obstr_get_data(ob_Context ctx, ob_Str str);
uint64_t obstr_get_hash(ob_Context ctx, ob_Str str);

void obstr_mark(ob_Str str);
void obstr_unmark(ob_Str str);
bool obstr_get_mark(ob_Str str);

void obstr_sweep(ob_Context ctx);

#endif
