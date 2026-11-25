// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_SUPPORT_STRINGOPS_P_H_INCLUDED
#define BLEND2D_SUPPORT_STRINGOPS_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/unicode/unicode_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_support
//! \{

namespace bl {
namespace StringOps {

//! \name Support - String Operations
//! \{

template<typename T>
static BL_INLINE size_t length(const T* data) noexcept {
  if (sizeof(T) == 1) {
    return strlen(reinterpret_cast<const char*>(data));
  }
  else {
    const T* p = data;
    while (p[0])
      p++;
    return (size_t)(p - data);
  }
}

static size_t length_with_encoding(const void* text, BLTextEncoding encoding) noexcept {
  switch (encoding) {
    case BL_TEXT_ENCODING_LATIN1:
    case BL_TEXT_ENCODING_UTF8:
      return length(static_cast<const uint8_t*>(text));

    case BL_TEXT_ENCODING_UTF16:
      return length(static_cast<const uint16_t*>(text));

    case BL_TEXT_ENCODING_UTF32:
      return length(static_cast<const uint32_t*>(text));

    default:
      return 0;
  }
}

static BL_INLINE bool memeq_ci(const char* a, const char* b, size_t size) noexcept {
  size_t i;
  for (i = 0; i < size; i++) {
    if (bl::Unicode::ascii_to_lower(a[i]) != bl::Unicode::ascii_to_lower(b[i])) {
      return false;
    }
  }
  return true;
}

//! \}

} // {StringOps}
} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_SUPPORT_STRINGOPS_P_H_INCLUDED
