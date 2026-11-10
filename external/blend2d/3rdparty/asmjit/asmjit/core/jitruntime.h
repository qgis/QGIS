// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_JITRUNTIME_H_INCLUDED
#define ASMJIT_CORE_JITRUNTIME_H_INCLUDED

#include <asmjit/core/api-config.h>
#ifndef ASMJIT_NO_JIT

#include <asmjit/core/codeholder.h>
#include <asmjit/core/jitallocator.h>
#include <asmjit/core/target.h>

ASMJIT_BEGIN_NAMESPACE

class CodeHolder;

//! \addtogroup asmjit_virtual_memory
//! \{

//! JIT execution runtime is a special `Target` that is designed to store and execute a generated code.
//!
//! JIT runtime is the easiest way of using AsmJit as it abstracts allocation and deallocation of virtual memory
//! where executable code can be placed and from which it can be executed as well.
class ASMJIT_VIRTAPI JitRuntime : public Target {
public:
  ASMJIT_NONCOPYABLE(JitRuntime)

  //! Virtual memory allocator.
  JitAllocator _allocator;

  //! \name Construction & Destruction
  //! \{

  //! Creates a `JitRuntime` instance.
  ASMJIT_API explicit JitRuntime(const JitAllocator::CreateParams* params = nullptr) noexcept;

  //! Creates a `JitRuntime` instance.
  ASMJIT_INLINE explicit JitRuntime(const JitAllocator::CreateParams& params) noexcept
    : JitRuntime(&params) {}

  //! Destroys the `JitRuntime` instance.
  ASMJIT_API ~JitRuntime() noexcept override;

  //! \}

  //! \name Accessors
  //! \{

  //! Resets the \ref JitRuntime, freeing everything that was allocated by it.
  //!
  //! Depending on `reset_policy` the currently held memory can be either freed entirely when ResetPolicy::kHard is used,
  //! or the allocator can keep some of it for next allocations when ResetPolicy::kSoft is used, which is the default
  //! behavior.
  ASMJIT_INLINE_NODEBUG void reset(ResetPolicy reset_policy = ResetPolicy::kSoft) noexcept {
    _allocator.reset(reset_policy);
  }

  //! \}

  //! \name Accessors
  //! \{

  //! Returns the associated `JitAllocator`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG JitAllocator& allocator() const noexcept { return const_cast<JitAllocator&>(_allocator); }

  //! \}

  //! \name Utilities
  //! \{

  // NOTE: To allow passing function pointers to `add()` and `release()` the
  // virtual methods are prefixed with `_` and called from templates instead.

  //! Allocates memory needed for a code stored in the `CodeHolder` and relocates the code to the pointer allocated.
  //!
  //! The beginning of the memory allocated for the function is returned in `dst`. If failed `Error` code is returned
  //! and `dst` is explicitly set to `nullptr`  (this means that you don't have to set it to null before calling `add()`).
  template<typename Func>
  ASMJIT_INLINE_NODEBUG Error add(Func* dst, CodeHolder* code) noexcept {
    return _add(Support::ptr_cast_impl<void**, Func*>(dst), code);
  }

  //! Releases `p` which was obtained by calling `add()`.
  template<typename Func>
  ASMJIT_INLINE_NODEBUG Error release(Func p) noexcept {
    return _release(Support::ptr_cast_impl<void*, Func>(p));
  }

  //! Type-unsafe version of `add()`.
  ASMJIT_API virtual Error _add(void** dst, CodeHolder* code) noexcept;

  //! Type-unsafe version of `release()`.
  ASMJIT_API virtual Error _release(void* p) noexcept;

  //! \}
};

//! \}

ASMJIT_END_NAMESPACE

#endif
#endif
