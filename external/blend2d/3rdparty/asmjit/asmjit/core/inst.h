// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_INST_H_INCLUDED
#define ASMJIT_CORE_INST_H_INCLUDED

#include <asmjit/core/cpuinfo.h>
#include <asmjit/core/operand.h>
#include <asmjit/core/string.h>
#include <asmjit/support/support.h>

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_instruction_db
//! \{

//! Describes an instruction id and modifiers used together with the id.
//!
//! Each architecture has a set of valid instructions indexed from 0. Instruction with 0 id is, however, a special
//! instruction that describes a "no instruction" or "invalid instruction". Different architectures can assign a.
//! different instruction to the same id, each architecture typically has its own instructions indexed from 1.
//!
//! Instruction identifiers listed by architecture:
//!
//!   - \ref x86::Inst (X86 and X86_64)
//!   - \ref a64::Inst (AArch64)
using InstId = uint32_t;

//! Instruction id parts.
//!
//! A mask that specifies a bit-layout of \ref InstId.
enum class InstIdParts : uint32_t {
  // Common Masks
  // ------------

  //! Real id without any modifiers (always 16 least significant bits).
  kRealId   = 0x0000FFFFu,
  //! Instruction is abstract (or virtual, IR, etc...).
  kAbstract = 0x80000000u,

  // ARM Specific
  // ------------

  //! AArch32 first data type, used by ASIMD instructions (`inst.dt.dt2`).
  kA32_DT   = 0x000F0000u,
  //! AArch32 second data type, used by ASIMD instructions (`inst.dt.dt2`).
  kA32_DT2  = 0x00F00000u,
  //! AArch32/AArch64 condition code.
  kARM_Cond = 0x78000000u
};

//! Instruction options.
//!
//! Instruction options complement instruction identifier and attributes.
enum class InstOptions : uint32_t {
  //! No options.
  kNone = 0,

  //! Used internally by emitters for handling errors and rare cases.
  kReserved = 0x00000001u,

  //! Prevents following a jump during compilation (Compiler).
  kUnfollow = 0x00000002u,

  //! Overwrite the destination operand(s) (Compiler).
  //!
  //! Hint that is important for register liveness analysis. It tells the compiler that the destination operand will
  //! be overwritten now or by adjacent instructions. Compiler knows when a register is completely overwritten by a
  //! single instruction, for example you don't have to mark "movaps" or "pxor x, x", however, if a pair of
  //! instructions is used and the first of them doesn't completely overwrite the content of the destination,
  //! Compiler fails to mark that register as dead.
  //!
  //! X86 Specific
  //! ------------
  //!
  //!   - All instructions that always overwrite at least the size of the register the virtual-register uses, for
  //!     example "mov", "movq", "movaps" don't need the overwrite option to be used - conversion, shuffle, and
  //!     other miscellaneous instructions included.
  //!
  //!   - All instructions that clear the destination register if all operands are the same, for example "xor x, x",
  //!     "pcmpeqb x x", etc...
  //!
  //!   - Consecutive instructions that partially overwrite the variable until there is no old content require
  //!     `BaseCompiler::overwrite()` to be used. Some examples (not always the best use cases thought):
  //!
  //!     - `movlps xmm0, ?` followed by `movhps xmm0, ?` and vice versa
  //!     - `movlpd xmm0, ?` followed by `movhpd xmm0, ?` and vice versa
  //!     - `mov al, ?` followed by `and ax, 0xFF`
  //!     - `mov al, ?` followed by `mov ah, al`
  //!     - `pinsrq xmm0, ?, 0` followed by `pinsrq xmm0, ?, 1`
  //!
  //!   - If the allocated virtual register is used temporarily for scalar operations. For example if you allocate a
  //!     full vector like `x86::Compiler::new_xmm()` and then use that vector for scalar operations you should use
  //!     `overwrite()` directive:
  //!
  //!     - `sqrtss x, y` - only LO element of `x` is changed, if you don't
  //!       use HI elements, use `compiler.overwrite().sqrtss(x, y)`.
  kOverwrite = 0x00000004u,

  //! Emit short-form of the instruction.
  kShortForm = 0x00000010u,
  //! Emit long-form of the instruction.
  kLongForm = 0x00000020u,

  //! Conditional jump is likely to be taken.
  kTaken = 0x00000040u,
  //! Conditional jump is unlikely to be taken.
  kNotTaken = 0x00000080u,

  // X86 & X64 Options
  // -----------------

