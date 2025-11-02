cmake_minimum_required(VERSION 3.5)

idf_component_register(
    SRCS 
        "src/resource_manager.cpp"
        "src/event_bus.cpp"
        "src/app_manager.cpp"
        "src/app_registry.cpp"
    INCLUDE_DIRS 
        "include"
    REQUIRES 
        "driver"
        "esp_common"
        "freertos"
)