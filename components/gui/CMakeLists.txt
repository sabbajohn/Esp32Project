cmake_minimum_required(VERSION 3.5)

idf_component_register(
    SRCS 
        "lvgl_component/gui_task.cpp"
    INCLUDE_DIRS 
        "lvgl_component"
    REQUIRES 
        "lvgl"
        "driver"
)