#include "rf/profile_store.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void rf_profile_store_init_defaults(rf_profile_store_t* store) {
    if(!store) {
        return;
    }

    memset(store, 0, sizeof(*store));
    store->count = 2;

    store->items[0].id = 1;
    strncpy(store->items[0].name, "ook_433_default", sizeof(store->items[0].name) - 1u);
    store->items[0].cfg.frequency_hz = 433920000u;
    store->items[0].cfg.channel = 0;
    store->items[0].cfg.profile_id = 1;

    store->items[1].id = 2;
    strncpy(store->items[1].name, "ook_315_default", sizeof(store->items[1].name) - 1u);
    store->items[1].cfg.frequency_hz = 315000000u;
    store->items[1].cfg.channel = 0;
    store->items[1].cfg.profile_id = 2;

    store->active_index = 0;
}

static bool parse_line(char* line, rf_profile_t* out) {
    char* tok = strtok(line, ",");
    if(!tok) {
        return false;
    }
    out->id = (uint16_t)strtoul(tok, 0, 10);

    tok = strtok(0, ",");
    if(!tok) {
        return false;
    }
    strncpy(out->name, tok, sizeof(out->name) - 1u);

    tok = strtok(0, ",");
    if(!tok) {
        return false;
    }
    out->cfg.frequency_hz = (uint32_t)strtoul(tok, 0, 10);

    tok = strtok(0, ",");
    if(!tok) {
        return false;
    }
    out->cfg.channel = (uint16_t)strtoul(tok, 0, 10);
    out->cfg.profile_id = out->id;

    return true;
}

bool rf_profile_store_load_csv(rf_profile_store_t* store, const char* path) {
    if(!store || !path) {
        return false;
    }

    FILE* f = fopen(path, "r");
    if(!f) {
        return false;
    }

    rf_profile_store_t tmp;
    memset(&tmp, 0, sizeof(tmp));

    char line[256];
    while(fgets(line, sizeof(line), f)) {
        if(line[0] == '#') {
            continue;
        }

        size_t n = strlen(line);
        while(n > 0u && (line[n - 1u] == '\n' || line[n - 1u] == '\r')) {
            line[--n] = '\0';
        }
        if(n == 0u) {
            continue;
        }

        if(tmp.count >= RF_MAX_PROFILES) {
            break;
        }

        rf_profile_t p;
        memset(&p, 0, sizeof(p));
        if(parse_line(line, &p)) {
            tmp.items[tmp.count++] = p;
        }
    }

    fclose(f);

    if(tmp.count == 0u) {
        return false;
    }

    tmp.active_index = 0;
    *store = tmp;
    return true;
}

const rf_profile_t* rf_profile_store_get_active(const rf_profile_store_t* store) {
    if(!store || store->active_index < 0 || (size_t)store->active_index >= store->count) {
        return 0;
    }
    return &store->items[store->active_index];
}

const rf_profile_t* rf_profile_store_find_by_id(const rf_profile_store_t* store, uint16_t id) {
    if(!store) {
        return 0;
    }

    for(size_t i = 0; i < store->count; i++) {
        if(store->items[i].id == id) {
            return &store->items[i];
        }
    }

    return 0;
}

bool rf_profile_store_set_active(rf_profile_store_t* store, uint16_t id) {
    if(!store) {
        return false;
    }

    for(size_t i = 0; i < store->count; i++) {
        if(store->items[i].id == id) {
            store->active_index = (int)i;
            return true;
        }
    }

    return false;
}
