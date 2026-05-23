#ifndef RF_BOARD_EVIL_CROW_RF_V2_H
#define RF_BOARD_EVIL_CROW_RF_V2_H

/*
 * Inferred from reference firmware:
 * - Referencia-EvlCrwRf/firmware.ino
 *   sck_pin=14 miso_pin=12 mosi_pin=13
 *   cc1101_0: rx_pin1=4 tx_pin1=2 cs_pin1=5
 *   cc1101_1: rx_pin2=26 tx_pin2=25 cs_pin2=27
 *
 * nRF24 pins are not present in the provided reference files and remain pending.
 */

#define RF_BOARD_NAME "evil_crow_rf_v2"

#define RF_SPI_BUS0_HOST 2
#define RF_SPI_BUS0_MISO 12
#define RF_SPI_BUS0_MOSI 13
#define RF_SPI_BUS0_SCLK 14

/* Same physical SPI lines are used for both CC1101 modules with different CS */
#define RF_SPI_BUS1_HOST 2
#define RF_SPI_BUS1_MISO 12
#define RF_SPI_BUS1_MOSI 13
#define RF_SPI_BUS1_SCLK 14

#define RF_CC1101_0_BUS 0
#define RF_CC1101_0_CS 5
#define RF_CC1101_0_GDO0 4
#define RF_CC1101_0_GDO2 2

#define RF_CC1101_1_BUS 1
#define RF_CC1101_1_CS 27
#define RF_CC1101_1_GDO0 26
#define RF_CC1101_1_GDO2 25

#define RF_NRF24_0_BUS 0
#define RF_NRF24_0_CSN -1
#define RF_NRF24_0_CE -1
#define RF_NRF24_0_IRQ -1

#endif
