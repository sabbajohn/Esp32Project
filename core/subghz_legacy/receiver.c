#include "receiver.h"

#include "registry.h"

#include <stdlib.h>
#include <string.h>

typedef struct {
    SubGhzProtocolDecoderBase* base;
} SubGhzReceiverSlot;

struct SubGhzReceiver {
    SubGhzReceiverSlot* slots;
    size_t slot_count;
    size_t slot_cap;
    SubGhzProtocolFlag filter;

    SubGhzReceiverCallback callback;
    void* context;
};

static bool subghz_receiver_push_slot(SubGhzReceiver* instance, SubGhzProtocolDecoderBase* base) {
    if(instance->slot_count == instance->slot_cap) {
        size_t ncap = instance->slot_cap ? instance->slot_cap * 2u : 16u;
        SubGhzReceiverSlot* next = realloc(instance->slots, ncap * sizeof(SubGhzReceiverSlot));
        if(!next) {
            return false;
        }
        instance->slots = next;
        instance->slot_cap = ncap;
    }

    instance->slots[instance->slot_count++].base = base;
    return true;
}

SubGhzReceiver* subghz_receiver_alloc_init(SubGhzEnvironment* environment) {
    SubGhzReceiver* instance = calloc(1, sizeof(SubGhzReceiver));
    if(!instance) {
        return NULL;
    }

    const SubGhzProtocolRegistry* protocol_registry_items =
        subghz_environment_get_protocol_registry(environment);

    for(size_t i = 0; i < subghz_protocol_registry_count(protocol_registry_items); ++i) {
        const SubGhzProtocol* protocol =
            subghz_protocol_registry_get_by_index(protocol_registry_items, i);

        if(protocol && protocol->decoder && protocol->decoder->alloc) {
            SubGhzProtocolDecoderBase* base = protocol->decoder->alloc(environment);
            if(base) {
                (void)subghz_receiver_push_slot(instance, base);
            }
        }
    }

    return instance;
}

void subghz_receiver_free(SubGhzReceiver* instance) {
    furi_check(instance);

    instance->callback = NULL;
    instance->context = NULL;

    for(size_t i = 0; i < instance->slot_count; i++) {
        SubGhzProtocolDecoderBase* base = instance->slots[i].base;
        if(base && base->protocol && base->protocol->decoder && base->protocol->decoder->free) {
            base->protocol->decoder->free(base);
        }
    }

    free(instance->slots);
    free(instance);
}

void subghz_receiver_decode(SubGhzReceiver* instance, bool level, uint32_t duration) {
    furi_check(instance);

    for(size_t i = 0; i < instance->slot_count; i++) {
        SubGhzProtocolDecoderBase* base = instance->slots[i].base;
        if(base && base->protocol && base->protocol->decoder &&
           (base->protocol->flag & instance->filter) != 0) {
            base->protocol->decoder->feed(base, level, duration);
        }
    }
}

void subghz_receiver_reset(SubGhzReceiver* instance) {
    furi_check(instance);

    for(size_t i = 0; i < instance->slot_count; i++) {
        SubGhzProtocolDecoderBase* base = instance->slots[i].base;
        if(base && base->protocol && base->protocol->decoder && base->protocol->decoder->reset) {
            base->protocol->decoder->reset(base);
        }
    }
}

static void subghz_receiver_rx_callback(SubGhzProtocolDecoderBase* decoder_base, void* context) {
    SubGhzReceiver* instance = context;
    if(instance->callback) {
        instance->callback(instance, decoder_base, instance->context);
    }
}

void subghz_receiver_set_rx_callback(
    SubGhzReceiver* instance,
    SubGhzReceiverCallback callback,
    void* context) {
    furi_check(instance);

    for(size_t i = 0; i < instance->slot_count; i++) {
        subghz_protocol_decoder_base_set_decoder_callback(
            instance->slots[i].base, subghz_receiver_rx_callback, instance);
    }

    instance->callback = callback;
    instance->context = context;
}

void subghz_receiver_set_filter(SubGhzReceiver* instance, SubGhzProtocolFlag filter) {
    furi_check(instance);
    instance->filter = filter;
}

SubGhzProtocolDecoderBase* subghz_receiver_search_decoder_base_by_name(
    SubGhzReceiver* instance,
    const char* decoder_name) {
    furi_check(instance);

    for(size_t i = 0; i < instance->slot_count; i++) {
        SubGhzProtocolDecoderBase* base = instance->slots[i].base;
        if(base && base->protocol && strcmp(base->protocol->name, decoder_name) == 0) {
            return base;
        }
    }

    return NULL;
}
