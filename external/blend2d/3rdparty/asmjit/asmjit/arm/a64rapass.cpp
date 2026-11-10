// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#if !defined(ASMJIT_NO_AARCH64) && !defined(ASMJIT_NO_COMPILER)

#include <asmjit/core/cpuinfo.h>
#include <asmjit/core/formatter_p.h>
#include <asmjit/core/type.h>
#include <asmjit/support/support.h>
#include <asmjit/arm/a64assembler.h>
#include <asmjit/arm/a64compiler.h>
#include <asmjit/arm/a64emithelper_p.h>
#include <asmjit/arm/a64instapi_p.h>
#include <asmjit/arm/a64instdb_p.h>
#include <asmjit/arm/a64rapass_p.h>

ASMJIT_BEGIN_SUB_NAMESPACE(a64)

// a64::ARMRAPass - Helpers
// ========================

// TODO: [ARM] These should be shared with all backends.
[[maybe_unused]]
static inline uint64_t ra_imm_mask_from_size(uint32_t size) noexcept {
  ASMJIT_ASSERT(size > 0 && size < 256);
  static const uint64_t masks[] = {
    0x00000000000000FFu, //   1
    0x000000000000FFFFu, //   2
    0x00000000FFFFFFFFu, //   4
    0xFFFFFFFFFFFFFFFFu, //   8
    0x0000000000000000u, //  16
    0x0000000000000000u, //  32
    0x0000000000000000u, //  64
    0x0000000000000000u, // 128
    0x0000000000000000u  // 256
  };
  return masks[Support::ctz(size)];
}

static const RegMask ra_consecutive_lead_count_to_reg_mask_filter[5] = {
  0xFFFFFFFFu, // [0] No consecutive.
  0x00000000u, // [1] Invalid, never used.
  0x7FFFFFFFu, // [2] 2 consecutive registers.
  0x3FFFFFFFu, // [3] 3 consecutive registers.
  0x1FFFFFFFu  // [4] 4 consecutive registers.
};

[[nodiscard]]
static inline RATiedFlags ra_use_out_flags_from_rw_flags(OpRWFlags rw_flags) noexcept {
  static constexpr RATiedFlags map[] = {
    RATiedFlags::kNone,
    RATiedFlags::kRead  | RATiedFlags::kUse, // kRead
    RATiedFlags::kWrite | RATiedFlags::kOut, // kWrite
    RATiedFlags::kRW    | RATiedFlags::kUse, // kRW
  };

  return map[uint32_t(rw_flags & OpRWFlags::kRW)];
}

[[nodiscard]]
static inline RATiedFlags ra_reg_rw_flags(OpRWFlags flags) noexcept {
  return ra_use_out_flags_from_rw_flags(flags);
}

[[nodiscard]]
static inline RATiedFlags ra_mem_base_rw_flags(OpRWFlags flags) noexcept {
  constexpr uint32_t shift = Support::ctz_const<OpRWFlags::kMemBaseRW>;
  return ra_use_out_flags_from_rw_flags(OpRWFlags(uint32_t(flags) >> shift) & OpRWFlags::kRW);
}

[[nodiscard]]
static inline RATiedFlags ra_mem_index_rw_flags(OpRWFlags flags) noexcept {
  constexpr uint32_t shift = Support::ctz_const<OpRWFlags::kMemIndexRW>;
  return ra_use_out_flags_from_rw_flags(OpRWFlags(uint32_t(flags) >> shift) & OpRWFlags::kRW);
}
// a64::RACFGBuilder
// =================

class RACFGBuilder : public RACFGBuilderT<RACFGBuilder> {
public:
  Arch _arch;

  inline RACFGBuilder(ARMRAPass& pass) noexcept
    : RACFGBuilderT<RACFGBuilder>(pass),
      _arch(pass.cc().arch()) {}

  [[nodiscard]]
  inline Compiler& cc() const noexcept { return static_cast<Compiler&>(_cc); }

  [[nodiscard]]
  Error on_instruction(InstNode* inst, InstControlFlow& control_type, RAInstBuilder& ib) noexcept;

  [[nodiscard]]
  Error on_before_invoke(InvokeNode* invoke_node) noexcept;

  [[nodiscard]]
  Error on_invoke(InvokeNode* invoke_node, RAInstBuilder& ib) noexcept;

  [[nodiscard]]
  Error move_imm_to_reg_arg(InvokeNode* invoke_node, const FuncValue& arg, const Imm& imm_, Out<Reg> out) noexcept;

  [[nodiscard]]
  Error move_imm_to_stack_arg(InvokeNode* invoke_node, const FuncValue& arg, const Imm& imm_) noexcept;

