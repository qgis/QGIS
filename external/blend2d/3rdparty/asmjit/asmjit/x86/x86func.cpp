// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#if !defined(ASMJIT_NO_X86)

#include <asmjit/x86/x86func_p.h>
#include <asmjit/x86/x86emithelper_p.h>
#include <asmjit/x86/x86operand.h>

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

namespace FuncInternal {

[[nodiscard]]
static inline bool should_treat_as_cdeclIn64BitMode(CallConvId call_conv_id) noexcept {
  return call_conv_id == CallConvId::kCDecl ||
         call_conv_id == CallConvId::kStdCall ||
         call_conv_id == CallConvId::kThisCall ||
         call_conv_id == CallConvId::kFastCall ||
         call_conv_id == CallConvId::kRegParm1 ||
         call_conv_id == CallConvId::kRegParm2 ||
         call_conv_id == CallConvId::kRegParm3;
}

ASMJIT_FAVOR_SIZE Error init_call_conv(CallConv& cc, CallConvId call_conv_id, const Environment& environment) noexcept {
  constexpr uint32_t kZax = Gp::kIdAx;
  constexpr uint32_t kZbx = Gp::kIdBx;
  constexpr uint32_t kZcx = Gp::kIdCx;
  constexpr uint32_t kZdx = Gp::kIdDx;
  constexpr uint32_t kZsp = Gp::kIdSp;
  constexpr uint32_t kZbp = Gp::kIdBp;
  constexpr uint32_t kZsi = Gp::kIdSi;
  constexpr uint32_t kZdi = Gp::kIdDi;

  bool win_abi = environment.is_platform_windows() || environment.is_msvc_abi();

  cc.set_arch(environment.arch());
  cc.set_save_restore_reg_size(RegGroup::kVec, 16);
  cc.set_save_restore_reg_size(RegGroup::kMask, 8);
  cc.set_save_restore_reg_size(RegGroup::kX86_MM, 8);
  cc.set_save_restore_alignment(RegGroup::kVec, 16);
  cc.set_save_restore_alignment(RegGroup::kMask, 8);
  cc.set_save_restore_alignment(RegGroup::kX86_MM, 8);

  if (environment.is_32bit()) {
    bool is_standard_call_conv = true;

    cc.set_save_restore_reg_size(RegGroup::kGp, 4);
    cc.set_save_restore_alignment(RegGroup::kGp, 4);

    cc.set_preserved_regs(RegGroup::kGp, Support::bit_mask<RegMask>(Gp::kIdBx, Gp::kIdSp, Gp::kIdBp, Gp::kIdSi, Gp::kIdDi));
    cc.set_natural_stack_alignment(4);

    switch (call_conv_id) {
      case CallConvId::kCDecl:
        break;

      case CallConvId::kStdCall:
        cc.set_flags(CallConvFlags::kCalleePopsStack);
        break;

      case CallConvId::kFastCall:
        cc.set_flags(CallConvFlags::kCalleePopsStack);
        cc.set_passed_order(RegGroup::kGp, kZcx, kZdx);
        break;

      case CallConvId::kVectorCall:
        cc.set_flags(CallConvFlags::kCalleePopsStack);
        cc.set_passed_order(RegGroup::kGp, kZcx, kZdx);
        cc.set_passed_order(RegGroup::kVec, 0, 1, 2, 3, 4, 5);
        break;

      case CallConvId::kThisCall:
        // NOTE: Even MINGW (starting with GCC 4.7.0) now uses __thiscall on MS Windows, so we won't bail to any
        // other calling convention if __thiscall was specified.
        if (win_abi) {
          cc.set_flags(CallConvFlags::kCalleePopsStack);
          cc.set_passed_order(RegGroup::kGp, kZcx);
        }
        else {
          call_conv_id = CallConvId::kCDecl;
        }
        break;

      case CallConvId::kRegParm1:
        cc.set_passed_order(RegGroup::kGp, kZax);
        break;

      case CallConvId::kRegParm2:
        cc.set_passed_order(RegGroup::kGp, kZax, kZdx);
        break;

      case CallConvId::kRegParm3:
        cc.set_passed_order(RegGroup::kGp, kZax, kZdx, kZcx);
        break;

      case CallConvId::kLightCall2:
      case CallConvId::kLightCall3:
      case CallConvId::kLightCall4: {
        uint32_t n = uint32_t(call_conv_id) - uint32_t(CallConvId::kLightCall2) + 2;

        cc.set_flags(CallConvFlags::kPassFloatsByVec);
        cc.set_passed_order(RegGroup::kGp, kZax, kZdx, kZcx, kZsi, kZdi);
        cc.set_passed_order(RegGroup::kVec, 0, 1, 2, 3, 4, 5, 6, 7);
        cc.set_passed_order(RegGroup::kMask, 0, 1, 2, 3, 4, 5, 6, 7);
        cc.set_passed_order(RegGroup::kX86_MM, 0, 1, 2, 3, 4, 5, 6, 7);
        cc.set_preserved_regs(RegGroup::kGp, Support::lsb_mask<uint32_t>(8));
        cc.set_preserved_regs(RegGroup::kVec, Support::lsb_mask<uint32_t>(8) & ~Support::lsb_mask<uint32_t>(n));

        cc.set_natural_stack_alignment(16);
        is_standard_call_conv = false;
        break;
      }

      default:
        return make_error(Error::kInvalidArgument);
    }

    if (is_standard_call_conv) {
      // MMX arguments is something where compiler vendors disagree. For example GCC and MSVC would pass first three
      // via registers and the rest via stack, however Clang passes all via stack. Returning MMX registers is even
      // more fun, where GCC uses MM0, but Clang uses EAX:EDX pair. I'm not sure it's something we should be worried
      // about as MMX is deprecated anyway.
      cc.set_passed_order(RegGroup::kX86_MM, 0, 1, 2);

      // Vector arguments (XMM|YMM|ZMM) are passed via registers. However, if the function is variadic then they have
      // to be passed via stack.
      cc.set_passed_order(RegGroup::kVec, 0, 1, 2);

      // Functions with variable arguments always use stack for MM and vector arguments.
      cc.add_flags(CallConvFlags::kPassVecByStackIfVA);
    }

    if (call_conv_id == CallConvId::kCDecl) {
      cc.add_flags(CallConvFlags::kVarArgCompatible);
    }
  }
  else {
    cc.set_save_restore_reg_size(RegGroup::kGp, 8);
    cc.set_save_restore_alignment(RegGroup::kGp, 8);

    // Preprocess the calling convention into a common id as many conventions are normally ignored even by C/C++
    // compilers and treated as `__cdecl`.
    if (should_treat_as_cdeclIn64BitMode(call_conv_id))
      call_conv_id = win_abi ? CallConvId::kX64Windows : CallConvId::kX64SystemV;

    switch (call_conv_id) {
      case CallConvId::kX64SystemV: {
        cc.set_flags(CallConvFlags::kPassFloatsByVec |
                    CallConvFlags::kPassMmxByXmm    |
                    CallConvFlags::kVarArgCompatible);
        cc.set_natural_stack_alignment(16);
        cc.set_red_zone_size(128);
        cc.set_passed_order(RegGroup::kGp, kZdi, kZsi, kZdx, kZcx, 8, 9);
        cc.set_passed_order(RegGroup::kVec, 0, 1, 2, 3, 4, 5, 6, 7);
        cc.set_preserved_regs(RegGroup::kGp, Support::bit_mask<RegMask>(kZbx, kZsp, kZbp, 12, 13, 14, 15));
        break;
      }

      case CallConvId::kX64Windows: {
        cc.set_strategy(CallConvStrategy::kX64Windows);
        cc.set_flags(CallConvFlags::kPassFloatsByVec |
                    CallConvFlags::kIndirectVecArgs |
                    CallConvFlags::kPassMmxByGp     |
                    CallConvFlags::kVarArgCompatible);
        cc.set_natural_stack_alignment(16);
        // Maximum 4 arguments in registers, each adds 8 bytes to the spill zone.
        cc.set_spill_zone_size(4 * 8);
        cc.set_passed_order(RegGroup::kGp, kZcx, kZdx, 8, 9);
        cc.set_passed_order(RegGroup::kVec, 0, 1, 2, 3);
        cc.set_preserved_regs(RegGroup::kGp, Support::bit_mask<RegMask>(kZbx, kZsp, kZbp, kZsi, kZdi, 12, 13, 14, 15));
        cc.set_preserved_regs(RegGroup::kVec, Support::bit_mask<RegMask>(6, 7, 8, 9, 10, 11, 12, 13, 14, 15));
        break;
      }

      case CallConvId::kVectorCall: {
        cc.set_strategy(CallConvStrategy::kX64VectorCall);
        cc.set_flags(CallConvFlags::kPassFloatsByVec |
                    CallConvFlags::kPassMmxByGp     );
        cc.set_natural_stack_alignment(16);
        // Maximum 6 arguments in registers, each adds 8 bytes to the spill zone.
        cc.set_spill_zone_size(6 * 8);
        cc.set_passed_order(RegGroup::kGp, kZcx, kZdx, 8, 9);
        cc.set_passed_order(RegGroup::kVec, 0, 1, 2, 3, 4, 5);
        cc.set_preserved_regs(RegGroup::kGp, Support::bit_mask<RegMask>(kZbx, kZsp, kZbp, kZsi, kZdi, 12, 13, 14, 15));
        cc.set_preserved_regs(RegGroup::kVec, Support::bit_mask<RegMask>(6, 7, 8, 9, 10, 11, 12, 13, 14, 15));
        break;
      }

      case CallConvId::kLightCall2:
      case CallConvId::kLightCall3:
      case CallConvId::kLightCall4: {
        uint32_t n = uint32_t(call_conv_id) - uint32_t(CallConvId::kLightCall2) + 2;

        cc.set_flags(CallConvFlags::kPassFloatsByVec);
        cc.set_natural_stack_alignment(16);
        cc.set_passed_order(RegGroup::kGp, kZax, kZdx, kZcx, kZsi, kZdi);
        cc.set_passed_order(RegGroup::kVec, 0, 1, 2, 3, 4, 5, 6, 7);
        cc.set_passed_order(RegGroup::kMask, 0, 1, 2, 3, 4, 5, 6, 7);
        cc.set_passed_order(RegGroup::kX86_MM, 0, 1, 2, 3, 4, 5, 6, 7);

        cc.set_preserved_regs(RegGroup::kGp, Support::lsb_mask<uint32_t>(16));
        cc.set_preserved_regs(RegGroup::kVec, ~Support::lsb_mask<uint32_t>(n));
        break;
      }

      default:
        return make_error(Error::kInvalidArgument);
    }
  }

  cc.set_id(call_conv_id);
  return Error::kOk;
}

ASMJIT_FAVOR_SIZE void unpack_values(FuncDetail& func, FuncValuePack& pack) noexcept {
  TypeId type_id = pack[0].type_id();
  switch (type_id) {
    case TypeId::kInt64:
    case TypeId::kUInt64: {
      if (Environment::is_32bit(func.call_conv().arch())) {
        // Convert a 64-bit return value to two 32-bit return values.
        pack[0].init_type_id(TypeId::kUInt32);
        pack[1].init_type_id(TypeId(uint32_t(type_id) - 2));
        break;
      }
      break;
    }

    default: {
      break;
    }
  }
}

ASMJIT_FAVOR_SIZE Error init_func_detail(FuncDetail& func, const FuncSignature& signature, uint32_t register_size) noexcept {
  const CallConv& cc = func.call_conv();
  Arch arch = cc.arch();
  uint32_t stack_offset = cc._spill_zone_size;
  uint32_t arg_count = func.arg_count();

  // Up to two return values can be returned in GP registers.
  static const uint8_t gp_return_indexes[4] = {
    uint8_t(Gp::kIdAx),
    uint8_t(Gp::kIdDx),
    uint8_t(Reg::kIdBad),
    uint8_t(Reg::kIdBad)
  };

  if (func.has_ret()) {
    unpack_values(func, func._rets);
    for (uint32_t value_index = 0; value_index < Globals::kMaxValuePack; value_index++) {
      TypeId type_id = func._rets[value_index].type_id();

      // Terminate at the first void type (end of the pack).
      if (type_id == TypeId::kVoid) {
        break;
      }

      switch (type_id) {
        case TypeId::kInt64:
        case TypeId::kUInt64: {
          if (gp_return_indexes[value_index] != Reg::kIdBad) {
            func._rets[value_index].init_reg(RegType::kGp64, gp_return_indexes[value_index], type_id);
          }
          else {
            return make_error(Error::kInvalidState);
          }
          break;
        }

        case TypeId::kInt8:
        case TypeId::kInt16:
        case TypeId::kInt32: {
          if (gp_return_indexes[value_index] != Reg::kIdBad) {
            func._rets[value_index].init_reg(RegType::kGp32, gp_return_indexes[value_index], TypeId::kInt32);
          }
          else {
            return make_error(Error::kInvalidState);
          }
          break;
        }

        case TypeId::kUInt8:
        case TypeId::kUInt16:
        case TypeId::kUInt32: {
          if (gp_return_indexes[value_index] != Reg::kIdBad) {
            func._rets[value_index].init_reg(RegType::kGp32, gp_return_indexes[value_index], TypeId::kUInt32);
          }
          else {
            return make_error(Error::kInvalidState);
          }
          break;
        }

        case TypeId::kFloat32:
        case TypeId::kFloat64: {
          RegType reg_type = Environment::is_32bit(arch) ? RegType::kX86_St : RegType::kVec128;
          func._rets[value_index].init_reg(reg_type, value_index, type_id);
          break;
        }

        case TypeId::kFloat80: {
          // 80-bit floats are always returned by FP0.
          func._rets[value_index].init_reg(RegType::kX86_St, value_index, type_id);
          break;
        }

        case TypeId::kMmx32:
        case TypeId::kMmx64: {
          // MM registers are returned through XMM (SystemV) or GPQ (Win64).
          RegType reg_type = RegType::kX86_Mm;
          uint32_t reg_index = value_index;
          if (Environment::is_64bit(arch)) {
            reg_type = cc.strategy() == CallConvStrategy::kDefault ? RegType::kVec128 : RegType::kGp64;
            reg_index = cc.strategy() == CallConvStrategy::kDefault ? value_index : gp_return_indexes[value_index];

            if (reg_index == Reg::kIdBad) {
              return make_error(Error::kInvalidState);
            }
          }

          func._rets[value_index].init_reg(reg_type, reg_index, type_id);
          break;
        }

        default: {
          func._rets[value_index].init_reg(vec_type_id_to_reg_type(type_id), value_index, type_id);
          break;
        }
      }
    }
  }

  switch (cc.strategy()) {
    case CallConvStrategy::kDefault:
    default: {
      uint32_t gpz_pos = 0;
      uint32_t vec_pos = 0;

      for (uint32_t arg_index = 0; arg_index < arg_count; arg_index++) {
        unpack_values(func, func._args[arg_index]);

        for (uint32_t value_index = 0; value_index < Globals::kMaxValuePack; value_index++) {
          FuncValue& arg = func._args[arg_index][value_index];

          // Terminate if there are no more arguments in the pack.
          if (!arg) {
            break;
          }

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
              uint32_t size = Support::max<uint32_t>(TypeUtils::size_of(type_id), register_size);
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

            if (TypeUtils::is_float(type_id)) {
              // If this is a float, but `kFlagPassFloatsByVec` is false, we have to use stack instead. This should
              // be only used by 32-bit calling conventions.
              if (!cc.has_flag(CallConvFlags::kPassFloatsByVec)) {
                reg_id = Reg::kIdBad;
              }
            }
            else {
              // Pass vector registers via stack if this is a variable arguments function. This should be only used
              // by 32-bit calling conventions.
              if (signature.has_var_args() && cc.has_flag(CallConvFlags::kPassVecByStackIfVA)) {
                reg_id = Reg::kIdBad;
              }
            }

            if (reg_id != Reg::kIdBad) {
              arg.init_type_id(type_id);
              arg.assign_reg_data(vec_type_id_to_reg_type(type_id), reg_id);
              func.add_used_regs(RegGroup::kVec, Support::bit_mask<RegMask>(reg_id));
              vec_pos++;
            }
            else {
              uint32_t size = TypeUtils::size_of(type_id);
              arg.assign_stack_offset(int32_t(stack_offset));
              stack_offset += size;
            }
            continue;
          }
        }
      }
      break;
    }

    case CallConvStrategy::kX64Windows:
    case CallConvStrategy::kX64VectorCall: {
      // Both X64 and VectorCall behave similarly - arguments are indexed from left to right. The position of the
      // argument determines in which register the argument is allocated, so it's either GP or one of XMM/YMM/ZMM
      // registers.
      //
      //       [       X64       ] [VecCall]
      // Index: #0   #1   #2   #3   #4   #5
      //
      // GP   : RCX  RDX  R8   R9
      // VEC  : XMM0 XMM1 XMM2 XMM3 XMM4 XMM5
      //
      // For example function `f(int a, double b, int c, double d)` will be:
      //
      //        (a)  (b)  (c)  (d)
      //        RCX  XMM1 R8   XMM3
      //
      // Unused vector registers are used by HVA.
      bool is_vector_call = (cc.strategy() == CallConvStrategy::kX64VectorCall);

      for (uint32_t arg_index = 0; arg_index < arg_count; arg_index++) {
        unpack_values(func, func._args[arg_index]);

        for (uint32_t value_index = 0; value_index < Globals::kMaxValuePack; value_index++) {
          FuncValue& arg = func._args[arg_index][value_index];

          // Terminate if there are no more arguments in the pack.
          if (!arg) {
            break;
          }

          TypeId type_id = arg.type_id();
          uint32_t size = TypeUtils::size_of(type_id);

          if (TypeUtils::is_int(type_id) || TypeUtils::is_mmx(type_id)) {
            uint32_t reg_id = Reg::kIdBad;

            if (arg_index < CallConv::kMaxRegArgsPerGroup) {
              reg_id = cc._passed_order[RegGroup::kGp].id[arg_index];
            }

            if (reg_id != Reg::kIdBad) {
              RegType reg_type = size <= 4 && !TypeUtils::is_mmx(type_id) ? RegType::kGp32 : RegType::kGp64;
              arg.assign_reg_data(reg_type, reg_id);
              func.add_used_regs(RegGroup::kGp, Support::bit_mask<RegMask>(reg_id));
            }
            else {
              arg.assign_stack_offset(int32_t(stack_offset));
              stack_offset += 8;
            }
            continue;
          }

          if (TypeUtils::is_float(type_id) || TypeUtils::is_vec(type_id)) {
            uint32_t reg_id = Reg::kIdBad;

            if (arg_index < CallConv::kMaxRegArgsPerGroup) {
              reg_id = cc._passed_order[RegGroup::kVec].id[arg_index];
            }

            if (reg_id != Reg::kIdBad) {
              // X64-ABI doesn't allow vector types (XMM|YMM|ZMM) to be passed via registers, however, VectorCall
              // was designed for that purpose.
              if (TypeUtils::is_float(type_id) || is_vector_call) {
                RegType reg_type = vec_type_id_to_reg_type(type_id);
                arg.assign_reg_data(reg_type, reg_id);
                func.add_used_regs(RegGroup::kVec, Support::bit_mask<RegMask>(reg_id));
                continue;
              }
            }

            // Passed via stack if the argument is float/double or indirectly. The trap is - if the argument is
            // passed indirectly, the address can be passed via register, if the argument's index has GP one.
            if (TypeUtils::is_float(type_id)) {
              arg.assign_stack_offset(int32_t(stack_offset));
            }
            else {
              uint32_t gp_reg_id = cc._passed_order[RegGroup::kGp].id[arg_index];
              if (gp_reg_id != Reg::kIdBad) {
                arg.assign_reg_data(RegType::kGp64, gp_reg_id);
              }
              else {
                arg.assign_stack_offset(int32_t(stack_offset));
              }
              arg.add_flags(FuncValue::kFlagIsIndirect);
            }

            // Always 8 bytes (float/double/pointer).
            stack_offset += 8;
            continue;
          }
        }
      }
      break;
    }
  }

  func._arg_stack_size = stack_offset;
  return Error::kOk;
}

} // {FuncInternal}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_X86
