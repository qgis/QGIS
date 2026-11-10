// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#ifndef ASMJIT_NO_JIT

#include <asmjit/core/osutils_p.h>
#include <asmjit/core/string.h>
#include <asmjit/core/virtmem.h>
#include <asmjit/support/support.h>

#if !defined(_WIN32)
  #include <errno.h>
  #include <fcntl.h>
  #include <sys/mman.h>
  #include <sys/stat.h>
  #include <sys/types.h>
  #include <unistd.h>

  #if !ASMJIT_ARCH_X86
    #include <sys/time.h> // required by gettimeofday()
  #endif

  // Linux has a `memfd_create` syscall that we would like to use, if available.
  #if defined(__linux__)
    #include <sys/syscall.h>
    #include <sys/utsname.h>

    #ifndef MAP_HUGETLB
      #define MAP_HUGETLB 0x40000
    #endif // MAP_HUGETLB

    #ifndef MAP_HUGE_SHIFT
      #define MAP_HUGE_SHIFT 26
    #endif // MAP_HUGE_SHIFT

    #if !defined(MFD_CLOEXEC)
      #define MFD_CLOEXEC 0x0001u
    #endif // MFD_CLOEXEC

    #if !defined(MFD_NOEXEC_SEAL)
      #define MFD_NOEXEC_SEAL 0x0008u
    #endif // MFD_NOEXEC_SEAL

    #if !defined(MFD_EXEC)
      #define MFD_EXEC 0x0010u
    #endif // MFD_EXEC

    #ifndef MFD_HUGETLB
      #define MFD_HUGETLB 0x0004
    #endif // MFD_HUGETLB

    #ifndef MFD_HUGE_SHIFT
      #define MFD_HUGE_SHIFT 26
    #endif // MFD_HUGE_SHIFT
  #endif

  // Apple recently introduced MAP_JIT flag, which we want to use.
  #if defined(__APPLE__)
    #include <pthread.h>
    #include <TargetConditionals.h>
    #if TARGET_OS_OSX
      #include <sys/utsname.h>
      #include <libkern/OSCacheControl.h> // sys_icache_invalidate().
    #endif
    // Older SDK doesn't define `MAP_JIT`.
    #ifndef MAP_JIT
      #define MAP_JIT 0x800
    #endif
  #endif

  // BSD/MAC: `MAP_ANONYMOUS` is not defined, `MAP_ANON` is.
  #if !defined(MAP_ANONYMOUS)
    #define MAP_ANONYMOUS MAP_ANON
  #endif

  // Android NDK doesn't provide `shm_open()` and `shm_unlink()`.
  #if !defined(__BIONIC__) && !defined(ASMJIT_NO_SHM_OPEN)
    #define ASMJIT_HAS_SHM_OPEN
  #endif

  #if defined(__APPLE__) || defined(__BIONIC__) || !defined(ASMJIT_HAS_SHM_OPEN)
    #define ASMJIT_VM_SHM_DETECT 0
  #else
    #define ASMJIT_VM_SHM_DETECT 1
  #endif

  #if defined(__APPLE__) && TARGET_OS_OSX
    #if ASMJIT_ARCH_X86 != 0
      #define ASMJIT_ANONYMOUS_MEMORY_USE_MACH_VM_REMAP
    #endif
    #if ASMJIT_ARCH_ARM >= 64
      #define ASMJIT_HAS_PTHREAD_JIT_WRITE_PROTECT_NP
    #endif
  #endif

  #if defined(__APPLE__) && ASMJIT_ARCH_X86 == 0
    #define ASMJIT_NO_DUAL_MAPPING
  #endif

  #if defined(__NetBSD__) && defined(MAP_REMAPDUP) && defined(PROT_MPROTECT)
    #define ASMJIT_ANONYMOUS_MEMORY_USE_REMAPDUP
  #endif

  #if !defined(ASMJIT_ANONYMOUS_MEMORY_USE_REMAPDUP) && \
      !defined(ASMJIT_ANONYMOUS_MEMORY_USE_MACH_VM_REMAP) && \
      !defined(ASMJIT_NO_DUAL_MAPPING)
    #define ASMJIT_ANONYMOUS_MEMORY_USE_FD
  #endif
#endif

#include <atomic>

#if defined(ASMJIT_ANONYMOUS_MEMORY_USE_MACH_VM_REMAP)
#include <mach/mach.h>
#include <mach/mach_time.h>

extern "C" {

#ifdef mig_external
mig_external
#else
extern
#endif
kern_return_t mach_vm_remap(
  vm_map_t target_task,
  mach_vm_address_t *target_address,
  mach_vm_size_t size,
  mach_vm_offset_t mask,
  int flags,
  vm_map_t src_task,
  mach_vm_address_t src_address,
  boolean_t copy,
  vm_prot_t *cur_protection,
  vm_prot_t *max_protection,
  vm_inherit_t inheritance
);

} // {extern "C"}
#endif

ASMJIT_BEGIN_SUB_NAMESPACE(VirtMem)

// Virtual Memory Utilities
// ========================

[[maybe_unused]]
static const constexpr MemoryFlags dual_mapping_filter[2] = {
  MemoryFlags::kAccessWrite | MemoryFlags::kMMapMaxAccessWrite,
  MemoryFlags::kAccessExecute | MemoryFlags::kMMapMaxAccessExecute
};

// Virtual Memory [Windows]
// ========================

#if defined(_WIN32)

struct ScopedHandle {
  inline ScopedHandle() noexcept
    : value(nullptr) {}

  inline ~ScopedHandle() noexcept {
    if (value != nullptr) {
      ::CloseHandle(value);
    }
  }

