#pragma once

#include "task.h"

namespace task {

static constexpr size_t WIFI_TASK_STACK_SIZE = 4096;

class WifiTask : public Task<WIFI_TASK_STACK_SIZE> {
public:
    enum class Event : uint32_t {
        CONNECTED = 1 << 0,
        DISCONNECTED = 1 << 1,
    };

    WifiTask()
        : Task<WIFI_TASK_STACK_SIZE>(TASK_NAME, TASK_PRIORITY)
    {}

    WifiTask* handle();

    bool connected();

protected:
    static constexpr uint8_t TASK_PRIORITY = 2;
    static constexpr char TASK_NAME[] = "WifiTask";
    static constexpr size_t RECV_BUFFER_LEN = 8192;

    void start() override;
private:
    bool connected_;

    uint8_t recv_buffer_[RECV_BUFFER_LEN];

    void receivePacket(size_t len);
};

} // namespace task