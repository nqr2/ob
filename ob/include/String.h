#ifndef STRING_H_INCLUDED
#define STRING_H_INCLUDED

#include <stddef.h>
#include <stdint.h>

#include "ContextFwd.h"

#define STRING_MARK_BIT 0x8000'0000'0000'0000
#define STRING_LENGTH_MASK 0x7fff'ffff'ffff'ffff

typedef struct String {
  uint64_t length;
  size_t offset;
  struct String *next;
} String;

typedef struct {
  size_t offset;
  size_t size;
} StrAvailable;

typedef String *Str;

Str str_create(Context ctx, size_t len, const char *data);

#define str_create_literal(Context, Literal)                                   \
  str_create((Context), sizeof(Literal) - 1, "" Literal "")

size_t str_get_length(Str str);
const char *str_get_data(Context ctx, Str str);
uint64_t str_get_hash(Context ctx, Str str);

void str_mark(Str str);
void str_unmark(Str str);
bool str_get_mark(Str str);

void str_sweep(Context ctx);

#endif
