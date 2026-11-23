// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_test_p.h>
#if defined(BL_TEST)

#include <blend2d/core/array_p.h>
#include <blend2d/core/image_p.h>
#include <blend2d/core/imagecodec.h>

// bl::ImageCodec - Tests
// ======================

namespace bl {
namespace Tests {

UNIT(image_codec_find, BL_TEST_GROUP_IMAGE_CODEC_ROUNDTRIP) {
  INFO("Testing BLImageCodec::find_by_name() and BLImageCodec::find_by_data()");
  {
    static const uint8_t bmp_signature[2] = { 'B', 'M' };
    static const uint8_t png_signature[8] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
    static const uint8_t jpg_signature[3] = { 0xFF, 0xD8, 0xFF };

    BLImageCodec codec;
    BLImageCodec bmp;
    BLImageCodec png;
    BLImageCodec jpg;

    EXPECT_SUCCESS(bmp.find_by_name("BMP"));
    EXPECT_SUCCESS(png.find_by_name("PNG"));
    EXPECT_SUCCESS(jpg.find_by_name("JPEG"));

    EXPECT_SUCCESS(codec.find_by_extension("bmp"));
    EXPECT_EQ(codec, bmp);

    EXPECT_SUCCESS(codec.find_by_extension(".bmp"));
    EXPECT_EQ(codec, bmp);

    EXPECT_SUCCESS(codec.find_by_extension("SomeFile.BMp"));
    EXPECT_EQ(codec, bmp);

    EXPECT_SUCCESS(codec.find_by_extension("png"));
    EXPECT_EQ(codec, png);

    EXPECT_SUCCESS(codec.find_by_extension(".png"));
    EXPECT_EQ(codec, png);

    EXPECT_SUCCESS(codec.find_by_extension(".jpg"));
    EXPECT_EQ(codec, jpg);

    EXPECT_SUCCESS(codec.find_by_extension(".jpeg"));
    EXPECT_EQ(codec, jpg);

    EXPECT_SUCCESS(codec.find_by_data(bmp_signature, BL_ARRAY_SIZE(bmp_signature)));
    EXPECT_EQ(codec, bmp);

    EXPECT_SUCCESS(codec.find_by_data(png_signature, BL_ARRAY_SIZE(png_signature)));
    EXPECT_EQ(codec, png);

    EXPECT_SUCCESS(codec.find_by_data(jpg_signature, BL_ARRAY_SIZE(jpg_signature)));
    EXPECT_EQ(codec, jpg);
  }
}

} // {Tests}
} // {bl}

#endif // BL_TEST
