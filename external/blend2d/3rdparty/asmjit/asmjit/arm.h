// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_ARM_H_INCLUDED
#define ASMJIT_ARM_H_INCLUDED

//! \addtogroup asmjit_arm
//!
//! ### Namespaces
//!
//!   - \ref arm - arm namespace provides common functionality for both AArch32 and AArch64 backends.
//!     \ref arm namespace, so you can only use a single namespace when targeting AArch32 architecture.
//!   - \ref a64 - a64 namespace provides support for AArch64 architecture. In addition it includes
//!     \ref arm namespace, so you can only use a single namespace when targeting AArch64 architecture.
//!
//! ### Emitters
//!
//!   - AArch64:
//!     - \ref a64::Assembler - AArch64 assembler (must read, provides examples).
//!     - \ref a64::Builder - AArch64 builder.
//!     - \ref a64::Compiler - AArch64 compiler.
//!     - \ref a64::Emitter - AArch64 emitter (abstract).
//!
//! ### Supported Instructions
//!
//!   - AArch64:
//!     - Emitters:
//!       - \ref a64::EmitterExplicitT - Provides all instructions that use explicit operands, provides also
//!         utility functions. The member functions provided are part of all AArch64 emitters.
//!     - Instruction representation:
//!       - \ref a64::Inst::Id - instruction identifiers.
//!
//! ### ARM Operands
//!
//!   - AArch64:
//!     - \ref a64::Gp - 32-bit or 64-bit general purpose register used by AArch64:
//!     - \ref a64::Vec - Vector (SIMD) register.
//!     - \ref a64::Mem - AArch64 memory operand that provides support for all AArch64 addressing features
//!       including base, index, pre/post increment, and AArch64 specific shift/extend of memory index.
//!
//! ### Memory Operands
//!
//! ### Other
//!
//!   - \ref arm::Shift - Shift operation and value (both AArch32 and AArch64).
//!   - \ref arm::Utils - Utilities that can help during code generation for AArch32 and AArch64.

#include <asmjit/core.h>

#include <asmjit/asmjit-scope-begin.h>
#include <asmjit/arm/armglobals.h>
#include <asmjit/arm/armutils.h>
#include <asmjit/asmjit-scope-end.h>

#endif // ASMJIT_ARM_H_INCLUDED
