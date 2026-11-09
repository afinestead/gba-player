#pragma once

#include "double_buffer.h"
#include "udp_server.h"

namespace libs::frame_buffer {

using FrameBuffer = DoubleBuffer<uint8_t, libs::network::FRAME_SIZE>;

};