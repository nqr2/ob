#include <ob/base/Array.h>
#include <ob/base/Hash.h>
#include <ob/core/Context.h>
#include <ob/core/String.h>

#include <string.h>

#define STRING_MARK_BIT 0x8000'0000'0000'0000
#define STRING_LENGTH_MASK 0x7fff'ffff'ffff'ffff

typedef struct {
  size_t offset;
  size_t size;
} StrAvailable;

ob_Str obstr_create(ob_Ctx ctx, size_t len, char const *data) {
  size_t target = 0;
  auto length = ql_array_length(&ctx->string_available, sizeof(StrAvailable));

  for (size_t i = 0; i < length; i++) {
    StrAvailable *avail =
        ql_array_at(&ctx->string_available, sizeof(StrAvailable), i);

    if (len <= avail->size) {
      target = avail->offset;
      memcpy((char *)(ctx->string_data.data) + target, data, len);

      avail->size -= len;
      avail->offset += len;

      if (avail->size == 0) {
        ql_array_remove(&ctx->string_available, sizeof(StrAvailable), i);
        i -= 1;
        length--;
      }
    }
  }

  if (target == 0) {
    ql_array_push(&ctx->string_data, len, data);
    target = ctx->string_data.size - len;
  }

  ob_Str str = ql_allocate(ctx->allocator, sizeof(struct ob_String));

  str->offset = target;
  str->length = len;

  str->next = ctx->strings;
  ctx->strings = str;

  return str;
}

size_t obstr_get_length(ob_Str str) {
  return str->length & STRING_LENGTH_MASK;
}

char const *obstr_get_data(ob_Ctx ctx, ob_Str str) {
  return ((char const *)ctx->string_data.data) + str->offset;
}

uint64_t obstr_get_hash(ob_Ctx ctx, ob_Str str) {
  return ql_hash_start(str->length, obstr_get_data(ctx, str));
}

void obstr_mark(ob_Str str) {
  str->length |= STRING_MARK_BIT;
}

void obstr_unmark(ob_Str str) {
  str->length &= STRING_LENGTH_MASK;
}

bool obstr_get_mark(ob_Str str) {
  return (str->length & STRING_MARK_BIT) != 0;
}

static void str__delete(ob_Ctx ctx, ob_Str str) {
  StrAvailable avail = {};

  avail.offset = str->offset;
  avail.size = str->length;

  memset(((char *)ctx->string_data.data) + str->offset, 0, str->length);

  ql_array_push(&ctx->string_available, sizeof(StrAvailable), (void *)&avail);
}

void obstr_sweep(ob_Ctx ctx) {
  ob_Str new = NULL;

  auto strings = ctx->strings;

  while (strings != NULL) {
    auto next = strings->next;

    if (obstr_get_mark(strings)) {
      obstr_unmark(strings);
      strings->next = new;
      new = strings;
    } else {
      str__delete(ctx, strings);
      ql_deallocate(ctx->allocator, sizeof(struct ob_String), strings);
    }

    strings = next;
  }

  ctx->strings = new;
}

ob_Str obstr_concat(ob_Ctx ctx, ob_Str left, ob_Str right) {
  auto buf = (ql_Array){};
  ql_array_init(&buf, ctx->allocator);

  auto data = obstr_get_data(ctx, left);
  auto len = obstr_get_length(left);

  ql_array_push(&buf, len, data);

  data = obstr_get_data(ctx, right);
  len = obstr_get_length(right);

  ql_array_push(&buf, len, data);

  auto res = obstr_create(ctx, buf.size, buf.data);
  ql_array_free(&buf);

  return res;
}