  [[nodiscard]]
  Error move_reg_to_stack_arg(InvokeNode* invoke_node, const FuncValue& arg, const Reg& reg) noexcept;

  [[nodiscard]]
  Error on_before_ret(FuncRetNode* func_ret) noexcept;

  [[nodiscard]]
  Error on_ret(FuncRetNode* func_ret, RAInstBuilder& ib) noexcept;
};

// a64::RACFGBuilder - OnInst
// ==========================

// TODO: [ARM] This is just a workaround...
static InstControlFlow get_control_flow_type(InstId inst_id) noexcept {
  switch (BaseInst::extract_real_id(inst_id)) {
    case Inst::kIdB:
    case Inst::kIdBr:
      if (BaseInst::extract_arm_cond_code(inst_id) == CondCode::kAL) {
        return InstControlFlow::kJump;
      }
      else {
        return InstControlFlow::kBranch;
      }

    case Inst::kIdBl:
    case Inst::kIdBlr:
      return InstControlFlow::kCall;

    case Inst::kIdCbz:
    case Inst::kIdCbnz:
    case Inst::kIdTbz:
    case Inst::kIdTbnz:
      return InstControlFlow::kBranch;

    case Inst::kIdRet:
      return InstControlFlow::kReturn;

    default:
      return InstControlFlow::kRegular;
  }
}

