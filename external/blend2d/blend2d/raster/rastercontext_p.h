// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_RASTER_RASTERCONTEXT_P_H_INCLUDED
#define BLEND2D_RASTER_RASTERCONTEXT_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/core/compopinfo_p.h>
#include <blend2d/core/context_p.h>
#include <blend2d/core/font_p.h>
#include <blend2d/core/gradient_p.h>
#include <blend2d/core/matrix_p.h>
#include <blend2d/core/path_p.h>
#include <blend2d/core/rgba.h>
#include <blend2d/pipeline/piperuntime_p.h>
#include <blend2d/raster/analyticrasterizer_p.h>
#include <blend2d/raster/edgebuilder_p.h>
#include <blend2d/raster/rasterdefs_p.h>
#include <blend2d/raster/rendercommand_p.h>
#include <blend2d/raster/renderfetchdata_p.h>
#include <blend2d/raster/renderjob_p.h>
#include <blend2d/raster/renderqueue_p.h>
#include <blend2d/raster/rendertargetinfo_p.h>
#include <blend2d/raster/statedata_p.h>
#include <blend2d/raster/styledata_p.h>
#include <blend2d/raster/workdata_p.h>
#include <blend2d/raster/workermanager_p.h>
#include <blend2d/support/arenaallocator_p.h>
#include <blend2d/threading/uniqueidgenerator_p.h>

#if !defined(BL_BUILD_NO_JIT)
  #include <blend2d/pipeline/jit/pipegenruntime_p.h>
#endif

//! \cond INTERNAL
//! \addtogroup blend2d_raster_engine_impl
//! \{

//! Preferred fill-rule (fastest) to use when the fill-rule doesn't matter.
//!
//! Since the filler doesn't care of fill-rule (it always uses the same code-path for non-zero and even-odd fills) it
//! doesn't really matter. However, if there is more rasterizers added in the future this can be adjusted to always
//! select the fastest one.
static constexpr const BLFillRule BL_RASTER_CONTEXT_PREFERRED_FILL_RULE = BL_FILL_RULE_EVEN_ODD;

//! Preferred extend mode (fastest) to use when blitting images. The extend mode can be either PAD or REFLECT as these
//! have the same effect on blits that are bound to the size of the image. We prefer REFLECT, because it's useful also
//! outside regular blits.
static constexpr const BLExtendMode BL_RASTER_CONTEXT_PREFERRED_BLIT_EXTEND = BL_EXTEND_MODE_REFLECT;

//! Minimum size of a path (in vertices) to make it an asynchronous job. The reason for this threshold is that very
//! small paths actually do not benefit from being dispatched into a worker thread (the cost of serializing the job
//! is higher than the cost of processing that path in a user thread).
static constexpr const uint32_t BL_RASTER_CONTEXT_MINIMUM_ASYNC_PATH_SIZE = 10;

//! Maximum size of a text to be copied as is when dispatching asynchronous jobs. When the limit is reached the job
//! serialized would create a BLGlyphBuffer instead of making raw copy of the text, as the glyph-buffer has to copy
//! it anyway.
static constexpr const uint32_t BL_RASTER_CONTEXT_MAXIMUM_EMBEDDED_TEXT_SIZE = 256;

static constexpr const uint32_t BL_RASTER_CONTEXT_DEFAULT_SAVED_STATE_LIMIT = 4096;

static constexpr const uint32_t BL_RASTER_CONTEXT_DEFAULT_COMMAND_QUEUE_LIMIT = 10240;

//! Raster rendering context implementation (software accelerated).
class BLRasterContextImpl : public BLContextImpl {
public:
  BL_NONCOPYABLE(BLRasterContextImpl)

  //! \name Members
  //! \{

  //! Context flags.
  bl::RasterEngine::ContextFlags context_flags;
  //! Rendering mode.
  uint8_t rendering_mode;
  //! Whether worker_mgr has been initialized.
  uint8_t worker_mgr_initialized;
  //! Precision information.
  bl::RasterEngine::RenderTargetInfo render_target_info;

