// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#ifndef ASMJIT_NO_BUILDER

#include <asmjit/core/builder.h>
#include <asmjit/core/emitterutils_p.h>
#include <asmjit/core/errorhandler.h>
#include <asmjit/core/formatter.h>
#include <asmjit/core/logger.h>
#include <asmjit/support/support.h>

ASMJIT_BEGIN_NAMESPACE

// PostponedErrorHandler (Internal)
// ================================

//! Postponed error handler that never throws. Used as a temporal error handler
//! to run passes. If error occurs, the caller is notified and will call the
//! real error handler, that can throw.
class PostponedErrorHandler : public ErrorHandler {
public:
  void handle_error(Error err, const char* message, BaseEmitter* origin) override {
    Support::maybe_unused(err, origin);
    _message.assign(message);
  }

  StringTmp<128> _message;
};

// BaseBuilder - Utilities
// =======================

static void BaseBuilder_delete_passes(BaseBuilder* self) noexcept {
  for (Pass* pass : self->_passes) {
    pass->~Pass();
  }
  self->_passes.reset();
}

// BaseBuilder - Construction & Destruction
// ========================================

BaseBuilder::BaseBuilder() noexcept
  : BaseEmitter(EmitterType::kBuilder),
    _builder_arena(64u * 1024u),
    _pass_arena(64u * 1024u) {}

BaseBuilder::~BaseBuilder() noexcept {
  BaseBuilder_delete_passes(this);
}

// BaseBuilder - Node Management
// =============================

Error BaseBuilder::new_inst_node(Out<InstNode*> out, InstId inst_id, InstOptions inst_options, uint32_t op_count) {
  uint32_t op_capacity = InstNode::capacity_of_op_count(op_count);
  ASMJIT_ASSERT(op_capacity >= InstNode::kBaseOpCapacity);

  void* ptr = _builder_arena.alloc_oneshot(InstNode::node_size_of_op_capacity(op_capacity));
  if (ASMJIT_UNLIKELY(!ptr)) {
    return report_error(make_error(Error::kOutOfMemory));
  }

  out = new(Support::PlacementNew{ptr}) InstNode(inst_id, inst_options, op_count, op_capacity);
  return Error::kOk;
}

Error BaseBuilder::new_label_node(Out<LabelNode*> out) {
  out = nullptr;

  ASMJIT_PROPAGATE(new_node_t<LabelNode>(out));
  return register_label_node(*out);
}

Error BaseBuilder::new_align_node(Out<AlignNode*> out, AlignMode align_mode, uint32_t alignment) {
  out = nullptr;
  return new_node_t<AlignNode>(out, align_mode, alignment);
}

Error BaseBuilder::new_embed_data_node(Out<EmbedDataNode*> out, TypeId type_id, const void* data, size_t item_count, size_t repeat_count) {
  out = nullptr;

  uint32_t deabstract_delta = TypeUtils::deabstract_delta_of_size(register_size());
  TypeId final_type_id = TypeUtils::deabstract(type_id, deabstract_delta);

  if (ASMJIT_UNLIKELY(!TypeUtils::is_valid(final_type_id))) {
    return report_error(make_error(Error::kInvalidArgument));
  }

  uint32_t type_size = TypeUtils::size_of(final_type_id);
  Support::FastUInt8 of = 0;

  size_t node_size = Support::madd_overflow(item_count, size_t(type_size), sizeof(EmbedDataNode), &of);
  if (ASMJIT_UNLIKELY(of)) {
    return report_error(make_error(Error::kOutOfMemory));
  }

  EmbedDataNode* node = nullptr;
  ASMJIT_PROPAGATE(
    new_node_with_size_t<EmbedDataNode>(Out(node), Arena::aligned_size(node_size), type_id, uint8_t(type_size), item_count, repeat_count)
  );

  if (data) {
    memcpy(node->data(), data, node->data_size());
  }

  out = node;
  return Error::kOk;
}

Error BaseBuilder::new_const_pool_node(Out<ConstPoolNode*> out) {
  out = nullptr;

  ASMJIT_PROPAGATE(new_node_t<ConstPoolNode>(out, _builder_arena));
  return register_label_node(*out);
}

