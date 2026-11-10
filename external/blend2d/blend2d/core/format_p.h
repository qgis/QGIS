// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_FORMAT_P_H_INCLUDED
#define BLEND2D_FORMAT_P_H_INCLUDED

#include <blend2d/core/format.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/lookuptable_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

namespace bl {

//! Pixel format that extends \ref BLFormat, used internally and never exposed to users.
enum class FormatExt : uint32_t {
  //! None or invalid pixel format.
  kNone = BL_FORMAT_NONE,
  //! 32-bit premultiplied ARGB pixel format (8-bit components).
  kPRGB32 = BL_FORMAT_PRGB32,
  //! 32-bit (X)RGB pixel format (8-bit components, alpha ignored).
  kXRGB32 = BL_FORMAT_XRGB32,
  //! 8-bit alpha-only pixel format.
  kA8 = BL_FORMAT_A8,

  //! 32-bit (X)RGB pixel format, where X is always 0xFF, thus the pixel is compatible with `kXRGB32` and `kPRGB32`.
  kFRGB32 = BL_FORMAT_MAX_VALUE + 1u,
  //! 32-bit (X)RGB pixel format where the pixel is always zero.
  kZERO32 = BL_FORMAT_MAX_VALUE + 2u,

  kPRGB64 = BL_FORMAT_MAX_VALUE + 3u,
  kFRGB64 = BL_FORMAT_MAX_VALUE + 4u,
  kZERO64 = BL_FORMAT_MAX_VALUE + 5u,

  // Maximum value of `FormatExt`.
  kMaxValue = kZERO64,

  // Maximum value of `FormatExt` that is a power of 2 minus 1, to make indexing of some tables easy.
  kMaxReserved = 15
};

static constexpr uint32_t kFormatExtCount = uint32_t(FormatExt::kMaxReserved) + 1u;

//! Pixel format flags that extend \ref BLFormatFlags, used internally.
enum class FormatFlagsExt : uint32_t {
  kNoFlags = BL_FORMAT_NO_FLAGS,
  kRGB = BL_FORMAT_FLAG_RGB,
  kAlpha = BL_FORMAT_FLAG_ALPHA,
  kRGBA = BL_FORMAT_FLAG_RGBA,
  kLUM = BL_FORMAT_FLAG_LUM,
  kLUMA = BL_FORMAT_FLAG_LUMA,
  kIndexed = BL_FORMAT_FLAG_INDEXED,
  kPremultiplied = BL_FORMAT_FLAG_PREMULTIPLIED,
  kByteSwap = BL_FORMAT_FLAG_BYTE_SWAP,
  kByteAligned = BL_FORMAT_FLAG_BYTE_ALIGNED,
  kUndefinedBits = BL_FORMAT_FLAG_UNDEFINED_BITS,
  kLE = BL_FORMAT_FLAG_LE,
  kBE = BL_FORMAT_FLAG_BE,

  kFullAlpha = 0x10000000u,
  kZeroAlpha = 0x20000000u,

  kComponentFlags = kLUM | kRGB | kAlpha,
  kAllPublicFlags = kComponentFlags | kIndexed | kPremultiplied | kByteSwap
};

static_assert(uint32_t(FormatFlagsExt::kComponentFlags) == 0x7u,
              "Component flags of FormatFlagsExt must be at LSB");

BL_DEFINE_ENUM_FLAGS(FormatFlagsExt)

namespace FormatInternal {

static BL_INLINE_CONSTEXPR FormatFlagsExt make_flags_static(FormatExt format) noexcept {
  return format == FormatExt::kPRGB32 ? FormatFlagsExt::kRGBA          |
                                        FormatFlagsExt::kPremultiplied |
                                        FormatFlagsExt::kByteAligned   :
         format == FormatExt::kXRGB32 ? FormatFlagsExt::kRGB           |
                                        FormatFlagsExt::kByteAligned   |
                                        FormatFlagsExt::kUndefinedBits :
         format == FormatExt::kA8     ? FormatFlagsExt::kAlpha         |
                                        FormatFlagsExt::kByteAligned   :
         format == FormatExt::kFRGB32 ? FormatFlagsExt::kRGB           |
                                        FormatFlagsExt::kByteAligned   |
                                        FormatFlagsExt::kFullAlpha     :
         format == FormatExt::kZERO32 ? FormatFlagsExt::kRGBA          |
                                        FormatFlagsExt::kByteAligned   |
                                        FormatFlagsExt::kZeroAlpha     :
         format == FormatExt::kPRGB64 ? FormatFlagsExt::kRGBA          |
                                        FormatFlagsExt::kByteAligned   :
         format == FormatExt::kFRGB64 ? FormatFlagsExt::kRGB           |
                                        FormatFlagsExt::kByteAligned   |
                                        FormatFlagsExt::kFullAlpha     :
         format == FormatExt::kZERO64 ? FormatFlagsExt::kRGBA          |
                                        FormatFlagsExt::kByteAligned   |
                                        FormatFlagsExt::kUndefinedBits |
                                        FormatFlagsExt::kZeroAlpha     : FormatFlagsExt::kNoFlags;
}

static BL_INLINE bool has_same_alpha_layout(const BLFormatInfo& a, const BLFormatInfo& b) noexcept {
  return (a.sizes[3] == b.sizes[3]) & (a.shifts[3] == b.shifts[3]) ;
}

static BL_INLINE bool has_same_rgb_layout(const BLFormatInfo& a, const BLFormatInfo& b) noexcept {
  return (a.sizes[0] == b.sizes[0]) & (a.shifts[0] == b.shifts[0]) &
         (a.sizes[1] == b.sizes[1]) & (a.shifts[1] == b.shifts[1]) &
         (a.sizes[2] == b.sizes[2]) & (a.shifts[2] == b.shifts[2]) ;
}

static BL_INLINE bool has_same_rgba_layout(const BLFormatInfo& a, const BLFormatInfo& b) noexcept {
  return (a.sizes[0] == b.sizes[0]) & (a.shifts[0] == b.shifts[0]) &
         (a.sizes[1] == b.sizes[1]) & (a.shifts[1] == b.shifts[1]) &
         (a.sizes[2] == b.sizes[2]) & (a.shifts[2] == b.shifts[2]) &
         (a.sizes[3] == b.sizes[3]) & (a.shifts[3] == b.shifts[3]) ;
}

//! Converts absolute masks like `0x3F0` to mask sizes and shift shifts as used by `BLFormatInfo`. Only useful for
//! pixel formats with absolute masks up to 64 bits (uint64_t input). Commonly used to convert pixel formats that
//! use 32 or less bits.
template<typename T>
static void assign_absolute_masks(BLFormatInfo& info, const T* masks, size_t n = 4) noexcept {
  using U = std::make_unsigned_t<T>;

  memset(info.sizes, 0, sizeof(info.sizes));
  memset(info.shifts, 0, sizeof(info.shifts));

  for (size_t i = 0; i < n; i++) {
    U m = U(masks[i]);
    if (!m) {
      continue;
    }

    uint32_t shift = IntOps::ctz(m);
    m >>= shift;
    uint32_t size = m >= 0xFFFFFFFFu ? uint32_t(32) : IntOps::ctz(~uint32_t(m));

    info.sizes[i] = uint8_t(size);
    info.shifts[i] = uint8_t(shift);
  }
}

} // {FormatInternal}
} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_FORMAT_P_H_INCLUDED
