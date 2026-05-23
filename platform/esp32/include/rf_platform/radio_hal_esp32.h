#ifndef RF_PLATFORM_RADIO_HAL_ESP32_H
#define RF_PLATFORM_RADIO_HAL_ESP32_H

#include "rf_platform/radio_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

const rf_radio_hal_t* rf_esp32_radio_hal_get(void);

#ifdef __cplusplus
}
#endif

#endif
