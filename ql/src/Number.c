#include <ql/Number.h>

ql_Number ql_number_of_int(int64_t num) {
  ql_Number res = {};
  res.as_int = num << 1;
  return res;
}

ql_Number ql_number_of_float(double num) {
  ql_Number res = {};
  res.as_float = num;
  res.as_word |= 1;
  return res;
}

bool ql_number_is_int(ql_Number num) {
  return (num.as_word & 1) == 0;
}

int64_t ql_number_to_int(ql_Number num) {
  if (ql_number_is_int(num)) {
    return num.as_int >> 1;
  }

  num.as_word ^= 1;
  return (int64_t)num.as_float;
}

double ql_number_to_float(ql_Number num) {
  if (ql_number_is_int(num)) {
    return (double)(num.as_int >> 1);
  }

  num.as_word ^= 1;
  return num.as_float;
}
