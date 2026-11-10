// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/font_p.h>
#include <blend2d/core/matrix_p.h>
#include <blend2d/core/path_p.h>
#include <blend2d/core/trace_p.h>
#include <blend2d/geometry/bezier_p.h>
#include <blend2d/opentype/otcff_p.h>
#include <blend2d/opentype/otface_p.h>
#include <blend2d/support/memops_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/lookuptable_p.h>
#include <blend2d/support/ptrops_p.h>
#include <blend2d/support/scopedbuffer_p.h>
#include <blend2d/support/traits_p.h>

// bl::OpenType::CFFImpl - Tracing
// ===============================

#if defined(BL_TRACE_OT_ALL) && !defined(BL_TRACE_OT_CFF)
  #define BL_TRACE_OT_CFF
#endif

#if defined(BL_TRACE_OT_CFF)
  #include <stdio.h>
  #define Trace BLDebugTrace
#else
  #define Trace BLDummyTrace
#endif

namespace bl::OpenType {
namespace CFFImpl {

// bl::OpenType::CFFImpl - Utilities
// =================================

// Specified by "CFF - Local/Global Subrs INDEXes"
static BL_INLINE uint16_t calc_subr_bias(uint32_t subr_count) noexcept {
  // NOTE: For CharStrings v1 this would return 0, but since OpenType fonts use exclusively CharStrings v2 we always
  // calculate the bias. The calculated bias is added to each call to global or local subroutine before its index is
  // used to get its offset.
  if (subr_count < 1240u)
    return 107u;
  else if (subr_count < 33900u)
    return 1131u;
  else
    return 32768u;
}

template<typename T>
static BL_INLINE uint32_t read_offset(const T* p, size_t offset_size) noexcept {
  const uint8_t* oPtr = reinterpret_cast<const uint8_t*>(p);
  const uint8_t* oEnd = oPtr + offset_size;

  uint32_t offset = 0;
  for (;;) {
    offset |= oPtr[0];
    if (++oPtr == oEnd)
      break;
    offset <<= 8;
  }
  return offset;
}

template<typename T>
static BL_INLINE void read_offset_array(const T* p, size_t offset_size, uint32_t* offset_array_out, size_t n) noexcept {
  const uint8_t* oPtr = reinterpret_cast<const uint8_t*>(p);
  const uint8_t* oEnd = oPtr;

  for (size_t i = 0; i < n; i++) {
    uint32_t offset = 0;
    oEnd += offset_size;

    for (;;) {
      offset |= oPtr[0];
      if (++oPtr == oEnd)
        break;
      offset <<= 8;
    }

    offset_array_out[i] = offset;
  }
}

BLResult read_float(const uint8_t* p, const uint8_t* pEnd, double& value_out, size_t& value_size_in_bytes) noexcept {
  // Maximum digits that we would attempt to read, excluding leading zeros.
  enum : uint32_t { kSafeDigits = 15 };

  // Meaning of nibbles above 9.
  enum NibbleAbove9 : uint32_t {
    kDecimalPoint     = 0xA,
    kPositiveExponent = 0xB,
    kNegativeExponent = 0xC,
    kReserved         = 0xD,
    kMinusSign        = 0xE,
    kEndOfNumber      = 0xF
  };

  const uint8_t* pStart = p;
  uint32_t acc = 0x100u;
  uint32_t nib = 0;
  uint32_t flags = 0;

  double value = 0.0;
  uint32_t digits = 0;
  int scale = 0;

  // Value.
  for (;;) {
    if (acc & 0x100u) {
      if (BL_UNLIKELY(p == pEnd))
        return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);
      acc = (uint32_t(*p++) << 24) | 0x1;
    }

    nib = acc >> 28;
    acc <<= 4;

    uint32_t msk = 1u << nib;
    if (nib < 10) {
      if (digits < kSafeDigits) {
        value = value * 10.0 + double(int(nib));
        digits += uint32_t(value != 0.0);
        if (IntOps::bit_test(flags, kDecimalPoint))
          scale--;
      }
      else {
        if (!IntOps::bit_test(flags, kDecimalPoint))
          scale++;
      }
      flags |= msk;
    }
    else {
      if (BL_UNLIKELY(flags & msk))
        return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);

      flags |= msk;
      if (nib == kMinusSign) {
        // Minus must start the string, so check the whole mask...
        if (BL_UNLIKELY(flags & (0xFFFF ^ (1u << kMinusSign))))
          return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);
      }
      else if (nib != kDecimalPoint) {
        break;
      }
    }
  }

  // Exponent.
  if (nib == kPositiveExponent || nib == kNegativeExponent) {
    int exp_value = 0;
    int exp_digits = 0;
    bool positive_exponent = (nib == kPositiveExponent);

    for (;;) {
      if (acc & 0x100u) {
        if (BL_UNLIKELY(p == pEnd))
          return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);
        acc = (uint32_t(*p++) << 24) | 0x1;
      }

      nib = acc >> 28;
      acc <<= 4;

      if (nib >= 10)
        break;

      // If this happens the data is probably invalid anyway...
      if (BL_UNLIKELY(exp_digits >= 6))
        return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);

      exp_value = exp_value * 10 + int(nib);
      exp_digits += int(exp_value != 0);
    }

    if (positive_exponent)
      scale += exp_value;
    else
      scale -= exp_value;
  }

  if (nib != kEndOfNumber)
    return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);

  if (scale) {
    double s = Math::pow(10.0, bl_abs(double(scale)));
    value = scale > 0 ? value * s : value / s;
  }

  value_out = IntOps::bit_test(flags, kMinusSign) ? -value : value;
  value_size_in_bytes = PtrOps::byte_offset(pStart, p);

  return BL_SUCCESS;
}

// bl::OpenType::CFFImpl - Index
// =============================

struct Index {
  uint32_t count;
  uint8_t header_size;
  uint8_t offset_size;
  uint16_t reserved;
  uint32_t payload_size;
  uint32_t total_size;
  const uint8_t* offsets;
  const uint8_t* payload;

  BL_INLINE uint32_t offset_at(size_t index) const noexcept {
    BL_ASSERT(index <= count);
    return read_offset(offsets + index * offset_size, offset_size) - CFFTable::kOffsetAdjustment;
  }
};

// bl::OpenType::CFFImpl - ReadIndex
// =================================

static BLResult read_index(const void* data, size_t data_size, uint32_t cff_version, Index* index_out) noexcept {
  uint32_t count = 0;
  uint32_t header_size = 0;

  if (cff_version == CFFData::kVersion1) {
    if (BL_UNLIKELY(data_size < 2))
      return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);

    count = MemOps::readU16uBE(data);
    header_size = 2;
  }
  else {
    if (BL_UNLIKELY(data_size < 4))
      return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);

    count = MemOps::readU32uBE(data);
    header_size = 4;
  }

  // Index with no data is allowed by the specification.
  if (!count) {
    index_out->total_size = header_size;
    return BL_SUCCESS;
  }

  // Include also `offset_size` in header, if the `count` is non-zero.
  header_size++;
  if (BL_UNLIKELY(data_size < header_size))
    return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);

  uint32_t offset_size = MemOps::readU8(PtrOps::offset<const uint8_t>(data, header_size - 1));
  uint32_t offset_array_size = (count + 1) * offset_size;
  uint32_t index_size_including_offsets = header_size + offset_array_size;

  if (BL_UNLIKELY(offset_size < 1 || offset_size > 4 || index_size_including_offsets > data_size))
    return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);

  const uint8_t* offset_array = PtrOps::offset<const uint8_t>(data, header_size);
  uint32_t offset = read_offset(offset_array, offset_size);

  // The first offset should be 1.
  if (BL_UNLIKELY(offset != 1))
    return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);

  // Validate that the offsets are increasing and don't cross each other. The specification says that size of each
  // object stored in the table can be determined by checking its offset and the next one, so valid data should
  // conform to these checks.
  //
  // Please note the use of `kOffsetAdjustment`. Since all offsets are relative to "RELATIVE TO THE BYTE THAT
  // PRECEDES THE OBJECT DATA" we must account that.
  uint32_t max_offset = uint32_t(bl_min<size_t>(Traits::max_value<uint32_t>(), data_size - index_size_including_offsets + CFFTable::kOffsetAdjustment));

  switch (offset_size) {
    case 1: {
      for (uint32_t i = 1; i <= count; i++) {
        uint32_t next = MemOps::readU8(offset_array + i);
        if (BL_UNLIKELY(next < offset || next > max_offset))
          return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);
        offset = next;
      }
      break;
    }

    case 2:
      for (uint32_t i = 1; i <= count; i++) {
        uint32_t next = MemOps::readU16uBE(offset_array + i * 2u);
        if (BL_UNLIKELY(next < offset || next > max_offset))
          return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);
        offset = next;
      }
      break;

    case 3:
      for (uint32_t i = 1; i <= count; i++) {
        uint32_t next = MemOps::readU24uBE(offset_array + i * 3u);
        if (BL_UNLIKELY(next < offset || next > max_offset))
          return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);
        offset = next;
      }
      break;

    case 4:
      for (uint32_t i = 1; i <= count; i++) {
        uint32_t next = MemOps::readU32uBE(offset_array + i * 4u);
        if (BL_UNLIKELY(next < offset || next > max_offset))
          return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);
        offset = next;
      }
      break;
  }

  const uint8_t* payload = offset_array + offset_array_size;
  uint32_t payload_size = offset - 1;

  index_out->count = count;
  index_out->header_size = uint8_t(header_size);
  index_out->offset_size = uint8_t(offset_size);
  index_out->reserved = 0;
  index_out->payload_size = payload_size;
  index_out->total_size = header_size + offset_array_size + payload_size;
  index_out->offsets = offset_array;
  index_out->payload = payload;

  return BL_SUCCESS;
}

// bl::OpenType::CFFImpl - DictIterator
// ====================================

BLResult DictIterator::next(DictEntry& entry) noexcept {
  BL_ASSERT(has_next());

  uint32_t i = 0;
  uint32_t op = 0;
  uint64_t fp_mask = 0;

  for (;;) {
    uint32_t b0 = *_data_ptr++;

    // Operators are encoded in range [0..21].
    if (b0 < 22) {
      // 12 is a special escape code to encode additional operators.
      if (b0 == CFFTable::kEscapeDictOp) {
        if (BL_UNLIKELY(_data_ptr == _data_end))
          return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);
        b0 = (b0 << 8) | (*_data_ptr++);
      }
      op = b0;
      break;
    }
    else {
      double v;

      if (b0 == 30) {
        size_t size;
        BL_PROPAGATE(read_float(_data_ptr, _data_end, v, size));

        fp_mask |= uint64_t(1) << i;
        _data_ptr += size;
      }
      else {
        int32_t v_int = 0;
        if (b0 >= 32 && b0 <= 246) {
          v_int = int32_t(b0) - 139;
        }
        else if (b0 >= 247 && b0 <= 254) {
          if (BL_UNLIKELY(_data_ptr == _data_end))
            return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);

          uint32_t b1 = *_data_ptr++;
          v_int = b0 <= 250 ? (108 - 247 * 256) + int(b0 * 256 + b1)
                           : (251 * 256 - 108) - int(b0 * 256 + b1);
        }
        else if (b0 == 28) {
          _data_ptr += 2;
          if (BL_UNLIKELY(_data_ptr > _data_end))
            return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);
          v_int = MemOps::readI16uBE(_data_ptr - 2);
        }
        else if (b0 == 29) {
          _data_ptr += 4;
          if (BL_UNLIKELY(_data_ptr > _data_end))
            return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);
          v_int = MemOps::readI32uBE(_data_ptr - 4);
        }
        else {
          // Byte values 22..27, 31, and 255 are reserved.
          return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);
        }

        v = double(v_int);
      }

      if (BL_UNLIKELY(i == DictEntry::kValueCapacity - 1))
        return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);

      entry.values[i++] = v;
    }
  }

  // Specification doesn't talk about entries that have no values.
  if (BL_UNLIKELY(!i))
    return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);

  entry.op = op;
  entry.count = i;
  entry.fp_mask = fp_mask;

  return BL_SUCCESS;
}

