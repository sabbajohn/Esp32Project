#ifndef RF_STORAGE_H
#define RF_STORAGE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct rf_storage rf_storage_t;

typedef struct {
    bool (*open)(rf_storage_t* s, const char* path);
    bool (*append)(rf_storage_t* s, const uint8_t* buf, size_t len);
    bool (*flush)(rf_storage_t* s);
    void (*close)(rf_storage_t* s);
} rf_storage_vtable_t;

struct rf_storage {
    const rf_storage_vtable_t* vtbl;
    void* impl;
};

#ifdef __cplusplus
}
#endif

#endif
