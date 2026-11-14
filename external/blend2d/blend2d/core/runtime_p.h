// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_RUNTIME_P_H_INCLUDED
#define BLEND2D_RUNTIME_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/core/runtime.h>
#include <blend2d/threading/atomic_p.h>

#include <atomic>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

//! Fixed array used by Runtime handlers, initial content should be zero initialized by the linker as it's used only
//! in a statically allocated `BLRuntimeContext`.
template<typename Func, size_t N>
struct BLRuntimeHandlers {
  size_t size;
  Func data[N];

  BL_INLINE void reset() noexcept { size = 0; }

  BL_INLINE void add(Func func) noexcept {
    BL_ASSERT(size < N);
    data[size++] = func;
  }

  template<typename... Args>
  BL_INLINE void call(Args&&... args) noexcept {
    for (size_t i = 0; i < size; i++)
      data[i](BLInternal::forward<Args>(args)...);
  }

  template<typename... Args>
  BL_INLINE void call_in_reverse_order(Args&&... args) noexcept {
    size_t i = size;
    while (i)
      data[--i](BLInternal::forward<Args>(args)...);
  }
};

enum BLRuntimeCpuVendor : uint32_t {
  BL_RUNTIME_CPU_VENDOR_UNKNOWN = 0,
  BL_RUNTIME_CPU_VENDOR_AMD     = 1,
  BL_RUNTIME_CPU_VENDOR_INTEL   = 2,
  BL_RUNTIME_CPU_VENDOR_VIA     = 3
};

enum BLRuntimeCpuHints : uint32_t {
  BL_RUNTIME_CPU_HINT_FAST_AVX256 = 0x00000001u,
  BL_RUNTIME_CPU_HINT_FAST_PSHUFB = 0x00000010u,
  BL_RUNTIME_CPU_HINT_FAST_PMULLD = 0x00000020u
};

struct BLRuntimeOptimizationInfo {
  uint32_t cpu_vendor;
  uint32_t cpu_hints;

  BL_INLINE bool has_cpu_hint(uint32_t hint) const noexcept { return (cpu_hints & hint) != 0; }
  BL_INLINE bool hasFastAvx256() const noexcept { return has_cpu_hint(BL_RUNTIME_CPU_HINT_FAST_AVX256); }
  BL_INLINE bool has_fast_pshufb() const noexcept { return has_cpu_hint(BL_RUNTIME_CPU_HINT_FAST_PSHUFB); }
  BL_INLINE bool has_fast_pmulld() const noexcept { return has_cpu_hint(BL_RUNTIME_CPU_HINT_FAST_PMULLD); }
};

struct BLRuntimeFeaturesInfo {
  uint32_t futex_enabled;
};

//! Blend2D runtime context.
//!
//! A singleton that is created at Blend2D startup and that can be used to query various information about the
//! library and its runtime.
struct BLRuntimeContext {
  //! Shutdown handler.
  typedef void (BL_CDECL* ShutdownFunc)(BLRuntimeContext* rt) noexcept;
  //! Cleanup handler.
  typedef void (BL_CDECL* CleanupFunc)(BLRuntimeContext* rt, BLRuntimeCleanupFlags cleanup_flags) noexcept;
  //! MemoryInfo handler.
  typedef void (BL_CDECL* ResourceInfoFunc)(BLRuntimeContext* rt, BLRuntimeResourceInfo* resource_info) noexcept;

  //! Counts how many times `bl_runtime_init()` has been called.
  //!
  //! Returns the current initialization count, which is incremented every time a `bl_runtime_init()` is called and
  //! decremented every time a `bl_runtime_shutdown()` is called.
  //!
  //! When this counter is incremented from 0 to 1 the library is initialized, when it's decremented to zero it
  //! will free all resources and it will no longer be safe to use.
  size_t ref_count;

  //! System information.
  BLRuntimeSystemInfo system_info;

  //! Optimization information.
  BLRuntimeOptimizationInfo optimization_info;

  //! Extended features information.
  BLRuntimeFeaturesInfo features_info;

  // NOTE: There is only a limited number of handlers that can be added to the context. The reason we do it this way
  // is that for builds of Blend2D that have conditionally disabled some features it's easier to have only `OnInit()`
  // handlers and let them register cleanup/shutdown handlers when needed.

