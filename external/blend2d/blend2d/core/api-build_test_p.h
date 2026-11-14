// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

// This is an internal header file that is always included first by each Blend2D test file.

#ifndef BLEND2D_API_BUILD_TEST_P_H_INCLUDED
#define BLEND2D_API_BUILD_TEST_P_H_INCLUDED

#include <blend2d/core/api-build_p.h>

// bl::Build - Tests
// =================

//! \cond NEVER
// Make sure '#ifdef'ed unit tests are not disabled by IDE.
#if !defined(BL_TEST) && defined(__INTELLISENSE__)
  #define BL_TEST
#endif
//! \endcond

// Include a unit testing package if this is a `bl_test_runner` build.
#if defined(BL_TEST)

#include <blend2d-testing/tests/broken.h>

//! \cond INTERNAL
#define EXPECT_SUCCESS(...) BROKEN_EXPECT_INTERNAL(__FILE__, __LINE__, "EXPECT_SUCCESS(" #__VA_ARGS__ ")", (__VA_ARGS__) == BL_SUCCESS)

//! Blend2D test group.
enum BLTestGroup : int {
  BL_TEST_GROUP_SIMD = 1,
  BL_TEST_GROUP_SUPPORT_UTILITIES,
  BL_TEST_GROUP_SUPPORT_CONTAINERS,
  BL_TEST_GROUP_THREADING,
  BL_TEST_GROUP_CORE_UTILITIES,
  BL_TEST_GROUP_CORE_OBJECT,
  BL_TEST_GROUP_CORE_CONTAINERS,
  BL_TEST_GROUP_COMPRESSION_CHECKSUMS,
  BL_TEST_GROUP_COMPRESSION_ALGORITHM,
  BL_TEST_GROUP_GEOMETRY_UTILITIES,
  BL_TEST_GROUP_GEOMETRY_CONTAINERS,
  BL_TEST_GROUP_IMAGE_CONTAINERS,
  BL_TEST_GROUP_IMAGE_UTILITIES,
  BL_TEST_GROUP_IMAGE_PIXEL_OPS,
  BL_TEST_GROUP_IMAGE_CODEC_OPS,
  BL_TEST_GROUP_IMAGE_CODEC_ROUNDTRIP,
  BL_TEST_GROUP_TEXT_OPENTYPE,
  BL_TEST_GROUP_TEXT_CONTAINERS,
  BL_TEST_GROUP_TEXT_COMBINED,
  BL_TEST_GROUP_RENDERING_UTILITIES,
  BL_TEST_GROUP_RENDERING_STYLES,
  BL_TEST_GROUP_RENDERING_CONTEXT
};
//! \endcond

#endif // BL_TEST

#endif // BLEND2D_API_BUILD_TEST_P_H_INCLUDED
