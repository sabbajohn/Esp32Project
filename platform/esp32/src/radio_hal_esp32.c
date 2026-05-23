#include "rf_platform/radio_hal_esp32.h"

#include "rf_platform/board.h"

#include <string.h>

#if defined(ESP_PLATFORM)

#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"

#define TAG "rf_hal_esp32"

#define CC1101_RADIO_COUNT 2u
#define EDGE_QUEUE_LEN 1024u
#define EDGE_MIN_US 100u
#define FRAME_GAP_US 100000u
#define FRAME_MIN_PULSES 30u

typedef struct {
    uint32_t duration_us;
    bool level;
} edge_event_t;

typedef struct {
    bool opened;
    bool running;
    uint8_t instance_id;
    spi_device_handle_t spi;
    int spi_host;
    gpio_num_t gdo0_pin;
    QueueHandle_t edge_q;
    TaskHandle_t task;
    rf_capture_profile_t profile;
    rf_hal_capture_callback_t cb;
    void* cb_ctx;
    uint32_t run_seq;
    uint64_t last_edge_us;
} cc1101_state_t;

static cc1101_state_t g_cc1101[CC1101_RADIO_COUNT];
static bool g_spi_bus_init[4];
static bool g_gpio_isr_service_installed = false;

/* CC1101 commands/registers used by MVP RAW capture */
#define CC1101_READ_BIT 0x80u
#define CC1101_BURST_BIT 0x40u
#define CC1101_REG_IOCFG2 0x00u
#define CC1101_REG_IOCFG0 0x02u
#define CC1101_REG_PKTLEN 0x06u
#define CC1101_REG_PKTCTRL0 0x08u
#define CC1101_REG_CHANNR 0x0Au
#define CC1101_REG_FREQ2 0x0Du
#define CC1101_REG_FREQ1 0x0Eu
#define CC1101_REG_FREQ0 0x0Fu
#define CC1101_REG_MDMCFG4 0x10u
#define CC1101_REG_MDMCFG3 0x11u
#define CC1101_REG_MDMCFG2 0x12u
#define CC1101_REG_FREND0 0x22u
#define CC1101_REG_RSSI 0x34u
#define CC1101_STROBE_SRES 0x30u
#define CC1101_STROBE_SRX 0x34u
#define CC1101_STROBE_SIDLE 0x36u
#define CC1101_STROBE_SFRX 0x3Au

static esp_err_t cc1101_strobe(spi_device_handle_t spi, uint8_t cmd) {
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &cmd,
    };
    return spi_device_transmit(spi, &t);
}

static esp_err_t cc1101_write_reg(spi_device_handle_t spi, uint8_t reg, uint8_t value) {
    uint8_t tx[2] = {reg, value};
    spi_transaction_t t = {
        .length = 16,
        .tx_buffer = tx,
    };
    return spi_device_transmit(spi, &t);
}

static esp_err_t cc1101_read_status_reg(spi_device_handle_t spi, uint8_t reg, uint8_t* out) {
    if(!out) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t tx[2] = {(uint8_t)(reg | CC1101_READ_BIT | CC1101_BURST_BIT), 0u};
    uint8_t rx[2] = {0u, 0u};

    spi_transaction_t t = {
        .length = 16,
        .tx_buffer = tx,
        .rx_buffer = rx,
    };

    esp_err_t err = spi_device_transmit(spi, &t);
    if(err == ESP_OK) {
        *out = rx[1];
    }
    return err;
}

static void cc1101_set_frequency_regs(spi_device_handle_t spi, uint32_t frequency_hz) {
    /* Same style as ELECHOUSE setMHZ() (26 MHz crystal). */
    float mhz = (float)frequency_hz / 1000000.0f;
    uint8_t freq2 = 0;
    uint8_t freq1 = 0;
    uint8_t freq0 = 0;

    while(mhz >= 26.0f) {
        mhz -= 26.0f;
        freq2++;
    }
    while(mhz >= 0.1015625f) {
        mhz -= 0.1015625f;
        freq1++;
    }
    while(mhz >= 0.00039675f) {
        mhz -= 0.00039675f;
        freq0++;
    }

    cc1101_write_reg(spi, CC1101_REG_FREQ2, freq2);
    cc1101_write_reg(spi, CC1101_REG_FREQ1, freq1);
    cc1101_write_reg(spi, CC1101_REG_FREQ0, freq0);
}

static int16_t cc1101_rssi_to_dbm_x10(uint8_t raw) {
    int rssi = (raw >= 128u) ? ((int)raw - 256) / 2 - 74 : ((int)raw / 2) - 74;
    return (int16_t)(rssi * 10);
}

