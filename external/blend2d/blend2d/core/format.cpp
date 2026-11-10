// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/format_p.h>
#include <blend2d/support/lookuptable_p.h>

// bl::FormatInfo - Globals
// ========================

const BLFormatInfo bl_format_info[] = {
  #define U 0 // Used only to distinguish between zero and unused.
  // Public Formats:
  { 0 , BLFormatFlags(bl::FormatInternal::make_flags_static(bl::FormatExt(0))), {{ { U , U , U , U  }, { U , U , U , U  } }} }, // <kNONE>
  { 32, BLFormatFlags(bl::FormatInternal::make_flags_static(bl::FormatExt(1))), {{ { 8 , 8 , 8 , 8  }, { 16, 8 , 0 , 24 } }} }, // <kPRGB32>
  { 32, BLFormatFlags(bl::FormatInternal::make_flags_static(bl::FormatExt(2))), {{ { 8 , 8 , 8 , U  }, { 16, 8 , 0 , U  } }} }, // <kXRGB32>
  { 8 , BLFormatFlags(bl::FormatInternal::make_flags_static(bl::FormatExt(3))), {{ { U , U , U , 8  }, { U , U , U , 0  } }} }, // <kA8>

  // Internal Formats:
  { 32, BLFormatFlags(bl::FormatInternal::make_flags_static(bl::FormatExt(4))), {{ { 8 , 8 , 8 , 8  }, { 16, 8 , 0 , 24 } }} }, // <kFRGB32>
  { 32, BLFormatFlags(bl::FormatInternal::make_flags_static(bl::FormatExt(5))), {{ { 8 , 8 , 8 , 8  }, { 16, 8 , 0 , 24 } }} }, // <kZERO32>

  // Internal Formats (currently only used only in few places, not supported in generic API).
  { 64, BLFormatFlags(bl::FormatInternal::make_flags_static(bl::FormatExt(6))), {{ { 16, 16, 16, 16 }, { 32, 16, 0 , 48 } }} }, // <kPRGB64>
  { 64, BLFormatFlags(bl::FormatInternal::make_flags_static(bl::FormatExt(7))), {{ { 16, 16, 16, 16 }, { 32, 16, 0 , 48 } }} }, // <kFRGB64>
  { 64, BLFormatFlags(bl::FormatInternal::make_flags_static(bl::FormatExt(8))), {{ { 16, 16, 16, 16 }, { 32, 16, 0 , 48 } }} }  // <kZERO64>
  #undef U
};

static_assert(uint32_t(bl::FormatExt::kMaxValue) == 8,
              "New formats must be added to 'bl_format_info' table");

// bl::FormatInfo - Tables
// =======================

// Indexes of components based on format flags that describe components. Each bit in the mask describes RGBA components
// (in order). Thus 0x1 describes red component, 0x2 green 0x4 blue, and 0x8 alpha. Components can be combined so 0x7
// describes RGB and 0xF RGBA.
struct BLPixelConverterComponentIndexesGen {
  static constexpr uint8_t value(size_t i) noexcept {
    return i == BL_FORMAT_FLAG_RGB   ? uint8_t(0x7) :
           i == BL_FORMAT_FLAG_ALPHA ? uint8_t(0x8) :
           i == BL_FORMAT_FLAG_RGBA  ? uint8_t(0xF) :
           i == BL_FORMAT_FLAG_LUM   ? uint8_t(0x7) :
           i == BL_FORMAT_FLAG_LUMA  ? uint8_t(0xF) : uint8_t(0);
  }
};

static constexpr const auto bl_pixel_converter_component_indexes_table =
  bl::make_lookup_table<uint8_t, 16, BLPixelConverterComponentIndexesGen>();

// bl::FormatInfo - Query
// ======================

BLResult bl_format_info_query(BLFormatInfo* self, BLFormat format) noexcept {
  if (BL_UNLIKELY(format == BL_FORMAT_NONE || format > BL_FORMAT_MAX_VALUE)) {
    self->reset();
    return bl_make_error(BL_ERROR_INVALID_VALUE);
  }

  *self = bl_format_info[format];
  return BL_SUCCESS;
}

// bl::FormatInfo - Sanitize
// =========================

static BL_INLINE bool bl_format_info_is_depth_valid(uint32_t depth) noexcept {
  switch (depth) {
    case 1:
    case 2:
    case 4:
    case 8:
    case 16:
    case 24:
    case 32:
      return true;

    default:
      return false;
  }
}