Error RACFGBuilder::on_instruction(InstNode* inst, InstControlFlow& control_type, RAInstBuilder& ib) noexcept {
  InstRWInfo rw_info;

  if (Inst::is_defined_id(inst->real_id())) {
    InstId inst_id = inst->inst_id();
    Span<const Operand> operands = inst->operands();

    ASMJIT_PROPAGATE(InstInternal::query_rw_info(inst->baseInst(), operands.data(), operands.size(), &rw_info));

    const InstDB::InstInfo& inst_info = InstDB::inst_info_by_id(inst_id);
    uint32_t single_reg_ops = 0;

    ib.add_inst_rw_flags(rw_info.inst_flags());

    if (!operands.is_empty()) {
      uint32_t consecutive_offset = 0xFFFFFFFFu;
      RAWorkReg* consecutive_parent = nullptr;

      for (size_t i = 0u; i < operands.size(); i++) {
        const Operand& op = operands[i];
        const OpRWInfo& op_rw_info = rw_info.operand(i);

        if (op.is_reg()) {
          // Register Operand
          // ----------------
          const Reg& reg = op.as<Reg>();

          RATiedFlags flags = ra_reg_rw_flags(op_rw_info.op_flags());
          uint32_t virt_index = Operand::virt_id_to_index(reg.id());

          if (virt_index < Operand::kVirtIdCount) {
            RAWorkReg* work_reg;
            ASMJIT_PROPAGATE(_pass.virt_index_as_work_reg(&work_reg, virt_index));

            // Use RW instead of Write in case that not the whole register is overwritten. This is important for
            // liveness as we cannot kill a register that will be used.
            if ((flags & RATiedFlags::kRW) == RATiedFlags::kWrite) {
              if (work_reg->reg_byte_mask() & ~(op_rw_info.write_byte_mask() | op_rw_info.extend_byte_mask())) {
                // Not write-only operation.
                flags = (flags & ~RATiedFlags::kOut) | (RATiedFlags::kRead | RATiedFlags::kUse);
              }
            }

            RegGroup group = work_reg->group();

            RegMask use_regs = _pass._available_regs[group];
            RegMask out_regs = use_regs;

            uint32_t use_id = Reg::kIdBad;
            uint32_t out_id = Reg::kIdBad;

            uint32_t use_rewrite_mask = 0;
            uint32_t out_rewrite_mask = 0;

            if (op_rw_info.consecutive_lead_count()) {
              // There must be a single consecutive register lead, otherwise the RW data is invalid.
              if (consecutive_offset != 0xFFFFFFFFu) {
                return make_error(Error::kInvalidState);
              }

              // A consecutive lead register cannot be used as a consecutive +1/+2/+3 register, the registers must be distinct.
              if (RATiedReg::consecutive_data_from_flags(flags) != 0) {
                return make_error(Error::kNotConsecutiveRegs);
              }

              flags |= RATiedFlags::kLeadConsecutive | RATiedReg::consecutive_data_to_flags(op_rw_info.consecutive_lead_count() - 1);
              consecutive_offset = 0;

              RegMask filter = ra_consecutive_lead_count_to_reg_mask_filter[op_rw_info.consecutive_lead_count()];
              if (Support::test(flags, RATiedFlags::kUse)) {
                flags |= RATiedFlags::kUseConsecutive;
                use_regs &= filter;
              }
              else {
                flags |= RATiedFlags::kOutConsecutive;
                out_regs &= filter;
              }
            }

            if (Support::test(flags, RATiedFlags::kUse)) {
              use_rewrite_mask = Support::bit_mask<uint32_t>(inst->_get_rewrite_index(&reg._base_id));
              if (op_rw_info.has_op_flag(OpRWFlags::kRegPhysId)) {
                use_id = op_rw_info.phys_id();
                flags |= RATiedFlags::kUseFixed;
              }
              else if (op_rw_info.has_op_flag(OpRWFlags::kConsecutive)) {
                if (consecutive_offset == 0xFFFFFFFFu) {
                  return make_error(Error::kInvalidState);
                }
                flags |= RATiedFlags::kUseConsecutive | RATiedReg::consecutive_data_to_flags(++consecutive_offset);
              }
            }
            else {
              out_rewrite_mask = Support::bit_mask<uint32_t>(inst->_get_rewrite_index(&reg._base_id));
              if (op_rw_info.has_op_flag(OpRWFlags::kRegPhysId)) {
                out_id = op_rw_info.phys_id();
                flags |= RATiedFlags::kOutFixed;
              }
              else if (op_rw_info.has_op_flag(OpRWFlags::kConsecutive)) {
                if (consecutive_offset == 0xFFFFFFFFu) {
                  return make_error(Error::kInvalidState);
                }
                flags |= RATiedFlags::kOutConsecutive | RATiedReg::consecutive_data_to_flags(++consecutive_offset);
              }
            }

            // Special cases regarding element access.
            if (reg.as<Vec>().has_element_index()) {
              // Only the first 0..15 registers can be used if the register uses
              // element accessor that accesses half-words (h[0..7] elements).
              if (inst_info.has_flag(InstDB::kInstFlagVH0_15) && reg.as<Vec>().element_type() == VecElementType::kH) {
                if (Support::test(flags, RATiedFlags::kUse)) {
                  use_id &= 0x0000FFFFu;
                }
                else {
                  out_id &= 0x0000FFFFu;
                }
              }
            }

            ASMJIT_PROPAGATE(ib.add(work_reg, flags, use_regs, use_id, use_rewrite_mask, out_regs, out_id, out_rewrite_mask, op_rw_info.rm_size(), consecutive_parent));
            if (single_reg_ops == i) {
              single_reg_ops++;
            }

            if (Support::test(flags, RATiedFlags::kLeadConsecutive | RATiedFlags::kUseConsecutive | RATiedFlags::kOutConsecutive)) {
              consecutive_parent = work_reg;
            }
          }
        }
        else if (op.is_mem()) {
          // Memory Operand
          // --------------
          const Mem& mem = op.as<Mem>();

          if (mem.is_reg_home()) {
            RAWorkReg* work_reg;
            ASMJIT_PROPAGATE(_pass.virt_index_as_work_reg(&work_reg, Operand::virt_id_to_index(mem.base_id())));
            if (ASMJIT_UNLIKELY(!_pass.get_or_create_stack_slot(work_reg))) {
              return make_error(Error::kOutOfMemory);
            }
          }
          else if (mem.has_base_reg()) {
            uint32_t virt_index = Operand::virt_id_to_index(mem.base_id());
            if (virt_index < Operand::kVirtIdCount) {
              RAWorkReg* work_reg;
              ASMJIT_PROPAGATE(_pass.virt_index_as_work_reg(&work_reg, virt_index));

              RATiedFlags flags = ra_mem_base_rw_flags(op_rw_info.op_flags());
              RegGroup group = work_reg->group();
              RegMask allocable = _pass._available_regs[group];

              // Base registers have never fixed id on ARM.
              const uint32_t use_id = Reg::kIdBad;
              const uint32_t out_id = Reg::kIdBad;

              uint32_t use_rewrite_mask = 0;
              uint32_t out_rewrite_mask = 0;

              if (Support::test(flags, RATiedFlags::kUse)) {
                use_rewrite_mask = Support::bit_mask<uint32_t>(inst->_get_rewrite_index(&mem._base_id));
              }
              else {
                out_rewrite_mask = Support::bit_mask<uint32_t>(inst->_get_rewrite_index(&mem._base_id));
              }

              ASMJIT_PROPAGATE(ib.add(work_reg, flags, allocable, use_id, use_rewrite_mask, allocable, out_id, out_rewrite_mask));
            }
          }

          if (mem.has_index_reg()) {
            uint32_t virt_index = Operand::virt_id_to_index(mem.index_id());
            if (virt_index < Operand::kVirtIdCount) {
              RAWorkReg* work_reg;
              ASMJIT_PROPAGATE(_pass.virt_index_as_work_reg(&work_reg, virt_index));

              RATiedFlags flags = ra_mem_index_rw_flags(op_rw_info.op_flags());
              RegGroup group = work_reg->group();
              RegMask allocable = _pass._available_regs[group];

              // Index registers have never fixed id on ARM.
              const uint32_t use_id = Reg::kIdBad;
              const uint32_t out_id = Reg::kIdBad;

              uint32_t use_rewrite_mask = 0;
              uint32_t out_rewrite_mask = 0;

              if (Support::test(flags, RATiedFlags::kUse)) {
                use_rewrite_mask = Support::bit_mask<uint32_t>(inst->_get_rewrite_index(&mem._data[Operand::kDataMemIndexId]));
              }
              else {
                out_rewrite_mask = Support::bit_mask<uint32_t>(inst->_get_rewrite_index(&mem._data[Operand::kDataMemIndexId]));
              }

              ASMJIT_PROPAGATE(ib.add(work_reg, RATiedFlags::kUse | RATiedFlags::kRead, allocable, use_id, use_rewrite_mask, allocable, out_id, out_rewrite_mask));
            }
          }
        }
      }
    }

    control_type = get_control_flow_type(inst_id);
  }

  return Error::kOk;
}