static esp_err_t cc1101_apply_raw_rx_config(cc1101_state_t* st) {
    if(!st || !st->spi) {
        return ESP_ERR_INVALID_STATE;
    }

    ESP_ERROR_CHECK_WITHOUT_ABORT(cc1101_strobe(st->spi, CC1101_STROBE_SIDLE));
    ESP_ERROR_CHECK_WITHOUT_ABORT(cc1101_strobe(st->spi, CC1101_STROBE_SRES));
    vTaskDelay(pdMS_TO_TICKS(2));

    /* Reference-aligned RAW async setup (similar to Evil Crow reference firmware). */
    ESP_ERROR_CHECK_WITHOUT_ABORT(cc1101_write_reg(st->spi, CC1101_REG_IOCFG2, 0x0D));
    ESP_ERROR_CHECK_WITHOUT_ABORT(cc1101_write_reg(st->spi, CC1101_REG_IOCFG0, 0x0D));
    ESP_ERROR_CHECK_WITHOUT_ABORT(cc1101_write_reg(st->spi, CC1101_REG_PKTLEN, 0x00));
    ESP_ERROR_CHECK_WITHOUT_ABORT(cc1101_write_reg(st->spi, CC1101_REG_PKTCTRL0, 0x32));
    ESP_ERROR_CHECK_WITHOUT_ABORT(cc1101_write_reg(st->spi, CC1101_REG_MDMCFG3, 0x93));
    ESP_ERROR_CHECK_WITHOUT_ABORT(cc1101_write_reg(st->spi, CC1101_REG_MDMCFG4, 0x07));
    ESP_ERROR_CHECK_WITHOUT_ABORT(cc1101_write_reg(st->spi, CC1101_REG_MDMCFG2, 0xB0));
    ESP_ERROR_CHECK_WITHOUT_ABORT(cc1101_write_reg(st->spi, CC1101_REG_FREND0, 0x11));
    ESP_ERROR_CHECK_WITHOUT_ABORT(cc1101_write_reg(st->spi, CC1101_REG_CHANNR, (uint8_t)st->profile.channel));
    cc1101_set_frequency_regs(st->spi, st->profile.frequency_hz);

    ESP_ERROR_CHECK_WITHOUT_ABORT(cc1101_strobe(st->spi, CC1101_STROBE_SIDLE));
    ESP_ERROR_CHECK_WITHOUT_ABORT(cc1101_strobe(st->spi, CC1101_STROBE_SFRX));
    return cc1101_strobe(st->spi, CC1101_STROBE_SRX);
}

static void IRAM_ATTR gdo0_isr(void* ctx) {
    cc1101_state_t* st = (cc1101_state_t*)ctx;
    if(!st || !st->running || !st->edge_q) {
        return;
    }

    uint64_t now = esp_timer_get_time();
    uint64_t prev = st->last_edge_us;
    st->last_edge_us = now;

    if(prev == 0u) {
        return;
    }

    uint32_t dur = (uint32_t)(now - prev);
    if(dur < EDGE_MIN_US) {
        return;
    }

    int current_level = gpio_get_level(st->gdo0_pin);
    edge_event_t ev = {
        .duration_us = dur,
        .level = current_level ? false : true,
    };

    BaseType_t hp = pdFALSE;
    xQueueSendFromISR(st->edge_q, &ev, &hp);
    if(hp == pdTRUE) {
        portYIELD_FROM_ISR();
    }
}

