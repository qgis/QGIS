// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/runtimescope.h>

#if BL_TARGET_ARCH_X86
  #include <xmmintrin.h>
#endif

#if BL_TARGET_ARCH_X86 == 32 && defined(__GNUC__)
  #define BL_RUNTIME_SCOPE_X87
#endif

// BLRuntimeScope - Internal - X86 FPU State
// =========================================

namespace bl {
namespace FPU {

#if BL_TARGET_ARCH_X86
static constexpr uint32_t kCSR_EM_Invalid   = 0x0080u;
static constexpr uint32_t kCSR_EM_Denormal  = 0x0100u;
static constexpr uint32_t kCSR_EM_DivByZero = 0x0200u;
static constexpr uint32_t kCSR_EM_Overflow  = 0x0400u;
static constexpr uint32_t kCSR_EM_Underflow = 0x0800u;
static constexpr uint32_t kCSR_EM_Inexact   = 0x1000u;
static constexpr uint32_t kCSR_EM_Mask      = 0x1f80u;

static constexpr uint32_t kCSR_RC_Nearest   = 0x0000u;
static constexpr uint32_t kCSR_RC_Mask      = 0x6000u;

static constexpr uint32_t kCSR_FZ_Off       = 0x0000u;
static constexpr uint32_t kCSR_FZ_Mask      = 0x8000u;

static BL_INLINE_NODEBUG uint32_t read_csr() noexcept { return _mm_getcsr(); }
static BL_INLINE_NODEBUG void write_csr(uint32_t csr) noexcept { _mm_setcsr(csr); }
#endif

#if defined(BL_RUNTIME_SCOPE_X87)
static constexpr uint32_t kFPU_EM_Invalid   = 0x0001u;
static constexpr uint32_t kFPU_EM_Denormal  = 0x0002u;
static constexpr uint32_t kFPU_EM_DivByZero = 0x0004u;
static constexpr uint32_t kFPU_EM_Overflow  = 0x0008u;
static constexpr uint32_t kFPU_EM_Underflow = 0x0010u;
static constexpr uint32_t kFPU_EM_Inexact   = 0x0020u;
static constexpr uint32_t kFPU_EM_Mask      = 0x003Fu;

static constexpr uint32_t kFPU_PC_Float     = 0x0000u;
static constexpr uint32_t kFPU_PC_Double    = 0x0200u;
static constexpr uint32_t kFPU_PC_Mask      = 0x0300u;

static constexpr uint32_t kFPU_RC_Nearest   = 0x0000u;
static constexpr uint32_t kFPU_RC_Mask      = 0x0C00u;

static BL_INLINE_NODEBUG uint16_t read_fpu_cw() noexcept {
  uint16_t cw = 0;
  __asm__ __volatile__("fstcw %w0" : "=m" (cw));
  return cw;
}

static BL_INLINE_NODEBUG void write_fpu_cw(uint16_t cw) noexcept {
  __asm__ __volatile__("fldcw %w0" :: "m" (cw));
}

#endif

} // {FPU}
} // {bl}

// BLRuntimeScope - API - Begin & End (X86)
// ========================================

