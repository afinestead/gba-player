#pragma once

#include <cstdint>

namespace helpers {

// Source - https://stackoverflow.com/a/12386915
// Posted by Minh Nguyen, modified by community. See post 'Timeline' for change history
// Retrieved 2026-09-25, License - CC BY-SA 4.0

// Yet, another good itoa implementation
// returns: the length of the number string
inline int itoa(int value, char *sp, int radix = 10)
{
    char tmp[16];// be careful with the length of the buffer
    char *tp = tmp;
    int i;
    unsigned v;

    int sign = (radix == 10 && value < 0);    
    if (sign)
        v = -value;
    else
        v = (unsigned)value;

    while (v || tp == tmp)
    {
        i = v % radix;
        v /= radix;
        if (i < 10)
          *tp++ = i+'0';
        else
          *tp++ = i + 'a' - 10;
    }

    int len = tp - tmp;

    if (sign) 
    {
        *sp++ = '-';
        len++;
    }

    while (tp > tmp)
        *sp++ = *--tp;

    return len;
}

inline int itoa(uint64_t value, char *sp, int radix = 10)
{
    char tmp[65];
    char *tp = tmp;

    do {
        const auto digit = value % static_cast<uint64_t>(radix);
        value /= static_cast<uint64_t>(radix);
        *tp++ = digit < 10 ? static_cast<char>(digit + '0') : static_cast<char>(digit + 'a' - 10);
    } while (value);

    const int len = static_cast<int>(tp - tmp);
    while (tp > tmp) {
        *sp++ = *--tp;
    }

    return len;
}

}