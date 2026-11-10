// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_test_p.h>
#if defined(BL_TEST)

#include <blend2d/core/context_p.h>
#include <blend2d/core/gradient_p.h>
#include <blend2d/core/image_p.h>
#include <blend2d/core/pattern_p.h>

// bl::Context - Tests
// ===================

namespace bl {
namespace Tests {

static void test_context_state(BLContext& ctx) {
  // Prepare non-solid styles.
  BLGradient gradient;
  EXPECT_SUCCESS(gradient.create(BLLinearGradientValues(0.0, 0.0, 0.0, 100.0)));
  EXPECT_SUCCESS(gradient.add_stop(0.0, BLRgba32(0x00000000u)));
  EXPECT_SUCCESS(gradient.add_stop(1.0, BLRgba32(0xFF0000FFu)));

  BLImage image(16, 16, BL_FORMAT_PRGB32);
  BLPattern pattern(image);

  INFO("Testing state management of global composition options");
  {
    EXPECT_SUCCESS(ctx.set_comp_op(BL_COMP_OP_SRC_ATOP));
    EXPECT_SUCCESS(ctx.set_global_alpha(0.5));

    EXPECT_EQ(ctx.comp_op(), BL_COMP_OP_SRC_ATOP);
    EXPECT_EQ(ctx.global_alpha(), 0.5);

    EXPECT_SUCCESS(ctx.save());
    EXPECT_SUCCESS(ctx.set_comp_op(BL_COMP_OP_MULTIPLY));
    EXPECT_SUCCESS(ctx.set_global_alpha(0.9));
    EXPECT_EQ(ctx.comp_op(), BL_COMP_OP_MULTIPLY);
    EXPECT_EQ(ctx.global_alpha(), 0.9);
    EXPECT_SUCCESS(ctx.restore());

    EXPECT_EQ(ctx.comp_op(), BL_COMP_OP_SRC_ATOP);
    EXPECT_EQ(ctx.global_alpha(), 0.5);

    EXPECT_SUCCESS(ctx.set_comp_op(BL_COMP_OP_SRC_OVER));
    EXPECT_SUCCESS(ctx.set_global_alpha(1.0));
  }

  INFO("Testing state management of global, fill, and stroke alpha values");
  {
    EXPECT_SUCCESS(ctx.set_global_alpha(0.5));
    EXPECT_SUCCESS(ctx.set_fill_alpha(0.3));
    EXPECT_SUCCESS(ctx.set_stroke_alpha(0.4));

    EXPECT_EQ(ctx.global_alpha(), 0.5);
    EXPECT_EQ(ctx.fill_alpha(), 0.3);
    EXPECT_EQ(ctx.stroke_alpha(), 0.4);

    EXPECT_SUCCESS(ctx.save());

    EXPECT_SUCCESS(ctx.set_global_alpha(1.0));
    EXPECT_SUCCESS(ctx.set_fill_alpha(0.7));
    EXPECT_SUCCESS(ctx.set_stroke_alpha(0.8));

    EXPECT_EQ(ctx.global_alpha(), 1.0);
    EXPECT_EQ(ctx.fill_alpha(), 0.7);
    EXPECT_EQ(ctx.stroke_alpha(), 0.8);

    EXPECT_SUCCESS(ctx.restore());

    EXPECT_EQ(ctx.global_alpha(), 0.5);
    EXPECT_EQ(ctx.fill_alpha(), 0.3);
    EXPECT_EQ(ctx.stroke_alpha(), 0.4);

    EXPECT_SUCCESS(ctx.set_global_alpha(0.1));
    EXPECT_SUCCESS(ctx.set_fill_alpha(1.0));
    EXPECT_SUCCESS(ctx.set_stroke_alpha(0.9));

    EXPECT_EQ(ctx.global_alpha(), 0.1);
    EXPECT_EQ(ctx.fill_alpha(), 1.0);
    EXPECT_EQ(ctx.stroke_alpha(), 0.9);
  }

  INFO("Testing state management of styles (Rgba, Rgba32, Rgba64)");
  {
    BLRgba32 rgba32;
    BLRgba64 rgba64;

    // Initial style.
    BLRgba fill_color(0.1f, 0.2f, 0.3f, 0.4f);
    BLRgba stroke_color(0.5f, 0.6f, 0.7f, 0.8f);

    EXPECT_SUCCESS(ctx.set_fill_style(fill_color));
    EXPECT_SUCCESS(ctx.set_stroke_style(stroke_color));

    BLVar fill_style_var;
    BLVar stroke_style_var;

    EXPECT_SUCCESS(ctx.get_fill_style(fill_style_var));
    EXPECT_SUCCESS(ctx.get_stroke_style(stroke_style_var));

    EXPECT_TRUE(fill_style_var.is_rgba());
    EXPECT_TRUE(stroke_style_var.is_rgba());

    EXPECT_EQ(fill_style_var.as<BLRgba>(), fill_color);
    EXPECT_EQ(stroke_style_var.as<BLRgba>(), stroke_color);

    // Save and change the style.
    {
      EXPECT_SUCCESS(ctx.save());
      BLRgba new_fill_color(0.9f, 0.8f, 0.7f, 0.6f);
      BLRgba new_stroke_color(0.7f, 0.6f, 0.5f, 0.4f);

      EXPECT_SUCCESS(ctx.set_fill_style(new_fill_color));
      EXPECT_SUCCESS(ctx.set_stroke_style(new_stroke_color));

      EXPECT_SUCCESS(ctx.get_fill_style(fill_style_var));
      EXPECT_SUCCESS(ctx.get_stroke_style(stroke_style_var));

      EXPECT_TRUE(fill_style_var.is_rgba());
      EXPECT_TRUE(stroke_style_var.is_rgba());

      EXPECT_EQ(fill_style_var.as<BLRgba>(), new_fill_color);
      EXPECT_EQ(stroke_style_var.as<BLRgba>(), new_stroke_color);

      EXPECT_SUCCESS(ctx.set_fill_style(BLRgba32(0xFFEEDDCCu)));
      EXPECT_SUCCESS(ctx.set_stroke_style(BLRgba64(0x9999AAAABBBBCCCCu)));

      EXPECT_SUCCESS(ctx.get_fill_style(fill_style_var));
      EXPECT_SUCCESS(ctx.get_stroke_style(stroke_style_var));

      EXPECT_TRUE(fill_style_var.is_rgba32());
      EXPECT_TRUE(stroke_style_var.is_rgba64());

      EXPECT_SUCCESS(fill_style_var.to_rgba32(&rgba32));
      EXPECT_SUCCESS(stroke_style_var.to_rgba64(&rgba64));

      EXPECT_EQ(rgba32, BLRgba32(0xFFEEDDCCu));
      EXPECT_EQ(rgba64, BLRgba64(0x9999AAAABBBBCCCCu));
      EXPECT_SUCCESS(ctx.restore());
    }

    // Now we should observe the initial style that was active before save().
    EXPECT_SUCCESS(ctx.get_fill_style(fill_style_var));
    EXPECT_SUCCESS(ctx.get_stroke_style(stroke_style_var));

    EXPECT_TRUE(fill_style_var.is_rgba());
    EXPECT_TRUE(stroke_style_var.is_rgba());

    EXPECT_EQ(fill_style_var.as<BLRgba>(), fill_color);
    EXPECT_EQ(stroke_style_var.as<BLRgba>(), stroke_color);
  }

  INFO("Testing state management of styles (Rgba32, Gradient)");
  {
    BLRgba32 rgba32;

    BLVar fill_style_var;
    BLVar stroke_style_var;

    // Initial style.
    EXPECT_SUCCESS(ctx.set_fill_style(gradient));
    EXPECT_SUCCESS(ctx.set_stroke_style(BLRgba32(0x44332211u)));

    EXPECT_SUCCESS(ctx.get_fill_style(fill_style_var));
    EXPECT_SUCCESS(ctx.get_stroke_style(stroke_style_var));

    EXPECT_TRUE(fill_style_var.is_gradient());
    EXPECT_TRUE(stroke_style_var.is_rgba32());

    EXPECT_EQ(fill_style_var.as<BLGradient>(), gradient);
    EXPECT_SUCCESS(stroke_style_var.to_rgba32(&rgba32));
    EXPECT_EQ(rgba32, BLRgba32(0x44332211u));

    // Save and change the style.
    {
      EXPECT_SUCCESS(ctx.save());
      EXPECT_SUCCESS(ctx.set_fill_style(BLRgba32(0xFFFFFFFFu)));
      EXPECT_SUCCESS(ctx.set_stroke_style(BLRgba32(0x00000000u)));

      EXPECT_SUCCESS(ctx.get_fill_style(fill_style_var));
      EXPECT_SUCCESS(ctx.get_stroke_style(stroke_style_var));

      EXPECT_TRUE(fill_style_var.is_rgba32());
      EXPECT_TRUE(stroke_style_var.is_rgba32());

      EXPECT_SUCCESS(fill_style_var.to_rgba32(&rgba32));
      EXPECT_EQ(rgba32, BLRgba32(0xFFFFFFFFu));

      EXPECT_SUCCESS(stroke_style_var.to_rgba32(&rgba32));
      EXPECT_EQ(rgba32, BLRgba32(0x00000000u));
      EXPECT_SUCCESS(ctx.restore());
    }

    // Now we should observe the initial style that was active before save().
    EXPECT_SUCCESS(ctx.get_fill_style(fill_style_var));
    EXPECT_SUCCESS(ctx.get_stroke_style(stroke_style_var));

    EXPECT_TRUE(fill_style_var.is_gradient());
    EXPECT_TRUE(stroke_style_var.is_rgba32());

    EXPECT_EQ(fill_style_var.as<BLGradient>(), gradient);
    EXPECT_SUCCESS(stroke_style_var.to_rgba32(&rgba32));
    EXPECT_EQ(rgba32, BLRgba32(0x44332211u));
  }

  INFO("Testing fill and stroke style swapping (Rgba32)");
  {
    BLRgba32 a(0xFF000000u);
    BLRgba32 b(0xFFFFFFFFu);

    double alphaA = 0.5;
    double alphaB = 0.7;

    BLVar va;
    BLVar vb;

    BLRgba32 vaRgba32;
    BLRgba32 vbRgba32;

    EXPECT_SUCCESS(ctx.set_fill_style(a));
    EXPECT_SUCCESS(ctx.set_stroke_style(b));

    ctx.set_fill_alpha(alphaA);
    ctx.set_stroke_alpha(alphaB);

    // Swap styles twice - first time to swap, second time to revert the swap.
    for (uint32_t i = 0; i < 2; i++) {
      for (uint32_t j = 0; j < 2; j++) {
        BLContextStyleSwapMode mode = BLContextStyleSwapMode(j);

        EXPECT_SUCCESS(ctx.swap_styles(mode));
        EXPECT_SUCCESS(ctx.get_fill_style(va));
        EXPECT_SUCCESS(ctx.get_stroke_style(vb));

        EXPECT_TRUE(va.is_rgba32());
        EXPECT_TRUE(vb.is_rgba32());
        EXPECT_SUCCESS(va.to_rgba32(&vaRgba32));
        EXPECT_SUCCESS(vb.to_rgba32(&vbRgba32));

        BLInternal::swap(a, b);
        EXPECT_EQ(a, vaRgba32);
        EXPECT_EQ(b, vbRgba32);

        if (mode == BL_CONTEXT_STYLE_SWAP_MODE_STYLES_WITH_ALPHA) {
          BLInternal::swap(alphaA, alphaB);
          EXPECT_EQ(ctx.fill_alpha(), alphaA);
          EXPECT_EQ(ctx.stroke_alpha(), alphaB);
        }
      }
    }
  }

  INFO("Testing fill and stroke style swapping (Gradient, Pattern)");
  {
    BLVar va;
    BLVar vb;

    EXPECT_SUCCESS(ctx.set_fill_style(gradient));
    EXPECT_SUCCESS(ctx.set_stroke_style(pattern));

    // First swap.
    EXPECT_SUCCESS(ctx.swap_styles(BL_CONTEXT_STYLE_SWAP_MODE_STYLES));
    EXPECT_SUCCESS(ctx.get_fill_style(va));
    EXPECT_SUCCESS(ctx.get_stroke_style(vb));

    EXPECT_TRUE(va.is_pattern());
    EXPECT_TRUE(vb.is_gradient());

    EXPECT_EQ(va.as<BLPattern>(), pattern);
    EXPECT_EQ(vb.as<BLGradient>(), gradient);

    // Second swap.
    EXPECT_SUCCESS(ctx.swap_styles(BL_CONTEXT_STYLE_SWAP_MODE_STYLES));
    EXPECT_SUCCESS(ctx.get_fill_style(va));
    EXPECT_SUCCESS(ctx.get_stroke_style(vb));

    EXPECT_TRUE(va.is_gradient());
    EXPECT_TRUE(vb.is_pattern());

    EXPECT_EQ(va.as<BLGradient>(), gradient);
    EXPECT_EQ(vb.as<BLPattern>(), pattern);
  }

  INFO("Testing state management of fill options");
  {
    BLFillRule initial_fill_rule = ctx.fill_rule();

    EXPECT_SUCCESS(ctx.save());

    EXPECT_SUCCESS(ctx.set_fill_rule(BL_FILL_RULE_EVEN_ODD));
    EXPECT_EQ(ctx.fill_rule(), BL_FILL_RULE_EVEN_ODD);

    EXPECT_SUCCESS(ctx.set_fill_rule(BL_FILL_RULE_NON_ZERO));
    EXPECT_EQ(ctx.fill_rule(), BL_FILL_RULE_NON_ZERO);

    EXPECT_SUCCESS(ctx.set_fill_rule(
      initial_fill_rule == BL_FILL_RULE_NON_ZERO
        ? BL_FILL_RULE_EVEN_ODD
        : BL_FILL_RULE_NON_ZERO));
    EXPECT_SUCCESS(ctx.restore());

    EXPECT_EQ(ctx.fill_rule(), initial_fill_rule);
  }

  INFO("Testing state management of stroke options");
  {
    BLArray<double> dashes;
    dashes.append(1.0, 2.0, 3.0, 4.0);

    EXPECT_SUCCESS(ctx.save());

    EXPECT_SUCCESS(ctx.set_stroke_width(2.0));
    EXPECT_EQ(ctx.stroke_width(), 2.0);

    EXPECT_SUCCESS(ctx.set_stroke_miter_limit(10.0));
    EXPECT_EQ(ctx.stroke_miter_limit(), 10.0);

    EXPECT_SUCCESS(ctx.set_stroke_join(BL_STROKE_JOIN_ROUND));
    EXPECT_EQ(ctx.stroke_join(), BL_STROKE_JOIN_ROUND);

    EXPECT_SUCCESS(ctx.set_stroke_start_cap(BL_STROKE_CAP_ROUND_REV));
    EXPECT_EQ(ctx.stroke_start_cap(), BL_STROKE_CAP_ROUND_REV);

    EXPECT_SUCCESS(ctx.set_stroke_end_cap(BL_STROKE_CAP_TRIANGLE_REV));
    EXPECT_EQ(ctx.stroke_end_cap(), BL_STROKE_CAP_TRIANGLE_REV);

    EXPECT_SUCCESS(ctx.set_stroke_dash_array(dashes));
    EXPECT_EQ(ctx.stroke_dash_array(), dashes);

    EXPECT_SUCCESS(ctx.set_stroke_dash_offset(5.0));
    EXPECT_EQ(ctx.stroke_dash_offset(), 5.0);

    BLStrokeOptions opt = ctx.stroke_options();
    EXPECT_EQ(opt, ctx.stroke_options());
    EXPECT_EQ(opt.width, 2.0);
    EXPECT_EQ(opt.miter_limit, 10.0);
    EXPECT_EQ(BLStrokeJoin(opt.join), BL_STROKE_JOIN_ROUND);
    EXPECT_EQ(BLStrokeCap(opt.caps[BL_STROKE_CAP_POSITION_START]), BL_STROKE_CAP_ROUND_REV);
    EXPECT_EQ(BLStrokeCap(opt.caps[BL_STROKE_CAP_POSITION_END]), BL_STROKE_CAP_TRIANGLE_REV);
    EXPECT_EQ(opt.dash_array, dashes);
    EXPECT_EQ(opt.dash_offset, 5.0);

    EXPECT_SUCCESS(ctx.restore());

    EXPECT_SUCCESS(ctx.save());
    EXPECT_SUCCESS(ctx.set_stroke_options(opt));
    EXPECT_EQ(ctx.stroke_width(), 2.0);
    EXPECT_EQ(ctx.stroke_miter_limit(), 10.0);
    EXPECT_EQ(ctx.stroke_join(), BL_STROKE_JOIN_ROUND);
    EXPECT_EQ(ctx.stroke_start_cap(), BL_STROKE_CAP_ROUND_REV);
    EXPECT_EQ(ctx.stroke_end_cap(), BL_STROKE_CAP_TRIANGLE_REV);
    EXPECT_EQ(ctx.stroke_dash_array(), dashes);
    EXPECT_EQ(ctx.stroke_dash_offset(), 5.0);

    BLStrokeOptions opt2 = ctx.stroke_options();
    EXPECT_EQ(opt, opt2);

    EXPECT_SUCCESS(ctx.restore());
  }

  INFO("Testing state management of transformations");
  {
    BLMatrix2D transform = BLMatrix2D::make_scaling(2.0);
    EXPECT_SUCCESS(ctx.apply_transform(transform));
    EXPECT_EQ(ctx.user_transform(), transform);
    EXPECT_EQ(ctx.meta_transform(), BLMatrix2D::make_identity());

    EXPECT_SUCCESS(ctx.save());
    EXPECT_SUCCESS(ctx.user_to_meta());
    EXPECT_EQ(ctx.meta_transform(), transform);
    EXPECT_EQ(ctx.user_transform(), BLMatrix2D::make_identity());
    EXPECT_SUCCESS(ctx.restore());

    EXPECT_EQ(ctx.user_transform(), transform);
    EXPECT_EQ(ctx.meta_transform(), BLMatrix2D::make_identity());

    EXPECT_SUCCESS(ctx.reset_transform());
    EXPECT_EQ(ctx.user_transform(), BLMatrix2D::make_identity());
  }
}

// NOTE: The purpose of these tests is to see whether the render call is clipped properly and that there is no out
// of bounds access or a failed assertion. The tests on CI are run with sanitizers so NaNs, Infs, and other extremely
// high numbers are great to verify whether we are not hitting UB in places where FetchData is initialized.
static void test_context_blit_fill_clip(BLContext& ctx) {
  const double kNaN = Math::nan<double>();
  const double kInf = Math::inf<double>();

  int cw = int(ctx.target_width());
  int ch = int(ctx.target_height());
  int sw = 23;
  int sh = 23;
  BLImage sprite(sw, sh, BL_FORMAT_PRGB32);

  {
    BLContext sctx(sprite);
    sctx.fill_all(BLRgba32(0xFFFFFFFF));
  }

  BLMatrix2D matrix_data[] = {
    BLMatrix2D::make_identity(),
    BLMatrix2D::make_translation(11.3, 11.9),
    BLMatrix2D::make_scaling(100.0, 100.0),
    BLMatrix2D::make_scaling(-100.0, -100.0),
    BLMatrix2D::make_scaling(1, 0.000001),
    BLMatrix2D::make_scaling(0.000001, 1),
    BLMatrix2D::make_scaling(0.000001, 0.000001),
    BLMatrix2D::make_scaling(1e-20, 1e-20),
    BLMatrix2D::make_scaling(1e-100, 1e-100),
  };

  BLPointI pointIData[] = {
    BLPointI(        0,         0),
    BLPointI(        0,        -1),
    BLPointI(       -1,         0),
    BLPointI(       -1,        -1),
    BLPointI(        0,    ch - 1),
    BLPointI(   cw - 1,         0),
    BLPointI(   cw - 1,    ch - 1),
    BLPointI(        0,   -sh + 1),
    BLPointI(  -sw + 1,         0),
    BLPointI(  -sw + 1,   -sh + 1),
    BLPointI(INT32_MIN,         0),
    BLPointI(INT32_MIN,        -1),
    BLPointI(        0, INT32_MIN),
    BLPointI(       -1, INT32_MIN),
    BLPointI(INT32_MIN, INT32_MIN),
    BLPointI(INT32_MAX,         0),
    BLPointI(INT32_MAX,        -1),
    BLPointI(        0, INT32_MAX),
    BLPointI(       -1, INT32_MAX),
    BLPointI(INT32_MAX, INT32_MAX)
  };

  BLPoint pointDData[] = {
    BLPoint(       0.0,       0.0),
    BLPoint(       0.0,       0.3),
    BLPoint(       0.3,       0.0),
    BLPoint(       0.3,       0.3),
    BLPoint(       0.0,       100),
    BLPoint(       100,       0.0),
    BLPoint(       100,       100),
    BLPoint(       0.0,  -sh+1e-1),
    BLPoint(       0.0,  -sh+1e-2),
    BLPoint(       0.0,  -sh+1e-3),
    BLPoint(       0.0,  -sh+1e-4),
    BLPoint(       0.0,  -sh+1e-5),
    BLPoint(       0.0,  -sh+1e-6),
    BLPoint(       0.0,  -sh+1e-7),
    BLPoint(         0,     -sh+1),
    BLPoint(  -sw+1e-1,       0.0),
    BLPoint(  -sw+1e-2,       0.0),
    BLPoint(  -sw+1e-3,       0.0),
    BLPoint(  -sw+1e-4,       0.0),
    BLPoint(  -sw+1e-5,       0.0),
    BLPoint(  -sw+1e-6,       0.0),
    BLPoint(  -sw+1e-7,       0.0),
    BLPoint(     -sw+1,         0),
    BLPoint(         0,    ch-0.1),
    BLPoint(         0,   ch-0.01),
    BLPoint(         0,  ch-0.001),
    BLPoint(         0, ch-0.0001),
    BLPoint(         0,ch-0.00001),
    BLPoint(    cw-0.1,         0),
    BLPoint(   cw-0.01,         0),
    BLPoint(  cw-0.001,         0),
    BLPoint( cw-0.0001,         0),
    BLPoint(cw-0.00001,         0),
    BLPoint(cw-0.00001,ch-0.00001),
    BLPoint(   -1000.0,       0.0),
    BLPoint(-1000000.0,       0.0),
    BLPoint(       0.0,   -1000.0),
    BLPoint(       0.0,-1000000.0),
    BLPoint(   -1000.0,   -1000.0),
    BLPoint(-1000000.0,-1000000.0),
    BLPoint(     -1e50,     -1e50),
    BLPoint(    -1e100,    -1e100),
    BLPoint(    -1e200,    -1e200),
    BLPoint(      1e50,      1e50),
    BLPoint(     1e100,     1e100),
    BLPoint(     1e200,     1e200),
    BLPoint(      kInf,       0.0),
    BLPoint(       0.0,      kInf),
    BLPoint(      kInf,      kInf),
    BLPoint(     -kInf,       0.0),
    BLPoint(       0.0,     -kInf),
    BLPoint(     -kInf,     -kInf),
    BLPoint(      kNaN,       0.0),
    BLPoint(       0.0,      kNaN),
    BLPoint(      kNaN,      kNaN)
  };

  BLSizeI sizeIData[] = {
    BLSizeI(       sw,       sh),
    BLSizeI(   sw / 2,   sh / 2),
    BLSizeI(        1,        1),
    BLSizeI(        0,        0),
    BLSizeI(        0,INT32_MIN),
    BLSizeI(INT32_MIN,        0),
    BLSizeI(INT32_MIN,INT32_MIN),
    BLSizeI(        0,INT32_MAX),
    BLSizeI(INT32_MAX,        0),
    BLSizeI(INT32_MAX,INT32_MAX)
  };

  BLSize sizeDData[] = {
    BLSize(       sw,       sh),
    BLSize(   sw / 2,   sh / 2),
    BLSize(      1.0,      1.0),
    BLSize(      0.0,      0.0),
    BLSize(      0.0,       sh),
    BLSize(       sw,      0.0),
    BLSize(      0.1,      0.1),
    BLSize(  0.00001,       sh),
    BLSize(       sw,  0.00001),
    BLSize(  0.00001,  0.00001),
    BLSize(0.0000001,0.0000001),
    BLSize( -0.00001,       sh),
    BLSize(       sw, -0.00001),
    BLSize( -0.00001, -0.00001),
    BLSize(       sw,     1e40),
    BLSize(       sw,     1e80),
    BLSize(       sw,    1e120),
    BLSize(       sw,    1e160),
    BLSize(       sw,    1e200),
    BLSize(     1e40,       sh),
    BLSize(     1e80,       sh),
    BLSize(    1e120,       sh),
    BLSize(    1e160,       sh),
    BLSize(    1e200,       sh),
    BLSize(     kInf,       sh),
    BLSize(     kInf,       sh),
    BLSize(     kInf,     kInf),
    BLSize(       sw,    -kInf),
    BLSize(    -kInf,       sh),
    BLSize(    -kInf,    -kInf),
    BLSize(       sw,     kNaN),
    BLSize(     kNaN,       sh),
    BLSize(     kNaN,     kNaN)
  };

  INFO("Testing fill clipping");

  ctx.clear_all();

  for (size_t i = 0; i < BL_ARRAY_SIZE(matrix_data); i++) {
    ctx.set_transform(matrix_data[i]);
    ctx.set_fill_style(BLRgba32(0xFFFFFFFF));

    for (size_t j = 0; j < BL_ARRAY_SIZE(pointIData); j++) {
      for (size_t k = 0; k < BL_ARRAY_SIZE(sizeIData); k++) {
        ctx.fill_rect(BLRectI(pointIData[j].x, pointIData[j].y, sizeIData[k].w, sizeIData[k].h));
      }
    }

    for (size_t j = 0; j < BL_ARRAY_SIZE(pointDData); j++) {
      for (size_t k = 0; k < BL_ARRAY_SIZE(sizeDData); k++) {
        ctx.fill_rect(BLRect(pointDData[j].x, pointDData[j].y, sizeDData[k].w, sizeDData[k].h));
      }
    }
  }

  INFO("Testing blit clipping");

  ctx.clear_all();

  for (size_t i = 0; i < BL_ARRAY_SIZE(matrix_data); i++) {
    ctx.set_transform(matrix_data[i]);

    for (size_t j = 0; j < BL_ARRAY_SIZE(pointIData); j++) {
      ctx.blit_image(pointIData[j], sprite);
      for (size_t k = 0; k < BL_ARRAY_SIZE(sizeIData); k++) {
        ctx.blit_image(BLRectI(pointIData[j].x, pointIData[j].y, sizeIData[k].w, sizeIData[k].h), sprite);
      }
    }

    for (size_t j = 0; j < BL_ARRAY_SIZE(pointDData); j++) {
      ctx.blit_image(pointDData[j], sprite);
      for (size_t k = 0; k < BL_ARRAY_SIZE(sizeDData); k++) {
        ctx.blit_image(BLRect(pointDData[j].x, pointDData[j].y, sizeDData[k].w, sizeDData[k].h), sprite);
      }
    }
  }
}

UNIT(context, BL_TEST_GROUP_RENDERING_CONTEXT) {
  BLImage img(256, 256, BL_FORMAT_PRGB32);
  BLContext ctx(img);

  test_context_state(ctx);
  test_context_blit_fill_clip(ctx);
}

} // {Tests}
} // {bl}

#endif // BL_TEST
