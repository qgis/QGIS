// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_X86_X86OPCODE_P_H_INCLUDED
#define ASMJIT_X86_X86OPCODE_P_H_INCLUDED

#include <asmjit/x86/x86globals.h>

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

//! \cond INTERNAL
//! \addtogroup asmjit_x86
//! \{

//! Helper class to store and manipulate X86 opcodes.
//!
//! The first 8 least significant bits describe the opcode byte as defined in ISA manuals, all other bits
//! describe other properties like prefixes, see `Opcode::Bits` for more information.
struct Opcode {
  uint32_t v;

  //! Describes a meaning of all bits of AsmJit's 32-bit opcode value.
  //!
  //! This schema is AsmJit specific and has been designed to allow encoding of all X86 instructions available. X86,
  //! MMX, and SSE+ instructions always use `MM` and `PP` fields, which are encoded to corresponding prefixes needed
  //! by X86 or SIMD instructions. AVX+ instructions embed `MMMMM` and `PP` fields in a VEX prefix, and AVX-512
  //! instructions embed `MM` and `PP` in EVEX prefix.
  //!
  //! The instruction opcode definition uses 1 or 2 bytes as an opcode value. 1 byte is needed by most of the
  //! instructions, 2 bytes are only used by legacy X87-FPU instructions. This means that a second byte is free to
  //! by used by instructions encoded by using VEX and/or EVEX prefix.
  //!
  //! The fields description:
  //!
  //! - `MM` field is used to encode prefixes needed by the instruction or as a part of VEX/EVEX prefix. Described as
  //!   `mm` and `mmmmm` in instruction manuals.
  //!
  //!   NOTE: Since `MM` field is defined as `mmmmm` (5 bits), but only 2 least significant bits are used by VEX and
  //!   EVEX prefixes, and additional 4th bit is used by XOP prefix, AsmJit uses the 3rd and 5th bit for it's own
  //!   purposes. These bits will probably never be used in future encodings as AVX512 uses only `000mm` from `mmmmm`.
  //!
  //! - `PP` field is used to encode prefixes needed by the instruction or as a part of VEX/EVEX prefix. Described as
  //!   `pp` in instruction manuals.
  //!
  //! - `LL` field is used exclusively by AVX+ and AVX512+ instruction sets. It describes vector size, which is `L.128`
  //!   for XMM register, `L.256` for YMM register, and `L.512` for ZMM register. The `LL` field is omitted in case
  //!   that instruction supports multiple vector lengths, however, if the instruction requires specific `L` value it
  //!   must be specified as a part of the opcode.
  //!
  //!   NOTE: `LL` having value `11` is not defined yet.
  //!
  //! - `W` field is the most complicated. It was added by 64-bit architecture to promote default operation width
  //!   (instructions that perform 32-bit operation by default require to override the width to 64-bit explicitly).
  //!   There is nothing wrong on this, however, some instructions introduced implicit `W` override, for example a
  //!   `cdqe` instruction is basically a `cwde` instruction with overridden `W` (set to 1). There are some others
  //!   in the base X86 instruction set. More recent instruction sets started using `W` field more often:
  //!
  //!   - AVX instructions started using `W` field as an extended opcode for FMA, GATHER, PERM, and other instructions.
  //!     It also uses `W` field to override the default operation width in instructions like `vmovq`.
  //!
  //!   - AVX-512 instructions started using `W` field as an extended opcode for all new instructions. This wouldn't
  //!     have been an issue if the `W` field of AVX-512 have matched AVX, but this is not always the case.
  //!
  //! - `O` field is an extended opcode field (3 bits) embedded in ModR/M BYTE.
  //!
  //! - `CDSHL` and `CDTT` fields describe 'compressed-displacement'. `CDSHL` is defined for each instruction that is
  //!   AVX-512 encodable (EVEX) and contains a base N shift (base shift to perform the calculation). The `CDTT` field
  //!   is derived from instruction specification and describes additional shift to calculate the final `CDSHL` that
  //!   will be used in SIB byte.
  //!
  //! \note Don't reorder any fields here, the shifts and masks were defined carefully to make encoding of X86
  //! instructions fast, especially to construct REX, VEX, and EVEX prefixes in the most efficient way. Changing
  //! values defined by these enums many cause AsmJit to emit invalid binary representations of instructions passed to
  //! `x86::Assembler::_emit`.
  enum Bits : uint32_t {
    // MM & VEX & EVEX & XOP
    // ---------------------
    //
    // Two meanings:
    //  * Part of a legacy opcode (prefixes emitted before the main opcode byte).
    //  * `MMMMM` field in VEX|EVEX|XOP instruction.
    //
    // AVX reserves 5 bits for `MMMMM` field, however AVX instructions only use 2 bits and XOP 3 bits. AVX-512 shrinks
    // `MMMMM` field into `MMM` so it's safe to use [4:3] bits of `MMMMM` field for internal payload.
    //
    // AsmJit divides MMMMM field into this layout:
    //
    // [2:0] - Used to describe 0F, 0F38 and 0F3A legacy prefix bytes and 3 bits of MMMMM field for XOP/AVX/AVX512.
    // [3]   - Required by XOP instructions, so we use this bit also to indicate that this is a XOP opcode.
    // [4]   - Used to force EVEX prefix - this bit is not used by any X86 instruction yet, so AsmJit uses it to
    //         describe EVEX only instructions or sets its bit when user uses InstOptions::kX86_Evex to force EVEX.
    kMM_Shift      = 8,
    kMM_Mask       = 0x1Fu << kMM_Shift,
    kMM_00         = 0x00u << kMM_Shift,
    kMM_0F         = 0x01u << kMM_Shift,
    kMM_0F38       = 0x02u << kMM_Shift,
    kMM_0F3A       = 0x03u << kMM_Shift,   // Described also as XOP.M3 in AMD manuals.
    kMM_0F01       = 0x04u << kMM_Shift,   // AsmJit way to describe 0F01 (never VEX/EVEX).

