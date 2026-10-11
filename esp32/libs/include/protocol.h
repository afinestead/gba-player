#pragma once

#include <cstddef>
#include <cstdint>

namespace gba_protocol {
namespace network {

static constexpr std::size_t FRAME_SIZE = 32 * 1024;

enum class PacketType : uint8_t {
    NONE = 0x00,
    VIDEO_FRAME,
};

struct PacketHeader {
    PacketType packet_type;
};

struct VideoPacketHeader {
    uint32_t frame_id;
    uint16_t packet_index;
    uint16_t packet_count;
};

}
}