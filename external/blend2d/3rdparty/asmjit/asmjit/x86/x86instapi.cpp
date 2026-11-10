// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#if !defined(ASMJIT_NO_X86)

#include <asmjit/core/cpuinfo.h>
#include <asmjit/core/instdb_p.h>
#include <asmjit/core/misc_p.h>
#include <asmjit/x86/x86instapi_p.h>
#include <asmjit/x86/x86instdb_p.h>
#include <asmjit/x86/x86opcode_p.h>
#include <asmjit/x86/x86operand.h>

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

namespace InstInternal {

// x86::InstInternal - Text
// ========================

#ifndef ASMJIT_NO_TEXT
Error inst_id_to_string(InstId inst_id, InstStringifyOptions options, String& output) noexcept {
  if (ASMJIT_UNLIKELY(!Inst::is_defined_id(inst_id)))
    return make_error(Error::kInvalidInstruction);

  return InstNameUtils::decode(InstDB::_inst_name_index_table[inst_id], options, InstDB::_inst_name_string_table, output);
}

InstId string_to_inst_id(const char* s, size_t len) noexcept {
  if (ASMJIT_UNLIKELY(!s)) {
    return BaseInst::kIdNone;
  }

  if (len == SIZE_MAX) {
    len = strlen(s);
  }

  if (len == 0u || len > InstDB::_inst_name_index.max_name_length) {
    return BaseInst::kIdNone;
  }

  InstId inst_id = InstNameUtils::find_instruction(s, len, InstDB::_inst_name_index_table, InstDB::_inst_name_string_table, InstDB::_inst_name_index);
  if (inst_id != BaseInst::kIdNone) {
    return inst_id;
  }

  uint32_t alias_index = InstNameUtils::find_alias(s, len, InstDB::alias_name_index_table, InstDB::alias_name_string_table, InstDB::kAliasTableSize);
  if (alias_index != Globals::kInvalidId) {
    return InstDB::alias_index_to_inst_id_table[alias_index];
  }

  return BaseInst::kIdNone;
}
#endif // !ASMJIT_NO_TEXT

// x86::InstInternal - Validate
// ============================

#ifndef ASMJIT_NO_INTROSPECTION
struct X86ValidationData {
  //! Allowed registers by \ref RegType.
  RegMask allowed_reg_mask[uint32_t(RegType::kMaxValue) + 1];
  uint32_t allowed_mem_base_regs;
  uint32_t allowed_mem_index_regs;
};

#define VALUE(x) \
  (x == uint32_t(RegType::kPC       )) ? InstDB::OpFlags::kNone     : \
  (x == uint32_t(RegType::kGp8Lo    )) ? InstDB::OpFlags::kRegGpbLo : \
  (x == uint32_t(RegType::kGp8Hi    )) ? InstDB::OpFlags::kRegGpbHi : \
  (x == uint32_t(RegType::kGp16     )) ? InstDB::OpFlags::kRegGpw   : \
  (x == uint32_t(RegType::kGp32     )) ? InstDB::OpFlags::kRegGpd   : \
  (x == uint32_t(RegType::kGp64     )) ? InstDB::OpFlags::kRegGpq   : \
  (x == uint32_t(RegType::kVec128   )) ? InstDB::OpFlags::kRegXmm   : \
  (x == uint32_t(RegType::kVec256   )) ? InstDB::OpFlags::kRegYmm   : \
  (x == uint32_t(RegType::kVec512   )) ? InstDB::OpFlags::kRegZmm   : \
  (x == uint32_t(RegType::kMask     )) ? InstDB::OpFlags::kRegKReg  : \
  (x == uint32_t(RegType::kX86_Mm   )) ? InstDB::OpFlags::kRegMm    : \
  (x == uint32_t(RegType::kSegment  )) ? InstDB::OpFlags::kRegSReg  : \
  (x == uint32_t(RegType::kControl  )) ? InstDB::OpFlags::kRegCReg  : \
  (x == uint32_t(RegType::kDebug    )) ? InstDB::OpFlags::kRegDReg  : \
  (x == uint32_t(RegType::kX86_St   )) ? InstDB::OpFlags::kRegSt    : \
  (x == uint32_t(RegType::kX86_Bnd  )) ? InstDB::OpFlags::kRegBnd   : \
  (x == uint32_t(RegType::kTile     )) ? InstDB::OpFlags::kRegTmm   : InstDB::OpFlags::kNone
static const InstDB::OpFlags op_flag_from_reg_type_table[uint32_t(RegType::kMaxValue) + 1] = { ASMJIT_LOOKUP_TABLE_32(VALUE, 0) };
#undef VALUE

#define REG_MASK_FROM_REG_TYPE_X86(x) \
  (x == uint32_t(RegType::kPC       )) ? 0x00000001u : \
  (x == uint32_t(RegType::kGp8Lo    )) ? 0x0000000Fu : \
  (x == uint32_t(RegType::kGp8Hi    )) ? 0x0000000Fu : \
  (x == uint32_t(RegType::kGp16     )) ? 0x000000FFu : \
  (x == uint32_t(RegType::kGp32     )) ? 0x000000FFu : \
  (x == uint32_t(RegType::kGp64     )) ? 0x000000FFu : \
  (x == uint32_t(RegType::kVec128   )) ? 0x000000FFu : \
  (x == uint32_t(RegType::kVec256   )) ? 0x000000FFu : \
  (x == uint32_t(RegType::kVec512   )) ? 0x000000FFu : \
  (x == uint32_t(RegType::kMask     )) ? 0x000000FFu : \
  (x == uint32_t(RegType::kX86_Mm   )) ? 0x000000FFu : \
  (x == uint32_t(RegType::kSegment  )) ? 0x0000007Eu : \
  (x == uint32_t(RegType::kControl  )) ? 0x0000FFFFu : \
  (x == uint32_t(RegType::kDebug    )) ? 0x000000FFu : \
  (x == uint32_t(RegType::kX86_St   )) ? 0x000000FFu : \
  (x == uint32_t(RegType::kX86_Bnd  )) ? 0x0000000Fu : \
  (x == uint32_t(RegType::kTile     )) ? 0x000000FFu : 0u

#define REG_MASK_FROM_REG_TYPE_X64(x) \
  (x == uint32_t(RegType::kPC       )) ? 0x00000001u : \
  (x == uint32_t(RegType::kGp8Lo    )) ? 0x0000FFFFu : \
  (x == uint32_t(RegType::kGp8Hi    )) ? 0x0000000Fu : \
  (x == uint32_t(RegType::kGp16     )) ? 0x0000FFFFu : \
  (x == uint32_t(RegType::kGp32     )) ? 0x0000FFFFu : \
  (x == uint32_t(RegType::kGp64     )) ? 0x0000FFFFu : \
  (x == uint32_t(RegType::kVec128   )) ? 0xFFFFFFFFu : \
  (x == uint32_t(RegType::kVec256   )) ? 0xFFFFFFFFu : \
  (x == uint32_t(RegType::kVec512   )) ? 0xFFFFFFFFu : \
  (x == uint32_t(RegType::kMask     )) ? 0x000000FFu : \
  (x == uint32_t(RegType::kX86_Mm   )) ? 0x000000FFu : \
  (x == uint32_t(RegType::kSegment  )) ? 0x0000007Eu : \
  (x == uint32_t(RegType::kControl  )) ? 0x0000FFFFu : \
  (x == uint32_t(RegType::kDebug    )) ? 0x0000FFFFu : \
  (x == uint32_t(RegType::kX86_St   )) ? 0x000000FFu : \
  (x == uint32_t(RegType::kX86_Bnd  )) ? 0x0000000Fu : \
  (x == uint32_t(RegType::kTile     )) ? 0x000000FFu : 0u

#define B(RegType) (uint32_t(1) << uint32_t(RegType))

static const X86ValidationData x86_validation_data = {
  { ASMJIT_LOOKUP_TABLE_32(REG_MASK_FROM_REG_TYPE_X86, 0) },
  B(RegType::kGp16) | B(RegType::kGp32) | B(RegType::kPC)     | B(RegType::kLabelTag),
  B(RegType::kGp16) | B(RegType::kGp32) | B(RegType::kVec128) | B(RegType::kVec256) | B(RegType::kVec512)
};

static const X86ValidationData x64_validation_data = {
  { ASMJIT_LOOKUP_TABLE_32(REG_MASK_FROM_REG_TYPE_X64, 0) },
  B(RegType::kGp32) | B(RegType::kGp64) | B(RegType::kPC)     | B(RegType::kLabelTag),
  B(RegType::kGp32) | B(RegType::kGp64) | B(RegType::kVec128) | B(RegType::kVec256) | B(RegType::kVec512)
};

#undef B

#undef REG_MASK_FROM_REG_TYPE_X64
#undef REG_MASK_FROM_REG_TYPE_X86

static ASMJIT_INLINE bool is_zmm_or_m512(const Operand_& op) noexcept {
  return op.is_vec512() || (op.is_mem() && op.as<Mem>().size() == 64);
}

static ASMJIT_INLINE bool check_op_sig(const InstDB::OpSignature& op, const InstDB::OpSignature& ref, bool& imm_out_of_range) noexcept {
  // Fail if operand types are incompatible.
  InstDB::OpFlags common_flags = op.flags() & ref.flags();

  if (!Support::test(common_flags, InstDB::OpFlags::kOpMask)) {
    // Mark temporarily `imm_out_of_range` so we can return a more descriptive error later.
    if (op.has_imm() && ref.has_imm()) {
      imm_out_of_range = true;
      return true;
    }

    return false;
  }

  // Fail if some memory specific flags do not match.
  if (Support::test(common_flags, InstDB::OpFlags::kMemMask)) {
    if (ref.has_flag(InstDB::OpFlags::kFlagMemBase) && !op.has_flag(InstDB::OpFlags::kFlagMemBase)) {
      return false;
    }
  }

  // Fail if register indexes do not match.
  if (Support::test(common_flags, InstDB::OpFlags::kRegMask)) {
    if (ref.reg_mask() && !Support::test(op.reg_mask(), ref.reg_mask())) {
      return false;
    }
  }

  return true;
}

static ASMJIT_FAVOR_SIZE Error validate(InstDB::Mode mode, const BaseInst& inst, const Operand_* operands, size_t op_count, ValidationFlags validation_flags) noexcept {
  uint32_t i;

  // Get the instruction data.
  const X86ValidationData* vd = (mode == InstDB::Mode::kX86) ? &x86_validation_data : &x64_validation_data;
  InstId inst_id = inst.inst_id();
  InstOptions options = inst.options();

  if (ASMJIT_UNLIKELY(!Inst::is_defined_id(inst_id))) {
    return make_error(Error::kInvalidInstruction);
  }

  const InstDB::InstInfo& inst_info = InstDB::inst_info_by_id(inst_id);
  const InstDB::CommonInfo& common_info = inst_info.common_info();

  InstDB::InstFlags inst_flags = inst_info.flags();

  constexpr InstOptions kRepAny = InstOptions::kX86_Rep | InstOptions::kX86_Repne;
  constexpr InstOptions kXAcqXRel = InstOptions::kX86_XAcquire | InstOptions::kX86_XRelease;
  constexpr InstOptions kAvx512Options = InstOptions::kX86_ZMask | InstOptions::kX86_ER | InstOptions::kX86_SAE;

  // Validate LOCK|XACQUIRE|XRELEASE Prefixes
  // ----------------------------------------

  if (Support::test(options, InstOptions::kX86_Lock | kXAcqXRel)) {
    if (Support::test(options, InstOptions::kX86_Lock)) {
      if (ASMJIT_UNLIKELY(!Support::test(inst_flags, InstDB::InstFlags::kLock) && !Support::test(options, kXAcqXRel))) {
        return make_error(Error::kInvalidLockPrefix);
      }

      if (ASMJIT_UNLIKELY(op_count < 1 || !operands[0].is_mem())) {
        return make_error(Error::kInvalidLockPrefix);
      }
    }

    if (Support::test(options, kXAcqXRel)) {
      if (ASMJIT_UNLIKELY(!Support::test(options, InstOptions::kX86_Lock) || (options & kXAcqXRel) == kXAcqXRel)) {
        return make_error(Error::kInvalidPrefixCombination);
      }

      if (ASMJIT_UNLIKELY(Support::test(options, InstOptions::kX86_XAcquire) && !Support::test(inst_flags, InstDB::InstFlags::kXAcquire))) {
        return make_error(Error::kInvalidXAcquirePrefix);
      }

      if (ASMJIT_UNLIKELY(Support::test(options, InstOptions::kX86_XRelease) && !Support::test(inst_flags, InstDB::InstFlags::kXRelease))) {
        return make_error(Error::kInvalidXReleasePrefix);
      }
    }
  }

  // Validate REP and REPNE Prefixes
  // -------------------------------

  if (Support::test(options, kRepAny)) {
    if (ASMJIT_UNLIKELY((options & kRepAny) == kRepAny)) {
      return make_error(Error::kInvalidPrefixCombination);
    }

    if (ASMJIT_UNLIKELY(!Support::test(inst_flags, InstDB::InstFlags::kRep))) {
      return make_error(Error::kInvalidRepPrefix);
    }
  }

  // Translate Each Operand to the Corresponding OpSignature
  // -------------------------------------------------------

  InstDB::OpSignature op_sig_translated[Globals::kMaxOpCount];
  InstDB::OpFlags combined_op_flags = InstDB::OpFlags::kNone;
  RegMask combined_reg_mask = 0;
  const Mem* mem_op = nullptr;

  for (i = 0; i < op_count; i++) {
    const Operand_& op = operands[i];
    if (op.op_type() == OperandType::kNone) {
      break;
    }

    InstDB::OpFlags op_flags = InstDB::OpFlags::kNone;
    RegMask reg_mask = 0;

    switch (op.op_type()) {
      case OperandType::kReg: {
        RegType reg_type = op.as<Reg>().reg_type();
        op_flags = op_flag_from_reg_type_table[size_t(reg_type)];

        if (ASMJIT_UNLIKELY(op_flags == InstDB::OpFlags::kNone)) {
          return make_error(Error::kInvalidRegType);
        }

        // If `reg_id` is equal or greater than Operand::kVirtIdMin it means that the register is virtual and its
        // index will be assigned later by the register allocator. We must pass unless asked to disallow virtual
        // registers.
        uint32_t reg_id = op.id();
        if (reg_id < Operand::kVirtIdMin) {
          if (ASMJIT_UNLIKELY(reg_id >= 32)) {
            return make_error(Error::kInvalidPhysId);
          }

          if (ASMJIT_UNLIKELY(Support::bit_test(vd->allowed_reg_mask[size_t(reg_type)], reg_id) == 0)) {
            return make_error(Error::kInvalidPhysId);
          }

          reg_mask = Support::bit_mask<RegMask>(reg_id);
          combined_reg_mask |= reg_mask;
        }
        else {
          if (uint32_t(validation_flags & ValidationFlags::kEnableVirtRegs) == 0) {
            return make_error(Error::kIllegalVirtReg);
          }
          reg_mask = 0xFFFFFFFFu;
        }
        break;
      }

      // TODO: Validate base and index and combine these with `combined_reg_mask`.
      case OperandType::kMem: {
        const Mem& m = op.as<Mem>();
        mem_op = &m;

        uint32_t mem_size = m.size();
        RegType base_type = m.base_type();
        RegType index_type = m.index_type();

        if (m.segment_id() > 6) {
          return make_error(Error::kInvalidSegment);
        }

        // Validate AVX-512 broadcast {1tox}.
        if (m.has_broadcast()) {
          if (mem_size != 0) {
            // If the size is specified it has to match the broadcast size.
            if (ASMJIT_UNLIKELY(common_info.has_avx512_bcst32() && mem_size != 4)) {
              return make_error(Error::kInvalidBroadcast);
            }

            if (ASMJIT_UNLIKELY(common_info.has_avx512_bcst64() && mem_size != 8)) {
              return make_error(Error::kInvalidBroadcast);
            }
          }
          else {
            // If there is no size we implicitly calculate it so we can validate N in {1toN} properly.
            mem_size = common_info.has_avx512_bcst64() ? 8 :
                      common_info.has_avx512_bcst32() ? 4 : 2;
          }

          mem_size <<= uint32_t(m.get_broadcast());
        }

        if (base_type != RegType::kNone && base_type > RegType::kLabelTag) {
          uint32_t base_id = m.base_id();

          if (m.is_reg_home()) {
            // Home address of a virtual register. In such case we don't want to validate the type of the
            // base register as it will always be patched to ESP|RSP.
          }
          else if (ASMJIT_UNLIKELY(!Support::bit_test(vd->allowed_mem_base_regs, base_type))) {
            return make_error(Error::kInvalidAddress);
          }

          // Create information that will be validated only if this is an implicit memory operand. Basically
          // only usable for string instructions and other instructions where memory operand is implicit and
          // has 'seg:[reg]' form.
          if (base_id < Operand::kVirtIdMin) {
            if (ASMJIT_UNLIKELY(base_id >= 32)) {
              return make_error(Error::kInvalidPhysId);
            }

            // Physical base id.
            reg_mask = Support::bit_mask<RegMask>(base_id);
            combined_reg_mask |= reg_mask;
          }
          else {
            // Virtual base id - fill the whole mask for implicit mem validation. The register is not assigned
            // yet, so we cannot predict the phys id.
            if (uint32_t(validation_flags & ValidationFlags::kEnableVirtRegs) == 0) {
              return make_error(Error::kIllegalVirtReg);
            }
            reg_mask = 0xFFFFFFFFu;
          }

          if (index_type == RegType::kNone && !m.offset_lo32()) {
            op_flags |= InstDB::OpFlags::kFlagMemBase;
          }
        }
        else if (base_type == RegType::kLabelTag) {
          // [Label] - there is no need to validate the base as it's label.
        }
        else {
          // Base is a 64-bit address.
          int64_t offset = m.offset();
          if (!Support::is_int_n<32>(offset)) {
            if (mode == InstDB::Mode::kX86) {
              // 32-bit mode: Make sure that the address is either `int32_t` or `uint32_t`.
              if (!Support::is_uint_n<32>(offset)) {
                return make_error(Error::kInvalidAddress64Bit);
              }
            }
            else {
              // 64-bit mode: Zero extension is allowed if the address has 32-bit index register or the address
              // has no index register (it's still encodable).
              if (index_type != RegType::kNone) {
                if (!Support::is_uint_n<32>(offset)) {
                  return make_error(Error::kInvalidAddress64Bit);
                }

                if (index_type != RegType::kGp32) {
                  return make_error(Error::kInvalidAddress64BitZeroExtension);
                }
              }
              else {
                // We don't validate absolute 64-bit addresses without an index register as this also depends
                // on the target's base address. We don't have the information to do it at this moment.
              }
            }
          }
        }

        if (index_type != RegType::kNone) {
          if (ASMJIT_UNLIKELY(!Support::bit_test(vd->allowed_mem_index_regs, index_type))) {
            return make_error(Error::kInvalidAddress);
          }

          if (index_type == RegType::kVec128) {
            op_flags |= InstDB::OpFlags::kVm32x | InstDB::OpFlags::kVm64x;
          }
          else if (index_type == RegType::kVec256) {
            op_flags |= InstDB::OpFlags::kVm32y | InstDB::OpFlags::kVm64y;
          }
          else if (index_type == RegType::kVec512) {
            op_flags |= InstDB::OpFlags::kVm32z | InstDB::OpFlags::kVm64z;
          }
          else {
            if (base_type != RegType::kNone)
              op_flags |= InstDB::OpFlags::kFlagMib;
          }

          // [RIP + {XMM|YMM|ZMM}] is not allowed.
          if (base_type == RegType::kPC && Support::test(op_flags, InstDB::OpFlags::kVmMask)) {
            return make_error(Error::kInvalidAddress);
          }

          uint32_t index_id = m.index_id();
          if (index_id < Operand::kVirtIdMin) {
            if (ASMJIT_UNLIKELY(index_id >= 32)) {
              return make_error(Error::kInvalidPhysId);
            }

            combined_reg_mask |= Support::bit_mask<RegMask>(index_id);
          }
          else if (uint32_t(validation_flags & ValidationFlags::kEnableVirtRegs) == 0) {
            return make_error(Error::kIllegalVirtReg);
          }

          // Only used for implicit memory operands having 'seg:[reg]' form, so clear it.
          reg_mask = 0;
        }

        switch (mem_size) {
          case  0: op_flags |= InstDB::OpFlags::kMemUnspecified; break;
          case  1: op_flags |= InstDB::OpFlags::kMem8; break;
          case  2: op_flags |= InstDB::OpFlags::kMem16; break;
          case  4: op_flags |= InstDB::OpFlags::kMem32; break;
          case  6: op_flags |= InstDB::OpFlags::kMem48; break;
          case  8: op_flags |= InstDB::OpFlags::kMem64; break;
          case 10: op_flags |= InstDB::OpFlags::kMem80; break;
          case 16: op_flags |= InstDB::OpFlags::kMem128; break;
          case 32: op_flags |= InstDB::OpFlags::kMem256; break;
          case 64: op_flags |= InstDB::OpFlags::kMem512; break;

          default:
            return make_error(Error::kInvalidOperandSize);
        }

        break;
      }

      case OperandType::kImm: {
        uint64_t imm_value = op.as<Imm>().value_as<uint64_t>();

        if (int64_t(imm_value) >= 0) {
          if (imm_value <= 0x7u) {
            op_flags = InstDB::OpFlags::kImmI64 | InstDB::OpFlags::kImmU64 | InstDB::OpFlags::kImmI32 | InstDB::OpFlags::kImmU32 |
                       InstDB::OpFlags::kImmI16 | InstDB::OpFlags::kImmU16 | InstDB::OpFlags::kImmI8  | InstDB::OpFlags::kImmU8  |
                       InstDB::OpFlags::kImmI4  | InstDB::OpFlags::kImmU4  ;
          }
          else if (imm_value <= 0xFu) {
            op_flags = InstDB::OpFlags::kImmI64 | InstDB::OpFlags::kImmU64 | InstDB::OpFlags::kImmI32 | InstDB::OpFlags::kImmU32 |
                       InstDB::OpFlags::kImmI16 | InstDB::OpFlags::kImmU16 | InstDB::OpFlags::kImmI8  | InstDB::OpFlags::kImmU8  |
                       InstDB::OpFlags::kImmU4  ;
          }
          else if (imm_value <= 0x7Fu) {
            op_flags = InstDB::OpFlags::kImmI64 | InstDB::OpFlags::kImmU64 | InstDB::OpFlags::kImmI32 | InstDB::OpFlags::kImmU32 |
                       InstDB::OpFlags::kImmI16 | InstDB::OpFlags::kImmU16 | InstDB::OpFlags::kImmI8  | InstDB::OpFlags::kImmU8  ;
          }
          else if (imm_value <= 0xFFu) {
            op_flags = InstDB::OpFlags::kImmI64 | InstDB::OpFlags::kImmU64 | InstDB::OpFlags::kImmI32 | InstDB::OpFlags::kImmU32 |
                       InstDB::OpFlags::kImmI16 | InstDB::OpFlags::kImmU16 | InstDB::OpFlags::kImmU8  ;
          }
          else if (imm_value <= 0x7FFFu) {
            op_flags = InstDB::OpFlags::kImmI64 | InstDB::OpFlags::kImmU64 | InstDB::OpFlags::kImmI32 | InstDB::OpFlags::kImmU32 |
                       InstDB::OpFlags::kImmI16 | InstDB::OpFlags::kImmU16 ;
          }
          else if (imm_value <= 0xFFFFu) {
            op_flags = InstDB::OpFlags::kImmI64 | InstDB::OpFlags::kImmU64 | InstDB::OpFlags::kImmI32 | InstDB::OpFlags::kImmU32 |
                       InstDB::OpFlags::kImmU16 ;
          }
          else if (imm_value <= 0x7FFFFFFFu) {
            op_flags = InstDB::OpFlags::kImmI64 | InstDB::OpFlags::kImmU64 | InstDB::OpFlags::kImmI32 | InstDB::OpFlags::kImmU32;
          }
          else if (imm_value <= 0xFFFFFFFFu) {
            op_flags = InstDB::OpFlags::kImmI64 | InstDB::OpFlags::kImmU64 | InstDB::OpFlags::kImmU32;
          }
          else if (imm_value <= 0x7FFFFFFFFFFFFFFFu) {
            op_flags = InstDB::OpFlags::kImmI64 | InstDB::OpFlags::kImmU64;
          }
          else {
            op_flags = InstDB::OpFlags::kImmU64;
          }
        }
        else {
          imm_value = Support::neg(imm_value);
          if (imm_value <= 0x8u) {
            op_flags = InstDB::OpFlags::kImmI64 | InstDB::OpFlags::kImmI32 | InstDB::OpFlags::kImmI16 | InstDB::OpFlags::kImmI8 | InstDB::OpFlags::kImmI4;
          }
          else if (imm_value <= 0x80u) {
            op_flags = InstDB::OpFlags::kImmI64 | InstDB::OpFlags::kImmI32 | InstDB::OpFlags::kImmI16 | InstDB::OpFlags::kImmI8;
          }
          else if (imm_value <= 0x8000u) {
            op_flags = InstDB::OpFlags::kImmI64 | InstDB::OpFlags::kImmI32 | InstDB::OpFlags::kImmI16;
          }
          else if (imm_value <= 0x80000000u) {
            op_flags = InstDB::OpFlags::kImmI64 | InstDB::OpFlags::kImmI32;
          }
          else {
            op_flags = InstDB::OpFlags::kImmI64;
          }
        }
        break;
      }

      case OperandType::kLabel: {
        op_flags |= InstDB::OpFlags::kRel8 | InstDB::OpFlags::kRel32;
        break;
      }

      default:
        return make_error(Error::kInvalidState);
    }

    InstDB::OpSignature& op_sig_dst = op_sig_translated[i];
    op_sig_dst._flags = uint64_t(op_flags) & 0x00FFFFFFFFFFFFFFu;
    op_sig_dst._reg_mask = uint8_t(reg_mask & 0xFFu);
    combined_op_flags |= op_flags;
  }

  // Decrease the number of operands of those that are none. This is important as Assembler and Compiler may just pass
  // more operands padded with none (which means that no operand is given at that index). However, validate that there
  // are no gaps (like [reg, none, reg] or [none, reg]).
  if (i < op_count) {
    while (--op_count > i) {
      if (ASMJIT_UNLIKELY(!operands[op_count].is_none())) {
        return make_error(Error::kInvalidInstruction);
      }
    }
  }

  // Validate X86 and X64 specific cases.
  if (mode == InstDB::Mode::kX86) {
    // Illegal use of 64-bit register in 32-bit mode.
    if (ASMJIT_UNLIKELY(Support::test(combined_op_flags, InstDB::OpFlags::kRegGpq))) {
      return make_error(Error::kInvalidUseOfGpq);
    }
  }
  else {
    // Illegal use of a high 8-bit register with REX prefix.
    bool has_rex = inst.has_option(InstOptions::kX86_Rex) || (combined_reg_mask & 0xFFFFFF00u) != 0;
    if (ASMJIT_UNLIKELY(has_rex && Support::test(combined_op_flags, InstDB::OpFlags::kRegGpbHi))) {
      return make_error(Error::kInvalidUseOfGpbHi);
    }
  }

  // Validate Instruction Signature by Comparing Against All Signature Records
  // -------------------------------------------------------------------------

  bool inst_signature_matched = false;
  Span<const InstDB::InstSignature> inst_signatures = common_info.inst_signatures();

  if (!inst_signatures.is_empty()) {
    const InstDB::OpSignature* op_signature_table = InstDB::_op_signature_table;

    // If set it means that we matched a signature where only immediate value
    // was out of bounds. We can return a more descriptive error if we know this.
    bool global_imm_out_of_range = false;

    for (const InstDB::InstSignature& inst_signature : inst_signatures) {
      // Only match signatures that are compatible with the requested mode.
      if (!inst_signature.supports_mode(mode)) {
        continue;
      }

      // Compare the operands table with reference operands.
      uint32_t j = 0;
      uint32_t inst_op_count = inst_signature.op_count();
      bool local_imm_out_of_range = false;

      if (inst_op_count == op_count) {
        for (j = 0; j < op_count; j++) {
          if (!check_op_sig(op_sig_translated[j], inst_signature.op_signature(j), local_imm_out_of_range)) {
            break;
          }
        }
      }
      else if (inst_op_count - inst_signature.implicit_op_count() == op_count) {
        uint32_t r = 0;
        for (j = 0; j < op_count && r < inst_op_count; j++, r++) {
          const InstDB::OpSignature* op_chk = op_sig_translated + j;
          const InstDB::OpSignature* op_ref;
Next:
          op_ref = op_signature_table + inst_signature.op_signature_index(r);
          // Skip implicit operands.
          if (op_ref->is_implicit()) {
            if (++r >= inst_op_count) {
              break;
            }
            else {
              goto Next;
            }
          }

          if (!check_op_sig(*op_chk, *op_ref, local_imm_out_of_range)) {
            break;
          }
        }
      }

      if (j == op_count) {
        if (!local_imm_out_of_range) {
          // The match must clear `global_imm_out_of_range` as we have matched all operands.
          global_imm_out_of_range = false;
          inst_signature_matched = true;
          break;
        }
        global_imm_out_of_range = local_imm_out_of_range;
      }
    }

    if (!inst_signature_matched) {
      return make_error(global_imm_out_of_range ? Error::kInvalidImmediate : Error::kInvalidInstruction);
    }
  }

  // Validate AVX512 Options
  // -----------------------

  const RegOnly& extra_reg = inst.extra_reg();

  if (Support::test(options, kAvx512Options)) {
    if (common_info.has_flag(InstDB::InstFlags::kEvex)) {
      // Validate AVX-512 {z}.
      if (Support::test(options, InstOptions::kX86_ZMask)) {
        if (ASMJIT_UNLIKELY(Support::test(options, InstOptions::kX86_ZMask) && !common_info.has_avx512_z())) {
          return make_error(Error::kInvalidKZeroUse);
        }
      }

      // Validate AVX-512 {sae} and {er}.
      if (Support::test(options, InstOptions::kX86_SAE | InstOptions::kX86_ER)) {
        // Rounding control is impossible if the instruction is not reg-to-reg.
        if (ASMJIT_UNLIKELY(mem_op)) {
          return make_error(Error::kInvalidEROrSAE);
        }

        // Check if {sae} or {er} is supported by the instruction.
        if (Support::test(options, InstOptions::kX86_ER)) {
          // NOTE: if both {sae} and {er} are set, we don't care, as {sae} is implied.
          if (ASMJIT_UNLIKELY(!common_info.has_avx512_er())) {
            return make_error(Error::kInvalidEROrSAE);
          }
        }
        else {
          if (ASMJIT_UNLIKELY(!common_info.has_avx512_sae())) {
            return make_error(Error::kInvalidEROrSAE);
          }
        }

        // {sae} and {er} are defined for either scalar ops or vector ops that require LL to be 10 (512-bit vector
        // operations). We don't need any more bits in the instruction database to be able to validate this, as
        // each AVX512 instruction that has broadcast is vector instruction (in this case we require zmm registers),
        // otherwise it's a scalar instruction, which is valid.
        if (common_info.has_avx512_bcst()) {
          // Supports broadcast, thus we require LL to be '10', which means there have to be ZMM registers used. We
          // don't calculate LL here, but we know that it would be '10' if there is at least one ZMM register used.

          // There is no {er}/{sae}-enabled instruction with less than two operands.
          ASMJIT_ASSERT(op_count >= 2);
          if (ASMJIT_UNLIKELY(!is_zmm_or_m512(operands[0]) && !is_zmm_or_m512(operands[1]))) {
            return make_error(Error::kInvalidEROrSAE);
          }
        }
      }
    }
    else {
      // Not an AVX512 instruction - maybe OpExtra is xCX register used by REP/REPNE prefix.
      if (Support::test(options, kAvx512Options) || !Support::test(options, kRepAny)) {
        return make_error(Error::kInvalidInstruction);
      }
    }
  }

  // Validate {Extra} Register
  // -------------------------

  if (extra_reg.is_reg()) {
    if (Support::test(options, kRepAny)) {
      // Validate REP|REPNE {cx|ecx|rcx}.
      if (ASMJIT_UNLIKELY(Support::test(inst_flags, InstDB::InstFlags::kRepIgnored))) {
        return make_error(Error::kInvalidExtraReg);
      }

      if (extra_reg.is_phys_reg()) {
        if (ASMJIT_UNLIKELY(extra_reg.id() != Gp::kIdCx)) {
          return make_error(Error::kInvalidExtraReg);
        }
      }

      // The type of the {...} register must match the type of the base register
      // of memory operand. So if the memory operand uses 32-bit register the
      // count register must also be 32-bit, etc...
      if (ASMJIT_UNLIKELY(!mem_op || extra_reg.type() != mem_op->base_type())) {
        return make_error(Error::kInvalidExtraReg);
      }
    }
    else if (common_info.has_flag(InstDB::InstFlags::kEvex)) {
      // Validate AVX-512 {k}.
      if (ASMJIT_UNLIKELY(extra_reg.type() != RegType::kMask)) {
        return make_error(Error::kInvalidExtraReg);
      }

      if (ASMJIT_UNLIKELY(extra_reg.id() == 0 || !common_info.has_avx512_k())) {
        return make_error(Error::kInvalidKMaskUse);
      }
    }
    else {
      return make_error(Error::kInvalidExtraReg);
    }
  }

  return Error::kOk;
}

Error validate_x86(const BaseInst& inst, const Operand_* operands, size_t op_count, ValidationFlags validation_flags) noexcept {
  return validate(InstDB::Mode::kX86, inst, operands, op_count, validation_flags);
}

Error validate_x64(const BaseInst& inst, const Operand_* operands, size_t op_count, ValidationFlags validation_flags) noexcept {
  return validate(InstDB::Mode::kX64, inst, operands, op_count, validation_flags);
}

#endif // !ASMJIT_NO_INTROSPECTION

// x86::InstInternal - QueryRWInfo
// ===============================

#ifndef ASMJIT_NO_INTROSPECTION
static const Support::Array<uint64_t, uint32_t(RegGroup::kMaxValue) + 1> rw_reg_group_byte_mask_table = {{
  0x00000000000000FFu, // GP.
  0xFFFFFFFFFFFFFFFFu, // XMM|YMM|ZMM.
  0x00000000000000FFu, // MM.
  0x00000000000000FFu, // KReg.
  0x0000000000000003u, // SReg.
  0x00000000000000FFu, // CReg.
  0x00000000000000FFu, // DReg.
  0x00000000000003FFu, // St().
  0x000000000000FFFFu, // BND.
  0x00000000000000FFu  // RIP.
}};

static ASMJIT_INLINE void rw_zero_extend_gp(OpRWInfo& op_rw_info, const Gp& reg, uint32_t native_gp_size) noexcept {
  if (reg.size() + 4 == native_gp_size) {
    op_rw_info.add_op_flags(OpRWFlags::kZExt);
    op_rw_info.set_extend_byte_mask(~op_rw_info.write_byte_mask() & 0xFFu);
  }
}

static ASMJIT_INLINE void rw_zero_extend_avx_vec(OpRWInfo& op_rw_info, const Vec& reg) noexcept {
  Support::maybe_unused(reg);

  uint64_t msk = ~Support::fill_trailing_bits(op_rw_info.write_byte_mask());
  if (msk) {
    op_rw_info.add_op_flags(OpRWFlags::kZExt);
    op_rw_info.set_extend_byte_mask(msk);
  }
}

static ASMJIT_INLINE void rw_zero_extend_non_vec(OpRWInfo& op_rw_info, const Reg& reg) noexcept {
  uint64_t msk = ~Support::fill_trailing_bits(op_rw_info.write_byte_mask()) & rw_reg_group_byte_mask_table[reg.reg_group()];
  if (msk) {
    op_rw_info.add_op_flags(OpRWFlags::kZExt);
    op_rw_info.set_extend_byte_mask(msk);
  }
}

static ASMJIT_INLINE Error rw_handle_avx512(const BaseInst& inst, const InstDB::CommonInfo& common_info, InstRWInfo* out) noexcept {
  if (inst.has_extra_reg() && inst.extra_reg().type() == RegType::kMask && out->op_count() > 0) {
    // AVX-512 instruction that uses a destination with {k} register (zeroing vs masking).
    out->_extra_reg.add_op_flags(OpRWFlags::kRead);
    out->_extra_reg.set_read_byte_mask(0xFF);
    if (!inst.has_option(InstOptions::kX86_ZMask) && !common_info.has_avx512_flag(InstDB::Avx512Flags::kImplicitZ)) {
      out->_operands[0].add_op_flags(OpRWFlags::kRead);
      out->_operands[0]._read_byte_mask |= out->_operands[0]._write_byte_mask;
    }
  }

  return Error::kOk;
}

static ASMJIT_INLINE bool has_same_reg_type(const Reg* regs, size_t op_count) noexcept {
  ASMJIT_ASSERT(op_count > 0);
  RegType reg_type = regs[0].reg_type();
  for (size_t i = 1; i < op_count; i++) {
    if (regs[i].reg_type() != reg_type) {
      return false;
    }
  }
  return true;
}

Error query_rw_info(Arch arch, const BaseInst& inst, const Operand_* operands, size_t op_count, InstRWInfo* out) noexcept {
  // Only called when `arch` matches X86 family.
  ASMJIT_ASSERT(Environment::is_family_x86(arch));

  // Get the instruction data.
  InstId inst_id = inst.inst_id();
  if (ASMJIT_UNLIKELY(!Inst::is_defined_id(inst_id))) {
    return make_error(Error::kInvalidInstruction);
  }

  // Read/Write flags.
  const InstDB::InstInfo& inst_info = InstDB::_inst_info_table[inst_id];
  const InstDB::CommonInfo& common_info = InstDB::_inst_common_info_table[inst_info._common_info_index];
  const InstDB::AdditionalInfo& additional_info = InstDB::additional_info_table[inst_info._additional_info_index];
  const InstDB::RWFlagsInfoTable& rw_flags = InstDB::rw_flags_info_table[additional_info._rw_flags_index];

  // There are two data tables, one for `op_count == 2` and the second for
  // `op_count != 2`. There are two reasons for that:
  //   - There are instructions that share the same name that have both 2 or 3 operands, which have different
  //     RW information / semantics.
  //   - There must be 2 tables otherwise the lookup index won't fit into 8 bits (there is more than 256 records
  //     of combined rw_info A and B).
  const InstDB::RWInfo& inst_rw_info = op_count == 2 ? InstDB::rw_info_a_table[InstDB::rw_info_index_a_table[inst_id]]
                                                     : InstDB::rw_info_b_table[InstDB::rw_info_index_b_table[inst_id]];
  const InstDB::RWInfoRm& inst_rm_info = InstDB::rw_info_rm_table[inst_rw_info.rm_info];

  out->_inst_flags = InstDB::inst_flags_table[additional_info._inst_flags_index];
  out->_op_count = uint8_t(op_count);
  out->_rm_feature = inst_rm_info.rm_feature;
  out->_extra_reg.reset();
  out->_read_flags = CpuRWFlags(rw_flags.read_flags);
  out->_write_flags = CpuRWFlags(rw_flags.write_flags);

  uint32_t op_type_mask = 0u;
  uint32_t native_gp_size = Environment::reg_size_of_arch(arch);

  constexpr OpRWFlags R = OpRWFlags::kRead;
  constexpr OpRWFlags W = OpRWFlags::kWrite;
  constexpr OpRWFlags X = OpRWFlags::kRW;
  constexpr OpRWFlags RegM = OpRWFlags::kRegMem;
  constexpr OpRWFlags RegPhys = OpRWFlags::kRegPhysId;
  constexpr OpRWFlags MibRead = OpRWFlags::kMemBaseRead | OpRWFlags::kMemIndexRead;

  if (inst_rw_info.category <= uint32_t(InstDB::RWInfo::kCategoryGenericEx)) {
    uint32_t i;
    uint32_t rm_ops_mask = 0;
    uint32_t rm_max_size = 0;

    for (i = 0; i < op_count; i++) {
      OpRWInfo& op = out->_operands[i];
      const Operand_& src_op = operands[i];
      const InstDB::RWInfoOp& rw_op_data = InstDB::rw_info_op_table[inst_rw_info.op_info_index[i]];

      op_type_mask |= Support::bit_mask<uint32_t>(src_op.op_type());

      if (!src_op.is_reg_or_mem()) {
        op.reset();
        continue;
      }

      op._op_flags = rw_op_data.flags & ~OpRWFlags::kZExt;
      op._phys_id = rw_op_data.phys_id;
      op._rm_size = 0;
      op._reset_reserved();

      uint64_t r_byte_mask = rw_op_data.r_byte_mask;
      uint64_t w_byte_mask = rw_op_data.w_byte_mask;

      if (op.is_read()  && !r_byte_mask) {
        r_byte_mask = Support::lsb_mask<uint64_t>(src_op.x86_rm_size());
      }

      if (op.is_write() && !w_byte_mask) {
        w_byte_mask = Support::lsb_mask<uint64_t>(src_op.x86_rm_size());
      }

      op._read_byte_mask = r_byte_mask;
      op._write_byte_mask = w_byte_mask;
      op._extend_byte_mask = 0;
      op._consecutive_lead_count = rw_op_data.consecutive_lead_count;

      if (src_op.is_reg()) {
        // Zero extension.
        if (op.is_write()) {
          if (src_op.as<Reg>().is_gp()) {
            // GP registers on X64 are special:
            //   - 8-bit and 16-bit writes aren't zero extended.
            //   - 32-bit writes ARE zero extended.
            rw_zero_extend_gp(op, src_op.as<Gp>(), native_gp_size);
          }
          else if (Support::test(rw_op_data.flags, OpRWFlags::kZExt)) {
            // Otherwise follow ZExt.
            rw_zero_extend_non_vec(op, src_op.as<Gp>());
          }
        }

        // Aggregate values required to calculate valid Reg/M info.
        rm_max_size  = Support::max(rm_max_size, src_op.x86_rm_size());
        rm_ops_mask |= Support::bit_mask<uint32_t>(i);
      }
      else {
        const x86::Mem& mem_op = src_op.as<x86::Mem>();
        // The RW flags of BASE+INDEX are either provided by the data, which means
        // that the instruction is border-case, or they are deduced from the operand.
        if (mem_op.has_base_reg() && !op.has_op_flag(OpRWFlags::kMemBaseRW)) {
          op.add_op_flags(OpRWFlags::kMemBaseRead);
        }
        if (mem_op.has_index_reg() && !op.has_op_flag(OpRWFlags::kMemIndexRW)) {
          op.add_op_flags(OpRWFlags::kMemIndexRead);
        }
      }
    }

    // Only keep kMovOp if the instruction is actually register to register move of the same kind.
    if (out->has_inst_flag(InstRWFlags::kMovOp)) {
      if (!(op_count >= 2 && op_type_mask == Support::bit_mask<uint32_t>(OperandType::kReg) && has_same_reg_type(reinterpret_cast<const Reg*>(operands), op_count))) {
        out->_inst_flags &= ~InstRWFlags::kMovOp;
      }
    }

    // Special cases require more logic.
    if (inst_rm_info.flags & (InstDB::RWInfoRm::kFlagMovssMovsd | InstDB::RWInfoRm::kFlagPextrw | InstDB::RWInfoRm::kFlagFeatureIfRMI)) {
      if (inst_rm_info.flags & InstDB::RWInfoRm::kFlagMovssMovsd) {
        if (op_count == 2) {
          if (operands[0].is_reg() && operands[1].is_reg()) {
            // Doesn't zero extend the destination.
            out->_operands[0]._extend_byte_mask = 0;
          }
        }
      }
      else if (inst_rm_info.flags & InstDB::RWInfoRm::kFlagPextrw) {
        if (op_count == 3 && operands[1].is_mm_reg()) {
          out->_rm_feature = 0;
          rm_ops_mask = 0;
        }
      }
      else if (inst_rm_info.flags & InstDB::RWInfoRm::kFlagFeatureIfRMI) {
        if (op_count != 3 || !operands[2].is_imm()) {
          out->_rm_feature = 0;
        }
      }
    }

    rm_ops_mask &= uint32_t(inst_rm_info.rm_ops_mask);
    if (rm_ops_mask && !inst.has_option(InstOptions::kX86_ER)) {
      Support::BitWordIterator<uint32_t> it(rm_ops_mask);
      do {
        i = it.next();

        OpRWInfo& op = out->_operands[i];
        op.add_op_flags(RegM);

        switch (inst_rm_info.category) {
          case InstDB::RWInfoRm::kCategoryFixed:
            op.set_rm_size(inst_rm_info.fixed_size);
            break;
          case InstDB::RWInfoRm::kCategoryConsistent:
            op.set_rm_size(operands[i].x86_rm_size());
            break;
          case InstDB::RWInfoRm::kCategoryHalf:
            op.set_rm_size(rm_max_size / 2u);
            break;
          case InstDB::RWInfoRm::kCategoryQuarter:
            op.set_rm_size(rm_max_size / 4u);
            break;
          case InstDB::RWInfoRm::kCategoryEighth:
            op.set_rm_size(rm_max_size / 8u);
            break;
        }
      } while (it.has_next());
    }

    // Special cases per instruction.
    if (inst_rw_info.category == InstDB::RWInfo::kCategoryGenericEx) {
      switch (inst.inst_id()) {
        case Inst::kIdVpternlogd:
        case Inst::kIdVpternlogq: {
          if (op_count == 4 && operands[3].is_imm()) {
            uint32_t predicate = operands[3].as<Imm>().value_as<uint8_t>();

            if ((predicate >> 4) == (predicate & 0xF)) {
              out->_operands[0].clear_op_flags(OpRWFlags::kRead);
              out->_operands[0].set_read_byte_mask(0);
            }
          }
          break;
        }

        default:
          break;
      }
    }

    return rw_handle_avx512(inst, common_info, out);
  }

  switch (inst_rw_info.category) {
    case InstDB::RWInfo::kCategoryMov: {
      // Special case for 'mov' instruction. Here there are some variants that we have to handle as 'mov' can be
      // used to move between GP, segment, control and debug registers. Moving between GP registers also allow to
      // use memory operand.

      // We will again set the flag if it's actually a move from GP to GP register, otherwise this flag cannot be set.
      out->_inst_flags &= ~InstRWFlags::kMovOp;

      if (op_count == 2) {
        if (operands[0].is_reg() && operands[1].is_reg()) {
          const Reg& o0 = operands[0].as<Reg>();
          const Reg& o1 = operands[1].as<Reg>();

          if (o0.is_gp() && o1.is_gp()) {
            out->_operands[0].reset(W | RegM, operands[0].x86_rm_size());
            out->_operands[1].reset(R | RegM, operands[1].x86_rm_size());

            rw_zero_extend_gp(out->_operands[0], operands[0].as<Gp>(), native_gp_size);
            out->_inst_flags |= InstRWFlags::kMovOp;
            return Error::kOk;
          }

          if (o0.is_gp() && o1.is_segment_reg()) {
            out->_operands[0].reset(W | RegM, native_gp_size);
            out->_operands[0].set_rm_size(2);
            out->_operands[1].reset(R, 2);
            return Error::kOk;
          }

          if (o0.is_segment_reg() && o1.is_gp()) {
            out->_operands[0].reset(W, 2);
            out->_operands[1].reset(R | RegM, 2);
            out->_operands[1].set_rm_size(2);
            return Error::kOk;
          }

          if (o0.is_gp() && (o1.is_control_reg() || o1.is_debug_reg())) {
            out->_operands[0].reset(W, native_gp_size);
            out->_operands[1].reset(R, native_gp_size);
            out->_write_flags = CpuRWFlags::kX86_OF |
                               CpuRWFlags::kX86_SF |
                               CpuRWFlags::kX86_ZF |
                               CpuRWFlags::kX86_AF |
                               CpuRWFlags::kX86_PF |
                               CpuRWFlags::kX86_CF;
            return Error::kOk;
          }

          if ((o0.is_control_reg() || o0.is_debug_reg()) && o1.is_gp()) {
            out->_operands[0].reset(W, native_gp_size);
            out->_operands[1].reset(R, native_gp_size);
            out->_write_flags = CpuRWFlags::kX86_OF |
                               CpuRWFlags::kX86_SF |
                               CpuRWFlags::kX86_ZF |
                               CpuRWFlags::kX86_AF |
                               CpuRWFlags::kX86_PF |
                               CpuRWFlags::kX86_CF;
            return Error::kOk;
          }
        }

        if (operands[0].is_reg() && operands[1].is_mem()) {
          const Reg& o0 = operands[0].as<Reg>();
          const Mem& o1 = operands[1].as<Mem>();

          if (o0.is_gp()) {
            if (!o1.is_offset_64bit()) {
              out->_operands[0].reset(W, o0.size());
            }
            else {
              out->_operands[0].reset(W | RegPhys, o0.size(), Gp::kIdAx);
            }

            out->_operands[1].reset(R | MibRead, o0.size());
            rw_zero_extend_gp(out->_operands[0], operands[0].as<Gp>(), native_gp_size);
            return Error::kOk;
          }

          if (o0.is_segment_reg()) {
            out->_operands[0].reset(W, 2);
            out->_operands[1].reset(R, 2);
            return Error::kOk;
          }
        }

        if (operands[0].is_mem() && operands[1].is_reg()) {
          const Mem& o0 = operands[0].as<Mem>();
          const Reg& o1 = operands[1].as<Reg>();

          if (o1.is_gp()) {
            out->_operands[0].reset(W | MibRead, o1.size());
            if (!o0.is_offset_64bit()) {
              out->_operands[1].reset(R, o1.size());
            }
            else {
              out->_operands[1].reset(R | RegPhys, o1.size(), Gp::kIdAx);
            }
            return Error::kOk;
          }

          if (o1.is_segment_reg()) {
            out->_operands[0].reset(W | MibRead, 2);
            out->_operands[1].reset(R, 2);
            return Error::kOk;
          }
        }

        if (operands[0].is_gp() && operands[1].is_imm()) {
          const Reg& o0 = operands[0].as<Reg>();
          out->_operands[0].reset(W | RegM, o0.size());
          out->_operands[1].reset();

          rw_zero_extend_gp(out->_operands[0], operands[0].as<Gp>(), native_gp_size);
          return Error::kOk;
        }

        if (operands[0].is_mem() && operands[1].is_imm()) {
          const Reg& o0 = operands[0].as<Reg>();
          out->_operands[0].reset(W | MibRead, o0.size());
          out->_operands[1].reset();
          return Error::kOk;
        }
      }
      break;
    }

    case InstDB::RWInfo::kCategoryMovabs: {
      if (op_count == 2) {
        if (operands[0].is_gp() && operands[1].is_mem()) {
          const Reg& o0 = operands[0].as<Reg>();
          out->_operands[0].reset(W | RegPhys, o0.size(), Gp::kIdAx);
          out->_operands[1].reset(R | MibRead, o0.size());
          rw_zero_extend_gp(out->_operands[0], operands[0].as<Gp>(), native_gp_size);
          return Error::kOk;
        }

        if (operands[0].is_mem() && operands[1].is_gp()) {
          const Reg& o1 = operands[1].as<Reg>();
          out->_operands[0].reset(W | MibRead, o1.size());
          out->_operands[1].reset(R | RegPhys, o1.size(), Gp::kIdAx);
          return Error::kOk;
        }

        if (operands[0].is_gp() && operands[1].is_imm()) {
          const Reg& o0 = operands[0].as<Reg>();
          out->_operands[0].reset(W, o0.size());
          out->_operands[1].reset();

          rw_zero_extend_gp(out->_operands[0], operands[0].as<Gp>(), native_gp_size);
          return Error::kOk;
        }
      }
      break;
    }

    case InstDB::RWInfo::kCategoryImul: {
      // Special case for 'imul' instruction.
      //
      // There are 3 variants in general:
      //
      //   1. Standard multiplication: 'A = A * B'.
      //   2. Multiplication with imm: 'A = B * C'.
      //   3. Extended multiplication: 'A:B = B * C'.

      if (op_count == 2) {
        if (operands[0].is_reg() && operands[1].is_imm()) {
          out->_operands[0].reset(X, operands[0].as<Reg>().size());
          out->_operands[1].reset();

          rw_zero_extend_gp(out->_operands[0], operands[0].as<Gp>(), native_gp_size);
          return Error::kOk;
        }

        if (operands[0].is_gp16() && operands[1].x86_rm_size() == 1) {
          // imul ax, r8/m8 <- AX = AL * r8/m8
          out->_operands[0].reset(X | RegPhys, 2, Gp::kIdAx);
          out->_operands[0].set_read_byte_mask(Support::lsb_mask<uint64_t>(1));
          out->_operands[1].reset(R | RegM, 1);
        }
        else {
          // imul r?, r?/m?
          out->_operands[0].reset(X, operands[0].as<Gp>().size());
          out->_operands[1].reset(R | RegM, operands[0].as<Gp>().size());
          rw_zero_extend_gp(out->_operands[0], operands[0].as<Gp>(), native_gp_size);
        }

        if (operands[1].is_mem()) {
          out->_operands[1].add_op_flags(MibRead);
        }
        return Error::kOk;
      }

      if (op_count == 3) {
        if (operands[2].is_imm()) {
          out->_operands[0].reset(W, operands[0].x86_rm_size());
          out->_operands[1].reset(R | RegM, operands[1].x86_rm_size());
          out->_operands[2].reset();

          rw_zero_extend_gp(out->_operands[0], operands[0].as<Gp>(), native_gp_size);
          if (operands[1].is_mem()) {
            out->_operands[1].add_op_flags(MibRead);
          }
          return Error::kOk;
        }
        else {
          out->_operands[0].reset(W | RegPhys, operands[0].x86_rm_size(), Gp::kIdDx);
          out->_operands[1].reset(X | RegPhys, operands[1].x86_rm_size(), Gp::kIdAx);
          out->_operands[2].reset(R | RegM, operands[2].x86_rm_size());

          rw_zero_extend_gp(out->_operands[0], operands[0].as<Gp>(), native_gp_size);
          rw_zero_extend_gp(out->_operands[1], operands[1].as<Gp>(), native_gp_size);
          if (operands[2].is_mem()) {
            out->_operands[2].add_op_flags(MibRead);
          }
          return Error::kOk;
        }
      }
      break;
    }

    case InstDB::RWInfo::kCategoryMovh64: {
      // Special case for 'movhpd|movhps' instructions. Note that this is only required for legacy (non-AVX)
      // variants as AVX instructions use either 2 or 3 operands that are in `kCategoryGeneric` category.
      if (op_count == 2) {
        if (operands[0].is_vec() && operands[1].is_mem()) {
          out->_operands[0].reset(W, 8);
          out->_operands[0].set_write_byte_mask(Support::lsb_mask<uint64_t>(8) << 8);
          out->_operands[1].reset(R | MibRead, 8);
          return Error::kOk;
        }

        if (operands[0].is_mem() && operands[1].is_vec()) {
          out->_operands[0].reset(W | MibRead, 8);
          out->_operands[1].reset(R, 8);
          out->_operands[1].set_read_byte_mask(Support::lsb_mask<uint64_t>(8) << 8);
          return Error::kOk;
        }
      }
      break;
    }

    case InstDB::RWInfo::kCategoryPunpcklxx: {
      // Special case for 'punpcklbw|punpckldq|punpcklwd' instructions.
      if (op_count == 2) {
        if (operands[0].is_vec128()) {
          out->_operands[0].reset(X, 16);
          out->_operands[0].set_read_byte_mask(0x0F0Fu);
          out->_operands[0].set_write_byte_mask(0xFFFFu);
          out->_operands[1].reset(R, 16);
          out->_operands[1].set_write_byte_mask(0x0F0Fu);

          if (operands[1].is_vec128()) {
            return Error::kOk;
          }

          if (operands[1].is_mem()) {
            out->_operands[1].add_op_flags(MibRead);
            return Error::kOk;
          }
        }

        if (operands[0].is_mm_reg()) {
          out->_operands[0].reset(X, 8);
          out->_operands[0].set_read_byte_mask(0x0Fu);
          out->_operands[0].set_write_byte_mask(0xFFu);
          out->_operands[1].reset(R, 4);
          out->_operands[1].set_read_byte_mask(0x0Fu);

          if (operands[1].is_mm_reg()) {
            return Error::kOk;
          }

          if (operands[1].is_mem()) {
            out->_operands[1].add_op_flags(MibRead);
            return Error::kOk;
          }
        }
      }
      break;
    }

    case InstDB::RWInfo::kCategoryVmaskmov: {
      // Special case for 'vmaskmovpd|vmaskmovps|vpmaskmovd|vpmaskmovq' instructions.
      if (op_count == 3) {
        if (operands[0].is_vec() && operands[1].is_vec() && operands[2].is_mem()) {
          out->_operands[0].reset(W, operands[0].x86_rm_size());
          out->_operands[1].reset(R, operands[1].x86_rm_size());
          out->_operands[2].reset(R | MibRead, operands[1].x86_rm_size());

          rw_zero_extend_avx_vec(out->_operands[0], operands[0].as<Vec>());
          return Error::kOk;
        }

        if (operands[0].is_mem() && operands[1].is_vec() && operands[2].is_vec()) {
          out->_operands[0].reset(X | MibRead, operands[1].x86_rm_size());
          out->_operands[1].reset(R, operands[1].x86_rm_size());
          out->_operands[2].reset(R, operands[2].x86_rm_size());
          return Error::kOk;
        }
      }
      break;
    }

    case InstDB::RWInfo::kCategoryVmovddup: {
      // Special case for 'vmovddup' instruction. This instruction has an interesting semantic as 128-bit XMM
      // version only uses 64-bit memory operand (m64), however, 256/512-bit versions use 256/512-bit memory
      // operand, respectively.
      if (op_count == 2) {
        if (operands[0].is_vec() && operands[1].is_vec()) {
          uint32_t o0_size = operands[0].x86_rm_size();
          uint32_t o1_size = o0_size == 16 ? 8 : o0_size;

          out->_operands[0].reset(W, o0_size);
          out->_operands[1].reset(R | RegM, o1_size);
          out->_operands[1]._read_byte_mask &= 0x00FF00FF00FF00FFu;

          rw_zero_extend_avx_vec(out->_operands[0], operands[0].as<Vec>());
          return rw_handle_avx512(inst, common_info, out);
        }

        if (operands[0].is_vec() && operands[1].is_mem()) {
          uint32_t o0_size = operands[0].x86_rm_size();
          uint32_t o1_size = o0_size == 16 ? 8 : o0_size;

          out->_operands[0].reset(W, o0_size);
          out->_operands[1].reset(R | MibRead, o1_size);

          rw_zero_extend_avx_vec(out->_operands[0], operands[0].as<Vec>());
          return rw_handle_avx512(inst, common_info, out);
        }
      }
      break;
    }

    case InstDB::RWInfo::kCategoryVmovmskpd:
    case InstDB::RWInfo::kCategoryVmovmskps: {
      // Special case for 'vmovmskpd|vmovmskps' instructions.
      if (op_count == 2) {
        if (operands[0].is_gp() && operands[1].is_vec()) {
          out->_operands[0].reset(W, 1);
          out->_operands[0].set_extend_byte_mask(Support::lsb_mask<uint32_t>(native_gp_size - 1) << 1);
          out->_operands[1].reset(R, operands[1].x86_rm_size());
          return Error::kOk;
        }
      }
      break;
    }

    case InstDB::RWInfo::kCategoryVmov1_2:
    case InstDB::RWInfo::kCategoryVmov1_4:
    case InstDB::RWInfo::kCategoryVmov1_8: {
      // Special case for instructions where the destination is 1:N (narrowing).
      //
      // Vmov1_2:
      //   vcvtpd2dq|vcvttpd2dq
      //   vcvtpd2udq|vcvttpd2udq
      //   vcvtpd2ps|vcvtps2ph
      //   vcvtqq2ps|vcvtuqq2ps
      //   vpmovwb|vpmovswb|vpmovuswb
      //   vpmovdw|vpmovsdw|vpmovusdw
      //   vpmovqd|vpmovsqd|vpmovusqd
      //
      // Vmov1_4:
      //   vpmovdb|vpmovsdb|vpmovusdb
      //   vpmovqw|vpmovsqw|vpmovusqw
      //
      // Vmov1_8:
      //   pmovmskb|vpmovmskb
      //   vpmovqb|vpmovsqb|vpmovusqb
      uint32_t shift = inst_rw_info.category - InstDB::RWInfo::kCategoryVmov1_2 + 1;

      if (op_count >= 2) {
        if (op_count >= 3) {
          if (op_count > 3) {
            return make_error(Error::kInvalidInstruction);
          }
          out->_operands[2].reset();
        }

        if (operands[0].is_reg() && operands[1].is_reg()) {
          uint32_t size1 = operands[1].x86_rm_size();
          uint32_t size0 = size1 >> shift;

          out->_operands[0].reset(W, size0);
          out->_operands[1].reset(R, size1);

          if (inst_rm_info.rm_ops_mask & 0x1) {
            out->_operands[0].add_op_flags(RegM);
            out->_operands[0].set_rm_size(size0);
          }

          if (inst_rm_info.rm_ops_mask & 0x2) {
            out->_operands[1].add_op_flags(RegM);
            out->_operands[1].set_rm_size(size1);
          }

          // Handle 'pmovmskb|vpmovmskb'.
          if (operands[0].is_gp()) {
            rw_zero_extend_gp(out->_operands[0], operands[0].as<Gp>(), native_gp_size);
          }

          if (operands[0].is_vec()) {
            rw_zero_extend_avx_vec(out->_operands[0], operands[0].as<Vec>());
          }

          return rw_handle_avx512(inst, common_info, out);
        }

        if (operands[0].is_reg() && operands[1].is_mem()) {
          uint32_t size1 = operands[1].x86_rm_size() ? operands[1].x86_rm_size() : uint32_t(16);
          uint32_t size0 = size1 >> shift;

          out->_operands[0].reset(W, size0);
          out->_operands[1].reset(R | MibRead, size1);

          if (operands[0].is_vec()) {
            rw_zero_extend_avx_vec(out->_operands[0], operands[0].as<Vec>());
          }

          return Error::kOk;
        }

        if (operands[0].is_mem() && operands[1].is_reg()) {
          uint32_t size1 = operands[1].x86_rm_size();
          uint32_t size0 = size1 >> shift;

          out->_operands[0].reset(W | MibRead, size0);
          out->_operands[1].reset(R, size1);

          return rw_handle_avx512(inst, common_info, out);
        }
      }
      break;
    }

    case InstDB::RWInfo::kCategoryVmov2_1:
    case InstDB::RWInfo::kCategoryVmov4_1:
    case InstDB::RWInfo::kCategoryVmov8_1: {
      // Special case for instructions where the destination is N:1 (widening).
      //
      // Vmov2_1:
      //   vcvtdq2pd|vcvtudq2pd
      //   vcvtps2pd|vcvtph2ps
      //   vcvtps2qq|vcvtps2uqq
      //   vcvttps2qq|vcvttps2uqq
      //   vpmovsxbw|vpmovzxbw
      //   vpmovsxwd|vpmovzxwd
      //   vpmovsxdq|vpmovzxdq
      //
      // Vmov4_1:
      //   vpmovsxbd|vpmovzxbd
      //   vpmovsxwq|vpmovzxwq
      //
      // Vmov8_1:
      //   vpmovsxbq|vpmovzxbq
      uint32_t shift = inst_rw_info.category - InstDB::RWInfo::kCategoryVmov2_1 + 1;

      if (op_count >= 2) {
        if (op_count >= 3) {
          if (op_count > 3) {
            return make_error(Error::kInvalidInstruction);
          }
          out->_operands[2].reset();
        }

        uint32_t size0 = operands[0].x86_rm_size();
        uint32_t size1 = size0 >> shift;

        out->_operands[0].reset(W, size0);
        out->_operands[1].reset(R, size1);

        if (operands[0].is_vec()) {
          rw_zero_extend_avx_vec(out->_operands[0], operands[0].as<Vec>());
        }

        if (operands[0].is_reg() && operands[1].is_reg()) {
          if (inst_rm_info.rm_ops_mask & 0x1) {
            out->_operands[0].add_op_flags(RegM);
            out->_operands[0].set_rm_size(size0);
          }

          if (inst_rm_info.rm_ops_mask & 0x2) {
            out->_operands[1].add_op_flags(RegM);
            out->_operands[1].set_rm_size(size1);
          }

          return rw_handle_avx512(inst, common_info, out);
        }

        if (operands[0].is_reg() && operands[1].is_mem()) {
          out->_operands[1].add_op_flags(MibRead);

          return rw_handle_avx512(inst, common_info, out);
        }
      }
      break;
    }
  }

  return make_error(Error::kInvalidInstruction);
}
#endif // !ASMJIT_NO_INTROSPECTION

// x86::InstInternal - QueryFeatures
// =================================

#ifndef ASMJIT_NO_INTROSPECTION
struct RegAnalysis {
  uint32_t reg_type_mask;
  uint32_t high_vec_used;

