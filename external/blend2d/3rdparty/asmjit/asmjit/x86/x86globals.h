// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_X86_X86GLOBALS_H_INCLUDED
#define ASMJIT_X86_X86GLOBALS_H_INCLUDED

#include <asmjit/core/archtraits.h>
#include <asmjit/core/inst.h>

//! \namespace asmjit::x86
//! \ingroup asmjit_x86
//!
//! X86/X64 API.

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

//! \addtogroup asmjit_x86
//! \{

//! Condition code.
enum class CondCode : uint8_t {
  kO             = 0x00u,       //!<         OF==1
  kNO            = 0x01u,       //!<         OF==0
  kC             = 0x02u,       //!< CF==1
  kB             = 0x02u,       //!< CF==1          (unsigned < )
  kNAE           = 0x02u,       //!< CF==1          (unsigned < )
  kNC            = 0x03u,       //!< CF==0
  kAE            = 0x03u,       //!< CF==0          (unsigned >=)
  kNB            = 0x03u,       //!< CF==0          (unsigned >=)
  kE             = 0x04u,       //!<         ZF==1  (any_sign ==)
  kZ             = 0x04u,       //!<         ZF==1  (any_sign ==)
  kNE            = 0x05u,       //!<         ZF==0  (any_sign !=)
  kNZ            = 0x05u,       //!<         ZF==0  (any_sign !=)
  kBE            = 0x06u,       //!< CF==1 | ZF==1  (unsigned <=)
  kNA            = 0x06u,       //!< CF==1 | ZF==1  (unsigned <=)
  kA             = 0x07u,       //!< CF==0 & ZF==0  (unsigned > )
  kNBE           = 0x07u,       //!< CF==0 & ZF==0  (unsigned > )
  kS             = 0x08u,       //!<         SF==1  (is negative)
  kNS            = 0x09u,       //!<         SF==0  (is positive or zero)
  kP             = 0x0Au,       //!< PF==1
  kPE            = 0x0Au,       //!< PF==1
  kPO            = 0x0Bu,       //!< PF==0
  kNP            = 0x0Bu,       //!< PF==0
  kL             = 0x0Cu,       //!<         SF!=OF (signed < )
  kNGE           = 0x0Cu,       //!<         SF!=OF (signed < )
  kGE            = 0x0Du,       //!<         SF==OF (signed >=)
  kNL            = 0x0Du,       //!<         SF==OF (signed >=)
  kLE            = 0x0Eu,       //!< ZF==1 | SF!=OF (signed <=)
  kNG            = 0x0Eu,       //!< ZF==1 | SF!=OF (signed <=)
  kG             = 0x0Fu,       //!< ZF==0 & SF==OF (signed > )
  kNLE           = 0x0Fu,       //!< ZF==0 & SF==OF (signed > )

  kZero          = kZ,          //!< Zero flag.
  kNotZero       = kNZ,         //!< Not zero.

  kEqual         = kE,          //!< `a == b` (equal).
  kNotEqual      = kNE,         //!< `a != b` (not equal).

  kCarry         = kC,          //!< Carry flag.
  kNotCarry      = kNC,         //!< Not carry.

  kSign          = kS,          //!< Sign flag.
  kNotSign       = kNS,         //!< Not sign.

  kNegative      = kS,          //!< Sign flag.
  kPositive      = kNS,         //!< Not sign.

  kOverflow      = kO,          //!< Overflow (signed).
  kNotOverflow   = kNO,         //!< Not overflow (signed).

  kSignedLT      = kL,          //!< `a <  b` (signed).
  kSignedLE      = kLE,         //!< `a <= b` (signed).
  kSignedGT      = kG,          //!< `a >  b` (signed).
  kSignedGE      = kGE,         //!< `a >= b` (signed).

  kUnsignedLT    = kB,          //!< `a <  b` (unsigned).
  kUnsignedLE    = kBE,         //!< `a <= b` (unsigned).
  kUnsignedGT    = kA,          //!< `a >  b` (unsigned).
  kUnsignedGE    = kAE,         //!< `a >= b` (unsigned).

  kBTZero        = kNC,         //!< Tested bit is zero.
  kBTNotZero     = kC,          //!< Tested bit is non-zero.

  kParityEven    = kP,          //!< Even parity flag.
  kParityOdd     = kPO,         //!< Odd parity flag.

  kMaxValue      = 0x0Fu
};

//! \cond
static constexpr CondCode _reverse_cond_table[] = {
  CondCode::kO,  // O  <- O
  CondCode::kNO, // NO <- NO
  CondCode::kA , // A  <- B
  CondCode::kBE, // BE <- AE
  CondCode::kE,  // E  <- E
  CondCode::kNE, // NE <- NE
  CondCode::kAE, // AE <- BE
  CondCode::kB , // B  <- A
  CondCode::kS,  // S  <- S
  CondCode::kNS, // NS <- NS
  CondCode::kPE, // PE <- PE
  CondCode::kPO, // PO <- PO
  CondCode::kG,  // G  <- L
  CondCode::kLE, // LE <- GE
  CondCode::kGE, // GE <- LE
  CondCode::kL   // L  <- G
};
//! \endcond

//! Reverses a condition code (reverses the corresponding operands of a comparison).
static ASMJIT_INLINE_CONSTEXPR CondCode reverse_cond(CondCode cond) noexcept { return _reverse_cond_table[uint8_t(cond)]; }
//! Negates a condition code.
static ASMJIT_INLINE_CONSTEXPR CondCode negate_cond(CondCode cond) noexcept { return CondCode(uint8_t(cond) ^ 1u); }