Error BaseBuilder::new_comment_node(Out<CommentNode*> out, const char* data, size_t size) {
  out = nullptr;

  if (data) {
    if (size == SIZE_MAX) {
      size = strlen(data);
    }

    if (size > 0) {
      data = static_cast<char*>(_builder_arena.dup(data, size, true));
      if (ASMJIT_UNLIKELY(!data)) {
        return report_error(make_error(Error::kOutOfMemory));
      }
    }
  }

  return new_node_t<CommentNode>(out, data);
}

BaseNode* BaseBuilder::add_node(BaseNode* node) noexcept {
  ASMJIT_ASSERT(!node->_prev);
  ASMJIT_ASSERT(!node->_next);
  ASMJIT_ASSERT(!node->is_active());

  if (!_cursor) {
    if (_node_list.is_empty()) {
      _node_list.reset(node, node);
    }
    else {
      node->_next = _node_list.first();
      _node_list._first->_prev = node;
      _node_list._first = node;
    }
  }
  else {
    BaseNode* prev = _cursor;
    BaseNode* next = _cursor->next();

    node->_prev = prev;
    node->_next = next;

    prev->_next = node;
    if (next) {
      next->_prev = node;
    }
    else {
      _node_list._last = node;
    }
  }

  node->_add_flags(NodeFlags::kIsActive);
  if (node->is_section()) {
    _dirty_section_links = true;
  }

  _cursor = node;
  return node;
}

BaseNode* BaseBuilder::add_after(BaseNode* node, BaseNode* ref) noexcept {
  ASMJIT_ASSERT(!node->_prev);
  ASMJIT_ASSERT(!node->_next);

  BaseNode* prev = ref;
  BaseNode* next = ref->next();

  node->_prev = prev;
  node->_next = next;

  node->_add_flags(NodeFlags::kIsActive);
  if (node->is_section()) {
    _dirty_section_links = true;
  }

  prev->_next = node;
  if (next) {
    next->_prev = node;
  }
  else {
    _node_list._last = node;
  }

  return node;
}

BaseNode* BaseBuilder::add_before(BaseNode* node, BaseNode* ref) noexcept {
  ASMJIT_ASSERT(!node->_prev);
  ASMJIT_ASSERT(!node->_next);
  ASMJIT_ASSERT(!node->is_active());
  ASMJIT_ASSERT(ref->is_active());

  BaseNode* prev = ref->prev();
  BaseNode* next = ref;

  node->_prev = prev;
  node->_next = next;

  node->_add_flags(NodeFlags::kIsActive);
  if (node->is_section()) {
    _dirty_section_links = true;
  }

  next->_prev = node;
  if (prev) {
    prev->_next = node;
  }
  else {
    _node_list._first = node;
  }

  return node;
}

BaseNode* BaseBuilder::remove_node(BaseNode* node) noexcept {
  if (!node->is_active()) {
    return node;
  }

  BaseNode* prev = node->prev();
  BaseNode* next = node->next();

  if (_node_list._first == node) {
    _node_list._first = next;
  }
  else {
    prev->_next = next;
  }

  if (_node_list._last == node) {
    _node_list._last  = prev;
  }
  else {
    next->_prev = prev;
  }

  node->_prev = nullptr;
  node->_next = nullptr;
  node->_clear_flags(NodeFlags::kIsActive);

  if (node->is_section()) {
    _dirty_section_links = true;
  }

  if (_cursor == node) {
    _cursor = prev;
  }

  return node;
}

