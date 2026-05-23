#ifndef RF_RECORD_CODEC_H
#define RF_RECORD_CODEC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "rf_record/record.h"

#ifdef __cplusplus
extern "C" {
#endif

size_t rf_record_encoded_size(const rf_record_t* rec);
bool rf_record_encode(const rf_record_t* rec, uint8_t* out_buf, size_t out_cap, size_t* out_size);
bool rf_record_decode(const uint8_t* buf, size_t size, rf_record_t* out_rec, size_t* out_consumed);

#ifdef __cplusplus
}
#endif

#endif
