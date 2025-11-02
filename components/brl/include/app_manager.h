#pragma once

#include "iapp.h"
#include "event_bus.h"
#include "resource_manager.h"
#include <vector>
#include <memory>
#include <mutex>

namespace brl {

class AppManager {
public:
    static AppManager& instance() {
        static AppManager instance;
        return instance;
    }

    // App lifecycle
    bool startApp(const char* name);
    bool suspendApp(const char* name);
    bool resumeApp(const char* name);
    bool closeApp(const char* name);

    // App state
    IApp* getForegroundApp() const;
    std::vector<IApp*> getBackgroundApps() const;
    bool isAppRunning(const char* name) const;

    // Settings
    void setMaxBackgroundApps(size_t max);
    void setAppSuspendTimeout(uint32_t timeout_ms);

    // State persistence
    bool saveAppState(const char* name, const char* path);
    bool loadAppState(const char* name, const char* path);

private:
    AppManager();

    struct AppState {
        std::unique_ptr<IApp> app;
        bool is_foreground;
        uint32_t last_active;
    };

    std::vector<AppState> apps;
    size_t max_background_apps{3};
    uint32_t suspend_timeout_ms{30000};
    mutable std::mutex mtx;

    bool enforceAppLimits();
    void cleanupSuspendedApps();
    AppState* findApp(const char* name);
};

} // namespace brl
