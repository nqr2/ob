#include <ob/base/Array.h>
#include <ob/base/Assert.h>
#include <ob/core/Memory.h>

#include <string.h>

ob_Address obmem_allocate(ob_Mem self, ob_Address source, size_t old_size,
                          size_t new_size, ob_MemUsage usage) {
  QL_ASSERT(self->allocate != nullptr, "unexpected null");

  ob_Address addr = self->allocate(self, source, old_size, new_size, usage);

  if (new_size != 0) {
    QL_ASSERT(addr != OB_ADDRESS_NULL, "allocation of %zu bytes failed",
              new_size);

    char *mem = obmem_translate_address(self, addr);

    if (new_size > old_size) {
      memset(mem + old_size, 0, new_size - old_size);
    }
  }

  return addr;
}

void *obmem_translate_address(ob_Mem self, ob_Address addr) {
  QL_ASSERT(self->translate.address != nullptr, "unexpected null");

  if (addr == OB_ADDRESS_NULL) {
    return nullptr;
  }

  return self->translate.address(self, addr);
}

ob_Address obmem_translate_pointer(ob_Mem self, void const *pointer) {
  QL_ASSERT(self->translate.pointer != nullptr, "unexpected null");

  if (pointer == nullptr) {
    return OB_ADDRESS_NULL;
  }

  return self->translate.pointer(self, pointer);
}

ob_Page *obmem_get_page(ob_Mem self, ob_Address addr) {
  QL_ASSERT(self->get_page != nullptr, "unexpected null");
  return self->get_page(self, addr);
}

ob_Address obmem_address_create(uint32_t page, uint32_t offset) {
  QL_ASSERT(page <= OB_MAX_PAGE_INDEX, "page index larger than allowed");
  QL_ASSERT(offset <= OB_MAX_PAGE_SIZE, "page offset larger than page size");

  auto p64 = (uint64_t)page;
  auto o64 = (uint64_t)offset;

  return (o64 << 32) | p64;
}

uint64_t obmem_get_page_offset(ob_Address addr) {
  return (addr) & ((1ULL << 32) - 1);
}

uint64_t obmem_get_page_index(ob_Address addr) {
  return (addr & OB_ADDRESS_MASK) >> 32;
}

//* ------------------------------------------------------------------------ *//
// implementing malloc1

struct malloc1 {};

ob_Memory obmem_create_malloc1() {
  auto mem = (ob_Memory){

  };

  return mem;
}
