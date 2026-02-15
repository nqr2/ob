#ifndef OB_CORE_STRING_H_INCLUDED
#define OB_CORE_STRING_H_INCLUDED

/*
 * Copyright (C) 2025-2026 nqr2
 *
 * This library is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library. If not, see <https://www.gnu.org/licenses/>.
 */

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
