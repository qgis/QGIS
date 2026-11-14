// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_CPUINFO_H_INCLUDED
#define ASMJIT_CORE_CPUINFO_H_INCLUDED

#include <asmjit/core/archtraits.h>
#include <asmjit/core/environment.h>
#include <asmjit/core/globals.h>
#include <asmjit/core/string.h>
#include <asmjit/support/support.h>

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core
//! \{

//! CPU features information.
//!
//! Each feature is represented by a single bit in an embedded bit array.
class CpuFeatures {
public:
  //! \name Constants
  //! \{

  //! \cond INTERNAL
  static inline constexpr uint32_t kMaxFeatures = 256;
  static inline constexpr uint32_t kNumBitWords = kMaxFeatures / Support::bit_size_of<Support::BitWord>;
  //! \endcond

  //! \}

  //! \name Types
  //! \{

  //! A word that is used to represents feature bits.
  using BitWord = Support::BitWord;
  //! Iterator that can iterate all CPU features set.
  using Iterator = Support::BitVectorIterator<BitWord>;

  using Bits = Support::Array<BitWord, kNumBitWords>;

  //! \}

  //! \name Data
  //! \{

  //! CPU features data.
  struct Data {
    //! \name Members
    //! \{

    //! Data bits.
    Bits _bits;

    //! \}

    //! \name Overloaded Operators
    //! \{

    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG bool operator==(const Data& other) const noexcept { return  equals(other); }

    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG bool operator!=(const Data& other) const noexcept { return !equals(other); }

    //! \}

    //! \name Accessors
    //! \{

    //! Returns true if there are no features set.
    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG bool is_empty() const noexcept { return _bits.aggregate<Support::Or>(0) == 0; }

    //! Returns all features as array of bitwords (see \ref Support::BitWord).
    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG BitWord* bits() noexcept { return _bits.data(); }

    //! Returns all features as array of bitwords (const).
    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG const BitWord* bits() const noexcept { return _bits.data(); }

    //! Returns the number of BitWords returned by \ref bits().
    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG size_t bit_word_count() const noexcept { return kNumBitWords; }

    //! Returns \ref Support::BitVectorIterator, that can be used to iterate over all features efficiently.
    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG Iterator iterator() const noexcept { return Iterator(_bits.as_span()); }

    //! Tests whether the feature `feature_id` is present.
    template<typename FeatureId>
    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG bool has(const FeatureId& feature_id) const noexcept {
      ASMJIT_ASSERT(uint32_t(feature_id) < kMaxFeatures);

      uint32_t idx = uint32_t(feature_id) / Support::bit_size_of<BitWord>;
      uint32_t bit = uint32_t(feature_id) % Support::bit_size_of<BitWord>;

      return bool((_bits[idx] >> bit) & 0x1);
    }

    //! \cond NONE
    template<typename FeatureId>
    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG bool has_any(const FeatureId& feature_id) const noexcept {
      return has(feature_id);
    }
    //! \endcond

    //! Tests whether any feature given is present.
    //!
    //! \note This is a variadic function template that can be used with multiple features.
    template<typename FeatureId, typename... Args>
    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG bool has_any(const FeatureId& feature_id, Args&&... other_feature_ids) const noexcept {
      return bool(unsigned(has(feature_id)) | unsigned(has_any(std::forward<Args>(other_feature_ids)...)));
    }

    //! Tests whether all features as defined by `other` are present.
    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG bool has_all(const Data& other) const noexcept {
      uint32_t result = 1;
      for (uint32_t i = 0; i < kNumBitWords; i++)
        result &= uint32_t((_bits[i] & other._bits[i]) == other._bits[i]);
      return bool(result);
    }

    //! \}

    //! \name Manipulation
    //! \{

    //! Clears all features set.
    ASMJIT_INLINE_NODEBUG void reset() noexcept { _bits.fill(0); }

    //! Adds the given CPU `feature_id` to the list of features.
    template<typename FeatureId>
    inline void add(const FeatureId& feature_id) noexcept {
      ASMJIT_ASSERT(uint32_t(feature_id) < kMaxFeatures);

      uint32_t idx = uint32_t(feature_id) / Support::bit_size_of<BitWord>;
      uint32_t bit = uint32_t(feature_id) % Support::bit_size_of<BitWord>;

      _bits[idx] |= BitWord(1) << bit;
    }

    template<typename FeatureId, typename... Args>
    inline void add(const FeatureId& feature_id, Args&&... other_feature_ids) noexcept {
      add(feature_id);
      add(std::forward<Args>(other_feature_ids)...);
    }

    template<typename FeatureId>
    inline void add_if(bool condition, const FeatureId& feature_id) noexcept {
      ASMJIT_ASSERT(uint32_t(feature_id) < kMaxFeatures);

      uint32_t idx = uint32_t(feature_id) / Support::bit_size_of<BitWord>;
      uint32_t bit = uint32_t(feature_id) % Support::bit_size_of<BitWord>;

      _bits[idx] |= BitWord(condition) << bit;
    }

    template<typename FeatureId, typename... Args>
    inline void add_if(bool condition, const FeatureId& feature_id, Args&&... other_feature_ids) noexcept {
      add_if(condition, feature_id);
      add_if(condition, std::forward<Args>(other_feature_ids)...);
    }

    //! Removes the given CPU `feature_id` from the list of features.
    template<typename FeatureId>
    inline void remove(const FeatureId& feature_id) noexcept {
      ASMJIT_ASSERT(uint32_t(feature_id) < kMaxFeatures);

      uint32_t idx = uint32_t(feature_id) / Support::bit_size_of<BitWord>;
      uint32_t bit = uint32_t(feature_id) % Support::bit_size_of<BitWord>;

      _bits[idx] &= ~(BitWord(1) << bit);
    }

    template<typename FeatureId, typename... Args>
    inline void remove(const FeatureId& feature_id, Args&&... other_feature_ids) noexcept {
      remove(feature_id);
      remove(std::forward<Args>(other_feature_ids)...);
    }

    //! Tests whether this CPU features data matches `other`.
    ASMJIT_INLINE_NODEBUG bool equals(const Data& other) const noexcept { return _bits == other._bits; }

    //! \}
  };

  //! X86 specific features data.
  struct X86 : public Data {
    //! X86 CPU feature identifiers.
    enum Id : uint8_t {
      // @EnumValuesBegin{"enum": "CpuFeatures::X86"}@
      kNone,                     //!< No feature (never set, used internally).

      kMT,                       //!< CPU has multi-threading capabilities.
      kNX,                       //!< CPU has Not-Execute-Bit aka DEP (data-execution prevention).
      kADX,                      //!< CPU has ADX              (multi-precision add-carry instruction extensions).
      kALTMOVCR8,                //!< CPU has LOCK MOV R<->CR0 (supports `MOV R<->CR8` via `LOCK MOV R<->CR0` in 32-bit mode) {AMD}.
      kAPX_F,                    //!< CPU has APX_F            (advanced performance extensions - 32 GP registers, REX2 prefix, ...) {X86_64}.
      kBMI,                      //!< CPU has BMI              (bit manipulation instructions #1).
      kBMI2,                     //!< CPU has BMI2             (bit manipulation instructions #2).
      kCET_IBT,                  //!< CPU has CET-IBT          (indirect branch tracking).
      kCET_SS,                   //!< CPU has CET-SS.
      kCET_SSS,                  //!< CPU has CET-SSS.
      kCLDEMOTE,                 //!< CPU has CLDEMOTE         (cache line demote).
      kCLFLUSH,                  //!< CPU has CLFUSH           (cache Line flush).
      kCLFLUSHOPT,               //!< CPU has CLFUSHOPT        (cache Line flush - optimized).
      kCLWB,                     //!< CPU has CLWB.
      kCLZERO,                   //!< CPU has CLZERO.
      kCMOV,                     //!< CPU has CMOV             (CMOV and FCMOV instructions).
      kCMPCCXADD,                //!< CPU has CMPCCXADD.
      kCMPXCHG16B,               //!< CPU has CMPXCHG16B       (compare-exchange 16 bytes) {X86_64}.
      kCMPXCHG8B,                //!< CPU has CMPXCHG8B        (compare-exchange 8 bytes).
      kENCLV,                    //!< CPU has ENCLV.
      kENQCMD,                   //!< CPU has ENQCMD           (enqueue stores).
      kERMS,                     //!< CPU has ERMS             (enhanced REP MOVSB/STOSB).
      kFSGSBASE,                 //!< CPU has FSGSBASE.
      kFSRM,                     //!< CPU has FSRM             (fast short REP MOVSB).
      kFSRC,                     //!< CPU has FSRC             (fast short REP CMPSB|SCASB).
      kFSRS,                     //!< CPU has FSRS             (fast short REP STOSB)
      kFXSR,                     //!< CPU has FXSR             (FXSAVE/FXRSTOR instructions).
      kFXSROPT,                  //!< CPU has FXSROTP          (FXSAVE/FXRSTOR is optimized).
      kFZRM,                     //!< CPU has FZRM             (fast zero-length REP MOVSB).
      kHRESET,                   //!< CPU has HRESET.
      kI486,                     //!< CPU has I486 features    (I486+ support).
      kINVLPGB,                  //!< CPU has INVLPGB.
      kLAHFSAHF,                 //!< CPU has LAHF/SAHF        (LAHF/SAHF in 64-bit mode) {X86_64}.
      kLAM,                      //!< CPU has LAM              (linear address masking) {X86_64}.
      kLWP,                      //!< CPU has LWP              (lightweight profiling) {AMD}.
      kLZCNT,                    //!< CPU has LZCNT            (LZCNT instruction).
      kMCOMMIT,                  //!< CPU has MCOMMIT          (MCOMMIT instruction).
      kMONITOR,                  //!< CPU has MONITOR          (MONITOR/MWAIT instructions).
      kMONITORX,                 //!< CPU has MONITORX         (MONITORX/MWAITX instructions).
      kMOVBE,                    //!< CPU has MOVBE            (move with byte-order swap).
      kMOVDIR64B,                //!< CPU has MOVDIR64B        (move 64 bytes as direct store).
      kMOVDIRI,                  //!< CPU has MOVDIRI          (move dword/qword as direct store).
      kMOVRS,                    //!< CPU has MOVRS            (move from shared memory).
      kMPX,                      //!< CPU has MPX              (memory protection extensions).
      kMSR,                      //!< CPU has MSR              (RDMSR/WRMSR instructions).
      kMSRLIST,                  //!< CPU has MSRLIST.
      kMSR_IMM,                  //!< CPU has MSR_IMM          (RDMSR/WRMSR immediate encoding).
      kMSSE,                     //!< CPU has MSSE             (misaligned SSE support).
      kOSXSAVE,                  //!< CPU has OSXSAVE          (XSAVE enabled by OS).
      kOSPKE,                    //!< CPU has OSPKE            (PKE enabled by OS).
      kPCONFIG,                  //!< CPU has PCONFIG          (PCONFIG instruction).
      kPOPCNT,                   //!< CPU has POPCNT           (POPCNT instruction).
      kPREFETCHI,                //!< CPU has PREFETCHI.
      kPREFETCHW,                //!< CPU has PREFETCHW.
      kPREFETCHWT1,              //!< CPU has PREFETCHWT1.
      kPTWRITE,                  //!< CPU has PTWRITE.
      kRAO_INT,                  //!< CPU has RAO_INT          (AADD, AAND, AOR, AXOR instructions).
      kRMPQUERY,                 //!< CPU has RMPQUERY         (RMPQUERY instruction).
      kRDPID,                    //!< CPU has RDPID            (RDPID instruction).
      kRDPRU,                    //!< CPU has RDPRU            (RDPRU instruction).
      kRDRAND,                   //!< CPU has RDRAND           (RDRAND instruction).
      kRDSEED,                   //!< CPU has RDSEED           (RDSEED instruction).
      kRDTSC,                    //!< CPU has RDTSC.
      kRDTSCP,                   //!< CPU has RDTSCP.
      kRTM,                      //!< CPU has RTM              (RTM instructions - deprecated).
      kSEAM,                     //!< CPU has SEAM.
      kSERIALIZE,                //!< CPU has SERIALIZE.
      kSEV,                      //!< CPU has SEV              (secure encrypted virtualization).
      kSEV_ES,                   //!< CPU has SEV_ES           (SEV encrypted state).
      kSEV_SNP,                  //!< CPU has SEV_SNP          (SEV secure nested paging).
      kSKINIT,                   //!< CPU has SKINIT           (SKINIT/STGI instructions) {AMD}.
      kSMAP,                     //!< CPU has SMAP             (supervisor-mode access prevention).
      kSME,                      //!< CPU has SME              (secure memory encryption).
      kSMEP,                     //!< CPU has SMEP             (supervisor-mode execution prevention).
      kSMX,                      //!< CPU has SMX              (safer mode extensions).
      kSVM,                      //!< CPU has SVM              (virtualization) {AMD}.
      kTBM,                      //!< CPU has TBM              (trailing bit manipulation) {AMD}.
      kTSE,                      //!< CPU has TSE.
      kTSXLDTRK,                 //!< CPU has TSXLDTRK.
      kUINTR,                    //!< CPU has UINTR            (user interrupts).
      kVMX,                      //!< CPU has VMX              (virtualization) {INTEL}.
      kWAITPKG,                  //!< CPU has WAITPKG          (UMONITOR, UMWAIT, TPAUSE).
      kWBNOINVD,                 //!< CPU has WBNOINVD.
      kWRMSRNS,                  //!< CPU has WRMSRNS.
      kXSAVE,                    //!< CPU has XSAVE.
      kXSAVEC,                   //!< CPU has XSAVEC.
      kXSAVEOPT,                 //!< CPU has XSAVEOPT.
      kXSAVES,                   //!< CPU has XSAVES.

