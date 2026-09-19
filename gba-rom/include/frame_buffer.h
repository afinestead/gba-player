#pragma once

#include "seven/memory.h"
#include "seven/video.h"

namespace gba {

size_t readFrame(volatile uint32_t* base);

};