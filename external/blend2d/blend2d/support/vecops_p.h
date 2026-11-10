// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_SUPPORT_VECOPS_P_H_INCLUDED
#define BLEND2D_SUPPORT_VECOPS_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/simd/simd_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/traits_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_support
//! \{

namespace bl {

//! \name Math Vectorized
//! \{

// Vec2 Storage
// ============

//! A storage type of a vector of two values.
template<typename T>
struct BL_ALIGN_TYPE(Vec2Data, sizeof(T) * 2u) {
  using Type = T;

  T x, y;
};

//! A storage type of a vector of three values.
template<typename T>
struct BL_ALIGN_TYPE(Vec3Data, sizeof(T)) {
  using Type = T;

  T x, y, z;
};

//! A storage type of a vector of four values.
template<typename T>
struct BL_ALIGN_TYPE(Vec4Data, sizeof(T) * 2u) {
  using Type = T;

  T x, y, z, w;
};

namespace Vec {
namespace {

// Scalar Operations
// =================

template<typename T> BL_INLINE_NODEBUG T not_(const T& a) noexcept { return bl_bit_cast<T>(~bl_bit_cast<UIntByType<T>>(a)); }
template<typename T> BL_INLINE_NODEBUG T msb_mask(const T& a) noexcept { return bl_bit_cast<T>(bl_bit_cast<IntByType<T>>(a) >> (IntOps::bit_size_of<T>() - 1u)); }

template<typename T> BL_INLINE_NODEBUG T abs(const T& a) noexcept {
  UIntByType<T> msk = UIntByType<T>(msb_mask(a));
  return T((UIntByType<T>(a) ^ msk) - msk);
}

template<> BL_INLINE_NODEBUG float abs(const float& a) noexcept { return bl_abs(a); }
template<> BL_INLINE_NODEBUG double abs(const double& a) noexcept { return bl_abs(a); }

template<typename T> BL_INLINE_NODEBUG T and_(const T& a, const T& b) noexcept { return bl_bit_cast<T>(bl_bit_cast<UIntByType<T>>(a) & bl_bit_cast<UIntByType<T>>(b)); }
template<typename T> BL_INLINE_NODEBUG T or_(const T& a, const T& b) noexcept { return bl_bit_cast<T>(bl_bit_cast<UIntByType<T>>(a) | bl_bit_cast<UIntByType<T>>(b)); }
template<typename T> BL_INLINE_NODEBUG T xor_(const T& a, const T& b) noexcept { return bl_bit_cast<T>(bl_bit_cast<UIntByType<T>>(a) ^ bl_bit_cast<UIntByType<T>>(b)); }
template<typename T> BL_INLINE_NODEBUG T min(const T& a, const T& b) noexcept { return bl_min(a, b); }
template<typename T> BL_INLINE_NODEBUG T max(const T& a, const T& b) noexcept { return bl_max(a, b); }

template<typename T> BL_INLINE_NODEBUG UIntByType<T> cmp_eq(const T& a, const T& b) noexcept { return UIntByType<T>(0) - UIntByType<T>(a == b); }
template<typename T> BL_INLINE_NODEBUG UIntByType<T> cmp_ne(const T& a, const T& b) noexcept { return UIntByType<T>(0) - UIntByType<T>(a != b); }
template<typename T> BL_INLINE_NODEBUG UIntByType<T> cmp_gt(const T& a, const T& b) noexcept { return UIntByType<T>(0) - UIntByType<T>(a >  b); }
template<typename T> BL_INLINE_NODEBUG UIntByType<T> cmp_ge(const T& a, const T& b) noexcept { return UIntByType<T>(0) - UIntByType<T>(a >= b); }
template<typename T> BL_INLINE_NODEBUG UIntByType<T> cmp_lt(const T& a, const T& b) noexcept { return UIntByType<T>(0) - UIntByType<T>(a <  b); }
template<typename T> BL_INLINE_NODEBUG UIntByType<T> cmp_le(const T& a, const T& b) noexcept { return UIntByType<T>(0) - UIntByType<T>(a <= b); }

// Vec2 Definitions
// ================

template<typename T>
struct Vec2 {
  using Type = T;

  Type x, y;

  BL_INLINE Vec2() noexcept = default;
  BL_INLINE Vec2(const Vec2& other) noexcept = default;

