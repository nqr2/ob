#ifndef OB_CORE_PARSE_H_INCLUDED
#define OB_CORE_PARSE_H_INCLUDED

#include "ContextFwd.h"
#include "Object.h"

/* Parse some text, and return a unary closure */
ob_Obj ob_load(ob_Context ctx, size_t length, const char *text);

/* Parse some text, and run it. */
void ob_run(ob_Context ctx, size_t length, const char *text);

#define ob_load_literal(Ctx, Lit) ob_load((Ctx), sizeof(Lit) - 1, "" Lit)
#define ob_run_literal(Ctx, Lit) ob_run((Ctx), sizeof(Lit) - 1, "" Lit)

#endif
