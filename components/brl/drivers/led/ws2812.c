#include "ws2812.h"
#include "esp_log.h"
#include "led_strip.h"

static const char *TAG = "ws2812";
static led_strip_t *strip = NULL;

int ws2812_init(int pin, int count) {
    if (strip) return 0;
    esp_err_t ret = led_strip_init(&strip, LED_STRIP_WS2812, pin, count);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "led_strip_init failed: %d", ret);
        return -1;
    }
    return 0;
}

int ws2812_set_pixel_rgb(int index, uint8_t r, uint8_t g, uint8_t b) {
    if (!strip) return -1;
    return led_strip_set_pixel(strip, index, r, g, b);
}

int ws2812_show(void) {
    if (!strip) return -1;
    return led_strip_refresh(strip, 100);
}
