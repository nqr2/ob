#ifndef OB_BASE_NUMBER_H_INCLUDED
#define OB_BASE_NUMBER_H_INCLUDED

/** @file
 *
 * @brief 63-bit integers and floats.
 */

#include <stdint.h>

typedef union {
  uint64_t as_word;
  int64_t as_int;
  double as_float;
} ql_Number;

ql_Number ql_number_of_int(int64_t num);
ql_Number ql_number_of_float(double num);

bool ql_number_is_int(ql_Number num);

int64_t ql_number_to_int(ql_Number num);
double ql_number_to_float(ql_Number num);

#endif
