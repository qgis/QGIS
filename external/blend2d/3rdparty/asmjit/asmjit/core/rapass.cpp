// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#ifndef ASMJIT_NO_COMPILER

#include <asmjit/core/formatter_p.h>
#include <asmjit/core/ralocal_p.h>
#include <asmjit/core/rapass_p.h>
#include <asmjit/core/type.h>
#include <asmjit/support/arenavector.h>
#include <asmjit/support/support_p.h>

ASMJIT_BEGIN_NAMESPACE

// RABlock - Control Flow
// ======================

Error RABlock::append_successor(RABlock* successor) noexcept {
  RABlock* predecessor = this;

  if (predecessor->has_successor(successor)) {
    return Error::kOk;
  }

  ASMJIT_PROPAGATE(successor->_predecessors.reserve_additional(arena()));
  ASMJIT_PROPAGATE(predecessor->_successors.reserve_additional(arena()));

  predecessor->_successors.append_unchecked(successor);
  successor->_predecessors.append_unchecked(predecessor);

  return Error::kOk;
}

Error RABlock::prepend_successor(RABlock* successor) noexcept {
  RABlock* predecessor = this;

  if (predecessor->has_successor(successor)) {
    return Error::kOk;
  }

  ASMJIT_PROPAGATE(successor->_predecessors.reserve_additional(arena()));
  ASMJIT_PROPAGATE(predecessor->_successors.reserve_additional(arena()));

  predecessor->_successors.prepend_unchecked(successor);
  successor->_predecessors.prepend_unchecked(predecessor);

  return Error::kOk;
}

// BaseRAPass - Construction & Destruction
// =======================================

BaseRAPass::BaseRAPass(BaseCompiler& cc) noexcept : Pass(cc, "RAPass") {}
BaseRAPass::~BaseRAPass() noexcept {}

static void RAPass_reset_virt_reg_data(BaseRAPass* self) noexcept {
  for (RAWorkReg* work_reg : self->_work_regs) {
    VirtReg* virt_reg = work_reg->virt_reg();

    // Update the information regarding the stack of the virtual register.
    if (work_reg->has_stack_slot()) {
      RAStackSlot* slot = work_reg->stack_slot();
      virt_reg->assign_stack_slot(slot->offset());
    }

    // Reset work reg association so it cannot be used by accident (RAWorkReg data will be destroyed).
    virt_reg->_work_reg = nullptr;
  }
}

// BaseRAPass - Run Prepare & Cleanup
// ==================================

#ifndef ASMJIT_NO_LOGGING
static ASMJIT_INLINE void RAPass_prepare_logging(BaseRAPass& pass, Logger* logger) noexcept {
  DiagnosticOptions diag = pass._cb.diagnostic_options();

  pass._logger = logger;

  if (logger) {
    pass._format_options = logger->options();
    pass._diagnostic_options = diag;
  }
  else {
    pass._format_options.reset();
    pass._diagnostic_options = diag & ~(DiagnosticOptions::kRADebugCFG |
                                       DiagnosticOptions::kRADebugUnreachable);
  }
}

static ASMJIT_INLINE void RAPass_cleanup_logging(BaseRAPass& pass) noexcept {
  pass._logger = nullptr;
  pass._format_options.reset();
  pass._diagnostic_options = DiagnosticOptions::kNone;
}
#else
static ASMJIT_INLINE void RAPass_prepare_logging(BaseRAPass&, Logger*) noexcept {}
static ASMJIT_INLINE void RAPass_cleanup_logging(BaseRAPass&) noexcept {}
#endif

static void RAPass_prepare_for_function(BaseRAPass* pass, FuncDetail* func_detail) noexcept {
  pass->_args_assignment.reset(func_detail);
  pass->_stack_allocator.reset(pass->_arena);
}

static void RAPass_cleanup_after_function(BaseRAPass* pass) noexcept {
  pass->_blocks.reset();
  pass->_exits.reset();
  pass->_pov.reset();
  pass->_instruction_count = 0;
  pass->_created_block_count = 0;

  pass->_shared_assignments.reset();
  pass->_last_timestamp = 0;

  pass->_arch_traits = nullptr;
  pass->_phys_reg_index.reset();
  pass->_phys_reg_count.reset();
  pass->_phys_reg_total = 0;
  pass->_scratch_reg_indexes.fill(Reg::kIdBad);

  pass->_available_regs.reset();
  pass->_clobbered_regs.reset();

  pass->_work_regs.reset();
  pass->_work_regs_of_group.for_each([](ArenaVector<RAWorkReg*>& regs) { regs.reset(); });
  pass->_multi_work_reg_count = 0u;
  pass->_total_work_reg_count = 0u;

  pass->_strategy.for_each([](RAStrategy& strategy) { strategy.reset(); });
  pass->_global_live_spans.fill(nullptr);
  pass->_global_live_max_count.reset();
  pass->_temporary_mem.reset();

  pass->_stack_allocator.reset(nullptr);
  pass->_args_assignment.reset(nullptr);
  pass->_num_stack_args_to_stack_slots = 0;
  pass->_max_work_reg_name_size = 0;
}

// BaseRAPass - Run & RunOnFunction
// ================================

Error BaseRAPass::run(Arena& arena, Logger* logger) {
  // Find the first function node by skipping all nodes that are not of `NodeType::kFunc` type.
  // If there is no function in the whole code, we would just return early and not setup anything.
  BaseNode* node = cc().first_node();
  for (;;) {
    if (!node) {
      // The code has no function.
      return Error::kOk;
    }

    if (node->type() == NodeType::kFunc) {
      break;
    }

    node = node->next();
  }

  Error err = Error::kOk;
  FuncNode* func = node->as<FuncNode>();

  RAPass_prepare_logging(*this, logger);
  do {
    // Try to find a second function in the code in order to know whether this function is last. Generally,
    // there are two use-cases we want to optimize for: The first is generating a function at a time and the
    // second is generating multiple functions at a time. In the first case we know we can do a little bit
    // cheaper cleanup at the end as we know we won't be running the register allocator again in this run().
    node = func->end_node();

    FuncNode* next_func = nullptr;
    while (node) {
      if (node->type() == NodeType::kFunc) {
        next_func = node->as<FuncNode>();
        break;
      }
      node = node->next();
    }

    err = run_on_function(arena, func, next_func != nullptr);
    if (err != Error::kOk) {
      break;
    }

    func = next_func;
  } while (func);
  RAPass_cleanup_logging(*this);

  if (ASMJIT_UNLIKELY(err != Error::kOk)) {
    return _cb.report_error(err);
  }

  return err;
}

Error BaseRAPass::run_on_function(Arena& arena, FuncNode* func, [[maybe_unused]] bool last) noexcept {
  // Initialize all core structures to use `arena` and `func`.
  BaseNode* end = func->end_node();

  _arena = &arena;
  _func = func;
  _stop = end->next();
  _injection_start = nullptr;
  _injection_end = end;

  RAPass_prepare_for_function(this, &_func->_func_detail);

  // Initialize architecture-specific members.
  on_init();

  // Perform all allocation steps required.
  Error err = on_perform_all_steps();

  // Must be called regardless of the allocation status.
  on_done();

  // Reset possible connections introduced by the register allocator.
  RAPass_reset_virt_reg_data(this);

  // Reset all core structures and everything that depends on the passed `Arena`.
  RAPass_cleanup_after_function(this);

  _arena = nullptr;
  _func = nullptr;
  _stop = nullptr;
  _injection_start = nullptr;
  _injection_end = nullptr;

  // Reset `Arena` as nothing should persist between `run_on_function()` calls.
  arena.reset();

  // We alter the compiler cursor, because it doesn't make sense to reference it after the compilation - some nodes
  // may disappear and the old cursor could be unreachable, so just set the cursor to the last node for better safety.
  cc().set_cursor(cc().last_node());

  return err;
}

// BaseRAPass - Perform All Steps
// ==============================

Error BaseRAPass::on_perform_all_steps() noexcept {
  ASMJIT_PROPAGATE(build_cfg_nodes());
  ASMJIT_PROPAGATE(build_cfg_views());
  ASMJIT_PROPAGATE(remove_unreachable_code());
  ASMJIT_PROPAGATE(build_cfg_dominators());
  ASMJIT_PROPAGATE(build_reg_ids());
  ASMJIT_PROPAGATE(build_liveness());
  ASMJIT_PROPAGATE(assign_arg_index_to_work_regs());

#ifndef ASMJIT_NO_LOGGING
  if (has_diagnostic_option(DiagnosticOptions::kRAAnnotate)) {
    ASMJIT_PROPAGATE(annotate_code());
  }
#endif

  ASMJIT_PROPAGATE(run_global_allocator());
  ASMJIT_PROPAGATE(run_local_allocator());

  ASMJIT_PROPAGATE(update_stack_frame());
  ASMJIT_PROPAGATE(insert_prolog_epilog());

  ASMJIT_PROPAGATE(rewrite());

  return Error::kOk;
}

// BaseRAPass - Events
// ===================

void BaseRAPass::on_init() noexcept {}
void BaseRAPass::on_done() noexcept {}

// BaseRAPass - CFG - Basic Block Management
// =========================================

RABlock* BaseRAPass::new_block(BaseNode* initial_node) noexcept {
  RABlock* block = arena().new_oneshot<RABlock>(this);

  if (ASMJIT_UNLIKELY(!block)) {
    return nullptr;
  }

  block->set_first(initial_node);
  block->set_last(initial_node);

  // Ignore return values here as we don't care if it was successful or not - this is a pre-allocation only
  // to make the default allocated block close to the block itself. In general, it's very common to have at
  // least 1 predecessor and successor, and in case of branches it's either 2 successors or predecessors in
  // case two basic blocks merge.
  (void)block->_predecessors.reserve_fit(arena(), 2u);
  (void)block->_successors.reserve_fit(arena(), 2u);

  _created_block_count++;
  return block;
}