//! Instruction.
//!
//! \note Only used to hold x86-specific instruction identifiers and some additional helper functions.
namespace Inst {
  //! Instruction id.
  enum Id : uint32_t {
    // ${InstId:Begin}
    kIdNone = 0,                         //!< Invalid instruction id.
    kIdAaa,                              //!< Instruction 'aaa' (X86).
    kIdAad,                              //!< Instruction 'aad' (X86).
    kIdAadd,                             //!< Instruction 'aadd' {RAO_INT}.
    kIdAam,                              //!< Instruction 'aam' (X86).
    kIdAand,                             //!< Instruction 'aand' {RAO_INT}.
    kIdAas,                              //!< Instruction 'aas' (X86).
    kIdAdc,                              //!< Instruction 'adc'.
    kIdAdcx,                             //!< Instruction 'adcx' {ADX}.
    kIdAdd,                              //!< Instruction 'add'.
    kIdAddpd,                            //!< Instruction 'addpd' {SSE2}.
    kIdAddps,                            //!< Instruction 'addps' {SSE}.
    kIdAddsd,                            //!< Instruction 'addsd' {SSE2}.
    kIdAddss,                            //!< Instruction 'addss' {SSE}.
    kIdAddsubpd,                         //!< Instruction 'addsubpd' {SSE3}.
    kIdAddsubps,                         //!< Instruction 'addsubps' {SSE3}.
    kIdAdox,                             //!< Instruction 'adox' {ADX}.
    kIdAesdec,                           //!< Instruction 'aesdec' {AESNI}.
    kIdAesdeclast,                       //!< Instruction 'aesdeclast' {AESNI}.
    kIdAesenc,                           //!< Instruction 'aesenc' {AESNI}.
    kIdAesenclast,                       //!< Instruction 'aesenclast' {AESNI}.
    kIdAesimc,                           //!< Instruction 'aesimc' {AESNI}.
    kIdAeskeygenassist,                  //!< Instruction 'aeskeygenassist' {AESNI}.
    kIdAnd,                              //!< Instruction 'and'.
    kIdAndn,                             //!< Instruction 'andn' {BMI}.
    kIdAndnpd,                           //!< Instruction 'andnpd' {SSE2}.
    kIdAndnps,                           //!< Instruction 'andnps' {SSE}.
    kIdAndpd,                            //!< Instruction 'andpd' {SSE2}.
    kIdAndps,                            //!< Instruction 'andps' {SSE}.
    kIdAor,                              //!< Instruction 'aor' {RAO_INT}.
    kIdArpl,                             //!< Instruction 'arpl' (X86).
    kIdAxor,                             //!< Instruction 'axor' {RAO_INT}.
    kIdBextr,                            //!< Instruction 'bextr' {BMI}.
    kIdBlcfill,                          //!< Instruction 'blcfill' {TBM}.
    kIdBlci,                             //!< Instruction 'blci' {TBM}.
    kIdBlcic,                            //!< Instruction 'blcic' {TBM}.
    kIdBlcmsk,                           //!< Instruction 'blcmsk' {TBM}.
    kIdBlcs,                             //!< Instruction 'blcs' {TBM}.
    kIdBlendpd,                          //!< Instruction 'blendpd' {SSE4_1}.
    kIdBlendps,                          //!< Instruction 'blendps' {SSE4_1}.
    kIdBlendvpd,                         //!< Instruction 'blendvpd' {SSE4_1}.
    kIdBlendvps,                         //!< Instruction 'blendvps' {SSE4_1}.
    kIdBlsfill,                          //!< Instruction 'blsfill' {TBM}.
    kIdBlsi,                             //!< Instruction 'blsi' {BMI}.
    kIdBlsic,                            //!< Instruction 'blsic' {TBM}.
    kIdBlsmsk,                           //!< Instruction 'blsmsk' {BMI}.
    kIdBlsr,                             //!< Instruction 'blsr' {BMI}.
    kIdBndcl,                            //!< Instruction 'bndcl' {MPX}.
    kIdBndcn,                            //!< Instruction 'bndcn' {MPX}.
    kIdBndcu,                            //!< Instruction 'bndcu' {MPX}.
    kIdBndldx,                           //!< Instruction 'bndldx' {MPX}.
    kIdBndmk,                            //!< Instruction 'bndmk' {MPX}.
    kIdBndmov,                           //!< Instruction 'bndmov' {MPX}.
    kIdBndstx,                           //!< Instruction 'bndstx' {MPX}.
    kIdBound,                            //!< Instruction 'bound' (X86).
    kIdBsf,                              //!< Instruction 'bsf'.
    kIdBsr,                              //!< Instruction 'bsr'.
    kIdBswap,                            //!< Instruction 'bswap'.
    kIdBt,                               //!< Instruction 'bt'.
    kIdBtc,                              //!< Instruction 'btc'.
    kIdBtr,                              //!< Instruction 'btr'.
    kIdBts,                              //!< Instruction 'bts'.
    kIdBzhi,                             //!< Instruction 'bzhi' {BMI2}.
    kIdCall,                             //!< Instruction 'call'.
    kIdCbw,                              //!< Instruction 'cbw'.
    kIdCdq,                              //!< Instruction 'cdq'.
    kIdCdqe,                             //!< Instruction 'cdqe' (X64).
    kIdClac,                             //!< Instruction 'clac' {SMAP}.
    kIdClc,                              //!< Instruction 'clc'.
    kIdCld,                              //!< Instruction 'cld'.
    kIdCldemote,                         //!< Instruction 'cldemote' {CLDEMOTE}.
    kIdClflush,                          //!< Instruction 'clflush' {CLFLUSH}.
    kIdClflushopt,                       //!< Instruction 'clflushopt' {CLFLUSHOPT}.
    kIdClgi,                             //!< Instruction 'clgi' {SVM}.
    kIdCli,                              //!< Instruction 'cli'.
    kIdClrssbsy,                         //!< Instruction 'clrssbsy' {CET_SS}.
    kIdClts,                             //!< Instruction 'clts'.
    kIdClui,                             //!< Instruction 'clui' {UINTR} (X64).
    kIdClwb,                             //!< Instruction 'clwb' {CLWB}.
    kIdClzero,                           //!< Instruction 'clzero' {CLZERO}.
    kIdCmc,                              //!< Instruction 'cmc'.
    kIdCmovb,                            //!< Instruction 'cmovb' {CMOV}.
    kIdCmovbe,                           //!< Instruction 'cmovbe' {CMOV}.
    kIdCmovl,                            //!< Instruction 'cmovl' {CMOV}.
    kIdCmovle,                           //!< Instruction 'cmovle' {CMOV}.
    kIdCmovnb,                           //!< Instruction 'cmovnb' {CMOV}.
    kIdCmovnbe,                          //!< Instruction 'cmovnbe' {CMOV}.
    kIdCmovnl,                           //!< Instruction 'cmovnl' {CMOV}.
    kIdCmovnle,                          //!< Instruction 'cmovnle' {CMOV}.
    kIdCmovno,                           //!< Instruction 'cmovno' {CMOV}.
    kIdCmovnp,                           //!< Instruction 'cmovnp' {CMOV}.
    kIdCmovns,                           //!< Instruction 'cmovns' {CMOV}.
    kIdCmovnz,                           //!< Instruction 'cmovnz' {CMOV}.
    kIdCmovo,                            //!< Instruction 'cmovo' {CMOV}.
    kIdCmovp,                            //!< Instruction 'cmovp' {CMOV}.
    kIdCmovs,                            //!< Instruction 'cmovs' {CMOV}.
    kIdCmovz,                            //!< Instruction 'cmovz' {CMOV}.
    kIdCmp,                              //!< Instruction 'cmp'.
    kIdCmpbexadd,                        //!< Instruction 'cmpbexadd' {CMPCCXADD}.
    kIdCmpbxadd,                         //!< Instruction 'cmpbxadd' {CMPCCXADD}.
    kIdCmplexadd,                        //!< Instruction 'cmplexadd' {CMPCCXADD}.
    kIdCmplxadd,                         //!< Instruction 'cmplxadd' {CMPCCXADD}.
    kIdCmpnbexadd,                       //!< Instruction 'cmpnbexadd' {CMPCCXADD}.
    kIdCmpnbxadd,                        //!< Instruction 'cmpnbxadd' {CMPCCXADD}.
    kIdCmpnlexadd,                       //!< Instruction 'cmpnlexadd' {CMPCCXADD}.
    kIdCmpnlxadd,                        //!< Instruction 'cmpnlxadd' {CMPCCXADD}.
    kIdCmpnoxadd,                        //!< Instruction 'cmpnoxadd' {CMPCCXADD}.
    kIdCmpnpxadd,                        //!< Instruction 'cmpnpxadd' {CMPCCXADD}.
    kIdCmpnsxadd,                        //!< Instruction 'cmpnsxadd' {CMPCCXADD}.
    kIdCmpnzxadd,                        //!< Instruction 'cmpnzxadd' {CMPCCXADD}.
    kIdCmpoxadd,                         //!< Instruction 'cmpoxadd' {CMPCCXADD}.
    kIdCmppd,                            //!< Instruction 'cmppd' {SSE2}.
    kIdCmpps,                            //!< Instruction 'cmpps' {SSE}.
    kIdCmppxadd,                         //!< Instruction 'cmppxadd' {CMPCCXADD}.
    kIdCmps,                             //!< Instruction 'cmps'.
    kIdCmpsd,                            //!< Instruction 'cmpsd' {SSE2}.
    kIdCmpss,                            //!< Instruction 'cmpss' {SSE}.
    kIdCmpsxadd,                         //!< Instruction 'cmpsxadd' {CMPCCXADD}.
    kIdCmpxchg,                          //!< Instruction 'cmpxchg' {I486}.
    kIdCmpxchg16b,                       //!< Instruction 'cmpxchg16b' {CMPXCHG16B} (X64).
    kIdCmpxchg8b,                        //!< Instruction 'cmpxchg8b' {CMPXCHG8B}.
    kIdCmpzxadd,                         //!< Instruction 'cmpzxadd' {CMPCCXADD}.
    kIdComisd,                           //!< Instruction 'comisd' {SSE2}.
    kIdComiss,                           //!< Instruction 'comiss' {SSE}.
    kIdCpuid,                            //!< Instruction 'cpuid' {I486}.
    kIdCqo,                              //!< Instruction 'cqo' (X64).
    kIdCrc32,                            //!< Instruction 'crc32' {SSE4_2}.
    kIdCvtdq2pd,                         //!< Instruction 'cvtdq2pd' {SSE2}.
    kIdCvtdq2ps,                         //!< Instruction 'cvtdq2ps' {SSE2}.
    kIdCvtpd2dq,                         //!< Instruction 'cvtpd2dq' {SSE2}.
    kIdCvtpd2pi,                         //!< Instruction 'cvtpd2pi' {SSE2}.
    kIdCvtpd2ps,                         //!< Instruction 'cvtpd2ps' {SSE2}.
    kIdCvtpi2pd,                         //!< Instruction 'cvtpi2pd' {SSE2}.
    kIdCvtpi2ps,                         //!< Instruction 'cvtpi2ps' {SSE}.
    kIdCvtps2dq,                         //!< Instruction 'cvtps2dq' {SSE2}.
    kIdCvtps2pd,                         //!< Instruction 'cvtps2pd' {SSE2}.
    kIdCvtps2pi,                         //!< Instruction 'cvtps2pi' {SSE}.
    kIdCvtsd2si,                         //!< Instruction 'cvtsd2si' {SSE2}.
    kIdCvtsd2ss,                         //!< Instruction 'cvtsd2ss' {SSE2}.
    kIdCvtsi2sd,                         //!< Instruction 'cvtsi2sd' {SSE2}.
    kIdCvtsi2ss,                         //!< Instruction 'cvtsi2ss' {SSE}.
    kIdCvtss2sd,                         //!< Instruction 'cvtss2sd' {SSE2}.
    kIdCvtss2si,                         //!< Instruction 'cvtss2si' {SSE}.
    kIdCvttpd2dq,                        //!< Instruction 'cvttpd2dq' {SSE2}.
    kIdCvttpd2pi,                        //!< Instruction 'cvttpd2pi' {SSE2}.
    kIdCvttps2dq,                        //!< Instruction 'cvttps2dq' {SSE2}.
    kIdCvttps2pi,                        //!< Instruction 'cvttps2pi' {SSE}.
    kIdCvttsd2si,                        //!< Instruction 'cvttsd2si' {SSE2}.
    kIdCvttss2si,                        //!< Instruction 'cvttss2si' {SSE}.
    kIdCwd,                              //!< Instruction 'cwd'.
    kIdCwde,                             //!< Instruction 'cwde'.
    kIdDaa,                              //!< Instruction 'daa' (X86).
    kIdDas,                              //!< Instruction 'das' (X86).
    kIdDec,                              //!< Instruction 'dec'.
    kIdDiv,                              //!< Instruction 'div'.
    kIdDivpd,                            //!< Instruction 'divpd' {SSE2}.
    kIdDivps,                            //!< Instruction 'divps' {SSE}.
    kIdDivsd,                            //!< Instruction 'divsd' {SSE2}.
    kIdDivss,                            //!< Instruction 'divss' {SSE}.
    kIdDppd,                             //!< Instruction 'dppd' {SSE4_1}.
    kIdDpps,                             //!< Instruction 'dpps' {SSE4_1}.
    kIdEmms,                             //!< Instruction 'emms' {MMX}.
    kIdEndbr32,                          //!< Instruction 'endbr32' {CET_IBT}.
    kIdEndbr64,                          //!< Instruction 'endbr64' {CET_IBT}.
    kIdEnqcmd,                           //!< Instruction 'enqcmd' {ENQCMD}.
    kIdEnqcmds,                          //!< Instruction 'enqcmds' {ENQCMD}.
    kIdEnter,                            //!< Instruction 'enter'.
    kIdExtractps,                        //!< Instruction 'extractps' {SSE4_1}.
    kIdExtrq,                            //!< Instruction 'extrq' {SSE4A}.
    kIdF2xm1,                            //!< Instruction 'f2xm1' {FPU}.
    kIdFabs,                             //!< Instruction 'fabs' {FPU}.
    kIdFadd,                             //!< Instruction 'fadd' {FPU}.
    kIdFaddp,                            //!< Instruction 'faddp' {FPU}.
    kIdFbld,                             //!< Instruction 'fbld' {FPU}.
    kIdFbstp,                            //!< Instruction 'fbstp' {FPU}.
    kIdFchs,                             //!< Instruction 'fchs' {FPU}.
    kIdFclex,                            //!< Instruction 'fclex' {FPU}.
    kIdFcmovb,                           //!< Instruction 'fcmovb' {CMOV|FPU}.
    kIdFcmovbe,                          //!< Instruction 'fcmovbe' {CMOV|FPU}.
    kIdFcmove,                           //!< Instruction 'fcmove' {CMOV|FPU}.
    kIdFcmovnb,                          //!< Instruction 'fcmovnb' {CMOV|FPU}.
    kIdFcmovnbe,                         //!< Instruction 'fcmovnbe' {CMOV|FPU}.
    kIdFcmovne,                          //!< Instruction 'fcmovne' {CMOV|FPU}.
    kIdFcmovnu,                          //!< Instruction 'fcmovnu' {CMOV|FPU}.
    kIdFcmovu,                           //!< Instruction 'fcmovu' {CMOV|FPU}.
    kIdFcom,                             //!< Instruction 'fcom' {FPU}.
    kIdFcomi,                            //!< Instruction 'fcomi' {FPU}.
    kIdFcomip,                           //!< Instruction 'fcomip' {FPU}.
    kIdFcomp,                            //!< Instruction 'fcomp' {FPU}.
    kIdFcompp,                           //!< Instruction 'fcompp' {FPU}.
    kIdFcos,                             //!< Instruction 'fcos' {FPU}.
    kIdFdecstp,                          //!< Instruction 'fdecstp' {FPU}.
    kIdFdiv,                             //!< Instruction 'fdiv' {FPU}.
    kIdFdivp,                            //!< Instruction 'fdivp' {FPU}.
    kIdFdivr,                            //!< Instruction 'fdivr' {FPU}.
    kIdFdivrp,                           //!< Instruction 'fdivrp' {FPU}.
    kIdFemms,                            //!< Instruction 'femms' {3DNOW}.
    kIdFfree,                            //!< Instruction 'ffree' {FPU}.
    kIdFiadd,                            //!< Instruction 'fiadd' {FPU}.
    kIdFicom,                            //!< Instruction 'ficom' {FPU}.
    kIdFicomp,                           //!< Instruction 'ficomp' {FPU}.
    kIdFidiv,                            //!< Instruction 'fidiv' {FPU}.
    kIdFidivr,                           //!< Instruction 'fidivr' {FPU}.
    kIdFild,                             //!< Instruction 'fild' {FPU}.
    kIdFimul,                            //!< Instruction 'fimul' {FPU}.
    kIdFincstp,                          //!< Instruction 'fincstp' {FPU}.
    kIdFinit,                            //!< Instruction 'finit' {FPU}.
    kIdFist,                             //!< Instruction 'fist' {FPU}.
    kIdFistp,                            //!< Instruction 'fistp' {FPU}.
    kIdFisttp,                           //!< Instruction 'fisttp' {SSE3|FPU}.
    kIdFisub,                            //!< Instruction 'fisub' {FPU}.
    kIdFisubr,                           //!< Instruction 'fisubr' {FPU}.
    kIdFld,                              //!< Instruction 'fld' {FPU}.
    kIdFld1,                             //!< Instruction 'fld1' {FPU}.
    kIdFldcw,                            //!< Instruction 'fldcw' {FPU}.
    kIdFldenv,                           //!< Instruction 'fldenv' {FPU}.
    kIdFldl2e,                           //!< Instruction 'fldl2e' {FPU}.
    kIdFldl2t,                           //!< Instruction 'fldl2t' {FPU}.
    kIdFldlg2,                           //!< Instruction 'fldlg2' {FPU}.
    kIdFldln2,                           //!< Instruction 'fldln2' {FPU}.
    kIdFldpi,                            //!< Instruction 'fldpi' {FPU}.
    kIdFldz,                             //!< Instruction 'fldz' {FPU}.
    kIdFmul,                             //!< Instruction 'fmul' {FPU}.
    kIdFmulp,                            //!< Instruction 'fmulp' {FPU}.
    kIdFnclex,                           //!< Instruction 'fnclex' {FPU}.
    kIdFninit,                           //!< Instruction 'fninit' {FPU}.
    kIdFnop,                             //!< Instruction 'fnop' {FPU}.
    kIdFnsave,                           //!< Instruction 'fnsave' {FPU}.
    kIdFnstcw,                           //!< Instruction 'fnstcw' {FPU}.
    kIdFnstenv,                          //!< Instruction 'fnstenv' {FPU}.
    kIdFnstsw,                           //!< Instruction 'fnstsw' {FPU}.
    kIdFpatan,                           //!< Instruction 'fpatan' {FPU}.
    kIdFprem,                            //!< Instruction 'fprem' {FPU}.
    kIdFprem1,                           //!< Instruction 'fprem1' {FPU}.
    kIdFptan,                            //!< Instruction 'fptan' {FPU}.
    kIdFrndint,                          //!< Instruction 'frndint' {FPU}.
    kIdFrstor,                           //!< Instruction 'frstor' {FPU}.
    kIdFsave,                            //!< Instruction 'fsave' {FPU}.
    kIdFscale,                           //!< Instruction 'fscale' {FPU}.
    kIdFsin,                             //!< Instruction 'fsin' {FPU}.
    kIdFsincos,                          //!< Instruction 'fsincos' {FPU}.
    kIdFsqrt,                            //!< Instruction 'fsqrt' {FPU}.
    kIdFst,                              //!< Instruction 'fst' {FPU}.
    kIdFstcw,                            //!< Instruction 'fstcw' {FPU}.
    kIdFstenv,                           //!< Instruction 'fstenv' {FPU}.
    kIdFstp,                             //!< Instruction 'fstp' {FPU}.
    kIdFstsw,                            //!< Instruction 'fstsw' {FPU}.
    kIdFsub,                             //!< Instruction 'fsub' {FPU}.
    kIdFsubp,                            //!< Instruction 'fsubp' {FPU}.
    kIdFsubr,                            //!< Instruction 'fsubr' {FPU}.
    kIdFsubrp,                           //!< Instruction 'fsubrp' {FPU}.
    kIdFtst,                             //!< Instruction 'ftst' {FPU}.
    kIdFucom,                            //!< Instruction 'fucom' {FPU}.
    kIdFucomi,                           //!< Instruction 'fucomi' {FPU}.
    kIdFucomip,                          //!< Instruction 'fucomip' {FPU}.
    kIdFucomp,                           //!< Instruction 'fucomp' {FPU}.
    kIdFucompp,                          //!< Instruction 'fucompp' {FPU}.
    kIdFwait,                            //!< Instruction 'fwait' {FPU}.
    kIdFxam,                             //!< Instruction 'fxam' {FPU}.
    kIdFxch,                             //!< Instruction 'fxch' {FPU}.
    kIdFxrstor,                          //!< Instruction 'fxrstor' {FXSR}.
    kIdFxrstor64,                        //!< Instruction 'fxrstor64' {FXSR} (X64).
    kIdFxsave,                           //!< Instruction 'fxsave' {FXSR}.
    kIdFxsave64,                         //!< Instruction 'fxsave64' {FXSR} (X64).
    kIdFxtract,                          //!< Instruction 'fxtract' {FPU}.
    kIdFyl2x,                            //!< Instruction 'fyl2x' {FPU}.
    kIdFyl2xp1,                          //!< Instruction 'fyl2xp1' {FPU}.
    kIdGetsec,                           //!< Instruction 'getsec' {SMX}.
    kIdGf2p8affineinvqb,                 //!< Instruction 'gf2p8affineinvqb' {GFNI}.
    kIdGf2p8affineqb,                    //!< Instruction 'gf2p8affineqb' {GFNI}.
    kIdGf2p8mulb,                        //!< Instruction 'gf2p8mulb' {GFNI}.
    kIdHaddpd,                           //!< Instruction 'haddpd' {SSE3}.
    kIdHaddps,                           //!< Instruction 'haddps' {SSE3}.
    kIdHlt,                              //!< Instruction 'hlt'.
    kIdHreset,                           //!< Instruction 'hreset' {HRESET}.
    kIdHsubpd,                           //!< Instruction 'hsubpd' {SSE3}.
    kIdHsubps,                           //!< Instruction 'hsubps' {SSE3}.
    kIdIdiv,                             //!< Instruction 'idiv'.
    kIdImul,                             //!< Instruction 'imul'.
    kIdIn,                               //!< Instruction 'in'.
    kIdInc,                              //!< Instruction 'inc'.
    kIdIncsspd,                          //!< Instruction 'incsspd' {CET_SS}.
    kIdIncsspq,                          //!< Instruction 'incsspq' {CET_SS} (X64).
    kIdIns,                              //!< Instruction 'ins'.
    kIdInsertps,                         //!< Instruction 'insertps' {SSE4_1}.
    kIdInsertq,                          //!< Instruction 'insertq' {SSE4A}.
    kIdInt,                              //!< Instruction 'int'.
    kIdInt3,                             //!< Instruction 'int3'.
    kIdInto,                             //!< Instruction 'into' (X86).
    kIdInvd,                             //!< Instruction 'invd' {I486}.
    kIdInvept,                           //!< Instruction 'invept' {VMX}.
    kIdInvlpg,                           //!< Instruction 'invlpg' {I486}.
    kIdInvlpga,                          //!< Instruction 'invlpga' {SVM}.
    kIdInvlpgb,                          //!< Instruction 'invlpgb' {INVLPGB}.
    kIdInvpcid,                          //!< Instruction 'invpcid' {I486}.
    kIdInvvpid,                          //!< Instruction 'invvpid' {VMX}.
    kIdIret,                             //!< Instruction 'iret'.
    kIdIretd,                            //!< Instruction 'iretd'.
    kIdIretq,                            //!< Instruction 'iretq' (X64).
    kIdJb,                               //!< Instruction 'jb'.
    kIdJbe,                              //!< Instruction 'jbe'.
    kIdJecxz,                            //!< Instruction 'jecxz'.
    kIdJl,                               //!< Instruction 'jl'.
    kIdJle,                              //!< Instruction 'jle'.
    kIdJmp,                              //!< Instruction 'jmp'.
    kIdJnb,                              //!< Instruction 'jnb'.
    kIdJnbe,                             //!< Instruction 'jnbe'.
    kIdJnl,                              //!< Instruction 'jnl'.
    kIdJnle,                             //!< Instruction 'jnle'.
    kIdJno,                              //!< Instruction 'jno'.
    kIdJnp,                              //!< Instruction 'jnp'.
    kIdJns,                              //!< Instruction 'jns'.
    kIdJnz,                              //!< Instruction 'jnz'.
    kIdJo,                               //!< Instruction 'jo'.
    kIdJp,                               //!< Instruction 'jp'.
    kIdJs,                               //!< Instruction 'js'.
    kIdJz,                               //!< Instruction 'jz'.
    kIdKaddb,                            //!< Instruction 'kaddb' {AVX512_DQ}.
    kIdKaddd,                            //!< Instruction 'kaddd' {AVX512_BW}.
    kIdKaddq,                            //!< Instruction 'kaddq' {AVX512_BW}.
    kIdKaddw,                            //!< Instruction 'kaddw' {AVX512_DQ}.
    kIdKandb,                            //!< Instruction 'kandb' {AVX512_DQ}.
    kIdKandd,                            //!< Instruction 'kandd' {AVX512_BW}.
    kIdKandnb,                           //!< Instruction 'kandnb' {AVX512_DQ}.
    kIdKandnd,                           //!< Instruction 'kandnd' {AVX512_BW}.
    kIdKandnq,                           //!< Instruction 'kandnq' {AVX512_BW}.
    kIdKandnw,                           //!< Instruction 'kandnw' {AVX512_F}.
    kIdKandq,                            //!< Instruction 'kandq' {AVX512_BW}.
    kIdKandw,                            //!< Instruction 'kandw' {AVX512_F}.
    kIdKmovb,                            //!< Instruction 'kmovb' {AVX512_DQ}.
    kIdKmovd,                            //!< Instruction 'kmovd' {AVX512_BW}.
    kIdKmovq,                            //!< Instruction 'kmovq' {AVX512_BW}.
    kIdKmovw,                            //!< Instruction 'kmovw' {AVX512_F}.
    kIdKnotb,                            //!< Instruction 'knotb' {AVX512_DQ}.
    kIdKnotd,                            //!< Instruction 'knotd' {AVX512_BW}.
    kIdKnotq,                            //!< Instruction 'knotq' {AVX512_BW}.
    kIdKnotw,                            //!< Instruction 'knotw' {AVX512_F}.
    kIdKorb,                             //!< Instruction 'korb' {AVX512_DQ}.
    kIdKord,                             //!< Instruction 'kord' {AVX512_BW}.
    kIdKorq,                             //!< Instruction 'korq' {AVX512_BW}.
    kIdKortestb,                         //!< Instruction 'kortestb' {AVX512_DQ}.
    kIdKortestd,                         //!< Instruction 'kortestd' {AVX512_BW}.
    kIdKortestq,                         //!< Instruction 'kortestq' {AVX512_BW}.
    kIdKortestw,                         //!< Instruction 'kortestw' {AVX512_F}.
    kIdKorw,                             //!< Instruction 'korw' {AVX512_F}.
    kIdKshiftlb,                         //!< Instruction 'kshiftlb' {AVX512_DQ}.
    kIdKshiftld,                         //!< Instruction 'kshiftld' {AVX512_BW}.
    kIdKshiftlq,                         //!< Instruction 'kshiftlq' {AVX512_BW}.
    kIdKshiftlw,                         //!< Instruction 'kshiftlw' {AVX512_F}.
    kIdKshiftrb,                         //!< Instruction 'kshiftrb' {AVX512_DQ}.
    kIdKshiftrd,                         //!< Instruction 'kshiftrd' {AVX512_BW}.
    kIdKshiftrq,                         //!< Instruction 'kshiftrq' {AVX512_BW}.
    kIdKshiftrw,                         //!< Instruction 'kshiftrw' {AVX512_F}.
    kIdKtestb,                           //!< Instruction 'ktestb' {AVX512_DQ}.
    kIdKtestd,                           //!< Instruction 'ktestd' {AVX512_BW}.
    kIdKtestq,                           //!< Instruction 'ktestq' {AVX512_BW}.
    kIdKtestw,                           //!< Instruction 'ktestw' {AVX512_DQ}.
    kIdKunpckbw,                         //!< Instruction 'kunpckbw' {AVX512_F}.
    kIdKunpckdq,                         //!< Instruction 'kunpckdq' {AVX512_BW}.
    kIdKunpckwd,                         //!< Instruction 'kunpckwd' {AVX512_BW}.
    kIdKxnorb,                           //!< Instruction 'kxnorb' {AVX512_DQ}.
    kIdKxnord,                           //!< Instruction 'kxnord' {AVX512_BW}.
    kIdKxnorq,                           //!< Instruction 'kxnorq' {AVX512_BW}.
    kIdKxnorw,                           //!< Instruction 'kxnorw' {AVX512_F}.
    kIdKxorb,                            //!< Instruction 'kxorb' {AVX512_DQ}.
    kIdKxord,                            //!< Instruction 'kxord' {AVX512_BW}.
    kIdKxorq,                            //!< Instruction 'kxorq' {AVX512_BW}.
    kIdKxorw,                            //!< Instruction 'kxorw' {AVX512_F}.
    kIdLahf,                             //!< Instruction 'lahf' {LAHFSAHF}.
    kIdLar,                              //!< Instruction 'lar'.
    kIdLcall,                            //!< Instruction 'lcall'.
    kIdLddqu,                            //!< Instruction 'lddqu' {SSE3}.
    kIdLdmxcsr,                          //!< Instruction 'ldmxcsr' {SSE}.
    kIdLds,                              //!< Instruction 'lds' (X86).
    kIdLdtilecfg,                        //!< Instruction 'ldtilecfg' {AMX_TILE} (X64).
    kIdLea,                              //!< Instruction 'lea'.
    kIdLeave,                            //!< Instruction 'leave'.
    kIdLes,                              //!< Instruction 'les' (X86).
    kIdLfence,                           //!< Instruction 'lfence' {SSE2}.
    kIdLfs,                              //!< Instruction 'lfs'.
    kIdLgdt,                             //!< Instruction 'lgdt'.
    kIdLgs,                              //!< Instruction 'lgs'.
    kIdLidt,                             //!< Instruction 'lidt'.
    kIdLjmp,                             //!< Instruction 'ljmp'.
    kIdLldt,                             //!< Instruction 'lldt'.
    kIdLlwpcb,                           //!< Instruction 'llwpcb' {LWP}.
    kIdLmsw,                             //!< Instruction 'lmsw'.
    kIdLods,                             //!< Instruction 'lods'.
    kIdLoop,                             //!< Instruction 'loop'.
    kIdLoope,                            //!< Instruction 'loope'.
    kIdLoopne,                           //!< Instruction 'loopne'.
    kIdLsl,                              //!< Instruction 'lsl'.
    kIdLss,                              //!< Instruction 'lss'.
    kIdLtr,                              //!< Instruction 'ltr'.
    kIdLwpins,                           //!< Instruction 'lwpins' {LWP}.
    kIdLwpval,                           //!< Instruction 'lwpval' {LWP}.
    kIdLzcnt,                            //!< Instruction 'lzcnt' {LZCNT}.
    kIdMaskmovdqu,                       //!< Instruction 'maskmovdqu' {SSE2}.
    kIdMaskmovq,                         //!< Instruction 'maskmovq' {MMX2}.
    kIdMaxpd,                            //!< Instruction 'maxpd' {SSE2}.
    kIdMaxps,                            //!< Instruction 'maxps' {SSE}.
    kIdMaxsd,                            //!< Instruction 'maxsd' {SSE2}.
    kIdMaxss,                            //!< Instruction 'maxss' {SSE}.
    kIdMcommit,                          //!< Instruction 'mcommit' {MCOMMIT}.
    kIdMfence,                           //!< Instruction 'mfence' {SSE2}.
    kIdMinpd,                            //!< Instruction 'minpd' {SSE2}.
    kIdMinps,                            //!< Instruction 'minps' {SSE}.
    kIdMinsd,                            //!< Instruction 'minsd' {SSE2}.
    kIdMinss,                            //!< Instruction 'minss' {SSE}.
    kIdMonitor,                          //!< Instruction 'monitor' {MONITOR}.
    kIdMonitorx,                         //!< Instruction 'monitorx' {MONITORX}.
    kIdMov,                              //!< Instruction 'mov'.
    kIdMovabs,                           //!< Instruction 'movabs'.
    kIdMovapd,                           //!< Instruction 'movapd' {SSE2}.
    kIdMovaps,                           //!< Instruction 'movaps' {SSE}.
    kIdMovbe,                            //!< Instruction 'movbe' {MOVBE}.
    kIdMovd,                             //!< Instruction 'movd' {MMX|SSE2}.
    kIdMovddup,                          //!< Instruction 'movddup' {SSE3}.
    kIdMovdir64b,                        //!< Instruction 'movdir64b' {MOVDIR64B}.
    kIdMovdiri,                          //!< Instruction 'movdiri' {MOVDIRI}.
    kIdMovdq2q,                          //!< Instruction 'movdq2q' {SSE2}.
    kIdMovdqa,                           //!< Instruction 'movdqa' {SSE2}.
    kIdMovdqu,                           //!< Instruction 'movdqu' {SSE2}.
    kIdMovhlps,                          //!< Instruction 'movhlps' {SSE}.
    kIdMovhpd,                           //!< Instruction 'movhpd' {SSE2}.
    kIdMovhps,                           //!< Instruction 'movhps' {SSE}.
    kIdMovlhps,                          //!< Instruction 'movlhps' {SSE}.
    kIdMovlpd,                           //!< Instruction 'movlpd' {SSE2}.
    kIdMovlps,                           //!< Instruction 'movlps' {SSE}.
    kIdMovmskpd,                         //!< Instruction 'movmskpd' {SSE2}.
    kIdMovmskps,                         //!< Instruction 'movmskps' {SSE}.
    kIdMovntdq,                          //!< Instruction 'movntdq' {SSE2}.
    kIdMovntdqa,                         //!< Instruction 'movntdqa' {SSE4_1}.
    kIdMovnti,                           //!< Instruction 'movnti' {SSE2}.
    kIdMovntpd,                          //!< Instruction 'movntpd' {SSE2}.
    kIdMovntps,                          //!< Instruction 'movntps' {SSE}.
    kIdMovntq,                           //!< Instruction 'movntq' {MMX2}.
    kIdMovntsd,                          //!< Instruction 'movntsd' {SSE4A}.
    kIdMovntss,                          //!< Instruction 'movntss' {SSE4A}.
    kIdMovq,                             //!< Instruction 'movq' {MMX|SSE2}.
    kIdMovq2dq,                          //!< Instruction 'movq2dq' {SSE2}.
    kIdMovs,                             //!< Instruction 'movs'.
    kIdMovsd,                            //!< Instruction 'movsd' {SSE2}.
    kIdMovshdup,                         //!< Instruction 'movshdup' {SSE3}.
    kIdMovsldup,                         //!< Instruction 'movsldup' {SSE3}.
    kIdMovss,                            //!< Instruction 'movss' {SSE}.
    kIdMovsx,                            //!< Instruction 'movsx'.
    kIdMovsxd,                           //!< Instruction 'movsxd' (X64).
    kIdMovupd,                           //!< Instruction 'movupd' {SSE2}.
    kIdMovups,                           //!< Instruction 'movups' {SSE}.
    kIdMovzx,                            //!< Instruction 'movzx'.
    kIdMpsadbw,                          //!< Instruction 'mpsadbw' {SSE4_1}.
    kIdMul,                              //!< Instruction 'mul'.
    kIdMulpd,                            //!< Instruction 'mulpd' {SSE2}.
    kIdMulps,                            //!< Instruction 'mulps' {SSE}.
    kIdMulsd,                            //!< Instruction 'mulsd' {SSE2}.
    kIdMulss,                            //!< Instruction 'mulss' {SSE}.
    kIdMulx,                             //!< Instruction 'mulx' {BMI2}.
    kIdMwait,                            //!< Instruction 'mwait' {MONITOR}.
    kIdMwaitx,                           //!< Instruction 'mwaitx' {MONITORX}.
    kIdNeg,                              //!< Instruction 'neg'.
    kIdNop,                              //!< Instruction 'nop'.
    kIdNot,                              //!< Instruction 'not'.
    kIdOr,                               //!< Instruction 'or'.
    kIdOrpd,                             //!< Instruction 'orpd' {SSE2}.
    kIdOrps,                             //!< Instruction 'orps' {SSE}.
    kIdOut,                              //!< Instruction 'out'.
    kIdOuts,                             //!< Instruction 'outs'.
    kIdPabsb,                            //!< Instruction 'pabsb' {SSSE3}.
    kIdPabsd,                            //!< Instruction 'pabsd' {SSSE3}.
    kIdPabsw,                            //!< Instruction 'pabsw' {SSSE3}.
    kIdPackssdw,                         //!< Instruction 'packssdw' {MMX|SSE2}.
    kIdPacksswb,                         //!< Instruction 'packsswb' {MMX|SSE2}.
    kIdPackusdw,                         //!< Instruction 'packusdw' {SSE4_1}.
    kIdPackuswb,                         //!< Instruction 'packuswb' {MMX|SSE2}.
    kIdPaddb,                            //!< Instruction 'paddb' {MMX|SSE2}.
    kIdPaddd,                            //!< Instruction 'paddd' {MMX|SSE2}.
    kIdPaddq,                            //!< Instruction 'paddq' {SSE2}.
    kIdPaddsb,                           //!< Instruction 'paddsb' {MMX|SSE2}.
    kIdPaddsw,                           //!< Instruction 'paddsw' {MMX|SSE2}.
    kIdPaddusb,                          //!< Instruction 'paddusb' {MMX|SSE2}.
    kIdPaddusw,                          //!< Instruction 'paddusw' {MMX|SSE2}.
    kIdPaddw,                            //!< Instruction 'paddw' {MMX|SSE2}.
    kIdPalignr,                          //!< Instruction 'palignr' {SSSE3}.
    kIdPand,                             //!< Instruction 'pand' {MMX|SSE2}.
    kIdPandn,                            //!< Instruction 'pandn' {MMX|SSE2}.
    kIdPause,                            //!< Instruction 'pause'.
    kIdPavgb,                            //!< Instruction 'pavgb' {MMX2|SSE2}.
    kIdPavgusb,                          //!< Instruction 'pavgusb' {3DNOW}.
    kIdPavgw,                            //!< Instruction 'pavgw' {MMX2|SSE2}.
    kIdPblendvb,                         //!< Instruction 'pblendvb' {SSE4_1}.
    kIdPblendw,                          //!< Instruction 'pblendw' {SSE4_1}.
    kIdPclmulqdq,                        //!< Instruction 'pclmulqdq' {PCLMULQDQ}.
    kIdPcmpeqb,                          //!< Instruction 'pcmpeqb' {MMX|SSE2}.
    kIdPcmpeqd,                          //!< Instruction 'pcmpeqd' {MMX|SSE2}.
    kIdPcmpeqq,                          //!< Instruction 'pcmpeqq' {SSE4_1}.
    kIdPcmpeqw,                          //!< Instruction 'pcmpeqw' {MMX|SSE2}.
    kIdPcmpestri,                        //!< Instruction 'pcmpestri' {SSE4_2}.
    kIdPcmpestrm,                        //!< Instruction 'pcmpestrm' {SSE4_2}.
    kIdPcmpgtb,                          //!< Instruction 'pcmpgtb' {MMX|SSE2}.
    kIdPcmpgtd,                          //!< Instruction 'pcmpgtd' {MMX|SSE2}.
    kIdPcmpgtq,                          //!< Instruction 'pcmpgtq' {SSE4_2}.
    kIdPcmpgtw,                          //!< Instruction 'pcmpgtw' {MMX|SSE2}.
    kIdPcmpistri,                        //!< Instruction 'pcmpistri' {SSE4_2}.
    kIdPcmpistrm,                        //!< Instruction 'pcmpistrm' {SSE4_2}.
    kIdPconfig,                          //!< Instruction 'pconfig' {PCONFIG}.
    kIdPdep,                             //!< Instruction 'pdep' {BMI2}.
    kIdPext,                             //!< Instruction 'pext' {BMI2}.
    kIdPextrb,                           //!< Instruction 'pextrb' {SSE4_1}.
    kIdPextrd,                           //!< Instruction 'pextrd' {SSE4_1}.
    kIdPextrq,                           //!< Instruction 'pextrq' {SSE4_1} (X64).
    kIdPextrw,                           //!< Instruction 'pextrw' {MMX2|SSE2|SSE4_1}.
    kIdPf2id,                            //!< Instruction 'pf2id' {3DNOW}.
    kIdPf2iw,                            //!< Instruction 'pf2iw' {3DNOW2}.
    kIdPfacc,                            //!< Instruction 'pfacc' {3DNOW}.
    kIdPfadd,                            //!< Instruction 'pfadd' {3DNOW}.
    kIdPfcmpeq,                          //!< Instruction 'pfcmpeq' {3DNOW}.
    kIdPfcmpge,                          //!< Instruction 'pfcmpge' {3DNOW}.
    kIdPfcmpgt,                          //!< Instruction 'pfcmpgt' {3DNOW}.
    kIdPfmax,                            //!< Instruction 'pfmax' {3DNOW}.
    kIdPfmin,                            //!< Instruction 'pfmin' {3DNOW}.
    kIdPfmul,                            //!< Instruction 'pfmul' {3DNOW}.
    kIdPfnacc,                           //!< Instruction 'pfnacc' {3DNOW2}.
    kIdPfpnacc,                          //!< Instruction 'pfpnacc' {3DNOW2}.
    kIdPfrcp,                            //!< Instruction 'pfrcp' {3DNOW}.
    kIdPfrcpit1,                         //!< Instruction 'pfrcpit1' {3DNOW}.
    kIdPfrcpit2,                         //!< Instruction 'pfrcpit2' {3DNOW}.
    kIdPfrcpv,                           //!< Instruction 'pfrcpv' {GEODE}.
    kIdPfrsqit1,                         //!< Instruction 'pfrsqit1' {3DNOW}.
    kIdPfrsqrt,                          //!< Instruction 'pfrsqrt' {3DNOW}.
    kIdPfrsqrtv,                         //!< Instruction 'pfrsqrtv' {GEODE}.
    kIdPfsub,                            //!< Instruction 'pfsub' {3DNOW}.
    kIdPfsubr,                           //!< Instruction 'pfsubr' {3DNOW}.
    kIdPhaddd,                           //!< Instruction 'phaddd' {SSSE3}.
    kIdPhaddsw,                          //!< Instruction 'phaddsw' {SSSE3}.
    kIdPhaddw,                           //!< Instruction 'phaddw' {SSSE3}.
    kIdPhminposuw,                       //!< Instruction 'phminposuw' {SSE4_1}.
    kIdPhsubd,                           //!< Instruction 'phsubd' {SSSE3}.
    kIdPhsubsw,                          //!< Instruction 'phsubsw' {SSSE3}.
    kIdPhsubw,                           //!< Instruction 'phsubw' {SSSE3}.
    kIdPi2fd,                            //!< Instruction 'pi2fd' {3DNOW}.
    kIdPi2fw,                            //!< Instruction 'pi2fw' {3DNOW2}.
    kIdPinsrb,                           //!< Instruction 'pinsrb' {SSE4_1}.
    kIdPinsrd,                           //!< Instruction 'pinsrd' {SSE4_1}.
    kIdPinsrq,                           //!< Instruction 'pinsrq' {SSE4_1} (X64).
    kIdPinsrw,                           //!< Instruction 'pinsrw' {MMX2|SSE2}.
    kIdPmaddubsw,                        //!< Instruction 'pmaddubsw' {SSSE3}.
    kIdPmaddwd,                          //!< Instruction 'pmaddwd' {MMX|SSE2}.
    kIdPmaxsb,                           //!< Instruction 'pmaxsb' {SSE4_1}.
    kIdPmaxsd,                           //!< Instruction 'pmaxsd' {SSE4_1}.
    kIdPmaxsw,                           //!< Instruction 'pmaxsw' {MMX2|SSE2}.
    kIdPmaxub,                           //!< Instruction 'pmaxub' {MMX2|SSE2}.
    kIdPmaxud,                           //!< Instruction 'pmaxud' {SSE4_1}.
    kIdPmaxuw,                           //!< Instruction 'pmaxuw' {SSE4_1}.
    kIdPminsb,                           //!< Instruction 'pminsb' {SSE4_1}.
    kIdPminsd,                           //!< Instruction 'pminsd' {SSE4_1}.
    kIdPminsw,                           //!< Instruction 'pminsw' {MMX2|SSE2}.
    kIdPminub,                           //!< Instruction 'pminub' {MMX2|SSE2}.
    kIdPminud,                           //!< Instruction 'pminud' {SSE4_1}.
    kIdPminuw,                           //!< Instruction 'pminuw' {SSE4_1}.
    kIdPmovmskb,                         //!< Instruction 'pmovmskb' {MMX2|SSE2}.
    kIdPmovsxbd,                         //!< Instruction 'pmovsxbd' {SSE4_1}.
    kIdPmovsxbq,                         //!< Instruction 'pmovsxbq' {SSE4_1}.
    kIdPmovsxbw,                         //!< Instruction 'pmovsxbw' {SSE4_1}.
    kIdPmovsxdq,                         //!< Instruction 'pmovsxdq' {SSE4_1}.
    kIdPmovsxwd,                         //!< Instruction 'pmovsxwd' {SSE4_1}.
    kIdPmovsxwq,                         //!< Instruction 'pmovsxwq' {SSE4_1}.
    kIdPmovzxbd,                         //!< Instruction 'pmovzxbd' {SSE4_1}.
    kIdPmovzxbq,                         //!< Instruction 'pmovzxbq' {SSE4_1}.
    kIdPmovzxbw,                         //!< Instruction 'pmovzxbw' {SSE4_1}.
    kIdPmovzxdq,                         //!< Instruction 'pmovzxdq' {SSE4_1}.
    kIdPmovzxwd,                         //!< Instruction 'pmovzxwd' {SSE4_1}.
    kIdPmovzxwq,                         //!< Instruction 'pmovzxwq' {SSE4_1}.
    kIdPmuldq,                           //!< Instruction 'pmuldq' {SSE4_1}.
    kIdPmulhrsw,                         //!< Instruction 'pmulhrsw' {SSSE3}.
    kIdPmulhrw,                          //!< Instruction 'pmulhrw' {3DNOW}.
    kIdPmulhuw,                          //!< Instruction 'pmulhuw' {MMX2|SSE2}.
    kIdPmulhw,                           //!< Instruction 'pmulhw' {MMX|SSE2}.
    kIdPmulld,                           //!< Instruction 'pmulld' {SSE4_1}.
    kIdPmullw,                           //!< Instruction 'pmullw' {MMX|SSE2}.
    kIdPmuludq,                          //!< Instruction 'pmuludq' {SSE2}.
    kIdPop,                              //!< Instruction 'pop'.
    kIdPopa,                             //!< Instruction 'popa' (X86).
    kIdPopad,                            //!< Instruction 'popad' (X86).
    kIdPopcnt,                           //!< Instruction 'popcnt' {POPCNT}.
    kIdPopf,                             //!< Instruction 'popf'.
    kIdPopfd,                            //!< Instruction 'popfd' (X86).
    kIdPopfq,                            //!< Instruction 'popfq' (X64).
    kIdPor,                              //!< Instruction 'por' {MMX|SSE2}.
    kIdPrefetch,                         //!< Instruction 'prefetch' {3DNOW}.
    kIdPrefetchit0,                      //!< Instruction 'prefetchit0' {PREFETCHI} (X64).
    kIdPrefetchit1,                      //!< Instruction 'prefetchit1' {PREFETCHI} (X64).
    kIdPrefetchnta,                      //!< Instruction 'prefetchnta' {SSE}.
    kIdPrefetcht0,                       //!< Instruction 'prefetcht0' {SSE}.
    kIdPrefetcht1,                       //!< Instruction 'prefetcht1' {SSE}.
    kIdPrefetcht2,                       //!< Instruction 'prefetcht2' {SSE}.
    kIdPrefetchw,                        //!< Instruction 'prefetchw' {PREFETCHW}.
    kIdPrefetchwt1,                      //!< Instruction 'prefetchwt1' {PREFETCHWT1}.
    kIdPsadbw,                           //!< Instruction 'psadbw' {MMX2|SSE2}.
    kIdPshufb,                           //!< Instruction 'pshufb' {SSSE3}.
    kIdPshufd,                           //!< Instruction 'pshufd' {SSE2}.
    kIdPshufhw,                          //!< Instruction 'pshufhw' {SSE2}.
    kIdPshuflw,                          //!< Instruction 'pshuflw' {SSE2}.
    kIdPshufw,                           //!< Instruction 'pshufw' {MMX2}.
    kIdPsignb,                           //!< Instruction 'psignb' {SSSE3}.
    kIdPsignd,                           //!< Instruction 'psignd' {SSSE3}.
    kIdPsignw,                           //!< Instruction 'psignw' {SSSE3}.
    kIdPslld,                            //!< Instruction 'pslld' {MMX|SSE2}.
    kIdPslldq,                           //!< Instruction 'pslldq' {SSE2}.
    kIdPsllq,                            //!< Instruction 'psllq' {MMX|SSE2}.
    kIdPsllw,                            //!< Instruction 'psllw' {MMX|SSE2}.
    kIdPsmash,                           //!< Instruction 'psmash' {SEV_SNP} (X64).
    kIdPsrad,                            //!< Instruction 'psrad' {MMX|SSE2}.
    kIdPsraw,                            //!< Instruction 'psraw' {MMX|SSE2}.
    kIdPsrld,                            //!< Instruction 'psrld' {MMX|SSE2}.
    kIdPsrldq,                           //!< Instruction 'psrldq' {SSE2}.
    kIdPsrlq,                            //!< Instruction 'psrlq' {MMX|SSE2}.
    kIdPsrlw,                            //!< Instruction 'psrlw' {MMX|SSE2}.
    kIdPsubb,                            //!< Instruction 'psubb' {MMX|SSE2}.
    kIdPsubd,                            //!< Instruction 'psubd' {MMX|SSE2}.
    kIdPsubq,                            //!< Instruction 'psubq' {SSE2}.
    kIdPsubsb,                           //!< Instruction 'psubsb' {MMX|SSE2}.
    kIdPsubsw,                           //!< Instruction 'psubsw' {MMX|SSE2}.
    kIdPsubusb,                          //!< Instruction 'psubusb' {MMX|SSE2}.
    kIdPsubusw,                          //!< Instruction 'psubusw' {MMX|SSE2}.
    kIdPsubw,                            //!< Instruction 'psubw' {MMX|SSE2}.
    kIdPswapd,                           //!< Instruction 'pswapd' {3DNOW2}.
    kIdPtest,                            //!< Instruction 'ptest' {SSE4_1}.
    kIdPtwrite,                          //!< Instruction 'ptwrite' {PTWRITE}.
    kIdPunpckhbw,                        //!< Instruction 'punpckhbw' {MMX|SSE2}.
    kIdPunpckhdq,                        //!< Instruction 'punpckhdq' {MMX|SSE2}.
    kIdPunpckhqdq,                       //!< Instruction 'punpckhqdq' {SSE2}.
    kIdPunpckhwd,                        //!< Instruction 'punpckhwd' {MMX|SSE2}.
    kIdPunpcklbw,                        //!< Instruction 'punpcklbw' {MMX|SSE2}.
    kIdPunpckldq,                        //!< Instruction 'punpckldq' {MMX|SSE2}.
    kIdPunpcklqdq,                       //!< Instruction 'punpcklqdq' {SSE2}.
    kIdPunpcklwd,                        //!< Instruction 'punpcklwd' {MMX|SSE2}.
    kIdPush,                             //!< Instruction 'push'.
    kIdPusha,                            //!< Instruction 'pusha' (X86).
    kIdPushad,                           //!< Instruction 'pushad' (X86).
    kIdPushf,                            //!< Instruction 'pushf'.
    kIdPushfd,                           //!< Instruction 'pushfd' (X86).
    kIdPushfq,                           //!< Instruction 'pushfq' (X64).
    kIdPushw,                            //!< Instruction 'pushw'.
    kIdPvalidate,                        //!< Instruction 'pvalidate' {SEV_SNP}.
    kIdPxor,                             //!< Instruction 'pxor' {MMX|SSE2}.
    kIdRcl,                              //!< Instruction 'rcl'.
    kIdRcpps,                            //!< Instruction 'rcpps' {SSE}.
    kIdRcpss,                            //!< Instruction 'rcpss' {SSE}.
    kIdRcr,                              //!< Instruction 'rcr'.
    kIdRdfsbase,                         //!< Instruction 'rdfsbase' {FSGSBASE} (X64).
    kIdRdgsbase,                         //!< Instruction 'rdgsbase' {FSGSBASE} (X64).
    kIdRdmsr,                            //!< Instruction 'rdmsr' {MSR|MSR_IMM}.
    kIdRdpid,                            //!< Instruction 'rdpid' {RDPID}.
    kIdRdpkru,                           //!< Instruction 'rdpkru' {OSPKE}.
    kIdRdpmc,                            //!< Instruction 'rdpmc'.
    kIdRdpru,                            //!< Instruction 'rdpru' {RDPRU}.
    kIdRdrand,                           //!< Instruction 'rdrand' {RDRAND}.
    kIdRdseed,                           //!< Instruction 'rdseed' {RDSEED}.
    kIdRdsspd,                           //!< Instruction 'rdsspd' {CET_SS}.
    kIdRdsspq,                           //!< Instruction 'rdsspq' {CET_SS} (X64).
    kIdRdtsc,                            //!< Instruction 'rdtsc' {RDTSC}.
    kIdRdtscp,                           //!< Instruction 'rdtscp' {RDTSCP}.
    kIdRet,                              //!< Instruction 'ret'.
    kIdRetf,                             //!< Instruction 'retf'.
    kIdRmpadjust,                        //!< Instruction 'rmpadjust' {SEV_SNP} (X64).
    kIdRmpupdate,                        //!< Instruction 'rmpupdate' {SEV_SNP} (X64).
    kIdRol,                              //!< Instruction 'rol'.
    kIdRor,                              //!< Instruction 'ror'.
    kIdRorx,                             //!< Instruction 'rorx' {BMI2}.
    kIdRoundpd,                          //!< Instruction 'roundpd' {SSE4_1}.
    kIdRoundps,                          //!< Instruction 'roundps' {SSE4_1}.
    kIdRoundsd,                          //!< Instruction 'roundsd' {SSE4_1}.
    kIdRoundss,                          //!< Instruction 'roundss' {SSE4_1}.
    kIdRsm,                              //!< Instruction 'rsm' (X86).
    kIdRsqrtps,                          //!< Instruction 'rsqrtps' {SSE}.
    kIdRsqrtss,                          //!< Instruction 'rsqrtss' {SSE}.
    kIdRstorssp,                         //!< Instruction 'rstorssp' {CET_SS}.
    kIdSahf,                             //!< Instruction 'sahf' {LAHFSAHF}.
    kIdSar,                              //!< Instruction 'sar'.
    kIdSarx,                             //!< Instruction 'sarx' {BMI2}.
    kIdSaveprevssp,                      //!< Instruction 'saveprevssp' {CET_SS}.
    kIdSbb,                              //!< Instruction 'sbb'.
    kIdScas,                             //!< Instruction 'scas'.
    kIdSeamcall,                         //!< Instruction 'seamcall' {SEAM}.
    kIdSeamops,                          //!< Instruction 'seamops' {SEAM}.
    kIdSeamret,                          //!< Instruction 'seamret' {SEAM}.
    kIdSenduipi,                         //!< Instruction 'senduipi' {UINTR} (X64).
    kIdSerialize,                        //!< Instruction 'serialize' {SERIALIZE}.
    kIdSetb,                             //!< Instruction 'setb'.
    kIdSetbe,                            //!< Instruction 'setbe'.
    kIdSetl,                             //!< Instruction 'setl'.
    kIdSetle,                            //!< Instruction 'setle'.
    kIdSetnb,                            //!< Instruction 'setnb'.
    kIdSetnbe,                           //!< Instruction 'setnbe'.
    kIdSetnl,                            //!< Instruction 'setnl'.
    kIdSetnle,                           //!< Instruction 'setnle'.
    kIdSetno,                            //!< Instruction 'setno'.
    kIdSetnp,                            //!< Instruction 'setnp'.
    kIdSetns,                            //!< Instruction 'setns'.
    kIdSetnz,                            //!< Instruction 'setnz'.
    kIdSeto,                             //!< Instruction 'seto'.
    kIdSetp,                             //!< Instruction 'setp'.
    kIdSets,                             //!< Instruction 'sets'.
    kIdSetssbsy,                         //!< Instruction 'setssbsy' {CET_SS}.
    kIdSetz,                             //!< Instruction 'setz'.
    kIdSfence,                           //!< Instruction 'sfence' {SSE}.
    kIdSgdt,                             //!< Instruction 'sgdt'.
    kIdSha1msg1,                         //!< Instruction 'sha1msg1' {SHA}.
    kIdSha1msg2,                         //!< Instruction 'sha1msg2' {SHA}.
    kIdSha1nexte,                        //!< Instruction 'sha1nexte' {SHA}.
    kIdSha1rnds4,                        //!< Instruction 'sha1rnds4' {SHA}.
    kIdSha256msg1,                       //!< Instruction 'sha256msg1' {SHA}.
    kIdSha256msg2,                       //!< Instruction 'sha256msg2' {SHA}.
    kIdSha256rnds2,                      //!< Instruction 'sha256rnds2' {SHA}.
    kIdShl,                              //!< Instruction 'shl'.
    kIdShld,                             //!< Instruction 'shld'.
    kIdShlx,                             //!< Instruction 'shlx' {BMI2}.
    kIdShr,                              //!< Instruction 'shr'.
    kIdShrd,                             //!< Instruction 'shrd'.
    kIdShrx,                             //!< Instruction 'shrx' {BMI2}.
    kIdShufpd,                           //!< Instruction 'shufpd' {SSE2}.
    kIdShufps,                           //!< Instruction 'shufps' {SSE}.
    kIdSidt,                             //!< Instruction 'sidt'.
    kIdSkinit,                           //!< Instruction 'skinit' {SKINIT}.
    kIdSldt,                             //!< Instruction 'sldt'.
    kIdSlwpcb,                           //!< Instruction 'slwpcb' {LWP}.
    kIdSmsw,                             //!< Instruction 'smsw'.
    kIdSqrtpd,                           //!< Instruction 'sqrtpd' {SSE2}.
    kIdSqrtps,                           //!< Instruction 'sqrtps' {SSE}.
    kIdSqrtsd,                           //!< Instruction 'sqrtsd' {SSE2}.
    kIdSqrtss,                           //!< Instruction 'sqrtss' {SSE}.
    kIdStac,                             //!< Instruction 'stac' {SMAP}.
    kIdStc,                              //!< Instruction 'stc'.
    kIdStd,                              //!< Instruction 'std'.
    kIdStgi,                             //!< Instruction 'stgi' {SKINIT}.
    kIdSti,                              //!< Instruction 'sti'.
    kIdStmxcsr,                          //!< Instruction 'stmxcsr' {SSE}.
    kIdStos,                             //!< Instruction 'stos'.
    kIdStr,                              //!< Instruction 'str'.
    kIdSttilecfg,                        //!< Instruction 'sttilecfg' {AMX_TILE} (X64).
    kIdStui,                             //!< Instruction 'stui' {UINTR} (X64).
    kIdSub,                              //!< Instruction 'sub'.
    kIdSubpd,                            //!< Instruction 'subpd' {SSE2}.
    kIdSubps,                            //!< Instruction 'subps' {SSE}.
    kIdSubsd,                            //!< Instruction 'subsd' {SSE2}.
    kIdSubss,                            //!< Instruction 'subss' {SSE}.
    kIdSwapgs,                           //!< Instruction 'swapgs' (X64).
    kIdSyscall,                          //!< Instruction 'syscall' (X64).
    kIdSysenter,                         //!< Instruction 'sysenter'.
    kIdSysexit,                          //!< Instruction 'sysexit'.
    kIdSysexitq,                         //!< Instruction 'sysexitq' (X64).
    kIdSysret,                           //!< Instruction 'sysret' (X64).
    kIdSysretq,                          //!< Instruction 'sysretq' (X64).
    kIdT1mskc,                           //!< Instruction 't1mskc' {TBM}.
    kIdTcmmimfp16ps,                     //!< Instruction 'tcmmimfp16ps' {AMX_COMPLEX} (X64).
    kIdTcmmrlfp16ps,                     //!< Instruction 'tcmmrlfp16ps' {AMX_COMPLEX} (X64).
    kIdTdcall,                           //!< Instruction 'tdcall' {SEAM}.
    kIdTdpbf16ps,                        //!< Instruction 'tdpbf16ps' {AMX_BF16} (X64).
    kIdTdpbssd,                          //!< Instruction 'tdpbssd' {AMX_INT8} (X64).
    kIdTdpbsud,                          //!< Instruction 'tdpbsud' {AMX_INT8} (X64).
    kIdTdpbusd,                          //!< Instruction 'tdpbusd' {AMX_INT8} (X64).
    kIdTdpbuud,                          //!< Instruction 'tdpbuud' {AMX_INT8} (X64).
    kIdTdpfp16ps,                        //!< Instruction 'tdpfp16ps' {AMX_FP16} (X64).
    kIdTest,                             //!< Instruction 'test'.
    kIdTestui,                           //!< Instruction 'testui' {UINTR} (X64).
    kIdTileloadd,                        //!< Instruction 'tileloadd' {AMX_TILE} (X64).
    kIdTileloaddt1,                      //!< Instruction 'tileloaddt1' {AMX_TILE} (X64).
    kIdTilerelease,                      //!< Instruction 'tilerelease' {AMX_TILE} (X64).
    kIdTilestored,                       //!< Instruction 'tilestored' {AMX_TILE} (X64).
    kIdTilezero,                         //!< Instruction 'tilezero' {AMX_TILE} (X64).
    kIdTlbsync,                          //!< Instruction 'tlbsync' {INVLPGB}.
    kIdTpause,                           //!< Instruction 'tpause' {WAITPKG}.
    kIdTzcnt,                            //!< Instruction 'tzcnt' {BMI}.
    kIdTzmsk,                            //!< Instruction 'tzmsk' {TBM}.
    kIdUcomisd,                          //!< Instruction 'ucomisd' {SSE2}.
    kIdUcomiss,                          //!< Instruction 'ucomiss' {SSE}.
    kIdUd0,                              //!< Instruction 'ud0'.
    kIdUd1,                              //!< Instruction 'ud1'.
    kIdUd2,                              //!< Instruction 'ud2'.
    kIdUiret,                            //!< Instruction 'uiret' {UINTR} (X64).
    kIdUmonitor,                         //!< Instruction 'umonitor' {WAITPKG}.
    kIdUmwait,                           //!< Instruction 'umwait' {WAITPKG}.
    kIdUnpckhpd,                         //!< Instruction 'unpckhpd' {SSE2}.
    kIdUnpckhps,                         //!< Instruction 'unpckhps' {SSE}.
    kIdUnpcklpd,                         //!< Instruction 'unpcklpd' {SSE2}.
    kIdUnpcklps,                         //!< Instruction 'unpcklps' {SSE}.
    kIdVaddpd,                           //!< Instruction 'vaddpd' {AVX|AVX512_F+VL}.
    kIdVaddph,                           //!< Instruction 'vaddph' {AVX512_FP16+VL}.
    kIdVaddps,                           //!< Instruction 'vaddps' {AVX|AVX512_F+VL}.
    kIdVaddsd,                           //!< Instruction 'vaddsd' {AVX|AVX512_F+VL}.
    kIdVaddsh,                           //!< Instruction 'vaddsh' {AVX512_FP16+VL}.
    kIdVaddss,                           //!< Instruction 'vaddss' {AVX|AVX512_F+VL}.
    kIdVaddsubpd,                        //!< Instruction 'vaddsubpd' {AVX}.
    kIdVaddsubps,                        //!< Instruction 'vaddsubps' {AVX}.
    kIdVaesdec,                          //!< Instruction 'vaesdec' {AVX|AVX512_F+VL & AESNI|VAES}.
    kIdVaesdeclast,                      //!< Instruction 'vaesdeclast' {AVX|AVX512_F+VL & AESNI|VAES}.
    kIdVaesenc,                          //!< Instruction 'vaesenc' {AVX|AVX512_F+VL & AESNI|VAES}.
    kIdVaesenclast,                      //!< Instruction 'vaesenclast' {AVX|AVX512_F+VL & AESNI|VAES}.
    kIdVaesimc,                          //!< Instruction 'vaesimc' {AVX & AESNI}.
    kIdVaeskeygenassist,                 //!< Instruction 'vaeskeygenassist' {AVX & AESNI}.
    kIdValignd,                          //!< Instruction 'valignd' {AVX512_F+VL}.
    kIdValignq,                          //!< Instruction 'valignq' {AVX512_F+VL}.
    kIdVandnpd,                          //!< Instruction 'vandnpd' {AVX|AVX512_DQ+VL}.
    kIdVandnps,                          //!< Instruction 'vandnps' {AVX|AVX512_DQ+VL}.
    kIdVandpd,                           //!< Instruction 'vandpd' {AVX|AVX512_DQ+VL}.
    kIdVandps,                           //!< Instruction 'vandps' {AVX|AVX512_DQ+VL}.
    kIdVbcstnebf162ps,                   //!< Instruction 'vbcstnebf162ps' {AVX_NE_CONVERT}.
    kIdVbcstnesh2ps,                     //!< Instruction 'vbcstnesh2ps' {AVX_NE_CONVERT}.
    kIdVblendmpd,                        //!< Instruction 'vblendmpd' {AVX512_F+VL}.
    kIdVblendmps,                        //!< Instruction 'vblendmps' {AVX512_F+VL}.
    kIdVblendpd,                         //!< Instruction 'vblendpd' {AVX}.
    kIdVblendps,                         //!< Instruction 'vblendps' {AVX}.
    kIdVblendvpd,                        //!< Instruction 'vblendvpd' {AVX}.
    kIdVblendvps,                        //!< Instruction 'vblendvps' {AVX}.
    kIdVbroadcastf128,                   //!< Instruction 'vbroadcastf128' {AVX}.
    kIdVbroadcastf32x2,                  //!< Instruction 'vbroadcastf32x2' {AVX512_DQ+VL}.
    kIdVbroadcastf32x4,                  //!< Instruction 'vbroadcastf32x4' {AVX512_F+VL}.
    kIdVbroadcastf32x8,                  //!< Instruction 'vbroadcastf32x8' {AVX512_DQ+VL}.
    kIdVbroadcastf64x2,                  //!< Instruction 'vbroadcastf64x2' {AVX512_DQ+VL}.
    kIdVbroadcastf64x4,                  //!< Instruction 'vbroadcastf64x4' {AVX512_F+VL}.
    kIdVbroadcasti128,                   //!< Instruction 'vbroadcasti128' {AVX2}.
    kIdVbroadcasti32x2,                  //!< Instruction 'vbroadcasti32x2' {AVX512_DQ+VL}.
    kIdVbroadcasti32x4,                  //!< Instruction 'vbroadcasti32x4' {AVX512_F+VL}.
    kIdVbroadcasti32x8,                  //!< Instruction 'vbroadcasti32x8' {AVX512_DQ+VL}.
    kIdVbroadcasti64x2,                  //!< Instruction 'vbroadcasti64x2' {AVX512_DQ+VL}.
    kIdVbroadcasti64x4,                  //!< Instruction 'vbroadcasti64x4' {AVX512_F+VL}.
    kIdVbroadcastsd,                     //!< Instruction 'vbroadcastsd' {AVX|AVX2|AVX512_F+VL}.
    kIdVbroadcastss,                     //!< Instruction 'vbroadcastss' {AVX|AVX2|AVX512_F+VL}.
    kIdVcmppd,                           //!< Instruction 'vcmppd' {AVX|AVX512_F+VL}.
    kIdVcmpph,                           //!< Instruction 'vcmpph' {AVX512_FP16+VL}.
    kIdVcmpps,                           //!< Instruction 'vcmpps' {AVX|AVX512_F+VL}.
    kIdVcmpsd,                           //!< Instruction 'vcmpsd' {AVX|AVX512_F+VL}.
    kIdVcmpsh,                           //!< Instruction 'vcmpsh' {AVX512_FP16+VL}.
    kIdVcmpss,                           //!< Instruction 'vcmpss' {AVX|AVX512_F+VL}.
    kIdVcomisd,                          //!< Instruction 'vcomisd' {AVX|AVX512_F+VL}.
    kIdVcomish,                          //!< Instruction 'vcomish' {AVX512_FP16+VL}.
    kIdVcomiss,                          //!< Instruction 'vcomiss' {AVX|AVX512_F+VL}.
    kIdVcompresspd,                      //!< Instruction 'vcompresspd' {AVX512_F+VL}.
    kIdVcompressps,                      //!< Instruction 'vcompressps' {AVX512_F+VL}.
    kIdVcvtdq2pd,                        //!< Instruction 'vcvtdq2pd' {AVX|AVX512_F+VL}.
    kIdVcvtdq2ph,                        //!< Instruction 'vcvtdq2ph' {AVX512_FP16+VL}.
    kIdVcvtdq2ps,                        //!< Instruction 'vcvtdq2ps' {AVX|AVX512_F+VL}.
    kIdVcvtne2ps2bf16,                   //!< Instruction 'vcvtne2ps2bf16' {AVX512_BF16+VL}.
    kIdVcvtneebf162ps,                   //!< Instruction 'vcvtneebf162ps' {AVX_NE_CONVERT}.
    kIdVcvtneeph2ps,                     //!< Instruction 'vcvtneeph2ps' {AVX_NE_CONVERT}.
    kIdVcvtneobf162ps,                   //!< Instruction 'vcvtneobf162ps' {AVX_NE_CONVERT}.
    kIdVcvtneoph2ps,                     //!< Instruction 'vcvtneoph2ps' {AVX_NE_CONVERT}.
    kIdVcvtneps2bf16,                    //!< Instruction 'vcvtneps2bf16' {AVX_NE_CONVERT|AVX512_BF16+VL}.
    kIdVcvtpd2dq,                        //!< Instruction 'vcvtpd2dq' {AVX|AVX512_F+VL}.
    kIdVcvtpd2ph,                        //!< Instruction 'vcvtpd2ph' {AVX512_FP16+VL}.
    kIdVcvtpd2ps,                        //!< Instruction 'vcvtpd2ps' {AVX|AVX512_F+VL}.
    kIdVcvtpd2qq,                        //!< Instruction 'vcvtpd2qq' {AVX512_DQ+VL}.
    kIdVcvtpd2udq,                       //!< Instruction 'vcvtpd2udq' {AVX512_F+VL}.
    kIdVcvtpd2uqq,                       //!< Instruction 'vcvtpd2uqq' {AVX512_DQ+VL}.
    kIdVcvtph2dq,                        //!< Instruction 'vcvtph2dq' {AVX512_FP16+VL}.
    kIdVcvtph2pd,                        //!< Instruction 'vcvtph2pd' {AVX512_FP16+VL}.
    kIdVcvtph2ps,                        //!< Instruction 'vcvtph2ps' {AVX512_F+VL & F16C}.
    kIdVcvtph2psx,                       //!< Instruction 'vcvtph2psx' {AVX512_FP16+VL}.
    kIdVcvtph2qq,                        //!< Instruction 'vcvtph2qq' {AVX512_FP16+VL}.
    kIdVcvtph2udq,                       //!< Instruction 'vcvtph2udq' {AVX512_FP16+VL}.
    kIdVcvtph2uqq,                       //!< Instruction 'vcvtph2uqq' {AVX512_FP16+VL}.
    kIdVcvtph2uw,                        //!< Instruction 'vcvtph2uw' {AVX512_FP16+VL}.
    kIdVcvtph2w,                         //!< Instruction 'vcvtph2w' {AVX512_FP16+VL}.
    kIdVcvtps2dq,                        //!< Instruction 'vcvtps2dq' {AVX|AVX512_F+VL}.
    kIdVcvtps2pd,                        //!< Instruction 'vcvtps2pd' {AVX|AVX512_F+VL}.
    kIdVcvtps2ph,                        //!< Instruction 'vcvtps2ph' {AVX512_F+VL & F16C}.
    kIdVcvtps2phx,                       //!< Instruction 'vcvtps2phx' {AVX512_FP16+VL}.
    kIdVcvtps2qq,                        //!< Instruction 'vcvtps2qq' {AVX512_DQ+VL}.
    kIdVcvtps2udq,                       //!< Instruction 'vcvtps2udq' {AVX512_F+VL}.
    kIdVcvtps2uqq,                       //!< Instruction 'vcvtps2uqq' {AVX512_DQ+VL}.
    kIdVcvtqq2pd,                        //!< Instruction 'vcvtqq2pd' {AVX512_DQ+VL}.
    kIdVcvtqq2ph,                        //!< Instruction 'vcvtqq2ph' {AVX512_FP16+VL}.
    kIdVcvtqq2ps,                        //!< Instruction 'vcvtqq2ps' {AVX512_DQ+VL}.
    kIdVcvtsd2sh,                        //!< Instruction 'vcvtsd2sh' {AVX512_FP16+VL}.
    kIdVcvtsd2si,                        //!< Instruction 'vcvtsd2si' {AVX|AVX512_F+VL}.
    kIdVcvtsd2ss,                        //!< Instruction 'vcvtsd2ss' {AVX|AVX512_F+VL}.
    kIdVcvtsd2usi,                       //!< Instruction 'vcvtsd2usi' {AVX512_F+VL}.
    kIdVcvtsh2sd,                        //!< Instruction 'vcvtsh2sd' {AVX512_FP16+VL}.
    kIdVcvtsh2si,                        //!< Instruction 'vcvtsh2si' {AVX512_FP16+VL}.
    kIdVcvtsh2ss,                        //!< Instruction 'vcvtsh2ss' {AVX512_FP16+VL}.
    kIdVcvtsh2usi,                       //!< Instruction 'vcvtsh2usi' {AVX512_FP16+VL}.
    kIdVcvtsi2sd,                        //!< Instruction 'vcvtsi2sd' {AVX|AVX512_F+VL}.
    kIdVcvtsi2sh,                        //!< Instruction 'vcvtsi2sh' {AVX512_FP16+VL}.
    kIdVcvtsi2ss,                        //!< Instruction 'vcvtsi2ss' {AVX|AVX512_F+VL}.
    kIdVcvtss2sd,                        //!< Instruction 'vcvtss2sd' {AVX|AVX512_F+VL}.
    kIdVcvtss2sh,                        //!< Instruction 'vcvtss2sh' {AVX512_FP16+VL}.
    kIdVcvtss2si,                        //!< Instruction 'vcvtss2si' {AVX|AVX512_F+VL}.
    kIdVcvtss2usi,                       //!< Instruction 'vcvtss2usi' {AVX512_F+VL}.
    kIdVcvttpd2dq,                       //!< Instruction 'vcvttpd2dq' {AVX|AVX512_F+VL}.
    kIdVcvttpd2qq,                       //!< Instruction 'vcvttpd2qq' {AVX512_F+VL}.
    kIdVcvttpd2udq,                      //!< Instruction 'vcvttpd2udq' {AVX512_F+VL}.
    kIdVcvttpd2uqq,                      //!< Instruction 'vcvttpd2uqq' {AVX512_DQ+VL}.
    kIdVcvttph2dq,                       //!< Instruction 'vcvttph2dq' {AVX512_FP16+VL}.
    kIdVcvttph2qq,                       //!< Instruction 'vcvttph2qq' {AVX512_FP16+VL}.
    kIdVcvttph2udq,                      //!< Instruction 'vcvttph2udq' {AVX512_FP16+VL}.
    kIdVcvttph2uqq,                      //!< Instruction 'vcvttph2uqq' {AVX512_FP16+VL}.
    kIdVcvttph2uw,                       //!< Instruction 'vcvttph2uw' {AVX512_FP16+VL}.
    kIdVcvttph2w,                        //!< Instruction 'vcvttph2w' {AVX512_FP16+VL}.
    kIdVcvttps2dq,                       //!< Instruction 'vcvttps2dq' {AVX|AVX512_F+VL}.
    kIdVcvttps2qq,                       //!< Instruction 'vcvttps2qq' {AVX512_DQ+VL}.
    kIdVcvttps2udq,                      //!< Instruction 'vcvttps2udq' {AVX512_F+VL}.
    kIdVcvttps2uqq,                      //!< Instruction 'vcvttps2uqq' {AVX512_DQ+VL}.
    kIdVcvttsd2si,                       //!< Instruction 'vcvttsd2si' {AVX|AVX512_F+VL}.
    kIdVcvttsd2usi,                      //!< Instruction 'vcvttsd2usi' {AVX512_F+VL}.
    kIdVcvttsh2si,                       //!< Instruction 'vcvttsh2si' {AVX512_FP16+VL}.
    kIdVcvttsh2usi,                      //!< Instruction 'vcvttsh2usi' {AVX512_FP16+VL}.
    kIdVcvttss2si,                       //!< Instruction 'vcvttss2si' {AVX|AVX512_F+VL}.
    kIdVcvttss2usi,                      //!< Instruction 'vcvttss2usi' {AVX512_F+VL}.
    kIdVcvtudq2pd,                       //!< Instruction 'vcvtudq2pd' {AVX512_F+VL}.
    kIdVcvtudq2ph,                       //!< Instruction 'vcvtudq2ph' {AVX512_FP16+VL}.
    kIdVcvtudq2ps,                       //!< Instruction 'vcvtudq2ps' {AVX512_F+VL}.
    kIdVcvtuqq2pd,                       //!< Instruction 'vcvtuqq2pd' {AVX512_DQ+VL}.
    kIdVcvtuqq2ph,                       //!< Instruction 'vcvtuqq2ph' {AVX512_FP16+VL}.
    kIdVcvtuqq2ps,                       //!< Instruction 'vcvtuqq2ps' {AVX512_DQ+VL}.
    kIdVcvtusi2sd,                       //!< Instruction 'vcvtusi2sd' {AVX512_F+VL}.
    kIdVcvtusi2sh,                       //!< Instruction 'vcvtusi2sh' {AVX512_FP16+VL}.
    kIdVcvtusi2ss,                       //!< Instruction 'vcvtusi2ss' {AVX512_F+VL}.
    kIdVcvtuw2ph,                        //!< Instruction 'vcvtuw2ph' {AVX512_FP16+VL}.
    kIdVcvtw2ph,                         //!< Instruction 'vcvtw2ph' {AVX512_FP16+VL}.
    kIdVdbpsadbw,                        //!< Instruction 'vdbpsadbw' {AVX512_BW+VL}.
    kIdVdivpd,                           //!< Instruction 'vdivpd' {AVX|AVX512_F+VL}.
    kIdVdivph,                           //!< Instruction 'vdivph' {AVX512_FP16+VL}.
    kIdVdivps,                           //!< Instruction 'vdivps' {AVX|AVX512_F+VL}.
    kIdVdivsd,                           //!< Instruction 'vdivsd' {AVX|AVX512_F+VL}.
    kIdVdivsh,                           //!< Instruction 'vdivsh' {AVX512_FP16+VL}.
    kIdVdivss,                           //!< Instruction 'vdivss' {AVX|AVX512_F+VL}.
    kIdVdpbf16ps,                        //!< Instruction 'vdpbf16ps' {AVX512_BF16+VL}.
    kIdVdppd,                            //!< Instruction 'vdppd' {AVX}.
    kIdVdpps,                            //!< Instruction 'vdpps' {AVX}.
    kIdVerr,                             //!< Instruction 'verr'.
    kIdVerw,                             //!< Instruction 'verw'.
    kIdVexpandpd,                        //!< Instruction 'vexpandpd' {AVX512_F+VL}.
    kIdVexpandps,                        //!< Instruction 'vexpandps' {AVX512_F+VL}.
    kIdVextractf128,                     //!< Instruction 'vextractf128' {AVX}.
    kIdVextractf32x4,                    //!< Instruction 'vextractf32x4' {AVX512_F+VL}.
    kIdVextractf32x8,                    //!< Instruction 'vextractf32x8' {AVX512_DQ+VL}.
    kIdVextractf64x2,                    //!< Instruction 'vextractf64x2' {AVX512_DQ+VL}.
    kIdVextractf64x4,                    //!< Instruction 'vextractf64x4' {AVX512_F+VL}.
    kIdVextracti128,                     //!< Instruction 'vextracti128' {AVX2}.
    kIdVextracti32x4,                    //!< Instruction 'vextracti32x4' {AVX512_F+VL}.
    kIdVextracti32x8,                    //!< Instruction 'vextracti32x8' {AVX512_DQ+VL}.
    kIdVextracti64x2,                    //!< Instruction 'vextracti64x2' {AVX512_DQ+VL}.
    kIdVextracti64x4,                    //!< Instruction 'vextracti64x4' {AVX512_F+VL}.
    kIdVextractps,                       //!< Instruction 'vextractps' {AVX|AVX512_F+VL}.
    kIdVfcmaddcph,                       //!< Instruction 'vfcmaddcph' {AVX512_FP16+VL}.
    kIdVfcmaddcsh,                       //!< Instruction 'vfcmaddcsh' {AVX512_FP16+VL}.
    kIdVfcmulcph,                        //!< Instruction 'vfcmulcph' {AVX512_FP16+VL}.
    kIdVfcmulcsh,                        //!< Instruction 'vfcmulcsh' {AVX512_FP16+VL}.
    kIdVfixupimmpd,                      //!< Instruction 'vfixupimmpd' {AVX512_F+VL}.
    kIdVfixupimmps,                      //!< Instruction 'vfixupimmps' {AVX512_F+VL}.
    kIdVfixupimmsd,                      //!< Instruction 'vfixupimmsd' {AVX512_F+VL}.
    kIdVfixupimmss,                      //!< Instruction 'vfixupimmss' {AVX512_F+VL}.
    kIdVfmadd132pd,                      //!< Instruction 'vfmadd132pd' {FMA|AVX512_F+VL}.
    kIdVfmadd132ph,                      //!< Instruction 'vfmadd132ph' {AVX512_FP16+VL}.
    kIdVfmadd132ps,                      //!< Instruction 'vfmadd132ps' {FMA|AVX512_F+VL}.
    kIdVfmadd132sd,                      //!< Instruction 'vfmadd132sd' {FMA|AVX512_F+VL}.
    kIdVfmadd132sh,                      //!< Instruction 'vfmadd132sh' {AVX512_FP16+VL}.
    kIdVfmadd132ss,                      //!< Instruction 'vfmadd132ss' {FMA|AVX512_F+VL}.
    kIdVfmadd213pd,                      //!< Instruction 'vfmadd213pd' {FMA|AVX512_F+VL}.
    kIdVfmadd213ph,                      //!< Instruction 'vfmadd213ph' {AVX512_FP16+VL}.
    kIdVfmadd213ps,                      //!< Instruction 'vfmadd213ps' {FMA|AVX512_F+VL}.
    kIdVfmadd213sd,                      //!< Instruction 'vfmadd213sd' {FMA|AVX512_F+VL}.
    kIdVfmadd213sh,                      //!< Instruction 'vfmadd213sh' {AVX512_FP16+VL}.
    kIdVfmadd213ss,                      //!< Instruction 'vfmadd213ss' {FMA|AVX512_F+VL}.
    kIdVfmadd231pd,                      //!< Instruction 'vfmadd231pd' {FMA|AVX512_F+VL}.
    kIdVfmadd231ph,                      //!< Instruction 'vfmadd231ph' {AVX512_FP16+VL}.
    kIdVfmadd231ps,                      //!< Instruction 'vfmadd231ps' {FMA|AVX512_F+VL}.
    kIdVfmadd231sd,                      //!< Instruction 'vfmadd231sd' {FMA|AVX512_F+VL}.
    kIdVfmadd231sh,                      //!< Instruction 'vfmadd231sh' {AVX512_FP16+VL}.
    kIdVfmadd231ss,                      //!< Instruction 'vfmadd231ss' {FMA|AVX512_F+VL}.
    kIdVfmaddcph,                        //!< Instruction 'vfmaddcph' {AVX512_FP16+VL}.
    kIdVfmaddcsh,                        //!< Instruction 'vfmaddcsh' {AVX512_FP16+VL}.
    kIdVfmaddpd,                         //!< Instruction 'vfmaddpd' {FMA4}.
    kIdVfmaddps,                         //!< Instruction 'vfmaddps' {FMA4}.
    kIdVfmaddsd,                         //!< Instruction 'vfmaddsd' {FMA4}.
    kIdVfmaddss,                         //!< Instruction 'vfmaddss' {FMA4}.
    kIdVfmaddsub132pd,                   //!< Instruction 'vfmaddsub132pd' {FMA|AVX512_F+VL}.
    kIdVfmaddsub132ph,                   //!< Instruction 'vfmaddsub132ph' {AVX512_FP16+VL}.
    kIdVfmaddsub132ps,                   //!< Instruction 'vfmaddsub132ps' {FMA|AVX512_F+VL}.
    kIdVfmaddsub213pd,                   //!< Instruction 'vfmaddsub213pd' {FMA|AVX512_F+VL}.
    kIdVfmaddsub213ph,                   //!< Instruction 'vfmaddsub213ph' {AVX512_FP16+VL}.
    kIdVfmaddsub213ps,                   //!< Instruction 'vfmaddsub213ps' {FMA|AVX512_F+VL}.
    kIdVfmaddsub231pd,                   //!< Instruction 'vfmaddsub231pd' {FMA|AVX512_F+VL}.
    kIdVfmaddsub231ph,                   //!< Instruction 'vfmaddsub231ph' {AVX512_FP16+VL}.
    kIdVfmaddsub231ps,                   //!< Instruction 'vfmaddsub231ps' {FMA|AVX512_F+VL}.
    kIdVfmaddsubpd,                      //!< Instruction 'vfmaddsubpd' {FMA4}.
    kIdVfmaddsubps,                      //!< Instruction 'vfmaddsubps' {FMA4}.
    kIdVfmsub132pd,                      //!< Instruction 'vfmsub132pd' {FMA|AVX512_F+VL}.
    kIdVfmsub132ph,                      //!< Instruction 'vfmsub132ph' {AVX512_FP16+VL}.
    kIdVfmsub132ps,                      //!< Instruction 'vfmsub132ps' {FMA|AVX512_F+VL}.
    kIdVfmsub132sd,                      //!< Instruction 'vfmsub132sd' {FMA|AVX512_F+VL}.
    kIdVfmsub132sh,                      //!< Instruction 'vfmsub132sh' {AVX512_FP16+VL}.
    kIdVfmsub132ss,                      //!< Instruction 'vfmsub132ss' {FMA|AVX512_F+VL}.
    kIdVfmsub213pd,                      //!< Instruction 'vfmsub213pd' {FMA|AVX512_F+VL}.
    kIdVfmsub213ph,                      //!< Instruction 'vfmsub213ph' {AVX512_FP16+VL}.
    kIdVfmsub213ps,                      //!< Instruction 'vfmsub213ps' {FMA|AVX512_F+VL}.
    kIdVfmsub213sd,                      //!< Instruction 'vfmsub213sd' {FMA|AVX512_F+VL}.
    kIdVfmsub213sh,                      //!< Instruction 'vfmsub213sh' {AVX512_FP16+VL}.
    kIdVfmsub213ss,                      //!< Instruction 'vfmsub213ss' {FMA|AVX512_F+VL}.
    kIdVfmsub231pd,                      //!< Instruction 'vfmsub231pd' {FMA|AVX512_F+VL}.
    kIdVfmsub231ph,                      //!< Instruction 'vfmsub231ph' {AVX512_FP16+VL}.
    kIdVfmsub231ps,                      //!< Instruction 'vfmsub231ps' {FMA|AVX512_F+VL}.
    kIdVfmsub231sd,                      //!< Instruction 'vfmsub231sd' {FMA|AVX512_F+VL}.
    kIdVfmsub231sh,                      //!< Instruction 'vfmsub231sh' {AVX512_FP16+VL}.
    kIdVfmsub231ss,                      //!< Instruction 'vfmsub231ss' {FMA|AVX512_F+VL}.
    kIdVfmsubadd132pd,                   //!< Instruction 'vfmsubadd132pd' {FMA|AVX512_F+VL}.
    kIdVfmsubadd132ph,                   //!< Instruction 'vfmsubadd132ph' {AVX512_FP16+VL}.
    kIdVfmsubadd132ps,                   //!< Instruction 'vfmsubadd132ps' {FMA|AVX512_F+VL}.
    kIdVfmsubadd213pd,                   //!< Instruction 'vfmsubadd213pd' {FMA|AVX512_F+VL}.
    kIdVfmsubadd213ph,                   //!< Instruction 'vfmsubadd213ph' {AVX512_FP16+VL}.
    kIdVfmsubadd213ps,                   //!< Instruction 'vfmsubadd213ps' {FMA|AVX512_F+VL}.
    kIdVfmsubadd231pd,                   //!< Instruction 'vfmsubadd231pd' {FMA|AVX512_F+VL}.
    kIdVfmsubadd231ph,                   //!< Instruction 'vfmsubadd231ph' {AVX512_FP16+VL}.
    kIdVfmsubadd231ps,                   //!< Instruction 'vfmsubadd231ps' {FMA|AVX512_F+VL}.
    kIdVfmsubaddpd,                      //!< Instruction 'vfmsubaddpd' {FMA4}.
    kIdVfmsubaddps,                      //!< Instruction 'vfmsubaddps' {FMA4}.
    kIdVfmsubpd,                         //!< Instruction 'vfmsubpd' {FMA4}.
    kIdVfmsubps,                         //!< Instruction 'vfmsubps' {FMA4}.
    kIdVfmsubsd,                         //!< Instruction 'vfmsubsd' {FMA4}.
    kIdVfmsubss,                         //!< Instruction 'vfmsubss' {FMA4}.
    kIdVfmulcph,                         //!< Instruction 'vfmulcph' {AVX512_FP16+VL}.
    kIdVfmulcsh,                         //!< Instruction 'vfmulcsh' {AVX512_FP16+VL}.
    kIdVfnmadd132pd,                     //!< Instruction 'vfnmadd132pd' {FMA|AVX512_F+VL}.
    kIdVfnmadd132ph,                     //!< Instruction 'vfnmadd132ph' {AVX512_FP16+VL}.
    kIdVfnmadd132ps,                     //!< Instruction 'vfnmadd132ps' {FMA|AVX512_F+VL}.
    kIdVfnmadd132sd,                     //!< Instruction 'vfnmadd132sd' {FMA|AVX512_F+VL}.
    kIdVfnmadd132sh,                     //!< Instruction 'vfnmadd132sh' {AVX512_FP16+VL}.
    kIdVfnmadd132ss,                     //!< Instruction 'vfnmadd132ss' {FMA|AVX512_F+VL}.
    kIdVfnmadd213pd,                     //!< Instruction 'vfnmadd213pd' {FMA|AVX512_F+VL}.
    kIdVfnmadd213ph,                     //!< Instruction 'vfnmadd213ph' {AVX512_FP16+VL}.
    kIdVfnmadd213ps,                     //!< Instruction 'vfnmadd213ps' {FMA|AVX512_F+VL}.
    kIdVfnmadd213sd,                     //!< Instruction 'vfnmadd213sd' {FMA|AVX512_F+VL}.
    kIdVfnmadd213sh,                     //!< Instruction 'vfnmadd213sh' {AVX512_FP16+VL}.
    kIdVfnmadd213ss,                     //!< Instruction 'vfnmadd213ss' {FMA|AVX512_F+VL}.
    kIdVfnmadd231pd,                     //!< Instruction 'vfnmadd231pd' {FMA|AVX512_F+VL}.
    kIdVfnmadd231ph,                     //!< Instruction 'vfnmadd231ph' {AVX512_FP16+VL}.
    kIdVfnmadd231ps,                     //!< Instruction 'vfnmadd231ps' {FMA|AVX512_F+VL}.
    kIdVfnmadd231sd,                     //!< Instruction 'vfnmadd231sd' {FMA|AVX512_F+VL}.
    kIdVfnmadd231sh,                     //!< Instruction 'vfnmadd231sh' {AVX512_FP16+VL}.
    kIdVfnmadd231ss,                     //!< Instruction 'vfnmadd231ss' {FMA|AVX512_F+VL}.
    kIdVfnmaddpd,                        //!< Instruction 'vfnmaddpd' {FMA4}.
    kIdVfnmaddps,                        //!< Instruction 'vfnmaddps' {FMA4}.
    kIdVfnmaddsd,                        //!< Instruction 'vfnmaddsd' {FMA4}.
    kIdVfnmaddss,                        //!< Instruction 'vfnmaddss' {FMA4}.
    kIdVfnmsub132pd,                     //!< Instruction 'vfnmsub132pd' {FMA|AVX512_F+VL}.
    kIdVfnmsub132ph,                     //!< Instruction 'vfnmsub132ph' {AVX512_FP16+VL}.
    kIdVfnmsub132ps,                     //!< Instruction 'vfnmsub132ps' {FMA|AVX512_F+VL}.
    kIdVfnmsub132sd,                     //!< Instruction 'vfnmsub132sd' {FMA|AVX512_F+VL}.
    kIdVfnmsub132sh,                     //!< Instruction 'vfnmsub132sh' {AVX512_FP16+VL}.
    kIdVfnmsub132ss,                     //!< Instruction 'vfnmsub132ss' {FMA|AVX512_F+VL}.
    kIdVfnmsub213pd,                     //!< Instruction 'vfnmsub213pd' {FMA|AVX512_F+VL}.
    kIdVfnmsub213ph,                     //!< Instruction 'vfnmsub213ph' {AVX512_FP16+VL}.
    kIdVfnmsub213ps,                     //!< Instruction 'vfnmsub213ps' {FMA|AVX512_F+VL}.
    kIdVfnmsub213sd,                     //!< Instruction 'vfnmsub213sd' {FMA|AVX512_F+VL}.
    kIdVfnmsub213sh,                     //!< Instruction 'vfnmsub213sh' {AVX512_FP16+VL}.
    kIdVfnmsub213ss,                     //!< Instruction 'vfnmsub213ss' {FMA|AVX512_F+VL}.
    kIdVfnmsub231pd,                     //!< Instruction 'vfnmsub231pd' {FMA|AVX512_F+VL}.
    kIdVfnmsub231ph,                     //!< Instruction 'vfnmsub231ph' {AVX512_FP16+VL}.
    kIdVfnmsub231ps,                     //!< Instruction 'vfnmsub231ps' {FMA|AVX512_F+VL}.
    kIdVfnmsub231sd,                     //!< Instruction 'vfnmsub231sd' {FMA|AVX512_F+VL}.
    kIdVfnmsub231sh,                     //!< Instruction 'vfnmsub231sh' {AVX512_FP16+VL}.
    kIdVfnmsub231ss,                     //!< Instruction 'vfnmsub231ss' {FMA|AVX512_F+VL}.
    kIdVfnmsubpd,                        //!< Instruction 'vfnmsubpd' {FMA4}.
    kIdVfnmsubps,                        //!< Instruction 'vfnmsubps' {FMA4}.
    kIdVfnmsubsd,                        //!< Instruction 'vfnmsubsd' {FMA4}.
    kIdVfnmsubss,                        //!< Instruction 'vfnmsubss' {FMA4}.
    kIdVfpclasspd,                       //!< Instruction 'vfpclasspd' {AVX512_DQ+VL}.
    kIdVfpclassph,                       //!< Instruction 'vfpclassph' {AVX512_FP16+VL}.
    kIdVfpclassps,                       //!< Instruction 'vfpclassps' {AVX512_DQ+VL}.
    kIdVfpclasssd,                       //!< Instruction 'vfpclasssd' {AVX512_DQ+VL}.
    kIdVfpclasssh,                       //!< Instruction 'vfpclasssh' {AVX512_FP16+VL}.
    kIdVfpclassss,                       //!< Instruction 'vfpclassss' {AVX512_DQ+VL}.
    kIdVfrczpd,                          //!< Instruction 'vfrczpd' {XOP}.
    kIdVfrczps,                          //!< Instruction 'vfrczps' {XOP}.
    kIdVfrczsd,                          //!< Instruction 'vfrczsd' {XOP}.
    kIdVfrczss,                          //!< Instruction 'vfrczss' {XOP}.
    kIdVgatherdpd,                       //!< Instruction 'vgatherdpd' {AVX2|AVX512_F+VL}.
    kIdVgatherdps,                       //!< Instruction 'vgatherdps' {AVX2|AVX512_F+VL}.
    kIdVgatherqpd,                       //!< Instruction 'vgatherqpd' {AVX2|AVX512_F+VL}.
    kIdVgatherqps,                       //!< Instruction 'vgatherqps' {AVX2|AVX512_F+VL}.
    kIdVgetexppd,                        //!< Instruction 'vgetexppd' {AVX512_F+VL}.
    kIdVgetexpph,                        //!< Instruction 'vgetexpph' {AVX512_FP16+VL}.
    kIdVgetexpps,                        //!< Instruction 'vgetexpps' {AVX512_F+VL}.
    kIdVgetexpsd,                        //!< Instruction 'vgetexpsd' {AVX512_F+VL}.
    kIdVgetexpsh,                        //!< Instruction 'vgetexpsh' {AVX512_FP16+VL}.
    kIdVgetexpss,                        //!< Instruction 'vgetexpss' {AVX512_F+VL}.
    kIdVgetmantpd,                       //!< Instruction 'vgetmantpd' {AVX512_F+VL}.
    kIdVgetmantph,                       //!< Instruction 'vgetmantph' {AVX512_FP16+VL}.
    kIdVgetmantps,                       //!< Instruction 'vgetmantps' {AVX512_F+VL}.
    kIdVgetmantsd,                       //!< Instruction 'vgetmantsd' {AVX512_F+VL}.
    kIdVgetmantsh,                       //!< Instruction 'vgetmantsh' {AVX512_FP16+VL}.
    kIdVgetmantss,                       //!< Instruction 'vgetmantss' {AVX512_F+VL}.
    kIdVgf2p8affineinvqb,                //!< Instruction 'vgf2p8affineinvqb' {AVX|AVX512_F+VL & GFNI}.
    kIdVgf2p8affineqb,                   //!< Instruction 'vgf2p8affineqb' {AVX|AVX512_F+VL & GFNI}.
    kIdVgf2p8mulb,                       //!< Instruction 'vgf2p8mulb' {AVX|AVX512_F+VL & GFNI}.
    kIdVhaddpd,                          //!< Instruction 'vhaddpd' {AVX}.
    kIdVhaddps,                          //!< Instruction 'vhaddps' {AVX}.
    kIdVhsubpd,                          //!< Instruction 'vhsubpd' {AVX}.
    kIdVhsubps,                          //!< Instruction 'vhsubps' {AVX}.
    kIdVinsertf128,                      //!< Instruction 'vinsertf128' {AVX}.
    kIdVinsertf32x4,                     //!< Instruction 'vinsertf32x4' {AVX512_F+VL}.
    kIdVinsertf32x8,                     //!< Instruction 'vinsertf32x8' {AVX512_DQ+VL}.
    kIdVinsertf64x2,                     //!< Instruction 'vinsertf64x2' {AVX512_DQ+VL}.
    kIdVinsertf64x4,                     //!< Instruction 'vinsertf64x4' {AVX512_F+VL}.
    kIdVinserti128,                      //!< Instruction 'vinserti128' {AVX2}.
    kIdVinserti32x4,                     //!< Instruction 'vinserti32x4' {AVX512_F+VL}.
    kIdVinserti32x8,                     //!< Instruction 'vinserti32x8' {AVX512_DQ+VL}.
    kIdVinserti64x2,                     //!< Instruction 'vinserti64x2' {AVX512_DQ+VL}.
    kIdVinserti64x4,                     //!< Instruction 'vinserti64x4' {AVX512_F+VL}.
    kIdVinsertps,                        //!< Instruction 'vinsertps' {AVX|AVX512_F+VL}.
    kIdVlddqu,                           //!< Instruction 'vlddqu' {AVX}.
    kIdVldmxcsr,                         //!< Instruction 'vldmxcsr' {AVX}.
    kIdVmaskmovdqu,                      //!< Instruction 'vmaskmovdqu' {AVX}.
    kIdVmaskmovpd,                       //!< Instruction 'vmaskmovpd' {AVX}.
    kIdVmaskmovps,                       //!< Instruction 'vmaskmovps' {AVX}.
    kIdVmaxpd,                           //!< Instruction 'vmaxpd' {AVX|AVX512_F+VL}.
    kIdVmaxph,                           //!< Instruction 'vmaxph' {AVX512_FP16+VL}.
    kIdVmaxps,                           //!< Instruction 'vmaxps' {AVX|AVX512_F+VL}.
    kIdVmaxsd,                           //!< Instruction 'vmaxsd' {AVX|AVX512_F+VL}.
    kIdVmaxsh,                           //!< Instruction 'vmaxsh' {AVX512_FP16+VL}.
    kIdVmaxss,                           //!< Instruction 'vmaxss' {AVX|AVX512_F+VL}.
    kIdVmcall,                           //!< Instruction 'vmcall' {VMX}.
    kIdVmclear,                          //!< Instruction 'vmclear' {VMX}.
    kIdVmfunc,                           //!< Instruction 'vmfunc' {VMX}.
    kIdVmgexit,                          //!< Instruction 'vmgexit' {SEV_ES}.
    kIdVminpd,                           //!< Instruction 'vminpd' {AVX|AVX512_F+VL}.
    kIdVminph,                           //!< Instruction 'vminph' {AVX512_FP16+VL}.
    kIdVminps,                           //!< Instruction 'vminps' {AVX|AVX512_F+VL}.
    kIdVminsd,                           //!< Instruction 'vminsd' {AVX|AVX512_F+VL}.
    kIdVminsh,                           //!< Instruction 'vminsh' {AVX512_FP16+VL}.
    kIdVminss,                           //!< Instruction 'vminss' {AVX|AVX512_F+VL}.
    kIdVmlaunch,                         //!< Instruction 'vmlaunch' {VMX}.
    kIdVmload,                           //!< Instruction 'vmload' {SVM}.
    kIdVmmcall,                          //!< Instruction 'vmmcall' {SVM}.
    kIdVmovapd,                          //!< Instruction 'vmovapd' {AVX|AVX512_F+VL}.
    kIdVmovaps,                          //!< Instruction 'vmovaps' {AVX|AVX512_F+VL}.
    kIdVmovd,                            //!< Instruction 'vmovd' {AVX|AVX512_F+VL}.
    kIdVmovddup,                         //!< Instruction 'vmovddup' {AVX|AVX512_F+VL}.
    kIdVmovdqa,                          //!< Instruction 'vmovdqa' {AVX}.
    kIdVmovdqa32,                        //!< Instruction 'vmovdqa32' {AVX512_F+VL}.
    kIdVmovdqa64,                        //!< Instruction 'vmovdqa64' {AVX512_F+VL}.
    kIdVmovdqu,                          //!< Instruction 'vmovdqu' {AVX}.
    kIdVmovdqu16,                        //!< Instruction 'vmovdqu16' {AVX512_BW+VL}.
    kIdVmovdqu32,                        //!< Instruction 'vmovdqu32' {AVX512_F+VL}.
    kIdVmovdqu64,                        //!< Instruction 'vmovdqu64' {AVX512_F+VL}.
    kIdVmovdqu8,                         //!< Instruction 'vmovdqu8' {AVX512_BW+VL}.
    kIdVmovhlps,                         //!< Instruction 'vmovhlps' {AVX|AVX512_F+VL}.
    kIdVmovhpd,                          //!< Instruction 'vmovhpd' {AVX|AVX512_F+VL}.
    kIdVmovhps,                          //!< Instruction 'vmovhps' {AVX|AVX512_F+VL}.
    kIdVmovlhps,                         //!< Instruction 'vmovlhps' {AVX|AVX512_F+VL}.
    kIdVmovlpd,                          //!< Instruction 'vmovlpd' {AVX|AVX512_F+VL}.
    kIdVmovlps,                          //!< Instruction 'vmovlps' {AVX|AVX512_F+VL}.
    kIdVmovmskpd,                        //!< Instruction 'vmovmskpd' {AVX}.
    kIdVmovmskps,                        //!< Instruction 'vmovmskps' {AVX}.
    kIdVmovntdq,                         //!< Instruction 'vmovntdq' {AVX|AVX512_F+VL}.
    kIdVmovntdqa,                        //!< Instruction 'vmovntdqa' {AVX|AVX2|AVX512_F+VL}.
    kIdVmovntpd,                         //!< Instruction 'vmovntpd' {AVX|AVX512_F+VL}.
    kIdVmovntps,                         //!< Instruction 'vmovntps' {AVX|AVX512_F+VL}.
    kIdVmovq,                            //!< Instruction 'vmovq' {AVX|AVX512_F+VL}.
    kIdVmovsd,                           //!< Instruction 'vmovsd' {AVX|AVX512_F+VL}.
    kIdVmovsh,                           //!< Instruction 'vmovsh' {AVX512_FP16+VL}.
    kIdVmovshdup,                        //!< Instruction 'vmovshdup' {AVX|AVX512_F+VL}.
    kIdVmovsldup,                        //!< Instruction 'vmovsldup' {AVX|AVX512_F+VL}.
    kIdVmovss,                           //!< Instruction 'vmovss' {AVX|AVX512_F+VL}.
    kIdVmovupd,                          //!< Instruction 'vmovupd' {AVX|AVX512_F+VL}.
    kIdVmovups,                          //!< Instruction 'vmovups' {AVX|AVX512_F+VL}.
    kIdVmovw,                            //!< Instruction 'vmovw' {AVX512_FP16+VL}.
    kIdVmpsadbw,                         //!< Instruction 'vmpsadbw' {AVX|AVX2}.
    kIdVmptrld,                          //!< Instruction 'vmptrld' {VMX}.
    kIdVmptrst,                          //!< Instruction 'vmptrst' {VMX}.
    kIdVmread,                           //!< Instruction 'vmread' {VMX}.
    kIdVmresume,                         //!< Instruction 'vmresume' {VMX}.
    kIdVmrun,                            //!< Instruction 'vmrun' {SVM}.
    kIdVmsave,                           //!< Instruction 'vmsave' {SVM}.
    kIdVmulpd,                           //!< Instruction 'vmulpd' {AVX|AVX512_F+VL}.
    kIdVmulph,                           //!< Instruction 'vmulph' {AVX512_FP16+VL}.
    kIdVmulps,                           //!< Instruction 'vmulps' {AVX|AVX512_F+VL}.
    kIdVmulsd,                           //!< Instruction 'vmulsd' {AVX|AVX512_F+VL}.
    kIdVmulsh,                           //!< Instruction 'vmulsh' {AVX512_FP16+VL}.
    kIdVmulss,                           //!< Instruction 'vmulss' {AVX|AVX512_F+VL}.
    kIdVmwrite,                          //!< Instruction 'vmwrite' {VMX}.
    kIdVmxoff,                           //!< Instruction 'vmxoff' {VMX}.
    kIdVmxon,                            //!< Instruction 'vmxon' {VMX}.
    kIdVorpd,                            //!< Instruction 'vorpd' {AVX|AVX512_DQ+VL}.
    kIdVorps,                            //!< Instruction 'vorps' {AVX|AVX512_DQ+VL}.
    kIdVp2intersectd,                    //!< Instruction 'vp2intersectd' {AVX512_VP2INTERSECT+VL}.
    kIdVp2intersectq,                    //!< Instruction 'vp2intersectq' {AVX512_VP2INTERSECT+VL}.
    kIdVpabsb,                           //!< Instruction 'vpabsb' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpabsd,                           //!< Instruction 'vpabsd' {AVX|AVX2|AVX512_F+VL}.
    kIdVpabsq,                           //!< Instruction 'vpabsq' {AVX512_F+VL}.
    kIdVpabsw,                           //!< Instruction 'vpabsw' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpackssdw,                        //!< Instruction 'vpackssdw' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpacksswb,                        //!< Instruction 'vpacksswb' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpackusdw,                        //!< Instruction 'vpackusdw' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpackuswb,                        //!< Instruction 'vpackuswb' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpaddb,                           //!< Instruction 'vpaddb' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpaddd,                           //!< Instruction 'vpaddd' {AVX|AVX2|AVX512_F+VL}.
    kIdVpaddq,                           //!< Instruction 'vpaddq' {AVX|AVX2|AVX512_F+VL}.
    kIdVpaddsb,                          //!< Instruction 'vpaddsb' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpaddsw,                          //!< Instruction 'vpaddsw' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpaddusb,                         //!< Instruction 'vpaddusb' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpaddusw,                         //!< Instruction 'vpaddusw' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpaddw,                           //!< Instruction 'vpaddw' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpalignr,                         //!< Instruction 'vpalignr' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpand,                            //!< Instruction 'vpand' {AVX|AVX2}.
    kIdVpandd,                           //!< Instruction 'vpandd' {AVX512_F+VL}.
    kIdVpandn,                           //!< Instruction 'vpandn' {AVX|AVX2}.
    kIdVpandnd,                          //!< Instruction 'vpandnd' {AVX512_F+VL}.
    kIdVpandnq,                          //!< Instruction 'vpandnq' {AVX512_F+VL}.
    kIdVpandq,                           //!< Instruction 'vpandq' {AVX512_F+VL}.
    kIdVpavgb,                           //!< Instruction 'vpavgb' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpavgw,                           //!< Instruction 'vpavgw' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpblendd,                         //!< Instruction 'vpblendd' {AVX2}.
    kIdVpblendmb,                        //!< Instruction 'vpblendmb' {AVX512_BW+VL}.
    kIdVpblendmd,                        //!< Instruction 'vpblendmd' {AVX512_F+VL}.
    kIdVpblendmq,                        //!< Instruction 'vpblendmq' {AVX512_F+VL}.
    kIdVpblendmw,                        //!< Instruction 'vpblendmw' {AVX512_BW+VL}.
    kIdVpblendvb,                        //!< Instruction 'vpblendvb' {AVX|AVX2}.
    kIdVpblendw,                         //!< Instruction 'vpblendw' {AVX|AVX2}.
    kIdVpbroadcastb,                     //!< Instruction 'vpbroadcastb' {AVX2|AVX512_BW+VL}.
    kIdVpbroadcastd,                     //!< Instruction 'vpbroadcastd' {AVX2|AVX512_F+VL}.
    kIdVpbroadcastmb2q,                  //!< Instruction 'vpbroadcastmb2q' {AVX512_CD+VL}.
    kIdVpbroadcastmw2d,                  //!< Instruction 'vpbroadcastmw2d' {AVX512_CD+VL}.
    kIdVpbroadcastq,                     //!< Instruction 'vpbroadcastq' {AVX2|AVX512_F+VL}.
    kIdVpbroadcastw,                     //!< Instruction 'vpbroadcastw' {AVX2|AVX512_BW+VL}.
    kIdVpclmulqdq,                       //!< Instruction 'vpclmulqdq' {AVX|AVX512_F+VL & PCLMULQDQ|VPCLMULQDQ}.
    kIdVpcmov,                           //!< Instruction 'vpcmov' {XOP}.
    kIdVpcmpb,                           //!< Instruction 'vpcmpb' {AVX512_BW+VL}.
    kIdVpcmpd,                           //!< Instruction 'vpcmpd' {AVX512_F+VL}.
    kIdVpcmpeqb,                         //!< Instruction 'vpcmpeqb' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpcmpeqd,                         //!< Instruction 'vpcmpeqd' {AVX|AVX2|AVX512_F+VL}.
    kIdVpcmpeqq,                         //!< Instruction 'vpcmpeqq' {AVX|AVX2|AVX512_F+VL}.
    kIdVpcmpeqw,                         //!< Instruction 'vpcmpeqw' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpcmpestri,                       //!< Instruction 'vpcmpestri' {AVX}.
    kIdVpcmpestrm,                       //!< Instruction 'vpcmpestrm' {AVX}.
    kIdVpcmpgtb,                         //!< Instruction 'vpcmpgtb' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpcmpgtd,                         //!< Instruction 'vpcmpgtd' {AVX|AVX2|AVX512_F+VL}.
    kIdVpcmpgtq,                         //!< Instruction 'vpcmpgtq' {AVX|AVX2|AVX512_F+VL}.
    kIdVpcmpgtw,                         //!< Instruction 'vpcmpgtw' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpcmpistri,                       //!< Instruction 'vpcmpistri' {AVX}.
    kIdVpcmpistrm,                       //!< Instruction 'vpcmpistrm' {AVX}.
    kIdVpcmpq,                           //!< Instruction 'vpcmpq' {AVX512_F+VL}.
    kIdVpcmpub,                          //!< Instruction 'vpcmpub' {AVX512_BW+VL}.
    kIdVpcmpud,                          //!< Instruction 'vpcmpud' {AVX512_F+VL}.
    kIdVpcmpuq,                          //!< Instruction 'vpcmpuq' {AVX512_F+VL}.
    kIdVpcmpuw,                          //!< Instruction 'vpcmpuw' {AVX512_BW+VL}.
    kIdVpcmpw,                           //!< Instruction 'vpcmpw' {AVX512_BW+VL}.
    kIdVpcomb,                           //!< Instruction 'vpcomb' {XOP}.
    kIdVpcomd,                           //!< Instruction 'vpcomd' {XOP}.
    kIdVpcompressb,                      //!< Instruction 'vpcompressb' {AVX512_VBMI2+VL}.
    kIdVpcompressd,                      //!< Instruction 'vpcompressd' {AVX512_F+VL}.
    kIdVpcompressq,                      //!< Instruction 'vpcompressq' {AVX512_F+VL}.
    kIdVpcompressw,                      //!< Instruction 'vpcompressw' {AVX512_VBMI2+VL}.
    kIdVpcomq,                           //!< Instruction 'vpcomq' {XOP}.
    kIdVpcomub,                          //!< Instruction 'vpcomub' {XOP}.
    kIdVpcomud,                          //!< Instruction 'vpcomud' {XOP}.
    kIdVpcomuq,                          //!< Instruction 'vpcomuq' {XOP}.
    kIdVpcomuw,                          //!< Instruction 'vpcomuw' {XOP}.
    kIdVpcomw,                           //!< Instruction 'vpcomw' {XOP}.
    kIdVpconflictd,                      //!< Instruction 'vpconflictd' {AVX512_CD+VL}.
    kIdVpconflictq,                      //!< Instruction 'vpconflictq' {AVX512_CD+VL}.
    kIdVpdpbssd,                         //!< Instruction 'vpdpbssd' {AVX_VNNI_INT8}.
    kIdVpdpbssds,                        //!< Instruction 'vpdpbssds' {AVX_VNNI_INT8}.
    kIdVpdpbsud,                         //!< Instruction 'vpdpbsud' {AVX_VNNI_INT8}.
    kIdVpdpbsuds,                        //!< Instruction 'vpdpbsuds' {AVX_VNNI_INT8}.
    kIdVpdpbusd,                         //!< Instruction 'vpdpbusd' {AVX_VNNI|AVX512_VNNI+VL}.
    kIdVpdpbusds,                        //!< Instruction 'vpdpbusds' {AVX_VNNI|AVX512_VNNI+VL}.
    kIdVpdpbuud,                         //!< Instruction 'vpdpbuud' {AVX_VNNI_INT8}.
    kIdVpdpbuuds,                        //!< Instruction 'vpdpbuuds' {AVX_VNNI_INT8}.
    kIdVpdpwssd,                         //!< Instruction 'vpdpwssd' {AVX_VNNI|AVX512_VNNI+VL}.
    kIdVpdpwssds,                        //!< Instruction 'vpdpwssds' {AVX_VNNI|AVX512_VNNI+VL}.
    kIdVpdpwsud,                         //!< Instruction 'vpdpwsud' {AVX_VNNI_INT16}.
    kIdVpdpwsuds,                        //!< Instruction 'vpdpwsuds' {AVX_VNNI_INT16}.
    kIdVpdpwusd,                         //!< Instruction 'vpdpwusd' {AVX_VNNI_INT16}.
    kIdVpdpwusds,                        //!< Instruction 'vpdpwusds' {AVX_VNNI_INT16}.
    kIdVpdpwuud,                         //!< Instruction 'vpdpwuud' {AVX_VNNI_INT16}.
    kIdVpdpwuuds,                        //!< Instruction 'vpdpwuuds' {AVX_VNNI_INT16}.
    kIdVperm2f128,                       //!< Instruction 'vperm2f128' {AVX}.
    kIdVperm2i128,                       //!< Instruction 'vperm2i128' {AVX2}.
    kIdVpermb,                           //!< Instruction 'vpermb' {AVX512_VBMI+VL}.
    kIdVpermd,                           //!< Instruction 'vpermd' {AVX2|AVX512_F+VL}.
    kIdVpermi2b,                         //!< Instruction 'vpermi2b' {AVX512_VBMI+VL}.
    kIdVpermi2d,                         //!< Instruction 'vpermi2d' {AVX512_F+VL}.
    kIdVpermi2pd,                        //!< Instruction 'vpermi2pd' {AVX512_F+VL}.
    kIdVpermi2ps,                        //!< Instruction 'vpermi2ps' {AVX512_F+VL}.
    kIdVpermi2q,                         //!< Instruction 'vpermi2q' {AVX512_F+VL}.
    kIdVpermi2w,                         //!< Instruction 'vpermi2w' {AVX512_BW+VL}.
    kIdVpermil2pd,                       //!< Instruction 'vpermil2pd' {XOP}.
    kIdVpermil2ps,                       //!< Instruction 'vpermil2ps' {XOP}.
    kIdVpermilpd,                        //!< Instruction 'vpermilpd' {AVX|AVX512_F+VL}.
    kIdVpermilps,                        //!< Instruction 'vpermilps' {AVX|AVX512_F+VL}.
    kIdVpermpd,                          //!< Instruction 'vpermpd' {AVX2|AVX512_F+VL}.
    kIdVpermps,                          //!< Instruction 'vpermps' {AVX2|AVX512_F+VL}.
    kIdVpermq,                           //!< Instruction 'vpermq' {AVX2|AVX512_F+VL}.
    kIdVpermt2b,                         //!< Instruction 'vpermt2b' {AVX512_VBMI+VL}.
    kIdVpermt2d,                         //!< Instruction 'vpermt2d' {AVX512_F+VL}.
    kIdVpermt2pd,                        //!< Instruction 'vpermt2pd' {AVX512_F+VL}.
    kIdVpermt2ps,                        //!< Instruction 'vpermt2ps' {AVX512_F+VL}.
    kIdVpermt2q,                         //!< Instruction 'vpermt2q' {AVX512_F+VL}.
    kIdVpermt2w,                         //!< Instruction 'vpermt2w' {AVX512_BW+VL}.
    kIdVpermw,                           //!< Instruction 'vpermw' {AVX512_BW+VL}.
    kIdVpexpandb,                        //!< Instruction 'vpexpandb' {AVX512_VBMI2+VL}.
    kIdVpexpandd,                        //!< Instruction 'vpexpandd' {AVX512_F+VL}.
    kIdVpexpandq,                        //!< Instruction 'vpexpandq' {AVX512_F+VL}.
    kIdVpexpandw,                        //!< Instruction 'vpexpandw' {AVX512_VBMI2+VL}.
    kIdVpextrb,                          //!< Instruction 'vpextrb' {AVX|AVX512_BW+VL}.
    kIdVpextrd,                          //!< Instruction 'vpextrd' {AVX|AVX512_DQ+VL}.
    kIdVpextrq,                          //!< Instruction 'vpextrq' {AVX|AVX512_DQ+VL} (X64).
    kIdVpextrw,                          //!< Instruction 'vpextrw' {AVX|AVX512_BW+VL}.
    kIdVpgatherdd,                       //!< Instruction 'vpgatherdd' {AVX2|AVX512_F+VL}.
    kIdVpgatherdq,                       //!< Instruction 'vpgatherdq' {AVX2|AVX512_F+VL}.
    kIdVpgatherqd,                       //!< Instruction 'vpgatherqd' {AVX2|AVX512_F+VL}.
    kIdVpgatherqq,                       //!< Instruction 'vpgatherqq' {AVX2|AVX512_F+VL}.
    kIdVphaddbd,                         //!< Instruction 'vphaddbd' {XOP}.
    kIdVphaddbq,                         //!< Instruction 'vphaddbq' {XOP}.
    kIdVphaddbw,                         //!< Instruction 'vphaddbw' {XOP}.
    kIdVphaddd,                          //!< Instruction 'vphaddd' {AVX|AVX2}.
    kIdVphadddq,                         //!< Instruction 'vphadddq' {XOP}.
    kIdVphaddsw,                         //!< Instruction 'vphaddsw' {AVX|AVX2}.
    kIdVphaddubd,                        //!< Instruction 'vphaddubd' {XOP}.
    kIdVphaddubq,                        //!< Instruction 'vphaddubq' {XOP}.
    kIdVphaddubw,                        //!< Instruction 'vphaddubw' {XOP}.
    kIdVphaddudq,                        //!< Instruction 'vphaddudq' {XOP}.
    kIdVphadduwd,                        //!< Instruction 'vphadduwd' {XOP}.
    kIdVphadduwq,                        //!< Instruction 'vphadduwq' {XOP}.
    kIdVphaddw,                          //!< Instruction 'vphaddw' {AVX|AVX2}.
    kIdVphaddwd,                         //!< Instruction 'vphaddwd' {XOP}.
    kIdVphaddwq,                         //!< Instruction 'vphaddwq' {XOP}.
    kIdVphminposuw,                      //!< Instruction 'vphminposuw' {AVX}.
    kIdVphsubbw,                         //!< Instruction 'vphsubbw' {XOP}.
    kIdVphsubd,                          //!< Instruction 'vphsubd' {AVX|AVX2}.
    kIdVphsubdq,                         //!< Instruction 'vphsubdq' {XOP}.
    kIdVphsubsw,                         //!< Instruction 'vphsubsw' {AVX|AVX2}.
    kIdVphsubw,                          //!< Instruction 'vphsubw' {AVX|AVX2}.
    kIdVphsubwd,                         //!< Instruction 'vphsubwd' {XOP}.
    kIdVpinsrb,                          //!< Instruction 'vpinsrb' {AVX|AVX512_BW+VL}.
    kIdVpinsrd,                          //!< Instruction 'vpinsrd' {AVX|AVX512_DQ+VL}.
    kIdVpinsrq,                          //!< Instruction 'vpinsrq' {AVX|AVX512_DQ+VL} (X64).
    kIdVpinsrw,                          //!< Instruction 'vpinsrw' {AVX|AVX512_BW+VL}.
    kIdVplzcntd,                         //!< Instruction 'vplzcntd' {AVX512_CD+VL}.
    kIdVplzcntq,                         //!< Instruction 'vplzcntq' {AVX512_CD+VL}.
    kIdVpmacsdd,                         //!< Instruction 'vpmacsdd' {XOP}.
    kIdVpmacsdqh,                        //!< Instruction 'vpmacsdqh' {XOP}.
    kIdVpmacsdql,                        //!< Instruction 'vpmacsdql' {XOP}.
    kIdVpmacssdd,                        //!< Instruction 'vpmacssdd' {XOP}.
    kIdVpmacssdqh,                       //!< Instruction 'vpmacssdqh' {XOP}.
    kIdVpmacssdql,                       //!< Instruction 'vpmacssdql' {XOP}.
    kIdVpmacsswd,                        //!< Instruction 'vpmacsswd' {XOP}.
    kIdVpmacssww,                        //!< Instruction 'vpmacssww' {XOP}.
    kIdVpmacswd,                         //!< Instruction 'vpmacswd' {XOP}.
    kIdVpmacsww,                         //!< Instruction 'vpmacsww' {XOP}.
    kIdVpmadcsswd,                       //!< Instruction 'vpmadcsswd' {XOP}.
    kIdVpmadcswd,                        //!< Instruction 'vpmadcswd' {XOP}.
    kIdVpmadd52huq,                      //!< Instruction 'vpmadd52huq' {AVX_IFMA|AVX512_IFMA+VL}.
    kIdVpmadd52luq,                      //!< Instruction 'vpmadd52luq' {AVX_IFMA|AVX512_IFMA+VL}.
    kIdVpmaddubsw,                       //!< Instruction 'vpmaddubsw' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpmaddwd,                         //!< Instruction 'vpmaddwd' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpmaskmovd,                       //!< Instruction 'vpmaskmovd' {AVX2}.
    kIdVpmaskmovq,                       //!< Instruction 'vpmaskmovq' {AVX2}.
    kIdVpmaxsb,                          //!< Instruction 'vpmaxsb' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpmaxsd,                          //!< Instruction 'vpmaxsd' {AVX|AVX2|AVX512_F+VL}.
    kIdVpmaxsq,                          //!< Instruction 'vpmaxsq' {AVX512_F+VL}.
    kIdVpmaxsw,                          //!< Instruction 'vpmaxsw' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpmaxub,                          //!< Instruction 'vpmaxub' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpmaxud,                          //!< Instruction 'vpmaxud' {AVX|AVX2|AVX512_F+VL}.
    kIdVpmaxuq,                          //!< Instruction 'vpmaxuq' {AVX512_F+VL}.
    kIdVpmaxuw,                          //!< Instruction 'vpmaxuw' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpminsb,                          //!< Instruction 'vpminsb' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpminsd,                          //!< Instruction 'vpminsd' {AVX|AVX2|AVX512_F+VL}.
    kIdVpminsq,                          //!< Instruction 'vpminsq' {AVX512_F+VL}.
    kIdVpminsw,                          //!< Instruction 'vpminsw' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpminub,                          //!< Instruction 'vpminub' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpminud,                          //!< Instruction 'vpminud' {AVX|AVX2|AVX512_F+VL}.
    kIdVpminuq,                          //!< Instruction 'vpminuq' {AVX512_F+VL}.
    kIdVpminuw,                          //!< Instruction 'vpminuw' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpmovb2m,                         //!< Instruction 'vpmovb2m' {AVX512_BW+VL}.
    kIdVpmovd2m,                         //!< Instruction 'vpmovd2m' {AVX512_DQ+VL}.
    kIdVpmovdb,                          //!< Instruction 'vpmovdb' {AVX512_F+VL}.
    kIdVpmovdw,                          //!< Instruction 'vpmovdw' {AVX512_F+VL}.
    kIdVpmovm2b,                         //!< Instruction 'vpmovm2b' {AVX512_BW+VL}.
    kIdVpmovm2d,                         //!< Instruction 'vpmovm2d' {AVX512_DQ+VL}.
    kIdVpmovm2q,                         //!< Instruction 'vpmovm2q' {AVX512_DQ+VL}.
    kIdVpmovm2w,                         //!< Instruction 'vpmovm2w' {AVX512_BW+VL}.
    kIdVpmovmskb,                        //!< Instruction 'vpmovmskb' {AVX|AVX2}.
    kIdVpmovq2m,                         //!< Instruction 'vpmovq2m' {AVX512_DQ+VL}.
    kIdVpmovqb,                          //!< Instruction 'vpmovqb' {AVX512_F+VL}.
    kIdVpmovqd,                          //!< Instruction 'vpmovqd' {AVX512_F+VL}.
    kIdVpmovqw,                          //!< Instruction 'vpmovqw' {AVX512_F+VL}.
    kIdVpmovsdb,                         //!< Instruction 'vpmovsdb' {AVX512_F+VL}.
    kIdVpmovsdw,                         //!< Instruction 'vpmovsdw' {AVX512_F+VL}.
    kIdVpmovsqb,                         //!< Instruction 'vpmovsqb' {AVX512_F+VL}.
    kIdVpmovsqd,                         //!< Instruction 'vpmovsqd' {AVX512_F+VL}.
    kIdVpmovsqw,                         //!< Instruction 'vpmovsqw' {AVX512_F+VL}.
    kIdVpmovswb,                         //!< Instruction 'vpmovswb' {AVX512_BW+VL}.
    kIdVpmovsxbd,                        //!< Instruction 'vpmovsxbd' {AVX|AVX2|AVX512_F+VL}.
    kIdVpmovsxbq,                        //!< Instruction 'vpmovsxbq' {AVX|AVX2|AVX512_F+VL}.
    kIdVpmovsxbw,                        //!< Instruction 'vpmovsxbw' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpmovsxdq,                        //!< Instruction 'vpmovsxdq' {AVX|AVX2|AVX512_F+VL}.
    kIdVpmovsxwd,                        //!< Instruction 'vpmovsxwd' {AVX|AVX2|AVX512_F+VL}.
    kIdVpmovsxwq,                        //!< Instruction 'vpmovsxwq' {AVX|AVX2|AVX512_F+VL}.
    kIdVpmovusdb,                        //!< Instruction 'vpmovusdb' {AVX512_F+VL}.
    kIdVpmovusdw,                        //!< Instruction 'vpmovusdw' {AVX512_F+VL}.
    kIdVpmovusqb,                        //!< Instruction 'vpmovusqb' {AVX512_F+VL}.
    kIdVpmovusqd,                        //!< Instruction 'vpmovusqd' {AVX512_F+VL}.
    kIdVpmovusqw,                        //!< Instruction 'vpmovusqw' {AVX512_F+VL}.
    kIdVpmovuswb,                        //!< Instruction 'vpmovuswb' {AVX512_BW+VL}.
    kIdVpmovw2m,                         //!< Instruction 'vpmovw2m' {AVX512_BW+VL}.
    kIdVpmovwb,                          //!< Instruction 'vpmovwb' {AVX512_BW+VL}.
    kIdVpmovzxbd,                        //!< Instruction 'vpmovzxbd' {AVX|AVX2|AVX512_F+VL}.
    kIdVpmovzxbq,                        //!< Instruction 'vpmovzxbq' {AVX|AVX2|AVX512_F+VL}.
    kIdVpmovzxbw,                        //!< Instruction 'vpmovzxbw' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpmovzxdq,                        //!< Instruction 'vpmovzxdq' {AVX|AVX2|AVX512_F+VL}.
    kIdVpmovzxwd,                        //!< Instruction 'vpmovzxwd' {AVX|AVX2|AVX512_F+VL}.
    kIdVpmovzxwq,                        //!< Instruction 'vpmovzxwq' {AVX|AVX2|AVX512_F+VL}.
    kIdVpmuldq,                          //!< Instruction 'vpmuldq' {AVX|AVX2|AVX512_F+VL}.
    kIdVpmulhrsw,                        //!< Instruction 'vpmulhrsw' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpmulhuw,                         //!< Instruction 'vpmulhuw' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpmulhw,                          //!< Instruction 'vpmulhw' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpmulld,                          //!< Instruction 'vpmulld' {AVX|AVX2|AVX512_F+VL}.
    kIdVpmullq,                          //!< Instruction 'vpmullq' {AVX512_DQ+VL}.
    kIdVpmullw,                          //!< Instruction 'vpmullw' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpmultishiftqb,                   //!< Instruction 'vpmultishiftqb' {AVX512_VBMI+VL}.
    kIdVpmuludq,                         //!< Instruction 'vpmuludq' {AVX|AVX2|AVX512_F+VL}.
    kIdVpopcntb,                         //!< Instruction 'vpopcntb' {AVX512_BITALG+VL}.
    kIdVpopcntd,                         //!< Instruction 'vpopcntd' {AVX512_VPOPCNTDQ+VL}.
    kIdVpopcntq,                         //!< Instruction 'vpopcntq' {AVX512_VPOPCNTDQ+VL}.
    kIdVpopcntw,                         //!< Instruction 'vpopcntw' {AVX512_BITALG+VL}.
    kIdVpor,                             //!< Instruction 'vpor' {AVX|AVX2}.
    kIdVpord,                            //!< Instruction 'vpord' {AVX512_F+VL}.
    kIdVporq,                            //!< Instruction 'vporq' {AVX512_F+VL}.
    kIdVpperm,                           //!< Instruction 'vpperm' {XOP}.
    kIdVprold,                           //!< Instruction 'vprold' {AVX512_F+VL}.
    kIdVprolq,                           //!< Instruction 'vprolq' {AVX512_F+VL}.
    kIdVprolvd,                          //!< Instruction 'vprolvd' {AVX512_F+VL}.
    kIdVprolvq,                          //!< Instruction 'vprolvq' {AVX512_F+VL}.
    kIdVprord,                           //!< Instruction 'vprord' {AVX512_F+VL}.
    kIdVprorq,                           //!< Instruction 'vprorq' {AVX512_F+VL}.
    kIdVprorvd,                          //!< Instruction 'vprorvd' {AVX512_F+VL}.
    kIdVprorvq,                          //!< Instruction 'vprorvq' {AVX512_F+VL}.
    kIdVprotb,                           //!< Instruction 'vprotb' {XOP}.
    kIdVprotd,                           //!< Instruction 'vprotd' {XOP}.
    kIdVprotq,                           //!< Instruction 'vprotq' {XOP}.
    kIdVprotw,                           //!< Instruction 'vprotw' {XOP}.
    kIdVpsadbw,                          //!< Instruction 'vpsadbw' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpscatterdd,                      //!< Instruction 'vpscatterdd' {AVX512_F+VL}.
    kIdVpscatterdq,                      //!< Instruction 'vpscatterdq' {AVX512_F+VL}.
    kIdVpscatterqd,                      //!< Instruction 'vpscatterqd' {AVX512_F+VL}.
    kIdVpscatterqq,                      //!< Instruction 'vpscatterqq' {AVX512_F+VL}.
    kIdVpshab,                           //!< Instruction 'vpshab' {XOP}.
    kIdVpshad,                           //!< Instruction 'vpshad' {XOP}.
    kIdVpshaq,                           //!< Instruction 'vpshaq' {XOP}.
    kIdVpshaw,                           //!< Instruction 'vpshaw' {XOP}.
    kIdVpshlb,                           //!< Instruction 'vpshlb' {XOP}.
    kIdVpshld,                           //!< Instruction 'vpshld' {XOP}.
    kIdVpshldd,                          //!< Instruction 'vpshldd' {AVX512_VBMI2+VL}.
    kIdVpshldq,                          //!< Instruction 'vpshldq' {AVX512_VBMI2+VL}.
    kIdVpshldvd,                         //!< Instruction 'vpshldvd' {AVX512_VBMI2+VL}.
    kIdVpshldvq,                         //!< Instruction 'vpshldvq' {AVX512_VBMI2+VL}.
    kIdVpshldvw,                         //!< Instruction 'vpshldvw' {AVX512_VBMI2+VL}.
    kIdVpshldw,                          //!< Instruction 'vpshldw' {AVX512_VBMI2+VL}.
    kIdVpshlq,                           //!< Instruction 'vpshlq' {XOP}.
    kIdVpshlw,                           //!< Instruction 'vpshlw' {XOP}.
    kIdVpshrdd,                          //!< Instruction 'vpshrdd' {AVX512_VBMI2+VL}.
    kIdVpshrdq,                          //!< Instruction 'vpshrdq' {AVX512_VBMI2+VL}.
    kIdVpshrdvd,                         //!< Instruction 'vpshrdvd' {AVX512_VBMI2+VL}.
    kIdVpshrdvq,                         //!< Instruction 'vpshrdvq' {AVX512_VBMI2+VL}.
    kIdVpshrdvw,                         //!< Instruction 'vpshrdvw' {AVX512_VBMI2+VL}.
    kIdVpshrdw,                          //!< Instruction 'vpshrdw' {AVX512_VBMI2+VL}.
    kIdVpshufb,                          //!< Instruction 'vpshufb' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpshufbitqmb,                     //!< Instruction 'vpshufbitqmb' {AVX512_BITALG+VL}.
    kIdVpshufd,                          //!< Instruction 'vpshufd' {AVX|AVX2|AVX512_F+VL}.
    kIdVpshufhw,                         //!< Instruction 'vpshufhw' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpshuflw,                         //!< Instruction 'vpshuflw' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpsignb,                          //!< Instruction 'vpsignb' {AVX|AVX2}.
    kIdVpsignd,                          //!< Instruction 'vpsignd' {AVX|AVX2}.
    kIdVpsignw,                          //!< Instruction 'vpsignw' {AVX|AVX2}.
    kIdVpslld,                           //!< Instruction 'vpslld' {AVX|AVX2|AVX512_F+VL}.
    kIdVpslldq,                          //!< Instruction 'vpslldq' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpsllq,                           //!< Instruction 'vpsllq' {AVX|AVX2|AVX512_F+VL}.
    kIdVpsllvd,                          //!< Instruction 'vpsllvd' {AVX2|AVX512_F+VL}.
    kIdVpsllvq,                          //!< Instruction 'vpsllvq' {AVX2|AVX512_F+VL}.
    kIdVpsllvw,                          //!< Instruction 'vpsllvw' {AVX512_BW+VL}.
    kIdVpsllw,                           //!< Instruction 'vpsllw' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpsrad,                           //!< Instruction 'vpsrad' {AVX|AVX2|AVX512_F+VL}.
    kIdVpsraq,                           //!< Instruction 'vpsraq' {AVX512_F+VL}.
    kIdVpsravd,                          //!< Instruction 'vpsravd' {AVX2|AVX512_F+VL}.
    kIdVpsravq,                          //!< Instruction 'vpsravq' {AVX512_F+VL}.
    kIdVpsravw,                          //!< Instruction 'vpsravw' {AVX512_BW+VL}.
    kIdVpsraw,                           //!< Instruction 'vpsraw' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpsrld,                           //!< Instruction 'vpsrld' {AVX|AVX2|AVX512_F+VL}.
    kIdVpsrldq,                          //!< Instruction 'vpsrldq' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpsrlq,                           //!< Instruction 'vpsrlq' {AVX|AVX2|AVX512_F+VL}.
    kIdVpsrlvd,                          //!< Instruction 'vpsrlvd' {AVX2|AVX512_F+VL}.
    kIdVpsrlvq,                          //!< Instruction 'vpsrlvq' {AVX2|AVX512_F+VL}.
    kIdVpsrlvw,                          //!< Instruction 'vpsrlvw' {AVX512_BW+VL}.
    kIdVpsrlw,                           //!< Instruction 'vpsrlw' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpsubb,                           //!< Instruction 'vpsubb' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpsubd,                           //!< Instruction 'vpsubd' {AVX|AVX2|AVX512_F+VL}.
    kIdVpsubq,                           //!< Instruction 'vpsubq' {AVX|AVX2|AVX512_F+VL}.
    kIdVpsubsb,                          //!< Instruction 'vpsubsb' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpsubsw,                          //!< Instruction 'vpsubsw' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpsubusb,                         //!< Instruction 'vpsubusb' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpsubusw,                         //!< Instruction 'vpsubusw' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpsubw,                           //!< Instruction 'vpsubw' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpternlogd,                       //!< Instruction 'vpternlogd' {AVX512_F+VL}.
    kIdVpternlogq,                       //!< Instruction 'vpternlogq' {AVX512_F+VL}.
    kIdVptest,                           //!< Instruction 'vptest' {AVX}.
    kIdVptestmb,                         //!< Instruction 'vptestmb' {AVX512_BW+VL}.
    kIdVptestmd,                         //!< Instruction 'vptestmd' {AVX512_F+VL}.
    kIdVptestmq,                         //!< Instruction 'vptestmq' {AVX512_F+VL}.
    kIdVptestmw,                         //!< Instruction 'vptestmw' {AVX512_BW+VL}.
    kIdVptestnmb,                        //!< Instruction 'vptestnmb' {AVX512_BW+VL}.
    kIdVptestnmd,                        //!< Instruction 'vptestnmd' {AVX512_F+VL}.
    kIdVptestnmq,                        //!< Instruction 'vptestnmq' {AVX512_F+VL}.
    kIdVptestnmw,                        //!< Instruction 'vptestnmw' {AVX512_BW+VL}.
    kIdVpunpckhbw,                       //!< Instruction 'vpunpckhbw' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpunpckhdq,                       //!< Instruction 'vpunpckhdq' {AVX|AVX2|AVX512_F+VL}.
    kIdVpunpckhqdq,                      //!< Instruction 'vpunpckhqdq' {AVX|AVX2|AVX512_F+VL}.
    kIdVpunpckhwd,                       //!< Instruction 'vpunpckhwd' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpunpcklbw,                       //!< Instruction 'vpunpcklbw' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpunpckldq,                       //!< Instruction 'vpunpckldq' {AVX|AVX2|AVX512_F+VL}.
    kIdVpunpcklqdq,                      //!< Instruction 'vpunpcklqdq' {AVX|AVX2|AVX512_F+VL}.
    kIdVpunpcklwd,                       //!< Instruction 'vpunpcklwd' {AVX|AVX2|AVX512_BW+VL}.
    kIdVpxor,                            //!< Instruction 'vpxor' {AVX|AVX2}.
    kIdVpxord,                           //!< Instruction 'vpxord' {AVX512_F+VL}.
    kIdVpxorq,                           //!< Instruction 'vpxorq' {AVX512_F+VL}.
    kIdVrangepd,                         //!< Instruction 'vrangepd' {AVX512_DQ+VL}.
    kIdVrangeps,                         //!< Instruction 'vrangeps' {AVX512_DQ+VL}.
    kIdVrangesd,                         //!< Instruction 'vrangesd' {AVX512_DQ+VL}.
    kIdVrangess,                         //!< Instruction 'vrangess' {AVX512_DQ+VL}.
    kIdVrcp14pd,                         //!< Instruction 'vrcp14pd' {AVX512_F+VL}.
    kIdVrcp14ps,                         //!< Instruction 'vrcp14ps' {AVX512_F+VL}.
    kIdVrcp14sd,                         //!< Instruction 'vrcp14sd' {AVX512_F+VL}.
    kIdVrcp14ss,                         //!< Instruction 'vrcp14ss' {AVX512_F+VL}.
    kIdVrcpph,                           //!< Instruction 'vrcpph' {AVX512_FP16+VL}.
    kIdVrcpps,                           //!< Instruction 'vrcpps' {AVX}.
    kIdVrcpsh,                           //!< Instruction 'vrcpsh' {AVX512_FP16+VL}.
    kIdVrcpss,                           //!< Instruction 'vrcpss' {AVX}.
    kIdVreducepd,                        //!< Instruction 'vreducepd' {AVX512_DQ+VL}.
    kIdVreduceph,                        //!< Instruction 'vreduceph' {AVX512_FP16+VL}.
    kIdVreduceps,                        //!< Instruction 'vreduceps' {AVX512_DQ+VL}.
    kIdVreducesd,                        //!< Instruction 'vreducesd' {AVX512_DQ+VL}.
    kIdVreducesh,                        //!< Instruction 'vreducesh' {AVX512_FP16+VL}.
    kIdVreducess,                        //!< Instruction 'vreducess' {AVX512_DQ+VL}.
    kIdVrndscalepd,                      //!< Instruction 'vrndscalepd' {AVX512_F+VL}.
    kIdVrndscaleph,                      //!< Instruction 'vrndscaleph' {AVX512_FP16+VL}.
    kIdVrndscaleps,                      //!< Instruction 'vrndscaleps' {AVX512_F+VL}.
    kIdVrndscalesd,                      //!< Instruction 'vrndscalesd' {AVX512_F+VL}.
    kIdVrndscalesh,                      //!< Instruction 'vrndscalesh' {AVX512_FP16+VL}.
    kIdVrndscaless,                      //!< Instruction 'vrndscaless' {AVX512_F+VL}.
    kIdVroundpd,                         //!< Instruction 'vroundpd' {AVX}.
    kIdVroundps,                         //!< Instruction 'vroundps' {AVX}.
    kIdVroundsd,                         //!< Instruction 'vroundsd' {AVX}.
    kIdVroundss,                         //!< Instruction 'vroundss' {AVX}.
    kIdVrsqrt14pd,                       //!< Instruction 'vrsqrt14pd' {AVX512_F+VL}.
    kIdVrsqrt14ps,                       //!< Instruction 'vrsqrt14ps' {AVX512_F+VL}.
    kIdVrsqrt14sd,                       //!< Instruction 'vrsqrt14sd' {AVX512_F+VL}.
    kIdVrsqrt14ss,                       //!< Instruction 'vrsqrt14ss' {AVX512_F+VL}.
    kIdVrsqrtph,                         //!< Instruction 'vrsqrtph' {AVX512_FP16+VL}.
    kIdVrsqrtps,                         //!< Instruction 'vrsqrtps' {AVX}.
    kIdVrsqrtsh,                         //!< Instruction 'vrsqrtsh' {AVX512_FP16+VL}.
    kIdVrsqrtss,                         //!< Instruction 'vrsqrtss' {AVX}.
    kIdVscalefpd,                        //!< Instruction 'vscalefpd' {AVX512_F+VL}.
    kIdVscalefph,                        //!< Instruction 'vscalefph' {AVX512_FP16+VL}.
    kIdVscalefps,                        //!< Instruction 'vscalefps' {AVX512_F+VL}.
    kIdVscalefsd,                        //!< Instruction 'vscalefsd' {AVX512_F+VL}.
    kIdVscalefsh,                        //!< Instruction 'vscalefsh' {AVX512_FP16+VL}.
    kIdVscalefss,                        //!< Instruction 'vscalefss' {AVX512_F+VL}.
    kIdVscatterdpd,                      //!< Instruction 'vscatterdpd' {AVX512_F+VL}.
    kIdVscatterdps,                      //!< Instruction 'vscatterdps' {AVX512_F+VL}.
    kIdVscatterqpd,                      //!< Instruction 'vscatterqpd' {AVX512_F+VL}.
    kIdVscatterqps,                      //!< Instruction 'vscatterqps' {AVX512_F+VL}.
    kIdVsha512msg1,                      //!< Instruction 'vsha512msg1' {AVX & SHA512}.
    kIdVsha512msg2,                      //!< Instruction 'vsha512msg2' {AVX & SHA512}.
    kIdVsha512rnds2,                     //!< Instruction 'vsha512rnds2' {AVX & SHA512}.
    kIdVshuff32x4,                       //!< Instruction 'vshuff32x4' {AVX512_F+VL}.
    kIdVshuff64x2,                       //!< Instruction 'vshuff64x2' {AVX512_F+VL}.
    kIdVshufi32x4,                       //!< Instruction 'vshufi32x4' {AVX512_F+VL}.
    kIdVshufi64x2,                       //!< Instruction 'vshufi64x2' {AVX512_F+VL}.
    kIdVshufpd,                          //!< Instruction 'vshufpd' {AVX|AVX512_F+VL}.
    kIdVshufps,                          //!< Instruction 'vshufps' {AVX|AVX512_F+VL}.
    kIdVsm3msg1,                         //!< Instruction 'vsm3msg1' {AVX & SM3}.
    kIdVsm3msg2,                         //!< Instruction 'vsm3msg2' {AVX & SM3}.
    kIdVsm3rnds2,                        //!< Instruction 'vsm3rnds2' {AVX & SM3}.
    kIdVsm4key4,                         //!< Instruction 'vsm4key4' {AVX & SM4}.
    kIdVsm4rnds4,                        //!< Instruction 'vsm4rnds4' {AVX & SM4}.
    kIdVsqrtpd,                          //!< Instruction 'vsqrtpd' {AVX|AVX512_F+VL}.
    kIdVsqrtph,                          //!< Instruction 'vsqrtph' {AVX512_FP16+VL}.
    kIdVsqrtps,                          //!< Instruction 'vsqrtps' {AVX|AVX512_F+VL}.
    kIdVsqrtsd,                          //!< Instruction 'vsqrtsd' {AVX|AVX512_F+VL}.
    kIdVsqrtsh,                          //!< Instruction 'vsqrtsh' {AVX512_FP16+VL}.
    kIdVsqrtss,                          //!< Instruction 'vsqrtss' {AVX|AVX512_F+VL}.
    kIdVstmxcsr,                         //!< Instruction 'vstmxcsr' {AVX}.
    kIdVsubpd,                           //!< Instruction 'vsubpd' {AVX|AVX512_F+VL}.
    kIdVsubph,                           //!< Instruction 'vsubph' {AVX512_FP16+VL}.
    kIdVsubps,                           //!< Instruction 'vsubps' {AVX|AVX512_F+VL}.
    kIdVsubsd,                           //!< Instruction 'vsubsd' {AVX|AVX512_F+VL}.
    kIdVsubsh,                           //!< Instruction 'vsubsh' {AVX512_FP16+VL}.
    kIdVsubss,                           //!< Instruction 'vsubss' {AVX|AVX512_F+VL}.
    kIdVtestpd,                          //!< Instruction 'vtestpd' {AVX}.
    kIdVtestps,                          //!< Instruction 'vtestps' {AVX}.
    kIdVucomisd,                         //!< Instruction 'vucomisd' {AVX|AVX512_F+VL}.
    kIdVucomish,                         //!< Instruction 'vucomish' {AVX512_FP16+VL}.
    kIdVucomiss,                         //!< Instruction 'vucomiss' {AVX|AVX512_F+VL}.
    kIdVunpckhpd,                        //!< Instruction 'vunpckhpd' {AVX|AVX512_F+VL}.
    kIdVunpckhps,                        //!< Instruction 'vunpckhps' {AVX|AVX512_F+VL}.
    kIdVunpcklpd,                        //!< Instruction 'vunpcklpd' {AVX|AVX512_F+VL}.
    kIdVunpcklps,                        //!< Instruction 'vunpcklps' {AVX|AVX512_F+VL}.
    kIdVxorpd,                           //!< Instruction 'vxorpd' {AVX|AVX512_DQ+VL}.
    kIdVxorps,                           //!< Instruction 'vxorps' {AVX|AVX512_DQ+VL}.
    kIdVzeroall,                         //!< Instruction 'vzeroall' {AVX}.
    kIdVzeroupper,                       //!< Instruction 'vzeroupper' {AVX}.
    kIdWbinvd,                           //!< Instruction 'wbinvd' {I486}.
    kIdWbnoinvd,                         //!< Instruction 'wbnoinvd' {WBNOINVD}.
    kIdWrfsbase,                         //!< Instruction 'wrfsbase' {FSGSBASE} (X64).
    kIdWrgsbase,                         //!< Instruction 'wrgsbase' {FSGSBASE} (X64).
    kIdWrmsr,                            //!< Instruction 'wrmsr' {MSR}.
    kIdWrssd,                            //!< Instruction 'wrssd' {CET_SS}.
    kIdWrssq,                            //!< Instruction 'wrssq' {CET_SS} (X64).
    kIdWrussd,                           //!< Instruction 'wrussd' {CET_SS}.
    kIdWrussq,                           //!< Instruction 'wrussq' {CET_SS} (X64).
    kIdXabort,                           //!< Instruction 'xabort' {RTM}.
    kIdXadd,                             //!< Instruction 'xadd' {I486}.
    kIdXbegin,                           //!< Instruction 'xbegin' {RTM}.
    kIdXchg,                             //!< Instruction 'xchg'.
    kIdXend,                             //!< Instruction 'xend' {RTM}.
    kIdXgetbv,                           //!< Instruction 'xgetbv' {XSAVE}.
    kIdXlatb,                            //!< Instruction 'xlatb'.
    kIdXor,                              //!< Instruction 'xor'.
    kIdXorpd,                            //!< Instruction 'xorpd' {SSE2}.
    kIdXorps,                            //!< Instruction 'xorps' {SSE}.
    kIdXresldtrk,                        //!< Instruction 'xresldtrk' {TSXLDTRK}.
    kIdXrstor,                           //!< Instruction 'xrstor' {XSAVE}.
    kIdXrstor64,                         //!< Instruction 'xrstor64' {XSAVE} (X64).
    kIdXrstors,                          //!< Instruction 'xrstors' {XSAVES}.
    kIdXrstors64,                        //!< Instruction 'xrstors64' {XSAVES} (X64).
    kIdXsave,                            //!< Instruction 'xsave' {XSAVE}.
    kIdXsave64,                          //!< Instruction 'xsave64' {XSAVE} (X64).
    kIdXsavec,                           //!< Instruction 'xsavec' {XSAVEC}.
    kIdXsavec64,                         //!< Instruction 'xsavec64' {XSAVEC} (X64).
    kIdXsaveopt,                         //!< Instruction 'xsaveopt' {XSAVEOPT}.
    kIdXsaveopt64,                       //!< Instruction 'xsaveopt64' {XSAVEOPT} (X64).
    kIdXsaves,                           //!< Instruction 'xsaves' {XSAVES}.
    kIdXsaves64,                         //!< Instruction 'xsaves64' {XSAVES} (X64).
    kIdXsetbv,                           //!< Instruction 'xsetbv' {XSAVE}.
    kIdXsusldtrk,                        //!< Instruction 'xsusldtrk' {TSXLDTRK}.
    kIdXtest,                            //!< Instruction 'xtest' {RTM}.
    _kIdCount,

