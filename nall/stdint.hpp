#pragma once

#include <stdint.h>

//note: (u)intmax actually mean it: use as many bits as is possible
#if defined(__SIZEOF_INT128__)
  using  int128_t =   signed __int128;
  using uint128_t = unsigned __int128;

  #define INTMAX_BITS 128
  using  intmax =  int128_t;
  using uintmax = uint128_t;
#else
  #define INTMAX_BITS 64
  using  intmax =  intmax_t;
  using uintmax = uintmax_t;
#endif

using  intptr =  intptr_t;
using uintptr = uintptr_t;

using float32_t = float;
using float64_t = double;
//note: long double size is not reliable across platforms
//using float80_t = long double;

static_assert(sizeof( int8_t)  == 1, "int8_t is not of the correct size" );
static_assert(sizeof(int16_t)  == 2, "int16_t is not of the correct size");
static_assert(sizeof(int32_t)  == 4, "int32_t is not of the correct size");
static_assert(sizeof(int64_t)  == 8, "int64_t is not of the correct size");

static_assert(sizeof( uint8_t) == 1, "int8_t is not of the correct size" );
static_assert(sizeof(uint16_t) == 2, "int16_t is not of the correct size");
static_assert(sizeof(uint32_t) == 4, "int32_t is not of the correct size");
static_assert(sizeof(uint64_t) == 8, "int64_t is not of the correct size");

static_assert(sizeof(float)  >= 4, "float32_t is not of the correct size");
static_assert(sizeof(double) >= 8, "float64_t is not of the correct size");
//static_assert(sizeof(long double) >= 10, "float80_t is not of the correct size");

using sint =   signed int;
using uint = unsigned int;
using real32_t = float;
using real64_t = double;

//shorthand
using  s8 =   int8_t;
using s16 =  int16_t;
using s32 =  int32_t;
using s64 =  int64_t;

using  u8 =  uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using f32 = float32_t;
using f64 = float64_t;

#if defined(__SIZEOF_INT128__)
  using s128 =  int128_t;
  using u128 = uint128_t;
#endif
