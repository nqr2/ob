#ifndef OB_BASE_ARRAY_H_INCLUDED
#define OB_BASE_ARRAY_H_INCLUDED

/** @file
 *
 * @brief Dynamically allocated arrays.
 */

#include <ob/base/Allocator.h>

typedef struct {
  ql_Allocator *allocator;
  size_t size, capacity;
  void *data;
} ql_Array;

void ql_array_init(ql_Array *arr, ql_Allocator *alloc);

void ql_array_free(ql_Array *arr);

void ql_array_reserve(ql_Array *arr, size_t newcap);

void ql_array_push(ql_Array *arr, size_t len, void const *data);

void ql_array_clear(ql_Array *arr);

bool ql_array_pop(ql_Array *arr, size_t len, void *data);

void ql_array_remove(ql_Array *arr, size_t size, size_t offset);

size_t ql_array_length(ql_Array *arr, size_t size);

void *ql_array_at(ql_Array *arr, size_t size, size_t index);

void *ql_array_last(ql_Array *arr, size_t size);

#define ql_ArrayT(...) ql_Array

#endif