    kMM_MAP5       = 0x05u << kMM_Shift,   // EVEX.MAP5.
    kMM_MAP6       = 0x06u << kMM_Shift,   // EVEX.MAP6.

    // `XOP` field is only used to force XOP prefix instead of VEX3 prefix. We know XOP encodings always use 0b1000
    // bit of MM field and that no VEX and EVEX instruction use such bit yet, so we can use this bit to force XOP
    // prefix to be emitted instead of VEX3 prefix.
    kMM_XOP08      = 0x08u << kMM_Shift,   // XOP.M8.
    kMM_XOP09      = 0x09u << kMM_Shift,   // XOP.M9.
    kMM_XOP0A      = 0x0Au << kMM_Shift,   // XOP.MA.

    kMM_IsXOP_Shift= kMM_Shift + 3,
    kMM_IsXOP      = kMM_XOP08,

    // NOTE: Force VEX3 allows to force to emit VEX3 instead of VEX2 in some cases (similar to forcing REX prefix).
    // Force EVEX will force emitting EVEX prefix instead of VEX2|VEX3. EVEX-only instructions will have ForceEvex
    // always set, however. instructions that can be encoded by either VEX or EVEX prefix should not have ForceEvex
    // set.
    kMM_ForceEvex  = 0x10u << kMM_Shift,   // Force 4-BYTE EVEX prefix.

    // FPU_2B - Second-Byte of the Opcode used by FPU
    // ----------------------------------------------
    //
    // Second byte opcode. This BYTE is ONLY used by FPU instructions and collides with 3 bits from `MM` and 5 bits
    // from 'CDSHL' and 'CDTT'. It's fine as FPU and AVX512 flags are never used at the same time.
    kFPU_2B_Shift  = 10,
    kFPU_2B_Mask   = 0xFF << kFPU_2B_Shift,

    // CDSHL & CDTT
    // ------------
    //
    // Compressed displacement bits.
    //
    // Each opcode defines the base size (N) shift:
    //   [0]: BYTE  (1 byte).
    //   [1]: WORD  (2 bytes).
    //   [2]: DWORD (4 bytes - float/int32).
    //   [3]: QWORD (8 bytes - double/int64).
    //   [4]: OWORD (16 bytes - used by FV|FVM|M128).
    //
    // Which is then scaled by the instruction's TT (TupleType) into possible:
    //   [5]: YWORD (32 bytes)
    //   [6]: ZWORD (64 bytes)
    //
    // These bits are then adjusted before calling EmitModSib or EmitModVSib.
    kCDSHL_Shift   = 13,
    kCDSHL_Mask    = 0x7u << kCDSHL_Shift,

