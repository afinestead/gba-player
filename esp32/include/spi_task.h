#pragma once

#include "task.h"
#include "mailbox.h"

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
private:
    libs::Mailbox<Message, 10> mailbox_;
};

} // namespace task