#ifndef HASH_H_INCLUDED
#define HASH_H_INCLUDED

#include <stddef.h>
#include <stdint.h>

#define FNV_PRIME 0x00000100000001b3ull
#define FNV_OFFSET 0xcbf29ce484222325ull

uint64_t hash_continue(uint64_t state, size_t len, const void *ptr);
uint64_t hash_start(size_t len, const void *ptr);

#endif
