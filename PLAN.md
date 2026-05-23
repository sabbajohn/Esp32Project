# Plano e Diagnóstico do MVP RF (ESP32 + Linux)

## 1) Auditoria do `./Esp32Project`

### Mapa atual do código
- `apps/radio_monitor`: app GUI de demonstração para display + CC1101.
- `components/brl/drivers/include`: driver low-level CC1101 (`cc1101.c/.h`), `board_config.h`, bateria, etc.
- `components/brl/drivers/subghz`: stack completo herdado do Flipper (`receiver`, `worker`, `protocols`, `devices`).
- `docs/*`: documentação de arquitetura Mini-OS focada em Lilygo + GUI.
- `main/` e `src/`: entrypoints IDF/Arduino misturados.

### Como CC1101/nRF24 está hoje
- CC1101 existe em dois níveis:
  - low-level SPI (`components/brl/drivers/include/cc1101.c/.h`).
  - wrapper incompleto em `components/brl/drivers/cc1101/cc1101_wrapper.*`.
- O stack `subghz` está presente, mas fortemente acoplado ao ecossistema Flipper/Furi.
- nRF24: **não há implementação** no `Esp32Project` atual.

### Pinagem e buses (Evil Crow RF v2)
- Não foi encontrada definição de board Evil Crow RF v2 no código.
- A pinagem existente em `components/brl/drivers/include/board_config.h` é de outro hardware (Lilygo/T-Embedded style), com apenas 1 CC1101.
- Não há mapeamento de `cc1101_1` nem de `nrf24_0` nesse projeto atual.

### Reaproveitar vs refazer

Reaproveitar:
- `components/brl/drivers/include/cc1101.c/.h`: base de acesso a registradores FIFO/strobe.
- Conceitos do stack `subghz` (registry, receiver, protocol abstractions, worker por fila).
- Presets de rádio (ideia de `devices/cc1101_configs*`).

Refazer:
- Camada de board (hoje fixa, sem instâncias múltiplas de rádio).
- Pipeline RX para ISR->queue->worker sem dependência Furi.
- API de transporte/record padronizada e estável para alto volume.
- CLI headless (sem GUI) para controle e captura.
- Separação estrita em `core` puro C vs `platform`.

## 2) Estudo do `./unleashed-firmware/lib/subghz`

### Organização/registro de protocolos
- Registro central em array estático (`protocol_items.c`) -> `SubGhzProtocolRegistry`.
- Cada protocolo expõe decoder/encoder com tabela de callbacks (`alloc/free/feed/reset/...`).

### Arquitetura feed/detect
- `subghz_worker`: callback de captura assíncrona -> stream buffer -> thread que agrega pares level/duration.
- `subghz_receiver`: instancia todos os decoders e faz fan-out de `feed(level,duration)`.
- Callback por decoder em match bem-sucedido.

### Subset mínimo para portar
- Interface de protocolo por callbacks (`feed/reset` + metadados de identificação).
- Registry de protocolos estático e simples.
- Worker/pipeline com fila fixa e sem malloc no hot path.
- Separação Device Interconnect (HAL) da lógica de detecção.

## 3) Nova arquitetura portável proposta (ESP32 + Linux)

Layout implementado para MVP:
- `core/subghz_core`: detecção/identificação pura C.
- `core/record`: record binário estável + encode/decode.
- `platform/common`: interfaces públicas de HAL/board.
- `platform/esp32`: board layer Evil Crow RF v2 (template) + HAL stubs.
- `platform/linux`: HAL simulado (MVP) + base para integração spidev/gpiod.
- `firmware/cli`: CLI de controle de captura (headless).
- `tools/client`: cliente CLI PC (UART/file replay + inspeção humana).
- `profiles`: perfis runtime em CSV.

Princípio de portabilidade:
- O `core` não conhece ESP-IDF, FreeRTOS, Linux, gpiod ou serial.
- `platform/*` traduz eventos do rádio para `subghz_input_t`.
- `core` identifica e `core/record` serializa.
- Ferramentas PC leem o mesmo formato sem branch por plataforma.

## 4) MVP executável entregue

Firmware CLI (`rf_firmware_cli`):
- Comandos: `profile list`, `profile load <id>`, `start`, `stop`, `status`, `radio list`.
- Captura E2E no fluxo do MVP: `HAL -> detector -> record -> arquivo binário`.
- Multi-instância lógica (`cc1101_0`, `cc1101_1`, `nrf24_0`) no plano de controle.

Cliente PC (`rf_client`):
- Entrada via `--file` e `--uart`.
- Decodifica records binários.
- Tabela humana e `show <n>`.
- Filtros: rádio, protocolo, RSSI, frequência, tamanho, CRC.

Protocolos no MVP:
- `princeton_like` (heurístico de pulsos OOK).
- `fixed_sync_packet` (packet mode com sync).
- fallback `raw_ook`.

Observações de escopo atual:
- HAL ESP32/CC1101 e Linux spidev/gpiod ainda estão como base/stub de integração.
- Board Evil Crow RF v2 está em template por falta de pinagem validada no repositório atual.

## 5) Roadmap em commits pequenos (sugerido)

1. `chore(core): add portable subghz input/identify API`
2. `feat(record): add stable binary record encoder/decoder`
3. `feat(platform): add common HAL + board definitions`
4. `feat(platform-esp32): add evil_crow_rf_v2 board template`
5. `feat(platform-linux): add simulated capture HAL`
6. `feat(firmware-cli): add runtime profiles and capture commands`
7. `feat(tools-client): add record receiver/replay/show/filter`
8. `docs: add architecture, record spec, and MVP usage`
9. `feat(platform-linux): integrate real cc1101 spidev/gpiod`
10. `feat(platform-esp32): integrate real cc1101 ISR pipeline`
11. `feat(protocols): port additional decoders from unleashed subset`
12. `feat(storage): add SD BIN/IDX/META on ESP32`
