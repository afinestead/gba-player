#include "network_task.h"
#include "esp_attr.h"
#include <cerrno>
#include <optional>
#include <sys/socket.h>
#include <unistd.h>

namespace task {

namespace {
constexpr char TAG[] = "NetworkTask";
}

DMA_ATTR FrameBuffer NetworkTask::buffers{};

namespace network_task {
static NetworkTask* handle;
}

NetworkTask* NetworkTask::handle() {
    return network_task::handle;
}

libs::logging::Message NetworkTask::printStats() const {
    return server_.printStats();
}

void NetworkTask::start() {
    network_task::handle = this;

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    uint32_t rcvbuf = 64 * 1024;
    setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(5000);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));

    std::optional<FrameBuffer::Buffer> write_buffer;

    while (true) {
        sockaddr_in source{};
        socklen_t source_len = sizeof(source);
        int len = recvfrom(
            sock,
            recv_buffer_,
            RECV_BUFFER_LEN,
            0,
            reinterpret_cast<sockaddr*>(&source),
            &source_len
        );
        if (!write_buffer.has_value()) {
            write_buffer = buffers.acquireFree(0);
        }

        if (len > 0 && write_buffer.has_value()) {
            PacketType completed_type = PacketType::NONE;
            size_t completed_length = 0;
            const bool frame_complete = server_.receivePacket(
                recv_buffer_,
                static_cast<size_t>(len),
                write_buffer->payload(),
                FrameBuffer::payloadSize(),
                completed_type,
                completed_length
            );
            if (frame_complete) {
                *write_buffer->header() = ProtocolHeader{
                    .opcode = completed_type,
                    .sequence = 0,
                    .length = static_cast<uint16_t>(completed_length)
                };
                buffers.publishReady(write_buffer->index);
                write_buffer.reset();
            }
        }
    }
}

} // namespace task
