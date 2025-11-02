# ESP32 Mini OS - Documentação Técnica

## Estrutura do Projeto
```
project-root/
├── components/
│   ├── brl/                    # Board Runtime Layer
│   │   ├── include/            # Interfaces base
│   │   │   ├── idriver.h       # Interface para drivers
│   │   │   ├── iapp.h         # Interface para aplicativos
│   │   │   ├── event_bus.h    # Sistema de eventos
│   │   │   ├── app_manager.h  # Gerenciador de apps
│   │   │   └── resource_manager.h
│   │   └── drivers/           # Implementações de drivers
│   │       ├── display/       # ST7789
│   │       ├── battery/       # BQ25898, BQ27220
│   │       ├── cc1101/       # Radio
│   │       └── led/          # WS2812
│   └── gui/                   # LVGL + SquareLine
├── apps/                      # Aplicativos
│   └── radio_monitor/        # App exemplo
└── docs/                     # Documentação
```

## Configuração de Hardware (Lilygo T-Embedded CC1101)

### Display (ST7789V)
```c
#define DISPLAY_WIDTH           240
#define DISPLAY_HEIGHT          320
#define DISPLAY_SPI_HOST       SPI2_HOST
#define DISPLAY_SPI_MOSI       GPIO_NUM_9
#define DISPLAY_SPI_CLK        GPIO_NUM_8
#define DISPLAY_SPI_CS         GPIO_NUM_6
#define DISPLAY_DC             GPIO_NUM_7
#define DISPLAY_RST            GPIO_NUM_5
#define DISPLAY_BACKLIGHT      GPIO_NUM_38
```

### CC1101
```c
#define CC1101_SPI_HOST       SPI3_HOST
#define CC1101_SPI_MISO       GPIO_NUM_13
#define CC1101_SPI_MOSI       GPIO_NUM_11
#define CC1101_SPI_CLK        GPIO_NUM_12
#define CC1101_SPI_CS         GPIO_NUM_10
#define CC1101_GDO0           GPIO_NUM_4
#define CC1101_GDO2           GPIO_NUM_14
```

### Sistema de Energia
```c
#define BAT_CHARGE_EN         GPIO_NUM_46
#define BAT_VOLTAGE_DIV_EN    GPIO_NUM_21
#define BAT_VOLTAGE_ADC       GPIO_NUM_1
#define BAT_CHARGE_STATE      GPIO_NUM_47

#define I2C_MASTER_SCL        GPIO_NUM_17
#define I2C_MASTER_SDA        GPIO_NUM_18
#define I2C_MASTER_FREQ       400000
```

## Interfaces Base

### IDriver
Interface base para todos os drivers de hardware:
```cpp
class IDriver {
    virtual bool init() = 0;
    virtual void deinit() = 0;
    virtual const char* name() const = 0;
};
```

### IApp
Interface para aplicativos:
```cpp
class IApp {
    virtual const char* name() = 0;
    virtual void onStart() = 0;
    virtual void onClose() = 0;
    virtual void onSuspend() {}
    virtual void onResume() {}
    virtual int getReturnCode() { return 0; }
};
```

### EventBus
Sistema de eventos para comunicação entre componentes:
```cpp
void subscribe(const std::string& event, Callback cb);
void publish(const std::string& event, void* data = nullptr);
```

Eventos principais:
- `system.battery` - Atualização de estado da bateria
- `system.low_memory` - Aviso de memória baixa
- `radio.signal` - Sinal recebido pelo CC1101

## Gerenciamento de Recursos

### ResourceManager
Monitora e gerencia recursos do sistema:
```cpp
struct SystemState {
    size_t heap_free;
    size_t psram_free;
    float battery_voltage;
    float battery_percentage;
    bool charging;
    uint32_t uptime_ms;
};
```

### AppManager
Gerencia ciclo de vida dos aplicativos:
```cpp
bool startApp(const char* name);
bool suspendApp(const char* name);
bool resumeApp(const char* name);
bool closeApp(const char* name);
```

## Drivers de Hardware

