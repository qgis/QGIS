// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_SIMD_SIMDARM_P_H_INCLUDED
#define BLEND2D_SIMD_SIMDARM_P_H_INCLUDED

#include <blend2d/simd/simdbase_p.h>
#include <blend2d/support/memops_p.h>
#include <blend2d/support/bitops_p.h>
#include <blend2d/tables/tables_p.h>

#include <arm_neon.h>

#if defined(__clang__)
  #define BL_SIMD_BUILTIN_SHUFFLEVECTOR
#elif defined(__GNUC__) && (__GNUC__ >= 12)
  #define BL_SIMD_BUILTIN_SHUFFLEVECTOR
#endif

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

// The reason for doing so is to make the public interface using SIMD::Internal easier to read.
#define I Internal

// SIMD - Register Widths
// ======================

#if BL_TARGET_ARCH_ARM >= 64
  #define BL_SIMD_AARCH64
#endif // BL_TARGET_ARCH_ARM >= 64

#define BL_SIMD_WIDTH_I 128
#define BL_SIMD_WIDTH_F 128

#if defined(BL_SIMD_AARCH64)
  #define BL_SIMD_WIDTH_D 128
#else
  #define BL_SIMD_WIDTH_D 0
#endif

// SIMD - Features
// ===============

#if defined(BL_SIMD_AARCH64)
  #define BL_SIMD_FEATURE_ARRAY_LOOKUP
#else
  #define BL_SIMD_IMPRECISE_FP_DIV
  #define BL_SIMD_IMPRECISE_FP_SQRT
#endif

#define BL_SIMD_FEATURE_BLEND_IMM
#define BL_SIMD_FEATURE_MOVW
#define BL_SIMD_FEATURE_SWIZZLEV_U8
#define BL_SIMD_FEATURE_RSRL

// SIMD - Cost Tables
// ==================

#define BL_SIMD_COST_ABS_I8          1  // native
#define BL_SIMD_COST_ABS_I16         1  // native
#define BL_SIMD_COST_ABS_I32         1  // native
#define BL_SIMD_COST_ALIGNR_U8       1  // native
#define BL_SIMD_COST_MIN_MAX_I8      1  // native
#define BL_SIMD_COST_MIN_MAX_U8      1  // native
#define BL_SIMD_COST_MIN_MAX_I16     1  // native
#define BL_SIMD_COST_MIN_MAX_U16     1  // native
#define BL_SIMD_COST_MIN_MAX_I32     1  // native
#define BL_SIMD_COST_MIN_MAX_U32     1  // native
#define BL_SIMD_COST_MUL_I16         1  // native
#define BL_SIMD_COST_MUL_I32         1  // native
#define BL_SIMD_COST_MUL_I64         7  // emulated (complex)

#if defined(BL_SIMD_AARCH64)
  #define BL_SIMD_COST_ABS_I64       1  // native
  #define BL_SIMD_COST_CMP_EQ_I64    1  // native
  #define BL_SIMD_COST_CMP_LT_GT_I64 1  // native
  #define BL_SIMD_COST_CMP_LE_GE_I64 1  // native
  #define BL_SIMD_COST_CMP_LT_GT_U64 1  // native
  #define BL_SIMD_COST_CMP_LE_GE_U64 1  // native
  #define BL_SIMD_COST_MIN_MAX_I64   2  // emulated ('cmp_gt_i64' + 'blend')
  #define BL_SIMD_COST_MIN_MAX_U64   2  // emulated ('cmp_gt_u64' + 'blend')
#else
  #define BL_SIMD_COST_ABS_I64       3  // emulated
  #define BL_SIMD_COST_CMP_EQ_I64    3  // emulated
  #define BL_SIMD_COST_CMP_LT_GT_I64 2  // emulated
  #define BL_SIMD_COST_CMP_LE_GE_I64 3  // emulated
  #define BL_SIMD_COST_CMP_LT_GT_U64 3  // emulated
  #define BL_SIMD_COST_CMP_LE_GE_U64 3  // emulated
  #define BL_SIMD_COST_MIN_MAX_I64   3  // emulated
  #define BL_SIMD_COST_MIN_MAX_U64   2  // emulated
#endif