  //! Use ModMR instead of ModRM if applicable.
  kX86_ModMR = 0x00000100u,
  //! Use ModRM instead of ModMR if applicable.
  kX86_ModRM = 0x00000200u,
  //! Use 3-byte VEX prefix if possible (AVX) (must be 0x00000400).
  kX86_Vex3 = 0x00000400u,
  //! Use VEX prefix when both VEX|EVEX prefixes are available (HINT: AVX_VNNI).
  kX86_Vex = 0x00000800u,
  //! Use 4-byte EVEX prefix if possible (AVX-512) (must be 0x00001000).
  kX86_Evex = 0x00001000u,

  //! LOCK prefix (lock-enabled instructions only).
  kX86_Lock = 0x00002000u,
  //! REP prefix (string instructions only).
  kX86_Rep = 0x00004000u,
  //! REPNE prefix (string instructions only).
  kX86_Repne = 0x00008000u,

  //! XACQUIRE prefix (only allowed instructions).
  kX86_XAcquire = 0x00010000u,
  //! XRELEASE prefix (only allowed instructions).
  kX86_XRelease = 0x00020000u,

  //! AVX-512: embedded-rounding {er} and implicit {sae}.
  kX86_ER = 0x00040000u,
  //! AVX-512: suppress-all-exceptions {sae}.
  kX86_SAE = 0x00080000u,
  //! AVX-512: round-to-nearest (even) {rn-sae} (bits 00).
  kX86_RN_SAE = 0x00000000u,
  //! AVX-512: round-down (toward -inf) {rd-sae} (bits 01).
  kX86_RD_SAE = 0x00200000u,
  //! AVX-512: round-up (toward +inf) {ru-sae} (bits 10).
  kX86_RU_SAE = 0x00400000u,
  //! AVX-512: round-toward-zero (truncate) {rz-sae} (bits 11).
  kX86_RZ_SAE = 0x00600000u,
  //! AVX-512: Use zeroing {k}{z} instead of merging {k}.
  kX86_ZMask = 0x00800000u,

  //! AVX-512: Mask to get embedded rounding bits (2 bits).
  kX86_ERMask = kX86_RZ_SAE,
  //! AVX-512: Mask of all possible AVX-512 options except EVEX prefix flag.
  kX86_AVX512Mask = 0x00FC0000u,

  //! Force REX.B and/or VEX.B field (X64 only, used internally).
  kX86_OpCodeB = 0x01000000u,
  //! Force REX.X and/or VEX.X field (X64 only, used internally).
  kX86_OpCodeX = 0x02000000u,
  //! Force REX.R and/or VEX.R field (X64 only, used internally).
  kX86_OpCodeR = 0x04000000u,
  //! Force REX.W and/or VEX.W field (X64 only, used internally).
  kX86_OpCodeW = 0x08000000u,
  //! Force REX prefix (X64 only).
  kX86_Rex = 0x40000000u,
  //! Invalid REX prefix (set by X86 or when AH|BH|CH|DH regs are used on X64).
  kX86_InvalidRex = 0x80000000u
};
ASMJIT_DEFINE_ENUM_FLAGS(InstOptions)

//! Instruction control flow.
enum class InstControlFlow : uint32_t {
  //! Regular instruction.
  kRegular = 0u,
  //! Unconditional jump.
  kJump = 1u,
  //! Conditional jump (branch).
  kBranch = 2u,
  //! Function call.
  kCall = 3u,
  //! Function return.
  kReturn = 4u,

  //! Maximum value of `InstType`.
  kMaxValue = kReturn
};

//! Hint that is used when both input operands to the instruction are the same.
//!
//! Provides hints to the instruction RW query regarding special cases in which two or more operands are the same
//! registers. This is required by instructions such as XOR, AND, OR, SUB, etc... These hints will influence the
//! RW operations query.
enum class InstSameRegHint : uint8_t {
  //! No special handling.
  kNone = 0,
  //! Operands become read-only, the operation doesn't change the content - `X & X` and similar.
  kRO = 1,
  //! Operands become write-only, the content of the input(s) don't matter - `X ^ X`, `X - X`, and similar.
  kWO = 2
};

//! Options that can be used when converting instruction IDs to strings.
enum class InstStringifyOptions : uint32_t {
  //! No options.
  kNone = 0x00000000u,

  //! Stringify a full instruction name with known aliases.
  //!
  //! This option is designed for architectures where instruction aliases are common, for example X86, and where
  //! multiple aliases can be used in assembly code to distinguish between intention - for example instructions
  //! such as JZ and JE are the same, but the first is used in a context of equality to zero, and the second is
  //! used when two values equal (for example JE next to CMP).
  kAliases = 0x00000001u
};
ASMJIT_DEFINE_ENUM_FLAGS(InstStringifyOptions)

