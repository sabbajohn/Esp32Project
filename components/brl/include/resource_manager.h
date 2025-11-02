#pragma once
#include <cstddef>

class ResourceManager {
public:
    static size_t getFreeHeap();
    static size_t getFreePsram();
    static float getBatteryVoltage();
    static void init();
};