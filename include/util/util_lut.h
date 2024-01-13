// Copyright (c) 2023 Patrick Dowling
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// -----------------------------------------------------------------------------
#ifndef STM32X_UTIL_CONSTEXPR_LUT_H
#define STM32X_UTIL_CONSTEXPR_LUT_H

#include <array>
#include <utility>

// Live dangerously, constexpr tables only work for gcc where powf, sinf et al. are constexpr.
// In a project one could also generate tables as python resource, but for tests and/or to compile
// with clang this just wraps things to be const.

#ifdef __clang__
#define LUT_GENERATOR_CONSTEXPR
#define LUT_CONSTEXPR const
#else
#define LUT_GENERATOR_CONSTEXPR constexpr
#define LUT_CONSTEXPR constexpr
#endif

namespace util {

// Trying to generalize LUT generators with templates. "Works for select use cases"
// and it not very well tested.
//
// TODO Better type handling, there are still some hidden conversions.
// TODO Traits instead of a bunch of template parameters?
// TODO enable_if interpolation
// TODO Also how to handle a fixed-point version. Perhaps templating the interpolation would help.
template <typename value_type, size_t N, typename index_type = value_type, size_t table_padding = 0>
struct LookupTable {
  static constexpr size_t kTableSize = N;
  static constexpr size_t kArraySize = N + table_padding;

  constexpr auto size() const { return N; }

  std::array<value_type, kArraySize> data_;

  constexpr auto read_interpolated(index_type index) const
  {
    static_assert(std::is_floating_point_v<index_type>);
    index *= kTableSize;
    auto i = static_cast<size_t>(index);
    auto a = data_[i];
    auto b = data_[i + 1];
    return a + (b - a) * (index - static_cast<float>(i));
  }

  constexpr auto read(index_type index) const
  {
    index = std::clamp<index_type>(index, 0, kArraySize);
    return data_[index];
  }

  constexpr auto operator[](size_t index) const { return data_[index]; }

  template <typename G>
  static constexpr LookupTable generate(G g)
  {
    return table_generator(g, std::make_index_sequence<kArraySize>());
  }

  template <typename G, size_t... Is>
  static constexpr auto table_generator(G g, std::index_sequence<Is...>)
  {
    return LookupTable{std::array<value_type, kArraySize>{g(Is, kTableSize)...}};
  }
};

}  // namespace util

#endif  // STM32X_UTIL_CONSTEXPR_LUT_H
