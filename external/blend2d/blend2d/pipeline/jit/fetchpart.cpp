// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#if !defined(BL_BUILD_NO_JIT)

#include <blend2d/pipeline/jit/fetchpart_p.h>
#include <blend2d/pipeline/jit/pipecompiler_p.h>

namespace bl::Pipeline::JIT {

// bl::Pipeline::JIT::FetchPart - Construction & Destruction
// =========================================================

FetchPart::FetchPart(PipeCompiler* pc, FetchType fetch_type, FormatExt format) noexcept
  : PipePart(pc, PipePartType::kFetch),
    _fetch_type(fetch_type),
    _fetch_info(format),
    _bpp(uint8_t(bl_format_info[size_t(format)].depth / 8u)) {}

// bl::Pipeline::JIT::FetchPart - Init & Fini
// ==========================================

void FetchPart::init(const PipeFunction& fn, Gp& x, Gp& y, PixelType pixel_type, uint32_t pixel_granularity) noexcept {
  add_part_flags(x.is_valid() ? PipePartFlags::kRectFill : PipePartFlags::kNone);

  _pixel_type = pixel_type;
  _pixel_granularity = uint8_t(pixel_granularity);

  // Initialize alpha fetch information. The fetch would be A8 if either the requested
  // pixel is alpha-only or the source pixel format is alpha-only (or both).
  _alpha_fetch = _pixel_type == PixelType::kA8 || format() == FormatExt::kA8;

  _init_part(fn, x, y);
  _init_global_hook(cc->cursor());
}

void FetchPart::fini() noexcept {
  remove_part_flags(PipePartFlags::kRectFill);

  _fini_part();
  _fini_global_hook();

  _pixel_type = PixelType::kNone;
  _pixel_granularity = 0;
}

void FetchPart::_init_part(const PipeFunction& fn, Gp& x, Gp& y) noexcept {
  bl_unused(fn, x, y);
}

void FetchPart::_fini_part() noexcept {}

// bl::Pipeline::JIT::FetchPart - Advance
// ======================================

// By default these do nothing, only used by `SolidFetch()` this way.
void FetchPart::advance_y() noexcept {
  // Nothing by default.
}

void FetchPart::start_at_x(const Gp& x) noexcept {
  // Nothing by default.
  bl_unused(x);
}

void FetchPart::advance_x(const Gp& x, const Gp& diff) noexcept {
  // Nothing by default.
  bl_unused(x, diff);
}

// bl::Pipeline::JIT::FetchPart - Fetch
// ====================================

void FetchPart::enter_n() noexcept {
  // Nothing by default.
}

void FetchPart::leave_n() noexcept {
  // Nothing by default.
}

void FetchPart::prefetch_n() noexcept {
  // Nothing by default.
}

void FetchPart::postfetch_n() noexcept {
  // Nothing by default.
}

// [[pure virtual]]
void FetchPart::fetch(Pixel& p, PixelCount n, PixelFlags flags, PixelPredicate& predicate) noexcept {
  bl_unused(p, n, flags, predicate);
  BL_NOT_REACHED();
}

} // {bl::Pipeline::JIT}

#endif // !BL_BUILD_NO_JIT