// bl::OpenType::CFFImpl - Constants
// =================================

// ADOBE uses a limit of 20 million instructions in their AVALON rasterizer, but it's not clear that it's because of
// font complexity or their PostScript support.
//
// It seems that this limit is too optimistic to be reached by any OpenType font. We use a different metric, a program
// size, which is referenced by `bytes_processed` counter in the decoder. This counter doesn't have to be advanced every
// time we process an opcode, instead, we advance it every time we enter a subroutine (or CharString program itself).
// If we reach `kCFFProgramLimit` the interpreter is terminated immediately.
static constexpr uint32_t kCFFProgramLimit = 1000000;
static constexpr uint32_t kCFFCallStackSize = 16;
static constexpr uint32_t kCFFStorageSize = 32;

static constexpr uint32_t kCFFValueStackSizeV1 = 48;

// TODO: [OpenType] Required by CFF2.
// static constexpr uint32_t kCFFValueStackSizeV2 = 513;

// We use `double` precision in our implementation, so this constant is used to convert a fixed-point.
static constexpr double kCFFDoubleFromF16x16 = (1.0 / 65536.0);

enum CSFlags : uint32_t {
  kCSFlagHasWidth  = 0x01, // Width has been already parsed (implicit in CFF2 mode).
  kCSFlagPathOpen  = 0x02  // Path is open (set after the first 'MoveTo').
};

enum CSOpCode : uint32_t {
  // We use the same notation as used by ADOBE specifications:
  //
  //   |- at the beginning means the beginning (bottom) of the stack.
  //   |- at the end means stack-cleaning operator.
  //    - at the end means to pop stack by one.
  //
  // CFF Version 1
  // -------------
  //
  // The first stack-clearing operator, which must be one of 'MoveTo', 'Stem', 'Hint', or 'EndChar', takes an
  // additional argument - the width, which may be expressed as zero or one numeric argument.
  //
  // CFF Version 2
  // -------------
  //
  // The concept of "width" specified in the program was removed. Arithmetic and Conditional operators were also
  // removed and control flow operators like 'Return' and 'EndChar' were made implicit and were removed as well.

  // Core Operators / Escapes:
  kCSOpEscape     = 0x000C,
  kCSOpPushI16    = 0x001C,
  kCSOpPushF16x16 = 0x00FF,

  // Path Construction Operators:
  kCSOpRMoveTo    = 0x0015, // CFFv*: |- dx1 dy1         rmoveto (21) |-
  kCSOpHMoveTo    = 0x0016, // CFFv*: |- dx1             hmoveto (22) |-
  kCSOpVMoveTo    = 0x0004, // CFFv*: |- dy1             vmoveto (4)  |-
  kCSOpRLineTo    = 0x0005, // CFFv*: |- {dxa dya}+      rlineto (5)  |-
  kCSOpHLineTo    = 0x0006, // CFFv*: |- dx1 {dya dxb}*  hlineto (6)  |-   or   |- {dxa dyb}+    hlineto    (6)  |-
  kCSOpVLineTo    = 0x0007, // CFFv*: |- dy1 {dxa dyb}*  vlineto (7)  |-   or   |- {dya dxb}+    vlineto    (7)  |-

  kCSOpRRCurveTo  = 0x0008, // CFFv*: |-                 {dxa dya dxb dyb dxc dyc}+              rrcurveto  (8)  |-
  kCSOpVVCurveTo  = 0x001A, // CFFv*: |- dx1?            {dya dxb dyb dyc}+                      vvcurveto  (26) |-
  kCSOpHHCurveTo  = 0x001B, // CFFv*: |- dy1?            {dxa dxb dyb dxc}+                      hhcurveto  (27) |-
  kCSOpVHCurveTo  = 0x001E, // CFFv*: |- dy1 dx2 dy2 dx3 {dxa dxb dyb dyc dyd dxe dye dxf}* dyf? vhcurveto  (30) |-
                            // CFFv*: |-                 {dya dxb dyb dxc dxd dxe dye dyf}+ dxf? vhcurveto  (30) |-
  kCSOpHVCurveTo  = 0x001F, // CFFv*: |- dx1 dx2 dy2 dy3 {dya dxb dyb dxc dxd dxe dye dyf}* dxf? hvcurveto  (31) |-
                            // CFFv*: |-                 {dxa dxb dyb dyc dyd dxe dye dxf}+ dyf? hvcurveto  (31) |-
  kCSOpRCurveLine = 0x0018, // CFFv*: |-                 {dxa dya dxb dyb dxc dyc}+ dxd dyd      rcurveline (24) |-
  kCSOpRLineCurve = 0x0019, // CFFv*: |-                 {dxa dya}+ dxb dyb dxc dyc dxd dyd      rlinecurve (25) |-

  kCSOpFlex       = 0x0C23, // CFFv*: |- dx1 dy1 dx2 dy2 dx3 dy3 dx4 dy4 dx5 dy5 dx6 dy6 fd      flex    (12 35) |-
  kCSOpFlex1      = 0x0C25, // CFFv*: |- dx1 dy1 dx2 dy2 dx3 dy3 dx4 dy4 dx5 dy5 d6              flex1   (12 37) |-
  kCSOpHFlex      = 0x0C22, // CFFv*: |- dx1 dx2 dy2 dx3 dx4 dx5 dx6                             hflex   (12 34) |-
  kCSOpHFlex1     = 0x0C24, // CFFv*: |- dx1 dy1 dx2 dy2 dx3 dx4 dx5 dy5 dx6                     hflex1  (12 36) |-

  // Hint Operators:
  kCSOpHStem      = 0x0001, // CFFv*: |- y dy {dya dyb}* hstem     (1)       |-
  kCSOpVStem      = 0x0003, // CFFv*: |- x dx {dxa dxb}* vstem     (3)       |-
  kCSOpHStemHM    = 0x0012, // CFFv*: |- y dy {dya dyb}* hstemhm   (18)      |-
  kCSOpVStemHM    = 0x0017, // CFFv*: |- x dx {dxa dxb}* vstemhm   (23)      |-
  kCSOpHintMask   = 0x0013, // CFFv*: |-                 hintmask  (19) mask |-
  kCSOpCntrMask   = 0x0014, // CFFv*: |-                 cntrmask  (20) mask |-

  // Variation Data Operators:
  kCSOpVSIndex    = 0x000F, // CFFv2: |- ivs vsindex (15) |-
  kCSOpBlend      = 0x0010, // CFFv2: in(0)...in(N-1), d(0,0)...d(K-1,0), d(0,1)...d(K-1,1) ... d(0,N-1)...d(K-1,N-1) N blend (16) out(0)...(N-1)

  // Control Flow Operators:
  kCSOpCallLSubR  = 0x000A, // CFFv*:          lsubr# calllsubr (10) -
  kCSOpCallGSubR  = 0x001D, // CFFv*:          gsubr# callgsubr (29) -
  kCSOpReturn     = 0x000B, // CFFv1:                 return    (11)
  kCSOpEndChar    = 0x000E, // CFFv1:                 endchar   (14)

  // Conditional & Arithmetic Operators (CFFv1 only!):
  kCSOpAnd        = 0x0C03, // CFFv1: in1 in2         and    (12 3)  out {in1 && in2}
  kCSOpOr         = 0x0C04, // CFFv1: in1 in2         or     (12 4)  out {in1 || in2}
  kCSOpEq         = 0x0C0F, // CFFv1: in1 in2         eq     (12 15) out {in1 == in2}
  kCSOpIfElse     = 0x0C16, // CFFv1: s1 s2 v1 v2     ifelse (12 22) out {v1 <= v2 ? s1 : s2}
  kCSOpNot        = 0x0C05, // CFFv1: in              not    (12 5)  out {!in}
  kCSOpNeg        = 0x0C0E, // CFFv1: in              neg    (12 14) out {-in}
  kCSOpAbs        = 0x0C09, // CFFv1: in              abs    (12 9)  out {abs(in)}
  kCSOpSqrt       = 0x0C1A, // CFFv1: in              sqrt   (12 26) out {sqrt(in)}
  kCSOpAdd        = 0x0C0A, // CFFv1: in1 in2         add    (12 10) out {in1 + in2}
  kCSOpSub        = 0x0C0B, // CFFv1: in1 in2         sub    (12 11) out {in1 - in2}
  kCSOpMul        = 0x0C18, // CFFv1: in1 in2         mul    (12 24) out {in1 * in2}
  kCSOpDiv        = 0x0C0C, // CFFv1: in1 in2         div    (12 12) out {in1 / in2}
  kCSOpRandom     = 0x0C17, // CFFv1:                 random (12 23) out
  kCSOpDup        = 0x0C1B, // CFFv1: in              dup    (12 27) out out
  kCSOpDrop       = 0x0C12, // CFFv1: in              drop   (12 18)
  kCSOpExch       = 0x0C1C, // CFFv1: in1 in2         exch   (12 28) out1 out2
  kCSOpIndex      = 0x0C1D, // CFFv1: nX...n0 I       index  (12 29) nX...n0 n[I]
  kCSOpRoll       = 0x0C1E, // CFFv1: n(N–1)...n0 N J roll   (12 30) n((J–1) % N)...n0 n(N–1)...n(J % N)

  // Storage Operators (CFFv1 only!):
  kCSOpPut        = 0x0C14, // CFFv1: in I put (12 20)
  kCSOpGet        = 0x0C15  // CFFv1:    I get (12 21) out
};

// bl::OpenType::CFFImpl - ExecutionFeaturesInfo
// =============================================

//! Describes features that can be used during execution and their requirements.
//!
//! There are two versions of `ExecutionFeaturesInfo` selected at runtime based on the font - either CFF or CFF2.
//! CFF provides some operators that are hardly used in fonts. CFF2 removed such operators and introduced new ones
//! that are used to support "OpenType Font Variations" feature.
//!
//! Both CFF and CFF2 specifications state that unsupported operators should be skipped and value stack cleared.
//! This is implemented by assigning `kUnknown` to all operators that are unsupported. The value is much higher
//! than a possible value stack size so when it's used it would always force the engine to decide between an
//! unsupported operator or operator that was called with less operands than it needs (in that case the execution
//! is terminated immediately).
struct ExecutionFeaturesInfo {
  static constexpr const uint32_t kBaseOpCount = 32;
  static constexpr const uint32_t kEscapedOpCount = 48;
  static constexpr const uint16_t kUnknown = 0xFFFFu;

  //! Stack size required to process a base operator.
  LookupTable<uint16_t, kBaseOpCount> base_op_stack_size;
  //! Stack size required to process an escaped operator.
  LookupTable<uint16_t, kEscapedOpCount> escaped_op_stack_size;
};