//! Instruction id, options, and extra_reg in a single structure. This structure exists mainly to simplify analysis
//! and validation API that requires `BaseInst` and `Operand[]` array.
class BaseInst {
public:
  //! \name Members
  //! \{

  //! Instruction id with modifiers.
  InstId _inst_id;
  //! Instruction options.
  InstOptions _options;
  //! Extra register used by the instruction (either REP register or AVX-512 selector).
  RegOnly _extra_reg;

  enum Id : uint32_t {
    //! Invalid or uninitialized instruction id.
    kIdNone = 0x00000000u,
    //! Abstract instruction (BaseBuilder and BaseCompiler).
    kIdAbstract = 0x80000000u
  };

  //! \}

  //! \name Construction & Destruction
  //! \{

  //! Creates a new BaseInst instance with `id` and `options` set.
  //!
  //! Default values of `id` and `options` are zero, which means 'none' instruction. Such instruction is guaranteed
  //! to never exist for any architecture supported by AsmJit.
  ASMJIT_INLINE_NODEBUG explicit BaseInst(InstId inst_id = 0, InstOptions options = InstOptions::kNone) noexcept
    : _inst_id(inst_id),
      _options(options),
      _extra_reg() {}

  ASMJIT_INLINE_NODEBUG BaseInst(InstId inst_id, InstOptions options, const RegOnly& extra_reg) noexcept
    : _inst_id(inst_id),
      _options(options),
      _extra_reg(extra_reg) {}

  ASMJIT_INLINE_NODEBUG BaseInst(InstId inst_id, InstOptions options, const Reg& extra_reg) noexcept
    : _inst_id(inst_id),
      _options(options),
      _extra_reg{extra_reg.signature(), extra_reg.id()} {}

  //! \}

  //! \name Instruction id and modifiers
  //! \{

  //! Returns the instruction id with modifiers.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG InstId inst_id() const noexcept { return _inst_id; }

  //! Sets the instruction id and modifiers from `inst_id`.
  ASMJIT_INLINE_NODEBUG void set_inst_id(InstId inst_id) noexcept { _inst_id = inst_id; }

  //! Resets the instruction id and modifiers to zero, see \ref kIdNone.
  ASMJIT_INLINE_NODEBUG void reset_inst_id() noexcept { _inst_id = 0; }

  //! Returns a real instruction id that doesn't contain any modifiers.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG InstId real_id() const noexcept { return _inst_id & uint32_t(InstIdParts::kRealId); }

  template<InstIdParts kPart>
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t inst_id_part() const noexcept {
    return (uint32_t(_inst_id) & uint32_t(kPart)) >> Support::ctz_const<kPart>;
  }

  template<InstIdParts kPart>
  ASMJIT_INLINE_NODEBUG void set_inst_id_part(uint32_t value) noexcept {
    _inst_id = (_inst_id & ~uint32_t(kPart)) | (value << Support::ctz_const<kPart>);
  }

  //! \}

  //! \name Instruction Options
  //! \{

  //! Returns instruction options associated with this instruction.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG InstOptions options() const noexcept { return _options; }

  //! Tests whether the given instruction `option` is enabled.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_option(InstOptions option) const noexcept { return Support::test(_options, option); }

  //! Replaces all instruction options by the given `options`.
  ASMJIT_INLINE_NODEBUG void set_options(InstOptions options) noexcept { _options = options; }

  //! Adds instruction options provided by `options`.
  ASMJIT_INLINE_NODEBUG void add_options(InstOptions options) noexcept { _options |= options; }

  //! Clears instruction options provided by `options`.
  ASMJIT_INLINE_NODEBUG void clear_options(InstOptions options) noexcept { _options &= ~options; }

  //! Resets all instruction options to `InstOptions::kNone` (there will be no instruction options active after reset).
  ASMJIT_INLINE_NODEBUG void reset_options() noexcept { _options = InstOptions::kNone; }

  //! \}

  //! \name Extra Register
  //! \{

  //! Tests whether the instruction has associated an extra register.
  //!
  //! \note Extra registers are currently only used on X86 by AVX-512 masking such as `{k}` and `{k}{z}` and by repeated
  //! instructions to explicitly assign a virtual register that would be ECX/RCX.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_extra_reg() const noexcept { return _extra_reg.is_reg(); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RegOnly& extra_reg() noexcept { return _extra_reg; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const RegOnly& extra_reg() const noexcept { return _extra_reg; }

