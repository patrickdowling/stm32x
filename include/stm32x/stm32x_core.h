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
#ifndef STM32X_CORE_H_
#define STM32X_CORE_H_

#include "stm32x.h"

namespace stm32x {
// GPIO forward declarations
enum GPIO_PORT : short;
enum struct GPIO_MODE : uint8_t;
enum struct GPIO_SPEED : uint8_t;
enum struct GPIO_OTYPE : uint8_t;
enum struct GPIO_PUPD : uint8_t;

// Core timing and other functionality common to all platforms
class Core {
public:
  Core() = default;
  DELETE_COPY_MOVE(Core);

  void Init(uint32_t systick_ticks);

  void Tick() {
    ++ticks_;
  }

  inline uint32_t now() const volatile {
    return ticks_;
  }

  void Delay(uint32_t ticks) {
    const uint32_t start = now();
    while ((now() - start) < ticks) { }
  }

private:
  volatile uint32_t ticks_ = 0;
};

extern Core core;
} // namespace stm32x

#define STM32X_CORE_DEFINE(attr) namespace stm32x { stm32x::Core core attr; }
#define STM32X_CORE_INIT(systick_ticks) do { stm32x::core.Init(systick_ticks); } while (0)
#define STM32X_CORE_TICK() do { stm32x::core.Tick(); } while (0)
#define STM32X_CORE_NOW() stm32x::core.now()

#include "stm32x_model.h"
#include "stm32x_gpio.h"

#endif // STM32X_CORE_H_
