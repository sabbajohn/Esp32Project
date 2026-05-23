#include "rf/storage.h"

static bool sd_open(rf_storage_t* s, const char* path) {
    (void)s;
    (void)path;
    return false;
}

static bool sd_append(rf_storage_t* s, const uint8_t* buf, size_t len) {
    (void)s;
    (void)buf;
    (void)len;
    return false;
}

static bool sd_flush(rf_storage_t* s) {
    (void)s;
    return false;
}

static void sd_close(rf_storage_t* s) {
    (void)s;
}

static const rf_storage_vtable_t g_vtbl = {
    .open = sd_open,
    .append = sd_append,
    .flush = sd_flush,
    .close = sd_close,
};

const rf_storage_vtable_t* rf_esp32_storage_vtable(void) {
    return &g_vtbl;
}
