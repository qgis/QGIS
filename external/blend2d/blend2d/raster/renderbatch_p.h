// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_RASTER_RENDERBATCH_P_H_INCLUDED
#define BLEND2D_RASTER_RENDERBATCH_P_H_INCLUDED

#include <blend2d/core/image.h>
#include <blend2d/raster/rasterdefs_p.h>
#include <blend2d/raster/renderqueue_p.h>
#include <blend2d/support/arenaallocator_p.h>
#include <blend2d/support/arenalist_p.h>
#include <blend2d/threading/atomic_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_raster_engine_impl
//! \{

namespace bl::RasterEngine {

class WorkerSynchronization;

//! Holds jobs and commands to be dispatched and then consumed by worker threads.
class alignas(BL_CACHE_LINE_SIZE) RenderBatch {
public:
  //! \name Members
  //! \{

  struct alignas(BL_CACHE_LINE_SIZE) {
    //! Job index, incremented by each worker when trying to get the next job.
    //! Can go out of range in case there is no more jobs to process.
    size_t _job_index;

    //! Accumulated errors, initially zero for each batch. Since all workers
    //! would only OR their errors (if happened) at the end we can share the
    //! cache line with `_job_index`.
    uint32_t _accumulated_error_flags;
  };

  //! Contains all jobs of this batch.
  ArenaList<RenderJobQueue> _job_list;
  //! Contains all commands of this batch.
  ArenaList<RenderCommandQueue> _command_list;

  ArenaAllocator::Block* _past_block;

  uint32_t _worker_count;
  uint32_t _job_count;
  uint32_t _command_count;
  uint32_t _band_count;
  uint32_t _state_slot_count;

  //! \}

  //! name Accessors
  //! \{

  BL_INLINE_NODEBUG size_t next_job_index() noexcept { return bl_atomic_fetch_add_strong(&_job_index); }

  BL_INLINE_NODEBUG const ArenaList<RenderJobQueue>& job_list() const noexcept { return _job_list; }
  BL_INLINE_NODEBUG const ArenaList<RenderCommandQueue>& command_list() const noexcept { return _command_list; }

  BL_INLINE_NODEBUG uint32_t worker_count() const noexcept { return _worker_count; }

  BL_INLINE_NODEBUG uint32_t job_count() const noexcept { return _job_count; }
  BL_INLINE_NODEBUG uint32_t command_count() const noexcept { return _command_count; }

  BL_INLINE_NODEBUG uint32_t band_count() const noexcept { return _band_count; }
  BL_INLINE_NODEBUG uint32_t state_slot_count() const noexcept { return _state_slot_count; }

  BL_INLINE void accumulate_error_flags(uint32_t error_flags) noexcept {
    bl_atomic_fetch_or_relaxed(&_accumulated_error_flags, error_flags);
  }

  //! \}
};

} // {bl::RasterEngine}

//! \}
//! \endcond

#endif // BLEND2D_RASTER_RENDERBATCH_P_H_INCLUDED
