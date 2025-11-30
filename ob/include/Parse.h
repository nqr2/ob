#ifndef PARSE_H_INCLUDED
#define PARSE_H_INCLUDED

#include "ContextFwd.h"
#include "Object.h"

/* Parse some text, and return a unary closure */
Obj load_file(Context ctx, size_t length, const char *text);

/* Parse some text, and run it. */
void run_file(Context ctx, size_t length, const char *text);

#define load_literal(Ctx, Lit) load_file((Ctx), sizeof(Lit) - 1, "" Lit)
#define run_literal(Ctx, Lit) run_file((Ctx), sizeof(Lit) - 1, "" Lit)

#endif
