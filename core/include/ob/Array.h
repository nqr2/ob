#ifndef OB_CORE_ARRAY_H_INCLUDED
#define OB_CORE_ARRAY_H_INCLUDED

/** @file
 *
 * @brief Dynamically allocated arrays.
 */

#include "Allocator.h"

typedef struct {
  ob_Allocator *allocator;
  size_t size, capacity;
  void *data;
} ob_Array;

void obarr_init(ob_Array *arr, ob_Allocator *alloc);

void obarr_free(ob_Array *arr);

void obarr_reserve(ob_Array *arr, size_t newcap);

void obarr_push(ob_Array *arr, size_t len, const void *data);

void obarr_clear(ob_Array *arr);

bool obarr_pop(ob_Array *arr, size_t len, void *data);

void obarr_remove(ob_Array *arr, size_t size, size_t offset);

size_t obarr_length(ob_Array *arr, size_t size);

void *obarr_at(ob_Array *arr, size_t size, size_t index);

void *obarr_last(ob_Array *arr, size_t size);

#endif
