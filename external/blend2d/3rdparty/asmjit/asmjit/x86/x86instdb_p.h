// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_X86_X86INSTDB_P_H_INCLUDED
#define ASMJIT_X86_X86INSTDB_P_H_INCLUDED

#include <asmjit/core/instdb_p.h>
#include <asmjit/x86/x86instdb.h>

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

//! \cond INTERNAL
//! \addtogroup asmjit_x86
//! \{

namespace InstDB {

//! Instruction encoding (X86|X86_64).
//!
//! This is a specific identifier that is used by AsmJit to describe the way each instruction is encoded. Some
//! encodings are special only for a single instruction as X86 instruction set contains a lot of legacy encodings,
//! and some encodings describe a group of instructions that share some commons, like MMX, SSE, AVX, AVX512
//! instructions, etc...
enum EncodingId : uint32_t {
  kEncodingNone = 0,                     //!< Never used.
  kEncodingX86Op,                        //!< X86 [OP].
  kEncodingX86Op_Mod11RM,                //!< X86 [OP] (opcode with ModRM byte where MOD must be 11b).
  kEncodingX86Op_Mod11RM_I8,             //!< X86 [OP] (opcode with ModRM byte + 8-bit immediate).
  kEncodingX86Op_xAddr,                  //!< X86 [OP] (implicit address in the first register operand).
  kEncodingX86Op_xAX,                    //!< X86 [OP] (implicit or explicit '?AX' form).
  kEncodingX86Op_xDX_xAX,                //!< X86 [OP] (implicit or explicit '?DX, ?AX' form).
  kEncodingX86Op_MemZAX,                 //!< X86 [OP] (implicit or explicit '[EAX|RAX]' form).
  kEncodingX86I_xAX,                     //!< X86 [I] (implicit or explicit '?AX' form).
  kEncodingX86M,                         //!< X86 [M] (handles 2|4|8-bytes size).
  kEncodingX86M_NoMemSize,               //!< X86 [M] (handles 2|4|8-bytes size, but doesn't consider memory size).
  kEncodingX86M_NoSize,                  //!< X86 [M] (doesn't handle any size).
  kEncodingX86M_GPB,                     //!< X86 [M] (handles single-byte size).
  kEncodingX86M_GPB_MulDiv,              //!< X86 [M] (like GPB, handles implicit|explicit MUL|DIV|IDIV).
  kEncodingX86M_Only,                    //!< X86 [M] (restricted to memory operand of any size).
  kEncodingX86M_Only_EDX_EAX,            //!< X86 [M] (memory operand only, followed by implicit `EDX` and `EAX`).
  kEncodingX86M_Nop,                     //!< X86 [M] (special case of NOP instruction).
  kEncodingX86R_Native,                  //!< X86 [R] (register must be either 32-bit or 64-bit depending on arch).
  kEncodingX86R_FromM,                   //!< X86 [R] - which specifies memory address.
  kEncodingX86R32_EDX_EAX,               //!< X86 [R32] followed by implicit `EDX` and `EAX`.
  kEncodingX86Rm,                        //!< X86 [RM] (doesn't handle single-byte size).
  kEncodingX86Rm_Raw66H,                 //!< X86 [RM] (used by LZCNT, POPCNT, and TZCNT).
  kEncodingX86Rm_NoSize,                 //!< X86 [RM] (doesn't add REX.W prefix if 64-bit reg is used).
  kEncodingX86Mr,                        //!< X86 [MR] (doesn't handle single-byte size).
  kEncodingX86Mr_NoSize,                 //!< X86 [MR] (doesn't handle any size).
  kEncodingX86Arith,                     //!< X86 adc, add, and, cmp, or, sbb, sub, xor.
  kEncodingX86Bswap,                     //!< X86 bswap.
  kEncodingX86Bt,                        //!< X86 bt, btc, btr, bts.
  kEncodingX86Call,                      //!< X86 call.
  kEncodingX86Cmpxchg,                   //!< X86 [MR] cmpxchg.
  kEncodingX86Cmpxchg8b_16b,             //!< X86 [MR] cmpxchg8b, cmpxchg16b.
  kEncodingX86Crc,                       //!< X86 crc32.
  kEncodingX86Enter,                     //!< X86 enter.
  kEncodingX86Imul,                      //!< X86 imul.
  kEncodingX86In,                        //!< X86 in.
  kEncodingX86Ins,                       //!< X86 ins[b|q|d].
  kEncodingX86IncDec,                    //!< X86 inc, dec.
  kEncodingX86Int,                       //!< X86 int (interrupt).
  kEncodingX86Jcc,                       //!< X86 jcc.
  kEncodingX86JecxzLoop,                 //!< X86 jcxz, jecxz, jrcxz, loop, loope, loopne.
  kEncodingX86Jmp,                       //!< X86 jmp.
  kEncodingX86JmpRel,                    //!< X86 xbegin.
  kEncodingX86LcallLjmp,                 //!< X86 lcall/ljmp.
  kEncodingX86Lea,                       //!< X86 lea.
  kEncodingX86Mov,                       //!< X86 mov (all possible cases).
  kEncodingX86Movabs,                    //!< X86 movabs.
  kEncodingX86MovsxMovzx,                //!< X86 movsx, movzx.
  kEncodingX86MovntiMovdiri,             //!< X86 movnti/movdiri.
  kEncodingX86EnqcmdMovdir64b,           //!< X86 enqcmd/enqcmds/movdir64b.
  kEncodingX86Out,                       //!< X86 out.
  kEncodingX86Outs,                      //!< X86 out[b|w|d].
  kEncodingX86Push,                      //!< X86 push.
  kEncodingX86Pushw,                     //!< X86 pushw.
  kEncodingX86Pop,                       //!< X86 pop.
  kEncodingX86Ret,                       //!< X86 ret.
  kEncodingX86Rot,                       //!< X86 rcl, rcr, rol, ror, sal, sar, shl, shr.
  kEncodingX86Set,                       //!< X86 setcc.
  kEncodingX86ShldShrd,                  //!< X86 shld, shrd.
  kEncodingX86StrRm,                     //!< X86 lods.
  kEncodingX86StrMr,                     //!< X86 scas, stos.
  kEncodingX86StrMm,                     //!< X86 cmps, movs.
  kEncodingX86Test,                      //!< X86 test.
  kEncodingX86Xadd,                      //!< X86 xadd.
  kEncodingX86Xchg,                      //!< X86 xchg.
  kEncodingX86Fence,                     //!< X86 lfence, mfence, sfence.
  kEncodingX86Bndmov,                    //!< X86 [RM|MR] (used by BNDMOV).
  kEncodingFpuOp,                        //!< FPU [OP].
  kEncodingFpuArith,                     //!< FPU fadd, fdiv, fdivr, fmul, fsub, fsubr.
  kEncodingFpuCom,                       //!< FPU fcom, fcomp.
  kEncodingFpuFldFst,                    //!< FPU fld, fst, fstp.
  kEncodingFpuM,                         //!< FPU fiadd, ficom, ficomp, fidiv, fidivr, fild, fimul, fist, fistp, fisttp, fisub, fisubr.
  kEncodingFpuR,                         //!< FPU fcmov, fcomi, fcomip, ffree, fucom, fucomi, fucomip, fucomp, fxch.
  kEncodingFpuRDef,                      //!< FPU faddp, fdivp, fdivrp, fmulp, fsubp, fsubrp.
  kEncodingFpuStsw,                      //!< FPU fnstsw, Fstsw.
  kEncodingExtRm,                        //!< EXT [RM].
  kEncodingExtRm_XMM0,                   //!< EXT [RM<XMM0>].
  kEncodingExtRm_ZDI,                    //!< EXT [RM<ZDI>].
  kEncodingExtRm_P,                      //!< EXT [RM] (propagates 66H if the instruction uses XMM register).
  kEncodingExtRm_Wx,                     //!< EXT [RM] (propagates REX.W if GPQ is used or the second operand is GPQ/QWORD_PTR).
  kEncodingExtRm_Wx_GpqOnly,             //!< EXT [RM] (propagates REX.W if the first operand is GPQ register).
  kEncodingExtRmRi,                      //!< EXT [RM|RI].
  kEncodingExtRmRi_P,                    //!< EXT [RM|RI] (propagates 66H if the instruction uses XMM register).
  kEncodingExtRmi,                       //!< EXT [RMI].
  kEncodingExtRmi_P,                     //!< EXT [RMI] (propagates 66H if the instruction uses XMM register).
  kEncodingExtPextrw,                    //!< EXT pextrw.
  kEncodingExtExtract,                   //!< EXT pextrb, pextrd, pextrq, extractps.
  kEncodingExtMov,                       //!< EXT mov?? - #1:[MM|XMM, MM|XMM|Mem] #2:[MM|XMM|Mem, MM|XMM].
  kEncodingExtMovbe,                     //!< EXT movbe.
  kEncodingExtMovd,                      //!< EXT movd.
  kEncodingExtMovq,                      //!< EXT movq.
  kEncodingExtExtrq,                     //!< EXT extrq (SSE4A).
  kEncodingExtInsertq,                   //!< EXT insrq (SSE4A).
  kEncodingExt3dNow,                     //!< EXT [RMI] (3DNOW specific).
  kEncodingVexOp,                        //!< VEX [OP].
  kEncodingVexOpMod,                     //!< VEX [OP] with MODR/M.
  kEncodingVexKmov,                      //!< VEX [RM|MR] (used by kmov[b|w|d|q]).
  kEncodingVexR_Wx,                      //!< VEX|EVEX [R] (propagatex VEX.W if GPQ used).
  kEncodingVexM,                         //!< VEX|EVEX [M].
  kEncodingVexMr_Lx,                     //!< VEX|EVEX [MR] (propagates VEX|EVEX.L if YMM used).
  kEncodingVexMr_VM,                     //!< VEX|EVEX [MR] (VSIB support).
  kEncodingVexMri,                       //!< VEX|EVEX [MRI].
  kEncodingVexMri_Lx,                    //!< VEX|EVEX [MRI] (propagates VEX|EVEX.L if YMM used).
  kEncodingVexMri_Vpextrw,               //!< VEX|EVEX [MRI] (special case required by VPEXTRW instruction).
  kEncodingVexMvr_Wx,                    //!< VEX|EVEX [MVR] (propagates VEX|EVEX.W if GPQ used).
  kEncodingVexRm,                        //!< VEX|EVEX [RM].
  kEncodingVexRm_ZDI,                    //!< VEX|EVEX [RM<ZDI>].
  kEncodingVexRm_Wx,                     //!< VEX|EVEX [RM] (propagates VEX|EVEX.W if GPQ used).
  kEncodingVexRm_Lx,                     //!< VEX|EVEX [RM] (propagates VEX|EVEX.L if YMM used).
  kEncodingVexRm_Lx_Narrow,              //!< VEX|EVEX [RM] (the destination vector size is narrowed).
  kEncodingVexRm_Lx_Bcst,                //!< VEX|EVEX [RM] (can handle broadcast r32/r64).
  kEncodingVexRm_VM,                     //!< VEX|EVEX [RM] (propagates VEX|EVEX.L, VSIB support).
  kEncodingVexRmi,                       //!< VEX|EVEX [RMI].
  kEncodingVexRmi_Wx,                    //!< VEX|EVEX [RMI] (propagates VEX|EVEX.W if GPQ used).
  kEncodingVexRmi_Lx,                    //!< VEX|EVEX [RMI] (propagates VEX|EVEX.L if YMM used).
  kEncodingVexRvm,                       //!< VEX|EVEX [RVM].
  kEncodingVexRvm_Wx,                    //!< VEX|EVEX [RVM] (propagates VEX|EVEX.W if GPQ used).
  kEncodingVexRvm_ZDX_Wx,                //!< VEX|EVEX [RVM<ZDX>] (propagates VEX|EVEX.W if GPQ used).
  kEncodingVexRvm_Lx,                    //!< VEX|EVEX [RVM] (propagates VEX|EVEX.L if YMM used).
  kEncodingVexRvm_Lx_KEvex,              //!< VEX|EVEX [RVM] (forces EVEX prefix if K register is used on destination).
  kEncodingVexRvm_Lx_2xK,                //!< VEX|EVEX [RVM] (vp2intersectd/vp2intersectq).
  kEncodingVexRvmr,                      //!< VEX|EVEX [RVMR].
  kEncodingVexRvmr_Lx,                   //!< VEX|EVEX [RVMR] (propagates VEX|EVEX.L if YMM used).
  kEncodingVexRvmi,                      //!< VEX|EVEX [RVMI].
  kEncodingVexRvmi_KEvex,                //!< VEX|EVEX [RVMI] (forces EVEX prefix if K register is used on destination).
  kEncodingVexRvmi_Lx,                   //!< VEX|EVEX [RVMI] (propagates VEX|EVEX.L if YMM used).
  kEncodingVexRvmi_Lx_KEvex,             //!< VEX|EVEX [RVMI] (forces EVEX prefix if K register is used on destination).
  kEncodingVexRmv,                       //!< VEX|EVEX [RMV].
  kEncodingVexRmv_Wx,                    //!< VEX|EVEX [RMV] (propagates VEX|EVEX.W if GPQ used).
  kEncodingVexRmv_VM,                    //!< VEX|EVEX [RMV] (propagates VEX|EVEX.L, VSIB support).
  kEncodingVexRmvRm_VM,                  //!< VEX|EVEX [RMV|RM] (propagates VEX|EVEX.L, VSIB support).
  kEncodingVexRmvi,                      //!< VEX|EVEX [RMVI].
  kEncodingVexRmMr,                      //!< VEX|EVEX [RM|MR].
  kEncodingVexRmMr_Lx,                   //!< VEX|EVEX [RM|MR] (propagates VEX|EVEX.L if YMM used).
  kEncodingVexRvmRmv,                    //!< VEX|EVEX [RVM|RMV].
  kEncodingVexRvmRmi,                    //!< VEX|EVEX [RVM|RMI].
  kEncodingVexRvmRmi_Lx,                 //!< VEX|EVEX [RVM|RMI] (propagates VEX|EVEX.L if YMM used).
  kEncodingVexRvmRmvRmi,                 //!< VEX|EVEX [RVM|RMV|RMI].
  kEncodingVexRvmMr,                     //!< VEX|EVEX [RVM|MR].
  kEncodingVexRvmMvr,                    //!< VEX|EVEX [RVM|MVR].
  kEncodingVexRvmMvr_Lx,                 //!< VEX|EVEX [RVM|MVR] (propagates VEX|EVEX.L if YMM used).
  kEncodingVexRvmVmi,                    //!< VEX|EVEX [RVM|VMI].
  kEncodingVexRvmVmi_Lx,                 //!< VEX|EVEX [RVM|VMI] (propagates VEX|EVEX.L if YMM used).
  kEncodingVexRvmVmi_Lx_MEvex,           //!< VEX|EVEX [RVM|VMI] (propagates EVEX if the second operand is memory).
  kEncodingVexVm,                        //!< VEX|EVEX [VM].
  kEncodingVexVm_Wx,                     //!< VEX|EVEX [VM] (propagates VEX|EVEX.W if GPQ used).
  kEncodingVexVmi,                       //!< VEX|EVEX [VMI].
  kEncodingVexVmi_Lx,                    //!< VEX|EVEX [VMI] (propagates VEX|EVEX.L if YMM used).
  kEncodingVexVmi4_Wx,                   //!< VEX|EVEX [VMI] (propagates VEX|EVEX.W if GPQ used, DWORD Immediate).
  kEncodingVexVmi_Lx_MEvex,              //!< VEX|EVEX [VMI] (force EVEX prefix when the second operand is memory)
  kEncodingVexRvrmRvmr,                  //!< VEX|EVEX [RVRM|RVMR].
  kEncodingVexRvrmRvmr_Lx,               //!< VEX|EVEX [RVRM|RVMR] (propagates VEX|EVEX.L if YMM used).
  kEncodingVexRvrmiRvmri_Lx,             //!< VEX|EVEX [RVRMI|RVMRI] (propagates VEX|EVEX.L if YMM used).
  kEncodingVexMovdMovq,                  //!< VEX|EVEX vmovd, vmovq.
  kEncodingVexMovssMovsd,                //!< VEX|EVEX vmovss, vmovsd.
  kEncodingFma4,                         //!< FMA4 [R, R, R/M, R/M].
  kEncodingFma4_Lx,                      //!< FMA4 [R, R, R/M, R/M] (propagates AVX.L if YMM used).
  kEncodingAmxCfg,                       //!< AMX ldtilecfg/sttilecfg.
  kEncodingAmxR,                         //!< AMX [R] - tilezero.
  kEncodingAmxRm,                        //!< AMX tileloadd/tileloaddt1.
  kEncodingAmxMr,                        //!< AMX tilestored.
  kEncodingAmxRmv,                       //!< AMX instructions that use TMM registers.
  kEncodingCount                         //!< Count of instruction encodings.
};

//! Additional information table, provides CPU extensions required to execute an instruction and RW flags.
struct AdditionalInfo {
  //! Index to `inst_flags_table`.
  uint8_t _inst_flags_index;
  //! Index to `rw_flags_info_table`.
  uint8_t _rw_flags_index;
  //! Features vector.
  uint8_t _features[6];