namespace SIMD {

// SIMD - Vector Registers
// =======================

//! \cond NEVER

template<> struct Vec<8, int8_t>;
template<> struct Vec<8, uint8_t>;
template<> struct Vec<8, int16_t>;
template<> struct Vec<8, uint16_t>;
template<> struct Vec<8, int32_t>;
template<> struct Vec<8, uint32_t>;
template<> struct Vec<8, int64_t>;
template<> struct Vec<8, uint64_t>;
template<> struct Vec<8, float>;
template<> struct Vec<8, double>;

template<> struct Vec<16, int8_t>;
template<> struct Vec<16, uint8_t>;
template<> struct Vec<16, int16_t>;
template<> struct Vec<16, uint16_t>;
template<> struct Vec<16, int32_t>;
template<> struct Vec<16, uint32_t>;
template<> struct Vec<16, int64_t>;
template<> struct Vec<16, uint64_t>;
template<> struct Vec<16, float>;
template<> struct Vec<16, double>;

#define BL_DECLARE_SIMD_TYPE(type_name, width, simd_type, element_type)            \
template<>                                                                         \
struct Vec<width, element_type> {                                                  \
  static inline constexpr size_t kW = width;                                       \
  static inline constexpr uint32_t kHalfVectorWidth = width > 8 ? width / 2u : 8;  \
                                                                                   \
  static inline constexpr uint32_t kElementWidth = uint32_t(sizeof(element_type)); \
  static inline constexpr uint32_t kElementCount = width / kElementWidth;          \
                                                                                   \
  using VectorType = Vec<width, element_type>;                                     \
  using VectorHalfType = Vec<kHalfVectorWidth, element_type>;                      \
  using Vector64Type = Vec<8, element_type>;                                       \
  using Vector128Type = Vec<16, element_type>;                                     \
                                                                                   \
  using SimdType = simd_type;                                                      \
  using HalfSimdType = typename VectorHalfType::SimdType;                          \
                                                                                   \
  using ElementType = element_type;                                                \
                                                                                   \
  SimdType v;                                                                      \
};                                                                                 \
                                                                                   \
typedef Vec<width, element_type> type_name

BL_DECLARE_SIMD_TYPE(Vec8xI8, 8, int8x8_t , int8_t);
BL_DECLARE_SIMD_TYPE(Vec8xU8, 8, uint8x8_t, uint8_t);
BL_DECLARE_SIMD_TYPE(Vec4xI16, 8, int16x4_t , int16_t);
BL_DECLARE_SIMD_TYPE(Vec4xU16, 8, uint16x4_t, uint16_t);
BL_DECLARE_SIMD_TYPE(Vec2xI32, 8, int32x2_t , int32_t);
BL_DECLARE_SIMD_TYPE(Vec2xU32, 8, uint32x2_t, uint32_t);
BL_DECLARE_SIMD_TYPE(Vec1xI64, 8, int64x1_t , int64_t);
BL_DECLARE_SIMD_TYPE(Vec1xU64, 8, uint64x1_t, uint64_t);
BL_DECLARE_SIMD_TYPE(Vec2xF32, 8, float32x2_t, float);

BL_DECLARE_SIMD_TYPE(Vec16xI8, 16, int8x16_t , int8_t);
BL_DECLARE_SIMD_TYPE(Vec16xU8, 16, uint8x16_t, uint8_t);
BL_DECLARE_SIMD_TYPE(Vec8xI16, 16, int16x8_t , int16_t);
BL_DECLARE_SIMD_TYPE(Vec8xU16, 16, uint16x8_t, uint16_t);
BL_DECLARE_SIMD_TYPE(Vec4xI32, 16, int32x4_t , int32_t);
BL_DECLARE_SIMD_TYPE(Vec4xU32, 16, uint32x4_t, uint32_t);
BL_DECLARE_SIMD_TYPE(Vec2xI64, 16, int64x2_t , int64_t);
BL_DECLARE_SIMD_TYPE(Vec2xU64, 16, uint64x2_t, uint64_t);
BL_DECLARE_SIMD_TYPE(Vec4xF32, 16, float32x4_t, float);

#if defined(BL_SIMD_AARCH64)
BL_DECLARE_SIMD_TYPE(Vec1xF64, 8, float64x1_t, double);
BL_DECLARE_SIMD_TYPE(Vec2xF64, 16, float64x2_t, double);
#endif // BL_SIMD_AARCH64

#undef BL_DECLARE_SIMD_TYPE

//! \endcond

// Everything must be in anonymous namespace to avoid ODR violation.
namespace {

// SIMD - Internal - Simd Info
// ===========================

namespace Internal {

template<size_t kW>
struct SimdInfo;

//! \cond NEVER

template<>
struct SimdInfo<8> {
  typedef int8x8_t RegI8;
  typedef uint8x8_t RegU8;
  typedef int16x4_t RegI16;
  typedef uint16x4_t RegU16;
  typedef int32x2_t RegI32;
  typedef uint32x2_t RegU32;
  typedef int64x1_t RegI64;
  typedef uint64x1_t RegU64;
  typedef float32x2_t RegF32;
#if defined(BL_SIMD_AARCH64)
  typedef float64x1_t RegF64;
#endif // BL_SIMD_AARCH64
};

template<>
struct SimdInfo<16> {
  typedef int8x16_t RegI8;
  typedef uint8x16_t RegU8;
  typedef int16x8_t RegI16;
  typedef uint16x8_t RegU16;
  typedef int32x4_t RegI32;
  typedef uint32x4_t RegU32;
  typedef int64x2_t RegI64;
  typedef uint64x2_t RegU64;
  typedef float32x4_t RegF32;
#if defined(BL_SIMD_AARCH64)
  typedef float64x2_t RegF64;
#endif // BL_SIMD_AARCH64
};

//! \endcond

} // {Internal}

// SIMD - Internal - Simd Cast
// ===========================

// Low-level cast operation that casts a SIMD register type (used by high-level wrappers).
template<typename DstSimdT, typename SrcSimdT>
BL_INLINE_NODEBUG DstSimdT simd_cast(const SrcSimdT& src) noexcept = delete;

// 8-byte SIMD register casts.
template<> BL_INLINE_NODEBUG int8x8_t simd_cast(const int8x8_t& src) noexcept { return src; }
template<> BL_INLINE_NODEBUG int8x8_t simd_cast(const uint8x8_t& src) noexcept { return vreinterpret_s8_u8(src); }
template<> BL_INLINE_NODEBUG int8x8_t simd_cast(const int16x4_t& src) noexcept { return vreinterpret_s8_s16(src); }
template<> BL_INLINE_NODEBUG int8x8_t simd_cast(const uint16x4_t& src) noexcept { return vreinterpret_s8_u16(src); }
template<> BL_INLINE_NODEBUG int8x8_t simd_cast(const int32x2_t& src) noexcept { return vreinterpret_s8_s32(src); }
template<> BL_INLINE_NODEBUG int8x8_t simd_cast(const uint32x2_t& src) noexcept { return vreinterpret_s8_u32(src); }
template<> BL_INLINE_NODEBUG int8x8_t simd_cast(const int64x1_t& src) noexcept { return vreinterpret_s8_s64(src); }
template<> BL_INLINE_NODEBUG int8x8_t simd_cast(const uint64x1_t& src) noexcept { return vreinterpret_s8_u64(src); }
template<> BL_INLINE_NODEBUG int8x8_t simd_cast(const float32x2_t& src) noexcept { return vreinterpret_s8_f32(src); }

template<> BL_INLINE_NODEBUG uint8x8_t simd_cast(const int8x8_t& src) noexcept { return vreinterpret_u8_s8(src); }
template<> BL_INLINE_NODEBUG uint8x8_t simd_cast(const uint8x8_t& src) noexcept { return src; }
template<> BL_INLINE_NODEBUG uint8x8_t simd_cast(const int16x4_t& src) noexcept { return vreinterpret_u8_s16(src); }
template<> BL_INLINE_NODEBUG uint8x8_t simd_cast(const uint16x4_t& src) noexcept { return vreinterpret_u8_u16(src); }
template<> BL_INLINE_NODEBUG uint8x8_t simd_cast(const int32x2_t& src) noexcept { return vreinterpret_u8_s32(src); }
template<> BL_INLINE_NODEBUG uint8x8_t simd_cast(const uint32x2_t& src) noexcept { return vreinterpret_u8_u32(src); }
template<> BL_INLINE_NODEBUG uint8x8_t simd_cast(const int64x1_t& src) noexcept { return vreinterpret_u8_s64(src); }
template<> BL_INLINE_NODEBUG uint8x8_t simd_cast(const uint64x1_t& src) noexcept { return vreinterpret_u8_u64(src); }
template<> BL_INLINE_NODEBUG uint8x8_t simd_cast(const float32x2_t& src) noexcept { return vreinterpret_u8_f32(src); }

template<> BL_INLINE_NODEBUG int16x4_t simd_cast(const int8x8_t& src) noexcept { return vreinterpret_s16_s8(src); }
template<> BL_INLINE_NODEBUG int16x4_t simd_cast(const uint8x8_t& src) noexcept { return vreinterpret_s16_u8(src); }
template<> BL_INLINE_NODEBUG int16x4_t simd_cast(const int16x4_t& src) noexcept { return src; }
template<> BL_INLINE_NODEBUG int16x4_t simd_cast(const uint16x4_t& src) noexcept { return vreinterpret_s16_u16(src); }
template<> BL_INLINE_NODEBUG int16x4_t simd_cast(const int32x2_t& src) noexcept { return vreinterpret_s16_s32(src); }
template<> BL_INLINE_NODEBUG int16x4_t simd_cast(const uint32x2_t& src) noexcept { return vreinterpret_s16_u32(src); }
template<> BL_INLINE_NODEBUG int16x4_t simd_cast(const int64x1_t& src) noexcept { return vreinterpret_s16_s64(src); }
template<> BL_INLINE_NODEBUG int16x4_t simd_cast(const uint64x1_t& src) noexcept { return vreinterpret_s16_u64(src); }
template<> BL_INLINE_NODEBUG int16x4_t simd_cast(const float32x2_t& src) noexcept { return vreinterpret_s16_f32(src); }

template<> BL_INLINE_NODEBUG uint16x4_t simd_cast(const int8x8_t& src) noexcept { return vreinterpret_u16_s8(src); }
template<> BL_INLINE_NODEBUG uint16x4_t simd_cast(const uint8x8_t& src) noexcept { return vreinterpret_u16_u8(src); }
template<> BL_INLINE_NODEBUG uint16x4_t simd_cast(const int16x4_t& src) noexcept { return vreinterpret_u16_s16(src); }
template<> BL_INLINE_NODEBUG uint16x4_t simd_cast(const uint16x4_t& src) noexcept { return src; }
template<> BL_INLINE_NODEBUG uint16x4_t simd_cast(const int32x2_t& src) noexcept { return vreinterpret_u16_s32(src); }
template<> BL_INLINE_NODEBUG uint16x4_t simd_cast(const uint32x2_t& src) noexcept { return vreinterpret_u16_u32(src); }
template<> BL_INLINE_NODEBUG uint16x4_t simd_cast(const int64x1_t& src) noexcept { return vreinterpret_u16_s64(src); }
template<> BL_INLINE_NODEBUG uint16x4_t simd_cast(const uint64x1_t& src) noexcept { return vreinterpret_u16_u64(src); }
template<> BL_INLINE_NODEBUG uint16x4_t simd_cast(const float32x2_t& src) noexcept { return vreinterpret_u16_f32(src); }

template<> BL_INLINE_NODEBUG int32x2_t simd_cast(const int8x8_t& src) noexcept { return vreinterpret_s32_s8(src); }
template<> BL_INLINE_NODEBUG int32x2_t simd_cast(const uint8x8_t& src) noexcept { return vreinterpret_s32_u8(src); }
template<> BL_INLINE_NODEBUG int32x2_t simd_cast(const int16x4_t& src) noexcept { return vreinterpret_s32_s16(src); }
template<> BL_INLINE_NODEBUG int32x2_t simd_cast(const uint16x4_t& src) noexcept { return vreinterpret_s32_u16(src); }
template<> BL_INLINE_NODEBUG int32x2_t simd_cast(const int32x2_t& src) noexcept { return src; }
template<> BL_INLINE_NODEBUG int32x2_t simd_cast(const uint32x2_t& src) noexcept { return vreinterpret_s32_u32(src); }
template<> BL_INLINE_NODEBUG int32x2_t simd_cast(const int64x1_t& src) noexcept { return vreinterpret_s32_s64(src); }
template<> BL_INLINE_NODEBUG int32x2_t simd_cast(const uint64x1_t& src) noexcept { return vreinterpret_s32_u64(src); }
template<> BL_INLINE_NODEBUG int32x2_t simd_cast(const float32x2_t& src) noexcept { return vreinterpret_s32_f32(src); }

template<> BL_INLINE_NODEBUG uint32x2_t simd_cast(const int8x8_t& src) noexcept { return vreinterpret_u32_s8(src); }
template<> BL_INLINE_NODEBUG uint32x2_t simd_cast(const uint8x8_t& src) noexcept { return vreinterpret_u32_u8(src); }
template<> BL_INLINE_NODEBUG uint32x2_t simd_cast(const int16x4_t& src) noexcept { return vreinterpret_u32_s16(src); }
template<> BL_INLINE_NODEBUG uint32x2_t simd_cast(const uint16x4_t& src) noexcept { return vreinterpret_u32_u16(src); }
template<> BL_INLINE_NODEBUG uint32x2_t simd_cast(const int32x2_t& src) noexcept { return vreinterpret_u32_s32(src); }
template<> BL_INLINE_NODEBUG uint32x2_t simd_cast(const uint32x2_t& src) noexcept { return src; }
template<> BL_INLINE_NODEBUG uint32x2_t simd_cast(const int64x1_t& src) noexcept { return vreinterpret_u32_s64(src); }
template<> BL_INLINE_NODEBUG uint32x2_t simd_cast(const uint64x1_t& src) noexcept { return vreinterpret_u32_u64(src); }
template<> BL_INLINE_NODEBUG uint32x2_t simd_cast(const float32x2_t& src) noexcept { return vreinterpret_u32_f32(src); }

template<> BL_INLINE_NODEBUG int64x1_t simd_cast(const int8x8_t& src) noexcept { return vreinterpret_s64_s8(src); }
template<> BL_INLINE_NODEBUG int64x1_t simd_cast(const uint8x8_t& src) noexcept { return vreinterpret_s64_u8(src); }
template<> BL_INLINE_NODEBUG int64x1_t simd_cast(const int16x4_t& src) noexcept { return vreinterpret_s64_s16(src); }
template<> BL_INLINE_NODEBUG int64x1_t simd_cast(const uint16x4_t& src) noexcept { return vreinterpret_s64_u16(src); }
template<> BL_INLINE_NODEBUG int64x1_t simd_cast(const int32x2_t& src) noexcept { return vreinterpret_s64_s32(src); }
template<> BL_INLINE_NODEBUG int64x1_t simd_cast(const uint32x2_t& src) noexcept { return vreinterpret_s64_u32(src); }
template<> BL_INLINE_NODEBUG int64x1_t simd_cast(const int64x1_t& src) noexcept { return src; }
template<> BL_INLINE_NODEBUG int64x1_t simd_cast(const uint64x1_t& src) noexcept { return vreinterpret_s64_u64(src); }
template<> BL_INLINE_NODEBUG int64x1_t simd_cast(const float32x2_t& src) noexcept { return vreinterpret_s64_f32(src); }

template<> BL_INLINE_NODEBUG uint64x1_t simd_cast(const int8x8_t& src) noexcept { return vreinterpret_u64_s8(src); }
template<> BL_INLINE_NODEBUG uint64x1_t simd_cast(const uint8x8_t& src) noexcept { return vreinterpret_u64_u8(src); }
template<> BL_INLINE_NODEBUG uint64x1_t simd_cast(const int16x4_t& src) noexcept { return vreinterpret_u64_s16(src); }
template<> BL_INLINE_NODEBUG uint64x1_t simd_cast(const uint16x4_t& src) noexcept { return vreinterpret_u64_u16(src); }
template<> BL_INLINE_NODEBUG uint64x1_t simd_cast(const int32x2_t& src) noexcept { return vreinterpret_u64_s32(src); }
template<> BL_INLINE_NODEBUG uint64x1_t simd_cast(const uint32x2_t& src) noexcept { return vreinterpret_u64_u32(src); }
template<> BL_INLINE_NODEBUG uint64x1_t simd_cast(const int64x1_t& src) noexcept { return vreinterpret_u64_s64(src); }
template<> BL_INLINE_NODEBUG uint64x1_t simd_cast(const uint64x1_t& src) noexcept { return src; }
template<> BL_INLINE_NODEBUG uint64x1_t simd_cast(const float32x2_t& src) noexcept { return vreinterpret_u64_f32(src); }

template<> BL_INLINE_NODEBUG float32x2_t simd_cast(const int8x8_t& src) noexcept { return vreinterpret_f32_s8(src); }
template<> BL_INLINE_NODEBUG float32x2_t simd_cast(const uint8x8_t& src) noexcept { return vreinterpret_f32_u8(src); }
template<> BL_INLINE_NODEBUG float32x2_t simd_cast(const int16x4_t& src) noexcept { return vreinterpret_f32_s16(src); }
template<> BL_INLINE_NODEBUG float32x2_t simd_cast(const uint16x4_t& src) noexcept { return vreinterpret_f32_u16(src); }
template<> BL_INLINE_NODEBUG float32x2_t simd_cast(const int32x2_t& src) noexcept { return vreinterpret_f32_s32(src); }
template<> BL_INLINE_NODEBUG float32x2_t simd_cast(const uint32x2_t& src) noexcept { return vreinterpret_f32_u32(src); }
template<> BL_INLINE_NODEBUG float32x2_t simd_cast(const int64x1_t& src) noexcept { return vreinterpret_f32_s64(src); }
template<> BL_INLINE_NODEBUG float32x2_t simd_cast(const uint64x1_t& src) noexcept { return vreinterpret_f32_u64(src); }
template<> BL_INLINE_NODEBUG float32x2_t simd_cast(const float32x2_t& src) noexcept { return src; }

#if defined(BL_SIMD_AARCH64)
template<> BL_INLINE_NODEBUG int8x8_t simd_cast(const float64x1_t& src) noexcept { return vreinterpret_s8_f64(src); }
template<> BL_INLINE_NODEBUG uint8x8_t simd_cast(const float64x1_t& src) noexcept { return vreinterpret_u8_f64(src); }
template<> BL_INLINE_NODEBUG int16x4_t simd_cast(const float64x1_t& src) noexcept { return vreinterpret_s16_f64(src); }
template<> BL_INLINE_NODEBUG uint16x4_t simd_cast(const float64x1_t& src) noexcept { return vreinterpret_u16_f64(src); }
template<> BL_INLINE_NODEBUG int32x2_t simd_cast(const float64x1_t& src) noexcept { return vreinterpret_s32_f64(src); }
template<> BL_INLINE_NODEBUG uint32x2_t simd_cast(const float64x1_t& src) noexcept { return vreinterpret_u32_f64(src); }
template<> BL_INLINE_NODEBUG int64x1_t simd_cast(const float64x1_t& src) noexcept { return vreinterpret_s64_f64(src); }
template<> BL_INLINE_NODEBUG uint64x1_t simd_cast(const float64x1_t& src) noexcept { return vreinterpret_u64_f64(src); }
template<> BL_INLINE_NODEBUG float32x2_t simd_cast(const float64x1_t& src) noexcept { return vreinterpret_f32_f64(src); }

template<> BL_INLINE_NODEBUG float64x1_t simd_cast(const int8x8_t& src) noexcept { return vreinterpret_f64_s8(src); }
template<> BL_INLINE_NODEBUG float64x1_t simd_cast(const uint8x8_t& src) noexcept { return vreinterpret_f64_u8(src); }
template<> BL_INLINE_NODEBUG float64x1_t simd_cast(const int16x4_t& src) noexcept { return vreinterpret_f64_s16(src); }
template<> BL_INLINE_NODEBUG float64x1_t simd_cast(const uint16x4_t& src) noexcept { return vreinterpret_f64_u16(src); }
template<> BL_INLINE_NODEBUG float64x1_t simd_cast(const int32x2_t& src) noexcept { return vreinterpret_f64_s32(src); }
template<> BL_INLINE_NODEBUG float64x1_t simd_cast(const uint32x2_t& src) noexcept { return vreinterpret_f64_u32(src); }
template<> BL_INLINE_NODEBUG float64x1_t simd_cast(const int64x1_t& src) noexcept { return vreinterpret_f64_s64(src); }
template<> BL_INLINE_NODEBUG float64x1_t simd_cast(const uint64x1_t& src) noexcept { return vreinterpret_f64_u64(src); }
template<> BL_INLINE_NODEBUG float64x1_t simd_cast(const float32x2_t& src) noexcept { return vreinterpret_f64_f32(src); }
template<> BL_INLINE_NODEBUG float64x1_t simd_cast(const float64x1_t& src) noexcept { return src; }
#endif // BL_SIMD_AARCH64

// 16-byte SIMD register casts.
template<> BL_INLINE_NODEBUG int8x16_t simd_cast(const int8x16_t& src) noexcept { return src; }
template<> BL_INLINE_NODEBUG int8x16_t simd_cast(const uint8x16_t& src) noexcept { return vreinterpretq_s8_u8(src); }
template<> BL_INLINE_NODEBUG int8x16_t simd_cast(const int16x8_t& src) noexcept { return vreinterpretq_s8_s16(src); }
template<> BL_INLINE_NODEBUG int8x16_t simd_cast(const uint16x8_t& src) noexcept { return vreinterpretq_s8_u16(src); }
template<> BL_INLINE_NODEBUG int8x16_t simd_cast(const int32x4_t& src) noexcept { return vreinterpretq_s8_s32(src); }
template<> BL_INLINE_NODEBUG int8x16_t simd_cast(const uint32x4_t& src) noexcept { return vreinterpretq_s8_u32(src); }
template<> BL_INLINE_NODEBUG int8x16_t simd_cast(const int64x2_t& src) noexcept { return vreinterpretq_s8_s64(src); }
template<> BL_INLINE_NODEBUG int8x16_t simd_cast(const uint64x2_t& src) noexcept { return vreinterpretq_s8_u64(src); }
template<> BL_INLINE_NODEBUG int8x16_t simd_cast(const float32x4_t& src) noexcept { return vreinterpretq_s8_f32(src); }

template<> BL_INLINE_NODEBUG uint8x16_t simd_cast(const int8x16_t& src) noexcept { return vreinterpretq_u8_s8(src); }
template<> BL_INLINE_NODEBUG uint8x16_t simd_cast(const uint8x16_t& src) noexcept { return src; }
template<> BL_INLINE_NODEBUG uint8x16_t simd_cast(const int16x8_t& src) noexcept { return vreinterpretq_u8_s16(src); }
template<> BL_INLINE_NODEBUG uint8x16_t simd_cast(const uint16x8_t& src) noexcept { return vreinterpretq_u8_u16(src); }
template<> BL_INLINE_NODEBUG uint8x16_t simd_cast(const int32x4_t& src) noexcept { return vreinterpretq_u8_s32(src); }
template<> BL_INLINE_NODEBUG uint8x16_t simd_cast(const uint32x4_t& src) noexcept { return vreinterpretq_u8_u32(src); }
template<> BL_INLINE_NODEBUG uint8x16_t simd_cast(const int64x2_t& src) noexcept { return vreinterpretq_u8_s64(src); }
template<> BL_INLINE_NODEBUG uint8x16_t simd_cast(const uint64x2_t& src) noexcept { return vreinterpretq_u8_u64(src); }
template<> BL_INLINE_NODEBUG uint8x16_t simd_cast(const float32x4_t& src) noexcept { return vreinterpretq_u8_f32(src); }

template<> BL_INLINE_NODEBUG int16x8_t simd_cast(const int8x16_t& src) noexcept { return vreinterpretq_s16_s8(src); }
template<> BL_INLINE_NODEBUG int16x8_t simd_cast(const uint8x16_t& src) noexcept { return vreinterpretq_s16_u8(src); }
template<> BL_INLINE_NODEBUG int16x8_t simd_cast(const int16x8_t& src) noexcept { return src; }
template<> BL_INLINE_NODEBUG int16x8_t simd_cast(const uint16x8_t& src) noexcept { return vreinterpretq_s16_u16(src); }
template<> BL_INLINE_NODEBUG int16x8_t simd_cast(const int32x4_t& src) noexcept { return vreinterpretq_s16_s32(src); }
template<> BL_INLINE_NODEBUG int16x8_t simd_cast(const uint32x4_t& src) noexcept { return vreinterpretq_s16_u32(src); }
template<> BL_INLINE_NODEBUG int16x8_t simd_cast(const int64x2_t& src) noexcept { return vreinterpretq_s16_s64(src); }
template<> BL_INLINE_NODEBUG int16x8_t simd_cast(const uint64x2_t& src) noexcept { return vreinterpretq_s16_u64(src); }
template<> BL_INLINE_NODEBUG int16x8_t simd_cast(const float32x4_t& src) noexcept { return vreinterpretq_s16_f32(src); }

template<> BL_INLINE_NODEBUG uint16x8_t simd_cast(const int8x16_t& src) noexcept { return vreinterpretq_u16_s8(src); }
template<> BL_INLINE_NODEBUG uint16x8_t simd_cast(const uint8x16_t& src) noexcept { return vreinterpretq_u16_u8(src); }
template<> BL_INLINE_NODEBUG uint16x8_t simd_cast(const int16x8_t& src) noexcept { return vreinterpretq_u16_s16(src); }
template<> BL_INLINE_NODEBUG uint16x8_t simd_cast(const uint16x8_t& src) noexcept { return src; }
template<> BL_INLINE_NODEBUG uint16x8_t simd_cast(const int32x4_t& src) noexcept { return vreinterpretq_u16_s32(src); }
template<> BL_INLINE_NODEBUG uint16x8_t simd_cast(const uint32x4_t& src) noexcept { return vreinterpretq_u16_u32(src); }
template<> BL_INLINE_NODEBUG uint16x8_t simd_cast(const int64x2_t& src) noexcept { return vreinterpretq_u16_s64(src); }
template<> BL_INLINE_NODEBUG uint16x8_t simd_cast(const uint64x2_t& src) noexcept { return vreinterpretq_u16_u64(src); }
template<> BL_INLINE_NODEBUG uint16x8_t simd_cast(const float32x4_t& src) noexcept { return vreinterpretq_u16_f32(src); }

template<> BL_INLINE_NODEBUG int32x4_t simd_cast(const int8x16_t& src) noexcept { return vreinterpretq_s32_s8(src); }
template<> BL_INLINE_NODEBUG int32x4_t simd_cast(const uint8x16_t& src) noexcept { return vreinterpretq_s32_u8(src); }
template<> BL_INLINE_NODEBUG int32x4_t simd_cast(const int16x8_t& src) noexcept { return vreinterpretq_s32_s16(src); }
template<> BL_INLINE_NODEBUG int32x4_t simd_cast(const uint16x8_t& src) noexcept { return vreinterpretq_s32_u16(src); }
template<> BL_INLINE_NODEBUG int32x4_t simd_cast(const int32x4_t& src) noexcept { return src; }
template<> BL_INLINE_NODEBUG int32x4_t simd_cast(const uint32x4_t& src) noexcept { return vreinterpretq_s32_u32(src); }
template<> BL_INLINE_NODEBUG int32x4_t simd_cast(const int64x2_t& src) noexcept { return vreinterpretq_s32_s64(src); }
template<> BL_INLINE_NODEBUG int32x4_t simd_cast(const uint64x2_t& src) noexcept { return vreinterpretq_s32_u64(src); }
template<> BL_INLINE_NODEBUG int32x4_t simd_cast(const float32x4_t& src) noexcept { return vreinterpretq_s32_f32(src); }

template<> BL_INLINE_NODEBUG uint32x4_t simd_cast(const int8x16_t& src) noexcept { return vreinterpretq_u32_s8(src); }
template<> BL_INLINE_NODEBUG uint32x4_t simd_cast(const uint8x16_t& src) noexcept { return vreinterpretq_u32_u8(src); }
template<> BL_INLINE_NODEBUG uint32x4_t simd_cast(const int16x8_t& src) noexcept { return vreinterpretq_u32_s16(src); }
template<> BL_INLINE_NODEBUG uint32x4_t simd_cast(const uint16x8_t& src) noexcept { return vreinterpretq_u32_u16(src); }
template<> BL_INLINE_NODEBUG uint32x4_t simd_cast(const int32x4_t& src) noexcept { return vreinterpretq_u32_s32(src); }
template<> BL_INLINE_NODEBUG uint32x4_t simd_cast(const uint32x4_t& src) noexcept { return src; }
template<> BL_INLINE_NODEBUG uint32x4_t simd_cast(const int64x2_t& src) noexcept { return vreinterpretq_u32_s64(src); }
template<> BL_INLINE_NODEBUG uint32x4_t simd_cast(const uint64x2_t& src) noexcept { return vreinterpretq_u32_u64(src); }
template<> BL_INLINE_NODEBUG uint32x4_t simd_cast(const float32x4_t& src) noexcept { return vreinterpretq_u32_f32(src); }

template<> BL_INLINE_NODEBUG int64x2_t simd_cast(const int8x16_t& src) noexcept { return vreinterpretq_s64_s8(src); }
template<> BL_INLINE_NODEBUG int64x2_t simd_cast(const uint8x16_t& src) noexcept { return vreinterpretq_s64_u8(src); }
template<> BL_INLINE_NODEBUG int64x2_t simd_cast(const int16x8_t& src) noexcept { return vreinterpretq_s64_s16(src); }
template<> BL_INLINE_NODEBUG int64x2_t simd_cast(const uint16x8_t& src) noexcept { return vreinterpretq_s64_u16(src); }
template<> BL_INLINE_NODEBUG int64x2_t simd_cast(const int32x4_t& src) noexcept { return vreinterpretq_s64_s32(src); }
template<> BL_INLINE_NODEBUG int64x2_t simd_cast(const uint32x4_t& src) noexcept { return vreinterpretq_s64_u32(src); }
template<> BL_INLINE_NODEBUG int64x2_t simd_cast(const int64x2_t& src) noexcept { return src; }
template<> BL_INLINE_NODEBUG int64x2_t simd_cast(const uint64x2_t& src) noexcept { return vreinterpretq_s64_u64(src); }
template<> BL_INLINE_NODEBUG int64x2_t simd_cast(const float32x4_t& src) noexcept { return vreinterpretq_s64_f32(src); }

template<> BL_INLINE_NODEBUG uint64x2_t simd_cast(const int8x16_t& src) noexcept { return vreinterpretq_u64_s8(src); }
template<> BL_INLINE_NODEBUG uint64x2_t simd_cast(const uint8x16_t& src) noexcept { return vreinterpretq_u64_u8(src); }
template<> BL_INLINE_NODEBUG uint64x2_t simd_cast(const int16x8_t& src) noexcept { return vreinterpretq_u64_s16(src); }
template<> BL_INLINE_NODEBUG uint64x2_t simd_cast(const uint16x8_t& src) noexcept { return vreinterpretq_u64_u16(src); }
template<> BL_INLINE_NODEBUG uint64x2_t simd_cast(const int32x4_t& src) noexcept { return vreinterpretq_u64_s32(src); }
template<> BL_INLINE_NODEBUG uint64x2_t simd_cast(const uint32x4_t& src) noexcept { return vreinterpretq_u64_u32(src); }
template<> BL_INLINE_NODEBUG uint64x2_t simd_cast(const int64x2_t& src) noexcept { return vreinterpretq_u64_s64(src); }
template<> BL_INLINE_NODEBUG uint64x2_t simd_cast(const uint64x2_t& src) noexcept { return src; }
template<> BL_INLINE_NODEBUG uint64x2_t simd_cast(const float32x4_t& src) noexcept { return vreinterpretq_u64_f32(src); }

template<> BL_INLINE_NODEBUG float32x4_t simd_cast(const int8x16_t& src) noexcept { return vreinterpretq_f32_s8(src); }
template<> BL_INLINE_NODEBUG float32x4_t simd_cast(const uint8x16_t& src) noexcept { return vreinterpretq_f32_u8(src); }
template<> BL_INLINE_NODEBUG float32x4_t simd_cast(const int16x8_t& src) noexcept { return vreinterpretq_f32_s16(src); }
template<> BL_INLINE_NODEBUG float32x4_t simd_cast(const uint16x8_t& src) noexcept { return vreinterpretq_f32_u16(src); }
template<> BL_INLINE_NODEBUG float32x4_t simd_cast(const int32x4_t& src) noexcept { return vreinterpretq_f32_s32(src); }
template<> BL_INLINE_NODEBUG float32x4_t simd_cast(const uint32x4_t& src) noexcept { return vreinterpretq_f32_u32(src); }
template<> BL_INLINE_NODEBUG float32x4_t simd_cast(const int64x2_t& src) noexcept { return vreinterpretq_f32_s64(src); }
template<> BL_INLINE_NODEBUG float32x4_t simd_cast(const uint64x2_t& src) noexcept { return vreinterpretq_f32_u64(src); }
template<> BL_INLINE_NODEBUG float32x4_t simd_cast(const float32x4_t& src) noexcept { return src; }

#if defined(BL_SIMD_AARCH64)
template<> BL_INLINE_NODEBUG int8x16_t simd_cast(const float64x2_t& src) noexcept { return vreinterpretq_s8_f64(src); }
template<> BL_INLINE_NODEBUG uint8x16_t simd_cast(const float64x2_t& src) noexcept { return vreinterpretq_u8_f64(src); }
template<> BL_INLINE_NODEBUG int16x8_t simd_cast(const float64x2_t& src) noexcept { return vreinterpretq_s16_f64(src); }
template<> BL_INLINE_NODEBUG uint16x8_t simd_cast(const float64x2_t& src) noexcept { return vreinterpretq_u16_f64(src); }
template<> BL_INLINE_NODEBUG int32x4_t simd_cast(const float64x2_t& src) noexcept { return vreinterpretq_s32_f64(src); }
template<> BL_INLINE_NODEBUG uint32x4_t simd_cast(const float64x2_t& src) noexcept { return vreinterpretq_u32_f64(src); }
template<> BL_INLINE_NODEBUG int64x2_t simd_cast(const float64x2_t& src) noexcept { return vreinterpretq_s64_f64(src); }
template<> BL_INLINE_NODEBUG uint64x2_t simd_cast(const float64x2_t& src) noexcept { return vreinterpretq_u64_f64(src); }
template<> BL_INLINE_NODEBUG float32x4_t simd_cast(const float64x2_t& src) noexcept { return vreinterpretq_f32_f64(src); }

template<> BL_INLINE_NODEBUG float64x2_t simd_cast(const int8x16_t& src) noexcept { return vreinterpretq_f64_s8(src); }
template<> BL_INLINE_NODEBUG float64x2_t simd_cast(const uint8x16_t& src) noexcept { return vreinterpretq_f64_u8(src); }
template<> BL_INLINE_NODEBUG float64x2_t simd_cast(const int16x8_t& src) noexcept { return vreinterpretq_f64_s16(src); }
template<> BL_INLINE_NODEBUG float64x2_t simd_cast(const uint16x8_t& src) noexcept { return vreinterpretq_f64_u16(src); }
template<> BL_INLINE_NODEBUG float64x2_t simd_cast(const int32x4_t& src) noexcept { return vreinterpretq_f64_s32(src); }
template<> BL_INLINE_NODEBUG float64x2_t simd_cast(const uint32x4_t& src) noexcept { return vreinterpretq_f64_u32(src); }
template<> BL_INLINE_NODEBUG float64x2_t simd_cast(const int64x2_t& src) noexcept { return vreinterpretq_f64_s64(src); }
template<> BL_INLINE_NODEBUG float64x2_t simd_cast(const uint64x2_t& src) noexcept { return vreinterpretq_f64_u64(src); }
template<> BL_INLINE_NODEBUG float64x2_t simd_cast(const float32x4_t& src) noexcept { return vreinterpretq_f64_f32(src); }
template<> BL_INLINE_NODEBUG float64x2_t simd_cast(const float64x2_t& src) noexcept { return src; }
#endif // BL_SIMD_AARCH64

template<typename T> BL_INLINE_NODEBUG typename I::SimdInfo<sizeof(T)>::RegI8 simd_i8(const T& src) noexcept { return simd_cast<typename I::SimdInfo<sizeof(T)>::RegI8>(src); }
template<typename T> BL_INLINE_NODEBUG typename I::SimdInfo<sizeof(T)>::RegU8 simd_u8(const T& src) noexcept { return simd_cast<typename I::SimdInfo<sizeof(T)>::RegU8>(src); }
template<typename T> BL_INLINE_NODEBUG typename I::SimdInfo<sizeof(T)>::RegI16 simd_i16(const T& src) noexcept { return simd_cast<typename I::SimdInfo<sizeof(T)>::RegI16>(src); }
template<typename T> BL_INLINE_NODEBUG typename I::SimdInfo<sizeof(T)>::RegU16 simd_u16(const T& src) noexcept { return simd_cast<typename I::SimdInfo<sizeof(T)>::RegU16>(src); }
template<typename T> BL_INLINE_NODEBUG typename I::SimdInfo<sizeof(T)>::RegI32 simd_i32(const T& src) noexcept { return simd_cast<typename I::SimdInfo<sizeof(T)>::RegI32>(src); }
template<typename T> BL_INLINE_NODEBUG typename I::SimdInfo<sizeof(T)>::RegU32 simd_u32(const T& src) noexcept { return simd_cast<typename I::SimdInfo<sizeof(T)>::RegU32>(src); }
template<typename T> BL_INLINE_NODEBUG typename I::SimdInfo<sizeof(T)>::RegI64 simd_i64(const T& src) noexcept { return simd_cast<typename I::SimdInfo<sizeof(T)>::RegI64>(src); }
template<typename T> BL_INLINE_NODEBUG typename I::SimdInfo<sizeof(T)>::RegU64 simd_u64(const T& src) noexcept { return simd_cast<typename I::SimdInfo<sizeof(T)>::RegU64>(src); }
template<typename T> BL_INLINE_NODEBUG typename I::SimdInfo<sizeof(T)>::RegF32 simd_f32(const T& src) noexcept { return simd_cast<typename I::SimdInfo<sizeof(T)>::RegF32>(src); }
#if defined(BL_SIMD_AARCH64)
template<typename T> BL_INLINE_NODEBUG typename I::SimdInfo<sizeof(T)>::RegF64 simd_f64(const T& src) noexcept { return simd_cast<typename I::SimdInfo<sizeof(T)>::RegF64>(src); }
#endif // BL_SIMD_AARCH64

// SIMD - Public - Vector Cast
// ===========================

template<typename DstVectorT, typename SrcVectorT>
BL_INLINE_NODEBUG DstVectorT vec_cast(const SrcVectorT& x) noexcept {
  return DstVectorT{simd_cast<typename DstVectorT::SimdType>(x.v)};
}

template<typename DstElementT, typename SrcVectorT>
BL_INLINE_NODEBUG DstElementT vec_of(const SrcVectorT& x) noexcept {
  return Vec<SrcVectorT::kW, DstElementT>{simd_cast<Vec<SrcVectorT::kW, DstElementT>>(x.v)};
}

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, int8_t> vec_i8(const Vec<W, T>& src) noexcept { return vec_cast<Vec<W, int8_t>>(src); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, uint8_t> vec_u8(const Vec<W, T>& src) noexcept { return vec_cast<Vec<W, uint8_t>>(src); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, int16_t> vec_i16(const Vec<W, T>& src) noexcept { return vec_cast<Vec<W, int16_t>>(src); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, uint16_t> vec_u16(const Vec<W, T>& src) noexcept { return vec_cast<Vec<W, uint16_t>>(src); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, int32_t> vec_i32(const Vec<W, T>& src) noexcept { return vec_cast<Vec<W, int32_t>>(src); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, uint32_t> vec_u32(const Vec<W, T>& src) noexcept { return vec_cast<Vec<W, uint32_t>>(src); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, int64_t> vec_i64(const Vec<W, T>& src) noexcept { return vec_cast<Vec<W, int64_t>>(src); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, uint64_t> vec_u64(const Vec<W, T>& src) noexcept { return vec_cast<Vec<W, uint64_t>>(src); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, float> vec_f32(const Vec<W, T>& src) noexcept { return vec_cast<Vec<W, float>>(src); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, double> vec_f64(const Vec<W, T>& src) noexcept { return vec_cast<Vec<W, double>>(src); }

template<typename DstVectorT, typename SrcT>
BL_INLINE_NODEBUG const DstVectorT& vec_const(const SrcT* src) noexcept { return *static_cast<const DstVectorT*>(static_cast<void*>(src)); }

// Converts a native SimdT register type to a wrapped V.
template<typename V, typename SimdT>
BL_INLINE_NODEBUG V from_simd(const SimdT& reg) noexcept { return V{simd_cast<typename V::SimdType>(reg)}; }

template<size_t W, typename T, typename SimdT>
BL_INLINE_NODEBUG Vec<W, T> vec_wt(const SimdT& reg) noexcept { return Vec<W, T>{simd_cast<typename Vec<W, T>::SimdType>(reg)}; }

// Converts a wrapped V to a native SimdT register type.
template<typename SimdT, typename V>
BL_INLINE_NODEBUG SimdT to_simd(const V& vec) noexcept { return simd_cast<SimdT>(vec.v); }

// SIMD - Internal - Make Zero & Ones & Undefined
// ==============================================

namespace Internal {

template<size_t kW> BL_INLINE_NODEBUG typename SimdInfo<kW>::RegU8 simd_make_zero_w() noexcept;
template<size_t kW> BL_INLINE_NODEBUG typename SimdInfo<kW>::RegU8 simd_make_ones_w() noexcept;
template<size_t kW> BL_INLINE_NODEBUG typename SimdInfo<kW>::RegU8 simd_make_undefined_w() noexcept;

template<> BL_INLINE_NODEBUG uint8x8_t simd_make_zero_w<8>() noexcept { return simd_u8(vdup_n_u32(0)); }
template<> BL_INLINE_NODEBUG uint8x16_t simd_make_zero_w<16>() noexcept { return simd_u8(vdupq_n_u32(0)); }

template<> BL_INLINE_NODEBUG uint8x8_t simd_make_ones_w<8>() noexcept { return simd_u8(vdup_n_s32(-1)); }
template<> BL_INLINE_NODEBUG uint8x16_t simd_make_ones_w<16>() noexcept { return simd_u8(vdupq_n_s32(-1)); }

// TODO: It seems that ARM has no such feature. So return something instead of returning uninitialized value.
template<> BL_INLINE_NODEBUG uint8x8_t simd_make_undefined_w<8>() noexcept { return simd_u8(vdup_n_u32(0)); }
template<> BL_INLINE_NODEBUG uint8x16_t simd_make_undefined_w<16>() noexcept { return simd_u8(vdupq_n_u32(0)); }

template<typename SimdT> BL_INLINE_NODEBUG SimdT simd_make_zero() noexcept { return simd_cast<SimdT>(simd_make_zero_w<sizeof(SimdT)>()); }
template<typename SimdT> BL_INLINE_NODEBUG SimdT simd_make_ones() noexcept { return simd_cast<SimdT>(simd_make_ones_w<sizeof(SimdT)>()); }
template<typename SimdT> BL_INLINE_NODEBUG SimdT simd_make_undefined() noexcept { return simd_cast<SimdT>(simd_make_undefined_w<sizeof(SimdT)>()); }

} // {Internal}

// SIMD - Internal - Make Vector
// =============================

namespace Internal {

BL_INLINE_NODEBUG uint64x2_t simd_make128_u64(uint64_t x0) noexcept {
  return vdupq_n_u64(x0);
}

BL_INLINE_NODEBUG uint64x2_t simd_make128_u64(uint64_t x1, uint64_t x0) noexcept {
  uint64x1_t v1 = vcreate_u64(x1);
  uint64x1_t v0 = vcreate_u64(x0);
  return vcombine_u64(v0, v1);
}

BL_INLINE_NODEBUG uint32x4_t simd_make128_u32(uint32_t x0) noexcept {
  return vdupq_n_u32(x0);
}

BL_INLINE_NODEBUG uint32x4_t simd_make128_u32(uint32_t x1, uint32_t x0) noexcept {
  return simd_u32(vdupq_n_u64(scalar_u64_from_2x_u32(x1, x0)));
}

BL_INLINE_NODEBUG uint32x4_t simd_make128_u32(uint32_t x3, uint32_t x2, uint32_t x1, uint32_t x0) noexcept {
  uint64x1_t v1 = vcreate_u64(scalar_u64_from_2x_u32(x3, x2));
  uint64x1_t v0 = vcreate_u64(scalar_u64_from_2x_u32(x1, x0));
  return simd_u32(vcombine_u64(v0, v1));
}

BL_INLINE_NODEBUG uint16x8_t simd_make128_u16(uint16_t x0) noexcept {
  return vdupq_n_u16(x0);
}

BL_INLINE_NODEBUG uint16x8_t simd_make128_u16(uint16_t x1, uint16_t x0) noexcept {
  return simd_u16(vdupq_n_u32(scalar_u32_from_2x_u16(x1, x0)));
}

BL_INLINE_NODEBUG uint16x8_t simd_make128_u16(uint16_t x3, uint16_t x2, uint16_t x1, uint16_t x0) noexcept {
  return simd_u16(vdupq_n_u64(scalar_u64_from_4x_u16(x3, x2, x1, x0)));
}

BL_INLINE_NODEBUG uint16x8_t simd_make128_u16(uint16_t x7, uint16_t x6, uint16_t x5, uint16_t x4,
                                              uint16_t x3, uint16_t x2, uint16_t x1, uint16_t x0) noexcept {
  uint64x1_t v1 = vcreate_u64(scalar_u64_from_4x_u16(x7, x6, x5, x4));
  uint64x1_t v0 = vcreate_u64(scalar_u64_from_4x_u16(x3, x2, x1, x0));
  return simd_u16(vcombine_u64(v0, v1));
}

BL_INLINE_NODEBUG uint8x16_t simd_make128_u8(uint8_t x0) noexcept {
  return vdupq_n_u8(x0);
}

BL_INLINE_NODEBUG uint8x16_t simd_make128_u8(uint8_t x1, uint8_t x0) noexcept {
  return simd_u8(vdupq_n_u16(scalar_u16_from_2x_u8(x1, x0)));
}

BL_INLINE_NODEBUG uint8x16_t simd_make128_u8(uint8_t x3, uint8_t x2, uint8_t x1, uint8_t x0) noexcept {
  return simd_u8(vdupq_n_u32(scalar_u32_from_4x_u8(x3, x2, x1, x0)));
}

BL_INLINE_NODEBUG uint8x16_t simd_make128_u8(uint8_t x7, uint8_t x6, uint8_t x5, uint8_t x4,
                                             uint8_t x3, uint8_t x2, uint8_t x1, uint8_t x0) noexcept {
  return simd_u8(vdupq_n_u64(scalar_u64_from_8x_u8(x7, x6, x5, x4, x3, x2, x1, x0)));
}

BL_INLINE_NODEBUG uint8x16_t simd_make128_u8(uint8_t x15, uint8_t x14, uint8_t x13, uint8_t x12,
                                             uint8_t x11, uint8_t x10, uint8_t x09, uint8_t x08,
                                             uint8_t x07, uint8_t x06, uint8_t x05, uint8_t x04,
                                             uint8_t x03, uint8_t x02, uint8_t x01, uint8_t x00) noexcept {
  uint64_t hi = scalar_u64_from_8x_u8(x15, x14, x13, x12, x11, x10, x09, x08);
  uint64_t lo = scalar_u64_from_8x_u8(x07, x06, x05, x04, x03, x02, x01, x00);
  return simd_u8(vcombine_u64(vcreate_u64(lo), vcreate_u64(hi)));
}

BL_INLINE_NODEBUG float32x4_t simd_make128_f32(float x0) noexcept {
  return vdupq_n_f32(x0);
}

BL_INLINE_NODEBUG float32x4_t simd_make128_f32(float x1, float x0) noexcept {
  alignas(16) float arr[4] = { x0, x1, x0, x1 };
  return vld1q_f32(arr);
}

BL_INLINE_NODEBUG float32x4_t simd_make128_f32(float x3, float x2, float x1, float x0) noexcept {
  alignas(16) float arr[4] = { x0, x1, x2, x3 };
  return vld1q_f32(arr);
}

#if defined(BL_SIMD_AARCH64)
BL_INLINE_NODEBUG float64x2_t simd_make128_f64(double x0) noexcept {
  return vdupq_n_f64(x0);
}

BL_INLINE_NODEBUG float64x2_t simd_make128_f64(double x1, double x0) noexcept {
  return vcombine_f64(vdup_n_f64(x0), vdup_n_f64(x1));
}
#endif // BL_SIMD_AARCH64

} // {Internal}

// SIMD - Internal - Cast Vector <-> Scalar
// ========================================

namespace Internal {

BL_INLINE_NODEBUG uint32x4_t simd_from_u32(uint32_t val) noexcept {
  return vsetq_lane_u32(val, vdupq_n_u32(0), 0);
}

BL_INLINE_NODEBUG int32_t simd_cast_to_i32(const int32x4_t& src) noexcept {
  return vgetq_lane_s32(src, 0);
}

BL_INLINE_NODEBUG uint32_t simd_cast_to_u32(const uint32x4_t& src) noexcept {
  return vgetq_lane_u32(src, 0);
}

BL_INLINE_NODEBUG uint64x2_t simd_from_u64(int64_t val) noexcept {
  return vsetq_lane_u64(val, vdupq_n_u64(0), 0);
}

BL_INLINE_NODEBUG uint64_t simd_cast_to_u64(const uint64x2_t& src) noexcept {
  return vgetq_lane_u64(src, 0);
}

BL_INLINE_NODEBUG float32x4_t simd_from_f32(float val) noexcept {
  alignas(16) float arr[4]{val, 0, 0, 0};
  return vld1q_f32(arr);
}

#if defined(BL_SIMD_AARCH64)
BL_INLINE_NODEBUG float64x2_t simd_from_f64(double val) noexcept {
  alignas(16) double arr[2]{val, 0};
  return vld1q_f64(arr);
}
#endif // BL_SIMD_AARCH64

BL_INLINE_NODEBUG float simd_cast_to_f32(const float32x4_t& src) noexcept {
  return vgetq_lane_f32(src, 0);
}

#if defined(BL_SIMD_AARCH64)
BL_INLINE_NODEBUG double simd_cast_to_f64(const float64x2_t& src) noexcept {
  return vgetq_lane_f64(src, 0);
}
#endif // BL_SIMD_AARCH64

} // {Internal}

// SIMD - Internal - Convert Vector <-> Vector
// ===========================================

namespace Internal {

BL_INLINE_NODEBUG float32x4_t simd_cvt_i32_f32(const int32x4_t& a) noexcept { return vcvtq_f32_s32(a); }

BL_INLINE_NODEBUG int32x4_t simd_cvt_f32_i32(const float32x4_t& a) noexcept {
#if defined(BL_SIMD_AARCH64) || defined(__ARM_FEATURE_DIRECTED_ROUNDING)
  return vcvtnq_s32_f32(a);
#elif defined(__ARM_FEATURE_FRINT)
  return vcvtq_s32_f32(vrnd32xq_f32(a));
#else
  // How to round without a rounding instruction:
  //   rounded = a >= kMaxN ? a : a + kMagic - kMagic;
  //
  // Can be rewritten as:
  //   pred = (a >= kMaxN) & kMagic;
  //   rounded = a + pred - pred;
  constexpr float kMaxN = 8388608;   // pow(2, 23)
  constexpr float kMagic = 12582912; // pow(2, 23) + pow(2, 22);

  float32x4_t vMaxN = simd_make128_f32(kMaxN);
  float32x4_t v_magic = simd_make128_f32(kMagic);

  uint32x4_t msk = vcgeq_f32(a, vMaxN);
  float32x4_t pred = simd_f32(vandq_u32(msk, simd_u32(v_magic)));

  float32x4_t rounded = vsubq_f32(vaddq_f32(a, pred), pred);
  return vcvtq_s32_f32(rounded);
#endif
}

BL_INLINE_NODEBUG int32x4_t simd_cvtt_f32_i32(const float32x4_t& a) noexcept { return vcvtq_s32_f32(a); }

} // {Internal}

// SIMD - Internal - Convert Vector <-> Scalar
// ===========================================

namespace Internal {

BL_INLINE_NODEBUG float32x4_t simd_cvt_f32_from_scalar_i32(int32_t val) noexcept { return simd_from_f32(float(val)); }
BL_INLINE_NODEBUG int32_t simd_cvt_f32_to_scalar_i32(const float32x4_t& src) noexcept { return  simd_cast_to_i32(simd_cvt_f32_i32(src)); }
BL_INLINE_NODEBUG int32_t simd_cvtt_f32_to_scalar_i32(const float32x4_t& src) noexcept { return  simd_cast_to_i32(simd_cvtt_f32_i32(src)); }

#if defined(BL_SIMD_AARCH64)
BL_INLINE_NODEBUG float64x2_t simd_cvt_f64_from_scalar_i32(int32_t val) noexcept { return simd_from_f64(double(val)); }
#endif // BL_SIMD_AARCH64

} // {Internal}

// SIMD - Internal - Shuffle & Permute
// ===================================

namespace Internal {

BL_INLINE_CONSTEXPR uint8_t simd_shuffle_predicate_4x2b(uint8_t d, uint8_t c, uint8_t b, uint8_t a) noexcept {
  return (uint8_t)((d << 6) | (c << 4) | (b << 2) | (a));
}

BL_INLINE_CONSTEXPR uint8_t simd_shuffle_predicate_2x1b(uint8_t b, uint8_t a) noexcept {
  return (uint8_t)((b << 1) | (a));
}

template<uint8_t kN>
BL_INLINE_NODEBUG uint8x16_t simd_dup_lane_u8(const uint8x16_t& a) noexcept {
#if defined(BL_SIMD_AARCH64)
  return vdupq_laneq_u8(a, kN);
#else
  return vdupq_n_u8(vgetq_lane_u8(a, kN));
#endif
}

template<uint8_t kN>
BL_INLINE_NODEBUG uint16x8_t simd_dup_lane_u16(const uint16x8_t& a) noexcept {
#if defined(BL_SIMD_AARCH64)
  return vdupq_laneq_u16(a, kN);
#else
  return vdupq_n_u16(vgetq_lane_u16(a, kN));
#endif
}

template<uint8_t kN>
BL_INLINE_NODEBUG uint32x4_t simd_dup_lane_u32(const uint32x4_t& a) noexcept {
#if defined(BL_SIMD_AARCH64)
  return vdupq_laneq_u32(a, kN);
#else
  return vdupq_n_u32(vgetq_lane_u32(a, kN));
#endif
}

template<uint8_t kN>
BL_INLINE_NODEBUG uint64x2_t simd_dup_lane_u64(const uint64x2_t& a) noexcept {
#if defined(BL_SIMD_AARCH64)
  return vdupq_laneq_u64(a, kN);
#else
  return vdupq_n_u64(vgetq_lane_u64(a, kN));
#endif
}

template<uint8_t kN>
BL_INLINE_NODEBUG float32x4_t simd_dup_lane_f32(const float32x4_t& a) noexcept {
#if defined(BL_SIMD_AARCH64)
  return vdupq_laneq_f32(a, kN);
#else
  return vdupq_n_f32(vgetq_lane_f32(a, kN));
#endif
}

#if defined(BL_SIMD_AARCH64)
template<uint8_t kN>
static float64x2_t simd_dup_lane_f64(const float64x2_t& a) noexcept { return vdupq_laneq_f64(a, kN); }
#endif // BL_SIMD_AARCH64

BL_INLINE_NODEBUG uint8x16_t simd_broadcast_u8(const uint8x16_t& a) noexcept { return simd_dup_lane_u8<0>(a); }
BL_INLINE_NODEBUG uint16x8_t simd_broadcast_u16(const uint16x8_t& a) noexcept { return simd_dup_lane_u16<0>(a); }
BL_INLINE_NODEBUG uint32x4_t simd_broadcast_u32(const uint32x4_t& a) noexcept { return simd_dup_lane_u32<0>(a); }
BL_INLINE_NODEBUG uint64x2_t simd_broadcast_u64(const uint64x2_t& a) noexcept { return simd_dup_lane_u64<0>(a); }
BL_INLINE_NODEBUG float32x4_t simd_broadcast_f32(const float32x4_t& a) noexcept { return simd_dup_lane_f32<0>(a); }
#if defined(BL_SIMD_AARCH64)
BL_INLINE_NODEBUG float64x2_t simd_broadcast_f64(const float64x2_t& a) noexcept { return simd_dup_lane_f64<0>(a); }
#endif // BL_SIMD_AARCH64

BL_INLINE_NODEBUG uint8x16_t simd_swizzlev_u8(const uint8x16_t& a, const uint8x16_t& b) noexcept {
  int8x16_t tbl = simd_i8(a);
  uint8x16_t idx = vandq_u8(b, vdupq_n_u8(0x8F));
#if defined(BL_SIMD_AARCH64)
  return simd_u8(vqtbl1q_s8(tbl, idx));
#elif defined(__GNUC__)
  int8x16_t ret;
  __asm__ __volatile__(
    "vtbl.8  %e[ret], {%e[tbl], %f[tbl]}, %e[idx]\n"
    "vtbl.8  %f[ret], {%e[tbl], %f[tbl]}, %f[idx]\n"
    : [ret] "=&w"(ret)
    : [tbl] "w"(tbl), [idx] "w"(idx));
  return simd_u8(ret);
#else
  int8x8x2_t p = {vget_low_s8(tbl), vget_high_s8(tbl)};
  return simd_u8(vcombine_s8(vtbl2_s8(p, vget_low_u8(idx)),
                             vtbl2_s8(p, vget_high_u8(idx))));
#endif
}

template<uint8_t D, uint8_t C, uint8_t B, uint8_t A>
BL_INLINE_NODEBUG uint16x8_t simd_swizzle_lo_u16(const uint16x8_t& a) noexcept {
#ifdef BL_SIMD_BUILTIN_SHUFFLEVECTOR
  return __builtin_shufflevector(a, a, A, B, C, D, 4, 5, 6, 7);
#else
  return uint16x8_t{a[A], a[B], a[C], a[D], a[4], a[5], a[6], a[7]};
#endif // BL_SIMD_BUILTIN_SHUFFLEVECTOR
}

template<uint8_t D, uint8_t C, uint8_t B, uint8_t A>
BL_INLINE_NODEBUG uint16x8_t simd_swizzle_hi_u16(const uint16x8_t& a) noexcept {
#ifdef BL_SIMD_BUILTIN_SHUFFLEVECTOR
  return __builtin_shufflevector(a, a, 0, 1, 2, 3, A + 4, B + 4, C + 4, D + 4);
#else
  return uint16x8_t{a[0], a[1], a[2], a[3], a[A + 4], a[B + 4], a[C + 4], a[D + 4]};
#endif // BL_SIMD_BUILTIN_SHUFFLEVECTOR
}

template<uint8_t D, uint8_t C, uint8_t B, uint8_t A>
BL_INLINE_NODEBUG uint16x8_t simd_swizzle_u16(const uint16x8_t& a) noexcept {
#ifdef BL_SIMD_BUILTIN_SHUFFLEVECTOR
  return __builtin_shufflevector(a, a, A, B, C, D, A + 4, B + 4, C + 4, D + 4);
#else
  return uint16x8_t{a[A], a[B], a[C], a[D], a[A + 4], a[B + 4], a[C + 4], a[D + 4]};
#endif // BL_SIMD_BUILTIN_SHUFFLEVECTOR
}

template<uint8_t D, uint8_t C, uint8_t B, uint8_t A>
BL_INLINE_NODEBUG uint32x4_t simd_swizzle_u32(const uint32x4_t& a) noexcept {
#ifdef BL_SIMD_BUILTIN_SHUFFLEVECTOR
  return __builtin_shufflevector(a, a, A, B, C, D);
#else
  constexpr uint8_t kPredicate = simd_shuffle_predicate_4x2b(D, C, B, A);
  constexpr uint8_t k0000 = simd_shuffle_predicate_4x2b(0, 0, 0, 0);
  constexpr uint8_t k0101 = simd_shuffle_predicate_4x2b(0, 1, 0, 1);
  constexpr uint8_t k1010 = simd_shuffle_predicate_4x2b(1, 0, 1, 0);
  constexpr uint8_t k1023 = simd_shuffle_predicate_4x2b(1, 0, 2, 3);
  constexpr uint8_t k1111 = simd_shuffle_predicate_4x2b(1, 1, 1, 1);
  constexpr uint8_t k2222 = simd_shuffle_predicate_4x2b(2, 2, 2, 2);
  constexpr uint8_t k2323 = simd_shuffle_predicate_4x2b(2, 3, 2, 3);
  constexpr uint8_t k3210 = simd_shuffle_predicate_4x2b(3, 2, 1, 0);
  constexpr uint8_t k3232 = simd_shuffle_predicate_4x2b(3, 2, 3, 2);
  constexpr uint8_t k3333 = simd_shuffle_predicate_4x2b(3, 3, 3, 3);

  if constexpr (kPredicate == k0000) {
    return simd_dup_lane_u32<0>(a);
  }
  else if constexpr (kPredicate == k0101) {
    uint32x2_t t = vrev64_u32(vget_low_u32(a));
    return vcombine_u32(t, t);
  }
  else if constexpr (kPredicate == k1010) {
    uint32x2_t t = vget_low_u32(a);
    return vcombine_u32(t, t);
  }
  else if constexpr (kPredicate == k1023) {
    return vrev64q_u32(a);
  }
  else if constexpr (kPredicate == k1111) {
    return simd_dup_lane_u32<1>(a);
  }
  else if constexpr (kPredicate == k2222) {
    return simd_dup_lane_u32<2>(a);
  }
  else if constexpr (kPredicate == k2323) {
    uint32x2_t t = vrev64_u32(vget_high_u32(a));
    return vcombine_u32(t, t);
  }
  else if constexpr (kPredicate == k3210) {
    return a;
  }
  else if constexpr (kPredicate == k3232) {
    uint32x2_t t = vget_high_u32(a);
    return vcombine_u32(t, t);
  }
  else if constexpr (kPredicate == k3333) {
    return simd_dup_lane_u32<3>(a);
  }
  else {
    return uint32x4_t{a[A], a[B], a[C], a[D]};
  }
#endif // BL_SIMD_BUILTIN_SHUFFLEVECTOR
}

template<uint8_t B, uint8_t A>
BL_INLINE_NODEBUG uint64x2_t simd_swizzle_u64(const uint64x2_t& a) noexcept {
#ifdef BL_SIMD_BUILTIN_SHUFFLEVECTOR
  return __builtin_shufflevector(a, a, A, B);
#else
  constexpr uint8_t kPredicate = simd_shuffle_predicate_2x1b(B, A);
  constexpr uint8_t k00 = simd_shuffle_predicate_2x1b(0, 0);
  constexpr uint8_t k01 = simd_shuffle_predicate_2x1b(0, 1);
  constexpr uint8_t k10 = simd_shuffle_predicate_2x1b(1, 0);
  // (not used in code)
  // constexpr uint8_t k11 = simd_shuffle_predicate_2x1b(1, 1);

  if constexpr (kPredicate == k00) {
    return simd_dup_lane_u64<0>(a);
  }
  else if constexpr (kPredicate == k01) {
    return vcombine_u64(vget_high_u64(a), vget_low_u64(a));
  }
  else if constexpr (kPredicate == k10) {
    return a;
  }
  else {
    return simd_dup_lane_u64<1>(a);
  }
#endif // BL_SIMD_BUILTIN_SHUFFLEVECTOR
}

template<uint8_t D, uint8_t C, uint8_t B, uint8_t A>
BL_INLINE_NODEBUG float32x4_t simd_swizzle_f32(const float32x4_t& a) noexcept {
  return simd_f32(simd_swizzle_u32<D, C, B, A>(simd_u32(a)));
}

#if defined(BL_SIMD_AARCH64)
template<uint8_t B, uint8_t A>
BL_INLINE_NODEBUG float64x2_t simd_swizzle_f64(const float64x2_t& a) noexcept {
  return simd_f64(simd_swizzle_u64<B, A>(simd_u64(a)));
}
#endif // BL_SIMD_AARCH64

template<uint8_t D, uint8_t C, uint8_t B, uint8_t A>
BL_INLINE_NODEBUG uint32x4_t simd_shuffle_u32(const uint32x4_t& lo, const uint32x4_t& hi) noexcept {
#ifdef BL_SIMD_BUILTIN_SHUFFLEVECTOR
  return __builtin_shufflevector(lo, hi, A, B, C + 4, D + 4);
#else
  return uint32x4_t{lo[A], lo[B], hi[C], hi[D]};
#endif // BL_SIMD_BUILTIN_SHUFFLEVECTOR
}

template<uint8_t B, uint8_t A>
BL_INLINE_NODEBUG uint64x2_t simd_shuffle_u64(const uint64x2_t& lo, const uint64x2_t& hi) noexcept {
#ifdef BL_SIMD_BUILTIN_SHUFFLEVECTOR
  return __builtin_shufflevector(lo, hi, A, B + 2);
#else
  return uint64x2_t{lo[A], hi[B]};
#endif // BL_SIMD_BUILTIN_SHUFFLEVECTOR
}

template<uint8_t D, uint8_t C, uint8_t B, uint8_t A>
BL_INLINE_NODEBUG float32x4_t simd_shuffle_f32(const float32x4_t& lo, const float32x4_t& hi) noexcept {
#ifdef BL_SIMD_BUILTIN_SHUFFLEVECTOR
  return __builtin_shufflevector(lo, hi, A, B, C + 4, D + 4);
#else
  return simd_f32(simd_shuffle_u32<D, C, B, A>(simd_u32(lo), simd_u32(hi)));
#endif // BL_SIMD_BUILTIN_SHUFFLEVECTOR
}

#if defined(BL_SIMD_AARCH64)
template<uint8_t B, uint8_t A>
BL_INLINE_NODEBUG float64x2_t simd_shuffle_f64(const float64x2_t& lo, const float64x2_t& hi) noexcept {
#ifdef BL_SIMD_BUILTIN_SHUFFLEVECTOR
  return __builtin_shufflevector(lo, hi, A, B + 2);
#else
  return simd_f64(simd_shuffle_u64<B, A>(simd_u64(lo), simd_u64(hi)));
#endif // BL_SIMD_BUILTIN_SHUFFLEVECTOR
}
#endif // BL_SIMD_AARCH64

BL_INLINE_NODEBUG uint32x4_t simd_dup_lo_u32(const uint32x4_t& a) noexcept { return simd_swizzle_u32<2, 2, 0, 0>(a); }
BL_INLINE_NODEBUG uint32x4_t simd_dup_hi_u32(const uint32x4_t& a) noexcept { return simd_swizzle_u32<3, 3, 1, 1>(a); }

BL_INLINE_NODEBUG uint64x2_t simd_dup_lo_u64(const uint64x2_t& a) noexcept { return simd_dup_lane_u64<0>(a); }
BL_INLINE_NODEBUG uint64x2_t simd_dup_hi_u64(const uint64x2_t& a) noexcept { return simd_dup_lane_u64<1>(a); }

BL_INLINE_NODEBUG float32x4_t simd_dup_lo_f32(const float32x4_t& a) noexcept { return simd_swizzle_f32<2, 2, 0, 0>(a); }
BL_INLINE_NODEBUG float32x4_t simd_dup_hi_f32(const float32x4_t& a) noexcept { return simd_swizzle_f32<3, 3, 1, 1>(a); }

BL_INLINE_NODEBUG float32x4_t simd_dup_lo_f32x2(const float32x4_t& a) noexcept { return simd_swizzle_f32<1, 0, 1, 0>(a); }
BL_INLINE_NODEBUG float32x4_t simd_dup_hi_f32x2(const float32x4_t& a) noexcept { return simd_swizzle_f32<3, 2, 3, 2>(a); }

BL_INLINE_NODEBUG uint32x4_t simd_swap_u32(const uint32x4_t& a) noexcept { return simd_swizzle_u32<2, 3, 0, 1>(a); }
BL_INLINE_NODEBUG uint64x2_t simd_swap_u64(const uint64x2_t& a) noexcept { return simd_swizzle_u64<0, 1>(a); }
BL_INLINE_NODEBUG float32x4_t simd_swap_f32(const float32x4_t& a) noexcept { return simd_swizzle_f32<2, 3, 0, 1>(a); }

#if defined(BL_SIMD_AARCH64)
BL_INLINE_NODEBUG float64x2_t simd_dup_lo_f64(const float64x2_t& a) noexcept { return simd_swizzle_f64<0, 0>(a); }
BL_INLINE_NODEBUG float64x2_t simd_dup_hi_f64(const float64x2_t& a) noexcept { return simd_swizzle_f64<1, 1>(a); }
BL_INLINE_NODEBUG float64x2_t simd_swap_f64(const float64x2_t& a) noexcept { return simd_swizzle_f64<0, 1>(a); }
#endif // BL_SIMD_AARCH64

BL_INLINE_NODEBUG uint8x16_t simd_interleave_lo_u8(const uint8x16_t& a, const uint8x16_t& b) noexcept {
#if defined(BL_SIMD_AARCH64)
  return vzip1q_u8(a, b);
#else
  uint8x8_t a_low = vget_low_u8(a);
  uint8x8_t b_low = vget_low_u8(b);
  uint8x8x2_t ab = vzip_u8(a_low, b_low);
  return vcombine_u8(ab.val[0], ab.val[1]);
#endif
}

BL_INLINE_NODEBUG uint8x16_t simd_interleave_hi_u8(const uint8x16_t& a, const uint8x16_t& b) noexcept {
#if defined(BL_SIMD_AARCH64)
  return vzip2q_u8(a, b);
#else
  uint8x8_t a_high = vget_high_u8(a);
  uint8x8_t b_high = vget_high_u8(b);
  uint8x8x2_t ab = vzip_u8(a_high, b_high);
  return vcombine_u8(ab.val[0], ab.val[1]);
#endif
}

BL_INLINE_NODEBUG uint16x8_t simd_interleave_lo_u16(const uint16x8_t& a, const uint16x8_t& b) noexcept {
#if defined(BL_SIMD_AARCH64)
  return vzip1q_u16(a, b);
#else
  uint16x4_t a_low = vget_low_u16(a);
  uint16x4_t b_low = vget_low_u16(b);
  uint16x4x2_t ab = vzip_u16(a_low, b_low);
  return vcombine_u16(ab.val[0], ab.val[1]);
#endif
}

BL_INLINE_NODEBUG uint16x8_t simd_interleave_hi_u16(const uint16x8_t& a, const uint16x8_t& b) noexcept {
#if defined(BL_SIMD_AARCH64)
  return vzip2q_u16(a, b);
#else
  uint16x4_t a_high = vget_high_u16(a);
  uint16x4_t b_high = vget_high_u16(b);
  uint16x4x2_t ab = vzip_u16(a_high, b_high);
  return vcombine_u16(ab.val[0], ab.val[1]);
#endif
}

BL_INLINE_NODEBUG uint32x4_t simd_interleave_lo_u32(const uint32x4_t& a, const uint32x4_t& b) noexcept {
#if defined(BL_SIMD_AARCH64)
  return vzip1q_u32(a, b);
#else
  uint32x2_t a_low = vget_low_u32(a);
  uint32x2_t b_low = vget_low_u32(b);
  uint32x2x2_t ab = vzip_u32(a_low, b_low);
  return vcombine_u32(ab.val[0], ab.val[1]);
#endif
}

BL_INLINE_NODEBUG uint32x4_t simd_interleave_hi_u32(const uint32x4_t& a, const uint32x4_t& b) noexcept {
#if defined(BL_SIMD_AARCH64)
  return vzip2q_u32(a, b);
#else
  uint32x2_t a_high = vget_high_u32(a);
  uint32x2_t b_high = vget_high_u32(b);
  uint32x2x2_t ab = vzip_u32(a_high, b_high);
  return vcombine_u32(ab.val[0], ab.val[1]);
#endif
}

BL_INLINE_NODEBUG uint64x2_t simd_interleave_lo_u64(const uint64x2_t& a, const uint64x2_t& b) noexcept {
#if defined(BL_SIMD_AARCH64)
  return vzip1q_u64(a, b);
#else
  uint64x1_t a_low = vget_low_u64(a);
  uint64x1_t b_low = vget_low_u64(b);
  return vcombine_u64(a_low, b_low);
#endif
}

BL_INLINE_NODEBUG uint64x2_t simd_interleave_hi_u64(const uint64x2_t& a, const uint64x2_t& b) noexcept {
#if defined(BL_SIMD_AARCH64)
  return vzip2q_u64(a, b);
#else
  uint64x1_t a_high = vget_high_u64(a);
  uint64x1_t b_high = vget_high_u64(b);
  return vcombine_u64(a_high, b_high);
#endif
}

BL_INLINE_NODEBUG float32x4_t simd_interleave_lo_f32(const float32x4_t& a, const float32x4_t& b) noexcept {
#if defined(BL_SIMD_AARCH64)
  return vzip1q_f32(a, b);
#else
  float32x2_t a_low = vget_low_f32(a);
  float32x2_t b_low = vget_low_f32(b);
  float32x2x2_t ab = vzip_f32(a_low, b_low);
  return vcombine_f32(ab.val[0], ab.val[1]);
#endif
}

BL_INLINE_NODEBUG float32x4_t simd_interleave_hi_f32(const float32x4_t& a, const float32x4_t& b) noexcept {
#if defined(BL_SIMD_AARCH64)
  return vzip2q_f32(a, b);
#else
  float32x2_t a_high = vget_high_f32(a);
  float32x2_t b_high = vget_high_f32(b);
  float32x2x2_t ab = vzip_f32(a_high, b_high);
  return vcombine_f32(ab.val[0], ab.val[1]);
#endif
}

#if defined(BL_SIMD_AARCH64)
BL_INLINE_NODEBUG float64x2_t simd_interleave_lo_f64(const float64x2_t& a, const float64x2_t& b) noexcept {
  return vzip1q_f64(a, b);
}

BL_INLINE_NODEBUG float64x2_t simd_interleave_hi_f64(const float64x2_t& a, const float64x2_t& b) noexcept {
  return vzip2q_f64(a, b);
}
#endif // BL_SIMD_AARCH64

template<int kN>
BL_INLINE_NODEBUG uint8x16_t simd_alignr_u128(const uint8x16_t& a, const uint8x16_t& b) noexcept { return vextq_u8(b, a, kN); }

} // {Internal}

// SIMD - Internal - Integer Packing & Unpacking
// =============================================

namespace Internal {

BL_INLINE_NODEBUG int8x16_t simd_packs_128_i16_i8(const int16x8_t& a) noexcept {
  int8x8_t packed = vqmovn_s16(a);
  return vcombine_s8(packed, packed);
}

BL_INLINE_NODEBUG uint8x16_t simd_packs_128_i16_u8(const int16x8_t& a) noexcept {
  uint8x8_t packed = vqmovun_s16(a);
  return vcombine_u8(packed, packed);
}

BL_INLINE_NODEBUG uint8x16_t simd_packs_128_u16_u8(const uint16x8_t& a) noexcept {
  uint8x8_t packed = vqmovn_u16(a);
  return vcombine_u8(packed, packed);
}

BL_INLINE_NODEBUG uint8x16_t simd_packz_128_u16_u8(const uint16x8_t& a) noexcept {
  uint8x8_t packed = vmovn_u16(a);
  return vcombine_u8(packed, packed);
}

BL_INLINE_NODEBUG int16x8_t simd_packs_128_i32_i16(const int32x4_t& a) noexcept {
  int16x4_t packed = vqmovn_s32(a);
  return vcombine_s16(packed, packed);
}

BL_INLINE_NODEBUG uint16x8_t simd_packs_128_i32_u16(const int32x4_t& a) noexcept {
  uint16x4_t packed = vqmovun_s32(a);
  return vcombine_u16(packed, packed);
}

BL_INLINE_NODEBUG uint16x8_t simd_packs_128_u32_u16(const uint32x4_t& a) noexcept {
  uint16x4_t packed = vqmovn_u32(a);
  return vcombine_u16(packed, packed);
}

BL_INLINE_NODEBUG uint16x8_t simd_packz_128_u32_u16(const uint32x4_t& a) noexcept {
  uint16x4_t packed = vmovn_u32(a);
  return vcombine_u16(packed, packed);
}

BL_INLINE_NODEBUG int8x16_t simd_packs_128_i32_i8(const int32x4_t& a) noexcept {
  int16x4_t p16 = vqmovn_s32(a);
  int8x8_t p8 = vqmovn_s16(vcombine_s16(p16, p16));
  return vcombine_s8(p8, p8);
}

BL_INLINE_NODEBUG uint8x16_t simd_packs_128_i32_u8(const int32x4_t& a) noexcept {
  int16x4_t p16 = vqmovn_s32(a);
  uint8x8_t p8 = vqmovun_s16(vcombine_s16(p16, p16));
  return vcombine_u8(p8, p8);
}

BL_INLINE_NODEBUG uint8x16_t simd_packs_128_u32_u8(const uint32x4_t& a) noexcept {
  uint16x4_t p16 = vqmovn_u32(a);
  uint8x8_t p8 = vqmovn_u16(vcombine_u16(p16, p16));
  return vcombine_u8(p8, p8);
}

BL_INLINE_NODEBUG uint8x16_t simd_packz_128_u32_u8(const uint32x4_t& a) noexcept {
  uint16x4_t p16 = vmovn_u32(a);
  uint8x8_t p8 = vmovn_u16(vcombine_u16(p16, p16));
  return vcombine_u8(p8, p8);
}

BL_INLINE_NODEBUG int8x16_t simd_packs_128_i16_i8(const int16x8_t& a, const int16x8_t& b) noexcept {
  return vcombine_s8(vqmovn_s16(a), vqmovn_s16(b));
}

BL_INLINE_NODEBUG uint8x16_t simd_packs_128_i16_u8(const int16x8_t& a, const int16x8_t& b) noexcept {
  return vcombine_u8(vqmovun_s16(a), vqmovun_s16(b));
}

BL_INLINE_NODEBUG uint8x16_t simd_packs_128_u16_u8(const uint16x8_t& a, const uint16x8_t& b) noexcept {
  return vcombine_u8(vqmovn_u16(a), vqmovn_u16(b));
}

BL_INLINE_NODEBUG uint8x16_t simd_packz_128_u16_u8(const uint16x8_t& a, const uint16x8_t& b) noexcept {
  return vcombine_u8(vmovn_u16(a), vmovn_u16(b));
}

BL_INLINE_NODEBUG int16x8_t simd_packs_128_i32_i16(const int32x4_t& a, const int32x4_t& b) noexcept {
  return vcombine_s16(vqmovn_s32(a), vqmovn_s32(b));
}

BL_INLINE_NODEBUG uint16x8_t simd_packs_128_i32_u16(const int32x4_t& a, const int32x4_t& b) noexcept {
  return vcombine_u16(vqmovun_s32(a), vqmovun_s32(b));
}

BL_INLINE_NODEBUG uint16x8_t simd_packs_128_u32_u16(const uint32x4_t& a, const uint32x4_t& b) noexcept {
  return vcombine_u16(vqmovn_u32(a), vqmovn_u32(b));
}

BL_INLINE_NODEBUG uint16x8_t simd_packz_128_u32_u16(const uint32x4_t& a, const uint32x4_t& b) noexcept {
  return vcombine_u16(vmovn_u32(a), vmovn_u32(b));
}

BL_INLINE_NODEBUG int8x16_t simd_packs_128_i32_i8(const int32x4_t& a, const int32x4_t& b) noexcept {
  return simd_packs_128_i16_i8(simd_packs_128_i32_i16(a, b));
}

BL_INLINE_NODEBUG uint8x16_t simd_packs_128_i32_u8(const int32x4_t& a, const int32x4_t& b) noexcept {
  return simd_packs_128_i16_u8(simd_packs_128_i32_i16(a, b));
}

BL_INLINE_NODEBUG uint8x16_t simd_packs_128_u32_u8(const uint32x4_t& a, const uint32x4_t& b) noexcept {
  return simd_packs_128_u16_u8(simd_packs_128_u32_u16(a, b));
}

BL_INLINE_NODEBUG uint8x16_t simd_packz_128_u32_u8(const uint32x4_t& a, const uint32x4_t& b) noexcept {
  return simd_packz_128_u16_u8(simd_packz_128_u32_u16(a, b));
}

BL_INLINE_NODEBUG int8x16_t simd_packs_128_i32_i8(const int32x4_t& a, const int32x4_t& b, const int32x4_t& c, const int32x4_t& d) noexcept {
  return simd_packs_128_i16_i8(simd_packs_128_i32_i16(a, b), simd_packs_128_i32_i16(c, d));
}

BL_INLINE_NODEBUG uint8x16_t simd_packs_128_i32_u8(const int32x4_t& a, const int32x4_t& b, const int32x4_t& c, const int32x4_t& d) noexcept {
  return simd_packs_128_i16_u8(simd_packs_128_i32_i16(a, b), simd_packs_128_i32_i16(c, d));
}

BL_INLINE_NODEBUG uint8x16_t simd_packs_128_u32_u8(const uint32x4_t& a, const uint32x4_t& b, const uint32x4_t& c, const uint32x4_t& d) noexcept {
  return simd_packs_128_u16_u8(simd_packs_128_u32_u16(a, b), simd_packs_128_u32_u16(c, d));
}

BL_INLINE_NODEBUG uint8x16_t simd_packz_128_u32_u8(const uint32x4_t& a, const uint32x4_t& b, const uint32x4_t& c, const uint32x4_t& d) noexcept {
  return simd_packz_128_u16_u8(simd_packz_128_u32_u16(a, b), simd_packz_128_u32_u16(c, d));
}

BL_INLINE_NODEBUG int16x8_t simd_unpack_lo64_i8_i16(const int8x16_t& a) noexcept { return vmovl_s8(vget_low_s8(a)); }
BL_INLINE_NODEBUG int16x8_t simd_unpack_hi64_i8_i16(const int8x16_t& a) noexcept { return vmovl_s8(vget_high_s8(a)); }
BL_INLINE_NODEBUG uint16x8_t simd_unpack_lo64_u8_u16(const uint8x16_t& a) noexcept { return vmovl_u8(vget_low_u8(a)); }
BL_INLINE_NODEBUG uint16x8_t simd_unpack_hi64_u8_u16(const uint8x16_t& a) noexcept { return vmovl_u8(vget_high_u8(a)); }
BL_INLINE_NODEBUG int32x4_t simd_unpack_lo64_i16_i32(const int16x8_t& a) noexcept { return vmovl_s16(vget_low_s16(a)); }
BL_INLINE_NODEBUG int32x4_t simd_unpack_hi64_i16_i32(const int16x8_t& a) noexcept { return vmovl_s16(vget_high_s16(a)); }
BL_INLINE_NODEBUG uint32x4_t simd_unpack_lo64_u16_u32(const uint16x8_t& a) noexcept { return vmovl_u16(vget_low_u16(a)); }
BL_INLINE_NODEBUG uint32x4_t simd_unpack_hi64_u16_u32(const uint16x8_t& a) noexcept { return vmovl_u16(vget_high_u16(a)); }
BL_INLINE_NODEBUG int64x2_t simd_unpack_lo64_i32_i64(const int32x4_t& a) noexcept { return vmovl_s32(vget_low_s32(a)); }
BL_INLINE_NODEBUG int64x2_t simd_unpack_hi64_i32_i64(const int32x4_t& a) noexcept { return vmovl_s32(vget_high_s32(a)); }
BL_INLINE_NODEBUG uint64x2_t simd_unpack_lo64_u32_u64(const uint32x4_t& a) noexcept { return vmovl_u32(vget_low_u32(a)); }
BL_INLINE_NODEBUG uint64x2_t simd_unpack_hi64_u32_u64(const uint32x4_t& a) noexcept { return vmovl_u32(vget_high_u32(a)); }
BL_INLINE_NODEBUG int32x4_t simd_unpack_lo32_i8_i32(const int8x16_t& a) noexcept { return vmovl_s16(vget_low_s16(vmovl_s8(vget_low_s8(a)))); }
BL_INLINE_NODEBUG uint32x4_t simd_unpack_lo32_u8_u32(const uint8x16_t& a) noexcept { return vmovl_u16(vget_low_u16(vmovl_u8(vget_low_u8(a)))); }

BL_INLINE_NODEBUG int16x8_t simd_movw_i8_i16(const int8x16_t& a) noexcept { return simd_unpack_lo64_i8_i16(a); }
BL_INLINE_NODEBUG uint16x8_t simd_movw_u8_u16(const uint8x16_t& a) noexcept { return simd_unpack_lo64_u8_u16(a); }
BL_INLINE_NODEBUG int32x4_t simd_movw_i16_i32(const int16x8_t& a) noexcept { return simd_unpack_lo64_i16_i32(a); }
BL_INLINE_NODEBUG uint32x4_t simd_movw_u16_u32(const uint16x8_t& a) noexcept { return simd_unpack_lo64_u16_u32(a); }
BL_INLINE_NODEBUG int64x2_t simd_movw_i32_i64(const int32x4_t& a) noexcept { return simd_unpack_lo64_i32_i64(a); }
BL_INLINE_NODEBUG uint64x2_t simd_movw_u32_u64(const uint32x4_t& a) noexcept { return simd_unpack_lo64_u32_u64(a); }
BL_INLINE_NODEBUG int32x4_t simd_movw_i8_i32(const int8x16_t& a) noexcept { return simd_unpack_lo32_i8_i32(a); }
BL_INLINE_NODEBUG uint32x4_t simd_movw_u8_u32(const uint8x16_t& a) noexcept { return simd_unpack_lo32_u8_u32(a); }

} // {Internal}

// SIMD - Extract & Insert
// =======================

template<uint32_t kIndex, typename V>
BL_INLINE_NODEBUG uint32_t extract_u8(const V& src) noexcept { return vdupb_laneq_u8(simd_u8(src.v), kIndex); }

template<uint32_t kIndex, typename V>
BL_INLINE_NODEBUG uint32_t extract_u16(const V& src) noexcept { return vduph_laneq_u16(simd_u16(src.v), kIndex); }

template<uint32_t kIndex, typename V>
BL_INLINE_NODEBUG uint32_t extract_u32(const V& src) noexcept { return vdups_laneq_u32(simd_u32(src.v), kIndex); }

#if BL_TARGET_ARCH_BITS >= 64
template<uint32_t kIndex, typename V>
BL_INLINE_NODEBUG uint64_t extract_u64(const V& src) noexcept { return vdupd_laneq_u64(simd_u64(src.v), kIndex); }
#endif // BL_TARGET_ARCH_BITS

// SIMD - Internal - Arithmetic and Logical Operations
// ===================================================

namespace Internal {

BL_INLINE_NODEBUG uint8x16_t simd_not(const uint8x16_t& a) noexcept { return vmvnq_u8(a); }

BL_INLINE_NODEBUG uint8x16_t simd_and(const uint8x16_t& a, const uint8x16_t& b) noexcept { return vandq_u8(a, b); }
BL_INLINE_NODEBUG uint8x16_t simd_andnot(const uint8x16_t& a, const uint8x16_t& b) noexcept { return vbicq_u8(b, a); }
BL_INLINE_NODEBUG uint8x16_t simd_or(const uint8x16_t& a, const uint8x16_t& b) noexcept { return vorrq_u8(a, b); }
BL_INLINE_NODEBUG uint8x16_t simd_xor(const uint8x16_t& a, const uint8x16_t& b) noexcept { return veorq_u8(a, b); }

BL_INLINE_NODEBUG uint8x16_t simd_blendv_bits_u8(const uint8x16_t& a, const uint8x16_t& b, const uint8x16_t& msk) noexcept { return vbslq_u8(msk, b, a); }
BL_INLINE_NODEBUG uint8x16_t simd_blendv_u8(const uint8x16_t& a, const uint8x16_t& b, const uint8x16_t& msk) noexcept { return simd_blendv_bits_u8(a, b, msk); }

template<uint32_t H, uint32_t G, uint32_t F, uint32_t E, uint32_t D, uint32_t C, uint32_t B, uint32_t A>
BL_INLINE_NODEBUG uint8x16_t simd_blend_u16(const uint8x16_t& a, const uint8x16_t& b) noexcept {
  alignas(16) static constexpr uint16_t msk[8] = {
    uint16_t(A ? 0xFFFFu : 0u), uint16_t(B ? 0xFFFFu : 0u),
    uint16_t(C ? 0xFFFFu : 0u), uint16_t(D ? 0xFFFFu : 0u),
    uint16_t(E ? 0xFFFFu : 0u), uint16_t(F ? 0xFFFFu : 0u),
    uint16_t(G ? 0xFFFFu : 0u), uint16_t(H ? 0xFFFFu : 0u)
  };
  return simd_blendv_bits_u8(a, b, simd_u8(vld1q_u16(msk)));
}

template<uint32_t D, uint32_t C, uint32_t B, uint32_t A>
BL_INLINE_NODEBUG uint8x16_t simd_blend_u32(const uint8x16_t& a, const uint8x16_t& b) noexcept {
  alignas(16) static constexpr uint32_t msk[8] = {
    uint32_t(A ? 0xFFFFFFFFu : 0u),
    uint32_t(B ? 0xFFFFFFFFu : 0u),
    uint32_t(C ? 0xFFFFFFFFu : 0u),
    uint32_t(D ? 0xFFFFFFFFu : 0u)
  };
  return simd_blendv_bits_u8(a, b, simd_u8(vld1q_u32(msk)));
}

template<uint32_t B, uint32_t A>
BL_INLINE_NODEBUG uint8x16_t simd_blend_u64(const uint8x16_t& a, const uint8x16_t& b) noexcept {
  alignas(16) static constexpr uint64_t msk[2] = {
    uint64_t(A ? 0xFFFFFFFFFFFFFFFFu : 0u),
    uint64_t(B ? 0xFFFFFFFFFFFFFFFFu : 0u)
  };
  return simd_blendv_bits_u8(a, b, simd_u8(vld1q_u64(msk)));
}

template<typename SimdT>
BL_INLINE_NODEBUG SimdT simd_blendv_bits(const SimdT& a, const SimdT& b, const SimdT& msk) noexcept {
  return simd_cast<SimdT>(simd_blendv_bits_u8(simd_u8(a), simd_u8(b), simd_u8(msk)));
}

BL_INLINE_NODEBUG float32x4_t simd_abs_f32(const float32x4_t& a) noexcept { return vabsq_f32(a); }

BL_INLINE_NODEBUG float32x4_t simd_sqrt_f32(const float32x4_t& a) noexcept {
#if defined(BL_SIMD_AARCH64)
  return vsqrtq_f32(a);
#else
  float32x4_t rcp = vrsqrteq_f32(a);

  rcp = vmulq_f32(rcp, vrsqrtsq_f32(vmulq_f32(a, rcp), rcp));
  rcp = vmulq_f32(rcp, vrsqrtsq_f32(vmulq_f32(a, rcp), rcp));
  rcp = vmulq_f32(rcp, vrsqrtsq_f32(vmulq_f32(a, rcp), rcp));

  float32x4_t zero = simd_make128_f32(0.0f);
  float32x4_t root = vmulq_f32(rcp, a);

  return simd_f32(
    simd_blendv_bits_u8(
      simd_u8(root),
      simd_u8(zero),
      simd_u8(vceqq_f32(a, zero))));
#endif
}

BL_INLINE_NODEBUG float32x4_t simd_add_f32(const float32x4_t& a, const float32x4_t& b) noexcept { return vaddq_f32(a, b); }
BL_INLINE_NODEBUG float32x4_t simd_sub_f32(const float32x4_t& a, const float32x4_t& b) noexcept { return vsubq_f32(a, b); }
BL_INLINE_NODEBUG float32x4_t simd_mul_f32(const float32x4_t& a, const float32x4_t& b) noexcept { return vmulq_f32(a, b); }
BL_INLINE_NODEBUG float32x4_t simd_min_f32(const float32x4_t& a, const float32x4_t& b) noexcept { return vminq_f32(a, b); }
BL_INLINE_NODEBUG float32x4_t simd_max_f32(const float32x4_t& a, const float32x4_t& b) noexcept { return vmaxq_f32(a, b); }

#if defined(BL_SIMD_AARCH64)
BL_INLINE_NODEBUG float32x4_t simd_div_f32(const float32x4_t& a, const float32x4_t& b) noexcept { return vdivq_f32(a, b); }
#else
BL_INLINE_NODEBUG float32x4_t simd_div_f32(const float32x4_t& a, const float32x4_t& b) noexcept {
  float32x4_t rcp = vrecpeq_f32(b);
  rcp = vmulq_f32(rcp, vrecpsq_f32(rcp, b));
  rcp = vmulq_f32(rcp, vrecpsq_f32(rcp, b));
  rcp = vmulq_f32(rcp, vrecpsq_f32(rcp, b));
  return vmulq_f32(a, rcp);
}
#endif

BL_INLINE_NODEBUG float32x4_t simd_cmp_eq_f32(const float32x4_t& a, const float32x4_t& b) noexcept { return simd_f32(vceqq_f32(a, b)); }
BL_INLINE_NODEBUG float32x4_t simd_cmp_ne_f32(const float32x4_t& a, const float32x4_t& b) noexcept { return simd_f32(vmvnq_u32(vceqq_f32(a, b))); }
BL_INLINE_NODEBUG float32x4_t simd_cmp_lt_f32(const float32x4_t& a, const float32x4_t& b) noexcept { return simd_f32(vcltq_f32(a, b)); }
BL_INLINE_NODEBUG float32x4_t simd_cmp_le_f32(const float32x4_t& a, const float32x4_t& b) noexcept { return simd_f32(vcleq_f32(a, b)); }
BL_INLINE_NODEBUG float32x4_t simd_cmp_gt_f32(const float32x4_t& a, const float32x4_t& b) noexcept { return simd_f32(vcgtq_f32(a, b)); }
BL_INLINE_NODEBUG float32x4_t simd_cmp_ge_f32(const float32x4_t& a, const float32x4_t& b) noexcept { return simd_f32(vcgeq_f32(a, b)); }

#if defined(BL_SIMD_AARCH64)
BL_INLINE_NODEBUG float64x2_t simd_abs_f64(const float64x2_t& a) noexcept { return vabsq_f64(a); }
BL_INLINE_NODEBUG float64x2_t simd_sqrt_f64(const float64x2_t& a) noexcept { return vsqrtq_f64(a); }

BL_INLINE_NODEBUG float64x2_t simd_add_f64(const float64x2_t& a, const float64x2_t& b) noexcept { return vaddq_f64(a, b); }
BL_INLINE_NODEBUG float64x2_t simd_sub_f64(const float64x2_t& a, const float64x2_t& b) noexcept { return vsubq_f64(a, b); }
BL_INLINE_NODEBUG float64x2_t simd_mul_f64(const float64x2_t& a, const float64x2_t& b) noexcept { return vmulq_f64(a, b); }
BL_INLINE_NODEBUG float64x2_t simd_div_f64(const float64x2_t& a, const float64x2_t& b) noexcept { return vdivq_f64(a, b); }
BL_INLINE_NODEBUG float64x2_t simd_min_f64(const float64x2_t& a, const float64x2_t& b) noexcept { return vminq_f64(a, b); }
BL_INLINE_NODEBUG float64x2_t simd_max_f64(const float64x2_t& a, const float64x2_t& b) noexcept { return vmaxq_f64(a, b); }

BL_INLINE_NODEBUG float64x2_t simd_cmp_eq_f64(const float64x2_t& a, const float64x2_t& b) noexcept { return simd_f64(vceqq_f64(a, b)); }
BL_INLINE_NODEBUG float64x2_t simd_cmp_ne_f64(const float64x2_t& a, const float64x2_t& b) noexcept { return simd_f64(vmvnq_u32(simd_u32(vceqq_f64(a, b)))); }
BL_INLINE_NODEBUG float64x2_t simd_cmp_lt_f64(const float64x2_t& a, const float64x2_t& b) noexcept { return simd_f64(vcltq_f64(a, b)); }
BL_INLINE_NODEBUG float64x2_t simd_cmp_le_f64(const float64x2_t& a, const float64x2_t& b) noexcept { return simd_f64(vcleq_f64(a, b)); }
BL_INLINE_NODEBUG float64x2_t simd_cmp_gt_f64(const float64x2_t& a, const float64x2_t& b) noexcept { return simd_f64(vcgtq_f64(a, b)); }
BL_INLINE_NODEBUG float64x2_t simd_cmp_ge_f64(const float64x2_t& a, const float64x2_t& b) noexcept { return simd_f64(vcgeq_f64(a, b)); }
#endif // BL_SIMD_AARCH64

BL_INLINE_NODEBUG int8x16_t simd_add_i8(const int8x16_t& a, const int8x16_t& b) noexcept { return vaddq_s8(a, b); }
BL_INLINE_NODEBUG int16x8_t simd_add_i16(const int16x8_t& a, const int16x8_t& b) noexcept { return vaddq_s16(a, b); }
BL_INLINE_NODEBUG int32x4_t simd_add_i32(const int32x4_t& a, const int32x4_t& b) noexcept { return vaddq_s32(a, b); }
BL_INLINE_NODEBUG int64x2_t simd_add_i64(const int64x2_t& a, const int64x2_t& b) noexcept { return vaddq_s64(a, b); }
BL_INLINE_NODEBUG uint8x16_t simd_add_u8(const uint8x16_t& a, const uint8x16_t& b) noexcept { return vaddq_u8(a, b); }
BL_INLINE_NODEBUG uint16x8_t simd_add_u16(const uint16x8_t& a, const uint16x8_t& b) noexcept { return vaddq_u16(a, b); }
BL_INLINE_NODEBUG uint32x4_t simd_add_u32(const uint32x4_t& a, const uint32x4_t& b) noexcept { return vaddq_u32(a, b); }
BL_INLINE_NODEBUG uint64x2_t simd_add_u64(const uint64x2_t& a, const uint64x2_t& b) noexcept { return vaddq_u64(a, b); }

BL_INLINE_NODEBUG int8x16_t simd_adds_i8(const int8x16_t& a, const int8x16_t& b) noexcept { return vqaddq_s8(a, b); }
BL_INLINE_NODEBUG int16x8_t simd_adds_i16(const int16x8_t& a, const int16x8_t& b) noexcept { return vqaddq_s16(a, b); }
BL_INLINE_NODEBUG int32x4_t simd_adds_i32(const int32x4_t& a, const int32x4_t& b) noexcept { return vqaddq_s32(a, b); }
BL_INLINE_NODEBUG int64x2_t simd_adds_i64(const int64x2_t& a, const int64x2_t& b) noexcept { return vqaddq_s64(a, b); }
BL_INLINE_NODEBUG uint8x16_t simd_adds_u8(const uint8x16_t& a, const uint8x16_t& b) noexcept { return vqaddq_u8(a, b); }
BL_INLINE_NODEBUG uint16x8_t simd_adds_u16(const uint16x8_t& a, const uint16x8_t& b) noexcept { return vqaddq_u16(a, b); }
BL_INLINE_NODEBUG uint32x4_t simd_adds_u32(const uint32x4_t& a, const uint32x4_t& b) noexcept { return vqaddq_u32(a, b); }
BL_INLINE_NODEBUG uint64x2_t simd_adds_u64(const uint64x2_t& a, const uint64x2_t& b) noexcept { return vqaddq_u64(a, b); }

#if defined(BL_SIMD_AARCH64)
BL_INLINE_NODEBUG int16x8_t simd_addl_lo_i8_to_i16(const int8x16_t& a, const int8x16_t& b) noexcept { return vaddl_s8(vget_low_s8(a), vget_low_s8(b)); }
BL_INLINE_NODEBUG int16x8_t simd_addl_hi_i8_to_i16(const int8x16_t& a, const int8x16_t& b) noexcept { return vaddl_high_s8(a, b); }
BL_INLINE_NODEBUG uint16x8_t simd_addl_lo_u8_to_u16(const uint8x16_t& a, const uint8x16_t& b) noexcept { return vaddl_u8(vget_low_u8(a), vget_low_u8(b)); }
BL_INLINE_NODEBUG uint16x8_t simd_addl_hi_u8_to_u16(const uint8x16_t& a, const uint8x16_t& b) noexcept { return vaddl_high_u8(a, b); }
BL_INLINE_NODEBUG int32x4_t simd_addl_lo_i16_to_i32(const int16x8_t& a, const int16x8_t& b) noexcept { return vaddl_s16(vget_low_s16(a), vget_low_s16(b)); }
BL_INLINE_NODEBUG int32x4_t simd_addl_hi_i16_to_i32(const int16x8_t& a, const int16x8_t& b) noexcept { return vaddl_high_s16(a, b); }
BL_INLINE_NODEBUG uint32x4_t simd_addl_lo_u16_to_u32(const uint16x8_t& a, const uint16x8_t& b) noexcept { return vaddl_u16(vget_low_u16(a), vget_low_u16(b)); }
BL_INLINE_NODEBUG uint32x4_t simd_addl_hi_u16_to_u32(const uint16x8_t& a, const uint16x8_t& b) noexcept { return vaddl_high_u16(a, b); }
BL_INLINE_NODEBUG int64x2_t simd_addl_lo_i32_to_i64(const int32x4_t& a, const int32x4_t& b) noexcept { return vaddl_s32(vget_low_s32(a), vget_low_s32(b)); }
BL_INLINE_NODEBUG int64x2_t simd_addl_hi_i32_to_i64(const int32x4_t& a, const int32x4_t& b) noexcept { return vaddl_high_s32(a, b); }
BL_INLINE_NODEBUG uint64x2_t simd_addl_lo_u32_to_u64(const uint32x4_t& a, const uint32x4_t& b) noexcept { return vaddl_u32(vget_low_u32(a), vget_low_u32(b)); }
BL_INLINE_NODEBUG uint64x2_t simd_addl_hi_u32_to_u64(const uint32x4_t& a, const uint32x4_t& b) noexcept { return vaddl_high_u32(a, b); }

BL_INLINE_NODEBUG int16x8_t simd_addw_lo_i8_to_i16(const int16x8_t& a, const int8x16_t& b) noexcept { return vaddw_s8(a, vget_low_s8(b)); }
BL_INLINE_NODEBUG int16x8_t simd_addw_hi_i8_to_i16(const int16x8_t& a, const int8x16_t& b) noexcept { return vaddw_high_s8(a, b); }
BL_INLINE_NODEBUG uint16x8_t simd_addw_lo_u8_to_u16(const uint16x8_t& a, const uint8x16_t& b) noexcept { return vaddw_u8(a, vget_low_u8(b)); }
BL_INLINE_NODEBUG uint16x8_t simd_addw_hi_u8_to_u16(const uint16x8_t& a, const uint8x16_t& b) noexcept { return vaddw_high_u8(a, b); }
BL_INLINE_NODEBUG int32x4_t simd_addw_lo_i16_to_i32(const int32x4_t& a, const int16x8_t& b) noexcept { return vaddw_s16(a, vget_low_s16(b)); }
BL_INLINE_NODEBUG int32x4_t simd_addw_hi_i16_to_i32(const int32x4_t& a, const int16x8_t& b) noexcept { return vaddw_high_s16(a, b); }
BL_INLINE_NODEBUG uint32x4_t simd_addw_lo_u16_to_u32(const uint32x4_t& a, const uint16x8_t& b) noexcept { return vaddw_u16(a, vget_low_u16(b)); }
BL_INLINE_NODEBUG uint32x4_t simd_addw_hi_u16_to_u32(const uint32x4_t& a, const uint16x8_t& b) noexcept { return vaddw_high_u16(a, b); }
BL_INLINE_NODEBUG int64x2_t simd_addw_lo_i32_to_i64(const int64x2_t& a, const int32x4_t& b) noexcept { return vaddw_s32(a, vget_low_s32(b)); }
BL_INLINE_NODEBUG int64x2_t simd_addw_hi_i32_to_i64(const int64x2_t& a, const int32x4_t& b) noexcept { return vaddw_high_s32(a, b); }
BL_INLINE_NODEBUG uint64x2_t simd_addw_lo_u32_to_u64(const uint64x2_t& a, const uint32x4_t& b) noexcept { return vaddw_u32(a, vget_low_u32(b)); }
BL_INLINE_NODEBUG uint64x2_t simd_addw_hi_u32_to_u64(const uint64x2_t& a, const uint32x4_t& b) noexcept { return vaddw_high_u32(a, b); }
#else
BL_INLINE_NODEBUG int16x8_t simd_addl_lo_i8_to_i16(const int8x16_t& a, const int8x16_t& b) noexcept { return vaddl_s8(vget_low_s8(a), vget_low_s8(b)); }
BL_INLINE_NODEBUG int16x8_t simd_addl_hi_i8_to_i16(const int8x16_t& a, const int8x16_t& b) noexcept { return vaddl_s8(vget_high_s8(a), vget_high_s8(b)); }
BL_INLINE_NODEBUG uint16x8_t simd_addl_lo_u8_to_u16(const uint8x16_t& a, const uint8x16_t& b) noexcept { return vaddl_u8(vget_low_u8(a), vget_low_u8(b)); }
BL_INLINE_NODEBUG uint16x8_t simd_addl_hi_u8_to_u16(const uint8x16_t& a, const uint8x16_t& b) noexcept { return vaddl_u8(vget_high_u8(a), vget_high_u8(b)); }
BL_INLINE_NODEBUG int32x4_t simd_addl_lo_i16_to_i32(const int16x8_t& a, const int16x8_t& b) noexcept { return vaddl_s16(vget_low_s16(a), vget_low_s16(b)); }
BL_INLINE_NODEBUG int32x4_t simd_addl_hi_i16_to_i32(const int16x8_t& a, const int16x8_t& b) noexcept { return vaddl_s16(vget_high_s16(a), vget_high_s16(b)); }
BL_INLINE_NODEBUG uint32x4_t simd_addl_lo_u16_to_u32(const uint16x8_t& a, const uint16x8_t& b) noexcept { return vaddl_u16(vget_low_u16(a), vget_low_u16(b)); }
BL_INLINE_NODEBUG uint32x4_t simd_addl_hi_u16_to_u32(const uint16x8_t& a, const uint16x8_t& b) noexcept { return vaddl_u16(vget_high_u16(a), vget_high_u16(b)); }
BL_INLINE_NODEBUG int64x2_t simd_addl_lo_i32_to_i64(const int32x4_t& a, const int32x4_t& b) noexcept { return vaddl_s32(vget_low_s32(a), vget_low_s32(b)); }
BL_INLINE_NODEBUG int64x2_t simd_addl_hi_i32_to_i64(const int32x4_t& a, const int32x4_t& b) noexcept { return vaddl_s32(vget_high_s32(a), vget_high_s32(b)); }
BL_INLINE_NODEBUG uint64x2_t simd_addl_lo_u32_to_u64(const uint32x4_t& a, const uint32x4_t& b) noexcept { return vaddl_u32(vget_low_u32(a), vget_low_u32(b)); }
BL_INLINE_NODEBUG uint64x2_t simd_addl_hi_u32_to_u64(const uint32x4_t& a, const uint32x4_t& b) noexcept { return vaddl_u32(vget_high_u32(a), vget_high_u32(b)); }

BL_INLINE_NODEBUG int16x8_t simd_addw_lo_i8_to_i16(const int16x8_t& a, const int8x16_t& b) noexcept { return vaddw_s8(a, vget_low_s8(b)); }
BL_INLINE_NODEBUG int16x8_t simd_addw_hi_i8_to_i16(const int16x8_t& a, const int8x16_t& b) noexcept { return vaddw_s8(a, vget_high_s8(b)); }
BL_INLINE_NODEBUG uint16x8_t simd_addw_lo_u8_to_u16(const uint16x8_t& a, const uint8x16_t& b) noexcept { return vaddw_u8(a, vget_low_u8(b)); }
BL_INLINE_NODEBUG uint16x8_t simd_addw_hi_u8_to_u16(const uint16x8_t& a, const uint8x16_t& b) noexcept { return vaddw_u8(a, vget_high_u8(b)); }
BL_INLINE_NODEBUG int32x4_t simd_addw_lo_i16_to_i32(const int32x4_t& a, const int16x8_t& b) noexcept { return vaddw_s16(a, vget_low_s16(b)); }
BL_INLINE_NODEBUG int32x4_t simd_addw_hi_i16_to_i32(const int32x4_t& a, const int16x8_t& b) noexcept { return vaddw_s16(a, vget_high_s16(b)); }
BL_INLINE_NODEBUG uint32x4_t simd_addw_lo_u16_to_u32(const uint32x4_t& a, const uint16x8_t& b) noexcept { return vaddw_u16(a, vget_low_u16(b)); }
BL_INLINE_NODEBUG uint32x4_t simd_addw_hi_u16_to_u32(const uint32x4_t& a, const uint16x8_t& b) noexcept { return vaddw_u16(a, vget_high_u16(b)); }
BL_INLINE_NODEBUG int64x2_t simd_addw_lo_i32_to_i64(const int64x2_t& a, const int32x4_t& b) noexcept { return vaddw_s32(a, vget_low_s32(b)); }
BL_INLINE_NODEBUG int64x2_t simd_addw_hi_i32_to_i64(const int64x2_t& a, const int32x4_t& b) noexcept { return vaddw_s32(a, vget_high_s32(b)); }
BL_INLINE_NODEBUG uint64x2_t simd_addw_lo_u32_to_u64(const uint64x2_t& a, const uint32x4_t& b) noexcept { return vaddw_u32(a, vget_low_u32(b)); }
BL_INLINE_NODEBUG uint64x2_t simd_addw_hi_u32_to_u64(const uint64x2_t& a, const uint32x4_t& b) noexcept { return vaddw_u32(a, vget_high_u32(b)); }
#endif

BL_INLINE_NODEBUG int8x16_t simd_sub_i8(const int8x16_t& a, const int8x16_t& b) noexcept { return vsubq_s8(a, b); }
BL_INLINE_NODEBUG int16x8_t simd_sub_i16(const int16x8_t& a, const int16x8_t& b) noexcept { return vsubq_s16(a, b); }
BL_INLINE_NODEBUG int32x4_t simd_sub_i32(const int32x4_t& a, const int32x4_t& b) noexcept { return vsubq_s32(a, b); }
BL_INLINE_NODEBUG int64x2_t simd_sub_i64(const int64x2_t& a, const int64x2_t& b) noexcept { return vsubq_s64(a, b); }
BL_INLINE_NODEBUG uint8x16_t simd_sub_u8(const uint8x16_t& a, const uint8x16_t& b) noexcept { return vsubq_u8(a, b); }
BL_INLINE_NODEBUG uint16x8_t simd_sub_u16(const uint16x8_t& a, const uint16x8_t& b) noexcept { return vsubq_u16(a, b); }
BL_INLINE_NODEBUG uint32x4_t simd_sub_u32(const uint32x4_t& a, const uint32x4_t& b) noexcept { return vsubq_u32(a, b); }
BL_INLINE_NODEBUG uint64x2_t simd_sub_u64(const uint64x2_t& a, const uint64x2_t& b) noexcept { return vsubq_u64(a, b); }

BL_INLINE_NODEBUG int8x16_t simd_subs_i8(const int8x16_t& a, const int8x16_t& b) noexcept { return vqsubq_s8(a, b); }
BL_INLINE_NODEBUG int16x8_t simd_subs_i16(const int16x8_t& a, const int16x8_t& b) noexcept { return vqsubq_s16(a, b); }
BL_INLINE_NODEBUG int32x4_t simd_subs_i32(const int32x4_t& a, const int32x4_t& b) noexcept { return vqsubq_s32(a, b); }
BL_INLINE_NODEBUG int64x2_t simd_subs_i64(const int64x2_t& a, const int64x2_t& b) noexcept { return vqsubq_s64(a, b); }
BL_INLINE_NODEBUG uint8x16_t simd_subs_u8(const uint8x16_t& a, const uint8x16_t& b) noexcept { return vqsubq_u8(a, b); }
BL_INLINE_NODEBUG uint16x8_t simd_subs_u16(const uint16x8_t& a, const uint16x8_t& b) noexcept { return vqsubq_u16(a, b); }
BL_INLINE_NODEBUG uint32x4_t simd_subs_u32(const uint32x4_t& a, const uint32x4_t& b) noexcept { return vqsubq_u32(a, b); }
BL_INLINE_NODEBUG uint64x2_t simd_subs_u64(const uint64x2_t& a, const uint64x2_t& b) noexcept { return vqsubq_u64(a, b); }

BL_INLINE_NODEBUG int8x16_t simd_mul_i8(const int8x16_t& a, const int8x16_t& b) noexcept { return vmulq_s8(a, b); }
BL_INLINE_NODEBUG int16x8_t simd_mul_i16(const int16x8_t& a, const int16x8_t& b) noexcept { return vmulq_s16(a, b); }
BL_INLINE_NODEBUG int32x4_t simd_mul_i32(const int32x4_t& a, const int32x4_t& b) noexcept { return vmulq_s32(a, b); }

BL_INLINE_NODEBUG int64x2_t simd_mul_i64(const int64x2_t& a, const int64x2_t& b) noexcept {
  uint32x4_t hi = vmulq_u32(simd_u32(b), vrev64q_u32(simd_u32(a)));
  return simd_i64(
    vmlal_u32(vshlq_n_u64(simd_u64(vpaddlq_u32(hi)), 32),
              vmovn_u64(simd_u64(a)),
              vmovn_u64(simd_u64(b))));
}

BL_INLINE_NODEBUG uint8x16_t simd_mul_u8(const uint8x16_t& a, const uint8x16_t& b) noexcept { return vmulq_u8(a, b); }
BL_INLINE_NODEBUG uint16x8_t simd_mul_u16(const uint16x8_t& a, const uint16x8_t& b) noexcept { return vmulq_u16(a, b); }
BL_INLINE_NODEBUG uint32x4_t simd_mul_u32(const uint32x4_t& a, const uint32x4_t& b) noexcept { return vmulq_u32(a, b); }
BL_INLINE_NODEBUG uint64x2_t simd_mul_u64(const uint64x2_t& a, const uint64x2_t& b) noexcept { return simd_u64(simd_mul_i64(simd_i64(a), simd_i64(b))); }

BL_INLINE_NODEBUG uint16x8_t simd_mul_lo_u8_u16(const uint8x16_t& a, const uint8x16_t& b) noexcept { return vmull_u8(vget_low_u8(a), vget_low_u8(b)); }
BL_INLINE_NODEBUG uint16x8_t simd_mul_hi_u8_u16(const uint8x16_t& a, const uint8x16_t& b) noexcept { return vmull_u8(vget_high_u8(a), vget_high_u8(b)); }

BL_INLINE_NODEBUG uint32x4_t simd_mul_lo_u16_u32(const uint16x8_t& a, const uint16x8_t& b) noexcept { return vmull_u16(vget_low_u16(a), vget_low_u16(b)); }
BL_INLINE_NODEBUG uint32x4_t simd_mul_hi_u16_u32(const uint16x8_t& a, const uint16x8_t& b) noexcept { return vmull_u16(vget_high_u16(a), vget_high_u16(b)); }

BL_INLINE_NODEBUG int8x16_t simd_cmp_eq_i8(const int8x16_t& a, const int8x16_t& b) noexcept { return simd_i8(vceqq_s8(a, b)); }
BL_INLINE_NODEBUG int16x8_t simd_cmp_eq_i16(const int16x8_t& a, const int16x8_t& b) noexcept { return simd_i16(vceqq_s16(a, b)); }
BL_INLINE_NODEBUG int32x4_t simd_cmp_eq_i32(const int32x4_t& a, const int32x4_t& b) noexcept { return simd_i32(vceqq_s32(a, b)); }

BL_INLINE_NODEBUG uint8x16_t simd_cmp_eq_u8(const uint8x16_t& a, const uint8x16_t& b) noexcept { return simd_u8(vceqq_u8(a, b)); }
BL_INLINE_NODEBUG uint16x8_t simd_cmp_eq_u16(const uint16x8_t& a, const uint16x8_t& b) noexcept { return simd_u16(vceqq_u16(a, b)); }
BL_INLINE_NODEBUG uint32x4_t simd_cmp_eq_u32(const uint32x4_t& a, const uint32x4_t& b) noexcept { return simd_u32(vceqq_u32(a, b)); }

BL_INLINE_NODEBUG int8x16_t simd_cmp_ne_i8(const int8x16_t& a, const int8x16_t& b) noexcept { return simd_i8(vmvnq_u8(vceqq_s8(a, b))); }
BL_INLINE_NODEBUG int16x8_t simd_cmp_ne_i16(const int16x8_t& a, const int16x8_t& b) noexcept { return simd_i16(vmvnq_u16(vceqq_s16(a, b))); }
BL_INLINE_NODEBUG int32x4_t simd_cmp_ne_i32(const int32x4_t& a, const int32x4_t& b) noexcept { return simd_i32(vmvnq_u32(vceqq_s32(a, b))); }

BL_INLINE_NODEBUG uint8x16_t simd_cmp_ne_u8(const uint8x16_t& a, const uint8x16_t& b) noexcept { return simd_u8(vmvnq_u8(vceqq_u8(a, b))); }
BL_INLINE_NODEBUG uint16x8_t simd_cmp_ne_u16(const uint16x8_t& a, const uint16x8_t& b) noexcept { return simd_u16(vmvnq_u16(vceqq_u16(a, b))); }
BL_INLINE_NODEBUG uint32x4_t simd_cmp_ne_u32(const uint32x4_t& a, const uint32x4_t& b) noexcept { return simd_u32(vmvnq_u32(vceqq_u32(a, b))); }

BL_INLINE_NODEBUG int8x16_t simd_cmp_gt_i8(const int8x16_t& a, const int8x16_t& b) noexcept { return simd_i8(vcgtq_s8(a, b)); }
BL_INLINE_NODEBUG int16x8_t simd_cmp_gt_i16(const int16x8_t& a, const int16x8_t& b) noexcept { return simd_i16(vcgtq_s16(a, b)); }
BL_INLINE_NODEBUG int32x4_t simd_cmp_gt_i32(const int32x4_t& a, const int32x4_t& b) noexcept { return simd_i32(vcgtq_s32(a, b)); }

BL_INLINE_NODEBUG uint8x16_t simd_cmp_gt_u8(const uint8x16_t& a, const uint8x16_t& b) noexcept { return simd_u8(vcgtq_u8(a, b)); }
BL_INLINE_NODEBUG uint16x8_t simd_cmp_gt_u16(const uint16x8_t& a, const uint16x8_t& b) noexcept { return simd_u16(vcgtq_u16(a, b)); }
BL_INLINE_NODEBUG uint32x4_t simd_cmp_gt_u32(const uint32x4_t& a, const uint32x4_t& b) noexcept { return simd_u32(vcgtq_u32(a, b)); }

BL_INLINE_NODEBUG int8x16_t simd_cmp_ge_i8(const int8x16_t& a, const int8x16_t& b) noexcept { return simd_i8(vcgeq_s8(a, b)); }
BL_INLINE_NODEBUG int16x8_t simd_cmp_ge_i16(const int16x8_t& a, const int16x8_t& b) noexcept { return simd_i16(vcgeq_s16(a, b)); }
BL_INLINE_NODEBUG int32x4_t simd_cmp_ge_i32(const int32x4_t& a, const int32x4_t& b) noexcept { return simd_i32(vcgeq_s32(a, b)); }

BL_INLINE_NODEBUG uint8x16_t simd_cmp_ge_u8(const uint8x16_t& a, const uint8x16_t& b) noexcept { return simd_u8(vcgeq_u8(a, b)); }
BL_INLINE_NODEBUG uint16x8_t simd_cmp_ge_u16(const uint16x8_t& a, const uint16x8_t& b) noexcept { return simd_u16(vcgeq_u16(a, b)); }
BL_INLINE_NODEBUG uint32x4_t simd_cmp_ge_u32(const uint32x4_t& a, const uint32x4_t& b) noexcept { return simd_u32(vcgeq_u32(a, b)); }

BL_INLINE_NODEBUG int8x16_t simd_cmp_lt_i8(const int8x16_t& a, const int8x16_t& b) noexcept { return simd_i8(vcltq_s8(a, b)); }
BL_INLINE_NODEBUG int16x8_t simd_cmp_lt_i16(const int16x8_t& a, const int16x8_t& b) noexcept { return simd_i16(vcltq_s16(a, b)); }
BL_INLINE_NODEBUG int32x4_t simd_cmp_lt_i32(const int32x4_t& a, const int32x4_t& b) noexcept { return simd_i32(vcltq_s32(a, b)); }

BL_INLINE_NODEBUG uint8x16_t simd_cmp_lt_u8(const uint8x16_t& a, const uint8x16_t& b) noexcept { return simd_u8(vcltq_u8(a, b)); }
BL_INLINE_NODEBUG uint16x8_t simd_cmp_lt_u16(const uint16x8_t& a, const uint16x8_t& b) noexcept { return simd_u16(vcltq_u16(a, b)); }
BL_INLINE_NODEBUG uint32x4_t simd_cmp_lt_u32(const uint32x4_t& a, const uint32x4_t& b) noexcept { return simd_u32(vcltq_u32(a, b)); }

BL_INLINE_NODEBUG int8x16_t simd_cmp_le_i8(const int8x16_t& a, const int8x16_t& b) noexcept { return simd_i8(vcleq_s8(a, b)); }
BL_INLINE_NODEBUG int16x8_t simd_cmp_le_i16(const int16x8_t& a, const int16x8_t& b) noexcept { return simd_i16(vcleq_s16(a, b)); }
BL_INLINE_NODEBUG int32x4_t simd_cmp_le_i32(const int32x4_t& a, const int32x4_t& b) noexcept { return simd_i32(vcleq_s32(a, b)); }

BL_INLINE_NODEBUG uint8x16_t simd_cmp_le_u8(const uint8x16_t& a, const uint8x16_t& b) noexcept { return simd_u8(vcleq_u8(a, b)); }
BL_INLINE_NODEBUG uint16x8_t simd_cmp_le_u16(const uint16x8_t& a, const uint16x8_t& b) noexcept { return simd_u16(vcleq_u16(a, b)); }
BL_INLINE_NODEBUG uint32x4_t simd_cmp_le_u32(const uint32x4_t& a, const uint32x4_t& b) noexcept { return simd_u32(vcleq_u32(a, b)); }

#if defined(BL_SIMD_AARCH64)
BL_INLINE_NODEBUG int64x2_t simd_cmp_eq_i64(const int64x2_t& a, const int64x2_t& b) noexcept { return simd_i64(vceqq_s64(a, b));}
BL_INLINE_NODEBUG uint64x2_t simd_cmp_eq_u64(const uint64x2_t& a, const uint64x2_t& b) noexcept { return simd_u64(vceqq_u64(a, b)); }

BL_INLINE_NODEBUG int64x2_t simd_cmp_ne_i64(const int64x2_t& a, const int64x2_t& b) noexcept { return simd_i64(vmvnq_u32(simd_u32(vceqq_s64(a, b)))); }
BL_INLINE_NODEBUG uint64x2_t simd_cmp_ne_u64(const uint64x2_t& a, const uint64x2_t& b) noexcept { return simd_u64(vmvnq_u32(simd_u32(vceqq_u64(a, b)))); }

BL_INLINE_NODEBUG int64x2_t simd_cmp_gt_i64(const int64x2_t& a, const int64x2_t& b) noexcept { return simd_i64(vcgtq_s64(a, b)); }
BL_INLINE_NODEBUG uint64x2_t simd_cmp_gt_u64(const uint64x2_t& a, const uint64x2_t& b) noexcept { return simd_u64(vcgtq_u64(a, b)); }

BL_INLINE_NODEBUG int64x2_t simd_cmp_ge_i64(const int64x2_t& a, const int64x2_t& b) noexcept { return simd_i64(vcgeq_s64(a, b)); }
BL_INLINE_NODEBUG uint64x2_t simd_cmp_ge_u64(const uint64x2_t& a, const uint64x2_t& b) noexcept { return simd_u64(vcgeq_u64(a, b)); }

BL_INLINE_NODEBUG int64x2_t simd_cmp_lt_i64(const int64x2_t& a, const int64x2_t& b) noexcept { return simd_i64(vcltq_s64(a, b)); }
BL_INLINE_NODEBUG uint64x2_t simd_cmp_lt_u64(const uint64x2_t& a, const uint64x2_t& b) noexcept { return simd_u64(vcltq_u64(a, b)); }

BL_INLINE_NODEBUG int64x2_t simd_cmp_le_i64(const int64x2_t& a, const int64x2_t& b) noexcept { return simd_i64(vcleq_s64(a, b)); }
BL_INLINE_NODEBUG uint64x2_t simd_cmp_le_u64(const uint64x2_t& a, const uint64x2_t& b) noexcept { return simd_u64(vcleq_u64(a, b)); }
#else
BL_INLINE_NODEBUG uint64x2_t simd_test_nz_u64(const uint64x2_t& a) noexcept {
  return simd_u64(vshrq_n_s64(simd_i64(vqshlq_n_u64(a, 63)), 63));
}

BL_INLINE_NODEBUG uint64x2_t simd_test_z_u64(const uint64x2_t& a) noexcept {
  return simd_u64(simd_not(simd_u8(simd_test_nz_u64(a))));
}

BL_INLINE_NODEBUG int64x2_t simd_cmp_eq_i64(const int64x2_t& a, const int64x2_t& b) noexcept {
  uint32x4_t msk0 = vceqq_u32(simd_u32(a), simd_u32(b));
  uint32x4_t msk1 = vrev64q_u32(msk0);
  return simd_i64(vandq_u32(msk0, msk1));
}

BL_INLINE_NODEBUG int64x2_t simd_cmp_ne_i64(const int64x2_t& a, const int64x2_t& b) noexcept {
  uint64x2_t msk = simd_u64(simd_xor(simd_u8(a), simd_u8(b)));
  return simd_i64(simd_test_nz_u64(msk));
}

BL_INLINE_NODEBUG uint64x2_t simd_cmp_eq_u64(const uint64x2_t& a, const uint64x2_t& b) noexcept { return simd_u64(simd_cmp_eq_i64(simd_i64(a), simd_i64(b))); }
BL_INLINE_NODEBUG uint64x2_t simd_cmp_ne_u64(const uint64x2_t& a, const uint64x2_t& b) noexcept { return simd_u64(simd_cmp_ne_i64(simd_i64(a), simd_i64(b))); }

BL_INLINE_NODEBUG int64x2_t simd_cmp_gt_i64(const int64x2_t& a, const int64x2_t& b) noexcept {
  return vshrq_n_s64(vqsubq_s64(b, a), 63);
}

BL_INLINE_NODEBUG uint64x2_t simd_cmp_gt_u64(const uint64x2_t& a, const uint64x2_t& b) noexcept {
  return simd_test_nz_u64(vqsubq_u64(a, b));
}

BL_INLINE_NODEBUG int64x2_t simd_cmp_ge_i64(const int64x2_t& a, const int64x2_t& b) noexcept {
  int64x2_t one = simd_i64(simd_make128_u64(1u));
  return vshrq_n_s64(vqsubq_s64(vqsubq_s64(b, a), one), 63);
}

BL_INLINE_NODEBUG uint64x2_t simd_cmp_ge_u64(const uint64x2_t& a, const uint64x2_t& b) noexcept {
  return simd_test_z_u64(vqsubq_u64(b, a));
}

BL_INLINE_NODEBUG int64x2_t simd_cmp_lt_i64(const int64x2_t& a, const int64x2_t& b) noexcept { return simd_cmp_gt_i64(b, a); }
BL_INLINE_NODEBUG uint64x2_t simd_cmp_lt_u64(const uint64x2_t& a, const uint64x2_t& b) noexcept { return simd_cmp_gt_u64(b, a); }

BL_INLINE_NODEBUG int64x2_t simd_cmp_le_i64(const int64x2_t& a, const int64x2_t& b) noexcept { return simd_cmp_ge_i64(b, a); }
BL_INLINE_NODEBUG uint64x2_t simd_cmp_le_u64(const uint64x2_t& a, const uint64x2_t& b) noexcept { return simd_cmp_ge_u64(b, a); }
#endif

BL_INLINE_NODEBUG int8x16_t simd_min_i8(const int8x16_t& a, const int8x16_t& b) noexcept { return vminq_s8(a, b); }
BL_INLINE_NODEBUG int8x16_t simd_max_i8(const int8x16_t& a, const int8x16_t& b) noexcept { return vmaxq_s8(a, b); }

BL_INLINE_NODEBUG uint8x16_t simd_min_u8(const uint8x16_t& a, const uint8x16_t& b) noexcept { return vminq_u8(a, b); }
BL_INLINE_NODEBUG uint8x16_t simd_max_u8(const uint8x16_t& a, const uint8x16_t& b) noexcept { return vmaxq_u8(a, b); }

BL_INLINE_NODEBUG int16x8_t simd_min_i16(const int16x8_t& a, const int16x8_t& b) noexcept { return vminq_s16(a, b); }
BL_INLINE_NODEBUG int16x8_t simd_max_i16(const int16x8_t& a, const int16x8_t& b) noexcept { return vmaxq_s16(a, b); }

BL_INLINE_NODEBUG uint16x8_t simd_min_u16(const uint16x8_t& a, const uint16x8_t& b) noexcept { return vminq_u16(a, b); }
BL_INLINE_NODEBUG uint16x8_t simd_max_u16(const uint16x8_t& a, const uint16x8_t& b) noexcept { return vmaxq_u16(a, b); }

BL_INLINE_NODEBUG int32x4_t simd_min_i32(const int32x4_t& a, const int32x4_t& b) noexcept { return vminq_s32(a, b); }
BL_INLINE_NODEBUG int32x4_t simd_max_i32(const int32x4_t& a, const int32x4_t& b) noexcept { return vmaxq_s32(a, b); }

BL_INLINE_NODEBUG uint32x4_t simd_min_u32(const uint32x4_t& a, const uint32x4_t& b) noexcept { return vminq_u32(a, b); }
BL_INLINE_NODEBUG uint32x4_t simd_max_u32(const uint32x4_t& a, const uint32x4_t& b) noexcept { return vmaxq_u32(a, b); }

#if defined(BL_SIMD_AARCH64)
BL_INLINE_NODEBUG int64x2_t simd_min_i64(const int64x2_t& a, const int64x2_t& b) noexcept { return simd_blendv_bits(a, b, simd_cmp_gt_i64(a, b)); }
BL_INLINE_NODEBUG int64x2_t simd_max_i64(const int64x2_t& a, const int64x2_t& b) noexcept { return simd_blendv_bits(a, b, simd_cmp_lt_i64(a, b)); }
BL_INLINE_NODEBUG uint64x2_t simd_min_u64(const uint64x2_t& a, const uint64x2_t& b) noexcept { return simd_blendv_bits(a, b, simd_cmp_gt_u64(a, b)); }
BL_INLINE_NODEBUG uint64x2_t simd_max_u64(const uint64x2_t& a, const uint64x2_t& b) noexcept { return simd_blendv_bits(a, b, simd_cmp_lt_u64(a, b)); }
#else
BL_INLINE_NODEBUG int64x2_t simd_min_i64(const int64x2_t& a, const int64x2_t& b) noexcept { return simd_blendv_bits(a, b, vshrq_n_s64(vqsubq_s64(b, a), 63)); }
BL_INLINE_NODEBUG int64x2_t simd_max_i64(const int64x2_t& a, const int64x2_t& b) noexcept { return simd_blendv_bits(a, b, vshrq_n_s64(vqsubq_s64(a, b), 63)); }
BL_INLINE_NODEBUG uint64x2_t simd_min_u64(const uint64x2_t& a, const uint64x2_t& b) noexcept { return vsubq_u64(a, vqsubq_u64(a, b)); }
BL_INLINE_NODEBUG uint64x2_t simd_max_u64(const uint64x2_t& a, const uint64x2_t& b) noexcept { return vaddq_u64(b, vqsubq_u64(a, b)); }
#endif

BL_INLINE_NODEBUG int8x16_t simd_abs_i8(const int8x16_t& a) noexcept { return vabsq_s8(a); }
BL_INLINE_NODEBUG int16x8_t simd_abs_i16(const int16x8_t& a) noexcept { return vabsq_s16(a); }
BL_INLINE_NODEBUG int32x4_t simd_abs_i32(const int32x4_t& a) noexcept { return vabsq_s32(a); }

BL_INLINE_NODEBUG int64x2_t simd_abs_i64(const int64x2_t& a) noexcept {
#if defined(BL_SIMD_AARCH64)
  return vabsq_s64(a);
#else
  int64x2_t msk = vshrq_n_s64(a, 63);
  return vsubq_s64(veorq_s64(a, msk), msk);
#endif
}

template<uint8_t kN> BL_INLINE_NODEBUG int8x16_t simd_slli_i8(const int8x16_t& a) noexcept { return kN ? vshlq_n_s8(a, kN) : a; }
template<uint8_t kN> BL_INLINE_NODEBUG int16x8_t simd_slli_i16(const int16x8_t& a) noexcept { return kN ? vshlq_n_s16(a, kN) : a; }
template<uint8_t kN> BL_INLINE_NODEBUG int32x4_t simd_slli_i32(const int32x4_t& a) noexcept { return kN ? vshlq_n_s32(a, kN) : a; }
template<uint8_t kN> BL_INLINE_NODEBUG int64x2_t simd_slli_i64(const int64x2_t& a) noexcept { return kN ? vshlq_n_s64(a, kN) : a; }

template<uint8_t kN> BL_INLINE_NODEBUG uint8x16_t simd_slli_u8(const uint8x16_t& a) noexcept { return kN ? vshlq_n_u8(a, kN) : a; }
template<uint8_t kN> BL_INLINE_NODEBUG uint16x8_t simd_slli_u16(const uint16x8_t& a) noexcept { return kN ? vshlq_n_u16(a, kN) : a; }
template<uint8_t kN> BL_INLINE_NODEBUG uint32x4_t simd_slli_u32(const uint32x4_t& a) noexcept { return kN ? vshlq_n_u32(a, kN) : a; }
template<uint8_t kN> BL_INLINE_NODEBUG uint64x2_t simd_slli_u64(const uint64x2_t& a) noexcept { return kN ? vshlq_n_u64(a, kN) : a; }

template<uint8_t kN> BL_INLINE_NODEBUG uint8x16_t simd_srli_u8(const uint8x16_t& a) noexcept { return kN ? vshrq_n_u8(a, kN) : a; }
template<uint8_t kN> BL_INLINE_NODEBUG uint16x8_t simd_srli_u16(const uint16x8_t& a) noexcept { return kN ? vshrq_n_u16(a, kN) : a; }
template<uint8_t kN> BL_INLINE_NODEBUG uint32x4_t simd_srli_u32(const uint32x4_t& a) noexcept { return kN ? vshrq_n_u32(a, kN) : a; }
template<uint8_t kN> BL_INLINE_NODEBUG uint64x2_t simd_srli_u64(const uint64x2_t& a) noexcept { return kN ? vshrq_n_u64(a, kN) : a; }

template<uint8_t kN> BL_INLINE_NODEBUG uint8x16_t simd_rsrli_u8(const uint8x16_t& a) noexcept { return vrshrq_n_u8(a, kN); }
template<uint8_t kN> BL_INLINE_NODEBUG uint16x8_t simd_rsrli_u16(const uint16x8_t& a) noexcept { return vrshrq_n_u16(a, kN); }
template<uint8_t kN> BL_INLINE_NODEBUG uint32x4_t simd_rsrli_u32(const uint32x4_t& a) noexcept { return vrshrq_n_u32(a, kN); }
template<uint8_t kN> BL_INLINE_NODEBUG uint64x2_t simd_rsrli_u64(const uint64x2_t& a) noexcept { return vrshrq_n_u64(a, kN); }

template<uint8_t kN> BL_INLINE_NODEBUG uint8x16_t simd_acc_rsrli_u8(const uint8x16_t& a, const uint8x16_t& b) noexcept { return vrsraq_n_u8(a, b, kN); }
template<uint8_t kN> BL_INLINE_NODEBUG uint16x8_t simd_acc_rsrli_u16(const uint16x8_t& a, const uint16x8_t& b) noexcept { return vrsraq_n_u16(a, b, kN); }
template<uint8_t kN> BL_INLINE_NODEBUG uint32x4_t simd_acc_rsrli_u32(const uint32x4_t& a, const uint32x4_t& b) noexcept { return vrsraq_n_u32(a, b, kN); }
template<uint8_t kN> BL_INLINE_NODEBUG uint64x2_t simd_acc_rsrli_u64(const uint64x2_t& a, const uint64x2_t& b) noexcept { return vrsraq_n_u64(a, b, kN); }

template<uint8_t kN> BL_INLINE_NODEBUG int8x16_t simd_srai_i8(const int8x16_t& a) noexcept { return kN ? vshrq_n_s8(a, kN) : a; }
template<uint8_t kN> BL_INLINE_NODEBUG int16x8_t simd_srai_i16(const int16x8_t& a) noexcept { return kN ? vshrq_n_s16(a, kN) : a; }
template<uint8_t kN> BL_INLINE_NODEBUG int32x4_t simd_srai_i32(const int32x4_t& a) noexcept { return kN ? vshrq_n_s32(a, kN) : a; }
template<uint8_t kN> BL_INLINE_NODEBUG int64x2_t simd_srai_i64(const int64x2_t& a) noexcept { return kN ? vshrq_n_s64(a, kN) : a; }

template<uint8_t kN> BL_INLINE_NODEBUG uint8x16_t simd_sllb_u128(const uint8x16_t& a) noexcept { return !(kN & 15u) ? a : vextq_u8(vdupq_n_u8(0), a, (16 - kN) & 15u); }
template<uint8_t kN> BL_INLINE_NODEBUG uint8x16_t simd_srlb_u128(const uint8x16_t& a) noexcept { return !(kN & 15u) ? a : vextq_u8(a, vdupq_n_u8(0), kN & 15u); }

#if defined(BL_TARGET_OPT_ASIMD_CRYPTO)
BL_INLINE_NODEBUG uint64x2_t simd_clmul_u128_ll(const uint64x2_t& a, const uint64x2_t& b) noexcept {
  return vreinterpretq_u64_p128(
    vmull_p64((poly64_t)vreinterpret_p64_u64(vget_low_u64(a)),
              (poly64_t)vreinterpret_p64_u64(vget_low_u64(b)))
  );
}

BL_INLINE_NODEBUG uint64x2_t simd_clmul_u128_lh(const uint64x2_t& a, const uint64x2_t& b) noexcept {
  return vreinterpretq_u64_p128(
    vmull_p64((poly64_t)vreinterpret_p64_u64(vget_low_u64(a)),
              (poly64_t)vreinterpret_p64_u64(vget_high_u64(b)))
  );
}

BL_INLINE_NODEBUG uint64x2_t simd_clmul_u128_hl(const uint64x2_t& a, const uint64x2_t& b) noexcept {
  return vreinterpretq_u64_p128(
    vmull_p64((poly64_t)vreinterpret_p64_u64(vget_high_u64(a)),
              (poly64_t)vreinterpret_p64_u64(vget_low_u64(b)))
  );
}

BL_INLINE_NODEBUG uint64x2_t simd_clmul_u128_hh(const uint64x2_t& a, const uint64x2_t& b) noexcept {
  return vreinterpretq_u64_p128(
    vmull_p64((poly64_t)vreinterpret_p64_u64(vget_high_u64(a)),
              (poly64_t)vreinterpret_p64_u64(vget_high_u64(b)))
  );
}
#endif // BL_TARGET_OPT_ASIMD_CRYPTO

} // {Internal}

// SIMD - Internal - Load & Store Operations
// =========================================

namespace Internal {

template<size_t kW> BL_INLINE_NODEBUG typename SimdInfo<kW>::RegU8 simd_load_broadcast_8(const void* src) noexcept;
template<size_t kW> BL_INLINE_NODEBUG typename SimdInfo<kW>::RegU16 simd_loada_broadcast_16(const void* src) noexcept;
template<size_t kW> BL_INLINE_NODEBUG typename SimdInfo<kW>::RegU32 simd_loada_broadcast_32(const void* src) noexcept;
template<size_t kW> BL_INLINE_NODEBUG typename SimdInfo<kW>::RegU64 simd_loada_broadcast_64(const void* src) noexcept;

template<size_t kW> BL_INLINE_NODEBUG typename SimdInfo<kW>::RegU8 simd_load_8(const void* src) noexcept;
template<size_t kW> BL_INLINE_NODEBUG typename SimdInfo<kW>::RegU8 simd_loada_16(const void* src) noexcept;
template<size_t kW> BL_INLINE_NODEBUG typename SimdInfo<kW>::RegU8 simd_loadu_16(const void* src) noexcept;
template<size_t kW> BL_INLINE_NODEBUG typename SimdInfo<kW>::RegU8 simd_loada_32(const void* src) noexcept;
template<size_t kW> BL_INLINE_NODEBUG typename SimdInfo<kW>::RegU8 simd_loadu_32(const void* src) noexcept;
template<size_t kW> BL_INLINE_NODEBUG typename SimdInfo<kW>::RegU8 simd_loada_64(const void* src) noexcept;
template<size_t kW> BL_INLINE_NODEBUG typename SimdInfo<kW>::RegU8 simd_loadu_64(const void* src) noexcept;

template<size_t kW> BL_INLINE_NODEBUG typename SimdInfo<kW>::RegU8 simd_loada(const void* src) noexcept;
template<size_t kW> BL_INLINE_NODEBUG typename SimdInfo<kW>::RegU8 simd_loadu(const void* src) noexcept;

template<> BL_INLINE_NODEBUG uint8x8_t simd_load_broadcast_8<8>(const void* src) noexcept { return vld1_dup_u8(static_cast<const uint8_t*>(src)); }
template<> BL_INLINE_NODEBUG uint16x4_t simd_loada_broadcast_16<8>(const void* src) noexcept { return vld1_dup_u16(static_cast<const uint16_t*>(src)); }
template<> BL_INLINE_NODEBUG uint32x2_t simd_loada_broadcast_32<8>(const void* src) noexcept { return vld1_dup_u32(static_cast<const uint32_t*>(src)); }
template<> BL_INLINE_NODEBUG uint64x1_t simd_loada_broadcast_64<8>(const void* src) noexcept { return vld1_u64(static_cast<const uint64_t*>(src)); }

template<> BL_INLINE_NODEBUG uint8x16_t simd_load_broadcast_8<16>(const void* src) noexcept { return vld1q_dup_u8(static_cast<const uint8_t*>(src)); }
template<> BL_INLINE_NODEBUG uint16x8_t simd_loada_broadcast_16<16>(const void* src) noexcept { return vld1q_dup_u16(static_cast<const uint16_t*>(src)); }
template<> BL_INLINE_NODEBUG uint32x4_t simd_loada_broadcast_32<16>(const void* src) noexcept { return vld1q_dup_u32(static_cast<const uint32_t*>(src)); }
template<> BL_INLINE_NODEBUG uint64x2_t simd_loada_broadcast_64<16>(const void* src) noexcept { return vld1q_dup_u64(static_cast<const uint64_t*>(src)); }

template<> BL_INLINE_NODEBUG uint8x8_t simd_load_8<8>(const void* src) noexcept { return simd_u8(vld1_lane_u8(static_cast<const uint8_t*>(src), vdup_n_u8(0), 0)); }
template<> BL_INLINE_NODEBUG uint8x8_t simd_loada_16<8>(const void* src) noexcept { return simd_u8(vld1_lane_u16(static_cast<const uint16_t*>(src), vdup_n_u16(0), 0)); }
template<> BL_INLINE_NODEBUG uint8x8_t simd_loada_32<8>(const void* src) noexcept { return simd_u8(vld1_lane_u32(static_cast<const uint32_t*>(src), vdup_n_u32(0), 0)); }
template<> BL_INLINE_NODEBUG uint8x8_t simd_loada_64<8>(const void* src) noexcept { return simd_u8(vld1_u64(static_cast<const uint64_t*>(src))); }

template<> BL_INLINE_NODEBUG uint8x8_t simd_loadu_16<8>(const void* src) noexcept { return simd_u8(vset_lane_u16(bl::MemOps::readU16u(src), vdup_n_u16(0), 0)); }
template<> BL_INLINE_NODEBUG uint8x8_t simd_loadu_32<8>(const void* src) noexcept { return simd_u8(vset_lane_u32(bl::MemOps::readU32u(src), vdup_n_u32(0), 0)); }
template<> BL_INLINE_NODEBUG uint8x8_t simd_loadu_64<8>(const void* src) noexcept { return simd_u8(vld1_u8(static_cast<const uint8_t*>(src))); }

template<> BL_INLINE_NODEBUG uint8x8_t simd_loada<8>(const void* src) noexcept { return simd_u8(vld1_u64(static_cast<const uint64_t*>(src))); }
template<> BL_INLINE_NODEBUG uint8x8_t simd_loadu<8>(const void* src) noexcept { return simd_u8(vld1_u8(static_cast<const uint8_t*>(src))); }

template<> BL_INLINE_NODEBUG uint8x16_t simd_load_8<16>(const void* src) noexcept { return simd_u8(vld1q_lane_u8(static_cast<const uint8_t*>(src), vdupq_n_u8(0), 0)); }
template<> BL_INLINE_NODEBUG uint8x16_t simd_loada_16<16>(const void* src) noexcept { return simd_u8(vld1q_lane_u16(static_cast<const uint16_t*>(src), vdupq_n_u16(0), 0)); }
template<> BL_INLINE_NODEBUG uint8x16_t simd_loada_32<16>(const void* src) noexcept { return simd_u8(vld1q_lane_u32(static_cast<const uint32_t*>(src), vdupq_n_u32(0), 0)); }
template<> BL_INLINE_NODEBUG uint8x16_t simd_loada_64<16>(const void* src) noexcept { return simd_u8(vld1q_lane_u64(static_cast<const uint64_t*>(src), vdupq_n_u64(0), 0)); }

template<> BL_INLINE_NODEBUG uint8x16_t simd_loadu_16<16>(const void* src) noexcept { return simd_u8(vsetq_lane_u16(bl::MemOps::readU16u(src), vdupq_n_u16(0), 0)); }
template<> BL_INLINE_NODEBUG uint8x16_t simd_loadu_32<16>(const void* src) noexcept { return simd_u8(vsetq_lane_u32(bl::MemOps::readU32u(src), vdupq_n_u32(0), 0)); }
template<> BL_INLINE_NODEBUG uint8x16_t simd_loadu_64<16>(const void* src) noexcept { return simd_u8(vsetq_lane_u64(bl::MemOps::readU64u(src), vdupq_n_u64(0), 0)); }

BL_INLINE_NODEBUG uint8x16_t simd_loada_128(const void* src) noexcept { return simd_u8(vld1q_u64(static_cast<const uint64_t*>(src))); }
BL_INLINE_NODEBUG uint8x16_t simd_loadu_128(const void* src) noexcept { return simd_u8(vld1q_u8(static_cast<const uint8_t*>(src))); }

template<> BL_INLINE_NODEBUG uint8x16_t simd_loada<16>(const void* src) noexcept { return simd_u8(vld1q_u64(static_cast<const uint64_t*>(src))); }
template<> BL_INLINE_NODEBUG uint8x16_t simd_loadu<16>(const void* src) noexcept { return simd_u8(vld1q_u8(static_cast<const uint8_t*>(src))); }

BL_INLINE_NODEBUG void simd_store_8(void* dst, uint8x8_t src) noexcept { vst1_lane_u8(static_cast<uint8_t*>(dst), src, 0); }
BL_INLINE_NODEBUG void simd_store_8(void* dst, uint8x16_t src) noexcept { vst1q_lane_u8(static_cast<uint8_t*>(dst), src, 0); }

BL_INLINE_NODEBUG void simd_storea_16(void* dst, uint8x8_t src) noexcept { vst1_lane_u16(static_cast<uint16_t*>(dst), simd_u16(src), 0); }
BL_INLINE_NODEBUG void simd_storea_16(void* dst, uint8x16_t src) noexcept { vst1q_lane_u16(static_cast<uint16_t*>(dst), simd_u16(src), 0); }

BL_INLINE_NODEBUG void simd_storeu_16(void* dst, uint8x8_t src) noexcept { bl::MemOps::writeU16u(dst, vget_lane_u16(simd_u16(src), 0)); }
BL_INLINE_NODEBUG void simd_storeu_16(void* dst, uint8x16_t src) noexcept { bl::MemOps::writeU16u(dst, vgetq_lane_u16(simd_u16(src), 0)); }

BL_INLINE_NODEBUG void simd_storea_32(void* dst, uint8x8_t src) noexcept { vst1_lane_u32(static_cast<uint32_t*>(dst), simd_u32(src), 0); }
BL_INLINE_NODEBUG void simd_storea_32(void* dst, uint8x16_t src) noexcept { vst1q_lane_u32(static_cast<uint32_t*>(dst), simd_u32(src), 0); }

BL_INLINE_NODEBUG void simd_storeu_32(void* dst, uint8x8_t src) noexcept { bl::MemOps::writeU32u(dst, vget_lane_u32(simd_u32(src), 0)); }
BL_INLINE_NODEBUG void simd_storeu_32(void* dst, uint8x16_t src) noexcept { bl::MemOps::writeU32u(dst, vgetq_lane_u32(simd_u32(src), 0)); }

BL_INLINE_NODEBUG void simd_storea_64(void* dst, uint8x8_t src) noexcept { vst1_u64(static_cast<uint64_t*>(dst), simd_u64(src)); }
BL_INLINE_NODEBUG void simd_storea_64(void* dst, uint8x16_t src) noexcept { vst1q_lane_u64(static_cast<uint64_t*>(dst), simd_u64(src), 0); }

BL_INLINE_NODEBUG void simd_storeu_64(void* dst, uint8x8_t src) noexcept { vst1_u8(static_cast<uint8_t*>(dst), src); }
BL_INLINE_NODEBUG void simd_storeu_64(void* dst, uint8x16_t src) noexcept { vst1_u8(static_cast<uint8_t*>(dst), vget_low_u8(src)); }

BL_INLINE_NODEBUG void simd_storeh_64(void* dst, uint8x16_t src) noexcept { vst1_u8(static_cast<uint8_t*>(dst), vget_high_u8(src)); }

BL_INLINE_NODEBUG void simd_storea_128(void* dst, uint8x16_t src) noexcept { vst1q_u64(static_cast<uint64_t*>(dst), simd_u64(src)); }
BL_INLINE_NODEBUG void simd_storeu_128(void* dst, uint8x16_t src) noexcept { vst1q_u8(static_cast<uint8_t*>(dst), src); }

BL_INLINE_NODEBUG void simd_storea(void* dst, uint8x8_t src) noexcept { simd_storea_64(dst, src); }
BL_INLINE_NODEBUG void simd_storeu(void* dst, uint8x8_t src) noexcept { simd_storeu_64(dst, src); }

BL_INLINE_NODEBUG void simd_storea(void* dst, uint8x16_t src) noexcept { simd_storea_128(dst, src); }
BL_INLINE_NODEBUG void simd_storeu(void* dst, uint8x16_t src) noexcept { simd_storeu_128(dst, src); }

} // {Internal}

// SIMD - Public - Make Zero & Ones & Undefined
// ============================================

template<typename V> BL_INLINE_NODEBUG V make_zero() noexcept { return V{I::simd_make_zero<typename V::SimdType>()}; }
template<typename V> BL_INLINE_NODEBUG V make_ones() noexcept { return V{I::simd_make_ones<typename V::SimdType>()}; }
template<typename V> BL_INLINE_NODEBUG V make_undefined() noexcept { return V{I::simd_make_undefined<typename V::SimdType>()}; }

// SIMD - Public - Make Vector (128-bit)
// =====================================

template<typename V = Vec16xI8>
BL_INLINE_NODEBUG V make128_i8(int8_t x0) noexcept {
  return from_simd<V>(I::simd_make128_u8(uint8_t(x0)));
}

template<typename V = Vec16xI8>
BL_INLINE_NODEBUG V make128_i8(int8_t x1, int8_t x0) noexcept {
  return from_simd<V>(I::simd_make128_u8(uint8_t(x1), uint8_t(x0)));
}

template<typename V = Vec16xI8>
BL_INLINE_NODEBUG V make128_i8(int8_t x3, int8_t x2, int8_t x1, int8_t x0) noexcept {
  return from_simd<V>(I::simd_make128_u8(uint8_t(x3), uint8_t(x2), uint8_t(x1), uint8_t(x0)));
}

template<typename V = Vec16xI8>
BL_INLINE_NODEBUG V make128_i8(int8_t x7, int8_t x6, int8_t x5, int8_t x4,
                               int8_t x3, int8_t x2, int8_t x1, int8_t x0) noexcept {
  return from_simd<V>(
    I::simd_make128_u8(
      uint8_t(x7), uint8_t(x6), uint8_t(x5), uint8_t(x4),
      uint8_t(x3), uint8_t(x2), uint8_t(x1), uint8_t(x0)));
}

template<typename V = Vec16xI8>
BL_INLINE_NODEBUG V make128_i8(int8_t x15, int8_t x14, int8_t x13, int8_t x12,
                               int8_t x11, int8_t x10, int8_t x09, int8_t x08,
                               int8_t x07, int8_t x06, int8_t x05, int8_t x04,
                               int8_t x03, int8_t x02, int8_t x01, int8_t x00) noexcept {
  return from_simd<V>(
    I::simd_make128_u8(
      uint8_t(x15), uint8_t(x14), uint8_t(x13), uint8_t(x12),
      uint8_t(x11), uint8_t(x10), uint8_t(x09), uint8_t(x08),
      uint8_t(x07), uint8_t(x06), uint8_t(x05), uint8_t(x04),
      uint8_t(x03), uint8_t(x02), uint8_t(x01), uint8_t(x00)));
}

template<typename V = Vec16xU8>
BL_INLINE_NODEBUG V make128_u8(uint8_t x0) noexcept {
  return from_simd<V>(I::simd_make128_u8(x0));
}

template<typename V = Vec16xU8>
BL_INLINE_NODEBUG V make128_u8(uint8_t x1, uint8_t x0) noexcept {
  return from_simd<V>(I::simd_make128_u8(x1, x0));
}

template<typename V = Vec16xU8>
BL_INLINE_NODEBUG V make128_u8(uint8_t x3, uint8_t x2, uint8_t x1, uint8_t x0) noexcept {
  return from_simd<V>(I::simd_make128_u8(x3, x2, x1, x0));
}

template<typename V = Vec16xU8>
BL_INLINE_NODEBUG V make128_u8(uint8_t x7, uint8_t x6, uint8_t x5, uint8_t x4,
                               uint8_t x3, uint8_t x2, uint8_t x1, uint8_t x0) noexcept {
  return from_simd<V>(
    I::simd_make128_u8(
      x7, x6, x5, x4,
      x3, x2, x1, x0));
}

template<typename V = Vec16xU8>
BL_INLINE_NODEBUG V make128_u8(uint8_t x15, uint8_t x14, uint8_t x13, uint8_t x12,
                               uint8_t x11, uint8_t x10, uint8_t x09, uint8_t x08,
                               uint8_t x07, uint8_t x06, uint8_t x05, uint8_t x04,
                               uint8_t x03, uint8_t x02, uint8_t x01, uint8_t x00) noexcept {
  return from_simd<V>(
    I::simd_make128_u8(
      x15, x14, x13, x12,
      x11, x10, x09, x08,
      x07, x06, x05, x04,
      x03, x02, x01, x00));
}

template<typename V = Vec8xI16>
BL_INLINE_NODEBUG V make128_i16(int16_t x0) noexcept {
  return from_simd<V>(I::simd_make128_u16(uint16_t(x0)));
}

template<typename V = Vec8xI16>
BL_INLINE_NODEBUG V make128_i16(int16_t x1, int16_t x0) noexcept {
  return from_simd<V>(I::simd_make128_u16(uint16_t(x1), uint16_t(x0)));
}

template<typename V = Vec8xI16>
BL_INLINE_NODEBUG V make128_i16(int16_t x3, int16_t x2, int16_t x1, int16_t x0) noexcept {
  return from_simd<V>(I::simd_make128_u16(uint16_t(x3), uint16_t(x2), uint16_t(x1), uint16_t(x0)));
}

template<typename V = Vec8xI16>
BL_INLINE_NODEBUG V make128_i16(int16_t x7, int16_t x6, int16_t x5, int16_t x4,
                                int16_t x3, int16_t x2, int16_t x1, int16_t x0) noexcept {
  return from_simd<V>(
    I::simd_make128_u16(
      uint16_t(x7), uint16_t(x6), uint16_t(x5), uint16_t(x4),
      uint16_t(x3), uint16_t(x2), uint16_t(x1), uint16_t(x0)));
}

template<typename V = Vec8xU16>
BL_INLINE_NODEBUG V make128_u16(uint16_t x0) noexcept {
  return from_simd<V>(I::simd_make128_u16(x0));
}

template<typename V = Vec8xU16>
BL_INLINE_NODEBUG V make128_u16(uint16_t x1, uint16_t x0) noexcept {
  return from_simd<V>(I::simd_make128_u16(x1, x0));
}

template<typename V = Vec8xU16>
BL_INLINE_NODEBUG V make128_u16(uint16_t x3, uint16_t x2, uint16_t x1, uint16_t x0) noexcept {
  return from_simd<V>(I::simd_make128_u16(x3, x2, x1, x0));
}

template<typename V = Vec8xU16>
BL_INLINE_NODEBUG V make128_u16(uint16_t x7, uint16_t x6, uint16_t x5, uint16_t x4,
                                uint16_t x3, uint16_t x2, uint16_t x1, uint16_t x0) noexcept {
  return from_simd<V>(
    I::simd_make128_u16(
      x7, x6, x5, x4,
      x3, x2, x1, x0));
}

template<typename V = Vec4xI32>
BL_INLINE_NODEBUG V make128_i32(int32_t x0) noexcept {
  return from_simd<V>(I::simd_make128_u32(uint32_t(x0)));
}

template<typename V = Vec4xI32>
BL_INLINE_NODEBUG V make128_i32(int32_t x1, int32_t x0) noexcept {
  return from_simd<V>(I::simd_make128_u32(uint32_t(x1), uint32_t(x0)));
}

template<typename V = Vec4xI32>
BL_INLINE_NODEBUG V make128_i32(int32_t x3, int32_t x2, int32_t x1, int32_t x0) noexcept {
  return from_simd<V>(I::simd_make128_u32(uint32_t(x3), uint32_t(x2), uint32_t(x1), uint32_t(x0)));
}

template<typename V = Vec4xU32>
BL_INLINE_NODEBUG V make128_u32(uint32_t x0) noexcept {
  return from_simd<V>(I::simd_make128_u32(x0));
}

template<typename V = Vec4xU32>
BL_INLINE_NODEBUG V make128_u32(uint32_t x1, uint32_t x0) noexcept {
  return from_simd<V>(I::simd_make128_u32(x1, x0));
}

template<typename V = Vec4xU32>
BL_INLINE_NODEBUG V make128_u32(uint32_t x3, uint32_t x2, uint32_t x1, uint32_t x0) noexcept {
  return from_simd<V>(I::simd_make128_u32(x3, x2, x1, x0));
}

template<typename V = Vec2xI64>
BL_INLINE_NODEBUG V make128_i64(int64_t x0) noexcept {
  return from_simd<V>(I::simd_make128_u64(uint64_t(x0)));
}

template<typename V = Vec2xI64>
BL_INLINE_NODEBUG V make128_i64(int64_t x1, int64_t x0) noexcept {
  return from_simd<V>(I::simd_make128_u64(uint64_t(x1), uint64_t(x0)));
}

template<typename V = Vec2xU64>
BL_INLINE_NODEBUG V make128_u64(uint64_t x0) noexcept {
  return from_simd<V>(I::simd_make128_u64(x0));
}

template<typename V = Vec2xU64>
BL_INLINE_NODEBUG V make128_u64(uint64_t x1, uint64_t x0) noexcept {
  return from_simd<V>(I::simd_make128_u64(x1, x0));
}

template<typename V = Vec4xF32>
BL_INLINE_NODEBUG V make128_f32(float x0) noexcept {
  return from_simd<V>(I::simd_make128_f32(x0));
}

template<typename V = Vec4xF32>
BL_INLINE_NODEBUG V make128_f32(float x1, float x0) noexcept {
  return from_simd<V>(I::simd_make128_f32(x1, x0));
}

template<typename V = Vec4xF32>
BL_INLINE_NODEBUG V make128_f32(float x3, float x2, float x1, float x0) noexcept {
  return from_simd<V>(I::simd_make128_f32(x3, x2, x1, x0));
}

#if defined(BL_SIMD_AARCH64)
template<typename V = Vec2xF64>
BL_INLINE_NODEBUG V make128_f64(double x0) noexcept {
  return from_simd<V>(I::simd_make128_f64(x0));
}

template<typename V = Vec2xF64>
BL_INLINE_NODEBUG V make128_f64(double x1, double x0) noexcept {
  return from_simd<V>(I::simd_make128_f64(x1, x0));
}
#endif // BL_SIMD_AARCH64

// SIMD - Public - Cast Vector <-> Scalar
// ======================================

template<typename V = Vec4xI32> BL_INLINE_NODEBUG V cast_from_i32(int32_t val) noexcept { return from_simd<V>(I::simd_from_u32(uint32_t(val))); }
template<typename V = Vec4xU32> BL_INLINE_NODEBUG V cast_from_u32(uint32_t val) noexcept { return from_simd<V>(I::simd_from_u32(val)); }
template<typename V = Vec2xI64> BL_INLINE_NODEBUG V cast_from_i64(int64_t val) noexcept { return from_simd<V>(I::simd_from_u64(uint64_t(val))); }
template<typename V = Vec2xU64> BL_INLINE_NODEBUG V cast_from_u64(uint64_t val) noexcept { return from_simd<V>(I::simd_from_u64(val)); }
template<typename V = Vec4xF32> BL_INLINE_NODEBUG V cast_from_f32(float val) noexcept { return from_simd<V>(I::simd_from_f32(val)); }

#if defined(BL_SIMD_AARCH64)
template<typename V = Vec2xF64> BL_INLINE_NODEBUG V cast_from_f64(double val) noexcept { return from_simd<V>(I::simd_from_f64(val)); }
#endif // BL_SIMD_AARCH64

template<typename V> BL_INLINE_NODEBUG int32_t cast_to_i32(const V& src) noexcept { return int32_t(I::simd_cast_to_u32(simd_u32(src.v))); }
template<typename V> BL_INLINE_NODEBUG uint32_t cast_to_u32(const V& src) noexcept { return I::simd_cast_to_u32(simd_u32(src.v)); }
template<typename V> BL_INLINE_NODEBUG int64_t cast_to_i64(const V& src) noexcept { return int64_t(I::simd_cast_to_u64(simd_u64(src.v))); }
template<typename V> BL_INLINE_NODEBUG uint64_t cast_to_u64(const V& src) noexcept { return I::simd_cast_to_u64(simd_u64(src.v)); }
template<typename V> BL_INLINE_NODEBUG float cast_to_f32(const V& src) noexcept { return I::simd_cast_to_f32(simd_f32(src.v)); }

#if defined(BL_SIMD_AARCH64)
template<typename V> BL_INLINE_NODEBUG double cast_to_f64(const V& src) noexcept { return I::simd_cast_to_f64(simd_f64(src.v)); }
#endif // BL_SIMD_AARCH64

// SIMD - Public - Convert Vector <-> Vector
// =========================================

template<typename V> BL_INLINE_NODEBUG Vec<V::kW, float> cvt_i32_f32(const V& a) noexcept { return vec_wt<V::kW, float>(I::simd_cvt_i32_f32(simd_i32(a.v))); }
template<typename V> BL_INLINE_NODEBUG Vec<V::kW, int32_t> cvt_f32_i32(const V& a) noexcept { return vec_wt<V::kW, int32_t>(I::simd_cvt_f32_i32(simd_f32(a.v))); }
template<typename V> BL_INLINE_NODEBUG Vec<V::kW, int32_t> cvtt_f32_i32(const V& a) noexcept { return vec_wt<V::kW, int32_t>(I::simd_cvtt_f32_i32(simd_f32(a.v))); }

// SIMD - Public - Convert Vector <-> Scalar
// =========================================

BL_INLINE_NODEBUG Vec4xF32 cvt_f32_from_scalar_i32(int32_t val) noexcept { return Vec4xF32{I::simd_cvt_f32_from_scalar_i32(val)}; }

#if defined(BL_SIMD_AARCH64)
BL_INLINE_NODEBUG Vec2xF64 cvt_f64_from_scalar_i32(int32_t val) noexcept { return Vec2xF64{I::simd_cvt_f64_from_scalar_i32(val)}; }
#endif // BL_SIMD_AARCH64

template<typename V> BL_INLINE_NODEBUG int32_t cvt_f32_to_scalar_i32(const V& src) noexcept { return I::simd_cvt_f32_to_scalar_i32(simd_f32(src.v)); }
template<typename V> BL_INLINE_NODEBUG int32_t cvtt_f32_to_scalar_i32(const V& src) noexcept { return I::simd_cvtt_f32_to_scalar_i32(simd_f32(src.v)); }

// SIMD - Public - Load & Store Operations
// =======================================

template<typename V> BL_INLINE_NODEBUG V loada(const void* src) noexcept { return from_simd<V>(I::simd_loada<V::kW>(src)); }
template<typename V> BL_INLINE_NODEBUG V loadu(const void* src) noexcept { return from_simd<V>(I::simd_loadu<V::kW>(src)); }
template<typename V> BL_INLINE_NODEBUG V load_8(const void* src) noexcept { return from_simd<V>(I::simd_load_8<V::kW>(src)); }
template<typename V> BL_INLINE_NODEBUG V loada_16(const void* src) noexcept { return from_simd<V>(I::simd_loada_16<V::kW>(src)); }
template<typename V> BL_INLINE_NODEBUG V loadu_16(const void* src) noexcept { return from_simd<V>(I::simd_loadu_16<V::kW>(src)); }
template<typename V> BL_INLINE_NODEBUG V loada_32(const void* src) noexcept { return from_simd<V>(I::simd_loada_32<V::kW>(src)); }
template<typename V> BL_INLINE_NODEBUG V loadu_32(const void* src) noexcept { return from_simd<V>(I::simd_loadu_32<V::kW>(src)); }
template<typename V> BL_INLINE_NODEBUG V loada_64(const void* src) noexcept { return from_simd<V>(I::simd_loada_64<V::kW>(src)); }
template<typename V> BL_INLINE_NODEBUG V loadu_64(const void* src) noexcept { return from_simd<V>(I::simd_loadu_64<V::kW>(src)); }
template<typename V> BL_INLINE_NODEBUG V loada_128(const void* src) noexcept { return from_simd<V>(I::simd_loada_128(src)); }
template<typename V> BL_INLINE_NODEBUG V loadu_128(const void* src) noexcept { return from_simd<V>(I::simd_loadu_128(src)); }

template<typename V> BL_INLINE_NODEBUG V loada_64_i8_i16(const void* src) noexcept { return from_simd<V>(I::simd_unpack_lo64_i8_i16(simd_i8(I::simd_loada_64<V::kW>(src)))); }
template<typename V> BL_INLINE_NODEBUG V loadu_64_i8_i16(const void* src) noexcept { return from_simd<V>(I::simd_unpack_lo64_i8_i16(simd_i8(I::simd_loadu_64<V::kW>(src)))); }
template<typename V> BL_INLINE_NODEBUG V loada_64_u8_u16(const void* src) noexcept { return from_simd<V>(I::simd_unpack_lo64_u8_u16(simd_u8(I::simd_loada_64<V::kW>(src)))); }
template<typename V> BL_INLINE_NODEBUG V loadu_64_u8_u16(const void* src) noexcept { return from_simd<V>(I::simd_unpack_lo64_u8_u16(simd_u8(I::simd_loadu_64<V::kW>(src)))); }
template<typename V> BL_INLINE_NODEBUG V loada_64_i16_i32(const void* src) noexcept { return from_simd<V>(I::simd_unpack_lo64_i16_i32(simd_i16(I::simd_loada_64<V::kW>(src)))); }
template<typename V> BL_INLINE_NODEBUG V loadu_64_i16_i32(const void* src) noexcept { return from_simd<V>(I::simd_unpack_lo64_i16_i32(simd_i16(I::simd_loadu_64<V::kW>(src)))); }
template<typename V> BL_INLINE_NODEBUG V loada_64_u16_u32(const void* src) noexcept { return from_simd<V>(I::simd_unpack_lo64_u16_u32(simd_u16(I::simd_loada_64<V::kW>(src)))); }
template<typename V> BL_INLINE_NODEBUG V loadu_64_u16_u32(const void* src) noexcept { return from_simd<V>(I::simd_unpack_lo64_u16_u32(simd_u16(I::simd_loadu_64<V::kW>(src)))); }
template<typename V> BL_INLINE_NODEBUG V loada_64_i32_i64(const void* src) noexcept { return from_simd<V>(I::simd_unpack_lo64_i32_i64(simd_i32(I::simd_loada_64<V::kW>(src)))); }
template<typename V> BL_INLINE_NODEBUG V loadu_64_i32_i64(const void* src) noexcept { return from_simd<V>(I::simd_unpack_lo64_i32_i64(simd_i32(I::simd_loadu_64<V::kW>(src)))); }
template<typename V> BL_INLINE_NODEBUG V loada_64_u32_u64(const void* src) noexcept { return from_simd<V>(I::simd_unpack_lo64_u32_u64(simd_u32(I::simd_loada_64<V::kW>(src)))); }
template<typename V> BL_INLINE_NODEBUG V loadu_64_u32_u64(const void* src) noexcept { return from_simd<V>(I::simd_unpack_lo64_u32_u64(simd_u32(I::simd_loadu_64<V::kW>(src)))); }

template<typename V> BL_INLINE_NODEBUG void storea(void* dst, const V& src) noexcept { I::simd_storea(dst, simd_u8(src.v)); }
template<typename V> BL_INLINE_NODEBUG void storeu(void* dst, const V& src) noexcept { I::simd_storeu(dst, simd_u8(src.v)); }
template<typename V> BL_INLINE_NODEBUG void store_8(void* dst, const V& src) noexcept { I::simd_store_8(dst, simd_u8(src.v)); }
template<typename V> BL_INLINE_NODEBUG void storea_16(void* dst, const V& src) noexcept { I::simd_storea_16(dst, simd_u8(src.v)); }
template<typename V> BL_INLINE_NODEBUG void storeu_16(void* dst, const V& src) noexcept { I::simd_storeu_16(dst, simd_u8(src.v)); }
template<typename V> BL_INLINE_NODEBUG void storea_32(void* dst, const V& src) noexcept { I::simd_storea_32(dst, simd_u8(src.v)); }
template<typename V> BL_INLINE_NODEBUG void storeu_32(void* dst, const V& src) noexcept { I::simd_storeu_32(dst, simd_u8(src.v)); }
template<typename V> BL_INLINE_NODEBUG void storea_64(void* dst, const V& src) noexcept { I::simd_storea_64(dst, simd_u8(src.v)); }
template<typename V> BL_INLINE_NODEBUG void storeu_64(void* dst, const V& src) noexcept { I::simd_storeu_64(dst, simd_u8(src.v)); }
template<typename V> BL_INLINE_NODEBUG void storeh_64(void* dst, const V& src) noexcept { I::simd_storeh_64(dst, simd_u8(src.v)); }
template<typename V> BL_INLINE_NODEBUG void storea_128(void* dst, const V& src) noexcept { I::simd_storea_128(dst, simd_u8(src.v)); }
template<typename V> BL_INLINE_NODEBUG void storeu_128(void* dst, const V& src) noexcept { I::simd_storeu_128(dst, simd_u8(src.v)); }

// SIMD - Public - Shuffle & Permute
// =================================

template<typename V, typename W>
BL_INLINE_NODEBUG V swizzlev_u8(const V& a, const W& b) noexcept { return from_simd<V>(I::simd_swizzlev_u8(simd_u8(a.v), simd_u8(b.v))); }

template<uint8_t D, uint8_t C, uint8_t B, uint8_t A, typename V>
BL_INLINE_NODEBUG V shuffle_u32(const V& lo, const V& hi) noexcept { return from_simd<V>(I::simd_shuffle_u32<D, C, B, A>(simd_u32(lo.v), simd_u32(hi.v))); }

template<uint8_t B, uint8_t A, typename V>
BL_INLINE_NODEBUG V shuffle_u64(const V& lo, const V& hi) noexcept { return from_simd<V>(I::simd_shuffle_u64<B, A>(simd_u64(lo.v), simd_u64(hi.v))); }

template<uint8_t D, uint8_t C, uint8_t B, uint8_t A, typename V>
BL_INLINE_NODEBUG V shuffle_f32(const V& lo, const V& hi) noexcept { return from_simd<V>(I::simd_shuffle_f32<D, C, B, A>(simd_f32(lo.v), simd_f32(hi.v))); }

#if defined(BL_SIMD_AARCH64)
template<uint8_t B, uint8_t A, typename V>
BL_INLINE_NODEBUG V shuffle_f64(const V& lo, const V& hi) noexcept { return from_simd<V>(I::simd_shuffle_f64<B, A>(simd_f64(lo.v), simd_f64(hi.v))); }
#endif // BL_SIMD_AARCH64

template<uint8_t D, uint8_t C, uint8_t B, uint8_t A, typename V>
BL_INLINE_NODEBUG V swizzle_u16(const V& a) noexcept { return from_simd<V>(I::simd_swizzle_u16<D, C, B, A>(simd_u16(a.v))); }

template<uint8_t D, uint8_t C, uint8_t B, uint8_t A, typename V>
BL_INLINE_NODEBUG V swizzle_lo_u16(const V& a) noexcept { return from_simd<V>(I::simd_swizzle_lo_u16<D, C, B, A>(simd_u16(a.v))); }

template<uint8_t D, uint8_t C, uint8_t B, uint8_t A, typename V>
BL_INLINE_NODEBUG V swizzle_hi_u16(const V& a) noexcept { return from_simd<V>(I::simd_swizzle_hi_u16<D, C, B, A>(simd_u16(a.v))); }

template<uint8_t D, uint8_t C, uint8_t B, uint8_t A, typename V>
BL_INLINE_NODEBUG V swizzle_u32(const V& a) noexcept { return from_simd<V>(I::simd_swizzle_u32<D, C, B, A>(simd_u32(a.v))); }

template<uint8_t B, uint8_t A, typename V>
BL_INLINE_NODEBUG V swizzle_u64(const V& a) noexcept { return from_simd<V>(I::simd_swizzle_u64<B, A>(simd_u64(a.v))); }

template<uint8_t D, uint8_t C, uint8_t B, uint8_t A, typename V>
BL_INLINE_NODEBUG V swizzle_f32(const V& a) noexcept { return from_simd<V>(I::simd_swizzle_f32<D, C, B, A>(simd_f32(a.v))); }

#if defined(BL_SIMD_AARCH64)
template<uint8_t B, uint8_t A, typename V>
BL_INLINE_NODEBUG V swizzle_f64(const V& a) noexcept { return from_simd<V>(I::simd_swizzle_f64<B, A>(simd_f64(a.v))); }
#endif // BL_SIMD_AARCH64

template<typename V, typename W> BL_INLINE_NODEBUG V broadcast_u8(const W& a) noexcept { return from_simd<V>(I::simd_broadcast_u8(simd_u8(a.v))); }
template<typename V, typename W> BL_INLINE_NODEBUG V broadcast_u16(const W& a) noexcept { return from_simd<V>(I::simd_broadcast_u16(simd_u16(a.v))); }
template<typename V, typename W> BL_INLINE_NODEBUG V broadcast_u32(const W& a) noexcept { return from_simd<V>(I::simd_broadcast_u32(simd_u32(a.v))); }
template<typename V, typename W> BL_INLINE_NODEBUG V broadcast_u64(const W& a) noexcept { return from_simd<V>(I::simd_broadcast_u64(simd_u64(a.v))); }
template<typename V, typename W> BL_INLINE_NODEBUG V broadcast_f32(const W& a) noexcept { return from_simd<V>(I::simd_broadcast_f32(simd_f32(a.v))); }
#if defined(BL_SIMD_AARCH64)
template<typename V, typename W> BL_INLINE_NODEBUG V broadcast_f64(const W& a) noexcept { return from_simd<V>(I::simd_broadcast_f64(simd_f64(a.v))); }
#endif // BL_SIMD_AARCH64

template<typename V> BL_INLINE_NODEBUG V dup_lo_u32(const V& a) noexcept { return from_simd<V>(I::simd_dup_lo_u32(simd_u32(a.v))); }
template<typename V> BL_INLINE_NODEBUG V dup_hi_u32(const V& a) noexcept { return from_simd<V>(I::simd_dup_hi_u32(simd_u32(a.v))); }
template<typename V> BL_INLINE_NODEBUG V dup_lo_u64(const V& a) noexcept { return from_simd<V>(I::simd_dup_lo_u64(simd_u64(a.v))); }
template<typename V> BL_INLINE_NODEBUG V dup_hi_u64(const V& a) noexcept { return from_simd<V>(I::simd_dup_hi_u64(simd_u64(a.v))); }

template<typename V> BL_INLINE_NODEBUG V dup_lo_f32(const V& a) noexcept { return from_simd<V>(I::simd_dup_lo_f32(simd_f32(a.v))); }
template<typename V> BL_INLINE_NODEBUG V dup_hi_f32(const V& a) noexcept { return from_simd<V>(I::simd_dup_hi_f32(simd_f32(a.v))); }
template<typename V> BL_INLINE_NODEBUG V dup_lo_f32x2(const V& a) noexcept { return from_simd<V>(I::simd_dup_lo_f32x2(simd_f32(a.v))); }
template<typename V> BL_INLINE_NODEBUG V dup_hi_f32x2(const V& a) noexcept { return from_simd<V>(I::simd_dup_hi_f32x2(simd_f32(a.v))); }

template<typename V> BL_INLINE_NODEBUG V swap_u32(const V& a) noexcept { return from_simd<V>(I::simd_swap_u32(simd_u32(a.v))); }
template<typename V> BL_INLINE_NODEBUG V swap_u64(const V& a) noexcept { return from_simd<V>(I::simd_swap_u64(simd_u64(a.v))); }
template<typename V> BL_INLINE_NODEBUG V swap_f32(const V& a) noexcept { return from_simd<V>(I::simd_swap_f32(simd_f32(a.v))); }

#if defined(BL_SIMD_AARCH64)
template<typename V> BL_INLINE_NODEBUG V dup_lo_f64(const V& a) noexcept { return from_simd<V>(I::simd_dup_lo_f64(simd_f64(a.v))); }
template<typename V> BL_INLINE_NODEBUG V dup_hi_f64(const V& a) noexcept { return from_simd<V>(I::simd_dup_hi_f64(simd_f64(a.v))); }
template<typename V> BL_INLINE_NODEBUG V swap_f64(const V& a) noexcept { return from_simd<V>(I::simd_swap_f64(a.v)); }
#endif // BL_SIMD_AARCH64

template<typename V> BL_INLINE_NODEBUG V interleave_lo_u8(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_interleave_lo_u8(simd_u8(a.v), simd_u8(b.v))); }
template<typename V> BL_INLINE_NODEBUG V interleave_hi_u8(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_interleave_hi_u8(simd_u8(a.v), simd_u8(b.v))); }
template<typename V> BL_INLINE_NODEBUG V interleave_lo_u16(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_interleave_lo_u16(simd_u16(a.v), simd_u16(b.v))); }
template<typename V> BL_INLINE_NODEBUG V interleave_hi_u16(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_interleave_hi_u16(simd_u16(a.v), simd_u16(b.v))); }
template<typename V> BL_INLINE_NODEBUG V interleave_lo_u32(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_interleave_lo_u32(simd_u32(a.v), simd_u32(b.v))); }
template<typename V> BL_INLINE_NODEBUG V interleave_hi_u32(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_interleave_hi_u32(simd_u32(a.v), simd_u32(b.v))); }
template<typename V> BL_INLINE_NODEBUG V interleave_lo_u64(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_interleave_lo_u64(simd_u64(a.v), simd_u64(b.v))); }
template<typename V> BL_INLINE_NODEBUG V interleave_hi_u64(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_interleave_hi_u64(simd_u64(a.v), simd_u64(b.v))); }

template<typename V> BL_INLINE_NODEBUG V interleave_lo_f32(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_interleave_lo_f32(simd_f32(a.v), simd_f32(b.v))); }
template<typename V> BL_INLINE_NODEBUG V interleave_hi_f32(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_interleave_hi_f32(simd_f32(a.v), simd_f32(b.v))); }

#if defined(BL_SIMD_AARCH64)
template<typename V> BL_INLINE_NODEBUG V interleave_lo_f64(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_interleave_lo_f64(simd_f64(a.v), simd_f64(b.v))); }
template<typename V> BL_INLINE_NODEBUG V interleave_hi_f64(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_interleave_hi_f64(simd_f64(a.v), simd_f64(b.v))); }
#endif // BL_SIMD_AARCH64

