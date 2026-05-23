#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern "C" void gui_task(void* arg) {
    (void)arg;
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
