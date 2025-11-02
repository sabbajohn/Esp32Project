#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "brl/resource_manager.h"
#include "gui/gui_task.h"
#include "brl/app_registry.h"
#include "brl/app_manager.h"

static const char* TAG = "Main";

void setup() {
    ESP_LOGI(TAG, "Iniciando ESP32 Mini-OS...");

    // Inicializar gerenciador de recursos
    ResourceManager::init();
    ESP_LOGI(TAG, "ResourceManager iniciado");

    // Criar task GUI (LVGL)
    xTaskCreate(gui_task, "gui", 8*1024, NULL, 5, NULL);
    ESP_LOGI(TAG, "GUI task criada");

    // Pequeno delay para garantir que LVGL está pronto
    vTaskDelay(pdMS_TO_TICKS(500));

    ESP_LOGI(TAG, "Sistema iniciado com sucesso");
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000)); // Economia de energia
}
