// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#if !defined(ASMJIT_NO_AARCH64)

#include <asmjit/core/codewriter_p.h>
#include <asmjit/core/cpuinfo.h>
#include <asmjit/core/emitterutils_p.h>
#include <asmjit/core/formatter.h>
#include <asmjit/core/logger.h>
#include <asmjit/core/misc_p.h>
#include <asmjit/support/support.h>
#include <asmjit/arm/armformatter_p.h>
#include <asmjit/arm/armutils.h>
#include <asmjit/arm/a64assembler.h>
#include <asmjit/arm/a64emithelper_p.h>
#include <asmjit/arm/a64instdb_p.h>

ASMJIT_BEGIN_SUB_NAMESPACE(a64)

// a64::Assembler - Utils
// ======================

static ASMJIT_INLINE_CONSTEXPR uint32_t diff(RegType a, RegType b) noexcept { return uint32_t(a) - uint32_t(b); }
static ASMJIT_INLINE_CONSTEXPR uint32_t diff(VecElementType element_type, VecElementType base_type) noexcept { return uint32_t(element_type) - uint32_t(base_type); }

// a64::Assembler - Cond
// =====================

static inline uint32_t cond_code_to_opcode_field(uint32_t cond) noexcept { return (uint32_t(cond) - 2u) & 0xFu; }

// a64::Assembler - Bits
// =====================

template<typename T>
static inline constexpr uint32_t B(const T& index) noexcept { return uint32_t(1u) << uint32_t(index); }

static constexpr uint32_t kSP = Gp::kIdSp;
static constexpr uint32_t kZR = Gp::kIdZr;
static constexpr uint32_t kWX = InstDB::kWX;

// a64::Assembler - ShiftOpToLdStOptMap
// ====================================

// Table that maps ShiftOp to OPT part in LD/ST (register) opcode.
#define VALUE(index) index == uint32_t(ShiftOp::kUXTW) ? 2u : \
                     index == uint32_t(ShiftOp::kLSL)  ? 3u : \
                     index == uint32_t(ShiftOp::kSXTW) ? 6u : \
                     index == uint32_t(ShiftOp::kSXTX) ? 7u : 0xFF
static const uint8_t shift_op_to_ld_st_opt_map[] = { ASMJIT_LOOKUP_TABLE_16(VALUE, 0) };
#undef VALUE

// a64::Assembler - ExtendOpToRegType
// ==================================

static inline RegType extend_option_to_reg_type(uint32_t option) noexcept {
  uint32_t pred = (uint32_t(RegType::kGp32) << (0x0 * 4)) | // 0b000 - UXTB.
                  (uint32_t(RegType::kGp32) << (0x1 * 4)) | // 0b001 - UXTH.
                  (uint32_t(RegType::kGp32) << (0x2 * 4)) | // 0b010 - UXTW.
                  (uint32_t(RegType::kGp64) << (0x3 * 4)) | // 0b011 - UXTX|LSL.
                  (uint32_t(RegType::kGp32) << (0x4 * 4)) | // 0b100 - SXTB.
                  (uint32_t(RegType::kGp32) << (0x5 * 4)) | // 0b101 - SXTH.
                  (uint32_t(RegType::kGp32) << (0x6 * 4)) | // 0b110 - SXTW.
                  (uint32_t(RegType::kGp64) << (0x7 * 4)) ; // 0b111 - SXTX.
  return RegType((pred >> (option * 4u)) & 0xFu);
}

// asmjit::a64::Assembler - SizeOp
// ===============================

//! Struct that contains Size (2 bits), Q flag, and S (scalar) flag. These values
//! are used to encode Q, Size, and Scalar fields in an opcode.
struct SizeOp {
  //! \name Constants
  //! \{

  static inline constexpr uint8_t k128BitShift = 0;
  static inline constexpr uint8_t kScalarShift = 1;
  static inline constexpr uint8_t kSizeShift = 2;

  static inline constexpr uint8_t kQ = uint8_t(1u << k128BitShift);
  static inline constexpr uint8_t kS = uint8_t(1u << kScalarShift);

  static inline constexpr uint8_t k00 = uint8_t(0 << kSizeShift);
  static inline constexpr uint8_t k01 = uint8_t(1 << kSizeShift);
  static inline constexpr uint8_t k10 = uint8_t(2 << kSizeShift);
  static inline constexpr uint8_t k11 = uint8_t(3 << kSizeShift);

  static inline constexpr uint8_t k00Q = k00 | kQ;
  static inline constexpr uint8_t k01Q = k01 | kQ;
  static inline constexpr uint8_t k10Q = k10 | kQ;
  static inline constexpr uint8_t k11Q = k11 | kQ;

  static inline constexpr uint8_t k00S = k00 | kS;
  static inline constexpr uint8_t k01S = k01 | kS;
  static inline constexpr uint8_t k10S = k10 | kS;
  static inline constexpr uint8_t k11S = k11 | kS;

  static inline constexpr uint8_t kInvalid = 0xFFu;

  // Masks used by SizeOpMap.
  static inline constexpr uint8_t kSzQ = (0x3u << kSizeShift) | kQ;
  static inline constexpr uint8_t kSzS = (0x3u << kSizeShift) | kS;
  static inline constexpr uint8_t kSzQS = (0x3u << kSizeShift) | kQ | kS;

  //! \}

  //! \name Members
  //! \{

  uint8_t value;

  //! \}

  //! \name Accessors
  //! \{

  inline bool is_valid() const noexcept { return value != kInvalid; }
  inline void make_invalid() noexcept { value = kInvalid; }

  inline uint32_t q() const noexcept { return (value >> k128BitShift) & 0x1u; }
  inline uint32_t qs() const noexcept { return ((value >> k128BitShift) | (value >> kScalarShift)) & 0x1u; }
  inline uint32_t scalar() const noexcept { return (value >> kScalarShift) & 0x1u; }
  inline uint32_t size() const noexcept { return (value >> kSizeShift) & 0x3u; }

  inline void decrement_size() noexcept {
    ASMJIT_ASSERT(size() > 0);
    value = uint8_t(value - (1u << kSizeShift));
  }

  //! \}
};

struct SizeOpTable {
  enum TableId : uint8_t {
    kTableBin = 0,
    kTableAny,
    kCount
  };

  // 40 elements for each combination.
  SizeOp array[(uint32_t(RegType::kVec128) - uint32_t(RegType::kVec8) + 1) * 8];
};

#define VALUE_BIN(x) { \
  x == (((uint32_t(RegType::kVec64 ) - uint32_t(RegType::kVec8)) << 3) | uint32_t(VecElementType::kNone)) ? SizeOp::k00  : \
  x == (((uint32_t(RegType::kVec128) - uint32_t(RegType::kVec8)) << 3) | uint32_t(VecElementType::kNone)) ? SizeOp::k00Q : \
  x == (((uint32_t(RegType::kVec64 ) - uint32_t(RegType::kVec8)) << 3) | uint32_t(VecElementType::kB   )) ? SizeOp::k00  : \
  x == (((uint32_t(RegType::kVec128) - uint32_t(RegType::kVec8)) << 3) | uint32_t(VecElementType::kB   )) ? SizeOp::k00Q : SizeOp::kInvalid \
}

#define VALUE_ANY(x) { \
  x == (((uint32_t(RegType::kVec8)   - uint32_t(RegType::kVec8)) << 3) | uint32_t(VecElementType::kNone)) ? SizeOp::k00S : \
  x == (((uint32_t(RegType::kVec16)  - uint32_t(RegType::kVec8)) << 3) | uint32_t(VecElementType::kNone)) ? SizeOp::k01S : \
  x == (((uint32_t(RegType::kVec32)  - uint32_t(RegType::kVec8)) << 3) | uint32_t(VecElementType::kNone)) ? SizeOp::k10S : \
  x == (((uint32_t(RegType::kVec64)  - uint32_t(RegType::kVec8)) << 3) | uint32_t(VecElementType::kNone)) ? SizeOp::k11S : \
  x == (((uint32_t(RegType::kVec64)  - uint32_t(RegType::kVec8)) << 3) | uint32_t(VecElementType::kB   )) ? SizeOp::k00  : \
  x == (((uint32_t(RegType::kVec128) - uint32_t(RegType::kVec8)) << 3) | uint32_t(VecElementType::kB   )) ? SizeOp::k00Q : \
  x == (((uint32_t(RegType::kVec64)  - uint32_t(RegType::kVec8)) << 3) | uint32_t(VecElementType::kH   )) ? SizeOp::k01  : \
  x == (((uint32_t(RegType::kVec128) - uint32_t(RegType::kVec8)) << 3) | uint32_t(VecElementType::kH   )) ? SizeOp::k01Q : \
  x == (((uint32_t(RegType::kVec64)  - uint32_t(RegType::kVec8)) << 3) | uint32_t(VecElementType::kS   )) ? SizeOp::k10  : \
  x == (((uint32_t(RegType::kVec128) - uint32_t(RegType::kVec8)) << 3) | uint32_t(VecElementType::kS   )) ? SizeOp::k10Q : \
  x == (((uint32_t(RegType::kVec64)  - uint32_t(RegType::kVec8)) << 3) | uint32_t(VecElementType::kD   )) ? SizeOp::k11S : \
  x == (((uint32_t(RegType::kVec128) - uint32_t(RegType::kVec8)) << 3) | uint32_t(VecElementType::kD   )) ? SizeOp::k11Q : SizeOp::kInvalid \
}

static const SizeOpTable size_op_table[SizeOpTable::kCount] = {
  {{ ASMJIT_LOOKUP_TABLE_40(VALUE_BIN, 0) }},
  {{ ASMJIT_LOOKUP_TABLE_40(VALUE_ANY, 0) }}
};

#undef VALUE_ANY
#undef VALUE_BIN

struct SizeOpMap {
  uint8_t table_id;
  uint8_t size_op_mask;
  uint16_t accept_mask;
};

static const constexpr SizeOpMap size_op_map[InstDB::kVO_Count] = {
  { // kVO_V_B:
    SizeOpTable::kTableBin, SizeOp::kQ   , uint16_t(B(SizeOp::k00) | B(SizeOp::k00Q))
  },

  { // kVO_V_BH:
    SizeOpTable::kTableAny, SizeOp::kSzQS, uint16_t(B(SizeOp::k00) | B(SizeOp::k00Q) | B(SizeOp::k01) | B(SizeOp::k01Q))
  },

  { // kVO_V_BH_4S:
    SizeOpTable::kTableAny, SizeOp::kSzQS, uint16_t(B(SizeOp::k00) | B(SizeOp::k00Q) | B(SizeOp::k01) | B(SizeOp::k01Q) | B(SizeOp::k10Q))
  },

  { // kVO_V_BHS:
    SizeOpTable::kTableAny, SizeOp::kSzQS, uint16_t(B(SizeOp::k00) | B(SizeOp::k00Q) | B(SizeOp::k01) | B(SizeOp::k01Q) | B(SizeOp::k10) | B(SizeOp::k10Q))
  },

  { // kVO_V_BHS_D2:
    SizeOpTable::kTableAny, SizeOp::kSzQS, uint16_t(B(SizeOp::k00) | B(SizeOp::k00Q) | B(SizeOp::k01) | B(SizeOp::k01Q) | B(SizeOp::k10) | B(SizeOp::k10Q) | B(SizeOp::k11Q))
  },

  { // kVO_V_HS:
    SizeOpTable::kTableAny, SizeOp::kSzQS, uint16_t(B(SizeOp::k01) | B(SizeOp::k01Q) | B(SizeOp::k10) | B(SizeOp::k10Q))
  },

  { // kVO_V_S:
    SizeOpTable::kTableAny, SizeOp::kQ   , uint16_t(B(SizeOp::k10) | B(SizeOp::k10Q))
  },

  { // kVO_V_B8H4:
    SizeOpTable::kTableAny, SizeOp::kSzQS, uint16_t(B(SizeOp::k00) | B(SizeOp::k01))
  },

  { // kVO_V_B8H4S2:
    SizeOpTable::kTableAny, SizeOp::kSzQS, uint16_t(B(SizeOp::k00) | B(SizeOp::k01) | B(SizeOp::k10))
  },

  { // kVO_V_B8D1:
    SizeOpTable::kTableAny, SizeOp::kSzQ , uint16_t(B(SizeOp::k00) | B(SizeOp::k11S))
  },

  { // kVO_V_H4S2:
    SizeOpTable::kTableAny, SizeOp::kSzQS, uint16_t(B(SizeOp::k01) | B(SizeOp::k10))
  },

  { // kVO_V_B16:
    SizeOpTable::kTableBin, SizeOp::kQ   , uint16_t(B(SizeOp::k00Q))
  },

  { // kVO_V_B16H8:
    SizeOpTable::kTableAny, SizeOp::kSzQS, uint16_t(B(SizeOp::k00Q) | B(SizeOp::k01Q))
  },

  { // kVO_V_B16H8S4:
    SizeOpTable::kTableAny, SizeOp::kSzQS, uint16_t(B(SizeOp::k00Q) | B(SizeOp::k01Q) | B(SizeOp::k10Q))
  },

  { // kVO_V_B16D2:
    SizeOpTable::kTableAny, SizeOp::kSzQS, uint16_t(B(SizeOp::k00Q) | B(SizeOp::k11Q))
  },

  { // kVO_V_H8S4:
    SizeOpTable::kTableAny, SizeOp::kSzQS, uint16_t(B(SizeOp::k01Q) | B(SizeOp::k10Q))
  },

  { // kVO_V_S4:
    SizeOpTable::kTableAny, 0            , uint16_t(B(SizeOp::k10Q))
  },

  { // kVO_V_D2:
    SizeOpTable::kTableAny, 0            , uint16_t(B(SizeOp::k11Q))
  },

  { // kVO_SV_BHS:
    SizeOpTable::kTableAny, SizeOp::kSzQS, uint16_t(B(SizeOp::k00) | B(SizeOp::k00Q) | B(SizeOp::k00S) | B(SizeOp::k01) | B(SizeOp::k01Q) | B(SizeOp::k01S) | B(SizeOp::k10) | B(SizeOp::k10Q) | B(SizeOp::k10S))
  },

  { // kVO_SV_B8H4S2:
    SizeOpTable::kTableAny, SizeOp::kSzQS, uint16_t(B(SizeOp::k00) | B(SizeOp::k00S) | B(SizeOp::k01) | B(SizeOp::k01S) | B(SizeOp::k10) | B(SizeOp::k10S))
  },

  { // kVO_SV_HS:
    SizeOpTable::kTableAny, SizeOp::kSzQS, uint16_t(B(SizeOp::k01) | B(SizeOp::k01Q) | B(SizeOp::k01S) | B(SizeOp::k10) | B(SizeOp::k10Q) | B(SizeOp::k10S))
  },

  { // kVO_V_Any:
    SizeOpTable::kTableAny, SizeOp::kSzQS, uint16_t(B(SizeOp::k00) | B(SizeOp::k00Q) | B(SizeOp::k01) | B(SizeOp::k01Q) | B(SizeOp::k10) | B(SizeOp::k10Q) | B(SizeOp::k11S) | B(SizeOp::k11Q))
  },

  { // kVO_SV_Any:
    SizeOpTable::kTableAny, SizeOp::kSzQS, uint16_t(B(SizeOp::k00) | B(SizeOp::k00Q) | B(SizeOp::k00S) |
                                                    B(SizeOp::k01) | B(SizeOp::k01Q) | B(SizeOp::k01S) |
                                                    B(SizeOp::k10) | B(SizeOp::k10Q) | B(SizeOp::k10S) |
                                                    B(SizeOp::k11) | B(SizeOp::k11Q) | B(SizeOp::k11S))
  }
};

static const Operand_& significant_simd_op(const Operand_& o0, const Operand_& o1, uint32_t inst_flags) noexcept {
  return !(inst_flags & InstDB::kInstFlagLong) ? o0 : o1;
}

static inline SizeOp element_type_to_size_op(uint32_t vec_op_type, RegType reg_type, VecElementType element_type) noexcept {
  // Instruction data or Assembler is wrong if this triggers an assertion failure.
  ASMJIT_ASSERT(vec_op_type < InstDB::kVO_Count);
  // ElementType uses 3 bits in the operand signature, it should never overflow.
  ASMJIT_ASSERT(uint32_t(element_type) <= 0x7u);

  const SizeOpMap& map = size_op_map[vec_op_type];
  const SizeOpTable& table = size_op_table[map.table_id];

  size_t index = (Support::min<uint32_t>(diff(reg_type, RegType::kVec8), diff(RegType::kVec128, RegType::kVec8) + 1) << 3) | uint32_t(element_type);
  SizeOp op = table.array[index];
  SizeOp modified_op { uint8_t(op.value & map.size_op_mask) };

  if (!Support::bit_test(map.accept_mask, op.value)) {
    modified_op.make_invalid();
  }

  return modified_op;
}

// a64::Assembler - Immediate Encoding Utilities (Integral)
// ========================================================

using Utils::LogicalImm;

struct HalfWordImm {
  uint32_t hw;
  uint32_t inv;
  uint32_t imm;
};

struct LMHImm {
  uint32_t lm;
  uint32_t h;
  uint32_t max_rm_id;
};

static inline uint32_t count_zero_half_words_64(uint64_t imm) noexcept {
  return uint32_t((imm & 0x000000000000FFFFu) == 0) +
         uint32_t((imm & 0x00000000FFFF0000u) == 0) +
         uint32_t((imm & 0x0000FFFF00000000u) == 0) +
         uint32_t((imm & 0xFFFF000000000000u) == 0) ;
}

static uint32_t encode_mov_sequence_32(uint32_t out[2], uint32_t imm, uint32_t rd, uint32_t x) noexcept {
  ASMJIT_ASSERT(rd <= 31);

  uint32_t kMovZ = 0b01010010100000000000000000000000 | (x << 31);
  uint32_t kMovN = 0b00010010100000000000000000000000;
  uint32_t kMovK = 0b01110010100000000000000000000000;

  if ((imm & 0xFFFF0000u) == 0x00000000u) {
    out[0] = kMovZ | (0 << 21) | ((imm & 0xFFFFu) << 5) | rd;
    return 1;
  }

  if ((imm & 0xFFFF0000u) == 0xFFFF0000u) {
    out[0] = kMovN | (0 << 21) | ((~imm & 0xFFFFu) << 5) | rd;
    return 1;
  }

  if ((imm & 0x0000FFFFu) == 0x00000000u) {
    out[0] = kMovZ | (1 << 21) | ((imm >> 16) << 5) | rd;
    return 1;
  }

  if ((imm & 0x0000FFFFu) == 0x0000FFFFu) {
    out[0] = kMovN | (1 << 21) | ((~imm >> 16) << 5) | rd;
    return 1;
  }

  out[0] = kMovZ | (0 << 21) | ((imm & 0xFFFFu) << 5) | rd;
  out[1] = kMovK | (1 << 21) | ((imm     >> 16) << 5) | rd;
  return 2;
}

static uint32_t encode_mov_sequence_64(uint32_t out[4], uint64_t imm, uint32_t rd, uint32_t x) noexcept {
  ASMJIT_ASSERT(rd <= 31);

  uint32_t kMovZ = 0b11010010100000000000000000000000;
  uint32_t kMovN = 0b10010010100000000000000000000000;
  uint32_t kMovK = 0b11110010100000000000000000000000;

  if (imm <= 0xFFFFFFFFu)
    return encode_mov_sequence_32(out, uint32_t(imm), rd, x);

  uint32_t zhw = count_zero_half_words_64( imm);
  uint32_t ohw = count_zero_half_words_64(~imm);

  if (zhw >= ohw) {
    uint32_t op = kMovZ;
    uint32_t count = 0;

    for (uint32_t hw_index = 0; hw_index < 4; hw_index++, imm >>= 16) {
      uint32_t hw_imm = uint32_t(imm & 0xFFFFu);
      if (hw_imm == 0) {
        continue;
      }

      out[count++] = op | (hw_index << 21) | (hw_imm << 5) | rd;
      op = kMovK;
    }

    // This should not happen - zero should be handled by encode_mov_sequence_32().
    ASMJIT_ASSERT(count > 0);

    return count;
  }
  else {
    uint32_t op = kMovN;
    uint32_t count = 0;
    uint32_t neg_mask = 0xFFFFu;

    for (uint32_t hw_index = 0; hw_index < 4; hw_index++, imm >>= 16) {
      uint32_t hw_imm = uint32_t(imm & 0xFFFFu);
      if (hw_imm == 0xFFFFu) {
        continue;
      }

      out[count++] = op | (hw_index << 21) | ((hw_imm ^ neg_mask) << 5) | rd;
      op = kMovK;
      neg_mask = 0;
    }

    if (count == 0) {
      out[count++] = kMovN | ((0xFFFF ^ neg_mask) << 5) | rd;
    }

    return count;
  }
}

static inline bool encode_lmh(uint32_t size_field, uint32_t element_index, Out<LMHImm> out) noexcept {
  if (size_field != 1 && size_field != 2)
    return false;

  uint32_t h_shift = 3u - size_field;
  uint32_t lm_shift = size_field - 1u;
  uint32_t max_element_index = 15u >> size_field;

  out->h = element_index >> h_shift;
  out->lm = (element_index << lm_shift) & 0x3u;
  out->max_rm_id = (8u << size_field) - 1;

  return element_index <= max_element_index;
}

// a64::Assembler - Opcode
// =======================

//! Helper class to store and manipulate ARM opcode.
struct Opcode {
  uint32_t v;

  enum Bits : uint32_t {
    kN = (1u << 22),
    kQ = (1u << 30),
    kX = (1u << 31)
  };

  // --------------------------------------------------------------------------
  // [Opcode Builder]
  // --------------------------------------------------------------------------

  inline uint32_t get() const noexcept { return v; }
  inline void reset(uint32_t value) noexcept { v = value; }

  inline bool has_q() const noexcept { return (v & kQ) != 0; }
  inline bool has_x() const noexcept { return (v & kX) != 0; }

  template<typename T>
  inline Opcode& add_imm(T value, uint32_t bit_index) noexcept { return operator|=(uint32_t(value) << bit_index); }

  template<typename T>
  inline Opcode& xor_imm(T value, uint32_t bit_index) noexcept { return operator^=(uint32_t(value) << bit_index); }

  template<typename T, typename Condition>
  inline Opcode& add_if(T value, const Condition& condition) noexcept { return operator|=(condition ? uint32_t(value) : uint32_t(0)); }

  inline Opcode& add_logical_imm(const LogicalImm& logical_imm) noexcept {
    add_imm(logical_imm.n, 22);
    add_imm(logical_imm.r, 16);
    add_imm(logical_imm.s, 10);
    return *this;
  }

  inline Opcode& add_reg(uint32_t id, uint32_t bit_index) noexcept { return operator|=((id & 31u) << bit_index); }
  inline Opcode& add_reg(const Operand_& op, uint32_t bit_index) noexcept { return add_reg(op.id(), bit_index); }

  inline Opcode& operator=(uint32_t x) noexcept { v = x; return *this; }
  inline Opcode& operator&=(uint32_t x) noexcept { v &= x; return *this; }
  inline Opcode& operator|=(uint32_t x) noexcept { v |= x; return *this; }
  inline Opcode& operator^=(uint32_t x) noexcept { v ^= x; return *this; }

  inline uint32_t operator&(uint32_t x) const noexcept { return v & x; }
  inline uint32_t operator|(uint32_t x) const noexcept { return v | x; }
  inline uint32_t operator^(uint32_t x) const noexcept { return v ^ x; }
};

// a64::Assembler - Signature Utilities
// ====================================

// TODO: [ARM] Deprecate match_signature.
static inline bool match_signature(const Operand_& o0, const Operand_& o1, uint32_t inst_flags) noexcept {
  if (!(inst_flags & (InstDB::kInstFlagLong | InstDB::kInstFlagNarrow)))
    return o0.signature() == o1.signature();

  // TODO: [ARM] Something smart to validate this.
  return true;
}

static inline bool match_signature(const Operand_& o0, const Operand_& o1, const Operand_& o2, uint32_t inst_flags) noexcept {
  return match_signature(o0, o1, inst_flags) && o1.signature() == o2.signature();
}

static inline bool match_signature(const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3, uint32_t inst_flags) noexcept {
  return match_signature(o0, o1, inst_flags) && o1.signature() == o2.signature() && o2.signature() == o3.signature();
}

