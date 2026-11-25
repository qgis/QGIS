// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_RACFGBUILDER_P_H_INCLUDED
#define ASMJIT_CORE_RACFGBUILDER_P_H_INCLUDED

#include <asmjit/core/api-config.h>
#ifndef ASMJIT_NO_COMPILER

#include <asmjit/core/formatter.h>
#include <asmjit/core/racfgblock_p.h>
#include <asmjit/core/rainst_p.h>
#include <asmjit/core/rapass_p.h>

ASMJIT_BEGIN_NAMESPACE

//! \cond INTERNAL
//! \addtogroup asmjit_ra
//! \{

template<typename This>
class RACFGBuilderT {
public:
  //! \name Constants
  //! \{

  static inline constexpr uint32_t kRootIndentation = 2;
  static inline constexpr uint32_t kCodeIndentation = 4;

  // NOTE: This is a bit hacky. There are some nodes which are processed twice (see `on_before_invoke()` and
  // `on_before_ret()`) as they can insert some nodes around them. Since we don't have any flags to mark these
  // we just use their position that is [at that time] unassigned.
  static inline constexpr NodePosition kNodePositionUnassigned = NodePosition(0u);
  static inline constexpr NodePosition kNodePositionDidOnBefore = NodePosition(0xFFFFFFFFu);

  //! \}

  //! \name Members
  //! \{

  BaseRAPass& _pass;
  BaseCompiler& _cc;
  RABlock* _cur_block = nullptr;
  RABlock* _ret_block = nullptr;
  FuncNode* _func_node = nullptr;
  RARegsStats _block_reg_stats {};
  uint32_t _exit_label_id = Globals::kInvalidId;
  ArenaVector<uint32_t> _shared_assignments_map {};

  // Only used by logging, it's fine to be here to prevent more #ifdefs...
  bool _has_code = false;
  RABlock* _last_logged_block = nullptr;

#ifndef ASMJIT_NO_LOGGING
  Logger* _logger = nullptr;
  FormatOptions _format_options {};
  StringTmp<512> _sb;
#endif

  //! \}

  inline RACFGBuilderT(BaseRAPass& pass) noexcept
    : _pass(pass),
      _cc(pass.cc()) {
#ifndef ASMJIT_NO_LOGGING
    _logger = _pass.has_diagnostic_option(DiagnosticOptions::kRADebugCFG) ? _pass.logger() : nullptr;
    if (_logger) {
      _format_options = _logger->options();
    }
#endif
  }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BaseCompiler& cc() const noexcept { return _cc; }

  //! \name Run
  //! \{

