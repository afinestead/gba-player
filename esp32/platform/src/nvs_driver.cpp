#include "nvs_driver.h"
#include "esp_log.h"

namespace platform {

NvsDriver::NvsDriver(const char* partition_name) :
    partition_name_(partition_name)
{}

void NvsDriver::init() {
    nvs_flash_init_partition(partition_name_);
}

nvs_handle_t NvsDriver::open_ns(const char* ns, Mode mode) {
    nvs_open_mode_t nvs_mode = (mode == Mode::R) ? NVS_READONLY : NVS_READWRITE;
    nvs_handle_t nvs;
    esp_err_t err = nvs_open_from_partition(partition_name_, ns, nvs_mode, &nvs);
    return err == ESP_OK ? nvs : 0;
}

void NvsDriver::close_ns(nvs_handle_t nvs) {
    nvs_close(nvs);
}

bool NvsDriver::save(
    const char* ns, 
    const char* key, 
    const uint8_t* value, 
    size_t value_size
) {
    nvs_handle_t nvs = open_ns(ns, Mode::RW);
    if (!nvs) {
    }
    esp_err_t err = nvs_set_blob(nvs, key, value, value_size);
    if (err != ESP_OK) {
        close_ns(nvs);
        return false;
    }
    err = nvs_commit(nvs);
    if (err != ESP_OK) {
        close_ns(nvs);
        return false;
    }
    close_ns(nvs);
    return true;
}

bool NvsDriver::load(const char* ns, const char* key, uint8_t* value, size_t value_size) {
    size_t required_size = value_size;
    nvs_handle_t nvs = open_ns(ns, Mode::R);
    if (!nvs) {
        return false;
    }
    esp_err_t err = nvs_get_blob(nvs, key, value, &required_size);
    close_ns(nvs);
    if (err != ESP_OK) {
        return false;
    }
    if (required_size != value_size) {
        return false;
    }
    return true;
}

} // namespace platform