    // Aliases.
    kIdCmovnae = kIdCmovb,
    kIdCmovc = kIdCmovb,
    kIdCmovna = kIdCmovbe,
    kIdCmovnge = kIdCmovl,
    kIdCmovng = kIdCmovle,
    kIdCmovae = kIdCmovnb,
    kIdCmovnc = kIdCmovnb,
    kIdCmova = kIdCmovnbe,
    kIdCmovge = kIdCmovnl,
    kIdCmovg = kIdCmovnle,
    kIdCmovpo = kIdCmovnp,
    kIdCmovne = kIdCmovnz,
    kIdCmovpe = kIdCmovp,
    kIdCmove = kIdCmovz,
    kIdWait = kIdFwait,
    kIdJnae = kIdJb,
    kIdJc = kIdJb,
    kIdJna = kIdJbe,
    kIdJnge = kIdJl,
    kIdJng = kIdJle,
    kIdJae = kIdJnb,
    kIdJnc = kIdJnb,
    kIdJa = kIdJnbe,
    kIdJge = kIdJnl,
    kIdJg = kIdJnle,
    kIdJpo = kIdJnp,
    kIdJne = kIdJnz,
    kIdJpe = kIdJp,
    kIdJe = kIdJz,
    kIdSetnae = kIdSetb,
    kIdSetc = kIdSetb,
    kIdSetna = kIdSetbe,
    kIdSetnge = kIdSetl,
    kIdSetng = kIdSetle,
    kIdSetae = kIdSetnb,
    kIdSetnc = kIdSetnb,
    kIdSeta = kIdSetnbe,
    kIdSetge = kIdSetnl,
    kIdSetg = kIdSetnle,
    kIdSetpo = kIdSetnp,
    kIdSetne = kIdSetnz,
    kIdSetpe = kIdSetp,
    kIdSete = kIdSetz,
    kIdSal = kIdShl
    // ${InstId:End}
  };

