#ifndef INTERNER_H_INCLUDED
#define INTERNER_H_INCLUDED

#include "Allocator.h"
#include "Array.h"
#include "ContextFwd.h"
#include "String.h"
#include "Table.h"

typedef struct {
  Context context; // where the strings are allocated
  Allocator *allocator;
  Array data;     // data of every interned symbol
  Table interned; // table of offsets
} Interner;

void intr_init(Interner *intr, Context ctx, Allocator *alloc);
void intr_free(Interner *intr);

// TODO: uninterning, etc
String *intr_intern(Interner *intr, size_t length, const char *data);

String *intr_find(Interner *intr, uint64_t hash);

void intr_mark(Interner *intr);

#endif
