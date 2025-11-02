#include "../include/resource_manager.h"
#include "esp_heap_caps.h"
#include "esp_system.h"

size_t ResourceManager::getFreeHeap() {
    return heap_caps_get_free_size(MALLOC_CAP_8BIT);
}

size_t ResourceManager::getFreePsram() {
    return heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
}

float ResourceManager::getBatteryVoltage() {
    // TODO: Implementar leitura do ADC/PMIC específico da placa
    return 3.7f;
}

void ResourceManager::init() {
    // TODO: Inicializar ADC/PMIC, timers de monitoramento
}