  BL_INLINE Vec2(const Vec2Data<Type>& other) noexcept
    : x(other.x),
      y(other.y) {}

  BL_INLINE Vec2(const Type& v0, const Type& v1) noexcept
    : x(v0),
      y(v1) {}

  BL_INLINE Vec2& operator=(const Vec2& other) noexcept = default;
};

typedef Vec2<int32_t> i32x2;
typedef Vec2<uint32_t> u32x2;
typedef Vec2<int64_t> i64x2;
typedef Vec2<uint64_t> u64x2;
typedef Vec2<float> f32x2;
typedef Vec2<double> f64x2;

// Vec2 Operations - Int64 & UInt64 (SIMD)
// =======================================

#if BL_SIMD_WIDTH_I >= 128 && BL_SIMD_WIDTH_D >= 128
template<>
struct Vec2<uint64_t> {
  using Type = uint64_t;
  using SIMDType = SIMD::Vec2xU64;

  union {
    struct { Type x, y; };
    SIMDType v;
  };

  BL_INLINE Vec2() noexcept = default;
  BL_INLINE Vec2(const Vec2& other) noexcept = default;

  BL_INLINE Vec2(const SIMDType& vec) noexcept
    : v(vec) {}

  BL_INLINE Vec2(const Vec2Data<Type>& other) noexcept
    : v(SIMD::make128_u64(other.y, other.x)) {}

  BL_INLINE Vec2(const Type& v0, const Type& v1) noexcept
    : v(SIMD::make128_u64(v1, v0)) {}

  BL_INLINE Vec2& operator=(const SIMDType& vec) noexcept { v = vec; return *this; }
  BL_INLINE Vec2& operator=(const Vec2& other) noexcept { v = other.v; return *this; }
};

BL_INLINE_NODEBUG u64x2 operator+(const u64x2& a, const u64x2& b) noexcept { return u64x2(a.v + b.v); }
BL_INLINE_NODEBUG u64x2 operator-(const u64x2& a, const u64x2& b) noexcept { return u64x2(a.v - b.v); }
BL_INLINE_NODEBUG u64x2 operator*(const u64x2& a, const u64x2& b) noexcept { return u64x2(a.v * b.v); }
BL_INLINE_NODEBUG u64x2 operator&(const u64x2& a, const u64x2& b) noexcept { return u64x2(a.v & b.v); }
BL_INLINE_NODEBUG u64x2 operator|(const u64x2& a, const u64x2& b) noexcept { return u64x2(a.v | b.v); }
BL_INLINE_NODEBUG u64x2 operator^(const u64x2& a, const u64x2& b) noexcept { return u64x2(a.v ^ b.v); }

BL_INLINE_NODEBUG u64x2 operator==(const u64x2& a, const u64x2& b) noexcept { return u64x2(SIMD::cmp_eq(a.v, b.v)); }
BL_INLINE_NODEBUG u64x2 operator!=(const u64x2& a, const u64x2& b) noexcept { return u64x2(SIMD::cmp_ne(a.v, b.v)); }
BL_INLINE_NODEBUG u64x2 operator> (const u64x2& a, const u64x2& b) noexcept { return u64x2(SIMD::cmp_gt(a.v, b.v)); }
BL_INLINE_NODEBUG u64x2 operator>=(const u64x2& a, const u64x2& b) noexcept { return u64x2(SIMD::cmp_ge(a.v, b.v)); }
BL_INLINE_NODEBUG u64x2 operator< (const u64x2& a, const u64x2& b) noexcept { return u64x2(SIMD::cmp_lt(a.v, b.v)); }
BL_INLINE_NODEBUG u64x2 operator<=(const u64x2& a, const u64x2& b) noexcept { return u64x2(SIMD::cmp_le(a.v, b.v)); }

BL_INLINE_NODEBUG u64x2 not_(const u64x2& a) noexcept { return u64x2(SIMD::not_(a.v)); }
BL_INLINE_NODEBUG u64x2 abs(const u64x2& a) noexcept { return a; }
BL_INLINE_NODEBUG u64x2 swap(const u64x2& a) noexcept { return u64x2(SIMD::swap_u64(a.v)); }
BL_INLINE_NODEBUG u64x2 msb_mask(const u64x2& a) noexcept { return u64x2(SIMD::srai_i64<63>(a.v)); }

BL_INLINE_NODEBUG u64x2 min(const u64x2& a, const u64x2& b) noexcept { return u64x2(SIMD::min(a.v, b.v)); }
BL_INLINE_NODEBUG u64x2 max(const u64x2& a, const u64x2& b) noexcept { return u64x2(SIMD::max(a.v, b.v)); }

template<>
struct Vec2<int64_t> {
  using Type = int64_t;
  using SIMDType = SIMD::Vec2xI64;

