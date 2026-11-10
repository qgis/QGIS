// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_RUNTIMESCOPE_H_INCLUDED
#define BLEND2D_RUNTIMESCOPE_H_INCLUDED

#include <blend2d/core/api.h>

//! \addtogroup bl_runtime
//! \{

//! \name BLRuntimeScope - C API
//! \{

//! Blend2D runtime scope [C API].
struct BLRuntimeScopeCore {
  uint32_t data[2];
};

BL_BEGIN_C_DECLS

BL_API BLResult BL_CDECL bl_runtime_scope_begin(BLRuntimeScopeCore* self) BL_NOEXCEPT_C;
BL_API BLResult BL_CDECL bl_runtime_scope_end(BLRuntimeScopeCore* self) BL_NOEXCEPT_C;
BL_API bool BL_CDECL bl_runtime_scope_is_active(const BLRuntimeScopeCore* self) BL_NOEXCEPT_C;

BL_END_C_DECLS

//! \}

#if defined(__cplusplus)
//! \name BLRuntimeScope - C++ API
//! \{

//! Blend2D runtime scope [C++ API].
//!
//! Runtime scope is a totally optional feature that can be used by Blend2D to setup the current thread's control
//! word in a way to make the behavior of some floating point computations consistent between platforms. Blend2D
//! doesn't rely on any specific behavior, however, for testing purposes and possibly consistency of rendering
//! across different architectures some setup may be necessary.
//!
//! The runtime scope current only changes FPU control word in 32-bit x86 case to 64-bit precision. This means
//! that if the FPU control word was set to 80-bits the precision of floating computations would be basically
//! lowered, but this is necessary to make sure that intermediate computations match other platforms that don't
//! have extended precision. Blend2D doesn't rely on extended precision in any way and this all is needed only
//! if 100% consistency is required across different platforms.
//!
//! At the moment BLRuntimeScope is only used by tests to ensure that we can compare reference implementation
//! written in C++ with SIMD optimized implementation written either in C++ or generated at runtime (JIT).
//!
//! As the name of the class suggests, `BLRuntimeScope` establishes a scope, so the FPU control word would only
//! be changed temporarily within the life-time of the scope.
//!
//! Example:
//!
//! ```
//! void do_something(BLImage& image) noexcept {
//!   // Establishes a new scope by possibly changing the environment (FPU control word).
//!   BLRuntimeScope scope;
//!
//!   BLContext ctx(image);
//!   // ... do something ...
//!
//!   // The scope ends here - the environment is restored.
//! }
//! ```
class BLRuntimeScope final : public BLRuntimeScopeCore {
  BLRuntimeScope(BLRuntimeScope& other) noexcept = delete;
  BLRuntimeScope& operator=(BLRuntimeScope& other) noexcept = delete;

public:
  //! \name Construction & Destruction
  //! \{

  //! Establishes a new runtime scope by possibly changing the state of FPU control word.
  BL_INLINE_NODEBUG BLRuntimeScope() noexcept { bl_runtime_scope_begin(this); }
  //! Restores the scope to the previous state.
  BL_INLINE_NODEBUG ~BLRuntimeScope() noexcept { bl_runtime_scope_end(this); }

  //! \}

  //! \name Accessors
  //! \{

  BL_INLINE_NODEBUG bool is_active() const noexcept { return bl_runtime_scope_is_active(this); }

  //! \}
};
//! \}
#endif

//! \}

#endif // BLEND2D_RUNTIMESCOPE_H_INCLUDED
