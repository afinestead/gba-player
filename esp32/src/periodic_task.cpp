#include "periodic_task.h"

#include "network_task.h"
#include "serial_task.h"

namespace task {

void PeriodicTask::start() {
    NetworkTask* network_task = NetworkTask::handle();
    SerialTask* serial_task = SerialTask::handle();

    while (true) {
        delay(5000);
        
        serial_task->enqueueMessage(network_task->printStats());
    }
}

}