// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/runtime_p.h>
#include <blend2d/support/intops_p.h>

// PTHREAD_STACK_MIN would be defined either by <pthread.h> or <limits.h>.
#include <limits.h>
#include <stdio.h>

#if !defined(_WIN32)
  #include <pthread.h>
  #include <unistd.h>
  #include <errno.h>
#endif

#ifndef BL_BUILD_NO_JIT
  #include <asmjit/core.h>
#endif

// BLRuntime - Runtime Context
// ===========================

BLRuntimeContext bl_runtime_context;

// BLRuntime - Build Information
// =============================

static const BLRuntimeBuildInfo bl_runtime_build_info = {
  // Blend2D major version.
  (BL_VERSION >> 16),
  // Blend2D minor version.
  (BL_VERSION >> 8) & 0xFF,
  // Blend2D patch version.
  (BL_VERSION >> 0) & 0xFF,

  // Build Type.
#ifdef BL_BUILD_DEBUG
  BL_RUNTIME_BUILD_TYPE_DEBUG,
#else
  BL_RUNTIME_BUILD_TYPE_RELEASE,
#endif

  // Baseline CPU features.
  0
#if defined(BL_TARGET_OPT_SSE2)
  | BL_RUNTIME_CPU_FEATURE_X86_SSE2
#endif
#if defined(BL_TARGET_OPT_SSE3)
  | BL_RUNTIME_CPU_FEATURE_X86_SSE3
#endif
#if defined(BL_TARGET_OPT_SSSE3)
  | BL_RUNTIME_CPU_FEATURE_X86_SSSE3
#endif
#if defined(BL_TARGET_OPT_SSE4_1)
  | BL_RUNTIME_CPU_FEATURE_X86_SSE4_1
#endif
#if defined(BL_TARGET_OPT_SSE4_2)
  | BL_RUNTIME_CPU_FEATURE_X86_SSE4_2
#endif
#if defined(BL_TARGET_OPT_AVX)
  | BL_RUNTIME_CPU_FEATURE_X86_AVX
#endif
#if defined(BL_TARGET_OPT_AVX2)
  | BL_RUNTIME_CPU_FEATURE_X86_AVX2
#endif
#if defined(BL_TARGET_OPT_AVX512)
  | BL_RUNTIME_CPU_FEATURE_X86_AVX512
#endif
  ,

  // Supported CPU features.
  0
#if defined(BL_BUILD_OPT_SSE2)
  | BL_RUNTIME_CPU_FEATURE_X86_SSE2
#endif
#if defined(BL_BUILD_OPT_SSE3)
  | BL_RUNTIME_CPU_FEATURE_X86_SSE3
#endif
#if defined(BL_BUILD_OPT_SSSE3)
  | BL_RUNTIME_CPU_FEATURE_X86_SSSE3
#endif
#if defined(BL_BUILD_OPT_SSE4_1)
  | BL_RUNTIME_CPU_FEATURE_X86_SSE4_1
#endif
#if defined(BL_BUILD_OPT_SSE4_2)
  | BL_RUNTIME_CPU_FEATURE_X86_SSE4_2
#endif
#if defined(BL_BUILD_OPT_AVX)
  | BL_RUNTIME_CPU_FEATURE_X86_AVX
#endif
#if defined(BL_BUILD_OPT_AVX2)
  | BL_RUNTIME_CPU_FEATURE_X86_AVX2
#endif
#if defined(BL_BUILD_OPT_AVX512)
  | BL_RUNTIME_CPU_FEATURE_X86_AVX512
#endif
  ,

  // Maximum image size.
  BL_RUNTIME_MAX_IMAGE_SIZE,

  // Maximum thread count.
  BL_RUNTIME_MAX_THREAD_COUNT,

  // Reserved
  { 0 },

  // Compiler Info.
#if defined(__clang_minor__)
  "Clang " BL_STRINGIFY(__clang_major__) "." BL_STRINGIFY(__clang_minor__)
#elif defined(__GNUC_MINOR__)
  "GCC "  BL_STRINGIFY(__GNUC__) "." BL_STRINGIFY(__GNUC_MINOR__)
#elif defined(_MSC_VER)
  "MSC"
#else
  "Unknown"
#endif
};

