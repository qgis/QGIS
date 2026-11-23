// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#include <asmjit/core/emitterutils_p.h>
#include <asmjit/core/errorhandler.h>
#include <asmjit/core/logger.h>
#include <asmjit/support/support.h>

ASMJIT_BEGIN_NAMESPACE

// BaseEmitter - Construction & Destruction
// ========================================

BaseEmitter::BaseEmitter(EmitterType emitter_type) noexcept
  : _emitter_type(emitter_type) {}

BaseEmitter::~BaseEmitter() noexcept {
  if (_code) {
    _add_emitter_flags(EmitterFlags::kDestroyed);
    _code->detach(this);
  }
}

// BaseEmitter - Finalize
// ======================

Error BaseEmitter::finalize() {
  // Does nothing by default, overridden by `BaseBuilder` and `BaseCompiler`.
  return Error::kOk;
}

// BaseEmitter - Internals
// =======================

static constexpr EmitterFlags kEmitterPreservedFlags = EmitterFlags::kOwnLogger | EmitterFlags::kOwnErrorHandler;

static ASMJIT_NOINLINE void BaseEmitter_updateForcedOptions(BaseEmitter* self) noexcept {
  bool emit_comments = false;
  bool has_diagnostic_options = false;

  if (self->emitter_type() == EmitterType::kAssembler) {
    // Assembler: Don't emit comments if logger is not attached.
    emit_comments = self->_code != nullptr && self->_logger != nullptr;
    has_diagnostic_options = self->has_diagnostic_option(DiagnosticOptions::kValidateAssembler);
  }
  else {
    // Builder/Compiler: Always emit comments, we cannot assume they won't be used.
    emit_comments = self->_code != nullptr;
    has_diagnostic_options = self->has_diagnostic_option(DiagnosticOptions::kValidateIntermediate);
  }

  if (emit_comments) {
    self->_add_emitter_flags(EmitterFlags::kLogComments);
  }
  else {
    self->_clear_emitter_flags(EmitterFlags::kLogComments);
  }

  // The reserved option tells emitter (Assembler/Builder/Compiler) that there may be either a border
  // case (CodeHolder not attached, for example) or that logging or validation is required.
  if (self->_code == nullptr || self->_logger || has_diagnostic_options) {
    self->_forced_inst_options |= InstOptions::kReserved;
  }
  else {
    self->_forced_inst_options &= ~InstOptions::kReserved;
  }
}

// BaseEmitter - Diagnostic Options
// ================================

void BaseEmitter::add_diagnostic_options(DiagnosticOptions options) noexcept {
  _diagnostic_options |= options;
  BaseEmitter_updateForcedOptions(this);
}

void BaseEmitter::clear_diagnostic_options(DiagnosticOptions options) noexcept {
  _diagnostic_options &= ~options;
  BaseEmitter_updateForcedOptions(this);
}

// BaseEmitter - Logging
// =====================

void BaseEmitter::set_logger(Logger* logger) noexcept {
#ifndef ASMJIT_NO_LOGGING
  if (logger) {
    _logger = logger;
    _add_emitter_flags(EmitterFlags::kOwnLogger);
  }
  else {
    _logger = nullptr;
    _clear_emitter_flags(EmitterFlags::kOwnLogger);
    if (_code) {
      _logger = _code->logger();
    }
  }
  BaseEmitter_updateForcedOptions(this);
#else
  Support::maybe_unused(logger);
#endif
}

// BaseEmitter - Error Handling
// ============================

void BaseEmitter::set_error_handler(ErrorHandler* error_handler) noexcept {
  if (error_handler) {
    _error_handler = error_handler;
    _add_emitter_flags(EmitterFlags::kOwnErrorHandler);
  }
  else {
    _error_handler = nullptr;
    _clear_emitter_flags(EmitterFlags::kOwnErrorHandler);
    if (_code) {
      _error_handler = _code->error_handler();
    }
  }
}

Error BaseEmitter::_report_error(Error err, const char* message) {
  ASMJIT_ASSERT(err != Error::kOk);

  ErrorHandler* eh = _error_handler;
  if (eh) {
    if (!message) {
      message = DebugUtils::error_as_string(err);
    }
    eh->handle_error(err, message, this);
  }

  return err;
}

// BaseEmitter - Sections
// ======================

// [[pure virtual]]
Error BaseEmitter::section(Section* section) {
  Support::maybe_unused(section);
  return make_error(Error::kInvalidState);
}

// BaseEmitter - Labels
// ====================

// [[pure virtual]]
Label BaseEmitter::new_label() {
  return Label(Globals::kInvalidId);
}

