#ifndef STM32X_DETAIL_FLASH_SECTOR_F4_H_
#define STM32X_DETAIL_FLASH_SECTOR_F4_H_

#include <cstdint>

namespace stm32x::detail {

// A lot of boilerplate for what could be a simple table :)

template <uint16_t sector>
constexpr uint32_t SectorSize()
{
  static_assert(sector < 12);
  constexpr uint32_t sector_sizes[] = {16, 16, 16, 16, 64, 128};
  if constexpr (sector > 4)
    return sector_sizes[5] * 1024;
  else
    return sector_sizes[sector] * 1024;
}

template <uint16_t sector>
constexpr uint32_t SectorOffset()
{
  return SectorOffset<sector - 1>() + SectorSize<sector - 1>();
}

template <>
constexpr uint32_t SectorOffset<0>()
{
  return 0;
}

template <uint16_t sector>
constexpr uint32_t SectorBaseAddress()
{
  static_assert(sector < 12);
  return 0x0800'0000 + SectorOffset<sector>();
}

template <uint16_t sector>
struct SectorInfo {
  static constexpr uint32_t BASE = SectorBaseAddress<sector>();
  static constexpr uint32_t SIZE = SectorSize<sector>();
  static constexpr uint32_t END = BASE + SIZE;
  static constexpr uint16_t ID = sector * 8;

  static constexpr bool contains(uint32_t address) { return address >= BASE && address < END; }
};

}  // namespace stm32x::detail

#endif  // STM32X_DETAIL_FLASH_SECTOR_F4_H_
