// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#include <asmjit/core/assembler.h>
#include <asmjit/core/codewriter_p.h>
#include <asmjit/core/constpool.h>
#include <asmjit/core/emitterutils_p.h>
#include <asmjit/core/formatter.h>
#include <asmjit/core/logger.h>
#include <asmjit/support/support.h>

ASMJIT_BEGIN_NAMESPACE

// BaseAssembler - Construction & Destruction
// ==========================================

BaseAssembler::BaseAssembler() noexcept
  : BaseEmitter(EmitterType::kAssembler) {}

BaseAssembler::~BaseAssembler() noexcept {}

// BaseAssembler - Buffer Management
// =================================

Error BaseAssembler::set_offset(size_t offset) {
  if (ASMJIT_UNLIKELY(!_code)) {
    return report_error(make_error(Error::kNotInitialized));
  }

  size_t size = Support::max<size_t>(_section->buffer_size(), this->offset());
  if (ASMJIT_UNLIKELY(offset > size)) {
    return report_error(make_error(Error::kInvalidArgument));
  }

  _buffer_ptr = _buffer_data + offset;
  return Error::kOk;
}

// BaseAssembler - Section Management
// ==================================

static ASMJIT_INLINE Error BaseAssembler_initSection(BaseAssembler* self, Section* section) noexcept {
  uint8_t* p = section->_buffer._data;

  self->_section = section;
  self->_buffer_data = p;
  self->_buffer_ptr  = p + section->_buffer._size;
  self->_buffer_end  = p + section->_buffer._capacity;

  return Error::kOk;
}

Error BaseAssembler::section(Section* section) {
  if (ASMJIT_UNLIKELY(!_code)) {
    return report_error(make_error(Error::kNotInitialized));
  }

  if (!_code->is_section_valid(section->section_id()) || _code->_sections[section->section_id()] != section) {
    return report_error(make_error(Error::kInvalidSection));
  }

#ifndef ASMJIT_NO_LOGGING
  if (_logger) {
    _logger->logf(".section %s {#%u}\n", section->name(), section->section_id());
  }
#endif

  return BaseAssembler_initSection(this, section);
}

// BaseAssembler - Label Management
// ================================

Label BaseAssembler::new_label() {
  Label label;

  if (ASMJIT_LIKELY(_code)) {
    Error err = _code->new_label_id(Out(label._base_id));
    if (ASMJIT_UNLIKELY(err != Error::kOk)) {
      report_error(err);
    }
  }

  return label;
}

Label BaseAssembler::new_named_label(const char* name, size_t name_size, LabelType type, uint32_t parent_id) {
  Label label;

  if (ASMJIT_LIKELY(_code)) {
    uint32_t label_id;
    Error err = _code->new_named_label_id(Out(label_id), name, name_size, type, parent_id);
    if (ASMJIT_UNLIKELY(err != Error::kOk)) {
      report_error(err);
    }
    else {
      label.set_id(label_id);
    }
  }

  return label;
}

Error BaseAssembler::bind(const Label& label) {
  if (ASMJIT_UNLIKELY(!_code)) {
    return report_error(make_error(Error::kNotInitialized));
  }

  Error err = _code->bind_label(label, _section->section_id(), offset());

#ifndef ASMJIT_NO_LOGGING
  if (_logger) {
    EmitterUtils::log_label_bound(this, label);
  }
#endif

  reset_inline_comment();
  if (err != Error::kOk) {
    return report_error(err);
  }

  return Error::kOk;
}

// BaseAssembler - Embed
// =====================

Error BaseAssembler::embed(const void* data, size_t data_size) {
  if (ASMJIT_UNLIKELY(!_code)) {
    return report_error(make_error(Error::kNotInitialized));
  }

  if (data_size == 0) {
    return Error::kOk;
  }

  CodeWriter writer(this);
  ASMJIT_PROPAGATE(writer.ensure_space(this, data_size));

  writer.emit_data(data, data_size);
  writer.done(this);

#ifndef ASMJIT_NO_LOGGING
  if (_logger) {
    StringTmp<512> sb;
    Formatter::format_data(sb, _logger->flags(), arch(), TypeId::kUInt8, data, data_size, 1);
    sb.append('\n');
    _logger->log(sb);
  }
#endif

  return Error::kOk;
}