      kFPU,                      //!< CPU has FPU              (FPU support).
      kMMX,                      //!< CPU has MMX              (MMX base instructions) (deprecated).
      kMMX2,                     //!< CPU has MMX2             (MMX2 extensions or initial SSE extensions) (deprecated).
      k3DNOW,                    //!< CPU has 3DNOW            (3DNOW base instructions) {AMD} (deprecated).
      k3DNOW2,                   //!< CPU has 3DNOW2           (enhanced 3DNOW) {AMD} (deprecated).
      kGEODE,                    //!< CPU has GEODE extensions (GEODE 3DNOW additions) (deprecated).

      kSSE,                      //!< CPU has SSE              (SSE instructions).
      kSSE2,                     //!< CPU has SSE2             (SSE2 instructions).
      kSSE3,                     //!< CPU has SSE3             (SSE3 instructions).
      kSSSE3,                    //!< CPU has SSSE3            (SSSE3 instructions).
      kSSE4_1,                   //!< CPU has SSE4.1           (SSE4.1 instructions).
      kSSE4_2,                   //!< CPU has SSE4.2           (SSE4.2 instructions).
      kSSE4A,                    //!< CPU has SSE4A            (SSE4.A instructions) {AMD} (deprecated).
      kPCLMULQDQ,                //!< CPU has PCLMULQDQ        (packed carry-less multiplication).

      kAVX,                      //!< CPU has AVX              (advanced vector extensions).
      kAVX2,                     //!< CPU has AVX2             (advanced vector extensions 2).
      kAVX_IFMA,                 //!< CPU has AVX_IFMA         (AVX/VEX encoding of vpmadd52huq/vpmadd52luq).
      kAVX_NE_CONVERT,           //!< CPU has AVX_NE_CONVERT.
      kAVX_VNNI,                 //!< CPU has AVX_VNNI         (AVX/VEX encoding of vpdpbusd/vpdpbusds/vpdpwssd/vpdpwssds).
      kAVX_VNNI_INT16,           //!< CPU has AVX_VNNI_INT16.
      kAVX_VNNI_INT8,            //!< CPU has AVX_VNNI_INT8.
      kF16C,                     //!< CPU has F16C             (AVX FP16 conversion instructions).
      kFMA,                      //!< CPU has FMA              (AVX fused-multiply-add - 3 operand form).
      kFMA4,                     //!< CPU has FMA4             (AVX fused-multiply-add - 4 operand form) (deprecated).
      kXOP,                      //!< CPU has XOP              (XOP instructions) {AMD} (deprecated).

      kAVX512_BF16,              //!< CPU has AVX512_BF16      (AVX512 BFLOAT16 support instructions).
      kAVX512_BITALG,            //!< CPU has AVX512_BITALG    (AVX512 VPOPCNT[B|W] and VPSHUFBITQMB instructions).
      kAVX512_BW,                //!< CPU has AVX512_BW        (AVX512 integer BYTE|WORD instructions).
      kAVX512_CD,                //!< CPU has AVX512_CD        (AVX512 conflict detection DWORD|QWORD instructions).
      kAVX512_DQ,                //!< CPU has AVX512_DQ        (AVX512 integer DWORD|QWORD instructions).
      kAVX512_F,                 //!< CPU has AVX512_F         (AVX512 foundation).
      kAVX512_FP16,              //!< CPU has AVX512_FP16      (AVX512 FP16 instructions).
      kAVX512_IFMA,              //!< CPU has AVX512_IFMA      (AVX512 integer fused-multiply-add using 52-bit precision).
      kAVX512_VBMI,              //!< CPU has AVX512_VBMI      (AVX512 vector byte manipulation instructions).
      kAVX512_VBMI2,             //!< CPU has AVX512_VBMI2     (AVX512 vector byte manipulation instructions v2).
      kAVX512_VL,                //!< CPU has AVX512_VL        (AVX512 vector length extensions).
      kAVX512_VNNI,              //!< CPU has AVX512_VNNI      (AVX512 vector neural network instructions).
      kAVX512_VP2INTERSECT,      //!< CPU has AVX512_VP2INTERSECT
      kAVX512_VPOPCNTDQ,         //!< CPU has AVX512_VPOPCNTDQ (AVX512 VPOPCNT[D|Q] instructions).

      kAESNI,                    //!< CPU has AESNI            (AES encode/decode instructions).
      kGFNI,                     //!< CPU has GFNI             (galois field instructions).
      kSHA,                      //!< CPU has SHA              (SHA-1 and SHA-256 instructions).
      kSHA512,                   //!< CPU has SHA512           (SHA-512 instructions).
      kSM3,                      //!< CPU has SM3              (SM3 hash extensions).
      kSM4,                      //!< CPU has SM4              (SM4 cipher extensions).
      kVAES,                     //!< CPU has VAES             (vector AES 256|512 bit support).
      kVPCLMULQDQ,               //!< CPU has VPCLMULQDQ       (vector PCLMULQDQ 256|512-bit support).

      kKL,                       //!< CPU has KL               (Key Locker).
      kAESKLE,                   //!< CPU has AESKLE           (AESKLE).
      kAESKLEWIDE_KL,            //!< CPU has AESKLE+WIDEKL+KL (AESKLE & WIDEKL instructions and KL enabled)

      kAVX10_1,                  //!< CPU has AVX10.1/512      (AVX10.1 with 512-bit vectors).
      kAVX10_2,                  //!< CPU has AVX10.2/512      (AVX10.2 with 512-bit vectors).

      kAMX_AVX512,               //!< CPU has AMX_AVX512       (AMX-AVX512 instructions).
      kAMX_BF16,                 //!< CPU has AMX_BF16         (AMX-BF16 instructions).
      kAMX_COMPLEX,              //!< CPU has AMX_COMPLEX      (AMX-COMPLEX instructions).
      kAMX_FP16,                 //!< CPU has AMX_FP16         (AMX-FP16 instructions).
      kAMX_FP8,                  //!< CPU has AMX_FP8          (AMX-FP8 instructions).
      kAMX_INT8,                 //!< CPU has AMX_INT8         (AMX-INT8 instructions).
      kAMX_MOVRS,                //!< CPU has AMX_MOVRS        (AMX-MOVRS instructions).
      kAMX_TF32,                 //!< CPU has AMX_TF32         (AMX-TF32 instructions).
      kAMX_TILE,                 //!< CPU has AMX_TILE         (advanced matrix extensions).
      kAMX_TRANSPOSE,            //!< CPU has AMX_TRANSPOSE    (AMX-TRANSPOSE instructions).
      // @EnumValuesEnd@

      kMaxValue = kAMX_TILE
    };

    #define ASMJIT_X86_FEATURE(accessor, feature) \
      /*! Tests whether feature is present. */ \
      ASMJIT_INLINE_NODEBUG bool accessor() const noexcept { return has(X86::feature); }

