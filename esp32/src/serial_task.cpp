#include "esp_log.h"
#include "include/serial_task.h"

namespace task {

namespace serial_task {
static SerialTask* handle;
}

SerialTask* SerialTask::handle() {
    return serial_task::handle;
}

void SerialTask::start() {
    serial_task::handle = this;
    
    while (true) {
        ESP_LOGI("SerialTask", "tick");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
};