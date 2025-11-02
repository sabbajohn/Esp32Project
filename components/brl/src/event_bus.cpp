#include "../include/event_bus.h"
#include <map>
#include <vector>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

static std::map<std::string, std::vector<EventBus::Callback>> subscribers;
static SemaphoreHandle_t eventBusMutex = nullptr;

void EventBus::subscribe(const std::string& event, Callback cb) {
    if (!eventBusMutex) {
        eventBusMutex = xSemaphoreCreateMutex();
    }
    
    if (xSemaphoreTake(eventBusMutex, portMAX_DELAY)) {
        subscribers[event].push_back(cb);
        xSemaphoreGive(eventBusMutex);
    }
}

void EventBus::publish(const std::string& event, void* data) {
    if (!eventBusMutex) return;
    
    if (xSemaphoreTake(eventBusMutex, portMAX_DELAY)) {
        auto it = subscribers.find(event);
        if (it != subscribers.end()) {
            for (auto& cb : it->second) {
                cb(data);
            }
        }
        xSemaphoreGive(eventBusMutex);
    }
}