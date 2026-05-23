#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rf/profile_store.h"
#include "rf_platform/radio_hal.h"
#include "rf_record/codec.h"
#include "subghz_legacy_bridge/bridge.h"
#include "subghz_core/detector.h"

#define MAX_LINE 256
#define MAX_RECORD_BUF (49u + RF_RECORD_MAX_DATA)

typedef struct {
    FILE* out;
    subghz_detector_t* detector;
    subghz_legacy_bridge_t* legacy_bridge;
    bool use_legacy;
    size_t records_written;
} pipeline_ctx_t;

static bool on_capture_input(const subghz_input_t* input, void* ctx) {
    pipeline_ctx_t* p = (pipeline_ctx_t*)ctx;
    subghz_identification_t ident;
    rf_record_t rec;
    uint8_t buf[MAX_RECORD_BUF];
    size_t out_sz = 0;

    if(p->use_legacy) {
        if(!subghz_legacy_bridge_identify(p->legacy_bridge, input, &ident)) {
            return false;
        }
    } else {
        if(!subghz_detector_identify(p->detector, input, &ident)) {
            return false;
        }
    }
    if(!rf_record_from_input(input, &ident, 0, &rec)) {
        return false;
    }
    if(!rf_record_encode(&rec, buf, sizeof(buf), &out_sz)) {
        return false;
    }

    if(fwrite(buf, 1, out_sz, p->out) != out_sz) {
        return false;
    }
    fflush(p->out);
    p->records_written++;
    return true;
}

static void print_help(void) {
    printf("commands:\n");
    printf("  profile list\n");
    printf("  profile load <id>\n");
    printf("  radio list\n");
    printf("  start [run_name] [radio=cc1101_0|cc1101_1|nrf24_0|all]\n");
    printf("  stop\n");
    printf("  status\n");
    printf("  help\n");
    printf("  quit\n");
}

static int radio_id_from_name(const char* name) {
    if(strcmp(name, "cc1101_0") == 0) return 0;
    if(strcmp(name, "cc1101_1") == 0) return 1;
    if(strcmp(name, "nrf24_0") == 0) return 0;
    return -1;
}

