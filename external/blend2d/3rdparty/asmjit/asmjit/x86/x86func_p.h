// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_X86_X86FUNC_P_H_INCLUDED
#define ASMJIT_X86_X86FUNC_P_H_INCLUDED

#include <asmjit/core/func.h>

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

//! \cond INTERNAL
//! \addtogroup asmjit_x86
//! \{

//! X86-specific function API (calling conventions and other utilities).
namespace FuncInternal {

//! Initialize `CallConv` structure (X86 specific).
Error init_call_conv(CallConv& cc, CallConvId call_conv_id, const Environment& environment) noexcept;

//! Initialize `FuncDetail` (X86 specific).
Error init_func_detail(FuncDetail& func, const FuncSignature& signature, uint32_t register_size) noexcept;

} // {FuncInternal}

//! \}
//! \endcond

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_X86_X86FUNC_P_H_INCLUDED
