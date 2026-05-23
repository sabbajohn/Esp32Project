#ifndef RF_PLATFORM_BOARD_H
#define RF_PLATFORM_BOARD_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int host;
    int miso;
    int mosi;
    int sclk;
} rf_spi_bus_def_t;

typedef struct {
    int bus_index;
    int cs;
    int gdo0;
    int gdo2;
} rf_cc1101_def_t;

typedef struct {
    int bus_index;
    int csn;
    int ce;
    int irq;
} rf_nrf24_def_t;

typedef struct {
    const char* board_name;
    rf_spi_bus_def_t spi_buses[2];
    rf_cc1101_def_t cc1101[2];
    rf_nrf24_def_t nrf24[1];
} rf_board_def_t;

const rf_board_def_t* rf_platform_board_get(void);

#ifdef __cplusplus
}
#endif

#endif
