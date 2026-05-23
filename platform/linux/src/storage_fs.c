#include "rf/storage.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    FILE* f;
} fs_impl_t;

static bool fs_open(rf_storage_t* s, const char* path) {
    fs_impl_t* i = calloc(1, sizeof(*i));
    if(!i) {
        return false;
    }
    i->f = fopen(path, "ab");
    if(!i->f) {
        free(i);
        return false;
    }
    s->impl = i;
    return true;
}

static bool fs_append(rf_storage_t* s, const uint8_t* buf, size_t len) {
    fs_impl_t* i = (fs_impl_t*)s->impl;
    if(!i || !i->f) {
        return false;
    }
    return fwrite(buf, 1, len, i->f) == len;
}

static bool fs_flush(rf_storage_t* s) {
    fs_impl_t* i = (fs_impl_t*)s->impl;
    if(!i || !i->f) {
        return false;
    }
    return fflush(i->f) == 0;
}

static void fs_close(rf_storage_t* s) {
    fs_impl_t* i = (fs_impl_t*)s->impl;
    if(i) {
        if(i->f) {
            fclose(i->f);
        }
        free(i);
        s->impl = 0;
    }
}

static const rf_storage_vtable_t g_vtbl = {
    .open = fs_open,
    .append = fs_append,
    .flush = fs_flush,
    .close = fs_close,
};

const rf_storage_vtable_t* rf_linux_storage_vtable(void) {
    return &g_vtbl;
}