    ASMJIT_X86_FEATURE(has_mt, kMT)
    ASMJIT_X86_FEATURE(has_nx, kNX)
    ASMJIT_X86_FEATURE(has_adx, kADX)
    ASMJIT_X86_FEATURE(has_altmovcr8, kALTMOVCR8)
    ASMJIT_X86_FEATURE(has_apx_f, kAPX_F)
    ASMJIT_X86_FEATURE(has_bmi, kBMI)
    ASMJIT_X86_FEATURE(has_bmi2, kBMI2)
    ASMJIT_X86_FEATURE(has_cet_ibt, kCET_IBT)
    ASMJIT_X86_FEATURE(has_cet_ss, kCET_SS)
    ASMJIT_X86_FEATURE(has_cet_sss, kCET_SSS)
    ASMJIT_X86_FEATURE(has_cldemote, kCLDEMOTE)
    ASMJIT_X86_FEATURE(has_clflush, kCLFLUSH)
    ASMJIT_X86_FEATURE(has_clflushopt, kCLFLUSHOPT)
    ASMJIT_X86_FEATURE(has_clwb, kCLWB)
    ASMJIT_X86_FEATURE(has_clzero, kCLZERO)
    ASMJIT_X86_FEATURE(has_cmov, kCMOV)
    ASMJIT_X86_FEATURE(has_cmpxchg16b, kCMPXCHG16B)
    ASMJIT_X86_FEATURE(has_cmpxchg8b, kCMPXCHG8B)
    ASMJIT_X86_FEATURE(has_enclv, kENCLV)
    ASMJIT_X86_FEATURE(has_enqcmd, kENQCMD)
    ASMJIT_X86_FEATURE(has_erms, kERMS)
    ASMJIT_X86_FEATURE(has_fsgsbase, kFSGSBASE)
    ASMJIT_X86_FEATURE(has_fsrm, kFSRM)
    ASMJIT_X86_FEATURE(has_fsrc, kFSRC)
    ASMJIT_X86_FEATURE(has_fsrs, kFSRS)
    ASMJIT_X86_FEATURE(has_fxsr, kFXSR)
    ASMJIT_X86_FEATURE(has_fxsropt, kFXSROPT)
    ASMJIT_X86_FEATURE(has_fzrm, kFZRM)
    ASMJIT_X86_FEATURE(has_hreset, kHRESET)
    ASMJIT_X86_FEATURE(has_i486, kI486)
    ASMJIT_X86_FEATURE(has_invlpgb, kINVLPGB)
    ASMJIT_X86_FEATURE(has_lahfsahf, kLAHFSAHF)
    ASMJIT_X86_FEATURE(has_lam, kLAM)
    ASMJIT_X86_FEATURE(has_lwp, kLWP)
    ASMJIT_X86_FEATURE(has_lzcnt, kLZCNT)
    ASMJIT_X86_FEATURE(has_mcommit, kMCOMMIT)
    ASMJIT_X86_FEATURE(has_monitor, kMONITOR)
    ASMJIT_X86_FEATURE(has_monitorx, kMONITORX)
    ASMJIT_X86_FEATURE(has_movbe, kMOVBE)
    ASMJIT_X86_FEATURE(has_movdir64b, kMOVDIR64B)
    ASMJIT_X86_FEATURE(has_movdiri, kMOVDIRI)
    ASMJIT_X86_FEATURE(has_movrs, kMOVRS)
    ASMJIT_X86_FEATURE(has_mpx, kMPX)
    ASMJIT_X86_FEATURE(has_msr, kMSR)
    ASMJIT_X86_FEATURE(has_msrlist, kMSRLIST)
    ASMJIT_X86_FEATURE(has_msr_imm, kMSR_IMM)
    ASMJIT_X86_FEATURE(has_msse, kMSSE)
    ASMJIT_X86_FEATURE(has_osxsave, kOSXSAVE)
    ASMJIT_X86_FEATURE(has_ospke, kOSPKE)
    ASMJIT_X86_FEATURE(has_pconfig, kPCONFIG)
    ASMJIT_X86_FEATURE(has_popcnt, kPOPCNT)
    ASMJIT_X86_FEATURE(has_prefetchi, kPREFETCHI)
    ASMJIT_X86_FEATURE(has_prefetchw, kPREFETCHW)
    ASMJIT_X86_FEATURE(has_prefetchwt1, kPREFETCHWT1)
    ASMJIT_X86_FEATURE(has_ptwrite, kPTWRITE)
    ASMJIT_X86_FEATURE(has_rao_int, kRAO_INT)
    ASMJIT_X86_FEATURE(has_rmpquery, kRMPQUERY)
    ASMJIT_X86_FEATURE(has_rdpid, kRDPID)
    ASMJIT_X86_FEATURE(has_rdpru, kRDPRU)
    ASMJIT_X86_FEATURE(has_rdrand, kRDRAND)
    ASMJIT_X86_FEATURE(has_rdseed, kRDSEED)
    ASMJIT_X86_FEATURE(has_rdtsc, kRDTSC)
    ASMJIT_X86_FEATURE(has_rdtscp, kRDTSCP)
    ASMJIT_X86_FEATURE(has_rtm, kRTM)
    ASMJIT_X86_FEATURE(has_seam, kSEAM)
    ASMJIT_X86_FEATURE(has_serialize, kSERIALIZE)
    ASMJIT_X86_FEATURE(has_sev, kSEV)
    ASMJIT_X86_FEATURE(has_sev_es, kSEV_ES)
    ASMJIT_X86_FEATURE(has_sev_snp, kSEV_SNP)
    ASMJIT_X86_FEATURE(has_skinit, kSKINIT)
    ASMJIT_X86_FEATURE(has_smap, kSMAP)
    ASMJIT_X86_FEATURE(has_smep, kSMEP)
    ASMJIT_X86_FEATURE(has_smx, kSMX)
    ASMJIT_X86_FEATURE(has_svm, kSVM)
    ASMJIT_X86_FEATURE(has_tbm, kTBM)
    ASMJIT_X86_FEATURE(has_tse, kTSE)
    ASMJIT_X86_FEATURE(has_tsxldtrk, kTSXLDTRK)
    ASMJIT_X86_FEATURE(has_uintr, kUINTR)
    ASMJIT_X86_FEATURE(has_vmx, kVMX)
    ASMJIT_X86_FEATURE(has_waitpkg, kWAITPKG)
    ASMJIT_X86_FEATURE(has_wbnoinvd, kWBNOINVD)
    ASMJIT_X86_FEATURE(has_wrmsrns, kWRMSRNS)
    ASMJIT_X86_FEATURE(has_xsave, kXSAVE)
    ASMJIT_X86_FEATURE(has_xsavec, kXSAVEC)
    ASMJIT_X86_FEATURE(has_xsaveopt, kXSAVEOPT)
    ASMJIT_X86_FEATURE(has_xsaves, kXSAVES)

    ASMJIT_X86_FEATURE(has_fpu, kFPU)
    ASMJIT_X86_FEATURE(has_mmx, kMMX)
    ASMJIT_X86_FEATURE(has_mmx2, kMMX2)
    ASMJIT_X86_FEATURE(has_3dnow, k3DNOW)
    ASMJIT_X86_FEATURE(has_3dnow2, k3DNOW2)
    ASMJIT_X86_FEATURE(has_geode, kGEODE)

    ASMJIT_X86_FEATURE(has_sse, kSSE)
    ASMJIT_X86_FEATURE(has_sse2, kSSE2)
    ASMJIT_X86_FEATURE(has_sse3, kSSE3)
    ASMJIT_X86_FEATURE(has_ssse3, kSSSE3)
    ASMJIT_X86_FEATURE(has_sse4_1, kSSE4_1)
    ASMJIT_X86_FEATURE(has_sse4_2, kSSE4_2)
    ASMJIT_X86_FEATURE(has_sse4a, kSSE4A)
    ASMJIT_X86_FEATURE(has_pclmulqdq, kPCLMULQDQ)

    ASMJIT_X86_FEATURE(has_avx, kAVX)
    ASMJIT_X86_FEATURE(has_avx2, kAVX2)
    ASMJIT_X86_FEATURE(has_avx_ifma, kAVX_IFMA)
    ASMJIT_X86_FEATURE(has_avx_ne_convert, kAVX_NE_CONVERT)
    ASMJIT_X86_FEATURE(has_avx_vnni, kAVX_VNNI)
    ASMJIT_X86_FEATURE(has_avx_vnni_int16, kAVX_VNNI_INT16)
    ASMJIT_X86_FEATURE(has_avx_vnni_int8, kAVX_VNNI_INT8)
    ASMJIT_X86_FEATURE(has_f16c, kF16C)
    ASMJIT_X86_FEATURE(has_fma, kFMA)
    ASMJIT_X86_FEATURE(has_fma4, kFMA4)
    ASMJIT_X86_FEATURE(has_xop, kXOP)

    ASMJIT_X86_FEATURE(has_avx512_bf16, kAVX512_BF16)
    ASMJIT_X86_FEATURE(has_avx512_bitalg, kAVX512_BITALG)
    ASMJIT_X86_FEATURE(has_avx512_bw, kAVX512_BW)
    ASMJIT_X86_FEATURE(has_avx512_cd, kAVX512_CD)
    ASMJIT_X86_FEATURE(has_avx512_dq, kAVX512_DQ)
    ASMJIT_X86_FEATURE(has_avx512_f, kAVX512_F)
    ASMJIT_X86_FEATURE(has_avx512_fp16, kAVX512_FP16)
    ASMJIT_X86_FEATURE(has_avx512_ifma, kAVX512_IFMA)
    ASMJIT_X86_FEATURE(has_avx512_vbmi, kAVX512_VBMI)
    ASMJIT_X86_FEATURE(has_avx512_vbmi2, kAVX512_VBMI2)
    ASMJIT_X86_FEATURE(has_avx512_vl, kAVX512_VL)
    ASMJIT_X86_FEATURE(has_avx512_vnni, kAVX512_VNNI)
    ASMJIT_X86_FEATURE(has_avx512_vp2intersect, kAVX512_VP2INTERSECT)
    ASMJIT_X86_FEATURE(has_avx512_vpopcntdq, kAVX512_VPOPCNTDQ)

    ASMJIT_X86_FEATURE(has_aesni, kAESNI)
    ASMJIT_X86_FEATURE(has_gfni, kGFNI)
    ASMJIT_X86_FEATURE(has_sha, kSHA)
    ASMJIT_X86_FEATURE(has_sha512, kSHA512)
    ASMJIT_X86_FEATURE(has_sm3, kSM3)
    ASMJIT_X86_FEATURE(has_sm4, kSM4)
    ASMJIT_X86_FEATURE(has_vaes, kVAES)
    ASMJIT_X86_FEATURE(has_vpclmulqdq, kVPCLMULQDQ)

    ASMJIT_X86_FEATURE(has_kl, kKL)
    ASMJIT_X86_FEATURE(has_aeskle, kAESKLE)
    ASMJIT_X86_FEATURE(has_aesklewide_kl, kAESKLEWIDE_KL)

    ASMJIT_X86_FEATURE(has_avx10_1, kAVX10_1)
    ASMJIT_X86_FEATURE(has_avx10_2, kAVX10_2)

    ASMJIT_X86_FEATURE(has_amx_avx512, kAMX_AVX512)
    ASMJIT_X86_FEATURE(has_amx_bf16, kAMX_BF16)
    ASMJIT_X86_FEATURE(has_amx_complex, kAMX_COMPLEX)
    ASMJIT_X86_FEATURE(has_amx_fp16, kAMX_FP16)
    ASMJIT_X86_FEATURE(has_amx_fp8, kAMX_FP8)
    ASMJIT_X86_FEATURE(has_amx_int8, kAMX_INT8)
    ASMJIT_X86_FEATURE(has_amx_movrs, kAMX_MOVRS)
    ASMJIT_X86_FEATURE(has_amx_tf32, kAMX_TF32)
    ASMJIT_X86_FEATURE(has_amx_tile, kAMX_TILE)
    ASMJIT_X86_FEATURE(has_amx_transpose, kAMX_TRANSPOSE)

    #undef ASMJIT_X86_FEATURE

    ASMJIT_INLINE void remove_avx() noexcept {
      remove(kAVX                 ,
             kAVX2                ,
             kAVX_IFMA            ,
             kAVX_NE_CONVERT      ,
             kAVX_VNNI            ,
             kAVX_VNNI_INT16      ,
             kAVX_VNNI_INT8       ,
             kF16C                ,
             kFMA                 ,
             kFMA4                ,
             kVAES                ,
             kVPCLMULQDQ          ,
             kXOP);
      remove_avx512();
    }

    ASMJIT_INLINE void remove_avx512() noexcept {
      remove(kAVX512_BF16         ,
             kAVX512_BITALG       ,
             kAVX512_BW           ,
             kAVX512_CD           ,
             kAVX512_DQ           ,
             kAVX512_F            ,
             kAVX512_FP16         ,
             kAVX512_IFMA         ,
             kAVX512_VBMI         ,
             kAVX512_VBMI2        ,
             kAVX512_VL           ,
             kAVX512_VNNI         ,
             kAVX512_VP2INTERSECT ,
             kAVX512_VPOPCNTDQ    ,
             kAMX_AVX512);
      remove_avx10();
    }