  //! Called per function by an architecture-specific CFG builder.
  [[nodiscard]]
  Error run() noexcept {
    log("[build_cfg_nodes]\n");
    ASMJIT_PROPAGATE(prepare());

    log_node(_func_node, kRootIndentation);
    log_block(_cur_block, kRootIndentation);

    RABlock* entry_block = _cur_block;
    BaseNode* node = _func_node->next();

    if (ASMJIT_UNLIKELY(!node)) {
      return make_error(Error::kInvalidState);
    }

    _cur_block->set_first(_func_node);
    _cur_block->set_last(_func_node);

    RAInstBuilder ib;
    ArenaVector<RABlock*> blocks_with_unknown_jumps;

    for (;;) {
      BaseNode* next = node->next();
      ASMJIT_ASSERT(node->position() == kNodePositionUnassigned || node->position() == kNodePositionDidOnBefore);

      if (node->is_inst()) {
        // Instruction | Jump | Invoke | Return
        // ------------------------------------

        // Handle `InstNode`, `InvokeNode`, and `FuncRetNode`. All of them share the same interface that provides
        // operands that have read/write semantics.
        if (ASMJIT_UNLIKELY(!_cur_block)) {
          // Unreachable code has to be removed, we cannot allocate registers in such code as we cannot do proper
          // liveness analysis in such case.
          remove_node(node);
          node = next;
          continue;
        }

        _has_code = true;

        if (node->is_invoke() || node->is_func_ret()) {
          if (node->position() != kNodePositionDidOnBefore) {
            // Call and Reg are complicated as they may insert some surrounding code around them. The simplest
            // approach is to get the previous node, call the `on_before_xxx()` handlers and then check whether
            // anything changed and restart if so. By restart we mean that the current `node` would go back to
            // the first possible inserted node by `on_before_invoke()` or `on_before_ret()`.
            BaseNode* prev = node->prev();

            if (node->type() == NodeType::kInvoke) {
              ASMJIT_PROPAGATE(static_cast<This*>(this)->on_before_invoke(node->as<InvokeNode>()));
            }
            else {
              ASMJIT_PROPAGATE(static_cast<This*>(this)->on_before_ret(node->as<FuncRetNode>()));
            }

            if (prev != node->prev()) {
              // If this was the first node in the block and something was
              // inserted before it then we have to update the first block.
              if (_cur_block->first() == node) {
                _cur_block->set_first(prev->next());
              }

              node->set_position(kNodePositionDidOnBefore);
              node = prev->next();

              // `on_before_invoke()` and `on_before_ret()` can only insert instructions.
              ASMJIT_ASSERT(node->is_inst());
            }

            // Necessary if something was inserted after `node`, but nothing before.
            next = node->next();
          }
          else {
            // Change the position back to its original value.
            node->set_position(kNodePositionUnassigned);
          }
        }

        InstNode* inst = node->as<InstNode>();
        log_node(inst, kCodeIndentation);

        InstControlFlow cf = InstControlFlow::kRegular;
        ib.reset(_cur_block->block_id());
        ASMJIT_PROPAGATE(static_cast<This*>(this)->on_instruction(inst, cf, ib));

        if (node->is_invoke()) {
          ASMJIT_PROPAGATE(static_cast<This*>(this)->on_invoke(inst->as<InvokeNode>(), ib));
        }

        if (node->is_func_ret()) {
          ASMJIT_PROPAGATE(static_cast<This*>(this)->on_ret(inst->as<FuncRetNode>(), ib));
          cf = InstControlFlow::kReturn;
        }

        if (cf == InstControlFlow::kJump) {
          uint32_t fixed_reg_count = 0;
          for (RATiedReg& tied_reg : ib) {
            RAWorkReg* work_reg = tied_reg.work_reg();
            if (work_reg->group() == RegGroup::kGp) {
              uint32_t use_id = tied_reg.use_id();
              if (use_id == Reg::kIdBad) {
                use_id = _pass._scratch_reg_indexes[fixed_reg_count++];
                tied_reg.set_use_id(use_id);
              }
              _cur_block->add_exit_scratch_gp_regs(Support::bit_mask<RegMask>(use_id));
            }
          }
        }

        ASMJIT_PROPAGATE(_pass.assign_ra_inst(inst, _cur_block, ib));
        _block_reg_stats.combine_with(ib._stats);

        if (cf != InstControlFlow::kRegular) {
          // Support for conditional and unconditional jumps.
          if (cf == InstControlFlow::kJump || cf == InstControlFlow::kBranch) {
            _cur_block->set_last(node);
            _cur_block->add_flags(RABlockFlags::kHasTerminator);
            _cur_block->make_constructed(_block_reg_stats);

            if (!inst->has_option(InstOptions::kUnfollow)) {
              // Jmp/Jcc/Call/Loop/etc... require at least 1 operand that describes the jump target.
              Span<const Operand> operands = inst->operands();
              if (ASMJIT_UNLIKELY(operands.is_empty())) {
                return make_error(Error::kInvalidState);
              }

              if (operands.last().is_label()) {
                // Labels are easy for constructing the control flow.
                LabelNode* label_node;
                ASMJIT_PROPAGATE(cc().label_node_of(Out(label_node), operands.last().as<Label>()));

                RABlock* target_block = _pass.new_block_or_existing_at(label_node);
                if (ASMJIT_UNLIKELY(!target_block)) {
                  return make_error(Error::kOutOfMemory);
                }

                target_block->make_targetable();
                ASMJIT_PROPAGATE(_cur_block->append_successor(target_block));
              }
              else {
                // Not a label - could be jump with reg/mem operand, which means that it can go anywhere. Such jumps
                // must either be annotated so the CFG can be properly constructed, otherwise we assume the worst case
                // - can jump to any basic block.
                JumpAnnotation* jump_annotation = nullptr;
                _cur_block->add_flags(RABlockFlags::kHasJumpTable);

                if (inst->type() == NodeType::kJump) {
                  jump_annotation = inst->as<JumpNode>()->annotation();
                }

                if (jump_annotation) {
                  RABlockTimestamp timestamp = _pass.next_timestamp();
                  for (uint32_t id : jump_annotation->label_ids()) {
                    LabelNode* label_node;
                    ASMJIT_PROPAGATE(cc().label_node_of(Out(label_node), id));

                    RABlock* target_block = _pass.new_block_or_existing_at(label_node);
                    if (ASMJIT_UNLIKELY(!target_block)) {
                      return make_error(Error::kOutOfMemory);
                    }

                    // Prevents adding basic-block successors multiple times.
                    if (!target_block->has_timestamp(timestamp)) {
                      target_block->set_timestamp(timestamp);
                      target_block->make_targetable();
                      ASMJIT_PROPAGATE(_cur_block->append_successor(target_block));
                    }
                  }
                  ASMJIT_PROPAGATE(share_assignment_across_successors(_cur_block));
                }
                else {
                  ASMJIT_PROPAGATE(blocks_with_unknown_jumps.append(_pass.arena(), _cur_block));
                }
              }
            }

            if (cf == InstControlFlow::kJump) {
              // Unconditional jump makes the code after the jump unreachable, which will be removed instantly during
              // the CFG construction; as we cannot allocate registers for instructions that are not part of any block.
              // Of course we can leave these instructions as they are, however, that would only postpone the problem
              // as assemblers can't encode instructions that use virtual registers.
              _cur_block = nullptr;
            }
            else {
              node = next;
              if (ASMJIT_UNLIKELY(!node)) {
                return make_error(Error::kInvalidState);
              }

              RABlock* consecutive_block;
              if (node->type() == NodeType::kLabel) {
                if (node->has_pass_data()) {
                  consecutive_block = node->pass_data<RABlock>();
                }
                else {
                  consecutive_block = _pass.new_block(node);
                  if (ASMJIT_UNLIKELY(!consecutive_block)) {
                    return make_error(Error::kOutOfMemory);
                  }
                  node->set_pass_data<RABlock>(consecutive_block);
                }
              }
              else {
                consecutive_block = _pass.new_block(node);
                if (ASMJIT_UNLIKELY(!consecutive_block)) {
                  return make_error(Error::kOutOfMemory);
                }
              }

              _cur_block->add_flags(RABlockFlags::kHasConsecutive);
              ASMJIT_PROPAGATE(_cur_block->prepend_successor(consecutive_block));

              _cur_block = consecutive_block;
              _has_code = false;
              _block_reg_stats.reset();

              if (_cur_block->is_constructed()) {
                break;
              }
              ASMJIT_PROPAGATE(_pass.add_block(consecutive_block));

              log_block(_cur_block, kRootIndentation);
              continue;
            }
          }

          if (cf == InstControlFlow::kReturn) {
            _cur_block->set_last(node);
            _cur_block->make_constructed(_block_reg_stats);
            ASMJIT_PROPAGATE(_cur_block->append_successor(_ret_block));

            _cur_block = nullptr;
          }
        }
      }
      else if (node->type() == NodeType::kLabel) {
        // Label - Basic-Block Management
        // ------------------------------

        if (!_cur_block) {
          // If the current code is unreachable the label makes it reachable again. We may remove the whole block in
          // the future if it's not referenced though.
          _cur_block = node->pass_data<RABlock>();

          if (_cur_block) {
            // If the label has a block assigned we can either continue with it or skip it if the block has been
            // constructed already.
            if (_cur_block->is_constructed()) {
              break;
            }
          }
          else {
            // No block assigned - create a new one and assign it.
            _cur_block = _pass.new_block(node);
            if (ASMJIT_UNLIKELY(!_cur_block)) {
              return make_error(Error::kOutOfMemory);
            }
            node->set_pass_data<RABlock>(_cur_block);
          }

          _cur_block->make_targetable();
          _has_code = false;
          _block_reg_stats.reset();
          ASMJIT_PROPAGATE(_pass.add_block(_cur_block));
        }
        else {
          if (node->has_pass_data()) {
            RABlock* consecutive = node->pass_data<RABlock>();
            consecutive->make_targetable();

            if (_cur_block == consecutive) {
              // The label currently processed is part of the current block. This is only possible for multiple labels
              // that are right next to each other or labels that are separated by non-code nodes like directives and
              // comments.
              if (ASMJIT_UNLIKELY(_has_code)) {
                return make_error(Error::kInvalidState);
              }
            }
            else {
              // Label makes the current block constructed. There is a chance that the Label is not used, but we don't
              // know that at this point. In the worst case there would be two blocks next to each other, it's just fine.
              ASMJIT_ASSERT(_cur_block->last() != node);
              _cur_block->set_last(node->prev());
              _cur_block->add_flags(RABlockFlags::kHasConsecutive);
              _cur_block->make_constructed(_block_reg_stats);

              ASMJIT_PROPAGATE(_cur_block->append_successor(consecutive));
              ASMJIT_PROPAGATE(_pass.add_block(consecutive));

              _cur_block = consecutive;
              _has_code = false;
              _block_reg_stats.reset();
            }
          }
          else {
            // First time we see this label.
            if (_has_code || _cur_block == entry_block) {
              // Cannot continue the current block if it already contains some code or it's a block entry. We need to
              // create a new block and make it a successor.
              ASMJIT_ASSERT(_cur_block->last() != node);
              _cur_block->set_last(node->prev());
              _cur_block->add_flags(RABlockFlags::kHasConsecutive);
              _cur_block->make_constructed(_block_reg_stats);

              RABlock* consecutive = _pass.new_block(node);
              if (ASMJIT_UNLIKELY(!consecutive)) {
                return make_error(Error::kOutOfMemory);
              }
              consecutive->make_targetable();

              ASMJIT_PROPAGATE(_cur_block->append_successor(consecutive));
              ASMJIT_PROPAGATE(_pass.add_block(consecutive));

              _cur_block = consecutive;
              _has_code = false;
              _block_reg_stats.reset();
            }

            node->set_pass_data<RABlock>(_cur_block);
          }
        }

        if (_cur_block && _cur_block != _last_logged_block) {
          log_block(_cur_block, kRootIndentation);
        }
        log_node(node, kRootIndentation);

        // Unlikely: Assume that the exit label is reached only once per function.
        if (ASMJIT_UNLIKELY(node->as<LabelNode>()->label_id() == _exit_label_id)) {
          _cur_block->set_last(node);
          _cur_block->make_constructed(_block_reg_stats);
          ASMJIT_PROPAGATE(_pass.add_exit_block(_cur_block));

          _cur_block = nullptr;
        }
      }
      else {
        // Other Nodes | Function Exit
        // ---------------------------

        log_node(node, kCodeIndentation);

        if (node->type() == NodeType::kSentinel) {
          if (node == _func_node->end_node()) {
            // Make sure we didn't flow here if this is the end of the function sentinel.
            if (ASMJIT_UNLIKELY(_cur_block && _has_code)) {
              return make_error(Error::kInvalidState);
            }
            break;
          }
        }
        else if (node->type() == NodeType::kFunc) {
          // RAPass can only compile a single function at a time. If we
          // encountered a function it must be the current one, bail if not.
          if (ASMJIT_UNLIKELY(node != _func_node)) {
            return make_error(Error::kInvalidState);
          }
          // PASS if this is the first node.
        }
        else {
          // PASS if this is a non-interesting or unknown node.
        }
      }

      // Advance to the next node.
      node = next;

      // NOTE: We cannot encounter a NULL node, because every function must be terminated by a sentinel (`stop`)
      // node. If we encountered a NULL node it means that something went wrong and this node list is corrupted;
      // bail in such case.
      if (ASMJIT_UNLIKELY(!node)) {
        return make_error(Error::kInvalidState);
      }
    }

    if (_pass.has_dangling_blocks()) {
      return make_error(Error::kInvalidState);
    }

    for (RABlock* block : blocks_with_unknown_jumps) {
      ASMJIT_PROPAGATE(handle_block_with_unknown_jump(block));
    }

    return _pass.init_shared_assignments(_shared_assignments_map);
  }

