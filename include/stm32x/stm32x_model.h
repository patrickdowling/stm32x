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

namespace stm32x {

#if defined STM32X_F0XX || defined STM32X_F37X
namespace model {

enum GPIO_SPEED : uint8_t { LOW = 0x0, MEDIUM = 0x1, FAST = 0x1, FASTEST = 0x3 };

static constexpr uint32_t RCC_PERIPH_MASK_GPIOA = RCC_AHBPeriph_GPIOA;
static constexpr uint32_t RCC_PERIPH_MASK_GPIOB = RCC_AHBPeriph_GPIOB;
static constexpr uint32_t RCC_PERIPH_MASK_GPIOC = RCC_AHBPeriph_GPIOC;
static constexpr uint32_t RCC_PERIPH_MASK_GPIOD = RCC_AHBPeriph_GPIOD;
static constexpr uint32_t RCC_PERIPH_MASK_GPIOE = RCC_AHBPeriph_GPIOE;
static constexpr uint32_t RCC_PERIPH_MASK_GPIOF = RCC_AHBPeriph_GPIOF;
}

template <typename base> struct GPIOxImpl {
  static inline void Set(uint16_t mask) ALWAYS_INLINE {
    ((GPIO_TypeDef *)base::REGS)->BSRR = mask;
  }
  static inline void Reset(uint16_t mask) ALWAYS_INLINE {
    ((GPIO_TypeDef *)base::REGS)->BRR = mask;
  }

  static inline bool Read(uint16_t mask) ALWAYS_INLINE {
    return ((GPIO_TypeDef *)base::REGS)->IDR & mask;
  }

  static inline void Init(uint16_t mask, uint16_t pinsource, GPIO_MODE mode, GPIO_SPEED speed, GPIO_OTYPE otype, GPIO_PUPD pupd, uint8_t af) {
    GPIO_PinAFConfig(((GPIO_TypeDef *)base::REGS), pinsource, af);
    Init(mask, mode, speed, otype, pupd);
  }

  static inline void Init(uint16_t mask, GPIO_MODE mode, GPIO_SPEED speed, GPIO_OTYPE otype, GPIO_PUPD pupd) {
    GPIO_InitTypeDef gpio_init;
    gpio_init.GPIO_Pin = mask;
    gpio_init.GPIO_Mode = static_cast<GPIOMode_TypeDef>(mode);
    gpio_init.GPIO_Speed = static_cast<GPIOSpeed_TypeDef>(speed);
    gpio_init.GPIO_OType = static_cast<GPIOOType_TypeDef>(otype);
    gpio_init.GPIO_PuPd = static_cast<GPIOPuPd_TypeDef>(pupd);
    GPIO_Init(((GPIO_TypeDef *)base::REGS), &gpio_init);
  }

  static void EnableClock(bool enable) {
    if (enable)
      RCC->AHBENR |= base::RCC_PERIPH_MASK;
    else
      RCC->AHBENR &= ~base::RCC_PERIPH_MASK;
  }
};

#elif defined STM32X_F4XX
namespace model {

// Helper to declare variable in CCM memory, pre-loaded from flash at startup
#define INCCM __attribute__ ((section(".ccm")))

// Helper to declare variable in CCM memory that is zeroed at startup
#define INCCMZ __attribute__ ((section(".ccmz")))

enum GPIO_SPEED : uint8_t { LOW = 0x0, MEDIUM = 0x1, FAST = 0x2, FASTEST = 0x3 };

static constexpr uint32_t RCC_PERIPH_MASK_GPIOA = RCC_AHB1Periph_GPIOA;
static constexpr uint32_t RCC_PERIPH_MASK_GPIOB = RCC_AHB1Periph_GPIOB;
static constexpr uint32_t RCC_PERIPH_MASK_GPIOC = RCC_AHB1Periph_GPIOC;
static constexpr uint32_t RCC_PERIPH_MASK_GPIOD = RCC_AHB1Periph_GPIOD;
static constexpr uint32_t RCC_PERIPH_MASK_GPIOE = RCC_AHB1Periph_GPIOE;
static constexpr uint32_t RCC_PERIPH_MASK_GPIOF = RCC_AHB1Periph_GPIOF;
}

template <typename base> struct GPIOxImpl {
  static inline void Set(uint16_t mask) ALWAYS_INLINE {
    ((GPIO_TypeDef *)base::REGS)->BSRRL = mask;
  }
  static inline void Reset(uint16_t mask) ALWAYS_INLINE {
    ((GPIO_TypeDef *)base::REGS)->BSRRH = mask;
  }

  static inline bool Read(uint16_t mask) ALWAYS_INLINE {
    return ((GPIO_TypeDef *)base::REGS)->IDR & mask;
  }

  static inline void Init(uint16_t mask, uint16_t pinsource, GPIO_MODE mode, GPIO_SPEED speed, GPIO_OTYPE otype, GPIO_PUPD pupd, uint8_t af) {
    GPIO_PinAFConfig(((GPIO_TypeDef *)base::REGS), pinsource, af);
    Init(mask, mode, speed, otype, pupd);
  }

  static inline void Init(uint16_t mask, GPIO_MODE mode, GPIO_SPEED speed, GPIO_OTYPE otype, GPIO_PUPD pupd) {
    GPIO_InitTypeDef gpio_init;
    gpio_init.GPIO_Pin = mask;
    gpio_init.GPIO_Mode = static_cast<GPIOMode_TypeDef>(mode);
    gpio_init.GPIO_Speed = static_cast<GPIOSpeed_TypeDef>(speed);
    gpio_init.GPIO_OType = static_cast<GPIOOType_TypeDef>(otype);
    gpio_init.GPIO_PuPd = static_cast<GPIOPuPd_TypeDef>(pupd);
    GPIO_Init(((GPIO_TypeDef *)base::REGS), &gpio_init);
  }

  static void EnableClock(bool enable) {
    if (enable)
      RCC->AHB1ENR |= base::RCC_PERIPH_MASK;
    else
      RCC->AHB1ENR &= ~base::RCC_PERIPH_MASK;
  }
};
#endif

} // namespace stm32x

#endif // STM32X_MODEL_H_
