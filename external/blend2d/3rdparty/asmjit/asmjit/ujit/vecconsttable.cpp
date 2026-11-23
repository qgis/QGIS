// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#include <asmjit/ujit/vecconsttable.h>

#if !defined(ASMJIT_NO_UJIT)

ASMJIT_BEGIN_SUB_NAMESPACE(ujit)

// REFACTOR [C++20]: Use 'constinit' to make sure the constant is initialized at compile-time.
static constexpr VecConstTable vec_const_table_;

const VecConstTable vec_const_table = vec_const_table_;

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_UJIT
