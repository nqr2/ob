#include <ob/core/Memory.h>

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
