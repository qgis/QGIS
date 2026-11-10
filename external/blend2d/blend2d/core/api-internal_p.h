// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_API_INTERNAL_P_H_INCLUDED
#define BLEND2D_API_INTERNAL_P_H_INCLUDED

#include <blend2d/core/api.h>
#include <blend2d/core/api-impl.h>
#include <blend2d/core/object.h>

// C Headers
// =========

// NOTE: Some headers are already included by <api.h>. This should be useful for creating an overview of what
// Blend2D really needs globally to be included.
#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// C++ Headers
// ===========

// We are just fine with <math.h>, however, there are some useful overloads in C++'s <cmath> that are nicer to use
// than those in <math.h>. Mostly low-level functionality like Math::is_finite() relies on <cmath> instead of <math.h>.
#include <cmath>
#include <limits>
#include <type_traits>

// Platform Specific Headers
// =========================

#if defined(_WIN32)
  //! \cond NEVER
  #if !defined(WIN32_LEAN_AND_MEAN)
    #define WIN32_LEAN_AND_MEAN
  #endif
  #if !defined(NOMINMAX)
    #define NOMINMAX
  #endif
  //! \endcond

  #include <windows.h>   // Required to build Blend2D on Windows platform.
  #include <synchapi.h>  // Synchronization primitivess.
#endif

// Some intrinsics defined by MSVC compiler are useful and used across the library.
#ifdef _MSC_VER
  #include <intrin.h>
#endif

//! \cond INTERNAL
//! \addtogroup bl_globals
//! \{

// Build - Target Architecture & Optimizations
// ===========================================

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

#if defined(__wasm64__)
  #define BL_TARGET_ARCH_WASM 64
#elif defined(__wasm32__) || defined(__wasm__)
  #define BL_TARGET_ARCH_WASM 32
#else
  #define BL_TARGET_ARCH_WASM 0
#endif

#if defined(_M_X64) || defined(__amd64) || defined(__amd64__) || defined(__x86_64) || defined(__x86_64__)
  #define BL_TARGET_ARCH_X86 64
#elif defined(_M_IX86) || defined(__i386) || defined(__i386__)
  #define BL_TARGET_ARCH_X86 32
#else
  #define BL_TARGET_ARCH_X86 0
#endif

#if defined(_M_ARM64) || defined(__ARM64__) || defined(__aarch64__)
  #define BL_TARGET_ARCH_ARM 64
#elif defined(_M_ARM) || defined(_M_ARMT) || defined(__arm__) || defined(__thumb__) || defined(__thumb2__)
  #define BL_TARGET_ARCH_ARM 32
#else
  #define BL_TARGET_ARCH_ARM 0
#endif

#if defined(_MIPS_ARCH_MIPS64) || defined(__mips64)
  #define BL_TARGET_ARCH_MIPS 64
#elif defined(_MIPS_ARCH_MIPS32) || defined(_M_MRX000) || defined(__mips) || defined(__mips__)
  #define BL_TARGET_ARCH_MIPS 32
#else
  #define BL_TARGET_ARCH_MIPS 0
#endif

#define BL_TARGET_ARCH_BITS (BL_TARGET_ARCH_X86 | BL_TARGET_ARCH_ARM | BL_TARGET_ARCH_MIPS | BL_TARGET_ARCH_WASM)
#if BL_TARGET_ARCH_BITS == 0
  #undef BL_TARGET_ARCH_BITS
  #if defined(__LP64__) || defined(_LP64)
    #define BL_TARGET_ARCH_BITS 64
  #else
    #define BL_TARGET_ARCH_BITS 32
  #endif
#endif

// Defined when it's safe to assume that std::atomic<uint64_t> would be non locking. We can always assume this on
// X86 architecture even 32-bit as it provides CMPXCHG8B, but we don't assume this on other architectures like ARM.
#if BL_TARGET_ARCH_BITS >= 64 || BL_TARGET_ARCH_X86 != 0
  #define BL_TARGET_HAS_ATOMIC_64B 1
#else
  #define BL_TARGET_HAS_ATOMIC_64B 0
#endif

