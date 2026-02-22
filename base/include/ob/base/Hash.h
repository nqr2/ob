#ifndef OB_BASE_HASH_H_INCLUDED
#define OB_BASE_HASH_H_INCLUDED

/** @file
 *
 * @brief Hashing.
 */

#include <stddef.h>
#include <stdint.h>

uint64_t ql_hash_continue(uint64_t state, size_t len, void const *ptr);
uint64_t ql_hash_start(size_t len, void const *ptr);

#define ql_hash_literal(Literal)                                               \
  ql_hash_start(sizeof("" Literal) - 1, (const void *)"" Literal "")

#endif
