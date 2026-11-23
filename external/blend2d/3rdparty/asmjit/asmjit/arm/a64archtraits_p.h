// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_ARM_A64ARCHTRAITS_P_H_INCLUDED
#define ASMJIT_ARM_A64ARCHTRAITS_P_H_INCLUDED

#include <asmjit/core/archtraits.h>
#include <asmjit/core/misc_p.h>
#include <asmjit/core/type.h>
#include <asmjit/arm/a64globals.h>
#include <asmjit/arm/a64operand.h>

ASMJIT_BEGIN_SUB_NAMESPACE(a64)

//! \cond INTERNAL
//! \addtogroup asmjit_a64
//! \{

static const constexpr ArchTraits a64_arch_traits = {
  // SP/FP/LR/PC.
  Gp::kIdSp, Gp::kIdFp, Gp::kIdLr, 0xFFu,

  // Reserved.
  { 0u, 0u, 0u },

  // HW stack alignment (AArch64 requires stack aligned to 16 bytes at HW level).
  16u,

  // Min/max stack offset - byte addressing is the worst, vec.q addressing the best.
  4095, 65520,

  // Supported register types.
  0u | (1u << uint32_t(RegType::kGp32  ))
     | (1u << uint32_t(RegType::kGp64  ))
     | (1u << uint32_t(RegType::kVec8  ))
     | (1u << uint32_t(RegType::kVec16 ))
     | (1u << uint32_t(RegType::kVec32 ))
     | (1u << uint32_t(RegType::kVec64 ))
     | (1u << uint32_t(RegType::kVec128))
     | (1u << uint32_t(RegType::kMask  )),

  // Instruction hints [Gp, Vec, Mask, Extra].
  {{
    InstHints::kPushPop,
    InstHints::kPushPop,
    InstHints::kNoHints,
    InstHints::kNoHints
  }},

  // TypeIdToRegType.
  #define V(index) (index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kInt8)    ? RegType::kGp32   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kUInt8)   ? RegType::kGp32   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kInt16)   ? RegType::kGp32   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kUInt16)  ? RegType::kGp32   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kInt32)   ? RegType::kGp32   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kUInt32)  ? RegType::kGp32   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kInt64)   ? RegType::kGp64   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kUInt64)  ? RegType::kGp64   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kIntPtr)  ? RegType::kGp64   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kUIntPtr) ? RegType::kGp64   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kFloat32) ? RegType::kVec32  : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kFloat64) ? RegType::kVec64  : RegType::kNone)
  {{ ASMJIT_LOOKUP_TABLE_32(V, 0) }},
  #undef V

  // Word names of 8-bit, 16-bit, 32-bit, and 64-bit quantities.
  {
    ArchTypeNameId::kByte,
    ArchTypeNameId::kHWord,
    ArchTypeNameId::kWord,
    ArchTypeNameId::kXWord
  }
};

//! \}
//! \endcond

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_ARM_A64ARCHTRAITS_P_H_INCLUDED
