// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_test_p.h>
#if defined(BL_TEST)

#include <blend2d/core/api-impl.h>
#include <blend2d/core/format_p.h>
#include <blend2d/core/pixelconverter_p.h>
#include <blend2d/core/random.h>
#include <blend2d/pixelops/scalar_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/memops_p.h>

// PixelConverter - Tests
// ======================

namespace bl {
namespace Tests {

// XRGB32 <-> A8 Conversion Tests
// ------------------------------

static void testRgb32A8Conversions() noexcept {
  INFO("Testing ?RGB32 <-> A8 conversions");

  // Pixel formats.
  BLFormatInfo a8_format = bl_format_info[BL_FORMAT_A8];
  BLFormatInfo xrgb32_format = bl_format_info[BL_FORMAT_XRGB32];
  BLFormatInfo argb32_format = bl_format_info[BL_FORMAT_PRGB32];
  BLFormatInfo prgb32_format = bl_format_info[BL_FORMAT_PRGB32];

  argb32_format.clear_flags(BL_FORMAT_FLAG_PREMULTIPLIED);

  // Pixel buffers.
  uint8_t srcX8[256];
  uint8_t dstX8[256];
  uint32_t rgb32[256];

  uint32_t i;
  uint32_t n;

  // Prepare.
  for (i = 0; i < 256; i++)
    srcX8[i] = uint8_t(i);

  BLPixelConverter cvtXrgb32FromA8;
  BLPixelConverter cvtArgb32FromA8;
  BLPixelConverter cvtPrgb32FromA8;
  BLPixelConverter cvtA8FromPrgb32;

  EXPECT_SUCCESS(cvtXrgb32FromA8.create(xrgb32_format, a8_format));
  EXPECT_SUCCESS(cvtArgb32FromA8.create(argb32_format, a8_format));
  EXPECT_SUCCESS(cvtPrgb32FromA8.create(prgb32_format, a8_format));
  EXPECT_SUCCESS(cvtA8FromPrgb32.create(a8_format, prgb32_format));

  // This would test the conversion and also whether the SIMD implementation
  // is okay. We test for 1..256 pixels and verify all pixels from 0..255.
  for (n = 1; n < 256; n++) {
    memset(rgb32, 0, sizeof(rgb32));
    EXPECT_SUCCESS(cvtXrgb32FromA8.convert_span(rgb32, srcX8, n));

    for (i = 0; i < 256; i++) {
      if (i < n) {
        uint32_t p0 = rgb32[i];
        uint32_t p1 = (uint32_t(srcX8[i]) * 0x01010101u) | 0xFF000000u;
        EXPECT_EQ(p0, p1).message("[%u] XRGB32<-A8 conversion error OUT[%08X] != EXP[%08X]", i, p0, p1);
      }
      else {
        EXPECT_EQ(rgb32[i], 0u).message("[%u] Detected buffer overrun after XRGB32<-A8 conversion", i);
      }
    }

    memset(rgb32, 0, sizeof(rgb32));
    EXPECT_SUCCESS(cvtArgb32FromA8.convert_span(rgb32, srcX8, n));

    for (i = 0; i < 256; i++) {
      if (i < n) {
        uint32_t p0 = rgb32[i];
        uint32_t p1 = (uint32_t(srcX8[i]) * 0x01010101u) | 0x00FFFFFFu;
        EXPECT_EQ(p0, p1).message("[%u] ARGB32<-A8 conversion error OUT[%08X] != EXP[%08X]", i, p0, p1);
      }
      else {
        EXPECT_EQ(rgb32[i], 0u).message("[%u] Detected buffer overrun after ARGB32<-A8 conversion", i);
      }
    }

    memset(rgb32, 0, sizeof(rgb32));
    EXPECT_SUCCESS(cvtPrgb32FromA8.convert_span(rgb32, srcX8, n));

    for (i = 0; i < 256; i++) {
      if (i < n) {
        uint32_t p0 = rgb32[i];
        uint32_t p1 = uint32_t(srcX8[i]) * 0x01010101u;
        EXPECT_EQ(p0, p1).message("[%u] PRGB32<-A8 conversion error OUT[%08X] != EXP[%08X]", i, p0, p1);
      }
      else {
        EXPECT_EQ(rgb32[i], 0u).message("[%u] Detected buffer overrun after PRGB32<-A8 conversion", i);
      }
    }
  }

  for (n = 1; n < 256; n++) {
    memset(dstX8, 0, sizeof(dstX8));
    EXPECT_SUCCESS(cvtA8FromPrgb32.convert_span(dstX8, rgb32, n));

    for (i = 0; i < 256; i++) {
      if (i < n) {
        uint32_t p0 = srcX8[i];
        uint32_t p1 = dstX8[i];
        EXPECT_EQ(p0, p1).message("[%u] A8<-PRGB32 conversion error OUT[%02X] != EXP[%02X]", i, p0, p1);
      }
      else {
        EXPECT_EQ(dstX8[i], 0u).message("[%u] Detected buffer overrun after A8<-PRGB32 conversion", i);
      }
    }
  }
}

// XRGB32 <-> RGB24 Conversion Tests
// ---------------------------------

static void testRgb32Rgb24Conversions() noexcept {
  INFO("Testing ?RGB32 <-> RGB24 conversions");

  // Pixel formats.
  BLFormatInfo rgb32_format = bl_format_info[BL_FORMAT_XRGB32];
  BLFormatInfo rgb24_format { 24, BLFormatFlags(BL_FORMAT_FLAG_RGB | BL_FORMAT_FLAG_BE), {{ { 8, 8, 8, 0 }, { 16, 8, 0, 0 } }} };
  BLFormatInfo bgr24_format { 24, BLFormatFlags(BL_FORMAT_FLAG_RGB | BL_FORMAT_FLAG_LE), {{ { 8, 8, 8, 0 }, { 16, 8, 0, 0 } }} };

  const uint8_t all_zeros[4] {};

  // Pixel buffers.
  uint8_t srcRgb24[256 * 3];
  uint8_t dstRgb24[256 * 3];
  uint32_t rgb32[256];

  // Prepare.
  uint32_t i;
  uint32_t n;

  for (i = 0; i < 256 * 3; i += 3) {
    srcRgb24[i    ] = uint8_t((i + 0) & 0xFF);
    srcRgb24[i + 1] = uint8_t((i + 1) & 0xFF);
    srcRgb24[i + 2] = uint8_t((i + 2) & 0xFF);
  }

  BLPixelConverter cvtRgb32FromRgb24;
  BLPixelConverter cvtRgb32FromBgr24;
  BLPixelConverter cvtBgr24FromRgb32;
  BLPixelConverter cvtRgb24FromRgb32;

  EXPECT_SUCCESS(cvtRgb32FromRgb24.create(rgb32_format, rgb24_format));
  EXPECT_SUCCESS(cvtRgb32FromBgr24.create(rgb32_format, bgr24_format));

  EXPECT_SUCCESS(cvtRgb24FromRgb32.create(rgb24_format, rgb32_format));
  EXPECT_SUCCESS(cvtBgr24FromRgb32.create(bgr24_format, rgb32_format));

  // This would test the conversion and also whether the SIMD implementation
  // is okay. We test for 1..256 pixels and verify all pixels from 0..255.
  for (n = 1; n < 256; n++) {
    memset(rgb32, 0, sizeof(rgb32));
    EXPECT_SUCCESS(cvtRgb32FromRgb24.convert_span(rgb32, srcRgb24, n));

    for (i = 0; i < 256; i++) {
      if (i < n) {
        uint32_t p0 = rgb32[i];
        uint32_t p1 = RgbaInternal::packRgba32(srcRgb24[i * 3 + 0], srcRgb24[i * 3 + 1], srcRgb24[i * 3 + 2]);
        EXPECT_EQ(p0, p1)
          .message("[%u] RGB32<-RGB24 conversion error OUT[%08X] != EXP[%08X]", i, p0, p1);
      }
      else {
        EXPECT_EQ(rgb32[i], 0u)
          .message("[%u] Detected buffer overrun after RGB32<-RGB24 conversion", i);
      }
    }
  }

  for (n = 1; n < 256; n++) {
    memset(dstRgb24, 0, sizeof(dstRgb24));
    EXPECT_SUCCESS(cvtRgb24FromRgb32.convert_span(dstRgb24, rgb32, n));

    for (i = 0; i < 256; i++) {
      if (i < n) {
        EXPECT_EQ(memcmp(dstRgb24 + i * 3, srcRgb24 + i * 3, 3), 0)
          .message("[%u] RGB24<-RGB32 conversion error OUT[%02X|%02X|%02X] != EXP[%02X|%02X|%02X]", i,
                   dstRgb24[i * 3 + 0], dstRgb24[i * 3 + 1], dstRgb24[i * 3 + 2],
                   srcRgb24[i * 3 + 0], srcRgb24[i * 3 + 1], srcRgb24[i * 3 + 2]);
      }
      else {
        EXPECT_EQ(memcmp(dstRgb24 + i * 3, all_zeros, 3), 0)
          .message("[%u] Detected buffer overrun after RGB24<-RGB32 conversion", i);
      }
    }
  }

  for (n = 1; n < 256; n++) {
    memset(rgb32, 0, sizeof(rgb32));
    EXPECT_SUCCESS(cvtRgb32FromBgr24.convert_span(rgb32, srcRgb24, n));

    for (i = 0; i < 256; i++) {
      if (i < n) {
        uint32_t p0 = rgb32[i];
        uint32_t p1 = RgbaInternal::packRgba32(srcRgb24[i * 3 + 2], srcRgb24[i * 3 + 1], srcRgb24[i * 3 + 0]);
        EXPECT_EQ(p0, p1)
          .message("[%u] RGB32<-BGR24 conversion error OUT[%08X] != EXP[%08X]", i, p0, p1);
      }
      else {
        EXPECT_EQ(rgb32[i], 0u)
          .message("[%u] Detected buffer overrun after RGB32<-BGR24 conversion", i);
      }
    }
  }

  for (n = 1; n < 256; n++) {
    memset(dstRgb24, 0, sizeof(dstRgb24));
    EXPECT_SUCCESS(cvtBgr24FromRgb32.convert_span(dstRgb24, rgb32, n));

    for (i = 0; i < 256; i++) {
      if (i < n) {
        EXPECT_EQ(memcmp(dstRgb24 + i * 3, srcRgb24 + i * 3, 3), 0)
          .message("[%u] BGR24<-RGB32 conversion error OUT[%02X|%02X|%02X] != EXP[%02X|%02X|%02X]", i,
                    dstRgb24[i * 3 + 0], dstRgb24[i * 3 + 1], dstRgb24[i * 3 + 2],
                    srcRgb24[i * 3 + 0], srcRgb24[i * 3 + 1], srcRgb24[i * 3 + 2]);
      }
      else {
        EXPECT_EQ(memcmp(dstRgb24 + i * 3, all_zeros, 3), 0)
          .message("[%u] Detected buffer overrun after BGR24<-RGB32 conversion", i);
      }
    }
  }
}

// Premultiply / Unpremultiply Conversion Tests
// --------------------------------------------

static void test_premultiply_conversions() noexcept {
  INFO("Testing premultiply & unpremultiply conversions");

  uint32_t i;
  uint32_t n;
  uint32_t f;

  constexpr uint32_t N = 1024;

  uint32_t src[N];
  uint32_t dst[N];
  uint32_t unp[N];

  uint64_t default_seed = 0x1234;

  BLFormatInfo unpremultiplied_fmt[4];
  BLFormatInfo premultiplied_fmt[4];

  // Shifts in host byte-order.
  static const uint8_t format_shifts[4][4] = {
    { 16,  8,  0, 24 }, // 0x[AA|RR|GG|BB]
    {  0,  8, 16, 24 }, // 0x[AA|BB|GG|RR]
    { 24, 16,  8,  0 }, // 0x[RR|GG|BB|AA]
    {  8, 16, 24,  0 }  // 0x[BB|GG|RR|AA]
  };

  static const char* format_names[4] = {
    "ARGB32",
    "ABGR32",
    "RGBA32",
    "BGRA32"
  };

  // Initialize both formats.
  for (f = 0; f < 4; f++) {
    const uint8_t* s = format_shifts[f];

    unpremultiplied_fmt[f].depth = 32;
    unpremultiplied_fmt[f].flags = BL_FORMAT_FLAG_RGBA;
    unpremultiplied_fmt[f].set_sizes(8, 8, 8, 8);
    unpremultiplied_fmt[f].set_shifts(s[0], s[1], s[2], s[3]);

    premultiplied_fmt[f] = unpremultiplied_fmt[f];
    premultiplied_fmt[f].add_flags(BL_FORMAT_FLAG_PREMULTIPLIED);
  }

  BLRandom r(default_seed);
  for (i = 0; i < N; i++) {
    src[i] = r.next_uint32();
  }

  for (f = 0; f < 4; f++) {
    INFO("  32-bit %s format", format_names[f]);

    bool leading_alpha = format_shifts[f][3] == 24;
    BLPixelConverter cvt1;
    BLPixelConverter cvt2;

    EXPECT_SUCCESS(cvt1.create(premultiplied_fmt[f], unpremultiplied_fmt[f]));
    EXPECT_SUCCESS(cvt2.create(unpremultiplied_fmt[f], premultiplied_fmt[f]));

    for (n = 1; n < N; n++) {
      memset(dst, 0, sizeof(dst));
      memset(unp, 0, sizeof(unp));

      EXPECT_SUCCESS(cvt1.convert_span(dst, src, n));
      EXPECT_SUCCESS(cvt2.convert_span(unp, dst, n));

      for (i = 0; i < n; i++) {
        if (i < n) {
          uint32_t sp = src[i]; // Source pixel.
          uint32_t dp = dst[i]; // Premultiply(sp).
          uint32_t up = unp[i]; // Unpremultiply(dp).

          uint32_t s0 = (sp >> 24) & 0xFFu;
          uint32_t s1 = (sp >> 16) & 0xFFu;
          uint32_t s2 = (sp >>  8) & 0xFFu;
          uint32_t s3 = (sp >>  0) & 0xFFu;

          uint32_t a = leading_alpha ? s0 : s3;
          if (leading_alpha)
            s0 = 0xFF;
          else
            s3 = 0xFF;

          uint32_t e0 = PixelOps::Scalar::udiv255(s0 * a);
          uint32_t e1 = PixelOps::Scalar::udiv255(s1 * a);
          uint32_t e2 = PixelOps::Scalar::udiv255(s2 * a);
          uint32_t e3 = PixelOps::Scalar::udiv255(s3 * a);
          uint32_t ep = (e0 << 24) | (e1 << 16) | (e2 << 8) | e3;

          EXPECT_EQ(dp, ep)
            .message("[%u] OUT[0x%08X] != EXP[0x%08X] <- Premultiply(SRC[0x%08X])", i, dp, ep, sp);

          if (leading_alpha)
            PixelOps::Scalar::unpremultiply_rgb_8bit(e1, e2, e3, e0);
          else
            PixelOps::Scalar::unpremultiply_rgb_8bit(e0, e1, e2, e3);

          ep = (e0 << 24) | (e1 << 16) | (e2 << 8) | e3;
          EXPECT_EQ(up, ep)
            .message("[%u] OUT[0x%08X] != EXP[0x%08X] <- Unpremultiply(DST[0x%08X])", i, up, ep, dp);
        }
        else {
          uint32_t dp = dst[i];
          EXPECT_EQ(dp, 0u)
            .message("[%u] Detected buffer overrun", i);
        }
      }
    }
  }
}

// Generic Conversion Tests
// ------------------------

template<typename T>
struct BLPixelConverterGenericTest {
  static void fill_masks(BLFormatInfo& fi) noexcept {
    fi.shifts[0] = uint8_t(T::kR > 0u ? IntOps::ctz(uint32_t(T::kR)) : uint32_t(0));
    fi.shifts[1] = uint8_t(T::kG > 0u ? IntOps::ctz(uint32_t(T::kG)) : uint32_t(0));
    fi.shifts[2] = uint8_t(T::kB > 0u ? IntOps::ctz(uint32_t(T::kB)) : uint32_t(0));
    fi.shifts[3] = uint8_t(T::kA > 0u ? IntOps::ctz(uint32_t(T::kA)) : uint32_t(0));
    fi.sizes[0] = uint8_t(T::kR > 0u ? IntOps::ctz(~(uint32_t(T::kR) >> fi.shifts[0])) : uint32_t(0));
    fi.sizes[1] = uint8_t(T::kG > 0u ? IntOps::ctz(~(uint32_t(T::kG) >> fi.shifts[1])) : uint32_t(0));
    fi.sizes[2] = uint8_t(T::kB > 0u ? IntOps::ctz(~(uint32_t(T::kB) >> fi.shifts[2])) : uint32_t(0));
    fi.sizes[3] = uint8_t(T::kA > 0u ? IntOps::ctz(~(uint32_t(T::kA) >> fi.shifts[3])) : uint32_t(0));
  }

