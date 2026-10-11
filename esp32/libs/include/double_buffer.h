#pragma once

#include <cstdint>
#include <optional>

#include "mailbox.h"

namespace libs {

template<typename T, size_t Size>
class DoubleBuffer {
public:
    enum class BufferIndex : uint8_t {A, B};

    struct Buffer {
        BufferIndex index;
        T* data;
    };

    DoubleBuffer() {
        free_buf_.put(BufferIndex::A);
        free_buf_.put(BufferIndex::B);
    }

    std::optional<Buffer> acquireFree(uint32_t duration = 0xffffffff) {
        return getBuffer(free_buf_, duration);
    }

    std::optional<Buffer> acquireReady(uint32_t duration = 0xffffffff) {
        return getBuffer(ready_buf_, duration);
    }

    bool publishFree(BufferIndex index) {
        return free_buf_.put(index);
    }

    bool publishReady(BufferIndex index) {
        return ready_buf_.put(index);
    }
private:
    T buffers_[2][Size];

    std::optional<Buffer> getBuffer(Mailbox<BufferIndex, 2>& mailbox, uint32_t duration) {
        BufferIndex index;
        if (mailbox.get(&index, duration)) {
            return Buffer{
                .index = index,
                .data = buffers_[static_cast<uint8_t>(index)]
            };
        }
        return std::nullopt;
    }

    Mailbox<BufferIndex, 2> free_buf_;
    Mailbox<BufferIndex, 2> ready_buf_;
};

}