RABlock* BaseRAPass::new_block_or_existing_at(LabelNode* label_node, BaseNode** stopped_at) noexcept {
  if (label_node->has_pass_data()) {
    return label_node->pass_data<RABlock>();
  }

  FuncNode* func = this->func();
  BaseNode* node = label_node->prev();
  RABlock* block = nullptr;

  // Try to find some label, but terminate the loop on any code. We try hard to coalesce code that contains two
  // consecutive labels or a combination of non-code nodes between 2 or more labels.
  //
  // Possible cases that would share the same basic block:
  //
  //   1. Two or more consecutive labels:
  //     Label1:
  //     Label2:
  //
  //   2. Two or more labels separated by non-code nodes:
  //     Label1:
  //     ; Some comment...
  //     .align 16
  //     Label2:
  size_t num_pending_labels = 0;

  while (node) {
    if (node->type() == NodeType::kLabel) {
      // Function has a different NodeType, just make sure this was not messed up as we must never associate
      // BasicBlock with a `func` itself.
      ASMJIT_ASSERT(node != func);

      block = node->pass_data<RABlock>();
      if (block) {
        // Exit node has always a block associated with it. If we went here it means that `label_node` passed
        // here is after the end of the function and cannot be merged with the function exit block.
        if (node == func->exit_node()) {
          block = nullptr;
        }
        break;
      }

      num_pending_labels++;
    }
    else if (node->type() == NodeType::kAlign) {
      // Align node is fine.
    }
    else {
      break;
    }

    node = node->prev();
  }

  if (stopped_at)
    *stopped_at = node;

  if (!block) {
    block = new_block();
    if (ASMJIT_UNLIKELY(!block)) {
      return nullptr;
    }
  }

  label_node->set_pass_data<RABlock>(block);
  node = label_node;

  while (num_pending_labels) {
    node = node->prev();
    for (;;) {
      if (node->type() == NodeType::kLabel) {
        node->set_pass_data<RABlock>(block);
        num_pending_labels--;
        break;
      }

      node = node->prev();
      ASMJIT_ASSERT(node != nullptr);
    }
  }

  if (!block->first()) {
    block->set_first(node);
    block->set_last(label_node);
  }

  return block;
}

Error BaseRAPass::add_block(RABlock* block) noexcept {
  ASMJIT_PROPAGATE(_blocks.reserve_additional(arena()));

  block->_block_id = RABlockId(block_count());
  _blocks.append_unchecked(block);
  return Error::kOk;
}

// BaseRAPass - CFG - Build
// ========================

// [[pure virtual]]
Error BaseRAPass::build_cfg_nodes() noexcept {
  return make_error(Error::kInvalidState);
}

Error BaseRAPass::init_shared_assignments(Span<uint32_t> shared_assignments_map) noexcept {
  if (shared_assignments_map.is_empty()) {
    return Error::kOk;
  }

  uint32_t count = 0;
  for (RABlock* block : _blocks) {
    if (block->has_shared_assignment_id()) {
      uint32_t shared_assignment_id = shared_assignments_map[block->shared_assignment_id()];
      block->set_shared_assignment_id(shared_assignment_id);
      count = Support::max(count, shared_assignment_id + 1);
    }
  }

  ASMJIT_PROPAGATE(_shared_assignments.resize_fit(arena(), count));

  // Aggregate all entry scratch GP regs from blocks of the same assignment to the assignment itself. It will then be
  // used instead of RABlock's own scratch regs mask, as shared assignments have precedence.
  for (RABlock* block : _blocks) {
    if (block->has_jump_table()) {
      Span<RABlock*> successors = block->successors();
      if (!successors.is_empty()) {
        RABlock* first_successor = successors[0];
        // NOTE: Shared assignments connect all possible successors so we only need the first to propagate exit scratch
        // GP registers.
        if (first_successor->has_shared_assignment_id()) {
          RASharedAssignment& sa = _shared_assignments[first_successor->shared_assignment_id()];
          sa.add_entry_scratch_gp_regs(block->exit_scratch_gp_regs());
        }
        else {
          // This is only allowed if there is a single successor - in that case shared assignment is not necessary.
          ASMJIT_ASSERT(successors.size() == 1u);
        }
      }
    }
    if (block->has_shared_assignment_id()) {
      RASharedAssignment& sa = _shared_assignments[block->shared_assignment_id()];
      sa.add_entry_scratch_gp_regs(block->_entry_scratch_gp_regs);
    }
  }

  return Error::kOk;
}

// BaseRAPass - CFG - Views Order
// ==============================

// Stack specific to building a post-order-view. It reuses the POV vector in a way that stacked items
// are added from the end, which would never collide with items already added to the POV vector as
// they are added from the beginning (and the number of stacked items cannot exceed the number of
// blocks). Additionally, it needs one vector of uint32_t to store the index of successors where it
// ended before it was pushed on the stack.
class RAPOVBuilderStack {
protected:
  RABlock** _block_ptr;
  uint32_t* _index_ptr;
  uint32_t* _index_begin;

public:
  ASMJIT_INLINE RAPOVBuilderStack(RABlock** pov_stack, uint32_t* index_stack, size_t block_count) noexcept
    : _block_ptr(pov_stack + block_count),
      _index_ptr(index_stack),
      _index_begin(index_stack) {}

  ASMJIT_INLINE void push(RABlock* block, uint32_t index) noexcept {
    *--_block_ptr = block;
    *_index_ptr++ = index;
  }

  ASMJIT_INLINE void pop(RABlock*& block, uint32_t& index) noexcept {
    ASMJIT_ASSERT(_index_ptr != _index_begin);

    block = *_block_ptr++;
    index = *--_index_ptr;
  }

  ASMJIT_INLINE_NODEBUG bool is_empty() const noexcept { return _index_ptr == _index_begin; }
};

Error BaseRAPass::build_cfg_views() noexcept {
#ifndef ASMJIT_NO_LOGGING
  Logger* logger = logger_if(DiagnosticOptions::kRADebugCFG);
  ASMJIT_RA_LOG_FORMAT("[build_cfg_views]\n");
#endif // !ASMJIT_NO_LOGGING

  size_t count = block_count();
  if (ASMJIT_UNLIKELY(!count)) {
    return Error::kOk;
  }

  ArenaVector<uint32_t> indexes;

  ASMJIT_PROPAGATE(_pov.reserve_fit(arena(), count));
  ASMJIT_PROPAGATE(indexes.reserve_fit(arena(), count));

  RABlock** pov_data = _pov.data();
  size_t pov_index = 0u;

  RABlock* cur_block = _blocks[0];
  uint32_t cur_index = 0u;

  RAPOVBuilderStack stack(pov_data, indexes.data(), count);

  // This loop uses reachable bit in a RABlock to mark visited blocks.
  cur_block->make_reachable();
  for (;;) {
    while (cur_index < cur_block->successors().size()) {
      RABlock* child = cur_block->successors()[cur_index++];
      if (!child->is_reachable()) {
        // Mark the block as reachable to prevent visiting the same block again.
        child->make_reachable();

        // Add the cur_block block to the stack, we will get back to it later.
        stack.push(cur_block, cur_index);

        // Visit the first successor.
        cur_block = child;
        cur_index = 0u;
      }
    }

    cur_block->_pov_index = uint32_t(pov_index);
    pov_data[pov_index++] = cur_block;

    if (stack.is_empty()) {
      break;
    }

    stack.pop(cur_block, cur_index);
  }

  _pov._set_size(pov_index);
  indexes.release(arena());

  ASMJIT_RA_LOG_COMPLEX({
    StringTmp<1024> sb;
    for (RABlock* block : blocks()) {
      sb.clear();
      if (block->has_successors()) {
        sb.append_format("  #%u -> {", block->block_id());
        dump_block_ids(sb, block->successors());
        sb.append("}\n");
      }
      else {
        sb.append_format("  #%u -> {Exit}\n", block->block_id());
      }
      logger->log(sb);
    }
  });

  return Error::kOk;
}

// BaseRAPass - CFG - Dominators
// =============================

static ASMJIT_INLINE RABlock* intersect_blocks(RABlock* b1, RABlock* b2) noexcept {
  while (b1 != b2) {
    while (b2->pov_index() > b1->pov_index()) b1 = b1->idom();
    while (b1->pov_index() > b2->pov_index()) b2 = b2->idom();
  }
  return b1;
}

// Based on "A Simple, Fast Dominance Algorithm".
Error BaseRAPass::build_cfg_dominators() noexcept {
#ifndef ASMJIT_NO_LOGGING
  Logger* logger = logger_if(DiagnosticOptions::kRADebugCFG);
  ASMJIT_RA_LOG_FORMAT("[build_cfg_dominators]\n");
#endif // !ASMJIT_NO_LOGGING

  if (_blocks.is_empty()) {
    return Error::kOk;
  }

  RABlock* entry_block = this->entry_block();
  entry_block->_idom = entry_block;

  bool changed = true;

#ifndef ASMJIT_NO_LOGGING
  uint32_t iter_count = 0;
#endif // !ASMJIT_NO_LOGGING

  while (changed) {
    changed = false;

#ifndef ASMJIT_NO_LOGGING
    iter_count++;
#endif // !ASMJIT_NO_LOGGING

    for (RABlock* block : _pov.iterate_reverse()) {
      if (block == entry_block) {
        continue;
      }

      RABlock* idom = nullptr;
      Span<RABlock*> predecessors = block->predecessors();

      for (RABlock* p : predecessors.iterate_reverse()) {
        if (!p->idom()) {
          continue;
        }
        idom = !idom ? p : intersect_blocks(idom, p);
      }

      if (block->idom() != idom) {
        ASMJIT_ASSUME(idom != nullptr);
        ASMJIT_RA_LOG_FORMAT("  idom of #%u -> #%u\n", block->block_id(), idom->block_id());
        block->_idom = idom;
        changed = true;
      }
    }
  }

  ASMJIT_RA_LOG_FORMAT("  done (%u iterations)\n", iter_count);
  return Error::kOk;
}

bool BaseRAPass::_strictly_dominates(const RABlock* a, const RABlock* b) const noexcept {
  ASMJIT_ASSERT(a != nullptr); // There must be at least one block if this function is
  ASMJIT_ASSERT(b != nullptr); // called, as both `a` and `b` must be valid blocks.
  ASMJIT_ASSERT(a != b);       // Checked by `dominates()` and `strictly_dominates()`.

  // Nothing strictly dominates the entry block.
  const RABlock* entry_block = this->entry_block();
  if (a == entry_block) {
    return false;
  }

  const RABlock* idom = b->idom();
  while (idom != a && idom != entry_block) {
    idom = idom->idom();
  }

  return idom != entry_block;
}

