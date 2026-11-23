// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

// This is an internal header file that is always included first by each Blend2D source file. This means that any
// macros we might need to define to build 'blend2d' can be defined here instead of passing them to the compiler
// through command line.

#ifndef BLEND2D_API_BUILD_P_H_INCLUDED
#define BLEND2D_API_BUILD_P_H_INCLUDED

// Build - Export
// ==============

//! \cond INTERNAL

//! Export mode is on when `BL_BUILD_EXPORT` is defined - this MUST be defined before including any other header
//! as "api.h" uses `BL_BUILD_EXPORT` to define a proper `BL_API` decorator that is used by all exported functions
//! and variables.
#define BL_BUILD_EXPORT

//! \endcond

// Build - Configuration
// =====================

// #define BL_BUILD_NO_JIT
// -----------------------
//
// Disables JIT pipeline generator. This should be turned off automatically by Blend2D's CMakeLists.txt on
// architectures for which JIT compilation is either not available or not allowed.

//! \cond NEVER
// Don't enable JIT if we don't have the implementation for the target platform.
//
// NOTE: This is a last resort check as this should be enabled/disabled by the build, not in the source code.
#if !(defined(BL_BUILD_NO_JIT)) && \
    !(defined(_M_X64)   || defined(__x86_64)  || defined(__x86_64__) || defined(__amd64) || defined(__amd64__) || \
      defined(_M_IX86)  || defined(__i386)    || defined(__i386__)   || \
      defined(__arm64)  || defined(__arm64__) || defined(__aarch64__))
  #define BL_BUILD_NO_JIT
#endif
//! \endcond

// #define BL_BUILD_NO_TLS
// -----------------------
//
// Disables all use of thread_local feature. Provided for compatibility with platforms where thread local
// storage is either very expensive to use or not supported at all.

// #define BL_BUILD_NO_FUTEX
// -------------------------
//
// Disable built-in support for futexes and use the portable implementation instead. Used by CI to verify the
// portable implementation works the same as futex implementation. Can be used by users to disable futexes if
// required.

// #define BL_BUILD_NO_INTRINSICS
// ------------------------------
//
// Disable most of compiler intrinsics used by Blend2D. Disabling them is only useful for testing fallback
// functions as otherwise there is no other way to test them.

// #define BL_BUILD_NO_STDCXX
// --------------------------
//
// Informs Blend2D that it's compiled without linking to the standard C++ library. This macro must be only
// defined by a build system that understands what has to be done to make this possible.
//
// See CMakeLists.txt for more details.

// #define BL_TRACE_OT_ALL          // Trace OpenType features (all).
// #define BL_TRACE_OT_CFF          // Trace OpenType CFF|CFF2 ('CFF ', 'CFF2).
// #define BL_TRACE_OT_CORE         // Trace OpenType core     ('OS/2', 'head', 'maxp', 'post').
// #define BL_TRACE_OT_KERN         // Trace OpenType kerning  ('kern').
// #define BL_TRACE_OT_LAYOUT       // Trace OpenType layout   ('BASE', 'GDEF', 'GPOS', 'GSUB', 'JSTF').
// #define BL_TRACE_OT_NAME         // Trace OpenType name     ('name').
//
// Blend2D provides traces that can be enabled during development. Traces can help to understand how certain
// things work and can be used to track bugs.

// Build - Sanitizers
// ==================

#if defined(__clang__)
  #if __has_feature(memory_sanitizer) || defined(__SANITIZE_MEMORY__)
    #define BL_SANITIZE_MEMORY
  #endif

  #if __has_feature(thread_sanitizer) || defined(__SANITIZE_THREAD__)
    #define BL_SANITIZE_THREAD
  #endif
#elif defined(__GNUC__)
  #if defined(__SANITIZE_MEMORY__)
    #define BL_SANITIZE_MEMORY
  #endif

  #if defined(__SANITIZE_THREAD__)
    #define BL_SANITIZE_THREAD
  #endif
#endif

#if defined(BL_SANITIZE_THREAD)
extern "C" void __tsan_acquire(void *addr);
extern "C" void __tsan_release(void *addr);
#endif

// Build - Requirements
// ====================

