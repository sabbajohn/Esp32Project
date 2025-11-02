#include "st7789.h"
#include "esp_log.h"
#include <string.h>
#include "driver/spi_master.h"
#include "driver/gpio.h"

static const char *TAG = "st7789";

int st7789_init(const st7789_init_config_t *config) {
    if (!config) return -1;
    ESP_LOGI(TAG, "Initializing ST7789 (%dx%d)", config->width, config->height);

    // Minimal SPI + GPIO setup. Real implementation should send init commands.
    spi_bus_config_t buscfg = {
        .miso_io_num = -1,
        .mosi_io_num = config->mosi,
        .sclk_io_num = config->sclk,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };
    esp_err_t ret = spi_bus_initialize(config->spi_host, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "spi_bus_initialize failed: %d", ret);
        return -2;
    }

    gpio_set_direction(config->dc, GPIO_MODE_OUTPUT);
    gpio_set_direction(config->rst, GPIO_MODE_OUTPUT);
    gpio_set_direction(config->bl, GPIO_MODE_OUTPUT);

    // Toggle reset
    gpio_set_level(config->rst, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(config->rst, 1);
    vTaskDelay(pdMS_TO_TICKS(120));

    // Backlight on by default
    st7789_set_backlight(255);

    // Note: full initialization sequence (commands) must be sent via SPI
    return 0;
}

void st7789_set_backlight(uint8_t brightness) {
    // For now simple on/off via GPIO. PWM could be implemented later.
    gpio_set_level(DISPLAY_BACKLIGHT, brightness > 10 ? 1 : 0);
}