template<uint32_t Escape, uint32_t V>
struct ExecutionFeaturesInfoOpStackSizeGen {
  static constexpr uint16_t value(size_t op) noexcept {
    return ((op | Escape) == kCSOpEscape           ) ? uint16_t(0)
         : ((op | Escape) == kCSOpPushI16          ) ? uint16_t(0)

         : ((op | Escape) == kCSOpRMoveTo          ) ? uint16_t(2)
         : ((op | Escape) == kCSOpHMoveTo          ) ? uint16_t(1)
         : ((op | Escape) == kCSOpVMoveTo          ) ? uint16_t(1)
         : ((op | Escape) == kCSOpRLineTo          ) ? uint16_t(2)
         : ((op | Escape) == kCSOpHLineTo          ) ? uint16_t(1)
         : ((op | Escape) == kCSOpVLineTo          ) ? uint16_t(1)
         : ((op | Escape) == kCSOpRRCurveTo        ) ? uint16_t(6)
         : ((op | Escape) == kCSOpHHCurveTo        ) ? uint16_t(4)
         : ((op | Escape) == kCSOpVVCurveTo        ) ? uint16_t(4)
         : ((op | Escape) == kCSOpVHCurveTo        ) ? uint16_t(4)
         : ((op | Escape) == kCSOpHVCurveTo        ) ? uint16_t(4)
         : ((op | Escape) == kCSOpRCurveLine       ) ? uint16_t(8)
         : ((op | Escape) == kCSOpRLineCurve       ) ? uint16_t(8)

         : ((op | Escape) == kCSOpFlex             ) ? uint16_t(13)
         : ((op | Escape) == kCSOpFlex1            ) ? uint16_t(11)
         : ((op | Escape) == kCSOpHFlex            ) ? uint16_t(7)
         : ((op | Escape) == kCSOpHFlex1           ) ? uint16_t(9)

         : ((op | Escape) == kCSOpHStem            ) ? uint16_t(2)
         : ((op | Escape) == kCSOpVStem            ) ? uint16_t(2)
         : ((op | Escape) == kCSOpHStemHM          ) ? uint16_t(2)
         : ((op | Escape) == kCSOpVStemHM          ) ? uint16_t(2)
         : ((op | Escape) == kCSOpHintMask         ) ? uint16_t(0)
         : ((op | Escape) == kCSOpCntrMask         ) ? uint16_t(0)

         : ((op | Escape) == kCSOpCallLSubR        ) ? uint16_t(1)
         : ((op | Escape) == kCSOpCallGSubR        ) ? uint16_t(1)
         : ((op | Escape) == kCSOpReturn  && V == 1) ? uint16_t(0)
         : ((op | Escape) == kCSOpEndChar && V == 1) ? uint16_t(0)

         : ((op | Escape) == kCSOpVSIndex && V == 2) ? uint16_t(1)
         : ((op | Escape) == kCSOpBlend   && V == 2) ? uint16_t(1)

         : ((op | Escape) == kCSOpAnd     && V == 1) ? uint16_t(2)
         : ((op | Escape) == kCSOpOr      && V == 1) ? uint16_t(2)
         : ((op | Escape) == kCSOpEq      && V == 1) ? uint16_t(2)
         : ((op | Escape) == kCSOpIfElse  && V == 1) ? uint16_t(4)
         : ((op | Escape) == kCSOpNot     && V == 1) ? uint16_t(1)
         : ((op | Escape) == kCSOpNeg     && V == 1) ? uint16_t(1)
         : ((op | Escape) == kCSOpAbs     && V == 1) ? uint16_t(1)
         : ((op | Escape) == kCSOpSqrt    && V == 1) ? uint16_t(1)
         : ((op | Escape) == kCSOpAdd     && V == 1) ? uint16_t(2)
         : ((op | Escape) == kCSOpSub     && V == 1) ? uint16_t(2)
         : ((op | Escape) == kCSOpMul     && V == 1) ? uint16_t(2)
         : ((op | Escape) == kCSOpDiv     && V == 1) ? uint16_t(2)
         : ((op | Escape) == kCSOpRandom  && V == 1) ? uint16_t(0)
         : ((op | Escape) == kCSOpDup     && V == 1) ? uint16_t(1)
         : ((op | Escape) == kCSOpDrop    && V == 1) ? uint16_t(1)
         : ((op | Escape) == kCSOpExch    && V == 1) ? uint16_t(2)
         : ((op | Escape) == kCSOpIndex   && V == 1) ? uint16_t(2)
         : ((op | Escape) == kCSOpRoll    && V == 1) ? uint16_t(2)
         : ((op | Escape) == kCSOpPut     && V == 1) ? uint16_t(2)
         : ((op | Escape) == kCSOpGet     && V == 1) ? uint16_t(1)

         : ExecutionFeaturesInfo::kUnknown;
  }
};

static constexpr const ExecutionFeaturesInfo execution_features_info[2] = {
  // CFFv1 [Index #0]
  {
    make_lookup_table<uint16_t, ExecutionFeaturesInfo::kBaseOpCount   , ExecutionFeaturesInfoOpStackSizeGen<0x0000, 1>>(),
    make_lookup_table<uint16_t, ExecutionFeaturesInfo::kEscapedOpCount, ExecutionFeaturesInfoOpStackSizeGen<0x0C00, 1>>()
  },

  // CFFv2 [Index #1]
  {
    make_lookup_table<uint16_t, ExecutionFeaturesInfo::kBaseOpCount   , ExecutionFeaturesInfoOpStackSizeGen<0x0000, 2>>(),
    make_lookup_table<uint16_t, ExecutionFeaturesInfo::kEscapedOpCount, ExecutionFeaturesInfoOpStackSizeGen<0x0C00, 2>>()
  }
};

// bl::OpenType::CFFImpl - ExecutionState
// ======================================

//! Execution state is used in a call-stack array to remember from where a subroutine was called. When a subroutine
//! reaches the end of a "Return" opcode it would pop the state from call-stack and return the execution after the
//! "CallLSubR" or "CallGSubR" instruction.
struct ExecutionState {
  BL_INLINE void reset(const uint8_t* ptr, const uint8_t* end) noexcept {
    _ptr = ptr;
    _end = end;
  }

  const uint8_t* _ptr;
  const uint8_t* _end;
};

// bl::OpenType::CFFImpl - Matrix2x2
// =================================

struct Matrix2x2 {
  BL_INLINE double x_by_a(double x, double y) const noexcept { return x * m00 + y * m10; }
  BL_INLINE double y_by_a(double x, double y) const noexcept { return x * m01 + y * m11; }

  BL_INLINE double x_by_x(double x) const noexcept { return x * m00; }
  BL_INLINE double x_by_y(double y) const noexcept { return y * m10; }

  BL_INLINE double y_by_x(double x) const noexcept { return x * m01; }
  BL_INLINE double y_by_y(double y) const noexcept { return y * m11; }

  double m00, m01;
  double m10, m11;
};

// bl::OpenType::CFFImpl - Trace
// =============================

#ifdef BL_TRACE_OT_CFF
static void trace_char_string_op(const OTFaceImpl* ot_face_impl, Trace& trace, uint32_t op, const double* values, size_t count) noexcept {
  char buf[64];
  const char* op_name = "";

  switch (op) {
    #define CASE(op) case kCSOp##op: op_name = #op; break

    CASE(Escape);
    CASE(PushI16);
    CASE(PushF16x16);

    CASE(RMoveTo);
    CASE(HMoveTo);
    CASE(VMoveTo);
    CASE(RLineTo);
    CASE(HLineTo);
    CASE(VLineTo);
    CASE(RRCurveTo);
    CASE(HHCurveTo);
    CASE(HVCurveTo);
    CASE(VHCurveTo);
    CASE(VVCurveTo);
    CASE(RCurveLine);
    CASE(RLineCurve);
    CASE(Flex);
    CASE(Flex1);
    CASE(HFlex);
    CASE(HFlex1);

    CASE(HStem);
    CASE(VStem);
    CASE(HStemHM);
    CASE(VStemHM);
    CASE(HintMask);
    CASE(CntrMask);

    CASE(CallLSubR);
    CASE(CallGSubR);
    CASE(Return);
    CASE(EndChar);

    CASE(VSIndex);
    CASE(Blend);

    CASE(And);
    CASE(Or);
    CASE(Eq);
    CASE(IfElse);
    CASE(Not);
    CASE(Neg);
    CASE(Abs);
    CASE(Sqrt);
    CASE(Add);
    CASE(Sub);
    CASE(Mul);
    CASE(Div);
    CASE(Random);
    CASE(Drop);
    CASE(Exch);
    CASE(Index);
    CASE(Roll);
    CASE(Dup);

    CASE(Put);
    CASE(Get);

    #undef CASE

    default:
      snprintf(buf, BL_ARRAY_SIZE(buf), "Op #%04X", op);
      op_name = buf;
      break;
  }

  trace.info("%s", op_name);
  if (count) {
    trace.out(" [");
    for (size_t i = 0; i < count; i++)
      trace.out(i == 0 ? "%g" : " %g", values[i]);
    trace.out("]");
  }

  if (count > 0 && (op == kCSOpCallGSubR || op == kCSOpCallLSubR)) {
    int32_t idx = int32_t(values[count - 1]);
    idx += face_impl->cff.index[op == kCSOpCallLSubR ? CFFData::kIndexLSubR : CFFData::kIndexGSubR].bias;
    trace.out(" {SubR #%d}", idx);
  }

  trace.out("\n");
}
#endif

// bl::OpenType::CFFImpl - Interpreter
// ===================================

static BL_INLINE bool find_glyph_in_range3(BLGlyphId glyph_id, const uint8_t* ranges, size_t n_ranges, uint32_t& fd) noexcept {
  constexpr size_t kRangeSize = 3;
  for (size_t i = n_ranges; i != 0; i >>= 1) {
    const uint8_t* half = ranges + (i >> 1) * kRangeSize;

    // Read either the next Range3[] record or sentinel.
    uint32_t gEnd = MemOps::readU16uBE(half + kRangeSize);

    if (glyph_id >= gEnd) {
      ranges = half + kRangeSize;
      i--;
      continue;
    }

    uint32_t gStart = MemOps::readU16uBE(half);
    if (glyph_id < gStart)
      continue;

    fd = half[2]; // Read `Range3::fd`.
    return true;
  }

  return false;
}