  //! Work data used by synchronous rendering that also holds part of the current state. In async mode the work data
  //! can still be used by user thread in case it's allowed, otherwise it would only hold some states that are used
  //! by the rendering context directly.
  bl::RasterEngine::WorkData sync_work_data;

  //! Pipeline lookup cache (always used before attempting to use `pipe_provider`).
  bl::Pipeline::PipeLookupCache pipe_lookup_cache;

  //! Composition operator simplification that matches the destination format and current `comp_op`.
  const bl::CompOpSimplifyInfo* comp_op_simplify_info;
  //! Solid format table used to select the best pixel format for solid fills.
  uint8_t solid_format_table[BL_RASTER_CONTEXT_SOLID_FORMAT_COUNT];
  //! Table that can be used to override a fill/stroke color by one from SolidId (after a simplification).
  bl::RasterEngine::RenderFetchDataSolid* solid_override_fill_table;
  //! Solid fill override table indexed by \ref bl::CompOpSolidId.
  bl::RasterEngine::RenderFetchDataHeader* solid_fetch_data_override_table[uint32_t(bl::CompOpSolidId::kAlwaysNop) + 1u];

  //! The current state of the rendering context, the `BLContextState` part is public.
  bl::RasterEngine::RasterContextState internal_state;
  //! Link to the previous saved state that will be restored by `BLContext::restore()`.
  bl::RasterEngine::SavedState* saved_state;
  //! An actual shared fill-state (asynchronous rendering).
  bl::RasterEngine::SharedFillState* shared_fill_state;
  //! An actual shared stroke-state (asynchronous rendering).
  bl::RasterEngine::SharedBaseStrokeState* shared_stroke_state;

  //! Arena allocator used to allocate base data structures required by `BLRasterContextImpl`.
  bl::ArenaAllocator base_zone;
  //! Object pool used to allocate `RenderFetchData`.
  bl::ArenaPool<bl::RasterEngine::RenderFetchData> fetch_data_pool;
  //! Object pool used to allocate `SavedState`.
  bl::ArenaPool<bl::RasterEngine::SavedState> saved_state_pool;

  //! Pipeline runtime (either global or isolated, depending on create-options).
  bl::Pipeline::PipeProvider pipe_provider;
  //! Worker manager (only used by asynchronous rendering context).
  bl::Wrap<bl::RasterEngine::WorkerManager> worker_mgr;

  //! Context origin ID used in `data0` member of `BLContextCookie`.
  uint64_t context_origin_id;
  //! Used to generate unique IDs of this context.
  uint64_t state_id_counter;

  //! The number of states that can be saved by `BLContext::save()` call.
  uint32_t saved_state_limit;

  //! Destination image.
  BLImageCore dst_image;
  //! Destination image data.
  BLImageData dst_data;

  //! Minimum safe coordinate for integral transformation (scaled by 256.0 or 65536.0).
  double fp_min_safe_coord_d;
  //! Maximum safe coordinate for integral transformation (scaled by 256.0 or 65536.0).
  double fp_max_safe_coord_d;

  //! Pointers to essential transformations that can be applied to styles.
  const BLMatrix2D* transform_ptrs[BL_CONTEXT_STYLE_TRANSFORM_MODE_MAX_VALUE + 1u];

  //! \}

  //! \name Construction & Destruction
  //! \{

