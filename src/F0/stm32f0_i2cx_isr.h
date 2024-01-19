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
//
// I2Cx ISR/ICR helper, without the hand-holding in stm32f0xx_i2c.c

#ifndef STM32F0_I2CX_ISR_H_
#define STM32F0_I2CX_ISR_H_

#include "stm32f0xx.h"

namespace stm32f0 {

// Due to the declaration of I2C1 et al, we can't use I2Cx as a template
// parameter directly. Hopefully the compiler will realize that it's a static
// location and Do The Right Thing.
template <uint32_t i2cx_base>
class I2CxISR {
public:
  I2CxISR() : ISR(I2Cx()->ISR) {}

  inline static I2C_TypeDef *I2Cx() { return (I2C_TypeDef *)i2cx_base; }

  inline bool GetITStatus(uint32_t it) const { return ISR & it; }

  inline void ClearITPendingBit(uint32_t it) const { I2Cx()->ICR = it; }

  inline uint8_t GetAddressMatched() const
  {
    return (uint8_t)(((uint32_t)ISR & I2C_ISR_ADDCODE) >> 16);
  }

  inline uint16_t GetTransferDirection() const
  {
    return (uint32_t)(ISR & I2C_ISR_DIR) ? I2C_Direction_Receiver : I2C_Direction_Transmitter;
  }

  const uint32_t ISR;
};

}  // namespace stm32f0

#endif  // STM32F0_I2CX_ISR_H_
