#ifndef INTERNER_H_INCLUDED
#define INTERNER_H_INCLUDED

typedef struct {
    Allocator *allocator;
    Array data;     // data of every interned symbol
    Table interned; // table of offsets
} Interner;

void intr_init(Interner *intr, Allocator *alloc) {
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


#endif
