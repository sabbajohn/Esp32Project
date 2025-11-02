#pragma once

#include "components/brl/drivers/include/board_config.h"
#include <stdint.h>

typedef struct {
    int spi_host;
    int mosi;
    int sclk;
    int cs;
    int dc;
    int rst;
    int bl;
    uint16_t width;
    uint16_t height;
} st7789_init_config_t;

// Initialize the ST7789 display with the provided config.
// Returns 0 on success, negative on error.
int st7789_init(const st7789_init_config_t *config);

// Simple helper to set the backlight (0-255)
void st7789_set_backlight(uint8_t brightness);
