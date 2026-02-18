#ifndef OB_CORE_MEMORY_H_INCLUDED
#define OB_CORE_MEMORY_H_INCLUDED

/*
 * Copyright (C) 2025-2026 nqr2
 *
 * This library is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library. If not, see <https://www.gnu.org/licenses/>.
 */

/** @file
 *
 * @brief
 * Memory allocation and management.
 */

#include <ob/bits/Begin.h>

#include <ql/Allocator.h>
#include <ql/Array.h>
#include <ql/Assert.h>

#include <stdint.h>

/*
 * I'm planning on implementing NaN boxing, so this is actually 53 bits.
 *
 * Of these 53, we use 21 bits to represent a "page index", and the remaining 32
 * for a "page offset", which leaves us with >2M pages, each almost 4GB max,
 * i.e. a LOT of memory. I say "almost 4GB" since every "page" has a header
 * attached to it.
 *
 * How the actual allocator interprets these depends on the implementation,
 * though.
 */

typedef uint64_t ob_Address;

typedef enum {
  /// The allocation is short-lived.
  OB_MEM_TEMPORARY,

  /// The allocation is for a longer-lived data structure.
  OB_MEM_DATA,

  /// The allocation is for an @ref ob_Obj value.
  OB_MEM_OBJECT,
} ob_MemUsage;

typedef struct {
  uint32_t size;
  char data[];
} ob_Page;

typedef ob_Address (*ob_FnMemAllocate)(void *self, ob_MemUsage usage,
                                       size_t requested);
typedef ob_Address (*ob_FnMemReallocate)(void *self, ob_Address source,
                                         size_t new_size);
typedef void (*ob_FnMemDeallocate)(void *self, ob_Address source);

typedef void *(*ob_FnMemTranslateAddress)(void *self, ob_Address addr);
typedef ob_Address (*ob_FnMemTranslatePointer)(void *self, void const *pointer);

typedef ob_Page *(*ob_FnMemGetPage)(void *self, ob_Address addr);

typedef struct {
  ob_FnMemAllocate allocate;
  ob_FnMemDeallocate deallocate;
  ob_FnMemReallocate reallocate;

  ob_FnMemTranslateAddress translate_address;
  ob_FnMemTranslatePointer translate_pointer;

  ob_FnMemGetPage get_page;

  struct {
    size_t used;
    size_t total;
  } stats;

  void *userdata;
} ob_Memory;

constexpr uint64_t OB_ADDRESS_MASK = (1ULL << 53) - 1;
constexpr uint64_t OB_MAX_PAGE_SIZE = (1ULL << 32) - 1;
constexpr uint64_t OB_MAX_PAGE_INDEX = (1ULL << 21) - 1;

constexpr uint64_t OB_MIN_PAGE_SIZE = 8192;
static_assert(OB_MIN_PAGE_SIZE >= sizeof("ob_PageHeader"));

ob_Address obmem_address_create(uint32_t page, uint32_t offset);
uint64_t obmem_get_page_offset(ob_Address addr);
uint64_t obmem_get_page_index(ob_Address addr);

#include <ob/bits/End.h>

#endif