  inline const uint8_t* features_begin() const noexcept { return _features; }
  inline const uint8_t* features_end() const noexcept { return _features + ASMJIT_ARRAY_SIZE(_features); }
};

struct RWInfo {
  enum Category : uint8_t {
    kCategoryGeneric = 0,
    kCategoryGenericEx,
    kCategoryMov,
    kCategoryMovabs,
    kCategoryImul,
    kCategoryMovh64,
    kCategoryPunpcklxx,
    kCategoryVmaskmov,
    kCategoryVmovddup,
    kCategoryVmovmskpd,
    kCategoryVmovmskps,
    kCategoryVmov1_2,
    kCategoryVmov1_4,
    kCategoryVmov1_8,
    kCategoryVmov2_1,
    kCategoryVmov4_1,
    kCategoryVmov8_1
  };

  uint8_t category;
  uint8_t rm_info;
  uint8_t op_info_index[6];
};

struct RWInfoOp {
  uint64_t r_byte_mask;
  uint64_t w_byte_mask;
  uint8_t phys_id;
  uint8_t consecutive_lead_count;
  uint8_t reserved[2];
  OpRWFlags flags;
};

//! R/M information.
//!
//! This data is used to replace register operand by a memory operand reliably.
struct RWInfoRm {
  enum Category : uint8_t {
    kCategoryNone = 0,
    kCategoryFixed,
    kCategoryConsistent,
    kCategoryHalf,
    kCategoryQuarter,
    kCategoryEighth
  };

