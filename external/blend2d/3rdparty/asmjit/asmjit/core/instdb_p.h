// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_INSTDB_P_H_INCLUDED
#define ASMJIT_CORE_INSTDB_P_H_INCLUDED

#include <asmjit/core/inst.h>
#include <asmjit/core/string.h>

ASMJIT_BEGIN_NAMESPACE

//! \cond INTERNAL
//! \addtogroup asmjit_instruction_db
//! \{

struct InstNameIndex {
  struct Span {
    uint16_t start;
    uint16_t end;
  };

  Span data[26];
  uint16_t max_name_length;
};

namespace InstNameUtils {

Error decode(uint32_t name_value, InstStringifyOptions options, const char* string_table, String& output) noexcept;
InstId find_instruction(const char* s, size_t len, const uint32_t* name_table, const char* string_table, const InstNameIndex& name_index) noexcept;
uint32_t find_alias(const char* s, size_t len, const uint32_t* name_table, const char* string_table, uint32_t alias_name_count) noexcept;

} // {InstNameUtils}

//! \}
//! \endcond

ASMJIT_END_NAMESPACE

#endif // ASMJIT_CORE_INSTDB_P_H_INCLUDED
