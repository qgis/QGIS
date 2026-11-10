// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_ARCHCOMMONS_H_INCLUDED
#define ASMJIT_CORE_ARCHCOMMONS_H_INCLUDED

// This file provides architecture-specific classes that are required in the core library. For example Imm operand
// allows to be created from arm::Shift in a const-expr way, so the arm::Shift must be provided. So this header file
// provides everything architecture-specific that is used by the Core API.

#include <asmjit/core/globals.h>

ASMJIT_BEGIN_SUB_NAMESPACE(arm)

//! \addtogroup asmjit_arm
//! \{

//! Condition code (both AArch32 & AArch64).
//!
//! \note This enumeration doesn't match condition code that is used in AArch32/AArch64 opcodes. In general this
//! condition code is encoded as `(cc - 2) & 0xF` so that `kAL` condition code is zero and encoded as 0xE in opcode.
//! This makes it easier to use a condition code as an instruction modifier that defaults to 'al'.
enum class CondCode : uint8_t {
  kAL             = 0x00u,      //!< (no condition code) (always)
  kNA             = 0x01u,      //!< (not available)     (special)
  kEQ             = 0x02u,      //!<        Z==1         (any_sign ==)
  kNE             = 0x03u,      //!<        Z==0         (any_sign !=)
  kCS             = 0x04u,      //!< C==1                (unsigned >=)
  kHS             = 0x04u,      //!< C==1                (unsigned >=)
  kLO             = 0x05u,      //!< C==0                (unsigned < )
  kCC             = 0x05u,      //!< C==0                (unsigned < )
  kMI             = 0x06u,      //!<               N==1  (is negative)
  kPL             = 0x07u,      //!<               N==0  (is positive or zero)
  kVS             = 0x08u,      //!<               V==1  (is overflow)
  kVC             = 0x09u,      //!<               V==0  (no overflow)
  kHI             = 0x0Au,      //!< C==1 & Z==0         (unsigned > )
  kLS             = 0x0Bu,      //!< C==0 | Z==1         (unsigned <=)
  kGE             = 0x0Cu,      //!<               N==V  (signed   >=)
  kLT             = 0x0Du,      //!<               N!=V  (signed   < )
  kGT             = 0x0Eu,      //!<        Z==0 & N==V  (signed   > )
  kLE             = 0x0Fu,      //!<        Z==1 | N!=V  (signed   <=)

  kZero           = kEQ,        //!< Zero flag (alias to equal).
  kNotZero        = kNE,        //!< Not zero (alias to Not Equal).

  kEqual          = kEQ,        //!< Equal     `a == b`.
  kNotEqual       = kNE,        //!< Not Equal `a != b`.

  kCarry          = kCS,        //!< Carry flag.
  kNotCarry       = kCC,        //!< Not carry.

  kSign           = kMI,        //!< Sign flag.
  kNotSign        = kPL,        //!< Not sign.

  kNegative       = kMI,        //!< Negative.
  kPositive       = kPL,        //!< Positive or zero.

  kOverflow       = kVS,        //!< Signed overflow.
  kNotOverflow    = kVC,        //!< Not signed overflow.

  kSignedLT       = kLT,        //!< Signed    `a <  b`.
  kSignedLE       = kLE,        //!< Signed    `a <= b`.
  kSignedGT       = kGT,        //!< Signed    `a >  b`.
  kSignedGE       = kGE,        //!< Signed    `a >= b`.

  kUnsignedLT     = kLO,        //!< Unsigned  `a <  b`.
  kUnsignedLE     = kLS,        //!< Unsigned  `a <= b`.
  kUnsignedGT     = kHI,        //!< Unsigned  `a >  b`.
  kUnsignedGE     = kHS,        //!< Unsigned  `a >= b`.

  kBTZero         = kZero,      //!< Tested bit is zero.
  kBTNotZero      = kNotZero,   //!< Tested bit is not zero.

  kAlways         = kAL,        //!< No condition code (always).

  kMaxValue       = 0x0Fu       //!< Maximum value of `CondCode`.
};


//! \cond
static constexpr CondCode _reverse_cond_table[] = {
  CondCode::kAL, // AL <- AL
  CondCode::kNA, // NA <- NA
  CondCode::kEQ, // EQ <- EQ
  CondCode::kNE, // NE <- NE
  CondCode::kLS, // LS <- CS
  CondCode::kHI, // HI <- LO
  CondCode::kMI, // MI <- MI
  CondCode::kPL, // PL <- PL
  CondCode::kVS, // VS <- VS
  CondCode::kVC, // VC <- VC
  CondCode::kLO, // LO <- HI
  CondCode::kCS, // CS <- LS
  CondCode::kLE, // LE <- GE
  CondCode::kGT, // GT <- LT
  CondCode::kLT, // LT <- GT
  CondCode::kGE  // GE <- LE
};
//! \endcond

//! Reverses a condition code (reverses the corresponding operands of a comparison).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR CondCode reverse_cond(CondCode cond) noexcept { return _reverse_cond_table[uint8_t(cond)]; }

//! Negates a condition code.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR CondCode negate_cond(CondCode cond) noexcept { return CondCode(uint8_t(cond) ^ uint8_t(1)); }

//! Memory offset mode.
//!
//! Describes either fixed, pre-index, or post-index offset modes.
enum class OffsetMode : uint32_t {
  //! Fixed offset mode (either no index at all or a regular index without a write-back).
  kFixed = 0u,
  //! Pre-index "[BASE, #Offset {, <shift>}]!" with write-back.
  kPreIndex = 1u,
  //! Post-index "[BASE], #Offset {, <shift>}" with write-back.
  kPostIndex = 2u
};

