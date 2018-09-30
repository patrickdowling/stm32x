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
// Various run-time debug utilities

#ifndef STM32X_DEBUG_H_
#define STM32X_DEBUG_H_

#if defined STM32X_F0XX
#error "DEBUG INTERFACE NOT SUPPORTED ON M0"
#else
#include "stm32x.h"
#include "stm32x_math.h"

namespace stm32x {

class Debug {
public:
  static void Init() {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    ITM->LAR = 0xC5ACCE55;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  }
};

static inline uint32_t cycles_to_us(uint32_t cycles) {
  return multiply_u32xu32_rshift32(cycles, (1ULL << 32) / (F_CPU / 1000000));
}

#define STM32X_DEBUG_INIT() stm32x::Debug::Init()

class CycleMeasurement {
public:
  DISALLOW_COPY_AND_ASSIGN(CycleMeasurement);

  CycleMeasurement() : start_(DWT->CYCCNT) { }

  uint32_t read() const {
    return DWT->CYCCNT - start_;
  }

private:
  uint32_t start_;
};

class AveragedCycles {
public:
  static constexpr uint32_t kSmoothing = 8;

  AveragedCycles() : value_(0) { }

  uint32_t value() const {
    return value_;
  }

  uint32_t value_in_us() const {
    return cycles_to_us(value_);
  }

  void Reset() {
    value_ = 0;
  }

  void Push(uint32_t cycles) {
    value_ = (value_ * (kSmoothing - 1) + cycles) / kSmoothing;
  }

private:
  uint32_t value_;
};

class ScopedCycleMeasurement {
public:
  ScopedCycleMeasurement(AveragedCycles &dest)
  : dest_(dest)
  { }

  ~ScopedCycleMeasurement() {
    dest_.Push(cycles_.read());
  }

private:
  AveragedCycles &dest_;
  CycleMeasurement cycles_;
};

}; // namespace stm32x

#endif
#endif // STM32X_DEBUG_H_
