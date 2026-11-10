// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_UJIT_UNICOMPILER_H_INCLUDED
#define ASMJIT_UJIT_UNICOMPILER_H_INCLUDED

#include <asmjit/ujit/ujitbase.h>
#include <asmjit/ujit/uniop.h>
#include <asmjit/ujit/vecconsttable.h>

#if !defined(ASMJIT_NO_UJIT)

ASMJIT_BEGIN_SUB_NAMESPACE(ujit)

//! \addtogroup asmjit_ujit
//! \{

class UniCondition;

//! Universal compiler.
class UniCompiler {
public:
  ASMJIT_NONCOPYABLE(UniCompiler)

  enum : uint32_t { kMaxKRegConstCount = 4 };

  //! \name Constants
  //! \{

  enum class StackId : uint32_t {
    kIndex,
    kCustom,
    kMaxValue = kCustom
  };

#if defined(ASMJIT_UJIT_X86)
  enum class GPExt : uint8_t {
    kADX,
    kBMI,
    kBMI2,
    kLZCNT,
    kMOVBE,
    kPOPCNT,

    kIntrin = 31
  };

  enum class SSEExt : uint8_t {
    kSSE2 = 0,
    kSSE3,
    kSSSE3,
    kSSE4_1,
    kSSE4_2,
    kPCLMULQDQ,

    //! Just to distinguish between a baseline instruction and intrinsic at operation info level.
    kIntrin = 7
  };

  enum class AVXExt : uint8_t  {
    kAVX = 0,
    kAVX2,
    kF16C,
    kFMA,
    kGFNI,
    kVAES,
    kVPCLMULQDQ,
    kAVX_IFMA,
    kAVX_NE_CONVERT,
    kAVX_VNNI,
    kAVX_VNNI_INT8,
    kAVX_VNNI_INT16,
    kAVX512,
    kAVX512_BF16,
    kAVX512_BITALG,
    kAVX512_FP16,
    kAVX512_IFMA,
    kAVX512_VBMI,
    kAVX512_VBMI2,
    kAVX512_VNNI,
    kAVX512_VPOPCNTDQ,

    //! Just to distinguish between a baseline instruction and intrinsic at operation info level.
    kIntrin = 63
  };
#endif // ASMJIT_UJIT_X86

#if defined(ASMJIT_UJIT_AARCH64)
  enum class GPExt : uint8_t {
    kCSSC,
    kFLAGM,
    kFLAGM2,
    kLS64,
    kLS64_V,
    kLSE,
    kLSE128,
    kLSE2,

    kIntrin = 31
  };

  enum class ASIMDExt : uint8_t {
    kASIMD,
    kBF16,
    kDOTPROD,
    kFCMA,
    kFHM,
    kFP16,
    kFP16CONV,
    kFP8,
    kFRINTTS,
    kI8MM,
    kJSCVT,
    kPMULL,
    kRDM,
    kSHA1,
    kSHA256,
    kSHA3,
    kSHA512,
    kSM3,
    kSM4,

    kIntrin = 63
  };

#endif // ASMJIT_UJIT_AARCH64

  //! \}

  //! \name Members
  //! \{

  //! AsmJit compiler.
  BackendCompiler* cc = nullptr;

  //! Reference to a table that provides global constants.
  //!
  //! \note This table can be extended by users so it fits a particular use-case, see \ref UniCompiler constructor.
  VecConstTableRef _ct_ref;

#if defined(ASMJIT_UJIT_X86)
  //! General purpose extension mask (X86 and X86_64 only).
  uint32_t _gp_ext_mask {};
  //! SSE extension mask (X86 and X86_64 only).
  uint32_t _sse_ext_mask {};
  //! AVX extension mask (X86 and X86_64 only).
  uint64_t _avx_ext_mask {};
#endif // ASMJIT_UJIT_X86

#if defined(ASMJIT_UJIT_AARCH64)
  //! General purpose extension mask (AArch64).
  uint64_t _gp_ext_mask {};
  //! NEON extensions (AArch64).
  uint64_t _asimd_ext_mask {};
#endif // ASMJIT_UJIT_AARCH64

  //! The behavior of scalar operations (mostly floating point).
  ScalarOpBehavior _scalar_op_behavior {};
  //! The behavior of floating point min/max operation.
  FMinFMaxOpBehavior _fmin_fmax_op_behavior {};
  //! The behavior of floating point `madd` operation.
  FMAddOpBehavior _fmadd_op_behavior {};
  //! The behavior of a float-to-int conversion when the float is out of integer range, infinite, or NaN.
  FloatToIntOutsideRangeBehavior _float_to_int_outside_range_behavior {};

  //! Target CPU features.
  CpuFeatures _features {};
  //! Optimization flags.
  CpuHints _cpu_hints {};

  //! Number of available vector registers.
  uint32_t _vec_reg_count = 0;

  //! SIMD width.
  VecWidth _vec_width = VecWidth::k128;
  //! SIMD multiplier, derived from `_vec_width` (1, 2, 4).
  uint8_t _vec_multiplier = 0;
  //! SIMD register type (AsmJit).
  RegType _vec_reg_type = RegType::kNone;
  //! SIMD type id (AsmJit).
  TypeId _vec_type_id = TypeId::kVoid;

  //! Function initialization hook.
  BaseNode* _func_init_hook = nullptr;

  //! Invalid GP register.
  Gp _gp_none;
  //! Temporary stack used to transfer SIMD regs to GP.
  Mem _tmp_stack[size_t(StackId::kMaxValue) + 1];

  //! Offset to the first constant to the `commonTable` global.
  int32_t _common_table_offset = 0;
  //! Pointer to the `commonTable` constant pool (only used in 64-bit mode).
  Gp _common_table_ptr;

#if defined(ASMJIT_UJIT_X86)
  x86::KReg _k_reg[kMaxKRegConstCount];
  uint64_t _k_imm[kMaxKRegConstCount] {};
#endif // ASMJIT_UJIT_X86

  struct VecConstData {
    const void* ptr;
    uint32_t virt_reg_id;
  };

  struct VecConstDataEx {
    uint8_t data[16];
    uint32_t virt_reg_id;
  };

  ArenaVector<VecConstData> _vec_consts;
  ArenaVector<VecConstDataEx> _vec_consts_ex;

  //! \}

  //! \name Construction & Destruction
  //! \{

  //! Creates `UniCompiler` that would use the existing BackendCompiler (it would keep the pointer to it).
  ASMJIT_API UniCompiler(BackendCompiler* cc, const CpuFeatures& features, CpuHints cpu_hints, VecConstTableRef ct_ref) noexcept;

  //! Creates `UniCompiler` that would use the existing BackendCompiler (it would keep the pointer to it).
  ASMJIT_INLINE UniCompiler(BackendCompiler* cc, const CpuFeatures& features, CpuHints cpu_hints) noexcept
    : UniCompiler(cc, features, cpu_hints, VecConstTableRef{vec_const_table, sizeof(VecConstTable)}) {}

  //! Destroys `UniCompiler` - the existing BackendCompiler would be untouched.
  ASMJIT_API ~UniCompiler() noexcept;

  //! \}

  //! \name Allocators
  //! \{

  //! Returns the arena used by `UniCompiler`.
  ASMJIT_INLINE_NODEBUG Arena& arena() noexcept { return cc->_builder_arena; }

  //! \}

  //! \name Constant Table
  //! \{

  template<typename T = VecConstTable>
  ASMJIT_INLINE_NODEBUG const T& ct() const noexcept { return static_cast<const T&>(_ct_ref.table); }

  template<typename T = VecConstTable>
  ASMJIT_INLINE_NODEBUG const T* ct_ptr() const noexcept { return static_cast<const T*>(&_ct_ref.table); }

  ASMJIT_INLINE_NODEBUG size_t ct_size() const noexcept { return _ct_ref.size; }

  //! \}

  //! \name CPU Architecture, Features and Optimization Options
  //! \{

  ASMJIT_API void _init_extensions(const CpuFeatures& features) noexcept;

  ASMJIT_INLINE_NODEBUG bool is_32bit() const noexcept { return cc->is_32bit(); }
  ASMJIT_INLINE_NODEBUG bool is_64bit() const noexcept { return cc->is_64bit(); }
  ASMJIT_INLINE_NODEBUG uint32_t register_size() const noexcept { return cc->register_size(); }

#if defined(ASMJIT_UJIT_X86)
  //! Tests whether a general purpose extension `ext` is available.
  ASMJIT_INLINE_NODEBUG bool has_gp_ext(GPExt ext) const noexcept { return (_gp_ext_mask & (1u << uint32_t(ext))) != 0; }
  //! Tests whether an SSE extension `ext` is available.
  ASMJIT_INLINE_NODEBUG bool has_sse_ext(SSEExt ext) const noexcept { return (_sse_ext_mask & (1u << uint32_t(ext))) != 0; }
  //! Tests whether an AVX or AVX-512 extension `ext` is available.
  ASMJIT_INLINE_NODEBUG bool has_avx_ext(AVXExt ext) const noexcept { return (_avx_ext_mask & (uint64_t(1) << uint32_t(ext))) != 0; }

  //! Tests whether ADX extension is available.
  ASMJIT_INLINE_NODEBUG bool has_adx() const noexcept { return has_gp_ext(GPExt::kADX); }
  //! Tests whether BMI extension is available.
  ASMJIT_INLINE_NODEBUG bool has_bmi() const noexcept { return has_gp_ext(GPExt::kBMI); }
  //! Tests whether BMI2 extension is available.
  ASMJIT_INLINE_NODEBUG bool has_bmi2() const noexcept { return has_gp_ext(GPExt::kBMI2); }
  //! Tests whether LZCNT extension is available.
  ASMJIT_INLINE_NODEBUG bool has_lzcnt() const noexcept { return has_gp_ext(GPExt::kLZCNT); }
  //! Tests whether MOVBE extension is available.
  ASMJIT_INLINE_NODEBUG bool has_movbe() const noexcept { return has_gp_ext(GPExt::kMOVBE); }
  //! Tests whether POPCNT extension is available.
  ASMJIT_INLINE_NODEBUG bool has_popcnt() const noexcept { return has_gp_ext(GPExt::kPOPCNT); }

  //! Tests whether SSE2 extensions are available (this should always return true).
  ASMJIT_INLINE_NODEBUG bool has_sse2() const noexcept { return has_sse_ext(SSEExt::kSSE2); }
  //! Tests whether SSE3 extension is available.
  ASMJIT_INLINE_NODEBUG bool has_sse3() const noexcept { return has_sse_ext(SSEExt::kSSE3); }
  //! Tests whether SSSE3 extension is available.
  ASMJIT_INLINE_NODEBUG bool has_ssse3() const noexcept { return has_sse_ext(SSEExt::kSSSE3); }
  //! Tests whether SSE4.1 extension is available.
  ASMJIT_INLINE_NODEBUG bool has_sse4_1() const noexcept { return has_sse_ext(SSEExt::kSSE4_1); }
  //! Tests whether SSE4.2 extension is available.
  ASMJIT_INLINE_NODEBUG bool has_sse4_2() const noexcept { return has_sse_ext(SSEExt::kSSE4_2); }
  //! Tests whether PCLMULQDQ extension is available.
  ASMJIT_INLINE_NODEBUG bool has_pclmulqdq() const noexcept { return has_sse_ext(SSEExt::kPCLMULQDQ); }

  //! Tests whether AVX extension is available.
  ASMJIT_INLINE_NODEBUG bool has_avx() const noexcept { return has_avx_ext(AVXExt::kAVX); }
  //! Tests whether AVX2 extension is available.
  ASMJIT_INLINE_NODEBUG bool has_avx2() const noexcept { return has_avx_ext(AVXExt::kAVX2); }
  //! Tests whether F16C extension is available.
  ASMJIT_INLINE_NODEBUG bool has_f16c() const noexcept { return has_avx_ext(AVXExt::kF16C); }
  //! Tests whether FMA extension is available.
  ASMJIT_INLINE_NODEBUG bool has_fma() const noexcept { return has_avx_ext(AVXExt::kFMA); }
  //! Tests whether GFNI extension is available.
  ASMJIT_INLINE_NODEBUG bool has_gfni() const noexcept { return has_avx_ext(AVXExt::kGFNI); }
  //! Tests whether VPCLMULQDQ extension is available.
  ASMJIT_INLINE_NODEBUG bool has_vpclmulqdq() const noexcept { return has_avx_ext(AVXExt::kVPCLMULQDQ); }

  //! Tests whether AVX_IFMA extension is available.
  ASMJIT_INLINE_NODEBUG bool has_avx_ifma() const noexcept { return has_avx_ext(AVXExt::kAVX_IFMA); }
  //! Tests whether AVX_NE_CONVERT extension is available.
  ASMJIT_INLINE_NODEBUG bool has_avx_ne_convert() const noexcept { return has_avx_ext(AVXExt::kAVX_NE_CONVERT); }
  //! Tests whether AVX_VNNI extension is available.
  ASMJIT_INLINE_NODEBUG bool has_avx_vnni() const noexcept { return has_avx_ext(AVXExt::kAVX_VNNI); }
  //! Tests whether AVX_VNNI_INT8 extension is available.
  ASMJIT_INLINE_NODEBUG bool has_avx_vnni_int8() const noexcept { return has_avx_ext(AVXExt::kAVX_VNNI_INT8); }
  //! Tests whether AVX_VNNI_INT16 extension is available.
  ASMJIT_INLINE_NODEBUG bool has_avx_vnni_int16() const noexcept { return has_avx_ext(AVXExt::kAVX_VNNI_INT16); }

  //! Tests whether a baseline AVX-512 extensions are available (F, CD, BW, DQ, and VL).
  ASMJIT_INLINE_NODEBUG bool has_avx512() const noexcept { return has_avx_ext(AVXExt::kAVX512); }
  //! Tests whether AVX512_BF16 extension is available.
  ASMJIT_INLINE_NODEBUG bool has_avx512_bf16() const noexcept { return has_avx_ext(AVXExt::kAVX512_BF16); }
  //! Tests whether AVX512_BITALT extension is available.
  ASMJIT_INLINE_NODEBUG bool has_avx512_bitalg() const noexcept { return has_avx_ext(AVXExt::kAVX512_BITALG); }
  //! Tests whether AVX512_FP16 extension is available.
  ASMJIT_INLINE_NODEBUG bool has_avx512_fp16() const noexcept { return has_avx_ext(AVXExt::kAVX512_FP16); }
  //! Tests whether AVX512_IFMA extension is available.
  ASMJIT_INLINE_NODEBUG bool has_avx512_ifma() const noexcept { return has_avx_ext(AVXExt::kAVX512_IFMA); }
  //! Tests whether AVX512_VBMI extension is available.
  ASMJIT_INLINE_NODEBUG bool has_avx512_vbmi() const noexcept { return has_avx_ext(AVXExt::kAVX512_VBMI); }
  //! Tests whether AVX512_VBMI2 extension is available.
  ASMJIT_INLINE_NODEBUG bool has_avx512_vbmi2() const noexcept { return has_avx_ext(AVXExt::kAVX512_VBMI2); }
  //! Tests whether AVX512_VNNI extension is available.
  ASMJIT_INLINE_NODEBUG bool has_avx512_vnni() const noexcept { return has_avx_ext(AVXExt::kAVX512_VNNI); }
  //! Tests whether AVX512_VPOPCNTDQ extension is available.
  ASMJIT_INLINE_NODEBUG bool has_avx512_vpopcntdq() const noexcept { return has_avx_ext(AVXExt::kAVX512_VPOPCNTDQ); }

  //! Tests whether the target SIMD ISA provides instructions with non-destructive source operand (AVX+).
  ASMJIT_INLINE_NODEBUG bool has_non_destructive_src() const noexcept { return has_avx(); }

#endif // ASMJIT_UJIT_X86

#if defined(ASMJIT_UJIT_AARCH64)
  //! Tests whether a general purpose extension `ext` is available.
  ASMJIT_INLINE_NODEBUG bool has_gp_ext(GPExt ext) const noexcept { return (_gp_ext_mask & (uint64_t(1) << uint32_t(ext))) != 0; }
  //! Tests whether an ASIMD extension `ext` is available.
  ASMJIT_INLINE_NODEBUG bool has_asimd_ext(ASIMDExt ext) const noexcept { return (_asimd_ext_mask & (uint64_t(1) << uint32_t(ext))) != 0; }

  //! Tests whether CSSC extension is available.
  ASMJIT_INLINE_NODEBUG bool has_cssc() const noexcept { return has_gp_ext(GPExt::kCSSC); }
  //! Tests whether FLAGM extension is available.
  ASMJIT_INLINE_NODEBUG bool has_flagm() const noexcept { return has_gp_ext(GPExt::kFLAGM); }
  //! Tests whether FLAGM2 extension is available.
  ASMJIT_INLINE_NODEBUG bool has_flagm2() const noexcept { return has_gp_ext(GPExt::kFLAGM2); }
  //! Tests whether LS64 extension is available.
  ASMJIT_INLINE_NODEBUG bool has_ls64() const noexcept { return has_gp_ext(GPExt::kLS64); }
  //! Tests whether LS64_V extension is available.
  ASMJIT_INLINE_NODEBUG bool has_ls64_v() const noexcept { return has_gp_ext(GPExt::kLS64_V); }
  //! Tests whether LSE extension is available.
  ASMJIT_INLINE_NODEBUG bool has_lse() const noexcept { return has_gp_ext(GPExt::kLSE); }
  //! Tests whether LSE128 extension is available.
  ASMJIT_INLINE_NODEBUG bool has_lse128() const noexcept { return has_gp_ext(GPExt::kLSE128); }
  //! Tests whether LSE2 extension is available.
  ASMJIT_INLINE_NODEBUG bool has_lse2() const noexcept { return has_gp_ext(GPExt::kLSE2); }

  //! Tests whether ASIMD extension is available (must always return true).
  ASMJIT_INLINE_NODEBUG bool has_asimd() const noexcept { return has_asimd_ext(ASIMDExt::kASIMD); }
  //! Tests whether BF16 extension is available.
  ASMJIT_INLINE_NODEBUG bool has_bf16() const noexcept { return has_asimd_ext(ASIMDExt::kBF16); }
  //! Tests whether DOTPROD extension is available.
  ASMJIT_INLINE_NODEBUG bool has_dotprod() const noexcept { return has_asimd_ext(ASIMDExt::kDOTPROD); }
  //! Tests whether FCMA extension is available.
  ASMJIT_INLINE_NODEBUG bool has_fcma() const noexcept { return has_asimd_ext(ASIMDExt::kFCMA); }
  //! Tests whether FHM extension is available.
  ASMJIT_INLINE_NODEBUG bool has_fhm() const noexcept { return has_asimd_ext(ASIMDExt::kFHM); }
  //! Tests whether FP16 extension is available.
  ASMJIT_INLINE_NODEBUG bool has_fp16() const noexcept { return has_asimd_ext(ASIMDExt::kFP16); }
  //! Tests whether FP16CONV extension is available.
  ASMJIT_INLINE_NODEBUG bool has_fp16conv() const noexcept { return has_asimd_ext(ASIMDExt::kFP16CONV); }
  //! Tests whether FP8 extension is available.
  ASMJIT_INLINE_NODEBUG bool has_fp8() const noexcept { return has_asimd_ext(ASIMDExt::kFP8); }
  //! Tests whether FRINTTS extension is available.
  ASMJIT_INLINE_NODEBUG bool has_frintts() const noexcept { return has_asimd_ext(ASIMDExt::kFRINTTS); }
  //! Tests whether I8MM extension is available.
  ASMJIT_INLINE_NODEBUG bool has_i8mm() const noexcept { return has_asimd_ext(ASIMDExt::kI8MM); }
  //! Tests whether JSCVT extension is available.
  ASMJIT_INLINE_NODEBUG bool has_jscvt() const noexcept { return has_asimd_ext(ASIMDExt::kJSCVT); }
  //! Tests whether PMULL extension is available.
  ASMJIT_INLINE_NODEBUG bool has_pmull() const noexcept { return has_asimd_ext(ASIMDExt::kPMULL); }
  //! Tests whether RDM extension is available.
  ASMJIT_INLINE_NODEBUG bool has_rdm() const noexcept { return has_asimd_ext(ASIMDExt::kRDM); }
  //! Tests whether SHA1 extension is available.
  ASMJIT_INLINE_NODEBUG bool has_sha1() const noexcept { return has_asimd_ext(ASIMDExt::kSHA1); }
  //! Tests whether SHA256 extension is available.
  ASMJIT_INLINE_NODEBUG bool has_sha256() const noexcept { return has_asimd_ext(ASIMDExt::kSHA256); }
  //! Tests whether SHA3 extension is available.
  ASMJIT_INLINE_NODEBUG bool has_sha3() const noexcept { return has_asimd_ext(ASIMDExt::kSHA3); }
  //! Tests whether SHA512 extension is available.
  ASMJIT_INLINE_NODEBUG bool has_sha512() const noexcept { return has_asimd_ext(ASIMDExt::kSHA512); }
  //! Tests whether SM3 extension is available.
  ASMJIT_INLINE_NODEBUG bool has_sm3() const noexcept { return has_asimd_ext(ASIMDExt::kSM3); }
  //! Tests whether SM4 extension is available.
  ASMJIT_INLINE_NODEBUG bool has_sm4() const noexcept { return has_asimd_ext(ASIMDExt::kSM4); }

