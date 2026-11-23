// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_RAPASS_P_H_INCLUDED
#define ASMJIT_CORE_RAPASS_P_H_INCLUDED

#include <asmjit/core/api-config.h>
#ifndef ASMJIT_NO_COMPILER

#include <asmjit/core/compiler.h>
#include <asmjit/core/emithelper_p.h>
#include <asmjit/core/raassignment_p.h>
#include <asmjit/core/racfgblock_p.h>
#include <asmjit/core/radefs_p.h>
#include <asmjit/core/rainst_p.h>
#include <asmjit/core/rastack_p.h>
#include <asmjit/support/support_p.h>

ASMJIT_BEGIN_NAMESPACE

//! \cond INTERNAL
//! \addtogroup asmjit_ra
//! \{

//! Register allocation pass used by `BaseCompiler`.
class BaseRAPass : public Pass {
public:
  ASMJIT_NONCOPYABLE(BaseRAPass)
  using Base = Pass;

  //! \name Constants
  //! \{

  static inline constexpr uint32_t kCallArgWeight = 80;

  //! \}

  //! \name Types
  //! \{

  using PhysToWorkMap = RAAssignment::PhysToWorkMap;
  using WorkToPhysMap = RAAssignment::WorkToPhysMap;

  //! \}

  //! \name Members
  //! \{

  //! Allocator that uses arena passed to `run_on_function()`.
  Arena* _arena {};
  //! Emit helper.
  BaseEmitHelper* _emit_helper_ptr = nullptr;

  //! Logger, disabled if null.
  Logger* _logger = nullptr;
  //! Format options, copied from Logger, or zeroed if there is no logger.
  FormatOptions _format_options {};
  //! Diagnostic options, copied from Emitter, or zeroed if there is no logger.
  DiagnosticOptions _diagnostic_options {};

  //! Function being processed.
  FuncNode* _func = nullptr;
  //! Stop node.
  BaseNode* _stop = nullptr;
  //! Start of the code that was injected.
  BaseNode* _injection_start = nullptr;
  //! End of the code that was injected.
  BaseNode* _injection_end = nullptr;

  //! Blocks (first block is the entry, always exists).
  ArenaVector<RABlock*> _blocks {};
  //! Function exit blocks (usually one, but can contain more).
  ArenaVector<RABlock*> _exits {};
  //! Post order view (POV).
  ArenaVector<RABlock*> _pov {};

  //! Number of instruction nodes.
  uint32_t _instruction_count = 0;
  //! Number of created blocks (internal).
  uint32_t _created_block_count = 0;

  //! Shared assignment blocks.
  ArenaVector<RASharedAssignment> _shared_assignments {};

  //! Timestamp generator (incremental).
  mutable uint64_t _last_timestamp = 0;

  //! Architecture traits.
  const ArchTraits* _arch_traits = nullptr;
  //! Index to physical registers in `RAAssignment::PhysToWorkMap`.
  RARegIndex _phys_reg_index = RARegIndex();
  //! Count of physical registers in `RAAssignment::PhysToWorkMap`.
  RARegCount _phys_reg_count = RARegCount();
  //! Total number of physical registers.
  uint32_t _phys_reg_total = 0;
  //! Indexes of a possible scratch registers that can be selected if necessary.
  Support::Array<uint8_t, 2> _scratch_reg_indexes {};

  //! Registers available for allocation.
  RARegMask _available_regs = RARegMask();
  //! Registers clobbered by the function.
  RARegMask _clobbered_regs = RARegMask();

  //! Work registers (registers used by the function).
  ArenaVector<RAWorkReg*> _work_regs;
  //! Work registers per register group.
  Support::Array<ArenaVector<RAWorkReg*>, Globals::kNumVirtGroups> _work_regs_of_group;

  //! Count of work registers that live across multiple basic blocks.
  uint32_t _multi_work_reg_count = 0u;
  //! Count of work registers, incremented before allocating _work_regs array.
  uint32_t _total_work_reg_count = 0u;

  //! Register allocation strategy per register group.
  Support::Array<RAStrategy, Globals::kNumVirtGroups> _strategy;
  //! Global max live-count (from all blocks) per register group.
  RALiveCount _global_live_max_count = RALiveCount();
  //! Global live spans per register group.
  Support::Array<RALiveSpans*, Globals::kNumVirtGroups> _global_live_spans {};
  //! Temporary stack slot.
  Operand _temporary_mem = Operand();