template<typename Consumer>
static BLResult get_glyph_outlines_t(
  const BLFontFaceImpl* face_impl,
  BLGlyphId glyph_id,
  const BLMatrix2D* transform,
  Consumer& consumer,
  ScopedBuffer* tmp_buffer) noexcept {

  bl_unused(tmp_buffer);
  const OTFaceImpl* ot_face_impl = static_cast<const OTFaceImpl*>(face_impl);

  // Will only do something if tracing is enabled.
  Trace trace;
  trace.info("bl::OpenType::CFFImpl::DecodeGlyph #%u\n", glyph_id);
  trace.indent();

  // --------------------------------------------------------------------------
  // [Prepare for Execution]
  // --------------------------------------------------------------------------

  const uint8_t* ip     = nullptr;             // Pointer in the instruction array.
  const uint8_t* ip_end = nullptr;             // End of the instruction array.

  ExecutionState c_buf[kCFFCallStackSize + 1]; // Call stack.
  double v_buf[kCFFValueStackSizeV1 + 1];      // Value stack.

  uint32_t c_idx = 0;                          // Call stack index.
  uint32_t v_idx = 0;                          // Value stack index.

  double s_buf[kCFFStorageSize + 1];           // Storage (get/put).
  uint32_t s_msk = 0;                          // Mask that contains which indexes in `s_buf` are used.
  s_buf[kCFFStorageSize] = 0.0;                // Only the last item is set to zero, used for out-of-range expressions.

  size_t bytes_processed = 0;                  // Bytes processed, increasing counter.
  uint32_t hint_bit_count = 0;                 // Number of bits required by 'HintMask' and 'CntrMask' operators.
  uint32_t execution_flags = 0;                // Execution status flags.
  uint32_t v_min_operands = 0;                 // Minimum operands the current opcode requires (updated per opcode).

  double px = transform->m20;                  // Current X coordinate.
  double py = transform->m21;                  // Current Y coordinate.

  const CFFData& cff_info = ot_face_impl->cff;
  const uint8_t* cff_data = ot_face_impl->cff.table.data;

  // Execution features describe either CFFv1 or CFFv2 environment. It contains minimum operand count for each
  // opcode (or operator) and some other data.
  const ExecutionFeaturesInfo* execution_features = &execution_features_info[0];

  // This is used to perform a function (subroutine) call. Initially we set it to the charstring referenced by the
  // `glyph_id`. Later, when we process a function call opcode it would be changed to either GSubR or LSubR index.
  const CFFData::IndexData* subr_index = &cff_info.index[CFFData::kIndexCharString];
  uint32_t subr_id = glyph_id;

  // We really want to report a correct error when we face an invalid glyph_id, this is the only difference between
  // handling a function call and handling the initial CharString program.
  if (BL_UNLIKELY(glyph_id >= subr_index->entry_count)) {
    trace.fail("Invalid Glyph ID\n");
    return bl_make_error(BL_ERROR_INVALID_GLYPH);
  }

  // LSubR index that will be used by CallLSubR operator. CID fonts provide multiple indexes that can be used based
  // on `glyph_id`.
  const CFFData::IndexData* local_subr_index = &cff_info.index[CFFData::kIndexLSubR];
  if (cff_info.fd_select_offset) {
    // We are not interested in format byte, we already know the format.
    size_t fd_select_offset = cff_info.fd_select_offset + 1;

    const uint8_t* fd_data = cff_data + fd_select_offset;
    size_t fd_data_size = cff_info.table.size - fd_select_offset;

    // There are only two formats - 0 and 3.
    uint32_t fd = 0xFFFFFFFFu;
    if (cff_info.fd_select_format == 0) {
      // Format 0:
      //   UInt8 format;
      //   UInt8 fds[n_glyphs];
      if (glyph_id < fd_data_size)
        fd = fd_data[glyph_id];
    }
    else {
      // Format 3:
      //   UInt8 format;
      //   UInt16 n_ranges;
      //   struct Range3 {
      //     UInt16 first;
      //     UInt8 id;
      //   } ranges[n_ranges];
      //   UInt16 sentinel;
      if (fd_data_size >= 2) {
        uint32_t n_ranges = MemOps::readU16uBE(fd_data);
        if (fd_data_size >= 2u + n_ranges * 3u + 2u)
          find_glyph_in_range3(glyph_id, fd_data + 2u, n_ranges, fd);
      }
    }

    if (fd < ot_face_impl->cff_fd_subr_indexes.size()) {
      local_subr_index = &ot_face_impl->cff_fd_subr_indexes[fd];
    }
  }

  // Compiler can better optimize the transform if it knows that it won't be changed outside of this function.
  Matrix2x2 m { transform->m00, transform->m01, transform->m10, transform->m11 };

  // Program | SubR - Init
  // ---------------------

  BL_PROPAGATE(consumer.begin(64));

OnSubRCall:
  {
    uint32_t offset_size = subr_index->offset_size;
    uint32_t payload_size = subr_index->payload_size();

    ip = cff_data + subr_index->data_range.offset;

    uint32_t o_array[2];
    read_offset_array(ip + subr_index->offsets_offset() + subr_id * offset_size, offset_size, o_array, 2);

    ip += subr_index->payload_offset();
    ip_end = ip;

    o_array[0] -= CFFTable::kOffsetAdjustment;
    o_array[1] -= CFFTable::kOffsetAdjustment;

    if (BL_UNLIKELY(o_array[0] >= o_array[1] || o_array[1] > payload_size)) {
      trace.fail("Invalid SubR range [Start=%u End=%u Max=%u]\n", o_array[0], o_array[1], payload_size);
      return bl_make_error(BL_ERROR_INVALID_DATA);
    }

    ip     += o_array[0];
    ip_end += o_array[1];

    size_t program_size = o_array[1] - o_array[0];
    if (BL_UNLIKELY(kCFFProgramLimit - bytes_processed < program_size)) {
      trace.fail("Program limit exceeded [%zu bytes processed]\n", bytes_processed);
      return bl_make_error(BL_ERROR_FONT_PROGRAM_TERMINATED);
    }
    bytes_processed += program_size;
  }

  // Program | SubR - Execute
  // ------------------------

  for (;;) {
    // Current opcode read from `ip`.
    uint32_t b0;

    if (BL_UNLIKELY(ip >= ip_end)) {
      // CFF vs CFF2 diverged a bit. CFF2 doesn't require 'Return' and 'EndChar'
      // operators and made them implicit. When we reach the end of the current
      // subroutine then a 'Return' is implied, similarly when we reach the end
      // of the current CharString 'EndChar' is implied as well.
      if (c_idx > 0)
        goto OnReturn;
      break;
    }

    // Read the opcode byte.
    b0 = *ip++;

    if (b0 >= 32) {
      if (BL_UNLIKELY(++v_idx > kCFFValueStackSizeV1)) {
        goto InvalidData;
      }
      else {
        // Push Number (Small)
        // -------------------

        if (ip < ip_end) {
          if (b0 <= 246) {
            // Number in range [-107..107].
            int v = int(b0) - 139;
            v_buf[v_idx - 1] = double(v);

            // There is a big chance that there would be another number. If it's
            // true then this acts as 2x unrolled push. If not then we perform
            // a direct jump to handle the operator as we would have done anyway.
            b0 = *ip++;
            if (b0 < 32)
              goto OnOperator;

            if (BL_UNLIKELY(++v_idx > kCFFValueStackSizeV1))
              goto InvalidData;

            if (b0 <= 246) {
              v = int(b0) - 139;
              v_buf[v_idx - 1] = double(v);
              continue;
            }

            if (ip == ip_end)
              goto InvalidData;
          }

          if (b0 <= 254) {
            // Number in range [-1131..-108] or [108..1131].
            uint32_t b1 = *ip++;
            int v = b0 <= 250 ? (108 - 247 * 256) + int(b0 * 256 + b1)
                              : (251 * 256 - 108) - int(b0 * 256 + b1);

            v_buf[v_idx - 1] = double(v);
          }
          else {
            // Number encoded as 16x16 fixed-point.
            BL_ASSERT(b0 == kCSOpPushF16x16);

            ip += 4;
            if (BL_UNLIKELY(ip > ip_end))
              goto InvalidData;

            int v = MemOps::readI32uBE(ip - 4);
            v_buf[v_idx - 1] = double(v) * kCFFDoubleFromF16x16;
          }
          continue;
        }
        else {
          // If this is the end of the program the number must be in range [-107..107].
          if (b0 > 246)
            goto InvalidData;

          // Number in range [-107..107].
          int v = int(b0) - 139;
          v_buf[v_idx - 1] = double(v);
          continue;
        }
      }
    }
    else {
OnOperator:
      #ifdef BL_TRACE_OT_CFF
      trace_char_string_op(ot_face_impl, trace, b0, v_buf, v_idx);
      #endif

      v_min_operands = execution_features->base_op_stack_size[b0];
      if (BL_UNLIKELY(v_idx < v_min_operands)) {
        // If this is not an unknown operand it would mean that we have less
        // values on stack than the operator requires. That's an error in CS.
        if (v_min_operands != ExecutionFeaturesInfo::kUnknown)
          goto InvalidData;

        // Unknown operators should clear the stack and act as NOPs.
        v_idx = 0;
        continue;
      }

      switch (b0) {
        // Push Number (2's Complement Int16)
        // ----------------------------------

        case kCSOpPushI16: {
          ip += 2;
          if (BL_UNLIKELY(ip > ip_end || ++v_idx > kCFFValueStackSizeV1))
            goto InvalidData;

          int v = MemOps::readI16uBE(ip - 2);
          v_buf[v_idx - 1] = double(v);
          continue;
        }

        // MoveTo
        // ------

        // |- dx1 dy1 rmoveto (21) |-
        case kCSOpRMoveTo: {
          BL_ASSERT(v_min_operands >= 2);
          BL_PROPAGATE(consumer.ensure(2));

          if (execution_flags & kCSFlagPathOpen)
            consumer.close();

          px += m.x_by_a(v_buf[v_idx - 2], v_buf[v_idx - 1]);
          py += m.y_by_a(v_buf[v_idx - 2], v_buf[v_idx - 1]);
          consumer.move_to(px, py);

          v_idx = 0;
          execution_flags |= kCSFlagHasWidth | kCSFlagPathOpen;
          continue;
        }

        // |- dx1 hmoveto (22) |-
        case kCSOpHMoveTo: {
          BL_ASSERT(v_min_operands >= 1);
          BL_PROPAGATE(consumer.ensure(2));

          if (execution_flags & kCSFlagPathOpen)
            consumer.close();

          px += m.x_by_x(v_buf[v_idx - 1]);
          py += m.y_by_x(v_buf[v_idx - 1]);
          consumer.move_to(px, py);

          v_idx = 0;
          execution_flags |= kCSFlagHasWidth | kCSFlagPathOpen;
          continue;
        }

        // |- dy1 vmoveto (4) |-
        case kCSOpVMoveTo: {
          BL_ASSERT(v_min_operands >= 1);
          BL_PROPAGATE(consumer.ensure(2));

          if (execution_flags & kCSFlagPathOpen)
            consumer.close();

          px += m.x_by_y(v_buf[v_idx - 1]);
          py += m.y_by_y(v_buf[v_idx - 1]);
          consumer.move_to(px, py);

          v_idx = 0;
          execution_flags |= kCSFlagHasWidth | kCSFlagPathOpen;
          continue;
        }

        // LineTo
        // ------

        // |- {dxa dya}+ rlineto (5) |-
        case kCSOpRLineTo: {
          BL_ASSERT(v_min_operands >= 2);
          BL_PROPAGATE(consumer.ensure((v_idx + 1) / 2u));

          // NOTE: The specification talks about a pair of numbers, however,
          // other implementations like FreeType allow odd number of arguments
          // implicitly adding zero as the last one argument missing... It's a
          // specification violation that we follow for compatibility reasons.
          size_t i = 0;
          while ((i += 2) <= v_idx) {
            px += m.x_by_a(v_buf[i - 2], v_buf[i - 1]);
            py += m.y_by_a(v_buf[i - 2], v_buf[i - 1]);
            consumer.line_to(px, py);
          }

          if (v_idx & 1) {
            px += m.x_by_x(v_buf[v_idx - 1]);
            py += m.y_by_x(v_buf[v_idx - 1]);
            consumer.line_to(px, py);
          }

          v_idx = 0;
          continue;
        }

        // |- dx1 {dya dxb}* hlineto (6) |- or |- {dxa dyb}+ hlineto (6) |-
        // |- dy1 {dxa dyb}* vlineto (7) |- or |- {dya dxb}+ vlineto (7) |-
        case kCSOpHLineTo:
        case kCSOpVLineTo: {
          BL_ASSERT(v_min_operands >= 1);
          BL_PROPAGATE(consumer.ensure(v_idx));

          size_t i = 0;
          if (b0 == kCSOpVLineTo)
            goto OnVLineTo;

          for (;;) {
            px += m.x_by_x(v_buf[i]);
            py += m.y_by_x(v_buf[i]);
            consumer.line_to(px, py);

            if (++i >= v_idx)
              break;
OnVLineTo:
            px += m.x_by_y(v_buf[i]);
            py += m.y_by_y(v_buf[i]);
            consumer.line_to(px, py);

            if (++i >= v_idx)
              break;
          }

          v_idx = 0;
          continue;
        }

        // CurveTo
        // -------

        // |- {dxa dya dxb dyb dxc dyc}+ rrcurveto (8) |-
        case kCSOpRRCurveTo: {
          BL_ASSERT(v_min_operands >= 6);
          BL_PROPAGATE(consumer.ensure(v_idx / 2u));

          size_t i = 0;
          double x1, y1, x2, y2;

          while ((i += 6) <= v_idx) {
            x1 = px + m.x_by_a(v_buf[i - 6], v_buf[i - 5]);
            y1 = py + m.y_by_a(v_buf[i - 6], v_buf[i - 5]);
            x2 = x1 + m.x_by_a(v_buf[i - 4], v_buf[i - 3]);
            y2 = y1 + m.y_by_a(v_buf[i - 4], v_buf[i - 3]);
            px = x2 + m.x_by_a(v_buf[i - 2], v_buf[i - 1]);
            py = y2 + m.y_by_a(v_buf[i - 2], v_buf[i - 1]);
            consumer.cubic_to(x1, y1, x2, y2, px, py);
          }

          v_idx = 0;
          continue;
        }

        // |- dy1 dx2 dy2 dx3 {dxa dxb dyb dyc dyd dxe dye dxf}* dyf? vhcurveto (30) |- or |- {dya dxb dyb dxc dxd dxe dye dyf}+ dxf? vhcurveto (30) |-
        // |- dx1 dx2 dy2 dy3 {dya dxb dyb dxc dxd dxe dye dyf}* dxf? hvcurveto (31) |- or |- {dxa dxb dyb dyc dyd dxe dye dxf}+ dyf? hvcurveto (31) |-
        case kCSOpVHCurveTo:
        case kCSOpHVCurveTo: {
          BL_ASSERT(v_min_operands >= 4);
          BL_PROPAGATE(consumer.ensure(v_idx));

          size_t i = 0;
          double x1, y1, x2, y2;

          if (b0 == kCSOpVHCurveTo)
            goto OnVHCurveTo;

          while ((i += 4) <= v_idx) {
            x1 = px + m.x_by_x(v_buf[i - 4]);
            y1 = py + m.y_by_x(v_buf[i - 4]);
            x2 = x1 + m.x_by_a(v_buf[i - 3], v_buf[i - 2]);
            y2 = y1 + m.y_by_a(v_buf[i - 3], v_buf[i - 2]);
            px = x2 + m.x_by_y(v_buf[i - 1]);
            py = y2 + m.y_by_y(v_buf[i - 1]);

            if (v_idx - i == 1) {
              px += m.x_by_x(v_buf[i]);
              py += m.y_by_x(v_buf[i]);
            }
            consumer.cubic_to(x1, y1, x2, y2, px, py);
OnVHCurveTo:
            if ((i += 4) > v_idx)
              break;

            x1 = px + m.x_by_y(v_buf[i - 4]);
            y1 = py + m.y_by_y(v_buf[i - 4]);
            x2 = x1 + m.x_by_a(v_buf[i - 3], v_buf[i - 2]);
            y2 = y1 + m.y_by_a(v_buf[i - 3], v_buf[i - 2]);
            px = x2 + m.x_by_x(v_buf[i - 1]);
            py = y2 + m.y_by_x(v_buf[i - 1]);

            if (v_idx - i == 1) {
              px += m.x_by_y(v_buf[i]);
              py += m.y_by_y(v_buf[i]);
            }
            consumer.cubic_to(x1, y1, x2, y2, px, py);
          }

          v_idx = 0;
          continue;
        }

        // |- dy1? {dxa dxb dyb dxc}+ hhcurveto (27) |-
        case kCSOpHHCurveTo: {
          BL_ASSERT(v_min_operands >= 4);
          BL_PROPAGATE(consumer.ensure(v_idx));

          size_t i = 0;
          double x1, y1, x2, y2;

          // Odd argument case.
          if (v_idx & 0x1) {
            px += m.x_by_y(v_buf[i]);
            py += m.y_by_y(v_buf[i]);
            i++;
          }

          while ((i += 4) <= v_idx) {
            x1 = px + m.x_by_x(v_buf[i - 4]);
            y1 = py + m.y_by_x(v_buf[i - 4]);
            x2 = x1 + m.x_by_a(v_buf[i - 3], v_buf[i - 2]);
            y2 = y1 + m.y_by_a(v_buf[i - 3], v_buf[i - 2]);
            px = x2 + m.x_by_x(v_buf[i - 1]);
            py = y2 + m.y_by_x(v_buf[i - 1]);
            consumer.cubic_to(x1, y1, x2, y2, px, py);
          }

          v_idx = 0;
          continue;
        }

        // |- dx1? {dya dxb dyb dyc}+ vvcurveto (26) |-
        case kCSOpVVCurveTo: {
          BL_ASSERT(v_min_operands >= 4);
          BL_PROPAGATE(consumer.ensure(v_idx));

          size_t i = 0;
          double x1, y1, x2, y2;

          // Odd argument case.
          if (v_idx & 0x1) {
            px += m.x_by_x(v_buf[i]);
            py += m.y_by_x(v_buf[i]);
            i++;
          }

          while ((i += 4) <= v_idx) {
            x1 = px + m.x_by_y(v_buf[i - 4]);
            y1 = py + m.y_by_y(v_buf[i - 4]);
            x2 = x1 + m.x_by_a(v_buf[i - 3], v_buf[i - 2]);
            y2 = y1 + m.y_by_a(v_buf[i - 3], v_buf[i - 2]);
            px = x2 + m.x_by_y(v_buf[i - 1]);
            py = y2 + m.y_by_y(v_buf[i - 1]);
            consumer.cubic_to(x1, y1, x2, y2, px, py);
          }

          v_idx = 0;
          continue;
        }

        // |- {dxa dya dxb dyb dxc dyc}+ dxd dyd rcurveline (24) |-
        case kCSOpRCurveLine: {
          BL_ASSERT(v_min_operands >= 8);
          BL_PROPAGATE(consumer.ensure(v_idx / 2u));

          size_t i = 0;
          double x1, y1, x2, y2;

          v_idx -= 2;
          while ((i += 6) <= v_idx) {
            x1 = px + m.x_by_a(v_buf[i - 6], v_buf[i - 5]);
            y1 = py + m.y_by_a(v_buf[i - 6], v_buf[i - 5]);
            x2 = x1 + m.x_by_a(v_buf[i - 4], v_buf[i - 3]);
            y2 = y1 + m.y_by_a(v_buf[i - 4], v_buf[i - 3]);
            px = x2 + m.x_by_a(v_buf[i - 2], v_buf[i - 1]);
            py = y2 + m.y_by_a(v_buf[i - 2], v_buf[i - 1]);
            consumer.cubic_to(x1, y1, x2, y2, px, py);
          }

          px += m.x_by_a(v_buf[v_idx + 0], v_buf[v_idx + 1]);
          py += m.y_by_a(v_buf[v_idx + 0], v_buf[v_idx + 1]);
          consumer.line_to(px, py);

          v_idx = 0;
          continue;
        }

        // |- {dxa dya}+ dxb dyb dxc dyc dxd dyd rlinecurve (25) |-
        case kCSOpRLineCurve: {
          BL_ASSERT(v_min_operands >= 8);
          BL_PROPAGATE(consumer.ensure(v_idx / 2u));

          size_t i = 0;
          double x1, y1, x2, y2;

          v_idx -= 6;
          while ((i += 2) <= v_idx) {
            px += m.x_by_a(v_buf[i - 2], v_buf[i - 1]);
            py += m.y_by_a(v_buf[i - 2], v_buf[i - 1]);
            consumer.line_to(px, py);
          }

          x1 = px + m.x_by_a(v_buf[v_idx + 0], v_buf[v_idx + 1]);
          y1 = py + m.y_by_a(v_buf[v_idx + 0], v_buf[v_idx + 1]);
          x2 = x1 + m.x_by_a(v_buf[v_idx + 2], v_buf[v_idx + 3]);
          y2 = y1 + m.y_by_a(v_buf[v_idx + 2], v_buf[v_idx + 3]);
          px = x2 + m.x_by_a(v_buf[v_idx + 4], v_buf[v_idx + 5]);
          py = y2 + m.y_by_a(v_buf[v_idx + 4], v_buf[v_idx + 5]);
          consumer.cubic_to(x1, y1, x2, y2, px, py);

          v_idx = 0;
          continue;
        }

        // Hints
        // -----

        // |- y dy {dya dyb}* hstem   (1)  |-
        // |- x dx {dxa dxb}* vstem   (3)  |-
        // |- y dy {dya dyb}* hstemhm (18) |-
        // |- x dx {dxa dxb}* vstemhm (23) |-
        case kCSOpHStem:
        case kCSOpVStem:
        case kCSOpHStemHM:
        case kCSOpVStemHM: {
          hint_bit_count += (v_idx / 2);

          v_idx = 0;
          continue;
        }

        // |- hintmask (19) mask |-
        // |- cntrmask (20) mask |-
        case kCSOpHintMask:
        case kCSOpCntrMask: {
          // Acts as an implicit VSTEM.
          hint_bit_count += (v_idx / 2);

          size_t hint_byte_size = (hint_bit_count + 7u) / 8u;
          if (BL_UNLIKELY(PtrOps::bytes_until(ip, ip_end) < hint_byte_size)) {
            goto InvalidData;
          }

          // TODO: [OpenType] CFF HINTING: These bits are ignored atm.
          ip += hint_byte_size;

          v_idx = 0;
          execution_flags |= kCSFlagHasWidth;
          continue;
        }

        // Variation Data Operators
        // ------------------------

        // |- ivs vsindex (15) |-
        case kCSOpVSIndex: {
          // TODO: [OpenType] CFF VARIATIONS
          v_idx = 0;
          continue;
        }

        // in(0)...in(N-1), d(0,0)...d(K-1,0), d(0,1)...d(K-1,1) ... d(0,N-1)...d(K-1,N-1) N blend (16) out(0)...(N-1)
        case kCSOpBlend: {
          // TODO: [OpenType] CFF VARIATIONS
          v_idx = 0;
          continue;
        }

        // Control Flow
        // ------------

        // lsubr# calllsubr (10) -
        case kCSOpCallLSubR: {
          BL_ASSERT(v_min_operands >= 1);

          c_buf[c_idx].reset(ip, ip_end);
          if (BL_UNLIKELY(++c_idx >= kCFFCallStackSize)) {
            goto InvalidData;
          }

          subr_index = local_subr_index;
          subr_id = uint32_t(int32_t(v_buf[--v_idx]) + int32_t(subr_index->bias));

          if (subr_id < subr_index->entry_count) {
            goto OnSubRCall;
          }

          goto InvalidData;
        }

        // gsubr# callgsubr (29) -
        case kCSOpCallGSubR: {
          BL_ASSERT(v_min_operands >= 1);

          c_buf[c_idx].reset(ip, ip_end);
          if (BL_UNLIKELY(++c_idx >= kCFFCallStackSize)) {
            goto InvalidData;
          }

          subr_index = &cff_info.index[CFFData::kIndexGSubR];
          subr_id = uint32_t(int32_t(v_buf[--v_idx]) + int32_t(subr_index->bias));

          if (subr_id < subr_index->entry_count)
            goto OnSubRCall;

          goto InvalidData;
        }

        // return (11)
        case kCSOpReturn: {
          if (BL_UNLIKELY(c_idx == 0)) {
            goto InvalidData;
          }
OnReturn:
          c_idx--;
          ip    = c_buf[c_idx]._ptr;
          ip_end = c_buf[c_idx]._end;
          continue;
        }

        // endchar (14)
        case kCSOpEndChar: {
          goto EndCharString;
        }

        // Escaped Operators
        // -----------------

        case kCSOpEscape: {
          if (BL_UNLIKELY(ip >= ip_end)) {
            goto InvalidData;
          }
          b0 = *ip++;

          #ifdef BL_TRACE_OT_CFF
          trace_char_string_op(ot_face_impl, trace, 0x0C00 | b0, v_buf, v_idx);
          #endif

          if (BL_UNLIKELY(b0 >= ExecutionFeaturesInfo::kEscapedOpCount)) {
            // Unknown operators should clear the stack and act as NOPs.
            v_idx = 0;
            continue;
          }

          v_min_operands = execution_features->escaped_op_stack_size[b0];
          if (BL_UNLIKELY(v_idx < v_min_operands)) {
            // If this is not an unknown operand it would mean that we have less
            // values on stack than the operator requires. That's an error in CS.
            if (v_min_operands != ExecutionFeaturesInfo::kUnknown) {
              goto InvalidData;
            }

            // Unknown operators should clear the stack and act as NOPs.
            v_idx = 0;
            continue;
          }

          // NOTE: CSOpCode enumeration uses escaped values, what we have in
          // `b0` is an unescaped value. It's much easier in terms of resulting
          // machine code to clear the escape sequence in the constant (kCSOp...)
          // rather than adding it to `b0`.
          switch (b0) {
            // |- dx1 dy1 dx2 dy2 dx3 dy3 dx4 dy4 dx5 dy5 dx6 dy6 fd flex (12 35) |-
            case kCSOpFlex & 0xFFu: {
              double x1, y1, x2, y2;
              BL_PROPAGATE(consumer.ensure(6));

              x1 = px + m.x_by_a(v_buf[0], v_buf[1]);
              y1 = py + m.y_by_a(v_buf[0], v_buf[1]);
              x2 = x1 + m.x_by_a(v_buf[2], v_buf[3]);
              y2 = y1 + m.y_by_a(v_buf[2], v_buf[3]);
              px = x2 + m.x_by_a(v_buf[4], v_buf[5]);
              py = y2 + m.y_by_a(v_buf[4], v_buf[5]);
              consumer.cubic_to(x1, y1, x2, y2, px, py);

              x1 = px + m.x_by_a(v_buf[6], v_buf[7]);
              y1 = py + m.y_by_a(v_buf[6], v_buf[7]);
              x2 = x1 + m.x_by_a(v_buf[8], v_buf[9]);
              y2 = y1 + m.y_by_a(v_buf[8], v_buf[9]);
              px = x2 + m.x_by_a(v_buf[10], v_buf[11]);
              py = y2 + m.y_by_a(v_buf[10], v_buf[11]);
              consumer.cubic_to(x1, y1, x2, y2, px, py);

              v_idx = 0;
              continue;
            }

            // |- dx1 dy1 dx2 dy2 dx3 dy3 dx4 dy4 dx5 dy5 d6 flex1 (12 37) |-
            case kCSOpFlex1 & 0xFFu: {
              double x1, y1, x2, y2, x3, y3, x4, y4, x5, y5;
              BL_PROPAGATE(consumer.ensure(6));

              x1 = px + m.x_by_a(v_buf[0], v_buf[1]);
              y1 = py + m.y_by_a(v_buf[0], v_buf[1]);
              x2 = x1 + m.x_by_a(v_buf[2], v_buf[3]);
              y2 = y1 + m.y_by_a(v_buf[2], v_buf[3]);
              x3 = x2 + m.x_by_a(v_buf[4], v_buf[5]);
              y3 = y2 + m.y_by_a(v_buf[4], v_buf[5]);
              consumer.cubic_to(x1, y1, x2, y2, x3, y3);

              x4 = x3 + m.x_by_a(v_buf[6], v_buf[7]);
              y4 = y3 + m.y_by_a(v_buf[6], v_buf[7]);
              x5 = x4 + m.x_by_a(v_buf[8], v_buf[9]);
              y5 = y4 + m.y_by_a(v_buf[8], v_buf[9]);

              double dx = bl_abs(v_buf[0] + v_buf[2] + v_buf[4] + v_buf[6] + v_buf[8]);
              double dy = bl_abs(v_buf[1] + v_buf[3] + v_buf[5] + v_buf[7] + v_buf[9]);
              if (dx > dy) {
                px = x5 + m.x_by_x(v_buf[10]);
                py = y5 + m.y_by_x(v_buf[10]);
              }
              else {
                px = x5 + m.x_by_y(v_buf[10]);
                py = y5 + m.y_by_y(v_buf[10]);
              }
              consumer.cubic_to(x4, y4, x5, y5, px, py);

              v_idx = 0;
              continue;
            }

            // |- dx1 dx2 dy2 dx3 dx4 dx5 dx6 hflex (12 34) |-
            case kCSOpHFlex & 0xFFu: {
              double x1, y1, x2, y2, x3, y3, x4, y4, x5, y5;
              BL_PROPAGATE(consumer.ensure(6));

              x1 = px + m.x_by_x(v_buf[0]);
              y1 = py + m.y_by_x(v_buf[0]);
              x2 = x1 + m.x_by_a(v_buf[1], v_buf[2]);
              y2 = y1 + m.y_by_a(v_buf[1], v_buf[2]);
              x3 = x2 + m.x_by_x(v_buf[3]);
              y3 = y2 + m.y_by_x(v_buf[3]);
              consumer.cubic_to(x1, y1, x2, y2, x3, y3);

              x4 = x3 + m.x_by_x(v_buf[4]);
              y4 = y3 + m.y_by_x(v_buf[4]);
              x5 = x4 + m.x_by_a(v_buf[5], -v_buf[2]);
              y5 = y4 + m.y_by_a(v_buf[5], -v_buf[2]);
              px = x5 + m.x_by_x(v_buf[6]);
              py = y5 + m.y_by_x(v_buf[6]);
              consumer.cubic_to(x4, y4, x5, y5, px, py);

              v_idx = 0;
              continue;
            }

            // |- dx1 dy1 dx2 dy2 dx3 dx4 dx5 dy5 dx6 hflex1 (12 36) |-
            case kCSOpHFlex1 & 0xFFu: {
              double x1, y1, x2, y2, x3, y3, x4, y4, x5, y5;
              BL_PROPAGATE(consumer.ensure(6));

              x1 = px + m.x_by_a(v_buf[0], v_buf[1]);
              y1 = py + m.y_by_a(v_buf[0], v_buf[1]);
              x2 = x1 + m.x_by_a(v_buf[2], v_buf[3]);
              y2 = y1 + m.y_by_a(v_buf[2], v_buf[3]);
              x3 = x2 + m.x_by_x(v_buf[4]);
              y3 = y2 + m.y_by_x(v_buf[4]);
              consumer.cubic_to(x1, y1, x2, y2, x3, y3);

              x4 = x3 + m.x_by_x(v_buf[5]);
              y4 = y3 + m.y_by_x(v_buf[5]);
              x5 = x4 + m.x_by_a(v_buf[6], v_buf[7]);
              y5 = y4 + m.y_by_a(v_buf[6], v_buf[7]);
              px = x5 + m.x_by_x(v_buf[8]);
              py = y5 + m.y_by_x(v_buf[8]);
              consumer.cubic_to(x4, y4, x5, y5, px, py);

              v_idx = 0;
              continue;
            }

            // in1 in2 and (12 3) out {in1 && in2}
            case kCSOpAnd & 0xFFu: {
              BL_ASSERT(v_min_operands >= 2);
              v_buf[v_idx - 2] = double((v_buf[v_idx - 2] != 0.0) & (v_buf[v_idx - 1] != 0.0));
              v_idx--;
              continue;
            }

            // in1 in2 or (12 4) out {in1 || in2}
            case kCSOpOr & 0xFFu: {
              BL_ASSERT(v_min_operands >= 2);
              v_buf[v_idx - 2] = double((v_buf[v_idx - 2] != 0.0) | (v_buf[v_idx - 1] != 0.0));
              v_idx--;
              continue;
            }

            // in1 in2 eq (12 15) out {in1 == in2}
            case kCSOpEq & 0xFFu: {
              BL_ASSERT(v_min_operands >= 2);
              v_buf[v_idx - 2] = double(v_buf[v_idx - 2] == v_buf[v_idx - 1]);
              v_idx--;
              continue;
            }

            // s1 s2 v1 v2 ifelse (12 22) out {v1 <= v2 ? s1 : s2}
            case kCSOpIfElse & 0xFFu: {
              BL_ASSERT(v_min_operands >= 4);
              v_buf[v_idx - 4] = v_buf[v_idx - 4 + size_t(v_buf[v_idx - 2] <= v_buf[v_idx - 1])];
              v_idx -= 3;
              continue;
            }

            // in not (12 5) out {!in}
            case kCSOpNot & 0xFFu: {
              BL_ASSERT(v_min_operands >= 1);
              v_buf[v_idx - 1] = double(v_buf[v_idx - 1] == 0.0);
              continue;
            }

            // in neg (12 14) out {-in}
            case kCSOpNeg & 0xFFu: {
              BL_ASSERT(v_min_operands >= 1);
              v_buf[v_idx - 1] = -v_buf[v_idx - 1];
              continue;
            }

            // in abs (12 9) out {abs(in)}
            case kCSOpAbs & 0xFFu: {
              BL_ASSERT(v_min_operands >= 1);
              v_buf[v_idx - 1] = bl_abs(v_buf[v_idx - 1]);
              continue;
            }

            // in sqrt (12 26) out {sqrt(in)}
            case kCSOpSqrt & 0xFFu: {
              BL_ASSERT(v_min_operands >= 1);
              v_buf[v_idx - 1] = Math::sqrt(bl_max(v_buf[v_idx - 1], 0.0));
              continue;
            }

            // in1 in2 add (12 10) out {in1 + in2}
            case kCSOpAdd & 0xFFu: {
              BL_ASSERT(v_min_operands >= 2);
              double result = v_buf[v_idx - 2] + v_buf[v_idx - 1];
              v_buf[v_idx - 2] = Math::is_finite(result) ? result : 0.0;
              v_idx--;
              continue;
            }

            // in1 in2 sub (12 11) out {in1 - in2}
            case kCSOpSub & 0xFFu: {
              BL_ASSERT(v_min_operands >= 2);
              double result = v_buf[v_idx - 2] - v_buf[v_idx - 1];
              v_buf[v_idx - 2] = Math::is_finite(result) ? result : 0.0;
              v_idx--;
              continue;
            }

            // CFFv1: in1 in2 mul (12 24) out {in1 * in2}
            case kCSOpMul & 0xFFu: {
              BL_ASSERT(v_min_operands >= 2);
              double result = v_buf[v_idx - 2] * v_buf[v_idx - 1];
              v_buf[v_idx - 2] = Math::is_finite(result) ? result : 0.0;
              v_idx--;
              continue;
            }

            // CFFv1: in1 in2 div (12 12) out {in1 / in2}
            case kCSOpDiv & 0xFFu: {
              BL_ASSERT(v_min_operands >= 2);
              double result = v_buf[v_idx - 2] / v_buf[v_idx - 1];
              v_buf[v_idx - 2] = Math::is_finite(result) ? result : 0.0;
              v_idx--;
              continue;
            }

            // random (12 23) out
            case kCSOpRandom & 0xFFu: {
              if (BL_UNLIKELY(++v_idx > kCFFValueStackSizeV1))
                goto InvalidData;

              // NOTE: Don't allow anything random.
              v_buf[v_idx - 1] = 0.5;
              continue;
            }

            // in dup (12 27) out out
            case kCSOpDup & 0xFFu: {
              BL_ASSERT(v_min_operands >= 1);
              if (BL_UNLIKELY(++v_idx > kCFFValueStackSizeV1))
                goto InvalidData;
              v_buf[v_idx - 1] = v_buf[v_idx - 2];
              continue;
            }

            // in drop (12 18)
            case kCSOpDrop & 0xFFu: {
              if (BL_UNLIKELY(v_idx == 0))
                goto InvalidData;
              v_idx--;
              continue;
            }

            // in1 in2 exch (12 28) out1 out2
            case kCSOpExch & 0xFFu: {
              BL_ASSERT(v_min_operands >= 2);
              BLInternal::swap(v_buf[v_idx - 2], v_buf[v_idx - 1]);
              continue;
            }

            // nX...n0 I index (12 29) nX...n0 n[I]
            case kCSOpIndex & 0xFFu: {
              BL_ASSERT(v_min_operands >= 2);

              double idx_value = v_buf[v_idx - 1];
              double val_to_push = 0.0;

              if (idx_value < 0.0) {
                // If I is negative, top element is copied.
                val_to_push = v_buf[v_idx - 2];
              }
              else {
                // It will overflow if idx_value is greater than `v_idx - 1`, thus,
                // `index_to_read` would become a very large number that would not
                // pass the condition afterwards.
                size_t index_to_read = v_idx - 1 - size_t(unsigned(idx_value));
                if (index_to_read < v_idx - 1) {
                  val_to_push = v_buf[index_to_read];
                }
              }

              v_buf[v_idx - 1] = val_to_push;
              continue;
            }

            // n(N–1)...n0 N J roll (12 30) n((J–1) % N)...n0 n(N–1)...n(J % N)
            case kCSOpRoll & 0xFFu: {
              unsigned int shift = unsigned(int(v_buf[--v_idx]));
              unsigned int count = unsigned(int(v_buf[--v_idx]));

              if (count > v_idx)
                count = unsigned(v_idx);

              if (count < 2)
                continue;

              // Always convert the shift to a positive number so we only rotate
              // to the right and not in both directions. This is easy as the
              // shift is always bound to [0, count) regardless of the direction.
              if (int(shift) < 0)
                shift = IntOps::negate(IntOps::negate(shift) % count) + count;
              else
                shift %= count;

              if (shift == 0)
                continue;

              double last = 0;
              uint32_t cur_idx = IntOps::negate(uint32_t(1));
              uint32_t base_idx = cur_idx;

              for (uint32_t i = 0; i < count; i++) {
                if (cur_idx == base_idx) {
                  last = v_buf[++cur_idx];
                  base_idx = cur_idx;
                }

                cur_idx += shift;
                if (cur_idx >= count) {
                  cur_idx -= count;
                }

                BLInternal::swap(v_buf[cur_idx], last);
              }

              continue;
            }

            // in I put (12 20)
            case kCSOpPut & 0xFFu: {
              unsigned int s_idx = unsigned(int(v_buf[v_idx - 1]));
              if (s_idx < kCFFStorageSize) {
                s_buf[s_idx] = v_buf[v_idx - 2];
                s_msk |= IntOps::lsb_bit_at<uint32_t>(s_idx);
              }

              v_idx -= 2;
              continue;
            }

            // I get (12 21) out
            case kCSOpGet & 0xFFu: {
              unsigned int s_idx = unsigned(int(v_buf[v_idx - 1]));

              // When `s_idx == kCFFStorageSize` it points to `0.0` (the only value guaranteed to be set).
              // Otherwise we check the bit in `s_msk` and won't allow to get an uninitialized value that
              // was not stored at `s_idx` before (for security reasons).
              if (s_idx >= kCFFStorageSize || !IntOps::bit_test(s_msk, s_idx)) {
                s_idx = kCFFStorageSize;
              }

              v_buf[v_idx - 1] = s_buf[s_idx];
              continue;
            }

            // Unknown operator - drop the stack and continue.
            default: {
              v_idx = 0;
              continue;
            }
          }
        }

        // Unknown operator - drop the stack and continue.
        default: {
          v_idx = 0;
          continue;
        }
      }
    }
  }

EndCharString:
  if (execution_flags & kCSFlagPathOpen) {
    BL_PROPAGATE(consumer.ensure(1));
    consumer.close();
  }

  consumer.done();
  trace.info("[%zu bytes processed]\n", bytes_processed);

  return BL_SUCCESS;

InvalidData:
  consumer.done();
  trace.fail("Invalid data [%zu bytes processed]\n", bytes_processed);

  return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);
}

