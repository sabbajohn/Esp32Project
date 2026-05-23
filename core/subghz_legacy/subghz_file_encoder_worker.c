#include "subghz_file_encoder_worker.h"

#include <furi.h>

struct SubGhzFileEncoderWorker {
    bool running;
    SubGhzFileEncoderWorkerCallbackEnd callback;
    void* callback_context;
};

SubGhzFileEncoderWorker* subghz_file_encoder_worker_alloc(void) {
    return calloc(1, sizeof(SubGhzFileEncoderWorker));
}

void subghz_file_encoder_worker_free(SubGhzFileEncoderWorker* worker) {
    free(worker);
}

bool subghz_file_encoder_worker_start(
    SubGhzFileEncoderWorker* worker,
    const char* file_name,
    const char* radio_device_name) {
    UNUSED(file_name);
    UNUSED(radio_device_name);
    if(!worker) {
        return false;
    }
    worker->running = true;
    return true;
}

void subghz_file_encoder_worker_stop(SubGhzFileEncoderWorker* worker) {
    if(!worker) return;
    worker->running = false;
    if(worker->callback) {
        worker->callback(worker->callback_context);
    }
}

bool subghz_file_encoder_worker_is_running(SubGhzFileEncoderWorker* worker) {
    return worker ? worker->running : false;
}

void subghz_file_encoder_worker_callback_end(
    SubGhzFileEncoderWorker* worker,
    SubGhzFileEncoderWorkerCallbackEnd callback,
    void* context) {
    if(!worker) return;
    worker->callback = callback;
    worker->callback_context = context;
}

LevelDuration subghz_file_encoder_worker_get_level_duration(SubGhzFileEncoderWorker* worker) {
    UNUSED(worker);
    return level_duration_reset();
}
