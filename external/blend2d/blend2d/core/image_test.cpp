// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_test_p.h>
#if defined(BL_TEST)

#include <blend2d/core/image_p.h>

// bl::Image - Tests
// =================

namespace bl {
namespace Tests {

UNIT(image, BL_TEST_GROUP_IMAGE_CONTAINERS) {
  constexpr uint32_t kSize = 256;

  INFO("Testing BLImage::create() and BLImage::make_mutable()");
  {
    BLImage img0;
    BLImage img1;

    EXPECT_SUCCESS(img0.create(kSize, kSize, BL_FORMAT_PRGB32));
    EXPECT_SUCCESS(img1.create(kSize, kSize, BL_FORMAT_PRGB32));

    EXPECT_EQ(img0.width(), int(kSize));
    EXPECT_EQ(img0.height(), int(kSize));
    EXPECT_EQ(img0.format(), BL_FORMAT_PRGB32);

    EXPECT_EQ(img1.width(), int(kSize));
    EXPECT_EQ(img1.height(), int(kSize));
    EXPECT_EQ(img1.format(), BL_FORMAT_PRGB32);

    BLImageData imgData0;
    BLImageData imgData1;

    EXPECT_SUCCESS(img0.make_mutable(&imgData0));
    EXPECT_SUCCESS(img1.make_mutable(&imgData1));

    EXPECT_EQ(imgData0.size.w, int(kSize));
    EXPECT_EQ(imgData0.size.h, int(kSize));
    EXPECT_EQ(imgData0.format, BL_FORMAT_PRGB32);

    EXPECT_EQ(imgData1.size.w, int(kSize));
    EXPECT_EQ(imgData1.size.h, int(kSize));
    EXPECT_EQ(imgData1.format, BL_FORMAT_PRGB32);

    // Direct memory manipulation.
    for (size_t y = 0; y < kSize; y++) {
      memset(static_cast<uint8_t*>(imgData0.pixel_data) + intptr_t(y) * imgData0.stride, int(y & 0xFF), kSize * 4);
      memset(static_cast<uint8_t*>(imgData1.pixel_data) + intptr_t(y) * imgData1.stride, int(y & 0xFF), kSize * 4);
    }

    EXPECT_TRUE(img0.equals(img1));
  }

  INFO("Testing BLImage::create() and BLImage::convert()");
  {
    BLImage img0;
    BLImage img1;

    EXPECT_SUCCESS(img0.create(kSize, kSize, BL_FORMAT_PRGB32));
    EXPECT_SUCCESS(img1.create(kSize, kSize, BL_FORMAT_A8));

    EXPECT_EQ(img0.width(), int(kSize));
    EXPECT_EQ(img0.height(), int(kSize));
    EXPECT_EQ(img0.format(), BL_FORMAT_PRGB32);

    EXPECT_EQ(img1.width(), int(kSize));
    EXPECT_EQ(img1.height(), int(kSize));
    EXPECT_EQ(img1.format(), BL_FORMAT_A8);

    BLImageData imgData0;
    BLImageData imgData1;

    EXPECT_SUCCESS(img0.make_mutable(&imgData0));
    EXPECT_SUCCESS(img1.make_mutable(&imgData1));

    for (size_t y = 0; y < kSize; y++) {
      uint32_t* line0 = reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(imgData0.pixel_data) + intptr_t(y) * imgData0.stride);
      uint8_t* line1 = static_cast<uint8_t*>(imgData1.pixel_data) + intptr_t(y) * imgData1.stride;

      for (uint32_t x = 0; x < kSize; x++) {
        line0[x] = uint32_t((x + y) & 0xFF) << 24;
        line1[x] = uint8_t((x + y) & 0xFF);
      }
    }

    EXPECT_SUCCESS(img0.convert(BL_FORMAT_A8));
    EXPECT_TRUE(img0.equals(img1));
  }

