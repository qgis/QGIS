// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#include <asmjit/core/environment.h>

ASMJIT_BEGIN_NAMESPACE

// X86 Target
// ----------
//
//   - 32-bit - Linux, OSX, BSD, and apparently also Haiku guarantee 16-byte
//              stack alignment. Other operating systems are assumed to have
//              4-byte alignment by default for safety reasons.
//   - 64-bit - stack must be aligned to 16 bytes.
//
// ARM Target
// ----------
//
//   - 32-bit - Stack must be aligned to 8 bytes.
//   - 64-bit - Stack must be aligned to 16 bytes (hardware requirement).
uint32_t Environment::stack_alignment() const noexcept {
  if (is_64bit()) {
    // Assume 16-byte alignment on any 64-bit target.
    return 16;
  }
  else {
    // The following platforms use 16-byte alignment in 32-bit mode.
    if (is_platform_linux() ||
        is_platform_bsd() ||
        is_platform_apple() ||
        is_platform_haiku()) {
      return 16u;
    }

    if (is_family_arm()) {
      return 8;
    }

    // Bail to 4-byte alignment if we don't know.
    return 4;
  }
}

ASMJIT_END_NAMESPACE
