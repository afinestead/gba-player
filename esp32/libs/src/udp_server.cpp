#include "udp_server.h"

#include <cstring>

namespace libs::network {

bool UdpServer::receivePacket(
    const uint8_t* packet,
    size_t length,
    uint8_t* frame,
    size_t frame_capacity,
    PacketType& completed_type,
    size_t& completed_length
) {

    ++stats_.packets_received;
    const auto reject_packet = [this]() {
        ++stats_.packets_rejected;
        return false;
    };

    if (frame == nullptr || frame_capacity == 0) {
        if (active_) {
            ++stats_.dropped_frames;
        }
        reset();
        return reject_packet();
    }

    if (packet == nullptr || length < sizeof(PacketHeader)) {
        return reject_packet();
    }

    PacketHeader packet_header{};
    std::memcpy(&packet_header, packet, sizeof(packet_header));
    const PacketType packet_type = static_cast<PacketType>(packet_header.packet_type);
    if (packet_type != VIDEO_FRAME && packet_type != AUDIO_FRAME) {
        return reject_packet();
    }
    if (packet_header.packet_count == 0 || packet_header.packet_count > PACKET_COUNT ||
        packet_header.packet_index >= packet_header.packet_count) {
        return reject_packet();
    }

    const size_t payload_length = length - sizeof(PacketHeader);
    const size_t offset = packet_header.packet_index * MAX_PACKET_CHUNK_SIZE;
    const bool is_last_packet = packet_header.packet_index + 1 == packet_header.packet_count;
    if (payload_length == 0 || payload_length > MAX_PACKET_CHUNK_SIZE ||
        (!is_last_packet && payload_length != MAX_PACKET_CHUNK_SIZE) ||
        offset + payload_length > frame_capacity) {
        return reject_packet();
    }
    const size_t packet_frame_length = offset + payload_length;

    if (!active_ || packet_header.frame_id != frame_id_ || packet_type != packet_type_) {
        if (active_) {
            ++stats_.dropped_frames;
        }
        reset();
        active_ = true;
        frame_id_ = packet_header.frame_id;
        packet_count_ = packet_header.packet_count;
        packet_type_ = packet_type;
        frame_length_ = 0;
    } else if (packet_header.packet_count != packet_count_) {
        reset();
        return reject_packet();
    }

    if (!received_packets_.test(packet_header.packet_index)) {
        std::memcpy(frame + offset, packet + sizeof(PacketHeader), payload_length);
        received_packets_.set(packet_header.packet_index);
        ++received_count_;
        if (is_last_packet) {
            if (frame_length_ != 0 && frame_length_ != packet_frame_length) {
                reset();
                return reject_packet();
            }
            frame_length_ = packet_frame_length;
        }
    } else {
        ++stats_.duplicate_packets;
    }

    if (received_count_ == packet_count_ && frame_length_ != 0) {
        completed_type = packet_type_;
        completed_length = frame_length_;
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
    frame_length_ = 0;
    packet_type_ = PacketType::NONE;
    active_ = false;
}

}
