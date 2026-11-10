// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

// The JPEG codec is based on stb_image <https://github.com/nothings/stb>
// released into PUBLIC DOMAIN. Blend2D's JPEG codec can be distributed
// under Blend2D's ZLIB license or under STB's PUBLIC DOMAIN as well.

#include <blend2d/core/api-build_p.h>
#if defined(BL_TARGET_OPT_SSE2)

#include <blend2d/core/rgba_p.h>
#include <blend2d/codec/jpegops_p.h>
#include <blend2d/simd/simd_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/memops_p.h>

namespace bl::Jpeg {

// bl::Jpeg::Opts - IDCT - SSE2
// ============================

struct alignas(16) OptConstSSE2 {
  // IDCT.
  int16_t idct_rot0a[8], idct_rot0b[8];
  int16_t idct_rot1a[8], idct_rot1b[8];
  int16_t idct_rot2a[8], idct_rot2b[8];
  int16_t idct_rot3a[8], idct_rot3b[8];

  int32_t idct_col_bias[4];
  int32_t idct_row_bias[4];

  // YCbCr.
  int32_t ycbcr_allones[4];
  int16_t ycbcr_tosigned[8];
  int32_t ycbcr_round[4];
  int16_t ycbcr_yycr_mul[8];
  int16_t ycbcr_yycb_mul[8];
  int16_t ycbcr_cbcr_mul[8];
};

#define DATA_4X(...) { __VA_ARGS__, __VA_ARGS__, __VA_ARGS__, __VA_ARGS__ }
static const OptConstSSE2 optConstSSE2 = {
  // IDCT.
  DATA_4X(BL_JPEG_IDCT_P_0_541196100                              ,
          BL_JPEG_IDCT_P_0_541196100 + BL_JPEG_IDCT_M_1_847759065),
  DATA_4X(BL_JPEG_IDCT_P_0_541196100 + BL_JPEG_IDCT_P_0_765366865 ,
          BL_JPEG_IDCT_P_0_541196100                             ),
  DATA_4X(BL_JPEG_IDCT_P_1_175875602 + BL_JPEG_IDCT_M_0_899976223 ,
          BL_JPEG_IDCT_P_1_175875602                             ),
  DATA_4X(BL_JPEG_IDCT_P_1_175875602                              ,
          BL_JPEG_IDCT_P_1_175875602 + BL_JPEG_IDCT_M_2_562915447),
  DATA_4X(BL_JPEG_IDCT_M_1_961570560 + BL_JPEG_IDCT_P_0_298631336 ,
          BL_JPEG_IDCT_M_1_961570560                             ),
  DATA_4X(BL_JPEG_IDCT_M_1_961570560                              ,
          BL_JPEG_IDCT_M_1_961570560 + BL_JPEG_IDCT_P_3_072711026),
  DATA_4X(BL_JPEG_IDCT_M_0_390180644 + BL_JPEG_IDCT_P_2_053119869 ,
          BL_JPEG_IDCT_M_0_390180644                             ),
  DATA_4X(BL_JPEG_IDCT_M_0_390180644                              ,
          BL_JPEG_IDCT_M_0_390180644 + BL_JPEG_IDCT_P_1_501321110),

  DATA_4X(BL_JPEG_IDCT_COL_BIAS),
  DATA_4X(BL_JPEG_IDCT_ROW_BIAS),

  // YCbCr.
  DATA_4X(-1),
  DATA_4X(-128, -128),
  DATA_4X(1 << (BL_JPEG_YCBCR_PREC - 1)),
  DATA_4X( BL_JPEG_YCBCR_FIXED(1.00000),  BL_JPEG_YCBCR_FIXED(1.40200)),
  DATA_4X( BL_JPEG_YCBCR_FIXED(1.00000),  BL_JPEG_YCBCR_FIXED(1.77200)),
  DATA_4X(-BL_JPEG_YCBCR_FIXED(0.34414), -BL_JPEG_YCBCR_FIXED(0.71414))
};
#undef DATA_4X

#define BL_JPEG_IDCT_INTERLEAVE8_XMM(a, b) { auto t = a; a = interleave_lo_u8(a, b); b = interleave_hi_u8(t, b); }
#define BL_JPEG_IDCT_INTERLEAVE16_XMM(a, b) { auto t = a; a = interleave_lo_u16(a, b); b = interleave_hi_u16(t, b); }

// out(0) = c0[even]*x + c0[odd]*y (in 16-bit, out 32-bit).
// out(1) = c1[even]*x + c1[odd]*y (in 16-bit, out 32-bit).
#define BL_JPEG_IDCT_ROTATE_XMM(dst0, dst1, x, y, c0, c1)               \
  VecPair<Vec4xI32> dst0;                                               \
  VecPair<Vec4xI32> dst1;                                               \
                                                                        \
  {                                                                     \
    VecPair<Vec4xI32> tmp;                                              \
                                                                        \
    tmp[0] = vec_i32(interleave_lo_u16(x, y));                          \
    tmp[1] = vec_i32(interleave_hi_u16(x, y));                          \
    dst0[0] = maddw_i16_i32(tmp[0], vec_const<Vec4xI32>(constants.c0)); \
    dst0[1] = maddw_i16_i32(tmp[1], vec_const<Vec4xI32>(constants.c0)); \
    dst1[0] = maddw_i16_i32(tmp[0], vec_const<Vec4xI32>(constants.c1)); \
    dst1[1] = maddw_i16_i32(tmp[1], vec_const<Vec4xI32>(constants.c1)); \
  }

// out = in << 12 (in 16-bit, out 32-bit)
#define BL_JPEG_IDCT_WIDEN_XMM(dst, in)                                        \
  VecPair<Vec4xI32> dst;                                                       \
  dst[0] = srai_i32<4>(vec_i32(interleave_lo_u16(make_zero<Vec8xI16>(), in))); \
  dst[1] = srai_i32<4>(vec_i32(interleave_hi_u16(make_zero<Vec8xI16>(), in)));

// Wide add (32-bit).
#define BL_JPEG_IDCT_WADD_XMM(dst, a, b) \
  VecPair<Vec4xI32> dst{add_i32(a[0], b[0]), add_i32(a[1], b[1])};

// Wide sub (32-bit).
#define BL_JPEG_IDCT_WSUB_XMM(dst, a, b) \
  VecPair<Vec4xI32> dst{sub_i32(a[0], b[0]), sub_i32(a[1], b[1])};

// Butterfly a/b, add bias, then shift by `norm` and pack to 16-bit.
#define BL_JPEG_IDCT_BFLY_XMM(dst0, dst1, a, b, bias, norm)                              \
  {                                                                                      \
    VecPair<Vec4xI32> a_biased{add_i32(a[0], bias), add_i32(a[1], bias)};                \
    BL_JPEG_IDCT_WADD_XMM(sum, a_biased, b)                                              \
    BL_JPEG_IDCT_WSUB_XMM(diff, a_biased, b)                                             \
                                                                                         \
    dst0 = vec_i16(packs_128_i32_i16(srai_i32<norm>(sum[0]), srai_i32<norm>(sum[1])));   \
    dst1 = vec_i16(packs_128_i32_i16(srai_i32<norm>(diff[0]), srai_i32<norm>(diff[1]))); \
  }

#define BL_JPEG_IDCT_IDCT_PASS_XMM(bias, norm) {                         \
  /* Even part. */                                                       \
  BL_JPEG_IDCT_ROTATE_XMM(t2e, t3e, row2, row6, idct_rot0a, idct_rot0b)  \
                                                                         \
  Vec8xI16 sum04 = add_i16(row0, row4);                                  \
  Vec8xI16 dif04 = sub_i16(row0, row4);                                  \
                                                                         \
  BL_JPEG_IDCT_WIDEN_XMM(t0e, sum04)                                     \
  BL_JPEG_IDCT_WIDEN_XMM(t1e, dif04)                                     \
                                                                         \
  BL_JPEG_IDCT_WADD_XMM(x0, t0e, t3e)                                    \
  BL_JPEG_IDCT_WSUB_XMM(x3, t0e, t3e)                                    \
  BL_JPEG_IDCT_WADD_XMM(x1, t1e, t2e)                                    \
  BL_JPEG_IDCT_WSUB_XMM(x2, t1e, t2e)                                    \
                                                                         \
  /* Odd part */                                                         \
  BL_JPEG_IDCT_ROTATE_XMM(y0o, y2o, row7, row3, idct_rot2a, idct_rot2b)  \
  BL_JPEG_IDCT_ROTATE_XMM(y1o, y3o, row5, row1, idct_rot3a, idct_rot3b)  \
  Vec8xI16 sum17 = add_i16(row1, row7);                                  \
  Vec8xI16 sum35 = add_i16(row3, row5);                                  \
  BL_JPEG_IDCT_ROTATE_XMM(y4o,y5o, sum17, sum35, idct_rot1a, idct_rot1b) \
                                                                         \
  BL_JPEG_IDCT_WADD_XMM(x4, y0o, y4o)                                    \
  BL_JPEG_IDCT_WADD_XMM(x5, y1o, y5o)                                    \
  BL_JPEG_IDCT_WADD_XMM(x6, y2o, y5o)                                    \
  BL_JPEG_IDCT_WADD_XMM(x7, y3o, y4o)                                    \
                                                                         \
  BL_JPEG_IDCT_BFLY_XMM(row0, row7, x0, x7, bias, norm)                  \
  BL_JPEG_IDCT_BFLY_XMM(row1, row6, x1, x6, bias, norm)                  \
  BL_JPEG_IDCT_BFLY_XMM(row2, row5, x2, x5, bias, norm)                  \
  BL_JPEG_IDCT_BFLY_XMM(row3, row4, x3, x4, bias, norm)                  \
}

