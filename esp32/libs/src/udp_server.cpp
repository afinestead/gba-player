#include "udp_server.h"

#include <algorithm>
#include <cstring>

#include "protocol.h"

namespace libs::network {

UdpServer::UdpServer(FrameHandler frame_handler, void* context)
    : frame_handler_{frame_handler}, context_{context}
{}

void UdpServer::receivePacket(const uint8_t* packet, size_t length) {
    using gba_protocol::network::PacketHeader;
    using gba_protocol::network::PacketType;
    using gba_protocol::network::VideoPacketHeader;

    if (length < sizeof(PacketHeader)) {
        return;
    }

    PacketHeader packet_header{};
    std::memcpy(&packet_header, packet, sizeof(packet_header));
    if (packet_header.packet_type != PacketType::VIDEO_FRAME ||
        length < sizeof(PacketHeader) + sizeof(VideoPacketHeader)) {
        return;
    }

    VideoPacketHeader video_header{};
    std::memcpy(&video_header, packet + sizeof(PacketHeader), sizeof(video_header));
    if (video_header.packet_count == 0 || video_header.packet_count > FRAME_SIZE ||
        video_header.packet_index >= video_header.packet_count) {
        return;
    }

    const size_t payload_length = length - sizeof(PacketHeader) - sizeof(VideoPacketHeader);
    const size_t chunk_size = (FRAME_SIZE + video_header.packet_count - 1) / video_header.packet_count;
    const size_t offset = video_header.packet_index * chunk_size;
    if (offset >= FRAME_SIZE) {
        return;
    }
    const size_t expected_length = std::min(chunk_size, FRAME_SIZE - offset);
    if (payload_length != expected_length) {
        return;
    }

    if (!active_ || video_header.frame_id != frame_id_) {
        reset();
        active_ = true;
        frame_id_ = video_header.frame_id;
        packet_count_ = video_header.packet_count;
    } else if (video_header.packet_count != packet_count_) {
        reset();
        return;
    }

    const size_t bitmap_index = video_header.packet_index / 8;
    const uint8_t bitmap_mask = static_cast<uint8_t>(1U << (video_header.packet_index % 8));
    if ((received_packets_[bitmap_index] & bitmap_mask) == 0) {
        std::memcpy(frame_.data() + offset, packet + sizeof(PacketHeader) + sizeof(VideoPacketHeader), payload_length);
        received_packets_[bitmap_index] |= bitmap_mask;
        ++received_count_;
    }

    if (received_count_ == packet_count_) {
        if (frame_handler_ != nullptr) {
            frame_handler_(context_, frame_.data(), FRAME_SIZE);
        }
        reset();
    }
}

void UdpServer::reset() {
    received_packets_.fill(0);
    frame_id_ = 0;
    packet_count_ = 0;
    received_count_ = 0;
    active_ = false;
}

}