  HANDLE value;
};

static void detect_vm_info(Info& vm_info) noexcept {
  SYSTEM_INFO system_info;

  ::GetSystemInfo(&system_info);
  vm_info.page_size = Support::align_up_power_of_2<uint32_t>(system_info.dwPageSize);
  vm_info.page_granularity = system_info.dwAllocationGranularity;
}

static size_t detect_large_page_size() noexcept {
  return ::GetLargePageMinimum();
}

static bool has_dual_mapping_support() noexcept {
  // TODO: This assumption works on X86 platforms, this may not work on AArch64.
  return true;
}

// Returns windows-specific protect_flags from \ref MemoryFlags.
static DWORD protect_flags_from_memory_flags(MemoryFlags memory_flags) noexcept {
  DWORD protect_flags;

  // READ|WRITE|EXECUTE.
  if (Support::test(memory_flags, MemoryFlags::kAccessExecute)) {
    protect_flags = Support::test(memory_flags, MemoryFlags::kAccessWrite) ? PAGE_EXECUTE_READWRITE : PAGE_EXECUTE_READ;
  }
  else if (Support::test(memory_flags, MemoryFlags::kAccessRW)) {
    protect_flags = Support::test(memory_flags, MemoryFlags::kAccessWrite) ? PAGE_READWRITE : PAGE_READONLY;
  }
  else {
    protect_flags = PAGE_NOACCESS;
  }

  // Any other flags to consider?
  return protect_flags;
}

static DWORD desired_access_from_memory_flags(MemoryFlags memory_flags) noexcept {
  DWORD access = Support::test(memory_flags, MemoryFlags::kAccessWrite) ? FILE_MAP_WRITE : FILE_MAP_READ;
  if (Support::test(memory_flags, MemoryFlags::kAccessExecute)) {
    access |= FILE_MAP_EXECUTE;
  }
  return access;
}

static HardenedRuntimeFlags get_hardened_runtime_flags() noexcept {
  HardenedRuntimeFlags flags = HardenedRuntimeFlags::kNone;

  if (has_dual_mapping_support()) {
    flags |= HardenedRuntimeFlags::kDualMapping;
  }

  return flags;
}

Error alloc(void** p, size_t size, MemoryFlags memory_flags) noexcept {
  *p = nullptr;

  if (size == 0) {
    return make_error(Error::kInvalidArgument);
  }

  DWORD allocation_type = MEM_COMMIT | MEM_RESERVE;
  DWORD protect_flags = protect_flags_from_memory_flags(memory_flags);

  if (Support::test(memory_flags, MemoryFlags::kMMapLargePages)) {
    size_t lp_size = large_page_size();

    // Does it make sense to call VirtualAlloc() if we failed to query large page size?
    if (lp_size == 0) {
      return make_error(Error::kFeatureNotEnabled);
    }

    if (!Support::is_aligned(size, lp_size)) {
      return make_error(Error::kInvalidArgument);
    }

    allocation_type |= MEM_LARGE_PAGES;
  }

  void* result = ::VirtualAlloc(nullptr, size, allocation_type, protect_flags);
  if (!result) {
    return make_error(Error::kOutOfMemory);
  }

  *p = result;
  return Error::kOk;
}

Error release(void* p, size_t size) noexcept {
  Support::maybe_unused(size);

  // NOTE: If the `dw_free_type` parameter is MEM_RELEASE, `size` parameter must be zero.
  constexpr DWORD dw_free_type = MEM_RELEASE;

  if (ASMJIT_UNLIKELY(!::VirtualFree(p, 0, dw_free_type))) {
    return make_error(Error::kInvalidArgument);

  }
  return Error::kOk;
}

Error protect(void* p, size_t size, MemoryFlags memory_flags) noexcept {
  DWORD protect_flags = protect_flags_from_memory_flags(memory_flags);
  DWORD old_flags;

  if (::VirtualProtect(p, size, protect_flags, &old_flags)) {
    return Error::kOk;
  }

  return make_error(Error::kInvalidArgument);
}

Error alloc_dual_mapping(Out<DualMapping> dm, size_t size, MemoryFlags memory_flags) noexcept {
  dm->rx = nullptr;
  dm->rw = nullptr;

  if (size == 0) {
    return make_error(Error::kInvalidArgument);
  }

  ScopedHandle handle;
  handle.value = ::CreateFileMappingW(
    INVALID_HANDLE_VALUE,
    nullptr,
    PAGE_EXECUTE_READWRITE,
    (DWORD)(uint64_t(size) >> 32),
    (DWORD)(size & 0xFFFFFFFFu),
    nullptr);

  if (ASMJIT_UNLIKELY(!handle.value)) {
    return make_error(Error::kOutOfMemory);
  }

  void* ptr[2];
  for (uint32_t i = 0; i < 2; i++) {
    MemoryFlags access_flags = memory_flags & ~dual_mapping_filter[i];
    DWORD desired_access = desired_access_from_memory_flags(access_flags);
    ptr[i] = ::MapViewOfFile(handle.value, desired_access, 0, 0, size);

    if (ptr[i] == nullptr) {
      if (i == 1u) {
        ::UnmapViewOfFile(ptr[0]);
      }
      return make_error(Error::kOutOfMemory);
    }
  }

  dm->rx = ptr[0];
  dm->rw = ptr[1];
  return Error::kOk;
}

Error release_dual_mapping(DualMapping& dm, size_t size) noexcept {
  Support::maybe_unused(size);
  bool failed = false;

  if (!::UnmapViewOfFile(dm.rx)) {
    failed = true;
  }

  if (dm.rx != dm.rw && !UnmapViewOfFile(dm.rw)) {
    failed = true;
  }

  if (failed) {
    return make_error(Error::kInvalidArgument);
  }

  dm.rx = nullptr;
  dm.rw = nullptr;
  return Error::kOk;
}

