#ifndef OB_CORE_INTERNER_H_INCLUDED
#define OB_CORE_INTERNER_H_INCLUDED

#include "Allocator.h"
#include "Array.h"
#include "ContextFwd.h"
#include "String.h"
#include "Table.h"

typedef struct {
  ob_Context context; // where the strings are allocated
  ob_Allocator *allocator;
  ob_Array data;     // data of every interned symbol
  ob_Table interned; // table of offsets
} ob_Interner;

void obintr_init(ob_Interner *intr, ob_Context ctx, ob_Allocator *alloc);
void obintr_free(ob_Interner *intr);

// TODO: uninterning, etc
ob_String *obintr_intern(ob_Interner *intr, size_t length, const char *data);

ob_String *obintr_find(ob_Interner *intr, uint64_t hash);

void obintr_mark(ob_Interner *intr);

#endif
