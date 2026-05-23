#include "rf_platform/radio_hal_linux.h"

#include <string.h>
#include <time.h>

static rf_capture_profile_t g_profile = {
    .frequency_hz = 433920000u,
    .channel = 0,
    .profile_id = 1,
};

static uint32_t g_run_id = 1;

static uint64_t now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ((uint64_t)ts.tv_sec * 1000000000ull) + (uint64_t)ts.tv_nsec;
}

static size_t linux_list_radios(rf_radio_desc_t* out, size_t cap) {
    static const rf_radio_desc_t radios[] = {
        {.name = "cc1101_0", .type = SUBGHZ_RADIO_CC1101, .instance_id = 0},
        {.name = "cc1101_1", .type = SUBGHZ_RADIO_CC1101, .instance_id = 1},
        {.name = "nrf24_0", .type = SUBGHZ_RADIO_NRF24, .instance_id = 0},
    };

    size_t n = sizeof(radios) / sizeof(radios[0]);
    if(out && cap > 0) {
        size_t m = (n < cap) ? n : cap;
        memcpy(out, radios, m * sizeof(radios[0]));
    }
    return n;
}

static rf_hal_status_t linux_open(uint8_t instance_id) {
    (void)instance_id;
    return RF_HAL_OK;
}

static rf_hal_status_t linux_apply_profile(uint8_t instance_id, const rf_capture_profile_t* profile) {
    (void)instance_id;
    if(!profile) {
        return RF_HAL_ERR;
    }
    g_profile = *profile;
    return RF_HAL_OK;
}

static bool emit_princeton_like(uint8_t instance_id, rf_hal_capture_callback_t cb, void* cb_ctx) {
    if(!cb) {
        return false;
    }

    subghz_input_t in;
    memset(&in, 0, sizeof(in));
    in.type = SUBGHZ_INPUT_PULSES;
    in.meta.timestamp_ns = now_ns();
    in.meta.radio_type = SUBGHZ_RADIO_CC1101;
    in.meta.radio_instance_id = instance_id;
    in.meta.profile_id = g_profile.profile_id;
    in.meta.run_id = g_run_id;
    in.meta.freq_hz = g_profile.frequency_hz;
    in.meta.channel = g_profile.channel;
    in.meta.rssi_dbm_x10 = -673;
    in.meta.lqi = 112;
    in.meta.crc_ok = false;

    static const uint16_t pulses[] = {
        360, 1080, 360, 1080, 360, 1080, 360, 1080,
        360, 1080, 360, 360, 360, 1080, 360, 360,
        360, 1080, 360, 360, 360, 1080, 360, 360,
        360, 1080, 360, 360, 360, 1080, 360, 360,
        360, 1080, 360, 1080, 360, 1080, 360, 1080,
        360, 1080, 360, 1080, 360, 1080, 360, 1080,
        360, 1080, 360, 1080, 360, 1080, 360, 1080,
        360, 1080, 360, 1080, 360, 1080, 360, 1080,
        360, 1080, 360, 1080, 360, 1080, 360, 1080,
        360, 1080, 360, 1080, 360, 1080, 360, 1080,
        360, 1080, 360, 1080, 360, 1080, 360, 1080,
        360, 1080, 360, 1080, 360, 1080, 360, 1080,
        360, 1080, 360, 1080, 360, 1080, 360, 1080,
        360, 1080, 360, 1080, 360, 1080, 360, 1080,
        360, 1080, 360, 1080, 360, 1080, 360, 1080,
        360, 12000
    };

    size_t n = sizeof(pulses) / sizeof(pulses[0]);
    in.v.pulses.start_level_high = true;
    in.v.pulses.count = (uint16_t)n;
    for(size_t i = 0; i < n; i++) {
        in.v.pulses.durations_us[i] = pulses[i];
    }

    return cb(&in, cb_ctx);
}

static bool emit_packet(uint8_t instance_id, rf_hal_capture_callback_t cb, void* cb_ctx) {
    if(!cb) {
        return false;
    }

    subghz_input_t in;
    memset(&in, 0, sizeof(in));
    in.type = SUBGHZ_INPUT_PACKET;
    in.meta.timestamp_ns = now_ns();
    in.meta.radio_type = SUBGHZ_RADIO_CC1101;
    in.meta.radio_instance_id = instance_id;
    in.meta.profile_id = g_profile.profile_id;
    in.meta.run_id = g_run_id;
    in.meta.freq_hz = g_profile.frequency_hz;
    in.meta.channel = g_profile.channel;
    in.meta.rssi_dbm_x10 = -523;
    in.meta.lqi = 174;
    in.meta.crc_ok = true;

    in.v.packet.sync_word = 0xD391u;
    in.v.packet.sync_word_len_bits = 16;
    in.v.packet.framing_hints = 1u << 2;
    in.v.packet.len = 8;
    in.v.packet.bytes[0] = 0x5A;
    in.v.packet.bytes[1] = 0xC3;
    in.v.packet.bytes[2] = 0x12;
    in.v.packet.bytes[3] = 0x9E;
    in.v.packet.bytes[4] = 0xEF;
    in.v.packet.bytes[5] = 0x00;
    in.v.packet.bytes[6] = 0x13;
    in.v.packet.bytes[7] = 0x7D;

    return cb(&in, cb_ctx);
}

static rf_hal_status_t linux_start_capture(
    uint8_t instance_id,
    rf_hal_capture_callback_t cb,
    void* cb_ctx) {
    if(!cb) {
        return RF_HAL_ERR;
    }

    if(!emit_princeton_like(instance_id, cb, cb_ctx)) {
        return RF_HAL_ERR;
    }
    if(!emit_packet(instance_id, cb, cb_ctx)) {
        return RF_HAL_ERR;
    }

    g_run_id++;
    return RF_HAL_OK;
}

static rf_hal_status_t linux_stop_capture(uint8_t instance_id) {
    (void)instance_id;
    return RF_HAL_OK;
}

static void linux_close(uint8_t instance_id) {
    (void)instance_id;
}

static const rf_radio_hal_t g_hal = {
    .impl_name = "linux_simulated",
    .list_radios = linux_list_radios,
    .open = linux_open,
    .apply_profile = linux_apply_profile,
    .start_capture = linux_start_capture,
    .stop_capture = linux_stop_capture,
    .close = linux_close,
};

const rf_radio_hal_t* rf_linux_radio_hal_get(void) {
    return &g_hal;
}

const rf_radio_hal_t* rf_platform_get_radio_hal(void) {
    return rf_linux_radio_hal_get();
}
