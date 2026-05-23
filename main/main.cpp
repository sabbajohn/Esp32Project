#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "../components/brl/include/resource_manager.h"
#include "../components/brl/include/event_bus.h"

static const char* TAG = "Main";

// Declaração prévia da task GUI (implementada no componente GUI)
extern "C" void gui_task(void* arg);

extern "C" void app_main() {
    // Inicialização do sistema
    ESP_LOGI(TAG, "Iniciando sistema...");
    
    // Inicializar gerenciador de recursos
    ResourceManager::init();
    
    // Criar task GUI (LVGL)
    xTaskCreate(gui_task, "gui", 8*1024, NULL, 5, NULL);
    
    // Delay pequeno para garantir que LVGL está pronto
    vTaskDelay(pdMS_TO_TICKS(500));
    
    ESP_LOGI(TAG, "Sistema iniciado com sucesso");
    
    // app_main retorna, tasks continuam executando
}
