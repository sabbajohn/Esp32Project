cmake_minimum_required(VERSION 3.5)

idf_component_register(
    SRCS
        "src/cc1101_driver.cpp"
        "src/subghz_environment.c"
        "src/subghz_receiver.c"
        "src/subghz_transmitter.c"
        "../../platform/esp32/src/board.c"
        "../../platform/esp32/src/radio_hal_esp32.c"
    INCLUDE_DIRS
        "include"
        "../brl/drivers/subghz/include"
        "../brl/drivers/subghz"
        "../../platform/common/include"
        "../../platform/esp32/include"
        "../../core/subghz_core/include"
    REQUIRES
        "driver"
        "esp_timer"
        "freertos"
        "brl"
)

target_compile_definitions(${COMPONENT_LIB} PRIVATE BOARD_EVIL_CROW_RF_V2=1)
