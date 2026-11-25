// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_FUNC_H_INCLUDED
#define ASMJIT_CORE_FUNC_H_INCLUDED

#include <asmjit/core/archtraits.h>
#include <asmjit/core/environment.h>
#include <asmjit/core/operand.h>
#include <asmjit/core/type.h>
#include <asmjit/support/support.h>

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_function
//! \{

//! Calling convention id.
//!
//! Calling conventions can be divided into the following groups:
//!
//!   - Universal - calling conventions are applicable to any target. They will be converted to a target dependent
//!     calling convention at runtime by \ref CallConv::init() with some help from \ref Environment. The purpose of
//!     these calling conventions is to make using functions less target dependent and closer to C and C++.
//!
//!   - Target specific - calling conventions that are used by a particular architecture and ABI. For example
//!     Windows 64-bit calling convention and AMD64 SystemV calling convention.
enum class CallConvId : uint8_t {
  // Universal Calling Conventions
  // -----------------------------

  //! Standard function call or explicit `__cdecl` where it can be specified.
  //!
  //! This is a universal calling convention, which is used to initialize specific calling conventions based on
  //! architecture, platform, and its ABI.
  kCDecl = 0,

  //! `__stdcall` on targets that support this calling convention (X86).
  //!
  //! \note This calling convention is only supported on 32-bit X86. If used on environment that doesn't support
  //! this calling convention it will be replaced by \ref CallConvId::kCDecl.
  kStdCall = 1,

  //! `__fastcall` on targets that support this calling convention (X86).
  //!
  //! \note This calling convention is only supported on 32-bit X86. If used on environment that doesn't support
  //! this calling convention it will be replaced by \ref CallConvId::kCDecl.
  kFastCall = 2,

  //! `__vectorcall` on targets that support this calling convention (X86|X86_64).
  //!
  //! \note This calling convention is only supported on 32-bit and 64-bit X86 architecture on Windows platform.
  //! If used on environment that doesn't support this calling it will be replaced by \ref CallConvId::kCDecl.
  kVectorCall = 3,

  //! `__thiscall` on targets that support this calling convention (X86).
  //!
  //! \note This calling convention is only supported on 32-bit X86 Windows platform. If used on environment that
  //! doesn't support this calling convention it will be replaced by \ref CallConvId::kCDecl.
  kThisCall = 4,

  //! `__attribute__((regparm(1)))` convention (GCC and Clang).
  kRegParm1 = 5,
  //! `__attribute__((regparm(2)))` convention (GCC and Clang).
  kRegParm2 = 6,
  //! `__attribute__((regparm(3)))` convention (GCC and Clang).
  kRegParm3 = 7,

  //! AsmJit specific calling convention designed for calling functions inside a multimedia code that don't use many
  //! registers internally, but are long enough to be called and not inlined. These functions are usually used to
  //! calculate trigonometric functions, logarithms, etc...
  kLightCall2 = 16,
  kLightCall3 = 17,
  kLightCall4 = 18,

  // ABI-Specific Calling Conventions
  // --------------------------------

  //! Soft-float calling convention (AArch32).
  //!
  //! Floating point arguments are passed via general purpose registers.
  kSoftFloat = 30,

  //! Hard-float calling convention (AArch32).
  //!
  //! Floating point arguments are passed via SIMD registers.
  kHardFloat = 31,

  //! X64 System-V calling convention.
  kX64SystemV = 32,
  //! X64 Windows calling convention.
  kX64Windows = 33,

  //! Maximum value of `CallConvId`.
  kMaxValue = kX64Windows
};

//! Strategy used by calling conventions to assign registers to function arguments.
//!
//! Calling convention strategy describes how AsmJit should convert function arguments used by \ref FuncSignature
//! into register identifiers and stack offsets. The \ref CallConvStrategy::kDefault strategy assigns registers
//! and then stack whereas \ref CallConvStrategy::kX64Windows strategy does register shadowing as defined by WIN64
//! calling convention, which is only used by 64-bit Windows.
enum class CallConvStrategy : uint8_t {
  //! Default register assignment strategy.
  kDefault = 0,
  //! Windows 64-bit ABI register assignment strategy.
  kX64Windows = 1,
  //! Windows 64-bit __vectorcall register assignment strategy.
  kX64VectorCall = 2,
  //! Apple's AArch64 calling convention (differs compared to AArch64 calling convention used by Linux).
  kAArch64Apple = 3,

  //! Maximum value of `CallConvStrategy`.
  kMaxValue = kX64VectorCall
};

//! Calling convention flags.
enum class CallConvFlags : uint32_t {
  //! No flags.
  kNone = 0,
  //! Callee is responsible for cleaning up the stack.
  kCalleePopsStack = 0x0001u,
  //! Pass vector arguments indirectly (as a pointer).
  kIndirectVecArgs = 0x0002u,
  //! Pass F32 and F64 arguments via VEC128 register.
  kPassFloatsByVec = 0x0004u,
  //! Pass MMX and vector arguments via stack if the function has variable arguments.
  kPassVecByStackIfVA = 0x0008u,
  //! MMX registers are passed and returned via GP registers.
  kPassMmxByGp = 0x0010u,
  //! MMX registers are passed and returned via XMM registers.
  kPassMmxByXmm = 0x0020u,
  //! Calling convention can be used with variable arguments.
  kVarArgCompatible = 0x0080u
};
ASMJIT_DEFINE_ENUM_FLAGS(CallConvFlags)

//! Function calling convention.
//!
//! Function calling convention is a scheme that defines how function parameters are passed and how function
//! returns its result. AsmJit defines a variety of architecture and OS specific calling conventions and also
//! provides a compile time detection to make the code-generation easier.
struct CallConv {
  //! \name Constants
  //! \{

  //! Maximum number of register arguments per register group.
  //!
  //! \note This is not really AsmJit's limitation, it's just the number that makes sense considering all common
  //! calling conventions. Usually even conventions that use registers to pass function arguments are limited to 8
  //! and less arguments passed via registers per group.
  static inline constexpr uint32_t kMaxRegArgsPerGroup = 16;

  //! \}

  //! \name Members
  //! \{

  //! Target architecture.
  Arch _arch;
  //! Calling convention id.
  CallConvId _id;
  //! Register assignment strategy.
  CallConvStrategy _strategy;

  //! Red zone size (AMD64 == 128 bytes).
  uint8_t _red_zone_size;
  //! Spill zone size (WIN-X64 == 32 bytes).
  uint8_t _spill_zone_size;
  //! Natural stack alignment as defined by OS/ABI.
  uint8_t _natural_stack_alignment;

  //! \cond INTERNAL
  //! Reserved for future use.
  uint8_t _reserved[2];
  //! \endcond

  //! Calling convention flags.
  CallConvFlags _flags;

  //! Size to save/restore per register group.
  Support::Array<uint8_t, Globals::kNumVirtGroups> _save_restore_reg_size;
  //! Alignment of save/restore groups.
  Support::Array<uint8_t, Globals::kNumVirtGroups> _save_restore_alignment;

  //! Mask of all passed registers, per group.
  Support::Array<RegMask, Globals::kNumVirtGroups> _passed_regs;
  //! Mask of all preserved registers, per group.
  Support::Array<RegMask, Globals::kNumVirtGroups> _preserved_regs;

  //! Passed registers' order.
  union RegOrder {
    //! Passed registers, ordered.
    uint8_t id[kMaxRegArgsPerGroup];
    //! Packed IDs in `uint32_t` array.
    uint32_t packed[(kMaxRegArgsPerGroup + 3) / 4];
  };

  //! Passed registers' order, per register group.
  Support::Array<RegOrder, Globals::kNumVirtGroups> _passed_order;

  //! \}

  //! \name Construction & Destruction
  //! \{

  //! Initializes this calling convention to the given `call_conv_id` based on the `environment`.
  //!
  //! See \ref CallConvId and \ref Environment for more details.
  ASMJIT_API Error init(CallConvId call_conv_id, const Environment& environment) noexcept;

  //! Resets this CallConv struct into a defined state.
  //!
  //! It's recommended to reset the \ref CallConv struct in case you would like create a custom calling convention
  //! as it prevents from using an uninitialized data (CallConv doesn't have a constructor that would initialize it,
  //! it's just a struct).
  ASMJIT_INLINE_NODEBUG void reset() noexcept {
    *this = CallConv{};
    memset(_passed_order.data(), 0xFF, sizeof(_passed_order));
  }

  //! \}

  //! \name Accessors
  //! \{

  //! Returns the target architecture of this calling convention.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Arch arch() const noexcept { return _arch; }

  //! Sets the target architecture of this calling convention.
  ASMJIT_INLINE_NODEBUG void set_arch(Arch arch) noexcept { _arch = arch; }

  //! Returns the calling convention id.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG CallConvId id() const noexcept { return _id; }

  //! Sets the calling convention id.
  ASMJIT_INLINE_NODEBUG void set_id(CallConvId call_conv_id) noexcept { _id = call_conv_id; }

  //! Returns the strategy used to assign registers to arguments.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG CallConvStrategy strategy() const noexcept { return _strategy; }

  //! Sets the strategy used to assign registers to arguments.
  ASMJIT_INLINE_NODEBUG void set_strategy(CallConvStrategy strategy) noexcept { _strategy = strategy; }

  //! Tests whether the calling convention has the given `flag` set.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_flag(CallConvFlags flag) const noexcept { return Support::test(_flags, flag); }

  //! Returns the calling convention flags, see `Flags`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG CallConvFlags flags() const noexcept { return _flags; }