#if !defined(BL_BUILD_NO_JIT)
  #if BL_TARGET_ARCH_X86 != 0
    #define BL_JIT_ARCH_X86
  #elif BL_TARGET_ARCH_ARM == 64
    #define BL_JIT_ARCH_A64
  #endif
#endif // !BL_BUILD_NO_JIT

// Build optimizations control compile-time optimizations to be used by Blend2D and C++ compiler. These optimizations
// are not related to the code-generator optimizations (JIT) that are always auto-detected at runtime.
#if BL_TARGET_ARCH_X86
  #if defined(BL_BUILD_OPT_AVX512) && !defined(BL_BUILD_OPT_AVX2)
    #define BL_BUILD_OPT_AVX2
  #endif
  #if defined(BL_BUILD_OPT_AVX2) && !defined(BL_BUILD_OPT_AVX)
    #define BL_BUILD_OPT_AVX
  #endif
  #if defined(BL_BUILD_OPT_AVX) && !defined(BL_BUILD_OPT_SSE4_2)
    #define BL_BUILD_OPT_SSE4_2
  #endif
  #if defined(BL_BUILD_OPT_SSE4_2) && !defined(BL_BUILD_OPT_SSE4_1)
    #define BL_BUILD_OPT_SSE4_1
  #endif
  #if defined(BL_BUILD_OPT_SSE4_1) && !defined(BL_BUILD_OPT_SSSE3)
    #define BL_BUILD_OPT_SSSE3
  #endif
  #if defined(BL_BUILD_OPT_SSSE3) && !defined(BL_BUILD_OPT_SSE3)
    #define BL_BUILD_OPT_SSE3
  #endif
  #if defined(BL_BUILD_OPT_SSE3) && !defined(BL_BUILD_OPT_SSE2)
    #define BL_BUILD_OPT_SSE2
  #endif

  #if defined(__AVX512F__)  && defined(__AVX512BW__) && defined(__AVX512DQ__) && defined(__AVX512CD__) && defined(__AVX512VL__)
    #define BL_TARGET_OPT_AVX512
  #endif
  #if defined(BL_TARGET_OPT_AVX512) || defined(__AVX2__)
    #define BL_TARGET_OPT_AVX2
  #endif
  #if defined(BL_TARGET_OPT_AVX2) || defined(__AVX__)
    #define BL_TARGET_OPT_AVX
  #endif
  #if defined(BL_TARGET_OPT_AVX) || defined(__SSE4_2__)
    #define BL_TARGET_OPT_SSE4_2
  #endif
  #if defined(BL_TARGET_OPT_SSE4_2) || defined(__SSE4_1__)
    #define BL_TARGET_OPT_SSE4_1
  #endif
  #if defined(BL_TARGET_OPT_SSE4_1) || defined(__SSSE3__)
    #define BL_TARGET_OPT_SSSE3
  #endif
  #if defined(BL_TARGET_OPT_SSSE3) || defined(__SSE3__)
    #define BL_TARGET_OPT_SSE3
  #endif
  #if defined(BL_TARGET_OPT_SSE3) || (BL_TARGET_ARCH_X86 == 64 || defined(__SSE2__) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2))
    #define BL_TARGET_OPT_SSE2
  #endif
  #if defined(BL_TARGET_OPT_SSE2) || (BL_TARGET_ARCH_X86 == 64 || defined(__SSE__) || (defined(_M_IX86_FP) && _M_IX86_FP >= 1))
    #define BL_TARGET_OPT_SSE
  #endif

  #if !defined(BL_TARGET_OPT_BMI2) && defined(__BMI2__)
    #define BL_TARGET_OPT_BMI2
  #endif

  #if !defined(BL_TARGET_OPT_POPCNT) && (defined(__POPCNT__) || defined(BL_TARGET_OPT_SSE4_2))
    #define BL_TARGET_OPT_POPCNT
  #endif
#endif

#if BL_TARGET_ARCH_ARM && (BL_TARGET_ARCH_ARM == 64 || defined(__ARM_NEON__))
  #ifndef BL_TARGET_OPT_ASIMD
    #define BL_TARGET_OPT_ASIMD
  #endif // BL_TARGET_OPT_ASIMD
  #ifndef BL_BUILD_OPT_ASIMD
    #define BL_BUILD_OPT_ASIMD
  #endif // BL_BUILD_OPT_ASIMD
