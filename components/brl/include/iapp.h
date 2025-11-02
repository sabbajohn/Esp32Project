#pragma once
#include <cstdio>

class IApp {
public:
    virtual ~IApp() {}
    virtual const char* name() = 0;
    virtual void onStart() = 0;
    virtual void onClose() = 0;
    virtual void onSuspend() {}
    virtual void onResume() {}
    virtual void serializeState(FILE* f) {}
    virtual void deserializeState(FILE* f) {}
    virtual int getReturnCode() { return 0; }
};