#pragma once

#include "task.h"
#include "mailbox.h"

namespace task {

static constexpr size_t SERIAL_TASK_STACK_SIZE = 2048;

class SerialTask : public Task<SERIAL_TASK_STACK_SIZE> {
public:
    static constexpr size_t MAX_MESSAGE_LEN = 40;

    struct Message {
        char s[MAX_MESSAGE_LEN];
    };

    SerialTask()
        : Task<SERIAL_TASK_STACK_SIZE>(TASK_NAME, TASK_PRIORITY)
    {}

    static SerialTask* handle();

    bool enqueueMessage(const Message& msg);

protected:
    static constexpr uint8_t TASK_PRIORITY = 2;
    static constexpr char TASK_NAME[] = "SerialTask";

    void start() override;
private:
    Mailbox<Message, 10> mailbox_;
};

} // namespace task