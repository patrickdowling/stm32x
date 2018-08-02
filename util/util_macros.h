// Copyright (c) 2018 Patrick Dowling
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// -----------------------------------------------------------------------------
//
// Misc useful macros & things

#ifndef STM32F0_UTIL_MACROS_H_
#define STM32F0_UTIL_MACROS_H_

#include <stdint.h>
#include <string.h>

#define DISALLOW_COPY_AND_ASSIGN(classname) \
  classname(const classname&) = delete; \
  void operator = (const classname&) = delete

#define CLIP16(x) if (x < -32767) x = -32768; if (x > 32767) x = 32767;

#define CONSTRAIN(x, lb, ub) do { if (x < (lb)) x = lb; else if (x > (ub)) x = ub; } while (0)

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

template <uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3>
struct FOURCC {
	static constexpr uint32_t value = (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
};

#define CONCAT(x, y) _CONCAT(x, y)
#define _CONCAT(x, y) x##y

#endif // STM32F0_UTIL_MACROS_H_
