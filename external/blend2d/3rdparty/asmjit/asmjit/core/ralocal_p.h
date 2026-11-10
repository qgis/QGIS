// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_RALOCAL_P_H_INCLUDED
#define ASMJIT_CORE_RALOCAL_P_H_INCLUDED

#include <asmjit/core/api-config.h>
#ifndef ASMJIT_NO_COMPILER

#include <asmjit/core/raassignment_p.h>
#include <asmjit/core/radefs_p.h>
#include <asmjit/core/rainst_p.h>
#include <asmjit/core/rapass_p.h>
#include <asmjit/support/support.h>

ASMJIT_BEGIN_NAMESPACE

//! \cond INTERNAL
//! \addtogroup asmjit_ra
//! \{

//! Local register allocator.
class RALocalAllocator {
public:
  ASMJIT_NONCOPYABLE(RALocalAllocator)

  using PhysToWorkMap = RAAssignment::PhysToWorkMap;
  using WorkToPhysMap = RAAssignment::WorkToPhysMap;

  //! Link to `BaseRAPass`.
  BaseRAPass& _pass;
  //! Link to `BaseCompiler`.
  BaseCompiler& _cc;

  //! Architecture traits.
  const ArchTraits* _arch_traits {};
  //! Registers available to the allocator.
  RARegMask _available_regs {};
  //! Registers clobbered by the allocator.
  RARegMask _clobbered_regs {};
  //! Registers that must be preserved by the function (clobbering means saving & restoring in function prolog & epilog).
  RARegMask _func_preserved_regs {};

  //! Register assignment (current).
  RAAssignment _cur_assignment {};
  //! Register assignment used temporarily during assignment switches.
  RAAssignment _tmp_assignment {};

  //! Link to the current `RABlock`.
  RABlock* _block {};
  //! InstNode.
  InstNode* _node {};
  //! RA instruction.
  RAInst* _ra_inst {};

  //! Count of all TiedReg's.
  uint32_t _tied_total {};
  //! TiedReg's total counter.
  RARegCount _tied_count {};

  //! Temporary work_to_phys_map that can be used freely by the allocator.
  WorkToPhysMap* _tmp_work_to_phys_map {};

  //! \name Construction & Destruction
  //! \{

  inline explicit RALocalAllocator(BaseRAPass& pass) noexcept
    : _pass(pass),
      _cc(pass.cc()),
      _arch_traits(pass._arch_traits),
      _available_regs(pass._available_regs) {
    _func_preserved_regs.init(pass.func()->frame().preserved_regs());
  }

  Error init() noexcept;

  //! \}

