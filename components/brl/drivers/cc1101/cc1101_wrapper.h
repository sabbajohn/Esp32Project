#pragma once

#include <stdint.h>
#include <stddef.h>
#include "components/brl/drivers/include/board_config.h"
#include <furi_hal_spi.h>

typedef struct {
    int spi_host;
    int miso;
    int mosi;
    int sclk;
    int cs;
    int gdo0;
} cc1101_init_config_t;

// Attach the existing low-level SPI handle to wrapper
int cc1101_wrapper_attach(const FuriHalSpiBusHandle* handle);
uint32_t cc1101_wrapper_set_frequency(const FuriHalSpiBusHandle* handle, uint32_t freq_hz);
int cc1101_wrapper_write_fifo(const FuriHalSpiBusHandle* handle, const uint8_t* data, uint8_t size);
int cc1101_wrapper_start_receive(const FuriHalSpiBusHandle* handle);
