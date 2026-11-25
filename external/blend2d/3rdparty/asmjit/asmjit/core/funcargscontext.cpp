// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#include <asmjit/core/funcargscontext_p.h>

ASMJIT_BEGIN_NAMESPACE

//! \cond INTERNAL
//! \addtogroup asmjit_core
//! \{

FuncArgsContext::FuncArgsContext() noexcept {
  for (WorkData& wd : _work_data) {
    wd.reset();
  }
}

ASMJIT_FAVOR_SIZE Error FuncArgsContext::init_work_data(const FuncFrame& frame, const FuncArgsAssignment& args, const RAConstraints* constraints) noexcept {
  Arch arch = frame.arch();
  const FuncDetail& func = *args.func_detail();

  _arch_traits = &ArchTraits::by_arch(arch);
  _constraints = constraints;
  _arch = arch;

  // Initialize `_arch_regs`.
  for (RegGroup group : Support::enumerate(RegGroup::kMaxVirt)) {
    _work_data[group]._arch_regs = _constraints->available_regs(group);
  }

  if (frame.has_preserved_fp()) {
    _work_data[size_t(RegGroup::kGp)]._arch_regs &= ~Support::bit_mask<RegMask>(arch_traits().fp_reg_id());
  }

  uint32_t reassignment_flag_mask = 0;

  // Extract information from all function arguments/assignments and build Var[] array.
  uint32_t var_id = 0;
  uint32_t arg_count = args.func_detail()->arg_count();

  for (uint32_t arg_index = 0; arg_index < arg_count; arg_index++) {
    for (uint32_t value_index = 0; value_index < Globals::kMaxValuePack; value_index++) {
      const FuncValue& dst_ = args.arg(arg_index, value_index);
      if (!dst_.is_assigned()) {
        continue;
      }

      const FuncValue& src_ = func.arg(arg_index, value_index);
      if (ASMJIT_UNLIKELY(!src_.is_assigned())) {
        return make_error(Error::kInvalidState);
      }

      Var& var = _vars[var_id];
      var.init(src_, dst_);

      FuncValue& src = var.cur;
      FuncValue& dst = var.out;

      RegGroup dst_group = RegGroup::kMaxValue;
      uint32_t dst_id = Reg::kIdBad;
      WorkData* dst_wd = nullptr;

      // Not supported.
      if (src.is_indirect()) {
        return make_error(Error::kInvalidAssignment);
      }

      if (dst.is_reg()) {
        RegType dst_type = dst.reg_type();
        if (ASMJIT_UNLIKELY(!arch_traits().has_reg_type(dst_type))) {
          return make_error(Error::kInvalidRegType);
        }

        // Copy TypeId from source if the destination doesn't have it. The RA used by BaseCompiler would never
        // leave TypeId undefined, but users of FuncAPI can just assign phys regs without specifying their types.
        if (!dst.has_type_id()) {
          dst.set_type_id(RegUtils::type_id_of(dst.reg_type()));
        }

        dst_group = RegUtils::group_of(dst_type);
        if (ASMJIT_UNLIKELY(dst_group > RegGroup::kMaxVirt)) {
          return make_error(Error::kInvalidRegGroup);
        }

        dst_wd = &_work_data[dst_group];
        dst_id = dst.reg_id();

        if (ASMJIT_UNLIKELY(dst_id >= 32 || !Support::bit_test(dst_wd->arch_regs(), dst_id))) {
          return make_error(Error::kInvalidPhysId);
        }

        if (ASMJIT_UNLIKELY(Support::bit_test(dst_wd->dst_regs(), dst_id))) {
          return make_error(Error::kOverlappedRegs);
        }

        dst_wd->_dst_regs  |= Support::bit_mask<RegMask>(dst_id);
        dst_wd->_dst_shuf  |= Support::bit_mask<RegMask>(dst_id);
        dst_wd->_used_regs |= Support::bit_mask<RegMask>(dst_id);
      }
      else {
        if (!dst.has_type_id()) {
          dst.set_type_id(src.type_id());
        }

        OperandSignature signature = get_suitable_reg_for_mem_to_mem_move(arch, dst.type_id(), src.type_id());
        if (ASMJIT_UNLIKELY(!signature.is_valid())) {
          return make_error(Error::kInvalidState);
        }
        _stack_dst_mask = uint8_t(_stack_dst_mask | Support::bit_mask<uint32_t>(signature.reg_group()));
      }

      if (src.is_reg()) {
        uint32_t src_id = src.reg_id();
        RegGroup src_group = RegUtils::group_of(src.reg_type());

        if (dst_group == src_group) {
          ASMJIT_ASSERT(dst_wd != nullptr);
          dst_wd->assign(var_id, src_id);

          reassignment_flag_mask |= uint32_t(dst_id != src_id) << uint32_t(dst_group);

          if (dst_id == src_id) {
            // The best case, register is allocated where it is expected to be. However, we should
            // not mark this as done if both registers are GP and sign or zero extension is required.
            if (dst_group != RegGroup::kGp) {
              var.mark_done();
            }
            else {
              TypeId dt = dst.type_id();
              TypeId st = src.type_id();

              uint32_t dst_size = TypeUtils::size_of(dt);
              uint32_t src_size = TypeUtils::size_of(st);

              if (dt == TypeId::kVoid || st == TypeId::kVoid || dst_size <= src_size) {
                var.mark_done();
              }
            }
          }
        }
        else {
          if (ASMJIT_UNLIKELY(src_group > RegGroup::kMaxVirt)) {
            return make_error(Error::kInvalidState);
          }

          WorkData& src_data = _work_data[size_t(src_group)];
          src_data.assign(var_id, src_id);
          reassignment_flag_mask |= 1u << uint32_t(dst_group);
        }
      }
      else {
        if (dst_wd)
          dst_wd->_num_stack_args++;
        _has_stack_src = true;
      }

      var_id++;
    }
  }

  // Initialize WorkData::work_regs.
  for (RegGroup group : Support::enumerate(RegGroup::kMaxVirt)) {
    _work_data[group]._work_regs =
      (_work_data[group].arch_regs() & (frame.dirty_regs(group) | ~frame.preserved_regs(group))) | _work_data[group].dst_regs() | _work_data[group].assigned_regs();
    _work_data[group]._needs_scratch = (reassignment_flag_mask >> uint32_t(group)) & 1u;
  }

  // Create a variable that represents `SARegId` if necessary.
  bool sa_reg_required = _has_stack_src && frame.has_dynamic_alignment() && !frame.has_preserved_fp();

  WorkData& gp_regs = _work_data[RegGroup::kGp];
  uint32_t sa_cur_reg_id = frame.sa_reg_id();
  uint32_t sa_out_reg_id = args.sa_reg_id();

  if (sa_cur_reg_id != Reg::kIdBad) {
    // Check if the provided `SARegId` doesn't collide with input registers.
    if (ASMJIT_UNLIKELY(gp_regs.is_assigned(sa_cur_reg_id))) {
      return make_error(Error::kOverlappedRegs);
    }
  }

  if (sa_out_reg_id != Reg::kIdBad) {
    // Check if the provided `SARegId` doesn't collide with argument assignments.
    if (ASMJIT_UNLIKELY(Support::bit_test(gp_regs.dst_regs(), sa_out_reg_id))) {
      return make_error(Error::kOverlappedRegs);
    }
    sa_reg_required = true;
  }

  if (sa_reg_required) {
    TypeId ptr_type_id = Environment::is_32bit(arch) ? TypeId::kUInt32 : TypeId::kUInt64;
    RegType ptr_reg_type = Environment::is_32bit(arch) ? RegType::kGp32 : RegType::kGp64;

    _sa_var_id = uint8_t(var_id);
    _has_preserved_fp = frame.has_preserved_fp();

    Var& var = _vars[var_id];
    var.reset();

    if (sa_cur_reg_id == Reg::kIdBad) {
      if (sa_out_reg_id != Reg::kIdBad && !gp_regs.is_assigned(sa_out_reg_id)) {
        sa_cur_reg_id = sa_out_reg_id;
      }
      else {
        RegMask available_regs = gp_regs.available_regs();
        if (!available_regs) {
          available_regs = gp_regs.arch_regs() & ~gp_regs.work_regs();
        }

        if (ASMJIT_UNLIKELY(!available_regs)) {
          return make_error(Error::kNoMorePhysRegs);
        }

        sa_cur_reg_id = Support::ctz(available_regs);
      }
    }

    var.cur.init_reg(ptr_reg_type, sa_cur_reg_id, ptr_type_id);
    gp_regs.assign(var_id, sa_cur_reg_id);
    gp_regs._work_regs |= Support::bit_mask<RegMask>(sa_cur_reg_id);

    if (sa_out_reg_id != Reg::kIdBad) {
      var.out.init_reg(ptr_reg_type, sa_out_reg_id, ptr_type_id);
      gp_regs._dst_regs  |= Support::bit_mask<RegMask>(sa_out_reg_id);
      gp_regs._work_regs |= Support::bit_mask<RegMask>(sa_out_reg_id);
    }
    else {
      var.mark_done();
    }

    var_id++;
  }

  _var_count = var_id;

  // Detect register swaps.
  for (var_id = 0; var_id < _var_count; var_id++) {
    Var& var = _vars[var_id];
    if (var.cur.is_reg() && var.out.is_reg()) {
      uint32_t src_id = var.cur.reg_id();
      uint32_t dst_id = var.out.reg_id();

      RegGroup group = RegUtils::group_of(var.cur.reg_type());
      if (group != RegUtils::group_of(var.out.reg_type())) {
        continue;
      }

      WorkData& wd = _work_data[group];
      if (wd.is_assigned(dst_id)) {
        Var& other = _vars[wd._phys_to_var_id[dst_id]];
        if (RegUtils::group_of(other.out.reg_type()) == group && other.out.reg_id() == src_id) {
          wd._num_swaps++;
          _reg_swaps_mask = uint8_t(_reg_swaps_mask | Support::bit_mask<uint32_t>(group));
        }
      }
    }
  }

  return Error::kOk;
}