  //! Adds the calling convention flags, see `Flags`.
  ASMJIT_INLINE_NODEBUG void set_flags(CallConvFlags flag) noexcept { _flags = flag; };

  //! Adds the calling convention flags, see `Flags`.
  ASMJIT_INLINE_NODEBUG void add_flags(CallConvFlags flags) noexcept { _flags |= flags; };

  //! Tests whether this calling convention specifies 'Red Zone'.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_red_zone() const noexcept { return _red_zone_size != 0; }

  //! Tests whether this calling convention specifies 'Spill Zone'.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_spill_zone() const noexcept { return _spill_zone_size != 0; }

  //! Returns size of 'Red Zone'.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t red_zone_size() const noexcept { return _red_zone_size; }

  //! Sets size of 'Red Zone'.
  ASMJIT_INLINE_NODEBUG void set_red_zone_size(uint32_t size) noexcept { _red_zone_size = uint8_t(size); }

  //! Returns size of 'Spill Zone'.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t spill_zone_size() const noexcept { return _spill_zone_size; }

  //! Sets size of 'Spill Zone'.
  ASMJIT_INLINE_NODEBUG void set_spill_zone_size(uint32_t size) noexcept { _spill_zone_size = uint8_t(size); }

  //! Returns a natural stack alignment.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t natural_stack_alignment() const noexcept { return _natural_stack_alignment; }

  //! Sets a natural stack alignment.
  //!
  //! This function can be used to override the default stack alignment in case that you know that it's alignment is
  //! different. For example it allows to implement custom calling conventions that guarantee higher stack alignment.
  ASMJIT_INLINE_NODEBUG void set_natural_stack_alignment(uint32_t value) noexcept { _natural_stack_alignment = uint8_t(value); }

  //! Returns the size of a register (or its part) to be saved and restored of the given `group`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t save_restore_reg_size(RegGroup group) const noexcept { return _save_restore_reg_size[group]; }

  //! Sets the size of a vector register (or its part) to be saved and restored.
  ASMJIT_INLINE_NODEBUG void set_save_restore_reg_size(RegGroup group, uint32_t size) noexcept { _save_restore_reg_size[group] = uint8_t(size); }

  //! Returns the alignment of a save-restore area of the given `group`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t save_restore_alignment(RegGroup group) const noexcept { return _save_restore_alignment[group]; }

  //! Sets the alignment of a save-restore area of the given `group`.
  ASMJIT_INLINE_NODEBUG void set_save_restore_alignment(RegGroup group, uint32_t alignment) noexcept { _save_restore_alignment[group] = uint8_t(alignment); }

  //! Returns the order of passed registers of the given `group`.
  [[nodiscard]]
  ASMJIT_INLINE const uint8_t* passed_order(RegGroup group) const noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    return _passed_order[size_t(group)].id;
  }

  //! Returns the mask of passed registers of the given `group`.
  [[nodiscard]]
  ASMJIT_INLINE RegMask passed_regs(RegGroup group) const noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    return _passed_regs[size_t(group)];
  }

  ASMJIT_INLINE void _set_passed_as_packed(RegGroup group, uint32_t p0, uint32_t p1, uint32_t p2, uint32_t p3) noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);

    _passed_order[group].packed[0] = p0;
    _passed_order[group].packed[1] = p1;
    _passed_order[group].packed[2] = p2;
    _passed_order[group].packed[3] = p3;
  }

  //! Resets the order and mask of passed registers.
  ASMJIT_INLINE void set_passed_to_none(RegGroup group) noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);

    _set_passed_as_packed(group, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu);
    _passed_regs[size_t(group)] = 0u;
  }

  //! Sets the order and mask of passed registers.
  ASMJIT_INLINE void set_passed_order(RegGroup group, uint32_t a0, uint32_t a1 = 0xFF, uint32_t a2 = 0xFF, uint32_t a3 = 0xFF, uint32_t a4 = 0xFF, uint32_t a5 = 0xFF, uint32_t a6 = 0xFF, uint32_t a7 = 0xFF) noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);

    // NOTE: This should always be called with all arguments known at compile time, so even if it looks scary it
    // should be translated into few instructions.
    _set_passed_as_packed(group, Support::bytepack32_4x8(a0, a1, a2, a3),
                                 Support::bytepack32_4x8(a4, a5, a6, a7),
                                 0xFFFFFFFFu,
                                 0xFFFFFFFFu);

    _passed_regs[group] = (a0 != 0xFF ? 1u << a0 : 0u) |
                         (a1 != 0xFF ? 1u << a1 : 0u) |
                         (a2 != 0xFF ? 1u << a2 : 0u) |
                         (a3 != 0xFF ? 1u << a3 : 0u) |
                         (a4 != 0xFF ? 1u << a4 : 0u) |
                         (a5 != 0xFF ? 1u << a5 : 0u) |
                         (a6 != 0xFF ? 1u << a6 : 0u) |
                         (a7 != 0xFF ? 1u << a7 : 0u) ;
  }

  //! Returns preserved register mask of the given `group`.
  [[nodiscard]]
  ASMJIT_INLINE RegMask preserved_regs(RegGroup group) const noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    return _preserved_regs[group];
  }

  //! Sets preserved register mask of the given `group`.
  ASMJIT_INLINE void set_preserved_regs(RegGroup group, RegMask regs) noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    _preserved_regs[group] = regs;
  }

  //! \}
};

//! Function signature.
//!
//! Contains information about a function return type, count of arguments, and their TypeIds. Function signature
//! is a low level structure which doesn't contain platform specific or calling convention specific information.
//! It's typically used to describe function arguments in a C-API like form, which is then used to calculate a
//! \ref FuncDetail instance, which then maps function signature into a platform and calling convention specific
//! format.
//!
//! Function signature can be built either dynamically by using \ref add_arg() and \ref add_arg_t() functionality,
//! or dynamically by using a template-based \ref FuncSignature::build() function, which maps template types
//! into a function signature.
struct FuncSignature {
  //! \name Constants
  //! \{

  //! Doesn't have variable number of arguments (`...`).
  static inline constexpr uint8_t kNoVarArgs = 0xFFu;

  //! \}

  //! \name Members
  //! \{

  //! Calling convention id.
  CallConvId _call_conv_id = CallConvId::kCDecl;
  //! Count of arguments.
  uint8_t _arg_count = 0;
  //! Index of a first VA or `kNoVarArgs`.
  uint8_t _va_index = kNoVarArgs;
  //! Return value TypeId.
  TypeId _ret = TypeId::kVoid;
  //! Reserved for future use.
  uint8_t _reserved[4] {};
  //! Function argument TypeIds.
  TypeId _args[Globals::kMaxFuncArgs] {};

  //! \}

  //! \name Construction & Destruction
  //! \{

  //! Default constructed function signature, initialized to \ref CallConvId::kCDecl, having no return value and no arguments.
  ASMJIT_INLINE_CONSTEXPR FuncSignature() = default;

  //! Copy constructor, which is initialized to the same function signature as `other`.
  ASMJIT_INLINE_CONSTEXPR FuncSignature(const FuncSignature& other) = default;

  //! Initializes the function signature with calling convention id `call_conv_id` and variable argument's index `va_index`.
  ASMJIT_INLINE_CONSTEXPR FuncSignature(CallConvId call_conv_id, uint32_t va_index = kNoVarArgs) noexcept
    : _call_conv_id(call_conv_id),
      _va_index(uint8_t(va_index)) {}

  //! Initializes the function signature with calling convention id `call_conv_id`, `va_index`, return value, and function arguments.
  template<typename... Args>
  ASMJIT_INLINE_CONSTEXPR FuncSignature(CallConvId call_conv_id, uint32_t va_index, TypeId ret, Args&&...args) noexcept
    : _call_conv_id(call_conv_id),
      _arg_count(uint8_t(sizeof...(args))),
      _va_index(uint8_t(va_index)),
      _ret(ret),
      _args{std::forward<Args>(args)...} {}

  //! Builds a function signature based on `RetValueAndArgs`. The first template argument is a function return type,
  //! and function arguments follow.
  //!
  //! \note This function returns a new function signature, which can be passed to functions where it's required. It's
  //! a convenience function that allows to build function signature statically based on types known at compile time,
  //! which is common in JIT code generation.
  template<typename... RetValueAndArgs>
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR FuncSignature build(CallConvId call_conv_id = CallConvId::kCDecl, uint32_t va_index = kNoVarArgs) noexcept {
    return FuncSignature(call_conv_id, va_index, (TypeId(TypeUtils::TypeIdOfT<RetValueAndArgs>::kTypeId))... );
  }

  //! \}

  //! \name Overloaded Operators
  //! \{

  //! Copy assignment - function signature can be copied by value.
  ASMJIT_INLINE FuncSignature& operator=(const FuncSignature& other) noexcept = default;

  //! Compares this function signature with `other` for equality..
  [[nodiscard]]
  ASMJIT_INLINE bool operator==(const FuncSignature& other) const noexcept { return equals(other); }

  //! Compares this function signature with `other` for inequality..
  [[nodiscard]]
  ASMJIT_INLINE bool operator!=(const FuncSignature& other) const noexcept { return !equals(other); }

  //! \}

  //! \name Initialization & Reset
  //! \{

  //! Resets this function signature to a default constructed state.
  ASMJIT_INLINE_NODEBUG void reset() noexcept { *this = FuncSignature{}; }

  //! \}

  //! \name Equality & Comparison
  //! \{

  //! Compares this function signature with `other` for equality..
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool equals(const FuncSignature& other) const noexcept {
    return _call_conv_id == other._call_conv_id &&
           _arg_count == other._arg_count &&
           _va_index == other._va_index &&
           _ret == other._ret &&
           memcmp(_args, other._args, sizeof(_args)) == 0;
  }

