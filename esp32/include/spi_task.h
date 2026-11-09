#pragma once

#include "task.h"

#include "spi_driver.h"

namespace task {

using SpiTaskBase = Task<4096>;

class SpiTask : public SpiTaskBase {
public:
    SpiTask()
        : SpiTaskBase("SpiTask", Priority::SPI), spi_driver_(SPI1_HOST)
    {}

    static SpiTask* handle();

    enum class Event : uint32_t {
        TRANSMIT_COMPLETE = 0x01,
    };

protected:
    void start() override;

    platform::SpiDriver spi_driver_;
};

} // namespace task