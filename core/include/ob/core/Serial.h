#ifndef OB_CORE_SERIAL_H_INCLUDED
#define OB_CORE_SERIAL_H_INCLUDED

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
 * @brief (de)Serializing objects.
 */

#include <ob/Core.h>

#include <ql/Table.h>

#define OB_SERIAL_HEADER "\x0bOB"

typedef struct {
  ob_Ctx ctx;
  ql_Array buffer;
  ql_Table identifiers;
} ob_Serial;

void obsrl_init(ob_Serial *srl, ob_Ctx ctx);
void obsrl_free(ob_Serial *srl);

void obsrl_write(ob_Serial *srl, ob_Obj object);
ob_Obj obsrl_read(ob_Serial *srl);

void obsrl_store(const ob_Serial *srl, size_t len, uint8_t *data);
void obsrl_load(ob_Serial *srl, size_t len, const uint8_t *data);

#endif
