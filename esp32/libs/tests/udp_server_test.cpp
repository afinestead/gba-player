#include "udp_server.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <vector>

#include "protocol/command.h"

namespace {

bool receivePacket(
    libs::network::UdpServer& server,
    const uint8_t* packet,
    size_t length,
    uint8_t* frame,
    PacketType* completed_type = nullptr,
    size_t* completed_length = nullptr
) {
    PacketType type = PacketType::NONE;
    size_t frame_length = 0;
    const bool complete = server.receivePacket(
        packet,
        length,
        frame,
        libs::network::FRAME_SIZE,
        type,
        frame_length
    );
    if (complete && completed_type != nullptr) {
        *completed_type = type;
    }
    if (complete && completed_length != nullptr) {
        *completed_length = frame_length;
    }
    return complete;
}

std::vector<uint8_t> makePacket(
    uint32_t frame_id,
    uint16_t packet_index,
    uint8_t value,
    PacketType type = PacketType::VIDEO_FRAME
) {
    using namespace libs::network;
    const size_t offset = packet_index * MAX_PACKET_CHUNK_SIZE;
    const size_t payload_length = std::min(MAX_PACKET_CHUNK_SIZE, FRAME_SIZE - offset);
    std::vector<uint8_t> packet(sizeof(PacketHeader) + payload_length, value);
    const PacketHeader packet_header{type, frame_id, packet_index, FRAME_PACKET_COUNT};
    std::memcpy(packet.data(), &packet_header, sizeof(packet_header));
    return packet;
}

void testAssemblesOutOfOrderAndIgnoresDuplicates() {
    libs::network::UdpServer server;
    std::vector<uint8_t> frame(libs::network::FRAME_SIZE);
    const auto first_packet = makePacket(7, 0, 0xA1);
    const auto second_packet = makePacket(7, 1, 0xB2);

    assert(!receivePacket(server, second_packet.data(), second_packet.size(), frame.data()));
    assert(!receivePacket(server, second_packet.data(), second_packet.size(), frame.data()));

    for (uint16_t packet_index = 2; packet_index < libs::network::FRAME_PACKET_COUNT; ++packet_index) {
        const auto packet = makePacket(7, packet_index, 0xB2);
        assert(!receivePacket(server, packet.data(), packet.size(), frame.data()));
    }

    assert(receivePacket(server, first_packet.data(), first_packet.size(), frame.data()));
    assert(frame.size() == libs::network::FRAME_SIZE);
    for (size_t i = 0; i < libs::network::MAX_PACKET_CHUNK_SIZE; ++i) {
        assert(frame[i] == 0xA1);
    }
    for (size_t i = libs::network::MAX_PACKET_CHUNK_SIZE; i < frame.size(); ++i) {
        assert(frame[i] == 0xB2);
    }

    const auto& stats = server.stats();
    assert(stats.packets_received == libs::network::FRAME_PACKET_COUNT + 1);
    assert(stats.packets_rejected == 0);
    assert(stats.duplicate_packets == 1);
    assert(stats.completed_frames == 1);
    assert(stats.dropped_frames == 0);
    const auto stats_message = server.printStats();
    assert(std::strcmp(stats_message.s, "udp stats rx:25 rej:0 dup:1 done:1 drop:0") == 0);
}

void testNewFrameReplacesIncompleteFrame() {
    libs::network::UdpServer server;
    std::vector<uint8_t> frame(libs::network::FRAME_SIZE);
    const auto old_frame_packet = makePacket(0, 0, 0x10);
    const auto new_frame_first_packet = makePacket(1, 0, 0xA1);

    assert(!receivePacket(server, old_frame_packet.data(), old_frame_packet.size(), frame.data()));

    assert(!receivePacket(server, new_frame_first_packet.data(), new_frame_first_packet.size(), frame.data()));
    for (uint16_t packet_index = 1; packet_index < libs::network::FRAME_PACKET_COUNT; ++packet_index) {
        const auto packet = makePacket(1, packet_index, 0xB2);
        const bool frame_complete = receivePacket(server, packet.data(), packet.size(), frame.data());
        assert(frame_complete == (packet_index + 1 == libs::network::FRAME_PACKET_COUNT));
    }

    assert(frame.size() == libs::network::FRAME_SIZE);
    for (size_t i = 0; i < libs::network::MAX_PACKET_CHUNK_SIZE; ++i) {
        assert(frame[i] == 0xA1);
    }
    for (size_t i = libs::network::MAX_PACKET_CHUNK_SIZE; i < frame.size(); ++i) {
        assert(frame[i] == 0xB2);
    }

    const auto& stats = server.stats();
    assert(stats.completed_frames == 1);
    assert(stats.dropped_frames == 1);
}

void testRejectsShortAndInvalidPackets() {
    libs::network::UdpServer server;
    std::vector<uint8_t> frame(libs::network::FRAME_SIZE);
    const uint8_t short_packet[] = {0x01};
    assert(!receivePacket(server, short_packet, sizeof(short_packet), frame.data()));

    using namespace libs::network;
    const PacketHeader invalid_header{PacketType::VIDEO_FRAME, 9, 2, static_cast<uint16_t>(FRAME_PACKET_COUNT - 1)};
    uint8_t invalid_packet[sizeof(invalid_header)]{};
    std::memcpy(invalid_packet, &invalid_header, sizeof(invalid_header));
    assert(!receivePacket(server, invalid_packet, sizeof(invalid_packet), frame.data()));

    const PacketHeader oversized_chunk_header{PacketType::VIDEO_FRAME, 10, 0, FRAME_PACKET_COUNT};
    uint8_t oversized_chunk_packet[sizeof(oversized_chunk_header) + MAX_PACKET_CHUNK_SIZE + 1]{};
    std::memcpy(oversized_chunk_packet, &oversized_chunk_header, sizeof(oversized_chunk_header));
    assert(!receivePacket(server, oversized_chunk_packet, sizeof(oversized_chunk_packet), frame.data()));

    const auto& stats = server.stats();
    assert(stats.packets_received == 3);
    assert(stats.packets_rejected == 3);
    assert(stats.duplicate_packets == 0);
    assert(stats.completed_frames == 0);
    assert(stats.dropped_frames == 0);
}

void testAssemblesAudioAndReportsItsType() {
    using namespace libs::network;
    UdpServer server;
    std::vector<uint8_t> frame(FRAME_SIZE);
    PacketType completed_type = PacketType::NONE;
    size_t completed_length = 0;
    for (uint16_t packet_index = 0; packet_index < FRAME_PACKET_COUNT; ++packet_index) {
        const auto packet = makePacket(42, packet_index, 0x5A, PacketType::AUDIO_FRAME);
        const bool complete = receivePacket(
            server,
            packet.data(),
            packet.size(),
            frame.data(),
            &completed_type,
            &completed_length
        );
        assert(complete == (packet_index + 1 == FRAME_PACKET_COUNT));
    }
    assert(completed_type == PacketType::AUDIO_FRAME);
    assert(completed_length == FRAME_SIZE);
    assert(server.stats().completed_frames == 1);
}

void testReportsVariableFrameLength() {
    using namespace libs::network;
    UdpServer server;
    std::vector<uint8_t> frame(FRAME_SIZE);
    constexpr size_t payload_length = 73;
    const PacketHeader header{PacketType::VIDEO_FRAME, 99, 0, 1};
    std::vector<uint8_t> packet(sizeof(header) + payload_length, 0xC7);
    std::memcpy(packet.data(), &header, sizeof(header));
    PacketType completed_type = PacketType::NONE;
    size_t completed_length = 0;

    assert(receivePacket(
        server,
        packet.data(),
        packet.size(),
        frame.data(),
        &completed_type,
        &completed_length
    ));
    assert(completed_type == PacketType::VIDEO_FRAME);
    assert(completed_length == payload_length);
    for (size_t i = 0; i < payload_length; ++i) {
        assert(frame[i] == 0xC7);
    }
}

}

int main() {
    testAssemblesOutOfOrderAndIgnoresDuplicates();
    testNewFrameReplacesIncompleteFrame();
    testRejectsShortAndInvalidPackets();
    testAssemblesAudioAndReportsItsType();
    testReportsVariableFrameLength();
}
