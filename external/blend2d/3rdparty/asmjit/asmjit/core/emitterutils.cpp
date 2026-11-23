// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#include <asmjit/core/assembler.h>
#include <asmjit/core/emitterutils_p.h>
#include <asmjit/core/formatter_p.h>
#include <asmjit/core/logger.h>
#include <asmjit/support/support.h>

ASMJIT_BEGIN_NAMESPACE

namespace EmitterUtils {

#ifndef ASMJIT_NO_LOGGING

Error finish_formatted_line(String& sb, const FormatOptions& format_options, const uint8_t* bin_data, size_t bin_size, size_t offset_size, size_t imm_size, const char* comment) noexcept {
  ASMJIT_ASSERT(bin_size >= offset_size);
  const size_t kNoBinSize = SIZE_MAX;

  size_t comment_size = comment ? Support::str_nlen(comment, Globals::kMaxCommentSize) : 0;

  if ((bin_size != 0 && bin_size != kNoBinSize) || comment_size) {
    char sep = ';';
    size_t padding = Formatter::padding_from_options(format_options, FormatPaddingGroup::kRegularLine);

    for (size_t i = (bin_size == kNoBinSize); i < 2; i++) {
      ASMJIT_PROPAGATE(sb.pad_end(padding));

      if (sep) {
        ASMJIT_PROPAGATE(sb.append(sep));
        ASMJIT_PROPAGATE(sb.append(' '));
      }

      // Append binary data or comment.
      if (i == 0) {
        ASMJIT_PROPAGATE(sb.append_hex(bin_data, bin_size - offset_size - imm_size));
        ASMJIT_PROPAGATE(sb.append_chars('.', offset_size * 2));
        ASMJIT_PROPAGATE(sb.append_hex(bin_data + bin_size - imm_size, imm_size));
        if (comment_size == 0) break;
      }
      else {
        ASMJIT_PROPAGATE(sb.append(comment, comment_size));
      }

      sep = '|';
      padding += Formatter::padding_from_options(format_options, FormatPaddingGroup::kMachineCode);
    }
  }

  return sb.append('\n');
}

void log_label_bound(BaseAssembler* self, const Label& label) noexcept {
  Logger* logger = self->logger();

  StringTmp<512> sb;
  size_t bin_size = logger->has_flag(FormatFlags::kMachineCode) ? size_t(0) : SIZE_MAX;

  sb.append_chars(' ', logger->indentation(FormatIndentationGroup::kLabel));
  Formatter::format_label(sb, logger->flags(), self, label.id());
  sb.append(':');
  finish_formatted_line(sb, logger->options(), nullptr, bin_size, 0, 0, self->_inline_comment);
  logger->log(sb.data(), sb.size());
}

void log_instruction_emitted(
  BaseAssembler* self,
  InstId inst_id,
  InstOptions options,
  const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_* op_ext,
  uint32_t rel_size, uint32_t imm_size, uint8_t* after_cursor) {

  Logger* logger = self->logger();
  ASMJIT_ASSERT(logger != nullptr);

  StringTmp<256> sb;
  FormatFlags format_flags = logger->flags();

  uint8_t* before_cursor = self->buffer_ptr();
  intptr_t emitted_size = (intptr_t)(after_cursor - before_cursor);

  Operand_ op_array[Globals::kMaxOpCount];
  op_array_from_emit_args(op_array, o0, o1, o2, op_ext);

  sb.append_chars(' ', logger->indentation(FormatIndentationGroup::kCode));
  self->_funcs.format_instruction(sb, format_flags, self, self->arch(), BaseInst(inst_id, options, self->extra_reg()), Span<Operand_>(op_array, Globals::kMaxOpCount));

  if (Support::test(format_flags, FormatFlags::kMachineCode)) {
    finish_formatted_line(sb, logger->options(), self->buffer_ptr(), size_t(emitted_size), rel_size, imm_size, self->inline_comment());
  }
  else {
    finish_formatted_line(sb, logger->options(), nullptr, SIZE_MAX, 0, 0, self->inline_comment());
  }
  logger->log(sb);
}

Error log_instruction_failed(
  BaseEmitter* self,
  Error err,
  InstId inst_id,
  InstOptions options,
  const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_* op_ext) {

  StringTmp<256> sb;
  sb.append(DebugUtils::error_as_string(err));
  sb.append(": ");

  Operand_ op_array[Globals::kMaxOpCount];
  op_array_from_emit_args(op_array, o0, o1, o2, op_ext);

  self->_funcs.format_instruction(sb, FormatFlags::kRegType, self, self->arch(), BaseInst(inst_id, options, self->extra_reg()), Span<Operand_>(op_array, Globals::kMaxOpCount));

  if (self->inline_comment()) {
    sb.append(" ; ");
    sb.append(self->inline_comment());
  }

  self->reset_state();
  return self->report_error(err, sb.data());
}

#endif

} // {EmitterUtils}

ASMJIT_END_NAMESPACE
