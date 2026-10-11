#pragma once

#include <cstddef>
#include <cstdint>

namespace gba_protocol {
namespace network {

static constexpr size_t FRAME_SIZE = 32 * 1024;
static constexpr size_t MAX_VIDEO_PACKET_CHUNK_SIZE = 1400;
static constexpr size_t VIDEO_FRAME_PACKET_COUNT =
    (FRAME_SIZE + MAX_VIDEO_PACKET_CHUNK_SIZE - 1) / MAX_VIDEO_PACKET_CHUNK_SIZE;

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