#if BL_TARGET_ARCH_X86
static BLResult bl_runtime_scope_begin_x86(BLRuntimeScopeCore* self) noexcept {
  using namespace bl::FPU;

  self->data[0] = 0;
  self->data[1] = 0;

  uint32_t prevCSR = read_csr();
  uint32_t csr = prevCSR;

  // Mask all exceptions - branchless code doesn't like it.
  csr = (csr & ~kCSR_EM_Mask) | kCSR_EM_Invalid   |
                                kCSR_EM_Denormal  |
                                kCSR_EM_DivByZero |
                                kCSR_EM_Overflow  |
                                kCSR_EM_Underflow |
                                kCSR_EM_Inexact   ;

  // Set an ABI compliant rounding mode (nearest).
  csr = (csr & ~kCSR_RC_Mask) | kCSR_RC_Nearest;

  // Set denormals flushing to off (we don't want to flush denormals to zero).
  csr = (csr & ~kCSR_FZ_Mask) | kCSR_FZ_Off;

#if defined(BL_RUNTIME_SCOPE_X87)
  uint32_t prev_fpu_cw = read_fpu_cw();
  uint32_t fcw = prev_fpu_cw;

  // Mask all exceptions - branchless code doesn't like it.
  fcw = (fcw & ~kFPU_EM_Mask) | kFPU_EM_Invalid   |
                                kFPU_EM_Denormal  |
                                kFPU_EM_DivByZero |
                                kFPU_EM_Overflow  |
                                kFPU_EM_Underflow |
                                kFPU_EM_Inexact   ;

  // Set an ABI compliant rounding mode (nearest).
  fcw = (fcw & ~kFPU_RC_Mask) | kFPU_RC_Nearest;

  // If the precisions is set to float (32-bit), make it double (64-bit) as we rely on 64-bit calculations.
  // However, if the precision is already double or extended, don't touch it as extended precision is what's
  // guaranteed by Linux ABI (this is the initial precision set for the thread/process).
  if ((fcw & kFPU_PC_Mask) == kFPU_PC_Float)
    fcw |= kFPU_PC_Double;

  uint32_t prev_state = (prev_fpu_cw << 16) | prevCSR;
  uint32_t new_state = (fcw << 16) | csr;

  // Don't update any states if we haven't changed anything.
  if (prev_state == new_state) {
    return BL_SUCCESS;
  }

  self->data[0] = prev_state;
  self->data[1] = 0xC0000000u;

  write_csr(csr);
  write_fpu_cw(uint16_t(fcw));
#else
  // Don't update any states if we haven't changed anything.
  if (prevCSR == csr) {
    return BL_SUCCESS;
  }

  self->data[0] = prevCSR;
  self->data[1] = 0x40000000u;

  write_csr(csr);
#endif

  return BL_SUCCESS;
}

static BLResult bl_runtime_scope_end_x86(BLRuntimeScopeCore* self) noexcept {
  using namespace bl::FPU;

  uint32_t state = self->data[0];
  uint32_t tag = self->data[1];

  // Reset the values to be sure that the state would never be used again.
  self->data[0] = 0;
  self->data[1] = 0;

  if (tag == 0u) {
    return BL_SUCCESS;
  }

  uint32_t csr = state & 0xFFFFu;

#if defined(BL_RUNTIME_SCOPE_X87)
  uint32_t fcw = state >> 16;

  if (BL_UNLIKELY(tag != 0xC0000000u)) {
    return bl_make_error(BL_ERROR_INVALID_STATE);
  }

  write_csr(csr);
  write_fpu_cw(uint16_t(fcw));
#else
  if (BL_UNLIKELY(tag != 0x40000000u)) {
    return bl_make_error(BL_ERROR_INVALID_STATE);
  }

  write_csr(csr);
#endif

  return BL_SUCCESS;
}
#endif

// BLRuntimeScope - API - Begin & End
// ==================================

BL_API_IMPL BLResult bl_runtime_scope_begin(BLRuntimeScopeCore* self) noexcept {
#if BL_TARGET_ARCH_X86
  return bl_runtime_scope_begin_x86(self);
#else
  self->data[0] = 0u;
  self->data[1] = 0u;
  return BL_SUCCESS;
#endif
}

BL_API_IMPL BLResult bl_runtime_scope_end(BLRuntimeScopeCore* self) noexcept {
#if BL_TARGET_ARCH_X86
  return bl_runtime_scope_end_x86(self);
#else
  self->data[0] = 0u;
  self->data[1] = 0u;
  return BL_SUCCESS;
#endif
}

BL_API_IMPL bool bl_runtime_scope_is_active(const BLRuntimeScopeCore* self) noexcept {
  // States saved are stored in MSB bits of data[1] - each platform has a different meaning, so just test the bits.
  return (self->data[1] & 0xC0000000u) != 0;
}