//! Shift operation predicate (ARM) describes either SHIFT or EXTEND operation.
//!
//! \note The constants are AsmJit specific. The first 5 values describe real constants on ARM32 and AArch64 hardware,
//! however, the addition constants that describe extend modes are specific to AsmJit and would be translated to the
//! AArch64 specific constants by the assembler.
enum class ShiftOp : uint32_t {
  //! Shift left logical operation (default).
  //!
  //! Available to all ARM architectures.
  kLSL = 0x00u,

  //! Shift right logical operation.
  //!
  //! Available to all ARM architectures.
  kLSR = 0x01u,

  //! Shift right arithmetic operation.
  //!
  //! Available to all ARM architectures.
  kASR = 0x02u,

  //! Rotate right operation (AArch32 only).
  kROR = 0x03u,

  //! Rotate right with carry operation (encoded as `ShiftOp::kROR` with zero) (AArch32 only).
  kRRX = 0x04u,

  //! Shift left by filling low order bits with ones.
  kMSL = 0x05u,

  //! UXTN extend register operation (AArch64 only).
  kUXTB = 0x06u,
  //! UXTH extend register operation (AArch64 only).
  kUXTH = 0x07u,
  //! UXTW extend register operation (AArch64 only).
  kUXTW = 0x08u,
  //! UXTX extend register operation (AArch64 only).
  kUXTX = 0x09u,

  //! SXTB extend register operation (AArch64 only).
  kSXTB = 0x0Au,
  //! SXTH extend register operation (AArch64 only).
  kSXTH = 0x0Bu,
  //! SXTW extend register operation (AArch64 only).
  kSXTW = 0x0Cu,
  //! SXTX extend register operation (AArch64 only).
  kSXTX = 0x0Du

  // NOTE: 0xE and 0xF are used by memory operand to specify POST|PRE offset mode.
};

//! Represents ARM immediate shift operation type and value.
class Shift {
public:
  //! Shift operation.
  ShiftOp _op;
  //! Shift Value.
  uint32_t _value;

  //! Default constructed Shift is not initialized.
  ASMJIT_INLINE_NODEBUG Shift() noexcept = default;

  //! Copy constructor (default)
  ASMJIT_INLINE_CONSTEXPR Shift(const Shift& other) noexcept = default;

  //! Constructs Shift from operation `op` and shift `value`.
  ASMJIT_INLINE_CONSTEXPR Shift(ShiftOp op, uint32_t value) noexcept
    : _op(op),
      _value(value) {}

  //! Returns the shift operation.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR ShiftOp op() const noexcept { return _op; }

  //! Sets shift operation to `op`.
  ASMJIT_INLINE_NODEBUG void set_pp(ShiftOp op) noexcept { _op = op; }

  //! Returns the shift amount.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR uint32_t value() const noexcept { return _value; }

  //! Sets shift amount to `value`.
  ASMJIT_INLINE_NODEBUG void set_value(uint32_t value) noexcept { _value = value; }
};

//! \}

ASMJIT_END_SUB_NAMESPACE

ASMJIT_BEGIN_SUB_NAMESPACE(a32)

using namespace arm;

//! Data type that can be encoded with AArch32 instruction identifier.
//!
//! \note Data types are frequently used with AArch32 SIMD instructions. For example `VMAX` instruction can
//! use almost all datatypes in a form `VMAX.F32`, `VMAX.S16`, `VMAX.U32`, etc... Emitter automatically adds
//! the required data type at emit level.
enum class DataType : uint32_t {
  //! No data type specified (default for all general purpose instructions).
  kNone = 0,
  //! 8-bit signed integer, specified as `.s8` in assembly.
  kS8 = 1,
  //! 16-bit signed integer, specified as `.s16` in assembly.
  kS16 = 2,
  //! 32-bit signed integer, specified as `.s32` in assembly.
  kS32 = 3,
  //! 64-bit signed integer, specified as `.s64` in assembly.
  kS64 = 4,
  //! 8-bit unsigned integer, specified as `.u8` in assembly.
  kU8 = 5,
  //! 16-bit unsigned integer, specified as `.u16` in assembly.
  kU16 = 6,
  //! 32-bit unsigned integer, specified as `.u32` in assembly.
  kU32 = 7,
  //! 64-bit unsigned integer, specified as `.u64` in assembly.
  kU64 = 8,
  //! 16-bit floating point (half precision), specified as `.f16` in assembly.
  kF16 = 10,
  //! 32-bit floating point (single precision), specified as `.f32` in assembly.
  kF32 = 11,
  //! 64-bit floating point (double precision), specified as `.f64` in assembly.
  kF64 = 12,
  //! 8-bit polynomial.
  kP8 = 13,
  //! 16-bit BF16 floating point.
  kBF16 = 14,
  //! 64-bit polynomial.
  kP64 = 15,

  //! Maximum value of `DataType`.
  kMaxValue = 15
};

static ASMJIT_INLINE_NODEBUG uint32_t data_type_size(DataType dt) noexcept {
  static constexpr uint8_t table[] = { 0, 1, 2, 4, 8, 1, 2, 4, 8, 2, 4, 8, 1, 2, 8 };
  return table[size_t(dt)];
}

ASMJIT_END_SUB_NAMESPACE

ASMJIT_BEGIN_SUB_NAMESPACE(a64)
using namespace arm;
ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_CORE_ARCHCOMMONS_H_INCLUDED
