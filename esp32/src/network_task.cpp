#include "network_task.h"
#include <sys/socket.h>
#include <algorithm>

namespace task {

FrameBuffer NetworkTask::buffers_{};

namespace network_task {
static NetworkTask* handle;
}

NetworkTask* NetworkTask::handle() {
    return network_task::handle;
}

void NetworkTask::start() {
    network_task::handle = this;

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(5000);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(sock, (sockaddr*)&addr, sizeof(addr));

    sockaddr_in source{};
    socklen_t source_len = sizeof(source);

    while (true) {
        int len = recvfrom(
            sock,
            recv_buffer_,
            RECV_BUFFER_LEN,
            0,
            reinterpret_cast<sockaddr*>(&source),
            &source_len
        );
        if (len > 0) {
            server_.receivePacket(recv_buffer_, static_cast<size_t>(len));
        }
    }
}

void NetworkTask::publishFrame(void*, const uint8_t* frame, size_t length) {
    auto buffer = buffers_.acquireFree(0);
    if (!buffer || length != gba_protocol::network::FRAME_SIZE) {
        return;
    }

    std::copy_n(frame, length, buffer->data);
    buffers_.publishReady(buffer->index);
}

} // namespace task
