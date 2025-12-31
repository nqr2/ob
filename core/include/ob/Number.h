#ifndef OB_CORE_NUMBER_H_INCLUDED
#define OB_CORE_NUMBER_H_INCLUDED

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
 * @brief 63-bit integers and floats.
 */

#include <stdint.h>

typedef union {
  uint64_t as_word;
  int64_t as_int;
  double as_float;
} ob_Number;

ob_Number obnum_of_int(int64_t num);
ob_Number obnum_of_float(double num);

bool obnum_is_int(ob_Number num);

int64_t obnum_to_int(ob_Number num);
double obnum_to_float(ob_Number num);

#endif
