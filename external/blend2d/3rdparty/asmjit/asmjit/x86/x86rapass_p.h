// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_X86_X86RAPASS_P_H_INCLUDED
#define ASMJIT_X86_X86RAPASS_P_H_INCLUDED

#include <asmjit/core/api-config.h>
#ifndef ASMJIT_NO_COMPILER

#include <asmjit/core/compiler.h>
#include <asmjit/core/racfgblock_p.h>
#include <asmjit/core/racfgbuilder_p.h>
#include <asmjit/core/rapass_p.h>
#include <asmjit/x86/x86assembler.h>
#include <asmjit/x86/x86compiler.h>
#include <asmjit/x86/x86emithelper_p.h>

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

//! \cond INTERNAL
//! \addtogroup asmjit_x86
//! \{

//! X86 register allocation pass.
//!
//! Takes care of generating function prologs and epilogs, and also performs register allocation.
class X86RAPass : public BaseRAPass {
public:
  ASMJIT_NONCOPYABLE(X86RAPass)
  using Base = BaseRAPass;

  //! \name Members
  //! \{

  EmitHelper _emit_helper;

  //! \}

  //! \name Construction & Destruction
  //! \{

  X86RAPass(BaseCompiler& cc) noexcept;
  ~X86RAPass() noexcept override;

  //! \}

  //! \name Accessors
  //! \{

  //! Returns the compiler casted to `x86::Compiler`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Compiler& cc() const noexcept { return static_cast<Compiler&>(_cb); }

  //! Returns emit helper.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG EmitHelper* emit_helper() noexcept { return &_emit_helper; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_avx_enabled() const noexcept { return _emit_helper.is_avx_enabled(); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_avx512_enabled() const noexcept { return _emit_helper.is_avx512_enabled(); }

  //! \}

  //! \name Interface
  //! \{

  void on_init() noexcept override;
  void on_done() noexcept override;

  Error build_cfg_nodes() noexcept override;

  Error rewrite() noexcept override;

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
#endif // ASMJIT_X86_X86RAPASS_P_H_INCLUDED
