// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_PIPELINE_JIT_FETCHUTILSCOVERAGE_P_H_INCLUDED
#define BLEND2D_PIPELINE_JIT_FETCHUTILSCOVERAGE_P_H_INCLUDED

#include <blend2d/pipeline/jit/pipecompiler_p.h>
#include <blend2d/pipeline/jit/pipeprimitives_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_pipeline_jit
//! \{

namespace bl::Pipeline::JIT {

// bl::Pipeline::JIT - GlobalAlpha
// ===============================

//! Provides a global alpha mask in a format that is requested by during the lifetime of `GlobalAlpha.
//!
//! Can be used by \ref FillPart and \ref CompOpPart as a global alpha abstraction and by othe functions as a global
//! alpha provider.
class GlobalAlpha {
public:
  //! \name Members
  //! \{

  //! Pipeline compiler.
  PipeCompiler* _pc = nullptr;
  //! Node where to emit additional code in case `sm` is not initialized, but required.
  asmjit::BaseNode* _hook = nullptr;

  //! Memory location from which to fetch the mask. This is only used when `init_from_mem()` is used. It's retained
  //! and then when wither \ref sa(), \ref pa(), or \ref ua() is used the respective members would be initialized
  //! on demand.
  Mem _mem {};

  //! Scalar global alpha (only used by scalar alpha-only processing operations that do 1 pixel at a time).
  Gp _sa {};
  //! Packed global alpha vector.
  Vec _pa {};
  //! Unpacked global alpha vector.
  Vec _ua {};

  //! \}

  //! \name Initialization
  //! \{

private:
  void _init_internal(PipeCompiler* pc) noexcept;

public:
  void init_from_mem(PipeCompiler* pc, const Mem& mem) noexcept;
  void init_from_scalar(PipeCompiler* pc, const Gp& sa) noexcept;
  void init_from_packed(PipeCompiler* pc, const Vec& pa) noexcept;
  void init_from_unpacked(PipeCompiler* pc, const Vec& ua) noexcept;

  //! \}

  //! \name Accessors
  //! \{

  //! Returns whether global alpha is initialized and should be applied
  BL_INLINE_NODEBUG bool is_initialized() const noexcept { return _hook != nullptr; }

  //! Returns scalar alpha (in a GP register).
  const Gp& sa() noexcept;
  //! Returns packed alpha (in a SIMD register).
  const Vec& pa() noexcept;
  //! Returns unpacked alpha (in a SIMD register).
  const Vec& ua() noexcept;

  //! \}
};

} // {bl::Pipeline::JIT}

//! \}
//! \endcond

#endif // BLEND2D_PIPELINE_JIT_FETCHUTILSCOVERAGE_P_H_INCLUDED
