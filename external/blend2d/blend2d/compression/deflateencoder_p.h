// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_COMPRESSION_DEFLATEENCODER_P_H_INCLUDED
#define BLEND2D_COMPRESSION_DEFLATEENCODER_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/core/array.h>
#include <blend2d/compression/deflatedefs_p.h>

//! \cond INTERNAL

namespace bl::Compression::Deflate {

struct EncoderImpl;

static constexpr uint32_t kMaxCompressionLevel = 12;

class Encoder {
public:
  EncoderImpl* impl;

  BL_INLINE Encoder() noexcept : impl(nullptr) {}
  BL_INLINE ~Encoder() noexcept { reset(); }

  BL_INLINE bool is_initialized() const noexcept { return impl != nullptr; }

  BLResult init(FormatType format, uint32_t compression_level) noexcept;
  void reset() noexcept;

  size_t minimum_output_buffer_size(size_t input_size) const noexcept;
  size_t compress_to(uint8_t* output, size_t output_size, const uint8_t* input, size_t input_size) noexcept;
  BLResult compress(BLArray<uint8_t>& dst, BLModifyOp modify_op, BLDataView input) noexcept;
};

} // {bl::Compression::Deflate}

//! \endcond

#endif // BLEND2D_COMPRESSION_DEFLATEENCODER_P_H_INCLUDED
