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
// -----------------------------------------------------------------------------
//
// FLASH storage helper -- a bit of a mix & match from o_C and stmlib. The first implementation used
// a different generation number method in the header and more complex scan, but after some
// deliberation the appraoch from stmlib page_storage approach does seem simpler. The generation
// number could probably be removed.
//
// The type_id and length maybe aren't required but seem nice (even if there's an additional version
// tag used to init the CRC). If there are multiple pages there might be more effective ways to
// handle this with less erasing but it hardly seems worth it.

#ifndef STM32X_UTIL_STORAGE_H_
#define STM32X_UTIL_STORAGE_H_

#include <cinttypes>

#include "util/util_fourcc.h"
#include "util/util_macros.h"

#ifdef STM32X_TESTING
#include "fmt/core.h"
#define DUMP_HEADER(op, addr, h)                                                                \
  do {                                                                                          \
    fmt::println("{} {:08x}: {:04x} {:04x} {} {} {:04x}", op, addr, (h)->type_id, (h)->version, \
                 (h)->generation, (h)->length, (h)->crc);                                       \
  } while (0)
#else
#define DUMP_HEADER(op, addr, h)
#endif

namespace util {

// NOTE Initial implementation used hardware for this but it's not super critical?
uint16_t CalcCRC16(const void *data, size_t len);

template <uint32_t end_address, uint32_t storage_length, typename StorageImpl, typename ValueType>
class Storage {
private:
  struct BlockHeader {
    util::FOURCC::value_type type_id;
    uint16_t version;
    uint16_t generation;
    uint16_t length;
    uint16_t crc;
  };

public:
  static constexpr uint32_t kStorageBaseAddress = end_address - storage_length;
  static constexpr uint32_t kStorageEndAddress = end_address;
  static constexpr uint32_t kPageSize = StorageImpl::PAGE_SIZE;
  static constexpr uint32_t kNumPages = storage_length / kPageSize;
  static constexpr uint32_t kBlockSize = sizeof(BlockHeader) + sizeof(ValueType);
  static constexpr uint32_t kNumBlocks = (kPageSize * kNumPages) / kBlockSize;

  static_assert(kNumPages >= 1, "At least one page required");
  static_assert(kNumBlocks >= 1, "ValueType too large");
  static_assert(0 == storage_length % kPageSize, "Length not page-aligned");
  static_assert(0 == kStorageBaseAddress % kPageSize, "Unaligned base address");
  static_assert(0 == sizeof(ValueType) % StorageImpl::ALIGNMENT, "Unaligned ValueType");
  static_assert(0 == sizeof(BlockHeader) % StorageImpl::ALIGNMENT, "Unaligned BlockHeader");

  Storage() { StorageImpl::Init(ValueType::STORAGE_VERSION); }

  ~Storage() {}

  bool Load(ValueType &value)
  {
    const BlockHeader *valid_block = 0;
    uint16_t block_number = kNumBlocks - 1;
    while (block_number--) {
      uint32_t current_block_address = block_address(block_number);
      const BlockHeader *header = header_from_addr(current_block_address);
      DUMP_HEADER('R', current_block_address, header);
      if (header->type_id == ValueType::STORAGE_TYPE_ID &&
          header->version == ValueType::STORAGE_VERSION && header->generation == block_number &&
          header->length == sizeof(ValueType) &&
          header->crc == CalcCRC16(header + 1, sizeof(ValueType))) {
        valid_block = header;
        break;
      } else {
        // If the block was at least partially written, we but not valid, then
        // we can't overwrite it and need to ensure the write pointer won't try
        // to overwrite it. Force a rewrite on the next save.
        // Since the structs might not align with the page boundaries, we'll
        // erase all pages
        if (header->type_id != 0xffffffff) rewrite_ = true;
      }
    }

    if (valid_block) {
      DUMP_HEADER('V', 0, valid_block);
      std::memcpy(&value, valid_block + 1, sizeof(value));
      generation_ = valid_block->generation + 1;
      return true;
    } else {
      generation_ = 0;
      rewrite_ = true;
      return false;
    }
  }

  bool Save(const ValueType &value)
  {
    BlockHeader header;
    header.type_id = ValueType::STORAGE_TYPE_ID;
    header.version = ValueType::STORAGE_VERSION;
    header.generation = generation_;
    header.length = sizeof(ValueType);
    header.crc = CalcCRC16(&value, sizeof(ValueType));

    StorageImpl::Unlock();
    uint32_t write_address = block_address(header.generation);
    if (rewrite_ || write_address + kBlockSize >= kStorageEndAddress) {
      EraseAllPages();
      write_address = kStorageBaseAddress;
      header.generation = 0;
    } else {
      // Erasing the page we're spilling into might not be necessary, but
      // better than not being able to write...
      uint32_t space_in_page = kPageSize - write_address % kPageSize;
      if (kPageSize == space_in_page)
        StorageImpl::ErasePage(write_address);
      else if (kBlockSize > space_in_page)
        StorageImpl::ErasePage(write_address + space_in_page);
    }

    DUMP_HEADER('W', write_address, &header);
    Write(write_address, &header, sizeof(header));
    Write(write_address + sizeof(header), &value, sizeof(ValueType));
    StorageImpl::Lock();

    generation_ = header.generation + 1;
    rewrite_ = false;
    return true;
  }

  inline uint16_t generation() const { return generation_; }

#ifdef STM32X_TESTING
  void Reset()
  {
    generation_ = 0;
    rewrite_ = false;
  }
#endif

private:
  uint16_t generation_ = 0;
  bool rewrite_ = false;

  inline static uint32_t block_address(uint16_t generation)
  {
    return kStorageBaseAddress + kBlockSize * generation;
  }

  inline static const BlockHeader *header_from_addr(uint32_t base_address)
  {
    return reinterpret_cast<const BlockHeader *>(StorageImpl::Map(base_address));
  }

  void EraseAllPages()
  {
    uint32_t page_addr = kStorageBaseAddress;
    size_t num_pages = kNumPages;
    while (num_pages--) {
      StorageImpl::ErasePage(page_addr);
      page_addr += kPageSize;
    }
  }

  bool Write(uint32_t address, const void *data, size_t length)
  {
    const uint32_t *src = static_cast<const uint32_t *>(data);
    length >>= 2;
    size_t written = 0;
    while (length--) {
      if (StorageImpl::ProgramWord(address, *src++)) written += 4;
      address += 4;
    }
    return written == length;
  }
};

}  // namespace util

#endif  // STM32X_UTIL_STORAGE_H_
