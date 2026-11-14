// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#if !defined(BL_BUILD_NO_JIT)

#include <blend2d/pipeline/jit/fetchpixelptrpart_p.h>
#include <blend2d/pipeline/jit/fetchutilspixelaccess_p.h>
#include <blend2d/pipeline/jit/pipecompiler_p.h>

namespace bl::Pipeline::JIT {

// bl::Pipeline::JIT::FetchPixelPtrPart - Construction & Destruction
// =================================================================

FetchPixelPtrPart::FetchPixelPtrPart(PipeCompiler* pc, FetchType fetch_type, FormatExt format) noexcept
  : FetchPart(pc, fetch_type, format) {

  _part_flags |= PipePartFlags::kMaskedAccess | PipePartFlags::kAdvanceXIsSimple;
  _max_vec_width_supported = kMaxPlatformWidth;
  _max_pixels = kUnlimitedMaxPixels;
}

// bl::Pipeline::JIT::FetchPixelPtrPart - Fetch
// ============================================

void FetchPixelPtrPart::fetch(Pixel& p, PixelCount n, PixelFlags flags, PixelPredicate& predicate) noexcept {
  FetchUtils::fetch_pixels(pc, p, n, flags, fetch_info(), _ptr, _alignment, AdvanceMode::kNoAdvance, predicate);
}

} // {bl::Pipeline::JIT}

#endif // !BL_BUILD_NO_JIT
