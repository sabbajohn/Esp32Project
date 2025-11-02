#pragma once

#include "environment.h"
#include <stdint.h>
#include <stdbool.h>

// Estados do receptor
typedef enum {
    SubGhzRxStateIdle,
    SubGhzRxStateRx,
    SubGhzRxStateAnalyze,
    SubGhzRxStateSave,
    SubGhzRxStateTimeout,
} SubGhzRxState;

// Configuração do receptor
typedef struct {
    uint32_t frequency;
    uint32_t filter_bandwidth;
    SubGhzModulation modulation;
    bool enable_rssi;
    uint32_t rssi_threshold;
    uint32_t timeout_ms;
} SubGhzRxConfig;

// Dados recebidos
typedef struct {
    uint8_t* data;
    size_t length;
    int16_t rssi;
    uint32_t frequency_error;
    uint64_t timestamp;
} SubGhzRxData;

// Callbacks do receptor
typedef struct {
    void (*on_data)(SubGhzRxData* data, void* context);
    void (*on_error)(void* context);
    void (*on_timeout)(void* context);
} SubGhzRxCallbacks;

// API do receptor
bool subghz_rx_init(const SubGhzRxConfig* config);
bool subghz_rx_start(const SubGhzRxCallbacks* callbacks, void* context);
void subghz_rx_stop(void);
SubGhzRxState subghz_rx_get_state(void);