template<int kN, typename V>
BL_INLINE_NODEBUG V alignr_u128(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_alignr_u128<kN>(simd_u8(a.v), simd_u8(b.v))); }

// SIMD - Public - Integer Packing & Unpacking
// ===========================================

template<typename V> BL_INLINE_NODEBUG V packs_128_i16_i8(const V& a) noexcept { return from_simd<V>(I::simd_packs_128_i16_i8(simd_i16(a.v))); }
template<typename V> BL_INLINE_NODEBUG V packs_128_i16_u8(const V& a) noexcept { return from_simd<V>(I::simd_packs_128_i16_u8(simd_i16(a.v))); }
template<typename V> BL_INLINE_NODEBUG V packz_128_u16_u8(const V& a) noexcept { return from_simd<V>(I::simd_packz_128_u16_u8(simd_u16(a.v))); }
template<typename V> BL_INLINE_NODEBUG V packs_128_i32_i8(const V& a) noexcept { return from_simd<V>(I::simd_packs_128_i32_i8(simd_i32(a.v))); }
template<typename V> BL_INLINE_NODEBUG V packs_128_i32_u8(const V& a) noexcept { return from_simd<V>(I::simd_packs_128_i32_u8(simd_i32(a.v))); }
template<typename V> BL_INLINE_NODEBUG V packs_128_i32_i16(const V& a) noexcept { return from_simd<V>(I::simd_packs_128_i32_i16(simd_i32(a.v))); }
template<typename V> BL_INLINE_NODEBUG V packs_128_i32_u16(const V& a) noexcept { return from_simd<V>(I::simd_packs_128_i32_u16(simd_i32(a.v))); }
template<typename V> BL_INLINE_NODEBUG V packz_128_u32_u8(const V& a) noexcept { return from_simd<V>(I::simd_packz_128_u32_u8(simd_u32(a.v))); }
template<typename V> BL_INLINE_NODEBUG V packz_128_u32_u16(const V& a) noexcept { return from_simd<V>(I::simd_packz_128_u32_u16(simd_u32(a.v))); }

