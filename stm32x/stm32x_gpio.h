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
  GPIO_PORT_A, GPIO_PORT_B, GPIO_PORT_C, GPIO_PORT_D, GPIO_PORT_E
};

enum struct GPIO_MODE : uint8_t {
  IN = 0x00, OUT = 0x01, AF = 0x02, AN = 0x03
};

enum struct GPIO_SPEED : uint8_t {
  LOW = 0x00, MEDIUM = 0x01, FAST = 0x2, FASTEST = 0x03
};

enum struct GPIO_OTYPE : uint8_t {
  PP = 0x00, OD = 0x01
};

enum struct GPIO_PUPD : uint8_t {
  NONE = 0x00, PULLUP = 0x01, PULLDOWN = 0x02
};

// Can't use the GPIOA, GPIOB et al. GPIO_TypeDef * as template parameter
template <GPIO_PORT port> struct GPIOxREGS { };
template <> struct GPIOxREGS<GPIO_PORT_A> { static constexpr GPIO_TypeDef *REGS = GPIOA; };
template <> struct GPIOxREGS<GPIO_PORT_B> { static constexpr GPIO_TypeDef *REGS = GPIOB; };
template <> struct GPIOxREGS<GPIO_PORT_C> { static constexpr GPIO_TypeDef *REGS = GPIOC; };
template <> struct GPIOxREGS<GPIO_PORT_D> { static constexpr GPIO_TypeDef *REGS = GPIOD; };
template <> struct GPIOxREGS<GPIO_PORT_E> { static constexpr GPIO_TypeDef *REGS = GPIOE; };


template <GPIO_PORT port>
struct GPIOx : public GPIOxImpl<GPIOx<port>> {
  static constexpr GPIO_TypeDef *REGS = GPIOxREGS<port>::REGS;

};

template <GPIO_PORT port, uint16_t pin, GPIO_MODE mode, GPIO_SPEED speed, GPIO_OTYPE otype, GPIO_PUPD pupd, uint8_t af = 0>
struct GPIO {
  using PORT = GPIOx<port>;

  static constexpr uint16_t Mask = 0x0001U << pin;
  static constexpr uint16_t Source = pin;

  static inline void Set() { PORT::Set(Mask); }
  static inline void Reset() { PORT::Reset(Mask); }
  static inline bool Read() { return PORT::Read(Mask); }

  static void Init() {
    if (GPIO_MODE::AF == mode)
      PORT::Init(Mask, Source, mode, speed, otype, pupd, af);
    else
      PORT::Init(Mask, mode, speed, otype, pupd);
  }
};

template <GPIO_PORT port, uint16_t pin, GPIO_SPEED speed, GPIO_OTYPE otype, GPIO_PUPD pupd>
using GPIO_OUT = GPIO<port, pin, GPIO_MODE::OUT, speed, otype, pupd>;

template <GPIO_PORT port, uint16_t pin, GPIO_SPEED speed, GPIO_OTYPE otype, GPIO_PUPD pupd, uint8_t af>
using GPIO_AF  = GPIO<port, pin, GPIO_MODE::AF, speed, otype, pupd, af>;

template <GPIO_PORT port, uint16_t pin>
using GPIO_AN  = GPIO<port, pin, GPIO_MODE::AN, GPIO_SPEED::LOW, GPIO_OTYPE::PP, GPIO_PUPD::PULLUP>;

}; // namespace stm32x

#endif // STM32X_GPIO_H_
