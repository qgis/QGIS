// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#ifndef ASMJIT_NO_COMPILER

#include <asmjit/core/ralocal_p.h>
#include <asmjit/support/support.h>

ASMJIT_BEGIN_NAMESPACE

// RALocalAllocator - Utilities
// ============================

static ASMJIT_INLINE RATiedReg* RALocal_findTiedReg(RATiedReg* tied_regs, size_t count, RAWorkReg* work_reg) noexcept {
  for (size_t i = 0; i < count; i++) {
    if (tied_regs[i].work_reg() == work_reg) {
      return &tied_regs[i];
    }
  }
  return nullptr;
}

// RALocalAllocator - Initialization & Reset
// =========================================

Error RALocalAllocator::init() noexcept {
  PhysToWorkMap* phys_to_work_map;
  WorkToPhysMap* work_to_phys_map;

  phys_to_work_map = _pass.new_phys_to_work_map();
  work_to_phys_map = _pass.new_work_to_phys_map();

  if (!phys_to_work_map || !work_to_phys_map) {
    return make_error(Error::kOutOfMemory);
  }

  _cur_assignment.init_layout(_pass._phys_reg_count, _pass.work_regs());
  _cur_assignment.init_maps(phys_to_work_map, work_to_phys_map);

  phys_to_work_map = _pass.new_phys_to_work_map();
  work_to_phys_map = _pass.new_work_to_phys_map();
  _tmp_work_to_phys_map = _pass.new_work_to_phys_map();

  if (!phys_to_work_map || !work_to_phys_map || !_tmp_work_to_phys_map) {
    return make_error(Error::kOutOfMemory);
  }

  _tmp_assignment.init_layout(_pass._phys_reg_count, _pass.work_regs());
  _tmp_assignment.init_maps(phys_to_work_map, work_to_phys_map);

  return Error::kOk;
}

// RALocalAllocator - Assignment
// =============================

Error RALocalAllocator::make_initial_assignment() noexcept {
  FuncNode* func = _pass.func();
  RABlock* entry = _pass.entry_block();

  Span<BitWord> live_in = entry->live_in();
  uint32_t multi_work_reg_count = _pass._multi_work_reg_count;

  uint32_t arg_count = func->arg_count();
  uint32_t iter_count = 1;

  for (uint32_t iter = 0; iter < iter_count; iter++) {
    for (uint32_t arg_index = 0; arg_index < arg_count; arg_index++) {
      for (uint32_t value_index = 0; value_index < Globals::kMaxValuePack; value_index++) {
        // Unassigned argument.
        const RegOnly& reg_arg = func->arg_pack(arg_index)[value_index];
        if (!reg_arg.is_reg() || !_cc.is_virt_id_valid(reg_arg.id())) {
          continue;
        }

        VirtReg* virt_reg = _cc.virt_reg_by_id(reg_arg.id());
        RAWorkReg* work_reg = virt_reg->work_reg();

        // Unreferenced argument.
        if (!work_reg) {
          continue;
        }

        // Overwritten argument.
        RAWorkId work_id = work_reg->work_id();
        if (uint32_t(work_id) >= multi_work_reg_count || !BitOps::bit_at(live_in, work_id)) {
          continue;
        }

        RegGroup group = work_reg->group();
        if (_cur_assignment.work_to_phys_id(group, work_id) != RAAssignment::kPhysNone) {
          continue;
        }

        RegMask allocable_regs = _available_regs[group] & ~_cur_assignment.assigned(group);
        if (iter == 0) {
          // First iteration: Try to allocate to home RegId.
          if (work_reg->has_home_reg_id()) {
            uint32_t phys_id = work_reg->home_reg_id();
            if (Support::bit_test(allocable_regs, phys_id)) {
              _cur_assignment.assign(group, work_id, phys_id, true);
              _pass._args_assignment.assign_reg_in_pack(arg_index, value_index, work_reg->type(), phys_id, work_reg->type_id());
              continue;
            }
          }

          iter_count = 2;
        }
        else {
          // Second iteration: Pick any other register if the is an unassigned one or assign to stack.
          if (allocable_regs) {
            uint32_t phys_id = Support::ctz(allocable_regs);
            _cur_assignment.assign(group, work_id, phys_id, true);
            _pass._args_assignment.assign_reg_in_pack(arg_index, value_index, work_reg->type(), phys_id, work_reg->type_id());
          }
          else {
            // This register will definitely need stack, create the slot now and assign also `arg_index`
            // to it. We will patch `_args_assignment` later after RAStackAllocator finishes.
            RAStackSlot* slot = _pass.get_or_create_stack_slot(work_reg);
            if (ASMJIT_UNLIKELY(!slot)) {
              return make_error(Error::kOutOfMemory);
            }

            // This means STACK_ARG may be moved to STACK.
            work_reg->add_flags(RAWorkRegFlags::kStackArgToStack);
            _pass._num_stack_args_to_stack_slots++;
          }
        }
      }
    }
  }

  return Error::kOk;
}

Error RALocalAllocator::replace_assignment(const PhysToWorkMap* phys_to_work_map) noexcept {
  _cur_assignment.copy_from(phys_to_work_map);
  return Error::kOk;
}