// BLRuntime - System Information
// ==============================

#ifndef BL_BUILD_NO_JIT
static BL_INLINE uint32_t bl_runtime_detect_cpu_features(const asmjit::CpuInfo& asm_cpu_info) noexcept {
  uint32_t features = 0;

#if BL_TARGET_ARCH_X86
  if (asm_cpu_info.has_feature(asmjit::CpuFeatures::X86::kSSE2  )) features |= BL_RUNTIME_CPU_FEATURE_X86_SSE2;
  if (asm_cpu_info.has_feature(asmjit::CpuFeatures::X86::kSSE3  )) features |= BL_RUNTIME_CPU_FEATURE_X86_SSE3;
  if (asm_cpu_info.has_feature(asmjit::CpuFeatures::X86::kSSSE3 )) features |= BL_RUNTIME_CPU_FEATURE_X86_SSSE3;
  if (asm_cpu_info.has_feature(asmjit::CpuFeatures::X86::kSSE4_1)) features |= BL_RUNTIME_CPU_FEATURE_X86_SSE4_1;

  if (asm_cpu_info.has_feature(asmjit::CpuFeatures::X86::kSSE4_2) &&
      asm_cpu_info.has_feature(asmjit::CpuFeatures::X86::kPCLMULQDQ)) {
    features |= BL_RUNTIME_CPU_FEATURE_X86_SSE4_2;

    if (asm_cpu_info.has_feature(asmjit::CpuFeatures::X86::kAVX)) {
      features |= BL_RUNTIME_CPU_FEATURE_X86_AVX;

      if (asm_cpu_info.has_feature(asmjit::CpuFeatures::X86::kAVX2) &&
          asm_cpu_info.has_feature(asmjit::CpuFeatures::X86::kBMI) &&
          asm_cpu_info.has_feature(asmjit::CpuFeatures::X86::kBMI2) &&
          asm_cpu_info.has_feature(asmjit::CpuFeatures::X86::kPOPCNT)) {
        features |= BL_RUNTIME_CPU_FEATURE_X86_AVX2;

        if (asm_cpu_info.has_feature(asmjit::CpuFeatures::X86::kAVX512_F) &&
            asm_cpu_info.has_feature(asmjit::CpuFeatures::X86::kAVX512_BW) &&
            asm_cpu_info.has_feature(asmjit::CpuFeatures::X86::kAVX512_CD) &&
            asm_cpu_info.has_feature(asmjit::CpuFeatures::X86::kAVX512_DQ) &&
            asm_cpu_info.has_feature(asmjit::CpuFeatures::X86::kAVX512_VL)) {
          features |= BL_RUNTIME_CPU_FEATURE_X86_AVX512;
        }
      }
    }
  }
#elif BL_TARGET_ARCH_ARM
  if (asm_cpu_info.has_feature(asmjit::CpuFeatures::ARM::kCRC32)) features |= BL_RUNTIME_CPU_FEATURE_ARM_CRC32;
  if (asm_cpu_info.has_feature(asmjit::CpuFeatures::ARM::kPMULL)) features |= BL_RUNTIME_CPU_FEATURE_ARM_PMULL;
#else
  bl_unused(asm_cpu_info);
#endif

  return features;
}
#endif

