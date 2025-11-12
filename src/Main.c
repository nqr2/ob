#include <stdbit.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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
  OBJ_NIL = 0,        // the nil object
  OBJ_SYMBOL = 1,     // #... / #a:b:...y:z: / #'...' / #+...-
  OBJ_STRING = 2,     // '...'
  OBJ_SLOTS = 3,      // slot objects
  OBJ_INTEGER = 4,    // integers
  OBJ_REAL = 5,       // floats
  OBJ_CLOSURE = 6,    // closures
  OBJ_CFUNCTION = 7,  // functions from C
  OBJ_CDATA = 8,      // data from C
  OBJ_ACTIVATION = 9, // call stack entry
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

typedef void (*FnVisitor)(Object *obj);

// NOTE: this also invokes visit on the obj in question
void obj_visit(Object *obj, FnVisitor visit);

void obj_mark(Object *obj) {
  if (HEADER_GET_MARK(obj->header)) {
    return;
  }

  obj->header = HEADER_SET_MARK(obj->header, true);

  obj_visit(obj, obj_mark);
}

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
  size_t size, capacity;
  Ptr data;
} Array;

void arr_init(Array *arr, VtAllocator *alloc) {
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

Object *live = NULL;

Object *obj_create(VtAllocator *alloc, size_t payload_size) {
  auto obj = (Object *)allocate(alloc, sizeof(Object) + payload_size);

  obj->header = 0;
  obj->next = live;

  live = obj;

  return obj;
}

static void obj__unref_(Object *obj) {
  IGNORE obj_unref(obj);
}

// NOTE: call before destroying an obj
void obj__destroy(Object *obj);

Ptr obj_payload(Object *obj) {
  auto bytes = (uint8_t *)obj;
  return bytes + sizeof(Object);
}

#define OBJ_CREATE_T(A, T) obj_create((A), sizeof(T))

void sweep(VtAllocator *alloc) {
  while ((live != NULL) && !(HEADER_GET_MARK(live->header))) {
    auto next = live->next;

    // TODO: recursively unref everything from live
    deallocate(alloc, live);

    live = next;
  }

  Object *newlive = live;

  while (live != NULL) {
    if (!(HEADER_GET_MARK(live->header))) {
      Object *next = live->next;

      obj__destroy(live);
      deallocate(alloc, live);

      live = next;

    } else {
      obj_unmark(live);
      live = live->next;
    }
  }

  live = newlive;
}

Array stack;

void obj_push(Object *obj) {
  arr_push(&stack, sizeof(Object *), (const void *)&obj);
}

Object *obj_pop() {
  Object *obj;

  if (!arr_pop(&stack, sizeof(Object *), (void *)&obj)) {
    // TODO: fail? cannot pop empty stack.
  }

  return obj;
}

typedef enum {
  TES_EMPTY = 0,
  TES_USED = 1,
  TES_DEAD = 2,
} TableEntryStatus;

typedef struct {
  uint64_t key;

  void *value;
  TableEntryStatus status;
} TableEntry;

typedef struct {
  VtAllocator *allocator;
  size_t length, capacity;
  TableEntry *data;
} Table;

#define FNV_PRIME 0x00000100000001b3ull
#define FNV_OFFSET 0xcbf29ce484222325ull

uint64_t hash_continue(uint64_t state, size_t len, void const *ptr) {
  const uint8_t *bytes = ptr;

  for (size_t i = 0; i < len; i++) {
    state ^= bytes[i];
    state *= FNV_PRIME;
  }

  return state;
}

uint64_t hash_start(size_t len, void const *ptr) {
  return hash_continue(FNV_OFFSET, len, ptr);
}

void tbl_init(Table *tbl, VtAllocator *alloc) {
  tbl->allocator = alloc;
  tbl->length = 0;
  tbl->capacity = 0;
  tbl->data = NULL;
}

void tbl_free(Table *tbl) {
  deallocate(tbl->allocator, tbl->data);

  tbl_init(tbl, NULL);
}

TableEntry *tbl__find(size_t capacity, TableEntry *entries, uint64_t key) {
  auto index = key % capacity;

  while (true) {
    auto entry = &entries[index];

    if (entry->key == key || entry->status != TES_USED) {
      return entry;
    }

    index = (index + 1) % capacity;
  }

  return NULL;
}

void tbl_reserve(Table *tbl, size_t newcap) {
  newcap = stdc_bit_ceil(newcap);
  auto new_entries =
      (TableEntry *)allocate(tbl->allocator, newcap * sizeof(TableEntry));

  memset(new_entries, 0, newcap * sizeof(TableEntry));

  tbl->length = 0;

  for (size_t i = 0; i < tbl->capacity; i++) {
    auto entry = &tbl->data[i];
    TableEntry *dest = NULL;

    // shouldnt this be != TES_USED?
    // else it keeps all tombstones???
    if (entry->status == TES_EMPTY) {
      continue;
    }

    dest = tbl__find(newcap, new_entries, entry->key);

    memcpy(dest, entry, sizeof(TableEntry));

    tbl->length++;
  }

  deallocate(tbl->allocator, tbl->data);

  tbl->data = new_entries;
  tbl->capacity = newcap;
}

// return true if entry is new
bool tbl_set(Table *tbl, uint64_t key, void *value) {
  TableEntry *entry = NULL;
  auto is_new = false;

  if (2 * (tbl->length + 1) > tbl->capacity) {
    tbl_reserve(tbl, tbl->length + 1);
  }

  entry = tbl__find(tbl->capacity, tbl->data, key);
  is_new = entry->status != TES_USED;

  if (is_new) {
    tbl->length++;
  }

  entry->key = key;
  entry->value = value;
  entry->status = TES_USED;
  return is_new;
}

void tbl_merge(Table *tbl, Table *from) {
  for (size_t i = 0; i < from->capacity; i++) {
    auto entry = &from->data[i];

    if (entry->status != TES_USED) {
      tbl_set(tbl, entry->key, entry->value);
    }
  }
}

bool tbl_get(Table *tbl, uint64_t key, void **value) {
  if (tbl->length == 0) {
    return false;
  }

  auto index = key % tbl->capacity;
  const auto start = index;

  while (true) {
    auto entry = &tbl->data[index];

    if (entry->key == key && entry->status == TES_USED) {
      if (value != NULL) {
        *value = entry->value;
      }

      return true;
    }

    index += 1;
    index %= tbl->capacity;

    if (index == start) {
      return false;
    }
  }

  return false;
}

bool tbl_remove(Table *table, uint64_t key) {
  TableEntry *entry = NULL;

  if (table->length == 0) {
    return false;
  }

  entry = tbl__find(table->capacity, table->data, key);

  if (entry->status != TES_USED) {
    return false;
  }

  entry->status = TES_DEAD;
  return true;
}

bool tbl_iterate(Table *table, uint64_t *index, uint64_t *key, void **value) {
  uint64_t current_key = *index;

  while (current_key < table->capacity) {
    auto entry = &table->data[current_key];

    if (entry->status == TES_USED) {
      if (key != NULL) {
        *key = entry->key;
      }

      if (key != NULL) {
        *value = entry->value;
      }

      break;
    }

    current_key += 1;
  }

  if (current_key >= table->capacity) {
    return false;
  }

  current_key += 1;

  *index = current_key;
  return true;
}

typedef uint64_t Symbol;

// NOTE: reverse mark, so 1 means "unreachable, to delete"
#define STRING_MARK_BIT 0x8000'0000'0000'0000

typedef struct String {
  uint64_t length;
  const char *data;
  struct String *next;
} String;

typedef struct {
  VtAllocator *allocator;
  Array data;     // data of every interned symbol
  Table interned; // table of offsets
} Interner;

Array string_data;
Array string_available;
String *strings = NULL;

String *str_create(size_t len, const char *data) {
  IGNORE len;
  IGNORE data;
  return NULL;
}

void intr_init(Interner *intr, VtAllocator *alloc) {
  memset(intr, 0, sizeof(Interner));
  intr->allocator = alloc;
  arr_init(&intr->data, alloc);
  tbl_init(&intr->interned, alloc);
}

void intr_free(Interner *intr) {
  arr_free(&intr->data);
  tbl_free(&intr->interned);

  auto str = strings;

  while (str != NULL) {
    auto next = str->next;

    deallocate(intr->allocator, str);

    str = next;
  }

  intr_init(intr, NULL);
}

// TODO: uninterning, etc
String *intr_intern(Interner *intr, size_t length, const char *data) {
  uint64_t hash = hash_start(length, data);

  String *str = NULL;
  if (!tbl_get(&intr->interned, hash, (void **)&str)) {
    str = (String *)allocate(intr->allocator, sizeof(String));

    arr_push(&intr->data, length, data);

    str->next = strings;
    str->data = ((const char *)&intr->data.data) + (intr->data.size - length);

    strings = str;

    tbl_set(&intr->interned, hash, str);
  }

  return str;
}

String *intr_find(Interner *intr, uint64_t hash) {
  String *str = NULL;

  tbl_get(&intr->interned, hash, (void **)&str);

  return str;
}

typedef struct {
  String *symbol;
} ObjSymbol;

typedef struct {
  String *inner;
} ObjString;

typedef struct {
  Object *prototype;
  Table slots;
} ObjSlots;

typedef struct {
  Object *env;
  Array parameters;
  Array bytecode;
  Array literals;
} ObjClosure;

typedef bool (*FnCFunction)();

typedef struct {
  FnCFunction cfunction;
} ObjCFunction;

typedef struct {
  void *cdata;
} ObjCData;

typedef struct {
  int64_t number;
} ObjInteger;

typedef struct {
  double number;
} ObjReal;

typedef struct {
  Object *parent;   // the parent activation
  Object *caller;   // the method's caller
  Object *method;   // this method
  Object *receiver; // this method's receiver
  Object *env;      // this context's environment
} ObjActivation;

// TODO: add every case
void obj__destroy(Object *obj) {
  obj_visit(obj, obj__unref_);

  switch (HEADER_GET_TAG(obj->header)) {

  case OBJ_SLOTS: {
    ObjSlots *data = obj_payload(obj);
    tbl_free(&data->slots);
  } break;

  case OBJ_CLOSURE: {
    ObjClosure *data = obj_payload(obj);
    arr_free(&data->parameters);
    arr_free(&data->literals);
    arr_free(&data->bytecode);
  } break;

    // TODO: implement uninterning afterwards

  default:
    break;
  }
}

void obj_visit(Object *obj, FnVisitor visit) {
  visit(obj);

  switch (HEADER_GET_TAG(obj->header)) {
  case OBJ_SLOTS: {
    ObjSlots *data = obj_payload(obj);

    Object *ref = NULL;
    uint64_t index = 0;

    while (tbl_iterate(&data->slots, &index, NULL, (void **)&ref)) {
      obj_visit(ref, visit);
    }

    obj_visit(data->prototype, visit);
  } break;

  case OBJ_CLOSURE: {
    ObjClosure *data = obj_payload(obj);

    for (size_t i = 0; i < data->literals.size / sizeof(Object *); i++) {
      Object *item = ((Object **)data->literals.data)[i];

      obj_visit(item, visit);
    }

    obj_visit(data->env, visit);
  }; break;

  case OBJ_ACTIVATION: {
    ObjActivation *data = obj_payload(obj);
    obj_visit(data->parent, visit);
    obj_visit(data->caller, visit);
    obj_visit(data->method, visit);
    obj_visit(data->receiver, visit);
    obj_visit(data->env, visit);
  }; break;

  default:
    break;
  }
}

#define MAX_PROTOTYPES 16
Object *base_prototypes[MAX_PROTOTYPES];

Object *obj_getproto(Object *obj) {
  auto tag = HEADER_GET_TAG(obj->header);

  if (tag == OBJ_SLOTS) {
    ObjSlots *data = obj_payload(obj);

    if (data->prototype != NULL) {
      return data->prototype;
    }
  }

  return base_prototypes[tag];
}

Object *obj_create_slots(VtAllocator *alloc, Object *prototype) {
  Object *obj = obj_create(alloc, sizeof(ObjSlots));

  obj->header = HEADER_SET_TAG(0, OBJ_SLOTS);

  ObjSlots *data = obj_payload(obj);
  data->prototype = prototype;

  return obj;
}

void init_prototypes(VtAllocator *alloc) {
  for (int i = 0; i < MAX_PROTOTYPES; i++) {
    base_prototypes[i] = obj_create_slots(alloc, NULL);
  }
}

Object *activation;

void mark() {
  obj_mark(activation);

  for (int i = 0; i < MAX_PROTOTYPES; i++) {
    obj_mark(base_prototypes[i]);
  }

  auto data = (Object **)stack.data;

  for (size_t i = 0; i < stack.size / sizeof(Object *); i++) {
    obj_mark(data[i]);
  }
}

VtAllocator *allocator;

typedef uint8_t Instruction;

#define INSN_GET_OPCODE(I) ((I) & 0xf)
#define INSN_GET_DATA(I) ((I) >> 4)

typedef enum {
  OP_PUSH_LITERAL = 0,  // push a literal
  OP_SEND = 1,          // send a message to a known receiver
  OP_IMPLICIT_SEND = 2, // send a message to the implicit receiver
  OP_EXTEND = 3,        // extend the payload by prepending 4 bits
  OP_RETURN = 4,        // implements ^
  OP_SELF = 5,          // push the explicit receiver
  OP_SELF_SEND = 6,     // send a message to the explicit receiver
  OP_R7 = 7,
  OP_R8 = 8,
  OP_R9 = 9,
  OP_Ra = 10,
  OP_Rb = 11,
  OP_Rc = 12,
  OP_Rd = 13,
  OP_Re = 14,
  OP_Rf = 15,
} Opcode;

Object *activation = NULL;

void enter_activation(Object *caller, Object *method, Object *receiver) {
  Object *act = obj_create(allocator, sizeof(ObjActivation));

  ObjActivation *data = obj_payload(act);
  data->parent = activation;
  data->caller = caller;
  data->method = method;
  data->receiver = receiver;
  data->env = obj_create_slots(allocator, NULL);

  activation = act;
}

void leave_activation() {
  ObjActivation *data = obj_payload(activation);
  activation = data->parent;
}

bool obj_isa(Object *obj, ObjectTag tag) {
  return HEADER_GET_TAG(obj->header) == tag;
}

Object *obj_get(Object *obj, String *selector) {
  if (obj_isa(obj, OBJ_SLOTS)) {
    ObjSlots *data = obj_payload(obj);
    IGNORE data;

    auto hash = hash_start(selector->length, selector->data);

    if (tbl_get(&data->slots, hash, (void **)&obj)) {
      return obj;
    }
  }

  return obj_get(obj_getproto(obj), selector);
}

int main() {
  auto alloc = get_libc_allocator();
  allocator = &alloc;

  init_prototypes(&alloc);

  arr_init(&stack, &alloc);

  auto one = OBJ_CREATE_T(&alloc, int);
  auto two = OBJ_CREATE_T(&alloc, int);

  IGNORE one;
  IGNORE two;

  sweep(&alloc);

  arr_free(&stack);

  return 0;
}