  //! Stack pointer.
  Reg _sp = Reg();
  //! Frame pointer.
  Reg _fp = Reg();
  //! Stack manager.
  RAStackAllocator _stack_allocator {};
  //! Function arguments assignment.
  FuncArgsAssignment _args_assignment {};
  //! Some StackArgs have to be assigned to StackSlots.
  uint32_t _num_stack_args_to_stack_slots = 0;

  //! Maximum name-size computed from all WorkRegs.
  uint32_t _max_work_reg_name_size = 0;
  //! Temporary string builder used to format comments.
  StringTmp<192> _tmp_string;

  //! \}

  //! \name Construction & Destruction
  //! \{

  BaseRAPass(BaseCompiler& cc) noexcept;
  ~BaseRAPass() noexcept override;

  //! \}

  //! \name Accessors
  //! \{

  //! Returns the associated `BaseCompiler`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BaseCompiler& cc() const noexcept { return static_cast<BaseCompiler&>(_cb); }

  //! Returns \ref Logger passed to \ref run().
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Logger* logger() const noexcept { return _logger; }

  //! Returns either a valid logger if the given `option` is set and logging is enabled, or nullptr.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Logger* logger_if(DiagnosticOptions option) const noexcept { return Support::test(_diagnostic_options, option) ? _logger : nullptr; }

  //! Returns whether the diagnostic `option` is enabled.
  //!
  //! \note Returns false if there is no logger (as diagnostics without logging make no sense).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_diagnostic_option(DiagnosticOptions option) const noexcept { return Support::test(_diagnostic_options, option); }

