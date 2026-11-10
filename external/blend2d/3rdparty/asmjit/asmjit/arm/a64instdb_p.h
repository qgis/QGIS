// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_ARM_A64INSTDB_H_P_INCLUDED
#define ASMJIT_ARM_A64INSTDB_H_P_INCLUDED

#include <asmjit/core/codeholder.h>
#include <asmjit/core/instdb_p.h>
#include <asmjit/arm/a64instdb.h>
#include <asmjit/arm/a64operand.h>

ASMJIT_BEGIN_SUB_NAMESPACE(a64)

//! \cond INTERNAL
//! \addtogroup asmjit_a64
//! \{

namespace InstDB {

// a64::InstDB - Constants Used by Instructions
// ============================================

// GP register types supported by base instructions.
static constexpr uint32_t kW = 0x1;
static constexpr uint32_t kX = 0x2;
static constexpr uint32_t kWX = 0x3;

// GP high register IDs supported by the instruction.
static constexpr uint32_t kZR = Gp::kIdZr;
static constexpr uint32_t kSP = Gp::kIdSp;

// a64::InstDB - RWInfo
// ====================

enum RWInfoType : uint32_t {
  kRWI_R,
  kRWI_RW,
  kRWI_RX,
  kRWI_RRW,
  kRWI_RWX,
  kRWI_W,
  kRWI_WRW,
  kRWI_WRX,
  kRWI_WRRW,
  kRWI_WRRX,
  kRWI_WW,
  kRWI_X,
  kRWI_XRX,
  kRWI_XXRRX,

  kRWI_LDn,
  kRWI_STn,

  kRWI_SpecialStart = kRWI_LDn
};

// a64::InstDB - ElementType
// =========================

enum InstElementType : uint8_t {
  kET_None = uint8_t(VecElementType::kNone),
  kET_B    = uint8_t(VecElementType::kB),
  kET_H    = uint8_t(VecElementType::kH),
  kET_S    = uint8_t(VecElementType::kS),
  kET_D    = uint8_t(VecElementType::kD),
  kET_2H   = uint8_t(VecElementType::kH2),
  kET_4B   = uint8_t(VecElementType::kB4)
};

// a64::InstDB - GpType
// ====================

enum GpType : uint8_t {
  kGp_W,
  kGp_X,
  kGp_X_SP
};

// a64::InstDB - OPSig
// ===================

enum kOpSignature : uint32_t {
  kOp_GpW = RegTraits<RegType::kGp32>::kSignature,
  kOp_GpX = RegTraits<RegType::kGp64>::kSignature,
  kOp_B = RegTraits<RegType::kVec8>::kSignature,
  kOp_H = RegTraits<RegType::kVec16>::kSignature,
  kOp_S = RegTraits<RegType::kVec32>::kSignature,
  kOp_D = RegTraits<RegType::kVec64>::kSignature,
  kOp_Q = RegTraits<RegType::kVec128>::kSignature,

  kOp_V8B = kOp_D | Vec::kSignatureElementB,
  kOp_V4H = kOp_D | Vec::kSignatureElementH,
  kOp_V2S = kOp_D | Vec::kSignatureElementS,

  kOp_V16B = kOp_Q | Vec::kSignatureElementB,
  kOp_V8H = kOp_Q | Vec::kSignatureElementH,
  kOp_V4S = kOp_Q | Vec::kSignatureElementS,
  kOp_V2D = kOp_Q | Vec::kSignatureElementD
};

// a64::InstDB - HFConv
// ====================

enum kHFConv : uint32_t {
  //! FP16 version of the instruction is not available.
  kHF_N,

  //! Doesn't do any change to the opcode.
  kHF_0,

  kHF_A,
  kHF_B,
  kHF_C,
  kHF_D,

  kHF_Count
};

// a64::InstDB - VOType
// ====================

//! Vector operand type combinations used by FP&SIMD instructions.
enum VOType : uint32_t {
  kVO_V_B,
  kVO_V_BH,
  kVO_V_BH_4S,
  kVO_V_BHS,
  kVO_V_BHS_D2,
  kVO_V_HS,
  kVO_V_S,

  kVO_V_B8H4,
  kVO_V_B8H4S2,
  kVO_V_B8D1,
  kVO_V_H4S2,

  kVO_V_B16,
  kVO_V_B16H8,
  kVO_V_B16H8S4,
  kVO_V_B16D2,
  kVO_V_H8S4,
  kVO_V_S4,
  kVO_V_D2,

