#include "cc1101_wrapper.h"
#include "esp_log.h"
#include "components/brl/drivers/include/cc1101.h"

static const char *TAG = "cc1101_wrap";

int cc1101_wrapper_attach(const FuriHalSpiBusHandle* handle) {
    if (!handle) return -1;
    // Basic sanity: get part number/version
    uint8_t part = cc1101_get_partnumber(handle);
    uint8_t ver = cc1101_get_version(handle);
    ESP_LOGI(TAG, "CC1101 attached, part=%02x ver=%02x", part, ver);
    return 0;
}

uint32_t cc1101_wrapper_set_frequency(const FuriHalSpiBusHandle* handle, uint32_t freq_hz) {
    return cc1101_set_frequency(handle, freq_hz);
}

int cc1101_wrapper_write_fifo(const FuriHalSpiBusHandle* handle, const uint8_t* data, uint8_t size) {
    if (!handle) return -1;
    uint8_t written = cc1101_write_fifo(handle, data, size);
    return (int)written;
}

int cc1101_wrapper_start_receive(const FuriHalSpiBusHandle* handle) {
    if (!handle) return -1;
    cc1101_flush_rx(handle);
    cc1101_switch_to_rx(handle);
    return 0;
}