template<typename V> BL_INLINE_NODEBUG V packs_128_i16_i8(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_packs_128_i16_i8(simd_i16(a.v), simd_i16(b.v))); }
template<typename V> BL_INLINE_NODEBUG V packs_128_i16_u8(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_packs_128_i16_u8(simd_i16(a.v), simd_i16(b.v))); }
template<typename V> BL_INLINE_NODEBUG V packz_128_u16_u8(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_packz_128_u16_u8(simd_u16(a.v), simd_u16(b.v))); }
template<typename V> BL_INLINE_NODEBUG V packs_128_i32_i8(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_packs_128_i32_i8(simd_i32(a.v), simd_i32(b.v))); }
template<typename V> BL_INLINE_NODEBUG V packs_128_i32_u8(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_packs_128_i32_u8(simd_i32(a.v), simd_i32(b.v))); }
template<typename V> BL_INLINE_NODEBUG V packs_128_i32_i16(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_packs_128_i32_i16(simd_i32(a.v), simd_i32(b.v))); }
template<typename V> BL_INLINE_NODEBUG V packs_128_i32_u16(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_packs_128_i32_u16(simd_i32(a.v), simd_i32(b.v))); }
template<typename V> BL_INLINE_NODEBUG V packz_128_u32_u8(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_packz_128_u32_u8(simd_u32(a.v), simd_u32(b.v))); }
template<typename V> BL_INLINE_NODEBUG V packz_128_u32_u16(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_packz_128_u32_u16(simd_u32(a.v), simd_u32(b.v))); }

