// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_OPENTYPE_OTGLYFSIMDIMPL_P_H_INCLUDED
#define BLEND2D_OPENTYPE_OTGLYFSIMDIMPL_P_H_INCLUDED

#include <blend2d/core/font_p.h>
#include <blend2d/core/matrix_p.h>
#include <blend2d/core/path_p.h>
#include <blend2d/geometry/commons_p.h>
#include <blend2d/opentype/otface_p.h>
#include <blend2d/opentype/otglyf_p.h>
#include <blend2d/opentype/otglyfsimddata_p.h>
#include <blend2d/simd/simd_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/memops_p.h>
#include <blend2d/support/ptrops_p.h>
#include <blend2d/support/scopedbuffer_p.h>
#include <blend2d/tables/tables_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_opentype_impl
//! \{

namespace bl::OpenType {
namespace GlyfImpl {

// bl::OpenType::GlyfImpl - GetGlyphOutlinesSimdImpl [SSE4.2 & AVX2 & ASIMD]
// =========================================================================

namespace {

using namespace SIMD;

// There are some differences between X86 and ARM we have to address. In general the implementation is pretty
// similar, however, extracting MSB bits from 8-bit elements of a vector is different and in general ARM lacks
// some instructions that X86 supports natively, and doing a full emulation just is not good for performance.
//
// So, instead of a full emulation, we provide two implementations for X86 and ARM that use a slightly different
// approach, but the result is the same.
#if BL_TARGET_ARCH_X86

// X86 implementation uses `extract_sign_bits_i8()`, which mask to [V]PMOVMSKB, so we get each MSB as a single bit.
struct RepeatFlagMask {
  uint32_t pred;

  BL_INLINE_NODEBUG bool has_repeats() const noexcept { return pred != 0u; }
  BL_INLINE_NODEBUG bool has_repeats_in_lo8_flags() const noexcept { return (pred & 0xFFu) != 0u; }
};

static BL_INLINE RepeatFlagMask calc_repeat_flag_mask(const Vec16xU8& vf) noexcept {
  return RepeatFlagMask{extract_sign_bits_i8(vf)};
}

struct OffCurveSplineAcc {
  uint32_t count = 0;

  BL_INLINE_NODEBUG void accumulate_all_flags(const Vec16xU8& vf) noexcept { count += IntOps::pop_count(extract_sign_bits_i8(vf)); }
  BL_INLINE_NODEBUG void accumulateLo8Flags(const Vec16xU8& vf) noexcept { count += IntOps::pop_count(extract_sign_bits_i8(vf) & 0xFFu); }

  BL_INLINE_NODEBUG size_t get() const noexcept { return count; }
};

#elif BL_TARGET_ARCH_ARM

// ARM implementation uses narrowing shift to pack 2x8 bits into 2x4 bits, which can be then converted to a
// GP register predicate, which we can test. This is a pretty good approach that expands to only slightly more
// instructions than X86 approach.
#if BL_TARGET_ARCH_ARM >= 64
struct RepeatFlagMask {
  uint64_t pred;

  BL_INLINE_NODEBUG bool has_repeats() const noexcept { return pred != 0u; }
  BL_INLINE_NODEBUG bool has_repeats_in_lo8_flags() const noexcept { return (pred & 0xFFFFFFFF) != 0u; }
};

static BL_INLINE RepeatFlagMask calc_repeat_flag_mask(const Vec16xU8& vf) noexcept {
  uint64x1_t bits = simd_u64(vshrn_n_u16(simd_u16(srai_i8<7>(vf).v), 4));
  return RepeatFlagMask{vget_lane_u64(bits, 0)};
}
#else
struct RepeatFlagMask {
  uint32_t pred_lo, pred_hi;

  BL_INLINE_NODEBUG bool has_repeats() const noexcept { return (pred_lo | pred_hi) != 0u; }
  BL_INLINE_NODEBUG bool has_repeats_in_lo8_flags() const noexcept { return pred_lo != 0u; }
};

static BL_INLINE RepeatFlagMask calc_repeat_flag_mask(const Vec16xU8& vf) noexcept {
  uint32x2_t bits = simd_u32(vshrn_n_u16(simd_u16(srai_i8<7>(vf).v), 4));
  return RepeatFlagMask{vget_lane_u32(bits, 0), vget_lane_u32(bits, 1)};
}
#endif

struct OffCurveSplineAcc {
  Vec8xU16 acc;

  BL_INLINE_NODEBUG void accumulate_all_flags(const Vec16xU8& vf) noexcept {
    Vec16xU8 bits = srli_u8<7>(vf);
    acc = addw_lo_u8_to_u16(acc, bits);
    acc = addw_hi_u8_to_u16(acc, bits);
  }

  BL_INLINE_NODEBUG void accumulateLo8Flags(const Vec16xU8& vf) noexcept {
    Vec16xU8 bits = srli_u8<7>(vf);
    acc = addw_lo_u8_to_u16(acc, bits);
  }

