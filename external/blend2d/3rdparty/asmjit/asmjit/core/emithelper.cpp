// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#include <asmjit/core/archtraits.h>
#include <asmjit/core/emithelper_p.h>
#include <asmjit/core/formatter.h>
#include <asmjit/core/funcargscontext_p.h>
#include <asmjit/core/radefs_p.h>

// Can be used for debugging...
// #define ASMJIT_DUMP_ARGS_ASSIGNMENT

ASMJIT_BEGIN_NAMESPACE

// BaseEmitHelper - Formatting
// ===========================

#ifdef ASMJIT_DUMP_ARGS_ASSIGNMENT
static void dump_func_value(String& sb, Arch arch, const FuncValue& value) noexcept {
  Formatter::format_type_id(sb, value.type_id());
  sb.append('@');

  if (value.is_indirect()) {
    sb.append('[');
  }

  if (value.is_reg()) {
    Formatter::format_register(sb, 0, nullptr, arch, value.reg_type(), value.reg_id());
  }
  else if (value.is_stack()) {
    sb.append_format("[%d]", value.stack_offset());
  }
  else {
    sb.append("<none>");
  }

  if (value.is_indirect()) {
    sb.append(']');
  }
}

static void dump_assignment(String& sb, const FuncArgsContext& ctx) noexcept {
  using Var = FuncArgsContext::Var;

  Arch arch = ctx.arch();
  uint32_t var_count = ctx.var_count();

  for (uint32_t i = 0; i < var_count; i++) {
    const Var& var = ctx.var(i);
    const FuncValue& dst = var.out;
    const FuncValue& cur = var.cur;

    sb.append_format("Var%u: ", i);
    dump_func_value(sb, arch, dst);
    sb.append(" <- ");
    dump_func_value(sb, arch, cur);

    if (var.is_done()) {
      sb.append(" {Done}");
    }

    sb.append('\n');
  }
}
#endif

// BaseEmitHelper - Abstract
// =========================

Error BaseEmitHelper::emit_reg_move(const Operand_& dst_, const Operand_& src_, TypeId type_id, const char* comment) {
  Support::maybe_unused(dst_, src_, type_id, comment);
  return make_error(Error::kInvalidState);
}

Error BaseEmitHelper::emit_reg_swap(const Reg& a, const Reg& b, const char* comment) {
  Support::maybe_unused(a, b, comment);
  return make_error(Error::kInvalidState);
}

Error BaseEmitHelper::emit_arg_move(const Reg& dst_, TypeId dst_type_id, const Operand_& src_, TypeId src_type_id, const char* comment) {
  Support::maybe_unused(dst_, dst_type_id, src_, src_type_id, comment);
  return make_error(Error::kInvalidState);
}

// BaseEmitHelper - EmitArgsAssignment
// ===================================