  inline bool has_reg_type(RegType reg_type) const noexcept {
    return Support::bit_test(reg_type_mask, reg_type);
  }
};

static RegAnalysis InstInternal_reg_analysis(const Operand_* operands, size_t op_count) noexcept {
  uint32_t mask = 0;
  uint32_t high_vec_used = 0;

  for (uint32_t i = 0; i < op_count; i++) {
    const Operand_& op = operands[i];
    if (op.is_reg()) {
      const Reg& reg = op.as<Reg>();
      mask |= Support::bit_mask<uint32_t>(reg.reg_type());
      if (reg.is_vec()) {
        high_vec_used |= uint32_t(reg.id() >= 16 && reg.id() < 32);
      }
    }
    else if (op.is_mem()) {
      const BaseMem& mem = op.as<BaseMem>();
      if (mem.has_base_reg()) {
        mask |= Support::bit_mask<uint32_t>(mem.base_type());
      }
      if (mem.has_index_reg()) {
        mask |= Support::bit_mask<uint32_t>(mem.index_type());
        high_vec_used |= uint32_t(mem.index_id() >= 16 && mem.index_id() < 32);
      }
    }
  }

  return RegAnalysis { mask, high_vec_used };
}

static inline uint32_t InstInternal_usesAvx512(InstOptions inst_options, const RegOnly& extra_reg, const RegAnalysis& reg_analysis) noexcept {
  uint32_t has_evex = uint32_t(inst_options & (InstOptions::kX86_Evex | InstOptions::kX86_AVX512Mask));
  uint32_t has_kmask = extra_reg.type() == RegType::kMask;
  uint32_t has_k_or_zmm = reg_analysis.reg_type_mask & Support::bit_mask<uint32_t>(RegType::kVec512, RegType::kMask);

  return has_evex | has_kmask | has_k_or_zmm;
}

Error query_features(Arch arch, const BaseInst& inst, const Operand_* operands, size_t op_count, CpuFeatures* out) noexcept {
  using Ext = CpuFeatures::X86;

  // Only called when `arch` matches X86 family.
  Support::maybe_unused(arch);
  ASMJIT_ASSERT(Environment::is_family_x86(arch));

  // Get the instruction data.
  InstId inst_id = inst.inst_id();
  InstOptions options = inst.options();

  if (ASMJIT_UNLIKELY(!Inst::is_defined_id(inst_id))) {
    return make_error(Error::kInvalidInstruction);
  }

  const InstDB::InstInfo& inst_info = InstDB::inst_info_by_id(inst_id);
  const InstDB::AdditionalInfo& additional_info = InstDB::additional_info_table[inst_info._additional_info_index];

  const uint8_t* feature_data = additional_info.features_begin();
  const uint8_t* feature_data_end = additional_info.features_end();

  // Copy all features to `out`.
  out->reset();
  do {
    uint32_t feature = feature_data[0];
    if (!feature) {
      break;
    }
    out->add(feature);
  } while (++feature_data != feature_data_end);

  // Since AsmJit aggregates instructions that share the same name we have to
  // deal with some special cases and also with MMX/SSE and AVX/AVX2 overlaps.
  if (feature_data != additional_info.features_begin()) {
    RegAnalysis reg_analysis = InstInternal_reg_analysis(operands, op_count);

    // Handle MMX vs SSE overlap.
    if (out->has(Ext::kMMX) || out->has(Ext::kMMX2)) {
      // Only instructions defined by SSE and SSE2 overlap. Instructions introduced by newer instruction sets like
      // SSE3+ don't state MMX as they require SSE3+.
      if (out->has(Ext::kSSE) || out->has(Ext::kSSE2)) {
        if (!reg_analysis.has_reg_type(RegType::kVec128)) {
          // The instruction doesn't use XMM register(s), thus it's MMX/MMX2 only.
          out->remove(Ext::kSSE);
          out->remove(Ext::kSSE2);
          out->remove(Ext::kSSE4_1);
        }
        else {
          out->remove(Ext::kMMX);
          out->remove(Ext::kMMX2);
        }

        // Special case: PEXTRW instruction is MMX/SSE2 instruction. However, MMX/SSE version cannot access memory
        // (only register to register extract) so when SSE4.1 introduced the whole family of PEXTR/PINSR instructions
        // they also introduced PEXTRW with a new opcode 0x15 that can extract directly to memory. This instruction
        // is, of course, not compatible with MMX/SSE2 and would #UD if SSE4.1 is not supported.
        if (inst_id == Inst::kIdPextrw) {
          if (op_count >= 1 && operands[0].is_mem())
            out->remove(Ext::kSSE2);
          else
            out->remove(Ext::kSSE4_1);
        }
      }
    }

    // Handle PCLMULQDQ vs VPCLMULQDQ.
    if (out->has(Ext::kVPCLMULQDQ)) {
      if (reg_analysis.has_reg_type(RegType::kVec512) || Support::test(options, InstOptions::kX86_Evex)) {
        // AVX512_F & VPCLMULQDQ.
        out->remove(Ext::kAVX, Ext::kPCLMULQDQ);
      }
      else if (reg_analysis.has_reg_type(RegType::kVec256)) {
        out->remove(Ext::kAVX512_F, Ext::kAVX512_VL);
      }
      else {
        // AVX & PCLMULQDQ.
        out->remove(Ext::kAVX512_F, Ext::kAVX512_VL, Ext::kVPCLMULQDQ);
      }
    }

    // Handle AVX vs AVX2 overlap.
    if (out->has(Ext::kAVX) && out->has(Ext::kAVX2)) {
      bool is_avx2 = true;
      // Special case: VBROADCASTSS and VBROADCASTSD were introduced in AVX, but only version that uses memory as a
      // source operand. AVX2 then added support for register source operand.
      if (inst_id == Inst::kIdVbroadcastss || inst_id == Inst::kIdVbroadcastsd) {
        if (op_count > 1 && operands[1].is_mem()) {
          is_avx2 = false;
        }
      }
      else {
        // AVX instruction set doesn't support integer operations on YMM registers as these were later introcuced by
        // AVX2. In our case we have to check if YMM register(s) are in use and if that is the case this is an AVX2
        // instruction.
        if (!(reg_analysis.reg_type_mask & Support::bit_mask<uint32_t>(RegType::kVec256, RegType::kVec512))) {
          is_avx2 = false;
        }
      }
      out->remove(is_avx2 ? Ext::kAVX : Ext::kAVX2);
    }

    // Handle AVX vs AVX512 overlap.
    //
    // In general, non-AVX encoding is preferred, however, AVX encoded instructions that were initially provided
    // as AVX-512 instructions must naturally prefer AVX-512 encoding, as that was the first one provided.
    if (out->has_any(Ext::kAVX,
                    Ext::kAVX_IFMA,
                    Ext::kAVX_NE_CONVERT,
                    Ext::kAVX_VNNI,
                    Ext::kAVX2,
                    Ext::kF16C,
                    Ext::kFMA) &&
        out->has_any(Ext::kAVX512_BF16,
                    Ext::kAVX512_BW,
                    Ext::kAVX512_DQ,
                    Ext::kAVX512_F,
                    Ext::kAVX512_IFMA,
                    Ext::kAVX512_VNNI)) {
      uint32_t use_evex = InstInternal_usesAvx512(options, inst.extra_reg(), reg_analysis) | reg_analysis.high_vec_used;
      switch (inst_id) {
        // Special case: VPBROADCAST[B|D|Q|W] only supports r32/r64 with EVEX prefix.
        case Inst::kIdVpbroadcastb:
        case Inst::kIdVpbroadcastd:
        case Inst::kIdVpbroadcastq:
        case Inst::kIdVpbroadcastw:
          use_evex |= uint32_t(op_count >= 2 && operands[1].is_gp());
          break;

        case Inst::kIdVcvtpd2dq:
        case Inst::kIdVcvtpd2ps:
        case Inst::kIdVcvttpd2dq:
          use_evex |= uint32_t(op_count >= 2 && operands[0].is_vec256());
          break;

        case Inst::kIdVgatherdpd:
        case Inst::kIdVgatherdps:
        case Inst::kIdVgatherqpd:
        case Inst::kIdVgatherqps:
        case Inst::kIdVpgatherdd:
        case Inst::kIdVpgatherdq:
        case Inst::kIdVpgatherqd:
        case Inst::kIdVpgatherqq:
          use_evex |= uint32_t(op_count == 2);
          break;

        // Special case: These instructions only allow `reg, reg. imm` combination in AVX|AVX2 mode, then
        // AVX-512 introduced `reg, reg/mem, imm` combination that uses EVEX prefix. This means that if
        // the second operand is memory then this is AVX-512_BW instruction and not AVX/AVX2 instruction.
        case Inst::kIdVpslldq:
        case Inst::kIdVpslld:
        case Inst::kIdVpsllq:
        case Inst::kIdVpsllw:
        case Inst::kIdVpsrad:
        case Inst::kIdVpsraq:
        case Inst::kIdVpsraw:
        case Inst::kIdVpsrld:
        case Inst::kIdVpsrldq:
        case Inst::kIdVpsrlq:
        case Inst::kIdVpsrlw:
          use_evex |= uint32_t(op_count >= 2 && operands[1].is_mem());
          break;

        // Special case: VPERMPD - AVX2 vs AVX512-F case.
        case Inst::kIdVpermpd:
          use_evex |= uint32_t(op_count >= 3 && !operands[2].is_imm());
          break;

        // Special case: VPERMQ - AVX2 vs AVX512-F case.
        case Inst::kIdVpermq:
          use_evex |= uint32_t(op_count >= 3 && (operands[1].is_mem() || !operands[2].is_imm()));
          break;
      }

      if (inst_info.common_info().prefer_evex() && !Support::test(options, InstOptions::kX86_Vex | InstOptions::kX86_Vex3))
        use_evex = 1;

      if (use_evex) {
        out->remove(Ext::kAVX,
                    Ext::kAVX_IFMA,
                    Ext::kAVX_NE_CONVERT,
                    Ext::kAVX_VNNI,
                    Ext::kAVX2,
                    Ext::kF16C,
                    Ext::kFMA);
      }
      else {
        out->remove(Ext::kAVX512_BF16,
                    Ext::kAVX512_BW,
                    Ext::kAVX512_DQ,
                    Ext::kAVX512_F,
                    Ext::kAVX512_IFMA,
                    Ext::kAVX512_VL,
                    Ext::kAVX512_VNNI);
      }
    }

    // Clear AVX512_VL if ZMM register is used.
    if (reg_analysis.has_reg_type(RegType::kVec512)) {
      out->remove(Ext::kAVX512_VL);
    }
  }

  return Error::kOk;
}
#endif // !ASMJIT_NO_INTROSPECTION

} // {InstInternal}