BL_API_IMPL BLResult bl_format_info_sanitize(BLFormatInfo* self) noexcept {
  using bl::FormatFlagsExt;

  BLFormatInfo& f = *self;

  // Filter out all flags that will be computed.
  FormatFlagsExt flags = FormatFlagsExt(f.flags) & FormatFlagsExt::kAllPublicFlags;

  bool masks_overlap = false;
  bool not_byte_aligned = false;
  bool crosses_byte_boundary = false;
  bool has_undefined_bits = false;

  // Check depth.
  if (!bl_format_info_is_depth_valid(f.depth))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  if (bl_test_flag(flags, FormatFlagsExt::kIndexed)) {
    // In 32-bit mode shifts are not overlapping with `palette` so zero them.
    if (sizeof(void*) == 4)
      memset(f.shifts, 0, sizeof(f.shifts));

    // Indexed formats are up to 8 bits-per-pixel and must have palette.
    if (f.depth > 8 || !f.palette)
      return bl_make_error(BL_ERROR_INVALID_VALUE);
  }
  else {
    // Check whether RGB|A components are correct.
    uint64_t masks_combined = 0;

    // Check whether pixel components are specified correctly.
    uint32_t component_indexes = bl_pixel_converter_component_indexes_table[uint32_t(flags) & 0xF];
    if (!component_indexes)
      return bl_make_error(BL_ERROR_INVALID_VALUE);

    for (uint32_t i = 0; i < 4; i++) {
      uint32_t size = f.sizes[i];
      uint32_t shift = f.shifts[i];

      if (size == 0) {
        // Fail if this component must be provided.
        if (component_indexes & (1u << i))
          return bl_make_error(BL_ERROR_INVALID_VALUE);

        // Undefined size (0) must have zero shift as well. As it's not used it doesn't make sense to assign
        // it a value.
        if (shift != 0)
          return bl_make_error(BL_ERROR_INVALID_VALUE);
      }
      else {
        // Fail if this component must not be provided.
        if (!(component_indexes & (1u << i)))
          return bl_make_error(BL_ERROR_INVALID_VALUE);

        // Fail if the size is too large.
        if (size > 16)
          return bl_make_error(BL_ERROR_INVALID_VALUE);

        // Shifted mask overflows the pixel depth?
        if (shift + size > f.depth)
          return bl_make_error(BL_ERROR_INVALID_VALUE);

        // Byte aligned means that shifts are [0, 8, 16, 24] and mask is 0xFF.
        if (size != 8 || (shift & 0x7u) != 0)
          not_byte_aligned = true;

        // Does the mask cross a byte-boundary?
        if ((shift / 8u) != ((shift + size - 1) / 8u))
          crosses_byte_boundary = true;

        // Does the mask overlap with others?
        uint64_t maskAsU64 = uint64_t(bl::IntOps::non_zero_lsb_mask<uint32_t>(size)) << shift;
        if (masks_combined & maskAsU64) {
          masks_overlap = true;
          // Alpha channels cannot overlap.
          if (i == 3)
            return bl_make_error(BL_ERROR_INVALID_VALUE);
        }

        masks_combined |= maskAsU64;
      }
    }

    if (bl::IntOps::non_zero_lsb_mask<uint64_t>(f.depth) ^ masks_combined)
      has_undefined_bits = true;

    // Unset `kPremultiplied` if the format doesn't have alpha.
    if (!bl_test_flag(flags, FormatFlagsExt::kAlpha))
      flags &= ~FormatFlagsExt::kPremultiplied;

    // It's allowed that masks overlap only when the pixel format describes a grayscale (LUM).
    bool isLUM = bl_test_flag(flags, FormatFlagsExt::kLUM);
    if (isLUM != masks_overlap)
      return bl_make_error(BL_ERROR_INVALID_VALUE);

    // RGB components must match in grayscale (LUM) mode.
    if (isLUM && (f.r_size != f.g_size || f.r_shift != f.g_shift ||
                  f.g_size != f.b_size || f.g_shift != f.b_shift))
      return bl_make_error(BL_ERROR_INVALID_VALUE);
  }

  // Switch to a native byte-order if possible.
  if (bl_test_flag(flags, FormatFlagsExt::kByteSwap)) {
    if (f.depth <= 8) {
      // Switch to native byte-order if the depth <= 8.
      flags &= ~FormatFlagsExt::kByteSwap;
    }
    else if (!crosses_byte_boundary) {
      // Switch to native byte-order if no mask crosses byte boundaries.
      for (uint32_t i = 0; i < 4; i++) {
        uint32_t size = f.sizes[i];
        if (!size)
          continue;
        f.shifts[i] = uint8_t(f.depth - f.shifts[i] - size);
      }

      flags &= ~FormatFlagsExt::kByteSwap;
    }
  }

  // Add computed flags.
  if (!not_byte_aligned)
    flags |= FormatFlagsExt::kByteAligned;

  if (has_undefined_bits)
    flags |= FormatFlagsExt::kUndefinedBits;

  f.flags = BLFormatFlags(flags);
  return BL_SUCCESS;
}