  //! Tests whether the target SIMD ISA provides instructions with non-destructive destination (always on AArch64).
  ASMJIT_INLINE_NODEBUG bool has_non_destructive_src() const noexcept { return true; }

#endif // ASMJIT_UJIT_AARCH64

  //! Returns the behavior of scalar operations (mostly floating point).
  ASMJIT_INLINE_NODEBUG ScalarOpBehavior scalar_op_behavior() const noexcept { return _scalar_op_behavior; }
  //! Returns the behavior of floating point min/max operations.
  ASMJIT_INLINE_NODEBUG FMinFMaxOpBehavior fmin_fmax_op_behavior() const noexcept { return _fmin_fmax_op_behavior; }
  //! Returns the behavior of floating point mul+add (`madd`) operations.
  ASMJIT_INLINE_NODEBUG FMAddOpBehavior fmadd_op_behavior() const noexcept { return _fmadd_op_behavior; }
  //! Returns the behavior of float-to-integer conversion when the floating point is outside of the integer representable
  //! range, infinite, or NaN.
  ASMJIT_INLINE_NODEBUG FloatToIntOutsideRangeBehavior float_to_int_outside_range_behavior() const noexcept { return _float_to_int_outside_range_behavior; }

  //! Tests whether a scalar operation is zeroing the rest of the destination register (AArch64).
  ASMJIT_INLINE_NODEBUG bool is_scalar_op_zeroing() const noexcept { return _scalar_op_behavior == ScalarOpBehavior::kZeroing; }
  //! Tests whether a scalar operation is preserving the low 128-bit part of the destination register (X86|X86_64).
  ASMJIT_INLINE_NODEBUG bool is_scalar_op_preserving_vec128() const noexcept { return _scalar_op_behavior == ScalarOpBehavior::kPreservingVec128; }

  //! Tests whether a floating point min/max operation selects a finite value if one of the values is NaN (AArch64).
  ASMJIT_INLINE_NODEBUG bool is_fmin_fmax_finite() const noexcept { return _fmin_fmax_op_behavior == FMinFMaxOpBehavior::kFiniteValue; }
  //! Tests whether a floating point min/max operation works as a ternary if - `if a <|> b ? a : b` (X86|X86_64).
  ASMJIT_INLINE_NODEBUG bool is_fmin_fmax_ternary() const noexcept { return _fmin_fmax_op_behavior == FMinFMaxOpBehavior::kTernaryLogic; }

  //! Tests whether a floating point mul+add operation is fused (uses FMA).
  ASMJIT_INLINE_NODEBUG bool is_fmadd_fused() const noexcept { return _fmadd_op_behavior != FMAddOpBehavior::kNoFMA; }
  //! Tests whether a FMA operation is available and that it can store the result to any register (true of X86).
  ASMJIT_INLINE_NODEBUG bool is_fma_storing_to_any_register() const noexcept { return _fmadd_op_behavior == FMAddOpBehavior::kFMAStoreToAny; }
  //! Tests whether a FMA operation is available and that it only stores the result to accumulator register.
  ASMJIT_INLINE_NODEBUG bool is_fma_storing_to_any_accumulator() const noexcept { return _fmadd_op_behavior == FMAddOpBehavior::kFMAStoreToAccumulator; }

  //! Returns CPU hints.
  ASMJIT_INLINE_NODEBUG CpuHints cpu_hints() const noexcept { return _cpu_hints; }
  //! Tests whether a CPU hint `hint` is enabled.
  ASMJIT_INLINE_NODEBUG bool has_cpu_hint(CpuHints hint) const noexcept { return Support::test(_cpu_hints, hint); }

  //! Returns a native register signature, either 32-bit or 64-bit depending on the target architecture).
  ASMJIT_INLINE_NODEBUG OperandSignature gp_signature() const noexcept { return cc->gp_signature(); }
  //! Clones the given `reg` register into a native register (either 32-bit or 64-bit depending on the target architecture).
  ASMJIT_INLINE_NODEBUG Gp gpz(const Gp& reg) const noexcept { return cc->gpz(reg); }

  ASMJIT_INLINE_NODEBUG uint32_t vec_reg_count() const noexcept { return _vec_reg_count; }

  ASMJIT_API VecWidth max_vec_width_from_cpu_features() noexcept;
  ASMJIT_API void init_vec_width(VecWidth vw) noexcept;

  ASMJIT_API bool has_masked_access_of(uint32_t data_size) const noexcept;

  //! \}

  //! \name CPU SIMD Width and SIMD Width Utilities
  //! \{

  //! Returns the current SIMD width (in bytes) that this compiler and all its parts must use.
  //!
  //! \note The returned width is in bytes and it's calculated from the maximum supported widths of all pipeline parts.
  //! This means that SIMD width returned could be actually lower than a SIMD width supported by the target CPU.
  ASMJIT_INLINE_NODEBUG VecWidth vec_width() const noexcept { return _vec_width; }

  //! Returns whether the compiler and all parts use 256-bit SIMD.
  ASMJIT_INLINE_NODEBUG bool use_256bit_simd() const noexcept { return _vec_width >= VecWidth::k256; }
  //! Returns whether the compiler and all parts use 512-bit SIMD.
  ASMJIT_INLINE_NODEBUG bool use_512bit_simd() const noexcept { return _vec_width >= VecWidth::k512; }

  //! Returns a constant that can be used to multiply a baseline SIMD width to get the value returned by `vec_width()`.
  //!
  //! \note A baseline SIMD width would be 16 bytes on most platforms.
  ASMJIT_INLINE_NODEBUG uint32_t vec_multiplier() const noexcept { return _vec_multiplier; }

  ASMJIT_INLINE_NODEBUG VecWidth vec_width_of(DataWidth data_width, uint32_t n) const noexcept { return VecWidthUtils::vec_width_of(vec_width(), data_width, n); }

  //! \}

  //! \name Labels
  //! \{

  //! Creates a new anonymous label.
  //!
  //! See \ref BaseEmitter::new_label() for more details.
  [[nodiscard]]
  ASMJIT_INLINE Label new_label() {
    return cc->new_label();
  }

  //! Creates a new named label.
  //!
  //! See \ref BaseEmitter::new_named_label() for more details.
  [[nodiscard]]
  ASMJIT_INLINE Label new_named_label(const char* name, size_t name_size = SIZE_MAX, LabelType type = LabelType::kGlobal, uint32_t parent_id = Globals::kInvalidId) {
    return cc->new_named_label(name, name_size, type, parent_id);
  }

  //! \overload
  [[nodiscard]]
  ASMJIT_INLINE Label new_named_label(Span<const char> name, LabelType type = LabelType::kGlobal, uint32_t parent_id = Globals::kInvalidId) {
    return cc->new_named_label(name, type, parent_id);
  }

  //! Creates a new anonymous label with a name, which can only be used for debugging purposes.
  //!
  //! See \ref BaseEmitter::new_anonymous_label() for more details.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Label new_anonymous_label(const char* name, size_t name_size = SIZE_MAX) {
    return cc->new_anonymous_label(name, name_size);
  }

  //! \overload
  [[nodiscard]]
  ASMJIT_INLINE Label new_anonymous_label(Span<const char> name) {
    return cc->new_anonymous_label(name);
  }

  //! Creates a new external label.
  //!
  //! See \ref BaseEmitter::new_external_label() for more details.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Label new_external_label(const char* name, size_t name_size = SIZE_MAX) {
    return cc->new_external_label(name, name_size);
  }

  //! \overload
  [[nodiscard]]
  ASMJIT_INLINE Label new_external_label(Span<const char> name) {
    return cc->new_external_label(name);
  }

  //! Binds the `label` to the current position of the current section.
  //!
  //! See \ref BaseEmitter::bind() for more details.
  ASMJIT_INLINE Error bind(const Label& label) {
    return cc->bind(label);
  }

  //! \}

  //! \name Align
  //! \{

  //! Aligns the current position to the `alignment` specified.
  //!
  //! See \ref BaseEmitter::align() for more details.
  ASMJIT_INLINE Error align(AlignMode align_mode, uint32_t alignment) {
    return cc->align(align_mode, alignment);
  }

  //! \name Embed
  //! \{

  //! Embeds raw data into the instruction stream.
  //!
  //! See \ref BaseEmitter::embed() for more details.
  ASMJIT_INLINE Error embed(const void* data, size_t data_size) {
    return cc->embed(data, data_size);
  }

  //! Embeds a typed data array.
  //!
  //! See \ref BaseEmitter::embed_data_array() for more details.
  ASMJIT_INLINE Error embed_data_array(TypeId type_id, const void* data, size_t item_count, size_t repeat_count = 1) {
    return cc->embed_data_array(type_id, data, item_count, repeat_count);
  }

  //! Embeds int8_t `value` repeated by `repeat_count`.
  ASMJIT_INLINE Error embed_int8(int8_t value, size_t repeat_count = 1) { return cc->embed_int8(value, repeat_count); }
  //! Embeds uint8_t `value` repeated by `repeat_count`.
  ASMJIT_INLINE Error embed_uint8(uint8_t value, size_t repeat_count = 1) { return cc->embed_uint8(value, repeat_count); }
  //! Embeds int16_t `value` repeated by `repeat_count`.
  ASMJIT_INLINE Error embed_int16(int16_t value, size_t repeat_count = 1) { return cc->embed_int16(value, repeat_count); }
  //! Embeds uint16_t `value` repeated by `repeat_count`.
  ASMJIT_INLINE Error embed_uint16(uint16_t value, size_t repeat_count = 1) { return cc->embed_uint16(value, repeat_count); }
  //! Embeds int32_t `value` repeated by `repeat_count`.
  ASMJIT_INLINE Error embed_int32(int32_t value, size_t repeat_count = 1) { return cc->embed_int32(value, repeat_count); }
  //! Embeds uint32_t `value` repeated by `repeat_count`.
  ASMJIT_INLINE Error embed_uint32(uint32_t value, size_t repeat_count = 1) { return cc->embed_uint32(value, repeat_count); }
  //! Embeds int64_t `value` repeated by `repeat_count`.
  ASMJIT_INLINE Error embed_int64(int64_t value, size_t repeat_count = 1) { return cc->embed_int64(value, repeat_count); }
  //! Embeds uint64_t `value` repeated by `repeat_count`.
  ASMJIT_INLINE Error embed_uint64(uint64_t value, size_t repeat_count = 1) { return cc->embed_uint64(value, repeat_count); }
  //! Embeds a 32-bit floating point `value` repeated by `repeat_count`.
  ASMJIT_INLINE Error embed_float(float value, size_t repeat_count = 1) { return cc->embed_float(value, repeat_count); }
  //! Embeds a 64-bit floating point `value` repeated by `repeat_count`.
  ASMJIT_INLINE Error embed_double(double value, size_t repeat_count = 1) { return cc->embed_double(value, repeat_count); }

  //! Embeds a constant pool at the current offset.
  //!
  //! See \ref BaseEmitter::embed_const_pool() for more details.
  ASMJIT_INLINE Error embed_const_pool(const Label& label, const ConstPool& pool) {
    return cc->embed_const_pool(label, pool);
  }

  //! Embeds an absolute `label` address as data.
  //!
  //! See \ref BaseEmitter::embed_label() for more details.
  ASMJIT_INLINE Error embed_label(const Label& label, size_t data_size = 0) {
    return cc->embed_label(label, data_size);
  }

  //! Embeds a delta (distance) between the `label` and `base` calculating it as `label - base`.
  //!
  //! See \ref BaseEmitter::embed_label_delta() for more details.
  ASMJIT_INLINE Error embed_label_delta(const Label& label, const Label& base, size_t data_size = 0) {
    return cc->embed_label_delta(label, base, data_size);
  }

  ASMJIT_API void embed_jump_table(Span<const Label> jump_table, const Label& jump_table_base, uint32_t entry_size);

  //! \}

  //! \name Comment
  //! \{

  //! Emits a comment stored in `data` with an optional `size` parameter.
  ASMJIT_INLINE Error comment(const char* data, size_t size = SIZE_MAX) {
    return cc->comment(data, size);
  }

  //! Emits a comment passed via a `data` span.
  ASMJIT_INLINE Error comment(Span<const char> data) {
    return cc->comment(data);
  }

  //! Emits a formatted comment specified by `fmt` and variable number of arguments.
  template<typename... Args>
  ASMJIT_INLINE Error commentf(const char* fmt, Args&&... args) {
    return cc->commentf(fmt, std::forward<Args>(args)...);
  }

  //! Emits a formatted comment specified by `fmt` and `ap`.
  ASMJIT_INLINE Error commentv(const char* fmt, va_list ap) {
    return cc->commentv(fmt, ap);
  }

  //! \}

  //! \name Function Management
  //! \{

  //! Returns the function being generated.
  //!
  //! This is just a convenience wrapper that calls \ref BaseCompiler::func().
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG FuncNode* func() const noexcept { return cc->func(); }

  //! Hooks a function that is being generated by \ref BaseCompiler.
  //!
  //! This function is called automatically by \ref add_func_node() and \ref add_func(). However, if \ref BaseCompiler
  //! was called instead of \ref UniCompiler to add the function node to the instruction stream, then it has to be
  //! hooked manually.
  ASMJIT_API void hook_func() noexcept;

  //! Unhooks a function.
  //!
  //! In general this mostly does a cleanup of \ref UniCompiler.
  ASMJIT_API void unhook_func() noexcept;

  //! Creates a new \ref FuncNode.
  //!
  //! This is just a convenience wrapper that calls \ref BaseCompiler::new_func_node().
  ASMJIT_INLINE Error new_func_node(Out<FuncNode*> out, const FuncSignature& signature) {
    return cc->new_func_node(out, signature);
  }

  //! Creates a new \ref FuncNode adds it to the instruction stream.
  //!
  //! This is just a convenience wrapper that calls \ref BaseCompiler::add_func_node().
  ASMJIT_INLINE Error add_func_node(Out<FuncNode*> out, const FuncSignature& signature) {
    Error err = cc->add_func_node(out, signature);
    hook_func();
    return err;
  }

  //! Creates a new \ref FuncNode with the given `signature`, adds it to the instruction stream by using
  //! `add_func(FuncNode* func)` overload, and returns the node.
  ASMJIT_INLINE FuncNode* add_func(const FuncSignature& signature) {
    FuncNode* node;
    add_func_node(Out(node), signature);
    hook_func();
    return node;
  }

  //! Adds a function `node` to the instruction stream.
  ASMJIT_INLINE FuncNode* add_func(FuncNode* ASMJIT_NONNULL(func)) {
    FuncNode* node = cc->add_func(func);
    hook_func();
    return node;
  }

  //! Creates a new \ref FuncRetNode.
  //!
  //! This is just a convenience wrapper that calls \ref BaseCompiler::new_func_ret_node().
  ASMJIT_INLINE Error new_func_ret_node(Out<FuncRetNode*> out, const Operand_& o0, const Operand_& o1) {
    return cc->new_func_ret_node(out, o0, o1);
  }

  //! Creates a new \ref FuncRetNode and adds it to the instruction stream.
  //!
  //! This is just a convenience wrapper that calls \ref BaseCompiler::add_func_ret_node().
  ASMJIT_INLINE Error add_func_ret_node(Out<FuncRetNode*> out, const Operand_& o0, const Operand_& o1) {
    return cc->add_func_ret_node(out, o0, o1);
  }

  //! Ends the current function by emitting a sentinel that marks the end of it.
  ASMJIT_INLINE Error end_func() {
    unhook_func();
    return cc->end_func();
  }

  //! Return from function, a convenience function that calls \ref BaseCompiler::ret().
  //!
  //! \note This doesn't end the function - it just emits a return.
  ASMJIT_INLINE Error ret() { return cc->ret(); }

  //! Return from function - one value.
  //!
  //! \note This doesn't end the function - it just emits a return.
  ASMJIT_INLINE Error ret(const Reg& o0) { return cc->ret(o0); }

  //! Return from function - two values / register pair.
  //!
  //! \note This doesn't end the function - it just emits a return.
  ASMJIT_INLINE Error ret(const Reg& o0, const Reg& o1) { return cc->ret(o0, o1); }

  //! \}

  //! \name Function Finalization
  //! \{

  //! Calls \ref BaseCompiler::finalize().
  ASMJIT_INLINE Error finalize() {
    return cc->finalize();
  }

  //! \}

  //! \name Function Invocation
  //! \{

  //! Creates a new \ref InvokeNode.
  ASMJIT_INLINE Error new_invoke_node(Out<InvokeNode*> out, InstId inst_id, const Operand_& o0, const FuncSignature& signature) {
    return cc->new_invoke_node(out, inst_id, o0, signature);
  }

  //! Creates a new \ref InvokeNode and adds it to the instruction stream.
  ASMJIT_INLINE Error add_invoke_node(Out<InvokeNode*> out, InstId inst_id, const Operand_& o0, const FuncSignature& signature) {
    return cc->add_invoke_node(out, inst_id, o0, signature);
  }

  //! \}

  //! \name Virtual Registers & Memory (Target Independent)
  //! \{

  //! Wraps `BackendCompiler::new_reg(type_id, args...)`.
  template<typename RegT, typename... Args>
  [[nodiscard]]
  ASMJIT_INLINE RegT new_reg(TypeId type_id, Args&&... args) noexcept {
    return cc->new_similar_reg<RegT>(type_id, std::forward<Args>(args)...);
  }

  //! Wraps `BackendCompiler::new_similar_reg(ref, args...)`.
  template<typename RegT, typename... Args>
  [[nodiscard]]
  ASMJIT_INLINE RegT new_similar_reg(const RegT& ref, Args&&... args) noexcept {
    return cc->new_similar_reg(ref, std::forward<Args>(args)...);
  }

  //! Wraps `BackendCompiler::new_gp32(args...)`.
  template<typename... Args>
  [[nodiscard]]
  ASMJIT_INLINE Gp new_gp32(Args&&... args) noexcept {
    return cc->new_gp32(std::forward<Args>(args)...);
  }

  //! Wraps `BackendCompiler::new_gp64(args...)`.
  template<typename... Args>
  [[nodiscard]]
  ASMJIT_INLINE Gp new_gp64(Args&&... args) noexcept {
    return cc->new_gp64(std::forward<Args>(args)...);
  }

  //! Wraps `BackendCompiler::new_gpz(args...)`.
  template<typename... Args>
  [[nodiscard]]
  ASMJIT_INLINE Gp new_gpz(Args&&... args) noexcept {
    return cc->new_gpz(std::forward<Args>(args)...);
  }

  //! Wraps `BackendCompiler::new_gpz(args...)`.
  template<typename... Args>
  [[nodiscard]]
  ASMJIT_INLINE Gp new_gp_ptr(Args&&... args) noexcept {
    return cc->new_gp_ptr(std::forward<Args>(args)...);
  }

  template<typename... Args>
  [[nodiscard]]
  ASMJIT_INLINE Vec new_vec(Args&&... args) noexcept {
    return cc->new_vec(_vec_type_id, std::forward<Args>(args)...);
  }

  template<typename... Args>
  [[nodiscard]]
  ASMJIT_INLINE Vec new_vec_with_width(VecWidth vw, Args&&... args) noexcept {
    return cc->new_reg<Vec>(VecWidthUtils::type_id_of(vw), std::forward<Args>(args)...);
  }

  template<typename... Args>
  [[nodiscard]]
  ASMJIT_INLINE Vec new_vec128(Args&&... args) noexcept {
    return cc->new_vec128(std::forward<Args>(args)...);
  }

  template<typename... Args>
  [[nodiscard]]
  ASMJIT_INLINE Vec new_vec128_f32x1(Args&&... args) noexcept {
    return cc->new_vec128_f32x1(std::forward<Args>(args)...);
  }

  template<typename... Args>
  [[nodiscard]]
  ASMJIT_INLINE Vec new_vec128_f64x1(Args&&... args) noexcept {
    return cc->new_vec128_f64x1(std::forward<Args>(args)...);
  }

  template<typename... Args>
  [[nodiscard]]
  ASMJIT_INLINE Vec new_vec128_f32x4(Args&&... args) noexcept {
    return cc->new_vec128_f32x4(std::forward<Args>(args)...);
  }

  template<typename... Args>
  [[nodiscard]]
  ASMJIT_INLINE Vec new_vec128_f64x2(Args&&... args) noexcept {
    return cc->new_vec128_f64x2(std::forward<Args>(args)...);
  }