    ASMJIT_INLINE void remove_avx10() noexcept {
      remove(kAVX10_1 | kAVX10_2);
    }

    ASMJIT_INLINE void remove_amx() noexcept {
      remove(kAMX_AVX512          ,
             kAMX_BF16            ,
             kAMX_COMPLEX         ,
             kAMX_FP16            ,
             kAMX_FP8             ,
             kAMX_INT8            ,
             kAMX_MOVRS           ,
             kAMX_TF32            ,
             kAMX_TILE            ,
             kAMX_TRANSPOSE);
    }
  };

  //! ARM specific features data.
  //!
  //! Naming reference:
  //!   - https://developer.arm.com/downloads/-/exploration-tools/feature-names-for-a-profile
  struct ARM : public Data {
    //! ARM CPU feature identifiers.
    enum Id : uint8_t {
      // @EnumValuesBegin{"enum": "CpuFeatures::ARM"}@
      kNone = 0,                 //!< No feature (never set, used internally).

      kARMv6,                    //!< CPU is at least ARMv6 {A32}.
      kARMv7,                    //!< CPU is at least ARMv7 {A32}.
      kARMv8a,                   //!< CPU is at least ARMv8A.
      kTHUMB,                    //!< CPU has THUMB               (16-bit THUMB encoding) {A32}.
      kTHUMBv2,                  //!< CPU has THUMBv2             (32-bit THUMB encoding) {A32}.

