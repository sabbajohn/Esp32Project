#pragma once

#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <storage/storage.h>

typedef struct FuriString FuriString;

struct FuriString {
    char* data;
    size_t len;
    size_t cap;
};

#define UNUSED(x) ((void)(x))
#define COUNT_OF(x) (sizeof(x) / sizeof((x)[0]))
#define EXT_PATH(x) x

#define furi_assert(x) assert(x)
#define furi_check(x) assert(x)
#define furi_crash(...) abort()

#define FURI_LOG_E(tag, fmt, ...) fprintf(stderr, "[E][%s] " fmt "\n", (tag), ##__VA_ARGS__)
#define FURI_LOG_W(tag, fmt, ...) fprintf(stderr, "[W][%s] " fmt "\n", (tag), ##__VA_ARGS__)
#define FURI_LOG_I(tag, fmt, ...) fprintf(stderr, "[I][%s] " fmt "\n", (tag), ##__VA_ARGS__)
#define FURI_LOG_D(tag, fmt, ...) fprintf(stderr, "[D][%s] " fmt "\n", (tag), ##__VA_ARGS__)
#define FURI_LOG_RAW_D(...) fprintf(stderr, __VA_ARGS__)

FuriString* furi_string_alloc(void);
FuriString* furi_string_alloc_set(const char* src);
void furi_string_free(FuriString* s);
const char* furi_string_get_cstr(const FuriString* s);
void furi_string_set(FuriString* s, const char* src);
void furi_string_set_n(FuriString* s, const FuriString* src, size_t start, size_t n);
void furi_string_cat(FuriString* s, const char* suffix);
void furi_string_printf(FuriString* s, const char* fmt, ...);
void furi_string_cat_printf(FuriString* s, const char* fmt, ...);
void furi_delay_ms(uint32_t ms);

void* furi_record_open(const char* name);
void furi_record_close(const char* name);

#define RECORD_STORAGE "storage"
