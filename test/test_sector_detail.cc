#include "fmt/core.h"
#include "gtest/gtest.h"
#include "stm32x/detail/flash_sector_f4xx.h"

namespace stm32x::test {

template <uint32_t sector>
void TestSector(uint32_t expected_base)
{
  using SECTOR = detail::SectorInfo<sector>;

  auto base = SECTOR::BASE;
  auto size = SECTOR::SIZE;
  fmt::println("{:2} {:08x} {}", sector, base, size);

  EXPECT_EQ(expected_base, base);
  EXPECT_TRUE(SECTOR::contains(base));
  EXPECT_TRUE(SECTOR::contains(base + 1));
  EXPECT_TRUE(SECTOR::contains(base + size - 1));
  EXPECT_FALSE(SECTOR::contains(base + size));
}

TEST(TestSectorDetailF4, BaseAddress)
{
  TestSector<0>(0x0800'0000);
  TestSector<1>(0x0800'4000);
  TestSector<2>(0x0800'8000);
  TestSector<3>(0x0800'C000);
  TestSector<4>(0x0801'0000);
  TestSector<5>(0x0802'0000);
  TestSector<6>(0x0804'0000);
  TestSector<11>(0x080E'0000);
}

}  // namespace stm32x::test
