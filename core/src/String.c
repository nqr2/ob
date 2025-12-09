#include <ob/Context.h>
#include <ob/Hash.h>
#include <ob/String.h>

#include <string.h>

String *str_create(Context ctx, size_t len, const char *data) {
  size_t target = 0;

  for (size_t i = 0; i < ctx->string_available.size / sizeof(StrAvailable);
       i++) {
    StrAvailable *avail = ((StrAvailable *)ctx->string_available.data) + i;

    if (len <= avail->size) {
      target = avail->offset;
      memcpy((char *)(ctx->string_available.data) + target, data, len);

      avail->size -= len;

      if (avail->size == 0) {
        arr_remove(&ctx->string_available, sizeof(StrAvailable),
                   i * sizeof(StrAvailable));
        i -= 1;
      }
    }
  }

  if (target == 0) {
    arr_push(&ctx->string_data, len, data);
    target = ctx->string_data.size - len;
  }

  String *str = allocate(ctx->allocator, sizeof(String));

  str->offset = target;
  str->length = len;

  str->next = ctx->strings;
  ctx->strings = str;

  return str;
}

size_t str_get_length(String *str) {
  return str->length & STRING_LENGTH_MASK;
}

const char *str_get_data(Context ctx, Str str) {
  return ((const char *)ctx->string_data.data) + str->offset;
}

uint64_t str_get_hash(Context ctx, Str str) {
  return hash_start(str->length, str_get_data(ctx, str));
}

void str_mark(String *str) {
  str->length |= STRING_MARK_BIT;
}

void str_unmark(String *str) {
  str->length &= STRING_LENGTH_MASK;
}

bool str_get_mark(String *str) {
  return (str->length & STRING_MARK_BIT) != 0;
}

static void str__delete(Context ctx, String *str) {
  StrAvailable avail = {};

  avail.offset = str->offset;
  avail.size = str->length;

  arr_push(&ctx->string_available, sizeof(StrAvailable), (void *)&avail);
}

void str_sweep(Context ctx) {
  String *new = NULL;

  auto strings = ctx->strings;

  while (strings != NULL) {
    auto next = strings->next;

    if (str_get_mark(strings)) {
      str_unmark(strings);
    } else {
      str__delete(ctx, strings);
      deallocate(ctx->allocator, strings);
      strings = NULL;
    }

    if (strings != NULL) {
      strings->next = new;
      new = strings;
    }

    strings = next;
  }

  ctx->strings = new;
}
