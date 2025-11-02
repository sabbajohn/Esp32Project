#pragma once

// Display ST7789V Configuration
#define DISPLAY_WIDTH           240
#define DISPLAY_HEIGHT          320
#define DISPLAY_SPI_HOST       SPI2_HOST
#define DISPLAY_SPI_MISO       GPIO_NUM_NC  // No MISO for display
#define DISPLAY_SPI_MOSI       GPIO_NUM_9
#define DISPLAY_SPI_CLK        GPIO_NUM_8
#define DISPLAY_SPI_CS         GPIO_NUM_6
#define DISPLAY_DC             GPIO_NUM_7
#define DISPLAY_RST            GPIO_NUM_5
#define DISPLAY_BACKLIGHT      GPIO_NUM_38

// CC1101 Configuration
#define CC1101_SPI_HOST       SPI3_HOST
#define CC1101_SPI_MISO       GPIO_NUM_13
#define CC1101_SPI_MOSI       GPIO_NUM_11
#define CC1101_SPI_CLK        GPIO_NUM_12
#define CC1101_SPI_CS         GPIO_NUM_10
#define CC1101_GDO0           GPIO_NUM_4    // Interrupt pin
#define CC1101_GDO2           GPIO_NUM_14   // Optional, for additional features

// Battery Management
#define BAT_CHARGE_EN         GPIO_NUM_46   // Enable charging
#define BAT_VOLTAGE_DIV_EN    GPIO_NUM_21   // Enable voltage divider
#define BAT_VOLTAGE_ADC       GPIO_NUM_1    // Battery voltage ADC
#define BAT_CHARGE_STATE      GPIO_NUM_47   // Charging status input

// I2C Configuration (For Battery Gauge BQ27220)
#define I2C_MASTER_SCL        GPIO_NUM_17
#define I2C_MASTER_SDA        GPIO_NUM_18
#define I2C_MASTER_FREQ       400000        // 400KHz
#define I2C_MASTER_NUM        I2C_NUM_0

// RGB LED (WS2812)
#define RGB_LED_PIN           GPIO_NUM_42
#define RGB_LED_COUNT         1

// User Interface
#define BUTTON_BOOT           GPIO_NUM_0    // BOOT button
#define BUTTON_USER           GPIO_NUM_41   // USER button