template<typename V> BL_INLINE_NODEBUG V packs_128_i32_i8(const V& a, const V& b, const V& c, const V& d) noexcept {
  return from_simd<V>(I::simd_packs_128_i32_i8(simd_i32(a.v), simd_i32(b.v), simd_i32(c.v), simd_i32(d.v)));
}

template<typename V> BL_INLINE_NODEBUG V packs_128_i32_u8(const V& a, const V& b, const V& c, const V& d) noexcept {
  return from_simd<V>(I::simd_packs_128_i32_u8(simd_i32(a.v), simd_i32(b.v), simd_i32(c.v), simd_i32(d.v)));
}

template<typename V> BL_INLINE_NODEBUG V packz_128_u32_u8(const V& a, const V& b, const V& c, const V& d) noexcept {
  return from_simd<V>(I::simd_packz_128_u32_u8(simd_u32(a.v), simd_u32(b.v), simd_u32(c.v), simd_u32(d.v)));
}

template<typename V> BL_INLINE_NODEBUG V unpack_lo64_i8_i16(const V& a) noexcept { return from_simd<V>(I::simd_unpack_lo64_i8_i16(simd_i8(a.v))); }
template<typename V> BL_INLINE_NODEBUG V unpack_lo64_u8_u16(const V& a) noexcept { return from_simd<V>(I::simd_unpack_lo64_u8_u16(simd_u8(a.v))); }
template<typename V> BL_INLINE_NODEBUG V unpack_lo64_i16_i32(const V& a) noexcept { return from_simd<V>(I::simd_unpack_lo64_i16_i32(simd_i16(a.v))); }
template<typename V> BL_INLINE_NODEBUG V unpack_lo64_u16_u32(const V& a) noexcept { return from_simd<V>(I::simd_unpack_lo64_u16_u32(simd_u16(a.v))); }
template<typename V> BL_INLINE_NODEBUG V unpack_lo64_i32_i64(const V& a) noexcept { return from_simd<V>(I::simd_unpack_lo64_i32_i64(simd_i32(a.v))); }
template<typename V> BL_INLINE_NODEBUG V unpack_lo64_u32_u64(const V& a) noexcept { return from_simd<V>(I::simd_unpack_lo64_u32_u64(simd_u32(a.v))); }
template<typename V> BL_INLINE_NODEBUG V unpack_lo32_i8_i32(const V& a) noexcept { return from_simd<V>(I::simd_unpack_lo32_i8_i32(simd_i8(a.v))); }
template<typename V> BL_INLINE_NODEBUG V unpack_lo32_u8_u32(const V& a) noexcept { return from_simd<V>(I::simd_unpack_lo32_u8_u32(simd_u8(a.v))); }