const RABlock* BaseRAPass::_nearest_common_dominator(const RABlock* a, const RABlock* b) const noexcept {
  ASMJIT_ASSERT(a != nullptr); // There must be at least one block if this function is
  ASMJIT_ASSERT(b != nullptr); // called, as both `a` and `b` must be valid blocks.
  ASMJIT_ASSERT(a != b);       // Checked by `dominates()` and `_strictly_dominates()`.

  if (a == b) {
    return a;
  }

  // If `a` strictly dominates `b` then `a` is the nearest common dominator.
  if (_strictly_dominates(a, b)) {
    return a;
  }

  // If `b` strictly dominates `a` then `b` is the nearest common dominator.
  if (_strictly_dominates(b, a)) {
    return b;
  }

  const RABlock* entry_block = this->entry_block();
  RABlockTimestamp timestamp = next_timestamp();

  // Mark all A's dominators.
  const RABlock* block = a->idom();
  while (block != entry_block) {
    block->set_timestamp(timestamp);
    block = block->idom();
  }

  // Check all B's dominators against marked dominators of A.
  block = b->idom();
  while (block != entry_block) {
    if (block->has_timestamp(timestamp)) {
      return block;
    }
    block = block->idom();
  }

  return entry_block;
}

// BaseRAPass - CFG - Utilities
// ============================

Error BaseRAPass::remove_unreachable_code() noexcept {
  size_t num_all_blocks = block_count();
  size_t num_reachable_blocks = reachable_block_count();

  // All reachable -> nothing to do.
  if (num_all_blocks == num_reachable_blocks) {
    return Error::kOk;
  }

#ifndef ASMJIT_NO_LOGGING
  Logger* logger = logger_if(DiagnosticOptions::kRADebugUnreachable);
  String& sb = _tmp_string;
  ASMJIT_RA_LOG_FORMAT("[remove_unreachable_code - detected %zu of %zu unreachable blocks]\n", num_all_blocks - num_reachable_blocks, num_all_blocks);
#endif

  for (RABlock* block : _blocks.iterate()) {
    if (block->is_reachable()) {
      continue;
    }

    ASMJIT_RA_LOG_FORMAT("  removing code from unreachable block {%u}\n", uint32_t(block->block_id()));
    BaseNode* first = block->first();
    BaseNode* last = block->last();

    BaseNode* before_first = first->prev();
    BaseNode* after_last = last->next();

    BaseNode* node = first;
    while (node != after_last) {
      BaseNode* next = node->next();

      if (node->is_code() || node->is_removable()) {
#ifndef ASMJIT_NO_LOGGING
        if (logger) {
          sb.clear();
          Formatter::format_node(sb, _format_options, &_cb, node);
          logger->logf("    %s\n", sb.data());
        }
#endif
        cc().remove_node(node);
      }
      node = next;
    }

    if (before_first->next() == after_last) {
      block->set_first(nullptr);
      block->set_last(nullptr);
    }
    else {
      block->set_first(before_first->next());
      block->set_last(after_last->prev());
    }
  }

  return Error::kOk;
}

BaseNode* BaseRAPass::find_successor_starting_at(BaseNode* node) noexcept {
  while (node && (node->is_informative() || node->has_no_effect())) {
    node = node->next();
  }
  return node;
}

bool BaseRAPass::is_next_to(BaseNode* node, BaseNode* target) noexcept {
  for (;;) {
    node = node->next();
    if (node == target) {
      return true;
    }

    if (!node) {
      return false;
    }

    if (node->is_code() || node->is_data()) {
      return false;
    }
  }
}

// BaseRAPass - Registers - VirtReg / WorkReg Mapping
// ==================================================

Error BaseRAPass::_as_work_reg(RAWorkReg** out, VirtReg* virt_reg) noexcept {
  // Checked by `as_work_reg()` - must be true.
  ASMJIT_ASSERT(virt_reg->_work_reg == nullptr);

  OperandSignature signature = RegUtils::signature_of(virt_reg->reg_type());
  RegGroup group = signature.reg_group();
  ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);

  ArenaVector<RAWorkReg*>& work_regs_by_group = work_regs(group);
  ASMJIT_PROPAGATE(work_regs_by_group.reserve_additional(arena()));

  RAWorkReg* work_reg = arena().new_oneshot<RAWorkReg>(virt_reg, signature, kBadWorkId);
  if (ASMJIT_UNLIKELY(!work_reg)) {
    return make_error(Error::kOutOfMemory);
  }

  virt_reg->set_work_reg(work_reg);
  if (!virt_reg->is_stack_area()) {
    work_reg->set_reg_byte_mask(Support::lsb_mask<uint64_t>(virt_reg->virt_size()));
  }

  work_regs_by_group.append_unchecked(work_reg);
  _total_work_reg_count++;

  // Only used by RA logging.
  _max_work_reg_name_size = Support::max(_max_work_reg_name_size, virt_reg->name_size());

  *out = work_reg;
  return Error::kOk;
}

RAStackSlot* BaseRAPass::_create_stack_slot(RAWorkReg* work_reg) noexcept {
  VirtReg* virt_reg = work_reg->virt_reg();
  RAStackSlot* slot = _stack_allocator.new_slot(_sp.id(), virt_reg->virt_size(), virt_reg->alignment(), RAStackSlot::kFlagRegHome);

  work_reg->_stack_slot = slot;
  work_reg->mark_stack_used();

  return slot;
}

RAAssignment::WorkToPhysMap* BaseRAPass::new_work_to_phys_map() noexcept {
  size_t count = work_reg_count();
  size_t size = WorkToPhysMap::size_of(count);

  // If no registers are used it could be zero, in that case return a dummy map instead of NULL.
  if (ASMJIT_UNLIKELY(!size)) {
    static const RAAssignment::WorkToPhysMap null_map = {{ 0 }};
    return const_cast<RAAssignment::WorkToPhysMap*>(&null_map);
  }

  WorkToPhysMap* map = arena().alloc_oneshot<WorkToPhysMap>(size);
  if (ASMJIT_UNLIKELY(!map)) {
    return nullptr;
  }

  map->reset(count);
  return map;
}

RAAssignment::PhysToWorkMap* BaseRAPass::new_phys_to_work_map() noexcept {
  uint32_t count = phys_reg_total();
  size_t size = PhysToWorkMap::size_of(count);

  PhysToWorkMap* map = arena().alloc_oneshot<PhysToWorkMap>(size);
  if (ASMJIT_UNLIKELY(!map)) {
    return nullptr;
  }

  map->reset(count);
  return map;
}

// BaseRAPass - Registers - Liveness Analysis and Statistics
// =========================================================

ASMJIT_FAVOR_SPEED Error BaseRAPass::build_reg_ids() noexcept {
  uint32_t count = _total_work_reg_count;
  ASMJIT_PROPAGATE(_work_regs.reserve_fit(arena(), count));

  RAWorkReg** work_regs = _work_regs.data();
  _work_regs._set_size(count);

  uint32_t multi_id = 0u;
  uint32_t single_id = count;

  for (uint32_t rg = 0; rg < Globals::kNumVirtGroups; rg++) {
    for (RAWorkReg* work_reg : _work_regs_of_group[rg]) {
      if (work_reg->is_within_single_basic_block()) {
        work_reg->_work_id = RAWorkId(--single_id);
        work_regs[single_id] = work_reg;
      }
      else {
        work_reg->_work_id = RAWorkId(multi_id);
        work_reg->_single_basic_block_id = kBadBlockId;
        work_regs[multi_id++] = work_reg;
      }
    }
  }

  ASMJIT_ASSERT(single_id == multi_id);
  _multi_work_reg_count = multi_id;

  return Error::kOk;
}

// BaseRAPass - Registers - Liveness Analysis and Statistics
// =========================================================

template<typename BitMutator>
static ASMJIT_INLINE void BaseRAPass_calculate_initial_in_out(RABlock* block, size_t live_word_count, Support::FixedStack<RABlock*>& queue, uint32_t pov_index) noexcept {
  using BitWord = Support::BitWord;

  // Calculate LIVE-OUT based on LIVE-IN of all successors and then recalculate LIVE-IN based on LIVE-OUT and KILL bits.
  Span<RABlock*> successors = block->successors();
  if (!successors.is_empty()) {
    BitMutator bm_live_in(block->live_in());
    BitMutator bm_live_out(block->live_out());
    BitMutator bm_kill(block->kill());

    for (RABlock* successor : successors.iterate_reverse()) {
      if (Support::bool_and(successor->pov_index() > pov_index, !successor->is_enqueued())) {
        successor->add_flags(RABlockFlags::kIsEnqueued);
        queue.push(successor);
        continue;
      }

      BitMutator bm_successor_live_in(successor->live_in());
      for (uint32_t bw = 0; bw < live_word_count; bw++) {
        BitWord sc = bm_successor_live_in.bit_word(bw);

        BitWord in = bm_live_in.bit_word(bw) | sc;
        BitWord out = bm_live_out.bit_word(bw) | sc;

        bm_live_in.set_bit_word(bw, in & ~bm_kill.bit_word(bw));
        bm_live_out.set_bit_word(bw, out);
      }

      bm_live_in.commit(block->live_in());
      bm_live_out.commit(block->live_out());
    }
  }
}

// Calculate LIVE-IN and LIVE-OUT of the given `block` and a single `successor` that has changed its LIVE-IN bits.
template<typename BitMutator>
static ASMJIT_INLINE Support::BitWord BaseRAPass_recalculateInOut(RABlock* block, size_t live_word_count, RABlock* successor) noexcept {
  using BitWord = Support::BitWord;

  BitMutator bm_live_in(block->live_in());
  BitMutator bm_live_out(block->live_out());
  BitMutator bm_kill(block->kill());

  BitMutator bm_successor_live_in(successor->live_in());
  BitWord changed = 0u;

  for (size_t i = 0; i < live_word_count; i++) {
    BitWord succ_in = bm_successor_live_in.bit_word(i);

    BitWord in = bm_live_in.bit_word(i);
    BitWord out = bm_live_out.bit_word(i);
    BitWord kill = bm_kill.bit_word(i);

    BitWord new_in = (in | succ_in) & ~kill;
    BitWord new_out = (out | succ_in);

    bm_live_in.set_bit_word(i, new_in);
    bm_live_out.set_bit_word(i, new_out);

    changed |= in ^ new_in;
  }

  bm_live_in.commit(block->live_in());
  bm_live_out.commit(block->live_out());

  return changed;
}