// a64::RACFGBuilder - OnInvoke
// ============================

Error RACFGBuilder::on_before_invoke(InvokeNode* invoke_node) noexcept {
  const FuncDetail& fd = invoke_node->detail();
  uint32_t arg_count = invoke_node->arg_count();

  cc().set_cursor(invoke_node->prev());

  for (uint32_t arg_index = 0; arg_index < arg_count; arg_index++) {
    const FuncValuePack& arg_pack = fd.arg_pack(arg_index);
    for (uint32_t value_index = 0; value_index < Globals::kMaxValuePack; value_index++) {
      if (!arg_pack[value_index])
        break;

      const FuncValue& arg = arg_pack[value_index];
      const Operand& op = invoke_node->arg(arg_index, value_index);

      if (op.is_none())
        continue;

      if (op.is_reg()) {
        const Reg& reg = op.as<Reg>();
        RAWorkReg* work_reg;
        ASMJIT_PROPAGATE(_pass.virt_index_as_work_reg(&work_reg, Operand::virt_id_to_index(reg.id())));

        if (arg.is_reg()) {
          RegGroup reg_group = work_reg->group();
          RegGroup arg_group = RegUtils::group_of(arg.reg_type());

          if (reg_group != arg_group) {
            // TODO: [ARM] Conversion is not supported.
            return make_error(Error::kInvalidAssignment);
          }
        }
        else {
          ASMJIT_PROPAGATE(move_reg_to_stack_arg(invoke_node, arg, reg));
        }
      }
      else if (op.is_imm()) {
        if (arg.is_reg()) {
          Reg reg;
          ASMJIT_PROPAGATE(move_imm_to_reg_arg(invoke_node, arg, op.as<Imm>(), Out(reg)));
          invoke_node->_args[arg_index][value_index] = reg;
        }
        else {
          ASMJIT_PROPAGATE(move_imm_to_stack_arg(invoke_node, arg, op.as<Imm>()));
        }
      }
    }
  }

  cc().set_cursor(invoke_node);

  if (fd.has_ret()) {
    for (uint32_t value_index = 0; value_index < Globals::kMaxValuePack; value_index++) {
      const FuncValue& ret = fd.ret(value_index);
      if (!ret) {
        break;
      }

      const Operand& op = invoke_node->ret(value_index);
      if (op.is_reg()) {
        const Reg& reg = op.as<Reg>();
        RAWorkReg* work_reg;
        ASMJIT_PROPAGATE(_pass.virt_index_as_work_reg(&work_reg, Operand::virt_id_to_index(reg.id())));

        if (ret.is_reg()) {
          RegGroup reg_group = work_reg->group();
          RegGroup ret_group = RegUtils::group_of(ret.reg_type());

          if (reg_group != ret_group) {
            // TODO: [ARM] Conversion is not supported.
            return make_error(Error::kInvalidAssignment);
          }
        }
      }
    }
  }

  // This block has function call(s).
  _cur_block->add_flags(RABlockFlags::kHasFuncCalls);
  _pass.func()->frame().add_attributes(FuncAttributes::kHasFuncCalls);
  _pass.func()->frame().update_call_stack_size(fd.arg_stack_size());

  return Error::kOk;
}

