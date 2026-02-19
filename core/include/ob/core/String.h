#ifndef OB_CORE_STRING_H_INCLUDED
#define OB_CORE_STRING_H_INCLUDED

/** @file
 *
 * @brief Strings.
 */

#include <ob/Core.h>

#include <stddef.h>
#include <stdint.h>

struct ob_String {
  uint64_t length;
  size_t offset;
  struct ob_String *next;
};

ob_Str obstr_create(ob_Ctx ctx, size_t len, char const *data);

#define obstr_create_literal(Context, Literal)                                 \
  obstr_create((Context), sizeof(Literal) - 1, "" Literal "")

size_t obstr_get_length(ob_Str str);
char const *obstr_get_data(ob_Ctx ctx, ob_Str str);
uint64_t obstr_get_hash(ob_Ctx ctx, ob_Str str);

void obstr_mark(ob_Str str);
void obstr_unmark(ob_Str str);
bool obstr_get_mark(ob_Str str);

void obstr_sweep(ob_Ctx ctx);

ob_Str obstr_concat(ob_Ctx ctx, ob_Str left, ob_Str right);

#endif
