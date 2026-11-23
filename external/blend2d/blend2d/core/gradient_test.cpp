// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_test_p.h>
#if defined(BL_TEST)

#include <blend2d/core/gradient_p.h>
#include <blend2d/core/object_p.h>
#include <blend2d/core/rgba_p.h>

namespace bl {
namespace Tests {

// bl::Gradient - Tests
// ====================

UNIT(gradient_allocation_strategy, BL_TEST_GROUP_RENDERING_STYLES) {
  BLGradient g;
  size_t kNumItems = 10000000;
  size_t capacity = g.capacity();

  for (size_t i = 0; i < kNumItems; i++) {
    g.add_stop(double(i) / double(kNumItems), BLRgba32(0xFFFFFFFF));

    if (capacity != g.capacity()) {
      size_t impl_size = GradientInternal::impl_size_from_capacity(g.capacity()).value();
      INFO("Capacity increased from %zu to %zu [ImplSize=%zu]\n", capacity, g.capacity(), impl_size);

      capacity = g.capacity();
    }
  }
}

UNIT(gradient_color_stops, BL_TEST_GROUP_RENDERING_STYLES) {
  INFO("Testing gradient color stops");
  {
    BLGradient g;

    g.add_stop(0.0, BLRgba32(0x00000000u));
    EXPECT_EQ(g.size(), 1u);
    EXPECT_EQ(g.stop_at(0).rgba.value, 0x0000000000000000u);

    g.add_stop(1.0, BLRgba32(0xFF000000u));
    EXPECT_EQ(g.size(), 2u);
    EXPECT_EQ(g.stop_at(1).rgba.value, 0xFFFF000000000000u);

    g.add_stop(0.5, BLRgba32(0xFFFF0000u));
    EXPECT_EQ(g.size(), 3u);
    EXPECT_EQ(g.stop_at(1).rgba.value, 0xFFFFFFFF00000000u);

    g.add_stop(0.5, BLRgba32(0xFFFFFF00u));
    EXPECT_EQ(g.size(), 4u);
    EXPECT_EQ(g.stop_at(2).rgba.value, 0xFFFFFFFFFFFF0000u);

    g.remove_stop_by_offset(0.5, true);
    EXPECT_EQ(g.size(), 2u);
    EXPECT_EQ(g.stop_at(0).rgba.value, 0x0000000000000000u);
    EXPECT_EQ(g.stop_at(1).rgba.value, 0xFFFF000000000000u);

    g.add_stop(0.5, BLRgba32(0x80000000u));
    EXPECT_EQ(g.size(), 3u);
    EXPECT_EQ(g.stop_at(1).rgba.value, 0x8080000000000000u);

    // Check whether copy-on-write works as expected.
    BLGradient copy(g);
    EXPECT_EQ(copy.size(), 3u);

    g.add_stop(0.5, BLRgba32(0xCC000000u));
    EXPECT_EQ(copy.size(), 3u);
    EXPECT_EQ(g.size(), 4u);
    EXPECT_EQ(g.stop_at(0).rgba.value, 0x0000000000000000u);
    EXPECT_EQ(g.stop_at(1).rgba.value, 0x8080000000000000u);
    EXPECT_EQ(g.stop_at(2).rgba.value, 0xCCCC000000000000u);
    EXPECT_EQ(g.stop_at(3).rgba.value, 0xFFFF000000000000u);

    g.reset_stops();
    EXPECT_EQ(g.size(), 0u);
  }
}

UNIT(gradient_values, BL_TEST_GROUP_RENDERING_STYLES) {
  INFO("Testing linear gradient values");
  {
    BLGradient g(BLLinearGradientValues(0.0, 0.5, 1.0, 1.5));

    EXPECT_EQ(g.type(), BL_GRADIENT_TYPE_LINEAR);
    EXPECT_EQ(g.x0(), 0.0);
    EXPECT_EQ(g.y0(), 0.5);
    EXPECT_EQ(g.x1(), 1.0);
    EXPECT_EQ(g.y1(), 1.5);

    g.set_x0(0.15);
    g.set_y0(0.85);
    g.set_x1(0.75);
    g.set_y1(0.25);

    EXPECT_EQ(g.x0(), 0.15);
    EXPECT_EQ(g.y0(), 0.85);
    EXPECT_EQ(g.x1(), 0.75);
    EXPECT_EQ(g.y1(), 0.25);
  }

  INFO("Testing radial gradient values");
  {
    BLGradient g(BLRadialGradientValues(1.0, 1.5, 0.0, 0.5, 500.0));

    EXPECT_EQ(g.type(), BL_GRADIENT_TYPE_RADIAL);
    EXPECT_EQ(g.x0(), 1.0);
    EXPECT_EQ(g.y0(), 1.5);
    EXPECT_EQ(g.x1(), 0.0);
    EXPECT_EQ(g.y1(), 0.5);
    EXPECT_EQ(g.r0(), 500.0);

    g.set_r0(150.0);
    EXPECT_EQ(g.r0(), 150.0);
  }

  INFO("Testing conic gradient values");
  {
    BLGradient g(BLConicGradientValues(1.0, 1.5, 0.1));

    EXPECT_EQ(g.type(), BL_GRADIENT_TYPE_CONIC);
    EXPECT_EQ(g.x0(), 1.0);
    EXPECT_EQ(g.y0(), 1.5);
    EXPECT_EQ(g.angle(), 0.1);
  }
}

} // {Tests}
} // {bl}

#endif // BL_TEST
