// Copyright (c) 2018-2023 Patrick Dowling
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
// Misc useful macros & things, some for historical reasons.

#ifndef STM32X_UTIL_MACROS_H_
#define STM32X_UTIL_MACROS_H_

#include <cinttypes>
#include <cstring>

#define DELETE_COPY_ASSIGN(classname)    \
  classname(const classname &) = delete; \
  void operator=(const classname &) = delete

#define DELETE_MOVE_ASSIGN(classname) \
  classname(classname &&) = delete;   \
  void operator=(classname &&) = delete

#define DELETE_COPY_MOVE(classname) \
  DELETE_COPY_ASSIGN(classname);    \
  DELETE_MOVE_ASSIGN(classname)

#define CLIP16(x)             \
  if (x < -32767) x = -32768; \
  if (x > 32767) x = 32767;

#define CONSTRAIN(x, lb, ub) \
  do {                       \
    if (x < (lb))            \
      x = lb;                \
    else if (x > (ub))       \
      x = ub;                \
  } while (0)

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

#define CONCAT(x, y) _CONCAT(x, y)
#define _CONCAT(x, y) x##y

#ifndef SWAP
#define SWAP(a, b) \
  do {             \
    auto t = a;    \
    a = b;         \
    b = t;         \
  } while (0)
#endif

#define ALWAYS_INLINE __attribute__((always_inline))

#endif  // STM32X_UTIL_MACROS_H_