void BL_CDECL idct8_sse2(uint8_t* dst, intptr_t dst_stride, const int16_t* src, const uint16_t* q_table) noexcept {
  using namespace SIMD;

  const OptConstSSE2& constants = optConstSSE2;

  // Load and dequantize (`src` is aligned to 16 bytes, `q_table` doesn't have to be).
  Vec8xI16 row0 = loadu<Vec8xI16>(q_table +  0) * loada<Vec8xI16>(src +  0);
  Vec8xI16 row1 = loadu<Vec8xI16>(q_table +  8) * loada<Vec8xI16>(src +  8);
  Vec8xI16 row2 = loadu<Vec8xI16>(q_table + 16) * loada<Vec8xI16>(src + 16);
  Vec8xI16 row3 = loadu<Vec8xI16>(q_table + 24) * loada<Vec8xI16>(src + 24);
  Vec8xI16 row4 = loadu<Vec8xI16>(q_table + 32) * loada<Vec8xI16>(src + 32);
  Vec8xI16 row5 = loadu<Vec8xI16>(q_table + 40) * loada<Vec8xI16>(src + 40);
  Vec8xI16 row6 = loadu<Vec8xI16>(q_table + 48) * loada<Vec8xI16>(src + 48);
  Vec8xI16 row7 = loadu<Vec8xI16>(q_table + 56) * loada<Vec8xI16>(src + 56);

  // IDCT columns.
  BL_JPEG_IDCT_IDCT_PASS_XMM(vec_const<Vec4xI32>(constants.idct_col_bias), BL_JPEG_IDCT_COL_NORM)

  // Transpose.
  BL_JPEG_IDCT_INTERLEAVE16_XMM(row0, row4) // [a0a4|b0b4|c0c4|d0d4] | [e0e4|f0f4|g0g4|h0h4]
  BL_JPEG_IDCT_INTERLEAVE16_XMM(row2, row6) // [a2a6|b2b6|c2c6|d2d6] | [e2e6|f2f6|g2g6|h2h6]
  BL_JPEG_IDCT_INTERLEAVE16_XMM(row1, row5) // [a1a5|b1b5|c2c5|d1d5] | [e1e5|f1f5|g1g5|h1h5]
  BL_JPEG_IDCT_INTERLEAVE16_XMM(row3, row7) // [a3a7|b3b7|c3c7|d3d7] | [e3e7|f3f7|g3g7|h3h7]

  BL_JPEG_IDCT_INTERLEAVE16_XMM(row0, row2) // [a0a2|a4a6|b0b2|b4b6] | [c0c2|c4c6|d0d2|d4d6]
  BL_JPEG_IDCT_INTERLEAVE16_XMM(row1, row3) // [a1a3|a5a7|b1b3|b5b7] | [c1c3|c5c7|d1d3|d5d7]
  BL_JPEG_IDCT_INTERLEAVE16_XMM(row4, row6) // [e0e2|e4e6|f0f2|f4f6] | [g0g2|g4g6|h0h2|h4h6]
  BL_JPEG_IDCT_INTERLEAVE16_XMM(row5, row7) // [e1e3|e5e7|f1f3|f5f7] | [g1g3|g5g7|h1h3|h5h7]

  BL_JPEG_IDCT_INTERLEAVE16_XMM(row0, row1) // [a0a1|a2a3|a4a5|a6a7] | [b0b1|b2b3|b4b5|b6b7]
  BL_JPEG_IDCT_INTERLEAVE16_XMM(row2, row3) // [c0c1|c2c3|c4c5|c6c7] | [d0d1|d2d3|d4d5|d6d7]
  BL_JPEG_IDCT_INTERLEAVE16_XMM(row4, row5) // [e0e1|e2e3|e4e5|e6e7] | [f0f1|f2f3|f4f5|f6f7]
  BL_JPEG_IDCT_INTERLEAVE16_XMM(row6, row7) // [g0g1|g2g3|g4g5|g6g7] | [h0h1|h2h3|h4h5|h6h7]

  // IDCT rows.
  BL_JPEG_IDCT_IDCT_PASS_XMM(vec_const<Vec4xI32>(constants.idct_row_bias), BL_JPEG_IDCT_ROW_NORM)

  // Pack to 8-bit unsigned integers with saturation.
  row0 = packs_128_i16_u8(row0, row1);      // [a0a1a2a3|a4a5a6a7|b0b1b2b3|b4b5b6b7]
  row2 = packs_128_i16_u8(row2, row3);      // [c0c1c2c3|c4c5c6c7|d0d1d2d3|d4d5d6d7]
  row4 = packs_128_i16_u8(row4, row5);      // [e0e1e2e3|e4e5e6e7|f0f1f2f3|f4f5f6f7]
  row6 = packs_128_i16_u8(row6, row7);      // [g0g1g2g3|g4g5g6g7|h0h1h2h3|h4h5h6h7]

  // Transpose.
  BL_JPEG_IDCT_INTERLEAVE8_XMM(row0, row4)  // [a0e0a1e1|a2e2a3e3|a4e4a5e5|a6e6a7e7] | [b0f0b1f1|b2f2b3f3|b4f4b5f5|b6f6b7f7]
  BL_JPEG_IDCT_INTERLEAVE8_XMM(row2, row6)  // [c0g0c1g1|c2g2c3g3|c4g4c5g5|c6g6c7g7] | [d0h0d1h1|d2h2d3h3|d4h4d5h5|d6h6d7h7]
  BL_JPEG_IDCT_INTERLEAVE8_XMM(row0, row2)  // [a0c0e0g0|a1c1e1g1|a2c2e2g2|a3c3e3g3] | [a4c4e4g4|a5c5e5g5|a6c6e6g6|a7c7e7g7]
  BL_JPEG_IDCT_INTERLEAVE8_XMM(row4, row6)  // [b0d0f0h0|b1d1f1h1|b2d2f2h2|b3d3f3h3| | [b4d4f4h4|b5d5f5h5|b6d6f6h6|b7d7f7h7]
  BL_JPEG_IDCT_INTERLEAVE8_XMM(row0, row4)  // [a0b0c0d0|e0f0g0h0|a1b1c1d1|e1f1g1h1] | [a2b2c2d2|e2f2g2h2|a3b3c3d3|e3f3g3h3]
  BL_JPEG_IDCT_INTERLEAVE8_XMM(row2, row6)  // [a4b4c4d4|e4f4g4h4|a5b5c5d5|e5f5g5h5] | [a6b6c6d6|e6f6g6h6|a7b7c7d7|e7f7g7h7]

  // Store.
  uint8_t* dst0 = dst;
  uint8_t* dst1 = dst + dst_stride;
  intptr_t dstStride2 = dst_stride * 2;

  storeu_64(dst0, row0); dst0 += dstStride2;
  storeh_64(dst1, row0); dst1 += dstStride2;

  storeu_64(dst0, row4); dst0 += dstStride2;
  storeh_64(dst1, row4); dst1 += dstStride2;

  storeu_64(dst0, row2); dst0 += dstStride2;
  storeh_64(dst1, row2); dst1 += dstStride2;

  storeu_64(dst0, row6);
  storeh_64(dst1, row6);
}

