#pragma once

#include "task.h"

namespace task {

using SpiTaskBase = Task<4096>;

class SpiTask : public SpiTaskBase {
public:
    SpiTask()
        : SpiTaskBase("SpiTask", Priority::SPI)
    {}

    static SpiTask* handle();

protected:
    void start() override;
};

} // namespace task