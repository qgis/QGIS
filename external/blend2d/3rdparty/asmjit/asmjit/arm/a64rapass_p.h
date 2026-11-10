// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_ARM_A64RAPASS_P_H_INCLUDED
#define ASMJIT_ARM_A64RAPASS_P_H_INCLUDED

#include <asmjit/core/api-config.h>
#ifndef ASMJIT_NO_COMPILER

#include <asmjit/core/compiler.h>
#include <asmjit/core/racfgblock_p.h>
#include <asmjit/core/racfgbuilder_p.h>
#include <asmjit/core/rapass_p.h>
#include <asmjit/arm/a64assembler.h>
#include <asmjit/arm/a64compiler.h>
#include <asmjit/arm/a64emithelper_p.h>

ASMJIT_BEGIN_SUB_NAMESPACE(a64)

//! \cond INTERNAL
//! \addtogroup asmjit_a64
//! \{

//! ARM register allocation pass.
//!
//! Takes care of generating function prologs and epilogs, and also performs
//! register allocation.
class ARMRAPass : public BaseRAPass {
public:
  ASMJIT_NONCOPYABLE(ARMRAPass)
  using Base = BaseRAPass;

  //! \name Members
  //! \{

  EmitHelper _emit_helper;

  //! \}

  //! \name Construction & Destruction
  //! \{

  ARMRAPass(BaseCompiler& cc) noexcept;
  ~ARMRAPass() noexcept override;

  //! \}

  //! \name Accessors
  //! \{

  //! Returns the compiler casted to `arm::Compiler`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Compiler& cc() const noexcept { return static_cast<Compiler&>(_cb); }

  //! Returns emit helper.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG EmitHelper* emit_helper() noexcept { return &_emit_helper; }

  //! \}

  //! \name Events
  //! \{

  void on_init() noexcept override;
  void on_done() noexcept override;

  //! \}

  //! \name CFG
  //! \{

  Error build_cfg_nodes() noexcept override;

  //! \}

  //! \name Rewrite
  //! \{

  Error rewrite() noexcept override;

  //! \}

  //! \name Prolog & Epilog
  //! \{

  Error update_stack_frame() noexcept override;

  //! \}

  //! \name Emit Helpers
  //! \{

  Error emit_move(RAWorkReg* work_reg, uint32_t dst_phys_id, uint32_t src_phys_id) noexcept override;
  Error emit_swap(RAWorkReg* a_reg, uint32_t a_phys_id, RAWorkReg* b_reg, uint32_t b_phys_id) noexcept override;

  Error emit_load(RAWorkReg* work_reg, uint32_t dst_phys_id) noexcept override;
  Error emit_save(RAWorkReg* work_reg, uint32_t src_phys_id) noexcept override;

  Error emit_jump(const Label& label) noexcept override;
  Error emit_pre_call(InvokeNode* invoke_node) noexcept override;

  //! \}
};

//! \}
//! \endcond

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_COMPILER
#endif // ASMJIT_ARM_A64RAPASS_P_H_INCLUDED
