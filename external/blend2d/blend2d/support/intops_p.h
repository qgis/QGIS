// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_SUPPORT_INTOPS_P_H_INCLUDED
#define BLEND2D_SUPPORT_INTOPS_P_H_INCLUDED

#include <blend2d/support/traits_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

namespace bl {

//! \name Types
//! \{

typedef unsigned char OverflowFlag;

//! \}

//! Utility functions and classes simplifying integer operations.
namespace IntOps {

using BLInternal::IntBySize;
using BLInternal::IntByType;
using BLInternal::UIntBySize;
using BLInternal::UIntByType;

namespace {

//! \name Integer Type Conversion
//! \{

//! Cast an integer `x` to a fixed-width type as defined by <stdint.h>
//!
//! This can help when specializing some functions for a particular type. Since
//! some C/C++ types may overlap (like `long` vs `long long`) it's easier to
//! just cast to a type as defined by <stdint.h> and specialize for it.
template<typename T>
[[nodiscard]]
static BL_INLINE_CONSTEXPR IntBySize<sizeof(T), std::is_unsigned_v<T>> as_std_int(T x) noexcept {
  return (IntBySize<sizeof(T), std::is_unsigned_v<T>>)x;
}

//! Cast an integer `x` to a fixed-width unsigned type as defined by <stdint.h>
template<typename T>
[[nodiscard]]
static BL_INLINE_CONSTEXPR UIntByType<T> asStdUInt(T x) noexcept {
  return (UIntByType<T>)x;
}

//! Cast an integer `x` to either `int32_t`, uint32_t`, `int64_t`, or `uint64_t`.
//!
//! Used to keep a signedness of `T`, but to promote it to at least 32-bit type.
template<typename T>
[[nodiscard]]
static BL_INLINE_CONSTEXPR IntBySize<bl_max<size_t>(sizeof(T), 4), std::is_unsigned_v<T>> asInt32AtLeast(T x) noexcept {
  return (IntBySize<bl_max<size_t>(sizeof(T), 4), std::is_unsigned_v<T>>)x;
}

//! Cast an integer `x` to either `uint32_t` or `uint64_t`.
template<typename T>
[[nodiscard]]
static BL_INLINE_CONSTEXPR UIntBySize<bl_max<size_t>(sizeof(T), 4)> asUInt32AtLeast(T x) noexcept {
  return (UIntBySize<bl_max<size_t>(sizeof(T), 4)>)(UIntByType<T>)x;
}

//! \}

//! \name Byte Swap Operations
//! \{

template<typename T>
[[nodiscard]]
static BL_INLINE T byteSwap32(const T& x) noexcept {
#if defined(__GNUC__) && !defined(BL_BUILD_NO_INTRINSICS)
  return T(uint32_t(__builtin_bswap32(uint32_t(x))));
#elif defined(_MSC_VER) && !defined(BL_BUILD_NO_INTRINSICS)
  return T(uint32_t(_byteswap_ulong(uint32_t(x))));
#else
  return T((uint32_t(x) << 24) | (uint32_t(x) >> 24) | ((uint32_t(x) << 8) & 0x00FF0000u) | ((uint32_t(x) >> 8) & 0x0000FF00));
#endif
}

template<typename T>
[[nodiscard]]
static BL_INLINE T byteSwap24(const T& x) noexcept {
  // This produces always much better code than trying to do a real 24-bit byteswap.
  return T(byteSwap32(uint32_t(x)) >> 8);
}

template<typename T>
[[nodiscard]]
static BL_INLINE T byteSwap16(const T& x) noexcept {
#if defined(_MSC_VER) && !defined(BL_BUILD_NO_INTRINSICS)
  return T(uint16_t(_byteswap_ushort(uint16_t(x))));
#else
  return T((uint16_t(x) << 8) | (uint16_t(x) >> 8));
#endif
}

template<typename T>
[[nodiscard]]
static BL_INLINE T byteSwap64(const T& x) noexcept {
#if defined(__GNUC__) && !defined(BL_BUILD_NO_INTRINSICS)
  return T(uint64_t(__builtin_bswap64(uint64_t(x))));
#elif defined(_MSC_VER) && !defined(BL_BUILD_NO_INTRINSICS)
  return T(uint64_t(_byteswap_uint64(uint64_t(x))));
#else
  return T( (uint64_t(byteSwap32(uint32_t(uint64_t(x) >> 32        )))      ) |
            (uint64_t(byteSwap32(uint32_t(uint64_t(x) & 0xFFFFFFFFu))) << 32) );
#endif
}

template<typename T>
[[nodiscard]]
static BL_INLINE T byte_swap(const T& x) noexcept {
  BL_STATIC_ASSERT(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8);

  if constexpr (sizeof(T) == 1)
    return x;
  else if constexpr (sizeof(T) == 2)
    return byteSwap16(x);
  else if constexpr (sizeof(T) == 4)
    return byteSwap32(x);
  else
    return byteSwap64(x);
}

// TODO: [GLOBAL] REMOVE?
template<typename T> [[nodiscard]] static BL_INLINE T byteSwap16LE(T x) noexcept { return BL_BYTE_ORDER_NATIVE == BL_BYTE_ORDER_LE ? T(x) : T(byteSwap16(uint16_t(x))); }
template<typename T> [[nodiscard]] static BL_INLINE T byteSwap24LE(T x) noexcept { return BL_BYTE_ORDER_NATIVE == BL_BYTE_ORDER_LE ? T(x) : T(byteSwap24(uint32_t(x))); }
template<typename T> [[nodiscard]] static BL_INLINE T byteSwap32LE(T x) noexcept { return BL_BYTE_ORDER_NATIVE == BL_BYTE_ORDER_LE ? T(x) : T(byteSwap32(uint32_t(x))); }
template<typename T> [[nodiscard]] static BL_INLINE T byteSwap64LE(T x) noexcept { return BL_BYTE_ORDER_NATIVE == BL_BYTE_ORDER_LE ? T(x) : T(byteSwap64(uint64_t(x))); }

template<typename T> [[nodiscard]] static BL_INLINE T byteSwap16BE(T x) noexcept { return BL_BYTE_ORDER_NATIVE == BL_BYTE_ORDER_BE ? T(x) : T(byteSwap16(uint16_t(x))); }
template<typename T> [[nodiscard]] static BL_INLINE T byteSwap24BE(T x) noexcept { return BL_BYTE_ORDER_NATIVE == BL_BYTE_ORDER_BE ? T(x) : T(byteSwap24(uint32_t(x))); }
template<typename T> [[nodiscard]] static BL_INLINE T byteSwap32BE(T x) noexcept { return BL_BYTE_ORDER_NATIVE == BL_BYTE_ORDER_BE ? T(x) : T(byteSwap32(uint32_t(x))); }
template<typename T> [[nodiscard]] static BL_INLINE T byteSwap64BE(T x) noexcept { return BL_BYTE_ORDER_NATIVE == BL_BYTE_ORDER_BE ? T(x) : T(byteSwap64(uint64_t(x))); }

//! \}

//! \name Arithmetic Operations
//! \{

//! Returns `0 - x` in a safe way (no undefined behavior), works for both signed and unsigned numbers.
template<typename T>
[[nodiscard]]
static BL_INLINE_CONSTEXPR T negate(const T& x) noexcept {
  using U = std::make_unsigned_t<T>;

  return T(U(0) - U(x));
}

// Look for "Carry-save adder" for more details.
template<typename T>
static BL_INLINE_NODEBUG void csa(T& hi, T& lo, T a, T b, T c) noexcept {
  T u = a ^ b;
  hi = (a & b) | (u & c);
  lo = u ^ c;
}

//! \}

//! \name Bit Manipulation
//! \{

template<typename T>
[[nodiscard]]
BL_INLINE_CONSTEXPR uint32_t bit_size_of() noexcept { return uint32_t(sizeof(T) * 8u); }

template<typename T>
[[nodiscard]]
BL_INLINE_CONSTEXPR T all_ones() noexcept { return T(~T(0)); }

template<typename T>
[[nodiscard]]
BL_INLINE_CONSTEXPR size_t word_count_from_bit_count(size_t n_bits) noexcept {
  return (n_bits + bit_size_of<T>() - 1) / bit_size_of<T>();
}

//! Returns `x << y` (shift left logical) by explicitly casting `x` to an unsigned type and back.
template<typename X, typename Y>
[[nodiscard]]
BL_INLINE_CONSTEXPR X shl(const X& x, const Y& y) noexcept {
  using U = std::make_unsigned_t<X>;
  return X(U(x) << y);
}

//! Returns `x >> y` (shift right logical) by explicitly casting `x` to an unsigned type and back.
template<typename X, typename Y>
[[nodiscard]]
BL_INLINE_CONSTEXPR X shr(const X& x, const Y& y) noexcept {
  using U = std::make_unsigned_t<X>;
  return X(U(x) >> y);
}

//! Returns `x >> y` (shift right arithmetic) by explicitly casting `x` to a signed type and back.
template<typename X, typename Y>
[[nodiscard]]
BL_INLINE_NODEBUG X sar(const X& x, const Y& y) noexcept {
  using S = std::make_signed_t<X>;
  return X(S(x) >> y);
}

template<typename T>
[[nodiscard]]
BL_INLINE_NODEBUG T rol_impl(const T& x, unsigned n) noexcept {
  return shl(x, n % unsigned(sizeof(T) * 8u)) | shr(x, (0u - n) % unsigned(sizeof(T) * 8u)) ;
}

template<typename T>
[[nodiscard]]
BL_INLINE_NODEBUG T ror_impl(const T& x, unsigned n) noexcept {
  return shr(x, n % unsigned(sizeof(T) * 8u)) | shl(x, (0u - n) % unsigned(sizeof(T) * 8u)) ;
}

// MSVC is unable to emit `rol|ror` instruction when `n` is not a constant so we have to help it a bit.
// This  prevents us from using `constexpr`.
#if defined(_MSC_VER)
template<> [[nodiscard]] BL_INLINE_NODEBUG uint8_t rol_impl(const uint8_t& x, unsigned n) noexcept { return uint8_t(_rotl8(x, uint8_t(n))); }
template<> [[nodiscard]] BL_INLINE_NODEBUG uint8_t ror_impl(const uint8_t& x, unsigned n) noexcept { return uint8_t(_rotr8(x, uint8_t(n))); }
template<> [[nodiscard]] BL_INLINE_NODEBUG uint16_t rol_impl(const uint16_t& x, unsigned n) noexcept { return uint16_t(_rotl16(x, uint8_t(n))); }
template<> [[nodiscard]] BL_INLINE_NODEBUG uint16_t ror_impl(const uint16_t& x, unsigned n) noexcept { return uint16_t(_rotr16(x, uint8_t(n))); }
template<> [[nodiscard]] BL_INLINE_NODEBUG uint32_t rol_impl(const uint32_t& x, unsigned n) noexcept { return uint32_t(_rotl(x, int(n))); }
template<> [[nodiscard]] BL_INLINE_NODEBUG uint32_t ror_impl(const uint32_t& x, unsigned n) noexcept { return uint32_t(_rotr(x, int(n))); }
template<> [[nodiscard]] BL_INLINE_NODEBUG uint64_t rol_impl(const uint64_t& x, unsigned n) noexcept { return uint64_t(_rotl64(x, int(n))); }
template<> [[nodiscard]] BL_INLINE_NODEBUG uint64_t ror_impl(const uint64_t& x, unsigned n) noexcept { return uint64_t(_rotr64(x, int(n))); }
#endif

template<typename X, typename Y>
[[nodiscard]]
BL_INLINE_NODEBUG X rol(const X& x, const Y& n) noexcept { return X(rol_impl(asUInt32AtLeast(x), unsigned(n))); }

template<typename X, typename Y>
[[nodiscard]]
BL_INLINE_NODEBUG X ror(const X& x, const Y& n) noexcept { return X(ror_impl(asUInt32AtLeast(x), unsigned(n))); }

//! Returns `x | (x >> y)` - helper used by some bit manipulation helpers.
template<typename X, typename Y>
[[nodiscard]]
BL_INLINE_CONSTEXPR X shr_or(const X& x, const Y& y) noexcept { return X(x | shr(x, y)); }

template<typename X, typename Y, typename... Args>
[[nodiscard]]
BL_INLINE_CONSTEXPR X shr_or(const X& x, const Y& y, Args... args) noexcept { return shr_or(shr_or(x, y), args...); }

//! Fills all trailing bits right from the first most significant bit set.
template<typename T>
[[nodiscard]]
BL_INLINE_CONSTEXPR T fill_trailing_bits(const T& x) noexcept { return T(fill_trailing_bits(UIntByType<T>(x))); }

template<> [[nodiscard]] BL_INLINE_CONSTEXPR uint8_t  fill_trailing_bits(const uint8_t& x) noexcept { return shr_or(x, 1, 2, 4); }
template<> [[nodiscard]] BL_INLINE_CONSTEXPR uint16_t fill_trailing_bits(const uint16_t& x) noexcept { return shr_or(x, 1, 2, 4, 8); }
template<> [[nodiscard]] BL_INLINE_CONSTEXPR uint32_t fill_trailing_bits(const uint32_t& x) noexcept { return shr_or(x, 1, 2, 4, 8, 16); }
template<> [[nodiscard]] BL_INLINE_CONSTEXPR uint64_t fill_trailing_bits(const uint64_t& x) noexcept { return shr_or(x, 1, 2, 4, 8, 16, 32); }

template<typename T, typename N = uint32_t>
[[nodiscard]]
static BL_INLINE_CONSTEXPR T non_zero_lsb_mask(const N& n = 1) noexcept {
  return shr(all_ones<T>(), N(bit_size_of<T>()) - n);
}

template<typename T, typename N = uint32_t>
[[nodiscard]]
static BL_INLINE_CONSTEXPR T non_zero_msb_mask(const N& n = 1) noexcept {
  return sar(shl(T(1), bit_size_of<T>() - 1u), n - 1u);
}

//! Returns a bit-mask that has `x` bit set.
template<typename T, typename Arg>
[[nodiscard]]
BL_INLINE_CONSTEXPR T lsb_bit_at(Arg x) noexcept { return T(T(1u) << x); }

//! Returns a bit-mask that has `x` bit set (multiple bits version).
template<typename T, typename Arg>
[[nodiscard]]
BL_INLINE_CONSTEXPR T lsb_bits_at(Arg x) noexcept { return T(T(1u) << x); }

template<typename T, typename Arg, typename... Args>
[[nodiscard]]
BL_INLINE_CONSTEXPR T lsb_bits_at(Arg x, Args... args) noexcept { return T(lsb_bits_at<T>(x) | lsb_bits_at<T>(args...)); }

//! Returns a bit-mask where all bits are set if the given value `x` is 1, or
//! zero otherwise. Please note that `x` must be either 0 or 1, all other
//! values will produce invalid output.
template<typename T, typename B>
[[nodiscard]]
BL_INLINE_CONSTEXPR T bool_as_mask(const B& x) noexcept { return negate(T(x)); }

//! Tests whether `x` has `n`th bit set.
template<typename T, typename I>
[[nodiscard]]
BL_INLINE_CONSTEXPR bool bit_test(const T& x, const I& i) noexcept {
  using U = std::make_unsigned_t<T>;
  return (U(x) & (U(1) << i)) != 0;
}

//! Tests whether bits specified by `y` are all set in `x`.
template<typename X, typename Y>
[[nodiscard]]
BL_INLINE_CONSTEXPR bool bit_match(const X& x, const Y& y) noexcept { return (x & y) == y; }

template<typename T>
[[nodiscard]]
BL_INLINE_CONSTEXPR bool is_bit_mask_consecutive(const T& x) noexcept {
  using U = std::make_unsigned_t<T>;
  return x != 0 && (U(x) ^ (U(x) + (U(x) & (~U(x) + 1u)))) >= U(x);
}

template<typename T>
[[nodiscard]]
static BL_INLINE T bit_swap(const T& x) noexcept {
  auto v = asUInt32AtLeast(x);

  auto m1 = asUInt32AtLeast(T(0x5555555555555555u & all_ones<T>()));
  auto m2 = asUInt32AtLeast(T(0x3333333333333333u & all_ones<T>()));
  auto m4 = asUInt32AtLeast(T(0x0F0F0F0F0F0F0F0Fu & all_ones<T>()));

  v = ((v >> 1) & m1) | ((v & m1) << 1);
  v = ((v >> 2) & m2) | ((v & m2) << 2);
  v = ((v >> 4) & m4) | ((v & m4) << 4);

  return byte_swap(T(v));
}

//! \}

//! \name Bit Scanning
//! \{

template<typename T>
struct BitScanResult { T x; uint32_t n; };

template<typename T, uint32_t N>
struct BitScanImpl {
  [[nodiscard]]
  static BL_INLINE_CONSTEXPR BitScanResult<T> advance_left(const BitScanResult<T>& data, uint32_t n) noexcept {
    return BitScanResult<T> { data.x << n, data.n + n };
  }

