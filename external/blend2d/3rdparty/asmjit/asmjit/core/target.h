// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_TARGET_H_INCLUDED
#define ASMJIT_CORE_TARGET_H_INCLUDED

#include <asmjit/core/archtraits.h>
#include <asmjit/core/cpuinfo.h>
#include <asmjit/core/func.h>

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core
//! \{

//! Target is an abstract class that describes a machine code target.
class ASMJIT_VIRTAPI Target {
public:
  ASMJIT_BASE_CLASS(Target)
  ASMJIT_NONCOPYABLE(Target)

  //! Target environment information.
  Environment _environment;
  //! Target CPU features.
  CpuFeatures _cpu_features;
  //! Target CPU hints.
  CpuHints _cpu_hints;

  //! \name Construction & Destruction
  //! \{

  //! Creates a `Target` instance.
  ASMJIT_API Target() noexcept;
  //! Destroys the `Target` instance.
  ASMJIT_API virtual ~Target() noexcept;

  //! \}

  //! \name Accessors
  //! \{

  //! Returns target's environment.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const Environment& environment() const noexcept { return _environment; }

  //! Returns the target architecture.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Arch arch() const noexcept { return _environment.arch(); }

  //! Returns the target sub-architecture.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG SubArch sub_arch() const noexcept { return _environment.sub_arch(); }

  [[nodiscard]]
  //! Returns target CPU features.
  ASMJIT_INLINE_NODEBUG const CpuFeatures& cpu_features() const noexcept { return _cpu_features; }

  [[nodiscard]]
  //! Returns target CPU hints.
  ASMJIT_INLINE_NODEBUG CpuHints cpu_hints() const noexcept { return _cpu_hints; }

  //! \}
};

//! \}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_CORE_TARGET_H_INCLUDED