// bl::OpenType::CFFImpl - GetGlyphBounds
// ======================================

namespace {

// Glyph outlines consumer that calculates glyph bounds.
class GlyphBoundsConsumer {
public:
  BLBox bounds {};
  double cx = 0.0;
  double cy = 0.0;

  BL_INLINE BLResult begin(size_t n) noexcept {
    bl_unused(n);
    bounds.reset(Traits::max_value<double>(), Traits::max_value<double>(), Traits::min_value<double>(), Traits::min_value<double>());
    cx = 0;
    cy = 0;
    return BL_SUCCESS;
  }

  BL_INLINE void done() noexcept {}

  BL_INLINE BLResult ensure(size_t n) noexcept {
    bl_unused(n);
    return BL_SUCCESS;
  }

  BL_INLINE void move_to(double x0, double y0) noexcept {
    Geometry::bound(bounds, BLPoint(x0, y0));
    cx = x0;
    cy = y0;
  }

  BL_INLINE void line_to(double x1, double y1) noexcept {
    Geometry::bound(bounds, BLPoint(x1, y1));
    cx = x1;
    cy = y1;
  }

  // Not used by CFF, provided for completness.
  BL_INLINE void quad_to(double x1, double y1, double x2, double y2) noexcept {
    Geometry::bound(bounds, BLPoint(x2, y2));
    if (!bounds.contains(x1, y1))
      merge_quad_extrema(x1, y1, x2, y2);
    cx = x2;
    cy = y2;
  }