int main(int argc, char** argv) {
    const char* out_path = "capture.records";
    const char* profile_path = "profiles/default_profiles.csv";
    const char* detector_mode = "legacy";

    for(int i = 1; i < argc; i++) {
        if(strcmp(argv[i], "--out") == 0 && (i + 1) < argc) {
            out_path = argv[++i];
        } else if(strcmp(argv[i], "--profiles") == 0 && (i + 1) < argc) {
            profile_path = argv[++i];
        } else if(strcmp(argv[i], "--detector") == 0 && (i + 1) < argc) {
            detector_mode = argv[++i];
        }
    }

    FILE* out = fopen(out_path, "ab");
    if(!out) {
        fprintf(stderr, "failed to open output: %s\n", out_path);
        return 1;
    }

    rf_profile_store_t store;
    rf_profile_store_init_defaults(&store);
    (void)rf_profile_store_load_csv(&store, profile_path);

    subghz_detector_t* detector = subghz_detector_create();
    if(!detector) {
        fprintf(stderr, "failed to init detector\n");
        fclose(out);
        return 1;
    }
    subghz_legacy_bridge_t* legacy_bridge = subghz_legacy_bridge_create();
    if(!legacy_bridge) {
        fprintf(stderr, "failed to init legacy bridge\n");
        subghz_detector_destroy(detector);
        fclose(out);
        return 1;
    }

    pipeline_ctx_t pipe = {
        .out = out,
        .detector = detector,
        .legacy_bridge = legacy_bridge,
        .use_legacy = (strcmp(detector_mode, "legacy") == 0),
        .records_written = 0,
    };

    const rf_radio_hal_t* hal = rf_platform_get_radio_hal();
    if(!hal) {
        fprintf(stderr, "radio HAL unavailable\n");
        subghz_detector_destroy(detector);
        fclose(out);
        return 1;
    }

    bool running = false;
    int current_radio = 0;

    printf("rf firmware cli (%s) detector=%s\n", hal->impl_name, pipe.use_legacy ? "legacy" : "core");
    print_help();

    char line[MAX_LINE];
    while(true) {
        printf("> ");
        fflush(stdout);
        if(!fgets(line, sizeof(line), stdin)) {
            break;
        }

        size_t n = strlen(line);
        while(n > 0u && (line[n - 1u] == '\n' || line[n - 1u] == '\r')) {
            line[--n] = '\0';
        }
        if(n == 0u) {
            continue;
        }

        if(strcmp(line, "quit") == 0) {
            break;
        }
        if(strcmp(line, "help") == 0) {
            print_help();
            continue;
        }
        if(strcmp(line, "profile list") == 0) {
            for(size_t i = 0; i < store.count; i++) {
                const rf_profile_t* p = &store.items[i];
                printf("id=%u name=%s freq=%u ch=%u%s\n",
                       p->id,
                       p->name,
                       p->cfg.frequency_hz,
                       p->cfg.channel,
                       ((int)i == store.active_index) ? " *" : "");
            }
            continue;
        }
        if(strncmp(line, "profile load ", 13) == 0) {
            uint16_t id = (uint16_t)strtoul(line + 13, 0, 10);
            if(rf_profile_store_set_active(&store, id)) {
                printf("ok profile=%u\n", id);
            } else {
                printf("err profile not found\n");
            }
            continue;
        }
        if(strcmp(line, "radio list") == 0) {
            rf_radio_desc_t radios[8];
            size_t total = hal->list_radios(radios, 8);
            size_t shown = (total < 8u) ? total : 8u;
            for(size_t i = 0; i < shown; i++) {
                printf("%s (type=%u id=%u)\n", radios[i].name, radios[i].type, radios[i].instance_id);
            }
            continue;
        }
        if(strncmp(line, "start", 5) == 0) {
            const rf_profile_t* active = rf_profile_store_get_active(&store);
            if(!active) {
                printf("err no active profile\n");
                continue;
            }

            const char* radio_name = "cc1101_0";
            char* tag = strstr(line, "radio=");
            if(tag) {
                radio_name = tag + 6;
            }

            if(strcmp(radio_name, "all") == 0) {
                printf("mode all: emitting from cc1101_0 and cc1101_1\n");
                for(int rid = 0; rid <= 1; rid++) {
                    (void)hal->open((uint8_t)rid);
                    (void)hal->apply_profile((uint8_t)rid, &active->cfg);
                    (void)hal->start_capture((uint8_t)rid, on_capture_input, &pipe);
                }
                running = true;
                continue;
            }

            int rid = radio_id_from_name(radio_name);
            if(rid < 0) {
                printf("err invalid radio\n");
                continue;
            }

            if(hal->open((uint8_t)rid) != RF_HAL_OK) {
                printf("err open radio\n");
                continue;
            }
            if(hal->apply_profile((uint8_t)rid, &active->cfg) != RF_HAL_OK) {
                printf("err apply profile\n");
                continue;
            }
            if(hal->start_capture((uint8_t)rid, on_capture_input, &pipe) != RF_HAL_OK) {
                printf("err start capture\n");
                continue;
            }

            running = true;
            current_radio = rid;
            printf("capture done run_id auto, records_total=%zu\n", pipe.records_written);
            continue;
        }
        if(strcmp(line, "stop") == 0) {
            if(running) {
                (void)hal->stop_capture((uint8_t)current_radio);
                hal->close((uint8_t)current_radio);
                running = false;
            }
            printf("ok\n");
            continue;
        }
        if(strcmp(line, "status") == 0) {
            const rf_profile_t* p = rf_profile_store_get_active(&store);
            printf("running=%s profile=%u records=%zu out=%s\n",
                   running ? "yes" : "no",
                   p ? p->id : 0u,
                   pipe.records_written,
                   out_path);
            continue;
        }

        printf("err unknown command\n");
    }

    subghz_detector_destroy(detector);
    subghz_legacy_bridge_destroy(legacy_bridge);
    fclose(out);
    return 0;
}
