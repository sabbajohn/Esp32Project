#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_rtc.h>
#include <storage/storage.h>
#include <toolbox/hex.h>
#include <toolbox/stream/stream.h>

#include <stdarg.h>
#include <time.h>

typedef struct Stream {
    uint8_t* data;
    size_t size;
    size_t pos;
} Stream;

static Storage g_storage;

static bool furi_string_reserve(FuriString* s, size_t needed) {
    if(needed <= s->cap) return true;
    size_t cap = s->cap ? s->cap : 32u;
    while(cap < needed) {
        cap *= 2u;
    }
    char* next = realloc(s->data, cap);
    if(!next) return false;
    s->data = next;
    s->cap = cap;
    return true;
}

FuriString* furi_string_alloc(void) {
    FuriString* s = calloc(1, sizeof(FuriString));
    if(!s) return NULL;
    if(!furi_string_reserve(s, 1)) {
        free(s);
        return NULL;
    }
    s->data[0] = '\0';
    return s;
}

FuriString* furi_string_alloc_set(const char* src) {
    FuriString* s = furi_string_alloc();
    if(!s) return NULL;
    furi_string_set(s, src ? src : "");
    return s;
}

void furi_string_free(FuriString* s) {
    if(!s) return;
    free(s->data);
    free(s);
}

const char* furi_string_get_cstr(const FuriString* s) {
    return (s && s->data) ? s->data : "";
}

void furi_string_set(FuriString* s, const char* src) {
    if(!s) return;
    const char* in = src ? src : "";
    size_t len = strlen(in);
    if(!furi_string_reserve(s, len + 1u)) return;
    memcpy(s->data, in, len + 1u);
    s->len = len;
}

void furi_string_set_n(FuriString* s, const FuriString* src, size_t start, size_t n) {
    if(!s) return;
    const char* begin = "";
    size_t src_len = 0;
    if(src && src->data) {
        src_len = src->len;
        if(start < src_len) {
            begin = src->data + start;
            if(n > (src_len - start)) {
                n = src_len - start;
            }
        } else {
            n = 0;
        }
    } else {
        n = 0;
    }
    if(!furi_string_reserve(s, n + 1u)) return;
    if(n > 0) {
        memcpy(s->data, begin, n);
    }
    s->data[n] = '\0';
    s->len = n;
}

void furi_string_cat(FuriString* s, const char* suffix) {
    if(!s) return;
    const char* in = suffix ? suffix : "";
    size_t add = strlen(in);
    if(!furi_string_reserve(s, s->len + add + 1u)) return;
    memcpy(s->data + s->len, in, add + 1u);
    s->len += add;
}

static void furi_string_vprintf_internal(FuriString* s, bool append, const char* fmt, va_list ap) {
    if(!s || !fmt) return;
    va_list cp;
    va_copy(cp, ap);
    int needed = vsnprintf(NULL, 0, fmt, cp);
    va_end(cp);
    if(needed < 0) return;

    size_t base = append ? s->len : 0u;
    size_t total = base + (size_t)needed;
    if(!furi_string_reserve(s, total + 1u)) return;
    vsnprintf(s->data + base, (size_t)needed + 1u, fmt, ap);
    s->len = total;
}

void furi_string_printf(FuriString* s, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    furi_string_vprintf_internal(s, false, fmt, ap);
    va_end(ap);
}

void furi_string_cat_printf(FuriString* s, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    furi_string_vprintf_internal(s, true, fmt, ap);
    va_end(ap);
}

void* furi_record_open(const char* name) {
    UNUSED(name);
    return &g_storage;
}

void furi_record_close(const char* name) {
    UNUSED(name);
}

uint32_t furi_hal_subghz_get_rolling_counter_mult(void) {
    return 1u;
}

bool furi_hal_crypto_enclave_load_key(uint8_t slot, const uint8_t* iv) {
    UNUSED(slot);
    UNUSED(iv);
    return true;
}

void furi_hal_crypto_enclave_unload_key(uint8_t slot) {
    UNUSED(slot);
}

bool furi_hal_crypto_encrypt(const uint8_t* in, uint8_t* out, size_t len) {
    if(!in || !out) return false;
    memcpy(out, in, len);
    return true;
}

bool furi_hal_crypto_decrypt(const uint8_t* in, uint8_t* out, size_t len) {
    if(!in || !out) return false;
    memcpy(out, in, len);
    return true;
}

uint32_t furi_hal_rtc_get_timestamp(void) {
    return (uint32_t)time(NULL);
}

bool hex_char_to_hex_nibble(char c, uint8_t* out) {
    if(!out) return false;
    if(c >= '0' && c <= '9') {
        *out = (uint8_t)(c - '0');
        return true;
    }
    if(c >= 'a' && c <= 'f') {
        *out = (uint8_t)(10 + (c - 'a'));
        return true;
    }
    if(c >= 'A' && c <= 'F') {
        *out = (uint8_t)(10 + (c - 'A'));
        return true;
    }
    return false;
}

size_t stream_read(Stream* s, void* data, size_t len) {
    if(!s || !data || s->pos >= s->size) return 0;
    size_t n = s->size - s->pos;
    if(n > len) n = len;
    memcpy(data, s->data + s->pos, n);
    s->pos += n;
    return n;
}

size_t stream_write(Stream* s, const void* data, size_t len) {
    UNUSED(s);
    UNUSED(data);
    return len;
}

void stream_write_char(Stream* s, char c) {
    UNUSED(s);
    UNUSED(c);
}

void stream_write_cstring(Stream* s, const char* str) {
    UNUSED(s);
    UNUSED(str);
}

bool stream_seek(Stream* s, size_t position) {
    if(!s || position > s->size) return false;
    s->pos = position;
    return true;
}

size_t stream_size(Stream* s) {
    return s ? s->size : 0u;
}

void stream_clean(Stream* s) {
    if(!s) return;
    s->pos = 0;
    s->size = 0;
}

void furi_delay_ms(uint32_t ms) {
    UNUSED(ms);
}

bool storage_simply_mkdir(Storage* storage, const char* path) {
    UNUSED(storage);
    UNUSED(path);
    return true;
}

bool storage_simply_remove(Storage* storage, const char* path) {
    UNUSED(storage);
    UNUSED(path);
    return true;
}
