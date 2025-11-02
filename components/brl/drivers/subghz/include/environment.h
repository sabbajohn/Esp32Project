#pragma once

#include <stdint.h>
#include <stdbool.h>

// Tipos de modulação suportados
typedef enum {
    SubGhzModulationFSK2 = 0,
    SubGhzModulationMSK = 1,
    SubGhzModulationGFSK = 2,
    SubGhzModulation2FSK = 3,
    SubGhzModulationDSPSK = 4,
    SubGhzModulationOOK = 5,
} SubGhzModulation;

// Presets de configuração
typedef enum {
    SubGhzPresetGD00,     // Genérico para 433.92MHz
    SubGhzPresetFPV,      // Configuração para FPV
    SubGhzPresetCustom,   // Configuração personalizada
} SubGhzPreset;

// Estrutura de ambiente SubGhz
typedef struct {
    uint32_t frequency;           // Frequência em Hz
    SubGhzModulation modulation;  // Tipo de modulação
    uint32_t baudrate;           // Taxa de transmissão
    uint8_t deviation;           // Desvio de frequência
    uint8_t packet_size;         // Tamanho do pacote
    bool manchester;             // Codificação Manchester
    bool whitening;             // Whitening de dados
} SubGhzEnvironment;

// Funções de configuração
bool subghz_environment_init(SubGhzEnvironment* env);
bool subghz_environment_set_preset(SubGhzEnvironment* env, SubGhzPreset preset);
bool subghz_environment_load_custom(SubGhzEnvironment* env, const uint8_t* config, size_t size);
bool subghz_environment_check_frequency(uint32_t frequency);