  ASMJIT_INLINE_NODEBUG void set_extra_reg(const Reg& reg) noexcept { _extra_reg.init(reg); }

  ASMJIT_INLINE_NODEBUG void set_extra_reg(const RegOnly& reg) noexcept { _extra_reg.init(reg); }

  ASMJIT_INLINE_NODEBUG void reset_extra_reg() noexcept { _extra_reg.reset(); }

  //! \}

  //! \name ARM Specific
  //! \{

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG arm::CondCode arm_cond_code() const noexcept { return (arm::CondCode)inst_id_part<InstIdParts::kARM_Cond>(); }

  ASMJIT_INLINE_NODEBUG void set_arm_cond_code(arm::CondCode cc) noexcept { set_inst_id_part<InstIdParts::kARM_Cond>(uint32_t(cc)); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG a32::DataType arm_dt() const noexcept { return (a32::DataType)inst_id_part<InstIdParts::kA32_DT>(); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG a32::DataType arm_dt2() const noexcept { return (a32::DataType)inst_id_part<InstIdParts::kA32_DT2>(); }

  //! \}

  //! \name Statics
  //! \{

  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR InstId compose_arm_inst_id(uint32_t id, arm::CondCode cc) noexcept {
    return id | (uint32_t(cc) << Support::ctz_const<InstIdParts::kARM_Cond>);
  }

  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR InstId compose_arm_inst_id(uint32_t id, a32::DataType dt, arm::CondCode cc = arm::CondCode::kAL) noexcept {
    return id | (uint32_t(dt) << Support::ctz_const<InstIdParts::kA32_DT>)
              | (uint32_t(cc) << Support::ctz_const<InstIdParts::kARM_Cond>);
  }

  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR InstId compose_arm_inst_id(uint32_t id, a32::DataType dt, a32::DataType dt2, arm::CondCode cc = arm::CondCode::kAL) noexcept {
    return id | (uint32_t(dt) << Support::ctz_const<InstIdParts::kA32_DT>)
              | (uint32_t(dt2) << Support::ctz_const<InstIdParts::kA32_DT2>)
              | (uint32_t(cc) << Support::ctz_const<InstIdParts::kARM_Cond>);
  }

  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR InstId extract_real_id(uint32_t id) noexcept {
    return id & uint32_t(InstIdParts::kRealId);
  }

  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR arm::CondCode extract_arm_cond_code(uint32_t id) noexcept {
    return (arm::CondCode)((uint32_t(id) & uint32_t(InstIdParts::kARM_Cond)) >> Support::ctz_const<InstIdParts::kARM_Cond>);
  }

  //! \}
};

//! CPU read/write flags used by \ref InstRWInfo.
//!
//! These flags can be used to get a basic overview about CPU specifics flags used by instructions.
enum class CpuRWFlags : uint32_t {
  //! No flags.
  kNone = 0x00000000u,

  // Common RW Flags (0x000000FF)
  // ----------------------------

  //! Signed overflow flag.
  kOF = 0x00000001u,
  //! Carry flag.
  kCF = 0x00000002u,
  //! Zero and/or equality flag (1 if zero/equal).
  kZF = 0x00000004u,
  //! Sign flag (negative/sign, if set).
  kSF = 0x00000008u,

  // X86 Specific RW Flags
  // ----------------------------------

  //! Carry flag (X86|X86_64).
  kX86_CF = kCF,
  //! Overflow flag (X86|X86_64).
  kX86_OF = kOF,
  //! Sign flag (X86|X86_64).
  kX86_SF = kSF,
  //! Zero flag (X86|X86_64).
  kX86_ZF = kZF,

  //! Adjust flag (X86|X86_64).
  kX86_AF = 0x00000100u,
  //! Parity flag (X86|X86_64).
  kX86_PF = 0x00000200u,
  //! Direction flag (X86|X86_64).
  kX86_DF = 0x00000400u,
  //! Interrupt enable flag (X86|X86_64).
  kX86_IF = 0x00000800u,

  //! Alignment check flag (X86|X86_64).
  kX86_AC = 0x00001000u,

  //! FPU C0 status flag (X86|X86_64).
  kX86_C0 = 0x00010000u,
  //! FPU C1 status flag (X86|X86_64).
  kX86_C1 = 0x00020000u,
  //! FPU C2 status flag (X86|X86_64).
  kX86_C2 = 0x00040000u,
  //! FPU C3 status flag (X86|X86_64).
  kX86_C3 = 0x00080000u,

  // ARM Specific RW Flags
  // ----------------------------------

  kARM_V = kOF,
  kARM_C = kCF,
  kARM_Z = kZF,
  kARM_N = kSF,
  kARM_Q = 0x00000100u,
  kARM_GE = 0x00000200u
};
ASMJIT_DEFINE_ENUM_FLAGS(CpuRWFlags)

//! Operand read/write flags describe how the operand is accessed and some additional features.
enum class OpRWFlags : uint32_t {
  //! No flags.
  kNone = 0,

  //! Operand is read.
  kRead = 0x00000001u,

  //! Operand is written.
  kWrite = 0x00000002u,

  //! Operand is both read and written.
  kRW = 0x00000003u,

  //! Register operand can be replaced by a memory operand.
  kRegMem = 0x00000004u,

  //! The register must be allocated to the index of the previous register + 1.
  //!
  //! This flag is used by all architectures to describe instructions that use consecutive registers, where only the
  //! first one is encoded in the instruction, and the others are just a sequence that starts with the first one. On
  //! X86|X86_64 architecture this is used by instructions such as VP2INTERSECTD and VP2INTERSECTQ. On ARM/AArch64
  //! this is used by vector load and store instructions that can load or store multiple registers at once.
  kConsecutive = 0x00000008u,

  //! The `extend_byte_mask()` represents a zero extension.
  kZExt = 0x00000010u,

  //! The register must have assigned a unique physical ID, which cannot be assigned to any other register.
  kUnique = 0x00000080u,

  //! Register operand must use \ref OpRWInfo::phys_id().
  kRegPhysId = 0x00000100u,
  //! Base register of a memory operand must use \ref OpRWInfo::phys_id().
  kMemPhysId = 0x00000200u,

  //! This memory operand is only used to encode registers and doesn't access memory.
  //!
  //! X86 Specific
  //! ------------
  //!
  //! Instructions that use such feature include BNDLDX, BNDSTX, and LEA.
  kMemFake = 0x000000400u,

  //! Base register of the memory operand will be read.
  kMemBaseRead = 0x00001000u,
  //! Base register of the memory operand will be written.
  kMemBaseWrite = 0x00002000u,
  //! Base register of the memory operand will be read & written.
  kMemBaseRW = 0x00003000u,

  //! Index register of the memory operand will be read.
  kMemIndexRead = 0x00004000u,
  //! Index register of the memory operand will be written.
  kMemIndexWrite = 0x00008000u,
  //! Index register of the memory operand will be read & written.
  kMemIndexRW = 0x0000C000u,

  //! Base register of the memory operand will be modified before the operation.
  kMemBasePreModify = 0x00010000u,
  //! Base register of the memory operand will be modified after the operation.
  kMemBasePostModify = 0x00020000u
};
ASMJIT_DEFINE_ENUM_FLAGS(OpRWFlags)

// Don't remove these asserts. Read/Write flags are used extensively
// by Compiler and they must always be compatible with constants below.
static_assert(uint32_t(OpRWFlags::kRead) == 0x1, "OpRWFlags::kRead flag must be 0x1");
static_assert(uint32_t(OpRWFlags::kWrite) == 0x2, "OpRWFlags::kWrite flag must be 0x2");
static_assert(uint32_t(OpRWFlags::kRegMem) == 0x4, "OpRWFlags::kRegMem flag must be 0x4");

//! Read/Write information related to a single operand, used by \ref InstRWInfo.
struct OpRWInfo {
  //! \name Members
  //! \{

  //! Read/Write flags.
  OpRWFlags _op_flags;
  //! Physical register index, if required.
  uint8_t _phys_id;
  //! Size of a possible memory operand that can replace a register operand.
  uint8_t _rm_size;
  //! If non-zero, then this is a consecutive lead register, and the value describes how many registers follow.
  uint8_t _consecutive_lead_count;
  //! Reserved for future use.
  uint8_t _reserved[1];
  //! Read bit-mask where each bit represents one byte read from Reg/Mem.
  uint64_t _read_byte_mask;
  //! Write bit-mask where each bit represents one byte written to Reg/Mem.
  uint64_t _write_byte_mask;
  //! Zero/Sign extend bit-mask where each bit represents one byte written to Reg/Mem.
  uint64_t _extend_byte_mask;

  //! \}

  //! \name Reset
  //! \{

  //! Resets this operand information to all zeros.
  ASMJIT_INLINE_NODEBUG void reset() noexcept { *this = OpRWInfo{}; }

  //! Resets this operand info (resets all members) and set common information to the given `op_flags`,
  //! `register_size`, and possibly `phys_id`.
  inline void reset(OpRWFlags op_flags, uint32_t register_size, uint32_t phys_id = Reg::kIdBad) noexcept {
    _op_flags = op_flags;
    _phys_id = uint8_t(phys_id);
    _rm_size = Support::test(op_flags, OpRWFlags::kRegMem) ? uint8_t(register_size) : uint8_t(0);
    _consecutive_lead_count = 0;
    _reset_reserved();

    uint64_t mask = Support::lsb_mask<uint64_t>(Support::min<uint32_t>(register_size, 64));
    _read_byte_mask = Support::test(op_flags, OpRWFlags::kRead) ? mask : uint64_t(0);
    _write_byte_mask = Support::test(op_flags, OpRWFlags::kWrite) ? mask : uint64_t(0);
    _extend_byte_mask = 0;
  }

  ASMJIT_INLINE_NODEBUG void _reset_reserved() noexcept {
    _reserved[0] = uint8_t(0);
  }

  //! \}

  //! \name Operand Flags
  //! \{

  //! Returns operand flags.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG OpRWFlags op_flags() const noexcept { return _op_flags; }

  //! Tests whether operand flags contain the given `flag`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_op_flag(OpRWFlags flag) const noexcept { return Support::test(_op_flags, flag); }

  //! Adds the given `flags` to operand flags.
  ASMJIT_INLINE_NODEBUG void add_op_flags(OpRWFlags flags) noexcept { _op_flags |= flags; }

  //! Removes the given `flags` from operand flags.
  ASMJIT_INLINE_NODEBUG void clear_op_flags(OpRWFlags flags) noexcept { _op_flags &= ~flags; }

  //! Tests whether this operand is read from.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_read() const noexcept { return has_op_flag(OpRWFlags::kRead); }

  //! Tests whether this operand is written to.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_write() const noexcept { return has_op_flag(OpRWFlags::kWrite); }

  //! Tests whether this operand is both read and write.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_read_write() const noexcept { return (_op_flags & OpRWFlags::kRW) == OpRWFlags::kRW; }

  //! Tests whether this operand is read only.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_read_only() const noexcept { return (_op_flags & OpRWFlags::kRW) == OpRWFlags::kRead; }

  //! Tests whether this operand is write only.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_write_only() const noexcept { return (_op_flags & OpRWFlags::kRW) == OpRWFlags::kWrite; }

  //! Returns the type of a lead register, which is followed by consecutive registers.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t consecutive_lead_count() const noexcept { return _consecutive_lead_count; }

  //! Tests whether this operand is Reg/Mem
  //!
  //! Reg/Mem operands can use either register or memory.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_rm() const noexcept { return has_op_flag(OpRWFlags::kRegMem); }

  //! Tests whether the operand will be zero extended.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_zext() const noexcept { return has_op_flag(OpRWFlags::kZExt); }

  //! Tests whether the operand must have allocated a unique physical id that cannot be shared with other register
  //! operands.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_unique() const noexcept { return has_op_flag(OpRWFlags::kUnique); }

  //! \}

  //! \name Memory Flags
  //! \{

  //! Tests whether this is a fake memory operand, which is only used, because of encoding. Fake memory operands do
  //! not access any memory, they are only used to encode registers.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_mem_fake() const noexcept { return has_op_flag(OpRWFlags::kMemFake); }

  //! Tests whether the instruction's memory BASE register is used.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_mem_base_used() const noexcept { return has_op_flag(OpRWFlags::kMemBaseRW); }

  //! Tests whether the instruction reads from its BASE registers.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_mem_base_read() const noexcept { return has_op_flag(OpRWFlags::kMemBaseRead); }

  //! Tests whether the instruction writes to its BASE registers.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_mem_base_write() const noexcept { return has_op_flag(OpRWFlags::kMemBaseWrite); }

  //! Tests whether the instruction reads and writes from/to its BASE registers.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_mem_base_read_write() const noexcept { return (_op_flags & OpRWFlags::kMemBaseRW) == OpRWFlags::kMemBaseRW; }

  //! Tests whether the instruction only reads from its BASE registers.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_mem_base_read_only() const noexcept { return (_op_flags & OpRWFlags::kMemBaseRW) == OpRWFlags::kMemBaseRead; }

  //! Tests whether the instruction only writes to its BASE registers.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_mem_base_write_only() const noexcept { return (_op_flags & OpRWFlags::kMemBaseRW) == OpRWFlags::kMemBaseWrite; }

  //! Tests whether the instruction modifies the BASE register before it uses it to calculate the target address.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_mem_base_pre_modify() const noexcept { return has_op_flag(OpRWFlags::kMemBasePreModify); }

  //! Tests whether the instruction modifies the BASE register after it uses it to calculate the target address.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_mem_base_post_modify() const noexcept { return has_op_flag(OpRWFlags::kMemBasePostModify); }

  //! Tests whether the instruction's memory INDEX register is used.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_mem_index_used() const noexcept { return has_op_flag(OpRWFlags::kMemIndexRW); }

  //! Tests whether the instruction reads the INDEX registers.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_mem_index_read() const noexcept { return has_op_flag(OpRWFlags::kMemIndexRead); }

  //! Tests whether the instruction writes to its INDEX registers.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_mem_index_write() const noexcept { return has_op_flag(OpRWFlags::kMemIndexWrite); }

  //! Tests whether the instruction reads and writes from/to its INDEX registers.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_mem_index_read_write() const noexcept { return (_op_flags & OpRWFlags::kMemIndexRW) == OpRWFlags::kMemIndexRW; }

  //! Tests whether the instruction only reads from its INDEX registers.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_mem_index_read_only() const noexcept { return (_op_flags & OpRWFlags::kMemIndexRW) == OpRWFlags::kMemIndexRead; }

  //! Tests whether the instruction only writes to its INDEX registers.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_mem_index_write_only() const noexcept { return (_op_flags & OpRWFlags::kMemIndexRW) == OpRWFlags::kMemIndexWrite; }

  //! \}

  //! \name Physical Register ID
  //! \{

  //! Returns a physical id of the register that is fixed for this operand.
  //!
  //! Returns \ref Reg::kIdBad if any register can be used.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t phys_id() const noexcept { return _phys_id; }

  //! Tests whether \ref phys_id() would return a valid physical register id.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_phys_id() const noexcept { return _phys_id != Reg::kIdBad; }

  //! Sets physical register id, which would be fixed for this operand.
  ASMJIT_INLINE_NODEBUG void set_phys_id(uint32_t phys_id) noexcept { _phys_id = uint8_t(phys_id); }

  //! \}

  //! \name Reg/Mem Information
  //! \{

  //! Returns Reg/Mem size of the operand.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t rm_size() const noexcept { return _rm_size; }

  //! Sets Reg/Mem size of the operand.
  ASMJIT_INLINE_NODEBUG void set_rm_size(uint32_t rm_size) noexcept { _rm_size = uint8_t(rm_size); }

  //! \}

  //! \name Read & Write Masks
  //! \{

  //! Returns read mask.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint64_t read_byte_mask() const noexcept { return _read_byte_mask; }

  //! Sets read mask.
  ASMJIT_INLINE_NODEBUG void set_read_byte_mask(uint64_t mask) noexcept { _read_byte_mask = mask; }

  //! Returns write mask.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint64_t write_byte_mask() const noexcept { return _write_byte_mask; }

  //! Sets write mask.
  ASMJIT_INLINE_NODEBUG void set_write_byte_mask(uint64_t mask) noexcept { _write_byte_mask = mask; }

  //! Returns extend mask.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint64_t extend_byte_mask() const noexcept { return _extend_byte_mask; }

  //! Sets extend mask.
  ASMJIT_INLINE_NODEBUG void set_extend_byte_mask(uint64_t mask) noexcept { _extend_byte_mask = mask; }

  //! \}
};

//! Flags used by \ref InstRWInfo.
enum class InstRWFlags : uint32_t {
  //! No flags.
  kNone = 0x00000000u,

  //! Describes a move operation.
  //!
  //! This flag is used by RA to eliminate moves that are guaranteed to be moves only.
  kMovOp = 0x00000001u
};
ASMJIT_DEFINE_ENUM_FLAGS(InstRWFlags)

//! Read/Write information of an instruction.
struct InstRWInfo {
  //! \name Members
  //! \{

  //! Instruction flags (there are no flags at the moment, this field is reserved).
  InstRWFlags _inst_flags;
  //! CPU flags read.
  CpuRWFlags _read_flags;
  //! CPU flags written.
  CpuRWFlags _write_flags;
  //! Count of operands.
  uint8_t _op_count;
  //! CPU feature required for replacing register operand with memory operand.
  uint8_t _rm_feature;
  //! Reserved for future use.
  uint8_t _reserved[18];
  //! Read/Write info of extra register (rep{} or kz{}).
  OpRWInfo _extra_reg;
  //! Read/Write info of instruction operands.
  OpRWInfo _operands[Globals::kMaxOpCount];

  //! \}

  //! \name Commons
  //! \{

  //! Resets this RW information to all zeros.
  ASMJIT_INLINE_NODEBUG void reset() noexcept { *this = InstRWInfo{}; }

  //! \}

  //! \name Instruction Flags
  //! \{

  //! Returns flags associated with the instruction, see \ref InstRWFlags.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG InstRWFlags inst_flags() const noexcept { return _inst_flags; }

  //! Tests whether the instruction flags contain `flag`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_inst_flag(InstRWFlags flag) const noexcept { return Support::test(_inst_flags, flag); }

  //! Tests whether the instruction flags contain \ref InstRWFlags::kMovOp.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_mov_op() const noexcept { return has_inst_flag(InstRWFlags::kMovOp); }

  //! \}

  //! \name CPU Flags Information
  //! \{

  //! Returns a mask of CPU flags read.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG CpuRWFlags read_flags() const noexcept { return _read_flags; }

  //! Returns a mask of CPU flags written.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG CpuRWFlags write_flags() const noexcept { return _write_flags; }

  //! \}

  //! \name Reg/Mem Information
  //! \{

  //! Returns the CPU feature required to replace a register operand with memory operand. If the returned feature is
  //! zero (none) then this instruction either doesn't provide memory operand combination or there is no extra CPU
  //! feature required.
  //!
  //! X86 Specific
  //! ------------
  //!
  //! Some AVX+ instructions may require extra features for replacing registers with memory operands, for example
  //! VPSLLDQ instruction only supports `vpslldq reg, reg, imm` combination on AVX/AVX2 capable CPUs and requires
  //! AVX-512 for `vpslldq reg, mem, imm` combination.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t rm_feature() const noexcept { return _rm_feature; }

  //! \}

  //! \name Operand Read/Write Information
  //! \{

  //! Returns RW information of extra register operand (extra_reg).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const OpRWInfo& extra_reg() const noexcept { return _extra_reg; }

  //! Returns RW information of all instruction's operands.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const OpRWInfo* operands() const noexcept { return _operands; }

  //! Returns RW information of the operand at the given `index`.
  [[nodiscard]]
  inline const OpRWInfo& operand(size_t index) const noexcept {
    ASMJIT_ASSERT(index < Globals::kMaxOpCount);
    return _operands[index];
  }

  //! Returns the number of operands this instruction has.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t op_count() const noexcept { return _op_count; }

  //! \}
};

//! Validation flags that can be used with \ref InstAPI::validate().
enum class ValidationFlags : uint8_t {
  //! No flags.
  kNone = 0,
  //! Allow virtual registers in the instruction.
  kEnableVirtRegs = 0x01u
};
ASMJIT_DEFINE_ENUM_FLAGS(ValidationFlags)

//! Instruction API.
namespace InstAPI {

#ifndef ASMJIT_NO_TEXT
//! Appends the name of the instruction specified by `inst_id` and `options` into the `output` string.
//!
//! \note Instruction options would only affect instruction prefix & suffix, other options would be ignored.
//! If `inst_options` is zero then only raw instruction name (without any additional text) will be appended.
ASMJIT_API Error inst_id_to_string(Arch arch, InstId inst_id, InstStringifyOptions options, String& output) noexcept;

//! Parses an instruction name in the given string `s`. Length is specified by `len` argument, which can be
//! `SIZE_MAX` if `s` is known to be null terminated.
//!
//! Returns the parsed instruction id or \ref BaseInst::kIdNone if no such instruction exists.
[[nodiscard]]
ASMJIT_API InstId string_to_inst_id(Arch arch, const char* s, size_t len) noexcept;
#endif // !ASMJIT_NO_TEXT

#ifndef ASMJIT_NO_INTROSPECTION
//! Validates the given instruction considering the given `validation_flags`.
[[nodiscard]]
ASMJIT_API Error validate(Arch arch, const BaseInst& inst, const Operand_* operands, size_t op_count, ValidationFlags validation_flags = ValidationFlags::kNone) noexcept;

//! Gets Read/Write information of the given instruction.
ASMJIT_API Error query_rw_info(Arch arch, const BaseInst& inst, const Operand_* operands, size_t op_count, InstRWInfo* out) noexcept;

//! Gets CPU features required by the given instruction.
ASMJIT_API Error query_features(Arch arch, const BaseInst& inst, const Operand_* operands, size_t op_count, CpuFeatures* out) noexcept;
#endif // !ASMJIT_NO_INTROSPECTION

} // {InstAPI}

//! \}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_CORE_INST_H_INCLUDED
