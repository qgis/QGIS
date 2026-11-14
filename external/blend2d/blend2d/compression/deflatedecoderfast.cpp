// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#if BL_TARGET_ARCH_BITS >= 64

#include <blend2d/compression/deflatedecoderfastimpl_p.h>

namespace bl::Compression::Deflate::Fast {

DecoderFastResult decode(
  Decoder* ctx,
  uint8_t* dst_start,
  uint8_t* dst_ptr,
  uint8_t* dst_end,
  const uint8_t* src_ptr,
  const uint8_t* src_end
) noexcept {
  return decode_impl(ctx, dst_start, dst_ptr, dst_end, src_ptr, src_end);
}

} // {bl::Compression::Deflate::Fast}

#endif // BL_TARGET_ARCH_BITS >= 64
