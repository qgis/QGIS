// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_COMPOPINFO_P_H_INCLUDED
#define BLEND2D_COMPOPINFO_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/core/compop_p.h>
#include <blend2d/pipeline/pipedefs_p.h>
#include <blend2d/support/bitops_p.h>
#include <blend2d/support/lookuptable_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

namespace bl {

//! Information about a composition operator.
struct CompOpInfo {
  uint16_t _flags;

  BL_INLINE_NODEBUG CompOpFlags flags() const noexcept { return (CompOpFlags)_flags; }
};

//! Provides flags for each composition operator.
BL_HIDDEN extern const LookupTable<CompOpInfo, kCompOpExtCount> comp_op_info_table;

//! Information that can be used to simplify a "Dst CompOp Src" into a simpler composition operator with a possible
//! format conversion and arbitrary source to solid conversion. This is used by the rendering engine to simplify every
//! composition operator before it considers which pipeline to use.
//!
//! There are two reasons for simplification - the first is performance and the second reason is about decreasing the
//! number of possible pipeline signatures the rendering context may require. For example by using "SRC-COPY" operator
//! instead of "CLEAR" operator the rendering engine basically eliminated a possible compilation of "CLEAR" operator
//! that would perform exactly the same as "SRC-COPY".
struct CompOpSimplifyInfo {
  //! \name Constants
  //! \{

  // Data shift specify where the value is stored in `data`.
  static inline constexpr uint32_t kCompOpShift = IntOps::bit_shift_of(Pipeline::Signature::kMaskCompOp);
  static inline constexpr uint32_t kDstFmtShift = IntOps::bit_shift_of(Pipeline::Signature::kMaskDstFormat);
  static inline constexpr uint32_t kSrcFmtShift = IntOps::bit_shift_of(Pipeline::Signature::kMaskSrcFormat);
  static inline constexpr uint32_t kSolidIdShift = 16;

  //! \}

  //! \name Members
  //! \{

  //! Alternative composition operator, destination format, source format, and solid-id information packed into 32 bits.
  uint32_t data;

  //! \}

  //! \name Accessors
  //! \{

  //! Returns all bits that form the signature (CompOp, DstFormat SrcFormat).
  BL_INLINE_CONSTEXPR uint32_t signature_bits() const noexcept { return data & 0xFFFFu; }
  //! Returns `Signature` configured to have the same bits set as `signature_bits()`.
  BL_INLINE_CONSTEXPR Pipeline::Signature signature() const noexcept { return Pipeline::Signature{signature_bits()}; }

  //! Returns the simplified composition operator.
  BL_INLINE_CONSTEXPR CompOpExt comp_op() const noexcept { return CompOpExt((data & Pipeline::Signature::kMaskCompOp) >> kCompOpShift); }
  //! Returns the simplified destination format.
  BL_INLINE_CONSTEXPR FormatExt dst_format() const noexcept { return FormatExt((data & Pipeline::Signature::kMaskDstFormat) >> kDstFmtShift); }
  //! Returns the simplified source format.
  BL_INLINE_CONSTEXPR FormatExt src_format() const noexcept { return FormatExt((data & Pipeline::Signature::kMaskSrcFormat) >> kSrcFmtShift); }

  //! Returns solid-id information regarding this simplification.
  BL_INLINE_CONSTEXPR CompOpSolidId solid_id() const noexcept { return CompOpSolidId(data >> kSolidIdShift); }

  //! \}

  //! \name Make
  //! \{

  //! Returns `CompOpSimplifyInfo` from decomposed arguments.
  static BL_INLINE_CONSTEXPR CompOpSimplifyInfo make(CompOpExt comp_op, FormatExt d, FormatExt s, CompOpSolidId solid_id) noexcept {
    return CompOpSimplifyInfo {
      uint32_t((uint32_t(comp_op) << kCompOpShift) |
               (uint32_t(d) << kDstFmtShift) |
               (uint32_t(s) << kSrcFmtShift) |
               (uint32_t(solid_id) << kSolidIdShift))
    };
  }

  //! Returns `CompOpSimplifyInfo` sentinel containing the only correct value of DST_COPY (NOP) operator. All other
  //! variations of DST_COPY are invalid.
  static BL_INLINE_CONSTEXPR CompOpSimplifyInfo dst_copy() noexcept {
    return make(CompOpExt::kDstCopy, FormatExt::kNone, FormatExt::kNone, CompOpSolidId::kAlwaysNop);
  }

  //! \}
};

// Initially we have used a single table, however, some older compilers would reach template instantiation depth limit
// (as the table is not small), so the implementation was changed to this instead to make sure this won't happen.
static constexpr uint32_t kCompOpSimplifyRecordSize = kCompOpExtCount * (uint32_t(FormatExt::kMaxReserved) + 1);
typedef LookupTable<CompOpSimplifyInfo, kCompOpSimplifyRecordSize> CompOpSimplifyInfoRecordSet;

struct CompOpSimplifyInfoTable { CompOpSimplifyInfoRecordSet data[BL_FORMAT_MAX_VALUE + 1u]; };
BL_HIDDEN extern const CompOpSimplifyInfoTable comp_op_simplify_info_table;

static BL_INLINE const CompOpSimplifyInfo* comp_op_simplify_info_array_of(CompOpExt comp_op, FormatExt dst_format) noexcept {
  return &comp_op_simplify_info_table.data[size_t(dst_format)][size_t(comp_op) * kFormatExtCount];
}

static BL_INLINE const CompOpSimplifyInfo& comp_op_simplify_info(CompOpExt comp_op, FormatExt dst_format, FormatExt src_format) noexcept {
  return comp_op_simplify_info_array_of(comp_op, dst_format)[size_t(src_format)];
}

} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_COMPOPINFO_P_H_INCLUDED
