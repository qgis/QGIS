// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_test_p.h>
#if defined(BL_TEST)

#include <blend2d/support/memops_p.h>

// bl::MemOps - Tests
// ==================

namespace bl {
namespace Tests {

UNIT(support_memops, BL_TEST_GROUP_SUPPORT_UTILITIES) {
  INFO("bl::MemOps - Read & Write");
  {
    uint8_t arr[32] = { 0 };

    MemOps::writeU16uBE(arr + 1, 0x0102u);
    MemOps::writeU16uBE(arr + 3, 0x0304u);
    EXPECT_EQ(MemOps::readU32uBE(arr + 1), 0x01020304u);
    EXPECT_EQ(MemOps::readU32uLE(arr + 1), 0x04030201u);
    EXPECT_EQ(MemOps::readU32uBE(arr + 2), 0x02030400u);
    EXPECT_EQ(MemOps::readU32uLE(arr + 2), 0x00040302u);

    MemOps::writeU32uLE(arr + 5, 0x05060708u);
    EXPECT_EQ(MemOps::readU64uBE(arr + 1), 0x0102030408070605u);
    EXPECT_EQ(MemOps::readU64uLE(arr + 1), 0x0506070804030201u);

    MemOps::writeU64uLE(arr + 7, 0x1122334455667788u);
    EXPECT_EQ(MemOps::readU32uBE(arr + 8), 0x77665544u);
  }

  INFO("bl::MemOps - Copy Forward");
  {
    uint8_t data[5] = { 1, 2, 3, 4, 5 };

    MemOps::copy_forward_inline_t(data, data + 1, 4);
    EXPECT_EQ(data[0], 2);
    EXPECT_EQ(data[1], 3);
    EXPECT_EQ(data[2], 4);
    EXPECT_EQ(data[3], 5);
    EXPECT_EQ(data[4], 5);
  }
}

} // {Tests}
} // {bl}

#endif // BL_TEST
