#pragma once

#include "task.h"
#include "mailbox.h"

namespace task {

using GbaTaskBase = Task<2048>;

class GbaTask : public GbaTaskBase {
public:
    GbaTask()
        : GbaTaskBase("GbaTask", Priority::GBA)
    {}

    static GbaTask* handle();

protected:
    void start() override;
private:
    enum class Opcode : uint8_t {
        None,
    };
};

} // namespace task