      kABLE,                     //!< CPU has ABLE                (address breakpoint linking extension) {A64}.
      kADERR,                    //!< CPU has ADERR               (asynchronous device error exceptions) {A64}.
      kAES,                      //!< CPU has AES                 (ASIMD AES instructions).
      kAFP,                      //!< CPU has AFP                 (alternate floating-point behavior) {A64}.
      kAIE,                      //!< CPU has AIE                 (memory attribute index enhancement) {A64}.
      kAMU1,                     //!< CPU has AMUv1               (activity monitors extension version 1) {A64}.
      kAMU1_1,                   //!< CPU has AMUv1p1             (activity monitors extension version 1.1) {A64}.
      kANERR,                    //!< CPU has ANERR               (asynchronous normal error exception) {A64}.
      kASIMD,                    //!< CPU has ASIMD               (NEON on ARM/THUMB).
      kBF16,                     //!< CPU has BF16                (BFloat16 instructions) {A64}.
      kBRBE,                     //!< CPU has BRBE                (branch record buffer extension) {A64}.
      kBTI,                      //!< CPU has BTI                 (branch target identification).
      kBWE,                      //!< CPU has BWE                 (breakpoint mismatch and range extension) {A64}.
      kCCIDX,                    //!< CPU has CCIDX               (extend of the CCSIDR number of sets).
      kCHK,                      //!< CPU has CHK                 (check feature status - CHKFEAT instruction) {A64}.
      kCLRBHB,                   //!< CPU has CLRBHB              (clear BHB instruction).
      kCMOW,                     //!< CPU has CMOW                (control for cache maintenance permission) {A64}.
      kCMPBR,                    //!< CPU has CMPBR               (Compare and branch instructions) {A64}.
      kCONSTPACFIELD,            //!< CPU has CONSTPACFIELD       (PAC algorithm enhancement) {A64}.
      kCPA,                      //!< CPU has CPA                 (instruction-only Checked Pointer Arithmetic) {A64}.
      kCPA2,                     //!< CPU has CPA2                (checked Pointer Arithmetic) {A64}.
      kCPUID,                    //!< CPU has CPUID               (CPUID registers accessible in user-space).
      kCRC32,                    //!< CPU has CRC32               (CRC32 instructions).
      kCSSC,                     //!< CPU has CSSC                (common short sequence compression) {A64}.
      kCSV2,                     //!< CPU has CSV2                (cache speculation variant 2 version 2.1) {A64}.
      kCSV2_3,                   //!< CPU has CSV2_3              (cache speculation variant 2 version 3) {A64}.
      kCSV3,                     //!< CPU has CSV3                (cache speculation Variant 3) {A64}.
      kD128,                     //!< CPU has D128                (128-bit translation tables, 56 bit PA) {A64}.
      kDGH,                      //!< CPU has DGH                 (data gathering hint) {A64}.
      kDIT,                      //!< CPU has DIT                 (data independent timing of instructions).
      kDOTPROD,                  //!< CPU has DOTPROD             (ASIMD Int8 dot product instructions).
      kDPB,                      //!< CPU has DPB                 (DC CVAP instruction) {A64}.
      kDPB2,                     //!< CPU has DPB2                (DC CVADP instruction) {A64}.
      kEBEP,                     //!< CPU has EBEP                (exception-based event profiling) {A64}.
      kEBF16,                    //!< CPU has EBF16               (extended BFloat16 mode) {A64}.
      kECBHB,                    //!< CPU has ECBHB               (exploitative control using branch history information) {A64}.
      kECV,                      //!< CPU has ECV                 (enhanced counter virtualization).
      kEDHSR,                    //!< CPU has EDHSR               (support for EDHSR) {A64}.
      kEDSP,                     //!< CPU has EDSP                (ARM/THUMB only).
      kF8E4M3,                   //!< CPU has F8E4M3              {A64}.
      kF8E5M2,                   //!< CPU has F8E5M2              {A64}.
      kF8F16MM,                  //!< CPU has F8F16MM             (8-bit floating-point matrix multiply-accumulate to half-precision) {A64}
      kF8F32MM,                  //!< CPU has F8F32MM             (8-bit floating-point matrix multiply-accumulate to single-precision) {A64}
      kFAMINMAX,                 //!< CPU has FAMINMAX            (floating-point maximum and minimum absolute value instructions) {A64}.
      kFCMA,                     //!< CPU has FCMA                (FCADD/FCMLA).
      kFGT,                      //!< CPU has FGT                 (fine-grained traps).
      kFGT2,                     //!< CPU has FGT2                (fine-grained traps 2).
      kFHM,                      //!< CPU has FHM                 (half-precision floating-point FMLAL instructions).
      kFLAGM,                    //!< CPU has FLAGM               (condition flag manipulation) {A64}.
      kFLAGM2,                   //!< CPU has FLAGM2              (condition flag manipulation version v2) {A64}.
      kFMAC,                     //!< CPU has FMAC                (ARM/THUMB only).
      kFP,                       //!< CPU has FP                  (floating-point) (on 32-bit ARM this means VFPv3).
      kFP16,                     //!< CPU has FP16                (half-precision floating-point data processing).
      kFP16CONV,                 //!< CPU has FP16CONV            (half-precision float conversion).
      kFP8,                      //!< CPU has FP8                 (FP8 convert instructions) {A64}.
      kFP8DOT2,                  //!< CPU has FP8DOT2             (FP8 2-way dot product to half-precision instructions) {A64}.
      kFP8DOT4,                  //!< CPU has FP8DOT4             (FP8 4-way dot product to single-precision instructions) {A64}.
      kFP8FMA,                   //!< CPU has FP8FMA              (FP8 multiply-accumulate to half-precision and single-precision instructions) {A64}.
      kFPMR,                     //!< CPU has FPMR                (floating-point Mode Register) {A64}.
      kFPRCVT,                   //!< CPU has FPRCVT              (floating-point to/from integer in scalar FP register) {A64}.
      kFRINTTS,                  //!< CPU has FRINTTS             (FRINT[32|64][X|Z] instructions) {A64}.
      kGCS,                      //!< CPU has GCS                 (guarded control stack extension) {A64}.
      kHACDBS,                   //!< CPU has HACDBS              (hardware accelerator for cleaning Dirty state) {A64}.
      kHAFDBS,                   //!< CPU has HAFDBS              (hardware management of the access flag and dirty state) {A64}.
      kHAFT,                     //!< CPU has HAFT                (hardware managed access flag for table descriptors) {A64}.
      kHDBSS,                    //!< CPU has HDBSS               (hardware Dirty state tracking Structure) {A64}.
      kHBC,                      //!< CPU has HBC                 (hinted conditional branches) {A64}.
      kHCX,                      //!< CPU has HCX                 (support for the HCRX_EL2 register) {A64}.
      kHPDS,                     //!< CPU has HPDS                (hierarchical permission disables in translation tables	) {A64}.
      kHPDS2,                    //!< CPU has HPDS2               (hierarchical permission disables) {A64}.
      kI8MM,                     //!< CPU has I8MM                (int8 matrix multiplication) {A64}.
      kIDIVA,                    //!< CPU has IDIV                (hardware SDIV and UDIV in ARM mode).
      kIDIVT,                    //!< CPU has IDIV                (hardware SDIV and UDIV in THUMB mode).
      kITE,                      //!< CPU has ITE                 (instrumentation extension) {A64}.
      kJSCVT,                    //!< CPU has JSCVT               (JavaScript FJCVTS conversion instruction) {A64}.
      kLOR,                      //!< CPU has LOR                 (limited ordering regions extension).
      kLRCPC,                    //!< CPU has LRCPC               (load-acquire RCpc instructions) {A64}.
      kLRCPC2,                   //!< CPU has LRCPC2              (load-acquire RCpc instructions v2) {A64}.
      kLRCPC3,                   //!< CPU has LRCPC3              (load-Acquire RCpc instructions v3) {A64}.
      kLS64,                     //!< CPU has LS64                (64 byte loads/stores without return) {A64}.
      kLS64_ACCDATA,             //!< CPU has LS64_ACCDATA        (64-byte EL0 stores with return) {A64}.
      kLS64_V,                   //!< CPU has LS64_V              (64-byte stores with return) {A64}.
      kLS64WB,                   //!< CPU has LS64WB              (LS64 for Write-back cacheable memory) {A64}
      kLSE,                      //!< CPU has LSE                 (large system extensions) {A64}.
      kLSE128,                   //!< CPU has LSE128              (128-bit atomics) {A64}.
      kLSE2,                     //!< CPU has LSE2                (large system extensions v2) {A64}.
      kLSFE,                     //!< CPU has LSFE                (large system float extension) {A64}.
      kLSUI,                     //!< CPU has LSUI                (unprivileged load store) {A64}.
      kLUT,                      //!< CPU has LUT                 (lookup table instructions with 2-bit and 4-bit indices) {A64}.
      kLVA,                      //!< CPU has LVA                 (large VA support) {A64}.
      kLVA3,                     //!< CPU has LVA3                (56-bit VA) {A64}.
      kMEC,                      //!< CPU has MEC                 (memory encryption contexts) {A64}.
      kMOPS,                     //!< CPU has MOPS                (memcpy and memset acceleration instructions) {A64}.
      kMPAM,                     //!< CPU has MPAM                (memory system partitioning and monitoring extension) {A64}.
      kMTE,                      //!< CPU has MTE                 (instruction-only memory tagging extension) {A64}.
      kMTE2,                     //!< CPU has MTE2                (full memory tagging extension) {A64}.
      kMTE3,                     //!< CPU has MTE3                (MTE asymmetric fault handling) {A64}.
      kMTE4,                     //!< CPU has MTE4                (MTE v4) {A64}.
      kMTE_ASYM_FAULT,           //!< CPU has MTE_ASYM_FAULT      (memory tagging asymmetric faults) {A64}.
      kMTE_ASYNC,                //!< CPU has MTE_ASYNC           (memory tagging asynchronous faulting) {A64}.
      kMTE_CANONICAL_TAGS,       //!< CPU has MTE_CANONICAL_TAGS  (canonical tag checking for untagged memory) {A64}.
      kMTE_NO_ADDRESS_TAGS,      //!< CPU has MTE_NO_ADDRESS_TAGS (memory tagging with address tagging disabled) {A64}.
      kMTE_PERM_S1,              //!< CPU has MTE_PERM_S1         (allocation tag access permission) {A64}.
      kMTE_STORE_ONLY,           //!< CPU has MTE_STORE_ONLY      (store-only tag checking) {A64}.
      kMTE_TAGGED_FAR,           //!< CPU has MTE_TAGGED_FAR      (FAR_ELx on a tag check fault) {A64}.
      kMTPMU,                    //!< CPU has MTPMU               (multi-threaded PMU extensions) {A64}.
      kNMI,                      //!< CPU has NMI                 (non-maskable Interrupt) {A64}.
      kNV,                       //!< CPU has NV                  (nested virtualization enchancement) {A64}.
      kNV2,                      //!< CPU has NV2                 (enhanced support for nested virtualization) {A64}.
      kOCCMO,                    //!< CPU has OCCMO               (outer cacheable cache maintenance operation) {A64}.
      kPAN,                      //!< CPU has PAN                 (privileged access-never extension) {A64}.
      kPAN2,                     //!< CPU has PAN2                (PAN s1e1R and s1e1W variants) {A64}.
      kPAN3,                     //!< CPU has PAN3                (support for SCTLR_ELx.EPAN) {A64}.
      kPAUTH,                    //!< CPU has PAUTH               (pointer authentication extension) {A64}.
      kPFAR,                     //!< CPU has PFAR                (physical fault address registers) {A64}.
      kPMU,                      //!< CPU has PMU                 {A64}.
      kPMULL,                    //!< CPU has PMULL               (ASIMD PMULL instructions) {A64}.
      kPRFMSLC,                  //!< CPU has PRFMSLC             (PRFM instructions support the SLC target) {A64}.
      kRAS,                      //!< CPU has RAS                 (reliability, availability and serviceability extensions).
      kRAS1_1,                   //!< CPU has RASv1p1             (RAS v1.1).
      kRAS2,                     //!< CPU has RASv2               (RAS v2).
      kRASSA2,                   //!< CPU has RASSAv2             (RAS v2 system architecture).
      kRDM,                      //!< CPU has RDM                 (rounding double multiply accumulate) {A64}.
      kRME,                      //!< CPU has RME                 (memory encryption contexts extension) {A64}.
      kRNG,                      //!< CPU has RNG                 (random number generation).
      kRNG_TRAP,                 //!< CPU has RNG_TRAP            (random number trap to EL3 field) {A64}.
      kRPRES,                    //!< CPU has RPRES               (increased precision of reciprocal estimate and RSQRT estimate) {A64}.
      kRPRFM,                    //!< CPU has RPRFM               (range prefetch hint instruction).
      kS1PIE,                    //!< CPU has S1PIE               (permission model enhancements) {A64}.
      kS1POE,                    //!< CPU has S1POE               (permission model enhancements) {A64}.
      kS2PIE,                    //!< CPU has S2PIE               (permission model enhancements) {A64}.
      kS2POE,                    //!< CPU has S2POE               (permission model enhancements) {A64}.
      kSB,                       //!< CPU has SB                  (speculative barrier).
      kSCTLR2,                   //!< CPU has SCTLR2              (extension to SCTLR_ELx) {A64}.
      kSEBEP,                    //!< CPU has SEBEP               (synchronous exception-based event profiling) {A64}.
      kSEL2,                     //!< CPU has SEL2                (secure EL2) {A64}.
      kSHA1,                     //!< CPU has SHA1                (ASIMD SHA1 instructions).
      kSHA256,                   //!< CPU has SHA256              (ASIMD SHA256 instructions).
      kSHA3,                     //!< CPU has SHA3                (ASIMD EOR3, RAX1, XAR, and BCAX instructions).
      kSHA512,                   //!< CPU has SHA512              (ASIMD SHA512 instructions).
      kSM3,                      //!< CPU has SM3                 (ASIMD SM3 instructions).
      kSM4,                      //!< CPU has SM4                 (ASIMD SM4 instructions).
      kSME,                      //!< CPU has SME                 (SME v1 - scalable matrix extension) {A64}.
      kSME2,                     //!< CPU has SME2                (SME v2) {A64}.
      kSME2_1,                   //!< CPU has SME2p1              (SME v2.1) {A64}.
      kSME2_2,                   //!< CPU has SME2p1              (SME v2.2) {A64}.
      kSME_AES,                  //!< CPU has SME_AES             {A64}.
      kSME_B16B16,               //!< CPU has SME_B16B16          (SME non-widening BFloat16 to BFloat16 arithmetic) {A64}.
      kSME_B16F32,               //!< CPU has SME_B16F32          (BFMOPA and BFMOPS instructions that accumulate BFloat16 outer products into single-precision tiles) {A64}.
      kSME_BI32I32,              //!< CPU has SME_BI32I32         (BMOPA and BMOPS instructions that accumulate 1-bit binary outer products into 32-bit integer tiles) {A64}.
      kSME_F16F16,               //!< CPU has SME_F16F16          (SME2.1 non-widening half-precision FP16 to FP16 arithmetic) {A64}.
      kSME_F16F32,               //!< CPU has SME_F16F32          {A64}.
      kSME_F32F32,               //!< CPU has SME_F32F32          {A64}.
      kSME_F64F64,               //!< CPU has SME_F64F64          {A64}.
      kSME_F8F16,                //!< CPU has SME_F8F16           (SME2 ZA-targeting FP8 multiply-accumulate, dot product, and outer product to half-precision instructions) {A64}.
      kSME_F8F32,                //!< CPU has SME_F8F32           (SME2 ZA-targeting FP8 multiply-accumulate, dot product, and outer product to single-precision instructions) {A64}.
      kSME_FA64,                 //!< CPU has SME_FA64            {A64}.
      kSME_I16I32,               //!< CPU has SME_I16I32          {A64}.
      kSME_I16I64,               //!< CPU has SME_I16I64          {A64}.
      kSME_I8I32,                //!< CPU has SME_I8I32           {A64}.
      kSME_LUTv2,                //!< CPU has SME_LUTv2           (lookup table instructions with 4-bit indices and 8-bit elements) {A64}.
      kSME_MOP4,                 //!< CPU has SME_MOP4            (quarter-tile outer product instructions) {A64}.
      kSME_TMOP,                 //!< CPU has SME_TMOP            {A64}.
      kSPE,                      //!< CPU has SPE                 (statistical profiling extension) {A64}.
      kSPE1_1,                   //!< CPU has SPEv1p1             (statistical profiling extensions version 1.1) {A64}.
      kSPE1_2,                   //!< CPU has SPEv1p2             (statistical profiling extensions version 1.2) {A64}.
      kSPE1_3,                   //!< CPU has SPEv1p3             (statistical profiling extensions version 1.3) {A64}.
      kSPE1_4,                   //!< CPU has SPEv1p4             (statistical profiling extensions version 1.4) {A64}.
      kSPE_ALTCLK,               //!< CPU has SPE_ALTCLK          (statistical profiling alternate clock domain extension) {A64}.
      kSPE_CRR,                  //!< CPU has SPE_CRR             (statistical profiling call return branch records) {A64}.
      kSPE_EFT,                  //!< CPU has SPE_EFT             (statistical profiling extended filtering by type) {A64}.
      kSPE_FDS,                  //!< CPU has SPE_FDS             (statistical profiling data source filtering) {A64}.
      kSPE_FPF,                  //!< CPU has SPE_FPF             (statistical profiling floating-point flag extension) {A64}.
      kSPE_SME,                  //!< CPU has SPE_SME             (statistical profiling extensions for SME) {A64}.
      kSPECRES,                  //!< CPU has SPECRES             (speculation restriction instructions).
      kSPECRES2,                 //!< CPU has SPECRES2            (clear other speculative predictions).
      kSPMU,                     //!< CPU has SPMU                (system performance monitors extension) {A64}.
      kSSBS,                     //!< CPU has SSBS                (speculative store bypass safe instruction).
      kSSBS2,                    //!< CPU has SSBS2               (MRS and MSR instructions for SSBS).
      kSSVE_AES,                 //!< CPU has SSVE_AES            {A64}.
      kSSVE_BITPERM,             //!< CPU has SSVE_BITPERM        {A64}.
      kSSVE_FEXPA,               //!< CPU has SSVE_FEXPA          {A64}.
      kSSVE_FP8DOT2,             //!< CPU has SSVE_FP8DOT2        (SVE2 FP8 2-way dot product to half-precision instructions in Streaming SVE mode) {A64}.
      kSSVE_FP8DOT4,             //!< CPU has SSVE_FP8DOT4        (SVE2 FP8 4-way dot product to single-precision instructions in Streaming SVE mode) {A64}.
      kSSVE_FP8FMA,              //!< CPU has SSVE_FP8FMA         (SVE2 FP8 multiply-accumulate to half-precision and single-precision instructions in Streaming SVE mode) {A64}.
      kSVE,                      //!< CPU has SVE                 (SVE v1 - scalable vector extension) {A64}.
      kSVE2,                     //!< CPU has SVE2                (SVE v2) {A64}.
      kSVE2_1,                   //!< CPU has SVE2p1              (SVE v2.1) {A64}.
      kSVE2_2,                   //!< CPU has SVE2p2              (SVE v2.2) {A64}.
      kSVE_AES,                  //!< CPU has SVE_AES             (SVE AES instructions) {A64}.
      kSVE_AES2,                 //!< CPU has SVE_AES2            {A64}.
      kSVE_B16B16,               //!< CPU has SVE_B16B16          (SVE non-widening BFloat16 to BFloat16 arithmetic) {A64}.
      kSVE_BF16,                 //!< CPU has SVE_BF16            (SVE BF16 instructions) {A64}.
      kSVE_BFSCALE,              //!< CPU has SVE_BFSCALE         {A64}.
      kSVE_BITPERM,              //!< CPU has SVE_BITPERM         (SVE bit permute) {A64}.
      kSVE_EBF16,                //!< CPU has SVE_EBF16           (SVE extended BFloat16 mode) {A64}.
      kSVE_ELTPERM,              //!< CPU has SVE_ELTPERM         {A64}.
      kSVE_F16MM,                //!< CPU has SVE_F16MM           (SVE half-precision floating-point matrix multiply instruction) {A64}.
      kSVE_F32MM,                //!< CPU has SVE_F32MM           (SVE single-precision floating-point matrix multiply instruction) {A64}.
      kSVE_F64MM,                //!< CPU has SVE_F64MM           (SVE double-precision floating-point matrix multiply instruction) {A64}.
      kSVE_I8MM,                 //!< CPU has SVE_I8MM            (SVE int8 matrix multiplication) {A64}.
      kSVE_PMULL128,             //!< CPU has SVE_PMULL128        (SVE PMULL instructions) {A64}.
      kSVE_SHA3,                 //!< CPU has SVE_SHA3            (SVE SHA-3 instructions) {A64}.
      kSVE_SM4,                  //!< CPU has SVE_SM4             (SVE SM4 instructions {A64}.
      kSYSINSTR128,              //!< CPU has SYSINSTR128         (128-bit system instructions) {A64}.
      kSYSREG128,                //!< CPU has SYSREG128           (128-bit system registers) {A64}.
      kTHE,                      //!< CPU has THE                 (translation hardening extension).
      kTLBIOS,                   //!< CPU has TLBIOS              (TLBI instructions in Outer Shareable domain) {A64}.
      kTLBIRANGE,                //!< CPU has TLBIRANGE           (TLBI range instructions) {A64}.
      kTLBIW,                    //!< CPU has TLBIW               (TLBI VMALL for dirty state) {A64}.
      kTME,                      //!< CPU has TME                 (transactional memory extensions).
      kTRF,                      //!< CPU has TRF                 (self-hosted trace extensions).
      kUAO,                      //!< CPU has UAO                 (AArch64 v8.2 UAO PState) {A64}.
      kVFP_D32,                  //!< CPU has VFP_D32             (32 VFP-D registers) (ARM/THUMB only).
      kVHE,                      //!< CPU has VHE                 (virtual host extension).
      kVMID16,                   //!< CPU has VMID16              (16-bit VMID) {A64}.
      kWFXT,                     //!< CPU has WFxT                (WFE and WFI instructions with timeout) {A64}.
      kXNX,                      //!< CPU has XNX                 (translation table stage 2 unprivileged execute-never) {A64}.
      kXS,                       //!< CPU has XS                  (XS attribute in TLBI and DSB instructions) {A64}.
      // @EnumValuesEnd@

