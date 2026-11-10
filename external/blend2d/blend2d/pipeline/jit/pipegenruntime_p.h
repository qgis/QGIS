// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_PIPELINE_JIT_PIPEGENRUNTIME_P_H_INCLUDED
#define BLEND2D_PIPELINE_JIT_PIPEGENRUNTIME_P_H_INCLUDED

#include <blend2d/support/arenaallocator_p.h>
#include <blend2d/support/arenahashmap_p.h>
#include <blend2d/support/wrap_p.h>
#include <blend2d/threading/mutex_p.h>
#include <blend2d/pipeline/piperuntime_p.h>
#include <blend2d/pipeline/jit/pipecompiler_p.h>
#include <blend2d/pipeline/jit/pipeprimitives_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_pipeline_jit
//! \{

namespace bl::Pipeline::JIT {

//! PipeGen function cache.
//!
//! \note No locking is preformed implicitly as `BLPipeGenRuntime` synchronizes the access on its own.
class FunctionCache {
public:
  struct FuncNode : public ArenaHashMapNode {
    //! Function pointer.
    void* _func;

    BL_INLINE FuncNode(uint32_t signature, void* func) noexcept
      : ArenaHashMapNode(signature) {
      _custom_data = signature;
      _func = func;
    }

    BL_INLINE void* func() const noexcept { return _func; }
    BL_INLINE uint32_t signature() const noexcept { return _custom_data; }
  };

  struct FuncMatcher {
    uint32_t _signature;

    BL_INLINE explicit FuncMatcher(uint32_t signature) noexcept
      : _signature(signature) {}

    BL_INLINE uint32_t hash_code() const noexcept { return _signature; }
    BL_INLINE bool matches(const FuncNode* node) const noexcept { return node->signature() == _signature; }
  };

  ArenaAllocator _allocator;
  ArenaHashMap<FuncNode> _func_map;

  FunctionCache() noexcept;
  ~FunctionCache() noexcept;

  BL_INLINE void* get(uint32_t signature) const noexcept {
    const FuncNode* node = _func_map.get(FuncMatcher(signature));
    return node ? node->func() : nullptr;
  }

  BLResult put(uint32_t signature, void* func) noexcept;
};

//! JIT pipeline runtime.
class PipeDynamicRuntime : public PipeRuntime {
public:
  //! JIT runtime (stores JIT functions).
  asmjit::JitRuntime _jit_runtime;
  //! Read/write lock used to read/write function cache.
  BLSharedMutex _mutex;
  //! Function cache (caches JIT functions).
  FunctionCache _function_cache;
  //! Count of cached pipelines.
  std::atomic<size_t> _pipeline_count;

  //! CPU features to use (either detected or restricted by the user).
  asmjit::CpuFeatures _cpu_features;
  //! Optimization flags.
  asmjit::CpuHints _cpu_hints;
  //! Maximum pixels at a time, 0 if no limit (debug).
  uint32_t _max_pixels;

  //! Whether to turn on asmjit's logging feature.
  bool _logger_enabled;
  //! Whether to emit correct stack frames to make debugging easier. Disabled by default, because it consumes
  //! one GP register, which is always useful.
  bool _emit_stack_frames;

  explicit PipeDynamicRuntime(PipeRuntimeFlags runtime_flags = PipeRuntimeFlags::kNone) noexcept;
  ~PipeDynamicRuntime() noexcept;

  void _init_cpu_info(const asmjit::CpuInfo& cpu_info) noexcept;

  //! Restricts CPU features not provided in the given mask. This function is only used by isolated runtimes
  //! to setup the runtime. It should never be used after the runtime is in use.
  void _restrict_features(uint32_t mask) noexcept;

  BL_INLINE uint32_t max_pixels() const noexcept { return _max_pixels; }
  BL_INLINE void set_max_pixel_step(uint32_t value) noexcept { _max_pixels = value; }
  BL_INLINE void set_logger_enabled(bool value) noexcept { _logger_enabled = value; }

  FillFunc _compile_fill_func(uint32_t signature) noexcept;

  static Wrap<PipeDynamicRuntime> _global;
};

} // {bl::Pipeline::JIT}

//! \}
//! \endcond

#endif // BLEND2D_PIPELINE_JIT_PIPEGENRUNTIME_P_H_INCLUDED