//! \cond NEVER

// Turn off deprecation warnings when building 'blend2d'. Required as <blend2d.h> and other headers include some
// essential C headers that could in some cases warn about using functions such as `snprintf()`, which we use
// correctly.
#ifdef _MSC_VER
  #if !defined(_CRT_SECURE_NO_DEPRECATE)
    #define _CRT_SECURE_NO_DEPRECATE
  #endif
  #if !defined(_CRT_SECURE_NO_WARNINGS)
    #define _CRT_SECURE_NO_WARNINGS
  #endif
#endif

#if defined(_WIN32) && !defined(_WIN32_WINNT)
  #if !defined(BL_PLATFORM_UWP) && defined(WINAPI_FAMILY) && defined(__has_include)
    #if __has_include(<winapifamily.h>)
      #include <winapifamily.h>
      #if WINAPI_FAMILY == WINAPI_FAMILY_PC_APP
        #define BL_PLATFORM_UWP
      #endif
    #endif
  #endif

  #if defined(BL_PLATFORM_UWP)
    #define _WIN32_WINNT 0x0602 // Windows 8+ (CreateFile2, CreateFileMappingFromApp, ...)
  #else
    #define _WIN32_WINNT 0x0600 // Windows Vista+ (CONDITION_VARIABLE, ...)
  #endif
#endif

// The FileSystem API works fully with 64-bit file sizes and offsets, however, this feature must be enabled before
// including any header.
#if !defined(_WIN32) && !defined(_LARGEFILE64_SOURCE)
  #define _LARGEFILE64_SOURCE 1

  // These OSes use 64-bit offsets by default.
  #if defined(__APPLE__    ) || \
      defined(__HAIKU__    ) || \
      defined(__bsdi__     ) || \
      defined(__DragonFly__) || \
      defined(__FreeBSD__  ) || \
      defined(__NetBSD__   ) || \
      defined(__OpenBSD__  )
    #define BL_FILE64_API(NAME) NAME
  #else
    #define BL_FILE64_API(NAME) NAME##64
  #endif
#endif

// The FileSystem API supports extensions offered by Linux.
#if defined(__linux__) && !defined(_GNU_SOURCE)
  #define _GNU_SOURCE
#endif

//! \endcond

// Build - Compiler Diagnostics
// ============================

//! \cond NEVER

#if defined(__clang__)
  #pragma clang diagnostic warning "-Wattributes"
  #pragma clang diagnostic ignored "-Wunnamed-type-template-args"
  #pragma clang diagnostic ignored "-Wunused-function"
#elif defined(__GNUC__)
  #pragma GCC diagnostic warning "-Wattributes"
  #pragma GCC diagnostic ignored "-Wmaybe-uninitialized" // Unfortunately GCC emits lots of false positives.
  #pragma GCC diagnostic ignored "-Wunused-function"
  #if __GNUC__ <= 8
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"     // Reports some cases that are perfectly fine.
  #endif
#elif defined(_MSC_VER)
  #pragma warning(disable: 4102) // Unreferenced label.
  #pragma warning(disable: 4127) // Conditional expression is constant.
  #pragma warning(disable: 4201) // Nameless struct/union.
  #pragma warning(disable: 4251) // Struct needs to have dll-interface.
  #pragma warning(disable: 4275) // Non dll-interface struct ... used.
  #pragma warning(disable: 4324) // Structure was padded due to alignment specifier.
  #pragma warning(disable: 4355) // This used in base member-init list.
  #pragma warning(disable: 4458) // declaration of 'X' hides class member.
  #pragma warning(disable: 4480) // Specifying underlying type for enum.
  #pragma warning(disable: 4505) // Unreferenced local function has been removed.
  #pragma warning(disable: 4800) // Forcing value to bool true or false.
  #pragma warning(disable: 4582) // Constructor is not implicitly called.
  #pragma warning(disable: 4583) // Destructor is not implicitly called.
#endif

//! \endcond

// build - Include API
// ===================

#include <blend2d/core/api.h>
#include <blend2d/core/api-impl.h>
#include <blend2d/core/api-internal_p.h>

#endif // BLEND2D_API_BUILD_P_H_INCLUDED