Error RACFGBuilder::on_invoke(InvokeNode* invoke_node, RAInstBuilder& ib) noexcept {
  uint32_t arg_count = invoke_node->arg_count();
  const FuncDetail& fd = invoke_node->detail();

  for (uint32_t arg_index = 0; arg_index < arg_count; arg_index++) {
    const FuncValuePack& arg_pack = fd.arg_pack(arg_index);
    for (uint32_t value_index = 0; value_index < Globals::kMaxValuePack; value_index++) {
      if (!arg_pack[value_index]) {
        continue;
      }

      const FuncValue& arg = arg_pack[value_index];
      const Operand& op = invoke_node->arg(arg_index, value_index);

      if (op.is_none()) {
        continue;
      }

      if (op.is_reg()) {
        const Reg& reg = op.as<Reg>();
        RAWorkReg* work_reg;
        ASMJIT_PROPAGATE(_pass.virt_index_as_work_reg(&work_reg, Operand::virt_id_to_index(reg.id())));

        if (arg.is_indirect()) {
          RegGroup reg_group = work_reg->group();
          if (reg_group != RegGroup::kGp) {
            return make_error(Error::kInvalidState);
          }
          ASMJIT_PROPAGATE(ib.add_call_arg(work_reg, arg.reg_id()));
        }
        else if (arg.is_reg()) {
          RegGroup reg_group = work_reg->group();
          RegGroup arg_group = RegUtils::group_of(arg.reg_type());

          if (reg_group == arg_group) {
            ASMJIT_PROPAGATE(ib.add_call_arg(work_reg, arg.reg_id()));
          }
        }
      }
    }
  }

  for (uint32_t ret_index = 0; ret_index < Globals::kMaxValuePack; ret_index++) {
    const FuncValue& ret = fd.ret(ret_index);
    if (!ret) {
      break;
    }

    const Operand& op = invoke_node->ret(ret_index);
    if (op.is_reg()) {
      const Reg& reg = op.as<Reg>();
      RAWorkReg* work_reg;
      ASMJIT_PROPAGATE(_pass.virt_index_as_work_reg(&work_reg, Operand::virt_id_to_index(reg.id())));

      if (ret.is_reg()) {
        RegGroup reg_group = work_reg->group();
        RegGroup ret_group = RegUtils::group_of(ret.reg_type());

        if (reg_group == ret_group) {
          ASMJIT_PROPAGATE(ib.add_call_ret(work_reg, ret.reg_id()));
        }
      }
      else {
        return make_error(Error::kInvalidAssignment);
      }
    }
  }

  // Setup clobbered registers.
  ib._clobbered[0] = Support::lsb_mask<RegMask>(_pass._phys_reg_count.get(RegGroup(0))) & ~fd.preserved_regs(RegGroup(0));
  ib._clobbered[1] = Support::lsb_mask<RegMask>(_pass._phys_reg_count.get(RegGroup(1))) & ~fd.preserved_regs(RegGroup(1));
  ib._clobbered[2] = Support::lsb_mask<RegMask>(_pass._phys_reg_count.get(RegGroup(2))) & ~fd.preserved_regs(RegGroup(2));
  ib._clobbered[3] = Support::lsb_mask<RegMask>(_pass._phys_reg_count.get(RegGroup(3))) & ~fd.preserved_regs(RegGroup(3));

  return Error::kOk;
}

// a64::RACFGBuilder - MoveImmToRegArg
// ===================================

Error RACFGBuilder::move_imm_to_reg_arg(InvokeNode* invoke_node, const FuncValue& arg, const Imm& imm_, Out<Reg> out) noexcept {
  Support::maybe_unused(invoke_node);
  ASMJIT_ASSERT(arg.is_reg());

  Imm imm(imm_);
  TypeId type_id = TypeId::kVoid;

  switch (arg.type_id()) {
    case TypeId::kInt8  : type_id = TypeId::kUInt64; imm.sign_extend_int8(); break;
    case TypeId::kUInt8 : type_id = TypeId::kUInt64; imm.zero_extend_uint8(); break;
    case TypeId::kInt16 : type_id = TypeId::kUInt64; imm.sign_extend_int16(); break;
    case TypeId::kUInt16: type_id = TypeId::kUInt64; imm.zero_extend_uint16(); break;
    case TypeId::kInt32 : type_id = TypeId::kUInt64; imm.sign_extend_int32(); break;
    case TypeId::kUInt32: type_id = TypeId::kUInt64; imm.zero_extend_uint32(); break;
    case TypeId::kInt64 : type_id = TypeId::kUInt64; break;
    case TypeId::kUInt64: type_id = TypeId::kUInt64; break;

    default:
      return make_error(Error::kInvalidAssignment);
  }

  ASMJIT_PROPAGATE(cc()._new_reg(out, type_id, nullptr));
  cc().virt_reg_by_id(out->id())->set_weight(BaseRAPass::kCallArgWeight);
  return cc().mov(out->as<Gp>(), imm);
}

