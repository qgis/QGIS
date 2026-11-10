// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_ARM_ARMFORMATTER_P_H_INCLUDED
#define ASMJIT_ARM_ARMFORMATTER_P_H_INCLUDED

#include <asmjit/core/api-config.h>
#ifndef ASMJIT_NO_LOGGING

#include <asmjit/core/formatter.h>
#include <asmjit/core/string.h>
#include <asmjit/arm/armglobals.h>

ASMJIT_BEGIN_SUB_NAMESPACE(arm)

//! \cond INTERNAL
//! \addtogroup asmjit_arm
//! \{

namespace FormatterInternal {

Error ASMJIT_CDECL format_feature(
  String& sb,
  uint32_t feature_id) noexcept;

Error ASMJIT_CDECL format_cond_code(
  String& sb,
  CondCode cc) noexcept;

Error ASMJIT_CDECL format_shift_op(
  String& sb,
  ShiftOp shift_op) noexcept;

Error ASMJIT_CDECL format_register(
  String& sb,
  FormatFlags flags,
  const BaseEmitter* emitter,
  Arch arch,
  RegType reg_type,
  uint32_t reg_id,
  uint32_t element_type = 0,
  uint32_t element_index = 0xFFFFFFFF) noexcept;

Error ASMJIT_CDECL format_register_list(
  String& sb,
  FormatFlags flags,
  const BaseEmitter* emitter,
  Arch arch,
  RegType reg_type,
  uint32_t reg_mask) noexcept;

Error ASMJIT_CDECL format_operand(
  String& sb,
  FormatFlags flags,
  const BaseEmitter* emitter,
  Arch arch,
  const Operand_& op) noexcept;

} // {FormatterInternal}

//! \}
//! \endcond

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_LOGGING
#endif // ASMJIT_ARM_ARMFORMATTER_P_H_INCLUDED