ASMJIT_FAVOR_SIZE Error BaseEmitHelper::emit_args_assignment(const FuncFrame& frame, const FuncArgsAssignment& args) {
  using Var = FuncArgsContext::Var;
  using WorkData = FuncArgsContext::WorkData;

  enum WorkFlags : uint32_t {
    kWorkNone      = 0x00,
    kWorkDidSome   = 0x01,
    kWorkPending   = 0x02,
    kWorkPostponed = 0x04
  };

  Arch arch = frame.arch();
  const ArchTraits& arch_traits = ArchTraits::by_arch(arch);

  RAConstraints constraints;
  FuncArgsContext ctx;

  ASMJIT_PROPAGATE(constraints.init(arch));
  ASMJIT_PROPAGATE(ctx.init_work_data(frame, args, &constraints));

#ifdef ASMJIT_DUMP_ARGS_ASSIGNMENT
  {
    String sb;
    dump_assignment(sb, ctx);
    printf("%s\n", sb.data());
  }
#endif

  auto& work_data = ctx._work_data;
  uint32_t var_count = ctx._var_count;
  uint32_t sa_var_id = ctx._sa_var_id;

  Reg sp = Reg(_emitter->_gp_signature, arch_traits.sp_reg_id());
  Reg sa = sp;

  if (frame.has_dynamic_alignment()) {
    if (frame.has_preserved_fp()) {
      sa.set_id(arch_traits.fp_reg_id());
    }
    else {
      sa.set_id(sa_var_id < var_count ? ctx._vars[sa_var_id].cur.reg_id() : frame.sa_reg_id());
    }
  }

  // Register to stack and stack to stack moves must be first as now we have
  // the biggest chance of having as many as possible unassigned registers.

  if (ctx._stack_dst_mask) {
    // Base address of all arguments passed by stack.
    BaseMem base_arg_ptr(sa, int32_t(frame.sa_offset(sa.id())));
    BaseMem base_stack_ptr(sp, 0);

    for (uint32_t var_id = 0; var_id < var_count; var_id++) {
      Var& var = ctx._vars[var_id];

      if (!var.out.is_stack()) {
        continue;
      }

      FuncValue& cur = var.cur;
      FuncValue& out = var.out;

      ASMJIT_ASSERT(cur.is_reg() || cur.is_stack());
      Reg reg;

      BaseMem dst_stack_ptr = base_stack_ptr.clone_adjusted(out.stack_offset());
      BaseMem src_stack_ptr = base_arg_ptr.clone_adjusted(cur.stack_offset());

      if (cur.is_indirect()) {
        if (cur.is_stack()) {
          // TODO: Indirect stack.
          return make_error(Error::kInvalidAssignment);
        }
        else {
          src_stack_ptr.set_base_id(cur.reg_id());
        }
      }

      if (cur.is_reg() && !cur.is_indirect()) {
        WorkData& wd = work_data[RegUtils::group_of(cur.reg_type())];
        uint32_t reg_id = cur.reg_id();

        reg.set_signature_and_id(RegUtils::signature_of(cur.reg_type()), reg_id);
        wd.unassign(var_id, reg_id);
      }
      else {
        // Stack to reg move - tricky since we move stack to stack we can decide which register to use. In general
        // we follow the rule that IntToInt moves will use GP regs with possibility to signature or zero extend,
        // and all other moves will either use GP or VEC regs depending on the size of the move.
        OperandSignature signature = get_suitable_reg_for_mem_to_mem_move(arch, out.type_id(), cur.type_id());
        if (ASMJIT_UNLIKELY(!signature.is_valid())) {
          return make_error(Error::kInvalidState);
        }

        WorkData& wd = work_data[signature.reg_group()];
        RegMask available_regs = wd.available_regs();
        if (ASMJIT_UNLIKELY(!available_regs)) {
          return make_error(Error::kInvalidState);
        }

        uint32_t available_id = Support::ctz(available_regs);
        reg.set_signature_and_id(signature, available_id);

        ASMJIT_PROPAGATE(emit_arg_move(reg, out.type_id(), src_stack_ptr, cur.type_id()));
      }

      if (cur.is_indirect() && cur.is_reg()) {
        work_data[RegGroup::kGp].unassign(var_id, cur.reg_id());
      }

      // Register to stack move.
      ASMJIT_PROPAGATE(emit_reg_move(dst_stack_ptr, reg, cur.type_id()));
      var.mark_done();
    }
  }

  // Shuffle all registers that are currently assigned accordingly to target assignment.

  uint32_t work_flags = kWorkNone;
  for (;;) {
    for (uint32_t var_id = 0; var_id < var_count; var_id++) {
      Var& var = ctx._vars[var_id];
      if (var.is_done() || !var.cur.is_reg()) {
        continue;
      }

      FuncValue& cur = var.cur;
      FuncValue& out = var.out;

      RegGroup cur_group = RegUtils::group_of(cur.reg_type());
      RegGroup out_group = RegUtils::group_of(out.reg_type());

      uint32_t cur_id = cur.reg_id();
      uint32_t out_id = out.reg_id();

      if (cur_group != out_group) {
        // TODO: Conversion is not supported.
        return make_error(Error::kInvalidAssignment);
      }
      else {
        WorkData& wd = work_data[out_group];
        if (!wd.is_assigned(out_id) || cur_id == out_id) {
EmitMove:
          ASMJIT_PROPAGATE(
            emit_arg_move(
              Reg(RegUtils::signature_of(out.reg_type()), out_id), out.type_id(),
              Reg(RegUtils::signature_of(cur.reg_type()), cur_id), cur.type_id()));

          // Only reassign if this is not a sign/zero extension that happens on the same in/out register.
          if (cur_id != out_id) {
            wd.reassign(var_id, out_id, cur_id);
          }

          cur.init_reg(out.reg_type(), out_id, out.type_id());

          if (out_id == out.reg_id()) {
            var.mark_done();
          }
          work_flags |= kWorkDidSome | kWorkPending;
        }
        else {
          uint32_t alt_id = wd._phys_to_var_id[out_id];
          Var& alt_var = ctx._vars[alt_id];

          if (!alt_var.out.is_initialized() || (alt_var.out.is_reg() && alt_var.out.reg_id() == cur_id)) {
            // Only few architectures provide swap operations, and only for few register groups.
            if (arch_traits.has_inst_reg_swap(cur_group)) {
              RegType highest_type = Support::max(cur.reg_type(), alt_var.cur.reg_type());
              if (Support::is_between(highest_type, RegType::kGp8Lo, RegType::kGp16)) {
                highest_type = RegType::kGp32;
              }

              OperandSignature signature = RegUtils::signature_of(highest_type);
              ASMJIT_PROPAGATE(emit_reg_swap(Reg(signature, out_id), Reg(signature, cur_id)));

              wd.swap(var_id, cur_id, alt_id, out_id);
              cur.set_reg_id(out_id);
              var.mark_done();
              alt_var.cur.set_reg_id(cur_id);

              if (alt_var.out.is_initialized()) {
                alt_var.mark_done();
              }
              work_flags |= kWorkDidSome;
            }
            else {
              // If there is a scratch register it can be used to perform the swap.
              RegMask available_regs = wd.available_regs();
              if (available_regs) {
                RegMask in_out_regs = wd.dst_regs();
                if (available_regs & ~in_out_regs) {
                  available_regs &= ~in_out_regs;
                }
                out_id = Support::ctz(available_regs);
                goto EmitMove;
              }
              else {
                work_flags |= kWorkPending;
              }
            }
          }
          else {
            work_flags |= kWorkPending;
          }
        }
      }
    }

    if (!(work_flags & kWorkPending)) {
      break;
    }

    // If we did nothing twice it means that something is really broken.
    if ((work_flags & (kWorkDidSome | kWorkPostponed)) == kWorkPostponed) {
      return make_error(Error::kInvalidState);
    }

    work_flags = (work_flags & kWorkDidSome) ? kWorkNone : kWorkPostponed;
  }

  // Load arguments passed by stack into registers. This is pretty simple and
  // it never requires multiple iterations like the previous phase.

  if (ctx._has_stack_src) {
    uint32_t iter_count = 1;
    if (frame.has_dynamic_alignment() && !frame.has_preserved_fp()) {
      sa.set_id(sa_var_id < var_count ? ctx._vars[sa_var_id].cur.reg_id() : frame.sa_reg_id());
    }

    // Base address of all arguments passed by stack.
    BaseMem base_arg_ptr(sa, int32_t(frame.sa_offset(sa.id())));

    for (uint32_t iter = 0; iter < iter_count; iter++) {
      for (uint32_t var_id = 0; var_id < var_count; var_id++) {
        Var& var = ctx._vars[var_id];
        if (var.is_done()) {
          continue;
        }

        if (var.cur.is_stack()) {
          ASMJIT_ASSERT(var.out.is_reg());

          uint32_t out_id = var.out.reg_id();
          RegType out_type = var.out.reg_type();

          RegGroup group = RegUtils::group_of(out_type);
          WorkData& wd = work_data[group];

          if (out_id == sa.id() && group == RegGroup::kGp) {
            // This register will be processed last as we still need `sa_reg_id`.
            if (iter_count == 1) {
              iter_count++;
              continue;
            }
            wd.unassign(wd._phys_to_var_id[out_id], out_id);
          }

          Reg dst_reg = Reg(RegUtils::signature_of(out_type), out_id);
          BaseMem src_mem = base_arg_ptr.clone_adjusted(var.cur.stack_offset());

          ASMJIT_PROPAGATE(emit_arg_move(
            dst_reg, var.out.type_id(),
            src_mem, var.cur.type_id()));

          wd.assign(var_id, out_id);
          var.cur.init_reg(out_type, out_id, var.cur.type_id(), FuncValue::kFlagIsDone);
        }
      }
    }
  }

  return Error::kOk;
}

ASMJIT_END_NAMESPACE
