#pragma once

#include "components/brl/include/iapp.h"
#include "components/brl/drivers/cc1101/cc1101_wrapper.h"
#include "components/brl/drivers/display/st7789.h"
#include "components/brl/drivers/include/board_config.h"
#include <lvgl.h>

namespace apps {

class RadioMonitorApp : public brl::IApp {
public:
    RadioMonitorApp();
    ~RadioMonitorApp();

    // IApp interface
    const char* name() override { return "RadioMonitor"; }
    void onStart() override;
    void onClose() override;
    void onSuspend() override;
    void onResume() override;
    bool needsGUI() override { return true; }

private:
    // Display
    st7789_init_config_t display_config;
    lv_obj_t* screen{nullptr};
    lv_obj_t* freq_label{nullptr};
    lv_obj_t* rssi_label{nullptr};
    lv_obj_t* battery_label{nullptr};

    // CC1101
    cc1101_init_config_t radio_config;
    uint32_t current_freq{433920000}; // 433.92 MHz

    // Event handlers
    void onSignalReceived(void* data);
    void onBatteryUpdate(void* data);

    // UI helpers
    void createUI();
    void updateUI();
};

} // namespace apps
