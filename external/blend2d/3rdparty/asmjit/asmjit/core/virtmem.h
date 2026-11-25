// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_VIRTMEM_H_INCLUDED
#define ASMJIT_CORE_VIRTMEM_H_INCLUDED

#include <asmjit/core/api-config.h>
#ifndef ASMJIT_NO_JIT

#include <asmjit/core/globals.h>
#include <asmjit/support/support.h>

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_virtual_memory
//! \{

//! Virtual memory management.
namespace VirtMem {

//! Describes whether instruction cache should be flushed after a write operation.
enum class CachePolicy : uint32_t {
  //! Default policy.
  //!
  //! In some places this would mean `kFlushAfterWrite` and in some places it would mean `kNeverFlush`.
  //! For example if it's known that an address has never been used before to execute code.
  kDefault = 0,

  //! Flush instruction cache after a write operation.
  kFlushAfterWrite = 1,

  //! Avoid flushing instruction cache after a write operation.
  kNeverFlush = 2
};

//! Flushes instruction cache in the given region.
//!
//! Only useful on non-x86 architectures, however, it's a good practice to call it on any platform to make your
//! code more portable.
ASMJIT_API void flush_instruction_cache(void* p, size_t size) noexcept;

//! Virtual memory information.
struct Info {
  //! Virtual memory page size.
  uint32_t page_size;
  //! Virtual memory page granularity.
  uint32_t page_granularity;
};

//! Returns virtual memory information, see `VirtMem::Info` for more details.
[[nodiscard]]
ASMJIT_API Info info() noexcept;

//! Returns the size of the smallest large page supported.
//!
//! AsmJit only uses the smallest large page at the moment as these are usually perfectly sized for executable
//! memory allocation (standard size is 2MB, but different sizes are possible).
//!
//! Returns either the detected large page size or 0, if large page support is either not supported by AsmJit
//! or not accessible to the process.
[[nodiscard]]
ASMJIT_API size_t large_page_size() noexcept;

//! Virtual memory access and mmap-specific flags.
enum class MemoryFlags : uint32_t {
  //! No flags.
  kNone = 0,

  //! Memory is readable.
  kAccessRead = 0x00000001u,

  //! Memory is writable.
  kAccessWrite = 0x00000002u,

  //! Memory is executable.
  kAccessExecute = 0x00000004u,

  //! A combination of \ref kAccessRead and \ref kAccessWrite.
  kAccessReadWrite = kAccessRead | kAccessWrite,

  //! A combination of \ref kAccessRead, \ref kAccessWrite.
  kAccessRW = kAccessRead | kAccessWrite,

  //! A combination of \ref kAccessRead and \ref kAccessExecute.
  kAccessRX = kAccessRead | kAccessExecute,

  //! A combination of \ref kAccessRead, \ref kAccessWrite, and \ref kAccessExecute.
  kAccessRWX = kAccessRead | kAccessWrite | kAccessExecute,

  //! Use a `MAP_JIT` flag available on Apple platforms (introduced by Mojave), which allows JIT code to be
  //! executed in a MAC bundle.
  //!
  //! This flag may be turned on by the allocator if there is no other way of allocating executable memory.
  //!
  //! \note This flag can only be used with \ref VirtMem::alloc(), `MAP_JIT` only works on OSX and not on iOS.
  //! When a process uses `fork()` the child process has no access to the pages mapped with `MAP_JIT`.
  kMMapEnableMapJit = 0x00000010u,

  //! Pass `PROT_MAX(PROT_READ)` or `PROT_MPROTECT(PROT_READ)` to `mmap()` on platforms that support it.
  //!
  //! This flag allows to set a "maximum access" that the memory page can get during its lifetime. Use
  //! \ref VirtMem::protect() to change the access flags.
  //!
  //! \note This flag can only be used with \ref VirtMem::alloc() and \ref VirtMem::alloc_dual_mapping().
  //! However \ref VirtMem::alloc_dual_mapping() may automatically use this if \ref kAccessRead is used.
  kMMapMaxAccessRead = 0x00000020u,

  //! Pass `PROT_MAX(PROT_WRITE)` or `PROT_MPROTECT(PROT_WRITE)` to `mmap()` on platforms that support it.
  //!
  //! This flag allows to set a "maximum access" that the memory page can get during its lifetime. Use
  //! \ref VirtMem::protect() to change the access flags.
  //!
  //! \note This flag can only be used with \ref VirtMem::alloc() and \ref VirtMem::alloc_dual_mapping().
  //! However \ref VirtMem::alloc_dual_mapping() may automatically use this if \ref kAccessWrite is used.
  kMMapMaxAccessWrite = 0x00000040u,

