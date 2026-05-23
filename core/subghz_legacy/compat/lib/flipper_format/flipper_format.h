#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <furi.h>

typedef struct FlipperFormat FlipperFormat;
typedef struct Stream Stream;
typedef struct Storage Storage;

FlipperFormat* flipper_format_file_alloc(Storage* storage);
void flipper_format_free(FlipperFormat* ff);

bool flipper_format_file_open_existing(FlipperFormat* ff, const char* path);
bool flipper_format_file_open_always(FlipperFormat* ff, const char* path);
bool flipper_format_file_close(FlipperFormat* ff);

bool flipper_format_read_header(FlipperFormat* ff, FuriString* type, uint32_t* version);
bool flipper_format_read_uint32(FlipperFormat* ff, const char* key, uint32_t* out, size_t count);
bool flipper_format_read_int32(FlipperFormat* ff, const char* key, int32_t* out, size_t count);
bool flipper_format_read_hex(FlipperFormat* ff, const char* key, uint8_t* out, size_t len);
bool flipper_format_read_string(FlipperFormat* ff, const char* key, FuriString* out);
bool flipper_format_read_bool(FlipperFormat* ff, const char* key, bool* out, size_t count);

bool flipper_format_write_header_cstr(FlipperFormat* ff, const char* type, uint32_t version);
bool flipper_format_write_uint32(FlipperFormat* ff, const char* key, const uint32_t* val, size_t count);
bool flipper_format_write_int32(FlipperFormat* ff, const char* key, const int32_t* val, size_t count);
bool flipper_format_write_hex(FlipperFormat* ff, const char* key, const uint8_t* val, size_t len);
bool flipper_format_write_string_cstr(FlipperFormat* ff, const char* key, const char* value);

bool flipper_format_insert_or_update_hex(FlipperFormat* ff, const char* key, const uint8_t* val, size_t len);
bool flipper_format_insert_or_update_uint32(FlipperFormat* ff, const char* key, const uint32_t* val, size_t count);
bool flipper_format_update_hex(FlipperFormat* ff, const char* key, const uint8_t* val, size_t len);
bool flipper_format_update_uint32(FlipperFormat* ff, const char* key, const uint32_t* val, size_t count);
bool flipper_format_rewind(FlipperFormat* ff);

Stream* flipper_format_get_raw_stream(FlipperFormat* ff);