  kVO_SV_BHS,
  kVO_SV_B8H4S2,
  kVO_SV_HS,
  kVO_V_Any,
  kVO_SV_Any,

  kVO_Count
};

// a64::InstDB - EncodingId
// ========================

// ${EncodingId:Begin}
// ------------------- Automatically generated, do not edit -------------------
enum EncodingId : uint32_t {
  kEncodingNone = 0,
  kEncodingBaseAddSub,
  kEncodingBaseAdr,
  kEncodingBaseAtDcIcTlbi,
  kEncodingBaseAtomicCasp,
  kEncodingBaseAtomicOp,
  kEncodingBaseAtomicSt,
  kEncodingBaseBfc,
  kEncodingBaseBfi,
  kEncodingBaseBfm,
  kEncodingBaseBfx,
  kEncodingBaseBranchCmp,
  kEncodingBaseBranchReg,
  kEncodingBaseBranchRel,
  kEncodingBaseBranchTst,
  kEncodingBaseCCmp,
  kEncodingBaseCInc,
  kEncodingBaseCSel,
  kEncodingBaseCSet,
  kEncodingBaseCmpCmn,
  kEncodingBaseExtend,
  kEncodingBaseExtract,
  kEncodingBaseLdSt,
  kEncodingBaseLdpStp,
  kEncodingBaseLdxp,
  kEncodingBaseLogical,
  kEncodingBaseMinMax,
  kEncodingBaseMov,
  kEncodingBaseMovKNZ,
  kEncodingBaseMrs,
  kEncodingBaseMsr,
  kEncodingBaseMvnNeg,
  kEncodingBaseOp,
  kEncodingBaseOpImm,
  kEncodingBaseOpX16,
  kEncodingBasePrfm,
  kEncodingBaseR,
  kEncodingBaseRM_NoImm,
  kEncodingBaseRM_SImm10,
  kEncodingBaseRM_SImm9,
  kEncodingBaseRR,
  kEncodingBaseRRII,
  kEncodingBaseRRR,
  kEncodingBaseRRRR,
  kEncodingBaseRev,
  kEncodingBaseShift,
  kEncodingBaseStx,
  kEncodingBaseStxp,
  kEncodingBaseSys,
  kEncodingBaseTst,
  kEncodingFSimdPair,
  kEncodingFSimdSV,
  kEncodingFSimdVV,
  kEncodingFSimdVVV,
  kEncodingFSimdVVVV,
  kEncodingFSimdVVVe,
  kEncodingISimdPair,
  kEncodingISimdSV,
  kEncodingISimdVV,
  kEncodingISimdVVV,
  kEncodingISimdVVVI,
  kEncodingISimdVVVV,
  kEncodingISimdVVVVx,
  kEncodingISimdVVVe,
  kEncodingISimdVVVx,
  kEncodingISimdVVx,
  kEncodingISimdWWV,
  kEncodingSimdBicOrr,
  kEncodingSimdCmp,
  kEncodingSimdDot,
  kEncodingSimdDup,
  kEncodingSimdFcadd,
  kEncodingSimdFccmpFccmpe,
  kEncodingSimdFcm,
  kEncodingSimdFcmla,
  kEncodingSimdFcmpFcmpe,
  kEncodingSimdFcsel,
  kEncodingSimdFcvt,
  kEncodingSimdFcvtLN,
  kEncodingSimdFcvtSV,
  kEncodingSimdFmlal,
  kEncodingSimdFmov,
  kEncodingSimdIns,
  kEncodingSimdLdNStN,
  kEncodingSimdLdSt,
  kEncodingSimdLdpStp,
  kEncodingSimdLdurStur,
  kEncodingSimdMov,
  kEncodingSimdMoviMvni,
  kEncodingSimdShift,
  kEncodingSimdShiftES,
  kEncodingSimdSm3tt,
  kEncodingSimdSmovUmov,
  kEncodingSimdSxtlUxtl,
  kEncodingSimdTblTbx
};
// ----------------------------------------------------------------------------
// ${EncodingId:End}

// a64::InstDB::EncodingData
// =========================

namespace EncodingData {

#define M_OPCODE(field, bits) \
  uint32_t _##field : bits; \
  ASMJIT_INLINE_CONSTEXPR uint32_t field() const noexcept { return uint32_t(_##field) << (32 - bits); }

struct BaseOp {
  uint32_t opcode;
};

struct BaseOpX16 {
  uint32_t opcode;
};

struct BaseOpImm {
  uint32_t opcode;
  uint16_t imm_bits;
  uint16_t imm_offset;
};

struct BaseR {
  uint32_t opcode;
  uint32_t reg_type : 8;
  uint32_t reg_hi_id : 8;
  uint32_t r_shift : 8;
};

struct BaseRR {
  uint32_t opcode;
  uint32_t a_type : 2;
  uint32_t a_hi_id : 6;
  uint32_t a_shift : 5;
  uint32_t b_type : 2;
  uint32_t b_hi_id : 6;
  uint32_t b_shift : 5;
  uint32_t uniform : 1;
};

struct BaseRRR {
  M_OPCODE(opcode, 22)
  uint32_t a_type : 2;
  uint32_t a_hi_id : 6;
  uint32_t b_type : 2;
  uint32_t b_hi_id : 6;
  uint32_t c_type : 2;
  uint32_t c_hi_id : 6;
  uint32_t uniform : 1;
};

struct BaseRRRR {
  M_OPCODE(opcode, 22)
  uint32_t a_type : 2;
  uint32_t a_hi_id : 6;
  uint32_t b_type : 2;
  uint32_t b_hi_id : 6;
  uint32_t c_type : 2;
  uint32_t c_hi_id : 6;
  uint32_t d_type : 2;
  uint32_t d_hi_id : 6;
  uint32_t uniform : 1;
};

struct BaseRRII {
  M_OPCODE(opcode, 22)
  uint32_t a_type : 2;
  uint32_t a_hi_id : 6;
  uint32_t b_type : 2;
  uint32_t b_hi_id : 6;
  uint32_t a_imm_size : 6;
  uint32_t a_imm_discard_lsb : 5;
  uint32_t a_imm_offset : 5;
  uint32_t b_imm_size : 6;
  uint32_t b_imm_discard_lsb : 5;
  uint32_t b_imm_offset : 5;
};

struct BaseAtDcIcTlbi {
  uint32_t imm_verify_mask : 14;
  uint32_t imm_verify_data : 14;
  uint32_t mandatory_reg : 1;
};

struct BaseAdcSbc {
  uint32_t opcode;
};

struct BaseMinMax {
  uint32_t register_op;
  uint32_t immediate_op;
};

struct BaseAddSub {
  uint32_t shifted_op  : 10; // sf|.......|Sh|.|Rm|  Imm:6 |Rn|Rd|
  uint32_t extended_op : 10; // sf|.......|..|.|Rm|Opt|Imm3|Rn|Rd|
  uint32_t immediate_op: 10; // sf|.......|Sh|    Imm:12   |Rn|Rd|
};

struct BaseAdr {
  M_OPCODE(opcode, 22)
  OffsetType offset_type : 8;
};

struct BaseBfm {
  uint32_t opcode;         // sf|........|N|ImmR:6|ImmS:6|Rn|Rd|
};

struct BaseCmpCmn {
  uint32_t shifted_op  : 10; // sf|.......|Sh|.|Rm|  Imm:6 |Rn|11111|
  uint32_t extended_op : 10; // sf|.......|..|.|Rm|Opt|Imm3|Rn|11111|
  uint32_t immediate_op: 10; // sf|.......|Sh|    Imm:12   |Rn|11111|
};

struct BaseExtend {
  M_OPCODE(opcode, 22)       // sf|........|N|......|......|Rn|Rd|
  uint32_t reg_type : 2;
  uint32_t u : 1;
};

struct BaseLogical {
  uint32_t shifted_op  : 10; // sf|.......|Sh|.|Rm|  Imm:6 |Rn|Rd|
  uint32_t immediate_op: 10; // sf|........|N|ImmR:6|ImmS:6|Rn|Rd|
  uint32_t negate_imm  : 1;  // True if this is an operation that must negate IMM.
};

struct BaseMvnNeg {
  uint32_t opcode;
};

struct BaseShift {
  M_OPCODE(register_op, 22)
  M_OPCODE(immediate_op, 22)
  uint32_t ror : 2;
};

struct BaseTst {
  uint32_t shifted_op  : 10; // sf|.......|Sh|.|Rm|  Imm:6 |Rn|11111|
  uint32_t immediate_op: 10; // sf|........|N|ImmR:6|ImmS:6|Rn|11111|
};

struct BaseRM_NoImm {
  M_OPCODE(opcode, 22)
  uint32_t reg_type : 2;
  uint32_t reg_hi_id : 6;
  uint32_t x_offset : 5;
};

struct BaseRM_SImm9 {
  M_OPCODE(offset_op, 22)
  M_OPCODE(pre_post_op, 22)
  uint32_t reg_type : 2;
  uint32_t reg_hi_id : 6;
  uint32_t x_offset : 5;
  uint32_t imm_shift : 4;
};

struct BaseRM_SImm10 {
  M_OPCODE(opcode, 22)
  uint32_t reg_type : 2;
  uint32_t reg_hi_id : 6;
  uint32_t x_offset : 5;
  uint32_t imm_shift : 4;
};

struct BasePrfm {
  uint32_t register_op : 11;
  uint32_t s_offset_op  : 10;
  uint32_t u_offset_op  : 11;
  uint32_t literal_op;
};

struct BaseLdSt {
  uint32_t u_offset_op    : 10;
  uint32_t pre_post_op    : 11;
  uint32_t register_op    : 11;
  uint32_t literal_op     : 8;
  uint32_t reg_type       : 2;
  uint32_t x_offset       : 5;
  uint32_t u_offset_shift : 3;
  uint32_t u_alt_inst_id  : 14;
};

struct BaseLdpStp {
  uint32_t offset_op    : 10;
  uint32_t pre_post_op  : 10;
  uint32_t reg_type     : 2;
  uint32_t x_offset     : 5;
  uint32_t offset_shift : 3;
};

struct BaseStx {
  M_OPCODE(opcode, 22)
  uint32_t reg_type : 2;
  uint32_t x_offset : 5;
};

struct BaseLdxp {
  M_OPCODE(opcode, 22)
  uint32_t reg_type : 2;
  uint32_t x_offset : 5;
};

struct BaseStxp {
  M_OPCODE(opcode, 22)
  uint32_t reg_type : 2;
  uint32_t x_offset : 5;
};

struct BaseAtomicOp {
  M_OPCODE(opcode, 22)
  uint32_t reg_type : 2;
  uint32_t x_offset : 5;
  uint32_t zr : 1;
};

struct BaseAtomicSt {
  M_OPCODE(opcode, 22)
  uint32_t reg_type : 2;
  uint32_t x_offset : 5;
};

struct BaseAtomicCasp {
  M_OPCODE(opcode, 22)
  uint32_t reg_type : 2;
  uint32_t x_offset : 5;
};

using BaseBranchReg = BaseOp;
using BaseBranchRel = BaseOp;
using BaseBranchCmp = BaseOp;
using BaseBranchTst = BaseOp;
using BaseExtract = BaseOp;
using BaseBfc = BaseOp;
using BaseBfi = BaseOp;
using BaseBfx = BaseOp;
using BaseCCmp = BaseOp;
using BaseCInc = BaseOp;
using BaseCSet = BaseOp;
using BaseCSel = BaseOp;
using BaseMovKNZ = BaseOp;
using BaseMull = BaseOp;

struct FSimdGeneric {
  uint32_t _scalar_op : 28;
  uint32_t _scalarHf : 4;
  uint32_t _vector_op : 28;
  uint32_t _vectorHf : 4;

  constexpr uint32_t scalar_op() const noexcept { return uint32_t(_scalar_op) << 10; }
  constexpr uint32_t vector_op() const noexcept { return uint32_t(_vector_op) << 10; }
  constexpr uint32_t scalar_hf() const noexcept { return uint32_t(_scalarHf); }
  constexpr uint32_t vector_hf() const noexcept { return uint32_t(_vectorHf); }
};

using FSimdVV = FSimdGeneric;
using FSimdVVV = FSimdGeneric;
using FSimdVVVV = FSimdGeneric;

struct FSimdSV {
  uint32_t opcode;
};

struct FSimdVVVe {
  uint32_t _scalar_op : 28;
  uint32_t _scalarHf : 4;
  uint32_t _vector_op;
  uint32_t _elementOp;

  constexpr uint32_t scalar_op() const noexcept { return uint32_t(_scalar_op) << 10; }
  constexpr uint32_t scalar_hf() const noexcept { return uint32_t(_scalarHf); };
  constexpr uint32_t vector_op() const noexcept { return uint32_t(_vector_op) << 10; }
  constexpr uint32_t vector_hf() const noexcept { return kHF_C; }
  constexpr uint32_t element_scalar_op() const noexcept { return (uint32_t(_elementOp) << 10) | (0x5u << 28); }
  constexpr uint32_t element_vector_op() const noexcept { return (uint32_t(_elementOp) << 10); }
};

struct SimdFcadd {
  uint32_t _opcode;

  constexpr uint32_t opcode() const noexcept { return _opcode << 10; }
};

struct SimdFcmla {
  uint32_t _regularOp;
  uint32_t _elementOp;

  constexpr uint32_t regular_op() const noexcept { return uint32_t(_regularOp) << 10; }
  constexpr uint32_t element_op() const noexcept { return (uint32_t(_elementOp) << 10); }
};

struct SimdFccmpFccmpe {
  uint32_t _opcode;
  constexpr uint32_t opcode() const noexcept { return _opcode; }
};

struct SimdFcm {
  uint32_t _registerOp : 28;
  uint32_t _registerHf : 4;

  uint32_t _zero_op : 28;

  constexpr bool has_register_op() const noexcept { return _registerOp != 0; }
  constexpr bool has_zero_op() const noexcept { return _zero_op != 0; }

  constexpr uint32_t register_scalar_op() const noexcept { return (uint32_t(_registerOp) << 10) | (0x5u << 28); }
  constexpr uint32_t register_vector_op() const noexcept { return uint32_t(_registerOp) << 10; }
  constexpr uint32_t register_scalar_hf() const noexcept { return uint32_t(_registerHf); }
  constexpr uint32_t register_vector_hf() const noexcept { return uint32_t(_registerHf); }

  constexpr uint32_t zero_scalar_op() const noexcept { return (uint32_t(_zero_op) << 10) | (0x5u << 28); }
  constexpr uint32_t zero_vector_op() const noexcept { return (uint32_t(_zero_op) << 10); }
};

struct SimdFcmpFcmpe {
  uint32_t _opcode;
  constexpr uint32_t opcode() const noexcept { return _opcode; }
};

struct SimdFcvtLN {
  uint32_t _opcode : 22;
  uint32_t _is_cvtxn : 1;
  uint32_t _has_scalar : 1;

  constexpr uint32_t scalar_op() const noexcept { return (uint32_t(_opcode) << 10) | (0x5u << 28); }
  constexpr uint32_t vector_op() const noexcept { return (uint32_t(_opcode) << 10); }

  constexpr uint32_t is_cvtxn() const noexcept { return _is_cvtxn; }
  constexpr uint32_t has_scalar() const noexcept { return _has_scalar; }
};

struct SimdFcvtSV {
  uint32_t _vectorIntOp;
  uint32_t _vectorFpOp;
  uint32_t _general_op : 31;
  uint32_t _isFloatToInt : 1;

  constexpr uint32_t scalar_int_op() const noexcept { return (uint32_t(_vectorIntOp) << 10) | (0x5u << 28); }
  constexpr uint32_t vector_int_op() const noexcept { return uint32_t(_vectorIntOp) << 10; }
  constexpr uint32_t scalar_fp_op() const noexcept { return (uint32_t(_vectorFpOp) << 10) | (0x5u << 28); }
  constexpr uint32_t vector_fp_op() const noexcept { return uint32_t(_vectorFpOp) << 10; }
  constexpr uint32_t general_op() const noexcept { return (uint32_t(_general_op) << 10); }

  constexpr uint32_t is_float_to_int() const noexcept { return _isFloatToInt; }
  constexpr uint32_t is_fixed_point() const noexcept { return _vectorFpOp != 0; }
};

struct SimdFmlal {
  uint32_t _vector_op;
  uint32_t _elementOp;
  uint8_t _optionalQ;
  uint8_t ta;
  uint8_t tb;
  uint8_t tElement;

  constexpr uint32_t vector_op() const noexcept { return uint32_t(_vector_op) << 10; }
  constexpr uint32_t element_op() const noexcept { return uint32_t(_elementOp) << 10; }
  constexpr uint32_t optional_q() const noexcept { return _optionalQ; }
};

struct FSimdPair {
  uint32_t _scalar_op;
  uint32_t _vector_op;

  constexpr uint32_t scalar_op() const noexcept { return uint32_t(_scalar_op) << 10; }
  constexpr uint32_t vector_op() const noexcept { return uint32_t(_vector_op) << 10; }
};

struct ISimdVV {
  M_OPCODE(opcode, 22)
  uint32_t vec_op_type : 6;
};

struct ISimdVVx {
  M_OPCODE(opcode, 22)
  uint32_t op0_signature;
  uint32_t op1_signature;
};

struct ISimdSV {
  M_OPCODE(opcode, 22)
  uint32_t vec_op_type : 6;
};

struct ISimdVVV {
  M_OPCODE(opcode, 22)
  uint32_t vec_op_type : 6;
};

struct ISimdVVVx {
  M_OPCODE(opcode, 22)
  uint32_t op0_signature;
  uint32_t op1_signature;
  uint32_t op2_signature;
};

struct ISimdWWV {
  M_OPCODE(opcode, 22)
  uint32_t vec_op_type : 6;
};

struct ISimdVVVe {
  uint32_t regular_op : 26; // 22 bits used.
  uint32_t regular_vec_type : 6;
  uint32_t element_op : 26; // 22 bits used.
  uint32_t element_vec_type : 6;
};

struct ISimdVVVI {
  M_OPCODE(opcode, 22)
  uint32_t vec_op_type : 6;
  uint32_t imm_size : 4;
  uint32_t imm_shift : 4;
  uint32_t imm64_has_one_bit_less : 1;
};

struct ISimdVVVV {
  uint32_t opcode : 22;
  uint32_t vec_op_type : 6;
};

struct ISimdVVVVx {
  uint32_t opcode;
  uint32_t op0_signature;
  uint32_t op1_signature;
  uint32_t op2_signature;
  uint32_t op3_signature;
};

struct SimdBicOrr {
  uint32_t register_op;   // 22 bits used.
  uint32_t immediate_op;   // 22 bits used.
};

struct SimdCmp {
  uint32_t register_op;
  uint32_t zero_op : 22;
  uint32_t vec_op_type : 6;
};

struct SimdDot {
  uint32_t vector_op;     // 22 bits used.
  uint32_t element_op;    // 22 bits used.
  uint8_t ta;             // Element-type of the first operand.
  uint8_t tb;             // Element-type of the second and third operands.
  uint8_t tElement;       // Element-type of the element index[] operand.
};

struct SimdMoviMvni {
  uint32_t opcode : 31;
  uint32_t inverted : 1;
};

struct SimdLdSt {
  uint32_t u_offset_op  : 10;
  uint32_t pre_post_op  : 11;
  uint32_t register_op : 11;
  uint32_t literal_op  : 8;
  uint32_t u_alt_inst_id : 16;
};

struct SimdLdNStN {
  uint32_t single_op;
  uint32_t multiple_op : 22;
  uint32_t n : 3;
  uint32_t replicate : 1;
};

struct SimdLdpStp {
  uint32_t offset_op : 10;
  uint32_t pre_post_op : 10;
};

struct SimdLdurStur {
  uint32_t opcode;
};

struct ISimdPair {
  uint32_t opcode2;       // 22 bits used.
  uint32_t opcode3 : 26;  // 22 bits used.
  uint32_t op_type3 : 6;
};

struct SimdShift {
  uint32_t register_op;       // 22 bits used.
  uint32_t immediate_op : 22; // 22 bits used.
  uint32_t inverted_imm : 1;
  uint32_t vec_op_type : 6;
};

struct SimdShiftES {
  uint32_t opcode : 22;
  uint32_t vec_op_type : 6;
};

struct SimdSm3tt {
  uint32_t opcode;
};

struct SimdSmovUmov {
  uint32_t opcode : 22;
  uint32_t vec_op_type : 6;
  uint32_t is_signed : 1;
};

struct SimdSxtlUxtl {
  uint32_t opcode : 22;
  uint32_t vec_op_type : 6;
};

struct SimdTblTbx {
  uint32_t opcode;
};

#undef M_OPCODE

// ${EncodingDataForward:Begin}
// ------------------- Automatically generated, do not edit -------------------
extern const BaseAddSub baseAddSub[4];
extern const BaseAdr baseAdr[2];
extern const BaseAtDcIcTlbi baseAtDcIcTlbi[4];
extern const BaseAtomicCasp baseAtomicCasp[4];
extern const BaseAtomicOp baseAtomicOp[123];
extern const BaseAtomicSt baseAtomicSt[48];
extern const BaseBfc baseBfc[1];
extern const BaseBfi baseBfi[3];
extern const BaseBfm baseBfm[3];
extern const BaseBfx baseBfx[3];
extern const BaseBranchCmp baseBranchCmp[2];
extern const BaseBranchReg baseBranchReg[3];
extern const BaseBranchRel baseBranchRel[3];
extern const BaseBranchTst baseBranchTst[2];
extern const BaseCCmp baseCCmp[2];
extern const BaseCInc baseCInc[3];
extern const BaseCSel baseCSel[4];
extern const BaseCSet baseCSet[2];
extern const BaseCmpCmn baseCmpCmn[2];
extern const BaseExtend baseExtend[5];
extern const BaseExtract baseExtract[1];
extern const BaseLdSt baseLdSt[9];
extern const BaseLdpStp baseLdpStp[6];
extern const BaseLdxp baseLdxp[2];
extern const BaseLogical baseLogical[8];
extern const BaseMinMax baseMinMax[4];
extern const BaseMovKNZ baseMovKNZ[3];
extern const BaseMvnNeg baseMvnNeg[3];
extern const BaseOp baseOp[24];
extern const BaseOpImm baseOpImm[15];
extern const BaseOpX16 baseOpX16[1];
extern const BasePrfm basePrfm[1];
extern const BaseR baseR[10];
extern const BaseRM_NoImm baseRM_NoImm[21];
extern const BaseRM_SImm10 baseRM_SImm10[2];
extern const BaseRM_SImm9 baseRM_SImm9[23];
extern const BaseRR baseRR[18];
extern const BaseRRII baseRRII[2];
extern const BaseRRR baseRRR[26];
extern const BaseRRRR baseRRRR[6];
extern const BaseShift baseShift[8];
extern const BaseStx baseStx[3];
extern const BaseStxp baseStxp[2];
extern const BaseTst baseTst[1];
extern const FSimdPair fSimdPair[5];
extern const FSimdSV fSimdSV[4];
extern const FSimdVV fSimdVV[17];
extern const FSimdVVV fSimdVVV[13];
extern const FSimdVVVV fSimdVVVV[4];
extern const FSimdVVVe fSimdVVVe[4];
extern const ISimdPair iSimdPair[1];
extern const ISimdSV iSimdSV[7];
extern const ISimdVV iSimdVV[29];
extern const ISimdVVV iSimdVVV[65];
extern const ISimdVVVI iSimdVVVI[2];
extern const ISimdVVVV iSimdVVVV[2];
extern const ISimdVVVVx iSimdVVVVx[1];
extern const ISimdVVVe iSimdVVVe[25];
extern const ISimdVVVx iSimdVVVx[17];
extern const ISimdVVx iSimdVVx[13];
extern const ISimdWWV iSimdWWV[8];
extern const SimdBicOrr simdBicOrr[2];
extern const SimdCmp simdCmp[7];
extern const SimdDot simdDot[5];
extern const SimdFcadd simdFcadd[1];
extern const SimdFccmpFccmpe simdFccmpFccmpe[2];
extern const SimdFcm simdFcm[5];
extern const SimdFcmla simdFcmla[1];
extern const SimdFcmpFcmpe simdFcmpFcmpe[2];
extern const SimdFcvtLN simdFcvtLN[6];
extern const SimdFcvtSV simdFcvtSV[12];
extern const SimdFmlal simdFmlal[6];
extern const SimdLdNStN simdLdNStN[12];
extern const SimdLdSt simdLdSt[2];
extern const SimdLdpStp simdLdpStp[4];
extern const SimdLdurStur simdLdurStur[2];
extern const SimdMoviMvni simdMoviMvni[2];
extern const SimdShift simdShift[40];
extern const SimdShiftES simdShiftES[2];
extern const SimdSm3tt simdSm3tt[4];
extern const SimdSmovUmov simdSmovUmov[2];
extern const SimdSxtlUxtl simdSxtlUxtl[4];
extern const SimdTblTbx simdTblTbx[2];
// ----------------------------------------------------------------------------
// ${EncodingDataForward:End}

} // {EncodingData}

// a64::InstDB - Tables
// ====================

#ifndef ASMJIT_NO_TEXT
extern const InstNameIndex _inst_name_index;
extern const char _inst_name_string_table[];
extern const uint32_t _inst_name_index_table[];
#endif // !ASMJIT_NO_TEXT

} // {InstDB}

//! \}
//! \endcond

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_A64_ARMINSTDB_H_P_INCLUDED

