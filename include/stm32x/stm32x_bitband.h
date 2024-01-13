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
// Experimental and mostly untested bit-banding support for M3/M4 etc.
// In theory, the compiler will reduce these ops to 2 (read) or 3 (write)
// instructions since the addresses are known at compile-time. This seems
// true for gcc 7.2.1 with -O >= 1 at least.
// https://godbolt.org/z/3dAr9g

#ifndef STM32X_BITBAND_H_
#define STM32X_BITBAND_H_

#include "stm32x.h"

#if !(defined STM32X_F37X || defined STM32X_F4XX)
#error "Bit banding not supported"
#endif

namespace stm32x {

namespace bitband {

struct bit {
    const uint32_t ptr_;

    template <typename T>
    inline void operator = (T i) {
        *((__IO uint32_t *)ptr_) = i;
    }

    inline operator uint32_t () const {
        return *((__IO uint32_t *)ptr_);
    }
};

template <uint32_t base, uint32_t bitnum>
inline constexpr bit map_peripheral_bit() {
  static_assert(0 == (base & 0x3), "Non word-aligned base");
  static_assert(bitnum > 0 && bitnum < 32, "32 bits");
  return {
      PERIPH_BB_BASE
      + ((base - PERIPH_BASE) << 5)
      + (bitnum << 2) };
}

template <uint32_t base, uint32_t mask>
inline constexpr bit map_peripheral_mask() {
  static_assert(0 == (base & 0x3), "Non word-aligned base");
  static_assert(mask, "At least one bit needs to be set");
  return {
      PERIPH_BB_BASE
      + ((base - PERIPH_BASE) << 5)
      + (__builtin_ctz(mask) << 2) };
}

// Map directly to bit at address
inline constexpr bit map_peripheral_bit(__IO void *base, uint32_t bitnum) {
  return {
      PERIPH_BB_BASE
      + (((uint32_t)base - PERIPH_BASE) << 5)
      + (bitnum << 2) };
}

// Use lowest non-zero bit in mask for the mapping
inline constexpr bit map_peripheral_mask(__IO void *base, uint32_t mask) {
  return {
      PERIPH_BB_BASE
      + (((uint32_t)base - PERIPH_BASE) << 5)
      + (__builtin_ctz(mask) << 2) };
}

};
} // namespace stm32x

#endif // STM32X_BITBAND_H_
