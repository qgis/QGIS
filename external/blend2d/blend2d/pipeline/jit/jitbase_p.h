// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_PIPELINE_JIT_JITBASE_P_H_INCLUDED
#define BLEND2D_PIPELINE_JIT_JITBASE_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/support/intops_p.h>

#include <asmjit/ujit.h>

//! \cond INTERNAL
//! \addtogroup blend2d_pipeline_jit
//! \{

namespace bl::Pipeline::JIT {

// AsmJit::UJIT Integration
// ========================

using asmjit::AlignMode;
using asmjit::CpuFeatures;
using asmjit::CpuHints;
using asmjit::InstId;
using asmjit::imm;
using asmjit::Imm;
using asmjit::JumpAnnotation;
using asmjit::Label;
using asmjit::Operand;
using asmjit::Operand_;
using asmjit::OperandSignature;
using asmjit::Reg;
using asmjit::RegMask;
using asmjit::RegType;
using asmjit::RegGroup;

using namespace asmjit::ujit;

#if defined(ASMJIT_UJIT_X86)
namespace x86 { using namespace asmjit::x86; }
using KReg = x86::KReg;
#endif

#if defined(ASMJIT_UJIT_AARCH64)
namespace a64 { using namespace asmjit::a64; }
using asmjit::a64::OffsetMode;
#endif

#if defined(ASMJIT_UJIT_X86)
static constexpr VecWidth kMaxPlatformWidth = VecWidth::k512;
#else
static constexpr VecWidth kMaxPlatformWidth = VecWidth::k128;
#endif


// Strong Enums
// ------------

enum class PixelCount : uint32_t;

// Vec Width Utils
// ---------------

namespace VecWidthUtils {

 using namespace asmjit::ujit::VecWidthUtils;

//! Calculates ideal SIMD width for the requested `byte_count` considering the given `max_vec_width`.
//!
//! The returned VecWidth is at most `max_vec_width`, but could be lesser in case
//! the whole VecWidth is not required to process the requested `byte_count`.
static BL_INLINE_NODEBUG VecWidth vec_width_for_byte_count(VecWidth max_vec_width, size_t byte_count) noexcept {
  return bl_min(max_vec_width, VecWidth((byte_count + 15) >> 5));
}

//! Calculates the number of registers that would be necessary to hold the requested `byte_count`, considering
//! the given `max_vec_width`.
static BL_INLINE_NODEBUG size_t vec_count_for_byte_count(VecWidth max_vec_width, size_t byte_count) noexcept {
  uint32_t shift = uint32_t(max_vec_width) + 4u;
  return (byte_count + ((1u << shift) - 1u)) >> shift;
}

static BL_INLINE_NODEBUG VecWidth vec_width_of(const Vec& reg) noexcept {
  return VecWidth(uint32_t(reg.reg_type()) - uint32_t(RegType::kVec128));
}

static BL_INLINE_NODEBUG VecWidth vec_width_of(VecWidth vw, DataWidth data_width, size_t n) noexcept {
  return VecWidth(bl_min<uint32_t>(uint32_t((n << uint32_t(data_width)) >> 5), uint32_t(vw)));
}

static BL_INLINE size_t vec_count_of(VecWidth vw, DataWidth data_width, size_t n) noexcept {
  uint32_t shift = uint32_t(vw) + 4;
  size_t total_width = n << uint32_t(data_width);
  return (total_width + ((1 << shift) - 1)) >> shift;
}

static BL_INLINE size_t vec_count_of(VecWidth vw, DataWidth data_width, PixelCount n) noexcept {
  return vec_count_of(vw, data_width, uint32_t(n));
}

} // {VecWidthUtils}

} // {bl::Pipeline::JIT}

//! \}
//! \endcond

#endif // BLEND2D_PIPELINE_JIT_JITBASE_P_H_INCLUDED