#endif

//! \}
//! \endcond

// C++ Compiler Support
// ====================

// Some compilers won't optimize our stuff if we won't tell them. However, we have to be careful as Blend2D doesn't
// use fast-math by default. So when applicable we turn some optimizations on and off locally, but not globally.
//
// This has also a lot of downsides. For example all wrappers in "std" namespace are compiled with default flags so
// even when we force the compiler to follow certain behavior it would only work if we use the "original" functions
// and not those wrapped in `std` namespace (so use floor and not std::floor, etc).
#if defined(_MSC_VER) && !defined(__clang__)
  #define BL_PRAGMA_FAST_MATH_PUSH __pragma(float_control(precise, off, push))
  #define BL_PRAGMA_FAST_MATH_POP __pragma(float_control(pop))
#else
  #define BL_PRAGMA_FAST_MATH_PUSH
  #define BL_PRAGMA_FAST_MATH_POP
#endif

#if defined(__clang__) || defined(__has_attribute__)
  #define BL_CC_HAS_ATTRIBUTE(x) __has_attribute(x)
#else
  #define BL_CC_HAS_ATTRIBUTE(x) 0
#endif

//! \def BL_HIDDEN
//!
//! Decorates a function that is used across more than one source file, but should never be exported. Expands to
//! a compiler-specific code that affects the visibility.
#if defined(__GNUC__) && !defined(__MINGW32__)
  #define BL_HIDDEN __attribute__((__visibility__("hidden")))
#else
  #define BL_HIDDEN
#endif

//! \def BL_OPTIMIZE
//!
//! Decorates a function that should be highly optimized by C++ compiler. In general Blend2D uses "-O2" optimization
//! level on GCC and Clang, this macro would change the optimization level to "-O3" for the decorated function.
#if !defined(BL_BUILD_DEBUG) && (BL_CC_HAS_ATTRIBUTE(__optimize__) || (!defined(__clang__) && defined(__GNUC__)))
  #define BL_OPTIMIZE __attribute__((__optimize__("O3")))
#else
  #define BL_OPTIMIZE
#endif

//! \def BL_NOINLINE
//!
//! Decorates a function that should never be inlined. Sometimes used by Blend2D to decorate functions that are
//! either called rarely or that are called from other code in corner cases - like buffer reallocation, etc...
#if defined(__GNUC__)
  #define BL_NOINLINE __attribute__((__noinline__))
#elif defined(_MSC_VER)
  #define BL_NOINLINE __declspec(noinline)
#else
  #define BL_NOINLINE
#endif

//! \def BL_FLATTEN
//!
//! Either `__attribute__((flatten))` or nothing, if not supported.
#if defined(__GNUC__)
  #define BL_FLATTEN __attribute__((__flatten__))
#else
  #define BL_FLATTEN
#endif

//! \def BL_STDCALL
//!
//! Calling convention used by Windows.
#if defined(__GNUC__) && defined(__i386__) && !defined(__x86_64__)
  #define BL_STDCALL __attribute__((__stdcall__))
#elif defined(_MSC_VER)
  #define BL_STDCALL __stdcall
#else
  #define BL_STDCALL
#endif

//! \def BL_OPTIMIZED_CALL
//!
//! Optimized calling convention used internally in some places.
#define BL_OPTIMIZED_CALL

//! \def BL_UNALIGNED_TYPE(TYPE, ALIGNMENT)
//!
//! Defines a type, that is aligned to less than its native alignment. In general this is not supported by the
//! C++ alignas() keyword, which only allows to increase the alignment. Compilers that support unaligned types
//! can generate nicer code when such type is accessed and UBSAN would not complain about such accesses, if
//! properly annotated.
#if defined(__GNUC__)
  #define BL_UNALIGNED_TYPE(TYPE, ALIGNMENT) __attribute__((__aligned__(ALIGNMENT))) __attribute__((__may_alias__)) TYPE
#elif defined(_MSC_VER)
  #define BL_UNALIGNED_TYPE(TYPE, ALIGNMENT) __declspec(align(ALIGNMENT)) TYPE
#else
  #define BL_UNALIGNED_TYPE(TYPE, ALIGNMENT) TYPE
