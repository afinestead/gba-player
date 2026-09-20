#pragma once

#include "task.h"

namespace task {

namespace serial_task {
static constexpr uint8_t TASK_PRIORITY = 2;
static constexpr size_t TASK_STACK_SIZE = 2048;
static constexpr char TASK_NAME[] = "SerialTask";
}

class SerialTask : public Task<serial_task::TASK_STACK_SIZE> {
public:
    SerialTask()
        : Task<serial_task::TASK_STACK_SIZE>(
            serial_task::TASK_NAME, serial_task::TASK_PRIORITY
        )
    {}

    SerialTask* handle();

protected:
    void start() override;
};

} // namespace task