Error RALocalAllocator::switch_to_assignment(PhysToWorkMap* dst_phys_to_work_map, Span<const BitWord> live_in, bool dst_is_read_only, bool try_mode) noexcept {
  RAAssignment dst;
  RAAssignment& cur = _cur_assignment;

  dst.init_layout(_pass._phys_reg_count, _pass.work_regs());
  dst.init_maps(dst_phys_to_work_map, _tmp_work_to_phys_map);
  dst.assign_work_ids_from_phys_ids();

  uint32_t multi_work_reg_count = _pass._multi_work_reg_count;

  for (RegGroup group : Support::enumerate(RegGroup::kMaxVirt)) {
    // STEP 1
    // ------
    //
    //   - KILL all registers that are not live at `dst`,
    //   - SPILL all registers that are not assigned at `dst`.

    if (!try_mode) {
      Support::BitWordIterator<RegMask> it(cur.assigned(group));
      while (it.has_next()) {
        uint32_t phys_id = it.next();
        RAWorkId work_id = cur.phys_to_work_id(group, phys_id);

        // Must be true as we iterate over assigned registers.
        ASMJIT_ASSERT(work_id != kBadWorkId);

        // KILL if it's not live on entry.
        if (uint32_t(work_id) >= multi_work_reg_count || !BitOps::bit_at(live_in, work_id)) {
          _unassign_reg(group, work_id, phys_id);
          continue;
        }

        // SPILL if it's not assigned on entry.
        uint32_t alt_id = dst.work_to_phys_id(group, work_id);
        if (alt_id == RAAssignment::kPhysNone) {
          ASMJIT_PROPAGATE(on_spill_reg(group, work_reg_by_id(work_id), work_id, phys_id));
        }
      }
    }

    // STEP 2
    // ------
    //
    //   - MOVE and SWAP registers from their current assignments into their DST assignments.
    //   - Build `will_load_regs` mask of registers scheduled for `on_load_reg()`.

    // Current run-id (1 means more aggressive decisions).
    int32_t run_id = -1;
    // Remaining registers scheduled for `on_load_reg()`.
    RegMask will_load_regs = 0;
    // Remaining registers to be allocated in this loop.
    RegMask affected_regs = dst.assigned(group);

    while (affected_regs) {
      if (++run_id == 2) {
        if (!try_mode) {
          return make_error(Error::kInvalidState);
        }

        // Stop in `try_mode` if we haven't done anything in past two rounds.
        break;
      }

      Support::BitWordIterator<RegMask> it(affected_regs);
      while (it.has_next()) {
        uint32_t phys_id = it.next();
        RegMask phys_mask = Support::bit_mask<RegMask>(phys_id);

        RAWorkId cur_work_id = cur.phys_to_work_id(group, phys_id);
        RAWorkId dst_work_id = dst.phys_to_work_id(group, phys_id);

        // The register must have assigned `dst_work_id` as we only iterate over assigned regs.
        ASMJIT_ASSERT(dst_work_id != kBadWorkId);
        // The register must be crossing multiple basic blocks, otherwise it should never be present in the map.
        ASMJIT_ASSERT(uint32_t(dst_work_id) < multi_work_reg_count);

        RAWorkReg* dst_work_reg = work_reg_by_id(dst_work_id);

        if (cur_work_id != kBadWorkId) {
          RAWorkReg* cur_work_reg = work_reg_by_id(cur_work_id);

          // Both assigned.
          if (cur_work_id != dst_work_id) {
            // Wait a bit if this is the first run, we may avoid this if `cur_work_id` moves out.
            if (run_id <= 0) {
              continue;
            }

            uint32_t alt_phys_id = cur.work_to_phys_id(group, dst_work_id);
            if (alt_phys_id == RAAssignment::kPhysNone) {
              continue;
            }

            // Reset as we will do some changes to the current assignment.
            run_id = -1;

            if (_arch_traits->has_inst_reg_swap(group)) {
              ASMJIT_PROPAGATE(on_swap_reg(group, cur_work_reg, cur_work_id, phys_id, dst_work_reg, dst_work_id, alt_phys_id));
            }
            else {
              // SPILL the reg if it's not dirty in DST, otherwise try to MOVE.
              if (!cur.is_phys_dirty(group, phys_id)) {
                _unassign_reg(group, cur_work_id, phys_id);
              }
              else {
                RegMask allocable_regs = _pass._available_regs[group] & ~cur.assigned(group);

                // If possible don't conflict with assigned regs at DST.
                if (allocable_regs & ~dst.assigned(group)) {
                  allocable_regs &= ~dst.assigned(group);
                }

                if (allocable_regs) {
                  // MOVE is possible, thus preferred.
                  uint32_t tmp_phys_id = Support::ctz(allocable_regs);

                  ASMJIT_PROPAGATE(on_move_reg(group, cur_work_reg, cur_work_id, tmp_phys_id, phys_id));
                  _clobbered_regs[group] |= Support::bit_mask<RegMask>(tmp_phys_id);
                }
                else {
                  // MOVE is impossible, must SPILL.
                  ASMJIT_PROPAGATE(on_spill_reg(group, cur_work_reg, cur_work_id, phys_id));
                }
              }

              goto Cleared;
            }
          }
        }
        else {
Cleared:
          // DST assigned, CUR unassigned.
          uint32_t alt_phys_id = cur.work_to_phys_id(group, dst_work_id);
          if (alt_phys_id == RAAssignment::kPhysNone) {
            if (BitOps::bit_at(live_in, dst_work_id)) {
              will_load_regs |= phys_mask; // Scheduled for `on_load_reg()`.
            }
            affected_regs &= ~phys_mask;  // Unaffected from now.
            continue;
          }
          ASMJIT_PROPAGATE(on_move_reg(group, dst_work_reg, dst_work_id, phys_id, alt_phys_id));
        }

        // Both DST and CUR assigned to the same reg or CUR just moved to DST.
        if ((dst.dirty(group) & phys_mask) != (cur.dirty(group) & phys_mask)) {
          if ((dst.dirty(group) & phys_mask) == 0) {
            // CUR dirty, DST not dirty (the assert is just to visualize the condition).
            ASMJIT_ASSERT(!dst.is_phys_dirty(group, phys_id) && cur.is_phys_dirty(group, phys_id));

            // If `dst_is_read_only` is true it means that that block was already processed and we cannot change from
            // CLEAN to DIRTY. In that case the register has to be saved as it cannot enter the block DIRTY.
            if (dst_is_read_only) {
              ASMJIT_PROPAGATE(on_save_reg(group, dst_work_reg, dst_work_id, phys_id));
            }
            else {
              dst.make_dirty(group, dst_work_id, phys_id);
            }
          }
          else {
            // DST dirty, CUR not dirty (the assert is just to visualize the condition).
            ASMJIT_ASSERT(dst.is_phys_dirty(group, phys_id) && !cur.is_phys_dirty(group, phys_id));

            cur.make_dirty(group, dst_work_id, phys_id);
          }
        }

        // Must match now...
        ASMJIT_ASSERT(dst.phys_to_work_id(group, phys_id) == cur.phys_to_work_id(group, phys_id));
        ASMJIT_ASSERT(dst.is_phys_dirty(group, phys_id) == cur.is_phys_dirty(group, phys_id));

        run_id = -1;
        affected_regs &= ~phys_mask;
      }
    }

    // STEP 3
    // ------
    //
    //   - Load registers specified by `will_load_regs`.

    {
      Support::BitWordIterator<RegMask> it(will_load_regs);
      while (it.has_next()) {
        uint32_t phys_id = it.next();

        if (!cur.is_phys_assigned(group, phys_id)) {
          RAWorkId work_id = dst.phys_to_work_id(group, phys_id);

          // The register must be crossing multiple basic blocks otherwise it should not be in the map.
          ASMJIT_ASSERT(uint32_t(work_id) < multi_work_reg_count);
          // The algorithm is broken if it tries to load a register that is not in LIVE-IN.
          ASMJIT_ASSERT(BitOps::bit_at(live_in, work_id) == true);

          RAWorkReg* work_reg = work_reg_by_id(work_id);
          ASMJIT_PROPAGATE(on_load_reg(group, work_reg, work_id, phys_id));
          if (dst.is_phys_dirty(group, phys_id)) {
            cur.make_dirty(group, work_id, phys_id);
          }
          ASMJIT_ASSERT(dst.is_phys_dirty(group, phys_id) == cur.is_phys_dirty(group, phys_id));
        }
        else {
          // Not possible otherwise.
          ASMJIT_ASSERT(try_mode == true);
        }
      }
    }
  }

  if (!try_mode) {
    // Here is a code that dumps the conflicting part if something fails here:
    // if (!dst.equals(cur)) {
    //   uint32_t phys_total = dst._layout.phys_total;
    //   uint32_t work_count = dst._layout.work_count;
    //
    //   fprintf(stderr, "Dirty    DST=0x%08X CUR=0x%08X\n", dst.dirty(RegGroup::kGp), cur.dirty(RegGroup::kGp));
    //   fprintf(stderr, "Assigned DST=0x%08X CUR=0x%08X\n", dst.assigned(RegGroup::kGp), cur.assigned(RegGroup::kGp));
    //
    //   for (uint32_t phys_id = 0; phys_id < phys_total; phys_id++) {
    //     uint32_t dst_work_id = dst._phys_to_work_map->work_ids[phys_id];
    //     uint32_t cur_work_id = cur._phys_to_work_map->work_ids[phys_id];
    //     if (dst_work_id != cur_work_id)
    //       fprintf(stderr, "[PhysIdWork] PhysId=%u WorkId[DST(%u) != CUR(%u)]\n", phys_id, dst_work_id, cur_work_id);
    //   }
    //
    //   for (uint32_t work_id = 0; work_id < work_count; work_id++) {
    //     uint32_t dst_phys_id = dst._work_to_phys_map->phys_ids[work_id];
    //     uint32_t cur_phys_id = cur._work_to_phys_map->phys_ids[work_id];
    //     if (dst_phys_id != cur_phys_id)
    //       fprintf(stderr, "[WorkToPhys] WorkId=%u PhysId[DST(%u) != CUR(%u)]\n", work_id, dst_phys_id, cur_phys_id);
    //   }
    // }
    ASMJIT_ASSERT(dst.equals(cur));
  }

  return Error::kOk;
}

