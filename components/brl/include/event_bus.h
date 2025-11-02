#pragma once
#include <functional>
#include <string>

class EventBus {
public:
    using Callback = std::function<void(void*)>;
    
    static void subscribe(const std::string& event, Callback cb);
    static void publish(const std::string& event, void* data = nullptr);
};