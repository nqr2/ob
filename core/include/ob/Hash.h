#ifndef OB_CORE_HASH_H_INCLUDED
#define OB_CORE_HASH_H_INCLUDED

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
 * @brief Hashing.
 */

#include <stddef.h>
#include <stdint.h>

uint64_t obhash_continue(uint64_t state, size_t len, const void *ptr);
uint64_t obhash_start(size_t len, const void *ptr);

#define obhash_literal(Literal)                                                \
  obhash_start(sizeof("" Literal) - 1, (const void *)"" Literal "")

#endif