  //! Pass `PROT_MAX(PROT_EXEC)` or `PROT_MPROTECT(PROT_EXEC)` to `mmap()` on platforms that support it.
  //!
  //! This flag allows to set a "maximum access" that the memory page can get during its lifetime. Use
  //! \ref VirtMem::protect() to change the access flags.
  //!
  //! \note This flag can only be used with \ref VirtMem::alloc() and \ref VirtMem::alloc_dual_mapping().
  //! However \ref VirtMem::alloc_dual_mapping() may automatically use this if \ref kAccessExecute is used.
  kMMapMaxAccessExecute = 0x00000080u,

  //! A combination of \ref kMMapMaxAccessRead and \ref kMMapMaxAccessWrite.
  kMMapMaxAccessReadWrite = kMMapMaxAccessRead | kMMapMaxAccessWrite,

  //! A combination of \ref kMMapMaxAccessRead and \ref kMMapMaxAccessWrite.
  kMMapMaxAccessRW = kMMapMaxAccessRead | kMMapMaxAccessWrite,

  //! A combination of \ref kMMapMaxAccessRead and \ref kMMapMaxAccessExecute.
  kMMapMaxAccessRX = kMMapMaxAccessRead | kMMapMaxAccessExecute,

  //! A combination of \ref kMMapMaxAccessRead, \ref kMMapMaxAccessWrite, \ref kMMapMaxAccessExecute.
  kMMapMaxAccessRWX = kMMapMaxAccessRead | kMMapMaxAccessWrite | kMMapMaxAccessExecute,

  //! Use `MAP_SHARED` when calling mmap().
  //!
  //! \note In some cases `MAP_SHARED` may be set automatically. For example, some dual mapping implementations must
  //! use `MAP_SHARED` instead of `MAP_PRIVATE` to ensure that the OS would not apply copy on write on RW page, which
  //! would cause RX page not having the updated content.
  kMapShared = 0x00000100u,

  //! Request large memory mapped pages.
  //!
  //! \remarks If this option is used and large page(s) cannot be mapped, the allocation will fail. Fallback to
  //! regular pages must be done by the user in this case. Higher level API such as \ref JitAllocator provides an
  //! additional mechanism to allocate regular page(s) when large page(s) allocation fails.
  kMMapLargePages = 0x00000200u,

  //! Not an access flag, only used by `alloc_dual_mapping()` to override the default allocation strategy to always use
  //! a 'tmp' directory instead of "/dev/shm" (on POSIX platforms). Please note that this flag will be ignored if the
  //! operating system allows to allocate an executable memory by a different API than `open()` or `shm_open()`. For
  //! example on Linux `memfd_create()` is preferred and on BSDs `shm_open(SHM_ANON, ...)` is used if SHM_ANON is
  //! defined.
  //!
  //! \note This flag can only be used with \ref VirtMem::alloc().
  kMappingPreferTmp = 0x80000000u
};
ASMJIT_DEFINE_ENUM_FLAGS(MemoryFlags)

//! Allocates virtual memory by either using `mmap()` (POSIX) or `VirtualAlloc()` (Windows).
//!
//! \note `size` should be aligned to page size, use \ref VirtMem::info() to obtain it. Invalid size will not be
//! corrected by the implementation and the allocation would not succeed in such case.
[[nodiscard]]
ASMJIT_API Error alloc(void** p, size_t size, MemoryFlags flags) noexcept;

//! Releases virtual memory previously allocated by \ref VirtMem::alloc().
//!
//! \note The size must be the same as used by \ref VirtMem::alloc(). If the size is not the same value the call
//! will fail on any POSIX system, but pass on Windows, because it's implemented differently.
[[nodiscard]]
ASMJIT_API Error release(void* p, size_t size) noexcept;

//! A cross-platform wrapper around `mprotect()` (POSIX) and `VirtualProtect()` (Windows).
[[nodiscard]]
ASMJIT_API Error protect(void* p, size_t size, MemoryFlags flags) noexcept;

//! Dual memory mapping used to map an anonymous memory into two memory regions where one region is read-only, but
//! executable, and the second region is read+write, but not executable. See \ref VirtMem::alloc_dual_mapping() for
//! more details.
struct DualMapping {
  //! Pointer to data with 'Read+Execute' access (this memory is not writable).
  void* rx;
  //! Pointer to data with 'Read+Write' access (this memory is not executable).
  void* rw;
};

//! Allocates virtual memory and creates two views of it where the first view has no write access. This is an addition
//! to the API that should be used in cases in which the operating system either enforces W^X security policy or the
//! application wants to use this policy by default to improve security and prevent an accidental (or purposed)
//! self-modifying code.
//!
//! The memory returned in the `dm` are two independent mappings of the same shared memory region. You must use
//! \ref VirtMem::release_dual_mapping() to release it when it's no longer needed. Never use `VirtMem::release()` to
//! release the memory returned by `alloc_dual_mapping()` as that would fail on Windows.
//!
//! \remarks Both pointers in `dm` would be set to `nullptr` if the function fails.
[[nodiscard]]
ASMJIT_API Error alloc_dual_mapping(Out<DualMapping> dm, size_t size, MemoryFlags flags) noexcept;

//! Releases virtual memory mapping previously allocated by \ref VirtMem::alloc_dual_mapping().
//!
//! \remarks Both pointers in `dm` would be set to `nullptr` if the function succeeds.
[[nodiscard]]
ASMJIT_API Error release_dual_mapping(DualMapping& dm, size_t size) noexcept;

//! Hardened runtime flags.
enum class HardenedRuntimeFlags : uint32_t {
  //! No flags.
  kNone = 0,

