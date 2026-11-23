// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_FORMATTER_P_H_INCLUDED
#define ASMJIT_CORE_FORMATTER_P_H_INCLUDED

#include <asmjit/core/compilerdefs.h>
#include <asmjit/core/formatter.h>
#include <asmjit/core/operand.h>

ASMJIT_BEGIN_NAMESPACE

//! \cond INTERNAL
//! \addtogroup asmjit_logging
//! \{

namespace Formatter {

[[maybe_unused]]
static ASMJIT_INLINE size_t padding_from_options(const FormatOptions& format_options, FormatPaddingGroup group) noexcept {
  static constexpr uint16_t default_padding_table[uint32_t(FormatPaddingGroup::kMaxValue) + 1] = { 44, 26 };
  static_assert(uint32_t(FormatPaddingGroup::kMaxValue) + 1 == 2, "If a new group is defined it must be added here");

  size_t padding = format_options.padding(group);
  return padding ? padding : size_t(default_padding_table[uint32_t(group)]);
}

Error format_virt_reg_name(String& sb, const VirtReg* v_reg) noexcept;
Error format_virt_reg_name_with_prefix(String& sb, const char* prefix, size_t prefix_size, const VirtReg* v_reg) noexcept;

} // {Formatter}

//! \}
//! \endcond

ASMJIT_END_NAMESPACE

#endif // ASMJIT_CORE_FORMATTER_H_P_INCLUDED
