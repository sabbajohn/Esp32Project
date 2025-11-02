#pragma once

#include "protocols/base.h"
#include <stdint.h>
#include <stdbool.h>

// Estruturas para análise de protocolo
typedef struct {
    uint32_t frequency;
    uint32_t bit_count;
    uint32_t te_short;
    uint32_t te_long;
    uint32_t te_delta;
    bool inverse;
    bool manchester;
} SubGhzProtocolDecoderConfig;

typedef struct {
    uint64_t timestamp;
    uint32_t length;
    bool level;
} SubGhzProtocolDecoderPulse;

typedef struct {
    SubGhzProtocolDecoderConfig config;
    SubGhzProtocolDecoderPulse last_pulse;
    uint8_t* decoded_data;
    size_t decoded_data_size;
    void* decoder_result;
} SubGhzProtocolDecoder;

// Funções de análise
bool subghz_protocol_decoder_init(SubGhzProtocolDecoder* decoder);
bool subghz_protocol_decoder_feed(SubGhzProtocolDecoder* decoder, bool level, uint32_t duration);
bool subghz_protocol_decoder_check_result(SubGhzProtocolDecoder* decoder);
void* subghz_protocol_decoder_get_result(SubGhzProtocolDecoder* decoder);

// Configurações de segurança
typedef struct {
    uint32_t min_frequency;
    uint32_t max_frequency;
    bool allow_dangerous;
    uint32_t max_power_dbm;
} SubGhzSafetyConfig;

bool subghz_is_frequency_allowed(uint32_t frequency);
bool subghz_is_power_allowed(uint32_t power_dbm);