// x86::InstInternal - Tests
// =========================

#if defined(ASMJIT_TEST)
#ifndef ASMJIT_NO_TEXT
UNIT(x86_inst_api_text) {
  // All known instructions should be matched.
  INFO("Matching all X86 instructions");
  for (uint32_t a = 1; a < Inst::_kIdCount; a++) {
    StringTmp<128> a_name;
    EXPECT_EQ(InstInternal::inst_id_to_string(a, InstStringifyOptions::kNone, a_name), Error::kOk)
      .message("Failed to get the name of instruction #%u", a);

    uint32_t b = InstInternal::string_to_inst_id(a_name.data(), a_name.size());
    StringTmp<128> b_name;
    InstInternal::inst_id_to_string(b, InstStringifyOptions::kNone, b_name);
    EXPECT_EQ(a, b)
      .message("Instructions do not match \"%s\" (#%u) != \"%s\" (#%u)", a_name.data(), a, b_name.data(), b);
  }
}
#endif // !ASMJIT_NO_TEXT

#ifndef ASMJIT_NO_INTROSPECTION
template<typename... Args>
static Error query_features_inline(CpuFeatures* out, Arch arch, BaseInst inst, Args&&... args) {
  Operand_ op_array[] = { std::forward<Args>(args)... };
  return InstInternal::query_features(arch, inst, op_array, sizeof...(args), out);
}

