#include <stdint.h>
#include <stdlib.h>

#define IGNORE (void)

typedef void *Ptr;

typedef Ptr (*FnAllocate)(Ptr self, size_t size);
typedef void (*FnDeallocate)(Ptr self, Ptr source);
typedef Ptr (*FnReallocate)(Ptr self, Ptr source, size_t new);

typedef struct {
  FnAllocate allocate;
  FnDeallocate deallocate;
  FnReallocate reallocate;
  Ptr self;
} VtAllocator;

static Ptr a_malloc(Ptr _self, size_t size) {
  IGNORE _self;
  return malloc(size);
}

static void a_free(Ptr _self, Ptr source) {
  IGNORE _self;
  free(source);
}

static Ptr a_realloc(Ptr _self, Ptr source, size_t new) {
  IGNORE _self;
  return realloc(source, new);
}

VtAllocator get_libc_allocator() {
  VtAllocator result = {};

  result.self = NULL;
  result.allocate = a_malloc;
  result.deallocate = a_free;
  result.reallocate = a_realloc;

  return result;
}

Ptr allocate(VtAllocator *alloc, size_t size) {
  return alloc->allocate(alloc->self, size);
}

Ptr reallocate(VtAllocator *alloc, Ptr source, size_t new) {
  return alloc->reallocate(alloc->self, source, new);
}

void deallocate(VtAllocator *alloc, Ptr source) {
  alloc->deallocate(alloc->self, source);
}

// 4 bit tag, 1 bit mark, 11 rc?
typedef uint16_t Header;

typedef enum Tag {
  OBJ_NIL = 0,
  OBJ_SYMBOL = 1,
  OBJ_STRING = 2,
  OBJ_SLOTS = 3,
  OBJ_INTEGER = 4,
  OBJ_REAL = 5,
  OBJ_CLOSURE = 6,
  OBJ_R7 = 7,
  OBJ_R8 = 8,
  OBJ_R9 = 9,
  OBJ_Ra = 10,
  OBJ_Rb = 11,
  OBJ_Rc = 12,
  OBJ_Rd = 13,
  OBJ_Re = 14,
  OBJ_Rf = 15,
} ObjectTag;

#define HEADER_GET_TAG(H) ((Header)((H) & 0xf))
#define HEADER_SET_TAG(H, T) ((Header)(((H) & ~0xf) | ((T) & 0xf)))

#define HEADER_GET_MARK(H) (((H) & 0x10) != 0)
#define HEADER_SET_MARK(H, M) ((Header)(((H) & ~0x10) | (((M) != 0) << 4)))

#define HEADER_GET_RC(H) ((H) >> 5)
#define HEADER_SET_RC(H, C) (((H) & 0x1f) | ((C) << 5))

#define RC_MAX 32

typedef struct Object {
  Header header;
  struct Object *next;
} Object;

void obj_mark(Object *obj) { obj->header = HEADER_SET_MARK(obj->header, true); }

void obj_unmark(Object *obj) {
  obj->header = HEADER_SET_MARK(obj->header, false);
}

void obj_ref(Object *obj) {
  auto refcount = HEADER_GET_RC(obj->header);

  if (refcount < RC_MAX) {
    refcount++;
    obj->header = HEADER_SET_RC(obj->header, refcount);
  }
}

// true if rc=0
bool obj_unref(Object *obj) {
  auto refcount = HEADER_GET_RC(obj->header);

  if (refcount < RC_MAX) {
    refcount--;
    obj->header = HEADER_SET_RC(obj->header, refcount);
  }

  return refcount == 0;
}

typedef struct {
  VtAllocator *allocator;
  size_t length, capacity, item_size;
  Ptr data;
} Array;

void arr_init(Array *arr, VtAllocator *alloc, size_t item) {
  arr->allocator = alloc;
  arr->item_size = item;

  arr->length = 0;
  arr->capacity = 0;
  arr->data = NULL;
}

#define ARR_INIT_T(Arr, Alloc, T) arr_init((Arr), (Alloc), sizeof(T))

void arr_free(Array *arr) {
  deallocate(arr->allocator, arr->data);
  arr_init(arr, NULL, 0);
}

int main() { return 0; }
