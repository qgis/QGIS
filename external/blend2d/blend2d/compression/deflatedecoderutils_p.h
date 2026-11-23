// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_COMPRESSION_DEFLATEDECODERUTILS_P_H_INCLUDED
#define BLEND2D_COMPRESSION_DEFLATEDECODERUTILS_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/compression/deflatedecoder_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/memops_p.h>

#if defined(BL_TARGET_OPT_BMI2)
  #if defined(_MSC_VER)
    #include <intrin.h>
  #else
    #include <x86intrin.h>
  #endif
#endif

#if defined(BL_TARGET_OPT_SSSE3) || (BL_TARGET_ARCH_ARM == 64)
  #include <blend2d/simd/simd_p.h>
  #define BL_DECODER_USE_SWIZZLEV
#endif

//! \cond INTERNAL

namespace bl::Compression::Deflate::DecoderUtils {
namespace {

#if defined(BL_TARGET_OPT_BMI2)
template<typename T>
BL_INLINE_NODEBUG uint32_t mask32(const T& n) noexcept { return uint32_t(_bzhi_u32(uint32_t(0xFFFFFFFFu), uint32_t(n))); }

BL_INLINE_NODEBUG uint32_t extract_n(BLBitWord src, size_t n) noexcept { return uint32_t(_bzhi_u32(uint32_t(src), uint32_t(n))); }

BL_INLINE_NODEBUG uint32_t extract_entry(BLBitWord src, DecodeEntry entry) noexcept { return uint32_t(_bzhi_u32(uint32_t(src), entry.value)); }
#else
template<typename T>
BL_INLINE_NODEBUG uint32_t mask32(const T& n) noexcept { return (1u << n) - 1u; }

BL_INLINE_NODEBUG uint32_t extract_n(BLBitWord src, size_t n) noexcept { return (uint32_t(src) & mask32(uint32_t(n))); }

BL_INLINE_NODEBUG uint32_t extract_entry(BLBitWord src, DecodeEntry entry) noexcept { return uint32_t(src) & ((1u << (entry.value & uint32_t(IntOps::bit_size_of<BLBitWord>() - 1))) - 1u); }
#endif

BL_INLINE_NODEBUG bool is_literal(DecodeEntry e) noexcept { return (e.value & DecodeEntry::kLiteralFlag) != 0u; }
BL_INLINE_NODEBUG bool is_off_or_len(DecodeEntry e) noexcept { return (e.value & DecodeEntry::kOffOrLenFlag) != 0u; }
BL_INLINE_NODEBUG bool is_off_and_len(DecodeEntry e) noexcept { return (e.value & DecodeEntry::kOffAndLenFlag) != 0u; }
BL_INLINE_NODEBUG bool is_end_of_block(DecodeEntry e) noexcept { return (e.value & (DecodeEntry::kEndOfBlockFlag)) != 0u; }
BL_INLINE_NODEBUG bool is_end_of_block_invalid(DecodeEntry e) noexcept { return (e.value & DecodeEntry::kEndOfBlockInvalidFlag) != 0u; }

// Extracts a field from DecodeEntry.
template<uint32_t kOffset, uint32_t kNBits>
BL_INLINE_NODEBUG uint32_t extract_field(DecodeEntry e) noexcept { return (e.value >> kOffset) & mask32(kNBits); }

BL_INLINE_NODEBUG uint32_t base_length(DecodeEntry e) noexcept { return extract_field<DecodeEntry::kBaseLengthOffset, DecodeEntry::kBaseLengthNBits>(e); }
BL_INLINE_NODEBUG uint32_t full_length(DecodeEntry e) noexcept { return extract_field<DecodeEntry::kFullLengthOffset, DecodeEntry::kFullLengthNBits>(e); }

BL_INLINE_NODEBUG uint32_t raw_payload(DecodeEntry e) noexcept { return (e.value >> DecodeEntry::kPayloadOffset) & 0xFFFFu; }
BL_INLINE_NODEBUG uint32_t payload_field(DecodeEntry e) noexcept { return extract_field<DecodeEntry::kPayloadOffset, DecodeEntry::kPayloadNBits>(e); }

BL_INLINE_NODEBUG uint32_t precode_value(DecodeEntry e) noexcept { return extract_field<DecodeEntry::kPrecodeValueOffset, DecodeEntry::kPrecodeValueNBits>(e); }
BL_INLINE_NODEBUG uint32_t precode_repeat(DecodeEntry e) noexcept { return extract_field<DecodeEntry::kPrecodeRepeatOffset, DecodeEntry::kPrecodeRepeatNBits>(e); }

BL_INLINE_NODEBUG uint32_t extract_extra(BLBitWord src, DecodeEntry e) noexcept { return extract_entry(src, e) >> base_length(e); }

} // {anonymous}
} // {bl::Compression::Deflate::DecoderUtils}