    kCDSHL__       = 0x0u << kCDSHL_Shift, // Base element size not used.
    kCDSHL_0       = 0x0u << kCDSHL_Shift, // N << 0.
    kCDSHL_1       = 0x1u << kCDSHL_Shift, // N << 1.
    kCDSHL_2       = 0x2u << kCDSHL_Shift, // N << 2.
    kCDSHL_3       = 0x3u << kCDSHL_Shift, // N << 3.
    kCDSHL_4       = 0x4u << kCDSHL_Shift, // N << 4.
    kCDSHL_5       = 0x5u << kCDSHL_Shift, // N << 5.

    // Compressed displacement tuple-type (specific to AsmJit).
    //
    // Since we store the base offset independently of CDTT we can simplify the number of 'TUPLE_TYPE' groups
    // significantly and just handle special cases.
    kCDTT_Shift    = 16,
    kCDTT_Mask     = 0x3u << kCDTT_Shift,
    kCDTT_None     = 0x0u << kCDTT_Shift,  // Does nothing.
    kCDTT_ByLL     = 0x1u << kCDTT_Shift,  // Scales by LL (1x 2x 4x).
    kCDTT_T1W      = 0x2u << kCDTT_Shift,  // Used to add 'W' to the shift.
    kCDTT_DUP      = 0x3u << kCDTT_Shift,  // Special 'VMOVDDUP' case.

    // Aliases that match names used in instruction manuals.
    kCDTT__        = kCDTT_None,
    kCDTT_FV       = kCDTT_ByLL,
    kCDTT_HV       = kCDTT_ByLL,
    kCDTT_QV       = kCDTT_ByLL,
    kCDTT_FVM      = kCDTT_ByLL,
    kCDTT_T1S      = kCDTT_None,
    kCDTT_T1F      = kCDTT_None,
    kCDTT_T2       = kCDTT_None,
    kCDTT_T4       = kCDTT_None,
    kCDTT_T8       = kCDTT_None,
    kCDTT_HVM      = kCDTT_ByLL,
    kCDTT_QVM      = kCDTT_ByLL,
    kCDTT_OVM      = kCDTT_ByLL,
    kCDTT_128      = kCDTT_None,

    // `O` Field in ModR/M (??:xxx:???)
    // --------------------------------

    kModO_Shift    = 18,
    kModO_Mask     = 0x7u << kModO_Shift,

    kModO__        = 0x0u,
    kModO_0        = 0x0u << kModO_Shift,
    kModO_1        = 0x1u << kModO_Shift,
    kModO_2        = 0x2u << kModO_Shift,
    kModO_3        = 0x3u << kModO_Shift,
    kModO_4        = 0x4u << kModO_Shift,
    kModO_5        = 0x5u << kModO_Shift,
    kModO_6        = 0x6u << kModO_Shift,
    kModO_7        = 0x7u << kModO_Shift,

    // `RM` Field in ModR/M (??:???:xxx)
    // ---------------------------------
    //
    // Second data field used by ModR/M byte. This is only used by few instructions that use OPCODE+MOD/RM where both
    // values in Mod/RM are part of the opcode.

    kModRM_Shift    = 13,
    kModRM_Mask     = 0x7u << kModRM_Shift,

    kModRM__        = 0x0u,
    kModRM_0        = 0x0u << kModRM_Shift,
    kModRM_1        = 0x1u << kModRM_Shift,
    kModRM_2        = 0x2u << kModRM_Shift,
    kModRM_3        = 0x3u << kModRM_Shift,
    kModRM_4        = 0x4u << kModRM_Shift,
    kModRM_5        = 0x5u << kModRM_Shift,
    kModRM_6        = 0x6u << kModRM_Shift,
    kModRM_7        = 0x7u << kModRM_Shift,

    // `PP` Field
    // ----------
    //
    // These fields are stored deliberately right after each other as it makes it easier to construct VEX prefix from
    // the opcode value stored in the instruction database.
    //
    // Two meanings:
    //   * "PP" field in AVX/XOP/AVX-512 instruction.
    //   * Mandatory Prefix in legacy encoding.
    //
    // AVX reserves 2 bits for `PP` field, but AsmJit extends the storage by 1 more bit that is used to emit 9B prefix
    // for some X87-FPU instructions.

