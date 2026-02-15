#ifndef OB_CORE_PARSE_H_INCLUDED
#define OB_CORE_PARSE_H_INCLUDED

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
 * @brief The lexer, parser, and bytecode compiler, called the "Reader".
 */

#include <ob/Core.h>

[[deprecated("replace ob_Reader* for ob_Rdr")]]
typedef struct ob_Reader ob_Reader;
typedef struct ob_Reader *ob_Rdr;

ob_Rdr obrdr_create(ob_Ctx ctx);
void obrdr_free(ob_Rdr rdr);

void obrdr_load(ob_Rdr rdr, const char *path, size_t length, const char *data);

ob_Obj obrdr_get_method(ob_Rdr rdr);

#endif
