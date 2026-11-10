// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_FORMAT_H_INCLUDED
#define BLEND2D_FORMAT_H_INCLUDED

#include <blend2d/core/api.h>

//! \addtogroup bl_imaging
//! \{

//! \name BLFormat - Constants
//! \{

//! Pixel format.
//!
//! Compatibility Table
//! -------------------
//!
//! ```
//! +---------------------+---------------------+-----------------------------+
//! | Blend2D Format      | Cairo Format        | QImage::Format              |
//! +---------------------+---------------------+-----------------------------+
//! | BL_FORMAT_PRGB32    | CAIRO_FORMAT_ARGB32 | Format_ARGB32_Premultiplied |
//! | BL_FORMAT_XRGB32    | CAIRO_FORMAT_RGB24  | Format_RGB32                |
//! | BL_FORMAT_A8        | CAIRO_FORMAT_A8     | n/a                         |
//! +---------------------+---------------------+-----------------------------+
//! ```
BL_DEFINE_ENUM(BLFormat) {
  //! None or invalid pixel format.
  BL_FORMAT_NONE = 0,
  //! 32-bit premultiplied ARGB pixel format (8-bit components).
  BL_FORMAT_PRGB32 = 1,
  //! 32-bit (X)RGB pixel format (8-bit components, alpha ignored).
  BL_FORMAT_XRGB32 = 2,
  //! 8-bit alpha-only pixel format.
  BL_FORMAT_A8 = 3,

  // Maximum value of `BLFormat`.
  BL_FORMAT_MAX_VALUE = 3

  BL_FORCE_ENUM_UINT32(BL_FORMAT)
};

//! Pixel format flags.
BL_DEFINE_ENUM(BLFormatFlags) {
  //! No flags.
  BL_FORMAT_NO_FLAGS = 0u,
  //! Pixel format provides RGB components.
  BL_FORMAT_FLAG_RGB = 0x00000001u,
  //! Pixel format provides only alpha component.
  BL_FORMAT_FLAG_ALPHA = 0x00000002u,
  //! A combination of `BL_FORMAT_FLAG_RGB | BL_FORMAT_FLAG_ALPHA`.
  BL_FORMAT_FLAG_RGBA = 0x00000003u,
  //! Pixel format provides LUM component (and not RGB components).
  BL_FORMAT_FLAG_LUM = 0x00000004u,
  //! A combination of `BL_FORMAT_FLAG_LUM | BL_FORMAT_FLAG_ALPHA`.
  BL_FORMAT_FLAG_LUMA = 0x00000006u,
  //! Indexed pixel format the requires a palette (I/O only).
  BL_FORMAT_FLAG_INDEXED = 0x00000010u,
  //! RGB components are premultiplied by alpha component.
  BL_FORMAT_FLAG_PREMULTIPLIED = 0x00000100u,
  //! Pixel format doesn't use native byte-order (I/O only).
  BL_FORMAT_FLAG_BYTE_SWAP = 0x00000200u,

  // The following flags are only informative. They are part of `bl_format_info[]`, but don't have to be passed to
  // `BLPixelConverter` as they will always be calculated automatically.

  //! Pixel components are byte aligned (all 8bpp).
  BL_FORMAT_FLAG_BYTE_ALIGNED = 0x00010000u,

  //! Pixel has some undefined bits that represent no information.
  //!
  //! For example a 32-bit XRGB pixel has 8 undefined bits that are usually set to all ones so the format can be
  //! interpreted as premultiplied RGB as well. There are other formats like 16_0555 where the bit has no information
  //! and is usually set to zero. Blend2D doesn't rely on the content of such bits.
  BL_FORMAT_FLAG_UNDEFINED_BITS = 0x00020000u,

  //! Convenience flag that contains either zero or `BL_FORMAT_FLAG_BYTE_SWAP` depending on host byte order. Little
  //! endian hosts have this flag set to zero and big endian hosts to `BL_FORMAT_FLAG_BYTE_SWAP`.
  //!
  //! \note This is not a real flag that you can test, it's only provided for convenience to define little endian
  //! pixel formats.
  BL_FORMAT_FLAG_LE = (BL_BYTE_ORDER == 1234) ? (uint32_t)0 : (uint32_t)BL_FORMAT_FLAG_BYTE_SWAP,

  //! Convenience flag that contains either zero or `BL_FORMAT_FLAG_BYTE_SWAP` depending on host byte order. Big
  //! endian hosts have this flag set to zero and little endian hosts to `BL_FORMAT_FLAG_BYTE_SWAP`.
  //!
  //! \note This is not a real flag that you can test, it's only provided for convenience to define big endian
  //! pixel formats.
  BL_FORMAT_FLAG_BE = (BL_BYTE_ORDER == 4321) ? (uint32_t)0 : (uint32_t)BL_FORMAT_FLAG_BYTE_SWAP

  BL_FORCE_ENUM_UINT32(BL_FORMAT_FLAG)
};

