#ifndef NUMBER_H_INCLUDED
#define NUMBER_H_INCLUDED

#include <stdint.h>

typedef union {
  uint64_t as_word;
  int64_t as_int;
  double as_float;
} Number;

Number num_of_int(int64_t num);
Number num_of_float(double num);

bool num_is_int(Number num);

int64_t num_to_int(Number num);
double num_to_float(Number num);

#endif
