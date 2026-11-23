// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#include <asmjit/core/archtraits.h>
#include <asmjit/core/func.h>
#include <asmjit/core/operand.h>
#include <asmjit/core/type.h>
#include <asmjit/core/funcargscontext_p.h>

#if !defined(ASMJIT_NO_X86)
  #include <asmjit/x86/x86func_p.h>
#endif

#if !defined(ASMJIT_NO_AARCH64)
  #include <asmjit/arm/a64func_p.h>
#endif

ASMJIT_BEGIN_NAMESPACE

// CallConv - Initialization & Reset
// =================================

ASMJIT_FAVOR_SIZE Error CallConv::init(CallConvId call_conv_id, const Environment& environment) noexcept {
  reset();

#if !defined(ASMJIT_NO_X86)
  if (environment.is_family_x86()) {
    return x86::FuncInternal::init_call_conv(*this, call_conv_id, environment);
  }
#endif

#if !defined(ASMJIT_NO_AARCH64)
  if (environment.is_family_aarch64()) {
    return a64::FuncInternal::init_call_conv(*this, call_conv_id, environment);
  }
#endif

  return make_error(Error::kInvalidArgument);
}

// FuncDetail - Init / Reset
// =========================

ASMJIT_FAVOR_SIZE Error FuncDetail::init(const FuncSignature& signature, const Environment& environment) noexcept {
  CallConvId call_conv_id = signature.call_conv_id();
  uint32_t arg_count = signature.arg_count();

  if (ASMJIT_UNLIKELY(arg_count > Globals::kMaxFuncArgs)) {
    return make_error(Error::kInvalidArgument);
  }

  CallConv& cc = _call_conv;
  ASMJIT_PROPAGATE(cc.init(call_conv_id, environment));

  uint32_t register_size = Environment::reg_size_of_arch(cc.arch());
  uint32_t deabstract_delta = TypeUtils::deabstract_delta_of_size(register_size);

  const TypeId* signature_args = signature.args();
  for (uint32_t arg_index = 0; arg_index < arg_count; arg_index++) {
    FuncValuePack& arg_pack = _args[arg_index];
    arg_pack[0].init_type_id(TypeUtils::deabstract(signature_args[arg_index], deabstract_delta));
  }

  _arg_count = uint8_t(arg_count);
  _va_index = uint8_t(signature.va_index());

  TypeId ret = signature.ret();
  if (ret != TypeId::kVoid) {
    _rets[0].init_type_id(TypeUtils::deabstract(ret, deabstract_delta));
  }

#if !defined(ASMJIT_NO_X86)
  if (environment.is_family_x86()) {
    return x86::FuncInternal::init_func_detail(*this, signature, register_size);
  }
#endif

#if !defined(ASMJIT_NO_AARCH64)
  if (environment.is_family_aarch64()) {
    return a64::FuncInternal::init_func_detail(*this, signature);
  }
#endif

  // We should never bubble here as if `cc.init()` succeeded then there has to be an implementation for the current
  // architecture. However, stay safe.
  return make_error(Error::kInvalidArgument);
}

// FuncFrame - Init
// ================

ASMJIT_FAVOR_SIZE Error FuncFrame::init(const FuncDetail& func) noexcept {
  Arch arch = func.call_conv().arch();
  if (!Environment::is_valid_arch(arch)) {
    return make_error(Error::kInvalidArch);
  }

  const ArchTraits& arch_traits = ArchTraits::by_arch(arch);

  // Initializing FuncFrame means making a copy of some properties of `func`. Properties like `_local_stack_size` will
  // be set by the user before the frame is finalized.
  reset();

  _arch = arch;
  _sp_reg_id = uint8_t(arch_traits.sp_reg_id());
  _sa_reg_id = uint8_t(Reg::kIdBad);

  uint32_t natural_stack_alignment = func.call_conv().natural_stack_alignment();
  uint32_t min_dynamic_alignment = Support::max<uint32_t>(natural_stack_alignment, 16);

  if (min_dynamic_alignment == natural_stack_alignment) {
    min_dynamic_alignment <<= 1;
  }

  _natural_stack_alignment = uint8_t(natural_stack_alignment);
  _min_dynamic_alignment = uint8_t(min_dynamic_alignment);
  _red_zone_size = uint8_t(func.red_zone_size());
  _spill_zone_size = uint8_t(func.spill_zone_size());
  _final_stack_alignment = uint8_t(_natural_stack_alignment);

  if (func.has_flag(CallConvFlags::kCalleePopsStack)) {
    _callee_stack_cleanup = uint16_t(func.arg_stack_size());
  }

  // Initial masks of dirty and preserved registers.
  for (RegGroup group : Support::enumerate(RegGroup::kMaxVirt)) {
    _dirty_regs[group] = func.used_regs(group);
    _preserved_regs[group] = func.preserved_regs(group);
  }

  // Exclude stack pointer - this register is never included in saved GP regs.
  _preserved_regs[RegGroup::kGp] &= ~Support::bit_mask<RegMask>(arch_traits.sp_reg_id());

  // The size and alignment of save/restore area of registers for each virtual register group
  _save_restore_reg_size = func.call_conv()._save_restore_reg_size;
  _save_restore_alignment = func.call_conv()._save_restore_alignment;

  return Error::kOk;
}