// [[pure virtual]]
Label BaseEmitter::new_named_label(const char* name, size_t name_size, LabelType type, uint32_t parent_id) {
  Support::maybe_unused(name, name_size, type, parent_id);
  return Label(Globals::kInvalidId);
}

Label BaseEmitter::label_by_name(const char* name, size_t name_size, uint32_t parent_id) noexcept {
  return Label(_code ? _code->label_id_by_name(name, name_size, parent_id) : Globals::kInvalidId);
}

// [[pure virtual]]
Error BaseEmitter::bind(const Label& label) {
  Support::maybe_unused(label);
  return make_error(Error::kInvalidState);
}

bool BaseEmitter::is_label_valid(uint32_t label_id) const noexcept {
  return _code && label_id < _code->label_count();
}

// BaseEmitter - Emit (Low-Level)
// ==============================

using EmitterUtils::no_ext;

Error BaseEmitter::_emitI(InstId inst_id) {
  return _emit(inst_id, no_ext[0], no_ext[1], no_ext[2], no_ext);
}

Error BaseEmitter::_emitI(InstId inst_id, const Operand_& o0) {
  return _emit(inst_id, o0, no_ext[1], no_ext[2], no_ext);
}

Error BaseEmitter::_emitI(InstId inst_id, const Operand_& o0, const Operand_& o1) {
  return _emit(inst_id, o0, o1, no_ext[2], no_ext);
}

Error BaseEmitter::_emitI(InstId inst_id, const Operand_& o0, const Operand_& o1, const Operand_& o2) {
  return _emit(inst_id, o0, o1, o2, no_ext);
}

Error BaseEmitter::_emitI(InstId inst_id, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3) {
  Operand_ op_ext[3] = { o3 };
  return _emit(inst_id, o0, o1, o2, op_ext);
}

Error BaseEmitter::_emitI(InstId inst_id, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3, const Operand_& o4) {
  Operand_ op_ext[3] = { o3, o4 };
  return _emit(inst_id, o0, o1, o2, op_ext);
}

Error BaseEmitter::_emitI(InstId inst_id, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3, const Operand_& o4, const Operand_& o5) {
  Operand_ op_ext[3] = { o3, o4, o5 };
  return _emit(inst_id, o0, o1, o2, op_ext);
}

// [[pure virtual]]
Error BaseEmitter::_emit(InstId inst_id, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_* op_ext) {
  Support::maybe_unused(inst_id, o0, o1, o2, op_ext);
  return make_error(Error::kInvalidState);
}

Error BaseEmitter::_emit_op_array(InstId inst_id, const Operand_* operands, size_t op_count) {
  const Operand_* op = operands;
  Operand_ op_ext[3];

  switch (op_count) {
    case 0:
      return _emit(inst_id, no_ext[0], no_ext[1], no_ext[2], no_ext);

    case 1:
      return _emit(inst_id, op[0], no_ext[1], no_ext[2], no_ext);

    case 2:
      return _emit(inst_id, op[0], op[1], no_ext[2], no_ext);

    case 3:
      return _emit(inst_id, op[0], op[1], op[2], no_ext);

    case 4:
      op_ext[0] = op[3];
      op_ext[1].reset();
      op_ext[2].reset();
      return _emit(inst_id, op[0], op[1], op[2], op_ext);

    case 5:
      op_ext[0] = op[3];
      op_ext[1] = op[4];
      op_ext[2].reset();
      return _emit(inst_id, op[0], op[1], op[2], op_ext);

    case 6:
      return _emit(inst_id, op[0], op[1], op[2], op + 3);

    default:
      return make_error(Error::kInvalidArgument);
  }
}

// BaseEmitter - Emit Utilities
// ============================

Error BaseEmitter::emit_prolog(const FuncFrame& frame) {
  if (ASMJIT_UNLIKELY(!_code))
    return make_error(Error::kNotInitialized);

  return _funcs.emit_prolog(this, frame);
}

Error BaseEmitter::emit_epilog(const FuncFrame& frame) {
  if (ASMJIT_UNLIKELY(!_code))
    return make_error(Error::kNotInitialized);

  return _funcs.emit_epilog(this, frame);
}

Error BaseEmitter::emit_args_assignment(const FuncFrame& frame, const FuncArgsAssignment& args) {
  if (ASMJIT_UNLIKELY(!_code))
    return make_error(Error::kNotInitialized);

  return _funcs.emit_args_assignment(this, frame, args);
}

// BaseEmitter - Align
// ===================

// [[pure virtual]]
Error BaseEmitter::align(AlignMode align_mode, uint32_t alignment) {
  Support::maybe_unused(align_mode, alignment);
  return make_error(Error::kInvalidState);
}