// a64::RACFGBuilder - MoveImmToStackArg
// =====================================

Error RACFGBuilder::move_imm_to_stack_arg(InvokeNode* invoke_node, const FuncValue& arg, const Imm& imm_) noexcept {
  Reg reg;

  ASMJIT_PROPAGATE(move_imm_to_reg_arg(invoke_node, arg, imm_, Out(reg)));
  ASMJIT_PROPAGATE(move_reg_to_stack_arg(invoke_node, arg, reg));

  return Error::kOk;
}

// a64::RACFGBuilder - MoveRegToStackArg
// =====================================

Error RACFGBuilder::move_reg_to_stack_arg(InvokeNode* invoke_node, const FuncValue& arg, const Reg& reg) noexcept {
  Support::maybe_unused(invoke_node);
  Mem stack_ptr = ptr(_pass._sp.as<Gp>(), arg.stack_offset());

  if (reg.is_gp()) {
    return cc().str(reg.as<Gp>(), stack_ptr);
  }

  if (reg.is_vec()) {
    return cc().str(reg.as<Vec>(), stack_ptr);
  }

  return make_error(Error::kInvalidState);
}

// a64::RACFGBuilder - OnReg
// =========================

Error RACFGBuilder::on_before_ret(FuncRetNode* func_ret) noexcept {
  Support::maybe_unused(func_ret);
  return Error::kOk;
}

Error RACFGBuilder::on_ret(FuncRetNode* func_ret, RAInstBuilder& ib) noexcept {
  const FuncDetail& func_detail = _pass.func()->detail();
  Span<const Operand> operands = func_ret->operands();

  for (size_t i = 0; i < operands.size(); i++) {
    const Operand& op = operands[i];
    if (op.is_none()) {
      continue;
    }

    const FuncValue& ret = func_detail.ret(i);
    if (ASMJIT_UNLIKELY(!ret.is_reg())) {
      return make_error(Error::kInvalidAssignment);
    }

    if (op.is_reg()) {
      // Register return value.
      const Reg& reg = op.as<Reg>();
      uint32_t virt_index = Operand::virt_id_to_index(reg.id());

      if (virt_index < Operand::kVirtIdCount) {
        RAWorkReg* work_reg;
        ASMJIT_PROPAGATE(_pass.virt_index_as_work_reg(&work_reg, virt_index));

        RegGroup group = work_reg->group();
        RegMask allocable = _pass._available_regs[group];
        ASMJIT_PROPAGATE(ib.add(work_reg, RATiedFlags::kUse | RATiedFlags::kRead, allocable, ret.reg_id(), 0, 0, Reg::kIdBad, 0));
      }
    }
    else {
      return make_error(Error::kInvalidAssignment);
    }
  }

  return Error::kOk;
}

// a64::ARMRAPass - Construction & Destruction
// ===========================================

ARMRAPass::ARMRAPass(BaseCompiler& cc) noexcept
  : BaseRAPass(cc) { _emit_helper_ptr = &_emit_helper; }
ARMRAPass::~ARMRAPass() noexcept {}

// a64::ARMRAPass - OnInit / OnDone
// ================================

void ARMRAPass::on_init() noexcept {
  Arch arch = cc().arch();

  _emit_helper.reset(&_cb);
  _arch_traits = &ArchTraits::by_arch(arch);
  _phys_reg_count.set(RegGroup::kGp, 32);
  _phys_reg_count.set(RegGroup::kVec, 32);
  _phys_reg_count.set(RegGroup::kMask, 0);
  _phys_reg_count.set(RegGroup::kExtra, 0);
  _build_phys_index();

  _available_regs[RegGroup::kGp] = Support::lsb_mask<uint32_t>(_phys_reg_count.get(RegGroup::kGp));
  _available_regs[RegGroup::kVec] = Support::lsb_mask<uint32_t>(_phys_reg_count.get(RegGroup::kVec));
  _available_regs[RegGroup::kMask] = Support::lsb_mask<uint32_t>(_phys_reg_count.get(RegGroup::kMask));
  _available_regs[RegGroup::kExtra] = Support::lsb_mask<uint32_t>(_phys_reg_count.get(RegGroup::kExtra));

  _scratch_reg_indexes[0] = uint8_t(27);
  _scratch_reg_indexes[1] = uint8_t(28);

  const FuncFrame& frame = _func->frame();

  // The architecture specific setup makes implicitly all registers available. So
  // make unavailable all registers that are special and cannot be used in general.
  bool has_preserved_fp = frame.has_preserved_fp();

  // Apple ABI requires that the frame-pointer register is not changed by leaf functions and properly updated
  // by non-leaf functions. So, let's make this register unavailable as it's just not safe to update it.
  if (has_preserved_fp || cc().environment().is_darwin_abi()) {
    make_unavailable(RegGroup::kGp, Gp::kIdFp);
  }
  make_unavailable(RegGroup::kGp, Gp::kIdSp);
  make_unavailable(RegGroup::kGp, Gp::kIdOs); // OS-specific use, usually TLS.
  make_unavailable(frame._unavailable_regs);

  _sp = sp;
  _fp = x29;
}