// FuncFrame - Finalize
// ====================

ASMJIT_FAVOR_SIZE Error FuncFrame::finalize() noexcept {
  if (!Environment::is_valid_arch(arch())) {
    return make_error(Error::kInvalidArch);
  }

  const ArchTraits& arch_traits = ArchTraits::by_arch(arch());

  uint32_t register_size = _save_restore_reg_size[RegGroup::kGp];
  uint32_t vector_size = _save_restore_reg_size[RegGroup::kVec];
  uint32_t return_address_size = arch_traits.has_link_reg() ? 0u : register_size;

  // The final stack alignment must be updated accordingly to call and local stack alignments.
  uint32_t stack_alignment = _final_stack_alignment;
  ASMJIT_ASSERT(stack_alignment == Support::max(_natural_stack_alignment, _call_stack_alignment, _local_stack_alignment));

  bool has_fp = has_preserved_fp();
  bool has_da = has_dynamic_alignment();

  uint32_t kSp = arch_traits.sp_reg_id();
  uint32_t kFp = arch_traits.fp_reg_id();
  uint32_t kLr = arch_traits.link_reg_id();

  // Make frame pointer dirty if the function uses it.
  if (has_fp) {
    _dirty_regs[RegGroup::kGp] |= Support::bit_mask<RegMask>(kFp);

    // Currently required by ARM, if this works differently across architectures we would have to generalize most
    // likely in CallConv.
    if (kLr != Reg::kIdBad) {
      _dirty_regs[RegGroup::kGp] |= Support::bit_mask<RegMask>(kLr);
    }
  }

  // These two are identical if the function doesn't align its stack dynamically.
  uint32_t sa_reg_id = _sa_reg_id;
  if (sa_reg_id == Reg::kIdBad) {
    sa_reg_id = kSp;
  }

  // Fix stack arguments base-register from SP to FP in case it was not picked before and the function performs
  // dynamic stack alignment.
  if (has_da && sa_reg_id == kSp) {
    sa_reg_id = kFp;
  }

  // Mark as dirty any register but SP if used as SA pointer.
  if (sa_reg_id != kSp) {
    _dirty_regs[RegGroup::kGp] |= Support::bit_mask<RegMask>(sa_reg_id);
  }

  _sp_reg_id = uint8_t(kSp);
  _sa_reg_id = uint8_t(sa_reg_id);

  // Setup stack size used to save preserved registers.
  uint32_t save_restore_sizes[2] {};
  for (RegGroup group : Support::enumerate(RegGroup::kMaxVirt)) {
    save_restore_sizes[size_t(!arch_traits.has_inst_push_pop(group))]
      += Support::align_up(Support::popcnt(saved_regs(group)) * save_restore_reg_size(group), save_restore_alignment(group));
  }

  _push_pop_save_size  = uint16_t(save_restore_sizes[0]);
  _extra_reg_save_size = uint16_t(save_restore_sizes[1]);

  uint32_t v = 0;                            // The beginning of the stack frame relative to SP after prolog.
  v += call_stack_size();                      // Count 'call_stack_size'      <- This is used to call functions.
  v  = Support::align_up(v, stack_alignment);  // Align to function's stack alignment.

  _local_stack_offset = v;                     // Store 'local_stack_offset'   <- Function's local stack starts here.
  v += local_stack_size();                     // Count 'local_stack_size'     <- Function's local stack ends here.

  // If the function's stack must be aligned, calculate the alignment necessary to store vector registers, and set
  // `FuncAttributes::kAlignedVecSR` to inform PEI that it can use instructions that perform aligned stores/loads.
  if (stack_alignment >= vector_size && _extra_reg_save_size) {
    add_attributes(FuncAttributes::kAlignedVecSR);
    v = Support::align_up(v, vector_size);     // Align 'extra_reg_save_offset'.
  }

  _extra_reg_save_offset = v;                   // Store 'extra_reg_save_offset' <- Non-GP save/restore starts here.
  v += _extra_reg_save_size;                    // Count 'extra_reg_save_size'   <- Non-GP save/restore ends here.

  // Calculate if dynamic alignment (DA) slot (stored as offset relative to SP) is required and its offset.
  if (has_da && !has_fp) {
    _da_offset = v;                           // Store 'da_offset'           <- DA pointer would be stored here.
    v += register_size;                       // Count 'da_offset'.
  }
  else {
    _da_offset = FuncFrame::kTagInvalidOffset;
  }

  // Link Register
  // -------------
  //
  // The stack is aligned after the function call as the return address is stored in a link register. Some
  // architectures may require to always have aligned stack after PUSH/POP operation, which is represented
  // by ArchTraits::hw_stack_alignment().
  //
  // No Link Register (X86/X64)
  // --------------------------
  //
  // The return address should be stored after GP save/restore regs. It has the same size as `register_size`
  // (basically the native register/pointer size). We don't adjust it now as `v` now contains the exact size
  // that the function requires to adjust (call frame + stack frame, vec stack size). The stack (if we consider
  // this size) is misaligned now, as it's always aligned before the function call - when `call()` is executed
  // it pushes the current EIP|RIP onto the stack, and unaligns it by 12 or 8 bytes (depending on the
  // architecture). So count number of bytes needed to align it up to the function's CallFrame (the beginning).
  if (v || has_func_calls() || !return_address_size) {
    v += Support::align_up_diff(v + push_pop_save_size() + return_address_size, stack_alignment);
  }

  _push_pop_save_offset = v;                  // Store 'push_pop_save_offset' <- Function's push/pop save/restore starts here.
  _stack_adjustment = v;                      // Store 'stack_adjustment'     <- SA used by 'add SP, SA' and 'sub SP, SA'.
  v += _push_pop_save_size;                   // Count 'push_pop_save_size'   <- Function's push/pop save/restore ends here.
  _final_stack_size = v;                      // Store 'final_stack_size'     <- Final stack used by the function.

  if (!arch_traits.has_link_reg()) {
    v += register_size;                       // Count 'ReturnAddress'        <- As CALL pushes onto stack.
  }

  // If the function performs dynamic stack alignment then the stack-adjustment must be aligned.
  if (has_da) {
    _stack_adjustment = Support::align_up(_stack_adjustment, stack_alignment);
  }

  // Calculate where the function arguments start relative to SP.
  _sa_offset_from_sp = has_da ? FuncFrame::kTagInvalidOffset : v;

  // Calculate where the function arguments start relative to FP or user-provided register.
  _sa_offset_from_sa = has_fp ? return_address_size + register_size      // Return address + frame pointer.
                          : return_address_size + _push_pop_save_size; // Return address + all push/pop regs.

  return Error::kOk;
}

