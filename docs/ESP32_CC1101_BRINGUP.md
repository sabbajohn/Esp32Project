# ESP32 + CC1101 Bring-up (Evil Crow RF v2)

Este documento descreve o estado atual do HAL real CC1101 para o fluxo novo (`HAL -> legacy bridge -> record -> client`).

## Implementação

Arquivo principal:
- `platform/esp32/src/radio_hal_esp32.c`

O HAL agora faz:
1. Inicializa SPI a partir da board definition (`platform/esp32/boards/evil_crow_rf_v2/board.h`)
2. Configura CC1101 em RX RAW assíncrono (GDO0 como saída de dados)
3. Captura bordas no GDO0 por interrupção GPIO
4. Agrupa pulsos por frame (gap de silêncio)
5. Emite `subghz_input_t` (tipo `SUBGHZ_INPUT_PULSES`) para o pipeline

Isso conecta diretamente ao detector legacy já integrado no firmware CLI.

## Pinagem usada (Evil Crow RF v2)

- SPI: SCK=14, MISO=12, MOSI=13
- `cc1101_0`: CS=5, GDO0=4, GDO2=2
- `cc1101_1`: CS=27, GDO0=26, GDO2=25

## Parâmetros MVP do capturador RAW

- `EDGE_MIN_US = 100`
- `FRAME_GAP_US = 100000`
- `FRAME_MIN_PULSES = 30`
- fila de bordas por rádio (`EDGE_QUEUE_LEN = 1024`)

## Observações

- A implementação atual é focada em captura de pulsos OOK/ASK para alimentar protocolos legados.
- RSSI é lido via status register do CC1101 e anexado ao metadata.
- `nrf24_0` permanece fora do escopo deste passo.

## Próximos ajustes de campo recomendados

1. Ajustar `MDMCFG4/3`, `MDMCFG2`, `FREND0` por banda/ambiente de teste
2. Criar profiles de RX RAW por cenário (433/315, bandwidth, data rate)
3. Validar estabilidade de ISR em alta taxa de pulsos e dimensionar fila/task stack
4. Opcional: suportar captura simultânea `cc1101_0` + `cc1101_1`