  BL_INLINE_NODEBUG size_t get() const noexcept {
    uint32x4_t sum_q = vaddl_u16(vget_low_u16(acc.v), vget_high_u16(acc.v));
#if BL_TARGET_ARCH_ARM >= 64
    return vaddvq_u32(sum_q);
#else
    uint32x2_t sum_d = vadd_u32(vget_low_u32(sum_q), vget_high_u32(sum_q));
    sum_d = vadd_u32(sum_d, vrev64_u32(sum_d));
    return vget_lane_u32(sum_d, 0);
#endif
  }
};

#else
#error "bl::OpenType::GlyfImpl - missing support for target architecture"
#endif

// Converts TrueType glyph flags:
//
//   [0|0|YSame|XSame|Repeat|YByte|XByte|OnCurve]
//
// To an internal representation used by SIMD code:
//
//   [Repeat|!OnCurve|OnCurve|0|!YSame|!XSame|YByte|XByte]
static BL_INLINE Vec16xU8 convert_flags(const Vec16xU8& vf, const Vec16xU8& vConvertFlagsPredicate, const Vec16xU8& v0x3030) noexcept {
  Vec16xU8 a = swizzlev_u8(vConvertFlagsPredicate, vf);
  Vec16xU8 b = srli_u16<2>((vf & v0x3030));
  return a ^ b;
}

static BL_INLINE VecPair<Vec16xU8> aggregate_vertex_sizes(const Vec16xU8& vf, const Vec16xU8& vSizesPerXYPredicate, const Vec16xU8& v0x0F0F) noexcept {
  Vec16xU8 yx_sizes = swizzlev_u8(vSizesPerXYPredicate, vf); // [H   G   F   E   D   C   B   A]

  yx_sizes = yx_sizes + slli_i64<8>(yx_sizes);                 // [H:G G:F F:E E:D D:C C:B B:A A]
  yx_sizes = yx_sizes + slli_i64<16>(yx_sizes);                // [H:E G:D F:C E:B D:A C:A B:A A]

  Vec16xU8 y_sizes = srli_u64<4>(yx_sizes) & v0x0F0F;         // Y sizes separated from YX sizes.
  Vec16xU8 x_sizes = yx_sizes & v0x0F0F;                      // X sizes separated from YX sizes.

  y_sizes = y_sizes + slli_i64<32>(y_sizes);                   // [H:A G:A F:A E:A D:A C:A B:A A]
  x_sizes = x_sizes + slli_i64<32>(x_sizes);                   // [H:A G:A F:A E:A D:A C:A B:A A]

  return VecPair<Vec16xU8>{x_sizes, y_sizes};
}

static BL_INLINE Vec4xU32 sumsFromAggregatedSizesOf8Bytes(const VecPair<Vec16xU8>& sizes) noexcept {
  return vec_u32(srli_u64<56>(shuffle_u32<1, 3, 1, 3>(sizes[0], sizes[1])));
}

static BL_INLINE Vec4xU32 sumsFromAggregatedSizesOf16Bytes(const VecPair<Vec16xU8>& sizes) noexcept {
  return vec_u32(srli_u32<24>(shuffle_u32<1, 3, 1, 3>(sizes[0], sizes[1])));
}

struct DecodedVertex {
  int16_t x;
  int16_t y;
};

static BL_INLINE Vec2xF64 transform_decoded_vertex(const DecodedVertex* decoded_vertex, const Vec2xF64& m00_m11, const Vec2xF64& m10_m01) noexcept {
  Vec4xI32 xy_i32 = vec_i32(unpack_lo64_i16_i32(loada_32<Vec8xI16>(decoded_vertex)));

  Vec2xF64 xy_f64 = cvt_2xi32_f64(xy_i32);
  Vec2xF64 yx_f64 = swap_f64(xy_f64);

  return xy_f64 * m00_m11 + yx_f64 * m10_m01;
}

static BL_INLINE void store_vertex(PathAppender& appender, uint8_t cmd, const Vec2xF64& vtx) noexcept {
  appender.cmd[0].value = cmd;
  storeu(appender.vtx, vtx);
}

static BL_INLINE void append_vertex(PathAppender& appender, uint8_t cmd, const Vec2xF64& vtx) noexcept {
  store_vertex(appender, cmd, vtx);
  appender._advance(1);
}

static BL_INLINE void appendVertex2x(PathAppender& appender, uint8_t cmd0, const Vec2xF64& vtx0, uint8_t cmd1, const Vec2xF64& vtx1) noexcept {
  appender.cmd[0].value = cmd0;
  appender.cmd[1].value = cmd1;
  storeu(appender.vtx + 0, vtx0);
  storeu(appender.vtx + 1, vtx1);
  appender._advance(2);
}

static BL_INLINE BLResult get_glyph_outlines_simd_impl(
  const BLFontFaceImpl* face_impl,
  BLGlyphId glyph_id,
  const BLMatrix2D* transform,
  BLPath* out,
  size_t* contour_count_out,
  ScopedBuffer* tmp_buffer) noexcept {

  using namespace SIMD;

  const OTFaceImpl* ot_face_impl = static_cast<const OTFaceImpl*>(face_impl);

  typedef GlyfTable::Simple Simple;
  typedef GlyfTable::Compound Compound;

  if (BL_UNLIKELY(glyph_id >= ot_face_impl->face_info.glyph_count))
    return bl_make_error(BL_ERROR_INVALID_GLYPH);

  RawTable glyf_table = ot_face_impl->glyf.glyf_table;
  RawTable loca_table = ot_face_impl->glyf.loca_table;
  uint32_t loca_offset_size = ot_face_impl->loca_offset_size();

  const uint8_t* gPtr = nullptr;
  size_t remaining_size = 0;
  size_t compound_level = 0;

  // Only matrix and compound_flags are important in the root entry.
  CompoundEntry compound_data[CompoundEntry::kMaxLevel];
  compound_data[0].gPtr = nullptr;
  compound_data[0].remaining_size = 0;
  compound_data[0].compound_flags = Compound::kArgsAreXYValues;
  compound_data[0].transform = *transform;

  PathAppender appender;
  size_t contour_count_total = 0;

  for (;;) {
    size_t offset;
    size_t end_off;
    size_t remaining_size_after_glyph_data;

    // NOTE: Maximum glyph_id is 65535, so we are always safe here regarding multiplying the `glyph_id` by 2 or 4
    // to calculate the correct index.
    if (loca_offset_size == 2) {
      size_t index = size_t(glyph_id) * 2u;
      if (BL_UNLIKELY(index + sizeof(UInt16) * 2u > loca_table.size))
        goto InvalidData;
      offset = uint32_t(reinterpret_cast<const UInt16*>(loca_table.data + index + 0)->value()) * 2u;
      end_off = uint32_t(reinterpret_cast<const UInt16*>(loca_table.data + index + 2)->value()) * 2u;
    }
    else {
      size_t index = size_t(glyph_id) * 4u;
      if (BL_UNLIKELY(index + sizeof(UInt32) * 2u > loca_table.size))
        goto InvalidData;
      offset = reinterpret_cast<const UInt32*>(loca_table.data + index + 0)->value();
      end_off = reinterpret_cast<const UInt32*>(loca_table.data + index + 4)->value();
    }

    remaining_size_after_glyph_data = glyf_table.size - end_off;

    // Simple or Empty Glyph
    // ---------------------

    if (BL_UNLIKELY(offset >= end_off || end_off > glyf_table.size)) {
      // Only ALLOWED when `offset == end_off`.
      if (BL_UNLIKELY(offset != end_off || end_off > glyf_table.size))
        goto InvalidData;
    }
    else {
      gPtr = glyf_table.data + offset;
      remaining_size = end_off - offset;

      if (BL_UNLIKELY(remaining_size < sizeof(GlyfTable::GlyphData)))
        goto InvalidData;

      int contour_count_signed = reinterpret_cast<const GlyfTable::GlyphData*>(gPtr)->number_of_contours();
      if (contour_count_signed > 0) {
        size_t contour_count = size_t(unsigned(contour_count_signed));
        bl::OverflowFlag of{};

        // Minimum data size is:
        //   10                     [GlyphData header]
        //   (number_of_contours * 2) [end_pts_of_contours]
        //   2                      [instruction_length]
        gPtr += sizeof(GlyfTable::GlyphData);
        remaining_size = IntOps::sub_overflow(remaining_size, sizeof(GlyfTable::GlyphData) + contour_count * 2u + 2u, &of);
        if (BL_UNLIKELY(of))
          goto InvalidData;

        const UInt16* contour_array = reinterpret_cast<const UInt16*>(gPtr);
        gPtr += contour_count * 2u;
        contour_count_total += contour_count;

        // We don't use hinting instructions, so skip them.
        size_t instruction_count = MemOps::readU16uBE(gPtr);
        remaining_size = IntOps::sub_overflow(remaining_size, instruction_count, &of);
        if (BL_UNLIKELY(of))
          goto InvalidData;

        gPtr += 2u + instruction_count;
        const uint8_t* gEnd = gPtr + remaining_size;

        // Number of vertices in TrueType sense (could be less than a number of points required by BLPath
        // representation, especially if TT outline contains consecutive off-curve points).
        size_t tt_vertex_count = size_t(contour_array[contour_count - 1].value()) + 1u;

        // Only try to decode vertices if there is more than 1.
        if (tt_vertex_count > 1u) {
          // Read TrueType Flags Data
          // ------------------------

          // We need 3 temporary buffers:
          //
          //  - fDataPtr - Converted flags data. These flags represent the same flags as used by TrueType, however,
          //               the bits representing each value are different so they can be used in VPSHUFB/TBL.
          //  - xPredPtr - Buffer that is used to calculate predicates for X coordinates.
          //  - yPredPtr - Buffer that is used to calculate predicates for Y coordinates.
          //
          // The `xPredPtr` and `yPredPtr` buffers contain data grouped for 8 flags. Each byte contains the side of
          // the coordinate (either 0, 1, or 2 bytes are used in TrueType data) aggregated in the following way:
          //
          // Input coordinate sizes     = [A B C D E F G H]
          // Aggregated in [x|y]PredPtr = [A A+B A+B+C A+B+C+D A+B+C+D+E A+B+C+D+E+F A+B+C+D+E+F+G A+B+C+D+E+F+G+H]
          //
          // The aggregated sizes are very useful, because they describe where each vertex starts in decode buffer.

#ifdef BL_TARGET_OPT_AVX2
          static constexpr uint32_t kDataAlignment = 32;
#else
          static constexpr uint32_t kDataAlignment = 16;
#endif

          uint8_t* fDataPtr = static_cast<uint8_t*>(tmp_buffer->alloc(tt_vertex_count * 3 + kDataAlignment * 6));
          if (BL_UNLIKELY(!fDataPtr))
            return bl_make_error(BL_ERROR_OUT_OF_MEMORY);

          fDataPtr = IntOps::align_up(fDataPtr, kDataAlignment);
          uint8_t* xPredPtr = fDataPtr + IntOps::align_up(tt_vertex_count, kDataAlignment) + kDataAlignment;
          uint8_t* yPredPtr = xPredPtr + IntOps::align_up(tt_vertex_count, kDataAlignment) + kDataAlignment;

          // Sizes of x_coordinates[] and y_coordinates[] arrays in TrueType data.
          size_t xCoordinatesSize;
          size_t yCoordinatesSize;

          OffCurveSplineAcc off_curve_spline_acc;
          size_t off_curve_spline_count;

          {
            Vec16xU8 v0x3030 = common_table.p_3030303030303030.as<Vec16xU8>();
            Vec16xU8 v0x0F0F = common_table.p_0F0F0F0F0F0F0F0F.as<Vec16xU8>();
            Vec16xU8 v0x8080 = common_table.p_8080808080808080.as<Vec16xU8>();
            Vec16xU8 vSizesPerXYPredicate = loada<Vec16xU8>(sizesPerXYPredicate);
            Vec16xU8 vConvertFlagsPredicate = loada<Vec16xU8>(convert_flags_predicate);

            Vec4xU32 vSumXY = make_zero<Vec4xU32>();
            Vec16xU8 vPrevFlags = make_zero<Vec16xU8>();

            size_t i = 0;

            // We want to read 16 bytes in main loop. This means that in the worst case we will read more than 15 bytes
            // than necessary (if reading a last flag via a 16-byte load). We must make sure that there are such bytes.
            // Instead of doing such checks in a loop, we check it here and go to the slow loop if we are at the end of
            // glyph table and 16-byte loads would read beyond. It's very unlikely, but we have to make sure it won't
            // happen.
            size_t slow_flags_decode_finished_check = IntOps::all_ones<size_t>();
            if (remaining_size + remaining_size_after_glyph_data < tt_vertex_count + 15)
              goto SlowFlagsDecode;

            // There is some space ahead, so try to leave slow flags decode loop after an 8-flag chunk has been decoded.
            slow_flags_decode_finished_check = 0;

            do {
              {
                size_t n = bl_min<size_t>(tt_vertex_count - i, 16u);

                Vec16xU8 vp = loadu<Vec16xU8>(overflow_flags_predicate + 16u - n);
                Vec16xU8 vf = swizzlev_u8(convert_flags(loadu<Vec16xU8>(gPtr - 16 + n), vConvertFlagsPredicate, v0x3030), vp);

                RepeatFlagMask repeat_flag_mask = calc_repeat_flag_mask(vf);
                Vec16xU8 quad_splines = (alignr_u128<15>(vf, vPrevFlags) + vf) & v0x8080;
                VecPair<Vec16xU8> vertex_sizes = aggregate_vertex_sizes(vf, vSizesPerXYPredicate, v0x0F0F);

                // Lucky if there are no repeats in 16 flags.
                if (!repeat_flag_mask.has_repeats()) {
                  off_curve_spline_acc.accumulate_all_flags(quad_splines);
                  vPrevFlags = vf;
                  vf |= srli_u16<3>(quad_splines);

                  storeu(fDataPtr + i, vf);
                  storeu(xPredPtr + i, vertex_sizes[0]);
                  storeu(yPredPtr + i, vertex_sizes[1]);

                  i += n;
                  gPtr += n;
                  vSumXY += sumsFromAggregatedSizesOf16Bytes(vertex_sizes);
                  continue;
                }

                // Still a bit lucky if there are no repeats in the first 8 flags.
                if (!repeat_flag_mask.has_repeats_in_lo8_flags()) {
                  // NOTE: Must be greater than 8 as all flags that overflow the flag count are non repeating.
                  BL_ASSERT(n >= 8);

                  off_curve_spline_acc.accumulateLo8Flags(quad_splines);
                  vPrevFlags = sllb_u128<8>(vf);
                  vf |= srli_u16<3>(quad_splines);

                  storeu_64(fDataPtr + i, vf);
                  storeu_64(xPredPtr + i, vertex_sizes[0]);
                  storeu_64(yPredPtr + i, vertex_sizes[1]);

                  i += 8;
                  gPtr += 8;
                  vSumXY += sumsFromAggregatedSizesOf8Bytes(vertex_sizes);
                }
              }

              // Slow loop, processes repeating flags in 8-flag chunks. The first chunk that is non-repeating goes back
              // to the fast loop. This loop can be slow as it's not common to have many repeating flags. Some glyphs
              // have no repeating flags at all, and some have less than 2. It's very unlikely to hit this loop often.
SlowFlagsDecode:
              {
                size_t slow_index = i;

                // First expand all repeated flags to fDataPtr[] array - X/Y data will be calculated once we have flags expanded.
                do {
                  if (BL_UNLIKELY(gPtr == gEnd))
                    goto InvalidData;

                  // Repeated flag?
                  uint32_t f = convert_flags_predicate[*gPtr++ & Simple::kImportantFlagsMask];

                  if (f & kVecFlagRepeat) {
                    if (BL_UNLIKELY(gPtr == gEnd))
                      goto InvalidData;

                    size_t n = *gPtr++;
                    f ^= kVecFlagRepeat;

                    if (BL_UNLIKELY(n >= tt_vertex_count - i))
                      goto InvalidData;

                    MemOps::fill_small(fDataPtr + i, uint8_t(f), n);
                    i += n;
                  }

                  fDataPtr[i++] = uint8_t(f);
                } while ((i & 0x7u) != slow_flags_decode_finished_check && i != tt_vertex_count);

                // We want to process 16 flags at a time in the next loop, however, we cannot have garbage in fDataPtr[]
                // as each byte contributes to vertex sizes we calculate out of flags. So explicitly zero the next 16
                // bytes to make sure there is no garbage.
                storeu(fDataPtr + i, make_zero<Vec16xU8>());

                // Calculate vertex sizes and off-curve spline bits of all expanded flags.
                do {
                  Vec16xU8 vf = loadu<Vec16xU8>(fDataPtr + slow_index);
                  Vec16xU8 quad_splines = (alignr_u128<15>(vf, vPrevFlags) + vf) & v0x8080;
                  off_curve_spline_acc.accumulate_all_flags(quad_splines);

                  vPrevFlags = vf;
                  vf |= srli_u16<3>(quad_splines);

                  VecPair<Vec16xU8> vertex_sizes = aggregate_vertex_sizes(vf, vSizesPerXYPredicate, v0x0F0F);
                  storeu(fDataPtr + slow_index, vf);
                  storeu(xPredPtr + slow_index, vertex_sizes[0]);
                  storeu(yPredPtr + slow_index, vertex_sizes[1]);

                  slow_index += 16;
                  vSumXY += sumsFromAggregatedSizesOf16Bytes(vertex_sizes);
                } while (slow_index < i);

                // Processed more flags than necessary? Correct vPrevFlags to make off-curve calculations correct.
                if (slow_index > i)
                  vPrevFlags = sllb_u128<8>(vPrevFlags);
              }
            } while (i < tt_vertex_count);

            // Finally, calculate the size of x_coordinates[] and y_coordinates[] arrays.
            vSumXY += srli_u64<32>(vSumXY);
            xCoordinatesSize = extract_u16<0>(vSumXY);
            yCoordinatesSize = extract_u16<4>(vSumXY);
          }

          off_curve_spline_count = off_curve_spline_acc.get();
          remaining_size = IntOps::sub_overflow(PtrOps::bytes_until(gPtr, gEnd), xCoordinatesSize + yCoordinatesSize, &of);

          // This makes the static analysis happy about not using `remaining_size` afterwards. We don't want to discard
          // the result of the subtraction as that could possibly introduce an issue in the future if code depending
          // on `remaining_size` is added below.
          bl_unused(remaining_size);

          if (BL_UNLIKELY(of))
            goto InvalidData;

          // Read TrueType Vertex Data
          // -------------------------

          // Vertex data in `glyf` table doesn't map 1:1 to how BLPath stores its data. Multiple off-point curves in
          // TrueType data are decomposed into a quad spline, which is one vertex larger (BLPath doesn't offer multiple
          // off-point quads). This means that the number of vertices required by BLPath can be greater than the number
          // of vertices stored in TrueType 'glyf' data. However, we should know exactly how many vertices we have to
          // add to `tt_vertex_count` as we calculated `off_curve_spline_count` during flags decoding.
          //
          // The number of resulting vertices is thus:
          //   - `tt_vertex_count` - base number of vertices stored in TrueType data.
          //   - `off_curve_spline_count` - the number of additional vertices we will need to add for each off-curve spline
          //     used in TrueType data.
          //   - `contour_count` - Number of contours, we multiply this by 3 as we want to include one 'MoveTo', 'Close',
          //     and one additional off-curve spline point per each contour in case it starts - ends with an off-curve
          //     point.
          //   - 16 extra vertices for SIMD stores and to prevent `decoded_vertex_array` overlapping BLPath data.
          size_t max_vertex_count = tt_vertex_count + off_curve_spline_count + contour_count * 3 + 16;

          // Increase max_vertex_count if the path was not allocated yet - this avoids a possible realloc of compound glyphs.
          if (out->capacity() == 0 && compound_level > 0)
            max_vertex_count += 128;

          BL_PROPAGATE(appender.begin_append(out, max_vertex_count));

          // Temporary data where 16-bit coordinates (per X and Y) are stored before they are converted to double precision.
          DecodedVertex* decoded_vertex_array = IntOps::align_up(
            reinterpret_cast<DecodedVertex*>(appender.vtx + max_vertex_count) - IntOps::align_up(tt_vertex_count, 16) - 4, 16);

          {
            // Since we know exactly how many bytes both vertex arrays consume we can decode both X and Y coordinates at
            // the same time. This gives us also the opportunity to start appending to BLPath immediately.
            const uint8_t* y_ptr = gPtr + xCoordinatesSize;

            // LO+HI predicate is added to interleaved predicates.
            Vec16xU8 vLoHiPredInc = make128_u16<Vec16xU8>(uint16_t(0x0041u));

            // These are predicates we need to combine with x_pred and y_pred to get the final predicate for VPSHUFB/TBL.
            Vec16xU8 vDecodeOpXImm = loada<Vec16xU8>(decodeOpXTable);
            Vec16xU8 vDecodeOpYImm = loada<Vec16xU8>(decodeOpYTable);

            // NOTE: It's super unlikely that there won't be 16 bytes available after the end of x/y coordinates. Basically
            // only last glyph could be affected. However, we still need to check whether the bytes are there as we cannot just
            // read outside of the glyph table.
            if (BL_LIKELY(remaining_size_after_glyph_data >= 16)) {
              // Common case - uses at most 16-byte reads ahead, processes 16 vertices at a time.
#ifdef BL_TARGET_OPT_AVX2
              Vec32xU8 vLoHiPredInc256 = broadcast_i128<Vec32xU8>(vLoHiPredInc);
              size_t i = 0;

              // Process 32 vertices at a time.
              if (tt_vertex_count > 16) {
                Vec32xU8 vDecodeOpXImm256 = broadcast_i128<Vec32xU8>(vDecodeOpXImm);
                Vec32xU8 vDecodeOpYImm256 = broadcast_i128<Vec32xU8>(vDecodeOpYImm);

                do {
                  Vec16xU8 xVerticesInitial0 = loadu<Vec16xU8>(gPtr);
                  Vec16xU8 yVerticesInitial0 = loadu<Vec16xU8>(y_ptr);

                  gPtr += xPredPtr[i + 7];
                  y_ptr += yPredPtr[i + 7];

                  Vec32xU8 fData = loada<Vec32xU8>(fDataPtr + i);
                  Vec32xU8 x_pred = slli_i64<8>(loada<Vec32xU8>(xPredPtr + i));
                  Vec32xU8 y_pred = slli_i64<8>(loada<Vec32xU8>(yPredPtr + i));

                  x_pred += swizzlev_u8(vDecodeOpXImm256, fData);
                  y_pred += swizzlev_u8(vDecodeOpYImm256, fData);

                  Vec16xU8 xVerticesInitial1 = loadu<Vec16xU8>(gPtr);
                  Vec16xU8 yVerticesInitial1 = loadu<Vec16xU8>(y_ptr);

                  gPtr += xPredPtr[i + 15];
                  y_ptr += yPredPtr[i + 15];

                  Vec32xU8 xPred0 = interleave_lo_u8(x_pred, x_pred);
                  Vec32xU8 xPred1 = interleave_hi_u8(x_pred, x_pred);
                  Vec32xU8 yPred0 = interleave_lo_u8(y_pred, y_pred);
                  Vec32xU8 yPred1 = interleave_hi_u8(y_pred, y_pred);

                  Vec16xI16 xVertices0 = make256_128<Vec16xI16>(loadu<Vec16xU8>(gPtr), xVerticesInitial0);
                  Vec16xI16 yVertices0 = make256_128<Vec16xI16>(loadu<Vec16xU8>(y_ptr), yVerticesInitial0);

                  gPtr += xPredPtr[i + 23];
                  y_ptr += yPredPtr[i + 23];

                  xPred0 += vLoHiPredInc256;
                  xPred1 += vLoHiPredInc256;
                  yPred0 += vLoHiPredInc256;
                  yPred1 += vLoHiPredInc256;

                  Vec16xI16 xVertices1 = make256_128<Vec16xI16>(loadu<Vec16xU8>(gPtr), xVerticesInitial1);
                  Vec16xI16 yVertices1 = make256_128<Vec16xI16>(loadu<Vec16xU8>(y_ptr), yVerticesInitial1);

                  gPtr += xPredPtr[i + 31];
                  y_ptr += yPredPtr[i + 31];

                  xVertices0 = swizzlev_u8(xVertices0, xPred0);
                  yVertices0 = swizzlev_u8(yVertices0, yPred0);
                  xVertices1 = swizzlev_u8(xVertices1, xPred1);
                  yVertices1 = swizzlev_u8(yVertices1, yPred1);

                  xPred0 = srai_i16<15>(slli_i16<2>(xPred0));
                  yPred0 = srai_i16<15>(slli_i16<2>(yPred0));
                  xPred1 = srai_i16<15>(slli_i16<2>(xPred1));
                  yPred1 = srai_i16<15>(slli_i16<2>(yPred1));

                  xVertices0 = (xVertices0 ^ vec_i16(xPred0)) - vec_i16(xPred0);
                  yVertices0 = (yVertices0 ^ vec_i16(yPred0)) - vec_i16(yPred0);
                  xVertices1 = (xVertices1 ^ vec_i16(xPred1)) - vec_i16(xPred1);
                  yVertices1 = (yVertices1 ^ vec_i16(yPred1)) - vec_i16(yPred1);

                  Vec16xI16 xyInterleavedLo0 = interleave_lo_u16(xVertices0, yVertices0);
                  Vec16xI16 xyInterleavedHi0 = interleave_hi_u16(xVertices0, yVertices0);
                  Vec16xI16 xyInterleavedLo1 = interleave_lo_u16(xVertices1, yVertices1);
                  Vec16xI16 xyInterleavedHi1 = interleave_hi_u16(xVertices1, yVertices1);

                  storea_128(decoded_vertex_array + i +  0, xyInterleavedLo0);
                  storea_128(decoded_vertex_array + i +  4, xyInterleavedHi0);
                  storea_128(decoded_vertex_array + i +  8, xyInterleavedLo1);
                  storea_128(decoded_vertex_array + i + 12, xyInterleavedHi1);
                  storea_128(decoded_vertex_array + i + 16, extract_i128<1>(xyInterleavedLo0));
                  storea_128(decoded_vertex_array + i + 20, extract_i128<1>(xyInterleavedHi0));
                  storea_128(decoded_vertex_array + i + 24, extract_i128<1>(xyInterleavedLo1));
                  storea_128(decoded_vertex_array + i + 28, extract_i128<1>(xyInterleavedHi1));

                  i += 32;
                } while (i < tt_vertex_count - 16);
              }

              // Process remaining 16 vertices.
              if (i < tt_vertex_count) {
                Vec16xU8 fData = loada<Vec16xU8>(fDataPtr + i);
                Vec16xU8 x_pred = slli_i64<8>(loada<Vec16xU8>(xPredPtr + i));
                Vec16xU8 y_pred = slli_i64<8>(loada<Vec16xU8>(yPredPtr + i));

                x_pred += swizzlev_u8(vDecodeOpXImm, fData);
                y_pred += swizzlev_u8(vDecodeOpYImm, fData);

                Vec32xU8 xPred256 = permute_i64<1, 1, 0, 0>(vec_cast<Vec32xU8>(x_pred));
                Vec32xU8 yPred256 = permute_i64<1, 1, 0, 0>(vec_cast<Vec32xU8>(y_pred));

                xPred256 = interleave_lo_u8(xPred256, xPred256);
                yPred256 = interleave_lo_u8(yPred256, yPred256);

                Vec16xU8 xVerticesInitial = loadu<Vec16xU8>(gPtr);
                Vec16xU8 yVerticesInitial = loadu<Vec16xU8>(y_ptr);

                gPtr += xPredPtr[i + 7];
                y_ptr += yPredPtr[i + 7];

                xPred256 += vLoHiPredInc256;
                yPred256 += vLoHiPredInc256;

                Vec16xI16 x_vertices = make256_128<Vec16xI16>(loadu<Vec16xU8>(gPtr), xVerticesInitial);
                Vec16xI16 y_vertices = make256_128<Vec16xI16>(loadu<Vec16xU8>(y_ptr), yVerticesInitial);

                // gPtr/y_ptr is no longer needed, so the following code is not needed as well:
                //   gPtr += xPredPtr[i + 15];
                //   y_ptr += yPredPtr[i + 15];

                x_vertices = swizzlev_u8(x_vertices, xPred256);
                y_vertices = swizzlev_u8(y_vertices, yPred256);

                xPred256 = srai_i16<15>(slli_i16<2>(xPred256));
                yPred256 = srai_i16<15>(slli_i16<2>(yPred256));

                x_vertices = (x_vertices ^ vec_i16(xPred256)) - vec_i16(xPred256);
                y_vertices = (y_vertices ^ vec_i16(yPred256)) - vec_i16(yPred256);

                Vec16xI16 xy_interleaved_lo = interleave_lo_u16(x_vertices, y_vertices);
                Vec16xI16 xy_interleaved_hi = interleave_hi_u16(x_vertices, y_vertices);

                storea_128(decoded_vertex_array + i +  0, xy_interleaved_lo);
                storea_128(decoded_vertex_array + i +  4, xy_interleaved_hi);
                storea_128(decoded_vertex_array + i +  8, extract_i128<1>(xy_interleaved_lo));
                storea_128(decoded_vertex_array + i + 12, extract_i128<1>(xy_interleaved_hi));
              }
#else
              for (size_t i = 0; i < tt_vertex_count; i += 16) {
                Vec16xU8 fData = loada<Vec16xU8>(fDataPtr + i);
                Vec16xU8 x_pred = slli_i64<8>(loada<Vec16xU8>(xPredPtr + i));
                Vec16xU8 y_pred = slli_i64<8>(loada<Vec16xU8>(yPredPtr + i));

                x_pred += swizzlev_u8(vDecodeOpXImm, fData);
                y_pred += swizzlev_u8(vDecodeOpYImm, fData);

                Vec16xU8 xPred0 = interleave_lo_u8(x_pred, x_pred);
                Vec16xU8 xPred1 = interleave_hi_u8(x_pred, x_pred);
                Vec16xU8 yPred0 = interleave_lo_u8(y_pred, y_pred);
                Vec16xU8 yPred1 = interleave_hi_u8(y_pred, y_pred);

                xPred0 += vLoHiPredInc;
                xPred1 += vLoHiPredInc;
                yPred0 += vLoHiPredInc;
                yPred1 += vLoHiPredInc;

                // Process low 8 vertices.
                Vec8xI16 xVertices0 = vec_i16(swizzlev_u8(loadu<Vec16xU8>(gPtr), xPred0));
                Vec8xI16 yVertices0 = vec_i16(swizzlev_u8(loadu<Vec16xU8>(y_ptr), yPred0));

                gPtr += xPredPtr[i + 7];
                y_ptr += yPredPtr[i + 7];

                xPred0 = srai_i16<15>(slli_i16<2>(xPred0));
                yPred0 = srai_i16<15>(slli_i16<2>(yPred0));

                xVertices0 = (xVertices0 ^ vec_i16(xPred0)) - vec_i16(xPred0);
                yVertices0 = (yVertices0 ^ vec_i16(yPred0)) - vec_i16(yPred0);

                storea(decoded_vertex_array + i + 0, interleave_lo_u16(xVertices0, yVertices0));
                storea(decoded_vertex_array + i + 4, interleave_hi_u16(xVertices0, yVertices0));

                // Process high 8 vertices.
                Vec8xI16 xVertices1 = vec_i16(swizzlev_u8(loadu<Vec16xU8>(gPtr), xPred1));
                Vec8xI16 yVertices1 = vec_i16(swizzlev_u8(loadu<Vec16xU8>(y_ptr), yPred1));

                gPtr += xPredPtr[i + 15];
                y_ptr += yPredPtr[i + 15];

                xPred1 = srai_i16<15>(slli_i16<2>(xPred1));
                yPred1 = srai_i16<15>(slli_i16<2>(yPred1));

                xVertices1 = (xVertices1 ^ vec_i16(xPred1)) - vec_i16(xPred1);
                yVertices1 = (yVertices1 ^ vec_i16(yPred1)) - vec_i16(yPred1);

                storea(decoded_vertex_array + i +  8, interleave_lo_u16(xVertices1, yVertices1));
                storea(decoded_vertex_array + i + 12, interleave_hi_u16(xVertices1, yVertices1));
              }
#endif
            }
            else {
              // Restricted case - uses at most 16-byte reads below, we know that there 16 bytes below, because:
              //   - Glyph header       [10 bytes]
              //   - NumberOfContours   [ 2 bytes]
              //   - InstructionLength  [ 2 bytes]
              //   - At least two flags [ 2 bytes] (one flag glyphs are refused as is not enough for a contour)
              for (size_t i = 0; i < tt_vertex_count; i += 8) {
                Vec16xU8 fData = loadu_64<Vec16xU8>(fDataPtr + i);
                Vec16xU8 x_pred = slli_i64<8>(loadu_64<Vec16xU8>(xPredPtr + i));
                Vec16xU8 y_pred = slli_i64<8>(loadu_64<Vec16xU8>(yPredPtr + i));

                size_t xBytesUsed = xPredPtr[i + 7];
                size_t yBytesUsed = yPredPtr[i + 7];

                gPtr += xBytesUsed;
                y_ptr += yBytesUsed;

                x_pred += swizzlev_u8(vDecodeOpXImm, fData);
                y_pred += swizzlev_u8(vDecodeOpYImm, fData);

                x_pred += make128_u8<Vec16xU8>(uint8_t(16u - uint32_t(xBytesUsed)));
                y_pred += make128_u8<Vec16xU8>(uint8_t(16u - uint32_t(yBytesUsed)));

                x_pred = interleave_lo_u8(x_pred, x_pred);
                y_pred = interleave_lo_u8(y_pred, y_pred);

                x_pred += vLoHiPredInc;
                y_pred += vLoHiPredInc;

                Vec8xI16 xVertices0 = vec_i16(swizzlev_u8(loadu<Vec16xU8>(gPtr - 16), x_pred));
                Vec8xI16 yVertices0 = vec_i16(swizzlev_u8(loadu<Vec16xU8>(y_ptr - 16), y_pred));

                x_pred = srai_i16<15>(slli_i16<2>(x_pred));
                y_pred = srai_i16<15>(slli_i16<2>(y_pred));

                xVertices0 = (xVertices0 ^ vec_i16(x_pred)) - vec_i16(x_pred);
                yVertices0 = (yVertices0 ^ vec_i16(y_pred)) - vec_i16(y_pred);

                storea(decoded_vertex_array + i + 0, interleave_lo_u16(xVertices0, yVertices0));
                storea(decoded_vertex_array + i + 4, interleave_hi_u16(xVertices0, yVertices0));
              }
            }
          }

          // Affine transform applied to each vertex.
          //
          // NOTE: Compilers are not able to vectorize the computations efficiently, so we do it instead.
          Vec2xF64 m00_m11 = make128_f64(compound_data[compound_level].transform.m11, compound_data[compound_level].transform.m00);
          Vec2xF64 m10_m01 = make128_f64(compound_data[compound_level].transform.m01, compound_data[compound_level].transform.m10);

          // Vertices are stored relative to each other, this is the current point.
          Vec2xF64 current_pt = make128_f64(compound_data[compound_level].transform.m21, compound_data[compound_level].transform.m20);

          // SIMD constants.
          Vec2xF64 half = make128_f64(0.5);

          // Current vertex index in TT sense, advanced until `tt_vertex_count`, which must be end index of the last contour.
          size_t i = 0;

          for (size_t contour_index = 0; contour_index < contour_count; contour_index++) {
            size_t iEnd = size_t(contour_array[contour_index].value()) + 1;
            if (BL_UNLIKELY(iEnd <= i || iEnd > tt_vertex_count))
              goto InvalidData;

            // We do the first vertex here as we want to emit 'MoveTo' and we want to remember it for a possible off-curve
            // start. Currently this means there is some code duplicated for move-to and for other commands, unfortunately.
            uint32_t f = fDataPtr[i];
            current_pt += transform_decoded_vertex(decoded_vertex_array + i, m00_m11, m10_m01);

            if (++i >= iEnd)
              continue;

            // Initial 'MoveTo' coordinates.
            Vec2xF64 initial_pt = current_pt;

            // We need to be able to handle a case in which the contour data starts off-curve.
            size_t starts_on_curve = size_t((f >> kVecFlagOnCurveShift) & 0x1u);
            size_t initial_vertex_index = appender.current_index(*out);

            // Only emit MoveTo here if we don't start off curve, which requires a special care.
            store_vertex(appender, BL_PATH_CMD_MOVE, initial_pt);
            appender._advance(starts_on_curve);

            size_t iEndMinus3 = IntOps::usub_saturate<size_t>(iEnd, 3);

            static constexpr uint32_t kPathCmdFromFlagsShift0 = kVecFlagOnCurveShift;
            static constexpr uint32_t kPathCmdFromFlagsShift1 = kVecFlagOnCurveShift + 8;
            static constexpr uint32_t kPathCmdFromFlagsShift2 = kVecFlagOnCurveShift + 8 + 8;
            static constexpr uint32_t kPathCmdFromFlagsShift3 = kVecFlagOnCurveShift + 8 + 8 + 8;

            static constexpr uint32_t kVecFlagOffSpline0 = uint32_t(kVecFlagOffSpline) << 0;
            static constexpr uint32_t kVecFlagOffSpline1 = uint32_t(kVecFlagOffSpline) << 8;
            static constexpr uint32_t kVecFlagOffSpline2 = uint32_t(kVecFlagOffSpline) << 16;
            static constexpr uint32_t kVecFlagOffSpline3 = uint32_t(kVecFlagOffSpline) << 24;

            // NOTE: This is actually the slowest loop. The 'OffSpline' flag is not easily predictable as it heavily
            // depends on a font face. It's not a rare flag though. If a glyph contains curves there is a high chance
            // that there will be multiple off-curve splines and it's not uncommon to have multiple off-curve splines
            // having more than 3 consecutive off points.
            while (i < iEndMinus3) {
              f = MemOps::readU32u(fDataPtr + i);

              Vec2xF64 d0 = transform_decoded_vertex(decoded_vertex_array + i + 0, m00_m11, m10_m01);
              Vec2xF64 d1 = transform_decoded_vertex(decoded_vertex_array + i + 1, m00_m11, m10_m01);
              Vec2xF64 d2 = transform_decoded_vertex(decoded_vertex_array + i + 2, m00_m11, m10_m01);
              Vec2xF64 d3 = transform_decoded_vertex(decoded_vertex_array + i + 3, m00_m11, m10_m01);
              Vec2xF64 on_pt;

              i += 4;
              current_pt += d0;

              uint32_t path_cmds = (f >> kPathCmdFromFlagsShift0) & 0x03030303u;
              MemOps::writeU32u(appender.cmd, path_cmds);

              if (f & kVecFlagOffSpline0)
                goto EmitSpline0Advance;

              storeu(appender.vtx + 0, current_pt);
              current_pt += d1;
              if (f & kVecFlagOffSpline1)
                goto EmitSpline1Advance;

              storeu(appender.vtx + 1, current_pt);
              current_pt += d2;
              if (f & kVecFlagOffSpline2)
                goto EmitSpline2Advance;

              storeu(appender.vtx + 2, current_pt);
              current_pt += d3;
              if (f & kVecFlagOffSpline3)
                goto EmitSpline3Advance;

              storeu(appender.vtx + 3, current_pt);
              appender._advance(4);
              continue;

EmitSpline0Advance:
              on_pt = current_pt - d0 * half;
              appendVertex2x(appender, BL_PATH_CMD_ON, on_pt, BL_PATH_CMD_QUAD, current_pt);
              current_pt += d1;
              if (f & kVecFlagOffSpline1)
                goto EmitSpline1Continue;

              append_vertex(appender, uint8_t((f >> kPathCmdFromFlagsShift1) & 0x3u), current_pt);
              current_pt += d2;
              if (f & kVecFlagOffSpline2)
                goto EmitSpline2Continue;

              append_vertex(appender, uint8_t((f >> kPathCmdFromFlagsShift2) & 0x3u), current_pt);
              current_pt += d3;
              if (f & kVecFlagOffSpline3)
                goto EmitSpline3Continue;

              append_vertex(appender, uint8_t((f >> kPathCmdFromFlagsShift3) & 0x3u), current_pt);
              continue;

EmitSpline1Advance:
              appender._advance(1);

EmitSpline1Continue:
              on_pt = current_pt - d1 * half;
              appendVertex2x(appender, BL_PATH_CMD_ON, on_pt, BL_PATH_CMD_QUAD, current_pt);
              current_pt += d2;

              if (f & kVecFlagOffSpline2)
                goto EmitSpline2Continue;

              append_vertex(appender, uint8_t((f >> kPathCmdFromFlagsShift2) & 0x3u), current_pt);
              current_pt += d3;

              if (f & kVecFlagOffSpline3)
                goto EmitSpline3Continue;

              append_vertex(appender, uint8_t((f >> kPathCmdFromFlagsShift3) & 0x3u), current_pt);
              continue;

EmitSpline2Advance:
              appender._advance(2);

EmitSpline2Continue:
              on_pt = current_pt - d2 * half;
              appendVertex2x(appender, BL_PATH_CMD_ON, on_pt, BL_PATH_CMD_QUAD, current_pt);
              current_pt += d3;

              if (f & kVecFlagOffSpline3)
                goto EmitSpline3Continue;

              append_vertex(appender, uint8_t((f >> kPathCmdFromFlagsShift3) & 0x3u), current_pt);
              continue;

EmitSpline3Advance:
              appender._advance(3);

EmitSpline3Continue:
              on_pt = current_pt - d3 * half;
              appendVertex2x(appender, BL_PATH_CMD_ON, on_pt, BL_PATH_CMD_QUAD, current_pt);
            }

            while (i < iEnd) {
              f = fDataPtr[i];
              Vec2xF64 delta = transform_decoded_vertex(decoded_vertex_array + i, m00_m11, m10_m01);
              current_pt += delta;
              i++;

              if ((f & kVecFlagOffSpline) == 0) {
                append_vertex(appender, uint8_t(f >> 5), current_pt);
              }
              else {
                Vec2xF64 on_pt = current_pt - delta * half;
                appendVertex2x(appender, BL_PATH_CMD_ON, on_pt, BL_PATH_CMD_QUAD, current_pt);
              }
            }

            f = fDataPtr[i - 1];
            if (!starts_on_curve) {
              BLPathImpl* out_impl = PathInternal::get_impl(out);
              Vec2xF64 final_pt = loadu<Vec2xF64>(out_impl->vertex_data + initial_vertex_index);

              out_impl->command_data[initial_vertex_index] = BL_PATH_CMD_MOVE;

              if (f & kVecFlagOffCurve) {
                Vec2xF64 on_pt = (current_pt + initial_pt) * half;
                append_vertex(appender, BL_PATH_CMD_ON, on_pt);
                final_pt = (initial_pt + final_pt) * half;
              }

              appendVertex2x(appender, BL_PATH_CMD_QUAD, initial_pt, BL_PATH_CMD_ON, final_pt);
            }
            else if (f & kVecFlagOffCurve) {
              append_vertex(appender, BL_PATH_CMD_ON, initial_pt);
            }

            appender.close();
          }
          appender.done(out);
        }
      }
      else if (contour_count_signed == -1) {
        gPtr += sizeof(GlyfTable::GlyphData);
        remaining_size -= sizeof(GlyfTable::GlyphData);

        if (BL_UNLIKELY(++compound_level >= CompoundEntry::kMaxLevel))
          goto InvalidData;

        goto ContinueCompound;
      }
      else {
        // Cannot be less than -1, only -1 specifies compound glyph, lesser value is invalid according to the
        // specification.
        if (BL_UNLIKELY(contour_count_signed < -1))
          goto InvalidData;

        // Otherwise the glyph has no contours.
      }
    }

    // Compound Glyph
    // --------------

    if (compound_level) {
      while (!(compound_data[compound_level].compound_flags & Compound::kMoreComponents))
        if (--compound_level == 0)
          break;

      if (compound_level) {
        gPtr = compound_data[compound_level].gPtr;
        remaining_size = compound_data[compound_level].remaining_size;

        // The structure that we are going to read is as follows:
        //
        //   [Header]
        //     uint16_t flags;
        //     uint16_t glyph_id;
        //
        //   [Translation]
        //     a) int8_t arg1/arg2;
        //     b) int16_t arg1/arg2;
        //
        //   [Scale/Affine]
        //     a) <None>
        //     b) int16_t scale;
        //     c) int16_t scale_x, scale_y;
        //     d) int16_t m00, m01, m10, m11;

ContinueCompound:
        {
          uint32_t flags;
          int arg1, arg2;
          OverflowFlag of{};

          remaining_size = IntOps::sub_overflow<size_t>(remaining_size, 6, &of);
          if (BL_UNLIKELY(of))
            goto InvalidData;

          flags = MemOps::readU16uBE(gPtr);
          glyph_id = MemOps::readU16uBE(gPtr + 2);
          if (BL_UNLIKELY(glyph_id >= ot_face_impl->face_info.glyph_count))
            goto InvalidData;

          arg1 = MemOps::readI8(gPtr + 4);
          arg2 = MemOps::readI8(gPtr + 5);
          gPtr += 6;

          if (flags & Compound::kArgsAreWords) {
            remaining_size = IntOps::sub_overflow<size_t>(remaining_size, 2, &of);
            if (BL_UNLIKELY(of))
              goto InvalidData;

            arg1 = IntOps::shl(arg1, 8) | (arg2 & 0xFF);
            arg2 = MemOps::readI16uBE(gPtr);
            gPtr += 2;
          }

          if (!(flags & Compound::kArgsAreXYValues)) {
            // This makes them unsigned.
            arg1 &= 0xFFFFu;
            arg2 &= 0xFFFFu;

            // TODO: [OpenType] GLYF ArgsAreXYValues not implemented. I don't know how atm.
          }

          constexpr double kScaleF2x14 = 1.0 / 16384.0;

          BLMatrix2D& cm = compound_data[compound_level].transform;
          cm.reset(1.0, 0.0, 0.0, 1.0, double(arg1), double(arg2));

          if (flags & Compound::kAnyCompoundScale) {
            if (flags & Compound::kWeHaveScale) {
              // Simple scaling:
              //   [Sc, 0]
              //   [0, Sc]
              remaining_size = IntOps::sub_overflow<size_t>(remaining_size, 2, &of);
              if (BL_UNLIKELY(of))
                goto InvalidData;

              double scale = double(MemOps::readI16uBE(gPtr)) * kScaleF2x14;
              cm.m00 = scale;
              cm.m11 = scale;
              gPtr += 2;
            }
            else if (flags & Compound::kWeHaveScaleXY) {
              // Simple scaling:
              //   [Sx, 0]
              //   [0, Sy]
              remaining_size = IntOps::sub_overflow<size_t>(remaining_size, 4, &of);
              if (BL_UNLIKELY(of))
                goto InvalidData;

              cm.m00 = double(MemOps::readI16uBE(gPtr + 0)) * kScaleF2x14;
              cm.m11 = double(MemOps::readI16uBE(gPtr + 2)) * kScaleF2x14;
              gPtr += 4;
            }
            else {
              // Affine case:
              //   [A, B]
              //   [C, D]
              remaining_size = IntOps::sub_overflow<size_t>(remaining_size, 8, &of);
              if (BL_UNLIKELY(of))
                goto InvalidData;

              cm.m00 = double(MemOps::readI16uBE(gPtr + 0)) * kScaleF2x14;
              cm.m01 = double(MemOps::readI16uBE(gPtr + 2)) * kScaleF2x14;
              cm.m10 = double(MemOps::readI16uBE(gPtr + 4)) * kScaleF2x14;
              cm.m11 = double(MemOps::readI16uBE(gPtr + 6)) * kScaleF2x14;
              gPtr += 8;
            }

            // Translation scale should only happen when `kArgsAreXYValues` is set. The default behavior according to
            // the specification is `kUnscaledComponentOffset`, which can be overridden by `kScaledComponentOffset`.
            // However, if both or neither are set then the behavior is the same as `kUnscaledComponentOffset`.
            if ((flags & (Compound::kArgsAreXYValues | Compound::kAnyCompoundOffset    )) ==
                         (Compound::kArgsAreXYValues | Compound::kScaledComponentOffset)) {
              // This is what FreeType does and what's not 100% according to the specification. However, according to
              // FreeType this would produce much better offsets so we will match FreeType instead of following the
              // specification.
              cm.m20 *= Geometry::magnitude(BLPoint(cm.m00, cm.m01));
              cm.m21 *= Geometry::magnitude(BLPoint(cm.m10, cm.m11));
            }
          }

          compound_data[compound_level].gPtr = gPtr;
          compound_data[compound_level].remaining_size = remaining_size;
          compound_data[compound_level].compound_flags = flags;
          TransformInternal::multiply(cm, cm, compound_data[compound_level - 1].transform);
          continue;
        }
      }
    }

    break;
  }

  *contour_count_out = contour_count_total;
  return BL_SUCCESS;

InvalidData:
  *contour_count_out = 0;
  return bl_make_error(BL_ERROR_INVALID_DATA);
}

} // {anonymous}
} // {GlyfImpl}
} // {bl::OpenType}

//! \}
//! \endcond

#endif // BLEND2D_OPENTYPE_OTGLYFSIMDIMPL_P_H_INCLUDED
