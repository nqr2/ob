#ifndef STRING_H_INCLUDED
#define STRING_H_INCLUDED

#include <stddef.h>
#include <stdint.h>

#include "ContextFwd.h"

#define STRING_MARK_BIT 0x8000'0000'0000'0000
#define STRING_LENGTH_MASK 0x7fff'ffff'ffff'ffff

typedef struct String {
  uint64_t length;
  const char *data;
  struct String *next;
} String;

typedef struct {
  size_t offset;
  size_t size;
} StrAvailable;

String *str_create(Context ctx, size_t len, const char *data);

size_t str_len(String *str);

void str_mark(String *str);
void str_unmark(String *str);
bool str_get_mark(String *str);

void str_sweep(Context ctx);

#endif
