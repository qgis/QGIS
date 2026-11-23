// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_PIPELINE_JIT_FETCHUTILSINLINELOOPS_P_H_INCLUDED
#define BLEND2D_PIPELINE_JIT_FETCHUTILSINLINELOOPS_P_H_INCLUDED

#include <blend2d/pipeline/jit/pipecompiler_p.h>
#include <blend2d/pipeline/jit/pipeprimitives_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_pipeline_jit
//! \{

namespace bl::Pipeline::JIT::FetchUtils {

// bl::Pipeline::Jit::FetchUtils -- FillSpan & FillRect Loops
// ==========================================================

void inline_fill_span_loop(
  PipeCompiler* pc,
  Gp dst,
  Vec src,
  Gp i,
  uint32_t main_loop_size, uint32_t item_size, uint32_t item_granularity) noexcept;

void inline_fill_rect_loop(
  PipeCompiler* pc,
  Gp dst_ptr,
  Gp stride,
  Gp w,
  Gp h,
  Vec src,
  uint32_t item_size,
  const Label& end) noexcept;

// bl::Pipeline::Jit::FetchUtils -- CopySpan & CopyRect Loops
// ==========================================================

void inline_copy_span_loop(
  PipeCompiler* pc,
  Gp dst,
  Gp src,
  Gp i,
  uint32_t main_loop_size, uint32_t item_size, uint32_t item_granularity, FormatExt format) noexcept;

} // {bl::Pipeline::JIT::FetchUtils}

//! \}
//! \endcond

#endif // BLEND2D_PIPELINE_JIT_FETCHUTILSINLINELOOPS_P_H_INCLUDED