// Memory must be either:
// 1. Absolute address, which will be converted to relative.
// 2. Relative displacement (Label).
// 3. Base register + either offset or index.
static inline bool check_mem_base_index_rel(const Mem& mem) noexcept {
  // Allowed base types (Nothing, Label, and Gp64).
  constexpr uint32_t kBaseMask  = B(0) | B(RegType::kLabelTag) | B(RegType::kGp64);
  // Allowed index types (Nothing, Gp32, and Gp64).
  constexpr uint32_t kIndexMask = B(0) | B(RegType::kGp32) | B(RegType::kGp64) ;

  RegType base_type = mem.base_type();
  RegType index_type = mem.index_type();

  if (!Support::bit_test(kBaseMask, base_type)) {
    return false;
  }

  if (base_type > RegType::kLabelTag) {
    // Index allows either Gp32 or Gp64.
    if (!Support::bit_test(kIndexMask, index_type)) {
      return false;
    }

    if (index_type == RegType::kNone) {
      return true;
    }
    else {
      return !mem.has_offset();
    }
  }
  else {
    // No index register allowed if this is a PC relative address (literal).
    return index_type == RegType::kNone;
  }
}

struct EncodeFpOpcodeBits {
  uint32_t size_mask;
  uint32_t mask[3];
};

static inline bool pick_fp_opcode(const Vec& reg, uint32_t s_op, uint32_t s_hf, uint32_t v_op, uint32_t v_hf, Opcode* opcode, uint32_t* sz_out) noexcept {
  static constexpr uint32_t kQBitIndex = 30;

  static const EncodeFpOpcodeBits sz_bits_table[InstDB::kHF_Count] = {
    { B(2) | B(1)       , { 0u                           , 0u, B(22) } },
    { B(2) | B(1) | B(0), { 0u                           , 0u, 0u    } },
    { B(2) | B(1) | B(0), { B(23) | B(22)                , 0u, B(22) } },
    { B(2) | B(1) | B(0), { B(22) | B(20) | B(19)        , 0u, B(22) } },
    { B(2) | B(1) | B(0), { B(22) | B(21) | B(15) | B(14), 0u, B(22) } },
    { B(2) | B(1) | B(0), { B(23)                        , 0u, B(22) } }
  };

  if (!reg.has_element_type()) {
    // Scalar operation [HSD].
    uint32_t sz = diff(reg.reg_type(), RegType::kVec16);
    if (sz > 2u || !Support::bit_test(sz_bits_table[s_hf].size_mask, sz)) {
      return false;
    }

    opcode->reset(sz_bits_table[s_hf].mask[sz] ^ s_op);
    *sz_out = sz;
    return s_op != 0;
  }
  else {
    // Vector operation [HSD].
    uint32_t q = diff(reg.reg_type(), RegType::kVec64);
    uint32_t sz = diff(reg.element_type(), VecElementType::kH);

    if (q > 1u || sz > 2u || !Support::bit_test(sz_bits_table[v_hf].size_mask, sz)) {
      return false;
    }

    opcode->reset(sz_bits_table[v_hf].mask[sz] ^ (v_op | (q << kQBitIndex)));
    *sz_out = sz;
    return v_op != 0;
  }
}

static inline bool pick_fp_opcode(const Vec& reg, uint32_t s_op, uint32_t s_hf, uint32_t v_op, uint32_t v_hf, Opcode* opcode) noexcept {
  uint32_t sz;
  return pick_fp_opcode(reg, s_op, s_hf, v_op, v_hf, opcode, &sz);
}

// a64::Assembler - Operand Checks
// ===============================

// Checks whether all operands have the same signature.
static inline bool check_signature(const Operand_& o0, const Operand_& o1) noexcept {
  return o0.signature() == o1.signature();
}

static inline bool check_signature(const Operand_& o0, const Operand_& o1, const Operand_& o2) noexcept {
  return o0.signature() == o1.signature() &&
         o1.signature() == o2.signature();
}

static inline bool check_signature(const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3) noexcept {
  return o0.signature() == o1.signature() &&
         o1.signature() == o2.signature() &&
         o2.signature() == o3.signature();
}

// Checks whether the register is GP register of the allowed types.
//
// Allowed is a 2-bit mask, where the first bits allows Gp32 and the second bit allows Gp64. These bits are usually
// stored within the instruction, but could be also hardcoded in the assembler for instructions where GP types are
// not selectable.
static inline bool check_gp_type(const Operand_& op, uint32_t allowed) noexcept {
  RegType type = op.as<Reg>().reg_type();
  return Support::bit_test(allowed << uint32_t(RegType::kGp32), type);
}

static inline bool check_gp_type(const Operand_& op, uint32_t allowed, uint32_t* x) noexcept {
  // NOTE: We set 'x' to one only when Gp32 is allowed, otherwise the X is part
  // of the opcode and we cannot set it. This is why this works without requiring
  // additional logic.
  RegType type = op.as<Reg>().reg_type();
  *x = diff(type, RegType::kGp32) & allowed;
  return Support::bit_test(allowed << uint32_t(RegType::kGp32), type);
}

static inline bool check_gp_type(const Operand_& o0, const Operand_& o1, uint32_t allowed, uint32_t* x) noexcept {
  return check_gp_type(o0, allowed, x) && check_signature(o0, o1);
}

static inline bool check_gp_type(const Operand_& o0, const Operand_& o1, const Operand_& o2, uint32_t allowed, uint32_t* x) noexcept {
  return check_gp_type(o0, allowed, x) && check_signature(o0, o1, o2);
}

static inline bool check_gp_id(const Operand_& op, uint32_t hi_id = kZR) noexcept {
  uint32_t id = op.as<Reg>().id();
  return id < 31u || id == hi_id;
}

static inline bool check_gp_id(const Operand_& o0, const Operand_& o1, uint32_t hi_id = kZR) noexcept {
  uint32_t id0 = o0.as<Reg>().id();
  uint32_t id1 = o1.as<Reg>().id();

  return (id0 < 31u || id0 == hi_id) && (id1 < 31u || id1 == hi_id);
}

static inline bool check_gp_id(const Operand_& o0, const Operand_& o1, const Operand_& o2, uint32_t hi_id = kZR) noexcept {
  uint32_t id0 = o0.as<Reg>().id();
  uint32_t id1 = o1.as<Reg>().id();
  uint32_t id2 = o2.as<Reg>().id();

  return (id0 < 31u || id0 == hi_id) && (id1 < 31u || id1 == hi_id) && (id2 < 31u || id2 == hi_id);
}

static inline bool check_vec_id(const Operand_& op) noexcept {
  uint32_t id = op.as<Reg>().id();
  return id <= 31u;
}

static inline bool check_vec_id(const Operand_& o0, const Operand_& o1) noexcept {
  uint32_t id0 = o0.as<Reg>().id();
  uint32_t id1 = o1.as<Reg>().id();

  return (id0 | id1) <= 31u;
}

/* Unused at the moment.
static inline bool check_vec_id(const Operand_& o0, const Operand_& o1, const Operand_& o2) noexcept {
  uint32_t id0 = o0.as<Reg>().id();
  uint32_t id1 = o1.as<Reg>().id();
  uint32_t id2 = o2.as<Reg>().id();

  return (id0 | id1 | id2) <= 31u;
}

static inline bool check_vec_id(const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3) noexcept {
  uint32_t id0 = o0.as<Reg>().id();
  uint32_t id1 = o1.as<Reg>().id();
  uint32_t id2 = o2.as<Reg>().id();
  uint32_t id3 = o3.as<Reg>().id();

  return (id0 | id1 | id2 | id3) <= 31u;
}
*/

static inline bool check_mem_base(const Mem& mem) noexcept {
  return mem.base_type() == RegType::kGp64 && mem.base_id() <= 31;
}

static inline bool check_even(const Operand_& o0, const Operand_& o1) noexcept {
  return ((o0.id() | o1.id()) & 1) == 0;
}

static inline bool check_consecutive(const Operand_& o0, const Operand_& o1) noexcept {
  return ((o0.id() + 1u) & 0x1Fu) == o1.id();
}

static inline bool check_consecutive(const Operand_& o0, const Operand_& o1, const Operand_& o2) noexcept {
  return ((o0.id() + 1u) & 0x1Fu) == o1.id() &&
         ((o0.id() + 2u) & 0x1Fu) == o2.id();
}

static inline bool check_consecutive(const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3) noexcept {
  return ((o0.id() + 1u) & 0x1Fu) == o1.id() &&
         ((o0.id() + 2u) & 0x1Fu) == o2.id() &&
         ((o0.id() + 3u) & 0x1Fu) == o3.id();
}

// a64::Assembler - CheckReg
// =========================

#define V(index) (index == uint32_t(RegType::kGp32)   ? Gp::kIdZr :  \
                  index == uint32_t(RegType::kGp64)   ? Gp::kIdZr :  \
                  index == uint32_t(RegType::kVec8)   ? 31u       :  \
                  index == uint32_t(RegType::kVec16)  ? 31u       :  \
                  index == uint32_t(RegType::kVec32)  ? 31u       :  \
                  index == uint32_t(RegType::kVec64)  ? 31u       :  \
                  index == uint32_t(RegType::kVec128) ? 31u       : 0)
static const Support::Array<uint8_t, 32> common_hi_reg_id_of_type_table = {{
  ASMJIT_LOOKUP_TABLE_32(V, 0)
}};
#undef V

static inline bool check_valid_regs(const Operand_& o0) noexcept {
  return bool(unsigned(o0.id() < 31) | unsigned(o0.id() == common_hi_reg_id_of_type_table[o0.as<Reg>().reg_type()]));
}

static inline bool check_valid_regs(const Operand_& o0, const Operand_& o1) noexcept {
  return bool((unsigned(o0.id() < 31) | unsigned(o0.id() == common_hi_reg_id_of_type_table[o0.as<Reg>().reg_type()])) &
              (unsigned(o1.id() < 31) | unsigned(o1.id() == common_hi_reg_id_of_type_table[o1.as<Reg>().reg_type()])));
}

static inline bool check_valid_regs(const Operand_& o0, const Operand_& o1, const Operand_& o2) noexcept {
  return bool((unsigned(o0.id() < 31) | unsigned(o0.id() == common_hi_reg_id_of_type_table[o0.as<Reg>().reg_type()])) &
              (unsigned(o1.id() < 31) | unsigned(o1.id() == common_hi_reg_id_of_type_table[o1.as<Reg>().reg_type()])) &
              (unsigned(o2.id() < 31) | unsigned(o2.id() == common_hi_reg_id_of_type_table[o2.as<Reg>().reg_type()])));
}

static inline bool check_valid_regs(const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3) noexcept {
  return bool((unsigned(o0.id() < 31) | unsigned(o0.id() == common_hi_reg_id_of_type_table[o0.as<Reg>().reg_type()])) &
              (unsigned(o1.id() < 31) | unsigned(o1.id() == common_hi_reg_id_of_type_table[o1.as<Reg>().reg_type()])) &
              (unsigned(o2.id() < 31) | unsigned(o2.id() == common_hi_reg_id_of_type_table[o2.as<Reg>().reg_type()])) &
              (unsigned(o3.id() < 31) | unsigned(o3.id() == common_hi_reg_id_of_type_table[o3.as<Reg>().reg_type()])));
}

// a64::Assembler - Construction & Destruction
// ===========================================

Assembler::Assembler(CodeHolder* code) noexcept : BaseAssembler() {
  _arch_mask = uint64_t(1) << uint32_t(Arch::kAArch64);
  init_emitter_funcs(this);

  if (code) {
    code->attach(this);
  }
}

Assembler::~Assembler() noexcept {}

// a64::Assembler - Emit
// =====================

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

