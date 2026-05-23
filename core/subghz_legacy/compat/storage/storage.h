#pragma once

#include <stdbool.h>

typedef struct Storage Storage;
struct Storage {
    int dummy;
};

bool storage_simply_mkdir(Storage* storage, const char* path);
bool storage_simply_remove(Storage* storage, const char* path);