static BL_INLINE void bl_runtime_init_system_info(BLRuntimeContext* rt) noexcept {
  BLRuntimeSystemInfo& info = rt->system_info;

  info.cpu_arch = BL_TARGET_ARCH_X86  ? BL_RUNTIME_CPU_ARCH_X86  :
                 BL_TARGET_ARCH_ARM  ? BL_RUNTIME_CPU_ARCH_ARM  :
                 BL_TARGET_ARCH_MIPS ? BL_RUNTIME_CPU_ARCH_MIPS : BL_RUNTIME_CPU_ARCH_UNKNOWN;

#ifndef BL_BUILD_NO_JIT
  const asmjit::CpuInfo& asm_cpu_info = asmjit::CpuInfo::host();
  info.cpu_features = bl_runtime_detect_cpu_features(asm_cpu_info);
  info.core_count = asm_cpu_info.hw_thread_count();
  info.thread_count = asm_cpu_info.hw_thread_count();
  memcpy(info.cpu_vendor, asm_cpu_info.vendor(), bl_min(sizeof(info.cpu_vendor), sizeof(asm_cpu_info._vendor)));
  memcpy(info.cpu_brand, asm_cpu_info.brand(), bl_min(sizeof(info.cpu_brand), sizeof(asm_cpu_info._brand)));
#endif

#ifdef _WIN32
  SYSTEM_INFO si;
  GetSystemInfo(&si);
  info.thread_stack_size = si.dwAllocationGranularity;
  info.allocation_granularity = si.dwAllocationGranularity;
#else
# if defined(_SC_PAGESIZE)
  info.allocation_granularity = uint32_t(sysconf(_SC_PAGESIZE));
# else
  info.allocation_granularity = uint32_t(getpagesize());
# endif

# if defined(PTHREAD_STACK_MIN)
  info.thread_stack_size = uint32_t(PTHREAD_STACK_MIN);
# elif defined(_SC_THREAD_STACK_MIN)
  info.thread_stack_size = uint32_t(sysconf(_SC_THREAD_STACK_MIN));
# else
# pragma message("Missing 'BLRuntimeSystemInfo::min_stack_size' implementation")
  info.thread_stack_size = bl_max<uint32_t>(info.allocation_granularity, 65536u);
# endif
#endif

  // NOTE: It seems that on some archs 16kB stack-size is the bare minimum even when sysconf() or PTHREAD_STACK_MIN
  // report a smaller value. Even if we don't need it we slightly increase the bare minimum to 128kB in release
  // builds and to 256kB in debug builds to make it safer especially on archs that have a big register file.
  // Additionally, modern compilers like GCC/Clang use stack slot for every variable in code in debug builds, which
  // means that heavily inlined code may need relatively large stack when compiled in debug mode - using sanitizers
  // such as ASAN makes the problem even bigger.
#if defined(BL_BUILD_DEBUG)
  constexpr uint32_t kMinStackKiB = 256;
#else
  constexpr uint32_t kMinStackKiB = 128;
#endif

  info.thread_stack_size = bl::IntOps::align_up(bl_max<uint32_t>(info.thread_stack_size, kMinStackKiB * 1024u), info.allocation_granularity);
}

static BL_INLINE void bl_runtime_init_optimization_info(BLRuntimeContext* rt) noexcept {
  // Maybe unused.
  bl_unused(rt);

#ifndef BL_BUILD_NO_JIT

#if BL_TARGET_ARCH_X86
  BLRuntimeOptimizationInfo& info = rt->optimization_info;
  const asmjit::CpuInfo& asm_cpu_info = asmjit::CpuInfo::host();

  if (asm_cpu_info.is_vendor("AMD")) {
    info.cpu_vendor = BL_RUNTIME_CPU_VENDOR_AMD;
    info.cpu_hints |= BL_RUNTIME_CPU_HINT_FAST_PSHUFB;
    info.cpu_hints |= BL_RUNTIME_CPU_HINT_FAST_PMULLD;
  }
  else if (asm_cpu_info.is_vendor("INTEL")) {
    info.cpu_vendor = BL_RUNTIME_CPU_VENDOR_INTEL;
    info.cpu_hints |= BL_RUNTIME_CPU_HINT_FAST_PSHUFB;
  }
  else if (asm_cpu_info.is_vendor("VIA")) {
    info.cpu_vendor = BL_RUNTIME_CPU_VENDOR_VIA;
    info.cpu_hints |= BL_RUNTIME_CPU_HINT_FAST_PSHUFB;
    info.cpu_hints |= BL_RUNTIME_CPU_HINT_FAST_PMULLD;
  }
  else {
    // Assume all other CPUs are okay.
    info.cpu_hints |= BL_RUNTIME_CPU_HINT_FAST_PSHUFB;
    info.cpu_hints |= BL_RUNTIME_CPU_HINT_FAST_PMULLD;
  }
#endif

#endif
}

