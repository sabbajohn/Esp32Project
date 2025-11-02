#include "radio_monitor.h"
#include "components/brl/include/event_bus.h"
#include "components/brl/include/resource_manager.h"
#include <esp_log.h>

static const char* TAG = "RadioMonitor";

namespace apps {

RadioMonitorApp::RadioMonitorApp() {
    // Configure display
    display_config = {
        .spi_host = DISPLAY_SPI_HOST,
        .mosi = DISPLAY_SPI_MOSI,
        .sclk = DISPLAY_SPI_CLK,
        .cs = DISPLAY_SPI_CS,
        .dc = DISPLAY_DC,
        .rst = DISPLAY_RST,
        .bl = DISPLAY_BACKLIGHT,
        .width = DISPLAY_WIDTH,
        .height = DISPLAY_HEIGHT
    };

    // Configure CC1101
    radio_config = {
        .spi_host = CC1101_SPI_HOST,
        .miso = CC1101_SPI_MISO,
        .mosi = CC1101_SPI_MOSI,
        .sclk = CC1101_SPI_CLK,
        .cs = CC1101_SPI_CS,
        .gdo0 = CC1101_GDO0
    };
}

RadioMonitorApp::~RadioMonitorApp() {
    // Cleanup subscriptions if any still active
    brl::EventBus::instance().unsubscribe("radio.signal");
    brl::EventBus::instance().unsubscribe("system.battery");
}

void RadioMonitorApp::onStart() {
    ESP_LOGI(TAG, "Starting RadioMonitor");

    // Initialize display
    if (st7789_init(&display_config) != 0) {
        ESP_LOGE(TAG, "Failed to init display");
        return;
    }

    // Initialize CC1101
    if (cc1101_wrapper_attach(&radio_config) != 0) {
        ESP_LOGE(TAG, "Failed to init CC1101");
        return;
    }

    // Set initial frequency
    cc1101_wrapper_set_frequency(&radio_config, current_freq);
    cc1101_wrapper_start_receive(&radio_config);

    // Create UI
    createUI();

    // Subscribe to events
    brl::EventBus::instance().subscribe("radio.signal",
        [this](void* data) { onSignalReceived(data); });
    brl::EventBus::instance().subscribe("system.battery",
        [this](void* data) { onBatteryUpdate(data); });

    // Initial UI update
    updateUI();
}

void RadioMonitorApp::onClose() {
    ESP_LOGI(TAG, "Closing RadioMonitor");

    // Unsubscribe from events
    brl::EventBus::instance().unsubscribe("radio.signal");
    brl::EventBus::instance().unsubscribe("system.battery");

    // Delete LVGL objects
    if (screen) {
        lv_obj_del(screen);
        screen = nullptr;
        freq_label = nullptr;
        rssi_label = nullptr;
        battery_label = nullptr;
    }
}

void RadioMonitorApp::onSuspend() {
    ESP_LOGI(TAG, "Suspending RadioMonitor");
    // Hide UI but keep radio running
    if (screen) {
        lv_obj_add_flag(screen, LV_OBJ_FLAG_HIDDEN);
    }
}

void RadioMonitorApp::onResume() {
    ESP_LOGI(TAG, "Resuming RadioMonitor");
    // Show UI and refresh
    if (screen) {
        lv_obj_clear_flag(screen, LV_OBJ_FLAG_HIDDEN);
        updateUI();
    }
}

void RadioMonitorApp::createUI() {
    screen = lv_obj_create(NULL);

    // Create labels
    freq_label = lv_label_create(screen);
    lv_obj_align(freq_label, LV_ALIGN_TOP_MID, 0, 10);

    rssi_label = lv_label_create(screen);
    lv_obj_align(rssi_label, LV_ALIGN_CENTER, 0, 0);

    battery_label = lv_label_create(screen);
    lv_obj_align(battery_label, LV_ALIGN_BOTTOM_RIGHT, -10, -10);

    // Load screen
    lv_scr_load(screen);
}

void RadioMonitorApp::updateUI() {
    if (!screen) return;

    // Update frequency display
    char freq_text[32];
    snprintf(freq_text, sizeof(freq_text), "%.3f MHz", current_freq / 1000000.0f);
    lv_label_set_text(freq_label, freq_text);

    // Update battery info
    auto state = brl::ResourceManager::instance().getSystemState();
    char bat_text[32];
    snprintf(bat_text, sizeof(bat_text), "BAT: %.1f%%", state.battery_percentage);
    lv_label_set_text(battery_label, bat_text);
}

void RadioMonitorApp::onSignalReceived(void* data) {
    // Update RSSI display
    if (rssi_label) {
        char rssi_text[32];
        snprintf(rssi_text, sizeof(rssi_text), "RSSI: %d dBm", *(int*)data);
        lv_label_set_text(rssi_label, rssi_text);
    }
}

void RadioMonitorApp::onBatteryUpdate(void* data) {
    // Refresh battery display
    updateUI();
}

} // namespace apps
