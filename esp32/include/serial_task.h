#pragma once

#include "task.h"
#include "mailbox.h"
#include "libs/include/logging.h"

namespace task {

using SerialTaskBase = Task<2048>;

class SerialTask : public SerialTaskBase {
public:
    static constexpr size_t MAX_MESSAGE_LEN = libs::logging::MESSAGE_MAX_LEN;
    using Message = libs::logging::Message;

    SerialTask()
        : SerialTaskBase("SerialTask", Priority::SERIAL)
    {}

    static SerialTask* handle();

    bool enqueueMessage(const Message& msg);

protected:
    void start() override;
private:
    libs::Mailbox<Message, 10> mailbox_;
};

} // namespace task