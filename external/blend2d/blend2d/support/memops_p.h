// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_SUPPORT_MEMOPS_P_H_INCLUDED
#define BLEND2D_SUPPORT_MEMOPS_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/support/intops_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

namespace bl {
namespace MemOps {
namespace {

//! \name Unaligned Constants
//! \{

static const constexpr bool kUnalignedMemIO = (BL_TARGET_ARCH_X86 != 0) || (BL_TARGET_ARCH_ARM == 64) || (BL_TARGET_ARCH_WASM != 0);
static const constexpr bool kUnalignedMem16 = (BL_TARGET_ARCH_X86 != 0) || (BL_TARGET_ARCH_ARM == 64) || (BL_TARGET_ARCH_WASM != 0);
static const constexpr bool kUnalignedMem32 = (BL_TARGET_ARCH_X86 != 0) || (BL_TARGET_ARCH_ARM == 64) || (BL_TARGET_ARCH_WASM != 0);
static const constexpr bool kUnalignedMem64 = (BL_TARGET_ARCH_X86 != 0) || (BL_TARGET_ARCH_ARM == 64) || (BL_TARGET_ARCH_WASM != 0);

//! \}

//! \name Unaligned Types
//! \{

//! An integer type that has possibly less alignment than its size.
template<typename T, size_t Alignment>
struct UnalignedInt {};

template<> struct UnalignedInt<int8_t, 1> { typedef int8_t T; };
template<> struct UnalignedInt<int16_t, 1> { typedef BL_MAY_ALIAS BL_UNALIGNED_TYPE(int16_t, 1) T; };
template<> struct UnalignedInt<int16_t, 2> { typedef BL_MAY_ALIAS int16_t T; };
template<> struct UnalignedInt<int32_t, 1> { typedef BL_MAY_ALIAS BL_UNALIGNED_TYPE(int32_t, 1) T; };
template<> struct UnalignedInt<int32_t, 2> { typedef BL_MAY_ALIAS BL_UNALIGNED_TYPE(int32_t, 2) T; };
template<> struct UnalignedInt<int32_t, 4> { typedef BL_MAY_ALIAS int32_t T; };
template<> struct UnalignedInt<int64_t, 1> { typedef BL_MAY_ALIAS BL_UNALIGNED_TYPE(int64_t, 1) T; };
template<> struct UnalignedInt<int64_t, 2> { typedef BL_MAY_ALIAS BL_UNALIGNED_TYPE(int64_t, 2) T; };
template<> struct UnalignedInt<int64_t, 4> { typedef BL_MAY_ALIAS BL_UNALIGNED_TYPE(int64_t, 4) T; };
template<> struct UnalignedInt<int64_t, 8> { typedef BL_MAY_ALIAS int64_t T; };

template<> struct UnalignedInt<uint8_t, 1> { typedef uint8_t T; };
template<> struct UnalignedInt<uint16_t, 1> { typedef BL_MAY_ALIAS BL_UNALIGNED_TYPE(uint16_t, 1) T; };
template<> struct UnalignedInt<uint16_t, 2> { typedef BL_MAY_ALIAS uint16_t T; };
template<> struct UnalignedInt<uint32_t, 1> { typedef BL_MAY_ALIAS BL_UNALIGNED_TYPE(uint32_t, 1) T; };
template<> struct UnalignedInt<uint32_t, 2> { typedef BL_MAY_ALIAS BL_UNALIGNED_TYPE(uint32_t, 2) T; };
template<> struct UnalignedInt<uint32_t, 4> { typedef BL_MAY_ALIAS uint32_t T; };
template<> struct UnalignedInt<uint64_t, 1> { typedef BL_MAY_ALIAS BL_UNALIGNED_TYPE(uint64_t, 1) T; };
template<> struct UnalignedInt<uint64_t, 2> { typedef BL_MAY_ALIAS BL_UNALIGNED_TYPE(uint64_t, 2) T; };
template<> struct UnalignedInt<uint64_t, 4> { typedef BL_MAY_ALIAS BL_UNALIGNED_TYPE(uint64_t, 4) T; };
template<> struct UnalignedInt<uint64_t, 8> { typedef BL_MAY_ALIAS uint64_t T; };

//! \}

//! \name Memory Read
//! \{

[[nodiscard]]
static BL_INLINE_NODEBUG uint32_t readU8(const void* p) noexcept { return uint32_t(static_cast<const uint8_t*>(p)[0]); }

[[nodiscard]]
static BL_INLINE_NODEBUG int32_t readI8(const void* p) noexcept { return int32_t(static_cast<const int8_t*>(p)[0]); }

template<uint32_t ByteOrder, size_t Alignment>
[[nodiscard]]
static BL_INLINE_NODEBUG uint32_t readU16(const void* p) noexcept {
  if (ByteOrder == BL_BYTE_ORDER_NATIVE && (kUnalignedMem16 || Alignment >= 2)) {
    typedef typename UnalignedInt<uint16_t, Alignment>::T U16AlignedToN;
    return uint32_t(static_cast<const U16AlignedToN*>(p)[0]);
  }
  else {
    uint32_t hi = readU8(static_cast<const uint8_t*>(p) + (ByteOrder == BL_BYTE_ORDER_LE ? 1 : 0));
    uint32_t lo = readU8(static_cast<const uint8_t*>(p) + (ByteOrder == BL_BYTE_ORDER_LE ? 0 : 1));
    return IntOps::shl(hi, 8) | lo;
  }
}

template<uint32_t ByteOrder, size_t Alignment>
[[nodiscard]]
static BL_INLINE_NODEBUG int32_t readI16(const void* p) noexcept {
  if (ByteOrder == BL_BYTE_ORDER_NATIVE && (kUnalignedMem16 || Alignment >= 2)) {
    typedef typename UnalignedInt<uint16_t, Alignment>::T U16AlignedToN;
    return int32_t(int16_t(static_cast<const U16AlignedToN*>(p)[0]));
  }
  else {
    int32_t hi = int32_t(readI8(static_cast<const uint8_t*>(p) + (ByteOrder == BL_BYTE_ORDER_LE ? 1 : 0)));
    int32_t lo = int32_t(readU8(static_cast<const uint8_t*>(p) + (ByteOrder == BL_BYTE_ORDER_LE ? 0 : 1)));
    return IntOps::shl(hi, 8) | lo;
  }
}

template<uint32_t ByteOrder = BL_BYTE_ORDER_NATIVE>
[[nodiscard]]
static BL_INLINE_NODEBUG uint32_t readU24u(const void* p) noexcept {
  uint32_t b0 = readU8(static_cast<const uint8_t*>(p) + (ByteOrder == BL_BYTE_ORDER_LE ? 2 : 0));
  uint32_t b1 = readU8(static_cast<const uint8_t*>(p) + 1);
  uint32_t b2 = readU8(static_cast<const uint8_t*>(p) + (ByteOrder == BL_BYTE_ORDER_LE ? 0 : 2));
  return IntOps::shl(b0, 16) | IntOps::shl(b1, 8) | b2;
}

template<uint32_t ByteOrder, size_t Alignment>
[[nodiscard]]
static BL_INLINE_NODEBUG uint32_t readU32(const void* p) noexcept {
  if (kUnalignedMem32 || Alignment >= 4) {
    typedef typename UnalignedInt<uint32_t, Alignment>::T U32AlignedToN;
    uint32_t x = static_cast<const U32AlignedToN*>(p)[0];
    return ByteOrder == BL_BYTE_ORDER_NATIVE ? x : IntOps::byteSwap32(x);
  }
  else {
    uint32_t hi = readU16<ByteOrder, Alignment >= 2 ? size_t(2) : Alignment>(static_cast<const uint8_t*>(p) + (ByteOrder == BL_BYTE_ORDER_LE ? 2 : 0));
    uint32_t lo = readU16<ByteOrder, Alignment >= 2 ? size_t(2) : Alignment>(static_cast<const uint8_t*>(p) + (ByteOrder == BL_BYTE_ORDER_LE ? 0 : 2));
    return IntOps::shl(hi, 16) | lo;
  }
}

template<uint32_t ByteOrder, size_t Alignment>
[[nodiscard]]
static BL_INLINE_NODEBUG uint64_t readU64(const void* p) noexcept {
  if (ByteOrder == BL_BYTE_ORDER_NATIVE && (kUnalignedMem64 || Alignment >= 8)) {
    typedef typename UnalignedInt<uint64_t, Alignment>::T U64AlignedToN;
    return static_cast<const U64AlignedToN*>(p)[0];
  }
  else {
    uint32_t hi = readU32<ByteOrder, Alignment >= 4 ? size_t(4) : Alignment>(static_cast<const uint8_t*>(p) + (ByteOrder == BL_BYTE_ORDER_LE ? 4 : 0));
    uint32_t lo = readU32<ByteOrder, Alignment >= 4 ? size_t(4) : Alignment>(static_cast<const uint8_t*>(p) + (ByteOrder == BL_BYTE_ORDER_LE ? 0 : 4));
    return IntOps::shl(uint64_t(hi), 32) | lo;
  }
}

template<uint32_t ByteOrder, size_t Alignment>
[[nodiscard]]
static  BL_INLINE_NODEBUG int32_t readI32(const void* p) noexcept { return int32_t(readU32<ByteOrder, Alignment>(p)); }

template<uint32_t ByteOrder, size_t Alignment>
[[nodiscard]]
static BL_INLINE_NODEBUG int64_t readI64(const void* p) noexcept { return int64_t(readU64<ByteOrder, Alignment>(p)); }

[[nodiscard]] static BL_INLINE_NODEBUG int32_t readI16a(const void* p) noexcept { return readI16<BL_BYTE_ORDER_NATIVE, 2>(p); }
[[nodiscard]] static BL_INLINE_NODEBUG int32_t readI16u(const void* p) noexcept { return readI16<BL_BYTE_ORDER_NATIVE, 1>(p); }
[[nodiscard]] static BL_INLINE_NODEBUG uint32_t readU16a(const void* p) noexcept { return readU16<BL_BYTE_ORDER_NATIVE, 2>(p); }
[[nodiscard]] static BL_INLINE_NODEBUG uint32_t readU16u(const void* p) noexcept { return readU16<BL_BYTE_ORDER_NATIVE, 1>(p); }

[[nodiscard]] static BL_INLINE_NODEBUG int32_t readI16aLE(const void* p) noexcept { return readI16<BL_BYTE_ORDER_LE, 2>(p); }
[[nodiscard]] static BL_INLINE_NODEBUG int32_t readI16uLE(const void* p) noexcept { return readI16<BL_BYTE_ORDER_LE, 1>(p); }
[[nodiscard]] static BL_INLINE_NODEBUG uint32_t readU16aLE(const void* p) noexcept { return readU16<BL_BYTE_ORDER_LE, 2>(p); }
[[nodiscard]] static BL_INLINE_NODEBUG uint32_t readU16uLE(const void* p) noexcept { return readU16<BL_BYTE_ORDER_LE, 1>(p); }

[[nodiscard]] static BL_INLINE_NODEBUG int32_t readI16aBE(const void* p) noexcept { return readI16<BL_BYTE_ORDER_BE, 2>(p); }
[[nodiscard]] static BL_INLINE_NODEBUG int32_t readI16uBE(const void* p) noexcept { return readI16<BL_BYTE_ORDER_BE, 1>(p); }
[[nodiscard]] static BL_INLINE_NODEBUG uint32_t readU16aBE(const void* p) noexcept { return readU16<BL_BYTE_ORDER_BE, 2>(p); }
[[nodiscard]] static BL_INLINE_NODEBUG uint32_t readU16uBE(const void* p) noexcept { return readU16<BL_BYTE_ORDER_BE, 1>(p); }

[[nodiscard]] static BL_INLINE_NODEBUG uint32_t readU24uLE(const void* p) noexcept { return readU24u<BL_BYTE_ORDER_LE>(p); }
[[nodiscard]] static BL_INLINE_NODEBUG uint32_t readU24uBE(const void* p) noexcept { return readU24u<BL_BYTE_ORDER_BE>(p); }

[[nodiscard]] static BL_INLINE_NODEBUG int32_t readI32a(const void* p) noexcept { return readI32<BL_BYTE_ORDER_NATIVE, 4>(p); }
[[nodiscard]] static BL_INLINE_NODEBUG int32_t readI32u(const void* p) noexcept { return readI32<BL_BYTE_ORDER_NATIVE, 1>(p); }
[[nodiscard]] static BL_INLINE_NODEBUG uint32_t readU32a(const void* p) noexcept { return readU32<BL_BYTE_ORDER_NATIVE, 4>(p); }
[[nodiscard]] static BL_INLINE_NODEBUG uint32_t readU32u(const void* p) noexcept { return readU32<BL_BYTE_ORDER_NATIVE, 1>(p); }

[[nodiscard]] static BL_INLINE_NODEBUG int32_t readI32aLE(const void* p) noexcept { return readI32<BL_BYTE_ORDER_LE, 4>(p); }
[[nodiscard]] static BL_INLINE_NODEBUG int32_t readI32uLE(const void* p) noexcept { return readI32<BL_BYTE_ORDER_LE, 1>(p); }
[[nodiscard]] static BL_INLINE_NODEBUG uint32_t readU32aLE(const void* p) noexcept { return readU32<BL_BYTE_ORDER_LE, 4>(p); }
[[nodiscard]] static BL_INLINE_NODEBUG uint32_t readU32uLE(const void* p) noexcept { return readU32<BL_BYTE_ORDER_LE, 1>(p); }

[[nodiscard]] static BL_INLINE_NODEBUG int32_t readI32aBE(const void* p) noexcept { return readI32<BL_BYTE_ORDER_BE, 4>(p); }
[[nodiscard]] static BL_INLINE_NODEBUG int32_t readI32uBE(const void* p) noexcept { return readI32<BL_BYTE_ORDER_BE, 1>(p); }
[[nodiscard]] static BL_INLINE_NODEBUG uint32_t readU32aBE(const void* p) noexcept { return readU32<BL_BYTE_ORDER_BE, 4>(p); }
[[nodiscard]] static BL_INLINE_NODEBUG uint32_t readU32uBE(const void* p) noexcept { return readU32<BL_BYTE_ORDER_BE, 1>(p); }

[[nodiscard]] static BL_INLINE_NODEBUG int64_t readI64a(const void* p) noexcept { return readI64<BL_BYTE_ORDER_NATIVE, 8>(p); }
[[nodiscard]] static BL_INLINE_NODEBUG int64_t readI64u(const void* p) noexcept { return readI64<BL_BYTE_ORDER_NATIVE, 1>(p); }
[[nodiscard]] static BL_INLINE_NODEBUG uint64_t readU64a(const void* p) noexcept { return readU64<BL_BYTE_ORDER_NATIVE, 8>(p); }
[[nodiscard]] static BL_INLINE_NODEBUG uint64_t readU64u(const void* p) noexcept { return readU64<BL_BYTE_ORDER_NATIVE, 1>(p); }

[[nodiscard]] static BL_INLINE_NODEBUG int64_t readI64aLE(const void* p) noexcept { return readI64<BL_BYTE_ORDER_LE, 8>(p); }
[[nodiscard]] static BL_INLINE_NODEBUG int64_t readI64uLE(const void* p) noexcept { return readI64<BL_BYTE_ORDER_LE, 1>(p); }
[[nodiscard]] static BL_INLINE_NODEBUG uint64_t readU64aLE(const void* p) noexcept { return readU64<BL_BYTE_ORDER_LE, 8>(p); }
[[nodiscard]] static BL_INLINE_NODEBUG uint64_t readU64uLE(const void* p) noexcept { return readU64<BL_BYTE_ORDER_LE, 1>(p); }

[[nodiscard]] static BL_INLINE_NODEBUG int64_t readI64aBE(const void* p) noexcept { return readI64<BL_BYTE_ORDER_BE, 8>(p); }
[[nodiscard]] static BL_INLINE_NODEBUG int64_t readI64uBE(const void* p) noexcept { return readI64<BL_BYTE_ORDER_BE, 1>(p); }
[[nodiscard]] static BL_INLINE_NODEBUG uint64_t readU64aBE(const void* p) noexcept { return readU64<BL_BYTE_ORDER_BE, 8>(p); }
[[nodiscard]] static BL_INLINE_NODEBUG uint64_t readU64uBE(const void* p) noexcept { return readU64<BL_BYTE_ORDER_BE, 1>(p); }

template<typename T>
[[nodiscard]] static BL_INLINE_NODEBUG T loadu(const void* p) noexcept {
  T tmp;
  memcpy(&tmp, p, sizeof(T));
  return tmp;
}

template<typename T>
[[nodiscard]] static BL_INLINE_NODEBUG T loadu_le(const void* p) noexcept {
  T tmp;
  memcpy(&tmp, p, sizeof(T));

  if (BL_BYTE_ORDER_NATIVE != BL_BYTE_ORDER_LE) {
    tmp = IntOps::byte_swap(tmp);
  }

  return tmp;
}

//! \}

//! \name Memory Write
//! \{

static BL_INLINE_NODEBUG void writeU8(void* p, uint32_t x) noexcept { static_cast<uint8_t*>(p)[0] = uint8_t(x & 0xFFu); }
static BL_INLINE_NODEBUG void writeI8(void* p, int32_t x) noexcept { static_cast<uint8_t*>(p)[0] = uint8_t(x & 0xFF); }

template<uint32_t ByteOrder, size_t Alignment>
static BL_INLINE_NODEBUG void writeU16(void* p, uint32_t x) noexcept {
  if (ByteOrder == BL_BYTE_ORDER_NATIVE && (kUnalignedMem16 || Alignment >= 2)) {
    typedef typename UnalignedInt<uint16_t, Alignment>::T U16AlignedToN;
    static_cast<U16AlignedToN*>(p)[0] = uint16_t(x & 0xFFFFu);
  }
  else {
    static_cast<uint8_t*>(p)[0] = uint8_t((x >> (ByteOrder == BL_BYTE_ORDER_LE ? 0 : 8)) & 0xFFu);
    static_cast<uint8_t*>(p)[1] = uint8_t((x >> (ByteOrder == BL_BYTE_ORDER_LE ? 8 : 0)) & 0xFFu);
  }
}

template<uint32_t ByteOrder = BL_BYTE_ORDER_NATIVE>
static BL_INLINE_NODEBUG void writeU24u(void* p, uint32_t v) noexcept {
  static_cast<uint8_t*>(p)[0] = uint8_t((v >> (ByteOrder == BL_BYTE_ORDER_LE ?  0 : 16)) & 0xFFu);
  static_cast<uint8_t*>(p)[1] = uint8_t((v >> 8) & 0xFFu);
  static_cast<uint8_t*>(p)[2] = uint8_t((v >> (ByteOrder == BL_BYTE_ORDER_LE ? 16 :  0)) & 0xFFu);
}

template<uint32_t ByteOrder, size_t Alignment>
static BL_INLINE_NODEBUG void writeU32(void* p, uint32_t x) noexcept {
  if (kUnalignedMem32 || Alignment >= 4) {
    typedef typename UnalignedInt<uint32_t, Alignment>::T U32AlignedToN;
    static_cast<U32AlignedToN*>(p)[0] = (ByteOrder == BL_BYTE_ORDER_NATIVE) ? x : IntOps::byteSwap32(x);
  }
  else {
    writeU16<ByteOrder, Alignment >= 2 ? size_t(2) : Alignment>(static_cast<uint8_t*>(p) + 0, x >> (ByteOrder == BL_BYTE_ORDER_LE ?  0 : 16));
    writeU16<ByteOrder, Alignment >= 2 ? size_t(2) : Alignment>(static_cast<uint8_t*>(p) + 2, x >> (ByteOrder == BL_BYTE_ORDER_LE ? 16 :  0));
  }
}

template<uint32_t ByteOrder, size_t Alignment>
static BL_INLINE_NODEBUG void writeU64(void* p, uint64_t x) noexcept {
  if (ByteOrder == BL_BYTE_ORDER_NATIVE && (kUnalignedMem64 || Alignment >= 8)) {
    typedef typename UnalignedInt<uint64_t, Alignment>::T U64AlignedToN;
    static_cast<U64AlignedToN*>(p)[0] = x;
  }
  else {
    writeU32<ByteOrder, Alignment >= 4 ? size_t(4) : Alignment>(static_cast<uint8_t*>(p) + 0, uint32_t((x >> (ByteOrder == BL_BYTE_ORDER_LE ?  0 : 32)) & 0xFFFFFFFFu));
    writeU32<ByteOrder, Alignment >= 4 ? size_t(4) : Alignment>(static_cast<uint8_t*>(p) + 4, uint32_t((x >> (ByteOrder == BL_BYTE_ORDER_LE ? 32 :  0)) & 0xFFFFFFFFu));
  }
}

template<uint32_t ByteOrder, size_t Alignment>
static BL_INLINE_NODEBUG void writeI16(void* p, int32_t x) noexcept { writeU16<ByteOrder, Alignment>(p, uint32_t(x)); }

template<uint32_t ByteOrder, size_t Alignment>
static BL_INLINE_NODEBUG void writeI32(void* p, int32_t x) noexcept { writeU32<ByteOrder, Alignment>(p, uint32_t(x)); }

template<uint32_t ByteOrder, size_t Alignment>
static BL_INLINE_NODEBUG void writeI64(void* p, int64_t x) noexcept { writeU64<ByteOrder, Alignment>(p, uint64_t(x)); }

static BL_INLINE_NODEBUG void writeI16a(void* p, int32_t x) noexcept { writeI16<BL_BYTE_ORDER_NATIVE, 2>(p, x); }
static BL_INLINE_NODEBUG void writeI16u(void* p, int32_t x) noexcept { writeI16<BL_BYTE_ORDER_NATIVE, 1>(p, x); }
static BL_INLINE_NODEBUG void writeU16a(void* p, uint32_t x) noexcept { writeU16<BL_BYTE_ORDER_NATIVE, 2>(p, x); }
static BL_INLINE_NODEBUG void writeU16u(void* p, uint32_t x) noexcept { writeU16<BL_BYTE_ORDER_NATIVE, 1>(p, x); }

static BL_INLINE_NODEBUG void writeI16aLE(void* p, int32_t x) noexcept { writeI16<BL_BYTE_ORDER_LE, 2>(p, x); }
static BL_INLINE_NODEBUG void writeI16uLE(void* p, int32_t x) noexcept { writeI16<BL_BYTE_ORDER_LE, 1>(p, x); }
static BL_INLINE_NODEBUG void writeU16aLE(void* p, uint32_t x) noexcept { writeU16<BL_BYTE_ORDER_LE, 2>(p, x); }
static BL_INLINE_NODEBUG void writeU16uLE(void* p, uint32_t x) noexcept { writeU16<BL_BYTE_ORDER_LE, 1>(p, x); }

static BL_INLINE_NODEBUG void writeI16aBE(void* p, int32_t x) noexcept { writeI16<BL_BYTE_ORDER_BE, 2>(p, x); }
static BL_INLINE_NODEBUG void writeI16uBE(void* p, int32_t x) noexcept { writeI16<BL_BYTE_ORDER_BE, 1>(p, x); }
static BL_INLINE_NODEBUG void writeU16aBE(void* p, uint32_t x) noexcept { writeU16<BL_BYTE_ORDER_BE, 2>(p, x); }
static BL_INLINE_NODEBUG void writeU16uBE(void* p, uint32_t x) noexcept { writeU16<BL_BYTE_ORDER_BE, 1>(p, x); }

static BL_INLINE_NODEBUG void writeU24uLE(void* p, uint32_t v) noexcept { writeU24u<BL_BYTE_ORDER_LE>(p, v); }
static BL_INLINE_NODEBUG void writeU24uBE(void* p, uint32_t v) noexcept { writeU24u<BL_BYTE_ORDER_BE>(p, v); }

static BL_INLINE_NODEBUG void writeI32a(void* p, int32_t x) noexcept { writeI32<BL_BYTE_ORDER_NATIVE, 4>(p, x); }
static BL_INLINE_NODEBUG void writeI32u(void* p, int32_t x) noexcept { writeI32<BL_BYTE_ORDER_NATIVE, 1>(p, x); }
static BL_INLINE_NODEBUG void writeU32a(void* p, uint32_t x) noexcept { writeU32<BL_BYTE_ORDER_NATIVE, 4>(p, x); }
static BL_INLINE_NODEBUG void writeU32u(void* p, uint32_t x) noexcept { writeU32<BL_BYTE_ORDER_NATIVE, 1>(p, x); }

static BL_INLINE_NODEBUG void writeI32aLE(void* p, int32_t x) noexcept { writeI32<BL_BYTE_ORDER_LE, 4>(p, x); }
static BL_INLINE_NODEBUG void writeI32uLE(void* p, int32_t x) noexcept { writeI32<BL_BYTE_ORDER_LE, 1>(p, x); }
static BL_INLINE_NODEBUG void writeU32aLE(void* p, uint32_t x) noexcept { writeU32<BL_BYTE_ORDER_LE, 4>(p, x); }
static BL_INLINE_NODEBUG void writeU32uLE(void* p, uint32_t x) noexcept { writeU32<BL_BYTE_ORDER_LE, 1>(p, x); }

static BL_INLINE_NODEBUG void writeI32aBE(void* p, int32_t x) noexcept { writeI32<BL_BYTE_ORDER_BE, 4>(p, x); }
static BL_INLINE_NODEBUG void writeI32uBE(void* p, int32_t x) noexcept { writeI32<BL_BYTE_ORDER_BE, 1>(p, x); }
static BL_INLINE_NODEBUG void writeU32aBE(void* p, uint32_t x) noexcept { writeU32<BL_BYTE_ORDER_BE, 4>(p, x); }
static BL_INLINE_NODEBUG void writeU32uBE(void* p, uint32_t x) noexcept { writeU32<BL_BYTE_ORDER_BE, 1>(p, x); }

static BL_INLINE_NODEBUG void writeI64a(void* p, int64_t x) noexcept { writeI64<BL_BYTE_ORDER_NATIVE, 8>(p, x); }
static BL_INLINE_NODEBUG void writeI64u(void* p, int64_t x) noexcept { writeI64<BL_BYTE_ORDER_NATIVE, 1>(p, x); }
static BL_INLINE_NODEBUG void writeU64a(void* p, uint64_t x) noexcept { writeU64<BL_BYTE_ORDER_NATIVE, 8>(p, x); }
static BL_INLINE_NODEBUG void writeU64u(void* p, uint64_t x) noexcept { writeU64<BL_BYTE_ORDER_NATIVE, 1>(p, x); }

static BL_INLINE_NODEBUG void writeI64aLE(void* p, int64_t x) noexcept { writeI64<BL_BYTE_ORDER_LE, 8>(p, x); }
static BL_INLINE_NODEBUG void writeI64uLE(void* p, int64_t x) noexcept { writeI64<BL_BYTE_ORDER_LE, 1>(p, x); }
static BL_INLINE_NODEBUG void writeU64aLE(void* p, uint64_t x) noexcept { writeU64<BL_BYTE_ORDER_LE, 8>(p, x); }
static BL_INLINE_NODEBUG void writeU64uLE(void* p, uint64_t x) noexcept { writeU64<BL_BYTE_ORDER_LE, 1>(p, x); }

static BL_INLINE_NODEBUG void writeI64aBE(void* p, int64_t x) noexcept { writeI64<BL_BYTE_ORDER_BE, 8>(p, x); }
static BL_INLINE_NODEBUG void writeI64uBE(void* p, int64_t x) noexcept { writeI64<BL_BYTE_ORDER_BE, 1>(p, x); }
static BL_INLINE_NODEBUG void writeU64aBE(void* p, uint64_t x) noexcept { writeU64<BL_BYTE_ORDER_BE, 8>(p, x); }
static BL_INLINE_NODEBUG void writeU64uBE(void* p, uint64_t x) noexcept { writeU64<BL_BYTE_ORDER_BE, 1>(p, x); }

template<typename T>
static BL_INLINE_NODEBUG void storeu(void* p, const T& v) noexcept {
  memcpy(p, &v, sizeof(T));
}

template<typename T>
static BL_INLINE_NODEBUG void storeu_le(void* p, const T& v) noexcept {
  T tmp = v;

  if constexpr (BL_BYTE_ORDER_NATIVE != BL_BYTE_ORDER_LE) {
    tmp = IntOps::byte_swap(tmp);
  }

  memcpy(p, &tmp, sizeof(T));
}

//! \}

//! \name Memory Fill
//! \{

template<typename T>
static BL_INLINE void fill_inline_t(T* dst, const T& pattern, size_t count) noexcept {
  for (size_t i = 0; i < count; i++)
    dst[i] = pattern;
}

template<typename T>
static BL_INLINE void fill_small_t(T* dst, const T& pattern, size_t count) noexcept {
  fill_inline_t(dst, pattern, count);
}

static BL_INLINE void fill_small(void* dst, uint8_t pattern, size_t count) noexcept {
#if defined(__GNUC__) && BL_TARGET_ARCH_X86 && !defined(BL_SANITIZE_MEMORY)
  size_t unused0, unused1;
  __asm__ __volatile__(
    "rep stosb" : "=&c"(unused0), "=&D"(unused1)
                : "0"(count), "a"(pattern), "1"(dst)
                : "memory");
#elif defined(_MSC_VER) && BL_TARGET_ARCH_X86
  __stosb(static_cast<unsigned char *>(dst), static_cast<unsigned char>(pattern), count);
#else
  fill_small_t(static_cast<uint8_t*>(dst), pattern, count);
#endif
}

//! \}

//! \name Memory Copy
//! \{

template<typename T>
static BL_INLINE void copy_forward_inline_t(T* dst, const T* src, size_t count) noexcept {
  for (size_t i = 0; i < count; i++)
    dst[i] = src[i];
}

template<typename T>
static BL_INLINE void copy_backward_inline_t(T* dst, const T* src, size_t count) noexcept {
  size_t i = count;
  while (i) {
    i--;
    dst[i] = src[i];
  }
}

template<typename T>
static BL_INLINE void copy_forward_and_zero_t(T* dst, T* src, size_t count) noexcept {
  for (size_t i = 0; i < count; i++) {
    T item = src[i];
    src[i] = T(0);
    dst[i] = item;
  }
}

//! Copies `n` bytes from `src` to `dst` - optimized for small buffers.
static BL_INLINE void copy_small(void* dst, const void* src, size_t n) noexcept {
#if defined(__GNUC__) && BL_TARGET_ARCH_X86 && !defined(BL_SANITIZE_MEMORY)
  size_t unused;
  __asm__ __volatile__(
    "rep movsb" : "=&D"(dst), "=&S"(src), "=&c"(unused)
                : "0"(dst), "1"(src), "2"(n)
                : "memory"
  );
#elif defined(_MSC_VER) && BL_TARGET_ARCH_X86
  __movsb(static_cast<unsigned char *>(dst), static_cast<const unsigned char *>(src), n);
#else
  copy_forward_inline_t<uint8_t>(static_cast<uint8_t*>(dst), static_cast<const uint8_t*>(src), n);
#endif
}

//! \}

//! \name Memory Ops
//! \{

template<class CombineOp, typename T>
static BL_INLINE void combine(T* dst, const T* src, size_t count) noexcept {
  for (size_t i = 0; i < count; i++)
    dst[i] = CombineOp::op(dst[i], src[i]);
}

template<class CombineOp, typename T>
static BL_INLINE void combine_small(T* dst, const T* src, size_t count) noexcept {
  BL_NOUNROLL
  for (size_t i = 0; i < count; i++)
    dst[i] = CombineOp::op(dst[i], src[i]);
}

//! \}

//! \name Memory Test
//! \{

template<typename T>
static BL_INLINE bool test_small_t(const T* p, size_t count, const T& value) noexcept {
  BL_NOUNROLL
  for (size_t i = 0; i < count; i++)
    if (p[i] != value)
      return false;
  return true;
}

//! \}

} // {anonymous}
} // {MemOps}
} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_SUPPORT_MEMOPS_P_H_INCLUDED
