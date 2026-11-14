// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_OPENTYPE_OTDEFS_P_H_INCLUDED
#define BLEND2D_OPENTYPE_OTDEFS_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/core/fontdata_p.h>
#include <blend2d/core/fontface_p.h>
#include <blend2d/support/memops_p.h>
#include <blend2d/support/ptrops_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_opentype_impl
//! \{

//! Assertion of a validated data.
//!
//! This type of assert is used in every place that works with a validated table.
#define BL_ASSERT_VALIDATED(...) BL_ASSERT(__VA_ARGS__)

//! \namespace bl::OpenType
//! Low-level OpenType functionality, not exposed to users directly.

namespace bl::OpenType {

struct OTFaceImpl;
union OTFaceTables;

//! Provides minimum and maximum glyph id - used by the API.
struct GlyphRange {
  uint32_t glyph_min;
  uint32_t glyph_max;

  BL_INLINE_NODEBUG bool contains(BLGlyphId glyph_id) const noexcept {
    return BLInternal::bool_and(glyph_id >= glyph_min, glyph_id <= glyph_max);
  }
};

struct OffsetRange {
  uint32_t start;
  uint32_t end;

  template<typename T>
  BL_INLINE_NODEBUG bool contains(const T& offset) const noexcept {
    return BLInternal::bool_and(offset >= start, offset < end);
  }
};

//! A range that specifies offset and size of a data table or some part of it.
struct DataRange {
  uint32_t offset;
  uint32_t size;

  BL_INLINE_NODEBUG void reset() noexcept { reset(0, 0); }
  BL_INLINE_NODEBUG void reset(uint32_t offset_, uint32_t size_) noexcept {
    this->offset = offset_;
    this->size = size_;
  }
};

template<typename T>
struct Table;

//! A read only data that represents a font table or its sub-table.
//!
//! \note This is functionally similar compared to \ref BLFontTable. The difference is that we prefer to have table
//! size as `uint32_t` integer instead of `size_t` as various offsets and slices in OpenType are 32-bit integers.
//! Having one value as `size_t` and the rest as `uint32_t` leads to a casting nightmare.
struct RawTable {
  //! \name Members
  //! \{

  //! Pointer to the beginning of the data interpreted as `uint8_t*`.
  const uint8_t* data;
  //! Size of `data` in bytes.
  uint32_t size;

  //! \}

  //! \name Construction & Destruction
  //! \{

  BL_INLINE_NODEBUG RawTable() noexcept = default;
  BL_INLINE_NODEBUG RawTable(const RawTable& other) noexcept = default;

  BL_INLINE_NODEBUG RawTable(const BLFontTable& other) noexcept
    : data(other.data),
      size(uint32_t(other.size)) {}

  BL_INLINE_NODEBUG RawTable(const uint8_t* data, uint32_t size) noexcept
    : data(data),
      size(size) {}

  //! \}

  //! \name Overloaded Operators
  //! \{

  //! Tests whether the table has a content.
  //!
  //! \note This is essentially the opposite of `is_empty()`.
  BL_INLINE_NODEBUG explicit operator bool() const noexcept { return size != 0; }

  BL_INLINE_NODEBUG RawTable& operator=(const RawTable& other) noexcept = default;

  //! \}

  //! \name Common Functionality
  //! \{

  //! Tests whether the table is empty (has no content).
  BL_INLINE_NODEBUG bool is_empty() const noexcept { return !size; }

  BL_INLINE_NODEBUG void reset() noexcept {
    data = nullptr;
    size = 0;
  }

  BL_INLINE_NODEBUG void reset(const uint8_t* data_, uint32_t size_) noexcept {
    data = data_;
    size = size_;
  }

  template<typename SizeT>
  BL_INLINE_NODEBUG bool fits(const SizeT& n_bytes) const noexcept { return n_bytes <= size; }

  //! \}

  //! \name Accessors
  //! \{

  template<typename T>
  BL_INLINE const T* data_as(size_t offset = 0u) const noexcept {
    BL_ASSERT(offset <= size);
    return reinterpret_cast<const T*>(data + offset);
  }

  BL_INLINE uint32_t readU8(size_t offset) const noexcept {
    BL_ASSERT(offset < size);
    return data[offset];
  }

  BL_INLINE uint32_t readU16(size_t offset) const noexcept {
    BL_ASSERT(offset + 2 <= size);
    return MemOps::readU16aBE(data + offset);
  }

  BL_INLINE RawTable sub_table(uint32_t offset) const noexcept {
    offset = bl_min(offset, size);
    return RawTable(data + offset, size - offset);
  }

