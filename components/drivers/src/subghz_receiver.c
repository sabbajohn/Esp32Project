#include "receiver.h"

#include <string.h>

#include "constants.h"
#include "rf_platform/radio_hal.h"

typedef struct {
    bool initialized;
    bool running;
    SubGhzRxState state;
    SubGhzRxConfig cfg;
    SubGhzRxCallbacks callbacks;
    void* cb_ctx;
    const rf_radio_hal_t* hal;
    uint8_t radio_instance;
} subghz_rx_runtime_t;

static subghz_rx_runtime_t g_rx = {
    .initialized = false,
    .running = false,
    .state = SubGhzRxStateIdle,
    .radio_instance = 0,
};

static bool subghz_rx_emit_input(const subghz_input_t* input, void* context) {
    subghz_rx_runtime_t* rx = (subghz_rx_runtime_t*)context;
    if(!rx || !input) {
        return false;
    }

    if(!rx->callbacks.on_data) {
        return true;
    }

    SubGhzRxData out = {0};
    uint8_t pulses_as_bytes[SUBGHZ_MAX_PACKET_SIZE] = {0};

    out.rssi = (int16_t)(input->meta.rssi_dbm_x10 / 10);
    out.frequency_error = 0;
    out.timestamp = input->meta.timestamp_ns / 1000ull;

    if(input->type == SUBGHZ_INPUT_PACKET) {
        out.data = (uint8_t*)input->v.packet.bytes;
        out.length = input->v.packet.len;
        rx->callbacks.on_data(&out, rx->cb_ctx);
        return true;
    }

    if(input->type == SUBGHZ_INPUT_PULSES) {
        size_t max_pulses = SUBGHZ_MAX_PACKET_SIZE / 2u;
        size_t pulses = input->v.pulses.count;
        if(pulses > max_pulses) {
            pulses = max_pulses;
        }

        for(size_t i = 0; i < pulses; i++) {
            uint16_t duration = input->v.pulses.durations_us[i];
            pulses_as_bytes[(2u * i) + 0u] = (uint8_t)(duration & 0xFFu);
            pulses_as_bytes[(2u * i) + 1u] = (uint8_t)((duration >> 8) & 0xFFu);
        }

        out.data = pulses_as_bytes;
        out.length = pulses * 2u;
        rx->callbacks.on_data(&out, rx->cb_ctx);
        return true;
    }

    return true;
}

static bool subghz_rx_apply_profile(subghz_rx_runtime_t* rx) {
    if(!rx || !rx->hal || !rx->hal->apply_profile) {
        return false;
    }

    rf_capture_profile_t profile = {
        .frequency_hz = rx->cfg.frequency,
        .channel = 0,
        .profile_id = 1,
    };

    return rx->hal->apply_profile(rx->radio_instance, &profile) == RF_HAL_OK;
}

bool subghz_rx_init(const SubGhzRxConfig* config) {
    if(!config) {
        return false;
    }
    if(!subghz_environment_check_frequency(config->frequency)) {
        return false;
    }

    memset(&g_rx, 0, sizeof(g_rx));
    g_rx.cfg = *config;
    g_rx.state = SubGhzRxStateIdle;
    g_rx.radio_instance = 0;
    g_rx.hal = rf_platform_get_radio_hal();

    if(!g_rx.hal || !g_rx.hal->open || !g_rx.hal->apply_profile || !g_rx.hal->start_capture ||
       !g_rx.hal->stop_capture || !g_rx.hal->close) {
        return false;
    }

    if(g_rx.hal->open(g_rx.radio_instance) != RF_HAL_OK) {
        return false;
    }

    if(!subghz_rx_apply_profile(&g_rx)) {
        g_rx.hal->close(g_rx.radio_instance);
        return false;
    }

    g_rx.initialized = true;
    return true;
}

bool subghz_rx_start(const SubGhzRxCallbacks* callbacks, void* context) {
    if(!g_rx.initialized || !callbacks || !callbacks->on_data) {
        return false;
    }

    g_rx.callbacks = *callbacks;
    g_rx.cb_ctx = context;

    if(g_rx.running) {
        return true;
    }

    if(!subghz_rx_apply_profile(&g_rx)) {
        g_rx.state = SubGhzRxStateTimeout;
        if(g_rx.callbacks.on_error) {
            g_rx.callbacks.on_error(g_rx.cb_ctx);
        }
        return false;
    }

    if(g_rx.hal->start_capture(g_rx.radio_instance, subghz_rx_emit_input, &g_rx) != RF_HAL_OK) {
        g_rx.state = SubGhzRxStateTimeout;
        if(g_rx.callbacks.on_error) {
            g_rx.callbacks.on_error(g_rx.cb_ctx);
        }
        return false;
    }

    g_rx.running = true;
    g_rx.state = SubGhzRxStateRx;
    return true;
}

void subghz_rx_stop(void) {
    if(!g_rx.initialized) {
        return;
    }

    if(g_rx.running && g_rx.hal && g_rx.hal->stop_capture) {
        (void)g_rx.hal->stop_capture(g_rx.radio_instance);
    }

    if(g_rx.hal && g_rx.hal->close) {
        g_rx.hal->close(g_rx.radio_instance);
    }

    g_rx.running = false;
    g_rx.initialized = false;
    g_rx.state = SubGhzRxStateIdle;
}

SubGhzRxState subghz_rx_get_state(void) {
    return g_rx.state;
}
