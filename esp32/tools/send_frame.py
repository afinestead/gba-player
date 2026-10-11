#!/usr/bin/env python3
"""Send raw video frame data to the ESP32 UDP server."""

import argparse
import socket
import struct
import time
from pathlib import Path
from typing import Iterator


FRAME_SIZE = 32 * 1024
MAX_VIDEO_PACKET_CHUNK_SIZE = 1400
VIDEO_FRAME_PACKET_COUNT = (FRAME_SIZE + MAX_VIDEO_PACKET_CHUNK_SIZE - 1) // MAX_VIDEO_PACKET_CHUNK_SIZE
PACKET_TYPE_VIDEO_FRAME = 1
PACKET_HEADER = struct.Struct("<B")
VIDEO_FRAME_HEADER = struct.Struct("<IHH")


def make_packets(frame: bytes, frame_id: int) -> Iterator[bytes]:
    if len(frame) != FRAME_SIZE:
        raise ValueError(f"frame must be exactly {FRAME_SIZE} bytes, got {len(frame)}")
    if not 0 <= frame_id <= 0xFFFFFFFF:
        raise ValueError("frame ID must fit in an unsigned 32-bit integer")

    packet_type = PACKET_HEADER.pack(PACKET_TYPE_VIDEO_FRAME)
    for packet_index in range(VIDEO_FRAME_PACKET_COUNT):
        offset = packet_index * MAX_VIDEO_PACKET_CHUNK_SIZE
        payload = frame[offset:offset + MAX_VIDEO_PACKET_CHUNK_SIZE]
        video_header = VIDEO_FRAME_HEADER.pack(
            frame_id,
            packet_index,
            VIDEO_FRAME_PACKET_COUNT,
        )
        yield packet_type + video_header + payload


def make_test_frame() -> bytes:
    return bytes((index % 255) + 1 for index in range(FRAME_SIZE))


def send_packets(packets: Iterator[bytes], send_packet, interval_ns: int) -> tuple[int, int]:
    deadline = time.perf_counter_ns()
    late_packets = 0
    max_lateness_ns = 0

    for packet in packets:
        now = time.perf_counter_ns()
        while now < deadline:
            time.sleep((deadline - now) / 1_000_000_000)
            now = time.perf_counter_ns()

        if now > deadline:
            lateness_ns = now - deadline
            late_packets += 1
            max_lateness_ns = max(max_lateness_ns, lateness_ns)

        send_packet(packet)
        deadline += interval_ns

    return late_packets, max_lateness_ns


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("frame", nargs="?", type=Path, help="optional raw frame file (exactly 32768 bytes)")
    parser.add_argument("--host", required=True, help="ESP32 IP address or hostname")
    parser.add_argument("--port", type=int, default=5000, help="UDP port (default: 5000)")
    parser.add_argument("--frame-id", type=int, default=1, help="unsigned frame ID (default: 1)")
    parser.add_argument("--nframes", type=int, default=1, help="number of frames to send (default: 1)")
    parser.add_argument(
        "--packet-delay",
        type=float,
        default=0.002,
        help="delay between UDP packets in seconds (default: 0.002; use 0 to disable)",
    )
    args = parser.parse_args()

    if args.nframes < 1:
        parser.error("--nframes must be at least 1")
    if args.packet_delay < 0:
        parser.error("--packet-delay cannot be negative")
    if not 0 <= args.frame_id <= 0xFFFFFFFF:
        parser.error("--frame-id must fit in an unsigned 32-bit integer")
    if args.frame_id + args.nframes - 1 > 0xFFFFFFFF:
        parser.error("the frame ID range exceeds the unsigned 32-bit limit")

    frame = args.frame.read_bytes() if args.frame else make_test_frame()
    packet_interval_ns = round(args.packet_delay * 1_000_000_000)

    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as udp_socket:
        packets = (
            packet
            for frame_offset in range(args.nframes)
            for packet in make_packets(frame, args.frame_id + frame_offset)
        )
        late_packets, max_lateness_ns = send_packets(
            packets,
            lambda packet: udp_socket.sendto(packet, (args.host, args.port)),
            packet_interval_ns,
        )

    source = str(args.frame) if args.frame else "generated test pattern"
    final_frame_id = args.frame_id + args.nframes - 1
    print(
        f"Sent {args.nframes} frame(s) from {source} "
        f"(IDs {args.frame_id}-{final_frame_id}, {VIDEO_FRAME_PACKET_COUNT} packets each) "
        f"to {args.host}:{args.port}"
    )
    if packet_interval_ns:
        print(
            f"Pacing: {args.packet_delay:g} s interval; "
            f"{late_packets} late packet(s), max lateness {max_lateness_ns / 1_000_000:.3f} ms"
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())