// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_ARM_A64BUILDER_H_INCLUDED
#define ASMJIT_ARM_A64BUILDER_H_INCLUDED

#include <asmjit/core/api-config.h>
#ifndef ASMJIT_NO_BUILDER

#include <asmjit/core/builder.h>
#include <asmjit/arm/a64emitter.h>

ASMJIT_BEGIN_SUB_NAMESPACE(a64)

//! \addtogroup asmjit_a64
//! \{

//! AArch64 builder implementation.
class ASMJIT_VIRTAPI Builder
  : public BaseBuilder,
    public EmitterExplicitT<Builder> {
public:
  ASMJIT_NONCOPYABLE(Builder)
  using Base = BaseBuilder;

  //! \name Construction & Destruction
  //! \{

  ASMJIT_API explicit Builder(CodeHolder* code = nullptr) noexcept;
  ASMJIT_API ~Builder() noexcept override;

  //! \}

  //! \name Events
  //! \{

  ASMJIT_API Error on_attach(CodeHolder& code) noexcept override;
  ASMJIT_API Error on_detach(CodeHolder& code) noexcept override;

  //! \}

  //! \name Finalize
  //! \{

  ASMJIT_API Error finalize() override;

  //! \}
};

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_BUILDER
#endif // ASMJIT_ARM_A64BUILDER_H_INCLUDED
