#pragma once

#include <stdbool.h>

#include "types.h"

typedef struct SubGhzFileEncoderWorker SubGhzFileEncoderWorker;

typedef void (*SubGhzFileEncoderWorkerCallbackEnd)(void* context);

SubGhzFileEncoderWorker* subghz_file_encoder_worker_alloc(void);
void subghz_file_encoder_worker_free(SubGhzFileEncoderWorker* worker);

bool subghz_file_encoder_worker_start(
    SubGhzFileEncoderWorker* worker,
    const char* file_name,
    const char* radio_device_name);
void subghz_file_encoder_worker_stop(SubGhzFileEncoderWorker* worker);
bool subghz_file_encoder_worker_is_running(SubGhzFileEncoderWorker* worker);

void subghz_file_encoder_worker_callback_end(
    SubGhzFileEncoderWorker* worker,
    SubGhzFileEncoderWorkerCallbackEnd callback,
    void* context);

LevelDuration subghz_file_encoder_worker_get_level_duration(SubGhzFileEncoderWorker* worker);
