// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_CODEC_JPEGOPS_P_H_INCLUDED
#define BLEND2D_CODEC_JPEGOPS_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_codec_impl
//! \{

namespace bl::Jpeg {

// Derived from jidctint's `jpeg_idct_islow`.
#define BL_JPEG_IDCT_PREC 12
#define BL_JPEG_IDCT_HALF(precision) (1 << ((precision) - 1))

#define BL_JPEG_IDCT_SCALE(x) ((x) << BL_JPEG_IDCT_PREC)
#define BL_JPEG_IDCT_FIXED(x) int(double(x) * double(1 << BL_JPEG_IDCT_PREC) + 0.5)

#define BL_JPEG_IDCT_M_2_562915447 (-BL_JPEG_IDCT_FIXED(2.562915447))
#define BL_JPEG_IDCT_M_1_961570560 (-BL_JPEG_IDCT_FIXED(1.961570560))
#define BL_JPEG_IDCT_M_1_847759065 (-BL_JPEG_IDCT_FIXED(1.847759065))
#define BL_JPEG_IDCT_M_0_899976223 (-BL_JPEG_IDCT_FIXED(0.899976223))
#define BL_JPEG_IDCT_M_0_390180644 (-BL_JPEG_IDCT_FIXED(0.390180644))
#define BL_JPEG_IDCT_P_0_298631336 ( BL_JPEG_IDCT_FIXED(0.298631336))
#define BL_JPEG_IDCT_P_0_541196100 ( BL_JPEG_IDCT_FIXED(0.541196100))
#define BL_JPEG_IDCT_P_0_765366865 ( BL_JPEG_IDCT_FIXED(0.765366865))
#define BL_JPEG_IDCT_P_1_175875602 ( BL_JPEG_IDCT_FIXED(1.175875602))
#define BL_JPEG_IDCT_P_1_501321110 ( BL_JPEG_IDCT_FIXED(1.501321110))
#define BL_JPEG_IDCT_P_2_053119869 ( BL_JPEG_IDCT_FIXED(2.053119869))
#define BL_JPEG_IDCT_P_3_072711026 ( BL_JPEG_IDCT_FIXED(3.072711026))

// Keep 2 bits of extra precision for the intermediate results.
#define BL_JPEG_IDCT_COL_NORM (BL_JPEG_IDCT_PREC - 2)
#define BL_JPEG_IDCT_COL_BIAS BL_JPEG_IDCT_HALF(BL_JPEG_IDCT_COL_NORM)

// Consume 2 bits of an intermediate results precision and 3 bits that were
// produced by `2 * sqrt(8)`. Also normalize to from `-128..127` to `0..255`.
#define BL_JPEG_IDCT_ROW_NORM (BL_JPEG_IDCT_PREC + 2 + 3)
#define BL_JPEG_IDCT_ROW_BIAS (BL_JPEG_IDCT_HALF(BL_JPEG_IDCT_ROW_NORM) + (128 << BL_JPEG_IDCT_ROW_NORM))

#define BL_JPEG_YCBCR_PREC 12
#define BL_JPEG_YCBCR_SCALE(x) ((x) << BL_JPEG_YCBCR_PREC)
#define BL_JPEG_YCBCR_FIXED(x) int(double(x) * double(1 << BL_JPEG_YCBCR_PREC) + 0.5)

// bl::Jpeg::Opts - Dispatch
// =========================

//! Optimized JPEG functions.
struct FuncOpts {
  //! Dequantize and perform IDCT and store clamped 8-bit results to `dst`.
  void (BL_CDECL* idct8)(uint8_t* dst, intptr_t dst_stride, const int16_t* src, const uint16_t* q_table) noexcept;

  //! No upsampling (stub).
  uint8_t* (BL_CDECL* upsample_1x1)(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t w, uint32_t hs) noexcept;
  //! Upsample row in vertical direction.
  uint8_t* (BL_CDECL* upsample_1x2)(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t w, uint32_t hs) noexcept;
  //! Upsample row in horizontal direction.
  uint8_t* (BL_CDECL* upsample_2x1)(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t w, uint32_t hs) noexcept;
  //! Upsample row in vertical and horizontal direction.
  uint8_t* (BL_CDECL* upsample_2x2)(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t w, uint32_t hs) noexcept;
  //! Upsample row any.
  uint8_t* (BL_CDECL* upsample_any)(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t w, uint32_t hs) noexcept;

  //! Perform planar YCbCr to RGB conversion and pack to XRGB32.
  void (BL_CDECL* conv_ycbcr8_to_rgb32)(uint8_t* dst, const uint8_t* pY, const uint8_t* pCb, const uint8_t* pCr, uint32_t count) noexcept;
};
extern FuncOpts opts;

// bl::Jpeg::Opts - Baseline
// =========================

BL_HIDDEN void BL_CDECL idct8(uint8_t* dst, intptr_t dst_stride, const int16_t* src, const uint16_t* q_table) noexcept;
BL_HIDDEN void BL_CDECL rgb32_from_ycbcr8(uint8_t* dst, const uint8_t* pY, const uint8_t* pCb, const uint8_t* pCr, uint32_t count) noexcept;

BL_HIDDEN uint8_t* BL_CDECL upsample_1x1(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t w, uint32_t hs) noexcept;
BL_HIDDEN uint8_t* BL_CDECL upsample_1x2(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t w, uint32_t hs) noexcept;
BL_HIDDEN uint8_t* BL_CDECL upsample_2x1(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t w, uint32_t hs) noexcept;
BL_HIDDEN uint8_t* BL_CDECL upsample_2x2(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t w, uint32_t hs) noexcept;
BL_HIDDEN uint8_t* BL_CDECL upsample_generic(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t w, uint32_t hs) noexcept;

// bl::Jpeg::Opts - SSE2
// ========================

#ifdef BL_BUILD_OPT_SSE2
BL_HIDDEN void BL_CDECL idct8_sse2(uint8_t* dst, intptr_t dst_stride, const int16_t* src, const uint16_t* q_table) noexcept;
BL_HIDDEN void BL_CDECL rgb32_from_ycbcr8_sse2(uint8_t* dst, const uint8_t* pY, const uint8_t* pCb, const uint8_t* pCr, uint32_t count) noexcept;
#endif

} // {bl::Jpeg}

//! \}
//! \endcond

#endif // BLEND2D_CODEC_JPEGOPS_P_H_INCLUDED
