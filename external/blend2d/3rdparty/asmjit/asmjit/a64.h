// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_A64_H_INCLUDED
#define ASMJIT_A64_H_INCLUDED

//! \addtogroup asmjit_a64
//!
//! ### Emitters
//!
//!   - \ref a64::Assembler - AArch64 assembler (must read, provides examples).
//!   - \ref a64::Builder - AArch64 builder.
//!   - \ref a64::Compiler - AArch64 compiler.
//!   - \ref a64::Emitter - AArch64 emitter (abstract).
//!
//! ### Supported Instructions
//!
//!   - Emitters:
//!     - \ref a64::EmitterExplicitT - Provides all instructions that use explicit operands, provides also utility
//!       functions. The member functions provided are part of all AArch64 emitters.
//!
//!   - Instruction representation:
//!     - \ref a64::Inst::Id - instruction identifiers.
//!
//! ### Register Operands
//!
//!   - \ref a64::Gp - General purpose register (abstracts 32-bit and 64-bit general purpose registers).
//!   - \ref a64::Vec - Vector register (abstracts B, H, S, D, and Q NEON register with possible element type and index).
//!
//! ### Memory Operands
//!
//!   - \ref a64::Mem - AArch64 memory operand that provides support for all ARM addressing features including base,
//!     index, pre/post increment, and ARM-specific shift addressing + index extending.
//!
//! ### Other
//!
//!   - \ref arm::Shift - Shift operation and value.
//!   - \ref arm::Utils - Utilities that can help during code generation for AArch32 and AArch64.

#include <asmjit/arm.h>

#include <asmjit/asmjit-scope-begin.h>
#include <asmjit/arm/a64assembler.h>
#include <asmjit/arm/a64builder.h>
#include <asmjit/arm/a64compiler.h>
#include <asmjit/arm/a64emitter.h>
#include <asmjit/arm/a64globals.h>
#include <asmjit/arm/a64instdb.h>
#include <asmjit/arm/a64operand.h>
#include <asmjit/asmjit-scope-end.h>

#endif // ASMJIT_A64_H_INCLUDED