UNIT(x86_inst_api_cpu_features) {
  INFO("Verifying whether SSE2+ features are reported correctly for legacy instructions");
  {
    CpuFeatures f;

    query_features_inline(&f, Arch::kX64, BaseInst(Inst::kIdPaddd), xmm1, xmm2);
    EXPECT_TRUE(f.x86().has_sse2());

    query_features_inline(&f, Arch::kX64, BaseInst(Inst::kIdAddsubpd), xmm1, xmm2);
    EXPECT_TRUE(f.x86().has_sse3());

    query_features_inline(&f, Arch::kX64, BaseInst(Inst::kIdPshufb), xmm1, xmm2);
    EXPECT_TRUE(f.x86().has_ssse3());

    query_features_inline(&f, Arch::kX64, BaseInst(Inst::kIdBlendpd), xmm1, xmm2, Imm(1));
    EXPECT_TRUE(f.x86().has_sse4_1());

    query_features_inline(&f, Arch::kX64, BaseInst(Inst::kIdCrc32), eax, al);
    EXPECT_TRUE(f.x86().has_sse4_2());
  }

  INFO("Verifying whether AVX+ features are reported correctly for AVX instructions");
  {
    CpuFeatures f;

    query_features_inline(&f, Arch::kX64, BaseInst(Inst::kIdVpaddd), xmm1, xmm2, xmm3);
    EXPECT_TRUE(f.x86().has_avx());

    query_features_inline(&f, Arch::kX64, BaseInst(Inst::kIdVpaddd), ymm1, ymm2, ymm3);
    EXPECT_TRUE(f.x86().has_avx2());

    query_features_inline(&f, Arch::kX64, BaseInst(Inst::kIdVaddsubpd), xmm1, xmm2, xmm3);
    EXPECT_TRUE(f.x86().has_avx());

    query_features_inline(&f, Arch::kX64, BaseInst(Inst::kIdVaddsubpd), ymm1, ymm2, ymm3);
    EXPECT_TRUE(f.x86().has_avx());

    query_features_inline(&f, Arch::kX64, BaseInst(Inst::kIdVpshufb), xmm1, xmm2, xmm3);
    EXPECT_TRUE(f.x86().has_avx());

    query_features_inline(&f, Arch::kX64, BaseInst(Inst::kIdVpshufb), ymm1, ymm2, ymm3);
    EXPECT_TRUE(f.x86().has_avx2());

    query_features_inline(&f, Arch::kX64, BaseInst(Inst::kIdVblendpd), xmm1, xmm2, xmm3, Imm(1));
    EXPECT_TRUE(f.x86().has_avx());

    query_features_inline(&f, Arch::kX64, BaseInst(Inst::kIdVblendpd), ymm1, ymm2, ymm3, Imm(1));
    EXPECT_TRUE(f.x86().has_avx());

    query_features_inline(&f, Arch::kX64, BaseInst(Inst::kIdVpunpcklbw), xmm1, xmm2, xmm3);
    EXPECT_TRUE(f.x86().has_avx());

    query_features_inline(&f, Arch::kX64, BaseInst(Inst::kIdVpunpcklbw), ymm1, ymm2, ymm3);
    EXPECT_TRUE(f.x86().has_avx2());
  }

  INFO("Verifying whether AVX2 / AVX512 features are reported correctly for vpgatherxx instructions");
  {
    CpuFeatures f;

    query_features_inline(&f, Arch::kX64, BaseInst(Inst::kIdVpgatherdd), xmm1, ptr(rax, xmm2), xmm3);
    EXPECT_TRUE(f.x86().has_avx2());
    EXPECT_FALSE(f.x86().has_avx512_f());

    // NOTE: This instruction is unencodable, but sometimes this signature is used to check the support (without the {k}).
    query_features_inline(&f, Arch::kX64, BaseInst(Inst::kIdVpgatherdd), xmm1, ptr(rax, xmm2));
    EXPECT_FALSE(f.x86().has_avx2());
    EXPECT_TRUE(f.x86().has_avx512_f());

    query_features_inline(&f, Arch::kX64, BaseInst(Inst::kIdVpgatherdd, InstOptions::kNone, k1), xmm1, ptr(rax, xmm2));
    EXPECT_FALSE(f.x86().has_avx2());
    EXPECT_TRUE(f.x86().has_avx512_f());
  }
}
#endif // !ASMJIT_NO_INTROSPECTION

