// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_GLOBALS_H_INCLUDED
#define ASMJIT_CORE_GLOBALS_H_INCLUDED

#include <asmjit/core/api-config.h>

ASMJIT_BEGIN_NAMESPACE

//! \cond INTERNAL
//! \addtogroup asmjit_utilities
//! \{

namespace Support {

//! Cast designed to cast between function and void* pointers.
template<typename Dst, typename Src>
static inline Dst ptr_cast_impl(Src p) noexcept { return (Dst)p; }

//! Helper to implement placement new/delete without relying on `<new>` header.
struct PlacementNew { void* ptr; };

} // {Support}

#if defined(ASMJIT_NO_STDCXX)
namespace Support {
  ASMJIT_INLINE void* operator_new(size_t n) noexcept { return malloc(n); }
  ASMJIT_INLINE void operator_delete(void* p) noexcept { if (p) free(p); }
} // {Support}

#define ASMJIT_BASE_CLASS(TYPE)                                                                    \
  ASMJIT_INLINE void* operator new(size_t n) noexcept { return Support::operator_new(n); }         \
  ASMJIT_INLINE void operator delete(void* ptr) noexcept { Support::operator_delete(ptr); }        \
                                                                                                   \
  ASMJIT_INLINE void* operator new(size_t, void* ptr) noexcept { return ptr; }                     \
  ASMJIT_INLINE void operator delete(void*, void*) noexcept {}                                     \
                                                                                                   \
  ASMJIT_INLINE void* operator new(size_t, Support::PlacementNew ptr) noexcept { return ptr.ptr; } \
  ASMJIT_INLINE void operator delete(void*, Support::PlacementNew) noexcept {}
#else
#define ASMJIT_BASE_CLASS(TYPE)
#endif

//! \}
//! \endcond

//! \addtogroup asmjit_core
//! \{

//! A policy that can be used with some `reset()` member functions.
enum class ResetPolicy : uint32_t {
  //! Soft reset, doesn't deallocate memory (default).
  kSoft = 0,
  //! Hard reset, releases all memory used, if any.
  kHard = 1
};

//! Contains constants and variables used globally across AsmJit.
namespace Globals {

//! Host memory allocator overhead.
static constexpr uint32_t kAllocOverhead = uint32_t(sizeof(intptr_t) * 4u);

//! Host memory allocator alignment.
static constexpr uint32_t kAllocAlignment = 8u;

//! Aggressive growing strategy threshold.
static constexpr uint32_t kGrowThreshold = 1024u * 1024u * 16u;

//! Maximum depth of RB-Tree is:
//!
//!   `2 * log2(n + 1)`
//!
//! Size of RB node is at least two pointers (without data), so a theoretical architecture limit would be:
//!
//!   `2 * log2(addressable_memory_size / sizeof(Node) + 1)`
//!
//! Which yields 30 on 32-bit arch and 61 on 64-bit arch. The final value was adjusted by +1 for safety reasons.
static constexpr uint32_t kMaxTreeHeight = (ASMJIT_ARCH_BITS == 32 ? 30 : 61) + 1;

//! Maximum number of operands per a single instruction.
static constexpr uint32_t kMaxOpCount = 6;

//! Maximum arguments of a function supported by the Compiler / Function API.
static constexpr uint32_t kMaxFuncArgs = 32;

//! The number of values that can be assigned to a single function argument or return value.
static constexpr uint32_t kMaxValuePack = 4;

//! Maximum number of physical registers AsmJit can use per register group.
static constexpr uint32_t kMaxPhysRegs = 32;

//! Maximum alignment.
static constexpr uint32_t kMaxAlignment = 64;

//! Maximum label or symbol size in bytes.
static constexpr uint32_t kMaxLabelNameSize = 2048;

//! Maximum section name size.
static constexpr uint32_t kMaxSectionNameSize = 35;

//! Maximum size of a comment.
static constexpr uint32_t kMaxCommentSize = 1024;

//! Invalid identifier.
static constexpr uint32_t kInvalidId = 0xFFFFFFFFu;

//! Invalid base address.
static constexpr uint64_t kNoBaseAddress = ~uint64_t(0);

//! Number of virtual register groups.
static constexpr uint32_t kNumVirtGroups = 4;

struct Init_ {};
struct NoInit_ {};

//! A decorator used to initialize.
static const constexpr Init_ Init {};
//! A decorator used to not initialize.
static const constexpr NoInit_ NoInit {};

template<typename T>
static ASMJIT_INLINE_NODEBUG bool is_npos(const T& index) noexcept { return index == T(~T(0)); }

} // {Globals}

