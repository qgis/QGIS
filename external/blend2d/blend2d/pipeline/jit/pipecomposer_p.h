// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_PIPELINE_JIT_PIPECOMPOSER_P_H_INCLUDED
#define BLEND2D_PIPELINE_JIT_PIPECOMPOSER_P_H_INCLUDED

#include <blend2d/pipeline/jit/jitbase_p.h>
#include <blend2d/pipeline/jit/pipeprimitives_p.h>
#include <blend2d/pipeline/jit/pipepart_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_pipeline_jit
//! \{

namespace bl::Pipeline::JIT {

//! Pipeline composer.
//!
//! The purpose of pipeline composer is to create pipeline parts and to compose them together. This functionality
//! was initially part of \ref PipeCompiler, but it was moved out to make pipeline compiler only focus on compiling
//! and composer only focus on combining multiple parts together.
class PipeComposer {
public:
  BL_NONCOPYABLE(PipeComposer)

  //! \name Members
  //! \{

  PipeCompiler& _pc;
  asmjit::Arena& _arena;

  //! \}

  //! \name Construction & Destruction
  //! \{

  explicit PipeComposer(PipeCompiler& pc) noexcept;

  //! \}

  //! \name Pipeline Parts Construction
  //! \{

  template<typename T>
  BL_INLINE T* new_part_t() noexcept {
    void* p = _arena.alloc_oneshot(asmjit::Arena::aligned_size_of<T>());
    if (BL_UNLIKELY(!p))
      return nullptr;
    return new(BLInternal::PlacementNew{p}) T(&_pc);
  }

  template<typename T, typename... Args>
  BL_INLINE T* new_part_t(Args&&... args) noexcept {
    void* p = _arena.alloc_oneshot(asmjit::Arena::aligned_size_of<T>());
    if (BL_UNLIKELY(!p))
      return nullptr;
    return new(BLInternal::PlacementNew{p}) T(&_pc, BLInternal::forward<Args>(args)...);
  }

  FillPart* new_fill_part(FillType fill_type, FetchPart* dst_part, CompOpPart* comp_op_part) noexcept;
  FetchPart* new_fetch_part(FetchType fetch_type, FormatExt format) noexcept;
  CompOpPart* new_comp_op_part(CompOpExt comp_op, FetchPart* dst_part, FetchPart* src_part) noexcept;

  //! \}
};

} // {bl::Pipeline::JIT}

//! \}
//! \endcond

#endif // BLEND2D_PIPELINE_JIT_PIPECOMPOSER_P_H_INCLUDED
