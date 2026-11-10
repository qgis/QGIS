// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_test_p.h>
#if defined(BL_TEST)

#include <blend2d/core/matrix_p.h>
#include <blend2d/core/runtime_p.h>
#include <blend2d/simd/simd_p.h>
#include <blend2d/support/math_p.h>

// BLTransform - Tests
// ===================

namespace BLTransformTests {

UNIT(matrix, BL_TEST_GROUP_GEOMETRY_UTILITIES) {
  INFO("Testing matrix types");
  {
    BLMatrix2D m;

    m = BLMatrix2D::make_identity();
    EXPECT_EQ(m.type(), BL_TRANSFORM_TYPE_IDENTITY);

    m = BLMatrix2D::make_translation(1.0, 2.0);
    EXPECT_EQ(m.type(), BL_TRANSFORM_TYPE_TRANSLATE);

    m = BLMatrix2D::make_scaling(2.0, 2.0);
    EXPECT_EQ(m.type(), BL_TRANSFORM_TYPE_SCALE);

    m.m10 = 3.0;
    EXPECT_EQ(m.type(), BL_TRANSFORM_TYPE_AFFINE);

    m.reset(0.0, 1.0, 1.0, 0.0, 0.0, 0.0);
    EXPECT_EQ(m.type(), BL_TRANSFORM_TYPE_SWAP);

    m.reset(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    EXPECT_EQ(m.type(), BL_TRANSFORM_TYPE_INVALID);
  }

  INFO("Testing whether special-case transformations match matrix multiplication");
  {
    enum BL_TEST_MATRIX : uint32_t {
      BL_TEST_MATRIX_IDENTITY,
      BL_TEST_MATRIX_TRANSLATE,
      BL_TEST_MATRIX_SCALE,
      BL_TEST_MATRIX_SKEW,
      BL_TEST_MATRIX_ROTATE,
      BL_TEST_MATRIX_COUNT
    };

    static const BLPoint pt_offset(128.0, 64.0);
    static const BLPoint pt_scale(1.5, 2.0);
    static const BLPoint pt_skew(1.5, 2.0);
    static const double angle = 0.9;

    auto test_matrix_name = [](uint32_t type) noexcept -> const char* {
      switch (type) {
        case BL_TEST_MATRIX_IDENTITY : return "Identity";
        case BL_TEST_MATRIX_TRANSLATE: return "Translate";
        case BL_TEST_MATRIX_SCALE    : return "Scale";
        case BL_TEST_MATRIX_SKEW     : return "Skew";
        case BL_TEST_MATRIX_ROTATE   : return "Rotate";
        default: return "Unknown";
      }
    };

    auto create_test_matrix = [](uint32_t type) noexcept -> BLMatrix2D {
      switch (type) {
        case BL_TEST_MATRIX_TRANSLATE: return BLMatrix2D::make_translation(pt_offset);
        case BL_TEST_MATRIX_SCALE    : return BLMatrix2D::make_scaling(pt_scale);
        case BL_TEST_MATRIX_SKEW     : return BLMatrix2D::make_skewing(pt_skew);
        case BL_TEST_MATRIX_ROTATE   : return BLMatrix2D::make_rotation(angle);

        default:
          return BLMatrix2D::make_identity();
      }
    };

    auto compare = [](const BLMatrix2D& a, const BLMatrix2D& b) noexcept -> bool {
      double diff = bl_max(bl_abs(a.m00 - b.m00),
                           bl_abs(a.m01 - b.m01),
                           bl_abs(a.m10 - b.m10),
                           bl_abs(a.m11 - b.m11),
                           bl_abs(a.m20 - b.m20),
                           bl_abs(a.m21 - b.m21));
      // If Blend2D is compiled with FMA enabled there could be a difference
      // greater than our Math::epsilon<double>, so use a more relaxed value here.
      return diff < 1e-8;
    };

    BLMatrix2D m, n;
    BLMatrix2D a = BLMatrix2D::make_identity();
    BLMatrix2D b;

    for (uint32_t a_type = 0; a_type < BL_TEST_MATRIX_COUNT; a_type++) {
      for (uint32_t b_type = 0; b_type < BL_TEST_MATRIX_COUNT; b_type++) {
        a = create_test_matrix(a_type);
        b = create_test_matrix(b_type);

        m = a;
        n = a;

        for (uint32_t post = 0; post < 2; post++) {
          if (!post)
            m.transform(b);
          else
            m.post_transform(b);

          switch (b_type) {
            case BL_TEST_MATRIX_IDENTITY:
              break;

            case BL_TEST_MATRIX_TRANSLATE:
              if (!post)
                n.translate(pt_offset);
              else
                n.post_translate(pt_offset);
              break;

            case BL_TEST_MATRIX_SCALE:
              if (!post)
                n.scale(pt_scale);
              else
                n.post_scale(pt_scale);
              break;

            case BL_TEST_MATRIX_SKEW:
              if (!post)
                n.skew(pt_skew);
              else
                n.post_skew(pt_skew);
              break;

            case BL_TEST_MATRIX_ROTATE:
              if (!post)
                n.rotate(angle);
              else
                n.post_rotate(angle);
              break;
          }

          if (!compare(m, n)) {
            INFO("Matrices don't match [%s x %s]\n", test_matrix_name(a_type), test_matrix_name(b_type));
            INFO("    [% 3.14f | % 3.14f]      [% 3.14f | % 3.14f]\n", a.m00, a.m01, b.m00, b.m01);
            INFO("  A [% 3.14f | % 3.14f]    B [% 3.14f | % 3.14f]\n", a.m10, a.m11, b.m10, b.m11);
            INFO("    [% 3.14f | % 3.14f]      [% 3.14f | % 3.14f]\n", a.m20, a.m21, b.m20, b.m21);
            INFO("\n");
            INFO("Operation: %s\n", post ? "M = A * B" : "M = B * A");
            INFO("    [% 3.14f | % 3.14f]      [% 3.14f | % 3.14f]\n", m.m00, m.m01, n.m00, n.m01);
            INFO("  M [% 3.14f | % 3.14f] != N [% 3.14f | % 3.14f]\n", m.m10, m.m11, n.m10, n.m11);
            INFO("    [% 3.14f | % 3.14f]      [% 3.14f | % 3.14f]\n", m.m20, m.m21, n.m20, n.m21);
            EXPECT_TRUE(false);
          }
        }
      }
    }
  }
}

} // {BLTransformTests}

#endif // BL_TEST