    kPP_Shift      = 21,
    kPP_VEXMask    = 0x03u << kPP_Shift,   // PP field mask used by VEX/EVEX.
    kPP_FPUMask    = 0x07u << kPP_Shift,   // Mask used by EMIT_PP, also includes '0x9B'.
    kPP_00         = 0x00u << kPP_Shift,
    kPP_66         = 0x01u << kPP_Shift,
    kPP_F3         = 0x02u << kPP_Shift,
    kPP_F2         = 0x03u << kPP_Shift,

    kPP_9B         = 0x07u << kPP_Shift,   // AsmJit specific to emit FPU's '9B' byte.

    // REX|VEX|EVEX B|X|R|W Bits
    // -------------------------
    //
    // NOTE: REX.[B|X|R] are never stored within the opcode itself, they are reserved by AsmJit and are added
    // dynamically to the opcode to represent [REX|VEX|EVEX].[B|X|R] bits. REX.W can be stored in DB as it's sometimes
    // part of the opcode itself.

    // These must be binary compatible with instruction options.
    kREX_Shift     = 24,
    kREX_Mask      = 0x0Fu << kREX_Shift,
    kB             = 0x01u << kREX_Shift,  // Never stored in DB, used by encoder.
    kX             = 0x02u << kREX_Shift,  // Never stored in DB, used by encoder.
    kR             = 0x04u << kREX_Shift,  // Never stored in DB, used by encoder.
    kW             = 0x08u << kREX_Shift,
    kW_Shift       = kREX_Shift + 3,

    kW__           = 0u << kW_Shift,       // REX.W/VEX.W is unspecified.
    kW_x           = 0u << kW_Shift,       // REX.W/VEX.W is based on instruction operands.
    kW_I           = 0u << kW_Shift,       // REX.W/VEX.W is ignored (WIG).
    kW_0           = 0u << kW_Shift,       // REX.W/VEX.W is 0 (W0).
    kW_1           = 1u << kW_Shift,       // REX.W/VEX.W is 1 (W1).

    // EVEX.W Field
    // ------------
    //
    // `W` field used by EVEX instruction encoding.

    kEvex_W_Shift  = 28,
    kEvex_W_Mask   = 1u << kEvex_W_Shift,

    kEvex_W__      = 0u << kEvex_W_Shift,  // EVEX.W is unspecified (not EVEX instruction).
    kEvex_W_x      = 0u << kEvex_W_Shift,  // EVEX.W is based on instruction operands.
    kEvex_W_I      = 0u << kEvex_W_Shift,  // EVEX.W is ignored (WIG).
    kEvex_W_0      = 0u << kEvex_W_Shift,  // EVEX.W is 0 (W0).
    kEvex_W_1      = 1u << kEvex_W_Shift,  // EVEX.W is 1 (W1).

    // `L` or `LL` field in AVX/XOP/AVX-512
    // ------------------------------------
    //
    // VEX/XOP prefix can only use the first bit `L.128` or `L.256`. EVEX prefix makes it possible to use also
    // `L.512`. If the instruction set manual describes an instruction by `LIG` it means that the `L` field is ignored
    // and AsmJit defaults to `0` in such case.
    kLL_Shift      = 29,
    kLL_Mask       = 0x3u << kLL_Shift,

    kLL__          = 0x0u << kLL_Shift,    // LL is unspecified.
    kLL_x          = 0x0u << kLL_Shift,    // LL is based on instruction operands.
    kLL_I          = 0x0u << kLL_Shift,    // LL is ignored (LIG).
    kLL_0          = 0x0u << kLL_Shift,    // LL is 0 (L.128).
    kLL_1          = 0x1u << kLL_Shift,    // LL is 1 (L.256).
    kLL_2          = 0x2u << kLL_Shift,    // LL is 2 (L.512).

    // Opcode Combinations
    // -------------------

