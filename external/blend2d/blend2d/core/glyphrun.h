// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_GLYPHRUN_H_INCLUDED
#define BLEND2D_GLYPHRUN_H_INCLUDED

#include <blend2d/core/fontdefs.h>

//! \addtogroup bl_text
//! \{

//! \name BLGlyphRun - Constants
//! \{

//! Flags used by \ref BLGlyphRun.
BL_DEFINE_ENUM(BLGlyphRunFlags) {
  //! No flags.
  BL_GLYPH_RUN_NO_FLAGS = 0u,
  //! Glyph-run contains UCS-4 string and not glyphs (glyph-buffer only).
  BL_GLYPH_RUN_FLAG_UCS4_CONTENT = 0x10000000u,
  //! Glyph-run was created from text that was not a valid unicode.
  BL_GLYPH_RUN_FLAG_INVALID_TEXT = 0x20000000u,
  //! Not the whole text was mapped to glyphs (contains undefined glyphs).
  BL_GLYPH_RUN_FLAG_UNDEFINED_GLYPHS = 0x40000000u,
  //! Encountered invalid font data during text / glyph processing.
  BL_GLYPH_RUN_FLAG_INVALID_FONT_DATA = 0x80000000u

  BL_FORCE_ENUM_UINT32(BL_GLYPH_RUN_FLAG)
};

//! Placement of glyphs stored in a \ref BLGlyphRun.
BL_DEFINE_ENUM(BLGlyphPlacementType) {
  //! No placement (custom handling by \ref BLPathSinkFunc).
  BL_GLYPH_PLACEMENT_TYPE_NONE = 0,
  //! Each glyph has a BLGlyphPlacement (advance + offset).
  BL_GLYPH_PLACEMENT_TYPE_ADVANCE_OFFSET = 1,
  //! Each glyph has a BLPoint offset in design-space units.
  BL_GLYPH_PLACEMENT_TYPE_DESIGN_UNITS = 2,
  //! Each glyph has a BLPoint offset in user-space units.
  BL_GLYPH_PLACEMENT_TYPE_USER_UNITS = 3,
  //! Each glyph has a BLPoint offset in absolute units.
  BL_GLYPH_PLACEMENT_TYPE_ABSOLUTE_UNITS = 4,

  //! Maximum value of `BLGlyphPlacementType`.
  BL_GLYPH_PLACEMENT_TYPE_MAX_VALUE = 4

  BL_FORCE_ENUM_UINT32(BL_GLYPH_PLACEMENT_TYPE)
};

//! \}

//! \name BLGlyphRun - Structs
//! \{

//! BLGlyphRun describes a set of consecutive glyphs and their placements.
//!
//! BLGlyphRun should only be used to pass glyph IDs and their placements to the rendering context. The purpose of
//! BLGlyphRun is to allow rendering glyphs, which could be shaped by various shaping engines (Blend2D, Harfbuzz, etc).
//!
//! BLGlyphRun allows to render glyphs that are stored as uint32_t[] array or part of a bigger structure (for example
//! `hb_glyph_info_t` used by HarfBuzz). Glyph placements at the moment use Blend2D's \ref BLGlyphPlacement or \ref
//! BLPoint, but it's possible to extend the data type in the future.
//!
//! See `BLGlyphRunPlacement` for placement modes provided by Blend2D.
struct BLGlyphRun {
  //! \name Members
  //! \{

  //! Glyph id data (abstract, incremented by `glyph_advance`).
  void* glyph_data;
  //! Glyph placement data (abstract, incremented by `placement_advance`).
  void* placement_data;
  //! Size of the glyph-run in glyph units.
  size_t size;
  //! Reserved for future use, muse be zero.
  uint8_t reserved;
  //! Type of placement, see \ref BLGlyphPlacementType.
  uint8_t placement_type;
  //! Advance of `glyph_data` array.
  int8_t glyph_advance;
  //! Advance of `placement_data` array.
  int8_t placement_advance;
  //! Glyph-run flags.
  uint32_t flags;

  //! \}

#ifdef __cplusplus
  //! \name Common Functionality
  //! \{

  BL_INLINE_NODEBUG void reset() noexcept { *this = BLGlyphRun{}; }

  //! \}

  //! \name Accessors
  //! \{

  BL_INLINE_NODEBUG bool is_empty() const noexcept { return size == 0; }

  template<typename T>
  BL_INLINE_NODEBUG T* glyph_data_as() const noexcept { return static_cast<T*>(glyph_data); }

