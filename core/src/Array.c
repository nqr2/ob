#include "Array.h"

#include <stdbit.h>
#include <string.h>

void arr_init(Array *arr, Allocator *alloc) {
  arr->allocator = alloc;

  arr->size = 0;
  arr->capacity = 0;
  arr->data = NULL;
}

void arr_free(Array *arr) {
  deallocate(arr->allocator, arr->data);
  arr_init(arr, NULL);
}

void arr_reserve(Array *arr, size_t newcap) {
  auto capacity = stdc_bit_ceil(newcap);

  if (capacity > arr->capacity) {
    arr->capacity = capacity;
    arr->data = reallocate(arr->allocator, arr->data, arr->capacity);
  }
}

void arr_push(Array *arr, size_t len, const void *data) {
  arr_reserve(arr, arr->size + len);

  memcpy(((uint8_t *)arr->data) + arr->size, data, len);

  arr->size += len;
}

bool arr_pop(Array *arr, size_t len, void *data) {
  if (arr->size < len) {
    return false;
  }

  arr->size -= len;

  if (data != NULL) {
    memcpy(data, ((uint8_t *)arr->data) + arr->size, len);
  }

  return true;
}

void arr_remove(Array *arr, size_t size, size_t offset) {
  uint8_t *bytes = arr->data;
  memcpy(bytes + offset, (bytes + arr->size - size), size);

  arr_pop(arr, size, NULL);
}

size_t arr_length(Array *arr, size_t size) {
  return arr->size / size;
}

void *arr_at(Array *arr, size_t size, size_t index) {
  uint8_t *bytes = arr->data;

  return bytes + index * size;
}

void *arr_last(Array *arr, size_t size) {
  return arr_at(arr, size, arr_length(arr, size) - 1);
}
