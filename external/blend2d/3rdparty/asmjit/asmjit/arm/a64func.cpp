// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#if !defined(ASMJIT_NO_AARCH64)

#include <asmjit/arm/a64func_p.h>
#include <asmjit/arm/a64operand.h>

ASMJIT_BEGIN_SUB_NAMESPACE(a64)

namespace FuncInternal {

static inline bool should_treat_as_cdecl(CallConvId call_conv_id) noexcept {
  return call_conv_id == CallConvId::kCDecl ||
         call_conv_id == CallConvId::kStdCall ||
         call_conv_id == CallConvId::kFastCall ||
         call_conv_id == CallConvId::kVectorCall ||
         call_conv_id == CallConvId::kThisCall ||
         call_conv_id == CallConvId::kRegParm1 ||
         call_conv_id == CallConvId::kRegParm2 ||
         call_conv_id == CallConvId::kRegParm3;
}

static RegType reg_type_from_fp_or_vec_type_id(TypeId type_id) noexcept {
  if (type_id == TypeId::kFloat32) {
    return RegType::kVec32;
  }
  else if (type_id == TypeId::kFloat64) {
    return RegType::kVec64;
  }
  else if (TypeUtils::is_vec32(type_id)) {
    return RegType::kVec32;
  }
  else if (TypeUtils::is_vec64(type_id)) {
    return RegType::kVec64;
  }
  else if (TypeUtils::is_vec128(type_id)) {
    return RegType::kVec128;
  }
  else {
    return RegType::kNone;
  }
}

ASMJIT_FAVOR_SIZE Error init_call_conv(CallConv& cc, CallConvId call_conv_id, const Environment& environment) noexcept {
  cc.set_arch(environment.arch());
  cc.set_strategy(environment.is_darwin_abi() ? CallConvStrategy::kAArch64Apple : CallConvStrategy::kDefault);

  cc.set_save_restore_reg_size(RegGroup::kGp, 8);
  cc.set_save_restore_reg_size(RegGroup::kVec, 8);
  cc.set_save_restore_alignment(RegGroup::kGp, 16);
  cc.set_save_restore_alignment(RegGroup::kVec, 16);
  cc.set_save_restore_alignment(RegGroup::kMask, 8);
  cc.set_save_restore_alignment(RegGroup::kExtra, 1);
  cc.set_passed_order(RegGroup::kGp, 0, 1, 2, 3, 4, 5, 6, 7);
  cc.set_passed_order(RegGroup::kVec, 0, 1, 2, 3, 4, 5, 6, 7);
  cc.set_natural_stack_alignment(16);

  if (should_treat_as_cdecl(call_conv_id)) {
    // ARM doesn't have that many calling conventions as we can find in X86 world, treat most conventions as __cdecl.
    cc.set_id(CallConvId::kCDecl);
    cc.set_preserved_regs(RegGroup::kGp, Support::bit_mask<RegMask>(Gp::kIdOs, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30));
    cc.set_preserved_regs(RegGroup::kVec, Support::bit_mask<RegMask>(8, 9, 10, 11, 12, 13, 14, 15));
  }
  else {
    cc.set_id(call_conv_id);
    cc.set_save_restore_reg_size(RegGroup::kVec, 16);
    cc.set_preserved_regs(RegGroup::kGp, Support::bit_mask<RegMask>(4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30));
    cc.set_preserved_regs(RegGroup::kVec, Support::bit_mask<RegMask>(4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31));
  }

  return Error::kOk;
}

ASMJIT_FAVOR_SIZE Error init_func_detail(FuncDetail& func, const FuncSignature& signature) noexcept {
  Support::maybe_unused(signature);

  const CallConv& cc = func.call_conv();
  uint32_t arg_count = func.arg_count();
  uint32_t stack_offset = 0;

  // Minimum stack size of a single argument passed via stack. The standard AArch64 calling convention
  // specifies 8 bytes, so each function argument would occupy at least 8 bytes even if it needs less.
  // However, Apple has decided to not follow this rule and function argument can occupy less, for
  // example two consecutive 32-bit arguments would occupy 8 bytes total, instead of 16 as specified
  // by ARM.
  uint32_t min_stack_arg_size = cc.strategy() == CallConvStrategy::kAArch64Apple ? 4u : 8u;

  if (func.has_ret()) {
    for (uint32_t value_index = 0; value_index < Globals::kMaxValuePack; value_index++) {
      TypeId type_id = func._rets[value_index].type_id();

      // Terminate at the first void type (end of the pack).
      if (type_id == TypeId::kVoid)
        break;

      switch (type_id) {
        case TypeId::kInt8:
        case TypeId::kInt16:
        case TypeId::kInt32: {
          func._rets[value_index].init_reg(RegType::kGp32, value_index, TypeId::kInt32);
          break;
        }

        case TypeId::kUInt8:
        case TypeId::kUInt16:
        case TypeId::kUInt32: {
          func._rets[value_index].init_reg(RegType::kGp32, value_index, TypeId::kUInt32);
          break;
        }

        case TypeId::kInt64:
        case TypeId::kUInt64: {
          func._rets[value_index].init_reg(RegType::kGp64, value_index, type_id);
          break;
        }

        default: {
          RegType reg_type = reg_type_from_fp_or_vec_type_id(type_id);
          if (reg_type == RegType::kNone) {
            return make_error(Error::kInvalidRegType);
          }

          func._rets[value_index].init_reg(reg_type, value_index, type_id);
          break;
        }
      }
    }
  }

  switch (cc.strategy()) {
    case CallConvStrategy::kDefault:
    case CallConvStrategy::kAArch64Apple: {
      uint32_t gpz_pos = 0;
      uint32_t vec_pos = 0;

      for (uint32_t i = 0; i < arg_count; i++) {
        FuncValue& arg = func._args[i][0];
        TypeId type_id = arg.type_id();

        if (TypeUtils::is_int(type_id)) {
          uint32_t reg_id = Reg::kIdBad;

          if (gpz_pos < CallConv::kMaxRegArgsPerGroup) {
            reg_id = cc._passed_order[RegGroup::kGp].id[gpz_pos];
          }

          if (reg_id != Reg::kIdBad) {
            RegType reg_type = type_id <= TypeId::kUInt32 ? RegType::kGp32 : RegType::kGp64;
            arg.assign_reg_data(reg_type, reg_id);
            func.add_used_regs(RegGroup::kGp, Support::bit_mask<RegMask>(reg_id));
            gpz_pos++;
          }
          else {
            uint32_t size = Support::max<uint32_t>(TypeUtils::size_of(type_id), min_stack_arg_size);
            if (size >= 8) {
              stack_offset = Support::align_up(stack_offset, 8);
            }
            arg.assign_stack_offset(int32_t(stack_offset));
            stack_offset += size;
          }
          continue;
        }

        if (TypeUtils::is_float(type_id) || TypeUtils::is_vec(type_id)) {
          uint32_t reg_id = Reg::kIdBad;

          if (vec_pos < CallConv::kMaxRegArgsPerGroup) {
            reg_id = cc._passed_order[RegGroup::kVec].id[vec_pos];
          }

          if (reg_id != Reg::kIdBad) {
            RegType reg_type = reg_type_from_fp_or_vec_type_id(type_id);
            if (reg_type == RegType::kNone) {
              return make_error(Error::kInvalidRegType);
            }

            arg.init_type_id(type_id);
            arg.assign_reg_data(reg_type, reg_id);
            func.add_used_regs(RegGroup::kVec, Support::bit_mask<RegMask>(reg_id));
            vec_pos++;
          }
          else {
            uint32_t size = Support::max<uint32_t>(TypeUtils::size_of(type_id), min_stack_arg_size);
            if (size >= 8) {
              stack_offset = Support::align_up(stack_offset, 8);
            }
            arg.assign_stack_offset(int32_t(stack_offset));
            stack_offset += size;
          }
          continue;
        }
      }
      break;
    }

    default:
      return make_error(Error::kInvalidState);
  }

  func._arg_stack_size = Support::align_up(stack_offset, 8u);
  return Error::kOk;
}

} // {FuncInternal}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_AARCH64
