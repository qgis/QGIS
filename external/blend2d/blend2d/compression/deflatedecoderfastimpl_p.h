// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_COMPRESSION_DEFLATEDECODERFASTIMPL_P_H_INCLUDED
#define BLEND2D_COMPRESSION_DEFLATEDECODERFASTIMPL_P_H_INCLUDED

#include <blend2d/core/runtime_p.h>
#include <blend2d/compression/deflatedecoder_p.h>
#include <blend2d/compression/deflatedecoderfast_p.h>
#include <blend2d/compression/deflatedecoderutils_p.h>
#include <blend2d/support/memops_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/ptrops_p.h>

//! \cond INTERNAL

namespace bl::Compression::Deflate::Fast {
namespace {

DecoderFastResult decode_impl(
  Decoder* ctx,
  uint8_t* dst_start,
  uint8_t* dst_ptr,
  uint8_t* dst_end,
  const uint8_t* src_ptr,
  const uint8_t* src_end
) noexcept {
  constexpr size_t kCopyRegSize = sizeof(CopyContext::Register);

  DecoderBits bits;
  bits.load_state(ctx);

  const DecodeTables& tables = ctx->tables;

  DecoderTableMask litlen_table_mask(ctx->_litlen_fast_table_bits);
  DecoderTableMask offset_table_mask(ctx->_offset_table_info.table_bits);

  for (;;) {
    // Destination and source pointer conditions:
    //  - at least one full refill and 8 additional bytes must be available to enter the fast loop.
    //  - at least one full match must be possible for decoding one entry (thus `kMaxMatchLen + kDstBytesPerIter`).
    intptr_t src_remaining_iters = intptr_t(PtrOps::bytes_until(src_ptr, src_end) >> kSrcBytesPerIterShift);
    intptr_t dst_remaining_iters = intptr_t(PtrOps::bytes_until(dst_ptr, dst_end) >> kDstBytesPerIterShift);

    // We can write up to kDstBytesPerIter bytes + one full match each iteration - if more bytes are written,
    // safe_iters is recalculated.
    intptr_t safe_iters = bl_min(src_remaining_iters - intptr_t(kSrcMinScratchShifted),
                               dst_remaining_iters - intptr_t(kDstMinScratchShifted));

    // NOTE: If safe_iters is low it will keep jumping to FastLoop_Restart too often, sometimes even after each
    // iteration, so we really want a reasonable number of iterations to execute before recalculating.
    if (safe_iters <= intptr_t(kMinimumFastIterationCount)) {
      break;
    }

    BL_DECODER_UPDATE_STATISTICS(ctx->statistics.fast.num_restarts++);
    src_ptr += bits.refill_bit_word(MemOps::loadu_le<BLBitWord>(src_ptr));

    // Decode one entry ahead here.
    DecodeEntry entry = tables.litlen_decode_table[bits.extract(litlen_table_mask)];

    while (safe_iters > 0) {
      // Make sure that the safe loop assumption is not breached - if any of the following assertion fails
      // it means that there is a bug, which must be fixed. The possible bug will be in the offset handler.
      BL_ASSERT(PtrOps::bytes_until(dst_ptr, dst_end) >= kDstBytesPerIter + kMaxMatchLen);
      BL_ASSERT(PtrOps::bytes_until(src_ptr, src_end) >= kSrcBytesPerIter);

      BLBitWord refill_data = MemOps::loadu_le<BLBitWord>(src_ptr);

      src_ptr += bits.refill_bit_word(refill_data);
      uint32_t payload = DecoderUtils::raw_payload(entry);

      if (DecoderUtils::is_literal(entry)) {
        bits.consumed(entry);
        size_t entry_index = bits.extract(litlen_table_mask);

        uint32_t lit_bits = entry.value & 0xFFu;
        uint32_t lit_count = lit_bits >> 6;

        entry = tables.litlen_decode_table[entry_index];

        if (BL_LIKELY(DecoderUtils::is_literal(entry))) {
          bits.consumed(entry);
          entry_index = bits.extract(litlen_table_mask);

          payload += (entry.value >> 8) << (lit_count * 8u);
          lit_count += (entry.value & 0xFFu) >> 6;
          entry = tables.litlen_decode_table[entry_index];

          MemOps::storeu_le(dst_ptr, payload);
          dst_ptr += lit_count;

          safe_iters--;
          continue;
        }
        else {
          MemOps::storeu_le(dst_ptr, uint16_t(payload));
          dst_ptr += lit_count;
          payload = DecoderUtils::raw_payload(entry);
        }
      }

      uint32_t length = payload + DecoderUtils::extract_extra(bits.bit_word, entry);

      if (BL_UNLIKELY(!DecoderUtils::is_off_or_len(entry))) {
        BLBitWord prev_bits = bits.bit_word;
        entry = tables.litlen_decode_table[length & 0x7FFFu];

        payload = DecoderUtils::raw_payload(entry);
        refill_data = MemOps::loadu_le<BLBitWord>(src_ptr);

        bits.consumed(entry);
        src_ptr += bits.refill_bit_word(refill_data);

        if (DecoderUtils::is_literal(entry)) {
          entry = tables.litlen_decode_table[bits.extract(litlen_table_mask)];
          *dst_ptr++ = uint8_t(payload & 0xFFu);

          safe_iters--;
          continue;
        }

        if (BL_UNLIKELY(DecoderUtils::is_end_of_block(entry))) {
          if (BL_LIKELY(!DecoderUtils::is_end_of_block_invalid(entry)))
            goto BlockDone;
          else
            goto ErrorInvalidData;
        }

        length = payload + DecoderUtils::extract_extra(prev_bits, entry);
      }
      else {
        bits.consumed(entry);
      }

      // There must be space for the whole copy - if not it's a bug in the fast loop!
      BL_ASSERT(PtrOps::bytes_until(dst_ptr, dst_end) >= length + kDstBytesPerIter);

      entry = tables.offset_decode_table[bits.extract(offset_table_mask)];
      uint32_t offset = DecoderUtils::raw_payload(entry) + DecoderUtils::extract_extra(bits.bit_word, entry);

      if (BL_UNLIKELY(!DecoderUtils::is_off_or_len(entry))) {
        entry = tables.offset_decode_table[offset];
        offset = DecoderUtils::raw_payload(entry) + DecoderUtils::extract_extra(bits.bit_word, entry);

        if (BL_UNLIKELY(DecoderUtils::is_end_of_block(entry))) {
          goto ErrorInvalidData;
        }
      }

      size_t dst_size = PtrOps::byte_offset(dst_start, dst_ptr);
      bits.consumed(entry);

      if (BL_UNLIKELY(offset > dst_size)) {
        goto ErrorInvalidData;
      }

      const uint8_t* match_ptr = dst_ptr - offset;
      CopyContext::Register r0 = CopyContext::load(match_ptr);

      uint8_t* copy_ptr = dst_ptr;
      dst_ptr += length;

      safe_iters -= intptr_t((length + (kDstBytesPerIter - 1)) / kDstBytesPerIter);

      BL_DECODER_UPDATE_STATISTICS(ctx->statistics.fast.match_up_to_8 += unsigned(length <= 8));
      BL_DECODER_UPDATE_STATISTICS(ctx->statistics.fast.match_up_to_16 += unsigned(length <= 16));
      BL_DECODER_UPDATE_STATISTICS(ctx->statistics.fast.match_up_to_32 += unsigned(length <= 32));
      BL_DECODER_UPDATE_STATISTICS(ctx->statistics.fast.match_up_to_64 += unsigned(length <= 64));

      BL_DECODER_UPDATE_STATISTICS(ctx->statistics.fast.match_more_than_8 += unsigned(length > 8));
      BL_DECODER_UPDATE_STATISTICS(ctx->statistics.fast.match_more_than_16 += unsigned(length > 16));
      BL_DECODER_UPDATE_STATISTICS(ctx->statistics.fast.match_more_than_32 += unsigned(length > 32));
      BL_DECODER_UPDATE_STATISTICS(ctx->statistics.fast.match_more_than_64 += unsigned(length > 64));

      if (BL_LIKELY(offset >= kCopyRegSize)) {
        BL_DECODER_UPDATE_STATISTICS(ctx->statistics.fast.match_near++);
        CopyContext::store(copy_ptr, r0);

        r0 = CopyContext::load_raw(match_ptr + kCopyRegSize);
        entry = tables.litlen_decode_table[bits.extract(litlen_table_mask)];
        match_ptr += kCopyRegSize * 2u;

        CopyContext::store_raw(copy_ptr + kCopyRegSize, r0);
        copy_ptr += kCopyRegSize * 2u;

        BL_NOUNROLL
        while (copy_ptr < dst_ptr) {
          r0 = CopyContext::load_raw(match_ptr);
          CopyContext::store_raw(copy_ptr, r0);

          r0 = CopyContext::load_raw(match_ptr + kCopyRegSize);
          match_ptr += kCopyRegSize * 2u;

          CopyContext::store_raw(copy_ptr + kCopyRegSize, r0);
          copy_ptr += kCopyRegSize * 2u;
        }
      }
      else {
        CopyContext match_ctx;

        match_ctx.init_repeat(offset);
        r0 = match_ctx.repeat(r0);

        match_ctx.init_rotate(offset);
        entry = tables.litlen_decode_table[bits.extract(litlen_table_mask)];

        CopyContext::store(copy_ptr, r0);
        copy_ptr += kCopyRegSize;

        BL_NOUNROLL
        while (copy_ptr < dst_ptr) {
          r0 = match_ctx.rotate(r0);
          CopyContext::store(copy_ptr, r0);
          copy_ptr += kCopyRegSize;
        }
      }
    }
  }

  bits.fix_length_after_fast_loop();
  bits.store_state(ctx);
  return DecoderFastResult{DecoderFastStatus::kOk, dst_ptr, src_ptr};

BlockDone:
  bits.fix_length_after_fast_loop();
  bits.store_state(ctx);
  return DecoderFastResult{DecoderFastStatus::kBlockDone, dst_ptr, src_ptr};

ErrorInvalidData:
  bits.fix_length_after_fast_loop();
  bits.store_state(ctx);
  return DecoderFastResult{DecoderFastStatus::kInvalidData, dst_ptr, src_ptr};
}

} // {anonymous}
} // {bl::Compression::Deflate::Fast}

//! \endcond

#endif // BLEND2D_COMPRESSION_DEFLATEDECODERFASTIMPL_P_H_INCLUDED
