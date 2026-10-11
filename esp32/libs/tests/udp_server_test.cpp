#include "udp_server.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <vector>

#include "protocol.h"

namespace {

std::vector<uint8_t> makePacket(uint32_t frame_id, uint16_t packet_index, uint8_t value) {
    using namespace gba_protocol::network;
    const size_t offset = packet_index * MAX_VIDEO_PACKET_CHUNK_SIZE;
    const size_t payload_length = std::min(MAX_VIDEO_PACKET_CHUNK_SIZE, FRAME_SIZE - offset);
    const size_t header_length = sizeof(PacketHeader) + sizeof(VideoPacketHeader);
    std::vector<uint8_t> packet(header_length + payload_length, value);
    const PacketHeader packet_header{PacketType::VIDEO_FRAME};
    const VideoPacketHeader video_header{frame_id, packet_index, VIDEO_FRAME_PACKET_COUNT};
    std::memcpy(packet.data(), &packet_header, sizeof(packet_header));
    std::memcpy(packet.data() + sizeof(packet_header), &video_header, sizeof(video_header));
    return packet;
}

void testAssemblesOutOfOrderAndIgnoresDuplicates() {
    libs::network::UdpServer server;
    std::vector<uint8_t> frame(gba_protocol::network::FRAME_SIZE);
    const auto first_packet = makePacket(7, 0, 0xA1);
    const auto second_packet = makePacket(7, 1, 0xB2);

    assert(!server.receivePacket(second_packet.data(), second_packet.size(), frame.data()));
    assert(!server.receivePacket(second_packet.data(), second_packet.size(), frame.data()));

    for (uint16_t packet_index = 2; packet_index < gba_protocol::network::VIDEO_FRAME_PACKET_COUNT; ++packet_index) {
        const auto packet = makePacket(7, packet_index, 0xB2);
        assert(!server.receivePacket(packet.data(), packet.size(), frame.data()));
    }

    assert(server.receivePacket(first_packet.data(), first_packet.size(), frame.data()));
    assert(frame.size() == gba_protocol::network::FRAME_SIZE);
    for (size_t i = 0; i < gba_protocol::network::MAX_VIDEO_PACKET_CHUNK_SIZE; ++i) {
        assert(frame[i] == 0xA1);
    }
    for (size_t i = gba_protocol::network::MAX_VIDEO_PACKET_CHUNK_SIZE; i < frame.size(); ++i) {
        assert(frame[i] == 0xB2);
    }

    const auto& stats = server.stats();
    assert(stats.packets_received == gba_protocol::network::VIDEO_FRAME_PACKET_COUNT + 1);
    assert(stats.packets_rejected == 0);
    assert(stats.duplicate_packets == 1);
    assert(stats.completed_frames == 1);
    assert(stats.dropped_frames == 0);
    const auto stats_message = server.printStats();
    assert(std::strcmp(stats_message.s, "udp stats rx:25 rej:0 dup:1 done:1 drop:0") == 0);
}

void testNewFrameReplacesIncompleteFrame() {
    libs::network::UdpServer server;
    std::vector<uint8_t> frame(gba_protocol::network::FRAME_SIZE);
    const auto old_frame_packet = makePacket(0, 0, 0x10);
    const auto new_frame_first_packet = makePacket(1, 0, 0xA1);

    assert(!server.receivePacket(old_frame_packet.data(), old_frame_packet.size(), frame.data()));

    assert(!server.receivePacket(new_frame_first_packet.data(), new_frame_first_packet.size(), frame.data()));
    for (uint16_t packet_index = 1; packet_index < gba_protocol::network::VIDEO_FRAME_PACKET_COUNT; ++packet_index) {
        const auto packet = makePacket(1, packet_index, 0xB2);
        const bool frame_complete = server.receivePacket(packet.data(), packet.size(), frame.data());
        assert(frame_complete == (packet_index + 1 == gba_protocol::network::VIDEO_FRAME_PACKET_COUNT));
    }

    assert(frame.size() == gba_protocol::network::FRAME_SIZE);
    for (size_t i = 0; i < gba_protocol::network::MAX_VIDEO_PACKET_CHUNK_SIZE; ++i) {
        assert(frame[i] == 0xA1);
    }
    for (size_t i = gba_protocol::network::MAX_VIDEO_PACKET_CHUNK_SIZE; i < frame.size(); ++i) {
        assert(frame[i] == 0xB2);
    }

    const auto& stats = server.stats();
    assert(stats.completed_frames == 1);
    assert(stats.dropped_frames == 1);
}

void testRejectsShortAndInvalidPackets() {
    libs::network::UdpServer server;
    std::vector<uint8_t> frame(gba_protocol::network::FRAME_SIZE);
    const uint8_t short_packet[] = {0x01};
    assert(!server.receivePacket(short_packet, sizeof(short_packet), frame.data()));

    using namespace gba_protocol::network;
    const PacketHeader packet_header{PacketType::VIDEO_FRAME};
    const VideoPacketHeader invalid_header{9, 2, static_cast<uint16_t>(VIDEO_FRAME_PACKET_COUNT - 1)};
    uint8_t invalid_packet[sizeof(packet_header) + sizeof(invalid_header)]{};
    std::memcpy(invalid_packet, &packet_header, sizeof(packet_header));
    std::memcpy(invalid_packet + sizeof(packet_header), &invalid_header, sizeof(invalid_header));
    assert(!server.receivePacket(invalid_packet, sizeof(invalid_packet), frame.data()));

    const VideoPacketHeader oversized_chunk_header{10, 0, VIDEO_FRAME_PACKET_COUNT};
    uint8_t oversized_chunk_packet[
        sizeof(packet_header) + sizeof(oversized_chunk_header) + MAX_VIDEO_PACKET_CHUNK_SIZE + 1
    ]{};
    std::memcpy(oversized_chunk_packet, &packet_header, sizeof(packet_header));
    std::memcpy(
        oversized_chunk_packet + sizeof(packet_header),
        &oversized_chunk_header,
        sizeof(oversized_chunk_header)
    );
    assert(!server.receivePacket(oversized_chunk_packet, sizeof(oversized_chunk_packet), frame.data()));

    const auto& stats = server.stats();
    assert(stats.packets_received == 3);
    assert(stats.packets_rejected == 3);
    assert(stats.duplicate_packets == 0);
    assert(stats.completed_frames == 0);
    assert(stats.dropped_frames == 0);
}

}

int main() {
    testAssemblesOutOfOrderAndIgnoresDuplicates();
    testNewFrameReplacesIncompleteFrame();
    testRejectsShortAndInvalidPackets();
}