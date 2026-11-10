// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#if !defined(ASMJIT_NO_X86) && !defined(ASMJIT_NO_COMPILER)

#include <asmjit/x86/x86assembler.h>
#include <asmjit/x86/x86compiler.h>
#include <asmjit/x86/x86instapi_p.h>
#include <asmjit/x86/x86rapass_p.h>

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

// x86::Compiler - Construction & Destruction
// ==========================================

Compiler::Compiler(CodeHolder* code) noexcept : BaseCompiler() {
  _arch_mask = (uint64_t(1) << uint32_t(Arch::kX86)) |
              (uint64_t(1) << uint32_t(Arch::kX64)) ;
  init_emitter_funcs(this);

  if (code) {
    code->attach(this);
  }
}
Compiler::~Compiler() noexcept {}

// x86::Compiler - Events
// ======================

Error Compiler::on_attach(CodeHolder& code) noexcept {
  ASMJIT_PROPAGATE(Base::on_attach(code));
  Error err = add_pass<X86RAPass>();

  if (ASMJIT_UNLIKELY(err != Error::kOk)) {
    on_detach(code);
    return err;
  }

  _instruction_alignment = uint8_t(1);
  update_emitter_funcs(this);

  return Error::kOk;
}

Error Compiler::on_detach(CodeHolder& code) noexcept {
  return Base::on_detach(code);
}

Error Compiler::on_reinit(CodeHolder& code) noexcept {
  Error err = Base::on_reinit(code);
  if (err == Error::kOk) {
    err = add_pass<X86RAPass>();
  }
  return err;
}

// x86::Compiler - Finalize
// ========================

Error Compiler::finalize() {
  ASMJIT_PROPAGATE(run_passes());
  Assembler a(_code);
  a.add_encoding_options(encoding_options());
  a.add_diagnostic_options(diagnostic_options());
  return serialize_to(&a);
}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_X86 && !ASMJIT_NO_COMPILER
