// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_PIPELINE_JIT_PIPEFUNCTION_P_H_INCLUDED
#define BLEND2D_PIPELINE_JIT_PIPEFUNCTION_P_H_INCLUDED

#include <blend2d/pipeline/jit/jitbase_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_pipeline_jit
//! \{

namespace bl::Pipeline::JIT {

class PipeCompiler;
class PipePart;

class PipeFunction {
public:
  BL_NONCOPYABLE(PipeFunction)

  //! \name Members
  //! \{

  //! Holds `ctx_data` argument.
  Gp _ctx_data;
  //! Holds `fill_data` argument.
  Gp _fill_data;
  //! Holds `fetch_data` argument.
  Gp _fetch_data;

  //! \}

  //! \name Construction & Destruction
  //! \{

  PipeFunction() noexcept;

  //! \}

  //! \name Accessors
  //! \{

  //! Returns `ctx_data` arguments of the pipeline function.
  BL_INLINE_NODEBUG const Gp& ctx_data() const noexcept { return _ctx_data; }
  //! Returns `fill_data` arguments of the pipeline function.
  BL_INLINE_NODEBUG const Gp& fill_data() const noexcept { return _fill_data; }
  //! Returns `fetch_data` arguments of the pipeline function.
  BL_INLINE_NODEBUG const Gp& fetch_data() const noexcept { return _fetch_data; }

  //! \}

  //! \name Interface
  //! \{

  //! Prepares all parts of the pipeline and configures the pipeline compiler for agreed simd width.
  void prepare(PipeCompiler& pc, PipePart* root) noexcept;

  void begin_function(PipeCompiler& pc) noexcept;
  void end_function(PipeCompiler& pc) noexcept;

  //! \}
};

} // {bl::Pipeline::JIT}

//! \}
//! \endcond

#endif // BLEND2D_PIPELINE_JIT_PIPEFUNCTION_P_H_INCLUDED