  template<typename T>
  BL_INLINE Table<T> sub_table(uint32_t offset) const noexcept {
    offset = bl_min(offset, size);
    return Table<T>(data + offset, size - offset);
  }

  BL_INLINE RawTable sub_table_unchecked(uint32_t offset) const noexcept {
    BL_ASSERT(offset <= size);
    return RawTable(data + offset, size - offset);
  }

  template<typename T>
  BL_INLINE Table<T> sub_table_unchecked(uint32_t offset) const noexcept {
    BL_ASSERT(offset <= size);
    return Table<T>(data + offset, size - offset);
  }

  //! \}
};

//! A convenience class that maps `RawTable` to a typed table.
template<typename T>
struct Table : public RawTable {
  //! \name Construction & Destruction
  //! \{

  BL_INLINE_NODEBUG Table() noexcept = default;
  BL_INLINE_NODEBUG Table(const Table& other) noexcept = default;

  BL_INLINE_NODEBUG Table(const RawTable& other) noexcept
    : RawTable(other.data, other.size) {}

  BL_INLINE_NODEBUG Table(const BLFontTable& other) noexcept
    : RawTable(other) {}

  BL_INLINE_NODEBUG Table(const uint8_t* data, uint32_t size) noexcept
    : RawTable(data, size) {}

  //! \}

  //! \name Overloaded Operators
  //! \{

  BL_INLINE_NODEBUG Table& operator=(const Table& other) noexcept = default;
  BL_INLINE_NODEBUG const T* operator->() const noexcept { return data_as<T>(); }

  //! \}

  //! \name Helpers
  //! \{

  using RawTable::fits;

  BL_INLINE_NODEBUG bool fits() const noexcept { return size >= T::kBaseSize; }

  //! \}
};

template<typename SizeT>
static BL_INLINE bool bl_font_table_fits_n(const RawTable& table, const SizeT& required_size, const SizeT& offset = 0) noexcept {
  return (table.size - offset) >= required_size;
}

template<typename T, typename SizeT = uint32_t>
static BL_INLINE bool blFontTableFitsT(const RawTable& table, const SizeT& offset = 0) noexcept {
  return bl_font_table_fits_n(table, SizeT(T::kBaseSize), offset);
}

/*
static BL_INLINE RawTable bl_font_sub_table(const RawTable& table, uint32_t offset) noexcept {
  BL_ASSERT(offset <= table.size);
  return RawTable(table.data + offset, table.size - offset);
}

static BL_INLINE RawTable bl_font_sub_table_checked(const RawTable& table, uint32_t offset) noexcept {
  return RawTable(table.data, bl_min(table.size, offset));
}

template<typename T>
static BL_INLINE Table<T> blFontSubTableT(const RawTable& table, uint32_t offset) noexcept {
  BL_ASSERT(offset <= table.size);
  return Table<T>(table.data + offset, table.size - offset);
}

template<typename T>
static BL_INLINE Table<T> blFontSubTableCheckedT(const RawTable& table, uint32_t offset) noexcept {
  return blFontSubTableT<T>(table, bl_min(table.size, offset));
}
*/

template<size_t Size>
struct DataAccess {};

template<>
struct DataAccess<1> {
  template<uint32_t ByteOrder, size_t Alignment>
  static BL_INLINE_NODEBUG uint32_t read_value(const uint8_t* data) noexcept { return MemOps::readU8(data); }

  template<uint32_t ByteOrder, size_t Alignment>
  static BL_INLINE void write_value(uint8_t* data, uint32_t value) noexcept { MemOps::writeU8(data, value); }
};

template<>
struct DataAccess<2> {
  template<uint32_t ByteOrder, size_t Alignment>
  static BL_INLINE_NODEBUG uint32_t read_value(const uint8_t* data) noexcept { return MemOps::readU16<ByteOrder, Alignment>(data); }

  template<uint32_t ByteOrder, size_t Alignment>
  static BL_INLINE void write_value(uint8_t* data, uint32_t value) noexcept { MemOps::writeU16<ByteOrder, Alignment>(data, value); }
};

template<>
struct DataAccess<3> {
  template<uint32_t ByteOrder, size_t Alignment>
  static BL_INLINE_NODEBUG uint32_t read_value(const uint8_t* data) noexcept { return MemOps::readU24u<ByteOrder>(data); }