template<typename BitMutator>
static ASMJIT_INLINE Error BaseRAPass_calculateInOutKill(
  BaseRAPass* pass,
  uint32_t* n_uses_per_work_reg,
  uint32_t* n_outs_per_work_reg,
  uint32_t* n_insts_per_block,
  Out<uint32_t> num_visits_out
) noexcept {
  Span<RABlock*> pov = pass->_pov.as_span();

  size_t multi_work_reg_count = pass->multi_work_reg_count();
  size_t multi_work_reg_count_as_bit_words = BitOps::size_in_words<BitWord>(multi_work_reg_count);

  constexpr RAWorkRegFlags kLiveFlag = RAWorkRegFlags::kSingleBlockLiveFlag;
  constexpr RAWorkRegFlags kVisitedFlag = RAWorkRegFlags::kSingleBlockVisitedFlag;

  // Calculate GEN and KILL and then initial LIVE-IN and LIVE-OUT bits.
  //
  // GEN is mapped to LIVE-IN, because it's not needed after LIVE-IN is calculated,
  // which is essentially `LIVE-IN = GEN & ~KILL` - so once we know GEN and KILL for
  // each block, calculating LIVE-IN is trivial.
  for (RABlock* block : pov.iterate()) {
    ASMJIT_PROPAGATE(block->alloc_live_bits(multi_work_reg_count));

    BaseNode* node = block->last();
    BaseNode* stop = block->first();

    BitMutator live_in(block->live_in()); // LIVE-IN which maps to GEN as well.
    BitMutator kill(block->kill());     // KILL only.

    RABlockId block_id = block->block_id();
    uint32_t inst_count = 0;

    for (;;) {
      if (node->is_inst()) {
        InstNode* inst = node->as<InstNode>();
        RAInst* ra_inst = inst->pass_data<RAInst>();
        ASMJIT_ASSERT(ra_inst != nullptr);

        RATiedReg* tied_regs = ra_inst->tied_regs();
        uint32_t count = ra_inst->tied_count();

        for (uint32_t j = 0; j < count; j++) {
          RATiedReg* tied_reg = &tied_regs[j];
          RAWorkReg* work_reg = tied_reg->work_reg();

          RAWorkId work_id = work_reg->work_id();

          // Update `n_uses` and `n_outs`.
          n_uses_per_work_reg[uint32_t(work_id)] += 1u;
          n_outs_per_work_reg[uint32_t(work_id)] += uint32_t(tied_reg->is_write());

          bool is_kill = tied_reg->is_write_only();
          RATiedFlags tied_flags = tied_reg->flags();

          // Mark as:
          //   KILL - if this VirtReg is killed afterwards.
          //   LAST - if this VirtReg is last in this basic block.
          if (work_reg->is_within_single_basic_block()) {
            bool was_kill = !Support::test(work_reg->flags(), kLiveFlag);
            bool was_last = !Support::test(work_reg->flags(), kVisitedFlag);

            tied_flags |= Support::bool_as_flag<RATiedFlags::kKill>(was_kill);
            tied_flags |= Support::bool_as_flag<RATiedFlags::kLast>(was_last);

            work_reg->add_flags(kVisitedFlag);
            work_reg->xor_flags(Support::bool_as_flag<kLiveFlag>(uint32_t(is_kill ^ was_kill)));
          }
          else {
            bool was_kill = kill.bit_at(work_id);
            bool was_last = !live_in.bit_at(work_id);

            tied_flags |= Support::bool_as_flag<RATiedFlags::kKill>(was_kill);
            tied_flags |= Support::bool_as_flag<RATiedFlags::kLast>(was_last);

            // KILL if the register is write only, otherwise GEN.
            live_in.add_bit(work_id, !is_kill);
            kill.xor_bit(work_id, bool(is_kill ^ was_kill));
          }

          tied_reg->_flags = tied_flags;

          if (tied_reg->is_lead_consecutive()) {
            work_reg->mark_lead_consecutive();
          }

          if (tied_reg->has_consecutive_parent()) {
            RAWorkReg* consecutive_parent_reg = tied_reg->consecutive_parent();
            ASMJIT_PROPAGATE(consecutive_parent_reg->add_immediate_consecutive(pass->arena(), work_id));
          }
        }

        inst_count++;
      }

      if (node == stop) {
        break;
      }

      node = node->prev();
      ASMJIT_ASSERT(node != nullptr);
    }

    n_insts_per_block[uint32_t(block_id)] = inst_count;

    // Calculate initial LIVE-IN from GEN - LIVE-IN = GEN & ~KILL.
    live_in.clear_bits(kill);

    live_in.commit(block->live_in());
    kill.commit(block->kill());
  }

  // Calculate initial values of LIVE-OUT and update LIVE-IN accordingly to LIVE-OUT.
  //
  // This step requires a queue, however, we only add a node's successors to the queue, which post-order-index
  // is greater than the post-order-index of the block being processed. This makes the next pass much faster to
  // converge.
  uint32_t num_visits = 0u;
  if (multi_work_reg_count_as_bit_words > 0u) {
    ArenaVector<RABlock*> queue_storage;
    ASMJIT_PROPAGATE(queue_storage.reserve_fit(pass->arena(), pov.size()));
    Support::FixedStack<RABlock*> queue(queue_storage.data(), pov.size());

    for (size_t pov_index = 0u; pov_index < pov.size(); pov_index++) {
      RABlock* block = pov[pov_index];
      BaseRAPass_calculate_initial_in_out<BitMutator>(block, multi_work_reg_count_as_bit_words, queue, uint32_t(pov_index));
    }

    // Iteratively keep recalculating LIVE-IN and LIVE-OUT once there are no more changes to the bits. This is
    // needed as there may be cycles in the CFG, which have to be propagated. This algorithm essentially uses a
    // work queue where nodes that change are pushed to propagate the changes to all predecessor nodes.
    while (!queue.is_empty()) {
      num_visits++;

      RABlock* block = queue.pop();
      block->clear_flags(RABlockFlags::kIsEnqueued);

      for (RABlock* predecessor : block->predecessors()) {
        Support::BitWord changed = BaseRAPass_recalculateInOut<BitMutator>(predecessor, multi_work_reg_count_as_bit_words, block);
        if (Support::bool_and(changed, !predecessor->is_enqueued())) {
          predecessor->add_flags(RABlockFlags::kIsEnqueued);
          queue.push(predecessor);
        }
      }
    }

    queue_storage.release(pass->arena());
  }

  num_visits_out = num_visits;
  return Error::kOk;
}

