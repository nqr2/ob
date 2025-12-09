#include <ob/Number.h>

Number num_of_int(int64_t num) {
  Number res = {};
  res.as_int = num << 1;
  return res;
}

Number num_of_float(double num) {
  Number res = {};
  res.as_float = num;
  res.as_word |= 1;
  return res;
}

bool num_is_int(Number num) {
  return (num.as_word & 1) == 0;
}

int64_t num_to_int(Number num) {
  if (num_is_int(num)) {
    return num.as_int >> 1;
  }

  num.as_word ^= 1;
  return (int64_t)num.as_float;
}

double num_to_float(Number num) {
  if (num_is_int(num)) {
    return (double)(num.as_int >> 1);
  }

  num.as_word ^= 1;
  return num.as_float;
}
