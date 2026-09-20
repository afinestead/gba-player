#include "freertos/FreeRTOS.h"
#include "serial_task.h"
#include "wifi_task.h"

namespace {
    auto serial_task = task::SerialTask();
    auto wifi_task = task::WifiTask();    
}

extern "C" void vApplicationIdleHook( void ) {}

extern "C" void app_main() {
    serial_task.init();
    wifi_task.init();
}