#include "rf_platform/cc1101_hal.h"

static bool l_open(uint8_t instance_id) {
    (void)instance_id;
    return true;
}

static bool l_cfg(uint8_t instance_id, const rf_cc1101_profile_t* profile) {
    (void)instance_id;
    (void)profile;
    return true;
}

static bool l_start(uint8_t instance_id) {
    (void)instance_id;
    return true;
}

static bool l_stop(uint8_t instance_id) {
    (void)instance_id;
    return true;
}

static bool l_read(uint8_t instance_id, rf_cc1101_packet_t* out_pkt) {
    (void)instance_id;
    (void)out_pkt;
    return false;
}

static bool l_rl(uint8_t instance_id, int16_t* out_rssi_x10, uint8_t* out_lqi) {
    (void)instance_id;
    if(out_rssi_x10) {
        *out_rssi_x10 = -600;
    }
    if(out_lqi) {
        *out_lqi = 100;
    }
    return true;
}

static void l_close(uint8_t instance_id) {
    (void)instance_id;
}

static const rf_cc1101_hal_t g_hal = {
    .open = l_open,
    .configure = l_cfg,
    .start_rx = l_start,
    .stop_rx = l_stop,
    .read_packet = l_read,
    .read_rssi_lqi = l_rl,
    .close = l_close,
};

const rf_cc1101_hal_t* rf_platform_get_cc1101_hal(void) {
    return &g_hal;
}