  BL_INLINE void cubic_to(double x1, double y1, double x2, double y2, double x3, double y3) noexcept {
    Geometry::bound(bounds, BLPoint(x3, y3));
    if (!Geometry::subsumes(bounds, BLBox(bl_min(x1, x2), bl_min(y1, y2), bl_max(x1, x2), bl_max(y1, y2))))
      merge_cubic_extrema(x1, y1, x2, y2, x3, y3);
    cx = x3;
    cy = y3;
  }

  BL_INLINE void close() noexcept {}

  // We calculate extrema here as the code may expand a bit and inlining everything doesn't bring any benefits in
  // such case, because most control points in fonts are within the bounding box defined by start/end points anyway.
  //
  // Making these two functions no-inline saves around 8kB.
  BL_NOINLINE void merge_quad_extrema(double x1, double y1, double x2, double y2) noexcept {
    Geometry::Quad<BLPoint> quad(cx, cy, x1, y1, x2, y2);
    BLPoint extrema = Geometry::quad_extrema_point(quad);
    Geometry::bound(bounds, extrema);
  }

  BL_NOINLINE void merge_cubic_extrema(double x1, double y1, double x2, double y2, double x3, double y3) noexcept {
    Geometry::Cubic<BLPoint> cubic(cx, cy, x1, y1, x2, y2, x3, y3);
    BLPoint extrema[2];

    Geometry::cubic_extrema_points(cubic, extrema);
    Geometry::bound(bounds, extrema[0]);
    Geometry::bound(bounds, extrema[1]);
  }
};

} // {anonymous}