Error Assembler::_emit(InstId inst_id, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_* op_ext) {
  // Logging/Validation/Error.
  constexpr InstOptions kRequiresSpecialHandling = InstOptions::kReserved;

  Error err;
  CodeWriter writer(this);

  // Combine all instruction options and also check whether the instruction
  // is valid. All options that require special handling (including invalid
  // instruction) are handled by the next branch.
  InstOptions options = InstOptions(inst_id - 1 >= Inst::_kIdCount - 1) | InstOptions((size_t)(_buffer_end - writer.cursor()) < 4) | inst_options() | forced_inst_options();

  CondCode inst_cc = BaseInst::extract_arm_cond_code(inst_id);
  inst_id = inst_id & uint32_t(InstIdParts::kRealId);

  if (inst_id >= Inst::_kIdCount) {
    inst_id = 0;
  }

  const InstDB::InstInfo* inst_info = &InstDB::_inst_info_table[inst_id];
  uint32_t encoding_index = inst_info->_encoding_data_index;

  Opcode opcode;
  uint32_t isign4;
  uint32_t inst_flags;

  const Operand_& o3 = op_ext[EmitterUtils::kOp3];
  const Operand_* rm_rel = nullptr;

  uint32_t multiple_op_data[4];
  uint32_t multiple_op_count;

  // These are only used when instruction uses a relative displacement.
  OffsetFormat offset_format;     // Offset format.
  uint64_t offset_value;          // Offset value (if known).

  if (ASMJIT_UNLIKELY(Support::test(options, kRequiresSpecialHandling))) {
    if (ASMJIT_UNLIKELY(!_code)) {
      return report_error(make_error(Error::kNotInitialized));
    }

    // Unknown instruction.
    if (ASMJIT_UNLIKELY(inst_id == 0)) {
      goto InvalidInstruction;
    }

    // Condition code can only be used with 'B' instruction.
    if (ASMJIT_UNLIKELY(inst_cc != CondCode::kAL && inst_id != Inst::kIdB)) {
      goto InvalidInstruction;
    }

    // Grow request, happens rarely.
    err = writer.ensure_space(this, 4);
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
  }

  // Signature of the first 4 operands.
  isign4 = (uint32_t(o0.op_type())     ) +
           (uint32_t(o1.op_type()) << 3) +
           (uint32_t(o2.op_type()) << 6) +
           (uint32_t(o3.op_type()) << 9);
  inst_flags = inst_info->flags();

  switch (inst_info->_encoding) {
    // ------------------------------------------------------------------------
    // [Base - Universal]
    // ------------------------------------------------------------------------

    case InstDB::kEncodingBaseOp: {
      const InstDB::EncodingData::BaseOp& op_data = InstDB::EncodingData::baseOp[encoding_index];

      if (isign4 == 0) {
        opcode.reset(op_data.opcode);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingBaseOpX16: {
      const InstDB::EncodingData::BaseOpX16& op_data = InstDB::EncodingData::baseOpX16[encoding_index];

      if (isign4 == ENC_OPS1(Reg) && o0.as<Reg>().is_gp64(16)) {
        opcode.reset(op_data.opcode);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingBaseOpImm: {
      const InstDB::EncodingData::BaseOpImm& op_data = InstDB::EncodingData::baseOpImm[encoding_index];

      if (isign4 == ENC_OPS1(Imm)) {
        uint64_t imm = o0.as<Imm>().value_as<uint64_t>();
        uint32_t immMax = 1u << op_data.imm_bits;

        if (imm >= immMax)
          goto InvalidImmediate;

        opcode.reset(op_data.opcode);
        opcode.add_imm(imm, op_data.imm_offset);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingBaseR: {
      const InstDB::EncodingData::BaseR& op_data = InstDB::EncodingData::baseR[encoding_index];

      if (isign4 == ENC_OPS1(Reg)) {
        if (!check_gp_type(o0, op_data.reg_type))
          goto InvalidInstruction;

        if (!check_gp_id(o0, op_data.reg_hi_id))
          goto InvalidPhysId;

        opcode.reset(op_data.opcode);
        opcode.add_reg(o0, op_data.r_shift);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingBaseRR: {
      const InstDB::EncodingData::BaseRR& op_data = InstDB::EncodingData::baseRR[encoding_index];

      if (isign4 == ENC_OPS2(Reg, Reg)) {
        uint32_t x;
        if (!check_gp_type(o0, op_data.a_type, &x))
          goto InvalidInstruction;

        if (!check_gp_type(o1, op_data.b_type))
          goto InvalidInstruction;

        if (op_data.uniform && !check_signature(o0, o1))
          goto InvalidInstruction;

        if (!check_gp_id(o0, op_data.a_hi_id))
          goto InvalidPhysId;

        if (!check_gp_id(o1, op_data.b_hi_id))
          goto InvalidPhysId;

        opcode.reset(op_data.opcode);
        opcode.add_imm(x, 31);
        opcode.add_reg(o1, op_data.b_shift);
        opcode.add_reg(o0, op_data.a_shift);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingBaseRRR: {
      const InstDB::EncodingData::BaseRRR& op_data = InstDB::EncodingData::baseRRR[encoding_index];

      if (isign4 == ENC_OPS3(Reg, Reg, Reg)) {
        uint32_t x;
        if (!check_gp_type(o0, op_data.a_type, &x))
          goto InvalidInstruction;

        if (!check_gp_type(o1, op_data.b_type))
          goto InvalidInstruction;

        if (!check_gp_type(o2, op_data.c_type))
          goto InvalidInstruction;

        if (op_data.uniform && !check_signature(o0, o1, o2))
          goto InvalidInstruction;

        if (!check_gp_id(o0, op_data.a_hi_id))
          goto InvalidPhysId;

        if (!check_gp_id(o1, op_data.b_hi_id))
          goto InvalidPhysId;

        if (!check_gp_id(o2, op_data.c_hi_id))
          goto InvalidPhysId;

        opcode.reset(op_data.opcode());
        opcode.add_imm(x, 31);
        opcode.add_reg(o2, 16);
        opcode.add_reg(o1, 5);
        opcode.add_reg(o0, 0);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingBaseRRRR: {
      const InstDB::EncodingData::BaseRRRR& op_data = InstDB::EncodingData::baseRRRR[encoding_index];

      if (isign4 == ENC_OPS4(Reg, Reg, Reg, Reg)) {
        uint32_t x;
        if (!check_gp_type(o0, op_data.a_type, &x))
          goto InvalidInstruction;

        if (!check_gp_type(o1, op_data.b_type))
          goto InvalidInstruction;

        if (!check_gp_type(o2, op_data.c_type))
          goto InvalidInstruction;

        if (!check_gp_type(o3, op_data.d_type))
          goto InvalidInstruction;

        if (op_data.uniform && !check_signature(o0, o1, o2, o3))
          goto InvalidInstruction;

        if (!check_gp_id(o0, op_data.a_hi_id))
          goto InvalidPhysId;

        if (!check_gp_id(o1, op_data.b_hi_id))
          goto InvalidPhysId;

        if (!check_gp_id(o2, op_data.c_hi_id))
          goto InvalidPhysId;

        if (!check_gp_id(o3, op_data.d_hi_id))
          goto InvalidPhysId;

        opcode.reset(op_data.opcode());
        opcode.add_imm(x, 31);
        opcode.add_reg(o2, 16);
        opcode.add_reg(o3, 10);
        opcode.add_reg(o1, 5);
        opcode.add_reg(o0, 0);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingBaseRRII: {
      const InstDB::EncodingData::BaseRRII& op_data = InstDB::EncodingData::baseRRII[encoding_index];

      if (isign4 == ENC_OPS4(Reg, Reg, Imm, Imm)) {
        if (!check_gp_type(o0, op_data.a_type))
          goto InvalidInstruction;

        if (!check_gp_type(o1, op_data.b_type))
          goto InvalidInstruction;

        if (!check_gp_id(o0, op_data.a_hi_id))
          goto InvalidPhysId;

        if (!check_gp_id(o1, op_data.b_hi_id))
          goto InvalidPhysId;

        if (o2.as<Imm>().value_as<uint64_t>() >= Support::bit_mask<uint32_t>(op_data.a_imm_size + op_data.a_imm_discard_lsb) ||
            o3.as<Imm>().value_as<uint64_t>() >= Support::bit_mask<uint32_t>(op_data.b_imm_size + op_data.b_imm_discard_lsb))
          goto InvalidImmediate;

        uint32_t a_imm = o2.as<Imm>().value_as<uint32_t>() >> op_data.a_imm_discard_lsb;
        uint32_t b_imm = o3.as<Imm>().value_as<uint32_t>() >> op_data.b_imm_discard_lsb;

        if ((a_imm << op_data.a_imm_discard_lsb) != o2.as<Imm>().value_as<uint32_t>() ||
            (b_imm << op_data.b_imm_discard_lsb) != o3.as<Imm>().value_as<uint32_t>())
          goto InvalidImmediate;

        opcode.reset(op_data.opcode());
        opcode.add_imm(a_imm, op_data.a_imm_offset);
        opcode.add_imm(b_imm, op_data.b_imm_offset);
        opcode.add_reg(o1, 5);
        opcode.add_reg(o0, 0);
        goto EmitOp;
      }

      break;
    }

    // ------------------------------------------------------------------------
    // [Base - Mov]
    // ------------------------------------------------------------------------

    case InstDB::kEncodingBaseMov: {
      // MOV is a pseudo instruction that uses various instructions depending on its signature.
      uint32_t x = diff(o0.as<Reg>().reg_type(), RegType::kGp32);
      if (x > 1)
        goto InvalidInstruction;

      if (isign4 == ENC_OPS2(Reg, Reg)) {
        if (!o0.as<Reg>().is_gp())
          goto InvalidInstruction;

        if (!check_signature(o0, o1))
          goto InvalidInstruction;

        bool has_sp = o0.as<Gp>().is_sp() || o1.as<Gp>().is_sp();
        if (has_sp) {
          // Cannot be combined with ZR.
          if (!check_gp_id(o0, o1, kSP))
            goto InvalidPhysId;

          // MOV Rd, Rm -> ADD Rd, Rn, #0.
          opcode.reset(0b00010001000000000000000000000000);
          opcode.add_imm(x, 31);
          opcode.add_reg(o1, 5);
          opcode.add_reg(o0, 0);
          goto EmitOp;
        }
        else {
          if (!check_gp_id(o0, o1, kZR))
            goto InvalidPhysId;

          // MOV Rd, Rm -> ORR Rd, <ZR>, Rm.
          opcode.reset(0b00101010000000000000001111100000);
          opcode.add_imm(x, 31);
          opcode.add_reg(o1, 16);
          opcode.add_reg(o0, 0);
          goto EmitOp;
        }
      }

      if (isign4 == ENC_OPS2(Reg, Imm)) {
        if (!o0.as<Reg>().is_gp())
          goto InvalidInstruction;

        uint64_t imm_value = o1.as<Imm>().value_as<uint64_t>();
        if (!x)
          imm_value &= 0xFFFFFFFFu;

        // Prefer a single MOVN/MOVZ instruction over a logical instruction.
        multiple_op_count = encode_mov_sequence_64(multiple_op_data, imm_value, o0.id() & 31, x);
        if (multiple_op_count == 1 && !o0.as<Gp>().is_sp()) {
          opcode.reset(multiple_op_data[0]);
          goto EmitOp;
        }

        // Logical instructions use 13-bit immediate pattern encoded as N:ImmR:ImmS.
        LogicalImm logical_imm;
        if (!o0.as<Gp>().is_zr()) {
          if (Utils::encode_logical_imm(imm_value, x ? 64 : 32, Out(logical_imm))) {
            if (!check_gp_id(o0, kSP))
              goto InvalidPhysId;

            opcode.reset(0b00110010000000000000001111100000);
            opcode.add_imm(x, 31);
            opcode.add_logical_imm(logical_imm);
            opcode.add_reg(o0, 0);
            goto EmitOp;
          }
        }

        if (!check_gp_id(o0, kZR))
          goto InvalidPhysId;

        goto EmitOp_Multiple;
      }

      break;
    }

    case InstDB::kEncodingBaseMovKNZ: {
      const InstDB::EncodingData::BaseMovKNZ& op_data = InstDB::EncodingData::baseMovKNZ[encoding_index];

      uint32_t x = diff(o0.as<Reg>().reg_type(), RegType::kGp32);
      if (x > 1)
        goto InvalidInstruction;

      if (!check_gp_id(o0, kZR))
        goto InvalidPhysId;

      opcode.reset(op_data.opcode);
      opcode.add_imm(x, 31);

      if (isign4 == ENC_OPS2(Reg, Imm)) {
        uint64_t imm16 = o1.as<Imm>().value_as<uint64_t>();
        if (imm16 > 0xFFFFu)
          goto InvalidImmediate;

        opcode.add_imm(imm16, 5);
        opcode.add_reg(o0, 0);
        goto EmitOp;
      }

      if (isign4 == ENC_OPS3(Reg, Imm, Imm)) {
        uint64_t imm16 = o1.as<Imm>().value_as<uint64_t>();
        uint32_t shift_type = o2.as<Imm>().predicate();
        uint64_t shiftValue = o2.as<Imm>().value_as<uint64_t>();

        if (imm16 > 0xFFFFu || shiftValue > 48 || shift_type != uint32_t(ShiftOp::kLSL))
          goto InvalidImmediate;

        // Convert shift value to 'hw' field.
        uint32_t hw = uint32_t(shiftValue) >> 4;
        if ((hw << 4) != uint32_t(shiftValue))
          goto InvalidImmediate;

        opcode.add_imm(hw, 21);
        opcode.add_imm(imm16, 5);
        opcode.add_reg(o0, 0);

        if (!x && hw > 1u)
          goto InvalidImmediate;

        goto EmitOp;
      }

      break;
    }

    // ------------------------------------------------------------------------
    // [Base - Adr]
    // ------------------------------------------------------------------------

    case InstDB::kEncodingBaseAdr: {
      const InstDB::EncodingData::BaseAdr& op_data = InstDB::EncodingData::baseAdr[encoding_index];

      if (isign4 == ENC_OPS2(Reg, Label) || isign4 == ENC_OPS2(Reg, Imm)) {
        if (!o0.as<Reg>().is_gp64())
          goto InvalidInstruction;

        if (!check_gp_id(o0, kZR))
          goto InvalidPhysId;

        opcode.reset(op_data.opcode());
        opcode.add_reg(o0, 0);
        offset_format.reset_to_imm_value(op_data.offset_type, 4, 5, 21, 0);

        if (inst_id == Inst::kIdAdrp)
          offset_format._imm_discard_lsb = 12;

        rm_rel = &o1;
        goto EmitOp_Rel;
      }

      break;
    }

    // ------------------------------------------------------------------------
    // [Base - Arithmetic and Logical]
    // ------------------------------------------------------------------------

    case InstDB::kEncodingBaseAddSub: {
      const InstDB::EncodingData::BaseAddSub& op_data = InstDB::EncodingData::baseAddSub[encoding_index];

      uint32_t x;
      if (!check_gp_type(o0, o1, kWX, &x))
        goto InvalidInstruction;

      if (isign4 == ENC_OPS3(Reg, Reg, Imm) || isign4 == ENC_OPS4(Reg, Reg, Imm, Imm)) {
        opcode.reset(uint32_t(op_data.immediate_op) << 24);

        // ADD | SUB (immediate) - ZR is not allowed.
        // ADDS|SUBS (immediate) - ZR allowed in Rd, SP allowed in Rn.
        uint32_t a_hi_id = opcode.get() & B(29) ? kZR : kSP;
        uint32_t b_hi_id = kSP;

        if (!check_gp_id(o0, a_hi_id) || !check_gp_id(o1, b_hi_id))
          goto InvalidPhysId;

        // ADD|SUB (immediate) use 12-bit immediate optionally shifted by 'LSL #12'.
        uint64_t imm = o2.as<Imm>().value_as<uint64_t>();
        uint32_t shift = 0;

        if (isign4 == ENC_OPS4(Reg, Reg, Imm, Imm)) {
          if (o3.as<Imm>().predicate() != uint32_t(ShiftOp::kLSL))
            goto InvalidImmediate;

          if (o3.as<Imm>().value() != 0 && o3.as<Imm>().value() != 12)
            goto InvalidImmediate;

          shift = uint32_t(o3.as<Imm>().value() != 0);
        }

        // Accept immediate value of '0x00XXX000' by setting 'shift' to 12.
        if (imm > 0xFFFu) {
          if (shift || (imm & ~uint64_t(0xFFFu << 12)) != 0)
            goto InvalidImmediate;
          shift = 1;
          imm >>= 12;
        }

        opcode.add_imm(x, 31);
        opcode.add_imm(shift, 22);
        opcode.add_imm(imm, 10);
        opcode.add_reg(o1, 5);
        opcode.add_reg(o0, 0);
        goto EmitOp;
      }

      if (isign4 == ENC_OPS3(Reg, Reg, Reg) || isign4 == ENC_OPS4(Reg, Reg, Reg, Imm)) {
        uint32_t op_size = x ? 64 : 32;
        uint64_t shift = 0;
        uint32_t shift_type = uint32_t(ShiftOp::kLSL);

        if (isign4 == ENC_OPS4(Reg, Reg, Reg, Imm)) {
          shift_type = o3.as<Imm>().predicate();
          shift = o3.as<Imm>().value_as<uint64_t>();
        }

        if (!check_gp_id(o2, kZR))
          goto InvalidPhysId;

        // Shift operation - LSL, LSR, ASR.
        if (shift_type <= uint32_t(ShiftOp::kASR)) {
          bool has_sp = o0.as<Gp>().is_sp() || o1.as<Gp>().is_sp();
          if (!has_sp) {
            if (!check_signature(o1, o2)) {
              goto InvalidInstruction;
            }

            if (!check_gp_id(o0, o1, kZR)) {
              goto InvalidPhysId;
            }

            if (shift >= op_size) {
              goto InvalidImmediate;
            }

            opcode.reset(uint32_t(op_data.shifted_op) << 21);
            opcode.add_imm(x, 31);
            opcode.add_imm(shift_type, 22);
            opcode.add_reg(o2, 16);
            opcode.add_imm(shift, 10);
            opcode.add_reg(o1, 5);
            opcode.add_reg(o0, 0);
            goto EmitOp;
          }

          // SP register can only be used with LSL or Extend.
          if (shift_type != uint32_t(ShiftOp::kLSL)) {
            goto InvalidImmediate;
          }

          shift_type = x ? uint32_t(ShiftOp::kUXTX) : uint32_t(ShiftOp::kUXTW);
        }

        // Extend operation - UXTB, UXTH, UXTW, UXTX, SXTB, SXTH, SXTW, SXTX.
        opcode.reset(uint32_t(op_data.extended_op) << 21);
        shift_type -= uint32_t(ShiftOp::kUXTB);

        if (shift_type > 7 || shift > 4) {
          goto InvalidImmediate;
        }

        if (!(opcode.get() & B(29))) {
          // ADD|SUB (extend) - ZR is not allowed.
          if (!check_gp_id(o0, o1, kSP))
            goto InvalidPhysId;
        }
        else {
          // ADDS|SUBS (extend) - ZR allowed in Rd, SP allowed in Rn.
          if (!check_gp_id(o0, kZR) || !check_gp_id(o1, kSP))
            goto InvalidPhysId;
        }

        // Validate whether the register operands match extend option.
        if (o2.as<Reg>().reg_type() != extend_option_to_reg_type(shift_type) || o1.as<Reg>().reg_type() < o2.as<Reg>().reg_type()) {
          goto InvalidInstruction;
        }

        opcode.add_imm(x, 31);
        opcode.add_reg(o2, 16);
        opcode.add_imm(shift_type, 13);
        opcode.add_imm(shift, 10);
        opcode.add_reg(o1, 5);
        opcode.add_reg(o0, 0);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingBaseLogical: {
      const InstDB::EncodingData::BaseLogical& op_data = InstDB::EncodingData::baseLogical[encoding_index];

      uint32_t x;
      if (!check_gp_type(o0, o1, kWX, &x))
        goto InvalidInstruction;

      if (!check_signature(o0, o1))
        goto InvalidInstruction;

      uint32_t op_size = x ? 64 : 32;

      if (isign4 == ENC_OPS3(Reg, Reg, Imm) && op_data.immediate_op != 0) {
        opcode.reset(uint32_t(op_data.immediate_op) << 23);

        // AND|ANDS|BIC|BICS|ORR|EOR (immediate) uses a LogicalImm format described by N:R:S values.
        uint64_t imm_mask = Support::lsb_mask<uint64_t>(op_size);
        uint64_t imm_value = o2.as<Imm>().value_as<uint64_t>();

        if (op_data.negate_imm)
          imm_value ^= imm_mask;

        // Logical instructions use 13-bit immediate pattern encoded as N:ImmS:ImmR.
        LogicalImm logical_imm;
        if (!Utils::encode_logical_imm(imm_value & imm_mask, op_size, Out(logical_imm)))
          goto InvalidImmediate;

        // AND|BIC|ORR|EOR (immediate) can have SP on destination, but ANDS|BICS (immediate) cannot.
        uint32_t kOpANDS = 0x3 << 29;
        bool isANDS = (opcode.get() & kOpANDS) == kOpANDS;

        if (!check_gp_id(o0, isANDS ? kZR : kSP) || !check_gp_id(o1, kZR))
          goto InvalidPhysId;

        opcode.add_imm(x, 31);
        opcode.add_logical_imm(logical_imm);
        opcode.add_reg(o1, 5);
        opcode.add_reg(o0, 0);
        goto EmitOp;
      }

      if (!check_signature(o1, o2))
        goto InvalidInstruction;

      if (isign4 == ENC_OPS3(Reg, Reg, Reg)) {
        if (!check_gp_id(o0, o1, o2, kZR))
          goto InvalidPhysId;

        opcode.reset(uint32_t(op_data.shifted_op) << 21);
        opcode.add_imm(x, 31);
        opcode.add_reg(o2, 16);
        opcode.add_reg(o1, 5);
        opcode.add_reg(o0, 0);
        goto EmitOp;
      }

      if (isign4 == ENC_OPS4(Reg, Reg, Reg, Imm)) {
        if (!check_gp_id(o0, o1, o2, kZR))
          goto InvalidPhysId;

        uint32_t shift_type = o3.as<Imm>().predicate();
        uint64_t op_shift = o3.as<Imm>().value_as<uint64_t>();

        if (shift_type > 0x3 || op_shift >= op_size)
          goto InvalidImmediate;

        opcode.reset(uint32_t(op_data.shifted_op) << 21);
        opcode.add_imm(x, 31);
        opcode.add_imm(shift_type, 22);
        opcode.add_reg(o2, 16);
        opcode.add_imm(op_shift, 10);
        opcode.add_reg(o1, 5);
        opcode.add_reg(o0, 0);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingBaseCmpCmn: {
      const InstDB::EncodingData::BaseCmpCmn& op_data = InstDB::EncodingData::baseCmpCmn[encoding_index];

      uint32_t x;
      if (!check_gp_type(o0, kWX, &x))
        goto InvalidInstruction;

      if (isign4 == ENC_OPS2(Reg, Imm)) {
        // CMN|CMP (immediate) - ZR is not allowed.
        if (!check_gp_id(o0, kSP))
          goto InvalidPhysId;

        // CMN|CMP (immediate) use 12-bit immediate optionally shifted by 'LSL #12'.
        const Imm& imm12 = o1.as<Imm>();
        uint32_t imm_shift = 0;
        uint64_t imm_value = imm12.value_as<uint64_t>();

        if (imm_value > 0xFFFu) {
          if ((imm_value & ~uint64_t(0xFFFu << 12)) != 0)
            goto InvalidImmediate;
          imm_shift = 1;
          imm_value >>= 12;
        }

        opcode.reset(uint32_t(op_data.immediate_op) << 24);
        opcode.add_imm(x, 31);
        opcode.add_imm(imm_shift, 22);
        opcode.add_imm(imm_value, 10);
        opcode.add_reg(o0, 5);
        opcode.add_reg(Gp::kIdZr, 0);
        goto EmitOp;
      }

      if (isign4 == ENC_OPS2(Reg, Reg) || isign4 == ENC_OPS3(Reg, Reg, Imm)) {
        uint32_t op_size = x ? 64 : 32;
        uint32_t shift_type = 0;
        uint64_t shift_value = 0;

        if (isign4 == ENC_OPS3(Reg, Reg, Imm)) {
          shift_type = o2.as<Imm>().predicate();
          shift_value = o2.as<Imm>().value_as<uint64_t>();
        }

        bool has_sp = o0.as<Gp>().is_sp() || o1.as<Gp>().is_sp();

        // Shift operation - LSL, LSR, ASR.
        if (shift_type <= uint32_t(ShiftOp::kASR)) {
          if (!has_sp) {
            if (!check_signature(o0, o1)) {
              goto InvalidInstruction;
            }

            if (shift_value >= op_size) {
              goto InvalidImmediate;
            }

            opcode.reset(uint32_t(op_data.shifted_op) << 21);
            opcode.add_imm(x, 31);
            opcode.add_imm(shift_type, 22);
            opcode.add_reg(o1, 16);
            opcode.add_imm(shift_value, 10);
            opcode.add_reg(o0, 5);
            opcode.add_reg(Gp::kIdZr, 0);
            goto EmitOp;
          }

          // SP register can only be used with LSL or Extend.
          if (shift_type != uint32_t(ShiftOp::kLSL))
            goto InvalidImmediate;

          shift_type = x ? uint32_t(ShiftOp::kUXTX) : uint32_t(ShiftOp::kUXTW);
        }

        // Extend operation - UXTB, UXTH, UXTW, UXTX, SXTB, SXTH, SXTW, SXTX.
        shift_type -= uint32_t(ShiftOp::kUXTB);
        if (shift_type > 7 || shift_value > 4) {
          goto InvalidImmediate;
        }

        // Validate whether the register operands match extend option.
        if (o1.as<Reg>().reg_type() != extend_option_to_reg_type(shift_type) || o0.as<Reg>().reg_type() < o1.as<Reg>().reg_type()) {
          goto InvalidInstruction;
        }

        opcode.reset(uint32_t(op_data.extended_op) << 21);
        opcode.add_imm(x, 31);
        opcode.add_reg(o1, 16);
        opcode.add_imm(shift_type, 13);
        opcode.add_imm(shift_value, 10);
        opcode.add_reg(o0, 5);
        opcode.add_reg(Gp::kIdZr, 0);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingBaseMvnNeg: {
      const InstDB::EncodingData::BaseMvnNeg& op_data = InstDB::EncodingData::baseMvnNeg[encoding_index];

      uint32_t x;
      if (!check_gp_type(o0, o1, kWX, &x))
        goto InvalidInstruction;

      opcode.reset(op_data.opcode);
      opcode.add_imm(x, 31);
      opcode.add_reg(o1, 16);
      opcode.add_reg(o0, 0);

      if (isign4 == ENC_OPS2(Reg, Reg)) {
        if (!check_gp_id(o0, o1, kZR))
          goto InvalidPhysId;

        goto EmitOp;
      }

      if (isign4 == ENC_OPS3(Reg, Reg, Imm)) {
        if (!check_gp_id(o0, o1, kZR))
          goto InvalidPhysId;

        uint32_t op_size = x ? 64 : 32;
        uint32_t shift_type = o2.as<Imm>().predicate();
        uint64_t shift_value = o2.as<Imm>().value_as<uint64_t>();

        if (shift_type > uint32_t(ShiftOp::kROR) || shift_value >= op_size)
          goto InvalidImmediate;

        opcode.add_imm(shift_type, 22);
        opcode.add_imm(shift_value, 10);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingBaseTst: {
      const InstDB::EncodingData::BaseTst& op_data = InstDB::EncodingData::baseTst[encoding_index];

      uint32_t x;
      if (!check_gp_type(o0, kWX, &x))
        goto InvalidInstruction;

      uint32_t op_size = x ? 64 : 32;

      if (isign4 == ENC_OPS2(Reg, Imm) && op_data.immediate_op != 0) {
        if (!check_gp_id(o0, kZR))
          goto InvalidPhysId;

        // TST (immediate) uses a LogicalImm format described by N:R:S values.
        uint64_t imm_mask = Support::lsb_mask<uint64_t>(op_size);
        uint64_t imm_value = o1.as<Imm>().value_as<uint64_t>();

        // Logical instructions use 13-bit immediate pattern encoded as N:ImmS:ImmR.
        LogicalImm logical_imm;
        if (!Utils::encode_logical_imm(imm_value & imm_mask, op_size, Out(logical_imm)))
          goto InvalidImmediate;

        opcode.reset(uint32_t(op_data.immediate_op) << 22);
        opcode.add_logical_imm(logical_imm);
        opcode.add_imm(x, 31);
        opcode.add_reg(o0, 5);
        opcode.add_reg(Gp::kIdZr, 0);
        goto EmitOp;
      }

      opcode.reset(uint32_t(op_data.shifted_op) << 21);
      opcode.add_imm(x, 31);
      opcode.add_reg(o1, 16);
      opcode.add_reg(o0, 5);
      opcode.add_reg(Gp::kIdZr, 0);

      if (isign4 == ENC_OPS2(Reg, Reg)) {
        if (!check_gp_id(o0, o1, kZR))
          goto InvalidPhysId;

        goto EmitOp;
      }

      if (isign4 == ENC_OPS3(Reg, Reg, Imm)) {
        if (!check_gp_id(o0, o1, kZR))
          goto InvalidPhysId;

        uint32_t shift_type = o2.as<Imm>().predicate();
        uint64_t op_shift = o2.as<Imm>().value_as<uint64_t>();

        if (shift_type > 0x3 || op_shift >= op_size)
          goto InvalidImmediate;

        opcode.add_imm(shift_type, 22);
        opcode.add_imm(op_shift, 10);
        goto EmitOp;
      }

      break;
    }

    // ------------------------------------------------------------------------
    // [Base - Bit Manipulation]
    // ------------------------------------------------------------------------

    case InstDB::kEncodingBaseBfc: {
      const InstDB::EncodingData::BaseBfc& op_data = InstDB::EncodingData::baseBfc[encoding_index];

      if (isign4 == ENC_OPS3(Reg, Imm, Imm)) {
        uint32_t x;
        if (!check_gp_type(o0, InstDB::kWX, &x))
          goto InvalidInstruction;

        if (!check_gp_id(o0))
          goto InvalidPhysId;

        uint64_t lsb = o1.as<Imm>().value_as<uint64_t>();
        uint64_t width = o2.as<Imm>().value_as<uint64_t>();
        uint32_t op_size = x ? 64 : 32;

        if (lsb >= op_size || width == 0 || width > op_size)
          goto InvalidImmediate;

        uint32_t lsb32 = Support::neg(uint32_t(lsb)) & (op_size - 1);
        uint32_t width32 = uint32_t(width) - 1;

        opcode.reset(op_data.opcode);
        opcode.add_imm(x, 31);
        opcode.add_imm(x, 22);
        opcode.add_imm(lsb32, 16);
        opcode.add_imm(width32, 10);
        opcode.add_reg(o0, 0);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingBaseBfi: {
      const InstDB::EncodingData::BaseBfi& op_data = InstDB::EncodingData::baseBfi[encoding_index];

      if (isign4 == ENC_OPS4(Reg, Reg, Imm, Imm)) {
        uint32_t x;
        if (!check_gp_type(o0, InstDB::kWX, &x))
          goto InvalidInstruction;

        if (!check_signature(o0, o1))
          goto InvalidInstruction;

        if (!check_gp_id(o0, o1))
          goto InvalidPhysId;

        uint64_t lsb = o2.as<Imm>().value_as<uint64_t>();
        uint64_t width = o3.as<Imm>().value_as<uint64_t>();
        uint32_t op_size = x ? 64 : 32;

        if (lsb >= op_size || width == 0 || width > op_size)
          goto InvalidImmediate;

        uint32_t imm_l = Support::neg(uint32_t(lsb)) & (op_size - 1);
        uint32_t imm_w = uint32_t(width) - 1;

        opcode.reset(op_data.opcode);
        opcode.add_imm(x, 31);
        opcode.add_imm(x, 22);
        opcode.add_imm(imm_l, 16);
        opcode.add_imm(imm_w, 10);
        opcode.add_reg(o1, 5);
        opcode.add_reg(o0, 0);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingBaseBfm: {
      const InstDB::EncodingData::BaseBfm& op_data = InstDB::EncodingData::baseBfm[encoding_index];

      if (isign4 == ENC_OPS4(Reg, Reg, Imm, Imm)) {
        uint32_t x;
        if (!check_gp_type(o0, InstDB::kWX, &x))
          goto InvalidInstruction;

        if (!check_signature(o0, o1))
          goto InvalidInstruction;

        if (!check_gp_id(o0, o1))
          goto InvalidPhysId;

        uint64_t imm_r = o2.as<Imm>().value_as<uint64_t>();
        uint64_t imm_s = o3.as<Imm>().value_as<uint64_t>();
        uint32_t op_size = x ? 64 : 32;

        if ((imm_r | imm_s) >= op_size)
          goto InvalidImmediate;

        opcode.reset(op_data.opcode);
        opcode.add_imm(x, 31);
        opcode.add_imm(x, 22);
        opcode.add_imm(imm_r, 16);
        opcode.add_imm(imm_s, 10);
        opcode.add_reg(o1, 5);
        opcode.add_reg(o0, 0);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingBaseBfx: {
      const InstDB::EncodingData::BaseBfx& op_data = InstDB::EncodingData::baseBfx[encoding_index];

      if (isign4 == ENC_OPS4(Reg, Reg, Imm, Imm)) {
        uint32_t x;
        if (!check_gp_type(o0, InstDB::kWX, &x))
          goto InvalidInstruction;

        if (!check_signature(o0, o1))
          goto InvalidInstruction;

        if (!check_gp_id(o0, o1))
          goto InvalidPhysId;

        uint64_t lsb = o2.as<Imm>().value_as<uint64_t>();
        uint64_t width = o3.as<Imm>().value_as<uint64_t>();
        uint32_t op_size = x ? 64 : 32;

        if (lsb >= op_size || width == 0 || width > op_size)
          goto InvalidImmediate;

        uint32_t lsb32 = uint32_t(lsb);
        uint32_t width32 = lsb32 + uint32_t(width) - 1u;

        if (width32 >= op_size)
          goto InvalidImmediate;

        opcode.reset(op_data.opcode);
        opcode.add_imm(x, 31);
        opcode.add_imm(x, 22);
        opcode.add_imm(lsb32, 16);
        opcode.add_imm(width32, 10);
        opcode.add_reg(o1, 5);
        opcode.add_reg(o0, 0);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingBaseExtend: {
      const InstDB::EncodingData::BaseExtend& op_data = InstDB::EncodingData::baseExtend[encoding_index];

      if (isign4 == ENC_OPS2(Reg, Reg)) {
        uint32_t x;
        if (!check_gp_type(o0, op_data.reg_type, &x))
          goto InvalidInstruction;

        if (!o1.as<Reg>().is_gp32())
          goto InvalidInstruction;

        if (!check_gp_id(o0, o1))
          goto InvalidPhysId;

        opcode.reset(op_data.opcode());
        opcode.add_imm(x, 31);
        opcode.add_imm(x, 22);
        opcode.add_reg(o1, 5);
        opcode.add_reg(o0, 0);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingBaseExtract: {
      const InstDB::EncodingData::BaseExtract& op_data = InstDB::EncodingData::baseExtract[encoding_index];

      if (isign4 == ENC_OPS4(Reg, Reg, Reg, Imm)) {
        uint32_t x;
        if (!check_gp_type(o0, kWX, &x))
          goto InvalidInstruction;

        if (!check_signature(o0, o1, o2))
          goto InvalidInstruction;

        if (!check_gp_id(o0, o1, o2))
          goto InvalidPhysId;

        uint64_t lsb = o3.as<Imm>().value_as<uint64_t>();
        uint32_t op_size = x ? 64 : 32;

        if (lsb >= op_size)
          goto InvalidImmediate;

        opcode.reset(op_data.opcode);
        opcode.add_imm(x, 31);
        opcode.add_imm(x, 22);
        opcode.add_reg(o2, 16);
        opcode.add_imm(lsb, 10);
        opcode.add_reg(o1, 5);
        opcode.add_reg(o0, 0);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingBaseRev: {
      if (isign4 == ENC_OPS2(Reg, Reg)) {
        uint32_t x;
        if (!check_gp_type(o0, InstDB::kWX, &x))
          goto InvalidInstruction;

        if (!check_signature(o0, o1))
          goto InvalidInstruction;

        if (!check_gp_id(o0, o1))
          goto InvalidPhysId;

        opcode.reset(0b01011010110000000000100000000000);
        opcode.add_imm(x, 31);
        opcode.add_imm(x, 10);
        opcode.add_reg(o1, 5);
        opcode.add_reg(o0, 0);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingBaseShift: {
      const InstDB::EncodingData::BaseShift& op_data = InstDB::EncodingData::baseShift[encoding_index];

      uint32_t x;
      if (!check_gp_type(o0, kWX, &x))
        goto InvalidInstruction;

      if (isign4 == ENC_OPS3(Reg, Reg, Reg)) {
        if (!check_signature(o0, o1, o2))
          goto InvalidInstruction;

        if (!check_gp_id(o0, o1, o2, kZR))
          goto InvalidPhysId;

        opcode.reset(op_data.register_op());
        opcode.add_imm(x, 31);
        opcode.add_reg(o2, 16);
        opcode.add_reg(o1, 5);
        opcode.add_reg(o0, 0);
        goto EmitOp;
      }

      if (isign4 == ENC_OPS3(Reg, Reg, Imm) && op_data.immediate_op()) {
        if (!check_signature(o0, o1))
          goto InvalidInstruction;

        if (!check_gp_id(o0, o1, kZR))
          goto InvalidPhysId;

        uint64_t imm_r = o2.as<Imm>().value_as<uint64_t>();
        uint32_t op_size = x ? 64 : 32;

        if (imm_r >= op_size)
          goto InvalidImmediate;

        opcode.reset(op_data.immediate_op());
        opcode.add_imm(x, 31);
        opcode.add_imm(x, 22);
        opcode.add_reg(o1, 5);
        opcode.add_reg(o0, 0);

        if (opcode.get() & B(10)) {
          // ASR and LSR (immediate) has the same logic.
          opcode.add_imm(x, 15);
          opcode.add_imm(imm_r, 16);
          goto EmitOp;
        }

        if (op_data.ror == 0) {
          // LSL (immediate) is an alias to UBFM
          uint32_t ubfm_imm_r = Support::neg(uint32_t(imm_r)) & (op_size - 1);
          uint32_t ubfm_imm_s = op_size - 1 - uint32_t(imm_r);

          opcode.add_imm(ubfm_imm_r, 16);
          opcode.add_imm(ubfm_imm_s, 10);
          goto EmitOp;
        }
        else {
          // ROR (immediate) is an alias to EXTR.
          opcode.add_imm(imm_r, 10);
          opcode.add_reg(o1, 16);
          goto EmitOp;
        }
      }

      break;
    }

    // ------------------------------------------------------------------------
    // [Base - Conditionals]
    // ------------------------------------------------------------------------

    case InstDB::kEncodingBaseCCmp: {
      const InstDB::EncodingData::BaseCCmp& op_data = InstDB::EncodingData::baseCCmp[encoding_index];

      if (isign4 == ENC_OPS4(Reg, Reg, Imm, Imm) || isign4 == ENC_OPS4(Reg, Imm, Imm, Imm)) {
        uint32_t x;
        if (!check_gp_type(o0, InstDB::kWX, &x))
          goto InvalidInstruction;

        if (!check_gp_id(o0, kZR))
          goto InvalidPhysId;

        uint64_t nzcv = o2.as<Imm>().value_as<uint64_t>();
        uint64_t cond = o3.as<Imm>().value_as<uint64_t>();

        if ((nzcv | cond) > 0xFu)
          goto InvalidImmediate;

        opcode.reset(op_data.opcode);
        opcode.add_imm(x, 31);
        opcode.add_imm(cond_code_to_opcode_field(uint32_t(cond)), 12);
        opcode.add_imm(nzcv, 0);

        if (isign4 == ENC_OPS4(Reg, Reg, Imm, Imm)) {
          // CCMN|CCMP (register) form.
          if (!check_signature(o0, o1))
            goto InvalidInstruction;

          if (!check_gp_id(o1, kZR))
            goto InvalidPhysId;

          opcode.add_reg(o1, 16);
          opcode.add_reg(o0, 5);
          goto EmitOp;
        }
        else {
          // CCMN|CCMP (immediate) form.
          uint64_t imm5 = o1.as<Imm>().value_as<uint64_t>();
          if (imm5 > 0x1F)
            goto InvalidImmediate;

          opcode.add_imm(1, 11);
          opcode.add_imm(imm5, 16);
          opcode.add_reg(o0, 5);
          goto EmitOp;
        }
      }

      break;
    }

    case InstDB::kEncodingBaseCInc: {
      const InstDB::EncodingData::BaseCInc& op_data = InstDB::EncodingData::baseCInc[encoding_index];

      if (isign4 == ENC_OPS3(Reg, Reg, Imm)) {
        uint32_t x;
        if (!check_gp_type(o0, o1, InstDB::kWX, &x))
          goto InvalidInstruction;

        if (!check_gp_id(o0, o1, kZR))
          goto InvalidPhysId;

        uint64_t cond = o2.as<Imm>().value_as<uint64_t>();
        if (cond - 2u > 0xEu)
          goto InvalidImmediate;

        opcode.reset(op_data.opcode);
        opcode.add_imm(x, 31);
        opcode.add_reg(o1, 16);
        opcode.add_imm(cond_code_to_opcode_field(uint32_t(cond)) ^ 1u, 12);
        opcode.add_reg(o1, 5);
        opcode.add_reg(o0, 0);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingBaseCSel: {
      const InstDB::EncodingData::BaseCSel& op_data = InstDB::EncodingData::baseCSel[encoding_index];

      if (isign4 == ENC_OPS4(Reg, Reg, Reg, Imm)) {
        uint32_t x;
        if (!check_gp_type(o0, o1, o2, InstDB::kWX, &x))
          goto InvalidInstruction;

        if (!check_gp_id(o0, o1, o2, kZR))
          goto InvalidPhysId;

        uint64_t cond = o3.as<Imm>().value_as<uint64_t>();
        if (cond > 0xFu)
          goto InvalidImmediate;

        opcode.reset(op_data.opcode);
        opcode.add_imm(x, 31);
        opcode.add_reg(o2, 16);
        opcode.add_imm(cond_code_to_opcode_field(uint32_t(cond)), 12);
        opcode.add_reg(o1, 5);
        opcode.add_reg(o0, 0);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingBaseCSet: {
      const InstDB::EncodingData::BaseCSet& op_data = InstDB::EncodingData::baseCSet[encoding_index];

      if (isign4 == ENC_OPS2(Reg, Imm)) {
        uint32_t x;
        if (!check_gp_type(o0, InstDB::kWX, &x))
          goto InvalidInstruction;

        if (!check_gp_id(o0, kZR))
          goto InvalidPhysId;

        uint64_t cond = o1.as<Imm>().value_as<uint64_t>();
        if (cond - 2u >= 0xEu)
          goto InvalidImmediate;

        opcode.reset(op_data.opcode);
        opcode.add_imm(x, 31);
        opcode.add_imm(cond_code_to_opcode_field(uint32_t(cond)) ^ 1u, 12);
        opcode.add_reg(o0, 0);
        goto EmitOp;
      }

      break;
    }

    // ------------------------------------------------------------------------
    // [Base - Min/Max]
    // ------------------------------------------------------------------------

    case InstDB::kEncodingBaseMinMax: {
      const InstDB::EncodingData::BaseMinMax& op_data = InstDB::EncodingData::baseMinMax[encoding_index];

      if (isign4 == ENC_OPS3(Reg, Reg, Reg)) {
        uint32_t x;
        if (!check_gp_type(o0, InstDB::kWX, &x))
          goto InvalidInstruction;

        if (!check_signature(o0, o1, o2))
          goto InvalidInstruction;

        opcode.reset(op_data.register_op);
        opcode.add_imm(x, 31);
        opcode.add_reg(o2, 16);
        opcode.add_reg(o1, 5);
        opcode.add_reg(o0, 0);
        goto EmitOp;
      }

      if (isign4 == ENC_OPS3(Reg, Reg, Imm)) {
        uint32_t x;
        if (!check_gp_type(o0, InstDB::kWX, &x))
          goto InvalidInstruction;

        if (!check_signature(o0, o1))
          goto InvalidInstruction;

        uint64_t imm = o2.as<Imm>().value_as<uint64_t>();

        if (op_data.immediate_op & (1u << 18)) {
          // Zero extend imm.
          if (!Support::is_uint_n<8>(imm)) {
            goto InvalidImmediate;
          }
        }
        else {
          // Sign extend imm.
          if (!Support::is_int_n<8>(int64_t(imm))) {
            goto InvalidImmediate;
          }
        }

        opcode.reset(op_data.immediate_op);
        opcode.add_imm(x, 31);
        opcode.add_imm(uint32_t(imm & 0xFFu), 10);
        opcode.add_reg(o1, 5);
        opcode.add_reg(o0, 0);
        goto EmitOp;
      }

      break;
    }

    // ------------------------------------------------------------------------
    // [Base - Special]
    // ------------------------------------------------------------------------

    case InstDB::kEncodingBaseAtDcIcTlbi: {
      const InstDB::EncodingData::BaseAtDcIcTlbi& op_data = InstDB::EncodingData::baseAtDcIcTlbi[encoding_index];

      if (isign4 == ENC_OPS1(Imm) || isign4 == ENC_OPS2(Imm, Reg)) {
        if (op_data.mandatory_reg && isign4 != ENC_OPS2(Imm, Reg))
          goto InvalidInstruction;

        if (o0.as<Imm>().value_as<uint64_t>() > 0x7FFFu)
          goto InvalidImmediate;

        uint32_t imm = o0.as<Imm>().value_as<uint32_t>();
        if ((imm & op_data.imm_verify_mask) != op_data.imm_verify_data)
          goto InvalidImmediate;

        uint32_t rt = 31;
        if (o1.is_reg()) {
          if (!o1.as<Reg>().is_gp64())
            goto InvalidInstruction;

          if (!check_gp_id(o1, kZR))
            goto InvalidPhysId;

          rt = o1.id() & 31;
        }

        opcode.reset(0b11010101000010000000000000000000);
        opcode.add_imm(imm, 5);
        opcode.add_reg(rt, 0);
        goto EmitOp;
      }
      break;
    }

    case InstDB::kEncodingBaseMrs: {
      if (isign4 == ENC_OPS2(Reg, Imm)) {
        if (!o0.as<Reg>().is_gp64())
          goto InvalidInstruction;

        if (!check_gp_id(o0, kZR))
          goto InvalidPhysId;

        if (o1.as<Imm>().value_as<uint64_t>() > 0xFFFFu)
          goto InvalidImmediate;

        uint32_t imm = o1.as<Imm>().value_as<uint32_t>();
        if (!(imm & B(15)))
          goto InvalidImmediate;

        opcode.reset(0b11010101001100000000000000000000);
        opcode.add_imm(imm, 5);
        opcode.add_reg(o0, 0);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingBaseMsr: {
      if (isign4 == ENC_OPS2(Imm, Reg)) {
        if (!o1.as<Reg>().is_gp64())
          goto InvalidInstruction;

        if (o0.as<Imm>().value_as<uint64_t>() > 0xFFFFu)
          goto InvalidImmediate;

        uint32_t imm = o0.as<Imm>().value_as<uint32_t>();
        if (!(imm & B(15)))
          goto InvalidImmediate;

        if (!check_gp_id(o1, kZR))
          goto InvalidPhysId;

        opcode.reset(0b11010101000100000000000000000000);
        opcode.add_imm(imm, 5);
        opcode.add_reg(o1, 0);
        goto EmitOp;
      }

      if (isign4 == ENC_OPS2(Imm, Imm)) {
        if (o0.as<Imm>().value_as<uint64_t>() > 0x1Fu)
          goto InvalidImmediate;

        if (o1.as<Imm>().value_as<uint64_t>() > 0xFu)
          goto InvalidImmediate;

        uint32_t op = o0.as<Imm>().value_as<uint32_t>();
        uint32_t crm = o1.as<Imm>().value_as<uint32_t>();

        uint32_t op1 = uint32_t(op) >> 3;
        uint32_t op2 = uint32_t(op) & 0x7u;

        opcode.reset(0b11010101000000000100000000011111);
        opcode.add_imm(op1, 16);
        opcode.add_imm(crm, 8);
        opcode.add_imm(op2, 5);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingBaseSys: {
      if (isign4 == ENC_OPS4(Imm, Imm, Imm, Imm)) {
        if (o0.as<Imm>().value_as<uint64_t>() > 0x7u ||
            o1.as<Imm>().value_as<uint64_t>() > 0xFu ||
            o2.as<Imm>().value_as<uint64_t>() > 0xFu ||
            o3.as<Imm>().value_as<uint64_t>() > 0x7u)
          goto InvalidImmediate;

        uint32_t op1 = o0.as<Imm>().value_as<uint32_t>();
        uint32_t crn = o1.as<Imm>().value_as<uint32_t>();
        uint32_t crm = o2.as<Imm>().value_as<uint32_t>();
        uint32_t op2 = o3.as<Imm>().value_as<uint32_t>();
        uint32_t rt = 31;

        const Operand_& o4 = op_ext[EmitterUtils::kOp4];
        if (o4.is_reg()) {
          if (!o4.as<Reg>().is_gp64())
            goto InvalidInstruction;

          if (!check_gp_id(o4, kZR))
            goto InvalidPhysId;

          rt = o4.id() & 31;
        }
        else if (!o4.is_none()) {
          goto InvalidInstruction;
        }

        opcode.reset(0b11010101000010000000000000000000);
        opcode.add_imm(op1, 16);
        opcode.add_imm(crn, 12);
        opcode.add_imm(crm, 8);
        opcode.add_imm(op2, 5);
        opcode.add_imm(rt, 0);
        goto EmitOp;
      }

      break;
    }

    // ------------------------------------------------------------------------
    // [Base - Branch]
    // ------------------------------------------------------------------------

    case InstDB::kEncodingBaseBranchReg: {
      const InstDB::EncodingData::BaseBranchReg& op_data = InstDB::EncodingData::baseBranchReg[encoding_index];

      if (isign4 == ENC_OPS1(Reg)) {
        if (!o0.as<Reg>().is_gp64())
          goto InvalidInstruction;

        if (!check_gp_id(o0, kZR))
          goto InvalidPhysId;

        opcode.reset(op_data.opcode);
        opcode.add_reg(o0, 5);
        goto EmitOp;
      }

      break;
    }

    case InstDB::kEncodingBaseBranchRel: {
      const InstDB::EncodingData::BaseBranchRel& op_data = InstDB::EncodingData::baseBranchRel[encoding_index];

      if (isign4 == ENC_OPS1(Label) || isign4 == ENC_OPS1(Imm)) {
        opcode.reset(op_data.opcode);
        rm_rel = &o0;

        // A variation that uses Cond code (or where Cond code is forced like BC.<cond>).
        if (inst_cc != CondCode::kAL || Support::bit_test(opcode.v, 30)) {
          if (opcode.has_x()) {
            // Condition code cannot be applied when the instruction has X bit set (this would be BL instruction).
            goto InvalidInstruction;
          }

          opcode |= B(30);
          opcode.add_imm(cond_code_to_opcode_field(uint32_t(inst_cc)), 0);
          offset_format.reset_to_imm_value(OffsetType::kSignedOffset, 4, 5, 19, 2);
          goto EmitOp_Rel;
        }

        offset_format.reset_to_imm_value(OffsetType::kSignedOffset, 4, 0, 26, 2);
        goto EmitOp_Rel;
      }

      break;
    }

    case InstDB::kEncodingBaseBranchCmp: {
      const InstDB::EncodingData::BaseBranchCmp& op_data = InstDB::EncodingData::baseBranchCmp[encoding_index];

      if (isign4 == ENC_OPS2(Reg, Label) || isign4 == ENC_OPS2(Reg, Imm)) {
        uint32_t x;
        if (!check_gp_type(o0, kWX, &x))
          goto InvalidInstruction;

        if (!check_gp_id(o0, kZR))
          goto InvalidPhysId;

        opcode.reset(op_data.opcode);
        opcode.add_imm(x, 31);
        opcode.add_reg(o0, 0);
        offset_format.reset_to_imm_value(OffsetType::kSignedOffset, 4, 5, 19, 2);

        rm_rel = &o1;
        goto EmitOp_Rel;
      }

      break;
    }

    case InstDB::kEncodingBaseBranchTst: {
      const InstDB::EncodingData::BaseBranchTst& op_data = InstDB::EncodingData::baseBranchTst[encoding_index];

      if (isign4 == ENC_OPS3(Reg, Imm, Label) || isign4 == ENC_OPS3(Reg, Imm, Imm)) {
        uint32_t x;
        if (!check_gp_type(o0, kWX, &x))
          goto InvalidInstruction;

        if (!check_gp_id(o0, kZR))
          goto InvalidPhysId;

        uint64_t imm = o1.as<Imm>().value_as<uint64_t>();

        opcode.reset(op_data.opcode);
        if (imm >= 32) {
          if (!x)
            goto InvalidImmediate;
          opcode.add_imm(x, 31);
          imm &= 0x1F;
        }

        opcode.add_reg(o0, 0);
        opcode.add_imm(imm, 19);
        offset_format.reset_to_imm_value(OffsetType::kSignedOffset, 4, 5, 14, 2);

        rm_rel = &o2;
        goto EmitOp_Rel;
      }

      break;
    }

    // ------------------------------------------------------------------------
    // [Base - Prefetch]
    // ------------------------------------------------------------------------

    case InstDB::kEncodingBasePrfm: {
      const InstDB::EncodingData::BasePrfm& op_data = InstDB::EncodingData::basePrfm[encoding_index];

      if (isign4 == ENC_OPS2(Imm, Mem)) {
        const Mem& m = o1.as<Mem>();
        rm_rel = &m;

        uint32_t imm_shift = 3u;

        if (o0.as<Imm>().value_as<uint64_t>() > 0x1Fu)
          goto InvalidImmediate;

        if (!check_mem_base_index_rel(m))
          goto InvalidAddress;

        int64_t offset = m.offset();
        uint32_t prfop = o0.as<Imm>().value_as<uint32_t>();

        if (m.has_base_reg()) {
          // [Base {Offset | Index}]
          if (m.has_index()) {
            uint32_t opt = shift_op_to_ld_st_opt_map[size_t(m.shift_op())];
            if (opt == 0xFF)
              goto InvalidAddress;

            uint32_t shift = m.shift();
            uint32_t s = shift != 0;

            if (s && shift != imm_shift)
              goto InvalidAddressScale;

            opcode.reset(uint32_t(op_data.register_op) << 21);
            opcode.add_imm(opt, 13);
            opcode.add_imm(s, 12);
            opcode |= B(11);
            opcode.add_imm(prfop, 0);
            goto EmitOp_MemBaseIndex_Rn5_Rm16;
          }

          if (!Support::is_int_n<32>(offset))
            goto InvalidDisplacement;

          int32_t offset32 = int32_t(offset);

          if (m.is_pre_or_post())
            goto InvalidAddress;

          uint32_t imm12 = uint32_t(offset32) >> imm_shift;

          if (Support::is_uint_n<12>(imm12) && (imm12 << imm_shift) == uint32_t(offset32)) {
            opcode.reset(uint32_t(op_data.s_offset_op) << 22);
            opcode.add_imm(imm12, 10);
            opcode.add_imm(prfop, 0);
            goto EmitOp_MemBase_Rn5;
          }

          if (Support::is_int_n<9>(offset32)) {
            opcode.reset(uint32_t(op_data.u_offset_op) << 21);
            opcode.add_imm(uint32_t(offset32) & 0x1FFu, 12);
            opcode.add_imm(prfop, 0);
            goto EmitOp_MemBase_Rn5;
          }

          goto InvalidAddress;
        }
        else {
          opcode.reset(uint32_t(op_data.literal_op) << 24);
          opcode.add_imm(prfop, 0);
          offset_format.reset_to_imm_value(OffsetType::kSignedOffset, 4, 5, 19, 2);
          goto EmitOp_Rel;
        }
      }

      break;
    }

    // ------------------------------------------------------------------------
    // [Base - Load / Store]
    // ------------------------------------------------------------------------

    case InstDB::kEncodingBaseLdSt: {
      const InstDB::EncodingData::BaseLdSt& op_data = InstDB::EncodingData::baseLdSt[encoding_index];

      if (isign4 == ENC_OPS2(Reg, Mem)) {
        const Mem& m = o1.as<Mem>();
        rm_rel = &m;

        uint32_t x;
        if (!check_gp_type(o0, op_data.reg_type, &x))
          goto InvalidInstruction;

        if (!check_gp_id(o0, kZR))
          goto InvalidPhysId;

        // Instructions that work with either word or dword have the unsigned
        // offset shift set to 2 (word), so we set it to 3 (dword) if this is
        // X version of the instruction.
        uint32_t x_shift_mask = uint32_t(op_data.u_offset_shift == 2);
        uint32_t imm_shift = uint32_t(op_data.u_offset_shift) + (x & x_shift_mask);

        if (!check_mem_base_index_rel(m))
          goto InvalidAddress;

        int64_t offset = m.offset();
        if (m.has_base_reg()) {
          // [Base {Offset | Index}]
          if (m.has_index()) {
            uint32_t opt = shift_op_to_ld_st_opt_map[size_t(m.shift_op())];
            if (opt == 0xFF)
              goto InvalidAddress;

            uint32_t shift = m.shift();
            uint32_t s = shift != 0;

            if (s && shift != imm_shift)
              goto InvalidAddressScale;

            opcode.reset(uint32_t(op_data.register_op) << 21);
            opcode.xor_imm(x, op_data.x_offset);
            opcode.add_imm(opt, 13);
            opcode.add_imm(s, 12);
            opcode |= B(11);
            opcode.add_reg(o0, 0);
            goto EmitOp_MemBaseIndex_Rn5_Rm16;
          }

          // Makes it easier to work with the offset especially on 32-bit arch.
          if (!Support::is_int_n<32>(offset))
            goto InvalidDisplacement;
          int32_t offset32 = int32_t(offset);

          if (m.is_pre_or_post()) {
            if (!Support::is_int_n<9>(offset32))
              goto InvalidDisplacement;

            opcode.reset(uint32_t(op_data.pre_post_op) << 21);
            opcode.xor_imm(x, op_data.x_offset);
            opcode.add_imm(offset32 & 0x1FF, 12);
            opcode.add_imm(m.is_pre_index(), 11);
            opcode |= B(10);
            opcode.add_reg(o0, 0);
            goto EmitOp_MemBase_Rn5;
          }
          else {
            uint32_t imm12 = uint32_t(offset32) >> imm_shift;

            // Alternative form of LDUR/STUR and related instructions as described by AArch64 reference manual:
            //
            // If this instruction is not encodable with scaled unsigned offset, try unscaled signed offset.
            if (!Support::is_uint_n<12>(imm12) || (imm12 << imm_shift) != uint32_t(offset32)) {
              inst_id = op_data.u_alt_inst_id;
              inst_info = &InstDB::_inst_info_table[inst_id];
              encoding_index = inst_info->_encoding_data_index;
              goto Case_BaseLdurStur;
            }

            opcode.reset(uint32_t(op_data.u_offset_op) << 22);
            opcode.xor_imm(x, op_data.x_offset);
            opcode.add_imm(imm12, 10);
            opcode.add_reg(o0, 0);
            goto EmitOp_MemBase_Rn5;
          }
        }
        else {
          if (!op_data.literal_op)
            goto InvalidAddress;

          opcode.reset(uint32_t(op_data.literal_op) << 24);
          opcode.xor_imm(x, op_data.x_offset);
          opcode.add_reg(o0, 0);
          offset_format.reset_to_imm_value(OffsetType::kSignedOffset, 4, 5, 19, 2);
          goto EmitOp_Rel;
        }
      }

      break;
    }

    case InstDB::kEncodingBaseLdpStp: {
      const InstDB::EncodingData::BaseLdpStp& op_data = InstDB::EncodingData::baseLdpStp[encoding_index];

      if (isign4 == ENC_OPS3(Reg, Reg, Mem)) {
        const Mem& m = o2.as<Mem>();
        rm_rel = &m;

        uint32_t x;
        if (!check_gp_type(o0, o1, op_data.reg_type, &x))
          goto InvalidInstruction;

        if (!check_gp_id(o0, o1, kZR))
          goto InvalidPhysId;

        if (m.base_type() != RegType::kGp64 || m.has_index())
          goto InvalidAddress;

        if (m.is_offset_64bit())
          goto InvalidDisplacement;

        uint32_t offset_shift = op_data.offset_shift + x;
        int32_t offset32 = m.offset_lo32() >> offset_shift;

        // Make sure we didn't lose bits by applying the mandatory offset shift.
        if (uint32_t(offset32) << offset_shift != uint32_t(m.offset_lo32()))
          goto InvalidDisplacement;

        // Offset is encoded as 7-bit immediate.
        if (!Support::is_int_n<7>(offset32))
          goto InvalidDisplacement;

        if (m.is_pre_or_post() && offset32 != 0) {
          if (!op_data.pre_post_op)
            goto InvalidAddress;

          opcode.reset(uint32_t(op_data.pre_post_op) << 22);
          opcode.add_imm(m.is_pre_index(), 24);
        }
        else {
          opcode.reset(uint32_t(op_data.offset_op) << 22);
        }

        opcode.add_imm(x, op_data.x_offset);
        opcode.add_imm(offset32 & 0x7F, 15);
        opcode.add_reg(o1, 10);
        opcode.add_reg(o0, 0);
        goto EmitOp_MemBase_Rn5;
      }

      break;
    }

    case InstDB::kEncodingBaseStx: {
      const InstDB::EncodingData::BaseStx& op_data = InstDB::EncodingData::baseStx[encoding_index];

      if (isign4 == ENC_OPS3(Reg, Reg, Mem)) {
        const Mem& m = o2.as<Mem>();
        uint32_t x;

        if (!o0.as<Reg>().is_gp32() || !check_gp_type(o1, op_data.reg_type, &x))
          goto InvalidInstruction;

        if (!check_gp_id(o0, o1, kZR))
          goto InvalidPhysId;

        opcode.reset(op_data.opcode());
        opcode.add_imm(x, op_data.x_offset);
        opcode.add_reg(o0, 16);
        opcode.add_reg(o1, 0);

        rm_rel = &m;
        goto EmitOp_MemBaseNoImm_Rn5;
      }

      break;
    }

    case InstDB::kEncodingBaseLdxp: {
      const InstDB::EncodingData::BaseLdxp& op_data = InstDB::EncodingData::baseLdxp[encoding_index];

      if (isign4 == ENC_OPS3(Reg, Reg, Mem)) {
        const Mem& m = o2.as<Mem>();
        uint32_t x;

        if (!check_gp_type(o0, op_data.reg_type, &x) || !check_signature(o0, o1))
          goto InvalidInstruction;

        if (!check_gp_id(o0, o1, kZR))
          goto InvalidPhysId;

        opcode.reset(op_data.opcode());
        opcode.add_imm(x, op_data.x_offset);
        opcode.add_reg(o1, 10);
        opcode.add_reg(o0, 0);

        rm_rel = &m;
        goto EmitOp_MemBaseNoImm_Rn5;
      }

      break;
    }

    case InstDB::kEncodingBaseStxp: {
      const InstDB::EncodingData::BaseStxp& op_data = InstDB::EncodingData::baseStxp[encoding_index];

      if (isign4 == ENC_OPS4(Reg, Reg, Reg, Mem)) {
        const Mem& m = o3.as<Mem>();
        uint32_t x;

        if (!o0.as<Reg>().is_gp32() || !check_gp_type(o1, op_data.reg_type, &x) || !check_signature(o1, o2))
          goto InvalidInstruction;

        if (!check_gp_id(o0, o1, o2, kZR))
          goto InvalidPhysId;

        opcode.reset(op_data.opcode());
        opcode.add_imm(x, op_data.x_offset);
        opcode.add_reg(o0, 16);
        opcode.add_reg(o2, 10);
        opcode.add_reg(o1, 0);

        rm_rel = &m;
        goto EmitOp_MemBaseNoImm_Rn5;
      }

      break;
    }

    case InstDB::kEncodingBaseRM_NoImm: {
      const InstDB::EncodingData::BaseRM_NoImm& op_data = InstDB::EncodingData::baseRM_NoImm[encoding_index];

      if (isign4 == ENC_OPS2(Reg, Mem)) {
        const Mem& m = o1.as<Mem>();
        rm_rel = &m;

        uint32_t x;
        if (!check_gp_type(o0, op_data.reg_type, &x))
          goto InvalidInstruction;

        if (!check_gp_id(o0, op_data.reg_hi_id))
          goto InvalidPhysId;

        opcode.reset(op_data.opcode());
        opcode.add_imm(x, op_data.x_offset);
        opcode.add_reg(o0, 0);
        goto EmitOp_MemBaseNoImm_Rn5;
      }

      break;
    }

    case InstDB::kEncodingBaseRM_SImm9: {
Case_BaseLdurStur:
      const InstDB::EncodingData::BaseRM_SImm9& op_data = InstDB::EncodingData::baseRM_SImm9[encoding_index];

      if (isign4 == ENC_OPS2(Reg, Mem)) {
        const Mem& m = o1.as<Mem>();
        rm_rel = &m;

        uint32_t x;
        if (!check_gp_type(o0, op_data.reg_type, &x))
          goto InvalidInstruction;

        if (!check_gp_id(o0, op_data.reg_hi_id))
          goto InvalidPhysId;

        if (m.has_base_reg() && !m.has_index()) {
          if (m.is_offset_64bit())
            goto InvalidDisplacement;

          int32_t offset32 = m.offset_lo32() >> op_data.imm_shift;
          if (Support::shl(offset32, op_data.imm_shift) != m.offset_lo32())
            goto InvalidDisplacement;

          if (!Support::is_int_n<9>(offset32))
            goto InvalidDisplacement;

          if (m.is_fixed_offset()) {
            opcode.reset(op_data.offset_op());
          }
          else {
            if (!op_data.pre_post_op())
              goto InvalidInstruction;

            opcode.reset(op_data.pre_post_op());
            opcode.xor_imm(m.is_pre_index(), 11);
          }

          opcode.xor_imm(x, op_data.x_offset);
          opcode.add_imm(offset32 & 0x1FF, 12);
          opcode.add_reg(o0, 0);
          goto EmitOp_MemBase_Rn5;
        }

        goto InvalidAddress;
      }

      break;
    }

    case InstDB::kEncodingBaseRM_SImm10: {
      const InstDB::EncodingData::BaseRM_SImm10& op_data = InstDB::EncodingData::baseRM_SImm10[encoding_index];

      if (isign4 == ENC_OPS2(Reg, Mem)) {
        const Mem& m = o1.as<Mem>();
        rm_rel = &m;

        uint32_t x;
        if (!check_gp_type(o0, op_data.reg_type, &x))
          goto InvalidInstruction;

        if (!check_gp_id(o0, op_data.reg_hi_id))
          goto InvalidPhysId;

        if (m.has_base_reg() && !m.has_index()) {
          if (m.is_offset_64bit())
            goto InvalidDisplacement;

          int32_t offset32 = m.offset_lo32() >> op_data.imm_shift;
          if (Support::shl(offset32, op_data.imm_shift) != m.offset_lo32())
            goto InvalidDisplacement;

          if (!Support::is_int_n<10>(offset32))
            goto InvalidDisplacement;

          if (m.is_post_index())
            goto InvalidAddress;

          // Offset has 10 bits, sign is stored in the 10th bit.
          offset32 &= 0x3FF;

          opcode.reset(op_data.opcode());
          opcode.xor_imm(m.is_pre_index(), 11);
          opcode.xor_imm(x, op_data.x_offset);
          opcode.add_imm(offset32 >> 9, 22);
          opcode.add_imm(offset32, 12);
          opcode.add_reg(o0, 0);
          goto EmitOp_MemBase_Rn5;
        }

        goto InvalidAddress;
      }

      break;
    }

    case InstDB::kEncodingBaseAtomicOp: {
      const InstDB::EncodingData::BaseAtomicOp& op_data = InstDB::EncodingData::baseAtomicOp[encoding_index];

      if (isign4 == ENC_OPS3(Reg, Reg, Mem)) {
        const Mem& m = o2.as<Mem>();
        uint32_t x;

        if (!check_gp_type(o0, op_data.reg_type, &x) || !check_signature(o0, o1))
          goto InvalidInstruction;

        if (!check_gp_id(o0, o1, kZR))
          goto InvalidInstruction;

        opcode.reset(op_data.opcode());
        opcode.add_imm(x, op_data.x_offset);
        opcode.add_reg(o0, 16);
        opcode.add_reg(o1, 0);

        rm_rel = &m;
        goto EmitOp_MemBaseNoImm_Rn5;
      }

      break;
    }

    case InstDB::kEncodingBaseAtomicSt: {
      const InstDB::EncodingData::BaseAtomicSt& op_data = InstDB::EncodingData::baseAtomicSt[encoding_index];

      if (isign4 == ENC_OPS2(Reg, Mem)) {
        const Mem& m = o1.as<Mem>();
        uint32_t x;

        if (!check_gp_type(o0, op_data.reg_type, &x))
          goto InvalidInstruction;

        if (!check_gp_id(o0, kZR))
          goto InvalidPhysId;

        opcode.reset(op_data.opcode());
        opcode.add_imm(x, op_data.x_offset);
        opcode.add_reg(o0, 16);
        opcode.add_reg(Gp::kIdZr, 0);

        rm_rel = &m;
        goto EmitOp_MemBaseNoImm_Rn5;
      }

      break;
    }

    case InstDB::kEncodingBaseAtomicCasp: {
      const InstDB::EncodingData::BaseAtomicCasp& op_data = InstDB::EncodingData::baseAtomicCasp[encoding_index];
      const Operand_& o4 = op_ext[EmitterUtils::kOp4];

      if (isign4 == ENC_OPS4(Reg, Reg, Reg, Reg) && o4.is_mem()) {
        const Mem& m = o4.as<Mem>();
        uint32_t x;

        if (!check_gp_type(o0, op_data.reg_type, &x))
          goto InvalidInstruction;

        if (!check_signature(o0, o1, o2, o3))
          goto InvalidInstruction;

        if (!check_even(o0, o2) || !check_gp_id(o0, o2, kZR))
          goto InvalidPhysId;

        if (!check_consecutive(o0, o1) || !check_consecutive(o2, o3))
          goto InvalidPhysId;

        opcode.reset(op_data.opcode());
        opcode.add_imm(x, op_data.x_offset);
        opcode.add_reg(o0, 16);
        opcode.add_reg(o2, 0);

        rm_rel = &m;
        goto EmitOp_MemBaseNoImm_Rn5;
      }

      break;
    }

    // ------------------------------------------------------------------------
    // [FSimd - Instructions]
    // ------------------------------------------------------------------------

    case InstDB::kEncodingFSimdSV: {
      const InstDB::EncodingData::FSimdSV& op_data = InstDB::EncodingData::fSimdSV[encoding_index];

      if (isign4 == ENC_OPS2(Reg, Reg)) {
        uint32_t q = diff(o1.as<Reg>().reg_type(), RegType::kVec64);
        if (q > 1)
          goto InvalidInstruction;

        if (o0.as<Vec>().has_element_type())
          goto InvalidInstruction;

        // This operation is only defined for:
        //   hD, vS.{4|8}h (16-bit)
        //   sD, vS.4s     (32-bit)
        uint32_t sz = diff(o0.as<Reg>().reg_type(), RegType::kVec16);
        uint32_t element_sz = diff(o1.as<Vec>().element_type(), VecElementType::kH);

        // Size greater than 1 means 64-bit elements, not supported.
        if ((sz | element_sz) > 1 || sz != element_sz)
          goto InvalidInstruction;

        // Size 1 (32-bit float) requires at least 4 elements.
        if (sz && !q)
          goto InvalidInstruction;

        // Bit flipping according to sz.
        static const uint32_t sz_bits_table[] = { B(29), 0 };

        opcode.reset(op_data.opcode << 10);
        opcode ^= sz_bits_table[sz];
        opcode.add_imm(q, 30);
        goto EmitOp_Rd0_Rn5;
      }

      break;
    }

    case InstDB::kEncodingFSimdVV: {
      const InstDB::EncodingData::FSimdVV& op_data = InstDB::EncodingData::fSimdVV[encoding_index];

      if (isign4 == ENC_OPS2(Reg, Reg)) {
        if (!match_signature(o0, o1, inst_flags))
          goto InvalidInstruction;

        if (!pick_fp_opcode(o0.as<Vec>(), op_data.scalar_op(), op_data.scalar_hf(), op_data.vector_op(), op_data.vector_hf(), &opcode))
          goto InvalidInstruction;

        goto EmitOp_Rd0_Rn5;
      }

      break;
    }

    case InstDB::kEncodingFSimdVVV: {
      const InstDB::EncodingData::FSimdVVV& op_data = InstDB::EncodingData::fSimdVVV[encoding_index];

      if (isign4 == ENC_OPS3(Reg, Reg, Reg)) {
        if (!match_signature(o0, o1, o2, inst_flags))
          goto InvalidInstruction;

        if (!pick_fp_opcode(o0.as<Vec>(), op_data.scalar_op(), op_data.scalar_hf(), op_data.vector_op(), op_data.vector_hf(), &opcode))
          goto InvalidInstruction;

        goto EmitOp_Rd0_Rn5_Rm16;
      }

      break;
    }

    case InstDB::kEncodingFSimdVVVe: {
      const InstDB::EncodingData::FSimdVVVe& op_data = InstDB::EncodingData::fSimdVVVe[encoding_index];

      if (isign4 == ENC_OPS3(Reg, Reg, Reg)) {
        if (!o2.as<Vec>().has_element_index()) {
          if (!match_signature(o0, o1, o2, inst_flags))
            goto InvalidInstruction;

          if (!pick_fp_opcode(o0.as<Vec>(), op_data.scalar_op(), op_data.scalar_hf(), op_data.vector_op(), op_data.vector_hf(), &opcode))
            goto InvalidInstruction;

          goto EmitOp_Rd0_Rn5_Rm16;
        }
        else {
          if (!match_signature(o0, o1, inst_flags))
            goto InvalidInstruction;

          uint32_t q = o1.as<Reg>().is_vec128();
          uint32_t sz;

          if (!pick_fp_opcode(o0.as<Vec>(), op_data.element_scalar_op(), InstDB::kHF_D, op_data.element_vector_op(), InstDB::kHF_D, &opcode, &sz))
            goto InvalidInstruction;

          if (sz == 0 && o2.as<Reg>().id() > 15)
            goto InvalidPhysId;

          uint32_t element_index = o2.as<Vec>().element_index();
          if (element_index > (7u >> sz))
            goto InvalidElementIndex;

          uint32_t hlm = element_index << sz;
          opcode.add_imm(q, 30);
          opcode.add_imm(hlm & 3u, 20);
          opcode.add_imm(hlm >> 2, 11);
          goto EmitOp_Rd0_Rn5_Rm16;
        }
      }

      break;
    }

    case InstDB::kEncodingFSimdVVVV: {
      const InstDB::EncodingData::FSimdVVVV& op_data = InstDB::EncodingData::fSimdVVVV[encoding_index];

      if (isign4 == ENC_OPS4(Reg, Reg, Reg, Reg)) {
        if (!match_signature(o0, o1, o2, o3, inst_flags))
          goto InvalidInstruction;

        if (!pick_fp_opcode(o0.as<Vec>(), op_data.scalar_op(), op_data.scalar_hf(), op_data.vector_op(), op_data.vector_hf(), &opcode))
          goto InvalidInstruction;

        goto EmitOp_Rd0_Rn5_Rm16_Ra10;
      }

      break;
    }

    case InstDB::kEncodingSimdFcadd: {
      const InstDB::EncodingData::SimdFcadd& op_data = InstDB::EncodingData::simdFcadd[encoding_index];

      if (isign4 == ENC_OPS4(Reg, Reg, Reg, Imm)) {
        if (!check_signature(o0, o1, o2) || o0.as<Vec>().has_element_index())
          goto InvalidInstruction;

        uint32_t q = diff(o0.as<Reg>().reg_type(), RegType::kVec64);
        if (q > 1)
          goto InvalidInstruction;

        uint32_t sz = diff(o0.as<Vec>().element_type(), VecElementType::kB);
        if (sz == 0 || sz > 3)
          goto InvalidInstruction;

        // 0 <- 90deg.
        // 1 <- 270deg.
        uint32_t rot = 0;
        if (o3.as<Imm>().value() == 270)
          rot = 1;
        else if (o3.as<Imm>().value() != 90)
          goto InvalidImmediate;

        opcode.reset(op_data.opcode());
        opcode.add_imm(q, 30);
        opcode.add_imm(sz, 22);
        opcode.add_imm(rot, 12);
        goto EmitOp_Rd0_Rn5_Rm16;
      }

      break;
    }

    case InstDB::kEncodingSimdFccmpFccmpe: {
      const InstDB::EncodingData::SimdFccmpFccmpe& op_data = InstDB::EncodingData::simdFccmpFccmpe[encoding_index];

      if (isign4 == ENC_OPS4(Reg, Reg, Imm, Imm)) {
        uint32_t sz = diff(o0.as<Reg>().reg_type(), RegType::kVec16);
        if (sz > 2)
          goto InvalidInstruction;

        if (!check_signature(o0, o1) || o0.as<Vec>().has_element_type())
          goto InvalidInstruction;

        uint64_t nzcv = o2.as<Imm>().value_as<uint64_t>();
        uint64_t cond = o3.as<Imm>().value_as<uint64_t>();

        if ((nzcv | cond) > 0xFu)
          goto InvalidImmediate;

        uint32_t type = (sz - 1) & 0x3u;

        opcode.reset(op_data.opcode());
        opcode.add_imm(type, 22);
        opcode.add_imm(cond_code_to_opcode_field(uint32_t(cond)), 12);
        opcode.add_imm(nzcv, 0);

        goto EmitOp_Rn5_Rm16;
      }

      break;
    }

    case InstDB::kEncodingSimdFcm: {
      const InstDB::EncodingData::SimdFcm& op_data = InstDB::EncodingData::simdFcm[encoding_index];

      if (isign4 == ENC_OPS3(Reg, Reg, Reg) && op_data.has_register_op()) {
        if (!match_signature(o0, o1, o2, inst_flags))
          goto InvalidInstruction;

        if (!pick_fp_opcode(o0.as<Vec>(), op_data.register_scalar_op(), op_data.register_scalar_hf(), op_data.register_vector_op(), op_data.register_vector_hf(), &opcode))
          goto InvalidInstruction;

        goto EmitOp_Rd0_Rn5_Rm16;
      }

      if (isign4 == ENC_OPS3(Reg, Reg, Imm) && op_data.has_zero_op()) {
        if (!check_signature(o0, o1))
          goto InvalidInstruction;

        if (o2.as<Imm>().value() != 0 || o2.as<Imm>().predicate() != 0)
          goto InvalidImmediate;

        if (!pick_fp_opcode(o0.as<Vec>(), op_data.zero_scalar_op(), InstDB::kHF_B, op_data.zero_vector_op(), InstDB::kHF_B, &opcode))
          goto InvalidInstruction;

        goto EmitOp_Rd0_Rn5;
      }

      break;
    }

    case InstDB::kEncodingSimdFcmla: {
      const InstDB::EncodingData::SimdFcmla& op_data = InstDB::EncodingData::simdFcmla[encoding_index];

      if (isign4 == ENC_OPS4(Reg, Reg, Reg, Imm)) {
        if (!check_signature(o0, o1))
          goto InvalidInstruction;

        uint32_t q = diff(o0.as<Reg>().reg_type(), RegType::kVec64);
        if (q > 1)
          goto InvalidInstruction;

        uint32_t sz = diff(o0.as<Vec>().element_type(), VecElementType::kB);
        if (sz == 0 || sz > 3)
          goto InvalidInstruction;

        uint32_t rot = 0;
        switch (o3.as<Imm>().value()) {
          case 0  : rot = 0; break;
          case 90 : rot = 1; break;
          case 180: rot = 2; break;
          case 270: rot = 3; break;
          default:
            goto InvalidImmediate;
        }

        if (!o2.as<Vec>().has_element_index()) {
          if (!check_signature(o1, o2))
            goto InvalidInstruction;

          opcode.reset(op_data.regular_op());
          opcode.add_imm(q, 30);
          opcode.add_imm(sz, 22);
          opcode.add_imm(rot, 11);
          goto EmitOp_Rd0_Rn5_Rm16;
        }
        else {
          if (o0.as<Vec>().element_type() != o2.as<Vec>().element_type())
            goto InvalidInstruction;

          // Only allowed vectors are: 4H, 8H, and 4S.
          if (!(sz == 1 || (q == 1 && sz == 2)))
            goto InvalidInstruction;

          // Element index ranges:
          //   4H - ElementIndex[0..1] (index 2..3 is UNDEFINED).
          //   8H - ElementIndex[0..3].
          //   4S - ElementIndex[0..1].
          uint32_t element_index = o2.as<Vec>().element_index();
          uint32_t hl_field_shift = sz == 1 ? 0u : 1u;
          uint32_t max_element_index = q == 1 && sz == 1 ? 3u : 1u;

          if (element_index > max_element_index)
            goto InvalidElementIndex;

          uint32_t hl = element_index << hl_field_shift;

          opcode.reset(op_data.element_op());
          opcode.add_imm(q, 30);
          opcode.add_imm(sz, 22);
          opcode.add_imm(hl & 1u, 21); // L field.
          opcode.add_imm(hl >> 1, 11); // H field.
          opcode.add_imm(rot, 13);
          goto EmitOp_Rd0_Rn5_Rm16;
        }
      }

      break;
    }

    case InstDB::kEncodingSimdFcmpFcmpe: {
      const InstDB::EncodingData::SimdFcmpFcmpe& op_data = InstDB::EncodingData::simdFcmpFcmpe[encoding_index];

      uint32_t sz = diff(o0.as<Reg>().reg_type(), RegType::kVec16);
      uint32_t type = (sz - 1) & 0x3u;

      if (sz > 2)
        goto InvalidInstruction;

      if (o0.as<Vec>().has_element_type())
        goto InvalidInstruction;

      opcode.reset(op_data.opcode());
      opcode.add_imm(type, 22);

      if (isign4 == ENC_OPS2(Reg, Reg)) {
        if (!check_signature(o0, o1))
          goto InvalidInstruction;

        goto EmitOp_Rn5_Rm16;
      }

      if (isign4 == ENC_OPS2(Reg, Imm)) {
        if (o1.as<Imm>().value() != 0 || o1.as<Imm>().predicate() != 0)
          goto InvalidInstruction;

        opcode |= B(3);
        goto EmitOp_Rn5;
      }

      break;
    }

    case InstDB::kEncodingSimdFcsel: {
      if (isign4 == ENC_OPS4(Reg, Reg, Reg, Imm)) {
        if (!check_signature(o0, o1, o2))
          goto InvalidInstruction;

        uint32_t sz = diff(o0.as<Reg>().reg_type(), RegType::kVec16);
        uint32_t type = (sz - 1) & 0x3u;

        if (sz > 2 || o0.as<Vec>().has_element_type())
          goto InvalidInstruction;

        uint64_t cond = o3.as<Imm>().value_as<uint64_t>();
        if (cond > 0xFu)
          goto InvalidImmediate;

        opcode.reset(0b00011110001000000000110000000000);
        opcode.add_imm(type, 22);
        opcode.add_imm(cond_code_to_opcode_field(uint32_t(cond)), 12);
        goto EmitOp_Rd0_Rn5_Rm16;
      }

      break;
    }

    case InstDB::kEncodingSimdFcvt: {
      if (isign4 == ENC_OPS2(Reg, Reg)) {
        uint32_t dst_sz = diff(o0.as<Reg>().reg_type(), RegType::kVec16);
        uint32_t src_sz = diff(o1.as<Reg>().reg_type(), RegType::kVec16);

        if ((dst_sz | src_sz) > 3)
          goto InvalidInstruction;

        if (o0.as<Vec>().has_element_type() || o1.as<Vec>().has_element_type())
          goto InvalidInstruction;

        // Table that provides 'type' and 'opc' according to the dst/src combination.
        static const uint8_t table[] = {
          0xFFu, // H <- H (Invalid).
          0x03u, // H <- S (type=00 opc=11).
          0x13u, // H <- D (type=01 opc=11).
          0xFFu, // H <- Q (Invalid).
          0x30u, // S <- H (type=11 opc=00).
          0xFFu, // S <- S (Invalid).
          0x10u, // S <- D (type=01 opc=00).
          0xFFu, // S <- Q (Invalid).
          0x31u, // D <- H (type=11 opc=01).
          0x01u, // D <- S (type=00 opc=01).
          0xFFu, // D <- D (Invalid).
          0xFFu, // D <- Q (Invalid).
          0xFFu, // Q <- H (Invalid).
          0xFFu, // Q <- S (Invalid).
          0xFFu, // Q <- D (Invalid).
          0xFFu  // Q <- Q (Invalid).
        };

        uint32_t type_opc = table[(dst_sz << 2) | src_sz];
        opcode.reset(0b0001111000100010010000 << 10);
        opcode.add_imm(type_opc >> 4, 22);
        opcode.add_imm(type_opc & 15, 15);
        goto EmitOp_Rd0_Rn5;
      }

      break;
    }

    case InstDB::kEncodingSimdFcvtLN: {
      const InstDB::EncodingData::SimdFcvtLN& op_data = InstDB::EncodingData::simdFcvtLN[encoding_index];

      if (isign4 == ENC_OPS2(Reg, Reg)) {
        // Scalar form - only FCVTXN.
        if (o0.as<Vec>().is_vec32() && o1.as<Vec>().is_vec64()) {
          if (!op_data.has_scalar())
            goto InvalidInstruction;

          if (o0.as<Vec>().has_element_type() || o1.as<Vec>().has_element_type())
            goto InvalidInstruction;

          opcode.reset(op_data.scalar_op());
          opcode |= B(22); // sz bit must be 1, the only supported combination of FCVTXN.
          goto EmitOp_Rd0_Rn5;
        }

        opcode.reset(op_data.vector_op());

        const Vec& rl = (inst_flags & InstDB::kInstFlagLong) ? o0.as<Vec>() : o1.as<Vec>();
        const Vec& rn = (inst_flags & InstDB::kInstFlagLong) ? o1.as<Vec>() : o0.as<Vec>();

        uint32_t q = diff(rn.reg_type(), RegType::kVec64);
        if (uint32_t(opcode.has_q()) != q)
          goto InvalidInstruction;

        if (rl.is_vec_s4() && rn.element_type() == VecElementType::kH && !op_data.is_cvtxn()) {
          goto EmitOp_Rd0_Rn5;
        }

        if (rl.is_vec_d2() && rn.element_type() == VecElementType::kS) {
          opcode |= B(22);
          goto EmitOp_Rd0_Rn5;
        }
      }

      break;
    }

    case InstDB::kEncodingSimdFcvtSV: {
      const InstDB::EncodingData::SimdFcvtSV& op_data = InstDB::EncodingData::simdFcvtSV[encoding_index];

      // So we can support both IntToFloat and FloatToInt conversions.
      const Operand_& op_gp = op_data.is_float_to_int() ? o0 : o1;
      const Operand_& op_vec = op_data.is_float_to_int() ? o1 : o0;

      if (isign4 == ENC_OPS2(Reg, Reg)) {
        if (op_gp.as<Reg>().is_gp() && op_vec.as<Reg>().is_vec()) {
          uint32_t x = op_gp.as<Reg>().is_gp64();
          uint32_t type = diff(op_vec.as<Reg>().reg_type(), RegType::kVec16);

          if (type > 2u)
            goto InvalidInstruction;

          type = (type - 1u) & 0x3;
          opcode.reset(op_data.general_op());
          opcode.add_imm(type, 22);
          opcode.add_imm(x, 31);
          goto EmitOp_Rd0_Rn5;
        }

        if (o0.as<Reg>().is_vec() && o1.as<Reg>().is_vec()) {
          if (!check_signature(o0, o1))
            goto InvalidInstruction;

          if (!pick_fp_opcode(o0.as<Vec>(), op_data.scalar_int_op(), InstDB::kHF_B, op_data.vector_int_op(), InstDB::kHF_B, &opcode))
            goto InvalidInstruction;

          goto EmitOp_Rd0_Rn5;
        }
      }

      if (isign4 == ENC_OPS3(Reg, Reg, Imm) && op_data.is_fixed_point()) {
        if (o2.as<Imm>().value_as<uint64_t>() >= 64)
          goto InvalidInstruction;

        uint32_t scale = o2.as<Imm>().value_as<uint32_t>();
        if (scale == 0)
          goto InvalidInstruction;

        if (op_gp.as<Reg>().is_gp() && op_vec.as<Reg>().is_vec()) {
          uint32_t x = op_gp.as<Reg>().is_gp64();
          uint32_t type = diff(op_vec.as<Reg>().reg_type(), RegType::kVec16);

          uint32_t scale_limit = 32u << x;
          if (scale > scale_limit)
            goto InvalidInstruction;

          type = (type - 1u) & 0x3;
          opcode.reset(op_data.general_op() ^ B(21));
          opcode.add_imm(type, 22);
          opcode.add_imm(x, 31);
          opcode.add_imm(64u - scale, 10);
          goto EmitOp_Rd0_Rn5;
        }

        if (o0.as<Reg>().is_vec() && o1.as<Reg>().is_vec()) {
          if (!check_signature(o0, o1))
            goto InvalidInstruction;

          uint32_t sz;
          if (!pick_fp_opcode(o0.as<Vec>(), op_data.scalar_fp_op(), InstDB::kHF_0, op_data.vector_fp_op(), InstDB::kHF_0, &opcode, &sz))
            goto InvalidInstruction;

          uint32_t scale_limit = 16u << sz;
          if (scale > scale_limit)
            goto InvalidInstruction;

          uint32_t imm = Support::neg(scale) & Support::lsb_mask<uint32_t>(sz + 4 + 1);
          opcode.add_imm(imm, 16);
          goto EmitOp_Rd0_Rn5;
        }
      }

      break;
    }

    case InstDB::kEncodingSimdFmlal: {
      const InstDB::EncodingData::SimdFmlal& op_data = InstDB::EncodingData::simdFmlal[encoding_index];

      if (isign4 == ENC_OPS3(Reg, Reg, Reg)) {
        uint32_t q = diff(o0.as<Reg>().reg_type(), RegType::kVec64);
        uint32_t q_is_optional = op_data.optional_q();

        if (q_is_optional) {
          // This instruction works with either 64-bit or 128-bit registers,
          // encoded by Q bit.
          if (q > 1)
            goto InvalidInstruction;
        }
        else {
          // This instruction requires 128-bit vector registers.
          if (q != 1)
            goto InvalidInstruction;

          // The instruction is ehtier B (bottom) or T (top), which is part of
          // the opcode, which uses Q bit, so we have to clear it explicitly.
          q = 0;
        }

        if (uint32_t(o0.as<Reg>().reg_type()) != uint32_t(o1.as<Reg>().reg_type()) + q_is_optional ||
            uint32_t(o0.as<Vec>().element_type()) != op_data.ta ||
            uint32_t(o1.as<Vec>().element_type()) != op_data.tb)
          goto InvalidInstruction;

        if (!o2.as<Vec>().has_element_index()) {
          if (!check_signature(o1, o2))
            goto InvalidInstruction;

          opcode.reset(op_data.vector_op());
          opcode.add_imm(q, 30);
          goto EmitOp_Rd0_Rn5_Rm16;
        }
        else {
          if (uint32_t(o2.as<Vec>().element_type()) != op_data.tElement)
            goto InvalidInstruction;

          if (o2.as<Reg>().id() > 15)
            goto InvalidPhysId;

          uint32_t element_index = o2.as<Vec>().element_index();
          if (element_index > 7u)
            goto InvalidElementIndex;

          opcode.reset(op_data.element_op());
          opcode.add_imm(q, 30);
          opcode.add_imm(element_index & 3u, 20);
          opcode.add_imm(element_index >> 2, 11);
          goto EmitOp_Rd0_Rn5_Rm16;
        }
      }

      break;
    }

    case InstDB::kEncodingSimdFmov: {
      if (isign4 == ENC_OPS2(Reg, Reg)) {
        // FMOV Gp <-> Vec opcode:
        opcode.reset(0b00011110001001100000000000000000);

        if (o0.as<Reg>().is_gp() && o1.as<Reg>().is_vec()) {
          // FMOV Wd, Hn      (sf=0 type=11 rmode=00 op=110)
          // FMOV Xd, Hn      (sf=1 type=11 rmode=00 op=110)
          // FMOV Wd, Sn      (sf=0 type=00 rmode=00 op=110)
          // FMOV Xd, Dn      (sf=1 type=11 rmode=00 op=110)
          // FMOV Xd, Vn.d[1] (sf=1 type=10 rmode=01 op=110)
          uint32_t x = o0.as<Reg>().is_gp64();
          uint32_t sz = diff(o1.as<Reg>().reg_type(), RegType::kVec16);

          uint32_t type = (sz - 1) & 0x3u;
          uint32_t r_mode_op = 0b00110;

          if (o1.as<Vec>().has_element_index()) {
            // Special case.
            if (!x || !o1.as<Vec>().is_vec_d2() || o1.as<Vec>().element_index() != 1)
              goto InvalidInstruction;
            type = 0b10;
            r_mode_op = 0b01110;
          }
          else {
            // Must be scalar.
            if (sz > 2)
              goto InvalidInstruction;

            if (o1.as<Vec>().has_element_type())
              goto InvalidInstruction;

            if (o1.as<Vec>().is_vec32() && x)
              goto InvalidInstruction;

            if (o1.as<Vec>().is_vec64() && !x)
              goto InvalidInstruction;
          }

          opcode.add_imm(x, 31);
          opcode.add_imm(type, 22);
          opcode.add_imm(r_mode_op, 16);
          goto EmitOp_Rd0_Rn5;
        }

        if (o0.as<Reg>().is_vec() && o1.as<Reg>().is_gp()) {
          // FMOV Hd, Wn      (sf=0 type=11 rmode=00 op=111)
          // FMOV Hd, Xn      (sf=1 type=11 rmode=00 op=111)
          // FMOV Sd, Wn      (sf=0 type=00 rmode=00 op=111)
          // FMOV Dd, Xn      (sf=1 type=11 rmode=00 op=111)
          // FMOV Vd.d[1], Xn (sf=1 type=10 rmode=01 op=111)
          uint32_t x = o1.as<Reg>().is_gp64();
          uint32_t sz = diff(o0.as<Reg>().reg_type(), RegType::kVec16);

          uint32_t type = (sz - 1) & 0x3u;
          uint32_t r_mode_op = 0b00111;

          if (o0.as<Vec>().has_element_index()) {
            // Special case.
            if (!x || !o0.as<Vec>().is_vec_d2() || o0.as<Vec>().element_index() != 1)
              goto InvalidInstruction;
            type = 0b10;
            r_mode_op = 0b01111;
          }
          else {
            // Must be scalar.
            if (sz > 2)
              goto InvalidInstruction;

            if (o0.as<Vec>().has_element_type())
              goto InvalidInstruction;

            if (o0.as<Vec>().is_vec32() && x)
              goto InvalidInstruction;

            if (o0.as<Vec>().is_vec64() && !x)
              goto InvalidInstruction;
          }

          opcode.add_imm(x, 31);
          opcode.add_imm(type, 22);
          opcode.add_imm(r_mode_op, 16);
          goto EmitOp_Rd0_Rn5;
        }

        if (check_signature(o0, o1)) {
          uint32_t sz = diff(o0.as<Reg>().reg_type(), RegType::kVec16);
          if (sz > 2)
            goto InvalidInstruction;

          if (o0.as<Vec>().has_element_type())
            goto InvalidInstruction;

          uint32_t type = (sz - 1) & 0x3;
          opcode.reset(0b00011110001000000100000000000000);
          opcode.add_imm(type, 22);
          goto EmitOp_Rd0_Rn5;
        }
      }

      if (isign4 == ENC_OPS2(Reg, Imm)) {
        if (o0.as<Reg>().is_vec()) {
          double fp_value;
          if (o1.as<Imm>().is_double())
            fp_value = o1.as<Imm>().value_as<double>();
          else if (o1.as<Imm>().is_int32())
            fp_value = o1.as<Imm>().value_as<int32_t>();
          else
            goto InvalidImmediate;

          if (!Utils::is_fp64_imm8(fp_value))
            goto InvalidImmediate;

          uint32_t imm8 = Utils::encode_fp64_to_imm8(fp_value);
          if (!o0.as<Vec>().has_element_type()) {
            // FMOV (scalar, immediate).
            uint32_t sz = diff(o0.as<Reg>().reg_type(), RegType::kVec16);
            uint32_t type = (sz - 1u) & 0x3u;

            if (sz > 2)
              goto InvalidInstruction;

            opcode.reset(0b00011110001000000001000000000000);
            opcode.add_imm(type, 22);
            opcode.add_imm(imm8, 13);
            goto EmitOp_Rd0;
          }
          else {
            uint32_t q = diff(o0.as<Vec>().reg_type(), RegType::kVec64);
            uint32_t sz = diff(o0.as<Vec>().element_type(), VecElementType::kH);

            if (q > 1 || sz > 2)
              goto InvalidInstruction;

            static const uint32_t sz_bits_table[3] = { B(11), B(0), B(29) };
            opcode.reset(0b00001111000000001111010000000000);
            opcode ^= sz_bits_table[sz];
            opcode.add_imm(q, 30);
            opcode.add_imm(imm8 >> 5, 16);
            opcode.add_imm(imm8 & 31, 5);
            goto EmitOp_Rd0;
          }
        }
      }

      break;
    }

    case InstDB::kEncodingFSimdPair: {
      const InstDB::EncodingData::FSimdPair& op_data = InstDB::EncodingData::fSimdPair[encoding_index];

      if (isign4 == ENC_OPS2(Reg, Reg)) {
        // This operation is only defined for:
        //   hD, vS.2h (16-bit)
        //   sD, vS.2s (32-bit)
        //   dD, vS.2d (64-bit)
        uint32_t sz = diff(o0.as<Reg>().reg_type(), RegType::kVec16);
        if (sz > 2)
          goto InvalidInstruction;

        static const uint32_t szSignatures[3] = {
          RegTraits<RegType::kVec32>::kSignature | (Vec::kSignatureElementH),
          RegTraits<RegType::kVec64>::kSignature | (Vec::kSignatureElementS),
          RegTraits<RegType::kVec128>::kSignature | (Vec::kSignatureElementD)
        };

        if (o1.signature() != szSignatures[sz])
          goto InvalidInstruction;

        static const uint32_t sz_bits_table[] = { B(29), 0, B(22) };
        opcode.reset(op_data.scalar_op());
        opcode ^= sz_bits_table[sz];
        goto EmitOp_Rd0_Rn5;
      }

      if (isign4 == ENC_OPS3(Reg, Reg, Reg)) {
        if (!check_signature(o0, o1, o2))
          goto InvalidInstruction;

        uint32_t q = diff(o0.as<Reg>().reg_type(), RegType::kVec64);
        if (q > 1)
          goto InvalidInstruction;

        uint32_t sz = diff(o0.as<Vec>().element_type(), VecElementType::kH);
        if (sz > 2)
          goto InvalidInstruction;

        static const uint32_t sz_bits_table[3] = { B(22) | B(21) | B(15) | B(14), 0, B(22) };
        opcode.reset(op_data.vector_op());
        opcode ^= sz_bits_table[sz];
        opcode.add_imm(q, 30);
        goto EmitOp_Rd0_Rn5_Rm16;
      }

      break;
    }

    // ------------------------------------------------------------------------
    // [ISimd - Instructions]
    // ------------------------------------------------------------------------

    case InstDB::kEncodingISimdSV: {
      const InstDB::EncodingData::ISimdSV& op_data = InstDB::EncodingData::iSimdSV[encoding_index];

      if (isign4 == ENC_OPS2(Reg, Reg)) {
        // The first destination operand is scalar, which matches element-type of source vectors.
        uint32_t L = (inst_flags & InstDB::kInstFlagLong) != 0;
        if (diff(o0.as<Vec>().reg_type(), RegType::kVec8) != diff(o1.as<Vec>().element_type(), VecElementType::kB) + L)
          goto InvalidInstruction;

        SizeOp size_op = element_type_to_size_op(op_data.vec_op_type, o1.as<Reg>().reg_type(), o1.as<Vec>().element_type());
        if (!size_op.is_valid())
          goto InvalidInstruction;

        opcode.reset(op_data.opcode());
        opcode.add_imm(size_op.q(), 30);
        opcode.add_imm(size_op.size(), 22);
        goto EmitOp_Rd0_Rn5;
      }

      break;
    }

    case InstDB::kEncodingISimdVV: {
      const InstDB::EncodingData::ISimdVV& op_data = InstDB::EncodingData::iSimdVV[encoding_index];

      if (isign4 == ENC_OPS2(Reg, Reg)) {
        const Operand_& sop = significant_simd_op(o0, o1, inst_flags);
        if (!match_signature(o0, o1, inst_flags))
          goto InvalidInstruction;

        SizeOp size_op = element_type_to_size_op(op_data.vec_op_type, sop.as<Reg>().reg_type(), sop.as<Vec>().element_type());
        if (!size_op.is_valid())
          goto InvalidInstruction;

        opcode.reset(op_data.opcode());
        opcode.add_imm(size_op.qs(), 30);
        opcode.add_imm(size_op.scalar(), 28);
        opcode.add_imm(size_op.size(), 22);
        goto EmitOp_Rd0_Rn5;
      }

      break;
    }

    case InstDB::kEncodingISimdVVx: {
      const InstDB::EncodingData::ISimdVVx& op_data = InstDB::EncodingData::iSimdVVx[encoding_index];

      if (isign4 == ENC_OPS2(Reg, Reg)) {
        if (o0.signature() != op_data.op0_signature ||
            o1.signature() != op_data.op1_signature)
          goto InvalidInstruction;

        opcode.reset(op_data.opcode());
        goto EmitOp_Rd0_Rn5;
      }

      break;
    }

    case InstDB::kEncodingISimdVVV: {
      const InstDB::EncodingData::ISimdVVV& op_data = InstDB::EncodingData::iSimdVVV[encoding_index];

      if (isign4 == ENC_OPS3(Reg, Reg, Reg)) {
        const Operand_& sop = significant_simd_op(o0, o1, inst_flags);
        if (!match_signature(o0, o1, o2, inst_flags))
          goto InvalidInstruction;

        SizeOp size_op = element_type_to_size_op(op_data.vec_op_type, sop.as<Reg>().reg_type(), sop.as<Vec>().element_type());
        if (!size_op.is_valid())
          goto InvalidInstruction;

        opcode.reset(op_data.opcode());
        opcode.add_imm(size_op.qs(), 30);
        opcode.add_imm(size_op.scalar(), 28);
        opcode.add_imm(size_op.size(), 22);
        goto EmitOp_Rd0_Rn5_Rm16;
      }

      break;
    }

    case InstDB::kEncodingISimdVVVx: {
      const InstDB::EncodingData::ISimdVVVx& op_data = InstDB::EncodingData::iSimdVVVx[encoding_index];

      if (isign4 == ENC_OPS3(Reg, Reg, Reg)) {
        if (o0.signature() != op_data.op0_signature ||
            o1.signature() != op_data.op1_signature ||
            o2.signature() != op_data.op2_signature)
          goto InvalidInstruction;

        opcode.reset(op_data.opcode());
        goto EmitOp_Rd0_Rn5_Rm16;
      }

      break;
    }

    case InstDB::kEncodingISimdWWV: {
      // Special case for wide add/sub [s|b][add|sub][w]{2}.
      const InstDB::EncodingData::ISimdWWV& op_data = InstDB::EncodingData::iSimdWWV[encoding_index];

      if (isign4 == ENC_OPS3(Reg, Reg, Reg)) {
        SizeOp size_op = element_type_to_size_op(op_data.vec_op_type, o2.as<Reg>().reg_type(), o2.as<Vec>().element_type());
        if (!size_op.is_valid())
          goto InvalidInstruction;

        if (!check_signature(o0, o1) || !o0.as<Reg>().is_vec128() || uint32_t(o0.as<Vec>().element_type()) != uint32_t(o2.as<Vec>().element_type()) + 1u)
          goto InvalidInstruction;

        opcode.reset(op_data.opcode());
        opcode.add_imm(size_op.qs(), 30);
        opcode.add_imm(size_op.scalar(), 28);
        opcode.add_imm(size_op.size(), 22);
        goto EmitOp_Rd0_Rn5_Rm16;
      }

      break;
    }

    case InstDB::kEncodingISimdVVVe: {
      const InstDB::EncodingData::ISimdVVVe& op_data = InstDB::EncodingData::iSimdVVVe[encoding_index];

      if (isign4 == ENC_OPS3(Reg, Reg, Reg)) {
        const Operand_& sop = significant_simd_op(o0, o1, inst_flags);
        if (!match_signature(o0, o1, inst_flags))
          goto InvalidInstruction;

        if (!o2.as<Vec>().has_element_index()) {
          SizeOp size_op = element_type_to_size_op(op_data.regular_vec_type, sop.as<Reg>().reg_type(), sop.as<Vec>().element_type());
          if (!size_op.is_valid())
            goto InvalidInstruction;

          if (!check_signature(o1, o2))
            goto InvalidInstruction;

          opcode.reset(uint32_t(op_data.regular_op) << 10);
          opcode.add_imm(size_op.qs(), 30);
          opcode.add_imm(size_op.scalar(), 28);
          opcode.add_imm(size_op.size(), 22);
          goto EmitOp_Rd0_Rn5_Rm16;
        }
        else {
          SizeOp size_op = element_type_to_size_op(op_data.element_vec_type, sop.as<Reg>().reg_type(), sop.as<Vec>().element_type());
          if (!size_op.is_valid())
            goto InvalidInstruction;

          uint32_t element_index = o2.as<Vec>().element_index();
          LMHImm lmh;

          if (!encode_lmh(size_op.size(), element_index, Out(lmh)))
            goto InvalidElementIndex;

          if (o2.as<Reg>().id() > lmh.max_rm_id)
            goto InvalidPhysId;

          opcode.reset(uint32_t(op_data.element_op) << 10);
          opcode.add_imm(size_op.q(), 30);
          opcode.add_imm(size_op.size(), 22);
          opcode.add_imm(lmh.lm, 20);
          opcode.add_imm(lmh.h, 11);
          goto EmitOp_Rd0_Rn5_Rm16;
        }
      }

      break;
    }

    case InstDB::kEncodingISimdVVVI: {
      const InstDB::EncodingData::ISimdVVVI& op_data = InstDB::EncodingData::iSimdVVVI[encoding_index];

      if (isign4 == ENC_OPS4(Reg, Reg, Reg, Imm)) {
        const Operand_& sop = significant_simd_op(o0, o1, inst_flags);
        if (!match_signature(o0, o1, o2, inst_flags))
          goto InvalidInstruction;

        SizeOp size_op = element_type_to_size_op(op_data.vec_op_type, sop.as<Reg>().reg_type(), sop.as<Vec>().element_type());
        if (!size_op.is_valid())
          goto InvalidInstruction;

        uint64_t imm_value = o3.as<Imm>().value_as<uint64_t>();
        uint32_t imm_size = op_data.imm_size;

        if (op_data.imm64_has_one_bit_less && !size_op.q())
          imm_size--;

        uint32_t immMax = 1u << imm_size;
        if (imm_value >= immMax)
          goto InvalidImmediate;

        opcode.reset(op_data.opcode());
        opcode.add_imm(size_op.qs(), 30);
        opcode.add_imm(size_op.scalar(), 28);
        opcode.add_imm(size_op.size(), 22);
        opcode.add_imm(imm_value, op_data.imm_shift);
        goto EmitOp_Rd0_Rn5_Rm16;
      }

      break;
    }

    case InstDB::kEncodingISimdVVVV: {
      const InstDB::EncodingData::ISimdVVVV& op_data = InstDB::EncodingData::iSimdVVVV[encoding_index];

      if (isign4 == ENC_OPS4(Reg, Reg, Reg, Reg)) {
        const Operand_& sop = significant_simd_op(o0, o1, inst_flags);
        if (!match_signature(o0, o1, o2, o3, inst_flags))
          goto InvalidInstruction;

        SizeOp size_op = element_type_to_size_op(op_data.vec_op_type, sop.as<Reg>().reg_type(), sop.as<Vec>().element_type());
        if (!size_op.is_valid())
          goto InvalidInstruction;

        opcode.reset(uint32_t(op_data.opcode) << 10);
        opcode.add_imm(size_op.qs(), 30);
        opcode.add_imm(size_op.scalar(), 28);
        opcode.add_imm(size_op.size(), 22);
        goto EmitOp_Rd0_Rn5_Rm16_Ra10;
      }

      break;
    }

    case InstDB::kEncodingISimdVVVVx: {
      const InstDB::EncodingData::ISimdVVVVx& op_data = InstDB::EncodingData::iSimdVVVVx[encoding_index];

      if (isign4 == ENC_OPS4(Reg, Reg, Reg, Reg)) {
        if (o0.signature() != op_data.op0_signature ||
            o1.signature() != op_data.op1_signature ||
            o2.signature() != op_data.op2_signature ||
            o3.signature() != op_data.op3_signature)
          goto InvalidInstruction;

        opcode.reset(uint32_t(op_data.opcode) << 10);
        goto EmitOp_Rd0_Rn5_Rm16_Ra10;
      }

      break;
    }


    case InstDB::kEncodingISimdPair: {
      const InstDB::EncodingData::ISimdPair& op_data = InstDB::EncodingData::iSimdPair[encoding_index];

      if (isign4 == ENC_OPS2(Reg, Reg) && op_data.opcode2) {
        if (o0.as<Vec>().is_vec_d1() && o1.as<Vec>().is_vec_d2()) {
          opcode.reset(uint32_t(op_data.opcode2) << 10);
          opcode.add_imm(0x3, 22); // size.
          goto EmitOp_Rd0_Rn5;
        }
      }

      if (isign4 == ENC_OPS3(Reg, Reg, Reg)) {
        if (!match_signature(o0, o1, o2, inst_flags))
          goto InvalidInstruction;

        SizeOp size_op = element_type_to_size_op(op_data.op_type3, o0.as<Reg>().reg_type(), o0.as<Vec>().element_type());
        if (!size_op.is_valid())
          goto InvalidInstruction;

        opcode.reset(uint32_t(op_data.opcode3) << 10);
        opcode.add_imm(size_op.qs(), 30);
        opcode.add_imm(size_op.scalar(), 28);
        opcode.add_imm(size_op.size(), 22);
        goto EmitOp_Rd0_Rn5_Rm16;
      }

      break;
    }

    case InstDB::kEncodingSimdBicOrr: {
      const InstDB::EncodingData::SimdBicOrr& op_data = InstDB::EncodingData::simdBicOrr[encoding_index];

      if (isign4 == ENC_OPS3(Reg, Reg, Reg)) {
        if (!match_signature(o0, o1, o2, inst_flags))
          goto InvalidInstruction;

        SizeOp size_op = element_type_to_size_op(InstDB::kVO_V_B, o0.as<Reg>().reg_type(), o0.as<Vec>().element_type());
        if (!size_op.is_valid())
          goto InvalidInstruction;

        opcode.reset(uint32_t(op_data.register_op) << 10);
        opcode.add_imm(size_op.q(), 30);
        goto EmitOp_Rd0_Rn5_Rm16;
      }

      if (isign4 == ENC_OPS2(Reg, Imm) || isign4 == ENC_OPS3(Reg, Imm, Imm)) {
        SizeOp size_op = element_type_to_size_op(InstDB::kVO_V_HS, o0.as<Reg>().reg_type(), o0.as<Vec>().element_type());
        if (!size_op.is_valid())
          goto InvalidInstruction;

        if (o1.as<Imm>().value_as<uint64_t>() > 0xFFFFFFFFu)
          goto InvalidImmediate;

        uint32_t imm = o1.as<Imm>().value_as<uint32_t>();
        uint32_t shift = 0;
        uint32_t max_shift = (8u << size_op.size()) - 8u;

        if (o2.is_imm()) {
          if (o2.as<Imm>().predicate() != uint32_t(ShiftOp::kLSL))
            goto InvalidImmediate;

          if (imm > 0xFFu || o2.as<Imm>().value_as<uint64_t>() > max_shift)
            goto InvalidImmediate;

          shift = o2.as<Imm>().value_as<uint32_t>();
          if ((shift & 0x7u) != 0u)
            goto InvalidImmediate;
        }
        else if (imm) {
          shift = Support::ctz(imm) & ~0x7u;
          imm >>= shift;

          if (imm > 0xFFu || shift > max_shift)
            goto InvalidImmediate;
        }

        uint32_t cmode = 0x1u | ((shift / 8u) << 1);
        if (size_op.size() == 1)
          cmode |= B(3);

        // The immediate value is split into ABC and DEFGH parts.
        uint32_t abc = (imm >> 5) & 0x7u;
        uint32_t defgh = imm & 0x1Fu;

        opcode.reset(uint32_t(op_data.immediate_op) << 10);
        opcode.add_imm(size_op.q(), 30);
        opcode.add_imm(abc, 16);
        opcode.add_imm(cmode, 12);
        opcode.add_imm(defgh, 5);
        goto EmitOp_Rd0;
      }

      break;
    }

    case InstDB::kEncodingSimdCmp: {
      const InstDB::EncodingData::SimdCmp& op_data = InstDB::EncodingData::simdCmp[encoding_index];

      if (isign4 == ENC_OPS3(Reg, Reg, Reg) && op_data.register_op) {
        if (!match_signature(o0, o1, o2, inst_flags))
          goto InvalidInstruction;

        SizeOp size_op = element_type_to_size_op(op_data.vec_op_type, o0.as<Reg>().reg_type(), o0.as<Vec>().element_type());
        if (!size_op.is_valid())
          goto InvalidInstruction;

        opcode.reset(uint32_t(op_data.register_op) << 10);
        opcode.add_imm(size_op.qs(), 30);
        opcode.add_imm(size_op.scalar(), 28);
        opcode.add_imm(size_op.size(), 22);
        goto EmitOp_Rd0_Rn5_Rm16;
      }

      if (isign4 == ENC_OPS3(Reg, Reg, Imm) && op_data.zero_op) {
        if (!match_signature(o0, o1, inst_flags))
          goto InvalidInstruction;

        if (o2.as<Imm>().value() != 0)
          goto InvalidImmediate;

        SizeOp size_op = element_type_to_size_op(op_data.vec_op_type, o0.as<Reg>().reg_type(), o0.as<Vec>().element_type());
        if (!size_op.is_valid())
          goto InvalidInstruction;

        opcode.reset(uint32_t(op_data.zero_op) << 10);
        opcode.add_imm(size_op.qs(), 30);
        opcode.add_imm(size_op.scalar(), 28);
        opcode.add_imm(size_op.size(), 22);
        goto EmitOp_Rd0_Rn5;
      }

      break;
    }

    case InstDB::kEncodingSimdDot: {
      const InstDB::EncodingData::SimdDot& op_data = InstDB::EncodingData::simdDot[encoding_index];

      if (isign4 == ENC_OPS3(Reg, Reg, Reg)) {
        uint32_t q = diff(o0.as<Reg>().reg_type(), RegType::kVec64);
        uint32_t size = 2;

        if (q > 1u)
          goto InvalidInstruction;

        if (!o2.as<Vec>().has_element_index()) {
          if (!op_data.vector_op)
            goto InvalidInstruction;

          if (o0.as<Reg>().reg_type() != o1.as<Reg>().reg_type() || o1.as<Reg>().reg_type() != o2.as<Reg>().reg_type())
            goto InvalidInstruction;

          if (uint32_t(o0.as<Vec>().element_type()) != op_data.ta ||
              uint32_t(o1.as<Vec>().element_type()) != op_data.tb ||
              uint32_t(o2.as<Vec>().element_type()) != op_data.tb)
            goto InvalidInstruction;

          opcode.reset(uint32_t(op_data.vector_op) << 10);
          opcode.add_imm(q, 30);
          goto EmitOp_Rd0_Rn5_Rm16;
        }
        else {
          if (!op_data.element_op)
            goto InvalidInstruction;

          if (o0.as<Reg>().reg_type() != o1.as<Reg>().reg_type() || !o2.as<Reg>().is_vec128())
            goto InvalidInstruction;

          if (uint32_t(o0.as<Vec>().element_type()) != op_data.ta ||
              uint32_t(o1.as<Vec>().element_type()) != op_data.tb ||
              uint32_t(o2.as<Vec>().element_type()) != op_data.tElement)
            goto InvalidInstruction;

          uint32_t element_index = o2.as<Vec>().element_index();
          LMHImm lmh;

          if (!encode_lmh(size, element_index, Out(lmh)))
            goto InvalidElementIndex;

          if (o2.as<Reg>().id() > lmh.max_rm_id)
            goto InvalidPhysId;

          opcode.reset(uint32_t(op_data.element_op) << 10);
          opcode.add_imm(q, 30);
          opcode.add_imm(lmh.lm, 20);
          opcode.add_imm(lmh.h, 11);
          goto EmitOp_Rd0_Rn5_Rm16;
        }
      }

      break;
    }

    case InstDB::kEncodingSimdDup: SimdDup: {
      if (isign4 == ENC_OPS2(Reg, Reg)) {
        // Truth table of valid encodings of `Q:1|ElementType:3`
        uint32_t kValidEncodings = B(uint32_t(VecElementType::kB) + 0) |
                                   B(uint32_t(VecElementType::kH) + 0) |
                                   B(uint32_t(VecElementType::kS) + 0) |
                                   B(uint32_t(VecElementType::kB) + 8) |
                                   B(uint32_t(VecElementType::kH) + 8) |
                                   B(uint32_t(VecElementType::kS) + 8) |
                                   B(uint32_t(VecElementType::kD) + 8) ;

        uint32_t q = diff(o0.as<Reg>().reg_type(), RegType::kVec64);

        if (o1.as<Reg>().is_gp()) {
          // DUP - Vec (scalar|vector) <- GP register.
          //
          // NOTE: This is only scalar for `dup d, x` case, otherwise the value
          // would be duplicated across all vector elements (1, 2, 4, 8, or 16).
          uint32_t element_type = uint32_t(o0.as<Vec>().element_type());
          if (q > 1 || !Support::bit_test(kValidEncodings, (q << 3) | element_type))
            goto InvalidInstruction;

          uint32_t lsb_index = element_type - 1u;
          uint32_t imm5 = 1u << lsb_index;

          opcode.reset(0b0000111000000000000011 << 10);
          opcode.add_imm(q, 30);
          opcode.add_imm(imm5, 16);
          goto EmitOp_Rd0_Rn5;
        }

        if (!o1.as<Reg>().is_vec() || !o1.as<Vec>().has_element_index())
          goto InvalidInstruction;

        uint32_t dst_index = o1.as<Vec>().element_index();
        if (!o0.as<Vec>().has_element_type()) {
          // DUP - Vec (scalar) <- Vec[N].
          uint32_t lsb_index = diff(o0.as<Reg>().reg_type(), RegType::kVec8);

          if (lsb_index != diff(o1.as<Vec>().element_type(), VecElementType::kB) || lsb_index > 3)
            goto InvalidInstruction;

          uint32_t imm5 = ((dst_index << 1) | 1u) << lsb_index;
          if (imm5 > 31)
            goto InvalidElementIndex;

          opcode.reset(0b0101111000000000000001 << 10);
          opcode.add_imm(imm5, 16);
          goto EmitOp_Rd0_Rn5;
        }
        else {
          // DUP - Vec (all) <- Vec[N].
          uint32_t element_type = uint32_t(o0.as<Vec>().element_type());
          if (q > 1 || !Support::bit_test(kValidEncodings, (q << 3) | element_type))
            goto InvalidInstruction;

          uint32_t lsb_index = element_type - 1u;
          uint32_t imm5 = ((dst_index << 1) | 1u) << lsb_index;

          if (imm5 > 31)
            goto InvalidElementIndex;

          opcode.reset(0b0000111000000000000001 << 10);
          opcode.add_imm(q, 30);
          opcode.add_imm(imm5, 16);
          goto EmitOp_Rd0_Rn5;
        }
      }

      break;
    }

    case InstDB::kEncodingSimdIns: SimdIns: {
      if (isign4 == ENC_OPS2(Reg, Reg) && o0.as<Reg>().is_vec128()) {
        if (!o0.as<Vec>().has_element_index())
          goto InvalidInstruction;

        uint32_t element_type = uint32_t(o0.as<Vec>().element_type());
        uint32_t dst_index = o0.as<Vec>().element_index();
        uint32_t lsb_index = element_type - 1u;

        uint32_t imm5 = ((dst_index << 1) | 1u) << lsb_index;
        if (imm5 > 31)
          goto InvalidElementIndex;

        if (o1.as<Reg>().is_gp()) {
          // INS - Vec[N] <- GP register.
          opcode.reset(0b0100111000000000000111 << 10);
          opcode.add_imm(imm5, 16);
          goto EmitOp_Rd0_Rn5;
        }
        else if (o1.as<Reg>().is_vec128() && o1.as<Vec>().has_element_index()) {
          // INS - Vec[N] <- Vec[M].
          if (o0.as<Vec>().element_type() != o1.as<Vec>().element_type())
            goto InvalidInstruction;

          uint32_t src_index = o1.as<Vec>().element_index();
          if (o0.as<Reg>().reg_type() != o1.as<Reg>().reg_type())
            goto InvalidInstruction;

          uint32_t imm4 = src_index << lsb_index;
          if (imm4 > 15)
            goto InvalidElementIndex;

          opcode.reset(0b0110111000000000000001 << 10);
          opcode.add_imm(imm5, 16);
          opcode.add_imm(imm4, 11);
          goto EmitOp_Rd0_Rn5;
        }
      }

      break;
    }

    case InstDB::kEncodingSimdMov: {
      if (isign4 == ENC_OPS2(Reg, Reg)) {
        if (o0.as<Reg>().is_vec() && o1.as<Reg>().is_vec()) {
          // INS v.x[index], v.x[index].
          if (o0.as<Vec>().has_element_index() && o1.as<Vec>().has_element_index())
            goto SimdIns;

          // DUP {b|h|s|d}, v.{b|h|s|d}[index].
          if (o1.as<Vec>().has_element_index())
            goto SimdDup;

          if (!check_signature(o0, o1))
            goto InvalidInstruction;

          // ORR Vd, Vn, Vm
          uint32_t q = diff(o0.as<Reg>().reg_type(), RegType::kVec64);
          if (q > 1)
            goto InvalidInstruction;

          opcode.reset(0b0000111010100000000111 << 10);
          opcode.add_imm(q, 30);
          opcode.add_reg(o1, 16); // Vn == Vm.
          goto EmitOp_Rd0_Rn5;
        }

        if (o0.as<Reg>().is_vec() && o1.as<Reg>().is_gp()) {
          // INS v.x[index], Rn.
          if (o0.as<Vec>().has_element_index())
            goto SimdIns;

          goto InvalidInstruction;
        }

        if (o0.as<Reg>().is_gp() && o1.as<Reg>().is_vec()) {
          // UMOV Rd, V.{s|d}[index].
          encoding_index = 1;
          goto SimdUmov;
        }
      }

      break;
    }

    case InstDB::kEncodingSimdMoviMvni: {
      const InstDB::EncodingData::SimdMoviMvni& op_data = InstDB::EncodingData::simdMoviMvni[encoding_index];

      if (isign4 == ENC_OPS2(Reg, Imm) || isign4 == ENC_OPS3(Reg, Imm, Imm)) {
        SizeOp size_op = element_type_to_size_op(InstDB::kVO_V_Any, o0.as<Reg>().reg_type(), o0.as<Vec>().element_type());
        if (!size_op.is_valid())
          goto InvalidInstruction;

        uint64_t imm64 = o1.as<Imm>().value_as<uint64_t>();
        uint32_t imm8 = 0;
        uint32_t cmode = 0;
        uint32_t inverted = op_data.inverted;
        uint32_t op = 0;
        uint32_t shift = 0;
        uint32_t shift_op = uint32_t(ShiftOp::kLSL);

        if (size_op.size() == 3u) {
          // The second immediate should not be present, however, we accept
          // an immediate value of zero as some user code may still pass it.
          if (o2.is_imm() && o0.as<Imm>().value() != 0)
            goto InvalidImmediate;

          if (Utils::is_byte_mask_imm(imm64)) {
            imm8 = Utils::encode_imm64_byte_mask_to_imm8(imm64);
          }
          else {
            // Change from D to S and from 64-bit imm to 32-bit imm if this
            // is not a byte-mask pattern.
            if ((imm64 >> 32) == (imm64 & 0xFFFFFFFFu)) {
              imm64 &= 0xFFFFFFFFu;
              size_op.decrement_size();
            }
            else {
              goto InvalidImmediate;
            }
          }
        }

        if (size_op.size() < 3u) {
          if (imm64 > 0xFFFFFFFFu)
            goto InvalidImmediate;
          imm8 = uint32_t(imm64);

          if (size_op.size() == 2) {
            if ((imm8 >> 16) == (imm8 & 0xFFFFu)) {
              imm8 >>= 16;
              size_op.decrement_size();
            }
          }

          if (size_op.size() == 1) {
            if (imm8 > 0xFFFFu)
              goto InvalidImmediate;

            if ((imm8 >> 8) == (imm8 & 0xFFu)) {
              imm8 >>= 8;
              size_op.decrement_size();
            }
          }

          uint32_t max_shift = (8u << size_op.size()) - 8u;
          if (o2.is_imm()) {
            if (imm8 > 0xFFu || o2.as<Imm>().value_as<uint64_t>() > max_shift)
              goto InvalidImmediate;

            shift = o2.as<Imm>().value_as<uint32_t>();
            shift_op = o2.as<Imm>().predicate();
          }
          else if (imm8) {
            shift = Support::ctz(imm8) & ~0x7u;
            imm8 >>= shift;

            if (imm8 > 0xFFu || shift > max_shift)
              goto InvalidImmediate;
          }

          if ((shift & 0x7u) != 0u)
            goto InvalidImmediate;
        }

        shift /= 8u;

        switch (size_op.size()) {
          case 0:
            if (shift_op != uint32_t(ShiftOp::kLSL))
              goto InvalidImmediate;

            if (inverted) {
              imm8 = ~imm8 & 0xFFu;
            }

            cmode = B(3) | B(2) | B(1);
            break;

          case 1:
            if (shift_op != uint32_t(ShiftOp::kLSL))
              goto InvalidImmediate;

            cmode = B(3) | (shift << 1);
            op = inverted;
            break;

          case 2:
            if (shift_op == uint32_t(ShiftOp::kLSL)) {
              cmode = shift << 1;
            }
            else if (shift_op == uint32_t(ShiftOp::kMSL)) {
              if (shift == 0 || shift > 2)
                goto InvalidImmediate;
              cmode = B(3) | B(2) | (shift - 1u);
            }
            else {
              goto InvalidImmediate;
            }

            op = inverted;
            break;

          case 3:
            if (inverted) {
              imm8 = ~imm8 & 0xFFu;
            }

            op = 1;
            cmode = B(3) | B(2) | B(1);
            break;
        }

        // The immediate value is split into ABC and DEFGH parts.
        uint32_t abc = (imm8 >> 5) & 0x7u;
        uint32_t defgh = imm8 & 0x1Fu;

        opcode.reset(uint32_t(op_data.opcode) << 10);
        opcode.add_imm(size_op.q(), 30);
        opcode.add_imm(op, 29);
        opcode.add_imm(abc, 16);
        opcode.add_imm(cmode, 12);
        opcode.add_imm(defgh, 5);
        goto EmitOp_Rd0;
      }

      break;
    }

    case InstDB::kEncodingSimdShift: {
      const InstDB::EncodingData::SimdShift& op_data = InstDB::EncodingData::simdShift[encoding_index];

      const Operand_& sop = significant_simd_op(o0, o1, inst_flags);
      SizeOp size_op = element_type_to_size_op(op_data.vec_op_type, sop.as<Reg>().reg_type(), sop.as<Vec>().element_type());

      if (!size_op.is_valid())
        goto InvalidInstruction;

      if (isign4 == ENC_OPS3(Reg, Reg, Imm) && op_data.immediate_op) {
        if (!match_signature(o0, o1, inst_flags))
          goto InvalidInstruction;

        if (o2.as<Imm>().value_as<uint64_t>() > 63)
          goto InvalidImmediate;

        uint32_t lsb_shift = size_op.size() + 3u;
        uint32_t lsb_mask = (1u << lsb_shift) - 1u;
        uint32_t imm = o2.as<Imm>().value_as<uint32_t>();

        // Some instructions use IMM and some X - IMM, so negate if required.
        if (op_data.inverted_imm) {
          if (imm == 0 || imm > (1u << lsb_shift))
            goto InvalidImmediate;
          imm = Support::neg(imm) & lsb_mask;
        }

        if (imm > lsb_mask)
          goto InvalidImmediate;
        imm |= (1u << lsb_shift);

        opcode.reset(uint32_t(op_data.immediate_op) << 10);
        opcode.add_imm(size_op.qs(), 30);
        opcode.add_imm(size_op.scalar(), 28);
        opcode.add_imm(imm, 16);
        goto EmitOp_Rd0_Rn5;
      }

      if (isign4 == ENC_OPS3(Reg, Reg, Reg) && op_data.register_op) {
        if (!match_signature(o0, o1, o2, inst_flags))
          goto InvalidInstruction;

        opcode.reset(uint32_t(op_data.register_op) << 10);
        opcode.add_imm(size_op.qs(), 30);
        opcode.add_imm(size_op.scalar(), 28);
        opcode.add_imm(size_op.size(), 22);
        goto EmitOp_Rd0_Rn5_Rm16;
      }

      break;
    }

    case InstDB::kEncodingSimdShiftES: {
      const InstDB::EncodingData::SimdShiftES& op_data = InstDB::EncodingData::simdShiftES[encoding_index];

      if (isign4 == ENC_OPS3(Reg, Reg, Imm)) {
        SizeOp size_op = element_type_to_size_op(op_data.vec_op_type, o1.as<Reg>().reg_type(), o1.as<Vec>().element_type());
        if (!size_op.is_valid())
          goto InvalidInstruction;

        if (!match_signature(o0, o1, inst_flags))
          goto InvalidInstruction;

        // The immediate value must match the element size.
        uint64_t shift = o2.as<Imm>().value_as<uint64_t>();
        uint32_t shift_op = o2.as<Imm>().predicate();

        if (shift != (8u << size_op.size()) || shift_op != uint32_t(ShiftOp::kLSL))
          goto InvalidImmediate;

        opcode.reset(uint32_t(op_data.opcode) << 10);
        opcode.add_imm(size_op.q(), 30);
        opcode.add_imm(size_op.size(), 22);
        goto EmitOp_Rd0_Rn5;
      }

      break;
    }

    case InstDB::kEncodingSimdSm3tt: {
      const InstDB::EncodingData::SimdSm3tt& op_data = InstDB::EncodingData::simdSm3tt[encoding_index];

      if (isign4 == ENC_OPS3(Reg, Reg, Reg)) {
        if (o0.as<Vec>().is_vec_s4() && o1.as<Vec>().is_vec_s4() && o2.as<Vec>().is_vec_s4() && o2.as<Vec>().has_element_index()) {
          uint32_t imm2 = o2.as<Vec>().element_index();
          if (imm2 > 3)
            goto InvalidElementIndex;

          opcode.reset(uint32_t(op_data.opcode) << 10);
          opcode.add_imm(imm2, 12);
          goto EmitOp_Rd0_Rn5_Rm16;
        }
      }

      break;
    }


    case InstDB::kEncodingSimdSmovUmov: SimdUmov: {
      const InstDB::EncodingData::SimdSmovUmov& op_data = InstDB::EncodingData::simdSmovUmov[encoding_index];

      if (isign4 == ENC_OPS2(Reg, Reg) && o0.as<Reg>().is_gp() && o1.as<Reg>().is_vec()) {
        SizeOp size_op = element_type_to_size_op(op_data.vec_op_type, o1.as<Reg>().reg_type(), o1.as<Vec>().element_type());
        if (!size_op.is_valid())
          goto InvalidInstruction;

        if (!o1.as<Vec>().has_element_index())
          goto InvalidInstruction;

        uint32_t x = o0.as<Gp>().is_gp64();
        uint32_t gp_must_be_x = uint32_t(size_op.size() >= 3u - op_data.is_signed);

        if (op_data.is_signed) {
          if (gp_must_be_x && !x)
            goto InvalidInstruction;
        }
        else {
          if (x != gp_must_be_x)
            goto InvalidInstruction;
        }

        uint32_t element_index = o1.as<Vec>().element_index();
        uint32_t max_element_index = 15u >> size_op.size();

        if (element_index > max_element_index)
          goto InvalidElementIndex;

        uint32_t imm5 = (1u | (element_index << 1)) << size_op.size();

        opcode.reset(uint32_t(op_data.opcode) << 10);
        opcode.add_imm(x, 30);
        opcode.add_imm(imm5, 16);
        goto EmitOp_Rd0_Rn5;
      }

      break;
    }

    case InstDB::kEncodingSimdSxtlUxtl: {
      const InstDB::EncodingData::SimdSxtlUxtl& op_data = InstDB::EncodingData::simdSxtlUxtl[encoding_index];

      if (isign4 == ENC_OPS2(Reg, Reg)) {
        SizeOp size_op = element_type_to_size_op(op_data.vec_op_type, o1.as<Reg>().reg_type(), o1.as<Vec>().element_type());
        if (!size_op.is_valid())
          goto InvalidInstruction;

        if (!match_signature(o0, o1, inst_flags))
          goto InvalidInstruction;

        opcode.reset(uint32_t(op_data.opcode) << 10);
        opcode.add_imm(size_op.q(), 30);
        opcode.add_imm(1u, size_op.size() + 19);
        goto EmitOp_Rd0_Rn5;
      }

      break;
    }

    case InstDB::kEncodingSimdTblTbx: {
      const InstDB::EncodingData::SimdTblTbx& op_data = InstDB::EncodingData::simdTblTbx[encoding_index];

      if (isign4 == ENC_OPS3(Reg, Reg, Reg) || isign4 == ENC_OPS4(Reg, Reg, Reg, Reg)) {
        // TBL/TBX <Vd>.<Ta>, { <Vn>.16B }, <Vm>.<Ta>
        // TBL/TBX <Vd>.<Ta>, { <Vn>.16B, <Vn+1>.16B }, <Vm>.<Ta>
        // TBL/TBX <Vd>.<Ta>, { <Vn>.16B, <Vn+1>.16B, <Vn+2>.16B }, <Vm>.<Ta>
        // TBL/TBX <Vd>.<Ta>, { <Vn>.16B, <Vn+1>.16B, <Vn+2>.16B, <Vn+3>.16B }, <Vm>.<Ta>
        opcode.reset(uint32_t(op_data.opcode) << 10);

        const Operand_& o4 = op_ext[EmitterUtils::kOp4];
        const Operand_& o5 = op_ext[EmitterUtils::kOp5];

        uint32_t q = diff(o0.as<Reg>().reg_type(), RegType::kVec64);
        if (q > 1 || o0.as<Vec>().has_element_index())
          goto InvalidInstruction;

        if (!o1.as<Vec>().is_vec_b16() || o1.as<Vec>().has_element_index())
          goto InvalidInstruction;

        uint32_t len = uint32_t(!o3.is_none()) + uint32_t(!o4.is_none()) + uint32_t(!o5.is_none());
        opcode.add_imm(q, 30);
        opcode.add_imm(len, 13);

        switch (len) {
          case 0:
            if (!check_signature(o0, o2))
              goto InvalidInstruction;

            if (o2.id() > 31)
              goto InvalidPhysId;

            opcode.add_reg(o2, 16);
            goto EmitOp_Rd0_Rn5;

          case 1:
            if (!check_signature(o0, o3))
              goto InvalidInstruction;

            if (o3.id() > 31)
              goto InvalidPhysId;

            opcode.add_reg(o3, 16);
            goto EmitOp_Rd0_Rn5;

          case 2:
            if (!check_signature(o0, o4))
              goto InvalidInstruction;

            if (o4.id() > 31)
              goto InvalidPhysId;

            opcode.add_reg(o4, 16);
            goto EmitOp_Rd0_Rn5;

          case 3:
            if (!check_signature(o0, o5))
              goto InvalidInstruction;

            if (o5.id() > 31)
              goto InvalidPhysId;

            opcode.add_reg(o5, 16);
            goto EmitOp_Rd0_Rn5;

          default:
            // Should never happen.
            goto InvalidInstruction;
        }
      }

      break;
    }

    // ------------------------------------------------------------------------
    // [Simd - Load / Store]
    // ------------------------------------------------------------------------

    case InstDB::kEncodingSimdLdSt: {
      const InstDB::EncodingData::SimdLdSt& op_data = InstDB::EncodingData::simdLdSt[encoding_index];

      if (isign4 == ENC_OPS2(Reg, Mem)) {
        const Mem& m = o1.as<Mem>();
        rm_rel = &m;

        // Width  |       SZ |        XY | XSZ
        // -------+----------+-----------+-----
        // 8-bit  | size==00 | opc == 01 | 000
        // 16-bit | size==01 | opc == 01 | 001
        // 32-bit | size==10 | opc == 01 | 010
        // 64-bit | size==11 | opc == 01 | 011
        // 128-bit| size==00 | opc == 11 | 100
        uint32_t xsz = diff(o0.as<Reg>().reg_type(), RegType::kVec8);
        if (xsz > 4u || o0.as<Vec>().has_element_index())
          goto InvalidRegType;

        if (!check_vec_id(o0))
          goto InvalidPhysId;

        if (!check_mem_base_index_rel(m))
          goto InvalidAddress;

        int64_t offset = m.offset();
        if (m.has_base_reg()) {
          // [Base {Offset | Index}]
          if (m.has_index()) {
            uint32_t opt = shift_op_to_ld_st_opt_map[size_t(m.shift_op())];
            if (opt == 0xFFu)
              goto InvalidAddress;

            uint32_t shift = m.shift();
            uint32_t s = (shift != 0);

            if (s && shift != xsz)
              goto InvalidAddressScale;

            opcode.reset(uint32_t(op_data.register_op) << 21);
            opcode.add_imm(xsz & 3u, 30);
            opcode.add_imm(xsz >> 2, 23);
            opcode.add_imm(opt, 13);
            opcode.add_imm(s, 12);
            opcode |= B(11);
            opcode.add_reg(o0, 0);
            goto EmitOp_MemBaseIndex_Rn5_Rm16;
          }

          // Makes it easier to work with the offset especially on 32-bit arch.
          if (!Support::is_int_n<32>(offset))
            goto InvalidDisplacement;
          int32_t offset32 = int32_t(offset);

          if (m.is_pre_or_post()) {
            if (!Support::is_int_n<9>(offset32))
              goto InvalidDisplacement;

            opcode.reset(uint32_t(op_data.pre_post_op) << 21);
            opcode.add_imm(xsz & 3u, 30);
            opcode.add_imm(xsz >> 2, 23);
            opcode.add_imm(offset32 & 0x1FF, 12);
            opcode.add_imm(m.is_pre_index(), 11);
            opcode |= B(10);
            opcode.add_reg(o0, 0);
            goto EmitOp_MemBase_Rn5;
          }
          else {
            uint32_t imm12 = uint32_t(offset32) >> xsz;

            // If this instruction is not encodable with scaled unsigned offset, try unscaled signed offset.
            if (!Support::is_uint_n<12>(imm12) || (imm12 << xsz) != uint32_t(offset32)) {
              inst_id = op_data.u_alt_inst_id;
              inst_info = &InstDB::_inst_info_table[inst_id];
              encoding_index = inst_info->_encoding_data_index;
              goto Case_SimdLdurStur;
            }

            opcode.reset(uint32_t(op_data.u_offset_op) << 22);
            opcode.add_imm(xsz & 3u, 30);
            opcode.add_imm(xsz >> 2, 23);
            opcode.add_imm(imm12, 10);
            opcode.add_reg(o0, 0);
            goto EmitOp_MemBase_Rn5;
          }
        }
        else {
          if (!op_data.literal_op)
            goto InvalidAddress;

          if (xsz < 2u)
            goto InvalidRegType;

          uint32_t opc = xsz - 2u;
          opcode.reset(uint32_t(op_data.literal_op) << 24);
          opcode.add_imm(opc, 30);
          opcode.add_reg(o0, 0);
          offset_format.reset_to_imm_value(OffsetType::kSignedOffset, 4, 5, 19, 2);
          goto EmitOp_Rel;
        }
      }

      break;
    }

    case InstDB::kEncodingSimdLdpStp: {
      const InstDB::EncodingData::SimdLdpStp& op_data = InstDB::EncodingData::simdLdpStp[encoding_index];

      if (isign4 == ENC_OPS3(Reg, Reg, Mem)) {
        const Mem& m = o2.as<Mem>();
        rm_rel = &m;

        uint32_t opc = diff(o0.as<Reg>().reg_type(), RegType::kVec32);
        if (opc > 2u || o0.as<Vec>().has_element_type_or_index())
          goto InvalidInstruction;

        if (!check_signature(o0, o1))
          goto InvalidInstruction;

        if (!check_vec_id(o0, o1))
          goto InvalidPhysId;

        if (m.base_type() != RegType::kGp64 || m.has_index())
          goto InvalidAddress;

        if (m.is_offset_64bit())
          goto InvalidDisplacement;

        uint32_t offset_shift = 2u + opc;
        int32_t offset32 = m.offset_lo32() >> offset_shift;

        // Make sure we didn't lose bits by applying the mandatory offset shift.
        if (Support::shl(offset32, offset_shift) != m.offset_lo32())
          goto InvalidDisplacement;

        // Offset is encoded as a 7-bit immediate.
        if (!Support::is_int_n<7>(offset32))
          goto InvalidDisplacement;

        if (m.is_pre_or_post() && offset32 != 0) {
          if (!op_data.pre_post_op)
            goto InvalidAddress;

          opcode.reset(uint32_t(op_data.pre_post_op) << 22);
          opcode.add_imm(m.is_pre_index(), 24);
        }
        else {
          opcode.reset(uint32_t(op_data.offset_op) << 22);
        }

        opcode.add_imm(opc, 30);
        opcode.add_imm(offset32 & 0x7F, 15);
        opcode.add_reg(o1, 10);
        opcode.add_reg(o0, 0);
        goto EmitOp_MemBase_Rn5;
      }

      break;
    }

    case InstDB::kEncodingSimdLdurStur: {
Case_SimdLdurStur:
      const InstDB::EncodingData::SimdLdurStur& op_data = InstDB::EncodingData::simdLdurStur[encoding_index];

      if (isign4 == ENC_OPS2(Reg, Mem)) {
        const Mem& m = o1.as<Mem>();
        rm_rel = &m;

        uint32_t sz = diff(o0.as<Reg>().reg_type(), RegType::kVec8);
        if (sz > 4 || o0.as<Vec>().has_element_type_or_index())
          goto InvalidInstruction;

        if (!check_vec_id(o0))
          goto InvalidPhysId;

        if (!check_mem_base_index_rel(m))
          goto InvalidAddress;

        if (m.has_base_reg() && !m.has_index() && !m.is_pre_or_post()) {
          if (m.is_offset_64bit())
            goto InvalidDisplacement;

          int32_t offset32 = m.offset_lo32();
          if (!Support::is_int_n<9>(offset32))
            goto InvalidDisplacement;

          opcode.reset(uint32_t(op_data.opcode) << 10);
          opcode.add_imm(sz & 3u, 30);
          opcode.add_imm(sz >> 2, 23);
          opcode.add_imm(offset32 & 0x1FF, 12);
          opcode.add_reg(o0, 0);
          goto EmitOp_MemBase_Rn5;
        }

        goto InvalidAddress;
      }

      break;
    }

    case InstDB::kEncodingSimdLdNStN: {
      const InstDB::EncodingData::SimdLdNStN& op_data = InstDB::EncodingData::simdLdNStN[encoding_index];
      const Operand_& o4 = op_ext[EmitterUtils::kOp4];

      uint32_t n = 1;

      if (isign4 == ENC_OPS2(Reg, Mem)) {
        if (op_data.n != 1)
          goto InvalidInstruction;

        rm_rel = &o1;
      }
      else if (isign4 == ENC_OPS3(Reg, Reg, Mem)) {
        if (op_data.n != 1 && op_data.n != 2)
          goto InvalidInstruction;

        if (!check_signature(o0, o1) || !check_consecutive(o0, o1))
          goto InvalidInstruction;

        n = 2;
        rm_rel = &o2;
      }
      else if (isign4 == ENC_OPS4(Reg, Reg, Reg, Mem) && o4.is_none()) {
        if (op_data.n != 1 && op_data.n != 3)
          goto InvalidInstruction;

        if (!check_signature(o0, o1, o2) || !check_consecutive(o0, o1, o2))
          goto InvalidInstruction;

        n = 3;
        rm_rel = &o3;
      }
      else if (isign4 == ENC_OPS4(Reg, Reg, Reg, Reg) && o4.is_mem()) {
        if (op_data.n != 1 && op_data.n != 4)
          goto InvalidInstruction;

        if (!check_signature(o0, o1, o2, o3) || !check_consecutive(o0, o1, o2, o3))
          goto InvalidInstruction;

        n = 4;
        rm_rel = &o4;
      }
      else {
        goto InvalidInstruction;
      }

      // We will use `v` and `m` from now as those are relevant for encoding.
      const Vec& v = o0.as<Vec>();
      const Mem& m = rm_rel->as<Mem>();

      uint32_t q = 0;
      uint32_t rm = 0;
      uint32_t rn = m.base_id();
      uint32_t sz = diff(v.element_type(), VecElementType::kB);
      uint32_t opc_s_size = sz;
      uint32_t offset_possibility = 0;

      if (sz > 3)
        goto InvalidInstruction;

      if (m.base_type() != RegType::kGp64)
        goto InvalidAddress;

      // Rn cannot be ZR, but can be SP.
      if (rn > 30 && rn != Gp::kIdSp)
        goto InvalidAddress;

      rn &= 31;

      if (op_data.replicate) {
        if (n != op_data.n)
          goto InvalidInstruction;

        // Replicates to the whole register, element index cannot be used.
        if (v.has_element_index())
          goto InvalidInstruction;

        q = diff(v.reg_type(), RegType::kVec64);
        if (q > 1)
          goto InvalidInstruction;

        opcode.reset(uint32_t(op_data.single_op) << 10);
        offset_possibility = (1u << sz) * n;
      }
      else if (v.has_element_index()) {
        if (n != op_data.n)
          goto InvalidInstruction;

        // LDx/STx (single structure).
        static const uint8_t opc_s_size_by_sz_table[] = { 0x0u << 3, 0x2u << 3, 0x4u << 3, (0x4u << 3) | 1u };

        opcode.reset(uint32_t(op_data.single_op) << 10);
        opc_s_size = opc_s_size_by_sz_table[sz];
        offset_possibility =  (1u << sz) * op_data.n;

        uint32_t element_index = v.element_index();
        uint32_t max_element_index = 15 >> sz;

        if (element_index > max_element_index)
          goto InvalidElementIndex;

        element_index <<= sz;
        q = element_index >> 3;
        opc_s_size |= element_index & 0x7u;
      }
      else {
        // LDx/STx (multiple structures).
        static const uint8_t opc_s_size_by_n_table[] = { 0u, 0x7u << 2, 0xAu << 2, 0x6u << 2, 0x2u << 2 };

        q = diff(v.reg_type(), RegType::kVec64);
        if (q > 1)
          goto InvalidInstruction;

        if (op_data.n == 1)
          opc_s_size |= opc_s_size_by_n_table[n];

        opcode.reset(uint32_t(op_data.multiple_op) << 10);
        offset_possibility = (8u << q) * n;
      }

      if (m.has_index()) {
        if (m.has_offset() || !m.is_post_index())
          goto InvalidAddress;

        rm = m.index_id();
        if (rm > 30)
          goto InvalidAddress;

        // Bit 23 - PostIndex.
        opcode |= B(23);
      }
      else {
        if (m.has_offset()) {
          if (m.offset() != int32_t(offset_possibility) || !m.is_post_index())
            goto InvalidAddress;
          rm = 31;

          // Bit 23 - PostIndex.
          opcode |= B(23);
        }
      }

      opcode.add_imm(q, 30);
      opcode.add_imm(rm, 16);
      opcode.add_imm(opc_s_size, 10);
      opcode.add_imm(rn, 5);
      goto EmitOp_Rd0;
    }

    default:
      break;
  }

  goto InvalidInstruction;

  // --------------------------------------------------------------------------
  // [EmitGp - Single]
  // --------------------------------------------------------------------------

EmitOp_Rd0:
  if (!check_valid_regs(o0))
    goto InvalidPhysId;

  opcode.add_reg(o0, 0);
  goto EmitOp;

EmitOp_Rn5:
  if (!check_valid_regs(o0))
    goto InvalidPhysId;

  opcode.add_reg(o0, 5);
  goto EmitOp;

EmitOp_Rn5_Rm16:
  if (!check_valid_regs(o0, o1))
    goto InvalidPhysId;

  opcode.add_reg(o0, 5);
  opcode.add_reg(o1, 16);
  goto EmitOp;

EmitOp_Rd0_Rn5:
  if (!check_valid_regs(o0, o1))
    goto InvalidPhysId;

  opcode.add_reg(o0, 0);
  opcode.add_reg(o1, 5);
  goto EmitOp;

EmitOp_Rd0_Rn5_Rm16_Ra10:
  if (!check_valid_regs(o0, o1, o2, o3))
    goto InvalidPhysId;

  opcode.add_reg(o0, 0);
  opcode.add_reg(o1, 5);
  opcode.add_reg(o2, 16);
  opcode.add_reg(o3, 10);
  goto EmitOp;

EmitOp_Rd0_Rn5_Rm16:
  if (!check_valid_regs(o0, o1, o3))
    goto InvalidPhysId;

  opcode.add_reg(o0, 0);
  opcode.add_reg(o1, 5);
  opcode.add_reg(o2, 16);
  goto EmitOp;

  // --------------------------------------------------------------------------
  // [EmitGp - Multiple]
  // --------------------------------------------------------------------------

EmitOp_Multiple:
  {
    ASMJIT_ASSERT(multiple_op_count > 0);
    err = writer.ensure_space(this, multiple_op_count * 4u);
    if (ASMJIT_UNLIKELY(err != Error::kOk)) {
      goto Failed;
    }

    for (uint32_t i = 0; i < multiple_op_count; i++) {
      writer.emit32u_le(multiple_op_data[i]);
    }

    goto EmitDone;
  }

  // --------------------------------------------------------------------------
  // [EmitGp - Memory]
  // --------------------------------------------------------------------------

EmitOp_MemBase_Rn5:
  if (!check_mem_base(rm_rel->as<Mem>())) {
    goto InvalidAddress;
  }

  opcode.add_reg(rm_rel->as<Mem>().base_id(), 5);
  goto EmitOp;

EmitOp_MemBaseNoImm_Rn5:
  if (!check_mem_base(rm_rel->as<Mem>()) || rm_rel->as<Mem>().has_index()) {
    goto InvalidAddress;
  }

  if (rm_rel->as<Mem>().has_offset()) {
    goto InvalidDisplacement;
  }

  opcode.add_reg(rm_rel->as<Mem>().base_id(), 5);
  goto EmitOp;

EmitOp_MemBaseIndex_Rn5_Rm16:
  if (!rm_rel->as<Mem>().has_base_reg()) {
    goto InvalidAddress;
  }

  if (rm_rel->as<Mem>().index_id() > 30 && rm_rel->as<Mem>().index_id() != Gp::kIdZr) {
    goto InvalidPhysId;
  }

  opcode.add_reg(rm_rel->as<Mem>().index_id(), 16);
  opcode.add_reg(rm_rel->as<Mem>().base_id(), 5);
  goto EmitOp;

  // --------------------------------------------------------------------------
  // [EmitOp - PC Relative]
  // --------------------------------------------------------------------------

EmitOp_Rel:
  {
    if (rm_rel->is_label() || rm_rel->is_mem()) {
      uint32_t label_id;
      int64_t label_offset = 0;

      if (rm_rel->is_label()) {
        label_id = rm_rel->as<Label>().id();
      }
      else {
        label_id = rm_rel->as<Mem>().base_id();
        label_offset = rm_rel->as<Mem>().offset();
      }

      if (ASMJIT_UNLIKELY(!_code->is_label_valid(label_id))) {
        goto InvalidLabel;
      }

      LabelEntry& le = _code->label_entry_of(label_id);

      if (offset_format.type() == OffsetType::kAArch64_ADRP) {
        // TODO: [ARM] Always create relocation entry.
      }

      if (le.is_bound_to(_section)) {
        // Label bound to the current section.
        offset_value = le.offset() - uint64_t(offset()) + uint64_t(label_offset);
        goto EmitOp_DispImm;
      }
      else {
        // Create a fixup referencing an non-bound label.
        size_t code_offset = writer.offset_from(_buffer_data);
        Fixup* fixup = _code->new_fixup(le, _section->section_id(), code_offset, intptr_t(label_offset), offset_format);

        if (ASMJIT_UNLIKELY(!fixup)) {
          goto OutOfMemory;
        }

        goto EmitOp;
      }
    }
  }

  if (rm_rel->is_imm()) {
    uint64_t base_address = _code->base_address();
    uint64_t target_offset = rm_rel->as<Imm>().value_as<uint64_t>();

    size_t code_offset = writer.offset_from(_buffer_data);

    if (base_address == Globals::kNoBaseAddress || _section->section_id() != 0) {
      // Create a new RelocEntry as we cannot calculate the offset right now.
      RelocEntry* re;
      err = _code->new_reloc_entry(Out(re), RelocType::kAbsToRel);
      if (err != Error::kOk) {
        goto Failed;
      }

      re->_source_section_id = _section->section_id();
      re->_source_offset = code_offset;
      re->_format = offset_format;
      re->_payload = rm_rel->as<Imm>().value_as<uint64_t>() + 4u;
      goto EmitOp;
    }
    else {
      uint64_t pc = base_address + code_offset;

      if (offset_format.type() == OffsetType::kAArch64_ADRP) {
        pc &= ~uint64_t(4096 - 1);
      }

      offset_value = target_offset - pc;
      goto EmitOp_DispImm;
    }
  }

  goto InvalidInstruction;

EmitOp_DispImm:
  {
    if ((offset_value & Support::lsb_mask<uint32_t>(offset_format.imm_discard_lsb())) != 0) {
      goto InvalidDisplacement;
    }

    int64_t disp_imm64 = int64_t(offset_value) >> offset_format.imm_discard_lsb();
    if (!EmitterUtils::is_encodable_offset_64(disp_imm64, offset_format.imm_bit_count())) {
      goto InvalidDisplacement;
    }

    uint32_t disp_imm32 = uint32_t(disp_imm64 & Support::lsb_mask<uint32_t>(offset_format.imm_bit_count()));
    switch (offset_format.type()) {
      case OffsetType::kSignedOffset: {
        opcode.add_imm(disp_imm32, offset_format.imm_bit_shift());
        goto EmitOp;
      }

      case OffsetType::kAArch64_ADR:
      case OffsetType::kAArch64_ADRP: {
        uint32_t imm_lo = disp_imm32 & 0x3u;
        uint32_t imm_hi = disp_imm32 >> 2;
        opcode.add_imm(imm_lo, 29);
        opcode.add_imm(imm_hi, 5);
        goto EmitOp;
      }

      default:
        goto InvalidDisplacement;
    }
  }

  // --------------------------------------------------------------------------
  // [EmitOp - Opcode]
  // --------------------------------------------------------------------------

EmitOp:
  writer.emit32u_le(opcode.get());
  goto EmitDone;

  // --------------------------------------------------------------------------
  // [Done]
  // --------------------------------------------------------------------------

EmitDone:
  if (Support::test(options, InstOptions::kReserved)) {
#ifndef ASMJIT_NO_LOGGING
    if (_logger) {
      EmitterUtils::log_instruction_emitted(this, BaseInst::compose_arm_inst_id(inst_id, inst_cc), options, o0, o1, o2, op_ext, 0, 0, writer.cursor());
    }
#endif
  }

  reset_state();

  writer.done(this);
  return Error::kOk;

  // --------------------------------------------------------------------------
  // [Error Handler]
  // --------------------------------------------------------------------------

#define ERROR_HANDLER(ERR) ERR: err = make_error(Error::k##ERR); goto Failed;
  ERROR_HANDLER(OutOfMemory)
  ERROR_HANDLER(InvalidAddress)
  ERROR_HANDLER(InvalidAddressScale)
  ERROR_HANDLER(InvalidDisplacement)
  ERROR_HANDLER(InvalidElementIndex)
  ERROR_HANDLER(InvalidLabel)
  ERROR_HANDLER(InvalidImmediate)
  ERROR_HANDLER(InvalidInstruction)
  ERROR_HANDLER(InvalidPhysId)
  ERROR_HANDLER(InvalidRegType)
#undef ERROR_HANDLER

Failed:
#ifndef ASMJIT_NO_LOGGING
  return EmitterUtils::log_instruction_failed(this, err, inst_id, options, o0, o1, o2, op_ext);
#else
  reset_state();
  return report_error(err);
#endif
}

#undef ENC_OPS1
#undef ENC_OPS2
#undef ENC_OPS3
#undef ENC_OPS4

// a64::Assembler - Align
// ======================

Error Assembler::align(AlignMode align_mode, uint32_t alignment) {
  constexpr uint32_t kNopA64 = 0xD503201Fu; // [11010101|00000011|00100000|00011111].

  if (ASMJIT_UNLIKELY(!_code)) {
    return report_error(make_error(Error::kNotInitialized));
  }

  if (ASMJIT_UNLIKELY(uint32_t(align_mode) > uint32_t(AlignMode::kMaxValue))) {
    return report_error(make_error(Error::kInvalidArgument));
  }

  if (alignment <= 1) {
    return Error::kOk;
  }

  if (ASMJIT_UNLIKELY(!Support::is_power_of_2_up_to(alignment, Globals::kMaxAlignment))) {
    return report_error(make_error(Error::kInvalidArgument));
  }

  uint32_t i = uint32_t(Support::align_up_diff<size_t>(offset(), alignment));
  if (i == 0) {
    return Error::kOk;
  }

  CodeWriter writer(this);
  ASMJIT_PROPAGATE(writer.ensure_space(this, i));

  switch (align_mode) {
    case AlignMode::kCode: {
      uint32_t pattern = kNopA64;

      if (ASMJIT_UNLIKELY(offset() & 0x3u)) {
        return make_error(Error::kInvalidState);
      }

      while (i >= 4) {
        writer.emit32u_le(pattern);
        i -= 4;
      }

      ASMJIT_ASSERT(i == 0);
      break;
    }

    case AlignMode::kData:
    case AlignMode::kZero:
      writer.emit_zeros(i);
      break;
  }

  writer.done(this);

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

// a64::Assembler - Events
// =======================

Error Assembler::on_attach(CodeHolder& code) noexcept {
  ASMJIT_PROPAGATE(Base::on_attach(code));

  _instruction_alignment = uint8_t(4);
  update_emitter_funcs(this);

  return Error::kOk;
}

Error Assembler::on_detach(CodeHolder& code) noexcept {
  return Base::on_detach(code);
}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_AARCH64
