// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_CODEC_PNGOPSSIMDIMPL_P_H_INCLUDED
#define BLEND2D_CODEC_PNGOPSSIMDIMPL_P_H_INCLUDED

#include <blend2d/codec/pngops_p.h>
#include <blend2d/simd/simd_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/memops_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_codec_impl
//! \{

namespace bl::Png::Ops {
namespace {

// bl::Png::Opt - SimdImpl
// =======================

// Precalculates D == 3C - B. This is a constant that only relies on the scanline above, thus it's fully vectorized.
static BL_INLINE SIMD::Vec8xU16 v_precalc_d(const SIMD::Vec8xU16& b, const SIMD::Vec8xU16& c) noexcept {
  using namespace SIMD;

  Vec8xU16 c_plus_c = add_i16(c, c);
  Vec8xU16 c_minus_b = sub_i16(c, b);
  return add_i16(c_plus_c, c_minus_b);
}

static BL_INLINE SIMD::Vec8xU16 v_paeth(const SIMD::Vec8xU16& a, const SIMD::Vec8xU16& b, const SIMD::Vec8xU16& c, const SIMD::Vec8xU16& d) noexcept {
  using namespace SIMD;

  Vec8xU16 hi = max_i16(a, b);
  Vec8xU16 threshold = sub_i16(d, a);
  Vec8xU16 lo = min_i16(a, b);

  Vec8xU16 pred_hi = cmp_gt_i16(hi, threshold);
  Vec8xU16 pred_lo = cmp_gt_i16(threshold, lo);

  Vec8xU16 t0 = blendv_u8(lo, c, pred_hi);
  return blendv_u8(hi, t0, pred_lo);
}

template<uint32_t Shift>
static BL_INLINE SIMD::Vec16xU8 v_sllb_addb(const SIMD::Vec16xU8& a) noexcept {
  using namespace SIMD;

  Vec16xU8 t = sllb_u128<Shift>(a);
  return add_i8(a, t);
}

template<uint32_t BPP>
BLResult BL_CDECL inverse_filter_simd_impl(uint8_t* p, uint32_t bpp, uint32_t bpl, uint32_t h) noexcept {
  using namespace SIMD;

  // Only used by asserts, unused in release mode.
  bl_unused(bpp);

  BL_ASSERT(bpp == BPP);
  BL_ASSERT(bpl > 1u);
  BL_ASSERT(h   > 0u);

  uint32_t y = h;
  uint8_t* u = nullptr;

  // Subtract one BYTE that is used to store the `filter` ID - it's always processed and not part of pixel data.
  bpl--;

  // First row uses a special filter that doesn't access the previous row,
  // which is assumed to contain all zeros.
  uint32_t filter_type = *p++;

  if (filter_type >= kFilterTypeCount)
    filter_type = kFilterTypeNone;

  filter_type = simplify_filter_of_first_row(filter_type);

  for (;;) {
    switch (filter_type) {
      // This is one of the easiest filters to parallelize. Although it looks like the data dependency
      // is too high, it's simply additions, which are really easy to parallelize. The following formula:
      //
      //     Y1' = BYTE(Y1 + Y0')
      //     Y2' = BYTE(Y2 + Y1')
      //     Y3' = BYTE(Y3 + Y2')
      //     Y4' = BYTE(Y4 + Y3')
      //
      // Expanded to (with byte casts removed, as they are implicit in our case):
      //
      //     Y1' = Y1 + Y0'
      //     Y2' = Y2 + Y1 + Y0'
      //     Y3' = Y3 + Y2 + Y1 + Y0'
      //     Y4' = Y4 + Y3 + Y2 + Y1 + Y0'
      //
      // The size of the register doesn't matter here. The Y0' dependency has been omitted to make the
      // flow cleaner, however, it can be added to Y1 before processing or it can be shifted to the
      // first cell so the first addition would be performed against [Y0', Y1, Y2, Y3].
      case kFilterTypeSub: {
        uint32_t i = bpl - BPP;

        if (i >= 32) {
          // Align to 16-BYTE boundary.
          uint32_t j = uint32_t(IntOps::align_up_diff(uintptr_t(p + BPP), 16));
          for (i -= j; j != 0; j--, p++)
            p[BPP] = apply_sum_filter(p[BPP], p[0]);

          if constexpr (BPP == 1) {
            Vec16xU8 p0, p1, p2, p3;
            Vec16xU8 t2;

            // Process 64 BYTEs at a time.
            p0 = cast_from_u32<Vec16xU8>(p[0]);
            while (i >= 64) {
              p0 = add_i8(p0, loada<Vec16xU8>(p + 1));
              p1 = loada<Vec16xU8>(p + 17);
              p2 = loada<Vec16xU8>(p + 33);
              p3 = loada<Vec16xU8>(p + 49);

              p0 = v_sllb_addb<1>(p0);
              p2 = v_sllb_addb<1>(p2);
              p0 = v_sllb_addb<2>(p0);
              p2 = v_sllb_addb<2>(p2);
              p0 = v_sllb_addb<4>(p0);
              p2 = v_sllb_addb<4>(p2);
              p0 = v_sllb_addb<8>(p0);
              p2 = v_sllb_addb<8>(p2);
              storea<Vec16xU8>(p + 1, p0);

              p0 = srlb_u128<15>(p0);
              t2 = srlb_u128<15>(p2);
              p1 = add_i8(p1, p0);
              p3 = add_i8(p3, t2);

              p1 = v_sllb_addb<1>(p1);
              p3 = v_sllb_addb<1>(p3);
              p1 = v_sllb_addb<2>(p1);
              p3 = v_sllb_addb<2>(p3);
              p1 = v_sllb_addb<4>(p1);
              p3 = v_sllb_addb<4>(p3);
              p1 = v_sllb_addb<8>(p1);
              p3 = v_sllb_addb<8>(p3);
              storea<Vec16xU8>(p + 17, p1);

              p1 = interleave_hi_u8(p1, p1);
              p1 = interleave_hi_u16(p1, p1);
              p1 = swizzle_u32<3, 3, 3, 3>(p1);

              p2 = add_i8(p2, p1);
              p3 = add_i8(p3, p1);

              storea<Vec16xU8>(p + 33, p2);
              storea<Vec16xU8>(p + 49, p3);
              p0 = srlb_u128<15>(p3);

              p += 64;
              i -= 64;
            }

            // Process 16 BYTEs at a time.
            while (i >= 16) {
              p0 = add_i8(p0, loada<Vec16xU8>(p + 1));
              p0 = v_sllb_addb<1>(p0);
              p0 = v_sllb_addb<2>(p0);
              p0 = v_sllb_addb<4>(p0);
              p0 = v_sllb_addb<8>(p0);

              storea(p + 1, p0);
              p0 = srlb_u128<15>(p0);

              p += 16;
              i -= 16;
            }
          }
          else if constexpr (BPP == 2) {
            Vec16xU8 p0, p1, p2, p3;
            Vec16xU8 t2;

            // Process 64 BYTEs at a time.
            p0 = cast_from_u32<Vec16xU8>(MemOps::readU16a(p));
            while (i >= 64) {
              p0 = add_i8(p0, loada<Vec16xU8>(p + 2));
              p1 = loada<Vec16xU8>(p + 18);
              p2 = loada<Vec16xU8>(p + 34);
              p3 = loada<Vec16xU8>(p + 50);

              p0 = v_sllb_addb<2>(p0);
              p2 = v_sllb_addb<2>(p2);
              p0 = v_sllb_addb<4>(p0);
              p2 = v_sllb_addb<4>(p2);
              p0 = v_sllb_addb<8>(p0);
              p2 = v_sllb_addb<8>(p2);
              storea(p + 2, p0);

              p0 = srlb_u128<14>(p0);
              t2 = srlb_u128<14>(p2);
              p1 = add_i8(p1, p0);
              p3 = add_i8(p3, t2);

              p1 = v_sllb_addb<2>(p1);
              p3 = v_sllb_addb<2>(p3);
              p1 = v_sllb_addb<4>(p1);
              p3 = v_sllb_addb<4>(p3);
              p1 = v_sllb_addb<8>(p1);
              p3 = v_sllb_addb<8>(p3);
              storea(p + 18, p1);

              p1 = interleave_hi_u16(p1, p1);
              p1 = swizzle_u32<3, 3, 3, 3>(p1);

              p2 = add_i8(p2, p1);
              p3 = add_i8(p3, p1);

              storea(p + 34, p2);
              storea(p + 50, p3);
              p0 = srlb_u128<14>(p3);

              p += 64;
              i -= 64;
            }

            // Process 16 BYTEs at a time.
            while (i >= 16) {
              p0 = add_i8(p0, loada<Vec16xU8>(p + 2));
              p0 = v_sllb_addb<2>(p0);
              p0 = v_sllb_addb<4>(p0);
              p0 = v_sllb_addb<8>(p0);

              storea(p + 2, p0);
              p0 = srlb_u128<14>(p0);

              p += 16;
              i -= 16;
            }
          }
          else if constexpr (BPP == 3) {
            Vec16xU8 p0, p1, p2, p3;
            Vec16xU8 t0, t2;

#if defined(BL_TARGET_OPT_SSSE3) || defined(BL_TARGET_OPT_ASIMD)
            Vec16xU8 ext3b = make128_u64<Vec16xU8>(0xFFFF0A09080A0908u, 0xFFFF020100020100u);
#else
            Vec16xU8 ext3b = make128_u32<Vec16xU8>(0x01000001u);
#endif

            // Process 64 BYTEs at a time.
            p0 = cast_from_u32<Vec16xU8>(MemOps::readU32u(p) & 0x00FFFFFFu);
            while (i >= 64) {
              p0 = add_i8(p0, loada<Vec16xU8>(p + 3));
              p1 = loada<Vec16xU8>(p + 19);
              p2 = loada<Vec16xU8>(p + 35);

              p0 = v_sllb_addb<3>(p0);
              p2 = v_sllb_addb<3>(p2);
              p0 = v_sllb_addb<6>(p0);
              p2 = v_sllb_addb<6>(p2);
              p0 = v_sllb_addb<12>(p0);
              p2 = v_sllb_addb<12>(p2);

              p3 = loada<Vec16xU8>(p + 51);
              t0 = srlb_u128<13>(p0);
              t2 = srlb_u128<13>(p2);

              p1 = add_i8(p1, t0);
              p3 = add_i8(p3, t2);

              p1 = v_sllb_addb<3>(p1);
              p3 = v_sllb_addb<3>(p3);
              p1 = v_sllb_addb<6>(p1);
              p3 = v_sllb_addb<6>(p3);
              p1 = v_sllb_addb<12>(p1);
              p3 = v_sllb_addb<12>(p3);
              storea(p + 3, p0);

              p0 = swizzle_u32<3, 3, 3, 3>(p1);
              p0 = srli_u32<8>(p0);

#if defined(BL_TARGET_OPT_SSSE3) || defined(BL_TARGET_OPT_ASIMD)
              p0 = swizzlev_u8(p0, ext3b);
#else
              p0 = mulw_u32(p0, ext3b);
#endif

              p0 = swizzle_lo_u16<0, 2, 1, 0>(p0);
              p0 = swizzle_hi_u16<1, 0, 2, 1>(p0);

              storea(p + 19, p1);
              p2 = add_i8(p2, p0);
              p0 = swizzle_u32<1, 3, 2, 1>(p0);

              storea(p + 35, p2);
              p0 = add_i8(p0, p3);

              storea(p + 51, p0);
              p0 = srlb_u128<13>(p0);

              p += 64;
              i -= 64;
            }

            // Process 16 BYTEs at a time.
            while (i >= 16) {
              p0 = add_i8(p0, loada<Vec16xU8>(p + 3));

              p0 = v_sllb_addb<3>(p0);
              p0 = v_sllb_addb<6>(p0);
              p0 = v_sllb_addb<12>(p0);

              storea(p + 3, p0);
              p0 = srlb_u128<13>(p0);

              p += 16;
              i -= 16;
            }
          }
          else if constexpr (BPP == 4) {
            Vec16xU8 p0, p1, p2, p3;
            Vec16xU8 t2;

            // Process 64 BYTEs at a time.
            p0 = cast_from_u32<Vec16xU8>(MemOps::readU32a(p));
            while (i >= 64) {
              p0 = add_i8(p0, loada<Vec16xU8>(p + 4));
              p1 = loada<Vec16xU8>(p + 20);
              p2 = loada<Vec16xU8>(p + 36);
              p3 = loada<Vec16xU8>(p + 52);

              p0 = v_sllb_addb<4>(p0);
              p2 = v_sllb_addb<4>(p2);
              p0 = v_sllb_addb<8>(p0);
              p2 = v_sllb_addb<8>(p2);
              storea(p + 4, p0);

              p0 = srlb_u128<12>(p0);
              t2 = srlb_u128<12>(p2);

              p1 = add_i8(p1, p0);
              p3 = add_i8(p3, t2);

              p1 = v_sllb_addb<4>(p1);
              p3 = v_sllb_addb<4>(p3);
              p1 = v_sllb_addb<8>(p1);
              p3 = v_sllb_addb<8>(p3);

              p0 = swizzle_u32<3, 3, 3, 3>(p1);
              storea(p + 20, p1);

              p2 = add_i8(p2, p0);
              p0 = add_i8(p0, p3);

              storea(p + 36, p2);
              storea(p + 52, p0);
              p0 = srlb_u128<12>(p0);

              p += 64;
              i -= 64;
            }

            // Process 16 BYTEs at a time.
            while (i >= 16) {
              p0 = add_i8(p0, loada<Vec16xU8>(p + 4));

              p0 = v_sllb_addb<4>(p0);
              p0 = v_sllb_addb<8>(p0);
              storea(p + 4, p0);
              p0 = srlb_u128<12>(p0);

              p += 16;
              i -= 16;
            }
          }
          else if constexpr (BPP == 6) {
            Vec16xU8 p0, p1, p2, p3;
            Vec16xU8 t2;

            p0 = loadu_64<Vec16xU8>(p);
            p0 = slli_i64<16>(p0);
            p0 = srli_u64<16>(p0);

            // Process 64 BYTEs at a time.
            while (i >= 64) {
              p0 = add_i8(p0, loada<Vec16xU8>(p + 6));
              p1 = loada<Vec16xU8>(p + 22);
              p2 = loada<Vec16xU8>(p + 38);

              p0 = v_sllb_addb<6>(p0);
              p2 = v_sllb_addb<6>(p2);
              p0 = v_sllb_addb<12>(p0);
              p2 = v_sllb_addb<12>(p2);

              p3 = loada<Vec16xU8>(p + 54);
              storea(p + 6, p0);

              p0 = srlb_u128<10>(p0);
              t2 = srlb_u128<10>(p2);

              p1 = add_i8(p1, p0);
              p3 = add_i8(p3, t2);

              p1 = v_sllb_addb<6>(p1);
              p3 = v_sllb_addb<6>(p3);
              p1 = v_sllb_addb<12>(p1);
              p3 = v_sllb_addb<12>(p3);

              p0 = dup_hi_u64(p1);
              p0 = swizzle_lo_u16<1, 3, 2, 1>(p0);
              p0 = swizzle_hi_u16<2, 1, 3, 2>(p0);

              storea(p + 22, p1);
              p2 = add_i8(p2, p0);
              p0 = swizzle_u32<1, 3, 2, 1>(p0);

              storea(p + 38, p2);
              p0 = add_i8(p0, p3);

              storea(p + 54, p0);
              p0 = srlb_u128<10>(p0);

              p += 64;
              i -= 64;
            }

            // Process 16 BYTEs at a time.
            while (i >= 16) {
              p0 = add_i8(p0, loada<Vec16xU8>(p + 6));

              p0 = v_sllb_addb<6>(p0);
              p0 = v_sllb_addb<12>(p0);

              storea(p + 6, p0);
              p0 = srlb_u128<10>(p0);

              p += 16;
              i -= 16;
            }
          }
          else if constexpr (BPP == 8) {
            Vec16xU8 p0, p1, p2, p3;
            Vec16xU8 t2;

            // Process 64 BYTEs at a time.
            p0 = loadu_64<Vec16xU8>(p);
            while (i >= 64) {
              p0 = add_i8(p0, loada<Vec16xU8>(p + 8));
              p1 = loada<Vec16xU8>(p + 24);
              p2 = loada<Vec16xU8>(p + 40);
              p3 = loada<Vec16xU8>(p + 56);

              p0 = v_sllb_addb<8>(p0);
              p2 = v_sllb_addb<8>(p2);
              storea(p + 8, p0);

              p0 = srlb_u128<8>(p0);
              t2 = dup_hi_u64(p2);
              p1 = add_i8(p1, p0);

              p1 = v_sllb_addb<8>(p1);
              p3 = v_sllb_addb<8>(p3);
              p0 = dup_hi_u64(p1);
              p3 = add_i8(p3, t2);
              storea(p + 24, p1);

              p2 = add_i8(p2, p0);
              p0 = add_i8(p0, p3);

              storea(p + 40, p2);
              storea(p + 56, p0);
              p0 = srlb_u128<8>(p0);

              p += 64;
              i -= 64;
            }

            // Process 16 BYTEs at a time.
            while (i >= 16) {
              p0 = add_i8(p0, loada<Vec16xU8>(p + 8));
              p0 = v_sllb_addb<8>(p0);

              storea(p + 8, p0);
              p0 = srlb_u128<8>(p0);

              p += 16;
              i -= 16;
            }
          }
        }

        for (; i != 0; i--, p++)
          p[BPP] = apply_sum_filter(p[BPP], p[0]);

        p += BPP;
        break;
      }

      // This is actually the easiest filter that doesn't require any kind of specialization for a particular BPP.
      case kFilterTypeUp: {
        BL_ASSERT(u != nullptr);

        uint32_t i = bpl;

        if (i >= 24) {
          // Align to 16-BYTE boundary.
          uint32_t j = uint32_t(IntOps::align_up_diff(uintptr_t(p), 16));
          for (i -= j; j != 0; j--, p++, u++)
            p[0] = apply_sum_filter(p[0], u[0]);

          // Process 64 BYTEs at a time.
          while (i >= 64) {
            Vec16xU8 u0 = loadu<Vec16xU8>(u +  0);
            Vec16xU8 u1 = loadu<Vec16xU8>(u + 16);
            Vec16xU8 p0 = add_i8(u0, loada<Vec16xU8>(p +  0));
            Vec16xU8 p1 = add_i8(u1, loada<Vec16xU8>(p + 16));

            Vec16xU8 u2 = loadu<Vec16xU8>(u + 32);
            Vec16xU8 u3 = loadu<Vec16xU8>(u + 48);
            Vec16xU8 p2 = add_i8(u2, loada<Vec16xU8>(p + 32));
            Vec16xU8 p3 = add_i8(u3, loada<Vec16xU8>(p + 48));

            storea(p     , p0);
            storea(p + 16, p1);
            storea(p + 32, p2);
            storea(p + 48, p3);

            p += 64;
            u += 64;
            i -= 64;
          }

          // Process 8 BYTEs at a time.
          while (i >= 8) {
            Vec16xU8 u0 = loadu_64<Vec16xU8>(u);
            Vec16xU8 p0 = loada_64<Vec16xU8>(p);

            storea_64(p, add_i8(p0, u0));

            p += 8;
            u += 8;
            i -= 8;
          }
        }

        for (; i != 0; i--, p++, u++)
          p[0] = apply_sum_filter(p[0], u[0]);
        break;
      }

      // This filter is extremely difficult for low BPP values as there is a huge sequential data dependency,
      // I didn't succeeded to solve it. 1-3 BPP implementations are pretty bad and I would like to hear about
      // a way to improve those. The implementation for 4 BPP and more is pretty good, as there is less data
      // dependency between individual bytes.
      //
      // Sequential Approach:
      //
      //     Y1' = byte((2*Y1 + U1 + Y0') >> 1)
      //     Y2' = byte((2*Y2 + U2 + Y1') >> 1)
      //     Y3' = byte((2*Y3 + U3 + Y2') >> 1)
      //     Y4' = byte((2*Y4 + U4 + Y3') >> 1)
      //     Y5' = ...
      //
      // Expanded, `U1 + Y0'` replaced with `U1`:
      //
      //     Y1' = byte((2*Y1 + U1) >> 1)
      //     Y2' = byte((2*Y2 + U2 + byte((2*Y1 + U1) >> 1)) >> 1)
      //     Y3' = byte((2*Y3 + U3 + byte((2*Y2 + U2 + byte((2*Y1 + U1) >> 1)) >> 1)) >> 1)
      //     Y4' = byte((2*Y4 + U4 + byte((2*Y3 + U3 + byte((2*Y2 + U2 + byte((2*Y1 + U1) >> 1)) >> 1)) >> 1)) >> 1)
      //     Y5' = ...
      case kFilterTypeAvg: {
        BL_ASSERT(u != nullptr);

        for (uint32_t i = 0; i < BPP; i++)
          p[i] = apply_sum_filter(p[i], u[i] >> 1);

        u += BPP;

        uint32_t i = bpl - BPP;
        if (i >= 32) {
          // Align to 16-BYTE boundary.
          uint32_t j = uint32_t(IntOps::align_up_diff(uintptr_t(p + BPP), 16));

          for (i -= j; j != 0; j--, p++, u++)
            p[BPP] = apply_sum_filter(p[BPP], apply_avg_filter(p[0], u[0]));

          if constexpr (BPP == 1) {
            // This is one of the most difficult AVG filters. 1-BPP has a huge sequential dependency, which is
            // nearly impossible to parallelize. The code below is the best I could have written, it's a mixture
            // of C++ and SIMD. Maybe using a pure C would be even better than this code, but, I tried to take
            // advantage of 8 BYTE fetches at least. Unrolling the loop any further doesn't lead to an improvement.
            uint32_t t0 = p[0];
            uint32_t t1;

            // Process 8 BYTEs at a time.
            while (i >= 8) {
              Vec16xU8 p0 = loada_64_u8_u16<Vec16xU8>(p + 1);
              Vec16xU8 u0 = loada_64_u8_u16<Vec16xU8>(u);

              p0 = slli_i16<1>(p0);
              p0 = add_i16(p0, u0);

              t1 = cast_to_u32(p0);
              p0 = srlb_u128<4>(p0);
              t0 = ((t0 + t1) >> 1) & 0xFF; t1 >>= 16;
              p[1] = uint8_t(t0);

              t0 = ((t0 + t1) >> 1) & 0xFF;
              t1 = cast_to_u32(p0);
              p0 = srlb_u128<4>(p0);
              p[2] = uint8_t(t0);

              t0 = ((t0 + t1) >> 1) & 0xFF; t1 >>= 16;
              p[3] = uint8_t(t0);

              t0 = ((t0 + t1) >> 1) & 0xFF;
              t1 = cast_to_u32(p0);
              p0 = srlb_u128<4>(p0);
              p[4] = uint8_t(t0);

              t0 = ((t0 + t1) >> 1) & 0xFF; t1 >>= 16;
              p[5] = uint8_t(t0);

              t0 = ((t0 + t1) >> 1) & 0xFF;
              t1 = cast_to_u32(p0);
              p[6] = uint8_t(t0);

              t0 = ((t0 + t1) >> 1) & 0xFF; t1 >>= 16;
              p[7] = uint8_t(t0);

              t0 = ((t0 + t1) >> 1) & 0xFF;
              p[8] = uint8_t(t0);

              p += 8;
              u += 8;
              i -= 8;
            }
          }
          // TODO: [PNG] Not complete / Not working.
          /*
          else if constexpr (BPP == 2) {
          }
          else if constexpr (BPP == 3) {
          }
          */
          else if constexpr (BPP == 4) {
            Vec16xU8 m00FF = make128_u32<Vec16xU8>(0x00FF00FFu);
            Vec16xU8 m01FF = make128_u32<Vec16xU8>(0x01FF01FFu);
            Vec16xU8 t1 = unpack_lo64_u8_u16(loada_32<Vec16xU8>(p));

            // Process 16 BYTEs at a time.
            while (i >= 16) {
              Vec16xU8 p0, p1;
              Vec16xU8 u0, u1;

              p1 = loada<Vec16xU8>(p + 4);
              u1 = loadu<Vec16xU8>(u);

              p0 = unpack_lo64_u8_u16(p1);         // LO | Unpack Ln
              p1 = unpack_hi64_u8_u16(p1);         // HI | Unpack Ln
              p0 = slli_i16<1>(p0);                // LO | << 1

              u0 = unpack_lo64_u8_u16(u1);         // LO | Unpack Up
              p0 = add_i16(p0, t1);                // LO | Add Last

              p0 = add_i16(p0, u0);                // LO | Add Up
              p0 = p0 & m01FF;                     // LO | & 0x01FE

              u1 = unpack_hi64_u8_u16(u1);         // HI | Unpack Up
              t1 = sllb_u128<8>(p0);               // LO | Get Last
              p0 = slli_i16<1>(p0);                // LO | << 1

              p1 = slli_i16<1>(p1);                // HI | << 1
              p0 = add_i16(p0, t1);                // LO | Add Last
              p0 = srli_u16<2>(p0);                // LO | >> 2

              p1 = add_i16(p1, u1);                // HI | Add Up
              p0 = p0 & m00FF;                     // LO | & 0x00FF
              t1 = srlb_u128<8>(p0);               // LO | Get Last

              p1 = add_i16(p1, t1);                // HI | Add Last
              p1 = p1 & m01FF;                     // HI | & 0x01FE

              t1 = sllb_u128<8>(p1);               // HI | Get Last
              p1 = slli_i16<1>(p1);                // HI | << 1

              t1 = add_i16(t1, p1);                // HI | Add Last
              t1 = srli_u16<2>(t1);                // HI | >> 2
              t1 = t1 & m00FF;                     // HI | & 0x00FF

              p0 = packz_128_u16_u8(p0, t1);
              t1 = srlb_u128<8>(t1);               // HI | Get Last
              storea(p + 4, p0);

              p += 16;
              u += 16;
              i -= 16;
            }
          }
          else if constexpr (BPP == 6) {
            Vec16xU8 t1 = loadu_64<Vec16xU8>(p);

            // Process 16 BYTEs at a time.
            while (i >= 16) {
              Vec16xU8 p0, p1, p2;
              Vec16xU8 u0, u1, u2;

              u0 = loadu<Vec16xU8>(u);
              t1 = unpack_lo64_u8_u16(t1);
              p0 = loada<Vec16xU8>(p + 6);

              p1 = srlb_u128<6>(p0);               // P1 | Extract
              u1 = srlb_u128<6>(u0);               // P1 | Extract

              p2 = srlb_u128<12>(p0);              // P2 | Extract
              u2 = srlb_u128<12>(u0);              // P2 | Extract

              p0 = unpack_lo64_u8_u16(p0);         // P0 | Unpack
              u0 = unpack_lo64_u8_u16(u0);         // P0 | Unpack

              p1 = unpack_lo64_u8_u16(p1);         // P1 | Unpack
              u1 = unpack_lo64_u8_u16(u1);         // P1 | Unpack

              p2 = unpack_lo64_u8_u16(p2);         // P2 | Unpack
              u2 = unpack_lo64_u8_u16(u2);         // P2 | Unpack

              u0 = add_i16(u0, t1);                // P0 | Add Last
              u0 = srli_u16<1>(u0);                // P0 | >> 1
              p0 = add_i8(p0, u0);                 // P0 | Add (Up+Last)/2

              u1 = add_i16(u1, p0);                // P1 | Add P0
              u1 = srli_u16<1>(u1);                // P1 | >> 1
              p1 = add_i8(p1, u1);                 // P1 | Add (Up+Last)/2

              u2 = add_i16(u2, p1);                // P2 | Add P1
              u2 = srli_u16<1>(u2);                // P2 | >> 1
              p2 = add_i8(p2, u2);                 // P2 | Add (Up+Last)/2

              p0 = sllb_u128<4>(p0);
              p0 = packz_128_u16_u8(p0, p1);
              p0 = sllb_u128<2>(p0);
              p0 = srlb_u128<4>(p0);

              p2 = packz_128_u16_u8(p2, p2);
              p2 = sllb_u128<12>(p2);
              p0 = p0 | p2;

              storea(p + 6, p0);
              t1 = srlb_u128<10>(p0);

              p += 16;
              u += 16;
              i -= 16;
            }
          }
          else if constexpr (BPP == 8) {
            // Process 16 BYTEs at a time.
            Vec16xU8 t1 = unpack_lo64_u8_u16(loadu_64<Vec16xU8>(p));

            while (i >= 16) {
              Vec16xU8 p0, p1;
              Vec16xU8 u0, u1;

              u1 = loadu<Vec16xU8>(u);
              p1 = loada<Vec16xU8>(p + 8);

              u0 = unpack_lo64_u8_u16(u1);         // LO | Unpack Up
              p0 = unpack_lo64_u8_u16(p1);         // LO | Unpack Ln

              u0 = add_i16(u0, t1);                // LO | Add Last
              p1 = unpack_hi64_u8_u16(p1);         // HI | Unpack Ln
              u0 = srli_u16<1>(u0);                // LO | >> 1
              u1 = unpack_hi64_u8_u16(u1);         // HI | Unpack Up

              p0 = add_i8(p0, u0);                 // LO | Add (Up+Last)/2
              u1 = add_i16(u1, p0);                // HI | Add LO
              u1 = srli_u16<1>(u1);                // HI | >> 1
              p1 = add_i8(p1, u1);                 // HI | Add (Up+LO)/2

              p0 = packz_128_u16_u8(p0, p1);
              t1 = p1;                             // HI | Get Last
              storea(p + 8, p0);

              p += 16;
              u += 16;
              i -= 16;
            }
          }
        }

        for (; i != 0; i--, p++, u++)
          p[BPP] = apply_sum_filter(p[BPP], apply_avg_filter(p[0], u[0]));

        p += BPP;
        break;
      }

      case kFilterTypePaeth: {
        BL_ASSERT(u != nullptr);

        // Paeth:
        //
        //   [C] [B]
        //   [A] [P] <- Current pixel
        //
        // Per pixel computation:
        //
        //   Q = (P + Paeth(A, B, C)) & 0xFF

        if constexpr (BPP == 1) {
          uint32_t sa0 = 0;
          uint32_t sc0 = 0;

          for (uint32_t i = 0; i < bpl; i++) {
            uint32_t sb0 = u[i];
            sa0 = (uint32_t(p[i]) + apply_paeth_filter(sa0, sb0, sc0)) & 0xFFu;
            sc0 = sb0;
            p[i] = uint8_t(sa0);
          }

          p += bpl;
        }
        else if constexpr (BPP == 2) {
          uint32_t sa0 = 0;
          uint32_t sa1 = 0;
          uint32_t sc0 = 0;
          uint32_t sc1 = 0;

          uint32_t i = bpl;
          while (i) {
            // Must hold as `bytes_per_line % 2 == 0`
            BL_ASSERT(i >= 2);

            uint32_t sb0 = u[0];
            uint32_t sb1 = u[1];

            sa0 = (uint32_t(p[0]) + apply_paeth_filter(sa0, sb0, sc0)) & 0xFFu;
            sa1 = (uint32_t(p[1]) + apply_paeth_filter(sa1, sb1, sc1)) & 0xFFu;

            sc0 = sb0;
            sc1 = sb1;

            p[0] = uint8_t(sa0);
            p[1] = uint8_t(sa1);

            p += 2;
            u += 2;
            i -= 2;
          }
        }
        else if constexpr (BPP == 3) {
          Vec8xU16 va0 = make_zero<Vec8xU16>();
          Vec8xU16 vc0 = make_zero<Vec8xU16>();
          Vec8xU16 vmask = make128_u64<Vec8xU16>(0x0000000000000000u, 0x0000FFFFFFFFFFFFu);

          // Process 12 BYTEs at a time (but load 16 BYTEs at a time for simplicity).
          uint32_t i = bpl;
          while (i >= 16) {
            Vec8xU16 vb0 = loadu_128<Vec8xU16>(u);
            Vec8xU16 vp0 = loadu_128<Vec8xU16>(p);

            vc0 = or_(vc0, sllb_u128<3>(vb0));

            Vec8xU16 vb1 = srlb_u128<6>(vb0);
            Vec8xU16 vc1 = srlb_u128<6>(vc0);
            Vec8xU16 vp1 = srlb_u128<6>(vp0);

            vb0 = unpack_lo64_u8_u16(vb0);
            vb1 = unpack_lo64_u8_u16(vb1);
            vc0 = unpack_lo64_u8_u16(vc0);
            vc1 = unpack_lo64_u8_u16(vc1);
            vp0 = unpack_lo64_u8_u16(vp0);
            vp1 = unpack_lo64_u8_u16(vp1);

            Vec8xU16 vd0 = v_precalc_d(vb0, vc0);
            Vec8xU16 vd1 = v_precalc_d(vb1, vc1);

            Vec8xU16 vq0 = add_i8(v_paeth(            (va0), vb0, vc0, vd0), vp0);
            Vec8xU16 vq1 = add_i8(v_paeth(sllb_u128<6>(vq0), vb0, vc0, vd0), vp0);
            Vec8xU16 vq2 = add_i8(v_paeth(srlb_u128<6>(vq1), vb1, vc1, vd1), vp1);
            Vec8xU16 vq3 = add_i8(v_paeth(sllb_u128<6>(vq2), vb1, vc1, vd1), vp1);

            vq0 = blendv_u8(vq1, vq0, vmask);
            vq2 = blendv_u8(vq3, vq2, vmask);

            vq0 = packz_128_u16_u8(vq0);
            va0 = and_(srlb_u128<6>(vq2), vmask);

            vq2 = packz_128_u16_u8(vq2);
            vc0 = srli_u64<24>(and_(packz_128_u16_u8(vb1), vmask));

            vq0 = or_(and_(vq0, vmask), slli_u64<48>(vq2));
            storeu_64(p + 0, vq0);
            storeu_32(p + 8, srli_u64<16>(vq2));

            p += 12;
            u += 12;
            i -= 12;
          }

          // Process 3 BYTEs at a time (but load 4 bytes at once to avoid byte loads).
          vc0 = unpack_lo64_u8_u16(vc0);
          vmask = make128_u64<Vec8xU16>(0x0000000000000000u, 0x000000000000FFFFu);

          while (i) {
            // Must hold as `bytes_per_line % 3 == 0`
            BL_ASSERT(i >= 3);

            Vec8xU16 vb0 = unpack_lo64_u8_u16(loadu_32<Vec8xU16>(u));
            Vec8xU16 vp0 = unpack_lo64_u8_u16(loadu_32<Vec8xU16>(p - 1));

            Vec8xU16 vd0 = v_precalc_d(vb0, vc0);
            va0 = add_i8(v_paeth(va0, vb0, vc0, vd0), srli_u64<16>(vp0));
            vc0 = vb0;
            storeu_32(p - 1, packz_128_u16_u8(or_(slli_u64<16>(va0), and_(vp0, vmask))));

            p += 3;
            u += 3;
            i -= 3;
          }
        }
        else if constexpr (BPP == 4) {
          Vec8xU16 va0 = make_zero<Vec8xU16>();
          Vec8xU16 vc0 = make_zero<Vec8xU16>();

          uint32_t i = bpl;
          while (i >= 16) {
            Vec8xU16 vb = loadu_128<Vec8xU16>(u);
            Vec8xU16 vp0 = loadu_128<Vec8xU16>(p);

            vc0 = or_(vc0, sllb_u128<4>(vb));

            Vec8xU16 vb1 = unpack_hi64_u8_u16(vb);
            Vec8xU16 vc1 = unpack_hi64_u8_u16(vc0);
            Vec8xU16 vp1 = unpack_hi64_u8_u16(vp0);
            Vec8xU16 vb0 = unpack_lo64_u8_u16(vb);

            vc0 = unpack_lo64_u8_u16(vc0);
            vp0 = unpack_lo64_u8_u16(vp0);

            Vec8xU16 vd0 = v_precalc_d(vb0, vc0);
            Vec8xU16 vd1 = v_precalc_d(vb1, vc1);

            Vec8xU16 vq0, vq1, vq2;
            vq0 = add_i8(v_paeth(        (va0), vb0, vc0, vd0), vp0);
            vq1 = add_i8(v_paeth(swap_u64(vq0), vb0, vc0, vd0), vp0);
            vq2 = add_i8(v_paeth(swap_u64(vq1), vb1, vc1, vd1), vp1);
            va0 = add_i8(v_paeth(swap_u64(vq2), vb1, vc1, vd1), vp1);

            vq0 = shuffle_u64<1, 0>(vq0, vq1);
            vq2 = shuffle_u64<1, 0>(vq2, va0);
            va0 = srlb_u128<8>(va0);

            vq0 = packz_128_u16_u8(vq0, vq2);
            vc0 = srlb_u128<12>(vb);

            storeu_128(p, vq0);

            p += 16;
            u += 16;
            i -= 16;
          }

          vc0 = unpack_lo64_u8_u16(vc0);
          while (i) {
            // Must hold as `bytes_per_line % 4 == 0`
            BL_ASSERT(i >= 4);

            Vec8xU16 vb0 = unpack_lo64_u8_u16(loadu_32<Vec8xU16>(u));
            Vec8xU16 vp0 = unpack_lo64_u8_u16(loadu_32<Vec8xU16>(p));

            Vec8xU16 vd0 = v_precalc_d(vb0, vc0);
            va0 = add_i8(v_paeth(va0, vb0, vc0, vd0), vp0);
            vc0 = vb0;

            storeu_32(p, packz_128_u16_u8(va0));

            p += 4;
            u += 4;
            i -= 4;
          }
        }
        else if constexpr (BPP == 6) {
          Vec8xU16 va0 = make_zero<Vec8xU16>();
          Vec8xU16 vc0 = make_zero<Vec8xU16>();

          uint32_t i = bpl;
          while (i >= 12) {
            Vec8xU16 vu0 = loadu_128<Vec8xU16>(u);
            Vec8xU16 vp0 = interleave_lo_u64(loadu_64<Vec8xU16>(p), loadu_32<Vec8xU16>(p + 8));

            Vec8xU16 vu1 = unpack_lo64_u8_u16(srlb_u128<6>(vu0));
            Vec8xU16 vp1 = unpack_lo64_u8_u16(srlb_u128<6>(vp0));

            vu0 = unpack_lo64_u8_u16(vu0);
            vp0 = unpack_lo64_u8_u16(vp0);

            Vec8xU16 vd0 = v_precalc_d(vu0, vc0);
            Vec8xU16 vd1 = v_precalc_d(vu1, vu0);

            vp0 = add_i8(v_paeth(va0, vu0, vc0, vd0), vp0);
            va0 = add_i8(v_paeth(vp0, vu1, vu0, vd1), vp1);

            vp0 = srlb_u128<2>(packz_128_u16_u8(sllb_u128<4>(vp0), va0));
            vc0 = vu1;

            storeu_64(p + 0, vp0);
            storeu_32(p + 8, swizzle_u32<2, 2, 2, 2>(vp0));

            p += 12;
            u += 12;
            i -= 12;
          }

          if (i) {
            // Must hold as `bytes_per_line % 6 == 0`
            BL_ASSERT(i == 6);

            Vec8xU16 vb0 = loadu_64_u8_u16<Vec8xU16>(u);
            Vec8xU16 vp0 = loadu_64<Vec8xU16>(p - 2);
            Vec8xU16 vd0 = v_precalc_d(vb0, vc0);

            vp0 = add_i8(vp0, slli_u64<16>(packz_128_u16_u8(v_paeth(va0, vb0, vc0, vd0))));
            storeu_64(p - 2, vp0);
            p += 6;
          }
        }
        else if constexpr (BPP == 8) {
          Vec8xU16 va0 = make_zero<Vec8xU16>();
          Vec8xU16 vc0 = make_zero<Vec8xU16>();

          uint32_t i = bpl;
          while (i >= 16) {
            Vec8xU16 vu0 = loadu_128<Vec8xU16>(u);
            Vec8xU16 vp0 = loadu_128<Vec8xU16>(p);

            Vec8xU16 vu1 = unpack_hi64_u8_u16(vu0);
            Vec8xU16 vp1 = unpack_hi64_u8_u16(vp0);

            vu0 = unpack_lo64_u8_u16(vu0);
            vp0 = unpack_lo64_u8_u16(vp0);

            Vec8xU16 vd0 = v_precalc_d(vu0, vc0);
            Vec8xU16 vd1 = v_precalc_d(vu1, vu0);

            vp0 = add_i8(v_paeth(va0, vu0, vc0, vd0), vp0);
            va0 = add_i8(v_paeth(vp0, vu1, vu0, vd1), vp1);

            vp0 = packz_128_u16_u8(vp0, va0);
            vc0 = vu1;

            storeu_128(p, vp0);

            p += 16;
            u += 16;
            i -= 16;
          }

          if (i) {
            // Must hold as `bytes_per_line % 8 == 0`
            BL_ASSERT(i == 8);

            Vec8xU16 vb0 = loadu_64_u8_u16<Vec8xU16>(u);
            Vec8xU16 vp0 = loadu_64_u8_u16<Vec8xU16>(p);
            Vec8xU16 vd0 = v_precalc_d(vb0, vc0);

            va0 = add_i8(v_paeth(va0, vb0, vc0, vd0), vp0);
            storeu_64(p, packz_128_u16_u8(va0));
            p += 8;
          }
        }
        break;
      }

      // This filter is artificial and only possible for the very first row, so there is no need to have it optimized.
      case kFilterTypeAvg0: {
        for (uint32_t i = bpl - BPP; i != 0; i--, p++)
          p[BPP] = apply_sum_filter(p[BPP], p[0] >> 1);

        p += BPP;
        break;
      }

      case kFilterTypeNone:
      default:
        p += bpl;
        break;
    }

    if (--y == 0)
      break;

    u = p - bpl;
    filter_type = *p++;

    if (filter_type >= kFilterTypeCount)
      filter_type = kFilterTypeNone;
  }

  return BL_SUCCESS;
}

void init_simd_functions(FunctionTable& ft) noexcept {
  ft.inverse_filter[1] = inverse_filter_simd_impl<1>;
  ft.inverse_filter[2] = inverse_filter_simd_impl<2>;
  ft.inverse_filter[3] = inverse_filter_simd_impl<3>;
  ft.inverse_filter[4] = inverse_filter_simd_impl<4>;
  ft.inverse_filter[6] = inverse_filter_simd_impl<6>;
  ft.inverse_filter[8] = inverse_filter_simd_impl<8>;
}

} // {anonymous}
} // {bl::Png::Ops}

//! \}
//! \endcond

#endif // BLEND2D_CODEC_PNGOPSSIMDIMPL_P_H_INCLUDED