static BLResult BL_CDECL get_glyph_bounds(
  const BLFontFaceImpl* face_impl,
  const uint32_t* glyph_data,
  intptr_t glyph_advance,
  BLBoxI* boxes,
  size_t count) noexcept {

  BLResult result = BL_SUCCESS;
  BLMatrix2D transform = BLMatrix2D::make_identity();

  ScopedBufferTmp<1024> tmp_buffer;
  GlyphBoundsConsumer consumer;

  for (size_t i = 0; i < count; i++) {
    BLGlyphId glyph_id = glyph_data[0];
    glyph_data = PtrOps::offset(glyph_data, glyph_advance);

    BLResult local_result = get_glyph_outlines_t<GlyphBoundsConsumer>(face_impl, glyph_id, &transform, consumer, &tmp_buffer);
    if (local_result) {
      boxes[i].reset();
      result = local_result;
      continue;
    }
    else {
      const BLBox& bounds = consumer.bounds;
      if (bounds.x0 <= bounds.x1 && bounds.y0 <= bounds.y1) {
        boxes[i].reset(Math::floor_to_int(bounds.x0),
                       Math::floor_to_int(bounds.y0),
                       Math::ceil_to_int(bounds.x1),
                       Math::ceil_to_int(bounds.y1));
      }
      else {
        boxes[i].reset();
      }
    }
  }

  return result;
}

// bl::OpenType::CFFImpl - GetGlyphOutlines
// ========================================

namespace {

// Glyph outlines consumer that appends the decoded outlines into `BLPath`.
class GlyphOutlineConsumer {
public:
  BLPath* path;
  size_t contour_count;
  PathAppender appender;

  BL_INLINE GlyphOutlineConsumer(BLPath* p) noexcept
    : path(p),
      contour_count(0) {}

  BL_INLINE BLResult begin(size_t n) noexcept {
    return appender.begin_append(path, n);
  }

  BL_INLINE BLResult ensure(size_t n) noexcept {
    return appender.ensure(path, n);
  }

  BL_INLINE void done() noexcept {
    appender.done(path);
  }

  BL_INLINE void move_to(double x0, double y0) noexcept {
    contour_count++;
    appender.move_to(x0, y0);
  }

  BL_INLINE void line_to(double x1, double y1) noexcept {
    appender.line_to(x1, y1);
  }

  // Not used by CFF, provided for completness.
  BL_INLINE void quad_to(double x1, double y1, double x2, double y2) noexcept {
    appender.quad_to(x1, y1, x2, y2);
  }

  BL_INLINE void cubic_to(double x1, double y1, double x2, double y2, double x3, double y3) noexcept {
    appender.cubic_to(x1, y1, x2, y2, x3, y3);
  }

  BL_INLINE void close() noexcept {
    appender.close();
  }
};

} // {anonymous}

static BLResult BL_CDECL get_glyph_outlines(
  const BLFontFaceImpl* face_impl,
  BLGlyphId glyph_id,
  const BLMatrix2D* transform,
  BLPath* out,
  size_t* contour_count_out,
  ScopedBuffer* tmp_buffer) noexcept {

  GlyphOutlineConsumer consumer(out);
  BLResult result = get_glyph_outlines_t<GlyphOutlineConsumer>(face_impl, glyph_id, transform, consumer, tmp_buffer);

  *contour_count_out = consumer.contour_count;
  return result;
}

// bl::OpenType::CIDInfo - Struct
// ==============================

struct CIDInfo {
  enum Flags : uint32_t {
    kFlagIsCID       = 0x00000001u,
    kFlagHasFDArray  = 0x00000002u,
    kFlagHasFDSelect = 0x00000004u,
    kFlagsAll        = 0x00000007u
  };

  uint32_t flags;
  uint32_t ros[2];
  uint32_t fd_array_offset;
  uint32_t fd_select_offset;
  uint8_t fd_select_format;
};

// bl::OpenType::CFFImpl - Init
// ============================

static BL_INLINE bool isSupportedFDSelectFormat(uint32_t format) noexcept {
  return format == 0 || format == 3;
}

