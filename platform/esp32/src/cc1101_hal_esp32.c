#include "rf_platform/cc1101_hal.h"

static bool e_open(uint8_t instance_id) {
    (void)instance_id;
    return true;
}

static bool e_cfg(uint8_t instance_id, const rf_cc1101_profile_t* profile) {
    (void)instance_id;
    (void)profile;
    return true;
}

static bool e_start(uint8_t instance_id) {
    (void)instance_id;
    return false;
}

static bool e_stop(uint8_t instance_id) {
    (void)instance_id;
    return true;
}

static bool e_read(uint8_t instance_id, rf_cc1101_packet_t* out_pkt) {
    (void)instance_id;
    (void)out_pkt;
    return false;
}

static bool e_rl(uint8_t instance_id, int16_t* out_rssi_x10, uint8_t* out_lqi) {
    (void)instance_id;
    if(out_rssi_x10) {
        *out_rssi_x10 = 0;
    }
    if(out_lqi) {
        *out_lqi = 0;
    }
    return false;
}

static void e_close(uint8_t instance_id) {
    (void)instance_id;
}

static const rf_cc1101_hal_t g_hal = {
    .open = e_open,
    .configure = e_cfg,
    .start_rx = e_start,
    .stop_rx = e_stop,
    .read_packet = e_read,
    .read_rssi_lqi = e_rl,
    .close = e_close,
};

const rf_cc1101_hal_t* rf_platform_get_cc1101_hal(void) {
    return &g_hal;
}
