// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#if !defined(ASMJIT_NO_X86)

#include <asmjit/core/assembler.h>
#include <asmjit/core/codewriter_p.h>
#include <asmjit/core/cpuinfo.h>
#include <asmjit/core/emitterutils_p.h>
#include <asmjit/core/formatter.h>
#include <asmjit/core/logger.h>
#include <asmjit/core/misc_p.h>
#include <asmjit/x86/x86assembler.h>
#include <asmjit/x86/x86emithelper_p.h>
#include <asmjit/x86/x86instapi_p.h>
#include <asmjit/x86/x86instdb_p.h>
#include <asmjit/x86/x86formatter_p.h>
#include <asmjit/x86/x86opcode_p.h>
#include <asmjit/support/support.h>

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

using FastUInt8 = Support::FastUInt8;

// x86::Assembler - Constants
// ==========================

//! X86 bytes used to encode important prefixes.
enum X86Byte : uint32_t {
  //! 1-byte REX prefix mask.
  kX86ByteRex = 0x40,

  //! 1-byte REX.W component.
  kX86ByteRexW = 0x08,

  kX86ByteInvalidRex = 0x80,

  //! 2-byte VEX prefix:
  //!   - `[0]` - `0xC5`.
  //!   - `[1]` - `RvvvvLpp`.
  kX86ByteVex2 = 0xC5,

  //! 3-byte VEX prefix:
  //!   - `[0]` - `0xC4`.
  //!   - `[1]` - `RXBmmmmm`.
  //!   - `[2]` - `WvvvvLpp`.
  kX86ByteVex3 = 0xC4,

  //! 3-byte XOP prefix:
  //!   - `[0]` - `0x8F`.
  //!   - `[1]` - `RXBmmmmm`.
  //!   - `[2]` - `WvvvvLpp`.
  kX86ByteXop3 = 0x8F,

  //! 4-byte EVEX prefix:
  //!   - `[0]` - `0x62`.
  //!   - `[1]` - Payload0 or `P[ 7: 0]` - `[R  X  B  R' 0  m  m  m]`.
  //!   - `[2]` - Payload1 or `P[15: 8]` - `[W  v  v  v  v  1  p  p]`.
  //!   - `[3]` - Payload2 or `P[23:16]` - `[z  L' L  b  V' a  a  a]`.
  //!
  //! Payload:
  //!   - `P[ 2: 0]` - OPCODE: EVEX.mmmmm, only lowest 3 bits [2:0] used.
  //!   - `P[    3]` - ______: Must be 0.
  //!   - `P[    4]` - REG-ID: EVEX.R' - 5th bit of 'RRRRR'.
  //!   - `P[    5]` - REG-ID: EVEX.B  - 4th bit of 'BBBBB'.
  //!   - `P[    6]` - REG-ID: EVEX.X  - 5th bit of 'BBBBB' or 4th bit of 'XXXX' (with SIB).
  //!   - `P[    7]` - REG-ID: EVEX.R  - 4th bit of 'RRRRR'.
  //!   - `P[ 9: 8]` - OPCODE: EVEX.pp.
  //!   - `P[   10]` - ______: Must be 1.
  //!   - `P[14:11]` - REG-ID: 4 bits of 'VVVV'.
  //!   - `P[   15]` - OPCODE: EVEX.W.
  //!   - `P[18:16]` - REG-ID: K register k0...k7 (Merging/Zeroing Vector Ops).
  //!   - `P[   19]` - REG-ID: 5th bit of 'VVVVV'.
  //!   - `P[   20]` - OPCODE: Broadcast/Rounding Control/SAE bit.
  //!   - `P[22.21]` - OPCODE: Vector Length (L' and  L) / Rounding Control.
  //!   - `P[   23]` - OPCODE: Zeroing/Merging.
  kX86ByteEvex = 0x62
};

// AsmJit specific (used to encode VVVVV field in XOP/VEX/EVEX).
enum VexVVVVV : uint32_t {
  kVexVVVVVShift = 7,
  kVexVVVVVMask = 0x1F << kVexVVVVVShift
};

//! Instruction 2-byte/3-byte opcode prefix definition.
struct X86OpcodeMM {
  uint8_t size;
  uint8_t data[3];
};

//! Mandatory prefixes used to encode legacy [66, F3, F2] or [9B] byte.
static const uint8_t opcode_pp_table[8] = { 0x00, 0x66, 0xF3, 0xF2, 0x00, 0x00, 0x00, 0x9B };

//! Instruction 2-byte/3-byte opcode prefix data.
static const X86OpcodeMM opcode_mm_table[] = {
  { 0, { 0x00, 0x00, 0 } }, // #00 (0b0000).
  { 1, { 0x0F, 0x00, 0 } }, // #01 (0b0001).
  { 2, { 0x0F, 0x38, 0 } }, // #02 (0b0010).
  { 2, { 0x0F, 0x3A, 0 } }, // #03 (0b0011).
  { 2, { 0x0F, 0x01, 0 } }, // #04 (0b0100).
  { 0, { 0x00, 0x00, 0 } }, // #05 (0b0101).
  { 0, { 0x00, 0x00, 0 } }, // #06 (0b0110).
  { 0, { 0x00, 0x00, 0 } }, // #07 (0b0111).
  { 0, { 0x00, 0x00, 0 } }, // #08 (0b1000).
  { 0, { 0x00, 0x00, 0 } }, // #09 (0b1001).
  { 0, { 0x00, 0x00, 0 } }, // #0A (0b1010).
  { 0, { 0x00, 0x00, 0 } }, // #0B (0b1011).
  { 0, { 0x00, 0x00, 0 } }, // #0C (0b1100).
  { 0, { 0x00, 0x00, 0 } }, // #0D (0b1101).
  { 0, { 0x00, 0x00, 0 } }, // #0E (0b1110).
  { 0, { 0x00, 0x00, 0 } }  // #0F (0b1111).
};

static const uint8_t segment_prefix_table[8] = {
  0x00, // None.
  0x26, // ES.
  0x2E, // CS.
  0x36, // SS.
  0x3E, // DS.
  0x64, // FS.
  0x65  // GS.
};

static const uint32_t opcode_push_sreg_table[8] = {
  Opcode::k000000 | 0x00, // None.
  Opcode::k000000 | 0x06, // Push ES.
  Opcode::k000000 | 0x0E, // Push CS.
  Opcode::k000000 | 0x16, // Push SS.
  Opcode::k000000 | 0x1E, // Push DS.
  Opcode::k000F00 | 0xA0, // Push FS.
  Opcode::k000F00 | 0xA8  // Push GS.
};

static const uint32_t opcode_pop_sreg_table[8]  = {
  Opcode::k000000 | 0x00, // None.
  Opcode::k000000 | 0x07, // Pop ES.
  Opcode::k000000 | 0x00, // Pop CS.
  Opcode::k000000 | 0x17, // Pop SS.
  Opcode::k000000 | 0x1F, // Pop DS.
  Opcode::k000F00 | 0xA1, // Pop FS.
  Opcode::k000F00 | 0xA9  // Pop GS.
};

// x86::Assembler - X86MemInfo | X86VEXPrefix | X86LLByRegType | X86CDisp8Table
// ============================================================================

//! Memory operand's info bits.
//!
//! A lookup table that contains various information based on the BASE and INDEX information of a memory operand. This
//! is much better and safer than playing with IFs in the code and can check for errors must faster and better.
enum X86MemInfo_Enum {
  kX86MemInfo_0         = 0x00,

  kX86MemInfo_BaseGp    = 0x01, //!< Has BASE reg, REX.B can be 1, compatible with REX.B byte.
  kX86MemInfo_Index     = 0x02, //!< Has INDEX reg, REX.X can be 1, compatible with REX.X byte.

  kX86MemInfo_BaseLabel = 0x10, //!< Base is Label.
  kX86MemInfo_BaseRip   = 0x20, //!< Base is RIP.

  kX86MemInfo_67H_X86   = 0x40, //!< Address-size override in 32-bit mode.
  kX86MemInfo_67H_X64   = 0x80, //!< Address-size override in 64-bit mode.
  kX86MemInfo_67H_Mask  = 0xC0  //!< Contains all address-size override bits.
};

template<uint32_t X>
struct X86MemInfo_T {
  static inline constexpr uint32_t B = (X     ) & 0x1F;
  static inline constexpr uint32_t I = (X >> 5) & 0x1F;

  static inline constexpr uint32_t kBase =
    (B >= uint32_t(RegType::kGp16)     && B <= uint32_t(RegType::kGp64)   ) ? kX86MemInfo_BaseGp    :
    (B == uint32_t(RegType::kPC)                                          ) ? kX86MemInfo_BaseRip   :
    (B == uint32_t(RegType::kLabelTag)                                    ) ? kX86MemInfo_BaseLabel : 0;

  static inline constexpr uint32_t kIndex =
    (I >= uint32_t(RegType::kGp16)     && I <= uint32_t(RegType::kGp64)   ) ? kX86MemInfo_Index     :
    (I >= uint32_t(RegType::kVec128)   && I <= uint32_t(RegType::kVec512) ) ? kX86MemInfo_Index     : 0;

  static inline constexpr uint32_t k67H =
    (B == uint32_t(RegType::kGp16)     && I == uint32_t(RegType::kNone)   ) ? kX86MemInfo_67H_X86   :
    (B == uint32_t(RegType::kGp32)     && I == uint32_t(RegType::kNone)   ) ? kX86MemInfo_67H_X64   :
    (B == uint32_t(RegType::kNone)     && I == uint32_t(RegType::kGp16)   ) ? kX86MemInfo_67H_X86   :
    (B == uint32_t(RegType::kNone)     && I == uint32_t(RegType::kGp32)   ) ? kX86MemInfo_67H_X64   :
    (B == uint32_t(RegType::kGp16)     && I == uint32_t(RegType::kGp16)   ) ? kX86MemInfo_67H_X86   :
    (B == uint32_t(RegType::kGp32)     && I == uint32_t(RegType::kGp32)   ) ? kX86MemInfo_67H_X64   :
    (B == uint32_t(RegType::kGp16)     && I == uint32_t(RegType::kVec128) ) ? kX86MemInfo_67H_X86   :
    (B == uint32_t(RegType::kGp32)     && I == uint32_t(RegType::kVec128) ) ? kX86MemInfo_67H_X64   :
    (B == uint32_t(RegType::kGp16)     && I == uint32_t(RegType::kVec256) ) ? kX86MemInfo_67H_X86   :
    (B == uint32_t(RegType::kGp32)     && I == uint32_t(RegType::kVec256) ) ? kX86MemInfo_67H_X64   :
    (B == uint32_t(RegType::kGp16)     && I == uint32_t(RegType::kVec512) ) ? kX86MemInfo_67H_X86   :
    (B == uint32_t(RegType::kGp32)     && I == uint32_t(RegType::kVec512) ) ? kX86MemInfo_67H_X64   :
    (B == uint32_t(RegType::kLabelTag) && I == uint32_t(RegType::kGp16)   ) ? kX86MemInfo_67H_X86   :
    (B == uint32_t(RegType::kLabelTag) && I == uint32_t(RegType::kGp32)   ) ? kX86MemInfo_67H_X64   : 0;

  static inline constexpr uint32_t kValue = kBase | kIndex | k67H | 0x04u | 0x08u;
};

// The result stored in the LUT is a combination of
//   - 67H - Address override prefix - depends on BASE+INDEX register types and the target architecture.
//   - REX - A possible combination of REX.[B|X|R|W] bits in REX prefix where REX.B and REX.X are possibly
//           masked out, but REX.R and REX.W are kept as is.
#define VALUE(x) X86MemInfo_T<x>::kValue
static const uint8_t mem_info_table[] = { ASMJIT_LOOKUP_TABLE_1024(VALUE, 0) };
#undef VALUE

// VEX3 or XOP xor bits applied to the opcode before emitted. The index to this table is 'mmmmm' value, which
// contains all we need. This is only used by a 3 BYTE VEX and XOP prefixes, 2 BYTE VEX prefix is handled differently.
// The idea is to minimize the difference between VEX3 vs XOP when encoding VEX or XOP instruction. This should
// minimize the code required to emit such instructions and should also make it faster as we don't need any branch to
// decide between VEX3 vs XOP.
//            ____    ___
// [_OPCODE_|WvvvvLpp|RXBmmmmm|VEX3_XOP]
#define VALUE(x) ((x & 0x08) ? kX86ByteXop3 : kX86ByteVex3) | (0xF << 19) | (0x7 << 13)
static const uint32_t vex_prefix_table[] = { ASMJIT_LOOKUP_TABLE_16(VALUE, 0) };
#undef VALUE

// Table that contains LL opcode field addressed by a register size / 16. It's used to propagate L.256 or L.512 when
// YMM or ZMM registers are used, respectively.
#define VALUE(x) (x & (64 >> 4)) ? Opcode::kLL_2 : \
                 (x & (32 >> 4)) ? Opcode::kLL_1 : Opcode::kLL_0
static const uint32_t ll_by_size_div_16_table[] = { ASMJIT_LOOKUP_TABLE_16(VALUE, 0) };
#undef VALUE

// Table that contains LL opcode field addressed by a register size / 16. It's used to propagate L.256 or L.512 when
// YMM or ZMM registers are used, respectively.
#define VALUE(x) x == uint32_t(RegType::kVec512) ? Opcode::kLL_2 : \
                 x == uint32_t(RegType::kVec256) ? Opcode::kLL_1 : Opcode::kLL_0
static const uint32_t ll_by_reg_type_table[] = { ASMJIT_LOOKUP_TABLE_16(VALUE, 0) };
#undef VALUE

// Table that contains a scale (shift left) based on 'TTWLL' field and the instruction's tuple-type (TT) field. The
// scale is then applied to the BASE-N stored in each opcode to calculate the final compressed displacement used by
// all EVEX encoded instructions.
template<uint32_t X>
struct X86CDisp8SHL_T {
  static inline constexpr uint32_t TT = (X >> 3) << Opcode::kCDTT_Shift;
  static inline constexpr uint32_t LL = (X >> 0) & 0x3;
  static inline constexpr uint32_t W  = (X >> 2) & 0x1;

  static inline constexpr uint32_t kValue = (
    TT == Opcode::kCDTT_None ? 0u :
    TT == Opcode::kCDTT_ByLL ? ((LL==0) ? 0 : (LL==1) ? 1u     : 2u   ) :
    TT == Opcode::kCDTT_T1W  ? ((LL==0) ? W : (LL==1) ? 1u + W : 2u + W) :
    TT == Opcode::kCDTT_DUP  ? ((LL==0) ? 0 : (LL==1) ? 2u     : 3u   ) : 0) << Opcode::kCDSHL_Shift;
};

#define VALUE(x) X86CDisp8SHL_T<x>::kValue
static const uint32_t cdisp8_shl_table[] = { ASMJIT_LOOKUP_TABLE_32(VALUE, 0) };
#undef VALUE

// Table that contains MOD byte of a 16-bit [BASE + disp] address.
//   0xFF == Invalid.
static const uint8_t mod16_base_table[8] = {
  0xFF, // AX -> N/A.
  0xFF, // CX -> N/A.
  0xFF, // DX -> N/A.
  0x07, // BX -> 111.
  0xFF, // SP -> N/A.
  0x06, // BP -> 110.
  0x04, // SI -> 100.
  0x05  // DI -> 101.
};

// Table that contains MOD byte of a 16-bit [BASE + INDEX + disp] combination.
//   0xFF == Invalid.
template<uint32_t X>
struct X86Mod16BaseIndexTable_T {
  static inline constexpr uint32_t B = X >> 3;
  static inline constexpr uint32_t I = X & 0x7u;

  static inline constexpr uint32_t kValue =
    ((B == Gp::kIdBx && I == Gp::kIdSi) || (B == Gp::kIdSi && I == Gp::kIdBx)) ? 0x00u :
    ((B == Gp::kIdBx && I == Gp::kIdDi) || (B == Gp::kIdDi && I == Gp::kIdBx)) ? 0x01u :
    ((B == Gp::kIdBp && I == Gp::kIdSi) || (B == Gp::kIdSi && I == Gp::kIdBp)) ? 0x02u :
    ((B == Gp::kIdBp && I == Gp::kIdDi) || (B == Gp::kIdDi && I == Gp::kIdBp)) ? 0x03u : 0xFFu;
};

#define VALUE(x) X86Mod16BaseIndexTable_T<x>::kValue
static const uint8_t mod16_base_index_table[] = { ASMJIT_LOOKUP_TABLE_64(VALUE, 0) };
#undef VALUE

// x86::Assembler - Helpers
// ========================

static ASMJIT_INLINE bool is_jmp_or_call(InstId inst_id) noexcept {
  return inst_id == Inst::kIdJmp || inst_id == Inst::kIdCall;
}

static ASMJIT_INLINE bool is_implicit_mem(const Operand_& op, uint32_t base) noexcept {
  return op.is_mem() && op.as<Mem>().base_id() == base && !op.as<Mem>().has_offset();
}

//! Combine `reg_id` and `vvvvv_id` into a single value (used by AVX and AVX-512).
static ASMJIT_INLINE uint32_t pack_reg_and_vvvvv(uint32_t reg_id, uint32_t vvvvv_id) noexcept {
  return reg_id + (vvvvv_id << kVexVVVVVShift);
}

static ASMJIT_INLINE uint32_t opcode_l_by_vmem(const Operand_& op) noexcept {
  return ll_by_reg_type_table[size_t(op.as<Mem>().index_type())];
}

static ASMJIT_INLINE uint32_t opcode_l_by_size(uint32_t size) noexcept {
  return ll_by_size_div_16_table[size / 16];
}

//! Encode MOD byte.
static ASMJIT_INLINE uint32_t encode_mod(uint32_t m, uint32_t o, uint32_t rm) noexcept {
  ASMJIT_ASSERT(m <= 3);
  ASMJIT_ASSERT(o <= 7);
  ASMJIT_ASSERT(rm <= 7);
  return (m << 6) + (o << 3) + rm;
}

//! Encode SIB byte.
static ASMJIT_INLINE uint32_t encode_sib(uint32_t s, uint32_t i, uint32_t b) noexcept {
  ASMJIT_ASSERT(s <= 3);
  ASMJIT_ASSERT(i <= 7);
  ASMJIT_ASSERT(b <= 7);
  return (s << 6) + (i << 3) + b;
}

static ASMJIT_INLINE bool is_rex_invalid(uint32_t rex) noexcept {
  // Validates the following possibilities:
  //   REX == 0x00      -> OKAY (X86_32 / X86_64).
  //   REX == 0x40-0x4F -> OKAY (X86_64).
  //   REX == 0x80      -> OKAY (X86_32 mode, rex prefix not used).
  //   REX == 0x81-0xCF -> BAD  (X86_32 mode, rex prefix used).
  return rex > kX86ByteInvalidRex;
}

static ASMJIT_INLINE uint32_t x86_get_force_evex3_mask_in_last_bit(InstOptions options) noexcept {
  constexpr uint32_t kVex3Bit = Support::ctz_const<InstOptions::kX86_Vex3>;
  return uint32_t(options & InstOptions::kX86_Vex3) << (31 - kVex3Bit);
}

template<typename T>
static ASMJIT_INLINE_CONSTEXPR T sign_extend_int32(T imm) noexcept { return T(int64_t(int32_t(imm & T(0xFFFFFFFF)))); }

static ASMJIT_INLINE uint32_t alt_opcode_of(const InstDB::InstInfo* info) noexcept {
  return InstDB::alt_opcode_table[info->_alt_opcode_index];
}

static ASMJIT_INLINE bool is_mmx_or_xmm(const Reg& reg) noexcept {
  return Support::bool_or(reg.reg_type() == RegType::kX86_Mm, reg.reg_type() == RegType::kVec128);
}

// x86::Assembler - X86BufferWriter
// ================================

class X86BufferWriter : public CodeWriter {
public:
  ASMJIT_INLINE explicit X86BufferWriter(Assembler* a) noexcept
    : CodeWriter(a) {}

  ASMJIT_INLINE void emit_pp(uint32_t opcode) noexcept {
    uint32_t pp_index = (opcode              >> Opcode::kPP_Shift) &
                        (Opcode::kPP_FPUMask >> Opcode::kPP_Shift) ;
    emit8_if(opcode_pp_table[pp_index], pp_index != 0);
  }

  ASMJIT_INLINE void emit_mm_and_opcode(uint32_t opcode) noexcept {
    uint32_t mm_index = (opcode & Opcode::kMM_Mask) >> Opcode::kMM_Shift;
    const X86OpcodeMM& mm_code = opcode_mm_table[mm_index];

    emit8_if(mm_code.data[0], mm_code.size > 0);
    emit8_if(mm_code.data[1], mm_code.size > 1);
    emit8(opcode);
  }

  ASMJIT_INLINE void emit_segment_override(uint32_t segment_id) noexcept {
    ASMJIT_ASSERT(segment_id < ASMJIT_ARRAY_SIZE(segment_prefix_table));

    FastUInt8 prefix = segment_prefix_table[segment_id];
    emit8_if(prefix, prefix != 0);
  }

  template<typename CondT>
  ASMJIT_INLINE void emit_address_override(CondT condition) noexcept {
    emit8_if(0x67, condition);
  }

  ASMJIT_INLINE void emit_imm_byte_or_dword(uint64_t imm_value, FastUInt8 imm_size) noexcept {
    if (!imm_size) {
      return;
    }

    ASMJIT_ASSERT(imm_size == 1 || imm_size == 4);

#if ASMJIT_ARCH_BITS >= 64
    uint64_t imm = uint64_t(imm_value);
#else
    uint32_t imm = uint32_t(imm_value & 0xFFFFFFFFu);
#endif

    // Many instructions just use a single byte immediate, so make it fast.
    emit8(imm & 0xFFu);
    if (imm_size == 1) return;

    imm >>= 8;
    emit8(imm & 0xFFu);
    imm >>= 8;
    emit8(imm & 0xFFu);
    imm >>= 8;
    emit8(imm & 0xFFu);
  }

  ASMJIT_INLINE void emit_immediate(uint64_t imm_value, FastUInt8 imm_size) noexcept {
#if ASMJIT_ARCH_BITS >= 64
    uint64_t imm = imm_value;
    if (imm_size >= 4) {
      emit32u_le(imm & 0xFFFFFFFFu);
      imm >>= 32;
      imm_size = FastUInt8(imm_size - 4u);
    }
#else
    uint32_t imm = uint32_t(imm_value & 0xFFFFFFFFu);
    if (imm_size >= 4) {
      emit32u_le(imm);
      imm = uint32_t(imm_value >> 32);
      imm_size = FastUInt8(imm_size - 4u);
    }
#endif

    if (!imm_size) {
      return;
    }
    emit8(imm & 0xFFu);
    imm >>= 8;

    if (--imm_size == 0) {
      return;
    }
    emit8(imm & 0xFFu);
    imm >>= 8;

    if (--imm_size == 0) {
      return;
    }
    emit8(imm & 0xFFu);
    imm >>= 8;

    if (--imm_size == 0) {
      return;
    }
    emit8(imm & 0xFFu);
  }
};

// If the operand is BPL|SPL|SIL|DIL|R8B-15B
//   - Force REX prefix
// If the operand is AH|BH|CH|DH
//   - patch its index from 0..3 to 4..7 as encoded by X86.
//   - Disallow REX prefix.
#define FIXUP_GPB(REG_OP, REG_ID)                                \
  do {                                                           \
    if (!static_cast<const Gp&>(REG_OP).is_gp8_hi()) {             \
      options |= (REG_ID) >= 4 ? InstOptions::kX86_Rex           \
                               : InstOptions::kNone;             \
    }                                                            \
    else {                                                       \
      options |= InstOptions::kX86_InvalidRex;                   \
      REG_ID += 4;                                               \
    }                                                            \
  } while (0)