ASMJIT_FAVOR_SIZE Error FuncArgsContext::mark_dst_regs_dirty(FuncFrame& frame) noexcept {
  for (RegGroup group : Support::enumerate(RegGroup::kMaxVirt)) {
    WorkData& wd = _work_data[group];
    uint32_t regs = wd.used_regs() | wd._dst_shuf;

    wd._work_regs |= regs;
    frame.add_dirty_regs(group, regs);
  }

  return Error::kOk;
}

ASMJIT_FAVOR_SIZE Error FuncArgsContext::mark_scratch_regs(FuncFrame& frame) noexcept {
  uint32_t group_mask = 0;

  // Handle stack to stack moves.
  group_mask |= _stack_dst_mask;

  // Handle register swaps.
  group_mask |= _reg_swaps_mask & ~Support::bit_mask<uint32_t>(RegGroup::kGp);

  if (!group_mask)
    return Error::kOk;

  // Selects one dirty register per affected group that can be used as a scratch register.
  for (RegGroup group : Support::enumerate(RegGroup::kMaxVirt)) {
    if (Support::bit_test(group_mask, group)) {
      WorkData& wd = _work_data[group];
      if (wd._needs_scratch) {
        // Initially, pick some clobbered or dirty register.
        RegMask work_regs = wd.work_regs();
        RegMask regs = work_regs & ~(wd.used_regs() | wd._dst_shuf);

        // If that didn't work out pick some register which is not in 'used'.
        if (!regs) {
          regs = work_regs & ~wd.used_regs();
        }

        // If that didn't work out pick any other register that is allocable.
        // This last resort case will, however, result in marking one more
        // register dirty.
        if (!regs) {
          regs = wd.arch_regs() & ~work_regs;
        }

        // If that didn't work out we will have to use XORs instead of MOVs.
        if (!regs) {
          continue;
        }

        RegMask reg_mask = Support::blsi(regs);
        wd._work_regs |= reg_mask;
        frame.add_dirty_regs(group, reg_mask);
      }
    }
  }

  return Error::kOk;
}

ASMJIT_FAVOR_SIZE Error FuncArgsContext::mark_stack_args_reg(FuncFrame& frame) noexcept {
  if (_sa_var_id != kVarIdNone) {
    const Var& var = _vars[_sa_var_id];
    frame.set_sa_reg_id(var.cur.reg_id());
  }
  else if (frame.has_preserved_fp()) {
    frame.set_sa_reg_id(arch_traits().fp_reg_id());
  }

  return Error::kOk;
}

//! \}
//! \endcond

ASMJIT_END_NAMESPACE
