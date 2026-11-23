// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_HOST_H_INCLUDED
#define ASMJIT_HOST_H_INCLUDED

#include <asmjit/core.h>

// Detect 'X86' or 'X86_64' host architectures.
#if ASMJIT_ARCH_X86 != 0 && !defined(ASMJIT_NO_X86)

#include <asmjit/x86.h>

ASMJIT_BEGIN_NAMESPACE
namespace host { using namespace x86; }
ASMJIT_END_NAMESPACE

#endif

// Detect 'AArch64' host architecture.
#if ASMJIT_ARCH_ARM == 64 && !defined(ASMJIT_NO_AARCH64)

#include <asmjit/a64.h>

ASMJIT_BEGIN_NAMESPACE
namespace host { using namespace a64; }
ASMJIT_END_NAMESPACE

#endif

#endif // ASMJIT_HOST_H_INCLUDED
