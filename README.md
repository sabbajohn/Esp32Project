# RF Capture MVP (ESP32/Linux portátil)

Este repositório contém um MVP em C para captura/identificação RF com arquitetura portável.

## Estrutura principal
- `core/subghz_core`: detector portável (modo core).
- `core/subghz_legacy`: protocolos legados completos (inclui `kia.c` e demais).
- `core/subghz_legacy_bridge`: bridge para usar protocolos legados no pipeline novo.
- `core/record`: record binário estável.
- `platform/common`: interfaces HAL/board.
- `platform/esp32`: board layer Evil Crow RF v2.
- `platform/linux`: HAL simulado para validação local.
- `firmware/cli`: CLI de firmware (headless).
- `tools/client`: cliente CLI para leitura/replay de records.

## Documentação
- `docs/PROTOCOL_INTEGRATION_FLOW.md`
- `docs/RECORD_SPEC.md`
- `docs/BOARD_EVIL_CROW_RF_V2.md`
- `docs/LEGACY_PROTOCOL_PORT.md`
- `docs/PROTOCOL_CATALOG.md`
- `docs/ESP32_CC1101_BRINGUP.md`

## Build (host)
```bash
cmake -S mvp -B mvp/build
cmake --build mvp/build -j4
```

## Captura (simulada)
```bash
./mvp/build/rf_firmware_cli --detector legacy --out /tmp/capture.records --profiles profiles/default_profiles.csv
```

- `--detector legacy`: usa bridge + protocolos de `core/subghz_legacy/protocols`
- `--detector core`: usa detector simples de `core/subghz_core`

## Cliente
```bash
./mvp/build/rf_client --file /tmp/capture.records
```

## Estado atual do MVP
- Pipeline E2E funcional.
- Bridge legacy integrada ao pipeline novo.
- Cliente resolve nome de protocolo legacy por `proto_id`.
