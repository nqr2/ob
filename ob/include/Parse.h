#ifndef PARSE_H_INCLUDED
#define PARSE_H_INCLUDED

#include "ContextFwd.h"
#include "Object.h"

/* Parse some text, and return a parsed object. Returns the last character read.
 */
const char *read(Context ctx, Obj *output, size_t length, const char *text);

/* Parse some text, and return a unary closure */
Obj parse(Context ctx, size_t length, const char *text);

#endif