  static void testPrgb32() noexcept {
    INFO("  %d-bit %s format", T::kDepth, T::format_string());

    BLPixelConverter from;
    BLPixelConverter back;

    BLFormatInfo fi {};
    fill_masks(fi);
    fi.depth = T::kDepth;
    fi.flags = fi.sizes[3] ? BLFormatFlags(BL_FORMAT_FLAG_RGBA | BL_FORMAT_FLAG_PREMULTIPLIED) : BL_FORMAT_FLAG_RGB;

    EXPECT_SUCCESS(from.create(fi, bl_format_info[BL_FORMAT_PRGB32]))
      .message("%s: Failed to create from [%dbpp 0x%08X 0x%08X 0x%08X 0x%08X]", T::format_string(), T::kDepth, T::kR, T::kG, T::kB, T::kA);

    EXPECT_SUCCESS(back.create(bl_format_info[BL_FORMAT_PRGB32], fi))
      .message("%s: Failed to create to [%dbpp 0x%08X 0x%08X 0x%08X 0x%08X]", T::format_string(), T::kDepth, T::kR, T::kG, T::kB, T::kA);

    enum : uint32_t { kCount = 8 };

    static const uint32_t src[kCount] = {
      0xFF000000, 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF,
      0xFFFF0000, 0xFFFF00FF, 0xFFFFFF00, 0xFFFFFFFF
    };

    uint32_t dst[kCount];
    uint8_t buf[kCount * 16];

    // The test is rather basic now, we basically convert from PRGB to external
    // pixel format, then back, and then compare if the output is matching input.
    // In the future we should also check the intermediate result.
    from.convert_span(buf, src, kCount);
    back.convert_span(dst, buf, kCount);

    for (uint32_t i = 0; i < kCount; i++) {
      uint32_t mid = 0;
      switch (uint32_t(T::kDepth)) {
        case 8 : mid = MemOps::readU8(buf + i); break;
        case 16: mid = MemOps::readU16u(buf + i * 2u); break;
        case 24: mid = MemOps::readU24u(buf + i * 3u); break;
        case 32: mid = MemOps::readU32u(buf + i * 4u); break;
      }

      EXPECT_EQ(dst[i], src[i])
        .message("%s: [%u] Dst(%08X) <- 0x%08X <- Src(0x%08X) [%dbpp %08X|%08X|%08X|%08X]",
                 T::format_string(), i, dst[i], mid, src[i], T::kDepth, T::kA, T::kR, T::kG, T::kB);
    }
  }