  //! \}

  //! \name Accessors
  //! \{

  //! Returns the calling convention.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR CallConvId call_conv_id() const noexcept { return _call_conv_id; }

  //! Sets the calling convention to `call_conv_id`;
  ASMJIT_INLINE_CONSTEXPR void set_call_conv_id(CallConvId call_conv_id) noexcept { _call_conv_id = call_conv_id; }

  //! Tests whether the function signature has a return value.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool has_ret() const noexcept { return _ret != TypeId::kVoid; }

  //! Returns the type of the return value.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR TypeId ret() const noexcept { return _ret; }

  //! Sets the return type to `ret_type`.
  ASMJIT_INLINE_CONSTEXPR void set_ret(TypeId ret_type) noexcept { _ret = ret_type; }

  //! Sets the return type based on `T`.
  template<typename T>
  ASMJIT_INLINE_CONSTEXPR void set_ret_t() noexcept { set_ret(TypeId(TypeUtils::TypeIdOfT<T>::kTypeId)); }

  //! Returns the array of function arguments' types.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR const TypeId* args() const noexcept { return _args; }

  //! Returns the number of function arguments.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR uint32_t arg_count() const noexcept { return _arg_count; }

  //! Returns the type of the argument at index `i`.
  [[nodiscard]]
  ASMJIT_INLINE TypeId arg(uint32_t i) const noexcept {
    ASMJIT_ASSERT(i < _arg_count);
    return _args[i];
  }

  //! Sets the argument at index `index` to `arg_type`.
  ASMJIT_INLINE void set_arg(uint32_t index, TypeId arg_type) noexcept {
    ASMJIT_ASSERT(index < _arg_count);
    _args[index] = arg_type;
  }

  //! Sets the argument at index `i` to the type based on `T`.
  template<typename T>
  ASMJIT_INLINE void set_arg_t(uint32_t index) noexcept { set_arg(index, TypeId(TypeUtils::TypeIdOfT<T>::kTypeId)); }

  //! Tests whether an argument can be added to the signature, use before calling \ref add_arg() and \ref add_arg_t().
  //!
  //! \note If you know that you are not adding more arguments than \ref Globals::kMaxFuncArgs then it's not necessary
  //! to use this function. However, if you are adding arguments based on user input, for example, then either check
  //! the number of arguments before using function signature or use \ref can_add_arg() before actually adding them to
  //! the function signature.
  [[nodiscard]]
  ASMJIT_INLINE bool can_add_arg() const noexcept { return _arg_count < Globals::kMaxFuncArgs; }

  //! Appends an argument of `type` to the function prototype.
  ASMJIT_INLINE void add_arg(TypeId type) noexcept {
    ASMJIT_ASSERT(_arg_count < Globals::kMaxFuncArgs);
    _args[_arg_count++] = type;
  }

  //! Appends an argument of type based on `T` to the function prototype.
  template<typename T>
  ASMJIT_INLINE void add_arg_t() noexcept { add_arg(TypeId(TypeUtils::TypeIdOfT<T>::kTypeId)); }

  //! Tests whether the function has variable number of arguments (...).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_var_args() const noexcept { return _va_index != kNoVarArgs; }

  //! Returns the variable arguments (...) index, `kNoVarArgs` if none.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t va_index() const noexcept { return _va_index; }

  //! Sets the variable arguments (...) index to `index`.
  ASMJIT_INLINE_NODEBUG void set_va_index(uint32_t index) noexcept { _va_index = uint8_t(index); }

  //! Resets the variable arguments index (making it a non-va function).
  ASMJIT_INLINE_NODEBUG void reset_va_index() noexcept { _va_index = kNoVarArgs; }

  //! \}
};

//! Argument or return value (or its part) as defined by `FuncSignature`, but with register or stack address
//! (and other metadata) assigned.
struct FuncValue {
  //! \name Constants
  //! \{

  enum Bits : uint32_t {
    kTypeIdShift      = 0,             //!< TypeId shift.
    kTypeIdMask       = 0x000000FFu,   //!< TypeId mask.

    kFlagIsReg        = 0x00000100u,   //!< Passed by register.
    kFlagIsStack      = 0x00000200u,   //!< Passed by stack.
    kFlagIsIndirect   = 0x00000400u,   //!< Passed indirectly by reference (internally a pointer).
    kFlagIsDone       = 0x00000800u,   //!< Used internally by arguments allocator.

    kStackOffsetShift = 12,            //!< Stack offset shift.
    kStackOffsetMask  = 0xFFFFF000u,   //!< Stack offset mask (must occupy MSB bits).

    kRegIdShift       = 16,            //!< RegId shift.
    kRegIdMask        = 0x00FF0000u,   //!< RegId mask.

    kRegTypeShift     = 24,            //!< RegType shift.
    kRegTypeMask      = 0xFF000000u    //!< RegType mask.
  };

  //! \}

  //! \name Members
  //! \{

  uint32_t _data;

  //! \}

  //! \name Initialization & Reset
  //!
  //! These initialize the whole `FuncValue` to either register or stack. Useful when you know all of these
  //! properties and wanna just set it up.
  //!
  //! \{

  //! Initializes this `FuncValue` only to the `type_id` provided - the rest of the values will be cleared.
  ASMJIT_INLINE_NODEBUG void init_type_id(TypeId type_id) noexcept {
    _data = uint32_t(type_id) << kTypeIdShift;
  }

  //! Initializes this `FuncValue` to a register of `reg_type`, `reg_id`, and assigns its `type_id` and `flags`.
  ASMJIT_INLINE_NODEBUG void init_reg(RegType reg_type, uint32_t reg_id, TypeId type_id, uint32_t flags = 0) noexcept {
    _data = (uint32_t(reg_type) << kRegTypeShift) | (reg_id << kRegIdShift) | (uint32_t(type_id) << kTypeIdShift) | kFlagIsReg | flags;
  }

  //! Initializes this `FuncValue` to a stack at the given `offset` and assigns its `type_id`.
  ASMJIT_INLINE_NODEBUG void init_stack(int32_t offset, TypeId type_id) noexcept {
    _data = (uint32_t(offset) << kStackOffsetShift) | (uint32_t(type_id) << kTypeIdShift) | kFlagIsStack;
  }

  //! Resets the value to its unassigned state.
  ASMJIT_INLINE_NODEBUG void reset() noexcept { _data = 0; }

  //! \}

  //! \name Assign
  //!
  //! These initialize only part of `FuncValue`, useful when building `FuncValue` incrementally. The caller
  //! should first init the type-id by calling `init_type_id` and then continue building either register or stack.
  //!
  //! \{

  //! Assigns a register of `reg_type` and `reg_id`.
  ASMJIT_INLINE void assign_reg_data(RegType reg_type, uint32_t reg_id) noexcept {
    ASMJIT_ASSERT((_data & (kRegTypeMask | kRegIdMask)) == 0);
    _data |= (uint32_t(reg_type) << kRegTypeShift) | (reg_id << kRegIdShift) | kFlagIsReg;
  }

  //! Assigns a stack location at `offset`.
  ASMJIT_INLINE void assign_stack_offset(int32_t offset) noexcept {
    ASMJIT_ASSERT((_data & kStackOffsetMask) == 0);
    _data |= (uint32_t(offset) << kStackOffsetShift) | kFlagIsStack;
  }

  //! \}

  //! \name Accessors
  //! \{

  //! Returns true if the value is initialized (explicit bool cast).
  ASMJIT_INLINE_NODEBUG explicit operator bool() const noexcept { return _data != 0; }

  //! \cond INTERNAL
  ASMJIT_INLINE_NODEBUG void _replace_value(uint32_t mask, uint32_t value) noexcept { _data = (_data & ~mask) | value; }
  //! \endcond

  //! Tests whether the `FuncValue` has a flag `flag` set.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_flag(uint32_t flag) const noexcept { return Support::test(_data, flag); }

  //! Adds `flags` to `FuncValue`.
  ASMJIT_INLINE_NODEBUG void add_flags(uint32_t flags) noexcept { _data |= flags; }

  //! Clears `flags` of `FuncValue`.
  ASMJIT_INLINE_NODEBUG void clear_flags(uint32_t flags) noexcept { _data &= ~flags; }

  //! Tests whether the value is initialized (i.e. contains a valid data).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_initialized() const noexcept { return _data != 0; }

  //! Tests whether the argument is passed by register.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_reg() const noexcept { return has_flag(kFlagIsReg); }

  //! Tests whether the argument is passed by stack.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_stack() const noexcept { return has_flag(kFlagIsStack); }

  //! Tests whether the argument is passed by register.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_assigned() const noexcept { return has_flag(kFlagIsReg | kFlagIsStack); }

  //! Tests whether the argument is passed through a pointer (used by WIN64 to pass XMM|YMM|ZMM).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_indirect() const noexcept { return has_flag(kFlagIsIndirect); }

  //! Tests whether the argument was already processed (used internally).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_done() const noexcept { return has_flag(kFlagIsDone); }

  //! Returns a register type of the register used to pass function argument or return value.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RegType reg_type() const noexcept { return RegType((_data & kRegTypeMask) >> kRegTypeShift); }

  //! Sets a register type of the register used to pass function argument or return value.
  ASMJIT_INLINE_NODEBUG void set_reg_type(RegType reg_type) noexcept { _replace_value(kRegTypeMask, uint32_t(reg_type) << kRegTypeShift); }

  //! Returns a physical id of the register used to pass function argument or return value.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t reg_id() const noexcept { return (_data & kRegIdMask) >> kRegIdShift; }

