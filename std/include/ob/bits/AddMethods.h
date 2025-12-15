#ifndef OB_BITS_ADDMETHODS_H_INCLUDED
#define OB_BITS_ADDMETHODS_H_INCLUDED

#include <ob/Object.h>

typedef struct {
  const char *name;
  ob_FnCMethod method;
} ob_MethodEntry;

#define OB_METHODS_END ((ob_MethodEntry){})

void ob_add_methods(ob_Context ctx, ob_Obj target,
                    const ob_MethodEntry *entries);

#endif
