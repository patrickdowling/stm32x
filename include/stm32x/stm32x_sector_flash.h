// Copyright 2024 Patrick Dowling
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
// Flash storage implementation for sector-based flash
// Assumes we're using a single sector, which may or may not pan out.

#ifndef STM32X_SECTOR_FLASH_H_
#define STM32X_SECTOR_FLASH_H_

#ifndef STM32X_F4XX
#error "SECTOR STORAGE NOT SUPPORTED"
#endif

#include <cstdint>

namespace stm32x {

class FlashStorageBase {
public:
  static void Init([[maybe_unused]] uint16_t version) {}

  static void Unlock() { FLASH_Unlock(); }
  static void Lock() { FLASH_Lock(); }

  static bool ProgramWord(uint32_t address, uint32_t data)
  {
    return FLASH_COMPLETE == FLASH_ProgramWord(address, data);
  }

  static bool ProgramHalfWord(uint32_t address, uint16_t data)
  {
    return FLASH_COMPLETE == FLASH_ProgramHalfWord(address, data);
  }

  inline static uint32_t Map(uint32_t address) { return address; }
};

template <uint32_t sector>
class FlashStorage : public FlashStorageBase {
  // static constexpr uint32_t PAGE_SIZE = FLASH_PAGE_SIZE;
  static constexpr uint32_t ALIGNMENT = 4;

  static bool ErasePage(uint32_t page_address)
  {
    return FLASH_COMPLETE == FLASH_EraseSector(page_address, VoltageRange_3);
  }
};

}  // namespace stm32x

#endif  // STM32X_SECTOR_FLASH_H_
