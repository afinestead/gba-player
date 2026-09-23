#pragma once

#include "mailbox.h"
#include "esp_attr.h"


namespace frame_buffer {

class FrameBufferManager {
public:
    static constexpr size_t FRAME_SIZE = 64 * 1024;
    static constexpr size_t NUM_BUFFERS = 3;

    void init();

private:
    DMA_ATTR static uint8_t frame_buffers[NUM_BUFFERS][FRAME_SIZE];

    Mailbox<uint8_t*, NUM_BUFFERS> ready_mailbox_;
    Mailbox<uint8_t*, NUM_BUFFERS> free_mailbox_;


};

};