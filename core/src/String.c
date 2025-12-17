#include <ob/Context.h>
#include <ob/Hash.h>
#include <ob/String.h>

#include <string.h>

#define STRING_MARK_BIT 0x8000'0000'0000'0000
#define STRING_LENGTH_MASK 0x7fff'ffff'ffff'ffff

typedef struct {
  size_t offset;
  size_t size;
} StrAvailable;

ob_String *obstr_create(ob_Context ctx, size_t len, const char *data) {
  size_t target = 0;

  for (size_t i = 0; i < ctx->string_available.size / sizeof(StrAvailable);
       i++) {
    StrAvailable *avail = ((StrAvailable *)ctx->string_available.data) + i;

    if (len <= avail->size) {
      target = avail->offset;
      memcpy((char *)(ctx->string_available.data) + target, data, len);

      avail->size -= len;

      if (avail->size == 0) {
        obarr_remove(&ctx->string_available, sizeof(StrAvailable),
                     i * sizeof(StrAvailable));
        i -= 1;
      }
    }
  }

  if (target == 0) {
    obarr_push(&ctx->string_data, len, data);
    target = ctx->string_data.size - len;
  }

  ob_String *str = ob_allocate(ctx->allocator, sizeof(ob_String));

  str->offset = target;
  str->length = len;

  str->next = ctx->strings;
  ctx->strings = str;

  return str;
}

size_t obstr_get_length(ob_String *str) {
  return str->length & STRING_LENGTH_MASK;
}

const char *obstr_get_data(ob_Context ctx, ob_Str str) {
  return ((const char *)ctx->string_data.data) + str->offset;
}

uint64_t obstr_get_hash(ob_Context ctx, ob_Str str) {
  return obhash_start(str->length, obstr_get_data(ctx, str));
}

void obstr_mark(ob_String *str) {
  str->length |= STRING_MARK_BIT;
}

void obstr_unmark(ob_String *str) {
  str->length &= STRING_LENGTH_MASK;
}

bool obstr_get_mark(ob_String *str) {
  return (str->length & STRING_MARK_BIT) != 0;
}

static void str__delete(ob_Context ctx, ob_String *str) {
  StrAvailable avail = {};

  avail.offset = str->offset;
  avail.size = str->length;

  obarr_push(&ctx->string_available, sizeof(StrAvailable), (void *)&avail);
}

void obstr_sweep(ob_Context ctx) {
  ob_String *new = NULL;

  auto strings = ctx->strings;

  while (strings != NULL) {
    auto next = strings->next;

    if (obstr_get_mark(strings)) {
      obstr_unmark(strings);
      strings->next = new;
      new = strings;
    } else {
      str__delete(ctx, strings);
      ob_deallocate(ctx->allocator, sizeof(struct String), strings);
    }

    strings = next;
  }

  ctx->strings = new;
}
