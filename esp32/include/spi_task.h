#pragma once

#include "task.h"

#include "spi_driver.h"
#include "libs/include/logging.h"

#include <cstdint>

namespace task {

using SpiTaskBase = Task<4096>;

class SpiTask : public SpiTaskBase {
public:
    SpiTask()
        : SpiTaskBase("SpiTask", Priority::SPI), spi_driver_(SPI2_HOST)
    {}

    static SpiTask* handle();
    libs::logging::Message printStats() const;

    enum class Event : uint32_t {
        TRANSMIT_COMPLETE = 0x01,
    };

protected:
    void start() override;

    platform::SpiDriver spi_driver_;
    uint64_t frames_sent_ = 0;
};

} // namespace task
