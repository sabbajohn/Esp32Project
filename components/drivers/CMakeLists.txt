cmake_minimum_required(VERSION 3.5)

idf_component_register(
    SRCS 
        "src/cc1101_driver.cpp"
    INCLUDE_DIRS 
        "include"
    REQUIRES 
        "driver"
        "brl"
)