template<typename V> BL_INLINE_NODEBUG V unpack_hi64_i8_i16(const V& a) noexcept { return from_simd<V>(I::simd_unpack_hi64_i8_i16(simd_i8(a.v))); }
template<typename V> BL_INLINE_NODEBUG V unpack_hi64_u8_u16(const V& a) noexcept { return from_simd<V>(I::simd_unpack_hi64_u8_u16(simd_u8(a.v))); }
template<typename V> BL_INLINE_NODEBUG V unpack_hi64_i16_i32(const V& a) noexcept { return from_simd<V>(I::simd_unpack_hi64_i16_i32(simd_i16(a.v))); }
template<typename V> BL_INLINE_NODEBUG V unpack_hi64_u16_u32(const V& a) noexcept { return from_simd<V>(I::simd_unpack_hi64_u16_u32(simd_u16(a.v))); }
template<typename V> BL_INLINE_NODEBUG V unpack_hi64_i32_i64(const V& a) noexcept { return from_simd<V>(I::simd_unpack_hi64_i32_i64(simd_i32(a.v))); }
template<typename V> BL_INLINE_NODEBUG V unpack_hi64_u32_u64(const V& a) noexcept { return from_simd<V>(I::simd_unpack_hi64_u32_u64(simd_u32(a.v))); }

/*
// TODO:
template<typename V> BL_INLINE_NODEBUG V movw_i8_i16(const V& a) noexcept { return from_simd<V>(I::simd_movw_i8_i16(a.v)); }
template<typename V> BL_INLINE_NODEBUG V movw_i8_i32(const V& a) noexcept { return from_simd<V>(I::simd_movw_i8_i32(a.v)); }
template<typename V> BL_INLINE_NODEBUG V movw_i8_i64(const V& a) noexcept { return from_simd<V>(I::simd_movw_i8_i64(a.v)); }
template<typename V> BL_INLINE_NODEBUG V movw_i16_i32(const V& a) noexcept { return from_simd<V>(I::simd_movw_i16_i32(a.v)); }
template<typename V> BL_INLINE_NODEBUG V movw_i16_i64(const V& a) noexcept { return from_simd<V>(I::simd_movw_i16_i64(a.v)); }
template<typename V> BL_INLINE_NODEBUG V movw_i32_i64(const V& a) noexcept { return from_simd<V>(I::simd_movw_i32_i64(a.v)); }

template<typename V> BL_INLINE_NODEBUG V movw_u8_u16(const V& a) noexcept { return from_simd<V>(I::simd_movw_u8_u16(a.v)); }
template<typename V> BL_INLINE_NODEBUG V movw_u8_u32(const V& a) noexcept { return from_simd<V>(I::simd_movw_u8_u32(a.v)); }
template<typename V> BL_INLINE_NODEBUG V movw_u8_u64(const V& a) noexcept { return from_simd<V>(I::simd_movw_u8_u64(a.v)); }
template<typename V> BL_INLINE_NODEBUG V movw_u16_u32(const V& a) noexcept { return from_simd<V>(I::simd_movw_u16_u32(a.v)); }
template<typename V> BL_INLINE_NODEBUG V movw_u16_u64(const V& a) noexcept { return from_simd<V>(I::simd_movw_u16_u64(a.v)); }
template<typename V> BL_INLINE_NODEBUG V movw_u32_u64(const V& a) noexcept { return from_simd<V>(I::simd_movw_u32_u64(a.v)); }
*/