void ARMRAPass::on_done() noexcept {}

// a64::ARMRAPass - BuildCFG
// =========================

Error ARMRAPass::build_cfg_nodes() noexcept {
  return RACFGBuilder(*this).run();
}

// a64::ARMRAPass - Rewrite
// ========================

ASMJIT_FAVOR_SPEED Error ARMRAPass::rewrite() noexcept {
  const size_t virt_count = cc()._virt_regs.size();
  return rewrite_iterate([&](BaseNode* node, BaseNode* stop, RABlock* block) noexcept -> Error {
    while (node != stop) {
      BaseNode* next = node->next();

      if (node->is_inst()) {
        InstNode* inst = node->as<InstNode>();
        RAInst* ra_inst = node->pass_data<RAInst>();

        Span<Operand> operands = inst->operands();

        // Rewrite virtual registers into physical registers.
        if (ra_inst) {
          // This data is allocated by Arena passed to `run_on_function()`, which will be reset after the RA pass
          // finishes. So reset this data to prevent having a dead pointer after the RA pass is complete.
          node->reset_pass_data();

          // If the instruction contains pass data (ra_inst) then it was a subject for register allocation and must be
          // rewritten to use physical regs.
          const RATiedReg* tied_regs = ra_inst->tied_regs();
          uint32_t tied_count = ra_inst->tied_count();

          for (uint32_t i = 0; i < tied_count; i++) {
            const RATiedReg& tied_reg = tied_regs[i];

            Support::BitWordIterator<uint32_t> use_it(tied_reg.use_rewrite_mask());
            if (use_it.has_next()) {
              uint32_t use_id = tied_reg.use_id();
              do {
                inst->_rewrite_id_at_index(use_it.next(), use_id);
              } while (use_it.has_next());
            }

            Support::BitWordIterator<uint32_t> out_it(tied_reg.out_rewrite_mask());
            if (out_it.has_next()) {
              uint32_t out_id = tied_reg.out_id();
              do {
                inst->_rewrite_id_at_index(out_it.next(), out_id);
              } while (out_it.has_next());
            }
          }

          if (ASMJIT_UNLIKELY(node->type() != NodeType::kInst)) {
            // FuncRet terminates the flow, it must either be removed if the exit
            // label is next to it (optimization) or patched to an architecture
            // dependent jump instruction that jumps to the function's exit before
            // the epilog.
            if (node->type() == NodeType::kFuncRet) {
              if (!is_next_to(node, _func->exit_node())) {
                cc().set_cursor(node->prev());
                ASMJIT_PROPAGATE(emit_jump(_func->exit_node()->label()));
              }

              BaseNode* prev = node->prev();
              cc().remove_node(node);

              if (block) {
                block->set_last(prev);
              }
            }
          }
        }

        // Rewrite stack slot addresses.
        for (Operand& op : operands) {
          if (op.is_mem()) {
            BaseMem& mem = op.as<BaseMem>();
            if (mem.is_reg_home()) {
              uint32_t virt_index = Operand::virt_id_to_index(mem.base_id());
              if (ASMJIT_UNLIKELY(virt_index >= virt_count)) {
                return make_error(Error::kInvalidVirtId);
              }

              VirtReg* virt_reg = cc().virt_reg_by_index(virt_index);
              RAWorkReg* work_reg = virt_reg->work_reg();
              ASMJIT_ASSERT(work_reg != nullptr);

              RAStackSlot* slot = work_reg->stack_slot();
              int32_t offset = slot->offset();

              mem._set_base(_sp.reg_type(), slot->base_reg_id());
              mem.clear_reg_home();
              mem.add_offset_lo32(offset);
            }
          }
        }

        // Rewrite `load_address_of()` construct.
        if (inst->real_id() == Inst::kIdAdr && operands.size() == 2 && operands[1].is_mem()) {
          BaseMem mem = operands[1].as<BaseMem>();
          int64_t offset = mem.offset();

          if (!mem.has_base_or_index()) {
            inst->set_inst_id(Inst::kIdMov);
            inst->set_op(1, Imm(offset));
          }
          else {
            if (mem.has_index()) {
              return make_error(Error::kInvalidAddressIndex);
            }

            Gp dst = Gp::make_r64(operands[0].as<Gp>().id());
            Gp base = Gp::make_r64(mem.base_id());

            InstId arith_inst = offset < 0 ? Inst::kIdSub : Inst::kIdAdd;
            uint64_t abs_offset = offset < 0 ? Support::neg(uint64_t(offset)) : uint64_t(offset);

            inst->set_inst_id(arith_inst);
            inst->set_op_count(3);
            inst->set_op(1, base);
            inst->set_op(2, Imm(abs_offset));

            // Use two operations if the offset cannot be encoded with ADD/SUB.
            if (abs_offset > 0xFFFu && (abs_offset & ~uint64_t(0xFFF000u)) != 0) {
              if (abs_offset <= 0xFFFFFFu) {
                cc().set_cursor(inst->prev());
                ASMJIT_PROPAGATE(cc().emit(arith_inst, dst, base, Imm(abs_offset & 0xFFFu)));

                inst->set_op(1, dst);
                inst->set_op(2, Imm(abs_offset & 0xFFF000u));
              }
              else {
                cc().set_cursor(inst->prev());
                ASMJIT_PROPAGATE(cc().emit(Inst::kIdMov, operands[0], Imm(abs_offset)));

                inst->set_op(1, base);
                inst->set_op(2, dst);
              }
            }
          }
        }
      }

      node = next;
    }

    return Error::kOk;
  });
}

