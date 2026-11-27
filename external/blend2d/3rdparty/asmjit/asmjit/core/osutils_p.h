// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_OSUTILS_P_H_INCLUDED
#define ASMJIT_CORE_OSUTILS_P_H_INCLUDED

#include <asmjit/core/osutils.h>
#include <asmjit/core/string.h>

ASMJIT_BEGIN_NAMESPACE

//! \cond INTERNAL
//! \addtogroup asmjit_utilities
//! \{

#if defined(_WIN32)

// Windows implementation.
static_assert(sizeof(Lock::Handle) == sizeof(CRITICAL_SECTION), "asmjit::Lock::Handle layout must match CRITICAL_SECTION");
static_assert(alignof(Lock::Handle) == alignof(CRITICAL_SECTION), "asmjit::Lock::Handle alignment must match CRITICAL_SECTION");

ASMJIT_INLINE_NODEBUG Lock::Lock() noexcept { InitializeCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(&_handle)); }
ASMJIT_INLINE_NODEBUG Lock::~Lock() noexcept { DeleteCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(&_handle)); }
ASMJIT_INLINE_NODEBUG void Lock::lock() noexcept { EnterCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(&_handle)); }
ASMJIT_INLINE_NODEBUG void Lock::unlock() noexcept { LeaveCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(&_handle)); }

#elif !defined(__EMSCRIPTEN__)

// PThread implementation.
#ifdef PTHREAD_MUTEX_INITIALIZER
ASMJIT_INLINE_NODEBUG Lock::Lock() noexcept : _handle(PTHREAD_MUTEX_INITIALIZER) {}
#else
ASMJIT_INLINE_NODEBUG Lock::Lock() noexcept { pthread_mutex_init(&_handle, nullptr); }
#endif
ASMJIT_INLINE_NODEBUG Lock::~Lock() noexcept { pthread_mutex_destroy(&_handle); }
ASMJIT_INLINE_NODEBUG void Lock::lock() noexcept { pthread_mutex_lock(&_handle); }
ASMJIT_INLINE_NODEBUG void Lock::unlock() noexcept { pthread_mutex_unlock(&_handle); }

#else

// Dummy implementation - Emscripten or other unsupported platform.
ASMJIT_INLINE_NODEBUG Lock::Lock() noexcept {}
ASMJIT_INLINE_NODEBUG Lock::~Lock() noexcept {}
ASMJIT_INLINE_NODEBUG void Lock::lock() noexcept {}
ASMJIT_INLINE_NODEBUG void Lock::unlock() noexcept {}

#endif

//! Scoped lock.
class LockGuard {
public:
  ASMJIT_NONCOPYABLE(LockGuard)

  Lock& _target;

  ASMJIT_INLINE_NODEBUG LockGuard(Lock& target) noexcept
    : _target(target) { _target.lock(); }
  ASMJIT_INLINE_NODEBUG ~LockGuard() noexcept { _target.unlock(); }
};

#if !defined(_WIN32)
namespace OSUtils {

//! Reads a file, only used on non-Windows platforms to access /sys or other files when necessary.
Error read_file(const char* name, String& dst, size_t max_size) noexcept;

} // {OSUtils}
#endif

//! \}
//! \endcond

ASMJIT_END_NAMESPACE

#endif // ASMJIT_CORE_OSUTILS_P_H_INCLUDED

