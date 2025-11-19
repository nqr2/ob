#ifndef STRING_H_INCLUDED
#define STRING_H_INCLUDED

#define STRING_MARK_BIT 0x8000'0000'0000'0000
#define STRING_LENGTH_MASK 0x7fff'ffff'ffff'ffff

typedef struct String {
    uint64_t length;
    const char *data;
    struct String *next;
} String;

typedef struct {
    size_t offset;
    size_t size;
} StrAvailable;

[[deprecated("use Context.string_data")]]
Array string_data;

[[deprecated("use Context.string_available")]]
Array string_available;

[[deprecated("use Context.strings")]]
String *strings = NULL;

String *str_create(size_t len, const char *data) {
    char *target = NULL;

    for (size_t i = 0; i < string_available.size / sizeof(StrAvailable); i++) {
        StrAvailable *avail = ((StrAvailable *)string_available.data) + i;

        if (len <= avail->size) {
            target = ((char *)string_data.data) + avail->offset;
            memcpy(target, data, len);

            avail->size -= len;

            if (avail->size == 0) {
                arr_remove(&string_available, sizeof(StrAvailable),
                           i * sizeof(StrAvailable));
                i -= 1;
            }
        }
    }

    if (target == NULL) {
        arr_push(&string_data, len, data);
        target = ((char *)string_data.data) + string_data.size - len;
    }

    String *str = allocate(allocator, sizeof(String));

    str->data = target;
    str->length = len;

    str->next = strings;
    strings = str;

    return str;
}

size_t str_len(String *str) {
    return str->length & STRING_LENGTH_MASK;
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

void str__delete(String *str) {
    StrAvailable avail = {};

    avail.offset = (str->data) - ((char *)string_data.data);
    avail.size = str->length;

    arr_push(&string_available, sizeof(StrAvailable), (void *)&avail);
}

void str_sweep() {
    String *new = NULL;

    while (strings != NULL) {
        auto next = strings->next;

        if (str_get_mark(strings)) {
            str_unmark(strings);
        } else {
            str__delete(strings);
            deallocate(allocator, strings);
            strings = NULL;
        }

        if (strings != NULL) {
            strings->next = new;
            new = strings;
        }

        strings = next;
    }

    strings = new;
}

#endif
