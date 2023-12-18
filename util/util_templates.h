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
#ifndef STM32X_UTIL_TEMPLATES_H_
#define STM32X_UTIL_TEMPLATES_H_

#include <stdint.h>

#include <type_traits>

namespace util {

// c++20 has it's own __cpp_lib_int_pow2
template <typename T>
constexpr bool has_single_bit(T value) noexcept
{
  return value && !(value & (value - 1));
}

// Converting enums to the underlying type is questionable.
// But if we're going to do it, might as well automatically find the underlying type (easier in
// c++23) and make it optional ("trust me, I know what I'm doing").

template <typename T>
struct is_index_enum {
  static constexpr bool value = false;
};

template <typename E, typename = typename std::enable_if<is_index_enum<E>::value, E>::type>
constexpr typename std::underlying_type<E>::type enum_to_i(E e)
{
  return static_cast<typename std::underlying_type<E>::type>(e);
}

// Assumes the enum has a ::LAST field
template <typename E, typename = typename std::enable_if<std::is_enum<E>::value, E>::type>
constexpr typename std::underlying_type<E>::type enum_count()
{
  return static_cast<typename std::underlying_type<E>::type>(E::LAST);
}

}  // namespace util

#define ENABLE_ENUM_TO_INDEX(enum_type)   \
  template <>                             \
  struct util::is_index_enum<enum_type> { \
    static constexpr bool value = true;   \
  }

#endif  // STM32X_UTIL_TEMPLATES_H_
