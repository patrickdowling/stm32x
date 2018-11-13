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
// -----------------------------------------------------------------------------
// Debounced switch

#ifndef STM32X_SWITCH_H_
#define STM32X_SWITCH_H_

#include <stdint.h>

namespace stm32x {

class SwitchState {
public:
  SwitchState() { Init(); }
  ~SwitchState() { }

  void Init() {
    state_ = 0xff;
  }

  template <typename GPIO>
  void Poll() {
    state_ = state_ << 1 | GPIO::Read();
  }

  template <typename GPIO>
  void Poll(GPIO &gpio) {
    state_ = state_ << 1 | gpio.Read();
  }

  inline bool pressed() const {
    return state_ == 0x00;
  }

  inline bool just_pressed() const {
    return state_ == 0x80;
  }

  inline bool released() const {
    return state_ == 0x7f;
  }

  template <typename GPIO>
  bool read_immediate() const {
    return !GPIO::Read();
  }

private:
  uint8_t state_;
};

} // namespace stm32x

#endif // STM32X_SWITCH_H_
