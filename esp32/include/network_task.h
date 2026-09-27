#pragma once

#include "task.h"

namespace task {

using NetworkTaskBase = Task<2048>;

class NetworkTask : public NetworkTaskBase {
public:
    NetworkTask()
        : NetworkTaskBase("NetworkTask", Priority::NETWORK)
    {}

    static NetworkTask* handle();
protected:
    void start() override;
private:
    static constexpr size_t RECV_BUFFER_LEN = 8192;
    uint8_t recv_buffer_[RECV_BUFFER_LEN];

    void receivePacket(size_t len);

};

} // namespace task