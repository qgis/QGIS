// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_X86_X86EMITTER_H_INCLUDED
#define ASMJIT_X86_X86EMITTER_H_INCLUDED

#include <asmjit/core/emitter.h>
#include <asmjit/support/support.h>
#include <asmjit/x86/x86globals.h>
#include <asmjit/x86/x86operand.h>

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

#define ASMJIT_INST_0x(NAME, ID) \
  inline Error NAME() { return _emitter()->_emitI(Inst::kId##ID); }

#define ASMJIT_INST_1x(NAME, ID, T0) \
  inline Error NAME(const T0& o0) { return _emitter()->_emitI(Inst::kId##ID, o0); }

#define ASMJIT_INST_1c(NAME, ID, CONV, T0) \
  inline Error NAME(CondCode cc, const T0& o0) { return _emitter()->_emitI(CONV(cc), o0); } \
  inline Error NAME##a(const T0& o0) { return _emitter()->_emitI(Inst::kId##ID##a, o0); } \
  inline Error NAME##ae(const T0& o0) { return _emitter()->_emitI(Inst::kId##ID##ae, o0); } \
  inline Error NAME##b(const T0& o0) { return _emitter()->_emitI(Inst::kId##ID##b, o0); } \
  inline Error NAME##be(const T0& o0) { return _emitter()->_emitI(Inst::kId##ID##be, o0); } \
  inline Error NAME##c(const T0& o0) { return _emitter()->_emitI(Inst::kId##ID##c, o0); } \
  inline Error NAME##e(const T0& o0) { return _emitter()->_emitI(Inst::kId##ID##e, o0); } \
  inline Error NAME##g(const T0& o0) { return _emitter()->_emitI(Inst::kId##ID##g, o0); } \
  inline Error NAME##ge(const T0& o0) { return _emitter()->_emitI(Inst::kId##ID##ge, o0); } \
  inline Error NAME##l(const T0& o0) { return _emitter()->_emitI(Inst::kId##ID##l, o0); } \
  inline Error NAME##le(const T0& o0) { return _emitter()->_emitI(Inst::kId##ID##le, o0); } \
  inline Error NAME##na(const T0& o0) { return _emitter()->_emitI(Inst::kId##ID##na, o0); } \
  inline Error NAME##nae(const T0& o0) { return _emitter()->_emitI(Inst::kId##ID##nae, o0); } \
  inline Error NAME##nb(const T0& o0) { return _emitter()->_emitI(Inst::kId##ID##nb, o0); } \
  inline Error NAME##nbe(const T0& o0) { return _emitter()->_emitI(Inst::kId##ID##nbe, o0); } \
  inline Error NAME##nc(const T0& o0) { return _emitter()->_emitI(Inst::kId##ID##nc, o0); } \
  inline Error NAME##ne(const T0& o0) { return _emitter()->_emitI(Inst::kId##ID##ne, o0); } \
  inline Error NAME##ng(const T0& o0) { return _emitter()->_emitI(Inst::kId##ID##ng, o0); } \
  inline Error NAME##nge(const T0& o0) { return _emitter()->_emitI(Inst::kId##ID##nge, o0); } \
  inline Error NAME##nl(const T0& o0) { return _emitter()->_emitI(Inst::kId##ID##nl, o0); } \
  inline Error NAME##nle(const T0& o0) { return _emitter()->_emitI(Inst::kId##ID##nle, o0); } \
  inline Error NAME##no(const T0& o0) { return _emitter()->_emitI(Inst::kId##ID##no, o0); } \
  inline Error NAME##np(const T0& o0) { return _emitter()->_emitI(Inst::kId##ID##np, o0); } \
  inline Error NAME##ns(const T0& o0) { return _emitter()->_emitI(Inst::kId##ID##ns, o0); } \
  inline Error NAME##nz(const T0& o0) { return _emitter()->_emitI(Inst::kId##ID##nz, o0); } \
  inline Error NAME##o(const T0& o0) { return _emitter()->_emitI(Inst::kId##ID##o, o0); } \
  inline Error NAME##p(const T0& o0) { return _emitter()->_emitI(Inst::kId##ID##p, o0); } \
  inline Error NAME##pe(const T0& o0) { return _emitter()->_emitI(Inst::kId##ID##pe, o0); } \
  inline Error NAME##po(const T0& o0) { return _emitter()->_emitI(Inst::kId##ID##po, o0); } \
  inline Error NAME##s(const T0& o0) { return _emitter()->_emitI(Inst::kId##ID##s, o0); } \
  inline Error NAME##z(const T0& o0) { return _emitter()->_emitI(Inst::kId##ID##z, o0); }

#define ASMJIT_INST_2x(NAME, ID, T0, T1) \
  inline Error NAME(const T0& o0, const T1& o1) { return _emitter()->_emitI(Inst::kId##ID, o0, o1); }

#define ASMJIT_INST_2c(NAME, ID, CONV, T0, T1) \
  inline Error NAME(CondCode cc, const T0& o0, const T1& o1) { return _emitter()->_emitI(CONV(cc), o0, o1); } \
  inline Error NAME##a(const T0& o0, const T1& o1) { return _emitter()->_emitI(Inst::kId##ID##a, o0, o1); } \
  inline Error NAME##ae(const T0& o0, const T1& o1) { return _emitter()->_emitI(Inst::kId##ID##ae, o0, o1); } \
  inline Error NAME##b(const T0& o0, const T1& o1) { return _emitter()->_emitI(Inst::kId##ID##b, o0, o1); } \
  inline Error NAME##be(const T0& o0, const T1& o1) { return _emitter()->_emitI(Inst::kId##ID##be, o0, o1); } \
  inline Error NAME##c(const T0& o0, const T1& o1) { return _emitter()->_emitI(Inst::kId##ID##c, o0, o1); } \
  inline Error NAME##e(const T0& o0, const T1& o1) { return _emitter()->_emitI(Inst::kId##ID##e, o0, o1); } \
  inline Error NAME##g(const T0& o0, const T1& o1) { return _emitter()->_emitI(Inst::kId##ID##g, o0, o1); } \
  inline Error NAME##ge(const T0& o0, const T1& o1) { return _emitter()->_emitI(Inst::kId##ID##ge, o0, o1); } \
  inline Error NAME##l(const T0& o0, const T1& o1) { return _emitter()->_emitI(Inst::kId##ID##l, o0, o1); } \
  inline Error NAME##le(const T0& o0, const T1& o1) { return _emitter()->_emitI(Inst::kId##ID##le, o0, o1); } \
  inline Error NAME##na(const T0& o0, const T1& o1) { return _emitter()->_emitI(Inst::kId##ID##na, o0, o1); } \
  inline Error NAME##nae(const T0& o0, const T1& o1) { return _emitter()->_emitI(Inst::kId##ID##nae, o0, o1); } \
  inline Error NAME##nb(const T0& o0, const T1& o1) { return _emitter()->_emitI(Inst::kId##ID##nb, o0, o1); } \
  inline Error NAME##nbe(const T0& o0, const T1& o1) { return _emitter()->_emitI(Inst::kId##ID##nbe, o0, o1); } \
  inline Error NAME##nc(const T0& o0, const T1& o1) { return _emitter()->_emitI(Inst::kId##ID##nc, o0, o1); } \
  inline Error NAME##ne(const T0& o0, const T1& o1) { return _emitter()->_emitI(Inst::kId##ID##ne, o0, o1); } \
  inline Error NAME##ng(const T0& o0, const T1& o1) { return _emitter()->_emitI(Inst::kId##ID##ng, o0, o1); } \
  inline Error NAME##nge(const T0& o0, const T1& o1) { return _emitter()->_emitI(Inst::kId##ID##nge, o0, o1); } \
  inline Error NAME##nl(const T0& o0, const T1& o1) { return _emitter()->_emitI(Inst::kId##ID##nl, o0, o1); } \
  inline Error NAME##nle(const T0& o0, const T1& o1) { return _emitter()->_emitI(Inst::kId##ID##nle, o0, o1); } \
  inline Error NAME##no(const T0& o0, const T1& o1) { return _emitter()->_emitI(Inst::kId##ID##no, o0, o1); } \
  inline Error NAME##np(const T0& o0, const T1& o1) { return _emitter()->_emitI(Inst::kId##ID##np, o0, o1); } \
  inline Error NAME##ns(const T0& o0, const T1& o1) { return _emitter()->_emitI(Inst::kId##ID##ns, o0, o1); } \
  inline Error NAME##nz(const T0& o0, const T1& o1) { return _emitter()->_emitI(Inst::kId##ID##nz, o0, o1); } \
  inline Error NAME##o(const T0& o0, const T1& o1) { return _emitter()->_emitI(Inst::kId##ID##o, o0, o1); } \
  inline Error NAME##p(const T0& o0, const T1& o1) { return _emitter()->_emitI(Inst::kId##ID##p, o0, o1); } \
  inline Error NAME##pe(const T0& o0, const T1& o1) { return _emitter()->_emitI(Inst::kId##ID##pe, o0, o1); } \
  inline Error NAME##po(const T0& o0, const T1& o1) { return _emitter()->_emitI(Inst::kId##ID##po, o0, o1); } \
  inline Error NAME##s(const T0& o0, const T1& o1) { return _emitter()->_emitI(Inst::kId##ID##s, o0, o1); } \
  inline Error NAME##z(const T0& o0, const T1& o1) { return _emitter()->_emitI(Inst::kId##ID##z, o0, o1); }

#define ASMJIT_INST_3x(NAME, ID, T0, T1, T2) \
  inline Error NAME(const T0& o0, const T1& o1, const T2& o2) { return _emitter()->_emitI(Inst::kId##ID, o0, o1, o2); }

#define ASMJIT_INST_4x(NAME, ID, T0, T1, T2, T3) \
  inline Error NAME(const T0& o0, const T1& o1, const T2& o2, const T3& o3) { return _emitter()->_emitI(Inst::kId##ID, o0, o1, o2, o3); }

#define ASMJIT_INST_5x(NAME, ID, T0, T1, T2, T3, T4) \
  inline Error NAME(const T0& o0, const T1& o1, const T2& o2, const T3& o3, const T4& o4) { return _emitter()->_emitI(Inst::kId##ID, o0, o1, o2, o3, o4); }

#define ASMJIT_INST_6x(NAME, ID, T0, T1, T2, T3, T4, T5) \
  inline Error NAME(const T0& o0, const T1& o1, const T2& o2, const T3& o3, const T4& o4, const T5& o5) { return _emitter()->_emitI(Inst::kId##ID, o0, o1, o2, o3, o4, o5); }

//! \addtogroup asmjit_x86
//! \{

//! Emitter (X86 - explicit).
template<typename This>
struct EmitterExplicitT {
  //! \cond

  // These types are used to describe implicit operands passed explicitly.
  using Gp_AL = Gp;
  using Gp_AH = Gp;
  using Gp_CL = Gp;
  using Gp_AX = Gp;
  using Gp_DX = Gp;

  using Gp_EAX = Gp;
  using Gp_EBX = Gp;
  using Gp_ECX = Gp;
  using Gp_EDX = Gp;

  using Gp_RAX = Gp;
  using Gp_RBX = Gp;
  using Gp_RCX = Gp;
  using Gp_RDX = Gp;

  using Gp_ZAX = Gp;
  using Gp_ZBX = Gp;
  using Gp_ZCX = Gp;
  using Gp_ZDX = Gp;

  using DS_ZAX = Mem; // ds:[zax]
  using DS_ZDI = Mem; // ds:[zdi]
  using ES_ZDI = Mem; // es:[zdi]
  using DS_ZSI = Mem; // ds:[zsi]

  using XMM0 = Vec;

  // These two are unfortunately reported by the sanitizer. We know what we do, however, the sanitizer doesn't.
  // I have tried to use reinterpret_cast instead, but that would generate bad code when compiled by MSC.
  ASMJIT_ATTRIBUTE_NO_SANITIZE_UNDEF ASMJIT_INLINE_NODEBUG This* _emitter() noexcept { return static_cast<This*>(this); }
  ASMJIT_ATTRIBUTE_NO_SANITIZE_UNDEF ASMJIT_INLINE_NODEBUG const This* _emitter() const noexcept { return static_cast<const This*>(this); }

  //! \endcond

  //! \name Native Registers
  //! \{

  //! Returns either 32-bit or 64-bit GP register of the given `id` depending on the emitter's architecture.
  inline Gp gpz(uint32_t id) const noexcept { return Gp(_emitter()->_gp_signature, id); }
  //! Clones the given `reg` to either 32-bit or 64-bit GP register depending on the emitter's architecture.
  inline Gp gpz(const Gp& reg) const noexcept { return Gp(_emitter()->_gp_signature, reg.id()); }

  inline Gp zax() const noexcept { return Gp(_emitter()->_gp_signature, Gp::kIdAx); }
  inline Gp zcx() const noexcept { return Gp(_emitter()->_gp_signature, Gp::kIdCx); }
  inline Gp zdx() const noexcept { return Gp(_emitter()->_gp_signature, Gp::kIdDx); }
  inline Gp zbx() const noexcept { return Gp(_emitter()->_gp_signature, Gp::kIdBx); }
  inline Gp zsp() const noexcept { return Gp(_emitter()->_gp_signature, Gp::kIdSp); }
  inline Gp zbp() const noexcept { return Gp(_emitter()->_gp_signature, Gp::kIdBp); }
  inline Gp zsi() const noexcept { return Gp(_emitter()->_gp_signature, Gp::kIdSi); }
  inline Gp zdi() const noexcept { return Gp(_emitter()->_gp_signature, Gp::kIdDi); }

  //! \}

  //! \name Native Pointers
  //! \{

  //! Creates a target dependent pointer of which base register's id is `base_id`.
  inline Mem ptr_base(uint32_t base_id, int32_t off = 0, uint32_t size = 0) const noexcept {
    return Mem(OperandSignature::from_op_type(OperandType::kMem) |
               OperandSignature::from_mem_base_type(_emitter()->_gp_signature.reg_type()) |
               OperandSignature::from_size(size),
               base_id, 0, off);
  }

  inline Mem ptr_zax(int32_t off = 0, uint32_t size = 0) const noexcept { return ptr_base(Gp::kIdAx, off, size); }
  inline Mem ptr_zcx(int32_t off = 0, uint32_t size = 0) const noexcept { return ptr_base(Gp::kIdCx, off, size); }
  inline Mem ptr_zdx(int32_t off = 0, uint32_t size = 0) const noexcept { return ptr_base(Gp::kIdDx, off, size); }
  inline Mem ptr_zbx(int32_t off = 0, uint32_t size = 0) const noexcept { return ptr_base(Gp::kIdBx, off, size); }
  inline Mem ptr_zsp(int32_t off = 0, uint32_t size = 0) const noexcept { return ptr_base(Gp::kIdSp, off, size); }
  inline Mem ptr_zbp(int32_t off = 0, uint32_t size = 0) const noexcept { return ptr_base(Gp::kIdBp, off, size); }
  inline Mem ptr_zsi(int32_t off = 0, uint32_t size = 0) const noexcept { return ptr_base(Gp::kIdSi, off, size); }
  inline Mem ptr_zdi(int32_t off = 0, uint32_t size = 0) const noexcept { return ptr_base(Gp::kIdDi, off, size); }

  //! Creates an `intptr_t` memory operand depending on the current architecture.
  inline Mem intptr_ptr(const Gp& base, int32_t offset = 0) const noexcept {
    uint32_t reg_size = _emitter()->register_size();
    return Mem(base, offset, reg_size);
  }
  //! \overload
  inline Mem intptr_ptr(const Gp& base, const Gp& index, uint32_t shift = 0, int32_t offset = 0) const noexcept {
    uint32_t reg_size = _emitter()->register_size();
    return Mem(base, index, shift, offset, reg_size);
  }
  //! \overload
  inline Mem intptr_ptr(const Gp& base, const Vec& index, uint32_t shift = 0, int32_t offset = 0) const noexcept {
    uint32_t reg_size = _emitter()->register_size();
    return Mem(base, index, shift, offset, reg_size);
  }
  //! \overload
  inline Mem intptr_ptr(const Label& base, int32_t offset = 0) const noexcept {
    uint32_t reg_size = _emitter()->register_size();
    return Mem(base, offset, reg_size);
  }
  //! \overload
  inline Mem intptr_ptr(const Label& base, const Gp& index, uint32_t shift, int32_t offset = 0) const noexcept {
    uint32_t reg_size = _emitter()->register_size();
    return Mem(base, index, shift, offset, reg_size);
  }
  //! \overload
  inline Mem intptr_ptr(const Label& base, const Vec& index, uint32_t shift, int32_t offset = 0) const noexcept {
    uint32_t reg_size = _emitter()->register_size();
    return Mem(base, index, shift, offset, reg_size);
  }
  //! \overload
  inline Mem intptr_ptr(const Rip& rip, int32_t offset = 0) const noexcept {
    uint32_t reg_size = _emitter()->register_size();
    return Mem(rip, offset, reg_size);
  }
  //! \overload
  inline Mem intptr_ptr(uint64_t base) const noexcept {
    uint32_t reg_size = _emitter()->register_size();
    return Mem(base, reg_size);
  }
  //! \overload
  inline Mem intptr_ptr(uint64_t base, const Gp& index, uint32_t shift = 0) const noexcept {
    uint32_t reg_size = _emitter()->register_size();
    return Mem(base, index, shift, reg_size);
  }
  //! \overload
  inline Mem intptr_ptr_abs(uint64_t base) const noexcept {
    uint32_t reg_size = _emitter()->register_size();
    return Mem(base, reg_size, OperandSignature::from_value<Mem::kSignatureMemAddrTypeMask>(Mem::AddrType::kAbs));
  }
  //! \overload
  inline Mem intptr_ptr_abs(uint64_t base, const Gp& index, uint32_t shift = 0) const noexcept {
    uint32_t reg_size = _emitter()->register_size();
    return Mem(base, index, shift, reg_size, OperandSignature::from_value<Mem::kSignatureMemAddrTypeMask>(Mem::AddrType::kRel));
  }

  //! \}

  //! \name Embed
  //! \{

  //! Embeds 8-bit integer data.
  inline Error db(uint8_t x, size_t repeat_count = 1) { return _emitter()->embed_uint8(x, repeat_count); }
  //! Embeds 16-bit integer data.
  inline Error dw(uint16_t x, size_t repeat_count = 1) { return _emitter()->embed_uint16(x, repeat_count); }
  //! Embeds 32-bit integer data.
  inline Error dd(uint32_t x, size_t repeat_count = 1) { return _emitter()->embed_uint32(x, repeat_count); }
  //! Embeds 64-bit integer data.
  inline Error dq(uint64_t x, size_t repeat_count = 1) { return _emitter()->embed_uint64(x, repeat_count); }

  //! Adds data in a given structure instance to the CodeBuffer.
  template<typename T>
  inline Error dstruct(const T& x) { return _emitter()->embed(&x, uint32_t(sizeof(T))); }

  //! \}

protected:
  //! \cond
  inline This& _add_inst_options(InstOptions options) noexcept {
    _emitter()->add_inst_options(options);
    return *_emitter();
  }
  //! \endcond

public:
  //! \name Short/Long Form Options
  //! \{

  //! Force short form of jmp/jcc instruction.
  inline This& short_() noexcept { return _add_inst_options(InstOptions::kShortForm); }
  //! Force long form of jmp/jcc instruction.
  inline This& long_() noexcept { return _add_inst_options(InstOptions::kLongForm); }

  //! \}

  //! \name Encoding Options
  //! \{

  //! Prefer MOD/RM encoding when both MOD/RM and MOD/MR forms are applicable.
  inline This& mod_rm() noexcept { return _add_inst_options(InstOptions::kX86_ModRM); }

  //! Prefer MOD/MR encoding when both MOD/RM and MOD/MR forms are applicable.
  inline This& mod_mr() noexcept { return _add_inst_options(InstOptions::kX86_ModMR); }

  //! \}

  //! \name Prefix Options
  //! \{

  //! Condition is likely to be taken (has only benefit on P4).
  inline This& taken() noexcept { return _add_inst_options(InstOptions::kTaken); }
  //! Condition is unlikely to be taken (has only benefit on P4).
  inline This& not_taken() noexcept { return _add_inst_options(InstOptions::kNotTaken); }

  //! Use LOCK prefix.
  inline This& lock() noexcept { return _add_inst_options(InstOptions::kX86_Lock); }
  //! Use XACQUIRE prefix.
  inline This& xacquire() noexcept { return _add_inst_options(InstOptions::kX86_XAcquire); }
  //! Use XRELEASE prefix.
  inline This& xrelease() noexcept { return _add_inst_options(InstOptions::kX86_XRelease); }

  //! Use BND/REPNE prefix.
  //!
  //! \note This is the same as using `repne()` or `repnz()` prefix.
  inline This& bnd() noexcept { return _add_inst_options(InstOptions::kX86_Repne); }

  //! Use REP/REPZ prefix.
  //!
  //! \note This is the same as using `repe()` or `repz()` prefix.
  inline This& rep(const Gp& zcx) noexcept {
    _emitter()->_extra_reg.init(zcx);
    return _add_inst_options(InstOptions::kX86_Rep);
  }

  //! Use REP/REPE prefix.
  //!
  //! \note This is the same as using `rep()` or `repz()` prefix.
  inline This& repe(const Gp& zcx) noexcept { return rep(zcx); }

  //! Use REP/REPE prefix.
  //!
  //! \note This is the same as using `rep()` or `repe()` prefix.
  inline This& repz(const Gp& zcx) noexcept { return rep(zcx); }

  //! Use REPNE prefix.
  //!
  //! \note This is the same as using `bnd()` or `repnz()` prefix.
  inline This& repne(const Gp& zcx) noexcept {
    _emitter()->_extra_reg.init(zcx);
    return _add_inst_options(InstOptions::kX86_Repne);
  }

  //! Use REPNE prefix.
  //!
  //! \note This is the same as using `bnd()` or `repne()` prefix.
  inline This& repnz(const Gp& zcx) noexcept { return repne(zcx); }

  //! \}

  //! \name REX Options
  //! \{

  //! Force REX prefix to be emitted even when it's not needed (X86_64).
  //!
  //! \note Don't use when using high 8-bit registers as REX prefix makes them inaccessible and `x86::Assembler`
  //! would fail to encode such instruction.
  inline This& rex() noexcept { return _add_inst_options(InstOptions::kX86_Rex); }

  //! \}

  //! \name VEX and EVEX Options
  //! \{

  //! Use VEX prefix instead of EVEX prefix (useful to select AVX_VNNI instruction instead of AVX512_VNNI).
  inline This& vex() noexcept { return _add_inst_options(InstOptions::kX86_Vex); }
  //! Force 3-byte VEX prefix (AVX+).
  inline This& vex3() noexcept { return _add_inst_options(InstOptions::kX86_Vex3); }
  //! Force 4-byte EVEX prefix (AVX512+).
  inline This& evex() noexcept { return _add_inst_options(InstOptions::kX86_Evex); }

  //! \}

  //! \name AVX-512 Options & Masking
  //! \{

  //! Use masking {k} (AVX512+).
  inline This& k(const KReg& kreg) noexcept {
    _emitter()->_extra_reg.init(kreg);
    return *_emitter();
  }

  //! Use zeroing instead of merging (AVX512+).
  inline This& z() noexcept { return _add_inst_options(InstOptions::kX86_ZMask); }

  //! Suppress all exceptions (AVX512+).
  inline This& sae() noexcept { return _add_inst_options(InstOptions::kX86_SAE); }
  //! Static rounding mode {rn} (round-to-nearest even) and {sae} (AVX512+).
  inline This& rn_sae() noexcept { return _add_inst_options(InstOptions::kX86_ER | InstOptions::kX86_RN_SAE); }
  //! Static rounding mode {rd} (round-down, toward -inf) and {sae} (AVX512+).
  inline This& rd_sae() noexcept { return _add_inst_options(InstOptions::kX86_ER | InstOptions::kX86_RD_SAE); }
  //! Static rounding mode {ru} (round-up, toward +inf) and {sae} (AVX512+).
  inline This& ru_sae() noexcept { return _add_inst_options(InstOptions::kX86_ER | InstOptions::kX86_RU_SAE); }
  //! Static rounding mode {rz} (round-toward-zero, truncate) and {sae} (AVX512+).
  inline This& rz_sae() noexcept { return _add_inst_options(InstOptions::kX86_ER | InstOptions::kX86_RZ_SAE); }

  //! \}

  //! \name Core Instructions
  //! \{

  ASMJIT_INST_2x(adc, Adc, Gp, Gp)                                     // ANY
  ASMJIT_INST_2x(adc, Adc, Gp, Mem)                                    // ANY
  ASMJIT_INST_2x(adc, Adc, Gp, Imm)                                    // ANY
  ASMJIT_INST_2x(adc, Adc, Mem, Gp)                                    // ANY
  ASMJIT_INST_2x(adc, Adc, Mem, Imm)                                   // ANY
  ASMJIT_INST_2x(add, Add, Gp, Gp)                                     // ANY
  ASMJIT_INST_2x(add, Add, Gp, Mem)                                    // ANY
  ASMJIT_INST_2x(add, Add, Gp, Imm)                                    // ANY
  ASMJIT_INST_2x(add, Add, Mem, Gp)                                    // ANY
  ASMJIT_INST_2x(add, Add, Mem, Imm)                                   // ANY
  ASMJIT_INST_2x(and_, And, Gp, Gp)                                    // ANY
  ASMJIT_INST_2x(and_, And, Gp, Mem)                                   // ANY
  ASMJIT_INST_2x(and_, And, Gp, Imm)                                   // ANY
  ASMJIT_INST_2x(and_, And, Mem, Gp)                                   // ANY
  ASMJIT_INST_2x(and_, And, Mem, Imm)                                  // ANY
  ASMJIT_INST_2x(bound, Bound, Gp, Mem)                                // X86
  ASMJIT_INST_2x(bsf, Bsf, Gp, Gp)                                     // ANY
  ASMJIT_INST_2x(bsf, Bsf, Gp, Mem)                                    // ANY
  ASMJIT_INST_2x(bsr, Bsr, Gp, Gp)                                     // ANY
  ASMJIT_INST_2x(bsr, Bsr, Gp, Mem)                                    // ANY
  ASMJIT_INST_1x(bswap, Bswap, Gp)                                     // ANY
  ASMJIT_INST_2x(bt, Bt, Gp, Gp)                                       // ANY
  ASMJIT_INST_2x(bt, Bt, Gp, Imm)                                      // ANY
  ASMJIT_INST_2x(bt, Bt, Mem, Gp)                                      // ANY
  ASMJIT_INST_2x(bt, Bt, Mem, Imm)                                     // ANY
  ASMJIT_INST_2x(btc, Btc, Gp, Gp)                                     // ANY
  ASMJIT_INST_2x(btc, Btc, Gp, Imm)                                    // ANY
  ASMJIT_INST_2x(btc, Btc, Mem, Gp)                                    // ANY
  ASMJIT_INST_2x(btc, Btc, Mem, Imm)                                   // ANY
  ASMJIT_INST_2x(btr, Btr, Gp, Gp)                                     // ANY
  ASMJIT_INST_2x(btr, Btr, Gp, Imm)                                    // ANY
  ASMJIT_INST_2x(btr, Btr, Mem, Gp)                                    // ANY
  ASMJIT_INST_2x(btr, Btr, Mem, Imm)                                   // ANY
  ASMJIT_INST_2x(bts, Bts, Gp, Gp)                                     // ANY
  ASMJIT_INST_2x(bts, Bts, Gp, Imm)                                    // ANY
  ASMJIT_INST_2x(bts, Bts, Mem, Gp)                                    // ANY
  ASMJIT_INST_2x(bts, Bts, Mem, Imm)                                   // ANY
  ASMJIT_INST_1x(cbw, Cbw, Gp_AX)                                      // ANY [EXPLICIT] AX      <- Sign Extend AL
  ASMJIT_INST_2x(cdq, Cdq, Gp_EDX, Gp_EAX)                             // ANY [EXPLICIT] EDX:EAX <- Sign Extend EAX
  ASMJIT_INST_1x(cdqe, Cdqe, Gp_EAX)                                   // X64 [EXPLICIT] RAX     <- Sign Extend EAX
  ASMJIT_INST_2x(cqo, Cqo, Gp_RDX, Gp_RAX)                             // X64 [EXPLICIT] RDX:RAX <- Sign Extend RAX
  ASMJIT_INST_2x(cwd, Cwd, Gp_DX, Gp_AX)                               // ANY [EXPLICIT] DX:AX   <- Sign Extend AX
  ASMJIT_INST_1x(cwde, Cwde, Gp_EAX)                                   // ANY [EXPLICIT] EAX     <- Sign Extend AX
  ASMJIT_INST_1x(call, Call, Gp)                                       // ANY
  ASMJIT_INST_1x(call, Call, Mem)                                      // ANY
  ASMJIT_INST_1x(call, Call, Label)                                    // ANY
  ASMJIT_INST_1x(call, Call, Imm)                                      // ANY
  ASMJIT_INST_2c(cmov, Cmov, Inst::cmovcc_from_cond, Gp, Gp)             // CMOV
  ASMJIT_INST_2c(cmov, Cmov, Inst::cmovcc_from_cond, Gp, Mem)            // CMOV
  ASMJIT_INST_2x(cmp, Cmp, Gp, Gp)                                     // ANY
  ASMJIT_INST_2x(cmp, Cmp, Gp, Mem)                                    // ANY
  ASMJIT_INST_2x(cmp, Cmp, Gp, Imm)                                    // ANY
  ASMJIT_INST_2x(cmp, Cmp, Mem, Gp)                                    // ANY
  ASMJIT_INST_2x(cmp, Cmp, Mem, Imm)                                   // ANY
  ASMJIT_INST_2x(cmps, Cmps, DS_ZSI, ES_ZDI)                           // ANY [EXPLICIT]
  ASMJIT_INST_3x(cmpxchg, Cmpxchg, Gp, Gp, Gp_ZAX)                     // I486 [EXPLICIT]
  ASMJIT_INST_3x(cmpxchg, Cmpxchg, Mem, Gp, Gp_ZAX)                    // I486 [EXPLICIT]
  ASMJIT_INST_5x(cmpxchg16b, Cmpxchg16b, Mem, Gp_RDX, Gp_RAX, Gp_RCX, Gp_RBX) // CMPXCHG16B [EXPLICIT] m == EDX:EAX ? m <- ECX:EBX
  ASMJIT_INST_5x(cmpxchg8b, Cmpxchg8b, Mem, Gp_EDX, Gp_EAX, Gp_ECX, Gp_EBX)   // CMPXCHG8B  [EXPLICIT] m == RDX:RAX ? m <- RCX:RBX
  ASMJIT_INST_1x(dec, Dec, Gp)                                         // ANY
  ASMJIT_INST_1x(dec, Dec, Mem)                                        // ANY
  ASMJIT_INST_2x(div, Div, Gp, Gp)                                     // ANY [EXPLICIT]  AH[Rem]: AL[Quot] <- AX / r8
  ASMJIT_INST_2x(div, Div, Gp, Mem)                                    // ANY [EXPLICIT]  AH[Rem]: AL[Quot] <- AX / m8
  ASMJIT_INST_3x(div, Div, Gp, Gp, Gp)                                 // ANY [EXPLICIT] xDX[Rem]:xAX[Quot] <- xDX:xAX / r16|r32|r64
  ASMJIT_INST_3x(div, Div, Gp, Gp, Mem)                                // ANY [EXPLICIT] xDX[Rem]:xAX[Quot] <- xDX:xAX / m16|m32|m64
  ASMJIT_INST_2x(idiv, Idiv, Gp, Gp)                                   // ANY [EXPLICIT]  AH[Rem]: AL[Quot] <- AX / r8
  ASMJIT_INST_2x(idiv, Idiv, Gp, Mem)                                  // ANY [EXPLICIT]  AH[Rem]: AL[Quot] <- AX / m8
  ASMJIT_INST_3x(idiv, Idiv, Gp, Gp, Gp)                               // ANY [EXPLICIT] xDX[Rem]:xAX[Quot] <- xDX:xAX / r16|r32|r64
  ASMJIT_INST_3x(idiv, Idiv, Gp, Gp, Mem)                              // ANY [EXPLICIT] xDX[Rem]:xAX[Quot] <- xDX:xAX / m16|m32|m64
  ASMJIT_INST_2x(imul, Imul, Gp, Gp)                                   // ANY [EXPLICIT] AX <- AL * r8 | ra <- ra * rb
  ASMJIT_INST_2x(imul, Imul, Gp, Mem)                                  // ANY [EXPLICIT] AX <- AL * m8 | ra <- ra * m16|m32|m64
  ASMJIT_INST_3x(imul, Imul, Gp, Gp, Imm)                              // ANY
  ASMJIT_INST_3x(imul, Imul, Gp, Mem, Imm)                             // ANY
  ASMJIT_INST_3x(imul, Imul, Gp, Gp, Gp)                               // ANY [EXPLICIT] xDX:xAX <- xAX * r16|r32|r64
  ASMJIT_INST_3x(imul, Imul, Gp, Gp, Mem)                              // ANY [EXPLICIT] xDX:xAX <- xAX * m16|m32|m64
  ASMJIT_INST_1x(inc, Inc, Gp)                                         // ANY
  ASMJIT_INST_1x(inc, Inc, Mem)                                        // ANY
  ASMJIT_INST_1c(j, J, Inst::jcc_from_cond, Label)                       // ANY
  ASMJIT_INST_1c(j, J, Inst::jcc_from_cond, Imm)                         // ANY
  ASMJIT_INST_2x(jecxz, Jecxz, Gp, Label)                              // ANY [EXPLICIT] Short jump if CX/ECX/RCX is zero.
  ASMJIT_INST_2x(jecxz, Jecxz, Gp, Imm)                                // ANY [EXPLICIT] Short jump if CX/ECX/RCX is zero.
  ASMJIT_INST_1x(jmp, Jmp, Gp)                                         // ANY
  ASMJIT_INST_1x(jmp, Jmp, Mem)                                        // ANY
  ASMJIT_INST_1x(jmp, Jmp, Label)                                      // ANY
  ASMJIT_INST_1x(jmp, Jmp, Imm)                                        // ANY
  ASMJIT_INST_2x(lcall, Lcall, Imm, Imm)                               // ANY
  ASMJIT_INST_1x(lcall, Lcall, Mem)                                    // ANY
  ASMJIT_INST_2x(lea, Lea, Gp, Mem)                                    // ANY
  ASMJIT_INST_2x(ljmp, Ljmp, Imm, Imm)                                 // ANY
  ASMJIT_INST_1x(ljmp, Ljmp, Mem)                                      // ANY
  ASMJIT_INST_2x(lods, Lods, Gp_ZAX, DS_ZSI)                           // ANY [EXPLICIT]
  ASMJIT_INST_2x(loop, Loop, Gp_ZCX, Label)                            // ANY [EXPLICIT] Decrement xCX; short jump if xCX != 0.
  ASMJIT_INST_2x(loop, Loop, Gp_ZCX, Imm)                              // ANY [EXPLICIT] Decrement xCX; short jump if xCX != 0.
  ASMJIT_INST_2x(loope, Loope, Gp_ZCX, Label)                          // ANY [EXPLICIT] Decrement xCX; short jump if xCX != 0 && ZF == 1.
  ASMJIT_INST_2x(loope, Loope, Gp_ZCX, Imm)                            // ANY [EXPLICIT] Decrement xCX; short jump if xCX != 0 && ZF == 1.
  ASMJIT_INST_2x(loopne, Loopne, Gp_ZCX, Label)                        // ANY [EXPLICIT] Decrement xCX; short jump if xCX != 0 && ZF == 0.
  ASMJIT_INST_2x(loopne, Loopne, Gp_ZCX, Imm)                          // ANY [EXPLICIT] Decrement xCX; short jump if xCX != 0 && ZF == 0.
  ASMJIT_INST_2x(mov, Mov, Gp, Gp)                                     // ANY
  ASMJIT_INST_2x(mov, Mov, Gp, Mem)                                    // ANY
  ASMJIT_INST_2x(mov, Mov, Gp, Imm)                                    // ANY
  ASMJIT_INST_2x(mov, Mov, Mem, Gp)                                    // ANY
  ASMJIT_INST_2x(mov, Mov, Mem, Imm)                                   // ANY
  ASMJIT_INST_2x(mov, Mov, Gp, CReg)                                   // ANY
  ASMJIT_INST_2x(mov, Mov, CReg, Gp)                                   // ANY
  ASMJIT_INST_2x(mov, Mov, Gp, DReg)                                   // ANY
  ASMJIT_INST_2x(mov, Mov, DReg, Gp)                                   // ANY
  ASMJIT_INST_2x(mov, Mov, Gp, SReg)                                   // ANY
  ASMJIT_INST_2x(mov, Mov, Mem, SReg)                                  // ANY
  ASMJIT_INST_2x(mov, Mov, SReg, Gp)                                   // ANY
  ASMJIT_INST_2x(mov, Mov, SReg, Mem)                                  // ANY
  ASMJIT_INST_2x(movabs, Movabs, Gp, Mem)                              // X64
  ASMJIT_INST_2x(movabs, Movabs, Gp, Imm)                              // X64
  ASMJIT_INST_2x(movabs, Movabs, Mem, Gp)                              // X64
  ASMJIT_INST_2x(movnti, Movnti, Mem, Gp)                              // SSE2
  ASMJIT_INST_2x(movs, Movs, ES_ZDI, DS_ZSI)                           // ANY [EXPLICIT]
  ASMJIT_INST_2x(movsx, Movsx, Gp, Gp)                                 // ANY
  ASMJIT_INST_2x(movsx, Movsx, Gp, Mem)                                // ANY
  ASMJIT_INST_2x(movsxd, Movsxd, Gp, Gp)                               // X64
  ASMJIT_INST_2x(movsxd, Movsxd, Gp, Mem)                              // X64
  ASMJIT_INST_2x(movzx, Movzx, Gp, Gp)                                 // ANY
  ASMJIT_INST_2x(movzx, Movzx, Gp, Mem)                                // ANY
  ASMJIT_INST_2x(mul, Mul, Gp_AX, Gp)                                  // ANY [EXPLICIT] AX      <-  AL * r8
  ASMJIT_INST_2x(mul, Mul, Gp_AX, Mem)                                 // ANY [EXPLICIT] AX      <-  AL * m8
  ASMJIT_INST_3x(mul, Mul, Gp_ZDX, Gp_ZAX, Gp)                         // ANY [EXPLICIT] xDX:xAX <- xAX * r16|r32|r64
  ASMJIT_INST_3x(mul, Mul, Gp_ZDX, Gp_ZAX, Mem)                        // ANY [EXPLICIT] xDX:xAX <- xAX * m16|m32|m64
  ASMJIT_INST_1x(neg, Neg, Gp)                                         // ANY
  ASMJIT_INST_1x(neg, Neg, Mem)                                        // ANY
  ASMJIT_INST_0x(nop, Nop)                                             // ANY
  ASMJIT_INST_1x(nop, Nop, Gp)                                         // ANY
  ASMJIT_INST_1x(nop, Nop, Mem)                                        // ANY
  ASMJIT_INST_2x(nop, Nop, Gp, Gp)                                     // ANY
  ASMJIT_INST_2x(nop, Nop, Mem, Gp)                                    // ANY
  ASMJIT_INST_1x(not_, Not, Gp)                                        // ANY
  ASMJIT_INST_1x(not_, Not, Mem)                                       // ANY
  ASMJIT_INST_2x(or_, Or, Gp, Gp)                                      // ANY
  ASMJIT_INST_2x(or_, Or, Gp, Mem)                                     // ANY
  ASMJIT_INST_2x(or_, Or, Gp, Imm)                                     // ANY
  ASMJIT_INST_2x(or_, Or, Mem, Gp)                                     // ANY
  ASMJIT_INST_2x(or_, Or, Mem, Imm)                                    // ANY
  ASMJIT_INST_1x(pop, Pop, Gp)                                         // ANY
  ASMJIT_INST_1x(pop, Pop, Mem)                                        // ANY
  ASMJIT_INST_1x(pop, Pop, SReg)                                       // ANY
  ASMJIT_INST_0x(popa, Popa)                                           // X86
  ASMJIT_INST_0x(popad, Popad)                                         // X86
  ASMJIT_INST_0x(popf, Popf)                                           // ANY
  ASMJIT_INST_0x(popfd, Popfd)                                         // X86
  ASMJIT_INST_0x(popfq, Popfq)                                         // X64
  ASMJIT_INST_1x(push, Push, Gp)                                       // ANY
  ASMJIT_INST_1x(push, Push, Mem)                                      // ANY
  ASMJIT_INST_1x(push, Push, SReg)                                     // ANY
  ASMJIT_INST_1x(push, Push, Imm)                                      // ANY
  ASMJIT_INST_0x(pusha, Pusha)                                         // X86
  ASMJIT_INST_0x(pushad, Pushad)                                       // X86
  ASMJIT_INST_0x(pushf, Pushf)                                         // ANY
  ASMJIT_INST_0x(pushfd, Pushfd)                                       // X86
  ASMJIT_INST_0x(pushfq, Pushfq)                                       // X64
  ASMJIT_INST_1x(pushw, Pushw, Imm)                                    // ANY
  ASMJIT_INST_2x(rcl, Rcl, Gp, Gp_CL)                                  // ANY
  ASMJIT_INST_2x(rcl, Rcl, Mem, Gp_CL)                                 // ANY
  ASMJIT_INST_2x(rcl, Rcl, Gp, Imm)                                    // ANY
  ASMJIT_INST_2x(rcl, Rcl, Mem, Imm)                                   // ANY
  ASMJIT_INST_2x(rcr, Rcr, Gp, Gp_CL)                                  // ANY
  ASMJIT_INST_2x(rcr, Rcr, Mem, Gp_CL)                                 // ANY
  ASMJIT_INST_2x(rcr, Rcr, Gp, Imm)                                    // ANY
  ASMJIT_INST_2x(rcr, Rcr, Mem, Imm)                                   // ANY
  ASMJIT_INST_2x(rol, Rol, Gp, Gp_CL)                                  // ANY
  ASMJIT_INST_2x(rol, Rol, Mem, Gp_CL)                                 // ANY
  ASMJIT_INST_2x(rol, Rol, Gp, Imm)                                    // ANY
  ASMJIT_INST_2x(rol, Rol, Mem, Imm)                                   // ANY
  ASMJIT_INST_2x(ror, Ror, Gp, Gp_CL)                                  // ANY
  ASMJIT_INST_2x(ror, Ror, Mem, Gp_CL)                                 // ANY
  ASMJIT_INST_2x(ror, Ror, Gp, Imm)                                    // ANY
  ASMJIT_INST_2x(ror, Ror, Mem, Imm)                                   // ANY
  ASMJIT_INST_2x(sbb, Sbb, Gp, Gp)                                     // ANY
  ASMJIT_INST_2x(sbb, Sbb, Gp, Mem)                                    // ANY
  ASMJIT_INST_2x(sbb, Sbb, Gp, Imm)                                    // ANY
  ASMJIT_INST_2x(sbb, Sbb, Mem, Gp)                                    // ANY
  ASMJIT_INST_2x(sbb, Sbb, Mem, Imm)                                   // ANY
  ASMJIT_INST_2x(sal, Shl, Gp, Gp_CL)                                  // ANY
  ASMJIT_INST_2x(sal, Shl, Mem, Gp_CL)                                 // ANY
  ASMJIT_INST_2x(sal, Shl, Gp, Imm)                                    // ANY
  ASMJIT_INST_2x(sal, Shl, Mem, Imm)                                   // ANY
  ASMJIT_INST_2x(sar, Sar, Gp, Gp_CL)                                  // ANY
  ASMJIT_INST_2x(sar, Sar, Mem, Gp_CL)                                 // ANY
  ASMJIT_INST_2x(sar, Sar, Gp, Imm)                                    // ANY
  ASMJIT_INST_2x(sar, Sar, Mem, Imm)                                   // ANY
  ASMJIT_INST_2x(scas, Scas, Gp_ZAX, ES_ZDI)                           // ANY [EXPLICIT]
  ASMJIT_INST_1c(set, Set, Inst::setcc_from_cond, Gp)                    // ANY
  ASMJIT_INST_1c(set, Set, Inst::setcc_from_cond, Mem)                   // ANY
  ASMJIT_INST_2x(shl, Shl, Gp, Gp_CL)                                  // ANY
  ASMJIT_INST_2x(shl, Shl, Mem, Gp_CL)                                 // ANY
  ASMJIT_INST_2x(shl, Shl, Gp, Imm)                                    // ANY
  ASMJIT_INST_2x(shl, Shl, Mem, Imm)                                   // ANY
  ASMJIT_INST_2x(shr, Shr, Gp, Gp_CL)                                  // ANY
  ASMJIT_INST_2x(shr, Shr, Mem, Gp_CL)                                 // ANY
  ASMJIT_INST_2x(shr, Shr, Gp, Imm)                                    // ANY
  ASMJIT_INST_2x(shr, Shr, Mem, Imm)                                   // ANY
  ASMJIT_INST_3x(shld, Shld, Gp, Gp, Gp_CL)                            // ANY
  ASMJIT_INST_3x(shld, Shld, Mem, Gp, Gp_CL)                           // ANY
  ASMJIT_INST_3x(shld, Shld, Gp, Gp, Imm)                              // ANY
  ASMJIT_INST_3x(shld, Shld, Mem, Gp, Imm)                             // ANY
  ASMJIT_INST_3x(shrd, Shrd, Gp, Gp, Gp_CL)                            // ANY
  ASMJIT_INST_3x(shrd, Shrd, Mem, Gp, Gp_CL)                           // ANY
  ASMJIT_INST_3x(shrd, Shrd, Gp, Gp, Imm)                              // ANY
  ASMJIT_INST_3x(shrd, Shrd, Mem, Gp, Imm)                             // ANY
  ASMJIT_INST_2x(stos, Stos, ES_ZDI, Gp_ZAX)                           // ANY [EXPLICIT]
  ASMJIT_INST_2x(sub, Sub, Gp, Gp)                                     // ANY
  ASMJIT_INST_2x(sub, Sub, Gp, Mem)                                    // ANY
  ASMJIT_INST_2x(sub, Sub, Gp, Imm)                                    // ANY
  ASMJIT_INST_2x(sub, Sub, Mem, Gp)                                    // ANY
  ASMJIT_INST_2x(sub, Sub, Mem, Imm)                                   // ANY
  ASMJIT_INST_2x(test, Test, Gp, Gp)                                   // ANY
  ASMJIT_INST_2x(test, Test, Gp, Imm)                                  // ANY
  ASMJIT_INST_2x(test, Test, Mem, Gp)                                  // ANY
  ASMJIT_INST_2x(test, Test, Mem, Imm)                                 // ANY
  ASMJIT_INST_2x(ud0, Ud0, Gp, Gp)                                     // ANY
  ASMJIT_INST_2x(ud0, Ud0, Gp, Mem)                                    // ANY
  ASMJIT_INST_2x(ud1, Ud1, Gp, Gp)                                     // ANY
  ASMJIT_INST_2x(ud1, Ud1, Gp, Mem)                                    // ANY
  ASMJIT_INST_0x(ud2, Ud2)                                             // ANY
  ASMJIT_INST_2x(xadd, Xadd, Gp, Gp)                                   // ANY
  ASMJIT_INST_2x(xadd, Xadd, Mem, Gp)                                  // ANY
  ASMJIT_INST_2x(xchg, Xchg, Gp, Gp)                                   // ANY
  ASMJIT_INST_2x(xchg, Xchg, Mem, Gp)                                  // ANY
  ASMJIT_INST_2x(xchg, Xchg, Gp, Mem)                                  // ANY
  ASMJIT_INST_2x(xor_, Xor, Gp, Gp)                                    // ANY
  ASMJIT_INST_2x(xor_, Xor, Gp, Mem)                                   // ANY
  ASMJIT_INST_2x(xor_, Xor, Gp, Imm)                                   // ANY
  ASMJIT_INST_2x(xor_, Xor, Mem, Gp)                                   // ANY
  ASMJIT_INST_2x(xor_, Xor, Mem, Imm)                                  // ANY

  //! \}

  //! \name Core Instructions (Aliases)
  //! \{

  //! The `imul(Gp, Imm)` instruction is an alias of `imul(Gp, Gp, Imm)` instruction.
  inline Error imul(const Gp& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdImul, o0, o0, o1); }

  //! \}

  //! \name Deprecated 32-bit Instructions
  //! \{

  ASMJIT_INST_1x(aaa, Aaa, Gp)                                         // X86 [EXPLICIT]
  ASMJIT_INST_2x(aad, Aad, Gp, Imm)                                    // X86 [EXPLICIT]
  ASMJIT_INST_2x(aam, Aam, Gp, Imm)                                    // X86 [EXPLICIT]
  ASMJIT_INST_1x(aas, Aas, Gp)                                         // X86 [EXPLICIT]
  ASMJIT_INST_1x(daa, Daa, Gp)                                         // X86 [EXPLICIT]
  ASMJIT_INST_1x(das, Das, Gp)                                         // X86 [EXPLICIT]

  //! \}

  //! \name ENTER/LEAVE Instructions
  //! \{

  ASMJIT_INST_2x(enter, Enter, Imm, Imm)                               // ANY
  ASMJIT_INST_0x(leave, Leave)                                         // ANY

  //! \}

  //! \name IN/OUT Instructions
  //! \{

  // NOTE: For some reason Doxygen is messed up here and thinks we are in cond.

  ASMJIT_INST_2x(in, In, Gp_ZAX, Imm)                                  // ANY
  ASMJIT_INST_2x(in, In, Gp_ZAX, Gp_DX)                                // ANY
  ASMJIT_INST_2x(ins, Ins, ES_ZDI, Gp_DX)                              // ANY
  ASMJIT_INST_2x(out, Out, Imm, Gp_ZAX)                                // ANY
  ASMJIT_INST_2x(out, Out, Gp_DX, Gp_ZAX)                              // ANY
  ASMJIT_INST_2x(outs, Outs, Gp_DX, DS_ZSI)                            // ANY

  //! \}

  //! \name Clear/Set CF/DF Instructions
  //! \{

  ASMJIT_INST_0x(clc, Clc)                                             // ANY
  ASMJIT_INST_0x(cld, Cld)                                             // ANY
  ASMJIT_INST_0x(cmc, Cmc)                                             // ANY
  ASMJIT_INST_0x(stc, Stc)                                             // ANY
  ASMJIT_INST_0x(std, Std)                                             // ANY

  //! \}

  //! \name ADX Instructions
  //! \{

  ASMJIT_INST_2x(adcx, Adcx, Gp, Gp)                                   // ADX
  ASMJIT_INST_2x(adcx, Adcx, Gp, Mem)                                  // ADX
  ASMJIT_INST_2x(adox, Adox, Gp, Gp)                                   // ADX
  ASMJIT_INST_2x(adox, Adox, Gp, Mem)                                  // ADX

  //! \}

  //! \name CPUID Instruction
  //! \{

  ASMJIT_INST_4x(cpuid, Cpuid, Gp_EAX, Gp_EBX, Gp_ECX, Gp_EDX)         // I486 [EXPLICIT] EAX:EBX:ECX:EDX <- CPUID[EAX:ECX]

  //! \}

  //! \name LAHF/SAHF Instructions
  //! \{

  ASMJIT_INST_1x(lahf, Lahf, Gp_AH)                                    // LAHFSAHF [EXPLICIT] AH <- EFL
  ASMJIT_INST_1x(sahf, Sahf, Gp_AH)                                    // LAHFSAHF [EXPLICIT] EFL <- AH

  //! \}

  //! \name BMI Instructions
  //! \{

  ASMJIT_INST_3x(andn, Andn, Gp, Gp, Gp)                               // BMI
  ASMJIT_INST_3x(andn, Andn, Gp, Gp, Mem)                              // BMI
  ASMJIT_INST_3x(bextr, Bextr, Gp, Gp, Gp)                             // BMI
  ASMJIT_INST_3x(bextr, Bextr, Gp, Mem, Gp)                            // BMI
  ASMJIT_INST_2x(blsi, Blsi, Gp, Gp)                                   // BMI
  ASMJIT_INST_2x(blsi, Blsi, Gp, Mem)                                  // BMI
  ASMJIT_INST_2x(blsmsk, Blsmsk, Gp, Gp)                               // BMI
  ASMJIT_INST_2x(blsmsk, Blsmsk, Gp, Mem)                              // BMI
  ASMJIT_INST_2x(blsr, Blsr, Gp, Gp)                                   // BMI
  ASMJIT_INST_2x(blsr, Blsr, Gp, Mem)                                  // BMI
  ASMJIT_INST_2x(tzcnt, Tzcnt, Gp, Gp)                                 // BMI
  ASMJIT_INST_2x(tzcnt, Tzcnt, Gp, Mem)                                // BMI

  //! \}

  //! \name BMI2 Instructions
  //! \{

  ASMJIT_INST_3x(bzhi, Bzhi, Gp, Gp, Gp)                               // BMI2
  ASMJIT_INST_3x(bzhi, Bzhi, Gp, Mem, Gp)                              // BMI2
  ASMJIT_INST_4x(mulx, Mulx, Gp, Gp, Gp, Gp_ZDX)                       // BMI2      [EXPLICIT]
  ASMJIT_INST_4x(mulx, Mulx, Gp, Gp, Mem, Gp_ZDX)                      // BMI2      [EXPLICIT]
  ASMJIT_INST_3x(pdep, Pdep, Gp, Gp, Gp)                               // BMI2
  ASMJIT_INST_3x(pdep, Pdep, Gp, Gp, Mem)                              // BMI2
  ASMJIT_INST_3x(pext, Pext, Gp, Gp, Gp)                               // BMI2
  ASMJIT_INST_3x(pext, Pext, Gp, Gp, Mem)                              // BMI2
  ASMJIT_INST_3x(rorx, Rorx, Gp, Gp, Imm)                              // BMI2
  ASMJIT_INST_3x(rorx, Rorx, Gp, Mem, Imm)                             // BMI2
  ASMJIT_INST_3x(sarx, Sarx, Gp, Gp, Gp)                               // BMI2
  ASMJIT_INST_3x(sarx, Sarx, Gp, Mem, Gp)                              // BMI2
  ASMJIT_INST_3x(shlx, Shlx, Gp, Gp, Gp)                               // BMI2
  ASMJIT_INST_3x(shlx, Shlx, Gp, Mem, Gp)                              // BMI2
  ASMJIT_INST_3x(shrx, Shrx, Gp, Gp, Gp)                               // BMI2
  ASMJIT_INST_3x(shrx, Shrx, Gp, Mem, Gp)                              // BMI2

  //! \}

  //! \name CMPCCXADD Instructions
  //! \{

  ASMJIT_INST_3x(cmpbexadd, Cmpbexadd, Mem, Gp, Gp)
  ASMJIT_INST_3x(cmpbxadd, Cmpbxadd, Mem, Gp, Gp)
  ASMJIT_INST_3x(cmplexadd, Cmplexadd, Mem, Gp, Gp)
  ASMJIT_INST_3x(cmplxadd, Cmplxadd, Mem, Gp, Gp)
  ASMJIT_INST_3x(cmpnbexadd, Cmpnbexadd, Mem, Gp, Gp)
  ASMJIT_INST_3x(cmpnbxadd, Cmpnbxadd, Mem, Gp, Gp)
  ASMJIT_INST_3x(cmpnlexadd, Cmpnlexadd, Mem, Gp, Gp)
  ASMJIT_INST_3x(cmpnlxadd, Cmpnlxadd, Mem, Gp, Gp)
  ASMJIT_INST_3x(cmpnoxadd, Cmpnoxadd, Mem, Gp, Gp)
  ASMJIT_INST_3x(cmpnpxadd, Cmpnpxadd, Mem, Gp, Gp)
  ASMJIT_INST_3x(cmpnsxadd, Cmpnsxadd, Mem, Gp, Gp)
  ASMJIT_INST_3x(cmpnzxadd, Cmpnzxadd, Mem, Gp, Gp)
  ASMJIT_INST_3x(cmpoxadd, Cmpoxadd, Mem, Gp, Gp)
  ASMJIT_INST_3x(cmppxadd, Cmppxadd, Mem, Gp, Gp)
  ASMJIT_INST_3x(cmpsxadd, Cmpsxadd, Mem, Gp, Gp)
  ASMJIT_INST_3x(cmpzxadd, Cmpzxadd, Mem, Gp, Gp)

  //! \}

  //! \name CacheLine Instructions
  //! \{

  ASMJIT_INST_1x(cldemote, Cldemote, Mem)                              // CLDEMOTE
  ASMJIT_INST_1x(clflush, Clflush, Mem)                                // CLFLUSH
  ASMJIT_INST_1x(clflushopt, Clflushopt, Mem)                          // CLFLUSH_OPT
  ASMJIT_INST_1x(clwb, Clwb, Mem)                                      // CLWB
  ASMJIT_INST_1x(clzero, Clzero, DS_ZAX)                               // CLZERO [EXPLICIT]

  //! \}

  //! \name CRC32 Instructions (SSE4.2)
  //! \{

  ASMJIT_INST_2x(crc32, Crc32, Gp, Gp)                                 // SSE4_2
  ASMJIT_INST_2x(crc32, Crc32, Gp, Mem)                                // SSE4_2

  //! \}

  //! \name FENCE Instructions (SSE and SSE2)
  //! \{

  ASMJIT_INST_0x(lfence, Lfence)                                       // SSE2
  ASMJIT_INST_0x(mfence, Mfence)                                       // SSE2
  ASMJIT_INST_0x(sfence, Sfence)                                       // SSE

  //! \}

  //! \name LZCNT Instructions
  //! \{

  ASMJIT_INST_2x(lzcnt, Lzcnt, Gp, Gp)                                 // LZCNT
  ASMJIT_INST_2x(lzcnt, Lzcnt, Gp, Mem)                                // LZCNT

  //! \}

  //! \name MOVBE Instructions
  //! \{

  ASMJIT_INST_2x(movbe, Movbe, Gp, Mem)                                // MOVBE
  ASMJIT_INST_2x(movbe, Movbe, Mem, Gp)                                // MOVBE

  //! \}

  //! \name MOVDIRI & MOVDIR64B Instructions
  //! \{

  ASMJIT_INST_2x(movdiri, Movdiri, Mem, Gp)                            // MOVDIRI
  ASMJIT_INST_2x(movdir64b, Movdir64b, Mem, Mem)                       // MOVDIR64B

  //! \}

  //! \name MXCSR Instructions (SSE)
  //! \{

  ASMJIT_INST_1x(ldmxcsr, Ldmxcsr, Mem)                                // SSE
  ASMJIT_INST_1x(stmxcsr, Stmxcsr, Mem)                                // SSE

  //! \}

  //! \name POPCNT Instructions
  //! \{

  ASMJIT_INST_2x(popcnt, Popcnt, Gp, Gp)                               // POPCNT
  ASMJIT_INST_2x(popcnt, Popcnt, Gp, Mem)                              // POPCNT

  //! \}

  //! \name PREFETCH Instructions
  //! \{

  ASMJIT_INST_1x(prefetch, Prefetch, Mem)                              // 3DNOW
  ASMJIT_INST_1x(prefetchnta, Prefetchnta, Mem)                        // SSE
  ASMJIT_INST_1x(prefetcht0, Prefetcht0, Mem)                          // SSE
  ASMJIT_INST_1x(prefetcht1, Prefetcht1, Mem)                          // SSE
  ASMJIT_INST_1x(prefetcht2, Prefetcht2, Mem)                          // SSE
  ASMJIT_INST_1x(prefetchw, Prefetchw, Mem)                            // PREFETCHW
  ASMJIT_INST_1x(prefetchwt1, Prefetchwt1, Mem)                        // PREFETCHW1

  //! \}

  //! \name PREFETCHI Instructions
  //! \{

  ASMJIT_INST_1x(prefetchit0, Prefetchit0, Mem)
  ASMJIT_INST_1x(prefetchit1, Prefetchit1, Mem)

  //! \}

  //! \name RAO_INT Instructions
  //! \{

  ASMJIT_INST_2x(aadd, Aadd, Mem, Gp)
  ASMJIT_INST_2x(aand, Aand, Mem, Gp)
  ASMJIT_INST_2x(aor, Aor, Mem, Gp)
  ASMJIT_INST_2x(axor, Axor, Mem, Gp)

  //! \}

  //! \name RDPID Instruction
  //! \{

  ASMJIT_INST_1x(rdpid, Rdpid, Gp)                                     // RDPID

  //! \}

  //! \name RDPRU/RDPKRU Instructions
  //! \{

  ASMJIT_INST_3x(rdpru, Rdpru, Gp_EDX, Gp_EAX, Gp_ECX)                 // RDPRU     [EXPLICIT] EDX:EAX <- PRU[ECX]
  ASMJIT_INST_3x(rdpkru, Rdpkru, Gp_EDX, Gp_EAX, Gp_ECX)               // RDPKRU    [EXPLICIT] EDX:EAX <- PKRU[ECX]

  //! \}

  //! \name RDTSC/RDTSCP Instructions
  //! \{

  ASMJIT_INST_2x(rdtsc, Rdtsc, Gp_EDX, Gp_EAX)                         // RDTSC     [EXPLICIT] EDX:EAX     <- Counter
  ASMJIT_INST_3x(rdtscp, Rdtscp, Gp_EDX, Gp_EAX, Gp_ECX)               // RDTSCP    [EXPLICIT] EDX:EAX:EXC <- Counter

  //! \}

  //! \name SERIALIZE Instruction
  //! \{

  ASMJIT_INST_0x(serialize, Serialize)                                 // SERIALIZE

  //! \}

  //! \name TBM Instructions
  //! \{

  ASMJIT_INST_2x(blcfill, Blcfill, Gp, Gp)                             // TBM
  ASMJIT_INST_2x(blcfill, Blcfill, Gp, Mem)                            // TBM
  ASMJIT_INST_2x(blci, Blci, Gp, Gp)                                   // TBM
  ASMJIT_INST_2x(blci, Blci, Gp, Mem)                                  // TBM
  ASMJIT_INST_2x(blcic, Blcic, Gp, Gp)                                 // TBM
  ASMJIT_INST_2x(blcic, Blcic, Gp, Mem)                                // TBM
  ASMJIT_INST_2x(blcmsk, Blcmsk, Gp, Gp)                               // TBM
  ASMJIT_INST_2x(blcmsk, Blcmsk, Gp, Mem)                              // TBM
  ASMJIT_INST_2x(blcs, Blcs, Gp, Gp)                                   // TBM
  ASMJIT_INST_2x(blcs, Blcs, Gp, Mem)                                  // TBM
  ASMJIT_INST_2x(blsfill, Blsfill, Gp, Gp)                             // TBM
  ASMJIT_INST_2x(blsfill, Blsfill, Gp, Mem)                            // TBM
  ASMJIT_INST_2x(blsic, Blsic, Gp, Gp)                                 // TBM
  ASMJIT_INST_2x(blsic, Blsic, Gp, Mem)                                // TBM
  ASMJIT_INST_2x(t1mskc, T1mskc, Gp, Gp)                               // TBM
  ASMJIT_INST_2x(t1mskc, T1mskc, Gp, Mem)                              // TBM
  ASMJIT_INST_2x(tzmsk, Tzmsk, Gp, Gp)                                 // TBM
  ASMJIT_INST_2x(tzmsk, Tzmsk, Gp, Mem)                                // TBM

  //! \}

  //! \name Other User-Mode Instructions
  //! \{

  ASMJIT_INST_2x(arpl, Arpl, Gp, Gp)                                   // X86
  ASMJIT_INST_2x(arpl, Arpl, Mem, Gp)                                  // X86
  ASMJIT_INST_0x(cli, Cli)                                             // ANY
  ASMJIT_INST_0x(getsec, Getsec)                                       // SMX
  ASMJIT_INST_1x(int_, Int, Imm)                                       // ANY
  ASMJIT_INST_0x(int3, Int3)                                           // ANY
  ASMJIT_INST_0x(into, Into)                                           // ANY
  ASMJIT_INST_2x(lar, Lar, Gp, Gp)                                     // ANY
  ASMJIT_INST_2x(lar, Lar, Gp, Mem)                                    // ANY
  ASMJIT_INST_2x(lds, Lds, Gp, Mem)                                    // X86
  ASMJIT_INST_2x(les, Les, Gp, Mem)                                    // X86
  ASMJIT_INST_2x(lfs, Lfs, Gp, Mem)                                    // ANY
  ASMJIT_INST_2x(lgs, Lgs, Gp, Mem)                                    // ANY
  ASMJIT_INST_2x(lsl, Lsl, Gp, Gp)                                     // ANY
  ASMJIT_INST_2x(lsl, Lsl, Gp, Mem)                                    // ANY
  ASMJIT_INST_2x(lss, Lss, Gp, Mem)                                    // ANY
  ASMJIT_INST_0x(pause, Pause)                                         // SSE2
  ASMJIT_INST_0x(rsm, Rsm)                                             // X86
  ASMJIT_INST_1x(sgdt, Sgdt, Mem)                                      // ANY
  ASMJIT_INST_1x(sidt, Sidt, Mem)                                      // ANY
  ASMJIT_INST_1x(sldt, Sldt, Gp)                                       // ANY
  ASMJIT_INST_1x(sldt, Sldt, Mem)                                      // ANY
  ASMJIT_INST_1x(smsw, Smsw, Gp)                                       // ANY
  ASMJIT_INST_1x(smsw, Smsw, Mem)                                      // ANY
  ASMJIT_INST_0x(sti, Sti)                                             // ANY
  ASMJIT_INST_1x(str, Str, Gp)                                         // ANY
  ASMJIT_INST_1x(str, Str, Mem)                                        // ANY
  ASMJIT_INST_1x(verr, Verr, Gp)                                       // ANY
  ASMJIT_INST_1x(verr, Verr, Mem)                                      // ANY
  ASMJIT_INST_1x(verw, Verw, Gp)                                       // ANY
  ASMJIT_INST_1x(verw, Verw, Mem)                                      // ANY

  //! \}

  //! \name FSGSBASE Instructions
  //! \{

  ASMJIT_INST_1x(rdfsbase, Rdfsbase, Gp)                               // FSGSBASE
  ASMJIT_INST_1x(rdgsbase, Rdgsbase, Gp)                               // FSGSBASE
  ASMJIT_INST_1x(wrfsbase, Wrfsbase, Gp)                               // FSGSBASE
  ASMJIT_INST_1x(wrgsbase, Wrgsbase, Gp)                               // FSGSBASE

  //! \}

  //! \name FXSR Instructions
  //! \{

  ASMJIT_INST_1x(fxrstor, Fxrstor, Mem)                                // FXSR
  ASMJIT_INST_1x(fxrstor64, Fxrstor64, Mem)                            // FXSR
  ASMJIT_INST_1x(fxsave, Fxsave, Mem)                                  // FXSR
  ASMJIT_INST_1x(fxsave64, Fxsave64, Mem)                              // FXSR

  //! \}

  //! \name XSAVE Instructions
  //! \{

  ASMJIT_INST_3x(xgetbv, Xgetbv, Gp_EDX, Gp_EAX, Gp_ECX)               // XSAVE     [EXPLICIT] EDX:EAX <- XCR[ECX]
  ASMJIT_INST_3x(xrstor, Xrstor, Mem, Gp_EDX, Gp_EAX)                  // XSAVE     [EXPLICIT]
  ASMJIT_INST_3x(xrstor64, Xrstor64, Mem, Gp_EDX, Gp_EAX)              // XSAVE+X64 [EXPLICIT]
  ASMJIT_INST_3x(xrstors, Xrstors, Mem, Gp_EDX, Gp_EAX)                // XSAVE     [EXPLICIT]
  ASMJIT_INST_3x(xrstors64, Xrstors64, Mem, Gp_EDX, Gp_EAX)            // XSAVE+X64 [EXPLICIT]
  ASMJIT_INST_3x(xsave, Xsave, Mem, Gp_EDX, Gp_EAX)                    // XSAVE     [EXPLICIT]
  ASMJIT_INST_3x(xsave64, Xsave64, Mem, Gp_EDX, Gp_EAX)                // XSAVE+X64 [EXPLICIT]
  ASMJIT_INST_3x(xsavec, Xsavec, Mem, Gp_EDX, Gp_EAX)                  // XSAVE     [EXPLICIT]
  ASMJIT_INST_3x(xsavec64, Xsavec64, Mem, Gp_EDX, Gp_EAX)              // XSAVE+X64 [EXPLICIT]
  ASMJIT_INST_3x(xsaveopt, Xsaveopt, Mem, Gp_EDX, Gp_EAX)              // XSAVE     [EXPLICIT]
  ASMJIT_INST_3x(xsaveopt64, Xsaveopt64, Mem, Gp_EDX, Gp_EAX)          // XSAVE+X64 [EXPLICIT]
  ASMJIT_INST_3x(xsaves, Xsaves, Mem, Gp_EDX, Gp_EAX)                  // XSAVE     [EXPLICIT]
  ASMJIT_INST_3x(xsaves64, Xsaves64, Mem, Gp_EDX, Gp_EAX)              // XSAVE+X64 [EXPLICIT]

  //! \}

  //! \name MPX Extensions
  //! \{

  ASMJIT_INST_2x(bndcl, Bndcl, Bnd, Gp)                                // MPX
  ASMJIT_INST_2x(bndcl, Bndcl, Bnd, Mem)                               // MPX
  ASMJIT_INST_2x(bndcn, Bndcn, Bnd, Gp)                                // MPX
  ASMJIT_INST_2x(bndcn, Bndcn, Bnd, Mem)                               // MPX
  ASMJIT_INST_2x(bndcu, Bndcu, Bnd, Gp)                                // MPX
  ASMJIT_INST_2x(bndcu, Bndcu, Bnd, Mem)                               // MPX
  ASMJIT_INST_2x(bndldx, Bndldx, Bnd, Mem)                             // MPX
  ASMJIT_INST_2x(bndmk, Bndmk, Bnd, Mem)                               // MPX
  ASMJIT_INST_2x(bndmov, Bndmov, Bnd, Bnd)                             // MPX
  ASMJIT_INST_2x(bndmov, Bndmov, Bnd, Mem)                             // MPX
  ASMJIT_INST_2x(bndmov, Bndmov, Mem, Bnd)                             // MPX
  ASMJIT_INST_2x(bndstx, Bndstx, Mem, Bnd)                             // MPX

  //! \}

  //! \name MONITORX Instructions
  //! \{

  ASMJIT_INST_3x(monitorx, Monitorx, Mem, Gp, Gp)                      // MONITORX
  ASMJIT_INST_3x(mwaitx, Mwaitx, Gp, Gp, Gp)                           // MONITORX

  //! \}

  //! \name MCOMMIT Instruction
  //! \{

  ASMJIT_INST_0x(mcommit, Mcommit)                                     // MCOMMIT

  //! \}

  //! \name PTWRITE Instruction
  //! \{

  ASMJIT_INST_1x(ptwrite, Ptwrite, Gp)                                 // PTWRITE
  ASMJIT_INST_1x(ptwrite, Ptwrite, Mem)                                // PTWRITE

  //! \}

  //! \name ENQCMD Instructions
  //! \{

  ASMJIT_INST_2x(enqcmd, Enqcmd, Mem, Mem)                             // ENQCMD
  ASMJIT_INST_2x(enqcmds, Enqcmds, Mem, Mem)                           // ENQCMD

  //! \}

  //! \name WAITPKG Instructions
  //! \{

  ASMJIT_INST_3x(tpause, Tpause, Gp, Gp, Gp)                           // WAITPKG
  ASMJIT_INST_1x(umonitor, Umonitor, Mem)                              // WAITPKG
  ASMJIT_INST_3x(umwait, Umwait, Gp, Gp, Gp)                           // WAITPKG

  //! \}

  //! \name RDRAND & RDSEED Instructions
  //! \{

  ASMJIT_INST_1x(rdrand, Rdrand, Gp)                                   // RDRAND
  ASMJIT_INST_1x(rdseed, Rdseed, Gp)                                   // RDSEED

  //! \}

  //! \name LWP Instructions
  //! \{

  ASMJIT_INST_1x(llwpcb, Llwpcb, Gp)                                   // LWP
  ASMJIT_INST_3x(lwpins, Lwpins, Gp, Gp, Imm)                          // LWP
  ASMJIT_INST_3x(lwpins, Lwpins, Gp, Mem, Imm)                         // LWP
  ASMJIT_INST_3x(lwpval, Lwpval, Gp, Gp, Imm)                          // LWP
  ASMJIT_INST_3x(lwpval, Lwpval, Gp, Mem, Imm)                         // LWP
  ASMJIT_INST_1x(slwpcb, Slwpcb, Gp)                                   // LWP

  //! \}

  //! \name RTM & TSX Instructions
  //! \{

  ASMJIT_INST_1x(xabort, Xabort, Imm)                                  // RTM
  ASMJIT_INST_1x(xbegin, Xbegin, Label)                                // RTM
  ASMJIT_INST_1x(xbegin, Xbegin, Imm)                                  // RTM
  ASMJIT_INST_0x(xend, Xend)                                           // RTM
  ASMJIT_INST_0x(xtest, Xtest)                                         // TSX

  //! \}

  //! \name TSXLDTRK Instructions
  //! \{

  ASMJIT_INST_0x(xresldtrk, Xresldtrk)                                 // TSXLDTRK
  ASMJIT_INST_0x(xsusldtrk, Xsusldtrk)                                 // TSXLDTRK

  //! \}

  //! \name CET-IBT Instructions
  //! \{

  ASMJIT_INST_0x(endbr32, Endbr32)                                     // CET_IBT
  ASMJIT_INST_0x(endbr64, Endbr64)                                     // CET_IBT

  //! \}

  //! \name CET-SS Instructions
  //! \{

  ASMJIT_INST_1x(clrssbsy, Clrssbsy, Mem)                              // CET_SS
  ASMJIT_INST_0x(setssbsy, Setssbsy)                                   // CET_SS

  ASMJIT_INST_1x(rstorssp, Rstorssp, Mem)                              // CET_SS
  ASMJIT_INST_0x(saveprevssp, Saveprevssp)                             // CET_SS

  ASMJIT_INST_1x(incsspd, Incsspd, Gp)                                 // CET_SS
  ASMJIT_INST_1x(incsspq, Incsspq, Gp)                                 // CET_SS
  ASMJIT_INST_1x(rdsspd, Rdsspd, Gp)                                   // CET_SS
  ASMJIT_INST_1x(rdsspq, Rdsspq, Gp)                                   // CET_SS
  ASMJIT_INST_2x(wrssd, Wrssd, Gp, Gp)                                 // CET_SS
  ASMJIT_INST_2x(wrssd, Wrssd, Mem, Gp)                                // CET_SS
  ASMJIT_INST_2x(wrssq, Wrssq, Gp, Gp)                                 // CET_SS
  ASMJIT_INST_2x(wrssq, Wrssq, Mem, Gp)                                // CET_SS
  ASMJIT_INST_2x(wrussd, Wrussd, Gp, Gp)                               // CET_SS
  ASMJIT_INST_2x(wrussd, Wrussd, Mem, Gp)                              // CET_SS
  ASMJIT_INST_2x(wrussq, Wrussq, Gp, Gp)                               // CET_SS
  ASMJIT_INST_2x(wrussq, Wrussq, Mem, Gp)                              // CET_SS

  //! \}

  //! \name HRESET Instructions
  //! \{

  ASMJIT_INST_2x(hreset, Hreset, Imm, Gp)                              // HRESET

  //! \}

  //! \name UINTR Instructions
  //! \{

  ASMJIT_INST_0x(clui, Clui)                                           // UINTR
  ASMJIT_INST_1x(senduipi, Senduipi, Gp)                               // UINTR
  ASMJIT_INST_0x(testui, Testui)                                       // UINTR
  ASMJIT_INST_0x(stui, Stui)                                           // UINTR
  ASMJIT_INST_0x(uiret, Uiret)                                         // UINTR

  //! \}

  //! \name Core Privileged Instructions
  //! \{

  ASMJIT_INST_0x(clts, Clts)                                           // ANY
  ASMJIT_INST_0x(hlt, Hlt)                                             // ANY
  ASMJIT_INST_0x(invd, Invd)                                           // ANY
  ASMJIT_INST_1x(invlpg, Invlpg, Mem)                                  // ANY
  ASMJIT_INST_2x(invpcid, Invpcid, Gp, Mem)                            // ANY
  ASMJIT_INST_1x(lgdt, Lgdt, Mem)                                      // ANY
  ASMJIT_INST_1x(lidt, Lidt, Mem)                                      // ANY
  ASMJIT_INST_1x(lldt, Lldt, Gp)                                       // ANY
  ASMJIT_INST_1x(lldt, Lldt, Mem)                                      // ANY
  ASMJIT_INST_1x(lmsw, Lmsw, Gp)                                       // ANY
  ASMJIT_INST_1x(lmsw, Lmsw, Mem)                                      // ANY
  ASMJIT_INST_1x(ltr, Ltr, Gp)                                         // ANY
  ASMJIT_INST_1x(ltr, Ltr, Mem)                                        // ANY
  ASMJIT_INST_3x(rdmsr, Rdmsr, Gp_EDX, Gp_EAX, Gp_ECX)                 // MSR       [EXPLICIT] RDX:EAX <- MSR[ECX]
  ASMJIT_INST_3x(rdpmc, Rdpmc, Gp_EDX, Gp_EAX, Gp_ECX)                 // ANY       [EXPLICIT] RDX:EAX <- PMC[ECX]
  ASMJIT_INST_0x(swapgs, Swapgs)                                       // X64
  ASMJIT_INST_0x(wbinvd, Wbinvd)                                       // ANY
  ASMJIT_INST_0x(wbnoinvd, Wbnoinvd)                                   // WBNOINVD
  ASMJIT_INST_3x(wrmsr, Wrmsr, Gp_EDX, Gp_EAX, Gp_ECX)                 // MSR       [EXPLICIT] RDX:EAX  -> MSR[ECX]
  ASMJIT_INST_3x(xsetbv, Xsetbv, Gp_EDX, Gp_EAX, Gp_ECX)               // XSAVE     [EXPLICIT] XCR[ECX] <- EDX:EAX

  //! \}

  //! \name INVLPGB Instructions
  //! \{

  ASMJIT_INST_3x(invlpgb, Invlpgb, Gp_EAX, Gp_EDX, Gp_ECX)
  ASMJIT_INST_0x(tlbsync, Tlbsync)

  //! \}

  //! \name MONITOR Instructions (Privileged)
  //! \{

  ASMJIT_INST_3x(monitor, Monitor, Mem, Gp, Gp)                        // MONITOR
  ASMJIT_INST_2x(mwait, Mwait, Gp, Gp)                                 // MONITOR

  //! \}

  //! \name SMAP Instructions (Privileged)
  //! \{

  ASMJIT_INST_0x(clac, Clac)                                           // SMAP
  ASMJIT_INST_0x(stac, Stac)                                           // SMAP

  //! \}

  //! \name SKINIT Instructions (Privileged)
  //! \{

  ASMJIT_INST_1x(skinit, Skinit, Gp)                                   // SKINIT    [EXPLICIT] <eax>
  ASMJIT_INST_0x(stgi, Stgi)                                           // SKINIT

  //! \}

  //! \name SNP Instructions (Privileged)
  //! \{

  ASMJIT_INST_0x(psmash, Psmash)                                       // SNP
  ASMJIT_INST_0x(pvalidate, Pvalidate)                                 // SNP
  ASMJIT_INST_0x(rmpadjust, Rmpadjust)                                 // SNP
  ASMJIT_INST_0x(rmpupdate, Rmpupdate)                                 // SNP

  //! \}

  //! \name VMX Instructions (All privileged except vmfunc)
  //! \{

  ASMJIT_INST_2x(invept, Invept, Gp, Mem)                              // VMX
  ASMJIT_INST_2x(invvpid, Invvpid, Gp, Mem)                            // VMX
  ASMJIT_INST_0x(vmcall, Vmcall)                                       // VMX
  ASMJIT_INST_1x(vmclear, Vmclear, Mem)                                // VMX
  ASMJIT_INST_0x(vmfunc, Vmfunc)                                       // VMX
  ASMJIT_INST_0x(vmlaunch, Vmlaunch)                                   // VMX
  ASMJIT_INST_1x(vmptrld, Vmptrld, Mem)                                // VMX
  ASMJIT_INST_1x(vmptrst, Vmptrst, Mem)                                // VMX
  ASMJIT_INST_2x(vmread, Vmread, Gp, Gp)                               // VMX
  ASMJIT_INST_2x(vmread, Vmread, Mem, Gp)                              // VMX
  ASMJIT_INST_0x(vmresume, Vmresume)                                   // VMX
  ASMJIT_INST_2x(vmwrite, Vmwrite, Gp, Mem)                            // VMX
  ASMJIT_INST_2x(vmwrite, Vmwrite, Gp, Gp)                             // VMX
  ASMJIT_INST_0x(vmxoff, Vmxoff)                                       // VMX
  ASMJIT_INST_1x(vmxon, Vmxon, Mem)                                    // VMX

  //! \}

  //! \name SVM Instructions (All privileged except vmmcall)
  //! \{

  ASMJIT_INST_0x(clgi, Clgi)                                           // SVM
  ASMJIT_INST_2x(invlpga, Invlpga, Gp, Gp)                             // SVM       [EXPLICIT] <eax|rax, ecx>
  ASMJIT_INST_1x(vmload, Vmload, Gp)                                   // SVM       [EXPLICIT] <zax>
  ASMJIT_INST_0x(vmmcall, Vmmcall)                                     // SVM
  ASMJIT_INST_1x(vmrun, Vmrun, Gp)                                     // SVM       [EXPLICIT] <zax>
  ASMJIT_INST_1x(vmsave, Vmsave, Gp)                                   // SVM       [EXPLICIT] <zax>

  //! \}

  //! \name SEV_ES Instructions
  //! \{

  ASMJIT_INST_0x(vmgexit, Vmgexit)

  //! \}

  //! \name FPU Instructions
  //! \{

  ASMJIT_INST_0x(f2xm1, F2xm1)                                         // FPU
  ASMJIT_INST_0x(fabs, Fabs)                                           // FPU
  ASMJIT_INST_2x(fadd, Fadd, St, St)                                   // FPU
  ASMJIT_INST_1x(fadd, Fadd, Mem)                                      // FPU
  ASMJIT_INST_1x(faddp, Faddp, St)                                     // FPU
  ASMJIT_INST_0x(faddp, Faddp)                                         // FPU
  ASMJIT_INST_1x(fbld, Fbld, Mem)                                      // FPU
  ASMJIT_INST_1x(fbstp, Fbstp, Mem)                                    // FPU
  ASMJIT_INST_0x(fchs, Fchs)                                           // FPU
  ASMJIT_INST_0x(fclex, Fclex)                                         // FPU
  ASMJIT_INST_1x(fcmovb, Fcmovb, St)                                   // FPU
  ASMJIT_INST_1x(fcmovbe, Fcmovbe, St)                                 // FPU
  ASMJIT_INST_1x(fcmove, Fcmove, St)                                   // FPU
  ASMJIT_INST_1x(fcmovnb, Fcmovnb, St)                                 // FPU
  ASMJIT_INST_1x(fcmovnbe, Fcmovnbe, St)                               // FPU
  ASMJIT_INST_1x(fcmovne, Fcmovne, St)                                 // FPU
  ASMJIT_INST_1x(fcmovnu, Fcmovnu, St)                                 // FPU
  ASMJIT_INST_1x(fcmovu, Fcmovu, St)                                   // FPU
  ASMJIT_INST_1x(fcom, Fcom, St)                                       // FPU
  ASMJIT_INST_0x(fcom, Fcom)                                           // FPU
  ASMJIT_INST_1x(fcom, Fcom, Mem)                                      // FPU
  ASMJIT_INST_1x(fcomp, Fcomp, St)                                     // FPU
  ASMJIT_INST_0x(fcomp, Fcomp)                                         // FPU
  ASMJIT_INST_1x(fcomp, Fcomp, Mem)                                    // FPU
  ASMJIT_INST_0x(fcompp, Fcompp)                                       // FPU
  ASMJIT_INST_1x(fcomi, Fcomi, St)                                     // FPU
  ASMJIT_INST_1x(fcomip, Fcomip, St)                                   // FPU
  ASMJIT_INST_0x(fcos, Fcos)                                           // FPU
  ASMJIT_INST_0x(fdecstp, Fdecstp)                                     // FPU
  ASMJIT_INST_2x(fdiv, Fdiv, St, St)                                   // FPU
  ASMJIT_INST_1x(fdiv, Fdiv, Mem)                                      // FPU
  ASMJIT_INST_1x(fdivp, Fdivp, St)                                     // FPU
  ASMJIT_INST_0x(fdivp, Fdivp)                                         // FPU
  ASMJIT_INST_2x(fdivr, Fdivr, St, St)                                 // FPU
  ASMJIT_INST_1x(fdivr, Fdivr, Mem)                                    // FPU
  ASMJIT_INST_1x(fdivrp, Fdivrp, St)                                   // FPU
  ASMJIT_INST_0x(fdivrp, Fdivrp)                                       // FPU
  ASMJIT_INST_1x(ffree, Ffree, St)                                     // FPU
  ASMJIT_INST_1x(fiadd, Fiadd, Mem)                                    // FPU
  ASMJIT_INST_1x(ficom, Ficom, Mem)                                    // FPU
  ASMJIT_INST_1x(ficomp, Ficomp, Mem)                                  // FPU
  ASMJIT_INST_1x(fidiv, Fidiv, Mem)                                    // FPU
  ASMJIT_INST_1x(fidivr, Fidivr, Mem)                                  // FPU
  ASMJIT_INST_1x(fild, Fild, Mem)                                      // FPU
  ASMJIT_INST_1x(fimul, Fimul, Mem)                                    // FPU
  ASMJIT_INST_0x(fincstp, Fincstp)                                     // FPU
  ASMJIT_INST_0x(finit, Finit)                                         // FPU
  ASMJIT_INST_1x(fisub, Fisub, Mem)                                    // FPU
  ASMJIT_INST_1x(fisubr, Fisubr, Mem)                                  // FPU
  ASMJIT_INST_0x(fninit, Fninit)                                       // FPU
  ASMJIT_INST_1x(fist, Fist, Mem)                                      // FPU
  ASMJIT_INST_1x(fistp, Fistp, Mem)                                    // FPU
  ASMJIT_INST_1x(fisttp, Fisttp, Mem)                                  // FPU+SSE3
  ASMJIT_INST_1x(fld, Fld, Mem)                                        // FPU
  ASMJIT_INST_1x(fld, Fld, St)                                         // FPU
  ASMJIT_INST_0x(fld1, Fld1)                                           // FPU
  ASMJIT_INST_0x(fldl2t, Fldl2t)                                       // FPU
  ASMJIT_INST_0x(fldl2e, Fldl2e)                                       // FPU
  ASMJIT_INST_0x(fldpi, Fldpi)                                         // FPU
  ASMJIT_INST_0x(fldlg2, Fldlg2)                                       // FPU
  ASMJIT_INST_0x(fldln2, Fldln2)                                       // FPU
  ASMJIT_INST_0x(fldz, Fldz)                                           // FPU
  ASMJIT_INST_1x(fldcw, Fldcw, Mem)                                    // FPU
  ASMJIT_INST_1x(fldenv, Fldenv, Mem)                                  // FPU
  ASMJIT_INST_2x(fmul, Fmul, St, St)                                   // FPU
  ASMJIT_INST_1x(fmul, Fmul, Mem)                                      // FPU
  ASMJIT_INST_1x(fmulp, Fmulp, St)                                     // FPU
  ASMJIT_INST_0x(fmulp, Fmulp)                                         // FPU
  ASMJIT_INST_0x(fnclex, Fnclex)                                       // FPU
  ASMJIT_INST_0x(fnop, Fnop)                                           // FPU
  ASMJIT_INST_1x(fnsave, Fnsave, Mem)                                  // FPU
  ASMJIT_INST_1x(fnstenv, Fnstenv, Mem)                                // FPU
  ASMJIT_INST_1x(fnstcw, Fnstcw, Mem)                                  // FPU
  ASMJIT_INST_0x(fpatan, Fpatan)                                       // FPU
  ASMJIT_INST_0x(fprem, Fprem)                                         // FPU
  ASMJIT_INST_0x(fprem1, Fprem1)                                       // FPU
  ASMJIT_INST_0x(fptan, Fptan)                                         // FPU
  ASMJIT_INST_0x(frndint, Frndint)                                     // FPU
  ASMJIT_INST_1x(frstor, Frstor, Mem)                                  // FPU
  ASMJIT_INST_1x(fsave, Fsave, Mem)                                    // FPU
  ASMJIT_INST_0x(fscale, Fscale)                                       // FPU
  ASMJIT_INST_0x(fsin, Fsin)                                           // FPU
  ASMJIT_INST_0x(fsincos, Fsincos)                                     // FPU
  ASMJIT_INST_0x(fsqrt, Fsqrt)                                         // FPU
  ASMJIT_INST_1x(fst, Fst, Mem)                                        // FPU
  ASMJIT_INST_1x(fst, Fst, St)                                         // FPU
  ASMJIT_INST_1x(fstp, Fstp, Mem)                                      // FPU
  ASMJIT_INST_1x(fstp, Fstp, St)                                       // FPU
  ASMJIT_INST_1x(fstcw, Fstcw, Mem)                                    // FPU
  ASMJIT_INST_1x(fstenv, Fstenv, Mem)                                  // FPU
  ASMJIT_INST_2x(fsub, Fsub, St, St)                                   // FPU
  ASMJIT_INST_1x(fsub, Fsub, Mem)                                      // FPU
  ASMJIT_INST_1x(fsubp, Fsubp, St)                                     // FPU
  ASMJIT_INST_0x(fsubp, Fsubp)                                         // FPU
  ASMJIT_INST_2x(fsubr, Fsubr, St, St)                                 // FPU
  ASMJIT_INST_1x(fsubr, Fsubr, Mem)                                    // FPU
  ASMJIT_INST_1x(fsubrp, Fsubrp, St)                                   // FPU
  ASMJIT_INST_0x(fsubrp, Fsubrp)                                       // FPU
  ASMJIT_INST_0x(ftst, Ftst)                                           // FPU
  ASMJIT_INST_1x(fucom, Fucom, St)                                     // FPU
  ASMJIT_INST_0x(fucom, Fucom)                                         // FPU
  ASMJIT_INST_1x(fucomi, Fucomi, St)                                   // FPU
  ASMJIT_INST_1x(fucomip, Fucomip, St)                                 // FPU
  ASMJIT_INST_1x(fucomp, Fucomp, St)                                   // FPU
  ASMJIT_INST_0x(fucomp, Fucomp)                                       // FPU
  ASMJIT_INST_0x(fucompp, Fucompp)                                     // FPU
  ASMJIT_INST_0x(fwait, Fwait)                                         // FPU
  ASMJIT_INST_0x(fxam, Fxam)                                           // FPU
  ASMJIT_INST_1x(fxch, Fxch, St)                                       // FPU
  ASMJIT_INST_0x(fxtract, Fxtract)                                     // FPU
  ASMJIT_INST_0x(fyl2x, Fyl2x)                                         // FPU
  ASMJIT_INST_0x(fyl2xp1, Fyl2xp1)                                     // FPU
  ASMJIT_INST_1x(fstsw, Fstsw, Gp)                                     // FPU
  ASMJIT_INST_1x(fstsw, Fstsw, Mem)                                    // FPU
  ASMJIT_INST_1x(fnstsw, Fnstsw, Gp)                                   // FPU
  ASMJIT_INST_1x(fnstsw, Fnstsw, Mem)                                  // FPU

  //! \}

  //! \name MMX & SSE+ Instructions
  //! \{

  ASMJIT_INST_2x(addpd, Addpd, Vec, Vec)                               // SSE2
  ASMJIT_INST_2x(addpd, Addpd, Vec, Mem)                               // SSE2
  ASMJIT_INST_2x(addps, Addps, Vec, Vec)                               // SSE
  ASMJIT_INST_2x(addps, Addps, Vec, Mem)                               // SSE
  ASMJIT_INST_2x(addsd, Addsd, Vec, Vec)                               // SSE2
  ASMJIT_INST_2x(addsd, Addsd, Vec, Mem)                               // SSE2
  ASMJIT_INST_2x(addss, Addss, Vec, Vec)                               // SSE
  ASMJIT_INST_2x(addss, Addss, Vec, Mem)                               // SSE
  ASMJIT_INST_2x(addsubpd, Addsubpd, Vec, Vec)                         // SSE3
  ASMJIT_INST_2x(addsubpd, Addsubpd, Vec, Mem)                         // SSE3
  ASMJIT_INST_2x(addsubps, Addsubps, Vec, Vec)                         // SSE3
  ASMJIT_INST_2x(addsubps, Addsubps, Vec, Mem)                         // SSE3
  ASMJIT_INST_2x(andnpd, Andnpd, Vec, Vec)                             // SSE2
  ASMJIT_INST_2x(andnpd, Andnpd, Vec, Mem)                             // SSE2
  ASMJIT_INST_2x(andnps, Andnps, Vec, Vec)                             // SSE
  ASMJIT_INST_2x(andnps, Andnps, Vec, Mem)                             // SSE
  ASMJIT_INST_2x(andpd, Andpd, Vec, Vec)                               // SSE2
  ASMJIT_INST_2x(andpd, Andpd, Vec, Mem)                               // SSE2
  ASMJIT_INST_2x(andps, Andps, Vec, Vec)                               // SSE
  ASMJIT_INST_2x(andps, Andps, Vec, Mem)                               // SSE
  ASMJIT_INST_3x(blendpd, Blendpd, Vec, Vec, Imm)                      // SSE4_1
  ASMJIT_INST_3x(blendpd, Blendpd, Vec, Mem, Imm)                      // SSE4_1
  ASMJIT_INST_3x(blendps, Blendps, Vec, Vec, Imm)                      // SSE4_1
  ASMJIT_INST_3x(blendps, Blendps, Vec, Mem, Imm)                      // SSE4_1
  ASMJIT_INST_3x(blendvpd, Blendvpd, Vec, Vec, XMM0)                   // SSE4_1 [EXPLICIT]
  ASMJIT_INST_3x(blendvpd, Blendvpd, Vec, Mem, XMM0)                   // SSE4_1 [EXPLICIT]
  ASMJIT_INST_3x(blendvps, Blendvps, Vec, Vec, XMM0)                   // SSE4_1 [EXPLICIT]
  ASMJIT_INST_3x(blendvps, Blendvps, Vec, Mem, XMM0)                   // SSE4_1 [EXPLICIT]
  ASMJIT_INST_3x(cmppd, Cmppd, Vec, Vec, Imm)                          // SSE2
  ASMJIT_INST_3x(cmppd, Cmppd, Vec, Mem, Imm)                          // SSE2
  ASMJIT_INST_3x(cmpps, Cmpps, Vec, Vec, Imm)                          // SSE
  ASMJIT_INST_3x(cmpps, Cmpps, Vec, Mem, Imm)                          // SSE
  ASMJIT_INST_3x(cmpsd, Cmpsd, Vec, Vec, Imm)                          // SSE2
  ASMJIT_INST_3x(cmpsd, Cmpsd, Vec, Mem, Imm)                          // SSE2
  ASMJIT_INST_3x(cmpss, Cmpss, Vec, Vec, Imm)                          // SSE
  ASMJIT_INST_3x(cmpss, Cmpss, Vec, Mem, Imm)                          // SSE
  ASMJIT_INST_2x(comisd, Comisd, Vec, Vec)                             // SSE2
  ASMJIT_INST_2x(comisd, Comisd, Vec, Mem)                             // SSE2
  ASMJIT_INST_2x(comiss, Comiss, Vec, Vec)                             // SSE
  ASMJIT_INST_2x(comiss, Comiss, Vec, Mem)                             // SSE
  ASMJIT_INST_2x(cvtdq2pd, Cvtdq2pd, Vec, Vec)                         // SSE2
  ASMJIT_INST_2x(cvtdq2pd, Cvtdq2pd, Vec, Mem)                         // SSE2
  ASMJIT_INST_2x(cvtdq2ps, Cvtdq2ps, Vec, Vec)                         // SSE2
  ASMJIT_INST_2x(cvtdq2ps, Cvtdq2ps, Vec, Mem)                         // SSE2
  ASMJIT_INST_2x(cvtpd2dq, Cvtpd2dq, Vec, Vec)                         // SSE2
  ASMJIT_INST_2x(cvtpd2dq, Cvtpd2dq, Vec, Mem)                         // SSE2
  ASMJIT_INST_2x(cvtpd2pi, Cvtpd2pi, Mm, Vec)                          // SSE2
  ASMJIT_INST_2x(cvtpd2pi, Cvtpd2pi, Mm, Mem)                          // SSE2
  ASMJIT_INST_2x(cvtpd2ps, Cvtpd2ps, Vec, Vec)                         // SSE2
  ASMJIT_INST_2x(cvtpd2ps, Cvtpd2ps, Vec, Mem)                         // SSE2
  ASMJIT_INST_2x(cvtpi2pd, Cvtpi2pd, Vec, Mm)                          // SSE2
  ASMJIT_INST_2x(cvtpi2pd, Cvtpi2pd, Vec, Mem)                         // SSE2
  ASMJIT_INST_2x(cvtpi2ps, Cvtpi2ps, Vec, Mm)                          // SSE
  ASMJIT_INST_2x(cvtpi2ps, Cvtpi2ps, Vec, Mem)                         // SSE
  ASMJIT_INST_2x(cvtps2dq, Cvtps2dq, Vec, Vec)                         // SSE2
  ASMJIT_INST_2x(cvtps2dq, Cvtps2dq, Vec, Mem)                         // SSE2
  ASMJIT_INST_2x(cvtps2pd, Cvtps2pd, Vec, Vec)                         // SSE2
  ASMJIT_INST_2x(cvtps2pd, Cvtps2pd, Vec, Mem)                         // SSE2
  ASMJIT_INST_2x(cvtps2pi, Cvtps2pi, Mm, Vec)                          // SSE
  ASMJIT_INST_2x(cvtps2pi, Cvtps2pi, Mm, Mem)                          // SSE
  ASMJIT_INST_2x(cvtsd2si, Cvtsd2si, Gp, Vec)                          // SSE2
  ASMJIT_INST_2x(cvtsd2si, Cvtsd2si, Gp, Mem)                          // SSE2
  ASMJIT_INST_2x(cvtsd2ss, Cvtsd2ss, Vec, Vec)                         // SSE2
  ASMJIT_INST_2x(cvtsd2ss, Cvtsd2ss, Vec, Mem)                         // SSE2
  ASMJIT_INST_2x(cvtsi2sd, Cvtsi2sd, Vec, Gp)                          // SSE2
  ASMJIT_INST_2x(cvtsi2sd, Cvtsi2sd, Vec, Mem)                         // SSE2
  ASMJIT_INST_2x(cvtsi2ss, Cvtsi2ss, Vec, Gp)                          // SSE
  ASMJIT_INST_2x(cvtsi2ss, Cvtsi2ss, Vec, Mem)                         // SSE
  ASMJIT_INST_2x(cvtss2sd, Cvtss2sd, Vec, Vec)                         // SSE2
  ASMJIT_INST_2x(cvtss2sd, Cvtss2sd, Vec, Mem)                         // SSE2
  ASMJIT_INST_2x(cvtss2si, Cvtss2si, Gp, Vec)                          // SSE
  ASMJIT_INST_2x(cvtss2si, Cvtss2si, Gp, Mem)                          // SSE
  ASMJIT_INST_2x(cvttpd2pi, Cvttpd2pi, Mm, Vec)                        // SSE2
  ASMJIT_INST_2x(cvttpd2pi, Cvttpd2pi, Mm, Mem)                        // SSE2
  ASMJIT_INST_2x(cvttpd2dq, Cvttpd2dq, Vec, Vec)                       // SSE2
  ASMJIT_INST_2x(cvttpd2dq, Cvttpd2dq, Vec, Mem)                       // SSE2
  ASMJIT_INST_2x(cvttps2dq, Cvttps2dq, Vec, Vec)                       // SSE2
  ASMJIT_INST_2x(cvttps2dq, Cvttps2dq, Vec, Mem)                       // SSE2
  ASMJIT_INST_2x(cvttps2pi, Cvttps2pi, Mm, Vec)                        // SSE
  ASMJIT_INST_2x(cvttps2pi, Cvttps2pi, Mm, Mem)                        // SSE
  ASMJIT_INST_2x(cvttsd2si, Cvttsd2si, Gp, Vec)                        // SSE2
  ASMJIT_INST_2x(cvttsd2si, Cvttsd2si, Gp, Mem)                        // SSE2
  ASMJIT_INST_2x(cvttss2si, Cvttss2si, Gp, Vec)                        // SSE
  ASMJIT_INST_2x(cvttss2si, Cvttss2si, Gp, Mem)                        // SSE
  ASMJIT_INST_2x(divpd, Divpd, Vec, Vec)                               // SSE2
  ASMJIT_INST_2x(divpd, Divpd, Vec, Mem)                               // SSE2
  ASMJIT_INST_2x(divps, Divps, Vec, Vec)                               // SSE
  ASMJIT_INST_2x(divps, Divps, Vec, Mem)                               // SSE
  ASMJIT_INST_2x(divsd, Divsd, Vec, Vec)                               // SSE2
  ASMJIT_INST_2x(divsd, Divsd, Vec, Mem)                               // SSE2
  ASMJIT_INST_2x(divss, Divss, Vec, Vec)                               // SSE
  ASMJIT_INST_2x(divss, Divss, Vec, Mem)                               // SSE
  ASMJIT_INST_3x(dppd, Dppd, Vec, Vec, Imm)                            // SSE4_1
  ASMJIT_INST_3x(dppd, Dppd, Vec, Mem, Imm)                            // SSE4_1
  ASMJIT_INST_3x(dpps, Dpps, Vec, Vec, Imm)                            // SSE4_1
  ASMJIT_INST_3x(dpps, Dpps, Vec, Mem, Imm)                            // SSE4_1
  ASMJIT_INST_3x(extractps, Extractps, Gp, Vec, Imm)                   // SSE4_1
  ASMJIT_INST_3x(extractps, Extractps, Mem, Vec, Imm)                  // SSE4_1
  ASMJIT_INST_2x(extrq, Extrq, Vec, Vec)                               // SSE4A
  ASMJIT_INST_3x(extrq, Extrq, Vec, Imm, Imm)                          // SSE4A
  ASMJIT_INST_2x(haddpd, Haddpd, Vec, Vec)                             // SSE3
  ASMJIT_INST_2x(haddpd, Haddpd, Vec, Mem)                             // SSE3
  ASMJIT_INST_2x(haddps, Haddps, Vec, Vec)                             // SSE3
  ASMJIT_INST_2x(haddps, Haddps, Vec, Mem)                             // SSE3
  ASMJIT_INST_2x(hsubpd, Hsubpd, Vec, Vec)                             // SSE3
  ASMJIT_INST_2x(hsubpd, Hsubpd, Vec, Mem)                             // SSE3
  ASMJIT_INST_2x(hsubps, Hsubps, Vec, Vec)                             // SSE3
  ASMJIT_INST_2x(hsubps, Hsubps, Vec, Mem)                             // SSE3
  ASMJIT_INST_3x(insertps, Insertps, Vec, Vec, Imm)                    // SSE4_1
  ASMJIT_INST_3x(insertps, Insertps, Vec, Mem, Imm)                    // SSE4_1
  ASMJIT_INST_2x(insertq, Insertq, Vec, Vec)                           // SSE4A
  ASMJIT_INST_4x(insertq, Insertq, Vec, Vec, Imm, Imm)                 // SSE4A
  ASMJIT_INST_2x(lddqu, Lddqu, Vec, Mem)                               // SSE3
  ASMJIT_INST_3x(maskmovq, Maskmovq, Mm, Mm, DS_ZDI)                   // SSE  [EXPLICIT]
  ASMJIT_INST_3x(maskmovdqu, Maskmovdqu, Vec, Vec, DS_ZDI)             // SSE2 [EXPLICIT]
  ASMJIT_INST_2x(maxpd, Maxpd, Vec, Vec)                               // SSE2
  ASMJIT_INST_2x(maxpd, Maxpd, Vec, Mem)                               // SSE2
  ASMJIT_INST_2x(maxps, Maxps, Vec, Vec)                               // SSE
  ASMJIT_INST_2x(maxps, Maxps, Vec, Mem)                               // SSE
  ASMJIT_INST_2x(maxsd, Maxsd, Vec, Vec)                               // SSE2
  ASMJIT_INST_2x(maxsd, Maxsd, Vec, Mem)                               // SSE2
  ASMJIT_INST_2x(maxss, Maxss, Vec, Vec)                               // SSE
  ASMJIT_INST_2x(maxss, Maxss, Vec, Mem)                               // SSE
  ASMJIT_INST_2x(minpd, Minpd, Vec, Vec)                               // SSE2
  ASMJIT_INST_2x(minpd, Minpd, Vec, Mem)                               // SSE2
  ASMJIT_INST_2x(minps, Minps, Vec, Vec)                               // SSE
  ASMJIT_INST_2x(minps, Minps, Vec, Mem)                               // SSE
  ASMJIT_INST_2x(minsd, Minsd, Vec, Vec)                               // SSE2
  ASMJIT_INST_2x(minsd, Minsd, Vec, Mem)                               // SSE2
  ASMJIT_INST_2x(minss, Minss, Vec, Vec)                               // SSE
  ASMJIT_INST_2x(minss, Minss, Vec, Mem)                               // SSE
  ASMJIT_INST_2x(movapd, Movapd, Vec, Vec)                             // SSE2
  ASMJIT_INST_2x(movapd, Movapd, Vec, Mem)                             // SSE2
  ASMJIT_INST_2x(movapd, Movapd, Mem, Vec)                             // SSE2
  ASMJIT_INST_2x(movaps, Movaps, Vec, Vec)                             // SSE
  ASMJIT_INST_2x(movaps, Movaps, Vec, Mem)                             // SSE
  ASMJIT_INST_2x(movaps, Movaps, Mem, Vec)                             // SSE
  ASMJIT_INST_2x(movd, Movd, Mem, Mm)                                  // MMX
  ASMJIT_INST_2x(movd, Movd, Mem, Vec)                                 // SSE
  ASMJIT_INST_2x(movd, Movd, Gp, Mm)                                   // MMX
  ASMJIT_INST_2x(movd, Movd, Gp, Vec)                                  // SSE
  ASMJIT_INST_2x(movd, Movd, Mm, Mem)                                  // MMX
  ASMJIT_INST_2x(movd, Movd, Vec, Mem)                                 // SSE
  ASMJIT_INST_2x(movd, Movd, Mm, Gp)                                   // MMX
  ASMJIT_INST_2x(movd, Movd, Vec, Gp)                                  // SSE
  ASMJIT_INST_2x(movddup, Movddup, Vec, Vec)                           // SSE3
  ASMJIT_INST_2x(movddup, Movddup, Vec, Mem)                           // SSE3
  ASMJIT_INST_2x(movdq2q, Movdq2q, Mm, Vec)                            // SSE2
  ASMJIT_INST_2x(movdqa, Movdqa, Vec, Vec)                             // SSE2
  ASMJIT_INST_2x(movdqa, Movdqa, Vec, Mem)                             // SSE2
  ASMJIT_INST_2x(movdqa, Movdqa, Mem, Vec)                             // SSE2
  ASMJIT_INST_2x(movdqu, Movdqu, Vec, Vec)                             // SSE2
  ASMJIT_INST_2x(movdqu, Movdqu, Vec, Mem)                             // SSE2
  ASMJIT_INST_2x(movdqu, Movdqu, Mem, Vec)                             // SSE2
  ASMJIT_INST_2x(movhlps, Movhlps, Vec, Vec)                           // SSE
  ASMJIT_INST_2x(movhpd, Movhpd, Vec, Mem)                             // SSE2
  ASMJIT_INST_2x(movhpd, Movhpd, Mem, Vec)                             // SSE2
  ASMJIT_INST_2x(movhps, Movhps, Vec, Mem)                             // SSE
  ASMJIT_INST_2x(movhps, Movhps, Mem, Vec)                             // SSE
  ASMJIT_INST_2x(movlhps, Movlhps, Vec, Vec)                           // SSE
  ASMJIT_INST_2x(movlpd, Movlpd, Vec, Mem)                             // SSE2
  ASMJIT_INST_2x(movlpd, Movlpd, Mem, Vec)                             // SSE2
  ASMJIT_INST_2x(movlps, Movlps, Vec, Mem)                             // SSE
  ASMJIT_INST_2x(movlps, Movlps, Mem, Vec)                             // SSE
  ASMJIT_INST_2x(movmskps, Movmskps, Gp, Vec)                          // SSE2
  ASMJIT_INST_2x(movmskpd, Movmskpd, Gp, Vec)                          // SSE2
  ASMJIT_INST_2x(movntdq, Movntdq, Mem, Vec)                           // SSE2
  ASMJIT_INST_2x(movntdqa, Movntdqa, Vec, Mem)                         // SSE4_1
  ASMJIT_INST_2x(movntpd, Movntpd, Mem, Vec)                           // SSE2
  ASMJIT_INST_2x(movntps, Movntps, Mem, Vec)                           // SSE
  ASMJIT_INST_2x(movntsd, Movntsd, Mem, Vec)                           // SSE4A
  ASMJIT_INST_2x(movntss, Movntss, Mem, Vec)                           // SSE4A
  ASMJIT_INST_2x(movntq, Movntq, Mem, Mm)                              // SSE
  ASMJIT_INST_2x(movq, Movq, Mm, Mm)                                   // MMX
  ASMJIT_INST_2x(movq, Movq, Vec, Vec)                                 // SSE
  ASMJIT_INST_2x(movq, Movq, Mem, Mm)                                  // MMX
  ASMJIT_INST_2x(movq, Movq, Mem, Vec)                                 // SSE
  ASMJIT_INST_2x(movq, Movq, Mm, Mem)                                  // MMX
  ASMJIT_INST_2x(movq, Movq, Vec, Mem)                                 // SSE
  ASMJIT_INST_2x(movq, Movq, Gp, Mm)                                   // MMX
  ASMJIT_INST_2x(movq, Movq, Gp, Vec)                                  // SSE+X64.
  ASMJIT_INST_2x(movq, Movq, Mm, Gp)                                   // MMX
  ASMJIT_INST_2x(movq, Movq, Vec, Gp)                                  // SSE+X64.
  ASMJIT_INST_2x(movq2dq, Movq2dq, Vec, Mm)                            // SSE2
  ASMJIT_INST_2x(movsd, Movsd, Vec, Vec)                               // SSE2
  ASMJIT_INST_2x(movsd, Movsd, Vec, Mem)                               // SSE2
  ASMJIT_INST_2x(movsd, Movsd, Mem, Vec)                               // SSE2
  ASMJIT_INST_2x(movshdup, Movshdup, Vec, Vec)                         // SSE3
  ASMJIT_INST_2x(movshdup, Movshdup, Vec, Mem)                         // SSE3
  ASMJIT_INST_2x(movsldup, Movsldup, Vec, Vec)                         // SSE3
  ASMJIT_INST_2x(movsldup, Movsldup, Vec, Mem)                         // SSE3
  ASMJIT_INST_2x(movss, Movss, Vec, Vec)                               // SSE
  ASMJIT_INST_2x(movss, Movss, Vec, Mem)                               // SSE
  ASMJIT_INST_2x(movss, Movss, Mem, Vec)                               // SSE
  ASMJIT_INST_2x(movupd, Movupd, Vec, Vec)                             // SSE2
  ASMJIT_INST_2x(movupd, Movupd, Vec, Mem)                             // SSE2
  ASMJIT_INST_2x(movupd, Movupd, Mem, Vec)                             // SSE2
  ASMJIT_INST_2x(movups, Movups, Vec, Vec)                             // SSE
  ASMJIT_INST_2x(movups, Movups, Vec, Mem)                             // SSE
  ASMJIT_INST_2x(movups, Movups, Mem, Vec)                             // SSE
  ASMJIT_INST_3x(mpsadbw, Mpsadbw, Vec, Vec, Imm)                      // SSE4_1
  ASMJIT_INST_3x(mpsadbw, Mpsadbw, Vec, Mem, Imm)                      // SSE4_1
  ASMJIT_INST_2x(mulpd, Mulpd, Vec, Vec)                               // SSE2
  ASMJIT_INST_2x(mulpd, Mulpd, Vec, Mem)                               // SSE2
  ASMJIT_INST_2x(mulps, Mulps, Vec, Vec)                               // SSE
  ASMJIT_INST_2x(mulps, Mulps, Vec, Mem)                               // SSE
  ASMJIT_INST_2x(mulsd, Mulsd, Vec, Vec)                               // SSE2
  ASMJIT_INST_2x(mulsd, Mulsd, Vec, Mem)                               // SSE2
  ASMJIT_INST_2x(mulss, Mulss, Vec, Vec)                               // SSE
  ASMJIT_INST_2x(mulss, Mulss, Vec, Mem)                               // SSE
  ASMJIT_INST_2x(orpd, Orpd, Vec, Vec)                                 // SSE2
  ASMJIT_INST_2x(orpd, Orpd, Vec, Mem)                                 // SSE2
  ASMJIT_INST_2x(orps, Orps, Vec, Vec)                                 // SSE
  ASMJIT_INST_2x(orps, Orps, Vec, Mem)                                 // SSE
  ASMJIT_INST_2x(packssdw, Packssdw, Mm, Mm)                           // MMX
  ASMJIT_INST_2x(packssdw, Packssdw, Mm, Mem)                          // MMX
  ASMJIT_INST_2x(packssdw, Packssdw, Vec, Vec)                         // SSE2
  ASMJIT_INST_2x(packssdw, Packssdw, Vec, Mem)                         // SSE2
  ASMJIT_INST_2x(packsswb, Packsswb, Mm, Mm)                           // MMX
  ASMJIT_INST_2x(packsswb, Packsswb, Mm, Mem)                          // MMX
  ASMJIT_INST_2x(packsswb, Packsswb, Vec, Vec)                         // SSE2
  ASMJIT_INST_2x(packsswb, Packsswb, Vec, Mem)                         // SSE2
  ASMJIT_INST_2x(packusdw, Packusdw, Vec, Vec)                         // SSE4_1
  ASMJIT_INST_2x(packusdw, Packusdw, Vec, Mem)                         // SSE4_1
  ASMJIT_INST_2x(packuswb, Packuswb, Mm, Mm)                           // MMX
  ASMJIT_INST_2x(packuswb, Packuswb, Mm, Mem)                          // MMX
  ASMJIT_INST_2x(packuswb, Packuswb, Vec, Vec)                         // SSE2
  ASMJIT_INST_2x(packuswb, Packuswb, Vec, Mem)                         // SSE2
  ASMJIT_INST_2x(pabsb, Pabsb, Mm, Mm)                                 // SSSE3
  ASMJIT_INST_2x(pabsb, Pabsb, Mm, Mem)                                // SSSE3
  ASMJIT_INST_2x(pabsb, Pabsb, Vec, Vec)                               // SSSE3
  ASMJIT_INST_2x(pabsb, Pabsb, Vec, Mem)                               // SSSE3
  ASMJIT_INST_2x(pabsd, Pabsd, Mm, Mm)                                 // SSSE3
  ASMJIT_INST_2x(pabsd, Pabsd, Mm, Mem)                                // SSSE3
  ASMJIT_INST_2x(pabsd, Pabsd, Vec, Vec)                               // SSSE3
  ASMJIT_INST_2x(pabsd, Pabsd, Vec, Mem)                               // SSSE3
  ASMJIT_INST_2x(pabsw, Pabsw, Mm, Mm)                                 // SSSE3
  ASMJIT_INST_2x(pabsw, Pabsw, Mm, Mem)                                // SSSE3
  ASMJIT_INST_2x(pabsw, Pabsw, Vec, Vec)                               // SSSE3
  ASMJIT_INST_2x(pabsw, Pabsw, Vec, Mem)                               // SSSE3
  ASMJIT_INST_2x(paddb, Paddb, Mm, Mm)                                 // MMX
  ASMJIT_INST_2x(paddb, Paddb, Mm, Mem)                                // MMX
  ASMJIT_INST_2x(paddb, Paddb, Vec, Vec)                               // SSE2
  ASMJIT_INST_2x(paddb, Paddb, Vec, Mem)                               // SSE2
  ASMJIT_INST_2x(paddd, Paddd, Mm, Mm)                                 // MMX
  ASMJIT_INST_2x(paddd, Paddd, Mm, Mem)                                // MMX
  ASMJIT_INST_2x(paddd, Paddd, Vec, Vec)                               // SSE2
  ASMJIT_INST_2x(paddd, Paddd, Vec, Mem)                               // SSE2
  ASMJIT_INST_2x(paddq, Paddq, Mm, Mm)                                 // SSE2
  ASMJIT_INST_2x(paddq, Paddq, Mm, Mem)                                // SSE2
  ASMJIT_INST_2x(paddq, Paddq, Vec, Vec)                               // SSE2
  ASMJIT_INST_2x(paddq, Paddq, Vec, Mem)                               // SSE2
  ASMJIT_INST_2x(paddsb, Paddsb, Mm, Mm)                               // MMX
  ASMJIT_INST_2x(paddsb, Paddsb, Mm, Mem)                              // MMX
  ASMJIT_INST_2x(paddsb, Paddsb, Vec, Vec)                             // SSE2
  ASMJIT_INST_2x(paddsb, Paddsb, Vec, Mem)                             // SSE2
  ASMJIT_INST_2x(paddsw, Paddsw, Mm, Mm)                               // MMX
  ASMJIT_INST_2x(paddsw, Paddsw, Mm, Mem)                              // MMX
  ASMJIT_INST_2x(paddsw, Paddsw, Vec, Vec)                             // SSE2
  ASMJIT_INST_2x(paddsw, Paddsw, Vec, Mem)                             // SSE2
  ASMJIT_INST_2x(paddusb, Paddusb, Mm, Mm)                             // MMX
  ASMJIT_INST_2x(paddusb, Paddusb, Mm, Mem)                            // MMX
  ASMJIT_INST_2x(paddusb, Paddusb, Vec, Vec)                           // SSE2
  ASMJIT_INST_2x(paddusb, Paddusb, Vec, Mem)                           // SSE2
  ASMJIT_INST_2x(paddusw, Paddusw, Mm, Mm)                             // MMX
  ASMJIT_INST_2x(paddusw, Paddusw, Mm, Mem)                            // MMX
  ASMJIT_INST_2x(paddusw, Paddusw, Vec, Vec)                           // SSE2
  ASMJIT_INST_2x(paddusw, Paddusw, Vec, Mem)                           // SSE2
  ASMJIT_INST_2x(paddw, Paddw, Mm, Mm)                                 // MMX
  ASMJIT_INST_2x(paddw, Paddw, Mm, Mem)                                // MMX
  ASMJIT_INST_2x(paddw, Paddw, Vec, Vec)                               // SSE2
  ASMJIT_INST_2x(paddw, Paddw, Vec, Mem)                               // SSE2
  ASMJIT_INST_3x(palignr, Palignr, Mm, Mm, Imm)                        // SSSE3
  ASMJIT_INST_3x(palignr, Palignr, Mm, Mem, Imm)                       // SSSE3
  ASMJIT_INST_3x(palignr, Palignr, Vec, Vec, Imm)                      // SSSE3
  ASMJIT_INST_3x(palignr, Palignr, Vec, Mem, Imm)                      // SSSE3
  ASMJIT_INST_2x(pand, Pand, Mm, Mm)                                   // MMX
  ASMJIT_INST_2x(pand, Pand, Mm, Mem)                                  // MMX
  ASMJIT_INST_2x(pand, Pand, Vec, Vec)                                 // SSE2
  ASMJIT_INST_2x(pand, Pand, Vec, Mem)                                 // SSE2
  ASMJIT_INST_2x(pandn, Pandn, Mm, Mm)                                 // MMX
  ASMJIT_INST_2x(pandn, Pandn, Mm, Mem)                                // MMX
  ASMJIT_INST_2x(pandn, Pandn, Vec, Vec)                               // SSE2
  ASMJIT_INST_2x(pandn, Pandn, Vec, Mem)                               // SSE2
  ASMJIT_INST_2x(pavgb, Pavgb, Mm, Mm)                                 // SSE
  ASMJIT_INST_2x(pavgb, Pavgb, Mm, Mem)                                // SSE
  ASMJIT_INST_2x(pavgb, Pavgb, Vec, Vec)                               // SSE2
  ASMJIT_INST_2x(pavgb, Pavgb, Vec, Mem)                               // SSE2
  ASMJIT_INST_2x(pavgw, Pavgw, Mm, Mm)                                 // SSE
  ASMJIT_INST_2x(pavgw, Pavgw, Mm, Mem)                                // SSE
  ASMJIT_INST_2x(pavgw, Pavgw, Vec, Vec)                               // SSE2
  ASMJIT_INST_2x(pavgw, Pavgw, Vec, Mem)                               // SSE2
  ASMJIT_INST_3x(pblendvb, Pblendvb, Vec, Vec, XMM0)                   // SSE4_1 [EXPLICIT]
  ASMJIT_INST_3x(pblendvb, Pblendvb, Vec, Mem, XMM0)                   // SSE4_1 [EXPLICIT]
  ASMJIT_INST_3x(pblendw, Pblendw, Vec, Vec, Imm)                      // SSE4_1
  ASMJIT_INST_3x(pblendw, Pblendw, Vec, Mem, Imm)                      // SSE4_1
  ASMJIT_INST_3x(pclmulqdq, Pclmulqdq, Vec, Vec, Imm)                  // PCLMULQDQ.
  ASMJIT_INST_3x(pclmulqdq, Pclmulqdq, Vec, Mem, Imm)                  // PCLMULQDQ.
  ASMJIT_INST_6x(pcmpestri, Pcmpestri, Vec, Vec, Imm, Gp_ECX, Gp_EAX, Gp_EDX) // SSE4_2 [EXPLICIT]
  ASMJIT_INST_6x(pcmpestri, Pcmpestri, Vec, Mem, Imm, Gp_ECX, Gp_EAX, Gp_EDX) // SSE4_2 [EXPLICIT]
  ASMJIT_INST_6x(pcmpestrm, Pcmpestrm, Vec, Vec, Imm, XMM0, Gp_EAX, Gp_EDX)   // SSE4_2 [EXPLICIT]
  ASMJIT_INST_6x(pcmpestrm, Pcmpestrm, Vec, Mem, Imm, XMM0, Gp_EAX, Gp_EDX)   // SSE4_2 [EXPLICIT]
  ASMJIT_INST_2x(pcmpeqb, Pcmpeqb, Mm, Mm)                             // MMX
  ASMJIT_INST_2x(pcmpeqb, Pcmpeqb, Mm, Mem)                            // MMX
  ASMJIT_INST_2x(pcmpeqb, Pcmpeqb, Vec, Vec)                           // SSE2
  ASMJIT_INST_2x(pcmpeqb, Pcmpeqb, Vec, Mem)                           // SSE2
  ASMJIT_INST_2x(pcmpeqd, Pcmpeqd, Mm, Mm)                             // MMX
  ASMJIT_INST_2x(pcmpeqd, Pcmpeqd, Mm, Mem)                            // MMX
  ASMJIT_INST_2x(pcmpeqd, Pcmpeqd, Vec, Vec)                           // SSE2
  ASMJIT_INST_2x(pcmpeqd, Pcmpeqd, Vec, Mem)                           // SSE2
  ASMJIT_INST_2x(pcmpeqq, Pcmpeqq, Vec, Vec)                           // SSE4_1
  ASMJIT_INST_2x(pcmpeqq, Pcmpeqq, Vec, Mem)                           // SSE4_1
  ASMJIT_INST_2x(pcmpeqw, Pcmpeqw, Mm, Mm)                             // MMX
  ASMJIT_INST_2x(pcmpeqw, Pcmpeqw, Mm, Mem)                            // MMX
  ASMJIT_INST_2x(pcmpeqw, Pcmpeqw, Vec, Vec)                           // SSE2
  ASMJIT_INST_2x(pcmpeqw, Pcmpeqw, Vec, Mem)                           // SSE2
  ASMJIT_INST_2x(pcmpgtb, Pcmpgtb, Mm, Mm)                             // MMX
  ASMJIT_INST_2x(pcmpgtb, Pcmpgtb, Mm, Mem)                            // MMX
  ASMJIT_INST_2x(pcmpgtb, Pcmpgtb, Vec, Vec)                           // SSE2
  ASMJIT_INST_2x(pcmpgtb, Pcmpgtb, Vec, Mem)                           // SSE2
  ASMJIT_INST_2x(pcmpgtd, Pcmpgtd, Mm, Mm)                             // MMX
  ASMJIT_INST_2x(pcmpgtd, Pcmpgtd, Mm, Mem)                            // MMX
  ASMJIT_INST_2x(pcmpgtd, Pcmpgtd, Vec, Vec)                           // SSE2
  ASMJIT_INST_2x(pcmpgtd, Pcmpgtd, Vec, Mem)                           // SSE2
  ASMJIT_INST_2x(pcmpgtq, Pcmpgtq, Vec, Vec)                           // SSE4_2.
  ASMJIT_INST_2x(pcmpgtq, Pcmpgtq, Vec, Mem)                           // SSE4_2.
  ASMJIT_INST_2x(pcmpgtw, Pcmpgtw, Mm, Mm)                             // MMX
  ASMJIT_INST_2x(pcmpgtw, Pcmpgtw, Mm, Mem)                            // MMX
  ASMJIT_INST_2x(pcmpgtw, Pcmpgtw, Vec, Vec)                           // SSE2
  ASMJIT_INST_2x(pcmpgtw, Pcmpgtw, Vec, Mem)                           // SSE2
  ASMJIT_INST_4x(pcmpistri, Pcmpistri, Vec, Vec, Imm, Gp_ECX)          // SSE4_2 [EXPLICIT]
  ASMJIT_INST_4x(pcmpistri, Pcmpistri, Vec, Mem, Imm, Gp_ECX)          // SSE4_2 [EXPLICIT]
  ASMJIT_INST_4x(pcmpistrm, Pcmpistrm, Vec, Vec, Imm, XMM0)            // SSE4_2 [EXPLICIT]
  ASMJIT_INST_4x(pcmpistrm, Pcmpistrm, Vec, Mem, Imm, XMM0)            // SSE4_2 [EXPLICIT]
  ASMJIT_INST_3x(pextrb, Pextrb, Gp, Vec, Imm)                         // SSE4_1
  ASMJIT_INST_3x(pextrb, Pextrb, Mem, Vec, Imm)                        // SSE4_1
  ASMJIT_INST_3x(pextrd, Pextrd, Gp, Vec, Imm)                         // SSE4_1
  ASMJIT_INST_3x(pextrd, Pextrd, Mem, Vec, Imm)                        // SSE4_1
  ASMJIT_INST_3x(pextrq, Pextrq, Gp, Vec, Imm)                         // SSE4_1
  ASMJIT_INST_3x(pextrq, Pextrq, Mem, Vec, Imm)                        // SSE4_1
  ASMJIT_INST_3x(pextrw, Pextrw, Gp, Mm, Imm)                          // SSE
  ASMJIT_INST_3x(pextrw, Pextrw, Gp, Vec, Imm)                         // SSE2
  ASMJIT_INST_3x(pextrw, Pextrw, Mem, Vec, Imm)                        // SSE4_1
  ASMJIT_INST_2x(phaddd, Phaddd, Mm, Mm)                               // SSSE3
  ASMJIT_INST_2x(phaddd, Phaddd, Mm, Mem)                              // SSSE3
  ASMJIT_INST_2x(phaddd, Phaddd, Vec, Vec)                             // SSSE3
  ASMJIT_INST_2x(phaddd, Phaddd, Vec, Mem)                             // SSSE3
  ASMJIT_INST_2x(phaddsw, Phaddsw, Mm, Mm)                             // SSSE3
  ASMJIT_INST_2x(phaddsw, Phaddsw, Mm, Mem)                            // SSSE3
  ASMJIT_INST_2x(phaddsw, Phaddsw, Vec, Vec)                           // SSSE3
  ASMJIT_INST_2x(phaddsw, Phaddsw, Vec, Mem)                           // SSSE3
  ASMJIT_INST_2x(phaddw, Phaddw, Mm, Mm)                               // SSSE3
  ASMJIT_INST_2x(phaddw, Phaddw, Mm, Mem)                              // SSSE3
  ASMJIT_INST_2x(phaddw, Phaddw, Vec, Vec)                             // SSSE3
  ASMJIT_INST_2x(phaddw, Phaddw, Vec, Mem)                             // SSSE3
  ASMJIT_INST_2x(phminposuw, Phminposuw, Vec, Vec)                     // SSE4_1
  ASMJIT_INST_2x(phminposuw, Phminposuw, Vec, Mem)                     // SSE4_1
  ASMJIT_INST_2x(phsubd, Phsubd, Mm, Mm)                               // SSSE3
  ASMJIT_INST_2x(phsubd, Phsubd, Mm, Mem)                              // SSSE3
  ASMJIT_INST_2x(phsubd, Phsubd, Vec, Vec)                             // SSSE3
  ASMJIT_INST_2x(phsubd, Phsubd, Vec, Mem)                             // SSSE3
  ASMJIT_INST_2x(phsubsw, Phsubsw, Mm, Mm)                             // SSSE3
  ASMJIT_INST_2x(phsubsw, Phsubsw, Mm, Mem)                            // SSSE3
  ASMJIT_INST_2x(phsubsw, Phsubsw, Vec, Vec)                           // SSSE3
  ASMJIT_INST_2x(phsubsw, Phsubsw, Vec, Mem)                           // SSSE3
  ASMJIT_INST_2x(phsubw, Phsubw, Mm, Mm)                               // SSSE3
  ASMJIT_INST_2x(phsubw, Phsubw, Mm, Mem)                              // SSSE3
  ASMJIT_INST_2x(phsubw, Phsubw, Vec, Vec)                             // SSSE3
  ASMJIT_INST_2x(phsubw, Phsubw, Vec, Mem)                             // SSSE3
  ASMJIT_INST_3x(pinsrb, Pinsrb, Vec, Gp, Imm)                         // SSE4_1
  ASMJIT_INST_3x(pinsrb, Pinsrb, Vec, Mem, Imm)                        // SSE4_1
  ASMJIT_INST_3x(pinsrd, Pinsrd, Vec, Gp, Imm)                         // SSE4_1
  ASMJIT_INST_3x(pinsrd, Pinsrd, Vec, Mem, Imm)                        // SSE4_1
  ASMJIT_INST_3x(pinsrq, Pinsrq, Vec, Gp, Imm)                         // SSE4_1
  ASMJIT_INST_3x(pinsrq, Pinsrq, Vec, Mem, Imm)                        // SSE4_1
  ASMJIT_INST_3x(pinsrw, Pinsrw, Mm, Gp, Imm)                          // SSE
  ASMJIT_INST_3x(pinsrw, Pinsrw, Mm, Mem, Imm)                         // SSE
  ASMJIT_INST_3x(pinsrw, Pinsrw, Vec, Gp, Imm)                         // SSE2
  ASMJIT_INST_3x(pinsrw, Pinsrw, Vec, Mem, Imm)                        // SSE2
  ASMJIT_INST_2x(pmaddubsw, Pmaddubsw, Mm, Mm)                         // SSSE3
  ASMJIT_INST_2x(pmaddubsw, Pmaddubsw, Mm, Mem)                        // SSSE3
  ASMJIT_INST_2x(pmaddubsw, Pmaddubsw, Vec, Vec)                       // SSSE3
  ASMJIT_INST_2x(pmaddubsw, Pmaddubsw, Vec, Mem)                       // SSSE3
  ASMJIT_INST_2x(pmaddwd, Pmaddwd, Mm, Mm)                             // MMX
  ASMJIT_INST_2x(pmaddwd, Pmaddwd, Mm, Mem)                            // MMX
  ASMJIT_INST_2x(pmaddwd, Pmaddwd, Vec, Vec)                           // SSE2
  ASMJIT_INST_2x(pmaddwd, Pmaddwd, Vec, Mem)                           // SSE2
  ASMJIT_INST_2x(pmaxsb, Pmaxsb, Vec, Vec)                             // SSE4_1
  ASMJIT_INST_2x(pmaxsb, Pmaxsb, Vec, Mem)                             // SSE4_1
  ASMJIT_INST_2x(pmaxsd, Pmaxsd, Vec, Vec)                             // SSE4_1
  ASMJIT_INST_2x(pmaxsd, Pmaxsd, Vec, Mem)                             // SSE4_1
  ASMJIT_INST_2x(pmaxsw, Pmaxsw, Mm, Mm)                               // SSE
  ASMJIT_INST_2x(pmaxsw, Pmaxsw, Mm, Mem)                              // SSE
  ASMJIT_INST_2x(pmaxsw, Pmaxsw, Vec, Vec)                             // SSE2
  ASMJIT_INST_2x(pmaxsw, Pmaxsw, Vec, Mem)                             // SSE2
  ASMJIT_INST_2x(pmaxub, Pmaxub, Mm, Mm)                               // SSE
  ASMJIT_INST_2x(pmaxub, Pmaxub, Mm, Mem)                              // SSE
  ASMJIT_INST_2x(pmaxub, Pmaxub, Vec, Vec)                             // SSE2
  ASMJIT_INST_2x(pmaxub, Pmaxub, Vec, Mem)                             // SSE2
  ASMJIT_INST_2x(pmaxud, Pmaxud, Vec, Vec)                             // SSE4_1
  ASMJIT_INST_2x(pmaxud, Pmaxud, Vec, Mem)                             // SSE4_1
  ASMJIT_INST_2x(pmaxuw, Pmaxuw, Vec, Vec)                             // SSE4_1
  ASMJIT_INST_2x(pmaxuw, Pmaxuw, Vec, Mem)                             // SSE4_1
  ASMJIT_INST_2x(pminsb, Pminsb, Vec, Vec)                             // SSE4_1
  ASMJIT_INST_2x(pminsb, Pminsb, Vec, Mem)                             // SSE4_1
  ASMJIT_INST_2x(pminsd, Pminsd, Vec, Vec)                             // SSE4_1
  ASMJIT_INST_2x(pminsd, Pminsd, Vec, Mem)                             // SSE4_1
  ASMJIT_INST_2x(pminsw, Pminsw, Mm, Mm)                               // SSE
  ASMJIT_INST_2x(pminsw, Pminsw, Mm, Mem)                              // SSE
  ASMJIT_INST_2x(pminsw, Pminsw, Vec, Vec)                             // SSE2
  ASMJIT_INST_2x(pminsw, Pminsw, Vec, Mem)                             // SSE2
  ASMJIT_INST_2x(pminub, Pminub, Mm, Mm)                               // SSE
  ASMJIT_INST_2x(pminub, Pminub, Mm, Mem)                              // SSE
  ASMJIT_INST_2x(pminub, Pminub, Vec, Vec)                             // SSE2
  ASMJIT_INST_2x(pminub, Pminub, Vec, Mem)                             // SSE2
  ASMJIT_INST_2x(pminud, Pminud, Vec, Vec)                             // SSE4_1
  ASMJIT_INST_2x(pminud, Pminud, Vec, Mem)                             // SSE4_1
  ASMJIT_INST_2x(pminuw, Pminuw, Vec, Vec)                             // SSE4_1
  ASMJIT_INST_2x(pminuw, Pminuw, Vec, Mem)                             // SSE4_1
  ASMJIT_INST_2x(pmovmskb, Pmovmskb, Gp, Mm)                           // SSE
  ASMJIT_INST_2x(pmovmskb, Pmovmskb, Gp, Vec)                          // SSE2
  ASMJIT_INST_2x(pmovsxbd, Pmovsxbd, Vec, Vec)                         // SSE4_1
  ASMJIT_INST_2x(pmovsxbd, Pmovsxbd, Vec, Mem)                         // SSE4_1
  ASMJIT_INST_2x(pmovsxbq, Pmovsxbq, Vec, Vec)                         // SSE4_1
  ASMJIT_INST_2x(pmovsxbq, Pmovsxbq, Vec, Mem)                         // SSE4_1
  ASMJIT_INST_2x(pmovsxbw, Pmovsxbw, Vec, Vec)                         // SSE4_1
  ASMJIT_INST_2x(pmovsxbw, Pmovsxbw, Vec, Mem)                         // SSE4_1
  ASMJIT_INST_2x(pmovsxdq, Pmovsxdq, Vec, Vec)                         // SSE4_1
  ASMJIT_INST_2x(pmovsxdq, Pmovsxdq, Vec, Mem)                         // SSE4_1
  ASMJIT_INST_2x(pmovsxwd, Pmovsxwd, Vec, Vec)                         // SSE4_1
  ASMJIT_INST_2x(pmovsxwd, Pmovsxwd, Vec, Mem)                         // SSE4_1
  ASMJIT_INST_2x(pmovsxwq, Pmovsxwq, Vec, Vec)                         // SSE4_1
  ASMJIT_INST_2x(pmovsxwq, Pmovsxwq, Vec, Mem)                         // SSE4_1
  ASMJIT_INST_2x(pmovzxbd, Pmovzxbd, Vec, Vec)                         // SSE4_1
  ASMJIT_INST_2x(pmovzxbd, Pmovzxbd, Vec, Mem)                         // SSE4_1
  ASMJIT_INST_2x(pmovzxbq, Pmovzxbq, Vec, Vec)                         // SSE4_1
  ASMJIT_INST_2x(pmovzxbq, Pmovzxbq, Vec, Mem)                         // SSE4_1
  ASMJIT_INST_2x(pmovzxbw, Pmovzxbw, Vec, Vec)                         // SSE4_1
  ASMJIT_INST_2x(pmovzxbw, Pmovzxbw, Vec, Mem)                         // SSE4_1
  ASMJIT_INST_2x(pmovzxdq, Pmovzxdq, Vec, Vec)                         // SSE4_1
  ASMJIT_INST_2x(pmovzxdq, Pmovzxdq, Vec, Mem)                         // SSE4_1
  ASMJIT_INST_2x(pmovzxwd, Pmovzxwd, Vec, Vec)                         // SSE4_1
  ASMJIT_INST_2x(pmovzxwd, Pmovzxwd, Vec, Mem)                         // SSE4_1
  ASMJIT_INST_2x(pmovzxwq, Pmovzxwq, Vec, Vec)                         // SSE4_1
  ASMJIT_INST_2x(pmovzxwq, Pmovzxwq, Vec, Mem)                         // SSE4_1
  ASMJIT_INST_2x(pmuldq, Pmuldq, Vec, Vec)                             // SSE4_1
  ASMJIT_INST_2x(pmuldq, Pmuldq, Vec, Mem)                             // SSE4_1
  ASMJIT_INST_2x(pmulhrsw, Pmulhrsw, Mm, Mm)                           // SSSE3
  ASMJIT_INST_2x(pmulhrsw, Pmulhrsw, Mm, Mem)                          // SSSE3
  ASMJIT_INST_2x(pmulhrsw, Pmulhrsw, Vec, Vec)                         // SSSE3
  ASMJIT_INST_2x(pmulhrsw, Pmulhrsw, Vec, Mem)                         // SSSE3
  ASMJIT_INST_2x(pmulhw, Pmulhw, Mm, Mm)                               // MMX
  ASMJIT_INST_2x(pmulhw, Pmulhw, Mm, Mem)                              // MMX
  ASMJIT_INST_2x(pmulhw, Pmulhw, Vec, Vec)                             // SSE2
  ASMJIT_INST_2x(pmulhw, Pmulhw, Vec, Mem)                             // SSE2
  ASMJIT_INST_2x(pmulhuw, Pmulhuw, Mm, Mm)                             // SSE
  ASMJIT_INST_2x(pmulhuw, Pmulhuw, Mm, Mem)                            // SSE
  ASMJIT_INST_2x(pmulhuw, Pmulhuw, Vec, Vec)                           // SSE2
  ASMJIT_INST_2x(pmulhuw, Pmulhuw, Vec, Mem)                           // SSE2
  ASMJIT_INST_2x(pmulld, Pmulld, Vec, Vec)                             // SSE4_1
  ASMJIT_INST_2x(pmulld, Pmulld, Vec, Mem)                             // SSE4_1
  ASMJIT_INST_2x(pmullw, Pmullw, Mm, Mm)                               // MMX
  ASMJIT_INST_2x(pmullw, Pmullw, Mm, Mem)                              // MMX
  ASMJIT_INST_2x(pmullw, Pmullw, Vec, Vec)                             // SSE2
  ASMJIT_INST_2x(pmullw, Pmullw, Vec, Mem)                             // SSE2
  ASMJIT_INST_2x(pmuludq, Pmuludq, Mm, Mm)                             // SSE2
  ASMJIT_INST_2x(pmuludq, Pmuludq, Mm, Mem)                            // SSE2
  ASMJIT_INST_2x(pmuludq, Pmuludq, Vec, Vec)                           // SSE2
  ASMJIT_INST_2x(pmuludq, Pmuludq, Vec, Mem)                           // SSE2
  ASMJIT_INST_2x(por, Por, Mm, Mm)                                     // MMX
  ASMJIT_INST_2x(por, Por, Mm, Mem)                                    // MMX
  ASMJIT_INST_2x(por, Por, Vec, Vec)                                   // SSE2
  ASMJIT_INST_2x(por, Por, Vec, Mem)                                   // SSE2
  ASMJIT_INST_2x(psadbw, Psadbw, Mm, Mm)                               // SSE
  ASMJIT_INST_2x(psadbw, Psadbw, Mm, Mem)                              // SSE
  ASMJIT_INST_2x(psadbw, Psadbw, Vec, Vec)                             // SSE
  ASMJIT_INST_2x(psadbw, Psadbw, Vec, Mem)                             // SSE
  ASMJIT_INST_2x(pslld, Pslld, Mm, Mm)                                 // MMX
  ASMJIT_INST_2x(pslld, Pslld, Mm, Mem)                                // MMX
  ASMJIT_INST_2x(pslld, Pslld, Mm, Imm)                                // MMX
  ASMJIT_INST_2x(pslld, Pslld, Vec, Vec)                               // SSE2
  ASMJIT_INST_2x(pslld, Pslld, Vec, Mem)                               // SSE2
  ASMJIT_INST_2x(pslld, Pslld, Vec, Imm)                               // SSE2
  ASMJIT_INST_2x(pslldq, Pslldq, Vec, Imm)                             // SSE2
  ASMJIT_INST_2x(psllq, Psllq, Mm, Mm)                                 // MMX
  ASMJIT_INST_2x(psllq, Psllq, Mm, Mem)                                // MMX
  ASMJIT_INST_2x(psllq, Psllq, Mm, Imm)                                // MMX
  ASMJIT_INST_2x(psllq, Psllq, Vec, Vec)                               // SSE2
  ASMJIT_INST_2x(psllq, Psllq, Vec, Mem)                               // SSE2
  ASMJIT_INST_2x(psllq, Psllq, Vec, Imm)                               // SSE2
  ASMJIT_INST_2x(psllw, Psllw, Mm, Mm)                                 // MMX
  ASMJIT_INST_2x(psllw, Psllw, Mm, Mem)                                // MMX
  ASMJIT_INST_2x(psllw, Psllw, Mm, Imm)                                // MMX
  ASMJIT_INST_2x(psllw, Psllw, Vec, Vec)                               // SSE2
  ASMJIT_INST_2x(psllw, Psllw, Vec, Mem)                               // SSE2
  ASMJIT_INST_2x(psllw, Psllw, Vec, Imm)                               // SSE2
  ASMJIT_INST_2x(psrad, Psrad, Mm, Mm)                                 // MMX
  ASMJIT_INST_2x(psrad, Psrad, Mm, Mem)                                // MMX
  ASMJIT_INST_2x(psrad, Psrad, Mm, Imm)                                // MMX
  ASMJIT_INST_2x(psrad, Psrad, Vec, Vec)                               // SSE2
  ASMJIT_INST_2x(psrad, Psrad, Vec, Mem)                               // SSE2
  ASMJIT_INST_2x(psrad, Psrad, Vec, Imm)                               // SSE2
  ASMJIT_INST_2x(psraw, Psraw, Mm, Mm)                                 // MMX
  ASMJIT_INST_2x(psraw, Psraw, Mm, Mem)                                // MMX
  ASMJIT_INST_2x(psraw, Psraw, Mm, Imm)                                // MMX
  ASMJIT_INST_2x(psraw, Psraw, Vec, Vec)                               // SSE2
  ASMJIT_INST_2x(psraw, Psraw, Vec, Mem)                               // SSE2
  ASMJIT_INST_2x(psraw, Psraw, Vec, Imm)                               // SSE2
  ASMJIT_INST_2x(pshufb, Pshufb, Mm, Mm)                               // SSSE3
  ASMJIT_INST_2x(pshufb, Pshufb, Mm, Mem)                              // SSSE3
  ASMJIT_INST_2x(pshufb, Pshufb, Vec, Vec)                             // SSSE3
  ASMJIT_INST_2x(pshufb, Pshufb, Vec, Mem)                             // SSSE3
  ASMJIT_INST_3x(pshufd, Pshufd, Vec, Vec, Imm)                        // SSE2
  ASMJIT_INST_3x(pshufd, Pshufd, Vec, Mem, Imm)                        // SSE2
  ASMJIT_INST_3x(pshufhw, Pshufhw, Vec, Vec, Imm)                      // SSE2
  ASMJIT_INST_3x(pshufhw, Pshufhw, Vec, Mem, Imm)                      // SSE2
  ASMJIT_INST_3x(pshuflw, Pshuflw, Vec, Vec, Imm)                      // SSE2
  ASMJIT_INST_3x(pshuflw, Pshuflw, Vec, Mem, Imm)                      // SSE2
  ASMJIT_INST_3x(pshufw, Pshufw, Mm, Mm, Imm)                          // SSE
  ASMJIT_INST_3x(pshufw, Pshufw, Mm, Mem, Imm)                         // SSE
  ASMJIT_INST_2x(psignb, Psignb, Mm, Mm)                               // SSSE3
  ASMJIT_INST_2x(psignb, Psignb, Mm, Mem)                              // SSSE3
  ASMJIT_INST_2x(psignb, Psignb, Vec, Vec)                             // SSSE3
  ASMJIT_INST_2x(psignb, Psignb, Vec, Mem)                             // SSSE3
  ASMJIT_INST_2x(psignd, Psignd, Mm, Mm)                               // SSSE3
  ASMJIT_INST_2x(psignd, Psignd, Mm, Mem)                              // SSSE3
  ASMJIT_INST_2x(psignd, Psignd, Vec, Vec)                             // SSSE3
  ASMJIT_INST_2x(psignd, Psignd, Vec, Mem)                             // SSSE3
  ASMJIT_INST_2x(psignw, Psignw, Mm, Mm)                               // SSSE3
  ASMJIT_INST_2x(psignw, Psignw, Mm, Mem)                              // SSSE3
  ASMJIT_INST_2x(psignw, Psignw, Vec, Vec)                             // SSSE3
  ASMJIT_INST_2x(psignw, Psignw, Vec, Mem)                             // SSSE3
  ASMJIT_INST_2x(psrld, Psrld, Mm, Mm)                                 // MMX
  ASMJIT_INST_2x(psrld, Psrld, Mm, Mem)                                // MMX
  ASMJIT_INST_2x(psrld, Psrld, Mm, Imm)                                // MMX
  ASMJIT_INST_2x(psrld, Psrld, Vec, Vec)                               // SSE2
  ASMJIT_INST_2x(psrld, Psrld, Vec, Mem)                               // SSE2
  ASMJIT_INST_2x(psrld, Psrld, Vec, Imm)                               // SSE2
  ASMJIT_INST_2x(psrldq, Psrldq, Vec, Imm)                             // SSE2
  ASMJIT_INST_2x(psrlq, Psrlq, Mm, Mm)                                 // MMX
  ASMJIT_INST_2x(psrlq, Psrlq, Mm, Mem)                                // MMX
  ASMJIT_INST_2x(psrlq, Psrlq, Mm, Imm)                                // MMX
  ASMJIT_INST_2x(psrlq, Psrlq, Vec, Vec)                               // SSE2
  ASMJIT_INST_2x(psrlq, Psrlq, Vec, Mem)                               // SSE2
  ASMJIT_INST_2x(psrlq, Psrlq, Vec, Imm)                               // SSE2
  ASMJIT_INST_2x(psrlw, Psrlw, Mm, Mm)                                 // MMX
  ASMJIT_INST_2x(psrlw, Psrlw, Mm, Mem)                                // MMX
  ASMJIT_INST_2x(psrlw, Psrlw, Mm, Imm)                                // MMX
  ASMJIT_INST_2x(psrlw, Psrlw, Vec, Vec)                               // SSE2
  ASMJIT_INST_2x(psrlw, Psrlw, Vec, Mem)                               // SSE2
  ASMJIT_INST_2x(psrlw, Psrlw, Vec, Imm)                               // SSE2
  ASMJIT_INST_2x(psubb, Psubb, Mm, Mm)                                 // MMX
  ASMJIT_INST_2x(psubb, Psubb, Mm, Mem)                                // MMX
  ASMJIT_INST_2x(psubb, Psubb, Vec, Vec)                               // SSE2
  ASMJIT_INST_2x(psubb, Psubb, Vec, Mem)                               // SSE2
  ASMJIT_INST_2x(psubd, Psubd, Mm, Mm)                                 // MMX
  ASMJIT_INST_2x(psubd, Psubd, Mm, Mem)                                // MMX
  ASMJIT_INST_2x(psubd, Psubd, Vec, Vec)                               // SSE2
  ASMJIT_INST_2x(psubd, Psubd, Vec, Mem)                               // SSE2
  ASMJIT_INST_2x(psubq, Psubq, Mm, Mm)                                 // SSE2
  ASMJIT_INST_2x(psubq, Psubq, Mm, Mem)                                // SSE2
  ASMJIT_INST_2x(psubq, Psubq, Vec, Vec)                               // SSE2
  ASMJIT_INST_2x(psubq, Psubq, Vec, Mem)                               // SSE2
  ASMJIT_INST_2x(psubsb, Psubsb, Mm, Mm)                               // MMX
  ASMJIT_INST_2x(psubsb, Psubsb, Mm, Mem)                              // MMX
  ASMJIT_INST_2x(psubsb, Psubsb, Vec, Vec)                             // SSE2
  ASMJIT_INST_2x(psubsb, Psubsb, Vec, Mem)                             // SSE2
  ASMJIT_INST_2x(psubsw, Psubsw, Mm, Mm)                               // MMX
  ASMJIT_INST_2x(psubsw, Psubsw, Mm, Mem)                              // MMX
  ASMJIT_INST_2x(psubsw, Psubsw, Vec, Vec)                             // SSE2
  ASMJIT_INST_2x(psubsw, Psubsw, Vec, Mem)                             // SSE2
  ASMJIT_INST_2x(psubusb, Psubusb, Mm, Mm)                             // MMX
  ASMJIT_INST_2x(psubusb, Psubusb, Mm, Mem)                            // MMX
  ASMJIT_INST_2x(psubusb, Psubusb, Vec, Vec)                           // SSE2
  ASMJIT_INST_2x(psubusb, Psubusb, Vec, Mem)                           // SSE2
  ASMJIT_INST_2x(psubusw, Psubusw, Mm, Mm)                             // MMX
  ASMJIT_INST_2x(psubusw, Psubusw, Mm, Mem)                            // MMX
  ASMJIT_INST_2x(psubusw, Psubusw, Vec, Vec)                           // SSE2
  ASMJIT_INST_2x(psubusw, Psubusw, Vec, Mem)                           // SSE2
  ASMJIT_INST_2x(psubw, Psubw, Mm, Mm)                                 // MMX
  ASMJIT_INST_2x(psubw, Psubw, Mm, Mem)                                // MMX
  ASMJIT_INST_2x(psubw, Psubw, Vec, Vec)                               // SSE2
  ASMJIT_INST_2x(psubw, Psubw, Vec, Mem)                               // SSE2
  ASMJIT_INST_2x(ptest, Ptest, Vec, Vec)                               // SSE4_1
  ASMJIT_INST_2x(ptest, Ptest, Vec, Mem)                               // SSE4_1
  ASMJIT_INST_2x(punpckhbw, Punpckhbw, Mm, Mm)                         // MMX
  ASMJIT_INST_2x(punpckhbw, Punpckhbw, Mm, Mem)                        // MMX
  ASMJIT_INST_2x(punpckhbw, Punpckhbw, Vec, Vec)                       // SSE2
  ASMJIT_INST_2x(punpckhbw, Punpckhbw, Vec, Mem)                       // SSE2
  ASMJIT_INST_2x(punpckhdq, Punpckhdq, Mm, Mm)                         // MMX
  ASMJIT_INST_2x(punpckhdq, Punpckhdq, Mm, Mem)                        // MMX
  ASMJIT_INST_2x(punpckhdq, Punpckhdq, Vec, Vec)                       // SSE2
  ASMJIT_INST_2x(punpckhdq, Punpckhdq, Vec, Mem)                       // SSE2
  ASMJIT_INST_2x(punpckhqdq, Punpckhqdq, Vec, Vec)                     // SSE2
  ASMJIT_INST_2x(punpckhqdq, Punpckhqdq, Vec, Mem)                     // SSE2
  ASMJIT_INST_2x(punpckhwd, Punpckhwd, Mm, Mm)                         // MMX
  ASMJIT_INST_2x(punpckhwd, Punpckhwd, Mm, Mem)                        // MMX
  ASMJIT_INST_2x(punpckhwd, Punpckhwd, Vec, Vec)                       // SSE2
  ASMJIT_INST_2x(punpckhwd, Punpckhwd, Vec, Mem)                       // SSE2
  ASMJIT_INST_2x(punpcklbw, Punpcklbw, Mm, Mm)                         // MMX
  ASMJIT_INST_2x(punpcklbw, Punpcklbw, Mm, Mem)                        // MMX
  ASMJIT_INST_2x(punpcklbw, Punpcklbw, Vec, Vec)                       // SSE2
  ASMJIT_INST_2x(punpcklbw, Punpcklbw, Vec, Mem)                       // SSE2
  ASMJIT_INST_2x(punpckldq, Punpckldq, Mm, Mm)                         // MMX
  ASMJIT_INST_2x(punpckldq, Punpckldq, Mm, Mem)                        // MMX
  ASMJIT_INST_2x(punpckldq, Punpckldq, Vec, Vec)                       // SSE2
  ASMJIT_INST_2x(punpckldq, Punpckldq, Vec, Mem)                       // SSE2
  ASMJIT_INST_2x(punpcklqdq, Punpcklqdq, Vec, Vec)                     // SSE2
  ASMJIT_INST_2x(punpcklqdq, Punpcklqdq, Vec, Mem)                     // SSE2
  ASMJIT_INST_2x(punpcklwd, Punpcklwd, Mm, Mm)                         // MMX
  ASMJIT_INST_2x(punpcklwd, Punpcklwd, Mm, Mem)                        // MMX
  ASMJIT_INST_2x(punpcklwd, Punpcklwd, Vec, Vec)                       // SSE2
  ASMJIT_INST_2x(punpcklwd, Punpcklwd, Vec, Mem)                       // SSE2
  ASMJIT_INST_2x(pxor, Pxor, Mm, Mm)                                   // MMX
  ASMJIT_INST_2x(pxor, Pxor, Mm, Mem)                                  // MMX
  ASMJIT_INST_2x(pxor, Pxor, Vec, Vec)                                 // SSE2
  ASMJIT_INST_2x(pxor, Pxor, Vec, Mem)                                 // SSE2
  ASMJIT_INST_2x(rcpps, Rcpps, Vec, Vec)                               // SSE
  ASMJIT_INST_2x(rcpps, Rcpps, Vec, Mem)                               // SSE
  ASMJIT_INST_2x(rcpss, Rcpss, Vec, Vec)                               // SSE
  ASMJIT_INST_2x(rcpss, Rcpss, Vec, Mem)                               // SSE
  ASMJIT_INST_3x(roundpd, Roundpd, Vec, Vec, Imm)                      // SSE4_1
  ASMJIT_INST_3x(roundpd, Roundpd, Vec, Mem, Imm)                      // SSE4_1
  ASMJIT_INST_3x(roundps, Roundps, Vec, Vec, Imm)                      // SSE4_1
  ASMJIT_INST_3x(roundps, Roundps, Vec, Mem, Imm)                      // SSE4_1
  ASMJIT_INST_3x(roundsd, Roundsd, Vec, Vec, Imm)                      // SSE4_1
  ASMJIT_INST_3x(roundsd, Roundsd, Vec, Mem, Imm)                      // SSE4_1
  ASMJIT_INST_3x(roundss, Roundss, Vec, Vec, Imm)                      // SSE4_1
  ASMJIT_INST_3x(roundss, Roundss, Vec, Mem, Imm)                      // SSE4_1
  ASMJIT_INST_2x(rsqrtps, Rsqrtps, Vec, Vec)                           // SSE
  ASMJIT_INST_2x(rsqrtps, Rsqrtps, Vec, Mem)                           // SSE
  ASMJIT_INST_2x(rsqrtss, Rsqrtss, Vec, Vec)                           // SSE
  ASMJIT_INST_2x(rsqrtss, Rsqrtss, Vec, Mem)                           // SSE
  ASMJIT_INST_3x(shufpd, Shufpd, Vec, Vec, Imm)                        // SSE2
  ASMJIT_INST_3x(shufpd, Shufpd, Vec, Mem, Imm)                        // SSE2
  ASMJIT_INST_3x(shufps, Shufps, Vec, Vec, Imm)                        // SSE
  ASMJIT_INST_3x(shufps, Shufps, Vec, Mem, Imm)                        // SSE
  ASMJIT_INST_2x(sqrtpd, Sqrtpd, Vec, Vec)                             // SSE2
  ASMJIT_INST_2x(sqrtpd, Sqrtpd, Vec, Mem)                             // SSE2
  ASMJIT_INST_2x(sqrtps, Sqrtps, Vec, Vec)                             // SSE
  ASMJIT_INST_2x(sqrtps, Sqrtps, Vec, Mem)                             // SSE
  ASMJIT_INST_2x(sqrtsd, Sqrtsd, Vec, Vec)                             // SSE2
  ASMJIT_INST_2x(sqrtsd, Sqrtsd, Vec, Mem)                             // SSE2
  ASMJIT_INST_2x(sqrtss, Sqrtss, Vec, Vec)                             // SSE
  ASMJIT_INST_2x(sqrtss, Sqrtss, Vec, Mem)                             // SSE
  ASMJIT_INST_2x(subpd, Subpd, Vec, Vec)                               // SSE2
  ASMJIT_INST_2x(subpd, Subpd, Vec, Mem)                               // SSE2
  ASMJIT_INST_2x(subps, Subps, Vec, Vec)                               // SSE
  ASMJIT_INST_2x(subps, Subps, Vec, Mem)                               // SSE
  ASMJIT_INST_2x(subsd, Subsd, Vec, Vec)                               // SSE2
  ASMJIT_INST_2x(subsd, Subsd, Vec, Mem)                               // SSE2
  ASMJIT_INST_2x(subss, Subss, Vec, Vec)                               // SSE
  ASMJIT_INST_2x(subss, Subss, Vec, Mem)                               // SSE
  ASMJIT_INST_2x(ucomisd, Ucomisd, Vec, Vec)                           // SSE2
  ASMJIT_INST_2x(ucomisd, Ucomisd, Vec, Mem)                           // SSE2
  ASMJIT_INST_2x(ucomiss, Ucomiss, Vec, Vec)                           // SSE
  ASMJIT_INST_2x(ucomiss, Ucomiss, Vec, Mem)                           // SSE
  ASMJIT_INST_2x(unpckhpd, Unpckhpd, Vec, Vec)                         // SSE2
  ASMJIT_INST_2x(unpckhpd, Unpckhpd, Vec, Mem)                         // SSE2
  ASMJIT_INST_2x(unpckhps, Unpckhps, Vec, Vec)                         // SSE
  ASMJIT_INST_2x(unpckhps, Unpckhps, Vec, Mem)                         // SSE
  ASMJIT_INST_2x(unpcklpd, Unpcklpd, Vec, Vec)                         // SSE2
  ASMJIT_INST_2x(unpcklpd, Unpcklpd, Vec, Mem)                         // SSE2
  ASMJIT_INST_2x(unpcklps, Unpcklps, Vec, Vec)                         // SSE
  ASMJIT_INST_2x(unpcklps, Unpcklps, Vec, Mem)                         // SSE
  ASMJIT_INST_2x(xorpd, Xorpd, Vec, Vec)                               // SSE2
  ASMJIT_INST_2x(xorpd, Xorpd, Vec, Mem)                               // SSE2
  ASMJIT_INST_2x(xorps, Xorps, Vec, Vec)                               // SSE
  ASMJIT_INST_2x(xorps, Xorps, Vec, Mem)                               // SSE

  //! \}

  //! \name 3DNOW and GEODE Instructions (Deprecated)
  //! \{

  ASMJIT_INST_2x(pavgusb, Pavgusb, Mm, Mm)                             // 3DNOW
  ASMJIT_INST_2x(pavgusb, Pavgusb, Mm, Mem)                            // 3DNOW
  ASMJIT_INST_2x(pf2id, Pf2id, Mm, Mm)                                 // 3DNOW
  ASMJIT_INST_2x(pf2id, Pf2id, Mm, Mem)                                // 3DNOW
  ASMJIT_INST_2x(pf2iw, Pf2iw, Mm, Mm)                                 // 3DNOW
  ASMJIT_INST_2x(pf2iw, Pf2iw, Mm, Mem)                                // 3DNOW
  ASMJIT_INST_2x(pfacc, Pfacc, Mm, Mm)                                 // 3DNOW
  ASMJIT_INST_2x(pfacc, Pfacc, Mm, Mem)                                // 3DNOW
  ASMJIT_INST_2x(pfadd, Pfadd, Mm, Mm)                                 // 3DNOW
  ASMJIT_INST_2x(pfadd, Pfadd, Mm, Mem)                                // 3DNOW
  ASMJIT_INST_2x(pfcmpeq, Pfcmpeq, Mm, Mm)                             // 3DNOW
  ASMJIT_INST_2x(pfcmpeq, Pfcmpeq, Mm, Mem)                            // 3DNOW
  ASMJIT_INST_2x(pfcmpge, Pfcmpge, Mm, Mm)                             // 3DNOW
  ASMJIT_INST_2x(pfcmpge, Pfcmpge, Mm, Mem)                            // 3DNOW
  ASMJIT_INST_2x(pfcmpgt, Pfcmpgt, Mm, Mm)                             // 3DNOW
  ASMJIT_INST_2x(pfcmpgt, Pfcmpgt, Mm, Mem)                            // 3DNOW
  ASMJIT_INST_2x(pfmax, Pfmax, Mm, Mm)                                 // 3DNOW
  ASMJIT_INST_2x(pfmax, Pfmax, Mm, Mem)                                // 3DNOW
  ASMJIT_INST_2x(pfmin, Pfmin, Mm, Mm)                                 // 3DNOW
  ASMJIT_INST_2x(pfmin, Pfmin, Mm, Mem)                                // 3DNOW
  ASMJIT_INST_2x(pfmul, Pfmul, Mm, Mm)                                 // 3DNOW
  ASMJIT_INST_2x(pfmul, Pfmul, Mm, Mem)                                // 3DNOW
  ASMJIT_INST_2x(pfnacc, Pfnacc, Mm, Mm)                               // 3DNOW
  ASMJIT_INST_2x(pfnacc, Pfnacc, Mm, Mem)                              // 3DNOW
  ASMJIT_INST_2x(pfpnacc, Pfpnacc, Mm, Mm)                             // 3DNOW
  ASMJIT_INST_2x(pfpnacc, Pfpnacc, Mm, Mem)                            // 3DNOW
  ASMJIT_INST_2x(pfrcp, Pfrcp, Mm, Mm)                                 // 3DNOW
  ASMJIT_INST_2x(pfrcp, Pfrcp, Mm, Mem)                                // 3DNOW
  ASMJIT_INST_2x(pfrcpit1, Pfrcpit1, Mm, Mm)                           // 3DNOW
  ASMJIT_INST_2x(pfrcpit1, Pfrcpit1, Mm, Mem)                          // 3DNOW
  ASMJIT_INST_2x(pfrcpit2, Pfrcpit2, Mm, Mm)                           // 3DNOW
  ASMJIT_INST_2x(pfrcpit2, Pfrcpit2, Mm, Mem)                          // 3DNOW
  ASMJIT_INST_2x(pfrcpv, Pfrcpv, Mm, Mm)                               // GEODE
  ASMJIT_INST_2x(pfrcpv, Pfrcpv, Mm, Mem)                              // GEODE
  ASMJIT_INST_2x(pfrsqit1, Pfrsqit1, Mm, Mm)                           // 3DNOW
  ASMJIT_INST_2x(pfrsqit1, Pfrsqit1, Mm, Mem)                          // 3DNOW
  ASMJIT_INST_2x(pfrsqrt, Pfrsqrt, Mm, Mm)                             // 3DNOW
  ASMJIT_INST_2x(pfrsqrt, Pfrsqrt, Mm, Mem)                            // 3DNOW
  ASMJIT_INST_2x(pfrsqrtv, Pfrsqrtv, Mm, Mm)                           // GEODE
  ASMJIT_INST_2x(pfrsqrtv, Pfrsqrtv, Mm, Mem)                          // GEODE
  ASMJIT_INST_2x(pfsub, Pfsub, Mm, Mm)                                 // 3DNOW
  ASMJIT_INST_2x(pfsub, Pfsub, Mm, Mem)                                // 3DNOW
  ASMJIT_INST_2x(pfsubr, Pfsubr, Mm, Mm)                               // 3DNOW
  ASMJIT_INST_2x(pfsubr, Pfsubr, Mm, Mem)                              // 3DNOW
  ASMJIT_INST_2x(pi2fd, Pi2fd, Mm, Mm)                                 // 3DNOW
  ASMJIT_INST_2x(pi2fd, Pi2fd, Mm, Mem)                                // 3DNOW
  ASMJIT_INST_2x(pi2fw, Pi2fw, Mm, Mm)                                 // 3DNOW
  ASMJIT_INST_2x(pi2fw, Pi2fw, Mm, Mem)                                // 3DNOW
  ASMJIT_INST_2x(pmulhrw, Pmulhrw, Mm, Mm)                             // 3DNOW
  ASMJIT_INST_2x(pmulhrw, Pmulhrw, Mm, Mem)                            // 3DNOW
  ASMJIT_INST_2x(pswapd, Pswapd, Mm, Mm)                               // 3DNOW
  ASMJIT_INST_2x(pswapd, Pswapd, Mm, Mem)                              // 3DNOW

  //! \}

  //! \name EMMS/FEMMS Instructions
  //! \{

  ASMJIT_INST_0x(emms, Emms)                                           // MMX
  ASMJIT_INST_0x(femms, Femms)                                         // 3DNOW

  //! \}

  //! \name AESNI Instructions
  //! \{

  ASMJIT_INST_2x(aesdec, Aesdec, Vec, Vec)                             // AESNI
  ASMJIT_INST_2x(aesdec, Aesdec, Vec, Mem)                             // AESNI
  ASMJIT_INST_2x(aesdeclast, Aesdeclast, Vec, Vec)                     // AESNI
  ASMJIT_INST_2x(aesdeclast, Aesdeclast, Vec, Mem)                     // AESNI
  ASMJIT_INST_2x(aesenc, Aesenc, Vec, Vec)                             // AESNI
  ASMJIT_INST_2x(aesenc, Aesenc, Vec, Mem)                             // AESNI
  ASMJIT_INST_2x(aesenclast, Aesenclast, Vec, Vec)                     // AESNI
  ASMJIT_INST_2x(aesenclast, Aesenclast, Vec, Mem)                     // AESNI
  ASMJIT_INST_2x(aesimc, Aesimc, Vec, Vec)                             // AESNI
  ASMJIT_INST_2x(aesimc, Aesimc, Vec, Mem)                             // AESNI
  ASMJIT_INST_3x(aeskeygenassist, Aeskeygenassist, Vec, Vec, Imm)      // AESNI
  ASMJIT_INST_3x(aeskeygenassist, Aeskeygenassist, Vec, Mem, Imm)      // AESNI

  //! \}

  //! \name SHA Instructions
  //! \{

  ASMJIT_INST_2x(sha1msg1, Sha1msg1, Vec, Vec)                         // SHA
  ASMJIT_INST_2x(sha1msg1, Sha1msg1, Vec, Mem)                         // SHA
  ASMJIT_INST_2x(sha1msg2, Sha1msg2, Vec, Vec)                         // SHA
  ASMJIT_INST_2x(sha1msg2, Sha1msg2, Vec, Mem)                         // SHA
  ASMJIT_INST_2x(sha1nexte, Sha1nexte, Vec, Vec)                       // SHA
  ASMJIT_INST_2x(sha1nexte, Sha1nexte, Vec, Mem)                       // SHA
  ASMJIT_INST_3x(sha1rnds4, Sha1rnds4, Vec, Vec, Imm)                  // SHA
  ASMJIT_INST_3x(sha1rnds4, Sha1rnds4, Vec, Mem, Imm)                  // SHA
  ASMJIT_INST_2x(sha256msg1, Sha256msg1, Vec, Vec)                     // SHA
  ASMJIT_INST_2x(sha256msg1, Sha256msg1, Vec, Mem)                     // SHA
  ASMJIT_INST_2x(sha256msg2, Sha256msg2, Vec, Vec)                     // SHA
  ASMJIT_INST_2x(sha256msg2, Sha256msg2, Vec, Mem)                     // SHA
  ASMJIT_INST_3x(sha256rnds2, Sha256rnds2, Vec, Vec, XMM0)             // SHA [EXPLICIT]
  ASMJIT_INST_3x(sha256rnds2, Sha256rnds2, Vec, Mem, XMM0)             // SHA [EXPLICIT]

  //! \}

  //! \name GFNI Instructions
  //! \{

  ASMJIT_INST_3x(gf2p8affineinvqb, Gf2p8affineinvqb, Vec, Vec, Imm)    // GFNI
  ASMJIT_INST_3x(gf2p8affineinvqb, Gf2p8affineinvqb, Vec, Mem, Imm)    // GFNI
  ASMJIT_INST_3x(gf2p8affineqb, Gf2p8affineqb, Vec, Vec, Imm)          // GFNI
  ASMJIT_INST_3x(gf2p8affineqb, Gf2p8affineqb, Vec, Mem, Imm)          // GFNI
  ASMJIT_INST_2x(gf2p8mulb, Gf2p8mulb, Vec, Vec)                       // GFNI
  ASMJIT_INST_2x(gf2p8mulb, Gf2p8mulb, Vec, Mem)                       // GFNI

  //! \}

  //! \name AVX, FMA, and AVX512 Instructions
  //! \{

  ASMJIT_INST_3x(kaddb, Kaddb, KReg, KReg, KReg)                       // AVX512_DQ
  ASMJIT_INST_3x(kaddd, Kaddd, KReg, KReg, KReg)                       // AVX512_BW
  ASMJIT_INST_3x(kaddq, Kaddq, KReg, KReg, KReg)                       // AVX512_BW
  ASMJIT_INST_3x(kaddw, Kaddw, KReg, KReg, KReg)                       // AVX512_DQ
  ASMJIT_INST_3x(kandb, Kandb, KReg, KReg, KReg)                       // AVX512_DQ
  ASMJIT_INST_3x(kandd, Kandd, KReg, KReg, KReg)                       // AVX512_BW
  ASMJIT_INST_3x(kandnb, Kandnb, KReg, KReg, KReg)                     // AVX512_DQ
  ASMJIT_INST_3x(kandnd, Kandnd, KReg, KReg, KReg)                     // AVX512_BW
  ASMJIT_INST_3x(kandnq, Kandnq, KReg, KReg, KReg)                     // AVX512_BW
  ASMJIT_INST_3x(kandnw, Kandnw, KReg, KReg, KReg)                     // AVX512_F
  ASMJIT_INST_3x(kandq, Kandq, KReg, KReg, KReg)                       // AVX512_BW
  ASMJIT_INST_3x(kandw, Kandw, KReg, KReg, KReg)                       // AVX512_F
  ASMJIT_INST_2x(kmovb, Kmovb, KReg, KReg)                             // AVX512_DQ
  ASMJIT_INST_2x(kmovb, Kmovb, KReg, Mem)                              // AVX512_DQ
  ASMJIT_INST_2x(kmovb, Kmovb, KReg, Gp)                               // AVX512_DQ
  ASMJIT_INST_2x(kmovb, Kmovb, Mem, KReg)                              // AVX512_DQ
  ASMJIT_INST_2x(kmovb, Kmovb, Gp, KReg)                               // AVX512_DQ
  ASMJIT_INST_2x(kmovd, Kmovd, KReg, KReg)                             // AVX512_BW
  ASMJIT_INST_2x(kmovd, Kmovd, KReg, Mem)                              // AVX512_BW
  ASMJIT_INST_2x(kmovd, Kmovd, KReg, Gp)                               // AVX512_BW
  ASMJIT_INST_2x(kmovd, Kmovd, Mem, KReg)                              // AVX512_BW
  ASMJIT_INST_2x(kmovd, Kmovd, Gp, KReg)                               // AVX512_BW
  ASMJIT_INST_2x(kmovq, Kmovq, KReg, KReg)                             // AVX512_BW
  ASMJIT_INST_2x(kmovq, Kmovq, KReg, Mem)                              // AVX512_BW
  ASMJIT_INST_2x(kmovq, Kmovq, KReg, Gp)                               // AVX512_BW
  ASMJIT_INST_2x(kmovq, Kmovq, Mem, KReg)                              // AVX512_BW
  ASMJIT_INST_2x(kmovq, Kmovq, Gp, KReg)                               // AVX512_BW
  ASMJIT_INST_2x(kmovw, Kmovw, KReg, KReg)                             // AVX512_F
  ASMJIT_INST_2x(kmovw, Kmovw, KReg, Mem)                              // AVX512_F
  ASMJIT_INST_2x(kmovw, Kmovw, KReg, Gp)                               // AVX512_F
  ASMJIT_INST_2x(kmovw, Kmovw, Mem, KReg)                              // AVX512_F
  ASMJIT_INST_2x(kmovw, Kmovw, Gp, KReg)                               // AVX512_F
  ASMJIT_INST_2x(knotb, Knotb, KReg, KReg)                             // AVX512_DQ
  ASMJIT_INST_2x(knotd, Knotd, KReg, KReg)                             // AVX512_BW
  ASMJIT_INST_2x(knotq, Knotq, KReg, KReg)                             // AVX512_BW
  ASMJIT_INST_2x(knotw, Knotw, KReg, KReg)                             // AVX512_F
  ASMJIT_INST_3x(korb, Korb, KReg, KReg, KReg)                         // AVX512_DQ
  ASMJIT_INST_3x(kord, Kord, KReg, KReg, KReg)                         // AVX512_BW
  ASMJIT_INST_3x(korq, Korq, KReg, KReg, KReg)                         // AVX512_BW
  ASMJIT_INST_2x(kortestb, Kortestb, KReg, KReg)                       // AVX512_DQ
  ASMJIT_INST_2x(kortestd, Kortestd, KReg, KReg)                       // AVX512_BW
  ASMJIT_INST_2x(kortestq, Kortestq, KReg, KReg)                       // AVX512_BW
  ASMJIT_INST_2x(kortestw, Kortestw, KReg, KReg)                       // AVX512_F
  ASMJIT_INST_3x(korw, Korw, KReg, KReg, KReg)                         // AVX512_F
  ASMJIT_INST_3x(kshiftlb, Kshiftlb, KReg, KReg, Imm)                  // AVX512_DQ
  ASMJIT_INST_3x(kshiftld, Kshiftld, KReg, KReg, Imm)                  // AVX512_BW
  ASMJIT_INST_3x(kshiftlq, Kshiftlq, KReg, KReg, Imm)                  // AVX512_BW
  ASMJIT_INST_3x(kshiftlw, Kshiftlw, KReg, KReg, Imm)                  // AVX512_F
  ASMJIT_INST_3x(kshiftrb, Kshiftrb, KReg, KReg, Imm)                  // AVX512_DQ
  ASMJIT_INST_3x(kshiftrd, Kshiftrd, KReg, KReg, Imm)                  // AVX512_BW
  ASMJIT_INST_3x(kshiftrq, Kshiftrq, KReg, KReg, Imm)                  // AVX512_BW
  ASMJIT_INST_3x(kshiftrw, Kshiftrw, KReg, KReg, Imm)                  // AVX512_F
  ASMJIT_INST_2x(ktestb, Ktestb, KReg, KReg)                           // AVX512_DQ
  ASMJIT_INST_2x(ktestd, Ktestd, KReg, KReg)                           // AVX512_BW
  ASMJIT_INST_2x(ktestq, Ktestq, KReg, KReg)                           // AVX512_BW
  ASMJIT_INST_2x(ktestw, Ktestw, KReg, KReg)                           // AVX512_DQ
  ASMJIT_INST_3x(kunpckbw, Kunpckbw, KReg, KReg, KReg)                 // AVX512_F
  ASMJIT_INST_3x(kunpckdq, Kunpckdq, KReg, KReg, KReg)                 // AVX512_BW
  ASMJIT_INST_3x(kunpckwd, Kunpckwd, KReg, KReg, KReg)                 // AVX512_BW
  ASMJIT_INST_3x(kxnorb, Kxnorb, KReg, KReg, KReg)                     // AVX512_DQ
  ASMJIT_INST_3x(kxnord, Kxnord, KReg, KReg, KReg)                     // AVX512_BW
  ASMJIT_INST_3x(kxnorq, Kxnorq, KReg, KReg, KReg)                     // AVX512_BW
  ASMJIT_INST_3x(kxnorw, Kxnorw, KReg, KReg, KReg)                     // AVX512_F
  ASMJIT_INST_3x(kxorb, Kxorb, KReg, KReg, KReg)                       // AVX512_DQ
  ASMJIT_INST_3x(kxord, Kxord, KReg, KReg, KReg)                       // AVX512_BW
  ASMJIT_INST_3x(kxorq, Kxorq, KReg, KReg, KReg)                       // AVX512_BW
  ASMJIT_INST_3x(kxorw, Kxorw, KReg, KReg, KReg)                       // AVX512_F
  ASMJIT_INST_3x(vaddpd, Vaddpd, Vec, Vec, Vec)                        // AVX  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vaddpd, Vaddpd, Vec, Vec, Mem)                        // AVX  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vaddps, Vaddps, Vec, Vec, Vec)                        // AVX  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vaddps, Vaddps, Vec, Vec, Mem)                        // AVX  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vaddsd, Vaddsd, Vec, Vec, Vec)                        // AVX  AVX512_F{kz|er}
  ASMJIT_INST_3x(vaddsd, Vaddsd, Vec, Vec, Mem)                        // AVX  AVX512_F{kz|er}
  ASMJIT_INST_3x(vaddss, Vaddss, Vec, Vec, Vec)                        // AVX  AVX512_F{kz|er}
  ASMJIT_INST_3x(vaddss, Vaddss, Vec, Vec, Mem)                        // AVX  AVX512_F{kz|er}
  ASMJIT_INST_3x(vaddsubpd, Vaddsubpd, Vec, Vec, Vec)                  // AVX
  ASMJIT_INST_3x(vaddsubpd, Vaddsubpd, Vec, Vec, Mem)                  // AVX
  ASMJIT_INST_3x(vaddsubps, Vaddsubps, Vec, Vec, Vec)                  // AVX
  ASMJIT_INST_3x(vaddsubps, Vaddsubps, Vec, Vec, Mem)                  // AVX
  ASMJIT_INST_3x(vaesdec, Vaesdec, Vec, Vec, Vec)                      // AVX+AESNI VAES
  ASMJIT_INST_3x(vaesdec, Vaesdec, Vec, Vec, Mem)                      // AVX+AESNI VAES
  ASMJIT_INST_3x(vaesdeclast, Vaesdeclast, Vec, Vec, Vec)              // AVX+AESNI VAES
  ASMJIT_INST_3x(vaesdeclast, Vaesdeclast, Vec, Vec, Mem)              // AVX+AESNI VAES
  ASMJIT_INST_3x(vaesenc, Vaesenc, Vec, Vec, Vec)                      // AVX+AESNI VAES
  ASMJIT_INST_3x(vaesenc, Vaesenc, Vec, Vec, Mem)                      // AVX+AESNI VAES
  ASMJIT_INST_3x(vaesenclast, Vaesenclast, Vec, Vec, Vec)              // AVX+AESNI VAES
  ASMJIT_INST_3x(vaesenclast, Vaesenclast, Vec, Vec, Mem)              // AVX+AESNI VAES
  ASMJIT_INST_2x(vaesimc, Vaesimc, Vec, Vec)                           // AVX+AESNI
  ASMJIT_INST_2x(vaesimc, Vaesimc, Vec, Mem)                           // AVX+AESNI
  ASMJIT_INST_3x(vaeskeygenassist, Vaeskeygenassist, Vec, Vec, Imm)    // AVX+AESNI
  ASMJIT_INST_3x(vaeskeygenassist, Vaeskeygenassist, Vec, Mem, Imm)    // AVX+AESNI
  ASMJIT_INST_4x(valignd, Valignd, Vec, Vec, Vec, Imm)                 //      AVX512_F{kz|b32}
  ASMJIT_INST_4x(valignd, Valignd, Vec, Vec, Mem, Imm)                 //      AVX512_F{kz|b32}
  ASMJIT_INST_4x(valignq, Valignq, Vec, Vec, Vec, Imm)                 //      AVX512_F{kz|b64}
  ASMJIT_INST_4x(valignq, Valignq, Vec, Vec, Mem, Imm)                 //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vandnpd, Vandnpd, Vec, Vec, Vec)                      // AVX  AVX512_DQ{kz|b64}
  ASMJIT_INST_3x(vandnpd, Vandnpd, Vec, Vec, Mem)                      // AVX  AVX512_DQ{kz|b64}
  ASMJIT_INST_3x(vandnps, Vandnps, Vec, Vec, Vec)                      // AVX  AVX512_DQ{kz|b32}
  ASMJIT_INST_3x(vandnps, Vandnps, Vec, Vec, Mem)                      // AVX  AVX512_DQ{kz|b32}
  ASMJIT_INST_3x(vandpd, Vandpd, Vec, Vec, Vec)                        // AVX  AVX512_DQ{kz|b64}
  ASMJIT_INST_3x(vandpd, Vandpd, Vec, Vec, Mem)                        // AVX  AVX512_DQ{kz|b64}
  ASMJIT_INST_3x(vandps, Vandps, Vec, Vec, Vec)                        // AVX  AVX512_DQ{kz|b32}
  ASMJIT_INST_3x(vandps, Vandps, Vec, Vec, Mem)                        // AVX  AVX512_DQ{kz|b32}
  ASMJIT_INST_3x(vblendmpd, Vblendmpd, Vec, Vec, Vec)                  //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vblendmpd, Vblendmpd, Vec, Vec, Mem)                  //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vblendmps, Vblendmps, Vec, Vec, Vec)                  //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vblendmps, Vblendmps, Vec, Vec, Mem)                  //      AVX512_F{kz|b32}
  ASMJIT_INST_4x(vblendpd, Vblendpd, Vec, Vec, Vec, Imm)               // AVX
  ASMJIT_INST_4x(vblendpd, Vblendpd, Vec, Vec, Mem, Imm)               // AVX
  ASMJIT_INST_4x(vblendps, Vblendps, Vec, Vec, Vec, Imm)               // AVX
  ASMJIT_INST_4x(vblendps, Vblendps, Vec, Vec, Mem, Imm)               // AVX
  ASMJIT_INST_4x(vblendvpd, Vblendvpd, Vec, Vec, Vec, Vec)             // AVX
  ASMJIT_INST_4x(vblendvpd, Vblendvpd, Vec, Vec, Mem, Vec)             // AVX
  ASMJIT_INST_4x(vblendvps, Vblendvps, Vec, Vec, Vec, Vec)             // AVX
  ASMJIT_INST_4x(vblendvps, Vblendvps, Vec, Vec, Mem, Vec)             // AVX
  ASMJIT_INST_2x(vbroadcastf128, Vbroadcastf128, Vec, Mem)             // AVX
  ASMJIT_INST_2x(vbroadcastf32x2, Vbroadcastf32x2, Vec, Vec)           //      AVX512_DQ{kz}
  ASMJIT_INST_2x(vbroadcastf32x2, Vbroadcastf32x2, Vec, Mem)           //      AVX512_DQ{kz}
  ASMJIT_INST_2x(vbroadcastf32x4, Vbroadcastf32x4, Vec, Mem)           //      AVX512_F{kz}
  ASMJIT_INST_2x(vbroadcastf32x8, Vbroadcastf32x8, Vec, Mem)           //      AVX512_DQ{kz}
  ASMJIT_INST_2x(vbroadcastf64x2, Vbroadcastf64x2, Vec, Mem)           //      AVX512_DQ{kz}
  ASMJIT_INST_2x(vbroadcastf64x4, Vbroadcastf64x4, Vec, Mem)           //      AVX512_F{kz}
  ASMJIT_INST_2x(vbroadcasti128, Vbroadcasti128, Vec, Mem)             // AVX2
  ASMJIT_INST_2x(vbroadcasti32x2, Vbroadcasti32x2, Vec, Vec)           //      AVX512_DQ{kz}
  ASMJIT_INST_2x(vbroadcasti32x2, Vbroadcasti32x2, Vec, Mem)           //      AVX512_DQ{kz}
  ASMJIT_INST_2x(vbroadcasti32x4, Vbroadcasti32x4, Vec, Mem)           //      AVX512_F{kz}
  ASMJIT_INST_2x(vbroadcasti32x8, Vbroadcasti32x8, Vec, Mem)           //      AVX512_DQ{kz}
  ASMJIT_INST_2x(vbroadcasti64x2, Vbroadcasti64x2, Vec, Vec)           //      AVX512_DQ{kz}
  ASMJIT_INST_2x(vbroadcasti64x2, Vbroadcasti64x2, Vec, Mem)           //      AVX512_DQ{kz}
  ASMJIT_INST_2x(vbroadcasti64x4, Vbroadcasti64x4, Vec, Vec)           //      AVX512_F{kz}
  ASMJIT_INST_2x(vbroadcasti64x4, Vbroadcasti64x4, Vec, Mem)           //      AVX512_F{kz}
  ASMJIT_INST_2x(vbroadcastsd, Vbroadcastsd, Vec, Mem)                 // AVX  AVX512_F{kz}
  ASMJIT_INST_2x(vbroadcastsd, Vbroadcastsd, Vec, Vec)                 // AVX2 AVX512_F{kz}
  ASMJIT_INST_2x(vbroadcastss, Vbroadcastss, Vec, Mem)                 // AVX  AVX512_F{kz}
  ASMJIT_INST_2x(vbroadcastss, Vbroadcastss, Vec, Vec)                 // AVX2 AVX512_F{kz}
  ASMJIT_INST_4x(vcmppd, Vcmppd, Vec, Vec, Vec, Imm)                   // AVX
  ASMJIT_INST_4x(vcmppd, Vcmppd, Vec, Vec, Mem, Imm)                   // AVX
  ASMJIT_INST_4x(vcmppd, Vcmppd, KReg, Vec, Vec, Imm)                  //      AVX512_F{kz|b64}
  ASMJIT_INST_4x(vcmppd, Vcmppd, KReg, Vec, Mem, Imm)                  //      AVX512_F{kz|b64}
  ASMJIT_INST_4x(vcmpps, Vcmpps, Vec, Vec, Vec, Imm)                   // AVX
  ASMJIT_INST_4x(vcmpps, Vcmpps, Vec, Vec, Mem, Imm)                   // AVX
  ASMJIT_INST_4x(vcmpps, Vcmpps, KReg, Vec, Vec, Imm)                  //      AVX512_F{kz|b32}
  ASMJIT_INST_4x(vcmpps, Vcmpps, KReg, Vec, Mem, Imm)                  //      AVX512_F{kz|b32}
  ASMJIT_INST_4x(vcmpsd, Vcmpsd, Vec, Vec, Vec, Imm)                   // AVX
  ASMJIT_INST_4x(vcmpsd, Vcmpsd, Vec, Vec, Mem, Imm)                   // AVX
  ASMJIT_INST_4x(vcmpsd, Vcmpsd, KReg, Vec, Vec, Imm)                  //      AVX512_F{kz|sae}
  ASMJIT_INST_4x(vcmpsd, Vcmpsd, KReg, Vec, Mem, Imm)                  //      AVX512_F{kz|sae}
  ASMJIT_INST_4x(vcmpss, Vcmpss, Vec, Vec, Vec, Imm)                   // AVX
  ASMJIT_INST_4x(vcmpss, Vcmpss, Vec, Vec, Mem, Imm)                   // AVX
  ASMJIT_INST_4x(vcmpss, Vcmpss, KReg, Vec, Vec, Imm)                  //      AVX512_F{kz|sae}
  ASMJIT_INST_4x(vcmpss, Vcmpss, KReg, Vec, Mem, Imm)                  //      AVX512_F{kz|sae}
  ASMJIT_INST_2x(vcomisd, Vcomisd, Vec, Vec)                           // AVX  AVX512_F{sae}
  ASMJIT_INST_2x(vcomisd, Vcomisd, Vec, Mem)                           // AVX  AVX512_F{sae}
  ASMJIT_INST_2x(vcomiss, Vcomiss, Vec, Vec)                           // AVX  AVX512_F{sae}
  ASMJIT_INST_2x(vcomiss, Vcomiss, Vec, Mem)                           // AVX  AVX512_F{sae}
  ASMJIT_INST_2x(vcompresspd, Vcompresspd, Vec, Vec)                   //      AVX512_F{kz}
  ASMJIT_INST_2x(vcompresspd, Vcompresspd, Mem, Vec)                   //      AVX512_F{kz}
  ASMJIT_INST_2x(vcompressps, Vcompressps, Vec, Vec)                   //      AVX512_F{kz}
  ASMJIT_INST_2x(vcompressps, Vcompressps, Mem, Vec)                   //      AVX512_F{kz}
  ASMJIT_INST_2x(vcvtdq2pd, Vcvtdq2pd, Vec, Vec)                       // AVX  AVX512_F{kz|b32}
  ASMJIT_INST_2x(vcvtdq2pd, Vcvtdq2pd, Vec, Mem)                       // AVX  AVX512_F{kz|b32}
  ASMJIT_INST_2x(vcvtdq2ps, Vcvtdq2ps, Vec, Vec)                       // AVX  AVX512_F{kz|b32}
  ASMJIT_INST_2x(vcvtdq2ps, Vcvtdq2ps, Vec, Mem)                       // AVX  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vcvtne2ps2bf16, Vcvtne2ps2bf16, Vec, Vec, Vec)        //      AVX512_BF16{kz|b32}
  ASMJIT_INST_3x(vcvtne2ps2bf16, Vcvtne2ps2bf16, Vec, Vec, Mem)        //      AVX512_BF16{kz|b32}
  ASMJIT_INST_2x(vcvtneps2bf16, Vcvtneps2bf16, Vec, Vec)               //      AVX512_BF16{kz|b32}
  ASMJIT_INST_2x(vcvtneps2bf16, Vcvtneps2bf16, Vec, Mem)               //      AVX512_BF16{kz|b32}
  ASMJIT_INST_2x(vcvtpd2dq, Vcvtpd2dq, Vec, Vec)                       // AVX  AVX512_F{kz|b64}
  ASMJIT_INST_2x(vcvtpd2dq, Vcvtpd2dq, Vec, Mem)                       // AVX  AVX512_F{kz|b64}
  ASMJIT_INST_2x(vcvtpd2ps, Vcvtpd2ps, Vec, Vec)                       // AVX  AVX512_F{kz|b64}
  ASMJIT_INST_2x(vcvtpd2ps, Vcvtpd2ps, Vec, Mem)                       // AVX  AVX512_F{kz|b64}
  ASMJIT_INST_2x(vcvtpd2qq, Vcvtpd2qq, Vec, Vec)                       //      AVX512_DQ{kz|b64}
  ASMJIT_INST_2x(vcvtpd2qq, Vcvtpd2qq, Vec, Mem)                       //      AVX512_DQ{kz|b64}
  ASMJIT_INST_2x(vcvtpd2udq, Vcvtpd2udq, Vec, Vec)                     //      AVX512_F{kz|b64}
  ASMJIT_INST_2x(vcvtpd2udq, Vcvtpd2udq, Vec, Mem)                     //      AVX512_F{kz|b64}
  ASMJIT_INST_2x(vcvtpd2uqq, Vcvtpd2uqq, Vec, Vec)                     //      AVX512_DQ{kz|b64}
  ASMJIT_INST_2x(vcvtpd2uqq, Vcvtpd2uqq, Vec, Mem)                     //      AVX512_DQ{kz|b64}
  ASMJIT_INST_2x(vcvtph2ps, Vcvtph2ps, Vec, Vec)                       // F16C AVX512_F{kz}
  ASMJIT_INST_2x(vcvtph2ps, Vcvtph2ps, Vec, Mem)                       // F16C AVX512_F{kz}
  ASMJIT_INST_2x(vcvtps2dq, Vcvtps2dq, Vec, Vec)                       // AVX  AVX512_F{kz|b32}
  ASMJIT_INST_2x(vcvtps2dq, Vcvtps2dq, Vec, Mem)                       // AVX  AVX512_F{kz|b32}
  ASMJIT_INST_2x(vcvtps2pd, Vcvtps2pd, Vec, Vec)                       // AVX  AVX512_F{kz|b32}
  ASMJIT_INST_2x(vcvtps2pd, Vcvtps2pd, Vec, Mem)                       // AVX  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vcvtps2ph, Vcvtps2ph, Vec, Vec, Imm)                  // F16C AVX512_F{kz}
  ASMJIT_INST_3x(vcvtps2ph, Vcvtps2ph, Mem, Vec, Imm)                  // F16C AVX512_F{kz}
  ASMJIT_INST_2x(vcvtps2qq, Vcvtps2qq, Vec, Vec)                       //      AVX512_DQ{kz|b32}
  ASMJIT_INST_2x(vcvtps2qq, Vcvtps2qq, Vec, Mem)                       //      AVX512_DQ{kz|b32}
  ASMJIT_INST_2x(vcvtps2udq, Vcvtps2udq, Vec, Vec)                     //      AVX512_F{kz|b32}
  ASMJIT_INST_2x(vcvtps2udq, Vcvtps2udq, Vec, Mem)                     //      AVX512_F{kz|b32}
  ASMJIT_INST_2x(vcvtps2uqq, Vcvtps2uqq, Vec, Vec)                     //      AVX512_DQ{kz|b32}
  ASMJIT_INST_2x(vcvtps2uqq, Vcvtps2uqq, Vec, Mem)                     //      AVX512_DQ{kz|b32}
  ASMJIT_INST_2x(vcvtqq2pd, Vcvtqq2pd, Vec, Vec)                       //      AVX512_DQ{kz|b64}
  ASMJIT_INST_2x(vcvtqq2pd, Vcvtqq2pd, Vec, Mem)                       //      AVX512_DQ{kz|b64}
  ASMJIT_INST_2x(vcvtqq2ps, Vcvtqq2ps, Vec, Vec)                       //      AVX512_DQ{kz|b64}
  ASMJIT_INST_2x(vcvtqq2ps, Vcvtqq2ps, Vec, Mem)                       //      AVX512_DQ{kz|b64}
  ASMJIT_INST_2x(vcvtsd2si, Vcvtsd2si, Gp, Vec)                        // AVX  AVX512_F{er}
  ASMJIT_INST_2x(vcvtsd2si, Vcvtsd2si, Gp, Mem)                        // AVX  AVX512_F{er}
  ASMJIT_INST_3x(vcvtsd2ss, Vcvtsd2ss, Vec, Vec, Vec)                  // AVX  AVX512_F{kz|er}
  ASMJIT_INST_3x(vcvtsd2ss, Vcvtsd2ss, Vec, Vec, Mem)                  // AVX  AVX512_F{kz|er}
  ASMJIT_INST_2x(vcvtsd2usi, Vcvtsd2usi, Gp, Vec)                      //      AVX512_F{er}
  ASMJIT_INST_2x(vcvtsd2usi, Vcvtsd2usi, Gp, Mem)                      //      AVX512_F{er}
  ASMJIT_INST_3x(vcvtsi2sd, Vcvtsi2sd, Vec, Vec, Gp)                   // AVX  AVX512_F{er}
  ASMJIT_INST_3x(vcvtsi2sd, Vcvtsi2sd, Vec, Vec, Mem)                  // AVX  AVX512_F{er}
  ASMJIT_INST_3x(vcvtsi2ss, Vcvtsi2ss, Vec, Vec, Gp)                   // AVX  AVX512_F{er}
  ASMJIT_INST_3x(vcvtsi2ss, Vcvtsi2ss, Vec, Vec, Mem)                  // AVX  AVX512_F{er}
  ASMJIT_INST_3x(vcvtss2sd, Vcvtss2sd, Vec, Vec, Vec)                  // AVX  AVX512_F{kz|sae}
  ASMJIT_INST_3x(vcvtss2sd, Vcvtss2sd, Vec, Vec, Mem)                  // AVX  AVX512_F{kz|sae}
  ASMJIT_INST_2x(vcvtss2si, Vcvtss2si, Gp, Vec)                        // AVX  AVX512_F{er}
  ASMJIT_INST_2x(vcvtss2si, Vcvtss2si, Gp, Mem)                        // AVX  AVX512_F{er}
  ASMJIT_INST_2x(vcvtss2usi, Vcvtss2usi, Gp, Vec)                      //      AVX512_F{er}
  ASMJIT_INST_2x(vcvtss2usi, Vcvtss2usi, Gp, Mem)                      //      AVX512_F{er}
  ASMJIT_INST_2x(vcvttpd2dq, Vcvttpd2dq, Vec, Vec)                     // AVX  AVX512_F{kz|b64}
  ASMJIT_INST_2x(vcvttpd2dq, Vcvttpd2dq, Vec, Mem)                     // AVX  AVX512_F{kz|b64}
  ASMJIT_INST_2x(vcvttpd2qq, Vcvttpd2qq, Vec, Vec)                     //      AVX512_F{kz|b64}
  ASMJIT_INST_2x(vcvttpd2qq, Vcvttpd2qq, Vec, Mem)                     //      AVX512_F{kz|b64}
  ASMJIT_INST_2x(vcvttpd2udq, Vcvttpd2udq, Vec, Vec)                   //      AVX512_F{kz|b64}
  ASMJIT_INST_2x(vcvttpd2udq, Vcvttpd2udq, Vec, Mem)                   //      AVX512_F{kz|b64}
  ASMJIT_INST_2x(vcvttpd2uqq, Vcvttpd2uqq, Vec, Vec)                   //      AVX512_DQ{kz|b64}
  ASMJIT_INST_2x(vcvttpd2uqq, Vcvttpd2uqq, Vec, Mem)                   //      AVX512_DQ{kz|b64}
  ASMJIT_INST_2x(vcvttps2dq, Vcvttps2dq, Vec, Vec)                     // AVX  AVX512_F{kz|b32}
  ASMJIT_INST_2x(vcvttps2dq, Vcvttps2dq, Vec, Mem)                     // AVX  AVX512_F{kz|b32}
  ASMJIT_INST_2x(vcvttps2qq, Vcvttps2qq, Vec, Vec)                     //      AVX512_DQ{kz|b32}
  ASMJIT_INST_2x(vcvttps2qq, Vcvttps2qq, Vec, Mem)                     //      AVX512_DQ{kz|b32}
  ASMJIT_INST_2x(vcvttps2udq, Vcvttps2udq, Vec, Vec)                   //      AVX512_F{kz|b32}
  ASMJIT_INST_2x(vcvttps2udq, Vcvttps2udq, Vec, Mem)                   //      AVX512_F{kz|b32}
  ASMJIT_INST_2x(vcvttps2uqq, Vcvttps2uqq, Vec, Vec)                   //      AVX512_DQ{kz|b32}
  ASMJIT_INST_2x(vcvttps2uqq, Vcvttps2uqq, Vec, Mem)                   //      AVX512_DQ{kz|b32}
  ASMJIT_INST_2x(vcvttsd2si, Vcvttsd2si, Gp, Vec)                      // AVX  AVX512_F{sae}
  ASMJIT_INST_2x(vcvttsd2si, Vcvttsd2si, Gp, Mem)                      // AVX  AVX512_F{sae}
  ASMJIT_INST_2x(vcvttsd2usi, Vcvttsd2usi, Gp, Vec)                    //      AVX512_F{sae}
  ASMJIT_INST_2x(vcvttsd2usi, Vcvttsd2usi, Gp, Mem)                    //      AVX512_F{sae}
  ASMJIT_INST_2x(vcvttss2si, Vcvttss2si, Gp, Vec)                      // AVX  AVX512_F{sae}
  ASMJIT_INST_2x(vcvttss2si, Vcvttss2si, Gp, Mem)                      // AVX  AVX512_F{sae}
  ASMJIT_INST_2x(vcvttss2usi, Vcvttss2usi, Gp, Vec)                    //      AVX512_F{sae}
  ASMJIT_INST_2x(vcvttss2usi, Vcvttss2usi, Gp, Mem)                    //      AVX512_F{sae}
  ASMJIT_INST_2x(vcvtudq2pd, Vcvtudq2pd, Vec, Vec)                     //      AVX512_F{kz|b32}
  ASMJIT_INST_2x(vcvtudq2pd, Vcvtudq2pd, Vec, Mem)                     //      AVX512_F{kz|b32}
  ASMJIT_INST_2x(vcvtudq2ps, Vcvtudq2ps, Vec, Vec)                     //      AVX512_F{kz|b32}
  ASMJIT_INST_2x(vcvtudq2ps, Vcvtudq2ps, Vec, Mem)                     //      AVX512_F{kz|b32}
  ASMJIT_INST_2x(vcvtuqq2pd, Vcvtuqq2pd, Vec, Vec)                     //      AVX512_DQ{kz|b64}
  ASMJIT_INST_2x(vcvtuqq2pd, Vcvtuqq2pd, Vec, Mem)                     //      AVX512_DQ{kz|b64}
  ASMJIT_INST_2x(vcvtuqq2ps, Vcvtuqq2ps, Vec, Vec)                     //      AVX512_DQ{kz|b64}
  ASMJIT_INST_2x(vcvtuqq2ps, Vcvtuqq2ps, Vec, Mem)                     //      AVX512_DQ{kz|b64}
  ASMJIT_INST_3x(vcvtusi2sd, Vcvtusi2sd, Vec, Vec, Gp)                 //      AVX512_F{er}
  ASMJIT_INST_3x(vcvtusi2sd, Vcvtusi2sd, Vec, Vec, Mem)                //      AVX512_F{er}
  ASMJIT_INST_3x(vcvtusi2ss, Vcvtusi2ss, Vec, Vec, Gp)                 //      AVX512_F{er}
  ASMJIT_INST_3x(vcvtusi2ss, Vcvtusi2ss, Vec, Vec, Mem)                //      AVX512_F{er}
  ASMJIT_INST_4x(vdbpsadbw, Vdbpsadbw, Vec, Vec, Vec, Imm)             //      AVX512_BW{kz}
  ASMJIT_INST_4x(vdbpsadbw, Vdbpsadbw, Vec, Vec, Mem, Imm)             //      AVX512_BW{kz}
  ASMJIT_INST_3x(vdivpd, Vdivpd, Vec, Vec, Vec)                        // AVX  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vdivpd, Vdivpd, Vec, Vec, Mem)                        // AVX  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vdivps, Vdivps, Vec, Vec, Vec)                        // AVX  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vdivps, Vdivps, Vec, Vec, Mem)                        // AVX  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vdivsd, Vdivsd, Vec, Vec, Vec)                        // AVX  AVX512_F{kz|er}
  ASMJIT_INST_3x(vdivsd, Vdivsd, Vec, Vec, Mem)                        // AVX  AVX512_F{kz|er}
  ASMJIT_INST_3x(vdivss, Vdivss, Vec, Vec, Vec)                        // AVX  AVX512_F{kz|er}
  ASMJIT_INST_3x(vdivss, Vdivss, Vec, Vec, Mem)                        // AVX  AVX512_F{kz|er}
  ASMJIT_INST_3x(vdpbf16ps, Vdpbf16ps, Vec, Vec, Vec)                  //      AVX512_BF16{kz|b32}
  ASMJIT_INST_3x(vdpbf16ps, Vdpbf16ps, Vec, Vec, Mem)                  //      AVX512_BF16{kz|b32}
  ASMJIT_INST_4x(vdppd, Vdppd, Vec, Vec, Vec, Imm)                     // AVX
  ASMJIT_INST_4x(vdppd, Vdppd, Vec, Vec, Mem, Imm)                     // AVX
  ASMJIT_INST_4x(vdpps, Vdpps, Vec, Vec, Vec, Imm)                     // AVX
  ASMJIT_INST_4x(vdpps, Vdpps, Vec, Vec, Mem, Imm)                     // AVX
  ASMJIT_INST_2x(vexpandpd, Vexpandpd, Vec, Vec)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vexpandpd, Vexpandpd, Vec, Mem)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vexpandps, Vexpandps, Vec, Vec)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vexpandps, Vexpandps, Vec, Mem)                       //      AVX512_F{kz}
  ASMJIT_INST_3x(vextractf128, Vextractf128, Vec, Vec, Imm)            // AVX
  ASMJIT_INST_3x(vextractf128, Vextractf128, Mem, Vec, Imm)            // AVX
  ASMJIT_INST_3x(vextractf32x4, Vextractf32x4, Vec, Vec, Imm)          //      AVX512_F{kz}
  ASMJIT_INST_3x(vextractf32x4, Vextractf32x4, Mem, Vec, Imm)          //      AVX512_F{kz}
  ASMJIT_INST_3x(vextractf32x8, Vextractf32x8, Vec, Vec, Imm)          //      AVX512_DQ{kz}
  ASMJIT_INST_3x(vextractf32x8, Vextractf32x8, Mem, Vec, Imm)          //      AVX512_DQ{kz}
  ASMJIT_INST_3x(vextractf64x2, Vextractf64x2, Vec, Vec, Imm)          //      AVX512_DQ{kz}
  ASMJIT_INST_3x(vextractf64x2, Vextractf64x2, Mem, Vec, Imm)          //      AVX512_DQ{kz}
  ASMJIT_INST_3x(vextractf64x4, Vextractf64x4, Vec, Vec, Imm)          //      AVX512_F{kz}
  ASMJIT_INST_3x(vextractf64x4, Vextractf64x4, Mem, Vec, Imm)          //      AVX512_F{kz}
  ASMJIT_INST_3x(vextracti128, Vextracti128, Vec, Vec, Imm)            // AVX2
  ASMJIT_INST_3x(vextracti128, Vextracti128, Mem, Vec, Imm)            // AVX2
  ASMJIT_INST_3x(vextracti32x4, Vextracti32x4, Vec, Vec, Imm)          //      AVX512_F{kz}
  ASMJIT_INST_3x(vextracti32x4, Vextracti32x4, Mem, Vec, Imm)          //      AVX512_F{kz}
  ASMJIT_INST_3x(vextracti32x8, Vextracti32x8, Vec, Vec, Imm)          //      AVX512_DQ{kz}
  ASMJIT_INST_3x(vextracti32x8, Vextracti32x8, Mem, Vec, Imm)          //      AVX512_DQ{kz}
  ASMJIT_INST_3x(vextracti64x2, Vextracti64x2, Vec, Vec, Imm)          //      AVX512_DQ{kz}
  ASMJIT_INST_3x(vextracti64x2, Vextracti64x2, Mem, Vec, Imm)          //      AVX512_DQ{kz}
  ASMJIT_INST_3x(vextracti64x4, Vextracti64x4, Vec, Vec, Imm)          //      AVX512_F{kz}
  ASMJIT_INST_3x(vextracti64x4, Vextracti64x4, Mem, Vec, Imm)          //      AVX512_F{kz}
  ASMJIT_INST_3x(vextractps, Vextractps, Gp, Vec, Imm)                 // AVX  AVX512_F
  ASMJIT_INST_3x(vextractps, Vextractps, Mem, Vec, Imm)                // AVX  AVX512_F
  ASMJIT_INST_4x(vfixupimmpd, Vfixupimmpd, Vec, Vec, Vec, Imm)         //      AVX512_F{kz|b64}
  ASMJIT_INST_4x(vfixupimmpd, Vfixupimmpd, Vec, Vec, Mem, Imm)         //      AVX512_F{kz|b64}
  ASMJIT_INST_4x(vfixupimmps, Vfixupimmps, Vec, Vec, Vec, Imm)         //      AVX512_F{kz|b32}
  ASMJIT_INST_4x(vfixupimmps, Vfixupimmps, Vec, Vec, Mem, Imm)         //      AVX512_F{kz|b32}
  ASMJIT_INST_4x(vfixupimmsd, Vfixupimmsd, Vec, Vec, Vec, Imm)         //      AVX512_F{kz|sae}
  ASMJIT_INST_4x(vfixupimmsd, Vfixupimmsd, Vec, Vec, Mem, Imm)         //      AVX512_F{kz|sae}
  ASMJIT_INST_4x(vfixupimmss, Vfixupimmss, Vec, Vec, Vec, Imm)         //      AVX512_F{kz|sae}
  ASMJIT_INST_4x(vfixupimmss, Vfixupimmss, Vec, Vec, Mem, Imm)         //      AVX512_F{kz|sae}
  ASMJIT_INST_3x(vfmadd132pd, Vfmadd132pd, Vec, Vec, Vec)              // FMA  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vfmadd132pd, Vfmadd132pd, Vec, Vec, Mem)              // FMA  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vfmadd132ps, Vfmadd132ps, Vec, Vec, Vec)              // FMA  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vfmadd132ps, Vfmadd132ps, Vec, Vec, Mem)              // FMA  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vfmadd132sd, Vfmadd132sd, Vec, Vec, Vec)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmadd132sd, Vfmadd132sd, Vec, Vec, Mem)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmadd132ss, Vfmadd132ss, Vec, Vec, Vec)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmadd132ss, Vfmadd132ss, Vec, Vec, Mem)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmadd213pd, Vfmadd213pd, Vec, Vec, Vec)              // FMA  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vfmadd213pd, Vfmadd213pd, Vec, Vec, Mem)              // FMA  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vfmadd213ps, Vfmadd213ps, Vec, Vec, Vec)              // FMA  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vfmadd213ps, Vfmadd213ps, Vec, Vec, Mem)              // FMA  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vfmadd213sd, Vfmadd213sd, Vec, Vec, Vec)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmadd213sd, Vfmadd213sd, Vec, Vec, Mem)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmadd213ss, Vfmadd213ss, Vec, Vec, Vec)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmadd213ss, Vfmadd213ss, Vec, Vec, Mem)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmadd231pd, Vfmadd231pd, Vec, Vec, Vec)              // FMA  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vfmadd231pd, Vfmadd231pd, Vec, Vec, Mem)              // FMA  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vfmadd231ps, Vfmadd231ps, Vec, Vec, Vec)              // FMA  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vfmadd231ps, Vfmadd231ps, Vec, Vec, Mem)              // FMA  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vfmadd231sd, Vfmadd231sd, Vec, Vec, Vec)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmadd231sd, Vfmadd231sd, Vec, Vec, Mem)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmadd231ss, Vfmadd231ss, Vec, Vec, Vec)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmadd231ss, Vfmadd231ss, Vec, Vec, Mem)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmaddsub132pd, Vfmaddsub132pd, Vec, Vec, Vec)        // FMA  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vfmaddsub132pd, Vfmaddsub132pd, Vec, Vec, Mem)        // FMA  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vfmaddsub132ps, Vfmaddsub132ps, Vec, Vec, Vec)        // FMA  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vfmaddsub132ps, Vfmaddsub132ps, Vec, Vec, Mem)        // FMA  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vfmaddsub213pd, Vfmaddsub213pd, Vec, Vec, Vec)        // FMA  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vfmaddsub213pd, Vfmaddsub213pd, Vec, Vec, Mem)        // FMA  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vfmaddsub213ps, Vfmaddsub213ps, Vec, Vec, Vec)        // FMA  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vfmaddsub213ps, Vfmaddsub213ps, Vec, Vec, Mem)        // FMA  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vfmaddsub231pd, Vfmaddsub231pd, Vec, Vec, Vec)        // FMA  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vfmaddsub231pd, Vfmaddsub231pd, Vec, Vec, Mem)        // FMA  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vfmaddsub231ps, Vfmaddsub231ps, Vec, Vec, Vec)        // FMA  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vfmaddsub231ps, Vfmaddsub231ps, Vec, Vec, Mem)        // FMA  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vfmsub132pd, Vfmsub132pd, Vec, Vec, Vec)              // FMA  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vfmsub132pd, Vfmsub132pd, Vec, Vec, Mem)              // FMA  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vfmsub132ps, Vfmsub132ps, Vec, Vec, Vec)              // FMA  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vfmsub132ps, Vfmsub132ps, Vec, Vec, Mem)              // FMA  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vfmsub132sd, Vfmsub132sd, Vec, Vec, Vec)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmsub132sd, Vfmsub132sd, Vec, Vec, Mem)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmsub132ss, Vfmsub132ss, Vec, Vec, Vec)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmsub132ss, Vfmsub132ss, Vec, Vec, Mem)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmsub213pd, Vfmsub213pd, Vec, Vec, Vec)              // FMA  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vfmsub213pd, Vfmsub213pd, Vec, Vec, Mem)              // FMA  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vfmsub213ps, Vfmsub213ps, Vec, Vec, Vec)              // FMA  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vfmsub213ps, Vfmsub213ps, Vec, Vec, Mem)              // FMA  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vfmsub213sd, Vfmsub213sd, Vec, Vec, Vec)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmsub213sd, Vfmsub213sd, Vec, Vec, Mem)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmsub213ss, Vfmsub213ss, Vec, Vec, Vec)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmsub213ss, Vfmsub213ss, Vec, Vec, Mem)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmsub231pd, Vfmsub231pd, Vec, Vec, Vec)              // FMA  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vfmsub231pd, Vfmsub231pd, Vec, Vec, Mem)              // FMA  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vfmsub231ps, Vfmsub231ps, Vec, Vec, Vec)              // FMA  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vfmsub231ps, Vfmsub231ps, Vec, Vec, Mem)              // FMA  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vfmsub231sd, Vfmsub231sd, Vec, Vec, Vec)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmsub231sd, Vfmsub231sd, Vec, Vec, Mem)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmsub231ss, Vfmsub231ss, Vec, Vec, Vec)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmsub231ss, Vfmsub231ss, Vec, Vec, Mem)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmsubadd132pd, Vfmsubadd132pd, Vec, Vec, Vec)        // FMA  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vfmsubadd132pd, Vfmsubadd132pd, Vec, Vec, Mem)        // FMA  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vfmsubadd132ps, Vfmsubadd132ps, Vec, Vec, Vec)        // FMA  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vfmsubadd132ps, Vfmsubadd132ps, Vec, Vec, Mem)        // FMA  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vfmsubadd213pd, Vfmsubadd213pd, Vec, Vec, Vec)        // FMA  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vfmsubadd213pd, Vfmsubadd213pd, Vec, Vec, Mem)        // FMA  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vfmsubadd213ps, Vfmsubadd213ps, Vec, Vec, Vec)        // FMA  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vfmsubadd213ps, Vfmsubadd213ps, Vec, Vec, Mem)        // FMA  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vfmsubadd231pd, Vfmsubadd231pd, Vec, Vec, Vec)        // FMA  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vfmsubadd231pd, Vfmsubadd231pd, Vec, Vec, Mem)        // FMA  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vfmsubadd231ps, Vfmsubadd231ps, Vec, Vec, Vec)        // FMA  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vfmsubadd231ps, Vfmsubadd231ps, Vec, Vec, Mem)        // FMA  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vfnmadd132pd, Vfnmadd132pd, Vec, Vec, Vec)            // FMA  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vfnmadd132pd, Vfnmadd132pd, Vec, Vec, Mem)            // FMA  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vfnmadd132ps, Vfnmadd132ps, Vec, Vec, Vec)            // FMA  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vfnmadd132ps, Vfnmadd132ps, Vec, Vec, Mem)            // FMA  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vfnmadd132sd, Vfnmadd132sd, Vec, Vec, Vec)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmadd132sd, Vfnmadd132sd, Vec, Vec, Mem)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmadd132ss, Vfnmadd132ss, Vec, Vec, Vec)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmadd132ss, Vfnmadd132ss, Vec, Vec, Mem)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmadd213pd, Vfnmadd213pd, Vec, Vec, Vec)            // FMA  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vfnmadd213pd, Vfnmadd213pd, Vec, Vec, Mem)            // FMA  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vfnmadd213ps, Vfnmadd213ps, Vec, Vec, Vec)            // FMA  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vfnmadd213ps, Vfnmadd213ps, Vec, Vec, Mem)            // FMA  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vfnmadd213sd, Vfnmadd213sd, Vec, Vec, Vec)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmadd213sd, Vfnmadd213sd, Vec, Vec, Mem)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmadd213ss, Vfnmadd213ss, Vec, Vec, Vec)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmadd213ss, Vfnmadd213ss, Vec, Vec, Mem)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmadd231pd, Vfnmadd231pd, Vec, Vec, Vec)            // FMA  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vfnmadd231pd, Vfnmadd231pd, Vec, Vec, Mem)            // FMA  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vfnmadd231ps, Vfnmadd231ps, Vec, Vec, Vec)            // FMA  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vfnmadd231ps, Vfnmadd231ps, Vec, Vec, Mem)            // FMA  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vfnmadd231sd, Vfnmadd231sd, Vec, Vec, Vec)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmadd231sd, Vfnmadd231sd, Vec, Vec, Mem)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmadd231ss, Vfnmadd231ss, Vec, Vec, Vec)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmadd231ss, Vfnmadd231ss, Vec, Vec, Mem)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmsub132pd, Vfnmsub132pd, Vec, Vec, Vec)            // FMA  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vfnmsub132pd, Vfnmsub132pd, Vec, Vec, Mem)            // FMA  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vfnmsub132ps, Vfnmsub132ps, Vec, Vec, Vec)            // FMA  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vfnmsub132ps, Vfnmsub132ps, Vec, Vec, Mem)            // FMA  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vfnmsub132sd, Vfnmsub132sd, Vec, Vec, Vec)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmsub132sd, Vfnmsub132sd, Vec, Vec, Mem)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmsub132ss, Vfnmsub132ss, Vec, Vec, Vec)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmsub132ss, Vfnmsub132ss, Vec, Vec, Mem)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmsub213pd, Vfnmsub213pd, Vec, Vec, Vec)            // FMA  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vfnmsub213pd, Vfnmsub213pd, Vec, Vec, Mem)            // FMA  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vfnmsub213ps, Vfnmsub213ps, Vec, Vec, Vec)            // FMA  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vfnmsub213ps, Vfnmsub213ps, Vec, Vec, Mem)            // FMA  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vfnmsub213sd, Vfnmsub213sd, Vec, Vec, Vec)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmsub213sd, Vfnmsub213sd, Vec, Vec, Mem)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmsub213ss, Vfnmsub213ss, Vec, Vec, Vec)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmsub213ss, Vfnmsub213ss, Vec, Vec, Mem)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmsub231pd, Vfnmsub231pd, Vec, Vec, Vec)            // FMA  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vfnmsub231pd, Vfnmsub231pd, Vec, Vec, Mem)            // FMA  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vfnmsub231ps, Vfnmsub231ps, Vec, Vec, Vec)            // FMA  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vfnmsub231ps, Vfnmsub231ps, Vec, Vec, Mem)            // FMA  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vfnmsub231sd, Vfnmsub231sd, Vec, Vec, Vec)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmsub231sd, Vfnmsub231sd, Vec, Vec, Mem)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmsub231ss, Vfnmsub231ss, Vec, Vec, Vec)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmsub231ss, Vfnmsub231ss, Vec, Vec, Mem)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfpclasspd, Vfpclasspd, KReg, Vec, Imm)               //      AVX512_DQ{k|b64}
  ASMJIT_INST_3x(vfpclasspd, Vfpclasspd, KReg, Mem, Imm)               //      AVX512_DQ{k|b64}
  ASMJIT_INST_3x(vfpclassps, Vfpclassps, KReg, Vec, Imm)               //      AVX512_DQ{k|b32}
  ASMJIT_INST_3x(vfpclassps, Vfpclassps, KReg, Mem, Imm)               //      AVX512_DQ{k|b32}
  ASMJIT_INST_3x(vfpclasssd, Vfpclasssd, KReg, Vec, Imm)               //      AVX512_DQ{k}
  ASMJIT_INST_3x(vfpclasssd, Vfpclasssd, KReg, Mem, Imm)               //      AVX512_DQ{k}
  ASMJIT_INST_3x(vfpclassss, Vfpclassss, KReg, Vec, Imm)               //      AVX512_DQ{k}
  ASMJIT_INST_3x(vfpclassss, Vfpclassss, KReg, Mem, Imm)               //      AVX512_DQ{k}
  ASMJIT_INST_2x(vgatherdpd, Vgatherdpd, Vec, Mem)                     //      AVX512_F{k}
  ASMJIT_INST_3x(vgatherdpd, Vgatherdpd, Vec, Mem, Vec)                // AVX2
  ASMJIT_INST_2x(vgatherdps, Vgatherdps, Vec, Mem)                     //      AVX512_F{k}
  ASMJIT_INST_3x(vgatherdps, Vgatherdps, Vec, Mem, Vec)                // AVX2
  ASMJIT_INST_2x(vgatherqpd, Vgatherqpd, Vec, Mem)                     //      AVX512_F{k}
  ASMJIT_INST_3x(vgatherqpd, Vgatherqpd, Vec, Mem, Vec)                // AVX2
  ASMJIT_INST_2x(vgatherqps, Vgatherqps, Vec, Mem)                     //      AVX512_F{k}
  ASMJIT_INST_3x(vgatherqps, Vgatherqps, Vec, Mem, Vec)                // AVX2
  ASMJIT_INST_2x(vgetexppd, Vgetexppd, Vec, Vec)                       //      AVX512_F{kz|b64}
  ASMJIT_INST_2x(vgetexppd, Vgetexppd, Vec, Mem)                       //      AVX512_F{kz|b64}
  ASMJIT_INST_2x(vgetexpps, Vgetexpps, Vec, Vec)                       //      AVX512_F{kz|b32}
  ASMJIT_INST_2x(vgetexpps, Vgetexpps, Vec, Mem)                       //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vgetexpsd, Vgetexpsd, Vec, Vec, Vec)                  //      AVX512_F{kz|sae}
  ASMJIT_INST_3x(vgetexpsd, Vgetexpsd, Vec, Vec, Mem)                  //      AVX512_F{kz|sae}
  ASMJIT_INST_3x(vgetexpss, Vgetexpss, Vec, Vec, Vec)                  //      AVX512_F{kz|sae}
  ASMJIT_INST_3x(vgetexpss, Vgetexpss, Vec, Vec, Mem)                  //      AVX512_F{kz|sae}
  ASMJIT_INST_3x(vgetmantpd, Vgetmantpd, Vec, Vec, Imm)                //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vgetmantpd, Vgetmantpd, Vec, Mem, Imm)                //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vgetmantps, Vgetmantps, Vec, Vec, Imm)                //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vgetmantps, Vgetmantps, Vec, Mem, Imm)                //      AVX512_F{kz|b32}
  ASMJIT_INST_4x(vgetmantsd, Vgetmantsd, Vec, Vec, Vec, Imm)           //      AVX512_F{kz|sae}
  ASMJIT_INST_4x(vgetmantsd, Vgetmantsd, Vec, Vec, Mem, Imm)           //      AVX512_F{kz|sae}
  ASMJIT_INST_4x(vgetmantss, Vgetmantss, Vec, Vec, Vec, Imm)           //      AVX512_F{kz|sae}
  ASMJIT_INST_4x(vgetmantss, Vgetmantss, Vec, Vec, Mem, Imm)           //      AVX512_F{kz|sae}
  ASMJIT_INST_4x(vgf2p8affineinvqb, Vgf2p8affineinvqb,Vec,Vec,Vec,Imm) // AVX  AVX512_VL{kz} GFNI
  ASMJIT_INST_4x(vgf2p8affineinvqb, Vgf2p8affineinvqb,Vec,Vec,Mem,Imm) // AVX  AVX512_VL{kz} GFNI
  ASMJIT_INST_4x(vgf2p8affineqb, Vgf2p8affineqb, Vec, Vec, Vec, Imm)   // AVX  AVX512_VL{kz} GFNI
  ASMJIT_INST_4x(vgf2p8affineqb, Vgf2p8affineqb, Vec, Vec, Mem, Imm)   // AVX  AVX512_VL{kz} GFNI
  ASMJIT_INST_3x(vgf2p8mulb, Vgf2p8mulb, Vec, Vec, Vec)                // AVX  AVX512_VL{kz} GFNI
  ASMJIT_INST_3x(vgf2p8mulb, Vgf2p8mulb, Vec, Vec, Mem)                // AVX  AVX512_VL{kz} GFNI
  ASMJIT_INST_3x(vhaddpd, Vhaddpd, Vec, Vec, Vec)                      // AVX
  ASMJIT_INST_3x(vhaddpd, Vhaddpd, Vec, Vec, Mem)                      // AVX
  ASMJIT_INST_3x(vhaddps, Vhaddps, Vec, Vec, Vec)                      // AVX
  ASMJIT_INST_3x(vhaddps, Vhaddps, Vec, Vec, Mem)                      // AVX
  ASMJIT_INST_3x(vhsubpd, Vhsubpd, Vec, Vec, Vec)                      // AVX
  ASMJIT_INST_3x(vhsubpd, Vhsubpd, Vec, Vec, Mem)                      // AVX
  ASMJIT_INST_3x(vhsubps, Vhsubps, Vec, Vec, Vec)                      // AVX
  ASMJIT_INST_3x(vhsubps, Vhsubps, Vec, Vec, Mem)                      // AVX
  ASMJIT_INST_4x(vinsertf128, Vinsertf128, Vec, Vec, Vec, Imm)         // AVX
  ASMJIT_INST_4x(vinsertf128, Vinsertf128, Vec, Vec, Mem, Imm)         // AVX
  ASMJIT_INST_4x(vinsertf32x4, Vinsertf32x4, Vec, Vec, Vec, Imm)       //      AVX512_F{kz}
  ASMJIT_INST_4x(vinsertf32x4, Vinsertf32x4, Vec, Vec, Mem, Imm)       //      AVX512_F{kz}
  ASMJIT_INST_4x(vinsertf32x8, Vinsertf32x8, Vec, Vec, Vec, Imm)       //      AVX512_DQ{kz}
  ASMJIT_INST_4x(vinsertf32x8, Vinsertf32x8, Vec, Vec, Mem, Imm)       //      AVX512_DQ{kz}
  ASMJIT_INST_4x(vinsertf64x2, Vinsertf64x2, Vec, Vec, Vec, Imm)       //      AVX512_DQ{kz}
  ASMJIT_INST_4x(vinsertf64x2, Vinsertf64x2, Vec, Vec, Mem, Imm)       //      AVX512_DQ{kz}
  ASMJIT_INST_4x(vinsertf64x4, Vinsertf64x4, Vec, Vec, Vec, Imm)       //      AVX512_F{kz}
  ASMJIT_INST_4x(vinsertf64x4, Vinsertf64x4, Vec, Vec, Mem, Imm)       //      AVX512_F{kz}
  ASMJIT_INST_4x(vinserti128, Vinserti128, Vec, Vec, Vec, Imm)         // AVX2
  ASMJIT_INST_4x(vinserti128, Vinserti128, Vec, Vec, Mem, Imm)         // AVX2
  ASMJIT_INST_4x(vinserti32x4, Vinserti32x4, Vec, Vec, Vec, Imm)       //      AVX512_F{kz}
  ASMJIT_INST_4x(vinserti32x4, Vinserti32x4, Vec, Vec, Mem, Imm)       //      AVX512_F{kz}
  ASMJIT_INST_4x(vinserti32x8, Vinserti32x8, Vec, Vec, Vec, Imm)       //      AVX512_DQ{kz}
  ASMJIT_INST_4x(vinserti32x8, Vinserti32x8, Vec, Vec, Mem, Imm)       //      AVX512_DQ{kz}
  ASMJIT_INST_4x(vinserti64x2, Vinserti64x2, Vec, Vec, Vec, Imm)       //      AVX512_DQ{kz}
  ASMJIT_INST_4x(vinserti64x2, Vinserti64x2, Vec, Vec, Mem, Imm)       //      AVX512_DQ{kz}
  ASMJIT_INST_4x(vinserti64x4, Vinserti64x4, Vec, Vec, Vec, Imm)       //      AVX512_F{kz}
  ASMJIT_INST_4x(vinserti64x4, Vinserti64x4, Vec, Vec, Mem, Imm)       //      AVX512_F{kz}
  ASMJIT_INST_4x(vinsertps, Vinsertps, Vec, Vec, Vec, Imm)             // AVX  AVX512_F
  ASMJIT_INST_4x(vinsertps, Vinsertps, Vec, Vec, Mem, Imm)             // AVX  AVX512_F
  ASMJIT_INST_2x(vlddqu, Vlddqu, Vec, Mem)                             // AVX
  ASMJIT_INST_1x(vldmxcsr, Vldmxcsr, Mem)                              // AVX
  ASMJIT_INST_3x(vmaskmovdqu, Vmaskmovdqu, Vec, Vec, DS_ZDI)           // AVX  [EXPLICIT]
  ASMJIT_INST_3x(vmaskmovpd, Vmaskmovpd, Mem, Vec, Vec)                // AVX
  ASMJIT_INST_3x(vmaskmovpd, Vmaskmovpd, Vec, Vec, Mem)                // AVX
  ASMJIT_INST_3x(vmaskmovps, Vmaskmovps, Mem, Vec, Vec)                // AVX
  ASMJIT_INST_3x(vmaskmovps, Vmaskmovps, Vec, Vec, Mem)                // AVX
  ASMJIT_INST_3x(vmaxpd, Vmaxpd, Vec, Vec, Vec)                        // AVX  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vmaxpd, Vmaxpd, Vec, Vec, Mem)                        // AVX  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vmaxps, Vmaxps, Vec, Vec, Vec)                        // AVX  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vmaxps, Vmaxps, Vec, Vec, Mem)                        // AVX  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vmaxsd, Vmaxsd, Vec, Vec, Vec)                        // AVX  AVX512_F{kz|sae}
  ASMJIT_INST_3x(vmaxsd, Vmaxsd, Vec, Vec, Mem)                        // AVX  AVX512_F{kz|sae}
  ASMJIT_INST_3x(vmaxss, Vmaxss, Vec, Vec, Vec)                        // AVX  AVX512_F{kz|sae}
  ASMJIT_INST_3x(vmaxss, Vmaxss, Vec, Vec, Mem)                        // AVX  AVX512_F{kz|sae}
  ASMJIT_INST_3x(vminpd, Vminpd, Vec, Vec, Vec)                        // AVX  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vminpd, Vminpd, Vec, Vec, Mem)                        // AVX  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vminps, Vminps, Vec, Vec, Vec)                        // AVX  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vminps, Vminps, Vec, Vec, Mem)                        // AVX  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vminsd, Vminsd, Vec, Vec, Vec)                        // AVX  AVX512_F{kz|sae}
  ASMJIT_INST_3x(vminsd, Vminsd, Vec, Vec, Mem)                        // AVX  AVX512_F{kz|sae}
  ASMJIT_INST_3x(vminss, Vminss, Vec, Vec, Vec)                        // AVX  AVX512_F{kz|sae}
  ASMJIT_INST_3x(vminss, Vminss, Vec, Vec, Mem)                        // AVX  AVX512_F{kz|sae}
  ASMJIT_INST_2x(vmovapd, Vmovapd, Vec, Vec)                           // AVX  AVX512_F{kz}
  ASMJIT_INST_2x(vmovapd, Vmovapd, Vec, Mem)                           // AVX  AVX512_F{kz}
  ASMJIT_INST_2x(vmovapd, Vmovapd, Mem, Vec)                           // AVX  AVX512_F{kz}
  ASMJIT_INST_2x(vmovaps, Vmovaps, Vec, Vec)                           // AVX  AVX512_F{kz}
  ASMJIT_INST_2x(vmovaps, Vmovaps, Vec, Mem)                           // AVX  AVX512_F{kz}
  ASMJIT_INST_2x(vmovaps, Vmovaps, Mem, Vec)                           // AVX  AVX512_F{kz}
  ASMJIT_INST_2x(vmovd, Vmovd, Gp, Vec)                                // AVX  AVX512_F
  ASMJIT_INST_2x(vmovd, Vmovd, Mem, Vec)                               // AVX  AVX512_F
  ASMJIT_INST_2x(vmovd, Vmovd, Vec, Gp)                                // AVX  AVX512_F
  ASMJIT_INST_2x(vmovd, Vmovd, Vec, Mem)                               // AVX  AVX512_F
  ASMJIT_INST_2x(vmovddup, Vmovddup, Vec, Vec)                         // AVX  AVX512_F{kz}
  ASMJIT_INST_2x(vmovddup, Vmovddup, Vec, Mem)                         // AVX  AVX512_F{kz}
  ASMJIT_INST_2x(vmovdqa, Vmovdqa, Vec, Vec)                           // AVX
  ASMJIT_INST_2x(vmovdqa, Vmovdqa, Vec, Mem)                           // AVX
  ASMJIT_INST_2x(vmovdqa, Vmovdqa, Mem, Vec)                           // AVX
  ASMJIT_INST_2x(vmovdqa32, Vmovdqa32, Vec, Vec)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vmovdqa32, Vmovdqa32, Vec, Mem)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vmovdqa32, Vmovdqa32, Mem, Vec)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vmovdqa64, Vmovdqa64, Vec, Vec)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vmovdqa64, Vmovdqa64, Vec, Mem)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vmovdqa64, Vmovdqa64, Mem, Vec)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vmovdqu, Vmovdqu, Vec, Vec)                           // AVX
  ASMJIT_INST_2x(vmovdqu, Vmovdqu, Vec, Mem)                           // AVX
  ASMJIT_INST_2x(vmovdqu, Vmovdqu, Mem, Vec)                           // AVX
  ASMJIT_INST_2x(vmovdqu16, Vmovdqu16, Vec, Vec)                       //      AVX512_BW{kz}
  ASMJIT_INST_2x(vmovdqu16, Vmovdqu16, Vec, Mem)                       //      AVX512_BW{kz}
  ASMJIT_INST_2x(vmovdqu16, Vmovdqu16, Mem, Vec)                       //      AVX512_BW{kz}
  ASMJIT_INST_2x(vmovdqu32, Vmovdqu32, Vec, Vec)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vmovdqu32, Vmovdqu32, Vec, Mem)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vmovdqu32, Vmovdqu32, Mem, Vec)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vmovdqu64, Vmovdqu64, Vec, Vec)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vmovdqu64, Vmovdqu64, Vec, Mem)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vmovdqu64, Vmovdqu64, Mem, Vec)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vmovdqu8, Vmovdqu8, Vec, Vec)                         //      AVX512_BW{kz}
  ASMJIT_INST_2x(vmovdqu8, Vmovdqu8, Vec, Mem)                         //      AVX512_BW{kz}
  ASMJIT_INST_2x(vmovdqu8, Vmovdqu8, Mem, Vec)                         //      AVX512_BW{kz}
  ASMJIT_INST_3x(vmovhlps, Vmovhlps, Vec, Vec, Vec)                    // AVX  AVX512_F
  ASMJIT_INST_2x(vmovhpd, Vmovhpd, Mem, Vec)                           // AVX  AVX512_F
  ASMJIT_INST_3x(vmovhpd, Vmovhpd, Vec, Vec, Mem)                      // AVX  AVX512_F
  ASMJIT_INST_2x(vmovhps, Vmovhps, Mem, Vec)                           // AVX  AVX512_F
  ASMJIT_INST_3x(vmovhps, Vmovhps, Vec, Vec, Mem)                      // AVX  AVX512_F
  ASMJIT_INST_3x(vmovlhps, Vmovlhps, Vec, Vec, Vec)                    // AVX  AVX512_F
  ASMJIT_INST_2x(vmovlpd, Vmovlpd, Mem, Vec)                           // AVX  AVX512_F
  ASMJIT_INST_3x(vmovlpd, Vmovlpd, Vec, Vec, Mem)                      // AVX  AVX512_F
  ASMJIT_INST_2x(vmovlps, Vmovlps, Mem, Vec)                           // AVX  AVX512_F
  ASMJIT_INST_3x(vmovlps, Vmovlps, Vec, Vec, Mem)                      // AVX  AVX512_F
  ASMJIT_INST_2x(vmovmskpd, Vmovmskpd, Gp, Vec)                        // AVX
  ASMJIT_INST_2x(vmovmskps, Vmovmskps, Gp, Vec)                        // AVX
  ASMJIT_INST_2x(vmovntdq, Vmovntdq, Mem, Vec)                         // AVX+ AVX512_F
  ASMJIT_INST_2x(vmovntdqa, Vmovntdqa, Vec, Mem)                       // AVX+ AVX512_F
  ASMJIT_INST_2x(vmovntpd, Vmovntpd, Mem, Vec)                         // AVX  AVX512_F
  ASMJIT_INST_2x(vmovntps, Vmovntps, Mem, Vec)                         // AVX  AVX512_F
  ASMJIT_INST_2x(vmovq, Vmovq, Gp, Vec)                                // AVX  AVX512_F
  ASMJIT_INST_2x(vmovq, Vmovq, Mem, Vec)                               // AVX  AVX512_F
  ASMJIT_INST_2x(vmovq, Vmovq, Vec, Mem)                               // AVX  AVX512_F
  ASMJIT_INST_2x(vmovq, Vmovq, Vec, Gp)                                // AVX  AVX512_F
  ASMJIT_INST_2x(vmovq, Vmovq, Vec, Vec)                               // AVX  AVX512_F
  ASMJIT_INST_2x(vmovsd, Vmovsd, Mem, Vec)                             // AVX  AVX512_F
  ASMJIT_INST_2x(vmovsd, Vmovsd, Vec, Mem)                             // AVX  AVX512_F{kz}
  ASMJIT_INST_3x(vmovsd, Vmovsd, Vec, Vec, Vec)                        // AVX  AVX512_F{kz}
  ASMJIT_INST_2x(vmovshdup, Vmovshdup, Vec, Vec)                       // AVX  AVX512_F{kz}
  ASMJIT_INST_2x(vmovshdup, Vmovshdup, Vec, Mem)                       // AVX  AVX512_F{kz}
  ASMJIT_INST_2x(vmovsldup, Vmovsldup, Vec, Vec)                       // AVX  AVX512_F{kz}
  ASMJIT_INST_2x(vmovsldup, Vmovsldup, Vec, Mem)                       // AVX  AVX512_F{kz}
  ASMJIT_INST_2x(vmovss, Vmovss, Mem, Vec)                             // AVX  AVX512_F
  ASMJIT_INST_2x(vmovss, Vmovss, Vec, Mem)                             // AVX  AVX512_F{kz}
  ASMJIT_INST_3x(vmovss, Vmovss, Vec, Vec, Vec)                        // AVX  AVX512_F{kz}
  ASMJIT_INST_2x(vmovupd, Vmovupd, Vec, Vec)                           // AVX  AVX512_F{kz}
  ASMJIT_INST_2x(vmovupd, Vmovupd, Vec, Mem)                           // AVX  AVX512_F{kz}
  ASMJIT_INST_2x(vmovupd, Vmovupd, Mem, Vec)                           // AVX  AVX512_F{kz}
  ASMJIT_INST_2x(vmovups, Vmovups, Vec, Vec)                           // AVX  AVX512_F{kz}
  ASMJIT_INST_2x(vmovups, Vmovups, Vec, Mem)                           // AVX  AVX512_F{kz}
  ASMJIT_INST_2x(vmovups, Vmovups, Mem, Vec)                           // AVX  AVX512_F{kz}
  ASMJIT_INST_4x(vmpsadbw, Vmpsadbw, Vec, Vec, Vec, Imm)               // AVX+
  ASMJIT_INST_4x(vmpsadbw, Vmpsadbw, Vec, Vec, Mem, Imm)               // AVX+
  ASMJIT_INST_3x(vmulpd, Vmulpd, Vec, Vec, Vec)                        // AVX  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vmulpd, Vmulpd, Vec, Vec, Mem)                        // AVX  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vmulps, Vmulps, Vec, Vec, Vec)                        // AVX  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vmulps, Vmulps, Vec, Vec, Mem)                        // AVX  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vmulsd, Vmulsd, Vec, Vec, Vec)                        // AVX  AVX512_F{kz|er}
  ASMJIT_INST_3x(vmulsd, Vmulsd, Vec, Vec, Mem)                        // AVX  AVX512_F{kz|er}
  ASMJIT_INST_3x(vmulss, Vmulss, Vec, Vec, Vec)                        // AVX  AVX512_F{kz|er}
  ASMJIT_INST_3x(vmulss, Vmulss, Vec, Vec, Mem)                        // AVX  AVX512_F{kz|er}
  ASMJIT_INST_3x(vorpd, Vorpd, Vec, Vec, Vec)                          // AVX  AVX512_DQ{kz|b64}
  ASMJIT_INST_3x(vorpd, Vorpd, Vec, Vec, Mem)                          // AVX  AVX512_DQ{kz|b64}
  ASMJIT_INST_3x(vorps, Vorps, Vec, Vec, Vec)                          // AVX  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vorps, Vorps, Vec, Vec, Mem)                          // AVX  AVX512_F{kz|b32}
  ASMJIT_INST_4x(vp2intersectd, Vp2intersectd, KReg, KReg, Vec, Vec)   //      AVX512_VP2INTERSECT{kz}
  ASMJIT_INST_4x(vp2intersectd, Vp2intersectd, KReg, KReg, Vec, Mem)   //      AVX512_VP2INTERSECT{kz}
  ASMJIT_INST_4x(vp2intersectq, Vp2intersectq, KReg, KReg, Vec, Vec)   //      AVX512_VP2INTERSECT{kz}
  ASMJIT_INST_4x(vp2intersectq, Vp2intersectq, KReg, KReg, Vec, Mem)   //      AVX512_VP2INTERSECT{kz}
  ASMJIT_INST_2x(vpabsb, Vpabsb, Vec, Vec)                             // AVX+ AVX512_BW{kz}
  ASMJIT_INST_2x(vpabsb, Vpabsb, Vec, Mem)                             // AVX+ AVX512_BW{kz}
  ASMJIT_INST_2x(vpabsd, Vpabsd, Vec, Vec)                             // AVX+ AVX512_F{kz}
  ASMJIT_INST_2x(vpabsd, Vpabsd, Vec, Mem)                             // AVX+ AVX512_F{kz}
  ASMJIT_INST_2x(vpabsq, Vpabsq, Vec, Vec)                             //      AVX512_F{kz}
  ASMJIT_INST_2x(vpabsq, Vpabsq, Vec, Mem)                             //      AVX512_F{kz}
  ASMJIT_INST_2x(vpabsw, Vpabsw, Vec, Vec)                             // AVX+ AVX512_BW{kz}
  ASMJIT_INST_2x(vpabsw, Vpabsw, Vec, Mem)                             // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpackssdw, Vpackssdw, Vec, Vec, Vec)                  // AVX+ AVX512_BW{kz|b32}
  ASMJIT_INST_3x(vpackssdw, Vpackssdw, Vec, Vec, Mem)                  // AVX+ AVX512_BW{kz|b32}
  ASMJIT_INST_3x(vpacksswb, Vpacksswb, Vec, Vec, Vec)                  // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpacksswb, Vpacksswb, Vec, Vec, Mem)                  // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpackusdw, Vpackusdw, Vec, Vec, Vec)                  // AVX+ AVX512_BW{kz|b32}
  ASMJIT_INST_3x(vpackusdw, Vpackusdw, Vec, Vec, Mem)                  // AVX+ AVX512_BW{kz|b32}
  ASMJIT_INST_3x(vpackuswb, Vpackuswb, Vec, Vec, Vec)                  // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpackuswb, Vpackuswb, Vec, Vec, Mem)                  // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpaddb, Vpaddb, Vec, Vec, Vec)                        // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpaddb, Vpaddb, Vec, Vec, Mem)                        // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpaddd, Vpaddd, Vec, Vec, Vec)                        // AVX+ AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpaddd, Vpaddd, Vec, Vec, Mem)                        // AVX+ AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpaddq, Vpaddq, Vec, Vec, Vec)                        // AVX+ AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpaddq, Vpaddq, Vec, Vec, Mem)                        // AVX+ AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpaddsb, Vpaddsb, Vec, Vec, Vec)                      // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpaddsb, Vpaddsb, Vec, Vec, Mem)                      // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpaddsw, Vpaddsw, Vec, Vec, Vec)                      // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpaddsw, Vpaddsw, Vec, Vec, Mem)                      // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpaddusb, Vpaddusb, Vec, Vec, Vec)                    // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpaddusb, Vpaddusb, Vec, Vec, Mem)                    // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpaddusw, Vpaddusw, Vec, Vec, Vec)                    // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpaddusw, Vpaddusw, Vec, Vec, Mem)                    // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpaddw, Vpaddw, Vec, Vec, Vec)                        // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpaddw, Vpaddw, Vec, Vec, Mem)                        // AVX+ AVX512_BW{kz}
  ASMJIT_INST_4x(vpalignr, Vpalignr, Vec, Vec, Vec, Imm)               // AVX+ AVX512_BW{kz}
  ASMJIT_INST_4x(vpalignr, Vpalignr, Vec, Vec, Mem, Imm)               // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpand, Vpand, Vec, Vec, Vec)                          // AVX+
  ASMJIT_INST_3x(vpand, Vpand, Vec, Vec, Mem)                          // AVX+
  ASMJIT_INST_3x(vpandd, Vpandd, Vec, Vec, Vec)                        //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpandd, Vpandd, Vec, Vec, Mem)                        //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpandn, Vpandn, Vec, Vec, Vec)                        // AVX+
  ASMJIT_INST_3x(vpandn, Vpandn, Vec, Vec, Mem)                        // AVX+
  ASMJIT_INST_3x(vpandnd, Vpandnd, Vec, Vec, Vec)                      //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpandnd, Vpandnd, Vec, Vec, Mem)                      //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpandnq, Vpandnq, Vec, Vec, Vec)                      //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpandnq, Vpandnq, Vec, Vec, Mem)                      //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpandq, Vpandq, Vec, Vec, Vec)                        //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpandq, Vpandq, Vec, Vec, Mem)                        //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpavgb, Vpavgb, Vec, Vec, Vec)                        // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpavgb, Vpavgb, Vec, Vec, Mem)                        // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpavgw, Vpavgw, Vec, Vec, Vec)                        // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpavgw, Vpavgw, Vec, Vec, Mem)                        // AVX+ AVX512_BW{kz}
  ASMJIT_INST_4x(vpblendd, Vpblendd, Vec, Vec, Vec, Imm)               // AVX2
  ASMJIT_INST_4x(vpblendd, Vpblendd, Vec, Vec, Mem, Imm)               // AVX2
  ASMJIT_INST_3x(vpblendmb, Vpblendmb, Vec, Vec, Vec)                  //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpblendmb, Vpblendmb, Vec, Vec, Mem)                  //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpblendmd, Vpblendmd, Vec, Vec, Vec)                  //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpblendmd, Vpblendmd, Vec, Vec, Mem)                  //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpblendmq, Vpblendmq, Vec, Vec, Vec)                  //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpblendmq, Vpblendmq, Vec, Vec, Mem)                  //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpblendmw, Vpblendmw, Vec, Vec, Vec)                  //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpblendmw, Vpblendmw, Vec, Vec, Mem)                  //      AVX512_BW{kz}
  ASMJIT_INST_4x(vpblendvb, Vpblendvb, Vec, Vec, Vec, Vec)             // AVX+
  ASMJIT_INST_4x(vpblendvb, Vpblendvb, Vec, Vec, Mem, Vec)             // AVX+
  ASMJIT_INST_4x(vpblendw, Vpblendw, Vec, Vec, Vec, Imm)               // AVX+
  ASMJIT_INST_4x(vpblendw, Vpblendw, Vec, Vec, Mem, Imm)               // AVX+
  ASMJIT_INST_2x(vpbroadcastb, Vpbroadcastb, Vec, Vec)                 // AVX2 AVX512_BW{kz}
  ASMJIT_INST_2x(vpbroadcastb, Vpbroadcastb, Vec, Mem)                 // AVX2 AVX512_BW{kz}
  ASMJIT_INST_2x(vpbroadcastb, Vpbroadcastb, Vec, Gp)                  //      AVX512_BW{kz}
  ASMJIT_INST_2x(vpbroadcastd, Vpbroadcastd, Vec, Vec)                 // AVX2 AVX512_F{kz}
  ASMJIT_INST_2x(vpbroadcastd, Vpbroadcastd, Vec, Mem)                 // AVX2 AVX512_F{kz}
  ASMJIT_INST_2x(vpbroadcastd, Vpbroadcastd, Vec, Gp)                  //      AVX512_F{kz}
  ASMJIT_INST_2x(vpbroadcastmb2q, Vpbroadcastmb2q, Vec, KReg)          //      AVX512_CD
  ASMJIT_INST_2x(vpbroadcastmw2d, Vpbroadcastmw2d, Vec, KReg)          //      AVX512_CD
  ASMJIT_INST_2x(vpbroadcastq, Vpbroadcastq, Vec, Vec)                 // AVX2 AVX512_F{kz}
  ASMJIT_INST_2x(vpbroadcastq, Vpbroadcastq, Vec, Mem)                 // AVX2 AVX512_F{kz}
  ASMJIT_INST_2x(vpbroadcastq, Vpbroadcastq, Vec, Gp)                  //      AVX512_F{kz}
  ASMJIT_INST_2x(vpbroadcastw, Vpbroadcastw, Vec, Vec)                 // AVX2 AVX512_BW{kz}
  ASMJIT_INST_2x(vpbroadcastw, Vpbroadcastw, Vec, Mem)                 // AVX2 AVX512_BW{kz}
  ASMJIT_INST_2x(vpbroadcastw, Vpbroadcastw, Vec, Gp)                  //      AVX512_BW{kz}
  ASMJIT_INST_4x(vpclmulqdq, Vpclmulqdq, Vec, Vec, Vec, Imm)           // AVX  VPCLMULQDQ AVX512_F
  ASMJIT_INST_4x(vpclmulqdq, Vpclmulqdq, Vec, Vec, Mem, Imm)           // AVX  VPCLMULQDQ AVX512_F
  ASMJIT_INST_4x(vpcmpb, Vpcmpb, KReg, Vec, Vec, Imm)                  //      AVX512_BW{k}
  ASMJIT_INST_4x(vpcmpb, Vpcmpb, KReg, Vec, Mem, Imm)                  //      AVX512_BW{k}
  ASMJIT_INST_4x(vpcmpd, Vpcmpd, KReg, Vec, Vec, Imm)                  //      AVX512_F{k|b32}
  ASMJIT_INST_4x(vpcmpd, Vpcmpd, KReg, Vec, Mem, Imm)                  //      AVX512_F{k|b32}
  ASMJIT_INST_3x(vpcmpeqb, Vpcmpeqb, Vec, Vec, Vec)                    // AVX+
  ASMJIT_INST_3x(vpcmpeqb, Vpcmpeqb, Vec, Vec, Mem)                    // AVX+
  ASMJIT_INST_3x(vpcmpeqb, Vpcmpeqb, KReg, Vec, Vec)                   //      AVX512_BW{k}
  ASMJIT_INST_3x(vpcmpeqb, Vpcmpeqb, KReg, Vec, Mem)                   //      AVX512_BW{k}
  ASMJIT_INST_3x(vpcmpeqd, Vpcmpeqd, Vec, Vec, Vec)                    // AVX+
  ASMJIT_INST_3x(vpcmpeqd, Vpcmpeqd, Vec, Vec, Mem)                    // AVX+
  ASMJIT_INST_3x(vpcmpeqd, Vpcmpeqd, KReg, Vec, Vec)                   //      AVX512_F{k|b32}
  ASMJIT_INST_3x(vpcmpeqd, Vpcmpeqd, KReg, Vec, Mem)                   //      AVX512_F{k|b32}
  ASMJIT_INST_3x(vpcmpeqq, Vpcmpeqq, Vec, Vec, Vec)                    // AVX+
  ASMJIT_INST_3x(vpcmpeqq, Vpcmpeqq, Vec, Vec, Mem)                    // AVX+
  ASMJIT_INST_3x(vpcmpeqq, Vpcmpeqq, KReg, Vec, Vec)                   //      AVX512_F{k|b64}
  ASMJIT_INST_3x(vpcmpeqq, Vpcmpeqq, KReg, Vec, Mem)                   //      AVX512_F{k|b64}
  ASMJIT_INST_3x(vpcmpeqw, Vpcmpeqw, Vec, Vec, Vec)                    // AVX+
  ASMJIT_INST_3x(vpcmpeqw, Vpcmpeqw, Vec, Vec, Mem)                    // AVX+
  ASMJIT_INST_3x(vpcmpeqw, Vpcmpeqw, KReg, Vec, Vec)                   //      AVX512_BW{k}
  ASMJIT_INST_3x(vpcmpeqw, Vpcmpeqw, KReg, Vec, Mem)                   //      AVX512_BW{k}
  ASMJIT_INST_6x(vpcmpestri, Vpcmpestri, Vec, Vec, Imm, Gp_ECX, Gp_EAX, Gp_EDX) // AVX  [EXPLICIT]
  ASMJIT_INST_6x(vpcmpestri, Vpcmpestri, Vec, Mem, Imm, Gp_ECX, Gp_EAX, Gp_EDX) // AVX  [EXPLICIT]
  ASMJIT_INST_6x(vpcmpestrm, Vpcmpestrm, Vec, Vec, Imm, XMM0, Gp_EAX, Gp_EDX)   // AVX  [EXPLICIT]
  ASMJIT_INST_6x(vpcmpestrm, Vpcmpestrm, Vec, Mem, Imm, XMM0, Gp_EAX, Gp_EDX)   // AVX  [EXPLICIT]
  ASMJIT_INST_3x(vpcmpgtb, Vpcmpgtb, Vec, Vec, Vec)                    // AVX+
  ASMJIT_INST_3x(vpcmpgtb, Vpcmpgtb, Vec, Vec, Mem)                    // AVX+
  ASMJIT_INST_3x(vpcmpgtb, Vpcmpgtb, KReg, Vec, Vec)                   //      AVX512_BW{k}
  ASMJIT_INST_3x(vpcmpgtb, Vpcmpgtb, KReg, Vec, Mem)                   //      AVX512_BW{k}
  ASMJIT_INST_3x(vpcmpgtd, Vpcmpgtd, Vec, Vec, Vec)                    // AVX+
  ASMJIT_INST_3x(vpcmpgtd, Vpcmpgtd, Vec, Vec, Mem)                    // AVX+
  ASMJIT_INST_3x(vpcmpgtd, Vpcmpgtd, KReg, Vec, Vec)                   //      AVX512_F{k|b32}
  ASMJIT_INST_3x(vpcmpgtd, Vpcmpgtd, KReg, Vec, Mem)                   //      AVX512_F{k|b32}
  ASMJIT_INST_3x(vpcmpgtq, Vpcmpgtq, Vec, Vec, Vec)                    // AVX+
  ASMJIT_INST_3x(vpcmpgtq, Vpcmpgtq, Vec, Vec, Mem)                    // AVX+
  ASMJIT_INST_3x(vpcmpgtq, Vpcmpgtq, KReg, Vec, Vec)                   //      AVX512_F{k|b64}
  ASMJIT_INST_3x(vpcmpgtq, Vpcmpgtq, KReg, Vec, Mem)                   //      AVX512_F{k|b64}
  ASMJIT_INST_3x(vpcmpgtw, Vpcmpgtw, Vec, Vec, Vec)                    // AVX+
  ASMJIT_INST_3x(vpcmpgtw, Vpcmpgtw, Vec, Vec, Mem)                    // AVX+
  ASMJIT_INST_3x(vpcmpgtw, Vpcmpgtw, KReg, Vec, Vec)                   //      AVX512_BW{k}
  ASMJIT_INST_3x(vpcmpgtw, Vpcmpgtw, KReg, Vec, Mem)                   //      AVX512_BW{k}
  ASMJIT_INST_4x(vpcmpistri, Vpcmpistri, Vec, Vec, Imm, Gp_ECX)        // AVX  [EXPLICIT]
  ASMJIT_INST_4x(vpcmpistri, Vpcmpistri, Vec, Mem, Imm, Gp_ECX)        // AVX  [EXPLICIT]
  ASMJIT_INST_4x(vpcmpistrm, Vpcmpistrm, Vec, Vec, Imm, XMM0)          // AVX  [EXPLICIT]
  ASMJIT_INST_4x(vpcmpistrm, Vpcmpistrm, Vec, Mem, Imm, XMM0)          // AVX  [EXPLICIT]
  ASMJIT_INST_4x(vpcmpq, Vpcmpq, KReg, Vec, Vec, Imm)                  //      AVX512_F{k|b64}
  ASMJIT_INST_4x(vpcmpq, Vpcmpq, KReg, Vec, Mem, Imm)                  //      AVX512_F{k|b64}
  ASMJIT_INST_4x(vpcmpub, Vpcmpub, KReg, Vec, Vec, Imm)                //      AVX512_BW{k}
  ASMJIT_INST_4x(vpcmpub, Vpcmpub, KReg, Vec, Mem, Imm)                //      AVX512_BW{k}
  ASMJIT_INST_4x(vpcmpud, Vpcmpud, KReg, Vec, Vec, Imm)                //      AVX512_F{k|b32}
  ASMJIT_INST_4x(vpcmpud, Vpcmpud, KReg, Vec, Mem, Imm)                //      AVX512_F{k|b32}
  ASMJIT_INST_4x(vpcmpuq, Vpcmpuq, KReg, Vec, Vec, Imm)                //      AVX512_F{k|b64}
  ASMJIT_INST_4x(vpcmpuq, Vpcmpuq, KReg, Vec, Mem, Imm)                //      AVX512_F{k|b64}
  ASMJIT_INST_4x(vpcmpuw, Vpcmpuw, KReg, Vec, Vec, Imm)                //      AVX512_BW{k|b64}
  ASMJIT_INST_4x(vpcmpuw, Vpcmpuw, KReg, Vec, Mem, Imm)                //      AVX512_BW{k|b64}
  ASMJIT_INST_4x(vpcmpw, Vpcmpw, KReg, Vec, Vec, Imm)                  //      AVX512_BW{k|b64}
  ASMJIT_INST_4x(vpcmpw, Vpcmpw, KReg, Vec, Mem, Imm)                  //      AVX512_BW{k|b64}
  ASMJIT_INST_2x(vpcompressb, Vpcompressb, Vec, Vec)                   //      AVX512_VBMI2{kz}
  ASMJIT_INST_2x(vpcompressb, Vpcompressb, Mem, Vec)                   //      AVX512_VBMI2{kz}
  ASMJIT_INST_2x(vpcompressd, Vpcompressd, Vec, Vec)                   //      AVX512_F{kz}
  ASMJIT_INST_2x(vpcompressd, Vpcompressd, Mem, Vec)                   //      AVX512_F{kz}
  ASMJIT_INST_2x(vpcompressq, Vpcompressq, Vec, Vec)                   //      AVX512_F{kz}
  ASMJIT_INST_2x(vpcompressq, Vpcompressq, Mem, Vec)                   //      AVX512_F{kz}
  ASMJIT_INST_2x(vpcompressw, Vpcompressw, Vec, Vec)                   //      AVX512_VBMI2{kz}
  ASMJIT_INST_2x(vpcompressw, Vpcompressw, Mem, Vec)                   //      AVX512_VBMI2{kz}
  ASMJIT_INST_2x(vpconflictd, Vpconflictd, Vec, Vec)                   //      AVX512_CD{kz|b32}
  ASMJIT_INST_2x(vpconflictd, Vpconflictd, Vec, Mem)                   //      AVX512_CD{kz|b32}
  ASMJIT_INST_2x(vpconflictq, Vpconflictq, Vec, Vec)                   //      AVX512_CD{kz|b32}
  ASMJIT_INST_2x(vpconflictq, Vpconflictq, Vec, Mem)                   //      AVX512_CD{kz|b32}
  ASMJIT_INST_3x(vpdpbusd, Vpdpbusd, Vec, Vec, Vec)                    // AVX_VNNI AVX512_VNNI{kz|b32}
  ASMJIT_INST_3x(vpdpbusd, Vpdpbusd, Vec, Vec, Mem)                    // AVX_VNNI AVX512_VNNI{kz|b32}
  ASMJIT_INST_3x(vpdpbusds, Vpdpbusds, Vec, Vec, Vec)                  // AVX_VNNI AVX512_VNNI{kz|b32}
  ASMJIT_INST_3x(vpdpbusds, Vpdpbusds, Vec, Vec, Mem)                  // AVX_VNNI AVX512_VNNI{kz|b32}
  ASMJIT_INST_3x(vpdpwssd, Vpdpwssd, Vec, Vec, Vec)                    // AVX_VNNI AVX512_VNNI{kz|b32}
  ASMJIT_INST_3x(vpdpwssd, Vpdpwssd, Vec, Vec, Mem)                    // AVX_VNNI AVX512_VNNI{kz|b32}
  ASMJIT_INST_3x(vpdpwssds, Vpdpwssds, Vec, Vec, Vec)                  // AVX_VNNI AVX512_VNNI{kz|b32}
  ASMJIT_INST_3x(vpdpwssds, Vpdpwssds, Vec, Vec, Mem)                  // AVX_VNNI AVX512_VNNI{kz|b32}
  ASMJIT_INST_4x(vperm2f128, Vperm2f128, Vec, Vec, Vec, Imm)           // AVX
  ASMJIT_INST_4x(vperm2f128, Vperm2f128, Vec, Vec, Mem, Imm)           // AVX
  ASMJIT_INST_4x(vperm2i128, Vperm2i128, Vec, Vec, Vec, Imm)           // AVX2
  ASMJIT_INST_4x(vperm2i128, Vperm2i128, Vec, Vec, Mem, Imm)           // AVX2
  ASMJIT_INST_3x(vpermb, Vpermb, Vec, Vec, Vec)                        //      AVX512_VBMI{kz}
  ASMJIT_INST_3x(vpermb, Vpermb, Vec, Vec, Mem)                        //      AVX512_VBMI{kz}
  ASMJIT_INST_3x(vpermd, Vpermd, Vec, Vec, Vec)                        // AVX2 AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpermd, Vpermd, Vec, Vec, Mem)                        // AVX2 AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpermi2b, Vpermi2b, Vec, Vec, Vec)                    //      AVX512_VBMI{kz}
  ASMJIT_INST_3x(vpermi2b, Vpermi2b, Vec, Vec, Mem)                    //      AVX512_VBMI{kz}
  ASMJIT_INST_3x(vpermi2d, Vpermi2d, Vec, Vec, Vec)                    //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpermi2d, Vpermi2d, Vec, Vec, Mem)                    //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpermi2pd, Vpermi2pd, Vec, Vec, Vec)                  //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpermi2pd, Vpermi2pd, Vec, Vec, Mem)                  //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpermi2ps, Vpermi2ps, Vec, Vec, Vec)                  //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpermi2ps, Vpermi2ps, Vec, Vec, Mem)                  //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpermi2q, Vpermi2q, Vec, Vec, Vec)                    //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpermi2q, Vpermi2q, Vec, Vec, Mem)                    //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpermi2w, Vpermi2w, Vec, Vec, Vec)                    //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpermi2w, Vpermi2w, Vec, Vec, Mem)                    //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpermilpd, Vpermilpd, Vec, Vec, Vec)                  // AVX  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpermilpd, Vpermilpd, Vec, Vec, Mem)                  // AVX  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpermilpd, Vpermilpd, Vec, Vec, Imm)                  // AVX  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpermilpd, Vpermilpd, Vec, Mem, Imm)                  // AVX  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpermilps, Vpermilps, Vec, Vec, Vec)                  // AVX  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpermilps, Vpermilps, Vec, Vec, Mem)                  // AVX  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpermilps, Vpermilps, Vec, Vec, Imm)                  // AVX  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpermilps, Vpermilps, Vec, Mem, Imm)                  // AVX  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpermpd, Vpermpd, Vec, Vec, Imm)                      // AVX2
  ASMJIT_INST_3x(vpermpd, Vpermpd, Vec, Mem, Imm)                      // AVX2
  ASMJIT_INST_3x(vpermpd, Vpermpd, Vec, Vec, Vec)                      //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpermpd, Vpermpd, Vec, Vec, Mem)                      //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpermps, Vpermps, Vec, Vec, Vec)                      // AVX2 AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpermps, Vpermps, Vec, Vec, Mem)                      // AVX2 AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpermq, Vpermq, Vec, Vec, Imm)                        // AVX2 AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpermq, Vpermq, Vec, Mem, Imm)                        // AVX2 AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpermq, Vpermq, Vec, Vec, Vec)                        //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpermq, Vpermq, Vec, Vec, Mem)                        //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpermt2b, Vpermt2b, Vec, Vec, Vec)                    //      AVX512_VBMI{kz}
  ASMJIT_INST_3x(vpermt2b, Vpermt2b, Vec, Vec, Mem)                    //      AVX512_VBMI{kz}
  ASMJIT_INST_3x(vpermt2d, Vpermt2d, Vec, Vec, Vec)                    //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpermt2d, Vpermt2d, Vec, Vec, Mem)                    //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpermt2pd, Vpermt2pd, Vec, Vec, Vec)                  //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpermt2pd, Vpermt2pd, Vec, Vec, Mem)                  //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpermt2ps, Vpermt2ps, Vec, Vec, Vec)                  //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpermt2ps, Vpermt2ps, Vec, Vec, Mem)                  //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpermt2q, Vpermt2q, Vec, Vec, Vec)                    //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpermt2q, Vpermt2q, Vec, Vec, Mem)                    //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpermt2w, Vpermt2w, Vec, Vec, Vec)                    //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpermt2w, Vpermt2w, Vec, Vec, Mem)                    //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpermw, Vpermw, Vec, Vec, Vec)                        //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpermw, Vpermw, Vec, Vec, Mem)                        //      AVX512_BW{kz}
  ASMJIT_INST_2x(vpexpandb, Vpexpandb, Vec, Vec)                       //      AVX512_VBMI2{kz}
  ASMJIT_INST_2x(vpexpandb, Vpexpandb, Vec, Mem)                       //      AVX512_VBMI2{kz}
  ASMJIT_INST_2x(vpexpandd, Vpexpandd, Vec, Vec)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpexpandd, Vpexpandd, Vec, Mem)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpexpandq, Vpexpandq, Vec, Vec)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpexpandq, Vpexpandq, Vec, Mem)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpexpandw, Vpexpandw, Vec, Vec)                       //      AVX512_VBMI2{kz}
  ASMJIT_INST_2x(vpexpandw, Vpexpandw, Vec, Mem)                       //      AVX512_VBMI2{kz}
  ASMJIT_INST_3x(vpextrb, Vpextrb, Gp, Vec, Imm)                       // AVX  AVX512_BW
  ASMJIT_INST_3x(vpextrb, Vpextrb, Mem, Vec, Imm)                      // AVX  AVX512_BW
  ASMJIT_INST_3x(vpextrd, Vpextrd, Gp, Vec, Imm)                       // AVX  AVX512_DQ
  ASMJIT_INST_3x(vpextrd, Vpextrd, Mem, Vec, Imm)                      // AVX  AVX512_DQ
  ASMJIT_INST_3x(vpextrq, Vpextrq, Gp, Vec, Imm)                       // AVX  AVX512_DQ
  ASMJIT_INST_3x(vpextrq, Vpextrq, Mem, Vec, Imm)                      // AVX  AVX512_DQ
  ASMJIT_INST_3x(vpextrw, Vpextrw, Gp, Vec, Imm)                       // AVX  AVX512_BW
  ASMJIT_INST_3x(vpextrw, Vpextrw, Mem, Vec, Imm)                      // AVX  AVX512_BW
  ASMJIT_INST_2x(vpgatherdd, Vpgatherdd, Vec, Mem)                     //      AVX512_F{k}
  ASMJIT_INST_3x(vpgatherdd, Vpgatherdd, Vec, Mem, Vec)                // AVX2
  ASMJIT_INST_2x(vpgatherdq, Vpgatherdq, Vec, Mem)                     //      AVX512_F{k}
  ASMJIT_INST_3x(vpgatherdq, Vpgatherdq, Vec, Mem, Vec)                // AVX2
  ASMJIT_INST_2x(vpgatherqd, Vpgatherqd, Vec, Mem)                     //      AVX512_F{k}
  ASMJIT_INST_3x(vpgatherqd, Vpgatherqd, Vec, Mem, Vec)                // AVX2
  ASMJIT_INST_2x(vpgatherqq, Vpgatherqq, Vec, Mem)                     //      AVX512_F{k}
  ASMJIT_INST_3x(vpgatherqq, Vpgatherqq, Vec, Mem, Vec)                // AVX2
  ASMJIT_INST_3x(vphaddd, Vphaddd, Vec, Vec, Vec)                      // AVX+
  ASMJIT_INST_3x(vphaddd, Vphaddd, Vec, Vec, Mem)                      // AVX+
  ASMJIT_INST_3x(vphaddsw, Vphaddsw, Vec, Vec, Vec)                    // AVX+
  ASMJIT_INST_3x(vphaddsw, Vphaddsw, Vec, Vec, Mem)                    // AVX+
  ASMJIT_INST_3x(vphaddw, Vphaddw, Vec, Vec, Vec)                      // AVX+
  ASMJIT_INST_3x(vphaddw, Vphaddw, Vec, Vec, Mem)                      // AVX+
  ASMJIT_INST_2x(vphminposuw, Vphminposuw, Vec, Vec)                   // AVX
  ASMJIT_INST_2x(vphminposuw, Vphminposuw, Vec, Mem)                   // AVX
  ASMJIT_INST_3x(vphsubd, Vphsubd, Vec, Vec, Vec)                      // AVX+
  ASMJIT_INST_3x(vphsubd, Vphsubd, Vec, Vec, Mem)                      // AVX+
  ASMJIT_INST_3x(vphsubsw, Vphsubsw, Vec, Vec, Vec)                    // AVX+
  ASMJIT_INST_3x(vphsubsw, Vphsubsw, Vec, Vec, Mem)                    // AVX+
  ASMJIT_INST_3x(vphsubw, Vphsubw, Vec, Vec, Vec)                      // AVX+
  ASMJIT_INST_3x(vphsubw, Vphsubw, Vec, Vec, Mem)                      // AVX+
  ASMJIT_INST_4x(vpinsrb, Vpinsrb, Vec, Vec, Gp, Imm)                  // AVX  AVX512_BW{kz}
  ASMJIT_INST_4x(vpinsrb, Vpinsrb, Vec, Vec, Mem, Imm)                 // AVX  AVX512_BW{kz}
  ASMJIT_INST_4x(vpinsrd, Vpinsrd, Vec, Vec, Gp, Imm)                  // AVX  AVX512_DQ{kz}
  ASMJIT_INST_4x(vpinsrd, Vpinsrd, Vec, Vec, Mem, Imm)                 // AVX  AVX512_DQ{kz}
  ASMJIT_INST_4x(vpinsrq, Vpinsrq, Vec, Vec, Gp, Imm)                  // AVX  AVX512_DQ{kz}
  ASMJIT_INST_4x(vpinsrq, Vpinsrq, Vec, Vec, Mem, Imm)                 // AVX  AVX512_DQ{kz}
  ASMJIT_INST_4x(vpinsrw, Vpinsrw, Vec, Vec, Gp, Imm)                  // AVX  AVX512_BW{kz}
  ASMJIT_INST_4x(vpinsrw, Vpinsrw, Vec, Vec, Mem, Imm)                 // AVX  AVX512_BW{kz}
  ASMJIT_INST_2x(vplzcntd, Vplzcntd, Vec, Vec)                         //      AVX512_CD{kz|b32}
  ASMJIT_INST_2x(vplzcntd, Vplzcntd, Vec, Mem)                         //      AVX512_CD{kz|b32}
  ASMJIT_INST_2x(vplzcntq, Vplzcntq, Vec, Vec)                         //      AVX512_CD{kz|b64}
  ASMJIT_INST_2x(vplzcntq, Vplzcntq, Vec, Mem)                         //      AVX512_CD{kz|b64}
  ASMJIT_INST_3x(vpmadd52huq, Vpmadd52huq, Vec, Vec, Vec)              //      AVX512_IFMA{kz|b64}
  ASMJIT_INST_3x(vpmadd52huq, Vpmadd52huq, Vec, Vec, Mem)              //      AVX512_IFMA{kz|b64}
  ASMJIT_INST_3x(vpmadd52luq, Vpmadd52luq, Vec, Vec, Vec)              //      AVX512_IFMA{kz|b64}
  ASMJIT_INST_3x(vpmadd52luq, Vpmadd52luq, Vec, Vec, Mem)              //      AVX512_IFMA{kz|b64}
  ASMJIT_INST_3x(vpmaddubsw, Vpmaddubsw, Vec, Vec, Vec)                // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpmaddubsw, Vpmaddubsw, Vec, Vec, Mem)                // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpmaddwd, Vpmaddwd, Vec, Vec, Vec)                    // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpmaddwd, Vpmaddwd, Vec, Vec, Mem)                    // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpmaskmovd, Vpmaskmovd, Mem, Vec, Vec)                // AVX2
  ASMJIT_INST_3x(vpmaskmovd, Vpmaskmovd, Vec, Vec, Mem)                // AVX2
  ASMJIT_INST_3x(vpmaskmovq, Vpmaskmovq, Mem, Vec, Vec)                // AVX2
  ASMJIT_INST_3x(vpmaskmovq, Vpmaskmovq, Vec, Vec, Mem)                // AVX2
  ASMJIT_INST_3x(vpmaxsb, Vpmaxsb, Vec, Vec, Vec)                      // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpmaxsb, Vpmaxsb, Vec, Vec, Mem)                      // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpmaxsd, Vpmaxsd, Vec, Vec, Vec)                      // AVX+ AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpmaxsd, Vpmaxsd, Vec, Vec, Mem)                      // AVX+ AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpmaxsq, Vpmaxsq, Vec, Vec, Vec)                      //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpmaxsq, Vpmaxsq, Vec, Vec, Mem)                      //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpmaxsw, Vpmaxsw, Vec, Vec, Vec)                      // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpmaxsw, Vpmaxsw, Vec, Vec, Mem)                      // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpmaxub, Vpmaxub, Vec, Vec, Vec)                      // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpmaxub, Vpmaxub, Vec, Vec, Mem)                      // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpmaxud, Vpmaxud, Vec, Vec, Vec)                      // AVX+ AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpmaxud, Vpmaxud, Vec, Vec, Mem)                      // AVX+ AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpmaxuq, Vpmaxuq, Vec, Vec, Vec)                      //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpmaxuq, Vpmaxuq, Vec, Vec, Mem)                      //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpmaxuw, Vpmaxuw, Vec, Vec, Vec)                      // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpmaxuw, Vpmaxuw, Vec, Vec, Mem)                      // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpminsb, Vpminsb, Vec, Vec, Vec)                      // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpminsb, Vpminsb, Vec, Vec, Mem)                      // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpminsd, Vpminsd, Vec, Vec, Vec)                      // AVX+ AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpminsd, Vpminsd, Vec, Vec, Mem)                      // AVX+ AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpminsq, Vpminsq, Vec, Vec, Vec)                      //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpminsq, Vpminsq, Vec, Vec, Mem)                      //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpminsw, Vpminsw, Vec, Vec, Vec)                      // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpminsw, Vpminsw, Vec, Vec, Mem)                      // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpminub, Vpminub, Vec, Vec, Vec)                      // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpminub, Vpminub, Vec, Vec, Mem)                      // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpminud, Vpminud, Vec, Vec, Vec)                      // AVX+ AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpminud, Vpminud, Vec, Vec, Mem)                      // AVX+ AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpminuq, Vpminuq, Vec, Vec, Vec)                      //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpminuq, Vpminuq, Vec, Vec, Mem)                      //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpminuw, Vpminuw, Vec, Vec, Vec)                      // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpminuw, Vpminuw, Vec, Vec, Mem)                      // AVX+ AVX512_BW{kz}
  ASMJIT_INST_2x(vpmovb2m, Vpmovb2m, KReg, Vec)                        //      AVX512_BW
  ASMJIT_INST_2x(vpmovd2m, Vpmovd2m, KReg, Vec)                        //      AVX512_DQ
  ASMJIT_INST_2x(vpmovdb, Vpmovdb, Vec, Vec)                           //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovdb, Vpmovdb, Mem, Vec)                           //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovdw, Vpmovdw, Vec, Vec)                           //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovdw, Vpmovdw, Mem, Vec)                           //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovm2b, Vpmovm2b, Vec, KReg)                        //      AVX512_BW
  ASMJIT_INST_2x(vpmovm2d, Vpmovm2d, Vec, KReg)                        //      AVX512_DQ
  ASMJIT_INST_2x(vpmovm2q, Vpmovm2q, Vec, KReg)                        //      AVX512_DQ
  ASMJIT_INST_2x(vpmovm2w, Vpmovm2w, Vec, KReg)                        //      AVX512_BW
  ASMJIT_INST_2x(vpmovmskb, Vpmovmskb, Gp, Vec)                        // AVX+
  ASMJIT_INST_2x(vpmovq2m, Vpmovq2m, KReg, Vec)                        //      AVX512_DQ
  ASMJIT_INST_2x(vpmovqb, Vpmovqb, Vec, Vec)                           //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovqb, Vpmovqb, Mem, Vec)                           //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovqd, Vpmovqd, Vec, Vec)                           //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovqd, Vpmovqd, Mem, Vec)                           //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovqw, Vpmovqw, Vec, Vec)                           //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovqw, Vpmovqw, Mem, Vec)                           //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovsdb, Vpmovsdb, Vec, Vec)                         //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovsdb, Vpmovsdb, Mem, Vec)                         //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovsdw, Vpmovsdw, Vec, Vec)                         //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovsdw, Vpmovsdw, Mem, Vec)                         //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovsqb, Vpmovsqb, Vec, Vec)                         //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovsqb, Vpmovsqb, Mem, Vec)                         //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovsqd, Vpmovsqd, Vec, Vec)                         //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovsqd, Vpmovsqd, Mem, Vec)                         //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovsqw, Vpmovsqw, Vec, Vec)                         //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovsqw, Vpmovsqw, Mem, Vec)                         //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovswb, Vpmovswb, Vec, Vec)                         //      AVX512_BW{kz}
  ASMJIT_INST_2x(vpmovswb, Vpmovswb, Mem, Vec)                         //      AVX512_BW{kz}
  ASMJIT_INST_2x(vpmovsxbd, Vpmovsxbd, Vec, Vec)                       // AVX+ AVX512_F{kz}
  ASMJIT_INST_2x(vpmovsxbd, Vpmovsxbd, Vec, Mem)                       // AVX+ AVX512_F{kz}
  ASMJIT_INST_2x(vpmovsxbq, Vpmovsxbq, Vec, Vec)                       // AVX+ AVX512_F{kz}
  ASMJIT_INST_2x(vpmovsxbq, Vpmovsxbq, Vec, Mem)                       // AVX+ AVX512_F{kz}
  ASMJIT_INST_2x(vpmovsxbw, Vpmovsxbw, Vec, Vec)                       // AVX+ AVX512_BW{kz}
  ASMJIT_INST_2x(vpmovsxbw, Vpmovsxbw, Vec, Mem)                       // AVX+ AVX512_BW{kz}
  ASMJIT_INST_2x(vpmovsxdq, Vpmovsxdq, Vec, Vec)                       // AVX+ AVX512_F{kz}
  ASMJIT_INST_2x(vpmovsxdq, Vpmovsxdq, Vec, Mem)                       // AVX+ AVX512_F{kz}
  ASMJIT_INST_2x(vpmovsxwd, Vpmovsxwd, Vec, Vec)                       // AVX+ AVX512_F{kz}
  ASMJIT_INST_2x(vpmovsxwd, Vpmovsxwd, Vec, Mem)                       // AVX+ AVX512_F{kz}
  ASMJIT_INST_2x(vpmovsxwq, Vpmovsxwq, Vec, Vec)                       // AVX+ AVX512_F{kz}
  ASMJIT_INST_2x(vpmovsxwq, Vpmovsxwq, Vec, Mem)                       // AVX+ AVX512_F{kz}
  ASMJIT_INST_2x(vpmovusdb, Vpmovusdb, Vec, Vec)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovusdb, Vpmovusdb, Mem, Vec)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovusdw, Vpmovusdw, Vec, Vec)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovusdw, Vpmovusdw, Mem, Vec)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovusqb, Vpmovusqb, Vec, Vec)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovusqb, Vpmovusqb, Mem, Vec)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovusqd, Vpmovusqd, Vec, Vec)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovusqd, Vpmovusqd, Mem, Vec)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovusqw, Vpmovusqw, Vec, Vec)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovusqw, Vpmovusqw, Mem, Vec)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovuswb, Vpmovuswb, Vec, Vec)                       //      AVX512_BW{kz}
  ASMJIT_INST_2x(vpmovuswb, Vpmovuswb, Mem, Vec)                       //      AVX512_BW{kz}
  ASMJIT_INST_2x(vpmovw2m, Vpmovw2m, KReg, Vec)                        //      AVX512_BW
  ASMJIT_INST_2x(vpmovwb, Vpmovwb, Vec, Vec)                           //      AVX512_BW{kz}
  ASMJIT_INST_2x(vpmovwb, Vpmovwb, Mem, Vec)                           //      AVX512_BW{kz}
  ASMJIT_INST_2x(vpmovzxbd, Vpmovzxbd, Vec, Vec)                       // AVX+ AVX512_F{kz}
  ASMJIT_INST_2x(vpmovzxbd, Vpmovzxbd, Vec, Mem)                       // AVX+ AVX512_F{kz}
  ASMJIT_INST_2x(vpmovzxbq, Vpmovzxbq, Vec, Vec)                       // AVX+ AVX512_F{kz}
  ASMJIT_INST_2x(vpmovzxbq, Vpmovzxbq, Vec, Mem)                       // AVX+ AVX512_F{kz}
  ASMJIT_INST_2x(vpmovzxbw, Vpmovzxbw, Vec, Vec)                       // AVX+ AVX512_BW{kz}
  ASMJIT_INST_2x(vpmovzxbw, Vpmovzxbw, Vec, Mem)                       // AVX+ AVX512_BW{kz}
  ASMJIT_INST_2x(vpmovzxdq, Vpmovzxdq, Vec, Vec)                       // AVX+ AVX512_F{kz}
  ASMJIT_INST_2x(vpmovzxdq, Vpmovzxdq, Vec, Mem)                       // AVX+ AVX512_F{kz}
  ASMJIT_INST_2x(vpmovzxwd, Vpmovzxwd, Vec, Vec)                       // AVX+ AVX512_F{kz}
  ASMJIT_INST_2x(vpmovzxwd, Vpmovzxwd, Vec, Mem)                       // AVX+ AVX512_F{kz}
  ASMJIT_INST_2x(vpmovzxwq, Vpmovzxwq, Vec, Vec)                       // AVX+ AVX512_F{kz}
  ASMJIT_INST_2x(vpmovzxwq, Vpmovzxwq, Vec, Mem)                       // AVX+ AVX512_F{kz}
  ASMJIT_INST_3x(vpmuldq, Vpmuldq, Vec, Vec, Vec)                      // AVX  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpmuldq, Vpmuldq, Vec, Vec, Mem)                      // AVX  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpmulhrsw, Vpmulhrsw, Vec, Vec, Vec)                  // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpmulhrsw, Vpmulhrsw, Vec, Vec, Mem)                  // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpmulhuw, Vpmulhuw, Vec, Vec, Vec)                    // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpmulhuw, Vpmulhuw, Vec, Vec, Mem)                    // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpmulhw, Vpmulhw, Vec, Vec, Vec)                      // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpmulhw, Vpmulhw, Vec, Vec, Mem)                      // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpmulld, Vpmulld, Vec, Vec, Vec)                      // AVX+ AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpmulld, Vpmulld, Vec, Vec, Mem)                      // AVX+ AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpmullq, Vpmullq, Vec, Vec, Vec)                      //      AVX512_DQ{kz|b64}
  ASMJIT_INST_3x(vpmullq, Vpmullq, Vec, Vec, Mem)                      //      AVX512_DQ{kz|b64}
  ASMJIT_INST_3x(vpmullw, Vpmullw, Vec, Vec, Vec)                      // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpmullw, Vpmullw, Vec, Vec, Mem)                      // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpmultishiftqb, Vpmultishiftqb, Vec, Vec, Vec)        //      AVX512_VBMI{kz|b64}
  ASMJIT_INST_3x(vpmultishiftqb, Vpmultishiftqb, Vec, Vec, Mem)        //      AVX512_VBMI{kz|b64}
  ASMJIT_INST_3x(vpmuludq, Vpmuludq, Vec, Vec, Vec)                    // AVX+ AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpmuludq, Vpmuludq, Vec, Vec, Mem)                    // AVX+ AVX512_F{kz|b64}
  ASMJIT_INST_2x(vpopcntb, Vpopcntb, Vec, Vec)                         //      AVX512_BITALG{kz|b32}
  ASMJIT_INST_2x(vpopcntb, Vpopcntb, Vec, Mem)                         //      AVX512_BITALG{kz|b32}
  ASMJIT_INST_2x(vpopcntd, Vpopcntd, Vec, Vec)                         //      AVX512_VPOPCNTDQ{kz|b32}
  ASMJIT_INST_2x(vpopcntd, Vpopcntd, Vec, Mem)                         //      AVX512_VPOPCNTDQ{kz|b32}
  ASMJIT_INST_2x(vpopcntq, Vpopcntq, Vec, Vec)                         //      AVX512_VPOPCNTDQ{kz|b64}
  ASMJIT_INST_2x(vpopcntq, Vpopcntq, Vec, Mem)                         //      AVX512_VPOPCNTDQ{kz|b64}
  ASMJIT_INST_2x(vpopcntw, Vpopcntw, Vec, Vec)                         //      AVX512_BITALG{kz|b32}
  ASMJIT_INST_2x(vpopcntw, Vpopcntw, Vec, Mem)                         //      AVX512_BITALG{kz|b32}
  ASMJIT_INST_3x(vpor, Vpor, Vec, Vec, Vec)                            // AVX+
  ASMJIT_INST_3x(vpor, Vpor, Vec, Vec, Mem)                            // AVX+
  ASMJIT_INST_3x(vpord, Vpord, Vec, Vec, Vec)                          //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpord, Vpord, Vec, Vec, Mem)                          //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vporq, Vporq, Vec, Vec, Vec)                          //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vporq, Vporq, Vec, Vec, Mem)                          //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vprold, Vprold, Vec, Vec, Imm)                        //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vprold, Vprold, Vec, Mem, Imm)                        //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vprolq, Vprolq, Vec, Vec, Imm)                        //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vprolq, Vprolq, Vec, Mem, Imm)                        //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vprolvd, Vprolvd, Vec, Vec, Vec)                      //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vprolvd, Vprolvd, Vec, Vec, Mem)                      //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vprolvq, Vprolvq, Vec, Vec, Vec)                      //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vprolvq, Vprolvq, Vec, Vec, Mem)                      //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vprord, Vprord, Vec, Vec, Imm)                        //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vprord, Vprord, Vec, Mem, Imm)                        //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vprorq, Vprorq, Vec, Vec, Imm)                        //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vprorq, Vprorq, Vec, Mem, Imm)                        //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vprorvd, Vprorvd, Vec, Vec, Vec)                      //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vprorvd, Vprorvd, Vec, Vec, Mem)                      //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vprorvq, Vprorvq, Vec, Vec, Vec)                      //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vprorvq, Vprorvq, Vec, Vec, Mem)                      //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpsadbw, Vpsadbw, Vec, Vec, Vec)                      // AVX+ AVX512_BW
  ASMJIT_INST_3x(vpsadbw, Vpsadbw, Vec, Vec, Mem)                      // AVX+ AVX512_BW
  ASMJIT_INST_2x(vpscatterdd, Vpscatterdd, Mem, Vec)                   //      AVX512_F{k}
  ASMJIT_INST_2x(vpscatterdq, Vpscatterdq, Mem, Vec)                   //      AVX512_F{k}
  ASMJIT_INST_2x(vpscatterqd, Vpscatterqd, Mem, Vec)                   //      AVX512_F{k}
  ASMJIT_INST_2x(vpscatterqq, Vpscatterqq, Mem, Vec)                   //      AVX512_F{k}
  ASMJIT_INST_4x(vpshldd, Vpshldd, Vec, Vec, Vec, Imm)                 //      AVX512_VBMI2{kz}
  ASMJIT_INST_4x(vpshldd, Vpshldd, Vec, Vec, Mem, Imm)                 //      AVX512_VBMI2{kz}
  ASMJIT_INST_4x(vpshldq, Vpshldq, Vec, Vec, Vec, Imm)                 //      AVX512_VBMI2{kz}
  ASMJIT_INST_4x(vpshldq, Vpshldq, Vec, Vec, Mem, Imm)                 //      AVX512_VBMI2{kz}
  ASMJIT_INST_3x(vpshldvd, Vpshldvd, Vec, Vec, Vec)                    //      AVX512_VBMI2{kz}
  ASMJIT_INST_3x(vpshldvd, Vpshldvd, Vec, Vec, Mem)                    //      AVX512_VBMI2{kz}
  ASMJIT_INST_3x(vpshldvq, Vpshldvq, Vec, Vec, Vec)                    //      AVX512_VBMI2{kz}
  ASMJIT_INST_3x(vpshldvq, Vpshldvq, Vec, Vec, Mem)                    //      AVX512_VBMI2{kz}
  ASMJIT_INST_3x(vpshldvw, Vpshldvw, Vec, Vec, Vec)                    //      AVX512_VBMI2{kz}
  ASMJIT_INST_3x(vpshldvw, Vpshldvw, Vec, Vec, Mem)                    //      AVX512_VBMI2{kz}
  ASMJIT_INST_4x(vpshldw, Vpshldw, Vec, Vec, Vec, Imm)                 //      AVX512_VBMI2{kz}
  ASMJIT_INST_4x(vpshldw, Vpshldw, Vec, Vec, Mem, Imm)                 //      AVX512_VBMI2{kz}
  ASMJIT_INST_4x(vpshrdd, Vpshrdd, Vec, Vec, Vec, Imm)                 //      AVX512_VBMI2{kz}
  ASMJIT_INST_4x(vpshrdd, Vpshrdd, Vec, Vec, Mem, Imm)                 //      AVX512_VBMI2{kz}
  ASMJIT_INST_4x(vpshrdq, Vpshrdq, Vec, Vec, Vec, Imm)                 //      AVX512_VBMI2{kz}
  ASMJIT_INST_4x(vpshrdq, Vpshrdq, Vec, Vec, Mem, Imm)                 //      AVX512_VBMI2{kz}
  ASMJIT_INST_3x(vpshrdvd, Vpshrdvd, Vec, Vec, Vec)                    //      AVX512_VBMI2{kz}
  ASMJIT_INST_3x(vpshrdvd, Vpshrdvd, Vec, Vec, Mem)                    //      AVX512_VBMI2{kz}
  ASMJIT_INST_3x(vpshrdvq, Vpshrdvq, Vec, Vec, Vec)                    //      AVX512_VBMI2{kz}
  ASMJIT_INST_3x(vpshrdvq, Vpshrdvq, Vec, Vec, Mem)                    //      AVX512_VBMI2{kz}
  ASMJIT_INST_3x(vpshrdvw, Vpshrdvw, Vec, Vec, Vec)                    //      AVX512_VBMI2{kz}
  ASMJIT_INST_3x(vpshrdvw, Vpshrdvw, Vec, Vec, Mem)                    //      AVX512_VBMI2{kz}
  ASMJIT_INST_4x(vpshrdw, Vpshrdw, Vec, Vec, Vec, Imm)                 //      AVX512_VBMI2{kz}
  ASMJIT_INST_4x(vpshrdw, Vpshrdw, Vec, Vec, Mem, Imm)                 //      AVX512_VBMI2{kz}
  ASMJIT_INST_3x(vpshufb, Vpshufb, Vec, Vec, Vec)                      // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpshufb, Vpshufb, Vec, Vec, Mem)                      // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpshufbitqmb, Vpshufbitqmb, KReg, Vec, Vec)           //      AVX512_BITALG{k}
  ASMJIT_INST_3x(vpshufbitqmb, Vpshufbitqmb, KReg, Vec, Mem)           //      AVX512_BITALG{k}
  ASMJIT_INST_3x(vpshufd, Vpshufd, Vec, Vec, Imm)                      // AVX+ AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpshufd, Vpshufd, Vec, Mem, Imm)                      // AVX+ AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpshufhw, Vpshufhw, Vec, Vec, Imm)                    // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpshufhw, Vpshufhw, Vec, Mem, Imm)                    // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpshuflw, Vpshuflw, Vec, Vec, Imm)                    // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpshuflw, Vpshuflw, Vec, Mem, Imm)                    // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpsignb, Vpsignb, Vec, Vec, Vec)                      // AVX+
  ASMJIT_INST_3x(vpsignb, Vpsignb, Vec, Vec, Mem)                      // AVX+
  ASMJIT_INST_3x(vpsignd, Vpsignd, Vec, Vec, Vec)                      // AVX+
  ASMJIT_INST_3x(vpsignd, Vpsignd, Vec, Vec, Mem)                      // AVX+
  ASMJIT_INST_3x(vpsignw, Vpsignw, Vec, Vec, Vec)                      // AVX+
  ASMJIT_INST_3x(vpsignw, Vpsignw, Vec, Vec, Mem)                      // AVX+
  ASMJIT_INST_3x(vpslld, Vpslld, Vec, Vec, Imm)                        // AVX+ AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpslld, Vpslld, Vec, Vec, Vec)                        // AVX+ AVX512_F{kz}
  ASMJIT_INST_3x(vpslld, Vpslld, Vec, Vec, Mem)                        // AVX+ AVX512_F{kz}
  ASMJIT_INST_3x(vpslld, Vpslld, Vec, Mem, Imm)                        //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpslldq, Vpslldq, Vec, Vec, Imm)                      // AVX+ AVX512_BW
  ASMJIT_INST_3x(vpslldq, Vpslldq, Vec, Mem, Imm)                      //      AVX512_BW
  ASMJIT_INST_3x(vpsllq, Vpsllq, Vec, Vec, Imm)                        // AVX+ AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpsllq, Vpsllq, Vec, Vec, Vec)                        // AVX+ AVX512_F{kz}
  ASMJIT_INST_3x(vpsllq, Vpsllq, Vec, Vec, Mem)                        // AVX+ AVX512_F{kz}
  ASMJIT_INST_3x(vpsllq, Vpsllq, Vec, Mem, Imm)                        //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpsllvd, Vpsllvd, Vec, Vec, Vec)                      // AVX2 AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpsllvd, Vpsllvd, Vec, Vec, Mem)                      // AVX2 AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpsllvq, Vpsllvq, Vec, Vec, Vec)                      // AVX2 AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpsllvq, Vpsllvq, Vec, Vec, Mem)                      // AVX2 AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpsllvw, Vpsllvw, Vec, Vec, Vec)                      //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpsllvw, Vpsllvw, Vec, Vec, Mem)                      //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpsllw, Vpsllw, Vec, Vec, Imm)                        // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpsllw, Vpsllw, Vec, Vec, Vec)                        // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpsllw, Vpsllw, Vec, Vec, Mem)                        // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpsllw, Vpsllw, Vec, Mem, Imm)                        //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpsrad, Vpsrad, Vec, Vec, Imm)                        // AVX+ AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpsrad, Vpsrad, Vec, Vec, Vec)                        // AVX+ AVX512_F{kz}
  ASMJIT_INST_3x(vpsrad, Vpsrad, Vec, Vec, Mem)                        // AVX+ AVX512_F{kz}
  ASMJIT_INST_3x(vpsrad, Vpsrad, Vec, Mem, Imm)                        //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpsraq, Vpsraq, Vec, Vec, Vec)                        //      AVX512_F{kz}
  ASMJIT_INST_3x(vpsraq, Vpsraq, Vec, Vec, Mem)                        //      AVX512_F{kz}
  ASMJIT_INST_3x(vpsraq, Vpsraq, Vec, Vec, Imm)                        //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpsraq, Vpsraq, Vec, Mem, Imm)                        //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpsravd, Vpsravd, Vec, Vec, Vec)                      // AVX2 AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpsravd, Vpsravd, Vec, Vec, Mem)                      // AVX2 AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpsravq, Vpsravq, Vec, Vec, Vec)                      //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpsravq, Vpsravq, Vec, Vec, Mem)                      //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpsravw, Vpsravw, Vec, Vec, Vec)                      //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpsravw, Vpsravw, Vec, Vec, Mem)                      //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpsraw, Vpsraw, Vec, Vec, Imm)                        // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpsraw, Vpsraw, Vec, Vec, Vec)                        // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpsraw, Vpsraw, Vec, Vec, Mem)                        // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpsraw, Vpsraw, Vec, Mem, Imm)                        //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpsrld, Vpsrld, Vec, Vec, Imm)                        // AVX+ AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpsrld, Vpsrld, Vec, Vec, Vec)                        // AVX+ AVX512_F{kz}
  ASMJIT_INST_3x(vpsrld, Vpsrld, Vec, Vec, Mem)                        // AVX+ AVX512_F{kz}
  ASMJIT_INST_3x(vpsrld, Vpsrld, Vec, Mem, Imm)                        //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpsrldq, Vpsrldq, Vec, Vec, Imm)                      // AVX+ AVX512_BW
  ASMJIT_INST_3x(vpsrldq, Vpsrldq, Vec, Mem, Imm)                      //      AVX512_BW
  ASMJIT_INST_3x(vpsrlq, Vpsrlq, Vec, Vec, Imm)                        // AVX  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpsrlq, Vpsrlq, Vec, Vec, Vec)                        // AVX  AVX512_F{kz}
  ASMJIT_INST_3x(vpsrlq, Vpsrlq, Vec, Vec, Mem)                        // AVX  AVX512_F{kz}
  ASMJIT_INST_3x(vpsrlq, Vpsrlq, Vec, Mem, Imm)                        //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpsrlvd, Vpsrlvd, Vec, Vec, Vec)                      // AVX2 AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpsrlvd, Vpsrlvd, Vec, Vec, Mem)                      // AVX2 AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpsrlvq, Vpsrlvq, Vec, Vec, Vec)                      // AVX2 AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpsrlvq, Vpsrlvq, Vec, Vec, Mem)                      // AVX2 AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpsrlvw, Vpsrlvw, Vec, Vec, Vec)                      //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpsrlvw, Vpsrlvw, Vec, Vec, Mem)                      //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpsrlw, Vpsrlw, Vec, Vec, Imm)                        // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpsrlw, Vpsrlw, Vec, Vec, Vec)                        // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpsrlw, Vpsrlw, Vec, Vec, Mem)                        // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpsrlw, Vpsrlw, Vec, Mem, Imm)                        //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpsubb, Vpsubb, Vec, Vec, Vec)                        // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpsubb, Vpsubb, Vec, Vec, Mem)                        // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpsubd, Vpsubd, Vec, Vec, Vec)                        // AVX+ AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpsubd, Vpsubd, Vec, Vec, Mem)                        // AVX+ AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpsubq, Vpsubq, Vec, Vec, Vec)                        // AVX+ AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpsubq, Vpsubq, Vec, Vec, Mem)                        // AVX+ AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpsubsb, Vpsubsb, Vec, Vec, Vec)                      // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpsubsb, Vpsubsb, Vec, Vec, Mem)                      // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpsubsw, Vpsubsw, Vec, Vec, Vec)                      // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpsubsw, Vpsubsw, Vec, Vec, Mem)                      // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpsubusb, Vpsubusb, Vec, Vec, Vec)                    // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpsubusb, Vpsubusb, Vec, Vec, Mem)                    // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpsubusw, Vpsubusw, Vec, Vec, Vec)                    // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpsubusw, Vpsubusw, Vec, Vec, Mem)                    // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpsubw, Vpsubw, Vec, Vec, Vec)                        // AVX  AVX512_BW{kz}
  ASMJIT_INST_3x(vpsubw, Vpsubw, Vec, Vec, Mem)                        // AVX  AVX512_BW{kz}
  ASMJIT_INST_4x(vpternlogd, Vpternlogd, Vec, Vec, Vec, Imm)           //      AVX512_F{kz|b32}
  ASMJIT_INST_4x(vpternlogd, Vpternlogd, Vec, Vec, Mem, Imm)           //      AVX512_F{kz|b32}
  ASMJIT_INST_4x(vpternlogq, Vpternlogq, Vec, Vec, Vec, Imm)           //      AVX512_F{kz|b64}
  ASMJIT_INST_4x(vpternlogq, Vpternlogq, Vec, Vec, Mem, Imm)           //      AVX512_F{kz|b64}
  ASMJIT_INST_2x(vptest, Vptest, Vec, Vec)                             // AVX
  ASMJIT_INST_2x(vptest, Vptest, Vec, Mem)                             // AVX
  ASMJIT_INST_3x(vptestmb, Vptestmb, KReg, Vec, Vec)                   //      AVX512_BW{k}
  ASMJIT_INST_3x(vptestmb, Vptestmb, KReg, Vec, Mem)                   //      AVX512_BW{k}
  ASMJIT_INST_3x(vptestmd, Vptestmd, KReg, Vec, Vec)                   //      AVX512_F{k|b32}
  ASMJIT_INST_3x(vptestmd, Vptestmd, KReg, Vec, Mem)                   //      AVX512_F{k|b32}
  ASMJIT_INST_3x(vptestmq, Vptestmq, KReg, Vec, Vec)                   //      AVX512_F{k|b64}
  ASMJIT_INST_3x(vptestmq, Vptestmq, KReg, Vec, Mem)                   //      AVX512_F{k|b64}
  ASMJIT_INST_3x(vptestmw, Vptestmw, KReg, Vec, Vec)                   //      AVX512_BW{k}
  ASMJIT_INST_3x(vptestmw, Vptestmw, KReg, Vec, Mem)                   //      AVX512_BW{k}
  ASMJIT_INST_3x(vptestnmb, Vptestnmb, KReg, Vec, Vec)                 //      AVX512_BW{k}
  ASMJIT_INST_3x(vptestnmb, Vptestnmb, KReg, Vec, Mem)                 //      AVX512_BW{k}
  ASMJIT_INST_3x(vptestnmd, Vptestnmd, KReg, Vec, Vec)                 //      AVX512_F{k|b32}
  ASMJIT_INST_3x(vptestnmd, Vptestnmd, KReg, Vec, Mem)                 //      AVX512_F{k|b32}
  ASMJIT_INST_3x(vptestnmq, Vptestnmq, KReg, Vec, Vec)                 //      AVX512_F{k|b64}
  ASMJIT_INST_3x(vptestnmq, Vptestnmq, KReg, Vec, Mem)                 //      AVX512_F{k|b64}
  ASMJIT_INST_3x(vptestnmw, Vptestnmw, KReg, Vec, Vec)                 //      AVX512_BW{k}
  ASMJIT_INST_3x(vptestnmw, Vptestnmw, KReg, Vec, Mem)                 //      AVX512_BW{k}
  ASMJIT_INST_3x(vpunpckhbw, Vpunpckhbw, Vec, Vec, Vec)                // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpunpckhbw, Vpunpckhbw, Vec, Vec, Mem)                // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpunpckhdq, Vpunpckhdq, Vec, Vec, Vec)                // AVX+ AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpunpckhdq, Vpunpckhdq, Vec, Vec, Mem)                // AVX+ AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpunpckhqdq, Vpunpckhqdq, Vec, Vec, Vec)              // AVX+ AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpunpckhqdq, Vpunpckhqdq, Vec, Vec, Mem)              // AVX+ AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpunpckhwd, Vpunpckhwd, Vec, Vec, Vec)                // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpunpckhwd, Vpunpckhwd, Vec, Vec, Mem)                // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpunpcklbw, Vpunpcklbw, Vec, Vec, Vec)                // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpunpcklbw, Vpunpcklbw, Vec, Vec, Mem)                // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpunpckldq, Vpunpckldq, Vec, Vec, Vec)                // AVX+ AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpunpckldq, Vpunpckldq, Vec, Vec, Mem)                // AVX+ AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpunpcklqdq, Vpunpcklqdq, Vec, Vec, Vec)              // AVX+ AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpunpcklqdq, Vpunpcklqdq, Vec, Vec, Mem)              // AVX+ AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpunpcklwd, Vpunpcklwd, Vec, Vec, Vec)                // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpunpcklwd, Vpunpcklwd, Vec, Vec, Mem)                // AVX+ AVX512_BW{kz}
  ASMJIT_INST_3x(vpxor, Vpxor, Vec, Vec, Vec)                          // AVX+
  ASMJIT_INST_3x(vpxor, Vpxor, Vec, Vec, Mem)                          // AVX+
  ASMJIT_INST_3x(vpxord, Vpxord, Vec, Vec, Vec)                        //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpxord, Vpxord, Vec, Vec, Mem)                        //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpxorq, Vpxorq, Vec, Vec, Vec)                        //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpxorq, Vpxorq, Vec, Vec, Mem)                        //      AVX512_F{kz|b64}
  ASMJIT_INST_4x(vrangepd, Vrangepd, Vec, Vec, Vec, Imm)               //      AVX512_DQ{kz|b64}
  ASMJIT_INST_4x(vrangepd, Vrangepd, Vec, Vec, Mem, Imm)               //      AVX512_DQ{kz|b64}
  ASMJIT_INST_4x(vrangeps, Vrangeps, Vec, Vec, Vec, Imm)               //      AVX512_DQ{kz|b32}
  ASMJIT_INST_4x(vrangeps, Vrangeps, Vec, Vec, Mem, Imm)               //      AVX512_DQ{kz|b32}
  ASMJIT_INST_4x(vrangesd, Vrangesd, Vec, Vec, Vec, Imm)               //      AVX512_DQ{kz|sae}
  ASMJIT_INST_4x(vrangesd, Vrangesd, Vec, Vec, Mem, Imm)               //      AVX512_DQ{kz|sae}
  ASMJIT_INST_4x(vrangess, Vrangess, Vec, Vec, Vec, Imm)               //      AVX512_DQ{kz|sae}
  ASMJIT_INST_4x(vrangess, Vrangess, Vec, Vec, Mem, Imm)               //      AVX512_DQ{kz|sae}
  ASMJIT_INST_2x(vrcp14pd, Vrcp14pd, Vec, Vec)                         //      AVX512_F{kz|b64}
  ASMJIT_INST_2x(vrcp14pd, Vrcp14pd, Vec, Mem)                         //      AVX512_F{kz|b64}
  ASMJIT_INST_2x(vrcp14ps, Vrcp14ps, Vec, Vec)                         //      AVX512_F{kz|b32}
  ASMJIT_INST_2x(vrcp14ps, Vrcp14ps, Vec, Mem)                         //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vrcp14sd, Vrcp14sd, Vec, Vec, Vec)                    //      AVX512_F{kz}
  ASMJIT_INST_3x(vrcp14sd, Vrcp14sd, Vec, Vec, Mem)                    //      AVX512_F{kz}
  ASMJIT_INST_3x(vrcp14ss, Vrcp14ss, Vec, Vec, Vec)                    //      AVX512_F{kz}
  ASMJIT_INST_3x(vrcp14ss, Vrcp14ss, Vec, Vec, Mem)                    //      AVX512_F{kz}
  ASMJIT_INST_2x(vrcpps, Vrcpps, Vec, Vec)                             // AVX
  ASMJIT_INST_2x(vrcpps, Vrcpps, Vec, Mem)                             // AVX
  ASMJIT_INST_3x(vrcpss, Vrcpss, Vec, Vec, Vec)                        // AVX
  ASMJIT_INST_3x(vrcpss, Vrcpss, Vec, Vec, Mem)                        // AVX
  ASMJIT_INST_3x(vreducepd, Vreducepd, Vec, Vec, Imm)                  //      AVX512_DQ{kz|b64}
  ASMJIT_INST_3x(vreducepd, Vreducepd, Vec, Mem, Imm)                  //      AVX512_DQ{kz|b64}
  ASMJIT_INST_3x(vreduceps, Vreduceps, Vec, Vec, Imm)                  //      AVX512_DQ{kz|b32}
  ASMJIT_INST_3x(vreduceps, Vreduceps, Vec, Mem, Imm)                  //      AVX512_DQ{kz|b32}
  ASMJIT_INST_4x(vreducesd, Vreducesd, Vec, Vec, Vec, Imm)             //      AVX512_DQ{kz}
  ASMJIT_INST_4x(vreducesd, Vreducesd, Vec, Vec, Mem, Imm)             //      AVX512_DQ{kz}
  ASMJIT_INST_4x(vreducess, Vreducess, Vec, Vec, Vec, Imm)             //      AVX512_DQ{kz}
  ASMJIT_INST_4x(vreducess, Vreducess, Vec, Vec, Mem, Imm)             //      AVX512_DQ{kz}
  ASMJIT_INST_3x(vrndscalepd, Vrndscalepd, Vec, Vec, Imm)              //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vrndscalepd, Vrndscalepd, Vec, Mem, Imm)              //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vrndscaleps, Vrndscaleps, Vec, Vec, Imm)              //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vrndscaleps, Vrndscaleps, Vec, Mem, Imm)              //      AVX512_F{kz|b32}
  ASMJIT_INST_4x(vrndscalesd, Vrndscalesd, Vec, Vec, Vec, Imm)         //      AVX512_F{kz|sae}
  ASMJIT_INST_4x(vrndscalesd, Vrndscalesd, Vec, Vec, Mem, Imm)         //      AVX512_F{kz|sae}
  ASMJIT_INST_4x(vrndscaless, Vrndscaless, Vec, Vec, Vec, Imm)         //      AVX512_F{kz|sae}
  ASMJIT_INST_4x(vrndscaless, Vrndscaless, Vec, Vec, Mem, Imm)         //      AVX512_F{kz|sae}
  ASMJIT_INST_3x(vroundpd, Vroundpd, Vec, Vec, Imm)                    // AVX
  ASMJIT_INST_3x(vroundpd, Vroundpd, Vec, Mem, Imm)                    // AVX
  ASMJIT_INST_3x(vroundps, Vroundps, Vec, Vec, Imm)                    // AVX
  ASMJIT_INST_3x(vroundps, Vroundps, Vec, Mem, Imm)                    // AVX
  ASMJIT_INST_4x(vroundsd, Vroundsd, Vec, Vec, Vec, Imm)               // AVX
  ASMJIT_INST_4x(vroundsd, Vroundsd, Vec, Vec, Mem, Imm)               // AVX
  ASMJIT_INST_4x(vroundss, Vroundss, Vec, Vec, Vec, Imm)               // AVX
  ASMJIT_INST_4x(vroundss, Vroundss, Vec, Vec, Mem, Imm)               // AVX
  ASMJIT_INST_2x(vrsqrt14pd, Vrsqrt14pd, Vec, Vec)                     //      AVX512_F{kz|b64}
  ASMJIT_INST_2x(vrsqrt14pd, Vrsqrt14pd, Vec, Mem)                     //      AVX512_F{kz|b64}
  ASMJIT_INST_2x(vrsqrt14ps, Vrsqrt14ps, Vec, Vec)                     //      AVX512_F{kz|b32}
  ASMJIT_INST_2x(vrsqrt14ps, Vrsqrt14ps, Vec, Mem)                     //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vrsqrt14sd, Vrsqrt14sd, Vec, Vec, Vec)                //      AVX512_F{kz}
  ASMJIT_INST_3x(vrsqrt14sd, Vrsqrt14sd, Vec, Vec, Mem)                //      AVX512_F{kz}
  ASMJIT_INST_3x(vrsqrt14ss, Vrsqrt14ss, Vec, Vec, Vec)                //      AVX512_F{kz}
  ASMJIT_INST_3x(vrsqrt14ss, Vrsqrt14ss, Vec, Vec, Mem)                //      AVX512_F{kz}
  ASMJIT_INST_2x(vrsqrtps, Vrsqrtps, Vec, Vec)                         // AVX
  ASMJIT_INST_2x(vrsqrtps, Vrsqrtps, Vec, Mem)                         // AVX
  ASMJIT_INST_3x(vrsqrtss, Vrsqrtss, Vec, Vec, Vec)                    // AVX
  ASMJIT_INST_3x(vrsqrtss, Vrsqrtss, Vec, Vec, Mem)                    // AVX
  ASMJIT_INST_3x(vscalefpd, Vscalefpd, Vec, Vec, Vec)                  //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vscalefpd, Vscalefpd, Vec, Vec, Mem)                  //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vscalefps, Vscalefps, Vec, Vec, Vec)                  //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vscalefps, Vscalefps, Vec, Vec, Mem)                  //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vscalefsd, Vscalefsd, Vec, Vec, Vec)                  //      AVX512_F{kz|er}
  ASMJIT_INST_3x(vscalefsd, Vscalefsd, Vec, Vec, Mem)                  //      AVX512_F{kz|er}
  ASMJIT_INST_3x(vscalefss, Vscalefss, Vec, Vec, Vec)                  //      AVX512_F{kz|er}
  ASMJIT_INST_3x(vscalefss, Vscalefss, Vec, Vec, Mem)                  //      AVX512_F{kz|er}
  ASMJIT_INST_2x(vscatterdpd, Vscatterdpd, Mem, Vec)                   //      AVX512_F{k}
  ASMJIT_INST_2x(vscatterdps, Vscatterdps, Mem, Vec)                   //      AVX512_F{k}
  ASMJIT_INST_2x(vscatterqpd, Vscatterqpd, Mem, Vec)                   //      AVX512_F{k}
  ASMJIT_INST_2x(vscatterqps, Vscatterqps, Mem, Vec)                   //      AVX512_F{k}
  ASMJIT_INST_4x(vshuff32x4, Vshuff32x4, Vec, Vec, Vec, Imm)           //      AVX512_F{kz|b32}
  ASMJIT_INST_4x(vshuff32x4, Vshuff32x4, Vec, Vec, Mem, Imm)           //      AVX512_F{kz|b32}
  ASMJIT_INST_4x(vshuff64x2, Vshuff64x2, Vec, Vec, Vec, Imm)           //      AVX512_F{kz|b64}
  ASMJIT_INST_4x(vshuff64x2, Vshuff64x2, Vec, Vec, Mem, Imm)           //      AVX512_F{kz|b64}
  ASMJIT_INST_4x(vshufi32x4, Vshufi32x4, Vec, Vec, Vec, Imm)           //      AVX512_F{kz|b32}
  ASMJIT_INST_4x(vshufi32x4, Vshufi32x4, Vec, Vec, Mem, Imm)           //      AVX512_F{kz|b32}
  ASMJIT_INST_4x(vshufi64x2, Vshufi64x2, Vec, Vec, Vec, Imm)           //      AVX512_F{kz|b64}
  ASMJIT_INST_4x(vshufi64x2, Vshufi64x2, Vec, Vec, Mem, Imm)           //      AVX512_F{kz|b64}
  ASMJIT_INST_4x(vshufpd, Vshufpd, Vec, Vec, Vec, Imm)                 // AVX  AVX512_F{kz|b32}
  ASMJIT_INST_4x(vshufpd, Vshufpd, Vec, Vec, Mem, Imm)                 // AVX  AVX512_F{kz|b32}
  ASMJIT_INST_4x(vshufps, Vshufps, Vec, Vec, Vec, Imm)                 // AVX  AVX512_F{kz|b64}
  ASMJIT_INST_4x(vshufps, Vshufps, Vec, Vec, Mem, Imm)                 // AVX  AVX512_F{kz|b64}
  ASMJIT_INST_2x(vsqrtpd, Vsqrtpd, Vec, Vec)                           // AVX  AVX512_F{kz|b64}
  ASMJIT_INST_2x(vsqrtpd, Vsqrtpd, Vec, Mem)                           // AVX  AVX512_F{kz|b64}
  ASMJIT_INST_2x(vsqrtps, Vsqrtps, Vec, Vec)                           // AVX  AVX512_F{kz|b32}
  ASMJIT_INST_2x(vsqrtps, Vsqrtps, Vec, Mem)                           // AVX  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vsqrtsd, Vsqrtsd, Vec, Vec, Vec)                      // AVX  AVX512_F{kz|er}
  ASMJIT_INST_3x(vsqrtsd, Vsqrtsd, Vec, Vec, Mem)                      // AVX  AVX512_F{kz|er}
  ASMJIT_INST_3x(vsqrtss, Vsqrtss, Vec, Vec, Vec)                      // AVX  AVX512_F{kz|er}
  ASMJIT_INST_3x(vsqrtss, Vsqrtss, Vec, Vec, Mem)                      // AVX  AVX512_F{kz|er}
  ASMJIT_INST_1x(vstmxcsr, Vstmxcsr, Mem)                              // AVX
  ASMJIT_INST_3x(vsubpd, Vsubpd, Vec, Vec, Vec)                        // AVX  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vsubpd, Vsubpd, Vec, Vec, Mem)                        // AVX  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vsubps, Vsubps, Vec, Vec, Vec)                        // AVX  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vsubps, Vsubps, Vec, Vec, Mem)                        // AVX  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vsubsd, Vsubsd, Vec, Vec, Vec)                        // AVX  AVX512_F{kz|er}
  ASMJIT_INST_3x(vsubsd, Vsubsd, Vec, Vec, Mem)                        // AVX  AVX512_F{kz|er}
  ASMJIT_INST_3x(vsubss, Vsubss, Vec, Vec, Vec)                        // AVX  AVX512_F{kz|er}
  ASMJIT_INST_3x(vsubss, Vsubss, Vec, Vec, Mem)                        // AVX  AVX512_F{kz|er}
  ASMJIT_INST_2x(vtestpd, Vtestpd, Vec, Vec)                           // AVX
  ASMJIT_INST_2x(vtestpd, Vtestpd, Vec, Mem)                           // AVX
  ASMJIT_INST_2x(vtestps, Vtestps, Vec, Vec)                           // AVX
  ASMJIT_INST_2x(vtestps, Vtestps, Vec, Mem)                           // AVX
  ASMJIT_INST_2x(vucomisd, Vucomisd, Vec, Vec)                         // AVX  AVX512_F{sae}
  ASMJIT_INST_2x(vucomisd, Vucomisd, Vec, Mem)                         // AVX  AVX512_F{sae}
  ASMJIT_INST_2x(vucomiss, Vucomiss, Vec, Vec)                         // AVX  AVX512_F{sae}
  ASMJIT_INST_2x(vucomiss, Vucomiss, Vec, Mem)                         // AVX  AVX512_F{sae}
  ASMJIT_INST_3x(vunpckhpd, Vunpckhpd, Vec, Vec, Vec)                  // AVX  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vunpckhpd, Vunpckhpd, Vec, Vec, Mem)                  // AVX  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vunpckhps, Vunpckhps, Vec, Vec, Vec)                  // AVX  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vunpckhps, Vunpckhps, Vec, Vec, Mem)                  // AVX  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vunpcklpd, Vunpcklpd, Vec, Vec, Vec)                  // AVX  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vunpcklpd, Vunpcklpd, Vec, Vec, Mem)                  // AVX  AVX512_F{kz|b64}
  ASMJIT_INST_3x(vunpcklps, Vunpcklps, Vec, Vec, Vec)                  // AVX  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vunpcklps, Vunpcklps, Vec, Vec, Mem)                  // AVX  AVX512_F{kz|b32}
  ASMJIT_INST_3x(vxorpd, Vxorpd, Vec, Vec, Vec)                        // AVX  AVX512_DQ{kz|b64}
  ASMJIT_INST_3x(vxorpd, Vxorpd, Vec, Vec, Mem)                        // AVX  AVX512_DQ{kz|b64}
  ASMJIT_INST_3x(vxorps, Vxorps, Vec, Vec, Vec)                        // AVX  AVX512_DQ{kz|b32}
  ASMJIT_INST_3x(vxorps, Vxorps, Vec, Vec, Mem)                        // AVX  AVX512_DQ{kz|b32}
  ASMJIT_INST_0x(vzeroall, Vzeroall)                                   // AVX
  ASMJIT_INST_0x(vzeroupper, Vzeroupper)                               // AVX

  //! \}

  //! \name FMA4 Instructions
  //! \{

  ASMJIT_INST_4x(vfmaddpd, Vfmaddpd, Vec, Vec, Vec, Vec)               // FMA4
  ASMJIT_INST_4x(vfmaddpd, Vfmaddpd, Vec, Vec, Mem, Vec)               // FMA4
  ASMJIT_INST_4x(vfmaddpd, Vfmaddpd, Vec, Vec, Vec, Mem)               // FMA4
  ASMJIT_INST_4x(vfmaddps, Vfmaddps, Vec, Vec, Vec, Vec)               // FMA4
  ASMJIT_INST_4x(vfmaddps, Vfmaddps, Vec, Vec, Mem, Vec)               // FMA4
  ASMJIT_INST_4x(vfmaddps, Vfmaddps, Vec, Vec, Vec, Mem)               // FMA4
  ASMJIT_INST_4x(vfmaddsd, Vfmaddsd, Vec, Vec, Vec, Vec)               // FMA4
  ASMJIT_INST_4x(vfmaddsd, Vfmaddsd, Vec, Vec, Mem, Vec)               // FMA4
  ASMJIT_INST_4x(vfmaddsd, Vfmaddsd, Vec, Vec, Vec, Mem)               // FMA4
  ASMJIT_INST_4x(vfmaddss, Vfmaddss, Vec, Vec, Vec, Vec)               // FMA4
  ASMJIT_INST_4x(vfmaddss, Vfmaddss, Vec, Vec, Mem, Vec)               // FMA4
  ASMJIT_INST_4x(vfmaddss, Vfmaddss, Vec, Vec, Vec, Mem)               // FMA4
  ASMJIT_INST_4x(vfmaddsubpd, Vfmaddsubpd, Vec, Vec, Vec, Vec)         // FMA4
  ASMJIT_INST_4x(vfmaddsubpd, Vfmaddsubpd, Vec, Vec, Mem, Vec)         // FMA4
  ASMJIT_INST_4x(vfmaddsubpd, Vfmaddsubpd, Vec, Vec, Vec, Mem)         // FMA4
  ASMJIT_INST_4x(vfmaddsubps, Vfmaddsubps, Vec, Vec, Vec, Vec)         // FMA4
  ASMJIT_INST_4x(vfmaddsubps, Vfmaddsubps, Vec, Vec, Mem, Vec)         // FMA4
  ASMJIT_INST_4x(vfmaddsubps, Vfmaddsubps, Vec, Vec, Vec, Mem)         // FMA4
  ASMJIT_INST_4x(vfmsubaddpd, Vfmsubaddpd, Vec, Vec, Vec, Vec)         // FMA4
  ASMJIT_INST_4x(vfmsubaddpd, Vfmsubaddpd, Vec, Vec, Mem, Vec)         // FMA4
  ASMJIT_INST_4x(vfmsubaddpd, Vfmsubaddpd, Vec, Vec, Vec, Mem)         // FMA4
  ASMJIT_INST_4x(vfmsubaddps, Vfmsubaddps, Vec, Vec, Vec, Vec)         // FMA4
  ASMJIT_INST_4x(vfmsubaddps, Vfmsubaddps, Vec, Vec, Mem, Vec)         // FMA4
  ASMJIT_INST_4x(vfmsubaddps, Vfmsubaddps, Vec, Vec, Vec, Mem)         // FMA4
  ASMJIT_INST_4x(vfmsubpd, Vfmsubpd, Vec, Vec, Vec, Vec)               // FMA4
  ASMJIT_INST_4x(vfmsubpd, Vfmsubpd, Vec, Vec, Mem, Vec)               // FMA4
  ASMJIT_INST_4x(vfmsubpd, Vfmsubpd, Vec, Vec, Vec, Mem)               // FMA4
  ASMJIT_INST_4x(vfmsubps, Vfmsubps, Vec, Vec, Vec, Vec)               // FMA4
  ASMJIT_INST_4x(vfmsubps, Vfmsubps, Vec, Vec, Mem, Vec)               // FMA4
  ASMJIT_INST_4x(vfmsubps, Vfmsubps, Vec, Vec, Vec, Mem)               // FMA4
  ASMJIT_INST_4x(vfmsubsd, Vfmsubsd, Vec, Vec, Vec, Vec)               // FMA4
  ASMJIT_INST_4x(vfmsubsd, Vfmsubsd, Vec, Vec, Mem, Vec)               // FMA4
  ASMJIT_INST_4x(vfmsubsd, Vfmsubsd, Vec, Vec, Vec, Mem)               // FMA4
  ASMJIT_INST_4x(vfmsubss, Vfmsubss, Vec, Vec, Vec, Vec)               // FMA4
  ASMJIT_INST_4x(vfmsubss, Vfmsubss, Vec, Vec, Mem, Vec)               // FMA4
  ASMJIT_INST_4x(vfmsubss, Vfmsubss, Vec, Vec, Vec, Mem)               // FMA4
  ASMJIT_INST_4x(vfnmaddpd, Vfnmaddpd, Vec, Vec, Vec, Vec)             // FMA4
  ASMJIT_INST_4x(vfnmaddpd, Vfnmaddpd, Vec, Vec, Mem, Vec)             // FMA4
  ASMJIT_INST_4x(vfnmaddpd, Vfnmaddpd, Vec, Vec, Vec, Mem)             // FMA4
  ASMJIT_INST_4x(vfnmaddps, Vfnmaddps, Vec, Vec, Vec, Vec)             // FMA4
  ASMJIT_INST_4x(vfnmaddps, Vfnmaddps, Vec, Vec, Mem, Vec)             // FMA4
  ASMJIT_INST_4x(vfnmaddps, Vfnmaddps, Vec, Vec, Vec, Mem)             // FMA4
  ASMJIT_INST_4x(vfnmaddsd, Vfnmaddsd, Vec, Vec, Vec, Vec)             // FMA4
  ASMJIT_INST_4x(vfnmaddsd, Vfnmaddsd, Vec, Vec, Mem, Vec)             // FMA4
  ASMJIT_INST_4x(vfnmaddsd, Vfnmaddsd, Vec, Vec, Vec, Mem)             // FMA4
  ASMJIT_INST_4x(vfnmaddss, Vfnmaddss, Vec, Vec, Vec, Vec)             // FMA4
  ASMJIT_INST_4x(vfnmaddss, Vfnmaddss, Vec, Vec, Mem, Vec)             // FMA4
  ASMJIT_INST_4x(vfnmaddss, Vfnmaddss, Vec, Vec, Vec, Mem)             // FMA4
  ASMJIT_INST_4x(vfnmsubpd, Vfnmsubpd, Vec, Vec, Vec, Vec)             // FMA4
  ASMJIT_INST_4x(vfnmsubpd, Vfnmsubpd, Vec, Vec, Mem, Vec)             // FMA4
  ASMJIT_INST_4x(vfnmsubpd, Vfnmsubpd, Vec, Vec, Vec, Mem)             // FMA4
  ASMJIT_INST_4x(vfnmsubps, Vfnmsubps, Vec, Vec, Vec, Vec)             // FMA4
  ASMJIT_INST_4x(vfnmsubps, Vfnmsubps, Vec, Vec, Mem, Vec)             // FMA4
  ASMJIT_INST_4x(vfnmsubps, Vfnmsubps, Vec, Vec, Vec, Mem)             // FMA4
  ASMJIT_INST_4x(vfnmsubsd, Vfnmsubsd, Vec, Vec, Vec, Vec)             // FMA4
  ASMJIT_INST_4x(vfnmsubsd, Vfnmsubsd, Vec, Vec, Mem, Vec)             // FMA4
  ASMJIT_INST_4x(vfnmsubsd, Vfnmsubsd, Vec, Vec, Vec, Mem)             // FMA4
  ASMJIT_INST_4x(vfnmsubss, Vfnmsubss, Vec, Vec, Vec, Vec)             // FMA4
  ASMJIT_INST_4x(vfnmsubss, Vfnmsubss, Vec, Vec, Mem, Vec)             // FMA4
  ASMJIT_INST_4x(vfnmsubss, Vfnmsubss, Vec, Vec, Vec, Mem)             // FMA4

  //! \}

  //! \name XOP Instructions (Deprecated)
  //! \{

  ASMJIT_INST_2x(vfrczpd, Vfrczpd, Vec, Vec)                           // XOP
  ASMJIT_INST_2x(vfrczpd, Vfrczpd, Vec, Mem)                           // XOP
  ASMJIT_INST_2x(vfrczps, Vfrczps, Vec, Vec)                           // XOP
  ASMJIT_INST_2x(vfrczps, Vfrczps, Vec, Mem)                           // XOP
  ASMJIT_INST_2x(vfrczsd, Vfrczsd, Vec, Vec)                           // XOP
  ASMJIT_INST_2x(vfrczsd, Vfrczsd, Vec, Mem)                           // XOP
  ASMJIT_INST_2x(vfrczss, Vfrczss, Vec, Vec)                           // XOP
  ASMJIT_INST_2x(vfrczss, Vfrczss, Vec, Mem)                           // XOP
  ASMJIT_INST_4x(vpcmov, Vpcmov, Vec, Vec, Vec, Vec)                   // XOP
  ASMJIT_INST_4x(vpcmov, Vpcmov, Vec, Vec, Mem, Vec)                   // XOP
  ASMJIT_INST_4x(vpcmov, Vpcmov, Vec, Vec, Vec, Mem)                   // XOP
  ASMJIT_INST_4x(vpcomb, Vpcomb, Vec, Vec, Vec, Imm)                   // XOP
  ASMJIT_INST_4x(vpcomb, Vpcomb, Vec, Vec, Mem, Imm)                   // XOP
  ASMJIT_INST_4x(vpcomd, Vpcomd, Vec, Vec, Vec, Imm)                   // XOP
  ASMJIT_INST_4x(vpcomd, Vpcomd, Vec, Vec, Mem, Imm)                   // XOP
  ASMJIT_INST_4x(vpcomq, Vpcomq, Vec, Vec, Vec, Imm)                   // XOP
  ASMJIT_INST_4x(vpcomq, Vpcomq, Vec, Vec, Mem, Imm)                   // XOP
  ASMJIT_INST_4x(vpcomw, Vpcomw, Vec, Vec, Vec, Imm)                   // XOP
  ASMJIT_INST_4x(vpcomw, Vpcomw, Vec, Vec, Mem, Imm)                   // XOP
  ASMJIT_INST_4x(vpcomub, Vpcomub, Vec, Vec, Vec, Imm)                 // XOP
  ASMJIT_INST_4x(vpcomub, Vpcomub, Vec, Vec, Mem, Imm)                 // XOP
  ASMJIT_INST_4x(vpcomud, Vpcomud, Vec, Vec, Vec, Imm)                 // XOP
  ASMJIT_INST_4x(vpcomud, Vpcomud, Vec, Vec, Mem, Imm)                 // XOP
  ASMJIT_INST_4x(vpcomuq, Vpcomuq, Vec, Vec, Vec, Imm)                 // XOP
  ASMJIT_INST_4x(vpcomuq, Vpcomuq, Vec, Vec, Mem, Imm)                 // XOP
  ASMJIT_INST_4x(vpcomuw, Vpcomuw, Vec, Vec, Vec, Imm)                 // XOP
  ASMJIT_INST_4x(vpcomuw, Vpcomuw, Vec, Vec, Mem, Imm)                 // XOP
  ASMJIT_INST_5x(vpermil2pd, Vpermil2pd, Vec, Vec, Vec, Vec, Imm)      // XOP
  ASMJIT_INST_5x(vpermil2pd, Vpermil2pd, Vec, Vec, Mem, Vec, Imm)      // XOP
  ASMJIT_INST_5x(vpermil2pd, Vpermil2pd, Vec, Vec, Vec, Mem, Imm)      // XOP
  ASMJIT_INST_5x(vpermil2ps, Vpermil2ps, Vec, Vec, Vec, Vec, Imm)      // XOP
  ASMJIT_INST_5x(vpermil2ps, Vpermil2ps, Vec, Vec, Mem, Vec, Imm)      // XOP
  ASMJIT_INST_5x(vpermil2ps, Vpermil2ps, Vec, Vec, Vec, Mem, Imm)      // XOP
  ASMJIT_INST_2x(vphaddbd, Vphaddbd, Vec, Vec)                         // XOP
  ASMJIT_INST_2x(vphaddbd, Vphaddbd, Vec, Mem)                         // XOP
  ASMJIT_INST_2x(vphaddbq, Vphaddbq, Vec, Vec)                         // XOP
  ASMJIT_INST_2x(vphaddbq, Vphaddbq, Vec, Mem)                         // XOP
  ASMJIT_INST_2x(vphaddbw, Vphaddbw, Vec, Vec)                         // XOP
  ASMJIT_INST_2x(vphaddbw, Vphaddbw, Vec, Mem)                         // XOP
  ASMJIT_INST_2x(vphadddq, Vphadddq, Vec, Vec)                         // XOP
  ASMJIT_INST_2x(vphadddq, Vphadddq, Vec, Mem)                         // XOP
  ASMJIT_INST_2x(vphaddwd, Vphaddwd, Vec, Vec)                         // XOP
  ASMJIT_INST_2x(vphaddwd, Vphaddwd, Vec, Mem)                         // XOP
  ASMJIT_INST_2x(vphaddwq, Vphaddwq, Vec, Vec)                         // XOP
  ASMJIT_INST_2x(vphaddwq, Vphaddwq, Vec, Mem)                         // XOP
  ASMJIT_INST_2x(vphaddubd, Vphaddubd, Vec, Vec)                       // XOP
  ASMJIT_INST_2x(vphaddubd, Vphaddubd, Vec, Mem)                       // XOP
  ASMJIT_INST_2x(vphaddubq, Vphaddubq, Vec, Vec)                       // XOP
  ASMJIT_INST_2x(vphaddubq, Vphaddubq, Vec, Mem)                       // XOP
  ASMJIT_INST_2x(vphaddubw, Vphaddubw, Vec, Vec)                       // XOP
  ASMJIT_INST_2x(vphaddubw, Vphaddubw, Vec, Mem)                       // XOP
  ASMJIT_INST_2x(vphaddudq, Vphaddudq, Vec, Vec)                       // XOP
  ASMJIT_INST_2x(vphaddudq, Vphaddudq, Vec, Mem)                       // XOP
  ASMJIT_INST_2x(vphadduwd, Vphadduwd, Vec, Vec)                       // XOP
  ASMJIT_INST_2x(vphadduwd, Vphadduwd, Vec, Mem)                       // XOP
  ASMJIT_INST_2x(vphadduwq, Vphadduwq, Vec, Vec)                       // XOP
  ASMJIT_INST_2x(vphadduwq, Vphadduwq, Vec, Mem)                       // XOP
  ASMJIT_INST_2x(vphsubbw, Vphsubbw, Vec, Vec)                         // XOP
  ASMJIT_INST_2x(vphsubbw, Vphsubbw, Vec, Mem)                         // XOP
  ASMJIT_INST_2x(vphsubdq, Vphsubdq, Vec, Vec)                         // XOP
  ASMJIT_INST_2x(vphsubdq, Vphsubdq, Vec, Mem)                         // XOP
  ASMJIT_INST_2x(vphsubwd, Vphsubwd, Vec, Vec)                         // XOP
  ASMJIT_INST_2x(vphsubwd, Vphsubwd, Vec, Mem)                         // XOP
  ASMJIT_INST_4x(vpmacsdd, Vpmacsdd, Vec, Vec, Vec, Vec)               // XOP
  ASMJIT_INST_4x(vpmacsdd, Vpmacsdd, Vec, Vec, Mem, Vec)               // XOP
  ASMJIT_INST_4x(vpmacsdqh, Vpmacsdqh, Vec, Vec, Vec, Vec)             // XOP
  ASMJIT_INST_4x(vpmacsdqh, Vpmacsdqh, Vec, Vec, Mem, Vec)             // XOP
  ASMJIT_INST_4x(vpmacsdql, Vpmacsdql, Vec, Vec, Vec, Vec)             // XOP
  ASMJIT_INST_4x(vpmacsdql, Vpmacsdql, Vec, Vec, Mem, Vec)             // XOP
  ASMJIT_INST_4x(vpmacswd, Vpmacswd, Vec, Vec, Vec, Vec)               // XOP
  ASMJIT_INST_4x(vpmacswd, Vpmacswd, Vec, Vec, Mem, Vec)               // XOP
  ASMJIT_INST_4x(vpmacsww, Vpmacsww, Vec, Vec, Vec, Vec)               // XOP
  ASMJIT_INST_4x(vpmacsww, Vpmacsww, Vec, Vec, Mem, Vec)               // XOP
  ASMJIT_INST_4x(vpmacssdd, Vpmacssdd, Vec, Vec, Vec, Vec)             // XOP
  ASMJIT_INST_4x(vpmacssdd, Vpmacssdd, Vec, Vec, Mem, Vec)             // XOP
  ASMJIT_INST_4x(vpmacssdqh, Vpmacssdqh, Vec, Vec, Vec, Vec)           // XOP
  ASMJIT_INST_4x(vpmacssdqh, Vpmacssdqh, Vec, Vec, Mem, Vec)           // XOP
  ASMJIT_INST_4x(vpmacssdql, Vpmacssdql, Vec, Vec, Vec, Vec)           // XOP
  ASMJIT_INST_4x(vpmacssdql, Vpmacssdql, Vec, Vec, Mem, Vec)           // XOP
  ASMJIT_INST_4x(vpmacsswd, Vpmacsswd, Vec, Vec, Vec, Vec)             // XOP
  ASMJIT_INST_4x(vpmacsswd, Vpmacsswd, Vec, Vec, Mem, Vec)             // XOP
  ASMJIT_INST_4x(vpmacssww, Vpmacssww, Vec, Vec, Vec, Vec)             // XOP
  ASMJIT_INST_4x(vpmacssww, Vpmacssww, Vec, Vec, Mem, Vec)             // XOP
  ASMJIT_INST_4x(vpmadcsswd, Vpmadcsswd, Vec, Vec, Vec, Vec)           // XOP
  ASMJIT_INST_4x(vpmadcsswd, Vpmadcsswd, Vec, Vec, Mem, Vec)           // XOP
  ASMJIT_INST_4x(vpmadcswd, Vpmadcswd, Vec, Vec, Vec, Vec)             // XOP
  ASMJIT_INST_4x(vpmadcswd, Vpmadcswd, Vec, Vec, Mem, Vec)             // XOP
  ASMJIT_INST_4x(vpperm, Vpperm, Vec, Vec, Vec, Vec)                   // XOP
  ASMJIT_INST_4x(vpperm, Vpperm, Vec, Vec, Mem, Vec)                   // XOP
  ASMJIT_INST_4x(vpperm, Vpperm, Vec, Vec, Vec, Mem)                   // XOP
  ASMJIT_INST_3x(vprotb, Vprotb, Vec, Vec, Vec)                        // XOP
  ASMJIT_INST_3x(vprotb, Vprotb, Vec, Mem, Vec)                        // XOP
  ASMJIT_INST_3x(vprotb, Vprotb, Vec, Vec, Mem)                        // XOP
  ASMJIT_INST_3x(vprotb, Vprotb, Vec, Vec, Imm)                        // XOP
  ASMJIT_INST_3x(vprotb, Vprotb, Vec, Mem, Imm)                        // XOP
  ASMJIT_INST_3x(vprotd, Vprotd, Vec, Vec, Vec)                        // XOP
  ASMJIT_INST_3x(vprotd, Vprotd, Vec, Mem, Vec)                        // XOP
  ASMJIT_INST_3x(vprotd, Vprotd, Vec, Vec, Mem)                        // XOP
  ASMJIT_INST_3x(vprotd, Vprotd, Vec, Vec, Imm)                        // XOP
  ASMJIT_INST_3x(vprotd, Vprotd, Vec, Mem, Imm)                        // XOP
  ASMJIT_INST_3x(vprotq, Vprotq, Vec, Vec, Vec)                        // XOP
  ASMJIT_INST_3x(vprotq, Vprotq, Vec, Mem, Vec)                        // XOP
  ASMJIT_INST_3x(vprotq, Vprotq, Vec, Vec, Mem)                        // XOP
  ASMJIT_INST_3x(vprotq, Vprotq, Vec, Vec, Imm)                        // XOP
  ASMJIT_INST_3x(vprotq, Vprotq, Vec, Mem, Imm)                        // XOP
  ASMJIT_INST_3x(vprotw, Vprotw, Vec, Vec, Vec)                        // XOP
  ASMJIT_INST_3x(vprotw, Vprotw, Vec, Mem, Vec)                        // XOP
  ASMJIT_INST_3x(vprotw, Vprotw, Vec, Vec, Mem)                        // XOP
  ASMJIT_INST_3x(vprotw, Vprotw, Vec, Vec, Imm)                        // XOP
  ASMJIT_INST_3x(vprotw, Vprotw, Vec, Mem, Imm)                        // XOP
  ASMJIT_INST_3x(vpshab, Vpshab, Vec, Vec, Vec)                        // XOP
  ASMJIT_INST_3x(vpshab, Vpshab, Vec, Mem, Vec)                        // XOP
  ASMJIT_INST_3x(vpshab, Vpshab, Vec, Vec, Mem)                        // XOP
  ASMJIT_INST_3x(vpshad, Vpshad, Vec, Vec, Vec)                        // XOP
  ASMJIT_INST_3x(vpshad, Vpshad, Vec, Mem, Vec)                        // XOP
  ASMJIT_INST_3x(vpshad, Vpshad, Vec, Vec, Mem)                        // XOP
  ASMJIT_INST_3x(vpshaq, Vpshaq, Vec, Vec, Vec)                        // XOP
  ASMJIT_INST_3x(vpshaq, Vpshaq, Vec, Mem, Vec)                        // XOP
  ASMJIT_INST_3x(vpshaq, Vpshaq, Vec, Vec, Mem)                        // XOP
  ASMJIT_INST_3x(vpshaw, Vpshaw, Vec, Vec, Vec)                        // XOP
  ASMJIT_INST_3x(vpshaw, Vpshaw, Vec, Mem, Vec)                        // XOP
  ASMJIT_INST_3x(vpshaw, Vpshaw, Vec, Vec, Mem)                        // XOP
  ASMJIT_INST_3x(vpshlb, Vpshlb, Vec, Vec, Vec)                        // XOP
  ASMJIT_INST_3x(vpshlb, Vpshlb, Vec, Mem, Vec)                        // XOP
  ASMJIT_INST_3x(vpshlb, Vpshlb, Vec, Vec, Mem)                        // XOP
  ASMJIT_INST_3x(vpshld, Vpshld, Vec, Vec, Vec)                        // XOP
  ASMJIT_INST_3x(vpshld, Vpshld, Vec, Mem, Vec)                        // XOP
  ASMJIT_INST_3x(vpshld, Vpshld, Vec, Vec, Mem)                        // XOP
  ASMJIT_INST_3x(vpshlq, Vpshlq, Vec, Vec, Vec)                        // XOP
  ASMJIT_INST_3x(vpshlq, Vpshlq, Vec, Mem, Vec)                        // XOP
  ASMJIT_INST_3x(vpshlq, Vpshlq, Vec, Vec, Mem)                        // XOP
  ASMJIT_INST_3x(vpshlw, Vpshlw, Vec, Vec, Vec)                        // XOP
  ASMJIT_INST_3x(vpshlw, Vpshlw, Vec, Mem, Vec)                        // XOP
  ASMJIT_INST_3x(vpshlw, Vpshlw, Vec, Vec, Mem)                        // XOP

  //! \}

  //! \name AVX_NE_CONVERT Instructions
  //! \{

  ASMJIT_INST_2x(vbcstnebf162ps, Vbcstnebf162ps, Vec, Mem)
  ASMJIT_INST_2x(vbcstnesh2ps, Vbcstnesh2ps, Vec, Mem)
  ASMJIT_INST_2x(vcvtneebf162ps, Vcvtneebf162ps, Vec, Mem)
  ASMJIT_INST_2x(vcvtneeph2ps, Vcvtneeph2ps, Vec, Mem)
  ASMJIT_INST_2x(vcvtneobf162ps, Vcvtneobf162ps, Vec, Mem)
  ASMJIT_INST_2x(vcvtneoph2ps, Vcvtneoph2ps, Vec, Mem)

  //! \}

  //! \name AVX_VNNI_INT8 Instructions
  //! \{

  ASMJIT_INST_3x(vpdpbssd, Vpdpbssd, Vec, Vec, Vec)
  ASMJIT_INST_3x(vpdpbssd, Vpdpbssd, Vec, Vec, Mem)
  ASMJIT_INST_3x(vpdpbssds, Vpdpbssds, Vec, Vec, Vec)
  ASMJIT_INST_3x(vpdpbssds, Vpdpbssds, Vec, Vec, Mem)
  ASMJIT_INST_3x(vpdpbsud, Vpdpbsud, Vec, Vec, Vec)
  ASMJIT_INST_3x(vpdpbsud, Vpdpbsud, Vec, Vec, Mem)
  ASMJIT_INST_3x(vpdpbsuds, Vpdpbsuds, Vec, Vec, Vec)
  ASMJIT_INST_3x(vpdpbsuds, Vpdpbsuds, Vec, Vec, Mem)
  ASMJIT_INST_3x(vpdpbuud, Vpdpbuud, Vec, Vec, Vec)
  ASMJIT_INST_3x(vpdpbuud, Vpdpbuud, Vec, Vec, Mem)
  ASMJIT_INST_3x(vpdpbuuds, Vpdpbuuds, Vec, Vec, Vec)
  ASMJIT_INST_3x(vpdpbuuds, Vpdpbuuds, Vec, Vec, Mem)

  //! \}

  //! \name AVX_VNNI_INT16 Instructions
  //! \{

  ASMJIT_INST_3x(vpdpwsud, Vpdpwsud, Vec, Vec, Vec)
  ASMJIT_INST_3x(vpdpwsud, Vpdpwsud, Vec, Vec, Mem)
  ASMJIT_INST_3x(vpdpwsuds, Vpdpwsuds, Vec, Vec, Vec)
  ASMJIT_INST_3x(vpdpwsuds, Vpdpwsuds, Vec, Vec, Mem)
  ASMJIT_INST_3x(vpdpwusd, Vpdpwusd, Vec, Vec, Vec)
  ASMJIT_INST_3x(vpdpwusd, Vpdpwusd, Vec, Vec, Mem)
  ASMJIT_INST_3x(vpdpwusds, Vpdpwusds, Vec, Vec, Vec)
  ASMJIT_INST_3x(vpdpwusds, Vpdpwusds, Vec, Vec, Mem)
  ASMJIT_INST_3x(vpdpwuud, Vpdpwuud, Vec, Vec, Vec)
  ASMJIT_INST_3x(vpdpwuud, Vpdpwuud, Vec, Vec, Mem)
  ASMJIT_INST_3x(vpdpwuuds, Vpdpwuuds, Vec, Vec, Vec)
  ASMJIT_INST_3x(vpdpwuuds, Vpdpwuuds, Vec, Vec, Mem)

  //! \}

  //! \name AVX+SHA512 Instructions
  //! \{
  ASMJIT_INST_2x(vsha512msg1, Vsha512msg1, Vec, Vec)
  ASMJIT_INST_2x(vsha512msg2, Vsha512msg2, Vec, Vec)
  ASMJIT_INST_3x(vsha512rnds2, Vsha512rnds2, Vec, Vec, Vec)
  //! \}

  //! \name AVX+SM3 Instructions
  //! \{

  ASMJIT_INST_3x(vsm3msg1, Vsm3msg1, Vec, Vec, Vec)
  ASMJIT_INST_3x(vsm3msg1, Vsm3msg1, Vec, Vec, Mem)
  ASMJIT_INST_3x(vsm3msg2, Vsm3msg2, Vec, Vec, Vec)
  ASMJIT_INST_3x(vsm3msg2, Vsm3msg2, Vec, Vec, Mem)
  ASMJIT_INST_4x(vsm3rnds2, Vsm3rnds2, Vec, Vec, Vec, Imm)
  ASMJIT_INST_4x(vsm3rnds2, Vsm3rnds2, Vec, Vec, Mem, Imm)

  //! \}

  //! \name AVX+SM4 Instructions
  //! \{

  ASMJIT_INST_3x(vsm4key4, Vsm4key4, Vec, Vec, Vec)
  ASMJIT_INST_3x(vsm4key4, Vsm4key4, Vec, Vec, Mem)
  ASMJIT_INST_3x(vsm4rnds4, Vsm4rnds4, Vec, Vec, Vec)
  ASMJIT_INST_3x(vsm4rnds4, Vsm4rnds4, Vec, Vec, Mem)

  //! \}

  //! \name AVX512_FP16 Instructions
  //! \{

  ASMJIT_INST_3x(vaddph, Vaddph, Vec, Vec, Vec)
  ASMJIT_INST_3x(vaddph, Vaddph, Vec, Vec, Mem)
  ASMJIT_INST_3x(vaddsh, Vaddsh, Vec, Vec, Vec)
  ASMJIT_INST_3x(vaddsh, Vaddsh, Vec, Vec, Mem)
  ASMJIT_INST_4x(vcmpph, Vcmpph, KReg, Vec, Vec, Imm)
  ASMJIT_INST_4x(vcmpph, Vcmpph, KReg, Vec, Mem, Imm)
  ASMJIT_INST_4x(vcmpsh, Vcmpsh, KReg, Vec, Vec, Imm)
  ASMJIT_INST_4x(vcmpsh, Vcmpsh, KReg, Vec, Mem, Imm)
  ASMJIT_INST_2x(vcomish, Vcomish, Vec, Vec)
  ASMJIT_INST_2x(vcomish, Vcomish, Vec, Mem)
  ASMJIT_INST_2x(vcvtdq2ph, Vcvtdq2ph, Vec, Vec)
  ASMJIT_INST_2x(vcvtdq2ph, Vcvtdq2ph, Vec, Mem)
  ASMJIT_INST_2x(vcvtpd2ph, Vcvtpd2ph, Vec, Vec)
  ASMJIT_INST_2x(vcvtpd2ph, Vcvtpd2ph, Vec, Mem)
  ASMJIT_INST_2x(vcvtph2dq, Vcvtph2dq, Vec, Vec)
  ASMJIT_INST_2x(vcvtph2dq, Vcvtph2dq, Vec, Mem)
  ASMJIT_INST_2x(vcvtph2pd, Vcvtph2pd, Vec, Vec)
  ASMJIT_INST_2x(vcvtph2pd, Vcvtph2pd, Vec, Mem)
  ASMJIT_INST_2x(vcvtph2psx, Vcvtph2psx, Vec, Vec)
  ASMJIT_INST_2x(vcvtph2psx, Vcvtph2psx, Vec, Mem)
  ASMJIT_INST_2x(vcvtph2qq, Vcvtph2qq, Vec, Vec)
  ASMJIT_INST_2x(vcvtph2qq, Vcvtph2qq, Vec, Mem)
  ASMJIT_INST_2x(vcvtph2udq, Vcvtph2udq, Vec, Vec)
  ASMJIT_INST_2x(vcvtph2udq, Vcvtph2udq, Vec, Mem)
  ASMJIT_INST_2x(vcvtph2uqq, Vcvtph2uqq, Vec, Vec)
  ASMJIT_INST_2x(vcvtph2uqq, Vcvtph2uqq, Vec, Mem)
  ASMJIT_INST_2x(vcvtph2uw, Vcvtph2uw, Vec, Vec)
  ASMJIT_INST_2x(vcvtph2uw, Vcvtph2uw, Vec, Mem)
  ASMJIT_INST_2x(vcvtph2w, Vcvtph2w, Vec, Vec)
  ASMJIT_INST_2x(vcvtph2w, Vcvtph2w, Vec, Mem)
  ASMJIT_INST_2x(vcvtps2phx, Vcvtps2phx, Vec, Vec)
  ASMJIT_INST_2x(vcvtps2phx, Vcvtps2phx, Vec, Mem)
  ASMJIT_INST_2x(vcvtqq2ph, Vcvtqq2ph, Vec, Vec)
  ASMJIT_INST_2x(vcvtqq2ph, Vcvtqq2ph, Vec, Mem)
  ASMJIT_INST_3x(vcvtsd2sh, Vcvtsd2sh, Vec, Vec, Vec)
  ASMJIT_INST_3x(vcvtsd2sh, Vcvtsd2sh, Vec, Vec, Mem)
  ASMJIT_INST_3x(vcvtsh2sd, Vcvtsh2sd, Vec, Vec, Vec)
  ASMJIT_INST_3x(vcvtsh2sd, Vcvtsh2sd, Vec, Vec, Mem)
  ASMJIT_INST_2x(vcvtsh2si, Vcvtsh2si, Gp, Vec)
  ASMJIT_INST_2x(vcvtsh2si, Vcvtsh2si, Gp, Mem)
  ASMJIT_INST_3x(vcvtsh2ss, Vcvtsh2ss, Vec, Vec, Vec)
  ASMJIT_INST_3x(vcvtsh2ss, Vcvtsh2ss, Vec, Vec, Mem)
  ASMJIT_INST_2x(vcvtsh2usi, Vcvtsh2usi, Gp, Vec)
  ASMJIT_INST_2x(vcvtsh2usi, Vcvtsh2usi, Gp, Mem)
  ASMJIT_INST_3x(vcvtsi2sh, Vcvtsi2sh, Vec, Vec, Gp)
  ASMJIT_INST_3x(vcvtsi2sh, Vcvtsi2sh, Vec, Vec, Mem)
  ASMJIT_INST_3x(vcvtss2sh, Vcvtss2sh, Vec, Vec, Vec)
  ASMJIT_INST_3x(vcvtss2sh, Vcvtss2sh, Vec, Vec, Mem)
  ASMJIT_INST_2x(vcvttph2dq, Vcvttph2dq, Vec, Vec)
  ASMJIT_INST_2x(vcvttph2dq, Vcvttph2dq, Vec, Mem)
  ASMJIT_INST_2x(vcvttph2qq, Vcvttph2qq, Vec, Vec)
  ASMJIT_INST_2x(vcvttph2qq, Vcvttph2qq, Vec, Mem)
  ASMJIT_INST_2x(vcvttph2udq, Vcvttph2udq, Vec, Vec)
  ASMJIT_INST_2x(vcvttph2udq, Vcvttph2udq, Vec, Mem)
  ASMJIT_INST_2x(vcvttph2uqq, Vcvttph2uqq, Vec, Vec)
  ASMJIT_INST_2x(vcvttph2uqq, Vcvttph2uqq, Vec, Mem)
  ASMJIT_INST_2x(vcvttph2uw, Vcvttph2uw, Vec, Vec)
  ASMJIT_INST_2x(vcvttph2uw, Vcvttph2uw, Vec, Mem)
  ASMJIT_INST_2x(vcvttph2w, Vcvttph2w, Vec, Vec)
  ASMJIT_INST_2x(vcvttph2w, Vcvttph2w, Vec, Mem)
  ASMJIT_INST_2x(vcvttsh2si, Vcvttsh2si, Gp, Vec)
  ASMJIT_INST_2x(vcvttsh2si, Vcvttsh2si, Gp, Mem)
  ASMJIT_INST_2x(vcvttsh2usi, Vcvttsh2usi, Gp, Vec)
  ASMJIT_INST_2x(vcvttsh2usi, Vcvttsh2usi, Gp, Mem)
  ASMJIT_INST_2x(vcvtudq2ph, Vcvtudq2ph, Vec, Vec)
  ASMJIT_INST_2x(vcvtudq2ph, Vcvtudq2ph, Vec, Mem)
  ASMJIT_INST_2x(vcvtuqq2ph, Vcvtuqq2ph, Vec, Vec)
  ASMJIT_INST_2x(vcvtuqq2ph, Vcvtuqq2ph, Vec, Mem)
  ASMJIT_INST_3x(vcvtusi2sh, Vcvtusi2sh, Vec, Vec, Gp)
  ASMJIT_INST_3x(vcvtusi2sh, Vcvtusi2sh, Vec, Vec, Mem)
  ASMJIT_INST_2x(vcvtuw2ph, Vcvtuw2ph, Vec, Vec)
  ASMJIT_INST_2x(vcvtuw2ph, Vcvtuw2ph, Vec, Mem)
  ASMJIT_INST_2x(vcvtw2ph, Vcvtw2ph, Vec, Vec)
  ASMJIT_INST_2x(vcvtw2ph, Vcvtw2ph, Vec, Mem)
  ASMJIT_INST_3x(vdivph, Vdivph, Vec, Vec, Vec)
  ASMJIT_INST_3x(vdivph, Vdivph, Vec, Vec, Mem)
  ASMJIT_INST_3x(vdivsh, Vdivsh, Vec, Vec, Vec)
  ASMJIT_INST_3x(vdivsh, Vdivsh, Vec, Vec, Mem)
  ASMJIT_INST_3x(vfcmaddcph, Vfcmaddcph, Vec, Vec, Vec)
  ASMJIT_INST_3x(vfcmaddcph, Vfcmaddcph, Vec, Vec, Mem)
  ASMJIT_INST_3x(vfcmaddcsh, Vfcmaddcsh, Vec, Vec, Vec)
  ASMJIT_INST_3x(vfcmaddcsh, Vfcmaddcsh, Vec, Vec, Mem)
  ASMJIT_INST_3x(vfcmulcph, Vfcmulcph, Vec, Vec, Vec)
  ASMJIT_INST_3x(vfcmulcph, Vfcmulcph, Vec, Vec, Mem)
  ASMJIT_INST_3x(vfcmulcsh, Vfcmulcsh, Vec, Vec, Vec)
  ASMJIT_INST_3x(vfcmulcsh, Vfcmulcsh, Vec, Vec, Mem)
  ASMJIT_INST_3x(vfmadd132ph, Vfmadd132ph, Vec, Vec, Vec)
  ASMJIT_INST_3x(vfmadd132ph, Vfmadd132ph, Vec, Vec, Mem)
  ASMJIT_INST_3x(vfmadd132sh, Vfmadd132sh, Vec, Vec, Vec)
  ASMJIT_INST_3x(vfmadd132sh, Vfmadd132sh, Vec, Vec, Mem)
  ASMJIT_INST_3x(vfmadd213ph, Vfmadd213ph, Vec, Vec, Vec)
  ASMJIT_INST_3x(vfmadd213ph, Vfmadd213ph, Vec, Vec, Mem)
  ASMJIT_INST_3x(vfmadd213sh, Vfmadd213sh, Vec, Vec, Vec)
  ASMJIT_INST_3x(vfmadd213sh, Vfmadd213sh, Vec, Vec, Mem)
  ASMJIT_INST_3x(vfmadd231ph, Vfmadd231ph, Vec, Vec, Vec)
  ASMJIT_INST_3x(vfmadd231ph, Vfmadd231ph, Vec, Vec, Mem)
  ASMJIT_INST_3x(vfmadd231sh, Vfmadd231sh, Vec, Vec, Vec)
  ASMJIT_INST_3x(vfmadd231sh, Vfmadd231sh, Vec, Vec, Mem)
  ASMJIT_INST_3x(vfmaddcph, Vfmaddcph, Vec, Vec, Vec)
  ASMJIT_INST_3x(vfmaddcph, Vfmaddcph, Vec, Vec, Mem)
  ASMJIT_INST_3x(vfmaddcsh, Vfmaddcsh, Vec, Vec, Vec)
  ASMJIT_INST_3x(vfmaddcsh, Vfmaddcsh, Vec, Vec, Mem)
  ASMJIT_INST_3x(vfmaddsub132ph, Vfmaddsub132ph, Vec, Vec, Vec)
  ASMJIT_INST_3x(vfmaddsub132ph, Vfmaddsub132ph, Vec, Vec, Mem)
  ASMJIT_INST_3x(vfmaddsub213ph, Vfmaddsub213ph, Vec, Vec, Vec)
  ASMJIT_INST_3x(vfmaddsub213ph, Vfmaddsub213ph, Vec, Vec, Mem)
  ASMJIT_INST_3x(vfmaddsub231ph, Vfmaddsub231ph, Vec, Vec, Vec)
  ASMJIT_INST_3x(vfmaddsub231ph, Vfmaddsub231ph, Vec, Vec, Mem)
  ASMJIT_INST_3x(vfmsub132ph, Vfmsub132ph, Vec, Vec, Vec)
  ASMJIT_INST_3x(vfmsub132ph, Vfmsub132ph, Vec, Vec, Mem)
  ASMJIT_INST_3x(vfmsub132sh, Vfmsub132sh, Vec, Vec, Vec)
  ASMJIT_INST_3x(vfmsub132sh, Vfmsub132sh, Vec, Vec, Mem)
  ASMJIT_INST_3x(vfmsub213ph, Vfmsub213ph, Vec, Vec, Vec)
  ASMJIT_INST_3x(vfmsub213ph, Vfmsub213ph, Vec, Vec, Mem)
  ASMJIT_INST_3x(vfmsub213sh, Vfmsub213sh, Vec, Vec, Vec)
  ASMJIT_INST_3x(vfmsub213sh, Vfmsub213sh, Vec, Vec, Mem)
  ASMJIT_INST_3x(vfmsub231ph, Vfmsub231ph, Vec, Vec, Vec)
  ASMJIT_INST_3x(vfmsub231ph, Vfmsub231ph, Vec, Vec, Mem)
  ASMJIT_INST_3x(vfmsub231sh, Vfmsub231sh, Vec, Vec, Vec)
  ASMJIT_INST_3x(vfmsub231sh, Vfmsub231sh, Vec, Vec, Mem)
  ASMJIT_INST_3x(vfmsubadd132ph, Vfmsubadd132ph, Vec, Vec, Vec)
  ASMJIT_INST_3x(vfmsubadd132ph, Vfmsubadd132ph, Vec, Vec, Mem)
  ASMJIT_INST_3x(vfmsubadd213ph, Vfmsubadd213ph, Vec, Vec, Vec)
  ASMJIT_INST_3x(vfmsubadd213ph, Vfmsubadd213ph, Vec, Vec, Mem)
  ASMJIT_INST_3x(vfmsubadd231ph, Vfmsubadd231ph, Vec, Vec, Vec)
  ASMJIT_INST_3x(vfmsubadd231ph, Vfmsubadd231ph, Vec, Vec, Mem)
  ASMJIT_INST_3x(vfmulcph, Vfmulcph, Vec, Vec, Vec)
  ASMJIT_INST_3x(vfmulcph, Vfmulcph, Vec, Vec, Mem)
  ASMJIT_INST_3x(vfmulcsh, Vfmulcsh, Vec, Vec, Vec)
  ASMJIT_INST_3x(vfmulcsh, Vfmulcsh, Vec, Vec, Mem)
  ASMJIT_INST_3x(vfnmadd132ph, Vfnmadd132ph, Vec, Vec, Vec)
  ASMJIT_INST_3x(vfnmadd132ph, Vfnmadd132ph, Vec, Vec, Mem)
  ASMJIT_INST_3x(vfnmadd132sh, Vfnmadd132sh, Vec, Vec, Vec)
  ASMJIT_INST_3x(vfnmadd132sh, Vfnmadd132sh, Vec, Vec, Mem)
  ASMJIT_INST_3x(vfnmadd213ph, Vfnmadd213ph, Vec, Vec, Vec)
  ASMJIT_INST_3x(vfnmadd213ph, Vfnmadd213ph, Vec, Vec, Mem)
  ASMJIT_INST_3x(vfnmadd213sh, Vfnmadd213sh, Vec, Vec, Vec)
  ASMJIT_INST_3x(vfnmadd213sh, Vfnmadd213sh, Vec, Vec, Mem)
  ASMJIT_INST_3x(vfnmadd231ph, Vfnmadd231ph, Vec, Vec, Vec)
  ASMJIT_INST_3x(vfnmadd231ph, Vfnmadd231ph, Vec, Vec, Mem)
  ASMJIT_INST_3x(vfnmadd231sh, Vfnmadd231sh, Vec, Vec, Vec)
  ASMJIT_INST_3x(vfnmadd231sh, Vfnmadd231sh, Vec, Vec, Mem)
  ASMJIT_INST_3x(vfnmsub132ph, Vfnmsub132ph, Vec, Vec, Vec)
  ASMJIT_INST_3x(vfnmsub132ph, Vfnmsub132ph, Vec, Vec, Mem)
  ASMJIT_INST_3x(vfnmsub132sh, Vfnmsub132sh, Vec, Vec, Vec)
  ASMJIT_INST_3x(vfnmsub132sh, Vfnmsub132sh, Vec, Vec, Mem)
  ASMJIT_INST_3x(vfnmsub213ph, Vfnmsub213ph, Vec, Vec, Vec)
  ASMJIT_INST_3x(vfnmsub213ph, Vfnmsub213ph, Vec, Vec, Mem)
  ASMJIT_INST_3x(vfnmsub213sh, Vfnmsub213sh, Vec, Vec, Vec)
  ASMJIT_INST_3x(vfnmsub213sh, Vfnmsub213sh, Vec, Vec, Mem)
  ASMJIT_INST_3x(vfnmsub231ph, Vfnmsub231ph, Vec, Vec, Vec)
  ASMJIT_INST_3x(vfnmsub231ph, Vfnmsub231ph, Vec, Vec, Mem)
  ASMJIT_INST_3x(vfnmsub231sh, Vfnmsub231sh, Vec, Vec, Vec)
  ASMJIT_INST_3x(vfnmsub231sh, Vfnmsub231sh, Vec, Vec, Mem)
  ASMJIT_INST_3x(vfpclassph, Vfpclassph, KReg, Vec, Imm)
  ASMJIT_INST_3x(vfpclassph, Vfpclassph, KReg, Mem, Imm)
  ASMJIT_INST_3x(vfpclasssh, Vfpclasssh, KReg, Vec, Imm)
  ASMJIT_INST_3x(vfpclasssh, Vfpclasssh, KReg, Mem, Imm)
  ASMJIT_INST_2x(vgetexpph, Vgetexpph, Vec, Vec)
  ASMJIT_INST_2x(vgetexpph, Vgetexpph, Vec, Mem)
  ASMJIT_INST_3x(vgetexpsh, Vgetexpsh, Vec, Vec, Vec)
  ASMJIT_INST_3x(vgetexpsh, Vgetexpsh, Vec, Vec, Mem)
  ASMJIT_INST_3x(vgetmantph, Vgetmantph, Vec, Vec, Imm)
  ASMJIT_INST_3x(vgetmantph, Vgetmantph, Vec, Mem, Imm)
  ASMJIT_INST_4x(vgetmantsh, Vgetmantsh, Vec, Vec, Vec, Imm)
  ASMJIT_INST_4x(vgetmantsh, Vgetmantsh, Vec, Vec, Mem, Imm)
  ASMJIT_INST_3x(vmaxph, Vmaxph, Vec, Vec, Vec)
  ASMJIT_INST_3x(vmaxph, Vmaxph, Vec, Vec, Mem)
  ASMJIT_INST_3x(vmaxsh, Vmaxsh, Vec, Vec, Vec)
  ASMJIT_INST_3x(vmaxsh, Vmaxsh, Vec, Vec, Mem)
  ASMJIT_INST_3x(vminph, Vminph, Vec, Vec, Vec)
  ASMJIT_INST_3x(vminph, Vminph, Vec, Vec, Mem)
  ASMJIT_INST_3x(vminsh, Vminsh, Vec, Vec, Vec)
  ASMJIT_INST_3x(vminsh, Vminsh, Vec, Vec, Mem)
  ASMJIT_INST_2x(vmovsh, Vmovsh, Mem, Vec)
  ASMJIT_INST_2x(vmovsh, Vmovsh, Vec, Mem)
  ASMJIT_INST_3x(vmovsh, Vmovsh, Vec, Vec, Vec)
  ASMJIT_INST_2x(vmovw, Vmovw, Gp, Vec)
  ASMJIT_INST_2x(vmovw, Vmovw, Mem, Vec)
  ASMJIT_INST_2x(vmovw, Vmovw, Vec, Gp)
  ASMJIT_INST_2x(vmovw, Vmovw, Vec, Mem)
  ASMJIT_INST_3x(vmulph, Vmulph, Vec, Vec, Vec)
  ASMJIT_INST_3x(vmulph, Vmulph, Vec, Vec, Mem)
  ASMJIT_INST_3x(vmulsh, Vmulsh, Vec, Vec, Vec)
  ASMJIT_INST_3x(vmulsh, Vmulsh, Vec, Vec, Mem)
  ASMJIT_INST_2x(vrcpph, Vrcpph, Vec, Vec)
  ASMJIT_INST_2x(vrcpph, Vrcpph, Vec, Mem)
  ASMJIT_INST_3x(vrcpsh, Vrcpsh, Vec, Vec, Vec)
  ASMJIT_INST_3x(vrcpsh, Vrcpsh, Vec, Vec, Mem)
  ASMJIT_INST_3x(vreduceph, Vreduceph, Vec, Vec, Imm)
  ASMJIT_INST_3x(vreduceph, Vreduceph, Vec, Mem, Imm)
  ASMJIT_INST_4x(vreducesh, Vreducesh, Vec, Vec, Vec, Imm)
  ASMJIT_INST_4x(vreducesh, Vreducesh, Vec, Vec, Mem, Imm)
  ASMJIT_INST_3x(vrndscaleph, Vrndscaleph, Vec, Vec, Imm)
  ASMJIT_INST_3x(vrndscaleph, Vrndscaleph, Vec, Mem, Imm)
  ASMJIT_INST_4x(vrndscalesh, Vrndscalesh, Vec, Vec, Vec, Imm)
  ASMJIT_INST_4x(vrndscalesh, Vrndscalesh, Vec, Vec, Mem, Imm)
  ASMJIT_INST_2x(vrsqrtph, Vrsqrtph, Vec, Vec)
  ASMJIT_INST_2x(vrsqrtph, Vrsqrtph, Vec, Mem)
  ASMJIT_INST_3x(vrsqrtsh, Vrsqrtsh, Vec, Vec, Vec)
  ASMJIT_INST_3x(vrsqrtsh, Vrsqrtsh, Vec, Vec, Mem)
  ASMJIT_INST_3x(vscalefph, Vscalefph, Vec, Vec, Vec)
  ASMJIT_INST_3x(vscalefph, Vscalefph, Vec, Vec, Mem)
  ASMJIT_INST_3x(vscalefsh, Vscalefsh, Vec, Vec, Vec)
  ASMJIT_INST_3x(vscalefsh, Vscalefsh, Vec, Vec, Mem)
  ASMJIT_INST_2x(vsqrtph, Vsqrtph, Vec, Vec)
  ASMJIT_INST_2x(vsqrtph, Vsqrtph, Vec, Mem)
  ASMJIT_INST_3x(vsqrtsh, Vsqrtsh, Vec, Vec, Vec)
  ASMJIT_INST_3x(vsqrtsh, Vsqrtsh, Vec, Vec, Mem)
  ASMJIT_INST_3x(vsubph, Vsubph, Vec, Vec, Vec)
  ASMJIT_INST_3x(vsubph, Vsubph, Vec, Vec, Mem)
  ASMJIT_INST_3x(vsubsh, Vsubsh, Vec, Vec, Vec)
  ASMJIT_INST_3x(vsubsh, Vsubsh, Vec, Vec, Mem)
  ASMJIT_INST_2x(vucomish, Vucomish, Vec, Vec)
  ASMJIT_INST_2x(vucomish, Vucomish, Vec, Mem)

  //! \}

  //! \name AMX_TILE Instructions
  //! \{

  ASMJIT_INST_1x(ldtilecfg, Ldtilecfg, Mem)
  ASMJIT_INST_1x(sttilecfg, Sttilecfg, Mem)
  ASMJIT_INST_2x(tileloadd, Tileloadd, Tmm, Mem)
  ASMJIT_INST_2x(tileloaddt1, Tileloaddt1, Tmm, Mem)
  ASMJIT_INST_0x(tilerelease, Tilerelease)
  ASMJIT_INST_2x(tilestored, Tilestored, Mem, Tmm)
  ASMJIT_INST_1x(tilezero, Tilezero, Tmm)

  //! \}

  //! \name AMX_BF16 Instructions
  //! \{

  ASMJIT_INST_3x(tdpbf16ps, Tdpbf16ps, Tmm, Tmm, Tmm)

  //! \}

  //! \name AMX_COMPLEX Instructions
  //! \{

  ASMJIT_INST_3x(tcmmimfp16ps, Tcmmimfp16ps, Tmm, Tmm, Tmm)
  ASMJIT_INST_3x(tcmmrlfp16ps, Tcmmrlfp16ps, Tmm, Tmm, Tmm)

  //! \}

  //! \name AMX_FP16 Instructions
  //! \{

  ASMJIT_INST_3x(tdpfp16ps, Tdpfp16ps, Tmm, Tmm, Tmm)

  //! \}

  //! \name AMX_INT8 Instructions
  //! \{

  ASMJIT_INST_3x(tdpbssd, Tdpbssd, Tmm, Tmm, Tmm)
  ASMJIT_INST_3x(tdpbsud, Tdpbsud, Tmm, Tmm, Tmm)
  ASMJIT_INST_3x(tdpbusd, Tdpbusd, Tmm, Tmm, Tmm)
  ASMJIT_INST_3x(tdpbuud, Tdpbuud, Tmm, Tmm, Tmm)

  //! \}
};

//! Emitter (X86 - implicit).
template<typename This>
struct EmitterImplicitT : public EmitterExplicitT<This> {
  //! \cond
  using EmitterExplicitT<This>::_emitter;
  //! \endcond

  //! \name Prefix Options
  //! \{

  //! Use REP/REPE prefix.
  inline This& rep() noexcept { return EmitterExplicitT<This>::_add_inst_options(InstOptions::kX86_Rep); }
  //! Use REP/REPE prefix.
  inline This& repe() noexcept { return rep(); }
  //! Use REP/REPE prefix.
  inline This& repz() noexcept { return rep(); }

  //! Use REPNE prefix.
  inline This& repne() noexcept { return EmitterExplicitT<This>::_add_inst_options(InstOptions::kX86_Repne); }
  //! Use REPNE prefix.
  inline This& repnz() noexcept { return repne(); }

  //! \}

  //! \name Core Instructions
  //! \{

  //! \cond
  using EmitterExplicitT<This>::cbw;
  using EmitterExplicitT<This>::cdq;
  using EmitterExplicitT<This>::cdqe;
  using EmitterExplicitT<This>::cqo;
  using EmitterExplicitT<This>::cwd;
  using EmitterExplicitT<This>::cwde;
  using EmitterExplicitT<This>::cmpsd;
  using EmitterExplicitT<This>::cmpxchg;
  using EmitterExplicitT<This>::cmpxchg8b;
  using EmitterExplicitT<This>::cmpxchg16b;
  using EmitterExplicitT<This>::div;
  using EmitterExplicitT<This>::idiv;
  using EmitterExplicitT<This>::imul;
  using EmitterExplicitT<This>::jecxz;
  using EmitterExplicitT<This>::loop;
  using EmitterExplicitT<This>::loope;
  using EmitterExplicitT<This>::loopne;
  using EmitterExplicitT<This>::mul;
  //! \endcond

  ASMJIT_INST_0x(cbw, Cbw)                                             // ANY       [IMPLICIT] AX      <- Sign Extend AL
  ASMJIT_INST_0x(cdq, Cdq)                                             // ANY       [IMPLICIT] EDX:EAX <- Sign Extend EAX
  ASMJIT_INST_0x(cdqe, Cdqe)                                           // X64       [IMPLICIT] RAX     <- Sign Extend EAX
  ASMJIT_INST_2x(cmpxchg, Cmpxchg, Gp, Gp)                             // I486      [IMPLICIT]
  ASMJIT_INST_2x(cmpxchg, Cmpxchg, Mem, Gp)                            // I486      [IMPLICIT]
  ASMJIT_INST_1x(cmpxchg16b, Cmpxchg16b, Mem)                          // CMPXCHG8B [IMPLICIT] m == RDX:RAX ? m <- RCX:RBX
  ASMJIT_INST_1x(cmpxchg8b, Cmpxchg8b, Mem)                            // CMPXCHG16B[IMPLICIT] m == EDX:EAX ? m <- ECX:EBX
  ASMJIT_INST_0x(cqo, Cqo)                                             // X64       [IMPLICIT] RDX:RAX <- Sign Extend RAX
  ASMJIT_INST_0x(cwd, Cwd)                                             // ANY       [IMPLICIT] DX:AX   <- Sign Extend AX
  ASMJIT_INST_0x(cwde, Cwde)                                           // ANY       [IMPLICIT] EAX     <- Sign Extend AX
  ASMJIT_INST_1x(div, Div, Gp)                                         // ANY       [IMPLICIT] {AH[Rem]: AL[Quot] <- AX / r8} {xDX[Rem]:xAX[Quot] <- DX:AX / r16|r32|r64}
  ASMJIT_INST_1x(div, Div, Mem)                                        // ANY       [IMPLICIT] {AH[Rem]: AL[Quot] <- AX / m8} {xDX[Rem]:xAX[Quot] <- DX:AX / m16|m32|m64}
  ASMJIT_INST_1x(idiv, Idiv, Gp)                                       // ANY       [IMPLICIT] {AH[Rem]: AL[Quot] <- AX / r8} {xDX[Rem]:xAX[Quot] <- DX:AX / r16|r32|r64}
  ASMJIT_INST_1x(idiv, Idiv, Mem)                                      // ANY       [IMPLICIT] {AH[Rem]: AL[Quot] <- AX / m8} {xDX[Rem]:xAX[Quot] <- DX:AX / m16|m32|m64}
  ASMJIT_INST_1x(imul, Imul, Gp)                                       // ANY       [IMPLICIT] {AX <- AL * r8} {xAX:xDX <- xAX * r16|r32|r64}
  ASMJIT_INST_1x(imul, Imul, Mem)                                      // ANY       [IMPLICIT] {AX <- AL * m8} {xAX:xDX <- xAX * m16|m32|m64}
  ASMJIT_INST_0x(iret, Iret)                                           // ANY       [IMPLICIT]
  ASMJIT_INST_0x(iretd, Iretd)                                         // ANY       [IMPLICIT]
  ASMJIT_INST_0x(iretq, Iretq)                                         // X64       [IMPLICIT]
  ASMJIT_INST_1x(jecxz, Jecxz, Label)                                  // ANY       [IMPLICIT] Short jump if CX/ECX/RCX is zero.
  ASMJIT_INST_1x(jecxz, Jecxz, Imm)                                    // ANY       [IMPLICIT] Short jump if CX/ECX/RCX is zero.
  ASMJIT_INST_1x(loop, Loop, Label)                                    // ANY       [IMPLICIT] Decrement xCX; short jump if xCX != 0.
  ASMJIT_INST_1x(loop, Loop, Imm)                                      // ANY       [IMPLICIT] Decrement xCX; short jump if xCX != 0.
  ASMJIT_INST_1x(loope, Loope, Label)                                  // ANY       [IMPLICIT] Decrement xCX; short jump if xCX != 0 && ZF == 1.
  ASMJIT_INST_1x(loope, Loope, Imm)                                    // ANY       [IMPLICIT] Decrement xCX; short jump if xCX != 0 && ZF == 1.
  ASMJIT_INST_1x(loopne, Loopne, Label)                                // ANY       [IMPLICIT] Decrement xCX; short jump if xCX != 0 && ZF == 0.
  ASMJIT_INST_1x(loopne, Loopne, Imm)                                  // ANY       [IMPLICIT] Decrement xCX; short jump if xCX != 0 && ZF == 0.
  ASMJIT_INST_1x(mul, Mul, Gp)                                         // ANY       [IMPLICIT] {AX <- AL * r8} {xDX:xAX <- xAX * r16|r32|r64}
  ASMJIT_INST_1x(mul, Mul, Mem)                                        // ANY       [IMPLICIT] {AX <- AL * m8} {xDX:xAX <- xAX * m16|m32|m64}
  ASMJIT_INST_0x(ret, Ret)
  ASMJIT_INST_1x(ret, Ret, Imm)
  ASMJIT_INST_0x(retf, Retf)
  ASMJIT_INST_1x(retf, Retf, Imm)
  ASMJIT_INST_0x(xlatb, Xlatb)                                         // ANY       [IMPLICIT]

  //! \}

  //! \name String Instruction Aliases
  //! \{

  //! \cond
  using EmitterExplicitT<This>::movsd;
  //! \endcond

  inline Error cmpsb() { return _emitter()->emit(Inst::kIdCmps, EmitterExplicitT<This>::ptr_zsi(0, 1), EmitterExplicitT<This>::ptr_zdi(0, 1)); }
  inline Error cmpsd() { return _emitter()->emit(Inst::kIdCmps, EmitterExplicitT<This>::ptr_zsi(0, 4), EmitterExplicitT<This>::ptr_zdi(0, 4)); }
  inline Error cmpsq() { return _emitter()->emit(Inst::kIdCmps, EmitterExplicitT<This>::ptr_zsi(0, 8), EmitterExplicitT<This>::ptr_zdi(0, 8)); }
  inline Error cmpsw() { return _emitter()->emit(Inst::kIdCmps, EmitterExplicitT<This>::ptr_zsi(0, 2), EmitterExplicitT<This>::ptr_zdi(0, 2)); }

  inline Error lodsb() { return _emitter()->emit(Inst::kIdLods, al , EmitterExplicitT<This>::ptr_zsi(0, 1)); }
  inline Error lodsd() { return _emitter()->emit(Inst::kIdLods, eax, EmitterExplicitT<This>::ptr_zsi(0, 4)); }
  inline Error lodsq() { return _emitter()->emit(Inst::kIdLods, rax, EmitterExplicitT<This>::ptr_zsi(0, 8)); }
  inline Error lodsw() { return _emitter()->emit(Inst::kIdLods, ax , EmitterExplicitT<This>::ptr_zsi(0, 2)); }

  inline Error movsb() { return _emitter()->emit(Inst::kIdMovs, EmitterExplicitT<This>::ptr_zdi(0, 1), EmitterExplicitT<This>::ptr_zsi(0, 1)); }
  inline Error movsd() { return _emitter()->emit(Inst::kIdMovs, EmitterExplicitT<This>::ptr_zdi(0, 4), EmitterExplicitT<This>::ptr_zsi(0, 4)); }
  inline Error movsq() { return _emitter()->emit(Inst::kIdMovs, EmitterExplicitT<This>::ptr_zdi(0, 8), EmitterExplicitT<This>::ptr_zsi(0, 8)); }
  inline Error movsw() { return _emitter()->emit(Inst::kIdMovs, EmitterExplicitT<This>::ptr_zdi(0, 2), EmitterExplicitT<This>::ptr_zsi(0, 2)); }

  inline Error scasb() { return _emitter()->emit(Inst::kIdScas, al , EmitterExplicitT<This>::ptr_zdi(0, 1)); }
  inline Error scasd() { return _emitter()->emit(Inst::kIdScas, eax, EmitterExplicitT<This>::ptr_zdi(0, 4)); }
  inline Error scasq() { return _emitter()->emit(Inst::kIdScas, rax, EmitterExplicitT<This>::ptr_zdi(0, 8)); }
  inline Error scasw() { return _emitter()->emit(Inst::kIdScas, ax , EmitterExplicitT<This>::ptr_zdi(0, 2)); }

  inline Error stosb() { return _emitter()->emit(Inst::kIdStos, EmitterExplicitT<This>::ptr_zdi(0, 1), al ); }
  inline Error stosd() { return _emitter()->emit(Inst::kIdStos, EmitterExplicitT<This>::ptr_zdi(0, 4), eax); }
  inline Error stosq() { return _emitter()->emit(Inst::kIdStos, EmitterExplicitT<This>::ptr_zdi(0, 8), rax); }
  inline Error stosw() { return _emitter()->emit(Inst::kIdStos, EmitterExplicitT<This>::ptr_zdi(0, 2), ax ); }

  //! \}

  //! \name Deprecated 32-bit Instructions
  //! \{

  //! \cond
  using EmitterExplicitT<This>::aaa;
  using EmitterExplicitT<This>::aad;
  using EmitterExplicitT<This>::aam;
  using EmitterExplicitT<This>::aas;
  using EmitterExplicitT<This>::daa;
  using EmitterExplicitT<This>::das;
  //! \endcond

  ASMJIT_INST_0x(aaa, Aaa)                                             // X86 [IMPLICIT]
  ASMJIT_INST_1x(aad, Aad, Imm)                                        // X86 [IMPLICIT]
  ASMJIT_INST_1x(aam, Aam, Imm)                                        // X86 [IMPLICIT]
  ASMJIT_INST_0x(aas, Aas)                                             // X86 [IMPLICIT]
  ASMJIT_INST_0x(daa, Daa)                                             // X86 [IMPLICIT]
  ASMJIT_INST_0x(das, Das)                                             // X86 [IMPLICIT]

  //! \}

  //! \name LAHF/SAHF Instructions
  //! \{

  //! \cond
  using EmitterExplicitT<This>::lahf;
  using EmitterExplicitT<This>::sahf;
  //! \endcond

  ASMJIT_INST_0x(lahf, Lahf)                                           // LAHFSAHF  [IMPLICIT] AH <- EFL
  ASMJIT_INST_0x(sahf, Sahf)                                           // LAHFSAHF  [IMPLICIT] EFL <- AH

  //! \}

  //! \name CPUID Instruction
  //! \{

  //! \cond
  using EmitterExplicitT<This>::cpuid;
  //! \endcond

  ASMJIT_INST_0x(cpuid, Cpuid)                                         // I486      [IMPLICIT] EAX:EBX:ECX:EDX  <- CPUID[EAX:ECX]

  //! \}

  //! \name CacheLine Instructions
  //! \{

  //! \cond
  using EmitterExplicitT<This>::clzero;
  //! \endcond

  ASMJIT_INST_0x(clzero, Clzero)                                       // CLZERO    [IMPLICIT]

  //! \}

  //! \name RDPRU/RDPKRU Instructions
  //! \{

  //! \cond
  using EmitterExplicitT<This>::rdpru;
  using EmitterExplicitT<This>::rdpkru;
  //! \endcond

  ASMJIT_INST_0x(rdpru, Rdpru)                                         // RDPRU     [IMPLICIT] EDX:EAX <- PRU[ECX]
  ASMJIT_INST_0x(rdpkru, Rdpkru)                                       // RDPKRU    [IMPLICIT] EDX:EAX <- PKRU[ECX]

  //! \}

  //! \name RDTSC/RDTSCP Instructions
  //! \{

  //! \cond
  using EmitterExplicitT<This>::rdtsc;
  using EmitterExplicitT<This>::rdtscp;
  //! \endcond

  ASMJIT_INST_0x(rdtsc, Rdtsc)                                         // RDTSC     [IMPLICIT] EDX:EAX <- CNT
  ASMJIT_INST_0x(rdtscp, Rdtscp)                                       // RDTSCP    [IMPLICIT] EDX:EAX:EXC <- CNT

  //! \}

  //! \name BMI2 Instructions
  //! \{

  //! \cond
  using EmitterExplicitT<This>::mulx;
  //! \endcond

  ASMJIT_INST_3x(mulx, Mulx, Gp, Gp, Gp)                               // BMI2      [IMPLICIT]
  ASMJIT_INST_3x(mulx, Mulx, Gp, Gp, Mem)                              // BMI2      [IMPLICIT]

  //! \}

  //! \name XSAVE Instructions
  //! \{

  //! \cond
  using EmitterExplicitT<This>::xgetbv;
  using EmitterExplicitT<This>::xrstor;
  using EmitterExplicitT<This>::xrstor64;
  using EmitterExplicitT<This>::xrstors;
  using EmitterExplicitT<This>::xrstors64;
  using EmitterExplicitT<This>::xsave;
  using EmitterExplicitT<This>::xsave64;
  using EmitterExplicitT<This>::xsavec;
  using EmitterExplicitT<This>::xsavec64;
  using EmitterExplicitT<This>::xsaveopt;
  using EmitterExplicitT<This>::xsaveopt64;
  using EmitterExplicitT<This>::xsaves;
  using EmitterExplicitT<This>::xsaves64;
  //! \endcond

  ASMJIT_INST_0x(xgetbv, Xgetbv)                                       // XSAVE     [IMPLICIT] EDX:EAX <- XCR[ECX]
  ASMJIT_INST_1x(xrstor, Xrstor, Mem)                                  // XSAVE     [IMPLICIT]
  ASMJIT_INST_1x(xrstor64, Xrstor64, Mem)                              // XSAVE+X64 [IMPLICIT]
  ASMJIT_INST_1x(xrstors, Xrstors, Mem)                                // XSAVE     [IMPLICIT]
  ASMJIT_INST_1x(xrstors64, Xrstors64, Mem)                            // XSAVE+X64 [IMPLICIT]
  ASMJIT_INST_1x(xsave, Xsave, Mem)                                    // XSAVE     [IMPLICIT]
  ASMJIT_INST_1x(xsave64, Xsave64, Mem)                                // XSAVE+X64 [IMPLICIT]
  ASMJIT_INST_1x(xsavec, Xsavec, Mem)                                  // XSAVE     [IMPLICIT]
  ASMJIT_INST_1x(xsavec64, Xsavec64, Mem)                              // XSAVE+X64 [IMPLICIT]
  ASMJIT_INST_1x(xsaveopt, Xsaveopt, Mem)                              // XSAVE     [IMPLICIT]
  ASMJIT_INST_1x(xsaveopt64, Xsaveopt64, Mem)                          // XSAVE+X64 [IMPLICIT]
  ASMJIT_INST_1x(xsaves, Xsaves, Mem)                                  // XSAVE     [IMPLICIT]
  ASMJIT_INST_1x(xsaves64, Xsaves64, Mem)                              // XSAVE+X64 [IMPLICIT]

  //! \}

  //! \name SYSCALL/SYSENTER Instructions
  //! \{

  ASMJIT_INST_0x(syscall, Syscall)                                     // X64       [IMPLICIT]
  ASMJIT_INST_0x(sysenter, Sysenter)                                   // X64       [IMPLICIT]

  //! \}

  //! \name HRESET Instructions
  //! \{

  //! \cond
  using EmitterExplicitT<This>::hreset;
  //! \endcond

  ASMJIT_INST_1x(hreset, Hreset, Imm)                                  // HRESET    [IMPLICIT]

  //! \}

  //! \name SEAM Instructions
  //! \{

  ASMJIT_INST_0x(seamcall, Seamcall)
  ASMJIT_INST_0x(seamops, Seamops)
  ASMJIT_INST_0x(seamret, Seamret)
  ASMJIT_INST_0x(tdcall, Tdcall)

  //! \}

  //! \name Privileged Instructions
  //! \{

  //! \cond
  using EmitterExplicitT<This>::rdmsr;
  using EmitterExplicitT<This>::rdpmc;
  using EmitterExplicitT<This>::wrmsr;
  using EmitterExplicitT<This>::xsetbv;
  //! \endcond

  ASMJIT_INST_0x(pconfig, Pconfig)                                     // PCONFIG   [IMPLICIT]
  ASMJIT_INST_0x(rdmsr, Rdmsr)                                         // ANY       [IMPLICIT]
  ASMJIT_INST_0x(rdpmc, Rdpmc)                                         // ANY       [IMPLICIT]
  ASMJIT_INST_0x(sysexit, Sysexit)                                     // X64       [IMPLICIT]
  ASMJIT_INST_0x(sysexitq, Sysexitq)                                   // X64       [IMPLICIT]
  ASMJIT_INST_0x(sysret, Sysret)                                       // X64       [IMPLICIT]
  ASMJIT_INST_0x(sysretq, Sysretq)                                     // X64       [IMPLICIT]
  ASMJIT_INST_0x(wrmsr, Wrmsr)                                         // ANY       [IMPLICIT]
  ASMJIT_INST_0x(xsetbv, Xsetbv)                                       // XSAVE     [IMPLICIT] XCR[ECX] <- EDX:EAX

  //! \}

  //! \name Monitor & MWait Instructions
  //! \{

  //! \cond
  using EmitterExplicitT<This>::monitor;
  using EmitterExplicitT<This>::monitorx;
  using EmitterExplicitT<This>::mwait;
  using EmitterExplicitT<This>::mwaitx;
  //! \endcond

  ASMJIT_INST_0x(monitor, Monitor)
  ASMJIT_INST_0x(monitorx, Monitorx)
  ASMJIT_INST_0x(mwait, Mwait)
  ASMJIT_INST_0x(mwaitx, Mwaitx)

  //! \}

  //! \name WAITPKG Instructions
  //! \{

  //! \cond
  using EmitterExplicitT<This>::tpause;
  using EmitterExplicitT<This>::umwait;
  //! \endcond

  ASMJIT_INST_1x(tpause, Tpause, Gp)
  ASMJIT_INST_1x(umwait, Umwait, Gp)

  //! \}

  //! \name MMX & SSE Instructions
  //! \{

  //! \cond
  using EmitterExplicitT<This>::blendvpd;
  using EmitterExplicitT<This>::blendvps;
  using EmitterExplicitT<This>::maskmovq;
  using EmitterExplicitT<This>::maskmovdqu;
  using EmitterExplicitT<This>::pblendvb;
  using EmitterExplicitT<This>::pcmpestri;
  using EmitterExplicitT<This>::pcmpestrm;
  using EmitterExplicitT<This>::pcmpistri;
  using EmitterExplicitT<This>::pcmpistrm;
  //! \endcond

  ASMJIT_INST_2x(blendvpd, Blendvpd, Vec, Vec)                         // SSE4_1 [IMPLICIT]
  ASMJIT_INST_2x(blendvpd, Blendvpd, Vec, Mem)                         // SSE4_1 [IMPLICIT]
  ASMJIT_INST_2x(blendvps, Blendvps, Vec, Vec)                         // SSE4_1 [IMPLICIT]
  ASMJIT_INST_2x(blendvps, Blendvps, Vec, Mem)                         // SSE4_1 [IMPLICIT]
  ASMJIT_INST_2x(pblendvb, Pblendvb, Vec, Vec)                         // SSE4_1 [IMPLICIT]
  ASMJIT_INST_2x(pblendvb, Pblendvb, Vec, Mem)                         // SSE4_1 [IMPLICIT]
  ASMJIT_INST_2x(maskmovq, Maskmovq, Mm, Mm)                           // SSE    [IMPLICIT]
  ASMJIT_INST_2x(maskmovdqu, Maskmovdqu, Vec, Vec)                     // SSE2   [IMPLICIT]
  ASMJIT_INST_3x(pcmpestri, Pcmpestri, Vec, Vec, Imm)                  // SSE4_1 [IMPLICIT]
  ASMJIT_INST_3x(pcmpestri, Pcmpestri, Vec, Mem, Imm)                  // SSE4_1 [IMPLICIT]
  ASMJIT_INST_3x(pcmpestrm, Pcmpestrm, Vec, Vec, Imm)                  // SSE4_1 [IMPLICIT]
  ASMJIT_INST_3x(pcmpestrm, Pcmpestrm, Vec, Mem, Imm)                  // SSE4_1 [IMPLICIT]
  ASMJIT_INST_3x(pcmpistri, Pcmpistri, Vec, Vec, Imm)                  // SSE4_1 [IMPLICIT]
  ASMJIT_INST_3x(pcmpistri, Pcmpistri, Vec, Mem, Imm)                  // SSE4_1 [IMPLICIT]
  ASMJIT_INST_3x(pcmpistrm, Pcmpistrm, Vec, Vec, Imm)                  // SSE4_1 [IMPLICIT]
  ASMJIT_INST_3x(pcmpistrm, Pcmpistrm, Vec, Mem, Imm)                  // SSE4_1 [IMPLICIT]

  //! \}

  //! \name SHA Instructions
  //! \{

  //! \cond
  using EmitterExplicitT<This>::sha256rnds2;
  //! \endcond

  ASMJIT_INST_2x(sha256rnds2, Sha256rnds2, Vec, Vec)                   // SHA [IMPLICIT]
  ASMJIT_INST_2x(sha256rnds2, Sha256rnds2, Vec, Mem)                   // SHA [IMPLICIT]

  //! \}

  //! \name AVX, FMA, and AVX512 Instructions
  //! \{

  //! \cond
  using EmitterExplicitT<This>::vmaskmovdqu;
  using EmitterExplicitT<This>::vpcmpestri;
  using EmitterExplicitT<This>::vpcmpestrm;
  using EmitterExplicitT<This>::vpcmpistri;
  using EmitterExplicitT<This>::vpcmpistrm;
  //! \endcond

  ASMJIT_INST_2x(vmaskmovdqu, Vmaskmovdqu, Vec, Vec)                   // AVX [IMPLICIT]
  ASMJIT_INST_3x(vpcmpestri, Vpcmpestri, Vec, Vec, Imm)                // AVX [IMPLICIT]
  ASMJIT_INST_3x(vpcmpestri, Vpcmpestri, Vec, Mem, Imm)                // AVX [IMPLICIT]
  ASMJIT_INST_3x(vpcmpestrm, Vpcmpestrm, Vec, Vec, Imm)                // AVX [IMPLICIT]
  ASMJIT_INST_3x(vpcmpestrm, Vpcmpestrm, Vec, Mem, Imm)                // AVX [IMPLICIT]
  ASMJIT_INST_3x(vpcmpistri, Vpcmpistri, Vec, Vec, Imm)                // AVX [IMPLICIT]
  ASMJIT_INST_3x(vpcmpistri, Vpcmpistri, Vec, Mem, Imm)                // AVX [IMPLICIT]
  ASMJIT_INST_3x(vpcmpistrm, Vpcmpistrm, Vec, Vec, Imm)                // AVX [IMPLICIT]
  ASMJIT_INST_3x(vpcmpistrm, Vpcmpistrm, Vec, Mem, Imm)                // AVX [IMPLICIT]

  //! \}
};

//! Emitter (X86|X86_64).
//!
//! \note This class cannot be instantiated, you can only cast to it and use it as emitter that emits to either
//! `x86::Assembler`, `x86::Builder`, or `x86::Compiler` (use with caution with `x86::Compiler` as it requires
//! virtual registers).
class Emitter : public BaseEmitter, public EmitterImplicitT<Emitter> {
  ASMJIT_NONCONSTRUCTIBLE(Emitter)
};

//! \}

#undef ASMJIT_INST_0x
#undef ASMJIT_INST_1x
#undef ASMJIT_INST_1c
#undef ASMJIT_INST_2x
#undef ASMJIT_INST_2c
#undef ASMJIT_INST_3x
#undef ASMJIT_INST_4x
#undef ASMJIT_INST_5x
#undef ASMJIT_INST_6x

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_X86_X86EMITTER_H_INCLUDED
