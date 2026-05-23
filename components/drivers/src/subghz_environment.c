#include "environment.h"

#include <string.h>

#include "constants.h"

bool subghz_environment_init(SubGhzEnvironment* env) {
    if(!env) {
        return false;
    }

    memset(env, 0, sizeof(*env));
    env->frequency = SUBGHZ_FREQUENCY_433_92;
    env->modulation = SubGhzModulationOOK;
    env->baudrate = 4800;
    env->deviation = 47;
    env->packet_size = SUBGHZ_MAX_PACKET_SIZE;
    env->manchester = false;
    env->whitening = false;
    return true;
}

bool subghz_environment_set_preset(SubGhzEnvironment* env, SubGhzPreset preset) {
    if(!env) {
        return false;
    }

    switch(preset) {
    case SubGhzPresetGD00:
        env->frequency = SUBGHZ_FREQUENCY_433_92;
        env->modulation = SubGhzModulationOOK;
        env->baudrate = 4800;
        env->deviation = 47;
        env->packet_size = SUBGHZ_MAX_PACKET_SIZE;
        env->manchester = false;
        env->whitening = false;
        return true;
    case SubGhzPresetFPV:
        env->frequency = SUBGHZ_FREQUENCY_868_35;
        env->modulation = SubGhzModulation2FSK;
        env->baudrate = 19200;
        env->deviation = 94;
        env->packet_size = SUBGHZ_MAX_PACKET_SIZE;
        env->manchester = false;
        env->whitening = true;
        return true;
    case SubGhzPresetCustom:
        return subghz_environment_check_frequency(env->frequency);
    default:
        return false;
    }
}

bool subghz_environment_load_custom(SubGhzEnvironment* env, const uint8_t* config, size_t size) {
    if(!env || !config || size < sizeof(SubGhzEnvironment)) {
        return false;
    }

    memcpy(env, config, sizeof(SubGhzEnvironment));
    return subghz_environment_check_frequency(env->frequency);
}

bool subghz_environment_check_frequency(uint32_t frequency) {
    if(frequency < SUBGHZ_FREQUENCY_MIN || frequency > SUBGHZ_FREQUENCY_MAX) {
        return false;
    }

    for(size_t i = 0; i < (sizeof(dangerous_frequencies) / sizeof(dangerous_frequencies[0])); i++) {
        if(frequency == dangerous_frequencies[i]) {
            return false;
        }
    }

    return true;
}
