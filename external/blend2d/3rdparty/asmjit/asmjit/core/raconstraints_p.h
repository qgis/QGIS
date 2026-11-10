// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_RACONSTRAINTS_P_H_INCLUDED
#define ASMJIT_CORE_RACONSTRAINTS_P_H_INCLUDED

#include <asmjit/core/api-config.h>
#include <asmjit/core/archtraits.h>
#include <asmjit/core/operand.h>
#include <asmjit/support/support.h>

ASMJIT_BEGIN_NAMESPACE

//! \cond INTERNAL
//! \addtogroup asmjit_ra
//! \{

//! Provides architecture constraints used by register allocator.
class RAConstraints {
public:
  //! \name Members
  //! \{

  Support::Array<RegMask, Globals::kNumVirtGroups> _available_regs {};

  //! \}

  [[nodiscard]]
  ASMJIT_NOINLINE Error init(Arch arch) noexcept {
    switch (arch) {
      case Arch::kX86:
      case Arch::kX64: {
        uint32_t register_count = arch == Arch::kX86 ? 8 : 16;
        _available_regs[RegGroup::kGp] = Support::lsb_mask<RegMask>(register_count) & ~Support::bit_mask<RegMask>(4u);
        _available_regs[RegGroup::kVec] = Support::lsb_mask<RegMask>(register_count);
        _available_regs[RegGroup::kMask] = Support::lsb_mask<RegMask>(8);
        _available_regs[RegGroup::kX86_MM] = Support::lsb_mask<RegMask>(8);
        return Error::kOk;
      }

      case Arch::kAArch64: {
        _available_regs[RegGroup::kGp] = 0xFFFFFFFFu & ~Support::bit_mask<RegMask>(18, 31u);
        _available_regs[RegGroup::kVec] = 0xFFFFFFFFu;
        _available_regs[RegGroup::kMask] = 0;
        _available_regs[RegGroup::kExtra] = 0;
        return Error::kOk;
      }

      default:
        return make_error(Error::kInvalidArch);
    }
  }

  [[nodiscard]]
  inline RegMask available_regs(RegGroup group) const noexcept { return _available_regs[group]; }
};

//! \}
//! \endcond

ASMJIT_END_NAMESPACE

#endif // ASMJIT_CORE_RACONSTRAINTS_P_H_INCLUDED