// SIMD - Public - Arithmetic & Logical Operations
// ===============================================

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> not_(const Vec<W, T>& a) noexcept { return vec_wt<W, T>(I::simd_not(simd_u8(a.v))); }

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> and_(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_and(simd_u8(a.v), simd_u8(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> andnot(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_andnot(simd_u8(a.v), simd_u8(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> or_(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_or(simd_u8(a.v), simd_u8(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> xor_(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_xor(simd_u8(a.v), simd_u8(b.v))); }

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> and_(const Vec<W, T>& a, const Vec<W, T>& b, const Vec<W, T>& c) noexcept { return and_(and_(a, b), c); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> or_(const Vec<W, T>& a, const Vec<W, T>& b, const Vec<W, T>& c) noexcept { return or_(or_(a, b), c); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> xor_(const Vec<W, T>& a, const Vec<W, T>& b, const Vec<W, T>& c) noexcept { return xor_(xor_(a, b), c); }

template<uint32_t H, uint32_t G, uint32_t F, uint32_t E, uint32_t D, uint32_t C, uint32_t B, uint32_t A, typename V>
BL_INLINE_NODEBUG V blend_u16(const V& a, const V& b) noexcept {
  return from_simd<V>(I::simd_blend_u16<H, G, F, E, D, C, B, A>(simd_u8(a.v), simd_u8(b.v)));
}

template<uint32_t D, uint32_t C, uint32_t B, uint32_t A, typename V>
BL_INLINE_NODEBUG V blend_u32(const V& a, const V& b) noexcept {
  return from_simd<V>(I::simd_blend_u32<D, C, B, A>(simd_u8(a.v), simd_u8(b.v)));
}

template<uint32_t B, uint32_t A, typename V>
BL_INLINE_NODEBUG V blend_u64(const V& a, const V& b) noexcept {
  return from_simd<V>(I::simd_blend_u64<B, A>(simd_u8(a.v), simd_u8(b.v)));
}

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> blendv_bits(const Vec<W, T>& a, const Vec<W, T>& b, const Vec<W, T>& msk) noexcept { return vec_wt<W, T>(I::simd_blendv_bits(simd_u8(a.v), simd_u8(b.v), simd_u8(msk.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> blendv_u8(const Vec<W, T>& a, const Vec<W, T>& b, const Vec<W, T>& msk) noexcept { return vec_wt<W, T>(I::simd_blendv_u8(simd_u8(a.v), simd_u8(b.v), simd_u8(msk.v))); }

template<typename V> BL_INLINE_NODEBUG V add_f32(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_add_f32(simd_f32(a.v), simd_f32(b.v))); }
template<typename V> BL_INLINE_NODEBUG V sub_f32(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_sub_f32(simd_f32(a.v), simd_f32(b.v))); }
template<typename V> BL_INLINE_NODEBUG V mul_f32(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_mul_f32(simd_f32(a.v), simd_f32(b.v))); }
template<typename V> BL_INLINE_NODEBUG V div_f32(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_div_f32(simd_f32(a.v), simd_f32(b.v))); }
template<typename V> BL_INLINE_NODEBUG V min_f32(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_min_f32(simd_f32(a.v), simd_f32(b.v))); }
template<typename V> BL_INLINE_NODEBUG V max_f32(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_max_f32(simd_f32(a.v), simd_f32(b.v))); }

template<typename V> BL_INLINE_NODEBUG V cmp_eq_f32(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_cmp_eq_f32(simd_f32(a.v), simd_f32(b.v))); }
template<typename V> BL_INLINE_NODEBUG V cmp_ne_f32(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_cmp_ne_f32(simd_f32(a.v), simd_f32(b.v))); }
template<typename V> BL_INLINE_NODEBUG V cmp_ge_f32(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_cmp_ge_f32(simd_f32(a.v), simd_f32(b.v))); }
template<typename V> BL_INLINE_NODEBUG V cmp_gt_f32(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_cmp_gt_f32(simd_f32(a.v), simd_f32(b.v))); }
template<typename V> BL_INLINE_NODEBUG V cmp_le_f32(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_cmp_le_f32(simd_f32(a.v), simd_f32(b.v))); }
template<typename V> BL_INLINE_NODEBUG V cmp_lt_f32(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_cmp_lt_f32(simd_f32(a.v), simd_f32(b.v))); }

template<typename V> BL_INLINE_NODEBUG V abs_f32(const V& a) noexcept { return from_simd<V>(I::simd_abs_f32(simd_f32(a.v))); }
template<typename V> BL_INLINE_NODEBUG V sqrt_f32(const V& a) noexcept { return from_simd<V>(I::simd_sqrt_f32(simd_f32(a.v))); }

#if defined(BL_SIMD_AARCH64)
template<typename V> BL_INLINE_NODEBUG V add_f64(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_add_f64(simd_f64(a.v), simd_f64(b.v))); }
template<typename V> BL_INLINE_NODEBUG V sub_f64(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_sub_f64(simd_f64(a.v), simd_f64(b.v))); }
template<typename V> BL_INLINE_NODEBUG V mul_f64(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_mul_f64(simd_f64(a.v), simd_f64(b.v))); }
template<typename V> BL_INLINE_NODEBUG V div_f64(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_div_f64(simd_f64(a.v), simd_f64(b.v))); }
template<typename V> BL_INLINE_NODEBUG V min_f64(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_min_f64(simd_f64(a.v), simd_f64(b.v))); }
template<typename V> BL_INLINE_NODEBUG V max_f64(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_max_f64(simd_f64(a.v), simd_f64(b.v))); }

template<typename V> BL_INLINE_NODEBUG V cmp_eq_f64(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_cmp_eq_f64(simd_f64(a.v), simd_f64(b.v))); }
template<typename V> BL_INLINE_NODEBUG V cmp_ne_f64(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_cmp_ne_f64(simd_f64(a.v), simd_f64(b.v))); }
template<typename V> BL_INLINE_NODEBUG V cmp_ge_f64(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_cmp_ge_f64(simd_f64(a.v), simd_f64(b.v))); }
template<typename V> BL_INLINE_NODEBUG V cmp_gt_f64(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_cmp_gt_f64(simd_f64(a.v), simd_f64(b.v))); }
template<typename V> BL_INLINE_NODEBUG V cmp_le_f64(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_cmp_le_f64(simd_f64(a.v), simd_f64(b.v))); }
template<typename V> BL_INLINE_NODEBUG V cmp_lt_f64(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_cmp_lt_f64(simd_f64(a.v), simd_f64(b.v))); }

template<typename V> BL_INLINE_NODEBUG V abs_f64(const V& a) noexcept { return from_simd<V>(I::simd_abs_f64(simd_f64(a.v))); }
template<typename V> BL_INLINE_NODEBUG V sqrt_f64(const V& a) noexcept { return from_simd<V>(I::simd_sqrt_f64(simd_f64(a.v))); }
#endif // BL_SIMD_AARCH64