#if defined(ASMJIT_UJIT_X86)
  template<typename... Args>
  [[nodiscard]]
  ASMJIT_INLINE Vec new_vec256(Args&&... args) noexcept {
    return cc->new_vec256(std::forward<Args>(args)...);
  }

  template<typename... Args>
  [[nodiscard]]
  ASMJIT_INLINE Vec new_vec512(Args&&... args) noexcept {
    return cc->new_vec512(std::forward<Args>(args)...);
  }
#endif // ASMJIT_UJIT_X86

  ASMJIT_NOINLINE void new_reg_array(OpArray& dst, size_t n, TypeId type_id, const char* name) noexcept {
    ASMJIT_ASSERT(n <= OpArray::kMaxSize);
    dst._size = n;
    for (size_t i = 0; i < n; i++) {
      cc->_new_reg(Out(dst[i].as<Reg>()), type_id, "%s%u", name, i);
    }
  }

  ASMJIT_NOINLINE void new_reg_array(OpArray& dst, size_t n, TypeId type_id, const char* prefix, const char* name) noexcept {
    ASMJIT_ASSERT(n <= OpArray::kMaxSize);
    dst._size = n;
    for (size_t i = 0; i < n; i++) {
      cc->_new_reg(Out(dst[i].as<Reg>()), type_id, "%s%s%u", prefix, name, i);
    }
  }

  ASMJIT_NOINLINE void new_reg_array(OpArray& dst, size_t n, const Reg& ref, const char* name) noexcept {
    ASMJIT_ASSERT(n <= OpArray::kMaxSize);
    dst._size = n;
    for (size_t i = 0; i < n; i++) {
      cc->_new_reg(Out(dst[i].as<Reg>()), ref, "%s%u", name, i);
    }
  }

  ASMJIT_NOINLINE void new_reg_array(OpArray& dst, size_t n, const Reg& ref, const char* prefix, const char* name) noexcept {
    ASMJIT_ASSERT(n <= OpArray::kMaxSize);
    dst._size = n;
    for (size_t i = 0; i < n; i++) {
      cc->_new_reg(Out(dst[i].as<Reg>()), ref, "%s%s%u", prefix, name, i);
    }
  }

  ASMJIT_INLINE void new_vec_array(OpArray& dst, size_t n, VecWidth vw, const char* name) noexcept {
    new_reg_array(dst, n, VecWidthUtils::type_id_of(vw), name);
  }

  ASMJIT_INLINE void new_vec_array(OpArray& dst, size_t n, VecWidth vw, const char* prefix, const char* name) noexcept {
    new_reg_array(dst, n, VecWidthUtils::type_id_of(vw), prefix, name);
  }

  ASMJIT_INLINE void new_vec_array(OpArray& dst, size_t n, const Vec& ref, const char* name) noexcept {
    new_reg_array(dst, n, ref, name);
  }

  ASMJIT_INLINE void new_vec_array(OpArray& dst, size_t n, const Vec& ref, const char* prefix, const char* name) noexcept {
    new_reg_array(dst, n, ref, prefix, name);
  }

  ASMJIT_INLINE void new_vec128_array(OpArray& dst, size_t n, const char* name) noexcept {
    new_reg_array(dst, n, TypeId::kInt32x4, name);
  }

  ASMJIT_INLINE void new_vec128_array(OpArray& dst, size_t n, const char* prefix, const char* name) noexcept {
    new_reg_array(dst, n, TypeId::kInt32x4, prefix, name);
  }

#if defined(ASMJIT_UJIT_X86)
  ASMJIT_INLINE void new_vec256_array(OpArray& dst, size_t n, const char* name) noexcept {
    new_reg_array(dst, n, TypeId::kInt32x8, name);
  }

  ASMJIT_INLINE void new_vec256_array(OpArray& dst, size_t n, const char* prefix, const char* name) noexcept {
    new_reg_array(dst, n, TypeId::kInt32x8, prefix, name);
  }

  ASMJIT_INLINE void new_vec512_array(OpArray& dst, size_t n, const char* name) noexcept {
    new_reg_array(dst, n, TypeId::kInt32x16, name);
  }

  ASMJIT_INLINE void new_vec512_array(OpArray& dst, size_t n, const char* prefix, const char* name) noexcept {
    new_reg_array(dst, n, TypeId::kInt32x16, prefix, name);
  }
#endif // ASMJIT_UJIT_X86

  ASMJIT_API Mem tmp_stack(StackId id, uint32_t size);

  //! \}

  ASMJIT_API void _init_vec_const_table_ptr();

  //! \name Miscellaneous Helpers
  //! \{

  ASMJIT_INLINE void rename(const OpArray& op_array, const char* name) noexcept {
    for (uint32_t i = 0; i < op_array.size(); i++) {
      cc->rename(op_array[i].as<Reg>(), "%s%u", name, unsigned(i));
    }
  }

  ASMJIT_INLINE void rename(const OpArray& op_array, const char* prefix, const char* name) noexcept {
    for (uint32_t i = 0; i < op_array.size(); i++) {
      cc->rename(op_array[i].as<Reg>(), "%s%s%u", prefix, name, unsigned(i));
    }
  }

  //! \}

  //! \name Constants (X86|X86_64)
  //! \{

#if defined(ASMJIT_UJIT_X86)
  ASMJIT_API x86::KReg k_const(uint64_t value);
#endif // ASMJIT_UJIT_X86

  ASMJIT_API Operand simd_const(const void* c, Bcst bcst_width, VecWidth const_width);
  ASMJIT_API Operand simd_const(const void* c, Bcst bcst_width, const Vec& similar_to);
  ASMJIT_API Operand simd_const(const void* c, Bcst bcst_width, const VecArray& similar_to);

  ASMJIT_API Vec simd_vec_const(const void* c, Bcst bcst_width, VecWidth const_width);
  ASMJIT_API Vec simd_vec_const(const void* c, Bcst bcst_width, const Vec& similar_to);
  ASMJIT_API Vec simd_vec_const(const void* c, Bcst bcst_width, const VecArray& similar_to);

  ASMJIT_API Mem simd_mem_const(const void* c, Bcst bcst_width, VecWidth const_width);
  ASMJIT_API Mem simd_mem_const(const void* c, Bcst bcst_width, const Vec& similar_to);
  ASMJIT_API Mem simd_mem_const(const void* c, Bcst bcst_width, const VecArray& similar_to);

  ASMJIT_API Mem _get_mem_const(const void* c);
  ASMJIT_API Vec _new_vec_const(const void* c, bool is_unique_const);

#if defined(ASMJIT_UJIT_AARCH64)
  ASMJIT_API Vec simd_const_16b(const void* data16);
#endif // ASMJIT_UJIT_AARCH64

#if defined(ASMJIT_UJIT_AARCH64)
  inline Vec simd_vec_zero(const Vec& similar_to) { return simd_vec_const(&ct().p_0000000000000000, Bcst::k32, similar_to); }