//! \}
//! \}

//! \addtogroup bl_c_api
//! \{

//! \name BLFormat - C API
//! \{

BL_BEGIN_C_DECLS

BL_API BLResult BL_CDECL bl_format_info_query(BLFormatInfo* self, BLFormat format) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_format_info_sanitize(BLFormatInfo* self) BL_NOEXCEPT_C;

BL_END_C_DECLS

//! \}
//! \}

//! \addtogroup bl_imaging
//! \{

//! \name BLFormat - Structs
//! \{

//! Provides a detailed information about a pixel format. Use \ref bl_format_info array to get an information of Blend2D
//! native pixel formats.
struct BLFormatInfo {
  uint32_t depth;
  BLFormatFlags flags;

  union {
    struct {
      uint8_t sizes[4];
      uint8_t shifts[4];
    };

    struct {
      uint8_t r_size;
      uint8_t g_size;
      uint8_t b_size;
      uint8_t a_size;

      uint8_t r_shift;
      uint8_t g_shift;
      uint8_t b_shift;
      uint8_t a_shift;
    };

    BLRgba32* palette;
  };

#ifdef __cplusplus
  [[nodiscard]]
  BL_INLINE_NODEBUG bool operator==(const BLFormatInfo& other) const noexcept { return memcmp(this, &other, sizeof(*this)) == 0; }

  [[nodiscard]]
  BL_INLINE_NODEBUG bool operator!=(const BLFormatInfo& other) const noexcept { return memcmp(this, &other, sizeof(*this)) != 0; }

  BL_INLINE_NODEBUG void reset() noexcept { *this = BLFormatInfo{}; }

  BL_INLINE void init(uint32_t depth_, BLFormatFlags flags_, const uint8_t sizes_[4], const uint8_t shifts_[4]) noexcept {
    depth = depth_;
    flags = flags_;
    set_sizes(sizes_[0], sizes_[1], sizes_[2], sizes_[3]);
    set_shifts(shifts_[0], shifts_[1], shifts_[2], shifts_[3]);
  }

  BL_INLINE void set_sizes(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0) noexcept {
    r_size = r;
    g_size = g;
    b_size = b;
    a_size = a;
  }

  BL_INLINE void set_shifts(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0) noexcept {
    r_shift = r;
    g_shift = g;
    b_shift = b;
    a_shift = a;
  }

  BL_INLINE_NODEBUG bool has_flag(BLFormatFlags f) const noexcept { return (flags & f) != 0; }
  BL_INLINE_NODEBUG void add_flags(BLFormatFlags f) noexcept { flags = BLFormatFlags(flags | f); }
  BL_INLINE_NODEBUG void clear_flags(BLFormatFlags f) noexcept { flags = BLFormatFlags(flags & ~f); }

  //! Query Blend2D `format` and copy it to this format info, see \ref BLFormat.
  //!
  //! Copies data from \ref bl_format_info() to this \ref BLFormatInfo struct and returns \ref BL_SUCCESS if the
  //! `format` was valid, otherwise the \ref BLFormatInfo is reset and \ref BL_ERROR_INVALID_VALUE is returned.
  //!
  //! \note The \ref BL_FORMAT_NONE is considered invalid format, thus if it's passed to `query()` the return
  //! value would be \ref BL_ERROR_INVALID_VALUE.
  BL_INLINE_NODEBUG BLResult query(BLFormat format) noexcept { return bl_format_info_query(this, format); }

  //! Sanitize this \ref BLFormatInfo.
  //!
  //! Sanitizer verifies whether the format is valid and updates the format information about flags to values that
  //! Blend2D expects. For example format flags are properly examined and simplified if possible, byte-swap is
  //! implicitly performed for formats where a single component matches one byte, etc...
  BL_INLINE_NODEBUG BLResult sanitize() noexcept { return bl_format_info_sanitize(this); }
#endif
};

//! \}

//! \name BLFormat - Globals
//! \{

BL_BEGIN_C_DECLS

//! Pixel format information of Blend2D native pixel formats, see \ref BLFormat.
extern BL_API const BLFormatInfo bl_format_info[];

BL_END_C_DECLS

//! \}
//! \}

#endif // BLEND2D_FORMAT_H_INCLUDED
