# Board Target: Evil Crow RF v2

Board-alvo do projeto: **Evil Crow RF v2** (ESP32 + 2x CC1101 + 1x nRF24).

Referência fornecida:
- https://www.bordergate.co.uk/evilcrowrf-v2/

## Status atual no projeto

- O board template já existe em:
  - `platform/esp32/boards/evil_crow_rf_v2/board.h`
- A seleção por build flag já existe:
  - `BOARD_EVIL_CROW_RF_V2=1`
- A pinagem dos **2 CC1101** foi inferida e preenchida a partir do firmware de referência.
- A pinagem do **nRF24** segue pendente (não aparece nos arquivos de referência enviados).

## Referência usada para inferência

Arquivos:
- `Referencia-EvlCrwRf/firmware.ino`
- `Referencia-EvlCrwRf/ELECHOUSE_CC1101_SRC_DRV.cpp`
- `Referencia-EvlCrwRf/ELECHOUSE_CC1101_SRC_DRV.h`

Campos encontrados em `firmware.ino`:
- SPI CC1101: `sck_pin=14`, `miso_pin=12`, `mosi_pin=13`
- CC1101 módulo 1: `rx_pin1=4`, `tx_pin1=2`, `cs_pin1=5`
- CC1101 módulo 2: `rx_pin2=26`, `tx_pin2=25`, `cs_pin2=27`

Mapeamento aplicado em `board.h`:
- `cc1101_0`: CS=5, GDO0=4, GDO2=2
- `cc1101_1`: CS=27, GDO0=26, GDO2=25
- SPI compartilhada: SCLK=14, MISO=12, MOSI=13

## Pinagem necessária para fechar a board definition

Preencher os campos abaixo no `board.h`:

1. SPI bus 0
- `RF_SPI_BUS0_HOST`
- `RF_SPI_BUS0_MISO`
- `RF_SPI_BUS0_MOSI`
- `RF_SPI_BUS0_SCLK`

2. SPI bus 1 (se usado fisicamente separado)
- `RF_SPI_BUS1_HOST`
- `RF_SPI_BUS1_MISO`
- `RF_SPI_BUS1_MOSI`
- `RF_SPI_BUS1_SCLK`

3. CC1101 #0
- `RF_CC1101_0_BUS`
- `RF_CC1101_0_CS`
- `RF_CC1101_0_GDO0`
- `RF_CC1101_0_GDO2`

4. CC1101 #1
- `RF_CC1101_1_BUS`
- `RF_CC1101_1_CS`
- `RF_CC1101_1_GDO0`
- `RF_CC1101_1_GDO2`

5. nRF24 #0
- `RF_NRF24_0_BUS`
- `RF_NRF24_0_CSN`
- `RF_NRF24_0_CE`
- `RF_NRF24_0_IRQ`

## Observação importante

Para evitar regressão elétrica/funcional, só vamos consolidar os valores quando houver uma fonte técnica verificável (esquemático/pinout oficial do hardware ou firmware de referência com pin map explícito).
