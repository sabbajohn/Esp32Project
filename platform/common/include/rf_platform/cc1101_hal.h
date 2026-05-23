#ifndef RF_PLATFORM_CC1101_HAL_H
#define RF_PLATFORM_CC1101_HAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t frequency_hz;
    uint8_t channel;
    uint8_t sync_word_len_bits;
    uint32_t sync_word;
    bool packet_mode;
} rf_cc1101_profile_t;

typedef struct {
    int16_t rssi_dbm_x10;
    uint8_t lqi;
    bool crc_ok;
    uint8_t payload[255];
    uint8_t len;
} rf_cc1101_packet_t;

typedef struct rf_cc1101_hal {
    bool (*open)(uint8_t instance_id);
    bool (*configure)(uint8_t instance_id, const rf_cc1101_profile_t* profile);
    bool (*start_rx)(uint8_t instance_id);
    bool (*stop_rx)(uint8_t instance_id);
    bool (*read_packet)(uint8_t instance_id, rf_cc1101_packet_t* out_pkt);
    bool (*read_rssi_lqi)(uint8_t instance_id, int16_t* out_rssi_x10, uint8_t* out_lqi);
    void (*close)(uint8_t instance_id);
} rf_cc1101_hal_t;

const rf_cc1101_hal_t* rf_platform_get_cc1101_hal(void);

#ifdef __cplusplus
}
#endif

#endif
