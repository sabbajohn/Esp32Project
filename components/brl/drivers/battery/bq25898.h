#pragma once

#include <stdint.h>
#include <stdbool.h>

// Basic API for BQ25898 charger driver
int bq25898_init(void);
bool bq25898_enable_charging(bool enable);
// Read charger status: returns bitmask or negative on error
int bq25898_get_status(void);
