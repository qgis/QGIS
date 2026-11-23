// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_test_p.h>
#if defined(BL_TEST) && defined(BL_TARGET_OPT_AVX)

#include <blend2d/simd/simd_test_p.h>

// SIMD - Tests - X86 - AVX
// ========================

namespace SIMDTests {

BL_HIDDEN void simd_test_x86_avx() noexcept {
  const char ext[] = "AVX";
  print_cost_matrix(ext);
  test_integer<16>(ext);
}

} // {SIMDTests}

#endif // BL_TEST && BL_TARGET_OPT_AVX