  //! Tests whether the `inst_id` is defined.
  static ASMJIT_INLINE_CONSTEXPR bool is_defined_id(InstId inst_id) noexcept { return inst_id < _kIdCount; }

  //! \cond
  #define ASMJIT_INST_FROM_COND(ID) \
    ID##o, ID##no, ID##b , ID##nb , \
    ID##z, ID##nz, ID##be, ID##nbe, \
    ID##s, ID##ns, ID##p , ID##np , \
    ID##l, ID##nl, ID##le, ID##nle

    static constexpr uint16_t _jcc_from_cond_table[] = { ASMJIT_INST_FROM_COND(Inst::kIdJ) };
    static constexpr uint16_t _setcc_from_cond_table[] = { ASMJIT_INST_FROM_COND(Inst::kIdSet) };
    static constexpr uint16_t _cmovcc_from_cond_table[] = { ASMJIT_INST_FROM_COND(Inst::kIdCmov) };

  #undef ASMJIT_INST_FROM_COND
  //! \endcond

  //! Translates a condition code `cond` to a `jcc` instruction id.
  static ASMJIT_INLINE_CONSTEXPR InstId jcc_from_cond(CondCode cond) noexcept { return _jcc_from_cond_table[uint8_t(cond)]; }
  //! Translates a condition code `cond` to a `setcc` instruction id.
  static ASMJIT_INLINE_CONSTEXPR InstId setcc_from_cond(CondCode cond) noexcept { return _setcc_from_cond_table[uint8_t(cond)]; }
  //! Translates a condition code `cond` to a `cmovcc` instruction id.
  static ASMJIT_INLINE_CONSTEXPR InstId cmovcc_from_cond(CondCode cond) noexcept { return _cmovcc_from_cond_table[uint8_t(cond)]; }
} // {Inst}