  //! Sets a physical id of the register used to pass function argument or return value.
  ASMJIT_INLINE_NODEBUG void set_reg_id(uint32_t reg_id) noexcept { _replace_value(kRegIdMask, reg_id << kRegIdShift); }

  //! Returns a stack offset of this argument.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG int32_t stack_offset() const noexcept { return int32_t(_data & kStackOffsetMask) >> kStackOffsetShift; }

  //! Sets a stack offset of this argument.
  ASMJIT_INLINE_NODEBUG void set_stack_offset(int32_t offset) noexcept { _replace_value(kStackOffsetMask, uint32_t(offset) << kStackOffsetShift); }

  //! Tests whether the argument or return value has associated `TypeId`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_type_id() const noexcept { return Support::test(_data, kTypeIdMask); }

  //! Returns a TypeId of this argument or return value.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG TypeId type_id() const noexcept { return TypeId((_data & kTypeIdMask) >> kTypeIdShift); }

  //! Sets a TypeId of this argument or return value.
  ASMJIT_INLINE_NODEBUG void set_type_id(TypeId type_id) noexcept { _replace_value(kTypeIdMask, uint32_t(type_id) << kTypeIdShift); }

  //! \}
};

//! Contains multiple `FuncValue` instances in an array so functions that use multiple registers for arguments or
//! return values can represent all inputs and outputs.
struct FuncValuePack {
public:
  //! \name Members
  //! \{

  //! Values of the pack.
  FuncValue _values[Globals::kMaxValuePack];

  //! \}

  //! \name Initialization & Reset
  //! \{

  //! Resets all values in the pack.
  ASMJIT_INLINE void reset() noexcept {
    for (FuncValue& value : _values) {
      value.reset();
    }
  }

  //! \}

  //! \name Accessors
  //! \{

  //! Calculates how many values are in the pack, checking for non-values from the end.
  [[nodiscard]]
  ASMJIT_INLINE uint32_t count() const noexcept {
    uint32_t n = Globals::kMaxValuePack;
    while (n && !_values[n - 1])
      n--;
    return n;
  }

  //! Returns values in this value in the pack.
  //!
  //! \note The returned array has exactly \ref Globals::kMaxValuePack elements.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG FuncValue* values() noexcept { return _values; }

  //! \overload
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const FuncValue* values() const noexcept { return _values; }

  //! Resets a value at the given `index` in the pack, which makes it unassigned.
  ASMJIT_INLINE void reset_value(size_t index) noexcept {
    ASMJIT_ASSERT(index < Globals::kMaxValuePack);
    _values[index].reset();
  }

  //! Tests whether the value at the given `index` in the pack is assigned.
  ASMJIT_INLINE bool has_value(size_t index) noexcept {
    ASMJIT_ASSERT(index < Globals::kMaxValuePack);
    return _values[index].is_initialized();
  }

  //! Assigns a register at the given `index` to `reg` and an optional `type_id`.
  ASMJIT_INLINE void assign_reg(size_t index, const Reg& reg, TypeId type_id = TypeId::kVoid) noexcept {
    ASMJIT_ASSERT(index < Globals::kMaxValuePack);
    ASMJIT_ASSERT(reg.is_phys_reg());
    _values[index].init_reg(reg.reg_type(), reg.id(), type_id);
  }

  //! Assigns a register at the given `index` to `reg_type`, `reg_id`, and an optional `type_id`.
  ASMJIT_INLINE void assign_reg(size_t index, RegType reg_type, uint32_t reg_id, TypeId type_id = TypeId::kVoid) noexcept {
    ASMJIT_ASSERT(index < Globals::kMaxValuePack);
    _values[index].init_reg(reg_type, reg_id, type_id);
  }

  //! Assigns a stack location at the given `index` to `offset` and an optional `type_id`.
  ASMJIT_INLINE void assign_stack(size_t index, int32_t offset, TypeId type_id = TypeId::kVoid) noexcept {
    ASMJIT_ASSERT(index < Globals::kMaxValuePack);
    _values[index].init_stack(offset, type_id);
  }

  //! Accesses the value in the pack at the given `index`.
  //!
  //! \note The maximum index value is `Globals::kMaxValuePack - 1`.
  [[nodiscard]]
  ASMJIT_INLINE FuncValue& operator[](size_t index) {
    ASMJIT_ASSERT(index < Globals::kMaxValuePack);
    return _values[index];
  }

  //! \overload
  [[nodiscard]]
  ASMJIT_INLINE const FuncValue& operator[](size_t index) const {
    ASMJIT_ASSERT(index < Globals::kMaxValuePack);
    return _values[index];
  }

  //! \}
};

//! Attributes are designed in a way that all are initially false, and user or \ref FuncFrame finalizer adds
//! them when necessary.
enum class FuncAttributes : uint32_t {
  //! No attributes.
  kNoAttributes = 0,

  //! Function has variable number of arguments.
  kHasVarArgs = 0x00000001u,
  //! Preserve frame pointer (don't omit FP).
  kHasPreservedFP = 0x00000010u,
  //! Function calls other functions (is not leaf).
  kHasFuncCalls = 0x00000020u,
  //! Function has aligned save/restore of vector registers.
  kAlignedVecSR = 0x00000040u,
  //! Function must begin with an instruction that marks a start of a branch or function.
  //!
  //! - `ENDBR32/ENDBR64` instruction is inserted at the beginning of the function (X86|X86_64).
  //! - `BTI` instruction is inserted at the beginning of the function (AArch64).
  kIndirectBranchProtection = 0x00000080u,
  //! FuncFrame is finalized and can be used by prolog/epilog inserter (PEI).
  kIsFinalized = 0x00000800u,

  // X86 Specific Attributes
  // -----------------------

  //! Enables the use of AVX within the function's body, prolog, and epilog (X86|X86_64).
  //!
  //! This flag instructs prolog and epilog emitter to use AVX instead of SSE for manipulating XMM registers.
  kX86_AVXEnabled = 0x00010000u,

  //! Enables the use of AVX-512 within the function's body, prolog, and epilog (X86|X86_64).
  //!
  //! This flag instructs Compiler register allocator to use additional 16 registers introduced by AVX-512.
  //! Additionally, if the functions saves full width of ZMM registers (custom calling conventions only) then
  //! the prolog/epilog inserter would use AVX-512 move instructions to emit the save and restore sequence.
  kX86_AVX512Enabled = 0x00020000u,

  //! This flag instructs the epilog writer to emit EMMS instruction before RET (X86|X86_64).
  kX86_MMXCleanup = 0x00040000u,

  //! This flag instructs the epilog writer to emit VZEROUPPER instruction before RET (X86|X86_64).
  kX86_AVXCleanup = 0x00080000u,

  //! This flag instructs the epilog writer to emit VZEROUPPER only if there are dirty vector registers (X86|X86_64).
  kX86_AVXAutoCleanup = 0x00100000u
};
ASMJIT_DEFINE_ENUM_FLAGS(FuncAttributes)

//! Function detail - \ref CallConv and expanded \ref FuncSignature.
//!
//! Function detail is architecture and OS dependent representation of a function. It contains a materialized
//! calling convention and expanded function signature so all arguments have assigned either register type/id
//! or stack address.
class FuncDetail {
public:
  //! \name Constants
  //! \{

  //! Function doesn't have a variable number of arguments (`...`).
  static inline constexpr uint8_t kNoVarArgs = 0xFFu;

  //! \}

  //! \name Members
  //! \{

  //! Calling convention.
  CallConv _call_conv {};
  //! Number of function arguments.
  uint8_t _arg_count = 0;
  //! Variable arguments index of `kNoVarArgs`.
  uint8_t _va_index = 0;
  //! Reserved for future use.
  uint16_t _reserved = 0;
  //! Registers that contain arguments.
  Support::Array<RegMask, Globals::kNumVirtGroups> _used_regs {};
  //! Size of arguments passed by stack.
  uint32_t _arg_stack_size = 0;
  //! Function return value(s).
  FuncValuePack _rets {};
  //! Function arguments.
  FuncValuePack _args[Globals::kMaxFuncArgs] {};

  //! \}

  //! \name Construction & Destruction
  //! \{

  //! Creates a default constructed \ref FuncDetail.
  ASMJIT_INLINE_NODEBUG FuncDetail() noexcept {}

  //! Copy constructor.
  //!
  //! Function details are copyable.
  ASMJIT_INLINE_NODEBUG FuncDetail(const FuncDetail& other) noexcept = default;

  //! Initializes this `FuncDetail` to the given signature.
  ASMJIT_API Error init(const FuncSignature& signature, const Environment& environment) noexcept;

  //! \}

  //! \name Overloaded Operators
  //! \{

  //! Assignment operator, copies `other` to this \ref FuncDetail.
  ASMJIT_INLINE_NODEBUG FuncDetail& operator=(const FuncDetail& other) noexcept = default;

  //! \}

  //! \name Reset
  //! \{

  //! Resets the function detail to its default constructed state.
  ASMJIT_INLINE_NODEBUG void reset() noexcept { *this = FuncDetail{}; }

  //! \}

  //! \name Accessors
  //! \{

  //! Returns the function's calling convention, see `CallConv`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const CallConv& call_conv() const noexcept { return _call_conv; }

  //! Returns the associated calling convention flags, see `CallConv::Flags`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG CallConvFlags flags() const noexcept { return _call_conv.flags(); }

  //! Checks whether a CallConv `flag` is set, see `CallConv::Flags`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_flag(CallConvFlags flag) const noexcept { return _call_conv.has_flag(flag); }

