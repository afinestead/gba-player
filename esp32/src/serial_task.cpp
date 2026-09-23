#include "esp_log.h"
#include "include/serial_task.h"

namespace task {

namespace serial_task {
static SerialTask* handle;
}

SerialTask* SerialTask::handle() {
    return serial_task::handle;
}

bool SerialTask::enqueueMessage(const Message& msg) {
    return mailbox_.put(msg, 0);
}

void SerialTask::start() {
    serial_task::handle = this;
    
    Message recvd;
    while (true) {
        bool ok = mailbox_.get(&recvd, 1000);
        if (ok) {
            ESP_LOGI("SerialTask", "%s", recvd.s);
        }
    }
}
};