#endif

//! \def BL_MAY_ALIAS
//!
//! Expands to `__attribute__((__may_alias__))` if supported.
#if defined(__GNUC__)
  #define BL_MAY_ALIAS __attribute__((__may_alias__))
#else
  #define BL_MAY_ALIAS
#endif

//! \def BL_NOUNROLL
//!
//! Compiler-specific macro that annotates a do/for/while loop to not get unrolled.
#if defined(__clang__)
  #define BL_NOUNROLL _Pragma("nounroll")
#elif defined(__GNUC__) && (__GNUC__ >= 8)
  // NOTE: We could use `_Pragma("GCC unroll 1")`, however, this doesn't apply to all loops and GCC emits a lot of
  // warnings such as "ignoring loop annotation", which cannot be turned off globally nor locally. So we disable
  // loop annotations when compiling with GCC. This comment is here so we can re-enable in the future.
  #define BL_NOUNROLL
#else
  #define BL_NOUNROLL
#endif

//! \def BL_RUNTIME_INITIALIZER
//!
//! Static initialization in C++ is full of surpsises and basically nothing is guaranteed. When Blend2D is compiled
//! as a shared library everything is fine, however, when it's compiled as a static library and user uses statically
//! allocated Blend2D objects then the object may initialize even before the library - this tries to solve that issue.
#if defined(__clang__) || (defined(__GNUC__) && !defined(__APPLE__))
  #define BL_RUNTIME_INITIALIZER __attribute__((init_priority(102)))
#else
  #define BL_RUNTIME_INITIALIZER
#endif

#if defined(BL_BUILD_DEBUG) && !defined(__OPTIMIZE__)
  #define BL_INLINE_IF_NOT_DEBUG
#else
  #define BL_INLINE_IF_NOT_DEBUG BL_INLINE
#endif

//! \def BL_API_IMPL
//!
//! Decorator used to mark all functions and variables that are exported - it expands to "extern C", which ensures
//! that an exported function or variable can be implemented within a private namespace and it would still be exported
//! properly.
#define BL_API_IMPL extern "C" BL_API

#define BL_STRINGIFY_WRAP(N) #N
#define BL_STRINGIFY(N) BL_STRINGIFY_WRAP(N)

#define BL_STATIC_ASSERT(...) static_assert(__VA_ARGS__, "Failed BL_STATIC_ASSERT(" #__VA_ARGS__ ")")

// Internal C++ Macros
// ===================

//! \def BL_NONCOPYABLE
//!
//! Makes a class noncopyable by making its copy constructor and copy assignment operator deleted.
#define BL_NONCOPYABLE(...)                                                   \
  __VA_ARGS__(const __VA_ARGS__& other) = delete;                             \
  __VA_ARGS__& operator=(const __VA_ARGS__& other) = delete;

//! \def BL_NOT_REACHED()
//!
//! Run-time assertion used in code that should never be reached.
#ifdef BL_BUILD_DEBUG
  #define BL_NOT_REACHED() bl_runtime_assertion_failure(__FILE__, __LINE__, "BL_NOT_REACHED()")
#elif defined(__GNUC__)
  #define BL_NOT_REACHED() __builtin_unreachable()
#else
  #define BL_NOT_REACHED() BL_ASSUME(0)
#endif

#if defined(_MSC_VER)
  #define BL_RESTRICT __restrict
#elif defined(__GNUC__)
  #define BL_RESTRICT __restrict__
#else
  #define BL_RESTRICT
#endif

#define BL_ARRAY_SIZE(X) uint32_t(sizeof(X) / sizeof(X[0]))
#define BL_OFFSET_OF(STRUCT, MEMBER) ((int)(offsetof(STRUCT, MEMBER)))

#define BL_PROPAGATE_(exp, cleanup)                                           \
  do {                                                                        \
    BLResult _result_to_propagate = (exp);                                    \
    if (BL_UNLIKELY(_result_to_propagate != BL_SUCCESS)) {                    \
      cleanup                                                                 \
      return _result_to_propagate;                                            \
    }                                                                         \
  } while (0)