  INFO("Testing BLImage::create_from_data()");
  {
    struct ExternalDataInfo {
      void* external_data;
      size_t destroy_count;
    };

    void* external_data_ptr = malloc(kSize * kSize * 4);
    EXPECT_NE(external_data_ptr, nullptr);

    BLImage img;
    BLImageData img_data;

    // Test create_from_data() without a destroy handler.
    EXPECT_SUCCESS(img.create_from_data(kSize, kSize, BL_FORMAT_PRGB32, external_data_ptr, kSize * 4, BL_DATA_ACCESS_RW));
    EXPECT_EQ(img.width(), int(kSize));
    EXPECT_EQ(img.height(), int(kSize));
    EXPECT_EQ(img.format(), BL_FORMAT_PRGB32);

    EXPECT_SUCCESS(img.make_mutable(&img_data));
    EXPECT_EQ(img_data.size.w, int(kSize));
    EXPECT_EQ(img_data.size.h, int(kSize));
    EXPECT_EQ(img_data.format, BL_FORMAT_PRGB32);
    EXPECT_EQ(img_data.pixel_data, external_data_ptr);

    EXPECT_SUCCESS(img.reset());

    // Test create_from_data() with a destroy handler.
    ExternalDataInfo external_data_info {};

    auto destroy_func = [](void* impl, void* external_data, void* user_data) noexcept -> void {
      bl_unused(impl);

      ExternalDataInfo* info = static_cast<ExternalDataInfo*>(user_data);
      info->destroy_count++;
      info->external_data = external_data;
    };

    EXPECT_SUCCESS(img.create_from_data(kSize, kSize, BL_FORMAT_PRGB32, external_data_ptr, kSize * 4, BL_DATA_ACCESS_RW, destroy_func, &external_data_info));
    EXPECT_EQ(img.width(), int(kSize));
    EXPECT_EQ(img.height(), int(kSize));
    EXPECT_EQ(img.format(), BL_FORMAT_PRGB32);

    EXPECT_SUCCESS(img.make_mutable(&img_data));
    EXPECT_EQ(img_data.size.w, int(kSize));
    EXPECT_EQ(img_data.size.h, int(kSize));
    EXPECT_EQ(img_data.format, BL_FORMAT_PRGB32);
    EXPECT_EQ(img_data.pixel_data, external_data_ptr);

    // Verify our info is zero initialized.
    EXPECT_EQ(external_data_info.external_data, nullptr);
    EXPECT_EQ(external_data_info.destroy_count, 0u);

    // Destroy the image, should call the destroy handler.
    EXPECT_SUCCESS(img.reset());

    // Verify the destroy handler was called once.
    EXPECT_EQ(external_data_info.external_data, external_data_ptr);
    EXPECT_EQ(external_data_info.destroy_count, 1u);

    free(external_data_ptr);
  }

  INFO("Testing BLImage reference counting");
  {
    BLImage img0;
    BLImageData imgData0;

    EXPECT_SUCCESS(img0.create(kSize, kSize, BL_FORMAT_PRGB32));
    EXPECT_SUCCESS(img0.make_mutable(&imgData0));

    for (size_t y = 0; y < kSize; y++) {
      uint32_t* line = reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(imgData0.pixel_data) + intptr_t(y) * imgData0.stride);
      for (uint32_t x = 0; x < kSize; x++) {
        line[x] = uint32_t((x + y) & 0xFF) << 24;
      }
    }

    BLImage img1(img0);
    EXPECT_EQ(img0, img1);

    BLImageData imgData1;
    EXPECT_SUCCESS(img0.make_mutable(&imgData0));
    EXPECT_SUCCESS(img1.make_mutable(&imgData1));

    EXPECT_NE(imgData0.pixel_data, imgData1.pixel_data);
    EXPECT_EQ(img0, img1);
  }
}

} // {Tests}
} // {bl}

#endif // BL_TEST