template<size_t W> BL_INLINE_NODEBUG Vec<W, float> add(const Vec<W, float>& a, const Vec<W, float>& b) noexcept { return vec_wt<W, float>(I::simd_add_f32(a.v, b.v)); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, float> sub(const Vec<W, float>& a, const Vec<W, float>& b) noexcept { return vec_wt<W, float>(I::simd_sub_f32(a.v, b.v)); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, float> mul(const Vec<W, float>& a, const Vec<W, float>& b) noexcept { return vec_wt<W, float>(I::simd_mul_f32(a.v, b.v)); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, float> div(const Vec<W, float>& a, const Vec<W, float>& b) noexcept { return vec_wt<W, float>(I::simd_div_f32(a.v, b.v)); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, float> min(const Vec<W, float>& a, const Vec<W, float>& b) noexcept { return vec_wt<W, float>(I::simd_min_f32(a.v, b.v)); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, float> max(const Vec<W, float>& a, const Vec<W, float>& b) noexcept { return vec_wt<W, float>(I::simd_max_f32(a.v, b.v)); }

template<size_t W> BL_INLINE_NODEBUG Vec<W, float> cmp_eq(const Vec<W, float>& a, const Vec<W, float>& b) noexcept { return vec_wt<W, float>(I::simd_cmp_eq_f32(a.v, b.v)); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, float> cmp_ne(const Vec<W, float>& a, const Vec<W, float>& b) noexcept { return vec_wt<W, float>(I::simd_cmp_ne_f32(a.v, b.v)); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, float> cmp_ge(const Vec<W, float>& a, const Vec<W, float>& b) noexcept { return vec_wt<W, float>(I::simd_cmp_ge_f32(a.v, b.v)); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, float> cmp_gt(const Vec<W, float>& a, const Vec<W, float>& b) noexcept { return vec_wt<W, float>(I::simd_cmp_gt_f32(a.v, b.v)); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, float> cmp_le(const Vec<W, float>& a, const Vec<W, float>& b) noexcept { return vec_wt<W, float>(I::simd_cmp_le_f32(a.v, b.v)); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, float> cmp_lt(const Vec<W, float>& a, const Vec<W, float>& b) noexcept { return vec_wt<W, float>(I::simd_cmp_lt_f32(a.v, b.v)); }

template<size_t W> BL_INLINE_NODEBUG Vec<W, float> abs(const Vec<W, float>& a) noexcept { return vec_wt<W, float>(I::simd_abs_f32(a.v)); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, float> sqrt(const Vec<W, float>& a) noexcept { return vec_wt<W, float>(I::simd_sqrt_f32(a.v)); }

#if defined(BL_SIMD_AARCH64)
template<size_t W> BL_INLINE_NODEBUG Vec<W, double> add(const Vec<W, double>& a, const Vec<W, double>& b) noexcept { return vec_wt<W, double>(I::simd_add_f64(a.v, b.v)); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, double> sub(const Vec<W, double>& a, const Vec<W, double>& b) noexcept { return vec_wt<W, double>(I::simd_sub_f64(a.v, b.v)); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, double> mul(const Vec<W, double>& a, const Vec<W, double>& b) noexcept { return vec_wt<W, double>(I::simd_mul_f64(a.v, b.v)); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, double> div(const Vec<W, double>& a, const Vec<W, double>& b) noexcept { return vec_wt<W, double>(I::simd_div_f64(a.v, b.v)); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, double> min(const Vec<W, double>& a, const Vec<W, double>& b) noexcept { return vec_wt<W, double>(I::simd_min_f64(a.v, b.v)); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, double> max(const Vec<W, double>& a, const Vec<W, double>& b) noexcept { return vec_wt<W, double>(I::simd_max_f64(a.v, b.v)); }

template<size_t W> BL_INLINE_NODEBUG Vec<W, double> cmp_eq(const Vec<W, double>& a, const Vec<W, double>& b) noexcept { return vec_wt<W, double>(I::simd_cmp_eq_f64(a.v, b.v)); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, double> cmp_ne(const Vec<W, double>& a, const Vec<W, double>& b) noexcept { return vec_wt<W, double>(I::simd_cmp_ne_f64(a.v, b.v)); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, double> cmp_ge(const Vec<W, double>& a, const Vec<W, double>& b) noexcept { return vec_wt<W, double>(I::simd_cmp_ge_f64(a.v, b.v)); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, double> cmp_gt(const Vec<W, double>& a, const Vec<W, double>& b) noexcept { return vec_wt<W, double>(I::simd_cmp_gt_f64(a.v, b.v)); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, double> cmp_le(const Vec<W, double>& a, const Vec<W, double>& b) noexcept { return vec_wt<W, double>(I::simd_cmp_le_f64(a.v, b.v)); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, double> cmp_lt(const Vec<W, double>& a, const Vec<W, double>& b) noexcept { return vec_wt<W, double>(I::simd_cmp_lt_f64(a.v, b.v)); }

template<size_t W> BL_INLINE_NODEBUG Vec<W, double> abs(const Vec<W, double>& a) noexcept { return vec_wt<W, double>(I::simd_abs_f64(a.v)); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, double> sqrt(const Vec<W, double>& a) noexcept { return vec_wt<W, double>(I::simd_sqrt_f64(a.v)); }
#endif // BL_SIMD_AARCH64

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> abs_i8(const Vec<W, T>& a) noexcept { return vec_wt<W, T>(I::simd_abs_i8(simd_i8(a.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> abs_i16(const Vec<W, T>& a) noexcept { return vec_wt<W, T>(I::simd_abs_i16(simd_i16(a.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> abs_i32(const Vec<W, T>& a) noexcept { return vec_wt<W, T>(I::simd_abs_i32(simd_i32(a.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> abs_i64(const Vec<W, T>& a) noexcept { return vec_wt<W, T>(I::simd_abs_i64(simd_i64(a.v))); }

template<size_t W> BL_INLINE_NODEBUG Vec<W, int8_t> abs(const Vec<W, int8_t>& a) noexcept { return abs_i8(a); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> abs(const Vec<W, int16_t>& a) noexcept { return abs_i16(a); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int32_t> abs(const Vec<W, int32_t>& a) noexcept { return abs_i32(a); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int64_t> abs(const Vec<W, int64_t>& a) noexcept { return abs_i64(a); }

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> add_i8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_add_i8(simd_i8(a.v), simd_i8(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> add_i16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_add_i16(simd_i16(a.v), simd_i16(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> add_i32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_add_i32(simd_i32(a.v), simd_i32(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> add_i64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_add_i64(simd_i64(a.v), simd_i64(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> add_u8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_add_u8(simd_u8(a.v), simd_u8(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> add_u16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_add_u16(simd_u16(a.v), simd_u16(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> add_u32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_add_u32(simd_u32(a.v), simd_u32(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> add_u64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_add_u64(simd_u64(a.v), simd_u64(b.v))); }

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> adds_i8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_adds_i8(simd_i8(a.v), simd_i8(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> adds_i16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_adds_i16(simd_i16(a.v), simd_i16(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> adds_i32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_adds_i32(simd_i32(a.v), simd_i32(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> adds_i64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_adds_i64(simd_i64(a.v), simd_i64(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> adds_u8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_adds_u8(simd_u8(a.v), simd_u8(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> adds_u16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_adds_u16(simd_u16(a.v), simd_u16(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> adds_u32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_adds_u32(simd_u32(a.v), simd_u32(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> adds_u64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_adds_u64(simd_u64(a.v), simd_u64(b.v))); }

template<size_t W> BL_INLINE_NODEBUG Vec<W, int8_t> add(const Vec<W, int8_t>& a, const Vec<W, int8_t>& b) noexcept { return add_i8(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> add(const Vec<W, int16_t>& a, const Vec<W, int16_t>& b) noexcept { return add_i16(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int32_t> add(const Vec<W, int32_t>& a, const Vec<W, int32_t>& b) noexcept { return add_i32(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int64_t> add(const Vec<W, int64_t>& a, const Vec<W, int64_t>& b) noexcept { return add_i64(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint8_t> add(const Vec<W, uint8_t>& a, const Vec<W, uint8_t>& b) noexcept { return add_i8(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint16_t> add(const Vec<W, uint16_t>& a, const Vec<W, uint16_t>& b) noexcept { return add_i16(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint32_t> add(const Vec<W, uint32_t>& a, const Vec<W, uint32_t>& b) noexcept { return add_i32(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint64_t> add(const Vec<W, uint64_t>& a, const Vec<W, uint64_t>& b) noexcept { return add_i64(a, b); }

template<size_t W> BL_INLINE_NODEBUG Vec<W, int8_t> adds(const Vec<W, int8_t>& a, const Vec<W, int8_t>& b) noexcept { return adds_i8(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> adds(const Vec<W, int16_t>& a, const Vec<W, int16_t>& b) noexcept { return adds_i16(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int32_t> adds(const Vec<W, int32_t>& a, const Vec<W, int32_t>& b) noexcept { return adds_i32(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int64_t> adds(const Vec<W, int64_t>& a, const Vec<W, int64_t>& b) noexcept { return adds_i64(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint8_t> adds(const Vec<W, uint8_t>& a, const Vec<W, uint8_t>& b) noexcept { return adds_u8(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint16_t> adds(const Vec<W, uint16_t>& a, const Vec<W, uint16_t>& b) noexcept { return adds_u16(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint32_t> adds(const Vec<W, uint32_t>& a, const Vec<W, uint32_t>& b) noexcept { return adds_u32(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint64_t> adds(const Vec<W, uint64_t>& a, const Vec<W, uint64_t>& b) noexcept { return adds_u64(a, b); }

BL_INLINE_NODEBUG Vec<16, int16_t> addl_lo_i8_to_i16(const Vec<16, int8_t>& a, const Vec<16, int8_t>& b) noexcept { return Vec<16, int16_t>{I::simd_addl_lo_i8_to_i16(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec<16, int16_t> addl_hi_i8_to_i16(const Vec<16, int8_t>& a, const Vec<16, int8_t>& b) noexcept { return Vec<16, int16_t>{I::simd_addl_hi_i8_to_i16(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec<16, uint16_t> addl_lo_u8_to_u16(const Vec<16, uint8_t>& a, const Vec<16, uint8_t>& b) noexcept { return Vec<16, uint16_t>{I::simd_addl_lo_u8_to_u16(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec<16, uint16_t> addl_hi_u8_to_u16(const Vec<16, uint8_t>& a, const Vec<16, uint8_t>& b) noexcept { return Vec<16, uint16_t>{I::simd_addl_hi_u8_to_u16(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec<16, int32_t> addl_lo_i16_to_i32(const Vec<16, int16_t>& a, const Vec<16, int16_t>& b) noexcept { return Vec<16, int32_t>{I::simd_addl_lo_i16_to_i32(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec<16, int32_t> addl_hi_i16_to_i32(const Vec<16, int16_t>& a, const Vec<16, int16_t>& b) noexcept { return Vec<16, int32_t>{I::simd_addl_hi_i16_to_i32(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec<16, uint32_t> addl_lo_u16_to_u32(const Vec<16, uint16_t>& a, const Vec<16, uint16_t>& b) noexcept { return Vec<16, uint32_t>{I::simd_addl_lo_u16_to_u32(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec<16, uint32_t> addl_hi_u16_to_u32(const Vec<16, uint16_t>& a, const Vec<16, uint16_t>& b) noexcept { return Vec<16, uint32_t>{I::simd_addl_hi_u16_to_u32(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec<16, int64_t> addl_lo_i32_to_i64(const Vec<16, int32_t>& a, const Vec<16, int32_t>& b) noexcept { return Vec<16, int64_t>{I::simd_addl_lo_i32_to_i64(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec<16, int64_t> addl_hi_i32_to_i64(const Vec<16, int32_t>& a, const Vec<16, int32_t>& b) noexcept { return Vec<16, int64_t>{I::simd_addl_hi_i32_to_i64(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec<16, uint64_t> addl_lo_u32_to_u64(const Vec<16, uint32_t>& a, const Vec<16, uint32_t>& b) noexcept { return Vec<16, uint64_t>{I::simd_addl_lo_u32_to_u64(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec<16, uint64_t> addl_hi_u32_to_u64(const Vec<16, uint32_t>& a, const Vec<16, uint32_t>& b) noexcept { return Vec<16, uint64_t>{I::simd_addl_hi_u32_to_u64(a.v, b.v)}; }

BL_INLINE_NODEBUG Vec<16, int16_t> addw_lo_i8_to_i16(const Vec<16, int16_t>& a, const Vec<16, int8_t>& b) noexcept { return Vec<16, int16_t>{I::simd_addw_lo_i8_to_i16(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec<16, int16_t> addw_hi_i8_to_i16(const Vec<16, int16_t>& a, const Vec<16, int8_t>& b) noexcept { return Vec<16, int16_t>{I::simd_addw_hi_i8_to_i16(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec<16, uint16_t> addw_lo_u8_to_u16(const Vec<16, uint16_t>& a, const Vec<16, uint8_t>& b) noexcept { return Vec<16, uint16_t>{I::simd_addw_lo_u8_to_u16(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec<16, uint16_t> addw_hi_u8_to_u16(const Vec<16, uint16_t>& a, const Vec<16, uint8_t>& b) noexcept { return Vec<16, uint16_t>{I::simd_addw_hi_u8_to_u16(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec<16, int32_t> addw_lo_i16_to_i32(const Vec<16, int32_t>& a, const Vec<16, int16_t>& b) noexcept { return Vec<16, int32_t>{I::simd_addw_lo_i16_to_i32(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec<16, int32_t> addw_hi_i16_to_i32(const Vec<16, int32_t>& a, const Vec<16, int16_t>& b) noexcept { return Vec<16, int32_t>{I::simd_addw_hi_i16_to_i32(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec<16, uint32_t> addw_lo_u16_to_u32(const Vec<16, uint32_t>& a, const Vec<16, uint16_t>& b) noexcept { return Vec<16, uint32_t>{I::simd_addw_lo_u16_to_u32(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec<16, uint32_t> addw_hi_u16_to_u32(const Vec<16, uint32_t>& a, const Vec<16, uint16_t>& b) noexcept { return Vec<16, uint32_t>{I::simd_addw_hi_u16_to_u32(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec<16, int64_t> addw_lo_i32_to_i64(const Vec<16, int64_t>& a, const Vec<16, int32_t>& b) noexcept { return Vec<16, int64_t>{I::simd_addw_lo_i32_to_i64(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec<16, int64_t> addw_hi_i32_to_i64(const Vec<16, int64_t>& a, const Vec<16, int32_t>& b) noexcept { return Vec<16, int64_t>{I::simd_addw_hi_i32_to_i64(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec<16, uint64_t> addw_lo_u32_to_u64(const Vec<16, uint64_t>& a, const Vec<16, uint32_t>& b) noexcept { return Vec<16, uint64_t>{I::simd_addw_lo_u32_to_u64(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec<16, uint64_t> addw_hi_u32_to_u64(const Vec<16, uint64_t>& a, const Vec<16, uint32_t>& b) noexcept { return Vec<16, uint64_t>{I::simd_addw_hi_u32_to_u64(a.v, b.v)}; }

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> sub_i8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_sub_i8(simd_i8(a.v), simd_i8(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> sub_i16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_sub_i16(simd_i16(a.v), simd_i16(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> sub_i32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_sub_i32(simd_i32(a.v), simd_i32(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> sub_i64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_sub_i64(simd_i64(a.v), simd_i64(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> sub_u8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_sub_u8(simd_u8(a.v), simd_u8(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> sub_u16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_sub_u16(simd_u16(a.v), simd_u16(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> sub_u32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_sub_u32(simd_u32(a.v), simd_u32(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> sub_u64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_sub_u64(simd_u64(a.v), simd_u64(b.v))); }

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> subs_i8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_subs_i8(simd_i8(a.v), simd_i8(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> subs_i16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_subs_i16(simd_i16(a.v), simd_i16(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> subs_i32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_subs_i32(simd_i32(a.v), simd_i32(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> subs_i64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_subs_i64(simd_i64(a.v), simd_i64(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> subs_u8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_subs_u8(simd_u8(a.v), simd_u8(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> subs_u16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_subs_u16(simd_u16(a.v), simd_u16(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> subs_u32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_subs_u32(simd_u32(a.v), simd_u32(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> subs_u64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_subs_u64(simd_u64(a.v), simd_u64(b.v))); }

template<size_t W> BL_INLINE_NODEBUG Vec<W, int8_t> sub(const Vec<W, int8_t>& a, const Vec<W, int8_t>& b) noexcept { return sub_i8(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> sub(const Vec<W, int16_t>& a, const Vec<W, int16_t>& b) noexcept { return sub_i16(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int32_t> sub(const Vec<W, int32_t>& a, const Vec<W, int32_t>& b) noexcept { return sub_i32(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int64_t> sub(const Vec<W, int64_t>& a, const Vec<W, int64_t>& b) noexcept { return sub_i64(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint8_t> sub(const Vec<W, uint8_t>& a, const Vec<W, uint8_t>& b) noexcept { return sub_i8(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint16_t> sub(const Vec<W, uint16_t>& a, const Vec<W, uint16_t>& b) noexcept { return sub_i16(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint32_t> sub(const Vec<W, uint32_t>& a, const Vec<W, uint32_t>& b) noexcept { return sub_i32(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint64_t> sub(const Vec<W, uint64_t>& a, const Vec<W, uint64_t>& b) noexcept { return sub_i64(a, b); }

template<size_t W> BL_INLINE_NODEBUG Vec<W, int8_t> subs(const Vec<W, int8_t>& a, const Vec<W, int8_t>& b) noexcept { return subs_i8(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> subs(const Vec<W, int16_t>& a, const Vec<W, int16_t>& b) noexcept { return subs_i16(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int32_t> subs(const Vec<W, int32_t>& a, const Vec<W, int32_t>& b) noexcept { return subs_i32(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int64_t> subs(const Vec<W, int64_t>& a, const Vec<W, int64_t>& b) noexcept { return subs_i64(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint8_t> subs(const Vec<W, uint8_t>& a, const Vec<W, uint8_t>& b) noexcept { return subs_u8(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint16_t> subs(const Vec<W, uint16_t>& a, const Vec<W, uint16_t>& b) noexcept { return subs_u16(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint32_t> subs(const Vec<W, uint32_t>& a, const Vec<W, uint32_t>& b) noexcept { return subs_u32(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint64_t> subs(const Vec<W, uint64_t>& a, const Vec<W, uint64_t>& b) noexcept { return subs_u64(a, b); }

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> mul_i8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_mul_i8(simd_i8(a.v), simd_i8(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> mul_i16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_mul_i16(simd_i16(a.v), simd_i16(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> mul_i32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_mul_i32(simd_i32(a.v), simd_i32(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> mul_i64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_mul_i64(simd_i64(a.v), simd_i64(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> mul_u8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_mul_u8(simd_u8(a.v), simd_u8(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> mul_u16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_mul_u16(simd_u16(a.v), simd_u16(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> mul_u32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_mul_u32(simd_u32(a.v), simd_u32(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> mul_u64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_mul_u64(simd_u64(a.v), simd_u64(b.v))); }

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, uint16_t> mul_lo_u8_u16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, uint16_t>(I::simd_mul_lo_u8_u16(simd_u8(a.v), simd_u8(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, uint16_t> mul_hi_u8_u16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, uint16_t>(I::simd_mul_hi_u8_u16(simd_u8(a.v), simd_u8(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, uint32_t> mul_lo_u16_u32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, uint32_t>(I::simd_mul_lo_u16_u32(simd_u16(a.v), simd_u16(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, uint32_t> mul_hi_u16_u32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, uint32_t>(I::simd_mul_hi_u16_u32(simd_u16(a.v), simd_u16(b.v))); }

template<size_t W> BL_INLINE_NODEBUG Vec<W, int8_t> mul(const Vec<W, int8_t>& a, const Vec<W, int8_t>& b) noexcept { return mul_i8(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> mul(const Vec<W, int16_t>& a, const Vec<W, int16_t>& b) noexcept { return mul_i16(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int32_t> mul(const Vec<W, int32_t>& a, const Vec<W, int32_t>& b) noexcept { return mul_i32(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int64_t> mul(const Vec<W, int64_t>& a, const Vec<W, int64_t>& b) noexcept { return mul_i64(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint8_t> mul(const Vec<W, uint8_t>& a, const Vec<W, uint8_t>& b) noexcept { return mul_u8(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint16_t> mul(const Vec<W, uint16_t>& a, const Vec<W, uint16_t>& b) noexcept { return mul_u16(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint32_t> mul(const Vec<W, uint32_t>& a, const Vec<W, uint32_t>& b) noexcept { return mul_u32(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint64_t> mul(const Vec<W, uint64_t>& a, const Vec<W, uint64_t>& b) noexcept { return mul_u64(a, b); }

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_eq_i8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_eq_i8(simd_i8(a.v), simd_i8(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_eq_i16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_eq_i16(simd_i16(a.v), simd_i16(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_eq_i32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_eq_i32(simd_i32(a.v), simd_i32(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_eq_i64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_eq_i64(simd_i64(a.v), simd_i64(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_eq_u8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_eq_u8(simd_u8(a.v), simd_u8(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_eq_u16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_eq_u16(simd_u16(a.v), simd_u16(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_eq_u32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_eq_u32(simd_u32(a.v), simd_u32(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_eq_u64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_eq_u64(simd_u64(a.v), simd_u64(b.v))); }

template<size_t W> BL_INLINE_NODEBUG Vec<W, int8_t> cmp_eq(const Vec<W, int8_t>& a, const Vec<W, int8_t>& b) noexcept { return cmp_eq_i8(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> cmp_eq(const Vec<W, int16_t>& a, const Vec<W, int16_t>& b) noexcept { return cmp_eq_i16(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int32_t> cmp_eq(const Vec<W, int32_t>& a, const Vec<W, int32_t>& b) noexcept { return cmp_eq_i32(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int64_t> cmp_eq(const Vec<W, int64_t>& a, const Vec<W, int64_t>& b) noexcept { return cmp_eq_i64(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint8_t> cmp_eq(const Vec<W, uint8_t>& a, const Vec<W, uint8_t>& b) noexcept { return cmp_eq_u8(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint16_t> cmp_eq(const Vec<W, uint16_t>& a, const Vec<W, uint16_t>& b) noexcept { return cmp_eq_u16(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint32_t> cmp_eq(const Vec<W, uint32_t>& a, const Vec<W, uint32_t>& b) noexcept { return cmp_eq_u32(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint64_t> cmp_eq(const Vec<W, uint64_t>& a, const Vec<W, uint64_t>& b) noexcept { return cmp_eq_u64(a, b); }

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_ne_i8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_ne_i8(simd_i8(a.v), simd_i8(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_ne_i16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_ne_i16(simd_i16(a.v), simd_i16(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_ne_i32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_ne_i32(simd_i32(a.v), simd_i32(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_ne_i64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_ne_i64(simd_i64(a.v), simd_i64(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_ne_u8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_ne_u8(simd_u8(a.v), simd_u8(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_ne_u16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_ne_u16(simd_u16(a.v), simd_u16(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_ne_u32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_ne_u32(simd_u32(a.v), simd_u32(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_ne_u64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_ne_u64(simd_u64(a.v), simd_u64(b.v))); }

template<size_t W> BL_INLINE_NODEBUG Vec<W, int8_t> cmp_ne(const Vec<W, int8_t>& a, const Vec<W, int8_t>& b) noexcept { return cmp_ne_i8(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> cmp_ne(const Vec<W, int16_t>& a, const Vec<W, int16_t>& b) noexcept { return cmp_ne_i16(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int32_t> cmp_ne(const Vec<W, int32_t>& a, const Vec<W, int32_t>& b) noexcept { return cmp_ne_i32(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int64_t> cmp_ne(const Vec<W, int64_t>& a, const Vec<W, int64_t>& b) noexcept { return cmp_ne_i64(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint8_t> cmp_ne(const Vec<W, uint8_t>& a, const Vec<W, uint8_t>& b) noexcept { return cmp_ne_u8(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint16_t> cmp_ne(const Vec<W, uint16_t>& a, const Vec<W, uint16_t>& b) noexcept { return cmp_ne_u16(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint32_t> cmp_ne(const Vec<W, uint32_t>& a, const Vec<W, uint32_t>& b) noexcept { return cmp_ne_u32(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint64_t> cmp_ne(const Vec<W, uint64_t>& a, const Vec<W, uint64_t>& b) noexcept { return cmp_ne_u64(a, b); }

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_gt_i8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_gt_i8(simd_i8(a.v), simd_i8(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_gt_i16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_gt_i16(simd_i16(a.v), simd_i16(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_gt_i32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_gt_i32(simd_i32(a.v), simd_i32(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_gt_i64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_gt_i64(a.v, b.v)); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_gt_u8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_gt_u8(simd_u8(a.v), simd_u8(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_gt_u16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_gt_u16(simd_u16(a.v), simd_u16(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_gt_u32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_gt_u32(simd_u32(a.v), simd_u32(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_gt_u64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_gt_u64(a.v, b.v)); }

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_ge_i8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_ge_i8(simd_i8(a.v), simd_i8(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_ge_i16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_ge_i16(simd_i16(a.v), simd_i16(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_ge_i32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_ge_i32(simd_i32(a.v), simd_i32(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_ge_i64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_ge_i64(a.v, b.v)); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_ge_u8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_ge_u8(simd_u8(a.v), simd_u8(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_ge_u16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_ge_u16(simd_u16(a.v), simd_u16(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_ge_u32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_ge_u32(simd_u32(a.v), simd_u32(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_ge_u64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_ge_u64(a.v, b.v)); }

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_lt_i8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_lt_i8(simd_i8(a.v), simd_i8(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_lt_i16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_lt_i16(simd_i16(a.v), simd_i16(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_lt_i32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_lt_i32(simd_i32(a.v), simd_i32(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_lt_i64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_lt_i64(a.v, b.v)); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_lt_u8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_lt_u8(simd_u8(a.v), simd_u8(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_lt_u16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_lt_u16(simd_u16(a.v), simd_u16(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_lt_u32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_lt_u32(simd_u32(a.v), simd_u32(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_lt_u64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_lt_u64(a.v, b.v)); }

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_le_i8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_le_i8(simd_i8(a.v), simd_i8(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_le_i16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_le_i16(simd_i16(a.v), simd_i16(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_le_i32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_le_i32(simd_i32(a.v), simd_i32(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_le_i64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_le_i64(a.v, b.v)); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_le_u8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_le_u8(simd_u8(a.v), simd_u8(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_le_u16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_le_u16(simd_u16(a.v), simd_u16(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_le_u32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_le_u32(simd_u32(a.v), simd_u32(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_le_u64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_cmp_le_u64(a.v, b.v)); }

template<size_t W> BL_INLINE_NODEBUG Vec<W, int8_t> cmp_gt(const Vec<W, int8_t>& a, const Vec<W, int8_t>& b) noexcept { return cmp_gt_i8(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> cmp_gt(const Vec<W, int16_t>& a, const Vec<W, int16_t>& b) noexcept { return cmp_gt_i16(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int32_t> cmp_gt(const Vec<W, int32_t>& a, const Vec<W, int32_t>& b) noexcept { return cmp_gt_i32(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int64_t> cmp_gt(const Vec<W, int64_t>& a, const Vec<W, int64_t>& b) noexcept { return cmp_gt_i64(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint8_t> cmp_gt(const Vec<W, uint8_t>& a, const Vec<W, uint8_t>& b) noexcept { return cmp_gt_u8(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint16_t> cmp_gt(const Vec<W, uint16_t>& a, const Vec<W, uint16_t>& b) noexcept { return cmp_gt_u16(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint32_t> cmp_gt(const Vec<W, uint32_t>& a, const Vec<W, uint32_t>& b) noexcept { return cmp_gt_u32(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint64_t> cmp_gt(const Vec<W, uint64_t>& a, const Vec<W, uint64_t>& b) noexcept { return cmp_gt_u64(a, b); }

template<size_t W> BL_INLINE_NODEBUG Vec<W, int8_t> cmp_ge(const Vec<W, int8_t>& a, const Vec<W, int8_t>& b) noexcept { return cmp_ge_i8(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> cmp_ge(const Vec<W, int16_t>& a, const Vec<W, int16_t>& b) noexcept { return cmp_ge_i16(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int32_t> cmp_ge(const Vec<W, int32_t>& a, const Vec<W, int32_t>& b) noexcept { return cmp_ge_i32(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int64_t> cmp_ge(const Vec<W, int64_t>& a, const Vec<W, int64_t>& b) noexcept { return cmp_ge_i64(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint8_t> cmp_ge(const Vec<W, uint8_t>& a, const Vec<W, uint8_t>& b) noexcept { return cmp_ge_u8(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint16_t> cmp_ge(const Vec<W, uint16_t>& a, const Vec<W, uint16_t>& b) noexcept { return cmp_ge_u16(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint32_t> cmp_ge(const Vec<W, uint32_t>& a, const Vec<W, uint32_t>& b) noexcept { return cmp_ge_u32(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint64_t> cmp_ge(const Vec<W, uint64_t>& a, const Vec<W, uint64_t>& b) noexcept { return cmp_ge_u64(a, b); }

template<size_t W> BL_INLINE_NODEBUG Vec<W, int8_t> cmp_lt(const Vec<W, int8_t>& a, const Vec<W, int8_t>& b) noexcept { return cmp_lt_i8(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> cmp_lt(const Vec<W, int16_t>& a, const Vec<W, int16_t>& b) noexcept { return cmp_lt_i16(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int32_t> cmp_lt(const Vec<W, int32_t>& a, const Vec<W, int32_t>& b) noexcept { return cmp_lt_i32(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int64_t> cmp_lt(const Vec<W, int64_t>& a, const Vec<W, int64_t>& b) noexcept { return cmp_lt_i64(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint8_t> cmp_lt(const Vec<W, uint8_t>& a, const Vec<W, uint8_t>& b) noexcept { return cmp_lt_u8(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint16_t> cmp_lt(const Vec<W, uint16_t>& a, const Vec<W, uint16_t>& b) noexcept { return cmp_lt_u16(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint32_t> cmp_lt(const Vec<W, uint32_t>& a, const Vec<W, uint32_t>& b) noexcept { return cmp_lt_u32(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint64_t> cmp_lt(const Vec<W, uint64_t>& a, const Vec<W, uint64_t>& b) noexcept { return cmp_lt_u64(a, b); }

template<size_t W> BL_INLINE_NODEBUG Vec<W, int8_t> cmp_le(const Vec<W, int8_t>& a, const Vec<W, int8_t>& b) noexcept { return cmp_le_i8(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> cmp_le(const Vec<W, int16_t>& a, const Vec<W, int16_t>& b) noexcept { return cmp_le_i16(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int32_t> cmp_le(const Vec<W, int32_t>& a, const Vec<W, int32_t>& b) noexcept { return cmp_le_i32(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int64_t> cmp_le(const Vec<W, int64_t>& a, const Vec<W, int64_t>& b) noexcept { return cmp_le_i64(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint8_t> cmp_le(const Vec<W, uint8_t>& a, const Vec<W, uint8_t>& b) noexcept { return cmp_le_u8(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint16_t> cmp_le(const Vec<W, uint16_t>& a, const Vec<W, uint16_t>& b) noexcept { return cmp_le_u16(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint32_t> cmp_le(const Vec<W, uint32_t>& a, const Vec<W, uint32_t>& b) noexcept { return cmp_le_u32(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint64_t> cmp_le(const Vec<W, uint64_t>& a, const Vec<W, uint64_t>& b) noexcept { return cmp_le_u64(a, b); }

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> min_i8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_min_i8(simd_i8(a.v), simd_i8(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> min_i16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_min_i16(simd_i16(a.v), simd_i16(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> min_i32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_min_i32(simd_i32(a.v), simd_i32(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> min_i64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_min_i64(simd_i64(a.v), simd_i64(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> min_u8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_min_u8(simd_u8(a.v), simd_u8(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> min_u16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_min_u16(simd_u16(a.v), simd_u16(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> min_u32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_min_u32(simd_u32(a.v), simd_u32(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> min_u64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_min_u64(simd_u64(a.v), simd_u64(b.v))); }

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> max_i8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_max_i8(simd_i8(a.v), simd_i8(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> max_i16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_max_i16(simd_i16(a.v), simd_i16(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> max_i32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_max_i32(simd_i32(a.v), simd_i32(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> max_i64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_max_i64(simd_i64(a.v), simd_i64(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> max_u8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_max_u8(simd_u8(a.v), simd_u8(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> max_u16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_max_u16(simd_u16(a.v), simd_u16(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> max_u32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_max_u32(simd_u32(a.v), simd_u32(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> max_u64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_max_u64(simd_u64(a.v), simd_u64(b.v))); }

template<size_t W> BL_INLINE_NODEBUG Vec<W, int8_t> min(const Vec<W, int8_t>& a, const Vec<W, int8_t>& b) noexcept { return min_i8(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> min(const Vec<W, int16_t>& a, const Vec<W, int16_t>& b) noexcept { return min_i16(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int32_t> min(const Vec<W, int32_t>& a, const Vec<W, int32_t>& b) noexcept { return min_i32(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int64_t> min(const Vec<W, int64_t>& a, const Vec<W, int64_t>& b) noexcept { return min_i64(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint8_t> min(const Vec<W, uint8_t>& a, const Vec<W, uint8_t>& b) noexcept { return min_u8(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint16_t> min(const Vec<W, uint16_t>& a, const Vec<W, uint16_t>& b) noexcept { return min_u16(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint32_t> min(const Vec<W, uint32_t>& a, const Vec<W, uint32_t>& b) noexcept { return min_u32(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint64_t> min(const Vec<W, uint64_t>& a, const Vec<W, uint64_t>& b) noexcept { return min_u64(a, b); }

template<size_t W> BL_INLINE_NODEBUG Vec<W, int8_t> max(const Vec<W, int8_t>& a, const Vec<W, int8_t>& b) noexcept { return max_i8(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> max(const Vec<W, int16_t>& a, const Vec<W, int16_t>& b) noexcept { return max_i16(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int32_t> max(const Vec<W, int32_t>& a, const Vec<W, int32_t>& b) noexcept { return max_i32(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int64_t> max(const Vec<W, int64_t>& a, const Vec<W, int64_t>& b) noexcept { return max_i64(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint8_t> max(const Vec<W, uint8_t>& a, const Vec<W, uint8_t>& b) noexcept { return max_u8(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint16_t> max(const Vec<W, uint16_t>& a, const Vec<W, uint16_t>& b) noexcept { return max_u16(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint32_t> max(const Vec<W, uint32_t>& a, const Vec<W, uint32_t>& b) noexcept { return max_u32(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint64_t> max(const Vec<W, uint64_t>& a, const Vec<W, uint64_t>& b) noexcept { return max_u64(a, b); }

template<size_t W> BL_INLINE_NODEBUG Vec<W, int8_t> smin(const Vec<W, int8_t>& a, const Vec<W, int8_t>& b) noexcept { return min_i8(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> smin(const Vec<W, int16_t>& a, const Vec<W, int16_t>& b) noexcept { return min_i16(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int32_t> smin(const Vec<W, int32_t>& a, const Vec<W, int32_t>& b) noexcept { return min_i32(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int64_t> smin(const Vec<W, int64_t>& a, const Vec<W, int64_t>& b) noexcept { return min_i64(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint8_t> smin(const Vec<W, uint8_t>& a, const Vec<W, uint8_t>& b) noexcept { return min_i8(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint16_t> smin(const Vec<W, uint16_t>& a, const Vec<W, uint16_t>& b) noexcept { return min_i16(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint32_t> smin(const Vec<W, uint32_t>& a, const Vec<W, uint32_t>& b) noexcept { return min_i32(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint64_t> smin(const Vec<W, uint64_t>& a, const Vec<W, uint64_t>& b) noexcept { return min_i64(a, b); }

template<size_t W> BL_INLINE_NODEBUG Vec<W, int8_t> smax(const Vec<W, int8_t>& a, const Vec<W, int8_t>& b) noexcept { return max_i8(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> smax(const Vec<W, int16_t>& a, const Vec<W, int16_t>& b) noexcept { return max_i16(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int32_t> smax(const Vec<W, int32_t>& a, const Vec<W, int32_t>& b) noexcept { return max_i32(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int64_t> smax(const Vec<W, int64_t>& a, const Vec<W, int64_t>& b) noexcept { return max_i64(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint8_t> smax(const Vec<W, uint8_t>& a, const Vec<W, uint8_t>& b) noexcept { return max_i8(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint16_t> smax(const Vec<W, uint16_t>& a, const Vec<W, uint16_t>& b) noexcept { return max_i16(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint32_t> smax(const Vec<W, uint32_t>& a, const Vec<W, uint32_t>& b) noexcept { return max_i32(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint64_t> smax(const Vec<W, uint64_t>& a, const Vec<W, uint64_t>& b) noexcept { return max_i64(a, b); }

template<size_t W> BL_INLINE_NODEBUG Vec<W, int8_t> umin(const Vec<W, int8_t>& a, const Vec<W, int8_t>& b) noexcept { return min_u8(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> umin(const Vec<W, int16_t>& a, const Vec<W, int16_t>& b) noexcept { return min_u16(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int32_t> umin(const Vec<W, int32_t>& a, const Vec<W, int32_t>& b) noexcept { return min_u32(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int64_t> umin(const Vec<W, int64_t>& a, const Vec<W, int64_t>& b) noexcept { return min_u64(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint8_t> umin(const Vec<W, uint8_t>& a, const Vec<W, uint8_t>& b) noexcept { return min_u8(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint16_t> umin(const Vec<W, uint16_t>& a, const Vec<W, uint16_t>& b) noexcept { return min_u16(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint32_t> umin(const Vec<W, uint32_t>& a, const Vec<W, uint32_t>& b) noexcept { return min_u32(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint64_t> umin(const Vec<W, uint64_t>& a, const Vec<W, uint64_t>& b) noexcept { return min_u64(a, b); }

template<size_t W> BL_INLINE_NODEBUG Vec<W, int8_t> umax(const Vec<W, int8_t>& a, const Vec<W, int8_t>& b) noexcept { return max_u8(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> umax(const Vec<W, int16_t>& a, const Vec<W, int16_t>& b) noexcept { return max_u16(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int32_t> umax(const Vec<W, int32_t>& a, const Vec<W, int32_t>& b) noexcept { return max_u32(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int64_t> umax(const Vec<W, int64_t>& a, const Vec<W, int64_t>& b) noexcept { return max_u64(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint8_t> umax(const Vec<W, uint8_t>& a, const Vec<W, uint8_t>& b) noexcept { return max_u8(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint16_t> umax(const Vec<W, uint16_t>& a, const Vec<W, uint16_t>& b) noexcept { return max_u16(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint32_t> umax(const Vec<W, uint32_t>& a, const Vec<W, uint32_t>& b) noexcept { return max_u32(a, b); }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint64_t> umax(const Vec<W, uint64_t>& a, const Vec<W, uint64_t>& b) noexcept { return max_u64(a, b); }

template<uint32_t kN, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> slli_i8(const Vec<W, T>& a) noexcept { return vec_wt<W, T>(I::simd_slli_i8<kN>(simd_i8(a.v))); }
template<uint32_t kN, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> slli_i16(const Vec<W, T>& a) noexcept { return vec_wt<W, T>(I::simd_slli_i16<kN>(simd_i16(a.v))); }
template<uint32_t kN, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> slli_i32(const Vec<W, T>& a) noexcept { return vec_wt<W, T>(I::simd_slli_i32<kN>(simd_i32(a.v))); }
template<uint32_t kN, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> slli_i64(const Vec<W, T>& a) noexcept { return vec_wt<W, T>(I::simd_slli_i64<kN>(simd_i64(a.v))); }

template<uint32_t kN, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> slli_u8(const Vec<W, T>& a) noexcept { return vec_wt<W, T>(I::simd_slli_u8<kN>(simd_u8(a.v))); }
template<uint32_t kN, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> slli_u16(const Vec<W, T>& a) noexcept { return vec_wt<W, T>(I::simd_slli_u16<kN>(simd_u16(a.v))); }
template<uint32_t kN, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> slli_u32(const Vec<W, T>& a) noexcept { return vec_wt<W, T>(I::simd_slli_u32<kN>(simd_u32(a.v))); }
template<uint32_t kN, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> slli_u64(const Vec<W, T>& a) noexcept { return vec_wt<W, T>(I::simd_slli_u64<kN>(simd_u64(a.v))); }

template<uint32_t kN, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> srli_u8(const Vec<W, T>& a) noexcept { return vec_wt<W, T>(I::simd_srli_u8<kN>(simd_u8(a.v))); }
template<uint32_t kN, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> srli_u16(const Vec<W, T>& a) noexcept { return vec_wt<W, T>(I::simd_srli_u16<kN>(simd_u16(a.v))); }
template<uint32_t kN, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> srli_u32(const Vec<W, T>& a) noexcept { return vec_wt<W, T>(I::simd_srli_u32<kN>(simd_u32(a.v))); }
template<uint32_t kN, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> srli_u64(const Vec<W, T>& a) noexcept { return vec_wt<W, T>(I::simd_srli_u64<kN>(simd_u64(a.v))); }

template<uint32_t kN, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> rsrli_u8(const Vec<W, T>& a) noexcept { return vec_wt<W, T>(I::simd_rsrli_u8<kN>(simd_u8(a.v))); }
template<uint32_t kN, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> rsrli_u16(const Vec<W, T>& a) noexcept { return vec_wt<W, T>(I::simd_rsrli_u16<kN>(simd_u16(a.v))); }
template<uint32_t kN, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> rsrli_u32(const Vec<W, T>& a) noexcept { return vec_wt<W, T>(I::simd_rsrli_u32<kN>(simd_u32(a.v))); }
template<uint32_t kN, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> rsrli_u64(const Vec<W, T>& a) noexcept { return vec_wt<W, T>(I::simd_rsrli_u64<kN>(simd_u64(a.v))); }

template<uint32_t kN, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> acc_rsrli_u8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_acc_rsrli_u8<kN>(simd_u8(a.v), simd_u8(b.v))); }
template<uint32_t kN, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> acc_rsrli_u16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_acc_rsrli_u16<kN>(simd_u16(a.v), simd_u16(b.v))); }
template<uint32_t kN, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> acc_rsrli_u32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_acc_rsrli_u32<kN>(simd_u32(a.v), simd_u32(b.v))); }
template<uint32_t kN, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> acc_rsrli_u64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_acc_rsrli_u64<kN>(simd_u64(a.v), simd_u64(b.v))); }

template<uint32_t kN, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> srai_i8(const Vec<W, T>& a) noexcept { return vec_wt<W, T>(I::simd_srai_i8<kN>(simd_i8(a.v))); }
template<uint32_t kN, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> srai_i16(const Vec<W, T>& a) noexcept { return vec_wt<W, T>(I::simd_srai_i16<kN>(simd_i16(a.v))); }
template<uint32_t kN, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> srai_i32(const Vec<W, T>& a) noexcept { return vec_wt<W, T>(I::simd_srai_i32<kN>(simd_i32(a.v))); }
template<uint32_t kN, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> srai_i64(const Vec<W, T>& a) noexcept { return vec_wt<W, T>(I::simd_srai_i64<kN>(simd_i64(a.v))); }

template<uint32_t kN, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> sllb_u128(const Vec<W, T>& a) noexcept { return vec_wt<W, T>(I::simd_sllb_u128<kN>(simd_u8(a.v))); }
template<uint32_t kN, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> srlb_u128(const Vec<W, T>& a) noexcept { return vec_wt<W, T>(I::simd_srlb_u128<kN>(simd_u8(a.v))); }

template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, int8_t> slli(const Vec<W, int8_t>& a) noexcept { return slli_i8<kN>(a); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> slli(const Vec<W, int16_t>& a) noexcept { return slli_i16<kN>(a); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, int32_t> slli(const Vec<W, int32_t>& a) noexcept { return slli_i32<kN>(a); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, int64_t> slli(const Vec<W, int64_t>& a) noexcept { return slli_i64<kN>(a); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, uint8_t> slli(const Vec<W, uint8_t>& a) noexcept { return slli_i8<kN>(a); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, uint16_t> slli(const Vec<W, uint16_t>& a) noexcept { return slli_i16<kN>(a); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, uint32_t> slli(const Vec<W, uint32_t>& a) noexcept { return slli_i32<kN>(a); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, uint64_t> slli(const Vec<W, uint64_t>& a) noexcept { return slli_i64<kN>(a); }

template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, int8_t> srli(const Vec<W, int8_t>& a) noexcept { return srli_u8<kN>(a); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> srli(const Vec<W, int16_t>& a) noexcept { return srli_u16<kN>(a); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, int32_t> srli(const Vec<W, int32_t>& a) noexcept { return srli_u32<kN>(a); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, int64_t> srli(const Vec<W, int64_t>& a) noexcept { return srli_u64<kN>(a); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, uint8_t> srli(const Vec<W, uint8_t>& a) noexcept { return srli_u8<kN>(a); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, uint16_t> srli(const Vec<W, uint16_t>& a) noexcept { return srli_u16<kN>(a); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, uint32_t> srli(const Vec<W, uint32_t>& a) noexcept { return srli_u32<kN>(a); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, uint64_t> srli(const Vec<W, uint64_t>& a) noexcept { return srli_u64<kN>(a); }

template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, int8_t> rsrli(const Vec<W, int8_t>& a) noexcept { return rsrli_u8<kN>(a); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> rsrli(const Vec<W, int16_t>& a) noexcept { return rsrli_u16<kN>(a); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, int32_t> rsrli(const Vec<W, int32_t>& a) noexcept { return rsrli_u32<kN>(a); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, int64_t> rsrli(const Vec<W, int64_t>& a) noexcept { return rsrli_u64<kN>(a); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, uint8_t> rsrli(const Vec<W, uint8_t>& a) noexcept { return rsrli_u8<kN>(a); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, uint16_t> rsrli(const Vec<W, uint16_t>& a) noexcept { return rsrli_u16<kN>(a); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, uint32_t> rsrli(const Vec<W, uint32_t>& a) noexcept { return rsrli_u32<kN>(a); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, uint64_t> rsrli(const Vec<W, uint64_t>& a) noexcept { return rsrli_u64<kN>(a); }

template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, int8_t> srai(const Vec<W, int8_t>& a) noexcept { return srai_i8<kN>(a); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> srai(const Vec<W, int16_t>& a) noexcept { return srai_i16<kN>(a); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, int32_t> srai(const Vec<W, int32_t>& a) noexcept { return srai_i32<kN>(a); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, int64_t> srai(const Vec<W, int64_t>& a) noexcept { return srai_i64<kN>(a); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, uint8_t> srai(const Vec<W, uint8_t>& a) noexcept { return srai_i8<kN>(a); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, uint16_t> srai(const Vec<W, uint16_t>& a) noexcept { return srai_i16<kN>(a); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, uint32_t> srai(const Vec<W, uint32_t>& a) noexcept { return srai_i32<kN>(a); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, uint64_t> srai(const Vec<W, uint64_t>& a) noexcept { return srai_i64<kN>(a); }

template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, int8_t> acc_rsrli(const Vec<W, int8_t>& a, const Vec<W, int8_t>& b) noexcept { return acc_rsrli_u8<kN>(a, b); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> acc_rsrli(const Vec<W, int16_t>& a, const Vec<W, int16_t>& b) noexcept { return acc_rsrli_u16<kN>(a, b); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, int32_t> acc_rsrli(const Vec<W, int32_t>& a, const Vec<W, int32_t>& b) noexcept { return acc_rsrli_u32<kN>(a, b); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, int64_t> acc_rsrli(const Vec<W, int64_t>& a, const Vec<W, int64_t>& b) noexcept { return acc_rsrli_u64<kN>(a, b); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, uint8_t> acc_rsrli(const Vec<W, uint8_t>& a, const Vec<W, uint8_t>& b) noexcept { return acc_rsrli_u8<kN>(a, b); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, uint16_t> acc_rsrli(const Vec<W, uint16_t>& a, const Vec<W, uint16_t>& b) noexcept { return acc_rsrli_u16<kN>(a, b); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, uint32_t> acc_rsrli(const Vec<W, uint32_t>& a, const Vec<W, uint32_t>& b) noexcept { return acc_rsrli_u32<kN>(a, b); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, uint64_t> acc_rsrli(const Vec<W, uint64_t>& a, const Vec<W, uint64_t>& b) noexcept { return acc_rsrli_u64<kN>(a, b); }

#if defined(BL_TARGET_OPT_ASIMD_CRYPTO)
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> clmul_u128_ll(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_clmul_u128_ll(simd_u64(a.v), simd_u64(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> clmul_u128_lh(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_clmul_u128_lh(simd_u64(a.v), simd_u64(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> clmul_u128_hl(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_clmul_u128_hl(simd_u64(a.v), simd_u64(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> clmul_u128_hh(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return vec_wt<W, T>(I::simd_clmul_u128_hh(simd_u64(a.v), simd_u64(b.v))); }
#endif // BL_TARGET_OPT_ASIMD_CRYPTO

// SIMD - Public - Overloaded Operators
// ====================================

// Overloaded operators won't compile if they are behind an anonymous namespace.
} // {anonymous}

template<size_t W, typename T> static BL_INLINE_NODEBUG Vec<W, T> operator&(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return and_(a, b); }
template<size_t W, typename T> static BL_INLINE_NODEBUG Vec<W, T> operator|(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return or_(a, b); }
template<size_t W, typename T> static BL_INLINE_NODEBUG Vec<W, T> operator^(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return xor_(a, b); }
template<size_t W, typename T> static BL_INLINE_NODEBUG Vec<W, T> operator+(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return add(a, b); }
template<size_t W, typename T> static BL_INLINE_NODEBUG Vec<W, T> operator-(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return sub(a, b); }
template<size_t W, typename T> static BL_INLINE_NODEBUG Vec<W, T> operator*(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return mul(a, b); }
template<size_t W, typename T> static BL_INLINE_NODEBUG Vec<W, T> operator/(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return div(a, b); }
template<size_t W, typename T> static BL_INLINE_NODEBUG Vec<W, T>& operator&=(Vec<W, T>& a, const Vec<W, T>& b) noexcept { a = and_(a, b); return a; }
template<size_t W, typename T> static BL_INLINE_NODEBUG Vec<W, T>& operator|=(Vec<W, T>& a, const Vec<W, T>& b) noexcept { a = or_(a, b); return a; }
template<size_t W, typename T> static BL_INLINE_NODEBUG Vec<W, T>& operator^=(Vec<W, T>& a, const Vec<W, T>& b) noexcept { a = xor_(a, b); return a; }
template<size_t W, typename T> static BL_INLINE_NODEBUG Vec<W, T>& operator+=(Vec<W, T>& a, const Vec<W, T>& b) noexcept { a = add(a, b); return a; }
template<size_t W, typename T> static BL_INLINE_NODEBUG Vec<W, T>& operator-=(Vec<W, T>& a, const Vec<W, T>& b) noexcept { a = sub(a, b); return a; }
template<size_t W, typename T> static BL_INLINE_NODEBUG Vec<W, T>& operator*=(Vec<W, T>& a, const Vec<W, T>& b) noexcept { a = mul(a, b); return a; }
template<size_t W, typename T> static BL_INLINE_NODEBUG Vec<W, T>& operator/=(Vec<W, T>& a, const Vec<W, T>& b) noexcept { a = div(a, b); return a; }

template<size_t W, typename T, uint32_t kN> static BL_INLINE_NODEBUG Vec<W, T> operator<<(const Vec<W, T>& a, Shift<kN>) noexcept { return slli<kN>(a); }
template<size_t W, typename T, uint32_t kN> static BL_INLINE_NODEBUG Vec<W, T> operator>>(const Vec<W, T>& a, Shift<kN>) noexcept { return std::is_unsigned_v<T> ? srli<kN>(a) : srai<kN>(a); }

template<size_t W, typename T, uint32_t kN> static BL_INLINE_NODEBUG Vec<W, T>& operator<<=(Vec<W, T>& a, Shift<kN> b) noexcept { a = a << b; return a; }
template<size_t W, typename T, uint32_t kN> static BL_INLINE_NODEBUG Vec<W, T>& operator>>=(Vec<W, T>& a, Shift<kN> b) noexcept { a = a >> b; return a; }

namespace {

// SIMD - Public - Workarounds
// ===========================

// TODO: These need a proper abstraction in Internal namespace.

#if defined(BL_SIMD_AARCH64)
BL_INLINE_NODEBUG Vec2xF64 cvt_2xi32_f64(const Vec4xI32& a) noexcept {
  return Vec2xF64{vcvtq_f64_s64(I::simd_unpack_lo64_i32_i64(simd_i32(a.v)))};
}
#endif // BL_SIMD_AARCH64

// SIMD - Public - Utilities - Div255 & Div65535
// =============================================

template<typename V>
BL_INLINE V div255_u16(const V& a) noexcept { return rsrli_u16<8>(acc_rsrli_u16<8>(a, a)); }

template<typename V>
BL_INLINE V div65535_u32(const V& a) noexcept { return rsrli_u32<16>(acc_rsrli_u32<16>(a, a)); }

// SIMD - Public - Extract MSB
// ===========================

template<typename V>
BL_INLINE uint32_t extract_mask_bits_i8(const Vec<16, V>& a) noexcept {
  Vec16xU8 bm = make128_u8(0x80u, 0x40u, 0x20u, 0x10u, 0x08u, 0x04u, 0x02u, 0x01u);
  Vec16xU8 m0 = and_(vec_cast<Vec16xU8>(a), bm);

#if defined(BL_SIMD_AARCH64)
  uint8x16_t acc = vpaddq_u8(m0.v, m0.v);
  acc = vpaddq_u8(acc, acc);
  acc = vpaddq_u8(acc, acc);
  return vgetq_lane_u16(vreinterpretq_u16_u8(acc), 0);
#else
  uint8x8_t acc = vpadd_u8(vget_low_u8(m0.v), vget_high_u8(m0.v));
  acc = vpadd_u8(acc, acc);
  acc = vpadd_u8(acc, acc);
  return vget_lane_u16(vreinterpret_u16_u8(acc), 0);
#endif
}

template<typename V>
BL_INLINE uint32_t extract_mask_bits_i8(const Vec<16, V>& a, const Vec<16, V>& b) noexcept {
  Vec16xU8 bm = make128_u8(0x80u, 0x40u, 0x20u, 0x10u, 0x08u, 0x04u, 0x02u, 0x01u);
  Vec16xU8 m0 = and_(vec_cast<Vec16xU8>(a), bm);
  Vec16xU8 m1 = and_(vec_cast<Vec16xU8>(b), bm);

#if defined(BL_SIMD_AARCH64)
  uint8x16_t acc = vpaddq_u8(m0.v, m1.v);
  acc = vpaddq_u8(acc, acc);
  acc = vpaddq_u8(acc, acc);
  return vgetq_lane_u32(vreinterpretq_u32_u8(acc), 0);
#else
  uint8x8_t acc0 = vpadd_u8(vget_low_u8(m0.v), vget_high_u8(m0.v));
  uint8x8_t acc1 = vpadd_u8(vget_low_u8(m1.v), vget_high_u8(m1.v));
  acc0 = vpadd_u8(acc0, acc1);
  acc0 = vpadd_u8(acc0, acc0);
  return vget_lane_u32(vreinterpret_u32_u8(acc0), 0);
#endif
}

#if defined(BL_SIMD_AARCH64)
template<typename V>
BL_INLINE uint64_t extract_mask_bits_i8(const Vec<16, V>& a, const Vec<16, V>& b, const Vec<16, V>& c, const Vec<16, V>& d) noexcept {
  Vec16xU8 bm = make128_u8(0x80u, 0x40u, 0x20u, 0x10u, 0x08u, 0x04u, 0x02u, 0x01u);
  Vec16xU8 m0 = and_(vec_cast<Vec16xU8>(a), bm);
  Vec16xU8 m1 = and_(vec_cast<Vec16xU8>(b), bm);
  Vec16xU8 m2 = and_(vec_cast<Vec16xU8>(c), bm);
  Vec16xU8 m3 = and_(vec_cast<Vec16xU8>(d), bm);

  uint8x16_t acc0 = vpaddq_u8(m0.v, m1.v);
  uint8x16_t acc1 = vpaddq_u8(m2.v, m3.v);
  acc0 = vpaddq_u8(acc0, acc1);
  acc0 = vpaddq_u8(acc0, acc0);

  return vgetq_lane_u64(vreinterpretq_u64_u8(acc0), 0);
}
#endif

// SIMD - Public - Utilities - Array Lookup
// ========================================

#if defined(BL_SIMD_AARCH64)
// Array lookup is based on the following article:
//   https://community.arm.com/arm-community-blogs/b/infrastructure-solutions-blog/posts/porting-x86-vector-bitmask-optimizations-to-arm-neon
template<uint32_t kN>
struct ArrayLookupResult {
  enum : uint32_t {
    kIndexShift = (kN == 4u) ? 4u :
                  (kN == 8u) ? 3u : 2u
  };

  enum : uint64_t {
    kInputMask = kIndexShift == 4 ? 0x0001000100010001u :
                 kIndexShift == 3 ? 0x0101010101010101u :
                 kIndexShift == 2 ? 0x1111111111111111u :
                 kIndexShift == 1 ? 0x5555555555555555u : 0xFFFFFFFFFFFFFFFFu
  };

  uint64_t _mask;

  BL_INLINE_NODEBUG bool matched() const noexcept { return _mask != 0; }
  BL_INLINE_NODEBUG uint32_t index() const noexcept { return (63 - bl::IntOps::clz(_mask)) >> kIndexShift; }

  using Iterator = bl::ParametrizedBitOps<bl::BitOrder::kLSB, uint64_t>::BitChunkIterator<kIndexShift>;
  BL_INLINE_NODEBUG Iterator iterate() const noexcept { return Iterator(static_cast<uint64_t>(_mask & 0x1111111111111111)); }
};

BL_INLINE_NODEBUG ArrayLookupResult<4> array_lookup_result_from_4x_u32(Vec4xU32 pred) noexcept {
  uint64_t mask = vget_lane_u64(simd_u64(vshrn_n_u64(simd_u64(pred.v), 16)), 0);
  return ArrayLookupResult<4>{mask};
}

BL_INLINE_NODEBUG ArrayLookupResult<8> array_lookup_result_from_8x_u16(Vec8xU16 pred) noexcept {
  uint64_t mask = vget_lane_u64(simd_u64(vshrn_n_u32(simd_u32(pred.v), 8)), 0);
  return ArrayLookupResult<8>{mask};
}

BL_INLINE_NODEBUG ArrayLookupResult<16> array_lookup_result_from_16x_u8(Vec16xU8 pred) noexcept {
  uint64_t mask = vget_lane_u64(simd_u64(vshrn_n_u16(simd_u16(pred.v), 4)), 0);
  return ArrayLookupResult<16>{mask};
}

template<uint32_t kN>
BL_INLINE_NODEBUG ArrayLookupResult<kN> array_lookup_u32_eq_aligned16(const uint32_t* array, uint32_t value) noexcept;

template<>
BL_INLINE_NODEBUG ArrayLookupResult<4> array_lookup_u32_eq_aligned16<4>(const uint32_t* array, uint32_t value) noexcept {
  Vec4xU32 v = make128_u32(value);
  return array_lookup_result_from_4x_u32(
    cmp_eq_u32(loada<Vec4xU32>(array), v));
}

template<>
BL_INLINE_NODEBUG ArrayLookupResult<8> array_lookup_u32_eq_aligned16<8>(const uint32_t* array, uint32_t value) noexcept {
  Vec4xU32 v = make128_u32(value);
  Vec4xU32 pred0 = cmp_eq_u32(loada<Vec4xU32>(array + 0), v);
  Vec4xU32 pred1 = cmp_eq_u32(loada<Vec4xU32>(array + 4), v);

  uint32x4_t combined = vcombine_u32(vshrn_n_u64(simd_u64(pred0.v), 16), vshrn_n_u64(simd_u64(pred1.v), 16));
  return array_lookup_result_from_8x_u16(Vec8xU16{simd_u16(combined)});
}

template<>
BL_INLINE_NODEBUG ArrayLookupResult<16> array_lookup_u32_eq_aligned16<16>(const uint32_t* array, uint32_t value) noexcept {
  Vec4xU32 v = make128_u32(value);
  Vec4xU32 pred0 = cmp_eq_u32(loada<Vec4xU32>(array + 0), v);
  Vec4xU32 pred1 = cmp_eq_u32(loada<Vec4xU32>(array + 4), v);
  Vec4xU32 pred2 = cmp_eq_u32(loada<Vec4xU32>(array + 8), v);
  Vec4xU32 pred3 = cmp_eq_u32(loada<Vec4xU32>(array + 12), v);

  uint32x4_t combined0 = vcombine_u32(vshrn_n_u64(simd_u64(pred0.v), 16), vshrn_n_u64(simd_u64(pred1.v), 16));
  uint32x4_t combined1 = vcombine_u32(vshrn_n_u64(simd_u64(pred2.v), 16), vshrn_n_u64(simd_u64(pred3.v), 16));
  uint16x8_t combined = vcombine_u16(vshrn_n_u32(combined0, 8), vshrn_n_u32(combined1, 8));
  return array_lookup_result_from_16x_u8(Vec16xU8{simd_u8(combined)});
}
#endif // BL_SIMD_AARCH64

} // {anonymous}
} // {SIMD}

#undef I
#undef BL_SIMD_AARCH64

//! \}
//! \endcond

#endif // BLEND2D_SIMD_SIMDARM_P_H_INCLUDED