//! Like BL_PROPAGATE, but propagates everything except `BL_RESULT_NOTHING`.
#define BL_PROPAGATE_IF_NOT_NOTHING(...)                                      \
  do {                                                                        \
    BLResult result_to_propagate = (__VA_ARGS__);                             \
    if (result_to_propagate != BL_RESULT_NOTHING) {                           \
      return result_to_propagate;                                             \
    }                                                                         \
  } while (0)

//! Decorates a base class that has virtual functions.
#define BL_OVERRIDE_NEW_DELETE(TYPE)                                          \
  BL_INLINE void* operator new(size_t n) noexcept { return malloc(n); }       \
  BL_INLINE void  operator delete(void* p) noexcept { if (p) free(p); }       \
                                                                              \
  BL_INLINE void* operator new(size_t, const BLInternal::PlacementNew& p) noexcept { return p.ptr; } \
  BL_INLINE void  operator delete(void*, void*) noexcept {}

//! \def BL_DEFINE_ENUM_FLAGS(T)
//!
//! Defines operations for enumeration flags.
#ifdef _DOXYGEN
  #define BL_DEFINE_ENUM_FLAGS(T)
#else
  #define BL_DEFINE_ENUM_FLAGS(T)                                             \
    static BL_INLINE_CONSTEXPR T operator~(T a) noexcept {                    \
      return T(~std::underlying_type_t<T>(a));                                \
    }                                                                         \
                                                                              \
    static BL_INLINE_CONSTEXPR T operator|(T a, T b) noexcept {               \
      return T(std::underlying_type_t<T>(a) | std::underlying_type_t<T>(b));  \
    }                                                                         \
    static BL_INLINE_CONSTEXPR T operator&(T a, T b) noexcept {               \
      return T(std::underlying_type_t<T>(a) & std::underlying_type_t<T>(b));  \
    }                                                                         \
    static BL_INLINE_CONSTEXPR T operator^(T a, T b) noexcept {               \
      return T(std::underlying_type_t<T>(a) ^ std::underlying_type_t<T>(b));  \
    }                                                                         \
                                                                              \
    static BL_INLINE_CONSTEXPR T& operator|=(T& a, T b) noexcept {            \
      a = T(std::underlying_type_t<T>(a) | std::underlying_type_t<T>(b));     \
      return a;                                                               \
    }                                                                         \
    static BL_INLINE_CONSTEXPR T& operator&=(T& a, T b) noexcept {            \
      a = T(std::underlying_type_t<T>(a) & std::underlying_type_t<T>(b));     \
      return a;                                                               \
    }                                                                         \
    static BL_INLINE_CONSTEXPR T& operator^=(T& a, T b) noexcept {            \
      a = T(std::underlying_type_t<T>(a) ^ std::underlying_type_t<T>(b));     \
      return a;                                                               \
    }
#endif

