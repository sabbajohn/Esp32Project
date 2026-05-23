#ifndef SUBGHZ_LEGACY_BRIDGE_H
#define SUBGHZ_LEGACY_BRIDGE_H

#include <stdbool.h>
#include <stdint.h>

#include "subghz_core/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SUBGHZ_LEGACY_PROTO_ID_BASE 1000u

typedef struct subghz_legacy_bridge subghz_legacy_bridge_t;

subghz_legacy_bridge_t* subghz_legacy_bridge_create(void);
void subghz_legacy_bridge_destroy(subghz_legacy_bridge_t* bridge);

bool subghz_legacy_bridge_identify(
    subghz_legacy_bridge_t* bridge,
    const subghz_input_t* input,
    subghz_identification_t* out_ident);

const char* subghz_legacy_bridge_proto_name(uint16_t proto_id);

#ifdef __cplusplus
}
#endif

#endif
