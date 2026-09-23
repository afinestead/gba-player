#pragma once

#include <cstddef>
#include <cstdint>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

template<typename T, size_t Size>
class Mailbox {
public:
    Mailbox() {
        handle_ = xQueueCreateStatic(Size, sizeof(T), buffer_, &storage_);
    }

    bool put(const T& item, uint32_t duration = 0xffffffff) {
        return xQueueSendToBack(
            handle_, reinterpret_cast<const void*>(&item), duration
        ) == pdPASS;
    }

    bool get(T* item, uint32_t duration = 0xffffffff) {
        return xQueueReceive(
            handle_, reinterpret_cast<void*>(item), duration
        ) == pdPASS;
    }
private:
    StaticQueue_t storage_;
    QueueHandle_t handle_;
    uint8_t buffer_[Size * sizeof(T)];
};
