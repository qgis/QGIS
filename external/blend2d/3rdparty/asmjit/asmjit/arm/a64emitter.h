// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_ARM_A64EMITTER_H_INCLUDED
#define ASMJIT_ARM_A64EMITTER_H_INCLUDED

#include <asmjit/core/emitter.h>
#include <asmjit/support/support.h>
#include <asmjit/arm/a64instdb.h>
#include <asmjit/arm/a64operand.h>

// MSVC targeting AArch64 defines a lot of macros without underscores clashing
// with AArch64 instruction names. We have to workaround until it's fixed in SDK.
#if defined(_MSC_VER) && defined(mvn)
  #define ASMJIT_RESTORE_MSVC_AARCH64_MACROS
  #pragma push_macro("mvn")
  #undef mvn
#endif

ASMJIT_BEGIN_SUB_NAMESPACE(a64)

#define ASMJIT_INST_0x(NAME, ID) \
  inline Error NAME() { return _emitter()->_emitI(Inst::kId##ID); }

#define ASMJIT_INST_1x(NAME, ID, T0) \
  inline Error NAME(const T0& o0) { return _emitter()->_emitI(Inst::kId##ID, o0); }

#define ASMJIT_INST_2x(NAME, ID, T0, T1) \
  inline Error NAME(const T0& o0, const T1& o1) { return _emitter()->_emitI(Inst::kId##ID, o0, o1); }

#define ASMJIT_INST_3x(NAME, ID, T0, T1, T2) \
  inline Error NAME(const T0& o0, const T1& o1, const T2& o2) { return _emitter()->_emitI(Inst::kId##ID, o0, o1, o2); }

#define ASMJIT_INST_4x(NAME, ID, T0, T1, T2, T3) \
  inline Error NAME(const T0& o0, const T1& o1, const T2& o2, const T3& o3) { return _emitter()->_emitI(Inst::kId##ID, o0, o1, o2, o3); }

#define ASMJIT_INST_5x(NAME, ID, T0, T1, T2, T3, T4) \
  inline Error NAME(const T0& o0, const T1& o1, const T2& o2, const T3& o3, const T4& o4) { return _emitter()->_emitI(Inst::kId##ID, o0, o1, o2, o3, o4); }

#define ASMJIT_INST_6x(NAME, ID, T0, T1, T2, T3, T4, T5) \
  inline Error NAME(const T0& o0, const T1& o1, const T2& o2, const T3& o3, const T4& o4, const T5& o5) { return _emitter()->_emitI(Inst::kId##ID, o0, o1, o2, o3, o4, o5); }

#define ASMJIT_INST_1cc(NAME, ID, T0) \
  inline Error NAME(const T0& o0) { return _emitter()->_emitI(Inst::kId##ID, o0); } \
  \
  inline Error NAME(CondCode cc, const T0& o0) { return _emitter()->_emitI(BaseInst::compose_arm_inst_id(Inst::kId##ID, cc), o0); } \
  \
  inline Error NAME##_eq(const T0& o0) { return _emitter()->_emitI(BaseInst::compose_arm_inst_id(Inst::kId##ID, CondCode::kEQ), o0); } \
  inline Error NAME##_ne(const T0& o0) { return _emitter()->_emitI(BaseInst::compose_arm_inst_id(Inst::kId##ID, CondCode::kNE), o0); } \
  inline Error NAME##_cs(const T0& o0) { return _emitter()->_emitI(BaseInst::compose_arm_inst_id(Inst::kId##ID, CondCode::kCS), o0); } \
  inline Error NAME##_hs(const T0& o0) { return _emitter()->_emitI(BaseInst::compose_arm_inst_id(Inst::kId##ID, CondCode::kHS), o0); } \
  inline Error NAME##_cc(const T0& o0) { return _emitter()->_emitI(BaseInst::compose_arm_inst_id(Inst::kId##ID, CondCode::kCC), o0); } \
  inline Error NAME##_lo(const T0& o0) { return _emitter()->_emitI(BaseInst::compose_arm_inst_id(Inst::kId##ID, CondCode::kLO), o0); } \
  inline Error NAME##_mi(const T0& o0) { return _emitter()->_emitI(BaseInst::compose_arm_inst_id(Inst::kId##ID, CondCode::kMI), o0); } \
  inline Error NAME##_pl(const T0& o0) { return _emitter()->_emitI(BaseInst::compose_arm_inst_id(Inst::kId##ID, CondCode::kPL), o0); } \
  inline Error NAME##_vs(const T0& o0) { return _emitter()->_emitI(BaseInst::compose_arm_inst_id(Inst::kId##ID, CondCode::kVS), o0); } \
  inline Error NAME##_vc(const T0& o0) { return _emitter()->_emitI(BaseInst::compose_arm_inst_id(Inst::kId##ID, CondCode::kVC), o0); } \
  inline Error NAME##_hi(const T0& o0) { return _emitter()->_emitI(BaseInst::compose_arm_inst_id(Inst::kId##ID, CondCode::kHI), o0); } \
  inline Error NAME##_ls(const T0& o0) { return _emitter()->_emitI(BaseInst::compose_arm_inst_id(Inst::kId##ID, CondCode::kLS), o0); } \
  inline Error NAME##_ge(const T0& o0) { return _emitter()->_emitI(BaseInst::compose_arm_inst_id(Inst::kId##ID, CondCode::kGE), o0); } \
  inline Error NAME##_lt(const T0& o0) { return _emitter()->_emitI(BaseInst::compose_arm_inst_id(Inst::kId##ID, CondCode::kLT), o0); } \
  inline Error NAME##_gt(const T0& o0) { return _emitter()->_emitI(BaseInst::compose_arm_inst_id(Inst::kId##ID, CondCode::kGT), o0); } \
  inline Error NAME##_le(const T0& o0) { return _emitter()->_emitI(BaseInst::compose_arm_inst_id(Inst::kId##ID, CondCode::kLE), o0); } \
  inline Error NAME##_al(const T0& o0) { return _emitter()->_emitI(BaseInst::compose_arm_inst_id(Inst::kId##ID, CondCode::kAL), o0); }

//! \addtogroup asmjit_a64
//! \{

//! ARM emitter.
//!
//! NOTE: This class cannot be instantiated, you can only cast to it and use it as emitter that emits to either
//! \ref Assembler, \ref Builder, or \ref Compiler (use with caution with \ref Compiler as it expects virtual
//! registers to be used).
template<typename This>
struct EmitterExplicitT {
  //! \cond

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

  //! \}

  //! \name General Purpose Instructions
  //! \{

  ASMJIT_INST_3x(adc, Adc, Gp, Gp, Gp)
  ASMJIT_INST_3x(adcs, Adcs, Gp, Gp, Gp)

  ASMJIT_INST_3x(add, Add, Gp, Gp, Gp)
  ASMJIT_INST_4x(add, Add, Gp, Gp, Gp, Imm)
  ASMJIT_INST_3x(add, Add, Gp, Gp, Imm)
  ASMJIT_INST_4x(add, Add, Gp, Gp, Imm, Imm)
  ASMJIT_INST_3x(adds, Adds, Gp, Gp, Gp)
  ASMJIT_INST_3x(adds, Adds, Gp, Gp, Imm)
  ASMJIT_INST_4x(adds, Adds, Gp, Gp, Gp, Imm)
  ASMJIT_INST_4x(adds, Adds, Gp, Gp, Imm, Imm)

  ASMJIT_INST_2x(adr, Adr, Gp, Imm)
  ASMJIT_INST_2x(adr, Adr, Gp, Label)
  ASMJIT_INST_2x(adrp, Adrp, Gp, Imm)
  ASMJIT_INST_2x(adrp, Adrp, Gp, Label)

  ASMJIT_INST_3x(and_, And, Gp, Gp, Imm)
  ASMJIT_INST_3x(and_, And, Gp, Gp, Gp)
  ASMJIT_INST_4x(and_, And, Gp, Gp, Gp, Imm)
  ASMJIT_INST_3x(ands, Ands, Gp, Gp, Imm)
  ASMJIT_INST_3x(ands, Ands, Gp, Gp, Gp)
  ASMJIT_INST_4x(ands, Ands, Gp, Gp, Gp, Imm)

  ASMJIT_INST_3x(asr, Asr, Gp, Gp, Imm)
  ASMJIT_INST_3x(asr, Asr, Gp, Gp, Gp)
  ASMJIT_INST_3x(asrv, Asrv, Gp, Gp, Gp)

  ASMJIT_INST_2x(at, At, Imm, Gp)

  ASMJIT_INST_3x(bfc, Bfc, Gp, Imm, Imm)
  ASMJIT_INST_4x(bfi, Bfi, Gp, Gp, Imm, Imm)
  ASMJIT_INST_4x(bfm, Bfm, Gp, Gp, Imm, Imm)
  ASMJIT_INST_4x(bfxil, Bfxil, Gp, Gp, Imm, Imm)

  ASMJIT_INST_3x(bic, Bic, Gp, Gp, Imm)
  ASMJIT_INST_3x(bic, Bic, Gp, Gp, Gp)
  ASMJIT_INST_4x(bic, Bic, Gp, Gp, Gp, Imm)
  ASMJIT_INST_3x(bics, Bics, Gp, Gp, Imm)
  ASMJIT_INST_3x(bics, Bics, Gp, Gp, Gp)
  ASMJIT_INST_4x(bics, Bics, Gp, Gp, Gp, Imm)

  ASMJIT_INST_1x(brk, Brk, Imm)

  ASMJIT_INST_4x(ccmn, Ccmn, Gp, Gp, Imm, Imm)
  ASMJIT_INST_4x(ccmn, Ccmn, Gp, Imm, Imm, Imm)
  ASMJIT_INST_4x(ccmp, Ccmp, Gp, Gp, Imm, Imm)
  ASMJIT_INST_4x(ccmp, Ccmp, Gp, Imm, Imm, Imm)

  ASMJIT_INST_3x(cinc, Cinc, Gp, Gp, Imm)
  ASMJIT_INST_3x(cinv, Cinv, Gp, Gp, Imm)

  ASMJIT_INST_1x(clrex, Clrex, Imm)

  ASMJIT_INST_2x(cls, Cls, Gp, Gp)
  ASMJIT_INST_2x(clz, Clz, Gp, Gp)

  ASMJIT_INST_2x(cmn, Cmn, Gp, Gp)
  ASMJIT_INST_3x(cmn, Cmn, Gp, Gp, Imm)
  ASMJIT_INST_2x(cmn, Cmn, Gp, Imm)
  ASMJIT_INST_3x(cmn, Cmn, Gp, Imm, Imm)
  ASMJIT_INST_2x(cmp, Cmp, Gp, Gp)
  ASMJIT_INST_3x(cmp, Cmp, Gp, Gp, Imm)
  ASMJIT_INST_2x(cmp, Cmp, Gp, Imm)
  ASMJIT_INST_3x(cmp, Cmp, Gp, Imm, Imm)

  ASMJIT_INST_3x(cneg, Cneg, Gp, Gp, Imm)

  ASMJIT_INST_4x(csel, Csel, Gp, Gp, Gp, Imm)
  ASMJIT_INST_2x(cset, Cset, Gp, Imm)
  ASMJIT_INST_2x(csetm, Csetm, Gp, Imm)

  ASMJIT_INST_4x(csinc, Csinc, Gp, Gp, Gp, Imm)
  ASMJIT_INST_4x(csinv, Csinv, Gp, Gp, Gp, Imm)
  ASMJIT_INST_4x(csneg, Csneg, Gp, Gp, Gp, Imm)

  ASMJIT_INST_2x(dc, Dc, Imm, Gp)
  ASMJIT_INST_1x(dmb, Dmb, Imm)
  ASMJIT_INST_1x(dsb, Dsb, Imm)
  ASMJIT_INST_0x(drps, Drps)

  ASMJIT_INST_3x(eon, Eon, Gp, Gp, Gp)
  ASMJIT_INST_4x(eon, Eon, Gp, Gp, Gp, Imm)

  ASMJIT_INST_3x(eor, Eor, Gp, Gp, Imm)
  ASMJIT_INST_3x(eor, Eor, Gp, Gp, Gp)
  ASMJIT_INST_4x(eor, Eor, Gp, Gp, Gp, Imm)

  ASMJIT_INST_0x(eret, Eret)
  ASMJIT_INST_0x(esb, Esb)

  ASMJIT_INST_4x(extr, Extr, Gp, Gp, Gp, Imm)

  ASMJIT_INST_1x(hlt, Hlt, Imm)
  ASMJIT_INST_1x(hvc, Hvc, Imm)
  ASMJIT_INST_2x(ic, Ic, Imm, Gp)
  ASMJIT_INST_1x(isb, Isb, Imm)

  ASMJIT_INST_3x(lsl, Lsl, Gp, Gp, Imm)
  ASMJIT_INST_3x(lsl, Lsl, Gp, Gp, Gp)
  ASMJIT_INST_3x(lslv, Lslv, Gp, Gp, Gp)

  ASMJIT_INST_3x(lsr, Lsr, Gp, Gp, Imm)
  ASMJIT_INST_3x(lsr, Lsr, Gp, Gp, Gp)
  ASMJIT_INST_3x(lsrv, Lsrv, Gp, Gp, Gp)

  ASMJIT_INST_4x(madd, Madd, Gp, Gp, Gp, Gp)
  ASMJIT_INST_3x(mneg, Mneg, Gp, Gp, Gp)

  ASMJIT_INST_2x(mov, Mov, Gp, Gp)
  ASMJIT_INST_2x(mov, Mov, Gp, Imm)
  ASMJIT_INST_2x(movk, Movk, Gp, Imm)
  ASMJIT_INST_3x(movk, Movk, Gp, Imm, Imm)
  ASMJIT_INST_2x(movn, Movn, Gp, Imm)
  ASMJIT_INST_3x(movn, Movn, Gp, Imm, Imm)
  ASMJIT_INST_2x(movz, Movz, Gp, Imm)
  ASMJIT_INST_3x(movz, Movz, Gp, Imm, Imm)

  ASMJIT_INST_2x(mrs, Mrs, Gp, Imm)
  ASMJIT_INST_2x(msr, Msr, Imm, Gp)
  ASMJIT_INST_2x(msr, Msr, Imm, Imm)

  ASMJIT_INST_4x(msub, Msub, Gp, Gp, Gp, Gp)
  ASMJIT_INST_3x(mul, Mul, Gp, Gp, Gp)

  ASMJIT_INST_2x(mvn, Mvn, Gp, Gp)
  ASMJIT_INST_3x(mvn, Mvn, Gp, Gp, Imm)

  ASMJIT_INST_2x(mvn_, Mvn, Gp, Gp)
  ASMJIT_INST_3x(mvn_, Mvn, Gp, Gp, Imm)

  ASMJIT_INST_2x(neg, Neg, Gp, Gp)
  ASMJIT_INST_3x(neg, Neg, Gp, Gp, Imm)
  ASMJIT_INST_2x(negs, Negs, Gp, Gp)
  ASMJIT_INST_3x(negs, Negs, Gp, Gp, Imm)

  ASMJIT_INST_2x(ngc, Ngc, Gp, Gp)
  ASMJIT_INST_2x(ngcs, Ngcs, Gp, Gp)

  ASMJIT_INST_3x(orn, Orn, Gp, Gp, Gp)
  ASMJIT_INST_4x(orn, Orn, Gp, Gp, Gp, Imm)

  ASMJIT_INST_3x(orr, Orr, Gp, Gp, Imm)
  ASMJIT_INST_3x(orr, Orr, Gp, Gp, Gp)
  ASMJIT_INST_4x(orr, Orr, Gp, Gp, Gp, Imm)

  ASMJIT_INST_2x(rbit, Rbit, Gp, Gp)
  ASMJIT_INST_1x(ret, Ret, Gp)

  ASMJIT_INST_2x(rev, Rev, Gp, Gp)
  ASMJIT_INST_2x(rev16, Rev16, Gp, Gp)
  ASMJIT_INST_2x(rev32, Rev32, Gp, Gp)
  ASMJIT_INST_2x(rev64, Rev64, Gp, Gp)

  ASMJIT_INST_3x(ror, Ror, Gp, Gp, Imm)
  ASMJIT_INST_3x(ror, Ror, Gp, Gp, Gp)
  ASMJIT_INST_3x(rorv, Rorv, Gp, Gp, Gp)

  ASMJIT_INST_3x(sbc, Sbc, Gp, Gp, Gp)
  ASMJIT_INST_3x(sbcs, Sbcs, Gp, Gp, Gp)

  ASMJIT_INST_4x(sbfiz, Sbfiz, Gp, Gp, Imm, Imm)
  ASMJIT_INST_4x(sbfm, Sbfm, Gp, Gp, Imm, Imm)
  ASMJIT_INST_4x(sbfx, Sbfx, Gp, Gp, Imm, Imm)

  ASMJIT_INST_3x(sdiv, Sdiv, Gp, Gp, Gp)

  ASMJIT_INST_4x(smaddl, Smaddl, Gp, Gp, Gp, Gp)
  ASMJIT_INST_1x(smc, Smc, Imm)
  ASMJIT_INST_3x(smnegl, Smnegl, Gp, Gp, Gp)
  ASMJIT_INST_4x(smsubl, Smsubl, Gp, Gp, Gp, Gp)
  ASMJIT_INST_3x(smulh, Smulh, Gp, Gp, Gp)
  ASMJIT_INST_3x(smull, Smull, Gp, Gp, Gp)

  ASMJIT_INST_3x(sub, Sub, Gp, Gp, Gp)
  ASMJIT_INST_4x(sub, Sub, Gp, Gp, Gp, Imm)
  ASMJIT_INST_3x(sub, Sub, Gp, Gp, Imm)
  ASMJIT_INST_4x(sub, Sub, Gp, Gp, Imm, Imm)
  ASMJIT_INST_3x(subs, Subs, Gp, Gp, Gp)
  ASMJIT_INST_4x(subs, Subs, Gp, Gp, Gp, Imm)
  ASMJIT_INST_3x(subs, Subs, Gp, Gp, Imm)
  ASMJIT_INST_4x(subs, Subs, Gp, Gp, Imm, Imm)

  ASMJIT_INST_1x(svc, Svc, Imm)

  ASMJIT_INST_2x(sxtb, Sxtb, Gp, Gp)
  ASMJIT_INST_2x(sxth, Sxth, Gp, Gp)
  ASMJIT_INST_2x(sxtw, Sxtw, Gp, Gp)

  ASMJIT_INST_4x(sys, Sys, Imm, Imm, Imm, Imm)
  ASMJIT_INST_5x(sys, Sys, Imm, Imm, Imm, Imm, Gp)

  ASMJIT_INST_2x(tlbi, Tlbi, Imm, Gp)
  ASMJIT_INST_2x(tst, Tst, Gp, Imm)
  ASMJIT_INST_2x(tst, Tst, Gp, Gp)
  ASMJIT_INST_3x(tst, Tst, Gp, Gp, Imm)

  ASMJIT_INST_3x(udiv, Udiv, Gp, Gp, Gp)

  ASMJIT_INST_4x(ubfiz, Ubfiz, Gp, Gp, Imm, Imm)
  ASMJIT_INST_4x(ubfm, Ubfm, Gp, Gp, Imm, Imm)
  ASMJIT_INST_4x(ubfx, Ubfx, Gp, Gp, Imm, Imm)

  ASMJIT_INST_4x(umaddl, Umaddl, Gp, Gp, Gp, Gp)
  ASMJIT_INST_3x(umnegl, Umnegl, Gp, Gp, Gp)
  ASMJIT_INST_4x(umsubl, Umsubl, Gp, Gp, Gp, Gp)
  ASMJIT_INST_3x(umull, Umull, Gp, Gp, Gp)
  ASMJIT_INST_3x(umulh, Umulh, Gp, Gp, Gp)

  ASMJIT_INST_2x(uxtb, Uxtb, Gp, Gp)
  ASMJIT_INST_2x(uxth, Uxth, Gp, Gp)

  ASMJIT_INST_0x(csdb, Csdb)
  ASMJIT_INST_1x(dcps1, Dcps1, Imm)
  ASMJIT_INST_1x(dcps2, Dcps2, Imm)
  ASMJIT_INST_1x(dcps3, Dcps3, Imm)
  ASMJIT_INST_0x(pssbb, Pssbb)
  ASMJIT_INST_0x(ssbb, Ssbb)
  ASMJIT_INST_1x(udf, Udf, Imm)

  //! \}

  //! \name Branch Instructions
  //! \{

  ASMJIT_INST_1cc(b, B, Imm)
  ASMJIT_INST_1cc(b, B, Label)
  ASMJIT_INST_1x(bl, Bl, Imm)
  ASMJIT_INST_1x(bl, Bl, Label)
  ASMJIT_INST_1x(blr, Blr, Gp)
  ASMJIT_INST_1x(br, Br, Gp)
  ASMJIT_INST_2x(cbz, Cbz, Gp, Imm)
  ASMJIT_INST_2x(cbz, Cbz, Gp, Label)
  ASMJIT_INST_2x(cbnz, Cbnz, Gp, Imm)
  ASMJIT_INST_2x(cbnz, Cbnz, Gp, Label)
  ASMJIT_INST_3x(tbnz, Tbnz, Gp, Imm, Imm)
  ASMJIT_INST_3x(tbnz, Tbnz, Gp, Imm, Label)
  ASMJIT_INST_3x(tbz, Tbz, Gp, Imm, Imm)
  ASMJIT_INST_3x(tbz, Tbz, Gp, Imm, Label)

  //! \}

  //! \name Load & Store Instructions
  //! \{

  ASMJIT_INST_3x(cas, Cas, Gp, Gp, Mem)
  ASMJIT_INST_3x(casa, Casa, Gp, Gp, Mem)
  ASMJIT_INST_3x(casab, Casab, Gp, Gp, Mem)
  ASMJIT_INST_3x(casah, Casah, Gp, Gp, Mem)
  ASMJIT_INST_3x(casal, Casal, Gp, Gp, Mem)
  ASMJIT_INST_3x(casalb, Casalb, Gp, Gp, Mem)
  ASMJIT_INST_3x(casalh, Casalh, Gp, Gp, Mem)
  ASMJIT_INST_3x(casb, Casb, Gp, Gp, Mem)
  ASMJIT_INST_3x(cash, Cash, Gp, Gp, Mem)
  ASMJIT_INST_3x(casl, Casl, Gp, Gp, Mem)
  ASMJIT_INST_3x(caslb, Caslb, Gp, Gp, Mem)
  ASMJIT_INST_3x(caslh, Caslh, Gp, Gp, Mem)

  ASMJIT_INST_5x(casp, Casp, Gp, Gp, Gp, Gp, Mem)
  ASMJIT_INST_5x(caspa, Caspa, Gp, Gp, Gp, Gp, Mem)
  ASMJIT_INST_5x(caspal, Caspal, Gp, Gp, Gp, Gp, Mem)
  ASMJIT_INST_5x(caspl, Caspl, Gp, Gp, Gp, Gp, Mem)

  ASMJIT_INST_3x(ldadd, Ldadd, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldadda, Ldadda, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldaddab, Ldaddab, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldaddah, Ldaddah, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldaddal, Ldaddal, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldaddalb, Ldaddalb, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldaddalh, Ldaddalh, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldaddb, Ldaddb, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldaddh, Ldaddh, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldaddl, Ldaddl, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldaddlb, Ldaddlb, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldaddlh, Ldaddlh, Gp, Gp, Mem)

  ASMJIT_INST_2x(ldar, Ldar, Gp, Mem)
  ASMJIT_INST_2x(ldarb, Ldarb, Gp, Mem)
  ASMJIT_INST_2x(ldarh, Ldarh, Gp, Mem)

  ASMJIT_INST_2x(ldaxr, Ldaxr, Gp, Mem)
  ASMJIT_INST_2x(ldaxrb, Ldaxrb, Gp, Mem)
  ASMJIT_INST_2x(ldaxrh, Ldaxrh, Gp, Mem)

  ASMJIT_INST_3x(ldclr, Ldclr, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldclra, Ldclra, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldclrab, Ldclrab, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldclrah, Ldclrah, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldclral, Ldclral, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldclralb, Ldclralb, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldclralh, Ldclralh, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldclrb, Ldclrb, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldclrh, Ldclrh, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldclrl, Ldclrl, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldclrlb, Ldclrlb, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldclrlh, Ldclrlh, Gp, Gp, Mem)

  ASMJIT_INST_3x(ldeor, Ldeor, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldeora, Ldeora, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldeorab, Ldeorab, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldeorah, Ldeorah, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldeoral, Ldeoral, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldeoralb, Ldeoralb, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldeoralh, Ldeoralh, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldeorb, Ldeorb, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldeorh, Ldeorh, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldeorl, Ldeorl, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldeorlb, Ldeorlb, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldeorlh, Ldeorlh, Gp, Gp, Mem)

  ASMJIT_INST_2x(ldlar, Ldlar, Gp, Mem)
  ASMJIT_INST_2x(ldlarb, Ldlarb, Gp, Mem)
  ASMJIT_INST_2x(ldlarh, Ldlarh, Gp, Mem)

  ASMJIT_INST_3x(ldnp, Ldnp, Gp, Gp, Mem)

  ASMJIT_INST_3x(ldp, Ldp, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldpsw, Ldpsw, Gp, Gp, Mem)

  ASMJIT_INST_2x(ldr, Ldr, Gp, Mem)
  ASMJIT_INST_2x(ldrb, Ldrb, Gp, Mem)
  ASMJIT_INST_2x(ldrh, Ldrh, Gp, Mem)
  ASMJIT_INST_2x(ldrsb, Ldrsb, Gp, Mem)
  ASMJIT_INST_2x(ldrsh, Ldrsh, Gp, Mem)
  ASMJIT_INST_2x(ldrsw, Ldrsw, Gp, Mem)

  ASMJIT_INST_3x(ldset, Ldset, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldseta, Ldseta, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldsetab, Ldsetab, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldsetah, Ldsetah, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldsetal, Ldsetal, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldsetalb, Ldsetalb, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldsetalh, Ldsetalh, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldsetb, Ldsetb, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldseth, Ldseth, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldsetl, Ldsetl, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldsetlb, Ldsetlb, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldsetlh, Ldsetlh, Gp, Gp, Mem)

  ASMJIT_INST_3x(ldsmax, Ldsmax, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldsmaxa, Ldsmaxa, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldsmaxab, Ldsmaxab, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldsmaxah, Ldsmaxah, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldsmaxal, Ldsmaxal, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldsmaxalb, Ldsmaxalb, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldsmaxalh, Ldsmaxalh, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldsmaxb, Ldsmaxb, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldsmaxh, Ldsmaxh, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldsmaxl, Ldsmaxl, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldsmaxlb, Ldsmaxlb, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldsmaxlh, Ldsmaxlh, Gp, Gp, Mem)

  ASMJIT_INST_3x(ldsmin, Ldsmin, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldsmina, Ldsmina, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldsminab, Ldsminab, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldsminah, Ldsminah, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldsminal, Ldsminal, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldsminalb, Ldsminalb, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldsminalh, Ldsminalh, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldsminb, Ldsminb, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldsminh, Ldsminh, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldsminl, Ldsminl, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldsminlb, Ldsminlb, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldsminlh, Ldsminlh, Gp, Gp, Mem)

  ASMJIT_INST_2x(ldtr, Ldtr, Gp, Mem)
  ASMJIT_INST_2x(ldtrb, Ldtrb, Gp, Mem)
  ASMJIT_INST_2x(ldtrh, Ldtrh, Gp, Mem)
  ASMJIT_INST_2x(ldtrsb, Ldtrsb, Gp, Mem)
  ASMJIT_INST_2x(ldtrsh, Ldtrsh, Gp, Mem)
  ASMJIT_INST_2x(ldtrsw, Ldtrsw, Gp, Mem)

  ASMJIT_INST_3x(ldumax, Ldumax, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldumaxa, Ldumaxa, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldumaxab, Ldumaxab, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldumaxah, Ldumaxah, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldumaxal, Ldumaxal, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldumaxalb, Ldumaxalb, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldumaxalh, Ldumaxalh, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldumaxb, Ldumaxb, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldumaxh, Ldumaxh, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldumaxl, Ldumaxl, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldumaxlb, Ldumaxlb, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldumaxlh, Ldumaxlh, Gp, Gp, Mem)

  ASMJIT_INST_3x(ldumin, Ldumin, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldumina, Ldumina, Gp, Gp, Mem)
  ASMJIT_INST_3x(lduminab, Lduminab, Gp, Gp, Mem)
  ASMJIT_INST_3x(lduminah, Lduminah, Gp, Gp, Mem)
  ASMJIT_INST_3x(lduminal, Lduminal, Gp, Gp, Mem)
  ASMJIT_INST_3x(lduminalb, Lduminalb, Gp, Gp, Mem)
  ASMJIT_INST_3x(lduminalh, Lduminalh, Gp, Gp, Mem)
  ASMJIT_INST_3x(lduminb, Lduminb, Gp, Gp, Mem)
  ASMJIT_INST_3x(lduminh, Lduminh, Gp, Gp, Mem)
  ASMJIT_INST_3x(lduminl, Lduminl, Gp, Gp, Mem)
  ASMJIT_INST_3x(lduminlb, Lduminlb, Gp, Gp, Mem)
  ASMJIT_INST_3x(lduminlh, Lduminlh, Gp, Gp, Mem)

  ASMJIT_INST_2x(ldur, Ldur, Gp, Mem)
  ASMJIT_INST_2x(ldurb, Ldurb, Gp, Mem)
  ASMJIT_INST_2x(ldurh, Ldurh, Gp, Mem)
  ASMJIT_INST_2x(ldursb, Ldursb, Gp, Mem)
  ASMJIT_INST_2x(ldursh, Ldursh, Gp, Mem)
  ASMJIT_INST_2x(ldursw, Ldursw, Gp, Mem)

  ASMJIT_INST_3x(ldxp, Ldxp, Gp, Gp, Mem)
  ASMJIT_INST_3x(ldaxp, Ldaxp, Gp, Gp, Mem)

  ASMJIT_INST_2x(ldxr, Ldxr, Gp, Mem)
  ASMJIT_INST_2x(ldxrb, Ldxrb, Gp, Mem)
  ASMJIT_INST_2x(ldxrh, Ldxrh, Gp, Mem)

  ASMJIT_INST_2x(prfm, Prfm, Imm, Mem)

  ASMJIT_INST_2x(stadd, Stadd, Gp, Mem)
  ASMJIT_INST_2x(staddb, Staddb, Gp, Mem)
  ASMJIT_INST_2x(staddh, Staddh, Gp, Mem)
  ASMJIT_INST_2x(staddl, Staddl, Gp, Mem)
  ASMJIT_INST_2x(staddlb, Staddlb, Gp, Mem)
  ASMJIT_INST_2x(staddlh, Staddlh, Gp, Mem)

  ASMJIT_INST_2x(stclr, Stclr, Gp, Mem)
  ASMJIT_INST_2x(stclrb, Stclrb, Gp, Mem)
  ASMJIT_INST_2x(stclrh, Stclrh, Gp, Mem)
  ASMJIT_INST_2x(stclrl, Stclrl, Gp, Mem)
  ASMJIT_INST_2x(stclrlb, Stclrlb, Gp, Mem)
  ASMJIT_INST_2x(stclrlh, Stclrlh, Gp, Mem)

  ASMJIT_INST_2x(steor, Steor, Gp, Mem)
  ASMJIT_INST_2x(steorb, Steorb, Gp, Mem)
  ASMJIT_INST_2x(steorh, Steorh, Gp, Mem)
  ASMJIT_INST_2x(steorl, Steorl, Gp, Mem)
  ASMJIT_INST_2x(steorlb, Steorlb, Gp, Mem)
  ASMJIT_INST_2x(steorlh, Steorlh, Gp, Mem)

  ASMJIT_INST_2x(stllr, Stllr, Gp, Mem)
  ASMJIT_INST_2x(stllrb, Stllrb, Gp, Mem)
  ASMJIT_INST_2x(stllrh, Stllrh, Gp, Mem)

  ASMJIT_INST_2x(stlr, Stllr, Gp, Mem)
  ASMJIT_INST_2x(stlrb, Stllrb, Gp, Mem)
  ASMJIT_INST_2x(stlrh, Stllrh, Gp, Mem)

  ASMJIT_INST_3x(stlxr, Stlxr, Gp, Gp, Mem)
  ASMJIT_INST_3x(stlxrb, Stlxrb, Gp, Gp, Mem)
  ASMJIT_INST_3x(stlxrh, Stlxrh, Gp, Gp, Mem)

  ASMJIT_INST_3x(stnp, Stnp, Gp, Gp, Mem)
  ASMJIT_INST_3x(stp, Stp, Gp, Gp, Mem)

  ASMJIT_INST_2x(str, Str, Gp, Mem)
  ASMJIT_INST_2x(strb, Strb, Gp, Mem)
  ASMJIT_INST_2x(strh, Strh, Gp, Mem)

  ASMJIT_INST_2x(stset, Stset, Gp, Mem)
  ASMJIT_INST_2x(stsetb, Stsetb, Gp, Mem)
  ASMJIT_INST_2x(stseth, Stseth, Gp, Mem)
  ASMJIT_INST_2x(stsetl, Stsetl, Gp, Mem)
  ASMJIT_INST_2x(stsetlb, Stsetlb, Gp, Mem)
  ASMJIT_INST_2x(stsetlh, Stsetlh, Gp, Mem)

  ASMJIT_INST_2x(stsmax, Stsmax, Gp, Mem)
  ASMJIT_INST_2x(stsmaxb, Stsmaxb, Gp, Mem)
  ASMJIT_INST_2x(stsmaxh, Stsmaxh, Gp, Mem)
  ASMJIT_INST_2x(stsmaxl, Stsmaxl, Gp, Mem)
  ASMJIT_INST_2x(stsmaxlb, Stsmaxlb, Gp, Mem)
  ASMJIT_INST_2x(stsmaxlh, Stsmaxlh, Gp, Mem)

  ASMJIT_INST_2x(stsmin, Stsmin, Gp, Mem)
  ASMJIT_INST_2x(stsminb, Stsminb, Gp, Mem)
  ASMJIT_INST_2x(stsminh, Stsminh, Gp, Mem)
  ASMJIT_INST_2x(stsminl, Stsminl, Gp, Mem)
  ASMJIT_INST_2x(stsminlb, Stsminlb, Gp, Mem)
  ASMJIT_INST_2x(stsminlh, Stsminlh, Gp, Mem)

  ASMJIT_INST_2x(sttr, Sttr, Gp, Mem)
  ASMJIT_INST_2x(sttrb, Sttrb, Gp, Mem)
  ASMJIT_INST_2x(sttrh, Sttrh, Gp, Mem)

  ASMJIT_INST_2x(stumax, Stumax, Gp, Mem)
  ASMJIT_INST_2x(stumaxb, Stumaxb, Gp, Mem)
  ASMJIT_INST_2x(stumaxh, Stumaxh, Gp, Mem)
  ASMJIT_INST_2x(stumaxl, Stumaxl, Gp, Mem)
  ASMJIT_INST_2x(stumaxlb, Stumaxlb, Gp, Mem)
  ASMJIT_INST_2x(stumaxlh, Stumaxlh, Gp, Mem)

  ASMJIT_INST_2x(stumin, Stumin, Gp, Mem)
  ASMJIT_INST_2x(stuminb, Stuminb, Gp, Mem)
  ASMJIT_INST_2x(stuminh, Stuminh, Gp, Mem)
  ASMJIT_INST_2x(stuminl, Stuminl, Gp, Mem)
  ASMJIT_INST_2x(stuminlb, Stuminlb, Gp, Mem)
  ASMJIT_INST_2x(stuminlh, Stuminlh, Gp, Mem)

  ASMJIT_INST_2x(stur, Stur, Gp, Mem)
  ASMJIT_INST_2x(sturb, Sturb, Gp, Mem)
  ASMJIT_INST_2x(sturh, Sturh, Gp, Mem)

  ASMJIT_INST_4x(stxp, Stxp, Gp, Gp, Gp, Mem)
  ASMJIT_INST_4x(stlxp, Stlxp, Gp, Gp, Gp, Mem)

  ASMJIT_INST_3x(stxr, Stxr, Gp, Gp, Mem)
  ASMJIT_INST_3x(stxrb, Stxrb, Gp, Gp, Mem)
  ASMJIT_INST_3x(stxrh, Stxrh, Gp, Gp, Mem)

  ASMJIT_INST_3x(swp, Swp, Gp, Gp, Mem)
  ASMJIT_INST_3x(swpa, Swpa, Gp, Gp, Mem)
  ASMJIT_INST_3x(swpab, Swpab, Gp, Gp, Mem)
  ASMJIT_INST_3x(swpah, Swpah, Gp, Gp, Mem)
  ASMJIT_INST_3x(swpal, Swpal, Gp, Gp, Mem)
  ASMJIT_INST_3x(swpalb, Swpalb, Gp, Gp, Mem)
  ASMJIT_INST_3x(swpalh, Swpalh, Gp, Gp, Mem)
  ASMJIT_INST_3x(swpb, Swpb, Gp, Gp, Mem)
  ASMJIT_INST_3x(swph, Swph, Gp, Gp, Mem)
  ASMJIT_INST_3x(swpl, Swpl, Gp, Gp, Mem)
  ASMJIT_INST_3x(swplb, Swplb, Gp, Gp, Mem)
  ASMJIT_INST_3x(swplh, Swplh, Gp, Gp, Mem)
  //! \}

  //! \name BTI Instructions
  //! \{

  ASMJIT_INST_1x(bti, Bti, Imm)

  //! \}

  //! \name CHK Instructions
  //! \{

  ASMJIT_INST_1x(chkfeat, Chkfeat, Gp)

  //! \}

  //! \name CLRBHB Instructions
  //! \{

  ASMJIT_INST_0x(clrbhb, Clrbhb)

  //! \}

  //! \name CRC Instructions (ARMv8.1-A, optional in ARMv8.0-A)
  //! \{

  ASMJIT_INST_3x(crc32b, Crc32b, Gp, Gp, Gp)
  ASMJIT_INST_3x(crc32h, Crc32h, Gp, Gp, Gp)
  ASMJIT_INST_3x(crc32w, Crc32w, Gp, Gp, Gp)
  ASMJIT_INST_3x(crc32x, Crc32x, Gp, Gp, Gp)

  ASMJIT_INST_3x(crc32cb, Crc32cb, Gp, Gp, Gp)
  ASMJIT_INST_3x(crc32ch, Crc32ch, Gp, Gp, Gp)
  ASMJIT_INST_3x(crc32cw, Crc32cw, Gp, Gp, Gp)
  ASMJIT_INST_3x(crc32cx, Crc32cx, Gp, Gp, Gp)

  //! \}

  //! \name CSSC Instructions
  //! \{

  ASMJIT_INST_2x(abs, Abs, Gp, Gp)
  ASMJIT_INST_2x(cnt, Cnt, Gp, Gp)
  ASMJIT_INST_2x(ctz, Ctz, Gp, Gp)
  ASMJIT_INST_3x(smax, Smax, Gp, Gp, Gp)
  ASMJIT_INST_3x(smax, Smax, Gp, Gp, Imm)
  ASMJIT_INST_3x(smin, Smin, Gp, Gp, Gp)
  ASMJIT_INST_3x(smin, Smin, Gp, Gp, Imm)
  ASMJIT_INST_3x(umax, Umax, Gp, Gp, Gp)
  ASMJIT_INST_3x(umax, Umax, Gp, Gp, Imm)
  ASMJIT_INST_3x(umin, Umin, Gp, Gp, Gp)
  ASMJIT_INST_3x(umin, Umin, Gp, Gp, Imm)

  //! \}

  //! \name DGH Instructions
  //! \{

  ASMJIT_INST_0x(dgh, Dgh)

  //! \}

  //! \name FLAGM Instructions
  //! \{

  ASMJIT_INST_0x(cfinv, Cfinv)
  ASMJIT_INST_1x(setf8, Setf8, Gp)
  ASMJIT_INST_1x(setf16, Setf16, Gp)

  //! \}

  //! \name FLAGM2 Instructions
  //! \{

  ASMJIT_INST_0x(axflag, Axflag)
  ASMJIT_INST_0x(xaflag, Xaflag)

  //! \}

  //! \name HBC Instructions
  //! \{

  ASMJIT_INST_1cc(bc, Bc, Imm)
  ASMJIT_INST_1cc(bc, Bc, Label)

  //! \}

  //! \name MTE Instructions
  //! \{

  ASMJIT_INST_2x(autda, Autda, Gp, Gp)
  ASMJIT_INST_2x(autdb, Autdb, Gp, Gp)
  ASMJIT_INST_1x(autdza, Autdza, Gp)
  ASMJIT_INST_1x(autdzb, Autdzb, Gp)
  ASMJIT_INST_2x(autia, Autia, Gp, Gp)
  ASMJIT_INST_0x(autia1716, Autia1716)
  ASMJIT_INST_0x(autiasp, Autiasp)
  ASMJIT_INST_0x(autiaz, Autiaz)
  ASMJIT_INST_2x(autib, Autib, Gp, Gp)
  ASMJIT_INST_0x(autib1716, Autib1716)
  ASMJIT_INST_0x(autibsp, Autibsp)
  ASMJIT_INST_0x(autibz, Autibz)
  ASMJIT_INST_1x(autiza, Autiza, Gp)
  ASMJIT_INST_1x(autizb, Autizb, Gp)

  ASMJIT_INST_3x(gmi, Gmi, Gp, Gp, Gp)

  ASMJIT_INST_2x(cmpp, Cmpp, Gp, Gp)
  ASMJIT_INST_4x(addg, Addg, Gp, Gp, Imm, Imm)

  ASMJIT_INST_2x(ldg, Ldg, Gp, Mem)
  ASMJIT_INST_2x(ldgm, Ldgm, Gp, Mem)
  ASMJIT_INST_2x(ldraa, Ldraa, Gp, Mem)
  ASMJIT_INST_2x(ldrab, Ldrab, Gp, Mem)

  ASMJIT_INST_2x(pacda, Pacda, Gp, Gp)
  ASMJIT_INST_2x(pacdb, Pacdb, Gp, Gp)
  ASMJIT_INST_1x(pacdza, Pacdza, Gp)
  ASMJIT_INST_1x(pacdzb, Pacdzb, Gp)
  ASMJIT_INST_3x(pacga, Pacga, Gp, Gp, Gp)

  ASMJIT_INST_3x(subp, Subp, Gp, Gp, Gp)
  ASMJIT_INST_3x(subps, Subps, Gp, Gp, Gp)
  ASMJIT_INST_4x(subg, Subg, Gp, Gp, Imm, Imm)

  ASMJIT_INST_2x(st2g, St2g, Gp, Mem)
  ASMJIT_INST_2x(stg, Stg, Gp, Mem)
  ASMJIT_INST_3x(stgp, Stgp, Gp, Gp, Mem)
  ASMJIT_INST_2x(stgm, Stgm, Gp, Mem)
  ASMJIT_INST_2x(stzg, Stzg, Gp, Mem)
  ASMJIT_INST_2x(stz2g, Stz2g, Gp, Mem)
  ASMJIT_INST_2x(stzgm, Stzgm, Gp, Mem)

  ASMJIT_INST_1x(xpacd, Xpacd, Gp)
  ASMJIT_INST_1x(xpaci, Xpaci, Gp)
  ASMJIT_INST_0x(xpaclri, Xpaclri)

  //! \}

  //! \name Hint Instructions
  //! \{

  ASMJIT_INST_1x(hint, Hint, Imm)
  ASMJIT_INST_0x(nop, Nop)
  ASMJIT_INST_0x(sev, Sev)
  ASMJIT_INST_0x(sevl, Sevl)
  ASMJIT_INST_0x(wfe, Wfe)
  ASMJIT_INST_0x(wfi, Wfi)
  ASMJIT_INST_0x(yield, Yield)

  //! \}

  //! \name SIMD & FP Instructions
  //! \{

  ASMJIT_INST_2x(abs, Abs_v, Vec, Vec)
  ASMJIT_INST_3x(add, Add_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(addhn, Addhn_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(addhn2, Addhn2_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(addp, Addp_v, Vec, Vec)
  ASMJIT_INST_3x(addp, Addp_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(addv, Addv_v, Vec, Vec)
  ASMJIT_INST_3x(and_, And_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(bic, Bic_v, Vec, Imm)
  ASMJIT_INST_3x(bic, Bic_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(bic, Bic_v, Vec, Imm, Imm)
  ASMJIT_INST_3x(bif, Bif_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(bit, Bit_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(bsl, Bsl_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(cls, Cls_v, Vec, Vec)
  ASMJIT_INST_2x(clz, Clz_v, Vec, Vec)
  ASMJIT_INST_3x(cmeq, Cmeq_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(cmeq, Cmeq_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(cmge, Cmge_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(cmge, Cmge_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(cmgt, Cmgt_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(cmgt, Cmgt_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(cmhi, Cmhi_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(cmhs, Cmhs_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(cmle, Cmle_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(cmlt, Cmlt_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(cmtst, Cmtst_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(cnt, Cnt_v, Vec, Vec)
  ASMJIT_INST_2x(dup, Dup_v, Vec, Gp)
  ASMJIT_INST_2x(dup, Dup_v, Vec, Vec)
  ASMJIT_INST_3x(eor, Eor_v, Vec, Vec, Vec)
  ASMJIT_INST_4x(ext, Ext_v, Vec, Vec, Vec, Imm)
  ASMJIT_INST_3x(fabd, Fabd_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(fabs, Fabs_v, Vec, Vec)
  ASMJIT_INST_3x(facge, Facge_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(facgt, Facgt_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(fadd, Fadd_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(faddp, Faddp_v, Vec, Vec)
  ASMJIT_INST_3x(faddp, Faddp_v, Vec, Vec, Vec)
  ASMJIT_INST_4x(fccmp, Fccmp_v, Vec, Vec, Imm, Imm)
  ASMJIT_INST_4x(fccmpe, Fccmpe_v, Vec, Vec, Imm, Imm)
  ASMJIT_INST_3x(fcmeq, Fcmeq_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(fcmeq, Fcmeq_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(fcmge, Fcmge_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(fcmge, Fcmge_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(fcmgt, Fcmgt_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(fcmgt, Fcmgt_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(fcmle, Fcmle_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(fcmlt, Fcmlt_v, Vec, Vec, Imm)
  ASMJIT_INST_2x(fcmp, Fcmp_v, Vec, Vec)
  ASMJIT_INST_2x(fcmp, Fcmp_v, Vec, Imm)
  ASMJIT_INST_2x(fcmpe, Fcmpe_v, Vec, Vec)
  ASMJIT_INST_2x(fcmpe, Fcmpe_v, Vec, Imm)
  ASMJIT_INST_4x(fcsel, Fcsel_v, Vec, Vec, Vec, Imm)
  ASMJIT_INST_2x(fcvt, Fcvt_v, Vec, Vec)
  ASMJIT_INST_2x(fcvtas, Fcvtas_v, Gp, Vec)
  ASMJIT_INST_2x(fcvtas, Fcvtas_v, Vec, Vec)
  ASMJIT_INST_2x(fcvtau, Fcvtau_v, Gp, Vec)
  ASMJIT_INST_2x(fcvtau, Fcvtau_v, Vec, Vec)
  ASMJIT_INST_2x(fcvtl, Fcvtl_v, Vec, Vec)
  ASMJIT_INST_2x(fcvtl2, Fcvtl2_v, Vec, Vec)
  ASMJIT_INST_2x(fcvtms, Fcvtms_v, Gp, Vec)
  ASMJIT_INST_2x(fcvtms, Fcvtms_v, Vec, Vec)
  ASMJIT_INST_2x(fcvtmu, Fcvtmu_v, Gp, Vec)
  ASMJIT_INST_2x(fcvtmu, Fcvtmu_v, Vec, Vec)
  ASMJIT_INST_2x(fcvtn, Fcvtn_v, Vec, Vec)
  ASMJIT_INST_2x(fcvtn2, Fcvtn2_v, Vec, Vec)
  ASMJIT_INST_2x(fcvtns, Fcvtns_v, Gp, Vec)
  ASMJIT_INST_2x(fcvtns, Fcvtns_v, Vec, Vec)
  ASMJIT_INST_2x(fcvtnu, Fcvtnu_v, Gp, Vec)
  ASMJIT_INST_2x(fcvtnu, Fcvtnu_v, Vec, Vec)
  ASMJIT_INST_2x(fcvtps, Fcvtps_v, Gp, Vec)
  ASMJIT_INST_2x(fcvtps, Fcvtps_v, Vec, Vec)
  ASMJIT_INST_2x(fcvtpu, Fcvtpu_v, Gp, Vec)
  ASMJIT_INST_2x(fcvtpu, Fcvtpu_v, Vec, Vec)
  ASMJIT_INST_2x(fcvtxn, Fcvtxn_v, Vec, Vec)
  ASMJIT_INST_2x(fcvtxn2, Fcvtxn2_v, Vec, Vec)
  ASMJIT_INST_2x(fcvtzs, Fcvtzs_v, Gp, Vec)
  ASMJIT_INST_3x(fcvtzs, Fcvtzs_v, Gp, Vec, Imm)
  ASMJIT_INST_2x(fcvtzs, Fcvtzs_v, Vec, Vec)
  ASMJIT_INST_3x(fcvtzs, Fcvtzs_v, Vec, Vec, Imm)
  ASMJIT_INST_2x(fcvtzu, Fcvtzu_v, Gp, Vec)
  ASMJIT_INST_3x(fcvtzu, Fcvtzu_v, Gp, Vec, Imm)
  ASMJIT_INST_2x(fcvtzu, Fcvtzu_v, Vec, Vec)
  ASMJIT_INST_3x(fcvtzu, Fcvtzu_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(fdiv, Fdiv_v, Vec, Vec, Vec)
  ASMJIT_INST_4x(fmadd, Fmadd_v, Vec, Vec, Vec, Vec)
  ASMJIT_INST_3x(fmax, Fmax_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(fmaxnm, Fmaxnm_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(fmaxnmp, Fmaxnmp_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(fmaxnmp, Fmaxnmp_v, Vec, Vec)
  ASMJIT_INST_2x(fmaxnmv, Fmaxnmv_v, Vec, Vec)
  ASMJIT_INST_3x(fmaxp, Fmaxp_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(fmaxp, Fmaxp_v, Vec, Vec)
  ASMJIT_INST_2x(fmaxv, Fmaxv_v, Vec, Vec)
  ASMJIT_INST_3x(fmin, Fmin_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(fminnm, Fminnm_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(fminnmv, Fminnmv_v, Vec, Vec)
  ASMJIT_INST_3x(fminnmp, Fminnmp_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(fminnmp, Fminnmp_v, Vec, Vec)
  ASMJIT_INST_2x(fminp, Fminp_v, Vec, Vec)
  ASMJIT_INST_3x(fminp, Fminp_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(fminv, Fminv_v, Vec, Vec)
  ASMJIT_INST_3x(fmla, Fmla_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(fmls, Fmls_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(fmov, Fmov_v, Gp, Vec)
  ASMJIT_INST_2x(fmov, Fmov_v, Vec, Gp)
  ASMJIT_INST_2x(fmov, Fmov_v, Vec, Vec)
  ASMJIT_INST_2x(fmov, Fmov_v, Vec, Imm)
  ASMJIT_INST_4x(fmsub, Fmsub_v, Vec, Vec, Vec, Vec)
  ASMJIT_INST_3x(fmul, Fmul_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(fmulx, Fmulx_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(fneg, Fneg_v, Vec, Vec)
  ASMJIT_INST_4x(fnmadd, Fnmadd_v, Vec, Vec, Vec, Vec)
  ASMJIT_INST_4x(fnmsub, Fnmsub_v, Vec, Vec, Vec, Vec)
  ASMJIT_INST_3x(fnmul, Fnmul_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(frecpe, Frecpe_v, Vec, Vec)
  ASMJIT_INST_3x(frecps, Frecps_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(frecpx, Frecpx_v, Vec, Vec)
  ASMJIT_INST_2x(frint32x, Frint32x_v, Vec, Vec)
  ASMJIT_INST_2x(frint32z, Frint32z_v, Vec, Vec)
  ASMJIT_INST_2x(frint64x, Frint64x_v, Vec, Vec)
  ASMJIT_INST_2x(frint64z, Frint64z_v, Vec, Vec)
  ASMJIT_INST_2x(frinta, Frinta_v, Vec, Vec)
  ASMJIT_INST_2x(frinti, Frinti_v, Vec, Vec)
  ASMJIT_INST_2x(frintm, Frintm_v, Vec, Vec)
  ASMJIT_INST_2x(frintn, Frintn_v, Vec, Vec)
  ASMJIT_INST_2x(frintp, Frintp_v, Vec, Vec)
  ASMJIT_INST_2x(frintx, Frintx_v, Vec, Vec)
  ASMJIT_INST_2x(frintz, Frintz_v, Vec, Vec)
  ASMJIT_INST_2x(frsqrte, Frsqrte_v, Vec, Vec)
  ASMJIT_INST_3x(frsqrts, Frsqrts_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(fsqrt, Fsqrt_v, Vec, Vec)
  ASMJIT_INST_3x(fsub, Fsub_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(ins, Ins_v, Vec, Gp)
  ASMJIT_INST_2x(ins, Ins_v, Vec, Vec)
  ASMJIT_INST_2x(ld1, Ld1_v, Vec, Mem)
  ASMJIT_INST_3x(ld1, Ld1_v, Vec, Vec, Mem)
  ASMJIT_INST_4x(ld1, Ld1_v, Vec, Vec, Vec, Mem)
  ASMJIT_INST_5x(ld1, Ld1_v, Vec, Vec, Vec, Vec, Mem)
  ASMJIT_INST_2x(ld1r, Ld1r_v, Vec, Mem)
  ASMJIT_INST_3x(ld2, Ld2_v, Vec, Vec, Mem)
  ASMJIT_INST_3x(ld2r, Ld2r_v, Vec, Vec, Mem)
  ASMJIT_INST_4x(ld3, Ld3_v, Vec, Vec, Vec, Mem)
  ASMJIT_INST_4x(ld3r, Ld3r_v, Vec, Vec, Vec, Mem)
  ASMJIT_INST_5x(ld4, Ld4_v, Vec, Vec, Vec, Vec, Mem)
  ASMJIT_INST_5x(ld4r, Ld4r_v, Vec, Vec, Vec, Vec, Mem)
  ASMJIT_INST_3x(ldnp, Ldnp_v, Vec, Vec, Mem)
  ASMJIT_INST_3x(ldp, Ldp_v, Vec, Vec, Mem)
  ASMJIT_INST_2x(ldr, Ldr_v, Vec, Mem)
  ASMJIT_INST_2x(ldur, Ldur_v, Vec, Mem)
  ASMJIT_INST_3x(mla, Mla_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(mls, Mls_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(mov, Mov_v, Vec, Vec)
  ASMJIT_INST_2x(mov, Mov_v, Gp, Vec)
  ASMJIT_INST_2x(mov, Mov_v, Vec, Gp)
  ASMJIT_INST_2x(movi, Movi_v, Vec, Imm)
  ASMJIT_INST_3x(movi, Movi_v, Vec, Imm, Imm)
  ASMJIT_INST_3x(mul, Mul_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(mvn, Mvn_v, Vec, Vec)
  ASMJIT_INST_2x(mvn_, Mvn_v, Vec, Vec)
  ASMJIT_INST_2x(mvni, Mvni_v, Vec, Imm)
  ASMJIT_INST_3x(mvni, Mvni_v, Vec, Imm, Imm)
  ASMJIT_INST_2x(neg, Neg_v, Vec, Vec)
  ASMJIT_INST_2x(not_, Not_v, Vec, Vec)
  ASMJIT_INST_3x(orn, Orn_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(orr, Orr_v, Vec, Imm)
  ASMJIT_INST_3x(orr, Orr_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(orr, Orr_v, Vec, Imm, Imm)
  ASMJIT_INST_3x(pmul, Pmul_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(pmull, Pmull_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(pmull2, Pmull2_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(raddhn, Raddhn_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(raddhn2, Raddhn2_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(rbit, Rbit_v, Vec, Vec)
  ASMJIT_INST_2x(rev16, Rev16_v, Vec, Vec)
  ASMJIT_INST_2x(rev32, Rev32_v, Vec, Vec)
  ASMJIT_INST_2x(rev64, Rev64_v, Vec, Vec)
  ASMJIT_INST_3x(rshrn, Rshrn_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(rshrn2, Rshrn2_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(rsubhn, Rsubhn_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(rsubhn2, Rsubhn2_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(saba, Saba_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(sabal, Sabal_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(sabal2, Sabal2_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(sabd, Sabd_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(sabdl, Sabdl_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(sabdl2, Sabdl2_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(sadalp, Sadalp_v, Vec, Vec)
  ASMJIT_INST_3x(saddl, Saddl_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(saddl2, Saddl2_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(saddlp, Saddlp_v, Vec, Vec)
  ASMJIT_INST_2x(saddlv, Saddlv_v, Vec, Vec)
  ASMJIT_INST_3x(saddw, Saddw_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(saddw2, Saddw2_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(scvtf, Scvtf_v, Vec, Gp)
  ASMJIT_INST_3x(scvtf, Scvtf_v, Vec, Gp, Imm)
  ASMJIT_INST_2x(scvtf, Scvtf_v, Vec, Vec)
  ASMJIT_INST_3x(scvtf, Scvtf_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(shadd, Shadd_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(shl, Shl_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(shll, Shll_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(shll2, Shll2_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(shrn, Shrn_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(shrn2, Shrn2_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(shsub, Shsub_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(sli, Sli_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(smax, Smax_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(smaxp, Smaxp_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(smaxv, Smaxv_v, Vec, Vec)
  ASMJIT_INST_3x(smin, Smin_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(sminp, Sminp_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(sminv, Sminv_v, Vec, Vec)
  ASMJIT_INST_3x(smlal, Smlal_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(smlal2, Smlal2_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(smlsl, Smlsl_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(smlsl2, Smlsl2_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(smov, Smov_v, Gp, Vec)
  ASMJIT_INST_3x(smull, Smull_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(smull2, Smull2_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(sqabs, Sqabs_v, Vec, Vec)
  ASMJIT_INST_3x(sqadd, Sqadd_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(sqdmlal, Sqdmlal_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(sqdmlal2, Sqdmlal2_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(sqdmlsl, Sqdmlsl_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(sqdmlsl2, Sqdmlsl2_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(sqdmulh, Sqdmulh_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(sqdmull, Sqdmull_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(sqdmull2, Sqdmull2_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(sqneg, Sqneg_v, Vec, Vec)
  ASMJIT_INST_3x(sqrdmulh, Sqrdmulh_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(sqrshl, Sqrshl_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(sqrshrn, Sqrshrn_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(sqrshrn2, Sqrshrn2_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(sqrshrun, Sqrshrun_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(sqrshrun2, Sqrshrun2_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(sqshl, Sqshl_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(sqshl, Sqshl_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(sqshlu, Sqshlu_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(sqshrn, Sqshrn_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(sqshrn2, Sqshrn2_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(sqshrun, Sqshrun_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(sqshrun2, Sqshrun2_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(sqsub, Sqsub_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(sqxtn, Sqxtn_v, Vec, Vec)
  ASMJIT_INST_2x(sqxtn2, Sqxtn2_v, Vec, Vec)
  ASMJIT_INST_2x(sqxtun, Sqxtun_v, Vec, Vec)
  ASMJIT_INST_2x(sqxtun2, Sqxtun2_v, Vec, Vec)
  ASMJIT_INST_3x(srhadd, Srhadd_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(sri, Sri_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(srshl, Srshl_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(srshr, Srshr_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(srsra, Srsra_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(sshl, Sshl_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(sshll, Sshll_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(sshll2, Sshll2_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(sshr, Sshr_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(ssra, Ssra_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(ssubl, Ssubl_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(ssubl2, Ssubl2_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(ssubw, Ssubw_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(ssubw2, Ssubw2_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(st1, St1_v, Vec, Mem)
  ASMJIT_INST_3x(st1, St1_v, Vec, Vec, Mem)
  ASMJIT_INST_4x(st1, St1_v, Vec, Vec, Vec, Mem)
  ASMJIT_INST_5x(st1, St1_v, Vec, Vec, Vec, Vec, Mem)
  ASMJIT_INST_3x(st2, St2_v, Vec, Vec, Mem)
  ASMJIT_INST_4x(st3, St3_v, Vec, Vec, Vec, Mem)
  ASMJIT_INST_5x(st4, St4_v, Vec, Vec, Vec, Vec, Mem)
  ASMJIT_INST_3x(stnp, Stnp_v, Vec, Vec, Mem)
  ASMJIT_INST_3x(stp, Stp_v, Vec, Vec, Mem)
  ASMJIT_INST_2x(str, Str_v, Vec, Mem)
  ASMJIT_INST_2x(stur, Stur_v, Vec, Mem)
  ASMJIT_INST_3x(sub, Sub_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(subhn, Subhn_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(subhn2, Subhn2_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(suqadd, Suqadd_v, Vec, Vec)
  ASMJIT_INST_2x(sxtl, Sxtl_v, Vec, Vec)
  ASMJIT_INST_2x(sxtl2, Sxtl2_v, Vec, Vec)
  ASMJIT_INST_3x(tbl, Tbl_v, Vec, Vec, Vec)
  ASMJIT_INST_4x(tbl, Tbl_v, Vec, Vec, Vec, Vec)
  ASMJIT_INST_5x(tbl, Tbl_v, Vec, Vec, Vec, Vec, Vec)
  ASMJIT_INST_6x(tbl, Tbl_v, Vec, Vec, Vec, Vec, Vec, Vec)
  ASMJIT_INST_3x(tbx, Tbx_v, Vec, Vec, Vec)
  ASMJIT_INST_4x(tbx, Tbx_v, Vec, Vec, Vec, Vec)
  ASMJIT_INST_5x(tbx, Tbx_v, Vec, Vec, Vec, Vec, Vec)
  ASMJIT_INST_6x(tbx, Tbx_v, Vec, Vec, Vec, Vec, Vec, Vec)
  ASMJIT_INST_3x(trn1, Trn1_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(trn2, Trn2_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(uaba, Uaba_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(uabal, Uabal_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(uabal2, Uabal2_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(uabd, Uabd_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(uabdl, Uabdl_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(uabdl2, Uabdl2_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(uadalp, Uadalp_v, Vec, Vec)
  ASMJIT_INST_3x(uaddl, Uaddl_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(uaddl2, Uaddl2_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(uaddlp, Uaddlp_v, Vec, Vec)
  ASMJIT_INST_2x(uaddlv, Uaddlv_v, Vec, Vec)
  ASMJIT_INST_3x(uaddw, Uaddw_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(uaddw2, Uaddw2_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(ucvtf, Ucvtf_v, Vec, Gp)
  ASMJIT_INST_3x(ucvtf, Ucvtf_v, Vec, Gp, Imm)
  ASMJIT_INST_2x(ucvtf, Ucvtf_v, Vec, Vec)
  ASMJIT_INST_3x(ucvtf, Ucvtf_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(uhadd, Uhadd_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(uhsub, Uhsub_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(umax, Umax_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(umaxp, Umaxp_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(umaxv, Umaxv_v, Vec, Vec)
  ASMJIT_INST_3x(umin, Umin_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(uminp, Uminp_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(uminv, Uminv_v, Vec, Vec)
  ASMJIT_INST_3x(umlal, Umlal_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(umlal2, Umlal2_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(umlsl, Umlsl_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(umlsl2, Umlsl2_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(umov, Umov_v, Gp, Vec)
  ASMJIT_INST_3x(umull, Umull_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(umull2, Umull2_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(uqadd, Uqadd_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(uqrshl, Uqrshl_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(uqrshl, Uqrshl_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(uqrshrn, Uqrshrn_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(uqrshrn2, Uqrshrn2_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(uqshl, Uqshl_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(uqshl, Uqshl_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(uqshrn, Uqshrn_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(uqshrn2, Uqshrn2_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(uqsub, Uqsub_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(uqxtn, Uqxtn_v, Vec, Vec)
  ASMJIT_INST_2x(uqxtn2, Uqxtn2_v, Vec, Vec)
  ASMJIT_INST_2x(urecpe, Urecpe_v, Vec, Vec)
  ASMJIT_INST_3x(urhadd, Urhadd_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(urshl, Urshl_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(urshr, Urshr_v, Vec, Vec, Imm)
  ASMJIT_INST_2x(ursqrte, Ursqrte_v, Vec, Vec)
  ASMJIT_INST_3x(ursra, Ursra_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(ushl, Ushl_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(ushll, Ushll_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(ushll2, Ushll2_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(ushr, Ushr_v, Vec, Vec, Imm)
  ASMJIT_INST_2x(usqadd, Usqadd_v, Vec, Vec)
  ASMJIT_INST_3x(usra, Usra_v, Vec, Vec, Imm)
  ASMJIT_INST_3x(usubl, Usubl_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(usubl2, Usubl2_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(usubw, Usubw_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(usubw2, Usubw2_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(uxtl, Uxtl_v, Vec, Vec)
  ASMJIT_INST_2x(uxtl2, Uxtl2_v, Vec, Vec)
  ASMJIT_INST_3x(uzp1, Uzp1_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(uzp2, Uzp2_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(xtn, Xtn_v, Vec, Vec)
  ASMJIT_INST_2x(xtn2, Xtn2_v, Vec, Vec)
  ASMJIT_INST_3x(zip1, Zip1_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(zip2, Zip2_v, Vec, Vec, Vec)

  //! \}

  //! \name AES Instructions
  //! \{

  ASMJIT_INST_2x(aesd, Aesd_v, Vec, Vec)
  ASMJIT_INST_2x(aese, Aese_v, Vec, Vec)
  ASMJIT_INST_2x(aesimc, Aesimc_v, Vec, Vec)
  ASMJIT_INST_2x(aesmc, Aesmc_v, Vec, Vec)

  //! \}

  //! \name SHA1 Instructions
  //! \{

  ASMJIT_INST_3x(sha1c, Sha1c_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(sha1h, Sha1h_v, Vec, Vec)
  ASMJIT_INST_3x(sha1m, Sha1m_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(sha1p, Sha1p_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(sha1su0, Sha1su0_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(sha1su1, Sha1su1_v, Vec, Vec)

  //! \}

  //! \name SHA2 Instructions
  //! \{

  ASMJIT_INST_3x(sha256h, Sha256h_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(sha256h2, Sha256h2_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(sha256su0, Sha256su0_v, Vec, Vec)
  ASMJIT_INST_3x(sha256su1, Sha256su1_v, Vec, Vec, Vec)

  //! \}

  //! \name RDMA Instructions (ARMv8.1-A)
  //! \{

  ASMJIT_INST_3x(sqrdmlah, Sqrdmlah_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(sqrdmlsh, Sqrdmlsh_v, Vec, Vec, Vec)

  //! \}

  //! \name FCMA Instruction (ARMv8.3-A)
  //! \{

  ASMJIT_INST_4x(fcadd, Fcadd_v, Vec, Vec, Vec, Imm)
  ASMJIT_INST_4x(fcmla, Fcmla_v, Vec, Vec, Vec, Imm)

  //! \}

  //! \name JSCVT Instruction (ARMv8.3-A)
  //! \{

  ASMJIT_INST_2x(fjcvtzs, Fjcvtzs_v, Gp, Vec)

  //! \}

  //! \name FHM Instructions
  //! \{

  ASMJIT_INST_3x(fmlal, Fmlal_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(fmlal2, Fmlal2_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(fmlsl, Fmlsl_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(fmlsl2, Fmlsl2_v, Vec, Vec, Vec)


  //! \}

  //! \name SHA3 Instructions (ARMv8.4-A, optional in ARMv8.2-A)
  //! \{

  ASMJIT_INST_4x(bcax, Bcax_v, Vec, Vec, Vec, Vec)
  ASMJIT_INST_4x(eor3, Eor3_v, Vec, Vec, Vec, Vec)
  ASMJIT_INST_3x(rax1, Rax1_v, Vec, Vec, Vec)
  ASMJIT_INST_4x(xar, Xar_v, Vec, Vec, Vec, Imm)

  //! \}

  //! \name SHA512 Instructions (ARMv8.4-A)
  //! \{

  ASMJIT_INST_3x(sha512h, Sha512h_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(sha512h2, Sha512h2_v, Vec, Vec, Vec)
  ASMJIT_INST_2x(sha512su0, Sha512su0_v, Vec, Vec)
  ASMJIT_INST_3x(sha512su1, Sha512su1_v, Vec, Vec, Vec)

  //! \}

  //! \name SM3 Instructions (ARMv8.4-A)
  //! \{

  ASMJIT_INST_3x(sm3partw1, Sm3partw1_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(sm3partw2, Sm3partw2_v, Vec, Vec, Vec)
  ASMJIT_INST_4x(sm3ss1, Sm3ss1_v, Vec, Vec, Vec, Vec)
  ASMJIT_INST_3x(sm3tt1a, Sm3tt1a_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(sm3tt1b, Sm3tt1b_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(sm3tt2a, Sm3tt2a_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(sm3tt2b, Sm3tt2b_v, Vec, Vec, Vec)

  //! \}

  //! \name SM4 Instructions (ARMv8.4-A)
  //! \{

  ASMJIT_INST_2x(sm4e, Sm4e_v, Vec, Vec)
  ASMJIT_INST_3x(sm4ekey, Sm4ekey_v, Vec, Vec, Vec)

  //! \}

  //! \name DOTPROD Instructions (ARMv8.4-A, optional in ARMv8.2-A)
  //! \{

  ASMJIT_INST_3x(sdot, Sdot_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(udot, Udot_v, Vec, Vec, Vec)

  //! \}

  //! \name BF16 Instructions (ARMv8.6-A)
  //! \{

  ASMJIT_INST_2x(bfcvt, Bfcvt_v, Vec, Vec)
  ASMJIT_INST_2x(bfcvtn, Bfcvtn_v, Vec, Vec)
  ASMJIT_INST_2x(bfcvtn2, Bfcvtn2_v, Vec, Vec)
  ASMJIT_INST_3x(bfmlalb, Bfmlalb_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(bfmlalt, Bfmlalt_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(bfmmla, Bfmmla_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(bfdot, Bfdot_v, Vec, Vec, Vec)

  //! \}

  //! \name I8MM Instructions (ARMv8.6-A)
  //! \{

  ASMJIT_INST_3x(smmla, Smmla_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(sudot, Sudot_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(ummla, Ummla_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(usdot, Usdot_v, Vec, Vec, Vec)
  ASMJIT_INST_3x(usmmla, Usmmla_v, Vec, Vec, Vec)

  //! \}
};

//! Emitter (AArch64).
//!
//! \note This class cannot be instantiated, you can only cast to it and use it as emitter that emits to either
//! `a64::Assembler`, `a64::Builder`, or `a64::Compiler` (use with caution with `a64::Compiler` as it requires
//! virtual registers).
class Emitter : public BaseEmitter, public EmitterExplicitT<Emitter> {
  ASMJIT_NONCONSTRUCTIBLE(Emitter)
};

//! \}

#undef ASMJIT_INST_0x
#undef ASMJIT_INST_1x
#undef ASMJIT_INST_2x
#undef ASMJIT_INST_3x
#undef ASMJIT_INST_4x
#undef ASMJIT_INST_5x
#undef ASMJIT_INST_6x
#undef ASMJIT_INST_1cc

ASMJIT_END_SUB_NAMESPACE

// Restore undefined MSVC AArch64 macros.
#if defined(ASMJIT_RESTORE_MSVC_AARCH64_MACROS)
  #pragma pop_macro("mvn")
#endif

#endif // ASMJIT_ARM_A64EMITTER_H_INCLUDED
