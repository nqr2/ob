#include <ql/Array.h>
#include <ql/Assert.h>

#include <stdbit.h>
#include <string.h>

void ql_array_init(ql_Array *arr, ql_Allocator *alloc) {
  arr->allocator = alloc;

  arr->size = 0;
  arr->capacity = 0;
  arr->data = NULL;
}

void ql_array_free(ql_Array *arr) {
  ql_deallocate(arr->allocator, arr->capacity, arr->data);
  ql_array_init(arr, NULL);
}

void ql_array_reserve(ql_Array *arr, size_t newcap) {
  auto capacity = stdc_bit_ceil(newcap);

  if (capacity > arr->capacity) {
    arr->data =
        ql_reallocate(arr->allocator, arr->capacity, arr->data, capacity);

    arr->capacity = capacity;
  }
}

void ql_array_push(ql_Array *arr, size_t len, const void *data) {
  ql_array_reserve(arr, arr->size + len);

  memcpy(((uint8_t *)arr->data) + arr->size, data, len);

  arr->size += len;
}

void ql_array_clear(ql_Array *arr) {
  arr->size = 0;
}

bool ql_array_pop(ql_Array *arr, size_t len, void *data) {
  if (arr->size < len) {
    return false;
  }

  arr->size -= len;

  if (data != NULL) {
    memcpy(data, ((uint8_t *)arr->data) + arr->size, len);
  }

  return true;
}

void ql_array_remove(ql_Array *arr, size_t size, size_t offset) {
  uint8_t *bytes = arr->data;

  auto length = arr->size / size;

  if (offset != length - 1) {
    QL_ASSERT(offset < length, "index out of bounds (%lu > %lu size)", offset,
              length);

    memmove(bytes + (offset * size), bytes + (offset + 1) * size,
            (length - offset - 1) * size);
  }

  ql_array_pop(arr, size, NULL);
}

size_t ql_array_length(ql_Array *arr, size_t size) {
  return arr->size / size;
}

void *ql_array_at(ql_Array *arr, size_t size, size_t index) {
  uint8_t *bytes = arr->data;
  auto length = ql_array_length(arr, size);

  QL_ASSERT(index < length, "index out of bounds (%lu > %lu size)", index,
            length);

  return bytes + (index * size);
}

void *ql_array_last(ql_Array *arr, size_t size) {
  auto length = ql_array_length(arr, size);

  QL_ASSERT(arr->size > 0, "cannot take last item of an empty array");

  return ql_array_at(arr, size, length - 1);
}