ASMJIT_FAVOR_SPEED Error BaseRAPass::build_liveness() noexcept {
#ifndef ASMJIT_NO_LOGGING
  Logger* logger = logger_if(DiagnosticOptions::kRADebugLiveness);
#endif

  ASMJIT_RA_LOG_FORMAT("[build_liveness]\n");

  size_t num_all_blocks = block_count();
  size_t num_work_regs = work_reg_count();
  size_t multi_work_reg_count = _multi_work_reg_count;

  if (!num_work_regs) {
    ASMJIT_RA_LOG_FORMAT("  done (no virtual registers)\n");
    return Error::kOk;
  }

  ArenaVector<uint32_t> n_uses_per_work_reg; // Number of USEs of each RAWorkReg.
  ArenaVector<uint32_t> n_outs_per_work_reg; // Number of OUTs of each RAWorkReg.
  ArenaVector<uint32_t> n_insts_per_block;  // Number of instructions of each RABlock.

  ASMJIT_PROPAGATE(n_uses_per_work_reg.resize_fit(arena(), num_work_regs));
  ASMJIT_PROPAGATE(n_outs_per_work_reg.resize_fit(arena(), num_work_regs));
  ASMJIT_PROPAGATE(n_insts_per_block.resize_fit(arena(), num_all_blocks));

  // Calculate GEN/KILL and then IN/OUT of Each Block
  // ------------------------------------------------

  {
    uint32_t num_visits = 0;

    if (multi_work_reg_count > 0u && multi_work_reg_count <= Support::bit_size_of<BitWord>) {
      // If the number of work registers as a mask fits into a single BitWord use a separate code-path that optimizes
      // for such case. This makes faster generating smaller code that doesn't have many virtual registers in use.
      ASMJIT_PROPAGATE(
        BaseRAPass_calculateInOutKill<Support::BitWordMutator>(
          this, n_uses_per_work_reg.data(), n_outs_per_work_reg.data(), n_insts_per_block.data(), Out(num_visits)
        )
      );
    }
    else {
      ASMJIT_PROPAGATE(
        BaseRAPass_calculateInOutKill<Support::BitVectorMutator>(
          this, n_uses_per_work_reg.data(), n_outs_per_work_reg.data(), n_insts_per_block.data(), Out(num_visits)
        )
      );
    }

    ASMJIT_RA_LOG_COMPLEX({
      String& sb = _tmp_string;
      logger->logf("  LiveIn/Out Done (%u visits)\n", num_visits);

      for (uint32_t i = 0; i < num_all_blocks; i++) {
        RABlock* block = _blocks[i];

        ASMJIT_PROPAGATE(sb.assign_format("  {#%u}\n", block->block_id()));
        ASMJIT_PROPAGATE(dump_block_liveness(sb, block));

        logger->log(sb);
      }
    });
  }

  // Reserve the space in each `RAWorkReg` for references
  // ----------------------------------------------------

  for (uint32_t i = 0; i < num_work_regs; i++) {
    RAWorkReg* work_reg = work_reg_by_id(RAWorkId(i));
    ASMJIT_PROPAGATE(work_reg->_refs.reserve_fit(arena(), n_uses_per_work_reg[i]));
    ASMJIT_PROPAGATE(work_reg->_writes.reserve_fit(arena(), n_outs_per_work_reg[i]));
  }

  // These are not needed anymore, so release the memory now so other allocations can reuse it.
  n_uses_per_work_reg.release(arena());
  n_outs_per_work_reg.release(arena());

  // Assign block and instruction positions, build LiveCount and LiveSpans
  // ---------------------------------------------------------------------

  // This is a starting position, reserving [0, 1] for function arguments.
  uint32_t position = 2;

  for (uint32_t i = 0; i < num_all_blocks; i++) {
    RABlock* block = _blocks[i];
    if (!block->is_reachable()) {
      continue;
    }

    BaseNode* node = block->first();
    BaseNode* stop = block->last();

    Span<const BitWord> live_out = block->live_out();

    uint32_t end_position = position + n_insts_per_block[i] * 2u;
    block->set_first_position(NodePosition(position));
    block->set_end_position(NodePosition(end_position));

    RALiveCount cur_live_count;
    RALiveCount max_live_count;

    // Process LIVE-IN.
    Support::BitVectorIterator<BitWord> it(block->live_in());
    while (it.has_next()) {
      RAWorkReg* work_reg = _work_regs[uint32_t(it.next())];
      cur_live_count[work_reg->group()]++;
      ASMJIT_PROPAGATE(work_reg->live_spans().open_at(arena(), NodePosition(position), NodePosition(end_position)));
    }

    for (;;) {
      if (node->is_inst()) {
        InstNode* inst = node->as<InstNode>();
        RAInst* ra_inst = inst->pass_data<RAInst>();

        // Impossible - each processed instruction node must have an associated RAInst.
        ASMJIT_ASSERT(ra_inst != nullptr);

        RATiedReg* tied_regs = ra_inst->tied_regs();
        uint32_t count = ra_inst->tied_count();

        inst->set_position(NodePosition(position));
        ra_inst->_live_count = cur_live_count;

        for (uint32_t j = 0; j < count; j++) {
          RATiedReg* tied_reg = &tied_regs[j];
          RAWorkReg* work_reg = tied_reg->work_reg();

          RAWorkId work_id = work_reg->work_id();

          // Create refs and writes.
          work_reg->_refs.append_unchecked(node);
          if (tied_reg->is_write()) {
            work_reg->_writes.append_unchecked(node);
          }

          // We couldn't calculate this in previous steps, but since we know all LIVE-OUT at this point it becomes
          // trivial. If this is the last instruction that uses this `work_reg` and it's not LIVE-OUT then it is a
          // KILL here.
          if (tied_reg->is_last() && (size_t(work_id) >= multi_work_reg_count || !BitOps::bit_at(live_out, work_id))) {
            tied_reg->add_flags(RATiedFlags::kKill);
          }

          RALiveSpans& live_spans = work_reg->live_spans();
          bool was_open;

          ASMJIT_PROPAGATE(live_spans.open_at(
            arena(), NodePosition(position + !tied_reg->is_read()), NodePosition(end_position), was_open));

          RegGroup group = work_reg->group();
          if (!was_open) {
            cur_live_count[group]++;
            ra_inst->_live_count[group]++;
          }

          if (tied_reg->is_kill()) {
            live_spans.close_at(NodePosition(position + !tied_reg->is_read() + 1u));
            cur_live_count[group]--;
          }

          // Update `RAWorkReg::use_id_mask` and `RAWorkReg::hint_reg_id`.
          if (tied_reg->has_use_id()) {
            uint32_t use_id = tied_reg->use_id();
            work_reg->add_use_id_mask(Support::bit_mask<RegMask>(use_id));
            if (!work_reg->has_hint_reg_id() && !Support::bit_test(ra_inst->_clobbered_regs[group], use_id)) {
              work_reg->set_hint_reg_id(use_id);
            }
          }

          if (tied_reg->use_reg_mask()) {
            work_reg->restrict_preferred_mask(tied_reg->use_reg_mask());
            if (work_reg->is_lead_consecutive()) {
              work_reg->restrict_consecutive_mask(tied_reg->use_reg_mask());
            }
          }

          if (tied_reg->out_reg_mask()) {
            work_reg->restrict_preferred_mask(tied_reg->out_reg_mask());
            if (work_reg->is_lead_consecutive()) {
              work_reg->restrict_consecutive_mask(tied_reg->out_reg_mask());
            }
          }

          // Update `RAWorkReg::clobber_survival_mask`.
          if (ra_inst->_clobbered_regs[group] && !tied_reg->is_out_or_kill()) {
            work_reg->add_clobber_survival_mask(ra_inst->_clobbered_regs[group]);
          }
        }

        if (node->is_invoke()) {
          func()->frame().update_call_stack_alignment(node->as<InvokeNode>()->detail().natural_stack_alignment());
        }

        position += 2;
        max_live_count.op<Support::Max>(ra_inst->_live_count);
      }

      if (node == stop) {
        break;
      }

      node = node->next();
      ASMJIT_ASSERT(node != nullptr);
    }

    block->_max_live_count = max_live_count;
    _global_live_max_count.op<Support::Max>(max_live_count);
    ASMJIT_ASSERT(NodePosition(position) == block->end_position());
  }

  // Calculate WorkReg statistics
  // ----------------------------

  for (uint32_t i = 0; i < num_work_regs; i++) {
    RAWorkReg* work_reg = _work_regs[i];

    RALiveSpans& spans = work_reg->live_spans();
    uint32_t width = spans.width();
    float freq = width ? float(double(work_reg->_refs.size()) / double(width)) : float(0);

    RALiveStats& stats = work_reg->live_stats();
    stats._width = width;
    stats._freq = freq;
    stats._priority = freq + float(int(work_reg->virt_reg()->weight())) * 0.01f;
  }

  ASMJIT_RA_LOG_COMPLEX({
    String& sb = _tmp_string;
    sb.clear();
    dump_live_spans(sb);
    logger->log(sb);
  });

  n_insts_per_block.release(arena());
  return Error::kOk;
}

Error BaseRAPass::assign_arg_index_to_work_regs() noexcept {
  Span<const BitWord> live_in = entry_block()->live_in();

  uint32_t arg_count = func()->arg_count();
  uint32_t multi_work_reg_count = _multi_work_reg_count;

  for (uint32_t arg_index = 0; arg_index < arg_count; arg_index++) {
    for (uint32_t value_index = 0; value_index < Globals::kMaxValuePack; value_index++) {
      // Unassigned argument.
      const RegOnly& reg_arg = func()->arg_pack(arg_index)[value_index];
      if (!reg_arg.is_reg() || !cc().is_virt_id_valid(reg_arg.id())) {
        continue;
      }

      VirtReg* virt_reg = cc().virt_reg_by_id(reg_arg.id());
      if (!virt_reg) {
        continue;
      }

      // Unreferenced argument.
      RAWorkReg* work_reg = virt_reg->work_reg();
      if (!work_reg) {
        continue;
      }

      // Overwritten argument.
      RAWorkId work_id = work_reg->work_id();
      if (uint32_t(work_id) >= multi_work_reg_count || !BitOps::bit_at(live_in, work_id)) {
        continue;
      }

      work_reg->set_arg_index(arg_index, value_index);
      const FuncValue& arg = func()->detail().arg(arg_index, value_index);

      if (arg.is_reg() && RegUtils::group_of(arg.reg_type()) == work_reg->group()) {
        work_reg->set_hint_reg_id(arg.reg_id());
      }
    }
  }

  return Error::kOk;
}

// BaseRAPass - Allocation - Global
// ================================

#ifndef ASMJIT_NO_LOGGING
static void RAPass_dump_spans(String& sb, uint32_t index, const RALiveSpans& live_spans) noexcept {
  sb.append_format("  %02u: ", index);

  for (uint32_t i = 0; i < live_spans.size(); i++) {
    const RALiveSpan& live_span = live_spans[i];
    if (i) {
      sb.append(", ");
    }
    sb.append_format("[%u:%u]", live_span.a, live_span.b);
  }

  sb.append('\n');
}
#endif

Error BaseRAPass::run_global_allocator() noexcept {
  ASMJIT_PROPAGATE(init_global_live_spans());

  for (RegGroup group : Support::enumerate(RegGroup::kMaxVirt)) {
    ASMJIT_PROPAGATE(bin_pack(group));
  }

  return Error::kOk;
}

ASMJIT_FAVOR_SPEED Error BaseRAPass::init_global_live_spans() noexcept {
  for (RegGroup group : Support::enumerate(RegGroup::kMaxVirt)) {
    size_t phys_count = _phys_reg_count.get(group);
    RALiveSpans* live_spans = nullptr;

    if (phys_count) {
      live_spans = arena().alloc_oneshot<RALiveSpans>(phys_count * sizeof(RALiveSpans));
      if (ASMJIT_UNLIKELY(!live_spans)) {
        return make_error(Error::kOutOfMemory);
      }

      for (size_t phys_id = 0; phys_id < phys_count; phys_id++) {
        new(Support::PlacementNew{&live_spans[phys_id]}) RALiveSpans();
      }
    }

    _global_live_spans[group] = live_spans;
  }

  return Error::kOk;
}

struct RAConsecutiveReg {
  RAWorkReg* work_reg;
  RAWorkReg* parent_reg;
};

