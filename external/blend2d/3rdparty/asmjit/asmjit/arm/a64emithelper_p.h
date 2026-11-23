// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_ARM_A64EMITHELPER_P_H_INCLUDED
#define ASMJIT_ARM_A64EMITHELPER_P_H_INCLUDED

#include <asmjit/core/api-config.h>

#include <asmjit/core/emithelper_p.h>
#include <asmjit/core/func.h>
#include <asmjit/arm/a64emitter.h>
#include <asmjit/arm/a64operand.h>

ASMJIT_BEGIN_SUB_NAMESPACE(a64)

//! \cond INTERNAL
//! \addtogroup asmjit_a64
//! \{

class EmitHelper : public BaseEmitHelper {
public:
  ASMJIT_INLINE_NODEBUG explicit EmitHelper(BaseEmitter* emitter = nullptr) noexcept
    : BaseEmitHelper(emitter) {}

  ASMJIT_INLINE_NODEBUG ~EmitHelper() noexcept override = default;

  ASMJIT_INLINE void reset(BaseEmitter* emitter) noexcept {
    _emitter = emitter;
  }

  Error emit_reg_move(
    const Operand_& dst_,
    const Operand_& src_, TypeId type_id, const char* comment = nullptr) override;

  Error emit_reg_swap(
    const Reg& a,
    const Reg& b, const char* comment = nullptr) override;

  Error emit_arg_move(
    const Reg& dst_, TypeId dst_type_id,
    const Operand_& src_, TypeId src_type_id, const char* comment = nullptr) override;

  Error emit_prolog(const FuncFrame& frame);
  Error emit_epilog(const FuncFrame& frame);
};

void init_emitter_funcs(BaseEmitter* emitter);

[[maybe_unused]]
static inline void update_emitter_funcs(BaseEmitter* emitter) noexcept { Support::maybe_unused(emitter); }

//! \}
//! \endcond

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_ARM_A64EMITHELPER_P_H_INCLUDED
