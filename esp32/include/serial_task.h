#pragma once

#include "task.h"
#include "mailbox.h"

namespace task {

using SerialTaskBase = Task<2048>;

class SerialTask : public SerialTaskBase {
public:
    static constexpr size_t MAX_MESSAGE_LEN = 40;

    struct Message {
        char s[MAX_MESSAGE_LEN];
    };

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