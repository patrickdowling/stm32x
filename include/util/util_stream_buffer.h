// Copyright 2018-2024 Patrick Dowling
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
#ifndef UTIL_STREAM_BUFFER_H_
#define UTIL_STREAM_BUFFER_H_

#include <stdint.h>

#include <algorithm>
#include <cstring>
#include <type_traits>

#include "util_fourcc.h"

namespace util {

class StreamBufferWriter {
public:
  StreamBufferWriter(uint8_t *buffer, size_t buffer_length)
      : begin_(buffer), cursor_(buffer), end_(buffer + buffer_length)
  {}

  template <typename T /*, class = typename std::enable_if<!std::is_integral<T>::value>::type*/>
  void Write(const T &t)
  {
    static_assert(std::is_pod<T>::value, "POD expected");
    if (sizeof(T) > available()) {
      cursor_ = end_;
      overflow_ = true;
    } else {
      memcpy(cursor_, &t, sizeof(T));
      cursor_ += sizeof(T);
    }
  }

  void Write(const void *data, size_t len)
  {
    if (len > available()) {
      len = available();
      overflow_ = true;
    }
    memcpy(cursor_, data, len);
    cursor_ += len;
  }

  size_t written() const { return cursor_ - begin_; }

  size_t available() const { return end_ - cursor_; }

  bool overflow() const { return overflow_; }

private:
  uint8_t *begin_;
  uint8_t *cursor_;
  uint8_t *end_;
  bool overflow_ = false;
};

class StreamBufferReader {
public:
  StreamBufferReader(const uint8_t *buffer, size_t buffer_length)
      : begin_(buffer), cursor_(buffer), end_(buffer + buffer_length)
  {}

  template <typename T>
  T Read()
  {
    static_assert(std::is_integral<T>::value, "Integral type expected");
    T t = 0;
    return Read(t);
  }

  template <typename T>
  T &Read(T &t)
  {
    static_assert(std::is_pod<T>::value, "POD expected");
    if (sizeof(T) > available()) {
      cursor_ = end_;
      underflow_ = true;
    } else {
      memcpy(&t, cursor_, sizeof(T));
      cursor_ += sizeof(T);
    }
    return t;
  }

  auto Read(void *dst, size_t len)
  {
    if (len > available()) {
      len = available();
      underflow_ = true;
    }
    memcpy(dst, cursor_, len);
    cursor_ += len;
    return len;
  }

  size_t read() const { return cursor_ - begin_; }

  size_t available() const { return end_ - cursor_; }

  size_t underflow() const { return underflow_; }

private:
  const uint8_t *begin_;
  const uint8_t *cursor_;
  const uint8_t *end_;

  bool underflow_ = false;
};

template <>
inline void StreamBufferWriter::Write(const FOURCC &fourcc)
{
  Write(fourcc.value);
}

template <>
inline FOURCC StreamBufferReader::Read<FOURCC>()
{
  return FOURCC{Read<FOURCC::value_type>()};
}

};  // namespace util

#endif  // UTIL_STREAM_BUFFER_H_
