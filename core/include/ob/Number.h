#ifndef OB_CORE_NUMBER_H_INCLUDED
#define OB_CORE_NUMBER_H_INCLUDED

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
