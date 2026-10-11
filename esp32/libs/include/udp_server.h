#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "protocol.h"

namespace libs::network {

class UdpServer {
public:
    using FrameHandler = void (*)(void*, const uint8_t*, size_t);

    UdpServer(FrameHandler frame_handler, void* context);

    void receivePacket(const uint8_t* packet, size_t length);

private:
    static constexpr size_t FRAME_SIZE = gba_protocol::network::FRAME_SIZE;

    std::array<uint8_t, FRAME_SIZE> frame_{};
    std::array<uint8_t, (FRAME_SIZE + 7) / 8> received_packets_{};
    FrameHandler frame_handler_;
    void* context_;
    uint32_t frame_id_ = 0;
    uint16_t packet_count_ = 0;
    uint16_t received_count_ = 0;
    bool active_ = false;

    void reset();
};

}