// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_RANDOM_P_H_INCLUDED
#define BLEND2D_RANDOM_P_H_INCLUDED

#include <blend2d/core/random.h>
#include <blend2d/simd/simd_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

namespace bl {
namespace RandomInternal {

//! \name Constants
//! \{

// Constants suggested as `23/18/5`.
static constexpr uint32_t kStep1Shift = 23;
static constexpr uint32_t kStep2Shift = 18;
static constexpr uint32_t kStep3Shift = 5;

//! Number of bits needed to shift right to extract mantissa.
static constexpr uint32_t kMantissaShift = 64 - 52;

//! \}

//! \name Inline API (Private)
//! \{

static BL_INLINE void reset_seed(BLRandom* self, uint64_t seed) noexcept {
  // The number is arbitrary, it means nothing.
  const uint64_t kZeroSeed = 0x1F0A2BE71D163FA0u;

  // Generate the state data by using splitmix64.
  for (uint32_t i = 0; i < 2; i++) {
    seed += 0x9E3779B97F4A7C15u;
    uint64_t x = seed;
    x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9u;
    x = (x ^ (x >> 27)) * 0x94D049BB133111EBu;
    x = (x ^ (x >> 31));
    self->data[i] = x != 0 ? x : kZeroSeed;
  }
}

static BL_INLINE uint64_t next_uint64(BLRandom* self) noexcept {
  uint64_t x = self->data[0];
  uint64_t y = self->data[1];

  x ^= x << kStep1Shift;
  y ^= y >> kStep3Shift;
  x ^= x >> kStep2Shift;
  x ^= y;

  self->data[0] = y;
  self->data[1] = x;

  return x + y;
}

static BL_INLINE uint32_t next_uint32(BLRandom* self) noexcept {
  return uint32_t(next_uint64(self) >> 32);
}

#if defined(BL_TARGET_OPT_SSE2)

//! High-performance SIMD implementation. Better utilizes CPU in 32-bit mode and it's a better candidate for
//! `bl_random_next_double()` in general on X86 as it returns a SIMD register, which is easier to convert to `double`
//! than GP.
static BL_INLINE SIMD::Vec2xU64 next_uint64AsI128(BLRandom* self) noexcept {
  using namespace SIMD;

  Vec2xU64 x = loada_64<Vec2xU64>(&self->data[0]);
  Vec2xU64 y = loada_64<Vec2xU64>(&self->data[1]);

  x = x ^ (x << Shift<kStep1Shift>{});
  y = y ^ (y >> Shift<kStep3Shift>{});
  x = x ^ (x >> Shift<kStep2Shift>{});
  x = x ^ y;

  storea_64(&self->data[0], y);
  storea_64(&self->data[1], x);

  return x + y;
}

static BL_INLINE double next_double(BLRandom* self) noexcept {
  using namespace SIMD;

  Vec2xU64 kExpMsk = make128_u64(0x3FF0000000000000u);
  Vec2xU64 u = (next_uint64AsI128(self) >> Shift<kMantissaShift>{}) | kExpMsk;
  return cast_to_f64(u) - 1.0;
}

#else

static BL_INLINE double next_double(BLRandom* self) noexcept {
  uint64_t kExpMsk = 0x3FF0000000000000u;
  uint64_t u = (next_uint64(self) >> kMantissaShift) | kExpMsk;
  return bl_bit_cast<double>(u) - 1.0;
}

#endif

//! \}

} // {RandomInternal}
} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_RANDOM_P_H_INCLUDED
