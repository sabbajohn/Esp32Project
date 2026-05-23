#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "environment.h"

#ifdef __cplusplus
extern "C" {
#endif

// Tipo de armazenamento de chave
typedef enum {
    SubGhzKeyStorageTypeRAM,
    SubGhzKeyStorageTypeNVS,
    SubGhzKeyStorageTypeSD,
} SubGhzKeyStorageType;

// Estrutura de chave
typedef struct {
    uint8_t* data;
    size_t length;
    char name[32];
    uint32_t protocol_id;
    uint64_t timestamp;
} SubGhzKey;

// API do keystore
bool subghz_keystore_init(SubGhzKeyStorageType storage_type);
bool subghz_keystore_add_key(const SubGhzKey* key);
bool subghz_keystore_delete_key(const char* name);
SubGhzKey* subghz_keystore_find_key(const char* name);
bool subghz_keystore_load_from_file(const char* filename);
bool subghz_keystore_save_to_file(const char* filename);

// Funções de manipulação de chaves
bool subghz_keystore_encrypt_key(SubGhzKey* key, const uint8_t* password, size_t password_length);
bool subghz_keystore_decrypt_key(SubGhzKey* key, const uint8_t* password, size_t password_length);

#ifdef __cplusplus
}
#endif
