// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_OPENTYPE_OTGLYFSIMDDATA_P_H_INCLUDED
#define BLEND2D_OPENTYPE_OTGLYFSIMDDATA_P_H_INCLUDED

#include <blend2d/opentype/otglyf_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_opentype_impl
//! \{

namespace bl::OpenType {
namespace GlyfImpl {

//! Flags that are used by the vectorized outline decoder implementation.
//!
//! Most of the flags are the same as flags used in TT outlines, however, the following modifications were made in
//! order to make the implementation faster:
//!
//!   1. XByte|YByte|XSame|YSame flags were moved to [3:0] bits so they can be used as a predicate with VPSHUFB
//!      instruction. These 4 bits are the only important bits to decode X/Y vertices.
//!   2. XSameOrPositive and YSameOrPositive flags were negated. After negation when all [3:0] bits are zero, the
//!      vertex is zero as well. This is required when processing multiple flags at once at the end. Extra flags
//!      in a loop that processes 8 or 16 flags at a time are zero, thus they don't contribute to X/Y data lengths.
//!   3. OnCurve flag and its complement flag (OffCurve) are stored next to each other. When these flags are shifted
//!      to [1:0] bits they represent either `BL_PATH_CMD_ON` or `BL_PATH_CMD_QUAD` commands, which is handy in the
//!      last loop that appends vertices.
//!   4. Additional OffSpline flag is a combination of OffCurve flag with previous OffCurve flag. If both were set
//!      then this flag would have OffSpline set as well. This is important for counting how many off-curve splines
//!      are in the data, and later in the last loop to check whether we are in off curve spline or not.
//!   5. Repeat flag is last so we can use VPMOVMSKB instruction to quickly check for repeated flags.
enum VecFlags : uint8_t {
  kVecFlagXByte = 0x01,
  kVecFlagYByte = 0x02,
  kVecFlagXNotSameOrPositive = 0x04,
  kVecFlagYNotSameOrPositive = 0x08,
  kVecFlagOffSpline = 0x10,
  kVecFlagOnCurve = 0x20,
  kVecFlagOffCurve = 0x40,
  kVecFlagRepeat = 0x80
};

static constexpr uint32_t kVecFlagOnCurveShift = IntOps::ctz_static(kVecFlagOnCurve);

// The first 16 bytes are used as a predicate for PSHUFB.
alignas(16) extern const uint8_t convert_flags_predicate[64];

alignas(8) extern const uint8_t overflow_flags_predicate[32];

// Vertex size for each y_same|x_same|y_byte|x_byte combination.
alignas(16) extern const uint8_t sizesPerXYPredicate[16];

// These tables provide PSHUFB predicate (and additional payload) for decoding X/Y coordinates based on flags.
// The trick is to add 0x41 to each even byte to create a predicate for both LO and HI byte based on a single
// table. We add values to the LO byte as TT words are stored in big endian, so this trick makes byteswapping
// of the input words for free.
//
// NOTEs:
//   PSHUFB only uses [7] and [3:0] bits, other bits are ignored
//   0x20 means negation of both LO and HI bytes (single byte decode having X/YSameOrPositive == 0).
//   hi = (val     ) & 0x8F
//   lo = (val+0x41) & 0x8F
alignas(16) extern const uint8_t decodeOpXTable[16];
alignas(16) extern const uint8_t decodeOpYTable[16];

} // {GlyfImpl}
} // {bl::OpenType}

//! \}
//! \endcond

#endif // BLEND2D_OPENTYPE_OTGLYFSIMDDATA_P_H_INCLUDED
