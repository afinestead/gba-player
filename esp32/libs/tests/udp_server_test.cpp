#include "udp_server.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <vector>

#include "protocol.h"

namespace {

struct CapturedFrame {
    size_t callback_count = 0;
    std::vector<uint8_t> bytes;
};

void captureFrame(void* context, const uint8_t* frame, size_t length) {
    auto* captured = static_cast<CapturedFrame*>(context);
    ++captured->callback_count;
    captured->bytes.assign(frame, frame + length);
}

std::vector<uint8_t> makePacket(uint32_t frame_id, uint16_t packet_index, uint16_t packet_count, uint8_t value) {
    using namespace gba_protocol::network;
    const size_t chunk_size = (gba_protocol::network::FRAME_SIZE + packet_count - 1) / packet_count;
    const size_t offset = packet_index * chunk_size;
    const size_t payload_length = std::min(chunk_size, gba_protocol::network::FRAME_SIZE - offset);
    const size_t header_length = sizeof(PacketHeader) + sizeof(VideoPacketHeader);
    std::vector<uint8_t> packet(header_length + payload_length, value);
    const PacketHeader packet_header{PacketType::VIDEO_FRAME};
    const VideoPacketHeader video_header{frame_id, packet_index, packet_count};
    std::memcpy(packet.data(), &packet_header, sizeof(packet_header));
    std::memcpy(packet.data() + sizeof(packet_header), &video_header, sizeof(video_header));
    return packet;
}

void testAssemblesOutOfOrderAndIgnoresDuplicates() {
    CapturedFrame captured;
    libs::network::UdpServer server{captureFrame, &captured};
    const auto first_packet = makePacket(7, 0, 2, 0xA1);
    const auto second_packet = makePacket(7, 1, 2, 0xB2);

    server.receivePacket(second_packet.data(), second_packet.size());
    server.receivePacket(second_packet.data(), second_packet.size());
    assert(captured.callback_count == 0);

    server.receivePacket(first_packet.data(), first_packet.size());
    assert(captured.callback_count == 1);
    assert(captured.bytes.size() == gba_protocol::network::FRAME_SIZE);
    for (size_t i = 0; i < captured.bytes.size() / 2; ++i) {
        assert(captured.bytes[i] == 0xA1);
    }
    for (size_t i = captured.bytes.size() / 2; i < captured.bytes.size(); ++i) {
        assert(captured.bytes[i] == 0xB2);
    }
}

void testNewFrameReplacesIncompleteFrame() {
    CapturedFrame captured;
    libs::network::UdpServer server{captureFrame, &captured};
    const auto old_frame_packet = makePacket(0, 0, 2, 0x10);
    const auto new_frame_first_packet = makePacket(1, 0, 2, 0xA1);
    const auto new_frame_second_packet = makePacket(1, 1, 2, 0xB2);

    server.receivePacket(old_frame_packet.data(), old_frame_packet.size());
    assert(captured.callback_count == 0);

    server.receivePacket(new_frame_first_packet.data(), new_frame_first_packet.size());
    server.receivePacket(new_frame_second_packet.data(), new_frame_second_packet.size());

    assert(captured.callback_count == 1);
    assert(captured.bytes.size() == gba_protocol::network::FRAME_SIZE);
    for (size_t i = 0; i < captured.bytes.size() / 2; ++i) {
        assert(captured.bytes[i] == 0xA1);
    }
    for (size_t i = captured.bytes.size() / 2; i < captured.bytes.size(); ++i) {
        assert(captured.bytes[i] == 0xB2);
    }
}

void testRejectsShortAndInvalidPackets() {
    CapturedFrame captured;
    libs::network::UdpServer server{captureFrame, &captured};
    const uint8_t short_packet[] = {0x01};
    server.receivePacket(short_packet, sizeof(short_packet));

    using namespace gba_protocol::network;
    const PacketHeader packet_header{PacketType::VIDEO_FRAME};
    const VideoPacketHeader invalid_header{9, 2, 2};
    uint8_t invalid_packet[sizeof(packet_header) + sizeof(invalid_header)]{};
    std::memcpy(invalid_packet, &packet_header, sizeof(packet_header));
    std::memcpy(invalid_packet + sizeof(packet_header), &invalid_header, sizeof(invalid_header));
    server.receivePacket(invalid_packet, sizeof(invalid_packet));

    const VideoPacketHeader invalid_offset_header{10, 16384, 20000};
    uint8_t invalid_offset_packet[sizeof(packet_header) + sizeof(invalid_offset_header) + 2]{};
    std::memcpy(invalid_offset_packet, &packet_header, sizeof(packet_header));
    std::memcpy(invalid_offset_packet + sizeof(packet_header), &invalid_offset_header, sizeof(invalid_offset_header));
    server.receivePacket(invalid_offset_packet, sizeof(invalid_offset_packet));

    assert(captured.callback_count == 0);
}

}

int main() {
    testAssemblesOutOfOrderAndIgnoresDuplicates();
    testNewFrameReplacesIncompleteFrame();
    testRejectsShortAndInvalidPackets();
}