#endif // ASMJIT_UJIT_AARCH64

  //! \}

  //! \name Emit - General Purpose Instructions
  //! \{

  ASMJIT_API void emit_mov(const Gp& dst, const Operand_& src);
  ASMJIT_API void emit_m(UniOpM op, const Mem& m);
  ASMJIT_API void emit_rm(UniOpRM op, const Gp& dst, const Mem& src);
  ASMJIT_API void emit_mr(UniOpMR op, const Mem& dst, const Gp& src);
  ASMJIT_API void emit_cmov(const Gp& dst, const Operand_& sel, const UniCondition& condition);
  ASMJIT_API void emit_select(const Gp& dst, const Operand_& sel1_, const Operand_& sel2_, const UniCondition& condition);
  ASMJIT_API void emit_2i(UniOpRR op, const Gp& dst, const Operand_& src_);
  ASMJIT_API void emit_3i(UniOpRRR op, const Gp& dst, const Operand_& src1_, const Operand_& src2_);
  ASMJIT_API void emit_j(const Operand_& target);
  ASMJIT_API void emit_j_if(const Label& target, const UniCondition& condition);

  ASMJIT_INLINE void mov(const Gp& dst, const Gp& src) { return emit_mov(dst, src); }
  ASMJIT_INLINE void mov(const Gp& dst, const Imm& src) { return emit_mov(dst, src); }

  ASMJIT_INLINE void load(const Gp& dst, const Mem& src) { return emit_rm(UniOpRM::kLoadReg, dst, src); }
  ASMJIT_INLINE void load_i8(const Gp& dst, const Mem& src) { return emit_rm(UniOpRM::kLoadI8, dst, src); }
  ASMJIT_INLINE void load_u8(const Gp& dst, const Mem& src) { return emit_rm(UniOpRM::kLoadU8, dst, src); }
  ASMJIT_INLINE void load_i16(const Gp& dst, const Mem& src) { return emit_rm(UniOpRM::kLoadI16, dst, src); }
  ASMJIT_INLINE void load_u16(const Gp& dst, const Mem& src) { return emit_rm(UniOpRM::kLoadU16, dst, src); }
  ASMJIT_INLINE void load_i32(const Gp& dst, const Mem& src) { return emit_rm(UniOpRM::kLoadI32, dst, src); }
  ASMJIT_INLINE void load_u32(const Gp& dst, const Mem& src) { return emit_rm(UniOpRM::kLoadU32, dst, src); }
  ASMJIT_INLINE void load_i64(const Gp& dst, const Mem& src) { return emit_rm(UniOpRM::kLoadI64, dst, src); }
  ASMJIT_INLINE void load_u64(const Gp& dst, const Mem& src) { return emit_rm(UniOpRM::kLoadU64, dst, src); }

  ASMJIT_INLINE void load_merge_u8(const Gp& dst, const Mem& src) { return emit_rm(UniOpRM::kLoadMergeU8, dst, src); }
  ASMJIT_INLINE void load_shift_u8(const Gp& dst, const Mem& src) { return emit_rm(UniOpRM::kLoadShiftU8, dst, src); }
  ASMJIT_INLINE void load_merge_u16(const Gp& dst, const Mem& src) { return emit_rm(UniOpRM::kLoadMergeU16, dst, src); }
  ASMJIT_INLINE void load_shift_u16(const Gp& dst, const Mem& src) { return emit_rm(UniOpRM::kLoadShiftU16, dst, src); }

  ASMJIT_INLINE void store(const Mem& dst, const Gp& src) { return emit_mr(UniOpMR::kStoreReg, dst, src); }
  ASMJIT_INLINE void store_u8(const Mem& dst, const Gp& src) { return emit_mr(UniOpMR::kStoreU8, dst, src); }
  ASMJIT_INLINE void store_u16(const Mem& dst, const Gp& src) { return emit_mr(UniOpMR::kStoreU16, dst, src); }
  ASMJIT_INLINE void store_u32(const Mem& dst, const Gp& src) { return emit_mr(UniOpMR::kStoreU32, dst, src); }
  ASMJIT_INLINE void store_u64(const Mem& dst, const Gp& src) { return emit_mr(UniOpMR::kStoreU64, dst, src); }

  ASMJIT_INLINE void prefetch(const Mem& mem) { return emit_m(UniOpM::kPrefetch, mem); }
  ASMJIT_INLINE void store_zero_reg(const Mem& dst) { return emit_m(UniOpM::kStoreZeroReg, dst); }
  ASMJIT_INLINE void store_zero_u8(const Mem& dst) { return emit_m(UniOpM::kStoreZeroU8, dst); }
  ASMJIT_INLINE void store_zero_u16(const Mem& dst) { return emit_m(UniOpM::kStoreZeroU16, dst); }
  ASMJIT_INLINE void store_zero_u32(const Mem& dst) { return emit_m(UniOpM::kStoreZeroU32, dst); }
  ASMJIT_INLINE void store_zero_u64(const Mem& dst) { return emit_m(UniOpM::kStoreZeroU64, dst); }

  ASMJIT_INLINE void mem_add(const Mem& dst, const Gp& src) { return emit_mr(UniOpMR::kAddReg, dst, src); }
  ASMJIT_INLINE void mem_add_u8(const Mem& dst, const Gp& src) { return emit_mr(UniOpMR::kAddU8, dst, src); }
  ASMJIT_INLINE void mem_add_u16(const Mem& dst, const Gp& src) { return emit_mr(UniOpMR::kAddU16, dst, src); }
  ASMJIT_INLINE void mem_add_u32(const Mem& dst, const Gp& src) { return emit_mr(UniOpMR::kAddU32, dst, src); }
  ASMJIT_INLINE void mem_add_u64(const Mem& dst, const Gp& src) { return emit_mr(UniOpMR::kAddU64, dst, src); }

  ASMJIT_INLINE void cmov(const Gp& dst, const Gp& sel, const UniCondition& condition) { emit_cmov(dst, sel, condition); }
  ASMJIT_INLINE void cmov(const Gp& dst, const Mem& sel, const UniCondition& condition) { emit_cmov(dst, sel, condition); }

  template<typename Sel1, typename Sel2>
  ASMJIT_INLINE void select(const Gp& dst, const Sel1& sel1, const Sel2& sel2, const UniCondition& condition) { emit_select(dst, sel1, sel2, condition); }

  ASMJIT_INLINE void abs(const Gp& dst, const Gp& src) { emit_2i(UniOpRR::kAbs, dst, src); }
  ASMJIT_INLINE void abs(const Gp& dst, const Mem& src) { emit_2i(UniOpRR::kAbs, dst, src); }

  ASMJIT_INLINE void neg(const Gp& dst, const Gp& src) { emit_2i(UniOpRR::kNeg, dst, src); }
  ASMJIT_INLINE void neg(const Gp& dst, const Mem& src) { emit_2i(UniOpRR::kNeg, dst, src); }

  ASMJIT_INLINE void not_(const Gp& dst, const Gp& src) { emit_2i(UniOpRR::kNot, dst, src); }
  ASMJIT_INLINE void not_(const Gp& dst, const Mem& src) { emit_2i(UniOpRR::kNot, dst, src); }

  ASMJIT_INLINE void bswap(const Gp& dst, const Gp& src) { emit_2i(UniOpRR::kBSwap, dst, src); }
  ASMJIT_INLINE void bswap(const Gp& dst, const Mem& src) { emit_2i(UniOpRR::kBSwap, dst, src); }

  ASMJIT_INLINE void clz(const Gp& dst, const Gp& src) { emit_2i(UniOpRR::kCLZ, dst, src); }
  ASMJIT_INLINE void clz(const Gp& dst, const Mem& src) { emit_2i(UniOpRR::kCLZ, dst, src); }

  ASMJIT_INLINE void ctz(const Gp& dst, const Gp& src) { emit_2i(UniOpRR::kCTZ, dst, src); }
  ASMJIT_INLINE void ctz(const Gp& dst, const Mem& src) { emit_2i(UniOpRR::kCTZ, dst, src); }

  ASMJIT_INLINE void reflect(const Gp& dst, const Gp& src) { emit_2i(UniOpRR::kReflect, dst, src); }
  ASMJIT_INLINE void reflect(const Gp& dst, const Mem& src) { emit_2i(UniOpRR::kReflect, dst, src); }

  ASMJIT_INLINE void inc(const Gp& dst) { emit_3i(UniOpRRR::kAdd, dst, dst, Imm(1)); }
  ASMJIT_INLINE void dec(const Gp& dst) { emit_3i(UniOpRRR::kSub, dst, dst, Imm(1)); }

  ASMJIT_INLINE void and_(const Gp& dst, const Gp& src1, const Gp& src2) { emit_3i(UniOpRRR::kAnd, dst, src1, src2); }
  ASMJIT_INLINE void and_(const Gp& dst, const Gp& src1, const Mem& src2) { emit_3i(UniOpRRR::kAnd, dst, src1, src2); }
  ASMJIT_INLINE void and_(const Gp& dst, const Gp& src1, const Imm& src2) { emit_3i(UniOpRRR::kAnd, dst, src1, src2); }
  ASMJIT_INLINE void and_(const Gp& dst, const Mem& src1, const Gp& src2) { emit_3i(UniOpRRR::kAnd, dst, src1, src2); }
  ASMJIT_INLINE void and_(const Gp& dst, const Mem& src1, const Imm& src2) { emit_3i(UniOpRRR::kAnd, dst, src1, src2); }

  ASMJIT_INLINE void or_(const Gp& dst, const Gp& src1, const Gp& src2) { emit_3i(UniOpRRR::kOr, dst, src1, src2); }
  ASMJIT_INLINE void or_(const Gp& dst, const Gp& src1, const Mem& src2) { emit_3i(UniOpRRR::kOr, dst, src1, src2); }
  ASMJIT_INLINE void or_(const Gp& dst, const Gp& src1, const Imm& src2) { emit_3i(UniOpRRR::kOr, dst, src1, src2); }
  ASMJIT_INLINE void or_(const Gp& dst, const Mem& src1, const Gp& src2) { emit_3i(UniOpRRR::kOr, dst, src1, src2); }
  ASMJIT_INLINE void or_(const Gp& dst, const Mem& src1, const Imm& src2) { emit_3i(UniOpRRR::kOr, dst, src1, src2); }

  ASMJIT_INLINE void xor_(const Gp& dst, const Gp& src1, const Gp& src2) { emit_3i(UniOpRRR::kXor, dst, src1, src2); }
  ASMJIT_INLINE void xor_(const Gp& dst, const Gp& src1, const Mem& src2) { emit_3i(UniOpRRR::kXor, dst, src1, src2); }
  ASMJIT_INLINE void xor_(const Gp& dst, const Gp& src1, const Imm& src2) { emit_3i(UniOpRRR::kXor, dst, src1, src2); }
  ASMJIT_INLINE void xor_(const Gp& dst, const Mem& src1, const Gp& src2) { emit_3i(UniOpRRR::kXor, dst, src1, src2); }
  ASMJIT_INLINE void xor_(const Gp& dst, const Mem& src1, const Imm& src2) { emit_3i(UniOpRRR::kXor, dst, src1, src2); }

  ASMJIT_INLINE void bic(const Gp& dst, const Gp& src1, const Gp& src2) { emit_3i(UniOpRRR::kBic, dst, src1, src2); }
  ASMJIT_INLINE void bic(const Gp& dst, const Gp& src1, const Mem& src2) { emit_3i(UniOpRRR::kBic, dst, src1, src2); }
  ASMJIT_INLINE void bic(const Gp& dst, const Gp& src1, const Imm& src2) { emit_3i(UniOpRRR::kBic, dst, src1, src2); }
  ASMJIT_INLINE void bic(const Gp& dst, const Mem& src1, const Gp& src2) { emit_3i(UniOpRRR::kBic, dst, src1, src2); }
  ASMJIT_INLINE void bic(const Gp& dst, const Mem& src1, const Imm& src2) { emit_3i(UniOpRRR::kBic, dst, src1, src2); }

  ASMJIT_INLINE void add(const Gp& dst, const Gp& src1, const Gp& src2) { emit_3i(UniOpRRR::kAdd, dst, src1, src2); }
  ASMJIT_INLINE void add(const Gp& dst, const Gp& src1, const Mem& src2) { emit_3i(UniOpRRR::kAdd, dst, src1, src2); }
  ASMJIT_INLINE void add(const Gp& dst, const Gp& src1, const Imm& src2) { emit_3i(UniOpRRR::kAdd, dst, src1, src2); }
  ASMJIT_INLINE void add(const Gp& dst, const Mem& src1, const Gp& src2) { emit_3i(UniOpRRR::kAdd, dst, src1, src2); }
  ASMJIT_INLINE void add(const Gp& dst, const Mem& src1, const Imm& src2) { emit_3i(UniOpRRR::kAdd, dst, src1, src2); }

  ASMJIT_INLINE void sub(const Gp& dst, const Gp& src1, const Gp& src2) { emit_3i(UniOpRRR::kSub, dst, src1, src2); }
  ASMJIT_INLINE void sub(const Gp& dst, const Gp& src1, const Mem& src2) { emit_3i(UniOpRRR::kSub, dst, src1, src2); }
  ASMJIT_INLINE void sub(const Gp& dst, const Gp& src1, const Imm& src2) { emit_3i(UniOpRRR::kSub, dst, src1, src2); }
  ASMJIT_INLINE void sub(const Gp& dst, const Mem& src1, const Gp& src2) { emit_3i(UniOpRRR::kSub, dst, src1, src2); }
  ASMJIT_INLINE void sub(const Gp& dst, const Mem& src1, const Imm& src2) { emit_3i(UniOpRRR::kSub, dst, src1, src2); }

  ASMJIT_INLINE void mul(const Gp& dst, const Gp& src1, const Gp& src2) { emit_3i(UniOpRRR::kMul, dst, src1, src2); }
  ASMJIT_INLINE void mul(const Gp& dst, const Gp& src1, const Mem& src2) { emit_3i(UniOpRRR::kMul, dst, src1, src2); }
  ASMJIT_INLINE void mul(const Gp& dst, const Gp& src1, const Imm& src2) { emit_3i(UniOpRRR::kMul, dst, src1, src2); }
  ASMJIT_INLINE void mul(const Gp& dst, const Mem& src1, const Gp& src2) { emit_3i(UniOpRRR::kMul, dst, src1, src2); }
  ASMJIT_INLINE void mul(const Gp& dst, const Mem& src1, const Imm& src2) { emit_3i(UniOpRRR::kMul, dst, src1, src2); }

  ASMJIT_INLINE void udiv(const Gp& dst, const Gp& src1, const Gp& src2) { emit_3i(UniOpRRR::kUDiv, dst, src1, src2); }
  ASMJIT_INLINE void udiv(const Gp& dst, const Gp& src1, const Mem& src2) { emit_3i(UniOpRRR::kUDiv, dst, src1, src2); }
  ASMJIT_INLINE void udiv(const Gp& dst, const Gp& src1, const Imm& src2) { emit_3i(UniOpRRR::kUDiv, dst, src1, src2); }
  ASMJIT_INLINE void udiv(const Gp& dst, const Mem& src1, const Gp& src2) { emit_3i(UniOpRRR::kUDiv, dst, src1, src2); }
  ASMJIT_INLINE void udiv(const Gp& dst, const Mem& src1, const Imm& src2) { emit_3i(UniOpRRR::kUDiv, dst, src1, src2); }

  ASMJIT_INLINE void umod(const Gp& dst, const Gp& src1, const Gp& src2) { emit_3i(UniOpRRR::kUMod, dst, src1, src2); }
  ASMJIT_INLINE void umod(const Gp& dst, const Gp& src1, const Mem& src2) { emit_3i(UniOpRRR::kUMod, dst, src1, src2); }
  ASMJIT_INLINE void umod(const Gp& dst, const Gp& src1, const Imm& src2) { emit_3i(UniOpRRR::kUMod, dst, src1, src2); }
  ASMJIT_INLINE void umod(const Gp& dst, const Mem& src1, const Gp& src2) { emit_3i(UniOpRRR::kUMod, dst, src1, src2); }
  ASMJIT_INLINE void umod(const Gp& dst, const Mem& src1, const Imm& src2) { emit_3i(UniOpRRR::kUMod, dst, src1, src2); }

  ASMJIT_INLINE void smin(const Gp& dst, const Gp& src1, const Gp& src2) { emit_3i(UniOpRRR::kSMin, dst, src1, src2); }
  ASMJIT_INLINE void smin(const Gp& dst, const Gp& src1, const Mem& src2) { emit_3i(UniOpRRR::kSMin, dst, src1, src2); }
  ASMJIT_INLINE void smin(const Gp& dst, const Gp& src1, const Imm& src2) { emit_3i(UniOpRRR::kSMin, dst, src1, src2); }
  ASMJIT_INLINE void smin(const Gp& dst, const Mem& src1, const Gp& src2) { emit_3i(UniOpRRR::kSMin, dst, src1, src2); }
  ASMJIT_INLINE void smin(const Gp& dst, const Mem& src1, const Imm& src2) { emit_3i(UniOpRRR::kSMin, dst, src1, src2); }

  ASMJIT_INLINE void smax(const Gp& dst, const Gp& src1, const Gp& src2) { emit_3i(UniOpRRR::kSMax, dst, src1, src2); }
  ASMJIT_INLINE void smax(const Gp& dst, const Gp& src1, const Mem& src2) { emit_3i(UniOpRRR::kSMax, dst, src1, src2); }
  ASMJIT_INLINE void smax(const Gp& dst, const Gp& src1, const Imm& src2) { emit_3i(UniOpRRR::kSMax, dst, src1, src2); }
  ASMJIT_INLINE void smax(const Gp& dst, const Mem& src1, const Gp& src2) { emit_3i(UniOpRRR::kSMax, dst, src1, src2); }
  ASMJIT_INLINE void smax(const Gp& dst, const Mem& src1, const Imm& src2) { emit_3i(UniOpRRR::kSMax, dst, src1, src2); }

  ASMJIT_INLINE void umin(const Gp& dst, const Gp& src1, const Gp& src2) { emit_3i(UniOpRRR::kUMin, dst, src1, src2); }
  ASMJIT_INLINE void umin(const Gp& dst, const Gp& src1, const Mem& src2) { emit_3i(UniOpRRR::kUMin, dst, src1, src2); }
  ASMJIT_INLINE void umin(const Gp& dst, const Gp& src1, const Imm& src2) { emit_3i(UniOpRRR::kUMin, dst, src1, src2); }
  ASMJIT_INLINE void umin(const Gp& dst, const Mem& src1, const Gp& src2) { emit_3i(UniOpRRR::kUMin, dst, src1, src2); }
  ASMJIT_INLINE void umin(const Gp& dst, const Mem& src1, const Imm& src2) { emit_3i(UniOpRRR::kUMin, dst, src1, src2); }

  ASMJIT_INLINE void umax(const Gp& dst, const Gp& src1, const Gp& src2) { emit_3i(UniOpRRR::kUMax, dst, src1, src2); }
  ASMJIT_INLINE void umax(const Gp& dst, const Gp& src1, const Mem& src2) { emit_3i(UniOpRRR::kUMax, dst, src1, src2); }
  ASMJIT_INLINE void umax(const Gp& dst, const Gp& src1, const Imm& src2) { emit_3i(UniOpRRR::kUMax, dst, src1, src2); }
  ASMJIT_INLINE void umax(const Gp& dst, const Mem& src1, const Gp& src2) { emit_3i(UniOpRRR::kUMax, dst, src1, src2); }
  ASMJIT_INLINE void umax(const Gp& dst, const Mem& src1, const Imm& src2) { emit_3i(UniOpRRR::kUMax, dst, src1, src2); }

  ASMJIT_INLINE void shl(const Gp& dst, const Gp& src1, const Gp& src2) { emit_3i(UniOpRRR::kSll, dst, src1, src2); }
  ASMJIT_INLINE void shl(const Gp& dst, const Gp& src1, const Imm& src2) { emit_3i(UniOpRRR::kSll, dst, src1, src2); }
  ASMJIT_INLINE void shl(const Gp& dst, const Mem& src1, const Gp& src2) { emit_3i(UniOpRRR::kSll, dst, src1, src2); }
  ASMJIT_INLINE void shl(const Gp& dst, const Mem& src1, const Imm& src2) { emit_3i(UniOpRRR::kSll, dst, src1, src2); }

  ASMJIT_INLINE void shr(const Gp& dst, const Gp& src1, const Gp& src2) { emit_3i(UniOpRRR::kSrl, dst, src1, src2); }
  ASMJIT_INLINE void shr(const Gp& dst, const Gp& src1, const Imm& src2) { emit_3i(UniOpRRR::kSrl, dst, src1, src2); }
  ASMJIT_INLINE void shr(const Gp& dst, const Mem& src1, const Gp& src2) { emit_3i(UniOpRRR::kSrl, dst, src1, src2); }
  ASMJIT_INLINE void shr(const Gp& dst, const Mem& src1, const Imm& src2) { emit_3i(UniOpRRR::kSrl, dst, src1, src2); }

  ASMJIT_INLINE void sar(const Gp& dst, const Gp& src1, const Gp& src2) { emit_3i(UniOpRRR::kSra, dst, src1, src2); }
  ASMJIT_INLINE void sar(const Gp& dst, const Gp& src1, const Imm& src2) { emit_3i(UniOpRRR::kSra, dst, src1, src2); }
  ASMJIT_INLINE void sar(const Gp& dst, const Mem& src1, const Gp& src2) { emit_3i(UniOpRRR::kSra, dst, src1, src2); }
  ASMJIT_INLINE void sar(const Gp& dst, const Mem& src1, const Imm& src2) { emit_3i(UniOpRRR::kSra, dst, src1, src2); }

  ASMJIT_INLINE void rol(const Gp& dst, const Gp& src1, const Gp& src2) { emit_3i(UniOpRRR::kRol, dst, src1, src2); }
  ASMJIT_INLINE void rol(const Gp& dst, const Gp& src1, const Imm& src2) { emit_3i(UniOpRRR::kRol, dst, src1, src2); }
  ASMJIT_INLINE void rol(const Gp& dst, const Mem& src1, const Gp& src2) { emit_3i(UniOpRRR::kRol, dst, src1, src2); }
  ASMJIT_INLINE void rol(const Gp& dst, const Mem& src1, const Imm& src2) { emit_3i(UniOpRRR::kRol, dst, src1, src2); }

  ASMJIT_INLINE void ror(const Gp& dst, const Gp& src1, const Gp& src2) { emit_3i(UniOpRRR::kRor, dst, src1, src2); }
  ASMJIT_INLINE void ror(const Gp& dst, const Gp& src1, const Imm& src2) { emit_3i(UniOpRRR::kRor, dst, src1, src2); }
  ASMJIT_INLINE void ror(const Gp& dst, const Mem& src1, const Gp& src2) { emit_3i(UniOpRRR::kRor, dst, src1, src2); }
  ASMJIT_INLINE void ror(const Gp& dst, const Mem& src1, const Imm& src2) { emit_3i(UniOpRRR::kRor, dst, src1, src2); }

  ASMJIT_INLINE void sbound(const Gp& dst, const Gp& src1, const Gp& src2) { emit_3i(UniOpRRR::kSBound, dst, src1, src2); }
  ASMJIT_INLINE void sbound(const Gp& dst, const Gp& src1, const Mem& src2) { emit_3i(UniOpRRR::kSBound, dst, src1, src2); }

  ASMJIT_INLINE void j(const Gp& target) { emit_j(target); }
  ASMJIT_INLINE void j(const Label& target) { emit_j(target); }
  ASMJIT_INLINE void j(const Label& target, const UniCondition& condition) { emit_j_if(target, condition); }

  ASMJIT_API void adds_u8(const Gp& dst, const Gp& src1, const Gp& src2);

  ASMJIT_API void inv_u8(const Gp& dst, const Gp& src);
  ASMJIT_API void div_255_u32(const Gp& dst, const Gp& src);
  ASMJIT_API void mul_257_hu16(const Gp& dst, const Gp& src);

  ASMJIT_API void add_scaled(const Gp& dst, const Gp& a, int b);
  ASMJIT_API void add_ext(const Gp& dst, const Gp& src_, const Gp& idx_, uint32_t scale, int32_t disp = 0);

  ASMJIT_API void lea(const Gp& dst, const Mem& src);

  //! \}

  //! \name Emit - Vector Instructions
  //! \{

  ASMJIT_API void emit_2v(UniOpVV op, const Operand_& dst_, const Operand_& src_);
  ASMJIT_API void emit_2v(UniOpVV op, const OpArray& dst_, const Operand_& src_);
  ASMJIT_API void emit_2v(UniOpVV op, const OpArray& dst_, const OpArray& src_);

  ASMJIT_API void emit_2vi(UniOpVVI op, const Operand_& dst_, const Operand_& src_, uint32_t imm);
  ASMJIT_API void emit_2vi(UniOpVVI op, const OpArray& dst_, const Operand_& src_, uint32_t imm);
  ASMJIT_API void emit_2vi(UniOpVVI op, const OpArray& dst_, const OpArray& src_, uint32_t imm);

  ASMJIT_API void emit_2vs(UniOpVR op, const Operand_& dst_, const Operand_& src_, uint32_t idx = 0);

  ASMJIT_API void emit_vm(UniOpVM op, const Vec& dst_, const Mem& src_, Alignment alignment, uint32_t idx = 0);
  ASMJIT_API void emit_vm(UniOpVM op, const OpArray& dst_, const Mem& src_, Alignment alignment, uint32_t idx = 0);

  ASMJIT_API void emit_mv(UniOpMV op, const Mem& dst_, const Vec& src_, Alignment alignment, uint32_t idx = 0);
  ASMJIT_API void emit_mv(UniOpMV op, const Mem& dst_, const OpArray& src_, Alignment alignment, uint32_t idx = 0);

  ASMJIT_API void emit_3v(UniOpVVV op, const Operand_& dst_, const Operand_& src1_, const Operand_& src2_);
  ASMJIT_API void emit_3v(UniOpVVV op, const OpArray& dst_, const Operand_& src1_, const OpArray& src2_);
  ASMJIT_API void emit_3v(UniOpVVV op, const OpArray& dst_, const OpArray& src1_, const Operand_& src2_);
  ASMJIT_API void emit_3v(UniOpVVV op, const OpArray& dst_, const OpArray& src1_, const OpArray& src2_);

  ASMJIT_API void emit_3vi(UniOpVVVI op, const Operand_& dst_, const Operand_& src1_, const Operand_& src2_, uint32_t imm);
  ASMJIT_API void emit_3vi(UniOpVVVI op, const OpArray& dst_, const Operand_& src1_, const OpArray& src2_, uint32_t imm);
  ASMJIT_API void emit_3vi(UniOpVVVI op, const OpArray& dst_, const OpArray& src1_, const Operand_& src2_, uint32_t imm);
  ASMJIT_API void emit_3vi(UniOpVVVI op, const OpArray& dst_, const OpArray& src1_, const OpArray& src2_, uint32_t imm);

  ASMJIT_API void emit_4v(UniOpVVVV op, const Operand_& dst_, const Operand_& src1_, const Operand_& src2_, const Operand_& src3_);
  ASMJIT_API void emit_4v(UniOpVVVV op, const OpArray& dst_, const Operand_& src1_, const Operand_& src2_, const OpArray& src3_);
  ASMJIT_API void emit_4v(UniOpVVVV op, const OpArray& dst_, const Operand_& src1_, const OpArray& src2_, const Operand& src3_);
  ASMJIT_API void emit_4v(UniOpVVVV op, const OpArray& dst_, const Operand_& src1_, const OpArray& src2_, const OpArray& src3_);
  ASMJIT_API void emit_4v(UniOpVVVV op, const OpArray& dst_, const OpArray& src1_, const Operand_& src2_, const Operand& src3_);
  ASMJIT_API void emit_4v(UniOpVVVV op, const OpArray& dst_, const OpArray& src1_, const Operand_& src2_, const OpArray& src3_);
  ASMJIT_API void emit_4v(UniOpVVVV op, const OpArray& dst_, const OpArray& src1_, const OpArray& src2_, const Operand& src3_);
  ASMJIT_API void emit_4v(UniOpVVVV op, const OpArray& dst_, const OpArray& src1_, const OpArray& src2_, const OpArray& src3_);

  #define DEFINE_OP_2V(name, op) \
    template<typename Dst, typename Src> \
    ASMJIT_INLINE void name(const Dst& dst, const Src& src) { emit_2v(op, dst, src); }

  #define DEFINE_OP_2VI(name, op) \
    template<typename Dst, typename Src> \
    ASMJIT_INLINE void name(const Dst& dst, const Src& src, uint32_t imm) { emit_2vi(op, dst, src, imm); }

  #define DEFINE_OP_2VI_WRAP(name, imm_wrapper, op) \
    template<typename Dst, typename Src> \
    ASMJIT_INLINE void name(const Dst& dst, const Src& src, const imm_wrapper& imm) { emit_2vi(op, dst, src, imm.value); }

  #define DEFINE_OP_VM_U(name, op, alignment) \
    ASMJIT_INLINE void name(const Vec& dst, const Mem& src) { emit_vm(op, dst, src, Alignment(alignment)); } \
    ASMJIT_INLINE void name(const VecArray& dst, const Mem& src) { emit_vm(op, dst, src, Alignment(alignment)); }

  #define DEFINE_OP_VM_A(name, op, default_alignment) \
    ASMJIT_INLINE void name(const Vec& dst, const Mem& src, Alignment alignment = Alignment{default_alignment}) { emit_vm(op, dst, src, alignment); } \
    ASMJIT_INLINE void name(const VecArray& dst, const Mem& src, Alignment alignment = Alignment{default_alignment}) { emit_vm(op, dst, src, alignment); }

  #define DEFINE_OP_VM_I(name, op, default_alignment) \
    ASMJIT_INLINE void name(const Vec& dst, const Mem& src, uint32_t idx) { emit_vm(op, dst, src, Alignment(default_alignment), idx); } \
    ASMJIT_INLINE void name(const VecArray& dst, const Mem& src, uint32_t idx) { emit_vm(op, dst, src, Alignment(default_alignment), idx); }

  #define DEFINE_OP_MV_U(name, op, alignment) \
    ASMJIT_INLINE void name(const Mem& dst, const Vec& src) { emit_mv(op, dst, src, Alignment(alignment)); } \
    ASMJIT_INLINE void name(const Mem& dst, const VecArray& src) { emit_mv(op, dst, src, Alignment(alignment)); }

  #define DEFINE_OP_MV_A(name, op, default_alignment) \
    ASMJIT_INLINE void name(const Mem& dst, const Vec& src, Alignment alignment = Alignment(default_alignment)) { emit_mv(op, dst, src, alignment); } \
    ASMJIT_INLINE void name(const Mem& dst, const VecArray& src, Alignment alignment = Alignment(default_alignment)) { emit_mv(op, dst, src, alignment); }

  #define DEFINE_OP_MV_I(name, op, default_alignment) \
    ASMJIT_INLINE void name(const Mem& dst, const Vec& src, uint32_t idx) { emit_mv(op, dst, src, Alignment(default_alignment), idx); } \
    ASMJIT_INLINE void name(const Mem& dst, const VecArray& src, uint32_t idx) { emit_mv(op, dst, src, Alignment(default_alignment), idx); }

  #define DEFINE_OP_3V(name, op) \
    template<typename Dst, typename Src1, typename Src2> \
    ASMJIT_INLINE void name(const Dst& dst, const Src1& src1, const Src2& src2) { emit_3v(op, dst, src1, src2); }

  #define DEFINE_OP_3VI(name, op) \
    template<typename Dst, typename Src1, typename Src2> \
    ASMJIT_INLINE void name(const Dst& dst, const Src1& src1, const Src2& src2, uint32_t imm) { emit_3vi(op, dst, src1, src2, imm); }

  #define DEFINE_OP_3VI_WRAP(name, imm_wrapper, op) \
    template<typename Dst, typename Src1, typename Src2> \
    ASMJIT_INLINE void name(const Dst& dst, const Src1& src1, const Src2& src2, const imm_wrapper& imm) { emit_3vi(op, dst, src1, src2, imm.value); }

  #define DEFINE_OP_4V(name, op) \
    template<typename Dst, typename Src1, typename Src2, typename Src3> \
    ASMJIT_INLINE void name(const Dst& dst, const Src1& src1, const Src2& src2, const Src3& src3) { emit_4v(op, dst, src1, src2, src3); }

  ASMJIT_INLINE void s_mov(const Gp& dst, const Vec& src) { emit_2vs(UniOpVR::kMov, dst, src); }
  ASMJIT_INLINE void s_mov(const Vec& dst, const Gp& src) { emit_2vs(UniOpVR::kMov, dst, src); }

  ASMJIT_INLINE void s_mov_u32(const Gp& dst, const Vec& src) { emit_2vs(UniOpVR::kMovU32, dst, src); }
  ASMJIT_INLINE void s_mov_u32(const Vec& dst, const Gp& src) { emit_2vs(UniOpVR::kMovU32, dst, src); }

  ASMJIT_INLINE void s_mov_u64(const Gp& dst, const Vec& src) { emit_2vs(UniOpVR::kMovU64, dst, src); }
  ASMJIT_INLINE void s_mov_u64(const Vec& dst, const Gp& src) { emit_2vs(UniOpVR::kMovU64, dst, src); }

  ASMJIT_INLINE void s_insert_u8(const Vec& dst, const Gp& src, uint32_t idx) { emit_2vs(UniOpVR::kInsertU8, dst, src, idx); }
  ASMJIT_INLINE void s_insert_u16(const Vec& dst, const Gp& src, uint32_t idx) { emit_2vs(UniOpVR::kInsertU16, dst, src, idx); }
  ASMJIT_INLINE void s_insert_u32(const Vec& dst, const Gp& src, uint32_t idx) { emit_2vs(UniOpVR::kInsertU32, dst, src, idx); }
  ASMJIT_INLINE void s_insert_u64(const Vec& dst, const Gp& src, uint32_t idx) { emit_2vs(UniOpVR::kInsertU64, dst, src, idx); }

  ASMJIT_INLINE void s_extract_u8(const Gp& dst, const Vec& src, uint32_t idx) { emit_2vs(UniOpVR::kExtractU8, dst, src, idx); }
  ASMJIT_INLINE void s_extract_u16(const Gp& dst, const Vec& src, uint32_t idx) { emit_2vs(UniOpVR::kExtractU16, dst, src, idx); }
  ASMJIT_INLINE void s_extract_u32(const Gp& dst, const Vec& src, uint32_t idx) { emit_2vs(UniOpVR::kExtractU32, dst, src, idx); }
  ASMJIT_INLINE void s_extract_u64(const Gp& dst, const Vec& src, uint32_t idx) { emit_2vs(UniOpVR::kExtractU64, dst, src, idx); }

  ASMJIT_INLINE void s_cvt_int_to_f32(const Vec& dst, const Gp& src) { emit_2vs(UniOpVR::kCvtIntToF32, dst, src); }
  ASMJIT_INLINE void s_cvt_int_to_f32(const Vec& dst, const Mem& src) { emit_2vs(UniOpVR::kCvtIntToF32, dst, src); }
  ASMJIT_INLINE void s_cvt_int_to_f64(const Vec& dst, const Gp& src) { emit_2vs(UniOpVR::kCvtIntToF64, dst, src); }
  ASMJIT_INLINE void s_cvt_int_to_f64(const Vec& dst, const Mem& src) { emit_2vs(UniOpVR::kCvtIntToF64, dst, src); }

  ASMJIT_INLINE void s_cvt_trunc_f32_to_int(const Gp& dst, const Vec& src) { emit_2vs(UniOpVR::kCvtTruncF32ToInt, dst, src); }
  ASMJIT_INLINE void s_cvt_trunc_f32_to_int(const Gp& dst, const Mem& src) { emit_2vs(UniOpVR::kCvtTruncF32ToInt, dst, src); }
  ASMJIT_INLINE void s_cvt_round_f32_to_int(const Gp& dst, const Vec& src) { emit_2vs(UniOpVR::kCvtRoundF32ToInt, dst, src); }
  ASMJIT_INLINE void s_cvt_round_f32_to_int(const Gp& dst, const Mem& src) { emit_2vs(UniOpVR::kCvtRoundF32ToInt, dst, src); }
  ASMJIT_INLINE void s_cvt_trunc_f64_to_int(const Gp& dst, const Vec& src) { emit_2vs(UniOpVR::kCvtTruncF64ToInt, dst, src); }
  ASMJIT_INLINE void s_cvt_trunc_f64_to_int(const Gp& dst, const Mem& src) { emit_2vs(UniOpVR::kCvtTruncF64ToInt, dst, src); }
  ASMJIT_INLINE void s_cvt_round_f64_to_int(const Gp& dst, const Vec& src) { emit_2vs(UniOpVR::kCvtRoundF64ToInt, dst, src); }
  ASMJIT_INLINE void s_cvt_round_f64_to_int(const Gp& dst, const Mem& src) { emit_2vs(UniOpVR::kCvtRoundF64ToInt, dst, src); }

  DEFINE_OP_2V(v_mov, UniOpVV::kMov)
  DEFINE_OP_2V(v_mov_u64, UniOpVV::kMovU64)
  DEFINE_OP_2V(v_broadcast_u8z, UniOpVV::kBroadcastU8Z)
  DEFINE_OP_2V(v_broadcast_u16z, UniOpVV::kBroadcastU16Z)
  DEFINE_OP_2V(v_broadcast_u8, UniOpVV::kBroadcastU8)
  DEFINE_OP_2V(v_broadcast_u16, UniOpVV::kBroadcastU16)
  DEFINE_OP_2V(v_broadcast_u32, UniOpVV::kBroadcastU32)
  DEFINE_OP_2V(v_broadcast_u64, UniOpVV::kBroadcastU64)
  DEFINE_OP_2V(v_broadcast_f32, UniOpVV::kBroadcastF32)
  DEFINE_OP_2V(v_broadcast_f64, UniOpVV::kBroadcastF64)
  DEFINE_OP_2V(v_broadcast_v128_u32, UniOpVV::kBroadcastV128_U32)
  DEFINE_OP_2V(v_broadcast_v128_u64, UniOpVV::kBroadcastV128_U64)
  DEFINE_OP_2V(v_broadcast_v128_f32, UniOpVV::kBroadcastV128_F32)
  DEFINE_OP_2V(v_broadcast_v128_f64, UniOpVV::kBroadcastV128_F64)
  DEFINE_OP_2V(v_broadcast_v256_u32, UniOpVV::kBroadcastV256_U32)
  DEFINE_OP_2V(v_broadcast_v256_u64, UniOpVV::kBroadcastV256_U64)
  DEFINE_OP_2V(v_broadcast_v256_f32, UniOpVV::kBroadcastV256_F32)
  DEFINE_OP_2V(v_broadcast_v256_f64, UniOpVV::kBroadcastV256_F64)
  DEFINE_OP_2V(v_abs_i8, UniOpVV::kAbsI8)
  DEFINE_OP_2V(v_abs_i16, UniOpVV::kAbsI16)
  DEFINE_OP_2V(v_abs_i32, UniOpVV::kAbsI32)
  DEFINE_OP_2V(v_abs_i64, UniOpVV::kAbsI64)
  DEFINE_OP_2V(v_not_u32, UniOpVV::kNotU32)
  DEFINE_OP_2V(v_not_u64, UniOpVV::kNotU64)
  DEFINE_OP_2V(v_cvt_i8_lo_to_i16, UniOpVV::kCvtI8LoToI16)
  DEFINE_OP_2V(v_cvt_i8_hi_to_i16, UniOpVV::kCvtI8HiToI16)
  DEFINE_OP_2V(v_cvt_u8_lo_to_u16, UniOpVV::kCvtU8LoToU16)
  DEFINE_OP_2V(v_cvt_u8_hi_to_u16, UniOpVV::kCvtU8HiToU16)
  DEFINE_OP_2V(v_cvt_i8_to_i32, UniOpVV::kCvtI8ToI32)
  DEFINE_OP_2V(v_cvt_u8_to_u32, UniOpVV::kCvtU8ToU32)
  DEFINE_OP_2V(v_cvt_i16_lo_to_i32, UniOpVV::kCvtI16LoToI32)
  DEFINE_OP_2V(v_cvt_i16_hi_to_i32, UniOpVV::kCvtI16HiToI32)
  DEFINE_OP_2V(v_cvt_u16_lo_to_u32, UniOpVV::kCvtU16LoToU32)
  DEFINE_OP_2V(v_cvt_u16_hi_to_u32, UniOpVV::kCvtU16HiToU32)
  DEFINE_OP_2V(v_cvt_i32_lo_to_i64, UniOpVV::kCvtI32LoToI64)
  DEFINE_OP_2V(v_cvt_i32_hi_to_i64, UniOpVV::kCvtI32HiToI64)
  DEFINE_OP_2V(v_cvt_u32_lo_to_u64, UniOpVV::kCvtU32LoToU64)
  DEFINE_OP_2V(v_cvt_u32_hi_to_u64, UniOpVV::kCvtU32HiToU64)
  DEFINE_OP_2V(s_abs_f32, UniOpVV::kAbsF32S)
  DEFINE_OP_2V(s_abs_f64, UniOpVV::kAbsF64S)
  DEFINE_OP_2V(v_abs_f32, UniOpVV::kAbsF32)
  DEFINE_OP_2V(v_abs_f64, UniOpVV::kAbsF64)
  DEFINE_OP_2V(s_neg_f32, UniOpVV::kNegF32S)
  DEFINE_OP_2V(s_neg_f64, UniOpVV::kNegF64S)
  DEFINE_OP_2V(v_neg_f32, UniOpVV::kNegF32)
  DEFINE_OP_2V(v_neg_f64, UniOpVV::kNegF64)
  DEFINE_OP_2V(v_not_f32, UniOpVV::kNotF32)
  DEFINE_OP_2V(v_not_f64, UniOpVV::kNotF64)
  DEFINE_OP_2V(s_trunc_f32, UniOpVV::kTruncF32S)
  DEFINE_OP_2V(s_trunc_f64, UniOpVV::kTruncF64S)
  DEFINE_OP_2V(v_trunc_f32, UniOpVV::kTruncF32)
  DEFINE_OP_2V(v_trunc_f64, UniOpVV::kTruncF64)
  DEFINE_OP_2V(s_floor_f32, UniOpVV::kFloorF32S)
  DEFINE_OP_2V(s_floor_f64, UniOpVV::kFloorF64S)
  DEFINE_OP_2V(v_floor_f32, UniOpVV::kFloorF32)
  DEFINE_OP_2V(v_floor_f64, UniOpVV::kFloorF64)
  DEFINE_OP_2V(s_ceil_f32, UniOpVV::kCeilF32S)
  DEFINE_OP_2V(s_ceil_f64, UniOpVV::kCeilF64S)
  DEFINE_OP_2V(v_ceil_f32, UniOpVV::kCeilF32)
  DEFINE_OP_2V(v_ceil_f64, UniOpVV::kCeilF64)
  DEFINE_OP_2V(s_round_even_f32, UniOpVV::kRoundEvenF32S)
  DEFINE_OP_2V(s_round_even_f64, UniOpVV::kRoundEvenF64S)
  DEFINE_OP_2V(v_round_even_f32, UniOpVV::kRoundEvenF32)
  DEFINE_OP_2V(v_round_even_f64, UniOpVV::kRoundEvenF64)
  DEFINE_OP_2V(s_round_half_away_f32, UniOpVV::kRoundHalfAwayF32S)
  DEFINE_OP_2V(s_round_half_away_f64, UniOpVV::kRoundHalfAwayF64S)
  DEFINE_OP_2V(v_round_half_away_f32, UniOpVV::kRoundHalfAwayF32)
  DEFINE_OP_2V(v_round_half_away_f64, UniOpVV::kRoundHalfAwayF64)
  DEFINE_OP_2V(s_round_half_up_f32, UniOpVV::kRoundHalfUpF32S)
  DEFINE_OP_2V(s_round_half_up_f64, UniOpVV::kRoundHalfUpF64S)
  DEFINE_OP_2V(v_round_half_up_f32, UniOpVV::kRoundHalfUpF32)
  DEFINE_OP_2V(v_round_half_up_f64, UniOpVV::kRoundHalfUpF64)
  DEFINE_OP_2V(v_rcp_f32, UniOpVV::kRcpF32)
  DEFINE_OP_2V(v_rcp_f64, UniOpVV::kRcpF64)
  DEFINE_OP_2V(s_sqrt_f32, UniOpVV::kSqrtF32S)
  DEFINE_OP_2V(s_sqrt_f64, UniOpVV::kSqrtF64S)
  DEFINE_OP_2V(v_sqrt_f32, UniOpVV::kSqrtF32)
  DEFINE_OP_2V(v_sqrt_f64, UniOpVV::kSqrtF64)
  DEFINE_OP_2V(s_cvt_f32_to_f64, UniOpVV::kCvtF32ToF64S)
  DEFINE_OP_2V(s_cvt_f64_to_f32, UniOpVV::kCvtF64ToF32S)
  DEFINE_OP_2V(v_cvt_i32_to_f32, UniOpVV::kCvtI32ToF32)
  DEFINE_OP_2V(v_cvt_f32_lo_to_f64, UniOpVV::kCvtF32LoToF64)
  DEFINE_OP_2V(v_cvt_f32_hi_to_f64, UniOpVV::kCvtF32HiToF64)
  DEFINE_OP_2V(v_cvt_f64_to_f32_lo, UniOpVV::kCvtF64ToF32Lo)
  DEFINE_OP_2V(v_cvt_f64_to_f32_hi, UniOpVV::kCvtF64ToF32Hi)
  DEFINE_OP_2V(v_cvt_i32_lo_to_f64, UniOpVV::kCvtI32LoToF64)
  DEFINE_OP_2V(v_cvt_i32_hi_to_f64, UniOpVV::kCvtI32HiToF64)
  DEFINE_OP_2V(v_cvt_trunc_f32_to_i32, UniOpVV::kCvtTruncF32ToI32)
  DEFINE_OP_2V(v_cvt_trunc_f64_to_i32_lo, UniOpVV::kCvtTruncF64ToI32Lo)
  DEFINE_OP_2V(v_cvt_trunc_f64_to_i32_hi, UniOpVV::kCvtTruncF64ToI32Hi)
  DEFINE_OP_2V(v_cvt_round_f32_to_i32, UniOpVV::kCvtRoundF32ToI32)
  DEFINE_OP_2V(v_cvt_round_f64_to_i32_lo, UniOpVV::kCvtRoundF64ToI32Lo)
  DEFINE_OP_2V(v_cvt_round_f64_to_i32_hi, UniOpVV::kCvtRoundF64ToI32Hi)

  DEFINE_OP_2VI(v_slli_i16, UniOpVVI::kSllU16)
  DEFINE_OP_2VI(v_slli_u16, UniOpVVI::kSllU16)
  DEFINE_OP_2VI(v_slli_i32, UniOpVVI::kSllU32)
  DEFINE_OP_2VI(v_slli_u32, UniOpVVI::kSllU32)
  DEFINE_OP_2VI(v_slli_i64, UniOpVVI::kSllU64)
  DEFINE_OP_2VI(v_slli_u64, UniOpVVI::kSllU64)
  DEFINE_OP_2VI(v_srli_u16, UniOpVVI::kSrlU16)
  DEFINE_OP_2VI(v_srli_u32, UniOpVVI::kSrlU32)
  DEFINE_OP_2VI(v_srli_u64, UniOpVVI::kSrlU64)
  DEFINE_OP_2VI(v_srai_i16, UniOpVVI::kSraI16)
  DEFINE_OP_2VI(v_srai_i32, UniOpVVI::kSraI32)
  DEFINE_OP_2VI(v_srai_i64, UniOpVVI::kSraI64)
  DEFINE_OP_2VI(v_sllb_u128, UniOpVVI::kSllbU128)
  DEFINE_OP_2VI(v_srlb_u128, UniOpVVI::kSrlbU128)
  DEFINE_OP_2VI_WRAP(v_swizzle_u16x4, Swizzle4, UniOpVVI::kSwizzleU16x4)
  DEFINE_OP_2VI_WRAP(v_swizzle_lo_u16x4, Swizzle4, UniOpVVI::kSwizzleLoU16x4)
  DEFINE_OP_2VI_WRAP(v_swizzle_hi_u16x4, Swizzle4, UniOpVVI::kSwizzleHiU16x4)
  DEFINE_OP_2VI_WRAP(v_swizzle_u32x4, Swizzle4, UniOpVVI::kSwizzleU32x4)
  DEFINE_OP_2VI_WRAP(v_swizzle_u64x2, Swizzle2, UniOpVVI::kSwizzleU64x2)
  DEFINE_OP_2VI_WRAP(v_swizzle_f32x4, Swizzle4, UniOpVVI::kSwizzleF32x4)
  DEFINE_OP_2VI_WRAP(v_swizzle_f64x2, Swizzle2, UniOpVVI::kSwizzleF64x2)
  DEFINE_OP_2VI_WRAP(v_swizzle_u64x4, Swizzle4, UniOpVVI::kSwizzleU64x4)
  DEFINE_OP_2VI_WRAP(v_swizzle_f64x4, Swizzle4, UniOpVVI::kSwizzleF64x4)
  DEFINE_OP_2VI(v_extract_v128, UniOpVVI::kExtractV128_I32)
  DEFINE_OP_2VI(v_extract_v128_i32, UniOpVVI::kExtractV128_I32)
  DEFINE_OP_2VI(v_extract_v128_i64, UniOpVVI::kExtractV128_I64)
  DEFINE_OP_2VI(v_extract_v128_f32, UniOpVVI::kExtractV128_F32)
  DEFINE_OP_2VI(v_extract_v128_f64, UniOpVVI::kExtractV128_F64)
  DEFINE_OP_2VI(v_extract_v256, UniOpVVI::kExtractV256_I32)
  DEFINE_OP_2VI(v_extract_v256_i32, UniOpVVI::kExtractV256_I32)
  DEFINE_OP_2VI(v_extract_v256_i64, UniOpVVI::kExtractV256_I64)
  DEFINE_OP_2VI(v_extract_v256_f32, UniOpVVI::kExtractV256_F32)
  DEFINE_OP_2VI(v_extract_v256_f64, UniOpVVI::kExtractV256_F64)

