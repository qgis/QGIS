// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/runtime_p.h>

// This file provides support for building Blend2D without C++ standard library.
// Blend2D doesn't really use any C++ features that would require a C++ runtime
// library, but there are little things here and there that require some attention.
#ifdef BL_BUILD_NO_STDCXX

BL_BEGIN_C_DECLS

// `__cxa_pure_virtual` replaces all abstract virtual functions (that have no
// implementation). We provide a replacement with `BL_HIDDEN` attribute to
// make it local to Blend2D library.
BL_HIDDEN void __cxa_pure_virtual() noexcept {
  bl_runtime_failure("[Blend2D] __cxa_pure_virtual(): Pure virtual function called");
}

BL_END_C_DECLS

#endif // BL_BUILD_NO_STDCXX
