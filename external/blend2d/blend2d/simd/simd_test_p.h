// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_SIMD_SIMD_TEST_P_H_INCLUDED
#define BLEND2D_SIMD_SIMD_TEST_P_H_INCLUDED

#include <blend2d/core/api-build_test_p.h>
#include <blend2d/core/api-internal_p.h>
#include <blend2d/core/random_p.h>
#include <blend2d/core/string_p.h>
#include <blend2d/simd/simd_p.h>

//! \cond NEVER

namespace SIMDTests {
namespace {

static constexpr uint64_t kRandomSeed = 0x1234u;
static constexpr uint32_t kTestIterCount = 1000u;

// SIMD - Tests - Costs
// ====================

static void print_cost_matrix(const char* ext) noexcept {
  INFO("%s Cost Matrix:", ext);
  INFO("  abs_i8=%u"        , BL_SIMD_COST_ABS_I8);
  INFO("  abs_i16=%u"       , BL_SIMD_COST_ABS_I16);
  INFO("  abs_i32=%u"       , BL_SIMD_COST_ABS_I32);
  INFO("  abs_i64=%u"       , BL_SIMD_COST_ABS_I64);
  INFO("  alignr_u8=%u"     , BL_SIMD_COST_ALIGNR_U8);
  INFO("  cmp_eq_i64=%u"    , BL_SIMD_COST_CMP_EQ_I64);
  INFO("  cmp_lt_gt_i64=%u" , BL_SIMD_COST_CMP_LT_GT_I64);
  INFO("  cmp_le_ge_i64=%u" , BL_SIMD_COST_CMP_LE_GE_I64);
  INFO("  cmp_lt_gt_u64=%u" , BL_SIMD_COST_CMP_LT_GT_U64);
  INFO("  cmp_le_ge_u64=%u" , BL_SIMD_COST_CMP_LE_GE_U64);
  INFO("  min_max_i8=%u"    , BL_SIMD_COST_MIN_MAX_I8);
  INFO("  min_max_u8=%u"    , BL_SIMD_COST_MIN_MAX_U8);
  INFO("  min_max_i16=%u"   , BL_SIMD_COST_MIN_MAX_I16);
  INFO("  min_max_u16=%u"   , BL_SIMD_COST_MIN_MAX_U16);
  INFO("  min_max_i32=%u"   , BL_SIMD_COST_MIN_MAX_I32);
  INFO("  min_max_u32=%u"   , BL_SIMD_COST_MIN_MAX_U32);
  INFO("  min_max_i64=%u"   , BL_SIMD_COST_MIN_MAX_I64);
  INFO("  min_max_u64=%u"   , BL_SIMD_COST_MIN_MAX_U64);
  INFO("  mul_i16=%u"       , BL_SIMD_COST_MUL_I16);
  INFO("  mul_i32=%u"       , BL_SIMD_COST_MUL_I32);
  INFO("  mul_i64=%u"       , BL_SIMD_COST_MUL_I64);
}

// SIMD - Tests - Vector Overlay
// =============================

template<uint32_t kW, typename T>
union VecOverlay {
  T items[kW / sizeof(T)];
  uint8_t data_u8[kW];
  uint16_t data_u16[kW / 2u];
  uint32_t data_u32[kW / 4u];
  uint64_t data_u64[kW / 8u];
};

// SIMD - Tests - Data Generators & Constraints
// ============================================

// Data generator, which is used to fill the content of SIMD registers.
class DataGenInt {
public:
  BLRandom rng;
  uint32_t step;

  BL_INLINE explicit DataGenInt(uint64_t seed) noexcept
    : rng(seed),
      step(0) {}

