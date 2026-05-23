#ifndef RF_PROFILE_STORE_H
#define RF_PROFILE_STORE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "rf_platform/radio_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RF_MAX_PROFILES 32

typedef struct {
    uint16_t id;
    char name[48];
    rf_capture_profile_t cfg;
} rf_profile_t;

typedef struct {
    rf_profile_t items[RF_MAX_PROFILES];
    size_t count;
    int active_index;
} rf_profile_store_t;

void rf_profile_store_init_defaults(rf_profile_store_t* store);
bool rf_profile_store_load_csv(rf_profile_store_t* store, const char* path);
const rf_profile_t* rf_profile_store_get_active(const rf_profile_store_t* store);
const rf_profile_t* rf_profile_store_find_by_id(const rf_profile_store_t* store, uint16_t id);
bool rf_profile_store_set_active(rf_profile_store_t* store, uint16_t id);

#ifdef __cplusplus
}
#endif

#endif
