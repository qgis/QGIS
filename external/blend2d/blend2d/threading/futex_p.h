// [Blend2D]
// 2D Vector Graphics Powered by a JIT Compiler.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#ifndef BLEND2D_THREADING_FUTEX_P_H
#define BLEND2D_THREADING_FUTEX_P_H

#include <blend2d/core/api-internal_p.h>
#include <blend2d/core/runtime_p.h>

#ifndef BL_BUILD_NO_FUTEX
  #if defined(__linux__)
    #include <linux/futex.h>
    #include <sys/syscall.h>
    #include <unistd.h>
  #elif defined(__OpenBSD__)
    #include <sys/futex.h>
  #elif defined(_WIN32)
    // Nothing to include when compiling for Windows...
  #else
    // No futex implementation for the target platform.
    #define BL_BUILD_NO_FUTEX
  #endif
#endif

//! \cond INTERNAL

#ifndef BL_BUILD_NO_FUTEX
  #define BL_FUTEX_ENABLED (bl_runtime_context.features_info.futex_enabled)
#else
  #define BL_FUTEX_ENABLED 0
#endif

namespace bl::Futex {

//! Namespace that contains native futex ops. The main reason they have to be wrapped is TSAN support, which
//! doesn't handle syscalls, so we have to manually annotate `wait()`, `wake_one()`, and `wake_all()` functions.
namespace Native {

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

#if !defined(BL_BUILD_NO_FUTEX) && defined(__linux__)

static BL_INLINE_NODEBUG int make_sys_call(uint32_t* addr, int op, int x) noexcept { return int(syscall(SYS_futex, (void*)addr, op, x, nullptr, nullptr, 0)); }

static BL_INLINE_NODEBUG int wait(uint32_t* addr, uint32_t value) noexcept { return make_sys_call(addr, FUTEX_WAIT_PRIVATE, int(value)); }
static BL_INLINE_NODEBUG int wake_one(uint32_t* addr) noexcept { return make_sys_call(addr, FUTEX_WAKE_PRIVATE, 1); }
static BL_INLINE_NODEBUG int wake_all(uint32_t* addr) noexcept { return make_sys_call(addr, FUTEX_WAKE_PRIVATE, std::numeric_limits<int>::max()); }

#elif !defined(BL_BUILD_NO_FUTEX) && defined(__OpenBSD__)

static BL_INLINE_NODEBUG int make_sys_call(uint32_t* addr, int op, int x) noexcept { return futex((volatile uint32_t *)addr, op, x, nullptr, nullptr); }

static BL_INLINE_NODEBUG int wait(uint32_t* addr, uint32_t value) noexcept { return make_sys_call(addr, FUTEX_WAIT, int(value)); }
static BL_INLINE_NODEBUG int wake_one(uint32_t* addr) noexcept { return make_sys_call(addr, FUTEX_WAKE, 1); }
static BL_INLINE_NODEBUG int wake_all(uint32_t* addr) noexcept { return make_sys_call(addr, FUTEX_WAKE, std::numeric_limits<int>::max()); }

#elif !defined(BL_BUILD_NO_FUTEX) && defined(_WIN32)

struct FutexSyncAPI {
  typedef int (BL_STDCALL *WaitOnAddressFunc)(void* Address, void* CompareAddress, size_t AddressSize, uint32_t dw_milliseconds);
  typedef void (BL_STDCALL *WakeByAddressSingleFunc)(void* Address);
  typedef void (BL_STDCALL *WakeByAddressAllFunc)(void* Address);

  WaitOnAddressFunc WaitOnAddress;
  WakeByAddressSingleFunc WakeByAddressSingle;
  WakeByAddressAllFunc WakeByAddressAll;
};

extern FutexSyncAPI futexSyncAPI;

static BL_INLINE_NODEBUG int wait(uint32_t* addr, uint32_t x) noexcept { futexSyncAPI.WaitOnAddress((void*)addr, &x, sizeof(x), INFINITE); return 0; }
static BL_INLINE_NODEBUG int wake_one(uint32_t* addr) noexcept { futexSyncAPI.WakeByAddressSingle((void*)addr); return 0; }
static BL_INLINE_NODEBUG int wake_all(uint32_t* addr) noexcept { futexSyncAPI.WakeByAddressAll((void*)addr); return 0; }

#endif

//! \}
//! \endcond

} // {Native}

#if !defined(BL_BUILD_NO_FUTEX)
static BL_INLINE int wait(uint32_t* addr, uint32_t value) noexcept {
  int result = Native::wait(addr, value);

#if defined(BL_SANITIZE_THREAD)
  if (result == 0) {
    __tsan_acquire(addr);
  }
#endif // BL_SANITIZE_THREAD

  return result;
}

static BL_INLINE int wake_one(uint32_t* addr) noexcept {
#if defined(BL_SANITIZE_THREAD)
  __tsan_release(addr);
#endif // BL_SANITIZE_THREAD

  return Native::wake_one(addr);
}

static BL_INLINE int wake_all(uint32_t* addr) noexcept {
#if defined(BL_SANITIZE_THREAD)
  __tsan_release(addr);
#endif // BL_SANITIZE_THREAD

  return Native::wake_all(addr);
}

#else
static BL_INLINE_NODEBUG int wait(uint32_t*, uint32_t) noexcept { return -1; }
static BL_INLINE_NODEBUG int wake_one(uint32_t*) noexcept { return -1; }
static BL_INLINE_NODEBUG int wake_all(uint32_t*) noexcept { return -1; }
#endif

} // {bl::Futex}

//! \endcond

#endif // BLEND2D_THREADING_FUTEX_P_H