#endif

// Virtual Memory [Unix]
// =====================

#if !defined(_WIN32)

// Virtual Memory [Unix] - Utilities
// =================================

#if defined(__linux__) || (defined(__APPLE__) && TARGET_OS_OSX)
struct KernelVersion {
  long ver[2];

  inline long major() const noexcept { return ver[0]; }
  inline long minor() const noexcept { return ver[1]; }

  inline bool eq(long major, long minor) const noexcept { return ver[0] == major && ver[1] == minor; }
  inline bool ge(long major, long minor) const noexcept { return ver[0] > major || (ver[0] == major && ver[1] >= minor); }
};

[[maybe_unused]]
static KernelVersion get_kernel_version() noexcept {
  KernelVersion out {};
  struct utsname buf {};

  uname(&buf);

  size_t i = 0;
  char* p = buf.release;

  while (*p && i < 2u) {
    uint8_t c = uint8_t(*p);
    if (c >= uint8_t('0') && c <= uint8_t('9')) {
      out.ver[i] = strtol(p, &p, 10);
      i++;
      continue;
    }

    p++;
  }

  return out;
}
#endif // get_kernel_version

// Translates libc errors specific to VirtualMemory mapping to `asmjit::Error`.
[[maybe_unused]]
static Error asmjit_error_from_errno(int e) noexcept {
  switch (e) {
    case EACCES:
    case EAGAIN:
    case ENODEV:
    case EPERM:
      return Error::kInvalidState;

    case EFBIG:
    case ENOMEM:
    case EOVERFLOW:
      return Error::kOutOfMemory;

    case EMFILE:
    case ENFILE:
      return Error::kTooManyHandles;

    default:
      return Error::kInvalidArgument;
  }
}

[[maybe_unused]]
static MemoryFlags max_access_flags_to_regular_access_flags(MemoryFlags memory_flags) noexcept {
  static constexpr uint32_t kMaxProtShift = Support::ctz_const<MemoryFlags::kMMapMaxAccessRead>;
  return MemoryFlags(uint32_t(memory_flags & MemoryFlags::kMMapMaxAccessRWX) >> kMaxProtShift);
}

[[maybe_unused]]
static MemoryFlags regular_access_flags_to_max_access_flags(MemoryFlags memory_flags) noexcept {
  static constexpr uint32_t kMaxProtShift = Support::ctz_const<MemoryFlags::kMMapMaxAccessRead>;
  return MemoryFlags(uint32_t(memory_flags & MemoryFlags::kAccessRWX) << kMaxProtShift);
}

// Returns `mmap()` protection flags from \ref MemoryFlags.
[[maybe_unused]]
static int mm_prot_from_memory_flags(MemoryFlags memory_flags) noexcept {
  int protection = 0;
  if (Support::test(memory_flags, MemoryFlags::kAccessRead)) protection |= PROT_READ;
  if (Support::test(memory_flags, MemoryFlags::kAccessWrite)) protection |= PROT_READ | PROT_WRITE;
  if (Support::test(memory_flags, MemoryFlags::kAccessExecute)) protection |= PROT_READ | PROT_EXEC;
  return protection;
}

// Returns maximum protection flags from `memory_flags`.
//
// Uses:
//   - `PROT_MPROTECT()` on NetBSD.
//   - `PROT_MAX()` when available on other BSDs.
[[maybe_unused]]
static inline int mm_max_prot_from_memory_flags(MemoryFlags memory_flags) noexcept {
  MemoryFlags acc = max_access_flags_to_regular_access_flags(memory_flags);
  if (acc != MemoryFlags::kNone) {
#if defined(__NetBSD__) && defined(PROT_MPROTECT)
    return PROT_MPROTECT(mm_prot_from_memory_flags(acc));
#elif defined(PROT_MAX)
    return PROT_MAX(mm_prot_from_memory_flags(acc));
#else
    return 0;
#endif
  }

  return 0;
}

static void detect_vm_info(Info& vm_info) noexcept {
  uint32_t page_size = uint32_t(::getpagesize());

  vm_info.page_size = page_size;
  vm_info.page_granularity = Support::max<uint32_t>(page_size, 65536);
}

static size_t detect_large_page_size() noexcept {
#if defined(__APPLE__) && defined(VM_FLAGS_SUPERPAGE_SIZE_2MB) && ASMJIT_ARCH_X86
  return 2u * 1024u * 1024u;
#elif defined(__FreeBSD__)
  Support::Array<size_t, 2> page_size;
  // TODO: Does it return unsigned?
  return (getpagesizes(page_size.data(), 2) < 2) ? 0 : uint32_t(page_size[1]);
#elif defined(__linux__)
  StringTmp<128> storage;

  if (OSUtils::read_file("/sys/kernel/mm/transparent_hugepage/hpage_pmd_size", storage, 16) != Error::kOk || storage.is_empty()) {
    return 0u;
  }

  // The first value should be the size of the page (hpage_pmd_size).
  size_t large_page_size = 0;

  const char* buf = storage.data();
  size_t buf_size = storage.size();

  for (size_t i = 0; i < buf_size; i++) {
    uint32_t digit = uint32_t(uint8_t(buf[i]) - uint8_t('0'));
    if (digit >= 10u) {
      break;
    }
    large_page_size = large_page_size * 10 + digit;
  }

  if (Support::is_power_of_2(large_page_size))
    return large_page_size;
  else
    return 0u;
#else
  return 0u;
#endif
}

