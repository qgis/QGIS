// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#include <asmjit/core/codeholder.h>
#include <asmjit/core/codewriter_p.h>
#include <asmjit/core/emitterutils_p.h>

#include <asmjit/arm/armutils.h>

ASMJIT_BEGIN_NAMESPACE

bool CodeWriterUtils::encode_offset32(uint32_t* dst, int64_t offset64, const OffsetFormat& format) noexcept {
  uint32_t bit_count = format.imm_bit_count();
  uint32_t bit_shift = format.imm_bit_shift();
  uint32_t discard_lsb = format.imm_discard_lsb();

  // Invalid offset (should not happen).
  if (!bit_count || bit_count > format.value_size() * 8u) {
    return false;
  }

  uint32_t value;
  uint32_t u = 0;
  bool unsigned_logic = format.type() == OffsetType::kUnsignedOffset;

  // First handle all offsets that use additional field for their sign and the offset is encoded as its
  // absolute value.
  if (format.has_sign_bit()) {
    u = uint32_t(offset64 >= 0);
    if (u == 0) {
      offset64 = -offset64;
    }
    unsigned_logic = true;
  }

  // First handle all unsigned offset types.
  if (unsigned_logic) {
    if (discard_lsb) {
      ASMJIT_ASSERT(discard_lsb <= 32);
      if ((offset64 & Support::lsb_mask<uint32_t>(discard_lsb)) != 0) {
        return false;
      }
      offset64 = int64_t(uint64_t(offset64) >> discard_lsb);
    }

    value = uint32_t(offset64 & Support::lsb_mask<uint32_t>(bit_count));
    if (value != offset64) {
      return false;
    }
  }
  else {
    // The rest of OffsetType options are all signed.
    if (discard_lsb) {
      ASMJIT_ASSERT(discard_lsb <= 32);
      if ((offset64 & Support::lsb_mask<uint32_t>(discard_lsb)) != 0) {
        return false;
      }
      offset64 >>= discard_lsb;
    }

    if (!Support::is_int_n<32>(offset64)) {
      return false;
    }

    value = uint32_t(int32_t(offset64));
    if (!EmitterUtils::is_encodable_offset_32(int32_t(value), bit_count)) {
      return false;
    }
  }

  switch (format.type()) {
    case OffsetType::kSignedOffset:
    case OffsetType::kUnsignedOffset: {
      *dst = (value & Support::lsb_mask<uint32_t>(bit_count)) << bit_shift;
      return true;
    }

    // Opcode: {.....|imm:1|..N.N|......|imm:3|....|imm:8}
    case OffsetType::kThumb32_ADR: {
      // Sanity checks.
      if (format.value_size() != 4 || bit_count != 12 || bit_shift != 0) {
        return false;
      }

      uint32_t imm8 = (value & 0x00FFu);
      uint32_t imm3 = (value & 0x0700u) << (12 - 8);
      uint32_t imm1 = (value & 0x0800u) << (26 - 11);
      uint32_t n = u ^ 1u;

      *dst = imm8 | imm3 | imm1 | (n << 21) | (n << 23);
      return true;
    }

    // Opcode: {....|.|imm[22]|imm[19:10]|..|ja|.|jb|imm[9:0]|.}
    case OffsetType::kThumb32_BLX:
      // The calculation is the same as `B`, but the first LSB bit must be zero, so account for that.
      value <<= 1;
      [[fallthrough]];

    // Opcode: {....|.|imm[23]|imm[20:11]|..|ja|.|jb|imm[10:0]}
    case OffsetType::kThumb32_B: {
      // Sanity checks.
      if (format.value_size() != 4) {
        return false;
      }

      uint32_t ia = (value & 0x0007FFu);
      uint32_t ib = (value & 0x1FF800u) << (16 - 11);
      uint32_t ic = (value & 0x800000u) << (26 - 23);
      uint32_t ja = ((~value >> 23) ^ (value >> 22)) & 1u;
      uint32_t jb = ((~value >> 23) ^ (value >> 21)) & 1u;

      *dst = ia | ib | ic | (ja << 14) | (jb << 11);
      return true;
    }

    // Opcode: {....|.|imm[19]|....|imm[16:11]|..|ja|.|jb|imm[10:0]}
    case OffsetType::kThumb32_BCond: {
      // Sanity checks.
      if (format.value_size() != 4 || bit_count != 20 || bit_shift != 0) {
        return false;
      }

      uint32_t ia = (value & 0x0007FFu);
      uint32_t ib = (value & 0x01F800u) << (16 - 11);
      uint32_t ic = (value & 0x080000u) << (26 - 19);
      uint32_t ja = ((~value >> 19) ^ (value >> 22)) & 1u;
      uint32_t jb = ((~value >> 19) ^ (value >> 21)) & 1u;

      *dst = ia | ib | ic | (ja << 14) | (jb << 11);
      return true;
    }

    case OffsetType::kAArch32_ADR: {
      uint32_t encoded_imm;
      if (!arm::Utils::encode_aarch32_imm(value, Out(encoded_imm))) {
        return false;
      }

      *dst = (Support::bit_mask<uint32_t>(22) << u) | (encoded_imm << bit_shift);
      return true;
    }

    case OffsetType::kAArch32_U23_SignedOffset: {
      *dst = (value << bit_shift) | (u << 23);
      return true;
    }

    case OffsetType::kAArch32_U23_0To3At0_4To7At8: {
      // Sanity checks.
      if (format.value_size() != 4 || bit_count != 8 || bit_shift != 0) {
        return false;
      }

      uint32_t imm_lo = (value & 0x0Fu);
      uint32_t imm_hi = (value & 0xF0u) << (8 - 4);

      *dst = imm_lo | imm_hi | (u << 23);
      return true;
    }

    case OffsetType::kAArch32_1To24At0_0At24: {
      // Sanity checks.
      if (format.value_size() != 4 || bit_count != 25 || bit_shift != 0) {
        return false;
      }

      uint32_t imm_lo = (value & 0x0000001u) << 24;
      uint32_t imm_hi = (value & 0x1FFFFFEu) >> 1;

      *dst = imm_lo | imm_hi;
      return true;
    }

    case OffsetType::kAArch64_ADR:
    case OffsetType::kAArch64_ADRP: {
      // Sanity checks.
      if (format.value_size() != 4 || bit_count != 21 || bit_shift != 5) {
        return false;
      }

      uint32_t imm_lo = value & 0x3u;
      uint32_t imm_hi = (value >> 2) & Support::lsb_mask<uint32_t>(19);

      *dst = (imm_lo << 29) | (imm_hi << 5);
      return true;
    }

    default:
      return false;
  }
}