//! FPU status word bits.
enum class FpuStatusWord : uint16_t {
  kNone          = 0x0000u,     //!< No bits set.

  kInvalid       = 0x0001u,     //!< Invalid operation.
  kDenormalized  = 0x0002u,     //!< Denormalized operand.
  kDivByZero     = 0x0004u,     //!< Division by zero.
  kOverflow      = 0x0008u,     //!< Overflown.
  kUnderflow     = 0x0010u,     //!< Underflown.
  kPrecision     = 0x0020u,     //!< Precision lost.
  kStackFault    = 0x0040u,     //!< Stack fault.
  kInterrupt     = 0x0080u,     //!< Interrupt.
  kC0            = 0x0100u,     //!< C0 flag.
  kC1            = 0x0200u,     //!< C1 flag.
  kC2            = 0x0400u,     //!< C2 flag.
  kTopMask       = 0x3800u,     //!< Top of the stack (mask).
  kC3            = 0x4000u,     //!< C3 flag.
  kBusy          = 0x8000u      //!< FPU is busy.
};
ASMJIT_DEFINE_ENUM_FLAGS(FpuStatusWord)

//! FPU control word bits.
enum class FpuControlWord : uint16_t {
  kNone          = 0x0000u,     //!< No bits set.

  // Bits 0-5
  // --------