#if defined(ASMJIT_UJIT_AARCH64)
  DEFINE_OP_2VI(v_srli_rnd_u16, UniOpVVI::kSrlRndU16)
  DEFINE_OP_2VI(v_srli_rnd_u32, UniOpVVI::kSrlRndU32)
  DEFINE_OP_2VI(v_srli_rnd_u64, UniOpVVI::kSrlRndU64)
  DEFINE_OP_2VI(v_srli_acc_u16, UniOpVVI::kSrlAccU16)
  DEFINE_OP_2VI(v_srli_acc_u32, UniOpVVI::kSrlAccU32)
  DEFINE_OP_2VI(v_srli_acc_u64, UniOpVVI::kSrlAccU64)
  DEFINE_OP_2VI(v_srli_rnd_acc_u16, UniOpVVI::kSrlRndAccU16)
  DEFINE_OP_2VI(v_srli_rnd_acc_u32, UniOpVVI::kSrlRndAccU32)
  DEFINE_OP_2VI(v_srli_rnd_acc_u64, UniOpVVI::kSrlRndAccU64)

  DEFINE_OP_2VI(v_srlni_lo_u16, UniOpVVI::kSrlnLoU16)
  DEFINE_OP_2VI(v_srlni_hi_u16, UniOpVVI::kSrlnHiU16)
  DEFINE_OP_2VI(v_srlni_lo_u32, UniOpVVI::kSrlnLoU32)
  DEFINE_OP_2VI(v_srlni_hi_u32, UniOpVVI::kSrlnHiU32)
  DEFINE_OP_2VI(v_srlni_lo_u64, UniOpVVI::kSrlnLoU64)
  DEFINE_OP_2VI(v_srlni_hi_u64, UniOpVVI::kSrlnHiU64)

  DEFINE_OP_2VI(v_srlni_rnd_lo_u16, UniOpVVI::kSrlnRndLoU16)
  DEFINE_OP_2VI(v_srlni_rnd_hi_u16, UniOpVVI::kSrlnRndHiU16)
  DEFINE_OP_2VI(v_srlni_rnd_lo_u32, UniOpVVI::kSrlnRndLoU32)
  DEFINE_OP_2VI(v_srlni_rnd_hi_u32, UniOpVVI::kSrlnRndHiU32)
  DEFINE_OP_2VI(v_srlni_rnd_lo_u64, UniOpVVI::kSrlnRndLoU64)
  DEFINE_OP_2VI(v_srlni_rnd_hi_u64, UniOpVVI::kSrlnRndHiU64)
