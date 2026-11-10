// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_RASTER_WORKERPROC_P_H_INCLUDED
#define BLEND2D_RASTER_WORKERPROC_P_H_INCLUDED

#include <blend2d/threading/thread_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_raster_engine_impl
//! \{

namespace bl::RasterEngine {
namespace WorkerProc {

BL_HIDDEN void process_work_data(WorkData* work_data, RenderBatch* batch) noexcept;
BL_HIDDEN void BL_CDECL worker_thread_entry(BLThread* thread, void* data) noexcept;

} // {WorkerProc}
} // {bl::RasterEngine}

//! \}
//! \endcond

#endif // BLEND2D_RASTER_WORKERPROC_P_H_INCLUDED