// bl::Compression::Deflate - Decoder Bits
// =======================================

namespace bl::Compression::Deflate {
namespace {

struct DecoderTableMask {
  uint32_t _mask;

  BL_INLINE_NODEBUG explicit DecoderTableMask(uint32_t bitlen) noexcept
    : _mask(DecoderUtils::mask32(bitlen)) {}

  BL_INLINE_NODEBUG uint32_t extract_index(BLBitWord bits) const noexcept {
    return uint32_t(bits & _mask);
  }
};

struct DecoderBits {
  BLBitWord bit_word {};
  size_t bit_length {};

  BL_INLINE_NODEBUG void reset() noexcept {
    bit_word = 0;
    bit_length = 0;
  }

  BL_INLINE_NODEBUG void load_state(Decoder* ctx) noexcept {
    bit_word = ctx->_bit_word;
    bit_length = ctx->_bit_length;
  }

  BL_INLINE_NODEBUG void store_state(Decoder* ctx) noexcept {
    ctx->_bit_word = bit_word;
    ctx->_bit_length = bit_length;
  }

  BL_INLINE_NODEBUG BLBitWord all() const noexcept { return bit_word; }
  BL_INLINE_NODEBUG size_t length() const noexcept { return bit_length; }

  BL_INLINE_NODEBUG bool is_empty() const noexcept { return bit_length == 0; }
  BL_INLINE_NODEBUG bool overflown() const noexcept { return bit_length > IntOps::bit_size_of<BLBitWord>(); }

  BL_INLINE_NODEBUG bool can_refill_byte() const noexcept {
    if constexpr (sizeof(BLBitWord) >= 8)
      return bit_length < (IntOps::bit_size_of<BLBitWord>() - 8u);
    else
      return bit_length <= (IntOps::bit_size_of<BLBitWord>() - 8u);
  }

  BL_INLINE_NODEBUG void refill_byte(uint8_t b) noexcept {
    BL_ASSERT(can_refill_byte());

    bit_word |= BLBitWord(b) << bit_length;
    bit_length += 8;
  }

  BL_INLINE_NODEBUG size_t calculate_bit_word_refill_count() const noexcept {
    constexpr size_t kFullMinusOne = sizeof(BLBitWord) - 1;
    return kFullMinusOne - ((bit_length >> 3) & 0x7u);
  }

  BL_INLINE_NODEBUG size_t refill_bit_word(BLBitWord b) noexcept {
    bit_word |= b << (bit_length & (IntOps::bit_size_of<BLBitWord>() - 1));
    size_t refill_size = (~bit_length >> 3) & (sizeof(BLBitWord) - 1);

    bit_length |= IntOps::bit_size_of<BLBitWord>() - 8u;
    return refill_size;
  }

  template<size_t Index = 0>
  BL_INLINE_NODEBUG uint32_t extract(size_t n) const noexcept {
    return DecoderUtils::extract_n(bit_word >> Index, n);
  }

  BL_INLINE_NODEBUG uint32_t extract(DecoderTableMask msk) const noexcept {
    return msk.extract_index(bit_word);
  }

  BL_INLINE_NODEBUG uint32_t extract(DecodeEntry entry) const noexcept {
    return DecoderUtils::extract_n(bit_word, entry.value);
  }

  BL_INLINE_NODEBUG uint32_t extract_extra(DecodeEntry entry) noexcept {
    return DecoderUtils::extract_extra(bit_word, entry);
  }

  BL_INLINE_NODEBUG uint32_t and_(uint32_t mask) const noexcept {
    return uint32_t(bit_word & mask);
  }

  BL_INLINE_NODEBUG uint32_t operator&(uint32_t mask) const noexcept {
    return uint32_t(bit_word & mask);
  }

  BL_INLINE_NODEBUG void consumed(size_t n) noexcept {
    bit_word >>= n;
    bit_length -= n;
  }

  BL_INLINE_NODEBUG void consumed_unsafe(uint32_t n) noexcept {
    bit_word >>= n & (IntOps::bit_size_of<BLBitWord>() - 1u);
    bit_length -= n;
  }

  BL_INLINE_NODEBUG void consumed(DecodeEntry n) noexcept {
    consumed_unsafe(n.value);
  }

  BL_INLINE_NODEBUG bool is_byte_aligned() const noexcept { return (bit_length & 0x7u) == 0u; }
  BL_INLINE_NODEBUG void make_byte_aligned() noexcept { consumed(bit_length & 0x7u); }

