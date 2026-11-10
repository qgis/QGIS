// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#if !defined(ASMJIT_NO_X86) && !defined(ASMJIT_NO_COMPILER)

#include <asmjit/core/cpuinfo.h>
#include <asmjit/core/formatter_p.h>
#include <asmjit/core/type.h>
#include <asmjit/support/support.h>
#include <asmjit/x86/x86assembler.h>
#include <asmjit/x86/x86compiler.h>
#include <asmjit/x86/x86instapi_p.h>
#include <asmjit/x86/x86instdb_p.h>
#include <asmjit/x86/x86emithelper_p.h>
#include <asmjit/x86/x86rapass_p.h>

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

// x86::X86RAPass - Utilities
// ==========================

[[nodiscard]]
static ASMJIT_INLINE uint64_t ra_imm_mask_from_size(uint32_t size) noexcept {
  ASMJIT_ASSERT(size > 0 && size < 256);
  static constexpr uint64_t masks[] = {
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
  0x55555555u, // [2] Even registers.
  0x00000000u, // [3] Invalid, never used.
  0x11111111u  // [4] Every fourth register.
};

[[nodiscard]]
static ASMJIT_INLINE RATiedFlags ra_use_out_flags_from_rw_flags(OpRWFlags rw_flags) noexcept {
  static constexpr RATiedFlags map[] = {
    RATiedFlags::kNone,
    RATiedFlags::kRead  | RATiedFlags::kUse,                       // kRead
    RATiedFlags::kWrite | RATiedFlags::kOut,                       // kWrite
    RATiedFlags::kRW    | RATiedFlags::kUse,                       // kRW
    RATiedFlags::kNone,
    RATiedFlags::kRead  | RATiedFlags::kUse | RATiedFlags::kUseRM, // kRead  | kRegMem
    RATiedFlags::kWrite | RATiedFlags::kOut | RATiedFlags::kOutRM, // kWrite | kRegMem
    RATiedFlags::kRW    | RATiedFlags::kUse | RATiedFlags::kUseRM  // kRW    | kRegMem
  };

  return map[uint32_t(rw_flags & (OpRWFlags::kRW | OpRWFlags::kRegMem))];
}

[[nodiscard]]
static ASMJIT_INLINE RATiedFlags ra_reg_rw_flags(OpRWFlags flags) noexcept {
  return (RATiedFlags)ra_use_out_flags_from_rw_flags(flags);
}

[[nodiscard]]
static ASMJIT_INLINE RATiedFlags ra_mem_base_rw_flags(OpRWFlags flags) noexcept {
  constexpr uint32_t kShift = Support::ctz_const<OpRWFlags::kMemBaseRW>;
  return (RATiedFlags)ra_use_out_flags_from_rw_flags(OpRWFlags(uint32_t(flags) >> kShift) & OpRWFlags::kRW);
}

[[nodiscard]]
static ASMJIT_INLINE RATiedFlags ra_mem_index_rw_flags(OpRWFlags flags) noexcept {
  constexpr uint32_t kShift = Support::ctz_const<OpRWFlags::kMemIndexRW>;
  return (RATiedFlags)ra_use_out_flags_from_rw_flags(OpRWFlags(uint32_t(flags) >> kShift) & OpRWFlags::kRW);
}

// x86::RACFGBuilder
// =================

class RACFGBuilder : public RACFGBuilderT<RACFGBuilder> {
public:
  Arch _arch;
  bool _is_64bit;
  const EmitHelperInstructionIds& _ids;

  ASMJIT_INLINE_NODEBUG RACFGBuilder(X86RAPass& pass) noexcept
    : RACFGBuilderT<RACFGBuilder>(pass),
      _arch(pass.cc().arch()),
      _is_64bit(pass.register_size() == 8),
      _ids(pass._emit_helper.ids()) {
  }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG X86RAPass& pass() const noexcept { return static_cast<X86RAPass&>(_pass); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Compiler& cc() const noexcept { return static_cast<Compiler&>(_cc); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const EmitHelperInstructionIds& ids() const noexcept { return _ids; }

  [[nodiscard]]
  Error on_instruction(InstNode* inst, InstControlFlow& cf, RAInstBuilder& ib) noexcept;

  [[nodiscard]]
  Error on_before_invoke(InvokeNode* invoke_node) noexcept;

  [[nodiscard]]
  Error on_invoke(InvokeNode* invoke_node, RAInstBuilder& ib) noexcept;

  [[nodiscard]]
  Error move_vec_to_ptr(InvokeNode* invoke_node, const FuncValue& arg, const Vec& src, Out<Reg> out) noexcept;

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

// x86::RACFGBuilder - OnInst
// ==========================

Error RACFGBuilder::on_instruction(InstNode* inst, InstControlFlow& cf, RAInstBuilder& ib) noexcept {
  InstId inst_id = inst->inst_id();
  InstRWInfo rw_info;

  if (Inst::is_defined_id(inst_id)) {
    Span<const Operand> operands = inst->operands();
    ASMJIT_PROPAGATE(InstInternal::query_rw_info(_arch, inst->baseInst(), operands.data(), operands.size(), &rw_info));

    const InstDB::InstInfo& inst_info = InstDB::inst_info_by_id(inst_id);
    bool has_gpb_hi_constraint = false;
    size_t single_reg_ops = 0;

    // Copy instruction RW flags to instruction builder except kMovOp, which is propagated manually later.
    ib.add_inst_rw_flags(rw_info.inst_flags() & ~InstRWFlags::kMovOp);

    // Mask of all operand types used by the instruction - can be used as an optimization later.
    uint32_t op_types_mask = 0u;

    if (!operands.is_empty()) {
      // The mask is for all registers, but we are mostly interested in AVX-512 registers at the moment. The mask
      // will be combined with all available registers of the Compiler at the end so we it never use more registers
      // than available.
      RegMask instruction_allowed_regs = 0xFFFFFFFFu;

      uint32_t consecutive_offset = 0;
      RAWorkId consecutive_lead_id = kBadWorkId;
      RAWorkReg* consecutive_parent = nullptr;

      if (inst_info.is_evex()) {
        // EVEX instruction and VEX instructions that can be encoded with EVEX have the possibility to use 32 SIMD
        // registers (XMM/YMM/ZMM).
        if (inst_info.is_vex() && !inst_info.is_evex_compatible()) {
          if (inst_info.is_evex_kreg_only()) {
            // EVEX encodable only if the first operand is K register (compare instructions).
            if (!operands[0].is_mask_reg()) {
              instruction_allowed_regs = 0xFFFFu;
            }
          }
          else if (inst_info.is_evex_two_op_only()) {
            // EVEX encodable only if the instruction has two operands (gather instructions).
            if (operands.size() != 2u) {
              instruction_allowed_regs = 0xFFFFu;
            }
          }
          else {
            instruction_allowed_regs = 0xFFFFu;
          }
        }
      }
      else if (inst_info.is_evex_transformable()) {
        ib.add_aggregated_flags(RATiedFlags::kInst_IsTransformable);
      }
      else {
        // Not EVEX, restrict everything to [0-15] registers.
        instruction_allowed_regs = 0xFFFFu;
      }

      for (size_t i = 0u; i < operands.size(); i++) {
        const Operand& op = operands[i];
        const OpRWInfo& op_rw_info = rw_info.operand(i);

        op_types_mask |= 1u << uint32_t(op.op_type());

        if (op.is_reg()) {
          // Register Operand
          // ----------------
          const Reg& reg = op.as<Reg>();

          RATiedFlags flags = ra_reg_rw_flags(op_rw_info.op_flags());
          RegMask allowed_regs = instruction_allowed_regs;

          if (op_rw_info.is_unique()) {
            flags |= RATiedFlags::kUnique;
          }

          // X86-specific constraints related to LO|HI general purpose registers. This is only required when the
          // register is part of the encoding. If the register is fixed we won't restrict anything as it doesn't
          // restrict encoding of other registers.
          if (reg.is_gp8() && !op_rw_info.has_op_flag(OpRWFlags::kRegPhysId)) {
            flags |= RATiedFlags::kX86_Gpb;
            if (!_is_64bit) {
              // Restrict to first four - AL|AH|BL|BH|CL|CH|DL|DH. In 32-bit mode it's not possible to access
              // SIL|DIL, etc, so this is just enough.
              allowed_regs = 0x0Fu;
            }
            else {
              // If we encountered GPB-HI register the situation is much more complicated than in 32-bit mode.
              // We need to patch all registers to not use ID higher than 7 and all GPB-LO registers to not use
              // index higher than 3. Instead of doing the patching here we just set a flag and will do it later,
              // to not complicate this loop.
              if (reg.is_gp8_hi()) {
                has_gpb_hi_constraint = true;
                allowed_regs = 0x0Fu;
              }
            }
          }

          uint32_t virt_index = Operand::virt_id_to_index(reg.id());
          if (virt_index < Operand::kVirtIdCount) {
            RAWorkReg* work_reg;
            ASMJIT_PROPAGATE(_pass.virt_index_as_work_reg(&work_reg, virt_index));

            // Use RW instead of Write in case that not the whole register is overwritten. This is important
            // for liveness as we cannot kill a register that will be used. For example `mov al, 0xFF` is not
            // a write-only operation if user allocated the whole `rax` register.
            if ((flags & RATiedFlags::kRW) == RATiedFlags::kWrite) {
              if (work_reg->reg_byte_mask() & ~(op_rw_info.write_byte_mask() | op_rw_info.extend_byte_mask())) {
                // Not write-only operation.
                flags = (flags & ~RATiedFlags::kOut) | (RATiedFlags::kRead | RATiedFlags::kUse);
              }
            }

            // Do not use RegMem flag if changing Reg to Mem requires a CPU feature that is not available.
            if (rw_info.rm_feature() && Support::test(flags, RATiedFlags::kUseRM | RATiedFlags::kOutRM)) {
              if (!cc().code()->cpu_features().has(rw_info.rm_feature())) {
                flags &= ~(RATiedFlags::kUseRM | RATiedFlags::kOutRM);
              }
            }

            RegGroup group = work_reg->group();
            RegMask use_regs = _pass._available_regs[group] & allowed_regs;
            RegMask out_regs = use_regs;

            uint32_t use_id = Reg::kIdBad;
            uint32_t out_id = Reg::kIdBad;

            uint32_t use_rewrite_mask = 0;
            uint32_t out_rewrite_mask = 0;

            if (op_rw_info.consecutive_lead_count()) {
              // There must be a single consecutive register lead, otherwise the RW data is invalid.
              if (consecutive_lead_id != kBadWorkId) {
                return make_error(Error::kInvalidState);
              }

              // A consecutive lead register cannot be used as a consecutive +1/+2/+3 register, the registers must be distinct.
              if (RATiedReg::consecutive_data_from_flags(flags) != 0) {
                return make_error(Error::kNotConsecutiveRegs);
              }

              flags |= RATiedFlags::kLeadConsecutive | RATiedReg::consecutive_data_to_flags(op_rw_info.consecutive_lead_count() - 1);
              consecutive_lead_id = work_reg->work_id();

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
                if (consecutive_lead_id == kBadWorkId) {
                  return make_error(Error::kInvalidState);
                }

                if (consecutive_lead_id == work_reg->work_id()) {
                  return make_error(Error::kOverlappedRegs);
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
                if (consecutive_lead_id == kBadWorkId) {
                  return make_error(Error::kInvalidState);
                }

                if (consecutive_lead_id == work_reg->work_id()) {
                  return make_error(Error::kOverlappedRegs);
                }

                flags |= RATiedFlags::kOutConsecutive | RATiedReg::consecutive_data_to_flags(++consecutive_offset);
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
          ib.add_forbidden_flags(RATiedFlags::kUseRM | RATiedFlags::kOutRM);

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
              RegMask in_out_regs = _pass._available_regs[group];

              uint32_t use_id = Reg::kIdBad;
              uint32_t out_id = Reg::kIdBad;

              uint32_t use_rewrite_mask = 0;
              uint32_t out_rewrite_mask = 0;

              if (Support::test(flags, RATiedFlags::kUse)) {
                use_rewrite_mask = Support::bit_mask<uint32_t>(inst->_get_rewrite_index(&mem._base_id));
                if (op_rw_info.has_op_flag(OpRWFlags::kMemPhysId)) {
                  use_id = op_rw_info.phys_id();
                  flags |= RATiedFlags::kUseFixed;
                }
              }
              else {
                out_rewrite_mask = Support::bit_mask<uint32_t>(inst->_get_rewrite_index(&mem._base_id));
                if (op_rw_info.has_op_flag(OpRWFlags::kMemPhysId)) {
                  out_id = op_rw_info.phys_id();
                  flags |= RATiedFlags::kOutFixed;
                }
              }

              ASMJIT_PROPAGATE(ib.add(work_reg, flags, in_out_regs, use_id, use_rewrite_mask, in_out_regs, out_id, out_rewrite_mask));
            }
          }

          if (mem.has_index_reg()) {
            uint32_t virt_index = Operand::virt_id_to_index(mem.index_id());
            if (virt_index < Operand::kVirtIdCount) {
              RAWorkReg* work_reg;
              ASMJIT_PROPAGATE(_pass.virt_index_as_work_reg(&work_reg, virt_index));

              RATiedFlags flags = ra_mem_index_rw_flags(op_rw_info.op_flags());
              RegGroup group = work_reg->group();
              RegMask in_out_regs = _pass._available_regs[group] & instruction_allowed_regs;

              // Index registers have never fixed id on X86/x64.
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

              ASMJIT_PROPAGATE(ib.add(work_reg, RATiedFlags::kUse | RATiedFlags::kRead, in_out_regs, use_id, use_rewrite_mask, in_out_regs, out_id, out_rewrite_mask));
            }
          }
        }
      }
    }

    // Handle extra operand (either REP {cx|ecx|rcx} or AVX-512 {k} selector).
    if (inst->has_extra_reg()) {
      uint32_t virt_index = Operand::virt_id_to_index(inst->extra_reg().id());
      if (virt_index < Operand::kVirtIdCount) {
        RAWorkReg* work_reg;
        ASMJIT_PROPAGATE(_pass.virt_index_as_work_reg(&work_reg, virt_index));

        RegGroup group = work_reg->group();
        RegMask in_out_regs = _pass._available_regs[group];
        uint32_t rewrite_mask = Support::bit_mask<uint32_t>(inst->_get_rewrite_index(&inst->extra_reg()._id));

        if (group == RegGroup::kMask) {
          // AVX-512 mask selector {k} register - read-only, allocable to any register except {k0}.
          ASMJIT_PROPAGATE(ib.add(work_reg, RATiedFlags::kUse | RATiedFlags::kRead, in_out_regs, Reg::kIdBad, rewrite_mask, in_out_regs, Reg::kIdBad, 0));
          single_reg_ops = 0u;
        }
        else {
          // REP {cx|ecx|rcx} register - read & write, allocable to {cx|ecx|rcx} only.
          ASMJIT_PROPAGATE(ib.add(work_reg, RATiedFlags::kUse | RATiedFlags::kRW, in_out_regs, Gp::kIdCx, rewrite_mask, in_out_regs, Gp::kIdBad, 0));
        }
      }
      else {
        RegGroup group = inst->extra_reg().group();
        if (group == RegGroup::kMask && inst->extra_reg().id() != 0) {
          single_reg_ops = 0u;
        }
      }
    }

    // If this instruction has move semantics then check whether it could be eliminated if all virtual registers
    // are allocated into the same register. Take into account the virtual size of the destination register as that's
    // more important than a physical register size in this case.
    if (rw_info.has_inst_flag(InstRWFlags::kMovOp) && !inst->has_extra_reg() && Support::bit_test(op_types_mask, uint32_t(OperandType::kReg))) {
      // AVX+ move instructions have 3 operand form - the first two operands must be the same to guarantee move semantics.
      if (operands.size() == 2 || (operands.size() == 3 && operands[0] == operands[1])) {
        uint32_t virt_index = Operand::virt_id_to_index(operands.first().as<Reg>().id());
        if (virt_index < Operand::kVirtIdCount) {
          const VirtReg* virt_reg = _cc.virt_reg_by_index(virt_index);
          const OpRWInfo& op_rw_info = rw_info.operand(0);

          uint64_t remaining_byte_mask = virt_reg->work_reg()->reg_byte_mask() & ~op_rw_info.write_byte_mask();
          if (remaining_byte_mask == 0u || (remaining_byte_mask & op_rw_info.extend_byte_mask()) == 0) {
            ib.add_inst_rw_flags(InstRWFlags::kMovOp);
          }
        }
      }
    }

    // Handle X86 constraints.
    if (has_gpb_hi_constraint) {
      for (RATiedReg& tied_reg : ib) {
        RegMask filter = tied_reg.has_flag(RATiedFlags::kX86_Gpb) ? 0x0Fu : 0xFFu;
        tied_reg._use_reg_mask &= filter;
        tied_reg._out_reg_mask &= filter;
      }
    }

    if (ib.tied_reg_count() == 1) {
      // Handle special cases of some instructions where all operands share the same
      // register. In such case the single operand becomes read-only or write-only.
      InstSameRegHint same_reg_hint = InstSameRegHint::kNone;
      if (single_reg_ops == operands.size()) {
        same_reg_hint = inst_info.same_reg_hint();
      }
      else if (operands.size() == 2 && operands[1].is_imm()) {
        // Handle some tricks used by X86 asm.
        const Reg& reg = operands[0].as<Reg>();
        const Imm& imm = operands[1].as<Imm>();

        const RAWorkReg* work_reg = ib[0]->work_reg();
        uint32_t work_reg_size = work_reg->signature().size();

        switch (inst->inst_id()) {
          case Inst::kIdOr: {
            // Sets the value of the destination register to -1, previous content unused.
            if (reg.size() >= 4 || reg.size() >= work_reg_size) {
              if (imm.value() == -1 || imm.value_as<uint64_t>() == ra_imm_mask_from_size(reg.size())) {
                same_reg_hint = InstSameRegHint::kWO;
              }
            }
            [[fallthrough]];
          }

          case Inst::kIdAdd:
          case Inst::kIdAnd:
          case Inst::kIdRol:
          case Inst::kIdRor:
          case Inst::kIdSar:
          case Inst::kIdShl:
          case Inst::kIdShr:
          case Inst::kIdSub:
          case Inst::kIdXor: {
            // Updates [E|R]FLAGS without changing the content.
            if (reg.size() != 4 || reg.size() >= work_reg_size) {
              if (imm.value() == 0) {
                same_reg_hint = InstSameRegHint::kRO;
              }
            }
            break;
          }
        }
      }
      else if (operands.size() == 4u && operands[3].is_imm()) {
        const Imm& imm = operands[3].as<Imm>();

        switch (inst->inst_id()) {
          case Inst::kIdVpternlogd:
          case Inst::kIdVpternlogq: {
            uint32_t predicate = uint32_t(imm.value() & 0xFFu);
            if (predicate == 0x00u || predicate == 0xFFu) {
              ib[0]->make_write_only();
            }
            break;
          }
        }
      }

      switch (same_reg_hint) {
        case InstSameRegHint::kNone:
          break;
        case InstSameRegHint::kRO:
          ib[0]->make_read_only();
          break;
        case InstSameRegHint::kWO:
          ib[0]->make_write_only();
          break;
      }
    }

    cf = inst_info.control_flow();
  }

  return Error::kOk;
}

// x86::RACFGBuilder - OnInvoke
// ============================

Error RACFGBuilder::on_before_invoke(InvokeNode* invoke_node) noexcept {
  const FuncDetail& fd = invoke_node->detail();
  uint32_t arg_count = invoke_node->arg_count();

  cc().set_cursor(invoke_node->prev());
  RegType native_reg_type = cc()._gp_signature.reg_type();

  for (uint32_t arg_index = 0; arg_index < arg_count; arg_index++) {
    const FuncValuePack& arg_pack = fd.arg_pack(arg_index);
    for (uint32_t value_index = 0; value_index < Globals::kMaxValuePack; value_index++) {
      if (!arg_pack[value_index]) {
        break;
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

        if (arg.is_reg()) {
          RegGroup reg_group = work_reg->group();
          RegGroup arg_group = RegUtils::group_of(arg.reg_type());

          if (arg.is_indirect()) {
            if (reg.is_gp()) {
              if (reg.reg_type() != native_reg_type) {
                return make_error(Error::kInvalidAssignment);
              }
              // It's considered allocated if this is an indirect argument and the user used GP.
              continue;
            }

            Reg indirect_reg;
            ASMJIT_PROPAGATE(move_vec_to_ptr(invoke_node, arg, reg.as<Vec>(), Out(indirect_reg)));
            invoke_node->_args[arg_index][value_index] = indirect_reg;
          }
          else {
            if (reg_group != arg_group) {
              // TODO: Conversion is not supported.
              return make_error(Error::kInvalidAssignment);
            }
          }
        }
        else {
          if (arg.is_indirect()) {
            if (reg.is_gp()) {
              if (reg.reg_type() != native_reg_type) {
                return make_error(Error::kInvalidAssignment);
              }

              ASMJIT_PROPAGATE(move_reg_to_stack_arg(invoke_node, arg, reg));
              continue;
            }

            Reg indirect_reg;
            ASMJIT_PROPAGATE(move_vec_to_ptr(invoke_node, arg, reg.as<Vec>(), Out(indirect_reg)));
            ASMJIT_PROPAGATE(move_reg_to_stack_arg(invoke_node, arg, indirect_reg));
          }
          else {
            ASMJIT_PROPAGATE(move_reg_to_stack_arg(invoke_node, arg, reg));
          }
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
  if (fd.has_flag(CallConvFlags::kCalleePopsStack) && fd.arg_stack_size() != 0) {
    ASMJIT_PROPAGATE(cc().sub(cc().zsp(), fd.arg_stack_size()));
  }

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
          if (ret.reg_type() == RegType::kX86_St) {
            if (work_reg->group() != RegGroup::kVec) {
              return make_error(Error::kInvalidAssignment);
            }

            Reg dst(work_reg->signature(), work_reg->virt_id());
            Mem mem;

            TypeId type_id = TypeUtils::scalar_of(work_reg->type_id());
            if (ret.has_type_id()) {
              type_id = ret.type_id();
            }

            switch (type_id) {
              case TypeId::kFloat32:
                ASMJIT_PROPAGATE(_pass.use_temporary_mem(mem, 4, 4));
                mem.set_size(4);
                ASMJIT_PROPAGATE(cc().fstp(mem));
                ASMJIT_PROPAGATE(cc().emit(ids().movss(), dst.as<Vec>(), mem));
                break;

              case TypeId::kFloat64:
                ASMJIT_PROPAGATE(_pass.use_temporary_mem(mem, 8, 4));
                mem.set_size(8);
                ASMJIT_PROPAGATE(cc().fstp(mem));
                ASMJIT_PROPAGATE(cc().emit(ids().movsd(), dst.as<Vec>(), mem));
                break;

              default:
                return make_error(Error::kInvalidAssignment);
            }
          }
          else {
            RegGroup reg_group = work_reg->group();
            RegGroup ret_group = RegUtils::group_of(ret.reg_type());

            if (reg_group != ret_group) {
              // TODO: Conversion is not supported.
              return make_error(Error::kInvalidAssignment);
            }
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

    // Not handled here...
    const Operand& op = invoke_node->ret(ret_index);
    if (ret.reg_type() == RegType::kX86_St) {
      continue;
    }

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
  for (RegGroup group : Support::enumerate(RegGroup::kMaxVirt)) {
    ib._clobbered[group] = Support::lsb_mask<RegMask>(_pass._phys_reg_count.get(group)) & ~fd.preserved_regs(group);
  }

  return Error::kOk;
}

// x86::RACFGBuilder - MoveVecToPtr
// ================================

static inline OperandSignature vec_reg_signature_by_size(uint32_t size) noexcept {
  return OperandSignature{
    size >= 64 ? RegTraits<RegType::kVec512>::kSignature :
    size >= 32 ? RegTraits<RegType::kVec256>::kSignature :
                 RegTraits<RegType::kVec128>::kSignature
  };
}

Error RACFGBuilder::move_vec_to_ptr(InvokeNode* invoke_node, const FuncValue& arg, const Vec& src, Out<Reg> out) noexcept {
  Support::maybe_unused(invoke_node);
  ASMJIT_ASSERT(arg.is_reg());

  uint32_t arg_size = TypeUtils::size_of(arg.type_id());
  if (arg_size == 0) {
    return make_error(Error::kInvalidState);
  }

  if (arg_size < 16) {
    arg_size = 16;
  }

  uint32_t arg_stack_offset = Support::align_up(invoke_node->detail()._arg_stack_size, arg_size);
  _func_node->frame().update_call_stack_alignment(arg_size);
  invoke_node->detail()._arg_stack_size = arg_stack_offset + arg_size;

  Vec vec_reg(vec_reg_signature_by_size(arg_size), src.id());
  Mem vec_ptr = ptr(_pass._sp.as<Gp>(), int32_t(arg_stack_offset));

  uint32_t vec_mov_inst_id = pass()._emit_helper.ids().movaps();
  if (arg_size > 16) {
    vec_mov_inst_id = Inst::kIdVmovaps;
  }

  ASMJIT_PROPAGATE(cc()._new_reg(out, RegUtils::type_id_of(cc()._gp_signature.reg_type()), nullptr));

  VirtReg* virt_reg = cc().virt_reg_by_id(out->id());
  virt_reg->set_weight(BaseRAPass::kCallArgWeight);

  ASMJIT_PROPAGATE(cc().lea(out->as<Gp>(), vec_ptr));
  ASMJIT_PROPAGATE(cc().emit(vec_mov_inst_id, ptr(out->as<Gp>()), vec_reg));

  if (arg.is_stack()) {
    Mem stack_ptr = ptr(_pass._sp.as<Gp>(), arg.stack_offset());
    ASMJIT_PROPAGATE(cc().mov(stack_ptr, out->as<Gp>()));
  }

  return Error::kOk;
}

// x86::RACFGBuilder - Move Imm to Reg Arg
// =======================================

Error RACFGBuilder::move_imm_to_reg_arg(InvokeNode* invoke_node, const FuncValue& arg, const Imm& imm_, Out<Reg> out) noexcept {
  Support::maybe_unused(invoke_node);
  ASMJIT_ASSERT(arg.is_reg());

  Imm imm(imm_);
  TypeId reg_type_id = TypeId::kUInt32;

  switch (arg.type_id()) {
    case TypeId::kInt8: imm.sign_extend_int8(); goto MovU32;
    case TypeId::kUInt8: imm.zero_extend_uint8(); goto MovU32;
    case TypeId::kInt16: imm.sign_extend_int16(); goto MovU32;
    case TypeId::kUInt16: imm.zero_extend_uint16(); goto MovU32;

    case TypeId::kInt32:
    case TypeId::kUInt32:
MovU32:
      imm.zero_extend_uint32();
      break;

    case TypeId::kInt64:
    case TypeId::kUInt64:
      // Moving to GPD automatically zero extends in 64-bit mode.
      if (imm.is_uint32()) {
        imm.zero_extend_uint32();
        break;
      }

      reg_type_id = TypeId::kUInt64;
      break;

    default:
      return make_error(Error::kInvalidAssignment);
  }

  ASMJIT_PROPAGATE(cc()._new_reg(out, reg_type_id, nullptr));
  cc().virt_reg_by_id(out->id())->set_weight(BaseRAPass::kCallArgWeight);

  return cc().mov(out->as<x86::Gp>(), imm);
}

// x86::RACFGBuilder - Move Imm to Stack Arg
// =========================================

Error RACFGBuilder::move_imm_to_stack_arg(InvokeNode* invoke_node, const FuncValue& arg, const Imm& imm_) noexcept {
  Support::maybe_unused(invoke_node);
  ASMJIT_ASSERT(arg.is_stack());

  Mem stack_ptr = ptr(_pass._sp.as<Gp>(), arg.stack_offset());
  Imm imm[2];

  stack_ptr.set_size(4);
  imm[0] = imm_;
  uint32_t mov_count = 0;

  // One stack entry has the same size as the native register size. That means that if we want to move a 32-bit
  // integer on the stack in 64-bit mode, we need to extend it to a 64-bit integer first. In 32-bit mode, pushing
  // a 64-bit on stack is done in two steps by pushing low and high parts separately.
  switch (arg.type_id()) {
    case TypeId::kInt8: imm[0].sign_extend_int8(); goto MovU32;
    case TypeId::kUInt8: imm[0].zero_extend_uint8(); goto MovU32;
    case TypeId::kInt16: imm[0].sign_extend_int16(); goto MovU32;
    case TypeId::kUInt16: imm[0].zero_extend_uint16(); goto MovU32;

    case TypeId::kInt32:
    case TypeId::kUInt32:
    case TypeId::kFloat32:
MovU32:
      imm[0].zero_extend_uint32();
      mov_count = 1;
      break;

    case TypeId::kInt64:
    case TypeId::kUInt64:
    case TypeId::kFloat64:
    case TypeId::kMmx32:
    case TypeId::kMmx64:
      if (_is_64bit && imm[0].is_int32()) {
        stack_ptr.set_size(8);
        mov_count = 1;
        break;
      }

      imm[1].set_value(imm[0].uint_hi32());
      imm[0].zero_extend_uint32();
      mov_count = 2;
      break;

    default:
      return make_error(Error::kInvalidAssignment);
  }

  for (uint32_t i = 0; i < mov_count; i++) {
    ASMJIT_PROPAGATE(cc().mov(stack_ptr, imm[i]));
    stack_ptr.add_offset_lo32(int32_t(stack_ptr.size()));
  }

  return Error::kOk;
}

// x86::RACFGBuilder - MoveRegToStackArg
// =====================================

Error RACFGBuilder::move_reg_to_stack_arg(InvokeNode* invoke_node, const FuncValue& arg, const Reg& reg) noexcept {
  Support::maybe_unused(invoke_node);
  ASMJIT_ASSERT(arg.is_stack());

  Mem stack_ptr = ptr(_pass._sp.as<Gp>(), arg.stack_offset());
  Reg r0, r1;

  VirtReg* vr = cc().virt_reg_by_id(reg.id());
  uint32_t register_size = cc().register_size();
  InstId inst_id = 0;

  TypeId dst_type_id = arg.type_id();
  TypeId src_type_id = vr->type_id();

  switch (dst_type_id) {
    case TypeId::kInt64:
    case TypeId::kUInt64:
      // Extend BYTE->QWORD (GP).
      if (TypeUtils::is_gp8(src_type_id)) {
        r1.set_reg_t<RegType::kGp8Lo>(reg.id());

        inst_id = (dst_type_id == TypeId::kInt64 && src_type_id == TypeId::kInt8) ? Inst::kIdMovsx : Inst::kIdMovzx;
        goto ExtendMovGpXQ;
      }

      // Extend WORD->QWORD (GP).
      if (TypeUtils::is_gp16(src_type_id)) {
        r1.set_reg_t<RegType::kGp16>(reg.id());

        inst_id = (dst_type_id == TypeId::kInt64 && src_type_id == TypeId::kInt16) ? Inst::kIdMovsx : Inst::kIdMovzx;
        goto ExtendMovGpXQ;
      }

      // Extend DWORD->QWORD (GP).
      if (TypeUtils::is_gp32(src_type_id)) {
        r1.set_reg_t<RegType::kGp32>(reg.id());

        inst_id = Inst::kIdMovsxd;
        if (dst_type_id == TypeId::kInt64 && src_type_id == TypeId::kInt32) {
          goto ExtendMovGpXQ;
        }
        else {
          goto ZeroExtendGpDQ;
        }
      }

      // Move QWORD (GP).
      if (TypeUtils::is_gp64(src_type_id)) goto MovGpQ;
      if (TypeUtils::is_mmx(src_type_id)) goto MovMmQ;
      if (TypeUtils::is_vec(src_type_id)) goto MovXmmQ;
      break;

    case TypeId::kInt32:
    case TypeId::kUInt32:
    case TypeId::kInt16:
    case TypeId::kUInt16:
      // DWORD <- WORD (Zero|Sign Extend).
      if (TypeUtils::is_gp16(src_type_id)) {
        bool is_dst_signed = dst_type_id == TypeId::kInt16 || dst_type_id == TypeId::kInt32;
        bool is_src_signed = src_type_id == TypeId::kInt8  || src_type_id == TypeId::kInt16;

        r1.set_reg_t<RegType::kGp16>(reg.id());
        inst_id = is_dst_signed && is_src_signed ? Inst::kIdMovsx : Inst::kIdMovzx;
        goto ExtendMovGpD;
      }

      // DWORD <- BYTE (Zero|Sign Extend).
      if (TypeUtils::is_gp8(src_type_id)) {
        bool is_dst_signed = dst_type_id == TypeId::kInt16 || dst_type_id == TypeId::kInt32;
        bool is_src_signed = src_type_id == TypeId::kInt8  || src_type_id == TypeId::kInt16;

        r1.set_reg_t<RegType::kGp8Lo>(reg.id());
        inst_id = is_dst_signed && is_src_signed ? Inst::kIdMovsx : Inst::kIdMovzx;
        goto ExtendMovGpD;
      }
      [[fallthrough]];

    case TypeId::kInt8:
    case TypeId::kUInt8:
      if (TypeUtils::is_int(src_type_id)) goto MovGpD;
      if (TypeUtils::is_mmx(src_type_id)) goto MovMmD;
      if (TypeUtils::is_vec(src_type_id)) goto MovXmmD;
      break;

    case TypeId::kMmx32:
    case TypeId::kMmx64:
      // Extend BYTE->QWORD (GP).
      if (TypeUtils::is_gp8(src_type_id)) {
        r1.set_reg_t<RegType::kGp8Lo>(reg.id());

        inst_id = Inst::kIdMovzx;
        goto ExtendMovGpXQ;
      }

      // Extend WORD->QWORD (GP).
      if (TypeUtils::is_gp16(src_type_id)) {
        r1.set_reg_t<RegType::kGp16>(reg.id());

        inst_id = Inst::kIdMovzx;
        goto ExtendMovGpXQ;
      }

      if (TypeUtils::is_gp32(src_type_id)) goto ExtendMovGpDQ;
      if (TypeUtils::is_gp64(src_type_id)) goto MovGpQ;
      if (TypeUtils::is_mmx(src_type_id)) goto MovMmQ;
      if (TypeUtils::is_vec(src_type_id)) goto MovXmmQ;
      break;

    case TypeId::kFloat32:
    case TypeId::kFloat32x1:
      if (TypeUtils::is_vec(src_type_id)) goto MovXmmD;
      break;

    case TypeId::kFloat64:
    case TypeId::kFloat64x1:
      if (TypeUtils::is_vec(src_type_id)) goto MovXmmQ;
      break;

    default:
      if (TypeUtils::is_vec(dst_type_id) && reg.as<Reg>().is_vec()) {
        stack_ptr.set_size(TypeUtils::size_of(dst_type_id));
        uint32_t vec_mov_inst_id = pass()._emit_helper.ids().movaps();

        if (TypeUtils::is_vec128(dst_type_id)) {
          r0.set_reg_t<RegType::kVec128>(reg.id());
        }
        else if (TypeUtils::is_vec256(dst_type_id)) {
          r0.set_reg_t<RegType::kVec256>(reg.id());
        }
        else if (TypeUtils::is_vec512(dst_type_id)) {
          r0.set_reg_t<RegType::kVec512>(reg.id());
        }
        else {
          break;
        }

        return cc().emit(vec_mov_inst_id, stack_ptr, r0);
      }
      break;
  }
  return make_error(Error::kInvalidAssignment);

  // Extend+Move Gp.
ExtendMovGpD:
  stack_ptr.set_size(4);
  r0.set_reg_t<RegType::kGp32>(reg.id());

  ASMJIT_PROPAGATE(cc().emit(inst_id, r0, r1));
  ASMJIT_PROPAGATE(cc().emit(Inst::kIdMov, stack_ptr, r0));
  return Error::kOk;

ExtendMovGpXQ:
  if (register_size == 8) {
    stack_ptr.set_size(8);
    r0.set_reg_t<RegType::kGp64>(reg.id());

    ASMJIT_PROPAGATE(cc().emit(inst_id, r0, r1));
    ASMJIT_PROPAGATE(cc().emit(Inst::kIdMov, stack_ptr, r0));
  }
  else {
    stack_ptr.set_size(4);
    r0.set_reg_t<RegType::kGp32>(reg.id());

    ASMJIT_PROPAGATE(cc().emit(inst_id, r0, r1));

ExtendMovGpDQ:
    ASMJIT_PROPAGATE(cc().emit(Inst::kIdMov, stack_ptr, r0));
    stack_ptr.add_offset_lo32(4);
    ASMJIT_PROPAGATE(cc().emit(Inst::kIdAnd, stack_ptr, 0));
  }
  return Error::kOk;

ZeroExtendGpDQ:
  stack_ptr.set_size(4);
  r0.set_reg_t<RegType::kGp32>(reg.id());
  goto ExtendMovGpDQ;

MovGpD:
  stack_ptr.set_size(4);
  r0.set_reg_t<RegType::kGp32>(reg.id());
  return cc().emit(Inst::kIdMov, stack_ptr, r0);

MovGpQ:
  stack_ptr.set_size(8);
  r0.set_reg_t<RegType::kGp64>(reg.id());
  return cc().emit(Inst::kIdMov, stack_ptr, r0);

MovMmD:
  stack_ptr.set_size(4);
  r0.set_reg_t<RegType::kX86_Mm>(reg.id());
  return cc().emit(ids().movd(), stack_ptr, r0);

MovMmQ:
  stack_ptr.set_size(8);
  r0.set_reg_t<RegType::kX86_Mm>(reg.id());
  return cc().emit(ids().movq(), stack_ptr, r0);

MovXmmD:
  stack_ptr.set_size(4);
  r0.set_reg_t<RegType::kVec128>(reg.id());
  return cc().emit(ids().movss(), stack_ptr, r0);

MovXmmQ:
  stack_ptr.set_size(8);
  r0.set_reg_t<RegType::kVec128>(reg.id());
  return cc().emit(ids().movlps(), stack_ptr, r0);
}

// x86::RACFGBuilder - OnReg
// =========================

Error RACFGBuilder::on_before_ret(FuncRetNode* func_ret) noexcept {
  const FuncDetail& func_detail = _pass.func()->detail();
  Span<const Operand> operands = func_ret->operands();

  cc().set_cursor(func_ret->prev());

  for (size_t i = 0; i < operands.size(); i++) {
    const Operand& op = operands[i];
    const FuncValue& ret = func_detail.ret(i);

    if (!op.is_reg()) {
      continue;
    }

    if (ret.reg_type() == RegType::kX86_St) {
      const Reg& reg = op.as<Reg>();
      uint32_t virt_index = Operand::virt_id_to_index(reg.id());

      if (virt_index < Operand::kVirtIdCount) {
        RAWorkReg* work_reg;
        ASMJIT_PROPAGATE(_pass.virt_index_as_work_reg(&work_reg, virt_index));

        if (work_reg->group() != RegGroup::kVec) {
          return make_error(Error::kInvalidAssignment);
        }

        Reg src(work_reg->signature(), work_reg->virt_id());
        Mem mem;

        TypeId type_id = TypeUtils::scalar_of(work_reg->type_id());
        if (ret.has_type_id()) {
          type_id = ret.type_id();
        }

        switch (type_id) {
          case TypeId::kFloat32:
            ASMJIT_PROPAGATE(_pass.use_temporary_mem(mem, 4, 4));
            mem.set_size(4);
            ASMJIT_PROPAGATE(cc().emit(ids().movss(), mem, src.as<Vec>()));
            ASMJIT_PROPAGATE(cc().fld(mem));
            break;

          case TypeId::kFloat64:
            ASMJIT_PROPAGATE(_pass.use_temporary_mem(mem, 8, 4));
            mem.set_size(8);
            ASMJIT_PROPAGATE(cc().emit(ids().movsd(), mem, src.as<Vec>()));
            ASMJIT_PROPAGATE(cc().fld(mem));
            break;

          default:
            return make_error(Error::kInvalidAssignment);
        }
      }
    }
  }

  return Error::kOk;
}

Error RACFGBuilder::on_ret(FuncRetNode* func_ret, RAInstBuilder& ib) noexcept {
  const FuncDetail& func_detail = _pass.func()->detail();
  Span<const Operand> operands = func_ret->operands();

  for (size_t i = 0u; i < operands.size(); i++) {
    const Operand& op = operands[i];
    if (op.is_none()) {
      continue;
    }

    const FuncValue& ret = func_detail.ret(i);
    if (ASMJIT_UNLIKELY(!ret.is_reg())) {
      return make_error(Error::kInvalidAssignment);
    }

    // Not handled here...
    if (ret.reg_type() == RegType::kX86_St) {
      continue;
    }

    if (op.is_reg()) {
      // Register return value.
      const Reg& reg = op.as<Reg>();
      uint32_t virt_index = Operand::virt_id_to_index(reg.id());

      if (virt_index < Operand::kVirtIdCount) {
        RAWorkReg* work_reg;
        ASMJIT_PROPAGATE(_pass.virt_index_as_work_reg(&work_reg, virt_index));

        RegGroup group = work_reg->group();
        RegMask in_out_regs = _pass._available_regs[group];
        ASMJIT_PROPAGATE(ib.add(work_reg, RATiedFlags::kUse | RATiedFlags::kRead, in_out_regs, ret.reg_id(), 0, in_out_regs, Reg::kIdBad, 0));
      }
    }
    else {
      return make_error(Error::kInvalidAssignment);
    }
  }

  return Error::kOk;
}

// x86::X86RAPass - Construction & Destruction
// ===========================================

X86RAPass::X86RAPass(BaseCompiler& cc) noexcept
  : BaseRAPass(cc) { _emit_helper_ptr = &_emit_helper; }
X86RAPass::~X86RAPass() noexcept {}

// x86::X86RAPass - OnInit & OnDone
// ================================

void X86RAPass::on_init() noexcept {
  Arch arch = cc().arch();
  uint32_t base_reg_count = Environment::is_32bit(arch) ? 8u : 16u;
  uint32_t simd_reg_count = base_reg_count;

  if (Environment::is_64bit(arch) && _func->frame().is_avx512_enabled()) {
    simd_reg_count = 32u;
  }

  _emit_helper.reset(&_cb, _func->frame().is_avx_enabled(), _func->frame().is_avx512_enabled());
  _arch_traits = &ArchTraits::by_arch(arch);
  _phys_reg_count.set(RegGroup::kGp, base_reg_count);
  _phys_reg_count.set(RegGroup::kVec, simd_reg_count);
  _phys_reg_count.set(RegGroup::kMask, 8);
  _phys_reg_count.set(RegGroup::kX86_MM, 8);
  _build_phys_index();

  _available_regs[RegGroup::kGp] = Support::lsb_mask<RegMask>(_phys_reg_count.get(RegGroup::kGp));
  _available_regs[RegGroup::kVec] = Support::lsb_mask<RegMask>(_phys_reg_count.get(RegGroup::kVec));
  _available_regs[RegGroup::kMask] = Support::lsb_mask<RegMask>(_phys_reg_count.get(RegGroup::kMask)) ^ 1u;
  _available_regs[RegGroup::kX86_MM] = Support::lsb_mask<RegMask>(_phys_reg_count.get(RegGroup::kX86_MM));

  _scratch_reg_indexes[0] = uint8_t(Gp::kIdCx);
  _scratch_reg_indexes[1] = uint8_t(base_reg_count - 1);

  const FuncFrame& frame = _func->frame();

  // The architecture specific setup makes implicitly all registers available. So
  // make unavailable all registers that are special and cannot be used in general.
  bool has_fp = frame.has_preserved_fp();

  make_unavailable(RegGroup::kGp, Gp::kIdSp);   // ESP|RSP used as a stack-pointer (SP).
  if (has_fp) {
    make_unavailable(RegGroup::kGp, Gp::kIdBp); // EBP|RBP used as a frame-pointer (FP).
  }
  make_unavailable(frame._unavailable_regs);

  _sp = cc().zsp();
  _fp = cc().zbp();
}

void X86RAPass::on_done() noexcept {}

// x86::X86RAPass - BuildCFG
// =========================

Error X86RAPass::build_cfg_nodes() noexcept {
  return RACFGBuilder(*this).run();
}

// x86::X86RAPass - Rewrite
// ========================

static InstId transform_vex_to_evex(InstId inst_id) {
  switch (inst_id) {
    case Inst::kIdVbroadcastf128: return Inst::kIdVbroadcastf32x4;
    case Inst::kIdVbroadcasti128: return Inst::kIdVbroadcasti32x4;
    case Inst::kIdVextractf128: return Inst::kIdVextractf32x4;
    case Inst::kIdVextracti128: return Inst::kIdVextracti32x4;
    case Inst::kIdVinsertf128: return Inst::kIdVinsertf32x4;
    case Inst::kIdVinserti128: return Inst::kIdVinserti32x4;
    case Inst::kIdVmovdqa: return Inst::kIdVmovdqa32;
    case Inst::kIdVmovdqu: return Inst::kIdVmovdqu32;
    case Inst::kIdVpand: return Inst::kIdVpandd;
    case Inst::kIdVpandn: return Inst::kIdVpandnd;
    case Inst::kIdVpor: return Inst::kIdVpord;
    case Inst::kIdVpxor: return Inst::kIdVpxord;
    case Inst::kIdVroundpd: return Inst::kIdVrndscalepd;
    case Inst::kIdVroundps: return Inst::kIdVrndscaleps;
    case Inst::kIdVroundsd: return Inst::kIdVrndscalesd;
    case Inst::kIdVroundss: return Inst::kIdVrndscaless;

    default:
      // This should never happen as only transformable instructions should go this path.
      ASMJIT_ASSERT(false);
      return 0;
  }
}

ASMJIT_FAVOR_SPEED Error X86RAPass::rewrite() noexcept {
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
          // This data is allocated by Arena passed to `run_on_function()`, which will be reset after the RA pass finishes.
          // So reset this data to prevent having a dead pointer after the RA pass is complete.
          node->reset_pass_data();

          // If the instruction contains pass data (ra_inst) then it was a subject for register allocation and must be
          // rewritten to use physical regs.
          const RATiedReg* tied_regs = ra_inst->tied_regs();
          uint32_t tied_count = ra_inst->tied_count();
          RegMask combined_reg_ids = 0;

          for (uint32_t i = 0; i < tied_count; i++) {
            const RATiedReg& tied_reg = tied_regs[i];

            Support::BitWordIterator<uint32_t> use_it(tied_reg.use_rewrite_mask());
            if (use_it.has_next()) {
              uint32_t use_id = tied_reg.use_id();
              do {
                inst->_rewrite_id_at_index(use_it.next(), use_id);
              } while (use_it.has_next());
              combined_reg_ids |= use_id;
            }

            Support::BitWordIterator<uint32_t> out_it(tied_reg.out_rewrite_mask());
            if (out_it.has_next()) {
              uint32_t out_id = tied_reg.out_id();
              do {
                inst->_rewrite_id_at_index(out_it.next(), out_id);
              } while (out_it.has_next());
              combined_reg_ids |= out_id;
            }
          }

          // If one operand was rewritten from Reg to Mem, we have to ensure that we are using the correct instruction.
          if (ra_inst->is_reg_to_mem_patched()) {
            switch (inst->inst_id()) {
              case Inst::kIdKmovb: {
                if (operands[0].is_gp() && operands[1].is_mem()) {
                  // Transform from [V]MOVD to MOV.
                  operands[1].as<Mem>().set_size(1);
                  inst->set_inst_id(Inst::kIdMovzx);
                }
                break;
              }

              case Inst::kIdVmovw: {
                if (operands[0].is_gp() && operands[1].is_mem()) {
                  // Transform from [V]MOVD to MOV.
                  operands[1].as<Mem>().set_size(2);
                  inst->set_inst_id(Inst::kIdMovzx);
                }
                break;
              }

              case Inst::kIdMovd:
              case Inst::kIdVmovd:
              case Inst::kIdKmovd: {
                if (operands[0].is_gp() && operands[1].is_mem()) {
                  // Transform from [V]MOVD to MOV.
                  operands[1].as<Mem>().set_size(4);
                  inst->set_inst_id(Inst::kIdMov);
                }
                break;
              }

              case Inst::kIdMovq:
              case Inst::kIdVmovq:
              case Inst::kIdKmovq: {
                if (operands[0].is_gp() && operands[1].is_mem()) {
                  // Transform from [V]MOVQ to MOV.
                  operands[1].as<Mem>().set_size(8);
                  inst->set_inst_id(Inst::kIdMov);
                }
                break;
              }

              default:
                break;
            }
          }

          // Transform VEX instruction to EVEX when necessary.
          if (ra_inst->is_transformable()) {
            if (combined_reg_ids >= 16u) {
              inst->set_inst_id(transform_vex_to_evex(inst->inst_id()));
            }
          }

          // Remove moves that do not do anything.
          //
          // Usually these moves are inserted during code generation and originally they used different registers. If RA
          // allocated these into the same register such redundant mov would appear.
          if (ra_inst->has_inst_rw_flag(InstRWFlags::kMovOp) && !inst->has_extra_reg()) {
            if (operands.size() == 2u) {
              if (operands[0] == operands[1]) {
                cc().remove_node(node);
                node = next;
                continue;
              }
            }
          }

          if (ASMJIT_UNLIKELY(node->type() != NodeType::kInst)) {
            // FuncRet terminates the flow, it must either be removed if the exit label is next to it (optimization) or
            // patched to an architecture dependent jump instruction that jumps to the function's exit before the epilog.
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
      }

      node = next;
    }

    return Error::kOk;
  });
}

// x86::X86RAPass - OnEmit
// =======================

Error X86RAPass::emit_move(RAWorkReg* work_reg, uint32_t dst_phys_id, uint32_t src_phys_id) noexcept {
  Reg dst(work_reg->signature(), dst_phys_id);
  Reg src(work_reg->signature(), src_phys_id);

  const char* comment = nullptr;

#ifndef ASMJIT_NO_LOGGING
  if (has_diagnostic_option(DiagnosticOptions::kRAAnnotate)) {
    _tmp_string.clear();
    Formatter::format_virt_reg_name_with_prefix(_tmp_string, "<MOVE> ", 7u, work_reg->virt_reg());
    comment = _tmp_string.data();
  }
#endif

  return _emit_helper.emit_reg_move(dst, src, work_reg->type_id(), comment);
}

Error X86RAPass::emit_swap(RAWorkReg* a_reg, uint32_t a_phys_id, RAWorkReg* b_reg, uint32_t b_phys_id) noexcept {
  bool is_64bit = Support::max(a_reg->type_id(), b_reg->type_id()) >= TypeId::kInt64;
  OperandSignature sign = is_64bit ? OperandSignature{RegTraits<RegType::kGp64>::kSignature}
                                  : OperandSignature{RegTraits<RegType::kGp32>::kSignature};

#ifndef ASMJIT_NO_LOGGING
  if (has_diagnostic_option(DiagnosticOptions::kRAAnnotate)) {
    _tmp_string.clear();
    Formatter::format_virt_reg_name_with_prefix(_tmp_string, "<SWAP> ", 7u, a_reg->virt_reg());
    Formatter::format_virt_reg_name_with_prefix(_tmp_string, ", "     , 2u, b_reg->virt_reg());
    cc().set_inline_comment(_tmp_string.data());
  }
#endif

  return cc().emit(Inst::kIdXchg, Reg(sign, a_phys_id), Reg(sign, b_phys_id));
}

Error X86RAPass::emit_load(RAWorkReg* work_reg, uint32_t dst_phys_id) noexcept {
  Reg dst_reg(work_reg->signature(), dst_phys_id);
  BaseMem src_mem(work_reg_as_mem(work_reg));

  const char* comment = nullptr;

#ifndef ASMJIT_NO_LOGGING
  if (has_diagnostic_option(DiagnosticOptions::kRAAnnotate)) {
    _tmp_string.clear();
    Formatter::format_virt_reg_name_with_prefix(_tmp_string, "<LOAD> ", 7u, work_reg->virt_reg());
    comment = _tmp_string.data();
  }
#endif

  return _emit_helper.emit_reg_move(dst_reg, src_mem, work_reg->type_id(), comment);
}

Error X86RAPass::emit_save(RAWorkReg* work_reg, uint32_t src_phys_id) noexcept {
  BaseMem dst_mem(work_reg_as_mem(work_reg));
  Reg src_reg(work_reg->signature(), src_phys_id);

  const char* comment = nullptr;

#ifndef ASMJIT_NO_LOGGING
  if (has_diagnostic_option(DiagnosticOptions::kRAAnnotate)) {
    _tmp_string.clear();
    Formatter::format_virt_reg_name_with_prefix(_tmp_string, "<SAVE> ", 7u, work_reg->virt_reg());
    comment = _tmp_string.data();
  }
#endif

  return _emit_helper.emit_reg_move(dst_mem, src_reg, work_reg->type_id(), comment);
}

Error X86RAPass::emit_jump(const Label& label) noexcept {
  return cc().jmp(label);
}

Error X86RAPass::emit_pre_call(InvokeNode* invoke_node) noexcept {
  if (invoke_node->detail().has_var_args() && cc().is_64bit()) {
    const FuncDetail& fd = invoke_node->detail();
    uint32_t arg_count = invoke_node->arg_count();

    switch (invoke_node->detail().call_conv().id()) {
      case CallConvId::kX64SystemV: {
        // AL register contains the number of arguments passed in XMM register(s).
        uint32_t n = 0;
        for (uint32_t arg_index = 0; arg_index < arg_count; arg_index++) {
          const FuncValuePack& arg_pack = fd.arg_pack(arg_index);
          for (uint32_t value_index = 0; value_index < Globals::kMaxValuePack; value_index++) {
            const FuncValue& arg = arg_pack[value_index];
            if (!arg) {
              break;
            }

            if (arg.is_reg() && RegUtils::group_of(arg.reg_type()) == RegGroup::kVec) {
              n++;
            }
          }
        }

        if (!n) {
          ASMJIT_PROPAGATE(cc().xor_(eax, eax));
        }
        else {
          ASMJIT_PROPAGATE(cc().mov(eax, n));
        }
        break;
      }

      case CallConvId::kX64Windows: {
        // Each double-precision argument passed in XMM must be also passed in GP.
        for (uint32_t arg_index = 0; arg_index < arg_count; arg_index++) {
          const FuncValuePack& arg_pack = fd.arg_pack(arg_index);
          for (uint32_t value_index = 0; value_index < Globals::kMaxValuePack; value_index++) {
            const FuncValue& arg = arg_pack[value_index];
            if (!arg) {
              break;
            }

            if (arg.is_reg() && RegUtils::group_of(arg.reg_type()) == RegGroup::kVec) {
              Gp dst = gpq(fd.call_conv().passed_order(RegGroup::kGp)[arg_index]);
              Vec src = xmm(arg.reg_id());
              ASMJIT_PROPAGATE(cc().emit(_emit_helper.ids().movq(), dst, src));
            }
          }
        }
        break;
      }

      default:
        return make_error(Error::kInvalidState);
    }
  }

  return Error::kOk;
}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_X86 && !ASMJIT_NO_COMPILER
