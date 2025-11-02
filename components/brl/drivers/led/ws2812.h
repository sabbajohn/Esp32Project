#pragma once

#include <stdint.h>

int ws2812_init(int pin, int count);
int ws2812_set_pixel_rgb(int index, uint8_t r, uint8_t g, uint8_t b);
int ws2812_show(void);
