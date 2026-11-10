// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_PIPELINE_JIT_FETCHPART_P_H_INCLUDED
#define BLEND2D_PIPELINE_JIT_FETCHPART_P_H_INCLUDED

#include <blend2d/pipeline/jit/pipefunction_p.h>
#include <blend2d/pipeline/jit/pipepart_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_pipeline_jit
//! \{

namespace bl::Pipeline::JIT {

//! Pipeline fetch part.
class FetchPart : public PipePart {
public:
  //! \name Constants
  //! \{

  static inline constexpr uint32_t kUnlimitedMaxPixels = 64;

  //! \}

  //! \name Members
  //! \{

  //! Fetch part type.
  FetchType _fetch_type {};
  //! Information about a fetched pixel.
  PixelFetchInfo _fetch_info {};

  //! Pixel type.
  PixelType _pixel_type = PixelType::kNone;
  //! True if the fetching should happen in alpha mode (no RGB).
  uint8_t _alpha_fetch = 0;
  //! Source bytes-per-pixel (only required by pattern fetcher).
  uint8_t _bpp = 0;
  //! Maximum pixel step that the fetcher can fetch at a time (0=unlimited).
  uint8_t _max_pixels = 1;
  //! Pixel granularity passed to init().
  uint8_t _pixel_granularity = 0;

  //! \}

  //! \name Construction & Destruction
  //! \{

  FetchPart(PipeCompiler* pc, FetchType fetch_type, FormatExt format) noexcept;

  //! \}

  //! \name Accessors
  //! \{

  //! Returns the fetch type.
  BL_INLINE_NODEBUG FetchType fetch_type() const noexcept { return _fetch_type; }

  //! Tests whether the fetch-type equals `value`.
  BL_INLINE_NODEBUG bool is_fetch_type(FetchType value) const noexcept { return _fetch_type == value; }
  //! Tests whether the fetch-type is between `first..last`, inclusive.
  BL_INLINE_NODEBUG bool is_fetch_type(FetchType first, FetchType last) const noexcept { return _fetch_type >= first && _fetch_type <= last; }

  //! Tests whether the fetch-type is solid.
  BL_INLINE_NODEBUG bool is_solid() const noexcept { return is_fetch_type(FetchType::kSolid); }
  //! Tests whether the fetch-type is gradient.
  BL_INLINE_NODEBUG bool is_gradient() const noexcept { return is_fetch_type(FetchType::kGradientAnyFirst, FetchType::kGradientAnyLast); }
  //! Tests whether the fetch-type is linear gradient.
  BL_INLINE_NODEBUG bool is_linear_gradient() const noexcept { return is_fetch_type(FetchType::kGradientLinearFirst, FetchType::kGradientLinearLast); }
  //! Tests whether the fetch-type is radial gradient.
  BL_INLINE_NODEBUG bool is_radial_gradient() const noexcept { return is_fetch_type(FetchType::kGradientRadialFirst, FetchType::kGradientRadialLast); }
  //! Tests whether the fetch-type is conic gradient.
  BL_INLINE_NODEBUG bool is_conic_gradient() const noexcept { return is_fetch_type(FetchType::kGradientConicFirst, FetchType::kGradientConicLast); }
  //! Tests whether the fetch-type is pattern.
  BL_INLINE_NODEBUG bool is_pattern() const noexcept { return is_fetch_type(FetchType::kPatternAnyFirst, FetchType::kPatternAnyLast); }
  //! Tests whether the fetch is the destination (special type).
  BL_INLINE_NODEBUG bool is_pixel_ptr() const noexcept { return is_fetch_type(FetchType::kPixelPtr); }