#endif // ASMJIT_UJIT_AARCH64

  DEFINE_OP_VM_U(v_load8, UniOpVM::kLoad8, 1)
  DEFINE_OP_VM_U(v_loadu16, UniOpVM::kLoad16_U16, 1)
  DEFINE_OP_VM_A(v_loada16, UniOpVM::kLoad16_U16, 2)
  DEFINE_OP_VM_U(v_loadu32, UniOpVM::kLoad32_U32, 1)
  DEFINE_OP_VM_A(v_loada32, UniOpVM::kLoad32_U32, 4)
  DEFINE_OP_VM_U(v_loadu32_u32, UniOpVM::kLoad32_U32, 1)
  DEFINE_OP_VM_A(v_loada32_u32, UniOpVM::kLoad32_U32, 4)
  DEFINE_OP_VM_U(v_loadu32_f32, UniOpVM::kLoad32_F32, 1)
  DEFINE_OP_VM_A(v_loada32_f32, UniOpVM::kLoad32_F32, 4)
  DEFINE_OP_VM_U(v_loadu64, UniOpVM::kLoad64_U32, 1)
  DEFINE_OP_VM_A(v_loada64, UniOpVM::kLoad64_U32, 8)
  DEFINE_OP_VM_U(v_loadu64_u32, UniOpVM::kLoad64_U32, 1)
  DEFINE_OP_VM_A(v_loada64_u32, UniOpVM::kLoad64_U32, 8)
  DEFINE_OP_VM_U(v_loadu64_u64, UniOpVM::kLoad64_U64, 1)
  DEFINE_OP_VM_A(v_loada64_u64, UniOpVM::kLoad64_U64, 8)
  DEFINE_OP_VM_U(v_loadu64_f32, UniOpVM::kLoad64_F32, 1)
  DEFINE_OP_VM_A(v_loada64_f32, UniOpVM::kLoad64_F32, 8)
  DEFINE_OP_VM_U(v_loadu64_f64, UniOpVM::kLoad64_F64, 1)
  DEFINE_OP_VM_A(v_loada64_f64, UniOpVM::kLoad64_F64, 8)
  DEFINE_OP_VM_U(v_loadu128, UniOpVM::kLoad128_U32, 1)
  DEFINE_OP_VM_A(v_loada128, UniOpVM::kLoad128_U32, 16)
  DEFINE_OP_VM_U(v_loadu128_u32, UniOpVM::kLoad128_U32, 1)
  DEFINE_OP_VM_A(v_loada128_u32, UniOpVM::kLoad128_U32, 16)
  DEFINE_OP_VM_U(v_loadu128_u64, UniOpVM::kLoad128_U64, 1)
  DEFINE_OP_VM_A(v_loada128_u64, UniOpVM::kLoad128_U64, 16)
  DEFINE_OP_VM_U(v_loadu128_f32, UniOpVM::kLoad128_F32, 1)
  DEFINE_OP_VM_A(v_loada128_f32, UniOpVM::kLoad128_F32, 16)
  DEFINE_OP_VM_U(v_loadu128_f64, UniOpVM::kLoad128_F64, 1)
  DEFINE_OP_VM_A(v_loada128_f64, UniOpVM::kLoad128_F64, 16)
  DEFINE_OP_VM_U(v_loadu256, UniOpVM::kLoad256_U32, 1)
  DEFINE_OP_VM_A(v_loada256, UniOpVM::kLoad256_U32, 32)
  DEFINE_OP_VM_U(v_loadu256_u32, UniOpVM::kLoad256_U32, 1)
  DEFINE_OP_VM_A(v_loada256_u32, UniOpVM::kLoad256_U32, 32)
  DEFINE_OP_VM_U(v_loadu256_u64, UniOpVM::kLoad256_U64, 1)
  DEFINE_OP_VM_A(v_loada256_u64, UniOpVM::kLoad256_U64, 32)
  DEFINE_OP_VM_U(v_loadu256_f32, UniOpVM::kLoad256_F32, 1)
  DEFINE_OP_VM_A(v_loada256_f32, UniOpVM::kLoad256_F32, 32)
  DEFINE_OP_VM_U(v_loadu256_f64, UniOpVM::kLoad256_F64, 1)
  DEFINE_OP_VM_A(v_loada256_f64, UniOpVM::kLoad256_F64, 32)
  DEFINE_OP_VM_U(v_loadu512, UniOpVM::kLoad512_U32, 1)
  DEFINE_OP_VM_A(v_loada512, UniOpVM::kLoad512_U32, 64)
  DEFINE_OP_VM_U(v_loadu512_u32, UniOpVM::kLoad512_U32, 1)
  DEFINE_OP_VM_A(v_loada512_u32, UniOpVM::kLoad512_U32, 64)
  DEFINE_OP_VM_U(v_loadu512_u64, UniOpVM::kLoad512_U64, 1)
  DEFINE_OP_VM_A(v_loada512_u64, UniOpVM::kLoad512_U64, 64)
  DEFINE_OP_VM_U(v_loadu512_f32, UniOpVM::kLoad512_F32, 1)
  DEFINE_OP_VM_A(v_loada512_f32, UniOpVM::kLoad512_F32, 64)
  DEFINE_OP_VM_U(v_loadu512_f64, UniOpVM::kLoad512_F64, 1)
  DEFINE_OP_VM_A(v_loada512_f64, UniOpVM::kLoad512_F64, 64)
  DEFINE_OP_VM_U(v_loaduvec, UniOpVM::kLoadN_U32, 1)
  DEFINE_OP_VM_A(v_loadavec, UniOpVM::kLoadN_U32, 0)
  DEFINE_OP_VM_U(v_loaduvec_u32, UniOpVM::kLoadN_U32, 1)
  DEFINE_OP_VM_A(v_loadavec_u32, UniOpVM::kLoadN_U32, 0)
  DEFINE_OP_VM_U(v_loaduvec_u64, UniOpVM::kLoadN_U64, 1)
  DEFINE_OP_VM_A(v_loadavec_u64, UniOpVM::kLoadN_U64, 0)
  DEFINE_OP_VM_U(v_loaduvec_f32, UniOpVM::kLoadN_F32, 1)
  DEFINE_OP_VM_A(v_loadavec_f32, UniOpVM::kLoadN_F32, 0)
  DEFINE_OP_VM_U(v_loaduvec_f64, UniOpVM::kLoadN_F64, 1)
  DEFINE_OP_VM_A(v_loadavec_f64, UniOpVM::kLoadN_F64, 0)

  DEFINE_OP_VM_A(v_loadu16_u8_to_u64, UniOpVM::kLoadCvt16_U8ToU64, 1)
  DEFINE_OP_VM_A(v_loada16_u8_to_u64, UniOpVM::kLoadCvt16_U8ToU64, 2)
  DEFINE_OP_VM_A(v_loadu32_u8_to_u64, UniOpVM::kLoadCvt32_U8ToU64, 1)
  DEFINE_OP_VM_A(v_loada32_u8_to_u64, UniOpVM::kLoadCvt32_U8ToU64, 2)
  DEFINE_OP_VM_A(v_loadu64_u8_to_u64, UniOpVM::kLoadCvt64_U8ToU64, 1)
  DEFINE_OP_VM_A(v_loada64_u8_to_u64, UniOpVM::kLoadCvt64_U8ToU64, 2)

  DEFINE_OP_VM_A(v_loadu32_i8_to_i16, UniOpVM::kLoadCvt32_I8ToI16, 1)
  DEFINE_OP_VM_A(v_loada32_i8_to_i16, UniOpVM::kLoadCvt32_I8ToI16, 4)
  DEFINE_OP_VM_A(v_loadu32_u8_to_u16, UniOpVM::kLoadCvt32_U8ToU16, 1)
  DEFINE_OP_VM_A(v_loada32_u8_to_u16, UniOpVM::kLoadCvt32_U8ToU16, 4)
  DEFINE_OP_VM_A(v_loadu32_i8_to_i32, UniOpVM::kLoadCvt32_I8ToI32, 1)
  DEFINE_OP_VM_A(v_loada32_i8_to_i32, UniOpVM::kLoadCvt32_I8ToI32, 4)
  DEFINE_OP_VM_A(v_loadu32_u8_to_u32, UniOpVM::kLoadCvt32_U8ToU32, 1)
  DEFINE_OP_VM_A(v_loada32_u8_to_u32, UniOpVM::kLoadCvt32_U8ToU32, 4)
  DEFINE_OP_VM_A(v_loadu32_i16_to_i32, UniOpVM::kLoadCvt32_I16ToI32, 1)
  DEFINE_OP_VM_A(v_loada32_i16_to_i32, UniOpVM::kLoadCvt32_I16ToI32, 4)
  DEFINE_OP_VM_A(v_loadu32_u16_to_u32, UniOpVM::kLoadCvt32_U16ToU32, 1)
  DEFINE_OP_VM_A(v_loada32_u16_to_u32, UniOpVM::kLoadCvt32_U16ToU32, 4)
  DEFINE_OP_VM_A(v_loadu32_i32_to_i64, UniOpVM::kLoadCvt32_I32ToI64, 1)
  DEFINE_OP_VM_A(v_loada32_i32_to_i64, UniOpVM::kLoadCvt32_I32ToI64, 4)
  DEFINE_OP_VM_A(v_loadu32_u32_to_u64, UniOpVM::kLoadCvt32_U32ToU64, 1)
  DEFINE_OP_VM_A(v_loada32_u32_to_u64, UniOpVM::kLoadCvt32_U32ToU64, 4)
  DEFINE_OP_VM_A(v_loadu64_i8_to_i16, UniOpVM::kLoadCvt64_I8ToI16, 1)
  DEFINE_OP_VM_A(v_loada64_i8_to_i16, UniOpVM::kLoadCvt64_I8ToI16, 8)
  DEFINE_OP_VM_A(v_loadu64_u8_to_u16, UniOpVM::kLoadCvt64_U8ToU16, 1)
  DEFINE_OP_VM_A(v_loada64_u8_to_u16, UniOpVM::kLoadCvt64_U8ToU16, 8)
  DEFINE_OP_VM_A(v_loadu64_i8_to_i32, UniOpVM::kLoadCvt64_I8ToI32, 1)
  DEFINE_OP_VM_A(v_loada64_i8_to_i32, UniOpVM::kLoadCvt64_I8ToI32, 8)
  DEFINE_OP_VM_A(v_loadu64_u8_to_u32, UniOpVM::kLoadCvt64_U8ToU32, 1)
  DEFINE_OP_VM_A(v_loada64_u8_to_u32, UniOpVM::kLoadCvt64_U8ToU32, 8)
  DEFINE_OP_VM_A(v_loadu64_i16_to_i32, UniOpVM::kLoadCvt64_I16ToI32, 1)
  DEFINE_OP_VM_A(v_loada64_i16_to_i32, UniOpVM::kLoadCvt64_I16ToI32, 8)
  DEFINE_OP_VM_A(v_loadu64_u16_to_u32, UniOpVM::kLoadCvt64_U16ToU32, 1)
  DEFINE_OP_VM_A(v_loada64_u16_to_u32, UniOpVM::kLoadCvt64_U16ToU32, 8)
  DEFINE_OP_VM_A(v_loadu64_i32_to_i64, UniOpVM::kLoadCvt64_I32ToI64, 1)
  DEFINE_OP_VM_A(v_loada64_i32_to_i64, UniOpVM::kLoadCvt64_I32ToI64, 8)
  DEFINE_OP_VM_A(v_loadu64_u32_to_u64, UniOpVM::kLoadCvt64_U32ToU64, 1)
  DEFINE_OP_VM_A(v_loada64_u32_to_u64, UniOpVM::kLoadCvt64_U32ToU64, 8)
  DEFINE_OP_VM_A(v_loadu128_i8_to_i16, UniOpVM::kLoadCvt128_I8ToI16, 1)
  DEFINE_OP_VM_A(v_loada128_i8_to_i16, UniOpVM::kLoadCvt128_I8ToI16, 16)
  DEFINE_OP_VM_A(v_loadu128_u8_to_u16, UniOpVM::kLoadCvt128_U8ToU16, 1)
  DEFINE_OP_VM_A(v_loada128_u8_to_u16, UniOpVM::kLoadCvt128_U8ToU16, 16)
  DEFINE_OP_VM_A(v_loadu128_i8_to_i32, UniOpVM::kLoadCvt128_I8ToI32, 1)
  DEFINE_OP_VM_A(v_loada128_i8_to_i32, UniOpVM::kLoadCvt128_I8ToI32, 16)
  DEFINE_OP_VM_A(v_loadu128_u8_to_u32, UniOpVM::kLoadCvt128_U8ToU32, 1)
  DEFINE_OP_VM_A(v_loada128_u8_to_u32, UniOpVM::kLoadCvt128_U8ToU32, 16)
  DEFINE_OP_VM_A(v_loadu128_i16_to_i32, UniOpVM::kLoadCvt128_I16ToI32, 1)
  DEFINE_OP_VM_A(v_loada128_i16_to_i32, UniOpVM::kLoadCvt128_I16ToI32, 16)
  DEFINE_OP_VM_A(v_loadu128_u16_to_u32, UniOpVM::kLoadCvt128_U16ToU32, 1)
  DEFINE_OP_VM_A(v_loada128_u16_to_u32, UniOpVM::kLoadCvt128_U16ToU32, 16)
  DEFINE_OP_VM_A(v_loadu128_i32_to_i64, UniOpVM::kLoadCvt128_I32ToI64, 1)
  DEFINE_OP_VM_A(v_loada128_i32_to_i64, UniOpVM::kLoadCvt128_I32ToI64, 16)
  DEFINE_OP_VM_A(v_loadu128_u32_to_u64, UniOpVM::kLoadCvt128_U32ToU64, 1)
  DEFINE_OP_VM_A(v_loada128_u32_to_u64, UniOpVM::kLoadCvt128_U32ToU64, 16)
  DEFINE_OP_VM_A(v_loadu256_i8_to_i16, UniOpVM::kLoadCvt256_I8ToI16, 1)
  DEFINE_OP_VM_A(v_loada256_i8_to_i16, UniOpVM::kLoadCvt256_I8ToI16, 32)
  DEFINE_OP_VM_A(v_loadu256_u8_to_u16, UniOpVM::kLoadCvt256_U8ToU16, 1)
  DEFINE_OP_VM_A(v_loada256_u8_to_u16, UniOpVM::kLoadCvt256_U8ToU16, 32)
  DEFINE_OP_VM_A(v_loadu256_i16_to_i32, UniOpVM::kLoadCvt256_I16ToI32, 1)
  DEFINE_OP_VM_A(v_loada256_i16_to_i32, UniOpVM::kLoadCvt256_I16ToI32, 32)
  DEFINE_OP_VM_A(v_loadu256_u16_to_u32, UniOpVM::kLoadCvt256_U16ToU32, 1)
  DEFINE_OP_VM_A(v_loada256_u16_to_u32, UniOpVM::kLoadCvt256_U16ToU32, 32)
  DEFINE_OP_VM_A(v_loadu256_i32_to_i64, UniOpVM::kLoadCvt256_I32ToI64, 1)
  DEFINE_OP_VM_A(v_loada256_i32_to_i64, UniOpVM::kLoadCvt256_I32ToI64, 32)
  DEFINE_OP_VM_A(v_loadu256_u32_to_u64, UniOpVM::kLoadCvt256_U32ToU64, 1)
  DEFINE_OP_VM_A(v_loada256_u32_to_u64, UniOpVM::kLoadCvt256_U32ToU64, 32)
  DEFINE_OP_VM_A(v_loaduvec_i8_to_i16, UniOpVM::kLoadCvtN_I8ToI16, 1)
  DEFINE_OP_VM_A(v_loadavec_i8_to_i16, UniOpVM::kLoadCvtN_I8ToI16, 0)
  DEFINE_OP_VM_A(v_loaduvec_u8_to_u16, UniOpVM::kLoadCvtN_U8ToU16, 1)
  DEFINE_OP_VM_A(v_loadavec_u8_to_u16, UniOpVM::kLoadCvtN_U8ToU16, 0)
  DEFINE_OP_VM_A(v_loaduvec_i8_to_i32, UniOpVM::kLoadCvtN_I8ToI32, 1)
  DEFINE_OP_VM_A(v_loadavec_i8_to_i32, UniOpVM::kLoadCvtN_I8ToI32, 0)
  DEFINE_OP_VM_A(v_loaduvec_u8_to_u32, UniOpVM::kLoadCvtN_U8ToU32, 1)
  DEFINE_OP_VM_A(v_loadavec_u8_to_u32, UniOpVM::kLoadCvtN_U8ToU32, 0)
  DEFINE_OP_VM_A(v_loaduvec_u8_to_u64, UniOpVM::kLoadCvtN_U8ToU64, 1)
  DEFINE_OP_VM_A(v_loadavec_u8_to_u64, UniOpVM::kLoadCvtN_U8ToU64, 0)
  DEFINE_OP_VM_A(v_loaduvec_i16_to_i32, UniOpVM::kLoadCvtN_I16ToI32, 1)
  DEFINE_OP_VM_A(v_loadavec_i16_to_i32, UniOpVM::kLoadCvtN_I16ToI32, 0)
  DEFINE_OP_VM_A(v_loaduvec_u16_to_u32, UniOpVM::kLoadCvtN_U16ToU32, 1)
  DEFINE_OP_VM_A(v_loadavec_u16_to_u32, UniOpVM::kLoadCvtN_U16ToU32, 0)
  DEFINE_OP_VM_A(v_loaduvec_i32_to_i64, UniOpVM::kLoadCvtN_I32ToI64, 1)
  DEFINE_OP_VM_A(v_loadavec_i32_to_i64, UniOpVM::kLoadCvtN_I32ToI64, 0)
  DEFINE_OP_VM_A(v_loaduvec_u32_to_u64, UniOpVM::kLoadCvtN_U32ToU64, 1)
  DEFINE_OP_VM_A(v_loadavec_u32_to_u64, UniOpVM::kLoadCvtN_U32ToU64, 0)

  DEFINE_OP_VM_I(v_insert_u8, UniOpVM::kLoadInsertU8, 1)
  DEFINE_OP_VM_I(v_insert_u16, UniOpVM::kLoadInsertU16, 1)
  DEFINE_OP_VM_I(v_insert_u32, UniOpVM::kLoadInsertU32, 1)
  DEFINE_OP_VM_I(v_insert_u64, UniOpVM::kLoadInsertU64, 1)
  DEFINE_OP_VM_I(v_insert_f32, UniOpVM::kLoadInsertF32, 1)
  DEFINE_OP_VM_I(v_insert_f32x2, UniOpVM::kLoadInsertF32x2, 1)
  DEFINE_OP_VM_I(v_insert_f64, UniOpVM::kLoadInsertF64, 1)

  DEFINE_OP_MV_U(v_store8, UniOpMV::kStore8, 1)
  DEFINE_OP_MV_U(v_storeu16, UniOpMV::kStore16_U16, 1)
  DEFINE_OP_MV_A(v_storea16, UniOpMV::kStore16_U16, 2)
  DEFINE_OP_MV_U(v_storeu32, UniOpMV::kStore32_U32, 1)
  DEFINE_OP_MV_A(v_storea32, UniOpMV::kStore32_U32, 4)
  DEFINE_OP_MV_U(v_storeu32_u32, UniOpMV::kStore32_U32, 1)
  DEFINE_OP_MV_A(v_storea32_u32, UniOpMV::kStore32_U32, 4)
  DEFINE_OP_MV_U(v_storeu32_f32, UniOpMV::kStore32_F32, 1)
  DEFINE_OP_MV_A(v_storea32_f32, UniOpMV::kStore32_F32, 4)
  DEFINE_OP_MV_U(v_storeu64, UniOpMV::kStore64_U32, 1)
  DEFINE_OP_MV_A(v_storea64, UniOpMV::kStore64_U32, 8)
  DEFINE_OP_MV_U(v_storeu64_u32, UniOpMV::kStore64_U32, 1)
  DEFINE_OP_MV_A(v_storea64_u32, UniOpMV::kStore64_U32, 8)
  DEFINE_OP_MV_U(v_storeu64_u64, UniOpMV::kStore64_U64, 1)
  DEFINE_OP_MV_A(v_storea64_u64, UniOpMV::kStore64_U64, 8)
  DEFINE_OP_MV_U(v_storeu64_f32, UniOpMV::kStore64_F32, 1)
  DEFINE_OP_MV_A(v_storea64_f32, UniOpMV::kStore64_F32, 8)
  DEFINE_OP_MV_U(v_storeu64_f64, UniOpMV::kStore64_F64, 1)
  DEFINE_OP_MV_A(v_storea64_f64, UniOpMV::kStore64_F64, 8)
  DEFINE_OP_MV_U(v_storeu128, UniOpMV::kStore128_U32, 1)
  DEFINE_OP_MV_A(v_storea128, UniOpMV::kStore128_U32, 16)
  DEFINE_OP_MV_U(v_storeu128_u32, UniOpMV::kStore128_U32, 1)
  DEFINE_OP_MV_A(v_storea128_u32, UniOpMV::kStore128_U32, 16)
  DEFINE_OP_MV_U(v_storeu128_u64, UniOpMV::kStore128_U64, 1)
  DEFINE_OP_MV_A(v_storea128_u64, UniOpMV::kStore128_U64, 16)
  DEFINE_OP_MV_U(v_storeu128_f32, UniOpMV::kStore128_F32, 1)
  DEFINE_OP_MV_A(v_storea128_f32, UniOpMV::kStore128_F32, 16)
  DEFINE_OP_MV_U(v_storeu128_f64, UniOpMV::kStore128_F64, 1)
  DEFINE_OP_MV_A(v_storea128_f64, UniOpMV::kStore128_F64, 16)
  DEFINE_OP_MV_U(v_storeu256, UniOpMV::kStore256_U32, 1)
  DEFINE_OP_MV_A(v_storea256, UniOpMV::kStore256_U32, 32)
  DEFINE_OP_MV_U(v_storeu256_u32, UniOpMV::kStore256_U32, 1)
  DEFINE_OP_MV_A(v_storea256_u32, UniOpMV::kStore256_U32, 32)
  DEFINE_OP_MV_U(v_storeu256_u64, UniOpMV::kStore256_U64, 1)
  DEFINE_OP_MV_A(v_storea256_u64, UniOpMV::kStore256_U64, 32)
  DEFINE_OP_MV_U(v_storeu256_f32, UniOpMV::kStore256_F32, 1)
  DEFINE_OP_MV_A(v_storea256_f32, UniOpMV::kStore256_F32, 32)
  DEFINE_OP_MV_U(v_storeu256_f64, UniOpMV::kStore256_F64, 1)
  DEFINE_OP_MV_A(v_storea256_f64, UniOpMV::kStore256_F64, 32)
  DEFINE_OP_MV_U(v_storeu512, UniOpMV::kStore512_U32, 1)
  DEFINE_OP_MV_A(v_storea512, UniOpMV::kStore512_U32, 64)
  DEFINE_OP_MV_U(v_storeu512_u32, UniOpMV::kStore512_U32, 1)
  DEFINE_OP_MV_A(v_storea512_u32, UniOpMV::kStore512_U32, 64)
  DEFINE_OP_MV_U(v_storeu512_u64, UniOpMV::kStore512_U64, 1)
  DEFINE_OP_MV_A(v_storea512_u64, UniOpMV::kStore512_U64, 64)
  DEFINE_OP_MV_U(v_storeu512_f32, UniOpMV::kStore512_F32, 1)
  DEFINE_OP_MV_A(v_storea512_f32, UniOpMV::kStore512_F32, 64)
  DEFINE_OP_MV_U(v_storeu512_f64, UniOpMV::kStore512_F64, 1)
  DEFINE_OP_MV_A(v_storea512_f64, UniOpMV::kStore512_F64, 64)
  DEFINE_OP_MV_U(v_storeuvec, UniOpMV::kStoreN_U32, 1)
  DEFINE_OP_MV_A(v_storeavec, UniOpMV::kStoreN_U32, 0)
  DEFINE_OP_MV_U(v_storeuvec_u32, UniOpMV::kStoreN_U32, 1)
  DEFINE_OP_MV_A(v_storeavec_u32, UniOpMV::kStoreN_U32, 0)
  DEFINE_OP_MV_U(v_storeuvec_u64, UniOpMV::kStoreN_U64, 1)
  DEFINE_OP_MV_A(v_storeavec_u64, UniOpMV::kStoreN_U64, 0)
  DEFINE_OP_MV_U(v_storeuvec_f32, UniOpMV::kStoreN_F32, 1)
  DEFINE_OP_MV_A(v_storeavec_f32, UniOpMV::kStoreN_F32, 0)
  DEFINE_OP_MV_U(v_storeuvec_f64, UniOpMV::kStoreN_F64, 1)
  DEFINE_OP_MV_A(v_storeavec_f64, UniOpMV::kStoreN_F64, 0)

  DEFINE_OP_MV_I(v_store_extract_u16, UniOpMV::kStoreExtractU16, 1)
  DEFINE_OP_MV_I(v_store_extract_u32, UniOpMV::kStoreExtractU32, 1)
  DEFINE_OP_MV_I(v_store_extract_u64, UniOpMV::kStoreExtractU64, 1)

  DEFINE_OP_3V(v_and_i32, UniOpVVV::kAndU32)
  DEFINE_OP_3V(v_and_u32, UniOpVVV::kAndU32)
  DEFINE_OP_3V(v_and_i64, UniOpVVV::kAndU64)
  DEFINE_OP_3V(v_and_u64, UniOpVVV::kAndU64)
  DEFINE_OP_3V(v_or_i32, UniOpVVV::kOrU32)
  DEFINE_OP_3V(v_or_u32, UniOpVVV::kOrU32)
  DEFINE_OP_3V(v_or_i64, UniOpVVV::kOrU64)
  DEFINE_OP_3V(v_or_u64, UniOpVVV::kOrU64)
  DEFINE_OP_3V(v_xor_i32, UniOpVVV::kXorU32)
  DEFINE_OP_3V(v_xor_u32, UniOpVVV::kXorU32)
  DEFINE_OP_3V(v_xor_i64, UniOpVVV::kXorU64)
  DEFINE_OP_3V(v_xor_u64, UniOpVVV::kXorU64)
  DEFINE_OP_3V(v_andn_i32, UniOpVVV::kAndnU32)
  DEFINE_OP_3V(v_andn_u32, UniOpVVV::kAndnU32)
  DEFINE_OP_3V(v_andn_i64, UniOpVVV::kAndnU64)
  DEFINE_OP_3V(v_andn_u64, UniOpVVV::kAndnU64)
  DEFINE_OP_3V(v_bic_i32, UniOpVVV::kBicU32)
  DEFINE_OP_3V(v_bic_u32, UniOpVVV::kBicU32)
  DEFINE_OP_3V(v_bic_i64, UniOpVVV::kBicU64)
  DEFINE_OP_3V(v_bic_u64, UniOpVVV::kBicU64)
  DEFINE_OP_3V(v_avgr_u8, UniOpVVV::kAvgrU8)
  DEFINE_OP_3V(v_avgr_u16, UniOpVVV::kAvgrU16)
  DEFINE_OP_3V(v_add_i8, UniOpVVV::kAddU8)
  DEFINE_OP_3V(v_add_u8, UniOpVVV::kAddU8)
  DEFINE_OP_3V(v_add_i16, UniOpVVV::kAddU16)
  DEFINE_OP_3V(v_add_u16, UniOpVVV::kAddU16)
  DEFINE_OP_3V(v_add_i32, UniOpVVV::kAddU32)
  DEFINE_OP_3V(v_add_u32, UniOpVVV::kAddU32)
  DEFINE_OP_3V(v_add_i64, UniOpVVV::kAddU64)
  DEFINE_OP_3V(v_add_u64, UniOpVVV::kAddU64)
  DEFINE_OP_3V(v_sub_i8, UniOpVVV::kSubU8)
  DEFINE_OP_3V(v_sub_u8, UniOpVVV::kSubU8)
  DEFINE_OP_3V(v_sub_i16, UniOpVVV::kSubU16)
  DEFINE_OP_3V(v_sub_u16, UniOpVVV::kSubU16)
  DEFINE_OP_3V(v_sub_i32, UniOpVVV::kSubU32)
  DEFINE_OP_3V(v_sub_u32, UniOpVVV::kSubU32)
  DEFINE_OP_3V(v_sub_i64, UniOpVVV::kSubU64)
  DEFINE_OP_3V(v_sub_u64, UniOpVVV::kSubU64)
  DEFINE_OP_3V(v_adds_i8, UniOpVVV::kAddsI8)
  DEFINE_OP_3V(v_adds_i16, UniOpVVV::kAddsI16)
  DEFINE_OP_3V(v_adds_u8, UniOpVVV::kAddsU8)
  DEFINE_OP_3V(v_adds_u16, UniOpVVV::kAddsU16)
  DEFINE_OP_3V(v_subs_i8, UniOpVVV::kSubsI8)
  DEFINE_OP_3V(v_subs_i16, UniOpVVV::kSubsI16)
  DEFINE_OP_3V(v_subs_u8, UniOpVVV::kSubsU8)
  DEFINE_OP_3V(v_subs_u16, UniOpVVV::kSubsU16)
  DEFINE_OP_3V(v_mul_i16, UniOpVVV::kMulU16)
  DEFINE_OP_3V(v_mul_u16, UniOpVVV::kMulU16)
  DEFINE_OP_3V(v_mul_i32, UniOpVVV::kMulU32)
  DEFINE_OP_3V(v_mul_u32, UniOpVVV::kMulU32)
  DEFINE_OP_3V(v_mul_i64, UniOpVVV::kMulU64)
  DEFINE_OP_3V(v_mul_u64, UniOpVVV::kMulU64)
  DEFINE_OP_3V(v_mul_u64_lo_u32, UniOpVVV::kMulU64_LoU32)
  DEFINE_OP_3V(v_mulh_i16, UniOpVVV::kMulhI16)
  DEFINE_OP_3V(v_mulh_u16, UniOpVVV::kMulhU16)
  DEFINE_OP_3V(v_mhadd_i16_to_i32, UniOpVVV::kMHAddI16_I32)
  DEFINE_OP_3V(v_min_i8, UniOpVVV::kMinI8)
  DEFINE_OP_3V(v_min_i16, UniOpVVV::kMinI16)
  DEFINE_OP_3V(v_min_i32, UniOpVVV::kMinI32)
  DEFINE_OP_3V(v_min_i64, UniOpVVV::kMinI64)
  DEFINE_OP_3V(v_min_u8, UniOpVVV::kMinU8)
  DEFINE_OP_3V(v_min_u16, UniOpVVV::kMinU16)
  DEFINE_OP_3V(v_min_u32, UniOpVVV::kMinU32)
  DEFINE_OP_3V(v_min_u64, UniOpVVV::kMinU64)
  DEFINE_OP_3V(v_max_i8, UniOpVVV::kMaxI8)
  DEFINE_OP_3V(v_max_i16, UniOpVVV::kMaxI16)
  DEFINE_OP_3V(v_max_i32, UniOpVVV::kMaxI32)
  DEFINE_OP_3V(v_max_i64, UniOpVVV::kMaxI64)
  DEFINE_OP_3V(v_max_u8, UniOpVVV::kMaxU8)
  DEFINE_OP_3V(v_max_u16, UniOpVVV::kMaxU16)
  DEFINE_OP_3V(v_max_u32, UniOpVVV::kMaxU32)
  DEFINE_OP_3V(v_max_u64, UniOpVVV::kMaxU64)
  DEFINE_OP_3V(v_cmp_eq_i8, UniOpVVV::kCmpEqU8)
  DEFINE_OP_3V(v_cmp_eq_u8, UniOpVVV::kCmpEqU8)
  DEFINE_OP_3V(v_cmp_eq_i16, UniOpVVV::kCmpEqU16)
  DEFINE_OP_3V(v_cmp_eq_u16, UniOpVVV::kCmpEqU16)
  DEFINE_OP_3V(v_cmp_eq_i32, UniOpVVV::kCmpEqU32)
  DEFINE_OP_3V(v_cmp_eq_u32, UniOpVVV::kCmpEqU32)
  DEFINE_OP_3V(v_cmp_eq_i64, UniOpVVV::kCmpEqU64)
  DEFINE_OP_3V(v_cmp_eq_u64, UniOpVVV::kCmpEqU64)
  DEFINE_OP_3V(v_cmp_gt_i8, UniOpVVV::kCmpGtI8)
  DEFINE_OP_3V(v_cmp_gt_u8, UniOpVVV::kCmpGtU8)
  DEFINE_OP_3V(v_cmp_gt_i16, UniOpVVV::kCmpGtI16)
  DEFINE_OP_3V(v_cmp_gt_u16, UniOpVVV::kCmpGtU16)
  DEFINE_OP_3V(v_cmp_gt_i32, UniOpVVV::kCmpGtI32)
  DEFINE_OP_3V(v_cmp_gt_u32, UniOpVVV::kCmpGtU32)
  DEFINE_OP_3V(v_cmp_gt_i64, UniOpVVV::kCmpGtI64)
  DEFINE_OP_3V(v_cmp_gt_u64, UniOpVVV::kCmpGtU64)
  DEFINE_OP_3V(v_cmp_ge_i8, UniOpVVV::kCmpGeI8)
  DEFINE_OP_3V(v_cmp_ge_u8, UniOpVVV::kCmpGeU8)
  DEFINE_OP_3V(v_cmp_ge_i16, UniOpVVV::kCmpGeI16)
  DEFINE_OP_3V(v_cmp_ge_u16, UniOpVVV::kCmpGeU16)
  DEFINE_OP_3V(v_cmp_ge_i32, UniOpVVV::kCmpGeI32)
  DEFINE_OP_3V(v_cmp_ge_u32, UniOpVVV::kCmpGeU32)
  DEFINE_OP_3V(v_cmp_ge_i64, UniOpVVV::kCmpGeI64)
  DEFINE_OP_3V(v_cmp_ge_u64, UniOpVVV::kCmpGeU64)
  DEFINE_OP_3V(v_cmp_lt_i8, UniOpVVV::kCmpLtI8)
  DEFINE_OP_3V(v_cmp_lt_u8, UniOpVVV::kCmpLtU8)
  DEFINE_OP_3V(v_cmp_lt_i16, UniOpVVV::kCmpLtI16)
  DEFINE_OP_3V(v_cmp_lt_u16, UniOpVVV::kCmpLtU16)
  DEFINE_OP_3V(v_cmp_lt_i32, UniOpVVV::kCmpLtI32)
  DEFINE_OP_3V(v_cmp_lt_u32, UniOpVVV::kCmpLtU32)
  DEFINE_OP_3V(v_cmp_lt_i64, UniOpVVV::kCmpLtI64)
  DEFINE_OP_3V(v_cmp_lt_u64, UniOpVVV::kCmpLtU64)
  DEFINE_OP_3V(v_cmp_le_i8, UniOpVVV::kCmpLeI8)
  DEFINE_OP_3V(v_cmp_le_u8, UniOpVVV::kCmpLeU8)
  DEFINE_OP_3V(v_cmp_le_i16, UniOpVVV::kCmpLeI16)
  DEFINE_OP_3V(v_cmp_le_u16, UniOpVVV::kCmpLeU16)
  DEFINE_OP_3V(v_cmp_le_i32, UniOpVVV::kCmpLeI32)
  DEFINE_OP_3V(v_cmp_le_u32, UniOpVVV::kCmpLeU32)
  DEFINE_OP_3V(v_cmp_le_i64, UniOpVVV::kCmpLeI64)
  DEFINE_OP_3V(v_cmp_le_u64, UniOpVVV::kCmpLeU64)
  DEFINE_OP_3V(v_and_f32, UniOpVVV::kAndF32)
  DEFINE_OP_3V(v_and_f64, UniOpVVV::kAndF64)
  DEFINE_OP_3V(v_or_f32, UniOpVVV::kOrF32)
  DEFINE_OP_3V(v_or_f64, UniOpVVV::kOrF64)
  DEFINE_OP_3V(v_xor_f32, UniOpVVV::kXorF32)
  DEFINE_OP_3V(v_xor_f64, UniOpVVV::kXorF64)
  DEFINE_OP_3V(v_andn_f32, UniOpVVV::kAndnF32)
  DEFINE_OP_3V(v_andn_f64, UniOpVVV::kAndnF64)
  DEFINE_OP_3V(v_bic_f32, UniOpVVV::kBicF32)
  DEFINE_OP_3V(v_bic_f64, UniOpVVV::kBicF64)
  DEFINE_OP_3V(s_add_f32, UniOpVVV::kAddF32S)
  DEFINE_OP_3V(s_add_f64, UniOpVVV::kAddF64S)
  DEFINE_OP_3V(v_add_f32, UniOpVVV::kAddF32)
  DEFINE_OP_3V(v_add_f64, UniOpVVV::kAddF64)
  DEFINE_OP_3V(s_sub_f32, UniOpVVV::kSubF32S)
  DEFINE_OP_3V(s_sub_f64, UniOpVVV::kSubF64S)
  DEFINE_OP_3V(v_sub_f32, UniOpVVV::kSubF32)
  DEFINE_OP_3V(v_sub_f64, UniOpVVV::kSubF64)
  DEFINE_OP_3V(s_mul_f32, UniOpVVV::kMulF32S)
  DEFINE_OP_3V(s_mul_f64, UniOpVVV::kMulF64S)
  DEFINE_OP_3V(v_mul_f32, UniOpVVV::kMulF32)
  DEFINE_OP_3V(v_mul_f64, UniOpVVV::kMulF64)
  DEFINE_OP_3V(s_div_f32, UniOpVVV::kDivF32S)
  DEFINE_OP_3V(s_div_f64, UniOpVVV::kDivF64S)
  DEFINE_OP_3V(v_div_f32, UniOpVVV::kDivF32)
  DEFINE_OP_3V(v_div_f64, UniOpVVV::kDivF64)
  DEFINE_OP_3V(s_mod_f32, UniOpVVV::kModF32S)
  DEFINE_OP_3V(s_mod_f64, UniOpVVV::kModF64S)
  DEFINE_OP_3V(v_mod_f32, UniOpVVV::kModF32)
  DEFINE_OP_3V(v_mod_f64, UniOpVVV::kModF64)
  DEFINE_OP_3V(s_min_f32, UniOpVVV::kMinF32S)
  DEFINE_OP_3V(s_min_f64, UniOpVVV::kMinF64S)
  DEFINE_OP_3V(v_min_f32, UniOpVVV::kMinF32)
  DEFINE_OP_3V(v_min_f64, UniOpVVV::kMinF64)
  DEFINE_OP_3V(s_max_f32, UniOpVVV::kMaxF32S)
  DEFINE_OP_3V(s_max_f64, UniOpVVV::kMaxF64S)
  DEFINE_OP_3V(v_max_f32, UniOpVVV::kMaxF32)
  DEFINE_OP_3V(v_max_f64, UniOpVVV::kMaxF64)
  DEFINE_OP_3V(s_cmp_eq_f32, UniOpVVV::kCmpEqF32S)
  DEFINE_OP_3V(s_cmp_eq_f64, UniOpVVV::kCmpEqF64S)
  DEFINE_OP_3V(v_cmp_eq_f32, UniOpVVV::kCmpEqF32)
  DEFINE_OP_3V(v_cmp_eq_f64, UniOpVVV::kCmpEqF64)
  DEFINE_OP_3V(s_cmp_ne_f32, UniOpVVV::kCmpNeF32S)
  DEFINE_OP_3V(s_cmp_ne_f64, UniOpVVV::kCmpNeF64S)
  DEFINE_OP_3V(v_cmp_ne_f32, UniOpVVV::kCmpNeF32)
  DEFINE_OP_3V(v_cmp_ne_f64, UniOpVVV::kCmpNeF64)
  DEFINE_OP_3V(s_cmp_gt_f32, UniOpVVV::kCmpGtF32S)
  DEFINE_OP_3V(s_cmp_gt_f64, UniOpVVV::kCmpGtF64S)
  DEFINE_OP_3V(v_cmp_gt_f32, UniOpVVV::kCmpGtF32)
  DEFINE_OP_3V(v_cmp_gt_f64, UniOpVVV::kCmpGtF64)
  DEFINE_OP_3V(s_cmp_ge_f32, UniOpVVV::kCmpGeF32S)
  DEFINE_OP_3V(s_cmp_ge_f64, UniOpVVV::kCmpGeF64S)
  DEFINE_OP_3V(v_cmp_ge_f32, UniOpVVV::kCmpGeF32)
  DEFINE_OP_3V(v_cmp_ge_f64, UniOpVVV::kCmpGeF64)
  DEFINE_OP_3V(s_cmp_lt_f32, UniOpVVV::kCmpLtF32S)
  DEFINE_OP_3V(s_cmp_lt_f64, UniOpVVV::kCmpLtF64S)
  DEFINE_OP_3V(v_cmp_lt_f32, UniOpVVV::kCmpLtF32)
  DEFINE_OP_3V(v_cmp_lt_f64, UniOpVVV::kCmpLtF64)
  DEFINE_OP_3V(s_cmp_le_f32, UniOpVVV::kCmpLeF32S)
  DEFINE_OP_3V(s_cmp_le_f64, UniOpVVV::kCmpLeF64S)
  DEFINE_OP_3V(v_cmp_le_f32, UniOpVVV::kCmpLeF32)
  DEFINE_OP_3V(v_cmp_le_f64, UniOpVVV::kCmpLeF64)
  DEFINE_OP_3V(s_cmp_ord_f32, UniOpVVV::kCmpOrdF32S)
  DEFINE_OP_3V(s_cmp_ord_f64, UniOpVVV::kCmpOrdF64S)
  DEFINE_OP_3V(v_cmp_ord_f32, UniOpVVV::kCmpOrdF32)
  DEFINE_OP_3V(v_cmp_ord_f64, UniOpVVV::kCmpOrdF64)
  DEFINE_OP_3V(s_cmp_unord_f32, UniOpVVV::kCmpUnordF32S)
  DEFINE_OP_3V(s_cmp_unord_f64, UniOpVVV::kCmpUnordF64S)
  DEFINE_OP_3V(v_cmp_unord_f32, UniOpVVV::kCmpUnordF32)
  DEFINE_OP_3V(v_cmp_unord_f64, UniOpVVV::kCmpUnordF64)
  DEFINE_OP_3V(v_hadd_f64, UniOpVVV::kHAddF64);
  DEFINE_OP_3V(v_combine_lo_hi_u64, UniOpVVV::kCombineLoHiU64)
  DEFINE_OP_3V(v_combine_lo_hi_f64, UniOpVVV::kCombineLoHiF64)
  DEFINE_OP_3V(v_combine_hi_lo_u64, UniOpVVV::kCombineHiLoU64)
  DEFINE_OP_3V(v_combine_hi_lo_f64, UniOpVVV::kCombineHiLoF64)
  DEFINE_OP_3V(v_interleave_lo_u8, UniOpVVV::kInterleaveLoU8)
  DEFINE_OP_3V(v_interleave_hi_u8, UniOpVVV::kInterleaveHiU8)
  DEFINE_OP_3V(v_interleave_lo_u16, UniOpVVV::kInterleaveLoU16)
  DEFINE_OP_3V(v_interleave_hi_u16, UniOpVVV::kInterleaveHiU16)
  DEFINE_OP_3V(v_interleave_lo_u32, UniOpVVV::kInterleaveLoU32)
  DEFINE_OP_3V(v_interleave_hi_u32, UniOpVVV::kInterleaveHiU32)
  DEFINE_OP_3V(v_interleave_lo_u64, UniOpVVV::kInterleaveLoU64)
  DEFINE_OP_3V(v_interleave_hi_u64, UniOpVVV::kInterleaveHiU64)
  DEFINE_OP_3V(v_interleave_lo_f32, UniOpVVV::kInterleaveLoF32)
  DEFINE_OP_3V(v_interleave_hi_f32, UniOpVVV::kInterleaveHiF32)
  DEFINE_OP_3V(v_interleave_lo_f64, UniOpVVV::kInterleaveLoF64)
  DEFINE_OP_3V(v_interleave_hi_f64, UniOpVVV::kInterleaveHiF64)
  DEFINE_OP_3V(v_packs_i16_i8, UniOpVVV::kPacksI16_I8)
  DEFINE_OP_3V(v_packs_i16_u8, UniOpVVV::kPacksI16_U8)
  DEFINE_OP_3V(v_packs_i32_i16, UniOpVVV::kPacksI32_I16)
  DEFINE_OP_3V(v_packs_i32_u16, UniOpVVV::kPacksI32_U16)
  DEFINE_OP_3V(v_swizzlev_u8, UniOpVVV::kSwizzlev_U8)