#define ENC_OPS1(OP0) \
  (uint32_t(OperandType::k##OP0))

#define ENC_OPS2(OP0, OP1) \
  (uint32_t(OperandType::k##OP0) + \
  (uint32_t(OperandType::k##OP1) << 3))

#define ENC_OPS3(OP0, OP1, OP2) \
  (uint32_t(OperandType::k##OP0) + \
  (uint32_t(OperandType::k##OP1) << 3) + \
  (uint32_t(OperandType::k##OP2) << 6))

#define ENC_OPS4(OP0, OP1, OP2, OP3) \
  (uint32_t(OperandType::k##OP0) + \
  (uint32_t(OperandType::k##OP1) << 3) + \
  (uint32_t(OperandType::k##OP2) << 6) + \
  (uint32_t(OperandType::k##OP3) << 9))

// x86::Assembler - Movabs Heuristics
// ==================================

static ASMJIT_INLINE uint32_t x86_get_movabs_inst_size_64bit(uint32_t register_size, InstOptions options, const Mem& rm_rel) noexcept {
  uint32_t segment_prefix_size = rm_rel.segment_id() != 0;
  uint32_t _66h_prefix_size = register_size == 2;
  uint32_t rex_prefix_size = register_size == 8 || Support::test(options, InstOptions::kX86_Rex);
  uint32_t opcode_byte_size = 1;
  uint32_t immediate_size = 8;

  return segment_prefix_size + _66h_prefix_size + rex_prefix_size + opcode_byte_size + immediate_size;
}

static ASMJIT_INLINE bool x86_should_use_movabs(Assembler* self, X86BufferWriter& writer, uint32_t register_size, InstOptions options, const Mem& rm_rel) noexcept {
  if (self->is_32bit()) {
    // There is no relative addressing, just decide whether to use MOV encoded with MOD R/M or absolute.
    return !Support::test(options, InstOptions::kX86_ModMR | InstOptions::kX86_ModRM);
  }
  else {
    // If the addressing type is REL or MOD R/M was specified then absolute mov won't be used.
    if (rm_rel.addr_type() == Mem::AddrType::kRel || Support::test(options, InstOptions::kX86_ModMR | InstOptions::kX86_ModRM)) {
      return false;
    }

    int64_t addr_value = rm_rel.offset();
    uint64_t base_address = self->code()->base_address();

    // If the address type is default, it means to basically check whether relative addressing is possible. However,
    // this is only possible when the base address is known - relative encoding uses RIP+N it has to be calculated.
    if (rm_rel.addr_type() == Mem::AddrType::kDefault && base_address != Globals::kNoBaseAddress && !rm_rel.has_segment()) {
      uint32_t instruction_size = x86_get_movabs_inst_size_64bit(register_size, options, rm_rel);
      uint64_t virtual_offset = uint64_t(writer.offset_from(self->_buffer_data));
      uint64_t rip64 = base_address + self->_section->offset() + virtual_offset + instruction_size;
      uint64_t rel64 = uint64_t(addr_value) - rip64;

      if (Support::is_int_n<32>(int64_t(rel64))) {
        return false;
      }
    }
    else {
      if (Support::is_int_n<32>(addr_value)) {
        return false;
      }
    }

    return uint64_t(addr_value) > 0xFFFFFFFFu;
  }
}

// x86::Assembler - Construction & Destruction
// ===========================================

Assembler::Assembler(CodeHolder* code) noexcept : BaseAssembler() {
  _arch_mask = (uint64_t(1) << uint32_t(Arch::kX86)) |
               (uint64_t(1) << uint32_t(Arch::kX64)) ;
  init_emitter_funcs(this);

  if (code) {
    code->attach(this);
  }
}
Assembler::~Assembler() noexcept {}

// x86::Assembler - Emit (Low-Level)
// =================================

ASMJIT_FAVOR_SPEED Error Assembler::_emit(InstId inst_id, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_* op_ext) {
  constexpr uint32_t kVSHR_W     = Opcode::kW_Shift  - 23;
  constexpr uint32_t kVSHR_PP    = Opcode::kPP_Shift - 16;
  constexpr uint32_t kVSHR_PP_EW = Opcode::kPP_Shift - 16;

  constexpr InstOptions kRequiresSpecialHandling =
    InstOptions::kReserved     |     // Logging/Validation/Error.
    InstOptions::kX86_Rep      |     // REP/REPE prefix.
    InstOptions::kX86_Repne    |     // REPNE prefix.
    InstOptions::kX86_Lock     |     // LOCK prefix.
    InstOptions::kX86_XAcquire |     // XACQUIRE prefix.
    InstOptions::kX86_XRelease ;     // XRELEASE prefix.

  Error err;

  Opcode opcode;                     // Instruction opcode.
  InstOptions options;               // Instruction options.
  uint32_t isign3;                   // A combined signature of first 3 operands.

  const Operand_* rm_rel;            // Memory operand or operand that holds Label|Imm.
  uint32_t rm_info;                  // Memory operand's info based on mem_info_table.
  uint32_t rb_reg = 0;               // Memory base or modRM register.
  uint32_t rx_reg;                   // Memory index register.
  uint32_t op_reg;                   // ModR/M opcode or register id.

  LabelEntry* label;                 // Label entry.
  RelocEntry* re = nullptr;          // Relocation entry.
  int32_t rel_offset;                // Relative offset
  FastUInt8 rel_size = 0;            // Relative size.
  uint8_t* mem_op_ao_mark = nullptr; // Marker that points before 'address-override prefix' is emitted.

  int64_t imm_value = 0;             // Immediate value (must be 64-bit).
  FastUInt8 imm_size = 0;            // Immediate size.

  X86BufferWriter writer(this);

  if (inst_id >= Inst::_kIdCount) {
    inst_id = 0;
  }

  const InstDB::InstInfo* inst_info = &InstDB::_inst_info_table[inst_id];
  const InstDB::CommonInfo* common_info = &inst_info->common_info();

  // Signature of the first 3 operands.
  isign3 = (uint32_t(o0.op_type())     ) +
           (uint32_t(o1.op_type()) << 3) +
           (uint32_t(o2.op_type()) << 6);

  // Combine all instruction options and also check whether the instruction is valid. All options
  // that require special handling (including invalid instruction) are handled by the next branch.
  options = InstOptions((inst_id == 0) | ((size_t)(_buffer_end - writer.cursor()) < 16)) | inst_options() | forced_inst_options();

  // Handle failure and rare cases first.
  if (ASMJIT_UNLIKELY(Support::test(options, kRequiresSpecialHandling))) {
    if (ASMJIT_UNLIKELY(!_code)) {
      return report_error(make_error(Error::kNotInitialized));
    }

    // Unknown instruction.
    if (ASMJIT_UNLIKELY(inst_id == 0)) {
      goto InvalidInstruction;
    }

    // Grow request, happens rarely.
    err = writer.ensure_space(this, 16);
    if (ASMJIT_UNLIKELY(err != Error::kOk)) {
      goto Failed;
    }

#ifndef ASMJIT_NO_INTROSPECTION
    // Strict validation.
    if (has_diagnostic_option(DiagnosticOptions::kValidateAssembler)) {
      Operand_ op_array[Globals::kMaxOpCount];
      EmitterUtils::op_array_from_emit_args(op_array, o0, o1, o2, op_ext);

      err = _funcs.validate(BaseInst(inst_id, options, _extra_reg), op_array, Globals::kMaxOpCount, ValidationFlags::kNone);
      if (ASMJIT_UNLIKELY(err != Error::kOk)) {
        goto Failed;
      }
    }
#endif

    InstDB::InstFlags inst_flags = inst_info->flags();

    // LOCK, XACQUIRE, and XRELEASE prefixes.
    if (Support::test(options, InstOptions::kX86_Lock)) {
      bool is_xacq_xrel = Support::test(options, InstOptions::kX86_XAcquire | InstOptions::kX86_XRelease);

      if (ASMJIT_UNLIKELY(!Support::test(inst_flags, InstDB::InstFlags::kLock) && !is_xacq_xrel)) {
        goto InvalidLockPrefix;
      }

      if (is_xacq_xrel) {
        if (ASMJIT_UNLIKELY(Support::test(options, InstOptions::kX86_XAcquire) && !Support::test(inst_flags, InstDB::InstFlags::kXAcquire))) {
          goto InvalidXAcquirePrefix;
        }

        if (ASMJIT_UNLIKELY(Support::test(options, InstOptions::kX86_XRelease) && !Support::test(inst_flags, InstDB::InstFlags::kXRelease))) {
          goto InvalidXReleasePrefix;
        }

        writer.emit8(Support::test(options, InstOptions::kX86_XAcquire) ? 0xF2u : 0xF3u);
      }

      writer.emit8(0xF0);
    }

    // REP and REPNE prefixes.
    if (Support::test(options, InstOptions::kX86_Rep | InstOptions::kX86_Repne)) {
      if (ASMJIT_UNLIKELY(!Support::test(inst_flags, InstDB::InstFlags::kRep))) {
        goto InvalidRepPrefix;
      }

      if (ASMJIT_UNLIKELY(_extra_reg.is_reg() && (_extra_reg.group() != RegGroup::kGp || _extra_reg.id() != Gp::kIdCx))) {
        goto InvalidRepPrefix;
      }

      writer.emit8(Support::test(options, InstOptions::kX86_Repne) ? 0xF2 : 0xF3);
    }
  }

  // This sequence seems to be the fastest.
  opcode = InstDB::main_opcode_table[inst_info->_main_opcode_index];
  op_reg = opcode.extract_mod_o();
  opcode |= inst_info->_main_opcode_value;

  // Encoding Scope
  // --------------

  // How it works? Each case here represents a unique encoding of a group of instructions, which is handled
  // separately. The handlers check instruction signature, possibly register types, etc, and process this
  // information by writing some bits to opcode, op_reg/rb_reg, imm_value/imm_size, etc, and then at the end of
  // the sequence it uses goto to jump into a lower level handler, that actually encodes the instruction.

  switch (inst_info->_encoding) {
    case InstDB::kEncodingNone:
      goto EmitDone;

    // Base Instructions
    // -----------------

    case InstDB::kEncodingX86Op:
      goto EmitX86Op;

    case InstDB::kEncodingX86Op_Mod11RM:
      rb_reg = opcode.extract_mod_rm();
      goto EmitX86R;

    case InstDB::kEncodingX86Op_Mod11RM_I8:
      // The first operand must be immediate, we don't care of other operands as they could be implicit.
      if (!o0.is_imm())
        goto InvalidInstruction;

      rb_reg = opcode.extract_mod_rm();
      imm_value = o0.as<Imm>().value_as<uint8_t>();
      imm_size = 1;
      goto EmitX86R;

    case InstDB::kEncodingX86Op_xAddr:
      if (ASMJIT_UNLIKELY(!o0.is_reg()))
        goto InvalidInstruction;

      rm_info = mem_info_table[size_t(o0.as<Reg>().reg_type())];
      writer.emit_address_override((rm_info & _address_override_mask()) != 0);
      goto EmitX86Op;

    case InstDB::kEncodingX86Op_xAX:
      if (isign3 == 0)
        goto EmitX86Op;

      if (isign3 == ENC_OPS1(Reg) && o0.id() == Gp::kIdAx)
        goto EmitX86Op;
      break;

    case InstDB::kEncodingX86Op_xDX_xAX:
      if (isign3 == 0)
        goto EmitX86Op;

      if (isign3 == ENC_OPS2(Reg, Reg) && o0.id() == Gp::kIdDx && o1.id() == Gp::kIdAx)
        goto EmitX86Op;
      break;

    case InstDB::kEncodingX86Op_MemZAX:
      if (isign3 == 0)
        goto EmitX86Op;

      rm_rel = &o0;
      if (isign3 == ENC_OPS1(Mem) && is_implicit_mem(o0, Gp::kIdAx))
        goto EmitX86OpImplicitMem;

      break;

    case InstDB::kEncodingX86I_xAX:
      // Implicit form.
      if (isign3 == ENC_OPS1(Imm)) {
        imm_value = o0.as<Imm>().value_as<uint8_t>();
        imm_size = 1;
        goto EmitX86Op;
      }

      // Explicit form.
      if (isign3 == ENC_OPS2(Reg, Imm) && o0.id() == Gp::kIdAx) {
        imm_value = o1.as<Imm>().value_as<uint8_t>();
        imm_size = 1;
        goto EmitX86Op;
      }
      break;

    case InstDB::kEncodingX86M_NoMemSize:
      if (o0.is_reg())
        opcode.add_prefix_by_size(o0.x86_rm_size());
      goto CaseX86M_NoSize;

    case InstDB::kEncodingX86M:
      opcode.add_prefix_by_size(o0.x86_rm_size());
      [[fallthrough]];

    case InstDB::kEncodingX86M_NoSize:
CaseX86M_NoSize:
      rb_reg = o0.id();
      if (isign3 == ENC_OPS1(Reg))
        goto EmitX86R;

      rm_rel = &o0;
      if (isign3 == ENC_OPS1(Mem))
        goto EmitX86M;
      break;

    case InstDB::kEncodingX86M_GPB_MulDiv:
CaseX86M_GPB_MulDiv:
      // Explicit form?
      if (isign3 > 0x7) {
        // [AX] <- [AX] div|mul r8.
        if (isign3 == ENC_OPS2(Reg, Reg)) {
          if (ASMJIT_UNLIKELY(!o0.is_gp16(Gp::kIdAx) || !o1.is_gp8()))
            goto InvalidInstruction;

          rb_reg = o1.id();
          FIXUP_GPB(o1, rb_reg);
          goto EmitX86R;
        }

        // [AX] <- [AX] div|mul m8.
        if (isign3 == ENC_OPS2(Reg, Mem)) {
          if (ASMJIT_UNLIKELY(!o0.is_gp16(Gp::kIdAx)))
            goto InvalidInstruction;

          rm_rel = &o1;
          goto EmitX86M;
        }

        // [?DX:?AX] <- [?DX:?AX] div|mul r16|r32|r64
        if (isign3 == ENC_OPS3(Reg, Reg, Reg)) {
          if (ASMJIT_UNLIKELY(o0.x86_rm_size() != o1.x86_rm_size()))
            goto InvalidInstruction;

          opcode.add_arith_by_size(o0.x86_rm_size());
          rb_reg = o2.id();
          goto EmitX86R;
        }

        // [?DX:?AX] <- [?DX:?AX] div|mul m16|m32|m64
        if (isign3 == ENC_OPS3(Reg, Reg, Mem)) {
          if (ASMJIT_UNLIKELY(o0.x86_rm_size() != o1.x86_rm_size()))
            goto InvalidInstruction;

          opcode.add_arith_by_size(o0.x86_rm_size());
          rm_rel = &o2;
          goto EmitX86M;
        }

        goto InvalidInstruction;
      }

      [[fallthrough]];

    case InstDB::kEncodingX86M_GPB:
      if (isign3 == ENC_OPS1(Reg)) {
        opcode.add_arith_by_size(o0.x86_rm_size());
        rb_reg = o0.id();

        if (o0.x86_rm_size() != 1)
          goto EmitX86R;

        FIXUP_GPB(o0, rb_reg);
        goto EmitX86R;
      }

      if (isign3 == ENC_OPS1(Mem)) {
        if (ASMJIT_UNLIKELY(o0.x86_rm_size() == 0))
          goto AmbiguousOperandSize;

        opcode.add_arith_by_size(o0.x86_rm_size());
        rm_rel = &o0;
        goto EmitX86M;
      }
      break;

    case InstDB::kEncodingX86M_Only_EDX_EAX:
      if (isign3 == ENC_OPS3(Mem, Reg, Reg) && o1.is_gp32(Gp::kIdDx) && o2.is_gp32(Gp::kIdAx)) {
        rm_rel = &o0;
        goto EmitX86M;
      }
      [[fallthrough]];

    case InstDB::kEncodingX86M_Only:
      if (isign3 == ENC_OPS1(Mem)) {
        rm_rel = &o0;
        goto EmitX86M;
      }
      break;

    case InstDB::kEncodingX86M_Nop:
      if (isign3 == ENC_OPS1(None))
        goto EmitX86Op;

      // Single operand NOP instruction "0F 1F /0".
      opcode = Opcode::k000F00 | 0x1F;
      op_reg = 0;

      if (isign3 == ENC_OPS1(Reg)) {
        opcode.add_prefix_by_size(o0.x86_rm_size());
        rb_reg = o0.id();
        goto EmitX86R;
      }

      if (isign3 == ENC_OPS1(Mem)) {
        opcode.add_prefix_by_size(o0.x86_rm_size());
        rm_rel = &o0;
        goto EmitX86M;
      }

      // Two operand NOP instruction "0F 1F /r".
      op_reg = o1.id();
      opcode.add_prefix_by_size(o1.x86_rm_size());

      if (isign3 == ENC_OPS2(Reg, Reg)) {
        rb_reg = o0.id();
        goto EmitX86R;
      }

      if (isign3 == ENC_OPS2(Mem, Reg)) {
        rm_rel = &o0;
        goto EmitX86M;
      }
      break;

    case InstDB::kEncodingX86R_FromM:
      if (isign3 == ENC_OPS1(Mem)) {
        rm_rel = &o0;
        rb_reg = o0.id();
        goto EmitX86RFromM;
      }
      break;

    case InstDB::kEncodingX86R32_EDX_EAX:
      // Explicit form: R32, EDX, EAX.
      if (isign3 == ENC_OPS3(Reg, Reg, Reg)) {
        if (!o1.is_gp32(Gp::kIdDx) || !o2.is_gp32(Gp::kIdAx))
          goto InvalidInstruction;
        rb_reg = o0.id();
        goto EmitX86R;
      }

      // Implicit form: R32.
      if (isign3 == ENC_OPS1(Reg)) {
        if (!o0.is_gp32())
          goto InvalidInstruction;
        rb_reg = o0.id();
        goto EmitX86R;
      }
      break;

    case InstDB::kEncodingX86R_Native:
      if (isign3 == ENC_OPS1(Reg)) {
        rb_reg = o0.id();
        goto EmitX86R;
      }
      break;

    case InstDB::kEncodingX86Rm:
      opcode.add_prefix_by_size(o0.x86_rm_size());
      [[fallthrough]];

    case InstDB::kEncodingX86Rm_NoSize:
      if (isign3 == ENC_OPS2(Reg, Reg)) {
        op_reg = o0.id();
        rb_reg = o1.id();
        goto EmitX86R;
      }

      if (isign3 == ENC_OPS2(Reg, Mem)) {
        op_reg = o0.id();
        rm_rel = &o1;
        goto EmitX86M;
      }
      break;

    case InstDB::kEncodingX86Rm_Raw66H:
      // We normally emit either [66|F2|F3], this instruction requires 66+[F2|F3].
      if (isign3 == ENC_OPS2(Reg, Reg)) {
        op_reg = o0.id();
        rb_reg = o1.id();

        if (o0.x86_rm_size() == 2)
          writer.emit8(0x66);
        else
          opcode.add_w_by_size(o0.x86_rm_size());
        goto EmitX86R;
      }

      if (isign3 == ENC_OPS2(Reg, Mem)) {
        op_reg = o0.id();
        rm_rel = &o1;

        if (o0.x86_rm_size() == 2)
          writer.emit8(0x66);
        else
          opcode.add_w_by_size(o0.x86_rm_size());
        goto EmitX86M;
      }
      break;

    case InstDB::kEncodingX86Mr:
      opcode.add_prefix_by_size(o1.x86_rm_size());
      [[fallthrough]];

    case InstDB::kEncodingX86Mr_NoSize:
      if (isign3 == ENC_OPS2(Reg, Reg)) {
        rb_reg = o0.id();
        op_reg = o1.id();
        goto EmitX86R;
      }

      if (isign3 == ENC_OPS2(Mem, Reg)) {
        rm_rel = &o0;
        op_reg = o1.id();
        goto EmitX86M;
      }
      break;

    case InstDB::kEncodingX86Arith:
      if (isign3 == ENC_OPS2(Reg, Reg)) {
        opcode.add_arith_by_size(o0.x86_rm_size());

        if (o0.x86_rm_size() != o1.x86_rm_size())
          goto OperandSizeMismatch;

        rb_reg = o0.id();
        op_reg = o1.id();

        if (o0.x86_rm_size() == 1) {
          FIXUP_GPB(o0, rb_reg);
          FIXUP_GPB(o1, op_reg);
        }

        // MOD/MR: The default encoding used if not instructed otherwise..
        if (!Support::test(options, InstOptions::kX86_ModRM))
          goto EmitX86R;

        // MOD/RM: Alternative encoding selected via instruction options.
        opcode += 2u;
        std::swap(op_reg, rb_reg);
        goto EmitX86R;
      }

      if (isign3 == ENC_OPS2(Reg, Mem)) {
        opcode += 2u;
        opcode.add_arith_by_size(o0.x86_rm_size());

        op_reg = o0.id();
        rm_rel = &o1;

        if (o0.x86_rm_size() != 1)
          goto EmitX86M;

        FIXUP_GPB(o0, op_reg);
        goto EmitX86M;
      }

      if (isign3 == ENC_OPS2(Mem, Reg)) {
        opcode.add_arith_by_size(o1.x86_rm_size());
        op_reg = o1.id();
        rm_rel = &o0;

        if (o1.x86_rm_size() != 1)
          goto EmitX86M;

        FIXUP_GPB(o1, op_reg);
        goto EmitX86M;
      }

      // The remaining instructions use 0x80 opcode.
      opcode = 0x80;

      if (isign3 == ENC_OPS2(Reg, Imm)) {
        uint32_t size = o0.x86_rm_size();

        rb_reg = o0.id();
        imm_value = o1.as<Imm>().value();

        if (size == 1) {
          FIXUP_GPB(o0, rb_reg);
          imm_size = 1;
        }
        else {
          if (size == 2) {
            opcode |= Opcode::kPP_66;
          }
          else if (size == 4) {
            // Sign extend so is_int_n<8> returns the right result.
            imm_value = sign_extend_int32<int64_t>(imm_value);
          }
          else if (size == 8) {
            bool can_transform_to_32bit = inst_id == Inst::kIdAnd && Support::is_uint_n<32>(imm_value);

            if (!Support::is_int_n<32>(imm_value)) {
              // We would do this by default when `kOptionOptimizedForSize` is
              // enabled, however, in this case we just force this as otherwise
              // we would have to fail.
              if (can_transform_to_32bit)
                size = 4;
              else
                goto InvalidImmediate;
            }
            else if (can_transform_to_32bit && has_encoding_option(EncodingOptions::kOptimizeForSize)) {
              size = 4;
            }

            opcode.add_w_by_size(size);
          }

          imm_size = FastUInt8(Support::min<uint32_t>(size, 4));
          if (Support::is_int_n<8>(imm_value) && !Support::test(options, InstOptions::kLongForm))
            imm_size = 1;
        }

        // Short form - AL, AX, EAX, RAX.
        if (rb_reg == 0 && (size == 1 || imm_size != 1) && !Support::test(options, InstOptions::kLongForm)) {
          opcode &= Opcode::kPP_66 | Opcode::kW;
          opcode |= ((op_reg << 3) | (0x04 + (size != 1)));
          imm_size = FastUInt8(Support::min<uint32_t>(size, 4));
          goto EmitX86Op;
        }

        opcode += size != 1 ? (imm_size != 1 ? 1u : 3u) : 0u;
        goto EmitX86R;
      }

      if (isign3 == ENC_OPS2(Mem, Imm)) {
        uint32_t mem_size = o0.x86_rm_size();

        if (ASMJIT_UNLIKELY(mem_size == 0))
          goto AmbiguousOperandSize;

        imm_value = o1.as<Imm>().value();
        imm_size = FastUInt8(Support::min<uint32_t>(mem_size, 4));

        // Sign extend so is_int_n<8> returns the right result.
        if (mem_size == 4)
          imm_value = sign_extend_int32<int64_t>(imm_value);

        if (Support::is_int_n<8>(imm_value) && !Support::test(options, InstOptions::kLongForm))
          imm_size = 1;

        opcode += mem_size != 1 ? (imm_size != 1 ? 1u : 3u) : 0u;
        opcode.add_prefix_by_size(mem_size);

        rm_rel = &o0;
        goto EmitX86M;
      }
      break;

    case InstDB::kEncodingX86Bswap:
      if (isign3 == ENC_OPS1(Reg)) {
        if (ASMJIT_UNLIKELY(o0.x86_rm_size() == 1))
          goto InvalidInstruction;

        op_reg = o0.id();
        opcode.add_prefix_by_size(o0.x86_rm_size());
        goto EmitX86OpReg;
      }
      break;

    case InstDB::kEncodingX86Bt:
      if (isign3 == ENC_OPS2(Reg, Reg)) {
        opcode.add_prefix_by_size(o1.x86_rm_size());
        op_reg = o1.id();
        rb_reg = o0.id();
        goto EmitX86R;
      }

      if (isign3 == ENC_OPS2(Mem, Reg)) {
        opcode.add_prefix_by_size(o1.x86_rm_size());
        op_reg = o1.id();
        rm_rel = &o0;
        goto EmitX86M;
      }

      // The remaining instructions use the secondary opcode/r.
      imm_value = o1.as<Imm>().value();
      imm_size = 1;

      opcode = alt_opcode_of(inst_info);
      opcode.add_prefix_by_size(o0.x86_rm_size());
      op_reg = opcode.extract_mod_o();

      if (isign3 == ENC_OPS2(Reg, Imm)) {
        rb_reg = o0.id();
        goto EmitX86R;
      }

      if (isign3 == ENC_OPS2(Mem, Imm)) {
        if (ASMJIT_UNLIKELY(o0.x86_rm_size() == 0))
          goto AmbiguousOperandSize;

        rm_rel = &o0;
        goto EmitX86M;
      }
      break;

    case InstDB::kEncodingX86Call:
      if (isign3 == ENC_OPS1(Reg)) {
        rb_reg = o0.id();
        goto EmitX86R;
      }

      rm_rel = &o0;
      if (isign3 == ENC_OPS1(Mem))
        goto EmitX86M;

      // Call with 32-bit displacement use 0xE8 opcode. Call with 8-bit displacement is not encodable so the
      // alternative opcode field in X86DB must be zero.
      opcode = 0xE8;
      op_reg = 0;
      goto EmitJmpCall;

    case InstDB::kEncodingX86Cmpxchg: {
      // Convert explicit to implicit.
      if (isign3 & (0x7 << 6)) {
        if (!o2.is_gp() || o2.id() != Gp::kIdAx) {
          goto InvalidInstruction;
        }
        isign3 &= 0x3F;
      }

      if (isign3 == ENC_OPS2(Reg, Reg)) {
        if (o0.x86_rm_size() != o1.x86_rm_size())
          goto OperandSizeMismatch;

        opcode.add_arith_by_size(o0.x86_rm_size());
        rb_reg = o0.id();
        op_reg = o1.id();

        if (o0.x86_rm_size() != 1)
          goto EmitX86R;

        FIXUP_GPB(o0, rb_reg);
        FIXUP_GPB(o1, op_reg);
        goto EmitX86R;
      }

      if (isign3 == ENC_OPS2(Mem, Reg)) {
        opcode.add_arith_by_size(o1.x86_rm_size());
        op_reg = o1.id();
        rm_rel = &o0;

        if (o1.x86_rm_size() != 1)
          goto EmitX86M;

        FIXUP_GPB(o1, op_reg);
        goto EmitX86M;
      }
      break;
    }

    case InstDB::kEncodingX86Cmpxchg8b_16b: {
      const Operand_& o3 = op_ext[EmitterUtils::kOp3];
      const Operand_& o4 = op_ext[EmitterUtils::kOp4];

      if (isign3 == ENC_OPS3(Mem, Reg, Reg)) {
        if (o3.is_reg() && o4.is_reg()) {
          rm_rel = &o0;
          goto EmitX86M;
        }
      }

      if (isign3 == ENC_OPS1(Mem)) {
        rm_rel = &o0;
        goto EmitX86M;
      }
      break;
    }

    case InstDB::kEncodingX86Crc:
      op_reg = o0.id();
      opcode.add_w_by_size(o0.x86_rm_size());

      if (isign3 == ENC_OPS2(Reg, Reg)) {
        rb_reg = o1.id();

        if (o1.x86_rm_size() == 1) {
          FIXUP_GPB(o1, rb_reg);
          goto EmitX86R;
        }
        else {
          // This seems to be the only exception of encoding '66F2' prefix.
          if (o1.x86_rm_size() == 2) writer.emit8(0x66);

          opcode.add(1);
          goto EmitX86R;
        }
      }

      if (isign3 == ENC_OPS2(Reg, Mem)) {
        rm_rel = &o1;
        if (o1.x86_rm_size() == 0)
          goto AmbiguousOperandSize;

        // This seems to be the only exception of encoding '66F2' prefix.
        if (o1.x86_rm_size() == 2) writer.emit8(0x66);

        opcode += uint32_t(o1.x86_rm_size() != 1u);
        goto EmitX86M;
      }
      break;

    case InstDB::kEncodingX86Enter:
      if (isign3 == ENC_OPS2(Imm, Imm)) {
        uint32_t iw = o0.as<Imm>().value_as<uint16_t>();
        uint32_t ib = o1.as<Imm>().value_as<uint8_t>();

        imm_value = iw | (ib << 16);
        imm_size = 3;
        goto EmitX86Op;
      }
      break;

    case InstDB::kEncodingX86Imul:
      // First process all forms distinct of `kEncodingX86M_OptB_MulDiv`.
      if (isign3 == ENC_OPS3(Reg, Reg, Imm)) {
        opcode = 0x6B;
        opcode.add_prefix_by_size(o0.x86_rm_size());

        imm_value = o2.as<Imm>().value();
        imm_size = 1;

        if (!Support::is_int_n<8>(imm_value) || Support::test(options, InstOptions::kLongForm)) {
          opcode -= 2;
          imm_size = o0.x86_rm_size() == 2 ? 2 : 4;
        }

        op_reg = o0.id();
        rb_reg = o1.id();

        goto EmitX86R;
      }

      if (isign3 == ENC_OPS3(Reg, Mem, Imm)) {
        opcode = 0x6B;
        opcode.add_prefix_by_size(o0.x86_rm_size());

        imm_value = o2.as<Imm>().value();
        imm_size = 1;

        // Sign extend so is_int_n<8> returns the right result.
        if (o0.x86_rm_size() == 4)
          imm_value = sign_extend_int32<int64_t>(imm_value);

        if (!Support::is_int_n<8>(imm_value) || Support::test(options, InstOptions::kLongForm)) {
          opcode -= 2;
          imm_size = o0.x86_rm_size() == 2 ? 2 : 4;
        }

        op_reg = o0.id();
        rm_rel = &o1;

        goto EmitX86M;
      }

      if (isign3 == ENC_OPS2(Reg, Reg)) {
        // Must be explicit 'ax, r8' form.
        if (o1.x86_rm_size() == 1)
          goto CaseX86M_GPB_MulDiv;

        if (o0.x86_rm_size() != o1.x86_rm_size())
          goto OperandSizeMismatch;

        op_reg = o0.id();
        rb_reg = o1.id();

        opcode = Opcode::k000F00 | 0xAF;
        opcode.add_prefix_by_size(o0.x86_rm_size());
        goto EmitX86R;
      }

      if (isign3 == ENC_OPS2(Reg, Mem)) {
        // Must be explicit 'ax, m8' form.
        if (o1.x86_rm_size() == 1)
          goto CaseX86M_GPB_MulDiv;

        op_reg = o0.id();
        rm_rel = &o1;

        opcode = Opcode::k000F00 | 0xAF;
        opcode.add_prefix_by_size(o0.x86_rm_size());
        goto EmitX86M;
      }

      // Shorthand to imul 'reg, reg, imm'.
      if (isign3 == ENC_OPS2(Reg, Imm)) {
        opcode = 0x6B;
        opcode.add_prefix_by_size(o0.x86_rm_size());

        imm_value = o1.as<Imm>().value();
        imm_size = 1;

        // Sign extend so is_int_n<8> returns the right result.
        if (o0.x86_rm_size() == 4)
          imm_value = sign_extend_int32<int64_t>(imm_value);

        if (!Support::is_int_n<8>(imm_value) || Support::test(options, InstOptions::kLongForm)) {
          opcode -= 2;
          imm_size = o0.x86_rm_size() == 2 ? 2 : 4;
        }

        op_reg = rb_reg = o0.id();
        goto EmitX86R;
      }

      // Try implicit form.
      goto CaseX86M_GPB_MulDiv;

    case InstDB::kEncodingX86In:
      if (isign3 == ENC_OPS2(Reg, Imm)) {
        if (ASMJIT_UNLIKELY(o0.id() != Gp::kIdAx))
          goto InvalidInstruction;

        imm_value = o1.as<Imm>().value_as<uint8_t>();
        imm_size = 1;

        opcode = alt_opcode_of(inst_info) + (o0.x86_rm_size() != 1);
        opcode.add_66h_by_size(o0.x86_rm_size());
        goto EmitX86Op;
      }

      if (isign3 == ENC_OPS2(Reg, Reg)) {
        if (ASMJIT_UNLIKELY(o0.id() != Gp::kIdAx || o1.id() != Gp::kIdDx))
          goto InvalidInstruction;

        opcode += uint32_t(o0.x86_rm_size() != 1u);
        opcode.add_66h_by_size(o0.x86_rm_size());
        goto EmitX86Op;
      }
      break;

    case InstDB::kEncodingX86Ins:
      if (isign3 == ENC_OPS2(Mem, Reg)) {
        if (ASMJIT_UNLIKELY(!is_implicit_mem(o0, Gp::kIdDi) || o1.id() != Gp::kIdDx))
          goto InvalidInstruction;

        uint32_t size = o0.x86_rm_size();
        if (ASMJIT_UNLIKELY(size == 0))
          goto AmbiguousOperandSize;

        rm_rel = &o0;
        opcode += uint32_t(size != 1u);

        opcode.add_66h_by_size(size);
        goto EmitX86OpImplicitMem;
      }
      break;

    case InstDB::kEncodingX86IncDec:
      if (isign3 == ENC_OPS1(Reg)) {
        rb_reg = o0.id();

        if (o0.x86_rm_size() == 1) {
          FIXUP_GPB(o0, rb_reg);
          goto EmitX86R;
        }

        if (is_32bit()) {
          // INC r16|r32 is only encodable in 32-bit mode (collides with REX).
          opcode = alt_opcode_of(inst_info) + (rb_reg & 0x07);
          opcode.add_66h_by_size(o0.x86_rm_size());
          goto EmitX86Op;
        }
        else {
          opcode.add_arith_by_size(o0.x86_rm_size());
          goto EmitX86R;
        }
      }

      if (isign3 == ENC_OPS1(Mem)) {
        if (!o0.x86_rm_size())
          goto AmbiguousOperandSize;
        opcode.add_arith_by_size(o0.x86_rm_size());
        rm_rel = &o0;
        goto EmitX86M;
      }
      break;

    case InstDB::kEncodingX86Int:
      if (isign3 == ENC_OPS1(Imm)) {
        imm_value = o0.as<Imm>().value();
        imm_size = 1;
        goto EmitX86Op;
      }
      break;

    case InstDB::kEncodingX86Jcc:
      if (Support::test(options, InstOptions::kTaken | InstOptions::kNotTaken) && has_encoding_option(EncodingOptions::kPredictedJumps)) {
        uint8_t prefix = Support::test(options, InstOptions::kTaken) ? uint8_t(0x3E) : uint8_t(0x2E);
        writer.emit8(prefix);
      }

      rm_rel = &o0;
      op_reg = 0;
      goto EmitJmpCall;

    case InstDB::kEncodingX86JecxzLoop:
      rm_rel = &o0;
      // Explicit jecxz|loop [r|e]cx, dst
      if (o0.is_reg()) {
        if (ASMJIT_UNLIKELY(!o0.is_gp(Gp::kIdCx))) {
          goto InvalidInstruction;
        }

        writer.emit_address_override((is_32bit() && o0.x86_rm_size() == 2) || (is_64bit() && o0.x86_rm_size() == 4));
        rm_rel = &o1;
      }

      op_reg = 0;
      goto EmitJmpCall;

    case InstDB::kEncodingX86Jmp:
      if (isign3 == ENC_OPS1(Reg)) {
        rb_reg = o0.id();
        goto EmitX86R;
      }

      rm_rel = &o0;
      if (isign3 == ENC_OPS1(Mem))
        goto EmitX86M;

      // Jump encoded with 32-bit displacement use 0xE9 opcode. Jump encoded with 8-bit displacement's opcode is
      // stored as an alternative opcode.
      opcode = 0xE9;
      op_reg = 0;
      goto EmitJmpCall;

    case InstDB::kEncodingX86JmpRel:
      rm_rel = &o0;
      goto EmitJmpCall;

    case InstDB::kEncodingX86LcallLjmp:
      if (isign3 == ENC_OPS1(Mem)) {
        rm_rel = &o0;
        uint32_t mem_size = rm_rel->as<Mem>().size();
        if (mem_size == 0) {
          mem_size = register_size();
        }
        else {
          mem_size -= 2;
          if (mem_size != 2 && mem_size != 4 && mem_size != register_size())
            goto InvalidAddress;
        }
        opcode.add_prefix_by_size(mem_size);
        goto EmitX86M;
      }

      if (isign3 == ENC_OPS2(Imm, Imm)) {
        if (!is_32bit())
          goto InvalidInstruction;

        const Imm& imm0 = o0.as<Imm>();
        const Imm& imm1 = o1.as<Imm>();

        if (imm0.value() > 0xFFFFu || imm1.value() > 0xFFFFFFFFu)
          goto InvalidImmediate;

        opcode = alt_opcode_of(inst_info);
        imm_value = imm1.value() | (imm0.value() << 32);
        imm_size = 6;
        goto EmitX86Op;
      }
      break;

    case InstDB::kEncodingX86Lea:
      if (isign3 == ENC_OPS2(Reg, Mem)) {
        opcode.add_prefix_by_size(o0.x86_rm_size());
        op_reg = o0.id();
        rm_rel = &o1;
        goto EmitX86M;
      }
      break;

    case InstDB::kEncodingX86Mov:
      // Reg <- Reg
      if (isign3 == ENC_OPS2(Reg, Reg)) {
        // Asmjit uses segment registers indexed from 1 to 6, leaving zero as "no segment register used". We have to
        // fix this (decrement the index of the register) when emitting MOV instructions which move to/from a segment
        // register. The segment register is always `op_reg`, because the MOV instruction uses either RM or MR encoding.

        // GP <- ??
        if (o0.as<Reg>().is_gp()) {
          rb_reg = o0.id();
          op_reg = o1.id();

          // GP <- GP
          if (o1.as<Reg>().is_gp()) {
            uint32_t op_size = o0.x86_rm_size();
            if (op_size != o1.x86_rm_size())
              goto InvalidInstruction;

            if (op_size == 1) {
              FIXUP_GPB(o0, rb_reg);
              FIXUP_GPB(o1, op_reg);
              opcode = 0x88;

              if (!Support::test(options, InstOptions::kX86_ModRM))
                goto EmitX86R;

              opcode += 2u;
              std::swap(op_reg, rb_reg);
              goto EmitX86R;
            }
            else {
              opcode = 0x89;
              opcode.add_prefix_by_size(op_size);

              if (!Support::test(options, InstOptions::kX86_ModRM))
                goto EmitX86R;

              opcode += 2u;
              std::swap(op_reg, rb_reg);
              goto EmitX86R;
            }
          }

          // GP <- SReg
          if (o1.is_segment_reg()) {
            opcode = 0x8C;
            opcode.add_prefix_by_size(o0.x86_rm_size());
            op_reg--;
            goto EmitX86R;
          }

          // GP <- CReg
          if (o1.is_control_reg()) {
            opcode = Opcode::k000F00 | 0x20;

            // Use `LOCK MOV` in 32-bit mode if CR8+ register is accessed (AMD extension).
            if ((op_reg & 0x8) && is_32bit()) {
              writer.emit8(0xF0);
              op_reg &= 0x7;
            }
            goto EmitX86R;
          }

          // GP <- DReg
          if (o1.is_debug_reg()) {
            opcode = Opcode::k000F00 | 0x21;
            goto EmitX86R;
          }
        }
        else {
          op_reg = o0.id();
          rb_reg = o1.id();

          // ?? <- GP
          if (!o1.as<Reg>().is_gp())
            goto InvalidInstruction;

          // SReg <- GP
          if (o0.is_segment_reg()) {
            opcode = 0x8E;
            opcode.add_prefix_by_size(o1.x86_rm_size());
            op_reg--;
            goto EmitX86R;
          }

          // CReg <- GP
          if (o0.is_control_reg()) {
            opcode = Opcode::k000F00 | 0x22;

            // Use `LOCK MOV` in 32-bit mode if CR8+ register is accessed (AMD extension).
            if ((op_reg & 0x8) && is_32bit()) {
              writer.emit8(0xF0);
              op_reg &= 0x7;
            }
            goto EmitX86R;
          }

          // DReg <- GP
          if (o0.is_debug_reg()) {
            opcode = Opcode::k000F00 | 0x23;
            goto EmitX86R;
          }
        }

        goto InvalidInstruction;
      }

      if (isign3 == ENC_OPS2(Reg, Mem)) {
        op_reg = o0.id();
        rm_rel = &o1;

        // SReg <- Mem
        if (o0.is_segment_reg()) {
          opcode = 0x8E;
          opcode.add_prefix_by_size(o1.x86_rm_size());
          op_reg--;
          goto EmitX86M;
        }
        // Reg <- Mem
        else {
          opcode = 0;
          opcode.add_arith_by_size(o0.x86_rm_size());

          // Handle a special form of `mov al|ax|eax|rax, [ptr64]` that doesn't use MOD.
          if (op_reg == Gp::kIdAx && !rm_rel->as<Mem>().has_base_or_index()) {
            if (x86_should_use_movabs(this, writer, o0.x86_rm_size(), options, rm_rel->as<Mem>())) {
              opcode += 0xA0u;
              imm_value = rm_rel->as<Mem>().offset();
              goto EmitX86OpMovAbs;
            }
          }

          if (o0.x86_rm_size() == 1)
            FIXUP_GPB(o0, op_reg);

          opcode += 0x8Au;
          goto EmitX86M;
        }
      }

      if (isign3 == ENC_OPS2(Mem, Reg)) {
        op_reg = o1.id();
        rm_rel = &o0;

        // Mem <- SReg
        if (o1.is_segment_reg()) {
          opcode = 0x8C;
          opcode.add_prefix_by_size(o0.x86_rm_size());
          op_reg--;
          goto EmitX86M;
        }
        // Mem <- Reg
        else {
          opcode = 0;
          opcode.add_arith_by_size(o1.x86_rm_size());

          // Handle a special form of `mov [ptr64], al|ax|eax|rax` that doesn't use MOD.
          if (op_reg == Gp::kIdAx && !rm_rel->as<Mem>().has_base_or_index()) {
            if (x86_should_use_movabs(this, writer, o1.x86_rm_size(), options, rm_rel->as<Mem>())) {
              opcode += 0xA2u;
              imm_value = rm_rel->as<Mem>().offset();
              goto EmitX86OpMovAbs;
            }
          }

          if (o1.x86_rm_size() == 1)
            FIXUP_GPB(o1, op_reg);

          opcode += 0x88u;
          goto EmitX86M;
        }
      }

      if (isign3 == ENC_OPS2(Reg, Imm)) {
        op_reg = o0.id();
        imm_size = FastUInt8(o0.x86_rm_size());

        if (imm_size == 1) {
          FIXUP_GPB(o0, op_reg);

          opcode = 0xB0;
          imm_value = o1.as<Imm>().value_as<uint8_t>();
          goto EmitX86OpReg;
        }
        else {
          // 64-bit immediate in 64-bit mode is allowed.
          imm_value = o1.as<Imm>().value();

          // Optimize the instruction size by using a 32-bit immediate if possible.
          if (imm_size == 8 && !Support::test(options, InstOptions::kLongForm)) {
            if (Support::is_uint_n<32>(imm_value) && has_encoding_option(EncodingOptions::kOptimizeForSize)) {
              // Zero-extend by using a 32-bit GPD destination instead of a 64-bit GPQ.
              imm_size = 4;
            }
            else if (Support::is_int_n<32>(imm_value)) {
              // Sign-extend, uses 'C7 /0' opcode.
              rb_reg = op_reg;

              opcode = Opcode::kW | 0xC7;
              op_reg = 0;

              imm_size = 4;
              goto EmitX86R;
            }
          }

          opcode = 0xB8;
          opcode.add_prefix_by_size(imm_size);
          goto EmitX86OpReg;
        }
      }

      if (isign3 == ENC_OPS2(Mem, Imm)) {
        uint32_t mem_size = o0.x86_rm_size();
        if (ASMJIT_UNLIKELY(mem_size == 0))
          goto AmbiguousOperandSize;

        opcode = 0xC6 + (mem_size != 1);
        opcode.add_prefix_by_size(mem_size);
        op_reg = 0;
        rm_rel = &o0;

        imm_value = o1.as<Imm>().value();
        imm_size = FastUInt8(Support::min<uint32_t>(mem_size, 4));
        goto EmitX86M;
      }
      break;

    case InstDB::kEncodingX86Movabs:
      // Reg <- Mem
      if (isign3 == ENC_OPS2(Reg, Mem)) {
        op_reg = o0.id();
        rm_rel = &o1;

        opcode = 0xA0;
        opcode.add_arith_by_size(o0.x86_rm_size());

        if (ASMJIT_UNLIKELY(!o0.as<Reg>().is_gp()) || op_reg != Gp::kIdAx)
          goto InvalidInstruction;

        if (ASMJIT_UNLIKELY(rm_rel->as<Mem>().has_base_or_index()))
          goto InvalidAddress;

        if (ASMJIT_UNLIKELY(rm_rel->as<Mem>().addr_type() == Mem::AddrType::kRel))
          goto InvalidAddress;

        imm_value = rm_rel->as<Mem>().offset();
        goto EmitX86OpMovAbs;
      }

      // Mem <- Reg
      if (isign3 == ENC_OPS2(Mem, Reg)) {
        op_reg = o1.id();
        rm_rel = &o0;

        opcode = 0xA2;
        opcode.add_arith_by_size(o1.x86_rm_size());

        if (ASMJIT_UNLIKELY(!o1.as<Reg>().is_gp()) || op_reg != Gp::kIdAx)
          goto InvalidInstruction;

        if (ASMJIT_UNLIKELY(rm_rel->as<Mem>().has_base_or_index()))
          goto InvalidAddress;

        imm_value = rm_rel->as<Mem>().offset();
        goto EmitX86OpMovAbs;
      }

      // Reg <- Imm.
      if (isign3 == ENC_OPS2(Reg, Imm)) {
        if (ASMJIT_UNLIKELY(!o0.is_gp64()))
          goto InvalidInstruction;

        op_reg = o0.id();
        opcode = 0xB8;

        imm_size = 8;
        imm_value = o1.as<Imm>().value();

        opcode.add_prefix_by_size(8);
        goto EmitX86OpReg;
      }
      break;

    case InstDB::kEncodingX86MovsxMovzx:
      opcode.add(o1.x86_rm_size() != 1);
      opcode.add_prefix_by_size(o0.x86_rm_size());

      if (isign3 == ENC_OPS2(Reg, Reg)) {
        op_reg = o0.id();
        rb_reg = o1.id();

        if (o1.x86_rm_size() != 1)
          goto EmitX86R;

        FIXUP_GPB(o1, rb_reg);
        goto EmitX86R;
      }

      if (isign3 == ENC_OPS2(Reg, Mem)) {
        op_reg = o0.id();
        rm_rel = &o1;
        goto EmitX86M;
      }
      break;

    case InstDB::kEncodingX86MovntiMovdiri:
      if (isign3 == ENC_OPS2(Mem, Reg)) {
        opcode.add_w_if(o1.is_gp64());

        op_reg = o1.id();
        rm_rel = &o0;
        goto EmitX86M;
      }
      break;

    case InstDB::kEncodingX86EnqcmdMovdir64b:
      if (isign3 == ENC_OPS2(Mem, Mem)) {
        const Mem& m0 = o0.as<Mem>();
        // This is the only required validation, the rest is handled afterwards.
        if (ASMJIT_UNLIKELY(m0.base_type() != o1.as<Mem>().base_type() ||
                            m0.has_index() ||
                            m0.has_offset() ||
                            (m0.has_segment() && m0.segment_id() != SReg::kIdEs)))
          goto InvalidInstruction;

        // The first memory operand is passed via register, the second memory operand is RM.
        op_reg = o0.as<Mem>().base_id();
        rm_rel = &o1;
        goto EmitX86M;
      }
      break;

    case InstDB::kEncodingX86Out:
      if (isign3 == ENC_OPS2(Imm, Reg)) {
        if (ASMJIT_UNLIKELY(o1.id() != Gp::kIdAx))
          goto InvalidInstruction;

        opcode = alt_opcode_of(inst_info) + (o1.x86_rm_size() != 1);
        opcode.add_66h_by_size(o1.x86_rm_size());

        imm_value = o0.as<Imm>().value_as<uint8_t>();
        imm_size = 1;
        goto EmitX86Op;
      }

      if (isign3 == ENC_OPS2(Reg, Reg)) {
        if (ASMJIT_UNLIKELY(o0.id() != Gp::kIdDx || o1.id() != Gp::kIdAx))
          goto InvalidInstruction;

        opcode.add(o1.x86_rm_size() != 1);
        opcode.add_66h_by_size(o1.x86_rm_size());
        goto EmitX86Op;
      }
      break;

    case InstDB::kEncodingX86Outs:
      if (isign3 == ENC_OPS2(Reg, Mem)) {
        if (ASMJIT_UNLIKELY(o0.id() != Gp::kIdDx || !is_implicit_mem(o1, Gp::kIdSi)))
          goto InvalidInstruction;

        uint32_t size = o1.x86_rm_size();
        if (ASMJIT_UNLIKELY(size == 0))
          goto AmbiguousOperandSize;

        rm_rel = &o1;
        opcode.add(size != 1);
        opcode.add_66h_by_size(size);
        goto EmitX86OpImplicitMem;
      }
      break;

    case InstDB::kEncodingX86Pushw:
      if (isign3 == ENC_OPS1(Imm)) {
        imm_value = o0.as<Imm>().value();
        imm_size = 2;
        opcode = 0x68u | Opcode::kPP_66;
        goto EmitX86Op;
      }
      break;

    case InstDB::kEncodingX86Push:
      if (isign3 == ENC_OPS1(Reg)) {
        if (o0.is_segment_reg()) {
          uint32_t segment = o0.id();
          if (ASMJIT_UNLIKELY(segment >= SReg::kIdCount))
            goto InvalidSegment;

          opcode = opcode_push_sreg_table[segment];
          goto EmitX86Op;
        }
        else {
          goto CaseX86PushPop_Gp;
        }
      }

      if (isign3 == ENC_OPS1(Imm)) {
        imm_value = o0.as<Imm>().value();
        imm_size = 4;

        if (Support::is_int_n<8>(imm_value) && !Support::test(options, InstOptions::kLongForm))
          imm_size = 1;

        opcode = imm_size == 1 ? 0x6A : 0x68;
        goto EmitX86Op;
      }
      [[fallthrough]];

    case InstDB::kEncodingX86Pop:
      if (isign3 == ENC_OPS1(Reg)) {
        if (o0.is_segment_reg()) {
          uint32_t segment = o0.id();
          if (ASMJIT_UNLIKELY(segment == SReg::kIdCs || segment >= SReg::kIdCount))
            goto InvalidSegment;

          opcode = opcode_pop_sreg_table[segment];
          goto EmitX86Op;
        }
        else {
CaseX86PushPop_Gp:
          // We allow 2 byte, 4 byte, and 8 byte register sizes, although PUSH and POP only allow 2 bytes or
          // native size. On 64-bit we simply PUSH/POP 64-bit register even if 32-bit register was given.
          if (ASMJIT_UNLIKELY(o0.x86_rm_size() < 2))
            goto InvalidInstruction;

          opcode = alt_opcode_of(inst_info);
          opcode.add_66h_by_size(o0.x86_rm_size());
          op_reg = o0.id();
          goto EmitX86OpReg;
        }
      }

      if (isign3 == ENC_OPS1(Mem)) {
        if (ASMJIT_UNLIKELY(o0.x86_rm_size() == 0))
          goto AmbiguousOperandSize;

        if (ASMJIT_UNLIKELY(o0.x86_rm_size() != 2 && o0.x86_rm_size() != register_size()))
          goto InvalidInstruction;

        opcode.add_66h_by_size(o0.x86_rm_size());
        rm_rel = &o0;
        goto EmitX86M;
      }
      break;

    case InstDB::kEncodingX86Ret:
      if (isign3 == 0) {
        // 'ret' without immediate, change C2 to C3.
        opcode.add(1);
        goto EmitX86Op;
      }

      if (isign3 == ENC_OPS1(Imm)) {
        imm_value = o0.as<Imm>().value();
        if (imm_value == 0 && !Support::test(options, InstOptions::kLongForm)) {
          // 'ret' without immediate, change C2 to C3.
          opcode.add(1);
          goto EmitX86Op;
        }
        else {
          imm_size = 2;
          goto EmitX86Op;
        }
      }
      break;

    case InstDB::kEncodingX86Rot:
      if (o0.is_reg()) {
        opcode.add_arith_by_size(o0.x86_rm_size());
        rb_reg = o0.id();

        if (o0.x86_rm_size() == 1)
          FIXUP_GPB(o0, rb_reg);

        if (isign3 == ENC_OPS2(Reg, Reg)) {
          if (ASMJIT_UNLIKELY(o1.id() != Gp::kIdCx))
            goto InvalidInstruction;

          opcode += 2u;
          goto EmitX86R;
        }

        if (isign3 == ENC_OPS2(Reg, Imm)) {
          imm_value = o1.as<Imm>().value() & 0xFF;
          imm_size = 0;

          if (imm_value == 1 && !Support::test(options, InstOptions::kLongForm))
            goto EmitX86R;

          opcode -= 0x10;
          imm_size = 1;
          goto EmitX86R;
        }
      }
      else {
        if (ASMJIT_UNLIKELY(o0.x86_rm_size() == 0))
          goto AmbiguousOperandSize;
        opcode.add_arith_by_size(o0.x86_rm_size());

        if (isign3 == ENC_OPS2(Mem, Reg)) {
          if (ASMJIT_UNLIKELY(o1.id() != Gp::kIdCx))
            goto InvalidInstruction;

          opcode += 2u;
          rm_rel = &o0;
          goto EmitX86M;
        }

        if (isign3 == ENC_OPS2(Mem, Imm)) {
          rm_rel = &o0;
          imm_value = o1.as<Imm>().value() & 0xFF;
          imm_size = 0;

          if (imm_value == 1 && !Support::test(options, InstOptions::kLongForm))
            goto EmitX86M;

          opcode -= 0x10;
          imm_size = 1;
          goto EmitX86M;
        }
      }
      break;

    case InstDB::kEncodingX86Set:
      if (isign3 == ENC_OPS1(Reg)) {
        rb_reg = o0.id();
        FIXUP_GPB(o0, rb_reg);
        goto EmitX86R;
      }

      if (isign3 == ENC_OPS1(Mem)) {
        rm_rel = &o0;
        goto EmitX86M;
      }
      break;

    case InstDB::kEncodingX86ShldShrd:
      if (isign3 == ENC_OPS3(Reg, Reg, Imm)) {
        opcode.add_prefix_by_size(o0.x86_rm_size());
        op_reg = o1.id();
        rb_reg = o0.id();

        imm_value = o2.as<Imm>().value();
        imm_size = 1;
        goto EmitX86R;
      }

      if (isign3 == ENC_OPS3(Mem, Reg, Imm)) {
        opcode.add_prefix_by_size(o1.x86_rm_size());
        op_reg = o1.id();
        rm_rel = &o0;

        imm_value = o2.as<Imm>().value();
        imm_size = 1;
        goto EmitX86M;
      }

      // The following instructions use opcode + 1.
      opcode.add(1);

      if (isign3 == ENC_OPS3(Reg, Reg, Reg)) {
        if (ASMJIT_UNLIKELY(o2.id() != Gp::kIdCx))
          goto InvalidInstruction;

        opcode.add_prefix_by_size(o0.x86_rm_size());
        op_reg = o1.id();
        rb_reg = o0.id();
        goto EmitX86R;
      }

      if (isign3 == ENC_OPS3(Mem, Reg, Reg)) {
        if (ASMJIT_UNLIKELY(o2.id() != Gp::kIdCx))
          goto InvalidInstruction;

        opcode.add_prefix_by_size(o1.x86_rm_size());
        op_reg = o1.id();
        rm_rel = &o0;
        goto EmitX86M;
      }
      break;

    case InstDB::kEncodingX86StrRm:
      if (isign3 == ENC_OPS2(Reg, Mem)) {
        rm_rel = &o1;
        if (ASMJIT_UNLIKELY(rm_rel->as<Mem>().offset_lo32() || !o0.as<Reg>().is_gp(Gp::kIdAx)))
          goto InvalidInstruction;

        uint32_t size = o0.x86_rm_size();
        if (o1.x86_rm_size() != 0u && ASMJIT_UNLIKELY(o1.x86_rm_size() != size))
          goto OperandSizeMismatch;

        opcode.add_arith_by_size(size);
        goto EmitX86OpImplicitMem;
      }
      break;

    case InstDB::kEncodingX86StrMr:
      if (isign3 == ENC_OPS2(Mem, Reg)) {
        rm_rel = &o0;
        if (ASMJIT_UNLIKELY(rm_rel->as<Mem>().offset_lo32() || !o1.is_gp(Gp::kIdAx)))
          goto InvalidInstruction;

        uint32_t size = o1.x86_rm_size();
        if (o0.x86_rm_size() != 0u && ASMJIT_UNLIKELY(o0.x86_rm_size() != size))
          goto OperandSizeMismatch;

        opcode.add_arith_by_size(size);
        goto EmitX86OpImplicitMem;
      }
      break;

    case InstDB::kEncodingX86StrMm:
      if (isign3 == ENC_OPS2(Mem, Mem)) {
        if (ASMJIT_UNLIKELY(o0.as<Mem>().base_and_index_types() !=
                            o1.as<Mem>().base_and_index_types()))
          goto InvalidInstruction;

        rm_rel = &o1;
        if (ASMJIT_UNLIKELY(o0.as<Mem>().has_offset()))
          goto InvalidInstruction;

        uint32_t size = o1.x86_rm_size();
        if (ASMJIT_UNLIKELY(size == 0))
          goto AmbiguousOperandSize;

        if (ASMJIT_UNLIKELY(o0.x86_rm_size() != size))
          goto OperandSizeMismatch;

        opcode.add_arith_by_size(size);
        goto EmitX86OpImplicitMem;
      }
      break;

    case InstDB::kEncodingX86Test:
      if (isign3 == ENC_OPS2(Reg, Reg)) {
        if (o0.x86_rm_size() != o1.x86_rm_size())
          goto OperandSizeMismatch;

        opcode.add_arith_by_size(o0.x86_rm_size());
        rb_reg = o0.id();
        op_reg = o1.id();

        if (o0.x86_rm_size() != 1)
          goto EmitX86R;

        FIXUP_GPB(o0, rb_reg);
        FIXUP_GPB(o1, op_reg);
        goto EmitX86R;
      }

      if (isign3 == ENC_OPS2(Mem, Reg)) {
        opcode.add_arith_by_size(o1.x86_rm_size());
        op_reg = o1.id();
        rm_rel = &o0;

        if (o1.x86_rm_size() != 1)
          goto EmitX86M;

        FIXUP_GPB(o1, op_reg);
        goto EmitX86M;
      }

      // The following instructions use the secondary opcode.
      opcode = alt_opcode_of(inst_info);
      op_reg = opcode.extract_mod_o();

      if (isign3 == ENC_OPS2(Reg, Imm)) {
        opcode.add_arith_by_size(o0.x86_rm_size());
        rb_reg = o0.id();

        if (o0.x86_rm_size() == 1) {
          FIXUP_GPB(o0, rb_reg);
          imm_value = o1.as<Imm>().value_as<uint8_t>();
          imm_size = 1;
        }
        else {
          imm_value = o1.as<Imm>().value();
          imm_size = FastUInt8(Support::min<uint32_t>(o0.x86_rm_size(), 4));
        }

        // Short form - AL, AX, EAX, RAX.
        if (rb_reg == 0 && !Support::test(options, InstOptions::kLongForm)) {
          opcode &= Opcode::kPP_66 | Opcode::kW;
          opcode |= 0xA8 + (o0.x86_rm_size() != 1);
          goto EmitX86Op;
        }

        goto EmitX86R;
      }

      if (isign3 == ENC_OPS2(Mem, Imm)) {
        if (ASMJIT_UNLIKELY(o0.x86_rm_size() == 0))
          goto AmbiguousOperandSize;

        opcode.add_arith_by_size(o0.x86_rm_size());
        rm_rel = &o0;

        imm_value = o1.as<Imm>().value();
        imm_size = FastUInt8(Support::min<uint32_t>(o0.x86_rm_size(), 4));
        goto EmitX86M;
      }
      break;

    case InstDB::kEncodingX86Xchg:
      if (isign3 == ENC_OPS2(Reg, Mem)) {
        opcode.add_arith_by_size(o0.x86_rm_size());
        op_reg = o0.id();
        rm_rel = &o1;

        if (o0.x86_rm_size() != 1)
          goto EmitX86M;

        FIXUP_GPB(o0, op_reg);
        goto EmitX86M;
      }
      [[fallthrough]];

    case InstDB::kEncodingX86Xadd:
      if (isign3 == ENC_OPS2(Reg, Reg)) {
        rb_reg = o0.id();
        op_reg = o1.id();

        uint32_t op_size = o0.x86_rm_size();
        if (op_size != o1.x86_rm_size())
          goto OperandSizeMismatch;

        if (op_size == 1) {
          FIXUP_GPB(o0, rb_reg);
          FIXUP_GPB(o1, op_reg);
          goto EmitX86R;
        }

        // Special cases for 'xchg ?ax, reg'.
        if (inst_id == Inst::kIdXchg && (op_reg == 0 || rb_reg == 0)) {
          if (is_64bit() && op_reg == rb_reg && op_size >= 4) {
            if (op_size == 8) {
              // Encode 'xchg rax, rax' as '90' (REX and other prefixes are optional).
              opcode &= Opcode::kW;
              opcode |= 0x90;
              goto EmitX86OpReg;
            }
            else {
              // Encode 'xchg eax, eax' by using a generic path.
            }
          }
          else if (!Support::test(options, InstOptions::kLongForm)) {
            // The special encoding encodes only one register, which is non-zero.
            op_reg += rb_reg;

            opcode.add_arith_by_size(op_size);
            opcode &= Opcode::kW | Opcode::kPP_66;
            opcode |= 0x90;
            goto EmitX86OpReg;
          }
        }

        opcode.add_arith_by_size(op_size);
        goto EmitX86R;
      }

      if (isign3 == ENC_OPS2(Mem, Reg)) {
        opcode.add_arith_by_size(o1.x86_rm_size());
        op_reg = o1.id();
        rm_rel = &o0;

        if (o1.x86_rm_size() == 1) {
          FIXUP_GPB(o1, op_reg);
        }

        goto EmitX86M;
      }
      break;

    case InstDB::kEncodingX86Fence:
      rb_reg = 0;
      goto EmitX86R;

    case InstDB::kEncodingX86Bndmov:
      if (isign3 == ENC_OPS2(Reg, Reg)) {
        op_reg = o0.id();
        rb_reg = o1.id();

        // ModRM encoding:
        if (!Support::test(options, InstOptions::kX86_ModMR))
          goto EmitX86R;

        // ModMR encoding:
        opcode = alt_opcode_of(inst_info);
        std::swap(op_reg, rb_reg);
        goto EmitX86R;
      }

      if (isign3 == ENC_OPS2(Reg, Mem)) {
        op_reg = o0.id();
        rm_rel = &o1;
        goto EmitX86M;
      }

      if (isign3 == ENC_OPS2(Mem, Reg)) {
        opcode = alt_opcode_of(inst_info);

        rm_rel = &o0;
        op_reg = o1.id();
        goto EmitX86M;
      }
      break;

    // FPU Instructions
    // ----------------

    case InstDB::kEncodingFpuOp:
      goto EmitFpuOp;

    case InstDB::kEncodingFpuArith:
      if (isign3 == ENC_OPS2(Reg, Reg)) {
        op_reg = o0.id();
        rb_reg = o1.id();

        // We switch to the alternative opcode if the first operand is zero.
        if (op_reg == 0) {
CaseFpuArith_Reg:
          opcode = ((0xD8   << Opcode::kFPU_2B_Shift)       ) +
                   ((opcode >> Opcode::kFPU_2B_Shift) & 0xFF) + rb_reg;
          goto EmitFpuOp;
        }
        else if (rb_reg == 0) {
          rb_reg = op_reg;
          opcode = ((0xDC   << Opcode::kFPU_2B_Shift)       ) +
                   ((opcode                         ) & 0xFF) + rb_reg;
          goto EmitFpuOp;
        }
        else {
          goto InvalidInstruction;
        }
      }

      if (isign3 == ENC_OPS1(Mem)) {
CaseFpuArith_Mem:
        // 0xD8/0xDC, depends on the size of the memory operand; op_reg is valid.
        opcode = (o0.x86_rm_size() == 4) ? 0xD8 : 0xDC;
        // Clear compressed displacement before going to EmitX86M.
        opcode &= ~uint32_t(Opcode::kCDSHL_Mask);

        rm_rel = &o0;
        goto EmitX86M;
      }
      break;

    case InstDB::kEncodingFpuCom:
      if (isign3 == 0) {
        rb_reg = 1;
        goto CaseFpuArith_Reg;
      }

      if (isign3 == ENC_OPS1(Reg)) {
        rb_reg = o0.id();
        goto CaseFpuArith_Reg;
      }

      if (isign3 == ENC_OPS1(Mem)) {
        goto CaseFpuArith_Mem;
      }
      break;

    case InstDB::kEncodingFpuFldFst:
      if (isign3 == ENC_OPS1(Mem)) {
        rm_rel = &o0;

        if (o0.x86_rm_size() == 4 && common_info->has_flag(InstDB::InstFlags::kFpuM32)) {
          goto EmitX86M;
        }

        if (o0.x86_rm_size() == 8 && common_info->has_flag(InstDB::InstFlags::kFpuM64)) {
          opcode += 4u;
          goto EmitX86M;
        }

        if (o0.x86_rm_size() == 10 && common_info->has_flag(InstDB::InstFlags::kFpuM80)) {
          opcode = alt_opcode_of(inst_info);
          op_reg  = opcode.extract_mod_o();
          goto EmitX86M;
        }
      }

      if (isign3 == ENC_OPS1(Reg)) {
        if (inst_id == Inst::kIdFld ) { opcode = (0xD9 << Opcode::kFPU_2B_Shift) + 0xC0 + o0.id(); goto EmitFpuOp; }
        if (inst_id == Inst::kIdFst ) { opcode = (0xDD << Opcode::kFPU_2B_Shift) + 0xD0 + o0.id(); goto EmitFpuOp; }
        if (inst_id == Inst::kIdFstp) { opcode = (0xDD << Opcode::kFPU_2B_Shift) + 0xD8 + o0.id(); goto EmitFpuOp; }
      }
      break;

    case InstDB::kEncodingFpuM:
      if (isign3 == ENC_OPS1(Mem)) {
        // Clear compressed displacement before going to EmitX86M.
        opcode &= ~uint32_t(Opcode::kCDSHL_Mask);

        rm_rel = &o0;
        if (o0.x86_rm_size() == 2 && common_info->has_flag(InstDB::InstFlags::kFpuM16)) {
          opcode += 4u;
          goto EmitX86M;
        }

        if (o0.x86_rm_size() == 4 && common_info->has_flag(InstDB::InstFlags::kFpuM32)) {
          goto EmitX86M;
        }

        if (o0.x86_rm_size() == 8 && common_info->has_flag(InstDB::InstFlags::kFpuM64)) {
          opcode = alt_opcode_of(inst_info) & ~uint32_t(Opcode::kCDSHL_Mask);
          op_reg  = opcode.extract_mod_o();
          goto EmitX86M;
        }
      }
      break;

    case InstDB::kEncodingFpuRDef:
      if (isign3 == 0) {
        opcode += 1u;
        goto EmitFpuOp;
      }
      [[fallthrough]];

    case InstDB::kEncodingFpuR:
      if (isign3 == ENC_OPS1(Reg)) {
        opcode += o0.id();
        goto EmitFpuOp;
      }
      break;

    case InstDB::kEncodingFpuStsw:
      if (isign3 == ENC_OPS1(Reg)) {
        if (ASMJIT_UNLIKELY(o0.id() != Gp::kIdAx))
          goto InvalidInstruction;

        opcode = alt_opcode_of(inst_info);
        goto EmitFpuOp;
      }

      if (isign3 == ENC_OPS1(Mem)) {
        // Clear compressed displacement before going to EmitX86M.
        opcode &= ~uint32_t(Opcode::kCDSHL_Mask);

        rm_rel = &o0;
        goto EmitX86M;
      }
      break;

    // Ext Instructions (Legacy Extensions)
    // ------------------------------------

    case InstDB::kEncodingExtPextrw:
      if (isign3 == ENC_OPS3(Reg, Reg, Imm)) {
        opcode.add_66h_if(o1.is_vec128());

        imm_value = o2.as<Imm>().value();
        imm_size = 1;

        op_reg = o0.id();
        rb_reg = o1.id();
        goto EmitX86R;
      }

      if (isign3 == ENC_OPS3(Mem, Reg, Imm)) {
        // Secondary opcode of 'pextrw' instruction (SSE4.1).
        opcode = alt_opcode_of(inst_info);
        opcode.add_66h_if(o1.is_vec128());

        imm_value = o2.as<Imm>().value();
        imm_size = 1;

        op_reg = o1.id();
        rm_rel = &o0;
        goto EmitX86M;
      }
      break;

    case InstDB::kEncodingExtExtract:
      if (isign3 == ENC_OPS3(Reg, Reg, Imm)) {
        opcode.add_66h_if(o1.is_vec128());

        imm_value = o2.as<Imm>().value();
        imm_size = 1;

        op_reg = o1.id();
        rb_reg = o0.id();
        goto EmitX86R;
      }

      if (isign3 == ENC_OPS3(Mem, Reg, Imm)) {
        opcode.add_66h_if(o1.is_vec128());

        imm_value = o2.as<Imm>().value();
        imm_size = 1;

        op_reg = o1.id();
        rm_rel = &o0;
        goto EmitX86M;
      }
      break;

    case InstDB::kEncodingExtMov:
      // GP|MM|XMM <- GP|MM|XMM
      if (isign3 == ENC_OPS2(Reg, Reg)) {
        op_reg = o0.id();
        rb_reg = o1.id();

        if (!Support::test(options, InstOptions::kX86_ModMR) || !inst_info->_alt_opcode_index)
          goto EmitX86R;

        opcode = alt_opcode_of(inst_info);
        std::swap(op_reg, rb_reg);
        goto EmitX86R;
      }

      // GP|MM|XMM <- Mem
      if (isign3 == ENC_OPS2(Reg, Mem)) {
        op_reg = o0.id();
        rm_rel = &o1;
        goto EmitX86M;
      }

      // The following instruction uses opcode[1].
      opcode = alt_opcode_of(inst_info);

      // Mem <- GP|MM|XMM
      if (isign3 == ENC_OPS2(Mem, Reg)) {
        op_reg = o1.id();
        rm_rel = &o0;
        goto EmitX86M;
      }
      break;

    case InstDB::kEncodingExtMovbe:
      if (isign3 == ENC_OPS2(Reg, Mem)) {
        if (o0.x86_rm_size() == 1)
          goto InvalidInstruction;

        opcode.add_prefix_by_size(o0.x86_rm_size());
        op_reg = o0.id();
        rm_rel = &o1;
        goto EmitX86M;
      }

      // The following instruction uses the secondary opcode.
      opcode = alt_opcode_of(inst_info);

      if (isign3 == ENC_OPS2(Mem, Reg)) {
        if (o1.x86_rm_size() == 1)
          goto InvalidInstruction;

        opcode.add_prefix_by_size(o1.x86_rm_size());
        op_reg = o1.id();
        rm_rel = &o0;
        goto EmitX86M;
      }
      break;

    case InstDB::kEncodingExtMovd:
CaseExtMovd:
      if (is_mmx_or_xmm(o0.as<Reg>())) {
        op_reg = o0.id();
        opcode.add_66h_if(o0.is_vec128());

        // MM/XMM <- Gp
        if (isign3 == ENC_OPS2(Reg, Reg) && o1.as<Reg>().is_gp()) {
          rb_reg = o1.id();
          goto EmitX86R;
        }

        // MM/XMM <- Mem
        if (isign3 == ENC_OPS2(Reg, Mem)) {
          rm_rel = &o1;
          goto EmitX86M;
        }
      }

      // The following instructions use the secondary opcode.
      if (is_mmx_or_xmm(o1.as<Reg>())) {
        opcode &= Opcode::kW;
        opcode |= alt_opcode_of(inst_info);
        op_reg = o1.id();
        opcode.add_66h_if(o1.is_vec128());

        // GP <- MM/XMM
        if (isign3 == ENC_OPS2(Reg, Reg) && o0.as<Reg>().is_gp()) {
          rb_reg = o0.id();
          goto EmitX86R;
        }

        // Mem <- MM/XMM
        if (isign3 == ENC_OPS2(Mem, Reg)) {
          rm_rel = &o0;
          goto EmitX86M;
        }
      }
      break;

    case InstDB::kEncodingExtMovq:
      if (isign3 == ENC_OPS2(Reg, Reg)) {
        op_reg = o0.id();
        rb_reg = o1.id();

        // MM <- MM
        if (o0.is_mm_reg() && o1.is_mm_reg()) {
          opcode = Opcode::k000F00 | 0x6F;

          if (!Support::test(options, InstOptions::kX86_ModMR))
            goto EmitX86R;

          opcode += 0x10u;
          std::swap(op_reg, rb_reg);
          goto EmitX86R;
        }

        // XMM <- XMM
        if (o0.is_vec128() && o1.is_vec128()) {
          opcode = Opcode::kF30F00 | 0x7E;

          if (!Support::test(options, InstOptions::kX86_ModMR))
            goto EmitX86R;

          opcode = Opcode::k660F00 | 0xD6;
          std::swap(op_reg, rb_reg);
          goto EmitX86R;
        }
      }

      if (isign3 == ENC_OPS2(Reg, Mem)) {
        op_reg = o0.id();
        rm_rel = &o1;

        // MM <- Mem
        if (o0.is_mm_reg()) {
          opcode = Opcode::k000F00 | 0x6F;
          goto EmitX86M;
        }

        // XMM <- Mem
        if (o0.is_vec128()) {
          opcode = Opcode::kF30F00 | 0x7E;
          goto EmitX86M;
        }
      }

      if (isign3 == ENC_OPS2(Mem, Reg)) {
        op_reg = o1.id();
        rm_rel = &o0;

        // Mem <- MM
        if (o1.is_mm_reg()) {
          opcode = Opcode::k000F00 | 0x7F;
          goto EmitX86M;
        }

        // Mem <- XMM
        if (o1.is_vec128()) {
          opcode = Opcode::k660F00 | 0xD6;
          goto EmitX86M;
        }
      }

      // MOVQ in other case is simply a MOVD instruction promoted to 64-bit.
      opcode |= Opcode::kW;
      goto CaseExtMovd;

    case InstDB::kEncodingExtRm_XMM0:
      if (ASMJIT_UNLIKELY(!o2.is_none() && !o2.is_vec128(0)))
        goto InvalidInstruction;

      isign3 &= 0x3F;
      goto CaseExtRm;

    case InstDB::kEncodingExtRm_ZDI:
      if (ASMJIT_UNLIKELY(!o2.is_none() && !is_implicit_mem(o2, Gp::kIdDi)))
        goto InvalidInstruction;

      isign3 &= 0x3F;
      goto CaseExtRm;

    case InstDB::kEncodingExtRm_Wx:
      opcode.add_w_if(o1.x86_rm_size() == 8);
      [[fallthrough]];

    case InstDB::kEncodingExtRm_Wx_GpqOnly:
      opcode.add_w_if(o0.is_gp64());
      [[fallthrough]];

    case InstDB::kEncodingExtRm:
CaseExtRm:
      if (isign3 == ENC_OPS2(Reg, Reg)) {
        op_reg = o0.id();
        rb_reg = o1.id();
        goto EmitX86R;
      }

      if (isign3 == ENC_OPS2(Reg, Mem)) {
        op_reg = o0.id();
        rm_rel = &o1;
        goto EmitX86M;
      }
      break;

    case InstDB::kEncodingExtRm_P:
      if (isign3 == ENC_OPS2(Reg, Reg)) {
        opcode.add_66h_if(Support::bool_or(o0.is_vec128(), o1.is_vec128()));

        op_reg = o0.id();
        rb_reg = o1.id();
        goto EmitX86R;
      }

      if (isign3 == ENC_OPS2(Reg, Mem)) {
        opcode.add_66h_if(o0.is_vec128());

        op_reg = o0.id();
        rm_rel = &o1;
        goto EmitX86M;
      }
      break;

    case InstDB::kEncodingExtRmRi:
      if (isign3 == ENC_OPS2(Reg, Reg)) {
        op_reg = o0.id();
        rb_reg = o1.id();
        goto EmitX86R;
      }

      if (isign3 == ENC_OPS2(Reg, Mem)) {
        op_reg = o0.id();
        rm_rel = &o1;
        goto EmitX86M;
      }

      // The following instruction uses the secondary opcode.
      opcode = alt_opcode_of(inst_info);
      op_reg  = opcode.extract_mod_o();

      if (isign3 == ENC_OPS2(Reg, Imm)) {
        imm_value = o1.as<Imm>().value();
        imm_size = 1;

        rb_reg = o0.id();
        goto EmitX86R;
      }
      break;

    case InstDB::kEncodingExtRmRi_P:
      if (isign3 == ENC_OPS2(Reg, Reg)) {
        opcode.add_66h_if(Support::bool_or(o0.is_vec128(), o1.is_vec128()));

        op_reg = o0.id();
        rb_reg = o1.id();
        goto EmitX86R;
      }

      if (isign3 == ENC_OPS2(Reg, Mem)) {
        opcode.add_66h_if(o0.is_vec128());

        op_reg = o0.id();
        rm_rel = &o1;
        goto EmitX86M;
      }

      // The following instruction uses the secondary opcode.
      opcode = alt_opcode_of(inst_info);
      op_reg  = opcode.extract_mod_o();

      if (isign3 == ENC_OPS2(Reg, Imm)) {
        opcode.add_66h_if(o0.is_vec128());

        imm_value = o1.as<Imm>().value();
        imm_size = 1;

        rb_reg = o0.id();
        goto EmitX86R;
      }
      break;

    case InstDB::kEncodingExtRmi:
      imm_value = o2.as<Imm>().value();
      imm_size = 1;

      if (isign3 == ENC_OPS3(Reg, Reg, Imm)) {
        op_reg = o0.id();
        rb_reg = o1.id();
        goto EmitX86R;
      }

      if (isign3 == ENC_OPS3(Reg, Mem, Imm)) {
        op_reg = o0.id();
        rm_rel = &o1;
        goto EmitX86M;
      }
      break;

    case InstDB::kEncodingExtRmi_P:
      imm_value = o2.as<Imm>().value();
      imm_size = 1;

      if (isign3 == ENC_OPS3(Reg, Reg, Imm)) {
        opcode.add_66h_if(Support::bool_or(o0.is_vec128(), o1.is_vec128()));

        op_reg = o0.id();
        rb_reg = o1.id();
        goto EmitX86R;
      }

      if (isign3 == ENC_OPS3(Reg, Mem, Imm)) {
        opcode.add_66h_if(o0.is_vec128());

        op_reg = o0.id();
        rm_rel = &o1;
        goto EmitX86M;
      }
      break;

    // Extrq & Insertq (SSE4A)
    // -----------------------

    case InstDB::kEncodingExtExtrq:
      op_reg = o0.id();
      rb_reg = o1.id();

      if (isign3 == ENC_OPS2(Reg, Reg))
        goto EmitX86R;

      if (isign3 == ENC_OPS3(Reg, Imm, Imm)) {
        // This variant of the instruction uses the secondary opcode.
        opcode = alt_opcode_of(inst_info);
        rb_reg = op_reg;
        op_reg = opcode.extract_mod_o();

        imm_value = (uint32_t(o1.as<Imm>().value_as<uint8_t>())     ) +
                   (uint32_t(o2.as<Imm>().value_as<uint8_t>()) << 8) ;
        imm_size = 2;
        goto EmitX86R;
      }
      break;

    case InstDB::kEncodingExtInsertq: {
      const Operand_& o3 = op_ext[EmitterUtils::kOp3];
      const uint32_t isign4 = isign3 + (uint32_t(o3.op_type()) << 9);

      op_reg = o0.id();
      rb_reg = o1.id();

      if (isign4 == ENC_OPS2(Reg, Reg))
        goto EmitX86R;

      if (isign4 == ENC_OPS4(Reg, Reg, Imm, Imm)) {
        // This variant of the instruction uses the secondary opcode.
        opcode = alt_opcode_of(inst_info);

        imm_value = (uint32_t(o2.as<Imm>().value_as<uint8_t>())     ) +
                   (uint32_t(o3.as<Imm>().value_as<uint8_t>()) << 8) ;
        imm_size = 2;
        goto EmitX86R;
      }
      break;
    }

    // 3DNOW Instructions
    // ------------------

    case InstDB::kEncodingExt3dNow:
      // Every 3dNow instruction starts with 0x0F0F and the actual opcode is
      // stored as 8-bit immediate.
      imm_value = opcode.v & 0xFFu;
      imm_size = 1;

      opcode = Opcode::k000F00 | 0x0F;
      op_reg = o0.id();

      if (isign3 == ENC_OPS2(Reg, Reg)) {
        rb_reg = o1.id();
        goto EmitX86R;
      }

      if (isign3 == ENC_OPS2(Reg, Mem)) {
        rm_rel = &o1;
        goto EmitX86M;
      }
      break;

    // VEX/EVEX Instructions
    // ---------------------

    case InstDB::kEncodingVexOp:
      goto EmitVexOp;

    case InstDB::kEncodingVexOpMod:
      rb_reg = 0;
      goto EmitVexEvexR;

    case InstDB::kEncodingVexKmov:
      if (isign3 == ENC_OPS2(Reg, Reg)) {
        op_reg = o0.id();
        rb_reg = o1.id();

        // Form 'k, reg'.
        if (o1.as<Reg>().is_gp()) {
          opcode = alt_opcode_of(inst_info);
          goto EmitVexEvexR;
        }

        // Form 'reg, k'.
        if (o0.as<Reg>().is_gp()) {
          opcode = alt_opcode_of(inst_info) + 1;
          goto EmitVexEvexR;
        }

        // Form 'k, k'.
        if (!Support::test(options, InstOptions::kX86_ModMR))
          goto EmitVexEvexR;

        opcode.add(1);
        std::swap(op_reg, rb_reg);
        goto EmitVexEvexR;
      }

      if (isign3 == ENC_OPS2(Reg, Mem)) {
        op_reg = o0.id();
        rm_rel = &o1;

        goto EmitVexEvexM;
      }

      if (isign3 == ENC_OPS2(Mem, Reg)) {
        opcode.add(1);
        op_reg = o1.id();
        rm_rel = &o0;
        goto EmitVexEvexM;
      }
      break;

    case InstDB::kEncodingVexR_Wx:
      if (isign3 == ENC_OPS1(Reg)) {
        rb_reg = o0.id();
        opcode.add_w_if(o0.is_gp64());
        goto EmitVexEvexR;
      }
      break;

    case InstDB::kEncodingVexM:
      if (isign3 == ENC_OPS1(Mem)) {
        rm_rel = &o0;
        goto EmitVexEvexM;
      }
      break;

    case InstDB::kEncodingVexMr_Lx:
      opcode |= opcode_l_by_size(o0.x86_rm_size() | o1.x86_rm_size());

      if (isign3 == ENC_OPS2(Reg, Reg)) {
        op_reg = o1.id();
        rb_reg = o0.id();
        goto EmitVexEvexR;
      }

      if (isign3 == ENC_OPS2(Mem, Reg)) {
        op_reg = o1.id();
        rm_rel = &o0;
        goto EmitVexEvexM;
      }
      break;

    case InstDB::kEncodingVexMr_VM:
      if (isign3 == ENC_OPS2(Mem, Reg)) {
        opcode |= Support::max(opcode_l_by_vmem(o0), opcode_l_by_size(o1.x86_rm_size()));

        op_reg = o1.id();
        rm_rel = &o0;
        goto EmitVexEvexM;
      }
      break;

    case InstDB::kEncodingVexMri_Vpextrw:
      // Use 'vpextrw reg, xmm1, i8' when possible.
      if (isign3 == ENC_OPS3(Reg, Reg, Imm)) {
        opcode = Opcode::k660F00 | 0xC5;

        op_reg = o0.id();
        rb_reg = o1.id();

        imm_value = o2.as<Imm>().value();
        imm_size = 1;
        goto EmitVexEvexR;
      }

      goto CaseVexMri;

    case InstDB::kEncodingVexMvr_Wx:
      if (isign3 == ENC_OPS3(Mem, Reg, Reg)) {
        opcode.add_w_if(o1.is_gp64());
        op_reg = pack_reg_and_vvvvv(o1.id(), o2.id());
        rm_rel = &o0;
        goto EmitVexEvexM;
      }
      break;

    case InstDB::kEncodingVexMri_Lx:
      opcode |= opcode_l_by_size(o0.x86_rm_size() | o1.x86_rm_size());
      [[fallthrough]];

    case InstDB::kEncodingVexMri:
CaseVexMri:
      imm_value = o2.as<Imm>().value();
      imm_size = 1;

      if (isign3 == ENC_OPS3(Reg, Reg, Imm)) {
        op_reg = o1.id();
        rb_reg = o0.id();
        goto EmitVexEvexR;
      }

      if (isign3 == ENC_OPS3(Mem, Reg, Imm)) {
        op_reg = o1.id();
        rm_rel = &o0;
        goto EmitVexEvexM;
      }
      break;

    case InstDB::kEncodingVexRm_ZDI:
      if (ASMJIT_UNLIKELY(!o2.is_none() && !is_implicit_mem(o2, Gp::kIdDi)))
        goto InvalidInstruction;

      isign3 &= 0x3F;
      goto CaseVexRm;

    case InstDB::kEncodingVexRm_Wx:
      opcode.add_w_if(Support::bool_or(o0.is_gp64(), o1.is_gp64()));
      goto CaseVexRm;

    case InstDB::kEncodingVexRm_Lx_Narrow:
      if (o1.x86_rm_size())
        opcode |= opcode_l_by_size(o1.x86_rm_size());
      else if (o0.x86_rm_size() == 32)
        opcode |= Opcode::kLL_2;
      goto CaseVexRm;

    case InstDB::kEncodingVexRm_Lx_Bcst:
      if (isign3 == ENC_OPS2(Reg, Reg) && o1.as<Reg>().is_gp()) {
        opcode = alt_opcode_of(inst_info) | opcode_l_by_size(o0.x86_rm_size() | o1.x86_rm_size());
        op_reg = o0.id();
        rb_reg = o1.id();
        goto EmitVexEvexR;
      }
      [[fallthrough]];

    case InstDB::kEncodingVexRm_Lx:
      opcode |= opcode_l_by_size(o0.x86_rm_size() | o1.x86_rm_size());
      [[fallthrough]];

    case InstDB::kEncodingVexRm:
CaseVexRm:
      if (isign3 == ENC_OPS2(Reg, Reg)) {
        op_reg = o0.id();
        rb_reg = o1.id();
        goto EmitVexEvexR;
      }

      if (isign3 == ENC_OPS2(Reg, Mem)) {
        op_reg = o0.id();
        rm_rel = &o1;
        goto EmitVexEvexM;
      }
      break;

    case InstDB::kEncodingVexRm_VM:
      if (isign3 == ENC_OPS2(Reg, Mem)) {
        opcode |= Support::max(opcode_l_by_vmem(o1), opcode_l_by_size(o0.x86_rm_size()));
        op_reg = o0.id();
        rm_rel = &o1;
        goto EmitVexEvexM;
      }
      break;

    case InstDB::kEncodingVexRmi_Wx:
      opcode.add_w_if(Support::bool_or(o0.is_gp64(), o1.is_gp64()));
      goto CaseVexRmi;

    case InstDB::kEncodingVexRmi_Lx:
      opcode |= opcode_l_by_size(o0.x86_rm_size() | o1.x86_rm_size());
      [[fallthrough]];

    case InstDB::kEncodingVexRmi:
CaseVexRmi:
      imm_value = o2.as<Imm>().value();
      imm_size = 1;

      if (isign3 == ENC_OPS3(Reg, Reg, Imm)) {
        op_reg = o0.id();
        rb_reg = o1.id();
        goto EmitVexEvexR;
      }

      if (isign3 == ENC_OPS3(Reg, Mem, Imm)) {
        op_reg = o0.id();
        rm_rel = &o1;
        goto EmitVexEvexM;
      }
      break;

    case InstDB::kEncodingVexRvm:
CaseVexRvm:
      if (isign3 == ENC_OPS3(Reg, Reg, Reg)) {
CaseVexRvm_R:
        op_reg = pack_reg_and_vvvvv(o0.id(), o1.id());
        rb_reg = o2.id();
        goto EmitVexEvexR;
      }

      if (isign3 == ENC_OPS3(Reg, Reg, Mem)) {
        op_reg = pack_reg_and_vvvvv(o0.id(), o1.id());
        rm_rel = &o2;
        goto EmitVexEvexM;
      }
      break;

    case InstDB::kEncodingVexRvm_ZDX_Wx: {
      const Operand_& o3 = op_ext[EmitterUtils::kOp3];
      if (ASMJIT_UNLIKELY(!o3.is_none() && !o3.is_gp(Gp::kIdDx)))
        goto InvalidInstruction;
      [[fallthrough]];
    }

    case InstDB::kEncodingVexRvm_Wx: {
      opcode.add_w_if(Support::bool_or(o0.is_gp64(), o2.x86_rm_size() == 8));
      goto CaseVexRvm;
    }

    case InstDB::kEncodingVexRvm_Lx_KEvex: {
      opcode.force_evex_if(o0.is_mask_reg());
      [[fallthrough]];
    }

    case InstDB::kEncodingVexRvm_Lx: {
      opcode |= opcode_l_by_size(o0.x86_rm_size() | o1.x86_rm_size());
      goto CaseVexRvm;
    }

    case InstDB::kEncodingVexRvm_Lx_2xK: {
      if (isign3 == ENC_OPS3(Reg, Reg, Reg)) {
        // Two registers are encoded as a single register.
        //   - First K register must be even.
        //   - Second K register must be first+1.
        if ((o0.id() & 1) != 0 || o0.id() + 1 != o1.id())
          goto InvalidPhysId;

        const Operand_& o3 = op_ext[EmitterUtils::kOp3];

        opcode |= opcode_l_by_size(o2.x86_rm_size());
        op_reg = pack_reg_and_vvvvv(o0.id(), o2.id());

        if (o3.is_reg()) {
          rb_reg = o3.id();
          goto EmitVexEvexR;
        }

        if (o3.is_mem()) {
          rm_rel = &o3;
          goto EmitVexEvexM;
        }
      }
      break;
    }

    case InstDB::kEncodingVexRvmr_Lx: {
      opcode |= opcode_l_by_size(o0.x86_rm_size() | o1.x86_rm_size());
      [[fallthrough]];
    }

    case InstDB::kEncodingVexRvmr: {
      const Operand_& o3 = op_ext[EmitterUtils::kOp3];
      const uint32_t isign4 = isign3 + (uint32_t(o3.op_type()) << 9);

      imm_value = o3.id() << 4;
      imm_size = 1;

      if (isign4 == ENC_OPS4(Reg, Reg, Reg, Reg)) {
        op_reg = pack_reg_and_vvvvv(o0.id(), o1.id());
        rb_reg = o2.id();
        goto EmitVexEvexR;
      }

      if (isign4 == ENC_OPS4(Reg, Reg, Mem, Reg)) {
        op_reg = pack_reg_and_vvvvv(o0.id(), o1.id());
        rm_rel = &o2;
        goto EmitVexEvexM;
      }
      break;
    }

    case InstDB::kEncodingVexRvmi_KEvex:
      opcode.force_evex_if(o0.is_mask_reg());
      goto VexRvmi;

    case InstDB::kEncodingVexRvmi_Lx_KEvex:
      opcode.force_evex_if(o0.is_mask_reg());
      [[fallthrough]];

    case InstDB::kEncodingVexRvmi_Lx:
      opcode |= opcode_l_by_size(o0.x86_rm_size() | o1.x86_rm_size());
      [[fallthrough]];

    case InstDB::kEncodingVexRvmi:
VexRvmi:
    {
      const Operand_& o3 = op_ext[EmitterUtils::kOp3];
      const uint32_t isign4 = isign3 + (uint32_t(o3.op_type()) << 9);

      imm_value = o3.as<Imm>().value();
      imm_size = 1;

      if (isign4 == ENC_OPS4(Reg, Reg, Reg, Imm)) {
        op_reg = pack_reg_and_vvvvv(o0.id(), o1.id());
        rb_reg = o2.id();
        goto EmitVexEvexR;
      }

      if (isign4 == ENC_OPS4(Reg, Reg, Mem, Imm)) {
        op_reg = pack_reg_and_vvvvv(o0.id(), o1.id());
        rm_rel = &o2;
        goto EmitVexEvexM;
      }
      break;
    }

    case InstDB::kEncodingVexRmv_Wx:
      opcode.add_w_if(Support::bool_or(o0.is_gp64(), o2.is_gp64()));
      [[fallthrough]];

    case InstDB::kEncodingVexRmv:
      if (isign3 == ENC_OPS3(Reg, Reg, Reg)) {
        op_reg = pack_reg_and_vvvvv(o0.id(), o2.id());
        rb_reg = o1.id();
        goto EmitVexEvexR;
      }

      if (isign3 == ENC_OPS3(Reg, Mem, Reg)) {
        op_reg = pack_reg_and_vvvvv(o0.id(), o2.id());
        rm_rel = &o1;
        goto EmitVexEvexM;
      }
      break;

    case InstDB::kEncodingVexRmvRm_VM:
      if (isign3 == ENC_OPS2(Reg, Mem)) {
        opcode  = alt_opcode_of(inst_info);
        opcode |= Support::max(opcode_l_by_vmem(o1), opcode_l_by_size(o0.x86_rm_size()));

        op_reg = o0.id();
        rm_rel = &o1;
        goto EmitVexEvexM;
      }
      [[fallthrough]];

    case InstDB::kEncodingVexRmv_VM:
      if (isign3 == ENC_OPS3(Reg, Mem, Reg)) {
        opcode |= Support::max(opcode_l_by_vmem(o1), opcode_l_by_size(o0.x86_rm_size() | o2.x86_rm_size()));

        op_reg = pack_reg_and_vvvvv(o0.id(), o2.id());
        rm_rel = &o1;
        goto EmitVexEvexM;
      }
      break;


    case InstDB::kEncodingVexRmvi: {
      const Operand_& o3 = op_ext[EmitterUtils::kOp3];
      const uint32_t isign4 = isign3 + (uint32_t(o3.op_type()) << 9);

      imm_value = o3.as<Imm>().value();
      imm_size = 1;

      if (isign4 == ENC_OPS4(Reg, Reg, Reg, Imm)) {
        op_reg = pack_reg_and_vvvvv(o0.id(), o2.id());
        rb_reg = o1.id();
        goto EmitVexEvexR;
      }

      if (isign4 == ENC_OPS4(Reg, Mem, Reg, Imm)) {
        op_reg = pack_reg_and_vvvvv(o0.id(), o2.id());
        rm_rel = &o1;
        goto EmitVexEvexM;
      }
      break;
    }

    case InstDB::kEncodingVexMovdMovq:
      if (isign3 == ENC_OPS2(Reg, Reg)) {
        if (o0.as<Reg>().is_gp()) {
          opcode = alt_opcode_of(inst_info);
          opcode.add_w_by_size(o0.x86_rm_size());
          op_reg = o1.id();
          rb_reg = o0.id();
          goto EmitVexEvexR;
        }

        if (o1.as<Reg>().is_gp()) {
          opcode.add_w_by_size(o1.x86_rm_size());
          op_reg = o0.id();
          rb_reg = o1.id();
          goto EmitVexEvexR;
        }

        // If this is a 'W' version (movq) then allow also vmovq 'xmm|xmm' form.
        if (opcode & Opcode::kEvex_W_1) {
          opcode &= ~(Opcode::kPP_VEXMask | Opcode::kMM_Mask | 0xFF);
          opcode |=  (Opcode::kF30F00 | 0x7E);

          op_reg = o0.id();
          rb_reg = o1.id();
          goto EmitVexEvexR;
        }
      }

      if (isign3 == ENC_OPS2(Reg, Mem)) {
        if (opcode & Opcode::kEvex_W_1) {
          opcode &= ~(Opcode::kPP_VEXMask | Opcode::kMM_Mask | 0xFF);
          opcode |=  (Opcode::kF30F00 | 0x7E);
        }

        op_reg = o0.id();
        rm_rel = &o1;
        goto EmitVexEvexM;
      }

      // The following instruction uses the secondary opcode.
      opcode = alt_opcode_of(inst_info);

      if (isign3 == ENC_OPS2(Mem, Reg)) {
        if (opcode & Opcode::kEvex_W_1) {
          opcode &= ~(Opcode::kPP_VEXMask | Opcode::kMM_Mask | 0xFF);
          opcode |=  (Opcode::k660F00 | 0xD6);
        }

        op_reg = o1.id();
        rm_rel = &o0;
        goto EmitVexEvexM;
      }
      break;

    case InstDB::kEncodingVexRmMr_Lx:
      opcode |= opcode_l_by_size(o0.x86_rm_size() | o1.x86_rm_size());
      [[fallthrough]];

    case InstDB::kEncodingVexRmMr:
      if (isign3 == ENC_OPS2(Reg, Reg)) {
        op_reg = o0.id();
        rb_reg = o1.id();
        goto EmitVexEvexR;
      }

      if (isign3 == ENC_OPS2(Reg, Mem)) {
        op_reg = o0.id();
        rm_rel = &o1;
        goto EmitVexEvexM;
      }

      // The following instruction uses the secondary opcode.
      opcode &= Opcode::kLL_Mask;
      opcode |= alt_opcode_of(inst_info);

      if (isign3 == ENC_OPS2(Mem, Reg)) {
        op_reg = o1.id();
        rm_rel = &o0;
        goto EmitVexEvexM;
      }
      break;

    case InstDB::kEncodingVexRvmRmv:
      if (isign3 == ENC_OPS3(Reg, Reg, Reg)) {
        op_reg = pack_reg_and_vvvvv(o0.id(), o2.id());
        rb_reg = o1.id();

        if (!Support::test(options, InstOptions::kX86_ModMR))
          goto EmitVexEvexR;

        opcode.add_w();
        op_reg = pack_reg_and_vvvvv(o0.id(), o1.id());
        rb_reg = o2.id();
        goto EmitVexEvexR;
      }

      if (isign3 == ENC_OPS3(Reg, Mem, Reg)) {
        op_reg = pack_reg_and_vvvvv(o0.id(), o2.id());
        rm_rel = &o1;
        goto EmitVexEvexM;
      }

      if (isign3 == ENC_OPS3(Reg, Reg, Mem)) {
        opcode.add_w();
        op_reg = pack_reg_and_vvvvv(o0.id(), o1.id());
        rm_rel = &o2;
        goto EmitVexEvexM;
      }
      break;

    case InstDB::kEncodingVexRvmRmi_Lx:
      opcode |= opcode_l_by_size(o0.x86_rm_size() | o1.x86_rm_size());
      [[fallthrough]];

    case InstDB::kEncodingVexRvmRmi:
      if (isign3 == ENC_OPS3(Reg, Reg, Reg)) {
        op_reg = pack_reg_and_vvvvv(o0.id(), o1.id());
        rb_reg = o2.id();
        goto EmitVexEvexR;
      }

      if (isign3 == ENC_OPS3(Reg, Reg, Mem)) {
        op_reg = pack_reg_and_vvvvv(o0.id(), o1.id());
        rm_rel = &o2;
        goto EmitVexEvexM;
      }

      // The following instructions use the secondary opcode.
      opcode &= Opcode::kLL_Mask;
      opcode |= alt_opcode_of(inst_info);

      imm_value = o2.as<Imm>().value();
      imm_size = 1;

      if (isign3 == ENC_OPS3(Reg, Reg, Imm)) {
        op_reg = o0.id();
        rb_reg = o1.id();
        goto EmitVexEvexR;
      }

      if (isign3 == ENC_OPS3(Reg, Mem, Imm)) {
        op_reg = o0.id();
        rm_rel = &o1;
        goto EmitVexEvexM;
      }
      break;

    case InstDB::kEncodingVexRvmRmvRmi:
      if (isign3 == ENC_OPS3(Reg, Reg, Reg)) {
        op_reg = pack_reg_and_vvvvv(o0.id(), o2.id());
        rb_reg = o1.id();

        if (!Support::test(options, InstOptions::kX86_ModMR))
          goto EmitVexEvexR;

        opcode.add_w();
        op_reg = pack_reg_and_vvvvv(o0.id(), o1.id());
        rb_reg = o2.id();
        goto EmitVexEvexR;
      }

      if (isign3 == ENC_OPS3(Reg, Mem, Reg)) {
        op_reg = pack_reg_and_vvvvv(o0.id(), o2.id());
        rm_rel = &o1;
        goto EmitVexEvexM;
      }

      if (isign3 == ENC_OPS3(Reg, Reg, Mem)) {
        opcode.add_w();
        op_reg = pack_reg_and_vvvvv(o0.id(), o1.id());
        rm_rel = &o2;
        goto EmitVexEvexM;
      }

      // The following instructions use the secondary opcode.
      opcode = alt_opcode_of(inst_info);

      imm_value = o2.as<Imm>().value();
      imm_size = 1;

      if (isign3 == ENC_OPS3(Reg, Reg, Imm)) {
        op_reg = o0.id();
        rb_reg = o1.id();
        goto EmitVexEvexR;
      }

      if (isign3 == ENC_OPS3(Reg, Mem, Imm)) {
        op_reg = o0.id();
        rm_rel = &o1;
        goto EmitVexEvexM;
      }
      break;

    case InstDB::kEncodingVexRvmMr:
      if (isign3 == ENC_OPS3(Reg, Reg, Reg)) {
        op_reg = pack_reg_and_vvvvv(o0.id(), o1.id());
        rb_reg = o2.id();
        goto EmitVexEvexR;
      }

      if (isign3 == ENC_OPS3(Reg, Reg, Mem)) {
        op_reg = pack_reg_and_vvvvv(o0.id(), o1.id());
        rm_rel = &o2;
        goto EmitVexEvexM;
      }

      // The following instructions use the secondary opcode.
      opcode = alt_opcode_of(inst_info);

      if (isign3 == ENC_OPS2(Reg, Reg)) {
        op_reg = o1.id();
        rb_reg = o0.id();
        goto EmitVexEvexR;
      }

      if (isign3 == ENC_OPS2(Mem, Reg)) {
        op_reg = o1.id();
        rm_rel = &o0;
        goto EmitVexEvexM;
      }
      break;

    case InstDB::kEncodingVexRvmMvr_Lx:
      opcode |= opcode_l_by_size(o0.x86_rm_size() | o1.x86_rm_size());
      [[fallthrough]];

    case InstDB::kEncodingVexRvmMvr:
      if (isign3 == ENC_OPS3(Reg, Reg, Reg)) {
        op_reg = pack_reg_and_vvvvv(o0.id(), o1.id());
        rb_reg = o2.id();
        goto EmitVexEvexR;
      }

      if (isign3 == ENC_OPS3(Reg, Reg, Mem)) {
        op_reg = pack_reg_and_vvvvv(o0.id(), o1.id());
        rm_rel = &o2;
        goto EmitVexEvexM;
      }

      // The following instruction uses the secondary opcode.
      opcode &= Opcode::kLL_Mask;
      opcode |= alt_opcode_of(inst_info);

      if (isign3 == ENC_OPS3(Mem, Reg, Reg)) {
        op_reg = pack_reg_and_vvvvv(o2.id(), o1.id());
        rm_rel = &o0;
        goto EmitVexEvexM;
      }
      break;

    case InstDB::kEncodingVexRvmVmi_Lx_MEvex:
      opcode.force_evex_if(o1.is_mem());
      [[fallthrough]];

    case InstDB::kEncodingVexRvmVmi_Lx:
      opcode |= opcode_l_by_size(o0.x86_rm_size() | o1.x86_rm_size());
      [[fallthrough]];

    case InstDB::kEncodingVexRvmVmi:
      if (isign3 == ENC_OPS3(Reg, Reg, Reg)) {
        op_reg = pack_reg_and_vvvvv(o0.id(), o1.id());
        rb_reg = o2.id();
        goto EmitVexEvexR;
      }

      if (isign3 == ENC_OPS3(Reg, Reg, Mem)) {
        op_reg = pack_reg_and_vvvvv(o0.id(), o1.id());
        rm_rel = &o2;
        goto EmitVexEvexM;
      }

      // The following instruction uses the secondary opcode.
      opcode &= Opcode::kLL_Mask | Opcode::kMM_ForceEvex;
      opcode |= alt_opcode_of(inst_info);
      op_reg = opcode.extract_mod_o();

      imm_value = o2.as<Imm>().value();
      imm_size = 1;

      if (isign3 == ENC_OPS3(Reg, Reg, Imm)) {
        op_reg = pack_reg_and_vvvvv(op_reg, o0.id());
        rb_reg = o1.id();
        goto EmitVexEvexR;
      }

      if (isign3 == ENC_OPS3(Reg, Mem, Imm)) {
        op_reg = pack_reg_and_vvvvv(op_reg, o0.id());
        rm_rel = &o1;
        goto EmitVexEvexM;
      }
      break;

    case InstDB::kEncodingVexVm_Wx:
      opcode.add_w_if(Support::bool_or(o0.is_gp64(), o1.is_gp64()));
      [[fallthrough]];

    case InstDB::kEncodingVexVm:
      if (isign3 == ENC_OPS2(Reg, Reg)) {
        op_reg = pack_reg_and_vvvvv(op_reg, o0.id());
        rb_reg = o1.id();
        goto EmitVexEvexR;
      }

      if (isign3 == ENC_OPS2(Reg, Mem)) {
        op_reg = pack_reg_and_vvvvv(op_reg, o0.id());
        rm_rel = &o1;
        goto EmitVexEvexM;
      }
      break;

    case InstDB::kEncodingVexVmi_Lx_MEvex:
      if (isign3 == ENC_OPS3(Reg, Mem, Imm))
        opcode.force_evex();
      [[fallthrough]];

    case InstDB::kEncodingVexVmi_Lx:
      opcode |= opcode_l_by_size(o0.x86_rm_size() | o1.x86_rm_size());
      [[fallthrough]];

    case InstDB::kEncodingVexVmi:
      imm_value = o2.as<Imm>().value();
      imm_size = 1;

CaseVexVmi_AfterImm:
      if (isign3 == ENC_OPS3(Reg, Reg, Imm)) {
        op_reg = pack_reg_and_vvvvv(op_reg, o0.id());
        rb_reg = o1.id();
        goto EmitVexEvexR;
      }

      if (isign3 == ENC_OPS3(Reg, Mem, Imm)) {
        op_reg = pack_reg_and_vvvvv(op_reg, o0.id());
        rm_rel = &o1;
        goto EmitVexEvexM;
      }
      break;

    case InstDB::kEncodingVexVmi4_Wx:
      opcode.add_w_if(Support::bool_or(o0.is_gp64(), o1.x86_rm_size() == 8));
      imm_value = o2.as<Imm>().value();
      imm_size = 4;
      goto CaseVexVmi_AfterImm;

    case InstDB::kEncodingVexRvrmRvmr_Lx:
      opcode |= opcode_l_by_size(o0.x86_rm_size() | o1.x86_rm_size());
      [[fallthrough]];

    case InstDB::kEncodingVexRvrmRvmr: {
      const Operand_& o3 = op_ext[EmitterUtils::kOp3];
      const uint32_t isign4 = isign3 + (uint32_t(o3.op_type()) << 9);

      if (isign4 == ENC_OPS4(Reg, Reg, Reg, Reg)) {
        op_reg = pack_reg_and_vvvvv(o0.id(), o1.id());
        rb_reg = o2.id();

        imm_value = o3.id() << 4;
        imm_size = 1;
        goto EmitVexEvexR;
      }

      if (isign4 == ENC_OPS4(Reg, Reg, Reg, Mem)) {
        opcode.add_w();
        op_reg = pack_reg_and_vvvvv(o0.id(), o1.id());
        rm_rel = &o3;

        imm_value = o2.id() << 4;
        imm_size = 1;
        goto EmitVexEvexM;
      }

      if (isign4 == ENC_OPS4(Reg, Reg, Mem, Reg)) {
        op_reg = pack_reg_and_vvvvv(o0.id(), o1.id());
        rm_rel = &o2;

        imm_value = o3.id() << 4;
        imm_size = 1;
        goto EmitVexEvexM;
      }
      break;
    }

    case InstDB::kEncodingVexRvrmiRvmri_Lx: {
      const Operand_& o3 = op_ext[EmitterUtils::kOp3];
      const Operand_& o4 = op_ext[EmitterUtils::kOp4];

      if (ASMJIT_UNLIKELY(!o4.is_imm()))
        goto InvalidInstruction;

      const uint32_t isign4 = isign3 + (uint32_t(o3.op_type()) << 9);
      opcode |= opcode_l_by_size(o0.x86_rm_size() | o1.x86_rm_size() | o2.x86_rm_size() | o3.x86_rm_size());

      imm_value = o4.as<Imm>().value_as<uint8_t>() & 0x0F;
      imm_size = 1;

      if (isign4 == ENC_OPS4(Reg, Reg, Reg, Reg)) {
        op_reg = pack_reg_and_vvvvv(o0.id(), o1.id());
        rb_reg = o2.id();

        imm_value |= o3.id() << 4;
        goto EmitVexEvexR;
      }

      if (isign4 == ENC_OPS4(Reg, Reg, Reg, Mem)) {
        opcode.add_w();
        op_reg = pack_reg_and_vvvvv(o0.id(), o1.id());
        rm_rel = &o3;

        imm_value |= o2.id() << 4;
        goto EmitVexEvexM;
      }

      if (isign4 == ENC_OPS4(Reg, Reg, Mem, Reg)) {
        op_reg = pack_reg_and_vvvvv(o0.id(), o1.id());
        rm_rel = &o2;

        imm_value |= o3.id() << 4;
        goto EmitVexEvexM;
      }
      break;
    }

    case InstDB::kEncodingVexMovssMovsd:
      if (isign3 == ENC_OPS3(Reg, Reg, Reg)) {
        goto CaseVexRvm_R;
      }

      if (isign3 == ENC_OPS2(Reg, Mem)) {
        op_reg = o0.id();
        rm_rel = &o1;
        goto EmitVexEvexM;
      }

      if (isign3 == ENC_OPS2(Mem, Reg)) {
        opcode = alt_opcode_of(inst_info);
        op_reg = o1.id();
        rm_rel = &o0;
        goto EmitVexEvexM;
      }
      break;

    // FMA4 Instructions
    // -----------------

    case InstDB::kEncodingFma4_Lx:
      // It's fine to just check the first operand, second is just for sanity.
      opcode |= opcode_l_by_size(o0.x86_rm_size() | o1.x86_rm_size());
      [[fallthrough]];

    case InstDB::kEncodingFma4: {
      const Operand_& o3 = op_ext[EmitterUtils::kOp3];
      const uint32_t isign4 = isign3 + (uint32_t(o3.op_type()) << 9);

      if (isign4 == ENC_OPS4(Reg, Reg, Reg, Reg)) {
        op_reg = pack_reg_and_vvvvv(o0.id(), o1.id());

        if (!Support::test(options, InstOptions::kX86_ModMR)) {
          // MOD/RM - Encoding preferred by LLVM.
          opcode.add_w();
          rb_reg = o3.id();

          imm_value = o2.id() << 4;
          imm_size = 1;
          goto EmitVexEvexR;
        }
        else {
          // MOD/MR - Alternative encoding.
          rb_reg = o2.id();

          imm_value = o3.id() << 4;
          imm_size = 1;
          goto EmitVexEvexR;
        }
      }

      if (isign4 == ENC_OPS4(Reg, Reg, Reg, Mem)) {
        opcode.add_w();
        op_reg = pack_reg_and_vvvvv(o0.id(), o1.id());
        rm_rel = &o3;

        imm_value = o2.id() << 4;
        imm_size = 1;
        goto EmitVexEvexM;
      }

      if (isign4 == ENC_OPS4(Reg, Reg, Mem, Reg)) {
        op_reg = pack_reg_and_vvvvv(o0.id(), o1.id());
        rm_rel = &o2;

        imm_value = o3.id() << 4;
        imm_size = 1;
        goto EmitVexEvexM;
      }
      break;
    }

    // AMX Instructions
    // ----------------

    case InstDB::kEncodingAmxCfg:
      if (isign3 == ENC_OPS1(Mem)) {
        rm_rel = &o0;
        goto EmitVexEvexM;
      }
      break;

    case InstDB::kEncodingAmxR:
      if (isign3 == ENC_OPS1(Reg)) {
        op_reg = o0.id();
        rb_reg = 0;
        goto EmitVexEvexR;
      }
      break;

    case InstDB::kEncodingAmxRm:
      if (isign3 == ENC_OPS2(Reg, Mem)) {
        op_reg = o0.id();
        rm_rel = &o1;
        goto EmitVexEvexM;
      }
      break;

    case InstDB::kEncodingAmxMr:
      if (isign3 == ENC_OPS2(Mem, Reg)) {
        op_reg = o1.id();
        rm_rel = &o0;
        goto EmitVexEvexM;
      }
      break;

    case InstDB::kEncodingAmxRmv:
      if (isign3 == ENC_OPS3(Reg, Reg, Reg)) {
        op_reg = pack_reg_and_vvvvv(o0.id(), o2.id());
        rb_reg = o1.id();
        goto EmitVexEvexR;
      }
      break;
  }

  goto InvalidInstruction;

  // Emit - X86 Opcode
  // -----------------

EmitX86OpMovAbs:
  imm_size = FastUInt8(register_size());
  writer.emit_segment_override(rm_rel->as<Mem>().segment_id());

EmitX86Op:
  // Emit mandatory instruction prefix.
  writer.emit_pp(opcode.v);

  // Emit REX prefix (64-bit only).
  {
    uint32_t rex = opcode.extract_rex(options);
    if (ASMJIT_UNLIKELY(is_rex_invalid(rex)))
      goto InvalidRexPrefix;
    rex &= ~kX86ByteInvalidRex & 0xFF;
    writer.emit8_if(rex | kX86ByteRex, rex != 0);
  }

  // Emit instruction opcodes.
  writer.emit_mm_and_opcode(opcode.v);
  writer.emit_immediate(uint64_t(imm_value), imm_size);
  goto EmitDone;

  // Emit - X86 - Opcode + Reg
  // -------------------------

EmitX86OpReg:
  // Emit mandatory instruction prefix.
  writer.emit_pp(opcode.v);

  // Emit REX prefix (64-bit only).
  {
    uint32_t rex = opcode.extract_rex(options) | (op_reg >> 3); // Rex.B (0x01).
    if (ASMJIT_UNLIKELY(is_rex_invalid(rex)))
      goto InvalidRexPrefix;
    rex &= ~kX86ByteInvalidRex & 0xFF;
    writer.emit8_if(rex | kX86ByteRex, rex != 0);

    op_reg &= 0x7;
  }

  // Emit instruction opcodes.
  opcode += op_reg;
  writer.emit_mm_and_opcode(opcode.v);
  writer.emit_immediate(uint64_t(imm_value), imm_size);
  goto EmitDone;

  // Emit - X86 - Opcode with Implicit <mem> Operand
  // -----------------------------------------------

EmitX86OpImplicitMem:
  rm_info = mem_info_table[rm_rel->as<Mem>().base_and_index_types()];
  if (ASMJIT_UNLIKELY(rm_rel->as<Mem>().has_offset() || (rm_info & kX86MemInfo_Index)))
    goto InvalidInstruction;

  // Emit mandatory instruction prefix.
  writer.emit_pp(opcode.v);

  // Emit REX prefix (64-bit only).
  {
    uint32_t rex = opcode.extract_rex(options);
    if (ASMJIT_UNLIKELY(is_rex_invalid(rex)))
      goto InvalidRexPrefix;
    rex &= ~kX86ByteInvalidRex & 0xFF;
    writer.emit8_if(rex | kX86ByteRex, rex != 0);
  }

  // Emit override prefixes.
  writer.emit_segment_override(rm_rel->as<Mem>().segment_id());
  writer.emit_address_override((rm_info & _address_override_mask()) != 0);

  // Emit instruction opcodes.
  writer.emit_mm_and_opcode(opcode.v);

  // Emit immediate value.
  writer.emit_immediate(uint64_t(imm_value), imm_size);
  goto EmitDone;

  // Emit - X86 - Opcode /r - Register
  // ---------------------------------

EmitX86R:
  // Mandatory instruction prefix.
  writer.emit_pp(opcode.v);

  // Emit REX prefix (64-bit only).
  {
    uint32_t rex = opcode.extract_rex(options) |
                   ((op_reg & 0x08) >> 1) | // REX.R (0x04).
                   ((rb_reg & 0x08) >> 3) ; // REX.B (0x01).

    if (ASMJIT_UNLIKELY(is_rex_invalid(rex)))
      goto InvalidRexPrefix;
    rex &= ~kX86ByteInvalidRex & 0xFF;
    writer.emit8_if(rex | kX86ByteRex, rex != 0);

    op_reg &= 0x07;
    rb_reg &= 0x07;
  }

  // Emit instruction opcodes.
  writer.emit_mm_and_opcode(opcode.v);

  // Emit ModR.
  writer.emit8(encode_mod(3, op_reg, rb_reg));

  // Emit immediate value.
  writer.emit_immediate(uint64_t(imm_value), imm_size);
  goto EmitDone;

  // Emit - X86 - Opcode /r - Memory Base
  // ------------------------------------

EmitX86RFromM:
  rm_info = mem_info_table[rm_rel->as<Mem>().base_and_index_types()];
  if (ASMJIT_UNLIKELY(rm_rel->as<Mem>().has_offset() || (rm_info & kX86MemInfo_Index)))
    goto InvalidInstruction;

  // Emit mandatory instruction prefix.
  writer.emit_pp(opcode.v);

  // Emit REX prefix (64-bit only).
  {
    uint32_t rex = opcode.extract_rex(options) |
                   ((op_reg & 0x08) >> 1) | // REX.R (0x04).
                   ((rb_reg       ) >> 3) ; // REX.B (0x01).

    if (ASMJIT_UNLIKELY(is_rex_invalid(rex)))
      goto InvalidRexPrefix;
    rex &= ~kX86ByteInvalidRex & 0xFF;
    writer.emit8_if(rex | kX86ByteRex, rex != 0);

    op_reg &= 0x07;
    rb_reg &= 0x07;
  }

  // Emit override prefixes.
  writer.emit_segment_override(rm_rel->as<Mem>().segment_id());
  writer.emit_address_override((rm_info & _address_override_mask()) != 0);

  // Emit instruction opcodes.
  writer.emit_mm_and_opcode(opcode.v);

  // Emit ModR/M.
  writer.emit8(encode_mod(3, op_reg, rb_reg));

  // Emit immediate value.
  writer.emit_immediate(uint64_t(imm_value), imm_size);
  goto EmitDone;

  // Emit - X86 - Opcode /r - memory Operand
  // ---------------------------------------

EmitX86M:
  // `rm_rel` operand must be memory.
  ASMJIT_ASSERT(rm_rel != nullptr);
  ASMJIT_ASSERT(rm_rel->op_type() == OperandType::kMem);
  ASMJIT_ASSERT((opcode & Opcode::kCDSHL_Mask) == 0);

  // Emit override prefixes.
  rm_info = mem_info_table[rm_rel->as<Mem>().base_and_index_types()];
  writer.emit_segment_override(rm_rel->as<Mem>().segment_id());

  mem_op_ao_mark = writer.cursor();
  writer.emit_address_override((rm_info & _address_override_mask()) != 0);

  // Emit mandatory instruction prefix.
  writer.emit_pp(opcode.v);

  // Emit REX prefix (64-bit only).
  rb_reg = rm_rel->as<Mem>().base_id();
  rx_reg = rm_rel->as<Mem>().index_id();
  {
    uint32_t rex;

    rex  = (rb_reg >> 3) & 0x01; // REX.B (0x01).
    rex |= (rx_reg >> 2) & 0x02; // REX.X (0x02).
    rex |= (op_reg >> 1) & 0x04; // REX.R (0x04).

    rex &= rm_info;
    rex |= opcode.extract_rex(options);

    if (ASMJIT_UNLIKELY(is_rex_invalid(rex)))
      goto InvalidRexPrefix;
    rex &= ~kX86ByteInvalidRex & 0xFF;
    writer.emit8_if(rex | kX86ByteRex, rex != 0);

    op_reg &= 0x07;
  }

  // Emit instruction opcodes.
  writer.emit_mm_and_opcode(opcode.v);

  // ... Fall through ...

  // Emit - MOD/SIB
  // --------------

EmitModSib:
  if (!(rm_info & (kX86MemInfo_Index | kX86MemInfo_67H_X86))) {
    // ==========|> [BASE + DISP8|DISP32].
    if (rm_info & kX86MemInfo_BaseGp) {
      rb_reg &= 0x7;
      rel_offset = rm_rel->as<Mem>().offset_lo32();

      uint32_t mod = encode_mod(0, op_reg, rb_reg);
      bool force_sib = common_info->is_tsib_op();

      if (rb_reg == Gp::kIdSp || force_sib) {
        // TSIB or [XSP|R12].
        mod = (mod & 0xF8u) | 0x04u;
        if (rb_reg != Gp::kIdBp && rel_offset == 0) {
          writer.emit8(mod);
          writer.emit8(encode_sib(0, 4, rb_reg));
        }
        // TSIB or [XSP|R12 + DISP8|DISP32].
        else {
          uint32_t cd_shift = (opcode & Opcode::kCDSHL_Mask) >> Opcode::kCDSHL_Shift;
          int32_t cd_offset = rel_offset >> cd_shift;

          if (Support::is_int_n<8>(cd_offset) && rel_offset == int32_t(uint32_t(cd_offset) << cd_shift)) {
            writer.emit8(mod + 0x40); // <- MOD(1, op_reg, rb_reg).
            writer.emit8(encode_sib(0, 4, rb_reg));
            writer.emit8(cd_offset & 0xFF);
          }
          else {
            writer.emit8(mod + 0x80); // <- MOD(2, op_reg, rb_reg).
            writer.emit8(encode_sib(0, 4, rb_reg));
            writer.emit32u_le(uint32_t(rel_offset));
          }
        }
      }
      else if (rb_reg != Gp::kIdBp && rel_offset == 0) {
        // [BASE].
        writer.emit8(mod);
      }
      else {
        // [BASE + DISP8|DISP32].
        uint32_t cd_shift = (opcode & Opcode::kCDSHL_Mask) >> Opcode::kCDSHL_Shift;
        int32_t cd_offset = rel_offset >> cd_shift;

        if (Support::is_int_n<8>(cd_offset) && rel_offset == int32_t(uint32_t(cd_offset) << cd_shift)) {
          writer.emit8(mod + 0x40);
          writer.emit8(cd_offset & 0xFF);
        }
        else {
          writer.emit8(mod + 0x80);
          writer.emit32u_le(uint32_t(rel_offset));
        }
      }
    }
    // ==========|> [ABSOLUTE | DISP32].
    else if (!(rm_info & (kX86MemInfo_BaseLabel | kX86MemInfo_BaseRip))) {
      Mem::AddrType addr_type = rm_rel->as<Mem>().addr_type();
      rel_offset = rm_rel->as<Mem>().offset_lo32();

      if (is_32bit()) {
        // Explicit relative addressing doesn't work in 32-bit mode.
        if (ASMJIT_UNLIKELY(addr_type == Mem::AddrType::kRel))
          goto InvalidAddress;

        writer.emit8(encode_mod(0, op_reg, 5));
        writer.emit32u_le(uint32_t(rel_offset));
      }
      else {
        bool is_offset_int32 = rm_rel->as<Mem>().offset_hi32() == (rel_offset >> 31);
        bool is_offset_uint32 = rm_rel->as<Mem>().offset_hi32() == 0;
        uint64_t base_address = code()->base_address();

        // If relative addressing was not explicitly set then we can try to guess. By guessing we check some
        // properties of the memory operand and try to base the decision on the segment prefix and the address type.
        if (addr_type == Mem::AddrType::kDefault) {
          if (base_address == Globals::kNoBaseAddress) {
            // Prefer absolute addressing mode if the offset is 32-bit.
            addr_type = is_offset_int32 || is_offset_uint32 ? Mem::AddrType::kAbs : Mem::AddrType::kRel;
          }
          else {
            // Prefer absolute addressing mode if FS|GS segment override is present.
            bool has_fs_gs = rm_rel->as<Mem>().segment_id() >= SReg::kIdFs;
            // Prefer absolute addressing mode if this is LEA with 32-bit immediate.
            bool is_lea_32 = (inst_id == Inst::kIdLea) && (is_offset_int32 || is_offset_uint32);

            addr_type = has_fs_gs || is_lea_32 ? Mem::AddrType::kAbs : Mem::AddrType::kRel;
          }
        }

        if (addr_type == Mem::AddrType::kRel) {
          uint32_t kModRel32Size = 5;
          uint64_t virtual_offset = uint64_t(writer.offset_from(_buffer_data)) + imm_size + kModRel32Size;

          if (base_address == Globals::kNoBaseAddress || _section->section_id() != 0) {
            // Create a new RelocEntry as we cannot calculate the offset right now.
            err = _code->new_reloc_entry(Out(re), RelocType::kAbsToRel);
            if (ASMJIT_UNLIKELY(err != Error::kOk))
              goto Failed;

            writer.emit8(encode_mod(0, op_reg, 5));

            re->_source_section_id = _section->section_id();
            re->_source_offset = offset();
            re->_format.reset_to_simple_value(OffsetType::kSignedOffset, 4);
            re->_format.set_leading_and_trailing_size(writer.offset_from(_buffer_ptr), imm_size);
            re->_payload = uint64_t(rm_rel->as<Mem>().offset());

            writer.emit32u_le(0);
            writer.emit_immediate(uint64_t(imm_value), imm_size);
            goto EmitDone;
          }
          else {
            uint64_t rip64 = base_address + _section->offset() + virtual_offset;
            uint64_t rel64 = uint64_t(rm_rel->as<Mem>().offset()) - rip64;

            if (Support::is_int_n<32>(int64_t(rel64))) {
              writer.emit8(encode_mod(0, op_reg, 5));
              writer.emit32u_le(uint32_t(rel64 & 0xFFFFFFFFu));
              writer.emit_immediate(uint64_t(imm_value), imm_size);
              goto EmitDone;
            }
            else {
              // We must check the original address type as we have modified
              // `addr_type`. We failed if the original address type is 'rel'.
              if (ASMJIT_UNLIKELY(rm_rel->as<Mem>().is_addr_rel()))
                goto InvalidAddress;
            }
          }
        }

        // Handle unsigned 32-bit address that doesn't work with sign extension. Consider the following instructions:
        //
        //   1. lea rax, [-1]         - Sign extended to 0xFFFFFFFFFFFFFFFF
        //   2. lea rax, [0xFFFFFFFF] - Zero extended to 0x00000000FFFFFFFF
        //   3. add rax, [-1]         - Sign extended to 0xFFFFFFFFFFFFFFFF
        //   4. add rax, [0xFFFFFFFF] - Zero extended to 0x00000000FFFFFFFF
        //
        // Sign extension is naturally performed by the CPU so we don't have to bother, however, zero extension
        // requires address-size override prefix, which we probably don't have at this moment. So to make the address
        // valid we need to insert it at `mem_op_ao_mark` if it's not already there.
        //
        // If this is 'lea' instruction then it's possible to remove REX.W part from REX prefix (if it's there), which
        // would be one-byte shorter than inserting address-size override.
        //
        // NOTE: If we don't do this then these instructions are unencodable.
        if (!is_offset_int32) {
          // 64-bit absolute address is unencodable.
          if (ASMJIT_UNLIKELY(!is_offset_uint32))
            goto InvalidAddress64Bit;

          // We only patch the existing code if we don't have address-size override.
          if (*mem_op_ao_mark != 0x67) {
            if (inst_id == Inst::kIdLea) {
              // LEA: Remove REX.W, if present. This is easy as we know that 'lea' doesn't use any PP prefix so if REX
              // prefix was emitted it would be at `mem_op_ao_mark`.
              uint32_t rex = *mem_op_ao_mark;
              if (rex & kX86ByteRex) {
                rex &= (~kX86ByteRexW) & 0xFF;
                *mem_op_ao_mark = uint8_t(rex);

                // We can remove the REX prefix completely if it was not forced.
                if (rex == kX86ByteRex && !Support::test(options, InstOptions::kX86_Rex))
                  writer.remove8(mem_op_ao_mark);
              }
            }
            else {
              // Any other instruction: Insert address-size override prefix.
              writer.insert8(mem_op_ao_mark, 0x67);
            }
          }
        }

        // Emit 32-bit absolute address.
        writer.emit8(encode_mod(0, op_reg, 4));
        writer.emit8(encode_sib(0, 4, 5));
        writer.emit32u_le(uint32_t(rel_offset));
      }
    }
    // ==========|> [LABEL|RIP + DISP32]
    else {
      writer.emit8(encode_mod(0, op_reg, 5));

      if (is_32bit()) {
EmitModSib_LabelRip_X86:
        if (ASMJIT_UNLIKELY(_code->_relocations.reserve_additional(_code->arena()) != Error::kOk))
          goto OutOfMemory;

        rel_offset = rm_rel->as<Mem>().offset_lo32();
        if (rm_info & kX86MemInfo_BaseLabel) {
          // [LABEL->ABS].
          uint32_t base_label_id = rm_rel->as<Mem>().base_id();
          if (ASMJIT_UNLIKELY(!_code->is_label_valid(base_label_id))) {
          }

          label = &_code->label_entry_of(base_label_id);
          err = _code->new_reloc_entry(Out(re), RelocType::kRelToAbs);

          if (ASMJIT_UNLIKELY(err != Error::kOk)) {
            goto Failed;
          }

          re->_source_section_id = _section->section_id();
          re->_source_offset = offset();
          re->_format.reset_to_simple_value(OffsetType::kUnsignedOffset, 4);
          re->_format.set_leading_and_trailing_size(writer.offset_from(_buffer_ptr), imm_size);
          re->_payload = uint64_t(int64_t(rel_offset));

          if (label->is_bound()) {
            // Label bound to the current section.
            re->_payload += label->offset();
            re->_target_section_id = label->section_id();
            writer.emit32u_le(0);
          }
          else {
            // Non-bound label or label bound to a different section.
            rel_offset = -4 - int32_t(imm_size);
            rel_size = 4;
            goto EmitRel;
          }
        }
        else {
          // [RIP->ABS].
          err = _code->new_reloc_entry(Out(re), RelocType::kRelToAbs);
          if (ASMJIT_UNLIKELY(err != Error::kOk))
            goto Failed;

          re->_source_section_id = _section->section_id();
          re->_target_section_id = _section->section_id();
          re->_format.reset_to_simple_value(OffsetType::kUnsignedOffset, 4);
          re->_format.set_leading_and_trailing_size(writer.offset_from(_buffer_ptr), imm_size);
          re->_source_offset = offset();
          re->_payload = re->_source_offset + re->_format.region_size() + uint64_t(int64_t(rel_offset));

          writer.emit32u_le(0);
        }
      }
      else {
        rel_offset = rm_rel->as<Mem>().offset_lo32();
        if (rm_info & kX86MemInfo_BaseLabel) {
          // [RIP].
          uint32_t base_label_id = rm_rel->as<Mem>().base_id();
          if (ASMJIT_UNLIKELY(!_code->is_label_valid(base_label_id))) {
            goto InvalidLabel;
          }

          label = &_code->label_entry_of(base_label_id);
          rel_offset -= (4 + imm_size);

          if (label->is_bound_to(_section)) {
            // Label bound to the current section.
            rel_offset += int32_t(label->offset() - writer.offset_from(_buffer_data));
            writer.emit32u_le(uint32_t(rel_offset));
          }
          else {
            // Non-bound label or label bound to a different section.
            rel_size = 4;
            goto EmitRel;
          }
        }
        else {
          // [RIP].
          writer.emit32u_le(uint32_t(rel_offset));
        }
      }
    }
  }
  else if (!(rm_info & kX86MemInfo_67H_X86)) {
    // ESP|RSP can't be used as INDEX in pure SIB mode, however, VSIB mode allows XMM4|YMM4|ZMM4 (that's why the
    // check is before the label).
    if (ASMJIT_UNLIKELY(rx_reg == Gp::kIdSp))
      goto InvalidAddressIndex;

EmitModVSib:
    rx_reg &= 0x7;

    // ==========|> [BASE + INDEX + DISP8|DISP32].
    if (rm_info & kX86MemInfo_BaseGp) {
      rb_reg &= 0x7;
      rel_offset = rm_rel->as<Mem>().offset_lo32();

      uint32_t mod = encode_mod(0, op_reg, 4);
      uint32_t sib = encode_sib(rm_rel->as<Mem>().shift(), rx_reg, rb_reg);

      if (rel_offset == 0 && rb_reg != Gp::kIdBp) {
        // [BASE + INDEX << SHIFT].
        writer.emit8(mod);
        writer.emit8(sib);
      }
      else {
        uint32_t cd_shift = (opcode & Opcode::kCDSHL_Mask) >> Opcode::kCDSHL_Shift;
        int32_t cd_offset = rel_offset >> cd_shift;

        if (Support::is_int_n<8>(cd_offset) && rel_offset == int32_t(uint32_t(cd_offset) << cd_shift)) {
          // [BASE + INDEX << SHIFT + DISP8].
          writer.emit8(mod + 0x40); // <- MOD(1, op_reg, 4).
          writer.emit8(sib);
          writer.emit8(uint32_t(cd_offset));
        }
        else {
          // [BASE + INDEX << SHIFT + DISP32].
          writer.emit8(mod + 0x80); // <- MOD(2, op_reg, 4).
          writer.emit8(sib);
          writer.emit32u_le(uint32_t(rel_offset));
        }
      }
    }
    // ==========|> [INDEX + DISP32].
    else if (!(rm_info & (kX86MemInfo_BaseLabel | kX86MemInfo_BaseRip))) {
      // [INDEX << SHIFT + DISP32].
      writer.emit8(encode_mod(0, op_reg, 4));
      writer.emit8(encode_sib(rm_rel->as<Mem>().shift(), rx_reg, 5));

      rel_offset = rm_rel->as<Mem>().offset_lo32();
      writer.emit32u_le(uint32_t(rel_offset));
    }
    // ==========|> [LABEL|RIP + INDEX + DISP32].
    else {
      if (is_32bit()) {
        writer.emit8(encode_mod(0, op_reg, 4));
        writer.emit8(encode_sib(rm_rel->as<Mem>().shift(), rx_reg, 5));
        goto EmitModSib_LabelRip_X86;
      }
      else {
        // NOTE: This also handles VSIB+RIP, which is not allowed in 64-bit mode.
        goto InvalidAddress;
      }
    }
  }
  else {
    // 16-bit address mode (32-bit mode with 67 override prefix).
    rel_offset = (int32_t(rm_rel->as<Mem>().offset_lo32()) << 16) >> 16;

    // NOTE: 16-bit addresses don't use SIB byte and their encoding differs. We use a table-based approach to
    // calculate the proper MOD byte as it's easier. Also, not all BASE [+ INDEX] combinations are supported
    // in 16-bit mode, so this may fail.
    const uint32_t kBaseGpIdx = (kX86MemInfo_BaseGp | kX86MemInfo_Index);

    if (rm_info & kBaseGpIdx) {
      // ==========|> [BASE + INDEX + DISP16].
      uint32_t mod;

      rb_reg &= 0x7;
      rx_reg &= 0x7;

      if ((rm_info & kBaseGpIdx) == kBaseGpIdx) {
        uint32_t shf = rm_rel->as<Mem>().shift();
        if (ASMJIT_UNLIKELY(shf != 0))
          goto InvalidAddress;
        mod = mod16_base_index_table[(rb_reg << 3) + rx_reg];
      }
      else {
        if (rm_info & kX86MemInfo_Index)
          rb_reg = rx_reg;
        mod = mod16_base_table[rb_reg];
      }

      if (ASMJIT_UNLIKELY(mod == 0xFF))
        goto InvalidAddress;

      mod += op_reg << 3;
      if (rel_offset == 0 && mod != 0x06) {
        writer.emit8(mod);
      }
      else if (Support::is_int_n<8>(rel_offset)) {
        writer.emit8(mod + 0x40);
        writer.emit8(uint32_t(rel_offset));
      }
      else {
        writer.emit8(mod + 0x80);
        writer.emit16u_le(uint32_t(rel_offset));
      }
    }
    else {
      // Not supported in 16-bit addresses.
      if (rm_info & (kX86MemInfo_BaseRip | kX86MemInfo_BaseLabel))
        goto InvalidAddress;

      // ==========|> [DISP16].
      writer.emit8(op_reg | 0x06);
      writer.emit16u_le(uint32_t(rel_offset));
    }
  }

  writer.emit_immediate(uint64_t(imm_value), imm_size);
  goto EmitDone;

  // Emit - FPU
  // ----------

EmitFpuOp:
  // Mandatory instruction prefix.
  writer.emit_pp(opcode.v);

  // FPU instructions consist of two opcodes.
  writer.emit8(opcode.v >> Opcode::kFPU_2B_Shift);
  writer.emit8(opcode.v);
  goto EmitDone;

  // Emit - VEX Opcode
  // -----------------

EmitVexOp:
  {
    // These don't use immediate.
    ASMJIT_ASSERT(imm_size == 0);

    // Only 'vzeroall' and 'vzeroupper' instructions use this encoding, they don't define 'W' to be '1' so we can
    // just check the 'mmmmm' field. Both functions can encode by using VEX2 prefix so VEX3 is basically only used
    // when specified as instruction option.
    ASMJIT_ASSERT((opcode & Opcode::kW) == 0);

    uint32_t x = (uint32_t(opcode  & Opcode::kMM_Mask      ) >> (Opcode::kMM_Shift     )) |
                 (uint32_t(opcode  & Opcode::kLL_Mask      ) >> (Opcode::kLL_Shift - 10)) |
                 (uint32_t(opcode  & Opcode::kPP_VEXMask   ) >> (Opcode::kPP_Shift -  8)) ;

    if (Support::test(options, InstOptions::kX86_Vex3)) {
      x  = (x & 0xFFFF) << 8;                               // [00000000|00000Lpp|000mmmmm|00000000].
      x ^= (kX86ByteVex3) |                                 // [........|00000Lpp|000mmmmm|__VEX3__].
           (0x07u  << 13) |                                 // [........|00000Lpp|111mmmmm|__VEX3__].
           (0x0Fu  << 19) |                                 // [........|01111Lpp|111mmmmm|__VEX3__].
           (opcode << 24) ;                                 // [_OPCODE_|01111Lpp|111mmmmm|__VEX3__].

      writer.emit32u_le(x);
      goto EmitDone;
    }
    else {
      x = ((x >> 8) ^ x) ^ 0xF9;
      writer.emit8(kX86ByteVex2);
      writer.emit8(x);
      writer.emit8(opcode.v);
      goto EmitDone;
    }
  }

  // Emit - VEX|EVEX - /r - Register
  // -------------------------------

EmitVexEvexR:
  {
    // Construct `x` - a complete EVEX|VEX prefix.
    uint32_t x = ((op_reg << 4) & 0xF980u) |                // [........|........|Vvvvv..R|R.......].
                 ((rb_reg << 2) & 0x0060u) |                // [........|........|........|.BB.....].
                 (opcode.extract_ll_mmmmm(options)) |       // [........|.LL.....|Vvvvv..R|RBBmmmmm].
                 (_extra_reg.id() << 16);                   // [........|.LL..aaa|Vvvvv..R|RBBmmmmm].
    op_reg &= 0x7;

    // Handle AVX512 options by a single branch.
    const InstOptions kAvx512Options = InstOptions::kX86_ZMask | InstOptions::kX86_ER | InstOptions::kX86_SAE;
    if (Support::test(options, kAvx512Options)) {
      static constexpr uint32_t kBcstMask = 0x1 << 20;
      static constexpr uint32_t kLLMask10 = 0x2 << 21;
      static constexpr uint32_t kLLMask11 = 0x3 << 21;

      // Designed to be easily encodable so the position must be exact. The {rz-sae} is encoded as {11},
      // so it should match the mask.
      static_assert(uint32_t(InstOptions::kX86_RZ_SAE) == kLLMask11,
                    "This code requires InstOptions::X86_RZ_SAE to match kLLMask11 to work properly");

      x |= uint32_t(options & InstOptions::kX86_ZMask);     // [........|zLLb.aaa|Vvvvv..R|RBBmmmmm].

      // Support embedded-rounding {er} and suppress-all-exceptions {sae}.
      if (Support::test(options, InstOptions::kX86_ER | InstOptions::kX86_SAE)) {
        // Embedded rounding is only encodable if the instruction is either scalar or it's a 512-bit
        // operation as the {er} rounding predicate collides with LL part of the instruction.
        if ((x & kLLMask11) != kLLMask10) {
          // Ok, so LL is not 10, thus the instruction must be scalar. Scalar instructions don't
          // support broadcast so if this instruction supports it {er} nor {sae} would be encodable.
          if (ASMJIT_UNLIKELY(common_info->has_avx512_bcst()))
            goto InvalidEROrSAE;
        }

        if (Support::test(options, InstOptions::kX86_ER)) {
          if (ASMJIT_UNLIKELY(!common_info->has_avx512_er()))
            goto InvalidEROrSAE;

          x &=~kLLMask11;                                   // [........|.00..aaa|Vvvvv..R|RBBmmmmm].
          x |= kBcstMask | (uint32_t(options) & kLLMask11); // [........|.LLb.aaa|Vvvvv..R|RBBmmmmm].
        }
        else {
          if (ASMJIT_UNLIKELY(!common_info->has_avx512_sae()))
            goto InvalidEROrSAE;

          x &=~kLLMask11;                                   // [........|.00..aaa|Vvvvv..R|RBBmmmmm].
          x |= kBcstMask;                                   // [........|.00b.aaa|Vvvvv..R|RBBmmmmm].
        }
      }
    }

    // These bits would force EVEX prefix.
    constexpr uint32_t kEvexForce = 0x00000010u;            // [........|........|........|...x....].
    constexpr uint32_t kEvexBits = 0x00D78150u;             // [........|xx.x.xxx|x......x|.x.x....].

    // Force EVEX prefix even in case the instruction has VEX encoding, because EVEX encoding is preferred. At the
    // moment this is only required by AVX_VNNI instructions, which were added after AVX512_VNNI instructions. If
    // such instruction doesn't specify prefix, EVEX (AVX512_VNNI) is selected by default.
    if (common_info->prefer_evex()) {
      if ((x & kEvexBits) == 0 && !Support::test(options, InstOptions::kX86_Vex | InstOptions::kX86_Vex3)) {
        x |= kEvexForce;
      }
    }

    // Check if EVEX is required by checking bits in `x` :     [........|xx.x.xxx|x......x|.x.x....].
    if (x & kEvexBits) {
      uint32_t y = ((x << 4) & 0x00080000u) |               // [........|...bV...|........|........].
                   ((x >> 4) & 0x00000010u) ;               // [........|...bV...|........|...R....].
      x  = (x & 0x00FF78EFu) | y;                           // [........|zLLbVaaa|0vvvv000|RBBRmmmm].
      x  = x << 8;                                          // [zLLbVaaa|0vvvv000|RBBRmmmm|00000000].
      x |= (opcode >> kVSHR_W    ) & 0x00800000u;           // [zLLbVaaa|Wvvvv000|RBBRmmmm|00000000].
      x |= (opcode >> kVSHR_PP_EW) & 0x00830000u;           // [zLLbVaaa|Wvvvv0pp|RBBRmmmm|00000000] (added PP and EVEX.W).
                                                            //      _     ____    ____
      x ^= 0x087CF000u | kX86ByteEvex;                      // [zLLbVaaa|Wvvvv1pp|RBBRmmmm|01100010].

      writer.emit32u_le(x);
      writer.emit8(opcode.v);

      rb_reg &= 0x7;
      writer.emit8(encode_mod(3, op_reg, rb_reg));
      writer.emit_imm_byte_or_dword(uint64_t(imm_value), imm_size);
      goto EmitDone;
    }

    // Not EVEX, prepare `x` for VEX2 or VEX3:             x = [........|00L00000|0vvvv000|R0Bmmmmm].
    x |= ((opcode >> (kVSHR_W  + 8)) & 0x8000u) |           // [00000000|00L00000|Wvvvv000|R0Bmmmmm].
         ((opcode >> (kVSHR_PP + 8)) & 0x0300u) |           // [00000000|00L00000|0vvvv0pp|R0Bmmmmm].
         ((x      >> 11            ) & 0x0400u) ;           // [00000000|00L00000|WvvvvLpp|R0Bmmmmm].
    x |= x86_get_force_evex3_mask_in_last_bit(options);     // [x0000000|00L00000|WvvvvLpp|R0Bmmmmm].

    // Check if VEX3 is required / forced:                     [x.......|........|x.......|..xxxxx.].
    if (x & 0x8000803Eu) {
      uint32_t xor_mask = vex_prefix_table[x & 0xF] | (opcode << 24);

      // Clear all high bits.
      x  = (x & 0xFFFF) << 8;                               // [00000000|WvvvvLpp|R0Bmmmmm|00000000].
                                                            //            ____    _ _
      x ^= xor_mask;                                        // [_OPCODE_|WvvvvLpp|R1Bmmmmm|VEX3|XOP].
      writer.emit32u_le(x);

      rb_reg &= 0x7;
      writer.emit8(encode_mod(3, op_reg, rb_reg));
      writer.emit_imm_byte_or_dword(uint64_t(imm_value), imm_size);
      goto EmitDone;
    }
    else {
      // 'mmmmm' must be '00001'.
      ASMJIT_ASSERT((x & 0x1F) == 0x01);

      x = ((x >> 8) ^ x) ^ 0xF9;
      writer.emit8(kX86ByteVex2);
      writer.emit8(x);
      writer.emit8(opcode.v);

      rb_reg &= 0x7;
      writer.emit8(encode_mod(3, op_reg, rb_reg));
      writer.emit_imm_byte_or_dword(uint64_t(imm_value), imm_size);
      goto EmitDone;
    }
  }

  // Emit - VEX|EVEX - /r - Memory
  // -----------------------------

EmitVexEvexM:
  ASMJIT_ASSERT(rm_rel != nullptr);
  ASMJIT_ASSERT(rm_rel->op_type() == OperandType::kMem);

  rm_info = mem_info_table[rm_rel->as<Mem>().base_and_index_types()];
  writer.emit_segment_override(rm_rel->as<Mem>().segment_id());

  mem_op_ao_mark = writer.cursor();
  writer.emit_address_override((rm_info & _address_override_mask()) != 0);

  rb_reg = rm_rel->as<Mem>().has_base_reg()  ? rm_rel->as<Mem>().base_id()  : uint32_t(0);
  rx_reg = rm_rel->as<Mem>().has_index_reg() ? rm_rel->as<Mem>().index_id() : uint32_t(0);

  {
    uint32_t broadcast_bit = uint32_t(rm_rel->as<Mem>().has_broadcast());

    // Construct `x` - a complete EVEX|VEX prefix.
    uint32_t x = ((op_reg <<  4) & 0x0000F980u)  |          // [........|........|Vvvvv..R|R.......].
                 ((rx_reg <<  3) & 0x00000040u)  |          // [........|........|........|.X......].
                 ((rx_reg << 15) & 0x00080000u)  |          // [........|....X...|........|........].
                 ((rb_reg <<  2) & 0x00000020u)  |          // [........|........|........|..B.....].
                 opcode.extract_ll_mmmmm(options) |         // [........|.LL.X...|Vvvvv..R|RXBmmmmm].
                 (_extra_reg.id()    << 16)      |          // [........|.LL.Xaaa|Vvvvv..R|RXBmmmmm].
                 (broadcast_bit      << 20)      ;          // [........|.LLbXaaa|Vvvvv..R|RXBmmmmm].
    op_reg &= 0x07u;

    // Mark invalid VEX (force EVEX) case:                  // [@.......|.LLbXaaa|Vvvvv..R|RXBmmmmm].
    x |= uint32_t(~common_info->flags() & InstDB::InstFlags::kVex) << (31 - Support::ctz_const<InstDB::InstFlags::kVex>);

    // Handle AVX512 options by a single branch.
    const InstOptions kAvx512Options = InstOptions::kX86_ZMask   |
                                       InstOptions::kX86_ER      |
                                       InstOptions::kX86_SAE     ;
    if (Support::test(options, kAvx512Options)) {
      // {er} and {sae} are both invalid if memory operand is used.
      if (ASMJIT_UNLIKELY(Support::test(options, InstOptions::kX86_ER | InstOptions::kX86_SAE)))
        goto InvalidEROrSAE;

      x |= uint32_t(options & InstOptions::kX86_ZMask);     // [@.......|zLLbXaaa|Vvvvv..R|RXBmmmmm].
    }

    // If these bits are used then EVEX prefix is required.
    constexpr uint32_t kEvexForce = 0x00000010u;            // [........|........|........|...x....].
    constexpr uint32_t kEvexBits = 0x80DF8110u;             // [@.......|xx.xxxxx|x......x|...x....].

    // Force EVEX prefix even in case the instruction has VEX encoding, because EVEX encoding is preferred. At the
    // moment this is only required for AVX_VNNI instructions, which were added after AVX512_VNNI instructions. If
    // such instruction doesn't specify prefix, EVEX (AVX512_VNNI) would be used by default,
    if (common_info->prefer_evex()) {
      if ((x & kEvexBits) == 0 && !Support::test(options, InstOptions::kX86_Vex | InstOptions::kX86_Vex3)) {
        x |= kEvexForce;
      }
    }

    // Check if EVEX is required by checking bits in `x` :     [@.......|xx.xxxxx|x......x|...x....].
    if (x & kEvexBits) {
      uint32_t y = ((x << 4) & 0x00080000u) |               // [@.......|....V...|........|........].
                   ((x >> 4) & 0x00000010u) ;               // [@.......|....V...|........|...R....].
      x  = (x & 0x00FF78EFu) | y;                           // [........|zLLbVaaa|0vvvv000|RXBRmmmm].
      x  = x << 8;                                          // [zLLbVaaa|0vvvv000|RBBRmmmm|00000000].
      x |= (opcode >> kVSHR_W    ) & 0x00800000u;           // [zLLbVaaa|Wvvvv000|RBBRmmmm|00000000].
      x |= (opcode >> kVSHR_PP_EW) & 0x00830000u;           // [zLLbVaaa|Wvvvv0pp|RBBRmmmm|00000000] (added PP and EVEX.W).
                                                            //      _     ____    ____
      x ^= 0x087CF000u | kX86ByteEvex;                      // [zLLbVaaa|Wvvvv1pp|RBBRmmmm|01100010].

      if (x & 0x10000000u) {
        // Broadcast support.
        //
        // 1. Verify our LL field is correct as broadcast changes the "size" of the source operand. For example if
        //    a broadcasted operand is qword_ptr[X] {1to8} the source size becomes 64 and not 8 as the memory operand
        //    would report.
        //
        // 2. Change the compressed displacement scale to either x2 (SHL1), x4 (SHL 2), or x8 (SHL 3) depending on
        //    the broadcast unit/element size.
        uint32_t broadcast_unit_size = common_info->broadcast_size();
        uint32_t broadcast_vector_size = broadcast_unit_size << uint32_t(rm_rel->as<Mem>().get_broadcast());

        if (ASMJIT_UNLIKELY(broadcast_unit_size == 0))
          goto InvalidBroadcast;

        // LL was already shifted 8 bits right.
        constexpr uint32_t kLLShift = 21 + 8;

        uint32_t current_ll = x & (0x3u << kLLShift);
        uint32_t broadcast_ll = (Support::max<uint32_t>(Support::ctz(broadcast_vector_size), 4) - 4) << kLLShift;

        if (broadcast_ll > (2u << kLLShift))
          goto InvalidBroadcast;

        uint32_t new_ll = Support::max(current_ll, broadcast_ll);
        x = (x & ~(uint32_t(0x3) << kLLShift)) | new_ll;

        opcode &=~uint32_t(Opcode::kCDSHL_Mask);
        opcode |= Support::ctz(broadcast_unit_size) << Opcode::kCDSHL_Shift;
      }
      else {
        // Add the compressed displacement 'SHF' to the opcode based on 'TTWLL'.
        // The index to `cdisp8_shl_table` is composed as `CDTT[4:3] | W[2] | LL[1:0]`.
        uint32_t TTWLL = ((opcode >> (Opcode::kCDTT_Shift - 3)) & 0x18) +
                         ((opcode >> (Opcode::kW_Shift    - 2)) & 0x04) +
                         ((x >> 29) & 0x3);
        opcode += cdisp8_shl_table[TTWLL];
      }

      writer.emit32u_le(x);
      writer.emit8(opcode.v);
    }
    else {
      // Not EVEX, prepare `x` for VEX2 or VEX3:           x = [........|00L00000|0vvvv000|RXBmmmmm].
      x |= ((opcode >> (kVSHR_W  + 8)) & 0x8000u) |         // [00000000|00L00000|Wvvvv000|RXBmmmmm].
           ((opcode >> (kVSHR_PP + 8)) & 0x0300u) |         // [00000000|00L00000|Wvvvv0pp|RXBmmmmm].
           ((x      >> 11            ) & 0x0400u) ;         // [00000000|00L00000|WvvvvLpp|RXBmmmmm].
      x |= x86_get_force_evex3_mask_in_last_bit(options);   // [x0000000|00L00000|WvvvvLpp|RXBmmmmm].

      // Clear a possible CDisp specified by EVEX.
      opcode &= ~Opcode::kCDSHL_Mask;

      // Check if VEX3 is required / forced:                   [x.......|........|x.......|.xxxxxx.].
      if (x & 0x8000807Eu) {
        uint32_t xor_mask = vex_prefix_table[x & 0xF] | (opcode << 24);

        // Clear all high bits.
        x  = (x & 0xFFFF) << 8;                             // [00000000|WvvvvLpp|RXBmmmmm|00000000].
                                                            //            ____    ___
        x ^= xor_mask;                                      // [_OPCODE_|WvvvvLpp|RXBmmmmm|VEX3_XOP].
        writer.emit32u_le(x);
      }
      else {
        // 'mmmmm' must be '00001'.
        ASMJIT_ASSERT((x & 0x1F) == 0x01);

        x = ((x >> 8) ^ x) ^ 0xF9;
        writer.emit8(kX86ByteVex2);
        writer.emit8(x);
        writer.emit8(opcode.v);
      }
    }
  }

  // MOD|SIB address.
  if (!common_info->has_flag(InstDB::InstFlags::kVsib))
    goto EmitModSib;

  // MOD|VSIB address without INDEX is invalid.
  if (rm_info & kX86MemInfo_Index)
    goto EmitModVSib;
  goto InvalidInstruction;

  // Emit - Jmp/Jcc/Call
  // -------------------

EmitJmpCall:
  {
    // Emit REX prefix if asked for (64-bit only).
    uint32_t rex = opcode.extract_rex(options);
    if (ASMJIT_UNLIKELY(is_rex_invalid(rex)))
      goto InvalidRexPrefix;
    rex &= ~kX86ByteInvalidRex & 0xFF;
    writer.emit8_if(rex | kX86ByteRex, rex != 0);

    uint64_t ip = uint64_t(writer.offset_from(_buffer_data));
    uint32_t rel32 = 0;
    uint32_t opcode8 = alt_opcode_of(inst_info);

    uint32_t inst8_size  = 1 + 1; //          OPCODE + REL8 .
    uint32_t inst32_size = 1 + 4; // [PREFIX] OPCODE + REL32.

    // Jcc instructions with 32-bit displacement use 0x0F prefix,
    // other instructions don't. No other prefixes are used by X86.
    ASMJIT_ASSERT((opcode8 & Opcode::kMM_Mask) == 0);
    ASMJIT_ASSERT((opcode  & Opcode::kMM_Mask) == 0 ||
                  (opcode  & Opcode::kMM_Mask) == Opcode::kMM_0F);

    // Only one of these should be used at the same time.
    inst32_size += uint32_t(op_reg != 0);
    inst32_size += uint32_t((opcode & Opcode::kMM_Mask) == Opcode::kMM_0F);

    if (rm_rel->is_label()) {
      uint32_t label_id = rm_rel->as<Label>().id();
      if (ASMJIT_UNLIKELY(!_code->is_label_valid(label_id))) {
        goto InvalidLabel;
      }

      label = &_code->label_entry_of(label_id);
      if (label->is_bound_to(_section)) {
        // Label bound to the current section.
        rel32 = uint32_t((label->offset() - ip - inst32_size) & 0xFFFFFFFFu);
        goto EmitJmpCallRel;
      }
      else {
        // Non-bound label or label bound to a different section.
        if (opcode8 && (!opcode.v || Support::test(options, InstOptions::kShortForm))) {
          writer.emit8(opcode8);

          // Record DISP8 (non-bound label).
          rel_offset = -1;
          rel_size = 1;
          goto EmitRel;
        }
        else {
          // Refuse also 'short' prefix, if specified.
          if (ASMJIT_UNLIKELY(!opcode.v || Support::test(options, InstOptions::kShortForm)))
            goto InvalidDisplacement;

          writer.emit8_if(0x0F, (opcode & Opcode::kMM_Mask) != 0);// Emit 0F prefix.
          writer.emit8(opcode.v);                                // Emit opcode.
          writer.emit8_if(encode_mod(3, op_reg, 0), op_reg != 0); // Emit MOD.

          // Record DISP32 (non-bound label).
          rel_offset = -4;
          rel_size = 4;
          goto EmitRel;
        }
      }
    }

    if (rm_rel->is_imm()) {
      uint64_t base_address = code()->base_address();
      uint64_t jump_address = rm_rel->as<Imm>().value_as<uint64_t>();

      // If the base-address is known calculate a relative displacement and check if it fits in 32 bits (which is
      // always true in 32-bit mode). Emit relative displacement as it was a bound label if all checks are ok.
      if (base_address != Globals::kNoBaseAddress) {
        uint64_t rel64 = jump_address - (ip + base_address) - inst32_size;
        if (Environment::is_32bit(arch()) || Support::is_int_n<32>(int64_t(rel64))) {
          rel32 = uint32_t(rel64 & 0xFFFFFFFFu);
          goto EmitJmpCallRel;
        }
        else {
          // Relative displacement exceeds 32-bits - relocator can only insert trampoline for jmp/call, but not
          // for jcc/jecxz.
          if (ASMJIT_UNLIKELY(!is_jmp_or_call(inst_id)))
            goto InvalidDisplacement;
        }
      }

      err = _code->new_reloc_entry(Out(re), RelocType::kAbsToRel);
      if (ASMJIT_UNLIKELY(err != Error::kOk))
        goto Failed;

      re->_source_offset = offset();
      re->_source_section_id = _section->section_id();
      re->_payload = jump_address;

      if (ASMJIT_LIKELY(opcode.v)) {
        // 64-bit: Emit REX prefix so the instruction can be patched later. REX prefix does nothing if not patched,
        // but allows to patch the instruction to use MOD/M and to point to a memory where the final 64-bit address
        // is stored.
        if (Environment::is_64bit(arch()) && is_jmp_or_call(inst_id)) {
          if (!rex)
            writer.emit8(kX86ByteRex);

          err = _code->add_address_to_address_table(jump_address);
          if (ASMJIT_UNLIKELY(err != Error::kOk))
            goto Failed;

          re->_reloc_type = RelocType::kX64AddressEntry;
        }

        writer.emit8_if(0x0F, (opcode & Opcode::kMM_Mask) != 0);  // Emit 0F prefix.
        writer.emit8(opcode.v);                                  // Emit opcode.
        writer.emit8_if(encode_mod(3, op_reg, 0), op_reg != 0);   // Emit MOD.
        re->_format.reset_to_simple_value(OffsetType::kSignedOffset, 4);
        re->_format.set_leading_and_trailing_size(writer.offset_from(_buffer_ptr), imm_size);
        writer.emit32u_le(0);                                     // Emit DISP32.
      }
      else {
        writer.emit8(opcode8);                                   // Emit opcode.
        re->_format.reset_to_simple_value(OffsetType::kSignedOffset, 1);
        re->_format.set_leading_and_trailing_size(writer.offset_from(_buffer_ptr), imm_size);
        writer.emit8(0);                                         // Emit DISP8 (zero).
      }
      goto EmitDone;
    }

    // Not Label|Imm -> Invalid.
    goto InvalidInstruction;

    // Emit jmp/call with relative displacement known at assembly-time. Decide between 8-bit and 32-bit displacement
    // encoding. Some instructions only allow either 8-bit or 32-bit encoding, others allow both encodings.
EmitJmpCallRel:
    if (Support::is_int_n<8>(int32_t(rel32 + inst32_size - inst8_size)) && opcode8 && !Support::test(options, InstOptions::kLongForm)) {
      options |= InstOptions::kShortForm;
      writer.emit8(opcode8);                                     // Emit opcode
      writer.emit8(rel32 + inst32_size - inst8_size);              // Emit DISP8.
      goto EmitDone;
    }
    else {
      if (ASMJIT_UNLIKELY(!opcode.v || Support::test(options, InstOptions::kShortForm)))
        goto InvalidDisplacement;

      options &= ~InstOptions::kShortForm;
      writer.emit8_if(0x0F, (opcode & Opcode::kMM_Mask) != 0);    // Emit 0x0F prefix.
      writer.emit8(opcode.v);                                    // Emit Opcode.
      writer.emit8_if(encode_mod(3, op_reg, 0), op_reg != 0);     // Emit MOD.
      writer.emit32u_le(rel32);                                   // Emit DISP32.
      goto EmitDone;
    }
  }

  // Emit - Relative
  // ---------------

EmitRel:
  {
    ASMJIT_ASSERT(rel_size == 1 || rel_size == 4);

    // Chain with label.
    size_t offset = size_t(writer.offset_from(_buffer_data));
    OffsetFormat of;
    of.reset_to_simple_value(OffsetType::kSignedOffset, rel_size);

    Fixup* fixup = _code->new_fixup(*label, _section->section_id(), offset, rel_offset, of);
    if (ASMJIT_UNLIKELY(!fixup)) {
      goto OutOfMemory;
    }

    if (re) {
      fixup->label_or_reloc_id = re->id();
    }

    // Emit dummy zeros, must be patched later when the reference becomes known.
    writer.emit_zeros(rel_size);
  }
  writer.emit_immediate(uint64_t(imm_value), imm_size);

  // Emit - Done
  // -----------

EmitDone:
  if (Support::test(options, InstOptions::kReserved)) {
#ifndef ASMJIT_NO_LOGGING
    if (_logger)
      EmitterUtils::log_instruction_emitted(this, inst_id, options, o0, o1, o2, op_ext, rel_size, imm_size, writer.cursor());
#endif
  }

  reset_state();
  writer.done(this);
  return Error::kOk;

  // Error Handler
  // -------------

#define ERROR_HANDLER(ERR) ERR: err = make_error(Error::k##ERR); goto Failed;
  ERROR_HANDLER(OutOfMemory)
  ERROR_HANDLER(InvalidLabel)
  ERROR_HANDLER(InvalidInstruction)
  ERROR_HANDLER(InvalidLockPrefix)
  ERROR_HANDLER(InvalidXAcquirePrefix)
  ERROR_HANDLER(InvalidXReleasePrefix)
  ERROR_HANDLER(InvalidRepPrefix)
  ERROR_HANDLER(InvalidRexPrefix)
  ERROR_HANDLER(InvalidEROrSAE)
  ERROR_HANDLER(InvalidAddress)
  ERROR_HANDLER(InvalidAddressIndex)
  ERROR_HANDLER(InvalidAddress64Bit)
  ERROR_HANDLER(InvalidDisplacement)
  ERROR_HANDLER(InvalidPhysId)
  ERROR_HANDLER(InvalidSegment)
  ERROR_HANDLER(InvalidImmediate)
  ERROR_HANDLER(InvalidBroadcast)
  ERROR_HANDLER(OperandSizeMismatch)
  ERROR_HANDLER(AmbiguousOperandSize)
#undef ERROR_HANDLER

Failed:
#ifndef ASMJIT_NO_LOGGING
  return EmitterUtils::log_instruction_failed(this, err, inst_id, options, o0, o1, o2, op_ext);
#else
  reset_state();
  return report_error(err);
#endif
}

//x86::Assembler - Align
// =====================

Error Assembler::align(AlignMode align_mode, uint32_t alignment) {
  if (ASMJIT_UNLIKELY(!_code))
    return report_error(make_error(Error::kNotInitialized));

  if (ASMJIT_UNLIKELY(uint32_t(align_mode) > uint32_t(AlignMode::kMaxValue)))
    return report_error(make_error(Error::kInvalidArgument));

  if (alignment <= 1)
    return Error::kOk;

  if (ASMJIT_UNLIKELY(!Support::is_power_of_2_up_to(alignment, Globals::kMaxAlignment)))
    return report_error(make_error(Error::kInvalidArgument));

  uint32_t i = uint32_t(Support::align_up_diff<size_t>(offset(), alignment));
  if (i > 0) {
    CodeWriter writer(this);
    ASMJIT_PROPAGATE(writer.ensure_space(this, i));

    uint8_t pattern = 0x00;
    switch (align_mode) {
      case AlignMode::kCode: {
        if (has_encoding_option(EncodingOptions::kOptimizedAlign)) {
          // Intel 64 and IA-32 Architectures Software Developer's Manual - Volume 2B (NOP).
          static constexpr uint32_t kMaxNop_size = 9;

          static const uint8_t nop_table[kMaxNop_size][kMaxNop_size] = {
            { 0x90 },
            { 0x66, 0x90 },
            { 0x0F, 0x1F, 0x00 },
            { 0x0F, 0x1F, 0x40, 0x00 },
            { 0x0F, 0x1F, 0x44, 0x00, 0x00 },
            { 0x66, 0x0F, 0x1F, 0x44, 0x00, 0x00 },
            { 0x0F, 0x1F, 0x80, 0x00, 0x00, 0x00, 0x00 },
            { 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00 },
            { 0x66, 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00 }
          };

          do {
            uint32_t n = Support::min<uint32_t>(i, kMaxNop_size);
            const uint8_t* src = nop_table[n - 1];

            i -= n;
            do {
              writer.emit8(*src++);
            } while (--n);
          } while (i);
        }

        pattern = 0x90;
        break;
      }

      case AlignMode::kData:
        pattern = 0xCC;
        break;

      case AlignMode::kZero:
        // Pattern already set to zero.
        break;
    }

    while (i) {
      writer.emit8(pattern);
      i--;
    }

    writer.done(this);
  }

#ifndef ASMJIT_NO_LOGGING
  if (_logger) {
    StringTmp<128> sb;
    sb.append_chars(' ', _logger->indentation(FormatIndentationGroup::kCode));
    sb.append_format("align %u\n", alignment);
    _logger->log(sb);
  }
#endif

  return Error::kOk;
}

// x86::Assembler - Events
// =======================

Error Assembler::on_attach(CodeHolder& code) noexcept {
  Arch arch = code.arch();
  ASMJIT_PROPAGATE(Base::on_attach(code));

  _instruction_alignment = uint8_t(1);
  update_emitter_funcs(this);

  if (Environment::is_32bit(arch)) {
    // 32 bit architecture - X86.
    _forced_inst_options |= InstOptions::kX86_InvalidRex;
    _set_address_override_mask(kX86MemInfo_67H_X86);
  }
  else {
    // 64 bit architecture - X64.
    _forced_inst_options &= ~InstOptions::kX86_InvalidRex;
    _set_address_override_mask(kX86MemInfo_67H_X64);
  }

  return Error::kOk;
}

Error Assembler::on_detach(CodeHolder& code) noexcept {
  _forced_inst_options &= ~InstOptions::kX86_InvalidRex;
  _set_address_override_mask(0);
  return Base::on_detach(code);
}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_X86
