#include "app_manager.h"
#include <esp_log.h>

static const char* TAG = "AppManager";

namespace brl {

AppManager::AppManager() {
    EventBus::instance().subscribe("system.low_memory", [this](void*) {
        this->enforceAppLimits();
    });
}

bool AppManager::startApp(const char* name) {
    std::lock_guard<std::mutex> lock(mtx);

    // Check if app is already running
    if (auto* existing = findApp(name)) {
        if (existing->is_foreground) {
            ESP_LOGW(TAG, "App %s is already in foreground", name);
            return true;
        }
        // Move to foreground if running in background
        existing->is_foreground = true;
        existing->last_active = esp_timer_get_time() / 1000;
        existing->app->onResume();
        return true;
    }

    // Create new app instance
    auto app = AppRegistry::instance().createApp(name);
    if (!app) {
        ESP_LOGE(TAG, "Failed to create app %s", name);
        return false;
    }

    // Enforce limits before adding new app
    enforceAppLimits();

    // Add and start the app
    AppState state;
    state.app = std::move(app);
    state.is_foreground = true;
    state.last_active = esp_timer_get_time() / 1000;

    try {
        state.app->onStart();
        apps.push_back(std::move(state));
        return true;
    } catch (const std::exception& e) {
        ESP_LOGE(TAG, "Exception in app %s start: %s", name, e.what());
        return false;
    }
}

bool AppManager::suspendApp(const char* name) {
    std::lock_guard<std::mutex> lock(mtx);

    if (auto* app_state = findApp(name)) {
        if (!app_state->is_foreground) {
            ESP_LOGW(TAG, "App %s is already suspended", name);
            return true;
        }

        try {
            app_state->app->onSuspend();
            app_state->is_foreground = false;
            return true;
        } catch (const std::exception& e) {
            ESP_LOGE(TAG, "Exception in app %s suspend: %s", name, e.what());
            return false;
        }
    }

    ESP_LOGW(TAG, "App %s not found for suspend", name);
    return false;
}

bool AppManager::enforceAppLimits() {
    size_t background_count = 0;

    // Count background apps
    for (const auto& state : apps) {
        if (!state.is_foreground) background_count++;
    }

    // Suspend oldest background apps if over limit
    while (background_count > max_background_apps) {
        uint32_t oldest_time = UINT32_MAX;
        AppState* oldest_app = nullptr;

        for (auto& state : apps) {
            if (!state.is_foreground && state.last_active < oldest_time) {
                oldest_time = state.last_active;
                oldest_app = &state;
            }
        }

        if (oldest_app) {
            ESP_LOGI(TAG, "Suspending old app %s", oldest_app->app->name());
            try {
                oldest_app->app->onSuspend();
                background_count--;
            } catch (const std::exception& e) {
                ESP_LOGE(TAG, "Error suspending app: %s", e.what());
            }
        }
    }

    return true;
}

AppManager::AppState* AppManager::findApp(const char* name) {
    for (auto& state : apps) {
        if (strcmp(state.app->name(), name) == 0) {
            return &state;
        }
    }
    return nullptr;
}

// Other methods implementation...

} // namespace brl