  //! Tests whether the function has a return value.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_ret() const noexcept { return bool(_rets[0]); }

  //! Returns the number of function arguments.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t arg_count() const noexcept { return _arg_count; }

  //! Returns function return values.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG FuncValuePack& ret_pack() noexcept { return _rets; }

  //! Returns function return values.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const FuncValuePack& ret_pack() const noexcept { return _rets; }

  //! Returns a function return value associated with the given `value_index`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG FuncValue& ret(size_t value_index = 0) noexcept { return _rets[value_index]; }

  //! Returns a function return value associated with the given `value_index` (const).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const FuncValue& ret(size_t value_index = 0) const noexcept { return _rets[value_index]; }

  //! Returns function argument packs array.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG FuncValuePack* arg_packs() noexcept { return _args; }

  //! Returns function argument packs array (const).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const FuncValuePack* arg_packs() const noexcept { return _args; }

  //! Returns function argument pack at the given `arg_index`.
  [[nodiscard]]
  ASMJIT_INLINE FuncValuePack& arg_pack(size_t arg_index) noexcept {
    ASMJIT_ASSERT(arg_index < Globals::kMaxFuncArgs);
    return _args[arg_index];
  }

  //! Returns function argument pack at the given `arg_index` (const).
  [[nodiscard]]
  ASMJIT_INLINE const FuncValuePack& arg_pack(size_t arg_index) const noexcept {
    ASMJIT_ASSERT(arg_index < Globals::kMaxFuncArgs);
    return _args[arg_index];
  }

  //! Returns an argument at `value_index` from the argument pack at the given `arg_index`.
  [[nodiscard]]
  ASMJIT_INLINE FuncValue& arg(size_t arg_index, size_t value_index = 0) noexcept {
    ASMJIT_ASSERT(arg_index < Globals::kMaxFuncArgs);
    return _args[arg_index][value_index];
  }

  //! Returns an argument at `value_index` from the argument pack at the given `arg_index` (const).
  [[nodiscard]]
  ASMJIT_INLINE const FuncValue& arg(size_t arg_index, size_t value_index = 0) const noexcept {
    ASMJIT_ASSERT(arg_index < Globals::kMaxFuncArgs);
    return _args[arg_index][value_index];
  }

  //! Resets an argument at the given `arg_index`.
  //!
  //! If the argument is a parameter pack (has multiple values) all values are reset.
  ASMJIT_INLINE void reset_arg(size_t arg_index) noexcept {
    ASMJIT_ASSERT(arg_index < Globals::kMaxFuncArgs);
    _args[arg_index].reset();
  }

  //! Tests whether the function has variable arguments.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_var_args() const noexcept { return _va_index != kNoVarArgs; }

  //! Returns an index of a first variable argument.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t va_index() const noexcept { return _va_index; }

  //! Tests whether the function passes one or more argument by stack.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_stack_args() const noexcept { return _arg_stack_size != 0; }

  //! Returns stack size needed for function arguments passed on the stack.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t arg_stack_size() const noexcept { return _arg_stack_size; }

  //! Returns red zone size.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t red_zone_size() const noexcept { return _call_conv.red_zone_size(); }

  //! Returns spill zone size.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t spill_zone_size() const noexcept { return _call_conv.spill_zone_size(); }

  //! Returns natural stack alignment.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t natural_stack_alignment() const noexcept { return _call_conv.natural_stack_alignment(); }

  //! Returns a mask of all passed registers of the given register `group`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RegMask passed_regs(RegGroup group) const noexcept { return _call_conv.passed_regs(group); }

  //! Returns a mask of all preserved registers of the given register `group`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RegMask preserved_regs(RegGroup group) const noexcept { return _call_conv.preserved_regs(group); }

  //! Returns a mask of all used registers of the given register `group`.
  [[nodiscard]]
  ASMJIT_INLINE RegMask used_regs(RegGroup group) const noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    return _used_regs[size_t(group)];
  }

  //! Adds `regs` to the mask of used registers of the given register `group`.
  ASMJIT_INLINE void add_used_regs(RegGroup group, RegMask regs) noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    _used_regs[size_t(group)] |= regs;
  }

  //! \}
};

//! Function frame.
//!
//! Function frame is used directly by prolog and epilog insertion (PEI) utils. It provides information necessary to
//! insert a proper and ABI conforming prolog and epilog. Function frame calculation is based on `CallConv` and
//! other function attributes.
//!
//! SSE vs AVX vs AVX-512
//! ---------------------
//!
//! Function frame provides a way to tell prolog/epilog inserter to use AVX instructions instead of SSE. Use
//! `set_avx_enabled()` and `set_avx512_enabled()`  to enable AVX and/or AVX-512, respectively. Enabling AVX-512
//! is mostly for Compiler as it would use 32 SIMD registers instead of 16 when enabled.
//!
//! \note If your code uses AVX instructions and AVX is not enabled there would be a performance hit in case that
//! some registers had to be saved/restored in function's prolog/epilog, respectively. Thus, it's recommended to
//! always let the function frame know about the use of AVX.
//!
//! Function Frame Structure
//! ------------------------
//!
//! Various properties can contribute to the size and structure of the function frame. The function frame in most
//! cases won't use all of the properties illustrated (for example Spill Zone and Red Zone are never used together).
//!
//! ```
//!   +-----------------------------+
//!   | Arguments Passed by Stack   |
//!   +-----------------------------+
//!   | Spill Zone                  |
//!   +-----------------------------+ <- Stack offset (args) starts from here.
//!   | Return Address, if Pushed   |
//!   +-----------------------------+ <- Stack pointer (SP) upon entry.
//!   | Save/Restore Stack.         |
//!   +-----------------------------+-----------------------------+
//!   | Local Stack                 |                             |
//!   +-----------------------------+          Final Stack        |
//!   | Call Stack                  |                             |
//!   +-----------------------------+-----------------------------+ <- SP after prolog.
//!   | Red Zone                    |
//!   +-----------------------------+
//! ```
class FuncFrame {
public:
  //! \name Constants
  //! \{

  //! Tag used to inform that some offset is invalid.
  static inline constexpr uint32_t kTagInvalidOffset = 0xFFFFFFFFu;

  //! \}

  //! \name Types
  //! \{

  using RegMasks = Support::Array<RegMask, Globals::kNumVirtGroups>;

  //! \}

  //! \name Members
  //! \{

  //! Function attributes.
  FuncAttributes _attributes {};

  //! Target architecture.
  Arch _arch {};
  //! SP register ID (to access call stack and local stack).
  uint8_t _sp_reg_id = uint8_t(Reg::kIdBad);
  //! SA register ID (to access stack arguments).
  uint8_t _sa_reg_id = uint8_t(Reg::kIdBad);

  //! Red zone size (copied from CallConv).
  uint8_t _red_zone_size = 0;
  //! Spill zone size (copied from CallConv).
  uint8_t _spill_zone_size = 0;
  //! Natural stack alignment (copied from CallConv).
  uint8_t _natural_stack_alignment = 0;
  //! Minimum stack alignment to turn on dynamic alignment.
  uint8_t _min_dynamic_alignment = 0;

  //! Call stack alignment.
  uint8_t _call_stack_alignment = 0;
  //! Local stack alignment.
  uint8_t _local_stack_alignment = 0;
  //! Final stack alignment.
  uint8_t _final_stack_alignment = 0;

  //! Adjustment of the stack before returning (X86-STDCALL).
  uint16_t _callee_stack_cleanup = 0;

  //! Call stack size.
  uint32_t _call_stack_size = 0;
  //! Local stack size.
  uint32_t _local_stack_size = 0;
  //! Final stack size (sum of call stack and local stack).
  uint32_t _final_stack_size = 0;

  //! Local stack offset (non-zero only if call stack is used).
  uint32_t _local_stack_offset = 0;
  //! Offset relative to SP that contains previous SP (before alignment).
  uint32_t _da_offset = 0;
  //! Offset of the first stack argument relative to SP.
  uint32_t _sa_offset_from_sp = 0;
  //! Offset of the first stack argument relative to SA (_sa_reg_id or FP).
  uint32_t _sa_offset_from_sa = 0;

  //! Local stack adjustment in prolog/epilog.
  uint32_t _stack_adjustment = 0;

  //! Registers that are dirty.
  RegMasks _dirty_regs {};
  //! Registers that must be preserved (copied from CallConv).
  RegMasks _preserved_regs {};
  //! Registers that are unavailable.
  RegMasks _unavailable_regs {};
  //! Size to save/restore per register group.
  Support::Array<uint8_t, Globals::kNumVirtGroups> _save_restore_reg_size {};
  //! Alignment of save/restore area per register group.
  Support::Array<uint8_t, Globals::kNumVirtGroups> _save_restore_alignment {};

  //! Stack size required to save registers with push/pop.
  uint16_t _push_pop_save_size = 0;
  //! Stack size required to save extra registers that cannot use push/pop.
  uint16_t _extra_reg_save_size = 0;
  //! Offset where registers saved/restored via push/pop are stored
  uint32_t _push_pop_save_offset = 0;
  //! Offset where extra registers that cannot use push/pop are stored.
  uint32_t _extra_reg_save_offset = 0;

  //! \}

  //! \name Construction & Destruction
  //! \{

  //! Creates a default constructed function frame, which has initialized all members to their default values.
  ASMJIT_INLINE_NODEBUG FuncFrame() noexcept = default;
  //! Creates a copy of `other` function frame.
  ASMJIT_INLINE_NODEBUG FuncFrame(const FuncFrame& other) noexcept = default;

  //! \}

