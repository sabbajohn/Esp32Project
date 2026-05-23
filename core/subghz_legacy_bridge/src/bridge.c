#include "subghz_legacy_bridge/bridge.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "environment.h"
#include "receiver.h"
#include "subghz_protocol_registry.h"

typedef struct {
    bool matched;
    const char* protocol_name;
} bridge_match_t;

struct subghz_legacy_bridge {
    SubGhzEnvironment* environment;
    SubGhzReceiver* receiver;
    bridge_match_t match;
};

static uint16_t bridge_proto_id_from_name(const char* name) {
    if(!name) {
        return SUBGHZ_PROTO_UNKNOWN;
    }

    for(size_t i = 0; i < subghz_protocol_registry_count(&subghz_protocol_registry); i++) {
        const SubGhzProtocol* p =
            subghz_protocol_registry_get_by_index(&subghz_protocol_registry, i);
        if(p && p->name && strcmp(p->name, name) == 0) {
            return (uint16_t)(SUBGHZ_LEGACY_PROTO_ID_BASE + i);
        }
    }

    return SUBGHZ_PROTO_UNKNOWN;
}

const char* subghz_legacy_bridge_proto_name(uint16_t proto_id) {
    if(proto_id < SUBGHZ_LEGACY_PROTO_ID_BASE) {
        return NULL;
    }

    size_t index = (size_t)(proto_id - SUBGHZ_LEGACY_PROTO_ID_BASE);
    const SubGhzProtocol* p = subghz_protocol_registry_get_by_index(&subghz_protocol_registry, index);
    if(!p) {
        return NULL;
    }

    return p->name;
}

static void on_legacy_rx(
    SubGhzReceiver* receiver,
    SubGhzProtocolDecoderBase* decoder_base,
    void* context) {
    (void)receiver;
    bridge_match_t* match = (bridge_match_t*)context;

    if(match->matched || !decoder_base || !decoder_base->protocol) {
        return;
    }

    match->matched = true;
    match->protocol_name = decoder_base->protocol->name;
}

subghz_legacy_bridge_t* subghz_legacy_bridge_create(void) {
    subghz_legacy_bridge_t* bridge = calloc(1, sizeof(subghz_legacy_bridge_t));
    if(!bridge) {
        return NULL;
    }

    bridge->environment = subghz_environment_alloc();
    if(!bridge->environment) {
        free(bridge);
        return NULL;
    }

    subghz_environment_set_protocol_registry(bridge->environment, &subghz_protocol_registry);

    bridge->receiver = subghz_receiver_alloc_init(bridge->environment);
    if(!bridge->receiver) {
        subghz_environment_free(bridge->environment);
        free(bridge);
        return NULL;
    }

    subghz_receiver_set_filter(bridge->receiver, SubGhzProtocolFlag_Decodable);
    subghz_receiver_set_rx_callback(bridge->receiver, on_legacy_rx, &bridge->match);

    return bridge;
}

void subghz_legacy_bridge_destroy(subghz_legacy_bridge_t* bridge) {
    if(!bridge) {
        return;
    }

    subghz_receiver_free(bridge->receiver);
    subghz_environment_free(bridge->environment);
    free(bridge);
}

bool subghz_legacy_bridge_identify(
    subghz_legacy_bridge_t* bridge,
    const subghz_input_t* input,
    subghz_identification_t* out_ident) {
    if(!bridge || !input || !out_ident) {
        return false;
    }

    memset(out_ident, 0, sizeof(*out_ident));

    if(input->type != SUBGHZ_INPUT_PULSES) {
        out_ident->proto_id = SUBGHZ_PROTO_UNKNOWN;
        out_ident->confidence = 0;
        return true;
    }

    bridge->match.matched = false;
    bridge->match.protocol_name = NULL;
    subghz_receiver_reset(bridge->receiver);

    bool level = input->v.pulses.start_level_high;
    for(uint16_t i = 0; i < input->v.pulses.count; i++) {
        subghz_receiver_decode(bridge->receiver, level, input->v.pulses.durations_us[i]);
        level = !level;
    }

    if(bridge->match.matched) {
        out_ident->proto_id = (subghz_proto_id_t)bridge_proto_id_from_name(bridge->match.protocol_name);
        out_ident->confidence = 96;
        return true;
    }

    out_ident->proto_id = SUBGHZ_PROTO_RAW_OOK;
    out_ident->confidence = 20;
    return true;
}
