// Copyright 2023 Patrick Dowling
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

#ifndef STM32X_GPIO_UTIL_H_
#define STM32X_GPIO_UTIL_H_

#include <cinttypes>
#include <type_traits>

namespace stm32x {

#if __cpp_fold_expressions
template <typename... Pins>
inline void Init()
{
  (Pins::Init(), ...);
}

template <typename... Pins>
inline void Set()
{
  (Pins::Set(), ...);
}

template <typename... Pins>
inline void Reset()
{
  (Pins::Reset(), ...);
}

template <typename... Pins>
constexpr uint16_t PinMask()
{
  return (Pins::Mask | ...);
}

template <typename Pin, typename... Pins>
struct same_port
    : public std::bool_constant<(std::is_same_v<typename Pin::PORT, typename Pins::PORT> && ...)> {
};

// TODO Ordering? We're assuming all pins are adjacent for set.
template <typename Pin, typename... Pins>
struct PinGroup {
  static_assert(same_port<Pin, Pins...>::value);
  using PORT = typename Pin::PORT;

  static constexpr uint16_t SHIFT = Pin::Pin;
  static constexpr uint16_t MASK = stm32x::PinMask<Pin, Pins...>();

  static void Init() { stm32x::Init<Pin, Pins...>(); }

  static void Reset() { PORT::Reset(MASK); }

  template <typename T>
  static void Set(T bits)
  {
    PORT::Reset(MASK);
    PORT::Set((bits << SHIFT) & MASK);
  }
};
#endif

}  // namespace stm32x

#endif  // STM32X_GPIO_UTIL_H_
