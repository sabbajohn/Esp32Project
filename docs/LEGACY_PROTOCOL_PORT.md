# Port Fiel dos Protocolos Legados (SubGhz)

## Objetivo
Preservar **todos os protocolos** e suas funcionalidades no mesmo nível de detalhe do código original, sem simplificação de lógica.

## O que foi portado
Foi criado um port espelho em:
- `core/subghz_legacy/`

Conteúdo espelhado diretamente de `components/brl/drivers/subghz`:
- `protocols/` (todos os arquivos `.c/.h`)
- `blocks/`
- `types.h`
- `receiver.c/.h`
- `registry.c/.h`
- `environment.c/.h`
- `subghz_keystore.c/.h` + `subghz_keystore_i.h`

## Garantia de fidelidade
- O port espelho mantém o código fonte legado sem alteração semântica.
- Isso preserva o comportamento exato de parsing/decoding de protocolos como `kia.c` e todos os demais.

## Sincronização contínua
Script para atualizar o espelho sempre que o legado mudar:
- `scripts/sync_subghz_legacy.sh`

Uso:
```bash
./scripts/sync_subghz_legacy.sh
```

## Como os protocolos entram no pipeline novo

### Arquitetura de duas camadas
1. `core/subghz_legacy` (compatibilidade/fidelidade)
- Fonte canônica dos protocolos legados completos.

2. `core/subghz_core` (portável)
- API estável para ESP32/Linux e integração com record/cliente.

### Bridge (adapter)
A integração operacional é feita por adapter:
- Entrada: pulsos/frames do HAL (`subghz_input_t`)
- Decodificação: protocolos legados (`subghz_legacy`)
- Saída normalizada: identificação + campos úteis -> `rf_record`

## Status atual
- Port espelho completo dos protocolos: concluído.
- Próximo passo técnico: finalizar compatibilidade de dependências Furi/Flipper para build host/ESP32 do adapter legado mantendo todas as funcionalidades.

## Regras para manter integridade funcional
- Não remover callbacks, serialização, CRCs, tabelas e parsers específicos dos protocolos.
- Evitar reescrita manual de lógica quando o código legado já implementa o comportamento.
- Toda alteração funcional em protocolo deve ser feita no legado + sync para o espelho.