ASMJIT_FAVOR_SPEED Error BaseRAPass::bin_pack(RegGroup group) noexcept {
  if (work_reg_count(group) == 0)
    return Error::kOk;

#ifndef ASMJIT_NO_LOGGING
  Logger* logger = logger_if(DiagnosticOptions::kRADebugAssignment);
  String& sb = _tmp_string;

  ASMJIT_RA_LOG_FORMAT("[bin_pack] Available=%u (0x%08X) Count=%u RegGroup=%u\n",
    Support::popcnt(_available_regs[group]),
    _available_regs[group],
    work_reg_count(group),
    uint32_t(group));
#endif

  uint32_t phys_count = _phys_reg_count.get(group);

  ArenaVector<RAWorkReg*> work_regs;
  ArenaVector<RAConsecutiveReg> consecutive_regs;
  RALiveSpans tmp_spans;

  ArenaVector<RAWorkReg*>& group_regs = this->work_regs(group);
  ASMJIT_PROPAGATE(work_regs.reserve_fit(arena(), group_regs.size()));

  work_regs.assign_unchecked(group_regs);
  work_regs.sort([](const RAWorkReg* a, const RAWorkReg* b) noexcept {
    return b->live_stats().priority() - a->live_stats().priority();
  });

  size_t num_work_regs = work_regs.size();
  RegMask available_regs = _available_regs[group];
  RegMask preserved_regs = func()->frame().preserved_regs(group);

  // First try to pack everything that provides register-id hint as these are most likely function arguments and fixed
  // (pre-colored) virtual registers.
  if (!work_regs.is_empty()) {
    uint32_t dst_index = 0;

    for (uint32_t index = 0; index < num_work_regs; index++) {
      RAWorkReg* work_reg = work_regs[index];

      if (work_reg->is_lead_consecutive()) {
        ASMJIT_PROPAGATE(consecutive_regs.append(arena(), RAConsecutiveReg{work_reg, nullptr}));
        work_reg->mark_processed_consecutive();
      }

      if (work_reg->has_hint_reg_id()) {
        uint32_t phys_id = work_reg->hint_reg_id();
        if (Support::bit_test(available_regs, phys_id)) {
          RALiveSpans& live = _global_live_spans[group][phys_id];
          Error err = tmp_spans.non_overlapping_union_of(arena(), live, work_reg->live_spans());

          if (err == Error::kOk) {
            live.swap(tmp_spans);
            work_reg->set_home_reg_id(phys_id);
            work_reg->mark_allocated();
            continue;
          }

          if (err != Error::kByPass) {
            return err;
          }
        }
      }

      work_regs[dst_index++] = work_reg;
    }

    work_regs._set_size(dst_index);
    num_work_regs = dst_index;
  }

  // Allocate consecutive registers - both leads and all consecutives. This is important and prioritized over the rest,
  // because once a lead is allocated we really need to allocate its consecutives, otherwise we may bin pack other
  // registers into their places, which would result in wrong hints to the local allocator, and then into many moves
  // or spills.
  if (!consecutive_regs.is_empty()) {
    // This loop appends all other consecutive registers into `consecutive_regs` array. Leads are at the beginning,
    // non-leads follow.
    for (size_t i = 0;;) {
      size_t stop = consecutive_regs.size();
      if (i == stop) {
        break;
      }

      while (i < stop) {
        RAWorkReg* work_reg = consecutive_regs[i].work_reg;
        if (work_reg->has_immediate_consecutives()) {
          ArenaBitSet::ForEachBitSet it(work_reg->immediate_consecutives());
          while (it.has_next()) {
            RAWorkId consecutive_work_id = RAWorkId(it.next());
            RAWorkReg* consecutive_reg = work_reg_by_id(consecutive_work_id);
            if (!consecutive_reg->is_processed_consecutive()) {
              ASMJIT_PROPAGATE(consecutive_regs.append(arena(), RAConsecutiveReg{consecutive_reg, work_reg}));
              consecutive_reg->mark_processed_consecutive();
            }
          }
        }
        i++;
      }
    }

    for (RAConsecutiveReg& consecutive_reg : consecutive_regs) {
      RAWorkReg* work_reg = consecutive_reg.work_reg;
      if (work_reg->is_allocated()) {
        continue;
      }

      RAWorkReg* parent_reg = consecutive_reg.parent_reg;
      RegMask phys_regs = 0;

      if (!parent_reg) {
        phys_regs = available_regs & work_reg->preferred_mask();
        if (!phys_regs) {
          phys_regs = available_regs & work_reg->consecutive_mask();

          // NOTE: This should never be true as it would mean we would never allocate this virtual register
          // (not here, and not later when local register allocator processes RATiedReg sets).
          if (ASMJIT_UNLIKELY(!phys_regs)) {
            return make_error(Error::kConsecutiveRegsAllocation);
          }
        }
      }
      else if (parent_reg->has_home_reg_id()) {
        uint32_t consecutive_id = parent_reg->home_reg_id() + 1;

        // NOTE: We don't support wrapping. If this goes beyond all allocable registers there is something wrong.
        if (consecutive_id > 31 || !Support::bit_test(available_regs, consecutive_id)) {
          return make_error(Error::kConsecutiveRegsAllocation);
        }

        work_reg->set_hint_reg_id(consecutive_id);
        phys_regs = Support::bit_mask<uint32_t>(consecutive_id);
      }

      while (phys_regs) {
        uint32_t phys_id = Support::bit_size_of<RegMask> - 1 - Support::clz(phys_regs);

        RALiveSpans& live = _global_live_spans[group][phys_id];
        Error err = tmp_spans.non_overlapping_union_of(arena(), live, work_reg->live_spans());

        if (err == Error::kOk) {
          work_reg->set_home_reg_id(phys_id);
          work_reg->mark_allocated();
          live.swap(tmp_spans);
          break;
        }

        if (ASMJIT_UNLIKELY(err != Error::kByPass)) {
          return err;
        }

        phys_regs ^= Support::bit_mask<RegMask>(phys_id);
      }
    }
  }

  // Try to pack the rest.
  if (!work_regs.is_empty()) {
    size_t dst_index = 0;

    for (size_t index = 0; index < num_work_regs; index++) {
      RAWorkReg* work_reg = work_regs[index];
      if (work_reg->is_allocated()) {
        continue;
      }

      RegMask remaining_phys_regs = available_regs;
      if (remaining_phys_regs & work_reg->preferred_mask()) {
        remaining_phys_regs &= work_reg->preferred_mask();
      }

      RegMask phys_regs = remaining_phys_regs & ~preserved_regs;
      remaining_phys_regs &= preserved_regs;

      for (;;) {
        if (!phys_regs) {
          if (!remaining_phys_regs) {
            break;
          }
          phys_regs = remaining_phys_regs;
          remaining_phys_regs = 0;
        }

        uint32_t phys_id = Support::ctz(phys_regs);

        if (work_reg->clobber_survival_mask()) {
          RegMask preferred_mask = (phys_regs | remaining_phys_regs) & work_reg->clobber_survival_mask();
          if (preferred_mask) {
            if (preferred_mask & ~remaining_phys_regs) {
              preferred_mask &= ~remaining_phys_regs;
            }
            phys_id = Support::ctz(preferred_mask);
          }
        }

        RALiveSpans& live = _global_live_spans[group][phys_id];
        Error err = tmp_spans.non_overlapping_union_of(arena(), live, work_reg->live_spans());

        if (err == Error::kOk) {
          work_reg->set_home_reg_id(phys_id);
          work_reg->mark_allocated();
          live.swap(tmp_spans);
          break;
        }

        if (ASMJIT_UNLIKELY(err != Error::kByPass)) {
          return err;
        }

        phys_regs &= ~Support::bit_mask<RegMask>(phys_id);
        remaining_phys_regs &= ~Support::bit_mask<RegMask>(phys_id);
      }

      // Keep it in `work_regs` if it was not allocated.
      if (!phys_regs) {
        work_regs[dst_index++] = work_reg;
      }
    }

    work_regs._set_size(dst_index);
    num_work_regs = dst_index;
  }

  ASMJIT_RA_LOG_COMPLEX({
    for (uint32_t phys_id = 0; phys_id < phys_count; phys_id++) {
      RALiveSpans& live = _global_live_spans[group][phys_id];
      if (live.is_empty()) {
        continue;
      }

      sb.clear();
      RAPass_dump_spans(sb, phys_id, live);
      logger->log(sb);
    }
  });

  // Maybe unused if logging is disabled.
  Support::maybe_unused(phys_count);

  if (work_regs.is_empty()) {
    ASMJIT_RA_LOG_FORMAT("  completed.\n");
  }
  else {
    _strategy[group].set_type(RAStrategyType::kComplex);
    for (RAWorkReg* work_reg : work_regs) {
      work_reg->mark_stack_preferred();
    }

    ASMJIT_RA_LOG_COMPLEX({
      size_t count = work_regs.size();
      sb.clear();
      sb.append_format("  Unassigned (%zu): ", count);
      for (uint32_t i = 0; i < num_work_regs; i++) {
        RAWorkReg* work_reg = work_regs[i];
        if (i) {
          sb.append(", ");
        }
        Formatter::format_virt_reg_name(sb, work_reg->virt_reg());
      }
      sb.append('\n');
      logger->log(sb);
    });
  }

  return Error::kOk;
}

// BaseRAPass - Allocation - Local
// ===============================

Error BaseRAPass::run_local_allocator() noexcept {
  RALocalAllocator lra(*this);
  ASMJIT_PROPAGATE(lra.init());

  if (!block_count()) {
    return Error::kOk;
  }

  // The allocation is done when this reaches zero.
  size_t blocks_remaining = reachable_block_count();

  // Current block.
  uint32_t block_id = 0;
  RABlock* block = _blocks[block_id];

  // The first block (entry) must always be reachable.
  ASMJIT_ASSERT(block->is_reachable());

  // Assign function arguments for the initial block. The `lra` is valid now.
  ASMJIT_PROPAGATE(lra.make_initial_assignment());
  ASMJIT_PROPAGATE(set_block_entry_assignment(block, block, lra._cur_assignment));

  // The loop starts from the first block and iterates blocks in order, however, the algorithm also allows to jump to
  // any other block when finished if it's a jump target. In-order iteration just makes sure that all blocks are visited.
  for (;;) {
    BaseNode* first = block->first();
    BaseNode* last = block->last();
    BaseNode* terminator = block->has_terminator() ? last : nullptr;

    BaseNode* before_first = first->prev();
    BaseNode* after_last = last->next();

    bool is_unconditional_jump = false;
    RABlock* consecutive = block->has_successors() ? block->successors()[0] : nullptr;

    lra.set_block(block);
    block->make_allocated();

    BaseNode* node = first;
    while (node != after_last) {
      BaseNode* next = node->next();
      if (node->is_inst()) {
        InstNode* inst = node->as<InstNode>();

        if (ASMJIT_UNLIKELY(inst == terminator)) {
          Span<RABlock*> successors = block->successors();
          if (block->has_consecutive()) {
            ASMJIT_PROPAGATE(lra.alloc_branch(inst, successors.last(), successors.first()));

            node = next;
            continue;
          }
          else if (successors.size() > 1) {
            RABlock* cont = block->has_consecutive() ? successors.first() : nullptr;
            ASMJIT_PROPAGATE(lra.alloc_jump_table(inst, successors, cont));

            node = next;
            continue;
          }
          else {
            // Otherwise this is an unconditional jump, special handling isn't required.
            is_unconditional_jump = true;
          }
        }

        ASMJIT_PROPAGATE(lra.alloc_instruction(inst));
        if (inst->type() == NodeType::kInvoke) {
          ASMJIT_PROPAGATE(emit_pre_call(inst->as<InvokeNode>()));
        }
        else {
          ASMJIT_PROPAGATE(lra.spill_after_allocation(inst));
        }
      }
      node = next;
    }

    if (consecutive) {
      BaseNode* prev = after_last ? after_last->prev() : cc().last_node();
      cc().set_cursor(is_unconditional_jump ? prev->prev() : prev);

      if (consecutive->has_entry_assignment()) {
        ASMJIT_PROPAGATE(lra.switch_to_assignment(consecutive->entry_phys_to_work_map(), consecutive->live_in(), consecutive->is_allocated(), false));
      }
      else {
        ASMJIT_PROPAGATE(lra.spill_regs_before_entry(consecutive));
        ASMJIT_PROPAGATE(set_block_entry_assignment(consecutive, block, lra._cur_assignment));
        lra._cur_assignment.copy_from(consecutive->entry_phys_to_work_map());
      }
    }

    // Important as the local allocator can insert instructions before
    // and after any instruction within the basic block.
    block->set_first(before_first->next());
    block->set_last(after_last ? after_last->prev() : cc().last_node());

    if (--blocks_remaining == 0) {
      break;
    }

    // Switch to the next consecutive block, if any.
    if (consecutive) {
      block = consecutive;
      if (!block->is_allocated()) {
        continue;
      }
    }

    // Get the next block.
    for (;;) {
      if (++block_id >= block_count()) {
        block_id = 0;
      }

      block = _blocks[block_id];
      if (!block->is_reachable() || block->is_allocated() || !block->has_entry_assignment()) {
        continue;
      }

      break;
    }

    // If we switched to another block we have to update the local allocator.
    ASMJIT_PROPAGATE(lra.replace_assignment(block->entry_phys_to_work_map()));
  }

  _clobbered_regs.op<Support::Or>(lra._clobbered_regs);
  return Error::kOk;
}