// FuncArgsAssignment - UpdateFuncFrame
// ====================================

ASMJIT_FAVOR_SIZE Error FuncArgsAssignment::update_func_frame(FuncFrame& frame) const noexcept {
  Arch arch = frame.arch();
  const FuncDetail* func = func_detail();

  if (!func) {
    return make_error(Error::kInvalidState);
  }

  RAConstraints constraints;
  ASMJIT_PROPAGATE(constraints.init(arch));

  FuncArgsContext ctx;
  ASMJIT_PROPAGATE(ctx.init_work_data(frame, *this, &constraints));
  ASMJIT_PROPAGATE(ctx.mark_dst_regs_dirty(frame));
  ASMJIT_PROPAGATE(ctx.mark_scratch_regs(frame));
  ASMJIT_PROPAGATE(ctx.mark_stack_args_reg(frame));
  return Error::kOk;
}

// Func API - Tests
// ================

#if defined(ASMJIT_TEST)
UNIT(func_signature) {
  FuncSignature signature;
  signature.set_ret_t<int8_t>();
  signature.add_arg_t<int16_t>();
  signature.add_arg(TypeId::kInt32);

  EXPECT_EQ(signature, FuncSignature::build<int8_t, int16_t, int32_t>());
}
#endif

ASMJIT_END_NAMESPACE
