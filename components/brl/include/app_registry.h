#pragma once

#include "iapp.h"

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace brl {

class AppRegistry {
public:
    using Factory = std::function<std::unique_ptr<IApp>()>;

    static AppRegistry& instance();

    void registerApp(const std::string& name, Factory factory);
    std::unique_ptr<IApp> createApp(const std::string& name) const;

private:
    std::unordered_map<std::string, Factory> factories_;
};

} // namespace brl