Error BaseAssembler::embed_data_array(TypeId type_id, const void* data, size_t item_count, size_t repeat_count) {
  uint32_t deabstract_delta = TypeUtils::deabstract_delta_of_size(register_size());
  TypeId final_type_id = TypeUtils::deabstract(type_id, deabstract_delta);

  if (ASMJIT_UNLIKELY(!TypeUtils::is_valid(final_type_id))) {
    return report_error(make_error(Error::kInvalidArgument));
  }

  if (item_count == 0 || repeat_count == 0) {
    return Error::kOk;
  }

  uint32_t type_size = TypeUtils::size_of(final_type_id);
  Support::FastUInt8 of = 0;

  size_t data_size = Support::mul_overflow(item_count, size_t(type_size), &of);
  size_t total_size = Support::mul_overflow(data_size, repeat_count, &of);

  if (ASMJIT_UNLIKELY(of)) {
    return report_error(make_error(Error::kOutOfMemory));
  }

  CodeWriter writer(this);
  ASMJIT_PROPAGATE(writer.ensure_space(this, total_size));

  for (size_t i = 0; i < repeat_count; i++) {
    writer.emit_data(data, data_size);
  }
  writer.done(this);

#ifndef ASMJIT_NO_LOGGING
  if (_logger) {
    StringTmp<512> sb;
    Formatter::format_data(sb, _logger->flags(), arch(), type_id, data, item_count, repeat_count);
    sb.append('\n');
    _logger->log(sb);
  }
#endif

  return Error::kOk;
}

#ifndef ASMJIT_NO_LOGGING
static const TypeId data_type_id_by_size_table[9] = {
  TypeId::kVoid,   // [0] (invalid)
  TypeId::kUInt8,  // [1] (uint8_t)
  TypeId::kUInt16, // [2] (uint16_t)
  TypeId::kVoid,   // [3] (invalid)
  TypeId::kUInt32, // [4] (uint32_t)
  TypeId::kVoid,   // [5] (invalid)
  TypeId::kVoid,   // [6] (invalid)
  TypeId::kVoid,   // [7] (invalid)
  TypeId::kUInt64  // [8] (uint64_t)
};
#endif

Error BaseAssembler::embed_const_pool(const Label& label, const ConstPool& pool) {
  if (ASMJIT_UNLIKELY(!_code)) {
    return report_error(make_error(Error::kNotInitialized));
  }

  if (ASMJIT_UNLIKELY(!is_label_valid(label))) {
    return report_error(make_error(Error::kInvalidLabel));
  }

  ASMJIT_PROPAGATE(align(AlignMode::kData, uint32_t(pool.alignment())));
  ASMJIT_PROPAGATE(bind(label));

  size_t size = pool.size();
  if (!size) {
    return Error::kOk;
  }

  CodeWriter writer(this);
  ASMJIT_PROPAGATE(writer.ensure_space(this, size));

#ifndef ASMJIT_NO_LOGGING
  uint8_t* data = writer.cursor();
#endif

  pool.fill(writer.cursor());
  writer.advance(size);
  writer.done(this);

#ifndef ASMJIT_NO_LOGGING
  if (_logger) {
    uint32_t data_size_log2 = Support::min<uint32_t>(Support::ctz(pool.min_item_size()), 3);
    uint32_t data_size = 1 << data_size_log2;

    StringTmp<512> sb;
    Formatter::format_data(sb, _logger->flags(), arch(), data_type_id_by_size_table[data_size], data, size >> data_size_log2);
    sb.append('\n');
    _logger->log(sb);
  }
#endif

  return Error::kOk;
}

Error BaseAssembler::embed_label(const Label& label, size_t data_size) {
  if (ASMJIT_UNLIKELY(!_code)) {
    return report_error(make_error(Error::kNotInitialized));
  }

  if (ASMJIT_UNLIKELY(is_label_valid(label))) {
    return report_error(make_error(Error::kInvalidLabel));
  }

  RelocEntry* re;
  LabelEntry& le = _code->label_entry_of(label);

  if (data_size == 0) {
    data_size = register_size();
  }

  if (ASMJIT_UNLIKELY(!Support::is_power_of_2_up_to(data_size, 8u))) {
    return report_error(make_error(Error::kInvalidOperandSize));
  }

  CodeWriter writer(this);
  ASMJIT_PROPAGATE(writer.ensure_space(this, data_size));

#ifndef ASMJIT_NO_LOGGING
  if (_logger) {
    StringTmp<256> sb;
    sb.append('.');
    Formatter::format_data_type(sb, _logger->flags(), arch(), data_type_id_by_size_table[data_size]);
    sb.append(' ');
    Formatter::format_label(sb, FormatFlags::kNone, this, label.id());
    sb.append('\n');
    _logger->log(sb);
  }
#endif

  Error err = _code->new_reloc_entry(Out(re), RelocType::kRelToAbs);
  if (ASMJIT_UNLIKELY(err != Error::kOk)) {
    return report_error(err);
  }

  re->_source_section_id = _section->section_id();
  re->_source_offset = offset();
  re->_format.reset_to_simple_value(OffsetType::kUnsignedOffset, data_size);

  if (le.is_bound()) {
    re->_target_section_id = le.section_id();
    re->_payload = le.offset();
  }
  else {
    OffsetFormat of;
    of.reset_to_simple_value(OffsetType::kUnsignedOffset, data_size);

    Fixup* fixup = _code->new_fixup(le, _section->section_id(), offset(), 0, of);
    if (ASMJIT_UNLIKELY(!fixup)) {
      return report_error(make_error(Error::kOutOfMemory));
    }

    fixup->label_or_reloc_id = re->id();
  }

  // Emit dummy DWORD/QWORD depending on the data size.
  writer.emit_zeros(data_size);
  writer.done(this);

  return Error::kOk;
}

