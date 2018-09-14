// Copyright 2018 Patrick Dowling
//
// Author: Patrick Dowling (pld@gurkenkiste.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// See http://creativecommons.org/licenses/MIT/ for more information.
//
#ifndef STM32X_UTIL_CLOCKS_H_
#define STM32X_UTIL_CLOCKS_H_

#include <stdint.h>

namespace stm32x {

template <uint32_t sub_ticks>
class FixedClockDivider {
public:
  FixedClockDivider() : sub_ticks_(0) { }
  ~FixedClockDivider() { }

  bool Tick() {
    uint32_t t = sub_ticks_;
    sub_ticks_ = t ? t - 1 : sub_ticks - 1;
    return 0 == t;
  }

private:
  uint32_t sub_ticks_;
};

template <>
class FixedClockDivider<1> {
public:
  FixedClockDivider() { }
  ~FixedClockDivider() { }
  bool Tick() {
    return true;
  }
};

};

#endif // STM32X_UTIL_CLOCKS_H_