Error BaseRAPass::set_block_entry_assignment(RABlock* block, const RABlock* from_block, const RAAssignment& from_assignment) noexcept {
  if (block->has_shared_assignment_id()) {
    uint32_t shared_assignment_id = block->shared_assignment_id();

    // Shouldn't happen. Entry assignment of a block that has a shared-state will assign to all blocks
    // with the same shared_assignment_id. It's a bug if the shared state has been already assigned.
    if (!_shared_assignments[shared_assignment_id].is_empty()) {
      return make_error(Error::kInvalidState);
    }

    return set_shared_assignment(shared_assignment_id, from_assignment);
  }

  PhysToWorkMap* phys_to_work_map = clone_phys_to_work_map(from_assignment.phys_to_work_map());
  if (ASMJIT_UNLIKELY(!phys_to_work_map)) {
    return make_error(Error::kOutOfMemory);
  }

  block->set_entry_assignment(phys_to_work_map);

  // True if this is the first (entry) block, nothing to do in this case.
  if (block == from_block) {
    // Entry block should never have a shared state.
    if (block->has_shared_assignment_id()) {
      return make_error(Error::kInvalidState);
    }

    return Error::kOk;
  }

  Span<const BitWord> live_out = from_block->live_out();
  Span<const BitWord> live_in = block->live_in();

  // It's possible that `from_block` has LIVE-OUT regs that `block` doesn't
  // have in LIVE-IN, these have to be unassigned.
  {
    Support::BitVectorOpIterator<BitWord, Support::AndNot> it(live_out, live_in);
    while (it.has_next()) {
      RAWorkId work_id = RAWorkId(it.next());
      RAWorkReg* work_reg = work_reg_by_id(work_id);

      RegGroup group = work_reg->group();
      uint32_t phys_id = from_assignment.work_to_phys_id(group, work_id);

      if (phys_id != RAAssignment::kPhysNone) {
        phys_to_work_map->unassign(group, phys_id, _phys_reg_index.get(group) + phys_id);
      }
    }
  }

  return block_entry_assigned(phys_to_work_map);
}

Error BaseRAPass::set_shared_assignment(uint32_t shared_assignment_id, const RAAssignment& from_assignment) noexcept {
  ASMJIT_ASSERT(_shared_assignments[shared_assignment_id].is_empty());

  PhysToWorkMap* phys_to_work_map = clone_phys_to_work_map(from_assignment.phys_to_work_map());
  if (ASMJIT_UNLIKELY(!phys_to_work_map)) {
    return make_error(Error::kOutOfMemory);
  }

  _shared_assignments[shared_assignment_id].assign_phys_to_work_map(phys_to_work_map);
  ASMJIT_PROPAGATE(_shared_assignments[shared_assignment_id]._live_in.resize(arena(), multi_work_reg_count()));

  Span<BitWord> shared_live_in = _shared_assignments[shared_assignment_id]._live_in.as_span();
  Support::Array<uint32_t, Globals::kNumVirtGroups> shared_assigned {};

  for (RABlock* block : blocks()) {
    if (block->shared_assignment_id() == shared_assignment_id) {
      ASMJIT_ASSERT(!block->has_entry_assignment());

      PhysToWorkMap* entry_phys_to_work_map = clone_phys_to_work_map(from_assignment.phys_to_work_map());
      if (ASMJIT_UNLIKELY(!entry_phys_to_work_map)) {
        return make_error(Error::kOutOfMemory);
      }

      block->set_entry_assignment(entry_phys_to_work_map);

      Span<const BitWord> live_in = block->live_in();
      BitOps::or_(shared_live_in, shared_live_in, live_in);

      for (RegGroup group : Support::enumerate(RegGroup::kMaxVirt)) {
        shared_assigned[group] |= entry_phys_to_work_map->assigned[group];

        uint32_t phys_base_index = _phys_reg_index.get(group);
        Support::BitWordIterator<RegMask> it(entry_phys_to_work_map->assigned[group]);

        while (it.has_next()) {
          uint32_t phys_id = it.next();
          RAWorkId work_id = entry_phys_to_work_map->work_ids[phys_base_index + phys_id];

          // Should not happen as a register that only lives in a single basic block should not appear in the map.
          ASMJIT_ASSERT(uint32_t(work_id) < _multi_work_reg_count);

          if (!BitOps::bit_at(live_in, work_id)) {
            entry_phys_to_work_map->unassign(group, phys_id, phys_base_index + phys_id);
          }
        }
      }
    }
  }

  for (RegGroup group : Support::enumerate(RegGroup::kMaxVirt)) {
    uint32_t phys_base_index = _phys_reg_index.get(group);
    Support::BitWordIterator<RegMask> it(_available_regs[group] & ~shared_assigned[group]);

    while (it.has_next()) {
      uint32_t phys_id = it.next();
      if (Support::bit_test(phys_to_work_map->assigned[group], phys_id)) {
        phys_to_work_map->unassign(group, phys_id, phys_base_index + phys_id);
      }
    }
  }

  return block_entry_assigned(phys_to_work_map);
}

Error BaseRAPass::block_entry_assigned(const PhysToWorkMap* phys_to_work_map) noexcept {
  // Complex allocation strategy requires to record register assignments upon block entry (or per shared state).
  for (RegGroup group : Support::enumerate(RegGroup::kMaxVirt)) {
    if (!_strategy[group].is_complex()) {
      continue;
    }

    uint32_t phys_base_index = _phys_reg_index.get(group);
    Support::BitWordIterator<RegMask> it(phys_to_work_map->assigned[group]);

    while (it.has_next()) {
      uint32_t phys_id = it.next();
      RAWorkId work_id = phys_to_work_map->work_ids[phys_base_index + phys_id];

      RAWorkReg* work_reg = work_reg_by_id(work_id);
      work_reg->add_allocated_mask(Support::bit_mask<RegMask>(phys_id));
    }
  }

  return Error::kOk;
}

// BaseRAPass - Allocation - Utilities
// ===================================

Error BaseRAPass::use_temporary_mem(BaseMem& out, uint32_t size, uint32_t alignment) noexcept {
  ASMJIT_ASSERT(alignment <= 64);

  if (_temporary_mem.is_none()) {
    ASMJIT_PROPAGATE(cc()._new_stack(Out(_temporary_mem.as<BaseMem>()), size, alignment));
  }
  else {
    ASMJIT_ASSERT(_temporary_mem.as<BaseMem>().is_reg_home());

    uint32_t virt_id = _temporary_mem.as<BaseMem>().base_id();
    VirtReg* virt_reg = cc().virt_reg_by_id(virt_id);

    cc().set_stack_size(virt_id, Support::max(virt_reg->virt_size(), size),
                               Support::max(virt_reg->alignment(), alignment));
  }

  out = _temporary_mem.as<BaseMem>();
  return Error::kOk;
}

// BaseRAPass - Allocation - Prolog & Epilog
// =========================================

Error BaseRAPass::update_stack_frame() noexcept {
  // Update some StackFrame information that we updated during allocation. The only information we don't have at the
  // moment is final local stack size, which is calculated last.
  FuncFrame& frame = func()->frame();
  for (RegGroup group : Support::enumerate(RegGroup::kMaxVirt)) {
    frame.add_dirty_regs(group, _clobbered_regs[group]);
  }
  frame.set_local_stack_alignment(_stack_allocator.alignment());

  // If there are stack arguments that are not assigned to registers upon entry and the function doesn't require
  // dynamic stack alignment we keep these arguments where they are. This will also mark all stack slots that match
  // these arguments as allocated.
  if (_num_stack_args_to_stack_slots) {
    ASMJIT_PROPAGATE(_mark_stack_args_to_keep());
  }

  // Calculate offsets of all stack slots and update StackSize to reflect the calculated local stack size.
  ASMJIT_PROPAGATE(_stack_allocator.calculate_stack_frame());
  frame.set_local_stack_size(_stack_allocator.stack_size());

  // Update the stack frame based on `_args_assignment` and finalize it. Finalization means to apply final calculation
  // to the stack layout.
  ASMJIT_PROPAGATE(_args_assignment.update_func_frame(frame));
  ASMJIT_PROPAGATE(frame.finalize());

  // StackAllocator allocates all stots starting from [0], adjust them when necessary.
  if (frame.local_stack_offset() != 0) {
    ASMJIT_PROPAGATE(_stack_allocator.adjust_slot_offsets(int32_t(frame.local_stack_offset())));
  }

  // Again, if there are stack arguments allocated in function's stack we have to handle them. This handles all cases
  // (either regular or dynamic stack alignment).
  if (_num_stack_args_to_stack_slots) {
    ASMJIT_PROPAGATE(_update_stack_args());
  }

  return Error::kOk;
}

