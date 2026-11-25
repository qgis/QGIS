// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

// NOTE: In general this implementation would work when SSSE3 is available, however, we really want to use POPCNT,
// which is available from SSE4.2 (Intel) and SSE4a (AMD). So we require SSE4.2 to make sure both SSSE3 and POPCNT
// are present.

#include <blend2d/core/api-build_p.h>
#ifdef BL_TARGET_OPT_SSE4_2

#include <blend2d/opentype/otglyfsimdimpl_p.h>

namespace bl::OpenType {
namespace GlyfImpl {

BLResult BL_CDECL get_glyph_outlines_sse4_2(
  const BLFontFaceImpl* face_impl,
  BLGlyphId glyph_id,
  const BLMatrix2D* transform,
  BLPath* out,
  size_t* contour_count_out,
  ScopedBuffer* tmp_buffer) noexcept {

  return get_glyph_outlines_simd_impl(face_impl, glyph_id, transform, out, contour_count_out, tmp_buffer);
}

} // {GlyfImpl}
} // {bl::OpenType}

#endif // BL_BUILD_OPT_SSE4_2