  //! \name Initialization & Reset
  //! \{

  //! Initializes the function frame based on `func` detail.
  ASMJIT_API Error init(const FuncDetail& func) noexcept;
  //! Resets the function frame into its default constructed state.
  ASMJIT_INLINE_NODEBUG void reset() noexcept { *this = FuncFrame{}; }

  //! \}

  //! \name Overloaded Operators
  //! \{

  //! Copy assignment - function frame is copy assignable.
  ASMJIT_INLINE_NODEBUG FuncFrame& operator=(const FuncFrame& other) noexcept = default;

  //! \}

  //! \name Accessors
  //! \{

  //! Returns the target architecture of the function frame.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Arch arch() const noexcept { return _arch; }

  //! Returns function frame attributes, see `Attributes`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG FuncAttributes attributes() const noexcept { return _attributes; }

  //! Checks whether the FuncFame contains an attribute `attr`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_attribute(FuncAttributes attr) const noexcept { return Support::test(_attributes, attr); }

  //! Adds attributes `attrs` to the FuncFrame.
  ASMJIT_INLINE_NODEBUG void add_attributes(FuncAttributes attrs) noexcept { _attributes |= attrs; }

  //! Clears attributes `attrs` from the FrameFrame.
  ASMJIT_INLINE_NODEBUG void clear_attributes(FuncAttributes attrs) noexcept { _attributes &= ~attrs; }

  //! Tests whether the function has variable number of arguments.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_var_args() const noexcept { return has_attribute(FuncAttributes::kHasVarArgs); }

  //! Sets the variable arguments flag.
  ASMJIT_INLINE_NODEBUG void set_var_args() noexcept { add_attributes(FuncAttributes::kHasVarArgs); }

  //! Resets variable arguments flag.
  ASMJIT_INLINE_NODEBUG void reset_var_args() noexcept { clear_attributes(FuncAttributes::kHasVarArgs); }

  //! Tests whether the function preserves frame pointer (EBP|ESP on X86).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_preserved_fp() const noexcept { return has_attribute(FuncAttributes::kHasPreservedFP); }

  //! Enables preserved frame pointer.
  ASMJIT_INLINE_NODEBUG void set_preserved_fp() noexcept { add_attributes(FuncAttributes::kHasPreservedFP); }

  //! Disables preserved frame pointer.
  ASMJIT_INLINE_NODEBUG void reset_preserved_fp() noexcept { clear_attributes(FuncAttributes::kHasPreservedFP); }

  //! Tests whether the function calls other functions.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_func_calls() const noexcept { return has_attribute(FuncAttributes::kHasFuncCalls); }

  //! Sets `FuncAttributes::kHasFuncCalls` to true.
  ASMJIT_INLINE_NODEBUG void set_func_calls() noexcept { add_attributes(FuncAttributes::kHasFuncCalls); }

  //! Sets `FuncAttributes::kHasFuncCalls` to false.
  ASMJIT_INLINE_NODEBUG void reset_func_calls() noexcept { clear_attributes(FuncAttributes::kHasFuncCalls); }

  //! Tests whether the function uses indirect branch protection, see \ref FuncAttributes::kIndirectBranchProtection.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_indirect_branch_protection() const noexcept { return has_attribute(FuncAttributes::kIndirectBranchProtection); }

  //! Enabled indirect branch protection (sets `FuncAttributes::kIndirectBranchProtection` attribute to true).
  ASMJIT_INLINE_NODEBUG void set_indirect_branch_protection() noexcept { add_attributes(FuncAttributes::kIndirectBranchProtection); }

  //! Disables indirect branch protection (sets `FuncAttributes::kIndirectBranchProtection` attribute to false).
  ASMJIT_INLINE_NODEBUG void reset_indirect_branch_protection() noexcept { clear_attributes(FuncAttributes::kIndirectBranchProtection); }

  //! Tests whether the function has AVX enabled.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_avx_enabled() const noexcept { return has_attribute(FuncAttributes::kX86_AVXEnabled); }

  //! Enables AVX use.
  ASMJIT_INLINE_NODEBUG void set_avx_enabled() noexcept { add_attributes(FuncAttributes::kX86_AVXEnabled); }

  //! Disables AVX use.
  ASMJIT_INLINE_NODEBUG void reset_avx_enabled() noexcept { clear_attributes(FuncAttributes::kX86_AVXEnabled); }

  //! Tests whether the function has AVX-512 enabled.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_avx512_enabled() const noexcept { return has_attribute(FuncAttributes::kX86_AVX512Enabled); }

  //! Enables AVX-512 use.
  ASMJIT_INLINE_NODEBUG void set_avx512_enabled() noexcept { add_attributes(FuncAttributes::kX86_AVX512Enabled); }

  //! Disables AVX-512 use.
  ASMJIT_INLINE_NODEBUG void reset_avx512_enabled() noexcept { clear_attributes(FuncAttributes::kX86_AVX512Enabled); }

  //! Tests whether the function has MMX cleanup - 'emms' instruction in epilog.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_mmx_cleanup() const noexcept { return has_attribute(FuncAttributes::kX86_MMXCleanup); }

  //! Enables MMX cleanup.
  ASMJIT_INLINE_NODEBUG void set_mmx_cleanup() noexcept { add_attributes(FuncAttributes::kX86_MMXCleanup); }

  //! Disables MMX cleanup.
  ASMJIT_INLINE_NODEBUG void reset_mmx_cleanup() noexcept { clear_attributes(FuncAttributes::kX86_MMXCleanup); }

  //! Tests whether the function has AVX cleanup - 'vzeroupper' instruction in epilog.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_avx_cleanup() const noexcept { return has_attribute(FuncAttributes::kX86_AVXCleanup); }

  //! Enables AVX cleanup.
  ASMJIT_INLINE_NODEBUG void set_avx_cleanup() noexcept { add_attributes(FuncAttributes::kX86_AVXCleanup); }

  //! Disables AVX cleanup.
  ASMJIT_INLINE_NODEBUG void reset_avx_cleanup() noexcept { clear_attributes(FuncAttributes::kX86_AVXCleanup); }

  //! Tests whether the function has automatic AVX cleanup - 'vzeroupper' instruction in epilog when vector registers are
  //! used.
  //!
  //! \note Automatic cleanup is currently determined via dirty registers, which are provided by \ref FuncFrame.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_avx_auto_cleanup() const noexcept { return has_attribute(FuncAttributes::kX86_AVXAutoCleanup); }

  //! Enables AVX automatic cleanup.
  ASMJIT_INLINE_NODEBUG void set_avx_auto_cleanup() noexcept { add_attributes(FuncAttributes::kX86_AVXAutoCleanup); }

  //! Disables AVX automatic cleanup.
  ASMJIT_INLINE_NODEBUG void reset_avx_auto_cleanup() noexcept { clear_attributes(FuncAttributes::kX86_AVXAutoCleanup); }

  //! Tests whether the function uses call stack.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_call_stack() const noexcept { return _call_stack_size != 0; }

  //! Tests whether the function uses local stack.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_local_stack() const noexcept { return _local_stack_size != 0; }

  //! Tests whether vector registers can be saved and restored by using aligned reads and writes.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_aligned_vec_save_restore() const noexcept { return has_attribute(FuncAttributes::kAlignedVecSR); }

  //! Tests whether the function has to align stack dynamically.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_dynamic_alignment() const noexcept { return _final_stack_alignment >= _min_dynamic_alignment; }

  //! Tests whether the calling convention specifies 'Red Zone'.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_red_zone() const noexcept { return _red_zone_size != 0; }

  //! Returns the size of 'Red Zone'.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t red_zone_size() const noexcept { return _red_zone_size; }

  //! Tests whether the calling convention specifies 'Spill Zone'.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_spill_zone() const noexcept { return _spill_zone_size != 0; }

  //! Returns the size of 'Spill Zone'.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t spill_zone_size() const noexcept { return _spill_zone_size; }

  //! Resets the size of red zone, which would disable it entirely.
  //!
  //! \note Red zone is currently only used by an AMD64 SystemV calling convention, which expects 128
  //! bytes of stack to be accessible below stack pointer. These bytes are then accessible within the
  //! function and Compiler can use this space as a spill area. However, sometimes it's better to
  //! disallow the use of red zone in case that a user wants to use this stack for a custom purpose.
  ASMJIT_INLINE_NODEBUG void reset_red_zone() noexcept { _red_zone_size = 0; }

  //! Returns natural stack alignment (guaranteed stack alignment upon entry).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t natural_stack_alignment() const noexcept { return _natural_stack_alignment; }

  //! Returns natural stack alignment (guaranteed stack alignment upon entry).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t min_dynamic_alignment() const noexcept { return _min_dynamic_alignment; }

  //! Tests whether the callee must adjust SP before returning (X86-STDCALL only)
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_callee_stack_cleanup() const noexcept { return _callee_stack_cleanup != 0; }

  //! Returns home many bytes of the stack the callee must adjust before returning (X86-STDCALL only)
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t callee_stack_cleanup() const noexcept { return _callee_stack_cleanup; }

  //! Returns call stack alignment.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t call_stack_alignment() const noexcept { return _call_stack_alignment; }

  //! Returns local stack alignment.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t local_stack_alignment() const noexcept { return _local_stack_alignment; }

  //! Returns final stack alignment (the maximum value of call, local, and natural stack alignments).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t final_stack_alignment() const noexcept { return _final_stack_alignment; }

  //! Sets call stack alignment.
  //!
  //! \note This also updates the final stack alignment.
  ASMJIT_INLINE void set_call_stack_alignment(uint32_t alignment) noexcept {
    _call_stack_alignment = uint8_t(alignment);
    _final_stack_alignment = Support::max(_natural_stack_alignment, _call_stack_alignment, _local_stack_alignment);
  }

