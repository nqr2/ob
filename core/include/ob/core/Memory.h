#ifndef OB_CORE_MEMORY_H_INCLUDED
#define OB_CORE_MEMORY_H_INCLUDED

/** @file
 *
 * @brief
 * Memory allocation and management.
 */

#include <ob/bits/Begin.h>

#include <stddef.h>
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

/// A virtual memory address.
typedef uint64_t ob_Address;

/// A memory allocation hint.
typedef enum {
  /// The allocation is for short-lived data.
  OB_MEM_TEMPORARY,

  /// The allocation is for longer-lived data.
  OB_MEM_DATA,

  /// The allocation is for an @ref ob_Obj value.
  OB_MEM_OBJECT,
} ob_MemUsage;

/// A *page* or block of memory provided by the system.
typedef struct {
  /// The size of this page.
  uint32_t size;

  /// An offset "separating" assigned and unused data from unassigned memory
  /// within this page.
  uint32_t bump;
  char data[];
} ob_Page;

typedef struct ob_Memory *ob_Mem;

/** @brief Allocate managed memory, and return it's address.
 *
 * This has 4 cases:
 * - `old == 0` and `new > 0` : Allocate at least @p new_size bytes.
 * - `old > 0` and `new == 0` : Deallocate at this address.
 * - `old != new` : Reallocate (shrink or grow) the allocation.
 * - Otherwise, it returns the same address.
 *
 * @note
 * - If any (de,re)allocation takes place, this must set the stats in @p self as
 *   needed.
 * - When deallocating or on allocation failure, it must return
 *   @ref OB_ADDRESS_NULL.
 * - If possible, it should return the same address on reallocation.
 * - Zeroing out any freshly allocated space is done elsewhere.
 * - The @p usage parameter is intended to be a hint, and can be safely ignored.
 *
 * @warning
 * Do not resize any allocated pages containing data.
 */
typedef ob_Address (*ob_FnMemAllocate)(ob_Mem self, ob_Address source,
                                       size_t old_size, size_t new_size,
                                       ob_MemUsage usage);

/** @brief Translate an address into a pointer.
 *
 * @returns A pointer such that, when translated back into an address, returns
 * @p addr, or @c NULL if it is equal to @ref OB_ADDRESS_NULL.
 */
typedef void *(*ob_FnMemTranslateAddress)(ob_Mem self, ob_Address addr);

/** @brief Translate a pointer into an address.
 *
 * @sa ob_FnMemTranslateAddress
 */
typedef ob_Address (*ob_FnMemTranslatePointer)(ob_Mem self,
                                               void const *pointer);

/// Obtain the page allocated for this address.
typedef ob_Page *(*ob_FnMemGetPage)(ob_Mem self, ob_Address addr);

/** @brief Perform a garbage collection, from an address "forward".
 *
 * @note
 * - The marking process is done independently of this.
 * - It must not perform a collection on addresses lower than @p from. It is not
 *   guaranteed that objects on these addresses are marked properly.
 * - Any collected live objects must have their mark cleared, and their
 *   finalizer must be called when destroyed (@ref obobj_destroy in
 *   core/Object.h)
 */
typedef void (*ob_FnMemCollect)(ob_Mem self, ob_Address from);

/// A memory allocator and garbage collector.
struct ob_Memory {
  ob_FnMemAllocate allocate;

  struct {
    ob_FnMemTranslateAddress address;
    ob_FnMemTranslatePointer pointer;
  } translate;

  ob_FnMemGetPage get_page;

  ob_FnMemCollect collect;

  struct {
    /// A measure of all currently used (live and garbage) memory.
    size_t used;

    /// A measure of all currently unassigned memory.
    size_t total;
  } stats;

  void *userdata;
};

constexpr uint64_t OB_ADDRESS_MASK = (1ULL << 53) - 1;
constexpr uint64_t OB_MAX_PAGE_SIZE = (1ULL << 32) - 1;
constexpr uint64_t OB_MAX_PAGE_INDEX = (1ULL << 21) - 1;

constexpr uint64_t OB_MIN_PAGE_SIZE = 8192;
static_assert(OB_MIN_PAGE_SIZE >= sizeof(ob_Page));

constexpr ob_Address OB_ADDRESS_NULL = 0;

ob_Address obmem_allocate(ob_Mem self, ob_Address source, size_t old_size,
                          size_t new_size, ob_MemUsage usage);
void *obmem_translate_address(ob_Mem self, ob_Address addr);
ob_Address obmem_translate_pointer(ob_Mem self, void const *pointer);
ob_Page *obmem_get_page(ob_Mem self, ob_Address addr);
void obmem_collect(ob_Mem self, ob_Address from);

ob_Address obmem_address_create(uint32_t page, uint32_t offset);
uint64_t obmem_get_page_offset(ob_Address addr);
uint64_t obmem_get_page_index(ob_Address addr);

#include <ob/bits/End.h>

#endif
