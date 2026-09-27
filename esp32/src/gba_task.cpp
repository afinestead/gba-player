#include "esp_log.h"
#include "gba_task.h"
#include "wifi_task.h"

namespace task {

namespace gba_task {
static GbaTask* handle;
}

GbaTask* GbaTask::handle() {
    return gba_task::handle;
}

void GbaTask::start() {
    gba_task::handle = this;
    
    while (true) {
        delay(10000);
        WifiTask::handle()->scanNetworks();
    }
}

}