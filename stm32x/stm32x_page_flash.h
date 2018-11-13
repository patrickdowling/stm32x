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
// Flash storage implementation for page-based flash

#ifndef STM32X_PAGE_FLASH_H_
#define STM32X_PAGE_FLASH_H_

// NOTE: Some variants of the F0 series might support 2K page size also
#if defined STM32X_F0XX
#define FLASH_PAGE_SIZE 0x400
#elif defined STM32X_F37X
#define FLASH_PAGE_SIZE 0x800
#else
#error "PAGE STORAGE NOT SUPPORTED"
#endif

#include "stm32x.h"

namespace stm32x {

class FlashStorage {
public:
  static constexpr uint32_t PAGE_SIZE = FLASH_PAGE_SIZE;
  static constexpr uint32_t ALIGNMENT = 4;

  static void Init(uint16_t version);

  static void Unlock() {
    FLASH_Unlock();
  }

  static void Lock() {
    return FLASH_Lock();
  }

  static bool ErasePage(uint32_t page_address) {
    return FLASH_COMPLETE == FLASH_ErasePage(page_address);
  }

  static bool ProgramWord(uint32_t address, uint32_t data) {
    return FLASH_COMPLETE == FLASH_ProgramWord(address, data);
  }

  static bool ProgramHalfWord(uint32_t address, uint16_t data) {
    return FLASH_COMPLETE == FLASH_ProgramHalfWord(address, data);
  }

  static uint16_t CalcCRC16(const void *data, uint32_t length) {
    CRC->CR |= CRC_CR_RESET;
    length >>= 1;
    const uint16_t *src = static_cast<const uint16_t *>(data);
    while (length--)
       *(uint16_t*)(CRC_BASE) = *src++;
    return CRC->DR;
  }

  inline static uint32_t Map(uint32_t address) {
    return address;
  }
};

} // namespace stm32x

#endif // STM32X_PAGE_FLASH_H_