  kEM_Mask       = 0x003Fu,     //!< Exception mask (0x3F).
  kEM_Invalid    = 0x0001u,     //!< Invalid operation exception.
  kEM_Denormal   = 0x0002u,     //!< Denormalized operand exception.
  kEM_DivByZero  = 0x0004u,     //!< Division by zero exception.
  kEM_Overflow   = 0x0008u,     //!< Overflow exception.
  kEM_Underflow  = 0x0010u,     //!< Underflow exception.
  kEM_Inexact    = 0x0020u,     //!< Inexact operation exception.

  // Bits 8-9
  // --------

  kPC_Mask       = 0x0300u,     //!< Precision control mask.
  kPC_Float      = 0x0000u,     //!< Single precision (24 bits).
  kPC_Reserved   = 0x0100u,     //!< Reserved.
  kPC_Double     = 0x0200u,     //!< Double precision (53 bits).
  kPC_Extended   = 0x0300u,     //!< Extended precision (64 bits).

  // Bits 10-11
  // ----------

  kRC_Mask       = 0x0C00u,     //!< Rounding control mask.
  kRC_Nearest    = 0x0000u,     //!< Round to nearest even.
  kRC_Down       = 0x0400u,     //!< Round down (floor).
  kRC_Up         = 0x0800u,     //!< Round up (ceil).
  kRC_Truncate   = 0x0C00u,     //!< Round towards zero (truncate).

