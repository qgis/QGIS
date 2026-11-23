// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_test_p.h>
#if defined(BL_TEST)

#include <blend2d/support/ptrops_p.h>

namespace bl {
namespace Tests {

UNIT(support_ptrops, BL_TEST_GROUP_SUPPORT_UTILITIES) {
  INFO("bl::PtrOps - Offset / Deoffset");
  {
    uint32_t array[16] = { 0 };

    EXPECT_EQ(PtrOps::offset(array, 4), array + 1);
    EXPECT_EQ(PtrOps::deoffset(array + 1, 4), array);

    EXPECT_TRUE(PtrOps::both_aligned((void*)(uintptr_t)0x0, (void*)(uintptr_t)0x4, 4));
    EXPECT_FALSE(PtrOps::both_aligned((void*)(uintptr_t)0x1, (void*)(uintptr_t)0x4, 4));
    EXPECT_FALSE(PtrOps::both_aligned((void*)(uintptr_t)0x1, (void*)(uintptr_t)0x5, 4));
    EXPECT_TRUE(PtrOps::both_aligned((void*)(uintptr_t)0x10, (void*)(uintptr_t)0x20, 16));
    EXPECT_FALSE(PtrOps::both_aligned((void*)(uintptr_t)0x1, (void*)(uintptr_t)0x5, 16));

    EXPECT_TRUE(PtrOps::have_equal_alignment((void*)(uintptr_t)0x1, (void*)(uintptr_t)0x5, 4));
    EXPECT_TRUE(PtrOps::have_equal_alignment((void*)(uintptr_t)0x1, (void*)(uintptr_t)0x11, 16));
    EXPECT_FALSE(PtrOps::have_equal_alignment((void*)(uintptr_t)0x1, (void*)(uintptr_t)0x12, 16));
  }
}

} // {Tests}
} // {bl}

#endif // BL_TEST
