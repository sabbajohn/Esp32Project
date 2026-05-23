#include "transmitter.h"

#include <string.h>

#include "constants.h"

typedef struct {
    bool initialized;
    SubGhzTxState state;
    SubGhzTxConfig cfg;
} subghz_tx_runtime_t;

static subghz_tx_runtime_t g_tx = {
    .initialized = false,
    .state = SubGhzTxStateIdle,
};

bool subghz_tx_init(const SubGhzTxConfig* config) {
    if(!config) {
        return false;
    }
    if(!subghz_environment_check_frequency(config->frequency)) {
        return false;
    }
    if((int32_t)config->power < SUBGHZ_POWER_MIN || (int32_t)config->power > SUBGHZ_POWER_MAX) {
        return false;
    }

    memset(&g_tx, 0, sizeof(g_tx));
    g_tx.cfg = *config;
    g_tx.initialized = true;
    g_tx.state = SubGhzTxStateIdle;
    return true;
}

bool subghz_tx_start(const SubGhzTxData* data, const SubGhzTxCallbacks* callbacks, void* context) {
    if(!g_tx.initialized || !data || !data->data || data->length == 0u) {
        return false;
    }

    g_tx.state = SubGhzTxStatePrepare;
    if(callbacks && callbacks->on_progress) {
        callbacks->on_progress(5, context);
    }

    /*
     * TX path is still pending migration to the new radio HAL.
     * Keep state and callbacks coherent so upper layers can handle it.
     */
    g_tx.state = SubGhzTxStateError;
    if(callbacks && callbacks->on_error) {
        callbacks->on_error(context);
    }
    return false;
}

void subghz_tx_stop(void) {
    g_tx.state = SubGhzTxStateIdle;
}

SubGhzTxState subghz_tx_get_state(void) {
    return g_tx.state;
}
