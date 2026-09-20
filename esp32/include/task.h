#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace task {

using TaskFunction = void (*)(void* params);

template<size_t Size>
class Task {
public:
    Task(const char* name, const uint8_t priority)
        : name_{name}, priority_{priority}
    {}

    void init() {
        task_handle_ = xTaskCreateStatic(
            task_entry,
            name_,
            Size,
            this,
            priority_,
            task_stack_,
            &task_buf_
        );
    }

    void notify(uint32_t bits) {
        xTaskNotify(task_handle_, bits, eSetBits);
    }

    uint32_t wait(uint32_t duration = 0xffffffff) {
        uint32_t event;
        xTaskNotifyWait(0, 0xffffffff, &event, duration);
        return event;
    }

protected:
    virtual void start() = 0;
    
private:
    const char* name_;
    const uint8_t priority_;
    StaticTask_t task_buf_;
    StackType_t task_stack_[Size];
    TaskHandle_t task_handle_;


    static void task_entry(void* params)
    {
        auto* self = static_cast<Task*>(params);
        self->start();
    }
};

};