  union {
    struct { Type x, y; };
    SIMDType v;
  };

  BL_INLINE Vec2() noexcept = default;
  BL_INLINE Vec2(const Vec2& other) noexcept = default;

  BL_INLINE Vec2(const SIMDType& vec) noexcept
    : v(vec) {}

  BL_INLINE Vec2(const Vec2Data<Type>& other) noexcept
    : v(SIMD::make128_i64(other.y, other.x)) {}

  BL_INLINE Vec2(const Type& v0, const Type& v1) noexcept
    : v(SIMD::make128_i64(v1, v0)) {}

  BL_INLINE Vec2& operator=(const SIMDType& vec) noexcept { v = vec; return *this; }
  BL_INLINE Vec2& operator=(const Vec2& other) noexcept { v = other.v; return *this; }
};

BL_INLINE_NODEBUG i64x2 operator+(const i64x2& a, const i64x2& b) noexcept { return i64x2(a.v + b.v); }
BL_INLINE_NODEBUG i64x2 operator-(const i64x2& a, const i64x2& b) noexcept { return i64x2(a.v - b.v); }
BL_INLINE_NODEBUG i64x2 operator*(const i64x2& a, const i64x2& b) noexcept { return i64x2(a.v * b.v); }
BL_INLINE_NODEBUG i64x2 operator&(const i64x2& a, const i64x2& b) noexcept { return i64x2(a.v & b.v); }
BL_INLINE_NODEBUG i64x2 operator|(const i64x2& a, const i64x2& b) noexcept { return i64x2(a.v | b.v); }
BL_INLINE_NODEBUG i64x2 operator^(const i64x2& a, const i64x2& b) noexcept { return i64x2(a.v ^ b.v); }

BL_INLINE_NODEBUG u64x2 operator==(const i64x2& a, const i64x2& b) noexcept { return u64x2(SIMD::vec_cast<SIMD::Vec2xU64>(SIMD::cmp_eq(a.v, b.v))); }
BL_INLINE_NODEBUG u64x2 operator!=(const i64x2& a, const i64x2& b) noexcept { return u64x2(SIMD::vec_cast<SIMD::Vec2xU64>(SIMD::cmp_ne(a.v, b.v))); }
BL_INLINE_NODEBUG u64x2 operator> (const i64x2& a, const i64x2& b) noexcept { return u64x2(SIMD::vec_cast<SIMD::Vec2xU64>(SIMD::cmp_gt(a.v, b.v))); }
BL_INLINE_NODEBUG u64x2 operator>=(const i64x2& a, const i64x2& b) noexcept { return u64x2(SIMD::vec_cast<SIMD::Vec2xU64>(SIMD::cmp_ge(a.v, b.v))); }
BL_INLINE_NODEBUG u64x2 operator< (const i64x2& a, const i64x2& b) noexcept { return u64x2(SIMD::vec_cast<SIMD::Vec2xU64>(SIMD::cmp_lt(a.v, b.v))); }
BL_INLINE_NODEBUG u64x2 operator<=(const i64x2& a, const i64x2& b) noexcept { return u64x2(SIMD::vec_cast<SIMD::Vec2xU64>(SIMD::cmp_le(a.v, b.v))); }

BL_INLINE_NODEBUG i64x2 not_(const i64x2& a) noexcept { return i64x2(SIMD::not_(a.v)); }
BL_INLINE_NODEBUG i64x2 abs(const i64x2& a) noexcept { return i64x2(SIMD::abs(a.v)); }
BL_INLINE_NODEBUG i64x2 swap(const i64x2& a) noexcept { return i64x2(SIMD::swap_u64(a.v)); }
BL_INLINE_NODEBUG i64x2 msb_mask(const i64x2& a) noexcept { return i64x2(SIMD::srai_i64<63>(a.v)); }

BL_INLINE_NODEBUG i64x2 min(const i64x2& a, const i64x2& b) noexcept { return i64x2(SIMD::min(a.v, b.v)); }
BL_INLINE_NODEBUG i64x2 max(const i64x2& a, const i64x2& b) noexcept { return i64x2(SIMD::max(a.v, b.v)); }

#endif // BL_SIMD_WIDTH_I >= 128 && BL_SIMD_WIDTH_D >= 128

// Vec2 Operations - Float64 (SIMD)
// ================================

#if BL_SIMD_WIDTH_I >= 128 && BL_SIMD_WIDTH_D >= 128
template<>
struct Vec2<double> {
  using Type = double;
  using SIMDType = SIMD::Vec2xF64;

