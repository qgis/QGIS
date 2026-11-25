// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_test_p.h>
#if defined(BL_TEST)

#include <blend2d/core/rgba_p.h>

// bl::Rgba - Tests
// ================

namespace bl {
namespace Tests {

UNIT(rgba, BL_TEST_GROUP_RENDERING_STYLES) {
  BLRgba32 c32(0x01, 0x02, 0x03, 0xFF);
  BLRgba64 c64(0x100, 0x200, 0x300, 0xFFFF);

  EXPECT_EQ(c32.value, 0xFF010203u);
  EXPECT_EQ(c64.value, 0xFFFF010002000300u);

  EXPECT_EQ(BLRgba64(c32).value, 0xFFFF010102020303u);
  EXPECT_EQ(BLRgba32(c64).value, 0xFF010203u);
}

} // {Tests}
} // {bl}

#endif // BL_TEST