  [[nodiscard]]
  static BL_INLINE_CONSTEXPR BitScanResult<T> advance_right(const BitScanResult<T>& data, uint32_t n) noexcept {
    return BitScanResult<T> { data.x >> n, data.n + n };
  }

  [[nodiscard]]
  static BL_INLINE_CONSTEXPR BitScanResult<T> clz(const BitScanResult<T>& data) noexcept {
    return BitScanImpl<T, N / 2>::clz(advance_left(data, data.x & (all_ones<T>() << (bit_size_of<T>() - N)) ? uint32_t(0) : N));
  }

  [[nodiscard]]
  static BL_INLINE_CONSTEXPR BitScanResult<T> ctz(const BitScanResult<T>& data) noexcept {
    return BitScanImpl<T, N / 2>::ctz(advance_right(data, data.x & (all_ones<T>() >> (bit_size_of<T>() - N)) ? uint32_t(0) : N));
  }
};

template<typename T>
struct BitScanImpl<T, 0> {
  [[nodiscard]]
  static BL_INLINE_CONSTEXPR BitScanResult<T> clz(const BitScanResult<T>& ctx) noexcept {
    return BitScanResult<T> { 0, ctx.n - uint32_t(ctx.x >> (bit_size_of<T>() - 1)) };
  }