bool CodeWriterUtils::encode_offset64(uint64_t* dst, int64_t offset64, const OffsetFormat& format) noexcept {
  uint32_t bit_count = format.imm_bit_count();
  uint32_t discard_lsb = format.imm_discard_lsb();

  if (!bit_count || bit_count > format.value_size() * 8u) {
    return false;
  }

  uint64_t value;

  // First handle all unsigned offset types.
  if (format.type() == OffsetType::kUnsignedOffset) {
    if (discard_lsb) {
      ASMJIT_ASSERT(discard_lsb <= 32);
      if ((offset64 & Support::lsb_mask<uint32_t>(discard_lsb)) != 0) {
        return false;
      }
      offset64 = int64_t(uint64_t(offset64) >> discard_lsb);
    }

    value = uint64_t(offset64) & Support::lsb_mask<uint64_t>(bit_count);
    if (value != uint64_t(offset64)) {
      return false;
    }
  }
  else {
    // The rest of OffsetType options are all signed.
    if (discard_lsb) {
      ASMJIT_ASSERT(discard_lsb <= 32);
      if ((offset64 & Support::lsb_mask<uint32_t>(discard_lsb)) != 0) {
        return false;
      }
      offset64 >>= discard_lsb;
    }

    if (!EmitterUtils::is_encodable_offset_64(offset64, bit_count)) {
      return false;
    }

    value = uint64_t(offset64);
  }

  switch (format.type()) {
    case OffsetType::kSignedOffset:
    case OffsetType::kUnsignedOffset: {
      *dst = (value & Support::lsb_mask<uint64_t>(bit_count)) << format.imm_bit_shift();
      return true;
    }

    default:
      return false;
  }
}

bool CodeWriterUtils::write_offset(void* dst, int64_t offset64, const OffsetFormat& format) noexcept {
  // Offset the destination by ValueOffset so the `dst` points to the
  // patched word instead of the beginning of the patched region.
  dst = static_cast<char*>(dst) + format.value_offset();

  switch (format.value_size()) {
    case 1: {
      uint32_t mask;
      if (!encode_offset32(&mask, offset64, format)) {
        return false;
      }

      Support::store_u8(dst, uint8_t(Support::load_u8(dst) | mask));
      return true;
    }

    case 2: {
      uint32_t mask;
      if (!encode_offset32(&mask, offset64, format)) {
        return false;
      }

      Support::storeu_u16_le(dst, uint16_t(Support::loadu_u16_le(dst) | mask));
      return true;
    }

    case 4: {
      uint32_t mask;
      if (!encode_offset32(&mask, offset64, format)) {
        return false;
      }

      Support::storeu_u32_le(dst, Support::loadu_u32_le(dst) | mask);
      return true;
    }

    case 8: {
      uint64_t mask;
      if (!encode_offset64(&mask, offset64, format)) {
        return false;
      }

      Support::storeu_u64_le(dst, Support::loadu_u64_le(dst) | mask);
      return true;
    }

    default:
      return false;
  }
}

ASMJIT_END_NAMESPACE