void BaseBuilder::remove_nodes(BaseNode* first, BaseNode* last) noexcept {
  if (first == last) {
    remove_node(first);
    return;
  }

  if (!first->is_active()) {
    return;
  }

  BaseNode* prev = first->prev();
  BaseNode* next = last->next();

  if (_node_list._first == first) {
    _node_list._first = next;
  }
  else {
    prev->_next = next;
  }

  if (_node_list._last == last) {
    _node_list._last  = prev;
  }
  else {
    next->_prev = prev;
  }

  BaseNode* node = first;
  uint32_t did_remove_section = false;

  for (;;) {
    next = node->next();
    ASMJIT_ASSERT(next != nullptr);

    node->_prev = nullptr;
    node->_next = nullptr;
    node->_clear_flags(NodeFlags::kIsActive);
    did_remove_section |= uint32_t(node->is_section());

    if (_cursor == node) {
      _cursor = prev;
    }

    if (node == last) {
      break;
    }
    node = next;
  }

  if (did_remove_section) {
    _dirty_section_links = true;
  }
}

// BaseBuilder - Sections
// ======================

Error BaseBuilder::section_node_of(Out<SectionNode*> out, uint32_t section_id) {
  out = nullptr;

  if (ASMJIT_UNLIKELY(!_code)) {
    return make_error(Error::kNotInitialized);
  }

  if (ASMJIT_UNLIKELY(!_code->is_section_valid(section_id))) {
    return report_error(make_error(Error::kInvalidSection));
  }

  if (section_id >= _section_nodes.size()) {
    Error err = _section_nodes.reserve_grow(_builder_arena, section_id + 1);
    if (ASMJIT_UNLIKELY(err != Error::kOk)) {
      return report_error(err);
    }
  }

  SectionNode* node = nullptr;
  if (section_id < _section_nodes.size()) {
    node = _section_nodes[section_id];
  }

  if (!node) {
    ASMJIT_PROPAGATE(new_node_t<SectionNode>(Out(node), section_id));

    // We have already reserved enough space, this cannot fail now.
    if (section_id >= _section_nodes.size()) {
      // SAFETY: No need to check for error condition as we have already reserved enough space.
      (void)_section_nodes.resize_grow(_builder_arena, section_id + 1);
    }

    _section_nodes[section_id] = node;
  }

  out = node;
  return Error::kOk;
}

Error BaseBuilder::section(Section* section) {
  SectionNode* node;
  ASMJIT_PROPAGATE(section_node_of(Out(node), section->section_id()));
  ASMJIT_ASSUME(node != nullptr);

  if (!node->is_active()) {
    // Insert the section at the end if it was not part of the code.
    add_after(node, last_node());
    _cursor = node;
  }
  else {
    // This is a bit tricky. We cache section links to make sure that switching sections doesn't involve
    // traversal in linked-list unless the position of the section has changed.
    if (has_dirty_section_links()) {
      update_section_links();
    }

    if (node->_next_section) {
      _cursor = node->_next_section->_prev;
    }
    else {
      _cursor = _node_list.last();
    }
  }

  return Error::kOk;
}

void BaseBuilder::update_section_links() noexcept {
  if (!_dirty_section_links) {
    return;
  }

  BaseNode* node_ = _node_list.first();
  SectionNode* current_section = nullptr;

  while (node_) {
    if (node_->is_section()) {
      if (current_section) {
        current_section->_next_section = node_->as<SectionNode>();
      }
      current_section = node_->as<SectionNode>();
    }
    node_ = node_->next();
  }

  if (current_section) {
    current_section->_next_section = nullptr;
  }

  _dirty_section_links = false;
}

// BaseBuilder - Labels
// ====================

Error BaseBuilder::label_node_of(Out<LabelNode*> out, uint32_t label_id) {
  out = nullptr;

  if (ASMJIT_UNLIKELY(!_code)) {
    return make_error(Error::kNotInitialized);
  }

  uint32_t index = label_id;
  if (ASMJIT_UNLIKELY(index >= _code->label_count())) {
    return make_error(Error::kInvalidLabel);
  }

  if (index >= _label_nodes.size()) {
    ASMJIT_PROPAGATE(_label_nodes.resize_grow(_builder_arena, index + 1));
  }

  LabelNode* node = _label_nodes[index];
  if (!node) {
    ASMJIT_PROPAGATE(new_node_t<LabelNode>(Out(node), label_id));
    _label_nodes[index] = node;
  }

  out = node;
  return Error::kOk;
}

