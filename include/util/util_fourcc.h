#ifndef UTIL_FOURCC_H_
#define UTIL_FOURCC_H_

#include <cstddef>
#include <cstdint>

namespace util {

// FourCC are often useful as template or constexpr types so they are mostly interhangebale with
// uint32_t. Definitely needs work.

namespace detail {
struct FourCCString {
  explicit constexpr FourCCString(uint32_t fourcc)
      : value{static_cast<char>(fourcc), static_cast<char>(fourcc >> 8),
              static_cast<char>(fourcc >> 16), static_cast<char>(fourcc >> 24), 0}
  {}

  const char value[5] = {0};
};
}  // namespace detail

struct FOURCC {
  using value_type = uint32_t;

  const value_type value = 0;

  bool operator!() const { return !value; }

  template <size_t N>
  static constexpr FOURCC FromString(const char (&s)[N])
  {
    static_assert(N == 5, "4 chars + '\0'");
    uint32_t fourcc = static_cast<uint32_t>(s[0]) | (static_cast<uint32_t>(s[1]) << 8) |
                      (static_cast<uint32_t>(s[2]) << 16) | (static_cast<uint32_t>(s[3]) << 24);
    return {fourcc};
  }

  static constexpr FOURCC FromString(const char *s, size_t len)
  {
    uint32_t fourcc = len ? static_cast<uint32_t>(s[0]) : 0;
    if (len > 1) fourcc |= static_cast<uint32_t>(s[1]) << 8;
    if (len > 2) fourcc |= static_cast<uint32_t>(s[2]) << 16;
    if (len > 3) fourcc |= static_cast<uint32_t>(s[3]) << 24;
    return {fourcc};
  }

  constexpr auto str() const { return detail::FourCCString{value}; }
};

static constexpr bool operator==(const FOURCC &lhs, const FOURCC &rhs)
{
  return lhs.value == rhs.value;
}

static constexpr bool operator==(const FOURCC &lhs, const FOURCC::value_type rhs)
{
  return lhs.value == rhs;
}

static constexpr bool operator!=(const FOURCC &lhs, const FOURCC &rhs)
{
  return lhs.value != rhs.value;
}

static constexpr bool operator!=(const FOURCC &lhs, const FOURCC::value_type rhs)
{
  return lhs.value != rhs;
}

}  // namespace util

constexpr util::FOURCC::value_type operator""_4CCV(const char *s, size_t len)
{
  return util::FOURCC::FromString(s, len).value;
}

constexpr util::FOURCC operator"" _4CC(const char *s, size_t len)
{
  return util::FOURCC::FromString(s, len);
}
#endif  // UTIL_FOURCC_H_
