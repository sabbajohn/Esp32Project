#include "rf_platform/board.h"

#if defined(BOARD_EVIL_CROW_RF_V2) && (BOARD_EVIL_CROW_RF_V2 == 1)
#include "../boards/evil_crow_rf_v2/board.h"
#else
#error "Define BOARD_EVIL_CROW_RF_V2=1 for MVP build."
#endif

static const rf_board_def_t g_board = {
    .board_name = RF_BOARD_NAME,
    .spi_buses = {
        {.host = RF_SPI_BUS0_HOST, .miso = RF_SPI_BUS0_MISO, .mosi = RF_SPI_BUS0_MOSI, .sclk = RF_SPI_BUS0_SCLK},
        {.host = RF_SPI_BUS1_HOST, .miso = RF_SPI_BUS1_MISO, .mosi = RF_SPI_BUS1_MOSI, .sclk = RF_SPI_BUS1_SCLK},
    },
    .cc1101 = {
        {.bus_index = RF_CC1101_0_BUS, .cs = RF_CC1101_0_CS, .gdo0 = RF_CC1101_0_GDO0, .gdo2 = RF_CC1101_0_GDO2},
        {.bus_index = RF_CC1101_1_BUS, .cs = RF_CC1101_1_CS, .gdo0 = RF_CC1101_1_GDO0, .gdo2 = RF_CC1101_1_GDO2},
    },
    .nrf24 = {
        {.bus_index = RF_NRF24_0_BUS, .csn = RF_NRF24_0_CSN, .ce = RF_NRF24_0_CE, .irq = RF_NRF24_0_IRQ},
    },
};

const rf_board_def_t* rf_platform_board_get(void) {
    return &g_board;
}
