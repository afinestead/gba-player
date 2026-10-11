#include "udp_server.h"

#include <algorithm>
#include <cstring>

#include "protocol.h"

namespace libs::network {

bool UdpServer::receivePacket(const uint8_t* packet, size_t length, uint8_t* frame) {
    using gba_protocol::network::PacketHeader;
    using gba_protocol::network::PacketType;
    using gba_protocol::network::VideoPacketHeader;

    if (frame == nullptr) {
        reset();
        return false;
    }

    if (length < sizeof(PacketHeader)) {
        return false;
    }

    PacketHeader packet_header{};
    std::memcpy(&packet_header, packet, sizeof(packet_header));
    if (packet_header.packet_type != PacketType::VIDEO_FRAME ||
        length < sizeof(PacketHeader) + sizeof(VideoPacketHeader)) {
        return false;
    }

    VideoPacketHeader video_header{};
    std::memcpy(&video_header, packet + sizeof(PacketHeader), sizeof(video_header));
    if (video_header.packet_count != PACKET_COUNT ||
        video_header.packet_index >= video_header.packet_count) {
        return false;
    }

    const size_t payload_length = length - sizeof(PacketHeader) - sizeof(VideoPacketHeader);
    const size_t offset = video_header.packet_index * gba_protocol::network::MAX_VIDEO_PACKET_CHUNK_SIZE;
    const size_t expected_length = std::min(
        gba_protocol::network::MAX_VIDEO_PACKET_CHUNK_SIZE,
        FRAME_SIZE - offset
    );
    if (payload_length != expected_length) {
        return false;
    }

    if (!active_ || video_header.frame_id != frame_id_) {
        reset();
        active_ = true;
        frame_id_ = video_header.frame_id;
        packet_count_ = video_header.packet_count;
    } else if (video_header.packet_count != packet_count_) {
        reset();
        return false;
    }

    if (!received_packets_.test(video_header.packet_index)) {
        std::memcpy(frame + offset, packet + sizeof(PacketHeader) + sizeof(VideoPacketHeader), payload_length);
        received_packets_.set(video_header.packet_index);
        ++received_count_;
    }

    if (received_count_ == packet_count_) {
        reset();
        return true;
    }

    return false;
}

void UdpServer::reset() {
    received_packets_.reset();
    frame_id_ = 0;
    packet_count_ = 0;
    received_count_ = 0;
    active_ = false;
}

}