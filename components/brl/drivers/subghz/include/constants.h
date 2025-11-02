#pragma once

// Frequências comuns (em Hz)
#define SUBGHZ_FREQUENCY_315_00 315000000
#define SUBGHZ_FREQUENCY_433_92 433920000
#define SUBGHZ_FREQUENCY_434_42 434420000
#define SUBGHZ_FREQUENCY_868_35 868350000

// Limites de frequência
#define SUBGHZ_FREQUENCY_MIN    299999999
#define SUBGHZ_FREQUENCY_MAX    928000000

// Potências de transmissão (em dBm)
#define SUBGHZ_POWER_MIN        -30
#define SUBGHZ_POWER_MAX        10
#define SUBGHZ_POWER_DEFAULT    0

// Timeouts e delays
#define SUBGHZ_TIMEOUT_RX       5000  // ms
#define SUBGHZ_TIMEOUT_TX       3000  // ms
#define SUBGHZ_RETRY_DELAY      100   // ms
#define SUBGHZ_DETECT_TIMEOUT   200   // ms

// Tamanhos de buffer
#define SUBGHZ_MAX_PACKET_SIZE  64
#define SUBGHZ_MAX_PROTOCOL_NAME_LENGTH 32
#define SUBGHZ_MAX_KEY_LENGTH   16

// Frequências consideradas perigosas (lista parcial)
static const uint32_t dangerous_frequencies[] = {
    390000000,  // Faixa de segurança
    433000000,  // Faixa de radioamador
    466000000,  // Faixa de serviços de emergência
    850000000,  // Faixa celular
    900000000   // Faixa celular
};

// Protocolos suportados
typedef enum {
    SUBGHZ_PROTOCOL_UNKNOWN = 0,
    SUBGHZ_PROTOCOL_KEELOQ,
    SUBGHZ_PROTOCOL_CAME,
    SUBGHZ_PROTOCOL_NICE_FLO,
    SUBGHZ_PROTOCOL_CHAMBERLAIN,
    SUBGHZ_PROTOCOL_LINEAR,
    SUBGHZ_PROTOCOL_PRINCETON,
    // Adicione mais conforme necessário
} SubGhzProtocolID;
