#pragma once

#include "task.h"

#include "libs/include/frame_buffer.h"
#include "libs/include/udp_server.h"

namespace task {

using NetworkTaskBase = Task<4096>;
using FrameBuffer = libs::FrameBuffer<uint8_t, libs::network::FRAME_SIZE>;

class NetworkTask : public NetworkTaskBase {
public:
    NetworkTask() :
        NetworkTaskBase("NetworkTask", Priority::NETWORK)
    {}

    static NetworkTask* handle();
    libs::logging::Message printStats() const;

    static FrameBuffer buffers;
protected:
    void start() override;
private:
    static constexpr size_t RECV_BUFFER_LEN = 1500;
    uint8_t recv_buffer_[RECV_BUFFER_LEN];
    libs::network::UdpServer server_;
};

} // namespace task