// BaseEmitter - Embed
// ===================

// [[pure virtual]]
Error BaseEmitter::embed(const void* data, size_t data_size) {
  Support::maybe_unused(data, data_size);
  return make_error(Error::kInvalidState);
}

// [[pure virtual]]
Error BaseEmitter::embed_data_array(TypeId type_id, const void* data, size_t item_count, size_t repeat_count) {
  Support::maybe_unused(type_id, data, item_count, repeat_count);
  return make_error(Error::kInvalidState);
}

// [[pure virtual]]
Error BaseEmitter::embed_const_pool(const Label& label, const ConstPool& pool) {
  Support::maybe_unused(label, pool);
  return make_error(Error::kInvalidState);
}

// [[pure virtual]]
Error BaseEmitter::embed_label(const Label& label, size_t data_size) {
  Support::maybe_unused(label, data_size);
  return make_error(Error::kInvalidState);
}

// [[pure virtual]]
Error BaseEmitter::embed_label_delta(const Label& label, const Label& base, size_t data_size) {
  Support::maybe_unused(label, base, data_size);
  return make_error(Error::kInvalidState);
}

// BaseEmitter - Comment
// =====================

// [[pure virtual]]
Error BaseEmitter::comment(const char* data, size_t size) {
  Support::maybe_unused(data, size);
  return make_error(Error::kInvalidState);
}

Error BaseEmitter::commentf(const char* fmt, ...) {
  if (!has_emitter_flag(EmitterFlags::kLogComments)) {
    if (!has_emitter_flag(EmitterFlags::kAttached)) {
      return report_error(make_error(Error::kNotInitialized));
    }
    return Error::kOk;
  }

#ifndef ASMJIT_NO_LOGGING
  StringTmp<1024> sb;

  va_list ap;
  va_start(ap, fmt);
  Error err = sb.append_vformat(fmt, ap);
  va_end(ap);

  ASMJIT_PROPAGATE(err);
  return comment(sb.data(), sb.size());
#else
  Support::maybe_unused(fmt);
  return Error::kOk;
#endif
}

Error BaseEmitter::commentv(const char* fmt, va_list ap) {
  if (!has_emitter_flag(EmitterFlags::kLogComments)) {
    if (!has_emitter_flag(EmitterFlags::kAttached)) {
      return report_error(make_error(Error::kNotInitialized));
    }
    return Error::kOk;
  }

#ifndef ASMJIT_NO_LOGGING
  StringTmp<1024> sb;
  Error err = sb.append_vformat(fmt, ap);

  ASMJIT_PROPAGATE(err);
  return comment(sb.data(), sb.size());
#else
  Support::maybe_unused(fmt, ap);
  return Error::kOk;
#endif
}

// BaseEmitter - Events
// ====================

Error BaseEmitter::on_attach(CodeHolder& code) noexcept {
  _code = &code;
  _environment = code.environment();
  _add_emitter_flags(EmitterFlags::kAttached);

  _gp_signature.set_bits(
    Environment::is_32bit(code.arch())
      ? RegTraits<RegType::kGp32>::kSignature
      : RegTraits<RegType::kGp64>::kSignature
  );

  on_settings_updated();
  return Error::kOk;
}

Error BaseEmitter::on_detach(CodeHolder& code) noexcept {
  Support::maybe_unused(code);

  if (!has_own_logger()) {
    _logger = nullptr;
  }

  if (!has_own_error_handler()) {
    _error_handler = nullptr;
  }

  _clear_emitter_flags(~kEmitterPreservedFlags);
  _instruction_alignment = uint8_t(0);
  _forced_inst_options = InstOptions::kReserved;
  _private_data = 0;

  _environment.reset();
  _gp_signature.reset();

  _inst_options = InstOptions::kNone;
  _extra_reg.reset();
  _inline_comment = nullptr;

  return Error::kOk;
}

Error BaseEmitter::on_reinit(CodeHolder& code) noexcept {
  ASMJIT_ASSERT(_code == &code);
  Support::maybe_unused(code);

  _inst_options = InstOptions::kNone;
  _extra_reg.reset();
  _inline_comment = nullptr;

  return Error::kOk;
}

void BaseEmitter::on_settings_updated() noexcept {
  // Only called when attached to CodeHolder by CodeHolder.
  ASMJIT_ASSERT(_code != nullptr);

  if (!has_own_logger()) {
    _logger = _code->logger();
  }

  if (!has_own_error_handler()) {
    _error_handler = _code->error_handler();
  }

  BaseEmitter_updateForcedOptions(this);
}

ASMJIT_END_NAMESPACE