  union {
    struct { Type x, y; };
    SIMDType v;
  };

  BL_INLINE Vec2() noexcept = default;
  BL_INLINE Vec2(const Vec2& other) noexcept = default;

  BL_INLINE Vec2(const SIMDType& vec) noexcept
    : v(vec) {}

  BL_INLINE Vec2(const Vec2Data<Type>& other) noexcept
    : v(SIMD::make128_f64(other.y, other.x)) {}

  BL_INLINE Vec2(const Type& v0, const Type& v1) noexcept
    : v(SIMD::make128_f64(v1, v0)) {}

  BL_INLINE Vec2& operator=(const SIMDType& vec) noexcept { v = vec; return *this; }
  BL_INLINE Vec2& operator=(const Vec2& other) noexcept { v = other.v; return *this; }
};

BL_INLINE_NODEBUG f64x2 operator+(const f64x2& a, const f64x2& b) noexcept { return f64x2(a.v + b.v); }
BL_INLINE_NODEBUG f64x2 operator-(const f64x2& a, const f64x2& b) noexcept { return f64x2(a.v - b.v); }
BL_INLINE_NODEBUG f64x2 operator*(const f64x2& a, const f64x2& b) noexcept { return f64x2(a.v * b.v); }
BL_INLINE_NODEBUG f64x2 operator/(const f64x2& a, const f64x2& b) noexcept { return f64x2(a.v / b.v); }
BL_INLINE_NODEBUG f64x2 operator&(const f64x2& a, const f64x2& b) noexcept { return f64x2(a.v & b.v); }
BL_INLINE_NODEBUG f64x2 operator|(const f64x2& a, const f64x2& b) noexcept { return f64x2(a.v | b.v); }
BL_INLINE_NODEBUG f64x2 operator^(const f64x2& a, const f64x2& b) noexcept { return f64x2(a.v ^ b.v); }

BL_INLINE_NODEBUG u64x2 operator==(const f64x2& a, const f64x2& b) noexcept { return u64x2(SIMD::vec_cast<SIMD::Vec2xU64>(SIMD::cmp_eq(a.v, b.v))); }
BL_INLINE_NODEBUG u64x2 operator!=(const f64x2& a, const f64x2& b) noexcept { return u64x2(SIMD::vec_cast<SIMD::Vec2xU64>(SIMD::cmp_ne(a.v, b.v))); }
BL_INLINE_NODEBUG u64x2 operator> (const f64x2& a, const f64x2& b) noexcept { return u64x2(SIMD::vec_cast<SIMD::Vec2xU64>(SIMD::cmp_gt(a.v, b.v))); }
BL_INLINE_NODEBUG u64x2 operator>=(const f64x2& a, const f64x2& b) noexcept { return u64x2(SIMD::vec_cast<SIMD::Vec2xU64>(SIMD::cmp_ge(a.v, b.v))); }
BL_INLINE_NODEBUG u64x2 operator< (const f64x2& a, const f64x2& b) noexcept { return u64x2(SIMD::vec_cast<SIMD::Vec2xU64>(SIMD::cmp_lt(a.v, b.v))); }
BL_INLINE_NODEBUG u64x2 operator<=(const f64x2& a, const f64x2& b) noexcept { return u64x2(SIMD::vec_cast<SIMD::Vec2xU64>(SIMD::cmp_le(a.v, b.v))); }

BL_INLINE_NODEBUG f64x2 not_(const f64x2& a) noexcept { return f64x2(SIMD::not_(a.v)); }
BL_INLINE_NODEBUG f64x2 abs(const f64x2& a) noexcept { return f64x2(SIMD::abs(a.v)); }
BL_INLINE_NODEBUG f64x2 swap(const f64x2& a) noexcept { return f64x2(SIMD::swap_f64(a.v)); }
BL_INLINE_NODEBUG f64x2 msb_mask(const f64x2& a) noexcept { return f64x2(SIMD::srai_i64<63>(a.v)); }

BL_INLINE_NODEBUG f64x2 min(const f64x2& a, const f64x2& b) noexcept { return f64x2(SIMD::min(a.v, b.v)); }
BL_INLINE_NODEBUG f64x2 max(const f64x2& a, const f64x2& b) noexcept { return f64x2(SIMD::max(a.v, b.v)); }

#endif // BL_SIMD_WIDTH_I >= 128 && BL_SIMD_WIDTH_D >= 128

// Vec2 Operations
// ===============

// Vector operations that need specializations when a SIMD specialization is provided.
template<typename T> BL_INLINE_NODEBUG Vec2<T> operator+(const Vec2<T>& a, const Vec2<T>& b) noexcept { return Vec2<T>(a.x + b.x, a.y + b.y); }
template<typename T> BL_INLINE_NODEBUG Vec2<T> operator-(const Vec2<T>& a, const Vec2<T>& b) noexcept { return Vec2<T>(a.x - b.x, a.y - b.y); }
template<typename T> BL_INLINE_NODEBUG Vec2<T> operator*(const Vec2<T>& a, const Vec2<T>& b) noexcept { return Vec2<T>(a.x * b.x, a.y * b.y); }
template<typename T> BL_INLINE_NODEBUG Vec2<T> operator/(const Vec2<T>& a, const Vec2<T>& b) noexcept { return Vec2<T>(a.x / b.x, a.y / b.y); }

template<typename T> BL_INLINE_NODEBUG Vec2<T> operator&(const Vec2<T>& a, const Vec2<T>& b) noexcept { return Vec2<T>(and_(a.x, b.x), and_(a.y, b.y)); }
template<typename T> BL_INLINE_NODEBUG Vec2<T> operator|(const Vec2<T>& a, const Vec2<T>& b) noexcept { return Vec2<T>(or_(a.x, b.x), or_(a.y, b.y)); }
template<typename T> BL_INLINE_NODEBUG Vec2<T> operator^(const Vec2<T>& a, const Vec2<T>& b) noexcept { return Vec2<T>(xor_(a.x, b.x), xor_(a.y, b.y)); }

template<typename T> BL_INLINE_NODEBUG Vec2<UIntByType<T>> operator==(const Vec2<T>& a, const Vec2<T>& b) noexcept { return Vec2<UIntByType<T>>(cmp_eq(a.x, b.x), cmp_eq(a.y, b.y)); }
template<typename T> BL_INLINE_NODEBUG Vec2<UIntByType<T>> operator!=(const Vec2<T>& a, const Vec2<T>& b) noexcept { return Vec2<UIntByType<T>>(cmp_ne(a.x, b.x), cmp_ne(a.y, b.y)); }
template<typename T> BL_INLINE_NODEBUG Vec2<UIntByType<T>> operator> (const Vec2<T>& a, const Vec2<T>& b) noexcept { return Vec2<UIntByType<T>>(cmp_gt(a.x, b.x), cmp_gt(a.y, b.y)); }
template<typename T> BL_INLINE_NODEBUG Vec2<UIntByType<T>> operator>=(const Vec2<T>& a, const Vec2<T>& b) noexcept { return Vec2<UIntByType<T>>(cmp_ge(a.x, b.x), cmp_ge(a.y, b.y)); }
template<typename T> BL_INLINE_NODEBUG Vec2<UIntByType<T>> operator< (const Vec2<T>& a, const Vec2<T>& b) noexcept { return Vec2<UIntByType<T>>(cmp_lt(a.x, b.x), cmp_lt(a.y, b.y)); }
template<typename T> BL_INLINE_NODEBUG Vec2<UIntByType<T>> operator<=(const Vec2<T>& a, const Vec2<T>& b) noexcept { return Vec2<UIntByType<T>>(cmp_le(a.x, b.x), cmp_le(a.y, b.y)); }

// Vector operations that don't need specializations when a SIMD specialization is provided.
template<typename T> BL_INLINE_NODEBUG Vec2<T> operator+(const Vec2<T>& a, const T& b) noexcept { return a + Vec2<T>(b, b); }
template<typename T> BL_INLINE_NODEBUG Vec2<T> operator-(const Vec2<T>& a, const T& b) noexcept { return a - Vec2<T>(b, b); }
template<typename T> BL_INLINE_NODEBUG Vec2<T> operator*(const Vec2<T>& a, const T& b) noexcept { return a * Vec2<T>(b, b); }
template<typename T> BL_INLINE_NODEBUG Vec2<T> operator/(const Vec2<T>& a, const T& b) noexcept { return a / Vec2<T>(b, b); }
template<typename T> BL_INLINE_NODEBUG Vec2<T> operator+(const T& a, const Vec2<T>& b) noexcept { return Vec2<T>(a, a) + b; }
template<typename T> BL_INLINE_NODEBUG Vec2<T> operator-(const T& a, const Vec2<T>& b) noexcept { return Vec2<T>(a, a) - b; }
template<typename T> BL_INLINE_NODEBUG Vec2<T> operator*(const T& a, const Vec2<T>& b) noexcept { return Vec2<T>(a, a) * b; }
template<typename T> BL_INLINE_NODEBUG Vec2<T> operator/(const T& a, const Vec2<T>& b) noexcept { return Vec2<T>(a, a) / b; }

template<typename T> BL_INLINE_NODEBUG Vec2<T> operator&(const Vec2<T>& a, const T& b) noexcept { return a & Vec2<T>(b, b); }
template<typename T> BL_INLINE_NODEBUG Vec2<T> operator|(const Vec2<T>& a, const T& b) noexcept { return a | Vec2<T>(b, b); }
template<typename T> BL_INLINE_NODEBUG Vec2<T> operator^(const Vec2<T>& a, const T& b) noexcept { return a ^ Vec2<T>(b, b); }
template<typename T> BL_INLINE_NODEBUG Vec2<T> operator&(const T& a, const Vec2<T>& b) noexcept { return Vec2<T>(a, a) & b; }
template<typename T> BL_INLINE_NODEBUG Vec2<T> operator|(const T& a, const Vec2<T>& b) noexcept { return Vec2<T>(a, a) | b; }
template<typename T> BL_INLINE_NODEBUG Vec2<T> operator^(const T& a, const Vec2<T>& b) noexcept { return Vec2<T>(a, a) ^ b; }

template<typename T> BL_INLINE_NODEBUG Vec2<UIntByType<T>> operator==(const Vec2<T>& a, const T& b) noexcept { return a == Vec2<T>(b, b); }
template<typename T> BL_INLINE_NODEBUG Vec2<UIntByType<T>> operator!=(const Vec2<T>& a, const T& b) noexcept { return a != Vec2<T>(b, b); }
template<typename T> BL_INLINE_NODEBUG Vec2<UIntByType<T>> operator> (const Vec2<T>& a, const T& b) noexcept { return a >  Vec2<T>(b, b); }
template<typename T> BL_INLINE_NODEBUG Vec2<UIntByType<T>> operator>=(const Vec2<T>& a, const T& b) noexcept { return a >= Vec2<T>(b, b); }
template<typename T> BL_INLINE_NODEBUG Vec2<UIntByType<T>> operator< (const Vec2<T>& a, const T& b) noexcept { return a <  Vec2<T>(b, b); }
template<typename T> BL_INLINE_NODEBUG Vec2<UIntByType<T>> operator<=(const Vec2<T>& a, const T& b) noexcept { return a <= Vec2<T>(b, b); }
template<typename T> BL_INLINE_NODEBUG Vec2<UIntByType<T>> operator==(const T& a, const Vec2<T>& b) noexcept { return Vec2<T>(a, a) == b; }
template<typename T> BL_INLINE_NODEBUG Vec2<UIntByType<T>> operator!=(const T& a, const Vec2<T>& b) noexcept { return Vec2<T>(a, a) != b; }
template<typename T> BL_INLINE_NODEBUG Vec2<UIntByType<T>> operator> (const T& a, const Vec2<T>& b) noexcept { return Vec2<T>(a, a) >  b; }
template<typename T> BL_INLINE_NODEBUG Vec2<UIntByType<T>> operator>=(const T& a, const Vec2<T>& b) noexcept { return Vec2<T>(a, a) >= b; }
template<typename T> BL_INLINE_NODEBUG Vec2<UIntByType<T>> operator< (const T& a, const Vec2<T>& b) noexcept { return Vec2<T>(a, a) <  b; }
template<typename T> BL_INLINE_NODEBUG Vec2<UIntByType<T>> operator<=(const T& a, const Vec2<T>& b) noexcept { return Vec2<T>(a, a) <= b; }

template<typename T> BL_INLINE_NODEBUG Vec2<T>& operator+=(Vec2<T>& a, Vec2<T> b) noexcept { a = a + b; return a; }
template<typename T> BL_INLINE_NODEBUG Vec2<T>& operator-=(Vec2<T>& a, Vec2<T> b) noexcept { a = a - b; return a; }
template<typename T> BL_INLINE_NODEBUG Vec2<T>& operator*=(Vec2<T>& a, Vec2<T> b) noexcept { a = a * b; return a; }
template<typename T> BL_INLINE_NODEBUG Vec2<T>& operator/=(Vec2<T>& a, Vec2<T> b) noexcept { a = a / b; return a; }
template<typename T> BL_INLINE_NODEBUG Vec2<T>& operator+=(Vec2<T>& a, const T& b) noexcept { a = a + b; return a; }
template<typename T> BL_INLINE_NODEBUG Vec2<T>& operator-=(Vec2<T>& a, const T& b) noexcept { a = a - b; return a; }
template<typename T> BL_INLINE_NODEBUG Vec2<T>& operator*=(Vec2<T>& a, const T& b) noexcept { a = a * b; return a; }
template<typename T> BL_INLINE_NODEBUG Vec2<T>& operator/=(Vec2<T>& a, const T& b) noexcept { a = a / b; return a; }

template<typename T1, typename T2> BL_INLINE_NODEBUG Vec2<T1>& operator&=(Vec2<T1>& a, const Vec2<T2>& b) noexcept { a = a & b; return a; }
template<typename T1, typename T2> BL_INLINE_NODEBUG Vec2<T1>& operator|=(Vec2<T1>& a, const Vec2<T2>& b) noexcept { a = a | b; return a; }
template<typename T1, typename T2> BL_INLINE_NODEBUG Vec2<T1>& operator^=(Vec2<T1>& a, const Vec2<T2>& b) noexcept { a = a ^ b; return a; }
template<typename T1, typename T2> BL_INLINE_NODEBUG Vec2<T1>& operator&=(Vec2<T1>& a, const T2& b) noexcept { a = a & b; return a; }
template<typename T1, typename T2> BL_INLINE_NODEBUG Vec2<T1>& operator|=(Vec2<T1>& a, const T2& b) noexcept { a = a | b; return a; }
template<typename T1, typename T2> BL_INLINE_NODEBUG Vec2<T1>& operator^=(Vec2<T1>& a, const T2& b) noexcept { a = a ^ b; return a; }

template<typename T> BL_INLINE_NODEBUG Vec2<T> not_(const Vec2<T>& a) noexcept { return Vec2<T>(not_(a.x), not_(a.y)); }
template<typename T> BL_INLINE_NODEBUG Vec2<T> msb_mask(const Vec2<T>& a) noexcept { return Vec2<T>(msb_mask(a.x), msb_mask(a.y)); }
template<typename T> BL_INLINE_NODEBUG Vec2<T> abs(const Vec2<T>& a) noexcept { return Vec2<T>(abs(a.x), abs(a.y)); }
template<typename T> BL_INLINE_NODEBUG Vec2<T> min(const Vec2<T>& a, const Vec2<T>& b) noexcept { return Vec2<T>(min(a.x, b.x), min(a.y, b.y)); }
template<typename T> BL_INLINE_NODEBUG Vec2<T> max(const Vec2<T>& a, const Vec2<T>& b) noexcept { return Vec2<T>(max(a.x, b.x), max(a.y, b.y)); }

template<typename T> BL_INLINE_NODEBUG T hadd(const Vec2<T>& a) noexcept { return a.x + a.y; }
template<typename T> BL_INLINE_NODEBUG T hmul(const Vec2<T>& a) noexcept { return a.x * a.y; }
template<typename T> BL_INLINE_NODEBUG Vec2<T> swap(const Vec2<T>& a) noexcept { return Vec2<T>(a.y, a.x); }

} // {anonymous}
} // {Vec}

//! \}

} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_SUPPORT_VECOPS_P_H_INCLUDED