      kMaxValue = kXS
    };

    #define ASMJIT_ARM_FEATURE(accessor, feature) \
      /*! Tests whether feature is present. */ \
      ASMJIT_INLINE_NODEBUG bool accessor() const noexcept { return has(ARM::feature); }

    ASMJIT_ARM_FEATURE(has_thumb, kTHUMB)
    ASMJIT_ARM_FEATURE(has_thumb_v2, kTHUMBv2)

    ASMJIT_ARM_FEATURE(has_armv6, kARMv6)
    ASMJIT_ARM_FEATURE(has_armv7, kARMv7)
    ASMJIT_ARM_FEATURE(has_armv8a, kARMv8a)

    ASMJIT_ARM_FEATURE(has_able, kABLE)
    ASMJIT_ARM_FEATURE(has_aderr, kADERR)
    ASMJIT_ARM_FEATURE(has_aes, kAES)
    ASMJIT_ARM_FEATURE(has_afp, kAFP)
    ASMJIT_ARM_FEATURE(has_aie, kAIE)
    ASMJIT_ARM_FEATURE(has_amu1, kAMU1)
    ASMJIT_ARM_FEATURE(has_amu1_1, kAMU1_1)
    ASMJIT_ARM_FEATURE(has_anerr, kANERR)
    ASMJIT_ARM_FEATURE(has_asimd, kASIMD)
    ASMJIT_ARM_FEATURE(has_bf16, kBF16)
    ASMJIT_ARM_FEATURE(has_brbe, kBRBE)
    ASMJIT_ARM_FEATURE(has_bti, kBTI)
    ASMJIT_ARM_FEATURE(has_bwe, kBWE)
    ASMJIT_ARM_FEATURE(has_ccidx, kCCIDX)
    ASMJIT_ARM_FEATURE(has_chk, kCHK)
    ASMJIT_ARM_FEATURE(has_clrbhb, kCLRBHB)
    ASMJIT_ARM_FEATURE(has_cmow, kCMOW)
    ASMJIT_ARM_FEATURE(has_cmpbr, kCMPBR)
    ASMJIT_ARM_FEATURE(has_constpacfield, kCONSTPACFIELD)
    ASMJIT_ARM_FEATURE(has_cpa, kCPA)
    ASMJIT_ARM_FEATURE(has_cpa2, kCPA2)
    ASMJIT_ARM_FEATURE(has_cpuid, kCPUID)
    ASMJIT_ARM_FEATURE(has_crc32, kCRC32)
    ASMJIT_ARM_FEATURE(has_cssc, kCSSC)
    ASMJIT_ARM_FEATURE(has_csv2, kCSV2)
    ASMJIT_ARM_FEATURE(has_csv2_3, kCSV2_3)
    ASMJIT_ARM_FEATURE(has_csv3, kCSV3)
    ASMJIT_ARM_FEATURE(has_d128, kD128)
    ASMJIT_ARM_FEATURE(has_dgh, kDGH)
    ASMJIT_ARM_FEATURE(has_dit, kDIT)
    ASMJIT_ARM_FEATURE(has_dotprod, kDOTPROD)
    ASMJIT_ARM_FEATURE(has_dpb, kDPB)
    ASMJIT_ARM_FEATURE(has_dpb2, kDPB2)
    ASMJIT_ARM_FEATURE(has_ebep, kEBEP)
    ASMJIT_ARM_FEATURE(has_ebf16, kEBF16)
    ASMJIT_ARM_FEATURE(has_ecbhb, kECBHB)
    ASMJIT_ARM_FEATURE(has_ecv, kECV)
    ASMJIT_ARM_FEATURE(has_edhsr, kEDHSR)
    ASMJIT_ARM_FEATURE(has_edsp, kEDSP)
    ASMJIT_ARM_FEATURE(has_f8e4m3, kF8E4M3)
    ASMJIT_ARM_FEATURE(has_f8e5m2, kF8E5M2)
    ASMJIT_ARM_FEATURE(has_f8f16mm, kF8F16MM)
    ASMJIT_ARM_FEATURE(has_f8f32mm, kF8F32MM)
    ASMJIT_ARM_FEATURE(has_faminmax, kFAMINMAX)
    ASMJIT_ARM_FEATURE(has_fcma, kFCMA)
    ASMJIT_ARM_FEATURE(has_fgt, kFGT)
    ASMJIT_ARM_FEATURE(has_fgt2, kFGT2)
    ASMJIT_ARM_FEATURE(has_fhm, kFHM)
    ASMJIT_ARM_FEATURE(has_flagm, kFLAGM)
    ASMJIT_ARM_FEATURE(has_flagm2, kFLAGM2)
    ASMJIT_ARM_FEATURE(has_fmac, kFMAC)
    ASMJIT_ARM_FEATURE(has_fp, kFP)
    ASMJIT_ARM_FEATURE(has_fp16, kFP16)
    ASMJIT_ARM_FEATURE(has_fp16conv, kFP16CONV)
    ASMJIT_ARM_FEATURE(has_fp8, kFP8)
    ASMJIT_ARM_FEATURE(has_fp8dot2, kFP8DOT2)
    ASMJIT_ARM_FEATURE(has_fp8dot4, kFP8DOT4)
    ASMJIT_ARM_FEATURE(has_fp8fma, kFP8FMA)
    ASMJIT_ARM_FEATURE(has_fpmr, kFPMR)
    ASMJIT_ARM_FEATURE(has_fprcvt, kFPRCVT)
    ASMJIT_ARM_FEATURE(has_frintts, kFRINTTS)
    ASMJIT_ARM_FEATURE(has_gcs, kGCS)
    ASMJIT_ARM_FEATURE(has_hacdbs, kHACDBS)
    ASMJIT_ARM_FEATURE(has_hafdbs, kHAFDBS)
    ASMJIT_ARM_FEATURE(has_haft, kHAFT)
    ASMJIT_ARM_FEATURE(has_hdbss, kHDBSS)
    ASMJIT_ARM_FEATURE(has_hbc, kHBC)
    ASMJIT_ARM_FEATURE(has_hcx, kHCX)
    ASMJIT_ARM_FEATURE(has_hpds, kHPDS)
    ASMJIT_ARM_FEATURE(has_hpds2, kHPDS2)
    ASMJIT_ARM_FEATURE(has_i8mm, kI8MM)
    ASMJIT_ARM_FEATURE(has_idiva, kIDIVA)
    ASMJIT_ARM_FEATURE(has_idivt, kIDIVT)
    ASMJIT_ARM_FEATURE(has_ite, kITE)
    ASMJIT_ARM_FEATURE(has_jscvt, kJSCVT)
    ASMJIT_ARM_FEATURE(has_lor, kLOR)
    ASMJIT_ARM_FEATURE(has_lrcpc, kLRCPC)
    ASMJIT_ARM_FEATURE(has_lrcpc2, kLRCPC2)
    ASMJIT_ARM_FEATURE(has_lrcpc3, kLRCPC3)
    ASMJIT_ARM_FEATURE(has_ls64, kLS64)
    ASMJIT_ARM_FEATURE(has_ls64_accdata, kLS64_ACCDATA)
    ASMJIT_ARM_FEATURE(has_ls64_v, kLS64_V)
    ASMJIT_ARM_FEATURE(has_ls64wb, kLS64WB)
    ASMJIT_ARM_FEATURE(has_lse, kLSE)
    ASMJIT_ARM_FEATURE(has_lse128, kLSE128)
    ASMJIT_ARM_FEATURE(has_lse2, kLSE2)
    ASMJIT_ARM_FEATURE(has_lsfe, kLSFE)
    ASMJIT_ARM_FEATURE(has_lsui, kLSUI)
    ASMJIT_ARM_FEATURE(has_lut, kLUT)
    ASMJIT_ARM_FEATURE(has_lva, kLVA)
    ASMJIT_ARM_FEATURE(has_lva3, kLVA3)
    ASMJIT_ARM_FEATURE(has_mec, kMEC)
    ASMJIT_ARM_FEATURE(has_mops, kMOPS)
    ASMJIT_ARM_FEATURE(has_mpam, kMPAM)
    ASMJIT_ARM_FEATURE(has_mte, kMTE)
    ASMJIT_ARM_FEATURE(has_mte2, kMTE2)
    ASMJIT_ARM_FEATURE(has_mte3, kMTE3)
    ASMJIT_ARM_FEATURE(has_mte4, kMTE4)
    ASMJIT_ARM_FEATURE(has_mte_asym_fault, kMTE_ASYM_FAULT)
    ASMJIT_ARM_FEATURE(has_mte_async, kMTE_ASYNC)
    ASMJIT_ARM_FEATURE(has_mte_canonical_tags, kMTE_CANONICAL_TAGS)
    ASMJIT_ARM_FEATURE(has_mte_no_address_tags, kMTE_NO_ADDRESS_TAGS)
    ASMJIT_ARM_FEATURE(has_mte_perm_s1, kMTE_PERM_S1)
    ASMJIT_ARM_FEATURE(has_mte_store_only, kMTE_STORE_ONLY)
    ASMJIT_ARM_FEATURE(has_mte_tagged_far, kMTE_TAGGED_FAR)
    ASMJIT_ARM_FEATURE(has_mtpmu, kMTPMU)
    ASMJIT_ARM_FEATURE(has_nmi, kNMI)
    ASMJIT_ARM_FEATURE(has_nv, kNV)
    ASMJIT_ARM_FEATURE(has_nv2, kNV2)
    ASMJIT_ARM_FEATURE(has_occmo, kOCCMO)
    ASMJIT_ARM_FEATURE(has_pan, kPAN)
    ASMJIT_ARM_FEATURE(has_pan2, kPAN2)
    ASMJIT_ARM_FEATURE(has_pan3, kPAN3)
    ASMJIT_ARM_FEATURE(has_pauth, kPAUTH)
    ASMJIT_ARM_FEATURE(has_pfar, kPFAR)
    ASMJIT_ARM_FEATURE(has_pmu, kPMU)
    ASMJIT_ARM_FEATURE(has_pmull, kPMULL)
    ASMJIT_ARM_FEATURE(has_prfmslc, kPRFMSLC)
    ASMJIT_ARM_FEATURE(has_ras, kRAS)
    ASMJIT_ARM_FEATURE(has_ras1_1, kRAS1_1)
    ASMJIT_ARM_FEATURE(has_ras2, kRAS2)
    ASMJIT_ARM_FEATURE(has_rassa2, kRASSA2)
    ASMJIT_ARM_FEATURE(has_rdm, kRDM)
    ASMJIT_ARM_FEATURE(has_rme, kRME)
    ASMJIT_ARM_FEATURE(has_rng, kRNG)
    ASMJIT_ARM_FEATURE(has_rng_trap, kRNG_TRAP)
    ASMJIT_ARM_FEATURE(has_rpres, kRPRES)
    ASMJIT_ARM_FEATURE(has_rprfm, kRPRFM)
    ASMJIT_ARM_FEATURE(has_s1pie, kS1PIE)
    ASMJIT_ARM_FEATURE(has_s1poe, kS1POE)
    ASMJIT_ARM_FEATURE(has_s2pie, kS2PIE)
    ASMJIT_ARM_FEATURE(has_s2poe, kS2POE)
    ASMJIT_ARM_FEATURE(has_sb, kSB)
    ASMJIT_ARM_FEATURE(has_sctlr2, kSCTLR2)
    ASMJIT_ARM_FEATURE(has_sebep, kSEBEP)
    ASMJIT_ARM_FEATURE(has_sel2, kSEL2)
    ASMJIT_ARM_FEATURE(has_sha1, kSHA1)
    ASMJIT_ARM_FEATURE(has_sha256, kSHA256)
    ASMJIT_ARM_FEATURE(has_sha3, kSHA3)
    ASMJIT_ARM_FEATURE(has_sha512, kSHA512)
    ASMJIT_ARM_FEATURE(has_sm3, kSM3)
    ASMJIT_ARM_FEATURE(has_sm4, kSM4)
    ASMJIT_ARM_FEATURE(has_sme, kSME)
    ASMJIT_ARM_FEATURE(has_sme2, kSME2)
    ASMJIT_ARM_FEATURE(has_sme2_1, kSME2_1)
    ASMJIT_ARM_FEATURE(has_sme2_2, kSME2_2)
    ASMJIT_ARM_FEATURE(has_sme_aes, kSME_AES)
    ASMJIT_ARM_FEATURE(has_sme_b16b16, kSME_B16B16)
    ASMJIT_ARM_FEATURE(has_sme_b16f32, kSME_B16F32)
    ASMJIT_ARM_FEATURE(has_sme_bi32i32, kSME_BI32I32)
    ASMJIT_ARM_FEATURE(has_sme_f16f16, kSME_F16F16)
    ASMJIT_ARM_FEATURE(has_sme_f16f32, kSME_F16F32)
    ASMJIT_ARM_FEATURE(has_sme_f32f32, kSME_F32F32)
    ASMJIT_ARM_FEATURE(has_sme_f64f64, kSME_F64F64)
    ASMJIT_ARM_FEATURE(has_sme_f8f16, kSME_F8F16)
    ASMJIT_ARM_FEATURE(has_sme_f8f32, kSME_F8F32)
    ASMJIT_ARM_FEATURE(has_sme_fa64, kSME_FA64)
    ASMJIT_ARM_FEATURE(has_sme_i16i32, kSME_I16I32)
    ASMJIT_ARM_FEATURE(has_sme_i16i64, kSME_I16I64)
    ASMJIT_ARM_FEATURE(has_sme_i8i32, kSME_I8I32)
    ASMJIT_ARM_FEATURE(has_sme_lutv2, kSME_LUTv2)
    ASMJIT_ARM_FEATURE(has_sme_mop4, kSME_MOP4)
    ASMJIT_ARM_FEATURE(has_sme_tmop, kSME_TMOP)
    ASMJIT_ARM_FEATURE(has_spe, kSPE)
    ASMJIT_ARM_FEATURE(has_spe1_1, kSPE1_1)
    ASMJIT_ARM_FEATURE(has_spe1_2, kSPE1_2)
    ASMJIT_ARM_FEATURE(has_spe1_3, kSPE1_3)
    ASMJIT_ARM_FEATURE(has_spe1_4, kSPE1_4)
    ASMJIT_ARM_FEATURE(has_spe_altclk, kSPE_ALTCLK)
    ASMJIT_ARM_FEATURE(has_spe_crr, kSPE_CRR)
    ASMJIT_ARM_FEATURE(has_spe_eft, kSPE_EFT)
    ASMJIT_ARM_FEATURE(has_spe_fds, kSPE_FDS)
    ASMJIT_ARM_FEATURE(has_spe_fpf, kSPE_FPF)
    ASMJIT_ARM_FEATURE(has_spe_sme, kSPE_SME)
    ASMJIT_ARM_FEATURE(has_specres, kSPECRES)
    ASMJIT_ARM_FEATURE(has_specres2, kSPECRES2)
    ASMJIT_ARM_FEATURE(has_spmu, kSPMU)
    ASMJIT_ARM_FEATURE(has_ssbs, kSSBS)
    ASMJIT_ARM_FEATURE(has_ssbs2, kSSBS2)
    ASMJIT_ARM_FEATURE(has_ssve_aes, kSSVE_AES)
    ASMJIT_ARM_FEATURE(has_ssve_bitperm, kSSVE_BITPERM)
    ASMJIT_ARM_FEATURE(has_ssve_fexpa, kSSVE_FEXPA)
    ASMJIT_ARM_FEATURE(has_ssve_fp8dot2, kSSVE_FP8DOT2)
    ASMJIT_ARM_FEATURE(has_ssve_fp8dot4, kSSVE_FP8DOT4)
    ASMJIT_ARM_FEATURE(has_ssve_fp8fma, kSSVE_FP8FMA)
    ASMJIT_ARM_FEATURE(has_sve, kSVE)
    ASMJIT_ARM_FEATURE(has_sve2, kSVE2)
    ASMJIT_ARM_FEATURE(has_sve2_1, kSVE2_1)
    ASMJIT_ARM_FEATURE(has_sve2_2, kSVE2_2)
    ASMJIT_ARM_FEATURE(has_sve_aes, kSVE_AES)
    ASMJIT_ARM_FEATURE(has_sve_aes2, kSVE_AES2)
    ASMJIT_ARM_FEATURE(has_sve_b16b16, kSVE_B16B16)
    ASMJIT_ARM_FEATURE(has_sve_bf16, kSVE_BF16)
    ASMJIT_ARM_FEATURE(has_sve_bfscale, kSVE_BFSCALE)
    ASMJIT_ARM_FEATURE(has_sve_bitperm, kSVE_BITPERM)
    ASMJIT_ARM_FEATURE(has_sve_ebf16, kSVE_EBF16)
    ASMJIT_ARM_FEATURE(has_sve_eltperm, kSVE_ELTPERM)
    ASMJIT_ARM_FEATURE(has_sve_f16mm, kSVE_F16MM)
    ASMJIT_ARM_FEATURE(has_sve_f32mm, kSVE_F32MM)
    ASMJIT_ARM_FEATURE(has_sve_f64mm, kSVE_F64MM)
    ASMJIT_ARM_FEATURE(has_sve_i8mm, kSVE_I8MM)
    ASMJIT_ARM_FEATURE(has_sve_pmull128, kSVE_PMULL128)
    ASMJIT_ARM_FEATURE(has_sve_sha3, kSVE_SHA3)
    ASMJIT_ARM_FEATURE(has_sve_sm4, kSVE_SM4)
    ASMJIT_ARM_FEATURE(has_sysinstr128, kSYSINSTR128)
    ASMJIT_ARM_FEATURE(has_sysreg128, kSYSREG128)
    ASMJIT_ARM_FEATURE(has_the, kTHE)
    ASMJIT_ARM_FEATURE(has_tlbios, kTLBIOS)
    ASMJIT_ARM_FEATURE(has_tlbirange, kTLBIRANGE)
    ASMJIT_ARM_FEATURE(has_tlbiw, kTLBIW)
    ASMJIT_ARM_FEATURE(has_tme, kTME)
    ASMJIT_ARM_FEATURE(has_trf, kTRF)
    ASMJIT_ARM_FEATURE(has_uao, kUAO)
    ASMJIT_ARM_FEATURE(has_vfp_d32, kVFP_D32)
    ASMJIT_ARM_FEATURE(has_vhe, kVHE)
    ASMJIT_ARM_FEATURE(has_vmid16, kVMID16)
    ASMJIT_ARM_FEATURE(has_wfxt, kWFXT)
    ASMJIT_ARM_FEATURE(has_xnx, kXNX)
    ASMJIT_ARM_FEATURE(has_xs, kXS)

