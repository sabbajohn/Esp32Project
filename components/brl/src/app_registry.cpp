#include "app_registry.h"

namespace brl {

AppRegistry& AppRegistry::instance() {
    static AppRegistry inst;
    return inst;
}

void AppRegistry::registerApp(const std::string& name, Factory factory) {
    factories_[name] = std::move(factory);
}

std::unique_ptr<IApp> AppRegistry::createApp(const std::string& name) const {
    auto it = factories_.find(name);
    if(it == factories_.end()) {
        return nullptr;
    }
    return it->second();
}

} // namespace brl
