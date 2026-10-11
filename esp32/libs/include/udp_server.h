#pragma once

#include <bitset>
#include <cstddef>
#include <cstdint>

#include "protocol.h"

namespace libs::network {

class UdpServer {
public:
    bool receivePacket(const uint8_t* packet, size_t length, uint8_t* frame);

private:
    static constexpr size_t FRAME_SIZE = gba_protocol::network::FRAME_SIZE;

    static constexpr size_t PACKET_COUNT = gba_protocol::network::VIDEO_FRAME_PACKET_COUNT;

    std::bitset<PACKET_COUNT> received_packets_{};
    uint32_t frame_id_ = 0;
    uint16_t packet_count_ = 0;
    uint16_t received_count_ = 0;
    bool active_ = false;

    void reset();
};

}