  [[nodiscard]]
  static BL_INLINE_CONSTEXPR BitScanResult<T> ctz(const BitScanResult<T>& ctx) noexcept {
    return BitScanResult<T> { 0, ctx.n - uint32_t(ctx.x & 0x1) };
  }
};

template<typename T>
[[nodiscard]]
static BL_INLINE_CONSTEXPR uint32_t clz_fallback(const T& x) noexcept {
  return BitScanImpl<T, bit_size_of<T>() / 2u>::clz(BitScanResult<T>{x, 1}).n;
}

template<typename T>
[[nodiscard]]
static BL_INLINE_CONSTEXPR uint32_t ctz_fallback(const T& x) noexcept {
  return BitScanImpl<T, bit_size_of<T>() / 2u>::ctz(BitScanResult<T>{x, 1}).n;
}

template<typename T> [[nodiscard]] BL_INLINE_CONSTEXPR uint32_t clz_static(const T& x) noexcept { return clz_fallback(asUInt32AtLeast(x)); }
template<typename T> [[nodiscard]] BL_INLINE_CONSTEXPR uint32_t ctz_static(const T& x) noexcept { return ctz_fallback(asUInt32AtLeast(x)); }

template<typename T> [[nodiscard]] BL_INLINE_NODEBUG uint32_t clz_impl(const T& x) noexcept { return clz_static(x); }
template<typename T> [[nodiscard]] BL_INLINE_NODEBUG uint32_t ctz_impl(const T& x) noexcept { return ctz_static(x); }

#if !defined(BL_BUILD_NO_INTRINSICS)
# if defined(__GNUC__)
template<> [[nodiscard]] BL_INLINE_NODEBUG uint32_t clz_impl(const uint32_t& x) noexcept { return uint32_t(__builtin_clz(x)); }
template<> [[nodiscard]] BL_INLINE_NODEBUG uint32_t clz_impl(const uint64_t& x) noexcept { return uint32_t(__builtin_clzll(x)); }
template<> [[nodiscard]] BL_INLINE_NODEBUG uint32_t ctz_impl(const uint32_t& x) noexcept { return uint32_t(__builtin_ctz(x)); }
template<> [[nodiscard]] BL_INLINE_NODEBUG uint32_t ctz_impl(const uint64_t& x) noexcept { return uint32_t(__builtin_ctzll(x)); }
# elif defined(_MSC_VER)
template<> [[nodiscard]] BL_INLINE_NODEBUG uint32_t clz_impl(const uint32_t& x) noexcept { unsigned long i; _BitScanReverse(&i, x); return uint32_t(i ^ 31); }
template<> [[nodiscard]] BL_INLINE_NODEBUG uint32_t ctz_impl(const uint32_t& x) noexcept { unsigned long i; _BitScanForward(&i, x); return uint32_t(i); }
#  if BL_TARGET_ARCH_X86 == 64 || BL_TARGET_ARCH_ARM == 64
template<> [[nodiscard]] BL_INLINE_NODEBUG uint32_t clz_impl(const uint64_t& x) noexcept { unsigned long i; _BitScanReverse64(&i, x); return uint32_t(i ^ 63); }
template<> [[nodiscard]] BL_INLINE_NODEBUG uint32_t ctz_impl(const uint64_t& x) noexcept { unsigned long i; _BitScanForward64(&i, x); return uint32_t(i); }
#  endif
# endif
#endif

//! Counts leading zeros in `x`.
//!
//! \note If the input is zero the result is undefined.
template<typename T>
[[nodiscard]]
static BL_INLINE_NODEBUG uint32_t clz(T x) noexcept { return clz_impl(asUInt32AtLeast(x)); }

//! Counts trailing zeros in `x`.
//!
//! \note If the input is zero the result is undefined.
template<typename T>
[[nodiscard]]
static BL_INLINE_NODEBUG uint32_t ctz(T x) noexcept { return ctz_impl(asUInt32AtLeast(x)); }

template<typename T>
[[nodiscard]]
static BL_INLINE_CONSTEXPR uint32_t bit_shift_of(const T& x) noexcept { return ctz_static(x); }

//! \}

//! \name Bit Counting
//! \{

// Based on the following resource:
//   http://graphics.stanford.edu/~seander/bithacks.html
//
// Alternatively, for a very small number of bits in `x`:
//   uint32_t n = 0;
//   while (x) {
//     x &= x - 1;
//     n++;
//   }
//   return n;
template<typename T>
[[nodiscard]]
static BL_INLINE uint32_t pop_count_static(const T& x) noexcept {
  using U = std::make_unsigned_t<T>;

  const U m1 = U(0x5555555555555555u & all_ones<U>());
  const U m2 = U(0x3333333333333333u & all_ones<U>());
  const U m4 = U(0x0F0F0F0F0F0F0F0Fu & all_ones<U>());
  const U mX = U(0x0101010101010101u & all_ones<U>());

  U u = U(x);
  u -= ((u >> 1) & m1);
  u  = ((u >> 2) & m2) + (u & m2);
  u  = ((u >> 4) + u) & m4;

  if (sizeof(T) > 1)
    return uint32_t((u * mX) >> (bit_size_of<T>() - 8));
  else
    return uint32_t(u & 0xFFu);
}

[[nodiscard]]
static BL_INLINE_NODEBUG uint32_t pop_count_impl(uint32_t x) noexcept {
#if defined(__GNUC__)
  return uint32_t(__builtin_popcount(x));
#elif defined(BL_TARGET_OPT_SSE4_2)
  return uint32_t(_mm_popcnt_u32(x));
#elif defined(_MSC_VER) && defined(BL_TARGET_OPT_POPCNT)
  return __popcnt(x);
#else
  return pop_count_static(x);
#endif
}

[[nodiscard]]
static BL_INLINE_NODEBUG uint32_t pop_count_impl(uint64_t x) noexcept {
#if defined(__GNUC__)
  return uint32_t(__builtin_popcountll(x));
#elif defined(BL_TARGET_OPT_SSE4_2) && BL_TARGET_ARCH_BITS >= 64
  return uint32_t(_mm_popcnt_u64(x));
#elif defined(_MSC_VER) && defined(BL_TARGET_OPT_POPCNT) && BL_TARGET_ARCH_BITS >= 64
  return uint32_t(__popcnt64(x));
#elif BL_TARGET_ARCH_BITS >= 64
  return pop_count_impl(uint32_t(x >> 32)) + pop_count_impl(uint32_t(x & 0xFFFFFFFFu));
#else
  return pop_count_static(x);
#endif
}

//! Calculates count of bits in `x`.
template<typename T>
[[nodiscard]]
static BL_INLINE_NODEBUG uint32_t pop_count(T x) noexcept { return pop_count_impl(asUInt32AtLeast(x)); }

// Simple PopCount context designed to take advantage of HW PopCount support.
template<typename T>
class PopCounterSimple {
public:
  uint32_t _counter = 0;