// Virtual Memory [Posix] - Anonymous Memory
// =========================================

#if defined(ASMJIT_ANONYMOUS_MEMORY_USE_FD)

// Some operating systems don't allow /dev/shm to be executable. On Linux this happens when /dev/shm is mounted with
// 'noexec', which is enforced by systemd. Other operating systems like MacOS also restrict executable permissions
// regarding /dev/shm, so we use a runtime detection before attempting to allocate executable memory. Sometimes we
// don't need the detection as we know it would always result in `AnonymousMemoryStrategy::kTmpDir`.
enum class AnonymousMemoryStrategy : uint32_t {
  kUnknown = 0,
  kDevShm = 1,
  kTmpDir = 2
};

#if !defined(SHM_ANON)
static const char* get_tmp_dir() noexcept {
  const char* tmp_dir = getenv("TMPDIR");
  return tmp_dir ? tmp_dir : "/tmp";
}
#endif

#if defined(__linux__) && defined(__NR_memfd_create)
static uint32_t get_mfd_exec_flag() noexcept {
  static std::atomic<uint32_t> cached_mfd_exec_supported;
  uint32_t val = cached_mfd_exec_supported.load();

  if (val == 0u) {
    KernelVersion ver = get_kernel_version();
    val = uint32_t(ver.ge(6, 3)) + 1u;
    cached_mfd_exec_supported.store(val);
  }

  return val == 2u ? uint32_t(MFD_EXEC) : uint32_t(0u);
}
#endif // __linux__ && __NR_memfd_create


// It's not fully random, just to avoid collisions when opening TMP or SHM file.
[[maybe_unused]]
static uint64_t generate_random_bits(uintptr_t stack_ptr, uint32_t attempt) noexcept {
  static std::atomic<uint32_t> internal_counter;

#if defined(__GNUC__) && ASMJIT_ARCH_X86
  // Use RDTSC instruction to avoid gettimeofday() as we just need some "random" bits.
  uint64_t mix = __builtin_ia32_rdtsc();
#else
  struct timeval tm {};
  uint64_t mix = 1; // only used when gettimeofday() fails, which is unlikely.
  if (gettimeofday(&tm, nullptr) == 0) {
    mix = uint64_t(tm.tv_usec) ^ uint64_t(tm.tv_sec);
  }
#endif

  uint64_t bits = (uint64_t(stack_ptr) & 0x1010505000055590u) - mix * 773703683;
  bits = (bits >> 33) ^ (bits << 7) ^ (attempt * 87178291199);
  return bits + uint64_t(++internal_counter) * 10619863;
}

class AnonymousMemory {
public:
  enum FileType : uint32_t {
    kFileTypeNone,
    kFileTypeShm,
    kFileTypeTmp
  };

  int _fd;
  FileType _file_type;
  StringTmp<128> _tmp_name;

  inline AnonymousMemory() noexcept
    : _fd(-1),
      _file_type(kFileTypeNone),
      _tmp_name() {}

  inline ~AnonymousMemory() noexcept {
    unlink();
    close();
  }

  inline int fd() const noexcept { return _fd; }

