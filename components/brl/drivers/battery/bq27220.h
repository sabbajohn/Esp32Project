#pragma once

#include <stdint.h>

int bq27220_init(void);
// Returns percentage 0..100 or negative on error
int bq27220_get_state_of_charge(void);
// Returns battery voltage in millivolts or negative on error
int bq27220_get_voltage_mv(void);
