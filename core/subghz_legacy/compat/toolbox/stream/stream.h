#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct Stream Stream;

size_t stream_read(Stream* s, void* data, size_t len);
size_t stream_write(Stream* s, const void* data, size_t len);
void stream_write_char(Stream* s, char c);
void stream_write_cstring(Stream* s, const char* str);
bool stream_seek(Stream* s, size_t position);
size_t stream_size(Stream* s);
void stream_clean(Stream* s);
