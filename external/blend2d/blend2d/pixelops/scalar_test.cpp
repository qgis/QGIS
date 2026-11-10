// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_test_p.h>
#if defined(BL_TEST)

#include <blend2d/pixelops/scalar_p.h>

// bl::PixelOps - Scalar - Tests
// =============================

namespace bl {
namespace Tests {

using namespace PixelOps;

static void test_udiv255() noexcept {
  INFO("bl::PixelOps::Scalar::udiv255()");
  for (uint32_t i = 0; i < 255 * 255; i++) {
    uint32_t result = Scalar::udiv255(i);
    uint32_t j = i + 128;

    // This version doesn't overflow 16 bits.
    uint32_t expected = (j + (j >> 8)) >> 8;

    EXPECT_EQ(result, expected)
      .message("bl::PixelOps::Scalar::udiv255(%u) -> %u (Expected %u)", i, result, expected);
  }
}

static void test_cvt_xrgb32_0888_from_xrgb16_0555() noexcept {
  INFO("Testing cvt_xrgb32_0888_from_xrgb16_0555");
  uint32_t c = 0;

  for (;;) {
    uint32_t r = ((c >> 10) & 0x1F) << 3;
    uint32_t g = ((c >>  5) & 0x1F) << 3;
    uint32_t b = ((c      ) & 0x1F) << 3;

    uint32_t result   = Scalar::cvt_xrgb32_0888_from_xrgb16_0555(c);
    uint32_t expected = RgbaInternal::packRgba32(r + (r >> 5), g + (g >> 5), b + (b >> 5), 0xFF);

    EXPECT_EQ(result, expected)
      .message("bl::PixelOps::Scalar::cvt_xrgb32_0888_from_xrgb16_0555() - %08X -> %08X (Expected %08X)", c, result, expected);

    if (c == 0xFFFF)
      break;
    c++;
  }
}

static void test_cvt_xrgb32_0888_from_xrgb16_0565() noexcept {
  INFO("Testing cvt_xrgb32_0888_from_xrgb16_0565");
  uint32_t c = 0;

  for (;;) {
    uint32_t r = ((c >> 11) & 0x1F) << 3;
    uint32_t g = ((c >>  5) & 0x3F) << 2;
    uint32_t b = ((c      ) & 0x1F) << 3;

    uint32_t result   = Scalar::cvt_xrgb32_0888_from_xrgb16_0565(c);
    uint32_t expected = RgbaInternal::packRgba32(r + (r >> 5), g + (g >> 6), b + (b >> 5), 0xFF);

    EXPECT_EQ(result, expected)
      .message("bl::PixelOps::Scalar::cvt_xrgb32_0888_from_xrgb16_0555() - %08X -> %08X (Expected %08X)", c, result, expected);

    if (c == 0xFFFF)
      break;
    c++;
  }
}

static void test_cvt_argb32_8888_from_argb16_4444() noexcept {
  INFO("Testing cvt_argb32_8888_from_argb16_4444");
  uint32_t c = 0;

  for (;;) {
    uint32_t a = ((c >> 12) & 0xF) * 0x11;
    uint32_t r = ((c >>  8) & 0xF) * 0x11;
    uint32_t g = ((c >>  4) & 0xF) * 0x11;
    uint32_t b = ((c      ) & 0xF) * 0x11;

    uint32_t result   = Scalar::cvt_argb32_8888_from_argb16_4444(c);
    uint32_t expected = RgbaInternal::packRgba32(r, g, b, a);

    EXPECT_EQ(result, expected)
      .message("bl::PixelOps::Scalar::cvt_argb32_8888_from_argb16_4444() - %08X -> %08X (Expected %08X)", c, result, expected);

    if (c == 0xFFFF)
      break;
    c++;
  }
}

static void test_cvt_prgb32_8888_from_argb32_8888() noexcept {
  INFO("Testing cvt_prgb32_8888_from_argb32_8888");

  uint32_t i;
  uint32_t c = 0;

  for (i = 0; i < 10000000; i++) {
    uint32_t a = (c >> 24) & 0xFF;
    uint32_t r = (c >> 16) & 0xFF;
    uint32_t g = (c >>  8) & 0xFF;
    uint32_t b = (c      ) & 0xFF;

    uint32_t result = Scalar::cvt_prgb32_8888_from_argb32_8888(c);
    uint32_t expected = RgbaInternal::packRgba32(Scalar::udiv255(r * a), Scalar::udiv255(g * a), Scalar::udiv255(b * a), a);

    EXPECT_EQ(result, expected)
      .message("bl::PixelOps::Scalar::cvt_prgb32_8888_from_argb32_8888() - %08X -> %08X (Expected %08X)", c, result, expected);

    c += 7919;
  }
}

UNIT(pixelops_scalar, BL_TEST_GROUP_IMAGE_PIXEL_OPS) {
  test_udiv255();
  test_cvt_xrgb32_0888_from_xrgb16_0555();
  test_cvt_xrgb32_0888_from_xrgb16_0565();
  test_cvt_argb32_8888_from_argb16_4444();
  test_cvt_prgb32_8888_from_argb32_8888();
}

} // {Tests}
} // {bl}

#endif // BL_TEST