  //! Shutdown handlers (always traversed from last to first).
  BLRuntimeHandlers<ShutdownFunc, 8> shutdown_handlers;
  //! Cleanup handlers (always executed from first to last).
  BLRuntimeHandlers<CleanupFunc, 8> cleanup_handlers;
  //! MemoryInfo handlers (always traversed from first to last).
  BLRuntimeHandlers<ResourceInfoFunc, 8> resource_info_handlers;
};

//! Instance of a global runtime context.
BL_HIDDEN extern BLRuntimeContext bl_runtime_context;

// NOTE: Must be in anonymous namespace. When the compilation unit uses certain optimizations we constexpr the
// check and return `true` without checking CPU features as the compilation unit uses them anyway [at that point].
BL_DIAGNOSTIC_PUSH(BL_DIAGNOSTIC_NO_UNUSED_PARAMETERS)

//! Returns true if the target architecture is 32-bit.
static constexpr bool bl_runtime_is_32bit() noexcept { return BL_TARGET_ARCH_BITS < 64; }

namespace {

#if defined(BL_TARGET_OPT_SSE2)
constexpr bool bl_runtime_has_sse2(BLRuntimeContext* rt) noexcept { return true; }
#else
BL_INLINE bool bl_runtime_has_sse2(BLRuntimeContext* rt) noexcept { return (rt->system_info.cpu_features & BL_RUNTIME_CPU_FEATURE_X86_SSE2) != 0; }
#endif

#if defined(BL_TARGET_OPT_SSE3)
constexpr bool bl_runtime_has_sse3(BLRuntimeContext* rt) noexcept { return true; }
#else
BL_INLINE bool bl_runtime_has_sse3(BLRuntimeContext* rt) noexcept { return (rt->system_info.cpu_features & BL_RUNTIME_CPU_FEATURE_X86_SSE3) != 0; }
#endif

#if defined(BL_TARGET_OPT_SSSE3)
constexpr bool bl_runtime_has_ssse3(BLRuntimeContext* rt) noexcept { return true; }
#else
BL_INLINE bool bl_runtime_has_ssse3(BLRuntimeContext* rt) noexcept { return (rt->system_info.cpu_features & BL_RUNTIME_CPU_FEATURE_X86_SSSE3) != 0; }
#endif

#if defined(BL_TARGET_OPT_SSE4_1)
constexpr bool bl_runtime_has_sse4_1(BLRuntimeContext* rt) noexcept { return true; }
#else
BL_INLINE bool bl_runtime_has_sse4_1(BLRuntimeContext* rt) noexcept { return (rt->system_info.cpu_features & BL_RUNTIME_CPU_FEATURE_X86_SSE4_1) != 0; }
#endif

#if defined(BL_TARGET_OPT_SSE4_2)
constexpr bool bl_runtime_has_sse4_2(BLRuntimeContext* rt) noexcept { return true; }
#else
BL_INLINE bool bl_runtime_has_sse4_2(BLRuntimeContext* rt) noexcept { return (rt->system_info.cpu_features & BL_RUNTIME_CPU_FEATURE_X86_SSE4_2) != 0; }
#endif

#if defined(BL_TARGET_OPT_AVX)
constexpr bool bl_runtime_has_avx(BLRuntimeContext* rt) noexcept { return true; }
#else
BL_INLINE bool bl_runtime_has_avx(BLRuntimeContext* rt) noexcept { return (rt->system_info.cpu_features & BL_RUNTIME_CPU_FEATURE_X86_AVX) != 0; }
#endif

#if defined(BL_TARGET_OPT_AVX2)
constexpr bool bl_runtime_has_avx2(BLRuntimeContext* rt) noexcept { return true; }
#else
BL_INLINE bool bl_runtime_has_avx2(BLRuntimeContext* rt) noexcept { return (rt->system_info.cpu_features & BL_RUNTIME_CPU_FEATURE_X86_AVX2) != 0; }
#endif

#if defined(BL_TARGET_OPT_AVX512)
constexpr bool bl_runtime_has_avx512(BLRuntimeContext* rt) noexcept { return true; }
#else
BL_INLINE bool bl_runtime_has_avx512(BLRuntimeContext* rt) noexcept { return (rt->system_info.cpu_features & BL_RUNTIME_CPU_FEATURE_X86_AVX512) != 0; }
#endif

#if defined(BL_TARGET_OPT_ASIMD)
constexpr bool bl_runtime_has_asimd(BLRuntimeContext* rt) noexcept { return true; }
#else
constexpr bool bl_runtime_has_asimd(BLRuntimeContext* rt) noexcept { return (rt->system_info.cpu_features & BL_RUNTIME_CPU_FEATURE_ARM_ASIMD) != 0; }
#endif

#if defined(BL_TARGET_OPT_ASIMD_CRYPTO)
constexpr bool bl_runtime_has_crc32(BLRuntimeContext* rt) noexcept { return true; }
#else
constexpr bool bl_runtime_has_crc32(BLRuntimeContext* rt) noexcept { return (rt->system_info.cpu_features & BL_RUNTIME_CPU_FEATURE_ARM_CRC32) != 0; }
#endif

#if defined(BL_TARGET_OPT_ASIMD_CRYPTO)
constexpr bool bl_runtime_has_pmull(BLRuntimeContext* rt) noexcept { return true; }
#else
constexpr bool bl_runtime_has_pmull(BLRuntimeContext* rt) noexcept { return (rt->system_info.cpu_features & BL_RUNTIME_CPU_FEATURE_ARM_PMULL) != 0; }
#endif

} // {anonymous}

