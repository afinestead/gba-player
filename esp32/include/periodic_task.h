#pragma once

#include "task.h"

namespace task {

using PeriodicTaskBase = Task<2048>;

class PeriodicTask : public PeriodicTaskBase {
public:
    PeriodicTask()
        : PeriodicTaskBase("PeriodicTask", Priority::IDLE)
    {}

protected:
    void start() override;
};

}