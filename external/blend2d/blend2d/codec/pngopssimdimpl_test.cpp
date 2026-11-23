// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_test_p.h>
#if defined(BL_TEST)

#include <blend2d/core/random.h>
#include <blend2d/core/runtime_p.h>
#include <blend2d/codec/pngcodec_p.h>
#include <blend2d/codec/pngops_p.h>
#include <blend2d/support/intops_p.h>

// bl::Array - Tests
// =================

namespace bl::Png::Tests {

static const uint32_t png_bpp_data[] = { 1, 2, 3, 4, 6, 8 };
static const char* png_filter_names[] = { "None", "Sub", "Up", "Avg", "Paeth", "Random" };

static void fill_random_image(uint8_t* p, uint32_t w, uint32_t h, uint32_t bpp, uint32_t filter, BLRandom rnd) noexcept {
  w *= bpp;

  for (uint32_t y = 0; y < h; y++) {
    if (filter < kFilterTypeCount) {
      *p++ = static_cast<uint8_t>(filter);
    }
    else {
      *p++ = uint8_t(rnd.next_uint32() % kFilterTypeCount);
    }

    for (uint32_t x = 0; x < w; x++) {
      *p++ = uint8_t(rnd.next_uint32() >> 24u);
    }
  }
}

static const uint8_t buffer_overrun_guard[16] = {
  0xFE, 0xAF, 0x10, 0x00, 0xFF, 0x01, 0x02, 0x03,
  0x04, 0xAA, 0xFA, 0xBB, 0xAA, 0x99, 0x88, 0x77
};

static void test_simd_impl(
  Ops::FunctionTable& reference,
  Ops::FunctionTable& optimized,
  const char* impl_name
) noexcept {
  // First test individual filters with each supported BPP.
  BLRandom rnd(0xFEEDFEEDFEEDFEEDu);

  INFO("Testing %s implementation", impl_name);

  uint32_t min_width = 1;
  uint32_t max_width = 111;
  uint32_t h = 24;

  uint32_t buffer_overrun_guard_size = BL_ARRAY_SIZE(buffer_overrun_guard);

  for (uint32_t w = min_width; w <= max_width; w++) {
    for (uint32_t bpp_index = 0; bpp_index < BL_ARRAY_SIZE(png_bpp_data); bpp_index++) {
      uint32_t bpp = png_bpp_data[bpp_index];
      uint32_t bpl = w * bpp + 1u;
      uint32_t png_size = bpl * h;

      uint8_t* ref_image = static_cast<uint8_t*>(malloc(png_size));
      uint8_t* ref_output = static_cast<uint8_t*>(malloc(png_size));
      uint8_t* opt_output_base = static_cast<uint8_t*>(malloc(png_size + 256));

      EXPECT_NOT_NULL(ref_image);
      EXPECT_NOT_NULL(ref_output);
      EXPECT_NOT_NULL(opt_output_base);

      for (uint32_t filter = 0; filter <= kFilterTypeCount; filter++) {
        fill_random_image(ref_image, w, h, bpp, filter, rnd);

        memcpy(ref_output, ref_image, png_size);
        reference.inverse_filter[bpp](ref_output, bpp, bpl, h);

        for (uint32_t misalignment = 0; misalignment < 64; misalignment++) {
          uint8_t* opt_output = IntOps::align_up(opt_output_base, 64) + misalignment;

          memcpy(opt_output, ref_image, png_size);
          memcpy(opt_output + png_size, buffer_overrun_guard, buffer_overrun_guard_size);
          optimized.inverse_filter[bpp](opt_output, bpp, bpl, h);

          bool output_match = memcmp(ref_output, opt_output, png_size) == 0;
          EXPECT_TRUE(output_match).message(
            "Invalid Output: W=%u H=%u Bpp=%u Misalignment=%u Filter=%s Impl=%s",
            w,
            h,
            bpp,
            misalignment,
            png_filter_names[filter],
            impl_name
          );

          bool guard_match = memcmp(opt_output + png_size, buffer_overrun_guard, buffer_overrun_guard_size) == 0;
          EXPECT_TRUE(guard_match).message(
            "BUFFER OVERRUN: W=%u H=%u Bpp=%u Misalignment=%u Filter=%s Impl=%s",
            w,
            h,
            bpp,
            misalignment,
            png_filter_names[filter],
            impl_name
          );
        }
      }

      free(opt_output_base);
      free(ref_output);
      free(ref_image);
    }
  }
}

UNIT(codec_png_simd_inverse_filter, BL_TEST_GROUP_IMAGE_CODEC_OPS) {
  Ops::FunctionTable reference;
  Ops::init_func_table_ref(reference);

#ifdef BL_BUILD_OPT_SSE2
  if (bl_runtime_has_sse2(&bl_runtime_context)) {
    Ops::FunctionTable optimized;
    init_func_table_sse2(optimized);
    test_simd_impl(reference, optimized, "SSE2");
  }
#endif // BL_BUILD_OPT_SSE2

#ifdef BL_BUILD_OPT_AVX
  if (bl_runtime_has_avx(&bl_runtime_context)) {
    Ops::FunctionTable optimized;
    init_func_table_avx(optimized);
    test_simd_impl(reference, optimized, "AVX");
  }
#endif // BL_BUILD_OPT_AVX

#ifdef BL_BUILD_OPT_ASIMD
  if (bl_runtime_has_asimd(&bl_runtime_context)) {
    Ops::FunctionTable optimized;
    init_func_table_asimd(optimized);
    test_simd_impl(reference, optimized, "ASIMD");
  }
#endif // BL_BUILD_OPT_ASIMD
}

} // {bl::Png::Tests}

#endif // BL_TEST
