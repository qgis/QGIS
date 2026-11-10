// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_COMPRESSION_CHECKSUMCRC32SIMDIMPL_P_H_INCLUDED
#define BLEND2D_COMPRESSION_CHECKSUMCRC32SIMDIMPL_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/compression/checksum_p.h>
#include <blend2d/simd/simd_p.h>

//! \cond INTERNAL

namespace bl::Compression::Checksum {
namespace {

static constexpr uint64_t kConstK1 = 0x0154442BD4u;
static constexpr uint64_t kConstK2 = 0x01C6E41596u;
static constexpr uint64_t kConstK3 = 0x01751997D0u;
static constexpr uint64_t kConstK4 = 0x00CCAA009Eu;
static constexpr uint64_t kConstK5 = 0x0163CD6124u;
static constexpr uint64_t kConstP0 = 0x01DB710641u;
static constexpr uint64_t kConstP1 = 0x01F7011641u;

BL_INLINE uint32_t crc32_update_clmul128(uint32_t checksum, const uint8_t* data, size_t size) noexcept {
  using namespace SIMD;

  size_t n = bl_min<size_t>(IntOps::align_up_diff(uintptr_t(data), 16u), size);
  size -= n;

  BL_NOUNROLL
  while (n) {
    checksum = crc32_update_byte(checksum, *data++);
    n--;
  }

  // Process 64-byte chunks.
  if (size >= 64u) {
    Vec2xU64 x1 = loada_128<Vec2xU64>(data +  0u) ^ cast_from_u32<Vec2xU64>(checksum);
    Vec2xU64 x2 = loada_128<Vec2xU64>(data + 16u);
    Vec2xU64 x3 = loada_128<Vec2xU64>(data + 32u);
    Vec2xU64 x4 = loada_128<Vec2xU64>(data + 48u);
    Vec2xU64 k2k1 = make128_u64(kConstK2, kConstK1);

    data += 64u;
    size -= 64u;

    BL_NOUNROLL
    while (size >= 64u) {
      Vec2xU64 t1 = clmul_u128_ll(x1, k2k1);
      Vec2xU64 t2 = clmul_u128_ll(x2, k2k1);
      Vec2xU64 t3 = clmul_u128_ll(x3, k2k1);
      Vec2xU64 t4 = clmul_u128_ll(x4, k2k1);

      x1 = clmul_u128_hh(x1, k2k1) ^ t1;
      x2 = clmul_u128_hh(x2, k2k1) ^ t2;
      x3 = clmul_u128_hh(x3, k2k1) ^ t3;
      x4 = clmul_u128_hh(x4, k2k1) ^ t4;

      x1 ^= loada_128<Vec2xU64>(data +  0u);
      x2 ^= loada_128<Vec2xU64>(data + 16u);
      x3 ^= loada_128<Vec2xU64>(data + 32u);
      x4 ^= loada_128<Vec2xU64>(data + 48u);

      data += 64u;
      size -= 64u;
    }

    // Fold 4x128 bits into 128 bits.
    Vec2xU64 k4k3 = make128_u64(kConstK4, kConstK3);
    Vec2xU64 t1;

    t1 = clmul_u128_ll(x1, k4k3) ^ x2;
    x1 = clmul_u128_hh(x1, k4k3) ^ t1;

    t1 = clmul_u128_ll(x1, k4k3) ^ x3;
    x1 = clmul_u128_hh(x1, k4k3) ^ t1;

    t1 = clmul_u128_ll(x1, k4k3) ^ x4;
    x1 = clmul_u128_hh(x1, k4k3) ^ t1;

    // Process remaining 16-byte chunks.
    BL_NOUNROLL
    while (size >= 16u) {
      t1 = clmul_u128_ll(x1, k4k3);
      x1 = clmul_u128_hh(x1, k4k3) ^ t1;
      x1 ^= loada_128<Vec2xU64>(data);

      data += 16u;
      size -= 16u;
    }

    // Fold 128 bits to 64 bits.
    t1 = clmul_u128_lh(x1, k4k3);
    x1 = srlb_u128<8>(x1) ^ t1;

    Vec2xU64 k5 = make128_u64(kConstK5);
    Vec2xU64 lo32 = make128_u64(0x00000000FFFFFFFFu);

    t1 = srlb_u128<4>(x1);
    x1 = clmul_u128_ll(x1 & lo32, k5) ^ t1;

    // Reduce 64 bits to 32 bits.
    Vec2xU64 poly = make128_u64(kConstP1, kConstP0);
    t1 = clmul_u128_lh(x1 & lo32, poly);
    x1 ^= clmul_u128_ll(t1 & lo32, poly);

    checksum = extract_u32<1>(x1);
  }

  BL_NOUNROLL
  while (size) {
    checksum = crc32_update_byte(checksum, *data++);
    size--;
  }

  return checksum;
}

} // {anonymous}
} // {bl::Compression::Checksum}

//! \endcond

#endif // BLEND2D_COMPRESSION_CHECKSUMCRC32SIMDIMPL_P_H_INCLUDED