  BL_INLINE_NODEBUG void reset() noexcept {
    _counter = 0;
  }

  BL_INLINE_NODEBUG uint32_t get() const noexcept {
    return _counter;
  }

  BL_INLINE_NODEBUG void add_population(uint32_t v) noexcept {
    _counter += v;
  }

  BL_INLINE void add_item(const T& x) noexcept {
    _counter += pop_count(x);
  }

  BL_INLINE void add_array(const T* data, size_t n) noexcept {
    while (n) {
      _counter += pop_count(data[0]);
      data++;
      n--;
    }
  }
};

// Harley-Seal PopCount from Hacker's Delight, Second Edition.
//
// This is one of the best implementation if the hardware doesn't provide POPCNT instruction.
template<typename T>
class PopCounterHarleySeal {
public:
  uint32_t _counter = 0;
  T _ones = 0;
  T _twos = 0;
  T _fours = 0;

  BL_INLINE void reset() noexcept {
    _counter = 0;
    _ones = 0;
    _twos = 0;
    _fours = 0;
  }

  BL_INLINE uint32_t get() const noexcept {
    return _counter + 4 * pop_count(_fours) + 2 * pop_count(_twos) + pop_count(_ones);
  }

  BL_INLINE void add_population(uint32_t v) noexcept {
    _counter += v;
  }

