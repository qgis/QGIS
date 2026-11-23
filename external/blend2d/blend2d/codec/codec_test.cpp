// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_test_p.h>
#if defined(BL_TEST)

#include <blend2d/core/array.h>
#include <blend2d/core/context.h>
#include <blend2d/core/image.h>
#include <blend2d/core/imagecodec.h>
#include <blend2d/core/imagedecoder.h>
#include <blend2d/core/imageencoder.h>
#include <blend2d/core/random.h>

#include <blend2d-testing/commons/imagediff.h>

// bl::Codecs - Tests
// ==================

namespace bl::Codecs::Tests {

static void render_simple_image(BLImage& image, BLRandom& rnd, uint32_t cmd_count) noexcept {
  BLContext ctx(image);
  ctx.clear_all();

  double w = image.width();
  double h = image.height();
  double s = bl_min(w, h);

  for (uint32_t i = 0; i < cmd_count; i++) {
    uint32_t shape = rnd.next_uint32() & 0x3u;
    BLRgba32 color = BLRgba32(rnd.next_uint32() | 0xFF000000u);

    ctx.set_fill_style(color);

    switch (shape) {
      case 0: {
        double x0 = rnd.next_double() * w;
        double y0 = rnd.next_double() * h;
        double x1 = rnd.next_double() * w;
        double y1 = rnd.next_double() * h;

        double rx = bl_min(x0, x1);
        double ry = bl_min(y0, y1);
        double rw = bl_max(x0, x1) - rx;
        double rh = bl_max(y0, y1) - ry;

        ctx.fill_rect(rx, ry, rw, rh);
        break;
      }

      case 1: {
        double x0 = rnd.next_double() * w;
        double y0 = rnd.next_double() * h;
        double x1 = rnd.next_double() * w;
        double y1 = rnd.next_double() * h;
        double x2 = rnd.next_double() * w;
        double y2 = rnd.next_double() * h;

        ctx.fill_triangle(x0, y0, x1, y1, x2, y2);
        break;
      }

      case 2: {
        double cx = rnd.next_double() * w;
        double cy = rnd.next_double() * h;
        double r = rnd.next_double() * s;

        ctx.fill_circle(cx, cy, r);
        break;
      }

      case 3: {
        double cx = rnd.next_double() * w;
        double cy = rnd.next_double() * h;
        double r = rnd.next_double() * s;
        double start = rnd.next_double() * 3;
        double sweep = rnd.next_double() * 6;

        ctx.fill_pie(cx, cy, r, start, sweep);
        break;
      }
    }
  }
}

struct TestOptions {
  uint32_t compression_level = 0xFFFFFFFFu;

  BL_INLINE bool has_compression_level() const noexcept { return compression_level != 0xFFFFFFFFu; }
};

static void test_encoding_decoding_random_images(BLSizeI size, BLFormat fmt, BLImageCodec& codec, BLRandom& rnd, uint32_t test_count, uint32_t cmd_count, const TestOptions& test_options) noexcept {
  for (uint32_t i = 0; i < test_count; i++) {
    BLImage image1;

    EXPECT_SUCCESS(image1.create(size.w, size.h, fmt));
    render_simple_image(image1, rnd, cmd_count);

    BLImageEncoder encoder;
    EXPECT_SUCCESS(codec.create_encoder(&encoder));

    if (test_options.has_compression_level()) {
      EXPECT_SUCCESS(encoder.set_property("compression", BLVar(test_options.compression_level)));
    }

    BLArray<uint8_t> encoded_data;
    encoder.write_frame(encoded_data, image1);

    BLImageDecoder decoder;
    EXPECT_SUCCESS(codec.create_decoder(&decoder));

    BLImage image2;
    EXPECT_SUCCESS(decoder.read_frame(image2, encoded_data));

    ImageUtils::DiffInfo diff_info = ImageUtils::diff_info(image1, image2);
    EXPECT_EQ(diff_info.max_diff, 0u);
  }
}

static constexpr BLSizeI image_codec_test_sizes[] = {
  { 1, 1 },
  { 1, 2 },
  { 2, 2 },
  { 3, 3 },
  { 4, 4 },
  { 5, 4 },
  { 6, 6 },
  { 1, 7 },
  { 7, 1 },
  { 11, 13 },
  { 15, 15 },
  { 16, 15 },
  { 99, 54 },
  { 132, 23 },
  { 301, 301 }
};

UNIT(image_codec_bmp, BL_TEST_GROUP_IMAGE_CODEC_ROUNDTRIP) {
  static constexpr uint32_t kCmdCount = 10;
  static constexpr uint32_t kTestCount = 100;

  for (BLSizeI image_size : image_codec_test_sizes) {
    BLImageCodec codec;
    EXPECT_SUCCESS(codec.find_by_name("BMP"));

    BLRandom rnd(0x123456789ABCDEFu);
    TestOptions test_options{};

    INFO("Testing BMP encoder & decoder with %dx%d images", image_size.w, image_size.h);
    test_encoding_decoding_random_images(image_size, BL_FORMAT_XRGB32, codec, rnd, kTestCount, kCmdCount, test_options);
    test_encoding_decoding_random_images(image_size, BL_FORMAT_PRGB32, codec, rnd, kTestCount, kCmdCount, test_options);
  }
}

UNIT(image_codec_png, BL_TEST_GROUP_IMAGE_CODEC_ROUNDTRIP) {
  static constexpr uint32_t kCmdCount = 10;
  static constexpr uint32_t kTestCount = 100;

  for (BLSizeI image_size : image_codec_test_sizes) {
    INFO("Testing PNG encoder & decoder with %dx%d images", image_size.w, image_size.h);

    BLImageCodec codec;
    EXPECT_SUCCESS(codec.find_by_name("PNG"));

    BLRandom rnd(0x123456789ABCDEFu);
    for (uint32_t compression_level = 0; compression_level <= 12; compression_level++) {
      TestOptions test_options{};
      test_options.compression_level = compression_level;

      test_encoding_decoding_random_images(image_size, BL_FORMAT_XRGB32, codec, rnd, kTestCount, kCmdCount, test_options);
      test_encoding_decoding_random_images(image_size, BL_FORMAT_PRGB32, codec, rnd, kTestCount, kCmdCount, test_options);
    }
  }
}

UNIT(image_codec_qoi, BL_TEST_GROUP_IMAGE_CODEC_ROUNDTRIP) {
  static constexpr uint32_t kCmdCount = 10;
  static constexpr uint32_t kTestCount = 100;

  for (BLSizeI image_size : image_codec_test_sizes) {
    BLImageCodec codec;
    EXPECT_SUCCESS(codec.find_by_name("QOI"));

    BLRandom rnd(0x123456789ABCDEFu);
    TestOptions test_options{};

    INFO("Testing QOI encoder & decoder with %dx%d images", image_size.w, image_size.h);
    test_encoding_decoding_random_images(image_size, BL_FORMAT_XRGB32, codec, rnd, kTestCount, kCmdCount, test_options);
    test_encoding_decoding_random_images(image_size, BL_FORMAT_PRGB32, codec, rnd, kTestCount, kCmdCount, test_options);
  }
}

} // {bl::Codecs::Tests}

#endif // BL_TEST
