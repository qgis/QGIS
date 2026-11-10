// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_ARM_A64INSTAPI_P_H_INCLUDED
#define ASMJIT_ARM_A64INSTAPI_P_H_INCLUDED

#include <asmjit/core/inst.h>
#include <asmjit/core/operand.h>

ASMJIT_BEGIN_SUB_NAMESPACE(a64)

//! \cond INTERNAL
//! \addtogroup asmjit_a64
//! \{

namespace InstInternal {

#ifndef ASMJIT_NO_TEXT
Error ASMJIT_CDECL inst_id_to_string(InstId inst_id, InstStringifyOptions options, String& output) noexcept;
InstId ASMJIT_CDECL string_to_inst_id(const char* s, size_t len) noexcept;
#endif // !ASMJIT_NO_TEXT

#ifndef ASMJIT_NO_INTROSPECTION
Error ASMJIT_CDECL validate(const BaseInst& inst, const Operand_* operands, size_t op_count, ValidationFlags validation_flags) noexcept;
Error ASMJIT_CDECL query_rw_info(const BaseInst& inst, const Operand_* operands, size_t op_count, InstRWInfo* out) noexcept;
Error ASMJIT_CDECL query_features(const BaseInst& inst, const Operand_* operands, size_t op_count, CpuFeatures* out) noexcept;
#endif // !ASMJIT_NO_INTROSPECTION

} // {InstInternal}

//! \}
//! \endcond

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_ARM_A64INSTAPI_P_H_INCLUDED