  static void test() noexcept {
    testPrgb32();
  }
};

#define BL_PIXEL_TEST(FORMAT, DEPTH, R_MASK, G_MASK, B_MASK, A_MASK)      \
  struct Test_##FORMAT {                                                  \
    static inline const char* format_string() noexcept { return #FORMAT; } \
                                                                          \
    enum : uint32_t {                                                     \
      kDepth = DEPTH,                                                     \
      kR = R_MASK,                                                        \
      kG = G_MASK,                                                        \
      kB = B_MASK,                                                        \
      kA = A_MASK                                                         \
    };                                                                    \
  }
BL_PIXEL_TEST(XRGB_0555, 16, 0x00007C00u, 0x000003E0u, 0x0000001Fu, 0x00000000u);
BL_PIXEL_TEST(XBGR_0555, 16, 0x0000001Fu, 0x000003E0u, 0x00007C00u, 0x00000000u);
BL_PIXEL_TEST(XRGB_0565, 16, 0x0000F800u, 0x000007E0u, 0x0000001Fu, 0x00000000u);
BL_PIXEL_TEST(XBGR_0565, 16, 0x0000001Fu, 0x000007E0u, 0x0000F800u, 0x00000000u);
BL_PIXEL_TEST(ARGB_4444, 16, 0x00000F00u, 0x000000F0u, 0x0000000Fu, 0x0000F000u);
BL_PIXEL_TEST(ABGR_4444, 16, 0x0000000Fu, 0x000000F0u, 0x00000F00u, 0x0000F000u);
BL_PIXEL_TEST(RGBA_4444, 16, 0x0000F000u, 0x00000F00u, 0x000000F0u, 0x0000000Fu);
BL_PIXEL_TEST(BGRA_4444, 16, 0x000000F0u, 0x00000F00u, 0x0000F000u, 0x0000000Fu);
BL_PIXEL_TEST(XRGB_0888, 24, 0x00FF0000u, 0x0000FF00u, 0x000000FFu, 0x00000000u);
BL_PIXEL_TEST(XBGR_0888, 24, 0x000000FFu, 0x0000FF00u, 0x00FF0000u, 0x00000000u);
BL_PIXEL_TEST(XRGB_8888, 32, 0x00FF0000u, 0x0000FF00u, 0x000000FFu, 0x00000000u);
BL_PIXEL_TEST(XBGR_8888, 32, 0x000000FFu, 0x0000FF00u, 0x00FF0000u, 0x00000000u);
BL_PIXEL_TEST(RGBX_8888, 32, 0xFF000000u, 0x00FF0000u, 0x0000FF00u, 0x00000000u);
BL_PIXEL_TEST(BGRX_8888, 32, 0x0000FF00u, 0x00FF0000u, 0xFF000000u, 0x00000000u);
BL_PIXEL_TEST(ARGB_8888, 32, 0x00FF0000u, 0x0000FF00u, 0x000000FFu, 0xFF000000u);
BL_PIXEL_TEST(ABGR_8888, 32, 0x000000FFu, 0x0000FF00u, 0x00FF0000u, 0xFF000000u);
BL_PIXEL_TEST(RGBA_8888, 32, 0xFF000000u, 0x00FF0000u, 0x0000FF00u, 0x000000FFu);
BL_PIXEL_TEST(BGRA_8888, 32, 0x0000FF00u, 0x00FF0000u, 0xFF000000u, 0x000000FFu);
BL_PIXEL_TEST(BRGA_8888, 32, 0x00FF0000u, 0x0000FF00u, 0xFF000000u, 0x000000FFu);
#undef BL_PIXEL_TEST

static void test_generic_conversions() noexcept {
  INFO("Testing generic conversions");
  BLPixelConverterGenericTest<Test_XRGB_0555>::test();
  BLPixelConverterGenericTest<Test_XBGR_0555>::test();
  BLPixelConverterGenericTest<Test_XRGB_0565>::test();
  BLPixelConverterGenericTest<Test_XBGR_0565>::test();
  BLPixelConverterGenericTest<Test_ARGB_4444>::test();
  BLPixelConverterGenericTest<Test_ABGR_4444>::test();
  BLPixelConverterGenericTest<Test_RGBA_4444>::test();
  BLPixelConverterGenericTest<Test_BGRA_4444>::test();
  BLPixelConverterGenericTest<Test_XRGB_0888>::test();
  BLPixelConverterGenericTest<Test_XBGR_0888>::test();
  BLPixelConverterGenericTest<Test_XRGB_8888>::test();
  BLPixelConverterGenericTest<Test_XBGR_8888>::test();
  BLPixelConverterGenericTest<Test_RGBX_8888>::test();
  BLPixelConverterGenericTest<Test_BGRX_8888>::test();
  BLPixelConverterGenericTest<Test_ARGB_8888>::test();
  BLPixelConverterGenericTest<Test_ABGR_8888>::test();
  BLPixelConverterGenericTest<Test_RGBA_8888>::test();
  BLPixelConverterGenericTest<Test_BGRA_8888>::test();
  BLPixelConverterGenericTest<Test_BRGA_8888>::test();
}

UNIT(pixel_converter, BL_TEST_GROUP_IMAGE_UTILITIES) {
  testRgb32A8Conversions();
  testRgb32Rgb24Conversions();
  test_premultiply_conversions();
  test_generic_conversions();
}

} // {Tests}
} // {bl}

#endif // BL_TEST