// BLRuntime - API - Initialization & Shutdown
// ===========================================

BL_API_IMPL BLResult bl_runtime_init() noexcept {
  BLRuntimeContext* rt = &bl_runtime_context;
  if (bl_atomic_fetch_add_strong(&rt->ref_count) != 0)
    return BL_SUCCESS;

  // Initializes system information - we need this first so we can properly initialize everything that relies
  // on system or CPU features (futex, thread-pool, SIMD optimized operations, etc...).
  bl_runtime_init_system_info(rt);

  // Initialize optimization information.
  bl_runtime_init_optimization_info(rt);

  // Call "Runtime Registration" handlers - These would automatically install shutdown handlers when necessary.
  bl_fuxex_rt_init(rt);
  bl_thread_rt_init(rt);
  bl_thread_pool_rt_init(rt);
  bl_zero_allocator_rt_init(rt);

  bl_compression_rt_init(rt);
  bl_pixel_ops_rt_init(rt);
  bl_bit_array_rt_init(rt);
  bl_bit_set_rt_init(rt);
  bl_array_rt_init(rt);
  bl_string_rt_init(rt);
  bl_transform_rt_init(rt);
  bl_path_rt_init(rt);
  bl_image_rt_init(rt);
  bl_image_codec_rt_init(rt);
  bl_image_decoder_rt_init(rt);
  bl_image_encoder_rt_init(rt);
  bl_image_scale_rt_init(rt);
  bl_pattern_rt_init(rt);
  bl_gradient_rt_init(rt);
  bl_font_feature_settings_rt_init(rt);
  bl_font_variation_settings_rt_init(rt);
  bl_font_data_rt_init(rt);
  bl_font_face_rt_init(rt);
  bl_open_type_rt_init(rt);
  bl_font_rt_init(rt);
  bl_font_manager_rt_init(rt);
  bl_static_pipeline_rt_init(rt);

#if !defined(BL_BUILD_NO_JIT)
  bl_dynamic_pipeline_rt_init(rt);
#endif

  bl_context_rt_init(rt);
  bl_register_built_in_codecs(rt);

  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_runtime_shutdown() noexcept {
  BLRuntimeContext* rt = &bl_runtime_context;
  if (bl_atomic_fetch_sub_strong(&rt->ref_count) != 1)
    return BL_SUCCESS;

  rt->shutdown_handlers.call_in_reverse_order(rt);
  rt->shutdown_handlers.reset();
  rt->cleanup_handlers.reset();
  rt->resource_info_handlers.reset();

  return BL_SUCCESS;
}

static BL_RUNTIME_INITIALIZER BLRuntimeInitializer bl_runtime_auto_init;

// BLRuntime - API - Cleanup
// =========================

BL_API_IMPL BLResult bl_runtime_cleanup(BLRuntimeCleanupFlags cleanup_flags) noexcept {
  BLRuntimeContext* rt = &bl_runtime_context;
  rt->cleanup_handlers.call(rt, cleanup_flags);
  return BL_SUCCESS;
}

// BLRuntime - API - Query Info
// ============================

BL_API_IMPL BLResult bl_runtime_query_info(BLRuntimeInfoType info_type, void* info_out) noexcept {
  BLRuntimeContext* rt = &bl_runtime_context;

  switch (info_type) {
    case BL_RUNTIME_INFO_TYPE_BUILD: {
      BLRuntimeBuildInfo* build_info = static_cast<BLRuntimeBuildInfo*>(info_out);
      memcpy(build_info, &bl_runtime_build_info, sizeof(BLRuntimeBuildInfo));
      return BL_SUCCESS;
    }

    case BL_RUNTIME_INFO_TYPE_SYSTEM: {
      BLRuntimeSystemInfo* system_info = static_cast<BLRuntimeSystemInfo*>(info_out);
      memcpy(system_info, &rt->system_info, sizeof(BLRuntimeSystemInfo));
      return BL_SUCCESS;
    }

    case BL_RUNTIME_INFO_TYPE_RESOURCE: {
      BLRuntimeResourceInfo* resource_info = static_cast<BLRuntimeResourceInfo*>(info_out);
      resource_info->reset();
      rt->resource_info_handlers.call(rt, resource_info);
      return BL_SUCCESS;
    }

    default:
      return bl_make_error(BL_ERROR_INVALID_VALUE);
  }
}

// BLRuntime - API - Message
// =========================

BL_API_IMPL BLResult bl_runtime_message_out(const char* msg) noexcept {
#if defined(_WIN32)
  // Support both Console and GUI applications on Windows.
  OutputDebugStringA(msg);
#endif

  fputs(msg, stderr);
  return BL_SUCCESS;
}

BL_API_IMPL BLResult bl_runtime_message_fmt(const char* fmt, ...) noexcept {
  va_list ap;
  va_start(ap, fmt);
  BLResult result = bl_runtime_message_vfmt(fmt, ap);
  va_end(ap);

  return result;
}

BL_API_IMPL BLResult bl_runtime_message_vfmt(const char* fmt, va_list ap) noexcept {
  char buf[1024];
  vsnprintf(buf, BL_ARRAY_SIZE(buf), fmt, ap);
  return bl_runtime_message_out(buf);
}

// BLRuntime - API - Failure
// =========================

void bl_runtime_failure(const char* fmt, ...) noexcept {
  va_list ap;
  va_start(ap, fmt);
  bl_runtime_message_vfmt(fmt, ap);
  va_end(ap);

  abort();
}

BL_API_IMPL void bl_runtime_assertion_failure(const char* file, int line, const char* msg) noexcept {
  bl_runtime_message_fmt("[Blend2D] ASSERTION FAILURE: '%s' at '%s' [line %d]\n", msg, file, line);
  abort();
}

// BLRuntime - ResultFrom{Win|Posix}Error
// ======================================

#ifdef _WIN32

// Fix possible problems with MinGW not defining these.
#ifndef ERROR_DISK_QUOTA_EXCEEDED
  #define ERROR_DISK_QUOTA_EXCEEDED 0x0000050F
#endif

BL_API_IMPL BLResult bl_result_from_win_error(uint32_t e) noexcept {
  switch (e) {
    case ERROR_SUCCESS                : return BL_SUCCESS;                       // 0x00000000
    case ERROR_INVALID_FUNCTION       : return BL_ERROR_NOT_PERMITTED;           // 0x00000001
    case ERROR_FILE_NOT_FOUND         : return BL_ERROR_NO_ENTRY;                // 0x00000002
    case ERROR_PATH_NOT_FOUND         : return BL_ERROR_NO_ENTRY;                // 0x00000003
    case ERROR_TOO_MANY_OPEN_FILES    : return BL_ERROR_TOO_MANY_OPEN_FILES;     // 0x00000004
    case ERROR_ACCESS_DENIED          : return BL_ERROR_ACCESS_DENIED;           // 0x00000005
    case ERROR_INVALID_HANDLE         : return BL_ERROR_INVALID_HANDLE;          // 0x00000006
    case ERROR_NOT_ENOUGH_MEMORY      : return BL_ERROR_OUT_OF_MEMORY;           // 0x00000008
    case ERROR_OUTOFMEMORY            : return BL_ERROR_OUT_OF_MEMORY;           // 0x0000000E
    case ERROR_INVALID_DRIVE          : return BL_ERROR_NO_ENTRY;                // 0x0000000F
    case ERROR_CURRENT_DIRECTORY      : return BL_ERROR_NOT_PERMITTED;           // 0x00000010
    case ERROR_NOT_SAME_DEVICE        : return BL_ERROR_NOT_SAME_DEVICE;         // 0x00000011
    case ERROR_NO_MORE_FILES          : return BL_ERROR_NO_MORE_FILES;           // 0x00000012
    case ERROR_WRITE_PROTECT          : return BL_ERROR_READ_ONLY_FS;            // 0x00000013
    case ERROR_NOT_READY              : return BL_ERROR_NO_MEDIA;                // 0x00000015
    case ERROR_CRC                    : return BL_ERROR_IO;                      // 0x00000017
    case ERROR_SEEK                   : return BL_ERROR_INVALID_SEEK;            // 0x00000019
    case ERROR_WRITE_FAULT            : return BL_ERROR_IO;                      // 0x0000001D
    case ERROR_READ_FAULT             : return BL_ERROR_IO;                      // 0x0000001E
    case ERROR_GEN_FAILURE            : return BL_ERROR_IO;                      // 0x0000001F
    case ERROR_SHARING_BUFFER_EXCEEDED: return BL_ERROR_TOO_MANY_OPEN_FILES;     // 0x00000024
    case ERROR_HANDLE_EOF             : return BL_ERROR_NO_MORE_DATA;            // 0x00000026
    case ERROR_HANDLE_DISK_FULL       : return BL_ERROR_NO_SPACE_LEFT;           // 0x00000027
    case ERROR_NOT_SUPPORTED          : return BL_ERROR_NOT_IMPLEMENTED;         // 0x00000032
    case ERROR_FILE_EXISTS            : return BL_ERROR_ALREADY_EXISTS;          // 0x00000050
    case ERROR_CANNOT_MAKE            : return BL_ERROR_NOT_PERMITTED;           // 0x00000052
    case ERROR_INVALID_PARAMETER      : return BL_ERROR_INVALID_VALUE;           // 0x00000057
    case ERROR_NET_WRITE_FAULT        : return BL_ERROR_IO;                      // 0x00000058
    case ERROR_DRIVE_LOCKED           : return BL_ERROR_BUSY;                    // 0x0000006C
    case ERROR_BROKEN_PIPE            : return BL_ERROR_BROKEN_PIPE;             // 0x0000006D
    case ERROR_OPEN_FAILED            : return BL_ERROR_OPEN_FAILED;             // 0x0000006E
    case ERROR_BUFFER_OVERFLOW        : return BL_ERROR_FILE_NAME_TOO_LONG;      // 0x0000006F
    case ERROR_DISK_FULL              : return BL_ERROR_NO_SPACE_LEFT;           // 0x00000070
    case ERROR_CALL_NOT_IMPLEMENTED   : return BL_ERROR_NOT_IMPLEMENTED;         // 0x00000078
    case ERROR_INVALID_NAME           : return BL_ERROR_INVALID_FILE_NAME;       // 0x0000007B
    case ERROR_NEGATIVE_SEEK          : return BL_ERROR_INVALID_SEEK;            // 0x00000083
    case ERROR_SEEK_ON_DEVICE         : return BL_ERROR_INVALID_SEEK;            // 0x00000084
    case ERROR_BUSY_DRIVE             : return BL_ERROR_BUSY;                    // 0x0000008E
    case ERROR_DIR_NOT_ROOT           : return BL_ERROR_NOT_ROOT_DEVICE;         // 0x00000090
    case ERROR_DIR_NOT_EMPTY          : return BL_ERROR_NOT_EMPTY;               // 0x00000091
    case ERROR_PATH_BUSY              : return BL_ERROR_BUSY;                    // 0x00000094
    case ERROR_TOO_MANY_TCBS          : return BL_ERROR_TOO_MANY_THREADS;        // 0x0000009B
    case ERROR_BAD_ARGUMENTS          : return BL_ERROR_INVALID_VALUE;           // 0x000000A0
    case ERROR_BAD_PATHNAME           : return BL_ERROR_INVALID_FILE_NAME;       // 0x000000A1
    case ERROR_SIGNAL_PENDING         : return BL_ERROR_BUSY;                    // 0x000000A2
    case ERROR_MAX_THRDS_REACHED      : return BL_ERROR_TOO_MANY_THREADS;        // 0x000000A4
    case ERROR_BUSY                   : return BL_ERROR_BUSY;                    // 0x000000AA
    case ERROR_ALREADY_EXISTS         : return BL_ERROR_ALREADY_EXISTS;          // 0x000000B7
    case ERROR_BAD_PIPE               : return BL_ERROR_BROKEN_PIPE;             // 0x000000E6
    case ERROR_PIPE_BUSY              : return BL_ERROR_BUSY;                    // 0x000000E7
    case ERROR_NO_MORE_ITEMS          : return BL_ERROR_NO_MORE_FILES;           // 0x00000103
    case ERROR_FILE_INVALID           : return BL_ERROR_NO_ENTRY;                // 0x000003EE
    case ERROR_NO_DATA_DETECTED       : return BL_ERROR_IO;                      // 0x00000450
    case ERROR_MEDIA_CHANGED          : return BL_ERROR_MEDIA_CHANGED;           // 0x00000456
    case ERROR_IO_DEVICE              : return BL_ERROR_NO_DEVICE;               // 0x0000045D
    case ERROR_NO_MEDIA_IN_DRIVE      : return BL_ERROR_NO_MEDIA;                // 0x00000458
    case ERROR_DISK_OPERATION_FAILED  : return BL_ERROR_IO;                      // 0x00000467
    case ERROR_TOO_MANY_LINKS         : return BL_ERROR_TOO_MANY_LINKS;          // 0x00000476
    case ERROR_DISK_QUOTA_EXCEEDED    : return BL_ERROR_NO_SPACE_LEFT;           // 0x0000050F
    case ERROR_INVALID_USER_BUFFER    : return BL_ERROR_BUSY;                    // 0x000006F8
    case ERROR_UNRECOGNIZED_MEDIA     : return BL_ERROR_IO;                      // 0x000006F9
    case ERROR_NOT_ENOUGH_QUOTA       : return BL_ERROR_OUT_OF_MEMORY;           // 0x00000718
    case ERROR_CANT_ACCESS_FILE       : return BL_ERROR_NOT_PERMITTED;           // 0x00000780
    case ERROR_CANT_RESOLVE_FILENAME  : return BL_ERROR_NO_ENTRY;                // 0x00000781
    case ERROR_OPEN_FILES             : return BL_ERROR_TRY_AGAIN;               // 0x00000961
  }

  // Pass the system error if it's below our error indexing.
  if (e < BL_ERROR_START_INDEX)
    return e;

  // Otherwise this is an unmapped system error code.
  return BL_ERROR_UNKNOWN_SYSTEM_ERROR;
}

#else

BL_API_IMPL BLResult bl_result_from_posix_error(int e) noexcept {
  #define MAP(C_ERROR, BL_ERROR) case C_ERROR: return BL_ERROR

  switch (e) {
  #ifdef EACCES
    MAP(EACCES, BL_ERROR_ACCESS_DENIED);
  #endif
  #ifdef EAGAIN
    MAP(EAGAIN, BL_ERROR_TRY_AGAIN);
  #endif
  #ifdef EBADF
    MAP(EBADF, BL_ERROR_INVALID_HANDLE);
  #endif
  #ifdef EBUSY
    MAP(EBUSY, BL_ERROR_BUSY);
  #endif
  #ifdef EDQUOT
    MAP(EDQUOT, BL_ERROR_NO_SPACE_LEFT);
  #endif
  #ifdef EEXIST
    MAP(EEXIST, BL_ERROR_ALREADY_EXISTS);
  #endif
  #ifdef EFAULT
    MAP(EFAULT, BL_ERROR_INVALID_STATE);
  #endif
  #ifdef EFBIG
    MAP(EFBIG, BL_ERROR_FILE_TOO_LARGE);
  #endif
  #ifdef EINTR
    MAP(EINTR, BL_ERROR_INTERRUPTED);
  #endif
  #ifdef EINVAL
    MAP(EINVAL, BL_ERROR_INVALID_VALUE);
  #endif
  #ifdef EIO
    MAP(EIO, BL_ERROR_IO);
  #endif
  #ifdef EISDIR
    MAP(EISDIR, BL_ERROR_NOT_FILE);
  #endif
  #ifdef ELOOP
    MAP(ELOOP, BL_ERROR_SYMLINK_LOOP);
  #endif
  #ifdef EMFILE
    MAP(EMFILE, BL_ERROR_TOO_MANY_OPEN_FILES);
  #endif
  #ifdef ENAMETOOLONG
    MAP(ENAMETOOLONG, BL_ERROR_FILE_NAME_TOO_LONG);
  #endif
  #ifdef ENFILE
    MAP(ENFILE, BL_ERROR_TOO_MANY_OPEN_FILES_BY_OS);
  #endif
  #ifdef ENMFILE
    MAP(ENMFILE, BL_ERROR_NO_MORE_FILES);
  #endif
  #ifdef ENODATA
    MAP(ENODATA, BL_ERROR_NO_MORE_DATA);
  #endif
  #ifdef ENODEV
    MAP(ENODEV, BL_ERROR_NO_DEVICE);
  #endif
  #ifdef ENOENT
    MAP(ENOENT, BL_ERROR_NO_ENTRY);
  #endif
  #ifdef ENOMEDIUM
    MAP(ENOMEDIUM, BL_ERROR_NO_MEDIA);
  #endif
  #ifdef ENOMEM
    MAP(ENOMEM, BL_ERROR_OUT_OF_MEMORY);
  #endif
  #ifdef ENOSPC
    MAP(ENOSPC, BL_ERROR_NO_SPACE_LEFT);
  #endif
  #ifdef ENOSYS
    MAP(ENOSYS, BL_ERROR_NOT_IMPLEMENTED);
  #endif
  #ifdef ENOTBLK
    MAP(ENOTBLK, BL_ERROR_NOT_BLOCK_DEVICE);
  #endif
  #ifdef ENOTDIR
    MAP(ENOTDIR, BL_ERROR_NOT_DIRECTORY);
  #endif
  #ifdef ENOTEMPTY
    MAP(ENOTEMPTY, BL_ERROR_NOT_EMPTY);
  #endif
  #ifdef ENXIO
    MAP(ENXIO, BL_ERROR_NO_DEVICE);
  #endif
  #ifdef EOVERFLOW
    MAP(EOVERFLOW, BL_ERROR_OVERFLOW);
  #endif
  #ifdef EPERM
    MAP(EPERM, BL_ERROR_NOT_PERMITTED);
  #endif
  #ifdef EROFS
    MAP(EROFS, BL_ERROR_READ_ONLY_FS);
  #endif
  #ifdef ESPIPE
    MAP(ESPIPE, BL_ERROR_INVALID_SEEK);
  #endif
  #ifdef ETIMEDOUT
    MAP(ETIMEDOUT, BL_ERROR_TIMED_OUT);
  #endif
  #ifdef EXDEV
    MAP(EXDEV, BL_ERROR_NOT_SAME_DEVICE);
  #endif
  }

  #undef MAP

  // Pass the system error if it's below our error indexing.
  if (e != 0 && unsigned(e) < BL_ERROR_START_INDEX)
    return uint32_t(unsigned(e));
  else
    return BL_ERROR_UNKNOWN_SYSTEM_ERROR;
}
#endif
