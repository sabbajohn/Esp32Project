#include "rf_record/record.h"

#include <string.h>

static uint16_t copy_pulses_as_bytes(const subghz_pulses_t* pulses, uint8_t* out, uint16_t cap) {
    if(!pulses || !out || cap < 3) {
        return 0;
    }

    /* format: [start_level][count_le16][durations_le16...] */
    uint16_t needed = (uint16_t)(3u + (uint32_t)pulses->count * 2u);
    if(needed > cap) {
        return 0;
    }

    out[0] = pulses->start_level_high ? 1u : 0u;
    out[1] = (uint8_t)(pulses->count & 0xFFu);
    out[2] = (uint8_t)((pulses->count >> 8) & 0xFFu);

    for(uint16_t i = 0; i < pulses->count; i++) {
        uint16_t d = pulses->durations_us[i];
        out[3u + i * 2u] = (uint8_t)(d & 0xFFu);
        out[4u + i * 2u] = (uint8_t)((d >> 8) & 0xFFu);
    }

    return needed;
}

void rf_record_init(rf_record_t* record) {
    if(!record) {
        return;
    }

    memset(record, 0, sizeof(*record));
    record->magic = RF_RECORD_MAGIC;
    record->version = RF_RECORD_VERSION;
    record->header_len = 49u;
    record->total_len = record->header_len;
}

bool rf_record_from_input(
    const subghz_input_t* input,
    const subghz_identification_t* ident,
    uint16_t extra_flags,
    rf_record_t* out_record) {
    if(!input || !ident || !out_record) {
        return false;
    }

    rf_record_init(out_record);

    out_record->timestamp_ns = input->meta.timestamp_ns;
    out_record->radio_type = (uint8_t)input->meta.radio_type;
    out_record->radio_instance_id = input->meta.radio_instance_id;
    out_record->profile_id = input->meta.profile_id;
    out_record->run_id = input->meta.run_id;
    out_record->frequency_hz = input->meta.freq_hz;
    out_record->channel = input->meta.channel;
    out_record->rssi_dbm_x10 = input->meta.rssi_dbm_x10;
    out_record->lqi = input->meta.lqi;

    out_record->flags = extra_flags;
    if(input->meta.crc_ok) {
        out_record->flags |= RF_REC_F_CRC_OK;
    }

    out_record->proto_id = (uint16_t)ident->proto_id;
    out_record->confidence = ident->confidence;
    out_record->preamble_type = (uint8_t)ident->preamble_type;
    out_record->preamble_bits = ident->preamble_bits;
    out_record->sync_word = ident->sync_word;
    out_record->sync_word_bits = ident->sync_word_bits;
    out_record->framing_hints = ident->framing_hints;

    if(input->type == SUBGHZ_INPUT_PACKET) {
        uint16_t to_copy = input->v.packet.len;
        if(to_copy > RF_RECORD_MAX_DATA) {
            to_copy = RF_RECORD_MAX_DATA;
            out_record->flags |= RF_REC_F_TRUNCATED;
        }

        memcpy(out_record->data, input->v.packet.bytes, to_copy);
        out_record->data_len = to_copy;
        out_record->flags |= RF_REC_F_IS_PACKET;

        if(out_record->sync_word == 0u && input->v.packet.sync_word_len_bits > 0u) {
            out_record->sync_word = input->v.packet.sync_word;
            out_record->sync_word_bits = input->v.packet.sync_word_len_bits;
        }
        if(out_record->framing_hints == 0u) {
            out_record->framing_hints = input->v.packet.framing_hints;
        }
    } else {
        uint16_t n = copy_pulses_as_bytes(&input->v.pulses, out_record->data, RF_RECORD_MAX_DATA);
        if(n == 0u) {
            return false;
        }
        out_record->data_len = n;
        out_record->flags |= RF_REC_F_IS_RAW_PULSES;
    }

    out_record->total_len = (uint16_t)(out_record->header_len + out_record->data_len);
    return true;
}
