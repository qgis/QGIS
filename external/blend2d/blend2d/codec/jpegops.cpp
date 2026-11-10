// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

// The JPEG codec is based on stb_image <https://github.com/nothings/stb>
// released into PUBLIC DOMAIN. Blend2D's JPEG codec can be distributed
// under Blend2D's ZLIB license or under STB's PUBLIC DOMAIN as well.

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/rgba_p.h>
#include <blend2d/codec/jpegcodec_p.h>
#include <blend2d/codec/jpegops_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/memops_p.h>

namespace bl::Jpeg {

// bl::Jpeg::Opts - Globals
// ========================

FuncOpts opts;

// bl::Jpeg::Opts - IDCT
// =====================

#define BL_JPEG_IDCT_IDCT(s0, s1, s2, s3, s4, s5, s6, s7) \
  int x0, x1, x2, x3;                             \
  int t0, t1, t2, t3;                             \
  int p1, p2, p3, p4, p5;                         \
                                                  \
  p2 = s2;                                        \
  p3 = s6;                                        \
  p1 = (p2 + p3) * BL_JPEG_IDCT_P_0_541196100;    \
  t2 = p3 * BL_JPEG_IDCT_M_1_847759065 + p1;      \
  t3 = p2 * BL_JPEG_IDCT_P_0_765366865 + p1;      \
                                                  \
  p2 = s0;                                        \
  p3 = s4;                                        \
  t0 = BL_JPEG_IDCT_SCALE(p2 + p3);               \
  t1 = BL_JPEG_IDCT_SCALE(p2 - p3);               \
                                                  \
  x0 = t0 + t3;                                   \
  x3 = t0 - t3;                                   \
  x1 = t1 + t2;                                   \
  x2 = t1 - t2;                                   \
                                                  \
  t0 = s7;                                        \
  t1 = s5;                                        \
  t2 = s3;                                        \
  t3 = s1;                                        \
                                                  \
  p3 = t0 + t2;                                   \
  p4 = t1 + t3;                                   \
  p1 = t0 + t3;                                   \
  p2 = t1 + t2;                                   \
  p5 = p3 + p4;                                   \
                                                  \
  p5 = p5 * BL_JPEG_IDCT_P_1_175875602;           \
  t0 = t0 * BL_JPEG_IDCT_P_0_298631336;           \
  t1 = t1 * BL_JPEG_IDCT_P_2_053119869;           \
  t2 = t2 * BL_JPEG_IDCT_P_3_072711026;           \
  t3 = t3 * BL_JPEG_IDCT_P_1_501321110;           \
                                                  \
  p1 = p1 * BL_JPEG_IDCT_M_0_899976223 + p5;      \
  p2 = p2 * BL_JPEG_IDCT_M_2_562915447 + p5;      \
  p3 = p3 * BL_JPEG_IDCT_M_1_961570560;           \
  p4 = p4 * BL_JPEG_IDCT_M_0_390180644;           \
                                                  \
  t3 += p1 + p4;                                  \
  t2 += p2 + p3;                                  \
  t1 += p2 + p4;                                  \
  t0 += p1 + p3;

void BL_CDECL idct8(uint8_t* dst, intptr_t dst_stride, const int16_t* src, const uint16_t* q_table) noexcept {
  uint32_t i;
  int32_t* tmp;
  int32_t tmp_data[64];

  for (i = 0, tmp = tmp_data; i < 8; i++, src++, tmp++, q_table++) {
    // Avoid dequantizing and IDCTing zeros.
    if (src[8] == 0 && src[16] == 0 && src[24] == 0 && src[32] == 0 && src[40] == 0 && src[48] == 0 && src[56] == 0) {
      int32_t dc_term = (int32_t(src[0]) * int32_t(q_table[0])) << (BL_JPEG_IDCT_PREC - BL_JPEG_IDCT_COL_NORM);
      tmp[0] = tmp[8] = tmp[16] = tmp[24] = tmp[32] = tmp[40] = tmp[48] = tmp[56] = dc_term;
    }
    else {
      BL_JPEG_IDCT_IDCT(
        int32_t(src[ 0]) * int32_t(q_table[ 0]),
        int32_t(src[ 8]) * int32_t(q_table[ 8]),
        int32_t(src[16]) * int32_t(q_table[16]),
        int32_t(src[24]) * int32_t(q_table[24]),
        int32_t(src[32]) * int32_t(q_table[32]),
        int32_t(src[40]) * int32_t(q_table[40]),
        int32_t(src[48]) * int32_t(q_table[48]),
        int32_t(src[56]) * int32_t(q_table[56]));

      x0 += BL_JPEG_IDCT_COL_BIAS;
      x1 += BL_JPEG_IDCT_COL_BIAS;
      x2 += BL_JPEG_IDCT_COL_BIAS;
      x3 += BL_JPEG_IDCT_COL_BIAS;

      tmp[ 0] = (x0 + t3) >> BL_JPEG_IDCT_COL_NORM;
      tmp[56] = (x0 - t3) >> BL_JPEG_IDCT_COL_NORM;
      tmp[ 8] = (x1 + t2) >> BL_JPEG_IDCT_COL_NORM;
      tmp[48] = (x1 - t2) >> BL_JPEG_IDCT_COL_NORM;
      tmp[16] = (x2 + t1) >> BL_JPEG_IDCT_COL_NORM;
      tmp[40] = (x2 - t1) >> BL_JPEG_IDCT_COL_NORM;
      tmp[24] = (x3 + t0) >> BL_JPEG_IDCT_COL_NORM;
      tmp[32] = (x3 - t0) >> BL_JPEG_IDCT_COL_NORM;
    }
  }

  for (i = 0, tmp = tmp_data; i < 8; i++, dst += dst_stride, tmp += 8) {
    BL_JPEG_IDCT_IDCT(tmp[0], tmp[1], tmp[2], tmp[3], tmp[4], tmp[5], tmp[6], tmp[7])

    x0 += BL_JPEG_IDCT_ROW_BIAS;
    x1 += BL_JPEG_IDCT_ROW_BIAS;
    x2 += BL_JPEG_IDCT_ROW_BIAS;
    x3 += BL_JPEG_IDCT_ROW_BIAS;

    dst[0] = IntOps::clamp_to_byte((x0 + t3) >> BL_JPEG_IDCT_ROW_NORM);
    dst[7] = IntOps::clamp_to_byte((x0 - t3) >> BL_JPEG_IDCT_ROW_NORM);
    dst[1] = IntOps::clamp_to_byte((x1 + t2) >> BL_JPEG_IDCT_ROW_NORM);
    dst[6] = IntOps::clamp_to_byte((x1 - t2) >> BL_JPEG_IDCT_ROW_NORM);
    dst[2] = IntOps::clamp_to_byte((x2 + t1) >> BL_JPEG_IDCT_ROW_NORM);
    dst[5] = IntOps::clamp_to_byte((x2 - t1) >> BL_JPEG_IDCT_ROW_NORM);
    dst[3] = IntOps::clamp_to_byte((x3 + t0) >> BL_JPEG_IDCT_ROW_NORM);
    dst[4] = IntOps::clamp_to_byte((x3 - t0) >> BL_JPEG_IDCT_ROW_NORM);
  }
}

// bl::Jpeg::Opts - RGB32 From YCbCr8
// ==================================

void BL_CDECL rgb32_from_ycbcr8(uint8_t* dst, const uint8_t* pY, const uint8_t* pCb, const uint8_t* pCr, uint32_t count) noexcept {
  for (uint32_t i = 0; i < count; i++) {
    int yy = (int(pY[i]) << BL_JPEG_YCBCR_PREC) + (1 << (BL_JPEG_YCBCR_PREC - 1));
    int cr = int(pCr[i]) - 128;
    int cb = int(pCb[i]) - 128;

    int r = yy + cr * BL_JPEG_YCBCR_FIXED(1.40200);
    int g = yy - cr * BL_JPEG_YCBCR_FIXED(0.71414) - cb * BL_JPEG_YCBCR_FIXED(0.34414);
    int b = yy + cb * BL_JPEG_YCBCR_FIXED(1.77200);

    uint32_t rgba32 = RgbaInternal::packRgba32(
      IntOps::clamp_to_byte(r >> BL_JPEG_YCBCR_PREC),
      IntOps::clamp_to_byte(g >> BL_JPEG_YCBCR_PREC),
      IntOps::clamp_to_byte(b >> BL_JPEG_YCBCR_PREC));
    MemOps::writeU32a(dst, rgba32);
    dst += 4;
  }
}

// bl::Jpeg::Opts - Upsample
// =========================

uint8_t* BL_CDECL upsample_1x1(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t w, uint32_t hs) noexcept {
  bl_unused(dst, src1, w, hs);

  return src0;
}

uint8_t* BL_CDECL upsample_1x2(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t w, uint32_t hs) noexcept {
  bl_unused(src1, hs);

  for (uint32_t i = 0; i < w; i++)
    dst[i] = uint8_t((3 * src0[i] + src1[i] + 2) >> 2);

  return dst;
}

uint8_t* BL_CDECL upsample_2x1(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t w, uint32_t hs) noexcept {
  bl_unused(hs, src1);

  // If only one sample, can't do any interpolation.
  if (w == 1) {
    dst[0] = dst[1] = src0[0];
    return dst;
  }

  dst[0] = src0[0];
  dst[1] = uint8_t((src0[0] * 3 + src0[1] + 2) >> 2);

  uint32_t i;
  for (i = 1; i < w - 1; i++) {
    uint32_t n = 3 * src0[i] + 2;
    dst[i * 2 + 0] = uint8_t((n + src0[i - 1]) >> 2);
    dst[i * 2 + 1] = uint8_t((n + src0[i + 1]) >> 2);
  }

  dst[i * 2 + 0] = uint8_t((src0[w - 2] * 3 + src0[w-1] + 2) >> 2);
  dst[i * 2 + 1] = src0[w - 1];

  return dst;
}

uint8_t* BL_CDECL upsample_2x2(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t w, uint32_t hs) noexcept {
  bl_unused(hs);

  if (w == 1) {
    dst[0] = dst[1] = uint8_t((3 * src0[0] + src1[0] + 2) >> 2);
    return dst;
  }

  uint32_t t0;
  uint32_t t1 = 3 * src0[0] + src1[0];

  dst[0] = uint8_t((t1 + 2) >> 2);
  for (uint32_t i = 1; i < w; i++) {
    t0 = t1;
    t1 = 3 * src0[i] + src1[i];

    dst[i * 2 - 1] = uint8_t((3 * t0 + t1 + 8) >> 4);
    dst[i * 2    ] = uint8_t((3 * t1 + t0 + 8) >> 4);
  }
  dst[w * 2 - 1] = uint8_t((t1 + 2) >> 2);

  return dst;
}

uint8_t* BL_CDECL upsample_generic(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t w, uint32_t hs) noexcept {
  bl_unused(src1);

  for (uint32_t i = 0; i < w; i++)
    for (uint32_t j = 0; j < hs; j++)
      dst[i * hs + j] = src0[i];

  return dst;
}

} // {bl::Jpeg}
