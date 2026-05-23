#ifndef RF_RECORD_RECORD_H
#define RF_RECORD_RECORD_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "subghz_core/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RF_RECORD_MAGIC 0x52465243u
#define RF_RECORD_VERSION 1u
#define RF_RECORD_MAX_DATA 1024u

typedef enum {
    RF_REC_F_CRC_OK = 1u << 0,
    RF_REC_F_IS_RAW_PULSES = 1u << 1,
    RF_REC_F_IS_PACKET = 1u << 2,
    RF_REC_F_OVERFLOW = 1u << 3,
    RF_REC_F_TRUNCATED = 1u << 4,
} rf_record_flags_t;

typedef enum {
    RF_FRAMING_NONE = 0,
    RF_FRAMING_MANCHESTER = 1u << 0,
    RF_FRAMING_WHITENING = 1u << 1,
    RF_FRAMING_CRC_PRESENT = 1u << 2,
} rf_framing_hints_t;

typedef struct {
    uint32_t magic;
    uint8_t version;
    uint8_t header_len;
    uint16_t total_len;
    uint64_t timestamp_ns;
    uint8_t radio_type;
    uint8_t radio_instance_id;
    uint16_t profile_id;
    uint32_t run_id;
    uint32_t frequency_hz;
    uint16_t channel;
    int16_t rssi_dbm_x10;
    uint8_t lqi;
    uint16_t flags;
    uint16_t proto_id;
    uint8_t confidence;
    uint8_t preamble_type;
    uint16_t preamble_bits;
    uint32_t sync_word;
    uint8_t sync_word_bits;
    uint8_t framing_hints;
    uint16_t data_len;
    uint8_t data[RF_RECORD_MAX_DATA];
} rf_record_t;

void rf_record_init(rf_record_t* record);
bool rf_record_from_input(
    const subghz_input_t* input,
    const subghz_identification_t* ident,
    uint16_t extra_flags,
    rf_record_t* out_record);

#ifdef __cplusplus
}
#endif

#endif
