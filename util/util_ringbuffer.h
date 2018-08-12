// Copyright (c) 2018 Patrick Dowling
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
//
// Ring buffer implementation
// There seem to be two types of common ring buffer implementations:
// One that doesn't use the last item in order to distinguish between "empty"
// and "full" states (e.g. MI stmlib/utils/ring_buffer.h). The other relies on
// wrapping read/write heads and appears to use all available items.
// This implements the latter.
// - Pretty much assumes single producer/consumer
// - Assume size is pow2

#ifndef STM32X_UTIL_RINGBUFFER_H_
#define STM32X_UTIL_RINGBUFFER_H_

#include <stdint.h>
#include "util/util_macros.h"

namespace util {

template <typename T, size_t size>
class RingBuffer {
public:
  DISALLOW_COPY_AND_ASSIGN(RingBuffer);

  RingBuffer() { }
//  ~RingBuffer() { }

  void Init() {
    write_ptr_ = read_ptr_ = 0;
  }

  inline size_t readable() const {
    return write_ptr_ - read_ptr_;
  }

  inline size_t writeable() const {
    return size - readable();
  }

  inline T Read() {
    size_t read_ptr = read_ptr_;
    T value = buffer_[read_ptr & (size - 1)];
    read_ptr_ = read_ptr + 1;
    return value;
  }

  inline T Peek() const {
    return buffer_[read_ptr_ & (size - 1)];
  }

  inline void Write(T value) {
    size_t write_ptr = write_ptr_;
    buffer_[write_ptr & (size - 1)] = value;
    write_ptr_ = write_ptr + 1;
  }

  inline void Flush() {
    write_ptr_ = read_ptr_ = 0;
  }

  template <class... Args>
  inline void EmplaceWrite(Args&&... args) {
    size_t write_ptr = write_ptr_;
    buffer_[write_ptr_ & (size -1)] = T{args...};
    write_ptr_ = write_ptr + 1;
  }

private:

  T buffer_[size];
  volatile size_t write_ptr_;
  volatile size_t read_ptr_;
};

}; // namespace util

#endif // STM32X_UTIL_RINGBUFFER_H_