  //! Sets local stack alignment.
  //!
  //! \note This also updates the final stack alignment.
  ASMJIT_INLINE void set_local_stack_alignment(uint32_t value) noexcept {
    _local_stack_alignment = uint8_t(value);
    _final_stack_alignment = Support::max(_natural_stack_alignment, _call_stack_alignment, _local_stack_alignment);
  }

  //! Combines call stack alignment with `alignment`, updating it to the greater value.
  //!
  //! \note This also updates the final stack alignment.
  ASMJIT_INLINE void update_call_stack_alignment(uint32_t alignment) noexcept {
    _call_stack_alignment = uint8_t(Support::max<uint32_t>(_call_stack_alignment, alignment));
    _final_stack_alignment = Support::max(_final_stack_alignment, _call_stack_alignment);
  }

  //! Combines local stack alignment with `alignment`, updating it to the greater value.
  //!
  //! \note This also updates the final stack alignment.
  ASMJIT_INLINE void update_local_stack_alignment(uint32_t alignment) noexcept {
    _local_stack_alignment = uint8_t(Support::max<uint32_t>(_local_stack_alignment, alignment));
    _final_stack_alignment = Support::max(_final_stack_alignment, _local_stack_alignment);
  }

  //! Returns call stack size.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t call_stack_size() const noexcept { return _call_stack_size; }

  //! Returns local stack size.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t local_stack_size() const noexcept { return _local_stack_size; }

  //! Sets call stack size.
  ASMJIT_INLINE_NODEBUG void set_call_stack_size(uint32_t size) noexcept { _call_stack_size = size; }

  //! Sets local stack size.
  ASMJIT_INLINE_NODEBUG void set_local_stack_size(uint32_t size) noexcept { _local_stack_size = size; }

  //! Combines call stack size with `size`, updating it to the greater value.
  ASMJIT_INLINE_NODEBUG void update_call_stack_size(uint32_t size) noexcept { _call_stack_size = Support::max(_call_stack_size, size); }

  //! Combines local stack size with `size`, updating it to the greater value.
  ASMJIT_INLINE_NODEBUG void update_local_stack_size(uint32_t size) noexcept { _local_stack_size = Support::max(_local_stack_size, size); }

  //! Returns final stack size (only valid after the FuncFrame is finalized).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t final_stack_size() const noexcept { return _final_stack_size; }

  //! Returns an offset to access the local stack (non-zero only if call stack is used).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t local_stack_offset() const noexcept { return _local_stack_offset; }

  //! Tests whether the function prolog/epilog requires a memory slot for storing unaligned SP.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_da_offset() const noexcept { return _da_offset != kTagInvalidOffset; }

  //! Returns a memory offset used to store DA (dynamic alignment) slot (relative to SP).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t da_offset() const noexcept { return _da_offset; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t sa_offset(uint32_t reg_id) const noexcept {
    return reg_id == _sp_reg_id ? sa_offset_from_sp() : sa_offset_from_sa();
  }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t sa_offset_from_sp() const noexcept { return _sa_offset_from_sp; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t sa_offset_from_sa() const noexcept { return _sa_offset_from_sa; }

  //! Returns mask of registers of the given register `group` that are modified by the function. The engine would
  //! then calculate which registers must be saved & restored by the function by using the data provided by the
  //! calling convention.
  [[nodiscard]]
  inline RegMask dirty_regs(RegGroup group) const noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    return _dirty_regs[group];
  }

  //! Sets which registers (as a mask) are modified by the function.
  //!
  //! \remarks Please note that this will completely overwrite the existing register mask, use `add_dirty_regs()`
  //! to modify the existing register mask.
  inline void set_dirty_regs(RegGroup group, RegMask regs) noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    _dirty_regs[group] = regs;
  }

  //! Adds which registers (as a mask) are modified by the function.
  inline void add_dirty_regs(RegGroup group, RegMask regs) noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    _dirty_regs[group] |= regs;
  }

  //! \overload
  inline void add_dirty_regs(const Reg& reg) noexcept {
    ASMJIT_ASSERT(reg.id() < Globals::kMaxPhysRegs);
    add_dirty_regs(reg.reg_group(), Support::bit_mask<RegMask>(reg.id()));
  }

  //! \overload
  template<typename... Args>
  inline void add_dirty_regs(const Reg& reg, Args&&... args) noexcept {
    add_dirty_regs(reg);
    add_dirty_regs(std::forward<Args>(args)...);
  }

  //! A helper function to set all registers from all register groups dirty.
  //!
  //! \note This should not be used in general as it's the most pessimistic case. However, it can be used for testing
  //! or in cases in which all registers are considered clobbered.
  ASMJIT_INLINE_NODEBUG void set_all_dirty() noexcept {
    for (size_t i = 0; i < ASMJIT_ARRAY_SIZE(_dirty_regs); i++) {
      _dirty_regs[i] = 0xFFFFFFFFu;
    }
  }

  //! A helper function to set all registers from the given register `group` dirty.
  ASMJIT_INLINE void set_all_dirty(RegGroup group) noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    _dirty_regs[group] = 0xFFFFFFFFu;
  }

  //! Returns a calculated mask of registers of the given `group` that will be saved and restored in the function's
  //! prolog and epilog, respectively. The register mask is calculated from both `dirty_regs` (provided by user) and
  //! `preserved_mask` (provided by the calling convention).
  [[nodiscard]]
  ASMJIT_INLINE RegMask saved_regs(RegGroup group) const noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    return _dirty_regs[group] & _preserved_regs[group];
  }

  //! Returns all dirty registers as a Support::Array<> type.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const RegMasks& dirty_regs() const noexcept { return _dirty_regs; }

  //! Returns all preserved registers as a Support::Array<> type.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const RegMasks& preserved_regs() const noexcept { return _preserved_regs; }

  //! Returns the mask of preserved registers of the given register `group`.
  //!
  //! Preserved registers are those that must survive the function call unmodified. The function can only modify
  //! preserved registers it they are saved and restored in function's prolog and epilog, respectively.
  [[nodiscard]]
  ASMJIT_INLINE RegMask preserved_regs(RegGroup group) const noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    return _preserved_regs[group];
  }

  //! Sets which registers (as a mask) are unavailable for allocation.
  //!
  //! \note This completely overwrites the current unavailable mask.
  ASMJIT_INLINE void set_unavailable_regs(RegGroup group, RegMask regs) noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    _unavailable_regs[group] = regs;
  }

  //! Adds registers (as a mask) to the unavailable set.
  ASMJIT_INLINE void add_unavailable_regs(RegGroup group, RegMask regs) noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    _unavailable_regs[group] |= regs;
  }

  //! Adds a single register to the unavailable set.
  ASMJIT_INLINE void add_unavailable_regs(const Reg& reg) noexcept {
    ASMJIT_ASSERT(reg.id() < Globals::kMaxPhysRegs);
    add_unavailable_regs(reg.reg_group(), Support::bit_mask<RegMask>(reg.id()));
  }

  //! Adds multiple registers to the unavailable set.
  template<typename... Args>
  ASMJIT_INLINE void add_unavailable_regs(const Reg& reg, Args&&... args) noexcept {
    add_unavailable_regs(reg);
    add_unavailable_regs(std::forward<Args>(args)...);
  }

  //! Clears all unavailable registers across all register groups (i.e., makes them all available again).
  ASMJIT_INLINE_NODEBUG void clear_unavailable_regs() noexcept {
    for (size_t i = 0; i < ASMJIT_ARRAY_SIZE(_unavailable_regs); i++)
      _unavailable_regs[i] = 0;
  }

  //! Clears all unavailable registers in a specific register group.
  ASMJIT_INLINE void clear_unavailable_regs(RegGroup group) noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    _unavailable_regs[group] = 0;
  }

  //! Returns the set of unavailable registers for the given group.
  [[nodiscard]]
  ASMJIT_INLINE RegMask unavailable_regs(RegGroup group) const noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    return _unavailable_regs[group];
  }

  //! Returns all unavailable registers as a Support::Array<>.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const RegMasks& unavailable_regs() const noexcept {
    return _unavailable_regs;
  }

  //! Returns the size of a save-restore are for the required register `group`.
  [[nodiscard]]
  ASMJIT_INLINE uint32_t save_restore_reg_size(RegGroup group) const noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    return _save_restore_reg_size[group];
  }

  //! Returns the alignment that must be guaranteed to save/restore the required register `group`.
  [[nodiscard]]
  ASMJIT_INLINE uint32_t save_restore_alignment(RegGroup group) const noexcept {
    ASMJIT_ASSERT(group <= RegGroup::kMaxVirt);
    return _save_restore_alignment[group];
  }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_sa_reg_id() const noexcept { return _sa_reg_id != Reg::kIdBad; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t sa_reg_id() const noexcept { return _sa_reg_id; }

  ASMJIT_INLINE_NODEBUG void set_sa_reg_id(uint32_t reg_id) { _sa_reg_id = uint8_t(reg_id); }

  ASMJIT_INLINE_NODEBUG void reset_sa_reg_id() { set_sa_reg_id(Reg::kIdBad); }

  //! Returns stack size required to save/restore registers via push/pop.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t push_pop_save_size() const noexcept { return _push_pop_save_size; }

  //! Returns an offset to the stack where registers are saved via push/pop.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t push_pop_save_offset() const noexcept { return _push_pop_save_offset; }

  //! Returns stack size required to save/restore extra registers that don't use push/pop/
  //!
  //! \note On X86 this covers all registers except GP registers, on other architectures it can be always
  //! zero (for example AArch64 saves all registers via push/pop like instructions, so this would be zero).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t extra_reg_save_size() const noexcept { return _extra_reg_save_size; }

  //! Returns an offset to the stack where extra registers are saved.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t extra_reg_save_offset() const noexcept { return _extra_reg_save_offset; }

  //! Tests whether the functions contains stack adjustment.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_stack_adjustment() const noexcept { return _stack_adjustment != 0; }

  //! Returns function's stack adjustment used in function's prolog and epilog.
  //!
  //! If the returned value is zero it means that the stack is not adjusted. This can mean both that the stack
  //! is not used and/or the stack is only adjusted by instructions that pust/pop registers into/from stack.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t stack_adjustment() const noexcept { return _stack_adjustment; }

  //! \}

  //! \name Finalization
  //! \{

  ASMJIT_API Error finalize() noexcept;

  //! \}
};

