#pragma once

class IDriver {
public:
    virtual ~IDriver() {}
    virtual bool init() = 0;
    virtual void deinit() = 0;
};