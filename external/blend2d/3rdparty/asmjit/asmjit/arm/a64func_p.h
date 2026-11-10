// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_ARM_A64FUNC_P_H_INCLUDED
#define ASMJIT_ARM_A64FUNC_P_H_INCLUDED

#include <asmjit/core/func.h>

ASMJIT_BEGIN_SUB_NAMESPACE(a64)

//! \cond INTERNAL
//! \addtogroup asmjit_a64
//! \{

//! AArch64-specific function API (calling conventions and other utilities).
namespace FuncInternal {

//! Initialize `CallConv` structure (AArch64 specific).
Error init_call_conv(CallConv& cc, CallConvId call_conv_id, const Environment& environment) noexcept;

//! Initialize `FuncDetail` (AArch64 specific).
Error init_func_detail(FuncDetail& func, const FuncSignature& signature) noexcept;

} // {FuncInternal}

//! \}
//! \endcond

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_ARM_A64FUNC_P_H_INCLUDED