Error BaseBuilder::register_label_node(LabelNode* node) {
  if (ASMJIT_UNLIKELY(!_code)) {
    return make_error(Error::kNotInitialized);
  }

  uint32_t label_id;
  ASMJIT_PROPAGATE(_code->new_label_id(Out(label_id)));

  // We just added one label so it must be true.
  ASMJIT_ASSERT(_label_nodes.size() < label_id + 1);
  ASMJIT_PROPAGATE(_label_nodes.resize_grow(_builder_arena, label_id + 1));

  _label_nodes[label_id] = node;
  node->_label_id = label_id;

  return Error::kOk;
}

static Error Builder_new_label_internal(BaseBuilder* self, uint32_t label_id) {
  ASMJIT_ASSERT(self->_label_nodes.size() < label_id + 1);

  uint32_t grow_by = label_id - self->_label_nodes._size + 1u;
  Error err = self->_label_nodes.reserve_additional(self->_builder_arena, grow_by);

  if (ASMJIT_UNLIKELY(err != Error::kOk)) {
    return self->report_error(err);
  }

  LabelNode* node = nullptr;
  ASMJIT_PROPAGATE(self->new_node_t<LabelNode>(Out(node), label_id));

  while (grow_by > 1u) {
    self->_label_nodes.append_unchecked(nullptr);
    grow_by--;
  }

  self->_label_nodes.append_unchecked(node);
  node->_label_id = label_id;

  return Error::kOk;
}

Label BaseBuilder::new_label() {
  Label label;

  if (ASMJIT_LIKELY(_code)) {
    uint32_t label_id;
    Error err = _code->new_label_id(Out(label_id));

    if (ASMJIT_UNLIKELY(err != Error::kOk)) {
      report_error(err);
    }
    else {
      if (ASMJIT_LIKELY(Builder_new_label_internal(this, label_id) == Error::kOk)) {
        label.set_id(label_id);
      }
    }
  }

  return label;
}

Label BaseBuilder::new_named_label(const char* name, size_t name_size, LabelType type, uint32_t parent_id) {
  Label label;

  if (ASMJIT_LIKELY(_code)) {
    uint32_t label_id;
    Error err = _code->new_named_label_id(Out(label_id), name, name_size, type, parent_id);

    if (ASMJIT_UNLIKELY(err != Error::kOk)) {
      report_error(err);
    }
    else {
      if (ASMJIT_LIKELY(Builder_new_label_internal(this, label_id) == Error::kOk)) {
        label.set_id(label_id);
      }
    }
  }

  return label;
}

Error BaseBuilder::bind(const Label& label) {
  LabelNode* node;
  ASMJIT_PROPAGATE(label_node_of(Out(node), label));

  add_node(node);
  return Error::kOk;
}

// BaseBuilder - Passes
// ====================

ASMJIT_FAVOR_SIZE Pass* BaseBuilder::pass_by_name(const char* name) const noexcept {
  for (Pass* pass : _passes) {
    if (strcmp(pass->name(), name) == 0) {
      return pass;
    }
  }
  return nullptr;
}

ASMJIT_FAVOR_SIZE Error BaseBuilder::_add_pass(Pass* pass) noexcept {
  if (ASMJIT_UNLIKELY(!_code)) {
    return make_error(Error::kNotInitialized);
  }

  if (ASMJIT_UNLIKELY(pass == nullptr)) {
    // Since this is directly called by `add_pass()` we treat `null` argument
    // as out-of-memory condition. Otherwise it would be an API misuse.
    return make_error(Error::kOutOfMemory);
  }

  if (ASMJIT_UNLIKELY(&pass->_cb != this)) {
    return make_error(Error::kInvalidState);
  }

  ASMJIT_PROPAGATE(_passes.append(_builder_arena, pass));
  return Error::kOk;
}

