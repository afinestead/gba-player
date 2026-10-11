#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>

#include "helpers.h"

namespace libs::logging {

static constexpr size_t MESSAGE_MAX_LEN = 144;

struct Message {
    char s[MESSAGE_MAX_LEN]{};
    size_t length = 0;

    void appendText(const char* text, size_t text_length) {
        const size_t available = MESSAGE_MAX_LEN - length - 1;
        const size_t copied = std::min(text_length, available);
        char* end = std::copy(text, text + copied, s + length);
        length = static_cast<size_t>(end - s);
        s[length] = '\0';
    }

    template<size_t Length>
    void appendText(const char (&text)[Length]) {
        appendText(text, Length - 1);
    }

    void appendCount(uint64_t value) {
        char digits[20];
        const int digit_count = helpers::itoa(value, digits);
        appendText(digits, static_cast<size_t>(digit_count));
    }
};

}