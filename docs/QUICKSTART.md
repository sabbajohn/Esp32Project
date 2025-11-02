# Guia Rápido - ESP32 Mini OS

## 1. Primeira Configuração

### Instalar Dependências
```bash
# ESP-IDF
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh
. ./export.sh

# LVGL
cd components
git clone https://github.com/lvgl/lvgl.git
cd lvgl
git checkout release/v8.3
```

### Configurar Projeto
```bash
# Clonar repositório
git clone https://seu-repo/esp32-mini-os.git
cd esp32-mini-os

# Configurar
idf.py set-target esp32s3
idf.py menuconfig
```

Configurações importantes no menuconfig:
- Component config → ESP PSRAM → Enable PSRAM
- Component config → LVGL → Increase buffer size
- Partition Table → Custom partition table CSV

## 2. Hardware Suportado

### Lilygo T-Embedded CC1101
- ESP32-S3
- Display ST7789V 240x320
- CC1101 para RF sub-1GHz
- Gerenciamento de bateria
- LED RGB WS2812

## 3. Criando um App

### Estrutura Básica
```cpp
// apps/meu_app/meu_app.h
class MeuApp : public brl::IApp {
public:
    const char* name() override { return "MeuApp"; }
    void onStart() override;
    void onClose() override;
private:
    lv_obj_t* screen{nullptr};
};

// apps/meu_app/meu_app.cpp
void MeuApp::onStart() {
    screen = lv_obj_create(NULL);
    lv_scr_load(screen);
}

void MeuApp::onClose() {
    if (screen) {
        lv_obj_del(screen);
        screen = nullptr;
    }
}
```

### CMakeLists.txt
```cmake
idf_component_register(
    SRCS
        "meu_app.cpp"
    INCLUDE_DIRS
        "."
    REQUIRES
        brl
        lvgl
)
```

## 4. APIs Importantes

### EventBus
```cpp
// Subscrever eventos
EventBus::instance().subscribe("system.battery",
    [](void* data) { /* ... */ });

// Publicar eventos
EventBus::instance().publish("app.event", &myData);
```

### ResourceManager
```cpp
// Obter estado do sistema
auto state = ResourceManager::instance().getSystemState();
printf("Bateria: %.1f%%\n", state.battery_percentage);
```

### AppManager
```cpp
// Iniciar app
AppManager::instance().startApp("MeuApp");

// Suspender app
AppManager::instance().suspendApp("MeuApp");
```

## 5. Usando Hardware

### Display
```cpp
st7789_init_config_t cfg = {
    .spi_host = DISPLAY_SPI_HOST,
    .mosi = DISPLAY_SPI_MOSI,
    .sclk = DISPLAY_SPI_CLK,
    .cs = DISPLAY_SPI_CS,
    .dc = DISPLAY_DC,
    .rst = DISPLAY_RST,
    .bl = DISPLAY_BACKLIGHT
};
st7789_init(&cfg);
```

### CC1101
```cpp
cc1101_init_config_t cfg = {
    .spi_host = CC1101_SPI_HOST,
    .miso = CC1101_SPI_MISO,
    .mosi = CC1101_SPI_MOSI,
    .sclk = CC1101_SPI_CLK,
    .cs = CC1101_SPI_CS,
    .gdo0 = CC1101_GDO0
};
cc1101_wrapper_attach(&cfg);
```

### LED RGB
```cpp
ws2812_init(RGB_LED_PIN, 1);
ws2812_set_pixel_rgb(0, 255, 0, 0); // Vermelho
ws2812_show();
```

## 6. Compilar e Flashear

### Desenvolvimento
```bash
# Compilar
idf.py build

# Flash + Monitor
idf.py -p /dev/ttyUSB0 flash monitor
```

### Produção
```bash
# Build otimizado
idf.py -DCMAKE_BUILD_TYPE=Release build

# Flash partição específica
idf.py -p /dev/ttyUSB0 app-flash
```

## 7. Depuração

### Logs
```cpp
ESP_LOGI(TAG, "Info normal");
ESP_LOGW(TAG, "Aviso: %d", valor);
ESP_LOGE(TAG, "Erro!");
```

### Monitor Serial
```bash
# Ver logs
idf.py monitor

# Filtrar logs
idf.py monitor | grep "MeuApp"
```

## 8. Dicas e Truques

### Memória
- Use PSRAM para buffers grandes
- Monitore heap com ESP_LOGI
- Limite número de apps em background

### Performance
- Evite operações longas no loop principal
- Use DMA para SPI quando possível
- Minimize alocações dinâmicas

### Interface
- Minimize updates LVGL
- Use eventos para atualizar UI
- Implemente onSuspend/onResume

## 9. Recursos

- [Documentação Completa](docs/TECHNICAL.md)
- [Exemplos](apps/radio_monitor/)
- [Esquemáticos](docs/schematics/)
- [Datasheet CC1101](https://www.ti.com/lit/ds/symlink/cc1101.pdf)
- [Wiki LVGL](https://docs.lvgl.io/8.3/)