//! \def BL_DEFINE_STRONG_TYPE(C, T)
//!
//! Defines a strong type `C` that wraps a value of `T`.
#define BL_DEFINE_STRONG_TYPE(C, T)                                                     \
struct C {                                                                              \
  T _v;                                                                                 \
                                                                                        \
  BL_INLINE_NODEBUG C() = default;                                                      \
  BL_INLINE_CONSTEXPR explicit C(T x) noexcept : _v(x) {}                               \
  BL_INLINE_CONSTEXPR C(const C& other) noexcept = default;                             \
                                                                                        \
  BL_INLINE_CONSTEXPR T value() const noexcept { return _v; }                           \
                                                                                        \
  BL_INLINE_CONSTEXPR T* value_ptr() noexcept { return &_v; }                           \
  BL_INLINE_CONSTEXPR const T* value_ptr() const noexcept { return &_v; }               \
                                                                                        \
  BL_INLINE_CONSTEXPR C& operator=(T x) noexcept { _v = x; return *this; };             \
  BL_INLINE_CONSTEXPR C& operator=(const C& x) noexcept { _v = x._v; return *this; }    \
                                                                                        \
  BL_INLINE_CONSTEXPR C operator+(T x) const noexcept { return C(_v + x); }             \
  BL_INLINE_CONSTEXPR C operator-(T x) const noexcept { return C(_v - x); }             \
  BL_INLINE_CONSTEXPR C operator*(T x) const noexcept { return C(_v * x); }             \
  BL_INLINE_CONSTEXPR C operator/(T x) const noexcept { return C(_v / x); }             \
                                                                                        \
  BL_INLINE_CONSTEXPR C operator+(const C& x) const noexcept { return C(_v + x._v); }   \
  BL_INLINE_CONSTEXPR C operator-(const C& x) const noexcept { return C(_v - x._v); }   \
  BL_INLINE_CONSTEXPR C operator*(const C& x) const noexcept { return C(_v * x._v); }   \
  BL_INLINE_CONSTEXPR C operator/(const C& x) const noexcept { return C(_v / x._v); }   \
                                                                                        \
  BL_INLINE_CONSTEXPR C& operator+=(T x) noexcept { _v += x; return *this; }            \
  BL_INLINE_CONSTEXPR C& operator-=(T x) noexcept { _v -= x; return *this; }            \
  BL_INLINE_CONSTEXPR C& operator*=(T x) noexcept { _v *= x; return *this; }            \
  BL_INLINE_CONSTEXPR C& operator/=(T x) noexcept { _v /= x; return *this; }            \
                                                                                        \
  BL_INLINE_CONSTEXPR C& operator+=(const C& x) noexcept { _v += x._v; return *this; }  \
  BL_INLINE_CONSTEXPR C& operator-=(const C& x) noexcept { _v -= x._v; return *this; }  \
  BL_INLINE_CONSTEXPR C& operator*=(const C& x) noexcept { _v *= x._v; return *this; }  \
  BL_INLINE_CONSTEXPR C& operator/=(const C& x) noexcept { _v /= x._v; return *this; }  \
                                                                                        \
  BL_INLINE_CONSTEXPR bool operator==(T x) const noexcept { return _v == x; }           \
  BL_INLINE_CONSTEXPR bool operator!=(T x) const noexcept { return _v != x; }           \
  BL_INLINE_CONSTEXPR bool operator> (T x) const noexcept { return _v >  x; }           \
  BL_INLINE_CONSTEXPR bool operator>=(T x) const noexcept { return _v >= x; }           \
  BL_INLINE_CONSTEXPR bool operator< (T x) const noexcept { return _v <  x; }           \
  BL_INLINE_CONSTEXPR bool operator<=(T x) const noexcept { return _v <= x; }           \
                                                                                        \
  BL_INLINE_CONSTEXPR bool operator==(const C& x) const noexcept { return _v == x._v; } \
  BL_INLINE_CONSTEXPR bool operator!=(const C& x) const noexcept { return _v != x._v; } \
  BL_INLINE_CONSTEXPR bool operator> (const C& x) const noexcept { return _v >  x._v; } \
  BL_INLINE_CONSTEXPR bool operator>=(const C& x) const noexcept { return _v >= x._v; } \
  BL_INLINE_CONSTEXPR bool operator< (const C& x) const noexcept { return _v <  x._v; } \
  BL_INLINE_CONSTEXPR bool operator<=(const C& x) const noexcept { return _v <= x._v; } \
};

// Internal Macros
// ===============

#define BL_RETURN_ERROR_IF_NULL(ptr)               \
  do {                                             \
    if (!(ptr))                                    \
      return bl_make_error(BL_ERROR_OUT_OF_MEMORY); \
  } while (0)

#define BL_RETURN_ERROR_IF_NULL_(ptr, ...)         \
  do {                                             \
    if (!(ptr)) {                                  \
      __VA_ARGS__                                  \
      return bl_make_error(BL_ERROR_OUT_OF_MEMORY); \
    }                                              \
  } while (0)

// Internal Types
// ==============

//! A type used to store a pack of bits (typedef to `uintptr_t`).
//!
//! BitWord should be equal in size to a machine word.
using BLBitWord = uintptr_t;

// Internal Constants
// ==================

//! To make checks for APPEND operation easier.
static constexpr BLModifyOp BL_MODIFY_OP_APPEND_START = BLModifyOp(2);
//! Mask that can be used to check whether `BLModifyOp` has a grow hint.
static constexpr BLModifyOp BL_MODIFY_OP_GROW_MASK = BLModifyOp(1);

