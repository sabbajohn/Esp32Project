#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "rf_record/codec.h"
#include "subghz_legacy_bridge/bridge.h"

typedef struct {
    rf_record_t* items;
    size_t count;
    size_t cap;
} rec_vec_t;

typedef struct {
    int radio_instance;
    uint16_t proto_id;
    int rssi_min_x10;
    int rssi_max_x10;
    uint32_t freq_hz;
    int len_min;
    int len_max;
    int crc_mode; /* -1 ignore, 0 bad, 1 ok */
} filters_t;

static const char* proto_name(uint16_t id) {
    const char* legacy_name = subghz_legacy_bridge_proto_name(id);
    if(legacy_name) {
        return legacy_name;
    }

    switch(id) {
    case SUBGHZ_PROTO_PRINCETON_LIKE:
        return "princeton_like";
    case SUBGHZ_PROTO_RAW_OOK:
        return "raw_ook";
    case SUBGHZ_PROTO_FIXED_SYNC_PACKET:
        return "fixed_sync_packet";
    default:
        return "unknown";
    }
}

static const char* radio_name(uint8_t radio_type, uint8_t inst) {
    static char b[32];
    const char* rt = (radio_type == SUBGHZ_RADIO_CC1101) ? "cc1101" :
                     (radio_type == SUBGHZ_RADIO_NRF24) ? "nrf24" : "radio";
    snprintf(b, sizeof(b), "%s_%u", rt, inst);
    return b;
}

static bool vec_push(rec_vec_t* v, const rf_record_t* r) {
    if(v->count == v->cap) {
        size_t ncap = (v->cap == 0) ? 128u : v->cap * 2u;
        rf_record_t* p = realloc(v->items, ncap * sizeof(*p));
        if(!p) {
            return false;
        }
        v->items = p;
        v->cap = ncap;
    }
    v->items[v->count++] = *r;
    return true;
}

static bool read_file_all(const char* path, uint8_t** out_buf, size_t* out_sz) {
    FILE* f = fopen(path, "rb");
    if(!f) {
        return false;
    }

    if(fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return false;
    }
    long sz = ftell(f);
    if(sz < 0) {
        fclose(f);
        return false;
    }
    if(fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return false;
    }

    uint8_t* b = malloc((size_t)sz);
    if(!b) {
        fclose(f);
        return false;
    }

    if(fread(b, 1, (size_t)sz, f) != (size_t)sz) {
        free(b);
        fclose(f);
        return false;
    }

    fclose(f);
    *out_buf = b;
    *out_sz = (size_t)sz;
    return true;
}

static speed_t baud_to_termios(int baud) {
    switch(baud) {
    case 9600: return B9600;
    case 19200: return B19200;
    case 38400: return B38400;
    case 57600: return B57600;
    case 115200: return B115200;
    case 230400: return B230400;
#ifdef B460800
    case 460800: return B460800;
#endif
    default: return B115200;
    }
}

static bool read_uart_capture(const char* dev, int baud, uint8_t** out_buf, size_t* out_sz) {
    int fd = open(dev, O_RDONLY | O_NOCTTY);
    if(fd < 0) {
        return false;
    }

    struct termios tio;
    if(tcgetattr(fd, &tio) != 0) {
        close(fd);
        return false;
    }

    cfmakeraw(&tio);
    speed_t spd = baud_to_termios(baud);
    cfsetispeed(&tio, spd);
    cfsetospeed(&tio, spd);
    tio.c_cc[VMIN] = 0;
    tio.c_cc[VTIME] = 10;

    if(tcsetattr(fd, TCSANOW, &tio) != 0) {
        close(fd);
        return false;
    }

    size_t cap = 1u << 20;
    uint8_t* b = malloc(cap);
    if(!b) {
        close(fd);
        return false;
    }

    size_t used = 0;
    int idle = 0;
    while(idle < 4) {
        if(used + 1024 > cap) {
            size_t ncap = cap * 2u;
            uint8_t* nb = realloc(b, ncap);
            if(!nb) {
                free(b);
                close(fd);
                return false;
            }
            b = nb;
            cap = ncap;
        }

        ssize_t n = read(fd, b + used, 1024);
        if(n > 0) {
            used += (size_t)n;
            idle = 0;
        } else {
            idle++;
        }
    }

    close(fd);
    *out_buf = b;
    *out_sz = used;
    return true;
}