  //! Returns \ref Arena passed to \ref run().
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Arena& arena() const noexcept { return *_arena; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<RASharedAssignment> shared_assignments() const { return _shared_assignments.as_span(); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t shared_assignment_count() const noexcept { return _shared_assignments.size(); }

  //! Returns the current function node.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG FuncNode* func() const noexcept { return _func; }

  //! Returns the stop of the current function.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BaseNode* stop() const noexcept { return _stop; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t end_position() const noexcept { return _instruction_count * 2u; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const RARegMask& available_regs() const noexcept { return _available_regs; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const RARegMask& clobbered_regs() const noexcept { return _clobbered_regs; }

  //! \}

  //! \name Utilities
  //! \{

  inline void make_unavailable(RegGroup group, uint32_t reg_id) noexcept {
    _available_regs[group] &= ~Support::bit_mask<RegMask>(reg_id);
  }

  inline void make_unavailable(const RARegMask::RegMasks& regs) noexcept {
    _available_regs.clear(regs);
  }

  //! \}

  //! \name Run
  //! \{

  Error run(Arena& arena, Logger* logger) override;

  //! Runs the register allocator for the given `func`.
  Error run_on_function(Arena& arena, FuncNode* func, bool last) noexcept;

  //! Performs all allocation steps sequentially, called by `run_on_function()`.
  Error on_perform_all_steps() noexcept;

  //! \}

  //! \name Events
  //! \{

  //! Called by \ref run_on_function() before the register allocation to initialize
  //! architecture-specific data and constraints.
  virtual void on_init() noexcept;

  //! Called by \ref run_on_function() after register allocation to clean everything
  //! up. Called even if the register allocation failed.
  virtual void on_done() noexcept;

  //! \}

  //! \name CFG - Basic-Block Management
  //! \{

  //! Returns the function's entry block.
  [[nodiscard]]
  inline RABlock* entry_block() noexcept {
    ASMJIT_ASSERT(!_blocks.is_empty());
    return _blocks[0];
  }

  //! \overload
  [[nodiscard]]
  inline const RABlock* entry_block() const noexcept {
    ASMJIT_ASSERT(!_blocks.is_empty());
    return _blocks[0];
  }

  //! Returns all basic blocks of this function.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<RABlock*> blocks() noexcept { return _blocks.as_span(); }

  //! Returns the count of basic blocks (returns size of `_blocks` array).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t block_count() const noexcept { return _blocks.size(); }

  //! Returns the count of reachable basic blocks (returns size of `_pov` array).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t reachable_block_count() const noexcept { return _pov.size(); }

  //! Tests whether the CFG has dangling blocks - these were created by `new_block()`, but not added to CFG through
  //! `add_block()`. If `true` is returned and the  CFG is constructed it means that something is missing and it's
  //! incomplete.
  //!
  //! \note This is only used to check if the number of created blocks matches the number of added blocks.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_dangling_blocks() const noexcept { return _created_block_count != block_count(); }

  //! Gest a next timestamp to be used to mark CFG blocks.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RABlockTimestamp next_timestamp() const noexcept { return RABlockTimestamp(++_last_timestamp); }

  //! Creates a new `RABlock` instance.
  //!
  //! \note New blocks don't have ID assigned until they are added to the block array by calling `add_block()`.
  [[nodiscard]]
  RABlock* new_block(BaseNode* initial_node = nullptr) noexcept;

  //! Tries to find a neighboring LabelNode (without going through code) that is already connected with `RABlock`.
  //! If no label is found then a new RABlock is created and assigned to all possible labels in a backward direction.
  [[nodiscard]]
  RABlock* new_block_or_existing_at(LabelNode* label_node, BaseNode** stopped_at = nullptr) noexcept;

  //! Adds the given `block` to the block list and assign it a unique block id.
  [[nodiscard]]
  Error add_block(RABlock* block) noexcept;

  [[nodiscard]]
  inline Error add_exit_block(RABlock* block) noexcept {
    block->add_flags(RABlockFlags::kIsFuncExit);
    return _exits.append(arena(), block);
  }

  [[nodiscard]]
  ASMJIT_INLINE RAInst* new_ra_inst(InstRWFlags inst_rw_flags, RATiedFlags flags, uint32_t tied_reg_count, const RARegMask& clobbered_regs) noexcept {
    void* p = arena().alloc_oneshot(RAInst::size_of(tied_reg_count));
    if (ASMJIT_UNLIKELY(!p)) {
      return nullptr;
    }
    return new(Support::PlacementNew{p}) RAInst(inst_rw_flags, flags, tied_reg_count, clobbered_regs);
  }

  [[nodiscard]]
  ASMJIT_INLINE Error assign_ra_inst(BaseNode* node, RABlock* block, RAInstBuilder& ib) noexcept {
    RABlockId block_id = block->block_id();
    uint32_t tied_reg_count = ib.tied_reg_count();
    RAInst* ra_inst = new_ra_inst(ib.inst_rw_flags(), ib.aggregated_flags(), tied_reg_count, ib._clobbered);

    if (ASMJIT_UNLIKELY(!ra_inst)) {
      return make_error(Error::kOutOfMemory);
    }

    RARegIndex index;
    RATiedFlags flags_filter = ~ib.forbidden_flags();

    index.build_indexes(ib._count);
    ra_inst->_tied_index = index;
    ra_inst->_tied_count = ib._count;

    for (uint32_t i = 0; i < tied_reg_count; i++) {
      RATiedReg* tied_reg = ib[i];
      RAWorkReg* work_reg = tied_reg->work_reg();

      RegGroup group = work_reg->group();
      work_reg->reset_tied_reg();

      if (work_reg->_single_basic_block_id != block_id) {
        if (Support::bool_or(work_reg->_single_basic_block_id != kBadBlockId, !tied_reg->is_write_only())) {
          work_reg->add_flags(RAWorkRegFlags::kMultiBlockUse);
        }

        work_reg->_single_basic_block_id = block_id;
        tied_reg->add_flags(RATiedFlags::kFirst);
      }

      if (tied_reg->has_use_id()) {
        block->add_flags(RABlockFlags::kHasFixedRegs);
        ra_inst->_used_regs[group] |= Support::bit_mask<RegMask>(tied_reg->use_id());
      }

      if (tied_reg->has_out_id()) {
        block->add_flags(RABlockFlags::kHasFixedRegs);
      }

      RATiedReg& dst = ra_inst->_tied_regs[index.get(group)];
      index.add(group);

      dst = *tied_reg;
      dst._flags &= flags_filter;

      if (!tied_reg->is_duplicate()) {
        dst._use_reg_mask &= ~ib._used[group];
      }

    }

    node->set_pass_data<RAInst>(ra_inst);
    return Error::kOk;
  }

  //! \}

  //! \name CFG - Build CFG
  //! \{

  //! Traverse the whole function and do the following:
  //!
  //!   1. Construct CFG (represented by `RABlock`) by populating `_blocks` and `_exits`. Blocks describe the control
  //!      flow of the function and contain some additional information that is used by the register allocator.
  //!
  //!   2. Remove unreachable code immediately. This is not strictly necessary for BaseCompiler itself as the register
  //!      allocator cannot reach such nodes, but keeping instructions that use virtual registers would fail during
  //!      instruction encoding phase (Assembler).
  //!
  //!   3. `RAInst` is created for each `InstNode` or compatible. It contains information that is essential for further
  //!      analysis and register allocation.
  //!
  //! Use `RACFGBuilderT` template that provides the necessary boilerplate.
  [[nodiscard]]
  virtual Error build_cfg_nodes() noexcept;

  //! Called after the CFG is built.
  [[nodiscard]]
  Error init_shared_assignments(Span<uint32_t> shared_assignments_map) noexcept;

  //! \}

  //! \name CFG - Views Order
  //! \{

  //! Constructs CFG views (only POV at the moment).
  [[nodiscard]]
  Error build_cfg_views() noexcept;

  //! \}

  //! \name CFG - Dominators
  //! \{

  // Terminology:
  //   - A node `X` dominates a node `Z` if any path from the entry point to `Z` has to go through `X`.
  //   - A node `Z` post-dominates a node `X` if any path from `X` to the end of the graph has to go through `Z`.

  //! Constructs a dominator-tree from CFG.
  [[nodiscard]]
  Error build_cfg_dominators() noexcept;

  [[nodiscard]]
  bool _strictly_dominates(const RABlock* a, const RABlock* b) const noexcept;

  [[nodiscard]]
  const RABlock* _nearest_common_dominator(const RABlock* a, const RABlock* b) const noexcept;

  //! Tests whether the basic block `a` dominates `b` - non-strict, returns true when `a == b`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool dominates(const RABlock* a, const RABlock* b) const noexcept { return a == b ? true : _strictly_dominates(a, b); }

  //! Tests whether the basic block `a` dominates `b` - strict dominance check, returns false when `a == b`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool strictly_dominates(const RABlock* a, const RABlock* b) const noexcept { return a == b ? false : _strictly_dominates(a, b); }

  //! Returns a nearest common dominator of `a` and `b`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RABlock* nearest_common_dominator(RABlock* a, RABlock* b) const noexcept { return const_cast<RABlock*>(_nearest_common_dominator(a, b)); }

  //! Returns a nearest common dominator of `a` and `b` (const).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const RABlock* nearest_common_dominator(const RABlock* a, const RABlock* b) const noexcept { return _nearest_common_dominator(a, b); }

  //! \}

  //! \name CFG - Utilities
  //! \{

  [[nodiscard]]
  Error remove_unreachable_code() noexcept;

  //! Returns `node` or some node after that is ideal for beginning a new block. This function is mostly used after
  //! a conditional or unconditional jump to select the successor node. In some cases the next node could be a label,
  //! which means it could have assigned some block already.
  [[nodiscard]]
  BaseNode* find_successor_starting_at(BaseNode* node) noexcept;

  //! Returns `true` of the `node` can flow to `target` without reaching code nor data. It's used to eliminate jumps
  //! to labels that are next right to them.
  [[nodiscard]]
  bool is_next_to(BaseNode* node, BaseNode* target) noexcept;

  //! \}

  //! \name Virtual Register Management
  //! \{

  //! Returns a native size of the general-purpose register of the target architecture.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t register_size() const noexcept { return _sp.size(); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RAWorkReg* work_reg_by_id(RAWorkId work_id) const noexcept { return _work_regs[uint32_t(work_id)]; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG ArenaVector<RAWorkReg*>& work_regs() noexcept { return _work_regs; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG ArenaVector<RAWorkReg*>& work_regs(RegGroup group) noexcept { return _work_regs_of_group[group]; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const ArenaVector<RAWorkReg*>& work_regs() const noexcept { return _work_regs; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const ArenaVector<RAWorkReg*>& work_regs(RegGroup group) const noexcept { return _work_regs_of_group[group]; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t multi_work_reg_count() const noexcept { return _multi_work_reg_count; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t work_reg_count() const noexcept { return _total_work_reg_count; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t work_reg_count(RegGroup group) const noexcept { return _work_regs_of_group[group].size(); }

  inline void _build_phys_index() noexcept {
    _phys_reg_index.build_indexes(_phys_reg_count);
    _phys_reg_total = _phys_reg_index.get(RegGroup::kMaxVirt) + _phys_reg_count.get(RegGroup::kMaxVirt);
  }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t phys_reg_index(RegGroup group) const noexcept { return _phys_reg_index.get(group); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t phys_reg_total() const noexcept { return _phys_reg_total; }

  [[nodiscard]]
  Error _as_work_reg(RAWorkReg** out, VirtReg* virt_reg) noexcept;

  //! Creates `RAWorkReg` data for the given `virt_reg`. The function does nothing if `virt_reg` already contains
  //! link to `RAWorkReg`.
  [[nodiscard]]
  ASMJIT_INLINE Error as_work_reg(RAWorkReg** out, VirtReg* virt_reg) noexcept {
    RAWorkReg* work_reg = virt_reg->work_reg();
    if (ASMJIT_LIKELY(work_reg)) {
      *out = work_reg;
      return Error::kOk;
    }
    else {
      return _as_work_reg(out, virt_reg);
    }
  }

  [[nodiscard]]
  ASMJIT_INLINE Error virt_index_as_work_reg(RAWorkReg** out, uint32_t virt_index) noexcept {
    Span<VirtReg*> virt_regs = cc().virt_regs();
    if (ASMJIT_UNLIKELY(virt_index >= virt_regs.size()))
      return make_error(Error::kInvalidVirtId);
    return as_work_reg(out, virt_regs[virt_index]);
  }

  [[nodiscard]]
  RAStackSlot* _create_stack_slot(RAWorkReg* work_reg) noexcept;

  [[nodiscard]]
  inline RAStackSlot* get_or_create_stack_slot(RAWorkReg* work_reg) noexcept {
    RAStackSlot* slot = work_reg->stack_slot();
    return slot ? slot : _create_stack_slot(work_reg);
  }

  [[nodiscard]]
  inline BaseMem work_reg_as_mem(RAWorkReg* work_reg) noexcept {
    (void)get_or_create_stack_slot(work_reg);
    return BaseMem(OperandSignature::from_op_type(OperandType::kMem) |
                   OperandSignature::from_mem_base_type(_sp.reg_type()) |
                   OperandSignature::from_bits(OperandSignature::kMemRegHomeFlag),
                   work_reg->virt_id(), 0, 0);
  }

  [[nodiscard]]
  WorkToPhysMap* new_work_to_phys_map() noexcept;

  [[nodiscard]]
  PhysToWorkMap* new_phys_to_work_map() noexcept;

  [[nodiscard]]
  inline PhysToWorkMap* clone_phys_to_work_map(const PhysToWorkMap* map) noexcept {
    return static_cast<PhysToWorkMap*>(arena().dup(map, PhysToWorkMap::size_of(_phys_reg_total)));
  }

  [[nodiscard]]
  Error build_reg_ids() noexcept;

  //! \name Liveness Analysis & Statistics
  //! \{

  //! 1. Calculates GEN/KILL/IN/OUT of each block.
  //! 2. Calculates live spans and basic statistics of each work register.
  [[nodiscard]]
  Error build_liveness() noexcept;

  //! Assigns arg_index to WorkRegs. Must be called after the liveness analysis
  //! finishes as it checks whether the argument is live upon entry.
  [[nodiscard]]
  Error assign_arg_index_to_work_regs() noexcept;

  //! \}

  //! \name Register Allocation - Global
  //! \{

  //! Runs a global register allocator.
  [[nodiscard]]
  Error run_global_allocator() noexcept;

  //! Initializes data structures used for global live spans.
  [[nodiscard]]
  Error init_global_live_spans() noexcept;

  [[nodiscard]]
  Error bin_pack(RegGroup group) noexcept;

  //! \}

  //! \name Register Allocation - Local
  //! \{

  //! Runs a local register allocator.
  [[nodiscard]]
  Error run_local_allocator() noexcept;

  [[nodiscard]]
  Error set_block_entry_assignment(RABlock* block, const RABlock* from_block, const RAAssignment& from_assignment) noexcept;

  [[nodiscard]]
  Error set_shared_assignment(uint32_t shared_assignment_id, const RAAssignment& from_assignment) noexcept;

  //! Called after the RA assignment has been assigned to a block.
  //!
  //! This cannot change the assignment, but can examine it.
  [[nodiscard]]
  Error block_entry_assigned(const PhysToWorkMap* phys_to_work_map) noexcept;

  //! \}

  //! \name Register Allocation Utilities
  //! \{

  [[nodiscard]]
  Error use_temporary_mem(BaseMem& out, uint32_t size, uint32_t alignment) noexcept;

  //! \}

  //! \name Function Prolog & Epilog
  //! \{

  [[nodiscard]]
  virtual Error update_stack_frame() noexcept;

  [[nodiscard]]
  Error _mark_stack_args_to_keep() noexcept;

  [[nodiscard]]
  Error _update_stack_args() noexcept;

  [[nodiscard]]
  Error insert_prolog_epilog() noexcept;

  //! \}

  //! \name Instruction Rewriter
  //! \{

  template<typename Lambda>
  ASMJIT_INLINE Error rewrite_iterate(Lambda&& fn) noexcept {
    // First iteration is not a block - it's possible register/stack changes at the end of the function.
    RABlock* block = nullptr;

    BaseNode* first = _injection_start;
    BaseNode* stop = _injection_end;

    Span<RABlock*> pov = _pov.as_span();
    size_t pov_index = pov.size();

    // If no injection happened, just do basic blocks.
    if (first == nullptr) {
      // The size of POV is always greater than zero, because there is always at least one basic block.
      ASMJIT_ASSERT(pov_index > 0u);

      block = pov[--pov_index];
      first = block->first();
      stop = block->last()->next();
    }

    for (;;) {
      ASMJIT_PROPAGATE(fn(first, stop, block));

      if (!pov_index) {
        break;
      }

      block = pov[--pov_index];
      first = block->first();
      stop = block->last()->next();
    }

    return Error::kOk;
  }

  [[nodiscard]]
  virtual Error rewrite() noexcept;

  //! \}

#ifndef ASMJIT_NO_LOGGING
  //! \name Logging
  //! \{

  Error annotate_code() noexcept;
  Error dump_block_ids(String& sb, Span<RABlock*> blocks) noexcept;
  Error dump_block_liveness(String& sb, const RABlock* block) noexcept;
  Error dump_live_spans(String& sb) noexcept;

  //! \}
#endif

  //! \name Emit
  //! \{

  [[nodiscard]]
  virtual Error emit_move(RAWorkReg* work_reg, uint32_t dst_phys_id, uint32_t src_phys_id) noexcept;

  [[nodiscard]]
  virtual Error emit_swap(RAWorkReg* a_reg, uint32_t a_phys_id, RAWorkReg* b_reg, uint32_t b_phys_id) noexcept;

  [[nodiscard]]
  virtual Error emit_load(RAWorkReg* work_reg, uint32_t dst_phys_id) noexcept;

  [[nodiscard]]
  virtual Error emit_save(RAWorkReg* work_reg, uint32_t src_phys_id) noexcept;

  [[nodiscard]]
  virtual Error emit_jump(const Label& label) noexcept;

  [[nodiscard]]
  virtual Error emit_pre_call(InvokeNode* invoke_node) noexcept;

  //! \}
};

// Late implementation of RABlock member functions:
inline Arena& RABlock::arena() const noexcept { return _ra->arena(); }

inline RegMask RABlock::entry_scratch_gp_regs() const noexcept {
  RegMask regs = _entry_scratch_gp_regs;
  if (has_shared_assignment_id()) {
    regs = _ra->_shared_assignments[_shared_assignment_id].entry_scratch_gp_regs();
  }
  return regs;
}

//! \}
//! \endcond

ASMJIT_END_NAMESPACE

#endif // !ASMJIT_NO_COMPILER
#endif // ASMJIT_CORE_RAPASS_P_H_INCLUDED
