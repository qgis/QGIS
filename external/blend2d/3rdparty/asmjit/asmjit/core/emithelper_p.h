// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_EMITHELPER_P_H_INCLUDED
#define ASMJIT_CORE_EMITHELPER_P_H_INCLUDED

#include <asmjit/core/emitter.h>
#include <asmjit/core/operand.h>
#include <asmjit/core/type.h>

ASMJIT_BEGIN_NAMESPACE

//! \cond INTERNAL
//! \addtogroup asmjit_core
//! \{

//! Helper class that provides utilities for each supported architecture.
class BaseEmitHelper {
protected:
  BaseEmitter* _emitter;

public:
  ASMJIT_INLINE_NODEBUG explicit BaseEmitHelper(BaseEmitter* emitter = nullptr) noexcept
    : _emitter(emitter) {}

  ASMJIT_INLINE_NODEBUG virtual ~BaseEmitHelper() noexcept = default;

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BaseEmitter* emitter() const noexcept { return _emitter; }

  //! Emits a pure move operation between two registers or the same type or between a register and its home
  //! slot. This function does not handle register conversion.
  virtual Error emit_reg_move(
    const Operand_& dst_,
    const Operand_& src_, TypeId type_id, const char* comment = nullptr);

  //! Emits swap between two registers.
  virtual Error emit_reg_swap(
    const Reg& a,
    const Reg& b, const char* comment = nullptr);

  //! Emits move from a function argument (either register or stack) to a register.
  //!
  //! This function can handle the necessary conversion from one argument to another, and from one register type
  //! to another, if it's possible. Any attempt of conversion that requires third register of a different group
  //! (for example conversion from K to MMX on X86/X64) will fail.
  virtual Error emit_arg_move(
    const Reg& dst_, TypeId dst_type_id,
    const Operand_& src_, TypeId src_type_id, const char* comment = nullptr);

  Error emit_args_assignment(const FuncFrame& frame, const FuncArgsAssignment& args);
};

//! \}
//! \endcond

ASMJIT_END_NAMESPACE

#endif // ASMJIT_CORE_EMITHELPER_P_H_INCLUDED
