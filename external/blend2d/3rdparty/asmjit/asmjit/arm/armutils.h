// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_ARM_ARMUTILS_H_INCLUDED
#define ASMJIT_ARM_ARMUTILS_H_INCLUDED

#include <asmjit/support/support.h>
#include <asmjit/arm/armglobals.h>

ASMJIT_BEGIN_SUB_NAMESPACE(arm)

//! \addtogroup asmjit_arm
//! \{

//! Public utilities and helpers for targeting AArch32 and AArch64 architectures.
namespace Utils {

//! Encodes a 12-bit immediate part of opcode that ise used by a standard 32-bit ARM encoding.
[[maybe_unused]]
[[nodiscard]]
static ASMJIT_INLINE bool encode_aarch32_imm(uint64_t imm, Out<uint32_t> imm_out) noexcept {
  if (imm & 0xFFFFFFFF00000000u)
    return false;

  uint32_t v = uint32_t(imm);
  uint32_t r = 0;

  if (v <= 0xFFu) {
    imm_out = v;
    return true;
  }

  // Rotate if there are bits on both ends (LSB and MSB)
  // (otherwise we would not be able to calculate the rotation with ctz).
  if (v & 0xFF0000FFu) {
    v = Support::ror(v, 16);
    r = 16u;
  }

  uint32_t n = Support::ctz(v) & ~0x1u;
  r = (r - n) & 0x1Eu;
  v = Support::ror(v, n);

  if (v > 0xFFu)
    return false;

  imm_out = v | (r << 7);
  return true;
}

//! Decomposed fields of a logical immediate value.
struct LogicalImm {
  uint32_t n;
  uint32_t s;
  uint32_t r;
};

//! Encodes the given `imm` value of the given `width` to a logical immediate value represented as N, S, and R fields
//! and writes these fields to `out`.
//!
//! Encoding Table:
//!
//! ```
//! +---+--------+--------+------+
//! | N |  ImmS  |  ImmR  | Size |
//! +---+--------+--------+------+
//! | 1 | ssssss | rrrrrr |  64  |
//! | 0 | 0sssss | .rrrrr |  32  |
//! | 0 | 10ssss | ..rrrr |  16  |
//! | 0 | 110sss | ...rrr |  8   |
//! | 0 | 1110ss | ....rr |  4   |
//! | 0 | 11110s | .....r |  2   |
//! +---+--------+--------+------+
//! ```
[[maybe_unused]]
[[nodiscard]]
static ASMJIT_INLINE bool encode_logical_imm(uint64_t imm, uint32_t width, Out<LogicalImm> out) noexcept {
  // Determine the element width, which must be 2, 4, 8, 16, 32, or 64 bits.
  do {
    width /= 2u;
    uint64_t mask = (uint64_t(1) << width) - 1u;
    if ((imm & mask) != ((imm >> width) & mask)) {
      width *= 2u;
      break;
    }
  } while (width > 2u);

  // Patterns of all zeros and all ones are not encodable.
  uint64_t lsb_mask = Support::lsb_mask<uint64_t>(width);
  imm &= lsb_mask;

  if (imm == 0 || imm == lsb_mask) {
    return false;
  }

  // Inspect the pattern and get the most important bit indexes.
  //
  //         o_index <-+      +-> z_index
  //                  |      |
  // |..zeros..|o_count|z_count|..ones..|
  // |000000000|111111|000000|11111111|

  uint32_t z_index = Support::ctz(~imm);
  uint64_t z_imm = imm ^ ((uint64_t(1) << z_index) - 1);
  uint32_t z_count = (z_imm ? Support::ctz(z_imm) : width) - z_index;

  uint32_t o_index = z_index + z_count;
  uint64_t o_imm = ~(z_imm ^ Support::lsb_mask<uint64_t>(o_index));
  uint32_t o_count = (o_imm ? Support::ctz(o_imm) : width) - (o_index);

  // Verify whether the bit-pattern is encodable.
  uint64_t must_be_zero = o_imm ^ ~Support::lsb_mask<uint64_t>(o_index + o_count);
  if (must_be_zero != 0 || (z_index > 0 && width - (o_index + o_count) != 0u)) {
    return false;
  }

  out->n = width == 64;
  out->s = (o_count + z_index - 1) | (Support::neg(width * 2u) & 0x3Fu);
  out->r = width - o_index;
  return true;
}

//! Returns true if the given `imm` value is encodable as a logical immediate. The `width` argument describes the
//! width of the operation, and must be either 32 or 64. This function can be used to test whether an immediate
//! value can be used with AND, ANDS, BIC, BICS, EON, EOR, ORN, and ORR instruction.
[[maybe_unused]]
[[nodiscard]]
static ASMJIT_INLINE bool is_logical_imm(uint64_t imm, uint32_t width) noexcept {
  LogicalImm dummy;
  return encode_logical_imm(imm, width, Out(dummy));
}

//! Returns true if the given `imm` value is encodable as an immediate with `add` and `sub` instructions on AArch64.
//! These two instructions can encode 12-bit immediate value optionally shifted left by 12 bits.
[[maybe_unused]]
[[nodiscard]]
static ASMJIT_INLINE bool is_add_sub_imm(uint64_t imm) noexcept {
  return imm <= 0xFFFu || (imm & ~uint64_t(0xFFFu << 12)) == 0;
}

//! Returns true if the given `imm` value is a byte mask. Byte mask has each byte part of the value set to either
//! 0x00 or 0xFF. Some ARM instructions accept immediate values that form a byte-mask and this function can be used
//! to verify that the immediate is encodable before using the value.
template<typename T>
[[nodiscard]]
static ASMJIT_INLINE bool is_byte_mask_imm(const T& imm) noexcept {
  constexpr T kMask = T(0x0101010101010101 & Support::bit_ones<T>);
  return imm == (imm & kMask) * T(255);
}

// [.......A|B.......|.......C|D.......|.......E|F.......|.......G|H.......]
[[maybe_unused]]
[[nodiscard]]
static ASMJIT_INLINE uint32_t encode_imm64_byte_mask_to_imm8(uint64_t imm) noexcept {
  return uint32_t(((imm >> (7  - 0)) & 0b00000011) | // [.......G|H.......]
                  ((imm >> (23 - 2)) & 0b00001100) | // [.......E|F.......]
                  ((imm >> (39 - 4)) & 0b00110000) | // [.......C|D.......]
                  ((imm >> (55 - 6)) & 0b11000000)); // [.......A|B.......]
}

//! \cond
//! A generic implementation that checks whether a floating point value can be converted to ARM Imm8.
template<typename T, uint32_t kNumBBits, uint32_t kNumCDEFGHBits, uint32_t kNumZeroBits>
[[nodiscard]]
static ASMJIT_INLINE bool is_fp_imm8_generic(T val) noexcept {
  constexpr uint32_t kAllBsMask = Support::lsb_mask_const<uint32_t>(kNumBBits);
  constexpr uint32_t kB0Pattern = Support::bit_mask<uint32_t>(kNumBBits - 1);
  constexpr uint32_t kB1Pattern = kAllBsMask ^ kB0Pattern;

  T imm_z = val & Support::lsb_mask<T>(kNumZeroBits);
  uint32_t imm_b = uint32_t(val >> (kNumZeroBits + kNumCDEFGHBits)) & kAllBsMask;

  // ImmZ must be all zeros and ImmB must either be B0 or B1 pattern.
  return imm_z == 0 && (imm_b == kB0Pattern || imm_b == kB1Pattern);
}
//! \endcond

//! Returns true if the given half precision floating point `val` can be encoded as ARM IMM8 value, which represents
//! a limited set of floating point immediate values, which can be used with FMOV instruction.
//!
//! The floating point must have bits distributed in the following way:
//!
//! ```
//! [aBbbcdef|gh000000]
//! ```
[[nodiscard]]
static ASMJIT_INLINE bool is_fp16_imm8(uint32_t val) noexcept { return is_fp_imm8_generic<uint32_t, 3, 6, 6>(val); }

//! Returns true if the given single precision floating point `val` can be encoded as ARM IMM8 value, which represents
//! a limited set of floating point immediate values, which can be used with FMOV instruction.
//!
//! The floating point must have bits distributed in the following way:
//!
//! ```
//! [aBbbbbbc|defgh000|00000000|00000000]
//! ```
[[nodiscard]]
static ASMJIT_INLINE bool is_fp32_imm8(uint32_t val) noexcept { return is_fp_imm8_generic<uint32_t, 6, 6, 19>(val); }

//! \overload
[[nodiscard]]
static ASMJIT_INLINE bool is_fp32_imm8(float val) noexcept { return is_fp32_imm8(Support::bit_cast<uint32_t>(val)); }

//! Returns true if the given double precision floating point `val` can be encoded as ARM IMM8 value, which represents
//! a limited set of floating point immediate values, which can be used with FMOV instruction.
//!
//! The floating point must have bits distributed in the following way:
//!
//! ```
//! [aBbbbbbb|bbcdefgh|00000000|00000000|00000000|00000000|00000000|00000000]
//! ```
[[nodiscard]]
static ASMJIT_INLINE bool is_fp64_imm8(uint64_t val) noexcept { return is_fp_imm8_generic<uint64_t, 9, 6, 48>(val); }

//! \overload
[[nodiscard]]
static ASMJIT_INLINE bool is_fp64_imm8(double val) noexcept { return is_fp64_imm8(Support::bit_cast<uint64_t>(val)); }

//! \cond
template<typename T, uint32_t kNumBBits, uint32_t kNumCDEFGHBits, uint32_t kNumZeroBits>
static ASMJIT_INLINE uint32_t encode_fp_to_imm8_generic(T val) noexcept {
  uint32_t bits = uint32_t(val >> kNumZeroBits);
  return ((bits >> (kNumBBits + kNumCDEFGHBits - 7)) & 0x80u) | (bits & 0x7F);
}
//! \endcond

//! Encodes a double precision floating point value into IMM8 format.
//!
//! \note This function expects that `is_fp64_imm8(val) == true` so it doesn't perform any checks of the value and just
//! rearranges some bits into Imm8 order.
[[nodiscard]]
static ASMJIT_INLINE uint32_t encode_fp64_to_imm8(uint64_t val) noexcept { return encode_fp_to_imm8_generic<uint64_t, 9, 6, 48>(val); }

//! \overload
[[nodiscard]]
static ASMJIT_INLINE uint32_t encode_fp64_to_imm8(double val) noexcept { return encode_fp64_to_imm8(Support::bit_cast<uint64_t>(val)); }

} // {Utils}

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_ARM_ARMUTILS_H_INCLUDED

