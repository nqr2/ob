#include <ob/base/Hash.h>

#define FNV_PRIME 0x00000100000001b3ull
#define FNV_OFFSET 0xcbf29ce484222325ull

uint64_t ql_hash_continue(uint64_t state, size_t len, void const *ptr) {
  uint8_t const *bytes = ptr;

  for (size_t i = 0; i < len; i++) {
    state ^= bytes[i];
    state *= FNV_PRIME;
  }

  return state;
}

uint64_t ql_hash_start(size_t len, void const *ptr) {
  return ql_hash_continue(FNV_OFFSET, len, ptr);
}