Error RALocalAllocator::spill_scratch_gp_regs_before_entry(RegMask scratch_regs) noexcept {
  RegGroup group = RegGroup::kGp;
  Support::BitWordIterator<RegMask> it(scratch_regs);

  while (it.has_next()) {
    uint32_t phys_id = it.next();
    if (_cur_assignment.is_phys_assigned(group, phys_id)) {
      RAWorkId work_id = _cur_assignment.phys_to_work_id(group, phys_id);
      ASMJIT_PROPAGATE(on_spill_reg(group, work_reg_by_id(work_id), work_id, phys_id));
    }
  }

  return Error::kOk;
}

// RALocalAllocator - Allocation
// =============================

Error RALocalAllocator::alloc_instruction(InstNode* node) noexcept {
  RAInst* ra_inst = node->pass_data<RAInst>();

  RATiedReg* out_tied_regs[Globals::kMaxPhysRegs];
  RATiedReg* dup_tied_regs[Globals::kMaxPhysRegs];
  RATiedReg* consecutive_regs[kMaxConsecutiveRegs];

  // The cursor must point to the previous instruction for a possible instruction insertion.
  _cc.set_cursor(node->prev());

  _node = node;
  _ra_inst = ra_inst;
  _tied_total = ra_inst->_tied_total;
  _tied_count = ra_inst->_tied_count;

  // Whether we already replaced register operand with memory operand.
  bool rm_allocated = false;

  for (RegGroup group : Support::enumerate(RegGroup::kMaxVirt)) {
    uint32_t i, count = this->tied_count(group);
    RATiedReg* tied_regs = this->tied_regs(group);

    RegMask will_use = _ra_inst->_used_regs[group];
    RegMask will_out = _ra_inst->_clobbered_regs[group];
    RegMask will_free = 0;

    uint32_t use_pending_count = count;
    uint32_t out_tied_count = 0;
    uint32_t dup_tied_count = 0;
    uint32_t consecutive_mask = 0;

    // STEP 1
    // ------
    //
    // Calculate `will_use` and `will_free` masks based on tied registers we have. In addition, aggregate information
    // regarding consecutive registers used by this instruction. We need that to make USE/OUT assignments.
    //
    // We don't do any assignment decisions at this stage as we just need to collect some information first. Then,
    // after we populate all masks needed we can finally make some decisions in the second loop. The main reason
    // for this is that we really need `will_free` to make assignment decisions for `will_use`, because if we mark
    // some registers that will be freed, we can consider them in decision making afterwards.

    for (i = 0; i < count; i++) {
      RATiedReg* tied_reg = &tied_regs[i];

      if (tied_reg->has_any_consecutive_flag()) {
        uint32_t consecutive_offset = tied_reg->is_lead_consecutive() ? uint32_t(0) : tied_reg->consecutive_data();

        if (ASMJIT_UNLIKELY(Support::bit_test(consecutive_mask, consecutive_offset))) {
          return make_error(Error::kInvalidState);
        }

        consecutive_mask |= Support::bit_mask<uint32_t>(consecutive_offset);
        consecutive_regs[consecutive_offset] = tied_reg;
      }

      // Add OUT and KILL to `out_pending_count` for CLOBBERing and/or OUT assignment.
      if (tied_reg->is_out_or_kill()) {
        out_tied_regs[out_tied_count++] = tied_reg;
      }

      if (tied_reg->is_duplicate()) {
        dup_tied_regs[dup_tied_count++] = tied_reg;
      }

      if (!tied_reg->is_use()) {
        tied_reg->mark_use_done();
        use_pending_count--;
        continue;
      }

      // Don't assign anything here if this is a consecutive USE - we will handle this in STEP 2 instead.
      if (tied_reg->is_use_consecutive()) {
        continue;
      }

      RAWorkReg* work_reg = tied_reg->work_reg();
      RAWorkId work_id = work_reg->work_id();
      uint32_t assigned_id = _cur_assignment.work_to_phys_id(group, work_id);

      if (tied_reg->has_use_id()) {
        // If the register has `use_id` it means it can only be allocated in that register.
        RegMask use_mask = Support::bit_mask<RegMask>(tied_reg->use_id());

        // RAInstBuilder must have collected `used_regs` on-the-fly.
        ASMJIT_ASSERT((will_use & use_mask) != 0);

        if (assigned_id == tied_reg->use_id()) {
          // If the register is already allocated in this one, mark it done and continue.
          tied_reg->mark_use_done();
          if (tied_reg->is_write()) {
            _cur_assignment.make_dirty(group, work_id, assigned_id);
          }
          use_pending_count--;
          will_use |= use_mask;
        }
        else {
          will_free |= use_mask & _cur_assignment.assigned(group);
        }
      }
      else {
        // Check if the register must be moved to `allocable_regs`.
        RegMask allocable_regs = tied_reg->use_reg_mask();
        if (assigned_id != RAAssignment::kPhysNone) {
          RegMask assigned_mask = Support::bit_mask<RegMask>(assigned_id);
          if ((allocable_regs & ~will_use) & assigned_mask) {
            tied_reg->set_use_id(assigned_id);
            tied_reg->mark_use_done();
            if (tied_reg->is_write()) {
              _cur_assignment.make_dirty(group, work_id, assigned_id);
            }
            use_pending_count--;
            will_use |= assigned_mask;
          }
          else {
            will_free |= assigned_mask;
          }
        }
      }
    }

    // STEP 2
    // ------
    //
    // Verify that all the consecutive registers are really consecutive. Terminate if there is a gap. In addition,
    // decide which USE ids will be used in case that this consecutive sequence is USE (OUT registers are allocated
    // in a different step).
    uint32_t consecutive_count = 0;

    if (consecutive_mask) {
      if ((consecutive_mask & (consecutive_mask + 1u)) != 0) {
        return make_error(Error::kInvalidState);
      }

      // Count of trailing ones is the count of consecutive registers. There cannot be gap.
      consecutive_count = Support::ctz(~consecutive_mask);

      // Prioritize allocation that would result in least moves even when moving registers away from their homes.
      RATiedReg* lead = consecutive_regs[0];

      // Assign the best possible USE Ids to all consecutives.
      if (lead->is_use_consecutive()) {
        uint32_t best_score = 0;
        uint32_t best_lead_reg = 0xFFFFFFFF;
        RegMask allocable_regs = (_available_regs[group] | will_free) & ~will_use;

        uint32_t assignments[kMaxConsecutiveRegs];

        for (i = 0; i < consecutive_count; i++) {
          assignments[i] = _cur_assignment.work_to_phys_id(group, consecutive_regs[i]->work_reg()->work_id());
        }

        Support::BitWordIterator<uint32_t> it(lead->use_reg_mask());
        while (it.has_next()) {
          uint32_t reg_index = it.next();
          if (Support::bit_test(lead->use_reg_mask(), reg_index)) {
            uint32_t score = 15;

            for (i = 0; i < consecutive_count; i++) {
              uint32_t consecutive_index = reg_index + i;
              if (!Support::bit_test(allocable_regs, consecutive_index)) {
                score = 0;
                break;
              }

              RAWorkReg* work_reg = consecutive_regs[i]->work_reg();
              score += uint32_t(work_reg->home_reg_id() == consecutive_index);
              score += uint32_t(assignments[i] == consecutive_index) * 2;
            }

            if (score > best_score) {
              best_score = score;
              best_lead_reg = reg_index;
            }
          }
        }

        if (best_lead_reg == 0xFFFFFFFF) {
          return make_error(Error::kConsecutiveRegsAllocation);
        }

        for (i = 0; i < consecutive_count; i++) {
          uint32_t consecutive_index = best_lead_reg + i;

          RATiedReg* tied_reg = consecutive_regs[i];
          RegMask use_mask = Support::bit_mask<uint32_t>(consecutive_index);

          RAWorkReg* work_reg = tied_reg->work_reg();
          RAWorkId work_id = work_reg->work_id();
          uint32_t assigned_id = _cur_assignment.work_to_phys_id(group, work_id);

          tied_reg->set_use_id(consecutive_index);

          if (assigned_id == consecutive_index) {
            // If the register is already allocated in this one, mark it done and continue.
            tied_reg->mark_use_done();
            if (tied_reg->is_write()) {
              _cur_assignment.make_dirty(group, work_id, assigned_id);
            }
            use_pending_count--;
            will_use |= use_mask;
          }
          else {
            will_use |= use_mask;
            will_free |= use_mask & _cur_assignment.assigned(group);
          }
        }
      }
    }

    // STEP 3
    // ------
    //
    // Do some decision making to find the best candidates of registers that need to be assigned, moved, and/or
    // spilled. Only USE registers are considered here, OUT will be decided later after all CLOBBERed and OUT
    // registers are unassigned.

    if (use_pending_count) {
      // TODO: Not sure `live_regs` should be used, maybe will_use and will_free would be enough and much more clear.

      // All registers that are currently alive without registers that will be freed.
      RegMask live_regs = _cur_assignment.assigned(group) & ~will_free;

      for (i = 0; i < count; i++) {
        RATiedReg* tied_reg = &tied_regs[i];
        if (tied_reg->is_use_done()) {
          continue;
        }

        RAWorkReg* work_reg = tied_reg->work_reg();
        RAWorkId work_id = work_reg->work_id();
        uint32_t assigned_id = _cur_assignment.work_to_phys_id(group, work_id);

        // REG/MEM: Patch register operand to memory operand if not allocated.
        if (!rm_allocated && tied_reg->has_use_rm()) {
          if (assigned_id == RAAssignment::kPhysNone && Support::is_power_of_2(tied_reg->use_rewrite_mask())) {
            uint32_t op_index = Support::ctz(tied_reg->use_rewrite_mask()) / uint32_t(sizeof(Operand) / sizeof(uint32_t));
            uint32_t rm_size = tied_reg->rm_size();

            if (rm_size <= work_reg->virt_reg()->virt_size()) {
              Operand& op = node->operands()[op_index];
              op = _pass.work_reg_as_mem(work_reg);

              // NOTE: We cannot use `x86::Mem::set_size()` from here, so let's manipulate the signature directly.
              op._signature.set_size(rm_size);

              tied_reg->_use_rewrite_mask = 0;

              tied_reg->mark_use_done();
              ra_inst->add_flags(RATiedFlags::kInst_RegToMemPatched);
              use_pending_count--;

              rm_allocated = true;
              continue;
            }
          }
        }

        if (!tied_reg->has_use_id()) {
          // DECIDE where to assign the USE register.
          RegMask allocable_regs = tied_reg->use_reg_mask() & ~(will_free | will_use);
          uint32_t use_id = decide_on_assignment(group, work_reg, assigned_id, allocable_regs);

          RegMask use_mask = Support::bit_mask<RegMask>(use_id);
          will_use |= use_mask;
          will_free |= use_mask & live_regs;
          tied_reg->set_use_id(use_id);

          if (assigned_id != RAAssignment::kPhysNone) {
            RegMask assigned_mask = Support::bit_mask<RegMask>(assigned_id);

            will_free |= assigned_mask;
            live_regs &= ~assigned_mask;

            // OPTIMIZATION: Assign the USE register here if it's possible.
            if (!(live_regs & use_mask)) {
              ASMJIT_PROPAGATE(on_move_reg(group, work_reg, work_id, use_id, assigned_id));
              tied_reg->mark_use_done();
              if (tied_reg->is_write()) {
                _cur_assignment.make_dirty(group, work_id, use_id);
              }
              use_pending_count--;
            }
          }
          else {
            // OPTIMIZATION: Assign the USE register here if it's possible.
            if (!(live_regs & use_mask)) {
              ASMJIT_PROPAGATE(on_load_reg(group, work_reg, work_id, use_id));
              tied_reg->mark_use_done();
              if (tied_reg->is_write()) {
                _cur_assignment.make_dirty(group, work_id, use_id);
              }
              use_pending_count--;
            }
          }

          live_regs |= use_mask;
        }
      }
    }

    // Initially all used regs will be marked as clobbered.
    RegMask clobbered_by_inst = will_use | will_out;

    // STEP 4
    // ------
    //
    // Free all registers that we marked as `will_free`. Only registers that are not USEd by the instruction are
    // considered as we don't want to free regs we need.

    if (will_free) {
      RegMask allocable_regs = _available_regs[group] & ~(_cur_assignment.assigned(group) | will_free | will_use | will_out);
      Support::BitWordIterator<RegMask> it(will_free);

      do {
        uint32_t assigned_id = it.next();
        if (_cur_assignment.is_phys_assigned(group, assigned_id)) {
          RAWorkId work_id = _cur_assignment.phys_to_work_id(group, assigned_id);
          RAWorkReg* work_reg = work_reg_by_id(work_id);

          // DECIDE whether to MOVE or SPILL.
          if (allocable_regs) {
            uint32_t reassigned_id = decide_on_reassignment(group, work_reg, assigned_id, allocable_regs, ra_inst);
            if (reassigned_id != RAAssignment::kPhysNone) {
              ASMJIT_PROPAGATE(on_move_reg(group, work_reg, work_id, reassigned_id, assigned_id));
              allocable_regs ^= Support::bit_mask<RegMask>(reassigned_id);
              _clobbered_regs[group] |= Support::bit_mask<RegMask>(reassigned_id);
              continue;
            }
          }

          ASMJIT_PROPAGATE(on_spill_reg(group, work_reg, work_id, assigned_id));
        }
      } while (it.has_next());
    }

    // STEP 5
    // ------
    //
    // ALLOCATE / SHUFFLE all registers that we marked as `will_use` and weren't allocated yet. This is a bit
    // complicated as the allocation is iterative. In some cases we have to wait before allocating a particular
    // physical register as it's still occupied by some other one, which we need to move before we can use it.
    // In this case we skip it and allocate another one instead (making it free for the next iteration).
    //
    // NOTE: Iterations are mostly important for complicated allocations like function calls, where there can
    // be up to N registers used at once. Asm instructions won't run the loop more than once in 99.9% of cases
    // as they use 2 to 3 registers in average.

    if (use_pending_count) {
      bool must_swap = false;
      do {
        uint32_t old_pending_count = use_pending_count;

        for (i = 0; i < count; i++) {
          RATiedReg* this_tied_reg = &tied_regs[i];
          if (this_tied_reg->is_use_done()) {
            continue;
          }

          RAWorkReg* this_work_reg = this_tied_reg->work_reg();
          RAWorkId this_work_id = this_work_reg->work_id();
          uint32_t this_phys_id = _cur_assignment.work_to_phys_id(group, this_work_id);

          // This would be a bug, fatal one!
          uint32_t target_phys_id = this_tied_reg->use_id();
          ASMJIT_ASSERT(target_phys_id != this_phys_id);

          RAWorkId target_work_id = _cur_assignment.phys_to_work_id(group, target_phys_id);
          if (target_work_id != kBadWorkId) {
            RAWorkReg* target_work_reg = work_reg_by_id(target_work_id);

            // Swapping two registers can solve two allocation tasks by emitting just a single instruction. However,
            // swap is only available on few architectures and it's definitely not available for each register group.
            // Calling `on_swap_reg()` before checking these would be fatal.
            if (_arch_traits->has_inst_reg_swap(group) && this_phys_id != RAAssignment::kPhysNone) {
              ASMJIT_PROPAGATE(on_swap_reg(group, this_work_reg, this_work_id, this_phys_id, target_work_reg, target_work_id, target_phys_id));

              this_tied_reg->mark_use_done();
              if (this_tied_reg->is_write()) {
                _cur_assignment.make_dirty(group, this_work_id, target_phys_id);
              }
              use_pending_count--;

              // Double-hit.
              RATiedReg* target_tied_reg = RALocal_findTiedReg(tied_regs, count, target_work_reg);
              if (target_tied_reg && target_tied_reg->use_id() == this_phys_id) {
                target_tied_reg->mark_use_done();
                if (target_tied_reg->is_write()) {
                  _cur_assignment.make_dirty(group, target_work_id, this_phys_id);
                }
                use_pending_count--;
              }
              continue;
            }

            if (!must_swap) {
              continue;
            }

            // Only branched here if the previous iteration did nothing. This is essentially a SWAP operation without
            // having a dedicated instruction for that purpose (vector registers, etc...). The simplest way to handle
            // such case is to SPILL the target register or MOVE it to another register so the loop can continue.
            RegMask available_regs = _available_regs[group] & ~_cur_assignment.assigned(group);
            if (available_regs) {
              uint32_t tmp_reg_id = pick_best_suitable_register(group, available_regs);
              ASMJIT_ASSERT(tmp_reg_id != RAAssignment::kPhysNone);

              ASMJIT_PROPAGATE(on_move_reg(group, this_work_reg, this_work_id, tmp_reg_id, this_phys_id));
              _clobbered_regs[group] |= Support::bit_mask<RegMask>(tmp_reg_id);

              // NOTE: This register is not done, we have just moved it to another physical spot, and we will have to
              // move it again into the correct spot once it's free (since this is essentially doing a swap operation
              // via moves).
              break;
            }

            ASMJIT_PROPAGATE(on_spill_reg(group, target_work_reg, target_work_id, target_phys_id));
          }

          if (this_phys_id != RAAssignment::kPhysNone) {
            ASMJIT_PROPAGATE(on_move_reg(group, this_work_reg, this_work_id, target_phys_id, this_phys_id));

            this_tied_reg->mark_use_done();
            if (this_tied_reg->is_write()) {
              _cur_assignment.make_dirty(group, this_work_id, target_phys_id);
            }
            use_pending_count--;
          }
          else {
            ASMJIT_PROPAGATE(on_load_reg(group, this_work_reg, this_work_id, target_phys_id));

            this_tied_reg->mark_use_done();
            if (this_tied_reg->is_write()) {
              _cur_assignment.make_dirty(group, this_work_id, target_phys_id);
            }
            use_pending_count--;
          }
        }

        must_swap = (old_pending_count == use_pending_count);
      } while (use_pending_count);
    }

    // STEP 6
    // ------
    //
    // KILL registers marked as KILL/OUT.

    uint32_t out_pending_count = out_tied_count;
    if (out_tied_count) {
      for (i = 0; i < out_tied_count; i++) {
        RATiedReg* tied_reg = out_tied_regs[i];

        RAWorkReg* work_reg = tied_reg->work_reg();
        RAWorkId work_id = work_reg->work_id();
        uint32_t phys_id = _cur_assignment.work_to_phys_id(group, work_id);

        // Must check if it's allocated as KILL can be related to OUT (like KILL immediately after OUT, which could
        // mean the register is not assigned).
        if (phys_id != RAAssignment::kPhysNone) {
          _unassign_reg(group, work_id, phys_id);
          will_out &= ~Support::bit_mask<RegMask>(phys_id);
        }

        // We still maintain number of pending registers for OUT assignment. So, if this is only KILL, not OUT, we
        // can safely decrement it.
        out_pending_count -= !tied_reg->is_out();
      }
    }

    // STEP 7
    // ------
    //
    // SPILL registers that will be CLOBBERed. Since OUT and KILL were already processed this is used mostly to
    // handle function CALLs.

    if (will_out) {
      Support::BitWordIterator<RegMask> it(will_out);
      do {
        uint32_t phys_id = it.next();
        RAWorkId work_id = _cur_assignment.phys_to_work_id(group, phys_id);

        if (work_id == kBadWorkId) {
          continue;
        }

        ASMJIT_PROPAGATE(on_spill_reg(group, work_reg_by_id(work_id), work_id, phys_id));
      } while (it.has_next());
    }

    // STEP 8
    // ------
    //
    // Duplication.

    for (i = 0; i < dup_tied_count; i++) {
      RATiedReg* tied_reg = dup_tied_regs[i];
      RAWorkReg* work_reg = tied_reg->work_reg();
      uint32_t src_id = tied_reg->use_id();

      Support::BitWordIterator<RegMask> it(tied_reg->use_reg_mask());
      while (it.has_next()) {
        uint32_t dst_id = it.next();
        if (dst_id == src_id) {
          continue;
        }
        ASMJIT_PROPAGATE(_pass.emit_move(work_reg, dst_id, src_id));
      }
    }

    // STEP 9
    // ------
    //
    // Vector registers can be clobbered partially by invoke - find if that's the case and clobber when necessary.

    if (node->is_invoke() && group == RegGroup::kVec) {
      const InvokeNode* invoke_node = node->as<InvokeNode>();

      RegMask maybe_clobbered_regs = invoke_node->detail().call_conv().preserved_regs(group) & _cur_assignment.assigned(group);
      if (maybe_clobbered_regs) {
        uint32_t save_restore_vec_size = invoke_node->detail().call_conv().save_restore_reg_size(group);
        Support::BitWordIterator<RegMask> it(maybe_clobbered_regs);

        do {
          uint32_t phys_id = it.next();
          RAWorkId work_id = _cur_assignment.phys_to_work_id(group, phys_id);

          RAWorkReg* work_reg = work_reg_by_id(work_id);
          uint32_t virt_size = work_reg->virt_reg()->virt_size();

          if (virt_size > save_restore_vec_size) {
            ASMJIT_PROPAGATE(on_spill_reg(group, work_reg, work_id, phys_id));
          }

        } while (it.has_next());
      }
    }

    // STEP 10
    // -------
    //
    // Assign OUT registers.

    if (out_pending_count) {
      // Live registers, we need a separate register (outside of `_cur_assignment) to hold these because of KILLed
      // registers. If we KILL a register here it will go out from `_cur_assignment`, but we cannot assign to it in
      // here.
      RegMask live_regs = _cur_assignment.assigned(group);

      // Must avoid as they have been already OUTed (added during the loop).
      RegMask out_regs = 0;

      // Must avoid as they collide with already allocated ones.
      RegMask avoid_regs = will_use & ~clobbered_by_inst;

      // Assign the best possible OUT ids of all consecutives.
      if (consecutive_count) {
        RATiedReg* lead = consecutive_regs[0];
        if (lead->is_out_consecutive()) {
          uint32_t best_score = 0;
          uint32_t best_lead_reg = 0xFFFFFFFF;
          RegMask allocable_regs = _available_regs[group] & ~(out_regs | avoid_regs);

          Support::BitWordIterator<uint32_t> it(lead->out_reg_mask());
          while (it.has_next()) {
            uint32_t reg_index = it.next();
            if (Support::bit_test(lead->out_reg_mask(), reg_index)) {
              uint32_t score = 15;

              for (i = 0; i < consecutive_count; i++) {
                uint32_t consecutive_index = reg_index + i;
                if (!Support::bit_test(allocable_regs, consecutive_index)) {
                  score = 0;
                  break;
                }

                RAWorkReg* work_reg = consecutive_regs[i]->work_reg();
                score += uint32_t(work_reg->home_reg_id() == consecutive_index);
              }

              if (score > best_score) {
                best_score = score;
                best_lead_reg = reg_index;
              }
            }
          }

          if (best_lead_reg == 0xFFFFFFFF) {
            return make_error(Error::kConsecutiveRegsAllocation);
          }

          for (i = 0; i < consecutive_count; i++) {
            uint32_t consecutive_index = best_lead_reg + i;
            RATiedReg* tied_reg = consecutive_regs[i];
            tied_reg->set_out_id(consecutive_index);
          }
        }
      }

      // Allocate OUT registers.
      for (i = 0; i < out_tied_count; i++) {
        RATiedReg* tied_reg = out_tied_regs[i];
        if (!tied_reg->is_out()) {
          continue;
        }

        RegMask avoid_out = avoid_regs;
        if (tied_reg->is_unique()) {
          avoid_out |= will_use;
        }

        RAWorkReg* work_reg = tied_reg->work_reg();
        RAWorkId work_id = work_reg->work_id();
        uint32_t assigned_id = _cur_assignment.work_to_phys_id(group, work_id);

        if (assigned_id != RAAssignment::kPhysNone) {
          _unassign_reg(group, work_id, assigned_id);
        }

        uint32_t phys_id = tied_reg->out_id();
        if (phys_id == RAAssignment::kPhysNone) {
          RegMask allocable_regs = tied_reg->out_reg_mask() & ~(out_regs | avoid_out);

          if (!(allocable_regs & ~live_regs)) {
            // There are no more registers, decide which one to spill.
            RAWorkId spill_work_id;
            phys_id = decide_on_spill_for(group, work_reg, allocable_regs & live_regs, &spill_work_id);
            ASMJIT_PROPAGATE(on_spill_reg(group, work_reg_by_id(spill_work_id), spill_work_id, phys_id));
          }
          else {
            phys_id = decide_on_assignment(group, work_reg, RAAssignment::kPhysNone, allocable_regs & ~live_regs);
          }
        }

        // OUTs are CLOBBERed thus cannot be ASSIGNed right now.
        ASMJIT_ASSERT(!_cur_assignment.is_phys_assigned(group, phys_id));

        if (!tied_reg->is_kill()) {
          ASMJIT_PROPAGATE(_assign_reg(group, work_id, phys_id, true));
        }

        tied_reg->set_out_id(phys_id);
        tied_reg->mark_out_done();

        out_regs |= Support::bit_mask<RegMask>(phys_id);
        live_regs &= ~Support::bit_mask<RegMask>(phys_id);
        out_pending_count--;
      }

      clobbered_by_inst |= out_regs;
      ASMJIT_ASSERT(out_pending_count == 0);
    }

    _clobbered_regs[group] |= clobbered_by_inst;
  }

  return Error::kOk;
}

