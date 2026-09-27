#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace task {

using TaskFunction = void (*)(void* params);

enum class Priority : uint8_t {
    IDLE,
    SPI,
    GBA,
    WIFI,
    NETWORK,
    SERIAL,
};

template<size_t Size>
class Task {
public:
    Task(const char* name, const Priority priority)
        : name_{name}, priority_{priority}
    {}

    void init() {
        task_handle_ = xTaskCreateStatic(
            task_entry,
            name_,
            Size,
            this,
            static_cast<uint8_t>(priority_),
            task_stack_,
            &task_buf_
        );
    }

    void notify(uint32_t bits) {
        xTaskNotify(task_handle_, bits, eSetBits);
    }

    uint32_t wait(uint32_t duration = 0xffffffff) {
        uint32_t event;
        xTaskNotifyWait(0, 0xffffffff, &event, duration / portTICK_PERIOD_MS);
        return event;
    }

    void delay(uint32_t duration) {
        vTaskDelay(duration / portTICK_PERIOD_MS);
    }

protected:
    virtual void start() = 0;
    
private:
    const char* name_;
    const Priority priority_;
    StaticTask_t task_buf_;
    StackType_t task_stack_[Size];
    TaskHandle_t task_handle_;


    static void task_entry(void* params)
    {
        auto* self = static_cast<Task*>(params);
        self->task_handle_ = xTaskGetCurrentTaskHandle();
        self->start();
    }
};

};