// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_FONTTAGDATA_P_H_INCLUDED
#define BLEND2D_FONTTAGDATA_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/core/fonttagdataids_p.h>
#include <blend2d/core/fonttagdatainfo_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

namespace bl {
namespace FontTagData {

//! Number of valid ASCII characters that can be used within a single tag character, includes 'A-Z', 'a-z', '0-9', ...
static constexpr uint32_t kCharRangeInTag = 95;

//! Number of unique tags.
//!
//! This constant is used as a maximum capacity of containers that store tag to value mapping. There is 95 characters
//! between ' ' (32) and 126 (~), which are allowed in tags, we just need to power it by 4 to get the number of all
//! combinations.
static constexpr uint32_t kUniqueTagCount = kCharRangeInTag * kCharRangeInTag * kCharRangeInTag * kCharRangeInTag;

//! Test whether all 4 characters encoded in `tag` are within [32, 126] range.
static BL_INLINE bool is_valid_tag(BLTag tag) noexcept {
  constexpr uint32_t kSubPattern = 32;        // Tests characters in range [0, 31].
  constexpr uint32_t kAddPattern = 127 - 126; // Tests characters in range [127, 255].

  uint32_t x = tag - BL_MAKE_TAG(kSubPattern, kSubPattern, kSubPattern, kSubPattern);
  uint32_t y = tag + BL_MAKE_TAG(kAddPattern, kAddPattern, kAddPattern, kAddPattern);

  // If `x` or `y` underflown/overflown it would have one or more bits in `0x80808080` mask set. In
  // that case the given `tag` is not valid and has one or more character outside of the allowed range.
  return ((x | y) & 0x80808080u) == 0u;
}

static BL_INLINE bool is_open_type_collection_tag(BLTag tag) noexcept {
  return tag == BL_MAKE_TAG('t', 't', 'c', 'f');
}

static BL_INLINE bool is_open_type_version_tag(BLTag tag) noexcept {
  return tag == BL_MAKE_TAG('O', 'T', 'T', 'O') ||
         tag == BL_MAKE_TAG( 0 ,  1 ,  0 ,  0 ) ||
         tag == BL_MAKE_TAG('t', 'r', 'u', 'e') ;
}

//! Converts `tag` to a null-terminated ASCII string `str`. Characters that are not printable are replaced by '?'.
static BL_INLINE void tag_to_ascii(char str[5], uint32_t tag) noexcept {
  for (size_t i = 0; i < 4; i++, tag <<= 8) {
    uint32_t c = tag >> 24;
    str[i] = (c < 32 || c > 126) ? char('?') : char(c);
  }
  str[4] = '\0';
}

} // {FontTagData}
} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_FONTTAGDATA_P_H_INCLUDED
