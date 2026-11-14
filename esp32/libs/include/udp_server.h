#pragma once

#include <bitset>
#include <cstddef>
#include <cstdint>

#include "logging.h"
#include "protocol/command.h"

namespace libs::network {

static constexpr size_t FRAME_SIZE = 32 * 1024;
static constexpr size_t MAX_PACKET_CHUNK_SIZE = 1400;
static constexpr size_t FRAME_PACKET_COUNT =
    (FRAME_SIZE + MAX_PACKET_CHUNK_SIZE - 1) / MAX_PACKET_CHUNK_SIZE;

#pragma pack(push, 1)
struct PacketHeader {
    uint8_t packet_type;
    uint32_t frame_id;
    uint16_t packet_index;
    uint16_t packet_count;
};
#pragma pack(pop)

class UdpServer {
public:
    struct Stats {
        uint64_t packets_received = 0;
        uint64_t packets_rejected = 0;
        uint64_t duplicate_packets = 0;
        uint64_t completed_frames = 0;
        uint64_t dropped_frames = 0;
    };

    bool receivePacket(
        const uint8_t* packet,
        size_t length,
        uint8_t* frame,
        size_t frame_capacity,
        PacketType& completed_type,
        size_t& completed_length
    );
    const Stats& stats() const { return stats_; }
    libs::logging::Message printStats() const;

private:
    static constexpr size_t PACKET_COUNT = FRAME_PACKET_COUNT;

    std::bitset<PACKET_COUNT> received_packets_{};
    uint32_t frame_id_ = 0;
    uint16_t packet_count_ = 0;
    uint16_t received_count_ = 0;
    size_t frame_length_ = 0;
    PacketType packet_type_ = PacketType::NONE;
    bool active_ = false;
    Stats stats_;

    void reset();
};

}
