// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_UJIT_UNICOMPILER_UTILS_P_H_INCLUDED
#define ASMJIT_UJIT_UNICOMPILER_UTILS_P_H_INCLUDED

#include <asmjit/ujit/ujitbase.h>

#if !defined(ASMJIT_NO_UJIT)

#include <asmjit/ujit/uniop.h>

ASMJIT_BEGIN_SUB_NAMESPACE(ujit)

//! \addtogroup asmjit_ujit
//! \{

template<typename UniOpDst, typename UniOpSrc>
static ASMJIT_INLINE UniOpDst translate_op(UniOpSrc op, UniOpSrc begin, UniOpDst target) noexcept {
  ASMJIT_ASSERT(begin <= op);
  uint32_t offset = uint32_t(op) - uint32_t(begin);
  return UniOpDst(uint32_t(target) + offset);
}

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_UJIT
#endif // ASMJIT_UJIT_UNICOMPILER_UTILS_P_H_INCLUDED
