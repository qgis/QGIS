// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_RASTER_RASTERCONTEXTOPS_P_H_INCLUDED
#define BLEND2D_RASTER_RASTERCONTEXTOPS_P_H_INCLUDED

#include <blend2d/core/path_p.h>
#include <blend2d/core/pathstroke_p.h>
#include <blend2d/raster/edgebuilder_p.h>
#include <blend2d/raster/rasterdefs_p.h>
#include <blend2d/raster/workdata_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_raster_engine_impl
//! \{

namespace bl::RasterEngine {

// The purpose of this file is to share as much as possible between both sync and async implementations.

// TODO: [Rendering Context] SIMD idea: shift left by 24 and check whether all bytes are zero.
static BL_INLINE bool isBoxAligned24x8(const BLBoxI& box) noexcept {
  if (BL_TARGET_ARCH_BITS < 64) {
    return ((box.x0 | box.y0 | box.x1 | box.y1) & 0xFF) == 0;
  }
  else {
    // Compilers like this one more than the previous code in 64-bit mode.
    uint64_t a;
    uint64_t b;

    memcpy(&a, &box.x0, sizeof(uint64_t));
    memcpy(&b, &box.x1, sizeof(uint64_t));

    return ((a | b) & 0x000000FF000000FF) == 0;
  }
}

BL_HIDDEN BLResult add_filled_polygon_edges(WorkData* work_data, const BLPointI* pts, size_t size, const BLMatrix2D& transform, BLTransformType transform_type) noexcept;
BL_HIDDEN BLResult add_filled_polygon_edges(WorkData* work_data, const BLPoint* pts, size_t size, const BLMatrix2D& transform, BLTransformType transform_type) noexcept;
BL_HIDDEN BLResult add_filled_path_edges(WorkData* work_data, const BLPathView& path_view, const BLMatrix2D& transform, BLTransformType transform_type) noexcept;

//! Edge builder sink - acts as a base class for other sinks, but can also be used as is, for example
//! by `add_filled_glyph_run_edges()` implementation.
struct EdgeBuilderSink {
  EdgeBuilder<int>* edge_builder;
};

//! Passes the stroked paths to `EdgeBuilder` and flips signs where necessary. This is much better than
//! using BLPath::add_stroked_path() as no reversal of `b` path is necessary, instead we flip sign of such
//! path directly in the EdgeBuilder.
struct StrokeSink : public EdgeBuilderSink {
  const BLMatrix2D* transform;
  BLTransformType transform_type;
};

struct StrokeGlyphRunSink : public StrokeSink {
  BLPath* paths;
  const BLStrokeOptions* stroke_options;
  const BLApproximationOptions* approximation_options;
};

BL_HIDDEN BLResult BL_CDECL fill_glyph_run_sink(BLPathCore* path, const void* info, void* user_data) noexcept;
BL_HIDDEN BLResult BL_CDECL stroke_geometry_sink(BLPathCore* a, BLPathCore* b, BLPathCore* c, size_t figure_start, size_t figure_end, void* user_data) noexcept;
BL_HIDDEN BLResult BL_CDECL stroke_glyph_run_sink(BLPathCore* path, const void* info, void* user_data) noexcept;

template<typename StateAccessor>
static BL_INLINE BLResult add_filled_glyph_run_edges(
  WorkData* work_data,
  const StateAccessor& accessor,
  const BLPoint& origin_fixed, const BLFontCore* font, const BLGlyphRun* glyph_run) noexcept {

  BLMatrix2D transform(accessor.final_transform_fixed(origin_fixed));
  BLPath* path = &work_data->tmp_path[3];
  path->clear();

  EdgeBuilderSink sink;
  sink.edge_builder = &work_data->edge_builder;
  sink.edge_builder->begin();

  BLResult result = bl_font_get_glyph_run_outlines(font, glyph_run, &transform, path, fill_glyph_run_sink, &sink);

  // EdgeBuilder::done() can only fail on out of memory condition.
  if (BL_LIKELY(result == BL_SUCCESS)) {
    result = work_data->edge_builder.done();
    if (BL_LIKELY(result == BL_SUCCESS))
      return result;
  }

  work_data->revert_edge_builder();
  return work_data->accumulate_error(result);
}

template<typename StateAccessor>
static BL_INLINE BLResult add_stroked_path_edges(
  WorkData* work_data,
  const StateAccessor& accessor,
  const BLPoint& origin_fixed, const BLPath* path) noexcept {

  StrokeSink sink;
  BLMatrix2D transform = accessor.final_transform_fixed(origin_fixed);

  sink.edge_builder = &work_data->edge_builder;
  sink.transform = &transform;
  sink.transform_type = accessor.final_transform_fixed_type();

  BLPath* a = &work_data->tmp_path[0];
  BLPath* b = &work_data->tmp_path[1];
  BLPath* c = &work_data->tmp_path[2];

  if (accessor.stroke_options().transform_order != BL_STROKE_TRANSFORM_ORDER_AFTER) {
    a->clear();
    BL_PROPAGATE(a->add_path(*path, accessor.user_transform()));

    path = a;
    a = &work_data->tmp_path[3];

    transform = accessor.meta_transform_fixed(origin_fixed);
    sink.transform_type = accessor.meta_transform_fixed_type();
  }

  a->clear();
  work_data->edge_builder.begin();

  BLResult result = PathInternal::stroke_path(
    path->view(),
    accessor.stroke_options(),
    accessor.approximation_options(),
    *a, *b, *c,
    stroke_geometry_sink, &sink);

  // EdgeBuilder::done() can only fail on out of memory condition.
  if (BL_LIKELY(result == BL_SUCCESS)) {
    result = work_data->edge_builder.done();
    if (BL_LIKELY(result == BL_SUCCESS)) {
      return result;
    }
  }

  work_data->revert_edge_builder();
  return work_data->accumulate_error(result);
}

template<typename StateAccessor>
static BL_INLINE BLResult add_stroked_glyph_run_edges(
  WorkData* work_data,
  const StateAccessor& accessor,
  const BLPoint& origin_fixed, const BLFontCore* font, const BLGlyphRun* glyph_run) noexcept {

  StrokeGlyphRunSink sink;
  sink.edge_builder = &work_data->edge_builder;
  sink.paths = work_data->tmp_path;
  sink.stroke_options = &accessor.stroke_options();
  sink.approximation_options = &accessor.approximation_options();

  BLMatrix2D glyph_run_transform;
  BLMatrix2D final_transform_fixed;

  if (accessor.stroke_options().transform_order == BL_STROKE_TRANSFORM_ORDER_AFTER) {
    glyph_run_transform.reset();
    final_transform_fixed = accessor.final_transform_fixed(origin_fixed);

    sink.transform = &final_transform_fixed;
    sink.transform_type = accessor.final_transform_fixed_type();
  }
  else {
    glyph_run_transform = accessor.user_transform();
    final_transform_fixed = accessor.meta_transform_fixed(origin_fixed);

    sink.transform = &final_transform_fixed;
    sink.transform_type = accessor.meta_transform_fixed_type();
  }

  BLPath* tmp_path = &work_data->tmp_path[3];
  tmp_path->clear();
  work_data->edge_builder.begin();

  // EdgeBuilder::done() can only fail on out of memory condition.
  BLResult result = bl_font_get_glyph_run_outlines(font, glyph_run, &glyph_run_transform, tmp_path, stroke_glyph_run_sink, &sink);
  if (BL_LIKELY(result == BL_SUCCESS)) {
    result = work_data->edge_builder.done();
    if (BL_LIKELY(result == BL_SUCCESS)) {
      return result;
    }
  }

  work_data->revert_edge_builder();
  return work_data->accumulate_error(result);
}

} // {bl::RasterEngine}

//! \}
//! \endcond

#endif // BLEND2D_RASTER_RASTERCONTEXTOPS_P_H_INCLUDED