  template<typename T>
  BL_INLINE_NODEBUG T* placement_data_as() const noexcept { return static_cast<T*>(placement_data); }

  BL_INLINE_NODEBUG void set_glyph_data(const uint32_t* glyph_data) noexcept { set_glyph_data(glyph_data, intptr_t(sizeof(uint32_t))); }

  BL_INLINE_NODEBUG void set_glyph_data(const void* data, intptr_t advance) noexcept {
    this->glyph_data = const_cast<void*>(data);
    this->glyph_advance = int8_t(advance);
  }

  BL_INLINE_NODEBUG void reset_glyph_id_data() noexcept {
    this->glyph_data = nullptr;
    this->glyph_advance = 0;
  }

  template<typename T>
  BL_INLINE_NODEBUG void set_placement_data(const T* data) noexcept {
    set_placement_data(data, sizeof(T));
  }

  BL_INLINE_NODEBUG void set_placement_data(const void* data, intptr_t advance) noexcept {
    this->placement_data = const_cast<void*>(data);
    this->placement_advance = int8_t(advance);
  }

  BL_INLINE_NODEBUG void reset_placement_data() noexcept {
    this->placement_data = nullptr;
    this->placement_advance = 0;
  }

  //! \}
#endif
};

//! \}

//! \name BLGlyphRun - C++ API
//! \{

#ifdef __cplusplus
//! A helper to iterate over a `BLGlyphRun`.
//!
//! Takes into consideration glyph-id advance and glyph-offset advance.
//!
//! Example:
//!
//! ```
//! void inspect_glyph_run(const BLGlyphRun& glyph_run) noexcept {
//!   BLGlyphRunIterator it(glyph_run);
//!   if (it.has_offsets()) {
//!     while (!it.at_end()) {
//!       BLGlyphId glyph_id = it.glyph_id();
//!       BLPoint offset = it.placement();
//!
//!       // Do something with `glyph_id` and `offset`.
//!
//!       it.advance();
//!     }
//!   }
//!   else {
//!     while (!it.at_end()) {
//!       BLGlyphId glyph_id = it.glyph_id();
//!
//!       // Do something with `glyph_id`.
//!
//!       it.advance();
//!     }
//!   }
//! }
//! ```
class BLGlyphRunIterator {
public:
  //! \name Members
  //! \{

  size_t index;
  size_t size;
  void* glyph_data;
  void* placement_data;
  intptr_t glyph_advance;
  intptr_t placement_advance;

  //! \}

  //! \name Construction & Destruction
  //! \{

  BL_INLINE BLGlyphRunIterator() noexcept { reset(); }
  BL_INLINE explicit BLGlyphRunIterator(const BLGlyphRun& glyph_run) noexcept { reset(glyph_run); }

  //! \}

  //! \name Common Functionality
  //! \{

  BL_INLINE void reset() noexcept {
    index = 0;
    size = 0;
    glyph_data = nullptr;
    placement_data = nullptr;
    glyph_advance = 0;
    placement_advance = 0;
  }

  BL_INLINE void reset(const BLGlyphRun& glyph_run) noexcept {
    index = 0;
    size = glyph_run.size;
    glyph_data = glyph_run.glyph_data;
    placement_data = glyph_run.placement_data;
    glyph_advance = glyph_run.glyph_advance;
    placement_advance = glyph_run.placement_advance;
  }

  //! \}

  //! \name Accessors
  //! \{

  BL_INLINE_NODEBUG bool is_empty() const noexcept { return size == 0; }
  BL_INLINE_NODEBUG bool at_end() const noexcept { return index == size; }
  BL_INLINE_NODEBUG bool has_placement() const noexcept { return placement_data != nullptr; }

  BL_INLINE_NODEBUG BLGlyphId glyph_id() const noexcept { return *static_cast<const BLGlyphId*>(glyph_data); }

  template<typename T>
  BL_INLINE_NODEBUG const T& placement() const noexcept { return *static_cast<const T*>(placement_data); }

  //! \}

  //! \name Iterator Interface
  //! \{

  BL_INLINE void advance() noexcept {
    BL_ASSERT(!at_end());

    index++;
    glyph_data = static_cast<uint8_t*>(glyph_data) + glyph_advance;
    placement_data = static_cast<uint8_t*>(placement_data) + placement_advance;
  }

  //! \}
};

#endif
//! \}

//! \}

#endif // BLEND2D_GLYPHRUN_H_INCLUDED