static size_t decode_records(const uint8_t* buf, size_t sz, rec_vec_t* out) {
    size_t i = 0;
    size_t ok = 0;

    while(i < sz) {
        rf_record_t rec;
        size_t consumed = 0;
        if(rf_record_decode(buf + i, sz - i, &rec, &consumed)) {
            if(!vec_push(out, &rec)) {
                break;
            }
            i += consumed;
            ok++;
        } else {
            i++;
        }
    }

    return ok;
}

static bool rec_match(const rf_record_t* r, const filters_t* f) {
    if(f->radio_instance >= 0 && (int)r->radio_instance_id != f->radio_instance) {
        return false;
    }
    if(f->proto_id > 0 && r->proto_id != f->proto_id) {
        return false;
    }
    if(r->rssi_dbm_x10 < f->rssi_min_x10 || r->rssi_dbm_x10 > f->rssi_max_x10) {
        return false;
    }
    if(f->freq_hz > 0 && r->frequency_hz != f->freq_hz) {
        return false;
    }
    if((int)r->data_len < f->len_min || (int)r->data_len > f->len_max) {
        return false;
    }

    if(f->crc_mode >= 0) {
        bool ok = (r->flags & RF_REC_F_CRC_OK) != 0;
        if(f->crc_mode == 1 && !ok) {
            return false;
        }
        if(f->crc_mode == 0 && ok) {
            return false;
        }
    }

    return true;
}

static void print_table(const rec_vec_t* v, const filters_t* f) {
    printf("idx timestamp_ns          radio      freq      rssi  proto               conf len crc\n");
    for(size_t i = 0; i < v->count; i++) {
        const rf_record_t* r = &v->items[i];
        if(!rec_match(r, f)) {
            continue;
        }

        printf("%-3zu %-20" PRIu64 " %-10s %-9u %5.1f %-18s %3u  %3u %s\n",
               i,
               r->timestamp_ns,
               radio_name(r->radio_type, r->radio_instance_id),
               r->frequency_hz,
               (double)r->rssi_dbm_x10 / 10.0,
               proto_name(r->proto_id),
               r->confidence,
               r->data_len,
               (r->flags & RF_REC_F_CRC_OK) ? "ok" : "err");
    }
}

static void print_hex(const uint8_t* p, size_t n) {
    for(size_t i = 0; i < n; i++) {
        printf("%02X", p[i]);
        if((i + 1u) < n) {
            printf(" ");
        }
    }
    printf("\n");
}

static void show_item(const rec_vec_t* v, size_t idx) {
    if(idx >= v->count) {
        printf("index out of range\n");
        return;
    }

    const rf_record_t* r = &v->items[idx];
    printf("idx: %zu\n", idx);
    printf("timestamp_ns: %" PRIu64 "\n", r->timestamp_ns);
    printf("radio: %s\n", radio_name(r->radio_type, r->radio_instance_id));
    printf("profile_id: %u run_id: %u\n", r->profile_id, r->run_id);
    printf("freq/ch: %u / %u\n", r->frequency_hz, r->channel);
    printf("rssi: %.1f dBm lqi: %u\n", (double)r->rssi_dbm_x10 / 10.0, r->lqi);
    printf("proto: %s (%u) confidence: %u\n", proto_name(r->proto_id), r->proto_id, r->confidence);
    printf("preamble: type=%u bits=%u\n", r->preamble_type, r->preamble_bits);
    printf("sync: 0x%08X (%u bits)\n", r->sync_word, r->sync_word_bits);
    printf("framing_hints: 0x%02X\n", r->framing_hints);
    printf("flags: 0x%04X\n", r->flags);
    printf("len: %u\n", r->data_len);
    printf("payload/raw: ");
    print_hex(r->data, r->data_len);

    printf("ascii: ");
    for(uint16_t i = 0; i < r->data_len; i++) {
        unsigned char c = r->data[i];
        putchar(isprint(c) ? c : '.');
    }
    putchar('\n');
}

static void filters_reset(filters_t* f) {
    f->radio_instance = -1;
    f->proto_id = 0;
    f->rssi_min_x10 = -32768;
    f->rssi_max_x10 = 32767;
    f->freq_hz = 0;
    f->len_min = 0;
    f->len_max = 65535;
    f->crc_mode = -1;
}

