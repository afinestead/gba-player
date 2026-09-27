#pragma once

#include "nvs.h"
#include "nvs_flash.h"

namespace platform {

class NvsDriver {
public:
    enum class Mode : uint8_t { R, RW };

    NvsDriver(const char* partition_name);

    void init();
    bool load(const char* ns, const char* key, uint8_t* value, size_t value_size);
    bool save(const char* ns, const char* key, const uint8_t* value, size_t value_size);
    // bool clear(nvs_handle_t nvs);
private:
    nvs_handle_t open_ns(const char* ns, Mode mode);
    void close_ns(nvs_handle_t nvs);

    const char* partition_name_;
};
}