// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_PIPELINE_REFERENCE_FILLGENERIC_P_H_INCLUDED
#define BLEND2D_PIPELINE_REFERENCE_FILLGENERIC_P_H_INCLUDED

#include <blend2d/pipeline/pipedefs_p.h>
#include <blend2d/pipeline/reference/pixelbufferptr_p.h>
#include <blend2d/support/bitops_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/ptrops_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_pipeline_reference
//! \{

namespace bl::Pipeline::Reference {
namespace {

template<typename CompOp>
struct FillBoxA_Base {
  static void BL_CDECL fill_func(ContextData* ctx_data, const void* fill_data_, const void* fetch_data_) noexcept {
    const FillData::BoxA* fill_data = static_cast<const FillData::BoxA*>(fill_data_);

    intptr_t dst_stride = ctx_data->dst.stride;
    uint8_t* dst_ptr = static_cast<uint8_t*>(ctx_data->dst.pixel_data);

    uint32_t y0 = uint32_t(fill_data->box.y0);
    dst_ptr += intptr_t(y0) * dst_stride;

    uint32_t x0 = uint32_t(fill_data->box.x0);
    dst_ptr += size_t(x0) * CompOp::kDstBPP;

    uint32_t w = uint32_t(fill_data->box.x1) - uint32_t(fill_data->box.x0);
    uint32_t h = uint32_t(fill_data->box.y1) - uint32_t(fill_data->box.y0);

    dst_stride -= intptr_t(size_t(w) * CompOp::kDstBPP);
    uint32_t msk = fill_data->alpha.u;

    CompOp comp_op;
    comp_op.rect_init_fetch(ctx_data, fetch_data_, x0, y0, w);

    BL_ASSUME(h > 0);
    if (CompOp::kOptimizeOpaque && msk == 255) {
      while (h) {
        comp_op.rectStartX(x0);
        dst_ptr = comp_op.compositeCSpanOpaque(dst_ptr, w);
        dst_ptr += dst_stride;
        comp_op.advance_y();
        h--;
      }
    }
    else {
      while (h) {
        comp_op.rectStartX(x0);
        dst_ptr = comp_op.compositeCSpanMasked(dst_ptr, w, msk);
        dst_ptr += dst_stride;
        comp_op.advance_y();
        h--;
      }
    }
  }
};

template<typename CompOp>
struct FillMask_Base {
  static void BL_CDECL fill_func(ContextData* ctx_data, const void* fill_data_, const void* fetch_data_) noexcept {
    const FillData::Mask* fill_data = static_cast<const FillData::Mask*>(fill_data_);

    uint8_t* dst_ptr = static_cast<uint8_t*>(ctx_data->dst.pixel_data);
    intptr_t dst_stride = ctx_data->dst.stride;

    uint32_t y0 = uint32_t(fill_data->box.y0);
    dst_ptr += intptr_t(y0) * dst_stride;

    CompOp comp_op;
    comp_op.spanInitY(ctx_data, fetch_data_, y0);

    uint32_t alpha = fill_data->alpha.u;
    MaskCommand* cmd_ptr = fill_data->mask_command_data;

    uint32_t h = uint32_t(fill_data->box.y1) - y0;

    for (;;) {
      uint32_t x1_and_type = cmd_ptr->_x1_and_type;
      uint32_t x = cmd_ptr->x0();

      MaskCommand* cmd_begin = cmd_ptr;

      // This is not really common to not be true, however, it's possible to skip entire scanlines
      // with kEndOrRepeat command, which is zero.
      if (BL_LIKELY((x1_and_type & MaskCommand::kTypeMask) != 0u)) {
        comp_op.spanStartX(x);
        dst_ptr += size_t(x) * CompOp::kDstBPP;

        uint32_t i = x1_and_type >> MaskCommand::kTypeBits;
        MaskCommandType cmd_type = MaskCommandType(x1_and_type & MaskCommand::kTypeMask);

        i -= x;
        x += i;

        uintptr_t mask_value = cmd_ptr->_value.data;
        cmd_ptr++;

        for (;;) {
          if (cmd_type == MaskCommandType::kCMask) {
            BL_ASSUME(mask_value <= 255);
            dst_ptr = comp_op.compositeCSpan(dst_ptr, i, uint32_t(mask_value));
          }
          else {
            // Increments the advance in the mask command in case it would be repeated.
            cmd_ptr[-1]._value.data = mask_value + uintptr_t(cmd_ptr[-1].mask_advance());
            if (cmd_type == MaskCommandType::kVMaskA8WithoutGA) {
              dst_ptr = comp_op.compositeVSpanWithoutGA(dst_ptr, reinterpret_cast<const uint8_t*>(mask_value), alpha, i);
            }
            else {
              dst_ptr = comp_op.compositeVSpanWithGA(dst_ptr, reinterpret_cast<const uint8_t*>(mask_value), i);
            }
          }

          x1_and_type = cmd_ptr->_x1_and_type;

          // Terminates this command span.
          if ((x1_and_type & MaskCommand::kTypeMask) == 0u)
            break;

          uint32_t x0 = cmd_ptr->x0();
          if (x != x0) {
            comp_op.spanAdvanceX(x0, x0 - x);
            x = x0;
          }

          i = (x1_and_type >> MaskCommand::kTypeBits) - x;
          x += i;
          cmd_type = MaskCommandType(x1_and_type & MaskCommand::kTypeMask);

          mask_value = cmd_ptr->_value.data;
          cmd_ptr++;
        }

        comp_op.spanEndX(x);
        dst_ptr -= size_t(x) * CompOp::kDstBPP;
      }

      uint32_t repeat_count = cmd_ptr->repeat_count();
      if (--h == 0)
        break;

      cmd_ptr++;
      dst_ptr += dst_stride;

      comp_op.advance_y();
      repeat_count--;
      cmd_ptr[-1].update_repeat_count(repeat_count);

      if (repeat_count != 0)
        cmd_ptr = cmd_begin;
    }
  }
};

template<typename CompOp>
struct FillAnalytic_Base {
  enum : uint32_t {
    kDstBPP = CompOp::kDstBPP,
    kPixelsPerOneBit = 4,
    kPixelsPerBitWord = kPixelsPerOneBit * IntOps::bit_size_of<BLBitWord>()
  };