static void cc1101_capture_task(void* arg) {
    cc1101_state_t* st = (cc1101_state_t*)arg;
    uint16_t durations[SUBGHZ_MAX_PULSES];
    uint16_t pulse_count = 0;
    bool start_level = false;
    bool have_start = false;
    uint64_t last_event_us = 0;

    while(st->running) {
        edge_event_t ev;
        if(xQueueReceive(st->edge_q, &ev, pdMS_TO_TICKS(20)) == pdTRUE) {
            if(!have_start) {
                start_level = ev.level;
                have_start = true;
            }
            if(pulse_count < SUBGHZ_MAX_PULSES) {
                durations[pulse_count++] = (uint16_t)((ev.duration_us > 65535u) ? 65535u : ev.duration_us);
            }
            last_event_us = esp_timer_get_time();
            continue;
        }

        if(pulse_count >= FRAME_MIN_PULSES && last_event_us > 0u) {
            uint64_t now = esp_timer_get_time();
            if((now - last_event_us) >= FRAME_GAP_US) {
                subghz_input_t in;
                memset(&in, 0, sizeof(in));
                in.type = SUBGHZ_INPUT_PULSES;
                in.meta.timestamp_ns = now * 1000ull;
                in.meta.radio_type = SUBGHZ_RADIO_CC1101;
                in.meta.radio_instance_id = st->instance_id;
                in.meta.profile_id = st->profile.profile_id;
                in.meta.run_id = st->run_seq++;
                in.meta.freq_hz = st->profile.frequency_hz;
                in.meta.channel = st->profile.channel;
                in.meta.lqi = 0;
                in.meta.crc_ok = false;

                uint8_t raw_rssi = 0;
                if(cc1101_read_status_reg(st->spi, CC1101_REG_RSSI, &raw_rssi) == ESP_OK) {
                    in.meta.rssi_dbm_x10 = cc1101_rssi_to_dbm_x10(raw_rssi);
                }

                in.v.pulses.start_level_high = start_level;
                in.v.pulses.count = pulse_count;
                memcpy(in.v.pulses.durations_us, durations, pulse_count * sizeof(uint16_t));

                if(st->cb && !st->cb(&in, st->cb_ctx)) {
                    st->running = false;
                }

                pulse_count = 0;
                have_start = false;
                last_event_us = 0;
            }
        }
    }

    vTaskDelete(NULL);
}

static size_t esp32_list_radios(rf_radio_desc_t* out, size_t cap) {
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

static rf_hal_status_t esp32_open(uint8_t instance_id) {
    if(instance_id >= CC1101_RADIO_COUNT) {
        return RF_HAL_UNSUPPORTED;
    }

    const rf_board_def_t* b = rf_platform_board_get();
    if(!b) {
        return RF_HAL_ERR;
    }

    cc1101_state_t* st = &g_cc1101[instance_id];
    if(st->opened) {
        return RF_HAL_OK;
    }

    const rf_cc1101_def_t* r = &b->cc1101[instance_id];
    const rf_spi_bus_def_t* spi_def = &b->spi_buses[r->bus_index];

    st->instance_id = instance_id;
    st->spi_host = spi_def->host;
    st->gdo0_pin = (gpio_num_t)r->gdo0;
    st->profile.frequency_hz = 433920000u;
    st->profile.channel = 0;
    st->profile.profile_id = 1;
    st->run_seq = 1;

    if(st->spi_host < 0 || st->spi_host > 3) {
        return RF_HAL_ERR;
    }

    if(!g_spi_bus_init[st->spi_host]) {
        spi_bus_config_t buscfg = {
            .miso_io_num = spi_def->miso,
            .mosi_io_num = spi_def->mosi,
            .sclk_io_num = spi_def->sclk,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1,
            .max_transfer_sz = 64,
        };

        esp_err_t err = spi_bus_initialize((spi_host_device_t)st->spi_host, &buscfg, SPI_DMA_CH_AUTO);
        if(err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
            ESP_LOGE(TAG, "spi_bus_initialize failed host=%d err=%d", st->spi_host, (int)err);
            return RF_HAL_ERR;
        }
        g_spi_bus_init[st->spi_host] = true;
    }

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 4000000,
        .mode = 0,
        .spics_io_num = r->cs,
        .queue_size = 4,
        .flags = SPI_DEVICE_HALFDUPLEX,
    };

    if(spi_bus_add_device((spi_host_device_t)st->spi_host, &devcfg, &st->spi) != ESP_OK) {
        ESP_LOGE(TAG, "spi_bus_add_device failed for radio %u", instance_id);
        return RF_HAL_ERR;
    }

    gpio_config_t g = {
        .pin_bit_mask = (1ULL << r->gdo0),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE,
    };
    if(gpio_config(&g) != ESP_OK) {
        spi_bus_remove_device(st->spi);
        st->spi = NULL;
        return RF_HAL_ERR;
    }

    if(!g_gpio_isr_service_installed) {
        if(gpio_install_isr_service(ESP_INTR_FLAG_IRAM) == ESP_OK ||
           gpio_install_isr_service(ESP_INTR_FLAG_IRAM) == ESP_ERR_INVALID_STATE) {
            g_gpio_isr_service_installed = true;
        }
    }

    st->edge_q = xQueueCreate(EDGE_QUEUE_LEN, sizeof(edge_event_t));
    if(!st->edge_q) {
        spi_bus_remove_device(st->spi);
        st->spi = NULL;
        return RF_HAL_ERR;
    }

    st->opened = true;
    return RF_HAL_OK;
}