    #undef ASMJIT_ARM_FEATURE
  };

  static_assert(uint32_t(X86::kMaxValue) < kMaxFeatures, "The number of X86 CPU features cannot exceed CpuFeatures::kMaxFeatures");
  static_assert(uint32_t(ARM::kMaxValue) < kMaxFeatures, "The number of ARM CPU features cannot exceed CpuFeatures::kMaxFeatures");

  //! \}

  //! \name Members
  //! \{

  Data _data {};

  //! \}

  //! \name Construction & Destruction
  //! \{

  ASMJIT_INLINE_NODEBUG CpuFeatures() noexcept {}
  ASMJIT_INLINE_NODEBUG CpuFeatures(const CpuFeatures& other) noexcept = default;
  ASMJIT_INLINE_NODEBUG explicit CpuFeatures(const Data& other) noexcept : _data{other._bits} {}
  ASMJIT_INLINE_NODEBUG explicit CpuFeatures(Globals::NoInit_) noexcept {}

  //! \}

  //! \name Overloaded Operators
  //! \{

  ASMJIT_INLINE_NODEBUG CpuFeatures& operator=(const CpuFeatures& other) noexcept = default;

  ASMJIT_INLINE_NODEBUG bool operator==(const CpuFeatures& other) const noexcept { return  equals(other); }
  ASMJIT_INLINE_NODEBUG bool operator!=(const CpuFeatures& other) const noexcept { return !equals(other); }

  //! \}

  //! \name Accessors
  //! \{

  //! Returns true if there are no features set.
  ASMJIT_INLINE_NODEBUG bool is_empty() const noexcept { return _data.is_empty(); }

  //! Casts this base class into a derived type `T`.
  template<typename T = Data>
  ASMJIT_INLINE_NODEBUG T& data() noexcept { return static_cast<T&>(_data); }

