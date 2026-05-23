# RF Record Binário (v1)

## Objetivo
Formato compacto e estável para transporte/armazenamento de alta taxa entre firmware e cliente PC.

## Endianness
- Little-endian para todos os campos multibyte.

## Layout (v1)

| Offset | Tamanho | Campo |
|---|---:|---|
| 0 | 4 | `magic` = `0x52465243` |
| 4 | 1 | `version` = `1` |
| 5 | 1 | `header_len` = `49` |
| 6 | 2 | `total_len` (`49 + data_len`) |
| 8 | 8 | `timestamp_ns` |
| 16 | 1 | `radio_type` (`1=cc1101`, `2=nrf24`) |
| 17 | 1 | `radio_instance_id` |
| 18 | 2 | `profile_id` |
| 20 | 4 | `run_id` |
| 24 | 4 | `frequency_hz` |
| 28 | 2 | `channel` |
| 30 | 2 | `rssi_dbm_x10` (signed) |
| 32 | 1 | `lqi` |
| 33 | 2 | `flags` |
| 35 | 2 | `proto_id` |
| 37 | 1 | `confidence` (0..100) |
| 38 | 1 | `preamble_type` |
| 39 | 2 | `preamble_bits` |
| 41 | 4 | `sync_word` |
| 45 | 1 | `sync_word_bits` |
| 46 | 1 | `framing_hints` |
| 47 | 2 | `data_len` |
| 49 | `data_len` | `data` |

## Flags (`flags`)
- bit0: `CRC_OK`
- bit1: `IS_RAW_PULSES`
- bit2: `IS_PACKET`
- bit3: `OVERFLOW`
- bit4: `TRUNCATED`

## Framing hints (`framing_hints`)
- bit0: `MANCHESTER`
- bit1: `WHITENING`
- bit2: `CRC_PRESENT`

## `data` payload
- Quando `IS_PACKET=1`: bytes do payload recebido.
- Quando `IS_RAW_PULSES=1`: formato interno v1:
  - byte 0: start level (`0`/`1`)
  - bytes 1..2: count (u16)
  - bytes seguintes: `count` durações (u16 cada, microsegundos)

## Compatibilidade
- Validar sempre `magic` + `version`.
- Para versões futuras, manter parser tolerante a campos extras com `header_len`.