  template<uint32_t ByteOrder, size_t Alignment>
  static BL_INLINE void write_value(uint8_t* data, uint32_t value) noexcept { MemOps::writeU24u<ByteOrder>(data, value); }
};

template<>
struct DataAccess<4> {
  template<uint32_t ByteOrder, size_t Alignment>
  static BL_INLINE_NODEBUG uint32_t read_value(const uint8_t* data) noexcept { return MemOps::readU32<ByteOrder, Alignment>(data); }

  template<uint32_t ByteOrder, size_t Alignment>
  static BL_INLINE void write_value(uint8_t* data, uint32_t value) noexcept { MemOps::writeU32<ByteOrder, Alignment>(data, value); }
};

template<>
struct DataAccess<8> {
  template<uint32_t ByteOrder, size_t Alignment>
  static BL_INLINE_NODEBUG uint64_t read_value(const uint8_t* data) noexcept { return MemOps::readU64<ByteOrder, Alignment>(data); }

  template<uint32_t ByteOrder, size_t Alignment>
  static BL_INLINE void write_value(uint8_t* data, uint64_t value) noexcept { MemOps::writeU64<ByteOrder, Alignment>(data, value); }
};

#pragma pack(push, 1)
template<typename T, uint32_t ByteOrder, size_t Size>
struct DataType {
  uint8_t data[Size];

  BL_INLINE_NODEBUG DataType(const DataType& other) noexcept = default;

  template<size_t Alignment = 1>
  BL_INLINE_NODEBUG T value() const noexcept { return T(DataAccess<Size>::template read_value<ByteOrder, Alignment>(data));  }

  template<size_t Alignment = 1>
  BL_INLINE_NODEBUG T raw_value() const noexcept { return T(DataAccess<Size>::template read_value<BL_BYTE_ORDER_NATIVE, Alignment>(data)); }

  template<size_t Alignment = 1>
  BL_INLINE void set_value(T value) noexcept {
    using U = std::make_unsigned_t<T>;
    DataAccess<Size>::template write_value<ByteOrder, Alignment>(data, U(value));
  }

  template<size_t Alignment = 1>
  BL_INLINE void set_raw_value(T value) noexcept {
    using U = std::make_unsigned_t<T>;
    DataAccess<Size>::template write_value<BL_BYTE_ORDER_NATIVE, Alignment>(data, U(value));
  }

  BL_INLINE_NODEBUG T operator()() const noexcept { return value(); }

  BL_INLINE_NODEBUG DataType& operator=(const DataType& other) noexcept = default;
  BL_INLINE_NODEBUG DataType& operator=(T other) noexcept { set_value(other); return *this; }
};
#pragma pack(pop)

// Everything in OpenType is big-endian.
typedef DataType<int8_t  , BL_BYTE_ORDER_BE, 1> Int8;
typedef DataType<int16_t , BL_BYTE_ORDER_BE, 2> Int16;
typedef DataType<int32_t , BL_BYTE_ORDER_BE, 4> Int32;
typedef DataType<int64_t , BL_BYTE_ORDER_BE, 8> Int64;

typedef DataType<uint8_t , BL_BYTE_ORDER_BE, 1> UInt8;
typedef DataType<uint16_t, BL_BYTE_ORDER_BE, 2> UInt16;
typedef DataType<uint32_t, BL_BYTE_ORDER_BE, 3> UInt24;
typedef DataType<uint32_t, BL_BYTE_ORDER_BE, 4> UInt32;
typedef DataType<uint64_t, BL_BYTE_ORDER_BE, 8> UInt64;

typedef UInt16 Offset16;
typedef UInt32 Offset32;

typedef Int16 FWord;
typedef UInt16 UFWord;
typedef UInt16 F2x14;
typedef UInt32 F16x16;
typedef UInt32 CheckSum;
typedef Int64 DateTime;

template<typename T>
struct Array16 {
  enum : uint32_t { kBaseSize = 2 };

  UInt16 count;
  BL_INLINE_NODEBUG const T* array() const noexcept { return PtrOps::offset<const T>(this, 2); }
};

template<typename T>
struct Array32 {
  enum : uint32_t { kBaseSize = 4 };

  UInt32 count;
  BL_INLINE_NODEBUG const T* array() const noexcept { return PtrOps::offset<const T>(this, 4); }
};

//! Tag and offset.
//!
//! Replaces a lot of OpenType tables that use this structure (GDEF|GPOS|GSUB).
struct TagRef16 {
  UInt32 tag;
  Offset16 offset;
};

} // {bl::OpenType}

//! \}
//! \endcond

#endif // BLEND2D_OPENTYPE_OTDEFS_P_H_INCLUDED
