// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_RASTER_RENDERJOBPROC_P_H_INCLUDED
#define BLEND2D_RASTER_RENDERJOBPROC_P_H_INCLUDED

#include <blend2d/core/path_p.h>
#include <blend2d/core/pathstroke_p.h>
#include <blend2d/raster/edgebuilder_p.h>
#include <blend2d/raster/rendercommand_p.h>
#include <blend2d/raster/rastercontextops_p.h>
#include <blend2d/raster/rasterdefs_p.h>
#include <blend2d/raster/renderjob_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_raster_engine_impl
//! \{

namespace bl::RasterEngine {
namespace JobProc {

// bl::RasterEngine - Job Processor - State Accessor
// =================================================

namespace {

class JobStateAccessor {
public:
  const RenderJob_BaseOp* job;

  BL_INLINE_NODEBUG explicit JobStateAccessor(const RenderJob_BaseOp* job) noexcept
    : job(job) {}

  BL_INLINE_NODEBUG const SharedFillState* fill_state() const noexcept {
    return job->fill_state();
  }

  BL_INLINE const SharedBaseStrokeState* base_stroke_state() const noexcept {
    BL_ASSERT(job->stroke_state());
    return job->stroke_state();
  }

  BL_INLINE const SharedExtendedStrokeState* ext_stroke_state() const noexcept {
    BL_ASSERT(stroke_options().transform_order != BL_STROKE_TRANSFORM_ORDER_AFTER);
    return static_cast<const SharedExtendedStrokeState*>(job->stroke_state());
  }

  // Fill states.
  BL_INLINE_NODEBUG BLTransformType final_transform_fixed_type() const noexcept { return job->final_transform_fixed_type(); }

  BL_INLINE BLMatrix2D final_transform_fixed(const BLPoint& origin_fixed) const noexcept {
    const Matrix2x2& t = fill_state()->final_transform_fixed;
    return BLMatrix2D(t.m[0], t.m[1], t.m[2], t.m[3], origin_fixed.x, origin_fixed.y);
  }

  BL_INLINE_NODEBUG const BLBox& final_clip_box_fixed_d() const noexcept { return fill_state()->final_clip_box_fixed_d; }

  // Stroke states.
  BL_INLINE_NODEBUG const BLApproximationOptions& approximation_options() const noexcept { return base_stroke_state()->approximation_options; }
  BL_INLINE_NODEBUG const BLStrokeOptions& stroke_options() const noexcept { return base_stroke_state()->stroke_options; }

  BL_INLINE_NODEBUG BLTransformType meta_transform_fixed_type() const noexcept { return job->meta_transform_fixed_type(); }

  BL_INLINE BLMatrix2D meta_transform_fixed(const BLPoint& origin_fixed) const noexcept {
    const Matrix2x2& t = ext_stroke_state()->meta_transform_fixed;
    return BLMatrix2D(t.m[0], t.m[1], t.m[2], t.m[3], origin_fixed.x, origin_fixed.y);
  }