  Error open(bool prefer_tmp_over_dev_shm) noexcept {
#if defined(__linux__) && defined(__NR_memfd_create)
    // Linux specific 'memfd_create' - if the syscall returns `ENOSYS` it means
    // it's not available and we will never call it again (would be pointless).
    //
    // NOTE: There is also memfd_create() libc function in FreeBSD, but it internally
    // uses `shm_open(SHM_ANON, ...)` so it's not needed to add support for it (it's
    // not a syscall as in Linux).

    // Zero initialized, if ever changed to '1' that would mean the syscall is not
    // available and we must use `shm_open()` and `shm_unlink()` (or regular `open()`).
    static volatile uint32_t memfd_create_not_supported;

    if (!memfd_create_not_supported) {
      _fd = (int)syscall(__NR_memfd_create, "vmem", MFD_CLOEXEC | get_mfd_exec_flag());
      if (ASMJIT_LIKELY(_fd >= 0)) {
        return Error::kOk;
      }

      int e = errno;
      if (e == ENOSYS) {
        memfd_create_not_supported = 1;
      }
      else {
        return make_error(asmjit_error_from_errno(e));
      }
    }
#endif // __linux__ && __NR_memfd_create

#if defined(ASMJIT_HAS_SHM_OPEN) && defined(SHM_ANON)
    // Originally FreeBSD extension, apparently works in other BSDs too.
    Support::maybe_unused(prefer_tmp_over_dev_shm);
    _fd = ::shm_open(SHM_ANON, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);

    if (ASMJIT_LIKELY(_fd >= 0)) {
      return Error::kOk;
    }
    else {
      return make_error(asmjit_error_from_errno(errno));
    }
#else
    // POSIX API. We have to generate somehow a unique name, so use `generate_random_bits()` helper. To prevent
    // having file collisions we use `shm_open()` with flags that require creation of the file so we never open
    // an existing shared memory.
    static const char shm_format_string[] = "/shm-id-%016llX";
    uint32_t retry_count = 100;

    for (uint32_t i = 0; i < retry_count; i++) {
      bool use_tmp = !ASMJIT_VM_SHM_DETECT || prefer_tmp_over_dev_shm;
      uint64_t bits = generate_random_bits((uintptr_t)this, i);

      if (use_tmp) {
        _tmp_name.assign(get_tmp_dir());
        _tmp_name.append_format(shm_format_string, (unsigned long long)bits);
        _fd = ASMJIT_FILE64_API(::open)(_tmp_name.data(), O_RDWR | O_CREAT | O_EXCL, 0);
        if (ASMJIT_LIKELY(_fd >= 0)) {
          _file_type = kFileTypeTmp;
          return Error::kOk;
        }
      }
#if defined(ASMJIT_HAS_SHM_OPEN)
      else {
        _tmp_name.assign_format(shm_format_string, (unsigned long long)bits);
        _fd = ::shm_open(_tmp_name.data(), O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
        if (ASMJIT_LIKELY(_fd >= 0)) {
          _file_type = kFileTypeShm;
          return Error::kOk;
        }
      }
#endif

      int e = errno;
      if (e != EEXIST) {
        return make_error(asmjit_error_from_errno(e));
      }
    }

    return make_error(Error::kFailedToOpenAnonymousMemory);
#endif
  }

  void unlink() noexcept {
    FileType type = _file_type;
    _file_type = kFileTypeNone;

#ifdef ASMJIT_HAS_SHM_OPEN
    if (type == kFileTypeShm) {
      ::shm_unlink(_tmp_name.data());
      return;
    }
#endif

    if (type == kFileTypeTmp) {
      ::unlink(_tmp_name.data());
      return;
    }
  }

  void close() noexcept {
    if (_fd >= 0) {
      ::close(_fd);
      _fd = -1;
    }
  }

  Error allocate(size_t size) noexcept {
    // TODO: Improve this by using `posix_fallocate()` when available.
    if (ASMJIT_FILE64_API(ftruncate)(_fd, off_t(size)) != 0) {
      return make_error(asmjit_error_from_errno(errno));
    }

    return Error::kOk;
  }
};

#if ASMJIT_VM_SHM_DETECT
static Error detect_anonymous_memory_strategy(Out<AnonymousMemoryStrategy> strategy_out) noexcept {
  AnonymousMemory anon_mem;
  Info vm_info = info();

  ASMJIT_PROPAGATE(anon_mem.open(false));
  ASMJIT_PROPAGATE(anon_mem.allocate(vm_info.page_size));

  void* ptr = mmap(nullptr, vm_info.page_size, PROT_READ | PROT_EXEC, MAP_SHARED, anon_mem.fd(), 0);
  if (ptr == MAP_FAILED) {
    int e = errno;
    if (e == EINVAL) {
      *strategy_out = AnonymousMemoryStrategy::kTmpDir;
      return Error::kOk;
    }
    return make_error(asmjit_error_from_errno(e));
  }
  else {
    munmap(ptr, vm_info.page_size);
    *strategy_out = AnonymousMemoryStrategy::kDevShm;
    return Error::kOk;
  }
}
#endif

static Error get_anonymous_memory_strategy(AnonymousMemoryStrategy* strategy_out) noexcept {
#if ASMJIT_VM_SHM_DETECT
  // Initially don't assume anything. It has to be tested whether '/dev/shm' was mounted with 'noexec' flag or not.
  static std::atomic<uint32_t> cached_strategy;

  AnonymousMemoryStrategy strategy = static_cast<AnonymousMemoryStrategy>(cached_strategy.load());
  if (strategy == AnonymousMemoryStrategy::kUnknown) {
    ASMJIT_PROPAGATE(detect_anonymous_memory_strategy(Out(strategy)));
    cached_strategy.store(static_cast<uint32_t>(strategy));
  }

  *strategy_out = strategy;
  return Error::kOk;
#else
  *strategy_out = AnonymousMemoryStrategy::kTmpDir;
  return Error::kOk;
#endif
}

#endif // ASMJIT_ANONYMOUS_MEMORY_USE_FD

// Virtual Memory [Posix] - Hardened Runtime & MAP_JIT
// ===================================================

// Detects whether the current process is hardened, which means that pages that have WRITE and EXECUTABLE flags
// cannot be normally allocated. On OSX + AArch64 such allocation requires MAP_JIT flag, other platforms don't
// support this combination.
static bool has_hardened_runtime() noexcept {
#if defined(__APPLE__) && TARGET_OS_OSX && ASMJIT_ARCH_ARM >= 64
  // OSX on AArch64 has always hardened runtime enabled.
  return true;
#else
  static std::atomic<uint32_t> cached_hardened_flag;

  constexpr uint32_t hf_unknown  = 0;
  constexpr uint32_t hf_disabled = 1;
  constexpr uint32_t hf_enabled  = 2;

  uint32_t flag = cached_hardened_flag.load();
  if (flag == hf_unknown) {
    size_t page_size = size_t(::getpagesize());
    void* ptr = mmap(nullptr, page_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (ptr == MAP_FAILED) {
      flag = hf_enabled;
    }
    else {
      flag = hf_disabled;
      munmap(ptr, page_size);
    }

    cached_hardened_flag.store(flag);
  }

  return flag == hf_enabled;
#endif
}

// Detects whether MAP_JIT is available.
static inline bool has_mapjit_support() noexcept {
#if defined(__APPLE__) && TARGET_OS_OSX && ASMJIT_ARCH_X86 == 0
  // Apple platforms always use hardened runtime + MAP_JIT on non-x86 hardware:
  //   - https://developer.apple.com/documentation/apple_silicon/porting_just-in-time_compilers_to_apple_silicon
  return true;
#elif defined(__APPLE__) && TARGET_OS_OSX
  // MAP_JIT flag required to run unsigned JIT code is only supported by kernel version 10.14+ (Mojave).
  static std::atomic<uint32_t> cached_mapjit_support;
  uint32_t val = cached_mapjit_support.load();

  if (val == 0u) {
    KernelVersion ver = get_kernel_version();
    val = uint32_t(ver.ge(18, 0)) + 1u;
    cached_mapjit_support.store(val);
  }

  return val == 2u;
#else
  // MAP_JIT is not available (it's only available on OSX).
  return false;
#endif
}

// Returns either MAP_JIT or 0 based on `memory_flags` and the host operating system.
static inline int mm_mapjit_from_memory_flags(MemoryFlags memory_flags) noexcept {
#if defined(__APPLE__)
  // Always use MAP_JIT flag if user asked for it (could be used for testing on non-hardened processes) and detect
  // whether it must be used when the process is actually hardened (in that case it doesn't make sense to rely on
  // user `memory_flags`).
  //
  // MAP_JIT is not required when dual-mapping memory and is incompatible with MAP_SHARED, so it will not be
  // added when the latter is enabled.
  bool use_mapjit = (Support::test(memory_flags, MemoryFlags::kMMapEnableMapJit) || has_hardened_runtime()) &&
                    !Support::test(memory_flags, MemoryFlags::kMapShared);
  if (use_mapjit) {
    return has_mapjit_support() ? int(MAP_JIT) : 0;
  }
  else {
    return 0;
  }
#else
  Support::maybe_unused(memory_flags);
  return 0;
#endif
}

static inline bool has_dual_mapping_support() noexcept {
#if defined(ASMJIT_NO_DUAL_MAPPING)
  return false;
#else
  return true;
#endif
}

static HardenedRuntimeFlags get_hardened_runtime_flags() noexcept {
  HardenedRuntimeFlags flags = HardenedRuntimeFlags::kNone;

  if (has_hardened_runtime()) {
    flags |= HardenedRuntimeFlags::kEnabled;
  }

  if (has_mapjit_support()) {
    flags |= HardenedRuntimeFlags::kMapJit;
  }

  if (has_dual_mapping_support()) {
    flags |= HardenedRuntimeFlags::kDualMapping;
  }

  return flags;
}

static Error map_memory(void** p, size_t size, MemoryFlags memory_flags, int fd = -1, off_t offset = 0) noexcept {
  *p = nullptr;

  if (size == 0) {
    return make_error(Error::kInvalidArgument);
  }

  int protection = mm_prot_from_memory_flags(memory_flags) | mm_max_prot_from_memory_flags(memory_flags);
  int mm_flags = mm_mapjit_from_memory_flags(memory_flags);

  mm_flags |= Support::test(memory_flags, MemoryFlags::kMapShared) ? MAP_SHARED : MAP_PRIVATE;
  if (fd == -1) {
    mm_flags |= MAP_ANONYMOUS;
  }

  bool use_large_pages = Support::test(memory_flags, VirtMem::MemoryFlags::kMMapLargePages);

  if (use_large_pages) {
#if defined(__linux__)
    size_t lp_size = large_page_size();
    if (lp_size == 0) {
      return make_error(Error::kFeatureNotEnabled);
    }

    if (!Support::is_aligned(size, lp_size)) {
      return make_error(Error::kInvalidArgument);
    }

    unsigned lp_size_log2 = Support::ctz(lp_size);
    mm_flags |= int(unsigned(MAP_HUGETLB) | (lp_size_log2 << MAP_HUGE_SHIFT));
#else
    return make_error(Error::kFeatureNotEnabled);
#endif // __linux__
  }

  void* ptr = mmap(nullptr, size, protection, mm_flags, fd, offset);
  if (ptr == MAP_FAILED) {
    return make_error(asmjit_error_from_errno(errno));
  }

#if defined(MADV_HUGEPAGE)
  if (use_large_pages) {
    madvise(ptr, size, MADV_HUGEPAGE);
  }
#endif

  *p = ptr;
  return Error::kOk;
}

static Error unmap_memory(void* p, size_t size) noexcept {
  if (ASMJIT_UNLIKELY(munmap(p, size) != 0)) {
    return make_error(asmjit_error_from_errno(errno));
  }

  return Error::kOk;
}

Error alloc(void** p, size_t size, MemoryFlags memory_flags) noexcept {
  return map_memory(p, size, memory_flags);
}

Error release(void* p, size_t size) noexcept {
  return unmap_memory(p, size);
}

Error protect(void* p, size_t size, MemoryFlags memory_flags) noexcept {
  int protection = mm_prot_from_memory_flags(memory_flags);
  if (mprotect(p, size, protection) == 0) {
    return Error::kOk;
  }
  return make_error(asmjit_error_from_errno(errno));
}

// Virtual Memory [Posix] - Dual Mapping
// =====================================

#if !defined(ASMJIT_NO_DUAL_MAPPING)
static Error unmap_dual_mapping(DualMapping& dm, size_t size) noexcept {
  Error err1 = unmap_memory(dm.rx, size);
  Error err2 = Error::kOk;

  if (dm.rx != dm.rw) {
    err2 = unmap_memory(dm.rw, size);
  }

  // We can report only one error, so report the first...
  if (err1 != Error::kOk || err2 != Error::kOk) {
    return make_error(err1 != Error::kOk ? err1 : err2);
  }

  dm.rx = nullptr;
  dm.rw = nullptr;
  return Error::kOk;
}
#endif // !ASMJIT_NO_DUAL_MAPPING

#if defined(ASMJIT_ANONYMOUS_MEMORY_USE_REMAPDUP)
static Error alloc_dual_mapping_via_remapdup(Out<DualMapping> dm_out, size_t size, MemoryFlags memory_flags) noexcept {
  MemoryFlags max_access_flags = regular_access_flags_to_max_access_flags(memory_flags);
  MemoryFlags final_flags = memory_flags | max_access_flags | MemoryFlags::kMapShared;

  MemoryFlags rx_flags = final_flags & ~(MemoryFlags::kAccessWrite | MemoryFlags::kMMapMaxAccessWrite);
  MemoryFlags rw_flags = final_flags & ~(MemoryFlags::kAccessExecute);

  // Allocate RW mapping.
  DualMapping dm {};
  ASMJIT_PROPAGATE(map_memory(&dm.rw, size, rw_flags));

  // Allocate RX mapping.
  dm.rx = mremap(dm.rw, size, nullptr, size, MAP_REMAPDUP);
  if (dm.rx == MAP_FAILED) {
    int e = errno;
    munmap(dm.rw, size);
    return make_error(asmjit_error_from_errno(e));
  }

  if (mprotect(dm.rx, size, mm_prot_from_memory_flags(rx_flags)) != 0) {
    int e = errno;
    unmap_dual_mapping(dm, size);
    return make_error(asmjit_error_from_errno(e));
  }

  *dm_out = dm;
  return Error::kOk;
}
#endif

#if defined(ASMJIT_ANONYMOUS_MEMORY_USE_MACH_VM_REMAP)
static Error asmjit_error_from_kern_result(kern_return_t result) noexcept {
  switch (result) {
    case KERN_PROTECTION_FAILURE:
      return make_error(Error::kProtectionFailure);
    case KERN_NO_SPACE:
      return make_error(Error::kOutOfMemory);
    case KERN_INVALID_ARGUMENT:
      return make_error(Error::kInvalidArgument);
    default:
      return make_error(Error::kInvalidState);
  }
}

static Error alloc_dual_mapping_using_mach_vm_remap(Out<DualMapping> dm_out, size_t size, MemoryFlags memory_flags) noexcept {
  DualMapping dm {};
  MemoryFlags mmap_flags = MemoryFlags::kAccessReadWrite | (memory_flags & MemoryFlags::kMapShared);

  ASMJIT_PROPAGATE(map_memory(&dm.rx, size, mmap_flags));

  vm_prot_t cur_prot;
  vm_prot_t max_prot;

  int rw_protect_flags = VM_PROT_READ | VM_PROT_WRITE;
  int rx_protect_flags = VM_PROT_READ;

  if (Support::test(memory_flags, MemoryFlags::kAccessExecute)) {
    rx_protect_flags |= VM_PROT_EXECUTE;
  }

  kern_return_t result {};
  do {
    vm_map_t task = mach_task_self();
    mach_vm_address_t remapped_addr {};

#if defined(VM_FLAGS_RANDOM_ADDR)
    int remap_flags = VM_FLAGS_ANYWHERE | VM_FLAGS_RANDOM_ADDR;
#else
    int remap_flags = VM_FLAGS_ANYWHERE;
#endif

    // Try to remap the existing memory into a different address.
    result = mach_vm_remap(
      task,                       // target_task
      &remapped_addr,             // target_address
      size,                       // size
      0,                          // mask
      remap_flags,                // flags
      task,                       // src_task
      (mach_vm_address_t)dm.rx,   // src_address
      false,                      // copy
      &cur_prot,                  // cur_protection
      &max_prot,                  // max_protection
      VM_INHERIT_DEFAULT);        // inheritance

    if (result != KERN_SUCCESS) {
      break;
    }

    dm.rw = (void*)remapped_addr;

    // Now, try to change permissions of both map regions into RW and RX. The vm_protect()
    // API is used twice as we also want to set maximum permissions, so nobody would be
    // allowed to change the RX region back to RW or RWX (if RWX is allowed).
    uint32_t i;
    for (i = 0; i < 2; i++) {
      bool set_maximum = (i == 0);

      result = vm_protect(
        task,                       // target_task
        (vm_address_t)dm.rx,        // address
        size,                       // size
        set_maximum,                // set_maximum
        rx_protect_flags);          // new_protection

      if (result != KERN_SUCCESS) {
        break;
      }

      result = vm_protect(task,     // target_task
        (vm_address_t)dm.rw,        // address
        size,                       // size
        set_maximum,                // set_maximum
        rw_protect_flags);          // new_protection

      if (result != KERN_SUCCESS) {
        break;
      }
    }
  } while (0);

  if (result != KERN_SUCCESS) {
    unmap_dual_mapping(dm, size);
    return make_error(asmjit_error_from_kern_result(result));
  }

  *dm_out = dm;
  return Error::kOk;
}
#endif // ASMJIT_ANONYMOUS_MEMORY_USE_MACH_VM_REMAP

#if defined(ASMJIT_ANONYMOUS_MEMORY_USE_FD)
static Error alloc_dual_mapping_using_file(Out<DualMapping> dm, size_t size, MemoryFlags memory_flags) noexcept {
  bool prefer_tmp_over_dev_shm = Support::test(memory_flags, MemoryFlags::kMappingPreferTmp);
  if (!prefer_tmp_over_dev_shm) {
    AnonymousMemoryStrategy strategy;
    ASMJIT_PROPAGATE(get_anonymous_memory_strategy(&strategy));
    prefer_tmp_over_dev_shm = (strategy == AnonymousMemoryStrategy::kTmpDir);
  }

  AnonymousMemory anon_mem;
  ASMJIT_PROPAGATE(anon_mem.open(prefer_tmp_over_dev_shm));
  ASMJIT_PROPAGATE(anon_mem.allocate(size));

  void* ptr[2];
  for (uint32_t i = 0; i < 2; i++) {
    MemoryFlags restricted_memory_flags = memory_flags & ~dual_mapping_filter[i];
    Error err = map_memory(&ptr[i], size, restricted_memory_flags | MemoryFlags::kMapShared, anon_mem.fd(), 0);
    if (err != Error::kOk) {
      if (i == 1) {
        unmap_memory(ptr[0], size);
      }
      return err;
    }
  }

  dm->rx = ptr[0];
  dm->rw = ptr[1];
  return Error::kOk;
}
#endif // ASMJIT_ANONYMOUS_MEMORY_USE_FD

Error alloc_dual_mapping(Out<DualMapping> dm, size_t size, MemoryFlags memory_flags) noexcept {
  dm = DualMapping{};

#if defined(ASMJIT_NO_DUAL_MAPPING)
  Support::maybe_unused(size, memory_flags);
  return make_error(Error::kFeatureNotEnabled);
#else
  if (off_t(size) <= 0) {
    return make_error(size == 0 ? Error::kInvalidArgument : Error::kTooLarge);
  }

#if defined(ASMJIT_ANONYMOUS_MEMORY_USE_REMAPDUP)
  return alloc_dual_mapping_via_remapdup(dm, size, memory_flags);
#elif defined(ASMJIT_ANONYMOUS_MEMORY_USE_MACH_VM_REMAP)
  return alloc_dual_mapping_using_mach_vm_remap(dm, size, memory_flags);
#elif defined(ASMJIT_ANONYMOUS_MEMORY_USE_FD)
  return alloc_dual_mapping_using_file(dm, size, memory_flags);
#else
  #error "[asmjit] VirtMem::alloc_dual_mapping() doesn't have implementation for the target OS or architecture"
#endif
#endif // ASMJIT_NO_DUAL_MAPPING
}

Error release_dual_mapping(DualMapping& dm, size_t size) noexcept {
#if defined(ASMJIT_NO_DUAL_MAPPING)
  Support::maybe_unused(dm, size);
  return make_error(Error::kFeatureNotEnabled);
#else
  return unmap_dual_mapping(dm, size);
#endif // ASMJIT_NO_DUAL_MAPPING
}
#endif

// Virtual Memory - Flush Instruction Cache
// ========================================

void flush_instruction_cache(void* p, size_t size) noexcept {
#if ASMJIT_ARCH_X86 || defined(__EMSCRIPTEN__)
  // X86|X86_64 architecture doesn't require to do anything to flush instruction cache.
  Support::maybe_unused(p, size);
#elif defined(__APPLE__)
  sys_icache_invalidate(p, size);
#elif defined(_WIN32)
  // Windows has a built-in support in `kernel32.dll`.
  FlushInstructionCache(GetCurrentProcess(), p, size);
#elif defined(__GNUC__)
  char* start = static_cast<char*>(p);
  char* end = start + size;
  __builtin___clear_cache(start, end);
#else
  #pragma message("[asmjit] VirtMem::flush_instruction_cache() doesn't have implementation for the target OS and compiler")
  Support::maybe_unused(p, size);
#endif
}

// Virtual Memory - Memory Info
// ============================

Info info() noexcept {
  static std::atomic<uint32_t> vm_info_initialized;
  static Info vm_info;

  if (!vm_info_initialized.load()) {
    Info local_mem_info;
    detect_vm_info(local_mem_info);

    vm_info = local_mem_info;
    vm_info_initialized.store(1u);
  }

  return vm_info;
}

size_t large_page_size() noexcept {
  static std::atomic<size_t> large_page_size;
  static constexpr size_t not_available = 1;

  size_t size = large_page_size.load();
  if (ASMJIT_LIKELY(size > not_available)) {
    return size;
  }

  if (size == not_available) {
    return 0;
  }

  size = detect_large_page_size();
  large_page_size.store(size != 0 ? size : not_available);
  return size;
}

// Virtual Memory - Hardened Runtime Info
// ======================================

HardenedRuntimeInfo hardened_runtime_info() noexcept {
  return HardenedRuntimeInfo { get_hardened_runtime_flags() };
}

// Virtual Memory - Project JIT Memory
// ===================================

void protect_jit_memory(ProtectJitAccess access) noexcept {
#if defined(ASMJIT_HAS_PTHREAD_JIT_WRITE_PROTECT_NP)
  pthread_jit_write_protect_np(static_cast<int>(access));
#else
  Support::maybe_unused(access);
#endif
}

ASMJIT_END_SUB_NAMESPACE

// Virtual Memory - Tests
// ======================

#if defined(ASMJIT_TEST)
ASMJIT_BEGIN_NAMESPACE

UNIT(virt_mem) {
  VirtMem::Info vm_info = VirtMem::info();

  INFO("VirtMem::info():");
  INFO("  page_size: %zu", size_t(vm_info.page_size));
  INFO("  page_granularity: %zu", size_t(vm_info.page_granularity));

  INFO("VirtMem::large_page_size():");
  INFO("  large_page_size: %zu", size_t(VirtMem::large_page_size()));

  VirtMem::HardenedRuntimeInfo hardened_rt_info = VirtMem::hardened_runtime_info();
  VirtMem::HardenedRuntimeFlags hardened_rt_flags = hardened_rt_info.flags;

  INFO("VirtMem::hardened_runtime_info():");
  INFO("  flags:");
  INFO("    kEnabled: %s"    , Support::test(hardened_rt_flags, VirtMem::HardenedRuntimeFlags::kEnabled    ) ? "true" : "false");
  INFO("    kMapJit: %s"     , Support::test(hardened_rt_flags, VirtMem::HardenedRuntimeFlags::kMapJit     ) ? "true" : "false");
  INFO("    kDualMapping: %s", Support::test(hardened_rt_flags, VirtMem::HardenedRuntimeFlags::kDualMapping) ? "true" : "false");
}

ASMJIT_END_NAMESPACE
#endif // ASMJIT_TEST

#endif // !ASMJIT_NO_JIT