Error BaseAssembler::embed_label_delta(const Label& label, const Label& base, size_t data_size) {
  if (ASMJIT_UNLIKELY(!_code)) {
    return report_error(make_error(Error::kNotInitialized));
  }

  if (ASMJIT_UNLIKELY(!Support::bool_and(_code->is_label_valid(label), _code->is_label_valid(base)))) {
    return report_error(make_error(Error::kInvalidLabel));
  }

  LabelEntry& label_entry = _code->label_entry_of(label);
  LabelEntry& base_entry = _code->label_entry_of(base);

  if (data_size == 0) {
    data_size = register_size();
  }

  if (ASMJIT_UNLIKELY(!Support::is_power_of_2_up_to(data_size, 8u))) {
    return report_error(make_error(Error::kInvalidOperandSize));
  }

  CodeWriter writer(this);
  ASMJIT_PROPAGATE(writer.ensure_space(this, data_size));

#ifndef ASMJIT_NO_LOGGING
  if (_logger) {
    StringTmp<256> sb;
    sb.append('.');
    Formatter::format_data_type(sb, _logger->flags(), arch(), data_type_id_by_size_table[data_size]);
    sb.append(" (");
    Formatter::format_label(sb, FormatFlags::kNone, this, label.id());
    sb.append(" - ");
    Formatter::format_label(sb, FormatFlags::kNone, this, base.id());
    sb.append(")\n");
    _logger->log(sb);
  }
#endif

  // If both labels are bound within the same section it means the delta can be calculated now.
  if (label_entry.is_bound() && base_entry.is_bound() && label_entry.section_id() == base_entry.section_id()) {
    uint64_t delta = label_entry.offset() - base_entry.offset();
    writer.emit_value_le(delta, data_size);
  }
  else {
    RelocEntry* re;
    Error err = _code->new_reloc_entry(Out(re), RelocType::kExpression);
    if (ASMJIT_UNLIKELY(err != Error::kOk)) {
      return report_error(err);
    }

    Expression* exp = _code->_arena.new_oneshot<Expression>();
    if (ASMJIT_UNLIKELY(!exp)) {
      return report_error(make_error(Error::kOutOfMemory));
    }

    exp->reset();
    exp->op_type = ExpressionOpType::kSub;
    exp->set_value_as_label_id(0, label.id());
    exp->set_value_as_label_id(1, base.id());

    re->_format.reset_to_simple_value(OffsetType::kSignedOffset, data_size);
    re->_source_section_id = _section->section_id();
    re->_source_offset = offset();
    re->_payload = (uint64_t)(uintptr_t)exp;

    writer.emit_zeros(data_size);
  }

  writer.done(this);
  return Error::kOk;
}

// BaseAssembler - Comment
// =======================

Error BaseAssembler::comment(const char* data, size_t size) {
  if (!has_emitter_flag(EmitterFlags::kLogComments)) {
    if (!has_emitter_flag(EmitterFlags::kAttached)) {
      return report_error(make_error(Error::kNotInitialized));
    }
    return Error::kOk;
  }

#ifndef ASMJIT_NO_LOGGING
  // Logger cannot be NULL if `EmitterFlags::kLogComments` is set.
  ASMJIT_ASSERT(_logger != nullptr);

  _logger->log(data, size);
  _logger->log("\n", 1);
  return Error::kOk;
#else
  Support::maybe_unused(data, size);
  return Error::kOk;
#endif
}

// BaseAssembler - Events
// ======================

Error BaseAssembler::on_attach(CodeHolder& code) noexcept {
  ASMJIT_PROPAGATE(Base::on_attach(code));

  // Attach to the end of the .text section.
  return BaseAssembler_initSection(this, code._sections[0]);
}

Error BaseAssembler::on_detach(CodeHolder& code) noexcept {
  _section    = nullptr;
  _buffer_data = nullptr;
  _buffer_end  = nullptr;
  _buffer_ptr  = nullptr;
  return Base::on_detach(code);
}

Error BaseAssembler::on_reinit(CodeHolder& code) noexcept {
  // BaseEmitter::on_reinit() never fails.
  (void)Base::on_reinit(code);

  return BaseAssembler_initSection(this, code._sections[0]);
}

ASMJIT_END_NAMESPACE