  //! \}

  //! \name Prepare
  //! \{

  //! Prepares the CFG builder of the current function.
  [[nodiscard]]
  Error prepare() noexcept {
    FuncNode* func = _pass.func();
    BaseNode* node = nullptr;

    // Create entry and exit blocks.
    _func_node = func;
    _ret_block = _pass.new_block_or_existing_at(func->exit_node(), &node);

    if (ASMJIT_UNLIKELY(!_ret_block))
      return make_error(Error::kOutOfMemory);

    _ret_block->make_targetable();
    ASMJIT_PROPAGATE(_pass.add_exit_block(_ret_block));

    if (node != func) {
      _cur_block = _pass.new_block();
      if (ASMJIT_UNLIKELY(!_cur_block))
        return make_error(Error::kOutOfMemory);
    }
    else {
      // Function that has no code at all.
      _cur_block = _ret_block;
    }

    // Reset everything we may need.
    _block_reg_stats.reset();
    _exit_label_id = func->exit_node()->label_id();

    // Initially we assume there is no code in the function body.
    _has_code = false;

    return _pass.add_block(_cur_block);
  }

  //! \}

  //! \name Utilities
  //! \{

  //! Called when a `node` is removed, e.g. because of a dead code elimination.
  void remove_node(BaseNode* node) noexcept {
    log_node(node, kRootIndentation, "<Removed>");
    cc().remove_node(node);
  }