  // Bit 12
  // ------

  kIC_Mask       = 0x1000u,     //!< Infinity control.
  kIC_Projective = 0x0000u,     //!< Projective (not supported on X64).
  kIC_Affine     = 0x1000u      //!< Affine (default).
};
ASMJIT_DEFINE_ENUM_FLAGS(FpuControlWord)

//! An immediate value that can be used with CMP[PD|PS|SD|SS] instructions.
enum class CmpImm : uint8_t {
  kEQ            = 0x00u,       //!< Equal (Quiet), same as \ref VCmpImm::kEQ_OQ.
  kLT            = 0x01u,       //!< Less (Signaling), same as \ref VCmpImm::kLT_OS.
  kLE            = 0x02u,       //!< Less/Equal (Signaling), same as \ref VCmpImm::kLE_OS.
  kUNORD         = 0x03u,       //!< Unordered (Quiet), same as \ref VCmpImm::kUNORD_Q.
  kNEQ           = 0x04u,       //!< Not Equal (Quiet), same as \ref VCmpImm::kNEQ_UQ.
  kNLT           = 0x05u,       //!< Not Less (Signaling), same as \ref VCmpImm::kNLT_US.
  kNLE           = 0x06u,       //!< Not Less/Equal (Signaling), same as \ref VCmpImm::kNLE_US.
  kORD           = 0x07u        //!< Ordered (Quiet), same as \ref VCmpImm::kORD_Q.
};

