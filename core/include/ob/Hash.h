#ifndef OB_CORE_HASH_H_INCLUDED
#define OB_CORE_HASH_H_INCLUDED

#include <stddef.h>
#include <stdint.h>

uint64_t obhash_continue(uint64_t state, size_t len, const void *ptr);
uint64_t obhash_start(size_t len, const void *ptr);

#define obhash_literal(Literal)                                                \
  obhash_start(sizeof("" Literal) - 1, (const void *)"" Literal "")

#endif
