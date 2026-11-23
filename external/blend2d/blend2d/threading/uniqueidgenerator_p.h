// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_THREADING_UNIQUEIDGENERATOR_P_H_INCLUDED
#define BLEND2D_THREADING_UNIQUEIDGENERATOR_P_H_INCLUDED

#include <blend2d/threading/atomic_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

namespace BLUniqueIdGenerator {

enum class Domain : uint32_t {
  kAny = 0,
  kContext = 1,

  kMaxValue = 1
};

BL_HIDDEN BLUniqueId generate_id(Domain domain) noexcept;

} // {BLUniqueIdGenerator}

//! \}
//! \endcond

#endif // BLEND2D_THREADING_UNIQUEIDGENERATOR_P_H_INCLUDED
