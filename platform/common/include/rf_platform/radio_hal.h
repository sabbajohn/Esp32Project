#ifndef RF_PLATFORM_RADIO_HAL_H
#define RF_PLATFORM_RADIO_HAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "subghz_core/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct rf_radio_hal rf_radio_hal_t;

typedef enum {
    RF_HAL_OK = 0,
    RF_HAL_ERR = -1,
    RF_HAL_UNSUPPORTED = -2,
} rf_hal_status_t;

typedef struct {
    const char* name;
    subghz_radio_type_t type;
    uint8_t instance_id;
} rf_radio_desc_t;

typedef struct {
    uint32_t frequency_hz;
    uint16_t channel;
    uint16_t profile_id;
} rf_capture_profile_t;

typedef bool (*rf_hal_capture_callback_t)(const subghz_input_t* input, void* ctx);

struct rf_radio_hal {
    const char* impl_name;
    size_t (*list_radios)(rf_radio_desc_t* out, size_t cap);
    rf_hal_status_t (*open)(uint8_t instance_id);
    rf_hal_status_t (*apply_profile)(uint8_t instance_id, const rf_capture_profile_t* profile);
    rf_hal_status_t (*start_capture)(uint8_t instance_id, rf_hal_capture_callback_t cb, void* cb_ctx);
    rf_hal_status_t (*stop_capture)(uint8_t instance_id);
    void (*close)(uint8_t instance_id);
};

const rf_radio_hal_t* rf_platform_get_radio_hal(void);

#ifdef __cplusplus
}
#endif

#endif
