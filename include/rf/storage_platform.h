#ifndef RF_STORAGE_PLATFORM_H
#define RF_STORAGE_PLATFORM_H

#include "rf/storage.h"

#ifdef __cplusplus
extern "C" {
#endif

const rf_storage_vtable_t* rf_linux_storage_vtable(void);
const rf_storage_vtable_t* rf_esp32_storage_vtable(void);

#ifdef __cplusplus
}
#endif

#endif
