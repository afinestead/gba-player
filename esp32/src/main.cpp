#include "freertos/FreeRTOS.h"
#include "gba_task.h"
#include "network_task.h"
#include "periodic_task.h"
#include "serial_task.h"
#include "spi_task.h"
#include "wifi_task.h"

#include "esp_err.h"
#include "nvs_flash.h"



namespace {
    auto serial_task = task::SerialTask();
    auto spi_task = task::SpiTask();
    auto wifi_task = task::WifiTask();
    auto gba_task = task::GbaTask();
    auto network_task = task::NetworkTask();
    auto periodic_task = task::PeriodicTask();

}

extern "C" void vApplicationIdleHook( void ) {}

extern "C" void app_main() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    serial_task.init();
    spi_task.init();
    wifi_task.init();
    gba_task.init();
    network_task.init();
    periodic_task.init();
}