  typedef PrivateBitWordOps BitOps;

  static void BL_CDECL fill_func(ContextData* ctx_data, const void* fill_data_, const void* fetch_data_) noexcept {
    const FillData::Analytic* fill_data = static_cast<const FillData::Analytic*>(fill_data_);

    uint32_t y = uint32_t(fill_data->box.y0);
    intptr_t dst_stride = ctx_data->dst.stride;
    uint8_t* dst_ptr = static_cast<uint8_t*>(ctx_data->dst.pixel_data) + intptr_t(y) * dst_stride;

    BLBitWord* bit_ptr = fill_data->bit_top_ptr;
    BLBitWord* bit_ptr_end = nullptr;
    uint32_t* cell_ptr = fill_data->cell_top_ptr;

    size_t bit_stride = fill_data->bit_stride;
    size_t cell_stride = fill_data->cell_stride;

    uint32_t global_alpha = fill_data->alpha.u;
    uint32_t fill_rule_mask = fill_data->fill_rule_mask;

    CompOp comp_op;
    comp_op.spanInitY(ctx_data, fetch_data_, y);

    y = uint32_t(fill_data->box.y1) - y;

    size_t x0;
    size_t x_end = uint32_t(fill_data->box.x1);
    size_t x_off;

    size_t i;
    uint32_t cov;
    uint32_t msk;

    BLBitWord bit_word;
    BLBitWord bit_word_tmp;

    goto L_Scanline_Init;

    // BitScan
    // -------

    // Called by Scanline iterator on the first non-zero BitWord it matches. The responsibility of BitScan is to find
    // the first bit in the passed BitWord followed by matching the bit that ends this match. This would essentially
    // produce the first [x0, x1) span that has to be composited as 'VMask' loop.
L_BitScan_Init:
    x0 = BitOps::count_zeros_from_start(bit_word);
    bit_ptr[-1] = 0;
    bit_word_tmp = BitOps::shift_to_end(BitOps::ones(), x0);
    x0 = x0 * kPixelsPerOneBit + x_off;

    // Load the given cells to `m0` and clear the BitWord and all cells it represents in memory. This is important as
    // the compositor has to clear the memory during composition. If this is a rare case where `x0` points at the end
    // of the raster there is still one cell that is non-zero. This makes sure it's cleared.
    dst_ptr += x0 * kDstBPP;
    cell_ptr += x0;
    comp_op.spanStartX(uint32_t(x0));

    // Rare case - line rasterized at the end of the raster boundary. In 99% cases this is a clipped line that was
    // rasterized as vertical-only line at the end of the render box. This is a completely valid case that produces
    // nothing.
    if (x0 >= x_end)
      goto L_Scanline_Done0;

    // Setup compositor and source/destination parts.
    cov = 256 << (A8Info::kShift + 1);
    msk = 0;

    // If `bit_word ^ bit_word_tmp` results in non-zero value it means that the current span ends within the same BitWord,
    // otherwise the span crosses multiple BitWords.
    bit_word ^= bit_word_tmp;
    if (bit_word)
      goto L_BitScan_Match;

    // Okay, so the span crosses multiple BitWords. Firstly we have to make sure this was not the last one. If that's
    // the case we must terminate the scanning immediately.
    i = BitOps::kNumBits;
    if (bit_ptr == bit_ptr_end)
      goto L_BitScan_End;

    // A BitScan loop - iterates over all consecutive BitWords and finds those that don't have all bits set to 1.
L_BitScan_Next:
    for (;;) {
      bit_word = BitOps::ones() ^ bit_ptr[0];
      *bit_ptr++ = 0;
      x_off += kPixelsPerBitWord;

      if (bit_word)
        goto L_BitScan_Match;

      if (bit_ptr == bit_ptr_end)
        goto L_BitScan_End;
    }

L_BitScan_Match:
    i = BitOps::count_zeros_from_start(bit_word);

L_BitScan_End:
    bit_word_tmp = BitOps::shift_to_end(BitOps::ones(), i);
    i *= kPixelsPerOneBit;
    bit_word ^= bit_word_tmp;
    i += x_off;

    // In cases where the raster width is not a multiply of `pixels_per_one_bit` we must make sure we won't overflow it.
    if (i > x_end)
      i = x_end;

    // `i` is now the number of pixels (and cells) to composite by using `v_mask`.
    i -= x0;
    x0 += i;

    // VLoop
    // -----

    goto VLoop_CalcMsk;
    for (;;) {
      i--;
      cell_ptr++;
      dst_ptr = comp_op.composite_pixel_masked(dst_ptr, msk);

VLoop_CalcMsk:
      cov += cell_ptr[0];
      *cell_ptr = 0;

      msk = calc_mask(cov, fill_rule_mask, global_alpha);
      if (!i)
        break;
    }

    if (x0 >= x_end)
      goto L_Scanline_Done1;

    // BitGap
    // ------

    // If we are here we are at the end of `v_mask` loop. There are two possibilities:
    //
    //   1. There is a gap between bits in a single or multiple BitWords. This means that there is a possibility for
    //      a `c_mask` loop which could be solid, masked, or have zero-mask (a real gap).
    //
    //   2. This was the last span and there are no more bits in consecutive BitWords. We will not consider this as
    //      a special case and just process the remaining BitWords in a normal way (scanning until the end of the
    //      current scanline).
    while (!bit_word) {
      x_off += kPixelsPerBitWord;
      if (bit_ptr == bit_ptr_end)
        goto L_Scanline_Done1;
      bit_word = *bit_ptr++;
    }

    i = BitOps::count_zeros_from_start(bit_word);
    bit_word ^= BitOps::shift_to_end(BitOps::ones(), i);
    bit_ptr[-1] = 0;

    i = i * kPixelsPerOneBit + x_off - x0;
    x0 += i;
    cell_ptr += i;

    BL_ASSERT(x0 <= x_end);

    if (!msk) {
      dst_ptr += i * kDstBPP;
      comp_op.spanAdvanceX(uint32_t(x0), uint32_t(i));
    }
    else {
      dst_ptr = comp_op.compositeCSpan(dst_ptr, i, msk);
    }

    if (bit_word)
      goto L_BitScan_Match;
    else
      goto L_BitScan_Next;

    // Scanline Iterator
    // -----------------

    // This loop is used to quickly test bit_words in `bit_ptr`. In some cases the whole scanline could be empty, so
    // this loop makes sure we won't enter more complicated loops if this happens. It's also used to quickly find
    // the first bit, which is non-zero - in that case it jumps directly to BitMatch section.
L_Scanline_Done0:
    cell_ptr[0] = 0;

L_Scanline_Done1:
    dst_ptr -= x0 * kDstBPP;
    cell_ptr -= x0;
    comp_op.spanEndX(uint32_t(x0));

    if (--y == 0)
      return;

    bit_ptr = bit_ptr_end;
    do {
      dst_ptr += dst_stride;
      cell_ptr = PtrOps::offset(cell_ptr, cell_stride);
      comp_op.advance_y();

L_Scanline_Init:
      x_off = 0;
      bit_word = 0;
      bit_ptr_end = PtrOps::offset(bit_ptr, bit_stride);

      do {
        bit_word |= *bit_ptr++;
        if (bit_word)
          goto L_BitScan_Init;

        x_off += kPixelsPerBitWord;
      } while (bit_ptr != bit_ptr_end);
    } while (--y);
  }

  static BL_INLINE uint32_t calc_mask(uint32_t cov, uint32_t fill_rule_mask, uint32_t global_alpha) noexcept {
    uint32_t c = A8Info::kScale;
    uint32_t m = (IntOps::sar(cov, A8Info::kShift + 1u) & fill_rule_mask) - c;
    m = bl_min<uint32_t>(uint32_t(bl_abs(int32_t(m))), c);

    return (m * global_alpha) >> 8;
  }
};

template<FillType kFillType, typename CompOp>
struct FillDispatch {};

template<typename CompOp>
struct FillDispatch<FillType::kBoxA, CompOp> { using Fill = FillBoxA_Base<CompOp>; };

template<typename CompOp>
struct FillDispatch<FillType::kMask, CompOp> { using Fill = FillMask_Base<CompOp>; };

template<typename CompOp>
struct FillDispatch<FillType::kAnalytic, CompOp> { using Fill = FillAnalytic_Base<CompOp>; };

} // {anonymous}
} // {bl::Pipeline::Reference}

//! \}
//! \endcond

#endif // BLEND2D_PIPELINE_REFERENCE_FILLGENERIC_P_H_INCLUDED