static BL_INLINE_CONSTEXPR bool bl_modify_op_is_assign(BLModifyOp modify_op) noexcept { return modify_op < BL_MODIFY_OP_APPEND_START; }
static BL_INLINE_CONSTEXPR bool bl_modify_op_is_append(BLModifyOp modify_op) noexcept { return modify_op >= BL_MODIFY_OP_APPEND_START; }
static BL_INLINE_CONSTEXPR bool bl_modify_op_does_grow(BLModifyOp modify_op) noexcept { return (modify_op & BL_MODIFY_OP_GROW_MASK) != 0; }

// Target CPU Properties Known at Compile Time
// -------------------------------------------

//! Size of a CPU cache-line or a minimum size if multiple CPUs are used.
//!
//! Mostly depends on architecture, we use 64 bytes by default.
static constexpr uint32_t BL_CACHE_LINE_SIZE = 64;

// Blend2D Limits and System Allocator Properties
// ----------------------------------------------

//! Host memory allocator overhead (estimated).
static constexpr uint32_t BL_ALLOC_OVERHEAD = uint32_t(sizeof(void*)) * 4u;
//! Host memory allocator alignment (can be lower than reality, but cannot be higher).
static constexpr uint32_t BL_ALLOC_ALIGNMENT = 8u;

//! Limits doubling of a container size after the limit size [in bytes] has reached 8MB. The container will
//! use a more conservative approach after the threshold has been reached.
static constexpr uint32_t BL_ALLOC_GROW_LIMIT = 1u << 23;

// Alloc Hints Used by Blend2D Containers
// --------------------------------------

//! Minimum vertices to amortize the check of a matrix type.
static constexpr uint32_t BL_MATRIX_TYPE_MINIMUM_SIZE = 16u;

//! Maximum number of faces per a single font collection.
static constexpr uint32_t BL_FONT_DATA_MAX_FACE_COUNT = 256u;

//! BLResult value that is used internally to signalize that the function didn't succeed, but also didn't fail.
//! This is not an error state. At the moment this is only used by \ref BLPixelConverter when setting up optimized
//! conversion functions.
//!
//! \note This result code can be never propagated to the user code!
static constexpr uint32_t BL_RESULT_NOTHING = 0xFFFFFFFFu;

//! Analysis result that describes whether an unknown input is conforming.
enum BLDataAnalysis : uint32_t {
  //! The input data is conforming (stored exactly as expected).
  BL_DATA_ANALYSIS_CONFORMING = 0, // Must be 0
  //! The input data is valid, but non-conforming (must be processed).
  BL_DATA_ANALYSIS_NON_CONFORMING = 1, // Must be 1
  //! The input data contains an invalid value.
  BL_DATA_ANALYSIS_INVALID_VALUE = 2
};

// Internal C++ Structs
// ====================

template<typename ValueT>
struct BLResultT {
  BLResult code;
  ValueT value;
};

// Internal C++ Functions
// ======================

//! Used to silence warnings about unused arguments or variables.
template<typename... Args>
static BL_INLINE_NODEBUG void bl_unused(Args&&...) noexcept {}

template<typename T>
static BL_INLINE_CONSTEXPR bool bl_test_flag(const T& x, const T& y) noexcept {
  return (std::underlying_type_t<T>(x) & std::underlying_type_t<T>(y)) != std::underlying_type_t<T>(0);
}

// TODO: Remove.
template<typename T, typename F>
static BL_INLINE_NODEBUG void bl_assign_func(T** dst, F f) noexcept { *(void**)dst = (void*)f; }

// Miscellaneous Internals
// =======================

//! Checks whether `data_access_flags` is valid.
static BL_INLINE_NODEBUG bool bl_data_access_flags_is_valid(uint32_t data_access_flags) noexcept {
  return data_access_flags == BL_DATA_ACCESS_READ ||
         data_access_flags == BL_DATA_ACCESS_RW;
}

static BL_INLINE_NODEBUG void bl_prefetch_w(const void* p) { (void)p; }

// BLInternal API Accessible Via 'bl' Namespace
// ============================================

namespace bl { using namespace BLInternal; }

//! \}
//! \endcond

#endif // BLEND2D_API_INTERNAL_P_H_INCLUDED
