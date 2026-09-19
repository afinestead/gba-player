#include "frame_buffer.h"

using namespace gba;

extern uint8_t __sframe_buf[];

volatile uint16_t* frame_buf = reinterpret_cast<volatile uint16_t*>(__sframe_buf);

size_t readFrame(volatile uint32_t* base) {
    return 0;
}