#if defined(ASMJIT_UJIT_AARCH64)
  DEFINE_OP_3V(v_mulw_lo_i8, UniOpVVV::kMulwLoI8)
  DEFINE_OP_3V(v_mulw_lo_u8, UniOpVVV::kMulwLoU8)
  DEFINE_OP_3V(v_mulw_hi_i8, UniOpVVV::kMulwHiI8)
  DEFINE_OP_3V(v_mulw_hi_u8, UniOpVVV::kMulwHiU8)
  DEFINE_OP_3V(v_mulw_lo_i16, UniOpVVV::kMulwLoI16)
  DEFINE_OP_3V(v_mulw_lo_u16, UniOpVVV::kMulwLoU16)
  DEFINE_OP_3V(v_mulw_hi_i16, UniOpVVV::kMulwHiI16)
  DEFINE_OP_3V(v_mulw_hi_u16, UniOpVVV::kMulwHiU16)
  DEFINE_OP_3V(v_mulw_lo_i32, UniOpVVV::kMulwLoI32)
  DEFINE_OP_3V(v_mulw_lo_u32, UniOpVVV::kMulwLoU32)
  DEFINE_OP_3V(v_mulw_hi_i32, UniOpVVV::kMulwHiI32)
  DEFINE_OP_3V(v_mulw_hi_u32, UniOpVVV::kMulwHiU32)
  DEFINE_OP_3V(v_maddw_lo_i8, UniOpVVV::kMAddwLoI8)
  DEFINE_OP_3V(v_maddw_lo_u8, UniOpVVV::kMAddwLoU8)
  DEFINE_OP_3V(v_maddw_hi_i8, UniOpVVV::kMAddwHiI8)
  DEFINE_OP_3V(v_maddw_hi_u8, UniOpVVV::kMAddwHiU8)
  DEFINE_OP_3V(v_maddw_lo_i16, UniOpVVV::kMAddwLoI16)
  DEFINE_OP_3V(v_maddw_lo_u16, UniOpVVV::kMAddwLoU16)
  DEFINE_OP_3V(v_maddw_hi_i16, UniOpVVV::kMAddwHiI16)
  DEFINE_OP_3V(v_maddw_hi_u16, UniOpVVV::kMAddwHiU16)
  DEFINE_OP_3V(v_maddw_lo_i32, UniOpVVV::kMAddwLoI32)
  DEFINE_OP_3V(v_maddw_lo_u32, UniOpVVV::kMAddwLoU32)
  DEFINE_OP_3V(v_maddw_hi_i32, UniOpVVV::kMAddwHiI32)
  DEFINE_OP_3V(v_maddw_hi_u32, UniOpVVV::kMAddwHiU32)
