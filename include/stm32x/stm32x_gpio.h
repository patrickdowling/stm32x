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

#ifndef STM32X_GPIO_H_
#define STM32X_GPIO_H_

#include "stm32x_model.h"

namespace stm32x {

enum GPIO_PORT : short {
  GPIO_PORT_A,
  GPIO_PORT_B,
  GPIO_PORT_C,
  GPIO_PORT_D,
  GPIO_PORT_E,
  GPIO_PORT_F
};

enum struct GPIO_MODE : uint32_t {
  IN = 0x00,
  OUT = LL_GPIO_MODE_OUTPUT,
  AF = LL_GPIO_MODE_ALTERNATE,
  AN = LL_GPIO_MODE_ANALOG
};

// Speeds can be platform dependent
enum struct GPIO_SPEED : uint32_t {
  LOW = model::GPIO_SPEED::LOW,
  MEDIUM = model::GPIO_SPEED::MEDIUM,
  FAST = model::GPIO_SPEED::FAST,
  FASTEST = model::GPIO_SPEED::FASTEST
};

enum struct GPIO_OTYPE : uint32_t { PP = 0x00, OD = LL_GPIO_OUTPUT_OPENDRAIN };

enum struct GPIO_PUPD : uint32_t {
  NONE = 0x00,
  PULLUP = LL_GPIO_PULL_UP,
  PULLDOWN = LL_GPIO_PULL_DOWN
};

// Can't use the GPIOA, GPIOB et al. GPIO_TypeDef * as template parameter
// Although we could define a constexpr function to return them instead of
// the template approach.
template <GPIO_PORT port>
struct GPIOxREGS {};
template <>
struct GPIOxREGS<GPIO_PORT_A> {
  static constexpr uint32_t REGS = GPIOA_BASE;
  static constexpr uint32_t RCC_PERIPH_MASK = model::RCC_PERIPH_MASK_GPIOA;
};
template <>
struct GPIOxREGS<GPIO_PORT_B> {
  static constexpr uint32_t REGS = GPIOB_BASE;
  static constexpr uint32_t RCC_PERIPH_MASK = model::RCC_PERIPH_MASK_GPIOB;
};
template <>
struct GPIOxREGS<GPIO_PORT_C> {
  static constexpr uint32_t REGS = GPIOC_BASE;
  static constexpr uint32_t RCC_PERIPH_MASK = model::RCC_PERIPH_MASK_GPIOC;
};
template <>
struct GPIOxREGS<GPIO_PORT_D> {
  static constexpr uint32_t REGS = GPIOD_BASE;
  static constexpr uint32_t RCC_PERIPH_MASK = model::RCC_PERIPH_MASK_GPIOD;
};
template <>
struct GPIOxREGS<GPIO_PORT_E> {
  static constexpr uint32_t REGS = GPIOE_BASE;
  static constexpr uint32_t RCC_PERIPH_MASK = model::RCC_PERIPH_MASK_GPIOE;
};
template <>
struct GPIOxREGS<GPIO_PORT_F> {
  static constexpr uint32_t REGS = GPIOF_BASE;
  static constexpr uint32_t RCC_PERIPH_MASK = model::RCC_PERIPH_MASK_GPIOF;
};

template <GPIO_PORT port, uint32_t pin, GPIO_MODE mode, GPIO_SPEED speed, GPIO_OTYPE otype,
          GPIO_PUPD pupd, uint32_t af = 0, bool enable_port_clock = false>
struct GPIO;

// GPIO Port template
template <GPIO_PORT port>
struct GPIOx : public GPIOxImpl<GPIOx<port>> {
  static constexpr uint32_t REGS = GPIOxREGS<port>::REGS;
  static constexpr uint32_t RCC_PERIPH_MASK = GPIOxREGS<port>::RCC_PERIPH_MASK;

  template <uint32_t pin, GPIO_PUPD pupd>
  using GPIO_IN = GPIO<port, pin, GPIO_MODE::IN, GPIO_SPEED::MEDIUM, GPIO_OTYPE::PP, pupd>;