BL_DIAGNOSTIC_POP

[[noreturn]]
BL_HIDDEN void bl_runtime_failure(const char* fmt, ...) noexcept;

BL_HIDDEN void bl_fuxex_rt_init(BLRuntimeContext* rt) noexcept;
BL_HIDDEN void bl_thread_rt_init(BLRuntimeContext* rt) noexcept;
BL_HIDDEN void bl_thread_pool_rt_init(BLRuntimeContext* rt) noexcept;
BL_HIDDEN void bl_zero_allocator_rt_init(BLRuntimeContext* rt) noexcept;

BL_HIDDEN void bl_compression_rt_init(BLRuntimeContext* rt) noexcept;
BL_HIDDEN void bl_pixel_ops_rt_init(BLRuntimeContext* rt) noexcept;
BL_HIDDEN void bl_bit_array_rt_init(BLRuntimeContext* rt) noexcept;
BL_HIDDEN void bl_bit_set_rt_init(BLRuntimeContext* rt) noexcept;
BL_HIDDEN void bl_array_rt_init(BLRuntimeContext* rt) noexcept;
BL_HIDDEN void bl_string_rt_init(BLRuntimeContext* rt) noexcept;
BL_HIDDEN void bl_transform_rt_init(BLRuntimeContext* rt) noexcept;
BL_HIDDEN void bl_path_rt_init(BLRuntimeContext* rt) noexcept;
BL_HIDDEN void bl_image_rt_init(BLRuntimeContext* rt) noexcept;
BL_HIDDEN void bl_image_codec_rt_init(BLRuntimeContext* rt) noexcept;
BL_HIDDEN void bl_image_decoder_rt_init(BLRuntimeContext* rt) noexcept;
BL_HIDDEN void bl_image_encoder_rt_init(BLRuntimeContext* rt) noexcept;
BL_HIDDEN void bl_image_scale_rt_init(BLRuntimeContext* rt) noexcept;
BL_HIDDEN void bl_pattern_rt_init(BLRuntimeContext* rt) noexcept;
BL_HIDDEN void bl_gradient_rt_init(BLRuntimeContext* rt) noexcept;
BL_HIDDEN void bl_font_feature_settings_rt_init(BLRuntimeContext* rt) noexcept;
BL_HIDDEN void bl_font_variation_settings_rt_init(BLRuntimeContext* rt) noexcept;
BL_HIDDEN void bl_font_data_rt_init(BLRuntimeContext* rt) noexcept;
BL_HIDDEN void bl_font_face_rt_init(BLRuntimeContext* rt) noexcept;
BL_HIDDEN void bl_open_type_rt_init(BLRuntimeContext* rt) noexcept;
BL_HIDDEN void bl_font_rt_init(BLRuntimeContext* rt) noexcept;
BL_HIDDEN void bl_font_manager_rt_init(BLRuntimeContext* rt) noexcept;
BL_HIDDEN void bl_context_rt_init(BLRuntimeContext* rt) noexcept;
BL_HIDDEN void bl_static_pipeline_rt_init(BLRuntimeContext* rt) noexcept;

#if !defined(BL_BUILD_NO_JIT)
BL_HIDDEN void bl_dynamic_pipeline_rt_init(BLRuntimeContext* rt) noexcept;
#endif

BL_HIDDEN void bl_register_built_in_codecs(BLRuntimeContext* rt) noexcept;

//! \}
//! \endcond

#endif // BLEND2D_RUNTIME_P_H_INCLUDED
