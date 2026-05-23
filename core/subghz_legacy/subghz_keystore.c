#include "subghz_keystore.h"
#include "subghz_keystore_i.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAG "SubGhzKeystore"

static bool subghz_key_array_reserve(SubGhzKeyArray_t* arr, size_t cap) {
    if(!arr) return false;
    if(cap <= arr->cap) return true;
    size_t ncap = arr->cap ? arr->cap : 16u;
    while(ncap < cap) ncap *= 2u;
    SubGhzKey* next = realloc(arr->items, ncap * sizeof(SubGhzKey));
    if(!next) return false;
    arr->items = next;
    arr->cap = ncap;
    return true;
}

static bool subghz_keystore_add_key(
    SubGhzKeystore* instance,
    const char* name,
    uint64_t key,
    uint16_t type) {
    if(!subghz_key_array_reserve(&instance->data, instance->data.size + 1u)) {
        return false;
    }

    SubGhzKey* dst = &instance->data.items[instance->data.size++];
    dst->name = furi_string_alloc_set(name ? name : "");
    dst->key = key;
    dst->type = type;
    return true;
}

SubGhzKeystore* subghz_keystore_alloc(void) {
    SubGhzKeystore* instance = calloc(1, sizeof(SubGhzKeystore));
    if(!instance) return NULL;
    subghz_keystore_reset_kl(instance);
    return instance;
}

void subghz_keystore_reset_kl(SubGhzKeystore* instance) {
    furi_assert(instance);
    instance->mfname = "";
    instance->kl_type = 0;
}

void subghz_keystore_free(SubGhzKeystore* instance) {
    furi_assert(instance);
    for(size_t i = 0; i < instance->data.size; i++) {
        furi_string_free(instance->data.items[i].name);
        instance->data.items[i].name = NULL;
    }
    free(instance->data.items);
    free(instance);
}

bool subghz_keystore_load(SubGhzKeystore* instance, const char* file_name) {
    furi_assert(instance);
    if(!file_name || !file_name[0]) return false;

    FILE* f = fopen(file_name, "r");
    if(!f) {
        FURI_LOG_W(TAG, "Keystore file not found: %s", file_name);
        return false;
    }

    char line[256];
    while(fgets(line, sizeof(line), f)) {
        char skey[17] = {0};
        unsigned int type = 0;
        char name[129] = {0};

        int ret = sscanf(line, "%16[^:]:%u:%128s", skey, &type, name);
        if(ret == 3) {
            uint64_t key = strtoull(skey, NULL, 16);
            (void)subghz_keystore_add_key(instance, name, key, (uint16_t)type);
        }
    }

    fclose(f);
    return instance->data.size > 0;
}

bool subghz_keystore_save(SubGhzKeystore* instance, const char* file_name, uint8_t* iv) {
    furi_assert(instance);
    UNUSED(iv);
    if(!file_name || !file_name[0]) return false;

    FILE* f = fopen(file_name, "w");
    if(!f) return false;

    for(size_t i = 0; i < instance->data.size; i++) {
        const SubGhzKey* k = &instance->data.items[i];
        fprintf(f, "%016llX:%u:%s\n", (unsigned long long)k->key, k->type, furi_string_get_cstr(k->name));
    }

    fclose(f);
    return true;
}

SubGhzKeyArray_t* subghz_keystore_get_data(SubGhzKeystore* instance) {
    furi_assert(instance);
    return &instance->data;
}

bool subghz_keystore_raw_encrypted_save(
    const char* input_file_name,
    const char* output_file_name,
    uint8_t* iv) {
    UNUSED(input_file_name);
    UNUSED(output_file_name);
    UNUSED(iv);
    return false;
}

bool subghz_keystore_raw_get_data(const char* file_name, size_t offset, uint8_t* data, size_t len) {
    UNUSED(file_name);
    UNUSED(offset);
    UNUSED(data);
    UNUSED(len);
    return false;
}
