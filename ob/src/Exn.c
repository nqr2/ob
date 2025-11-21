#include "Exn.h"

void exn__begin(Exnbuf *buf, jmp_buf jmp) {
}

void exn__end(Exnbuf *buf) {
}

void exn_init(Exnbuf *buf, Allocator *alloc) {
}

void exn_free(Exnbuf *buf) {
}

const Exndata *exn_data(Exnbuf *buf) {
}

void exn_throw(Exnbuf *buf, Exndata data) {
}

void exn_rethrow(Exnbuf *buf) {
}