  BL_INLINE_NODEBUG void fix_length_after_fast_loop() noexcept {
    if constexpr (sizeof(BLBitWord) >= 8) {
      bit_length &= (IntOps::bit_size_of<BLBitWord>() - 1u);
    }
  }
};

// Copy at least BLBitWord quantities.
template<size_t N>
BL_INLINE void copy_bitwords(uint8_t* dst, const uint8_t* src) noexcept {
  BL_STATIC_ASSERT((N % sizeof(BLBitWord)) == 0);

  for (size_t i = 0; i < N; i += sizeof(BLBitWord)) {
    BLBitWord v = MemOps::loadu<BLBitWord>(src + i);
    MemOps::storeu(dst + i, v);
  }
}

template<size_t N>
BL_INLINE void fill_bitwords(uint8_t* dst, BLBitWord src) noexcept {
  BL_STATIC_ASSERT((N % sizeof(BLBitWord)) == 0);

  for (size_t i = 0; i < N; i += sizeof(BLBitWord)) {
    MemOps::storeu(dst + i, src);
  }
}

} // {anonymous}

extern const BLBitWord kScalarRepeatMultiply[sizeof(BLBitWord)];
extern const uint8_t kScalarRotatePredicateL[sizeof(BLBitWord)];
extern const uint8_t kScalarRotatePredicateR[sizeof(BLBitWord)];

struct BL_ALIGN_TYPE(SimdRepeatTable16, 16) { uint8_t data[16]; };
extern const SimdRepeatTable16 kSimdRepeatTable16[16];
extern const SimdRepeatTable16 kSimdRotateTable16[16];

namespace {

class ScalarCopyContext {
public:
  using Register = BLBitWord;

  Register repeat_mask;
  Register repeat_pred;
  Register rotate_left;
  Register rotate_right;

  BL_INLINE void init_repeat(size_t offset) noexcept {
    BL_ASSERT(offset < sizeof(Register));

    repeat_mask = IntOps::all_ones<BLBitWord>() >> (64u - (offset * 8u));
    repeat_pred = BLBitWord(kScalarRepeatMultiply[offset]);
  }

  BL_INLINE void init_rotate(size_t offset) noexcept {
    BL_ASSERT(offset < sizeof(Register));

    rotate_left = kScalarRotatePredicateL[offset];
    rotate_right = kScalarRotatePredicateR[offset];
  }

  static BL_INLINE Register load(const uint8_t* src) noexcept { return MemOps::loadu_le<BLBitWord>(src); }
  static BL_INLINE Register load_raw(const uint8_t* src) noexcept { return MemOps::loadu<BLBitWord>(src); }

  static BL_INLINE void store(uint8_t* dst, Register r) noexcept { MemOps::storeu_le(dst, r); }
  static BL_INLINE void store_raw(uint8_t* dst, Register r) noexcept { MemOps::storeu(dst, r); }

  BL_INLINE Register repeat(Register r) const noexcept { return (r & repeat_mask) * repeat_pred; }
  BL_INLINE Register rotate(Register r) noexcept { return (r >> rotate_right) | (r << rotate_left); }
};

#if defined(BL_DECODER_USE_SWIZZLEV)
class SimdCopyContext {
public:
  using Register = SIMD::Vec16xU8;

  Register repeat_predicate;
  Register rotate_predicate;

  BL_INLINE void init_repeat(size_t offset) noexcept {
    BL_ASSERT(offset < sizeof(Register));

    repeat_predicate = SIMD::loada_128<SIMD::Vec16xU8>(&kSimdRepeatTable16[offset]);
  }

  BL_INLINE void init_rotate(size_t offset) noexcept {
    BL_ASSERT(offset < sizeof(Register));

    rotate_predicate = SIMD::loada_128<SIMD::Vec16xU8>(&kSimdRotateTable16[offset]);
  }

  BL_INLINE Register repeat(Register r) const noexcept { return SIMD::swizzlev_u8(r, repeat_predicate); }
  BL_INLINE Register rotate(Register r) const noexcept { return SIMD::swizzlev_u8(r, rotate_predicate); }

  static BL_INLINE Register load(const uint8_t* src) noexcept { return SIMD::loadu_128<Register>(src); }
  static BL_INLINE Register load_raw(const uint8_t* src) noexcept { return SIMD::loadu_128<Register>(src); }

  static BL_INLINE void store(uint8_t* dst, Register r) noexcept { SIMD::storeu_128(dst, r); }
  static BL_INLINE void store_raw(uint8_t* dst, Register r) noexcept { SIMD::storeu_128(dst, r); }
};

using CopyContext = SimdCopyContext;
#else
using CopyContext = ScalarCopyContext;
#endif

} // {anonymous}
} // {bl::Compression::Deflate}

//! \endcond

#endif // BLEND2D_COMPRESSION_DEFLATEDECODERUTILS_P_H_INCLUDED