// a64::ARMRAPass - Prolog & Epilog
// ================================

Error ARMRAPass::update_stack_frame() noexcept {
  if (_func->frame().has_func_calls()) {
    _func->frame().add_dirty_regs(RegGroup::kGp, Support::bit_mask<RegMask>(Gp::kIdLr));
  }

  return BaseRAPass::update_stack_frame();
}

// a64::ARMRAPass - OnEmit
// =======================

Error ARMRAPass::emit_move(RAWorkReg* w_reg, uint32_t dst_phys_id, uint32_t src_phys_id) noexcept {
  Reg dst(w_reg->signature(), dst_phys_id);
  Reg src(w_reg->signature(), src_phys_id);

  const char* comment = nullptr;

#ifndef ASMJIT_NO_LOGGING
  if (has_diagnostic_option(DiagnosticOptions::kRAAnnotate)) {
    _tmp_string.clear();
    Formatter::format_virt_reg_name_with_prefix(_tmp_string, "<MOVE> ", 7u, w_reg->virt_reg());
    comment = _tmp_string.data();
  }
#endif

  return _emit_helper.emit_reg_move(dst, src, w_reg->type_id(), comment);
}

Error ARMRAPass::emit_swap(RAWorkReg* a_reg, uint32_t a_phys_id, RAWorkReg* b_reg, uint32_t b_phys_id) noexcept {
  Support::maybe_unused(a_reg, a_phys_id, b_reg, b_phys_id);
  return make_error(Error::kInvalidState);
}

Error ARMRAPass::emit_load(RAWorkReg* w_reg, uint32_t dst_phys_id) noexcept {
  Reg dst_reg(w_reg->signature(), dst_phys_id);
  BaseMem src_mem(work_reg_as_mem(w_reg));

  const char* comment = nullptr;

#ifndef ASMJIT_NO_LOGGING
  if (has_diagnostic_option(DiagnosticOptions::kRAAnnotate)) {
    _tmp_string.clear();
    Formatter::format_virt_reg_name_with_prefix(_tmp_string, "<LOAD> ", 7u, w_reg->virt_reg());
    comment = _tmp_string.data();
  }
#endif

  return _emit_helper.emit_reg_move(dst_reg, src_mem, w_reg->type_id(), comment);
}

Error ARMRAPass::emit_save(RAWorkReg* w_reg, uint32_t src_phys_id) noexcept {
  BaseMem dst_mem(work_reg_as_mem(w_reg));
  Reg src_reg(w_reg->signature(), src_phys_id);

  const char* comment = nullptr;

#ifndef ASMJIT_NO_LOGGING
  if (has_diagnostic_option(DiagnosticOptions::kRAAnnotate)) {
    _tmp_string.clear();
    Formatter::format_virt_reg_name_with_prefix(_tmp_string, "<SAVE> ", 7u, w_reg->virt_reg());
    comment = _tmp_string.data();
  }
#endif

  return _emit_helper.emit_reg_move(dst_mem, src_reg, w_reg->type_id(), comment);
}

Error ARMRAPass::emit_jump(const Label& label) noexcept {
  return cc().b(label);
}

Error ARMRAPass::emit_pre_call(InvokeNode* invoke_node) noexcept {
  Support::maybe_unused(invoke_node);
  return Error::kOk;
}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_AARCH64 && !ASMJIT_NO_COMPILER