Error RALocalAllocator::spill_after_allocation(InstNode* node) noexcept {
  // This is experimental feature that would spill registers that don't have home-id and are last in this basic block.
  // This prevents saving these regs in other basic blocks and then restoring them (mostly relevant for loops).
  RAInst* ra_inst = node->pass_data<RAInst>();
  uint32_t count = ra_inst->tied_count();

  for (uint32_t i = 0; i < count; i++) {
    RATiedReg* tied_reg = ra_inst->tied_at(i);

    if (tied_reg->is_last()) {
      RAWorkReg* work_reg = tied_reg->work_reg();

      if (!work_reg->has_home_reg_id()) {
        RAWorkId work_id = work_reg->work_id();
        RegGroup group = work_reg->group();
        uint32_t assigned_id = _cur_assignment.work_to_phys_id(group, work_id);

        if (assigned_id != RAAssignment::kPhysNone) {
          _cc.set_cursor(node);
          ASMJIT_PROPAGATE(on_spill_reg(group, work_reg, work_id, assigned_id));
        }
      }
    }
  }

  return Error::kOk;
}

Error RALocalAllocator::alloc_branch(InstNode* node, RABlock* target, RABlock* cont) noexcept {
  // TODO: This should be used to make the branch allocation better.
  Support::maybe_unused(cont);

  // The cursor must point to the previous instruction for a possible instruction insertion.
  _cc.set_cursor(node->prev());

  // Use TryMode of `switch_to_assignment()` if possible.
  if (target->has_entry_assignment()) {
    ASMJIT_PROPAGATE(switch_to_assignment(target->entry_phys_to_work_map(), target->live_in(), target->is_allocated(), true));
  }

  ASMJIT_PROPAGATE(alloc_instruction(node));
  ASMJIT_PROPAGATE(spill_regs_before_entry(target));

  if (target->has_entry_assignment()) {
    BaseNode* injection_point = _pass._injection_end->prev();
    BaseNode* prev_cursor = _cc.set_cursor(injection_point);

    _tmp_assignment.copy_from(_cur_assignment);
    ASMJIT_PROPAGATE(switch_to_assignment(target->entry_phys_to_work_map(), target->live_in(), target->is_allocated(), false));

    BaseNode* cur_cursor = _cc.cursor();
    if (cur_cursor != injection_point) {
      // Additional instructions emitted to switch from the current state to the `target` state. This means
      // that we have to move these instructions into an independent code block and patch the jump location.
      Operand& target_op = node->op(node->op_count() - 1);
      if (ASMJIT_UNLIKELY(!target_op.is_label())) {
        return make_error(Error::kInvalidState);
      }

      Label trampoline = _cc.new_label();
      Label saved_target = target_op.as<Label>();

      // Patch `target` to point to the `trampoline` we just created.
      target_op = trampoline;

      // Clear a possible SHORT form as we have no clue now if the SHORT form would be encodable after patching
      // the target to `trampoline` (X86 specific).
      node->clear_options(InstOptions::kShortForm);

      // Finalize the switch assignment sequence.
      ASMJIT_PROPAGATE(_pass.emit_jump(saved_target));
      _cc.set_cursor(injection_point);
      _cc.bind(trampoline);

      if (_pass._injection_start == nullptr) {
        _pass._injection_start = injection_point->next();
      }
    }

    _cc.set_cursor(prev_cursor);
    _cur_assignment.swap(_tmp_assignment);
  }
  else {
    ASMJIT_PROPAGATE(_pass.set_block_entry_assignment(target, block(), _cur_assignment));
  }

  return Error::kOk;
}

