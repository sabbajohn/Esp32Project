#include "subghz_core/detector.h"

#include <stdlib.h>
#include <string.h>

struct subghz_detector {
    uint32_t reserved;
};

static bool detect_princeton_like(const subghz_pulses_t* pulses, subghz_identification_t* out) {
    if(!pulses || pulses->count < 48) {
        return false;
    }

    uint32_t short_sum = 0;
    uint32_t long_sum = 0;
    uint32_t short_count = 0;
    uint32_t long_count = 0;

    for(uint16_t i = 0; i < pulses->count; i++) {
        uint16_t d = pulses->durations_us[i];
        if(d < 700) {
            short_sum += d;
            short_count++;
        } else if(d < 2500) {
            long_sum += d;
            long_count++;
        }
    }

    if(short_count < 24 || long_count < 10) {
        return false;
    }

    uint32_t short_avg = short_sum / short_count;
    uint32_t long_avg = long_sum / long_count;
    if(short_avg == 0 || long_avg < (short_avg * 2U) || long_avg > (short_avg * 6U)) {
        return false;
    }

    uint32_t sync_like = 0;
    for(uint16_t i = 0; i < pulses->count; i++) {
        if(pulses->durations_us[i] > (uint16_t)(long_avg * 4U)) {
            sync_like++;
        }
    }

    if(sync_like == 0) {
        return false;
    }

    memset(out, 0, sizeof(*out));
    out->proto_id = SUBGHZ_PROTO_PRINCETON_LIKE;
    out->confidence = 78;
    out->preamble_type = SUBGHZ_PREAMBLE_ALT_1010;
    out->preamble_bits = 12;
    return true;
}

static bool detect_fixed_sync_packet(const subghz_packet_t* packet, subghz_identification_t* out) {
    if(!packet || packet->len < 4) {
        return false;
    }

    if(packet->sync_word_len_bits >= 8 && packet->sync_word != 0) {
        memset(out, 0, sizeof(*out));
        out->proto_id = SUBGHZ_PROTO_FIXED_SYNC_PACKET;
        out->confidence = 84;
        out->sync_word = packet->sync_word;
        out->sync_word_bits = packet->sync_word_len_bits;
        out->framing_hints = packet->framing_hints;
        return true;
    }

    return false;
}

subghz_detector_t* subghz_detector_create(void) {
    return (subghz_detector_t*)calloc(1, sizeof(subghz_detector_t));
}

void subghz_detector_destroy(subghz_detector_t* detector) {
    free(detector);
}

bool subghz_detector_identify(
    subghz_detector_t* detector,
    const subghz_input_t* input,
    subghz_identification_t* out_ident) {
    (void)detector;

    if(!input || !out_ident) {
        return false;
    }

    memset(out_ident, 0, sizeof(*out_ident));

    if(input->type == SUBGHZ_INPUT_PULSES) {
        if(detect_princeton_like(&input->v.pulses, out_ident)) {
            return true;
        }

        out_ident->proto_id = SUBGHZ_PROTO_RAW_OOK;
        out_ident->confidence = 35;
        return true;
    }

    if(input->type == SUBGHZ_INPUT_PACKET) {
        if(detect_fixed_sync_packet(&input->v.packet, out_ident)) {
            return true;
        }

        out_ident->proto_id = SUBGHZ_PROTO_UNKNOWN;
        out_ident->confidence = 0;
        return true;
    }

    return false;
}
