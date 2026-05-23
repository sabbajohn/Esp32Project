#include "rf_record/codec.h"

#include <string.h>

static void w16(uint8_t* p, uint16_t v) {
    p[0] = (uint8_t)(v & 0xFFu);
    p[1] = (uint8_t)((v >> 8) & 0xFFu);
}

static void w32(uint8_t* p, uint32_t v) {
    p[0] = (uint8_t)(v & 0xFFu);
    p[1] = (uint8_t)((v >> 8) & 0xFFu);
    p[2] = (uint8_t)((v >> 16) & 0xFFu);
    p[3] = (uint8_t)((v >> 24) & 0xFFu);
}

static void w64(uint8_t* p, uint64_t v) {
    for(int i = 0; i < 8; i++) {
        p[i] = (uint8_t)((v >> (8 * i)) & 0xFFu);
    }
}

static uint16_t r16(const uint8_t* p) {
    return (uint16_t)p[0] | (uint16_t)((uint16_t)p[1] << 8);
}

static uint32_t r32(const uint8_t* p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static uint64_t r64(const uint8_t* p) {
    uint64_t v = 0;
    for(int i = 0; i < 8; i++) {
        v |= ((uint64_t)p[i] << (8 * i));
    }
    return v;
}

size_t rf_record_encoded_size(const rf_record_t* rec) {
    if(!rec) {
        return 0;
    }

    return (size_t)rec->header_len + rec->data_len;
}

bool rf_record_encode(const rf_record_t* rec, uint8_t* out_buf, size_t out_cap, size_t* out_size) {
    if(!rec || !out_buf || !out_size) {
        return false;
    }

    size_t n = rf_record_encoded_size(rec);
    if(n > out_cap || rec->data_len > RF_RECORD_MAX_DATA) {
        return false;
    }

    memset(out_buf, 0, n);

    w32(&out_buf[0], rec->magic);
    out_buf[4] = rec->version;
    out_buf[5] = rec->header_len;
    w16(&out_buf[6], rec->total_len);

    w64(&out_buf[8], rec->timestamp_ns);
    out_buf[16] = rec->radio_type;
    out_buf[17] = rec->radio_instance_id;
    w16(&out_buf[18], rec->profile_id);
    w32(&out_buf[20], rec->run_id);

    w32(&out_buf[24], rec->frequency_hz);
    w16(&out_buf[28], rec->channel);
    w16(&out_buf[30], (uint16_t)rec->rssi_dbm_x10);
    out_buf[32] = rec->lqi;
    w16(&out_buf[33], rec->flags);

    w16(&out_buf[35], rec->proto_id);
    out_buf[37] = rec->confidence;
    out_buf[38] = rec->preamble_type;
    w16(&out_buf[39], rec->preamble_bits);
    w32(&out_buf[41], rec->sync_word);
    out_buf[45] = rec->sync_word_bits;
    out_buf[46] = rec->framing_hints;
    w16(&out_buf[47], rec->data_len);

    if(rec->data_len > 0u) {
        memcpy(&out_buf[49], rec->data, rec->data_len);
    }

    *out_size = (size_t)49u + rec->data_len;
    return true;
}

bool rf_record_decode(const uint8_t* buf, size_t size, rf_record_t* out_rec, size_t* out_consumed) {
    if(!buf || !out_rec || !out_consumed) {
        return false;
    }

    if(size < 49u) {
        return false;
    }

    if(r32(&buf[0]) != RF_RECORD_MAGIC) {
        return false;
    }

    uint8_t version = buf[4];
    if(version != RF_RECORD_VERSION) {
        return false;
    }

    uint16_t data_len = r16(&buf[47]);
    size_t total = 49u + data_len;
    if(total > size || data_len > RF_RECORD_MAX_DATA) {
        return false;
    }

    memset(out_rec, 0, sizeof(*out_rec));
    out_rec->magic = RF_RECORD_MAGIC;
    out_rec->version = version;
    out_rec->header_len = buf[5];
    out_rec->total_len = r16(&buf[6]);
    out_rec->timestamp_ns = r64(&buf[8]);
    out_rec->radio_type = buf[16];
    out_rec->radio_instance_id = buf[17];
    out_rec->profile_id = r16(&buf[18]);
    out_rec->run_id = r32(&buf[20]);
    out_rec->frequency_hz = r32(&buf[24]);
    out_rec->channel = r16(&buf[28]);
    out_rec->rssi_dbm_x10 = (int16_t)r16(&buf[30]);
    out_rec->lqi = buf[32];
    out_rec->flags = r16(&buf[33]);
    out_rec->proto_id = r16(&buf[35]);
    out_rec->confidence = buf[37];
    out_rec->preamble_type = buf[38];
    out_rec->preamble_bits = r16(&buf[39]);
    out_rec->sync_word = r32(&buf[41]);
    out_rec->sync_word_bits = buf[45];
    out_rec->framing_hints = buf[46];
    out_rec->data_len = data_len;
    if(data_len > 0u) {
        memcpy(out_rec->data, &buf[49], data_len);
    }

    *out_consumed = total;
    return true;
}
