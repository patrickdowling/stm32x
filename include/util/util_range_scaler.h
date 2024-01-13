#ifndef STM32X_UTIL_RANGE_SCALER_H_
#define STM32X_UTIL_RANGE_SCALER_H_

#include <algorithm>
#include <inttypes.h>
#include "util_macros.h"

namespace util {

template <typename Traits>
class RangeScaler {
public:

  static const int32_t kMin = Traits::kMin;
  static const int32_t kMax = Traits::kMax;
  static const int32_t kShift = Traits::kShift;
  static const int32_t kFractionalBits = 16;

  RangeScaler() : min_(kMin), max_(kMax) { }

  void set_min(int32_t min) {
    min_ = min; //std::max(min, kMin + 0); // +0 to avoid & which neesd a physical address of the static
  }

  void set_max(int32_t max) {
    max_ = max;//std::min(max, kMax + 0);
  }

  int32_t map(int32_t value) const {
    int32_t min = min_;
    int32_t max = max_;
    CONSTRAIN(value, kMin, kMax); 
    value = min + ((((value - kMin) << kShift) * (max - min + 1)) >> kFractionalBits);
//    CONSTRAIN(value, min, max);
    return value;
  }

  void Reset() {
    min_ = kMin;
    max_ = kMax;
  }

protected:

  int32_t min_, max_;
};

} // namespace util

#endif // STM32X_UTIL_RANGE_SCALER_H_
