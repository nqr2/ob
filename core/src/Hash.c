#include <ob/Hash.h>

uint64_t hash_continue(uint64_t state, size_t len, const void *ptr) {
  const uint8_t *bytes = ptr;

  for (size_t i = 0; i < len; i++) {
    state ^= bytes[i];
    state *= FNV_PRIME;
  }

  return state;
}

uint64_t hash_start(size_t len, const void *ptr) {
  return hash_continue(FNV_OFFSET, len, ptr);
}
