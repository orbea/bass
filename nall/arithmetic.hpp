#pragma once

//multi-precision arithmetic
//warning: each size is quadratically more expensive than the size before it!

#include <nall/arithmetic/unsigned.hpp>

namespace nall {
  template<uint Bits> struct ArithmeticNatural;
  template<> struct ArithmeticNatural<  8> { using type =   uint8_t; };
  template<> struct ArithmeticNatural< 16> { using type =  uint16_t; };
  template<> struct ArithmeticNatural< 32> { using type =  uint32_t; };
  template<> struct ArithmeticNatural< 64> { using type =  uint64_t; };
  #if INTMAX_BITS >= 128
  template<> struct ArithmeticNatural<128> { using type = uint128_t; };
  #endif
}