Error BaseRAPass::_mark_stack_args_to_keep() noexcept {
  FuncFrame& frame = func()->frame();
  bool has_sa_reg = frame.has_preserved_fp() || !frame.has_dynamic_alignment();

  ArenaVector<RAWorkReg*>& work_regs = _work_regs;
  size_t num_work_regs = work_reg_count();

  for (size_t work_id = 0; work_id < num_work_regs; work_id++) {
    RAWorkReg* work_reg = work_regs[work_id];
    if (work_reg->has_flag(RAWorkRegFlags::kStackArgToStack)) {
      ASMJIT_ASSERT(work_reg->has_arg_index());
      const FuncValue& src_arg = _func->detail().arg(work_reg->arg_index());

      // If the register doesn't have stack slot then we failed. It doesn't make much sense as it was marked as
      // `kFlagStackArgToStack`, which requires the WorkReg was live-in upon function entry.
      RAStackSlot* slot = work_reg->stack_slot();
      if (ASMJIT_UNLIKELY(!slot)) {
        return make_error(Error::kInvalidState);
      }

      if (has_sa_reg && src_arg.is_stack() && !src_arg.is_indirect()) {
        uint32_t type_size = TypeUtils::size_of(src_arg.type_id());
        if (type_size == slot->size()) {
          slot->add_flags(RAStackSlot::kFlagStackArg);
          continue;
        }
      }

      // NOTE: Update StackOffset here so when `_args_assignment.update_func_frame()` is called it will take into
      // consideration moving to stack slots. Without this we may miss some scratch registers later.
      FuncValue& dst_arg = _args_assignment.arg(work_reg->arg_index(), work_reg->arg_value_index());
      dst_arg.assign_stack_offset(0);
    }
  }

  return Error::kOk;
}

Error BaseRAPass::_update_stack_args() noexcept {
  FuncFrame& frame = func()->frame();

  ArenaVector<RAWorkReg*>& work_regs = _work_regs;
  size_t num_work_regs = work_reg_count();

  for (size_t work_id = 0; work_id < num_work_regs; work_id++) {
    RAWorkReg* work_reg = work_regs[work_id];
    if (work_reg->has_flag(RAWorkRegFlags::kStackArgToStack)) {
      ASMJIT_ASSERT(work_reg->has_arg_index());
      RAStackSlot* slot = work_reg->stack_slot();

      if (ASMJIT_UNLIKELY(!slot)) {
        return make_error(Error::kInvalidState);
      }

      if (slot->is_stack_arg()) {
        const FuncValue& src_arg = _func->detail().arg(work_reg->arg_index());
        if (frame.has_preserved_fp()) {
          slot->set_base_reg_id(_fp.id());
          slot->set_offset(int32_t(frame.sa_offset_from_sa()) + src_arg.stack_offset());
        }
        else {
          slot->set_offset(int32_t(frame.sa_offset_from_sp()) + src_arg.stack_offset());
        }
      }
      else {
        FuncValue& dst_arg = _args_assignment.arg(work_reg->arg_index(), work_reg->arg_value_index());
        dst_arg.set_stack_offset(slot->offset());
      }
    }
  }

  return Error::kOk;
}

Error BaseRAPass::insert_prolog_epilog() noexcept {
  FuncFrame& frame = _func->frame();

  cc().set_cursor(func());
  ASMJIT_PROPAGATE(cc().emit_prolog(frame));
  ASMJIT_PROPAGATE(_emit_helper_ptr->emit_args_assignment(frame, _args_assignment));

  cc().set_cursor(func()->exit_node());
  ASMJIT_PROPAGATE(cc().emit_epilog(frame));

  return Error::kOk;
}

// BaseRAPass - Rewriter
// =====================

// [[pure virtual]]
Error BaseRAPass::rewrite() noexcept {
  return make_error(Error::kInvalidState);
}

// BaseRAPass - Emit
// =================

// [[pure virtual]]
Error BaseRAPass::emit_move(RAWorkReg* work_reg, uint32_t dst_phys_id, uint32_t src_phys_id) noexcept {
  Support::maybe_unused(work_reg, dst_phys_id, src_phys_id);
  return make_error(Error::kInvalidState);
}

// [[pure virtual]]
Error BaseRAPass::emit_swap(RAWorkReg* a_reg, uint32_t a_phys_id, RAWorkReg* b_reg, uint32_t b_phys_id) noexcept {
  Support::maybe_unused(a_reg, a_phys_id, b_reg, b_phys_id);
  return make_error(Error::kInvalidState);
}

// [[pure virtual]]
Error BaseRAPass::emit_load(RAWorkReg* work_reg, uint32_t dst_phys_id) noexcept {
  Support::maybe_unused(work_reg, dst_phys_id);
  return make_error(Error::kInvalidState);
}

// [[pure virtual]]
Error BaseRAPass::emit_save(RAWorkReg* work_reg, uint32_t src_phys_id) noexcept {
  Support::maybe_unused(work_reg, src_phys_id);
  return make_error(Error::kInvalidState);
}

// [[pure virtual]]
Error BaseRAPass::emit_jump(const Label& label) noexcept {
  Support::maybe_unused(label);
  return make_error(Error::kInvalidState);
}

Error BaseRAPass::emit_pre_call(InvokeNode* invoke_node) noexcept {
  Support::maybe_unused(invoke_node);
  return make_error(Error::kOk);
}

// BaseRAPass - Logging
// ====================

#ifndef ASMJIT_NO_LOGGING
static void RAPass_formatLiveness(BaseRAPass* pass, String& sb, const RAInst* ra_inst) noexcept {
  Support::maybe_unused(pass);

  const RATiedReg* tied_regs = ra_inst->tied_regs();
  uint32_t tied_count = ra_inst->tied_count();

  for (uint32_t i = 0; i < tied_count; i++) {
    const RATiedReg& tied_reg = tied_regs[i];

    if (i != 0) {
      sb.append(' ');
    }

    Formatter::format_virt_reg_name(sb, tied_reg.work_reg()->virt_reg());
    sb.append('{');
    sb.append(tied_reg.is_read_write() ? 'X' :
              tied_reg.is_read()      ? 'R' :
              tied_reg.is_write()     ? 'W' : '?');

    if (tied_reg.is_lead_consecutive()) {
      sb.append_format("|Lead[%u]", tied_reg.consecutive_data() + 1u);
    }

    if (tied_reg.has_use_id()) {
      sb.append_format("|Use=%u", tied_reg.use_id());
    }
    else if (tied_reg.is_use()) {
      sb.append("|Use");
    }

    if (tied_reg.is_use_consecutive() && !tied_reg.is_lead_consecutive()) {
      sb.append_format("+%u", tied_reg.consecutive_data());
    }

    if (tied_reg.has_out_id()) {
      sb.append_format("|Out=%u", tied_reg.out_id());
    }
    else if (tied_reg.is_out()) {
      sb.append("|Out");
    }

    if (tied_reg.is_out_consecutive() && !tied_reg.is_lead_consecutive()) {
      sb.append_format("+%u", tied_reg.consecutive_data());
    }

    if (tied_reg.is_first()) {
      sb.append("|First");
    }

    if (tied_reg.is_last()) {
      sb.append("|Last");
    }

    if (tied_reg.is_kill()) {
      sb.append("|Kill");
    }

    sb.append("}");
  }
}

ASMJIT_FAVOR_SIZE Error BaseRAPass::annotate_code() noexcept {
  StringTmp<1024> sb;

  for (const RABlock* block : _blocks) {
    BaseNode* node = block->first();
    if (!node) {
      continue;
    }

    BaseNode* last = block->last();
    for (;;) {
      sb.clear();
      Formatter::format_node(sb, _format_options, &_cb, node);

      if (has_diagnostic_option(DiagnosticOptions::kRADebugLiveness) && node->is_inst() && node->has_pass_data()) {
        const RAInst* ra_inst = node->pass_data<RAInst>();
        if (ra_inst->tied_count() > 0) {
          sb.pad_end(40);
          sb.append(" | ");
          RAPass_formatLiveness(this, sb, ra_inst);
        }
      }

      node->set_inline_comment(static_cast<char*>(cc()._builder_arena.dup(sb.data(), sb.size(), true)));
      if (node == last) {
        break;
      }
      node = node->next();
    }
  }

  return Error::kOk;
}

ASMJIT_FAVOR_SIZE Error BaseRAPass::dump_block_ids(String& sb, Span<RABlock*> blocks) noexcept {
  for (size_t i = 0; i < blocks.size(); i++) {
    const RABlock* block = blocks[i];
    ASMJIT_PROPAGATE(sb.append_format(!i ? "#%u" : ", #%u", uint32_t(block->block_id())));
  }
  return Error::kOk;
}

ASMJIT_FAVOR_SIZE Error BaseRAPass::dump_block_liveness(String& sb, const RABlock* block) noexcept {
  for (uint32_t live_type = 0; live_type < RABlock::kLiveCount; live_type++) {
    const char* info_name = live_type == RABlock::kLiveIn  ? "IN  " :
                            live_type == RABlock::kLiveOut ? "OUT " : "KILL";

    Support::BitVectorIterator<BitWord> it(block->live_bits(live_type));
    if (it.has_next()) {
      bool first = true;

      sb.append_format("    %s [", info_name);
      do {
        const RAWorkReg* work_reg = work_reg_by_id(RAWorkId(it.next()));
        if (!first) {
          sb.append(", ");
        }

        Formatter::format_virt_reg_name(sb, work_reg->virt_reg());
        first = false;
      } while (it.has_next());
      sb.append("]\n");
    }
  }

  return Error::kOk;
}

ASMJIT_FAVOR_SIZE Error BaseRAPass::dump_live_spans(String& sb) noexcept {
  size_t max_size = _max_work_reg_name_size;

  for (RAWorkReg* work_reg : _work_regs.iterate()) {
    RALiveStats& stats = work_reg->live_stats();

    sb.append("  ");
    size_t old_size = sb.size();

    Formatter::format_virt_reg_name(sb, work_reg->virt_reg());
    sb.pad_end(old_size + max_size);

    sb.append_format(" {ra_id=%-5u virt_id=%-5u width=%-5u freq=%0.5f priority=%0.5f ",
      uint32_t(work_reg->work_id()),
      work_reg->virt_id(),
      stats.width(),
      double(stats.freq()),
      double(stats.priority()));

    if (work_reg->is_within_single_basic_block()) {
      sb.append_format("bb=#%-4u", uint32_t(work_reg->single_basic_block_id()));
    }
    else {
      sb.append("bb=<...>");
    }

    sb.append_format("}: ");

    RALiveSpans& live_spans = work_reg->live_spans();
    for (uint32_t x = 0; x < live_spans.size(); x++) {
      const RALiveSpan& live_span = live_spans[x];
      if (x) {
        sb.append(", ");
      }
      sb.append_format("[%u:%u]", live_span.a, live_span.b);
    }

    sb.append('\n');
  }

  return Error::kOk;
}
#endif

ASMJIT_END_NAMESPACE

#endif // !ASMJIT_NO_COMPILER
