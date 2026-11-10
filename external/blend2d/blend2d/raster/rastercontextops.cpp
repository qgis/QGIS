// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/path_p.h>
#include <blend2d/core/pathstroke_p.h>
#include <blend2d/raster/edgebuilder_p.h>
#include <blend2d/raster/rastercontextops_p.h>
#include <blend2d/raster/workdata_p.h>

namespace bl::RasterEngine {

// bl::RasterEngine - Edge Building Utilities
// ==========================================

template<typename PointType>
static BL_INLINE BLResult bl_raster_context_build_poly_edges_t(
  WorkData* work_data,
  const PointType* pts, size_t size, const BLMatrix2D& transform, BLTransformType transform_type) noexcept {

  BLResult result = work_data->edge_builder.init_from_poly(pts, size, transform, transform_type);
  if (BL_LIKELY(result == BL_SUCCESS))
    return result;

  work_data->revert_edge_builder();
  return work_data->accumulate_error(result);
}

BLResult add_filled_polygon_edges(WorkData* work_data, const BLPointI* pts, size_t size, const BLMatrix2D& transform, BLTransformType transform_type) noexcept {
  return bl_raster_context_build_poly_edges_t(work_data, pts, size, transform, transform_type);
}

BLResult add_filled_polygon_edges(WorkData* work_data, const BLPoint* pts, size_t size, const BLMatrix2D& transform, BLTransformType transform_type) noexcept {
  return bl_raster_context_build_poly_edges_t(work_data, pts, size, transform, transform_type);
}

BLResult add_filled_path_edges(WorkData* work_data, const BLPathView& path_view, const BLMatrix2D& transform, BLTransformType transform_type) noexcept {
  BLResult result = work_data->edge_builder.init_from_path(path_view, true, transform, transform_type);
  if (BL_LIKELY(result == BL_SUCCESS))
    return result;

  work_data->revert_edge_builder();
  return work_data->accumulate_error(result);
}

// bl::RasterEngine - Sinks & Sink Utilities
// =========================================

BLResult fill_glyph_run_sink(BLPathCore* path, const void* info, void* user_data) noexcept {
  bl_unused(info);

  EdgeBuilderSink* sink = static_cast<EdgeBuilderSink*>(user_data);
  EdgeBuilder<int>* edge_builder = sink->edge_builder;

  BL_PROPAGATE(edge_builder->add_path(path->dcast().view(), true, TransformInternal::identity_transform, BL_TRANSFORM_TYPE_IDENTITY));
  return path->dcast().clear();
}

BLResult stroke_geometry_sink(BLPathCore* a, BLPathCore* b, BLPathCore* c, size_t figure_start, size_t figure_end, void* user_data) noexcept {
  bl_unused(figure_start, figure_end);

  StrokeSink* self = static_cast<StrokeSink*>(user_data);
  EdgeBuilder<int>* edge_builder = self->edge_builder;

  BL_PROPAGATE(edge_builder->add_path(a->dcast().view(), false, *self->transform, self->transform_type));
  BL_PROPAGATE(edge_builder->add_reverse_path_from_stroke_sink(b->dcast().view(), *self->transform, self->transform_type));

  if (!c->dcast().is_empty())
    BL_PROPAGATE(edge_builder->add_path(c->dcast().view(), false, *self->transform, self->transform_type));

  return a->dcast().clear();
}

BLResult stroke_glyph_run_sink(BLPathCore* path, const void* info, void* user_data) noexcept {
  bl_unused(info);

  StrokeGlyphRunSink* sink = static_cast<StrokeGlyphRunSink*>(user_data);
  BLPath& a = sink->paths[0];
  BLPath& b = sink->paths[1];
  BLPath& c = sink->paths[2];

  a.clear();
  BLResult local_result = PathInternal::stroke_path(
    path->dcast().view(),
    *sink->stroke_options,
    *sink->approximation_options,
    a, b, c,
    stroke_geometry_sink, sink);

  // We must clear the input path, because glyph outlines are appended to it and we just just consumed its content.
  // If we haven't cleared it we would process the same data that we have already processed the next time.
  bl_path_clear(path);

  return local_result;
}

} // {bl::RasterEngine}
