// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_test_p.h>
#if defined(BL_TEST)

#include <blend2d/core/runtime_p.h>
#include <blend2d/simd/simd_p.h>

// SIMD - Tests
// ============

namespace SIMDTests {

// SIMD - Tests - ARM
// ==================

#if BL_TARGET_ARCH_ARM

#if defined(BL_BUILD_OPT_ASIMD)
BL_HIDDEN void simd_test_arm_asimd() noexcept;
#endif // BL_BUILD_OPT_ASIMD

static void simd_test_arm(BLRuntimeContext& rt) noexcept {
#if defined(BL_BUILD_OPT_ASIMD)
  if (bl_runtime_has_asimd(&rt))
    simd_test_arm_asimd();
#endif // BL_BUILD_OPT_ASIMD

  bl_unused(rt);
}
#endif // BL_TARGET_ARCH_ARM

// SIMD - Tests - X86
// ==================

#if BL_TARGET_ARCH_X86

#if defined(BL_BUILD_OPT_SSE2)
BL_HIDDEN void simd_test_x86_sse2() noexcept;
#endif // BL_BUILD_OPT_SSE2

#if defined(BL_BUILD_OPT_SSSE3)
BL_HIDDEN void simd_test_x86_ssse3() noexcept;
#endif // BL_BUILD_OPT_SSSE3

#if defined(BL_BUILD_OPT_SSE4_1)
BL_HIDDEN void simd_test_x86_sse4_1() noexcept;
#endif // BL_BUILD_OPT_SSE4_1

#if defined(BL_BUILD_OPT_SSE4_2)
BL_HIDDEN void simd_test_x86_sse4_2() noexcept;
#endif // BL_BUILD_OPT_SSE4_2

#if defined(BL_BUILD_OPT_AVX)
BL_HIDDEN void simd_test_x86_avx() noexcept;
#endif // BL_BUILD_OPT_AVX

#if defined(BL_BUILD_OPT_AVX2)
BL_HIDDEN void simd_test_x86_avx2() noexcept;
#endif // BL_BUILD_OPT_AVX2

#if defined(BL_BUILD_OPT_AVX512)
BL_HIDDEN void simd_test_x86_avx512() noexcept;
#endif // BL_BUILD_OPT_AVX512

static void simd_test_x86(BLRuntimeContext& rt) noexcept {
#if defined(BL_BUILD_OPT_SSE2)
  if (bl_runtime_has_sse2(&rt))
    simd_test_x86_sse2();
#endif // BL_BUILD_OPT_SSE2

#if defined(BL_BUILD_OPT_SSSE3)
  if (bl_runtime_has_ssse3(&rt))
    simd_test_x86_ssse3();
#endif // BL_BUILD_OPT_SSSE3

#if defined(BL_BUILD_OPT_SSE4_1)
  if (bl_runtime_has_sse4_1(&rt))
    simd_test_x86_sse4_1();
#endif // BL_BUILD_OPT_SSE4_1

#if defined(BL_BUILD_OPT_SSE4_2)
  if (bl_runtime_has_sse4_2(&rt))
    simd_test_x86_sse4_2();
#endif // BL_BUILD_OPT_SSE4_2

#if defined(BL_BUILD_OPT_AVX)
  if (bl_runtime_has_avx(&rt))
    simd_test_x86_avx();
#endif // BL_BUILD_OPT_AVX

#if defined(BL_BUILD_OPT_AVX2)
  if (bl_runtime_has_avx2(&rt))
    simd_test_x86_avx2();
#endif // BL_BUILD_OPT_AVX2

#if defined(BL_BUILD_OPT_AVX512)
  if (bl_runtime_has_avx512(&rt))
    simd_test_x86_avx512();
#endif // BL_BUILD_OPT_AVX512

  bl_unused(rt);
}
#endif // BL_TARGET_ARCH_X86

// SIMD - Tests - Dispatcher
// =========================

UNIT(simd, BL_TEST_GROUP_SIMD) {
#if BL_TARGET_ARCH_ARM
  simd_test_arm(bl_runtime_context);
#endif

#if BL_TARGET_ARCH_X86
  simd_test_x86(bl_runtime_context);
#endif
}

} // {SIMDTests}

#endif // BL_TEST