  //! \name Accessors
  //! \{

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RAWorkReg* work_reg_by_id(RAWorkId work_id) const noexcept { return _pass.work_reg_by_id(work_id); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG PhysToWorkMap* phys_to_work_map() const noexcept { return _cur_assignment.phys_to_work_map(); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG WorkToPhysMap* work_to_phys_map() const noexcept { return _cur_assignment.work_to_phys_map(); }

  //! Returns the currently processed block.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RABlock* block() const noexcept { return _block; }

  //! Sets the currently processed block.
  ASMJIT_INLINE_NODEBUG void set_block(RABlock* block) noexcept { _block = block; }

  //! Returns the currently processed `InstNode`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG InstNode* node() const noexcept { return _node; }

  //! Returns the currently processed `RAInst`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RAInst* ra_inst() const noexcept { return _ra_inst; }

  //! Returns all tied regs as `RATiedReg` array.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RATiedReg* tied_regs() const noexcept { return _ra_inst->tied_regs(); }

  //! Returns tied registers grouped by the given `group`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RATiedReg* tied_regs(RegGroup group) const noexcept { return _ra_inst->tied_regs(group); }

  //! Returns count of all TiedRegs used by the instruction.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t tied_count() const noexcept { return _tied_total; }

  //! Returns count of TiedRegs used by the given register `group`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t tied_count(RegGroup group) const noexcept { return _tied_count.get(group); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_group_used(RegGroup group) const noexcept { return _tied_count.get(group) != 0; }

  //! \}

  //! \name Assignment
  //! \{

  [[nodiscard]]
  Error make_initial_assignment() noexcept;

  [[nodiscard]]
  Error replace_assignment(const PhysToWorkMap* phys_to_work_map) noexcept;

  //! Switch to the given assignment by reassigning all register and emitting code that reassigns them.
  //! This is always used to switch to a previously stored assignment.
  //!
  //! If `try_mode` is true then the final assignment doesn't have to be exactly same as specified by
  //! `dst_phys_to_work_map`. This mode is only used before conditional jumps that already have assignment
  //! to generate a code sequence that is always executed regardless of the flow.
  [[nodiscard]]
  Error switch_to_assignment(PhysToWorkMap* dst_phys_to_work_map, Span<const BitWord> live_in, bool dst_is_read_only, bool try_mode) noexcept;

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Error spill_regs_before_entry(RABlock* block) noexcept {
    return spill_scratch_gp_regs_before_entry(block->entry_scratch_gp_regs());
  }

  [[nodiscard]]
  Error spill_scratch_gp_regs_before_entry(uint32_t scratch_regs) noexcept;

  //! \}

  //! \name Allocation
  //! \{

  [[nodiscard]]
  Error alloc_instruction(InstNode* node) noexcept;

  [[nodiscard]]
  Error spill_after_allocation(InstNode* node) noexcept;

  [[nodiscard]]
  Error alloc_branch(InstNode* node, RABlock* target, RABlock* cont) noexcept;

  [[nodiscard]]
  Error alloc_jump_table(InstNode* node, Span<RABlock*> targets, RABlock* cont) noexcept;

  //! \}

  //! \name Decision Making
  //! \{

  enum CostModel : uint32_t {
    kCostOfFrequency = 1048576,
    kCostOfDirtyFlag = kCostOfFrequency / 4
  };

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t cost_by_frequency(float freq) const noexcept {
    return uint32_t(int32_t(freq * float(kCostOfFrequency)));
  }

  [[nodiscard]]
  ASMJIT_INLINE uint32_t calc_spill_cost(RegGroup group, RAWorkReg* work_reg, uint32_t assigned_id) const noexcept {
    uint32_t cost = cost_by_frequency(work_reg->live_stats().freq());

    if (_cur_assignment.is_phys_dirty(group, assigned_id))
      cost += kCostOfDirtyFlag;

    return cost;
  }

  [[nodiscard]]
  ASMJIT_INLINE uint32_t pick_best_suitable_register(RegGroup group, RegMask allocable_regs) const noexcept {
    // These are registers must be preserved by the function itself.
    RegMask preserved_regs = _func_preserved_regs[group];

    // Reduce the set by removing preserved registers when possible.
    if (allocable_regs & ~preserved_regs) {
      allocable_regs &= ~preserved_regs;
    }

    return Support::ctz(allocable_regs);
  }

  //! Decides on register assignment.
  [[nodiscard]]
  uint32_t decide_on_assignment(RegGroup group, RAWorkReg* work_reg, uint32_t assigned_id, RegMask allocable_regs) const noexcept;

  //! Decides on whether to MOVE or SPILL the given WorkReg, because it's allocated in a physical register that have
  //! to be used by another WorkReg.
  //!
  //! The function must return either `RAAssignment::kPhysNone`, which means that the WorkReg of `work_id` should be
  //! spilled, or a valid physical register ID, which means that the register should be moved to that physical register
  //! instead.
  [[nodiscard]]
  uint32_t decide_on_reassignment(RegGroup group, RAWorkReg* work_reg, uint32_t assigned_id, RegMask allocable_regs, RAInst* ra_inst) const noexcept;

  //! Decides on best spill given a register mask `spillable_regs`
  [[nodiscard]]
  uint32_t decide_on_spill_for(RegGroup group, RAWorkReg* work_reg, RegMask spillable_regs, RAWorkId* spill_work_id) const noexcept;

  //! \}

  //! \name Emit
  //! \{

  //! Assigns a register, the content of it is undefined at this point.
  [[nodiscard]]
  ASMJIT_INLINE Error _assign_reg(RegGroup rg, RAWorkId work_id, uint32_t phys_id, bool dirty) noexcept {
    _cur_assignment.assign(rg, work_id, phys_id, dirty);
    return Error::kOk;
  }

  ASMJIT_INLINE void _unassign_reg(RegGroup rg, RAWorkId work_id, uint32_t phys_id) noexcept {
    _cur_assignment.unassign(rg, work_id, phys_id);
  }

  //! Emits a load from [VirtReg/WorkReg]'s spill slot to a physical register
  //! and makes it assigned and clean.
  [[nodiscard]]
  ASMJIT_INLINE Error on_load_reg(RegGroup rg, RAWorkReg* work_reg, RAWorkId work_id, uint32_t phys_id) noexcept {
    _cur_assignment.assign(rg, work_id, phys_id, RAAssignment::kClean);
    return _pass.emit_load(work_reg, phys_id);
  }

  //! Emits a save a physical register to a [VirtReg/WorkReg]'s spill slot,
  //! keeps it assigned, and makes it clean.
  [[nodiscard]]
  ASMJIT_INLINE Error on_save_reg(RegGroup rg, RAWorkReg* work_reg, RAWorkId work_id, uint32_t phys_id) noexcept {
    ASMJIT_ASSERT(_cur_assignment.work_to_phys_id(rg, work_id) == phys_id);
    ASMJIT_ASSERT(_cur_assignment.phys_to_work_id(rg, phys_id) == work_id);

    _cur_assignment.make_clean(rg, work_id, phys_id);
    return _pass.emit_save(work_reg, phys_id);
  }

  //! Emits a move between a destination and source register, and fixes the
  //! register assignment.
  [[nodiscard]]
  ASMJIT_INLINE Error on_move_reg(RegGroup rg, RAWorkReg* work_reg, RAWorkId work_id, uint32_t dst_phys_id, uint32_t src_phys_id) noexcept {
    if (dst_phys_id == src_phys_id) {
      return Error::kOk;
    }

    _cur_assignment.reassign(rg, work_id, dst_phys_id, src_phys_id);
    return _pass.emit_move(work_reg, dst_phys_id, src_phys_id);
  }

  //! Spills a variable/register, saves the content to the memory-home if modified.
  [[nodiscard]]
  ASMJIT_INLINE Error on_spill_reg(RegGroup rg, RAWorkReg* work_reg, RAWorkId work_id, uint32_t phys_id) noexcept {
    if (_cur_assignment.is_phys_dirty(rg, phys_id)) {
      ASMJIT_PROPAGATE(on_save_reg(rg, work_reg, work_id, phys_id));
    }
    _unassign_reg(rg, work_id, phys_id);
    return Error::kOk;
  }

  //! Emits a swap between two physical registers and fixes their assignment.
  //!
  //! \note Target must support this operation otherwise this would ASSERT.
  [[nodiscard]]
  ASMJIT_INLINE Error on_swap_reg(RegGroup rg, RAWorkReg* a_reg, RAWorkId a_work_id, uint32_t a_phys_id, RAWorkReg* b_reg, RAWorkId b_work_id, uint32_t b_phys_id) noexcept {
    _cur_assignment.swap(rg, a_work_id, a_phys_id, b_work_id, b_phys_id);
    return _pass.emit_swap(a_reg, a_phys_id, b_reg, b_phys_id);
  }

  //! \}
};

//! \}
//! \endcond

ASMJIT_END_NAMESPACE

#endif // !ASMJIT_NO_COMPILER
#endif // ASMJIT_CORE_RALOCAL_P_H_INCLUDED
