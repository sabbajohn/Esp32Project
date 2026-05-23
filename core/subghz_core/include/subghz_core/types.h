#ifndef SUBGHZ_CORE_TYPES_H
#define SUBGHZ_CORE_TYPES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SUBGHZ_MAX_PAYLOAD_BYTES 255
#define SUBGHZ_MAX_PULSES 512

typedef enum {
    SUBGHZ_INPUT_PACKET = 0,
    SUBGHZ_INPUT_PULSES = 1,
} subghz_input_type_t;

typedef enum {
    SUBGHZ_RADIO_CC1101 = 1,
    SUBGHZ_RADIO_NRF24 = 2,
} subghz_radio_type_t;

typedef enum {
    SUBGHZ_PROTO_UNKNOWN = 0,
    SUBGHZ_PROTO_PRINCETON_LIKE = 100,
    SUBGHZ_PROTO_RAW_OOK = 101,
    SUBGHZ_PROTO_FIXED_SYNC_PACKET = 102,
} subghz_proto_id_t;

typedef struct {
    uint64_t timestamp_ns;
    subghz_radio_type_t radio_type;
    uint8_t radio_instance_id;
    uint16_t profile_id;
    uint32_t run_id;
    uint32_t freq_hz;
    uint16_t channel;
    int16_t rssi_dbm_x10;
    uint8_t lqi;
    bool crc_ok;
} subghz_meta_t;

typedef struct {
    uint8_t bytes[SUBGHZ_MAX_PAYLOAD_BYTES];
    uint16_t len;
    uint8_t sync_word_len_bits;
    uint32_t sync_word;
    uint8_t framing_hints;
} subghz_packet_t;

typedef struct {
    uint16_t durations_us[SUBGHZ_MAX_PULSES];
    uint16_t count;
    bool start_level_high;
} subghz_pulses_t;

typedef struct {
    subghz_input_type_t type;
    subghz_meta_t meta;
    union {
        subghz_packet_t packet;
        subghz_pulses_t pulses;
    } v;
} subghz_input_t;

typedef enum {
    SUBGHZ_PREAMBLE_UNKNOWN = 0,
    SUBGHZ_PREAMBLE_ALT_1010 = 1,
    SUBGHZ_PREAMBLE_ZEROES = 2,
    SUBGHZ_PREAMBLE_ONES = 3,
} subghz_preamble_type_t;

typedef struct {
    subghz_proto_id_t proto_id;
    uint8_t confidence;
    subghz_preamble_type_t preamble_type;
    uint16_t preamble_bits;
    uint32_t sync_word;
    uint8_t sync_word_bits;
    uint8_t framing_hints;
} subghz_identification_t;

#ifdef __cplusplus
}
#endif

#endif
