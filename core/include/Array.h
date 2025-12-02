#ifndef ARRAY_H_INCLUDED
#define ARRAY_H_INCLUDED

#include "Allocator.h"

typedef struct {
  Allocator *allocator;
  size_t size, capacity;
  void *data;
} Array;

void arr_init(Array *arr, Allocator *alloc);

void arr_free(Array *arr);
void arr_reserve(Array *arr, size_t newcap);
void arr_push(Array *arr, size_t len, const void *data);

bool arr_pop(Array *arr, size_t len, void *data);

void arr_remove(Array *arr, size_t size, size_t offset);

size_t arr_length(Array *arr, size_t size);
void *arr_at(Array *arr, size_t size, size_t index);
void *arr_last(Array *arr, size_t size);

#endif