  //! Returns information about a fetched pixel.
  BL_INLINE_NODEBUG PixelFetchInfo fetch_info() const noexcept { return _fetch_info; }
  //! Returns source pixel format.
  BL_INLINE_NODEBUG FormatExt format() const noexcept { return _fetch_info.format(); }
  //! Returns source pixel format information.
  BL_INLINE_NODEBUG BLFormatInfo format_info() const noexcept { return _fetch_info.format_info(); }
  //! Tests whether the fetched pixels contain RGB channels.
  BL_INLINE_NODEBUG bool has_rgb() const noexcept { return _fetch_info.has_rgb(); }
  //! Tests whether the fetched pixels contain Alpha channel.
  BL_INLINE_NODEBUG bool has_alpha() const noexcept { return _fetch_info.has_alpha(); }
  //! Returns source bytes-per-pixel (only used when `is_pattern()` is true).
  BL_INLINE_NODEBUG uint32_t bpp() const noexcept { return _bpp; }

  //! Returns the maximum pixels the fetch part can fetch at a time.
  BL_INLINE_NODEBUG uint32_t max_pixels() const noexcept { return _max_pixels; }

  //! Tests whether the fetching should happen in alpha-only mode.
  BL_INLINE_NODEBUG bool is_alpha_fetch() const noexcept { return _alpha_fetch != 0; }

  //! Returns the pixel granularity passed to `FetchPath::init()`.
  BL_INLINE_NODEBUG uint32_t pixel_granularity() const noexcept { return _pixel_granularity; }

  //! \}

  //! \name Initialization & Finalization
  //! \{

  void init(const PipeFunction& fn, Gp& x, Gp& y, PixelType pixel_type, uint32_t pixel_granularity) noexcept;
  void fini() noexcept;

  virtual void _init_part(const PipeFunction& fn, Gp& x, Gp& y) noexcept;
  virtual void _fini_part() noexcept;

  //! \}

  //! \name Interface
  //! \{

  //! Advances the current y coordinate by one pixel.
  virtual void advance_y() noexcept;

  //! Initializes the current horizontal cursor of the current scanline to `x`.
  //!
  //! \note This initializer is generally called once per scanline to setup the current position by initializing it
  //! to `x`. The position is then advanced automatically by pixel fetchers and by `advance_x()`, which is used when
  //! there is a gap in the scanline that has to be skipped.
  virtual void start_at_x(const Gp& x) noexcept;

  //! Advances the current x coordinate by `diff` pixels. The final x position after advance will be `x`. The fetcher
  //! can decide whether to use `x`, `diff`, or both.
  virtual void advance_x(const Gp& x, const Gp& diff) noexcept;

  //! Called as a prolog before fetching multiple pixels at once. This must be called before any loop that would call
  //! `fetch()` with `n` greater than 1 unless the fetcher is in a vector mode because of `pixel_granularity`.
  virtual void enter_n() noexcept;

  //! Called as an epilog after fetching multiple pixels at once. This must be called after a loop that uses `fetch()`
  //! with `n` greater than 1 unless the fetcher is in a vector mode because of `pixel_granularity`.
  virtual void leave_n() noexcept;

  //! Called before a loop that calls `fetch()` with `n` greater than 1. In some cases there will be some instructions
  //! placed between `prefetch()` and `fetch()`, which means that if the fetcher requires an expensive operation that
  //! has greater latency then it would be better to place that code into the prefetch area.
  virtual void prefetch_n() noexcept;

  //! Cancels the effect of `prefetch_n()` and also automatic prefetch that happens inside `fetch()` with `n` greater than
  //! 1. Must be called after a loop that calls `fetch()` to fetch multiple pixels, or immediately after `prefetch_n()` if
  //! no loop would be entered, but prefetch_n() was already used.
  virtual void postfetch_n() noexcept;

  //! Fetches N pixels to `p` and advances by N.
  virtual void fetch(Pixel& p, PixelCount n, PixelFlags flags, PixelPredicate& predicate) noexcept;

  //! \}
};

} // {bl::Pipeline::JIT}

//! \}
//! \endcond

#endif // BLEND2D_PIPELINE_JIT_FETCHPART_P_H_INCLUDED
