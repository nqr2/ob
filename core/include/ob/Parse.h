#ifndef OB_CORE_PARSE_H_INCLUDED
#define OB_CORE_PARSE_H_INCLUDED

/*
 * Copyright (C) 2025 nqr2
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
 * @brief Loading and running text.
 */

#include "ContextFwd.h"
#include "Object.h"

/* Parse some text, and return a unary closure */
ob_Obj ob_load(ob_Context ctx, size_t length, const char *text);

/* Parse some text, and run it. */
void ob_run(ob_Context ctx, size_t length, const char *text);

#define ob_load_literal(Ctx, Lit) ob_load((Ctx), sizeof(Lit) - 1, "" Lit)
#define ob_run_literal(Ctx, Lit) ob_run((Ctx), sizeof(Lit) - 1, "" Lit)

#endif
