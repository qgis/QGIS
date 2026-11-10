// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#if !defined(BL_BUILD_NO_JIT)

#include <blend2d/pipeline/jit/pipecompiler_p.h>
#include <blend2d/pipeline/jit/pipefunction_p.h>
#include <blend2d/pipeline/jit/pipepart_p.h>

namespace bl::Pipeline::JIT {

// bl::Pipeline::PipeFunction - Construction & Destruction
// =======================================================

PipeFunction::PipeFunction() noexcept {}

// bl::Pipeline::PipeFunction - Interface
// ======================================

void PipeFunction::prepare(PipeCompiler& pc, PipePart* root) noexcept {
  // Initialize SIMD width and everything that relies on it.
  VecWidth vw = pc.max_vec_width_from_cpu_features();

  // NOTE 1: It depends on parts which SIMD width will be used by the pipeline. We set the maximum SIMD width
  // available for this host CPU, but if any part doesn't support such width it will end up lower. For example
  // it's possible that the pipeline would use only 128-bit SIMD width even when the CPU has support for AVX-512.

  root->for_each_part([&](PipePart* part) {
    vw = VecWidth(bl_min<uint32_t>(uint32_t(vw), uint32_t(part->max_vec_width_supported())));
  });

  pc.init_vec_width(vw);

  // Prepare all parts (the flag marks all visited parts).
  root->for_each_part_and_mark(PipePartFlags::kPrepareDone, [&](PipePart* part) {
    part->prepare_part();
  });
}

void PipeFunction::begin_function(PipeCompiler& pc) noexcept {
  asmjit::FuncNode* func = pc.add_func(asmjit::FuncSignature::build<void, ContextData*, const void*, const void*>(asmjit::CallConvId::kCDecl));

  pc._func_end = func->end_node()->prev();

  _ctx_data = pc.new_gpz("ctx_data");
  _fill_data = pc.new_gpz("fill_data");
  _fetch_data = pc.new_gpz("fetch_data");

  func->set_arg(0, _ctx_data);
  func->set_arg(1, _fill_data);
  func->set_arg(2, _fetch_data);
}

void PipeFunction::end_function(PipeCompiler& pc) noexcept {
  pc.end_func();
}

} // {bl::Pipeline::JIT}

#endif // !BL_BUILD_NO_JIT