  BL_INLINE BLRasterContextImpl(BLContextVirt* virt_in, void* static_data, size_t static_size) noexcept
    : context_flags(bl::RasterEngine::ContextFlags::kNoFlagsSet),
      rendering_mode(uint8_t(bl::RasterEngine::RenderingMode::kSync)),
      worker_mgr_initialized(false),
      render_target_info {},
      sync_work_data(this, nullptr),
      pipe_lookup_cache{},
      comp_op_simplify_info{},
      solid_format_table{},
      solid_override_fill_table{},
      solid_fetch_data_override_table{},
      saved_state{},
      shared_fill_state{},
      shared_stroke_state{},
      base_zone(8192, 16, static_data, static_size),
      fetch_data_pool(),
      saved_state_pool(),
      pipe_provider(),
      context_origin_id(BLUniqueIdGenerator::generate_id(BLUniqueIdGenerator::Domain::kContext)),
      state_id_counter(0),
      saved_state_limit(0),
      dst_image{},
      dst_data{},
      fp_min_safe_coord_d(0.0),
      fp_max_safe_coord_d(0.0) {

    // Initializes BLRasterContext2DImpl.
    virt = virt_in;
    context_type = BL_CONTEXT_TYPE_RASTER;
    state = &internal_state;
    transform_ptrs[BL_CONTEXT_STYLE_TRANSFORM_MODE_USER] = &internal_state.final_transform;
    transform_ptrs[BL_CONTEXT_STYLE_TRANSFORM_MODE_META] = &internal_state.meta_transform;
    transform_ptrs[BL_CONTEXT_STYLE_TRANSFORM_MODE_NONE] = &bl::TransformInternal::identity_transform;
  }

  BL_INLINE ~BLRasterContextImpl() noexcept {
    destroy_worker_mgr();
  }

  //! \}

  //! \name Memory Management
  //! \{

  BL_INLINE_NODEBUG bl::ArenaAllocator& fetch_data_zone() noexcept { return base_zone; }
  BL_INLINE_NODEBUG bl::ArenaAllocator& saved_state_zone() noexcept { return base_zone; }

  BL_INLINE bl::RasterEngine::RenderFetchData* alloc_fetch_data() noexcept { return fetch_data_pool.alloc(fetch_data_zone()); }
  BL_INLINE void free_fetch_data(bl::RasterEngine::RenderFetchData* fetch_data) noexcept { fetch_data_pool.free(fetch_data); }

  BL_INLINE bl::RasterEngine::SavedState* alloc_saved_state() noexcept { return saved_state_pool.alloc(saved_state_zone()); }
  BL_INLINE void free_saved_state(bl::RasterEngine::SavedState* state) noexcept { saved_state_pool.free(state); }

  BL_INLINE void ensure_worker_mgr() noexcept {
    if (!worker_mgr_initialized) {
      worker_mgr.init();
      worker_mgr_initialized = true;
    }
  }

  BL_INLINE void destroy_worker_mgr() noexcept {
    if (worker_mgr_initialized) {
      worker_mgr.destroy();
      worker_mgr_initialized = false;
    }
  }

  //! \}

  //! \name Context Accessors
  //! \{

  BL_INLINE_NODEBUG bool is_sync() const noexcept { return rendering_mode == uint8_t(bl::RasterEngine::RenderingMode::kSync); }

  BL_INLINE_NODEBUG bl::FormatExt format() const noexcept { return bl::FormatExt(dst_data.format); }
  BL_INLINE_NODEBUG double fp_scale_d() const noexcept { return render_target_info.fp_scale_d; }
  BL_INLINE_NODEBUG double full_alpha_d() const noexcept { return render_target_info.full_alpha_d; }

  BL_INLINE_NODEBUG uint32_t band_count() const noexcept { return sync_work_data.band_count(); }
  BL_INLINE_NODEBUG uint32_t band_height() const noexcept { return sync_work_data.band_height(); }
  BL_INLINE_NODEBUG uint32_t command_quantization_shift_aa() const noexcept { return sync_work_data.command_quantization_shift_aa(); }
  BL_INLINE_NODEBUG uint32_t command_quantization_shift_fp() const noexcept { return sync_work_data.command_quantization_shift_fp(); }

  //! \}

  //! \name State Accessors
  //! \{

  BL_INLINE_NODEBUG uint8_t clip_mode() const noexcept { return sync_work_data.clip_mode; }

