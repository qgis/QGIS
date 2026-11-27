// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_PIXELOPS_FUNCS_P_H_INCLUDED
#define BLEND2D_PIXELOPS_FUNCS_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

namespace bl::PixelOps {

struct Funcs {
  void (BL_CDECL* interpolate_prgb32)(uint32_t* dst, uint32_t dst_size, const BLGradientStop* stops, size_t stop_count) noexcept;
  void (BL_CDECL* interpolate_prgb64)(uint64_t* dst, uint32_t dst_size, const BLGradientStop* stops, size_t stop_count) noexcept;
};

extern Funcs funcs;

namespace Interpolation {

BL_HIDDEN void BL_CDECL interpolate_prgb32(uint32_t* d_ptr, uint32_t d_size, const BLGradientStop* s_ptr, size_t s_size) noexcept;
BL_HIDDEN void BL_CDECL interpolate_prgb64(uint64_t* d_ptr, uint32_t d_size, const BLGradientStop* s_ptr, size_t s_size) noexcept;

#ifdef BL_BUILD_OPT_SSE2
BL_HIDDEN void BL_CDECL interpolate_prgb32_sse2(uint32_t* d_ptr, uint32_t d_size, const BLGradientStop* s_ptr, size_t s_size) noexcept;
#endif

#ifdef BL_BUILD_OPT_AVX2
BL_HIDDEN void BL_CDECL interpolate_prgb32_avx2(uint32_t* d_ptr, uint32_t d_width, const BLGradientStop* s_ptr, size_t s_size) noexcept;
#endif

} // {Interpolation}

} // {bl::PixelOps}

//! \}
//! \endcond

#endif // BLEND2D_PIXELOPS_FUNCS_P_H_INCLUDED
