idf_component_register(SRCS
    "display/st7789.c"
    "battery/bq25898.c"
    "battery/bq27220.c"
    "cc1101/cc1101_wrapper.c"
    "led/ws2812.c"
    "../subghz/cc1101_configs.c"
    "../subghz/devices.c"
    "../subghz/receiver.c"
    "../subghz/transmitter.c"

    INCLUDE_DIRS
    "include"
    "display"
    "battery"
    "cc1101"
    "led"
)

target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-unused-variable" "-Wno-unused-function")
