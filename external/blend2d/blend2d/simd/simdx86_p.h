// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_SIMD_SIMDX86_P_H_INCLUDED
#define BLEND2D_SIMD_SIMDX86_P_H_INCLUDED

#include <blend2d/simd/simdbase_p.h>
#include <blend2d/support/bitops_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/memops_p.h>
#include <blend2d/tables/tables_p.h>

#if defined(_MSC_VER)
  #include <intrin.h>
#endif

#if defined(BL_TARGET_OPT_SSE)
  #include <xmmintrin.h>
#endif

#if defined(BL_TARGET_OPT_SSE2)
  #include <emmintrin.h>
#endif

#if defined(BL_TARGET_OPT_SSE3) && !defined(_MSC_VER)
  #include <pmmintrin.h>
#endif

#if defined(BL_TARGET_OPT_SSSE3)
  #include <tmmintrin.h>
#endif

#if defined(BL_TARGET_OPT_SSE4_1)
  #include <smmintrin.h>
#endif

#if defined(BL_TARGET_OPT_SSE4_2)
  #include <nmmintrin.h>
  #include <wmmintrin.h>
#endif

#if defined(BL_TARGET_OPT_AVX) || defined(BL_TARGET_OPT_AVX2) || defined(BL_TARGET_OPT_AVX512)
  #include <immintrin.h>
#endif

#ifdef min
  #undef min
#endif

#ifdef max
  #undef max
#endif

#if defined(__GNUC__) && !defined(__clang__) && __GNUC__ < 11
  #define BL_SIMD_MISSING_INTRINSICS
#endif

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

// The reason for doing so is to make the public interface using SIMD::Internal easier to read.
#define I Internal

// SIMD - Register Widths
// ======================

#if defined(BL_TARGET_OPT_AVX512)
  #define BL_SIMD_WIDTH_I 512
  #define BL_SIMD_WIDTH_F 512
  #define BL_SIMD_WIDTH_D 512
#elif defined(BL_TARGET_OPT_AVX2)
  #define BL_SIMD_WIDTH_I 256
  #define BL_SIMD_WIDTH_F 256
  #define BL_SIMD_WIDTH_D 256
#elif defined(BL_TARGET_OPT_AVX)
  #define BL_SIMD_WIDTH_I 128
  #define BL_SIMD_WIDTH_F 256
  #define BL_SIMD_WIDTH_D 256
#else
  #define BL_SIMD_WIDTH_I 128
  #define BL_SIMD_WIDTH_F 128
  #define BL_SIMD_WIDTH_D 128
#endif

// SIMD - Features
// ===============

// Features describe the availability of some SIMD instructions that are not emulated if not available.

#if defined(BL_TARGET_OPT_AVX512)
  #define BL_SIMD_FEATURE_TERNLOG
#endif // BL_TARGET_OPT_AVX512

#if defined(BL_TARGET_OPT_AVX2)
  #define BL_SIMD_FEATURE_MOVW
#endif // BL_TARGET_OPT_AVX2

#if defined(BL_TARGET_OPT_SSE4_1)
  #define BL_SIMD_FEATURE_BLEND_IMM
#endif // BL_TARGET_OPT_SSE4_1

#if defined(BL_TARGET_OPT_SSSE3)
  #define BL_SIMD_FEATURE_SWIZZLEV_U8
#endif // BL_TARGET_OPT_SSSE3

// SIMD - Cost Tables
// ==================

// NOTE: Cost in general tells us how complex is to emulate the given instruction in terms of other instructions.
// In general it doesn't tell us the latency as that depends on a micro-architecture, which we don't specialize
// for here. So, if the cost is 1 it means a single instruction can do the operation, if it's 3 it means that 3
// instructions are needed. The instruction cost doesn't include moves, which would only be present when targeting
// pre-AVX hardware.

#if defined(BL_TARGET_OPT_SSE2)
  #define BL_SIMD_COST_MIN_MAX_U8    1  // native
  #define BL_SIMD_COST_MIN_MAX_I16   1  // native
  #define BL_SIMD_COST_MUL_I16       1  // native
#endif

#if defined(BL_TARGET_OPT_SSSE3)
  #define BL_SIMD_COST_ABS_I8        1  // native
  #define BL_SIMD_COST_ABS_I16       1  // native
  #define BL_SIMD_COST_ABS_I32       1  // native
  #define BL_SIMD_COST_ALIGNR_U8     1  // native
#else
  #define BL_SIMD_COST_ABS_I8        2  // emulated ('sub_i8' + 'min_u8')
  #define BL_SIMD_COST_ABS_I16       2  // emulated ('sub_i16' + 'max_i16')
  #define BL_SIMD_COST_ABS_I32       3  // emulated ('srai_i32' + 'sub_i32' + 'xor')
  #define BL_SIMD_COST_ALIGNR_U8     3  // emulated ('srlb' + 'sllb' + 'or')
#endif

#if defined(BL_TARGET_OPT_AVX512)
  #define BL_SIMD_COST_ABS_I64       1  // native
  #define BL_SIMD_COST_CMP_EQ_I64    1  // native
  #define BL_SIMD_COST_CMP_LT_GT_I64 1  // native
  #define BL_SIMD_COST_CMP_LE_GE_I64 1  // native
  #define BL_SIMD_COST_CMP_LT_GT_U64 1  // native
  #define BL_SIMD_COST_CMP_LE_GE_U64 1  // native
  #define BL_SIMD_COST_MIN_MAX_I8    1  // native
  #define BL_SIMD_COST_MIN_MAX_U16   1  // native
  #define BL_SIMD_COST_MIN_MAX_I32   1  // native
  #define BL_SIMD_COST_MIN_MAX_U32   1  // native
  #define BL_SIMD_COST_MIN_MAX_I64   1  // native
  #define BL_SIMD_COST_MIN_MAX_U64   1  // native
  #define BL_SIMD_COST_MUL_I32       1  // native
  #define BL_SIMD_COST_MUL_I64       1  // native
#elif defined(BL_TARGET_OPT_SSE4_2)
  #define BL_SIMD_COST_ABS_I64       4  // emulated (complex)
  #define BL_SIMD_COST_CMP_EQ_I64    1  // native
  #define BL_SIMD_COST_CMP_LT_GT_I64 1  // native
  #define BL_SIMD_COST_CMP_LE_GE_I64 2  // emulated ('cmp_lt_gt_i64' + 'not')
  #define BL_SIMD_COST_CMP_LT_GT_U64 3  // emulated ('cmp_lt_gt_i64' + 2x'xor')
  #define BL_SIMD_COST_CMP_LE_GE_U64 4  // emulated ('cmp_lt_gt_u64' + 'not')
  #define BL_SIMD_COST_MIN_MAX_I8    1  // native
  #define BL_SIMD_COST_MIN_MAX_U16   1  // native
  #define BL_SIMD_COST_MIN_MAX_I32   1  // native
  #define BL_SIMD_COST_MIN_MAX_U32   1  // native
  #define BL_SIMD_COST_MIN_MAX_I64   2  // emulated ('cmp_lt_gt_i64' + 'blend')
  #define BL_SIMD_COST_MIN_MAX_U64   4  // emulated ('cmp_lt_gt_u64' + 'blend')
  #define BL_SIMD_COST_MUL_I32       1  // native
  #define BL_SIMD_COST_MUL_I64       7  // emulated (complex)
#elif defined(BL_TARGET_OPT_SSE4_1)
  #define BL_SIMD_COST_ABS_I64       4  // emulated (complex)
  #define BL_SIMD_COST_CMP_EQ_I64    1  // native
  #define BL_SIMD_COST_CMP_LT_GT_I64 6  // emulated (complex)
  #define BL_SIMD_COST_CMP_LE_GE_I64 7  // emulated ('cmp_lt_gt_i64' + 'not')
  #define BL_SIMD_COST_CMP_LT_GT_U64 8  // emulated ('cmp_lt_gt_i64' + 2x'xor')
  #define BL_SIMD_COST_CMP_LE_GE_U64 9  // emulated ('cmp_lt_gt_u64' + 'not')
  #define BL_SIMD_COST_MIN_MAX_I8    1  // native
  #define BL_SIMD_COST_MIN_MAX_U16   1  // native
  #define BL_SIMD_COST_MIN_MAX_I32   1  // native
  #define BL_SIMD_COST_MIN_MAX_U32   1  // native
  #define BL_SIMD_COST_MIN_MAX_I64   7  // emulated ('cmp_lt_gt_i64' + 'blend')
  #define BL_SIMD_COST_MIN_MAX_U64   9  // emulated ('cmp_lt_gt_u64' + 'blend')
  #define BL_SIMD_COST_MUL_I32       1  // native
  #define BL_SIMD_COST_MUL_I64       7  // emulated (complex)
#else
  #define BL_SIMD_COST_ABS_I64       4  // emulated (complex)
  #define BL_SIMD_COST_CMP_EQ_I64    3  // emulated ('cmp_eq_i32' + 'shuffle_i32' + 'and')
  #define BL_SIMD_COST_CMP_LT_GT_I64 6  // emulated (complex)
  #define BL_SIMD_COST_CMP_LE_GE_I64 7  // emulated ('cmp_lt_gt_i64' + 'not')
  #define BL_SIMD_COST_CMP_LT_GT_U64 8  // emulated ('cmp_lt_gt_i64' + 2x'xor')
  #define BL_SIMD_COST_CMP_LE_GE_U64 9  // emulated ('cmp_lt_gt_u64' + 'not')
  #define BL_SIMD_COST_MIN_MAX_I8    4  // emulated ('cmp_lt_gt_i8' + 'blend')
  #define BL_SIMD_COST_MIN_MAX_U16   2  // emulated ('sub_u16' and 'subs_u16')
  #define BL_SIMD_COST_MIN_MAX_I32   4  // emulated ('cmp_lt_gt_i8' + 'blend')
  #define BL_SIMD_COST_MIN_MAX_U32   6  // emulated ('cmp_lt_gt_u32' + 'blend')
  #define BL_SIMD_COST_MIN_MAX_I64   9  // emulated ('cmp_lt_gt_i64' + 'blend')
  #define BL_SIMD_COST_MIN_MAX_U64   11 // emulated ('cmp_lt_gt_u64' + 'blend')
  #define BL_SIMD_COST_MUL_I32       6  // emulated (complex)
  #define BL_SIMD_COST_MUL_I64       7  // emulated (complex)
#endif

// SIMD - Features
// ===============

#define BL_SIMD_FEATURE_ARRAY_LOOKUP
#define BL_SIMD_FEATURE_EXTRACT_SIGN_BITS

namespace SIMD {

// SIMD - Vector Registers
// =======================

//! \cond NEVER

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

template<> struct Vec<32, int8_t>;
template<> struct Vec<32, uint8_t>;
template<> struct Vec<32, int16_t>;
template<> struct Vec<32, uint16_t>;
template<> struct Vec<32, int32_t>;
template<> struct Vec<32, uint32_t>;
template<> struct Vec<32, int64_t>;
template<> struct Vec<32, uint64_t>;
template<> struct Vec<32, float>;
template<> struct Vec<32, double>;

template<> struct Vec<64, int8_t>;
template<> struct Vec<64, uint8_t>;
template<> struct Vec<64, int16_t>;
template<> struct Vec<64, uint16_t>;
template<> struct Vec<64, int32_t>;
template<> struct Vec<64, uint32_t>;
template<> struct Vec<64, int64_t>;
template<> struct Vec<64, uint64_t>;
template<> struct Vec<64, float>;
template<> struct Vec<64, double>;

#define BL_DECLARE_SIMD_TYPE(type_name, width, simd_type, element_type)                 \
template<>                                                                           \
struct Vec<width, element_type> {                                                     \
  static inline constexpr uint32_t kW = width;                                       \
  static inline constexpr uint32_t kHalfVectorWidth = width > 16 ? width / 2u : 16;  \
                                                                                     \
  static inline constexpr uint32_t kElementWidth = uint32_t(sizeof(element_type));    \
  static inline constexpr uint32_t kElementCount = width / kElementWidth;            \
                                                                                     \
  typedef Vec<width, element_type> VectorType;                                        \
  typedef Vec<kHalfVectorWidth, element_type> VectorHalfType;                         \
  typedef Vec<16, element_type> Vector128Type;                                        \
  typedef Vec<32, element_type> Vector256Type;                                        \
  typedef Vec<64, element_type> Vector512Type;                                        \
                                                                                     \
  typedef simd_type SimdType;                                                         \
  typedef typename VectorHalfType::SimdType HalfSimdType;                            \
                                                                                     \
  typedef element_type ElementType;                                                   \
                                                                                     \
  SimdType v;                                                                        \
};                                                                                   \
                                                                                     \
typedef Vec<width, element_type> type_name

typedef BL_UNALIGNED_TYPE(__m128i, 1) unaligned_m128i;
typedef BL_UNALIGNED_TYPE(__m128 , 1) unaligned_m128;
typedef BL_UNALIGNED_TYPE(__m128d, 1) unaligned_m128d;

BL_DECLARE_SIMD_TYPE(Vec16xI8 , 16, __m128i, int8_t);
BL_DECLARE_SIMD_TYPE(Vec16xU8 , 16, __m128i, uint8_t);
BL_DECLARE_SIMD_TYPE(Vec8xI16 , 16, __m128i, int16_t);
BL_DECLARE_SIMD_TYPE(Vec8xU16 , 16, __m128i, uint16_t);
BL_DECLARE_SIMD_TYPE(Vec4xI32 , 16, __m128i, int32_t);
BL_DECLARE_SIMD_TYPE(Vec4xU32 , 16, __m128i, uint32_t);
BL_DECLARE_SIMD_TYPE(Vec2xI64 , 16, __m128i, int64_t);
BL_DECLARE_SIMD_TYPE(Vec2xU64 , 16, __m128i, uint64_t);
BL_DECLARE_SIMD_TYPE(Vec4xF32 , 16, __m128 , float);
BL_DECLARE_SIMD_TYPE(Vec2xF64 , 16, __m128d, double);

// 256-bit types (including integers) are accessible through AVX as AVX also includes conversion instructions
// between integer types and FP types. So, we don't need AVX2 check to provide 256-bit integer vectors.
#if defined(BL_TARGET_OPT_AVX)
typedef BL_UNALIGNED_TYPE(__m256i, 1) unaligned_m256i;
typedef BL_UNALIGNED_TYPE(__m256 , 1) unaligned_m256;
typedef BL_UNALIGNED_TYPE(__m256d, 1) unaligned_m256d;

BL_DECLARE_SIMD_TYPE(Vec32xI8 , 32, __m256i, int8_t);
BL_DECLARE_SIMD_TYPE(Vec32xU8 , 32, __m256i, uint8_t);
BL_DECLARE_SIMD_TYPE(Vec16xI16, 32, __m256i, int16_t);
BL_DECLARE_SIMD_TYPE(Vec16xU16, 32, __m256i, uint16_t);
BL_DECLARE_SIMD_TYPE(Vec8xI32 , 32, __m256i, int32_t);
BL_DECLARE_SIMD_TYPE(Vec8xU32 , 32, __m256i, uint32_t);
BL_DECLARE_SIMD_TYPE(Vec4xI64 , 32, __m256i, int64_t);
BL_DECLARE_SIMD_TYPE(Vec4xU64 , 32, __m256i, uint64_t);
BL_DECLARE_SIMD_TYPE(Vec8xF32 , 32, __m256 , float);
BL_DECLARE_SIMD_TYPE(Vec4xF64 , 32, __m256d, double);
#endif // BL_TARGET_OPT_AVX

#if defined(BL_TARGET_OPT_AVX512)
typedef BL_UNALIGNED_TYPE(__m512i, 1) unaligned_m512i;
typedef BL_UNALIGNED_TYPE(__m512 , 1) unaligned_m512;
typedef BL_UNALIGNED_TYPE(__m512d, 1) unaligned_m512d;

BL_DECLARE_SIMD_TYPE(Vec64xI8 , 64, __m512i, int8_t);
BL_DECLARE_SIMD_TYPE(Vec64xU8 , 64, __m512i, uint8_t);
BL_DECLARE_SIMD_TYPE(Vec32xI16, 64, __m512i, int16_t);
BL_DECLARE_SIMD_TYPE(Vec32xU16, 64, __m512i, uint16_t);
BL_DECLARE_SIMD_TYPE(Vec16xI32, 64, __m512i, int32_t);
BL_DECLARE_SIMD_TYPE(Vec16xU32, 64, __m512i, uint32_t);
BL_DECLARE_SIMD_TYPE(Vec8xI64 , 64, __m512i, int64_t);
BL_DECLARE_SIMD_TYPE(Vec8xU64 , 64, __m512i, uint64_t);
BL_DECLARE_SIMD_TYPE(Vec16xF32, 64, __m512 , float);
BL_DECLARE_SIMD_TYPE(Vec8xF64 , 64, __m512d, double);
#endif // BL_TARGET_OPT_AVX512

#undef BL_DECLARE_SIMD_TYPE

//! \endcond

// Everything must be in anonymous namespace to avoid ODR violation.
namespace {

// SIMD - Internal - Info
// ======================

namespace Internal {

template<size_t kW>
struct SimdInfo;

//! \cond NEVER

template<>
struct SimdInfo<16> {
  typedef __m128i RegI;
  typedef __m128  RegF;
  typedef __m128d RegD;
};

#if defined(BL_TARGET_OPT_AVX)
template<>
struct SimdInfo<32> {
  typedef __m256i RegI;
  typedef __m256  RegF;
  typedef __m256d RegD;
};
#endif // BL_TARGET_OPT_AVX

#if defined(BL_TARGET_OPT_AVX512)
template<>
struct SimdInfo<64> {
  typedef __m512i RegI;
  typedef __m512  RegF;
  typedef __m512d RegD;
};
#endif // BL_TARGET_OPT_AVX512

//! \endcond

} // {Internal}

// SIMD - Internal - Cast
// ======================

// Low-level cast operation that casts a SIMD register type (used by high-level wrappers).
template<typename DstSimdT, typename SrcSimdT>
BL_INLINE_NODEBUG DstSimdT simd_cast(const SrcSimdT& src) noexcept;

BL_INLINE_NODEBUG __m128i simd_as_i(const __m128i& src) noexcept { return src; }
BL_INLINE_NODEBUG __m128i simd_as_i(const __m128&  src) noexcept { return _mm_castps_si128(src); }
BL_INLINE_NODEBUG __m128i simd_as_i(const __m128d& src) noexcept { return _mm_castpd_si128(src); }

BL_INLINE_NODEBUG __m128 simd_as_f(const __m128i& src) noexcept { return _mm_castsi128_ps(src); }
BL_INLINE_NODEBUG __m128 simd_as_f(const __m128&  src) noexcept { return src; }
BL_INLINE_NODEBUG __m128 simd_as_f(const __m128d& src) noexcept { return _mm_castpd_ps(src); }

BL_INLINE_NODEBUG __m128d simd_as_d(const __m128i& src) noexcept { return _mm_castsi128_pd(src); }
BL_INLINE_NODEBUG __m128d simd_as_d(const __m128&  src) noexcept { return _mm_castps_pd(src); }
BL_INLINE_NODEBUG __m128d simd_as_d(const __m128d& src) noexcept { return src; }

template<> BL_INLINE_NODEBUG __m128i simd_cast(const __m128i& src) noexcept { return src; }
template<> BL_INLINE_NODEBUG __m128i simd_cast(const __m128&  src) noexcept { return _mm_castps_si128(src); }
template<> BL_INLINE_NODEBUG __m128i simd_cast(const __m128d& src) noexcept { return _mm_castpd_si128(src); }

template<> BL_INLINE_NODEBUG __m128 simd_cast(const __m128i& src) noexcept { return _mm_castsi128_ps(src); }
template<> BL_INLINE_NODEBUG __m128 simd_cast(const __m128&  src) noexcept { return src; }
template<> BL_INLINE_NODEBUG __m128 simd_cast(const __m128d& src) noexcept { return _mm_castpd_ps(src); }

template<> BL_INLINE_NODEBUG __m128d simd_cast(const __m128i& src) noexcept { return _mm_castsi128_pd(src); }
template<> BL_INLINE_NODEBUG __m128d simd_cast(const __m128&  src) noexcept { return _mm_castps_pd(src); }
template<> BL_INLINE_NODEBUG __m128d simd_cast(const __m128d& src) noexcept { return src; }

#if defined(BL_TARGET_OPT_AVX)
BL_INLINE_NODEBUG __m256i simd_as_i(const __m256i& src) noexcept { return src; }
BL_INLINE_NODEBUG __m256i simd_as_i(const __m256&  src) noexcept { return _mm256_castps_si256(src); }
BL_INLINE_NODEBUG __m256i simd_as_i(const __m256d& src) noexcept { return _mm256_castpd_si256(src); }
BL_INLINE_NODEBUG __m256 simd_as_f(const __m256i& src) noexcept { return _mm256_castsi256_ps(src); }
BL_INLINE_NODEBUG __m256 simd_as_f(const __m256&  src) noexcept { return src; }
BL_INLINE_NODEBUG __m256 simd_as_f(const __m256d& src) noexcept { return _mm256_castpd_ps(src); }
BL_INLINE_NODEBUG __m256d simd_as_d(const __m256i& src) noexcept { return _mm256_castsi256_pd(src); }
BL_INLINE_NODEBUG __m256d simd_as_d(const __m256&  src) noexcept { return _mm256_castps_pd(src); }
BL_INLINE_NODEBUG __m256d simd_as_d(const __m256d& src) noexcept { return src; }

template<> BL_INLINE_NODEBUG __m128i simd_cast(const __m256i& src) noexcept { return _mm256_castsi256_si128(src); }
template<> BL_INLINE_NODEBUG __m128i simd_cast(const __m256&  src) noexcept { return _mm256_castsi256_si128(simd_as_i(src)); }
template<> BL_INLINE_NODEBUG __m128i simd_cast(const __m256d& src) noexcept { return _mm256_castsi256_si128(simd_as_i(src)); }
template<> BL_INLINE_NODEBUG __m256i simd_cast(const __m128i& src) noexcept { return _mm256_castsi128_si256(src); }
template<> BL_INLINE_NODEBUG __m256i simd_cast(const __m128&  src) noexcept { return _mm256_castsi128_si256(simd_as_i(src)); }
template<> BL_INLINE_NODEBUG __m256i simd_cast(const __m128d& src) noexcept { return _mm256_castsi128_si256(simd_as_i(src)); }
template<> BL_INLINE_NODEBUG __m256i simd_cast(const __m256i& src) noexcept { return src; }
template<> BL_INLINE_NODEBUG __m256i simd_cast(const __m256&  src) noexcept { return _mm256_castps_si256(src); }
template<> BL_INLINE_NODEBUG __m256i simd_cast(const __m256d& src) noexcept { return _mm256_castpd_si256(src); }

template<> BL_INLINE_NODEBUG __m128 simd_cast(const __m256i& src) noexcept { return _mm256_castps256_ps128(simd_as_f(src)); }
template<> BL_INLINE_NODEBUG __m128 simd_cast(const __m256&  src) noexcept { return _mm256_castps256_ps128(src); }
template<> BL_INLINE_NODEBUG __m128 simd_cast(const __m256d& src) noexcept { return _mm256_castps256_ps128(simd_as_f(src)); }
template<> BL_INLINE_NODEBUG __m256 simd_cast(const __m128i& src) noexcept { return _mm256_castps128_ps256(simd_as_f(src)); }
template<> BL_INLINE_NODEBUG __m256 simd_cast(const __m128&  src) noexcept { return _mm256_castps128_ps256(src); }
template<> BL_INLINE_NODEBUG __m256 simd_cast(const __m128d& src) noexcept { return _mm256_castps128_ps256(simd_as_f(src)); }
template<> BL_INLINE_NODEBUG __m256 simd_cast(const __m256i& src) noexcept { return _mm256_castsi256_ps(src); }
template<> BL_INLINE_NODEBUG __m256 simd_cast(const __m256&  src) noexcept { return src; }
template<> BL_INLINE_NODEBUG __m256 simd_cast(const __m256d& src) noexcept { return _mm256_castpd_ps(src); }

template<> BL_INLINE_NODEBUG __m128d simd_cast(const __m256i& src) noexcept { return _mm256_castpd256_pd128(simd_as_d(src)); }
template<> BL_INLINE_NODEBUG __m128d simd_cast(const __m256&  src) noexcept { return _mm256_castpd256_pd128(simd_as_d(src)); }
template<> BL_INLINE_NODEBUG __m128d simd_cast(const __m256d& src) noexcept { return _mm256_castpd256_pd128(src); }
template<> BL_INLINE_NODEBUG __m256d simd_cast(const __m128i& src) noexcept { return _mm256_castpd128_pd256(simd_as_d(src)); }
template<> BL_INLINE_NODEBUG __m256d simd_cast(const __m128&  src) noexcept { return _mm256_castpd128_pd256(simd_as_d(src)); }
template<> BL_INLINE_NODEBUG __m256d simd_cast(const __m128d& src) noexcept { return _mm256_castpd128_pd256(src); }
template<> BL_INLINE_NODEBUG __m256d simd_cast(const __m256i& src) noexcept { return _mm256_castsi256_pd(src); }
template<> BL_INLINE_NODEBUG __m256d simd_cast(const __m256&  src) noexcept { return _mm256_castps_pd(src); }
template<> BL_INLINE_NODEBUG __m256d simd_cast(const __m256d& src) noexcept { return src; }
#endif // BL_TARGET_OPT_AVX

#if defined(BL_TARGET_OPT_AVX512)
BL_INLINE_NODEBUG __m512i simd_as_i(const __m512i& src) noexcept { return src; }
BL_INLINE_NODEBUG __m512i simd_as_i(const __m512&  src) noexcept { return _mm512_castps_si512(src); }
BL_INLINE_NODEBUG __m512i simd_as_i(const __m512d& src) noexcept { return _mm512_castpd_si512(src); }

BL_INLINE_NODEBUG __m512 simd_as_f(const __m512i& src) noexcept { return _mm512_castsi512_ps(src); }
BL_INLINE_NODEBUG __m512 simd_as_f(const __m512&  src) noexcept { return src; }
BL_INLINE_NODEBUG __m512 simd_as_f(const __m512d& src) noexcept { return _mm512_castpd_ps(src); }

BL_INLINE_NODEBUG __m512d simd_as_d(const __m512i& src) noexcept { return _mm512_castsi512_pd(src); }
BL_INLINE_NODEBUG __m512d simd_as_d(const __m512&  src) noexcept { return _mm512_castps_pd(src); }
BL_INLINE_NODEBUG __m512d simd_as_d(const __m512d& src) noexcept { return src; }

template<> BL_INLINE_NODEBUG __m128i simd_cast(const __m512i& src) noexcept { return _mm512_castsi512_si128(src); }
template<> BL_INLINE_NODEBUG __m128i simd_cast(const __m512&  src) noexcept { return _mm512_castsi512_si128(simd_as_i(src)); }
template<> BL_INLINE_NODEBUG __m128i simd_cast(const __m512d& src) noexcept { return _mm512_castsi512_si128(simd_as_i(src)); }
template<> BL_INLINE_NODEBUG __m256i simd_cast(const __m512i& src) noexcept { return _mm512_castsi512_si256(src); }
template<> BL_INLINE_NODEBUG __m256i simd_cast(const __m512&  src) noexcept { return _mm512_castsi512_si256(simd_as_i(src)); }
template<> BL_INLINE_NODEBUG __m256i simd_cast(const __m512d& src) noexcept { return _mm512_castsi512_si256(simd_as_i(src)); }
template<> BL_INLINE_NODEBUG __m512i simd_cast(const __m128i& src) noexcept { return _mm512_castsi128_si512(src); }
template<> BL_INLINE_NODEBUG __m512i simd_cast(const __m128&  src) noexcept { return _mm512_castsi128_si512(simd_as_i(src)); }
template<> BL_INLINE_NODEBUG __m512i simd_cast(const __m128d& src) noexcept { return _mm512_castsi128_si512(simd_as_i(src)); }
template<> BL_INLINE_NODEBUG __m512i simd_cast(const __m256i& src) noexcept { return _mm512_castsi256_si512(src); }
template<> BL_INLINE_NODEBUG __m512i simd_cast(const __m256&  src) noexcept { return _mm512_castsi256_si512(simd_as_i(src)); }
template<> BL_INLINE_NODEBUG __m512i simd_cast(const __m256d& src) noexcept { return _mm512_castsi256_si512(simd_as_i(src)); }
template<> BL_INLINE_NODEBUG __m512i simd_cast(const __m512i& src) noexcept { return src; }
template<> BL_INLINE_NODEBUG __m512i simd_cast(const __m512&  src) noexcept { return simd_as_i(src); }
template<> BL_INLINE_NODEBUG __m512i simd_cast(const __m512d& src) noexcept { return simd_as_i(src); }

template<> BL_INLINE_NODEBUG __m128 simd_cast(const __m512i& src) noexcept { return _mm512_castps512_ps128(simd_as_f(src)); }
template<> BL_INLINE_NODEBUG __m128 simd_cast(const __m512&  src) noexcept { return _mm512_castps512_ps128(src); }
template<> BL_INLINE_NODEBUG __m128 simd_cast(const __m512d& src) noexcept { return _mm512_castps512_ps128(simd_as_f(src)); }
template<> BL_INLINE_NODEBUG __m256 simd_cast(const __m512i& src) noexcept { return _mm512_castps512_ps256(simd_as_f(src)); }
template<> BL_INLINE_NODEBUG __m256 simd_cast(const __m512&  src) noexcept { return _mm512_castps512_ps256(src); }
template<> BL_INLINE_NODEBUG __m256 simd_cast(const __m512d& src) noexcept { return _mm512_castps512_ps256(simd_as_f(src)); }
template<> BL_INLINE_NODEBUG __m512 simd_cast(const __m128i& src) noexcept { return _mm512_castps128_ps512(simd_as_f(src)); }
template<> BL_INLINE_NODEBUG __m512 simd_cast(const __m128&  src) noexcept { return _mm512_castps128_ps512(src); }
template<> BL_INLINE_NODEBUG __m512 simd_cast(const __m128d& src) noexcept { return _mm512_castps128_ps512(simd_as_f(src)); }
template<> BL_INLINE_NODEBUG __m512 simd_cast(const __m256i& src) noexcept { return _mm512_castps256_ps512(simd_as_f(src)); }
template<> BL_INLINE_NODEBUG __m512 simd_cast(const __m256&  src) noexcept { return _mm512_castps256_ps512(src); }
template<> BL_INLINE_NODEBUG __m512 simd_cast(const __m256d& src) noexcept { return _mm512_castps256_ps512(simd_as_f(src)); }
template<> BL_INLINE_NODEBUG __m512 simd_cast(const __m512i& src) noexcept { return _mm512_castsi512_ps(src); }
template<> BL_INLINE_NODEBUG __m512 simd_cast(const __m512&  src) noexcept { return src; }
template<> BL_INLINE_NODEBUG __m512 simd_cast(const __m512d& src) noexcept { return _mm512_castpd_ps(src); }

template<> BL_INLINE_NODEBUG __m128d simd_cast(const __m512i& src) noexcept { return _mm512_castpd512_pd128(simd_as_d(src)); }
template<> BL_INLINE_NODEBUG __m128d simd_cast(const __m512&  src) noexcept { return _mm512_castpd512_pd128(simd_as_d(src)); }
template<> BL_INLINE_NODEBUG __m128d simd_cast(const __m512d& src) noexcept { return _mm512_castpd512_pd128(src); }
template<> BL_INLINE_NODEBUG __m256d simd_cast(const __m512i& src) noexcept { return _mm512_castpd512_pd256(simd_as_d(src)); }
template<> BL_INLINE_NODEBUG __m256d simd_cast(const __m512&  src) noexcept { return _mm512_castpd512_pd256(simd_as_d(src)); }
template<> BL_INLINE_NODEBUG __m256d simd_cast(const __m512d& src) noexcept { return _mm512_castpd512_pd256(src); }
template<> BL_INLINE_NODEBUG __m512d simd_cast(const __m128i& src) noexcept { return _mm512_castpd128_pd512(simd_as_d(src)); }
template<> BL_INLINE_NODEBUG __m512d simd_cast(const __m128&  src) noexcept { return _mm512_castpd128_pd512(simd_as_d(src)); }
template<> BL_INLINE_NODEBUG __m512d simd_cast(const __m128d& src) noexcept { return _mm512_castpd128_pd512(src); }
template<> BL_INLINE_NODEBUG __m512d simd_cast(const __m256i& src) noexcept { return _mm512_castpd256_pd512(simd_as_d(src)); }
template<> BL_INLINE_NODEBUG __m512d simd_cast(const __m256&  src) noexcept { return _mm512_castpd256_pd512(simd_as_d(src)); }
template<> BL_INLINE_NODEBUG __m512d simd_cast(const __m256d& src) noexcept { return _mm512_castpd256_pd512(src); }
template<> BL_INLINE_NODEBUG __m512d simd_cast(const __m512i& src) noexcept { return _mm512_castsi512_pd(src); }
template<> BL_INLINE_NODEBUG __m512d simd_cast(const __m512&  src) noexcept { return _mm512_castps_pd(src); }
template<> BL_INLINE_NODEBUG __m512d simd_cast(const __m512d& src) noexcept { return src; }
#endif // BL_TARGET_OPT_AVX512

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

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<16, T> vec_128(const Vec<W, T>& src) noexcept { return vec_cast<Vec<16, T>>(src); }

#if defined(BL_TARGET_OPT_AVX)
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<32, T> vec_256(const Vec<W, T>& src) noexcept { return vec_cast<Vec<32, T>>(src); }
#endif // BL_TARGET_OPT_AVX

#if defined(BL_TARGET_OPT_AVX512)
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<64, T> vec_512(const Vec<W, T>& src) noexcept { return vec_cast<Vec<64, T>>(src); }
#endif // BL_TARGET_OPT_AVX512

template<typename DstVectorT, typename SrcT>
BL_INLINE_NODEBUG const DstVectorT& vec_const(const SrcT* src) noexcept { return *static_cast<const DstVectorT*>(static_cast<const void*>(src)); }

// Converts a native SimdT register type to a wrapped V.
template<typename V, typename SimdT>
BL_INLINE_NODEBUG V from_simd(const SimdT& reg) noexcept { return V{simd_cast<typename V::SimdType>(reg)}; }

// Converts a wrapped V to a native SimdT register type.
template<typename SimdT, typename V>
BL_INLINE_NODEBUG SimdT to_simd(const V& vec) noexcept { return simd_cast<SimdT>(vec.v); }

// SIMD - Internal - Make Zero & Ones & Undefined
// ==============================================

namespace Internal {

template<typename SimdT>
BL_INLINE_NODEBUG SimdT simd_make_zero() noexcept;

template<typename SimdT>
BL_INLINE_NODEBUG SimdT simd_make_ones() noexcept;

template<typename SimdT>
BL_INLINE_NODEBUG SimdT simd_make_undefined() noexcept;

template<> BL_INLINE_NODEBUG __m128i simd_make_zero() noexcept { return _mm_setzero_si128(); }
template<> BL_INLINE_NODEBUG __m128  simd_make_zero() noexcept { return _mm_setzero_ps(); }
template<> BL_INLINE_NODEBUG __m128d simd_make_zero() noexcept { return _mm_setzero_pd(); }

template<> BL_INLINE_NODEBUG __m128i simd_make_ones() noexcept { return _mm_set1_epi32(-1); }
template<> BL_INLINE_NODEBUG __m128  simd_make_ones() noexcept { return simd_cast<__m128>(simd_make_ones<__m128i>()); }
template<> BL_INLINE_NODEBUG __m128d simd_make_ones() noexcept { return simd_cast<__m128d>(simd_make_ones<__m128i>()); }

template<> BL_INLINE_NODEBUG __m128i simd_make_undefined() noexcept { return _mm_undefined_si128(); }
template<> BL_INLINE_NODEBUG __m128  simd_make_undefined() noexcept { return _mm_undefined_ps(); }
template<> BL_INLINE_NODEBUG __m128d simd_make_undefined() noexcept { return _mm_undefined_pd(); }

#if defined(BL_TARGET_OPT_AVX)
template<> BL_INLINE_NODEBUG __m256i simd_make_zero() noexcept { return _mm256_setzero_si256(); }
template<> BL_INLINE_NODEBUG __m256  simd_make_zero() noexcept { return _mm256_setzero_ps(); }
template<> BL_INLINE_NODEBUG __m256d simd_make_zero() noexcept { return _mm256_setzero_pd(); }

template<> BL_INLINE_NODEBUG __m256i simd_make_ones() noexcept { return _mm256_set1_epi32(-1); }
template<> BL_INLINE_NODEBUG __m256  simd_make_ones() noexcept { return simd_cast<__m256>(simd_make_ones<__m256i>()); }
template<> BL_INLINE_NODEBUG __m256d simd_make_ones() noexcept { return simd_cast<__m256d>(simd_make_ones<__m256i>()); }

template<> BL_INLINE_NODEBUG __m256i simd_make_undefined() noexcept { return _mm256_undefined_si256(); }
template<> BL_INLINE_NODEBUG __m256  simd_make_undefined() noexcept { return _mm256_undefined_ps(); }
template<> BL_INLINE_NODEBUG __m256d simd_make_undefined() noexcept { return _mm256_undefined_pd(); }
#endif // BL_TARGET_OPT_AVX

#if defined(BL_TARGET_OPT_AVX512)
template<> BL_INLINE_NODEBUG __m512i simd_make_zero() noexcept { return _mm512_setzero_si512(); }
template<> BL_INLINE_NODEBUG __m512  simd_make_zero() noexcept { return _mm512_setzero_ps(); }
template<> BL_INLINE_NODEBUG __m512d simd_make_zero() noexcept { return _mm512_setzero_pd(); }

template<> BL_INLINE_NODEBUG __m512i simd_make_ones() noexcept { return _mm512_set1_epi32(-1); }
template<> BL_INLINE_NODEBUG __m512  simd_make_ones() noexcept { return simd_cast<__m512>(simd_make_ones<__m512i>()); }
template<> BL_INLINE_NODEBUG __m512d simd_make_ones() noexcept { return simd_cast<__m512d>(simd_make_ones<__m512i>()); }

template<> BL_INLINE_NODEBUG __m512i simd_make_undefined() noexcept { return _mm512_undefined_epi32(); }
template<> BL_INLINE_NODEBUG __m512  simd_make_undefined() noexcept { return _mm512_undefined_ps(); }
template<> BL_INLINE_NODEBUG __m512d simd_make_undefined() noexcept { return _mm512_undefined_pd(); }
#endif // BL_TARGET_OPT_AVX512

} // {Internal}

// SIMD - Internal - Make Vector
// =============================

namespace Internal {

template<size_t kW>
struct SimdMake;

BL_INLINE_NODEBUG __m128i simd_make128_u64(uint64_t x0) noexcept {
#if BL_TARGET_ARCH_BITS >= 64
  return _mm_set1_epi64x(int64_t(x0));
#else
  return _mm_set_epi32(int32_t(x0 >> 32), int32_t(x0 & 0xFFFFFFFF),
                       int32_t(x0 >> 32), int32_t(x0 & 0xFFFFFFFF));
#endif
}

BL_INLINE_NODEBUG __m128i simd_make128_u64(uint64_t x1, uint64_t x0) noexcept {
#if BL_TARGET_ARCH_BITS >= 64
  return _mm_set_epi64x(int64_t(x1), int64_t(x0));
#else
  return _mm_set_epi32(int32_t(x1 >> 32), int32_t(x1 & 0xFFFFFFFF),
                       int32_t(x0 >> 32), int32_t(x0 & 0xFFFFFFFF));
#endif
}

BL_INLINE_NODEBUG __m128i simd_make128_u32(uint32_t x0) noexcept {
  return _mm_set1_epi32(int32_t(x0));
}

BL_INLINE_NODEBUG __m128i simd_make128_u32(uint32_t x1, uint32_t x0) noexcept {
#if BL_TARGET_ARCH_BITS >= 64
  return _mm_set1_epi64x(int64_t((uint64_t(x1) << 32) | x0));
#else
  return _mm_set_epi32(int32_t(x1), int32_t(x0), int32_t(x1), int32_t(x0));
#endif
}

BL_INLINE_NODEBUG __m128i simd_make128_u32(uint32_t x3, uint32_t x2, uint32_t x1, uint32_t x0) noexcept {
  return _mm_set_epi32(int32_t(x3), int32_t(x2), int32_t(x1), int32_t(x0));
}

BL_INLINE_NODEBUG __m128i simd_make128_u16(uint16_t x0) noexcept {
  return _mm_set1_epi16(int16_t(x0));
}

BL_INLINE_NODEBUG __m128i simd_make128_u16(uint16_t x1, uint16_t x0) noexcept {
  uint32_t v = (uint32_t(x1) << 16) | x0;
  return _mm_set1_epi32(int32_t(v));
}

BL_INLINE_NODEBUG __m128i simd_make128_u16(uint16_t x3, uint16_t x2, uint16_t x1, uint16_t x0) noexcept {
#if BL_TARGET_ARCH_BITS >= 64
  uint64_t v = (uint64_t(x3) << 48) | (uint64_t(x2) << 32) | (uint64_t(x1) << 16) | x0;
  return _mm_set1_epi64x(int64_t(v));
#else
  return _mm_set_epi16(int16_t(x3), int16_t(x2), int16_t(x1), int16_t(x0),
                       int16_t(x3), int16_t(x2), int16_t(x1), int16_t(x0));
#endif
}

BL_INLINE_NODEBUG __m128i simd_make128_u16(uint16_t x7, uint16_t x6, uint16_t x5, uint16_t x4,
                                           uint16_t x3, uint16_t x2, uint16_t x1, uint16_t x0) noexcept {
  return _mm_set_epi16(int16_t(x7), int16_t(x6), int16_t(x5), int16_t(x4),
                       int16_t(x3), int16_t(x2), int16_t(x1), int16_t(x0));
}

BL_INLINE_NODEBUG __m128i simd_make128_u8(uint8_t x0) noexcept {
  return _mm_set1_epi8(int8_t(x0));
}

BL_INLINE_NODEBUG __m128i simd_make128_u8(uint8_t x1, uint8_t x0) noexcept {
  uint16_t v = uint16_t((uint16_t(x1) << 8) | (uint16_t(x0) << 0));
  return _mm_set1_epi16(int16_t(v));
}

BL_INLINE_NODEBUG __m128i simd_make128_u8(uint8_t x3, uint8_t x2, uint8_t x1, uint8_t x0) noexcept {
  uint32_t v = (uint32_t(x3) << 24) | (uint32_t(x2) << 16) | (uint32_t(x1) << 8) | (uint32_t(x0) << 0);
  return _mm_set1_epi32(int32_t(v));
}

BL_INLINE_NODEBUG __m128i simd_make128_u8(uint8_t x7, uint8_t x6, uint8_t x5, uint8_t x4,
                                          uint8_t x3, uint8_t x2, uint8_t x1, uint8_t x0) noexcept {
#if BL_TARGET_ARCH_BITS >= 64
  uint64_t v = (uint64_t(x7) << 56) | (uint64_t(x6) << 48) | (uint64_t(x5) << 40) | (uint64_t(x4) << 32)
             | (uint64_t(x3) << 24) | (uint64_t(x2) << 16) | (uint64_t(x1) <<  8) | (uint64_t(x0) <<  0);
  return _mm_set1_epi64x(int64_t(v));
#else
  return _mm_set_epi8(int8_t(x7), int8_t(x6), int8_t(x5), int8_t(x4),
                      int8_t(x3), int8_t(x2), int8_t(x1), int8_t(x0),
                      int8_t(x7), int8_t(x6), int8_t(x5), int8_t(x4),
                      int8_t(x3), int8_t(x2), int8_t(x1), int8_t(x0));
#endif
}

BL_INLINE_NODEBUG __m128i simd_make128_u8(uint8_t x15, uint8_t x14, uint8_t x13, uint8_t x12,
                                          uint8_t x11, uint8_t x10, uint8_t x09, uint8_t x08,
                                          uint8_t x07, uint8_t x06, uint8_t x05, uint8_t x04,
                                          uint8_t x03, uint8_t x02, uint8_t x01, uint8_t x00) noexcept {
  return _mm_set_epi8(int8_t(x15), int8_t(x14), int8_t(x13), int8_t(x12),
                      int8_t(x11), int8_t(x10), int8_t(x09), int8_t(x08),
                      int8_t(x07), int8_t(x06), int8_t(x05), int8_t(x04),
                      int8_t(x03), int8_t(x02), int8_t(x01), int8_t(x00));
}

BL_INLINE_NODEBUG __m128 simd_make128_f32(float x0) noexcept {
  return _mm_set1_ps(x0);
}

BL_INLINE_NODEBUG __m128 simd_make128_f32(float x1, float x0) noexcept {
  return _mm_set_ps(x1, x0, x1, x0);
}

BL_INLINE_NODEBUG __m128 simd_make128_f32(float x3, float x2, float x1, float x0) noexcept {
  return _mm_set_ps(x3, x2, x1, x0);
}

BL_INLINE_NODEBUG __m128d simd_make128_f64(double x0) noexcept {
  return _mm_set1_pd(x0);
}

BL_INLINE_NODEBUG __m128d simd_make128_f64(double x1, double x0) noexcept {
  return _mm_set_pd(x1, x0);
}

template<>
struct SimdMake<16> {
  template<typename... Args> static BL_INLINE_NODEBUG __m128i make_u8(Args&&... args) noexcept { return simd_make128_u8(BLInternal::forward<Args>(args)...); }
  template<typename... Args> static BL_INLINE_NODEBUG __m128i make_u16(Args&&... args) noexcept { return simd_make128_u16(BLInternal::forward<Args>(args)...); }
  template<typename... Args> static BL_INLINE_NODEBUG __m128i make_u32(Args&&... args) noexcept { return simd_make128_u32(BLInternal::forward<Args>(args)...); }
  template<typename... Args> static BL_INLINE_NODEBUG __m128i make_u64(Args&&... args) noexcept { return simd_make128_u64(BLInternal::forward<Args>(args)...); }
  template<typename... Args> static BL_INLINE_NODEBUG __m128 make_f32(Args&&... args) noexcept { return simd_make128_f32(BLInternal::forward<Args>(args)...); }
  template<typename... Args> static BL_INLINE_NODEBUG __m128d make_f64(Args&&... args) noexcept { return simd_make128_f64(BLInternal::forward<Args>(args)...); }
};

#ifdef BL_TARGET_OPT_AVX
BL_INLINE_NODEBUG __m256i simd_make256_u64(uint64_t x0) noexcept {
#if BL_TARGET_ARCH_BITS >= 64
  return _mm256_set1_epi64x(int64_t(x0));
#else
  return _mm256_set_epi32(int32_t(x0 >> 32), int32_t(x0 & 0xFFFFFFFF),
                          int32_t(x0 >> 32), int32_t(x0 & 0xFFFFFFFF),
                          int32_t(x0 >> 32), int32_t(x0 & 0xFFFFFFFF),
                          int32_t(x0 >> 32), int32_t(x0 & 0xFFFFFFFF));
#endif
}

BL_INLINE_NODEBUG __m256i simd_make256_u64(uint64_t x1, uint64_t x0) noexcept {
#if BL_TARGET_ARCH_BITS >= 64
  return _mm256_set_epi64x(int64_t(x1), int64_t(x0), int64_t(x1), int64_t(x0));
#else
  return _mm256_set_epi32(int32_t(x1 >> 32), int32_t(x1 & 0xFFFFFFFF),
                          int32_t(x0 >> 32), int32_t(x0 & 0xFFFFFFFF),
                          int32_t(x1 >> 32), int32_t(x1 & 0xFFFFFFFF),
                          int32_t(x0 >> 32), int32_t(x0 & 0xFFFFFFFF));
#endif
}

BL_INLINE_NODEBUG __m256i simd_make256_u64(uint64_t x3, uint64_t x2, uint64_t x1, uint64_t x0) noexcept {
#if BL_TARGET_ARCH_BITS >= 64
  return _mm256_set_epi64x(int64_t(x3), int64_t(x2), int64_t(x1), int64_t(x0));
#else
  return _mm256_set_epi32(int32_t(x3 >> 32), int32_t(x3 & 0xFFFFFFFF),
                          int32_t(x2 >> 32), int32_t(x2 & 0xFFFFFFFF),
                          int32_t(x1 >> 32), int32_t(x1 & 0xFFFFFFFF),
                          int32_t(x0 >> 32), int32_t(x0 & 0xFFFFFFFF));
#endif
}

BL_INLINE_NODEBUG __m256i simd_make256_u32(uint32_t x0) noexcept {
  return _mm256_set1_epi32(int32_t(x0));
}

BL_INLINE_NODEBUG __m256i simd_make256_u32(uint32_t x1, uint32_t x0) noexcept {
#if BL_TARGET_ARCH_BITS >= 64
  return _mm256_set1_epi64x(int64_t((uint64_t(x1) << 32) | x0));
#else
  return _mm256_set_epi32(int32_t(x1), int32_t(x0), int32_t(x1), int32_t(x0),
                          int32_t(x1), int32_t(x0), int32_t(x1), int32_t(x0));
#endif
}

BL_INLINE_NODEBUG __m256i simd_make256_u32(uint32_t x3, uint32_t x2, uint32_t x1, uint32_t x0) noexcept {
  return _mm256_set_epi32(int32_t(x3), int32_t(x2), int32_t(x1), int32_t(x0),
                          int32_t(x3), int32_t(x2), int32_t(x1), int32_t(x0));
}

BL_INLINE_NODEBUG __m256i simd_make256_u32(uint32_t x7, uint32_t x6, uint32_t x5, uint32_t x4,
                                           uint32_t x3, uint32_t x2, uint32_t x1, uint32_t x0) noexcept {
  return _mm256_set_epi32(int32_t(x7), int32_t(x6), int32_t(x5), int32_t(x4),
                          int32_t(x3), int32_t(x2), int32_t(x1), int32_t(x0));
}

BL_INLINE_NODEBUG __m256i simd_make256_u16(uint16_t x0) noexcept {
  return _mm256_set1_epi16(int16_t(x0));
}

BL_INLINE_NODEBUG __m256i simd_make256_u16(uint16_t x1, uint16_t x0) noexcept {
  uint32_t v = (uint32_t(x1) << 16) | x0;
  return _mm256_set1_epi32(int32_t(v));
}

BL_INLINE_NODEBUG __m256i simd_make256_u16(uint16_t x3, uint16_t x2, uint16_t x1, uint16_t x0) noexcept {
#if BL_TARGET_ARCH_BITS >= 64
  uint64_t v = (uint64_t(x3) << 48) | (uint64_t(x2) << 32) | (uint64_t(x1) << 16) | x0;
  return _mm256_set1_epi64x(int64_t(v));
#else
  return _mm256_set_epi16(int16_t(x3), int16_t(x2), int16_t(x1), int16_t(x0),
                          int16_t(x3), int16_t(x2), int16_t(x1), int16_t(x0),
                          int16_t(x3), int16_t(x2), int16_t(x1), int16_t(x0),
                          int16_t(x3), int16_t(x2), int16_t(x1), int16_t(x0));
#endif
}

BL_INLINE_NODEBUG __m256i simd_make256_u16(uint16_t x7, uint16_t x6, uint16_t x5, uint16_t x4,
                                           uint16_t x3, uint16_t x2, uint16_t x1, uint16_t x0) noexcept {
  return _mm256_set_epi16(int16_t(x7), int16_t(x6), int16_t(x5), int16_t(x4),
                          int16_t(x3), int16_t(x2), int16_t(x1), int16_t(x0),
                          int16_t(x7), int16_t(x6), int16_t(x5), int16_t(x4),
                          int16_t(x3), int16_t(x2), int16_t(x1), int16_t(x0));
}

BL_INLINE_NODEBUG __m256i simd_make256_u16(uint16_t x15, uint16_t x14, uint16_t x13, uint16_t x12,
                                           uint16_t x11, uint16_t x10, uint16_t x09, uint16_t x08,
                                           uint16_t x07, uint16_t x06, uint16_t x05, uint16_t x04,
                                           uint16_t x03, uint16_t x02, uint16_t x01, uint16_t x00) noexcept {
  return _mm256_set_epi16(int16_t(x15), int16_t(x14), int16_t(x13), int16_t(x12),
                          int16_t(x11), int16_t(x10), int16_t(x09), int16_t(x08),
                          int16_t(x07), int16_t(x06), int16_t(x05), int16_t(x04),
                          int16_t(x03), int16_t(x02), int16_t(x01), int16_t(x00));
}

BL_INLINE_NODEBUG __m256i simd_make256_u8(uint8_t x0) noexcept {
  return _mm256_set1_epi8(int8_t(x0));
}

BL_INLINE_NODEBUG __m256i simd_make256_u8(uint8_t x1, uint8_t x0) noexcept {
  uint16_t v = uint16_t((uint16_t(x1) << 8) | (uint16_t(x0) << 0));
  return _mm256_set1_epi16(int16_t(v));
}

BL_INLINE_NODEBUG __m256i simd_make256_u8(uint8_t x3, uint8_t x2, uint8_t x1, uint8_t x0) noexcept {
  uint32_t v = (uint32_t(x3) << 24) | (uint32_t(x2) << 16) | (uint32_t(x1) << 8) | (uint32_t(x0) << 0);
  return _mm256_set1_epi32(int32_t(v));
}

BL_INLINE_NODEBUG __m256i simd_make256_u8(uint8_t x7, uint8_t x6, uint8_t x5, uint8_t x4,
                                          uint8_t x3, uint8_t x2, uint8_t x1, uint8_t x0) noexcept {
#if BL_TARGET_ARCH_BITS >= 64
  uint64_t v = (uint64_t(x7) << 56) | (uint64_t(x6) << 48) | (uint64_t(x5) << 40) | (uint64_t(x4) << 32)
             | (uint64_t(x3) << 24) | (uint64_t(x2) << 16) | (uint64_t(x1) <<  8) | (uint64_t(x0) <<  0);
  return _mm256_set1_epi64x(int64_t(v));
#else
  int32_t hi = int32_t((uint32_t(x7) << 24) | (uint32_t(x6) << 16) | (uint32_t(x5) << 8) | uint32_t(x4));
  int32_t lo = int32_t((uint32_t(x3) << 24) | (uint32_t(x2) << 16) | (uint32_t(x1) << 8) | uint32_t(x0));
  return _mm256_set_epi32(hi, lo, hi, lo, hi, lo, hi, lo);
#endif
}

BL_INLINE_NODEBUG __m256i simd_make256_u8(uint8_t x15, uint8_t x14, uint8_t x13, uint8_t x12,
                                          uint8_t x11, uint8_t x10, uint8_t x09, uint8_t x08,
                                          uint8_t x07, uint8_t x06, uint8_t x05, uint8_t x04,
                                          uint8_t x03, uint8_t x02, uint8_t x01, uint8_t x00) noexcept {
  int32_t v3 = int32_t((uint32_t(x15) << 24) | (uint32_t(x14) << 16) | (uint32_t(x13) << 8) | uint32_t(x12));
  int32_t v2 = int32_t((uint32_t(x11) << 24) | (uint32_t(x10) << 16) | (uint32_t(x09) << 8) | uint32_t(x08));
  int32_t v1 = int32_t((uint32_t(x07) << 24) | (uint32_t(x06) << 16) | (uint32_t(x05) << 8) | uint32_t(x04));
  int32_t v0 = int32_t((uint32_t(x03) << 24) | (uint32_t(x02) << 16) | (uint32_t(x01) << 8) | uint32_t(x00));
  return _mm256_set_epi32(v3, v2, v1, v0, v3, v2, v1, v0);
}

BL_INLINE_NODEBUG __m256i simd_make256_u8(uint8_t x31, uint8_t x30, uint8_t x29, uint8_t x28,
                                          uint8_t x27, uint8_t x26, uint8_t x25, uint8_t x24,
                                          uint8_t x23, uint8_t x22, uint8_t x21, uint8_t x20,
                                          uint8_t x19, uint8_t x18, uint8_t x17, uint8_t x16,
                                          uint8_t x15, uint8_t x14, uint8_t x13, uint8_t x12,
                                          uint8_t x11, uint8_t x10, uint8_t x09, uint8_t x08,
                                          uint8_t x07, uint8_t x06, uint8_t x05, uint8_t x04,
                                          uint8_t x03, uint8_t x02, uint8_t x01, uint8_t x00) noexcept {
  return _mm256_set_epi8(int8_t(x31), int8_t(x30), int8_t(x29), int8_t(x28),
                         int8_t(x27), int8_t(x26), int8_t(x25), int8_t(x24),
                         int8_t(x23), int8_t(x22), int8_t(x21), int8_t(x20),
                         int8_t(x19), int8_t(x18), int8_t(x17), int8_t(x16),
                         int8_t(x15), int8_t(x14), int8_t(x13), int8_t(x12),
                         int8_t(x11), int8_t(x10), int8_t(x09), int8_t(x08),
                         int8_t(x07), int8_t(x06), int8_t(x05), int8_t(x04),
                         int8_t(x03), int8_t(x02), int8_t(x01), int8_t(x00));
}

BL_INLINE_NODEBUG __m256 simd_make256_f32(float x0) noexcept {
  return _mm256_set1_ps(x0);
}

BL_INLINE_NODEBUG __m256 simd_make256_f32(float x1, float x0) noexcept {
  return _mm256_set_ps(x1, x0, x1, x0,
                       x1, x0, x1, x0);
}

BL_INLINE_NODEBUG __m256 simd_make256_f32(float x3, float x2, float x1, float x0) noexcept {
  return _mm256_set_ps(x3, x2, x1, x0,
                       x3, x2, x1, x0);
}

BL_INLINE_NODEBUG __m256 simd_make256_f32(float x7, float x6, float x5, float x4,
                                          float x3, float x2, float x1, float x0) noexcept {
  return _mm256_set_ps(x7, x6, x5, x4,
                       x3, x2, x1, x0);
}

BL_INLINE_NODEBUG __m256d simd_make256_f64(double x0) noexcept {
  return _mm256_set1_pd(x0);
}

BL_INLINE_NODEBUG __m256d simd_make256_f64(double x1, double x0) noexcept {
  return _mm256_set_pd(x1, x0, x1, x0);
}

BL_INLINE_NODEBUG __m256d simd_make256_f64(double x3, double x2, double x1, double x0) noexcept {
  return _mm256_set_pd(x3, x2, x1, x0);
}

template<>
struct SimdMake<32> {
  template<typename... Args> static BL_INLINE_NODEBUG __m256i make_u8(Args&&... args) noexcept { return simd_make256_u8(BLInternal::forward<Args>(args)...); }
  template<typename... Args> static BL_INLINE_NODEBUG __m256i make_u16(Args&&... args) noexcept { return simd_make256_u16(BLInternal::forward<Args>(args)...); }
  template<typename... Args> static BL_INLINE_NODEBUG __m256i make_u32(Args&&... args) noexcept { return simd_make256_u32(BLInternal::forward<Args>(args)...); }
  template<typename... Args> static BL_INLINE_NODEBUG __m256i make_u64(Args&&... args) noexcept { return simd_make256_u64(BLInternal::forward<Args>(args)...); }
  template<typename... Args> static BL_INLINE_NODEBUG __m256 make_f32(Args&&... args) noexcept { return simd_make256_f32(BLInternal::forward<Args>(args)...); }
  template<typename... Args> static BL_INLINE_NODEBUG __m256d make_f64(Args&&... args) noexcept { return simd_make256_f64(BLInternal::forward<Args>(args)...); }
};
#endif // BL_TARGET_OPT_AVX

#ifdef BL_TARGET_OPT_AVX512
BL_INLINE_NODEBUG __m512i simd_make512_u64(uint64_t x0) noexcept {
#if BL_TARGET_ARCH_BITS >= 64
  return _mm512_set1_epi64(int64_t(x0));
#else
  return _mm512_broadcast_i32x4(
    _mm_set_epi32(
      int32_t(x0 >> 32), int32_t(x0 & 0xFFFFFFFF),
      int32_t(x0 >> 32), int32_t(x0 & 0xFFFFFFFF)));
#endif
}

BL_INLINE_NODEBUG __m512i simd_make512_u64(uint64_t x1, uint64_t x0) noexcept {
#if BL_TARGET_ARCH_BITS >= 64
  return _mm512_set_epi64(int64_t(x1), int64_t(x0), int64_t(x1), int64_t(x0),
                          int64_t(x1), int64_t(x0), int64_t(x1), int64_t(x0));
#else
  return _mm512_broadcast_i32x4(
    _mm_set_epi32(
      int32_t(x1 >> 32), int32_t(x1 & 0xFFFFFFFF),
      int32_t(x0 >> 32), int32_t(x0 & 0xFFFFFFFF)));
#endif
}

BL_INLINE_NODEBUG __m512i simd_make512_u64(uint64_t x3, uint64_t x2, uint64_t x1, uint64_t x0) noexcept {
#if BL_TARGET_ARCH_BITS >= 64
  return _mm512_set_epi64(int64_t(x3), int64_t(x2), int64_t(x1), int64_t(x0),
                          int64_t(x3), int64_t(x2), int64_t(x1), int64_t(x0));
#else
  return _mm512_broadcast_i32x8(
    _mm256_set_epi32(
      int32_t(x3 >> 32), int32_t(x3 & 0xFFFFFFFF),
      int32_t(x2 >> 32), int32_t(x2 & 0xFFFFFFFF),
      int32_t(x1 >> 32), int32_t(x1 & 0xFFFFFFFF),
      int32_t(x0 >> 32), int32_t(x0 & 0xFFFFFFFF)));
#endif
}

BL_INLINE_NODEBUG __m512i simd_make512_u64(uint64_t x7, uint64_t x6, uint64_t x5, uint64_t x4,
                                           uint64_t x3, uint64_t x2, uint64_t x1, uint64_t x0) noexcept {
#if BL_TARGET_ARCH_BITS >= 64
  return _mm512_set_epi64(int64_t(x7), int64_t(x6), int64_t(x5), int64_t(x4),
                          int64_t(x3), int64_t(x2), int64_t(x1), int64_t(x0));
#else
  return _mm512_set_epi32(int32_t(x7 >> 32), int32_t(x7 & 0xFFFFFFFF),
                          int32_t(x6 >> 32), int32_t(x6 & 0xFFFFFFFF),
                          int32_t(x5 >> 32), int32_t(x5 & 0xFFFFFFFF),
                          int32_t(x4 >> 32), int32_t(x4 & 0xFFFFFFFF),
                          int32_t(x3 >> 32), int32_t(x3 & 0xFFFFFFFF),
                          int32_t(x2 >> 32), int32_t(x2 & 0xFFFFFFFF),
                          int32_t(x1 >> 32), int32_t(x1 & 0xFFFFFFFF),
                          int32_t(x0 >> 32), int32_t(x0 & 0xFFFFFFFF));
#endif
}

BL_INLINE_NODEBUG __m512i simd_make512_u32(uint32_t x0) noexcept {
  return _mm512_set1_epi32(int32_t(x0));
}

BL_INLINE_NODEBUG __m512i simd_make512_u32(uint32_t x1, uint32_t x0) noexcept {
#if BL_TARGET_ARCH_BITS >= 64
  return _mm512_set1_epi64(int64_t((uint64_t(x1) << 32) | x0));
#else
  return _mm512_broadcast_i32x4(
    _mm_set_epi32(
      int32_t(x1), int32_t(x0), int32_t(x1), int32_t(x0)));
#endif
}

BL_INLINE_NODEBUG __m512i simd_make512_u32(uint32_t x3, uint32_t x2, uint32_t x1, uint32_t x0) noexcept {
  return _mm512_broadcast_i32x4(
    _mm_set_epi32(
      int32_t(x3), int32_t(x2), int32_t(x1), int32_t(x0)));
}

BL_INLINE_NODEBUG __m512i simd_make512_u32(uint32_t x7, uint32_t x6, uint32_t x5, uint32_t x4,
                                           uint32_t x3, uint32_t x2, uint32_t x1, uint32_t x0) noexcept {
  return _mm512_broadcast_i32x8(
    _mm256_set_epi32(
      int32_t(x7), int32_t(x6), int32_t(x5), int32_t(x4),
      int32_t(x3), int32_t(x2), int32_t(x1), int32_t(x0)));
}

BL_INLINE_NODEBUG __m512i simd_make512_u32(uint32_t x15, uint32_t x14, uint32_t x13, uint32_t x12,
                                           uint32_t x11, uint32_t x10, uint32_t x09, uint32_t x08,
                                           uint32_t x07, uint32_t x06, uint32_t x05, uint32_t x04,
                                           uint32_t x03, uint32_t x02, uint32_t x01, uint32_t x00) noexcept {
  return _mm512_set_epi32(int32_t(x15), int32_t(x14), int32_t(x13), int32_t(x12),
                          int32_t(x11), int32_t(x10), int32_t(x09), int32_t(x08),
                          int32_t(x07), int32_t(x06), int32_t(x05), int32_t(x04),
                          int32_t(x03), int32_t(x02), int32_t(x01), int32_t(x00));
}

BL_INLINE_NODEBUG __m512i simd_make512_u16(uint16_t x0) noexcept {
  return _mm512_set1_epi16(int16_t(x0));
}

BL_INLINE_NODEBUG __m512i simd_make512_u16(uint16_t x1, uint16_t x0) noexcept {
  uint32_t v = (uint32_t(x1) << 16) | x0;
  return _mm512_set1_epi32(int32_t(v));
}

BL_INLINE_NODEBUG __m512i simd_make512_u16(uint16_t x3, uint16_t x2, uint16_t x1, uint16_t x0) noexcept {
#if BL_TARGET_ARCH_BITS >= 64
  uint64_t v = (uint64_t(x3) << 48) | (uint64_t(x2) << 32) | (uint64_t(x1) << 16) | x0;
  return _mm512_set1_epi64(int64_t(v));
#else
  return _mm512_broadcast_i32x4(
    _mm_set_epi16(
      int16_t(x3), int16_t(x2), int16_t(x1), int16_t(x0),
      int16_t(x3), int16_t(x2), int16_t(x1), int16_t(x0)));
#endif
}

BL_INLINE_NODEBUG __m512i simd_make512_u16(uint16_t x7, uint16_t x6, uint16_t x5, uint16_t x4,
                                           uint16_t x3, uint16_t x2, uint16_t x1, uint16_t x0) noexcept {
  return _mm512_broadcast_i32x4(
    _mm_set_epi16(
      int16_t(x7), int16_t(x6), int16_t(x5), int16_t(x4),
      int16_t(x3), int16_t(x2), int16_t(x1), int16_t(x0)));
}

BL_INLINE_NODEBUG __m512i simd_make512_u16(uint16_t x15, uint16_t x14, uint16_t x13, uint16_t x12,
                                           uint16_t x11, uint16_t x10, uint16_t x09, uint16_t x08,
                                           uint16_t x07, uint16_t x06, uint16_t x05, uint16_t x04,
                                           uint16_t x03, uint16_t x02, uint16_t x01, uint16_t x00) noexcept {
  return _mm512_broadcast_i32x8(
    _mm256_set_epi16(
      int16_t(x15), int16_t(x14), int16_t(x13), int16_t(x12),
      int16_t(x11), int16_t(x10), int16_t(x09), int16_t(x08),
      int16_t(x07), int16_t(x06), int16_t(x05), int16_t(x04),
      int16_t(x03), int16_t(x02), int16_t(x01), int16_t(x00)));
}

BL_INLINE_NODEBUG __m512i simd_make512_u16(uint16_t x31, uint16_t x30, uint16_t x29, uint16_t x28,
                                           uint16_t x27, uint16_t x26, uint16_t x25, uint16_t x24,
                                           uint16_t x23, uint16_t x22, uint16_t x21, uint16_t x20,
                                           uint16_t x19, uint16_t x18, uint16_t x17, uint16_t x16,
                                           uint16_t x15, uint16_t x14, uint16_t x13, uint16_t x12,
                                           uint16_t x11, uint16_t x10, uint16_t x09, uint16_t x08,
                                           uint16_t x07, uint16_t x06, uint16_t x05, uint16_t x04,
                                           uint16_t x03, uint16_t x02, uint16_t x01, uint16_t x00) noexcept {
#if defined(__GNUC__) && __GNUC__ < 10 && !defined(__clang__)
  // GCC doesn't provide this intrinsic up to GCC 9.2.
  uint32_t u15 = scalar_u32_from_2x_u16(x31, x30);
  uint32_t u14 = scalar_u32_from_2x_u16(x29, x28);
  uint32_t u13 = scalar_u32_from_2x_u16(x27, x26);
  uint32_t u12 = scalar_u32_from_2x_u16(x25, x24);
  uint32_t u11 = scalar_u32_from_2x_u16(x23, x22);
  uint32_t u10 = scalar_u32_from_2x_u16(x21, x20);
  uint32_t u09 = scalar_u32_from_2x_u16(x19, x18);
  uint32_t u08 = scalar_u32_from_2x_u16(x17, x16);
  uint32_t u07 = scalar_u32_from_2x_u16(x15, x14);
  uint32_t u06 = scalar_u32_from_2x_u16(x13, x12);
  uint32_t u05 = scalar_u32_from_2x_u16(x11, x10);
  uint32_t u04 = scalar_u32_from_2x_u16(x09, x08);
  uint32_t u03 = scalar_u32_from_2x_u16(x07, x06);
  uint32_t u02 = scalar_u32_from_2x_u16(x05, x04);
  uint32_t u01 = scalar_u32_from_2x_u16(x03, x02);
  uint32_t u00 = scalar_u32_from_2x_u16(x01, x00);
  return _mm512_set_epi32(u15, u14, u13, u12, u11, u10, u09, u08,
                          u07, u06, u05, u04, u03, u02, u01, u00);
#else
  return _mm512_set_epi16(int16_t(x31), int16_t(x30), int16_t(x29), int16_t(x28),
                          int16_t(x27), int16_t(x26), int16_t(x25), int16_t(x24),
                          int16_t(x23), int16_t(x22), int16_t(x21), int16_t(x20),
                          int16_t(x19), int16_t(x18), int16_t(x17), int16_t(x16),
                          int16_t(x15), int16_t(x14), int16_t(x13), int16_t(x12),
                          int16_t(x11), int16_t(x10), int16_t(x09), int16_t(x08),
                          int16_t(x07), int16_t(x06), int16_t(x05), int16_t(x04),
                          int16_t(x03), int16_t(x02), int16_t(x01), int16_t(x00));
#endif
}

BL_INLINE_NODEBUG __m512i simd_make512_u8(uint8_t x0) noexcept {
  return _mm512_set1_epi8(int8_t(x0));
}

BL_INLINE_NODEBUG __m512i simd_make512_u8(uint8_t x1, uint8_t x0) noexcept {
  uint16_t v = uint16_t((uint16_t(x1) << 8) | (uint16_t(x0) << 0));
  return _mm512_set1_epi16(int16_t(v));
}

BL_INLINE_NODEBUG __m512i simd_make512_u8(uint8_t x3, uint8_t x2, uint8_t x1, uint8_t x0) noexcept {
  uint32_t v = (uint32_t(x3) << 24) | (uint32_t(x2) << 16) | (uint32_t(x1) << 8) | (uint32_t(x0) << 0);
  return _mm512_set1_epi32(int32_t(v));
}

BL_INLINE_NODEBUG __m512i simd_make512_u8(uint8_t x7, uint8_t x6, uint8_t x5, uint8_t x4,
                                          uint8_t x3, uint8_t x2, uint8_t x1, uint8_t x0) noexcept {
#if BL_TARGET_ARCH_BITS >= 64
  uint64_t v = (uint64_t(x7) << 56) | (uint64_t(x6) << 48) | (uint64_t(x5) << 40) | (uint64_t(x4) << 32)
             | (uint64_t(x3) << 24) | (uint64_t(x2) << 16) | (uint64_t(x1) <<  8) | (uint64_t(x0) <<  0);
  return _mm512_set1_epi64(int64_t(v));
#else
  int32_t hi = int32_t((uint32_t(x7) << 24) | (uint32_t(x6) << 16) | (uint32_t(x5) << 8) | uint32_t(x4));
  int32_t lo = int32_t((uint32_t(x3) << 24) | (uint32_t(x2) << 16) | (uint32_t(x1) << 8) | uint32_t(x0));
  return _mm512_broadcast_i32x4(_mm_set_epi32(hi, lo, hi, lo));
#endif
}

BL_INLINE_NODEBUG __m512i simd_make512_u8(uint8_t x15, uint8_t x14, uint8_t x13, uint8_t x12,
                                          uint8_t x11, uint8_t x10, uint8_t x09, uint8_t x08,
                                          uint8_t x07, uint8_t x06, uint8_t x05, uint8_t x04,
                                          uint8_t x03, uint8_t x02, uint8_t x01, uint8_t x00) noexcept {
  int32_t v3 = int32_t((uint32_t(x15) << 24) | (uint32_t(x14) << 16) | (uint32_t(x13) << 8) | uint32_t(x12));
  int32_t v2 = int32_t((uint32_t(x11) << 24) | (uint32_t(x10) << 16) | (uint32_t(x09) << 8) | uint32_t(x08));
  int32_t v1 = int32_t((uint32_t(x07) << 24) | (uint32_t(x06) << 16) | (uint32_t(x05) << 8) | uint32_t(x04));
  int32_t v0 = int32_t((uint32_t(x03) << 24) | (uint32_t(x02) << 16) | (uint32_t(x01) << 8) | uint32_t(x00));
  return _mm512_broadcast_i32x4(_mm_set_epi32(v3, v2, v1, v0));
}

BL_INLINE_NODEBUG __m512i simd_make512_u8(uint8_t x31, uint8_t x30, uint8_t x29, uint8_t x28,
                                          uint8_t x27, uint8_t x26, uint8_t x25, uint8_t x24,
                                          uint8_t x23, uint8_t x22, uint8_t x21, uint8_t x20,
                                          uint8_t x19, uint8_t x18, uint8_t x17, uint8_t x16,
                                          uint8_t x15, uint8_t x14, uint8_t x13, uint8_t x12,
                                          uint8_t x11, uint8_t x10, uint8_t x09, uint8_t x08,
                                          uint8_t x07, uint8_t x06, uint8_t x05, uint8_t x04,
                                          uint8_t x03, uint8_t x02, uint8_t x01, uint8_t x00) noexcept {
  return _mm512_broadcast_i32x8(
    _mm256_set_epi8(
      int8_t(x31), int8_t(x30), int8_t(x29), int8_t(x28),
      int8_t(x27), int8_t(x26), int8_t(x25), int8_t(x24),
      int8_t(x23), int8_t(x22), int8_t(x21), int8_t(x20),
      int8_t(x19), int8_t(x18), int8_t(x17), int8_t(x16),
      int8_t(x15), int8_t(x14), int8_t(x13), int8_t(x12),
      int8_t(x11), int8_t(x10), int8_t(x09), int8_t(x08),
      int8_t(x07), int8_t(x06), int8_t(x05), int8_t(x04),
      int8_t(x03), int8_t(x02), int8_t(x01), int8_t(x00)));
}

BL_INLINE_NODEBUG __m512i simd_make512_u8(uint8_t x63, uint8_t x62, uint8_t x61, uint8_t x60,
                                          uint8_t x59, uint8_t x58, uint8_t x57, uint8_t x56,
                                          uint8_t x55, uint8_t x54, uint8_t x53, uint8_t x52,
                                          uint8_t x51, uint8_t x50, uint8_t x49, uint8_t x48,
                                          uint8_t x47, uint8_t x46, uint8_t x45, uint8_t x44,
                                          uint8_t x43, uint8_t x42, uint8_t x41, uint8_t x40,
                                          uint8_t x39, uint8_t x38, uint8_t x37, uint8_t x36,
                                          uint8_t x35, uint8_t x34, uint8_t x33, uint8_t x32,
                                          uint8_t x31, uint8_t x30, uint8_t x29, uint8_t x28,
                                          uint8_t x27, uint8_t x26, uint8_t x25, uint8_t x24,
                                          uint8_t x23, uint8_t x22, uint8_t x21, uint8_t x20,
                                          uint8_t x19, uint8_t x18, uint8_t x17, uint8_t x16,
                                          uint8_t x15, uint8_t x14, uint8_t x13, uint8_t x12,
                                          uint8_t x11, uint8_t x10, uint8_t x09, uint8_t x08,
                                          uint8_t x07, uint8_t x06, uint8_t x05, uint8_t x04,
                                          uint8_t x03, uint8_t x02, uint8_t x01, uint8_t x00) noexcept {
#if defined(__GNUC__) && __GNUC__ < 10 && !defined(__clang__)
  // GCC doesn't provide this intrinsic up to GCC 9.2.
  uint32_t u15 = scalar_u32_from_4x_u8(x63, x62, x61, x60);
  uint32_t u14 = scalar_u32_from_4x_u8(x59, x58, x57, x56);
  uint32_t u13 = scalar_u32_from_4x_u8(x55, x54, x53, x52);
  uint32_t u12 = scalar_u32_from_4x_u8(x51, x50, x49, x48);
  uint32_t u11 = scalar_u32_from_4x_u8(x47, x46, x45, x44);
  uint32_t u10 = scalar_u32_from_4x_u8(x43, x42, x41, x40);
  uint32_t u09 = scalar_u32_from_4x_u8(x39, x38, x37, x36);
  uint32_t u08 = scalar_u32_from_4x_u8(x35, x34, x33, x32);
  uint32_t u07 = scalar_u32_from_4x_u8(x31, x30, x29, x28);
  uint32_t u06 = scalar_u32_from_4x_u8(x27, x26, x25, x24);
  uint32_t u05 = scalar_u32_from_4x_u8(x23, x22, x21, x20);
  uint32_t u04 = scalar_u32_from_4x_u8(x19, x18, x17, x16);
  uint32_t u03 = scalar_u32_from_4x_u8(x15, x14, x13, x12);
  uint32_t u02 = scalar_u32_from_4x_u8(x11, x10, x09, x08);
  uint32_t u01 = scalar_u32_from_4x_u8(x07, x06, x05, x04);
  uint32_t u00 = scalar_u32_from_4x_u8(x03, x02, x01, x00);
  return _mm512_set_epi32(u15, u14, u13, u12, u11, u10, u09, u08,
                          u07, u06, u05, u04, u03, u02, u01, u00);
#else
  return _mm512_set_epi8(
    int8_t(x63), int8_t(x62), int8_t(x61), int8_t(x60),
    int8_t(x59), int8_t(x58), int8_t(x57), int8_t(x56),
    int8_t(x55), int8_t(x54), int8_t(x53), int8_t(x52),
    int8_t(x51), int8_t(x50), int8_t(x49), int8_t(x48),
    int8_t(x47), int8_t(x46), int8_t(x45), int8_t(x44),
    int8_t(x43), int8_t(x42), int8_t(x41), int8_t(x40),
    int8_t(x39), int8_t(x38), int8_t(x37), int8_t(x36),
    int8_t(x35), int8_t(x34), int8_t(x33), int8_t(x32),
    int8_t(x31), int8_t(x30), int8_t(x29), int8_t(x28),
    int8_t(x27), int8_t(x26), int8_t(x25), int8_t(x24),
    int8_t(x23), int8_t(x22), int8_t(x21), int8_t(x20),
    int8_t(x19), int8_t(x18), int8_t(x17), int8_t(x16),
    int8_t(x15), int8_t(x14), int8_t(x13), int8_t(x12),
    int8_t(x11), int8_t(x10), int8_t(x09), int8_t(x08),
    int8_t(x07), int8_t(x06), int8_t(x05), int8_t(x04),
    int8_t(x03), int8_t(x02), int8_t(x01), int8_t(x00));
#endif
}

BL_INLINE_NODEBUG __m512 simd_make512_f32(float x0) noexcept {
  return _mm512_set1_ps(x0);
}

BL_INLINE_NODEBUG __m512 simd_make512_f32(float x1, float x0) noexcept {
  return _mm512_broadcast_f32x4(_mm_set_ps(x1, x0, x1, x0));
}

BL_INLINE_NODEBUG __m512 simd_make512_f32(float x3, float x2, float x1, float x0) noexcept {
  return _mm512_broadcast_f32x4(_mm_set_ps(x3, x2, x1, x0));
}

BL_INLINE_NODEBUG __m512 simd_make512_f32(float x7, float x6, float x5, float x4,
                                          float x3, float x2, float x1, float x0) noexcept {
  return _mm512_set_ps(x7, x6, x5, x4,
                       x3, x2, x1, x0,
                       x7, x6, x5, x4,
                       x3, x2, x1, x0);
}

BL_INLINE_NODEBUG __m512 simd_make512_f32(float x15, float x14, float x13, float x12,
                                          float x11, float x10, float x09, float x08,
                                          float x07, float x06, float x05, float x04,
                                          float x03, float x02, float x01, float x00) noexcept {
  return _mm512_set_ps(x15, x14, x13, x12,
                       x11, x10, x09, x08,
                       x07, x06, x05, x04,
                       x03, x02, x01, x00);
}

BL_INLINE_NODEBUG __m512d simd_make512_f64(double x0) noexcept {
  return _mm512_set1_pd(x0);
}

BL_INLINE_NODEBUG __m512d simd_make512_f64(double x1, double x0) noexcept {
  return _mm512_broadcast_f64x2(_mm_set_pd(x1, x0));
}

BL_INLINE_NODEBUG __m512d simd_make512_f64(double x3, double x2, double x1, double x0) noexcept {
  return _mm512_broadcast_f64x4(_mm256_set_pd(x3, x2, x1, x0));
}

BL_INLINE_NODEBUG __m512d simd_make512_f64(double x7, double x6, double x5, double x4,
                                           double x3, double x2, double x1, double x0) noexcept {
  return _mm512_set_pd(x7, x6, x5, x4,
                       x3, x2, x1, x0);
}

template<>
struct SimdMake<64> {
  template<typename... Args> static BL_INLINE_NODEBUG __m512i make_u8(Args&&... args) noexcept { return simd_make512_u8(BLInternal::forward<Args>(args)...); }
  template<typename... Args> static BL_INLINE_NODEBUG __m512i make_u16(Args&&... args) noexcept { return simd_make512_u16(BLInternal::forward<Args>(args)...); }
  template<typename... Args> static BL_INLINE_NODEBUG __m512i make_u32(Args&&... args) noexcept { return simd_make512_u32(BLInternal::forward<Args>(args)...); }
  template<typename... Args> static BL_INLINE_NODEBUG __m512i make_u64(Args&&... args) noexcept { return simd_make512_u64(BLInternal::forward<Args>(args)...); }
  template<typename... Args> static BL_INLINE_NODEBUG __m512 make_f32(Args&&... args) noexcept { return simd_make512_f32(BLInternal::forward<Args>(args)...); }
  template<typename... Args> static BL_INLINE_NODEBUG __m512d make_f64(Args&&... args) noexcept { return simd_make512_f64(BLInternal::forward<Args>(args)...); }
};
#endif // BL_TARGET_OPT_AVX512

template<typename SimdT, typename... Args> BL_INLINE_NODEBUG SimdT simd_make_i8(Args&&... args) noexcept { return simd_cast<SimdT>(SimdMake<sizeof(SimdT)>::make_u8(uint8_t(BLInternal::forward<Args>(args))...)); }
template<typename SimdT, typename... Args> BL_INLINE_NODEBUG SimdT simd_make_i16(Args&&... args) noexcept { return simd_cast<SimdT>(SimdMake<sizeof(SimdT)>::make_u16(uint16_t(BLInternal::forward<Args>(args))...)); }
template<typename SimdT, typename... Args> BL_INLINE_NODEBUG SimdT simd_make_i32(Args&&... args) noexcept { return simd_cast<SimdT>(SimdMake<sizeof(SimdT)>::make_u32(uint32_t(BLInternal::forward<Args>(args))...)); }
template<typename SimdT, typename... Args> BL_INLINE_NODEBUG SimdT simd_make_i64(Args&&... args) noexcept { return simd_cast<SimdT>(SimdMake<sizeof(SimdT)>::make_u64(uint64_t(BLInternal::forward<Args>(args))...)); }
template<typename SimdT, typename... Args> BL_INLINE_NODEBUG SimdT simd_make_u8(Args&&... args) noexcept { return simd_cast<SimdT>(SimdMake<sizeof(SimdT)>::make_u8(uint8_t(BLInternal::forward<Args>(args))...)); }
template<typename SimdT, typename... Args> BL_INLINE_NODEBUG SimdT simd_make_u16(Args&&... args) noexcept { return simd_cast<SimdT>(SimdMake<sizeof(SimdT)>::make_u16(uint16_t(BLInternal::forward<Args>(args))...)); }
template<typename SimdT, typename... Args> BL_INLINE_NODEBUG SimdT simd_make_u32(Args&&... args) noexcept { return simd_cast<SimdT>(SimdMake<sizeof(SimdT)>::make_u32(uint32_t(BLInternal::forward<Args>(args))...)); }
template<typename SimdT, typename... Args> BL_INLINE_NODEBUG SimdT simd_make_u64(Args&&... args) noexcept { return simd_cast<SimdT>(SimdMake<sizeof(SimdT)>::make_u64(uint64_t(BLInternal::forward<Args>(args))...)); }
template<typename SimdT, typename... Args> BL_INLINE_NODEBUG SimdT simd_make_f32(Args&&... args) noexcept { return simd_cast<SimdT>(SimdMake<sizeof(SimdT)>::make_f32(float(BLInternal::forward<Args>(args))...)); }
template<typename SimdT, typename... Args> BL_INLINE_NODEBUG SimdT simd_make_f64(Args&&... args) noexcept { return simd_cast<SimdT>(SimdMake<sizeof(SimdT)>::make_f64(double(BLInternal::forward<Args>(args))...)); }

} // {Internal}

// SIMD - Internal - Cast Vector <-> Scalar
// ========================================

namespace Internal {

BL_INLINE_NODEBUG __m128i simd_cast_from_u32(uint32_t val) noexcept { return _mm_cvtsi32_si128(int(val)); }
BL_INLINE_NODEBUG uint32_t simd_cast_to_u32(const __m128i& src) noexcept { return uint32_t(_mm_cvtsi128_si32(src)); }

#if BL_TARGET_ARCH_BITS >= 64
BL_INLINE_NODEBUG __m128i simd_cast_from_u64(uint64_t val) noexcept { return _mm_cvtsi64_si128(int64_t(val)); }
BL_INLINE_NODEBUG uint64_t simd_cast_to_u64(const __m128i& src) noexcept { return uint64_t(_mm_cvtsi128_si64(src)); }
#else
BL_INLINE_NODEBUG __m128i simd_cast_from_u64(uint64_t val) noexcept { return _mm_loadl_epi64(reinterpret_cast<const __m128i*>(&val)); }
BL_INLINE_NODEBUG uint64_t simd_cast_to_u64(const __m128i& src) noexcept {
  uint64_t result;
  _mm_storel_epi64(reinterpret_cast<__m128i*>(&result), src);
  return result;
}
#endif

#if defined(__GNUC__) && !defined(__clang__)
// See: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=70708
BL_INLINE_NODEBUG __m128 simd_cast_from_f32(float val) noexcept { __m128 reg; __asm__("" : "=x" (reg) : "0" (val)); return reg; }
BL_INLINE_NODEBUG __m128d simd_cast_from_f64(double val) noexcept { __m128d reg; __asm__("" : "=x" (reg) : "0" (val)); return reg; }
#else
BL_INLINE_NODEBUG __m128 simd_cast_from_f32(float val) noexcept { return _mm_set_ss(val); }
BL_INLINE_NODEBUG __m128d simd_cast_from_f64(double val) noexcept { return _mm_set_sd(val); }
#endif

BL_INLINE_NODEBUG float simd_cast_to_f32(const __m128& src) noexcept { return _mm_cvtss_f32(src); }
BL_INLINE_NODEBUG double simd_cast_to_f64(const __m128d& src) noexcept { return _mm_cvtsd_f64(src); }

} // {Internal}

// SIMD - Internal - Convert Vector <-> Vector
// ===========================================

namespace Internal {

BL_INLINE_NODEBUG __m128 simd_cvt_i32_f32(const __m128i& a) noexcept { return _mm_cvtepi32_ps(a); }
BL_INLINE_NODEBUG __m128i simd_cvt_f32_i32(const __m128& a) noexcept { return _mm_cvtps_epi32(a); }
BL_INLINE_NODEBUG __m128i simd_cvtt_f32_i32(const __m128& a) noexcept { return _mm_cvttps_epi32(a); }

/*
// TODO: SIMD WRAP
BL_INLINE_NODEBUG __m128i simd_cvt_f64_i32(const __m128d& a) noexcept { return _mm_cvtpd_epi32(a); }
BL_INLINE_NODEBUG __m128i simd_cvtt_f64_i32(const __m128d& a) noexcept { return _mm_cvttpd_epi32(a); }
BL_INLINE_NODEBUG __m128 simd_cvt_f64_f32(const __m128d& a) noexcept { return _mm_cvtpd_ps(a); }
BL_INLINE_NODEBUG __m128d simd_cvt_2xi32_f64(const __m128i& a) noexcept { return _mm_cvtepi32_pd(a); }
BL_INLINE_NODEBUG __m128d simd_cvt_f32x2_f64(const __m128& a) noexcept { return _mm_cvtps_pd(a); }
*/

#if defined(BL_TARGET_OPT_AVX2)
BL_INLINE_NODEBUG __m256 simd_cvt_i32_f32(const __m256i& a) noexcept { return _mm256_cvtepi32_ps(a); }
BL_INLINE_NODEBUG __m256i simd_cvt_f32_i32(const __m256& a) noexcept { return _mm256_cvtps_epi32(a); }
BL_INLINE_NODEBUG __m256i simd_cvtt_f32_i32(const __m256& a) noexcept { return _mm256_cvttps_epi32(a); }
#endif // BL_TARGET_OPT_AVX2

#if defined(BL_TARGET_OPT_AVX512)
BL_INLINE_NODEBUG __m512 simd_cvt_i32_f32(const __m512i& a) noexcept { return _mm512_cvtepi32_ps(a); }
BL_INLINE_NODEBUG __m512i simd_cvt_f32_i32(const __m512& a) noexcept { return _mm512_cvtps_epi32(a); }
BL_INLINE_NODEBUG __m512i simd_cvtt_f32_i32(const __m512& a) noexcept { return _mm512_cvttps_epi32(a); }
#endif // BL_TARGET_OPT_AVX512

} // {Internal}

// SIMD - Internal - Convert Vector <-> Scalar
// ===========================================

namespace Internal {

BL_INLINE_NODEBUG __m128 simd_cvt_f32_from_scalar_i32(int32_t val) noexcept { return _mm_cvtsi32_ss(_mm_setzero_ps(), val); }
BL_INLINE_NODEBUG __m128d simd_cvt_f64_from_scalar_i32(int32_t val) noexcept { return _mm_cvtsi32_sd(_mm_setzero_pd(), val); }

BL_INLINE_NODEBUG int32_t simd_cvt_f32_to_scalar_i32(const __m128& src) noexcept { return _mm_cvtss_si32(src); }
BL_INLINE_NODEBUG int32_t simd_cvt_f64_to_scalar_i32(const __m128d& src) noexcept { return _mm_cvtsd_si32(src); }
BL_INLINE_NODEBUG int32_t simd_cvtt_f32_to_scalar_i32(const __m128& src) noexcept { return _mm_cvttss_si32(src); }
BL_INLINE_NODEBUG int32_t simd_cvtt_f64_to_scalar_i32(const __m128d& src) noexcept { return _mm_cvttsd_si32(src); }

#if BL_TARGET_ARCH_BITS >= 64
BL_INLINE_NODEBUG __m128 simd_cvt_f32_from_scalar_i64(int64_t val) noexcept { return _mm_cvtsi64_ss(_mm_setzero_ps(), val); }
BL_INLINE_NODEBUG __m128d simd_cvt_f64_from_scalar_i64(int64_t val) noexcept { return _mm_cvtsi64_sd(_mm_setzero_pd(), val); }

BL_INLINE_NODEBUG int64_t simd_cvt_f32_to_scalar_i64(const __m128& src) noexcept { return _mm_cvtss_si64(src); }
BL_INLINE_NODEBUG int64_t simd_cvt_f64_to_scalar_i64(const __m128d& src) noexcept { return _mm_cvtsd_si64(src); }
BL_INLINE_NODEBUG int64_t simd_cvtt_f32_to_scalar_i64(const __m128& src) noexcept { return _mm_cvttss_si64(src); }
BL_INLINE_NODEBUG int64_t simd_cvtt_f64_to_scalar_i64(const __m128d& src) noexcept { return _mm_cvttsd_si64(src); }
#endif

} // {Internal}

// SIMD - Internal - Convert Vector <-> Mask
// =========================================

namespace Internal {

#if defined(BL_TARGET_OPT_AVX512)
BL_INLINE_NODEBUG __m128i simd_128i_from_mask8(__mmask16 k) noexcept { return _mm_movm_epi8(k); }
BL_INLINE_NODEBUG __m128  simd_128f_from_mask8(__mmask16 k) noexcept { return simd_as_f(simd_128i_from_mask8(k)); }
BL_INLINE_NODEBUG __m128d simd_128d_from_mask8(__mmask16 k) noexcept { return simd_as_d(simd_128i_from_mask8(k)); }

BL_INLINE_NODEBUG __m256i simd_256i_from_mask8(__mmask32 k) noexcept { return _mm256_movm_epi8(k); }
BL_INLINE_NODEBUG __m256  simd_256f_from_mask8(__mmask32 k) noexcept { return simd_as_f(simd_256i_from_mask8(k)); }
BL_INLINE_NODEBUG __m256d simd_256d_from_mask8(__mmask32 k) noexcept { return simd_as_d(simd_256i_from_mask8(k)); }

BL_INLINE_NODEBUG __m512i simd_512i_from_mask8(__mmask64 k) noexcept { return _mm512_movm_epi8(k); }
BL_INLINE_NODEBUG __m512  simd_512f_from_mask8(__mmask64 k) noexcept { return simd_as_f(simd_512i_from_mask8(k)); }
BL_INLINE_NODEBUG __m512d simd_512d_from_mask8(__mmask64 k) noexcept { return simd_as_d(simd_512i_from_mask8(k)); }

BL_INLINE_NODEBUG __m128i simd_128i_from_mask16(__mmask8 k) noexcept { return _mm_movm_epi16(k); }
BL_INLINE_NODEBUG __m128  simd_128f_from_mask16(__mmask8 k) noexcept { return simd_as_f(simd_128i_from_mask8(k)); }
BL_INLINE_NODEBUG __m128d simd_128d_from_mask16(__mmask8 k) noexcept { return simd_as_d(simd_128i_from_mask8(k)); }

BL_INLINE_NODEBUG __m256i simd_256i_from_mask16(__mmask16 k) noexcept { return _mm256_movm_epi16(k); }
BL_INLINE_NODEBUG __m256  simd_256f_from_mask16(__mmask16 k) noexcept { return simd_as_f(simd_256i_from_mask8(k)); }
BL_INLINE_NODEBUG __m256d simd_256d_from_mask16(__mmask16 k) noexcept { return simd_as_d(simd_256i_from_mask8(k)); }

BL_INLINE_NODEBUG __m512i simd_512i_from_mask16(__mmask32 k) noexcept { return _mm512_movm_epi16(k); }
BL_INLINE_NODEBUG __m512  simd_512f_from_mask16(__mmask32 k) noexcept { return simd_as_f(simd_512i_from_mask8(k)); }
BL_INLINE_NODEBUG __m512d simd_512d_from_mask16(__mmask32 k) noexcept { return simd_as_d(simd_512i_from_mask8(k)); }

BL_INLINE_NODEBUG __m128i simd_128i_from_mask32(__mmask8 k) noexcept { return _mm_movm_epi32(k); }
BL_INLINE_NODEBUG __m128  simd_128f_from_mask32(__mmask8 k) noexcept { return simd_as_f(simd_128i_from_mask8(k)); }
BL_INLINE_NODEBUG __m128d simd_128d_from_mask32(__mmask8 k) noexcept { return simd_as_d(simd_128i_from_mask8(k)); }

BL_INLINE_NODEBUG __m256i simd_256i_from_mask32(__mmask8 k) noexcept { return _mm256_movm_epi32(k); }
BL_INLINE_NODEBUG __m256  simd_256f_from_mask32(__mmask8 k) noexcept { return simd_as_f(simd_256i_from_mask8(k)); }
BL_INLINE_NODEBUG __m256d simd_256d_from_mask32(__mmask8 k) noexcept { return simd_as_d(simd_256i_from_mask8(k)); }

BL_INLINE_NODEBUG __m512i simd_512i_from_mask32(__mmask16 k) noexcept { return _mm512_movm_epi32(k); }
BL_INLINE_NODEBUG __m512  simd_512f_from_mask32(__mmask16 k) noexcept { return simd_as_f(simd_512i_from_mask8(k)); }
BL_INLINE_NODEBUG __m512d simd_512d_from_mask32(__mmask16 k) noexcept { return simd_as_d(simd_512i_from_mask8(k)); }

BL_INLINE_NODEBUG __m128i simd_128i_from_mask64(__mmask8 k) noexcept { return _mm_movm_epi64(k); }
BL_INLINE_NODEBUG __m128  simd_128f_from_mask64(__mmask8 k) noexcept { return simd_as_f(simd_128i_from_mask8(k)); }
BL_INLINE_NODEBUG __m128d simd_128d_from_mask64(__mmask8 k) noexcept { return simd_as_d(simd_128i_from_mask8(k)); }

BL_INLINE_NODEBUG __m256i simd_256i_from_mask64(__mmask8 k) noexcept { return _mm256_movm_epi64(k); }
BL_INLINE_NODEBUG __m256  simd_256f_from_mask64(__mmask8 k) noexcept { return simd_as_f(simd_256i_from_mask8(k)); }
BL_INLINE_NODEBUG __m256d simd_256d_from_mask64(__mmask8 k) noexcept { return simd_as_d(simd_256i_from_mask8(k)); }

BL_INLINE_NODEBUG __m512i simd_512i_from_mask64(__mmask8 k) noexcept { return _mm512_movm_epi64(k); }
BL_INLINE_NODEBUG __m512  simd_512f_from_mask64(__mmask8 k) noexcept { return simd_as_f(simd_512i_from_mask8(k)); }
BL_INLINE_NODEBUG __m512d simd_512d_from_mask64(__mmask8 k) noexcept { return simd_as_d(simd_512i_from_mask8(k)); }
#endif // BL_TARGET_OPT_AVX512

} // {Internal}

// SIMD - Internal - Shuffle & Permute
// ===================================

namespace Internal {

template<size_t kW> BL_INLINE_NODEBUG typename SimdInfo<kW>::RegI simd_broadcast_u8(const __m128i& a) noexcept;
template<size_t kW> BL_INLINE_NODEBUG typename SimdInfo<kW>::RegI simd_broadcast_u16(const __m128i& a) noexcept;
template<size_t kW> BL_INLINE_NODEBUG typename SimdInfo<kW>::RegI simd_broadcast_u32(const __m128i& a) noexcept;
template<size_t kW> BL_INLINE_NODEBUG typename SimdInfo<kW>::RegI simd_broadcast_u64(const __m128i& a) noexcept;
template<size_t kW> BL_INLINE_NODEBUG typename SimdInfo<kW>::RegF simd_broadcast_f32(const __m128& a) noexcept;
template<size_t kW> BL_INLINE_NODEBUG typename SimdInfo<kW>::RegD simd_broadcast_f64(const __m128d& a) noexcept;

#if defined(BL_TARGET_OPT_SSSE3)
BL_INLINE_NODEBUG __m128i simd_swizzlev_u8(const __m128i& a, const __m128i& b) noexcept { return _mm_shuffle_epi8(a, b); }
#endif // BL_TARGET_OPT_SSSE3

template<uint8_t D, uint8_t C, uint8_t B, uint8_t A>
BL_INLINE_NODEBUG __m128i simd_swizzle_lo_u16(const __m128i& a) noexcept { return _mm_shufflelo_epi16(a, _MM_SHUFFLE(D, C, B, A)); }

template<uint8_t D, uint8_t C, uint8_t B, uint8_t A>
BL_INLINE_NODEBUG __m128i simd_swizzle_hi_u16(const __m128i& a) noexcept { return _mm_shufflehi_epi16(a, _MM_SHUFFLE(D, C, B, A)); }

template<uint8_t D, uint8_t C, uint8_t B, uint8_t A>
BL_INLINE_NODEBUG __m128i simd_swizzle_u16(const __m128i& a) noexcept { return simd_swizzle_hi_u16<D, C, B, A>(simd_swizzle_lo_u16<D, C, B, A>(a)); }

template<uint8_t D, uint8_t C, uint8_t B, uint8_t A>
BL_INLINE_NODEBUG __m128i simd_swizzle_u32(const __m128i& a) noexcept { return _mm_shuffle_epi32(a, _MM_SHUFFLE(D, C, B, A)); }

template<uint8_t B, uint8_t A>
BL_INLINE_NODEBUG __m128i simd_swizzle_u64(const __m128i& a) noexcept { return simd_swizzle_u32<B*2 + 1, B*2, A*2 + 1, A*2>(a); }

#if defined(BL_TARGET_OPT_AVX)
template<uint8_t D, uint8_t C, uint8_t B, uint8_t A>
BL_INLINE_NODEBUG __m128 simd_swizzle_f32(const __m128& a) noexcept { return _mm_shuffle_ps(a, a, _MM_SHUFFLE(D, C, B, A)); }
template<uint8_t B, uint8_t A>
BL_INLINE_NODEBUG __m128d simd_swizzle_f64(const __m128d& a) noexcept { return _mm_shuffle_pd(a, a, (B << 1) | A); }
#else
template<uint8_t D, uint8_t C, uint8_t B, uint8_t A>
BL_INLINE_NODEBUG __m128 simd_swizzle_f32(const __m128& a) noexcept { return simd_cast<__m128>(_mm_shuffle_epi32(simd_cast<__m128i>(a), _MM_SHUFFLE(D, C, B, A))); }
template<uint8_t B, uint8_t A>
BL_INLINE_NODEBUG __m128d simd_swizzle_f64(const __m128d& a) noexcept { return simd_cast<__m128d>(simd_swizzle_u32<B*2 + 1, B*2, A*2 + 1, A*2>(simd_cast<__m128i>(a))); }
#endif // BL_TARGET_OPT_AVX

template<uint8_t D, uint8_t C, uint8_t B, uint8_t A>
BL_INLINE_NODEBUG __m128 simd_shuffle_f32(const __m128& lo, const __m128& hi) noexcept { return _mm_shuffle_ps(lo, hi, _MM_SHUFFLE(D, C, B, A)); }

template<uint8_t D, uint8_t C, uint8_t B, uint8_t A>
BL_INLINE_NODEBUG __m128i simd_shuffle_u32(const __m128i& lo, const __m128i& hi) noexcept { return simd_cast<__m128i>(_mm_shuffle_ps(simd_cast<__m128>(lo), simd_cast<__m128>(hi), _MM_SHUFFLE(D, C, B, A))); }

template<uint8_t B, uint8_t A>
BL_INLINE_NODEBUG __m128d simd_shuffle_f64(const __m128d& lo, const __m128d& hi) noexcept { return _mm_shuffle_pd(lo, hi, (B << 1) | A); }

template<uint8_t B, uint8_t A>
BL_INLINE_NODEBUG __m128i simd_shuffle_u64(const __m128i& lo, const __m128i& hi) noexcept { return simd_cast<__m128i>(_mm_shuffle_pd(simd_cast<__m128d>(lo), simd_cast<__m128d>(hi), (B << 1) | A)); }

#if defined(BL_TARGET_OPT_AVX2) && !defined(BL_SIMD_MISSING_INTRINSICS)
BL_INLINE_NODEBUG __m256i simd_interleave_u128(const __m128i& a, const __m128i& b) noexcept { return _mm256_set_m128i(b, a); }
#elif defined(BL_TARGET_OPT_AVX)
BL_INLINE_NODEBUG __m256i simd_interleave_u128(const __m128i& a, const __m128i& b) noexcept { return _mm256_insertf128_si256(simd_cast<__m256i>(a), b, 1); }
#endif

#if defined(BL_TARGET_OPT_AVX2)
template<uint8_t D, uint8_t C, uint8_t B, uint8_t A>
BL_INLINE_NODEBUG __m256i simd_permute_u64(const __m128i& a) noexcept { return _mm256_permute4x64_epi64(simd_cast<__m256i>(a), _MM_SHUFFLE(D, C, B, A)); }
template<uint8_t D, uint8_t C, uint8_t B, uint8_t A>
BL_INLINE_NODEBUG __m256i simd_permute_u64(const __m256i& a) noexcept { return _mm256_permute4x64_epi64(a, _MM_SHUFFLE(D, C, B, A)); }

template<uint8_t B, uint8_t A>
BL_INLINE_NODEBUG __m256i simd_permute_u128(const __m256i& a, const __m256i& b) noexcept { return _mm256_permute2x128_si256(a, b, ((B & 0xF) << 4) + (A & 0xF)); }
template<uint8_t B, uint8_t A>
BL_INLINE_NODEBUG __m256i simd_permute_u128(const __m256i& a) noexcept { return simd_permute_u128<B, A>(a, a); }
#endif // BL_TARGET_OPT_AVX2

#if defined(BL_TARGET_OPT_AVX2)
template<> BL_INLINE_NODEBUG __m128i simd_broadcast_u8<16>(const __m128i& a) noexcept { return _mm_broadcastb_epi8(a); }
template<> BL_INLINE_NODEBUG __m128i simd_broadcast_u16<16>(const __m128i& a) noexcept { return _mm_broadcastw_epi16(a); }
template<> BL_INLINE_NODEBUG __m128i simd_broadcast_u32<16>(const __m128i& a) noexcept { return _mm_broadcastd_epi32(a); }
template<> BL_INLINE_NODEBUG __m128i simd_broadcast_u64<16>(const __m128i& a) noexcept { return _mm_broadcastq_epi64(a); }
#elif defined(BL_TARGET_OPT_SSSE3)
template<> BL_INLINE_NODEBUG __m128i simd_broadcast_u8<16>(const __m128i& a) noexcept { return _mm_shuffle_epi8(a, _mm_setzero_si128()); }
template<> BL_INLINE_NODEBUG __m128i simd_broadcast_u16<16>(const __m128i& a) noexcept { return _mm_shuffle_epi8(a, bl::common_table.p_0100010001000100.as<__m128i>()); }
template<> BL_INLINE_NODEBUG __m128i simd_broadcast_u32<16>(const __m128i& a) noexcept { return _mm_shuffle_epi32(a, _MM_SHUFFLE(0, 0, 0, 0)); }
template<> BL_INLINE_NODEBUG __m128i simd_broadcast_u64<16>(const __m128i& a) noexcept { return _mm_shuffle_epi32(a, _MM_SHUFFLE(1, 0, 1, 0)); }
#else
template<> BL_INLINE_NODEBUG __m128i simd_broadcast_u16<16>(const __m128i& a) noexcept { return _mm_shuffle_epi32(_mm_unpacklo_epi16(a, a), _MM_SHUFFLE(0, 0, 0, 0)); }
template<> BL_INLINE_NODEBUG __m128i simd_broadcast_u8<16>(const __m128i& a) noexcept { return simd_broadcast_u16<16>(_mm_unpacklo_epi8(a, a)); }
template<> BL_INLINE_NODEBUG __m128i simd_broadcast_u32<16>(const __m128i& a) noexcept { return _mm_shuffle_epi32(a, _MM_SHUFFLE(0, 0, 0, 0)); }
template<> BL_INLINE_NODEBUG __m128i simd_broadcast_u64<16>(const __m128i& a) noexcept { return _mm_shuffle_epi32(a, _MM_SHUFFLE(1, 0, 1, 0)); }
#endif

#if defined(BL_TARGET_OPT_AVX2) && !defined(BL_SIMD_MISSING_INTRINSICS)
template<> BL_INLINE_NODEBUG __m128 simd_broadcast_f32<16>(const __m128& a) noexcept { return _mm_broadcastss_ps(a); }
template<> BL_INLINE_NODEBUG __m128d simd_broadcast_f64<16>(const __m128d& a) noexcept { return _mm_broadcastsd_pd(a); }
#else
template<> BL_INLINE_NODEBUG __m128 simd_broadcast_f32<16>(const __m128& a) noexcept { return simd_cast<__m128>(simd_broadcast_u32<16>(simd_cast<__m128i>(a))); }
template<> BL_INLINE_NODEBUG __m128d simd_broadcast_f64<16>(const __m128d& a) noexcept { return simd_cast<__m128d>(simd_broadcast_u64<16>(simd_cast<__m128i>(a))); }
#endif

BL_INLINE_NODEBUG __m128i simd_dup_lo_u32(const __m128i& a) noexcept { return simd_swizzle_u32<2, 2, 0, 0>(a); }
BL_INLINE_NODEBUG __m128i simd_dup_hi_u32(const __m128i& a) noexcept { return simd_swizzle_u32<3, 3, 1, 1>(a); }

BL_INLINE_NODEBUG __m128i simd_dup_lo_u64(const __m128i& a) noexcept { return simd_swizzle_u64<0, 0>(a); }
BL_INLINE_NODEBUG __m128i simd_dup_hi_u64(const __m128i& a) noexcept { return simd_swizzle_u64<1, 1>(a); }

BL_INLINE_NODEBUG __m128 simd_dup_lo_f32(const __m128& a) noexcept { return simd_swizzle_f32<2, 2, 0, 0>(a); }
BL_INLINE_NODEBUG __m128 simd_dup_hi_f32(const __m128& a) noexcept { return simd_swizzle_f32<3, 3, 1, 1>(a); }

BL_INLINE_NODEBUG __m128 simd_dup_lo_f32x2(const __m128& a) noexcept { return simd_swizzle_f32<1, 0, 1, 0>(a); }
BL_INLINE_NODEBUG __m128 simd_dup_hi_f32x2(const __m128& a) noexcept { return simd_swizzle_f32<3, 2, 3, 2>(a); }

BL_INLINE_NODEBUG __m128d simd_dup_lo_f64(const __m128d& a) noexcept { return simd_swizzle_f64<0, 0>(a); }
BL_INLINE_NODEBUG __m128d simd_dup_hi_f64(const __m128d& a) noexcept { return simd_swizzle_f64<1, 1>(a); }

BL_INLINE_NODEBUG __m128i simd_swap_u32(const __m128i& a) noexcept { return simd_swizzle_u32<2, 3, 0, 1>(a); }
BL_INLINE_NODEBUG __m128 simd_swap_f32(const __m128& a) noexcept { return simd_swizzle_f32<2, 3, 0, 1>(a); }
BL_INLINE_NODEBUG __m128i simd_swap_u64(const __m128i& a) noexcept { return simd_swizzle_u64<0, 1>(a); }
BL_INLINE_NODEBUG __m128d simd_swap_f64(const __m128d& a) noexcept { return simd_swizzle_f64<0, 1>(a); }

BL_INLINE_NODEBUG __m128i simd_interleave_lo_u8(const __m128i& a, const __m128i& b) noexcept { return _mm_unpacklo_epi8(a, b); }
BL_INLINE_NODEBUG __m128i simd_interleave_hi_u8(const __m128i& a, const __m128i& b) noexcept { return _mm_unpackhi_epi8(a, b); }
BL_INLINE_NODEBUG __m128i simd_interleave_lo_u16(const __m128i& a, const __m128i& b) noexcept { return _mm_unpacklo_epi16(a, b); }
BL_INLINE_NODEBUG __m128i simd_interleave_hi_u16(const __m128i& a, const __m128i& b) noexcept { return _mm_unpackhi_epi16(a, b); }
BL_INLINE_NODEBUG __m128i simd_interleave_lo_u32(const __m128i& a, const __m128i& b) noexcept { return _mm_unpacklo_epi32(a, b); }
BL_INLINE_NODEBUG __m128i simd_interleave_hi_u32(const __m128i& a, const __m128i& b) noexcept { return _mm_unpackhi_epi32(a, b); }
BL_INLINE_NODEBUG __m128i simd_interleave_lo_u64(const __m128i& a, const __m128i& b) noexcept { return _mm_unpacklo_epi64(a, b); }
BL_INLINE_NODEBUG __m128i simd_interleave_hi_u64(const __m128i& a, const __m128i& b) noexcept { return _mm_unpackhi_epi64(a, b); }

BL_INLINE_NODEBUG __m128 simd_interleave_lo_f32(const __m128& a, const __m128& b) noexcept { return _mm_unpacklo_ps(a, b); }
BL_INLINE_NODEBUG __m128 simd_interleave_hi_f32(const __m128& a, const __m128& b) noexcept { return _mm_unpackhi_ps(a, b); }
BL_INLINE_NODEBUG __m128d simd_interleave_lo_f64(const __m128d& a, const __m128d& b) noexcept { return _mm_unpacklo_pd(a, b); }
BL_INLINE_NODEBUG __m128d simd_interleave_hi_f64(const __m128d& a, const __m128d& b) noexcept { return _mm_unpackhi_pd(a, b); }

#if defined(BL_TARGET_OPT_SSSE3)
template<int kNumBytes>
BL_INLINE_NODEBUG __m128i simd_alignr_u128(const __m128i& a, const __m128i& b) noexcept { return _mm_alignr_epi8(a, b, kNumBytes); }
#else
template<int kNumBytes>
BL_INLINE_NODEBUG __m128i simd_alignr_u128(const __m128i& a, const __m128i& b) noexcept {
  __m128i a_shifted = _mm_slli_si128(a, (16u - kNumBytes) % 16u);
  __m128i b_shifted = _mm_srli_si128(b, kNumBytes);
  return kNumBytes > 0u ? _mm_or_si128(a_shifted, b_shifted) : a;
}
#endif

#if defined(BL_TARGET_OPT_AVX)
template<uint8_t D, uint8_t C, uint8_t B, uint8_t A>
BL_INLINE_NODEBUG __m256 simd_swizzle_f32(const __m256& a) noexcept { return _mm256_shuffle_ps(a, a, _MM_SHUFFLE(D, C, B, A)); }

template<uint8_t B, uint8_t A>
BL_INLINE_NODEBUG __m256d simd_swizzle_f64(const __m256d& a) noexcept { return _mm256_shuffle_pd(a, a, (B << 3) | (A << 2) | (B << 1) | A); }

#if defined(BL_TARGET_OPT_AVX2)
template<> BL_INLINE_NODEBUG __m256i simd_broadcast_u8<32>(const __m128i& a) noexcept { return _mm256_broadcastb_epi8(a); }
template<> BL_INLINE_NODEBUG __m256i simd_broadcast_u16<32>(const __m128i& a) noexcept { return _mm256_broadcastw_epi16(a); }
template<> BL_INLINE_NODEBUG __m256i simd_broadcast_u32<32>(const __m128i& a) noexcept { return _mm256_broadcastd_epi32(a); }
template<> BL_INLINE_NODEBUG __m256i simd_broadcast_u64<32>(const __m128i& a) noexcept { return _mm256_broadcastq_epi64(a); }
#else
template<> BL_INLINE_NODEBUG __m256i simd_broadcast_u32<32>(const __m128i& a) noexcept { return simd_cast<__m256i>(_mm256_broadcastss_ps(simd_cast<__m128>(a))); }
template<> BL_INLINE_NODEBUG __m256i simd_broadcast_u64<32>(const __m128i& a) noexcept { return simd_cast<__m256i>(_mm256_broadcastsd_pd(simd_cast<__m128d>(a))); }
template<> BL_INLINE_NODEBUG __m256i simd_broadcast_u8<32>(const __m128i& a) noexcept { return simd_broadcast_u64<32>(_mm_shuffle_epi8(a, _mm_setzero_si128())); }
template<> BL_INLINE_NODEBUG __m256i simd_broadcast_u16<32>(const __m128i& a) noexcept { return simd_broadcast_u64<32>(_mm_shufflelo_epi16(a, _MM_SHUFFLE(0, 0, 0, 0))); }
#endif

template<> BL_INLINE_NODEBUG __m256 simd_broadcast_f32<32>(const __m128& a) noexcept { return _mm256_broadcastss_ps(a); }
template<> BL_INLINE_NODEBUG __m256d simd_broadcast_f64<32>(const __m128d& a) noexcept { return _mm256_broadcastsd_pd(a); }

BL_INLINE_NODEBUG __m256 simd_dup_lo_f32(const __m256& a) noexcept { return simd_swizzle_f32<2, 2, 0, 0>(a); }
BL_INLINE_NODEBUG __m256 simd_dup_hi_f32(const __m256& a) noexcept { return simd_swizzle_f32<3, 3, 1, 1>(a); }

BL_INLINE_NODEBUG __m256 simd_dup_lo_f32x2(const __m256& a) noexcept { return simd_swizzle_f32<1, 0, 1, 0>(a); }
BL_INLINE_NODEBUG __m256 simd_dup_hi_f32x2(const __m256& a) noexcept { return simd_swizzle_f32<3, 2, 3, 2>(a); }

BL_INLINE_NODEBUG __m256d simd_dup_lo_f64(const __m256d& a) noexcept { return simd_swizzle_f64<0, 0>(a); }
BL_INLINE_NODEBUG __m256d simd_dup_hi_f64(const __m256d& a) noexcept { return simd_swizzle_f64<1, 1>(a); }

BL_INLINE_NODEBUG __m256 simd_swap_f32(const __m256& a) noexcept { return simd_swizzle_f32<2, 3, 0, 1>(a); }
BL_INLINE_NODEBUG __m256d simd_swap_f64(const __m256d& a) noexcept { return simd_swizzle_f64<0, 1>(a); }

BL_INLINE_NODEBUG __m256 simd_interleave_lo_f32(const __m256& a, const __m256& b) noexcept { return _mm256_unpacklo_ps(a, b); }
BL_INLINE_NODEBUG __m256 simd_interleave_hi_f32(const __m256& a, const __m256& b) noexcept { return _mm256_unpackhi_ps(a, b); }
BL_INLINE_NODEBUG __m256d simd_interleave_lo_f64(const __m256d& a, const __m256d& b) noexcept { return _mm256_unpacklo_pd(a, b); }
BL_INLINE_NODEBUG __m256d simd_interleave_hi_f64(const __m256d& a, const __m256d& b) noexcept { return _mm256_unpackhi_pd(a, b); }
#endif // BL_TARGET_OPT_AVX

#if defined(BL_TARGET_OPT_AVX2)
BL_INLINE_NODEBUG __m256i simd_swizzlev_u8(const __m256i& a, const __m256i& b) noexcept { return _mm256_shuffle_epi8(a, b); }

template<uint8_t D, uint8_t C, uint8_t B, uint8_t A>
BL_INLINE_NODEBUG __m256i simd_swizzle_lo_u16(const __m256i& a) noexcept { return _mm256_shufflelo_epi16(a, _MM_SHUFFLE(D, C, B, A)); }

template<uint8_t D, uint8_t C, uint8_t B, uint8_t A>
BL_INLINE_NODEBUG __m256i simd_swizzle_hi_u16(const __m256i& a) noexcept { return _mm256_shufflehi_epi16(a, _MM_SHUFFLE(D, C, B, A)); }

template<uint8_t D, uint8_t C, uint8_t B, uint8_t A>
BL_INLINE_NODEBUG __m256i simd_swizzle_u16(const __m256i& a) noexcept { return simd_swizzle_hi_u16<D, C, B, A>(simd_swizzle_lo_u16<D, C, B, A>(a)); }

template<uint8_t D, uint8_t C, uint8_t B, uint8_t A>
BL_INLINE_NODEBUG __m256i simd_swizzle_u32(const __m256i& a) noexcept { return _mm256_shuffle_epi32(a, _MM_SHUFFLE(D, C, B, A)); }

template<uint8_t B, uint8_t A>
BL_INLINE_NODEBUG __m256i simd_swizzle_u64(const __m256i& a) noexcept { return simd_swizzle_u32<B*2 + 1, B*2, A*2 + 1, A*2>(a); }

template<uint8_t D, uint8_t C, uint8_t B, uint8_t A>
BL_INLINE_NODEBUG __m256 simd_shuffle_f32(const __m256& lo, const __m256& hi) noexcept { return _mm256_shuffle_ps(lo, hi, _MM_SHUFFLE(D, C, B, A)); }

template<uint8_t D, uint8_t C, uint8_t B, uint8_t A>
BL_INLINE_NODEBUG __m256i simd_shuffle_u32(const __m256i& lo, const __m256i& hi) noexcept { return simd_cast<__m256i>(_mm256_shuffle_ps(simd_cast<__m256>(lo), simd_cast<__m256>(hi), _MM_SHUFFLE(D, C, B, A))); }

template<uint8_t B, uint8_t A>
BL_INLINE_NODEBUG __m256d simd_shuffle_f64(const __m256d& lo, const __m256d& hi) noexcept { return _mm256_shuffle_pd(lo, hi, (B << 3) | (A << 2) | (B << 1) | A); }

template<uint8_t B, uint8_t A>
BL_INLINE_NODEBUG __m256i simd_shuffle_u64(const __m256i& lo, const __m256i& hi) noexcept { return simd_cast<__m256i>(simd_shuffle_f64<B, A>(simd_cast<__m256d>(lo), simd_cast<__m256d>(hi))); }

BL_INLINE_NODEBUG __m256i simd_broadcast256_u8(const __m128i& a) noexcept { return _mm256_broadcastb_epi8(a); }
BL_INLINE_NODEBUG __m256i simd_broadcast256_u16(const __m128i& a) noexcept { return _mm256_broadcastw_epi16(a); }
BL_INLINE_NODEBUG __m256i simd_broadcast256_u32(const __m128i& a) noexcept { return _mm256_broadcastd_epi32(a); }
BL_INLINE_NODEBUG __m256i simd_broadcast256_u64(const __m128i& a) noexcept { return _mm256_broadcastq_epi64(a); }

BL_INLINE_NODEBUG __m256 simd_broadcast256_f32(const __m256& a) noexcept { return simd_cast<__m256>(simd_broadcast256_u32(simd_cast<__m128i>(a))); }
BL_INLINE_NODEBUG __m256d simd_broadcast256_f64(const __m256d& a) noexcept { return simd_cast<__m256d>(simd_broadcast256_u64(simd_cast<__m128i>(a))); }

BL_INLINE_NODEBUG __m256i simd_dup_lo_u32(const __m256i& a) noexcept { return simd_swizzle_u32<2, 2, 0, 0>(a); }
BL_INLINE_NODEBUG __m256i simd_dup_hi_u32(const __m256i& a) noexcept { return simd_swizzle_u32<3, 3, 1, 1>(a); }

BL_INLINE_NODEBUG __m256i simd_dup_lo_u64(const __m256i& a) noexcept { return simd_swizzle_u64<0, 0>(a); }
BL_INLINE_NODEBUG __m256i simd_dup_hi_u64(const __m256i& a) noexcept { return simd_swizzle_u64<1, 1>(a); }

BL_INLINE_NODEBUG __m256i simd_swap_u32(const __m256i& a) noexcept { return simd_swizzle_u32<2, 3, 0, 1>(a); }
BL_INLINE_NODEBUG __m256i simd_swap_u64(const __m256i& a) noexcept { return simd_swizzle_u64<0, 1>(a); }

BL_INLINE_NODEBUG __m256i simd_interleave_lo_u8(const __m256i& a, const __m256i& b) noexcept { return _mm256_unpacklo_epi8(a, b); }
BL_INLINE_NODEBUG __m256i simd_interleave_hi_u8(const __m256i& a, const __m256i& b) noexcept { return _mm256_unpackhi_epi8(a, b); }
BL_INLINE_NODEBUG __m256i simd_interleave_lo_u16(const __m256i& a, const __m256i& b) noexcept { return _mm256_unpacklo_epi16(a, b); }
BL_INLINE_NODEBUG __m256i simd_interleave_hi_u16(const __m256i& a, const __m256i& b) noexcept { return _mm256_unpackhi_epi16(a, b); }
BL_INLINE_NODEBUG __m256i simd_interleave_lo_u32(const __m256i& a, const __m256i& b) noexcept { return _mm256_unpacklo_epi32(a, b); }
BL_INLINE_NODEBUG __m256i simd_interleave_hi_u32(const __m256i& a, const __m256i& b) noexcept { return _mm256_unpackhi_epi32(a, b); }
BL_INLINE_NODEBUG __m256i simd_interleave_lo_u64(const __m256i& a, const __m256i& b) noexcept { return _mm256_unpacklo_epi64(a, b); }
BL_INLINE_NODEBUG __m256i simd_interleave_hi_u64(const __m256i& a, const __m256i& b) noexcept { return _mm256_unpackhi_epi64(a, b); }

template<int kNumBytes>
BL_INLINE_NODEBUG __m256i simd_alignr_u128(const __m256i& a, const __m256i& b) noexcept { return _mm256_alignr_epi8(a, b, kNumBytes); }

#endif // BL_TARGET_OPT_AVX2

#if defined(BL_TARGET_OPT_AVX512)
BL_INLINE_NODEBUG __m512i simd_swizzlev_u8(const __m512i& a, const __m512i& b) noexcept { return _mm512_shuffle_epi8(a, b); }

template<uint8_t D, uint8_t C, uint8_t B, uint8_t A>
BL_INLINE_NODEBUG __m512i simd_swizzle_lo_u16(const __m512i& a) noexcept { return _mm512_shufflelo_epi16(a, _MM_SHUFFLE(D, C, B, A)); }

template<uint8_t D, uint8_t C, uint8_t B, uint8_t A>
BL_INLINE_NODEBUG __m512i simd_swizzle_hi_u16(const __m512i& a) noexcept { return _mm512_shufflehi_epi16(a, _MM_SHUFFLE(D, C, B, A)); }

template<uint8_t D, uint8_t C, uint8_t B, uint8_t A>
BL_INLINE_NODEBUG __m512i simd_swizzle_u16(const __m512i& a) noexcept { return simd_swizzle_hi_u16<D, C, B, A>(simd_swizzle_lo_u16<D, C, B, A>(a)); }

template<uint8_t D, uint8_t C, uint8_t B, uint8_t A>
BL_INLINE_NODEBUG __m512i simd_swizzle_u32(const __m512i& a) noexcept { return _mm512_shuffle_epi32(a, _MM_PERM_ENUM(_MM_SHUFFLE(D, C, B, A))); }

template<uint8_t B, uint8_t A>
BL_INLINE_NODEBUG __m512i simd_swizzle_u64(const __m512i& a) noexcept { return simd_swizzle_u32<B*2 + 1, B*2, A*2 + 1, A*2>(a); }

template<uint8_t D, uint8_t C, uint8_t B, uint8_t A>
BL_INLINE_NODEBUG __m512 simd_shuffle_f32(const __m512& lo, const __m512& hi) noexcept { return _mm512_shuffle_ps(lo, hi, _MM_SHUFFLE(D, C, B, A)); }

template<uint8_t D, uint8_t C, uint8_t B, uint8_t A>
BL_INLINE_NODEBUG __m512i simd_shuffle_u32(const __m512i& lo, const __m512i& hi) noexcept { return simd_cast<__m512i>(_mm512_shuffle_ps(simd_cast<__m512>(lo), simd_cast<__m512>(hi), _MM_SHUFFLE(D, C, B, A))); }

template<uint8_t B, uint8_t A>
BL_INLINE_NODEBUG __m512d simd_shuffle_f64(const __m512d& lo, const __m512d& hi) noexcept { return _mm512_shuffle_pd(lo, hi, (B << 1) | A); }

template<uint8_t B, uint8_t A>
BL_INLINE_NODEBUG __m512i simd_shuffle_u64(const __m512i& lo, const __m512i& hi) noexcept { return simd_cast<__m512i>(_mm512_shuffle_pd(simd_cast<__m512d>(lo), simd_cast<__m512d>(hi), (B << 1) | A)); }

BL_INLINE_NODEBUG __m512i simd_broadcast512_u8(const __m128i& a) noexcept { return _mm512_broadcastb_epi8(a); }
BL_INLINE_NODEBUG __m512i simd_broadcast512_u16(const __m128i& a) noexcept { return _mm512_broadcastw_epi16(a); }
BL_INLINE_NODEBUG __m512i simd_broadcast512_u32(const __m128i& a) noexcept { return _mm512_broadcastd_epi32(a); }
BL_INLINE_NODEBUG __m512i simd_broadcast512_u64(const __m128i& a) noexcept { return _mm512_broadcastq_epi64(a); }

BL_INLINE_NODEBUG __m512 simd_broadcast512_f32(const __m128& a) noexcept { return simd_cast<__m512>(simd_broadcast512_u32(simd_cast<__m128i>(a))); }
BL_INLINE_NODEBUG __m512d simd_broadcast512_f64(const __m128d& a) noexcept { return simd_cast<__m512d>(simd_broadcast512_u64(simd_cast<__m128i>(a))); }

template<> BL_INLINE_NODEBUG __m512i simd_broadcast_u8<64>(const __m128i& a) noexcept { return _mm512_broadcastb_epi8(a); }
template<> BL_INLINE_NODEBUG __m512i simd_broadcast_u16<64>(const __m128i& a) noexcept { return _mm512_broadcastw_epi16(a); }
template<> BL_INLINE_NODEBUG __m512i simd_broadcast_u32<64>(const __m128i& a) noexcept { return _mm512_broadcastd_epi32(a); }
template<> BL_INLINE_NODEBUG __m512i simd_broadcast_u64<64>(const __m128i& a) noexcept { return _mm512_broadcastq_epi64(a); }
template<> BL_INLINE_NODEBUG __m512 simd_broadcast_f32<64>(const __m128& a) noexcept { return _mm512_broadcastss_ps(a); }
template<> BL_INLINE_NODEBUG __m512d simd_broadcast_f64<64>(const __m128d& a) noexcept { return _mm512_broadcastsd_pd(a); }

BL_INLINE_NODEBUG __m512i simd_dup_lo_u32(const __m512i& a) noexcept { return simd_swizzle_u32<2, 2, 0, 0>(a); }
BL_INLINE_NODEBUG __m512i simd_dup_hi_u32(const __m512i& a) noexcept { return simd_swizzle_u32<3, 3, 1, 1>(a); }

BL_INLINE_NODEBUG __m512i simd_dup_lo_u64(const __m512i& a) noexcept { return simd_swizzle_u64<0, 0>(a); }
BL_INLINE_NODEBUG __m512i simd_dup_hi_u64(const __m512i& a) noexcept { return simd_swizzle_u64<1, 1>(a); }

BL_INLINE_NODEBUG __m512i simd_swap_u32(const __m512i& a) noexcept { return simd_swizzle_u32<2, 3, 0, 1>(a); }
BL_INLINE_NODEBUG __m512i simd_swap_u64(const __m512i& a) noexcept { return simd_swizzle_u64<0, 1>(a); }

BL_INLINE_NODEBUG __m512i simd_interleave_lo_u8(const __m512i& a, const __m512i& b) noexcept { return _mm512_unpacklo_epi8(a, b); }
BL_INLINE_NODEBUG __m512i simd_interleave_hi_u8(const __m512i& a, const __m512i& b) noexcept { return _mm512_unpackhi_epi8(a, b); }
BL_INLINE_NODEBUG __m512i simd_interleave_lo_u16(const __m512i& a, const __m512i& b) noexcept { return _mm512_unpacklo_epi16(a, b); }
BL_INLINE_NODEBUG __m512i simd_interleave_hi_u16(const __m512i& a, const __m512i& b) noexcept { return _mm512_unpackhi_epi16(a, b); }
BL_INLINE_NODEBUG __m512i simd_interleave_lo_u32(const __m512i& a, const __m512i& b) noexcept { return _mm512_unpacklo_epi32(a, b); }
BL_INLINE_NODEBUG __m512i simd_interleave_hi_u32(const __m512i& a, const __m512i& b) noexcept { return _mm512_unpackhi_epi32(a, b); }
BL_INLINE_NODEBUG __m512i simd_interleave_lo_u64(const __m512i& a, const __m512i& b) noexcept { return _mm512_unpacklo_epi64(a, b); }
BL_INLINE_NODEBUG __m512i simd_interleave_hi_u64(const __m512i& a, const __m512i& b) noexcept { return _mm512_unpackhi_epi64(a, b); }

template<int kNumBytes>
BL_INLINE_NODEBUG __m512i simd_alignr_u128(const __m512i& a, const __m512i& b) noexcept { return _mm512_alignr_epi8(a, b, kNumBytes); }

#endif // BL_TARGET_OPT_AVX512

} // {Internal}

// SIMD - Internal - Integer Packing & Unpacking
// =============================================

namespace Internal {

BL_INLINE_NODEBUG __m128i simd_packs_128_i16_i8(const __m128i& a, const __m128i& b) noexcept { return _mm_packs_epi16(a, b); }
BL_INLINE_NODEBUG __m128i simd_packs_128_i16_u8(const __m128i& a, const __m128i& b) noexcept { return _mm_packus_epi16(a, b); }
BL_INLINE_NODEBUG __m128i simd_packs_128_i32_i16(const __m128i& a, const __m128i& b) noexcept { return _mm_packs_epi32(a, b); }

BL_INLINE_NODEBUG __m128i simd_packs_128_i16_i8(const __m128i& a) noexcept { return _mm_packs_epi16(a, a); }
BL_INLINE_NODEBUG __m128i simd_packs_128_i16_u8(const __m128i& a) noexcept { return _mm_packus_epi16(a, a); }
BL_INLINE_NODEBUG __m128i simd_packs_128_i32_i16(const __m128i& a) noexcept { return _mm_packs_epi32(a, a); }

#if defined(BL_TARGET_OPT_SSE4_1)
BL_INLINE_NODEBUG __m128i simd_packs_128_i32_u16(const __m128i& a) noexcept { return _mm_packus_epi32(a, a); }
BL_INLINE_NODEBUG __m128i simd_packs_128_i32_u16(const __m128i& a, const __m128i& b) noexcept { return _mm_packus_epi32(a, b); }
#else
BL_INLINE_NODEBUG __m128i simd_packs_128_i32_u16(const __m128i& a) noexcept {
  __m128i a_shifted = _mm_srai_epi32(_mm_slli_epi32(a, 16), 16);
  return _mm_packs_epi32(a_shifted, a_shifted);
}
BL_INLINE_NODEBUG __m128i simd_packs_128_i32_u16(const __m128i& a, const __m128i& b) noexcept {
  __m128i a_shifted = _mm_srai_epi32(_mm_slli_epi32(a, 16), 16);
  __m128i b_shifted = _mm_srai_epi32(_mm_slli_epi32(b, 16), 16);
  return _mm_packs_epi32(a_shifted, b_shifted);
}
#endif

BL_INLINE_NODEBUG __m128i simd_packs_128_i32_i8(const __m128i& a) noexcept { return simd_packs_128_i16_i8(_mm_packs_epi32(a, a)); }
BL_INLINE_NODEBUG __m128i simd_packs_128_i32_u8(const __m128i& a) noexcept { return simd_packs_128_i16_u8(_mm_packs_epi32(a, a)); }

BL_INLINE_NODEBUG __m128i simd_packs_128_i32_i8(const __m128i& a, const __m128i& b) noexcept { return simd_packs_128_i16_i8(_mm_packs_epi32(a, b)); }
BL_INLINE_NODEBUG __m128i simd_packs_128_i32_u8(const __m128i& a, const __m128i& b) noexcept { return simd_packs_128_i32_i16(_mm_packs_epi32(a, b)); }

BL_INLINE_NODEBUG __m128i simd_packs_128_i32_i8(const __m128i& a, const __m128i& b, const __m128i& c, const __m128i& d) noexcept { return _mm_packs_epi16(_mm_packs_epi32(a, b), _mm_packs_epi32(c, d)); }
BL_INLINE_NODEBUG __m128i simd_packs_128_i32_u8(const __m128i& a, const __m128i& b, const __m128i& c, const __m128i& d) noexcept { return _mm_packus_epi16(_mm_packs_epi32(a, b), _mm_packs_epi32(c, d)); }

// These assume that HI bytes of all inputs are always zero, so the implementation
// can decide between packing with signed/unsigned saturation or vector swizzling.
BL_INLINE_NODEBUG __m128i simd_packz_128_u16_u8(const __m128i& a) noexcept { return _mm_packus_epi16(a, a); }
BL_INLINE_NODEBUG __m128i simd_packz_128_u16_u8(const __m128i& a, const __m128i& b) noexcept { return _mm_packus_epi16(a, b); }

#if defined(BL_TARGET_OPT_SSE4_1) || !defined(BL_TARGET_OPT_SSSE3)
BL_INLINE_NODEBUG __m128i simd_packz_128_u32_u16(const __m128i& a) noexcept { return simd_packs_128_i32_u16(a); }
BL_INLINE_NODEBUG __m128i simd_packz_128_u32_u16(const __m128i& a, const __m128i& b) noexcept { return simd_packs_128_i32_u16(a, b); }
#else
BL_INLINE_NODEBUG __m128i simd_packz_128_u32_u16(const __m128i& a) noexcept {
  return simd_swizzlev_u8(a, vec_const<__m128i>(&bl::common_table.swizu8_xx76xx54xx32xx10_to_7654321076543210));
}

BL_INLINE_NODEBUG __m128i simd_packz_128_u32_u16(const __m128i& a, const __m128i& b) noexcept {
  __m128i a_lo = simd_swizzlev_u8(a, vec_const<__m128i>(&bl::common_table.swizu8_xx76xx54xx32xx10_to_7654321076543210));
  __m128i b_lo = simd_swizzlev_u8(b, vec_const<__m128i>(&bl::common_table.swizu8_xx76xx54xx32xx10_to_7654321076543210));
  return _mm_unpacklo_epi64(a_lo, b_lo);
}
#endif

#if defined(BL_TARGET_OPT_SSSE3)
BL_INLINE_NODEBUG __m128i simd_packz_128_u32_u8(const __m128i& a) noexcept { return simd_swizzlev_u8(a, vec_const<__m128i>(&bl::common_table.swizu8_xxx3xxx2xxx1xxx0_to_3210321032103210)); }
#else
BL_INLINE_NODEBUG __m128i simd_packz_128_u32_u8(const __m128i& a) noexcept { return simd_packs_128_i16_u8(_mm_packs_epi32(a, a)); }
#endif

BL_INLINE_NODEBUG __m128i simd_packz_128_u32_u8(const __m128i& a, const __m128i& b) noexcept { return simd_packz_128_u16_u8(_mm_packs_epi32(a, b)); }
BL_INLINE_NODEBUG __m128i simd_packz_128_u32_u8(const __m128i& a, const __m128i& b, const __m128i& c, const __m128i& d) noexcept { return _mm_packus_epi16(_mm_packs_epi32(a, b), _mm_packs_epi32(c, d)); }

#if defined(BL_TARGET_OPT_SSE4_1)
BL_INLINE_NODEBUG __m128i simd_unpack_lo64_i8_i16(const __m128i& a) noexcept { return _mm_cvtepi8_epi16(a); }
BL_INLINE_NODEBUG __m128i simd_unpack_lo64_u8_u16(const __m128i& a) noexcept { return _mm_cvtepu8_epi16(a); }
BL_INLINE_NODEBUG __m128i simd_unpack_lo64_i16_i32(const __m128i& a) noexcept { return _mm_cvtepi16_epi32(a); }
BL_INLINE_NODEBUG __m128i simd_unpack_lo64_u16_u32(const __m128i& a) noexcept { return _mm_cvtepu16_epi32(a); }
BL_INLINE_NODEBUG __m128i simd_unpack_lo64_i32_i64(const __m128i& a) noexcept { return _mm_cvtepi32_epi64(a); }
BL_INLINE_NODEBUG __m128i simd_unpack_lo64_u32_u64(const __m128i& a) noexcept { return _mm_cvtepu32_epi64(a); }
BL_INLINE_NODEBUG __m128i simd_unpack_lo32_i8_i32(const __m128i& a) noexcept { return _mm_cvtepi8_epi32(a); }
BL_INLINE_NODEBUG __m128i simd_unpack_lo32_u8_u32(const __m128i& a) noexcept { return _mm_cvtepu8_epi32(a); }
#else
BL_INLINE_NODEBUG __m128i simd_unpack_lo64_i8_i16(const __m128i& a) noexcept { return _mm_srai_epi16(_mm_unpacklo_epi8(a, a), 8); }
BL_INLINE_NODEBUG __m128i simd_unpack_lo64_u8_u16(const __m128i& a) noexcept { return _mm_unpacklo_epi8(a, _mm_setzero_si128()); }
BL_INLINE_NODEBUG __m128i simd_unpack_lo64_i16_i32(const __m128i& a) noexcept { return _mm_srai_epi32(_mm_unpacklo_epi16(a, a), 16); }
BL_INLINE_NODEBUG __m128i simd_unpack_lo64_u16_u32(const __m128i& a) noexcept { return _mm_unpacklo_epi16(a, _mm_setzero_si128()); }
BL_INLINE_NODEBUG __m128i simd_unpack_lo64_i32_i64(const __m128i& a) noexcept { return _mm_unpacklo_epi32(a, _mm_srai_epi32(a, 31)); }
BL_INLINE_NODEBUG __m128i simd_unpack_lo64_u32_u64(const __m128i& a) noexcept { return _mm_unpacklo_epi32(a, _mm_setzero_si128()); }

BL_INLINE_NODEBUG __m128i simd_unpack_lo32_i8_i32(const __m128i& a) noexcept {
  __m128i x = _mm_unpacklo_epi8(a, a);
  __m128i y = _mm_unpacklo_epi8(x, x);
  return _mm_srai_epi32(y, 24);
}

BL_INLINE_NODEBUG __m128i simd_unpack_lo32_u8_u32(const __m128i& a) noexcept {
  return simd_unpack_lo64_u16_u32(simd_unpack_lo64_u8_u16(a));
}
#endif

BL_INLINE_NODEBUG __m128i simd_unpack_hi64_i8_i16(const __m128i& a) noexcept { return _mm_srai_epi16(_mm_unpackhi_epi8(a, a), 8); }
BL_INLINE_NODEBUG __m128i simd_unpack_hi64_u8_u16(const __m128i& a) noexcept { return _mm_unpackhi_epi8(a, _mm_setzero_si128()); }
BL_INLINE_NODEBUG __m128i simd_unpack_hi64_i16_i32(const __m128i& a) noexcept { return _mm_srai_epi32(_mm_unpackhi_epi16(a, a), 16); }
BL_INLINE_NODEBUG __m128i simd_unpack_hi64_u16_u32(const __m128i& a) noexcept { return _mm_unpackhi_epi16(a, _mm_setzero_si128()); }
BL_INLINE_NODEBUG __m128i simd_unpack_hi64_i32_i64(const __m128i& a) noexcept { return _mm_unpackhi_epi32(a, _mm_srai_epi32(a, 31)); }
BL_INLINE_NODEBUG __m128i simd_unpack_hi64_u32_u64(const __m128i& a) noexcept { return _mm_unpackhi_epi32(a, _mm_setzero_si128()); }

BL_INLINE_NODEBUG __m128i simd_movw_i8_i16(const __m128i& a) noexcept { return simd_unpack_lo64_i8_i16(a); }
BL_INLINE_NODEBUG __m128i simd_movw_u8_u16(const __m128i& a) noexcept { return simd_unpack_lo64_u8_u16(a); }
BL_INLINE_NODEBUG __m128i simd_movw_i16_i32(const __m128i& a) noexcept { return simd_unpack_lo64_i16_i32(a); }
BL_INLINE_NODEBUG __m128i simd_movw_u16_u32(const __m128i& a) noexcept { return simd_unpack_lo64_u16_u32(a); }
BL_INLINE_NODEBUG __m128i simd_movw_i32_i64(const __m128i& a) noexcept { return simd_unpack_lo64_i32_i64(a); }
BL_INLINE_NODEBUG __m128i simd_movw_u32_u64(const __m128i& a) noexcept { return simd_unpack_lo64_u32_u64(a); }
BL_INLINE_NODEBUG __m128i simd_movw_i8_i32(const __m128i& a) noexcept { return simd_unpack_lo32_i8_i32(a); }
BL_INLINE_NODEBUG __m128i simd_movw_u8_u32(const __m128i& a) noexcept { return simd_unpack_lo32_u8_u32(a); }

#if defined(BL_TARGET_OPT_AVX2)
BL_INLINE_NODEBUG __m256i simd_packs_128_i16_i8(const __m256i& a) noexcept { return _mm256_packs_epi16(a, a); }
BL_INLINE_NODEBUG __m256i simd_packs_128_i16_u8(const __m256i& a) noexcept { return _mm256_packus_epi16(a, a); }
BL_INLINE_NODEBUG __m256i simd_packs_128_i32_i16(const __m256i& a) noexcept { return _mm256_packs_epi32(a, a); }
BL_INLINE_NODEBUG __m256i simd_packs_128_i32_u16(const __m256i& a) noexcept { return _mm256_packus_epi32(a, a); }

BL_INLINE_NODEBUG __m256i simd_packs_128_i16_i8(const __m256i& a, const __m256i& b) noexcept { return _mm256_packs_epi16(a, b); }
BL_INLINE_NODEBUG __m256i simd_packs_128_i16_u8(const __m256i& a, const __m256i& b) noexcept { return _mm256_packus_epi16(a, b); }
BL_INLINE_NODEBUG __m256i simd_packs_128_i32_i16(const __m256i& a, const __m256i& b) noexcept { return _mm256_packs_epi32(a, b); }
BL_INLINE_NODEBUG __m256i simd_packs_128_i32_u16(const __m256i& a, const __m256i& b) noexcept { return _mm256_packus_epi32(a, b); }

BL_INLINE_NODEBUG __m256i simd_packs_128_i32_i8(const __m256i& a) noexcept { return simd_packs_128_i16_i8(_mm256_packs_epi32(a, a)); }
BL_INLINE_NODEBUG __m256i simd_packs_128_i32_u8(const __m256i& a) noexcept { return simd_packs_128_i16_u8(_mm256_packs_epi32(a, a)); }

BL_INLINE_NODEBUG __m256i simd_packs_128_i32_i8(const __m256i& a, const __m256i& b) noexcept { return simd_packs_128_i16_i8(_mm256_packs_epi32(a, b)); }
BL_INLINE_NODEBUG __m256i simd_packs_128_i32_u8(const __m256i& a, const __m256i& b) noexcept { return simd_packs_128_i16_u8(_mm256_packs_epi32(a, b)); }

BL_INLINE_NODEBUG __m256i simd_packs_128_i32_i8(const __m256i& a, const __m256i& b, const __m256i& c, const __m256i& d) noexcept { return _mm256_packs_epi16(_mm256_packs_epi32(a, b), _mm256_packs_epi32(c, d)); }
BL_INLINE_NODEBUG __m256i simd_packs_128_i32_u8(const __m256i& a, const __m256i& b, const __m256i& c, const __m256i& d) noexcept { return _mm256_packus_epi16(_mm256_packs_epi32(a, b), _mm256_packs_epi32(c, d)); }

BL_INLINE_NODEBUG __m256i simd_packz_128_u32_u16(const __m256i& a) noexcept { return _mm256_packus_epi32(a, a); }
BL_INLINE_NODEBUG __m256i simd_packz_128_u32_u16(const __m256i& a, const __m256i& b) noexcept { return _mm256_packus_epi32(a, b); }

BL_INLINE_NODEBUG __m256i simd_packz_128_u32_u8(const __m256i& a) noexcept { return simd_packs_128_i16_u8(_mm256_packs_epi32(a, a)); }
BL_INLINE_NODEBUG __m256i simd_packz_128_u32_u8(const __m256i& a, const __m256i& b) noexcept { return simd_packs_128_i16_u8(_mm256_packs_epi32(a, b)); }
BL_INLINE_NODEBUG __m256i simd_packz_128_u32_u8(const __m256i& a, const __m256i& b, const __m256i& c, const __m256i& d) noexcept { return _mm256_packus_epi16(_mm256_packs_epi32(a, b), _mm256_packs_epi32(c, d)); }

BL_INLINE_NODEBUG __m256i simd_unpack_lo64_i8_i16(const __m256i& a) noexcept { return _mm256_srai_epi16(_mm256_unpacklo_epi8(a, a), 8); }
BL_INLINE_NODEBUG __m256i simd_unpack_lo64_u8_u16(const __m256i& a) noexcept { return _mm256_unpacklo_epi8(a, _mm256_setzero_si256()); }
BL_INLINE_NODEBUG __m256i simd_unpack_lo64_u8_u32(const __m256i& a) noexcept { return _mm256_unpacklo_epi8(a, _mm256_setzero_si256()); }
BL_INLINE_NODEBUG __m256i simd_unpack_lo64_i16_i32(const __m256i& a) noexcept { return _mm256_srai_epi32(_mm256_unpacklo_epi16(a, a), 16); }
BL_INLINE_NODEBUG __m256i simd_unpack_lo64_u16_u32(const __m256i& a) noexcept { return _mm256_unpacklo_epi16(a, _mm256_setzero_si256()); }
BL_INLINE_NODEBUG __m256i simd_unpack_lo64_i32_i64(const __m256i& a) noexcept { return _mm256_unpacklo_epi32(a, _mm256_srai_epi32(a, 31)); }
BL_INLINE_NODEBUG __m256i simd_unpack_lo64_u32_u64(const __m256i& a) noexcept { return _mm256_unpacklo_epi32(a, _mm256_setzero_si256()); }

BL_INLINE_NODEBUG __m256i simd_unpack_hi64_i8_i16(const __m256i& a) noexcept { return _mm256_srai_epi16(_mm256_unpackhi_epi8(a, a), 8); }
BL_INLINE_NODEBUG __m256i simd_unpack_hi64_u8_u16(const __m256i& a) noexcept { return _mm256_unpackhi_epi8(a, _mm256_setzero_si256()); }
BL_INLINE_NODEBUG __m256i simd_unpack_hi64_i16_i32(const __m256i& a) noexcept { return _mm256_srai_epi32(_mm256_unpackhi_epi16(a, a), 16); }
BL_INLINE_NODEBUG __m256i simd_unpack_hi64_u16_u32(const __m256i& a) noexcept { return _mm256_unpackhi_epi16(a, _mm256_setzero_si256()); }
BL_INLINE_NODEBUG __m256i simd_unpack_hi64_i32_i64(const __m256i& a) noexcept { return _mm256_unpackhi_epi32(a, _mm256_srai_epi32(a, 31)); }
BL_INLINE_NODEBUG __m256i simd_unpack_hi64_u32_u64(const __m256i& a) noexcept { return _mm256_unpackhi_epi32(a, _mm256_setzero_si256()); }

BL_INLINE_NODEBUG __m256i simd_movw_i8_i16(const __m256i& a) noexcept { return _mm256_cvtepi8_epi16(simd_cast<__m128i>(a)); }
BL_INLINE_NODEBUG __m256i simd_movw_u8_u16(const __m256i& a) noexcept { return _mm256_cvtepu8_epi16(simd_cast<__m128i>(a)); }
BL_INLINE_NODEBUG __m256i simd_movw_i8_i32(const __m256i& a) noexcept { return _mm256_cvtepi8_epi32(simd_cast<__m128i>(a)); }
BL_INLINE_NODEBUG __m256i simd_movw_u8_u32(const __m256i& a) noexcept { return _mm256_cvtepu8_epi32(simd_cast<__m128i>(a)); }
BL_INLINE_NODEBUG __m256i simd_movw_i8_i64(const __m256i& a) noexcept { return _mm256_cvtepi8_epi64(simd_cast<__m128i>(a)); }
BL_INLINE_NODEBUG __m256i simd_movw_u8_u64(const __m256i& a) noexcept { return _mm256_cvtepu8_epi64(simd_cast<__m128i>(a)); }
BL_INLINE_NODEBUG __m256i simd_movw_i16_i32(const __m256i& a) noexcept { return _mm256_cvtepi16_epi32(simd_cast<__m128i>(a)); }
BL_INLINE_NODEBUG __m256i simd_movw_u16_u32(const __m256i& a) noexcept { return _mm256_cvtepu16_epi32(simd_cast<__m128i>(a)); }
BL_INLINE_NODEBUG __m256i simd_movw_i16_i64(const __m256i& a) noexcept { return _mm256_cvtepi16_epi64(simd_cast<__m128i>(a)); }
BL_INLINE_NODEBUG __m256i simd_movw_u16_u64(const __m256i& a) noexcept { return _mm256_cvtepu16_epi64(simd_cast<__m128i>(a)); }
BL_INLINE_NODEBUG __m256i simd_movw_i32_i64(const __m256i& a) noexcept { return _mm256_cvtepi32_epi64(simd_cast<__m128i>(a)); }
BL_INLINE_NODEBUG __m256i simd_movw_u32_u64(const __m256i& a) noexcept { return _mm256_cvtepu32_epi64(simd_cast<__m128i>(a)); }
#endif // BL_TARGET_OPT_AVX2

#if defined(BL_TARGET_OPT_AVX512)
BL_INLINE_NODEBUG __m512i simd_packs_128_i16_i8(const __m512i& a) noexcept { return _mm512_packs_epi16(a, a); }
BL_INLINE_NODEBUG __m512i simd_packs_128_i16_u8(const __m512i& a) noexcept { return _mm512_packus_epi16(a, a); }
BL_INLINE_NODEBUG __m512i simd_packs_128_i32_i16(const __m512i& a) noexcept { return _mm512_packs_epi32(a, a); }
BL_INLINE_NODEBUG __m512i simd_packs_128_i32_u16(const __m512i& a) noexcept { return _mm512_packus_epi32(a, a); }

BL_INLINE_NODEBUG __m512i simd_packs_128_i16_i8(const __m512i& a, const __m512i& b) noexcept { return _mm512_packs_epi16(a, b); }
BL_INLINE_NODEBUG __m512i simd_packs_128_i16_u8(const __m512i& a, const __m512i& b) noexcept { return _mm512_packus_epi16(a, b); }
BL_INLINE_NODEBUG __m512i simd_packs_128_i32_i16(const __m512i& a, const __m512i& b) noexcept { return _mm512_packs_epi32(a, b); }
BL_INLINE_NODEBUG __m512i simd_packs_128_i32_u16(const __m512i& a, const __m512i& b) noexcept { return _mm512_packus_epi32(a, b); }

BL_INLINE_NODEBUG __m512i simd_packs_128_i32_i8(const __m512i& a) noexcept { return simd_packs_128_i16_i8(_mm512_packs_epi32(a, a)); }
BL_INLINE_NODEBUG __m512i simd_packs_128_i32_u8(const __m512i& a) noexcept { return simd_packs_128_i16_u8(_mm512_packs_epi32(a, a)); }

BL_INLINE_NODEBUG __m512i simd_packs_128_i32_i8(const __m512i& a, const __m512i& b) noexcept { return simd_packs_128_i16_i8(_mm512_packs_epi32(a, b)); }
BL_INLINE_NODEBUG __m512i simd_packs_128_i32_u8(const __m512i& a, const __m512i& b) noexcept { return simd_packs_128_i16_u8(_mm512_packs_epi32(a, b)); }

BL_INLINE_NODEBUG __m512i simd_packs_128_i32_i8(const __m512i& a, const __m512i& b, const __m512i& c, const __m512i& d) noexcept { return simd_packs_128_i16_i8(_mm512_packs_epi32(a, b), _mm512_packs_epi32(c, d)); }
BL_INLINE_NODEBUG __m512i simd_packs_128_i32_u8(const __m512i& a, const __m512i& b, const __m512i& c, const __m512i& d) noexcept { return simd_packs_128_i16_u8(_mm512_packs_epi32(a, b), _mm512_packs_epi32(c, d)); }

BL_INLINE_NODEBUG __m512i simd_packz_128_u32_u16(const __m512i& a) noexcept { return simd_packs_128_i32_u16(a); }
BL_INLINE_NODEBUG __m512i simd_packz_128_u32_u16(const __m512i& a, const __m512i& b) noexcept { return _mm512_packus_epi32(a, b); }

BL_INLINE_NODEBUG __m512i simd_packz_128_u32_u8(const __m512i& a) noexcept { return simd_packs_128_i16_u8(_mm512_packs_epi32(a, a)); }
BL_INLINE_NODEBUG __m512i simd_packz_128_u32_u8(const __m512i& a, const __m512i& b) noexcept { return simd_packs_128_i16_u8(_mm512_packs_epi32(a, b)); }
BL_INLINE_NODEBUG __m512i simd_packz_128_u32_u8(const __m512i& a, const __m512i& b, const __m512i& c, const __m512i& d) noexcept { return simd_packs_128_i16_u8(_mm512_packs_epi32(a, b), _mm512_packs_epi32(c, d)); }

BL_INLINE_NODEBUG __m512i simd_unpack_lo64_i8_i16(const __m512i& a) noexcept { return _mm512_srai_epi16(_mm512_unpacklo_epi8(a, a), 8); }
BL_INLINE_NODEBUG __m512i simd_unpack_lo64_u8_u16(const __m512i& a) noexcept { return _mm512_unpacklo_epi8(a, _mm512_setzero_si512()); }
BL_INLINE_NODEBUG __m512i simd_unpack_lo64_i16_i32(const __m512i& a) noexcept { return _mm512_srai_epi32(_mm512_unpacklo_epi16(a, a), 16); }
BL_INLINE_NODEBUG __m512i simd_unpack_lo64_u16_u32(const __m512i& a) noexcept { return _mm512_unpacklo_epi16(a, _mm512_setzero_si512()); }
BL_INLINE_NODEBUG __m512i simd_unpack_lo64_i32_i64(const __m512i& a) noexcept { return _mm512_unpacklo_epi32(a, _mm512_srai_epi32(a, 31)); }
BL_INLINE_NODEBUG __m512i simd_unpack_lo64_u32_u64(const __m512i& a) noexcept { return _mm512_unpacklo_epi32(a, _mm512_setzero_si512()); }

BL_INLINE_NODEBUG __m512i simd_unpack_hi64_i8_i16(const __m512i& a) noexcept { return _mm512_srai_epi16(_mm512_unpackhi_epi8(a, a), 8); }
BL_INLINE_NODEBUG __m512i simd_unpack_hi64_u8_u16(const __m512i& a) noexcept { return _mm512_unpackhi_epi8(a, _mm512_setzero_si512()); }
BL_INLINE_NODEBUG __m512i simd_unpack_hi64_i16_i32(const __m512i& a) noexcept { return _mm512_srai_epi32(_mm512_unpackhi_epi16(a, a), 16); }
BL_INLINE_NODEBUG __m512i simd_unpack_hi64_u16_u32(const __m512i& a) noexcept { return _mm512_unpackhi_epi16(a, _mm512_setzero_si512()); }
BL_INLINE_NODEBUG __m512i simd_unpack_hi64_i32_i64(const __m512i& a) noexcept { return _mm512_unpackhi_epi32(a, _mm512_srai_epi32(a, 31)); }
BL_INLINE_NODEBUG __m512i simd_unpack_hi64_u32_u64(const __m512i& a) noexcept { return _mm512_unpackhi_epi32(a, _mm512_setzero_si512()); }

BL_INLINE_NODEBUG __m512i simd_movw_i8_i16(const __m512i& a) noexcept { return _mm512_cvtepi8_epi16(simd_cast<__m256i>(a)); }
BL_INLINE_NODEBUG __m512i simd_movw_u8_u16(const __m512i& a) noexcept { return _mm512_cvtepu8_epi16(simd_cast<__m256i>(a)); }
BL_INLINE_NODEBUG __m512i simd_movw_i8_i32(const __m512i& a) noexcept { return _mm512_cvtepi8_epi32(simd_cast<__m128i>(a)); }
BL_INLINE_NODEBUG __m512i simd_movw_u8_u32(const __m512i& a) noexcept { return _mm512_cvtepu8_epi32(simd_cast<__m128i>(a)); }
BL_INLINE_NODEBUG __m512i simd_movw_i8_i64(const __m512i& a) noexcept { return _mm512_cvtepi8_epi64(simd_cast<__m128i>(a)); }
BL_INLINE_NODEBUG __m512i simd_movw_u8_u64(const __m512i& a) noexcept { return _mm512_cvtepu8_epi64(simd_cast<__m128i>(a)); }
BL_INLINE_NODEBUG __m512i simd_movw_i16_i32(const __m512i& a) noexcept { return _mm512_cvtepi16_epi32(simd_cast<__m256i>(a)); }
BL_INLINE_NODEBUG __m512i simd_movw_u16_u32(const __m512i& a) noexcept { return _mm512_cvtepu16_epi32(simd_cast<__m256i>(a)); }
BL_INLINE_NODEBUG __m512i simd_movw_i16_i64(const __m512i& a) noexcept { return _mm512_cvtepi16_epi64(simd_cast<__m128i>(a)); }
BL_INLINE_NODEBUG __m512i simd_movw_u16_u64(const __m512i& a) noexcept { return _mm512_cvtepu16_epi64(simd_cast<__m128i>(a)); }
BL_INLINE_NODEBUG __m512i simd_movw_i32_i64(const __m512i& a) noexcept { return _mm512_cvtepi32_epi64(simd_cast<__m256i>(a)); }
BL_INLINE_NODEBUG __m512i simd_movw_u32_u64(const __m512i& a) noexcept { return _mm512_cvtepu32_epi64(simd_cast<__m256i>(a)); }
#endif // BL_TARGET_OPT_AVX512

} // {Internal}

// SIMD - Extract & Insert
// =======================

template<uint32_t kIndex, typename V>
BL_INLINE_NODEBUG uint32_t extract_u16(const V& src) noexcept {
  return uint32_t(_mm_extract_epi16(to_simd<__m128i>(src), kIndex));
}

template<uint32_t kIndex, typename V>
BL_INLINE_NODEBUG Vec<16, typename V::ElementType> insert_u16(const V& dst, uint16_t val) noexcept {
  typedef Vec<16, typename V::ElementType> Vec128;
  return from_simd<Vec128>(_mm_insert_epi16(to_simd<__m128i>(dst), int(unsigned(val)), kIndex));
}

template<uint32_t kIndex, typename V>
BL_INLINE_NODEBUG Vec<16, typename V::ElementType> insert_i16(const V& dst, int16_t val) noexcept {
  return insert_u16<kIndex>(dst, uint16_t(val));
}

template<uint32_t kIndex, typename V>
BL_INLINE_NODEBUG Vec<16, typename V::ElementType> insert_m16(const V& dst, const void* src) noexcept {
  return insert_u16<kIndex>(dst, bl::MemOps::readU16u(src));
}

#if defined(BL_TARGET_OPT_SSE4_1)

template<uint32_t kIndex, typename V>
BL_INLINE_NODEBUG uint32_t extract_u8(const V& src) noexcept { return uint32_t(_mm_extract_epi8(to_simd<__m128i>(src), kIndex)); }
template<uint32_t kIndex, typename V>
BL_INLINE_NODEBUG uint32_t extract_u32(const V& src) noexcept { return uint32_t(_mm_extract_epi32(to_simd<__m128i>(src), kIndex)); }

template<uint32_t kIndex, typename V>
BL_INLINE_NODEBUG Vec<16, typename V::ElementType> insert_u8(const V& dst, uint8_t val) noexcept {
  typedef Vec<16, typename V::ElementType> Vec128;
  return Vec128{simd_cast<typename Vec128::SimdType>(_mm_insert_epi8(to_simd<__m128i>(dst), int(unsigned(val)), kIndex))};
}

template<uint32_t kIndex, typename V>
BL_INLINE_NODEBUG Vec<16, typename V::ElementType> insert_i8(const V& dst, int8_t val) noexcept {
  return insert_u8<kIndex>(dst, uint8_t(val));
}

template<uint32_t kIndex, typename V>
BL_INLINE_NODEBUG Vec<16, typename V::ElementType> insert_m8(const V& dst, const void* src) noexcept {
  return insert_u8<kIndex>(dst, *static_cast<const uint8_t*>(src));
}

template<uint32_t kIndex, typename V>
BL_INLINE_NODEBUG Vec<16, typename V::ElementType> insert_u32(const V& dst, uint32_t val) noexcept {
  typedef Vec<16, typename V::ElementType> Vec128;
  return Vec128{simd_cast<typename Vec128::SimdType>(_mm_insert_epi32(to_simd<__m128i>(dst), int(val), kIndex))};
}

template<uint32_t kIndex, typename V>
BL_INLINE_NODEBUG Vec<16, typename V::ElementType> insert_i32(const V& dst, int32_t val) noexcept {
  return insert_u32<kIndex>(dst, uint32_t(val));
}

template<uint32_t kIndex, typename V>
BL_INLINE_NODEBUG Vec<16, typename V::ElementType> insert_m32(const V& dst, const void* src) noexcept {
  return insert_u32<kIndex>(dst, bl::MemOps::readU32u(src));
}

// Convenience function used to insert RGB24 components.
template<uint32_t kIndex, typename V>
BL_INLINE_NODEBUG Vec<16, typename V::ElementType> insert_m24(const V& dst, const void* src) noexcept {
  typedef Vec<16, typename V::ElementType> Vec128;
  const uint8_t* src_u8 = static_cast<const uint8_t*>(src);

  __m128i v = to_simd<__m128i>(dst);

  if constexpr ((kIndex & 0x1) == 0) {
    uint32_t u16_val = bl::MemOps::readU16u(src_u8);
    v = _mm_insert_epi16(v, int16_t(u16_val), kIndex / 2u);

    uint32_t u8_val = bl::MemOps::readU8(src_u8 + 2u);
    v = _mm_insert_epi8(v, int8_t(u8_val), kIndex + 2u);
  }
  else {
    uint32_t u8_val = bl::MemOps::readU8(src_u8);
    v = _mm_insert_epi8(v, int8_t(u8_val), kIndex);

    uint32_t u16_val = bl::MemOps::readU16u(src_u8 + 1u);
    v = _mm_insert_epi16(v, int16_t(u16_val), (kIndex + 1u) / 2u);
  }

  return from_simd<Vec128>(v);
}

#else

// Emulation of PEXTRB.
template<uint32_t kIndex, typename V>
BL_INLINE_NODEBUG uint32_t extract_u8(const V& src) noexcept {
  if constexpr ((kIndex & 1) == 0)
    return uint32_t(_mm_extract_epi16(to_simd<__m128i>(src), kIndex / 2u)) & 0xFFu;
  else
    return uint32_t(_mm_extract_epi16(to_simd<__m128i>(src), kIndex / 2u)) >> 8;
}

// Emulation of PEXTRD.
template<uint32_t kIndex, typename V>
BL_INLINE_NODEBUG uint32_t extract_u32(const V& src) noexcept {
#if BL_TARGET_ARCH_BITS >= 64
  if constexpr (kIndex == 1) {
    return uint32_t(uint64_t(_mm_cvtsi128_si64x(to_simd<__m128i>(src))) >> 32);
  }
  else
#endif
  if constexpr (kIndex == 0) {
    return uint32_t(_mm_cvtsi128_si32(to_simd<__m128i>(src)));
  }
  else {
    uint32_t lo = uint32_t(_mm_extract_epi16(to_simd<__m128i>(src), kIndex * 2u));
    uint32_t hi = uint32_t(_mm_extract_epi16(to_simd<__m128i>(src), kIndex * 2u + 1u));
    return (hi << 16) | lo;
  }
}

// Emulation of PINSRD.
template<uint32_t kIndex, typename V>
BL_INLINE_NODEBUG Vec<16, typename V::ElementType> insert_u32(const V& dst, uint32_t val) noexcept {
  int lo = int(val & 0xFFFFu);
  int hi = int(val >> 16);

  typedef Vec<16, typename V::ElementType> Vec128;
  return Vec128{
    simd_cast<typename Vec128::SimdType>(
      _mm_insert_epi16(
        _mm_insert_epi16(to_simd<__m128i>(dst), lo, kIndex), hi, kIndex + 1))};
}

template<uint32_t kIndex, typename V>
BL_INLINE_NODEBUG Vec<16, typename V::ElementType> insert_i32(const V& dst, int32_t val) noexcept {
  return insert_u32<kIndex>(dst, uint32_t(val));
}

template<uint32_t kIndex, typename V>
BL_INLINE_NODEBUG Vec<16, typename V::ElementType> insert_m32(const V& dst, const void* src) noexcept {
  return insert_m16<kIndex + 1>(insert_m16<kIndex>(dst, src), static_cast<const uint8_t*>(src) + 2);
}

#endif // BL_TARGET_OPT_SSE4_1

// SIMD - Internal - Arithmetic and Logical Operations
// ===================================================

namespace Internal {

#if defined(BL_TARGET_OPT_AVX512)
template<uint8_t kPred>
BL_INLINE_NODEBUG __m128i simd_ternlog(const __m128i& a, const __m128i& b, const __m128i& c) noexcept { return _mm_ternarylogic_epi32(a, b, c, kPred); }

template<uint8_t kPred>
BL_INLINE_NODEBUG __m128 simd_ternlog(const __m128& a, const __m128& b, const __m128& c) noexcept { return simd_as_f(simd_ternlog<kPred>(simd_as_i(a), simd_as_i(b), simd_as_i(c))); }

template<uint8_t kPred>
BL_INLINE_NODEBUG __m128d simd_ternlog(const __m128d& a, const __m128d& b, const __m128d& c) noexcept { return simd_as_d(simd_ternlog<kPred>(simd_as_i(a), simd_as_i(b), simd_as_i(c))); }
#endif // BL_TARGET_OPT_AVX512

BL_INLINE_NODEBUG __m128 simd_and(const __m128& a, const __m128& b) noexcept { return _mm_and_ps(a, b); }
BL_INLINE_NODEBUG __m128d simd_and(const __m128d& a, const __m128d& b) noexcept { return _mm_and_pd(a, b); }
BL_INLINE_NODEBUG __m128i simd_and(const __m128i& a, const __m128i& b) noexcept { return _mm_and_si128(a, b); }

BL_INLINE_NODEBUG __m128 simd_andnot(const __m128& a, const __m128& b) noexcept { return _mm_andnot_ps(a, b); }
BL_INLINE_NODEBUG __m128d simd_andnot(const __m128d& a, const __m128d& b) noexcept { return _mm_andnot_pd(a, b); }
BL_INLINE_NODEBUG __m128i simd_andnot(const __m128i& a, const __m128i& b) noexcept { return _mm_andnot_si128(a, b); }

BL_INLINE_NODEBUG __m128 simd_or(const __m128& a, const __m128& b) noexcept { return _mm_or_ps(a, b); }
BL_INLINE_NODEBUG __m128d simd_or(const __m128d& a, const __m128d& b) noexcept { return _mm_or_pd(a, b); }
BL_INLINE_NODEBUG __m128i simd_or(const __m128i& a, const __m128i& b) noexcept { return _mm_or_si128(a, b); }

BL_INLINE_NODEBUG __m128 simd_xor(const __m128& a, const __m128& b) noexcept { return _mm_xor_ps(a, b); }
BL_INLINE_NODEBUG __m128d simd_xor(const __m128d& a, const __m128d& b) noexcept { return _mm_xor_pd(a, b); }
BL_INLINE_NODEBUG __m128i simd_xor(const __m128i& a, const __m128i& b) noexcept { return _mm_xor_si128(a, b); }

#if defined(BL_TARGET_OPT_AVX512)
BL_INLINE_NODEBUG __m128 simd_not(const __m128& a) noexcept { return simd_ternlog<0x55u>(a, a, a); }
BL_INLINE_NODEBUG __m128d simd_not(const __m128d& a) noexcept { return simd_ternlog<0x55u>(a, a, a); }
BL_INLINE_NODEBUG __m128i simd_not(const __m128i& a) noexcept { return simd_ternlog<0x55u>(a, a, a); }

BL_INLINE_NODEBUG __m128 simd_blendv_bits(const __m128& a, const __m128& b, const __m128& msk) noexcept { return simd_ternlog<0xD8>(a, b, msk); }
BL_INLINE_NODEBUG __m128d simd_blendv_bits(const __m128d& a, const __m128d& b, const __m128d& msk) noexcept { return simd_ternlog<0xD8>(a, b, msk); }
BL_INLINE_NODEBUG __m128i simd_blendv_bits(const __m128i& a, const __m128i& b, const __m128i& msk) noexcept { return simd_ternlog<0xD8>(a, b, msk); }
#else
BL_INLINE_NODEBUG __m128 simd_not(const __m128& a) noexcept { return simd_xor(a, simd_make_ones<__m128>()); }
BL_INLINE_NODEBUG __m128d simd_not(const __m128d& a) noexcept { return simd_xor(a, simd_make_ones<__m128d>()); }
BL_INLINE_NODEBUG __m128i simd_not(const __m128i& a) noexcept { return simd_xor(a, simd_make_ones<__m128i>()); }

BL_INLINE_NODEBUG __m128 simd_blendv_bits(const __m128& a, const __m128& b, const __m128& msk) noexcept { return simd_or(simd_and(b, msk), simd_andnot(msk, a)); }
BL_INLINE_NODEBUG __m128d simd_blendv_bits(const __m128d& a, const __m128d& b, const __m128d& msk) noexcept { return simd_or(simd_and(b, msk), simd_andnot(msk, a)); }
BL_INLINE_NODEBUG __m128i simd_blendv_bits(const __m128i& a, const __m128i& b, const __m128i& msk) noexcept { return simd_or(simd_and(b, msk), simd_andnot(msk, a)); }
#endif

#if defined(BL_TARGET_OPT_SSE4_1)
BL_INLINE_NODEBUG __m128i simd_blendv_u8(const __m128i& a, const __m128i& b, const __m128i& msk) noexcept { return _mm_blendv_epi8(a, b, msk); }
#else
BL_INLINE_NODEBUG __m128i simd_blendv_u8(const __m128i& a, const __m128i& b, const __m128i& msk) noexcept { return simd_blendv_bits(a, b, msk); }
#endif

#if defined(BL_TARGET_OPT_SSE4_1)
template<uint32_t H, uint32_t G, uint32_t F, uint32_t E, uint32_t D, uint32_t C, uint32_t B, uint32_t A>
BL_INLINE_NODEBUG __m128i simd_blend_i16(const __m128i& a, const __m128i& b) noexcept {
  return _mm_blend_epi16(a, b, (H << 7) | (G << 6) | (F << 5) | (E << 4) | (D << 3) | (C << 2) | (B << 1) | A);
}

template<uint32_t D, uint32_t C, uint32_t B, uint32_t A>
BL_INLINE_NODEBUG __m128i simd_blend_i32(const __m128i& a, const __m128i& b) noexcept {
  return _mm_blend_epi16(a, b, ((D * 0x3) << 3) | ((C * 0x3) << 2) | ((B * 0x3) << 1) | (A * 0x3));
}

template<uint32_t B, uint32_t A>
BL_INLINE_NODEBUG __m128i simd_blend_i64(const __m128i& a, const __m128i& b) noexcept {
  return _mm_blend_epi16(a, b, ((B * 0xF) << 1) | (A * 0xF));
}
#endif

BL_INLINE_NODEBUG __m128i simd_flip_sign_i8(const __m128i& a) noexcept { return simd_xor(a, simd_make128_u32(0x80808080u)); }
BL_INLINE_NODEBUG __m128i simd_flip_sign_i16(const __m128i& a) noexcept { return simd_xor(a, simd_make128_u32(0x80008000u)); }
BL_INLINE_NODEBUG __m128i simd_flip_sign_i32(const __m128i& a) noexcept { return simd_xor(a, simd_make128_u32(0x80000000u)); }
BL_INLINE_NODEBUG __m128i simd_flip_sign_i64(const __m128i& a) noexcept { return simd_xor(a, simd_make128_u64(uint64_t(1) << 63)); }

BL_INLINE_NODEBUG __m128 simd_add_f32(const __m128& a, const __m128& b) noexcept { return _mm_add_ps(a, b); }
BL_INLINE_NODEBUG __m128d simd_add_f64(const __m128d& a, const __m128d& b) noexcept { return _mm_add_pd(a, b); }

BL_INLINE_NODEBUG __m128 simd_sub_f32(const __m128& a, const __m128& b) noexcept { return _mm_sub_ps(a, b); }
BL_INLINE_NODEBUG __m128d simd_sub_f64(const __m128d& a, const __m128d& b) noexcept { return _mm_sub_pd(a, b); }

BL_INLINE_NODEBUG __m128 simd_mul_f32(const __m128& a, const __m128& b) noexcept { return _mm_mul_ps(a, b); }
BL_INLINE_NODEBUG __m128d simd_mul_f64(const __m128d& a, const __m128d& b) noexcept { return _mm_mul_pd(a, b); }

BL_INLINE_NODEBUG __m128 simd_div_f32(const __m128& a, const __m128& b) noexcept { return _mm_div_ps(a, b); }
BL_INLINE_NODEBUG __m128d simd_div_f64(const __m128d& a, const __m128d& b) noexcept { return _mm_div_pd(a, b); }

BL_INLINE_NODEBUG __m128 simd_cmp_eq_f32(const __m128& a, const __m128& b) noexcept { return _mm_cmpeq_ps(a, b); }
BL_INLINE_NODEBUG __m128d simd_cmp_eq_f64(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpeq_pd(a, b); }

BL_INLINE_NODEBUG __m128 simd_cmp_ne_f32(const __m128& a, const __m128& b) noexcept { return _mm_cmpneq_ps(a, b); }
BL_INLINE_NODEBUG __m128d simd_cmp_ne_f64(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpneq_pd(a, b); }

BL_INLINE_NODEBUG __m128 simd_cmp_lt_f32(const __m128& a, const __m128& b) noexcept { return _mm_cmplt_ps(a, b); }
BL_INLINE_NODEBUG __m128d simd_cmp_lt_f64(const __m128d& a, const __m128d& b) noexcept { return _mm_cmplt_pd(a, b); }

BL_INLINE_NODEBUG __m128 simd_cmp_le_f32(const __m128& a, const __m128& b) noexcept { return _mm_cmple_ps(a, b); }
BL_INLINE_NODEBUG __m128d simd_cmp_le_f64(const __m128d& a, const __m128d& b) noexcept { return _mm_cmple_pd(a, b); }

BL_INLINE_NODEBUG __m128 simd_cmp_gt_f32(const __m128& a, const __m128& b) noexcept { return _mm_cmpgt_ps(a, b); }
BL_INLINE_NODEBUG __m128d simd_cmp_gt_f64(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpgt_pd(a, b); }

BL_INLINE_NODEBUG __m128 simd_cmp_ge_f32(const __m128& a, const __m128& b) noexcept { return _mm_cmpge_ps(a, b); }
BL_INLINE_NODEBUG __m128d simd_cmp_ge_f64(const __m128d& a, const __m128d& b) noexcept { return _mm_cmpge_pd(a, b); }

BL_INLINE_NODEBUG __m128 simd_min_f32(const __m128& a, const __m128& b) noexcept { return _mm_min_ps(a, b); }
BL_INLINE_NODEBUG __m128d simd_min_f64(const __m128d& a, const __m128d& b) noexcept { return _mm_min_pd(a, b); }

BL_INLINE_NODEBUG __m128 simd_max_f32(const __m128& a, const __m128& b) noexcept { return _mm_max_ps(a, b); }
BL_INLINE_NODEBUG __m128d simd_max_f64(const __m128d& a, const __m128d& b) noexcept { return _mm_max_pd(a, b); }

BL_INLINE_NODEBUG __m128 simd_abs_f32(const __m128& a) noexcept { return _mm_and_ps(a, bl::common_table.p_7FFFFFFF7FFFFFFF.as<__m128>()); }
BL_INLINE_NODEBUG __m128d simd_abs_f64(const __m128d& a) noexcept { return _mm_and_pd(a, bl::common_table.p_7FFFFFFFFFFFFFFF.as<__m128d>()); }

BL_INLINE_NODEBUG __m128 simd_sqrt_f32(const __m128& a) noexcept { return _mm_sqrt_ps(a); }
BL_INLINE_NODEBUG __m128d simd_sqrt_f64(const __m128d& a) noexcept { return _mm_sqrt_pd(a); }

BL_INLINE_NODEBUG __m128i simd_add_i8(const __m128i& a, const __m128i& b) noexcept { return _mm_add_epi8(a, b); }
BL_INLINE_NODEBUG __m128i simd_add_i16(const __m128i& a, const __m128i& b) noexcept { return _mm_add_epi16(a, b); }
BL_INLINE_NODEBUG __m128i simd_add_i32(const __m128i& a, const __m128i& b) noexcept { return _mm_add_epi32(a, b); }
BL_INLINE_NODEBUG __m128i simd_add_i64(const __m128i& a, const __m128i& b) noexcept { return _mm_add_epi64(a, b); }
BL_INLINE_NODEBUG __m128i simd_add_u8(const __m128i& a, const __m128i& b) noexcept { return _mm_add_epi8(a, b); }
BL_INLINE_NODEBUG __m128i simd_add_u16(const __m128i& a, const __m128i& b) noexcept { return _mm_add_epi16(a, b); }
BL_INLINE_NODEBUG __m128i simd_add_u32(const __m128i& a, const __m128i& b) noexcept { return _mm_add_epi32(a, b); }
BL_INLINE_NODEBUG __m128i simd_add_u64(const __m128i& a, const __m128i& b) noexcept { return _mm_add_epi64(a, b); }

BL_INLINE_NODEBUG __m128i simd_adds_i8(const __m128i& a, const __m128i& b) noexcept { return _mm_adds_epi8(a, b); }
BL_INLINE_NODEBUG __m128i simd_adds_i16(const __m128i& a, const __m128i& b) noexcept { return _mm_adds_epi16(a, b); }
BL_INLINE_NODEBUG __m128i simd_adds_u8(const __m128i& a, const __m128i& b) noexcept { return _mm_adds_epu8(a, b); }
BL_INLINE_NODEBUG __m128i simd_adds_u16(const __m128i& a, const __m128i& b) noexcept { return _mm_adds_epu16(a, b); }

BL_INLINE_NODEBUG __m128i simd_sub_i8(const __m128i& a, const __m128i& b) noexcept { return _mm_sub_epi8(a, b); }
BL_INLINE_NODEBUG __m128i simd_sub_i16(const __m128i& a, const __m128i& b) noexcept { return _mm_sub_epi16(a, b); }
BL_INLINE_NODEBUG __m128i simd_sub_i32(const __m128i& a, const __m128i& b) noexcept { return _mm_sub_epi32(a, b); }
BL_INLINE_NODEBUG __m128i simd_sub_i64(const __m128i& a, const __m128i& b) noexcept { return _mm_sub_epi64(a, b); }
BL_INLINE_NODEBUG __m128i simd_sub_u8(const __m128i& a, const __m128i& b) noexcept { return _mm_sub_epi8(a, b); }
BL_INLINE_NODEBUG __m128i simd_sub_u16(const __m128i& a, const __m128i& b) noexcept { return _mm_sub_epi16(a, b); }
BL_INLINE_NODEBUG __m128i simd_sub_u32(const __m128i& a, const __m128i& b) noexcept { return _mm_sub_epi32(a, b); }
BL_INLINE_NODEBUG __m128i simd_sub_u64(const __m128i& a, const __m128i& b) noexcept { return _mm_sub_epi64(a, b); }

BL_INLINE_NODEBUG __m128i simd_subs_i8(const __m128i& a, const __m128i& b) noexcept { return _mm_subs_epi8(a, b); }
BL_INLINE_NODEBUG __m128i simd_subs_i16(const __m128i& a, const __m128i& b) noexcept { return _mm_subs_epi16(a, b); }
BL_INLINE_NODEBUG __m128i simd_subs_u8(const __m128i& a, const __m128i& b) noexcept { return _mm_subs_epu8(a, b); }
BL_INLINE_NODEBUG __m128i simd_subs_u16(const __m128i& a, const __m128i& b) noexcept { return _mm_subs_epu16(a, b); }

BL_INLINE_NODEBUG __m128i simd_mul_i16(const __m128i& a, const __m128i& b) noexcept { return _mm_mullo_epi16(a, b); }
BL_INLINE_NODEBUG __m128i simd_mul_u16(const __m128i& a, const __m128i& b) noexcept { return _mm_mullo_epi16(a, b); }

BL_INLINE_NODEBUG __m128i simd_mulh_i16(const __m128i& a, const __m128i& b) noexcept { return _mm_mulhi_epi16(a, b); }
BL_INLINE_NODEBUG __m128i simd_mulh_u16(const __m128i& a, const __m128i& b) noexcept { return _mm_mulhi_epu16(a, b); }
BL_INLINE_NODEBUG __m128i simd_mulw_u32(const __m128i& a, const __m128i& b) noexcept { return _mm_mul_epu32(a, b); }

#if defined(BL_TARGET_OPT_SSE4_1)
BL_INLINE_NODEBUG __m128i simd_mulw_i32(const __m128i& a, const __m128i& b) noexcept { return _mm_mul_epi32(a, b); }
BL_INLINE_NODEBUG __m128i simd_mul_i32(const __m128i& a, const __m128i& b) noexcept { return _mm_mullo_epi32(a, b); }
BL_INLINE_NODEBUG __m128i simd_mul_u32(const __m128i& a, const __m128i& b) noexcept { return _mm_mullo_epi32(a, b); }
#else
BL_INLINE_NODEBUG __m128i simd_mul_i32(const __m128i& a, const __m128i& b) noexcept {
  __m128i hi = _mm_mul_epu32(_mm_shuffle_epi32(a, _MM_SHUFFLE(3, 3, 1, 1)),
                             _mm_shuffle_epi32(b, _MM_SHUFFLE(3, 3, 1, 1)));
  __m128i lo = _mm_mul_epu32(a, b);
  __m128i result3120 = simd_cast<__m128i>(_mm_shuffle_ps(simd_cast<__m128>(lo), simd_cast<__m128>(hi), _MM_SHUFFLE(2, 0, 2, 0)));
  return _mm_shuffle_epi32(result3120, _MM_SHUFFLE(3, 1, 2, 0));
}
BL_INLINE_NODEBUG __m128i simd_mul_u32(const __m128i& a, const __m128i& b) noexcept { return simd_mul_i32(a, b); }
#endif // BL_TARGET_OPT_SSE4_1

#if defined(BL_TARGET_OPT_AVX512)
BL_INLINE_NODEBUG __m128i simd_mul_i64(const __m128i& a, const __m128i& b) noexcept { return _mm_mullo_epi64(a, b); }
#else
BL_INLINE_NODEBUG __m128i simd_mul_i64(const __m128i& a, const __m128i& b) noexcept {
  union u64x2_view { __m128i reg; uint64_t elements[2]; };

  u64x2_view a_view{a};
  u64x2_view b_view{b};
  return simd_make128_u64(a_view.elements[1] * b_view.elements[1], a_view.elements[0] * b_view.elements[0]);
}
#endif
BL_INLINE_NODEBUG __m128i simd_mul_u64(const __m128i& a, const __m128i& b) noexcept { return simd_mul_i64(a, b); }

BL_INLINE_NODEBUG __m128i simd_maddw_i16_i32(const __m128i& a, const __m128i& b) noexcept { return _mm_madd_epi16(a, b); }

BL_INLINE_NODEBUG __m128i simd_cmp_eq_i8(const __m128i& a, const __m128i& b) noexcept { return _mm_cmpeq_epi8(a, b); }
BL_INLINE_NODEBUG __m128i simd_cmp_eq_i16(const __m128i& a, const __m128i& b) noexcept { return _mm_cmpeq_epi16(a, b); }
BL_INLINE_NODEBUG __m128i simd_cmp_eq_i32(const __m128i& a, const __m128i& b) noexcept { return _mm_cmpeq_epi32(a, b); }

#if defined(BL_TARGET_OPT_SSE4_1)
BL_INLINE_NODEBUG __m128i simd_cmp_eq_i64(const __m128i& a, const __m128i& b) noexcept { return _mm_cmpeq_epi64(a, b); }
#else
BL_INLINE_NODEBUG __m128i simd_cmp_eq_i64(const __m128i& a, const __m128i& b) noexcept {
  __m128i x = _mm_cmpeq_epi32(a, b);
  __m128i y = _mm_shuffle_epi32(x, _MM_SHUFFLE(2, 3, 0, 1));
  return _mm_and_si128(x, y);
}
#endif

BL_INLINE_NODEBUG __m128i simd_cmp_ne_i8(const __m128i& a, const __m128i& b) noexcept { return simd_not(simd_cmp_eq_i8(a, b)); }
BL_INLINE_NODEBUG __m128i simd_cmp_ne_i16(const __m128i& a, const __m128i& b) noexcept { return simd_not(simd_cmp_eq_i16(a, b)); }
BL_INLINE_NODEBUG __m128i simd_cmp_ne_i32(const __m128i& a, const __m128i& b) noexcept { return simd_not(simd_cmp_eq_i32(a, b)); }
BL_INLINE_NODEBUG __m128i simd_cmp_ne_i64(const __m128i& a, const __m128i& b) noexcept { return simd_not(simd_cmp_eq_i64(a, b)); }

BL_INLINE_NODEBUG __m128i simd_cmp_gt_i8(const __m128i& a, const __m128i& b) noexcept { return _mm_cmpgt_epi8(a, b); }
BL_INLINE_NODEBUG __m128i simd_cmp_gt_i16(const __m128i& a, const __m128i& b) noexcept { return _mm_cmpgt_epi16(a, b); }
BL_INLINE_NODEBUG __m128i simd_cmp_gt_i32(const __m128i& a, const __m128i& b) noexcept { return _mm_cmpgt_epi32(a, b); }

#if defined(BL_TARGET_OPT_SSE4_2)
BL_INLINE_NODEBUG __m128i simd_cmp_gt_i64(const __m128i& a, const __m128i& b) noexcept { return _mm_cmpgt_epi64(a, b); }
#else
BL_INLINE_NODEBUG __m128i simd_cmp_gt_i64(const __m128i& a, const __m128i& b) noexcept {
  // Possibly the best solution:
  //   https://stackoverflow.com/questions/65166174/how-to-simulate-pcmpgtq-on-sse2
  //
  // The good thing on this solution is that it doesn't need any constants, just temporaries.
  __m128i msk = _mm_and_si128(_mm_cmpeq_epi32(a, b), _mm_sub_epi64(b, a));
  msk = _mm_or_si128(msk, _mm_cmpgt_epi32(a, b));
  return _mm_shuffle_epi32(msk, _MM_SHUFFLE(3, 3, 1, 1));
}
#endif

BL_INLINE_NODEBUG __m128i simd_cmp_lt_i8(const __m128i& a, const __m128i& b) noexcept { return simd_cmp_gt_i8(b, a); }
BL_INLINE_NODEBUG __m128i simd_cmp_lt_i16(const __m128i& a, const __m128i& b) noexcept { return simd_cmp_gt_i16(b, a); }
BL_INLINE_NODEBUG __m128i simd_cmp_lt_i32(const __m128i& a, const __m128i& b) noexcept { return simd_cmp_gt_i32(b, a); }
BL_INLINE_NODEBUG __m128i simd_cmp_lt_i64(const __m128i& a, const __m128i& b) noexcept { return simd_cmp_gt_i64(b, a); }

#if defined(BL_TARGET_OPT_SSE4_1)
BL_INLINE_NODEBUG __m128i simd_cmp_ge_i8(const __m128i& a, const __m128i& b) noexcept { return simd_cmp_eq_i8(_mm_min_epi8(a, b), b); }
BL_INLINE_NODEBUG __m128i simd_cmp_le_i8(const __m128i& a, const __m128i& b) noexcept { return simd_cmp_eq_i8(_mm_max_epi8(a, b), b); }
BL_INLINE_NODEBUG __m128i simd_cmp_ge_i16(const __m128i& a, const __m128i& b) noexcept { return simd_cmp_eq_i16(_mm_min_epi16(a, b), b); }
BL_INLINE_NODEBUG __m128i simd_cmp_le_i16(const __m128i& a, const __m128i& b) noexcept { return simd_cmp_eq_i16(_mm_max_epi16(a, b), b); }
BL_INLINE_NODEBUG __m128i simd_cmp_ge_i32(const __m128i& a, const __m128i& b) noexcept { return simd_cmp_eq_i32(_mm_min_epi32(a, b), b); }
BL_INLINE_NODEBUG __m128i simd_cmp_le_i32(const __m128i& a, const __m128i& b) noexcept { return simd_cmp_eq_i32(_mm_max_epi32(a, b), b); }
#else
BL_INLINE_NODEBUG __m128i simd_cmp_ge_i8(const __m128i& a, const __m128i& b) noexcept { return simd_not(simd_cmp_lt_i8(a, b)); }
BL_INLINE_NODEBUG __m128i simd_cmp_le_i8(const __m128i& a, const __m128i& b) noexcept { return simd_not(simd_cmp_gt_i8(a, b)); }
BL_INLINE_NODEBUG __m128i simd_cmp_ge_i16(const __m128i& a, const __m128i& b) noexcept { return simd_not(simd_cmp_lt_i16(a, b)); }
BL_INLINE_NODEBUG __m128i simd_cmp_le_i16(const __m128i& a, const __m128i& b) noexcept { return simd_not(simd_cmp_gt_i16(a, b)); }
BL_INLINE_NODEBUG __m128i simd_cmp_ge_i32(const __m128i& a, const __m128i& b) noexcept { return simd_not(simd_cmp_lt_i32(a, b)); }
BL_INLINE_NODEBUG __m128i simd_cmp_le_i32(const __m128i& a, const __m128i& b) noexcept { return simd_not(simd_cmp_gt_i32(a, b)); }
#endif

#if defined(BL_TARGET_OPT_AVX512)
BL_INLINE_NODEBUG __m128i simd_cmp_ge_i64(const __m128i& a, const __m128i& b) noexcept { return simd_cmp_eq_i64(_mm_min_epi64(a, b), b); }
BL_INLINE_NODEBUG __m128i simd_cmp_le_i64(const __m128i& a, const __m128i& b) noexcept { return simd_cmp_eq_i64(_mm_max_epi64(a, b), b); }
#else
BL_INLINE_NODEBUG __m128i simd_cmp_ge_i64(const __m128i& a, const __m128i& b) noexcept { return simd_not(simd_cmp_lt_i64(a, b)); }
BL_INLINE_NODEBUG __m128i simd_cmp_le_i64(const __m128i& a, const __m128i& b) noexcept { return simd_not(simd_cmp_gt_i64(a, b)); }
#endif

BL_INLINE_NODEBUG __m128i simd_cmp_ge_u8(const __m128i& a, const __m128i& b) noexcept { return simd_cmp_eq_i8(_mm_min_epu8(a, b), b); }
BL_INLINE_NODEBUG __m128i simd_cmp_le_u8(const __m128i& a, const __m128i& b) noexcept { return simd_cmp_eq_i8(_mm_max_epu8(a, b), b); }

#if defined(BL_TARGET_OPT_SSE4_1)
BL_INLINE_NODEBUG __m128i simd_cmp_ge_u16(const __m128i& a, const __m128i& b) noexcept { return simd_cmp_eq_i16(_mm_min_epu16(a, b), b); }
BL_INLINE_NODEBUG __m128i simd_cmp_le_u16(const __m128i& a, const __m128i& b) noexcept { return simd_cmp_eq_i16(_mm_max_epu16(a, b), b); }
#else
BL_INLINE_NODEBUG __m128i simd_cmp_ge_u16(const __m128i& a, const __m128i& b) noexcept { return simd_cmp_eq_i16(_mm_subs_epu16(b, a), simd_make_zero<__m128i>()); }
BL_INLINE_NODEBUG __m128i simd_cmp_le_u16(const __m128i& a, const __m128i& b) noexcept { return simd_cmp_eq_i16(_mm_subs_epu16(a, b), simd_make_zero<__m128i>()); }
#endif

BL_INLINE_NODEBUG __m128i simd_cmp_gt_u8(const __m128i& a, const __m128i& b) noexcept { return simd_not(simd_cmp_le_u8(a, b)); }
BL_INLINE_NODEBUG __m128i simd_cmp_lt_u8(const __m128i& a, const __m128i& b) noexcept { return simd_not(simd_cmp_ge_u8(a, b)); }
BL_INLINE_NODEBUG __m128i simd_cmp_gt_u16(const __m128i& a, const __m128i& b) noexcept { return simd_not(simd_cmp_le_u16(a, b)); }
BL_INLINE_NODEBUG __m128i simd_cmp_lt_u16(const __m128i& a, const __m128i& b) noexcept { return simd_not(simd_cmp_ge_u16(a, b)); }

#if defined(BL_TARGET_OPT_SSE4_1)
BL_INLINE_NODEBUG __m128i simd_cmp_ge_u32(const __m128i& a, const __m128i& b) noexcept { return simd_cmp_eq_i32(_mm_min_epu32(a, b), b); }
BL_INLINE_NODEBUG __m128i simd_cmp_le_u32(const __m128i& a, const __m128i& b) noexcept { return simd_cmp_eq_i32(_mm_max_epu32(a, b), b); }
BL_INLINE_NODEBUG __m128i simd_cmp_gt_u32(const __m128i& a, const __m128i& b) noexcept { return simd_not(simd_cmp_le_u32(a, b)); }
BL_INLINE_NODEBUG __m128i simd_cmp_lt_u32(const __m128i& a, const __m128i& b) noexcept { return simd_not(simd_cmp_ge_u32(a, b)); }
#else
BL_INLINE_NODEBUG __m128i simd_cmp_gt_u32(const __m128i& a, const __m128i& b) noexcept { return simd_cmp_gt_i32(simd_flip_sign_i32(a), simd_flip_sign_i32(b)); }
BL_INLINE_NODEBUG __m128i simd_cmp_lt_u32(const __m128i& a, const __m128i& b) noexcept { return simd_cmp_gt_i32(simd_flip_sign_i32(b), simd_flip_sign_i32(a)); }

BL_INLINE_NODEBUG __m128i simd_cmp_ge_u32(const __m128i& a, const __m128i& b) noexcept { return simd_not(simd_cmp_lt_u32(a, b)); }
BL_INLINE_NODEBUG __m128i simd_cmp_le_u32(const __m128i& a, const __m128i& b) noexcept { return simd_not(simd_cmp_gt_u32(a, b)); }
#endif

#if defined(BL_TARGET_OPT_AVX512)
BL_INLINE_NODEBUG __m128i simd_cmp_gt_u64(const __m128i& a, const __m128i& b) noexcept { return simd_128i_from_mask64(_mm_cmp_epu64_mask(a, b, _MM_CMPINT_NLE)); }
BL_INLINE_NODEBUG __m128i simd_cmp_lt_u64(const __m128i& a, const __m128i& b) noexcept { return simd_128i_from_mask64(_mm_cmp_epu64_mask(a, b, _MM_CMPINT_LT)); }
#elif defined(BL_TARGET_OPT_SSE4_2)
BL_INLINE_NODEBUG __m128i simd_cmp_gt_u64(const __m128i& a, const __m128i& b) noexcept { return _mm_cmpgt_epi64(simd_flip_sign_i64(a), simd_flip_sign_i64(b)); }
BL_INLINE_NODEBUG __m128i simd_cmp_lt_u64(const __m128i& a, const __m128i& b) noexcept { return simd_cmp_gt_u64(b, a); }
#else
BL_INLINE_NODEBUG __m128i simd_cmp_gt_u64(const __m128i& a, const __m128i& b) noexcept {
  __m128i msk = _mm_andnot_si128(_mm_xor_si128(b, a), _mm_sub_epi64(b, a));
  msk = _mm_or_si128(msk, _mm_andnot_si128(b, a));
  return _mm_shuffle_epi32(_mm_srai_epi32(msk, 31), _MM_SHUFFLE(3, 3, 1, 1));
}
BL_INLINE_NODEBUG __m128i simd_cmp_lt_u64(const __m128i& a, const __m128i& b) noexcept { return simd_cmp_gt_u64(b, a); }
#endif

#if defined(BL_TARGET_OPT_AVX512)
BL_INLINE_NODEBUG __m128i simd_cmp_ge_u64(const __m128i& a, const __m128i& b) noexcept { return simd_cmp_eq_i64(_mm_min_epu64(a, b), b); }
BL_INLINE_NODEBUG __m128i simd_cmp_le_u64(const __m128i& a, const __m128i& b) noexcept { return simd_cmp_eq_i64(_mm_max_epu64(a, b), b); }
#else
BL_INLINE_NODEBUG __m128i simd_cmp_ge_u64(const __m128i& a, const __m128i& b) noexcept { return simd_not(simd_cmp_gt_u64(b, a)); }
BL_INLINE_NODEBUG __m128i simd_cmp_le_u64(const __m128i& a, const __m128i& b) noexcept { return simd_cmp_ge_u64(b, a); }
#endif

BL_INLINE_NODEBUG __m128i simd_min_i16(const __m128i& a, const __m128i& b) noexcept { return _mm_min_epi16(a, b); }
BL_INLINE_NODEBUG __m128i simd_max_i16(const __m128i& a, const __m128i& b) noexcept { return _mm_max_epi16(a, b); }
BL_INLINE_NODEBUG __m128i simd_min_u8(const __m128i& a, const __m128i& b) noexcept { return _mm_min_epu8(a, b); }
BL_INLINE_NODEBUG __m128i simd_max_u8(const __m128i& a, const __m128i& b) noexcept { return _mm_max_epu8(a, b); }

#if defined(BL_TARGET_OPT_SSE4_1)
BL_INLINE_NODEBUG __m128i simd_min_i8(const __m128i& a, const __m128i& b) noexcept { return _mm_min_epi8(a, b); }
BL_INLINE_NODEBUG __m128i simd_max_i8(const __m128i& a, const __m128i& b) noexcept { return _mm_max_epi8(a, b); }
BL_INLINE_NODEBUG __m128i simd_min_i32(const __m128i& a, const __m128i& b) noexcept { return _mm_min_epi32(a, b); }
BL_INLINE_NODEBUG __m128i simd_max_i32(const __m128i& a, const __m128i& b) noexcept { return _mm_max_epi32(a, b); }
BL_INLINE_NODEBUG __m128i simd_min_u16(const __m128i& a, const __m128i& b) noexcept { return _mm_min_epu16(a, b); }
BL_INLINE_NODEBUG __m128i simd_max_u16(const __m128i& a, const __m128i& b) noexcept { return _mm_max_epu16(a, b); }
BL_INLINE_NODEBUG __m128i simd_min_u32(const __m128i& a, const __m128i& b) noexcept { return _mm_min_epu32(a, b); }
BL_INLINE_NODEBUG __m128i simd_max_u32(const __m128i& a, const __m128i& b) noexcept { return _mm_max_epu32(a, b); }
#else
BL_INLINE_NODEBUG __m128i simd_min_i8(const __m128i& a, const __m128i& b) noexcept { return simd_blendv_u8(a, b, _mm_cmpgt_epi8(a, b)); }
BL_INLINE_NODEBUG __m128i simd_max_i8(const __m128i& a, const __m128i& b) noexcept { return simd_blendv_u8(b, a, _mm_cmpgt_epi8(a, b)); }
BL_INLINE_NODEBUG __m128i simd_min_i32(const __m128i& a, const __m128i& b) noexcept { return simd_blendv_u8(a, b, _mm_cmpgt_epi32(a, b)); }
BL_INLINE_NODEBUG __m128i simd_max_i32(const __m128i& a, const __m128i& b) noexcept { return simd_blendv_u8(b, a, _mm_cmpgt_epi32(a, b)); }
BL_INLINE_NODEBUG __m128i simd_min_u16(const __m128i& a, const __m128i& b) noexcept { return _mm_sub_epi16(a, _mm_subs_epu16(a, b)); }
BL_INLINE_NODEBUG __m128i simd_max_u16(const __m128i& a, const __m128i& b) noexcept { return _mm_add_epi16(a, _mm_subs_epu16(b, a)); }
BL_INLINE_NODEBUG __m128i simd_min_u32(const __m128i& a, const __m128i& b) noexcept { return simd_blendv_u8(a, b, _mm_cmpgt_epi32(simd_flip_sign_i32(a), simd_flip_sign_i32(b))); }
BL_INLINE_NODEBUG __m128i simd_max_u32(const __m128i& a, const __m128i& b) noexcept { return simd_blendv_u8(b, a, _mm_cmpgt_epi32(simd_flip_sign_i32(a), simd_flip_sign_i32(b))); }
#endif

#if defined(BL_TARGET_OPT_AVX512)
BL_INLINE_NODEBUG __m128i simd_min_i64(const __m128i& a, const __m128i& b) noexcept { return _mm_min_epi64(a, b); }
BL_INLINE_NODEBUG __m128i simd_max_i64(const __m128i& a, const __m128i& b) noexcept { return _mm_max_epi64(a, b); }
BL_INLINE_NODEBUG __m128i simd_min_u64(const __m128i& a, const __m128i& b) noexcept { return _mm_min_epu64(a, b); }
BL_INLINE_NODEBUG __m128i simd_max_u64(const __m128i& a, const __m128i& b) noexcept { return _mm_max_epu64(a, b); }
#else
BL_INLINE_NODEBUG __m128i simd_min_i64(const __m128i& a, const __m128i& b) noexcept { return simd_blendv_u8(a, b, simd_cmp_gt_i64(a, b)); }
BL_INLINE_NODEBUG __m128i simd_max_i64(const __m128i& a, const __m128i& b) noexcept { return simd_blendv_u8(a, b, simd_cmp_gt_i64(b, a)); }
BL_INLINE_NODEBUG __m128i simd_min_u64(const __m128i& a, const __m128i& b) noexcept { return simd_blendv_u8(a, b, simd_cmp_gt_i64(simd_flip_sign_i64(a), simd_flip_sign_i64(b))); }
BL_INLINE_NODEBUG __m128i simd_max_u64(const __m128i& a, const __m128i& b) noexcept { return simd_blendv_u8(b, a, simd_cmp_gt_i64(simd_flip_sign_i64(a), simd_flip_sign_i64(b))); }
#endif

#if defined(BL_TARGET_OPT_SSSE3)
BL_INLINE_NODEBUG __m128i simd_abs_i8(const __m128i& a) noexcept { return _mm_abs_epi8(a); }
BL_INLINE_NODEBUG __m128i simd_abs_i16(const __m128i& a) noexcept { return _mm_abs_epi16(a); }
BL_INLINE_NODEBUG __m128i simd_abs_i32(const __m128i& a) noexcept { return _mm_abs_epi32(a); }
#else
BL_INLINE_NODEBUG __m128i simd_abs_i8(const __m128i& a) noexcept {
  __m128i neg_val = _mm_sub_epi8(_mm_setzero_si128(), a);
  return _mm_min_epu8(neg_val, a);
}
BL_INLINE_NODEBUG __m128i simd_abs_i16(const __m128i& a) noexcept {
  __m128i neg_val = _mm_sub_epi16(_mm_setzero_si128(), a);
  return _mm_max_epi16(neg_val, a);
}
BL_INLINE_NODEBUG __m128i simd_abs_i32(const __m128i& a) noexcept {
  __m128i neg_msk = _mm_srai_epi32(a, 31);
  return _mm_sub_epi32(simd_xor(a, neg_msk), neg_msk);
}
#endif

#if defined(BL_TARGET_OPT_AVX512)
BL_INLINE_NODEBUG __m128i simd_abs_i64(const __m128i& a) noexcept { return _mm_abs_epi64(a); }
#else
BL_INLINE_NODEBUG __m128i simd_abs_i64(const __m128i& a) noexcept {
  __m128i neg_mask = _mm_srai_epi32(_mm_shuffle_epi32(a, _MM_SHUFFLE(3, 3, 1, 1)), 31);
  return _mm_sub_epi64(simd_xor(a, neg_mask), neg_mask);
}
#endif

template<uint8_t kN> BL_INLINE_NODEBUG __m128i simd_slli_i8(const __m128i& a) noexcept {
  constexpr uint8_t kShift = uint8_t(kN & 0x7);

  if constexpr (kShift == 0) {
    return a;
  }
  else if constexpr (kShift == 1) {
    return _mm_add_epi8(a, a);
  }
  else {
    __m128i msk = _mm_set1_epi8(int8_t((0xFFu << kShift) & 0xFFu));
    return _mm_and_si128(_mm_slli_epi16(a, kShift), msk);
  }
}

template<uint8_t kN> BL_INLINE_NODEBUG __m128i simd_srli_u8(const __m128i& a) noexcept {
  constexpr uint8_t kShift = uint8_t(kN & 0x7);

  if constexpr (kShift == 0) {
    return a;
  }
  else {
    __m128i msk = _mm_set1_epi8(int8_t((0xFFu >> kShift) & 0xFFu));
    return _mm_and_si128(_mm_srli_epi16(a, kShift), msk);
  }
}

template<uint8_t kN> BL_INLINE_NODEBUG __m128i simd_srai_i8(const __m128i& a) noexcept {
  constexpr uint8_t kShift = uint8_t(kN & 0x7);

  if constexpr (kShift == 0) {
    return a;
  }
  else if constexpr (kShift == 7) {
    return _mm_cmpgt_epi8(simd_make_zero<__m128i>(), a);
  }
  else {
    __m128i tmp = simd_srli_u8<kShift>(a);
    __m128i sgn = simd_make128_u8(0x80u >> kShift);
    return _mm_sub_epi8(simd_xor(tmp, sgn), sgn);
  }
}

template<uint8_t kN> BL_INLINE_NODEBUG __m128i simd_slli_i16(const __m128i& a) noexcept { return kN ? _mm_slli_epi16(a, kN) : a; }
template<uint8_t kN> BL_INLINE_NODEBUG __m128i simd_slli_i32(const __m128i& a) noexcept { return kN ? _mm_slli_epi32(a, kN) : a; }
template<uint8_t kN> BL_INLINE_NODEBUG __m128i simd_slli_i64(const __m128i& a) noexcept { return kN ? _mm_slli_epi64(a, kN) : a; }

template<uint8_t kN> BL_INLINE_NODEBUG __m128i simd_srli_u16(const __m128i& a) noexcept { return kN ? _mm_srli_epi16(a, kN) : a; }
template<uint8_t kN> BL_INLINE_NODEBUG __m128i simd_srli_u32(const __m128i& a) noexcept { return kN ? _mm_srli_epi32(a, kN) : a; }
template<uint8_t kN> BL_INLINE_NODEBUG __m128i simd_srli_u64(const __m128i& a) noexcept { return kN ? _mm_srli_epi64(a, kN) : a; }

template<uint8_t kN> BL_INLINE_NODEBUG __m128i simd_srai_i16(const __m128i& a) noexcept { return kN ? _mm_srai_epi16(a, kN) : a; }
template<uint8_t kN> BL_INLINE_NODEBUG __m128i simd_srai_i32(const __m128i& a) noexcept { return kN ? _mm_srai_epi32(a, kN) : a; }

template<uint8_t kN> BL_INLINE_NODEBUG __m128i simd_srai_i64(const __m128i& a) noexcept {
#if defined(BL_TARGET_OPT_AVX512)
  return kN ? _mm_srai_epi64(a, kN) : a;
#else
  if constexpr (kN == 0) {
    return a;
  }
  else if constexpr (kN == 63u) {
    return _mm_srai_epi32(_mm_shuffle_epi32(a, _MM_SHUFFLE(3, 3, 1, 1)), 31);
  }
#if defined(BL_TARGET_OPT_SSE4_1)
  else if constexpr (kN < 32u) {
    __m128i hi = _mm_srai_epi32(a, kN & 31u);
    __m128i lo = _mm_srli_epi64(a, kN & 31u);
    return _mm_blend_epi16(lo, hi, 0xCCu);
  }
#endif
  else {
    __m128i highs = _mm_shuffle_epi32(a, _MM_SHUFFLE(3, 3, 1, 1));
    __m128i signs = _mm_srai_epi32(highs, 31);
    __m128i msk = _mm_slli_epi64(signs, (64 - kN) & 63);
    return _mm_or_si128(msk, _mm_srli_epi64(a, kN));
  }
#endif
}

template<uint8_t kNumBytes> BL_INLINE_NODEBUG __m128i simd_sllb_u128(const __m128i& a) noexcept { return kNumBytes ? _mm_slli_si128(a, kNumBytes) : a; }
template<uint8_t kNumBytes> BL_INLINE_NODEBUG __m128i simd_srlb_u128(const __m128i& a) noexcept { return kNumBytes ? _mm_srli_si128(a, kNumBytes) : a; }

BL_INLINE_NODEBUG __m128i simd_sad_u8_u64(const __m128i& a, const __m128i& b) noexcept { return _mm_sad_epu8(a, b); }

#if defined(BL_TARGET_OPT_SSSE3)
BL_INLINE_NODEBUG __m128i simd_maddws_u8xi8_i16(const __m128i& a, const __m128i& b) noexcept { return _mm_maddubs_epi16(a, b); }
#endif // BL_TARGET_OPT_SSSE3

#if defined(BL_TARGET_OPT_SSE4_2)
BL_INLINE_NODEBUG __m128i simd_clmul_u128_ll(const __m128i& a, const __m128i& b) noexcept { return _mm_clmulepi64_si128(a, b, 0x00); }
BL_INLINE_NODEBUG __m128i simd_clmul_u128_lh(const __m128i& a, const __m128i& b) noexcept { return _mm_clmulepi64_si128(a, b, 0x10); }
BL_INLINE_NODEBUG __m128i simd_clmul_u128_hl(const __m128i& a, const __m128i& b) noexcept { return _mm_clmulepi64_si128(a, b, 0x01); }
BL_INLINE_NODEBUG __m128i simd_clmul_u128_hh(const __m128i& a, const __m128i& b) noexcept { return _mm_clmulepi64_si128(a, b, 0x11); }
#endif // BL_TARGET_OPT_SSE4_2

#if defined(BL_TARGET_OPT_AVX512)
template<uint8_t kPred>
BL_INLINE_NODEBUG __m256i simd_ternlog(const __m256i& a, const __m256i& b, const __m256i& c) noexcept { return _mm256_ternarylogic_epi32(a, b, c, kPred); }

template<uint8_t kPred>
BL_INLINE_NODEBUG __m256 simd_ternlog(const __m256& a, const __m256& b, const __m256& c) noexcept { return simd_as_f(simd_ternlog<kPred>(simd_as_i(a), simd_as_i(b), simd_as_i(c))); }

template<uint8_t kPred>
BL_INLINE_NODEBUG __m256d simd_ternlog(const __m256d& a, const __m256d& b, const __m256d& c) noexcept { return simd_as_d(simd_ternlog<kPred>(simd_as_i(a), simd_as_i(b), simd_as_i(c))); }
#endif // BL_TARGET_OPT_AVX512

#if defined(BL_TARGET_OPT_AVX)
BL_INLINE_NODEBUG __m256 simd_and(const __m256& a, const __m256& b) noexcept { return _mm256_and_ps(a, b); }
BL_INLINE_NODEBUG __m256d simd_and(const __m256d& a, const __m256d& b) noexcept { return _mm256_and_pd(a, b); }

BL_INLINE_NODEBUG __m256 simd_andnot(const __m256& a, const __m256& b) noexcept { return _mm256_andnot_ps(a, b); }
BL_INLINE_NODEBUG __m256d simd_andnot(const __m256d& a, const __m256d& b) noexcept { return _mm256_andnot_pd(a, b); }

BL_INLINE_NODEBUG __m256 simd_or(const __m256& a, const __m256& b) noexcept { return _mm256_or_ps(a, b); }
BL_INLINE_NODEBUG __m256d simd_or(const __m256d& a, const __m256d& b) noexcept { return _mm256_or_pd(a, b); }

BL_INLINE_NODEBUG __m256 simd_xor(const __m256& a, const __m256& b) noexcept { return _mm256_xor_ps(a, b); }
BL_INLINE_NODEBUG __m256d simd_xor(const __m256d& a, const __m256d& b) noexcept { return _mm256_xor_pd(a, b); }

#if defined(BL_TARGET_OPT_AVX512)
BL_INLINE_NODEBUG __m256 simd_not(const __m256& a) noexcept { return simd_ternlog<0x55>(a, a, a); }
BL_INLINE_NODEBUG __m256d simd_not(const __m256d& a) noexcept { return simd_ternlog<0x55>(a, a, a); }

BL_INLINE_NODEBUG __m256 simd_blendv_bits(const __m256& a, const __m256& b, const __m256& msk) noexcept { return simd_ternlog<0xD8>(a, b, msk); }
BL_INLINE_NODEBUG __m256d simd_blendv_bits(const __m256d& a, const __m256d& b, const __m256d& msk) noexcept { return simd_ternlog<0xD8>(a, b, msk); }
#else
BL_INLINE_NODEBUG __m256 simd_not(const __m256& a) noexcept { return simd_xor(a, simd_make_ones<__m256>()); }
BL_INLINE_NODEBUG __m256d simd_not(const __m256d& a) noexcept { return simd_xor(a, simd_make_ones<__m256d>()); }

BL_INLINE_NODEBUG __m256 simd_blendv_bits(const __m256& a, const __m256& b, const __m256& msk) noexcept { return simd_or(simd_and(b, msk), simd_andnot(msk, a)); }
BL_INLINE_NODEBUG __m256d simd_blendv_bits(const __m256d& a, const __m256d& b, const __m256d& msk) noexcept { return simd_or(simd_and(b, msk), simd_andnot(msk, a)); }
#endif

#if defined(BL_TARGET_OPT_AVX2)
BL_INLINE_NODEBUG __m256i simd_and(const __m256i& a, const __m256i& b) noexcept { return _mm256_and_si256(a, b); }
BL_INLINE_NODEBUG __m256i simd_andnot(const __m256i& a, const __m256i& b) noexcept { return _mm256_andnot_si256(a, b); }
BL_INLINE_NODEBUG __m256i simd_or(const __m256i& a, const __m256i& b) noexcept { return _mm256_or_si256(a, b); }
BL_INLINE_NODEBUG __m256i simd_xor(const __m256i& a, const __m256i& b) noexcept { return _mm256_xor_si256(a, b); }
#else
BL_INLINE_NODEBUG __m256i simd_and(const __m256i& a, const __m256i& b) noexcept { return simd_as_i(_mm256_and_ps(simd_as_f(a), simd_as_f(b))); }
BL_INLINE_NODEBUG __m256i simd_andnot(const __m256i& a, const __m256i& b) noexcept { return simd_as_i(_mm256_andnot_ps(simd_as_f(a), simd_as_f(b))); }
BL_INLINE_NODEBUG __m256i simd_or(const __m256i& a, const __m256i& b) noexcept { return simd_as_i(_mm256_or_ps(simd_as_f(a), simd_as_f(b))); }
BL_INLINE_NODEBUG __m256i simd_xor(const __m256i& a, const __m256i& b) noexcept { return simd_as_i(_mm256_xor_ps(simd_as_f(a), simd_as_f(b))); }
#endif

#if defined(BL_TARGET_OPT_AVX512)
BL_INLINE_NODEBUG __m256i simd_not(const __m256i& a) noexcept { return simd_ternlog<0x55>(a, a, a); }
BL_INLINE_NODEBUG __m256i simd_blendv_bits(const __m256i& a, const __m256i& b, const __m256i& msk) noexcept { return simd_ternlog<0xD8>(a, b, msk); }
#else
BL_INLINE_NODEBUG __m256i simd_not(const __m256i& a) noexcept { return simd_xor(a, simd_make_ones<__m256i>()); }
BL_INLINE_NODEBUG __m256i simd_blendv_bits(const __m256i& a, const __m256i& b, const __m256i& msk) noexcept { return simd_or(simd_and(b, msk), simd_andnot(msk, a)); }
#endif

#if defined(BL_TARGET_OPT_AVX2)
BL_INLINE_NODEBUG __m256i simd_blendv_u8(const __m256i& a, const __m256i& b, const __m256i& msk) noexcept { return _mm256_blendv_epi8(a, b, msk); }
#endif

BL_INLINE_NODEBUG __m256i simd_flip_sign_i8(const __m256i& a) noexcept { return simd_xor(a, simd_make256_u32(0x80808080u)); }
BL_INLINE_NODEBUG __m256i simd_flip_sign_i16(const __m256i& a) noexcept { return simd_xor(a, simd_make256_u32(0x80008000u)); }
BL_INLINE_NODEBUG __m256i simd_flip_sign_i32(const __m256i& a) noexcept { return simd_xor(a, simd_make256_u32(0x80000000u)); }
BL_INLINE_NODEBUG __m256i simd_flip_sign_i64(const __m256i& a) noexcept { return simd_xor(a, simd_make256_u64(uint64_t(1) << 63)); }

BL_INLINE_NODEBUG __m256 simd_add_f32(const __m256& a, const __m256& b) noexcept { return _mm256_add_ps(a, b); }
BL_INLINE_NODEBUG __m256d simd_add_f64(const __m256d& a, const __m256d& b) noexcept { return _mm256_add_pd(a, b); }

BL_INLINE_NODEBUG __m256 simd_sub_f32(const __m256& a, const __m256& b) noexcept { return _mm256_sub_ps(a, b); }
BL_INLINE_NODEBUG __m256d simd_sub_f64(const __m256d& a, const __m256d& b) noexcept { return _mm256_sub_pd(a, b); }

BL_INLINE_NODEBUG __m256 simd_mul_f32(const __m256& a, const __m256& b) noexcept { return _mm256_mul_ps(a, b); }
BL_INLINE_NODEBUG __m256d simd_mul_f64(const __m256d& a, const __m256d& b) noexcept { return _mm256_mul_pd(a, b); }

BL_INLINE_NODEBUG __m256 simd_div_f32(const __m256& a, const __m256& b) noexcept { return _mm256_div_ps(a, b); }
BL_INLINE_NODEBUG __m256d simd_div_f64(const __m256d& a, const __m256d& b) noexcept { return _mm256_div_pd(a, b); }

BL_INLINE_NODEBUG __m256 simd_cmp_eq_f32(const __m256& a, const __m256& b) noexcept { return _mm256_cmp_ps(a, b, _CMP_EQ_OQ); }
BL_INLINE_NODEBUG __m256d simd_cmp_eq_f64(const __m256d& a, const __m256d& b) noexcept { return _mm256_cmp_pd(a, b, _CMP_EQ_OQ); }

BL_INLINE_NODEBUG __m256 simd_cmp_ne_f32(const __m256& a, const __m256& b) noexcept { return _mm256_cmp_ps(a, b, _CMP_NEQ_OQ); }
BL_INLINE_NODEBUG __m256d simd_cmp_ne_f64(const __m256d& a, const __m256d& b) noexcept { return _mm256_cmp_pd(a, b, _CMP_NEQ_OQ); }

BL_INLINE_NODEBUG __m256 simd_cmp_lt_f32(const __m256& a, const __m256& b) noexcept { return _mm256_cmp_ps(a, b, _CMP_LT_OQ); }
BL_INLINE_NODEBUG __m256d simd_cmp_lt_f64(const __m256d& a, const __m256d& b) noexcept { return _mm256_cmp_pd(a, b, _CMP_LT_OQ); }

BL_INLINE_NODEBUG __m256 simd_cmp_le_f32(const __m256& a, const __m256& b) noexcept { return _mm256_cmp_ps(a, b, _CMP_LE_OQ); }
BL_INLINE_NODEBUG __m256d simd_cmp_le_f64(const __m256d& a, const __m256d& b) noexcept { return _mm256_cmp_pd(a, b, _CMP_LE_OQ); }

BL_INLINE_NODEBUG __m256 simd_cmp_gt_f32(const __m256& a, const __m256& b) noexcept { return _mm256_cmp_ps(a, b, _CMP_GT_OQ); }
BL_INLINE_NODEBUG __m256d simd_cmp_gt_f64(const __m256d& a, const __m256d& b) noexcept { return _mm256_cmp_pd(a, b, _CMP_GT_OQ); }

BL_INLINE_NODEBUG __m256 simd_cmp_ge_f32(const __m256& a, const __m256& b) noexcept { return _mm256_cmp_ps(a, b, _CMP_GE_OQ); }
BL_INLINE_NODEBUG __m256d simd_cmp_ge_f64(const __m256d& a, const __m256d& b) noexcept { return _mm256_cmp_pd(a, b, _CMP_GE_OQ); }

BL_INLINE_NODEBUG __m256 simd_min_f32(const __m256& a, const __m256& b) noexcept { return _mm256_min_ps(a, b); }
BL_INLINE_NODEBUG __m256d simd_min_f64(const __m256d& a, const __m256d& b) noexcept { return _mm256_min_pd(a, b); }

BL_INLINE_NODEBUG __m256 simd_max_f32(const __m256& a, const __m256& b) noexcept { return _mm256_max_ps(a, b); }
BL_INLINE_NODEBUG __m256d simd_max_f64(const __m256d& a, const __m256d& b) noexcept { return _mm256_max_pd(a, b); }

BL_INLINE_NODEBUG __m256 simd_abs_f32(const __m256& a) noexcept { return _mm256_and_ps(a, bl::common_table.p_7FFFFFFF7FFFFFFF.as<__m256>()); }
BL_INLINE_NODEBUG __m256d simd_abs_f64(const __m256d& a) noexcept { return _mm256_and_pd(a, bl::common_table.p_7FFFFFFFFFFFFFFF.as<__m256d>()); }

BL_INLINE_NODEBUG __m256 simd_sqrt_f32(const __m256& a) noexcept { return _mm256_sqrt_ps(a); }
BL_INLINE_NODEBUG __m256d simd_sqrt_f64(const __m256d& a) noexcept { return _mm256_sqrt_pd(a); }
#endif // BL_TARGET_OPT_AVX

#if defined(BL_TARGET_OPT_AVX2)
BL_INLINE_NODEBUG __m256i simd_add_i8(const __m256i& a, const __m256i& b) noexcept { return _mm256_add_epi8(a, b); }
BL_INLINE_NODEBUG __m256i simd_add_i16(const __m256i& a, const __m256i& b) noexcept { return _mm256_add_epi16(a, b); }
BL_INLINE_NODEBUG __m256i simd_add_i32(const __m256i& a, const __m256i& b) noexcept { return _mm256_add_epi32(a, b); }
BL_INLINE_NODEBUG __m256i simd_add_i64(const __m256i& a, const __m256i& b) noexcept { return _mm256_add_epi64(a, b); }
BL_INLINE_NODEBUG __m256i simd_add_u8(const __m256i& a, const __m256i& b) noexcept { return _mm256_add_epi8(a, b); }
BL_INLINE_NODEBUG __m256i simd_add_u16(const __m256i& a, const __m256i& b) noexcept { return _mm256_add_epi16(a, b); }
BL_INLINE_NODEBUG __m256i simd_add_u32(const __m256i& a, const __m256i& b) noexcept { return _mm256_add_epi32(a, b); }
BL_INLINE_NODEBUG __m256i simd_add_u64(const __m256i& a, const __m256i& b) noexcept { return _mm256_add_epi64(a, b); }

BL_INLINE_NODEBUG __m256i simd_adds_i8(const __m256i& a, const __m256i& b) noexcept { return _mm256_adds_epi8(a, b); }
BL_INLINE_NODEBUG __m256i simd_adds_i16(const __m256i& a, const __m256i& b) noexcept { return _mm256_adds_epi16(a, b); }
BL_INLINE_NODEBUG __m256i simd_adds_u8(const __m256i& a, const __m256i& b) noexcept { return _mm256_adds_epu8(a, b); }
BL_INLINE_NODEBUG __m256i simd_adds_u16(const __m256i& a, const __m256i& b) noexcept { return _mm256_adds_epu16(a, b); }

BL_INLINE_NODEBUG __m256i simd_sub_i8(const __m256i& a, const __m256i& b) noexcept { return _mm256_sub_epi8(a, b); }
BL_INLINE_NODEBUG __m256i simd_sub_i16(const __m256i& a, const __m256i& b) noexcept { return _mm256_sub_epi16(a, b); }
BL_INLINE_NODEBUG __m256i simd_sub_i32(const __m256i& a, const __m256i& b) noexcept { return _mm256_sub_epi32(a, b); }
BL_INLINE_NODEBUG __m256i simd_sub_i64(const __m256i& a, const __m256i& b) noexcept { return _mm256_sub_epi64(a, b); }
BL_INLINE_NODEBUG __m256i simd_sub_u8(const __m256i& a, const __m256i& b) noexcept { return _mm256_sub_epi8(a, b); }
BL_INLINE_NODEBUG __m256i simd_sub_u16(const __m256i& a, const __m256i& b) noexcept { return _mm256_sub_epi16(a, b); }
BL_INLINE_NODEBUG __m256i simd_sub_u32(const __m256i& a, const __m256i& b) noexcept { return _mm256_sub_epi32(a, b); }
BL_INLINE_NODEBUG __m256i simd_sub_u64(const __m256i& a, const __m256i& b) noexcept { return _mm256_sub_epi64(a, b); }

BL_INLINE_NODEBUG __m256i simd_subs_i8(const __m256i& a, const __m256i& b) noexcept { return _mm256_subs_epi8(a, b); }
BL_INLINE_NODEBUG __m256i simd_subs_i16(const __m256i& a, const __m256i& b) noexcept { return _mm256_subs_epi16(a, b); }
BL_INLINE_NODEBUG __m256i simd_subs_u8(const __m256i& a, const __m256i& b) noexcept { return _mm256_subs_epu8(a, b); }
BL_INLINE_NODEBUG __m256i simd_subs_u16(const __m256i& a, const __m256i& b) noexcept { return _mm256_subs_epu16(a, b); }

BL_INLINE_NODEBUG __m256i simd_mul_i16(const __m256i& a, const __m256i& b) noexcept { return _mm256_mullo_epi16(a, b); }
BL_INLINE_NODEBUG __m256i simd_mul_u16(const __m256i& a, const __m256i& b) noexcept { return _mm256_mullo_epi16(a, b); }

BL_INLINE_NODEBUG __m256i simd_mul_i32(const __m256i& a, const __m256i& b) noexcept { return _mm256_mullo_epi32(a, b); }
BL_INLINE_NODEBUG __m256i simd_mul_u32(const __m256i& a, const __m256i& b) noexcept { return _mm256_mullo_epi32(a, b); }

#if defined(BL_TARGET_OPT_AVX512)
BL_INLINE_NODEBUG __m256i simd_mul_i64(const __m256i& a, const __m256i& b) noexcept { return _mm256_mullo_epi64(a, b); }
#else
BL_INLINE_NODEBUG __m256i simd_mul_i64(const __m256i& a, const __m256i& b) noexcept {
  __m256i al_bh = _mm256_mul_epu32(a, _mm256_srli_epi64(b, 32));
  __m256i ah_bl = _mm256_mul_epu32(b, _mm256_srli_epi64(a, 32));
  __m256i al_bl = _mm256_mul_epu32(a, b);
  __m256i prod1 = _mm256_slli_epi64(_mm256_add_epi64(al_bh, ah_bl), 32);
  return _mm256_add_epi64(al_bl, prod1);
}
#endif
BL_INLINE_NODEBUG __m256i simd_mul_u64(const __m256i& a, const __m256i& b) noexcept { return simd_mul_i64(a, b); }

BL_INLINE_NODEBUG __m256i simd_mulh_i16(const __m256i& a, const __m256i& b) noexcept { return _mm256_mulhi_epi16(a, b); }
BL_INLINE_NODEBUG __m256i simd_mulh_u16(const __m256i& a, const __m256i& b) noexcept { return _mm256_mulhi_epu16(a, b); }
BL_INLINE_NODEBUG __m256i simd_mulw_i32(const __m256i& a, const __m256i& b) noexcept { return _mm256_mul_epi32(a, b); }
BL_INLINE_NODEBUG __m256i simd_mulw_u32(const __m256i& a, const __m256i& b) noexcept { return _mm256_mul_epu32(a, b); }

BL_INLINE_NODEBUG __m256i simd_maddw_i16_i32(const __m256i& a, const __m256i& b) noexcept { return _mm256_madd_epi16(a, b); }

BL_INLINE_NODEBUG __m256i simd_cmp_eq_i8(const __m256i& a, const __m256i& b) noexcept { return _mm256_cmpeq_epi8(a, b); }
BL_INLINE_NODEBUG __m256i simd_cmp_eq_i16(const __m256i& a, const __m256i& b) noexcept { return _mm256_cmpeq_epi16(a, b); }
BL_INLINE_NODEBUG __m256i simd_cmp_eq_i32(const __m256i& a, const __m256i& b) noexcept { return _mm256_cmpeq_epi32(a, b); }
BL_INLINE_NODEBUG __m256i simd_cmp_eq_i64(const __m256i& a, const __m256i& b) noexcept { return _mm256_cmpeq_epi64(a, b); }

BL_INLINE_NODEBUG __m256i simd_cmp_ne_i8(const __m256i& a, const __m256i& b) noexcept { return simd_not(simd_cmp_eq_i8(a, b)); }
BL_INLINE_NODEBUG __m256i simd_cmp_ne_i16(const __m256i& a, const __m256i& b) noexcept { return simd_not(simd_cmp_eq_i16(a, b)); }
BL_INLINE_NODEBUG __m256i simd_cmp_ne_i32(const __m256i& a, const __m256i& b) noexcept { return simd_not(simd_cmp_eq_i32(a, b)); }
BL_INLINE_NODEBUG __m256i simd_cmp_ne_i64(const __m256i& a, const __m256i& b) noexcept { return simd_not(simd_cmp_eq_i64(a, b)); }

BL_INLINE_NODEBUG __m256i simd_cmp_gt_i8(const __m256i& a, const __m256i& b) noexcept { return _mm256_cmpgt_epi8(a, b); }
BL_INLINE_NODEBUG __m256i simd_cmp_lt_i8(const __m256i& a, const __m256i& b) noexcept { return simd_cmp_gt_i8(b, a); }
BL_INLINE_NODEBUG __m256i simd_cmp_gt_i16(const __m256i& a, const __m256i& b) noexcept { return _mm256_cmpgt_epi16(a, b); }
BL_INLINE_NODEBUG __m256i simd_cmp_lt_i16(const __m256i& a, const __m256i& b) noexcept { return simd_cmp_gt_i16(b, a); }
BL_INLINE_NODEBUG __m256i simd_cmp_gt_i32(const __m256i& a, const __m256i& b) noexcept { return _mm256_cmpgt_epi32(a, b); }
BL_INLINE_NODEBUG __m256i simd_cmp_lt_i32(const __m256i& a, const __m256i& b) noexcept { return simd_cmp_gt_i32(b, a); }

BL_INLINE_NODEBUG __m256i simd_cmp_ge_i8(const __m256i& a, const __m256i& b) noexcept { return simd_cmp_eq_i8(_mm256_min_epi8(a, b), b); }
BL_INLINE_NODEBUG __m256i simd_cmp_le_i8(const __m256i& a, const __m256i& b) noexcept { return simd_cmp_eq_i8(_mm256_max_epi8(a, b), b); }
BL_INLINE_NODEBUG __m256i simd_cmp_ge_i16(const __m256i& a, const __m256i& b) noexcept { return simd_cmp_eq_i16(_mm256_min_epi16(a, b), b); }
BL_INLINE_NODEBUG __m256i simd_cmp_le_i16(const __m256i& a, const __m256i& b) noexcept { return simd_cmp_eq_i16(_mm256_max_epi16(a, b), b); }
BL_INLINE_NODEBUG __m256i simd_cmp_ge_i32(const __m256i& a, const __m256i& b) noexcept { return simd_cmp_eq_i32(_mm256_min_epi32(a, b), b); }
BL_INLINE_NODEBUG __m256i simd_cmp_le_i32(const __m256i& a, const __m256i& b) noexcept { return simd_cmp_eq_i32(_mm256_max_epi32(a, b), b); }

BL_INLINE_NODEBUG __m256i simd_cmp_ge_u8(const __m256i& a, const __m256i& b) noexcept { return simd_cmp_eq_i8(_mm256_min_epu8(a, b), b); }
BL_INLINE_NODEBUG __m256i simd_cmp_le_u8(const __m256i& a, const __m256i& b) noexcept { return simd_cmp_eq_i8(_mm256_max_epu8(a, b), b); }
BL_INLINE_NODEBUG __m256i simd_cmp_ge_u16(const __m256i& a, const __m256i& b) noexcept { return simd_cmp_eq_i16(_mm256_min_epu16(a, b), b); }
BL_INLINE_NODEBUG __m256i simd_cmp_le_u16(const __m256i& a, const __m256i& b) noexcept { return simd_cmp_eq_i16(_mm256_max_epu16(a, b), b); }
BL_INLINE_NODEBUG __m256i simd_cmp_ge_u32(const __m256i& a, const __m256i& b) noexcept { return simd_cmp_eq_i32(_mm256_min_epu32(a, b), b); }
BL_INLINE_NODEBUG __m256i simd_cmp_le_u32(const __m256i& a, const __m256i& b) noexcept { return simd_cmp_eq_i32(_mm256_max_epu32(a, b), b); }

BL_INLINE_NODEBUG __m256i simd_cmp_gt_u8(const __m256i& a, const __m256i& b) noexcept { return simd_not(simd_cmp_le_u8(a, b)); }
BL_INLINE_NODEBUG __m256i simd_cmp_lt_u8(const __m256i& a, const __m256i& b) noexcept { return simd_not(simd_cmp_ge_u8(a, b)); }
BL_INLINE_NODEBUG __m256i simd_cmp_gt_u16(const __m256i& a, const __m256i& b) noexcept { return simd_not(simd_cmp_le_u16(a, b)); }
BL_INLINE_NODEBUG __m256i simd_cmp_lt_u16(const __m256i& a, const __m256i& b) noexcept { return simd_not(simd_cmp_ge_u16(a, b)); }
BL_INLINE_NODEBUG __m256i simd_cmp_gt_u32(const __m256i& a, const __m256i& b) noexcept { return simd_not(simd_cmp_le_u32(a, b)); }
BL_INLINE_NODEBUG __m256i simd_cmp_lt_u32(const __m256i& a, const __m256i& b) noexcept { return simd_not(simd_cmp_ge_u32(a, b)); }

BL_INLINE_NODEBUG __m256i simd_cmp_gt_i64(const __m256i& a, const __m256i& b) noexcept { return _mm256_cmpgt_epi64(a, b); }
BL_INLINE_NODEBUG __m256i simd_cmp_lt_i64(const __m256i& a, const __m256i& b) noexcept { return _mm256_cmpgt_epi64(b, a); }

#if defined(BL_TARGET_OPT_AVX512)
BL_INLINE_NODEBUG __m256i simd_cmp_ge_i64(const __m256i& a, const __m256i& b) noexcept { return simd_cmp_eq_i64(_mm256_min_epi64(a, b), b); }
BL_INLINE_NODEBUG __m256i simd_cmp_le_i64(const __m256i& a, const __m256i& b) noexcept { return simd_cmp_eq_i64(_mm256_max_epi64(a, b), b); }
BL_INLINE_NODEBUG __m256i simd_cmp_gt_u64(const __m256i& a, const __m256i& b) noexcept { return simd_256i_from_mask64(_mm256_cmp_epu64_mask(a, b, _MM_CMPINT_NLE)); }
BL_INLINE_NODEBUG __m256i simd_cmp_ge_u64(const __m256i& a, const __m256i& b) noexcept { return simd_256i_from_mask64(_mm256_cmp_epu64_mask(a, b, _MM_CMPINT_NLT)); }
#else
BL_INLINE_NODEBUG __m256i simd_cmp_ge_i64(const __m256i& a, const __m256i& b) noexcept { return simd_not(simd_cmp_lt_i64(a, b)); }
BL_INLINE_NODEBUG __m256i simd_cmp_le_i64(const __m256i& a, const __m256i& b) noexcept { return simd_not(simd_cmp_gt_i64(a, b)); }
BL_INLINE_NODEBUG __m256i simd_cmp_gt_u64(const __m256i& a, const __m256i& b) noexcept { return _mm256_cmpgt_epi64(simd_flip_sign_i64(a), simd_flip_sign_i64(b)); }
BL_INLINE_NODEBUG __m256i simd_cmp_ge_u64(const __m256i& a, const __m256i& b) noexcept { return simd_not(simd_cmp_gt_u64(b, a)); }
#endif

BL_INLINE_NODEBUG __m256i simd_cmp_lt_u64(const __m256i& a, const __m256i& b) noexcept { return simd_cmp_gt_u64(b, a); }
BL_INLINE_NODEBUG __m256i simd_cmp_le_u64(const __m256i& a, const __m256i& b) noexcept { return simd_cmp_ge_u64(b, a); }

BL_INLINE_NODEBUG __m256i simd_min_i8(const __m256i& a, const __m256i& b) noexcept { return _mm256_min_epi8(a, b); }
BL_INLINE_NODEBUG __m256i simd_min_i16(const __m256i& a, const __m256i& b) noexcept { return _mm256_min_epi16(a, b); }
BL_INLINE_NODEBUG __m256i simd_min_i32(const __m256i& a, const __m256i& b) noexcept { return _mm256_min_epi32(a, b); }
BL_INLINE_NODEBUG __m256i simd_min_u8(const __m256i& a, const __m256i& b) noexcept { return _mm256_min_epu8(a, b); }
BL_INLINE_NODEBUG __m256i simd_min_u16(const __m256i& a, const __m256i& b) noexcept { return _mm256_min_epu16(a, b); }
BL_INLINE_NODEBUG __m256i simd_min_u32(const __m256i& a, const __m256i& b) noexcept { return _mm256_min_epu32(a, b); }

BL_INLINE_NODEBUG __m256i simd_max_i8(const __m256i& a, const __m256i& b) noexcept { return _mm256_max_epi8(a, b); }
BL_INLINE_NODEBUG __m256i simd_max_i16(const __m256i& a, const __m256i& b) noexcept { return _mm256_max_epi16(a, b); }
BL_INLINE_NODEBUG __m256i simd_max_i32(const __m256i& a, const __m256i& b) noexcept { return _mm256_max_epi32(a, b); }
BL_INLINE_NODEBUG __m256i simd_max_u8(const __m256i& a, const __m256i& b) noexcept { return _mm256_max_epu8(a, b); }
BL_INLINE_NODEBUG __m256i simd_max_u16(const __m256i& a, const __m256i& b) noexcept { return _mm256_max_epu16(a, b); }
BL_INLINE_NODEBUG __m256i simd_max_u32(const __m256i& a, const __m256i& b) noexcept { return _mm256_max_epu32(a, b); }

#if defined(BL_TARGET_OPT_AVX512)
BL_INLINE_NODEBUG __m256i simd_min_i64(const __m256i& a, const __m256i& b) noexcept { return _mm256_min_epi64(a, b); }
BL_INLINE_NODEBUG __m256i simd_max_i64(const __m256i& a, const __m256i& b) noexcept { return _mm256_max_epi64(a, b); }
BL_INLINE_NODEBUG __m256i simd_min_u64(const __m256i& a, const __m256i& b) noexcept { return _mm256_min_epu64(a, b); }
BL_INLINE_NODEBUG __m256i simd_max_u64(const __m256i& a, const __m256i& b) noexcept { return _mm256_max_epu64(a, b); }
#else
BL_INLINE_NODEBUG __m256i simd_min_i64(const __m256i& a, const __m256i& b) noexcept { return simd_blendv_u8(a, b, simd_cmp_gt_i64(a, b)); }
BL_INLINE_NODEBUG __m256i simd_max_i64(const __m256i& a, const __m256i& b) noexcept { return simd_blendv_u8(b, a, simd_cmp_gt_i64(a, b)); }
BL_INLINE_NODEBUG __m256i simd_min_u64(const __m256i& a, const __m256i& b) noexcept { return simd_blendv_u8(a, b, simd_cmp_gt_u64(a, b)); }
BL_INLINE_NODEBUG __m256i simd_max_u64(const __m256i& a, const __m256i& b) noexcept { return simd_blendv_u8(b, a, simd_cmp_gt_u64(a, b)); }
#endif

BL_INLINE_NODEBUG __m256i simd_abs_i8(const __m256i& a) noexcept { return _mm256_abs_epi8(a); }
BL_INLINE_NODEBUG __m256i simd_abs_i16(const __m256i& a) noexcept { return _mm256_abs_epi16(a); }
BL_INLINE_NODEBUG __m256i simd_abs_i32(const __m256i& a) noexcept { return _mm256_abs_epi32(a); }

#if defined(BL_TARGET_OPT_AVX512)
BL_INLINE_NODEBUG __m256i simd_abs_i64(const __m256i& a) noexcept { return _mm256_abs_epi64(a); }
#else
BL_INLINE_NODEBUG __m256i simd_abs_i64(const __m256i& a) noexcept {
  __m256i neg_mask = _mm256_srai_epi32(_mm256_shuffle_epi32(a, _MM_SHUFFLE(3, 3, 1, 1)), 31);
  return _mm256_sub_epi64(simd_xor(a, neg_mask), neg_mask);
}
#endif

template<uint8_t kN> BL_INLINE_NODEBUG __m256i simd_slli_i8(const __m256i& a) noexcept {
  constexpr uint8_t kShift = uint8_t(kN & 0x7);

  if constexpr (kShift == 0) {
    return a;
  }
  else if constexpr (kShift == 1) {
    return _mm256_add_epi8(a, a);
  }
  else {
    __m256i msk = _mm256_set1_epi8(int8_t((0xFFu << kShift) & 0xFFu));
    return _mm256_and_si256(_mm256_slli_epi16(a, kShift), msk);
  }
}

template<uint8_t kN> BL_INLINE_NODEBUG __m256i simd_srli_u8(const __m256i& a) noexcept {
  constexpr uint8_t kShift = uint8_t(kN & 0x7);

  if constexpr (kShift == 0) {
    return a;
  }
  else {
    __m256i msk = _mm256_set1_epi8(int8_t((0xFFu >> kShift) & 0xFFu));
    return _mm256_and_si256(_mm256_srli_epi16(a, kShift), msk);
  }
}

template<uint8_t kN> BL_INLINE_NODEBUG __m256i simd_srai_i8(const __m256i& a) noexcept {
  constexpr uint8_t kShift = uint8_t(kN & 0x7);

  if constexpr (kShift == 0) {
    return a;
  }
  else if constexpr (kShift == 7) {
    return _mm256_cmpgt_epi8(simd_make_zero<__m256i>(), a);
  }
  else {
    __m256i tmp = simd_srli_u8<kShift>(a);
    __m256i sgn = simd_make256_u8(0x80u >> kShift);
    return _mm256_sub_epi8(simd_xor(tmp, sgn), sgn);
  }
}

template<uint8_t kN> BL_INLINE_NODEBUG __m256i simd_slli_i16(const __m256i& a) noexcept { return kN ? _mm256_slli_epi16(a, kN) : a; }
template<uint8_t kN> BL_INLINE_NODEBUG __m256i simd_slli_i32(const __m256i& a) noexcept { return kN ? _mm256_slli_epi32(a, kN) : a; }
template<uint8_t kN> BL_INLINE_NODEBUG __m256i simd_slli_i64(const __m256i& a) noexcept { return kN ? _mm256_slli_epi64(a, kN) : a; }

template<uint8_t kN> BL_INLINE_NODEBUG __m256i simd_srli_u16(const __m256i& a) noexcept { return kN ? _mm256_srli_epi16(a, kN) : a; }
template<uint8_t kN> BL_INLINE_NODEBUG __m256i simd_srli_u32(const __m256i& a) noexcept { return kN ? _mm256_srli_epi32(a, kN) : a; }
template<uint8_t kN> BL_INLINE_NODEBUG __m256i simd_srli_u64(const __m256i& a) noexcept { return kN ? _mm256_srli_epi64(a, kN) : a; }

template<uint8_t kN> BL_INLINE_NODEBUG __m256i simd_srai_i16(const __m256i& a) noexcept { return kN ? _mm256_srai_epi16(a, kN) : a; }
template<uint8_t kN> BL_INLINE_NODEBUG __m256i simd_srai_i32(const __m256i& a) noexcept { return kN ? _mm256_srai_epi32(a, kN) : a; }

#if defined(BL_TARGET_OPT_AVX512)
template<uint8_t kN> BL_INLINE_NODEBUG __m256i simd_srai_i64(const __m256i& a) noexcept { return kN ? _mm256_srai_epi64(a, kN) : a; }
#else
template<uint8_t kN> BL_INLINE_NODEBUG __m256i simd_srai_i64(const __m256i& a) noexcept {
  if constexpr (kN == 0) {
    return a;
  }
  else if constexpr (kN == 63u) {
    return _mm256_srai_epi32(_mm256_shuffle_epi32(a, _MM_SHUFFLE(3, 3, 1, 1)), 31);
  }
  else if constexpr (kN < 32u) {
    __m256i hi = _mm256_srai_epi32(a, kN & 31u);
    __m256i lo = _mm256_srli_epi64(a, kN & 31u);
    return _mm256_blend_epi16(lo, hi, 0xCCu);
  }
  else {
    __m256i highs = _mm256_shuffle_epi32(a, _MM_SHUFFLE(3, 3, 1, 1));
    __m256i signs = _mm256_srai_epi32(highs, 31);
    __m256i msk = _mm256_slli_epi64(signs, (64 - kN) & 63);
    return _mm256_or_si256(msk, _mm256_srli_epi64(a, kN));
  }
}
#endif

template<uint8_t kNumBytes> BL_INLINE_NODEBUG __m256i simd_sllb_u128(const __m256i& a) noexcept { return kNumBytes ? _mm256_slli_si256(a, kNumBytes) : a; }
template<uint8_t kNumBytes> BL_INLINE_NODEBUG __m256i simd_srlb_u128(const __m256i& a) noexcept { return kNumBytes ? _mm256_srli_si256(a, kNumBytes) : a; }

BL_INLINE_NODEBUG __m256i simd_sad_u8_u64(const __m256i& a, const __m256i& b) noexcept { return _mm256_sad_epu8(a, b); }
BL_INLINE_NODEBUG __m256i simd_maddws_u8xi8_i16(const __m256i& a, const __m256i& b) noexcept { return _mm256_maddubs_epi16(a, b); }

#endif // BL_TARGET_OPT_AVX2

#if defined(BL_TARGET_OPT_AVX512)
template<uint8_t kPred>
BL_INLINE_NODEBUG __m512i simd_ternlog(const __m512i& a, const __m512i& b, const __m512i& c) noexcept { return _mm512_ternarylogic_epi32(a, b, c, kPred); }

template<uint8_t kPred>
BL_INLINE_NODEBUG __m512 simd_ternlog(const __m512& a, const __m512& b, const __m512& c) noexcept { return simd_as_f(simd_ternlog<kPred>(simd_as_i(a), simd_as_i(b), simd_as_i(c))); }

template<uint8_t kPred>
BL_INLINE_NODEBUG __m512d simd_ternlog(const __m512d& a, const __m512d& b, const __m512d& c) noexcept { return simd_as_d(simd_ternlog<kPred>(simd_as_i(a), simd_as_i(b), simd_as_i(c))); }

BL_INLINE_NODEBUG __m512 simd_and(const __m512& a, const __m512& b) noexcept { return _mm512_and_ps(a, b); }
BL_INLINE_NODEBUG __m512d simd_and(const __m512d& a, const __m512d& b) noexcept { return _mm512_and_pd(a, b); }
BL_INLINE_NODEBUG __m512i simd_and(const __m512i& a, const __m512i& b) noexcept { return _mm512_and_si512(a, b); }

BL_INLINE_NODEBUG __m512 simd_andnot(const __m512& a, const __m512& b) noexcept { return _mm512_andnot_ps(a, b); }
BL_INLINE_NODEBUG __m512d simd_andnot(const __m512d& a, const __m512d& b) noexcept { return _mm512_andnot_pd(a, b); }
BL_INLINE_NODEBUG __m512i simd_andnot(const __m512i& a, const __m512i& b) noexcept { return _mm512_andnot_si512(a, b); }

BL_INLINE_NODEBUG __m512 simd_or(const __m512& a, const __m512& b) noexcept { return _mm512_or_ps(a, b); }
BL_INLINE_NODEBUG __m512d simd_or(const __m512d& a, const __m512d& b) noexcept { return _mm512_or_pd(a, b); }
BL_INLINE_NODEBUG __m512i simd_or(const __m512i& a, const __m512i& b) noexcept { return _mm512_or_si512(a, b); }

BL_INLINE_NODEBUG __m512 simd_xor(const __m512& a, const __m512& b) noexcept { return _mm512_xor_ps(a, b); }
BL_INLINE_NODEBUG __m512d simd_xor(const __m512d& a, const __m512d& b) noexcept { return _mm512_xor_pd(a, b); }
BL_INLINE_NODEBUG __m512i simd_xor(const __m512i& a, const __m512i& b) noexcept { return _mm512_xor_si512(a, b); }

BL_INLINE_NODEBUG __m512i simd_not(const __m512i& a) noexcept { return simd_ternlog<0x55>(a, a, a); }
BL_INLINE_NODEBUG __m512 simd_not(const __m512& a) noexcept { return simd_ternlog<0x55>(a, a, a); }
BL_INLINE_NODEBUG __m512d simd_not(const __m512d& a) noexcept { return simd_ternlog<0x55>(a, a, a); }

BL_INLINE_NODEBUG __m512 simd_blendv_bits(const __m512& a, const __m512& b, const __m512& msk) noexcept { return simd_ternlog<0xD8>(a, b, msk); }
BL_INLINE_NODEBUG __m512d simd_blendv_bits(const __m512d& a, const __m512d& b, const __m512d& msk) noexcept { return simd_ternlog<0xD8>(a, b, msk); }
BL_INLINE_NODEBUG __m512i simd_blendv_bits(const __m512i& a, const __m512i& b, const __m512i& msk) noexcept { return simd_ternlog<0xD8>(a, b, msk); }

BL_INLINE_NODEBUG __m512i simd_blendv_u8(const __m512i& a, const __m512i& b, const __m512i& msk) noexcept { return simd_blendv_bits(a, b, msk); }

BL_INLINE_NODEBUG __m512i simd_flip_sign_i8(const __m512i& a) noexcept { return simd_xor(a, simd_make512_u32(0x80808080u)); }
BL_INLINE_NODEBUG __m512i simd_flip_sign_i16(const __m512i& a) noexcept { return simd_xor(a, simd_make512_u32(0x80008000u)); }
BL_INLINE_NODEBUG __m512i simd_flip_sign_i32(const __m512i& a) noexcept { return simd_xor(a, simd_make512_u32(0x80000000u)); }
BL_INLINE_NODEBUG __m512i simd_flip_sign_i64(const __m512i& a) noexcept { return simd_xor(a, simd_make512_u64(uint64_t(1) << 63)); }

BL_INLINE_NODEBUG __m512 simd_add_f32(const __m512& a, const __m512& b) noexcept { return _mm512_add_ps(a, b); }
BL_INLINE_NODEBUG __m512d simd_add_f64(const __m512d& a, const __m512d& b) noexcept { return _mm512_add_pd(a, b); }

BL_INLINE_NODEBUG __m512 simd_sub_f32(const __m512& a, const __m512& b) noexcept { return _mm512_sub_ps(a, b); }
BL_INLINE_NODEBUG __m512d simd_sub_f64(const __m512d& a, const __m512d& b) noexcept { return _mm512_sub_pd(a, b); }

BL_INLINE_NODEBUG __m512 simd_mul_f32(const __m512& a, const __m512& b) noexcept { return _mm512_mul_ps(a, b); }
BL_INLINE_NODEBUG __m512d simd_mul_f64(const __m512d& a, const __m512d& b) noexcept { return _mm512_mul_pd(a, b); }

BL_INLINE_NODEBUG __m512 simd_div_f32(const __m512& a, const __m512& b) noexcept { return _mm512_div_ps(a, b); }
BL_INLINE_NODEBUG __m512d simd_div_f64(const __m512d& a, const __m512d& b) noexcept { return _mm512_div_pd(a, b); }

BL_INLINE_NODEBUG __m512 simd_min_f32(const __m512& a, const __m512& b) noexcept { return _mm512_min_ps(a, b); }
BL_INLINE_NODEBUG __m512d simd_min_f64(const __m512d& a, const __m512d& b) noexcept { return _mm512_min_pd(a, b); }

BL_INLINE_NODEBUG __m512 simd_max_f32(const __m512& a, const __m512& b) noexcept { return _mm512_max_ps(a, b); }
BL_INLINE_NODEBUG __m512d simd_max_f64(const __m512d& a, const __m512d& b) noexcept { return _mm512_max_pd(a, b); }

BL_INLINE_NODEBUG __m512 simd_cmp_eq_f32(const __m512& a, const __m512& b) noexcept { return simd_512f_from_mask32(_mm512_cmp_ps_mask(a, b, _CMP_EQ_OQ)); }
BL_INLINE_NODEBUG __m512d simd_cmp_eq_f64(const __m512d& a, const __m512d& b) noexcept { return simd_512d_from_mask64(_mm512_cmp_pd_mask(a, b, _CMP_EQ_OQ)); }

BL_INLINE_NODEBUG __m512 simd_cmp_ne_f32(const __m512& a, const __m512& b) noexcept { return simd_512f_from_mask32(_mm512_cmp_ps_mask(a, b, _CMP_NEQ_OQ)); }
BL_INLINE_NODEBUG __m512d simd_cmp_ne_f64(const __m512d& a, const __m512d& b) noexcept { return simd_512d_from_mask64(_mm512_cmp_pd_mask(a, b, _CMP_NEQ_OQ)); }

BL_INLINE_NODEBUG __m512 simd_cmp_lt_f32(const __m512& a, const __m512& b) noexcept { return simd_512f_from_mask32(_mm512_cmp_ps_mask(a, b, _CMP_LT_OQ)); }
BL_INLINE_NODEBUG __m512d simd_cmp_lt_f64(const __m512d& a, const __m512d& b) noexcept { return simd_512d_from_mask64(_mm512_cmp_pd_mask(a, b, _CMP_LT_OQ)); }

BL_INLINE_NODEBUG __m512 simd_cmp_le_f32(const __m512& a, const __m512& b) noexcept { return simd_512f_from_mask32(_mm512_cmp_ps_mask(a, b, _CMP_LE_OQ)); }
BL_INLINE_NODEBUG __m512d simd_cmp_le_f64(const __m512d& a, const __m512d& b) noexcept { return simd_512d_from_mask64(_mm512_cmp_pd_mask(a, b, _CMP_LE_OQ)); }

BL_INLINE_NODEBUG __m512 simd_cmp_gt_f32(const __m512& a, const __m512& b) noexcept { return simd_512f_from_mask32(_mm512_cmp_ps_mask(a, b, _CMP_GT_OQ)); }
BL_INLINE_NODEBUG __m512d simd_cmp_gt_f64(const __m512d& a, const __m512d& b) noexcept { return simd_512d_from_mask64(_mm512_cmp_pd_mask(a, b, _CMP_GT_OQ)); }

BL_INLINE_NODEBUG __m512 simd_cmp_ge_f32(const __m512& a, const __m512& b) noexcept { return simd_512f_from_mask32(_mm512_cmp_ps_mask(a, b, _CMP_GE_OQ)); }
BL_INLINE_NODEBUG __m512d simd_cmp_ge_f64(const __m512d& a, const __m512d& b) noexcept { return simd_512d_from_mask64(_mm512_cmp_pd_mask(a, b, _CMP_GE_OQ)); }

BL_INLINE_NODEBUG __m512 simd_abs_f32(const __m512& a) noexcept { return _mm512_and_ps(a, _mm512_broadcastss_ps(_mm_load_ss(&bl::common_table.p_7FFFFFFF7FFFFFFF.as<float>()))); }
BL_INLINE_NODEBUG __m512d simd_abs_f64(const __m512d& a) noexcept { return _mm512_and_pd(a, _mm512_broadcastsd_pd(_mm_load_sd(&bl::common_table.p_7FFFFFFFFFFFFFFF.as<double>()))); }

BL_INLINE_NODEBUG __m512 simd_sqrt_f32(const __m512& a) noexcept { return _mm512_sqrt_ps(a); }
BL_INLINE_NODEBUG __m512d simd_sqrt_f64(const __m512d& a) noexcept { return _mm512_sqrt_pd(a); }

BL_INLINE_NODEBUG __m512i simd_add_i8(const __m512i& a, const __m512i& b) noexcept { return _mm512_add_epi8(a, b); }
BL_INLINE_NODEBUG __m512i simd_add_i16(const __m512i& a, const __m512i& b) noexcept { return _mm512_add_epi16(a, b); }
BL_INLINE_NODEBUG __m512i simd_add_i32(const __m512i& a, const __m512i& b) noexcept { return _mm512_add_epi32(a, b); }
BL_INLINE_NODEBUG __m512i simd_add_i64(const __m512i& a, const __m512i& b) noexcept { return _mm512_add_epi64(a, b); }
BL_INLINE_NODEBUG __m512i simd_add_u8(const __m512i& a, const __m512i& b) noexcept { return _mm512_add_epi8(a, b); }
BL_INLINE_NODEBUG __m512i simd_add_u16(const __m512i& a, const __m512i& b) noexcept { return _mm512_add_epi16(a, b); }
BL_INLINE_NODEBUG __m512i simd_add_u32(const __m512i& a, const __m512i& b) noexcept { return _mm512_add_epi32(a, b); }
BL_INLINE_NODEBUG __m512i simd_add_u64(const __m512i& a, const __m512i& b) noexcept { return _mm512_add_epi64(a, b); }

BL_INLINE_NODEBUG __m512i simd_adds_i8(const __m512i& a, const __m512i& b) noexcept { return _mm512_adds_epi8(a, b); }
BL_INLINE_NODEBUG __m512i simd_adds_i16(const __m512i& a, const __m512i& b) noexcept { return _mm512_adds_epi16(a, b); }
BL_INLINE_NODEBUG __m512i simd_adds_u8(const __m512i& a, const __m512i& b) noexcept { return _mm512_adds_epu8(a, b); }
BL_INLINE_NODEBUG __m512i simd_adds_u16(const __m512i& a, const __m512i& b) noexcept { return _mm512_adds_epu16(a, b); }

BL_INLINE_NODEBUG __m512i simd_sub_i8(const __m512i& a, const __m512i& b) noexcept { return _mm512_sub_epi8(a, b); }
BL_INLINE_NODEBUG __m512i simd_sub_i16(const __m512i& a, const __m512i& b) noexcept { return _mm512_sub_epi16(a, b); }
BL_INLINE_NODEBUG __m512i simd_sub_i32(const __m512i& a, const __m512i& b) noexcept { return _mm512_sub_epi32(a, b); }
BL_INLINE_NODEBUG __m512i simd_sub_i64(const __m512i& a, const __m512i& b) noexcept { return _mm512_sub_epi64(a, b); }
BL_INLINE_NODEBUG __m512i simd_sub_u8(const __m512i& a, const __m512i& b) noexcept { return _mm512_sub_epi8(a, b); }
BL_INLINE_NODEBUG __m512i simd_sub_u16(const __m512i& a, const __m512i& b) noexcept { return _mm512_sub_epi16(a, b); }
BL_INLINE_NODEBUG __m512i simd_sub_u32(const __m512i& a, const __m512i& b) noexcept { return _mm512_sub_epi32(a, b); }
BL_INLINE_NODEBUG __m512i simd_sub_u64(const __m512i& a, const __m512i& b) noexcept { return _mm512_sub_epi64(a, b); }

BL_INLINE_NODEBUG __m512i simd_subs_i8(const __m512i& a, const __m512i& b) noexcept { return _mm512_subs_epi8(a, b); }
BL_INLINE_NODEBUG __m512i simd_subs_i16(const __m512i& a, const __m512i& b) noexcept { return _mm512_subs_epi16(a, b); }
BL_INLINE_NODEBUG __m512i simd_subs_u8(const __m512i& a, const __m512i& b) noexcept { return _mm512_subs_epu8(a, b); }
BL_INLINE_NODEBUG __m512i simd_subs_u16(const __m512i& a, const __m512i& b) noexcept { return _mm512_subs_epu16(a, b); }

BL_INLINE_NODEBUG __m512i simd_mul_i16(const __m512i& a, const __m512i& b) noexcept { return _mm512_mullo_epi16(a, b); }
BL_INLINE_NODEBUG __m512i simd_mul_u16(const __m512i& a, const __m512i& b) noexcept { return _mm512_mullo_epi16(a, b); }
BL_INLINE_NODEBUG __m512i simd_mul_i32(const __m512i& a, const __m512i& b) noexcept { return _mm512_mullo_epi32(a, b); }
BL_INLINE_NODEBUG __m512i simd_mul_u32(const __m512i& a, const __m512i& b) noexcept { return _mm512_mullo_epi32(a, b); }
BL_INLINE_NODEBUG __m512i simd_mul_i64(const __m512i& a, const __m512i& b) noexcept { return _mm512_mullo_epi64(a, b); }
BL_INLINE_NODEBUG __m512i simd_mul_u64(const __m512i& a, const __m512i& b) noexcept { return _mm512_mullo_epi64(a, b); }

BL_INLINE_NODEBUG __m512i simd_mulh_i16(const __m512i& a, const __m512i& b) noexcept { return _mm512_mulhi_epi16(a, b); }
BL_INLINE_NODEBUG __m512i simd_mulh_u16(const __m512i& a, const __m512i& b) noexcept { return _mm512_mulhi_epu16(a, b); }
BL_INLINE_NODEBUG __m512i simd_mulw_i32(const __m512i& a, const __m512i& b) noexcept { return _mm512_mul_epi32(a, b); }
BL_INLINE_NODEBUG __m512i simd_mulw_u32(const __m512i& a, const __m512i& b) noexcept { return _mm512_mul_epu32(a, b); }

BL_INLINE_NODEBUG __m512i simd_maddw_i16_i32(const __m512i& a, const __m512i& b) noexcept { return _mm512_madd_epi16(a, b); }

BL_INLINE_NODEBUG __m512i simd_cmp_eq_i8(const __m512i& a, const __m512i& b) noexcept { return simd_512i_from_mask8(_mm512_cmpeq_epi8_mask(a, b)); }
BL_INLINE_NODEBUG __m512i simd_cmp_eq_i16(const __m512i& a, const __m512i& b) noexcept { return simd_512i_from_mask16(_mm512_cmpeq_epi16_mask(a, b)); }
BL_INLINE_NODEBUG __m512i simd_cmp_eq_i32(const __m512i& a, const __m512i& b) noexcept { return simd_512i_from_mask32(_mm512_cmpeq_epi32_mask(a, b)); }
BL_INLINE_NODEBUG __m512i simd_cmp_eq_i64(const __m512i& a, const __m512i& b) noexcept { return simd_512i_from_mask64(_mm512_cmpeq_epi64_mask(a, b)); }

BL_INLINE_NODEBUG __m512i simd_cmp_ne_i8(const __m512i& a, const __m512i& b) noexcept { return simd_512i_from_mask8(_mm512_cmp_epi8_mask(a, b, _MM_CMPINT_NE)); }
BL_INLINE_NODEBUG __m512i simd_cmp_ne_i16(const __m512i& a, const __m512i& b) noexcept { return simd_512i_from_mask16(_mm512_cmp_epi16_mask(a, b, _MM_CMPINT_NE)); }
BL_INLINE_NODEBUG __m512i simd_cmp_ne_i32(const __m512i& a, const __m512i& b) noexcept { return simd_512i_from_mask32(_mm512_cmp_epi32_mask(a, b, _MM_CMPINT_NE)); }
BL_INLINE_NODEBUG __m512i simd_cmp_ne_i64(const __m512i& a, const __m512i& b) noexcept { return simd_512i_from_mask64(_mm512_cmp_epi64_mask(a, b, _MM_CMPINT_NE)); }

BL_INLINE_NODEBUG __m512i simd_cmp_gt_i8(const __m512i& a, const __m512i& b) noexcept { return simd_512i_from_mask8(_mm512_cmpgt_epi8_mask(a, b)); }
BL_INLINE_NODEBUG __m512i simd_cmp_gt_i16(const __m512i& a, const __m512i& b) noexcept { return simd_512i_from_mask16(_mm512_cmpgt_epi16_mask(a, b)); }
BL_INLINE_NODEBUG __m512i simd_cmp_gt_i32(const __m512i& a, const __m512i& b) noexcept { return simd_512i_from_mask32(_mm512_cmpgt_epi32_mask(a, b)); }
BL_INLINE_NODEBUG __m512i simd_cmp_gt_i64(const __m512i& a, const __m512i& b) noexcept { return simd_512i_from_mask64(_mm512_cmpgt_epi64_mask(a, b)); }

BL_INLINE_NODEBUG __m512i simd_cmp_lt_i8(const __m512i& a, const __m512i& b) noexcept { return simd_cmp_gt_i8(b, a); }
BL_INLINE_NODEBUG __m512i simd_cmp_lt_i16(const __m512i& a, const __m512i& b) noexcept { return simd_cmp_gt_i16(b, a); }
BL_INLINE_NODEBUG __m512i simd_cmp_lt_i32(const __m512i& a, const __m512i& b) noexcept { return simd_cmp_gt_i32(b, a); }
BL_INLINE_NODEBUG __m512i simd_cmp_lt_i64(const __m512i& a, const __m512i& b) noexcept { return simd_cmp_gt_i64(b, a); }

BL_INLINE_NODEBUG __m512i simd_cmp_ge_i8(const __m512i& a, const __m512i& b) noexcept { return simd_512i_from_mask8(_mm512_cmp_epi8_mask(a, b, _MM_CMPINT_NLT)); }
BL_INLINE_NODEBUG __m512i simd_cmp_ge_i16(const __m512i& a, const __m512i& b) noexcept { return simd_512i_from_mask16(_mm512_cmp_epi16_mask(a, b, _MM_CMPINT_NLT)); }
BL_INLINE_NODEBUG __m512i simd_cmp_ge_i32(const __m512i& a, const __m512i& b) noexcept { return simd_512i_from_mask32(_mm512_cmp_epi32_mask(a, b, _MM_CMPINT_NLT)); }
BL_INLINE_NODEBUG __m512i simd_cmp_ge_i64(const __m512i& a, const __m512i& b) noexcept { return simd_512i_from_mask64(_mm512_cmp_epi64_mask(a, b, _MM_CMPINT_NLT)); }

BL_INLINE_NODEBUG __m512i simd_cmp_le_i8(const __m512i& a, const __m512i& b) noexcept { return simd_512i_from_mask8(_mm512_cmp_epi8_mask(a, b, _MM_CMPINT_LE)); }
BL_INLINE_NODEBUG __m512i simd_cmp_le_i16(const __m512i& a, const __m512i& b) noexcept { return simd_512i_from_mask16(_mm512_cmp_epi16_mask(a, b, _MM_CMPINT_LE)); }
BL_INLINE_NODEBUG __m512i simd_cmp_le_i32(const __m512i& a, const __m512i& b) noexcept { return simd_512i_from_mask32(_mm512_cmp_epi32_mask(a, b, _MM_CMPINT_LE)); }
BL_INLINE_NODEBUG __m512i simd_cmp_le_i64(const __m512i& a, const __m512i& b) noexcept { return simd_512i_from_mask64(_mm512_cmp_epi64_mask(a, b, _MM_CMPINT_LE)); }

BL_INLINE_NODEBUG __m512i simd_cmp_gt_u8(const __m512i& a, const __m512i& b) noexcept { return simd_512i_from_mask8(_mm512_cmp_epu8_mask(a, b, _MM_CMPINT_NLE)); }
BL_INLINE_NODEBUG __m512i simd_cmp_gt_u16(const __m512i& a, const __m512i& b) noexcept { return simd_512i_from_mask16(_mm512_cmp_epu16_mask(a, b, _MM_CMPINT_NLE)); }
BL_INLINE_NODEBUG __m512i simd_cmp_gt_u32(const __m512i& a, const __m512i& b) noexcept { return simd_512i_from_mask32(_mm512_cmp_epu32_mask(a, b, _MM_CMPINT_NLE)); }
BL_INLINE_NODEBUG __m512i simd_cmp_gt_u64(const __m512i& a, const __m512i& b) noexcept { return simd_512i_from_mask64(_mm512_cmp_epu64_mask(a, b, _MM_CMPINT_NLE)); }

BL_INLINE_NODEBUG __m512i simd_cmp_ge_u8(const __m512i& a, const __m512i& b) noexcept { return simd_512i_from_mask8(_mm512_cmp_epu8_mask(a, b, _MM_CMPINT_NLT)); }
BL_INLINE_NODEBUG __m512i simd_cmp_ge_u16(const __m512i& a, const __m512i& b) noexcept { return simd_512i_from_mask16(_mm512_cmp_epu16_mask(a, b, _MM_CMPINT_NLT)); }
BL_INLINE_NODEBUG __m512i simd_cmp_ge_u32(const __m512i& a, const __m512i& b) noexcept { return simd_512i_from_mask32(_mm512_cmp_epu32_mask(a, b, _MM_CMPINT_NLT)); }
BL_INLINE_NODEBUG __m512i simd_cmp_ge_u64(const __m512i& a, const __m512i& b) noexcept { return simd_512i_from_mask64(_mm512_cmp_epu64_mask(a, b, _MM_CMPINT_NLT)); }

BL_INLINE_NODEBUG __m512i simd_cmp_lt_u8(const __m512i& a, const __m512i& b) noexcept { return simd_512i_from_mask8(_mm512_cmp_epu8_mask(a, b, _MM_CMPINT_LT)); }
BL_INLINE_NODEBUG __m512i simd_cmp_lt_u16(const __m512i& a, const __m512i& b) noexcept { return simd_512i_from_mask16(_mm512_cmp_epu16_mask(a, b, _MM_CMPINT_LT)); }
BL_INLINE_NODEBUG __m512i simd_cmp_lt_u32(const __m512i& a, const __m512i& b) noexcept { return simd_512i_from_mask32(_mm512_cmp_epu32_mask(a, b, _MM_CMPINT_LT)); }
BL_INLINE_NODEBUG __m512i simd_cmp_lt_u64(const __m512i& a, const __m512i& b) noexcept { return simd_512i_from_mask64(_mm512_cmp_epu64_mask(a, b, _MM_CMPINT_LT)); }

BL_INLINE_NODEBUG __m512i simd_cmp_le_u8(const __m512i& a, const __m512i& b) noexcept { return simd_512i_from_mask8(_mm512_cmp_epu8_mask(a, b, _MM_CMPINT_LE)); }
BL_INLINE_NODEBUG __m512i simd_cmp_le_u16(const __m512i& a, const __m512i& b) noexcept { return simd_512i_from_mask16(_mm512_cmp_epu16_mask(a, b, _MM_CMPINT_LE)); }
BL_INLINE_NODEBUG __m512i simd_cmp_le_u32(const __m512i& a, const __m512i& b) noexcept { return simd_512i_from_mask32(_mm512_cmp_epu32_mask(a, b, _MM_CMPINT_LE)); }
BL_INLINE_NODEBUG __m512i simd_cmp_le_u64(const __m512i& a, const __m512i& b) noexcept { return simd_512i_from_mask64(_mm512_cmp_epu64_mask(a, b, _MM_CMPINT_LE)); }

BL_INLINE_NODEBUG __m512i simd_min_i8(const __m512i& a, const __m512i& b) noexcept { return _mm512_min_epi8(a, b); }
BL_INLINE_NODEBUG __m512i simd_min_i16(const __m512i& a, const __m512i& b) noexcept { return _mm512_min_epi16(a, b); }
BL_INLINE_NODEBUG __m512i simd_min_i32(const __m512i& a, const __m512i& b) noexcept { return _mm512_min_epi32(a, b); }
BL_INLINE_NODEBUG __m512i simd_min_i64(const __m512i& a, const __m512i& b) noexcept { return _mm512_min_epi64(a, b); }
BL_INLINE_NODEBUG __m512i simd_min_u8(const __m512i& a, const __m512i& b) noexcept { return _mm512_min_epu8(a, b); }
BL_INLINE_NODEBUG __m512i simd_min_u16(const __m512i& a, const __m512i& b) noexcept { return _mm512_min_epu16(a, b); }
BL_INLINE_NODEBUG __m512i simd_min_u32(const __m512i& a, const __m512i& b) noexcept { return _mm512_min_epu32(a, b); }
BL_INLINE_NODEBUG __m512i simd_min_u64(const __m512i& a, const __m512i& b) noexcept { return _mm512_min_epu64(a, b); }

BL_INLINE_NODEBUG __m512i simd_max_i8(const __m512i& a, const __m512i& b) noexcept { return _mm512_max_epi8(a, b); }
BL_INLINE_NODEBUG __m512i simd_max_i16(const __m512i& a, const __m512i& b) noexcept { return _mm512_max_epi16(a, b); }
BL_INLINE_NODEBUG __m512i simd_max_i32(const __m512i& a, const __m512i& b) noexcept { return _mm512_max_epi32(a, b); }
BL_INLINE_NODEBUG __m512i simd_max_i64(const __m512i& a, const __m512i& b) noexcept { return _mm512_max_epi64(a, b); }
BL_INLINE_NODEBUG __m512i simd_max_u8(const __m512i& a, const __m512i& b) noexcept { return _mm512_max_epu8(a, b); }
BL_INLINE_NODEBUG __m512i simd_max_u16(const __m512i& a, const __m512i& b) noexcept { return _mm512_max_epu16(a, b); }
BL_INLINE_NODEBUG __m512i simd_max_u32(const __m512i& a, const __m512i& b) noexcept { return _mm512_max_epu32(a, b); }
BL_INLINE_NODEBUG __m512i simd_max_u64(const __m512i& a, const __m512i& b) noexcept { return _mm512_max_epu64(a, b); }

BL_INLINE_NODEBUG __m512i simd_abs_i8(const __m512i& a) noexcept { return _mm512_abs_epi8(a); }
BL_INLINE_NODEBUG __m512i simd_abs_i16(const __m512i& a) noexcept { return _mm512_abs_epi16(a); }
BL_INLINE_NODEBUG __m512i simd_abs_i32(const __m512i& a) noexcept { return _mm512_abs_epi32(a); }
BL_INLINE_NODEBUG __m512i simd_abs_i64(const __m512i& a) noexcept { return _mm512_abs_epi64(a); }

template<uint8_t kN> BL_INLINE_NODEBUG __m512i simd_slli_i8(const __m512i& a) noexcept {
  constexpr uint8_t kShift = uint8_t(kN & 0x7);

  if constexpr (kShift == 0u) {
    return a;
  }
  else if constexpr (kShift == 1) {
    return _mm512_add_epi8(a, a);
  }
  else {
    __m512i msk = _mm512_set1_epi8(int8_t((0xFFu << kShift) & 0xFFu));
    return _mm512_and_si512(_mm512_slli_epi16(a, kShift), msk);
  }
}

template<uint8_t kN> BL_INLINE_NODEBUG __m512i simd_srli_u8(const __m512i& a) noexcept {
  constexpr uint8_t kShift = uint8_t(kN & 0x7);

  if constexpr (kShift == 0) {
    return a;
  }
  else {
    __m512i msk = _mm512_set1_epi8(int8_t((0xFFu >> kShift) & 0xFFu));
    return _mm512_and_si512(_mm512_srli_epi16(a, kShift), msk);
  }
}

template<uint8_t kN> BL_INLINE_NODEBUG __m512i simd_srai_i8(const __m512i& a) noexcept {
  constexpr uint8_t kShift = uint8_t(kN & 0x7);

  if constexpr (kShift == 0) {
    return a;
  }
  else {
    __m512i tmp = simd_srli_u8<kShift>(a);
    __m512i sgn = simd_make512_u8(0x80u >> kShift);
    return _mm512_sub_epi8(simd_xor(tmp, sgn), sgn);
  }
}

template<uint8_t kN> BL_INLINE_NODEBUG __m512i simd_slli_i16(const __m512i& a) noexcept { return kN ? _mm512_slli_epi16(a, kN) : a; }
template<uint8_t kN> BL_INLINE_NODEBUG __m512i simd_slli_i32(const __m512i& a) noexcept { return kN ? _mm512_slli_epi32(a, kN) : a; }
template<uint8_t kN> BL_INLINE_NODEBUG __m512i simd_slli_i64(const __m512i& a) noexcept { return kN ? _mm512_slli_epi64(a, kN) : a; }

template<uint8_t kN> BL_INLINE_NODEBUG __m512i simd_srli_u16(const __m512i& a) noexcept { return kN ? _mm512_srli_epi16(a, kN) : a; }
template<uint8_t kN> BL_INLINE_NODEBUG __m512i simd_srli_u32(const __m512i& a) noexcept { return kN ? _mm512_srli_epi32(a, kN) : a; }
template<uint8_t kN> BL_INLINE_NODEBUG __m512i simd_srli_u64(const __m512i& a) noexcept { return kN ? _mm512_srli_epi64(a, kN) : a; }

template<uint8_t kN> BL_INLINE_NODEBUG __m512i simd_srai_i16(const __m512i& a) noexcept { return kN ? _mm512_srai_epi16(a, kN) : a; }
template<uint8_t kN> BL_INLINE_NODEBUG __m512i simd_srai_i32(const __m512i& a) noexcept { return kN ? _mm512_srai_epi32(a, kN) : a; }
template<uint8_t kN> BL_INLINE_NODEBUG __m512i simd_srai_i64(const __m512i& a) noexcept { return kN ? _mm512_srai_epi64(a, kN) : a; }

template<uint8_t kNumBytes> BL_INLINE_NODEBUG __m512i simd_sllb_u128(const __m512i& a) noexcept { return kNumBytes ? _mm512_bslli_epi128(a, kNumBytes) : a; }
template<uint8_t kNumBytes> BL_INLINE_NODEBUG __m512i simd_srlb_u128(const __m512i& a) noexcept { return kNumBytes ? _mm512_bsrli_epi128(a, kNumBytes) : a; }

BL_INLINE_NODEBUG __m512i simd_sad_u8_u64(const __m512i& a, const __m512i& b) noexcept { return _mm512_sad_epu8(a, b); }
BL_INLINE_NODEBUG __m512i simd_maddws_u8xi8_i16(const __m512i& a, const __m512i& b) noexcept { return _mm512_maddubs_epi16(a, b); }

#endif // BL_TARGET_OPT_AVX512

} // {Internal}

// SIMD - Internal - Extract MSB
// =============================

namespace Internal {

BL_INLINE_NODEBUG uint32_t simd_extract_sign_bits_i8(const __m128i& a) noexcept { return uint32_t(_mm_movemask_epi8(a)); }
BL_INLINE_NODEBUG uint32_t simd_extract_sign_bits_i32(const __m128i& a) noexcept { return uint32_t(_mm_movemask_ps(simd_cast<__m128>(a))); }
BL_INLINE_NODEBUG uint32_t simd_extract_sign_bits_i64(const __m128i& a) noexcept { return uint32_t(_mm_movemask_pd(simd_cast<__m128d>(a))); }

#if defined(BL_TARGET_OPT_AVX2)
BL_INLINE_NODEBUG uint32_t simd_extract_sign_bits_i8(const __m256i& a) noexcept { return uint32_t(_mm256_movemask_epi8(a)); }
BL_INLINE_NODEBUG uint32_t simd_extract_sign_bits_i32(const __m256i& a) noexcept { return uint32_t(_mm256_movemask_ps(simd_cast<__m256>(a))); }
BL_INLINE_NODEBUG uint32_t simd_extract_sign_bits_i64(const __m256i& a) noexcept { return uint32_t(_mm256_movemask_pd(simd_cast<__m256d>(a))); }
#endif // BL_TARGET_OPT_AVX2

#if defined(BL_TARGET_OPT_AVX512)
BL_INLINE_NODEBUG uint64_t simd_extract_sign_bits_i8(const __m512i& a) noexcept { return uint64_t(_cvtmask64_u64(_mm512_movepi8_mask(a))); }
BL_INLINE_NODEBUG uint32_t simd_extract_sign_bits_i32(const __m512i& a) noexcept { return uint32_t(_cvtmask16_u32(_mm512_movepi32_mask(a))); }
BL_INLINE_NODEBUG uint32_t simd_extract_sign_bits_i64(const __m512i& a) noexcept { return uint32_t(_cvtmask8_u32(_mm512_movepi64_mask(a))); }
#endif // BL_TARGET_OPT_AVX512

} // {Internal}

// SIMD - Internal - Load & Store Operations
// =========================================

namespace Internal {

template<typename SimdT> BL_INLINE_NODEBUG SimdT simd_load_8(const void* src) noexcept;
template<typename SimdT> BL_INLINE_NODEBUG SimdT simd_loada_16(const void* src) noexcept;
template<typename SimdT> BL_INLINE_NODEBUG SimdT simd_loadu_16(const void* src) noexcept;
template<typename SimdT> BL_INLINE_NODEBUG SimdT simd_loada_32(const void* src) noexcept;
template<typename SimdT> BL_INLINE_NODEBUG SimdT simd_loadu_32(const void* src) noexcept;
template<typename SimdT> BL_INLINE_NODEBUG SimdT simd_loada_64(const void* src) noexcept;
template<typename SimdT> BL_INLINE_NODEBUG SimdT simd_loadu_64(const void* src) noexcept;
template<typename SimdT> BL_INLINE_NODEBUG SimdT simd_loadl_64(SimdT dst, const void* src) noexcept;
template<typename SimdT> BL_INLINE_NODEBUG SimdT simd_loadh_64(SimdT dst, const void* src) noexcept;
template<typename SimdT> BL_INLINE_NODEBUG SimdT simd_loada_128(const void* src) noexcept;
template<typename SimdT> BL_INLINE_NODEBUG SimdT simd_loadu_128(const void* src) noexcept;

template<typename SimdT> BL_INLINE_NODEBUG SimdT simd_loada(const void* src) noexcept;
template<typename SimdT> BL_INLINE_NODEBUG SimdT simd_loadu(const void* src) noexcept;

template<size_t kW> BL_INLINE_NODEBUG typename SimdInfo<kW>::RegI simd_load_broadcast_u32(const void* src) noexcept;
template<size_t kW> BL_INLINE_NODEBUG typename SimdInfo<kW>::RegI simd_load_broadcast_u64(const void* src) noexcept;
template<size_t kW> BL_INLINE_NODEBUG typename SimdInfo<kW>::RegF simd_load_broadcast_f32(const void* src) noexcept;
template<size_t kW> BL_INLINE_NODEBUG typename SimdInfo<kW>::RegD simd_load_broadcast_f64(const void* src) noexcept;

template<size_t kW> BL_INLINE_NODEBUG typename SimdInfo<kW>::RegI simd_load_broadcast_4xi32(const void* src) noexcept;
template<size_t kW> BL_INLINE_NODEBUG typename SimdInfo<kW>::RegI simd_load_broadcast_2xi64(const void* src) noexcept;
template<size_t kW> BL_INLINE_NODEBUG typename SimdInfo<kW>::RegF simd_load_broadcast_f32x4(const void* src) noexcept;
template<size_t kW> BL_INLINE_NODEBUG typename SimdInfo<kW>::RegD simd_load_broadcast_f64x2(const void* src) noexcept;

template<> BL_INLINE_NODEBUG __m128i simd_loada(const void* src) noexcept { return _mm_load_si128(static_cast<const __m128i*>(src)); }
template<> BL_INLINE_NODEBUG __m128i simd_loadu(const void* src) noexcept { return _mm_loadu_si128(static_cast<const __m128i*>(src)); }
template<> BL_INLINE_NODEBUG __m128 simd_loada(const void* src) noexcept { return _mm_load_ps(static_cast<const float*>(src)); }
template<> BL_INLINE_NODEBUG __m128 simd_loadu(const void* src) noexcept { return _mm_loadu_ps(static_cast<const float*>(src)); }
template<> BL_INLINE_NODEBUG __m128d simd_loada(const void* src) noexcept { return _mm_load_pd(static_cast<const double*>(src)); }
template<> BL_INLINE_NODEBUG __m128d simd_loadu(const void* src) noexcept { return _mm_loadu_pd(static_cast<const double*>(src)); }

#if defined(BL_TARGET_OPT_SSE4_1)
template<> BL_INLINE_NODEBUG __m128i simd_load_8(const void* src) noexcept { return _mm_insert_epi8(_mm_setzero_si128(), *static_cast<const uint8_t*>(src), 0); }
#else
template<> BL_INLINE_NODEBUG __m128i simd_load_8(const void* src) noexcept { return _mm_cvtsi32_si128(int(unsigned(*static_cast<const uint8_t*>(src)))); }
#endif

template<> BL_INLINE_NODEBUG __m128i simd_loada_16(const void* src) noexcept { return _mm_cvtsi32_si128(*static_cast<const uint16_t*>(src)); }
template<> BL_INLINE_NODEBUG __m128i simd_loadu_16(const void* src) noexcept { return _mm_cvtsi32_si128(int(bl::MemOps::readU16u(src))); }
template<> BL_INLINE_NODEBUG __m128i simd_loada_32(const void* src) noexcept { return _mm_cvtsi32_si128(*static_cast<const int*>(src)); }
template<> BL_INLINE_NODEBUG __m128i simd_loadu_32(const void* src) noexcept { return _mm_cvtsi32_si128(int(bl::MemOps::readU32u(src))); }
template<> BL_INLINE_NODEBUG __m128i simd_loada_64(const void* src) noexcept { return _mm_loadl_epi64(static_cast<const __m128i*>(src)); }
template<> BL_INLINE_NODEBUG __m128i simd_loadu_64(const void* src) noexcept { return _mm_loadl_epi64(static_cast<const __m128i*>(src)); }
template<> BL_INLINE_NODEBUG __m128i simd_loada_128(const void* src) noexcept { return _mm_load_si128(static_cast<const __m128i*>(src)); }
template<> BL_INLINE_NODEBUG __m128i simd_loadu_128(const void* src) noexcept { return _mm_loadu_si128(static_cast<const __m128i*>(src)); }

template<> BL_INLINE_NODEBUG __m128 simd_loada_32(const void* src) noexcept { return _mm_load_ss(static_cast<const float*>(src)); }
template<> BL_INLINE_NODEBUG __m128 simd_loadu_32(const void* src) noexcept { return _mm_castsi128_ps(simd_loadu_32<__m128i>(src)); }
template<> BL_INLINE_NODEBUG __m128 simd_loada_64(const void* src) noexcept { return _mm_castsi128_ps(simd_loada_64<__m128i>(src)); }
template<> BL_INLINE_NODEBUG __m128 simd_loadu_64(const void* src) noexcept { return _mm_castsi128_ps(simd_loadu_64<__m128i>(src)); }
template<> BL_INLINE_NODEBUG __m128 simd_loada_128(const void* src) noexcept { return _mm_load_ps(static_cast<const float*>(src)); }
template<> BL_INLINE_NODEBUG __m128 simd_loadu_128(const void* src) noexcept { return _mm_loadu_ps(static_cast<const float*>(src)); }

template<> BL_INLINE_NODEBUG __m128d simd_loada_64(const void* src) noexcept { return _mm_load_sd(static_cast<const double*>(src)); }
template<> BL_INLINE_NODEBUG __m128d simd_loadu_64(const void* src) noexcept { return _mm_castsi128_pd(simd_loadu_64<__m128i>(src)); }
template<> BL_INLINE_NODEBUG __m128d simd_loada_128(const void* src) noexcept { return _mm_load_pd(static_cast<const double*>(src)); }
template<> BL_INLINE_NODEBUG __m128d simd_loadu_128(const void* src) noexcept { return _mm_loadu_pd(static_cast<const double*>(src)); }

template<> BL_INLINE_NODEBUG __m128i simd_loadl_64(__m128i dst, const void* src) noexcept { return _mm_castpd_si128(_mm_loadl_pd(_mm_castsi128_pd(dst), static_cast<const double*>(src))); }
template<> BL_INLINE_NODEBUG __m128i simd_loadh_64(__m128i dst, const void* src) noexcept { return _mm_castpd_si128(_mm_loadh_pd(_mm_castsi128_pd(dst), static_cast<const double*>(src))); }
template<> BL_INLINE_NODEBUG __m128 simd_loadl_64(__m128 dst, const void* src) noexcept { return _mm_loadl_pi(dst, static_cast<const __m64*>(src)); }
template<> BL_INLINE_NODEBUG __m128 simd_loadh_64(__m128 dst, const void* src) noexcept { return _mm_loadh_pi(dst, static_cast<const __m64*>(src)); }
template<> BL_INLINE_NODEBUG __m128d simd_loadl_64(__m128d dst, const void* src) noexcept { return _mm_loadl_pd(dst, static_cast<const double*>(src)); }
template<> BL_INLINE_NODEBUG __m128d simd_loadh_64(__m128d dst, const void* src) noexcept { return _mm_loadh_pd(dst, static_cast<const double*>(src)); }

// MSCV won't emit a single instruction if load_XX() is used to load from memory first.
#if defined(BL_TARGET_OPT_SSE4_1) && defined(_MSC_VER) && !defined(__clang__)
BL_INLINE_NODEBUG __m128i simd_loadu_64_i8_i16(const void* src) noexcept { return _mm_cvtepi8_epi16(*static_cast<const unaligned_m128i*>(src)); }
BL_INLINE_NODEBUG __m128i simd_loadu_64_u8_u16(const void* src) noexcept { return _mm_cvtepu8_epi16(*static_cast<const unaligned_m128i*>(src)); }
BL_INLINE_NODEBUG __m128i simd_loadu_32_i8_i32(const void* src) noexcept { return _mm_cvtepi8_epi32(*static_cast<const unaligned_m128i*>(src)); }
BL_INLINE_NODEBUG __m128i simd_loadu_32_u8_u32(const void* src) noexcept { return _mm_cvtepu8_epi32(*static_cast<const unaligned_m128i*>(src)); }
BL_INLINE_NODEBUG __m128i simd_loadu_16_i8_i64(const void* src) noexcept { return _mm_cvtepi8_epi64(*static_cast<const unaligned_m128i*>(src)); }
BL_INLINE_NODEBUG __m128i simd_loadu_16_u8_u64(const void* src) noexcept { return _mm_cvtepu8_epi64(*static_cast<const unaligned_m128i*>(src)); }
#elif defined(BL_TARGET_OPT_SSE4_1)
BL_INLINE_NODEBUG __m128i simd_loadu_64_i8_i16(const void* src) noexcept { return _mm_cvtepi8_epi16(simd_loadu_64<__m128i>(src)); }
BL_INLINE_NODEBUG __m128i simd_loadu_64_u8_u16(const void* src) noexcept { return _mm_cvtepu8_epi16(simd_loadu_64<__m128i>(src)); }
BL_INLINE_NODEBUG __m128i simd_loadu_32_i8_i32(const void* src) noexcept { return _mm_cvtepi8_epi32(simd_loadu_32<__m128i>(src)); }
BL_INLINE_NODEBUG __m128i simd_loadu_32_u8_u32(const void* src) noexcept { return _mm_cvtepu8_epi32(simd_loadu_32<__m128i>(src)); }
BL_INLINE_NODEBUG __m128i simd_loadu_16_i8_i64(const void* src) noexcept { return _mm_cvtepi8_epi64(simd_loadu_16<__m128i>(src)); }
BL_INLINE_NODEBUG __m128i simd_loadu_16_u8_u64(const void* src) noexcept { return _mm_cvtepu8_epi64(simd_loadu_16<__m128i>(src)); }
#else
BL_INLINE_NODEBUG __m128i simd_loadu_64_i8_i16(const void* src) noexcept { return simd_unpack_lo64_i8_i16(simd_loadu_64<__m128i>(src)); }
BL_INLINE_NODEBUG __m128i simd_loadu_64_u8_u16(const void* src) noexcept { return simd_unpack_lo64_u8_u16(simd_loadu_64<__m128i>(src)); }
BL_INLINE_NODEBUG __m128i simd_loadu_32_i8_i32(const void* src) noexcept { return simd_unpack_lo32_i8_i32(simd_loadu_32<__m128i>(src)); }
BL_INLINE_NODEBUG __m128i simd_loadu_32_u8_u32(const void* src) noexcept { return simd_unpack_lo32_u8_u32(simd_loadu_32<__m128i>(src)); }
#endif

BL_INLINE_NODEBUG __m128i simd_loada_64_i8_i16(const void* src) noexcept { return simd_loadu_64_i8_i16(src); }
BL_INLINE_NODEBUG __m128i simd_loada_64_u8_u16(const void* src) noexcept { return simd_loadu_64_u8_u16(src); }
BL_INLINE_NODEBUG __m128i simd_loada_32_i8_i32(const void* src) noexcept { return simd_loadu_32_i8_i32(src); }
BL_INLINE_NODEBUG __m128i simd_loada_32_u8_u32(const void* src) noexcept { return simd_loadu_32_u8_u32(src); }

#if defined(BL_TARGET_OPT_SSE4_1)
BL_INLINE_NODEBUG __m128i simd_loada_16_i8_i64(const void* src) noexcept { return simd_loadu_16_i8_i64(src); }
BL_INLINE_NODEBUG __m128i simd_loada_16_u8_u64(const void* src) noexcept { return simd_loadu_16_u8_u64(src); }
#endif

BL_INLINE_NODEBUG void simd_store_8(void* dst, __m128i src) noexcept { *static_cast<uint8_t*>(dst) = uint8_t(_mm_cvtsi128_si32(src)); }
BL_INLINE_NODEBUG void simd_storea_16(void* dst, __m128i src) noexcept { *static_cast<uint16_t*>(dst) = uint16_t(_mm_cvtsi128_si32(src)); }
BL_INLINE_NODEBUG void simd_storeu_16(void* dst, __m128i src) noexcept { bl::MemOps::writeU16u(dst, uint16_t(_mm_cvtsi128_si32(src))); }
BL_INLINE_NODEBUG void simd_storea_32(void* dst, __m128i src) noexcept { *static_cast<uint32_t*>(dst) = uint32_t(_mm_cvtsi128_si32(src)); }
BL_INLINE_NODEBUG void simd_storeu_32(void* dst, __m128i src) noexcept { bl::MemOps::writeU32u(dst, uint32_t(_mm_cvtsi128_si32(src))); }
BL_INLINE_NODEBUG void simd_storea_64(void* dst, __m128i src) noexcept { _mm_storel_epi64(static_cast<__m128i*>(dst), src); }
BL_INLINE_NODEBUG void simd_storeu_64(void* dst, __m128i src) noexcept { _mm_storel_epi64(static_cast<__m128i*>(dst), src); }
BL_INLINE_NODEBUG void simd_storeh_64(void* dst, __m128i src) noexcept { _mm_storeh_pd(static_cast<double*>(dst), _mm_castsi128_pd(src)); }
BL_INLINE_NODEBUG void simd_storea_128(void* dst, __m128i src) noexcept { _mm_store_si128(static_cast<__m128i*>(dst), src); }
BL_INLINE_NODEBUG void simd_storeu_128(void* dst, __m128i src) noexcept { _mm_storeu_si128(static_cast<__m128i*>(dst), src); }

BL_INLINE_NODEBUG void simd_storea_32(void* dst, __m128 src) noexcept { _mm_store_ss(static_cast<float*>(dst), src); }
BL_INLINE_NODEBUG void simd_storeu_32(void* dst, __m128 src) noexcept { _mm_store_ss(static_cast<float*>(dst), src); }
BL_INLINE_NODEBUG void simd_storea_64(void* dst, __m128 src) noexcept { _mm_storel_pi(static_cast<__m64*>(dst), src); }
BL_INLINE_NODEBUG void simd_storeu_64(void* dst, __m128 src) noexcept { _mm_storel_pi(static_cast<__m64*>(dst), src); }
BL_INLINE_NODEBUG void simd_storeh_64(void* dst, __m128 src) noexcept { _mm_storeh_pi(static_cast<__m64*>(dst), src); }
BL_INLINE_NODEBUG void simd_storea_128(void* dst, __m128 src) noexcept { _mm_store_ps(static_cast<float*>(dst), src); }
BL_INLINE_NODEBUG void simd_storeu_128(void* dst, __m128 src) noexcept { _mm_storeu_ps(static_cast<float*>(dst), src); }

BL_INLINE_NODEBUG void simd_storea_64(void* dst, __m128d src) noexcept { _mm_store_sd(static_cast<double*>(dst), src); }
BL_INLINE_NODEBUG void simd_storeu_64(void* dst, __m128d src) noexcept { _mm_store_sd(static_cast<double*>(dst), src); }
BL_INLINE_NODEBUG void simd_storeh_64(void* dst, __m128d src) noexcept { _mm_storeh_pd(static_cast<double*>(dst), src); }
BL_INLINE_NODEBUG void simd_storea_128(void* dst, __m128d src) noexcept { _mm_store_pd(static_cast<double*>(dst), src); }
BL_INLINE_NODEBUG void simd_storeu_128(void* dst, __m128d src) noexcept { _mm_storeu_pd(static_cast<double*>(dst), src); }

BL_INLINE_NODEBUG void simd_storea(void* dst, __m128i src) noexcept { _mm_store_si128(static_cast<__m128i*>(dst), src); }
BL_INLINE_NODEBUG void simd_storeu(void* dst, __m128i src) noexcept { _mm_storeu_si128(static_cast<__m128i*>(dst), src); }
BL_INLINE_NODEBUG void simd_storea(void* dst, __m128 src) noexcept { _mm_store_ps(static_cast<float*>(dst), src); }
BL_INLINE_NODEBUG void simd_storeu(void* dst, __m128 src) noexcept { _mm_storeu_ps(static_cast<float*>(dst), src); }
BL_INLINE_NODEBUG void simd_storea(void* dst, __m128d src) noexcept { _mm_store_pd(static_cast<double*>(dst), src); }
BL_INLINE_NODEBUG void simd_storeu(void* dst, __m128d src) noexcept { _mm_storeu_pd(static_cast<double*>(dst), src); }

#if defined(BL_TARGET_OPT_AVX)
template<typename SimdT> BL_INLINE_NODEBUG SimdT simd_loada_256(const void* src) noexcept;
template<typename SimdT> BL_INLINE_NODEBUG SimdT simd_loadu_256(const void* src) noexcept;

template<> BL_INLINE_NODEBUG __m256i simd_loada(const void* src) noexcept { return _mm256_load_si256(static_cast<const __m256i*>(src)); }
template<> BL_INLINE_NODEBUG __m256i simd_loadu(const void* src) noexcept { return _mm256_loadu_si256(static_cast<const __m256i*>(src)); }
template<> BL_INLINE_NODEBUG __m256 simd_loada(const void* src) noexcept { return _mm256_load_ps(static_cast<const float*>(src)); }
template<> BL_INLINE_NODEBUG __m256 simd_loadu(const void* src) noexcept { return _mm256_loadu_ps(static_cast<const float*>(src)); }
template<> BL_INLINE_NODEBUG __m256d simd_loada(const void* src) noexcept { return _mm256_load_pd(static_cast<const double*>(src)); }
template<> BL_INLINE_NODEBUG __m256d simd_loadu(const void* src) noexcept { return _mm256_loadu_pd(static_cast<const double*>(src)); }

template<> BL_INLINE_NODEBUG __m256i simd_load_8(const void* src) noexcept { return _mm256_castsi128_si256(simd_load_8<__m128i>(src)); }
template<> BL_INLINE_NODEBUG __m256i simd_loada_16(const void* src) noexcept { return _mm256_castsi128_si256(simd_loada_16<__m128i>(src)); }
template<> BL_INLINE_NODEBUG __m256i simd_loadu_16(const void* src) noexcept { return _mm256_castsi128_si256(simd_loadu_16<__m128i>(src)); }
template<> BL_INLINE_NODEBUG __m256i simd_loada_32(const void* src) noexcept { return _mm256_castsi128_si256(simd_loada_32<__m128i>(src)); }
template<> BL_INLINE_NODEBUG __m256i simd_loadu_32(const void* src) noexcept { return _mm256_castsi128_si256(simd_loadu_32<__m128i>(src)); }
template<> BL_INLINE_NODEBUG __m256i simd_loada_64(const void* src) noexcept { return _mm256_castsi128_si256(simd_loada_64<__m128i>(src)); }
template<> BL_INLINE_NODEBUG __m256i simd_loadu_64(const void* src) noexcept { return _mm256_castsi128_si256(simd_loadu_64<__m128i>(src)); }
template<> BL_INLINE_NODEBUG __m256i simd_loada_128(const void* src) noexcept { return _mm256_castsi128_si256(simd_loada_128<__m128i>(src)); }
template<> BL_INLINE_NODEBUG __m256i simd_loadu_128(const void* src) noexcept { return _mm256_castsi128_si256(simd_loadu_128<__m128i>(src)); }
template<> BL_INLINE_NODEBUG __m256i simd_loada_256(const void* src) noexcept { return _mm256_load_si256(static_cast<const __m256i*>(src)); }
template<> BL_INLINE_NODEBUG __m256i simd_loadu_256(const void* src) noexcept { return _mm256_loadu_si256(static_cast<const __m256i*>(src)); }

template<> BL_INLINE_NODEBUG __m256 simd_loada_32(const void* src) noexcept { return _mm256_castps128_ps256(simd_loada_32<__m128>(src)); }
template<> BL_INLINE_NODEBUG __m256 simd_loadu_32(const void* src) noexcept { return _mm256_castps128_ps256(simd_loadu_32<__m128>(src)); }
template<> BL_INLINE_NODEBUG __m256 simd_loada_64(const void* src) noexcept { return _mm256_castps128_ps256(simd_loada_64<__m128>(src)); }
template<> BL_INLINE_NODEBUG __m256 simd_loadu_64(const void* src) noexcept { return _mm256_castps128_ps256(simd_loadu_64<__m128>(src)); }
template<> BL_INLINE_NODEBUG __m256 simd_loada_128(const void* src) noexcept { return _mm256_castps128_ps256(simd_loada_128<__m128>(src)); }
template<> BL_INLINE_NODEBUG __m256 simd_loadu_128(const void* src) noexcept { return _mm256_castps128_ps256(simd_loadu_128<__m128>(src)); }
template<> BL_INLINE_NODEBUG __m256 simd_loada_256(const void* src) noexcept { return _mm256_load_ps(static_cast<const float*>(src)); }
template<> BL_INLINE_NODEBUG __m256 simd_loadu_256(const void* src) noexcept { return _mm256_loadu_ps(static_cast<const float*>(src)); }

template<> BL_INLINE_NODEBUG __m256d simd_loada_64(const void* src) noexcept { return _mm256_castpd128_pd256(simd_loada_64<__m128d>(src)); }
template<> BL_INLINE_NODEBUG __m256d simd_loadu_64(const void* src) noexcept { return _mm256_castpd128_pd256(simd_loadu_64<__m128d>(src)); }
template<> BL_INLINE_NODEBUG __m256d simd_loada_128(const void* src) noexcept { return _mm256_castpd128_pd256(simd_loada_128<__m128d>(src)); }
template<> BL_INLINE_NODEBUG __m256d simd_loadu_128(const void* src) noexcept { return _mm256_castpd128_pd256(simd_loadu_128<__m128d>(src)); }
template<> BL_INLINE_NODEBUG __m256d simd_loada_256(const void* src) noexcept { return _mm256_load_pd(static_cast<const double*>(src)); }
template<> BL_INLINE_NODEBUG __m256d simd_loadu_256(const void* src) noexcept { return _mm256_loadu_pd(static_cast<const double*>(src)); }

// MSCV won't emit a single instruction if load_XX() is used to load from memory first.
#if defined(_MSC_VER) && !defined(__clang__)
BL_INLINE_NODEBUG __m256i simd_loadu_64_i8_i32(const void* src) noexcept { return _mm256_cvtepi8_epi32(*static_cast<const unaligned_m128i*>(src)); }
BL_INLINE_NODEBUG __m256i simd_loadu_64_u8_u32(const void* src) noexcept { return _mm256_cvtepu8_epi32(*static_cast<const unaligned_m128i*>(src)); }
BL_INLINE_NODEBUG __m256i simd_loadu_32_i8_i64(const void* src) noexcept { return _mm256_cvtepi8_epi64(*static_cast<const unaligned_m128i*>(src)); }
BL_INLINE_NODEBUG __m256i simd_loadu_32_u8_u64(const void* src) noexcept { return _mm256_cvtepu8_epi64(*static_cast<const unaligned_m128i*>(src)); }
#else
BL_INLINE_NODEBUG __m256i simd_loadu_64_i8_i32(const void* src) noexcept { return _mm256_cvtepi8_epi32(simd_loadu_64<__m128i>(src)); }
BL_INLINE_NODEBUG __m256i simd_loadu_64_u8_u32(const void* src) noexcept { return _mm256_cvtepu8_epi32(simd_loadu_64<__m128i>(src)); }
BL_INLINE_NODEBUG __m256i simd_loadu_32_i8_i64(const void* src) noexcept { return _mm256_cvtepi8_epi64(simd_loadu_32<__m128i>(src)); }
BL_INLINE_NODEBUG __m256i simd_loadu_32_u8_u64(const void* src) noexcept { return _mm256_cvtepu8_epi64(simd_loadu_32<__m128i>(src)); }
#endif

BL_INLINE_NODEBUG __m256i simd_loada_64_i8_i32(const void* src) noexcept { return simd_loadu_64_i8_i32(src); }
BL_INLINE_NODEBUG __m256i simd_loada_64_u8_u32(const void* src) noexcept { return simd_loadu_64_u8_u32(src); }
BL_INLINE_NODEBUG __m256i simd_loada_32_i8_i64(const void* src) noexcept { return simd_loadu_32_i8_i64(src); }
BL_INLINE_NODEBUG __m256i simd_loada_32_u8_u64(const void* src) noexcept { return simd_loadu_32_u8_u64(src); }

BL_INLINE_NODEBUG __m256i simd_loada_128_i8_i16(const void* src) noexcept { return _mm256_cvtepi8_epi16(*static_cast<const __m128i*>(src)); }
BL_INLINE_NODEBUG __m256i simd_loadu_128_i8_i16(const void* src) noexcept { return _mm256_cvtepi8_epi16(*static_cast<const unaligned_m128i*>(src)); }
BL_INLINE_NODEBUG __m256i simd_loada_128_u8_u16(const void* src) noexcept { return _mm256_cvtepu8_epi16(*static_cast<const __m128i*>(src)); }
BL_INLINE_NODEBUG __m256i simd_loadu_128_u8_u16(const void* src) noexcept { return _mm256_cvtepu8_epi16(*static_cast<const unaligned_m128i*>(src)); }

BL_INLINE_NODEBUG __m128i simd_load_broadcast128_i32(const void* src) noexcept { return simd_cast<__m128i>(_mm_broadcast_ss(static_cast<const float*>(src))); }
BL_INLINE_NODEBUG __m128i simd_load_broadcast128_i64(const void* src) noexcept { return simd_cast<__m128i>(_mm_movedup_pd(simd_loadu_64<__m128d>(src))); }
BL_INLINE_NODEBUG __m128 simd_load_broadcast128_f32(const void* src) noexcept { return _mm_broadcast_ss(static_cast<const float*>(src)); }
BL_INLINE_NODEBUG __m128d simd_load_broadcast128_f64(const void* src) noexcept { return _mm_movedup_pd(simd_loadu_64<__m128d>(src)); }

BL_INLINE_NODEBUG __m256i simd_load_broadcast256_i32(const void* src) noexcept { return simd_cast<__m256i>(_mm256_broadcast_ss(static_cast<const float*>(src))); }
BL_INLINE_NODEBUG __m256i simd_load_broadcast256_i64(const void* src) noexcept { return simd_cast<__m256i>(_mm256_broadcast_sd(static_cast<const double*>(src))); }
BL_INLINE_NODEBUG __m256 simd_load_broadcast256_f32(const void* src) noexcept { return _mm256_broadcast_ss(static_cast<const float*>(src)); }
BL_INLINE_NODEBUG __m256d simd_load_broadcast256_f64(const void* src) noexcept { return _mm256_broadcast_sd(static_cast<const double*>(src)); }

BL_INLINE_NODEBUG __m256i simd_load_broadcast256_4xi32(const void* src) noexcept { return _mm256_broadcastsi128_si256(simd_loadu_128<__m128i>(src)); }
BL_INLINE_NODEBUG __m256i simd_load_broadcast256_2xi64(const void* src) noexcept { return _mm256_broadcastsi128_si256(simd_loadu_128<__m128i>(src)); }
BL_INLINE_NODEBUG __m256 simd_load_broadcast256_f32x4(const void* src) noexcept { return _mm256_broadcast_ps(static_cast<const __m128*>(src)); }
BL_INLINE_NODEBUG __m256d simd_load_broadcast256_f64x2(const void* src) noexcept { return _mm256_broadcast_pd(static_cast<const __m128d*>(src)); }

template<> BL_INLINE_NODEBUG __m128i simd_load_broadcast_u32<16>(const void* src) noexcept { return simd_load_broadcast128_i32(src); }
template<> BL_INLINE_NODEBUG __m128i simd_load_broadcast_u64<16>(const void* src) noexcept { return simd_load_broadcast128_i64(src); }
template<> BL_INLINE_NODEBUG __m128 simd_load_broadcast_f32<16>(const void* src) noexcept { return simd_load_broadcast128_f32(src); }
template<> BL_INLINE_NODEBUG __m128d simd_load_broadcast_f64<16>(const void* src) noexcept { return simd_load_broadcast128_f64(src); }

template<> BL_INLINE_NODEBUG __m256i simd_load_broadcast_u32<32>(const void* src) noexcept { return simd_load_broadcast256_i32(src); }
template<> BL_INLINE_NODEBUG __m256i simd_load_broadcast_u64<32>(const void* src) noexcept { return simd_load_broadcast256_i64(src); }
template<> BL_INLINE_NODEBUG __m256 simd_load_broadcast_f32<32>(const void* src) noexcept { return simd_load_broadcast256_f32(src); }
template<> BL_INLINE_NODEBUG __m256d simd_load_broadcast_f64<32>(const void* src) noexcept { return simd_load_broadcast256_f64(src); }

template<> BL_INLINE_NODEBUG __m256i simd_load_broadcast_4xi32<32>(const void* src) noexcept { return simd_load_broadcast256_4xi32(src); }
template<> BL_INLINE_NODEBUG __m256i simd_load_broadcast_2xi64<32>(const void* src) noexcept { return simd_load_broadcast256_2xi64(src); }
template<> BL_INLINE_NODEBUG __m256 simd_load_broadcast_f32x4<32>(const void* src) noexcept { return simd_load_broadcast256_f32x4(src); }
template<> BL_INLINE_NODEBUG __m256d simd_load_broadcast_f64x2<32>(const void* src) noexcept { return simd_load_broadcast256_f64x2(src); }

BL_INLINE_NODEBUG void simd_store_8(void* dst, __m256i src) noexcept { simd_store_8(dst, _mm256_castsi256_si128(src)); }
BL_INLINE_NODEBUG void simd_storea_16(void* dst, __m256i src) noexcept { simd_storea_16(dst, _mm256_castsi256_si128(src)); }
BL_INLINE_NODEBUG void simd_storeu_16(void* dst, __m256i src) noexcept { simd_storeu_16(dst, _mm256_castsi256_si128(src)); }
BL_INLINE_NODEBUG void simd_storea_32(void* dst, __m256i src) noexcept { simd_storea_32(dst, _mm256_castsi256_si128(src)); }
BL_INLINE_NODEBUG void simd_storeu_32(void* dst, __m256i src) noexcept { simd_storeu_32(dst, _mm256_castsi256_si128(src)); }
BL_INLINE_NODEBUG void simd_storea_64(void* dst, __m256i src) noexcept { simd_storea_64(dst, _mm256_castsi256_si128(src)); }
BL_INLINE_NODEBUG void simd_storeu_64(void* dst, __m256i src) noexcept { simd_storeu_64(dst, _mm256_castsi256_si128(src)); }
BL_INLINE_NODEBUG void simd_storeh_64(void* dst, __m256i src) noexcept { simd_storeh_64(dst, _mm256_castsi256_si128(src)); }
BL_INLINE_NODEBUG void simd_storea_128(void* dst, __m256i src) noexcept { simd_storea_128(dst, _mm256_castsi256_si128(src)); }
BL_INLINE_NODEBUG void simd_storeu_128(void* dst, __m256i src) noexcept { simd_storeu_128(dst, _mm256_castsi256_si128(src)); }
BL_INLINE_NODEBUG void simd_storea_256(void* dst, __m256i src) noexcept { _mm256_store_si256(static_cast<__m256i*>(dst), src); }
BL_INLINE_NODEBUG void simd_storeu_256(void* dst, __m256i src) noexcept { _mm256_storeu_si256(static_cast<__m256i*>(dst), src); }

BL_INLINE_NODEBUG void simd_storea_32(void* dst, __m256 src) noexcept { simd_storea_32(dst, _mm256_castps256_ps128(src)); }
BL_INLINE_NODEBUG void simd_storeu_32(void* dst, __m256 src) noexcept { simd_storeu_32(dst, _mm256_castps256_ps128(src)); }
BL_INLINE_NODEBUG void simd_storea_64(void* dst, __m256 src) noexcept { simd_storea_64(dst, _mm256_castps256_ps128(src)); }
BL_INLINE_NODEBUG void simd_storeu_64(void* dst, __m256 src) noexcept { simd_storeu_64(dst, _mm256_castps256_ps128(src)); }
BL_INLINE_NODEBUG void simd_storeh_64(void* dst, __m256 src) noexcept { simd_storeh_64(dst, _mm256_castps256_ps128(src)); }
BL_INLINE_NODEBUG void simd_storea_128(void* dst, __m256 src) noexcept { simd_storea_128(dst, _mm256_castps256_ps128(src)); }
BL_INLINE_NODEBUG void simd_storeu_128(void* dst, __m256 src) noexcept { simd_storeu_128(dst, _mm256_castps256_ps128(src)); }
BL_INLINE_NODEBUG void simd_storea_256(void* dst, __m256 src) noexcept { _mm256_store_ps(static_cast<float*>(dst), src); }
BL_INLINE_NODEBUG void simd_storeu_256(void* dst, __m256 src) noexcept { _mm256_storeu_ps(static_cast<float*>(dst), src); }

BL_INLINE_NODEBUG void simd_storea_64(void* dst, __m256d src) noexcept { simd_storea_64(dst, _mm256_castpd256_pd128(src)); }
BL_INLINE_NODEBUG void simd_storeu_64(void* dst, __m256d src) noexcept { simd_storeu_64(dst, _mm256_castpd256_pd128(src)); }
BL_INLINE_NODEBUG void simd_storeh_64(void* dst, __m256d src) noexcept { simd_storeh_64(dst, _mm256_castpd256_pd128(src)); }
BL_INLINE_NODEBUG void simd_storea_128(void* dst, __m256d src) noexcept { simd_storea_128(dst, _mm256_castpd256_pd128(src)); }
BL_INLINE_NODEBUG void simd_storeu_128(void* dst, __m256d src) noexcept { simd_storeu_128(dst, _mm256_castpd256_pd128(src)); }
BL_INLINE_NODEBUG void simd_storea_256(void* dst, __m256d src) noexcept { _mm256_store_pd(static_cast<double*>(dst), src); }
BL_INLINE_NODEBUG void simd_storeu_256(void* dst, __m256d src) noexcept { _mm256_storeu_pd(static_cast<double*>(dst), src); }

BL_INLINE_NODEBUG void simd_storea(void* dst, __m256i src) noexcept { _mm256_store_si256(static_cast<__m256i*>(dst), src); }
BL_INLINE_NODEBUG void simd_storeu(void* dst, __m256i src) noexcept { _mm256_storeu_si256(static_cast<__m256i*>(dst), src); }
BL_INLINE_NODEBUG void simd_storea(void* dst, __m256 src) noexcept { _mm256_store_ps(static_cast<float*>(dst), src); }
BL_INLINE_NODEBUG void simd_storeu(void* dst, __m256 src) noexcept { _mm256_storeu_ps(static_cast<float*>(dst), src); }
BL_INLINE_NODEBUG void simd_storea(void* dst, __m256d src) noexcept { _mm256_store_pd(static_cast<double*>(dst), src); }
BL_INLINE_NODEBUG void simd_storeu(void* dst, __m256d src) noexcept { _mm256_storeu_pd(static_cast<double*>(dst), src); }
#endif // BL_TARGET_OPT_AVX

#if defined(BL_TARGET_OPT_AVX512)
template<typename SimdT> BL_INLINE_NODEBUG SimdT simd_loada_512(const void* src) noexcept;
template<typename SimdT> BL_INLINE_NODEBUG SimdT simd_loadu_512(const void* src) noexcept;

template<> BL_INLINE_NODEBUG __m512i simd_loada(const void* src) noexcept { return _mm512_load_si512(src); }
template<> BL_INLINE_NODEBUG __m512i simd_loadu(const void* src) noexcept { return _mm512_loadu_si512(src); }
template<> BL_INLINE_NODEBUG __m512 simd_loada(const void* src) noexcept { return _mm512_load_ps(src); }
template<> BL_INLINE_NODEBUG __m512 simd_loadu(const void* src) noexcept { return _mm512_loadu_ps(src); }
template<> BL_INLINE_NODEBUG __m512d simd_loada(const void* src) noexcept { return _mm512_load_pd(src); }
template<> BL_INLINE_NODEBUG __m512d simd_loadu(const void* src) noexcept { return _mm512_loadu_pd(src); }

template<> BL_INLINE_NODEBUG __m512i simd_load_8(const void* src) noexcept { return _mm512_castsi128_si512(simd_load_8<__m128i>(src)); }
template<> BL_INLINE_NODEBUG __m512i simd_loada_16(const void* src) noexcept { return _mm512_castsi128_si512(simd_loada_16<__m128i>(src)); }
template<> BL_INLINE_NODEBUG __m512i simd_loadu_16(const void* src) noexcept { return _mm512_castsi128_si512(simd_loadu_16<__m128i>(src)); }
template<> BL_INLINE_NODEBUG __m512i simd_loada_32(const void* src) noexcept { return _mm512_castsi128_si512(simd_loada_32<__m128i>(src)); }
template<> BL_INLINE_NODEBUG __m512i simd_loadu_32(const void* src) noexcept { return _mm512_castsi128_si512(simd_loadu_32<__m128i>(src)); }
template<> BL_INLINE_NODEBUG __m512i simd_loada_64(const void* src) noexcept { return _mm512_castsi128_si512(simd_loada_64<__m128i>(src)); }
template<> BL_INLINE_NODEBUG __m512i simd_loadu_64(const void* src) noexcept { return _mm512_castsi128_si512(simd_loadu_64<__m128i>(src)); }
template<> BL_INLINE_NODEBUG __m512i simd_loada_128(const void* src) noexcept { return _mm512_castsi128_si512(simd_loada_128<__m128i>(src)); }
template<> BL_INLINE_NODEBUG __m512i simd_loadu_128(const void* src) noexcept { return _mm512_castsi128_si512(simd_loadu_128<__m128i>(src)); }
template<> BL_INLINE_NODEBUG __m512i simd_loada_256(const void* src) noexcept { return _mm512_castsi256_si512(simd_loada_256<__m256i>(src)); }
template<> BL_INLINE_NODEBUG __m512i simd_loadu_256(const void* src) noexcept { return _mm512_castsi256_si512(simd_loadu_256<__m256i>(src)); }
template<> BL_INLINE_NODEBUG __m512i simd_loada_512(const void* src) noexcept { return _mm512_load_si512(src); }
template<> BL_INLINE_NODEBUG __m512i simd_loadu_512(const void* src) noexcept { return _mm512_loadu_si512(src); }

template<> BL_INLINE_NODEBUG __m512 simd_loada_32(const void* src) noexcept { return _mm512_castps128_ps512(simd_loada_32<__m128>(src)); }
template<> BL_INLINE_NODEBUG __m512 simd_loadu_32(const void* src) noexcept { return _mm512_castps128_ps512(simd_loadu_32<__m128>(src)); }
template<> BL_INLINE_NODEBUG __m512 simd_loada_64(const void* src) noexcept { return _mm512_castps128_ps512(simd_loada_64<__m128>(src)); }
template<> BL_INLINE_NODEBUG __m512 simd_loadu_64(const void* src) noexcept { return _mm512_castps128_ps512(simd_loadu_64<__m128>(src)); }
template<> BL_INLINE_NODEBUG __m512 simd_loada_128(const void* src) noexcept { return _mm512_castps128_ps512(simd_loada_128<__m128>(src)); }
template<> BL_INLINE_NODEBUG __m512 simd_loadu_128(const void* src) noexcept { return _mm512_castps128_ps512(simd_loadu_128<__m128>(src)); }
template<> BL_INLINE_NODEBUG __m512 simd_loada_256(const void* src) noexcept { return _mm512_castps256_ps512(simd_loada_256<__m256>(src)); }
template<> BL_INLINE_NODEBUG __m512 simd_loadu_256(const void* src) noexcept { return _mm512_castps256_ps512(simd_loadu_256<__m256>(src)); }
template<> BL_INLINE_NODEBUG __m512 simd_loada_512(const void* src) noexcept { return _mm512_load_ps(src); }
template<> BL_INLINE_NODEBUG __m512 simd_loadu_512(const void* src) noexcept { return _mm512_loadu_ps(src); }

template<> BL_INLINE_NODEBUG __m512d simd_loada_64(const void* src) noexcept { return _mm512_castpd128_pd512(simd_loada_64<__m128d>(src)); }
template<> BL_INLINE_NODEBUG __m512d simd_loadu_64(const void* src) noexcept { return _mm512_castpd128_pd512(simd_loadu_64<__m128d>(src)); }
template<> BL_INLINE_NODEBUG __m512d simd_loada_128(const void* src) noexcept { return _mm512_castpd128_pd512(simd_loada_128<__m128d>(src)); }
template<> BL_INLINE_NODEBUG __m512d simd_loadu_128(const void* src) noexcept { return _mm512_castpd128_pd512(simd_loadu_128<__m128d>(src)); }
template<> BL_INLINE_NODEBUG __m512d simd_loada_256(const void* src) noexcept { return _mm512_castpd256_pd512(simd_loada_256<__m256d>(src)); }
template<> BL_INLINE_NODEBUG __m512d simd_loadu_256(const void* src) noexcept { return _mm512_castpd256_pd512(simd_loadu_256<__m256d>(src)); }
template<> BL_INLINE_NODEBUG __m512d simd_loada_512(const void* src) noexcept { return _mm512_load_pd(src); }
template<> BL_INLINE_NODEBUG __m512d simd_loadu_512(const void* src) noexcept { return _mm512_loadu_pd(src); }

// MSCV won't emit a single instruction if load_XX() is used to load from memory first.
#if defined(_MSC_VER) && !defined(__clang__)
BL_INLINE_NODEBUG __m512i simd_loada_128_i8_i32(const void* src) noexcept { return _mm512_cvtepi8_epi32(*static_cast<const __m128i*>(src)); }
BL_INLINE_NODEBUG __m512i simd_loada_128_u8_u32(const void* src) noexcept { return _mm512_cvtepu8_epi32(*static_cast<const __m128i*>(src)); }
BL_INLINE_NODEBUG __m512i simd_loadu_128_i8_i32(const void* src) noexcept { return _mm512_cvtepi8_epi32(*static_cast<const unaligned_m128i*>(src)); }
BL_INLINE_NODEBUG __m512i simd_loadu_128_u8_u32(const void* src) noexcept { return _mm512_cvtepu8_epi32(*static_cast<const unaligned_m128i*>(src)); }
BL_INLINE_NODEBUG __m512i simd_loadu_64_i8_i64(const void* src) noexcept { return _mm512_cvtepi8_epi64(*static_cast<const unaligned_m128i*>(src)); }
BL_INLINE_NODEBUG __m512i simd_loadu_64_u8_u64(const void* src) noexcept { return _mm512_cvtepu8_epi64(*static_cast<const unaligned_m128i*>(src)); }
#else
BL_INLINE_NODEBUG __m512i simd_loada_128_i8_i32(const void* src) noexcept { return _mm512_cvtepi8_epi32(*static_cast<const __m128i*>(src)); }
BL_INLINE_NODEBUG __m512i simd_loada_128_u8_u32(const void* src) noexcept { return _mm512_cvtepu8_epi32(*static_cast<const __m128i*>(src)); }
BL_INLINE_NODEBUG __m512i simd_loadu_128_i8_i32(const void* src) noexcept { return _mm512_cvtepi8_epi32(simd_loadu_128<__m128i>(src)); }
BL_INLINE_NODEBUG __m512i simd_loadu_128_u8_u32(const void* src) noexcept { return _mm512_cvtepu8_epi32(simd_loadu_128<__m128i>(src)); }
BL_INLINE_NODEBUG __m512i simd_loadu_64_i8_i64(const void* src) noexcept { return _mm512_cvtepi8_epi64(simd_loadu_64<__m128i>(src)); }
BL_INLINE_NODEBUG __m512i simd_loadu_64_u8_u64(const void* src) noexcept { return _mm512_cvtepu8_epi64(simd_loadu_64<__m128i>(src)); }
#endif

BL_INLINE_NODEBUG __m512i simd_loada_64_i8_i64(const void* src) noexcept { return simd_loadu_64_i8_i64(src); }
BL_INLINE_NODEBUG __m512i simd_loada_64_u8_u64(const void* src) noexcept { return simd_loadu_64_u8_u64(src); }

BL_INLINE_NODEBUG __m512i simd_loada_256_i8_i16(const void* src) noexcept { return _mm512_cvtepi8_epi16(*static_cast<const __m256i*>(src)); }
BL_INLINE_NODEBUG __m512i simd_loadu_256_i8_i16(const void* src) noexcept { return _mm512_cvtepi8_epi16(*static_cast<const unaligned_m256i*>(src)); }
BL_INLINE_NODEBUG __m512i simd_loada_256_u8_u16(const void* src) noexcept { return _mm512_cvtepu8_epi16(*static_cast<const __m256i*>(src)); }
BL_INLINE_NODEBUG __m512i simd_loadu_256_u8_u16(const void* src) noexcept { return _mm512_cvtepu8_epi16(*static_cast<const unaligned_m256i*>(src)); }

BL_INLINE_NODEBUG __m512i simd_load_broadcast512_i32(const void* src) noexcept { return _mm512_broadcastd_epi32(simd_loadu_32<__m128i>(src)); }
BL_INLINE_NODEBUG __m512i simd_load_broadcast512_i64(const void* src) noexcept { return _mm512_broadcastq_epi64(simd_loadu_64<__m128i>(src)); }
BL_INLINE_NODEBUG __m512 simd_load_broadcast512_f32(const void* src) noexcept { return _mm512_broadcastss_ps(simd_loadu_32<__m128>(src)); }
BL_INLINE_NODEBUG __m512d simd_load_broadcast512_f64(const void* src) noexcept { return _mm512_broadcastsd_pd(simd_loadu_64<__m128d>(src)); }

BL_INLINE_NODEBUG __m512i simd_load_broadcast512_4xi32(const void* src) noexcept { return _mm512_broadcast_i32x4(simd_loadu_128<__m128i>(src)); }
BL_INLINE_NODEBUG __m512i simd_load_broadcast512_2xi64(const void* src) noexcept { return _mm512_broadcast_i64x2(simd_loadu_128<__m128i>(src)); }
BL_INLINE_NODEBUG __m512 simd_load_broadcast512_f32x4(const void* src) noexcept { return _mm512_broadcast_f32x4(simd_loadu_128<__m128>(src)); }
BL_INLINE_NODEBUG __m512d simd_load_broadcast512_f64x2(const void* src) noexcept { return _mm512_broadcast_f64x2(simd_loadu_128<__m128d>(src)); }

template<> BL_INLINE_NODEBUG __m512i simd_load_broadcast_u32<64>(const void* src) noexcept { return simd_load_broadcast512_i32(src); }
template<> BL_INLINE_NODEBUG __m512i simd_load_broadcast_u64<64>(const void* src) noexcept { return simd_load_broadcast512_i64(src); }
template<> BL_INLINE_NODEBUG __m512 simd_load_broadcast_f32<64>(const void* src) noexcept { return simd_load_broadcast512_f32(src); }
template<> BL_INLINE_NODEBUG __m512d simd_load_broadcast_f64<64>(const void* src) noexcept { return simd_load_broadcast512_f64(src); }

template<> BL_INLINE_NODEBUG __m512i simd_load_broadcast_4xi32<64>(const void* src) noexcept { return simd_load_broadcast512_4xi32(src); }
template<> BL_INLINE_NODEBUG __m512i simd_load_broadcast_2xi64<64>(const void* src) noexcept { return simd_load_broadcast512_2xi64(src); }
template<> BL_INLINE_NODEBUG __m512 simd_load_broadcast_f32x4<64>(const void* src) noexcept { return simd_load_broadcast512_f32x4(src); }
template<> BL_INLINE_NODEBUG __m512d simd_load_broadcast_f64x2<64>(const void* src) noexcept { return simd_load_broadcast512_f64x2(src); }

BL_INLINE_NODEBUG void simd_store_8(void* dst, __m512i src) noexcept { simd_store_8(dst, _mm512_castsi512_si128(src)); }
BL_INLINE_NODEBUG void simd_storea_16(void* dst, __m512i src) noexcept { simd_storea_16(dst, _mm512_castsi512_si128(src)); }
BL_INLINE_NODEBUG void simd_storeu_16(void* dst, __m512i src) noexcept { simd_storeu_16(dst, _mm512_castsi512_si128(src)); }
BL_INLINE_NODEBUG void simd_storea_32(void* dst, __m512i src) noexcept { simd_storea_32(dst, _mm512_castsi512_si128(src)); }
BL_INLINE_NODEBUG void simd_storeu_32(void* dst, __m512i src) noexcept { simd_storeu_32(dst, _mm512_castsi512_si128(src)); }
BL_INLINE_NODEBUG void simd_storea_64(void* dst, __m512i src) noexcept { simd_storea_64(dst, _mm512_castsi512_si128(src)); }
BL_INLINE_NODEBUG void simd_storeu_64(void* dst, __m512i src) noexcept { simd_storeu_64(dst, _mm512_castsi512_si128(src)); }
BL_INLINE_NODEBUG void simd_storeh_64(void* dst, __m512i src) noexcept { simd_storeh_64(dst, _mm512_castsi512_si128(src)); }
BL_INLINE_NODEBUG void simd_storea_128(void* dst, __m512i src) noexcept { simd_storea_128(dst, _mm512_castsi512_si128(src)); }
BL_INLINE_NODEBUG void simd_storeu_128(void* dst, __m512i src) noexcept { simd_storeu_128(dst, _mm512_castsi512_si128(src)); }
BL_INLINE_NODEBUG void simd_storea_256(void* dst, __m512i src) noexcept { simd_storea_256(dst, _mm512_castsi512_si256(src)); }
BL_INLINE_NODEBUG void simd_storeu_256(void* dst, __m512i src) noexcept { simd_storeu_256(dst, _mm512_castsi512_si256(src)); }
BL_INLINE_NODEBUG void simd_storea_512(void* dst, __m512i src) noexcept { _mm512_store_si512(static_cast<__m512i*>(dst), src); }
BL_INLINE_NODEBUG void simd_storeu_512(void* dst, __m512i src) noexcept { _mm512_storeu_si512(static_cast<__m512i*>(dst), src); }

BL_INLINE_NODEBUG void simd_storea_32(void* dst, __m512 src) noexcept { simd_storea_32(dst, _mm512_castps512_ps128(src)); }
BL_INLINE_NODEBUG void simd_storeu_32(void* dst, __m512 src) noexcept { simd_storeu_32(dst, _mm512_castps512_ps128(src)); }
BL_INLINE_NODEBUG void simd_storea_64(void* dst, __m512 src) noexcept { simd_storea_64(dst, _mm512_castps512_ps128(src)); }
BL_INLINE_NODEBUG void simd_storeu_64(void* dst, __m512 src) noexcept { simd_storeu_64(dst, _mm512_castps512_ps128(src)); }
BL_INLINE_NODEBUG void simd_storeh_64(void* dst, __m512 src) noexcept { simd_storeh_64(dst, _mm512_castps512_ps128(src)); }
BL_INLINE_NODEBUG void simd_storea_128(void* dst, __m512 src) noexcept { simd_storea_128(dst, _mm512_castps512_ps128(src)); }
BL_INLINE_NODEBUG void simd_storeu_128(void* dst, __m512 src) noexcept { simd_storeu_128(dst, _mm512_castps512_ps128(src)); }
BL_INLINE_NODEBUG void simd_storea_256(void* dst, __m512 src) noexcept { simd_storea_256(dst, _mm512_castps512_ps256(src)); }
BL_INLINE_NODEBUG void simd_storeu_256(void* dst, __m512 src) noexcept { simd_storeu_256(dst, _mm512_castps512_ps256(src)); }
BL_INLINE_NODEBUG void simd_storea_512(void* dst, __m512 src) noexcept { _mm512_store_ps(static_cast<float*>(dst), src); }
BL_INLINE_NODEBUG void simd_storeu_512(void* dst, __m512 src) noexcept { _mm512_storeu_ps(static_cast<float*>(dst), src); }

BL_INLINE_NODEBUG void simd_storea_64(void* dst, __m512d src) noexcept { simd_storea_64(dst, _mm512_castpd512_pd128(src)); }
BL_INLINE_NODEBUG void simd_storeu_64(void* dst, __m512d src) noexcept { simd_storeu_64(dst, _mm512_castpd512_pd128(src)); }
BL_INLINE_NODEBUG void simd_storeh_64(void* dst, __m512d src) noexcept { simd_storeh_64(dst, _mm512_castpd512_pd128(src)); }
BL_INLINE_NODEBUG void simd_storea_128(void* dst, __m512d src) noexcept { simd_storea_128(dst, _mm512_castpd512_pd128(src)); }
BL_INLINE_NODEBUG void simd_storeu_128(void* dst, __m512d src) noexcept { simd_storeu_128(dst, _mm512_castpd512_pd128(src)); }
BL_INLINE_NODEBUG void simd_storea_256(void* dst, __m512d src) noexcept { simd_storea_256(dst, _mm512_castpd512_pd256(src)); }
BL_INLINE_NODEBUG void simd_storeu_256(void* dst, __m512d src) noexcept { simd_storeu_256(dst, _mm512_castpd512_pd256(src)); }
BL_INLINE_NODEBUG void simd_storea_512(void* dst, __m512d src) noexcept { _mm512_store_pd(static_cast<double*>(dst), src); }
BL_INLINE_NODEBUG void simd_storeu_512(void* dst, __m512d src) noexcept { _mm512_storeu_pd(static_cast<double*>(dst), src); }

BL_INLINE_NODEBUG void simd_storea(void* dst, __m512i src) noexcept { _mm512_store_si512(static_cast<__m512i*>(dst), src); }
BL_INLINE_NODEBUG void simd_storeu(void* dst, __m512i src) noexcept { _mm512_storeu_si512(static_cast<__m512i*>(dst), src); }
BL_INLINE_NODEBUG void simd_storea(void* dst, __m512 src) noexcept { _mm512_store_ps(static_cast<float*>(dst), src); }
BL_INLINE_NODEBUG void simd_storeu(void* dst, __m512 src) noexcept { _mm512_storeu_ps(static_cast<float*>(dst), src); }
BL_INLINE_NODEBUG void simd_storea(void* dst, __m512d src) noexcept { _mm512_store_pd(static_cast<double*>(dst), src); }
BL_INLINE_NODEBUG void simd_storeu(void* dst, __m512d src) noexcept { _mm512_storeu_pd(static_cast<double*>(dst), src); }
#endif // BL_TARGET_OPT_AVX512

} // {Internal}

// SIMD - Public - Make Zero & Ones & Undefined
// ============================================

template<typename V>
BL_INLINE_NODEBUG V make_zero() noexcept { return V{I::simd_make_zero<typename V::SimdType>()}; }

template<typename V>
BL_INLINE_NODEBUG V make_ones() noexcept { return V{I::simd_make_ones<typename V::SimdType>()}; }

template<typename V>
BL_INLINE_NODEBUG V make_undefined() noexcept { return V{I::simd_make_undefined<typename V::SimdType>()}; }

// SIMD - Public - Make Vector (Any)
// =================================

template<typename V, typename... Args> BL_INLINE_NODEBUG V make_i8(Args&&... args) noexcept { return V{I::simd_make_i8<typename V::SimdType>(BLInternal::forward<Args>(args)...)}; }
template<typename V, typename... Args> BL_INLINE_NODEBUG V make_i16(Args&&... args) noexcept { return V{I::simd_make_i16<typename V::SimdType>(BLInternal::forward<Args>(args)...)}; }
template<typename V, typename... Args> BL_INLINE_NODEBUG V make_i32(Args&&... args) noexcept { return V{I::simd_make_i32<typename V::SimdType>(BLInternal::forward<Args>(args)...)}; }
template<typename V, typename... Args> BL_INLINE_NODEBUG V make_i64(Args&&... args) noexcept { return V{I::simd_make_i64<typename V::SimdType>(BLInternal::forward<Args>(args)...)}; }
template<typename V, typename... Args> BL_INLINE_NODEBUG V make_u8(Args&&... args) noexcept { return V{I::simd_make_u8<typename V::SimdType>(BLInternal::forward<Args>(args)...)}; }
template<typename V, typename... Args> BL_INLINE_NODEBUG V make_u16(Args&&... args) noexcept { return V{I::simd_make_u16<typename V::SimdType>(BLInternal::forward<Args>(args)...)}; }
template<typename V, typename... Args> BL_INLINE_NODEBUG V make_u32(Args&&... args) noexcept { return V{I::simd_make_u32<typename V::SimdType>(BLInternal::forward<Args>(args)...)}; }
template<typename V, typename... Args> BL_INLINE_NODEBUG V make_u64(Args&&... args) noexcept { return V{I::simd_make_u64<typename V::SimdType>(BLInternal::forward<Args>(args)...)}; }
template<typename V, typename... Args> BL_INLINE_NODEBUG V make_f32(Args&&... args) noexcept { return V{I::simd_make_f32<typename V::SimdType>(BLInternal::forward<Args>(args)...)}; }
template<typename V, typename... Args> BL_INLINE_NODEBUG V make_f64(Args&&... args) noexcept { return V{I::simd_make_f64<typename V::SimdType>(BLInternal::forward<Args>(args)...)}; }

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

template<typename V = Vec2xF64>
BL_INLINE_NODEBUG V make128_f64(double x0) noexcept {
  return from_simd<V>(I::simd_make128_f64(x0));
}

template<typename V = Vec2xF64>
BL_INLINE_NODEBUG V make128_f64(double x1, double x0) noexcept {
  return from_simd<V>(I::simd_make128_f64(x1, x0));
}

// SIMD - Public - Make Vector (256-bit)
// =====================================

#ifdef BL_TARGET_OPT_AVX
template<typename V = Vec32xI8>
BL_INLINE_NODEBUG V make256_i8(int8_t x0) noexcept {
  return from_simd<V>(I::simd_make256_u8(uint8_t(x0)));
}

template<typename V = Vec32xI8>
BL_INLINE_NODEBUG V make256_i8(int8_t x1, int8_t x0) noexcept {
  return from_simd<V>(I::simd_make256_u8(uint8_t(x1), uint8_t(x0)));
}

template<typename V = Vec32xI8>
BL_INLINE_NODEBUG V make256_i8(int8_t x3, int8_t x2, int8_t x1, int8_t x0) noexcept {
  return from_simd<V>(I::simd_make256_u8(uint8_t(x3), uint8_t(x2), uint8_t(x1), uint8_t(x0)));
}

template<typename V = Vec32xI8>
BL_INLINE_NODEBUG V make256_i8(int8_t x7, int8_t x6, int8_t x5, int8_t x4,
                               int8_t x3, int8_t x2, int8_t x1, int8_t x0) noexcept {
  return from_simd<V>(
    I::simd_make256_u8(
      uint8_t(x7), uint8_t(x6), uint8_t(x5), uint8_t(x4),
      uint8_t(x3), uint8_t(x2), uint8_t(x1), uint8_t(x0)));
}

template<typename V = Vec32xI8>
BL_INLINE_NODEBUG V make256_i8(int8_t x15, int8_t x14, int8_t x13, int8_t x12,
                               int8_t x11, int8_t x10, int8_t x09, int8_t x08,
                               int8_t x07, int8_t x06, int8_t x05, int8_t x04,
                               int8_t x03, int8_t x02, int8_t x01, int8_t x00) noexcept {
  return from_simd<V>(
    I::simd_make256_u8(
      uint8_t(x15), uint8_t(x14), uint8_t(x13), uint8_t(x12),
      uint8_t(x11), uint8_t(x10), uint8_t(x09), uint8_t(x08),
      uint8_t(x07), uint8_t(x06), uint8_t(x05), uint8_t(x04),
      uint8_t(x03), uint8_t(x02), uint8_t(x01), uint8_t(x00)));
}

template<typename V = Vec32xI8>
BL_INLINE_NODEBUG V make256_i8(int8_t x31, int8_t x30, int8_t x29, int8_t x28,
                               int8_t x27, int8_t x26, int8_t x25, int8_t x24,
                               int8_t x23, int8_t x22, int8_t x21, int8_t x20,
                               int8_t x19, int8_t x18, int8_t x17, int8_t x16,
                               int8_t x15, int8_t x14, int8_t x13, int8_t x12,
                               int8_t x11, int8_t x10, int8_t x09, int8_t x08,
                               int8_t x07, int8_t x06, int8_t x05, int8_t x04,
                               int8_t x03, int8_t x02, int8_t x01, int8_t x00) noexcept {
  return from_simd<V>(
    I::simd_make256_u8(
      uint8_t(x31), uint8_t(x30), uint8_t(x29), uint8_t(x28),
      uint8_t(x27), uint8_t(x26), uint8_t(x25), uint8_t(x24),
      uint8_t(x23), uint8_t(x22), uint8_t(x21), uint8_t(x20),
      uint8_t(x19), uint8_t(x18), uint8_t(x17), uint8_t(x16),
      uint8_t(x15), uint8_t(x14), uint8_t(x13), uint8_t(x12),
      uint8_t(x11), uint8_t(x10), uint8_t(x09), uint8_t(x08),
      uint8_t(x07), uint8_t(x06), uint8_t(x05), uint8_t(x04),
      uint8_t(x03), uint8_t(x02), uint8_t(x01), uint8_t(x00)));
}

template<typename V = Vec32xU8>
BL_INLINE_NODEBUG V make256_u8(uint8_t x0) noexcept {
  return from_simd<V>(I::simd_make256_u8(x0));
}

template<typename V = Vec32xU8>
BL_INLINE_NODEBUG V make256_u8(uint8_t x1, uint8_t x0) noexcept {
  return from_simd<V>(I::simd_make256_u8(x1, x0));
}

template<typename V = Vec32xU8>
BL_INLINE_NODEBUG V make256_u8(uint8_t x3, uint8_t x2, uint8_t x1, uint8_t x0) noexcept {
  return from_simd<V>(I::simd_make256_u8(x3, x2, x1, x0));
}

template<typename V = Vec32xU8>
BL_INLINE_NODEBUG V make256_u8(uint8_t x7, uint8_t x6, uint8_t x5, uint8_t x4,
                               uint8_t x3, uint8_t x2, uint8_t x1, uint8_t x0) noexcept {
  return from_simd<V>(
    I::simd_make256_u8(
      x7, x6, x5, x4,
      x3, x2, x1, x0));
}

template<typename V = Vec32xU8>
BL_INLINE_NODEBUG V make256_u8(uint8_t x15, uint8_t x14, uint8_t x13, uint8_t x12,
                               uint8_t x11, uint8_t x10, uint8_t x09, uint8_t x08,
                               uint8_t x07, uint8_t x06, uint8_t x05, uint8_t x04,
                               uint8_t x03, uint8_t x02, uint8_t x01, uint8_t x00) noexcept {
  return from_simd<V>(
    I::simd_make256_u8(
      x15, x14, x13, x12,
      x11, x10, x09, x08,
      x07, x06, x05, x04,
      x03, x02, x01, x00));
}

template<typename V = Vec32xU8>
BL_INLINE_NODEBUG V make256_u8(uint8_t x31, uint8_t x30, uint8_t x29, uint8_t x28,
                               uint8_t x27, uint8_t x26, uint8_t x25, uint8_t x24,
                               uint8_t x23, uint8_t x22, uint8_t x21, uint8_t x20,
                               uint8_t x19, uint8_t x18, uint8_t x17, uint8_t x16,
                               uint8_t x15, uint8_t x14, uint8_t x13, uint8_t x12,
                               uint8_t x11, uint8_t x10, uint8_t x09, uint8_t x08,
                               uint8_t x07, uint8_t x06, uint8_t x05, uint8_t x04,
                               uint8_t x03, uint8_t x02, uint8_t x01, uint8_t x00) noexcept {
  return from_simd<V>(
    I::simd_make256_u8(
      x31, x30, x29, x28,
      x27, x26, x25, x24,
      x23, x22, x21, x20,
      x19, x18, x17, x16,
      x15, x14, x13, x12,
      x11, x10, x09, x08,
      x07, x06, x05, x04,
      x03, x02, x01, x00));
}

template<typename V = Vec16xI16>
BL_INLINE_NODEBUG V make256_i16(int16_t x0) noexcept {
  return from_simd<V>(I::simd_make256_u16(uint16_t(x0)));
}

template<typename V = Vec16xI16>
BL_INLINE_NODEBUG V make256_i16(int16_t x1, int16_t x0) noexcept {
  return from_simd<V>(I::simd_make256_u16(uint16_t(x1), uint16_t(x0)));
}

template<typename V = Vec16xI16>
BL_INLINE_NODEBUG V make256_i16(int16_t x3, int16_t x2, int16_t x1, int16_t x0) noexcept {
  return from_simd<V>(I::simd_make256_u16(uint16_t(x3), uint16_t(x2), uint16_t(x1), uint16_t(x0)));
}

template<typename V = Vec16xI16>
BL_INLINE_NODEBUG V make256_i16(int16_t x7, int16_t x6, int16_t x5, int16_t x4,
                                int16_t x3, int16_t x2, int16_t x1, int16_t x0) noexcept {
  return from_simd<V>(
    I::simd_make256_u16(
      uint16_t(x7), uint16_t(x6), uint16_t(x5), uint16_t(x4),
      uint16_t(x3), uint16_t(x2), uint16_t(x1), uint16_t(x0)));
}

template<typename V = Vec16xI16>
BL_INLINE_NODEBUG V make256_i16(int16_t x15, int16_t x14, int16_t x13, int16_t x12,
                                int16_t x11, int16_t x10, int16_t x09, int16_t x08,
                                int16_t x07, int16_t x06, int16_t x05, int16_t x04,
                                int16_t x03, int16_t x02, int16_t x01, int16_t x00) noexcept {
  return from_simd<V>(
    I::simd_make256_u16(
      uint16_t(x15), uint16_t(x14), uint16_t(x13), uint16_t(x12),
      uint16_t(x11), uint16_t(x10), uint16_t(x09), uint16_t(x08),
      uint16_t(x07), uint16_t(x06), uint16_t(x05), uint16_t(x04),
      uint16_t(x03), uint16_t(x02), uint16_t(x01), uint16_t(x00)));
}

template<typename V = Vec16xU16>
BL_INLINE_NODEBUG V make256_u16(uint16_t x0) noexcept {
  return from_simd<V>(I::simd_make256_u16(x0));
}

template<typename V = Vec16xU16>
BL_INLINE_NODEBUG V make256_u16(uint16_t x1, uint16_t x0) noexcept {
  return from_simd<V>(I::simd_make256_u16(x1, x0));
}

template<typename V = Vec16xU16>
BL_INLINE_NODEBUG V make256_u16(uint16_t x3, uint16_t x2, uint16_t x1, uint16_t x0) noexcept {
  return from_simd<V>(I::simd_make256_u16(x3, x2, x1, x0));
}

template<typename V = Vec16xU16>
BL_INLINE_NODEBUG V make256_u16(uint16_t x7, uint16_t x6, uint16_t x5, uint16_t x4,
                                uint16_t x3, uint16_t x2, uint16_t x1, uint16_t x0) noexcept {
  return from_simd<V>(
    I::simd_make256_u16(
      x7, x6, x5, x4,
      x3, x2, x1, x0));
}

template<typename V = Vec16xU16>
BL_INLINE_NODEBUG V make256_u16(uint16_t x15, uint16_t x14, uint16_t x13, uint16_t x12,
                                uint16_t x11, uint16_t x10, uint16_t x09, uint16_t x08,
                                uint16_t x07, uint16_t x06, uint16_t x05, uint16_t x04,
                                uint16_t x03, uint16_t x02, uint16_t x01, uint16_t x00) noexcept {
  return from_simd<V>(
    I::simd_make256_u16(
      x15, x14, x13, x12,
      x11, x10, x09, x08,
      x07, x06, x05, x04,
      x03, x02, x01, x00));
}

template<typename V = Vec8xI32>
BL_INLINE_NODEBUG V make256_i32(int32_t x0) noexcept {
  return from_simd<V>(I::simd_make256_u32(uint32_t(x0)));
}

template<typename V = Vec8xI32>
BL_INLINE_NODEBUG V make256_i32(int32_t x1, int32_t x0) noexcept {
  return from_simd<V>(I::simd_make256_u32(uint32_t(x1), uint32_t(x0)));
}

template<typename V = Vec8xI32>
BL_INLINE_NODEBUG V make256_i32(int32_t x3, int32_t x2, int32_t x1, int32_t x0) noexcept {
  return from_simd<V>(I::simd_make256_u32(uint32_t(x3), uint32_t(x2), uint32_t(x1), uint32_t(x0)));
}

template<typename V = Vec8xI32>
BL_INLINE_NODEBUG V make256_i32(int32_t x7, int32_t x6, int32_t x5, int32_t x4,
                                int32_t x3, int32_t x2, int32_t x1, int32_t x0) noexcept {
  return from_simd<V>(
    I::simd_make256_u32(
      uint32_t(x7), uint32_t(x6), uint32_t(x5), uint32_t(x4),
      uint32_t(x3), uint32_t(x2), uint32_t(x1), uint32_t(x0)));
}

template<typename V = Vec8xU32>
BL_INLINE_NODEBUG V make256_u32(uint32_t x0) noexcept {
  return from_simd<V>(I::simd_make256_u32(x0));
}

template<typename V = Vec8xU32>
BL_INLINE_NODEBUG V make256_u32(uint32_t x1, uint32_t x0) noexcept {
  return from_simd<V>(I::simd_make256_u32(x1, x0));
}

template<typename V = Vec8xU32>
BL_INLINE_NODEBUG V make256_u32(uint32_t x3, uint32_t x2, uint32_t x1, uint32_t x0) noexcept {
  return from_simd<V>(I::simd_make256_u32(x3, x2, x1, x0));
}

template<typename V = Vec8xU32>
BL_INLINE_NODEBUG V make256_u32(uint32_t x7, uint32_t x6, uint32_t x5, uint32_t x4,
                                uint32_t x3, uint32_t x2, uint32_t x1, uint32_t x0) noexcept {
  return from_simd<V>(I::simd_make256_u32(x7, x6, x5, x4, x3, x2, x1, x0));
}

template<typename V = Vec4xI64>
BL_INLINE_NODEBUG V make256_i64(int64_t x0) noexcept {
  return from_simd<V>(I::simd_make256_u64(uint64_t(x0)));
}

template<typename V = Vec4xI64>
BL_INLINE_NODEBUG V make256_i64(int64_t x1, int64_t x0) noexcept {
  return from_simd<V>(I::simd_make256_u64(uint64_t(x1), uint64_t(x0)));
}

template<typename V = Vec4xI64>
BL_INLINE_NODEBUG V make256_i64(int64_t x3, int64_t x2, int64_t x1, int64_t x0) noexcept {
  return from_simd<V>(I::simd_make256_u64(uint64_t(x3), uint64_t(x2), uint64_t(x1), uint64_t(x0)));
}

template<typename V = Vec4xU64>
BL_INLINE_NODEBUG V make256_u64(uint64_t x0) noexcept {
  return from_simd<V>(I::simd_make256_u64(x0));
}

template<typename V = Vec4xU64>
BL_INLINE_NODEBUG V make256_u64(uint64_t x1, uint64_t x0) noexcept {
  return from_simd<V>(I::simd_make256_u64(x1, x0));
}

template<typename V = Vec4xU64>
BL_INLINE_NODEBUG V make256_u64(uint64_t x3, uint64_t x2, uint64_t x1, uint64_t x0) noexcept {
  return from_simd<V>(I::simd_make256_u64(x3, x2, x1, x0));
}

template<typename V = Vec8xF32>
BL_INLINE_NODEBUG V make256_f32(float x0) noexcept {
  return from_simd<V>(I::simd_make256_f32(x0));
}

template<typename V = Vec8xF32>
BL_INLINE_NODEBUG V make256_f32(float x1, float x0) noexcept {
  return from_simd<V>(I::simd_make256_f32(x1, x0));
}

template<typename V = Vec8xF32>
BL_INLINE_NODEBUG V make256_f32(float x3, float x2, float x1, float x0) noexcept {
  return from_simd<V>(I::simd_make256_f32(x3, x2, x1, x0));
}

template<typename V = Vec8xF32>
BL_INLINE_NODEBUG V make256_f32(float x7, float x6, float x5, float x4,
                                      float x3, float x2, float x1, float x0) noexcept {
  return from_simd<V>(I::simd_make256_f32(x7, x6, x5, x4, x3, x2, x1, x0));
}

template<typename V = Vec4xF64>
BL_INLINE_NODEBUG V make256_f64(double x0) noexcept {
  return from_simd<V>(I::simd_make256_f64(x0));
}

template<typename V = Vec4xF64>
BL_INLINE_NODEBUG V make256_f64(double x1, double x0) noexcept {
  return from_simd<V>(I::simd_make256_f64(x1, x0));
}

template<typename V = Vec4xF64>
BL_INLINE_NODEBUG V make256_f64(double x3, double x2, double x1, double x0) noexcept {
  return from_simd<V>(I::simd_make256_f64(x3, x2, x1, x0));
}
#endif // BL_TARGET_OPT_AVX

// SIMD - Public - Make Vector (512-bit)
// =====================================

#ifdef BL_TARGET_OPT_AVX512
template<typename V = Vec64xI8>
BL_INLINE_NODEBUG V make512_i8(int8_t x0) noexcept {
  return from_simd<V>(I::simd_make512_u8(uint8_t(x0)));
}

template<typename V = Vec64xI8>
BL_INLINE_NODEBUG V make512_i8(int8_t x1, int8_t x0) noexcept {
  return from_simd<V>(I::simd_make512_u8(uint8_t(x1), uint8_t(x0)));
}

template<typename V = Vec64xI8>
BL_INLINE_NODEBUG V make512_i8(int8_t x3, int8_t x2, int8_t x1, int8_t x0) noexcept {
  return from_simd<V>(I::simd_make512_u8(uint8_t(x3), uint8_t(x2), uint8_t(x1), uint8_t(x0)));
}

template<typename V = Vec64xI8>
BL_INLINE_NODEBUG V make512_i8(int8_t x7, int8_t x6, int8_t x5, int8_t x4,
                               int8_t x3, int8_t x2, int8_t x1, int8_t x0) noexcept {
  return from_simd<V>(
    I::simd_make512_u8(
      uint8_t(x7), uint8_t(x6), uint8_t(x5), uint8_t(x4),
      uint8_t(x3), uint8_t(x2), uint8_t(x1), uint8_t(x0)));
}

template<typename V = Vec64xI8>
BL_INLINE_NODEBUG V make512_i8(int8_t x15, int8_t x14, int8_t x13, int8_t x12,
                               int8_t x11, int8_t x10, int8_t x09, int8_t x08,
                               int8_t x07, int8_t x06, int8_t x05, int8_t x04,
                               int8_t x03, int8_t x02, int8_t x01, int8_t x00) noexcept {
  return from_simd<V>(
    I::simd_make512_u8(
      uint8_t(x15), uint8_t(x14), uint8_t(x13), uint8_t(x12),
      uint8_t(x11), uint8_t(x10), uint8_t(x09), uint8_t(x08),
      uint8_t(x07), uint8_t(x06), uint8_t(x05), uint8_t(x04),
      uint8_t(x03), uint8_t(x02), uint8_t(x01), uint8_t(x00)));
}

template<typename V = Vec64xI8>
BL_INLINE_NODEBUG V make512_i8(int8_t x31, int8_t x30, int8_t x29, int8_t x28,
                               int8_t x27, int8_t x26, int8_t x25, int8_t x24,
                               int8_t x23, int8_t x22, int8_t x21, int8_t x20,
                               int8_t x19, int8_t x18, int8_t x17, int8_t x16,
                               int8_t x15, int8_t x14, int8_t x13, int8_t x12,
                               int8_t x11, int8_t x10, int8_t x09, int8_t x08,
                               int8_t x07, int8_t x06, int8_t x05, int8_t x04,
                               int8_t x03, int8_t x02, int8_t x01, int8_t x00) noexcept {
  return from_simd<V>(
    I::simd_make512_u8(
      uint8_t(x31), uint8_t(x30), uint8_t(x29), uint8_t(x28),
      uint8_t(x27), uint8_t(x26), uint8_t(x25), uint8_t(x24),
      uint8_t(x23), uint8_t(x22), uint8_t(x21), uint8_t(x20),
      uint8_t(x19), uint8_t(x18), uint8_t(x17), uint8_t(x16),
      uint8_t(x15), uint8_t(x14), uint8_t(x13), uint8_t(x12),
      uint8_t(x11), uint8_t(x10), uint8_t(x09), uint8_t(x08),
      uint8_t(x07), uint8_t(x06), uint8_t(x05), uint8_t(x04),
      uint8_t(x03), uint8_t(x02), uint8_t(x01), uint8_t(x00)));
}

template<typename V = Vec64xI8>
BL_INLINE_NODEBUG V make512_i8(int8_t x63, int8_t x62, int8_t x61, int8_t x60,
                               int8_t x59, int8_t x58, int8_t x57, int8_t x56,
                               int8_t x55, int8_t x54, int8_t x53, int8_t x52,
                               int8_t x51, int8_t x50, int8_t x49, int8_t x48,
                               int8_t x47, int8_t x46, int8_t x45, int8_t x44,
                               int8_t x43, int8_t x42, int8_t x41, int8_t x40,
                               int8_t x39, int8_t x38, int8_t x37, int8_t x36,
                               int8_t x35, int8_t x34, int8_t x33, int8_t x32,
                               int8_t x31, int8_t x30, int8_t x29, int8_t x28,
                               int8_t x27, int8_t x26, int8_t x25, int8_t x24,
                               int8_t x23, int8_t x22, int8_t x21, int8_t x20,
                               int8_t x19, int8_t x18, int8_t x17, int8_t x16,
                               int8_t x15, int8_t x14, int8_t x13, int8_t x12,
                               int8_t x11, int8_t x10, int8_t x09, int8_t x08,
                               int8_t x07, int8_t x06, int8_t x05, int8_t x04,
                               int8_t x03, int8_t x02, int8_t x01, int8_t x00) noexcept {
  return from_simd<V>(
    I::simd_make512_u8(
      uint8_t(x63), uint8_t(x62), uint8_t(x61), uint8_t(x60),
      uint8_t(x59), uint8_t(x58), uint8_t(x57), uint8_t(x56),
      uint8_t(x55), uint8_t(x54), uint8_t(x53), uint8_t(x52),
      uint8_t(x51), uint8_t(x50), uint8_t(x49), uint8_t(x48),
      uint8_t(x47), uint8_t(x46), uint8_t(x45), uint8_t(x44),
      uint8_t(x43), uint8_t(x42), uint8_t(x41), uint8_t(x40),
      uint8_t(x39), uint8_t(x38), uint8_t(x37), uint8_t(x36),
      uint8_t(x35), uint8_t(x34), uint8_t(x33), uint8_t(x32),
      uint8_t(x31), uint8_t(x30), uint8_t(x29), uint8_t(x28),
      uint8_t(x27), uint8_t(x26), uint8_t(x25), uint8_t(x24),
      uint8_t(x23), uint8_t(x22), uint8_t(x21), uint8_t(x20),
      uint8_t(x19), uint8_t(x18), uint8_t(x17), uint8_t(x16),
      uint8_t(x15), uint8_t(x14), uint8_t(x13), uint8_t(x12),
      uint8_t(x11), uint8_t(x10), uint8_t(x09), uint8_t(x08),
      uint8_t(x07), uint8_t(x06), uint8_t(x05), uint8_t(x04),
      uint8_t(x03), uint8_t(x02), uint8_t(x01), uint8_t(x00)));
}

template<typename V = Vec64xU8>
BL_INLINE_NODEBUG V make512_u8(uint8_t x0) noexcept {
  return from_simd<V>(I::simd_make512_u8(x0));
}

template<typename V = Vec64xU8>
BL_INLINE_NODEBUG V make512_u8(uint8_t x1, uint8_t x0) noexcept {
  return from_simd<V>(I::simd_make512_u8(x1, x0));
}

template<typename V = Vec64xU8>
BL_INLINE_NODEBUG V make512_u8(uint8_t x3, uint8_t x2, uint8_t x1, uint8_t x0) noexcept {
  return from_simd<V>(I::simd_make512_u8(x3, x2, x1, x0));
}

template<typename V = Vec64xU8>
BL_INLINE_NODEBUG V make512_u8(uint8_t x7, uint8_t x6, uint8_t x5, uint8_t x4,
                               uint8_t x3, uint8_t x2, uint8_t x1, uint8_t x0) noexcept {
  return from_simd<V>(
    I::simd_make512_u8(
      x7, x6, x5, x4,
      x3, x2, x1, x0));
}

template<typename V = Vec64xU8>
BL_INLINE_NODEBUG V make512_u8(uint8_t x15, uint8_t x14, uint8_t x13, uint8_t x12,
                               uint8_t x11, uint8_t x10, uint8_t x09, uint8_t x08,
                               uint8_t x07, uint8_t x06, uint8_t x05, uint8_t x04,
                               uint8_t x03, uint8_t x02, uint8_t x01, uint8_t x00) noexcept {
  return from_simd<V>(
    I::simd_make512_u8(
      x15, x14, x13, x12,
      x11, x10, x09, x08,
      x07, x06, x05, x04,
      x03, x02, x01, x00));
}

template<typename V = Vec64xU8>
BL_INLINE_NODEBUG V make512_u8(uint8_t x31, uint8_t x30, uint8_t x29, uint8_t x28,
                               uint8_t x27, uint8_t x26, uint8_t x25, uint8_t x24,
                               uint8_t x23, uint8_t x22, uint8_t x21, uint8_t x20,
                               uint8_t x19, uint8_t x18, uint8_t x17, uint8_t x16,
                               uint8_t x15, uint8_t x14, uint8_t x13, uint8_t x12,
                               uint8_t x11, uint8_t x10, uint8_t x09, uint8_t x08,
                               uint8_t x07, uint8_t x06, uint8_t x05, uint8_t x04,
                               uint8_t x03, uint8_t x02, uint8_t x01, uint8_t x00) noexcept {
  return from_simd<V>(
    I::simd_make512_u8(
      x31, x30, x29, x28,
      x27, x26, x25, x24,
      x23, x22, x21, x20,
      x19, x18, x17, x16,
      x15, x14, x13, x12,
      x11, x10, x09, x08,
      x07, x06, x05, x04,
      x03, x02, x01, x00));
}

template<typename V = Vec64xU8>
BL_INLINE_NODEBUG V make512_u8(uint8_t x63, uint8_t x62, uint8_t x61, uint8_t x60,
                               uint8_t x59, uint8_t x58, uint8_t x57, uint8_t x56,
                               uint8_t x55, uint8_t x54, uint8_t x53, uint8_t x52,
                               uint8_t x51, uint8_t x50, uint8_t x49, uint8_t x48,
                               uint8_t x47, uint8_t x46, uint8_t x45, uint8_t x44,
                               uint8_t x43, uint8_t x42, uint8_t x41, uint8_t x40,
                               uint8_t x39, uint8_t x38, uint8_t x37, uint8_t x36,
                               uint8_t x35, uint8_t x34, uint8_t x33, uint8_t x32,
                               uint8_t x31, uint8_t x30, uint8_t x29, uint8_t x28,
                               uint8_t x27, uint8_t x26, uint8_t x25, uint8_t x24,
                               uint8_t x23, uint8_t x22, uint8_t x21, uint8_t x20,
                               uint8_t x19, uint8_t x18, uint8_t x17, uint8_t x16,
                               uint8_t x15, uint8_t x14, uint8_t x13, uint8_t x12,
                               uint8_t x11, uint8_t x10, uint8_t x09, uint8_t x08,
                               uint8_t x07, uint8_t x06, uint8_t x05, uint8_t x04,
                               uint8_t x03, uint8_t x02, uint8_t x01, uint8_t x00) noexcept {
  return from_simd<V>(
    I::simd_make512_u8(
      x63, x62, x61, x60,
      x59, x58, x57, x56,
      x55, x54, x53, x52,
      x51, x50, x49, x48,
      x47, x46, x45, x44,
      x43, x42, x41, x40,
      x39, x38, x37, x36,
      x35, x34, x33, x32,
      x31, x30, x29, x28,
      x27, x26, x25, x24,
      x23, x22, x21, x20,
      x19, x18, x17, x16,
      x15, x14, x13, x12,
      x11, x10, x09, x08,
      x07, x06, x05, x04,
      x03, x02, x01, x00));
}

template<typename V = Vec32xI16>
BL_INLINE_NODEBUG V make512_i16(int16_t x0) noexcept {
  return from_simd<V>(I::simd_make512_u16(uint16_t(x0)));
}

template<typename V = Vec32xI16>
BL_INLINE_NODEBUG V make512_i16(int16_t x1, int16_t x0) noexcept {
  return from_simd<V>(I::simd_make512_u16(uint16_t(x1), uint16_t(x0)));
}

template<typename V = Vec32xI16>
BL_INLINE_NODEBUG V make512_i16(int16_t x3, int16_t x2, int16_t x1, int16_t x0) noexcept {
  return from_simd<V>(I::simd_make512_u16(uint16_t(x3), uint16_t(x2), uint16_t(x1), uint16_t(x0)));
}

template<typename V = Vec32xI16>
BL_INLINE_NODEBUG V make512_i16(int16_t x7, int16_t x6, int16_t x5, int16_t x4,
                                int16_t x3, int16_t x2, int16_t x1, int16_t x0) noexcept {
  return from_simd<V>(
    I::simd_make512_u16(
      uint16_t(x7), uint16_t(x6), uint16_t(x5), uint16_t(x4),
      uint16_t(x3), uint16_t(x2), uint16_t(x1), uint16_t(x0)));
}

template<typename V = Vec32xI16>
BL_INLINE_NODEBUG V make512_i16(int16_t x15, int16_t x14, int16_t x13, int16_t x12,
                                int16_t x11, int16_t x10, int16_t x09, int16_t x08,
                                int16_t x07, int16_t x06, int16_t x05, int16_t x04,
                                int16_t x03, int16_t x02, int16_t x01, int16_t x00) noexcept {
  return from_simd<V>(
    I::simd_make512_u16(
      uint16_t(x15), uint16_t(x14), uint16_t(x13), uint16_t(x12),
      uint16_t(x11), uint16_t(x10), uint16_t(x09), uint16_t(x08),
      uint16_t(x07), uint16_t(x06), uint16_t(x05), uint16_t(x04),
      uint16_t(x03), uint16_t(x02), uint16_t(x01), uint16_t(x00)));
}

template<typename V = Vec32xI16>
BL_INLINE_NODEBUG V make512_i16(int16_t x31, int16_t x30, int16_t x29, int16_t x28,
                                int16_t x27, int16_t x26, int16_t x25, int16_t x24,
                                int16_t x23, int16_t x22, int16_t x21, int16_t x20,
                                int16_t x19, int16_t x18, int16_t x17, int16_t x16,
                                int16_t x15, int16_t x14, int16_t x13, int16_t x12,
                                int16_t x11, int16_t x10, int16_t x09, int16_t x08,
                                int16_t x07, int16_t x06, int16_t x05, int16_t x04,
                                int16_t x03, int16_t x02, int16_t x01, int16_t x00) noexcept {
  return from_simd<V>(
    I::simd_make512_u16(
      uint16_t(x31), uint16_t(x30), uint16_t(x29), uint16_t(x28),
      uint16_t(x27), uint16_t(x26), uint16_t(x25), uint16_t(x24),
      uint16_t(x23), uint16_t(x22), uint16_t(x21), uint16_t(x20),
      uint16_t(x19), uint16_t(x18), uint16_t(x17), uint16_t(x16),
      uint16_t(x15), uint16_t(x14), uint16_t(x13), uint16_t(x12),
      uint16_t(x11), uint16_t(x10), uint16_t(x09), uint16_t(x08),
      uint16_t(x07), uint16_t(x06), uint16_t(x05), uint16_t(x04),
      uint16_t(x03), uint16_t(x02), uint16_t(x01), uint16_t(x00)));
}

template<typename V = Vec32xU16>
BL_INLINE_NODEBUG V make512_u16(uint16_t x0) noexcept {
  return from_simd<V>(I::simd_make512_u16(x0));
}

template<typename V = Vec32xU16>
BL_INLINE_NODEBUG V make512_u16(uint16_t x1, uint16_t x0) noexcept {
  return from_simd<V>(I::simd_make512_u16(x1, x0));
}

template<typename V = Vec32xU16>
BL_INLINE_NODEBUG V make512_u16(uint16_t x3, uint16_t x2, uint16_t x1, uint16_t x0) noexcept {
  return from_simd<V>(I::simd_make512_u16(x3, x2, x1, x0));
}

template<typename V = Vec32xU16>
BL_INLINE_NODEBUG V make512_u16(uint16_t x7, uint16_t x6, uint16_t x5, uint16_t x4,
                                uint16_t x3, uint16_t x2, uint16_t x1, uint16_t x0) noexcept {
  return from_simd<V>(
    I::simd_make512_u16(
      x7, x6, x5, x4,
      x3, x2, x1, x0));
}

template<typename V = Vec32xU16>
BL_INLINE_NODEBUG V make512_u16(uint16_t x15, uint16_t x14, uint16_t x13, uint16_t x12,
                                uint16_t x11, uint16_t x10, uint16_t x09, uint16_t x08,
                                uint16_t x07, uint16_t x06, uint16_t x05, uint16_t x04,
                                uint16_t x03, uint16_t x02, uint16_t x01, uint16_t x00) noexcept {
  return from_simd<V>(
    I::simd_make512_u16(
      x15, x14, x13, x12,
      x11, x10, x09, x08,
      x07, x06, x05, x04,
      x03, x02, x01, x00));
}

template<typename V = Vec32xU16>
BL_INLINE_NODEBUG V make512_u16(uint16_t x31, uint16_t x30, uint16_t x29, uint16_t x28,
                                uint16_t x27, uint16_t x26, uint16_t x25, uint16_t x24,
                                uint16_t x23, uint16_t x22, uint16_t x21, uint16_t x20,
                                uint16_t x19, uint16_t x18, uint16_t x17, uint16_t x16,
                                uint16_t x15, uint16_t x14, uint16_t x13, uint16_t x12,
                                uint16_t x11, uint16_t x10, uint16_t x09, uint16_t x08,
                                uint16_t x07, uint16_t x06, uint16_t x05, uint16_t x04,
                                uint16_t x03, uint16_t x02, uint16_t x01, uint16_t x00) noexcept {
  return from_simd<V>(
    I::simd_make512_u16(
      x31, x30, x29, x28,
      x27, x26, x25, x24,
      x23, x22, x21, x20,
      x19, x18, x17, x16,
      x15, x14, x13, x12,
      x11, x10, x09, x08,
      x07, x06, x05, x04,
      x03, x02, x01, x00));
}

template<typename V = Vec16xI32>
BL_INLINE_NODEBUG V make512_i32(int32_t x0) noexcept {
  return from_simd<V>(I::simd_make512_u32(uint32_t(x0)));
}

template<typename V = Vec16xI32>
BL_INLINE_NODEBUG V make512_i32(int32_t x1, int32_t x0) noexcept {
  return from_simd<V>(I::simd_make512_u32(uint32_t(x1), uint32_t(x0)));
}

template<typename V = Vec16xI32>
BL_INLINE_NODEBUG V make512_i32(int32_t x3, int32_t x2, int32_t x1, int32_t x0) noexcept {
  return from_simd<V>(I::simd_make512_u32(uint32_t(x3), uint32_t(x2), uint32_t(x1), uint32_t(x0)));
}

template<typename V = Vec16xI32>
BL_INLINE_NODEBUG V make512_i32(int32_t x7, int32_t x6, int32_t x5, int32_t x4,
                                int32_t x3, int32_t x2, int32_t x1, int32_t x0) noexcept {
  return from_simd<V>(
    I::simd_make512_u32(
      uint32_t(x7), uint32_t(x6), uint32_t(x5), uint32_t(x4),
      uint32_t(x3), uint32_t(x2), uint32_t(x1), uint32_t(x0)));
}

template<typename V = Vec16xI32>
BL_INLINE_NODEBUG V make512_i32(int32_t x15, int32_t x14, int32_t x13, int32_t x12,
                                int32_t x11, int32_t x10, int32_t x09, int32_t x08,
                                int32_t x07, int32_t x06, int32_t x05, int32_t x04,
                                int32_t x03, int32_t x02, int32_t x01, int32_t x00) noexcept {
  return from_simd<V>(
    I::simd_make512_u32(
      uint32_t(x15), uint32_t(x14), uint32_t(x13), uint32_t(x12),
      uint32_t(x11), uint32_t(x10), uint32_t(x09), uint32_t(x08),
      uint32_t(x07), uint32_t(x06), uint32_t(x05), uint32_t(x04),
      uint32_t(x03), uint32_t(x02), uint32_t(x01), uint32_t(x00)));
}

template<typename V = Vec16xU32>
BL_INLINE_NODEBUG V make512_u32(uint32_t x0) noexcept {
  return from_simd<V>(I::simd_make512_u32(x0));
}

template<typename V = Vec16xU32>
BL_INLINE_NODEBUG V make512_u32(uint32_t x1, uint32_t x0) noexcept {
  return from_simd<V>(I::simd_make512_u32(x1, x0));
}

template<typename V = Vec16xU32>
BL_INLINE_NODEBUG V make512_u32(uint32_t x3, uint32_t x2, uint32_t x1, uint32_t x0) noexcept {
  return from_simd<V>(I::simd_make512_u32(x3, x2, x1, x0));
}

template<typename V = Vec16xU32>
BL_INLINE_NODEBUG V make512_u32(uint32_t x7, uint32_t x6, uint32_t x5, uint32_t x4,
                                uint32_t x3, uint32_t x2, uint32_t x1, uint32_t x0) noexcept {
  return from_simd<V>(I::simd_make512_u32(x7, x6, x5, x4, x3, x2, x1, x0));
}

template<typename V = Vec16xU32>
BL_INLINE_NODEBUG V make512_u32(uint32_t x15, uint32_t x14, uint32_t x13, uint32_t x12,
                                uint32_t x11, uint32_t x10, uint32_t x09, uint32_t x08,
                                uint32_t x07, uint32_t x06, uint32_t x05, uint32_t x04,
                                uint32_t x03, uint32_t x02, uint32_t x01, uint32_t x00) noexcept {
  return from_simd<V>(
    I::simd_make512_u32(
      x15, x14, x13, x12,
      x11, x10, x09, x08,
      x07, x06, x05, x04,
      x03, x02, x01, x00));
}

template<typename V = Vec8xI64>
BL_INLINE_NODEBUG V make512_i64(int64_t x0) noexcept {
  return from_simd<V>(I::simd_make512_u64(uint64_t(x0)));
}

template<typename V = Vec8xI64>
BL_INLINE_NODEBUG V make512_i64(int64_t x1, int64_t x0) noexcept {
  return from_simd<V>(I::simd_make512_u64(uint64_t(x1), uint64_t(x0)));
}

template<typename V = Vec8xI64>
BL_INLINE_NODEBUG V make512_i64(int64_t x3, int64_t x2, int64_t x1, int64_t x0) noexcept {
  return from_simd<V>(I::simd_make512_u64(uint64_t(x3), uint64_t(x2), uint64_t(x1), uint64_t(x0)));
}

template<typename V = Vec8xI64>
BL_INLINE_NODEBUG V make512_i64(int64_t x7, int64_t x6, int64_t x5, int64_t x4,
                                int64_t x3, int64_t x2, int64_t x1, int64_t x0) noexcept {
  return from_simd<V>(
    I::simd_make512_u64(
      uint64_t(x7), uint64_t(x6), uint64_t(x5), uint64_t(x4),
      uint64_t(x3), uint64_t(x2), uint64_t(x1), uint64_t(x0)));
}

template<typename V = Vec8xU64>
BL_INLINE_NODEBUG V make512_u64(uint64_t x0) noexcept {
  return from_simd<V>(I::simd_make512_u64(x0));
}

template<typename V = Vec8xU64>
BL_INLINE_NODEBUG V make512_u64(uint64_t x1, uint64_t x0) noexcept {
  return from_simd<V>(I::simd_make512_u64(x1, x0));
}

template<typename V = Vec8xU64>
BL_INLINE_NODEBUG V make512_u64(uint64_t x3, uint64_t x2, uint64_t x1, uint64_t x0) noexcept {
  return from_simd<V>(I::simd_make512_u64(x3, x2, x1, x0));
}

template<typename V = Vec8xU64>
BL_INLINE_NODEBUG V make512_u64(uint64_t x7, uint64_t x6, uint64_t x5, uint64_t x4,
                                uint64_t x3, uint64_t x2, uint64_t x1, uint64_t x0) noexcept {
  return from_simd<V>(
    I::simd_make512_u64(
      x7, x6, x5, x4,
      x3, x2, x1, x0));
}

template<typename V = Vec16xF32>
BL_INLINE_NODEBUG V make512_f32(float x0) noexcept {
  return from_simd<V>(I::simd_make512_f32(x0));
}

template<typename V = Vec16xF32>
BL_INLINE_NODEBUG V make512_f32(float x1, float x0) noexcept {
  return from_simd<V>(I::simd_make512_f32(x1, x0));
}

template<typename V = Vec16xF32>
BL_INLINE_NODEBUG V make512_f32(float x3, float x2, float x1, float x0) noexcept {
  return from_simd<V>(I::simd_make512_f32(x3, x2, x1, x0));
}

template<typename V = Vec16xF32>
BL_INLINE_NODEBUG V make512_f32(float x7, float x6, float x5, float x4,
                                float x3, float x2, float x1, float x0) noexcept {
  return from_simd<V>(I::simd_make512_f32(x7, x6, x5, x4, x3, x2, x1, x0));
}

template<typename V = Vec16xF32>
BL_INLINE_NODEBUG V make512_f32(float x15, float x14, float x13, float x12,
                                float x11, float x10, float x09, float x08,
                                float x07, float x06, float x05, float x04,
                                float x03, float x02, float x01, float x00) noexcept {
  return from_simd<V>(
    I::simd_make512_f32(
      x15, x14, x13, x12,
      x11, x10, x09, x08,
      x07, x06, x05, x04,
      x03, x02, x01, x00));
}

template<typename V = Vec8xF64>
BL_INLINE_NODEBUG V make512_f64(double x0) noexcept {
  return from_simd<V>(I::simd_make512_f64(x0));
}

template<typename V = Vec8xF64>
BL_INLINE_NODEBUG V make512_f64(double x1, double x0) noexcept {
  return from_simd<V>(I::simd_make512_f64(x1, x0));
}

template<typename V = Vec8xF64>
BL_INLINE_NODEBUG V make512_f64(double x3, double x2, double x1, double x0) noexcept {
  return from_simd<V>(I::simd_make512_f64(x3, x2, x1, x0));
}

template<typename V = Vec8xF64>
BL_INLINE_NODEBUG V make512_f32(double x7, double x6, double x5, double x4,
                                double x3, double x2, double x1, double x0) noexcept {
  return from_simd<V>(I::simd_make512_f64(x7, x6, x5, x4, x3, x2, x1, x0));
}
#endif // BL_TARGET_OPT_AVX512

// SIMD - Public - Cast Vector <-> Scalar
// ======================================

template<typename V = Vec4xI32> BL_INLINE_NODEBUG V cast_from_i32(int32_t val) noexcept { return from_simd<V>(I::simd_cast_from_u32(uint32_t(val))); }
template<typename V = Vec4xU32> BL_INLINE_NODEBUG V cast_from_u32(uint32_t val) noexcept { return from_simd<V>(I::simd_cast_from_u32(val)); }
template<typename V = Vec2xI64> BL_INLINE_NODEBUG V cast_from_i64(int64_t val) noexcept { return from_simd<V>(I::simd_cast_from_u64(uint64_t(val))); }
template<typename V = Vec2xU64> BL_INLINE_NODEBUG V cast_from_u64(uint64_t val) noexcept { return from_simd<V>(I::simd_cast_from_u64(val)); }
template<typename V = Vec4xF32> BL_INLINE_NODEBUG V cast_from_f32(float val) noexcept { return from_simd<V>(I::simd_cast_from_f32(val)); }
template<typename V = Vec2xF64> BL_INLINE_NODEBUG V cast_from_f64(double val) noexcept { return from_simd<V>(I::simd_cast_from_f64(val)); }

template<typename V> BL_INLINE_NODEBUG int32_t cast_to_i32(const V& src) noexcept { return int32_t(I::simd_cast_to_u32(simd_cast<__m128i>(src.v))); }
template<typename V> BL_INLINE_NODEBUG uint32_t cast_to_u32(const V& src) noexcept { return I::simd_cast_to_u32(simd_cast<__m128i>(src.v)); }
template<typename V> BL_INLINE_NODEBUG int64_t cast_to_i64(const V& src) noexcept { return int64_t(I::simd_cast_to_u64(simd_cast<__m128i>(src.v))); }
template<typename V> BL_INLINE_NODEBUG uint64_t cast_to_u64(const V& src) noexcept { return I::simd_cast_to_u64(simd_cast<__m128i>(src.v)); }
template<typename V> BL_INLINE_NODEBUG float cast_to_f32(const V& src) noexcept { return I::simd_cast_to_f32(simd_cast<__m128>(src.v)); }
template<typename V> BL_INLINE_NODEBUG double cast_to_f64(const V& src) noexcept { return I::simd_cast_to_f64(simd_cast<__m128d>(src.v)); }

// SIMD - Public - Convert Vector <-> Vector
// =========================================

template<typename V> BL_INLINE_NODEBUG Vec<V::kW, float> cvt_i32_f32(const V& a) noexcept { return Vec<V::kW, float>{I::simd_cvt_i32_f32(simd_as_i(a.v))}; }
template<typename V> BL_INLINE_NODEBUG Vec<V::kW, int32_t> cvt_f32_i32(const V& a) noexcept { return Vec<V::kW, int32_t>{I::simd_cvt_f32_i32(simd_as_f(a.v))}; }
template<typename V> BL_INLINE_NODEBUG Vec<V::kW, int32_t> cvtt_f32_i32(const V& a) noexcept { return Vec<V::kW, int32_t>{I::simd_cvtt_f32_i32(simd_as_f(a.v))}; }

// SIMD - Public - Convert Vector <-> Scalar
// =========================================

BL_INLINE_NODEBUG Vec4xF32 cvt_f32_from_scalar_i32(int32_t val) noexcept { return Vec4xF32{I::simd_cvt_f32_from_scalar_i32(val)}; }
BL_INLINE_NODEBUG Vec2xF64 cvt_f64_from_scalar_i32(int32_t val) noexcept { return Vec2xF64{I::simd_cvt_f64_from_scalar_i32(val)}; }

template<typename V> BL_INLINE_NODEBUG int32_t cvt_f32_to_scalar_i32(const V& src) noexcept { return I::simd_cvt_f32_to_scalar_i32(simd_cast<__m128>(src.v)); }
template<typename V> BL_INLINE_NODEBUG int32_t cvt_f64_to_scalar_i32(const V& src) noexcept { return I::simd_cvt_f64_to_scalar_i32(simd_cast<__m128d>(src.v)); }
template<typename V> BL_INLINE_NODEBUG int32_t cvtt_f32_to_scalar_i32(const V& src) noexcept { return I::simd_cvtt_f32_to_scalar_i32(simd_cast<__m128>(src.v)); }
template<typename V> BL_INLINE_NODEBUG int32_t cvtt_f64_to_scalar_i32(const V& src) noexcept { return I::simd_cvtt_f64_to_scalar_i32(simd_cast<__m128d>(src.v)); }

#if BL_TARGET_ARCH_BITS >= 64
BL_INLINE_NODEBUG Vec4xF32 cvt_f32_from_scalar_i64(int64_t val) noexcept { return Vec4xF32{I::simd_cvt_f32_from_scalar_i64(val)}; }
BL_INLINE_NODEBUG Vec2xF64 cvt_f64_from_scalar_i64(int64_t val) noexcept { return Vec2xF64{I::simd_cvt_f64_from_scalar_i64(val)}; }

template<typename V> BL_INLINE_NODEBUG int64_t cvt_f32_to_scalar_i64(const V& src) noexcept { return I::simd_cvt_f32_to_scalar_i64(simd_cast<__m128>(src.v)); }
template<typename V> BL_INLINE_NODEBUG int64_t cvt_f64_to_scalar_i64(const V& src) noexcept { return I::simd_cvt_f64_to_scalar_i64(simd_cast<__m128d>(src.v)); }
template<typename V> BL_INLINE_NODEBUG int64_t cvtt_f32_to_scalar_i64(const V& src) noexcept { return I::simd_cvtt_f32_to_scalar_i64(simd_cast<__m128>(src.v)); }
template<typename V> BL_INLINE_NODEBUG int64_t cvtt_f64_to_scalar_i64(const V& src) noexcept { return I::simd_cvtt_f64_to_scalar_i64(simd_cast<__m128d>(src.v)); }
#endif

// SIMD - Public - Extract MSB
// ===========================

template<typename T> BL_INLINE_NODEBUG uint32_t extract_sign_bits_i8(const Vec<16, T>& a) noexcept {
  return I::simd_extract_sign_bits_i8(simd_as_i(a.v));
}

template<typename T> BL_INLINE_NODEBUG uint32_t extract_sign_bits_i8(const Vec<16, T>& a, const Vec<16, T>& b) noexcept {
  return extract_sign_bits_i8(a) | (extract_sign_bits_i8(b) << 16);
}

template<typename T> BL_INLINE_NODEBUG uint64_t extract_sign_bits_i8(const Vec<16, T>& a, const Vec<16, T>& b, const Vec<16, T>& c, const Vec<16, T>& d) noexcept {
  uint32_t i0 = extract_sign_bits_i8(a) | (extract_sign_bits_i8(b) << 16);
  uint32_t i1 = extract_sign_bits_i8(c) | (extract_sign_bits_i8(d) << 16);

  return uint64_t(i0) | (uint64_t(i1) << 32);
}

template<typename T> BL_INLINE_NODEBUG uint32_t extract_mask_bits_i8(const Vec<16, T>& a) noexcept {
  return extract_sign_bits_i8(a);
}

template<typename T> BL_INLINE_NODEBUG uint32_t extract_mask_bits_i8(const Vec<16, T>& a, const Vec<16, T>& b) noexcept {
  return extract_sign_bits_i8(a, b);
}

template<typename T> BL_INLINE_NODEBUG uint64_t extract_mask_bits_i8(const Vec<16, T>& a, const Vec<16, T>& b, const Vec<16, T>& c, const Vec<16, T>& d) noexcept {
  return extract_sign_bits_i8(a, b, c, d);
}

#if defined(BL_TARGET_OPT_AVX2)
template<typename T> BL_INLINE_NODEBUG uint32_t extract_sign_bits_i8(const Vec<32, T>& a) noexcept {
    return I::simd_extract_sign_bits_i8(simd_as_i(a.v));
}

template<typename T> BL_INLINE_NODEBUG uint64_t extract_sign_bits_i8(const Vec<32, T>& a, const Vec<32, T>& b) noexcept {
  return uint64_t(extract_sign_bits_i8(a)) | (uint64_t(extract_sign_bits_i8(b)) << 32);
}

template<typename T> BL_INLINE_NODEBUG uint32_t extract_mask_bits_i8(const Vec<32, T>& a) noexcept {
  return extract_sign_bits_i8(a);
}

template<typename T> BL_INLINE_NODEBUG uint64_t extract_mask_bits_i8(const Vec<32, T>& a, const Vec<32, T>& b) noexcept {
  return extract_sign_bits_i8(a, b);
}
#endif // BL_TARGET_OPT_AVX2

#if defined(BL_TARGET_OPT_AVX512)
// NOTE: 64-byte vectors require 64-bit integer to store all most significant bits.
template<typename T> BL_INLINE_NODEBUG uint64_t extract_sign_bits_i8(const Vec<64, T>& a) noexcept {
  return I::simd_extract_sign_bits_i8(simd_as_i(a.v));
}

template<typename T> BL_INLINE_NODEBUG uint64_t extract_mask_bits_i8(const Vec<64, T>& a) noexcept {
  return extract_sign_bits_i8(a);
}
#endif // BL_TARGET_OPT_AVX512

template<typename V> BL_INLINE_NODEBUG uint32_t extract_sign_bits_i32(const V& a) noexcept { return I::simd_extract_sign_bits_i32(simd_as_i(a.v)); }
template<typename V> BL_INLINE_NODEBUG uint32_t extract_sign_bits_i64(const V& a) noexcept { return I::simd_extract_sign_bits_i64(simd_as_i(a.v)); }

template<typename V> BL_INLINE_NODEBUG uint32_t extract_mask_bits_i32(const V& a) noexcept { return extract_sign_bits_i32(a); }
template<typename V> BL_INLINE_NODEBUG uint32_t extract_mask_bits_i64(const V& a) noexcept { return extract_sign_bits_i64(a); }

// SIMD - Public - Load & Store Operations (128-bit)
// =================================================

template<typename V> BL_INLINE_NODEBUG V load_8(const void* src) noexcept { return V{I::simd_load_8<typename V::SimdType>(src)}; }
template<typename V> BL_INLINE_NODEBUG V loada_16(const void* src) noexcept { return V{I::simd_loada_16<typename V::SimdType>(src)}; }
template<typename V> BL_INLINE_NODEBUG V loadu_16(const void* src) noexcept { return V{I::simd_loadu_16<typename V::SimdType>(src)}; }
template<typename V> BL_INLINE_NODEBUG V loada_32(const void* src) noexcept { return V{I::simd_loada_32<typename V::SimdType>(src)}; }
template<typename V> BL_INLINE_NODEBUG V loadu_32(const void* src) noexcept { return V{I::simd_loadu_32<typename V::SimdType>(src)}; }
template<typename V> BL_INLINE_NODEBUG V loada_64(const void* src) noexcept { return V{I::simd_loada_64<typename V::SimdType>(src)}; }
template<typename V> BL_INLINE_NODEBUG V loadu_64(const void* src) noexcept { return V{I::simd_loadu_64<typename V::SimdType>(src)}; }
template<typename V> BL_INLINE_NODEBUG V loada_128(const void* src) noexcept { return V{I::simd_loada_128<typename V::SimdType>(src)}; }
template<typename V> BL_INLINE_NODEBUG V loadu_128(const void* src) noexcept { return V{I::simd_loadu_128<typename V::SimdType>(src)}; }

template<typename T> BL_INLINE_NODEBUG Vec<16, T> loadl_64(const Vec<16, T>& dst, const void* src) noexcept { return Vec<16, T>{I::simd_loadl_64(dst.v, src)}; }
template<typename T> BL_INLINE_NODEBUG Vec<16, T> loadh_64(const Vec<16, T>& dst, const void* src) noexcept { return Vec<16, T>{I::simd_loadh_64(dst.v, src)}; }

template<typename V> BL_INLINE_NODEBUG void store_8(void* dst, const V& src) noexcept { I::simd_store_8(dst, src.v); }
template<typename V> BL_INLINE_NODEBUG void storea_16(void* dst, const V& src) noexcept { I::simd_storea_16(dst, src.v); }
template<typename V> BL_INLINE_NODEBUG void storeu_16(void* dst, const V& src) noexcept { I::simd_storeu_16(dst, src.v); }
template<typename V> BL_INLINE_NODEBUG void storea_32(void* dst, const V& src) noexcept { I::simd_storea_32(dst, src.v); }
template<typename V> BL_INLINE_NODEBUG void storeu_32(void* dst, const V& src) noexcept { I::simd_storeu_32(dst, src.v); }
template<typename V> BL_INLINE_NODEBUG void storea_64(void* dst, const V& src) noexcept { I::simd_storea_64(dst, src.v); }
template<typename V> BL_INLINE_NODEBUG void storeu_64(void* dst, const V& src) noexcept { I::simd_storeu_64(dst, src.v); }
template<typename V> BL_INLINE_NODEBUG void storeh_64(void* dst, const V& src) noexcept { I::simd_storeh_64(dst, src.v); }
template<typename V> BL_INLINE_NODEBUG void storea_128(void* dst, const V& src) noexcept { I::simd_storea_128(dst, src.v); }
template<typename V> BL_INLINE_NODEBUG void storeu_128(void* dst, const V& src) noexcept { I::simd_storeu_128(dst, src.v); }

#if defined(BL_TARGET_OPT_SSE4_1)
template<typename V> BL_INLINE_NODEBUG V loada_16_i8_i64(const void* src) noexcept { return from_simd<V>(I::simd_loada_16_i8_i64(src)); }
template<typename V> BL_INLINE_NODEBUG V loadu_16_i8_i64(const void* src) noexcept { return from_simd<V>(I::simd_loadu_16_i8_i64(src)); }
template<typename V> BL_INLINE_NODEBUG V loada_16_u8_u64(const void* src) noexcept { return from_simd<V>(I::simd_loada_16_u8_u64(src)); }
template<typename V> BL_INLINE_NODEBUG V loadu_16_u8_u64(const void* src) noexcept { return from_simd<V>(I::simd_loadu_16_u8_u64(src)); }
#endif // BL_TARGET_OPT_SSE4_1

template<typename V> BL_INLINE_NODEBUG V loada_32_i8_i32(const void* src) noexcept { return from_simd<V>(I::simd_loada_32_i8_i32(src)); }
template<typename V> BL_INLINE_NODEBUG V loadu_32_i8_i32(const void* src) noexcept { return from_simd<V>(I::simd_loadu_32_i8_i32(src)); }
template<typename V> BL_INLINE_NODEBUG V loada_32_u8_u32(const void* src) noexcept { return from_simd<V>(I::simd_loada_32_u8_u32(src)); }
template<typename V> BL_INLINE_NODEBUG V loadu_32_u8_u32(const void* src) noexcept { return from_simd<V>(I::simd_loadu_32_u8_u32(src)); }
template<typename V> BL_INLINE_NODEBUG V loada_64_i8_i16(const void* src) noexcept { return from_simd<V>(I::simd_loada_64_i8_i16(src)); }
template<typename V> BL_INLINE_NODEBUG V loadu_64_i8_i16(const void* src) noexcept { return from_simd<V>(I::simd_loadu_64_i8_i16(src)); }
template<typename V> BL_INLINE_NODEBUG V loada_64_u8_u16(const void* src) noexcept { return from_simd<V>(I::simd_loada_64_u8_u16(src)); }
template<typename V> BL_INLINE_NODEBUG V loadu_64_u8_u16(const void* src) noexcept { return from_simd<V>(I::simd_loadu_64_u8_u16(src)); }

// SIMD - Public - Load & Store Operations (256-bit)
// =================================================

#if defined(BL_TARGET_OPT_AVX)
template<typename V> BL_INLINE_NODEBUG V loada_256(const void* src) noexcept { return V{I::simd_loada_256<typename V::SimdType>(src)}; }
template<typename V> BL_INLINE_NODEBUG V loadu_256(const void* src) noexcept { return V{I::simd_loadu_256<typename V::SimdType>(src)}; }
template<typename V> BL_INLINE_NODEBUG void storea_256(void* dst, const V& src) noexcept { I::simd_storea_256(dst, src.v); }
template<typename V> BL_INLINE_NODEBUG void storeu_256(void* dst, const V& src) noexcept { I::simd_storeu_256(dst, src.v); }

template<typename V> BL_INLINE_NODEBUG V load_broadcast_u32(const void* src) noexcept { return from_simd<V>(I::simd_load_broadcast_u32<V::kW>(src)); }
template<typename V> BL_INLINE_NODEBUG V load_broadcast_u64(const void* src) noexcept { return from_simd<V>(I::simd_load_broadcast_u64<V::kW>(src)); }
template<typename V> BL_INLINE_NODEBUG V load_broadcast_f32(const void* src) noexcept { return from_simd<V>(I::simd_load_broadcast_f32<V::kW>(src)); }
template<typename V> BL_INLINE_NODEBUG V load_broadcast_f64(const void* src) noexcept { return from_simd<V>(I::simd_load_broadcast_f64<V::kW>(src)); }

template<typename V> BL_INLINE_NODEBUG V load_broadcast_4xi32(const void* src) noexcept { return from_simd<V>(I::simd_load_broadcast_4xi32<V::kW>(src)); }
template<typename V> BL_INLINE_NODEBUG V load_broadcast_2xi64(const void* src) noexcept { return from_simd<V>(I::simd_load_broadcast_2xi64<V::kW>(src)); }
template<typename V> BL_INLINE_NODEBUG V load_broadcast_f32x4(const void* src) noexcept { return from_simd<V>(I::simd_load_broadcast_f32x4<V::kW>(src)); }
template<typename V> BL_INLINE_NODEBUG V load_broadcast_f64x2(const void* src) noexcept { return from_simd<V>(I::simd_load_broadcast_f64x2<V::kW>(src)); }
#endif // BL_TARGET_OPT_AVX

#if defined(BL_TARGET_OPT_AVX2)
template<typename V> BL_INLINE_NODEBUG V loada_32_i8_i64(const void* src) noexcept { return from_simd<V>(I::simd_loada_32_i8_i64(src)); }
template<typename V> BL_INLINE_NODEBUG V loadu_32_i8_i64(const void* src) noexcept { return from_simd<V>(I::simd_loadu_32_i8_i64(src)); }
template<typename V> BL_INLINE_NODEBUG V loada_32_u8_u64(const void* src) noexcept { return from_simd<V>(I::simd_loada_32_u8_u64(src)); }
template<typename V> BL_INLINE_NODEBUG V loadu_32_u8_u64(const void* src) noexcept { return from_simd<V>(I::simd_loadu_32_u8_u64(src)); }
template<typename V> BL_INLINE_NODEBUG V loada_64_i8_i32(const void* src) noexcept { return from_simd<V>(I::simd_loada_64_i8_i32(src)); }
template<typename V> BL_INLINE_NODEBUG V loadu_64_i8_i32(const void* src) noexcept { return from_simd<V>(I::simd_loadu_64_i8_i32(src)); }
template<typename V> BL_INLINE_NODEBUG V loada_64_u8_u32(const void* src) noexcept { return from_simd<V>(I::simd_loada_64_u8_u32(src)); }
template<typename V> BL_INLINE_NODEBUG V loadu_64_u8_u32(const void* src) noexcept { return from_simd<V>(I::simd_loadu_64_u8_u32(src)); }
template<typename V> BL_INLINE_NODEBUG V loada_128_i8_i16(const void* src) noexcept { return from_simd<V>(I::simd_loada_128_i8_i16(src)); }
template<typename V> BL_INLINE_NODEBUG V loadu_128_i8_i16(const void* src) noexcept { return from_simd<V>(I::simd_loadu_128_i8_i16(src)); }
template<typename V> BL_INLINE_NODEBUG V loada_128_u8_u16(const void* src) noexcept { return from_simd<V>(I::simd_loada_128_u8_u16(src)); }
template<typename V> BL_INLINE_NODEBUG V loadu_128_u8_u16(const void* src) noexcept { return from_simd<V>(I::simd_loadu_128_u8_u16(src)); }
#endif // BL_TARGET_OPT_AVX2

// SIMD - Public - Load & Store Operations (512-bit)
// =================================================

#if defined(BL_TARGET_OPT_AVX512)
template<typename V> BL_INLINE_NODEBUG V loada_512(const void* src) noexcept { return V{I::simd_loada_512<typename V::SimdType>(src)}; }
template<typename V> BL_INLINE_NODEBUG V loadu_512(const void* src) noexcept { return V{I::simd_loadu_512<typename V::SimdType>(src)}; }
template<typename V> BL_INLINE_NODEBUG void storea_512(void* dst, const V& src) noexcept { I::simd_storea_512(dst, src.v); }
template<typename V> BL_INLINE_NODEBUG void storeu_512(void* dst, const V& src) noexcept { I::simd_storeu_512(dst, src.v); }

template<typename V> BL_INLINE_NODEBUG V loada_64_i8_i64(const void* src) noexcept { return from_simd<V>(I::simd_loada_64_i8_i64(src)); }
template<typename V> BL_INLINE_NODEBUG V loadu_64_i8_i64(const void* src) noexcept { return from_simd<V>(I::simd_loadu_64_i8_i64(src)); }
template<typename V> BL_INLINE_NODEBUG V loada_64_u8_u64(const void* src) noexcept { return from_simd<V>(I::simd_loada_64_u8_u64(src)); }
template<typename V> BL_INLINE_NODEBUG V loadu_64_u8_u64(const void* src) noexcept { return from_simd<V>(I::simd_loadu_64_u8_u64(src)); }
template<typename V> BL_INLINE_NODEBUG V loada_128_i8_i32(const void* src) noexcept { return from_simd<V>(I::simd_loada_128_i8_i32(src)); }
template<typename V> BL_INLINE_NODEBUG V loadu_128_i8_i32(const void* src) noexcept { return from_simd<V>(I::simd_loadu_128_i8_i32(src)); }
template<typename V> BL_INLINE_NODEBUG V loada_128_u8_u32(const void* src) noexcept { return from_simd<V>(I::simd_loada_128_u8_u32(src)); }
template<typename V> BL_INLINE_NODEBUG V loadu_128_u8_u32(const void* src) noexcept { return from_simd<V>(I::simd_loadu_128_u8_u32(src)); }
template<typename V> BL_INLINE_NODEBUG V loada_256_i8_i16(const void* src) noexcept { return from_simd<V>(I::simd_loada_256_i8_i16(src)); }
template<typename V> BL_INLINE_NODEBUG V loadu_256_i8_i16(const void* src) noexcept { return from_simd<V>(I::simd_loadu_256_i8_i16(src)); }
template<typename V> BL_INLINE_NODEBUG V loada_256_u8_u16(const void* src) noexcept { return from_simd<V>(I::simd_loada_256_u8_u16(src)); }
template<typename V> BL_INLINE_NODEBUG V loadu_256_u8_u16(const void* src) noexcept { return from_simd<V>(I::simd_loadu_256_u8_u16(src)); }
#endif // BL_TARGET_OPT_AVX512

// SIMD - Public - Load & Store Operations (Native)
// ================================================

template<typename V> BL_INLINE_NODEBUG V loada(const void* src) noexcept { return V{I::simd_loada<typename V::SimdType>(src)}; }
template<typename V> BL_INLINE_NODEBUG V loadu(const void* src) noexcept { return V{I::simd_loadu<typename V::SimdType>(src)}; }

template<typename V> BL_INLINE_NODEBUG void storea(void* dst, const V& src) noexcept { I::simd_storea(dst, src.v); }
template<typename V> BL_INLINE_NODEBUG void storeu(void* dst, const V& src) noexcept { I::simd_storeu(dst, src.v); }

// SIMD - Public - Shuffle & Permute
// =================================

#if defined(BL_SIMD_FEATURE_SWIZZLEV_U8)
template<typename V, typename W>
BL_INLINE_NODEBUG V swizzlev_u8(const V& a, const W& b) noexcept { return V{I::simd_swizzlev_u8(a.v, b.v)}; }
#endif // BL_SIMD_FEATURE_SWIZZLEV_U8

template<uint8_t D, uint8_t C, uint8_t B, uint8_t A, typename V>
BL_INLINE_NODEBUG V swizzle_u16(const V& a) noexcept { return V{I::simd_swizzle_u16<D, C, B, A>(a.v)}; }
template<uint8_t D, uint8_t C, uint8_t B, uint8_t A, typename V>
BL_INLINE_NODEBUG V swizzle_lo_u16(const V& a) noexcept { return V{I::simd_swizzle_lo_u16<D, C, B, A>(a.v)}; }
template<uint8_t D, uint8_t C, uint8_t B, uint8_t A, typename V>
BL_INLINE_NODEBUG V swizzle_hi_u16(const V& a) noexcept { return V{I::simd_swizzle_hi_u16<D, C, B, A>(a.v)}; }

template<uint8_t D, uint8_t C, uint8_t B, uint8_t A, typename V>
BL_INLINE_NODEBUG V swizzle_f32(const V& a) noexcept { return V{I::simd_swizzle_f32<D, C, B, A>(a.v)}; }
template<uint8_t B, uint8_t A, typename V>
BL_INLINE_NODEBUG V swizzle_f64(const V& a) noexcept { return V{I::simd_swizzle_f64<B, A>(a.v)}; }

template<uint8_t D, uint8_t C, uint8_t B, uint8_t A, typename V>
BL_INLINE_NODEBUG V swizzle_u32(const V& a) noexcept { return V{I::simd_swizzle_u32<D, C, B, A>(a.v)}; }
template<uint8_t B, uint8_t A, typename V>
BL_INLINE_NODEBUG V swizzle_u64(const V& a) noexcept { return V{I::simd_swizzle_u64<B, A>(a.v)}; }

template<uint8_t D, uint8_t C, uint8_t B, uint8_t A, typename V>
BL_INLINE_NODEBUG V shuffle_f32(const V& lo, const V& hi) noexcept { return V{I::simd_shuffle_f32<D, C, B, A>(lo.v, hi.v)}; }
template<uint8_t B, uint8_t A, typename V>
BL_INLINE_NODEBUG V shuffle_f64(const V& lo, const V& hi) noexcept { return V{I::simd_shuffle_f64<B, A>(lo.v, hi.v)}; }

template<uint8_t D, uint8_t C, uint8_t B, uint8_t A, typename V>
BL_INLINE_NODEBUG V shuffle_u32(const V& lo, const V& hi) noexcept { return V{I::simd_shuffle_u32<D, C, B, A>(lo.v, hi.v)}; }
template<uint8_t B, uint8_t A, typename V>
BL_INLINE_NODEBUG V shuffle_u64(const V& lo, const V& hi) noexcept { return V{I::simd_shuffle_u64<B, A>(lo.v, hi.v)}; }

#if defined(BL_TARGET_OPT_AVX2)
template<uint8_t D, uint8_t C, uint8_t B, uint8_t A, typename V>
BL_INLINE_NODEBUG V permute_i64(const V& a) noexcept { return from_simd<V>(I::simd_permute_u64<D, C, B, A>(simd_as_i(a.v))); }

template<uint8_t B, uint8_t A, typename V>
BL_INLINE_NODEBUG V permute_i128(const V& a) noexcept { return from_simd<V>(I::simd_permute_u128<B, A>(simd_cast<__m256i>(a.v))); }
template<uint8_t B, uint8_t A, typename V>
BL_INLINE_NODEBUG V permute_i128(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_permute_u128<B, A>(simd_cast<__m256i>(a.v), simd_cast<__m256i>(b.v))); }

template<typename V>
BL_INLINE_NODEBUG V interleave_i128(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_interleave_u128(simd_cast<__m128i>(a.v), simd_cast<__m128i>(b.v))); }
template<typename V, typename W>
BL_INLINE_NODEBUG V interleave_i128(const W& a, const W& b) noexcept { return from_simd<V>(I::simd_interleave_u128(simd_cast<__m128i>(a.v), simd_cast<__m128i>(b.v))); }
#endif

template<typename V, typename W> BL_INLINE_NODEBUG V broadcast_u8(const W& a) noexcept { return V{I::simd_broadcast_u8<V::kW>(simd_cast<__m128i>(a.v))}; }
template<typename V, typename W> BL_INLINE_NODEBUG V broadcast_u16(const W& a) noexcept { return V{I::simd_broadcast_u16<V::kW>(simd_cast<__m128i>(a.v))}; }
template<typename V, typename W> BL_INLINE_NODEBUG V broadcast_u32(const W& a) noexcept { return V{I::simd_broadcast_u32<V::kW>(simd_cast<__m128i>(a.v))}; }
template<typename V, typename W> BL_INLINE_NODEBUG V broadcast_u64(const W& a) noexcept { return V{I::simd_broadcast_u64<V::kW>(simd_cast<__m128i>(a.v))}; }
template<typename V, typename W> BL_INLINE_NODEBUG V broadcast_f32(const W& a) noexcept { return V{I::simd_broadcast_f32<V::kW>(simd_cast<__m128>(a.v))}; }
template<typename V, typename W> BL_INLINE_NODEBUG V broadcast_f64(const W& a) noexcept { return V{I::simd_broadcast_f64<V::kW>(simd_cast<__m128d>(a.v))}; }

template<typename V> BL_INLINE_NODEBUG V dup_lo_u32(const V& a) noexcept { return V{I::simd_dup_lo_u32(a.v)}; }
template<typename V> BL_INLINE_NODEBUG V dup_hi_u32(const V& a) noexcept { return V{I::simd_dup_hi_u32(a.v)}; }
template<typename V> BL_INLINE_NODEBUG V dup_lo_u64(const V& a) noexcept { return V{I::simd_dup_lo_u64(a.v)}; }
template<typename V> BL_INLINE_NODEBUG V dup_hi_u64(const V& a) noexcept { return V{I::simd_dup_hi_u64(a.v)}; }

template<typename V> BL_INLINE_NODEBUG V dup_lo_f32(const V& a) noexcept { return V{I::simd_dup_lo_f32(a.v)}; }
template<typename V> BL_INLINE_NODEBUG V dup_hi_f32(const V& a) noexcept { return V{I::simd_dup_hi_f32(a.v)}; }
template<typename V> BL_INLINE_NODEBUG V dup_lo_f32x2(const V& a) noexcept { return V{I::simd_dup_lo_f32x2(a.v)}; }
template<typename V> BL_INLINE_NODEBUG V dup_hi_f32x2(const V& a) noexcept { return V{I::simd_dup_hi_f32x2(a.v)}; }
template<typename V> BL_INLINE_NODEBUG V dup_lo_f64(const V& a) noexcept { return V{I::simd_dup_lo_f64(a.v)}; }
template<typename V> BL_INLINE_NODEBUG V dup_hi_f64(const V& a) noexcept { return V{I::simd_dup_hi_f64(a.v)}; }

template<typename V> BL_INLINE_NODEBUG V swap_u32(const V& a) noexcept { return V{I::simd_swap_u32(a.v)}; }
template<typename V> BL_INLINE_NODEBUG V swap_u64(const V& a) noexcept { return V{I::simd_swap_u64(a.v)}; }
template<typename V> BL_INLINE_NODEBUG V swap_f32(const V& a) noexcept { return V{I::simd_swap_f32(a.v)}; }
template<typename V> BL_INLINE_NODEBUG V swap_f64(const V& a) noexcept { return V{I::simd_swap_f64(a.v)}; }

template<typename V> BL_INLINE_NODEBUG V interleave_lo_u8(const V& a, const V& b) noexcept { return V{I::simd_interleave_lo_u8(a.v, b.v)}; }
template<typename V> BL_INLINE_NODEBUG V interleave_hi_u8(const V& a, const V& b) noexcept { return V{I::simd_interleave_hi_u8(a.v, b.v)}; }
template<typename V> BL_INLINE_NODEBUG V interleave_lo_u16(const V& a, const V& b) noexcept { return V{I::simd_interleave_lo_u16(a.v, b.v)}; }
template<typename V> BL_INLINE_NODEBUG V interleave_hi_u16(const V& a, const V& b) noexcept { return V{I::simd_interleave_hi_u16(a.v, b.v)}; }
template<typename V> BL_INLINE_NODEBUG V interleave_lo_u32(const V& a, const V& b) noexcept { return V{I::simd_interleave_lo_u32(a.v, b.v)}; }
template<typename V> BL_INLINE_NODEBUG V interleave_hi_u32(const V& a, const V& b) noexcept { return V{I::simd_interleave_hi_u32(a.v, b.v)}; }
template<typename V> BL_INLINE_NODEBUG V interleave_lo_u64(const V& a, const V& b) noexcept { return V{I::simd_interleave_lo_u64(a.v, b.v)}; }
template<typename V> BL_INLINE_NODEBUG V interleave_hi_u64(const V& a, const V& b) noexcept { return V{I::simd_interleave_hi_u64(a.v, b.v)}; }

template<typename V> BL_INLINE_NODEBUG V interleave_lo_f32(const V& a, const V& b) noexcept { return V{I::simd_interleave_lo_f32(a.v, b.v)}; }
template<typename V> BL_INLINE_NODEBUG V interleave_hi_f32(const V& a, const V& b) noexcept { return V{I::simd_interleave_hi_f32(a.v, b.v)}; }
template<typename V> BL_INLINE_NODEBUG V interleave_lo_f64(const V& a, const V& b) noexcept { return V{I::simd_interleave_lo_f64(a.v, b.v)}; }
template<typename V> BL_INLINE_NODEBUG V interleave_hi_f64(const V& a, const V& b) noexcept { return V{I::simd_interleave_hi_f64(a.v, b.v)}; }

template<int kNumBytes, typename V>
BL_INLINE_NODEBUG V alignr_u128(const V& a, const V& b) noexcept { return V{I::simd_alignr_u128<kNumBytes>(a.v, b.v)}; }

// SIMD - Public - Integer Packing & Unpacking
// ===========================================

template<typename V> BL_INLINE_NODEBUG V packs_128_i16_i8(const V& a) noexcept { return V{I::simd_packs_128_i16_i8(a.v)}; }
template<typename V> BL_INLINE_NODEBUG V packs_128_i16_u8(const V& a) noexcept { return V{I::simd_packs_128_i16_u8(a.v)}; }
template<typename V> BL_INLINE_NODEBUG V packz_128_u16_u8(const V& a) noexcept { return V{I::simd_packz_128_u16_u8(a.v)}; }
template<typename V> BL_INLINE_NODEBUG V packs_128_i32_i8(const V& a) noexcept { return V{I::simd_packs_128_i32_i8(a.v)}; }
template<typename V> BL_INLINE_NODEBUG V packs_128_i32_u8(const V& a) noexcept { return V{I::simd_packs_128_i32_u8(a.v)}; }
template<typename V> BL_INLINE_NODEBUG V packs_128_i32_i16(const V& a) noexcept { return V{I::simd_packs_128_i32_i16(a.v)}; }
template<typename V> BL_INLINE_NODEBUG V packs_128_i32_u16(const V& a) noexcept { return V{I::simd_packs_128_i32_u16(a.v)}; }
template<typename V> BL_INLINE_NODEBUG V packz_128_u32_u8(const V& a) noexcept { return V{I::simd_packz_128_u32_u8(a.v)}; }
template<typename V> BL_INLINE_NODEBUG V packz_128_u32_u16(const V& a) noexcept { return V{I::simd_packz_128_u32_u16(a.v)}; }

template<typename V> BL_INLINE_NODEBUG V packs_128_i16_i8(const V& a, const V& b) noexcept { return V{I::simd_packs_128_i16_i8(a.v, b.v)}; }
template<typename V> BL_INLINE_NODEBUG V packs_128_i16_u8(const V& a, const V& b) noexcept { return V{I::simd_packs_128_i16_u8(a.v, b.v)}; }
template<typename V> BL_INLINE_NODEBUG V packz_128_u16_u8(const V& a, const V& b) noexcept { return V{I::simd_packz_128_u16_u8(a.v, b.v)}; }
template<typename V> BL_INLINE_NODEBUG V packs_128_i32_i8(const V& a, const V& b) noexcept { return V{I::simd_packs_128_i32_i8(a.v, b.v)}; }
template<typename V> BL_INLINE_NODEBUG V packs_128_i32_u8(const V& a, const V& b) noexcept { return V{I::simd_packs_128_i32_u8(a.v, b.v)}; }
template<typename V> BL_INLINE_NODEBUG V packs_128_i32_i16(const V& a, const V& b) noexcept { return V{I::simd_packs_128_i32_i16(a.v, b.v)}; }
template<typename V> BL_INLINE_NODEBUG V packs_128_i32_u16(const V& a, const V& b) noexcept { return V{I::simd_packs_128_i32_u16(a.v, b.v)}; }
template<typename V> BL_INLINE_NODEBUG V packz_128_u32_u8(const V& a, const V& b) noexcept { return V{I::simd_packz_128_u32_u8(a.v, b.v)}; }
template<typename V> BL_INLINE_NODEBUG V packz_128_u32_u16(const V& a, const V& b) noexcept { return V{I::simd_packz_128_u32_u16(a.v, b.v)}; }

template<typename V> BL_INLINE_NODEBUG V packs_128_i32_i8(const V& a, const V& b, const V& c, const V& d) noexcept { return V{I::simd_packs_128_i32_i8(a.v, b.v, c.v, d.v)}; }
template<typename V> BL_INLINE_NODEBUG V packs_128_i32_u8(const V& a, const V& b, const V& c, const V& d) noexcept { return V{I::simd_packs_128_i32_u8(a.v, b.v, c.v, d.v)}; }
template<typename V> BL_INLINE_NODEBUG V packz_128_u32_u8(const V& a, const V& b, const V& c, const V& d) noexcept { return V{I::simd_packz_128_u32_u8(a.v, b.v, c.v, d.v)}; }

template<typename V> BL_INLINE_NODEBUG V unpack_lo64_i8_i16(const V& a) noexcept { return V{I::simd_unpack_lo64_i8_i16(a.v)}; }
template<typename V> BL_INLINE_NODEBUG V unpack_lo64_u8_u16(const V& a) noexcept { return V{I::simd_unpack_lo64_u8_u16(a.v)}; }
template<typename V> BL_INLINE_NODEBUG V unpack_lo64_i16_i32(const V& a) noexcept { return V{I::simd_unpack_lo64_i16_i32(a.v)}; }
template<typename V> BL_INLINE_NODEBUG V unpack_lo64_u16_u32(const V& a) noexcept { return V{I::simd_unpack_lo64_u16_u32(a.v)}; }
template<typename V> BL_INLINE_NODEBUG V unpack_lo64_i32_i64(const V& a) noexcept { return V{I::simd_unpack_lo64_i32_i64(a.v)}; }
template<typename V> BL_INLINE_NODEBUG V unpack_lo64_u32_u64(const V& a) noexcept { return V{I::simd_unpack_lo64_u32_u64(a.v)}; }
template<typename V> BL_INLINE_NODEBUG V unpack_lo32_i8_i32(const V& a) noexcept { return V{I::simd_unpack_lo32_i8_i32(a.v)}; }
template<typename V> BL_INLINE_NODEBUG V unpack_lo32_u8_u32(const V& a) noexcept { return V{I::simd_unpack_lo32_u8_u32(a.v)}; }

template<typename V> BL_INLINE_NODEBUG V unpack_hi64_i8_i16(const V& a) noexcept { return V{I::simd_unpack_hi64_i8_i16(a.v)}; }
template<typename V> BL_INLINE_NODEBUG V unpack_hi64_u8_u16(const V& a) noexcept { return V{I::simd_unpack_hi64_u8_u16(a.v)}; }
template<typename V> BL_INLINE_NODEBUG V unpack_hi64_i16_i32(const V& a) noexcept { return V{I::simd_unpack_hi64_i16_i32(a.v)}; }
template<typename V> BL_INLINE_NODEBUG V unpack_hi64_u16_u32(const V& a) noexcept { return V{I::simd_unpack_hi64_u16_u32(a.v)}; }
template<typename V> BL_INLINE_NODEBUG V unpack_hi64_i32_i64(const V& a) noexcept { return V{I::simd_unpack_hi64_i32_i64(a.v)}; }
template<typename V> BL_INLINE_NODEBUG V unpack_hi64_u32_u64(const V& a) noexcept { return V{I::simd_unpack_hi64_u32_u64(a.v)}; }

#if defined(BL_SIMD_FEATURE_MOVW)
template<typename V> BL_INLINE_NODEBUG V movw_i8_i16(const V& a) noexcept { return V{I::simd_movw_i8_i16(a.v)}; }
template<typename V> BL_INLINE_NODEBUG V movw_i8_i32(const V& a) noexcept { return V{I::simd_movw_i8_i32(a.v)}; }
template<typename V> BL_INLINE_NODEBUG V movw_i8_i64(const V& a) noexcept { return V{I::simd_movw_i8_i64(a.v)}; }
template<typename V> BL_INLINE_NODEBUG V movw_i16_i32(const V& a) noexcept { return V{I::simd_movw_i16_i32(a.v)}; }
template<typename V> BL_INLINE_NODEBUG V movw_i16_i64(const V& a) noexcept { return V{I::simd_movw_i16_i64(a.v)}; }
template<typename V> BL_INLINE_NODEBUG V movw_i32_i64(const V& a) noexcept { return V{I::simd_movw_i32_i64(a.v)}; }

template<typename V> BL_INLINE_NODEBUG V movw_u8_u16(const V& a) noexcept { return V{I::simd_movw_u8_u16(a.v)}; }
template<typename V> BL_INLINE_NODEBUG V movw_u8_u32(const V& a) noexcept { return V{I::simd_movw_u8_u32(a.v)}; }
template<typename V> BL_INLINE_NODEBUG V movw_u8_u64(const V& a) noexcept { return V{I::simd_movw_u8_u64(a.v)}; }
template<typename V> BL_INLINE_NODEBUG V movw_u16_u32(const V& a) noexcept { return V{I::simd_movw_u16_u32(a.v)}; }
template<typename V> BL_INLINE_NODEBUG V movw_u16_u64(const V& a) noexcept { return V{I::simd_movw_u16_u64(a.v)}; }
template<typename V> BL_INLINE_NODEBUG V movw_u32_u64(const V& a) noexcept { return V{I::simd_movw_u32_u64(a.v)}; }
#endif // BL_SIMD_FEATURE_MOVW

// SIMD - Public - Arithmetic & Logical Operations
// ===============================================

#if defined(BL_TARGET_OPT_AVX512)
template<uint8_t kPred, size_t W, typename T>
BL_INLINE_NODEBUG Vec<W, T> simd_ternlog(const Vec<W, T>& a, const Vec<W, T>& b, const Vec<W, T>& c) noexcept { return Vec<W, T>{I::simd_ternlog(a.v, b.v, c.v)}; }
#endif // BL_TARGET_OPT_AVX512

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> not_(const Vec<W, T>& a) noexcept { return from_simd<Vec<W, T>>(I::simd_not(a.v)); }

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> and_(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return from_simd<Vec<W, T>>(I::simd_and(a.v, b.v)); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> andnot(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return from_simd<Vec<W, T>>(I::simd_andnot(a.v, b.v)); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> or_(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return from_simd<Vec<W, T>>(I::simd_or(a.v, b.v)); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> xor_(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return from_simd<Vec<W, T>>(I::simd_xor(a.v, b.v)); }

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> and_(const Vec<W, T>& a, const Vec<W, T>& b, const Vec<W, T>& c) noexcept { return and_(and_(a, b), c); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> or_(const Vec<W, T>& a, const Vec<W, T>& b, const Vec<W, T>& c) noexcept { return or_(or_(a, b), c); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> xor_(const Vec<W, T>& a, const Vec<W, T>& b, const Vec<W, T>& c) noexcept { return xor_(xor_(a, b), c); }

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> blendv_bits(const Vec<W, T>& a, const Vec<W, T>& b, const Vec<W, T>& msk) noexcept { return from_simd<Vec<W, T>>(I::simd_blendv_bits(a.v, b.v, msk.v)); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> blendv_u8(const Vec<W, T>& a, const Vec<W, T>& b, const Vec<W, T>& msk) noexcept { return from_simd<Vec<W, T>>(I::simd_blendv_u8(a.v, b.v, msk.v)); }

#if defined(BL_SIMD_FEATURE_BLEND_IMM)
template<uint32_t H, uint32_t G, uint32_t F, uint32_t E, uint32_t D, uint32_t C, uint32_t B, uint32_t A, typename V>
BL_INLINE_NODEBUG V blend_i16(const V& a, const V& b) noexcept { return V{I::simd_blend_i16<H, G, F, E, D, C, B, A>(a.v, b.v)}; }

template<uint32_t D, uint32_t C, uint32_t B, uint32_t A, typename V>
BL_INLINE_NODEBUG V blend_i32(const V& a, const V& b) noexcept { return V{I::simd_blend_i32<D, C, B, A>(a.v, b.v)}; }

template<uint32_t B, uint32_t A, typename V>
BL_INLINE_NODEBUG V blend_i64(const V& a, const V& b) noexcept { return V{I::simd_blend_i64<B, A>(a.v, b.v)}; }
#endif // BL_SIMD_FEATURE_BLEND_IMM

BL_INLINE_NODEBUG Vec4xF32 add_f32x1(const Vec4xF32& a, const Vec4xF32& b) noexcept { return Vec4xF32{_mm_add_ss(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec2xF64 add_f64x1(const Vec2xF64& a, const Vec2xF64& b) noexcept { return Vec2xF64{_mm_add_sd(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec4xF32 sub_f32x1(const Vec4xF32& a, const Vec4xF32& b) noexcept { return Vec4xF32{_mm_sub_ss(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec2xF64 sub_f64x1(const Vec2xF64& a, const Vec2xF64& b) noexcept { return Vec2xF64{_mm_sub_sd(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec4xF32 mul_f32x1(const Vec4xF32& a, const Vec4xF32& b) noexcept { return Vec4xF32{_mm_mul_ss(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec2xF64 mul_f64x1(const Vec2xF64& a, const Vec2xF64& b) noexcept { return Vec2xF64{_mm_mul_sd(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec4xF32 div_f32x1(const Vec4xF32& a, const Vec4xF32& b) noexcept { return Vec4xF32{_mm_div_ss(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec2xF64 div_f64x1(const Vec2xF64& a, const Vec2xF64& b) noexcept { return Vec2xF64{_mm_div_sd(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec4xF32 min_f32x1(const Vec4xF32& a, const Vec4xF32& b) noexcept { return Vec4xF32{_mm_min_ss(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec2xF64 min_f64x1(const Vec2xF64& a, const Vec2xF64& b) noexcept { return Vec2xF64{_mm_min_sd(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec4xF32 max_f32x1(const Vec4xF32& a, const Vec4xF32& b) noexcept { return Vec4xF32{_mm_max_ss(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec2xF64 max_f64x1(const Vec2xF64& a, const Vec2xF64& b) noexcept { return Vec2xF64{_mm_max_sd(a.v, b.v)}; }

BL_INLINE_NODEBUG Vec4xF32 sqrt_f32x1(const Vec4xF32& a) noexcept { return Vec4xF32{_mm_sqrt_ss(a.v)}; }
BL_INLINE_NODEBUG Vec2xF64 sqrt_f64x1(const Vec2xF64& a) noexcept { return Vec2xF64{_mm_sqrt_sd(a.v, a.v)}; }

BL_INLINE_NODEBUG Vec4xF32 cmp_eq_f32x1(const Vec4xF32& a, const Vec4xF32& b) noexcept { return Vec4xF32{_mm_cmpeq_ss(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec2xF64 cmp_eq_f64x1(const Vec2xF64& a, const Vec2xF64& b) noexcept { return Vec2xF64{_mm_cmpeq_sd(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec4xF32 cmp_ne_f32x1(const Vec4xF32& a, const Vec4xF32& b) noexcept { return Vec4xF32{_mm_cmpneq_ss(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec2xF64 cmp_ne_f64x1(const Vec2xF64& a, const Vec2xF64& b) noexcept { return Vec2xF64{_mm_cmpneq_sd(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec4xF32 cmp_ge_f32x1(const Vec4xF32& a, const Vec4xF32& b) noexcept { return Vec4xF32{_mm_cmpge_ss(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec2xF64 cmp_ge_f64x1(const Vec2xF64& a, const Vec2xF64& b) noexcept { return Vec2xF64{_mm_cmpge_sd(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec4xF32 cmp_gt_f32x1(const Vec4xF32& a, const Vec4xF32& b) noexcept { return Vec4xF32{_mm_cmpgt_ss(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec2xF64 cmp_gt_f64x1(const Vec2xF64& a, const Vec2xF64& b) noexcept { return Vec2xF64{_mm_cmpgt_sd(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec4xF32 cmp_le_f32x1(const Vec4xF32& a, const Vec4xF32& b) noexcept { return Vec4xF32{_mm_cmple_ss(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec2xF64 cmp_le_f64x1(const Vec2xF64& a, const Vec2xF64& b) noexcept { return Vec2xF64{_mm_cmple_sd(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec4xF32 cmp_lt_f32x1(const Vec4xF32& a, const Vec4xF32& b) noexcept { return Vec4xF32{_mm_cmplt_ss(a.v, b.v)}; }
BL_INLINE_NODEBUG Vec2xF64 cmp_lt_f64x1(const Vec2xF64& a, const Vec2xF64& b) noexcept { return Vec2xF64{_mm_cmplt_sd(a.v, b.v)}; }

template<typename V> BL_INLINE_NODEBUG V add_f32(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_add_f32(simd_as_f(a.v), simd_as_f(b.v))); }
template<typename V> BL_INLINE_NODEBUG V add_f64(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_add_f64(simd_as_d(a.v), simd_as_d(b.v))); }
template<typename V> BL_INLINE_NODEBUG V sub_f32(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_sub_f32(simd_as_f(a.v), simd_as_f(b.v))); }
template<typename V> BL_INLINE_NODEBUG V sub_f64(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_sub_f64(simd_as_d(a.v), simd_as_d(b.v))); }
template<typename V> BL_INLINE_NODEBUG V mul_f32(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_mul_f32(simd_as_f(a.v), simd_as_f(b.v))); }
template<typename V> BL_INLINE_NODEBUG V mul_f64(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_mul_f64(simd_as_d(a.v), simd_as_d(b.v))); }
template<typename V> BL_INLINE_NODEBUG V div_f32(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_div_f32(simd_as_f(a.v), simd_as_f(b.v))); }
template<typename V> BL_INLINE_NODEBUG V div_f64(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_div_f64(simd_as_d(a.v), simd_as_d(b.v))); }
template<typename V> BL_INLINE_NODEBUG V min_f32(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_min_f32(simd_as_f(a.v), simd_as_f(b.v))); }
template<typename V> BL_INLINE_NODEBUG V min_f64(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_min_f64(simd_as_d(a.v), simd_as_d(b.v))); }
template<typename V> BL_INLINE_NODEBUG V max_f32(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_max_f32(simd_as_f(a.v), simd_as_f(b.v))); }
template<typename V> BL_INLINE_NODEBUG V max_f64(const V& a, const V& b) noexcept { return from_simd<V>(I::simd_max_f64(simd_as_d(a.v), simd_as_d(b.v))); }

template<typename V> BL_INLINE_NODEBUG V cmp_eq_f32(const V& a, const V& b) noexcept { return V{I::simd_cmp_eq_f32(a.v, b.v)}; }
template<typename V> BL_INLINE_NODEBUG V cmp_eq_f64(const V& a, const V& b) noexcept { return V{I::simd_cmp_eq_f64(a.v, b.v)}; }
template<typename V> BL_INLINE_NODEBUG V cmp_ne_f32(const V& a, const V& b) noexcept { return V{I::simd_cmp_ne_f32(a.v, b.v)}; }
template<typename V> BL_INLINE_NODEBUG V cmp_ne_f64(const V& a, const V& b) noexcept { return V{I::simd_cmp_ne_f64(a.v, b.v)}; }
template<typename V> BL_INLINE_NODEBUG V cmp_ge_f32(const V& a, const V& b) noexcept { return V{I::simd_cmp_ge_f32(a.v, b.v)}; }
template<typename V> BL_INLINE_NODEBUG V cmp_ge_f64(const V& a, const V& b) noexcept { return V{I::simd_cmp_ge_f64(a.v, b.v)}; }
template<typename V> BL_INLINE_NODEBUG V cmp_gt_f32(const V& a, const V& b) noexcept { return V{I::simd_cmp_gt_f32(a.v, b.v)}; }
template<typename V> BL_INLINE_NODEBUG V cmp_gt_f64(const V& a, const V& b) noexcept { return V{I::simd_cmp_gt_f64(a.v, b.v)}; }
template<typename V> BL_INLINE_NODEBUG V cmp_le_f32(const V& a, const V& b) noexcept { return V{I::simd_cmp_le_f32(a.v, b.v)}; }
template<typename V> BL_INLINE_NODEBUG V cmp_le_f64(const V& a, const V& b) noexcept { return V{I::simd_cmp_le_f64(a.v, b.v)}; }
template<typename V> BL_INLINE_NODEBUG V cmp_lt_f32(const V& a, const V& b) noexcept { return V{I::simd_cmp_lt_f32(a.v, b.v)}; }
template<typename V> BL_INLINE_NODEBUG V cmp_lt_f64(const V& a, const V& b) noexcept { return V{I::simd_cmp_lt_f64(a.v, b.v)}; }

template<typename V> BL_INLINE_NODEBUG V abs_f32(const V& a) noexcept { return from_simd<V>(I::simd_abs_f32(simd_as_f(a.v))); }
template<typename V> BL_INLINE_NODEBUG V abs_f64(const V& a) noexcept { return from_simd<V>(I::simd_abs_f64(simd_as_d(a.v))); }

template<typename V> BL_INLINE_NODEBUG V sqrt_f32(const V& a) noexcept { return from_simd<V>(I::simd_sqrt_f32(simd_as_f(a.v))); }
template<typename V> BL_INLINE_NODEBUG V sqrt_f64(const V& a) noexcept { return from_simd<V>(I::simd_sqrt_f64(simd_as_d(a.v))); }

template<size_t W> BL_INLINE_NODEBUG Vec<W, float > add(const Vec<W, float >& a, const Vec<W, float >& b) noexcept { return Vec<W, float >{I::simd_add_f32(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, double> add(const Vec<W, double>& a, const Vec<W, double>& b) noexcept { return Vec<W, double>{I::simd_add_f64(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, float > sub(const Vec<W, float >& a, const Vec<W, float >& b) noexcept { return Vec<W, float >{I::simd_sub_f32(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, double> sub(const Vec<W, double>& a, const Vec<W, double>& b) noexcept { return Vec<W, double>{I::simd_sub_f64(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, float > mul(const Vec<W, float >& a, const Vec<W, float >& b) noexcept { return Vec<W, float >{I::simd_mul_f32(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, double> mul(const Vec<W, double>& a, const Vec<W, double>& b) noexcept { return Vec<W, double>{I::simd_mul_f64(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, float > div(const Vec<W, float >& a, const Vec<W, float >& b) noexcept { return Vec<W, float >{I::simd_div_f32(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, double> div(const Vec<W, double>& a, const Vec<W, double>& b) noexcept { return Vec<W, double>{I::simd_div_f64(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, float > min(const Vec<W, float >& a, const Vec<W, float >& b) noexcept { return Vec<W, float >{I::simd_min_f32(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, double> min(const Vec<W, double>& a, const Vec<W, double>& b) noexcept { return Vec<W, double>{I::simd_min_f64(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, float > max(const Vec<W, float >& a, const Vec<W, float >& b) noexcept { return Vec<W, float >{I::simd_max_f32(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, double> max(const Vec<W, double>& a, const Vec<W, double>& b) noexcept { return Vec<W, double>{I::simd_max_f64(a.v, b.v)}; }

template<size_t W> BL_INLINE_NODEBUG Vec<W, float > cmp_eq(const Vec<W, float >& a, const Vec<W, float >& b) noexcept { return Vec<W, float >{I::simd_cmp_eq_f32(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, double> cmp_eq(const Vec<W, double>& a, const Vec<W, double>& b) noexcept { return Vec<W, double>{I::simd_cmp_eq_f64(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, float > cmp_ne(const Vec<W, float >& a, const Vec<W, float >& b) noexcept { return Vec<W, float >{I::simd_cmp_ne_f32(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, double> cmp_ne(const Vec<W, double>& a, const Vec<W, double>& b) noexcept { return Vec<W, double>{I::simd_cmp_ne_f64(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, float > cmp_ge(const Vec<W, float >& a, const Vec<W, float >& b) noexcept { return Vec<W, float >{I::simd_cmp_ge_f32(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, double> cmp_ge(const Vec<W, double>& a, const Vec<W, double>& b) noexcept { return Vec<W, double>{I::simd_cmp_ge_f64(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, float > cmp_gt(const Vec<W, float >& a, const Vec<W, float >& b) noexcept { return Vec<W, float >{I::simd_cmp_gt_f32(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, double> cmp_gt(const Vec<W, double>& a, const Vec<W, double>& b) noexcept { return Vec<W, double>{I::simd_cmp_gt_f64(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, float > cmp_le(const Vec<W, float >& a, const Vec<W, float >& b) noexcept { return Vec<W, float >{I::simd_cmp_le_f32(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, double> cmp_le(const Vec<W, double>& a, const Vec<W, double>& b) noexcept { return Vec<W, double>{I::simd_cmp_le_f64(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, float > cmp_lt(const Vec<W, float >& a, const Vec<W, float >& b) noexcept { return Vec<W, float >{I::simd_cmp_lt_f32(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, double> cmp_lt(const Vec<W, double>& a, const Vec<W, double>& b) noexcept { return Vec<W, double>{I::simd_cmp_lt_f64(a.v, b.v)}; }

template<size_t W> BL_INLINE_NODEBUG Vec<W, float > abs(const Vec<W, float >& a) noexcept { return Vec<W, float >{I::simd_abs_f32(a.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, double> abs(const Vec<W, double>& a) noexcept { return Vec<W, double>{I::simd_abs_f64(a.v)}; }

template<size_t W> BL_INLINE_NODEBUG Vec<W, float > sqrt(const Vec<W, float >& a) noexcept { return Vec<W, float >{I::simd_sqrt_f32(a.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, double> sqrt(const Vec<W, double>& a) noexcept { return Vec<W, double>{I::simd_sqrt_f64(a.v)}; }

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> abs_i8(const Vec<W, T>& a) noexcept { return from_simd<Vec<W, T>>(I::simd_abs_i8(simd_as_i(a.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> abs_i16(const Vec<W, T>& a) noexcept { return from_simd<Vec<W, T>>(I::simd_abs_i16(simd_as_i(a.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> abs_i32(const Vec<W, T>& a) noexcept { return from_simd<Vec<W, T>>(I::simd_abs_i32(simd_as_i(a.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> abs_i64(const Vec<W, T>& a) noexcept { return from_simd<Vec<W, T>>(I::simd_abs_i64(simd_as_i(a.v))); }

template<size_t W> BL_INLINE_NODEBUG Vec<W, int8_t> abs(const Vec<W, int8_t>& a) noexcept { return Vec<W, int8_t>{I::simd_abs_i8(simd_as_i(a.v))}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> abs(const Vec<W, int16_t>& a) noexcept { return Vec<W, int16_t>{I::simd_abs_i16(simd_as_i(a.v))}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int32_t> abs(const Vec<W, int32_t>& a) noexcept { return Vec<W, int32_t>{I::simd_abs_i32(simd_as_i(a.v))}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int64_t> abs(const Vec<W, int64_t>& a) noexcept { return Vec<W, int64_t>{I::simd_abs_i64(simd_as_i(a.v))}; }

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> add_i8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_add_i8(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> add_i16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_add_i16(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> add_i32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_add_i32(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> add_i64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_add_i64(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> add_u8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_add_u8(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> add_u16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_add_u16(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> add_u32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_add_u32(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> add_u64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_add_u64(a.v, b.v)}; }

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> adds_i8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_adds_i8(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> adds_i16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_adds_i16(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> adds_u8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_adds_u8(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> adds_u16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_adds_u16(a.v, b.v)}; }

template<size_t W> BL_INLINE_NODEBUG Vec<W, int8_t> add(const Vec<W, int8_t>& a, const Vec<W, int8_t>& b) noexcept { return Vec<W, int8_t>{I::simd_add_i8(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> add(const Vec<W, int16_t>& a, const Vec<W, int16_t>& b) noexcept { return Vec<W, int16_t>{I::simd_add_i16(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int32_t> add(const Vec<W, int32_t>& a, const Vec<W, int32_t>& b) noexcept { return Vec<W, int32_t>{I::simd_add_i32(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int64_t> add(const Vec<W, int64_t>& a, const Vec<W, int64_t>& b) noexcept { return Vec<W, int64_t>{I::simd_add_i64(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint8_t> add(const Vec<W, uint8_t>& a, const Vec<W, uint8_t>& b) noexcept { return Vec<W, uint8_t>{I::simd_add_u8(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint16_t> add(const Vec<W, uint16_t>& a, const Vec<W, uint16_t>& b) noexcept { return Vec<W, uint16_t>{I::simd_add_u16(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint32_t> add(const Vec<W, uint32_t>& a, const Vec<W, uint32_t>& b) noexcept { return Vec<W, uint32_t>{I::simd_add_u32(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint64_t> add(const Vec<W, uint64_t>& a, const Vec<W, uint64_t>& b) noexcept { return Vec<W, uint64_t>{I::simd_add_u64(a.v, b.v)}; }

template<size_t W> BL_INLINE_NODEBUG Vec<W, int8_t> adds(const Vec<W, int8_t>& a, const Vec<W, int8_t>& b) noexcept { return Vec<W, int8_t>{I::simd_adds_i8(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> adds(const Vec<W, int16_t>& a, const Vec<W, int16_t>& b) noexcept { return Vec<W, int16_t>{I::simd_adds_i16(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint8_t> adds(const Vec<W, uint8_t>& a, const Vec<W, uint8_t>& b) noexcept { return Vec<W, uint8_t>{I::simd_adds_u8(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint16_t> adds(const Vec<W, uint16_t>& a, const Vec<W, uint16_t>& b) noexcept { return Vec<W, uint16_t>{I::simd_adds_u16(a.v, b.v)}; }

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> sub_i8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_sub_i8(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> sub_i16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_sub_i16(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> sub_i32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_sub_i32(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> sub_i64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_sub_i64(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> sub_u8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_sub_u8(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> sub_u16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_sub_u16(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> sub_u32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_sub_u32(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> sub_u64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_sub_u64(a.v, b.v)}; }

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> subs_i8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_subs_i8(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> subs_i16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_subs_i16(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> subs_u8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_subs_u8(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> subs_u16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_subs_u16(a.v, b.v)}; }

template<size_t W> BL_INLINE_NODEBUG Vec<W, int8_t> sub(const Vec<W, int8_t>& a, const Vec<W, int8_t>& b) noexcept { return Vec<W, int8_t>{I::simd_sub_i8(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> sub(const Vec<W, int16_t>& a, const Vec<W, int16_t>& b) noexcept { return Vec<W, int16_t>{I::simd_sub_i16(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int32_t> sub(const Vec<W, int32_t>& a, const Vec<W, int32_t>& b) noexcept { return Vec<W, int32_t>{I::simd_sub_i32(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int64_t> sub(const Vec<W, int64_t>& a, const Vec<W, int64_t>& b) noexcept { return Vec<W, int64_t>{I::simd_sub_i64(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint8_t> sub(const Vec<W, uint8_t>& a, const Vec<W, uint8_t>& b) noexcept { return Vec<W, uint8_t>{I::simd_sub_u8(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint16_t> sub(const Vec<W, uint16_t>& a, const Vec<W, uint16_t>& b) noexcept { return Vec<W, uint16_t>{I::simd_sub_u16(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint32_t> sub(const Vec<W, uint32_t>& a, const Vec<W, uint32_t>& b) noexcept { return Vec<W, uint32_t>{I::simd_sub_u32(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint64_t> sub(const Vec<W, uint64_t>& a, const Vec<W, uint64_t>& b) noexcept { return Vec<W, uint64_t>{I::simd_sub_u64(a.v, b.v)}; }

template<size_t W> BL_INLINE_NODEBUG Vec<W, int8_t> subs(const Vec<W, int8_t>& a, const Vec<W, int8_t>& b) noexcept { return Vec<W, int8_t>{I::simd_subs_i8(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> subs(const Vec<W, int16_t>& a, const Vec<W, int16_t>& b) noexcept { return Vec<W, int16_t>{I::simd_subs_i16(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint8_t> subs(const Vec<W, uint8_t>& a, const Vec<W, uint8_t>& b) noexcept { return Vec<W, uint8_t>{I::simd_subs_u8(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint16_t> subs(const Vec<W, uint16_t>& a, const Vec<W, uint16_t>& b) noexcept { return Vec<W, uint16_t>{I::simd_subs_u16(a.v, b.v)}; }

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> mul_i16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_mul_i16(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> mul_i32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_mul_i32(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> mul_i64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_mul_i64(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> mul_u16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_mul_u16(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> mul_u32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_mul_u32(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> mul_u64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_mul_u64(a.v, b.v)}; }

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> mulh_i16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_mulh_i16(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> mulh_u16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_mulh_u16(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> mulw_u32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_mulw_u32(a.v, b.v)}; }

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> maddw_i16_i32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_maddw_i16_i32(a.v, b.v)}; }

template<size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> mul(const Vec<W, int16_t>& a, const Vec<W, int16_t>& b) noexcept { return Vec<W, int16_t>{I::simd_mul_i16(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int32_t> mul(const Vec<W, int32_t>& a, const Vec<W, int32_t>& b) noexcept { return Vec<W, int32_t>{I::simd_mul_i32(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int64_t> mul(const Vec<W, int64_t>& a, const Vec<W, int64_t>& b) noexcept { return Vec<W, int64_t>{I::simd_mul_i64(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint16_t> mul(const Vec<W, uint16_t>& a, const Vec<W, uint16_t>& b) noexcept { return Vec<W, uint16_t>{I::simd_mul_u16(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint32_t> mul(const Vec<W, uint32_t>& a, const Vec<W, uint32_t>& b) noexcept { return Vec<W, uint32_t>{I::simd_mul_u32(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint64_t> mul(const Vec<W, uint64_t>& a, const Vec<W, uint64_t>& b) noexcept { return Vec<W, uint64_t>{I::simd_mul_u64(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> mulh(const Vec<W, int16_t>& a, const Vec<W, int16_t>& b) noexcept { return Vec<W, int16_t>{I::simd_mulh_i16(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint16_t> mulh(const Vec<W, uint16_t>& a, const Vec<W, uint16_t>& b) noexcept { return Vec<W, uint16_t>{I::simd_mulh_u16(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint64_t> mulw(const Vec<W, uint32_t>& a, const Vec<W, uint32_t>& b) noexcept { return Vec<W, uint64_t>{I::simd_mulw_u32(a.v, b.v)}; }

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_eq_i8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_eq_i8(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_eq_i16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_eq_i16(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_eq_i32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_eq_i32(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_eq_i64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_eq_i64(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_eq_u8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_eq_i8(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_eq_u16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_eq_i16(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_eq_u32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_eq_i32(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_eq_u64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_eq_i64(a.v, b.v)}; }

template<size_t W> BL_INLINE_NODEBUG Vec<W, int8_t> cmp_eq(const Vec<W, int8_t>& a, const Vec<W, int8_t>& b) noexcept { return Vec<W, int8_t>{I::simd_cmp_eq_i8(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> cmp_eq(const Vec<W, int16_t>& a, const Vec<W, int16_t>& b) noexcept { return Vec<W, int16_t>{I::simd_cmp_eq_i16(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int32_t> cmp_eq(const Vec<W, int32_t>& a, const Vec<W, int32_t>& b) noexcept { return Vec<W, int32_t>{I::simd_cmp_eq_i32(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int64_t> cmp_eq(const Vec<W, int64_t>& a, const Vec<W, int64_t>& b) noexcept { return Vec<W, int64_t>{I::simd_cmp_eq_i64(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint8_t> cmp_eq(const Vec<W, uint8_t>& a, const Vec<W, uint8_t>& b) noexcept { return Vec<W, uint8_t>{I::simd_cmp_eq_i8(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint16_t> cmp_eq(const Vec<W, uint16_t>& a, const Vec<W, uint16_t>& b) noexcept { return Vec<W, uint16_t>{I::simd_cmp_eq_i16(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint32_t> cmp_eq(const Vec<W, uint32_t>& a, const Vec<W, uint32_t>& b) noexcept { return Vec<W, uint32_t>{I::simd_cmp_eq_i32(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint64_t> cmp_eq(const Vec<W, uint64_t>& a, const Vec<W, uint64_t>& b) noexcept { return Vec<W, uint64_t>{I::simd_cmp_eq_i64(a.v, b.v)}; }

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_ne_i8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_ne_i8(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_ne_i16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_ne_i16(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_ne_i32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_ne_i32(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_ne_i64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_ne_i64(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_ne_u8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_ne_i8(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_ne_u16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_ne_i16(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_ne_u32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_ne_i32(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_ne_u64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_ne_i64(a.v, b.v)}; }

template<size_t W> BL_INLINE_NODEBUG Vec<W, int8_t> cmp_ne(const Vec<W, int8_t>& a, const Vec<W, int8_t>& b) noexcept { return Vec<W, int8_t>{I::simd_cmp_ne_i8(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> cmp_ne(const Vec<W, int16_t>& a, const Vec<W, int16_t>& b) noexcept { return Vec<W, int16_t>{I::simd_cmp_ne_i16(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int32_t> cmp_ne(const Vec<W, int32_t>& a, const Vec<W, int32_t>& b) noexcept { return Vec<W, int32_t>{I::simd_cmp_ne_i32(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int64_t> cmp_ne(const Vec<W, int64_t>& a, const Vec<W, int64_t>& b) noexcept { return Vec<W, int64_t>{I::simd_cmp_ne_i64(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint8_t> cmp_ne(const Vec<W, uint8_t>& a, const Vec<W, uint8_t>& b) noexcept { return Vec<W, uint8_t>{I::simd_cmp_ne_i8(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint16_t> cmp_ne(const Vec<W, uint16_t>& a, const Vec<W, uint16_t>& b) noexcept { return Vec<W, uint16_t>{I::simd_cmp_ne_i16(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint32_t> cmp_ne(const Vec<W, uint32_t>& a, const Vec<W, uint32_t>& b) noexcept { return Vec<W, uint32_t>{I::simd_cmp_ne_i32(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint64_t> cmp_ne(const Vec<W, uint64_t>& a, const Vec<W, uint64_t>& b) noexcept { return Vec<W, uint64_t>{I::simd_cmp_ne_i64(a.v, b.v)}; }

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_gt_i8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_gt_i8(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_gt_i16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_gt_i16(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_gt_i32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_gt_i32(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_gt_i64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_gt_i64(a.v, b.v)}; }

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_gt_u8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_gt_u8(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_gt_u16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_gt_u16(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_gt_u32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_gt_u32(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_gt_u64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_gt_u64(a.v, b.v)}; }

template<size_t W> BL_INLINE_NODEBUG Vec<W, int8_t> cmp_gt(const Vec<W, int8_t>& a, const Vec<W, int8_t>& b) noexcept { return Vec<W, int8_t>{I::simd_cmp_gt_i8(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> cmp_gt(const Vec<W, int16_t>& a, const Vec<W, int16_t>& b) noexcept { return Vec<W, int16_t>{I::simd_cmp_gt_i16(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int32_t> cmp_gt(const Vec<W, int32_t>& a, const Vec<W, int32_t>& b) noexcept { return Vec<W, int32_t>{I::simd_cmp_gt_i32(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int64_t> cmp_gt(const Vec<W, int64_t>& a, const Vec<W, int64_t>& b) noexcept { return Vec<W, int64_t>{I::simd_cmp_gt_i64(a.v, b.v)}; }

template<size_t W> BL_INLINE_NODEBUG Vec<W, uint8_t> cmp_gt(const Vec<W, uint8_t>& a, const Vec<W, uint8_t>& b) noexcept { return Vec<W, uint8_t>{I::simd_cmp_gt_u8(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint16_t> cmp_gt(const Vec<W, uint16_t>& a, const Vec<W, uint16_t>& b) noexcept { return Vec<W, uint16_t>{I::simd_cmp_gt_u16(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint32_t> cmp_gt(const Vec<W, uint32_t>& a, const Vec<W, uint32_t>& b) noexcept { return Vec<W, uint32_t>{I::simd_cmp_gt_u32(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint64_t> cmp_gt(const Vec<W, uint64_t>& a, const Vec<W, uint64_t>& b) noexcept { return Vec<W, uint64_t>{I::simd_cmp_gt_u64(a.v, b.v)}; }

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_ge_i8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_ge_i8(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_ge_i16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_ge_i16(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_ge_i32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_ge_i32(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_ge_i64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_ge_i64(a.v, b.v)}; }

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_ge_u8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_ge_u8(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_ge_u16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_ge_u16(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_ge_u32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_ge_u32(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_ge_u64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_ge_u64(a.v, b.v)}; }

template<size_t W> BL_INLINE_NODEBUG Vec<W, int8_t> cmp_ge(const Vec<W, int8_t>& a, const Vec<W, int8_t>& b) noexcept { return Vec<W, int8_t>{I::simd_cmp_ge_i8(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> cmp_ge(const Vec<W, int16_t>& a, const Vec<W, int16_t>& b) noexcept { return Vec<W, int16_t>{I::simd_cmp_ge_i16(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int32_t> cmp_ge(const Vec<W, int32_t>& a, const Vec<W, int32_t>& b) noexcept { return Vec<W, int32_t>{I::simd_cmp_ge_i32(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int64_t> cmp_ge(const Vec<W, int64_t>& a, const Vec<W, int64_t>& b) noexcept { return Vec<W, int64_t>{I::simd_cmp_ge_i64(a.v, b.v)}; }

template<size_t W> BL_INLINE_NODEBUG Vec<W, uint8_t> cmp_ge(const Vec<W, uint8_t>& a, const Vec<W, uint8_t>& b) noexcept { return Vec<W, uint8_t>{I::simd_cmp_ge_u8(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint16_t> cmp_ge(const Vec<W, uint16_t>& a, const Vec<W, uint16_t>& b) noexcept { return Vec<W, uint16_t>{I::simd_cmp_ge_u16(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint32_t> cmp_ge(const Vec<W, uint32_t>& a, const Vec<W, uint32_t>& b) noexcept { return Vec<W, uint32_t>{I::simd_cmp_ge_u32(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint64_t> cmp_ge(const Vec<W, uint64_t>& a, const Vec<W, uint64_t>& b) noexcept { return Vec<W, uint64_t>{I::simd_cmp_ge_u64(a.v, b.v)}; }

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_lt_i8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_lt_i8(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_lt_i16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_lt_i16(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_lt_i32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_lt_i32(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_lt_i64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_lt_i64(a.v, b.v)}; }

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_lt_u8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_lt_u8(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_lt_u16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_lt_u16(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_lt_u32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_lt_u32(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_lt_u64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_lt_u64(a.v, b.v)}; }

template<size_t W> BL_INLINE_NODEBUG Vec<W, int8_t> cmp_lt(const Vec<W, int8_t>& a, const Vec<W, int8_t>& b) noexcept { return Vec<W, int8_t>{I::simd_cmp_lt_i8(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> cmp_lt(const Vec<W, int16_t>& a, const Vec<W, int16_t>& b) noexcept { return Vec<W, int16_t>{I::simd_cmp_lt_i16(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int32_t> cmp_lt(const Vec<W, int32_t>& a, const Vec<W, int32_t>& b) noexcept { return Vec<W, int32_t>{I::simd_cmp_lt_i32(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int64_t> cmp_lt(const Vec<W, int64_t>& a, const Vec<W, int64_t>& b) noexcept { return Vec<W, int64_t>{I::simd_cmp_lt_i64(a.v, b.v)}; }

template<size_t W> BL_INLINE_NODEBUG Vec<W, uint8_t> cmp_lt(const Vec<W, uint8_t>& a, const Vec<W, uint8_t>& b) noexcept { return Vec<W, uint8_t>{I::simd_cmp_lt_u8(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint16_t> cmp_lt(const Vec<W, uint16_t>& a, const Vec<W, uint16_t>& b) noexcept { return Vec<W, uint16_t>{I::simd_cmp_lt_u16(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint32_t> cmp_lt(const Vec<W, uint32_t>& a, const Vec<W, uint32_t>& b) noexcept { return Vec<W, uint32_t>{I::simd_cmp_lt_u32(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint64_t> cmp_lt(const Vec<W, uint64_t>& a, const Vec<W, uint64_t>& b) noexcept { return Vec<W, uint64_t>{I::simd_cmp_lt_u64(a.v, b.v)}; }

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_le_i8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_le_i8(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_le_i16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_le_i16(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_le_i32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_le_i32(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_le_i64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_le_i64(a.v, b.v)}; }

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_le_u8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_le_u8(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_le_u16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_le_u16(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_le_u32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_le_u32(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> cmp_le_u64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_cmp_le_u64(a.v, b.v)}; }

template<size_t W> BL_INLINE_NODEBUG Vec<W, int8_t> cmp_le(const Vec<W, int8_t>& a, const Vec<W, int8_t>& b) noexcept { return Vec<W, int8_t>{I::simd_cmp_le_i8(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> cmp_le(const Vec<W, int16_t>& a, const Vec<W, int16_t>& b) noexcept { return Vec<W, int16_t>{I::simd_cmp_le_i16(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int32_t> cmp_le(const Vec<W, int32_t>& a, const Vec<W, int32_t>& b) noexcept { return Vec<W, int32_t>{I::simd_cmp_le_i32(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int64_t> cmp_le(const Vec<W, int64_t>& a, const Vec<W, int64_t>& b) noexcept { return Vec<W, int64_t>{I::simd_cmp_le_i64(a.v, b.v)}; }

template<size_t W> BL_INLINE_NODEBUG Vec<W, uint8_t> cmp_le(const Vec<W, uint8_t>& a, const Vec<W, uint8_t>& b) noexcept { return Vec<W, uint8_t>{I::simd_cmp_le_u8(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint16_t> cmp_le(const Vec<W, uint16_t>& a, const Vec<W, uint16_t>& b) noexcept { return Vec<W, uint16_t>{I::simd_cmp_le_u16(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint32_t> cmp_le(const Vec<W, uint32_t>& a, const Vec<W, uint32_t>& b) noexcept { return Vec<W, uint32_t>{I::simd_cmp_le_u32(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint64_t> cmp_le(const Vec<W, uint64_t>& a, const Vec<W, uint64_t>& b) noexcept { return Vec<W, uint64_t>{I::simd_cmp_le_u64(a.v, b.v)}; }

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> min_i8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_min_i8(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> min_i16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_min_i16(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> min_i32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_min_i32(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> min_i64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_min_i64(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> min_u8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_min_u8(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> min_u16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_min_u16(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> min_u32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_min_u32(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> min_u64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_min_u64(a.v, b.v)}; }

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> max_i8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_max_i8(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> max_i16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_max_i16(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> max_i32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_max_i32(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> max_i64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_max_i64(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> max_u8(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_max_u8(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> max_u16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_max_u16(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> max_u32(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_max_u32(a.v, b.v)}; }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> max_u64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return Vec<W, T>{I::simd_max_u64(a.v, b.v)}; }

template<size_t W> BL_INLINE_NODEBUG Vec<W, int8_t> min(const Vec<W, int8_t>& a, const Vec<W, int8_t>& b) noexcept { return Vec<W, int8_t>{I::simd_min_i8(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> min(const Vec<W, int16_t>& a, const Vec<W, int16_t>& b) noexcept { return Vec<W, int16_t>{I::simd_min_i16(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int32_t> min(const Vec<W, int32_t>& a, const Vec<W, int32_t>& b) noexcept { return Vec<W, int32_t>{I::simd_min_i32(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int64_t> min(const Vec<W, int64_t>& a, const Vec<W, int64_t>& b) noexcept { return Vec<W, int64_t>{I::simd_min_i64(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint8_t> min(const Vec<W, uint8_t>& a, const Vec<W, uint8_t>& b) noexcept { return Vec<W, uint8_t>{I::simd_min_u8(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint16_t> min(const Vec<W, uint16_t>& a, const Vec<W, uint16_t>& b) noexcept { return Vec<W, uint16_t>{I::simd_min_u16(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint32_t> min(const Vec<W, uint32_t>& a, const Vec<W, uint32_t>& b) noexcept { return Vec<W, uint32_t>{I::simd_min_u32(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint64_t> min(const Vec<W, uint64_t>& a, const Vec<W, uint64_t>& b) noexcept { return Vec<W, uint64_t>{I::simd_min_u64(a.v, b.v)}; }

template<size_t W> BL_INLINE_NODEBUG Vec<W, int8_t> max(const Vec<W, int8_t>& a, const Vec<W, int8_t>& b) noexcept { return Vec<W, int8_t>{I::simd_max_i8(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> max(const Vec<W, int16_t>& a, const Vec<W, int16_t>& b) noexcept { return Vec<W, int16_t>{I::simd_max_i16(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int32_t> max(const Vec<W, int32_t>& a, const Vec<W, int32_t>& b) noexcept { return Vec<W, int32_t>{I::simd_max_i32(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int64_t> max(const Vec<W, int64_t>& a, const Vec<W, int64_t>& b) noexcept { return Vec<W, int64_t>{I::simd_max_i64(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint8_t> max(const Vec<W, uint8_t>& a, const Vec<W, uint8_t>& b) noexcept { return Vec<W, uint8_t>{I::simd_max_u8(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint16_t> max(const Vec<W, uint16_t>& a, const Vec<W, uint16_t>& b) noexcept { return Vec<W, uint16_t>{I::simd_max_u16(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint32_t> max(const Vec<W, uint32_t>& a, const Vec<W, uint32_t>& b) noexcept { return Vec<W, uint32_t>{I::simd_max_u32(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint64_t> max(const Vec<W, uint64_t>& a, const Vec<W, uint64_t>& b) noexcept { return Vec<W, uint64_t>{I::simd_max_u64(a.v, b.v)}; }

template<size_t W> BL_INLINE_NODEBUG Vec<W, int8_t> smin(const Vec<W, int8_t>& a, const Vec<W, int8_t>& b) noexcept { return Vec<W, int8_t>{I::simd_min_i8(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> smin(const Vec<W, int16_t>& a, const Vec<W, int16_t>& b) noexcept { return Vec<W, int16_t>{I::simd_min_i16(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int32_t> smin(const Vec<W, int32_t>& a, const Vec<W, int32_t>& b) noexcept { return Vec<W, int32_t>{I::simd_min_i32(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int64_t> smin(const Vec<W, int64_t>& a, const Vec<W, int64_t>& b) noexcept { return Vec<W, int64_t>{I::simd_min_i64(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint8_t> smin(const Vec<W, uint8_t>& a, const Vec<W, uint8_t>& b) noexcept { return Vec<W, uint8_t>{I::simd_min_i8(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint16_t> smin(const Vec<W, uint16_t>& a, const Vec<W, uint16_t>& b) noexcept { return Vec<W, uint16_t>{I::simd_min_i16(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint32_t> smin(const Vec<W, uint32_t>& a, const Vec<W, uint32_t>& b) noexcept { return Vec<W, uint32_t>{I::simd_min_i32(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint64_t> smin(const Vec<W, uint64_t>& a, const Vec<W, uint64_t>& b) noexcept { return Vec<W, uint64_t>{I::simd_min_i64(a.v, b.v)}; }

template<size_t W> BL_INLINE_NODEBUG Vec<W, int8_t> smax(const Vec<W, int8_t>& a, const Vec<W, int8_t>& b) noexcept { return Vec<W, int8_t>{I::simd_max_i8(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> smax(const Vec<W, int16_t>& a, const Vec<W, int16_t>& b) noexcept { return Vec<W, int16_t>{I::simd_max_i16(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int32_t> smax(const Vec<W, int32_t>& a, const Vec<W, int32_t>& b) noexcept { return Vec<W, int32_t>{I::simd_max_i32(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int64_t> smax(const Vec<W, int64_t>& a, const Vec<W, int64_t>& b) noexcept { return Vec<W, int64_t>{I::simd_max_i64(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint8_t> smax(const Vec<W, uint8_t>& a, const Vec<W, uint8_t>& b) noexcept { return Vec<W, uint8_t>{I::simd_max_i8(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint16_t> smax(const Vec<W, uint16_t>& a, const Vec<W, uint16_t>& b) noexcept { return Vec<W, uint16_t>{I::simd_max_i16(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint32_t> smax(const Vec<W, uint32_t>& a, const Vec<W, uint32_t>& b) noexcept { return Vec<W, uint32_t>{I::simd_max_i32(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint64_t> smax(const Vec<W, uint64_t>& a, const Vec<W, uint64_t>& b) noexcept { return Vec<W, uint64_t>{I::simd_max_i64(a.v, b.v)}; }

template<size_t W> BL_INLINE_NODEBUG Vec<W, int8_t> umin(const Vec<W, int8_t>& a, const Vec<W, int8_t>& b) noexcept { return Vec<W, int8_t>{I::simd_min_u8(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> umin(const Vec<W, int16_t>& a, const Vec<W, int16_t>& b) noexcept { return Vec<W, int16_t>{I::simd_min_u16(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int32_t> umin(const Vec<W, int32_t>& a, const Vec<W, int32_t>& b) noexcept { return Vec<W, int32_t>{I::simd_min_u32(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int64_t> umin(const Vec<W, int64_t>& a, const Vec<W, int64_t>& b) noexcept { return Vec<W, int64_t>{I::simd_min_u64(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint8_t> umin(const Vec<W, uint8_t>& a, const Vec<W, uint8_t>& b) noexcept { return Vec<W, uint8_t>{I::simd_min_u8(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint16_t> umin(const Vec<W, uint16_t>& a, const Vec<W, uint16_t>& b) noexcept { return Vec<W, uint16_t>{I::simd_min_u16(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint32_t> umin(const Vec<W, uint32_t>& a, const Vec<W, uint32_t>& b) noexcept { return Vec<W, uint32_t>{I::simd_min_u32(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint64_t> umin(const Vec<W, uint64_t>& a, const Vec<W, uint64_t>& b) noexcept { return Vec<W, uint64_t>{I::simd_min_u64(a.v, b.v)}; }

template<size_t W> BL_INLINE_NODEBUG Vec<W, int8_t> umax(const Vec<W, int8_t>& a, const Vec<W, int8_t>& b) noexcept { return Vec<W, int8_t>{I::simd_max_u8(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> umax(const Vec<W, int16_t>& a, const Vec<W, int16_t>& b) noexcept { return Vec<W, int16_t>{I::simd_max_u16(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int32_t> umax(const Vec<W, int32_t>& a, const Vec<W, int32_t>& b) noexcept { return Vec<W, int32_t>{I::simd_max_u32(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, int64_t> umax(const Vec<W, int64_t>& a, const Vec<W, int64_t>& b) noexcept { return Vec<W, int64_t>{I::simd_max_u64(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint8_t> umax(const Vec<W, uint8_t>& a, const Vec<W, uint8_t>& b) noexcept { return Vec<W, uint8_t>{I::simd_max_u8(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint16_t> umax(const Vec<W, uint16_t>& a, const Vec<W, uint16_t>& b) noexcept { return Vec<W, uint16_t>{I::simd_max_u16(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint32_t> umax(const Vec<W, uint32_t>& a, const Vec<W, uint32_t>& b) noexcept { return Vec<W, uint32_t>{I::simd_max_u32(a.v, b.v)}; }
template<size_t W> BL_INLINE_NODEBUG Vec<W, uint64_t> umax(const Vec<W, uint64_t>& a, const Vec<W, uint64_t>& b) noexcept { return Vec<W, uint64_t>{I::simd_max_u64(a.v, b.v)}; }

template<uint32_t kN, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> slli_i8(const Vec<W, T>& a) noexcept { return from_simd<Vec<W, T>>(I::simd_slli_i8<kN>(simd_as_i(a.v))); }
template<uint32_t kN, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> slli_i16(const Vec<W, T>& a) noexcept { return from_simd<Vec<W, T>>(I::simd_slli_i16<kN>(simd_as_i(a.v))); }
template<uint32_t kN, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> slli_i32(const Vec<W, T>& a) noexcept { return from_simd<Vec<W, T>>(I::simd_slli_i32<kN>(simd_as_i(a.v))); }
template<uint32_t kN, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> slli_i64(const Vec<W, T>& a) noexcept { return from_simd<Vec<W, T>>(I::simd_slli_i64<kN>(simd_as_i(a.v))); }

template<uint32_t kN, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> slli_u8(const Vec<W, T>& a) noexcept { return from_simd<Vec<W, T>>(I::simd_slli_i8<kN>(simd_as_i(a.v))); }
template<uint32_t kN, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> slli_u16(const Vec<W, T>& a) noexcept { return from_simd<Vec<W, T>>(I::simd_slli_i16<kN>(simd_as_i(a.v))); }
template<uint32_t kN, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> slli_u32(const Vec<W, T>& a) noexcept { return from_simd<Vec<W, T>>(I::simd_slli_i32<kN>(simd_as_i(a.v))); }
template<uint32_t kN, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> slli_u64(const Vec<W, T>& a) noexcept { return from_simd<Vec<W, T>>(I::simd_slli_i64<kN>(simd_as_i(a.v))); }

template<uint32_t kN, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> srli_u8(const Vec<W, T>& a) noexcept { return from_simd<Vec<W, T>>(I::simd_srli_u8<kN>(simd_as_i(a.v))); }
template<uint32_t kN, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> srli_u16(const Vec<W, T>& a) noexcept { return from_simd<Vec<W, T>>(I::simd_srli_u16<kN>(simd_as_i(a.v))); }
template<uint32_t kN, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> srli_u32(const Vec<W, T>& a) noexcept { return from_simd<Vec<W, T>>(I::simd_srli_u32<kN>(simd_as_i(a.v))); }
template<uint32_t kN, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> srli_u64(const Vec<W, T>& a) noexcept { return from_simd<Vec<W, T>>(I::simd_srli_u64<kN>(simd_as_i(a.v))); }

template<uint32_t kN, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> srai_i8(const Vec<W, T>& a) noexcept { return from_simd<Vec<W, T>>(I::simd_srai_i8<kN>(simd_as_i(a.v))); }
template<uint32_t kN, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> srai_i16(const Vec<W, T>& a) noexcept { return from_simd<Vec<W, T>>(I::simd_srai_i16<kN>(simd_as_i(a.v))); }
template<uint32_t kN, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> srai_i32(const Vec<W, T>& a) noexcept { return from_simd<Vec<W, T>>(I::simd_srai_i32<kN>(simd_as_i(a.v))); }
template<uint32_t kN, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> srai_i64(const Vec<W, T>& a) noexcept { return from_simd<Vec<W, T>>(I::simd_srai_i64<kN>(simd_as_i(a.v))); }

template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, int8_t> slli(const Vec<W, int8_t>& a) noexcept { return from_simd<Vec<W, int8_t>>(I::simd_slli_i8<kN>(simd_as_i(a.v))); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> slli(const Vec<W, int16_t>& a) noexcept { return from_simd<Vec<W, int16_t>>(I::simd_slli_i16<kN>(simd_as_i(a.v))); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, int32_t> slli(const Vec<W, int32_t>& a) noexcept { return from_simd<Vec<W, int32_t>>(I::simd_slli_i32<kN>(simd_as_i(a.v))); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, int64_t> slli(const Vec<W, int64_t>& a) noexcept { return from_simd<Vec<W, int64_t>>(I::simd_slli_i64<kN>(simd_as_i(a.v))); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, uint8_t> slli(const Vec<W, uint8_t>& a) noexcept { return from_simd<Vec<W, uint8_t>>(I::simd_slli_i8<kN>(simd_as_i(a.v))); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, uint16_t> slli(const Vec<W, uint16_t>& a) noexcept { return from_simd<Vec<W, uint16_t>>(I::simd_slli_i16<kN>(simd_as_i(a.v))); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, uint32_t> slli(const Vec<W, uint32_t>& a) noexcept { return from_simd<Vec<W, uint32_t>>(I::simd_slli_i32<kN>(simd_as_i(a.v))); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, uint64_t> slli(const Vec<W, uint64_t>& a) noexcept { return from_simd<Vec<W, uint64_t>>(I::simd_slli_i64<kN>(simd_as_i(a.v))); }

template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, int8_t> srli(const Vec<W, int8_t>& a) noexcept { return from_simd<Vec<W, int8_t>>(I::simd_srli_u8<kN>(simd_as_i(a.v))); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> srli(const Vec<W, int16_t>& a) noexcept { return from_simd<Vec<W, int16_t>>(I::simd_srli_u16<kN>(simd_as_i(a.v))); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, int32_t> srli(const Vec<W, int32_t>& a) noexcept { return from_simd<Vec<W, int32_t>>(I::simd_srli_u32<kN>(simd_as_i(a.v))); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, int64_t> srli(const Vec<W, int64_t>& a) noexcept { return from_simd<Vec<W, int64_t>>(I::simd_srli_u64<kN>(simd_as_i(a.v))); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, uint8_t> srli(const Vec<W, uint8_t>& a) noexcept { return from_simd<Vec<W, uint8_t>>(I::simd_srli_u8<kN>(simd_as_i(a.v))); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, uint16_t> srli(const Vec<W, uint16_t>& a) noexcept { return from_simd<Vec<W, uint16_t>>(I::simd_srli_u16<kN>(simd_as_i(a.v))); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, uint32_t> srli(const Vec<W, uint32_t>& a) noexcept { return from_simd<Vec<W, uint32_t>>(I::simd_srli_u32<kN>(simd_as_i(a.v))); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, uint64_t> srli(const Vec<W, uint64_t>& a) noexcept { return from_simd<Vec<W, uint64_t>>(I::simd_srli_u64<kN>(simd_as_i(a.v))); }

template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, int8_t> srai(const Vec<W, int8_t>& a) noexcept { return from_simd<Vec<W, int8_t>>(I::simd_srai_i8<kN>(simd_as_i(a.v))); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, int16_t> srai(const Vec<W, int16_t>& a) noexcept { return from_simd<Vec<W, int16_t>>(I::simd_srai_i16<kN>(simd_as_i(a.v))); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, int32_t> srai(const Vec<W, int32_t>& a) noexcept { return from_simd<Vec<W, int32_t>>(I::simd_srai_i32<kN>(simd_as_i(a.v))); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, int64_t> srai(const Vec<W, int64_t>& a) noexcept { return from_simd<Vec<W, int64_t>>(I::simd_srai_i64<kN>(simd_as_i(a.v))); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, uint8_t> srai(const Vec<W, uint8_t>& a) noexcept { return from_simd<Vec<W, uint8_t>>(I::simd_srai_i8<kN>(simd_as_i(a.v))); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, uint16_t> srai(const Vec<W, uint16_t>& a) noexcept { return from_simd<Vec<W, uint16_t>>(I::simd_srai_i16<kN>(simd_as_i(a.v))); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, uint32_t> srai(const Vec<W, uint32_t>& a) noexcept { return from_simd<Vec<W, uint32_t>>(I::simd_srai_i32<kN>(simd_as_i(a.v))); }
template<uint32_t kN, size_t W> BL_INLINE_NODEBUG Vec<W, uint64_t> srai(const Vec<W, uint64_t>& a) noexcept { return from_simd<Vec<W, uint64_t>>(I::simd_srai_i64<kN>(simd_as_i(a.v))); }

template<uint32_t kNumBytes, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> sllb_u128(const Vec<W, T>& a) noexcept { return from_simd<Vec<W, T>>(I::simd_sllb_u128<kNumBytes>(simd_as_i(a.v))); }
template<uint32_t kNumBytes, size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> srlb_u128(const Vec<W, T>& a) noexcept { return from_simd<Vec<W, T>>(I::simd_srlb_u128<kNumBytes>(simd_as_i(a.v))); }

template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> sad_u8_u64(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return from_simd<Vec<W, T>>(I::simd_sad_u8_u64(simd_as_i(a.v), simd_as_i(b.v))); }

#if defined(BL_TARGET_OPT_SSSE3)
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> maddws_u8xi8_i16(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return from_simd<Vec<W, T>>(I::simd_maddws_u8xi8_i16(simd_as_i(a.v), simd_as_i(b.v))); }
#endif // BL_TARGET_OPT_SSSE3

#if defined(BL_TARGET_OPT_SSE4_2)
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> clmul_u128_ll(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return from_simd<Vec<W, T>>(I::simd_clmul_u128_ll(simd_as_i(a.v), simd_as_i(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> clmul_u128_lh(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return from_simd<Vec<W, T>>(I::simd_clmul_u128_lh(simd_as_i(a.v), simd_as_i(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> clmul_u128_hl(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return from_simd<Vec<W, T>>(I::simd_clmul_u128_hl(simd_as_i(a.v), simd_as_i(b.v))); }
template<size_t W, typename T> BL_INLINE_NODEBUG Vec<W, T> clmul_u128_hh(const Vec<W, T>& a, const Vec<W, T>& b) noexcept { return from_simd<Vec<W, T>>(I::simd_clmul_u128_hh(simd_as_i(a.v), simd_as_i(b.v))); }
#endif // BL_TARGET_OPT_SSE4_2

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

BL_INLINE_NODEBUG Vec2xF64 cvt_2xi32_f64(const Vec4xI32& a) noexcept { return Vec2xF64{_mm_cvtepi32_pd(a.v)}; }
BL_INLINE_NODEBUG Vec4xI32 cvtt_f64_i32(const Vec2xF64& a) noexcept { return Vec4xI32{_mm_cvttpd_epi32(a.v)}; }

#if defined(BL_TARGET_OPT_AVX2)
BL_INLINE_NODEBUG Vec4xI32 cvtt_f64_i32(const Vec4xF64& a) noexcept { return Vec4xI32{_mm256_cvttpd_epi32(a.v)}; }
#endif // BL_TARGET_OPT_AVX2

namespace Internal {

template<typename T>
BL_INLINE_NODEBUG T simd_broadcast_u128(const __m128i& a) noexcept;

template<> BL_INLINE_NODEBUG __m128i simd_broadcast_u128(const __m128i& a) noexcept { return a; }

#if defined(BL_TARGET_OPT_AVX2)
template<> BL_INLINE_NODEBUG __m256i simd_broadcast_u128(const __m128i& a) noexcept { return _mm256_broadcastsi128_si256(a); }
#endif // BL_TARGET_OPT_AVX2

#if defined(BL_TARGET_OPT_AVX512)
template<> BL_INLINE_NODEBUG __m512i simd_broadcast_u128(const __m128i& a) noexcept { return _mm512_broadcast_i32x4(a); }
#endif // BL_TARGET_OPT_AVX512

}

#if defined(BL_TARGET_OPT_AVX)
template<typename Dst, typename Src>
BL_INLINE_NODEBUG Dst make256_128(const Src& hi, const Src& lo) noexcept {
  return from_simd<Dst>(I::simd_interleave_u128(simd_cast<__m128i>(lo.v), simd_cast<__m128i>(hi.v)));
}

template<typename Dst, typename Src>
BL_INLINE_NODEBUG Dst broadcast_i128(const Src& a) noexcept {
  return from_simd<Dst>(I::simd_broadcast_u128<typename Dst::SimdType>(simd_cast<__m128i>(a.v)));
}

template<uint32_t kN, typename V>
BL_INLINE_NODEBUG Vec<16, typename V::ElementType> extract_i128(const V& a) noexcept {
  return from_simd<Vec<16, typename V::ElementType>>(
    _mm256_extracti128_si256(a.v, kN));
}

BL_INLINE_NODEBUG Vec4xF64 cvt_4xi32_f64(const Vec4xI32& a) noexcept { return Vec4xF64{_mm256_cvtepi32_pd(a.v)}; }

#endif // BL_TARGET_OPT_AVX

#if defined(BL_TARGET_OPT_AVX2)

template<typename V, typename W>
BL_INLINE_NODEBUG V loadu_256_mask32(const void* src, const W& msk) noexcept {
  return V{_mm256_maskload_epi32(static_cast<const int*>(src), simd_cast<__m256i>(msk.v))};
}

template<size_t W, typename T1, typename T2>
BL_INLINE_NODEBUG void storeu_128_mask32(void* dst, const Vec<W, T1>& src, const Vec<W, T2>& msk) noexcept {
  _mm_maskstore_epi32(static_cast<int*>(dst), simd_cast<__m128i>(msk.v), simd_cast<__m128i>(src.v));
}

template<size_t W, typename T1, typename T2>
BL_INLINE_NODEBUG void storeu_256_mask32(void* dst, const Vec<W, T1>& src, const Vec<W, T2>& msk) noexcept {
  _mm256_maskstore_epi32(static_cast<int*>(dst), simd_cast<__m256i>(msk.v), simd_cast<__m256i>(src.v));
}

#endif // BL_TARGET_OPT_AVX2

// SIMD - Public - Utilities - Div255 & Div65535
// =============================================

template<typename V>
BL_INLINE_NODEBUG V div255_u16(const V& a) noexcept {
  V x = add_u16(a, make_u16<V>(0x80u));
  return mulh_u16(x, make_u16<V>(0x0101u));
}

template<typename V>
BL_INLINE_NODEBUG V div65535_u32(const V& a) noexcept {
  V x = add_u32(a, make_u32<V>(0x8000u));
  return srli_u32<16>(add_i32(x, srli_u32<16>(x)));
}

// SIMD - Public - Utilities - Mask Extraction
// ===========================================

// SIMD - Public - Utilities - Array Lookup
// ========================================

template<uint32_t kN>
struct ArrayLookupResult {
  uint32_t _mask;

  BL_INLINE_NODEBUG bool matched() const noexcept { return _mask != 0; }
  BL_INLINE_NODEBUG uint32_t index() const noexcept { return bl::IntOps::ctz(_mask); }

  using Iterator = bl::ParametrizedBitOps<bl::BitOrder::kLSB, uint32_t>::BitIterator;
  BL_INLINE_NODEBUG Iterator iterate() const noexcept { return Iterator(_mask); }
};

template<>
struct ArrayLookupResult<64> {
  uint64_t _mask;

  BL_INLINE_NODEBUG bool matched() const noexcept { return _mask != 0; }
  BL_INLINE_NODEBUG uint32_t index() const noexcept { return bl::IntOps::ctz(_mask); }

  using Iterator = bl::ParametrizedBitOps<bl::BitOrder::kLSB, uint64_t>::BitIterator;
  BL_INLINE_NODEBUG Iterator iterate() const noexcept { return Iterator(_mask); }
};

BL_INLINE_NODEBUG ArrayLookupResult<4> array_lookup_result_from_4x_u32(Vec4xU32 pred) noexcept {
  return ArrayLookupResult<4u>{extract_sign_bits_i32(pred)};
}

#if defined(BL_TARGET_OPT_AVX2)
BL_INLINE_NODEBUG ArrayLookupResult<8> array_lookup_result_from_8x_u32(Vec8xU32 pred) noexcept {
  return ArrayLookupResult<8u>{extract_sign_bits_i32(pred)};
}
#endif // BL_TARGET_OPT_AVX2

BL_INLINE_NODEBUG ArrayLookupResult<8> array_lookup_result_from_8x_u16(Vec8xU16 pred) noexcept {
  return ArrayLookupResult<8u>{extract_sign_bits_i8(packs_128_i16_i8(pred, make_zero<Vec8xU16>()))};
}

BL_INLINE_NODEBUG ArrayLookupResult<16> array_lookup_result_from_16x_u8(Vec16xU8 pred) noexcept {
  return ArrayLookupResult<16u>{extract_sign_bits_i8(pred)};
}

BL_INLINE_NODEBUG ArrayLookupResult<32> array_lookup_result_from_32x_u8(Vec16xU8 pred0, Vec16xU8 pred1) noexcept {
  return ArrayLookupResult<32u>{extract_sign_bits_i8(pred0, pred1)};
}

#if defined(BL_TARGET_OPT_AVX2)
BL_INLINE_NODEBUG ArrayLookupResult<32> array_lookup_result_from_32x_u8(Vec32xU8 pred0) noexcept {
  return ArrayLookupResult<32u>{extract_sign_bits_i8(pred0)};
}
#endif // BL_TARGET_OPT_AVX2

BL_INLINE_NODEBUG ArrayLookupResult<64> array_lookup_result_from_64x_u8(Vec16xU8 pred0, Vec16xU8 pred1, Vec16xU8 pred2, Vec16xU8 pred3) noexcept {
  return ArrayLookupResult<64u>{extract_sign_bits_i8(pred0, pred1, pred2, pred3)};
}

#if defined(BL_TARGET_OPT_AVX2)
BL_INLINE_NODEBUG ArrayLookupResult<64> array_lookup_result_from_64x_u8(Vec32xU8 pred0, Vec32xU8 pred1) noexcept {
  return ArrayLookupResult<64u>{extract_sign_bits_i8(pred0, pred1)};
}
#endif // BL_TARGET_OPT_AVX2

template<uint32_t kN>
BL_INLINE_NODEBUG ArrayLookupResult<kN> array_lookup_u32_eq_aligned16(const uint32_t* array, uint32_t value) noexcept;

template<>
BL_INLINE_NODEBUG ArrayLookupResult<4u> array_lookup_u32_eq_aligned16<4u>(const uint32_t* array, uint32_t value) noexcept {
  return array_lookup_result_from_4x_u32(cmp_eq_u32(loada<Vec4xU32>(array), make_u32<Vec4xU32>(value)));
}

template<>
BL_INLINE_NODEBUG ArrayLookupResult<8u> array_lookup_u32_eq_aligned16<8u>(const uint32_t* array, uint32_t value) noexcept {
#if defined(BL_TARGET_OPT_AVX2)
  return array_lookup_result_from_8x_u32(cmp_eq_u32(loadu<Vec8xU32>(array), make_u32<Vec8xU32>(value)));
#else
  Vec4xU32 v = make_u32<Vec4xU32>(value);
  Vec4xU32 m0 = cmp_eq_u32(loada<Vec4xU32>(array + 0), v);
  Vec4xU32 m1 = cmp_eq_u32(loada<Vec4xU32>(array + 4), v);
  Vec4xU32 m = packs_128_i32_i16(m0, m1);
  return array_lookup_result_from_8x_u16(vec_cast<Vec8xU16>(m));
#endif
}

template<>
BL_INLINE_NODEBUG ArrayLookupResult<16u> array_lookup_u32_eq_aligned16<16u>(const uint32_t* array, uint32_t value) noexcept {
#if defined(BL_TARGET_OPT_AVX512)
  Vec16xU32 v = make_u32<Vec16xU32>(value);
  return ArrayLookupResult<16u>{_cvtmask16_u32(_mm512_cmpeq_epi32_mask(loadu<Vec16xU32>(array).v, v.v))};
#elif defined(BL_TARGET_OPT_AVX2)
  Vec8xU32 v = make_u32<Vec8xU32>(value);
  Vec8xU32 m0 = cmp_eq_u32(loadu<Vec8xU32>(array + 0), v);
  Vec8xU32 m1 = cmp_eq_u32(loadu<Vec8xU32>(array + 8), v);
  uint32_t i0 = extract_sign_bits_i32(m0);
  uint32_t i1 = extract_sign_bits_i32(m1);
  return ArrayLookupResult<16u>{i0 + (i1 << 8)};
#else
  Vec4xU32 v = make_u32<Vec4xU32>(value);
  Vec4xU32 m0 = cmp_eq_u32(loada<Vec4xU32>(array + 0), v);
  Vec4xU32 m1 = cmp_eq_u32(loada<Vec4xU32>(array + 4), v);
  Vec4xU32 m2 = cmp_eq_u32(loada<Vec4xU32>(array + 8), v);
  Vec4xU32 m3 = cmp_eq_u32(loada<Vec4xU32>(array + 12), v);
  return array_lookup_result_from_16x_u8(vec_cast<Vec16xU8>(packs_128_i32_i8(m0, m1, m2, m3)));
#endif
}

} // {anonymous}
} // {SIMD}

#undef I

#if defined(BL_SIMD_MISSING_INTRINSICS)
  #undef BL_SIMD_MISSING_INTRINSICS
#endif // BL_SIMD_MISSING_INTRINSICS

//! \}
//! \endcond

#endif // BLEND2D_SIMD_SIMDX86_P_H_INCLUDED