Error BaseBuilder::run_passes() {
  if (ASMJIT_UNLIKELY(!_code)) {
    return make_error(Error::kNotInitialized);
  }

  if (_passes.is_empty()) {
    return Error::kOk;
  }

  ErrorHandler* prev = error_handler();
  PostponedErrorHandler postponed;

  Error err = Error::kOk;
  set_error_handler(&postponed);

  for (Pass* pass : _passes) {
    _pass_arena.reset();
    err = pass->run(_pass_arena, _logger);
    if (err != Error::kOk) {
      break;
    }
  }
  _pass_arena.reset();
  set_error_handler(prev);

  if (ASMJIT_UNLIKELY(err != Error::kOk)) {
    return report_error(err, !postponed._message.is_empty() ? postponed._message.data() : nullptr);
  }

  return Error::kOk;
}

// BaseBuilder - Emit
// ==================

Error BaseBuilder::_emit(InstId inst_id, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_* op_ext) {
  uint32_t op_count = EmitterUtils::op_count_from_emit_args(o0, o1, o2, op_ext);
  InstOptions options = inst_options() | forced_inst_options();

  if (Support::test(options, InstOptions::kReserved)) {
    if (ASMJIT_UNLIKELY(!_code)) {
      return make_error(Error::kNotInitialized);
    }

#ifndef ASMJIT_NO_INTROSPECTION
    // Strict validation.
    if (has_diagnostic_option(DiagnosticOptions::kValidateIntermediate)) {
      Operand_ op_array[Globals::kMaxOpCount];
      EmitterUtils::op_array_from_emit_args(op_array, o0, o1, o2, op_ext);

      ValidationFlags validation_flags = is_compiler() ? ValidationFlags::kEnableVirtRegs : ValidationFlags::kNone;
      Error err = _funcs.validate(BaseInst(inst_id, options, _extra_reg), op_array, op_count, validation_flags);

      if (ASMJIT_UNLIKELY(err != Error::kOk)) {
#ifndef ASMJIT_NO_LOGGING
        return EmitterUtils::log_instruction_failed(this, err, inst_id, options, o0, o1, o2, op_ext);
#else
        reset_state();
        return report_error(err);
#endif
      }
    }
#endif

    // Clear instruction options that should never be part of a regular instruction.
    options &= ~InstOptions::kReserved;
  }

  uint32_t op_capacity = InstNode::capacity_of_op_count(op_count);
  ASMJIT_ASSERT(op_capacity >= InstNode::kBaseOpCapacity);

  void* ptr = _builder_arena.alloc_oneshot(InstNode::node_size_of_op_capacity(op_capacity));
  const char* comment = inline_comment();

  reset_inst_options();
  reset_inline_comment();

  if (ASMJIT_UNLIKELY(!ptr)) {
    reset_extra_reg();
    return report_error(make_error(Error::kOutOfMemory));
  }

  InstNode* node = new(Support::PlacementNew{ptr}) InstNode(inst_id, options, op_count, op_capacity);
  node->set_extra_reg(extra_reg());
  node->set_op(0, o0);
  node->set_op(1, o1);
  node->set_op(2, o2);
  for (uint32_t i = 3; i < op_count; i++) {
    node->set_op(i, op_ext[i - 3]);
  }
  node->reset_op_range(op_count, op_capacity);

  if (comment) {
    node->set_inline_comment(static_cast<char*>(_builder_arena.dup(comment, strlen(comment), true)));
  }

  add_node(node);
  reset_extra_reg();
  return Error::kOk;
}

// BaseBuilder - Align
// ===================

Error BaseBuilder::align(AlignMode align_mode, uint32_t alignment) {
  if (ASMJIT_UNLIKELY(!_code)) {
    return make_error(Error::kNotInitialized);
  }

  AlignNode* node;
  ASMJIT_PROPAGATE(new_align_node(Out(node), align_mode, alignment));
  ASMJIT_ASSUME(node != nullptr);

  add_node(node);
  return Error::kOk;
}

// BaseBuilder - Embed
// ===================

Error BaseBuilder::embed(const void* data, size_t data_size) {
  if (ASMJIT_UNLIKELY(!_code)) {
    return make_error(Error::kNotInitialized);
  }

  EmbedDataNode* node;
  ASMJIT_PROPAGATE(new_embed_data_node(Out(node), TypeId::kUInt8, data, data_size));
  ASMJIT_ASSUME(node != nullptr);

  add_node(node);
  return Error::kOk;
}

