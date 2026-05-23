#include <lib/flipper_format/flipper_format.h>

#include <stdlib.h>
#include <string.h>

struct FlipperFormat {
    int dummy;
};

FlipperFormat* flipper_format_file_alloc(Storage* storage) {
    (void)storage;
    return calloc(1, sizeof(FlipperFormat));
}

void flipper_format_free(FlipperFormat* ff) {
    free(ff);
}

bool flipper_format_file_open_existing(FlipperFormat* ff, const char* path) {
    (void)ff;
    (void)path;
    return false;
}

bool flipper_format_file_open_always(FlipperFormat* ff, const char* path) {
    (void)ff;
    (void)path;
    return true;
}

bool flipper_format_file_close(FlipperFormat* ff) {
    (void)ff;
    return true;
}

bool flipper_format_read_header(FlipperFormat* ff, FuriString* type, uint32_t* version) {
    (void)ff;
    if(type) furi_string_set(type, "");
    if(version) *version = 0;
    return false;
}

bool flipper_format_read_uint32(FlipperFormat* ff, const char* key, uint32_t* out, size_t count) {
    (void)ff;
    (void)key;
    if(out && count > 0) memset(out, 0, sizeof(uint32_t) * count);
    return false;
}

bool flipper_format_read_int32(FlipperFormat* ff, const char* key, int32_t* out, size_t count) {
    (void)ff;
    (void)key;
    if(out && count > 0) memset(out, 0, sizeof(int32_t) * count);
    return false;
}

bool flipper_format_read_hex(FlipperFormat* ff, const char* key, uint8_t* out, size_t len) {
    (void)ff;
    (void)key;
    if(out && len > 0) memset(out, 0, len);
    return false;
}

bool flipper_format_read_string(FlipperFormat* ff, const char* key, FuriString* out) {
    (void)ff;
    (void)key;
    if(out) furi_string_set(out, "");
    return false;
}

bool flipper_format_read_bool(FlipperFormat* ff, const char* key, bool* out, size_t count) {
    (void)ff;
    (void)key;
    if(out && count > 0) memset(out, 0, sizeof(bool) * count);
    return false;
}

bool flipper_format_write_header_cstr(FlipperFormat* ff, const char* type, uint32_t version) {
    (void)ff;
    (void)type;
    (void)version;
    return true;
}

bool flipper_format_write_uint32(FlipperFormat* ff, const char* key, const uint32_t* val, size_t count) {
    (void)ff;
    (void)key;
    (void)val;
    (void)count;
    return true;
}

bool flipper_format_write_int32(FlipperFormat* ff, const char* key, const int32_t* val, size_t count) {
    (void)ff;
    (void)key;
    (void)val;
    (void)count;
    return true;
}

bool flipper_format_write_hex(FlipperFormat* ff, const char* key, const uint8_t* val, size_t len) {
    (void)ff;
    (void)key;
    (void)val;
    (void)len;
    return true;
}

bool flipper_format_write_string_cstr(FlipperFormat* ff, const char* key, const char* value) {
    (void)ff;
    (void)key;
    (void)value;
    return true;
}

bool flipper_format_insert_or_update_hex(FlipperFormat* ff, const char* key, const uint8_t* val, size_t len) {
    return flipper_format_write_hex(ff, key, val, len);
}

bool flipper_format_insert_or_update_uint32(FlipperFormat* ff, const char* key, const uint32_t* val, size_t count) {
    return flipper_format_write_uint32(ff, key, val, count);
}

bool flipper_format_update_hex(FlipperFormat* ff, const char* key, const uint8_t* val, size_t len) {
    return flipper_format_write_hex(ff, key, val, len);
}

bool flipper_format_update_uint32(FlipperFormat* ff, const char* key, const uint32_t* val, size_t count) {
    return flipper_format_write_uint32(ff, key, val, count);
}

bool flipper_format_rewind(FlipperFormat* ff) {
    (void)ff;
    return true;
}

Stream* flipper_format_get_raw_stream(FlipperFormat* ff) {
    (void)ff;
    return NULL;
}
