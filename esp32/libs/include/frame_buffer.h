#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>

#include "mailbox.h"
#include "udp_server.h"
#include "protocol/command.h"

namespace libs {

template<typename T, size_t Size>
class FrameBuffer {
public:
    enum class BufferIndex : uint8_t {A, B};

    struct Buffer {
        BufferIndex index;
        T* data_;

        T* payload() {
            return reinterpret_cast<T*>(
                reinterpret_cast<uint8_t*>(data_) + sizeof(ProtocolHeader)
            );
        }

        size_t len() {
            return static_cast<size_t>(header()->length) + sizeof(ProtocolHeader);
        }

        ProtocolHeader* header() {
            return reinterpret_cast<ProtocolHeader*>(data_);
        }
    };

    FrameBuffer() {
        static_assert(sizeof(T) == 1, "FrameBuffer stores byte-addressed payloads");
        free_buf_.put(BufferIndex::A);
        free_buf_.put(BufferIndex::B);
    }

    static constexpr size_t size() {
        return payloadSize() + sizeof(ProtocolHeader);
    }

    static constexpr size_t payloadSize() {
        return Size * sizeof(T);
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
    alignas(uint32_t) T buffers_[2][Size + sizeof(ProtocolHeader)];

    std::optional<Buffer> getBuffer(Mailbox<BufferIndex, 2>& mailbox, uint32_t duration) {
        BufferIndex index;
        if (mailbox.get(&index, duration)) {
            return Buffer{
                .index = index,
                .data_ = buffers_[static_cast<uint8_t>(index)]
            };
        }
        return std::nullopt;
    }

    Mailbox<BufferIndex, 2> free_buf_;
    Mailbox<BufferIndex, 2> ready_buf_;
};

}