static rf_hal_status_t esp32_apply_profile(uint8_t instance_id, const rf_capture_profile_t* profile) {
    if(instance_id >= CC1101_RADIO_COUNT || !profile) {
        return RF_HAL_ERR;
    }

    cc1101_state_t* st = &g_cc1101[instance_id];
    if(!st->opened) {
        return RF_HAL_ERR;
    }

    st->profile = *profile;
    if(cc1101_apply_raw_rx_config(st) != ESP_OK) {
        return RF_HAL_ERR;
    }

    return RF_HAL_OK;
}

static rf_hal_status_t esp32_start_capture(
    uint8_t instance_id,
    rf_hal_capture_callback_t cb,
    void* cb_ctx) {
    if(instance_id >= CC1101_RADIO_COUNT || !cb) {
        return RF_HAL_ERR;
    }

    cc1101_state_t* st = &g_cc1101[instance_id];
    if(!st->opened) {
        return RF_HAL_ERR;
    }
    if(st->running) {
        return RF_HAL_OK;
    }

    st->cb = cb;
    st->cb_ctx = cb_ctx;
    st->last_edge_us = 0;

    if(cc1101_apply_raw_rx_config(st) != ESP_OK) {
        return RF_HAL_ERR;
    }

    if(gpio_isr_handler_add(st->gdo0_pin, gdo0_isr, st) != ESP_OK) {
        return RF_HAL_ERR;
    }

    st->running = true;
    if(xTaskCreate(cc1101_capture_task, "cc1101_cap", 4096, st, 12, &st->task) != pdPASS) {
        st->running = false;
        gpio_isr_handler_remove(st->gdo0_pin);
        return RF_HAL_ERR;
    }

    ESP_LOGI(TAG, "capture started on cc1101_%u freq=%u", instance_id, st->profile.frequency_hz);
    return RF_HAL_OK;
}

static rf_hal_status_t esp32_stop_capture(uint8_t instance_id) {
    if(instance_id >= CC1101_RADIO_COUNT) {
        return RF_HAL_ERR;
    }

    cc1101_state_t* st = &g_cc1101[instance_id];
    if(!st->opened) {
        return RF_HAL_ERR;
    }

    st->running = false;
    gpio_isr_handler_remove(st->gdo0_pin);
    if(st->spi) {
        ESP_ERROR_CHECK_WITHOUT_ABORT(cc1101_strobe(st->spi, CC1101_STROBE_SIDLE));
    }

    return RF_HAL_OK;
}

static void esp32_close(uint8_t instance_id) {
    if(instance_id >= CC1101_RADIO_COUNT) {
        return;
    }

    cc1101_state_t* st = &g_cc1101[instance_id];
    if(!st->opened) {
        return;
    }

    st->running = false;
    gpio_isr_handler_remove(st->gdo0_pin);

    if(st->task) {
        vTaskDelay(pdMS_TO_TICKS(5));
        st->task = NULL;
    }

    if(st->edge_q) {
        vQueueDelete(st->edge_q);
        st->edge_q = NULL;
    }

    if(st->spi) {
        spi_bus_remove_device(st->spi);
        st->spi = NULL;
    }

    memset(st, 0, sizeof(*st));
}

#else

static size_t esp32_list_radios(rf_radio_desc_t* out, size_t cap) {
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

static rf_hal_status_t esp32_open(uint8_t instance_id) {
    (void)instance_id;
    return RF_HAL_UNSUPPORTED;
}

static rf_hal_status_t esp32_apply_profile(uint8_t instance_id, const rf_capture_profile_t* profile) {
    (void)instance_id;
    (void)profile;
    return RF_HAL_UNSUPPORTED;
}

static rf_hal_status_t esp32_start_capture(
    uint8_t instance_id,
    rf_hal_capture_callback_t cb,
    void* cb_ctx) {
    (void)instance_id;
    (void)cb;
    (void)cb_ctx;
    return RF_HAL_UNSUPPORTED;
}

static rf_hal_status_t esp32_stop_capture(uint8_t instance_id) {
    (void)instance_id;
    return RF_HAL_UNSUPPORTED;
}

static void esp32_close(uint8_t instance_id) {
    (void)instance_id;
}

#endif

static const rf_radio_hal_t g_hal = {
    .impl_name = "esp32_cc1101_raw",
    .list_radios = esp32_list_radios,
    .open = esp32_open,
    .apply_profile = esp32_apply_profile,
    .start_capture = esp32_start_capture,
    .stop_capture = esp32_stop_capture,
    .close = esp32_close,
};

const rf_radio_hal_t* rf_esp32_radio_hal_get(void) {
    return &g_hal;
}

const rf_radio_hal_t* rf_platform_get_radio_hal(void) {
    return rf_esp32_radio_hal_get();
}
