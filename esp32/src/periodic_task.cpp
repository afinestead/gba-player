#include "periodic_task.h"

#include "network_task.h"
#include "serial_task.h"
#include "spi_task.h"

namespace task {

void PeriodicTask::start() {
    NetworkTask* network_task = NetworkTask::handle();
    SpiTask* spi_task = SpiTask::handle();
    SerialTask* serial_task = SerialTask::handle();
    uint64_t previous_frames_sent = spi_task->framesSent();

    while (true) {
        delay(5000);

        const uint64_t frames_sent = spi_task->framesSent();
        serial_task->enqueueMessage(spi_task->printStats((frames_sent - previous_frames_sent) / 5));
        previous_frames_sent = frames_sent;

        serial_task->enqueueMessage(network_task->printStats());
    }
}

}
