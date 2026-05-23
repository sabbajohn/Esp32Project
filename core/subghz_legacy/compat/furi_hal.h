#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

uint32_t furi_hal_subghz_get_rolling_counter_mult(void);
bool furi_hal_crypto_enclave_load_key(uint8_t slot, const uint8_t* iv);
void furi_hal_crypto_enclave_unload_key(uint8_t slot);
bool furi_hal_crypto_encrypt(const uint8_t* in, uint8_t* out, size_t len);
bool furi_hal_crypto_decrypt(const uint8_t* in, uint8_t* out, size_t len);
