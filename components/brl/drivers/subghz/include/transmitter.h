#pragma once

#include "environment.h"
#include <stdint.h>
#include <stdbool.h>

// Estados do transmissor
typedef enum {
    SubGhzTxStateIdle,
    SubGhzTxStatePrepare,
    SubGhzTxStateTx,
    SubGhzTxStateComplete,
    SubGhzTxStateError,
} SubGhzTxState;

// Configuração do transmissor
typedef struct {
    uint32_t frequency;
    uint32_t power;  // em dBm
    SubGhzModulation modulation;
    bool enable_retry;
    uint8_t retry_count;
    uint32_t retry_delay_ms;
} SubGhzTxConfig;

// Dados para transmissão
typedef struct {
    const uint8_t* data;
    size_t length;
    uint32_t repeat_count;
    uint32_t repeat_delay_ms;
} SubGhzTxData;

// Callbacks do transmissor
typedef struct {
    void (*on_complete)(void* context);
    void (*on_error)(void* context);
    void (*on_progress)(uint8_t progress, void* context);
} SubGhzTxCallbacks;

// API do transmissor
bool subghz_tx_init(const SubGhzTxConfig* config);
bool subghz_tx_start(const SubGhzTxData* data, const SubGhzTxCallbacks* callbacks, void* context);
void subghz_tx_stop(void);
SubGhzTxState subghz_tx_get_state(void);