  //! Handles block with unknown jump, which could be a jump to a jump table.
  //!
  //! If we encounter such block we basically insert all existing blocks as successors except the function entry
  //! block and a natural successor, if such block exists.
  [[nodiscard]]
  Error handle_block_with_unknown_jump(RABlock* block) noexcept {
    // NOTE: Iterate from `1` as the first block is the entry block, we don't allow the entry to be a successor
    // of any block as it contains a function prolog sequence, which just cannot be re-executed (executed twice).
    Span blocks = _pass.blocks();
    RABlock* consecutive = block->consecutive();

    for (size_t i = 1; i < blocks.size(); i++) {
      RABlock* candidate = blocks[i];
      if (candidate == consecutive || !candidate->is_targetable()) {
        continue;
      }
      ASMJIT_PROPAGATE(block->append_successor(candidate));
    }

    return share_assignment_across_successors(block);
  }

  [[nodiscard]]
  Error share_assignment_across_successors(RABlock* block) noexcept {
    if (block->successors().size() <= 1u) {
      return Error::kOk;
    }

    RABlock* consecutive = block->consecutive();
    uint32_t shared_assignment_id = Globals::kInvalidId;

    for (RABlock* successor : block->successors()) {
      if (successor == consecutive) {
        continue;
      }

      if (successor->has_shared_assignment_id()) {
        if (shared_assignment_id == Globals::kInvalidId) {
          shared_assignment_id = successor->shared_assignment_id();
        }
        else {
          _shared_assignments_map[successor->shared_assignment_id()] = shared_assignment_id;
        }
      }
      else {
        if (shared_assignment_id == Globals::kInvalidId) {
          ASMJIT_PROPAGATE(new_shared_assignment_id(&shared_assignment_id));
        }
        successor->set_shared_assignment_id(shared_assignment_id);
      }
    }
    return Error::kOk;
  }

