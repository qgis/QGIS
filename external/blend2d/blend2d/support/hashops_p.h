// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_SUPPORT_HASHOPS_P_H_INCLUDED
#define BLEND2D_SUPPORT_HASHOPS_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/unicode/unicode_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

namespace bl {
namespace HashOps {
namespace {

//! \name Hash Functions
//! \{

static BL_INLINE uint32_t hash_char(uint32_t hash, uint32_t c) noexcept {
  return hash * 65599u + c;
}

static BL_INLINE uint32_t hash_charCI(uint32_t hash, uint32_t c) noexcept {
  return hash * 65599u + Unicode::ascii_to_lower(c);
}

// Gets a hash of the given string `data` of size `size`. Size must be valid
// as this function doesn't check for a null terminator and allows it in the
// middle of the string.
static BL_INLINE uint32_t hash_string(const char* data, size_t size) noexcept {
  uint32_t hash_code = 0;
  for (uint32_t i = 0; i < size; i++)
    hash_code = hash_char(hash_code, uint8_t(data[i]));
  return hash_code;
}

static BL_INLINE uint32_t hash_string(BLStringView view) noexcept {
  return hash_string(view.data, view.size);
}

// Gets a hash of the given string `data` of size `size`. Size must be valid
// as this function doesn't check for a null terminator and allows it in the
// middle of the string.
static BL_INLINE uint32_t hash_stringCI(const char* data, size_t size) noexcept {
  uint32_t hash_code = 0;
  for (uint32_t i = 0; i < size; i++)
    hash_code = hash_charCI(hash_code, uint8_t(data[i]));
  return hash_code;
}

static BL_INLINE uint32_t hash_stringCI(BLStringView view) noexcept {
  return hash_stringCI(view.data, view.size);
}

//! \}

} // {anonymous}
} // {HashOps}
} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_SUPPORT_HASHOPS_P_H_INCLUDED