Error BaseBuilder::embed_data_array(TypeId type_id, const void* data, size_t item_count, size_t item_repeat) {
  if (ASMJIT_UNLIKELY(!_code)) {
    return make_error(Error::kNotInitialized);
  }

  EmbedDataNode* node;
  ASMJIT_PROPAGATE(new_embed_data_node(Out(node), type_id, data, item_count, item_repeat));
  ASMJIT_ASSUME(node != nullptr);

  add_node(node);
  return Error::kOk;
}

Error BaseBuilder::embed_const_pool(const Label& label, const ConstPool& pool) {
  if (ASMJIT_UNLIKELY(!_code)) {
    return make_error(Error::kNotInitialized);
  }

  if (!is_label_valid(label)) {
    return report_error(make_error(Error::kInvalidLabel));
  }

  ASMJIT_PROPAGATE(align(AlignMode::kData, uint32_t(pool.alignment())));
  ASMJIT_PROPAGATE(bind(label));

  EmbedDataNode* node;
  ASMJIT_PROPAGATE(new_embed_data_node(Out(node), TypeId::kUInt8, nullptr, pool.size()));
  ASMJIT_ASSUME(node != nullptr);

  pool.fill(node->data());
  add_node(node);
  return Error::kOk;
}

// BaseBuilder - EmbedLabel & EmbedLabelDelta
// ==========================================

Error BaseBuilder::embed_label(const Label& label, size_t data_size) {
  if (ASMJIT_UNLIKELY(!Support::bool_and(_code, Support::is_zero_or_power_of_2_up_to(data_size, 8u)))) {
    return report_error(make_error(!_code ? Error::kNotInitialized : Error::kInvalidArgument));
  }

  EmbedLabelNode* node = nullptr;
  ASMJIT_PROPAGATE(new_node_t<EmbedLabelNode>(Out(node), label.id(), uint32_t(data_size)));

  add_node(node);
  return Error::kOk;
}

Error BaseBuilder::embed_label_delta(const Label& label, const Label& base, size_t data_size) {
  if (ASMJIT_UNLIKELY(!Support::bool_and(_code, Support::is_zero_or_power_of_2_up_to(data_size, 8u)))) {
    return report_error(make_error(!_code ? Error::kNotInitialized : Error::kInvalidArgument));
  }

  EmbedLabelDeltaNode* node = nullptr;
  ASMJIT_PROPAGATE(new_node_t<EmbedLabelDeltaNode>(Out(node), label.id(), base.id(), uint32_t(data_size)));

  add_node(node);
  return Error::kOk;
}

// BaseBuilder - Comment
// =====================

Error BaseBuilder::comment(const char* data, size_t size) {
  if (ASMJIT_UNLIKELY(!_code)) {
    return make_error(Error::kNotInitialized);
  }

  CommentNode* node;
  ASMJIT_PROPAGATE(new_comment_node(Out(node), data, size));
  ASMJIT_ASSUME(node != nullptr);

  add_node(node);
  return Error::kOk;
}

// BaseBuilder - SerializeTo
// =========================

