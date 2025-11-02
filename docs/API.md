## API Reference

### Board Runtime Layer (BRL)

#### IDriver Interface
```cpp
namespace brl {

class IDriver {
public:
    virtual ~IDriver() = default;
    virtual bool init() = 0;
    virtual void deinit() = 0;
    virtual const char* name() const = 0;
};

} // namespace brl
```

Main methods:
- `init()`: Initialize the driver and its hardware
- `deinit()`: Clean up and release resources
- `name()`: Get driver identifier string

#### IApp Interface
```cpp
namespace brl {

class IApp {
public:
    virtual ~IApp() = default;

    // Identification
    virtual const char* name() = 0;
    virtual const char* version() { return "1.0.0"; }

    // Lifecycle
    virtual void onStart() = 0;
    virtual void onClose() = 0;
    virtual void onSuspend() {}
    virtual void onResume() {}

    // State
    virtual int getReturnCode() { return 0; }
    virtual void serializeState(FILE* f) {}
    virtual void deserializeState(FILE* f) {}

    // Resources
    virtual size_t getMemoryUsage() { return 0; }
    virtual bool usesPSRAM() { return false; }
    virtual bool needsGUI() { return true; }
};

} // namespace brl
```

Lifecycle methods:
- `onStart()`: Called when app becomes active
- `onClose()`: Called when app is terminated
- `onSuspend()`: Called when app moves to background
- `onResume()`: Called when app returns to foreground

State methods:
- `serializeState()`: Save app state to file
- `deserializeState()`: Restore app state from file
- `getReturnCode()`: Get app exit status

Resource methods:
- `getMemoryUsage()`: Report app memory usage
- `usesPSRAM()`: Whether app needs PSRAM
- `needsGUI()`: Whether app needs display

#### EventBus
```cpp
namespace brl {

class EventBus {
public:
    using Callback = std::function<void(void*)>;

    static EventBus& instance();

    void subscribe(const std::string& event, Callback cb);
    void unsubscribe(const std::string& event);
    void publish(const std::string& event, void* data = nullptr);
};

} // namespace brl
```

Main methods:
- `subscribe()`: Register callback for event
- `unsubscribe()`: Remove callback registration
- `publish()`: Send event to all subscribers

Standard events:
- `system.battery`: Battery state update
- `system.memory`: Memory state update
- `system.button`: Button press/release
- `radio.signal`: RF signal received
- `app.launched`: New app started
- `app.closed`: App terminated

#### ResourceManager
```cpp
namespace brl {

struct SystemState {
    size_t heap_free;
    size_t psram_free;
    float battery_voltage;
    float battery_percentage;
    bool charging;
    uint32_t uptime_ms;
};

class ResourceManager {
public:
    static ResourceManager& instance();

    // Resource tracking
    void lockResource(const std::string& resource, const std::string& owner);
    void unlockResource(const std::string& resource);
    bool isResourceLocked(const std::string& resource) const;

    // System state
    SystemState getSystemState() const;

    // Specific getters
    size_t getFreeHeap() const;
    size_t getFreePsram() const;
    float getBatteryVoltage() const;
    float getBatteryPercentage() const;
    bool isCharging() const;
};

} // namespace brl
```

Main methods:
- Resource locking:
  - `lockResource()`: Claim exclusive resource access
  - `unlockResource()`: Release resource
  - `isResourceLocked()`: Check resource availability

- System monitoring:
  - `getSystemState()`: Get full system status
  - `getFreeHeap()`: Get available heap
  - `getFreePsram()`: Get available PSRAM
  - `getBatteryVoltage()`: Get battery voltage
  - `getBatteryPercentage()`: Get battery level
  - `isCharging()`: Get charging status

#### AppManager
```cpp
namespace brl {

class AppManager {
public:
    static AppManager& instance();

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
};

} // namespace brl
```

Main methods:
- Lifecycle management:
  - `startApp()`: Launch new app
  - `suspendApp()`: Move app to background
  - `resumeApp()`: Restore app to foreground
  - `closeApp()`: Terminate app

- State queries:
  - `getForegroundApp()`: Get active app
  - `getBackgroundApps()`: Get background apps
  - `isAppRunning()`: Check if app is active

- Configuration:
  - `setMaxBackgroundApps()`: Limit background apps
  - `setAppSuspendTimeout()`: Set suspend timeout

- State persistence:
  - `saveAppState()`: Save app state to storage
  - `loadAppState()`: Restore app state

### Hardware Drivers

#### Display (ST7789)
```cpp
struct st7789_init_config_t {
    int spi_host;
    int mosi;
    int sclk;
    int cs;
    int dc;
    int rst;
    int bl;
    uint16_t width;
    uint16_t height;
};

int st7789_init(const st7789_init_config_t *config);
void st7789_set_backlight(uint8_t brightness);
```

#### CC1101 Wrapper
```cpp
struct cc1101_init_config_t {
    int spi_host;
    int miso;
    int mosi;
    int sclk;
    int cs;
    int gdo0;
};

int cc1101_wrapper_attach(const FuriHalSpiBusHandle* handle);
uint32_t cc1101_wrapper_set_frequency(const FuriHalSpiBusHandle* handle, uint32_t freq_hz);
int cc1101_wrapper_write_fifo(const FuriHalSpiBusHandle* handle, const uint8_t* data, uint8_t size);
int cc1101_wrapper_start_receive(const FuriHalSpiBusHandle* handle);
```

#### Battery Management
```cpp
// BQ25898 (Charger)
int bq25898_init(void);
bool bq25898_enable_charging(bool enable);
int bq25898_get_status(void);

// BQ27220 (Fuel Gauge)
int bq27220_init(void);
int bq27220_get_state_of_charge(void);
int bq27220_get_voltage_mv(void);
```

#### RGB LED (WS2812)
```cpp
int ws2812_init(int pin, int count);
int ws2812_set_pixel_rgb(int index, uint8_t r, uint8_t g, uint8_t b);
int ws2812_show(void);
```