//! Casts a `void*` pointer `func` to a function pointer `Func`.
template<typename Func>
static ASMJIT_INLINE_NODEBUG Func ptr_as_func(void* p) noexcept {
  return Support::ptr_cast_impl<Func, void*>(p);
}

//! Casts a `void*` pointer `func` to a function pointer `Func`.
template<typename Func>
static ASMJIT_INLINE_NODEBUG Func ptr_as_func(void* p, size_t offset) noexcept {
  return Support::ptr_cast_impl<Func, void*>(static_cast<void*>(static_cast<char*>(p) + offset));
}

//! Casts a function pointer `func` to a void pointer `void*`.
template<typename Func>
static ASMJIT_INLINE_NODEBUG void* func_as_ptr(Func func) noexcept {
  return Support::ptr_cast_impl<void*, Func>(func);
}

//! \}

//! \addtogroup asmjit_error_handling
//! \{

//! AsmJit error code.
enum class Error : uint32_t {
  // @EnumValuesBegin{"enum": "Error"}@

  //! No error (success).
  kOk = 0,

  //! Out of memory.
  kOutOfMemory,

  //! Invalid argument.
  kInvalidArgument,

  //! Invalid state.
  //!
  //! If this error is returned it means that either you are doing something wrong or AsmJit caught itself by
  //! doing something wrong. This error should never be ignored.
  kInvalidState,

  //! Invalid or incompatible architecture.
  kInvalidArch,

  //! The object is not initialized.
  kNotInitialized,
  //! The object is already initialized.
  kAlreadyInitialized,

  //! Either a built-in feature was disabled at compile time and it's not available or the feature is not
  //! available on the target platform.
  //!
  //! For example trying to allocate large pages on unsupported platform would return this error.
  kFeatureNotEnabled,

  //! Too many handles (Windows) or file descriptors (Unix/Posix).
  kTooManyHandles,
  //! Code generated is larger than allowed.
  kTooLarge,

  //! No code generated.
  //!
  //! Returned by runtime if the \ref CodeHolder contains no code.
  kNoCodeGenerated,

  //! Invalid directive.
  kInvalidDirective,
  //! Attempt to use uninitialized label.
  kInvalidLabel,
  //! Label index overflow - a single \ref BaseAssembler instance can hold almost 2^32 (4 billion) labels. If
  //! there is an attempt to create more labels then this error is returned.
  kTooManyLabels,
  //! Label is already bound.
  kLabelAlreadyBound,
  //! Label is already defined (named labels).
  kLabelAlreadyDefined,
  //! Label name is too long.
  kLabelNameTooLong,
  //! Label must always be local if it's anonymous (without a name).
  kInvalidLabelName,
  //! Parent id passed to \ref CodeHolder::new_named_label_id() was either invalid or parent is not supported by
  //! the requested `LabelType`.
  kInvalidParentLabel,

  //! Invalid section.
  kInvalidSection,
  //! Too many sections (section index overflow).
  kTooManySections,
  //! Invalid section name (most probably too long).
  kInvalidSectionName,

  //! Relocation index overflow (too many relocations).
  kTooManyRelocations,
  //! Invalid relocation entry.
  kInvalidRelocEntry,
  //! Reloc entry contains address that is out of range (unencodable).
  kRelocOffsetOutOfRange,