  BL_INLINE void add_item(const T& x) noexcept {
    _counter += pop_count(x);
  }

  BL_INLINE void add_array(const T* data, size_t n) noexcept {
    uint32_t eights_count = 0;
    while (n >= 8) {
      T twosA, twosB;
      T foursA, foursB;
      T eights;

      csa(twosA, _ones, _ones, data[0], data[1]);
      csa(twosB, _ones, _ones, data[2], data[3]);
      csa(foursA, _twos, _twos, twosA, twosB);
      csa(twosA, _ones, _ones, data[4], data[5]);
      csa(twosB, _ones, _ones, data[6], data[7]);
      csa(foursB, _twos, _twos, twosA, twosB);
      csa(eights, _fours, _fours, foursA, foursB);

      eights_count += pop_count(eights);
      data += 8;
      n -= 8;
    }

    _counter += 8 * eights_count;
    while (n) {
      _counter += pop_count(data[0]);
      data++;
      n--;
    }
  }
};

#if defined(BL_TARGET_OPT_POPCNT)
template<typename T>
using PopCounter = PopCounterSimple<T>;
#else
template<typename T>
using PopCounter = PopCounterHarleySeal<T>;
#endif

//! \}

//! \name Alignment Operations
//! \{

template<typename X, typename Y>
[[nodiscard]]
static BL_INLINE_CONSTEXPR bool is_aligned(const X& x, const Y& alignment) noexcept {
  using U = UIntByType<X>;
  return ((U)x % (U)alignment) == 0;
}

//! Tests whether the `x` is a power of two (only one bit is set).
template<typename T>
[[nodiscard]]
static BL_INLINE_CONSTEXPR bool is_power_of_2(const T& x) noexcept {
  using U = std::make_unsigned_t<T>;
  U x_minus_1 = U(U(x) - U(1));
  return U(U(x) ^ x_minus_1) > x_minus_1;
}

template<typename X, typename Y>
[[nodiscard]]
static BL_INLINE_CONSTEXPR X align_up(const X& x, const Y& alignment) noexcept {
  using U = UIntByType<X>;
  return (X)( ((U)x + ((U)(alignment) - 1u)) & ~((U)(alignment) - 1u) );
}

//! Returns zero or a positive difference between `x` and `x` aligned to `alignment`.
template<typename X, typename Y>
[[nodiscard]]
static BL_INLINE_CONSTEXPR X align_up_diff(const X& x, const Y& alignment) noexcept {
  using U = UIntByType<X>;
  return X( U(U(0) - U(x)) & U(alignment - 1) );
}

template<typename T>
[[nodiscard]]
static BL_INLINE_CONSTEXPR T align_up_power_of_2(const T& x) noexcept {
  using U = UIntByType<T>;
  return T( fill_trailing_bits(U(x) - 1u) + 1u );
}

template<typename X, typename Y>
[[nodiscard]]
static BL_INLINE_CONSTEXPR X align_down(const X& x, const Y& alignment) noexcept {
  using U = UIntByType<X>;
  return X( (U)x & ~((U)(alignment) - 1u) );
}

//! \}

//! \name Arithmetic Operations
//! \{

template<typename T>
BL_INLINE T add_overflow_fallback(T x, T y, OverflowFlag* of) noexcept {
  using U = std::make_unsigned_t<T>;

  U result = U(U(x) + U(y));
  *of = OverflowFlag(*of | OverflowFlag(std::is_unsigned_v<T> ? result < U(x) : T((U(x) ^ ~U(y)) & (U(x) ^ result)) < 0));
  return T(result);
}

template<typename T>
BL_INLINE T sub_overflow_fallback(T x, T y, OverflowFlag* of) noexcept {
  using U = std::make_unsigned_t<T>;

  U result = U(U(x) - U(y));
  *of = OverflowFlag(*of | OverflowFlag(std::is_unsigned_v<T> ? result > U(x) : T((U(x) ^ U(y)) & (U(x) ^ result)) < 0));
  return T(result);
}

template<typename T>
BL_INLINE T mul_overflow_fallback(T x, T y, OverflowFlag* of) noexcept {
  using I = IntBySize<sizeof(T) * 2, std::is_unsigned_v<T>>;
  using U = UIntByType<I>;

  U mask = U(Traits::max_value<std::make_unsigned_t<T>>());
  if constexpr (std::is_signed_v<T>) {
    U prod = U(I(x)) * U(I(y));
    *of = OverflowFlag(*of | OverflowFlag(I(prod) < I(Traits::min_value<T>()) || I(prod) > Traits::max_value<T>()));
    return T(I(prod & mask));
  }
  else {
    U prod = U(x) * U(y);
    *of = OverflowFlag(*of | OverflowFlag((prod & ~mask) != 0));
    return T(prod & mask);
  }
}

template<>
BL_INLINE int64_t mul_overflow_fallback(int64_t x, int64_t y, OverflowFlag* of) noexcept {
  int64_t result = int64_t(uint64_t(x) * uint64_t(y));
  *of = OverflowFlag(*of | OverflowFlag(x && (result / x != y)));
  return result;
}

template<>
BL_INLINE uint64_t mul_overflow_fallback(uint64_t x, uint64_t y, OverflowFlag* of) noexcept {
  uint64_t result = x * y;
  *of = OverflowFlag(*of | OverflowFlag(y != 0 && Traits::max_value<uint64_t>() / y < x));
  return result;
}

// These can be specialized.
template<typename T> BL_INLINE T add_overflow_impl(const T& x, const T& y, OverflowFlag* of) noexcept { return add_overflow_fallback(x, y, of); }
template<typename T> BL_INLINE T sub_overflow_impl(const T& x, const T& y, OverflowFlag* of) noexcept { return sub_overflow_fallback(x, y, of); }
template<typename T> BL_INLINE T mul_overflow_impl(const T& x, const T& y, OverflowFlag* of) noexcept { return mul_overflow_fallback(x, y, of); }

#if defined(__GNUC__) && !defined(BL_BUILD_NO_INTRINSICS)
#define BL_ARITH_OVERFLOW_SPECIALIZE(FUNC, T, RESULT_T, BUILTIN)                  \
  template<>                                                                      \
  BL_INLINE_NODEBUG T FUNC(const T& x, const T& y, OverflowFlag* of) noexcept { \
    RESULT_T result;                                                              \
    *of = OverflowFlag(*of | (BUILTIN((RESULT_T)x, (RESULT_T)y, &result)));     \
    return T(result);                                                             \
  }
BL_ARITH_OVERFLOW_SPECIALIZE(add_overflow_impl, int32_t , int               , __builtin_sadd_overflow  )
BL_ARITH_OVERFLOW_SPECIALIZE(add_overflow_impl, uint32_t, unsigned int      , __builtin_uadd_overflow  )
BL_ARITH_OVERFLOW_SPECIALIZE(add_overflow_impl, int64_t , long long         , __builtin_saddll_overflow)
BL_ARITH_OVERFLOW_SPECIALIZE(add_overflow_impl, uint64_t, unsigned long long, __builtin_uaddll_overflow)
BL_ARITH_OVERFLOW_SPECIALIZE(sub_overflow_impl, int32_t , int               , __builtin_ssub_overflow  )
BL_ARITH_OVERFLOW_SPECIALIZE(sub_overflow_impl, uint32_t, unsigned int      , __builtin_usub_overflow  )
BL_ARITH_OVERFLOW_SPECIALIZE(sub_overflow_impl, int64_t , long long         , __builtin_ssubll_overflow)
BL_ARITH_OVERFLOW_SPECIALIZE(sub_overflow_impl, uint64_t, unsigned long long, __builtin_usubll_overflow)
BL_ARITH_OVERFLOW_SPECIALIZE(mul_overflow_impl, int32_t , int               , __builtin_smul_overflow  )
BL_ARITH_OVERFLOW_SPECIALIZE(mul_overflow_impl, uint32_t, unsigned int      , __builtin_umul_overflow  )

// Do not use these in 32-bit mode as some compilers would instead emit a call
// into a helper function (__mulodi4, for example), which could then fail if
// the resulting binary is not linked to the compiler runtime library.
#if BL_TARGET_ARCH_BITS == 64
BL_ARITH_OVERFLOW_SPECIALIZE(mul_overflow_impl, int64_t , long long         , __builtin_smulll_overflow)
BL_ARITH_OVERFLOW_SPECIALIZE(mul_overflow_impl, uint64_t, unsigned long long, __builtin_umulll_overflow)
#endif

#undef BL_ARITH_OVERFLOW_SPECIALIZE
#endif

// There is a bug in MSVC that makes these specializations unusable, maybe in the future...
#if defined(_MSC_VER) && 0
#define BL_ARITH_OVERFLOW_SPECIALIZE(FUNC, T, ALT_T, BUILTIN)                 \
  template<>                                                                  \
  BL_INLINE_NODEBUG T FUNC(T x, T y, OverflowFlag* of) noexcept {           \
    ALT_T result;                                                             \
    *of = OverflowFlag(*of | (BUILTIN(0, (ALT_T)x, (ALT_T)y, &result)));    \
    return T(result);                                                         \
  }
BL_ARITH_OVERFLOW_SPECIALIZE(add_overflow_impl, uint32_t, unsigned int      , _addcarry_u32 )
BL_ARITH_OVERFLOW_SPECIALIZE(sub_overflow_impl, uint32_t, unsigned int      , _subborrow_u32)
#if ARCH_BITS >= 64
BL_ARITH_OVERFLOW_SPECIALIZE(add_overflow_impl, uint64_t, unsigned __int64  , _addcarry_u64 )
BL_ARITH_OVERFLOW_SPECIALIZE(sub_overflow_impl, uint64_t, unsigned __int64  , _subborrow_u64)
#endif
#undef BL_ARITH_OVERFLOW_SPECIALIZE
#endif

template<typename T>
static BL_INLINE T add_overflow(const T& x, const T& y, OverflowFlag* of) noexcept { return T(add_overflow_impl(as_std_int(x), as_std_int(y), of)); }

template<typename T>
static BL_INLINE T sub_overflow(const T& x, const T& y, OverflowFlag* of) noexcept { return T(sub_overflow_impl(as_std_int(x), as_std_int(y), of)); }

template<typename T>
static BL_INLINE T mul_overflow(const T& x, const T& y, OverflowFlag* of) noexcept { return T(mul_overflow_impl(as_std_int(x), as_std_int(y), of)); }

template<typename T>
[[nodiscard]]
static BL_INLINE T uadd_saturate(const T& x, const T& y) noexcept {
  OverflowFlag of{};
  T result = add_overflow(x, y, &of);
  return T(result | bool_as_mask<T>(of));
}

template<typename T>
[[nodiscard]]
static BL_INLINE T usub_saturate(const T& x, const T& y) noexcept {
  OverflowFlag of{};
  T result = sub_overflow(x, y, &of);
  return T(result & bool_as_mask<T>(!of));
}

template<typename T>
[[nodiscard]]
static BL_INLINE T umul_saturate(const T& x, const T& y) noexcept {
  OverflowFlag of{};
  T result = mul_overflow(x, y, &of);
  return T(result | bool_as_mask<T>(of));
}

//! \}

//! \name Clamp
//! \{

template<typename SrcT, typename DstT>
[[nodiscard]]
static BL_INLINE_CONSTEXPR DstT clamp_to_impl(const SrcT& x, const DstT& y) noexcept {
  using U = std::make_unsigned_t<SrcT>;
  return U(x) <= U(y) ? DstT(x)
                      : std::is_unsigned_v<SrcT> ? DstT(y) : DstT(SrcT(y) & SrcT(sar(negate(x), sizeof(SrcT) * 8 - 1)));
}

//! Clamp a value `x` to a byte (unsigned 8-bit type).
template<typename T>
[[nodiscard]]
static BL_INLINE_CONSTEXPR uint8_t clamp_to_byte(const T& x) noexcept {
  return clamp_to_impl<T, uint8_t>(x, uint8_t(0xFFu));
}

//! Clamp a value `x` to a word (unsigned 16-bit type).
template<typename T>
[[nodiscard]]
static BL_INLINE_CONSTEXPR uint16_t clamp_to_word(const T& x) noexcept {
  return clamp_to_impl<T, uint16_t>(x, uint16_t(0xFFFFu));
}

//! \}

//! \name Positive Modulo
//! \{

//! Returns a positive modulo - similar to `x % y`, but for example `-4 % 3` would return `2` instead of `-1`.
template<typename T>
[[nodiscard]]
static BL_INLINE_NODEBUG T pmod(const T& x, const T& y) noexcept {
  T result = x % y;
  return std::is_unsigned_v<T> ? result : result + (y + (result >> (bit_size_of<T>() - 1)));
}

//! \}

} // {anonymous}
} // {IntOps}
} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_SUPPORT_INTOPS_P_H_INCLUDED