Error RALocalAllocator::alloc_jump_table(InstNode* node, Span<RABlock*> targets, RABlock* cont) noexcept {
  // TODO: Do we really need to use `cont`?
  Support::maybe_unused(cont);

  if (targets.is_empty()) {
    return make_error(Error::kInvalidState);
  }

  // The cursor must point to the previous instruction for a possible instruction insertion.
  _cc.set_cursor(node->prev());

  // All `targets` should have the same shared_assignment_id, we just read the first.
  RABlock* any_target = targets[0];
  if (!any_target->has_shared_assignment_id()) {
    return make_error(Error::kInvalidState);
  }

  RASharedAssignment& shared_assignment = _pass._shared_assignments[any_target->shared_assignment_id()];

  ASMJIT_PROPAGATE(alloc_instruction(node));

  if (!shared_assignment.is_empty()) {
    ASMJIT_PROPAGATE(switch_to_assignment(
      shared_assignment.phys_to_work_map(),
      shared_assignment.live_in(),
      true,  // Read-only.
      false  // Try-mode.
    ));
  }

  ASMJIT_PROPAGATE(spill_regs_before_entry(any_target));

  if (shared_assignment.is_empty()) {
    ASMJIT_PROPAGATE(_pass.set_block_entry_assignment(any_target, block(), _cur_assignment));
  }

  return Error::kOk;
}

