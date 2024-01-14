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
// NOTE we don't care about banks (yet?)

#ifndef STM32X_SECTOR_FLASH_H_
#define STM32X_SECTOR_FLASH_H_

#ifndef STM32X_F4XX
#error "SECTOR STORAGE NOT SUPPORTED"
#endif

#include <cstdint>

#include "detail/flash_sector_f4xx.h"
#include "stm32f4xx_flash.h"

namespace stm32x {

class FlashStorageBase {
public:
  static void Init([[maybe_unused]] uint16_t version) {}

  static void Unlock() { FLASH_Unlock(); }
  static void Lock() { FLASH_Lock(); }
};

template <uint16_t sector, bool enable_checks>
class FlashStorage : public FlashStorageBase {
public:
  using SECTOR = detail::SectorInfo<sector>;

  static constexpr uint32_t PAGE_SIZE = SECTOR::SIZE;
  static constexpr uint32_t ALIGNMENT = 4;

  static bool ErasePage(uint32_t page_address)
  {
    if constexpr (enable_checks)
      if (page_address != SECTOR::BASE) return FLASH_ERROR_PROGRAM;

    return FLASH_COMPLETE == FLASH_EraseSector(SECTOR::ID, VoltageRange_3);
  }

  static bool ProgramWord(uint32_t address, uint32_t data)
  {
    if constexpr (enable_checks)
      if (!SECTOR::contains(address)) return FLASH_ERROR_PROGRAM;
    return FLASH_COMPLETE == FLASH_ProgramWord(address, data);
  }

  constexpr static uint32_t Map(uint32_t address) { return address; }
};

}  // namespace stm32x

#endif  // STM32X_SECTOR_FLASH_H_