  BL_INLINE_NODEBUG uint8_t comp_op() const noexcept { return internal_state.comp_op; }
  BL_INLINE_NODEBUG BLFillRule fill_rule() const noexcept { return BLFillRule(internal_state.fill_rule); }
  BL_INLINE_NODEBUG const BLContextHints& hints() const noexcept { return internal_state.hints; }

  BL_INLINE_NODEBUG const BLStrokeOptions& stroke_options() const noexcept { return internal_state.stroke_options.dcast(); }
  BL_INLINE_NODEBUG const BLApproximationOptions& approximation_options() const noexcept { return internal_state.approximation_options; }

  BL_INLINE_NODEBUG uint32_t global_alpha_i() const noexcept { return internal_state.global_alpha_i; }
  BL_INLINE_NODEBUG double global_alpha_d() const noexcept { return internal_state.global_alpha; }

  BL_INLINE_NODEBUG const bl::RasterEngine::StyleData* get_style(size_t index) const noexcept { return &internal_state.style[index]; }

  BL_INLINE_NODEBUG const BLMatrix2D& meta_transform() const noexcept { return internal_state.meta_transform; }
  BL_INLINE_NODEBUG BLTransformType meta_transform_type() const noexcept { return BLTransformType(internal_state.meta_transform_type); }

  BL_INLINE_NODEBUG const BLMatrix2D& meta_transform_fixed() const noexcept { return internal_state.meta_transform_fixed; }
  BL_INLINE_NODEBUG BLTransformType meta_transform_fixed_type() const noexcept { return BLTransformType(internal_state.meta_transform_fixed_type); }

  BL_INLINE_NODEBUG const BLMatrix2D& user_transform() const noexcept { return internal_state.user_transform; }

  BL_INLINE_NODEBUG const BLMatrix2D& final_transform() const noexcept { return internal_state.final_transform; }
  BL_INLINE_NODEBUG BLTransformType final_transform_type() const noexcept { return BLTransformType(internal_state.final_transform_type); }

  BL_INLINE_NODEBUG const BLMatrix2D& final_transform_fixed() const noexcept { return internal_state.final_transform_fixed; }
  BL_INLINE_NODEBUG BLTransformType final_transform_fixed_type() const noexcept { return BLTransformType(internal_state.final_transform_fixed_type); }

  BL_INLINE_NODEBUG const BLPointI& translation_i() const noexcept { return internal_state.translation_i; }
  BL_INLINE_NODEBUG void set_translation_i(const BLPointI& pt) noexcept { internal_state.translation_i = pt; }

  BL_INLINE_NODEBUG const BLBoxI& meta_clip_box_i() const noexcept { return internal_state.meta_clip_box_i; }
  BL_INLINE_NODEBUG const BLBoxI& final_clip_box_i() const noexcept { return internal_state.final_clip_box_i; }
  BL_INLINE_NODEBUG const BLBox& final_clip_box_d() const noexcept { return internal_state.final_clip_box_d; }

  BL_INLINE_NODEBUG const BLBoxI& final_clip_box_fixed_i() const noexcept { return sync_work_data.edge_builder._clip_box_i; }
  BL_INLINE_NODEBUG const BLBox& final_clip_box_fixed_d() const noexcept { return sync_work_data.edge_builder._clip_box_d; }
  BL_INLINE_NODEBUG void set_final_clip_box_fixed_d(const BLBox& clip_box) { sync_work_data.edge_builder.set_clip_box(clip_box); }

  //! \}

  //! \name Error Accumulation
  //! \{

  BL_INLINE_NODEBUG BLResult accumulate_error(BLResult error) noexcept { return sync_work_data.accumulate_error(error); }

  //! \}
};

BL_HIDDEN BLResult bl_raster_context_init_impl(BLContextCore* self, BLImageCore* image, const BLContextCreateInfo* options) noexcept;
BL_HIDDEN void bl_raster_context_on_init(BLRuntimeContext* rt) noexcept;

//! \}
//! \endcond

#endif // BLEND2D_RASTER_RASTERCONTEXT_P_H_INCLUDED