  uint64_t next_uint64() noexcept {
    if (++step >= 256)
      step = 0;

    // NOTE: Nothing really elaborate - sometimes we want to test also numbers
    // that in general random number generators won't return often.
    switch (step) {
      case   0: return 0u;
      case   1: return 0u;
      case   2: return 0u;
      case   6: return 1u;
      case   7: return 0u;
      case  10: return 0u;
      case  11: return 0xFFu;
      case  15: return 0xFFFFu;
      case  17: return 0xFFFFFFFFu;
      case  21: return 0xFFFFFFFFFFFFFFFFu;
      case  24: return 1u;
      case  40: return 0xFFu;
      case  55: return 0x8080808080808080u;
      case  66: return 0x80000080u;
      case  69: return 1u;
      case  79: return 0x7F;
      case 122: return 0xFFFFu;
      case 123: return 0xFFFFu;
      case 124: return 0xFFFFu;
      case 127: return 1u;
      case 130: return 0xFFu;
      case 142: return 0x7FFFu;
      case 143: return 0x7FFFu;
      case 144: return 0u;
      case 145: return 0x7FFFu;
      default : return rng.next_uint64();
    }
  }
};

// Some SIMD operations are constrained, especially those higher level. So, to successfully test these we
// have to model the constraints in a way that the SIMD instruction we test actually gets the correct input.
// Note that a constraint doesn't have to be always range based, it could be anything.
struct ConstraintNone {
  template<uint32_t kW, typename T>
  static BL_INLINE_NODEBUG void apply(VecOverlay<kW, T>& v) noexcept { bl_unused(v); }
};

template<typename ElementT, typename Derived>
struct ConstraintBase {
  template<uint32_t kW, typename T>
  static BL_INLINE void apply(VecOverlay<kW, T>& v) noexcept {
    ElementT elements[kW / sizeof(ElementT)];

    memcpy(elements, v.data_u8, kW);
    for (size_t i = 0; i < kW / sizeof(ElementT); i++)
      elements[i] = Derived::apply_one(elements[i]);
    memcpy(v.data_u8, elements, kW);
  }
};

template<uint8_t kMin, uint8_t kMax>
struct ConstraintRangeU8 : public ConstraintBase<uint16_t, ConstraintRangeU8<kMin, kMax>> {
  static BL_INLINE_NODEBUG uint8_t apply_one(uint8_t x) noexcept { return bl_clamp(x, kMin, kMax); }
};

template<uint16_t kMin, uint16_t kMax>
struct ConstraintRangeU16 : public ConstraintBase<uint16_t, ConstraintRangeU16<kMin, kMax>> {
  static BL_INLINE_NODEBUG uint16_t apply_one(uint16_t x) noexcept { return bl_clamp(x, kMin, kMax); }
};

template<uint32_t kMin, uint32_t kMax>
struct ConstraintRangeU32 : public ConstraintBase<uint32_t, ConstraintRangeU32<kMin, kMax>> {
  static BL_INLINE_NODEBUG uint32_t apply_one(uint32_t x) noexcept { return bl_clamp(x, kMin, kMax); }
};

// SIMD - Tests - Generic Operations
// =================================

template<typename T>
static BL_INLINE_NODEBUG std::make_unsigned_t<T> cast_uint(const T& x) noexcept {
  return static_cast<std::make_unsigned_t<T>>(x);
}

template<typename T>
static BL_INLINE_NODEBUG std::make_signed_t<T> cast_int(const T& x) noexcept {
  return static_cast<std::make_signed_t<T>>(x);
}

template<typename T, typename Derived> struct op_base_1 {
  template<uint32_t kW>
  static BL_INLINE VecOverlay<kW, T> apply(const VecOverlay<kW, T>& a) noexcept {
    VecOverlay<kW, T> out{};
    for (uint32_t i = 0; i < kW / sizeof(T); i++)
      out.items[i] = Derived::apply_one(a.items[i]);
    return out;
  }
};

template<typename T, typename Derived> struct op_base_2 {
  template<uint32_t kW>
  static BL_INLINE VecOverlay<kW, T> apply(const VecOverlay<kW, T>& a, const VecOverlay<kW, T>& b) noexcept {
    VecOverlay<kW, T> out{};
    for (uint32_t i = 0; i < kW / sizeof(T); i++)
      out.items[i] = Derived::apply_one(a.items[i], b.items[i]);
    return out;
  }
};

template<typename T, typename Derived> struct op_base_3 {
  template<uint32_t kW>
  static BL_INLINE VecOverlay<kW, T> apply(const VecOverlay<kW, T>& a, const VecOverlay<kW, T>& b, const VecOverlay<kW, T>& c) noexcept {
    VecOverlay<kW, T> out{};
    for (uint32_t i = 0; i < kW / sizeof(T); i++)
      out.items[i] = Derived::apply_one(a.items[i], b.items[i], c.items[i]);
    return out;
  }
};

template<typename T> struct iop_and : public op_base_2<T, iop_and<T>> {
  static BL_INLINE_NODEBUG T apply_one(const T& a, const T& b) noexcept { return T(a & b); }
};

template<typename T> struct iop_andnot : public op_base_2<T, iop_andnot<T>> {
  static BL_INLINE_NODEBUG T apply_one(const T& a, const T& b) noexcept { return T(~a & b); }
};

template<typename T> struct iop_or : public op_base_2<T, iop_or<T>> {
  static BL_INLINE_NODEBUG T apply_one(const T& a, const T& b) noexcept { return T(a | b); }
};

template<typename T> struct iop_xor : public op_base_2<T, iop_xor<T>> {
  static BL_INLINE_NODEBUG T apply_one(const T& a, const T& b) noexcept { return T(a ^ b); }
};

template<typename T> struct iop_blendv_bits : public op_base_3<T, iop_blendv_bits<T>> {
  static BL_INLINE_NODEBUG T apply_one(const T& a, const T& b, const T& c) noexcept { return T((a & ~c) | (b & c)); }
};

template<typename T> struct iop_abs : public op_base_1<T, iop_abs<T>> {
  static BL_INLINE_NODEBUG T apply_one(const T& a) noexcept { return a < 0 ? T(cast_uint(T(0)) - cast_uint(a)) : a; }
};

template<typename T> struct iop_not : public op_base_1<T, iop_not<T>> {
  static BL_INLINE_NODEBUG T apply_one(const T& a) noexcept { return T(~a); }
};

template<typename T> struct iop_add : public op_base_2<T, iop_add<T>> {
  static BL_INLINE_NODEBUG T apply_one(const T& a, const T& b) noexcept { return T(cast_uint(a) + cast_uint(b)); }
};

template<typename T> struct iop_adds : public op_base_2<T, iop_adds<T>> {
  static BL_INLINE T apply_one(const T& a, const T& b) noexcept {
    bl::OverflowFlag of{};
    T result = bl::IntOps::add_overflow(a, b, &of);

    if (!of)
      return result;

    if (std::is_unsigned_v<T> || b > 0)
      return bl::Traits::max_value<T>();
    else
      return bl::Traits::min_value<T>();
  }
};

template<typename T> struct iop_sub : public op_base_2<T, iop_sub<T>> {
  static BL_INLINE_NODEBUG T apply_one(const T& a, const T& b) noexcept { return T(cast_uint(a) - cast_uint(b)); }
};

template<typename T> struct iop_subs : public op_base_2<T, iop_subs<T>> {
  static BL_INLINE T apply_one(const T& a, const T& b) noexcept {
    bl::OverflowFlag of{};
    T result = bl::IntOps::sub_overflow(a, b, &of);

    if (!of)
      return result;

    if (std::is_unsigned_v<T> || b > 0)
      return bl::Traits::min_value<T>();
    else
      return bl::Traits::max_value<T>();
  }
};

template<typename T> struct iop_mul : public op_base_2<T, iop_mul<T>> {
  static BL_INLINE_NODEBUG T apply_one(const T& a, const T& b) noexcept { return T(T(uint64_t(a) * uint64_t(b)) & T(~T(0))); }
};

template<typename T> struct iop_min : public op_base_2<T, iop_min<T>> {
  static BL_INLINE_NODEBUG T apply_one(const T& a, const T& b) noexcept { return a < b ? a : b; }
};

template<typename T> struct iop_max : public op_base_2<T, iop_max<T>> {
  static BL_INLINE_NODEBUG T apply_one(const T& a, const T& b) noexcept { return a > b ? a : b; }
};

template<typename T, uint32_t kN> struct iop_slli : public op_base_1<T, iop_slli<T, kN>> {
  static BL_INLINE_NODEBUG T apply_one(const T& a) noexcept { return T(cast_uint(a) << kN); }
};

template<typename T, uint32_t kN> struct iop_srli : public op_base_1<T, iop_srli<T, kN>> {
  static BL_INLINE_NODEBUG T apply_one(const T& a) noexcept { return T(cast_uint(a) >> kN); }
};

template<typename T, uint32_t kN> struct iop_rsrli : public op_base_1<T, iop_rsrli<T, kN>> {
  static BL_INLINE T apply_one(const T& a) noexcept {
    T add = T((a & (T(1) << (kN - 1))) != 0);
    return T((cast_uint(a) >> kN) + cast_uint(add));
  }
};

template<typename T, uint32_t kN> struct iop_srai : public op_base_1<T, iop_srai<T, kN>> {
  static BL_INLINE_NODEBUG T apply_one(const T& a) noexcept { return T(cast_int(a) >> kN); }
};

template<typename T> struct iop_cmp_eq : public op_base_2<T, iop_cmp_eq<T>> {
  static BL_INLINE_NODEBUG T apply_one(const T& a, const T& b) noexcept { return a == b ? bl::IntOps::all_ones<T>() : T(0); }
};

template<typename T> struct iop_cmp_ne : public op_base_2<T, iop_cmp_ne<T>> {
  static BL_INLINE_NODEBUG T apply_one(const T& a, const T& b) noexcept { return a != b ? bl::IntOps::all_ones<T>() : T(0); }
};

template<typename T> struct iop_cmp_gt : public op_base_2<T, iop_cmp_gt<T>> {
  static BL_INLINE_NODEBUG T apply_one(const T& a, const T& b) noexcept { return a >  b ? bl::IntOps::all_ones<T>() : T(0); }
};

template<typename T> struct iop_cmp_ge : public op_base_2<T, iop_cmp_ge<T>> {
  static BL_INLINE_NODEBUG T apply_one(const T& a, const T& b) noexcept { return a >= b ? bl::IntOps::all_ones<T>() : T(0); }
};

template<typename T> struct iop_cmp_lt : public op_base_2<T, iop_cmp_lt<T>> {
  static BL_INLINE_NODEBUG T apply_one(const T& a, const T& b) noexcept { return a <  b ? bl::IntOps::all_ones<T>() : T(0); }
};

template<typename T> struct iop_cmp_le : public op_base_2<T, iop_cmp_le<T>> {
  static BL_INLINE_NODEBUG T apply_one(const T& a, const T& b) noexcept { return a <= b ? bl::IntOps::all_ones<T>() : T(0); }
};

template<typename T, uint32_t kN> struct iop_sllb_u128 {
  template<uint32_t kW>
  static BL_INLINE VecOverlay<kW, T> apply(const VecOverlay<kW, T>& a) noexcept {
    VecOverlay<kW, T> out{};
    for (uint32_t off = 0; off < kW; off += 16) {
      for (uint32_t i = 0; i < 16; i++) {
        out.data_u8[off + i] = i < kN ? uint8_t(0) : a.data_u8[off + i - kN];
      }
    }
    return out;
  }
};

template<typename T, uint32_t kN> struct iop_srlb_u128 {
  template<uint32_t kW>
  static BL_INLINE VecOverlay<kW, T> apply(const VecOverlay<kW, T>& a) noexcept {
    VecOverlay<kW, T> out{};
    for (uint32_t off = 0; off < kW; off += 16) {
      for (uint32_t i = 0; i < 16; i++) {
        out.data_u8[off + i] = i + kN < 16u ? a.data_u8[off + i + kN] : uint8_t(0);
      }
    }
    return out;
  }
};

template<typename T, uint32_t kN> struct iop_alignr_u128 {
  template<uint32_t kW>
  static BL_INLINE VecOverlay<kW, T> apply(const VecOverlay<kW, T>& a, const VecOverlay<kW, T>& b) noexcept {
    VecOverlay<kW, T> out{};
    for (uint32_t off = 0; off < kW; off += 16) {
      for (uint32_t i = 0; i < 16; i++) {
        out.data_u8[off + i] = i + kN < 16 ? b.data_u8[off + i + kN] : a.data_u8[off + i + kN - 16];
      }
    }
    return out;
  }
};

template<typename T> struct iop_broadcast_u8 {
  template<uint32_t kW>
  static BL_INLINE VecOverlay<kW, T> apply(const VecOverlay<kW, T>& a) noexcept {
    VecOverlay<kW, T> out{};
    for (uint32_t i = 0; i < kW; i++)
      out.data_u8[i] = a.data_u8[0];
    return out;
  }
};

template<typename T> struct iop_broadcast_u16 {
  template<uint32_t kW>
  static BL_INLINE VecOverlay<kW, T> apply(const VecOverlay<kW, T>& a) noexcept {
    VecOverlay<kW, T> out{};
    for (uint32_t i = 0; i < kW / 2u; i++)
      out.data_u16[i] = a.data_u16[0];
    return out;
  }
};

template<typename T> struct iop_broadcast_u32 {
  template<uint32_t kW>
  static BL_INLINE VecOverlay<kW, T> apply(const VecOverlay<kW, T>& a) noexcept {
    VecOverlay<kW, T> out{};
    for (uint32_t i = 0; i < kW / 4u; i++)
      out.data_u32[i] = a.data_u32[0];
    return out;
  }
};

template<typename T> struct iop_broadcast_u64 {
  template<uint32_t kW>
  static BL_INLINE VecOverlay<kW, T> apply(const VecOverlay<kW, T>& a) noexcept {
    VecOverlay<kW, T> out{};
    for (uint32_t i = 0; i < kW / 8u; i++)
      out.data_u64[i] = a.data_u64[0];
    return out;
  }
};

template<typename T> struct iop_swizzlev_u8 {
  template<uint32_t kW>
  static BL_INLINE VecOverlay<kW, T> apply(const VecOverlay<kW, T>& a, const VecOverlay<kW, T>& b) noexcept {
    VecOverlay<kW, T> out{};
    for (uint32_t off = 0; off < kW; off += 16) {
      for (uint32_t i = 0; i < 16; i++) {
        size_t sel = b.data_u8[off + i] & (0x8F); // 3 bits ignored.
        out.data_u8[off + i] = sel & 0x80 ? uint8_t(0) : a.data_u8[off + sel];
      }
    }
    return out;
  }
};

template<typename T, uint8_t D, uint8_t C, uint8_t B, uint8_t A> struct iop_swizzle_u16 {
  template<uint32_t kW>
  static BL_INLINE VecOverlay<kW, T> apply(const VecOverlay<kW, T>& a) noexcept {
    VecOverlay<kW, T> out{};
    for (uint32_t off = 0; off < kW; off += 16) {
      out.data_u16[off / 2 + 0] = a.data_u16[off / 2 + 0 + A];
      out.data_u16[off / 2 + 1] = a.data_u16[off / 2 + 0 + B];
      out.data_u16[off / 2 + 2] = a.data_u16[off / 2 + 0 + C];
      out.data_u16[off / 2 + 3] = a.data_u16[off / 2 + 0 + D];
      out.data_u16[off / 2 + 4] = a.data_u16[off / 2 + 4 + A];
      out.data_u16[off / 2 + 5] = a.data_u16[off / 2 + 4 + B];
      out.data_u16[off / 2 + 6] = a.data_u16[off / 2 + 4 + C];
      out.data_u16[off / 2 + 7] = a.data_u16[off / 2 + 4 + D];
    }
    return out;
  }
};

template<typename T, uint8_t D, uint8_t C, uint8_t B, uint8_t A> struct iop_swizzle_lo_u16 {
  template<uint32_t kW>
  static BL_INLINE VecOverlay<kW, T> apply(const VecOverlay<kW, T>& a) noexcept {
    VecOverlay<kW, T> out{};
    for (uint32_t off = 0; off < kW; off += 16) {
      out.data_u16[off / 2 + 0] = a.data_u16[off / 2 + A];
      out.data_u16[off / 2 + 1] = a.data_u16[off / 2 + B];
      out.data_u16[off / 2 + 2] = a.data_u16[off / 2 + C];
      out.data_u16[off / 2 + 3] = a.data_u16[off / 2 + D];
      memcpy(out.data_u8 + off + 8, a.data_u8 + off + 8, 8);
    }
    return out;
  }
};

template<typename T, uint8_t D, uint8_t C, uint8_t B, uint8_t A> struct iop_swizzle_hi_u16 {
  template<uint32_t kW>
  static BL_INLINE VecOverlay<kW, T> apply(const VecOverlay<kW, T>& a) noexcept {
    VecOverlay<kW, T> out{};
    for (uint32_t off = 0; off < kW; off += 16) {
      memcpy(out.data_u8 + off, a.data_u8 + off, 8);
      out.data_u16[off / 2 + 4] = a.data_u16[off / 2 + 4 + A];
      out.data_u16[off / 2 + 5] = a.data_u16[off / 2 + 4 + B];
      out.data_u16[off / 2 + 6] = a.data_u16[off / 2 + 4 + C];
      out.data_u16[off / 2 + 7] = a.data_u16[off / 2 + 4 + D];
    }
    return out;
  }
};

template<typename T, uint8_t D, uint8_t C, uint8_t B, uint8_t A> struct iop_swizzle_u32 {
  template<uint32_t kW>
  static BL_INLINE VecOverlay<kW, T> apply(const VecOverlay<kW, T>& a) noexcept {
    VecOverlay<kW, T> out{};
    for (uint32_t off = 0; off < kW; off += 16) {
      out.data_u32[off / 4 + 0] = a.data_u32[off / 4 + A];
      out.data_u32[off / 4 + 1] = a.data_u32[off / 4 + B];
      out.data_u32[off / 4 + 2] = a.data_u32[off / 4 + C];
      out.data_u32[off / 4 + 3] = a.data_u32[off / 4 + D];
    }
    return out;
  }
};

template<typename T, uint8_t B, uint8_t A> struct iop_swizzle_u64 {
  template<uint32_t kW>
  static BL_INLINE VecOverlay<kW, T> apply(const VecOverlay<kW, T>& a) noexcept {
    VecOverlay<kW, T> out{};
    for (uint32_t off = 0; off < kW; off += 16) {
      out.data_u64[off / 8 + 0] = a.data_u64[off / 8 + A];
      out.data_u64[off / 8 + 1] = a.data_u64[off / 8 + B];
    }
    return out;
  }
};

template<typename T> struct iop_interleave_lo_u8 {
  template<uint32_t kW>
  static BL_INLINE VecOverlay<kW, T> apply(const VecOverlay<kW, T>& a, const VecOverlay<kW, T>& b) noexcept {
    VecOverlay<kW, T> out{};
    for (uint32_t off = 0; off < kW; off += 16) {
      for (uint32_t i = 0; i < 8; i++) {
        out.data_u8[off + i * 2 + 0] = a.data_u8[off + i];
        out.data_u8[off + i * 2 + 1] = b.data_u8[off + i];
      }
    }
    return out;
  }
};

template<typename T> struct iop_interleave_hi_u8 {
  template<uint32_t kW>
  static BL_INLINE VecOverlay<kW, T> apply(const VecOverlay<kW, T>& a, const VecOverlay<kW, T>& b) noexcept {
    VecOverlay<kW, T> out{};
    for (uint32_t off = 0; off < kW; off += 16) {
      for (uint32_t i = 0; i < 8; i++) {
        out.data_u8[off + i * 2 + 0] = a.data_u8[off + 8 + i];
        out.data_u8[off + i * 2 + 1] = b.data_u8[off + 8 + i];
      }
    }
    return out;
  }
};

template<typename T> struct iop_interleave_lo_u16 {
  template<uint32_t kW>
  static BL_INLINE VecOverlay<kW, T> apply(const VecOverlay<kW, T>& a, const VecOverlay<kW, T>& b) noexcept {
    VecOverlay<kW, T> out{};
    for (uint32_t off = 0; off < kW; off += 16) {
      for (uint32_t i = 0; i < 4; i++) {
        out.data_u16[off / 2 + i * 2 + 0] = a.data_u16[off / 2 + i];
        out.data_u16[off / 2 + i * 2 + 1] = b.data_u16[off / 2 + i];
      }
    }
    return out;
  }
};

template<typename T> struct iop_interleave_hi_u16 {
  template<uint32_t kW>
  static BL_INLINE VecOverlay<kW, T> apply(const VecOverlay<kW, T>& a, const VecOverlay<kW, T>& b) noexcept {
    VecOverlay<kW, T> out{};
    for (uint32_t off = 0; off < kW; off += 16) {
      for (uint32_t i = 0; i < 4; i++) {
        out.data_u16[off / 2 + i * 2 + 0] = a.data_u16[off / 2 + 4 + i];
        out.data_u16[off / 2 + i * 2 + 1] = b.data_u16[off / 2 + 4 + i];
      }
    }
    return out;
  }
};

template<typename T> struct iop_interleave_lo_u32 {
  template<uint32_t kW>
  static BL_INLINE VecOverlay<kW, T> apply(const VecOverlay<kW, T>& a, const VecOverlay<kW, T>& b) noexcept {
    VecOverlay<kW, T> out{};
    for (uint32_t off = 0; off < kW; off += 16) {
      for (uint32_t i = 0; i < 2; i++) {
        out.data_u32[off / 4 + i * 2 + 0] = a.data_u32[off / 4 + i];
        out.data_u32[off / 4 + i * 2 + 1] = b.data_u32[off / 4 + i];
      }
    }
    return out;
  }
};

template<typename T> struct iop_interleave_hi_u32 {
  template<uint32_t kW>
  static BL_INLINE VecOverlay<kW, T> apply(const VecOverlay<kW, T>& a, const VecOverlay<kW, T>& b) noexcept {
    VecOverlay<kW, T> out{};
    for (uint32_t off = 0; off < kW; off += 16) {
      for (uint32_t i = 0; i < 2; i++) {
        out.data_u32[off / 4 + i * 2 + 0] = a.data_u32[off / 4 + 2 + i];
        out.data_u32[off / 4 + i * 2 + 1] = b.data_u32[off / 4 + 2 + i];
      }
    }
    return out;
  }
};

template<typename T> struct iop_interleave_lo_u64 {
  template<uint32_t kW>
  static BL_INLINE VecOverlay<kW, T> apply(const VecOverlay<kW, T>& a, const VecOverlay<kW, T>& b) noexcept {
    VecOverlay<kW, T> out{};
    for (uint32_t off = 0; off < kW; off += 16) {
      out.data_u64[off / 8 + 0] = a.data_u64[off / 8 + 0];
      out.data_u64[off / 8 + 1] = b.data_u64[off / 8 + 0];
    }
    return out;
  }
};

template<typename T> struct iop_interleave_hi_u64 {
  template<uint32_t kW>
  static BL_INLINE VecOverlay<kW, T> apply(const VecOverlay<kW, T>& a, const VecOverlay<kW, T>& b) noexcept {
    VecOverlay<kW, T> out{};
    for (uint32_t off = 0; off < kW; off += 16) {
      out.data_u64[off / 8 + 0] = a.data_u64[off / 8 + 1];
      out.data_u64[off / 8 + 1] = b.data_u64[off / 8 + 1];
    }
    return out;
  }
};

struct iop_div255_u16 : public op_base_1<uint16_t, iop_div255_u16> {
  static BL_INLINE uint16_t apply_one(const uint16_t& a) noexcept {
    uint32_t x = a + 0x80u;
    return uint16_t((x + (x >> 8)) >> 8);
  }
};

struct iop_div65535_u32 : public op_base_1<uint32_t, iop_div65535_u32> {
  static BL_INLINE uint32_t apply_one(const uint32_t& a) noexcept {
    uint32_t x = a + 0x8000u;
    return uint32_t((x + (x >> 16)) >> 16);
  }
};

// SIMD - Tests - Verification
// ===========================

template<typename T> struct TypeNameToString {};
template<> struct TypeNameToString<int8_t  > { static BL_INLINE_NODEBUG const char* get() noexcept { return "int8"; } };
template<> struct TypeNameToString<int16_t > { static BL_INLINE_NODEBUG const char* get() noexcept { return "int16"; } };
template<> struct TypeNameToString<int32_t > { static BL_INLINE_NODEBUG const char* get() noexcept { return "int32"; } };
template<> struct TypeNameToString<int64_t > { static BL_INLINE_NODEBUG const char* get() noexcept { return "int64"; } };
template<> struct TypeNameToString<uint8_t > { static BL_INLINE_NODEBUG const char* get() noexcept { return "uint8"; } };
template<> struct TypeNameToString<uint16_t> { static BL_INLINE_NODEBUG const char* get() noexcept { return "uint16"; } };
template<> struct TypeNameToString<uint32_t> { static BL_INLINE_NODEBUG const char* get() noexcept { return "uint32"; } };
template<> struct TypeNameToString<uint64_t> { static BL_INLINE_NODEBUG const char* get() noexcept { return "uint64"; } };
template<> struct TypeNameToString<float   > { static BL_INLINE_NODEBUG const char* get() noexcept { return "float32"; } };
template<> struct TypeNameToString<double  > { static BL_INLINE_NODEBUG const char* get() noexcept { return "float64"; } };

template<typename T>
static BL_NOINLINE BLString format_items(const T* items, uint32_t count) noexcept {
  BLString s;
  s.append('{');
  for (uint32_t i = 0; i < count; i++)
    s.append_format("%s%llu", i == 0 ? "" : ", ", (unsigned long long)items[i] & bl::IntOps::all_ones<std::make_unsigned_t<T>>());
  s.append('}');
  return s;
}

template<typename T>
static bool compare_ivec(const T* observed, const T* expected, uint32_t count) noexcept {
  for (uint32_t i = 0; i < count; i++)
    if (BL_UNLIKELY(observed[i] != expected[i]))
      return false;
  return true;
}

template<typename T>
static void verify_ivec(const T* observed, const T* expected, uint32_t count) noexcept {
  if (!compare_ivec(observed, expected, count)) {
    BLString observed_str = format_items(observed, count);
    BLString expected_str = format_items(expected, count);

    EXPECT_EQ(observed_str, expected_str)
      .message("Operation failed\n"
              "      Observed: %s\n"
              "      Expected: %s",
              observed_str.data(),
              expected_str.data());
  }
}

template<typename T>
static BL_NOINLINE void test_iop1_failed(const T* input1, const T* observed, const T* expected, uint32_t count) noexcept {
  BLString input1_str = format_items(input1, count);
  BLString observed_str = format_items(observed, count);
  BLString expected_str = format_items(expected, count);

  EXPECT_EQ(observed_str, expected_str)
    .message("Operation failed\n"
             "      Input #1: %s\n"
             "      Observed: %s\n"
             "      Expected: %s",
             input1_str.data(),
             observed_str.data(),
             expected_str.data());
}

template<typename T>
static BL_NOINLINE void test_iop2_failed(const T* input1, const T* input2, const T* observed, const T* expected, uint32_t count) noexcept {
  BLString input1_str = format_items(input1, count);
  BLString input2_str = format_items(input2, count);
  BLString observed_str = format_items(observed, count);
  BLString expected_str = format_items(expected, count);

  EXPECT_EQ(observed_str, expected_str)
    .message("Operation failed\n"
             "      Input #1: %s\n"
             "      Input #2: %s\n"
             "      Observed: %s\n"
             "      Expected: %s",
             input1_str.data(),
             input2_str.data(),
             observed_str.data(),
             expected_str.data());
}

template<typename T>
static BL_NOINLINE void test_iop3_failed(const T* input1, const T* input2, const T* input3, const T* observed, const T* expected, uint32_t count) noexcept {
  BLString input1_str = format_items(input1, count);
  BLString input2_str = format_items(input2, count);
  BLString input3_str = format_items(input3, count);
  BLString observed_str = format_items(observed, count);
  BLString expected_str = format_items(expected, count);

  EXPECT_EQ(observed_str, expected_str)
    .message("Operation failed\n"
             "      Input #1: %s\n"
             "      Input #2: %s\n"
             "      Input #3: %s\n"
             "      Observed: %s\n"
             "      Expected: %s",
             input1_str.data(),
             input2_str.data(),
             input3_str.data(),
             observed_str.data(),
             expected_str.data());
}

// SIMD - Tests - Utilities
// ========================

template<uint32_t kW, typename T>
static void fill_random(DataGenInt& dg, VecOverlay<kW, T>& dst) noexcept {
  for (uint32_t i = 0; i < kW / 8u; i++)
    dst.data_u64[i] = dg.next_uint64();
}

template<typename T>
static void fill_val(T* arr, T v, uint32_t count, uint32_t repeat = 1) noexcept {
  uint32_t add = 0;
  for (uint32_t i = 0; i < count; i++) {
    arr[i] = T(v + T(add));
    if (++add >= repeat)
      add = 0;
  }
}

// SIMD - Tests - Integer Operations - 1 Source Operand
// ====================================================

template<typename V, typename GenericOp, typename Constraint, typename VecOp>
static BL_NOINLINE void test_iop1_constraint(VecOp&& vec_op) noexcept {
  typedef typename V::ElementType T;
  constexpr uint32_t kItemCount = uint32_t(V::kW / sizeof(T));

  DataGenInt dg(kRandomSeed);
  for (uint32_t iter = 0; iter < kTestIterCount; iter++) {
    VecOverlay<V::kW, T> a;
    VecOverlay<V::kW, T> observed;
    VecOverlay<V::kW, T> expected;

    fill_random(dg, a);
    Constraint::apply(a);

    V va = SIMD::loadu<V>(a.data_u8);
    V vr = vec_op(va);

    SIMD::storeu(observed.data_u8, vr);
    expected = GenericOp::apply(a);

    if (!compare_ivec(observed.items, expected.items, kItemCount))
      test_iop1_failed(a.items, observed.items, expected.items, kItemCount);
  }
}

template<typename V, typename GenericOp, typename VecOp>
static void test_iop1(VecOp&& vec_op) noexcept {
  return test_iop1_constraint<V, GenericOp, ConstraintNone, VecOp>(BLInternal::forward<VecOp>(vec_op));
}

// SIMD - Tests - Integer Operations - 2 Source Operands
// =====================================================

template<typename V, typename GenericOp, typename Constraint, typename VecOp>
static BL_NOINLINE void test_iop2_constraint(VecOp&& vec_op) noexcept {
  typedef typename V::ElementType T;
  constexpr uint32_t kItemCount = uint32_t(V::kW / sizeof(T));

  DataGenInt dg(kRandomSeed);
  for (uint32_t iter = 0; iter < kTestIterCount; iter++) {
    VecOverlay<V::kW, T> a;
    VecOverlay<V::kW, T> b;
    VecOverlay<V::kW, T> observed;
    VecOverlay<V::kW, T> expected;

    fill_random(dg, a);
    fill_random(dg, b);
    Constraint::apply(a);
    Constraint::apply(b);

    V va = SIMD::loadu<V>(a.data_u8);
    V vb = SIMD::loadu<V>(b.data_u8);
    V vr = vec_op(va, vb);

    SIMD::storeu(observed.data_u8, vr);
    expected = GenericOp::apply(a, b);

    if (!compare_ivec(observed.items, expected.items, kItemCount))
      test_iop2_failed(a.items, b.items, observed.items, expected.items, kItemCount);
  }
}

template<typename V, typename GenericOp, typename VecOp>
static void test_iop2(VecOp&& vec_op) noexcept {
  return test_iop2_constraint<V, GenericOp, ConstraintNone, VecOp>(BLInternal::forward<VecOp>(vec_op));
}

// SIMD - Tests - Integer Operations - 3 Source Operands
// =====================================================

template<typename V, typename GenericOp, typename Constraint, typename VecOp>
static BL_NOINLINE void test_iop3_constraint(VecOp&& vec_op) noexcept {
  typedef typename V::ElementType T;
  constexpr uint32_t kItemCount = uint32_t(V::kW / sizeof(T));

  DataGenInt dg(kRandomSeed);
  for (uint32_t iter = 0; iter < kTestIterCount; iter++) {
    VecOverlay<V::kW, T> a;
    VecOverlay<V::kW, T> b;
    VecOverlay<V::kW, T> c;
    VecOverlay<V::kW, T> observed;
    VecOverlay<V::kW, T> expected;

    fill_random(dg, a);
    fill_random(dg, b);
    fill_random(dg, c);
    Constraint::apply(a);
    Constraint::apply(b);
    Constraint::apply(c);

    V va = SIMD::loadu<V>(a.data_u8);
    V vb = SIMD::loadu<V>(b.data_u8);
    V vc = SIMD::loadu<V>(c.data_u8);
    V vr = vec_op(va, vb, vc);

    SIMD::storeu(observed.data_u8, vr);
    expected = GenericOp::apply(a, b, c);

    if (!compare_ivec(observed.items, expected.items, kItemCount))
      test_iop3_failed(a.items, b.items, c.items, observed.items, expected.items, kItemCount);
  }
}

template<typename V, typename GenericOp, typename VecOp>
static void test_iop3(VecOp&& vec_op) noexcept {
  return test_iop3_constraint<V, GenericOp, ConstraintNone, VecOp>(BLInternal::forward<VecOp>(vec_op));
}

// SIMD - Tests - Integer Operations - Dispatcher
// ==============================================

template<uint32_t kW>
static BL_NOINLINE void test_integer(const char* ext) noexcept {
  using namespace SIMD;

  typedef Vec<kW, int8_t> V_I8;
  typedef Vec<kW, int16_t> V_I16;
  typedef Vec<kW, int32_t> V_I32;
  typedef Vec<kW, int64_t> V_I64;

  typedef Vec<kW, uint8_t> V_U8;
  typedef Vec<kW, uint16_t> V_U16;
  typedef Vec<kW, uint32_t> V_U32;
  typedef Vec<kW, uint64_t> V_U64;

  INFO("Testing %d-bit %s vector ops - make128_u[8|16|32|64]", kW*8, ext);
  {
    VecOverlay<16, uint8_t> a;
    VecOverlay<16, uint8_t> b;

    SIMD::storeu(a.data_u8, make128_u8(1));
    fill_val(b.data_u8, uint8_t(1), 16, 1u);
    verify_ivec(a.items, b.items, 16);

    SIMD::storeu(a.data_u8, make128_u8(2, 1));
    fill_val(b.data_u8, uint8_t(1), 16, 2u);
    verify_ivec(a.items, b.items, 16);

    SIMD::storeu(a.data_u8, make128_u8(4, 3, 2, 1));
    fill_val(b.data_u8, uint8_t(1), 16, 4u);
    verify_ivec(a.items, b.items, 16);

    SIMD::storeu(a.data_u8, make128_u8(8, 7, 6, 5, 4, 3, 2, 1));
    fill_val(b.data_u8, uint8_t(1), 16, 8u);
    verify_ivec(a.items, b.items, 16);

    SIMD::storeu(a.data_u8, make128_u8(16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1));
    fill_val(b.data_u8, uint8_t(1), 16, 16u);
    verify_ivec(a.items, b.items, 16);

    SIMD::storeu(a.data_u8, make128_u16(1));
    fill_val(b.data_u16, uint16_t(1), 16 / 2u, 1u);
    verify_ivec(a.items, b.items, 16);

    SIMD::storeu(a.data_u8, make128_u16(2, 1));
    fill_val(b.data_u16, uint16_t(1), 16 / 2u, 2u);
    verify_ivec(a.items, b.items, 16);

    SIMD::storeu(a.data_u8, make128_u16(4, 3, 2, 1));
    fill_val(b.data_u16, uint16_t(1), 16 / 2u, 4u);
    verify_ivec(a.items, b.items, 16);

    SIMD::storeu(a.data_u8, make128_u16(8, 7, 6, 5, 4, 3, 2, 1));
    fill_val(b.data_u16, uint16_t(1), 16 / 2u, 8u);
    verify_ivec(a.items, b.items, 16);

    SIMD::storeu(a.data_u8, make128_u32(1));
    fill_val(b.data_u32, uint32_t(1), 16 / 4u, 1u);
    verify_ivec(a.items, b.items, 16);

    SIMD::storeu(a.data_u8, make128_u32(2, 1));
    fill_val(b.data_u32, uint32_t(1), 16 / 4u, 2u);
    verify_ivec(a.items, b.items, 16);

    SIMD::storeu(a.data_u8, make128_u32(4, 3, 2, 1));
    fill_val(b.data_u32, uint32_t(1), 16 / 4u, 4u);
    verify_ivec(a.items, b.items, 16);

    SIMD::storeu(a.data_u8, make128_u64(1));
    fill_val(b.data_u64, uint64_t(1), 16 / 8u, 1u);
    verify_ivec(a.items, b.items, 16);

    SIMD::storeu(a.data_u8, make128_u64(2, 1));
    fill_val(b.data_u64, uint64_t(1), 16 / 8u, 2u);
    verify_ivec(a.items, b.items, 16);
  }

  INFO("Testing %d-bit %s vector ops - logical", kW*8, ext);
  {
    test_iop1<V_I8, iop_not<int8_t>>([](const V_I8& a) { return not_(a); });
    test_iop1<V_I16, iop_not<int16_t>>([](const V_I16& a) { return not_(a); });
    test_iop1<V_I32, iop_not<int32_t>>([](const V_I32& a) { return not_(a); });
    test_iop1<V_I64, iop_not<int64_t>>([](const V_I64& a) { return not_(a); });

    test_iop2<V_I8, iop_and<int8_t>>([](const V_I8& a, const V_I8& b) { return and_(a, b); });
    test_iop2<V_I16, iop_and<int16_t>>([](const V_I16& a, const V_I16& b) { return and_(a, b); });
    test_iop2<V_I32, iop_and<int32_t>>([](const V_I32& a, const V_I32& b) { return and_(a, b); });
    test_iop2<V_I64, iop_and<int64_t>>([](const V_I64& a, const V_I64& b) { return and_(a, b); });

    test_iop2<V_U8, iop_and<uint8_t>>([](const V_U8& a, const V_U8& b) { return and_(a, b); });
    test_iop2<V_U16, iop_and<uint16_t>>([](const V_U16& a, const V_U16& b) { return and_(a, b); });
    test_iop2<V_U32, iop_and<uint32_t>>([](const V_U32& a, const V_U32& b) { return and_(a, b); });
    test_iop2<V_U64, iop_and<uint64_t>>([](const V_U64& a, const V_U64& b) { return and_(a, b); });

    test_iop2<V_I8, iop_andnot<int8_t>>([](const V_I8& a, const V_I8& b) { return andnot(a, b); });
    test_iop2<V_I16, iop_andnot<int16_t>>([](const V_I16& a, const V_I16& b) { return andnot(a, b); });
    test_iop2<V_I32, iop_andnot<int32_t>>([](const V_I32& a, const V_I32& b) { return andnot(a, b); });
    test_iop2<V_I64, iop_andnot<int64_t>>([](const V_I64& a, const V_I64& b) { return andnot(a, b); });

    test_iop2<V_U8, iop_andnot<uint8_t>>([](const V_U8& a, const V_U8& b) { return andnot(a, b); });
    test_iop2<V_U16, iop_andnot<uint16_t>>([](const V_U16& a, const V_U16& b) { return andnot(a, b); });
    test_iop2<V_U32, iop_andnot<uint32_t>>([](const V_U32& a, const V_U32& b) { return andnot(a, b); });
    test_iop2<V_U64, iop_andnot<uint64_t>>([](const V_U64& a, const V_U64& b) { return andnot(a, b); });

    test_iop2<V_I8, iop_or<int8_t>>([](const V_I8& a, const V_I8& b) { return or_(a, b); });
    test_iop2<V_I16, iop_or<int16_t>>([](const V_I16& a, const V_I16& b) { return or_(a, b); });
    test_iop2<V_I32, iop_or<int32_t>>([](const V_I32& a, const V_I32& b) { return or_(a, b); });
    test_iop2<V_I64, iop_or<int64_t>>([](const V_I64& a, const V_I64& b) { return or_(a, b); });

    test_iop2<V_U8, iop_or<uint8_t>>([](const V_U8& a, const V_U8& b) { return or_(a, b); });
    test_iop2<V_U16, iop_or<uint16_t>>([](const V_U16& a, const V_U16& b) { return or_(a, b); });
    test_iop2<V_U32, iop_or<uint32_t>>([](const V_U32& a, const V_U32& b) { return or_(a, b); });
    test_iop2<V_U64, iop_or<uint64_t>>([](const V_U64& a, const V_U64& b) { return or_(a, b); });

    test_iop2<V_I8, iop_xor<int8_t>>([](const V_I8& a, const V_I8& b) { return xor_(a, b); });
    test_iop2<V_I16, iop_xor<int16_t>>([](const V_I16& a, const V_I16& b) { return xor_(a, b); });
    test_iop2<V_I32, iop_xor<int32_t>>([](const V_I32& a, const V_I32& b) { return xor_(a, b); });
    test_iop2<V_I64, iop_xor<int64_t>>([](const V_I64& a, const V_I64& b) { return xor_(a, b); });

    test_iop2<V_U8, iop_xor<uint8_t>>([](const V_U8& a, const V_U8& b) { return xor_(a, b); });
    test_iop2<V_U16, iop_xor<uint16_t>>([](const V_U16& a, const V_U16& b) { return xor_(a, b); });
    test_iop2<V_U32, iop_xor<uint32_t>>([](const V_U32& a, const V_U32& b) { return xor_(a, b); });
    test_iop2<V_U64, iop_xor<uint64_t>>([](const V_U64& a, const V_U64& b) { return xor_(a, b); });
  }

  INFO("Testing %d-bit %s vector ops - blendv", kW*8, ext);
  {
    test_iop3<V_I8, iop_blendv_bits<int8_t>>([](const V_I8& a, const V_I8& b, const V_I8& c) { return blendv_bits(a, b, c); });
    test_iop3<V_I16, iop_blendv_bits<int16_t>>([](const V_I16& a, const V_I16& b, const V_I16& c) { return blendv_bits(a, b, c); });
    test_iop3<V_I32, iop_blendv_bits<int32_t>>([](const V_I32& a, const V_I32& b, const V_I32& c) { return blendv_bits(a, b, c); });
    test_iop3<V_I64, iop_blendv_bits<int64_t>>([](const V_I64& a, const V_I64& b, const V_I64& c) { return blendv_bits(a, b, c); });

    test_iop3<V_U8, iop_blendv_bits<uint8_t>>([](const V_U8& a, const V_U8& b, const V_U8& c) { return blendv_bits(a, b, c); });
    test_iop3<V_U16, iop_blendv_bits<uint16_t>>([](const V_U16& a, const V_U16& b, const V_U16& c) { return blendv_bits(a, b, c); });
    test_iop3<V_U32, iop_blendv_bits<uint32_t>>([](const V_U32& a, const V_U32& b, const V_U32& c) { return blendv_bits(a, b, c); });
    test_iop3<V_U64, iop_blendv_bits<uint64_t>>([](const V_U64& a, const V_U64& b, const V_U64& c) { return blendv_bits(a, b, c); });
  }

  INFO("Testing %d-bit %s vector ops - abs", kW*8, ext);
  {
    test_iop1<V_I8, iop_abs<int8_t>>([](const V_I8& a) { return abs(a); });
    test_iop1<V_I16, iop_abs<int16_t>>([](const V_I16& a) { return abs(a); });
    test_iop1<V_I32, iop_abs<int32_t>>([](const V_I32& a) { return abs(a); });
    test_iop1<V_I64, iop_abs<int64_t>>([](const V_I64& a) { return abs(a); });

    test_iop1<V_I8, iop_abs<int8_t>>([](const V_I8& a) { return abs_i8(a); });
    test_iop1<V_I16, iop_abs<int16_t>>([](const V_I16& a) { return abs_i16(a); });
    test_iop1<V_I32, iop_abs<int32_t>>([](const V_I32& a) { return abs_i32(a); });
    test_iop1<V_I64, iop_abs<int64_t>>([](const V_I64& a) { return abs_i64(a); });
  }

  INFO("Testing %d-bit %s vector ops - add / adds", kW*8, ext);
  {
    test_iop2<V_I8, iop_add<int8_t>>([](const V_I8& a, const V_I8& b) { return add(a, b); });
    test_iop2<V_I16, iop_add<int16_t>>([](const V_I16& a, const V_I16& b) { return add(a, b); });
    test_iop2<V_I32, iop_add<int32_t>>([](const V_I32& a, const V_I32& b) { return add(a, b); });
    test_iop2<V_I64, iop_add<int64_t>>([](const V_I64& a, const V_I64& b) { return add(a, b); });

    test_iop2<V_I8, iop_add<int8_t>>([](const V_I8& a, const V_I8& b) { return add_i8(a, b); });
    test_iop2<V_I16, iop_add<int16_t>>([](const V_I16& a, const V_I16& b) { return add_i16(a, b); });
    test_iop2<V_I32, iop_add<int32_t>>([](const V_I32& a, const V_I32& b) { return add_i32(a, b); });
    test_iop2<V_I64, iop_add<int64_t>>([](const V_I64& a, const V_I64& b) { return add_i64(a, b); });

    test_iop2<V_U8, iop_add<uint8_t>>([](const V_U8& a, const V_U8& b) { return add(a, b); });
    test_iop2<V_U16, iop_add<uint16_t>>([](const V_U16& a, const V_U16& b) { return add(a, b); });
    test_iop2<V_U32, iop_add<uint32_t>>([](const V_U32& a, const V_U32& b) { return add(a, b); });
    test_iop2<V_U64, iop_add<uint64_t>>([](const V_U64& a, const V_U64& b) { return add(a, b); });

    test_iop2<V_U8, iop_add<uint8_t>>([](const V_U8& a, const V_U8& b) { return add_u8(a, b); });
    test_iop2<V_U16, iop_add<uint16_t>>([](const V_U16& a, const V_U16& b) { return add_u16(a, b); });
    test_iop2<V_U32, iop_add<uint32_t>>([](const V_U32& a, const V_U32& b) { return add_u32(a, b); });
    test_iop2<V_U64, iop_add<uint64_t>>([](const V_U64& a, const V_U64& b) { return add_u64(a, b); });

    test_iop2<V_I8, iop_adds<int8_t>>([](const V_I8& a, const V_I8& b) { return adds(a, b); });
    test_iop2<V_I16, iop_adds<int16_t>>([](const V_I16& a, const V_I16& b) { return adds(a, b); });

    test_iop2<V_I8, iop_adds<int8_t>>([](const V_I8& a, const V_I8& b) { return adds_i8(a, b); });
    test_iop2<V_I16, iop_adds<int16_t>>([](const V_I16& a, const V_I16& b) { return adds_i16(a, b); });

    test_iop2<V_U8, iop_adds<uint8_t>>([](const V_U8& a, const V_U8& b) { return adds(a, b); });
    test_iop2<V_U16, iop_adds<uint16_t>>([](const V_U16& a, const V_U16& b) { return adds(a, b); });

    test_iop2<V_U8, iop_adds<uint8_t>>([](const V_U8& a, const V_U8& b) { return adds_u8(a, b); });
    test_iop2<V_U16, iop_adds<uint16_t>>([](const V_U16& a, const V_U16& b) { return adds_u16(a, b); });
  }

  INFO("Testing %d-bit %s vector ops - sub / subs", kW*8, ext);
  {
    test_iop2<V_I8, iop_sub<int8_t>>([](const V_I8& a, const V_I8& b) { return sub(a, b); });
    test_iop2<V_I16, iop_sub<int16_t>>([](const V_I16& a, const V_I16& b) { return sub(a, b); });
    test_iop2<V_I32, iop_sub<int32_t>>([](const V_I32& a, const V_I32& b) { return sub(a, b); });
    test_iop2<V_I64, iop_sub<int64_t>>([](const V_I64& a, const V_I64& b) { return sub(a, b); });

    test_iop2<V_I8, iop_sub<int8_t>>([](const V_I8& a, const V_I8& b) { return sub_i8(a, b); });
    test_iop2<V_I16, iop_sub<int16_t>>([](const V_I16& a, const V_I16& b) { return sub_i16(a, b); });
    test_iop2<V_I32, iop_sub<int32_t>>([](const V_I32& a, const V_I32& b) { return sub_i32(a, b); });
    test_iop2<V_I64, iop_sub<int64_t>>([](const V_I64& a, const V_I64& b) { return sub_i64(a, b); });

    test_iop2<V_U8, iop_sub<uint8_t>>([](const V_U8& a, const V_U8& b) { return sub(a, b); });
    test_iop2<V_U16, iop_sub<uint16_t>>([](const V_U16& a, const V_U16& b) { return sub(a, b); });
    test_iop2<V_U32, iop_sub<uint32_t>>([](const V_U32& a, const V_U32& b) { return sub(a, b); });
    test_iop2<V_U64, iop_sub<uint64_t>>([](const V_U64& a, const V_U64& b) { return sub(a, b); });

    test_iop2<V_U8, iop_sub<uint8_t>>([](const V_U8& a, const V_U8& b) { return sub_u8(a, b); });
    test_iop2<V_U16, iop_sub<uint16_t>>([](const V_U16& a, const V_U16& b) { return sub_u16(a, b); });
    test_iop2<V_U32, iop_sub<uint32_t>>([](const V_U32& a, const V_U32& b) { return sub_u32(a, b); });
    test_iop2<V_U64, iop_sub<uint64_t>>([](const V_U64& a, const V_U64& b) { return sub_u64(a, b); });

    test_iop2<V_I8, iop_subs<int8_t>>([](const V_I8& a, const V_I8& b) { return subs(a, b); });
    test_iop2<V_I16, iop_subs<int16_t>>([](const V_I16& a, const V_I16& b) { return subs(a, b); });

    test_iop2<V_I8, iop_subs<int8_t>>([](const V_I8& a, const V_I8& b) { return subs_i8(a, b); });
    test_iop2<V_I16, iop_subs<int16_t>>([](const V_I16& a, const V_I16& b) { return subs_i16(a, b); });

    test_iop2<V_U8, iop_subs<uint8_t>>([](const V_U8& a, const V_U8& b) { return subs(a, b); });
    test_iop2<V_U16, iop_subs<uint16_t>>([](const V_U16& a, const V_U16& b) { return subs(a, b); });

    test_iop2<V_U8, iop_subs<uint8_t>>([](const V_U8& a, const V_U8& b) { return subs_u8(a, b); });
    test_iop2<V_U16, iop_subs<uint16_t>>([](const V_U16& a, const V_U16& b) { return subs_u16(a, b); });
  }

  INFO("Testing %d-bit %s vector ops - mul", kW*8, ext);
  {
    test_iop2<V_I16, iop_mul<int16_t>>([](const V_I16& a, const V_I16& b) { return mul(a, b); });
    test_iop2<V_I32, iop_mul<int32_t>>([](const V_I32& a, const V_I32& b) { return mul(a, b); });
    test_iop2<V_I64, iop_mul<int64_t>>([](const V_I64& a, const V_I64& b) { return mul(a, b); });
    test_iop2<V_U16, iop_mul<uint16_t>>([](const V_U16& a, const V_U16& b) { return mul(a, b); });
    test_iop2<V_U32, iop_mul<uint32_t>>([](const V_U32& a, const V_U32& b) { return mul(a, b); });
    test_iop2<V_U64, iop_mul<uint64_t>>([](const V_U64& a, const V_U64& b) { return mul(a, b); });

    test_iop2<V_I16, iop_mul<int16_t>>([](const V_I16& a, const V_I16& b) { return mul_i16(a, b); });
    test_iop2<V_I32, iop_mul<int32_t>>([](const V_I32& a, const V_I32& b) { return mul_i32(a, b); });
    test_iop2<V_I64, iop_mul<int64_t>>([](const V_I64& a, const V_I64& b) { return mul_i64(a, b); });
    test_iop2<V_U16, iop_mul<uint16_t>>([](const V_U16& a, const V_U16& b) { return mul_u16(a, b); });
    test_iop2<V_U32, iop_mul<uint32_t>>([](const V_U32& a, const V_U32& b) { return mul_u32(a, b); });
    test_iop2<V_U64, iop_mul<uint64_t>>([](const V_U64& a, const V_U64& b) { return mul_u64(a, b); });
  }

  INFO("Testing %d-bit %s vector ops - cmp", kW*8, ext);
  {
    test_iop2<V_I8, iop_cmp_eq<int8_t>>([](const V_I8& a, const V_I8& b) { return cmp_eq(a, b); });
    test_iop2<V_I16, iop_cmp_eq<int16_t>>([](const V_I16& a, const V_I16& b) { return cmp_eq(a, b); });
    test_iop2<V_I32, iop_cmp_eq<int32_t>>([](const V_I32& a, const V_I32& b) { return cmp_eq(a, b); });
    test_iop2<V_I64, iop_cmp_eq<int64_t>>([](const V_I64& a, const V_I64& b) { return cmp_eq(a, b); });

    test_iop2<V_I8, iop_cmp_eq<int8_t>>([](const V_I8& a, const V_I8& b) { return cmp_eq_i8(a, b); });
    test_iop2<V_I16, iop_cmp_eq<int16_t>>([](const V_I16& a, const V_I16& b) { return cmp_eq_i16(a, b); });
    test_iop2<V_I32, iop_cmp_eq<int32_t>>([](const V_I32& a, const V_I32& b) { return cmp_eq_i32(a, b); });
    test_iop2<V_I64, iop_cmp_eq<int64_t>>([](const V_I64& a, const V_I64& b) { return cmp_eq_i64(a, b); });

    test_iop2<V_U8, iop_cmp_eq<uint8_t>>([](const V_U8& a, const V_U8& b) { return cmp_eq(a, b); });
    test_iop2<V_U16, iop_cmp_eq<uint16_t>>([](const V_U16& a, const V_U16& b) { return cmp_eq(a, b); });
    test_iop2<V_U32, iop_cmp_eq<uint32_t>>([](const V_U32& a, const V_U32& b) { return cmp_eq(a, b); });
    test_iop2<V_U64, iop_cmp_eq<uint64_t>>([](const V_U64& a, const V_U64& b) { return cmp_eq(a, b); });

    test_iop2<V_U8, iop_cmp_eq<uint8_t>>([](const V_U8& a, const V_U8& b) { return cmp_eq_u8(a, b); });
    test_iop2<V_U16, iop_cmp_eq<uint16_t>>([](const V_U16& a, const V_U16& b) { return cmp_eq_u16(a, b); });
    test_iop2<V_U32, iop_cmp_eq<uint32_t>>([](const V_U32& a, const V_U32& b) { return cmp_eq_u32(a, b); });
    test_iop2<V_U64, iop_cmp_eq<uint64_t>>([](const V_U64& a, const V_U64& b) { return cmp_eq_u64(a, b); });

    test_iop2<V_I8, iop_cmp_ne<int8_t>>([](const V_I8& a, const V_I8& b) { return cmp_ne(a, b); });
    test_iop2<V_I16, iop_cmp_ne<int16_t>>([](const V_I16& a, const V_I16& b) { return cmp_ne(a, b); });
    test_iop2<V_I32, iop_cmp_ne<int32_t>>([](const V_I32& a, const V_I32& b) { return cmp_ne(a, b); });
    test_iop2<V_I64, iop_cmp_ne<int64_t>>([](const V_I64& a, const V_I64& b) { return cmp_ne(a, b); });

    test_iop2<V_I8, iop_cmp_ne<int8_t>>([](const V_I8& a, const V_I8& b) { return cmp_ne_i8(a, b); });
    test_iop2<V_I16, iop_cmp_ne<int16_t>>([](const V_I16& a, const V_I16& b) { return cmp_ne_i16(a, b); });
    test_iop2<V_I32, iop_cmp_ne<int32_t>>([](const V_I32& a, const V_I32& b) { return cmp_ne_i32(a, b); });
    test_iop2<V_I64, iop_cmp_ne<int64_t>>([](const V_I64& a, const V_I64& b) { return cmp_ne_i64(a, b); });

    test_iop2<V_U8, iop_cmp_ne<uint8_t>>([](const V_U8& a, const V_U8& b) { return cmp_ne(a, b); });
    test_iop2<V_U16, iop_cmp_ne<uint16_t>>([](const V_U16& a, const V_U16& b) { return cmp_ne(a, b); });
    test_iop2<V_U32, iop_cmp_ne<uint32_t>>([](const V_U32& a, const V_U32& b) { return cmp_ne(a, b); });
    test_iop2<V_U64, iop_cmp_ne<uint64_t>>([](const V_U64& a, const V_U64& b) { return cmp_ne(a, b); });

    test_iop2<V_U8, iop_cmp_ne<uint8_t>>([](const V_U8& a, const V_U8& b) { return cmp_ne_u8(a, b); });
    test_iop2<V_U16, iop_cmp_ne<uint16_t>>([](const V_U16& a, const V_U16& b) { return cmp_ne_u16(a, b); });
    test_iop2<V_U32, iop_cmp_ne<uint32_t>>([](const V_U32& a, const V_U32& b) { return cmp_ne_u32(a, b); });
    test_iop2<V_U64, iop_cmp_ne<uint64_t>>([](const V_U64& a, const V_U64& b) { return cmp_ne_u64(a, b); });

    test_iop2<V_I8, iop_cmp_gt<int8_t>>([](const V_I8& a, const V_I8& b) { return cmp_gt(a, b); });
    test_iop2<V_I16, iop_cmp_gt<int16_t>>([](const V_I16& a, const V_I16& b) { return cmp_gt(a, b); });
    test_iop2<V_I32, iop_cmp_gt<int32_t>>([](const V_I32& a, const V_I32& b) { return cmp_gt(a, b); });
    test_iop2<V_I64, iop_cmp_gt<int64_t>>([](const V_I64& a, const V_I64& b) { return cmp_gt(a, b); });

    test_iop2<V_U8, iop_cmp_gt<uint8_t>>([](const V_U8& a, const V_U8& b) { return cmp_gt(a, b); });
    test_iop2<V_U16, iop_cmp_gt<uint16_t>>([](const V_U16& a, const V_U16& b) { return cmp_gt(a, b); });
    test_iop2<V_U32, iop_cmp_gt<uint32_t>>([](const V_U32& a, const V_U32& b) { return cmp_gt(a, b); });
    test_iop2<V_U64, iop_cmp_gt<uint64_t>>([](const V_U64& a, const V_U64& b) { return cmp_gt(a, b); });

    test_iop2<V_I8, iop_cmp_gt<int8_t>>([](const V_I8& a, const V_I8& b) { return cmp_gt_i8(a, b); });
    test_iop2<V_I16, iop_cmp_gt<int16_t>>([](const V_I16& a, const V_I16& b) { return cmp_gt_i16(a, b); });
    test_iop2<V_I32, iop_cmp_gt<int32_t>>([](const V_I32& a, const V_I32& b) { return cmp_gt_i32(a, b); });
    test_iop2<V_I64, iop_cmp_gt<int64_t>>([](const V_I64& a, const V_I64& b) { return cmp_gt_i64(a, b); });

    test_iop2<V_U8, iop_cmp_gt<uint8_t>>([](const V_U8& a, const V_U8& b) { return cmp_gt_u8(a, b); });
    test_iop2<V_U16, iop_cmp_gt<uint16_t>>([](const V_U16& a, const V_U16& b) { return cmp_gt_u16(a, b); });
    test_iop2<V_U32, iop_cmp_gt<uint32_t>>([](const V_U32& a, const V_U32& b) { return cmp_gt_u32(a, b); });
    test_iop2<V_U64, iop_cmp_gt<uint64_t>>([](const V_U64& a, const V_U64& b) { return cmp_gt_u64(a, b); });

    test_iop2<V_I8, iop_cmp_ge<int8_t>>([](const V_I8& a, const V_I8& b) { return cmp_ge(a, b); });
    test_iop2<V_I16, iop_cmp_ge<int16_t>>([](const V_I16& a, const V_I16& b) { return cmp_ge(a, b); });
    test_iop2<V_I32, iop_cmp_ge<int32_t>>([](const V_I32& a, const V_I32& b) { return cmp_ge(a, b); });
    test_iop2<V_I64, iop_cmp_ge<int64_t>>([](const V_I64& a, const V_I64& b) { return cmp_ge(a, b); });

    test_iop2<V_U8, iop_cmp_ge<uint8_t>>([](const V_U8& a, const V_U8& b) { return cmp_ge(a, b); });
    test_iop2<V_U16, iop_cmp_ge<uint16_t>>([](const V_U16& a, const V_U16& b) { return cmp_ge(a, b); });
    test_iop2<V_U32, iop_cmp_ge<uint32_t>>([](const V_U32& a, const V_U32& b) { return cmp_ge(a, b); });
    test_iop2<V_U64, iop_cmp_ge<uint64_t>>([](const V_U64& a, const V_U64& b) { return cmp_ge(a, b); });

    test_iop2<V_I8, iop_cmp_ge<int8_t>>([](const V_I8& a, const V_I8& b) { return cmp_ge_i8(a, b); });
    test_iop2<V_I16, iop_cmp_ge<int16_t>>([](const V_I16& a, const V_I16& b) { return cmp_ge_i16(a, b); });
    test_iop2<V_I32, iop_cmp_ge<int32_t>>([](const V_I32& a, const V_I32& b) { return cmp_ge_i32(a, b); });
    test_iop2<V_I64, iop_cmp_ge<int64_t>>([](const V_I64& a, const V_I64& b) { return cmp_ge_i64(a, b); });

    test_iop2<V_U8, iop_cmp_ge<uint8_t>>([](const V_U8& a, const V_U8& b) { return cmp_ge_u8(a, b); });
    test_iop2<V_U16, iop_cmp_ge<uint16_t>>([](const V_U16& a, const V_U16& b) { return cmp_ge_u16(a, b); });
    test_iop2<V_U32, iop_cmp_ge<uint32_t>>([](const V_U32& a, const V_U32& b) { return cmp_ge_u32(a, b); });
    test_iop2<V_U64, iop_cmp_ge<uint64_t>>([](const V_U64& a, const V_U64& b) { return cmp_ge_u64(a, b); });

    test_iop2<V_I8, iop_cmp_lt<int8_t>>([](const V_I8& a, const V_I8& b) { return cmp_lt(a, b); });
    test_iop2<V_I16, iop_cmp_lt<int16_t>>([](const V_I16& a, const V_I16& b) { return cmp_lt(a, b); });
    test_iop2<V_I32, iop_cmp_lt<int32_t>>([](const V_I32& a, const V_I32& b) { return cmp_lt(a, b); });
    test_iop2<V_I64, iop_cmp_lt<int64_t>>([](const V_I64& a, const V_I64& b) { return cmp_lt(a, b); });

    test_iop2<V_U8, iop_cmp_lt<uint8_t>>([](const V_U8& a, const V_U8& b) { return cmp_lt(a, b); });
    test_iop2<V_U16, iop_cmp_lt<uint16_t>>([](const V_U16& a, const V_U16& b) { return cmp_lt(a, b); });
    test_iop2<V_U32, iop_cmp_lt<uint32_t>>([](const V_U32& a, const V_U32& b) { return cmp_lt(a, b); });
    test_iop2<V_U64, iop_cmp_lt<uint64_t>>([](const V_U64& a, const V_U64& b) { return cmp_lt(a, b); });

    test_iop2<V_I8, iop_cmp_lt<int8_t>>([](const V_I8& a, const V_I8& b) { return cmp_lt_i8(a, b); });
    test_iop2<V_I16, iop_cmp_lt<int16_t>>([](const V_I16& a, const V_I16& b) { return cmp_lt_i16(a, b); });
    test_iop2<V_I32, iop_cmp_lt<int32_t>>([](const V_I32& a, const V_I32& b) { return cmp_lt_i32(a, b); });
    test_iop2<V_I64, iop_cmp_lt<int64_t>>([](const V_I64& a, const V_I64& b) { return cmp_lt_i64(a, b); });

    test_iop2<V_U8, iop_cmp_lt<uint8_t>>([](const V_U8& a, const V_U8& b) { return cmp_lt_u8(a, b); });
    test_iop2<V_U16, iop_cmp_lt<uint16_t>>([](const V_U16& a, const V_U16& b) { return cmp_lt_u16(a, b); });
    test_iop2<V_U32, iop_cmp_lt<uint32_t>>([](const V_U32& a, const V_U32& b) { return cmp_lt_u32(a, b); });
    test_iop2<V_U64, iop_cmp_lt<uint64_t>>([](const V_U64& a, const V_U64& b) { return cmp_lt_u64(a, b); });

    test_iop2<V_I8, iop_cmp_le<int8_t>>([](const V_I8& a, const V_I8& b) { return cmp_le(a, b); });
    test_iop2<V_I16, iop_cmp_le<int16_t>>([](const V_I16& a, const V_I16& b) { return cmp_le(a, b); });
    test_iop2<V_I32, iop_cmp_le<int32_t>>([](const V_I32& a, const V_I32& b) { return cmp_le(a, b); });
    test_iop2<V_I64, iop_cmp_le<int64_t>>([](const V_I64& a, const V_I64& b) { return cmp_le(a, b); });

    test_iop2<V_U8, iop_cmp_le<uint8_t>>([](const V_U8& a, const V_U8& b) { return cmp_le(a, b); });
    test_iop2<V_U16, iop_cmp_le<uint16_t>>([](const V_U16& a, const V_U16& b) { return cmp_le(a, b); });
    test_iop2<V_U32, iop_cmp_le<uint32_t>>([](const V_U32& a, const V_U32& b) { return cmp_le(a, b); });
    test_iop2<V_U64, iop_cmp_le<uint64_t>>([](const V_U64& a, const V_U64& b) { return cmp_le(a, b); });

    test_iop2<V_I8, iop_cmp_le<int8_t>>([](const V_I8& a, const V_I8& b) { return cmp_le_i8(a, b); });
    test_iop2<V_I16, iop_cmp_le<int16_t>>([](const V_I16& a, const V_I16& b) { return cmp_le_i16(a, b); });
    test_iop2<V_I32, iop_cmp_le<int32_t>>([](const V_I32& a, const V_I32& b) { return cmp_le_i32(a, b); });
    test_iop2<V_I64, iop_cmp_le<int64_t>>([](const V_I64& a, const V_I64& b) { return cmp_le_i64(a, b); });

    test_iop2<V_U8, iop_cmp_le<uint8_t>>([](const V_U8& a, const V_U8& b) { return cmp_le_u8(a, b); });
    test_iop2<V_U16, iop_cmp_le<uint16_t>>([](const V_U16& a, const V_U16& b) { return cmp_le_u16(a, b); });
    test_iop2<V_U32, iop_cmp_le<uint32_t>>([](const V_U32& a, const V_U32& b) { return cmp_le_u32(a, b); });
    test_iop2<V_U64, iop_cmp_le<uint64_t>>([](const V_U64& a, const V_U64& b) { return cmp_le_u64(a, b); });
  }

  INFO("Testing %d-bit %s vector ops - min / max", kW*8, ext);
  {
    test_iop2<V_I8, iop_min<int8_t>>([](const V_I8& a, const V_I8& b) { return min(a, b); });
    test_iop2<V_I16, iop_min<int16_t>>([](const V_I16& a, const V_I16& b) { return min(a, b); });
    test_iop2<V_I32, iop_min<int32_t>>([](const V_I32& a, const V_I32& b) { return min(a, b); });
    test_iop2<V_I64, iop_min<int64_t>>([](const V_I64& a, const V_I64& b) { return min(a, b); });

    test_iop2<V_I8, iop_min<int8_t>>([](const V_I8& a, const V_I8& b) { return min_i8(a, b); });
    test_iop2<V_I16, iop_min<int16_t>>([](const V_I16& a, const V_I16& b) { return min_i16(a, b); });
    test_iop2<V_I32, iop_min<int32_t>>([](const V_I32& a, const V_I32& b) { return min_i32(a, b); });
    test_iop2<V_I64, iop_min<int64_t>>([](const V_I64& a, const V_I64& b) { return min_i64(a, b); });

    test_iop2<V_U8, iop_min<uint8_t>>([](const V_U8& a, const V_U8& b) { return min(a, b); });
    test_iop2<V_U16, iop_min<uint16_t>>([](const V_U16& a, const V_U16& b) { return min(a, b); });
    test_iop2<V_U32, iop_min<uint32_t>>([](const V_U32& a, const V_U32& b) { return min(a, b); });
    test_iop2<V_U64, iop_min<uint64_t>>([](const V_U64& a, const V_U64& b) { return min(a, b); });

    test_iop2<V_U8, iop_min<uint8_t>>([](const V_U8& a, const V_U8& b) { return min_u8(a, b); });
    test_iop2<V_U16, iop_min<uint16_t>>([](const V_U16& a, const V_U16& b) { return min_u16(a, b); });
    test_iop2<V_U32, iop_min<uint32_t>>([](const V_U32& a, const V_U32& b) { return min_u32(a, b); });
    test_iop2<V_U64, iop_min<uint64_t>>([](const V_U64& a, const V_U64& b) { return min_u64(a, b); });

    test_iop2<V_I8, iop_max<int8_t>>([](const V_I8& a, const V_I8& b) { return max(a, b); });
    test_iop2<V_I16, iop_max<int16_t>>([](const V_I16& a, const V_I16& b) { return max(a, b); });
    test_iop2<V_I32, iop_max<int32_t>>([](const V_I32& a, const V_I32& b) { return max(a, b); });
    test_iop2<V_I64, iop_max<int64_t>>([](const V_I64& a, const V_I64& b) { return max(a, b); });

    test_iop2<V_I8, iop_max<int8_t>>([](const V_I8& a, const V_I8& b) { return max_i8(a, b); });
    test_iop2<V_I16, iop_max<int16_t>>([](const V_I16& a, const V_I16& b) { return max_i16(a, b); });
    test_iop2<V_I32, iop_max<int32_t>>([](const V_I32& a, const V_I32& b) { return max_i32(a, b); });
    test_iop2<V_I64, iop_max<int64_t>>([](const V_I64& a, const V_I64& b) { return max_i64(a, b); });

    test_iop2<V_U8, iop_max<uint8_t>>([](const V_U8& a, const V_U8& b) { return max(a, b); });
    test_iop2<V_U16, iop_max<uint16_t>>([](const V_U16& a, const V_U16& b) { return max(a, b); });
    test_iop2<V_U32, iop_max<uint32_t>>([](const V_U32& a, const V_U32& b) { return max(a, b); });
    test_iop2<V_U64, iop_max<uint64_t>>([](const V_U64& a, const V_U64& b) { return max(a, b); });

    test_iop2<V_U8, iop_max<uint8_t>>([](const V_U8& a, const V_U8& b) { return max_u8(a, b); });
    test_iop2<V_U16, iop_max<uint16_t>>([](const V_U16& a, const V_U16& b) { return max_u16(a, b); });
    test_iop2<V_U32, iop_max<uint32_t>>([](const V_U32& a, const V_U32& b) { return max_u32(a, b); });
    test_iop2<V_U64, iop_max<uint64_t>>([](const V_U64& a, const V_U64& b) { return max_u64(a, b); });
  }

  INFO("Testing %d-bit %s vector ops - bit shift", kW*8, ext);
  {
    test_iop1<V_I8, iop_slli<int8_t, 1>>([](const V_I8& a) { return slli<1>(a); });
    test_iop1<V_I16, iop_slli<int16_t, 1>>([](const V_I16& a) { return slli<1>(a); });
    test_iop1<V_I32, iop_slli<int32_t, 1>>([](const V_I32& a) { return slli<1>(a); });
    test_iop1<V_I64, iop_slli<int64_t, 1>>([](const V_I64& a) { return slli<1>(a); });

    test_iop1<V_U8, iop_slli<uint8_t, 1>>([](const V_U8& a) { return slli<1>(a); });
    test_iop1<V_U16, iop_slli<uint16_t, 1>>([](const V_U16& a) { return slli<1>(a); });
    test_iop1<V_U32, iop_slli<uint32_t, 1>>([](const V_U32& a) { return slli<1>(a); });
    test_iop1<V_U64, iop_slli<uint64_t, 1>>([](const V_U64& a) { return slli<1>(a); });

    test_iop1<V_I8, iop_slli<int8_t, 1>>([](const V_I8& a) { return slli_i8<1>(a); });
    test_iop1<V_I16, iop_slli<int16_t, 1>>([](const V_I16& a) { return slli_i16<1>(a); });
    test_iop1<V_I32, iop_slli<int32_t, 1>>([](const V_I32& a) { return slli_i32<1>(a); });
    test_iop1<V_I64, iop_slli<int64_t, 1>>([](const V_I64& a) { return slli_i64<1>(a); });

    test_iop1<V_U8, iop_slli<uint8_t, 1>>([](const V_U8& a) { return slli_u8<1>(a); });
    test_iop1<V_U16, iop_slli<uint16_t, 1>>([](const V_U16& a) { return slli_u16<1>(a); });
    test_iop1<V_U32, iop_slli<uint32_t, 1>>([](const V_U32& a) { return slli_u32<1>(a); });
    test_iop1<V_U64, iop_slli<uint64_t, 1>>([](const V_U64& a) { return slli_u64<1>(a); });

    test_iop1<V_I8, iop_slli<int8_t, 5>>([](const V_I8& a) { return slli<5>(a); });
    test_iop1<V_I16, iop_slli<int16_t, 5>>([](const V_I16& a) { return slli<5>(a); });
    test_iop1<V_I32, iop_slli<int32_t, 5>>([](const V_I32& a) { return slli<5>(a); });
    test_iop1<V_I64, iop_slli<int64_t, 5>>([](const V_I64& a) { return slli<5>(a); });

    test_iop1<V_U8, iop_slli<uint8_t, 5>>([](const V_U8& a) { return slli<5>(a); });
    test_iop1<V_U16, iop_slli<uint16_t, 5>>([](const V_U16& a) { return slli<5>(a); });
    test_iop1<V_U32, iop_slli<uint32_t, 5>>([](const V_U32& a) { return slli<5>(a); });
    test_iop1<V_U64, iop_slli<uint64_t, 5>>([](const V_U64& a) { return slli<5>(a); });

    test_iop1<V_I8, iop_slli<int8_t, 5>>([](const V_I8& a) { return slli_i8<5>(a); });
    test_iop1<V_I16, iop_slli<int16_t, 5>>([](const V_I16& a) { return slli_i16<5>(a); });
    test_iop1<V_I32, iop_slli<int32_t, 5>>([](const V_I32& a) { return slli_i32<5>(a); });
    test_iop1<V_I64, iop_slli<int64_t, 5>>([](const V_I64& a) { return slli_i64<5>(a); });

    test_iop1<V_U8, iop_slli<uint8_t, 5>>([](const V_U8& a) { return slli_u8<5>(a); });
    test_iop1<V_U16, iop_slli<uint16_t, 5>>([](const V_U16& a) { return slli_u16<5>(a); });
    test_iop1<V_U32, iop_slli<uint32_t, 5>>([](const V_U32& a) { return slli_u32<5>(a); });
    test_iop1<V_U64, iop_slli<uint64_t, 5>>([](const V_U64& a) { return slli_u64<5>(a); });

    test_iop1<V_I8, iop_slli<int8_t, 7>>([](const V_I8& a) { return slli<7>(a); });
    test_iop1<V_I16, iop_slli<int16_t, 15>>([](const V_I16& a) { return slli<15>(a); });
    test_iop1<V_I32, iop_slli<int32_t, 31>>([](const V_I32& a) { return slli<31>(a); });
    test_iop1<V_I64, iop_slli<int64_t, 63>>([](const V_I64& a) { return slli<63>(a); });

    test_iop1<V_U8, iop_slli<uint8_t, 7>>([](const V_U8& a) { return slli<7>(a); });
    test_iop1<V_U16, iop_slli<uint16_t, 15>>([](const V_U16& a) { return slli<15>(a); });
    test_iop1<V_U32, iop_slli<uint32_t, 31>>([](const V_U32& a) { return slli<31>(a); });
    test_iop1<V_U64, iop_slli<uint64_t, 63>>([](const V_U64& a) { return slli<63>(a); });

    test_iop1<V_I8, iop_slli<int8_t, 7>>([](const V_I8& a) { return slli_i8<7>(a); });
    test_iop1<V_I16, iop_slli<int16_t, 15>>([](const V_I16& a) { return slli_i16<15>(a); });
    test_iop1<V_I32, iop_slli<int32_t, 31>>([](const V_I32& a) { return slli_i32<31>(a); });
    test_iop1<V_I64, iop_slli<int64_t, 63>>([](const V_I64& a) { return slli_i64<63>(a); });

    test_iop1<V_U8, iop_slli<uint8_t, 7>>([](const V_U8& a) { return slli_u8<7>(a); });
    test_iop1<V_U16, iop_slli<uint16_t, 15>>([](const V_U16& a) { return slli_u16<15>(a); });
    test_iop1<V_U32, iop_slli<uint32_t, 31>>([](const V_U32& a) { return slli_u32<31>(a); });
    test_iop1<V_U64, iop_slli<uint64_t, 63>>([](const V_U64& a) { return slli_u64<63>(a); });

    test_iop1<V_I8, iop_srli<int8_t, 1>>([](const V_I8& a) { return srli<1>(a); });
    test_iop1<V_I16, iop_srli<int16_t, 1>>([](const V_I16& a) { return srli<1>(a); });
    test_iop1<V_I32, iop_srli<int32_t, 1>>([](const V_I32& a) { return srli<1>(a); });
    test_iop1<V_I64, iop_srli<int64_t, 1>>([](const V_I64& a) { return srli<1>(a); });

    test_iop1<V_U8, iop_srli<uint8_t, 1>>([](const V_U8& a) { return srli<1>(a); });
    test_iop1<V_U16, iop_srli<uint16_t, 1>>([](const V_U16& a) { return srli<1>(a); });
    test_iop1<V_U32, iop_srli<uint32_t, 1>>([](const V_U32& a) { return srli<1>(a); });
    test_iop1<V_U64, iop_srli<uint64_t, 1>>([](const V_U64& a) { return srli<1>(a); });

    test_iop1<V_U8, iop_srli<uint8_t, 1>>([](const V_U8& a) { return srli_u8<1>(a); });
    test_iop1<V_U16, iop_srli<uint16_t, 1>>([](const V_U16& a) { return srli_u16<1>(a); });
    test_iop1<V_U32, iop_srli<uint32_t, 1>>([](const V_U32& a) { return srli_u32<1>(a); });
    test_iop1<V_U64, iop_srli<uint64_t, 1>>([](const V_U64& a) { return srli_u64<1>(a); });

    test_iop1<V_I8, iop_srli<int8_t, 5>>([](const V_I8& a) { return srli<5>(a); });
    test_iop1<V_I16, iop_srli<int16_t, 5>>([](const V_I16& a) { return srli<5>(a); });
    test_iop1<V_I32, iop_srli<int32_t, 5>>([](const V_I32& a) { return srli<5>(a); });
    test_iop1<V_I64, iop_srli<int64_t, 5>>([](const V_I64& a) { return srli<5>(a); });

    test_iop1<V_U8, iop_srli<uint8_t, 5>>([](const V_U8& a) { return srli<5>(a); });
    test_iop1<V_U16, iop_srli<uint16_t, 5>>([](const V_U16& a) { return srli<5>(a); });
    test_iop1<V_U32, iop_srli<uint32_t, 5>>([](const V_U32& a) { return srli<5>(a); });
    test_iop1<V_U64, iop_srli<uint64_t, 5>>([](const V_U64& a) { return srli<5>(a); });

    test_iop1<V_U8, iop_srli<uint8_t, 5>>([](const V_U8& a) { return srli_u8<5>(a); });
    test_iop1<V_U16, iop_srli<uint16_t, 5>>([](const V_U16& a) { return srli_u16<5>(a); });
    test_iop1<V_U32, iop_srli<uint32_t, 5>>([](const V_U32& a) { return srli_u32<5>(a); });
    test_iop1<V_U64, iop_srli<uint64_t, 5>>([](const V_U64& a) { return srli_u64<5>(a); });

    test_iop1<V_I8, iop_srli<int8_t, 7>>([](const V_I8& a) { return srli<7>(a); });
    test_iop1<V_I16, iop_srli<int16_t, 15>>([](const V_I16& a) { return srli<15>(a); });
    test_iop1<V_I32, iop_srli<int32_t, 31>>([](const V_I32& a) { return srli<31>(a); });
    test_iop1<V_I64, iop_srli<int64_t, 63>>([](const V_I64& a) { return srli<63>(a); });

    test_iop1<V_U8, iop_srli<uint8_t, 7>>([](const V_U8& a) { return srli<7>(a); });
    test_iop1<V_U16, iop_srli<uint16_t, 15>>([](const V_U16& a) { return srli<15>(a); });
    test_iop1<V_U32, iop_srli<uint32_t, 31>>([](const V_U32& a) { return srli<31>(a); });
    test_iop1<V_U64, iop_srli<uint64_t, 63>>([](const V_U64& a) { return srli<63>(a); });

    test_iop1<V_U8, iop_srli<uint8_t, 7>>([](const V_U8& a) { return srli_u8<7>(a); });
    test_iop1<V_U16, iop_srli<uint16_t, 15>>([](const V_U16& a) { return srli_u16<15>(a); });
    test_iop1<V_U32, iop_srli<uint32_t, 31>>([](const V_U32& a) { return srli_u32<31>(a); });
    test_iop1<V_U64, iop_srli<uint64_t, 63>>([](const V_U64& a) { return srli_u64<63>(a); });

    test_iop1<V_I8, iop_srai<int8_t, 1>>([](const V_I8& a) { return srai<1>(a); });
    test_iop1<V_I16, iop_srai<int16_t, 1>>([](const V_I16& a) { return srai<1>(a); });
    test_iop1<V_I32, iop_srai<int32_t, 1>>([](const V_I32& a) { return srai<1>(a); });
    test_iop1<V_I64, iop_srai<int64_t, 1>>([](const V_I64& a) { return srai<1>(a); });

    test_iop1<V_U8, iop_srai<uint8_t, 1>>([](const V_U8& a) { return srai<1>(a); });
    test_iop1<V_U16, iop_srai<uint16_t, 1>>([](const V_U16& a) { return srai<1>(a); });
    test_iop1<V_U32, iop_srai<uint32_t, 1>>([](const V_U32& a) { return srai<1>(a); });
    test_iop1<V_U64, iop_srai<uint64_t, 1>>([](const V_U64& a) { return srai<1>(a); });

    test_iop1<V_U8, iop_srai<uint8_t, 1>>([](const V_U8& a) { return srai_i8<1>(a); });
    test_iop1<V_U16, iop_srai<uint16_t, 1>>([](const V_U16& a) { return srai_i16<1>(a); });
    test_iop1<V_U32, iop_srai<uint32_t, 1>>([](const V_U32& a) { return srai_i32<1>(a); });
    test_iop1<V_U64, iop_srai<uint64_t, 1>>([](const V_U64& a) { return srai_i64<1>(a); });

    test_iop1<V_I8, iop_srai<int8_t, 5>>([](const V_I8& a) { return srai<5>(a); });
    test_iop1<V_I16, iop_srai<int16_t, 5>>([](const V_I16& a) { return srai<5>(a); });
    test_iop1<V_I32, iop_srai<int32_t, 5>>([](const V_I32& a) { return srai<5>(a); });
    test_iop1<V_I64, iop_srai<int64_t, 5>>([](const V_I64& a) { return srai<5>(a); });

    test_iop1<V_U8, iop_srai<uint8_t, 5>>([](const V_U8& a) { return srai<5>(a); });
    test_iop1<V_U16, iop_srai<uint16_t, 5>>([](const V_U16& a) { return srai<5>(a); });
    test_iop1<V_U32, iop_srai<uint32_t, 5>>([](const V_U32& a) { return srai<5>(a); });
    test_iop1<V_U64, iop_srai<uint64_t, 5>>([](const V_U64& a) { return srai<5>(a); });

    test_iop1<V_U8, iop_srai<uint8_t, 5>>([](const V_U8& a) { return srai_i8<5>(a); });
    test_iop1<V_U16, iop_srai<uint16_t, 5>>([](const V_U16& a) { return srai_i16<5>(a); });
    test_iop1<V_U32, iop_srai<uint32_t, 5>>([](const V_U32& a) { return srai_i32<5>(a); });
    test_iop1<V_U64, iop_srai<uint64_t, 5>>([](const V_U64& a) { return srai_i64<5>(a); });

    test_iop1<V_I8, iop_srai<int8_t, 7>>([](const V_I8& a) { return srai<7>(a); });
    test_iop1<V_I16, iop_srai<int16_t, 15>>([](const V_I16& a) { return srai<15>(a); });
    test_iop1<V_I32, iop_srai<int32_t, 31>>([](const V_I32& a) { return srai<31>(a); });
    test_iop1<V_I64, iop_srai<int64_t, 63>>([](const V_I64& a) { return srai<63>(a); });

    test_iop1<V_U8, iop_srai<uint8_t, 7>>([](const V_U8& a) { return srai<7>(a); });
    test_iop1<V_U16, iop_srai<uint16_t, 15>>([](const V_U16& a) { return srai<15>(a); });
    test_iop1<V_U32, iop_srai<uint32_t, 31>>([](const V_U32& a) { return srai<31>(a); });
    test_iop1<V_U64, iop_srai<uint64_t, 63>>([](const V_U64& a) { return srai<63>(a); });

    test_iop1<V_U8, iop_srai<uint8_t, 7>>([](const V_U8& a) { return srai_i8<7>(a); });
    test_iop1<V_U16, iop_srai<uint16_t, 15>>([](const V_U16& a) { return srai_i16<15>(a); });
    test_iop1<V_U32, iop_srai<uint32_t, 31>>([](const V_U32& a) { return srai_i32<31>(a); });
    test_iop1<V_U64, iop_srai<uint64_t, 63>>([](const V_U64& a) { return srai_i64<63>(a); });
  }

#if defined(BL_SIMD_FEATURE_RSRL)
  INFO("Testing %d-bit %s vector ops - bit shift (rounding)", kW*8, ext);
  {
    test_iop1<V_I8, iop_rsrli<int8_t, 1>>([](const V_I8& a) { return rsrli<1>(a); });
    test_iop1<V_I16, iop_rsrli<int16_t, 1>>([](const V_I16& a) { return rsrli<1>(a); });
    test_iop1<V_I32, iop_rsrli<int32_t, 1>>([](const V_I32& a) { return rsrli<1>(a); });
    test_iop1<V_I64, iop_rsrli<int64_t, 1>>([](const V_I64& a) { return rsrli<1>(a); });

    test_iop1<V_U8, iop_rsrli<uint8_t, 1>>([](const V_U8& a) { return rsrli<1>(a); });
    test_iop1<V_U16, iop_rsrli<uint16_t, 1>>([](const V_U16& a) { return rsrli<1>(a); });
    test_iop1<V_U32, iop_rsrli<uint32_t, 1>>([](const V_U32& a) { return rsrli<1>(a); });
    test_iop1<V_U64, iop_rsrli<uint64_t, 1>>([](const V_U64& a) { return rsrli<1>(a); });

    test_iop1<V_U8, iop_rsrli<uint8_t, 1>>([](const V_U8& a) { return rsrli_u8<1>(a); });
    test_iop1<V_U16, iop_rsrli<uint16_t, 1>>([](const V_U16& a) { return rsrli_u16<1>(a); });
    test_iop1<V_U32, iop_rsrli<uint32_t, 1>>([](const V_U32& a) { return rsrli_u32<1>(a); });
    test_iop1<V_U64, iop_rsrli<uint64_t, 1>>([](const V_U64& a) { return rsrli_u64<1>(a); });

    test_iop1<V_I8, iop_rsrli<int8_t, 5>>([](const V_I8& a) { return rsrli<5>(a); });
    test_iop1<V_I16, iop_rsrli<int16_t, 5>>([](const V_I16& a) { return rsrli<5>(a); });
    test_iop1<V_I32, iop_rsrli<int32_t, 5>>([](const V_I32& a) { return rsrli<5>(a); });
    test_iop1<V_I64, iop_rsrli<int64_t, 5>>([](const V_I64& a) { return rsrli<5>(a); });

    test_iop1<V_U8, iop_rsrli<uint8_t, 5>>([](const V_U8& a) { return rsrli<5>(a); });
    test_iop1<V_U16, iop_rsrli<uint16_t, 5>>([](const V_U16& a) { return rsrli<5>(a); });
    test_iop1<V_U32, iop_rsrli<uint32_t, 5>>([](const V_U32& a) { return rsrli<5>(a); });
    test_iop1<V_U64, iop_rsrli<uint64_t, 5>>([](const V_U64& a) { return rsrli<5>(a); });

    test_iop1<V_U8, iop_rsrli<uint8_t, 5>>([](const V_U8& a) { return rsrli_u8<5>(a); });
    test_iop1<V_U16, iop_rsrli<uint16_t, 5>>([](const V_U16& a) { return rsrli_u16<5>(a); });
    test_iop1<V_U32, iop_rsrli<uint32_t, 5>>([](const V_U32& a) { return rsrli_u32<5>(a); });
    test_iop1<V_U64, iop_rsrli<uint64_t, 5>>([](const V_U64& a) { return rsrli_u64<5>(a); });

    test_iop1<V_I8, iop_rsrli<int8_t, 7>>([](const V_I8& a) { return rsrli<7>(a); });
    test_iop1<V_I16, iop_rsrli<int16_t, 15>>([](const V_I16& a) { return rsrli<15>(a); });
    test_iop1<V_I32, iop_rsrli<int32_t, 31>>([](const V_I32& a) { return rsrli<31>(a); });
    test_iop1<V_I64, iop_rsrli<int64_t, 63>>([](const V_I64& a) { return rsrli<63>(a); });

    test_iop1<V_U8, iop_rsrli<uint8_t, 7>>([](const V_U8& a) { return rsrli<7>(a); });
    test_iop1<V_U16, iop_rsrli<uint16_t, 15>>([](const V_U16& a) { return rsrli<15>(a); });
    test_iop1<V_U32, iop_rsrli<uint32_t, 31>>([](const V_U32& a) { return rsrli<31>(a); });
    test_iop1<V_U64, iop_rsrli<uint64_t, 63>>([](const V_U64& a) { return rsrli<63>(a); });

    test_iop1<V_U8, iop_rsrli<uint8_t, 7>>([](const V_U8& a) { return rsrli_u8<7>(a); });
    test_iop1<V_U16, iop_rsrli<uint16_t, 15>>([](const V_U16& a) { return rsrli_u16<15>(a); });
    test_iop1<V_U32, iop_rsrli<uint32_t, 31>>([](const V_U32& a) { return rsrli_u32<31>(a); });
    test_iop1<V_U64, iop_rsrli<uint64_t, 63>>([](const V_U64& a) { return rsrli_u64<63>(a); });
  }
#endif

  INFO("Testing %d-bit %s vector ops - sllb_u128", kW*8, ext);
  {
    test_iop1<V_U8, iop_sllb_u128<uint8_t, 1>>([](const V_U8& a) { return sllb_u128<1>(a); });
    test_iop1<V_U8, iop_sllb_u128<uint8_t, 2>>([](const V_U8& a) { return sllb_u128<2>(a); });
    test_iop1<V_U8, iop_sllb_u128<uint8_t, 3>>([](const V_U8& a) { return sllb_u128<3>(a); });
    test_iop1<V_U8, iop_sllb_u128<uint8_t, 4>>([](const V_U8& a) { return sllb_u128<4>(a); });
    test_iop1<V_U8, iop_sllb_u128<uint8_t, 5>>([](const V_U8& a) { return sllb_u128<5>(a); });
    test_iop1<V_U8, iop_sllb_u128<uint8_t, 6>>([](const V_U8& a) { return sllb_u128<6>(a); });
    test_iop1<V_U8, iop_sllb_u128<uint8_t, 7>>([](const V_U8& a) { return sllb_u128<7>(a); });
    test_iop1<V_U8, iop_sllb_u128<uint8_t, 8>>([](const V_U8& a) { return sllb_u128<8>(a); });
    test_iop1<V_U8, iop_sllb_u128<uint8_t, 9>>([](const V_U8& a) { return sllb_u128<9>(a); });
    test_iop1<V_U8, iop_sllb_u128<uint8_t, 10>>([](const V_U8& a) { return sllb_u128<10>(a); });
    test_iop1<V_U8, iop_sllb_u128<uint8_t, 11>>([](const V_U8& a) { return sllb_u128<11>(a); });
    test_iop1<V_U8, iop_sllb_u128<uint8_t, 12>>([](const V_U8& a) { return sllb_u128<12>(a); });
    test_iop1<V_U8, iop_sllb_u128<uint8_t, 13>>([](const V_U8& a) { return sllb_u128<13>(a); });
    test_iop1<V_U8, iop_sllb_u128<uint8_t, 14>>([](const V_U8& a) { return sllb_u128<14>(a); });
    test_iop1<V_U8, iop_sllb_u128<uint8_t, 15>>([](const V_U8& a) { return sllb_u128<15>(a); });
  }

  INFO("Testing %d-bit %s vector ops - srlb_u128", kW*8, ext);
  {
    test_iop1<V_U8, iop_srlb_u128<uint8_t, 1>>([](const V_U8& a) { return srlb_u128<1>(a); });
    test_iop1<V_U8, iop_srlb_u128<uint8_t, 2>>([](const V_U8& a) { return srlb_u128<2>(a); });
    test_iop1<V_U8, iop_srlb_u128<uint8_t, 3>>([](const V_U8& a) { return srlb_u128<3>(a); });
    test_iop1<V_U8, iop_srlb_u128<uint8_t, 4>>([](const V_U8& a) { return srlb_u128<4>(a); });
    test_iop1<V_U8, iop_srlb_u128<uint8_t, 5>>([](const V_U8& a) { return srlb_u128<5>(a); });
    test_iop1<V_U8, iop_srlb_u128<uint8_t, 6>>([](const V_U8& a) { return srlb_u128<6>(a); });
    test_iop1<V_U8, iop_srlb_u128<uint8_t, 7>>([](const V_U8& a) { return srlb_u128<7>(a); });
    test_iop1<V_U8, iop_srlb_u128<uint8_t, 8>>([](const V_U8& a) { return srlb_u128<8>(a); });
    test_iop1<V_U8, iop_srlb_u128<uint8_t, 9>>([](const V_U8& a) { return srlb_u128<9>(a); });
    test_iop1<V_U8, iop_srlb_u128<uint8_t, 10>>([](const V_U8& a) { return srlb_u128<10>(a); });
    test_iop1<V_U8, iop_srlb_u128<uint8_t, 11>>([](const V_U8& a) { return srlb_u128<11>(a); });
    test_iop1<V_U8, iop_srlb_u128<uint8_t, 12>>([](const V_U8& a) { return srlb_u128<12>(a); });
    test_iop1<V_U8, iop_srlb_u128<uint8_t, 13>>([](const V_U8& a) { return srlb_u128<13>(a); });
    test_iop1<V_U8, iop_srlb_u128<uint8_t, 14>>([](const V_U8& a) { return srlb_u128<14>(a); });
    test_iop1<V_U8, iop_srlb_u128<uint8_t, 15>>([](const V_U8& a) { return srlb_u128<15>(a); });
  }

  INFO("Testing %d-bit %s vector ops - alignr_u128", kW*8, ext);
  {
    test_iop2<V_U8, iop_alignr_u128<uint8_t, 1>>([](const V_U8& a, const V_U8& b) { return alignr_u128<1>(a, b); });
    test_iop2<V_U8, iop_alignr_u128<uint8_t, 2>>([](const V_U8& a, const V_U8& b) { return alignr_u128<2>(a, b); });
    test_iop2<V_U8, iop_alignr_u128<uint8_t, 3>>([](const V_U8& a, const V_U8& b) { return alignr_u128<3>(a, b); });
    test_iop2<V_U8, iop_alignr_u128<uint8_t, 4>>([](const V_U8& a, const V_U8& b) { return alignr_u128<4>(a, b); });
    test_iop2<V_U8, iop_alignr_u128<uint8_t, 5>>([](const V_U8& a, const V_U8& b) { return alignr_u128<5>(a, b); });
    test_iop2<V_U8, iop_alignr_u128<uint8_t, 6>>([](const V_U8& a, const V_U8& b) { return alignr_u128<6>(a, b); });
    test_iop2<V_U8, iop_alignr_u128<uint8_t, 7>>([](const V_U8& a, const V_U8& b) { return alignr_u128<7>(a, b); });
    test_iop2<V_U8, iop_alignr_u128<uint8_t, 8>>([](const V_U8& a, const V_U8& b) { return alignr_u128<8>(a, b); });
    test_iop2<V_U8, iop_alignr_u128<uint8_t, 9>>([](const V_U8& a, const V_U8& b) { return alignr_u128<9>(a, b); });
    test_iop2<V_U8, iop_alignr_u128<uint8_t, 10>>([](const V_U8& a, const V_U8& b) { return alignr_u128<10>(a, b); });
    test_iop2<V_U8, iop_alignr_u128<uint8_t, 11>>([](const V_U8& a, const V_U8& b) { return alignr_u128<11>(a, b); });
    test_iop2<V_U8, iop_alignr_u128<uint8_t, 12>>([](const V_U8& a, const V_U8& b) { return alignr_u128<12>(a, b); });
    test_iop2<V_U8, iop_alignr_u128<uint8_t, 13>>([](const V_U8& a, const V_U8& b) { return alignr_u128<13>(a, b); });
    test_iop2<V_U8, iop_alignr_u128<uint8_t, 14>>([](const V_U8& a, const V_U8& b) { return alignr_u128<14>(a, b); });
    test_iop2<V_U8, iop_alignr_u128<uint8_t, 15>>([](const V_U8& a, const V_U8& b) { return alignr_u128<15>(a, b); });
  }

  INFO("Testing %d-bit %s vector ops - broadcast", kW*8, ext);
  {
    test_iop1<V_U8, iop_broadcast_u8<uint8_t>>([](const V_U8& a) { return broadcast_u8<V_U8>(a); });
    test_iop1<V_U8, iop_broadcast_u16<uint8_t>>([](const V_U8& a) { return broadcast_u16<V_U8>(a); });
    test_iop1<V_U8, iop_broadcast_u32<uint8_t>>([](const V_U8& a) { return broadcast_u32<V_U8>(a); });
    test_iop1<V_U8, iop_broadcast_u64<uint8_t>>([](const V_U8& a) { return broadcast_u64<V_U8>(a); });
  }

  INFO("Testing %d-bit %s vector ops - swizzle_[lo|hi]_u16", kW*8, ext);
  {
    test_iop1<V_U8, iop_swizzle_lo_u16<uint8_t, 0, 0, 0, 0>>([](const V_U8& a) { return swizzle_lo_u16<0, 0, 0, 0>(a); });
    test_iop1<V_U8, iop_swizzle_lo_u16<uint8_t, 1, 1, 1, 1>>([](const V_U8& a) { return swizzle_lo_u16<1, 1, 1, 1>(a); });
    test_iop1<V_U8, iop_swizzle_lo_u16<uint8_t, 2, 2, 2, 2>>([](const V_U8& a) { return swizzle_lo_u16<2, 2, 2, 2>(a); });
    test_iop1<V_U8, iop_swizzle_lo_u16<uint8_t, 3, 3, 3, 3>>([](const V_U8& a) { return swizzle_lo_u16<3, 3, 3, 3>(a); });
    test_iop1<V_U8, iop_swizzle_lo_u16<uint8_t, 0, 1, 2, 3>>([](const V_U8& a) { return swizzle_lo_u16<0, 1, 2, 3>(a); });
    test_iop1<V_U8, iop_swizzle_lo_u16<uint8_t, 3, 2, 1, 0>>([](const V_U8& a) { return swizzle_lo_u16<3, 2, 1, 0>(a); });
    test_iop1<V_U8, iop_swizzle_lo_u16<uint8_t, 2, 3, 0, 1>>([](const V_U8& a) { return swizzle_lo_u16<2, 3, 0, 1>(a); });
    test_iop1<V_U8, iop_swizzle_lo_u16<uint8_t, 3, 1, 2, 0>>([](const V_U8& a) { return swizzle_lo_u16<3, 1, 2, 0>(a); });
    test_iop1<V_U8, iop_swizzle_lo_u16<uint8_t, 1, 3, 0, 2>>([](const V_U8& a) { return swizzle_lo_u16<1, 3, 0, 2>(a); });
    test_iop1<V_U8, iop_swizzle_lo_u16<uint8_t, 0, 0, 2, 2>>([](const V_U8& a) { return swizzle_lo_u16<0, 0, 2, 2>(a); });
    test_iop1<V_U8, iop_swizzle_lo_u16<uint8_t, 2, 2, 0, 0>>([](const V_U8& a) { return swizzle_lo_u16<2, 2, 0, 0>(a); });
    test_iop1<V_U8, iop_swizzle_lo_u16<uint8_t, 3, 3, 3, 1>>([](const V_U8& a) { return swizzle_lo_u16<3, 3, 3, 1>(a); });
    test_iop1<V_U8, iop_swizzle_lo_u16<uint8_t, 1, 3, 3, 1>>([](const V_U8& a) { return swizzle_lo_u16<1, 3, 3, 1>(a); });

    test_iop1<V_U8, iop_swizzle_hi_u16<uint8_t, 0, 0, 0, 0>>([](const V_U8& a) { return swizzle_hi_u16<0, 0, 0, 0>(a); });
    test_iop1<V_U8, iop_swizzle_hi_u16<uint8_t, 1, 1, 1, 1>>([](const V_U8& a) { return swizzle_hi_u16<1, 1, 1, 1>(a); });
    test_iop1<V_U8, iop_swizzle_hi_u16<uint8_t, 2, 2, 2, 2>>([](const V_U8& a) { return swizzle_hi_u16<2, 2, 2, 2>(a); });
    test_iop1<V_U8, iop_swizzle_hi_u16<uint8_t, 3, 3, 3, 3>>([](const V_U8& a) { return swizzle_hi_u16<3, 3, 3, 3>(a); });
    test_iop1<V_U8, iop_swizzle_hi_u16<uint8_t, 0, 1, 2, 3>>([](const V_U8& a) { return swizzle_hi_u16<0, 1, 2, 3>(a); });
    test_iop1<V_U8, iop_swizzle_hi_u16<uint8_t, 3, 2, 1, 0>>([](const V_U8& a) { return swizzle_hi_u16<3, 2, 1, 0>(a); });
    test_iop1<V_U8, iop_swizzle_hi_u16<uint8_t, 2, 3, 0, 1>>([](const V_U8& a) { return swizzle_hi_u16<2, 3, 0, 1>(a); });
    test_iop1<V_U8, iop_swizzle_hi_u16<uint8_t, 3, 1, 2, 0>>([](const V_U8& a) { return swizzle_hi_u16<3, 1, 2, 0>(a); });
    test_iop1<V_U8, iop_swizzle_hi_u16<uint8_t, 1, 3, 0, 2>>([](const V_U8& a) { return swizzle_hi_u16<1, 3, 0, 2>(a); });
    test_iop1<V_U8, iop_swizzle_hi_u16<uint8_t, 0, 0, 2, 2>>([](const V_U8& a) { return swizzle_hi_u16<0, 0, 2, 2>(a); });
    test_iop1<V_U8, iop_swizzle_hi_u16<uint8_t, 2, 2, 0, 0>>([](const V_U8& a) { return swizzle_hi_u16<2, 2, 0, 0>(a); });
    test_iop1<V_U8, iop_swizzle_hi_u16<uint8_t, 3, 3, 3, 1>>([](const V_U8& a) { return swizzle_hi_u16<3, 3, 3, 1>(a); });
    test_iop1<V_U8, iop_swizzle_hi_u16<uint8_t, 1, 3, 3, 1>>([](const V_U8& a) { return swizzle_hi_u16<1, 3, 3, 1>(a); });

    test_iop1<V_U8, iop_swizzle_u16<uint8_t, 0, 0, 0, 0>>([](const V_U8& a) { return swizzle_u16<0, 0, 0, 0>(a); });
    test_iop1<V_U8, iop_swizzle_u16<uint8_t, 1, 1, 1, 1>>([](const V_U8& a) { return swizzle_u16<1, 1, 1, 1>(a); });
    test_iop1<V_U8, iop_swizzle_u16<uint8_t, 2, 2, 2, 2>>([](const V_U8& a) { return swizzle_u16<2, 2, 2, 2>(a); });
    test_iop1<V_U8, iop_swizzle_u16<uint8_t, 3, 3, 3, 3>>([](const V_U8& a) { return swizzle_u16<3, 3, 3, 3>(a); });
    test_iop1<V_U8, iop_swizzle_u16<uint8_t, 0, 1, 2, 3>>([](const V_U8& a) { return swizzle_u16<0, 1, 2, 3>(a); });
    test_iop1<V_U8, iop_swizzle_u16<uint8_t, 3, 2, 1, 0>>([](const V_U8& a) { return swizzle_u16<3, 2, 1, 0>(a); });
    test_iop1<V_U8, iop_swizzle_u16<uint8_t, 2, 3, 0, 1>>([](const V_U8& a) { return swizzle_u16<2, 3, 0, 1>(a); });
    test_iop1<V_U8, iop_swizzle_u16<uint8_t, 3, 1, 2, 0>>([](const V_U8& a) { return swizzle_u16<3, 1, 2, 0>(a); });
    test_iop1<V_U8, iop_swizzle_u16<uint8_t, 1, 3, 0, 2>>([](const V_U8& a) { return swizzle_u16<1, 3, 0, 2>(a); });
    test_iop1<V_U8, iop_swizzle_u16<uint8_t, 0, 0, 2, 2>>([](const V_U8& a) { return swizzle_u16<0, 0, 2, 2>(a); });
    test_iop1<V_U8, iop_swizzle_u16<uint8_t, 2, 2, 0, 0>>([](const V_U8& a) { return swizzle_u16<2, 2, 0, 0>(a); });
    test_iop1<V_U8, iop_swizzle_u16<uint8_t, 3, 3, 3, 1>>([](const V_U8& a) { return swizzle_u16<3, 3, 3, 1>(a); });
    test_iop1<V_U8, iop_swizzle_u16<uint8_t, 1, 3, 3, 1>>([](const V_U8& a) { return swizzle_u16<1, 3, 3, 1>(a); });
  }

  INFO("Testing %d-bit %s vector ops - swizzle_u32", kW*8, ext);
  {
    test_iop1<V_U8, iop_swizzle_u32<uint8_t, 0, 0, 0, 0>>([](const V_U8& a) { return swizzle_u32<0, 0, 0, 0>(a); });
    test_iop1<V_U8, iop_swizzle_u32<uint8_t, 1, 1, 1, 1>>([](const V_U8& a) { return swizzle_u32<1, 1, 1, 1>(a); });
    test_iop1<V_U8, iop_swizzle_u32<uint8_t, 2, 2, 2, 2>>([](const V_U8& a) { return swizzle_u32<2, 2, 2, 2>(a); });
    test_iop1<V_U8, iop_swizzle_u32<uint8_t, 3, 3, 3, 3>>([](const V_U8& a) { return swizzle_u32<3, 3, 3, 3>(a); });
    test_iop1<V_U8, iop_swizzle_u32<uint8_t, 0, 1, 2, 3>>([](const V_U8& a) { return swizzle_u32<0, 1, 2, 3>(a); });
    test_iop1<V_U8, iop_swizzle_u32<uint8_t, 3, 2, 1, 0>>([](const V_U8& a) { return swizzle_u32<3, 2, 1, 0>(a); });
    test_iop1<V_U8, iop_swizzle_u32<uint8_t, 2, 3, 0, 1>>([](const V_U8& a) { return swizzle_u32<2, 3, 0, 1>(a); });
    test_iop1<V_U8, iop_swizzle_u32<uint8_t, 3, 1, 2, 0>>([](const V_U8& a) { return swizzle_u32<3, 1, 2, 0>(a); });
    test_iop1<V_U8, iop_swizzle_u32<uint8_t, 1, 3, 0, 2>>([](const V_U8& a) { return swizzle_u32<1, 3, 0, 2>(a); });
    test_iop1<V_U8, iop_swizzle_u32<uint8_t, 0, 0, 2, 2>>([](const V_U8& a) { return swizzle_u32<0, 0, 2, 2>(a); });
    test_iop1<V_U8, iop_swizzle_u32<uint8_t, 2, 2, 0, 0>>([](const V_U8& a) { return swizzle_u32<2, 2, 0, 0>(a); });
    test_iop1<V_U8, iop_swizzle_u32<uint8_t, 3, 3, 3, 1>>([](const V_U8& a) { return swizzle_u32<3, 3, 3, 1>(a); });
    test_iop1<V_U8, iop_swizzle_u32<uint8_t, 1, 3, 3, 1>>([](const V_U8& a) { return swizzle_u32<1, 3, 3, 1>(a); });
  }

  INFO("Testing %d-bit %s vector ops - swizzle_u64", kW*8, ext);
  {
    test_iop1<V_U8, iop_swizzle_u64<uint8_t, 0, 0>>([](const V_U8& a) { return swizzle_u64<0, 0>(a); });
    test_iop1<V_U8, iop_swizzle_u64<uint8_t, 0, 1>>([](const V_U8& a) { return swizzle_u64<0, 1>(a); });
    test_iop1<V_U8, iop_swizzle_u64<uint8_t, 1, 0>>([](const V_U8& a) { return swizzle_u64<1, 0>(a); });
    test_iop1<V_U8, iop_swizzle_u64<uint8_t, 1, 1>>([](const V_U8& a) { return swizzle_u64<1, 1>(a); });
  }

#if defined(BL_SIMD_FEATURE_SWIZZLEV_U8)
  INFO("Testing %d-bit %s vector ops - swizzlev_u8", kW*8, ext);
  {
    test_iop2<V_U8, iop_swizzlev_u8<uint8_t>>([](const V_U8& a, const V_U8& b) { return swizzlev_u8(a, b); });
  }
#endif // BL_SIMD_FEATURE_SWIZZLEV_U8

  INFO("Testing %d-bit %s vector ops - dup_[lo|hi]", kW*8, ext);
  {
    test_iop1<V_U8, iop_swizzle_u32<uint8_t, 2, 2, 0, 0>>([](const V_U8& a) { return dup_lo_u32(a); });
    test_iop1<V_U8, iop_swizzle_u32<uint8_t, 3, 3, 1, 1>>([](const V_U8& a) { return dup_hi_u32(a); });
    test_iop1<V_U8, iop_swizzle_u64<uint8_t, 0, 0>>([](const V_U8& a) { return dup_lo_u64(a); });
    test_iop1<V_U8, iop_swizzle_u64<uint8_t, 1, 1>>([](const V_U8& a) { return dup_hi_u64(a); });
  }

  INFO("Testing %d-bit %s vector ops - swap", kW*8, ext);
  {
    test_iop1<V_U8, iop_swizzle_u32<uint8_t, 2, 3, 0, 1>>([](const V_U8& a) { return swap_u32(a); });
    test_iop1<V_U8, iop_swizzle_u64<uint8_t, 0, 1>>([](const V_U8& a) { return swap_u64(a); });
  }

  INFO("Testing %d-bit %s vector ops - interleave", kW*8, ext);
  {
    test_iop2<V_U8, iop_interleave_lo_u8<uint8_t>>([](const V_U8& a, const V_U8& b) { return interleave_lo_u8(a, b); });
    test_iop2<V_U8, iop_interleave_hi_u8<uint8_t>>([](const V_U8& a, const V_U8& b) { return interleave_hi_u8(a, b); });
    test_iop2<V_U8, iop_interleave_lo_u16<uint8_t>>([](const V_U8& a, const V_U8& b) { return interleave_lo_u16(a, b); });
    test_iop2<V_U8, iop_interleave_hi_u16<uint8_t>>([](const V_U8& a, const V_U8& b) { return interleave_hi_u16(a, b); });
    test_iop2<V_U8, iop_interleave_lo_u32<uint8_t>>([](const V_U8& a, const V_U8& b) { return interleave_lo_u32(a, b); });
    test_iop2<V_U8, iop_interleave_hi_u32<uint8_t>>([](const V_U8& a, const V_U8& b) { return interleave_hi_u32(a, b); });
    test_iop2<V_U8, iop_interleave_lo_u64<uint8_t>>([](const V_U8& a, const V_U8& b) { return interleave_lo_u64(a, b); });
    test_iop2<V_U8, iop_interleave_hi_u64<uint8_t>>([](const V_U8& a, const V_U8& b) { return interleave_hi_u64(a, b); });
  }

  INFO("Testing %d-bit %s vector ops - utilities - div255_u16", kW*8, ext);
  {
    test_iop1_constraint<V_U16,
      iop_div255_u16, ConstraintRangeU16<0, 255u*255u>>([](const V_U16& a) { return div255_u16(a); });
  }

  INFO("Testing %d-bit %s vector ops - utilities - div65535_u32", kW*8, ext);
  {
    test_iop1_constraint<V_U32,
      iop_div65535_u32, ConstraintRangeU32<0, 65535u*65535u>>([](const V_U32& a) { return div65535_u32(a); });
  }

#if defined(BL_SIMD_FEATURE_ARRAY_LOOKUP)
  INFO("Testing %d-bit %s vector ops - utilities - array_lookup_u32", kW*8, ext);
  {
    alignas(16) uint32_t arr[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

    {
      auto mi = array_lookup_u32_eq_aligned16<4>(arr, 255);
      EXPECT_FALSE(mi.matched());

      for (uint32_t i = 0; i < 4; i++) {
        auto m = array_lookup_u32_eq_aligned16<4>(arr, i + 1);
        EXPECT_TRUE(m.matched());
        EXPECT_EQ(m.index(), i);
      }
    }

    {
      auto mi = array_lookup_u32_eq_aligned16<8>(arr, 255);
      EXPECT_FALSE(mi.matched());

      for (uint32_t i = 0; i < 8; i++) {
        auto m = array_lookup_u32_eq_aligned16<8>(arr, i + 1);
        EXPECT_TRUE(m.matched());
        EXPECT_EQ(m.index(), i);
      }
    }

    {
      auto mi = array_lookup_u32_eq_aligned16<16>(arr, 255);
      EXPECT_FALSE(mi.matched());

      for (uint32_t i = 0; i < 16; i++) {
        auto m = array_lookup_u32_eq_aligned16<16>(arr, i + 1);
        EXPECT_TRUE(m.matched());
        EXPECT_EQ(m.index(), i);
      }
    }
  }
#endif // BL_SIMD_FEATURE_ARRAY_LOOKUP
}

} // {anonymous}
} // {SIMDTests}

//! \endcond

#endif // BLEND2D_SIMD_SIMD_TEST_P_H_INCLUDED