  //! Casts this base class into a derived type `T` (const).
  template<typename T = Data>
  ASMJIT_INLINE_NODEBUG const T& data() const noexcept { return static_cast<const T&>(_data); }

  //! Returns CpuFeatures::Data as \ref CpuFeatures::X86.
  ASMJIT_INLINE_NODEBUG X86& x86() noexcept { return data<X86>(); }
  //! Returns CpuFeatures::Data as \ref CpuFeatures::X86 (const).
  ASMJIT_INLINE_NODEBUG const X86& x86() const noexcept { return data<X86>(); }

  //! Returns CpuFeatures::Data as \ref CpuFeatures::ARM.
  ASMJIT_INLINE_NODEBUG ARM& arm() noexcept { return data<ARM>(); }
  //! Returns CpuFeatures::Data as \ref CpuFeatures::ARM (const).
  ASMJIT_INLINE_NODEBUG const ARM& arm() const noexcept { return data<ARM>(); }

  //! Returns all features as array of bitwords (see \ref Support::BitWord).
  ASMJIT_INLINE_NODEBUG BitWord* bits() noexcept { return _data.bits(); }
  //! Returns all features as array of bitwords (const).
  ASMJIT_INLINE_NODEBUG const BitWord* bits() const noexcept { return _data.bits(); }
  //! Returns the number of BitWords returned by \ref bits().
  ASMJIT_INLINE_NODEBUG size_t bit_word_count() const noexcept { return _data.bit_word_count(); }

  //! Returns \ref Support::BitVectorIterator, that can be used to iterate over all features efficiently.
  ASMJIT_INLINE_NODEBUG Iterator iterator() const noexcept { return _data.iterator(); }

  //! Tests whether the feature `feature_id` is present.
  template<typename FeatureId>
  ASMJIT_INLINE_NODEBUG bool has(const FeatureId& feature_id) const noexcept { return _data.has(feature_id); }

  //! Tests whether any of the features is present.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG bool has_any(Args&&... args) const noexcept { return _data.has_any(std::forward<Args>(args)...); }

  //! Tests whether all features as defined by `other` are present.
  ASMJIT_INLINE_NODEBUG bool has_all(const CpuFeatures& other) const noexcept { return _data.has_all(other._data); }

  //! \}

  //! \name Manipulation
  //! \{

  //! Clears all features set.
  ASMJIT_INLINE_NODEBUG void reset() noexcept { _data.reset(); }

  //! Adds the given CPU `feature_id` to the list of features.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG void add(Args&&... args) noexcept { return _data.add(std::forward<Args>(args)...); }

  //! Adds the given CPU `feature_id` to the list of features if `condition` is true.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG void add_if(bool condition, Args&&... args) noexcept { return _data.add_if(condition, std::forward<Args>(args)...); }

  //! Removes the given CPU `feature_id` from the list of features.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG void remove(Args&&... args) noexcept { return _data.remove(std::forward<Args>(args)...); }

  //! Tests whether this CPU features matches `other`.
  ASMJIT_INLINE_NODEBUG bool equals(const CpuFeatures& other) const noexcept { return _data.equals(other._data); }

  //! \}
};

//! Describe micro-architectural hints that can be used for optimization purposes and are not part of \ref CpuFeatures.
enum class CpuHints : uint32_t {
  //! No honts.
  kNone = 0x0u,

  //! CPU provides fast 8-bit masked loads and stores.
  kVecMaskedOps8 = 0x00000001u,

  //! CPU provides fast 16-bit masked loads and stores.
  kVecMaskedOps16 = 0x00000002u,

  //! CPU provides fast 32-bit masked loads and stores.
  kVecMaskedOps32 = 0x00000004u,

  //! CPU provides fast 64-bit masked loads and stores.
  kVecMaskedOps64 = 0x00000008u,

  //! CPU provides low-latency 32-bit multiplication (AMD CPUs).
  kVecFastIntMul32 = 0x00000010u,

  //! CPU provides low-latency 64-bit multiplication (AMD CPUs).
  kVecFastIntMul64 = 0x00000020u,

  //! CPU provides fast hardware gathers, which are faster than a sequence of loads and inserts.
  kVecFastGather = 0x00000040u,

  //! CPU has fast stores with mask.
  //!
  //! \note This is a hint to the compiler to emit a masked store instead of a sequence having branches.
  kVecMaskedStore = 0x00000080u
};
ASMJIT_DEFINE_ENUM_FLAGS(CpuHints)

//! CPU information.
class CpuInfo {
public:
  //! \name Members
  //! \{

  //! Architecture.
  Arch _arch {};
  //! Sub-architecture.
  SubArch _sub_arch {};
  //! True if the CPU was detected, false if the detection failed or it's not available.
  bool _was_detected {};
  //! Reserved for future use.
  uint8_t _reserved {};
  //! CPU family ID.
  uint32_t _family_id {};
  //! CPU model ID.
  uint32_t _model_id {};
  //! CPU brand ID.
  uint32_t _brand_id {};
  //! CPU stepping.
  uint32_t _stepping {};
  //! Processor type.
  uint32_t _processor_type {};
  //! Maximum number of addressable IDs for logical processors.
  uint32_t _max_logical_processors {};
  //! Cache line size (in bytes).
  uint32_t _cache_line_size {};
  //! Number of hardware threads.
  uint32_t _hw_thread_count {};

  //! CPU vendor string.
  FixedString<16> _vendor {};
  //! CPU brand string.
  FixedString<64> _brand {};
  //! CPU features.
  CpuFeatures _features {};

  //! CPU hints.
  CpuHints _hints {};

  //! \}

  //! \name Construction & Destruction
  //! \{

  //! Creates a new CpuInfo instance.
  ASMJIT_INLINE_NODEBUG CpuInfo() noexcept {}
  //! Creates a copy of `other` instance.
  ASMJIT_INLINE_NODEBUG CpuInfo(const CpuInfo& other) noexcept = default;

  //! Creates an unitialized `CpuInfo` instance.
  ASMJIT_INLINE_NODEBUG explicit CpuInfo(Globals::NoInit_) noexcept
    : _features(Globals::NoInit) {};

  //! \}

  //! \name CPU Information Detection
  //! \{

  //! Returns the host CPU information.
  //!
  //! \note The returned reference is global - it's setup only once and then shared.
  [[nodiscard]]
  ASMJIT_API static const CpuInfo& host() noexcept;

  //! Updates CPU hints based on the CPU data and features.
  //!
  //! \note This function is called automatically by the CPU detection logic. However, if you change the CPU features
  //! in your own instance of \ref CpuInfo, CPU hints must be updated too, otherwise they would be out of sync.
  ASMJIT_API static CpuHints recalculate_hints(const CpuInfo& info, const CpuFeatures& features) noexcept;

  //! \}

  //! \name Overloaded Operators
  //! \{

  //! Copy assignment.
  ASMJIT_INLINE_NODEBUG CpuInfo& operator=(const CpuInfo& other) noexcept = default;

  //! \}

  //! \name Initialization & Reset
  //! \{

  //! Initializes CpuInfo architecture and sub-architecture members to `arch` and `sub_arch`, respectively.
  ASMJIT_INLINE_NODEBUG void init_arch(Arch arch, SubArch sub_arch = SubArch::kUnknown) noexcept {
    _arch = arch;
    _sub_arch = sub_arch;
  }

  //! Resets this \ref CpuInfo to a default constructed state.
  ASMJIT_INLINE_NODEBUG void reset() noexcept { *this = CpuInfo{}; }

  //! \}

  //! \name Accessors
  //! \{

  //! Returns the CPU architecture this information relates to.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Arch arch() const noexcept { return _arch; }

  //! Returns the CPU sub-architecture this information relates to.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG SubArch sub_arch() const noexcept { return _sub_arch; }

  //! Returns whether the CPU was detected successfully.
  //!
  //! If the returned value is false it means that AsmJit either failed to detect the CPU or it doesn't have
  //! implementation targeting the host architecture and operating system.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool was_detected() const noexcept { return _was_detected; }

  //! Returns the CPU family ID.
  //!
  //! The information provided depends on architecture and OS:
  //!   - X86:
  //!     - Family identifier matches the FamilyId read by using CPUID.
  //!   - ARM:
  //!     - Apple - returns Apple Family identifier returned by sysctlbyname("hw.cpufamily").
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t family_id() const noexcept { return _family_id; }

  //! Returns the CPU model ID.
  //!
  //! The information provided depends on architecture and OS:
  //!   - X86:
  //!     - Model identifier matches the ModelId read by using CPUID.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t model_id() const noexcept { return _model_id; }

  //! Returns the CPU brand id.
  //!
  //! The information provided depends on architecture and OS:
  //!   - X86:
  //!     - Brand identifier matches the BrandId read by using CPUID.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t brand_id() const noexcept { return _brand_id; }

  //! Returns the CPU stepping.
  //!
  //! The information provided depends on architecture and OS:
  //!   - X86:
  //!     - Stepping identifier matches the Stepping information read by using CPUID.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t stepping() const noexcept { return _stepping; }

  //! Returns the processor type.
  //!
  //! The information provided depends on architecture and OS:
  //!   - X86:
  //!     - Processor type identifier matches the ProcessorType read by using CPUID.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t processor_type() const noexcept { return _processor_type; }

  //! Returns the maximum number of logical processors.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t max_logical_processors() const noexcept { return _max_logical_processors; }

  //! Returns the size of a CPU cache line.
  //!
  //! On a multi-architecture system this should return the smallest cache line of all CPUs.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t cache_line_size() const noexcept { return _cache_line_size; }

  //! Returns number of hardware threads available.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t hw_thread_count() const noexcept { return _hw_thread_count; }

  //! Returns a CPU vendor string.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const char* vendor() const noexcept { return _vendor.str; }

  //! Tests whether the CPU vendor string is equal to `s`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_vendor(const char* s) const noexcept { return _vendor.equals(s); }

  //! Returns a CPU brand string.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const char* brand() const noexcept { return _brand.str; }

  //! Returns CPU features.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG CpuFeatures& features() noexcept { return _features; }

  //! Returns CPU features (const).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const CpuFeatures& features() const noexcept { return _features; }

  //! Tests whether the CPU has the given `feature`.
  template<typename FeatureId>
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_feature(const FeatureId& feature_id) const noexcept { return _features.has(feature_id); }

  //! Adds the given CPU `feature_id` to the list of features.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG void add_feature(Args&&... args) noexcept { return _features.add(std::forward<Args>(args)...); }

  //! Removes the given CPU `feature_id` from the list of features.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG void remove_feature(Args&&... args) noexcept { return _features.remove(std::forward<Args>(args)...); }

  //! Returns CPU hints.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG CpuHints hints() const noexcept { return _hints; }

  //! Updates CPU hints based on the CPU data and features.
  //!
  //! \note This function is called automatically by the CPU detection logic. However, if you change the CPU features
  //! in your own instance of \ref CpuInfo, CPU hints must be updated too, otherwise they would be out of sync.
  ASMJIT_INLINE void update_hints() noexcept { _hints = recalculate_hints(*this, _features); }

  //! \}
};

//! \}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_CORE_CPUINFO_H_INCLUDED
