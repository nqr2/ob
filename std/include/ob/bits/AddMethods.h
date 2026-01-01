#ifndef OB_BITS_ADDMETHODS_H_INCLUDED
#define OB_BITS_ADDMETHODS_H_INCLUDED

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

#include <ob/Object.h>

typedef struct {
  const char *name;
  ob_FnCMethod method;
} ob_MethodEntry;

#define OB_METHODS_END ((ob_MethodEntry){})

void ob_add_methods(ob_Context ctx, ob_Obj target,
                    const ob_MethodEntry *entries);

#endif