  //! Hardened runtime is enabled - it's not possible to have "Write & Execute" memory protection. The runtime
  //! enforces W^X (either write or execute).
  //!
  //! \note If the runtime is hardened it means that an operating system specific protection is used. For example
  //! on Apple OSX it's possible to allocate memory with MAP_JIT flag and then use `pthread_jit_write_protect_np()`
  //! to temporarily swap access permissions for the current thread. Dual mapping is also a possibility on X86/X64
  //! architecture.
  kEnabled = 0x00000001u,

  //! Read+Write+Execute can only be allocated with MAP_JIT flag (Apple specific, only available on Apple platforms).
  kMapJit = 0x00000002u,

  //! Read+Write+Execute can be allocated with dual mapping approach (one region with RW and the other with RX).
  kDualMapping = 0x00000004u
};
ASMJIT_DEFINE_ENUM_FLAGS(HardenedRuntimeFlags)

//! Hardened runtime information.
struct HardenedRuntimeInfo {
  //! \name Members
  //! \{

  //! Hardened runtime flags.
  HardenedRuntimeFlags flags;

  //! \}

  //! \name Accessors
  //! \{

  //! Tests whether the hardened runtime `flag` is set.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_flag(HardenedRuntimeFlags flag) const noexcept { return Support::test(flags, flag); }

  //! \}
};

//! Returns runtime features provided by the OS.
[[nodiscard]]
ASMJIT_API HardenedRuntimeInfo hardened_runtime_info() noexcept;

//! Values that can be used with `protect_jit_memory()` function.
enum class ProtectJitAccess : uint32_t {
  //! Protect JIT memory with Read+Write permissions.
  kReadWrite = 0,
  //! Protect JIT memory with Read+Execute permissions.
  kReadExecute = 1
};

//! Protects access of memory mapped with MAP_JIT flag for the current thread.
//!
//! \note This feature is only available on Apple hardware (AArch64) at the moment and uses a non-portable
//! `pthread_jit_write_protect_np()` call when available.
//!
//! This function must be called before and after a memory mapped with MAP_JIT flag is modified. Example:
//!
//! ```
//! void* code_ptr = ...;
//! size_t code_size = ...;
//!
//! VirtMem::protect_jit_memory(VirtMem::ProtectJitAccess::kReadWrite);
//! memcpy(code_ptr, source, code_size);
//! VirtMem::protect_jit_memory(VirtMem::ProtectJitAccess::kReadExecute);
//! VirtMem::flush_instruction_cache(code_ptr, code_size);
//! ```
//!
//! See \ref ProtectJitReadWriteScope, which makes it simpler than the code above.
ASMJIT_API void protect_jit_memory(ProtectJitAccess access) noexcept;

//! JIT protection scope that prepares the given memory block to be written to in the current thread.
//!
//! It calls `VirtMem::protect_jit_memory(VirtMem::ProtectJitAccess::kReadWrite)` at construction time and
//! `VirtMem::protect_jit_memory(VirtMem::ProtectJitAccess::kReadExecute)` combined with `flush_instruction_cache()`
//! in destructor. The purpose of this class is to make writing to JIT memory easier.
class ProtectJitReadWriteScope {
public:
  ASMJIT_NONCOPYABLE(ProtectJitReadWriteScope)

  //! \name Members
  //! \{

  void* _rx_ptr;
  size_t _size;
  CachePolicy _policy;

  //! \}

  //! \name Construction & Destruction
  //! \{

  //! Makes the given memory block RW protected.
  ASMJIT_INLINE ProtectJitReadWriteScope(
    void* rx_ptr,
    size_t size,
    CachePolicy policy = CachePolicy::kDefault
  ) noexcept
    : _rx_ptr(rx_ptr),
      _size(size),
      _policy(policy) {
    protect_jit_memory(ProtectJitAccess::kReadWrite);
  }

  //! Makes the memory block RX protected again and flushes instruction cache.
  ASMJIT_INLINE  ~ProtectJitReadWriteScope() noexcept {
    protect_jit_memory(ProtectJitAccess::kReadExecute);

    if (_policy != CachePolicy::kNeverFlush) {
      flush_instruction_cache(_rx_ptr, _size);
    }
  }

  //! \}
};

} // VirtMem

//! \}

ASMJIT_END_NAMESPACE

#endif
#endif // ASMJIT_CORE_VIRTMEM_H_INCLUDED