// RALocalAllocator - Decision Making
// ==================================

uint32_t RALocalAllocator::decide_on_assignment(RegGroup group, RAWorkReg* work_reg, uint32_t phys_id, RegMask allocable_regs) const noexcept {
  ASMJIT_ASSERT(allocable_regs != 0);
  Support::maybe_unused(group, phys_id);

  // Prefer home register id, if possible.
  if (work_reg->has_home_reg_id()) {
    uint32_t home_id = work_reg->home_reg_id();
    if (Support::bit_test(allocable_regs, home_id)) {
      return home_id;
    }
  }

  // Prefer registers used upon block entries.
  RegMask previously_assigned_regs = work_reg->allocated_mask();
  if (allocable_regs & previously_assigned_regs) {
    allocable_regs &= previously_assigned_regs;
  }

  return pick_best_suitable_register(group, allocable_regs);
}

uint32_t RALocalAllocator::decide_on_reassignment(RegGroup group, RAWorkReg* work_reg, uint32_t phys_id, RegMask allocable_regs, RAInst* ra_inst) const noexcept {
  ASMJIT_ASSERT(allocable_regs != 0);
  Support::maybe_unused(phys_id);

  // Prefer reassignment back to HomeId, if possible.
  if (work_reg->has_home_reg_id()) {
    if (Support::bit_test(allocable_regs, work_reg->home_reg_id())) {
      return work_reg->home_reg_id();
    }
  }

  // Prefer assignment to a temporary register in case this register is killed by the instruction (or has an out slot).
  const RATiedReg* tied_reg = ra_inst->tied_reg_for_work_reg(group, work_reg);
  if (tied_reg && tied_reg->is_out_or_kill()) {
    return Support::ctz(allocable_regs);
  }

  // Prefer reassignment if this register is only used within a single basic block.
  if (work_reg->is_within_single_basic_block()) {
    RegMask filtered_regs = allocable_regs & ~work_reg->clobber_survival_mask();
    if (filtered_regs) {
      return pick_best_suitable_register(group, filtered_regs);
    }
  }

  // TODO: [Register Allocator] This could be improved.

  // Decided to SPILL.
  return RAAssignment::kPhysNone;
}