//! A helper class that can be used to assign a physical register for each function argument. The assignment
//! is passed to \ref BaseEmitter::emit_args_assignment() function.
class FuncArgsAssignment {
public:
  //! \name Members
  //! \{

  //! Function detail.
  const FuncDetail* _func_detail {};
  //! Register that can be used to access arguments passed by stack.
  uint8_t _sa_reg_id = uint8_t(Reg::kIdBad);
  //! Reserved for future use.
  uint8_t _reserved[3] {};
  //! Mapping of each function argument.
  FuncValuePack _arg_packs[Globals::kMaxFuncArgs] {};

  //! \}

  //! \name Construction & Destruction
  //! \{

  //! Creates either a default initialized `FuncArgsAssignment` or to assignment that links to `fd`, if non-null.
  ASMJIT_INLINE_NODEBUG explicit FuncArgsAssignment(const FuncDetail* fd = nullptr) noexcept
    : _func_detail(fd),
      _sa_reg_id(uint8_t(Reg::kIdBad)) {}

  //! Copy constructor.
  ASMJIT_INLINE_NODEBUG FuncArgsAssignment(const FuncArgsAssignment& other) noexcept = default;

  //! Resets this `FuncArgsAssignment` to either default constructed state or to assignment that links to `fd`,
  //! if non-null.
  ASMJIT_INLINE void reset(const FuncDetail* fd = nullptr) noexcept {
    _func_detail = fd;
    _sa_reg_id = uint8_t(Reg::kIdBad);
    memset(_reserved, 0, sizeof(_reserved));
    memset(_arg_packs, 0, sizeof(_arg_packs));
  }

  //! \}

  //! \name Overloaded Operators
  //! \{

  //! Copy assignment.
  ASMJIT_INLINE_NODEBUG FuncArgsAssignment& operator=(const FuncArgsAssignment& other) noexcept = default;

  //! \}

  //! \name Accessors
  //! \{

  //! Returns the associated \ref FuncDetail of this `FuncArgsAssignment`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const FuncDetail* func_detail() const noexcept { return _func_detail; }

  //! Associates \ref FuncDetail with this `FuncArgsAssignment`.
  ASMJIT_INLINE_NODEBUG void set_func_detail(const FuncDetail* fd) noexcept { _func_detail = fd; }

  //! Returns whether a register to access stack arguments is available.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_sa_reg_id() const noexcept { return _sa_reg_id != Reg::kIdBad; }

  //! Returns a physical ID of a register that can access stack arguments.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t sa_reg_id() const noexcept { return _sa_reg_id; }

  //! Sets a physical ID of a register that can access stack arguments.
  ASMJIT_INLINE_NODEBUG void set_sa_reg_id(uint32_t reg_id) { _sa_reg_id = uint8_t(reg_id); }

  //! Resets a physical ID of a register that can access stack arguments.
  ASMJIT_INLINE_NODEBUG void reset_sa_reg_id() { _sa_reg_id = uint8_t(Reg::kIdBad); }

  //! Returns assigned argument at `arg_index` and `value_index`.
  //!
  //! \note `arg_index` refers to he function argument and `value_index` refers to a value pack (in case multiple
  //! values are passed as a single argument).
  [[nodiscard]]
  ASMJIT_INLINE FuncValue& arg(size_t arg_index, size_t value_index) noexcept {
    ASMJIT_ASSERT(arg_index < ASMJIT_ARRAY_SIZE(_arg_packs));
    return _arg_packs[arg_index][value_index];
  }

  //! \overload
  [[nodiscard]]
  ASMJIT_INLINE const FuncValue& arg(size_t arg_index, size_t value_index) const noexcept {
    ASMJIT_ASSERT(arg_index < ASMJIT_ARRAY_SIZE(_arg_packs));
    return _arg_packs[arg_index][value_index];
  }

  //! Tests whether argument at `arg_index` and `value_index` has been assigned.
  [[nodiscard]]
  ASMJIT_INLINE bool is_assigned(size_t arg_index, size_t value_index) const noexcept {
    ASMJIT_ASSERT(arg_index < ASMJIT_ARRAY_SIZE(_arg_packs));
    return _arg_packs[arg_index][value_index].is_assigned();
  }

  //! Assigns register at `arg_index` and value index of 0 to `reg` and an optional `type_id`.
  ASMJIT_INLINE void assign_reg(size_t arg_index, const Reg& reg, TypeId type_id = TypeId::kVoid) noexcept {
    ASMJIT_ASSERT(arg_index < ASMJIT_ARRAY_SIZE(_arg_packs));
    ASMJIT_ASSERT(reg.is_phys_reg());
    _arg_packs[arg_index][0].init_reg(reg.reg_type(), reg.id(), type_id);
  }

  //! Assigns register at `arg_index` and value index of 0 to `reg_type`, `reg_id`, and an optional `type_id`.
  ASMJIT_INLINE void assign_reg(size_t arg_index, RegType reg_type, uint32_t reg_id, TypeId type_id = TypeId::kVoid) noexcept {
    ASMJIT_ASSERT(arg_index < ASMJIT_ARRAY_SIZE(_arg_packs));
    _arg_packs[arg_index][0].init_reg(reg_type, reg_id, type_id);
  }

  //! Assigns stack at `arg_index` and value index of 0 to `offset` and an optional `type_id`.
  ASMJIT_INLINE void assign_stack(size_t arg_index, int32_t offset, TypeId type_id = TypeId::kVoid) noexcept {
    ASMJIT_ASSERT(arg_index < ASMJIT_ARRAY_SIZE(_arg_packs));
    _arg_packs[arg_index][0].init_stack(offset, type_id);
  }

  //! Assigns register at `arg_index` and `value_index` to `reg` and an optional `type_id`.
  ASMJIT_INLINE void assign_reg_in_pack(size_t arg_index, size_t value_index, const Reg& reg, TypeId type_id = TypeId::kVoid) noexcept {
    ASMJIT_ASSERT(arg_index < ASMJIT_ARRAY_SIZE(_arg_packs));
    ASMJIT_ASSERT(reg.is_phys_reg());
    _arg_packs[arg_index][value_index].init_reg(reg.reg_type(), reg.id(), type_id);
  }

  //! Assigns register at `arg_index` and `value_index` to `reg_type`, `reg_id`, and an optional `type_id`.
  ASMJIT_INLINE void assign_reg_in_pack(size_t arg_index, size_t value_index, RegType reg_type, uint32_t reg_id, TypeId type_id = TypeId::kVoid) noexcept {
    ASMJIT_ASSERT(arg_index < ASMJIT_ARRAY_SIZE(_arg_packs));
    _arg_packs[arg_index][value_index].init_reg(reg_type, reg_id, type_id);
  }

  //! Assigns stack at `arg_index` and `value_index` to `offset` and an optional `type_id`.
  ASMJIT_INLINE void assign_stack_in_pack(size_t arg_index, size_t value_index, int32_t offset, TypeId type_id = TypeId::kVoid) noexcept {
    ASMJIT_ASSERT(arg_index < ASMJIT_ARRAY_SIZE(_arg_packs));
    _arg_packs[arg_index][value_index].init_stack(offset, type_id);
  }

  //! \cond INTERNAL
  // NOTE: All `assign_all()` methods are shortcuts to assign all arguments at once, however, since registers are
  // passed all at once these initializers don't provide any way to pass TypeId and/or to keep any argument between
  // the arguments passed unassigned.
  ASMJIT_INLINE void _assign_all_internal(size_t arg_index, const Reg& reg) noexcept {
    assign_reg(arg_index, reg);
  }

  template<typename... Args>
  ASMJIT_INLINE void _assign_all_internal(size_t arg_index, const Reg& reg, Args&&... args) noexcept {
    assign_reg(arg_index, reg);
    _assign_all_internal(arg_index + 1, std::forward<Args>(args)...);
  }
  //! \endcond

  //! Assigns all argument at once.
  //!
  //! \note This function can be only used if the arguments don't contain value packs (multiple values per argument).
  template<typename... Args>
  ASMJIT_INLINE void assign_all(Args&&... args) noexcept {
    _assign_all_internal(0, std::forward<Args>(args)...);
  }

  //! \}

  //! \name Utilities
  //! \{

  //! Update `FuncFrame` based on function's arguments assignment.
  //!
  //! \note This function must be called in order to use `BaseEmitter::emit_args_assignment()`, otherwise the \ref FuncFrame
  //! would not contain the information necessary to assign all arguments into the registers and/or stack specified.
  ASMJIT_API Error update_func_frame(FuncFrame& frame) const noexcept;

  //! \}
};

//! \}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_CORE_FUNC_H_INCLUDED