Error BaseBuilder::serialize_to(BaseEmitter* dst) {
  Error err = Error::kOk;
  BaseNode* node_ = _node_list.first();

  Operand_ op_array[Globals::kMaxOpCount];

  do {
    dst->set_inline_comment(node_->inline_comment());

    if (node_->is_inst()) {
      InstNode* node = node_->as<InstNode>();

      // NOTE: Inlined to remove one additional call per instruction.
      dst->set_inst_options(node->options());
      dst->set_extra_reg(node->extra_reg());

      const Operand_* op = node->operands_data();
      const Operand_* op_ext = EmitterUtils::no_ext;

      size_t op_count = node->op_count();
      if (op_count > 3) {
        size_t i = 4;
        op_array[3] = op[3];

        while (i < op_count) {
          op_array[i].copy_from(op[i]);
          i++;
        }
        while (i < Globals::kMaxOpCount) {
          op_array[i].reset();
          i++;
        }
        op_ext = op_array + 3;
      }

      err = dst->_emit(node->inst_id(), op[0], op[1], op[2], op_ext);
    }
    else if (node_->is_label()) {
      if (node_->is_const_pool()) {
        ConstPoolNode* node = node_->as<ConstPoolNode>();
        err = dst->embed_const_pool(node->label(), node->const_pool());
      }
      else {
        LabelNode* node = node_->as<LabelNode>();
        err = dst->bind(node->label());
      }
    }
    else if (node_->is_align()) {
      AlignNode* node = node_->as<AlignNode>();
      err = dst->align(node->align_mode(), node->alignment());
    }
    else if (node_->is_embed_data()) {
      EmbedDataNode* node = node_->as<EmbedDataNode>();
      err = dst->embed_data_array(node->type_id(), node->data(), node->item_count(), node->repeat_count());
    }
    else if (node_->is_embed_label()) {
      EmbedLabelNode* node = node_->as<EmbedLabelNode>();
      err = dst->embed_label(node->label(), node->data_size());
    }
    else if (node_->is_embed_label_delta()) {
      EmbedLabelDeltaNode* node = node_->as<EmbedLabelDeltaNode>();
      err = dst->embed_label_delta(node->label(), node->base_babel(), node->data_size());
    }
    else if (node_->is_section()) {
      SectionNode* node = node_->as<SectionNode>();
      err = dst->section(_code->section_by_id(node->section_id()));
    }
    else if (node_->is_comment()) {
      CommentNode* node = node_->as<CommentNode>();
      err = dst->comment(node->inline_comment());
    }

    if (err != Error::kOk) {
      break;
    }
    node_ = node_->next();
  } while (node_);

  return err;
}

// BaseBuilder - Events
// ====================

static ASMJIT_INLINE void BaseBuilder_clear_all(BaseBuilder* self) noexcept {
  self->_section_nodes.reset();
  self->_label_nodes.reset();

  self->_builder_arena.reset();
  self->_pass_arena.reset();

  self->_cursor = nullptr;
  self->_node_list.reset();
}

static ASMJIT_INLINE Error BaseBuilder_init_section(BaseBuilder* self) noexcept {
  SectionNode* initial_section;

  ASMJIT_PROPAGATE(self->section_node_of(Out(initial_section), 0));
  ASMJIT_PROPAGATE(self->_passes.reserve_additional(self->_builder_arena, 4));

  ASMJIT_ASSUME(initial_section != nullptr);
  self->_cursor = initial_section;
  self->_node_list.reset(initial_section, initial_section);
  initial_section->_assign_flags(NodeFlags::kIsActive);

  return Error::kOk;
}

Error BaseBuilder::on_attach(CodeHolder& code) noexcept {
  ASMJIT_PROPAGATE(Base::on_attach(code));

  Error err = BaseBuilder_init_section(this);
  if (ASMJIT_UNLIKELY(err != Error::kOk)) {
    on_detach(code);
  }
  return err;
}

Error BaseBuilder::on_detach(CodeHolder& code) noexcept {
  BaseBuilder_delete_passes(this);
  BaseBuilder_clear_all(this);

  return Base::on_detach(code);
}

Error BaseBuilder::on_reinit(CodeHolder& code) noexcept {
  // BaseEmitter::on_reinit() never fails.
  (void)Base::on_reinit(code);

  BaseBuilder_delete_passes(this);
  BaseBuilder_clear_all(this);
  return BaseBuilder_init_section(this);
}

// Pass - Construction & Destruction
// =================================

Pass::Pass(BaseBuilder& cb, const char* name) noexcept
  : _cb(cb),
    _name(name) {}
Pass::~Pass() noexcept {}

// Pass - Interface
// ================

// [[pure virtual]]
Error Pass::run(Arena& arena, Logger* logger) {
  Support::maybe_unused(arena, logger);
  return make_error(Error::kInvalidState);
}

ASMJIT_END_NAMESPACE

#endif // !ASMJIT_NO_BUILDER
