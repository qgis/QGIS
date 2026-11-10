// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_X86_X86FORMATTER_P_H_INCLUDED
#define ASMJIT_X86_X86FORMATTER_P_H_INCLUDED

#include <asmjit/core/api-config.h>
#ifndef ASMJIT_NO_LOGGING

#include <asmjit/core/formatter.h>
#include <asmjit/core/string.h>
#include <asmjit/x86/x86globals.h>

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

//! \cond INTERNAL
//! \addtogroup asmjit_x86
//! \{

namespace FormatterInternal {

Error ASMJIT_CDECL format_feature(
  String& sb,
  uint32_t feature_id) noexcept;

Error ASMJIT_CDECL format_register(
  String& sb,
  FormatFlags flags,
  const BaseEmitter* emitter,
  Arch arch,
  RegType reg_type,
  uint32_t reg_id) noexcept;

Error ASMJIT_CDECL format_operand(
  String& sb,
  FormatFlags flags,
  const BaseEmitter* emitter,
  Arch arch,
  const Operand_& op) noexcept;

Error ASMJIT_CDECL format_instruction(
  String& sb,
  FormatFlags flags,
  const BaseEmitter* emitter,
  Arch arch,
  const BaseInst& inst, Span<const Operand_> operands) noexcept;

} // {FormatterInternal}

//! \}
//! \endcond

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_LOGGING
#endif // ASMJIT_X86_X86FORMATTER_P_H_INCLUDED