  BL_INLINE BLMatrix2D user_transform() const noexcept {
    const Matrix2x2& t = ext_stroke_state()->user_transform;
    return BLMatrix2D(t.m[0], t.m[1], t.m[2], t.m[3], 0.0, 0.0);
  }
};

} // {anonymous}

// bl::RasterEngine - Job Processor - Utilities
// ============================================

static BL_INLINE void prepare_edge_builder(WorkData* work_data, const SharedFillState* fill_state) noexcept {
  work_data->save_state();
  work_data->edge_builder.set_clip_box(fill_state->final_clip_box_fixed_d);
  work_data->edge_builder.set_flatten_tolerance_sq(Math::square(fill_state->toleranceFixedD));
}

static BL_INLINE BLPath* get_geometry_as_path(WorkData* work_data, RenderJob_GeometryOp* job) noexcept {
  BLPath* path = nullptr;
  BLGeometryType geometry_type = job->geometry_type();

  if (geometry_type == BL_GEOMETRY_TYPE_PATH)
    return job->geometry_data<BLPath>();

  path = &work_data->tmp_path[3];
  path->clear();
  BLResult result = path->add_geometry(geometry_type, job->geometry_data<void>());

  if (result != BL_SUCCESS) {
    work_data->accumulate_error(result);
    return nullptr;
  }

  return path;
}

static BL_INLINE void finalize_geometry_data(WorkData* work_data, RenderJob_GeometryOp* job) noexcept {
  bl_unused(work_data);
  if (job->geometry_type() == BL_GEOMETRY_TYPE_PATH)
    job->geometry_data<BLPath>()->~BLPath();
}

template<typename Job>
static BL_INLINE void assign_edges(WorkData* work_data, Job* job, EdgeStorage<int>* edge_storage) noexcept {
  if (!edge_storage->is_empty()) {
    RenderCommandQueue* command_queue = job->command_queue();
    size_t command_index = job->command_index();
    uint8_t qy0 = uint8_t((edge_storage->bounding_box().y0) >> work_data->command_quantization_shift_fp());

    command_queue->initQuantizedY0(command_index, qy0);
    command_queue->at(command_index).set_analytic_edges(edge_storage);
    edge_storage->reset_bounding_box();
  }
}

// bl::RasterEngine - Job Processor - Fill Geometry Job
// ====================================================

static void process_fill_geometry_job(WorkData* work_data, RenderJob_GeometryOp* job) noexcept {
  BLPath* path = get_geometry_as_path(work_data, job);
  if (BL_UNLIKELY(!path))
    return;

  JobStateAccessor accessor(job);
  prepare_edge_builder(work_data, accessor.fill_state());
  BLResult result = add_filled_path_edges(work_data, path->view(), accessor.final_transform_fixed(job->origin_fixed()), accessor.final_transform_fixed_type());

  if (result == BL_SUCCESS) {
    assign_edges(work_data, job, &work_data->edge_storage);
  }

  finalize_geometry_data(work_data, job);
}

// bl::RasterEngine - Job Processor - Fill Text Job
// ================================================

static void process_fill_text_job(WorkData* work_data, RenderJob_TextOp* job) noexcept {
  BLResult result = BL_SUCCESS;
  uint32_t data_type = job->text_data_type();

  const BLFont& font = job->_font.dcast();
  const BLPoint& origin_fixed = job->origin_fixed();
  const BLGlyphRun* glyph_run = nullptr;

  if (data_type != RenderJob::kTextDataGlyphRun) {
    BLGlyphBuffer* glyph_buffer;

    if (data_type != RenderJob::kTextDataGlyphBuffer) {
      glyph_buffer = &work_data->glyph_buffer;
      glyph_buffer->set_text(job->text_data(), job->text_size(), (BLTextEncoding)data_type);
    }
    else {
      glyph_buffer = &job->_glyph_buffer.dcast();
    }

    result = font.shape(*glyph_buffer);
    glyph_run = &glyph_buffer->glyph_run();
  }
  else {
    glyph_run = &job->_glyph_run;
  }

  if (result == BL_SUCCESS) {
    JobStateAccessor accessor(job);
    prepare_edge_builder(work_data, accessor.fill_state());

    result = add_filled_glyph_run_edges(work_data, accessor, origin_fixed, &font, glyph_run);
    if (result == BL_SUCCESS) {
      assign_edges(work_data, job, &work_data->edge_storage);
    }
  }

  job->destroy();
}

// bl::RasterEngine - Job Processor - Stroke Geometry Job
// ======================================================

static void process_stroke_geometry_job(WorkData* work_data, RenderJob_GeometryOp* job) noexcept {
  BLPath* path = get_geometry_as_path(work_data, job);
  if (BL_UNLIKELY(!path))
    return;

  JobStateAccessor accessor(job);
  prepare_edge_builder(work_data, accessor.fill_state());

  if (add_stroked_path_edges(work_data, accessor, job->origin_fixed(), path) == BL_SUCCESS) {
    assign_edges(work_data, job, &work_data->edge_storage);
  }

  finalize_geometry_data(work_data, job);
}

// bl::RasterEngine - Job Processor - Stroke Text Job
// ==================================================

static void process_stroke_text_job(WorkData* work_data, RenderJob_TextOp* job) noexcept {
  BLResult result = BL_SUCCESS;
  uint32_t data_type = job->text_data_type();

  const BLFont& font = job->_font.dcast();
  const BLPoint& origin_fixed = job->origin_fixed();
  const BLGlyphRun* glyph_run = nullptr;

  if (data_type != RenderJob::kTextDataGlyphRun) {
    BLGlyphBuffer* glyph_buffer;

    if (data_type != RenderJob::kTextDataGlyphBuffer) {
      glyph_buffer = &work_data->glyph_buffer;
      glyph_buffer->set_text(job->text_data(), job->text_size(), (BLTextEncoding)data_type);
    }
    else {
      glyph_buffer = &job->_glyph_buffer.dcast();
    }

    result = font.shape(*glyph_buffer);
    glyph_run = &glyph_buffer->glyph_run();
  }
  else {
    glyph_run = &job->_glyph_run;
  }

  if (result == BL_SUCCESS) {
    JobStateAccessor accessor(job);
    prepare_edge_builder(work_data, accessor.fill_state());

    result = add_stroked_glyph_run_edges(work_data, accessor, origin_fixed, &font, glyph_run);
    if (result == BL_SUCCESS) {
      assign_edges(work_data, job, &work_data->edge_storage);
    }
  }

  job->destroy();
}

// bl::RasterEngine - Job Processor - Dispatch
// ===========================================

static void process_job(WorkData* work_data, RenderJob* job) noexcept {
  if (job->has_job_flag(RenderJobFlags::kComputePendingFetchData)) {
    RenderCommand& command = job->command();
    compute_pending_fetch_data(command._source.fetch_data);
  }

  switch (job->job_type()) {
    case RenderJobType::kFillGeometry:
      process_fill_geometry_job(work_data, static_cast<RenderJob_GeometryOp*>(job));
      break;

    case RenderJobType::kFillText:
      process_fill_text_job(work_data, static_cast<RenderJob_TextOp*>(job));
      break;

    case RenderJobType::kStrokeGeometry:
      process_stroke_geometry_job(work_data, static_cast<RenderJob_GeometryOp*>(job));
      break;

    case RenderJobType::kStrokeText:
      process_stroke_text_job(work_data, static_cast<RenderJob_TextOp*>(job));
      break;

    default:
      BL_NOT_REACHED();
  }
}

} // {JobProc}
} // {bl::RasterEngine}

//! \}
//! \endcond

#endif // BLEND2D_RASTER_RENDERJOBPROC_P_H_INCLUDED