static void parse_filter(filters_t* f, char* expr) {
    char* tok = strtok(expr, " ");
    while(tok) {
        if(strncmp(tok, "radio=", 6) == 0) {
            const char* v = tok + 6;
            if(strcmp(v, "cc1101_0") == 0 || strcmp(v, "nrf24_0") == 0) {
                f->radio_instance = 0;
            } else if(strcmp(v, "cc1101_1") == 0) {
                f->radio_instance = 1;
            }
        } else if(strncmp(tok, "proto=", 6) == 0) {
            f->proto_id = (uint16_t)strtoul(tok + 6, 0, 10);
        } else if(strncmp(tok, "rssi_min=", 9) == 0) {
            int d = atoi(tok + 9);
            f->rssi_min_x10 = d * 10;
        } else if(strncmp(tok, "rssi_max=", 9) == 0) {
            int d = atoi(tok + 9);
            f->rssi_max_x10 = d * 10;
        } else if(strncmp(tok, "freq=", 5) == 0) {
            f->freq_hz = (uint32_t)strtoul(tok + 5, 0, 10);
        } else if(strncmp(tok, "len_min=", 8) == 0) {
            f->len_min = atoi(tok + 8);
        } else if(strncmp(tok, "len_max=", 8) == 0) {
            f->len_max = atoi(tok + 8);
        } else if(strncmp(tok, "crc=", 4) == 0) {
            const char* v = tok + 4;
            if(strcmp(v, "ok") == 0) {
                f->crc_mode = 1;
            } else if(strcmp(v, "err") == 0) {
                f->crc_mode = 0;
            } else {
                f->crc_mode = -1;
            }
        }

        tok = strtok(0, " ");
    }
}

static void usage(const char* argv0) {
    printf("usage:\n");
    printf("  %s --file <records.bin>\n", argv0);
    printf("  %s --uart <device> [--baud 115200]\n", argv0);
}

int main(int argc, char** argv) {
    const char* file = 0;
    const char* uart = 0;
    int baud = 115200;

    for(int i = 1; i < argc; i++) {
        if(strcmp(argv[i], "--file") == 0 && i + 1 < argc) {
            file = argv[++i];
        } else if(strcmp(argv[i], "--uart") == 0 && i + 1 < argc) {
            uart = argv[++i];
        } else if(strcmp(argv[i], "--baud") == 0 && i + 1 < argc) {
            baud = atoi(argv[++i]);
        }
    }

    if(!file && !uart) {
        usage(argv[0]);
        return 1;
    }

    uint8_t* buf = 0;
    size_t sz = 0;
    bool ok = false;
    if(file) {
        ok = read_file_all(file, &buf, &sz);
    } else {
        ok = read_uart_capture(uart, baud, &buf, &sz);
    }

    if(!ok) {
        fprintf(stderr, "failed to read source\n");
        return 2;
    }

    rec_vec_t v = {0};
    size_t n = decode_records(buf, sz, &v);
    free(buf);

    printf("decoded %zu records (from %zu bytes)\n", n, sz);

    filters_t f;
    filters_reset(&f);
    print_table(&v, &f);

    char line[256];
    while(true) {
        printf("client> ");
        fflush(stdout);
        if(!fgets(line, sizeof(line), stdin)) {
            break;
        }

        size_t ln = strlen(line);
        while(ln > 0 && (line[ln - 1] == '\n' || line[ln - 1] == '\r')) {
            line[--ln] = '\0';
        }
        if(ln == 0) {
            continue;
        }

        if(strcmp(line, "quit") == 0 || strcmp(line, "exit") == 0) {
            break;
        }
        if(strcmp(line, "list") == 0) {
            print_table(&v, &f);
            continue;
        }
        if(strncmp(line, "show ", 5) == 0) {
            size_t idx = (size_t)strtoul(line + 5, 0, 10);
            show_item(&v, idx);
            continue;
        }
        if(strncmp(line, "filter ", 7) == 0) {
            char expr[256];
            strncpy(expr, line + 7, sizeof(expr) - 1u);
            expr[sizeof(expr) - 1u] = '\0';
            filters_reset(&f);
            parse_filter(&f, expr);
            print_table(&v, &f);
            continue;
        }
        if(strcmp(line, "filter clear") == 0) {
            filters_reset(&f);
            print_table(&v, &f);
            continue;
        }
        if(strcmp(line, "help") == 0) {
            printf("commands: list | show <idx> | filter <expr> | filter clear | quit\n");
            printf("filter expr tokens: radio=cc1101_0|cc1101_1|nrf24_0 proto=<id> rssi_min=<dBm> rssi_max=<dBm> freq=<hz> len_min=<n> len_max=<n> crc=ok|err\n");
            continue;
        }

        printf("unknown command\n");
    }

    free(v.items);
    return 0;
}
