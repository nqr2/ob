#include <ob/Array.h>

#include <stdbit.h>
#include <string.h>

void obarr_init(ob_Array *arr, ob_Allocator *alloc) {
  arr->allocator = alloc;

  arr->size = 0;
  arr->capacity = 0;
  arr->data = NULL;
}

void obarr_free(ob_Array *arr) {
  ob_deallocate(arr->allocator, arr->capacity, arr->data);
  obarr_init(arr, NULL);
}

void obarr_reserve(ob_Array *arr, size_t newcap) {
  auto capacity = stdc_bit_ceil(newcap);

  if (capacity > arr->capacity) {
    arr->data =
        ob_reallocate(arr->allocator, arr->capacity, arr->data, capacity);

    arr->capacity = capacity;
  }
}

void obarr_push(ob_Array *arr, size_t len, const void *data) {
  obarr_reserve(arr, arr->size + len);

  memcpy(((uint8_t *)arr->data) + arr->size, data, len);

  arr->size += len;
}

void obarr_clear(ob_Array *arr) {
  arr->size = 0;
}

bool obarr_pop(ob_Array *arr, size_t len, void *data) {
  if (arr->size < len) {
    return false;
  }

  arr->size -= len;

  if (data != NULL) {
    memcpy(data, ((uint8_t *)arr->data) + arr->size, len);
  }

  return true;
}

void obarr_remove(ob_Array *arr, size_t size, size_t offset) {
  uint8_t *bytes = arr->data;

  auto length = arr->size / size;

  if (offset != length - 1) {
    memmove(bytes + (offset * size), bytes + (offset + 1) * size,
            (length - offset - 1) * size);
  }

  obarr_pop(arr, size, NULL);
}

size_t obarr_length(ob_Array *arr, size_t size) {
  return arr->size / size;
}

void *obarr_at(ob_Array *arr, size_t size, size_t index) {
  uint8_t *bytes = arr->data;

  return bytes + (index * size);
}

void *obarr_last(ob_Array *arr, size_t size) {
  return obarr_at(arr, size, obarr_length(arr, size) - 1);
}
