# Fluxo de Protocolos (Legacy -> Pipeline Novo)

Este documento descreve como os protocolos da pasta legada (ex.: `kia.c`) entram no novo pipeline e como adicionar novos protocolos mantendo o mesmo nível de detalhe.

## 1) Fonte dos protocolos

Fonte canônica dos protocolos legados:
- `core/subghz_legacy/protocols/*.c`

Registro oficial (ordem e ativação):
- `core/subghz_legacy/protocols/protocol_items.c`
- `core/subghz_legacy/protocols/protocol_items.h`

O protocolo `KIA` está em:
- `core/subghz_legacy/protocols/kia.c`
- `core/subghz_legacy/protocols/kia.h`

## 2) Como os protocolos são usados no pipeline novo

Bridge ativa no firmware CLI:
- `core/subghz_legacy_bridge/src/bridge.c`
- `firmware/cli/mvp_firmware_cli.c`

Fluxo E2E:
1. HAL entrega `subghz_input_t` (pulses ou packet)
2. Bridge (`subghz_legacy_bridge_identify`) recebe pulses
3. Bridge alimenta `SubGhzReceiver` legado (`feed(level, duration)`)
4. Um decoder legado dispara callback ao reconhecer protocolo
5. Bridge converte para `subghz_identification_t`
6. `rf_record_from_input` monta record binário
7. `rf_record_encode` serializa
8. firmware envia stream binário (arquivo/UART)
9. cliente (`tools/client/src/rf_client.c`) decodifica e mostra nome do protocolo

## 3) IDs de protocolo no modo legacy

- IDs legacy são atribuídos por índice do registro:
- `proto_id = SUBGHZ_LEGACY_PROTO_ID_BASE + index`
- Base atual: `1000`

Isso garante estabilidade desde que a ordem em `protocol_items.c` não seja alterada sem versionamento.

## 4) Como adicionar nossos protocolos

### Opção A (imediata, mantendo biblioteca legada)
1. Adicione `<novo>.c` e `<novo>.h` em `core/subghz_legacy/protocols/`
2. Declare `extern const SubGhzProtocol subghz_protocol_<novo>;` no header
3. Inclua o header em `protocol_items.h`
4. Adicione `&subghz_protocol_<novo>` em `protocol_items.c`
5. Recompile

Esse caminho coloca o protocolo direto no fluxo novo via bridge, sem perder funcionalidades já existentes da implementação legada.

### Opção B (portável pura, longo prazo)
- Portar também para `core/subghz_core` como matcher nativo.
- Mantém compatibilidade ESP32/Linux sem camada legacy.

## 5) Compatibilidade usada para portar todos os protocolos

Para compilar os protocolos legados fora do ambiente Flipper, foi criada camada de compatibilidade em:
- `core/subghz_legacy/compat/`

Ela fornece os contratos mínimos de:
- `furi.h`, `furi_hal.h`, `furi_hal_rtc.h`
- `flipper_format`
- `storage/stream`
- `m-array` (iteradores usados pelos protocolos)
- `manchester` / `level_duration`

Importante:
- A lógica dos protocolos (`kia.c` e demais) foi mantida.
- Ajustes feitos foram de infraestrutura/compat para execução no pipeline novo.

## 6) Comando para rodar detector legacy

Firmware CLI:
- `./mvp/build/rf_firmware_cli --detector legacy --out capture.records`

Modo antigo do core simples ainda existe:
- `--detector core`

## 7) Cliente humano

Cliente:
- `./mvp/build/rf_client --file capture.records`

Saída mostra:
- timestamp
- rádio
- frequência/canal
- RSSI
- protocolo (incluindo nomes legacy)
- confiança
- tamanho
- CRC

Detalhe por item:
- `show <idx>`