//! An immediate value that can be used with [V]PCMP[I|E]STR[I|M] instructions.
enum class PCmpStrImm : uint8_t {
  // Source Data Format
  // ------------------

  kUB            = 0x00u << 0,  //!< The source data format is unsigned bytes.
  kUW            = 0x01u << 0,  //!< The source data format is unsigned words.
  kSB            = 0x02u << 0,  //!< The source data format is signed bytes.
  kSW            = 0x03u << 0,  //!< The source data format is signed words.

  // Aggregation Operation
  // ---------------------

  kEqualAny      = 0x00u << 2,  //!< The arithmetic comparison is "equal".
  kRanges        = 0x01u << 2,  //!< The arithmetic comparison is "greater than or equal" between even indexed
                                //!< elements and "less than or equal" between odd indexed elements.
  kEqualEach     = 0x02u << 2,  //!< The arithmetic comparison is "equal".
  kEqualOrdered  = 0x03u << 2,  //!< The arithmetic comparison is "equal".

  // Polarity
  // --------

  kPosPolarity   = 0x00u << 4,  //!< IntRes2 = IntRes1.
  kNegPolarity   = 0x01u << 4,  //!< IntRes2 = -1 XOR IntRes1.
  kPosMasked     = 0x02u << 4,  //!< IntRes2 = IntRes1.
  kNegMasked     = 0x03u << 4,  //!< IntRes2[i] = second[i] == invalid ? IntRes1[i] : ~IntRes1[i].

  // Output Selection (pcmpstri)
  // ---------------------------

  kOutputLSI     = 0x00u << 6,  //!< The index returned to ECX is of the least significant set bit in IntRes2.
  kOutputMSI     = 0x01u << 6,  //!< The index returned to ECX is of the most significant set bit in IntRes2.

  // Output Selection (pcmpstrm)
  // ---------------------------

  kBitMask       = 0x00u << 6,  //!< IntRes2 is returned as the mask to the least significant bits of XMM0.
  kIndexMask     = 0x01u << 6   //!< IntRes2 is expanded into a byte/word mask and placed in XMM0.
};
ASMJIT_DEFINE_ENUM_FLAGS(PCmpStrImm)

//! An immediate value that can be used with ROUND[PD|PS|SD|SS] instructions.
//!
//! \note `kSuppress` is a mask that can be used with any other value.
enum class RoundImm : uint8_t {
  kNearest       = 0x00u,       //!< Round to nearest (even).
  kDown          = 0x01u,       //!< Round to down toward -INF (floor),
  kUp            = 0x02u,       //!< Round to up toward +INF (ceil).
  kTrunc         = 0x03u,       //!< Round toward zero (truncate).
  kCurrent       = 0x04u,       //!< Round to the current rounding mode set (ignores other RC bits).
  kSuppress      = 0x08u        //!< Suppress exceptions (avoids inexact exception, if set).
};
ASMJIT_DEFINE_ENUM_FLAGS(RoundImm)

//! An immediate value that can be used with VCMP[PD|PS|SD|SS] instructions (AVX).
//!
//! The first 8 values are compatible with \ref CmpImm.
enum class VCmpImm : uint8_t {
  kEQ_OQ         = 0x00u,       //!< Equal             (Quiet    , Ordered)  , same as \ref CmpImm::kEQ.
  kLT_OS         = 0x01u,       //!< Less              (Signaling, Ordered)  , same as \ref CmpImm::kLT.
  kLE_OS         = 0x02u,       //!< Less/Equal        (Signaling, Ordered)  , same as \ref CmpImm::kLE.
  kUNORD_Q       = 0x03u,       //!< Unordered         (Quiet)               , same as \ref CmpImm::kUNORD.
  kNEQ_UQ        = 0x04u,       //!< Not Equal         (Quiet    , Unordered), same as \ref CmpImm::kNEQ.
  kNLT_US        = 0x05u,       //!< Not Less          (Signaling, Unordered), same as \ref CmpImm::kNLT.
  kNLE_US        = 0x06u,       //!< Not Less/Equal    (Signaling, Unordered), same as \ref CmpImm::kNLE.
  kORD_Q         = 0x07u,       //!< Ordered           (Quiet)               , same as \ref CmpImm::kORD.
  kEQ_UQ         = 0x08u,       //!< Equal             (Quiet    , Unordered).
  kNGE_US        = 0x09u,       //!< Not Greater/Equal (Signaling, Unordered).
  kNGT_US        = 0x0Au,       //!< Not Greater       (Signaling, Unordered).
  kFALSE_OQ      = 0x0Bu,       //!< False             (Quiet    , Ordered).
  kNEQ_OQ        = 0x0Cu,       //!< Not Equal         (Quiet    , Ordered).
  kGE_OS         = 0x0Du,       //!< Greater/Equal     (Signaling, Ordered).
  kGT_OS         = 0x0Eu,       //!< Greater           (Signaling, Ordered).
  kTRUE_UQ       = 0x0Fu,       //!< True              (Quiet    , Unordered).
  kEQ_OS         = 0x10u,       //!< Equal             (Signaling, Ordered).
  kLT_OQ         = 0x11u,       //!< Less              (Quiet    , Ordered).
  kLE_OQ         = 0x12u,       //!< Less/Equal        (Quiet    , Ordered).
  kUNORD_S       = 0x13u,       //!< Unordered         (Signaling).
  kNEQ_US        = 0x14u,       //!< Not Equal         (Signaling, Unordered).
  kNLT_UQ        = 0x15u,       //!< Not Less          (Quiet    , Unordered).
  kNLE_UQ        = 0x16u,       //!< Not Less/Equal    (Quiet    , Unordered).
  kORD_S         = 0x17u,       //!< Ordered           (Signaling).
  kEQ_US         = 0x18u,       //!< Equal             (Signaling, Unordered).
  kNGE_UQ        = 0x19u,       //!< Not Greater/Equal (Quiet    , Unordered).
  kNGT_UQ        = 0x1Au,       //!< Not Greater       (Quiet    , Unordered).
  kFALSE_OS      = 0x1Bu,       //!< False             (Signaling, Ordered).
  kNEQ_OS        = 0x1Cu,       //!< Not Equal         (Signaling, Ordered).
  kGE_OQ         = 0x1Du,       //!< Greater/Equal     (Quiet    , Ordered).
  kGT_OQ         = 0x1Eu,       //!< Greater           (Quiet    , Ordered).
  kTRUE_US       = 0x1Fu        //!< True              (Signaling, Unordered).
};

//! An immediate value that can be used with VFIXUPIMM[PD|PS|SD|SS] instructions (AVX-512).
//!
//! The final immediate is a combination of all possible control bits.
enum class VFixupImm : uint8_t {
  kNone          = 0x00u,
  kZEOnZero      = 0x01u,
  kIEOnZero      = 0x02u,
  kZEOnOne       = 0x04u,
  kIEOnOne       = 0x08u,
  kIEOnSNaN      = 0x10u,
  kIEOnNInf      = 0x20u,
  kIEOnNegative  = 0x40u,
  kIEOnPInf      = 0x80u
};
ASMJIT_DEFINE_ENUM_FLAGS(VFixupImm)

//! An immediate value that can be used with VFPCLASS[PD|PS|SD|SS] instructions (AVX-512).
//!
//! The values can be combined together to form the final 8-bit mask.
enum class VFPClassImm : uint8_t {
  kNone          = 0x00u,
  kQNaN          = 0x01u,       //!< Checks for QNaN.
  kPZero         = 0x02u,       //!< Checks for +0.
  kNZero         = 0x04u,       //!< Checks for -0.
  kPInf          = 0x08u,       //!< Checks for +Inf.
  kNInf          = 0x10u,       //!< Checks for -Inf.
  kDenormal      = 0x20u,       //!< Checks for denormal.
  kNegative      = 0x40u,       //!< Checks for negative finite value.
  kSNaN          = 0x80u        //!< Checks for SNaN.
};
ASMJIT_DEFINE_ENUM_FLAGS(VFPClassImm)

//! An immediate value that can be used with VGETMANT[PD|PS|SD|SS] instructions (AVX-512).
//!
//! The value is a combination of a normalization interval and a sign control.
enum class VGetMantImm : uint8_t {
  // Normalization Interval
  // ----------------------

  k1To2          = 0x00u,       //!< Normalization interval is [1, 2)
  k1Div2To2      = 0x01u,       //!< Normalization interval is [0.5, 2)
  k1Div2To1      = 0x02u,       //!< Normalization interval is [0.5, 1)
  k3Div4To3Div2  = 0x03u,       //!< Normalization interval is [3/4, 3/2)

  // Sign Control
  // ------------

  kSrcSign       = 0x00u,       //!< Source sign.
  kNoSign        = 0x04u,       //!< Zero sign
  kQNaNIfSign    = 0x08u        //!< QNAN_Indefinite if sign(src) != 0, regardless of `kSignSrc` or `kNoSign`.
};
ASMJIT_DEFINE_ENUM_FLAGS(VGetMantImm)

//! A predicate used by VPCMP[U][B|W|D|Q] instructions (AVX-512).
enum class VPCmpImm : uint8_t {
  kEQ            = 0x00u,       //!< Equal.
  kLT            = 0x01u,       //!< Less.
  kLE            = 0x02u,       //!< Less/Equal.
  kFALSE         = 0x03u,       //!< False.
  kNE            = 0x04u,       //!< Not Equal.
  kGE            = 0x05u,       //!< Greater/Equal.
  kGT            = 0x06u,       //!< Greater.
  kTRUE          = 0x07u        //!< True.
};

//! A predicate used by VPCOM[U][B|W|D|Q] instructions (XOP).
enum class VPComImm : uint8_t {
  kLT            = 0x00u,       //!< Less.
  kLE            = 0x01u,       //!< Less/Equal
  kGT            = 0x02u,       //!< Greater.
  kGE            = 0x03u,       //!< Greater/Equal.
  kEQ            = 0x04u,       //!< Equal.
  kNE            = 0x05u,       //!< Not Equal.
  kFALSE         = 0x06u,       //!< False.
  kTRUE          = 0x07u        //!< True.
};

//! A predicate used by VRANGE[PD|PS|SD|SS] instructions (AVX-512).
enum class VRangeImm : uint8_t {
  // Selector
  // --------

  kSelectMin     = 0x00u,       //!< Select minimum value.
  kSelectMax     = 0x01u,       //!< Select maximum value.
  kSelectAbsMin  = 0x02u,       //!< Select minimum absolute value.
  kSelectAbsMax  = 0x03u,       //!< Select maximum absolute value.

  // Sign
  // ----

  kSignSrc1      = 0x00u,       //!< Select sign of SRC1.
  kSignSrc2      = 0x04u,       //!< Select sign of SRC2.
  kSign0         = 0x08u,       //!< Set sign to 0.
  kSign1         = 0x0Cu        //!< Set sign to 1.
};
ASMJIT_DEFINE_ENUM_FLAGS(VRangeImm)

//! A predicate used by VREDUCE[PD|PS|SD|SS] instructions (AVX-512).
enum class VReduceImm : uint8_t {
  kRoundEven     = 0x00u,       //!< Round to nearest even.
  kRoundDown     = 0x01u,       //!< Round down.
  kRoundUp       = 0x02u,       //!< Round up.
  kRoundTrunc    = 0x03u,       //!< Truncate.
  kRoundCurrent  = 0x04u,       //!< Round to the current mode set.
  kSuppress      = 0x08u,       //!< Suppress exceptions.
  kFixedImmMask  = 0xF0u        //!< Fixed length value mask.
};
ASMJIT_DEFINE_ENUM_FLAGS(VReduceImm)

//! Creates a \ref VReduceImm from a combination of `flags` and `fixed_point_length`.
static ASMJIT_INLINE_CONSTEXPR VReduceImm vreduce_imm(VReduceImm flags, uint32_t fixed_point_length) noexcept {
  return flags | VReduceImm(fixed_point_length << 4);
}

//! A predicate that can be used as an immediate value with VPTERNLOG[D|Q] instruction.
//!
//! There are 3 inputs to the instruction (\ref kA, \ref kB, \ref kC). Ternary logic can define any combination
//! that would be performed on these 3 inputs to get the desired output - any combination of AND, OR, XOR, NOT
//! is possible.
//!
//! \sa \ref tlog_from_bits and \ref tlog_if_else
enum class TLogImm : uint8_t {
  k0             = 0x00u,       //!< 0 value.
  k1             = 0xFFu,       //!< 1 value.
  kA             = 0xF0u,       //!< A value.
  kB             = 0xCCu,       //!< B value.
  kC             = 0xAAu,       //!< C value.

  kNotA          = kA ^ k1,     //!< `!A` expression.
  kNotB          = kB ^ k1,     //!< `!B` expression.
  kNotC          = kC ^ k1,     //!< `!C` expression.

  kAB            = kA & kB,     //!< `A & B` expression.
  kAC            = kA & kC,     //!< `A & C` expression.
  kBC            = kB & kC,     //!< `B & C` expression.
  kNotAB         = kAB ^ k1,    //!< `!(A & B)` expression.
  kNotAC         = kAC ^ k1,    //!< `!(A & C)` expression.
  kNotBC         = kBC ^ k1,    //!< `!(B & C)` expression.

  kABC           = kAB & kC,    //!< `A & B & C` expression.
  kNotABC        = kABC ^ k1    //!< `!(A & B & C)` expression.
};
ASMJIT_DEFINE_ENUM_FLAGS(TLogImm)

//! Creates an immediate that can be used by VPTERNLOG[D|Q] instructions.
static ASMJIT_INLINE_CONSTEXPR TLogImm tlog_from_bits(uint8_t b000, uint8_t b001, uint8_t b010, uint8_t b011, uint8_t b100, uint8_t b101, uint8_t b110, uint8_t b111) noexcept {
  return TLogImm(uint8_t(b000 << 0) |
                 uint8_t(b001 << 1) |
                 uint8_t(b010 << 2) |
                 uint8_t(b011 << 3) |
                 uint8_t(b100 << 4) |
                 uint8_t(b101 << 5) |
                 uint8_t(b110 << 6) |
                 uint8_t(b111 << 7));
}

//! Creates an if/else logic that can be used by VPTERNLOG[D|Q] instructions.
static ASMJIT_INLINE_CONSTEXPR TLogImm tlog_if_else(TLogImm condition, TLogImm a, TLogImm b) noexcept { return (condition & a) | (~condition & b); }

//! Creates a shuffle immediate value that be used with SSE/AVX/AVX-512 instructions to shuffle 2 elements in a vector.
//!
//! \param a Position of the first  component [0, 1].
//! \param b Position of the second component [0, 1].
//!
//! Shuffle constants can be used to encode an immediate for these instructions:
//!   - `shufpd|vshufpd`
static ASMJIT_INLINE_CONSTEXPR uint32_t shuffle_imm(uint32_t a, uint32_t b) noexcept {
  return (a << 1) | b;
}

//! Creates a shuffle immediate value that be used with SSE/AVX/AVX-512 instructions to shuffle 4 elements in a vector.
//!
//! \param a Position of the first  component [0, 3].
//! \param b Position of the second component [0, 3].
//! \param c Position of the third  component [0, 3].
//! \param d Position of the fourth component [0, 3].
//!
//! Shuffle constants can be used to encode an immediate for these instructions:
//!   - `pshufw`
//!   - `pshuflw|vpshuflw`
//!   - `pshufhw|vpshufhw`
//!   - `pshufd|vpshufd`
//!   - `shufps|vshufps`
static ASMJIT_INLINE_CONSTEXPR uint32_t shuffle_imm(uint32_t a, uint32_t b, uint32_t c, uint32_t d) noexcept {
  return (a << 6) | (b << 4) | (c << 2) | d;
}

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_X86_X86GLOBALS_H_INCLUDED
