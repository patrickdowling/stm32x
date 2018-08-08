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
// I2C slave helper; hold receive and transmitting data buffer as well as some
// minimal state to be able to make decisions based on I2C events.

#ifndef TTIEXP_I2C_SLAVE_H_
#define TTIEXP_I2C_SLAVE_H_

#include <string.h>
#include <stdint.h>
#include "util/util_macros.h"

namespace i2c {

enum EI2CSlaveState {
  I2C_SLAVE_STATE_IDLE,
  I2C_SLAVE_STATE_RX,
  I2C_SLAVE_STATE_TX_PENDING,
  I2C_SLAVE_STATE_TX,
};

template <size_t tx_buffer_size, size_t rx_buffer_size>
class I2CSlaveHelper {
public:
  DISALLOW_COPY_AND_ASSIGN(I2CSlaveHelper);
  I2CSlaveHelper() { }
  ~I2CSlaveHelper() { }

  static const size_t kTxBufferSize = tx_buffer_size;
  static const size_t kRxBufferSize = rx_buffer_size;

  void Init() {
    SetState(I2C_SLAVE_STATE_IDLE);
  }

  inline EI2CSlaveState slave_state() const {
    return slave_state_;
  }

  void Transmit(size_t num_bytes) {
    slave_state_ = I2C_SLAVE_STATE_TX;
    tx_length_ = num_bytes;
    tx_data_ = tx_buffer_;
  }

  inline size_t tx_available() const {
    return tx_length_;
  }

  inline uint8_t *tx_buffer() {
    return tx_buffer_;
  }

  inline uint8_t tx_next_byte() {
    --tx_length_;
    return *tx_data_++;
  }

  void Receive() {
    SetState(I2C_SLAVE_STATE_RX);
  }

  void Receive(uint8_t data) {
    if (rx_data_ < rx_buffer_ + kRxBufferSize) {
      *rx_data_++ = data;
    } else {
      rx_overflow_ = true;
    }
  }

  inline size_t received_bytes() const {
    return rx_data_ - rx_buffer_;
  }

  inline const uint8_t *rx_buffer() const {
    return rx_buffer_;
  }

  void SetState(EI2CSlaveState slave_state) {
    slave_state_ = slave_state;
    tx_length_ = 0;
    tx_data_ = tx_buffer_;
    rx_data_ = rx_buffer_;
    rx_overflow_ = false;

    // doesn't actually reset received data, since that's where the register
    // address for a master read is held
  }

  void Reset() {
    SetState(I2C_SLAVE_STATE_IDLE);
  }

  // Only valid if state is I2C_SLAVE_STATE_TX_PENDING
  inline uint8_t register_address() const {
    return rx_buffer_[0];
  }

  // Return true if in valid state for master read
  inline bool accept_master_read() const {
    return I2C_SLAVE_STATE_TX_PENDING == slave_state_
        || (I2C_SLAVE_STATE_RX == slave_state_ && received_bytes() == 1);
  }

private:
  EI2CSlaveState slave_state_;

  size_t tx_length_;
  uint8_t *tx_data_;
  uint8_t *rx_data_;

  uint8_t tx_buffer_[kTxBufferSize];
  uint8_t rx_buffer_[kRxBufferSize];

  bool rx_overflow_;
};

}; // namespace i2c

#endif // TTIEXP_I2C_SLAVE_H_
