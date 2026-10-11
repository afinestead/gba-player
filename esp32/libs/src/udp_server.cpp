#include "udp_server.h"

#include <algorithm>
#include <cstring>

#include "protocol.h"

namespace libs::network {

bool UdpServer::receivePacket(const uint8_t* packet, size_t length, uint8_t* frame) {
    using gba_protocol::network::PacketHeader;
    using gba_protocol::network::PacketType;
    using gba_protocol::network::VideoPacketHeader;

    ++stats_.packets_received;
    const auto reject_packet = [this]() {
        ++stats_.packets_rejected;
        return false;
    };

    if (frame == nullptr) {
        if (active_) {
            ++stats_.dropped_frames;
        }
        reset();
        return reject_packet();
    }

    if (length < sizeof(PacketHeader)) {
        return reject_packet();
    }

    PacketHeader packet_header{};
    std::memcpy(&packet_header, packet, sizeof(packet_header));
    if (packet_header.packet_type != PacketType::VIDEO_FRAME ||
        length < sizeof(PacketHeader) + sizeof(VideoPacketHeader)) {
        return reject_packet();
    }

    VideoPacketHeader video_header{};
    std::memcpy(&video_header, packet + sizeof(PacketHeader), sizeof(video_header));
    if (video_header.packet_count != PACKET_COUNT ||
        video_header.packet_index >= video_header.packet_count) {
        return reject_packet();
    }

    const size_t payload_length = length - sizeof(PacketHeader) - sizeof(VideoPacketHeader);
    const size_t offset = video_header.packet_index * gba_protocol::network::MAX_VIDEO_PACKET_CHUNK_SIZE;
    const size_t expected_length = std::min(
        gba_protocol::network::MAX_VIDEO_PACKET_CHUNK_SIZE,
        FRAME_SIZE - offset
    );
    if (payload_length != expected_length) {
        return reject_packet();
    }

    if (!active_ || video_header.frame_id != frame_id_) {
        if (active_) {
            ++stats_.dropped_frames;
        }
        reset();
        active_ = true;
        frame_id_ = video_header.frame_id;
        packet_count_ = video_header.packet_count;
    } else if (video_header.packet_count != packet_count_) {
        reset();
        return reject_packet();
    }

    if (!received_packets_.test(video_header.packet_index)) {
        std::memcpy(frame + offset, packet + sizeof(PacketHeader) + sizeof(VideoPacketHeader), payload_length);
        received_packets_.set(video_header.packet_index);
        ++received_count_;
    } else {
        ++stats_.duplicate_packets;
    }

    if (received_count_ == packet_count_) {
        ++stats_.completed_frames;
        reset();
        return true;
    }

    return false;
}

libs::logging::Message UdpServer::printStats() const {
    libs::logging::Message message{};
    message.appendText("udp stats rx:");
    message.appendCount(stats_.packets_received);
    message.appendText(" rej:");
    message.appendCount(stats_.packets_rejected);
    message.appendText(" dup:");
    message.appendCount(stats_.duplicate_packets);
    message.appendText(" done:");
    message.appendCount(stats_.completed_frames);
    message.appendText(" drop:");
    message.appendCount(stats_.dropped_frames);
    return message;
}

void UdpServer::reset() {
    received_packets_.reset();
    frame_id_ = 0;
    packet_count_ = 0;
    received_count_ = 0;
    active_ = false;
}

}