uint32_t RALocalAllocator::decide_on_spill_for(RegGroup group, RAWorkReg* work_reg, RegMask spillable_regs, RAWorkId* spill_work_id) const noexcept {
  // May be used in the future to decide which register would be best to spill so `work_id` can be assigned.
  Support::maybe_unused(work_reg);
  ASMJIT_ASSERT(spillable_regs != 0);

  Support::BitWordIterator<RegMask> it(spillable_regs);
  uint32_t best_phys_id = it.next();
  RAWorkId best_work_id = _cur_assignment.phys_to_work_id(group, best_phys_id);

  // Avoid calculating the cost model if there is only one spillable register.
  if (it.has_next()) {
    uint32_t best_cost = calc_spill_cost(group, work_reg_by_id(best_work_id), best_phys_id);
    do {
      uint32_t local_phys_id = it.next();
      RAWorkId local_work_id = _cur_assignment.phys_to_work_id(group, local_phys_id);
      uint32_t local_cost = calc_spill_cost(group, work_reg_by_id(local_work_id), local_phys_id);

      if (local_cost < best_cost) {
        best_cost = local_cost;
        best_phys_id = local_phys_id;
        best_work_id = local_work_id;
      }
    } while (it.has_next());
  }

  *spill_work_id = best_work_id;
  return best_phys_id;
}

ASMJIT_END_NAMESPACE

#endif // !ASMJIT_NO_COMPILER