    k0      = 0,                           // '__' (no prefix, used internally).
    k000000 = kPP_00 | kMM_00,             // '__' (no prefix, to be the same width as others).
    k000F00 = kPP_00 | kMM_0F,             // '0F'
    k000F01 = kPP_00 | kMM_0F01,           // '0F01'
    k000F0F = kPP_00 | kMM_0F,             // '0F0F' - 3DNOW, equal to 0x0F, must have special encoding to take effect.
    k000F38 = kPP_00 | kMM_0F38,           // 'NP.0F38'
    k000F3A = kPP_00 | kMM_0F3A,           // 'NP.0F3A'
    k00MAP5 = kPP_00 | kMM_MAP5,           // 'NP.MAP5'
    k00MAP6 = kPP_00 | kMM_MAP6,           // 'NP.MAP5'
    k660000 = kPP_66 | kMM_00,             // '66'
    k660F00 = kPP_66 | kMM_0F,             // '66.0F'
    k660F01 = kPP_66 | kMM_0F01,           // '66.0F01'
    k660F38 = kPP_66 | kMM_0F38,           // '66.0F38'
    k660F3A = kPP_66 | kMM_0F3A,           // '66.0F3A'
    k66MAP5 = kPP_66 | kMM_MAP5,           // '66.MAP5'
    k66MAP6 = kPP_66 | kMM_MAP6,           // '66.MAP5'
    kF20000 = kPP_F2 | kMM_00,             // 'F2'
    kF20F00 = kPP_F2 | kMM_0F,             // 'F2.0F'
    kF20F01 = kPP_F2 | kMM_0F01,           // 'F2.0F01'
    kF20F38 = kPP_F2 | kMM_0F38,           // 'F2.0F38'
    kF20F3A = kPP_F2 | kMM_0F3A,           // 'F2.0F3A'
    kF2MAP5 = kPP_F2 | kMM_MAP5,           // 'F2.MAP5'
    kF2MAP6 = kPP_F2 | kMM_MAP6,           // 'F2.MAP5'
    kF30000 = kPP_F3 | kMM_00,             // 'F3'
    kF30F00 = kPP_F3 | kMM_0F,             // 'F3.0F'
    kF30F01 = kPP_F3 | kMM_0F01,           // 'F3.0F01'
    kF30F38 = kPP_F3 | kMM_0F38,           // 'F3.0F38'
    kF30F3A = kPP_F3 | kMM_0F3A,           // 'F3.0F3A'
    kF3MAP5 = kPP_F3 | kMM_MAP5,           // 'F3.MAP5'
    kF3MAP6 = kPP_F3 | kMM_MAP6,           // 'F3.MAP5'
    kFPU_00 = kPP_00 | kMM_00,             // '__' (FPU)
    kFPU_9B = kPP_9B | kMM_00,             // '9B' (FPU)
    kXOP_M8 = kPP_00 | kMM_XOP08,          // 'M8' (XOP)
    kXOP_M9 = kPP_00 | kMM_XOP09,          // 'M9' (XOP)
    kXOP_MA = kPP_00 | kMM_XOP0A           // 'MA' (XOP)
  };

  // Opcode Builder
  // --------------

  ASMJIT_INLINE_NODEBUG uint32_t get() const noexcept { return v; }

  ASMJIT_INLINE_NODEBUG bool has_w() const noexcept { return (v & kW) != 0; }
  ASMJIT_INLINE_NODEBUG bool has_66h() const noexcept { return (v & kPP_66) != 0; }

  ASMJIT_INLINE_NODEBUG Opcode& add(uint32_t x) noexcept { return operator+=(x); }

  ASMJIT_INLINE_NODEBUG Opcode& add_66h() noexcept { return operator|=(kPP_66); }
  template<typename T>
  ASMJIT_INLINE_NODEBUG Opcode& add_66h_if(T exp) noexcept { return operator|=(uint32_t(exp) << kPP_Shift); }
  template<typename T>
  ASMJIT_INLINE_NODEBUG Opcode& add_66h_by_size(T size) noexcept { return add_66h_if(size == 2); }

  ASMJIT_INLINE_NODEBUG Opcode& add_w() noexcept { return operator|=(kW); }
  template<typename T>
  ASMJIT_INLINE_NODEBUG Opcode& add_w_if(T exp) noexcept { return operator|=(uint32_t(exp) << kW_Shift); }
  template<typename T>
  ASMJIT_INLINE_NODEBUG Opcode& add_w_by_size(T size) noexcept { return add_w_if(size == 8); }