### Display (ST7789)
```cpp
st7789_init_config_t config = {
    .spi_host = DISPLAY_SPI_HOST,
    .mosi = DISPLAY_SPI_MOSI,
    // ... outros pinos ...
};
st7789_init(&config);
```

### CC1101
```cpp
cc1101_init_config_t config = {
    .spi_host = CC1101_SPI_HOST,
    .miso = CC1101_SPI_MISO,
    // ... outros pinos ...
};
cc1101_wrapper_attach(&config);
```

### Sistema de Bateria
```cpp
float voltage = bq27220_get_voltage_mv();
int soc = bq27220_get_state_of_charge();
bool charging = bq25898_get_status() & BQ25898_STATUS_CHARGING;
```

## Desenvolvendo Aplicativos

### Criando um Novo App
1. Crie uma pasta em `apps/seu_app/`
2. Implemente a interface IApp
3. Adicione CMakeLists.txt
4. Registre o app no AppRegistry

Exemplo mínimo:
```cpp
class MyApp : public brl::IApp {
public:
    const char* name() override { return "MyApp"; }
    void onStart() override {
        // Inicialização
    }
    void onClose() override {
        // Cleanup
    }
};
```

### Usando LVGL
```cpp
void MyApp::createUI() {
    screen = lv_obj_create(NULL);
    lv_obj_t* label = lv_label_create(screen);
    lv_label_set_text(label, "Hello World");
    lv_scr_load(screen);
}
```

### Usando EventBus
```cpp
brl::EventBus::instance().subscribe("system.battery",
    [this](void* data) {
        auto state = *(SystemState*)data;
        updateBatteryDisplay(state.battery_percentage);
    });
```

## Compilação e Flash

### Requisitos
- ESP-IDF v5.0 ou superior
- CMake 3.5+
- LVGL 8.3+

### Comandos
```bash
# Configurar
idf.py menuconfig

# Compilar
idf.py build

# Flash
idf.py -p /dev/ttyUSB0 flash

# Monitor
idf.py monitor
```

### Configurações Importantes
- Habilitar PSRAM em menuconfig
- Configurar partição para SPIFFS/SD
- Ajustar tamanho de heap LVGL

## Depuração e Logs

### Níveis de Log
```cpp
ESP_LOGE(TAG, "Erro crítico");
ESP_LOGW(TAG, "Aviso");
ESP_LOGI(TAG, "Info");
ESP_LOGD(TAG, "Debug");
ESP_LOGV(TAG, "Verbose");
```

### Monitoramento de Recursos
```cpp
auto state = brl::ResourceManager::instance().getSystemState();
printf("Heap: %d, Battery: %.1f%%\n",
    state.heap_free,
    state.battery_percentage);
```

## Referências

- [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf)
- [LVGL Docs](https://docs.lvgl.io)
- [Lilygo T-Embedded Docs](https://github.com/Xinyuan-LilyGO/T-Embedded)
- [CC1101 Datasheet](https://www.ti.com/lit/ds/symlink/cc1101.pdf)
- [BQ25898 Datasheet](https://www.ti.com/lit/ds/symlink/bq25898.pdf)
- [BQ27220 Datasheet](https://www.ti.com/lit/ds/symlink/bq27220.pdf)

## Próximos Passos

1. Implementar sistema de persistência de estado
2. Adicionar suporte a SD Card
3. Criar interface de depuração via UART
4. Implementar sistema de atualização OTA
5. Adicionar testes automatizados
6. Documentar protocolo de comunicação CC1101

## Notas de Versão

### v0.1.0 (Base)
- Estrutura inicial do projeto
- Interfaces base (IDriver, IApp)
- EventBus e ResourceManager
- Drivers básicos (display, bateria, CC1101)
- App exemplo (RadioMonitor)

## Troubleshooting

### Problemas Comuns

1. Display não inicializa
   - Verificar pinagem SPI
   - Confirmar alimentação 3.3V
   - Testar reset manual

2. CC1101 não responde
   - Verificar CS e MISO
   - Testar com frequência 433MHz
   - Verificar alimentação

3. Bateria não carrega
   - Verificar enable pin
   - Testar tensão USB
   - Verificar I2C