  //! Invalid assignment to a register, function argument, or function return value.
  kInvalidAssignment,
  //! Invalid instruction.
  kInvalidInstruction,
  //! Invalid register type.
  kInvalidRegType,
  //! Invalid register group.
  kInvalidRegGroup,
  //! Invalid physical register id.
  kInvalidPhysId,
  //! Invalid virtual register id.
  kInvalidVirtId,
  //! Invalid element index (ARM).
  kInvalidElementIndex,
  //! Invalid prefix combination (X86|X64).
  kInvalidPrefixCombination,
  //! Invalid LOCK prefix (X86|X64).
  kInvalidLockPrefix,
  //! Invalid XACQUIRE prefix (X86|X64).
  kInvalidXAcquirePrefix,
  //! Invalid XRELEASE prefix (X86|X64).
  kInvalidXReleasePrefix,
  //! Invalid REP prefix (X86|X64).
  kInvalidRepPrefix,
  //! Invalid REX prefix (X86|X64).
  kInvalidRexPrefix,
  //! Invalid {...} register (X86|X64).
  kInvalidExtraReg,
  //! Invalid {k} use (not supported by the instruction) (X86|X64).
  kInvalidKMaskUse,
  //! Invalid {k}{z} use (not supported by the instruction) (X86|X64).
  kInvalidKZeroUse,
  //! Invalid broadcast - Currently only related to invalid use of AVX-512 {1tox} (X86|X64).
  kInvalidBroadcast,
  //! Invalid 'embedded-rounding' {er} or 'suppress-all-exceptions' {sae} (AVX-512) (X86|X64).
  kInvalidEROrSAE,
  //! Invalid address used (not encodable).
  kInvalidAddress,
  //! Invalid index register used in memory address (not encodable).
  kInvalidAddressIndex,
  //! Invalid address scale (not encodable).
  kInvalidAddressScale,
  //! Invalid use of 64-bit address.
  kInvalidAddress64Bit,
  //! Invalid use of 64-bit address that require 32-bit zero-extension (X64).
  kInvalidAddress64BitZeroExtension,
  //! Invalid displacement (not encodable).
  kInvalidDisplacement,
  //! Invalid segment (X86|X86_64).
  kInvalidSegment,

  //! Invalid immediate (out of bounds on X86 and invalid pattern on ARM).
  kInvalidImmediate,

  //! Invalid operand size.
  kInvalidOperandSize,
  //! Ambiguous operand size (memory has zero size while it's required to determine the operation type.
  kAmbiguousOperandSize,
  //! Mismatching operand size (size of multiple operands doesn't match the operation size).
  kOperandSizeMismatch,

  //! Invalid option.
  kInvalidOption,
  //! Option already defined.
  kOptionAlreadyDefined,

  //! Invalid TypeId.
  kInvalidTypeId,
  //! Invalid use of a 8-bit GPB-HIGH register.
  kInvalidUseOfGpbHi,
  //! Invalid use of a 64-bit GPQ register in 32-bit mode.
  kInvalidUseOfGpq,
  //! Invalid use of an 80-bit float (\ref TypeId::kFloat80).
  kInvalidUseOfF80,
  //! Instruction requires the use of consecutive registers, but registers in operands weren't (AVX512, ASIMD load/store, etc...).
  kNotConsecutiveRegs,
  //! Failed to allocate consecutive registers - allocable registers either too restricted or a bug in RW info.
  kConsecutiveRegsAllocation,

  //! Illegal virtual register - reported by instruction validation.
  kIllegalVirtReg,
  //! AsmJit cannot create more virtual registers.
  kTooManyVirtRegs,

  //! AsmJit requires a physical register, but no one is available.
  kNoMorePhysRegs,
  //! A variable has been assigned more than once to a function argument (BaseCompiler).
  kOverlappedRegs,
  //! Invalid register to hold stack arguments offset.
  kOverlappingStackRegWithRegArg,

  //! Unbound label cannot be evaluated by expression.
  kExpressionLabelNotBound,
  //! Arithmetic overflow during expression evaluation.
  kExpressionOverflow,

  //! Failed to open anonymous memory handle or file descriptor.
  kFailedToOpenAnonymousMemory,

  //! Failed to open a file.
  //!
  //! \note This is a generic error that is used by internal filesystem API.
  kFailedToOpenFile,

  //! Protection failure can be returned from a virtual memory allocator or when trying to change memory access
  //! permissions.
  kProtectionFailure,

  // @EnumValuesEnd@

  //! Maximum value of a valid AsmJit `Error` code.
  kMaxValue = kProtectionFailure,

  //! \cond INTERNAL
  //! Error code used to inform the caller about an alternative success state.
  //!
  //! \remarks This value is only used internally in AsmJit.
  kByPass = 0xFFFFFFFFu
  //! \endcond
};

static inline constexpr Error kErrorOk = Error::kOk;