  template<typename T>
  ASMJIT_INLINE_NODEBUG Opcode& add_prefix_by_size(T size) noexcept {
    static constexpr uint32_t mask[16] = {
      0,          // #0
      0,          // #1 -> nothing (already handled or not possible)
      kPP_66,     // #2 -> 66H
      0,          // #3
      0,          // #4 -> nothing
      0,          // #5
      0,          // #6
      0,          // #7
      kW          // #8 -> REX.W
    };
    return operator|=(mask[size & 0xF]);
  }

  template<typename T>
  ASMJIT_INLINE_NODEBUG Opcode& add_arith_by_size(T size) noexcept {
    static const uint32_t mask[16] = {
      0,          // #0
      0,          // #1 -> nothing
      1 | kPP_66, // #2 -> NOT_BYTE_OP(1) and 66H
      0,          // #3
      1,          // #4 -> NOT_BYTE_OP(1)
      0,          // #5
      0,          // #6
      0,          // #7
      1 | kW      // #8 -> NOT_BYTE_OP(1) and REX.W
    };
    return operator|=(mask[size & 0xF]);
  }

  ASMJIT_INLINE_NODEBUG Opcode& force_evex() noexcept { return operator|=(kMM_ForceEvex); }
  template<typename T>
  ASMJIT_INLINE_NODEBUG Opcode& force_evex_if(T exp) noexcept { return operator|=(uint32_t(exp) << Support::ctz_const<uint32_t(kMM_ForceEvex)>); }

  //! Extract `O` field (R) from the opcode (specified as /0..7 in instruction manuals).
  ASMJIT_INLINE_NODEBUG uint32_t extract_mod_o() const noexcept {
    return (v >> kModO_Shift) & 0x07;
  }

  //! Extract `RM` field (RM) from the opcode (usually specified as another opcode value).
  ASMJIT_INLINE_NODEBUG uint32_t extract_mod_rm() const noexcept {
    return (v >> kModRM_Shift) & 0x07;
  }

  //! Extract `REX` prefix from opcode combined with `options`.
  ASMJIT_INLINE_NODEBUG uint32_t extract_rex(InstOptions options) const noexcept {
    // kREX was designed in a way that when shifted there will be no bytes set except REX.[B|X|R|W].
    // The returned value forms a real REX prefix byte. This case should be unit-tested as well.
    return (v | uint32_t(options)) >> kREX_Shift;
  }

  ASMJIT_INLINE_NODEBUG uint32_t extract_ll_mmmmm(InstOptions options) const noexcept {
    uint32_t ll_mmmmm = uint32_t(v & (kLL_Mask | kMM_Mask));
    uint32_t vex_evex = uint32_t(options & InstOptions::kX86_Evex);
    return (ll_mmmmm | vex_evex) >> kMM_Shift;
  }

  ASMJIT_INLINE_NODEBUG Opcode& operator=(uint32_t x) noexcept { v = x; return *this; }
  ASMJIT_INLINE_NODEBUG Opcode& operator+=(uint32_t x) noexcept { v += x; return *this; }
  ASMJIT_INLINE_NODEBUG Opcode& operator-=(uint32_t x) noexcept { v -= x; return *this; }
  ASMJIT_INLINE_NODEBUG Opcode& operator&=(uint32_t x) noexcept { v &= x; return *this; }
  ASMJIT_INLINE_NODEBUG Opcode& operator|=(uint32_t x) noexcept { v |= x; return *this; }
  ASMJIT_INLINE_NODEBUG Opcode& operator^=(uint32_t x) noexcept { v ^= x; return *this; }

  ASMJIT_INLINE_NODEBUG uint32_t operator&(uint32_t x) const noexcept { return v & x; }
  ASMJIT_INLINE_NODEBUG uint32_t operator|(uint32_t x) const noexcept { return v | x; }
  ASMJIT_INLINE_NODEBUG uint32_t operator^(uint32_t x) const noexcept { return v ^ x; }
  ASMJIT_INLINE_NODEBUG uint32_t operator<<(uint32_t x) const noexcept { return v << x; }
  ASMJIT_INLINE_NODEBUG uint32_t operator>>(uint32_t x) const noexcept { return v >> x; }
};

//! \}
//! \endcond

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_X86_X86OPCODE_P_H_INCLUDED
