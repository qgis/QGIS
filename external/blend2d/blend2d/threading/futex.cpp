// [Blend2D]
// 2D Vector Graphics Powered by a JIT Compiler.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/runtime_p.h>
#include <blend2d/threading/futex_p.h>

#if !defined(BL_BUILD_NO_FUTEX)
  #ifdef _WIN32
    #include <windows.h>
  #endif
#endif

// bl::Futex - Runtime Registration
// ================================

#if !defined(BL_BUILD_NO_FUTEX) && defined(__linux__)

void bl_fuxex_rt_init(BLRuntimeContext* rt) noexcept {
  // Initially in Linux 2.6.0, improved at 2.6.7, which is the minimum for FUTEX_WAIT_PRIVATE / FUTEX_WAKE_PRIVATE.
  rt->features_info.futex_enabled = 1;
}

#elif !defined(BL_BUILD_NO_FUTEX) && defined(__OpenBSD__)

void bl_fuxex_rt_init(BLRuntimeContext* rt) noexcept {
  // TODO: How to detect support on OpenBSD?
  rt->features_info.futex_enabled = 0;
}

#elif !defined(BL_BUILD_NO_FUTEX) && defined(_WIN32)

namespace bl::Futex::Native {
FutexSyncAPI futexSyncAPI;
} // {bl::Futex::Native}

void bl_fuxex_rt_init(BLRuntimeContext* rt) noexcept {
  HMODULE hModule = GetModuleHandleA("api-ms-win-core-synch-l1-2-0.dll");

  if (!hModule)
    return;

  using FutexSyncAPI = bl::Futex::Native::FutexSyncAPI;
  FutexSyncAPI& fn = bl::Futex::Native::futexSyncAPI;

  fn.WaitOnAddress       = (FutexSyncAPI::WaitOnAddressFunc      )GetProcAddress(hModule, "WaitOnAddress");
  fn.WakeByAddressSingle = (FutexSyncAPI::WakeByAddressSingleFunc)GetProcAddress(hModule, "WakeByAddressSingle");
  fn.WakeByAddressAll    = (FutexSyncAPI::WakeByAddressAllFunc   )GetProcAddress(hModule, "WakeByAddressAll");

  bool ok = fn.WaitOnAddress != nullptr && fn.WakeByAddressSingle != nullptr && fn.WakeByAddressAll != nullptr;
  rt->features_info.futex_enabled = ok;
}

#else

// Futex not supported.
void bl_fuxex_rt_init(BLRuntimeContext* rt) noexcept { bl_unused(rt); }

#endif
