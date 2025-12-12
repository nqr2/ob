#include <ob/Number.h>

ob_Number obnum_of_int(int64_t num) {
  ob_Number res = {};
  res.as_int = num << 1;
  return res;
}

ob_Number obnum_of_float(double num) {
  ob_Number res = {};
  res.as_float = num;
  res.as_word |= 1;
  return res;
}

bool obnum_is_int(ob_Number num) {
  return (num.as_word & 1) == 0;
}

int64_t obnum_to_int(ob_Number num) {
  if (obnum_is_int(num)) {
    return num.as_int >> 1;
  }

  num.as_word ^= 1;
  return (int64_t)num.as_float;
}

double obnum_to_float(ob_Number num) {
  if (obnum_is_int(num)) {
    return (double)(num.as_int >> 1);
  }

  num.as_word ^= 1;
  return num.as_float;
}
