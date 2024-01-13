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
#ifndef STM32X_UTIL_GATE_STATE_H_
#define STM32X_UTIL_GATE_STATE_H_

namespace stm32x {

struct GateState {
  enum State : uint8_t { LOW = 0x0, RISING = 0x1, RAISED = 0x2, FALLING = 0x4 };

  void Update(bool state)
  {
    if (state_ & RAISED) {
      state_ = state ? RAISED : FALLING;
    } else {
      state_ = state ? (RISING | RAISED) : LOW;
    }
  }

  inline bool raised() const { return state_ & RAISED; }

  inline bool rising() const { return state_ & RAISED; }

  inline bool falling() const { return state_ & FALLING; }

  uint8_t state_;
};

}  // namespace stm32x

#endif  // STM32X_UTIL_GATE_STATE_H_
