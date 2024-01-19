#include <array>

#include "fmt/core.h"
#include "gtest/gtest.h"
#include "util/util_storage.h"

namespace stm32x::test {

static constexpr uint8_t kFenceValue = 0xAA;

template <uint32_t num_pages, uint32_t page_size, uint32_t alignment>
class StorageImpl {
public:
  static constexpr uint32_t PAGE_SIZE = page_size;
  static constexpr uint32_t ALIGNMENT = alignment;
  static constexpr uint32_t BASE = page_size;

  static constexpr uint32_t LENGTH = num_pages * page_size;
  static constexpr uint32_t kTotalSize = page_size + LENGTH + page_size;

  static void Init(uint16_t version)
  {
    fmt::println("PAGE_SIZE={} LENGTH={}", PAGE_SIZE, LENGTH);
    auto fence_value = kFenceValue;
    std::fill(kFenceArray.begin(), kFenceArray.end(), kFenceValue);
    std::fill(FLASH.begin(), FLASH.begin() + PAGE_SIZE, fence_value);
    std::fill(FLASH.begin() + PAGE_SIZE, FLASH.begin() + PAGE_SIZE + LENGTH, 0xdd);
    std::fill(FLASH.begin() + PAGE_SIZE + LENGTH, FLASH.end(), fence_value);
    VERSION = version;
  }

  static void Unlock() { locked = false; }

  static void Lock() { locked = true; }

  static bool ErasePage(uint32_t page_address)
  {
    fmt::println("ErasePage({:08X}={})", page_address, page_address);
    EXPECT_EQ(0, page_address % PAGE_SIZE);
    EXPECT_FALSE(locked);
    std::fill(&FLASH[page_address], &FLASH[page_address + PAGE_SIZE], 0xff);
    return true;
  }

  static bool ProgramWord(uint32_t address, uint32_t data)
  {
    EXPECT_TRUE(address >= BASE);
    EXPECT_TRUE(address < PAGE_SIZE + LENGTH);
    EXPECT_FALSE(locked);

    uint32_t *dst = reinterpret_cast<uint32_t *>(&FLASH[address]);
    EXPECT_EQ(0xffffffff, *dst);
    *dst = data;
    return true;
  }

  static void *Map(uint32_t address)
  {
    EXPECT_TRUE(address >= BASE);
    EXPECT_TRUE(address < PAGE_SIZE + LENGTH);
    return &FLASH[address];
  }

  static std::array<uint8_t, kTotalSize> FLASH;
  static std::array<uint8_t, PAGE_SIZE> kFenceArray;
  static uint16_t VERSION;
  static bool locked;

  static void CheckFences()
  {
    EXPECT_EQ(0, std::memcmp(&FLASH[0], &kFenceArray[0], PAGE_SIZE));
    EXPECT_EQ(0, std::memcmp(&FLASH[PAGE_SIZE + LENGTH], &kFenceArray[0], PAGE_SIZE));
  }
};

template <uint32_t num_pages, uint32_t page_size, uint32_t alignment>
std::array<uint8_t, StorageImpl<num_pages, page_size, alignment>::kTotalSize>
    StorageImpl<num_pages, page_size, alignment>::FLASH;

template <uint32_t num_pages, uint32_t page_size, uint32_t alignment>
std::array<uint8_t, StorageImpl<num_pages, page_size, alignment>::PAGE_SIZE>
    StorageImpl<num_pages, page_size, alignment>::kFenceArray;

template <uint32_t num_pages, uint32_t page_size, uint32_t alignment>
uint16_t StorageImpl<num_pages, page_size, alignment>::VERSION = 0;

template <uint32_t num_pages, uint32_t page_size, uint32_t alignment>
bool StorageImpl<num_pages, page_size, alignment>::locked = true;

struct StorageData {
  static constexpr util::FOURCC STORAGE_TYPE_ID = "TEST"_4CC;
  static constexpr uint16_t STORAGE_VERSION = 0x1234;

  std::array<int32_t, 4> values = {};
};

bool operator==(const StorageData &lhs, const StorageData &rhs)
{
  return lhs.values == rhs.values;
}

template <typename test_params>
class TestStorage : public ::testing::Test {
public:
  static constexpr uint32_t kStorageLength = test_params::kNumPages * test_params::kPageSize;
  static constexpr uint32_t kStorageEnd = test_params::kPageSize + kStorageLength;

  using ThisStorageImpl = StorageImpl<test_params::kNumPages, test_params::kPageSize, 4>;
  using ThisStorage = util::Storage<kStorageEnd, kStorageLength, ThisStorageImpl, StorageData>;

  virtual void SetUp() {}

  virtual void TearDown() {}

protected:
  ThisStorage storage;
};
TYPED_TEST_SUITE_P(TestStorage);

TYPED_TEST_P(TestStorage, Basics)
{
  StorageData data;
  std::fill(data.values.begin(), data.values.end(), 0xFF);

  EXPECT_FALSE(this->storage.Load(data));
  EXPECT_EQ(0, this->storage.generation());
  for (auto v : data.values) EXPECT_EQ(0xff, v);

  std::fill(data.values.begin(), data.values.end(), 0x1234);

  EXPECT_TRUE(this->storage.Save(data));
  EXPECT_EQ(1, this->storage.generation());

  TestFixture::ThisStorageImpl::CheckFences();

  this->storage.Reset();
  StorageData loaded_data;
  EXPECT_TRUE(this->storage.Load(loaded_data));
  EXPECT_EQ(1, this->storage.generation());
  EXPECT_EQ(data, loaded_data);
}

TYPED_TEST_P(TestStorage, Wrap)
{
  StorageData data;

  EXPECT_FALSE(this->storage.Load(data));
  EXPECT_EQ(0, this->storage.generation());

  uint32_t i = 0xDEADBEEF;
  static constexpr uint32_t num_blocks = TestFixture::ThisStorage::kNumBlocks;
  for (uint32_t block = 0; block < num_blocks; ++block, ++i) {
    std::fill(data.values.begin(), data.values.end(), i);
    EXPECT_TRUE(this->storage.Save(data));
    EXPECT_EQ(block + 1, this->storage.generation());
  }

  std::fill(data.values.begin(), data.values.end(), i);
  EXPECT_TRUE(this->storage.Save(data));
  EXPECT_EQ(1, this->storage.generation());

  this->storage.Reset();
  StorageData loaded_data;
  EXPECT_TRUE(this->storage.Load(loaded_data));
  EXPECT_EQ(1, this->storage.generation());
  EXPECT_EQ(data, loaded_data);
}

// gtest doesn't provide an obvious way to define tests that use a template
// value rather than a type, so this is a bit of a roundabout way using a
// traits struct instead.
template <uint32_t pages, uint32_t page_size>
struct TestParam {
  static constexpr uint32_t kNumPages = pages;
  static constexpr uint32_t kPageSize = page_size;
};

REGISTER_TYPED_TEST_SUITE_P(TestStorage, Basics, Wrap);
using TestTypes = ::testing::Types<TestParam<1, 1024>, TestParam<2, 1024>, TestParam<1, 4096>>;
INSTANTIATE_TYPED_TEST_SUITE_P(T, TestStorage, TestTypes);

}  // namespace stm32x::test