  template <uint32_t pin, GPIO_SPEED speed, GPIO_OTYPE otype, GPIO_PUPD pupd>
  using GPIO_OUT = GPIO<port, pin, GPIO_MODE::OUT, speed, otype, pupd>;

  template <uint32_t pin, GPIO_SPEED speed, GPIO_OTYPE otype, GPIO_PUPD pupd, uint32_t af>
  using GPIO_AF = GPIO<port, pin, GPIO_MODE::AF, speed, otype, pupd, af>;

  template <uint32_t pin>
  using GPIO_AN =
      GPIO<port, pin, GPIO_MODE::AN, GPIO_SPEED::MEDIUM, GPIO_OTYPE::PP, GPIO_PUPD::NONE>;
};

// TODO Only allow set on GPIO_OUT

// GPIO pin definition
// Automatically enables peripheral clock. This leads to some duplication of
// that code (since each pin calls the enable function), so there's some room
// for optimzation. That said, it's not obvious how to ensure the construction
// order in all cases to ensure an automatic central clock enable...
// NOTE: For GPIOs defined through a GPIOx port, the clocks are assumed to be
// enabled elsewhere
template <GPIO_PORT port, uint32_t pin, GPIO_MODE mode, GPIO_SPEED speed, GPIO_OTYPE otype,
          GPIO_PUPD pupd, uint32_t af, bool enable_port_clock>
struct GPIO {
  using PORT = GPIOx<port>;

  static constexpr uint32_t Mask = 0x0001U << pin;
  static constexpr uint32_t Source = pin;
  static constexpr uint32_t Pin = pin;

  static inline void Set() { PORT::Set(Mask); }
  static inline void Reset() { PORT::Reset(Mask); }
  static inline bool Read() { return PORT::Read(Mask); }

  GPIO() { Init(); }

  static void Init()
  {
    if (enable_port_clock) PORT::EnableClock(true);
    if (GPIO_MODE::AF == mode)
      PORT::Init(Mask, mode, speed, otype, pupd, af);
    else
      PORT::Init(Mask, mode, speed, otype, pupd);
  }

  static void ReInit(GPIO_MODE new_mode, GPIO_SPEED new_speed, GPIO_OTYPE new_otype,
                     GPIO_PUPD new_pupd, uint32_t new_af = 0)
  {
    if (GPIO_MODE::AF == new_mode)
      PORT::Init(Mask, new_mode, new_speed, new_otype, new_pupd, new_af);
    else
      PORT::Init(Mask, new_mode, new_speed, new_otype, new_pupd);
  }

  GPIO &operator=(bool set)
  {
    if (set)
      Set();
    else
      Reset();
    return *this;
  }
};

// Standalone GPIO definitions
template <GPIO_PORT port, uint32_t pin, GPIO_SPEED speed, GPIO_OTYPE otype, GPIO_PUPD pupd,
          bool enable_port_clock = true>
using GPIO_OUT = GPIO<port, pin, GPIO_MODE::OUT, speed, otype, pupd, 0, enable_port_clock>;

template <GPIO_PORT port, uint32_t pin, GPIO_PUPD pupd, bool enable_port_clock = true>
using GPIO_IN =
    GPIO<port, pin, GPIO_MODE::IN, GPIO_SPEED::MEDIUM, GPIO_OTYPE::PP, pupd, 0, enable_port_clock>;

template <GPIO_PORT port, uint32_t pin, GPIO_SPEED speed, GPIO_OTYPE otype, GPIO_PUPD pupd,
          uint32_t af, bool enable_port_clock = true>
using GPIO_AF = GPIO<port, pin, GPIO_MODE::AF, speed, otype, pupd, af, enable_port_clock>;

template <GPIO_PORT port, uint32_t pin, bool enable_port_clock = true>
using GPIO_AN = GPIO<port, pin, GPIO_MODE::AN, GPIO_SPEED::MEDIUM, GPIO_OTYPE::PP, GPIO_PUPD::NONE,
                     0, enable_port_clock>;

}  // namespace stm32x

#endif  // STM32X_GPIO_H_
