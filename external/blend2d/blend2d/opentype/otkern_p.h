// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_OPENTYPE_OTKERN_P_H_INCLUDED
#define BLEND2D_OPENTYPE_OTKERN_P_H_INCLUDED

#include <blend2d/core/array_p.h>
#include <blend2d/opentype/otcore_p.h>
#include <blend2d/support/ptrops_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_opentype_impl
//! \{

namespace bl::OpenType {

//! OpenType 'kern' table.
//!
//! External Resources:
//!   - https://docs.microsoft.com/en-us/typography/opentype/spec/kern
//!   - https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6kern.html
struct KernTable {
  enum : uint32_t { kBaseSize = 4 };

  struct WinTableHeader {
    UInt16 version;
    UInt16 table_count;
  };

  struct MacTableHeader {
    F16x16 version;
    UInt32 table_count;
  };

  struct WinGroupHeader {
    enum Coverage : uint8_t {
      kCoverageHorizontal   = 0x01u,
      kCoverageMinimum      = 0x02u,
      kCoverageCrossStream  = 0x04u,
      kCoverageOverride     = 0x08u,
      kCoverageReservedBits = 0xF0u
    };

    UInt16 version;
    UInt16 length;
    UInt8 format;
    UInt8 coverage;
  };

  struct MacGroupHeader {
    enum Coverage : uint8_t {
      kCoverageVertical     = 0x80u,
      kCoverageCrossStream  = 0x40u,
      kCoverageVariation    = 0x20u,
      kCoverageReservedBits = 0x1Fu
    };

    UInt32 length;
    UInt8 coverage;
    UInt8 format;
    UInt16 tuple_index;
  };

  struct Pair {
    union {
      UInt32 combined;
      struct {
        UInt16 left;
        UInt16 right;
      };
    };
    Int16 value;
  };

  struct Format0 {
    UInt16 pair_count;
    UInt16 search_range;
    UInt16 entry_selector;
    UInt16 range_shift;
    /*
    Pair pair_array[pair_count];
    */

    BL_INLINE const Pair* pair_array() const noexcept { return PtrOps::offset<const Pair>(this, 8); }
  };

  struct Format1 {
    enum ValueBits : uint16_t {
      kValueOffsetMask      = 0x3FFFu,
      kValueNoAdvance       = 0x4000u,
      kValuePush            = 0x8000u
    };

    struct StateHeader {
      UInt16 state_size;
      Offset16 class_table;
      Offset16 state_array;
      Offset16 entry_table;
    };

    StateHeader state_header;
    Offset16 value_table;
  };

  struct Format2 {
    struct ClassTable {
      UInt16 first_glyph;
      UInt16 glyph_count;
      /*
      Offset16 offset_array[glyph_count];
      */

      BL_INLINE const Offset16* offset_array() const noexcept { return PtrOps::offset<const Offset16>(this, 4); }
    };

    UInt16 row_width;
    Offset16 left_class_table;
    Offset16 right_class_table;
    Offset16 kerning_array;
  };

  struct Format3 {
    UInt16 glyph_count;
    UInt8 kern_value_count;
    UInt8 left_class_count;
    UInt8 right_class_count;
    UInt8 flags;
    /*
    FWord kern_value[kern_value_count];
    UInt8 left_class[glyph_count];
    UInt8 right_class[glyph_count];
    UInt8 kern_index[left_class_count * right_class_count];
    */
  };

  WinTableHeader header;
};

//! Kerning group.
//!
//! Helper data that we create for each kerning group (sub-table).
struct KernGroup {
  // Using the same bits as `KernTable::WinGroupHeader::Coverage` except for Horizontal.
  enum Flags : uint32_t {
    kFlagSynthesized = 0x01u,
    kFlagMinimum     = 0x02u,
    kFlagCrossStream = 0x04u,
    kFlagOverride    = 0x08u,
    kFlagsMask       = 0x0Fu
  };

  size_t packed_data;

  union {
    uintptr_t data_offset;
    void* data_ptr;
  };

  BL_INLINE bool has_flag(uint32_t flag) const noexcept { return (packed_data & flag) != 0u; }
  BL_INLINE bool is_synthesized() const noexcept { return has_flag(kFlagSynthesized); }
  BL_INLINE bool is_minimum() const noexcept { return has_flag(kFlagMinimum); }
  BL_INLINE bool is_cross_stream() const noexcept { return has_flag(kFlagCrossStream); }
  BL_INLINE bool is_override() const noexcept { return has_flag(kFlagOverride); }

  BL_INLINE uint32_t format() const noexcept { return uint32_t((packed_data >> 4u) & 3u); }
  BL_INLINE uint32_t flags() const noexcept { return uint32_t(packed_data & 0xFu); }
  BL_INLINE size_t data_size() const noexcept { return packed_data >> 6u; }

  BL_INLINE const void* calc_data_ptr(const void* base_ptr) const noexcept {
    return is_synthesized() ? data_ptr : static_cast<const void*>(static_cast<const uint8_t*>(base_ptr) + data_offset);
  }

  static BL_INLINE KernGroup make_referenced(uint32_t format, uint32_t flags, uintptr_t data_offset, uint32_t data_size) noexcept {
    return KernGroup { flags | (format << 2) | (data_size << 6), { data_offset } };
  }

  static BL_INLINE KernGroup make_synthesized(uint32_t format, uint32_t flags, void* data_ptr, uint32_t data_size) noexcept {
    return KernGroup { flags | (format << 2) | (data_size << 6) | kFlagSynthesized, { (uintptr_t)data_ptr } };
  }
};

class KernCollection {
public:
  BL_NONCOPYABLE(KernCollection)

  BLArray<KernGroup> groups;

  BL_INLINE KernCollection() noexcept = default;
  BL_INLINE ~KernCollection() noexcept { release_data(); }

  BL_INLINE bool is_empty() const noexcept { return groups.is_empty(); }

  BL_INLINE void reset() noexcept {
    release_data();
    groups.reset();
  }

  void release_data() noexcept {
    size_t count = groups.size();
    for (size_t i = 0; i < count; i++) {
      const KernGroup& group = groups[i];
      if (group.is_synthesized())
        free(group.data_ptr);
    }
  }
};

//! Kerning data stored in `OTFace` and used to perform kerning.
class KernData {
public:
  BL_NONCOPYABLE(KernData)

  enum HeaderType : uint32_t {
    kHeaderWindows = 0,
    kHeaderMac = 1
  };

  RawTable table {};
  uint8_t header_type {};
  uint8_t header_size {};
  uint8_t reserved[6] {};
  KernCollection collection[2];

  BL_INLINE KernData() noexcept = default;
};

namespace KernImpl {
BLResult init(OTFaceImpl* ot_face_impl, OTFaceTables& tables) noexcept;
} // {KernImpl}

} // {bl::OpenType}

//! \}
//! \endcond

#endif // BLEND2D_OPENTYPE_OTKERN_P_H_INCLUDED