//! \def ASMJIT_ASSERT(...)
//!
//! AsmJit's own assert macro used in AsmJit code-base.
#if defined(ASMJIT_BUILD_DEBUG)
  #define ASMJIT_ASSERT(...)                                                       \
    do {                                                                           \
      if (ASMJIT_UNLIKELY(!(__VA_ARGS__))) {                                       \
        ::asmjit::DebugUtils::assertion_failure(__FILE__, __LINE__, #__VA_ARGS__); \
      }                                                                            \
    } while (0)
#else
  #define ASMJIT_ASSERT(...) ((void)0)
#endif

//! \def ASMJIT_NOT_REACHED()
//!
//! Run-time assertion used in code that should never be reached.
#if defined(ASMJIT_BUILD_DEBUG)
  #define ASMJIT_NOT_REACHED() ::asmjit::DebugUtils::assertion_failure(__FILE__, __LINE__, "ASMJIT_NOT_REACHED()")
#elif defined(__GNUC__)
  #define ASMJIT_NOT_REACHED() __builtin_unreachable()
#else
  #define ASMJIT_NOT_REACHED() ASMJIT_ASSUME(0)
#endif

//! \def ASMJIT_PROPAGATE(...)
//!
//! Propagates a possible `Error` produced by `...` to the caller by returning the error immediately. Used by AsmJit
//! internally, but kept public for users that want to use the same technique to propagate errors to the caller.
#define ASMJIT_PROPAGATE(...)                                          \
  do {                                                                 \
    ::asmjit::Error error_to_propagate = __VA_ARGS__;                  \
    if (ASMJIT_UNLIKELY(error_to_propagate != ::asmjit::Error::kOk)) { \
      return error_to_propagate;                                       \
    }                                                                  \
  } while (0)

//! \}

//! Returns the error `err` passed.
//!
//! Provided for debugging purposes. Putting a breakpoint inside `make_error` can help with tracing the origin of any
//! error reported or returned by AsmJit.
[[nodiscard]]
static constexpr Error make_error(Error err) noexcept { return err; }

//! Debugging utilities.
namespace DebugUtils {

//! Returns a printable version of `asmjit::Error` code.
[[nodiscard]]
ASMJIT_API const char* error_as_string(Error err) noexcept;

//! Called to output debugging messages.
ASMJIT_API void debug_output(const char* str) noexcept;

//! Called on assertion failure.
//!
//! \param file Source file name where it happened.
//! \param line Line in the source file.
//! \param msg Message to display.
//!
//! If you have problems with assertion failures a breakpoint can be put at \ref assertion_failure() function
//! (asmjit/core/globals.cpp). A call stack will be available when such assertion failure is triggered. AsmJit
//! always returns errors on failures, assertions are a last resort and usually mean unrecoverable state due to out
//! of range array access or totally invalid arguments like nullptr where a valid pointer should be provided, etc...
[[noreturn]]
ASMJIT_API void assertion_failure(const char* file, int line, const char* msg) noexcept;

} // {DebugUtils}

//! Output parameter.
template<typename T>
class Out {
protected:
  T& _val;

public:
  ASMJIT_INLINE_NODEBUG explicit Out(T& val) noexcept
    : _val(val) {}

  ASMJIT_INLINE_NODEBUG Out& operator=(const T& val) noexcept {
    _val = val;
    return *this;
  }

  ASMJIT_INLINE_NODEBUG T& value() const noexcept { return _val; }
  ASMJIT_INLINE_NODEBUG T& operator*() const noexcept { return _val; }
  ASMJIT_INLINE_NODEBUG T* operator->() const noexcept { return &_val; }
};

ASMJIT_END_NAMESPACE

//! Implementation of a placement new so we don't have to depend on `<new>`.
ASMJIT_INLINE_NODEBUG void* operator new(size_t, const asmjit::Support::PlacementNew& p) noexcept {
#if defined(_MSC_VER) && !defined(__clang__)
  __assume(p.ptr != nullptr); // Otherwise MSVC would emit a nullptr check.
#endif
  return p.ptr;
}

ASMJIT_INLINE_NODEBUG void operator delete(void*, const asmjit::Support::PlacementNew&) noexcept {}

#endif // ASMJIT_CORE_GLOBALS_H_INCLUDED