  [[nodiscard]]
  Error new_shared_assignment_id(uint32_t* out) noexcept {
    uint32_t id = uint32_t(_shared_assignments_map.size());
    ASMJIT_PROPAGATE(_shared_assignments_map.append(_pass.arena(), id));

    *out = id;
    return Error::kOk;
  }

  //! \}

  //! \name Logging
  //! \{

#ifndef ASMJIT_NO_LOGGING
  template<typename... Args>
  inline void log(const char* fmt, Args&&... args) noexcept {
    if (_logger) {
      _logger->logf(fmt, std::forward<Args>(args)...);
    }
  }

  inline void log_block(RABlock* block, uint32_t indentation = 0) noexcept {
    if (_logger) {
      _log_block(block, indentation);
    }
  }

  inline void log_node(BaseNode* node, uint32_t indentation = 0, const char* action = nullptr) noexcept {
    if (_logger) {
      _log_node(node, indentation, action);
    }
  }

  void _log_block(RABlock* block, uint32_t indentation) noexcept {
    _sb.clear();
    _sb.append_chars(' ', indentation);
    _sb.append_format("{#%u}\n", block->block_id());
    _logger->log(_sb);
    _last_logged_block = block;
  }

  void _log_node(BaseNode* node, uint32_t indentation, const char* action) noexcept {
    _sb.clear();
    _sb.append_chars(' ', indentation);
    if (action) {
      _sb.append(action);
      _sb.append(' ');
    }
    Formatter::format_node(_sb, _format_options, &_cc, node);
    _sb.append('\n');
    _logger->log(_sb);
  }
#else
  template<typename... Args>
  inline void log(const char* fmt, Args&&... args) noexcept {
    Support::maybe_unused(fmt);
    Support::maybe_unused(std::forward<Args>(args)...);
  }

  inline void log_block(RABlock* block, uint32_t indentation = 0) noexcept {
    Support::maybe_unused(block, indentation);
  }

  inline void log_node(BaseNode* node, uint32_t indentation = 0, const char* action = nullptr) noexcept {
    Support::maybe_unused(node, indentation, action);
  }
#endif

  //! \}
};

//! \}
//! \endcond

ASMJIT_END_NAMESPACE

#endif // !ASMJIT_NO_COMPILER
#endif // ASMJIT_CORE_RACFGBUILDER_P_H_INCLUDED
