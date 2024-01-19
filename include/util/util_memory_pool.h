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
// Simple memory pool/buffer chunk manager
// Alloc/Free are not safe for use in different contexts (e.g. ISR)

#ifndef STM32X_UTIL_MEMORY_POOL_H_
#define STM32X_UTIL_MEMORY_POOL_H_

#include <stdint.h>

#include <array>

#include "util_macros.h"

namespace stm32x {

template <size_t buffer_size>
class MemoryPool {
public:
  DELETE_COPY_MOVE(MemoryPool);
  MemoryPool() {}
  ~MemoryPool() {}

  static constexpr size_t kBufferSize = buffer_size;

  // TODO aligned alloc

  inline uint8_t *Alloc(size_t requested_size)
  {
    if (used_ + requested_size > kBufferSize) {
      return nullptr;
    } else {
      uint8_t *p = &buffer_[used_];
      used_ += requested_size;
      return p;
    }
  }

  template <typename T>
  inline T *AllocArray(size_t count)
  {
    return reinterpret_cast<T *>(Alloc(sizeof(T) * count));
  }

  inline void Free() { used_ = 0; }

  size_t size() const { return kBufferSize; }

  size_t available() const { return kBufferSize - used_; }

private:
  size_t used_ = 0;
  std::array<uint8_t, kBufferSize> buffer_;
};

}  // namespace stm32x

#endif  // STM32X_UTIL_MEMORY_POOL_H_