BLResult init(OTFaceImpl* ot_face_impl, OTFaceTables& tables, uint32_t cff_version) noexcept {
  DictIterator dict_iter;
  DictEntry dict_entry;

  Index name_index {};
  Index top_dict_index {};
  Index string_index {};
  Index gsubr_index {};
  Index lsubr_index {};
  Index char_string_index {};

  uint32_t name_offset = 0;
  uint32_t top_dict_offset = 0;
  uint32_t string_offset = 0;
  uint32_t gsubr_offset = 0;
  uint32_t char_string_offset = 0;

  uint32_t begin_data_offset = 0;
  uint32_t private_offset = 0;
  uint32_t private_length = 0;
  uint32_t lsubr_offset = 0;

  CIDInfo cid {};
  BLArray<CFFData::IndexData> fd_subr_indexes;

  ot_face_impl->face_info.outline_type = uint8_t(BL_FONT_OUTLINE_TYPE_CFF + cff_version);

  // CFF Header
  // ----------

  Table<CFFTable> cff { cff_version == CFFData::kVersion1 ? tables.cff : tables.cff2 };
  if (BL_UNLIKELY(!cff.fits()))
    return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);

  // The specification says that the implementation should refuse MAJOR version, which it doesn't understand.
  // We understand version 1 & 2 (there seems to be no other version) so refuse anything else. It also says
  // that change in MINOR version should never cause an incompatibility, so we ignore it completely.
  if (BL_UNLIKELY(cff_version + 1 != cff->header.major_version()))
    return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);

  uint32_t top_dict_size = 0;
  uint32_t header_size = cff->header.header_size();

  if (cff_version == CFFData::kVersion1) {
    if (BL_UNLIKELY(header_size < 4 || header_size > cff.size - 4)) {
      return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);
    }

    uint32_t offset_size = cff->headerV1()->offset_size();
    if (BL_UNLIKELY(offset_size < 1 || offset_size > 4)) {
      return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);
    }
  }
  else {
    if (BL_UNLIKELY(header_size < 5 || header_size > cff.size - 5)) {
      return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);
    }

    top_dict_size = cff->headerV2()->top_dict_length();
  }

  // CFF NameIndex
  // -------------

  // NameIndex is only used by CFF, CFF2 doesn't use it.
  if (cff_version == CFFData::kVersion1) {
    name_offset = header_size;
    BL_PROPAGATE(read_index(cff.data + name_offset, cff.size - name_offset, cff_version, &name_index));

    // There should be exactly one font in the table according to OpenType specification.
    if (BL_UNLIKELY(name_index.count != 1)) {
      return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);
    }

    top_dict_offset = name_offset + name_index.total_size;
  }
  else {
    top_dict_offset = header_size;
  }

  // CFF TopDictIndex
  // ----------------

  if (cff_version == CFFData::kVersion1) {
    // CFF doesn't have the size specified in the header, so we have to compute it.
    top_dict_size = uint32_t(cff.size - top_dict_offset);
  }
  else {
    // CFF2 specifies the size in the header, so make sure it doesn't overflow our limits.
    if (BL_UNLIKELY(top_dict_size > cff.size - top_dict_offset)) {
      return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);
    }
  }

  BL_PROPAGATE(read_index(cff.data + top_dict_offset, top_dict_size, cff_version, &top_dict_index));
  if (cff_version == CFFData::kVersion1) {
    // TopDict index size must match NameIndex size (v1).
    if (BL_UNLIKELY(name_index.count != top_dict_index.count)) {
      return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);
    }
  }

  {
    uint32_t offsets[2] = { top_dict_index.offset_at(0), top_dict_index.offset_at(1) };
    dict_iter.reset(top_dict_index.payload + offsets[0], offsets[1] - offsets[0]);
  }

  while (dict_iter.has_next()) {
    BL_PROPAGATE(dict_iter.next(dict_entry));
    switch (dict_entry.op) {
      case CFFTable::kDictOpTopCharStrings: {
        if (BL_UNLIKELY(dict_entry.count != 1)) {
          return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);
        }

        char_string_offset = uint32_t(dict_entry.values[0]);
        break;
      }

      case CFFTable::kDictOpTopPrivate: {
        if (BL_UNLIKELY(dict_entry.count != 2)) {
          return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);
        }

        private_offset = uint32_t(dict_entry.values[1]);
        private_length = uint32_t(dict_entry.values[0]);
        break;
      }

      case CFFTable::kDictOpTopROS: {
        if (dict_entry.count == 3) {
          cid.ros[0] = uint32_t(dict_entry.values[0]);
          cid.ros[1] = uint32_t(dict_entry.values[1]);
          cid.flags |= CIDInfo::kFlagIsCID;
        }
        break;
      }

      case CFFTable::kDictOpTopFDArray: {
        if (dict_entry.count == 1) {
          cid.fd_array_offset = uint32_t(dict_entry.values[0]);
          cid.flags |= CIDInfo::kFlagHasFDArray;
        }
        break;
      }

      case CFFTable::kDictOpTopFDSelect: {
        if (dict_entry.count == 1) {
          cid.fd_select_offset = uint32_t(dict_entry.values[0]);
          cid.flags |= CIDInfo::kFlagHasFDSelect;
        }
        break;
      }
    }
  }

  // CFF StringIndex
  // ---------------

  // StringIndex is only used by CFF, CFF2 doesn't use it.
  if (cff_version == CFFData::kVersion1) {
    string_offset = top_dict_offset + top_dict_index.total_size;
    BL_PROPAGATE(read_index(cff.data + string_offset, cff.size - string_offset, cff_version, &string_index));
    gsubr_offset = string_offset + string_index.total_size;
  }
  else {
    gsubr_offset = top_dict_offset + top_dict_index.total_size;
  }

  // CFF GSubRIndex
  // --------------

  BL_PROPAGATE(read_index(cff.data + gsubr_offset, cff.size - gsubr_offset, cff_version, &gsubr_index));
  begin_data_offset = gsubr_offset + gsubr_index.total_size;

  // CFF PrivateDict
  // ---------------

  if (private_offset) {
    if (BL_UNLIKELY(private_offset < begin_data_offset ||
                    private_offset > cff.size ||
                    private_length > cff.size - private_offset)) {
        return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);
    }

    // There are fonts where `private_offset` is equal to `cff.size` and `private_length` is
    // zero. So only search the private dictionary if `private_length` is greater than zero.
    if (private_length) {
      dict_iter.reset(cff.data + private_offset, private_length);
      while (dict_iter.has_next()) {
        BL_PROPAGATE(dict_iter.next(dict_entry));
        switch (dict_entry.op) {
          case CFFTable::kDictOpPrivSubrs: {
            if (BL_UNLIKELY(dict_entry.count != 1)) {
              return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);
            }

            lsubr_offset = uint32_t(dict_entry.values[0]);
            break;
          }
        }
      }
    }
  }

  // CFF LSubRIndex
  // --------------

  if (lsubr_offset) {
    // `lsubr_offset` is relative to `private_offset`.
    if (BL_UNLIKELY(lsubr_offset < private_length || lsubr_offset > cff.size - private_offset)) {
      return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);
    }

    lsubr_offset += private_offset;
    BL_PROPAGATE(read_index(cff.data + lsubr_offset, cff.size - lsubr_offset, cff_version, &lsubr_index));
  }

  // CFF CharStrings
  // ---------------

  if (BL_UNLIKELY(char_string_offset < begin_data_offset || char_string_offset >= cff.size)) {
    return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);
  }

  BL_PROPAGATE(read_index(cff.data + char_string_offset, cff.size - char_string_offset, cff_version, &char_string_index));

  // CFF/CID
  // -------

  if ((cid.flags & CIDInfo::kFlagsAll) == CIDInfo::kFlagsAll) {
    uint32_t fd_array_offset = cid.fd_array_offset;
    uint32_t fd_select_offset = cid.fd_select_offset;

    // CID fonts require both FDArray and FDOffset.
    if (fd_array_offset && fd_select_offset) {
      if (fd_array_offset < begin_data_offset || fd_array_offset >= cff.size) {
        return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);
      }

      if (fd_select_offset < begin_data_offset || fd_select_offset >= cff.size) {
        return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);
      }

      // The index contains offsets to the additional TopDicts. To speed up glyph processing we read
      // these TopDicts and build our own array that will be used during glyph metrics/outline decoding.
      Index fd_array_index;
      BL_PROPAGATE(read_index(cff.data + fd_array_offset, cff.size - fd_array_offset, cff_version, &fd_array_index));
      BL_PROPAGATE(fd_subr_indexes.reserve(fd_array_index.count));

      const uint8_t* fd_array_offsets = fd_array_index.offsets;
      for (uint32_t i = 0; i < fd_array_index.count; i++) {
        Index fd_subr_index {};
        uint32_t subr_offset = 0;
        uint32_t subr_base_offset = 0;

        // NOTE: The offsets were already verified by `read_index()`.
        uint32_t offsets[2];
        read_offset_array(fd_array_offsets, fd_array_index.offset_size, offsets, 2);

        // Offsets start from 1, we have to adjust them to start from 0.
        offsets[0] -= CFFTable::kOffsetAdjustment;
        offsets[1] -= CFFTable::kOffsetAdjustment;

        // dict_data[1] would be a private dictionary, if present...
        RawTable dict_data[2];
        dict_data[0].reset(fd_array_index.payload + offsets[0], offsets[1] - offsets[0]);
        dict_data[1].reset();

        for (uint32_t d = 0; d < 2; d++) {
          dict_iter.reset(dict_data[d].data, dict_data[d].size);
          while (dict_iter.has_next()) {
            BL_PROPAGATE(dict_iter.next(dict_entry));
            switch (dict_entry.op) {
              case CFFTable::kDictOpTopPrivate: {
                if (BL_UNLIKELY(dict_entry.count != 2)) {
                  return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);
                }

                uint32_t offset = uint32_t(dict_entry.values[1]);
                uint32_t length = uint32_t(dict_entry.values[0]);

                if (BL_UNLIKELY(offset < begin_data_offset || offset > cff.size || length > cff.size - offset)) {
                  return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);
                }

                dict_data[1].reset(cff.data + offset, length);
                subr_base_offset = offset;
                break;
              }

              case CFFTable::kDictOpPrivSubrs: {
                if (BL_UNLIKELY(dict_entry.count != 1)) {
                  return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);
                }

                // The local subr `offset` is relative to the `subr_base_offset`.
                subr_offset = uint32_t(dict_entry.values[0]);
                if (BL_UNLIKELY(subr_offset > cff.size - subr_base_offset)) {
                  return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);
                }

                subr_offset += subr_base_offset;
                BL_PROPAGATE(read_index(cff.data + subr_offset, cff.size - subr_offset, cff_version, &fd_subr_index));
                break;
              }
            }
          }
        }

        CFFData::IndexData fd_subr_index_data;
        fd_subr_index_data.reset(
          DataRange { subr_offset, fd_subr_index.total_size },
          fd_subr_index.header_size,
          fd_subr_index.offset_size,
          fd_subr_index.count,
          calc_subr_bias(fd_subr_index.count));

        fd_subr_indexes.append(fd_subr_index_data);
        fd_array_offsets += fd_array_index.offset_size;
      }

      // Validate FDSelect data.
      cid.fd_select_format = cff.data[fd_select_offset];
      if (BL_UNLIKELY(!isSupportedFDSelectFormat(cid.fd_select_format))) {
        return bl_make_error(BL_ERROR_FONT_CFF_INVALID_DATA);
      }
    }
  }

  // Done
  // ----

  ot_face_impl->cff.table = cff;

  ot_face_impl->cff.index[CFFData::kIndexGSubR].reset(
    DataRange { gsubr_offset, gsubr_index.total_size },
    gsubr_index.header_size,
    gsubr_index.offset_size,
    gsubr_index.count,
    calc_subr_bias(gsubr_index.count));

  ot_face_impl->cff.index[CFFData::kIndexLSubR].reset(
    DataRange { lsubr_offset, lsubr_index.total_size },
    lsubr_index.header_size,
    lsubr_index.offset_size,
    lsubr_index.count,
    calc_subr_bias(lsubr_index.count));

  ot_face_impl->cff.index[CFFData::kIndexCharString].reset(
    DataRange { char_string_offset, char_string_index.total_size },
    char_string_index.header_size,
    char_string_index.offset_size,
    char_string_index.count,
    0);

  ot_face_impl->cff.fd_select_offset = cid.fd_select_offset;
  ot_face_impl->cff.fd_select_format = cid.fd_select_format;
  ot_face_impl->cff_fd_subr_indexes.swap(fd_subr_indexes);

  ot_face_impl->funcs.get_glyph_bounds = get_glyph_bounds;
  ot_face_impl->funcs.get_glyph_outlines = get_glyph_outlines;

  return BL_SUCCESS;
};

} // {CFFImpl}
} // {bl::OpenType}