#endif // ASMJIT_UJIT_AARCH64

#if defined(ASMJIT_UJIT_X86)
  DEFINE_OP_3V(v_permute_u8, UniOpVVV::kPermuteU8)
  DEFINE_OP_3V(v_permute_u16, UniOpVVV::kPermuteU16)
  DEFINE_OP_3V(v_permute_u32, UniOpVVV::kPermuteU32)
  DEFINE_OP_3V(v_permute_u64, UniOpVVV::kPermuteU64)
#endif // ASMJIT_UJIT_X86

  DEFINE_OP_3VI(v_alignr_u128, UniOpVVVI::kAlignr_U128)
  DEFINE_OP_3VI_WRAP(v_interleave_shuffle_u32x4, Swizzle4, UniOpVVVI::kInterleaveShuffleU32x4)
  DEFINE_OP_3VI_WRAP(v_interleave_shuffle_u64x2, Swizzle2, UniOpVVVI::kInterleaveShuffleU64x2)
  DEFINE_OP_3VI_WRAP(v_interleave_shuffle_f32x4, Swizzle4, UniOpVVVI::kInterleaveShuffleF32x4)
  DEFINE_OP_3VI_WRAP(v_interleave_shuffle_f64x2, Swizzle2, UniOpVVVI::kInterleaveShuffleF64x2)
  DEFINE_OP_3VI(v_insert_v128, UniOpVVVI::kInsertV128_U32)
  DEFINE_OP_3VI(v_insert_v128_u32, UniOpVVVI::kInsertV128_U32)
  DEFINE_OP_3VI(v_insert_v128_f32, UniOpVVVI::kInsertV128_F32)
  DEFINE_OP_3VI(v_insert_v128_u64, UniOpVVVI::kInsertV128_U64)
  DEFINE_OP_3VI(v_insert_v128_f64, UniOpVVVI::kInsertV128_F64)
  DEFINE_OP_3VI(v_insert_v256, UniOpVVVI::kInsertV256_U32)
  DEFINE_OP_3VI(v_insert_v256_u32, UniOpVVVI::kInsertV256_U32)
  DEFINE_OP_3VI(v_insert_v256_f32, UniOpVVVI::kInsertV256_F32)
  DEFINE_OP_3VI(v_insert_v256_u64, UniOpVVVI::kInsertV256_U64)
  DEFINE_OP_3VI(v_insert_v256_f64, UniOpVVVI::kInsertV256_F64)

  DEFINE_OP_4V(v_blendv_u8, UniOpVVVV::kBlendV_U8)
  DEFINE_OP_4V(v_madd_i16, UniOpVVVV::kMAddU16)
  DEFINE_OP_4V(v_madd_u16, UniOpVVVV::kMAddU16)
  DEFINE_OP_4V(v_madd_i32, UniOpVVVV::kMAddU32)
  DEFINE_OP_4V(v_madd_u32, UniOpVVVV::kMAddU32)
  DEFINE_OP_4V(s_madd_f32, UniOpVVVV::kMAddF32S)
  DEFINE_OP_4V(s_madd_f64, UniOpVVVV::kMAddF64S)
  DEFINE_OP_4V(v_madd_f32, UniOpVVVV::kMAddF32)
  DEFINE_OP_4V(v_madd_f64, UniOpVVVV::kMAddF64)
  DEFINE_OP_4V(s_msub_f32, UniOpVVVV::kMSubF32S)
  DEFINE_OP_4V(s_msub_f64, UniOpVVVV::kMSubF64S)
  DEFINE_OP_4V(v_msub_f32, UniOpVVVV::kMSubF32)
  DEFINE_OP_4V(v_msub_f64, UniOpVVVV::kMSubF64)
  DEFINE_OP_4V(s_nmadd_f32, UniOpVVVV::kNMAddF32S)
  DEFINE_OP_4V(s_nmadd_f64, UniOpVVVV::kNMAddF64S)
  DEFINE_OP_4V(v_nmadd_f32, UniOpVVVV::kNMAddF32)
  DEFINE_OP_4V(v_nmadd_f64, UniOpVVVV::kNMAddF64)
  DEFINE_OP_4V(s_nmsub_f32, UniOpVVVV::kNMSubF32S)
  DEFINE_OP_4V(s_nmsub_f64, UniOpVVVV::kNMSubF64S)
  DEFINE_OP_4V(v_nmsub_f32, UniOpVVVV::kNMSubF32)
  DEFINE_OP_4V(v_nmsub_f64, UniOpVVVV::kNMSubF64)

  #undef DEFINE_OP_4V
  #undef DEFINE_OP_3VI_WRAP
  #undef DEFINE_OP_3VI
  #undef DEFINE_OP_3V
  #undef DEFINE_OP_MV_A
  #undef DEFINE_OP_MV_U
  #undef DEFINE_OP_MV_I
  #undef DEFINE_OP_VM_A
  #undef DEFINE_OP_VM_U
  #undef DEFINE_OP_VM_I
  #undef DEFINE_OP_2VI_WRAP
  #undef DEFINE_OP_2VI
  #undef DEFINE_OP_2V

  template<typename DstT, typename SrcT> ASMJIT_INLINE void v_swap_u32(const DstT& dst, const SrcT& src) { v_swizzle_u32x4(dst, src, swizzle(2, 3, 0, 1)); }
  template<typename DstT, typename SrcT> ASMJIT_INLINE void v_swap_u64(const DstT& dst, const SrcT& src) { v_swizzle_u64x2(dst, src, swizzle(0, 1)); }
  template<typename DstT, typename SrcT> ASMJIT_INLINE void v_swap_f32(const DstT& dst, const SrcT& src) { v_swizzle_f32x4(dst, src, swizzle(2, 3, 0, 1)); }
  template<typename DstT, typename SrcT> ASMJIT_INLINE void v_swap_f64(const DstT& dst, const SrcT& src) { v_swizzle_f64x2(dst, src, swizzle(0, 1)); }

  template<typename DstT, typename SrcT> ASMJIT_INLINE void v_dup_lo_u32(const DstT& dst, const SrcT& src) { v_swizzle_u32x4(dst, src, swizzle(2, 2, 0, 0)); }
  template<typename DstT, typename SrcT> ASMJIT_INLINE void v_dup_hi_u32(const DstT& dst, const SrcT& src) { v_swizzle_u32x4(dst, src, swizzle(3, 3, 1, 1)); }
  template<typename DstT, typename SrcT> ASMJIT_INLINE void v_dup_lo_u64(const DstT& dst, const SrcT& src) { v_swizzle_u64x2(dst, src, swizzle(0, 0)); }
  template<typename DstT, typename SrcT> ASMJIT_INLINE void v_dup_hi_u64(const DstT& dst, const SrcT& src) { v_swizzle_u64x2(dst, src, swizzle(1, 1)); }
  template<typename DstT, typename SrcT> ASMJIT_INLINE void v_dup_lo_f64(const DstT& dst, const SrcT& src) { v_swizzle_f64x2(dst, src, swizzle(0, 0)); }
  template<typename DstT, typename SrcT> ASMJIT_INLINE void v_dup_hi_f64(const DstT& dst, const SrcT& src) { v_swizzle_f64x2(dst, src, swizzle(1, 1)); }

  template<typename T> ASMJIT_INLINE void v_zero_i(const T& dst) { v_xor_i32(dst, dst, dst); }
  template<typename T> ASMJIT_INLINE void v_zero_f(const T& dst) { v_xor_f32(dst, dst, dst); }
  template<typename T> ASMJIT_INLINE void v_zero_d(const T& dst) { v_xor_f64(dst, dst, dst); }
  template<typename T> ASMJIT_INLINE void v_ones_i(const T& dst) { v_cmp_eq_u8(dst, dst, dst); }

  //! \}

  //! \name Memory Loads & Stores
  //! \{

  ASMJIT_NOINLINE void v_load_u8_u16_2x(const Vec& dst, const Mem& lo, const Mem& hi) {
#if defined(ASMJIT_UJIT_X86)
    Gp reg = new_gp32("@tmp");
    Mem m_lo(lo);
    Mem m_hi(hi);

    m_lo.set_size(1);
    m_hi.set_size(1);

    load_u8(reg, m_hi);
    shl(reg, reg, 16);
    cc->mov(reg.r8(), m_lo);
    s_mov_u32(dst.xmm(), reg);
#elif defined(ASMJIT_UJIT_AARCH64)
    Gp tmp_a = new_gp32("@tmp_a");
    Gp tmp_b = new_gp32("@tmp_b");

    load_u8(tmp_a, lo);
    load_u8(tmp_b, hi);
    cc->orr(tmp_a, tmp_a, tmp_b, a64::lsl(16));
    s_mov_u32(dst, tmp_a);
#endif
  }

  //! \}

  //! \name Memory Loads & Stores with Parameterized Size
  //! \{

  ASMJIT_NOINLINE void v_load_iany(const Vec& dst, const Mem& src, size_t n_bytes, Alignment alignment) {
    switch (n_bytes) {
      case 1: v_load8(dst, src); break;
      case 2: v_loada16(dst, src, alignment); break;
      case 4: v_loada32(dst, src, alignment); break;
      case 8: v_loada64(dst, src, alignment); break;
      case 16: v_loada128(dst, src, alignment); break;
      case 32: v_loada256(dst, src, alignment); break;
      case 64: v_loada512(dst, src, alignment); break;

      default:
        ASMJIT_NOT_REACHED();
    }
  }

  ASMJIT_NOINLINE void v_store_iany(const Mem& dst, const Vec& src, size_t n_bytes, Alignment alignment) {
    switch (n_bytes) {
      case 1: v_store8(dst, src); break;
      case 2: v_storea16(dst, src, alignment); break;
      case 4: v_storea32(dst, src, alignment); break;
      case 8: v_storea64(dst, src, alignment); break;
      case 16: v_storea128(dst, src, alignment); break;
      case 32: v_storea256(dst, src, alignment); break;
      case 64: v_storea512(dst, src, alignment); break;

      default:
        ASMJIT_NOT_REACHED();
    }
  }

  //! \}

  //! \name Utilities
  //! \{

  template<typename Dst, typename Src>
  ASMJIT_INLINE void shift_or_rotate_left(const Dst& dst, const Src& src, uint32_t n) {
  #if defined(ASMJIT_UJIT_X86)
    if ((n & 3) == 0)
      v_alignr_u128(dst, src, src, (16u - n) & 15);
    else
      v_sllb_u128(dst, src, n);
  #else
    // This doesn't rely on a zero constant on AArch64, which is okay as we don't care what's shifted in.
    v_alignr_u128(dst, src, src, (16u - n) & 15);
  #endif
  }

  template<typename Dst, typename Src>
  ASMJIT_INLINE void shift_or_rotate_right(const Dst& dst, const Src& src, uint32_t n) {
  #if defined(ASMJIT_UJIT_X86)
    if ((n & 3) == 0)
      v_alignr_u128(dst, src, src, n);
    else
      v_srlb_u128(dst, src, n);
  #else
    // This doesn't rely on a zero constant on AArch64, which is okay as we don't care what's shifted in.
    v_alignr_u128(dst, src, src, n);
  #endif
  }

  //! \}
};

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_UJIT
#endif // ASMJIT_UJIT_UNICOMPILER_H_INCLUDED
