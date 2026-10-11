#pragma once

#include "double_buffer.h"
#include "protocol.h"

namespace libs::frame_buffer {

using FrameBuffer = DoubleBuffer<uint8_t, gba_protocol::network::FRAME_SIZE>;

};