// bl::Jpeg::Opts - RGB32 From YCbCr8 - SSE2
// =========================================

void BL_CDECL rgb32_from_ycbcr8_sse2(uint8_t* dst, const uint8_t* pY, const uint8_t* pCb, const uint8_t* pCr, uint32_t count) noexcept {
  using namespace SIMD;
  uint32_t i = count;

  const OptConstSSE2& constants = optConstSSE2;

  while (i >= 8) {
    Vec8xI16 yy = unpack_lo64_u8_u16(loadu_64<Vec8xI16>(pY));
    Vec8xI16 cb = unpack_lo64_u8_u16(loadu_64<Vec8xI16>(pCb));
    Vec8xI16 cr = unpack_lo64_u8_u16(loadu_64<Vec8xI16>(pCr));

    cb = add_i16(cb, vec_const<Vec8xI16>(constants.ycbcr_tosigned));
    cr = add_i16(cr, vec_const<Vec8xI16>(constants.ycbcr_tosigned));

    Vec4xI32 r_l = vec_i32(maddw_i16_i32(interleave_lo_u16(yy, cr), vec_const<Vec8xI16>(constants.ycbcr_yycr_mul)));
    Vec4xI32 r_h = vec_i32(maddw_i16_i32(interleave_hi_u16(yy, cr), vec_const<Vec8xI16>(constants.ycbcr_yycr_mul)));

    Vec4xI32 b_l = vec_i32(maddw_i16_i32(interleave_lo_u16(yy, cb), vec_const<Vec8xI16>(constants.ycbcr_yycb_mul)));
    Vec4xI32 b_h = vec_i32(maddw_i16_i32(interleave_hi_u16(yy, cb), vec_const<Vec8xI16>(constants.ycbcr_yycb_mul)));

    Vec4xI32 g_l = vec_i32(maddw_i16_i32(interleave_lo_u16(cb, cr), vec_const<Vec8xI16>(constants.ycbcr_cbcr_mul)));
    Vec4xI32 g_h = vec_i32(maddw_i16_i32(interleave_hi_u16(cb, cr), vec_const<Vec8xI16>(constants.ycbcr_cbcr_mul)));

    g_l = add_i32(g_l, slli_i32<BL_JPEG_YCBCR_PREC>(vec_i32(unpack_lo64_u16_u32(yy))));
    g_h = add_i32(g_h, slli_i32<BL_JPEG_YCBCR_PREC>(vec_i32(unpack_hi64_u16_u32(yy))));

    r_l = add_i32(r_l, vec_const<Vec4xI32>(constants.ycbcr_round));
    r_h = add_i32(r_h, vec_const<Vec4xI32>(constants.ycbcr_round));
    g_l = add_i32(g_l, vec_const<Vec4xI32>(constants.ycbcr_round));
    g_h = add_i32(g_h, vec_const<Vec4xI32>(constants.ycbcr_round));
    b_l = add_i32(b_l, vec_const<Vec4xI32>(constants.ycbcr_round));
    b_h = add_i32(b_h, vec_const<Vec4xI32>(constants.ycbcr_round));

    r_l = srai_i32<BL_JPEG_YCBCR_PREC>(r_l);
    r_h = srai_i32<BL_JPEG_YCBCR_PREC>(r_h);
    g_l = srai_i32<BL_JPEG_YCBCR_PREC>(g_l);
    g_h = srai_i32<BL_JPEG_YCBCR_PREC>(g_h);
    b_l = srai_i32<BL_JPEG_YCBCR_PREC>(b_l);
    b_h = srai_i32<BL_JPEG_YCBCR_PREC>(b_h);

    Vec16xU8 r = vec_u8(packz_128_u32_u8(r_l, r_h));
    Vec16xU8 g = vec_u8(packz_128_u32_u8(g_l, g_h));
    Vec16xU8 b = vec_u8(packz_128_u32_u8(b_l, b_h));

    Vec16xU8 ra = interleave_lo_u8(r, vec_const<Vec16xU8>(constants.ycbcr_allones));
    Vec16xU8 bg = interleave_lo_u8(b, g);

    Vec16xU8 bgra0 = interleave_lo_u16(bg, ra);
    Vec16xU8 bgra1 = interleave_hi_u16(bg, ra);

    storeu(dst +  0, bgra0);
    storeu(dst + 16, bgra1);

    dst += 32;
    pY  += 8;
    pCb += 8;
    pCr += 8;
    i   -= 8;
  }

  while (i) {
    int yy = (int(pY[0]) << BL_JPEG_YCBCR_PREC) + (1 << (BL_JPEG_YCBCR_PREC - 1));
    int cr = int(pCr[0]) - 128;
    int cb = int(pCb[0]) - 128;

    int r = yy + cr * BL_JPEG_YCBCR_FIXED(1.40200);
    int g = yy - cr * BL_JPEG_YCBCR_FIXED(0.71414) - cb * BL_JPEG_YCBCR_FIXED(0.34414);
    int b = yy + cb * BL_JPEG_YCBCR_FIXED(1.77200);

    uint32_t rgba32 = RgbaInternal::packRgba32(IntOps::clamp_to_byte(r >> BL_JPEG_YCBCR_PREC),
                                               IntOps::clamp_to_byte(g >> BL_JPEG_YCBCR_PREC),
                                               IntOps::clamp_to_byte(b >> BL_JPEG_YCBCR_PREC));
    MemOps::writeU32a(dst, rgba32);

    dst += 4;
    pY  += 1;
    pCb += 1;
    pCr += 1;
    i   -= 1;
  }
}

} // {bl::Jpeg}

#endif // BL_TARGET_OPT_SSE2
