#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

extern "C" void app_main()
{
    while (true) {
        ESP_LOGI("MAIN", "Hello from ESP32-S3!");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}