#ifndef ASMJIT_NO_INTROSPECTION
template<typename... Args>
static Error query_rw_info_inline(InstRWInfo* out, Arch arch, BaseInst inst, Args&&... args) {
  Operand_ op_array[] = { std::forward<Args>(args)... };
  return InstInternal::query_rw_info(arch, inst, op_array, sizeof...(args), out);
}

UNIT(x86_inst_api_rm_features) {
  INFO("Verifying whether RM/feature is reported correctly for PEXTRW instruction");
  {
    InstRWInfo rwi;

    query_rw_info_inline(&rwi, Arch::kX64, BaseInst(Inst::kIdPextrw), eax, mm1, imm(1));
    EXPECT_EQ(rwi.rm_feature(), 0u);

    query_rw_info_inline(&rwi, Arch::kX64, BaseInst(Inst::kIdPextrw), eax, xmm1, imm(1));
    EXPECT_EQ(rwi.rm_feature(), CpuFeatures::X86::kSSE4_1);
  }

  INFO("Verifying whether RM/feature is reported correctly for AVX512 shift instructions");
  {
    InstRWInfo rwi;

    query_rw_info_inline(&rwi, Arch::kX64, BaseInst(Inst::kIdVpslld), xmm1, xmm2, imm(8));
    EXPECT_EQ(rwi.rm_feature(), CpuFeatures::X86::kAVX512_F);

    query_rw_info_inline(&rwi, Arch::kX64, BaseInst(Inst::kIdVpsllq), ymm1, ymm2, imm(8));
    EXPECT_EQ(rwi.rm_feature(), CpuFeatures::X86::kAVX512_F);

    query_rw_info_inline(&rwi, Arch::kX64, BaseInst(Inst::kIdVpsrad), xmm1, xmm2, imm(8));
    EXPECT_EQ(rwi.rm_feature(), CpuFeatures::X86::kAVX512_F);

    query_rw_info_inline(&rwi, Arch::kX64, BaseInst(Inst::kIdVpsrld), ymm1, ymm2, imm(8));
    EXPECT_EQ(rwi.rm_feature(), CpuFeatures::X86::kAVX512_F);

    query_rw_info_inline(&rwi, Arch::kX64, BaseInst(Inst::kIdVpsrlq), xmm1, xmm2, imm(8));
    EXPECT_EQ(rwi.rm_feature(), CpuFeatures::X86::kAVX512_F);

    query_rw_info_inline(&rwi, Arch::kX64, BaseInst(Inst::kIdVpslldq), xmm1, xmm2, imm(8));
    EXPECT_EQ(rwi.rm_feature(), CpuFeatures::X86::kAVX512_BW);

    query_rw_info_inline(&rwi, Arch::kX64, BaseInst(Inst::kIdVpsllw), ymm1, ymm2, imm(8));
    EXPECT_EQ(rwi.rm_feature(), CpuFeatures::X86::kAVX512_BW);

    query_rw_info_inline(&rwi, Arch::kX64, BaseInst(Inst::kIdVpsraw), xmm1, xmm2, imm(8));
    EXPECT_EQ(rwi.rm_feature(), CpuFeatures::X86::kAVX512_BW);

    query_rw_info_inline(&rwi, Arch::kX64, BaseInst(Inst::kIdVpsrldq), ymm1, ymm2, imm(8));
    EXPECT_EQ(rwi.rm_feature(), CpuFeatures::X86::kAVX512_BW);

    query_rw_info_inline(&rwi, Arch::kX64, BaseInst(Inst::kIdVpsrlw), xmm1, xmm2, imm(8));
    EXPECT_EQ(rwi.rm_feature(), CpuFeatures::X86::kAVX512_BW);

    query_rw_info_inline(&rwi, Arch::kX64, BaseInst(Inst::kIdVpslld), xmm1, xmm2, xmm3);
    EXPECT_EQ(rwi.rm_feature(), 0u);

    query_rw_info_inline(&rwi, Arch::kX64, BaseInst(Inst::kIdVpsllw), xmm1, xmm2, xmm3);
    EXPECT_EQ(rwi.rm_feature(), 0u);
  }
}
#endif // !ASMJIT_NO_INTROSPECTION

#endif // ASMJIT_TEST

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_X86
