#pragma once

#include <cstdio>

#define LOG_DEBUG(fmt, ...) std::printf("%s: " fmt "\n", __FUNCTION__, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) std::printf("%s: " fmt "\n", __FUNCTION__, ##__VA_ARGS__)

void dumpHex(const char* name, const void* data, size_t size);
#define HEXDUMP_DEBUG(name, buf, size) dumpHex(name, buf, size)