  enum Flags : uint8_t {
    kFlagAmbiguous = 0x01,
    //! Special semantics for PEXTRW - memory operand can only be used with SSE4.1 instruction and it's forbidden in MMX.
    kFlagPextrw = 0x02,
    //! Special semantics for MOVSS and MOVSD - doesn't zero extend the destination if the operation is a reg to reg move.
    kFlagMovssMovsd = 0x04,
    //! Special semantics for AVX shift instructions that do not provide reg/mem in AVX/AVX2 mode (AVX-512 is required).
    kFlagFeatureIfRMI = 0x08
  };

  uint8_t category;
  uint8_t rm_ops_mask;
  uint8_t fixed_size;
  uint8_t flags;
  uint8_t rm_feature;
};

struct RWFlagsInfoTable {
  //! CPU/FPU flags read.
  uint32_t read_flags;
  //! CPU/FPU flags written or undefined.
  uint32_t write_flags;
};

extern const uint8_t rw_info_index_a_table[Inst::_kIdCount];
extern const uint8_t rw_info_index_b_table[Inst::_kIdCount];
extern const RWInfo rw_info_a_table[];
extern const RWInfo rw_info_b_table[];
extern const RWInfoOp rw_info_op_table[];
extern const RWInfoRm rw_info_rm_table[];
extern const RWFlagsInfoTable rw_flags_info_table[];
extern const InstRWFlags inst_flags_table[];

extern const uint32_t main_opcode_table[];
extern const uint32_t alt_opcode_table[];

#ifndef ASMJIT_NO_TEXT

extern const InstNameIndex _inst_name_index;
extern const char _inst_name_string_table[];
extern const uint32_t _inst_name_index_table[];

extern const char alias_name_string_table[];
extern const uint32_t alias_name_index_table[];
extern const uint32_t alias_index_to_inst_id_table[];

// ${NameDataInfo:Begin}
// ------------------- Automatically generated, do not edit -------------------
static constexpr uint32_t kAliasTableSize = 44;
// ----------------------------------------------------------------------------
// ${NameDataInfo:End}

#endif // !ASMJIT_NO_TEXT

extern const AdditionalInfo additional_info_table[];

} // {InstDB}

//! \}
//! \endcond

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_X86_X86INSTDB_P_H_INCLUDED
