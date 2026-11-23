// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_ARM_A64COMPILER_H_INCLUDED
#define ASMJIT_ARM_A64COMPILER_H_INCLUDED

#include <asmjit/core/api-config.h>
#ifndef ASMJIT_NO_COMPILER

#include <asmjit/core/compiler.h>
#include <asmjit/core/type.h>
#include <asmjit/arm/a64emitter.h>

ASMJIT_BEGIN_SUB_NAMESPACE(a64)

//! \addtogroup asmjit_a64
//! \{

//! AArch64 compiler implementation.
class ASMJIT_VIRTAPI Compiler
  : public BaseCompiler,
    public EmitterExplicitT<Compiler> {
public:
  ASMJIT_NONCOPYABLE(Compiler)
  using Base = BaseCompiler;

  //! \name Construction & Destruction
  //! \{

  ASMJIT_API explicit Compiler(CodeHolder* code = nullptr) noexcept;
  ASMJIT_API ~Compiler() noexcept override;

  //! \}

  //! \name Virtual Registers
  //! \{

  //! Creates a new general-purpose register with `type_id` type and optional name passed via `args`.
  //!
  //! \note Using \ref TypeId is too generic. In general it's recommended to use \ref new_gp32(),
  //! \ref new_gp64(), and \ref new_gpz() or \ref new_gp_ptr().
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp new_gp(TypeId type_id, Args&&... args) { return new_reg<Gp>(type_id, std::forward<Args>(args)...); }

  //! Creates a new vector register with `type_id` type and optional name passed via `args`.
  //!
  //! \note Using \ref TypeId is too generic. In general it's recommended to use \ref new_vec128(),
  //! \ref new_vec_s(), \ref new_vec_d(), \ref new_vec_q(), ...
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_vec(TypeId type_id, Args&&... args) { return new_reg<Vec>(type_id, std::forward<Args>(args)...); }

  //! Creates a new 32-bit general purpose register mapped to low 32 bits of a full register (on 64-bit targets).
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp new_gp32(Args&&... args) { return new_reg<Gp>(TypeId::kUInt32, std::forward<Args>(args)...); }

  //! Creates a new 64-bit general purpose register.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp new_gp64(Args&&... args) { return new_reg<Gp>(TypeId::kUInt64, std::forward<Args>(args)...); }

  //! Creates a new 32-bit general purpose register.
  //!
  //! \note This is a convenience function alias of \ref new_gp32().
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp new_gpw(Args&&... args) { return new_reg<Gp>(TypeId::kUIntPtr, std::forward<Args>(args)...); }

  //! Creates a new 64-bit general purpose register.
  //!
  //! \note This is a convenience function alias of \ref new_gp64().
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp new_gpx(Args&&... args) { return new_reg<Gp>(TypeId::kUIntPtr, std::forward<Args>(args)...); }

  //! Creates a new 32-bit or 64-bit general purpose register depending on the target register width.
  //!
  //! \note This is a convenience function, on aarch64 target it always creates a 64-bit general-purpose register.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp new_gpz(Args&&... args) { return new_reg<Gp>(TypeId::kUIntPtr, std::forward<Args>(args)...); }

  //! Creates a new 32-bit or 64-bit general purpose register depending on the target register width.
  //!
  //! \note This is a convenience function, on aarch64 target it always creates a 64-bit general-purpose register.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp new_gp_ptr(Args&&... args) { return new_reg<Gp>(TypeId::kUIntPtr, std::forward<Args>(args)...); }

  //! Creates a new 128-bit vector register.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_vec128(Args&&... args) { return new_reg<Vec>(TypeId::kInt32x4, std::forward<Args>(args)...); }

  //! Creates a new 128-bit vector register that will be used for scalar 32-bit floating point operation.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_vec128_f32x1(Args&&... args) { return new_reg<Vec>(TypeId::kFloat32x1, std::forward<Args>(args)...); }

  //! Creates a new 128-bit vector register that will be used for scalar 64-bit floating point operation.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_vec128_f64x1(Args&&... args) { return new_reg<Vec>(TypeId::kFloat64x1, std::forward<Args>(args)...); }

  //! Creates a new 128-bit vector register that will be used for packed 32-bit floating point operation.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_vec128_f32x4(Args&&... args) { return new_reg<Vec>(TypeId::kFloat32x4, std::forward<Args>(args)...); }

  //! Creates a new 128-bit vector register that will be used for packed 64-bit floating point operation.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_vec128_f64x2(Args&&... args) { return new_reg<Vec>(TypeId::kFloat64x2, std::forward<Args>(args)...); }

  //! Creates a new 32-bit vector register (S).
  //!
  //! \note This may look like an alias of \ref new_vec128_f32x1(), but it's not. This really creates a 32-bit
  //! register, which has a type \ref RegType::kVec32, whereas \ref new_vec128_f32x1() creates a register,
  //! which has a type \ref RegType::kVec64
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_vec_s(Args&&... args) { return new_reg<Vec>(TypeId::kFloat32, std::forward<Args>(args)...); }

  //! Alias of \ref new_vec128_f64x1() that matches aarch64 architecture terminology.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_vec_d(Args&&... args) { return new_reg<Vec>(TypeId::kFloat64, std::forward<Args>(args)...); }

  //! Alias of \ref new_vec128() that matches aarch64 architecture terminology.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_vec_q(Args&&... args) { return new_reg<Vec>(TypeId::kInt32x4, std::forward<Args>(args)...); }

  //! \}

  //! \name Stack
  //! \{

  //! Creates a new stack and returns a \ref Mem operand that can be used to address it.
  ASMJIT_INLINE_NODEBUG Mem new_stack(uint32_t size, uint32_t alignment, const char* name = nullptr) {
    Mem m(Globals::NoInit);
    _new_stack(Out<BaseMem>(m), size, alignment, name);
    return m;
  }

  //! \}

  //! \name Constants
  //! \{

  //! Put data to a constant-pool and get a memory reference to it.
  ASMJIT_INLINE_NODEBUG Mem new_const(ConstPoolScope scope, const void* data, size_t size) {
    Mem m(Globals::NoInit);
    _new_const(Out<BaseMem>(m), scope, data, size);
    return m;
  }

  //! Put a BYTE `val` to a constant-pool (8 bits).
  ASMJIT_INLINE_NODEBUG Mem new_byte_const(ConstPoolScope scope, uint8_t val) noexcept { return new_const(scope, &val, 1); }
  //! Put a HWORD `val` to a constant-pool (16 bits).
  ASMJIT_INLINE_NODEBUG Mem new_half_const(ConstPoolScope scope, uint16_t val) noexcept { return new_const(scope, &val, 2); }
  //! Put a WORD `val` to a constant-pool (32 bits).
  ASMJIT_INLINE_NODEBUG Mem new_word_const(ConstPoolScope scope, uint32_t val) noexcept { return new_const(scope, &val, 4); }
  //! Put a DWORD `val` to a constant-pool (64 bits).
  ASMJIT_INLINE_NODEBUG Mem new_dword_const(ConstPoolScope scope, uint64_t val) noexcept { return new_const(scope, &val, 8); }

  //! Put a WORD `val` to a constant-pool.
  ASMJIT_INLINE_NODEBUG Mem new_int16_const(ConstPoolScope scope, int16_t val) noexcept { return new_const(scope, &val, 2); }
  //! Put a WORD `val` to a constant-pool.
  ASMJIT_INLINE_NODEBUG Mem new_uint16_const(ConstPoolScope scope, uint16_t val) noexcept { return new_const(scope, &val, 2); }
  //! Put a DWORD `val` to a constant-pool.
  ASMJIT_INLINE_NODEBUG Mem new_int32_const(ConstPoolScope scope, int32_t val) noexcept { return new_const(scope, &val, 4); }
  //! Put a DWORD `val` to a constant-pool.
  ASMJIT_INLINE_NODEBUG Mem new_uint32_const(ConstPoolScope scope, uint32_t val) noexcept { return new_const(scope, &val, 4); }
  //! Put a QWORD `val` to a constant-pool.
  ASMJIT_INLINE_NODEBUG Mem new_int64_const(ConstPoolScope scope, int64_t val) noexcept { return new_const(scope, &val, 8); }
  //! Put a QWORD `val` to a constant-pool.
  ASMJIT_INLINE_NODEBUG Mem new_uint64_const(ConstPoolScope scope, uint64_t val) noexcept { return new_const(scope, &val, 8); }

  //! Put a SP-FP `val` to a constant-pool.
  ASMJIT_INLINE_NODEBUG Mem new_float_const(ConstPoolScope scope, float val) noexcept { return new_const(scope, &val, 4); }
  //! Put a DP-FP `val` to a constant-pool.
  ASMJIT_INLINE_NODEBUG Mem new_double_const(ConstPoolScope scope, double val) noexcept { return new_const(scope, &val, 8); }

  //! \}

  //! \name Instruction Options
  //! \{

  //! Force the compiler to not follow the conditional or unconditional jump.
  ASMJIT_INLINE_NODEBUG Compiler& unfollow() noexcept { _inst_options |= InstOptions::kUnfollow; return *this; }

  //! \}

  //! \name Compiler specific
  //! \{

  //! Special pseudo-instruction that can be used to load a memory address into `o0` GP register.
  //!
  //! \note At the moment this instruction is only useful to load a stack allocated address into a GP register
  //! for further use. It makes very little sense to use it for anything else. The semantics of this instruction
  //! is the same as X86 `LEA` (load effective address) instruction.
  ASMJIT_INLINE_NODEBUG Error load_address_of(const Gp& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdAdr, o0, o1); }

  //! \}

  //! \name Function Call & Ret Intrinsics
  //! \{

  //! Invoke a function call without `target` type enforcement.
  ASMJIT_INLINE_NODEBUG Error invoke_(Out<InvokeNode*> out, const Operand_& target, const FuncSignature& signature) {
    return add_invoke_node(out, Inst::kIdBlr, target, signature);
  }

  //! Invoke a function call of the given `target` and `signature` and store the added node to `out`.
  //!
  //! Creates a new \ref InvokeNode, initializes all the necessary members to match the given function `signature`,
  //! adds the node to the compiler, and stores its pointer to `out`. The operation is atomic, if anything fails
  //! nullptr is stored in `out` and error code is returned.
  ASMJIT_INLINE_NODEBUG Error invoke(Out<InvokeNode*> out, const Gp& target, const FuncSignature& signature) { return invoke_(out, target, signature); }
  //! \overload
  ASMJIT_INLINE_NODEBUG Error invoke(Out<InvokeNode*> out, const Mem& target, const FuncSignature& signature) { return invoke_(out, target, signature); }
  //! \overload
  ASMJIT_INLINE_NODEBUG Error invoke(Out<InvokeNode*> out, const Label& target, const FuncSignature& signature) { return invoke_(out, target, signature); }
  //! \overload
  ASMJIT_INLINE_NODEBUG Error invoke(Out<InvokeNode*> out, const Imm& target, const FuncSignature& signature) { return invoke_(out, target, signature); }
  //! \overload
  ASMJIT_INLINE_NODEBUG Error invoke(Out<InvokeNode*> out, uint64_t target, const FuncSignature& signature) { return invoke_(out, Imm(int64_t(target)), signature); }

  //! Return from function.
  //!
  //! \note This doesn't end the function - it just emits a return.
  ASMJIT_INLINE_NODEBUG Error ret() { return add_ret(Operand(), Operand()); }

  //! Return from function - one value.
  //!
  //! \note This doesn't end the function - it just emits a return.
  ASMJIT_INLINE_NODEBUG Error ret(const Reg& o0) { return add_ret(o0, Operand()); }

  //! Return from function - two values / register pair.
  //!
  //! \note This doesn't end the function - it just emits a return.
  ASMJIT_INLINE_NODEBUG Error ret(const Reg& o0, const Reg& o1) { return add_ret(o0, o1); }

  //! \}

  //! \name Jump Tables Support
  //! \{

  using EmitterExplicitT<Compiler>::br;

  //! Adds a jump to the given `target` with the provided jump `annotation`.
  ASMJIT_INLINE_NODEBUG Error br(const Reg& target, JumpAnnotation* annotation) { return emit_annotated_jump(Inst::kIdBr, target, annotation); }

  //! \}

  //! \name Events
  //! \{

  ASMJIT_API Error on_attach(CodeHolder& code) noexcept override;
  ASMJIT_API Error on_detach(CodeHolder& code) noexcept override;
  ASMJIT_API Error on_reinit(CodeHolder& code) noexcept override;

  //! \}

  //! \name Finalize
  //! \{

  ASMJIT_API Error finalize() override;

  //! \}
};

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_COMPILER
#endif // ASMJIT_ARM_A64COMPILER_H_INCLUDED
