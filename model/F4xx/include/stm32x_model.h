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
#ifndef STM32X_MODEL_H_
#define STM32X_MODEL_H_

#include "stm32f4xx.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32x/stm32x.h"

namespace stm32x {

namespace model {

// Helper to declare variable in CCM memory, pre-loaded from flash at startup
#define INCCM __attribute__((section(".ccm")))

// Helper to declare variable in CCM memory that is zeroed at startup
#define INCCMZ __attribute__((section(".ccmz")))

enum GPIO_SPEED : uint8_t {
  LOW = LL_GPIO_SPEED_FREQ_LOW,
  MEDIUM = LL_GPIO_SPEED_FREQ_MEDIUM,
  FAST = LL_GPIO_SPEED_FREQ_HIGH,
  FASTEST = LL_GPIO_SPEED_FREQ_VERY_HIGH
};

static constexpr uint32_t RCC_PERIPH_MASK_GPIOA = LL_AHB1_GRP1_PERIPH_GPIOA;
static constexpr uint32_t RCC_PERIPH_MASK_GPIOB = LL_AHB1_GRP1_PERIPH_GPIOB;
static constexpr uint32_t RCC_PERIPH_MASK_GPIOC = LL_AHB1_GRP1_PERIPH_GPIOC;
static constexpr uint32_t RCC_PERIPH_MASK_GPIOD = LL_AHB1_GRP1_PERIPH_GPIOD;
static constexpr uint32_t RCC_PERIPH_MASK_GPIOE = LL_AHB1_GRP1_PERIPH_GPIOE;
static constexpr uint32_t RCC_PERIPH_MASK_GPIOF = LL_AHB1_GRP1_PERIPH_GPIOF;
}  // namespace model

template <typename base>
struct GPIOxImpl {
  static inline void Set(uint32_t mask) ALWAYS_INLINE { ((GPIO_TypeDef *)base::REGS)->BSRR = mask; }
  static inline void Reset(uint32_t mask) ALWAYS_INLINE
  {
    ((GPIO_TypeDef *)base::REGS)->BSRR = mask << 16;
  }

  static inline bool Read(uint16_t mask) ALWAYS_INLINE
  {
    return ((GPIO_TypeDef *)base::REGS)->IDR & mask;
  }

  static inline void Init(uint32_t mask, GPIO_MODE mode, GPIO_SPEED speed, GPIO_OTYPE otype,
                          GPIO_PUPD pupd, uint8_t af)
  {
    LL_GPIO_InitTypeDef gpio_init;
    gpio_init.Pin = mask;
    gpio_init.Mode = static_cast<uint32_t>(mode);
    gpio_init.Speed = static_cast<uint32_t>(speed);
    gpio_init.OutputType = static_cast<uint32_t>(otype);
    gpio_init.Pull = static_cast<uint32_t>(pupd);
    gpio_init.Alternate = af;
    LL_GPIO_Init(((GPIO_TypeDef *)base::REGS), &gpio_init);
  }

  static inline void Init(uint32_t mask, GPIO_MODE mode, GPIO_SPEED speed, GPIO_OTYPE otype,
                          GPIO_PUPD pupd)
  {
    LL_GPIO_InitTypeDef gpio_init;
    gpio_init.Pin = mask;
    gpio_init.Mode = static_cast<uint32_t>(mode);
    gpio_init.Speed = static_cast<uint32_t>(speed);
    gpio_init.OutputType = static_cast<uint32_t>(otype);
    gpio_init.Pull = static_cast<uint32_t>(pupd);
    gpio_init.Alternate = LL_GPIO_AF_0;
    LL_GPIO_Init(((GPIO_TypeDef *)base::REGS), &gpio_init);
  }

  static void EnableClock(bool enable)
  {
    if (enable)
      RCC->AHB1ENR |= base::RCC_PERIPH_MASK;
    else
      RCC->AHB1ENR &= ~base::RCC_PERIPH_MASK;
  }
};

}  // namespace stm32x

#endif  // STM32X_MODEL_H_
