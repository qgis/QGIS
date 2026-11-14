// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_TABLES_TABLES_P_H_INCLUDED
#define BLEND2D_TABLES_TABLES_P_H_INCLUDED

#include <blend2d/support/lookuptable_p.h>

#if !defined(BL_BUILD_NO_JIT)
#include <asmjit/ujit/vecconsttable.h>
#endif // !BL_BUILD_NO_JIT

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

namespace bl {

//! Table, which provides bit count for 8-bit quantities.
BL_HIDDEN extern const LookupTable<uint8_t, 256> bit_count_byte_table;

//! Table that contains precomputed `{1..16} % N`.
struct BL_ALIGN_TYPE(ModuloTable, 16) {
  uint8_t x1_16[16];
};

BL_HIDDEN extern const ModuloTable modulo_table[18];

template<typename T>
struct BL_MAY_ALIAS BL_ALIGN_TYPE(VecConst64, 8) {
  T data[8 / sizeof(T)];
};

template<typename T>
struct BL_MAY_ALIAS BL_ALIGN_TYPE(VecConst128, 16) {
  T data[16 / sizeof(T)];

  template<typename VecType>
  BL_INLINE const VecType& as() const noexcept { return *reinterpret_cast<const VecType*>(this); }
};

template<typename T>
struct BL_MAY_ALIAS BL_ALIGN_TYPE(VecConst256, 32) {
  T data[32 / sizeof(T)];

  template<typename VecType>
  BL_INLINE const VecType& as() const noexcept { return *reinterpret_cast<const VecType*>(this); }
};

template<typename T>
struct BL_MAY_ALIAS BL_ALIGN_TYPE(VecConst512, 64) {
  T data[64 / sizeof(T)];

  template<typename VecType>
  BL_INLINE const VecType& as() const noexcept { return *reinterpret_cast<const VecType*>(this); }
};

#if BL_TARGET_ARCH_X86
template<typename T>
using VecConstNative = VecConst256<T>;
#else
template<typename T>
using VecConstNative = VecConst128<T>;
#endif

#define REPEAT_2X(...) __VA_ARGS__, __VA_ARGS__
#define REPEAT_4X(...) REPEAT_2X(__VA_ARGS__), REPEAT_2X(__VA_ARGS__)
#define REPEAT_8X(...) REPEAT_4X(__VA_ARGS__), REPEAT_4X(__VA_ARGS__)
#define REPEAT_16X(...) REPEAT_8X(__VA_ARGS__), REPEAT_8X(__VA_ARGS__)

#if BL_TARGET_ARCH_X86
#define REPEAT_32B(...) REPEAT_8X(__VA_ARGS__)
#define REPEAT_64B(...) REPEAT_4X(__VA_ARGS__)
#define REPEAT_128B(...) REPEAT_2X(__VA_ARGS__)
#else
#define REPEAT_32B(...) REPEAT_4X(__VA_ARGS__)
#define REPEAT_64B(...) REPEAT_2X(__VA_ARGS__)
#define REPEAT_128B(...) __VA_ARGS__
#endif

//! Common table that contains constants used across Blend2D library, but most importantly in pipelines (either static
//! or dynamic. The advantage of this table is that it contains all constants that SIMD code (or also a generic code)
//! requires so only one register (pointer) is required to address all of them in either static or generated pipelines.
struct BL_ALIGN_TYPE(CommonTable, 64)
#if !defined(BL_BUILD_NO_JIT)
  : public asmjit::ujit::VecConstTable
#endif
{
#if defined(BL_BUILD_NO_JIT)
  //! \name Constants otherwise provided by UJIT
  //! \{

  VecConstNative<uint64_t> p_0000000000000000 {{ REPEAT_64B(0x0000000000000000u) }};
  VecConstNative<uint64_t> p_8080808080808080 {{ REPEAT_64B(0x8080808080808080u) }};
  VecConstNative<uint64_t> p_8000800080008000 {{ REPEAT_64B(0x8000800080008000u) }};
  VecConstNative<uint64_t> p_8000000080000000 {{ REPEAT_64B(0x8000000080000000u) }};
  VecConstNative<uint64_t> p_8000000000000000 {{ REPEAT_64B(0x8000000000000000u) }};

  VecConstNative<uint64_t> p_7FFFFFFF7FFFFFFF {{ REPEAT_64B(0x7FFFFFFF7FFFFFFFu) }};
  VecConstNative<uint64_t> p_7FFFFFFFFFFFFFFF {{ REPEAT_64B(0x7FFFFFFFFFFFFFFFu) }};

  VecConstNative<uint64_t> p_0F0F0F0F0F0F0F0F {{ REPEAT_64B(0x0F0F0F0F0F0F0F0Fu) }};
  VecConstNative<uint64_t> p_1010101010101010 {{ REPEAT_64B(0x1010101010101010u) }};

  VecConstNative<uint64_t> p_00FF00FF00FF00FF {{ REPEAT_64B(0x00FF00FF00FF00FFu) }};
  VecConstNative<uint64_t> p_0100010001000100 {{ REPEAT_64B(0x0100010001000100u) }};
  VecConstNative<uint64_t> p_01FF01FF01FF01FF {{ REPEAT_64B(0x01FF01FF01FF01FFu) }};

  VecConstNative<uint64_t> p_FFFFFFFF00000000 {{ REPEAT_64B(0xFFFFFFFF00000000u) }};

  VecConstNative<float> f32_1 {{ REPEAT_32B(1.0f) }};
  VecConstNative<float> f32_round_magic {{ REPEAT_32B(8388608.0f) }};

  VecConstNative<double> f64_1 {{ REPEAT_64B(1.0) }};
  VecConstNative<double> f64_round_magic {{ REPEAT_64B(4503599627370496.0) }};

#endif

  //! }

  //! \name 128-bit and 256-bit Constants
  //!
  //! These constants are shared between 128-bit and 256-bit code paths.
  //!
  //! \{

  VecConstNative<uint64_t> p_007F007F007F007F {{ REPEAT_64B(0x007F007F007F007Fu) }};
  VecConstNative<uint64_t> p_0080008000800080 {{ REPEAT_64B(0x0080008000800080u) }};
  VecConstNative<uint64_t> p_0101010101010101 {{ REPEAT_64B(0x0101010101010101u) }};
  VecConstNative<uint64_t> p_0200020002000200 {{ REPEAT_64B(0x0200020002000200u) }};

  VecConstNative<uint64_t> p_3030303030303030 {{ REPEAT_64B(0x3030303030303030u) }};

  VecConstNative<uint64_t> p_0000010000000100 {{ REPEAT_64B(0x0000010000000100u) }};
  VecConstNative<uint64_t> p_0000020000000200 {{ REPEAT_64B(0x0000020000000200u) }};
  VecConstNative<uint64_t> p_0002000000020000 {{ REPEAT_64B(0x0002000000020000u) }}; // 256 << 9
  VecConstNative<uint64_t> p_00FFFFFF00FFFFFF {{ REPEAT_64B(0x00FFFFFF00FFFFFFu) }};
  VecConstNative<uint64_t> p_0101000001010000 {{ REPEAT_64B(0x0101000001010000u) }};
  VecConstNative<uint64_t> p_FF000000FF000000 {{ REPEAT_64B(0xFF000000FF000000u) }};
  VecConstNative<uint64_t> p_FFFF0000FFFF0000 {{ REPEAT_64B(0xFFFF0000FFFF0000u) }};

  VecConstNative<uint64_t> p_000000FF00FF00FF {{ REPEAT_64B(0x000000FF00FF00FFu) }};
  VecConstNative<uint64_t> p_0000800000000000 {{ REPEAT_64B(0x0000800000000000u) }};
  VecConstNative<uint64_t> p_0000FFFFFFFFFFFF {{ REPEAT_64B(0x0000FFFFFFFFFFFFu) }};
  VecConstNative<uint64_t> p_00FF000000000000 {{ REPEAT_64B(0x00FF000000000000u) }};
  VecConstNative<uint64_t> p_0101010100000000 {{ REPEAT_64B(0x0101010100000000u) }};
  VecConstNative<uint64_t> p_FFFF000000000000 {{ REPEAT_64B(0xFFFF000000000000u) }};

  VecConstNative<uint32_t> p_FFFFFFFF_FFFFFFFF_FFFFFFFF_0 {{ REPEAT_128B(0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0) }};

  VecConstNative<uint32_t> u32_0_1_2_3 {{ REPEAT_128B(0, 1, 2, 3) }};
  VecConstNative<uint32_t> u32_4_4_4_4 {{ REPEAT_128B(4, 4, 4, 4) }};

  // Vector of `4.0f`.
  VecConstNative<float> f32_4 {{ REPEAT_32B(4.0f) }};
  // Vector of `8.0f`.
  VecConstNative<float> f32_8 {{ REPEAT_32B(8.0f) }};
  // Vector of `16.0f`.
  VecConstNative<float> f32_16 {{ REPEAT_32B(16.0f) }};
  // Vector of `255.0f`.
  VecConstNative<float> f32_255 {{ REPEAT_32B(255.0f) }};
  // Vector of `1e-3`.
  VecConstNative<float> f32_1e_m3 {{ REPEAT_32B(1e-3f) }};
  // Vector of `1e-20`.
  VecConstNative<float> f32_1e_m20 {{ REPEAT_32B(1e-20f) }};
  // Vector of `1.0f / 255.0f`.
  VecConstNative<float> f32_1div255 {{ REPEAT_32B(1.0f / 255.0f) }};
  // Vector of `[15f, 14f, 13f, 12f, 11f, 10f, 9f, 8f, 7f, 6f, 5f, 4f, 3f, 2f, 1f, 0f]`.
  VecConst512<float> f32_increments {{ 0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f }};

  // Vector of `4.0`.
  VecConstNative<double> f64_4 {{ REPEAT_64B(4.0) }};
  // Vector of `1e-20`.
  VecConstNative<double> f64_1e_m20 {{ REPEAT_64B(1e-20) }};
  // Vector of `-1.0`.
  VecConstNative<double> f64_m1 {{ REPEAT_64B(-1.0) }};

  // Vector of `[4.0, 8.0]`.
  VecConstNative<double> f64_4_8 {{ REPEAT_128B(4.0, 8.0) }};
  // Vector of `[8.0, 4.0]`.
  VecConstNative<double> f64_8_4 {{ REPEAT_128B(8.0, 4.0) }};

  //! \}

  //! \name 128-bit and 256-bit VPSHUFB (X86) and TBL (ARM) predicates.
  //! \{

  VecConstNative<uint64_t> swizu8_xxxxxxxx1xxx0xxx_to_z1z1z1z1z0z0z0z0 {{ REPEAT_128B(0xff03ff03ff03ff03u, 0xff07ff07ff07ff07u) }};
  VecConstNative<uint64_t> swizu8_xxxxxxx1xxxxxxx0_to_zzzzzzzz11110000 {{ REPEAT_128B(0x0808080800000000u, 0xffffffffffffffffu) }};
  VecConstNative<uint64_t> swizu8_xxxxxxx1xxxxxxx0_to_z1z1z1z1z0z0z0z0 {{ REPEAT_128B(0xff00ff00ff00ff00u, 0xff08ff08ff08ff08u) }};
  VecConstNative<uint64_t> swizu8_xxx3xxx2xxx1xxx0_to_3210321032103210 {{ REPEAT_128B(0x0C0804000C080400u, 0x0C0804000C080400u) }};
  VecConstNative<uint64_t> swizu8_xxx3xxx2xxx1xxx0_to_3333222211110000 {{ REPEAT_128B(0x0404040400000000u, 0x0C0C0C0C08080808u) }};
  VecConstNative<uint64_t> swizu8_xxx3xxx2xxx1xxx0_to_z3z3z2z2z1z1z0z0 {{ REPEAT_128B(0xff04ff04ff00ff00u, 0xff0Cff0Cff08ff08u) }};
  VecConstNative<uint64_t> swizu8_xxxxxxxxx3x2x1x0_to_3333222211110000 {{ REPEAT_128B(0x0202020200000000u, 0x0606060604040404u) }};
  VecConstNative<uint64_t> swizu8_xxxxxxxxxxxxxx10_to_z1z1z1z1z0z0z0z0 {{ REPEAT_128B(0xff00ff00ff00ff00u, 0xff01ff01ff01ff01u) }};
  VecConstNative<uint64_t> swizu8_xx76xx54xx32xx10_to_7654321076543210 {{ REPEAT_128B(0x0D0C090805040100u, 0x0D0C090805040100u) }};
  VecConstNative<uint64_t> swizu8_1xxx0xxxxxxxxxxx_to_z1z1z1z1z0z0z0z0 {{ REPEAT_128B(0xff0Bff0Bff0Bff0Bu, 0xff0ffF0ffF0ffF0Fu) }};
  VecConstNative<uint64_t> swizu8_3xxx2xxx1xxx0xxx_to_zzzzzzzzzzzz3210 {{ REPEAT_128B(0xffffffff0F0B0703u, 0xffffffffffffffffu) }};
  VecConstNative<uint64_t> swizu8_3xxx2xxx1xxx0xxx_to_3333222211110000 {{ REPEAT_128B(0x0707070703030303u, 0x0F0F0F0F0B0B0B0Bu) }};
  VecConstNative<uint64_t> swizu8_32xxxxxx10xxxxxx_to_3232323210101010 {{ REPEAT_128B(0x0706070607060706u, 0x0F0E0F0E0F0E0F0Eu) }};
  VecConstNative<uint64_t> swizu8_x1xxxxxxx0xxxxxx_to_1111000011110000 {{ REPEAT_128B(0x0E0E0E0E06060606u, 0x0E0E0E0E06060606u) }};
  VecConstNative<uint64_t> swizu8_76543210xxxxxxxx_to_z7z6z5z4z3z2z1z0 {{ REPEAT_128B(0xff0Bff0Aff09ff08u, 0xff0ffF0Eff0Dff0Cu) }};

  VecConstNative<uint64_t> swizu8_xxxxxxxxxxxx3210_to_3333222211110000 {{ REPEAT_128B(0x0101010100000000u, 0x0303030302020202u) }};
  VecConstNative<uint64_t> swizu8_xxxxxxxx3210xxxx_to_3333222211110000 {{ REPEAT_128B(0x0505050504040404u, 0x0707070706060606u) }};
  VecConstNative<uint64_t> swizu8_xxxx3210xxxxxxxx_to_3333222211110000 {{ REPEAT_128B(0x0909090908080808u, 0x0B0B0B0B0A0A0A0Au) }};
  VecConstNative<uint64_t> swizu8_3210xxxxxxxxxxxx_to_3333222211110000 {{ REPEAT_128B(0x0D0D0D0D0C0C0C0Cu, 0x0F0F0F0F0E0E0E0Eu) }};

  VecConstNative<uint64_t> swizu8_xxxx1xxxxxxx0xxx_to_z1z1z1z1z0z0z0z0 {{ REPEAT_128B(0xff03ff03ff03ff03u, 0xff0Bff0Bff0Bff0Bu) }};

#if BL_TARGET_ARCH_X86

  VecConst512<uint64_t> permu8_a8_to_rgba32_pc {{
    0x0101010100000000u, 0x0303030302020202u,
    0x0505050504040404u, 0x0707070706060606u,
    0x0909090908080808u, 0x0B0B0B0B0A0A0A0Au,
    0x0D0D0D0D0C0C0C0Cu, 0x0F0F0F0F0E0E0E0Eu
  }};

  VecConst512<uint64_t> permu8_a8_to_rgba32_pc_second {{
    0x1111111110101010u, 0x1313131312121212u,
    0x1515151514141414u, 0x1717171716161616u,
    0x1919191918181818u, 0x1B1B1B1B1A1A1A1Au,
    0x1D1D1D1D1C1C1C1Cu, 0x1F1F1F1F1E1E1E1Eu
  }};

  VecConst512<uint64_t> permu8_a8_to_rgba32_uc {{
    0xff00ff00ff00ff00u, 0xff01ff01ff01ff01u,
    0xff02ff02ff02ff02u, 0xff03ff03ff03ff03u,
    0xff04ff04ff04ff04u, 0xff05ff05ff05ff05u,
    0xff06ff06ff06ff06u, 0xff07ff07ff07ff07u
  }};

  VecConst512<uint64_t> permu8_4xa8_lo_to_rgba32_uc {{
    0x0100010001000100u, 0x0302030203020302u,
    0x0504050405040504u, 0x0706070607060706u,
    0x1110111011101110u, 0x1312131213121312u,
    0x1514151415141514u, 0x1716171617161716u
  }};

  VecConst512<uint64_t> permu8_4xu8_lo_to_rgba32_uc {{
    0x0100010001000100u, 0x0302030203020302u,
    0x0504050405040504u, 0x0706070607060706u,
    0x0908090809080908u, 0x0B0A0B0A0B0A0B0Au,
    0x0D0C0D0C0D0C0D0Cu, 0x0F0E0F0E0F0E0F0Eu
  }};

  VecConst512<uint64_t> permu8_pc_to_pa {{
    0x1C1814100C080400u, 0x3C3834302C282420u,
    0x5C5854504C484440u, 0x7C7874706C686460u,
    0xffffffffffffffffu, 0xffffffffffffffffu,
    0xffffffffffffffffu, 0xffffffffffffffffu
  }};

  VecConst512<uint16_t> permu16_pc_to_ua {{
    1 ,  3,  5,  7,  9, 11, 13, 15,
    17, 19, 21, 23, 25, 27, 29, 31,
    33, 35, 37, 39, 41, 43, 45, 47,
    49, 51, 53, 55, 57, 59, 61, 63
  }};

  VecConst512<uint64_t> swizu8_dither_rgba64_lo {{
    0xffffff00ff00ff00u, 0xffffff01ff01ff01u,
    0xffffff02ff02ff02u, 0xffffff03ff03ff03u,
    0xffffff04ff04ff04u, 0xffffff05ff05ff05u,
    0xffffff06ff06ff06u, 0xffffff07ff07ff07u
  }};

  VecConst512<uint64_t> swizu8_dither_rgba64_hi {{
    0xffffff08ff08ff08u, 0xffffff09ff09ff09u,
    0xffffff0Aff0Aff0Au, 0xffffff0Bff0Bff0Bu,
    0xffffff0Cff0Cff0Cu, 0xffffff0Dff0Dff0Du,
    0xffffff0Eff0Eff0Eu, 0xffffff0Fff0Fff0Fu
  }};

  VecConst256<uint32_t> permu32_fix_2x_pack_avx2 {{
    0, 4, 1, 5, 2, 6, 3, 7,
  }};

#else

  VecConst128<uint64_t> swizu8_dither_rgba64_lo {{ 0xffffff00ff00ff00u, 0xffffff01ff01ff01u }};
  VecConst128<uint64_t> swizu8_dither_rgba64_hi {{ 0xffffff08ff08ff08u, 0xffffff09ff09ff09u }};

#endif // BL_TARGET_ARCH_X86

  VecConst128<uint64_t> swizu8_rotate_right[8] {
    {{ 0x0706050403020100u, 0x0F0E0D0C0B0A0908u }},
    {{ 0x0706050403020100u, 0x0F0E0D0C0B0A0908u }},
    {{ 0x0706050403020100u, 0x0F0E0D0C0B0A0908u }},
    {{ 0x0706050403020100u, 0x0F0E0D0C0B0A0908u }},
    {{ 0x0706050403020100u, 0x0F0E0D0C0B0A0908u }},
    {{ 0x0706050403020100u, 0x0F0E0D0C0B0A0908u }},
    {{ 0x0706050403020100u, 0x0F0E0D0C0B0A0908u }},
    {{ 0x0706050403020100u, 0x0F0E0D0C0B0A0908u }}
  };

  //! \}

  //! \name Load / Store Masks for VPMASKMOV Instruction (X86 specific)
  //! \{

#if BL_TARGET_ARCH_X86

  // 16-bit masks for use with AVX-512 {k} registers.
  //
  // NOTE: It's better to calculate the mask if the number of elements is greater than 16.

  uint64_t k_msk64_data[64 + 1] = {
    0x0000000000000000u,
    0x0000000000000001u, 0x0000000000000003u, 0x0000000000000007u, 0x000000000000000Fu,
    0x000000000000001Fu, 0x000000000000003Fu, 0x000000000000007Fu, 0x00000000000000FFu,
    0x00000000000001FFu, 0x00000000000003FFu, 0x00000000000007FFu, 0x0000000000000FFFu,
    0x0000000000001FFFu, 0x0000000000003FFFu, 0x0000000000007FFFu, 0x000000000000FFFFu,
    0x000000000001FFFFu, 0x000000000003FFFFu, 0x000000000007FFFFu, 0x00000000000FFFFFu,
    0x00000000001FFFFFu, 0x00000000003FFFFFu, 0x00000000007FFFFFu, 0x0000000000FFFFFFu,
    0x0000000001FFFFFFu, 0x0000000003FFFFFFu, 0x0000000007FFFFFFu, 0x000000000FFFFFFFu,
    0x000000001FFFFFFFu, 0x000000003FFFFFFFu, 0x000000007FFFFFFFu, 0x00000000FFFFFFFFu,
    0x00000001FFFFFFFFu, 0x00000003FFFFFFFFu, 0x00000007FFFFFFFFu, 0x0000000FFFFFFFFFu,
    0x0000001FFFFFFFFFu, 0x0000003FFFFFFFFFu, 0x0000007FFFFFFFFFu, 0x000000FFFFFFFFFFu,
    0x000001FFFFFFFFFFu, 0x000003FFFFFFFFFFu, 0x000007FFFFFFFFFFu, 0x00000FFFFFFFFFFFu,
    0x00001FFFFFFFFFFFu, 0x00003FFFFFFFFFFFu, 0x00007FFFFFFFFFFFu, 0x0000FFFFFFFFFFFFu,
    0x0001FFFFFFFFFFFFu, 0x0003FFFFFFFFFFFFu, 0x0007FFFFFFFFFFFFu, 0x000FFFFFFFFFFFFFu,
    0x001FFFFFFFFFFFFFu, 0x003FFFFFFFFFFFFFu, 0x007FFFFFFFFFFFFFu, 0x00FFFFFFFFFFFFFFu,
    0x01FFFFFFFFFFFFFFu, 0x03FFFFFFFFFFFFFFu, 0x07FFFFFFFFFFFFFFu, 0x0FFFFFFFFFFFFFFFu,
    0x1FFFFFFFFFFFFFFFu, 0x3FFFFFFFFFFFFFFFu, 0x7FFFFFFFFFFFFFFFu, 0xFFFFFFFFFFFFFFFFu
  };

  // NOTE: Use VPMOVSXBD to extend BYTEs to DWORDs or VPMOVSXBQ to extend BYTEs to QWORDs to
  // extend the mask into the correct data size. VPMOVSX?? is not an expensive instruction.

  VecConst64<uint64_t> loadstore_msk8_data[32 + 8 + 32 + 1] = {
    {{ 0x0000000000000000u }}, // [0]
    {{ 0x0000000000000000u }},
    {{ 0x0000000000000000u }},
    {{ 0x0000000000000000u }},
    {{ 0x0000000000000000u }},
    {{ 0x0000000000000000u }},
    {{ 0x0000000000000000u }},
    {{ 0x0000000000000000u }},
    {{ 0x0000000000000000u }}, // [8]
    {{ 0x0000000000000000u }},
    {{ 0x0000000000000000u }},
    {{ 0x0000000000000000u }},
    {{ 0x0000000000000000u }},
    {{ 0x0000000000000000u }},
    {{ 0x0000000000000000u }},
    {{ 0x0000000000000000u }},
    {{ 0x0000000000000000u }}, // [16]
    {{ 0x0000000000000000u }},
    {{ 0x0000000000000000u }},
    {{ 0x0000000000000000u }},
    {{ 0x0000000000000000u }},
    {{ 0x0000000000000000u }},
    {{ 0x0000000000000000u }},
    {{ 0x0000000000000000u }},
    {{ 0x0000000000000000u }}, // [24]
    {{ 0x0000000000000000u }}, //
    {{ 0x0000000000000000u }}, //
    {{ 0x0000000000000000u }}, //
    {{ 0x0000000000000000u }}, //
    {{ 0x0000000000000000u }}, //
    {{ 0x0000000000000000u }}, //
    {{ 0x0000000000000000u }}, //
    {{ 0x0000000000000000u }}, // [32]
    {{ 0x00000000000000FFu }}, //
    {{ 0x000000000000FFFFu }}, //
    {{ 0x0000000000FFFFFFu }}, //
    {{ 0x00000000FFFFFFFFu }}, //
    {{ 0x000000FFFFFFFFFFu }}, //
    {{ 0x0000FFFFFFFFFFFFu }}, //
    {{ 0x00FFFFFFFFFFFFFFu }}, //
    {{ 0xFFFFFFFFFFFFFFFFu }}, // [40]
    {{ 0xFFFFFFFFFFFFFFFFu }}, //
    {{ 0xFFFFFFFFFFFFFFFFu }}, //
    {{ 0xFFFFFFFFFFFFFFFFu }}, //
    {{ 0xFFFFFFFFFFFFFFFFu }}, //
    {{ 0xFFFFFFFFFFFFFFFFu }}, //
    {{ 0xFFFFFFFFFFFFFFFFu }}, //
    {{ 0xFFFFFFFFFFFFFFFFu }}, //
    {{ 0xFFFFFFFFFFFFFFFFu }}, // [48]
    {{ 0xFFFFFFFFFFFFFFFFu }}, //
    {{ 0xFFFFFFFFFFFFFFFFu }}, //
    {{ 0xFFFFFFFFFFFFFFFFu }}, //
    {{ 0xFFFFFFFFFFFFFFFFu }}, //
    {{ 0xFFFFFFFFFFFFFFFFu }}, //
    {{ 0xFFFFFFFFFFFFFFFFu }}, //
    {{ 0xFFFFFFFFFFFFFFFFu }}, //
    {{ 0xFFFFFFFFFFFFFFFFu }}, // [56]
    {{ 0xFFFFFFFFFFFFFFFFu }}, //
    {{ 0xFFFFFFFFFFFFFFFFu }}, //
    {{ 0xFFFFFFFFFFFFFFFFu }}, //
    {{ 0xFFFFFFFFFFFFFFFFu }}, //
    {{ 0xFFFFFFFFFFFFFFFFu }}, //
    {{ 0xFFFFFFFFFFFFFFFFu }}, //
    {{ 0xFFFFFFFFFFFFFFFFu }}, //
    {{ 0xFFFFFFFFFFFFFFFFu }}, // [64]
    {{ 0xFFFFFFFFFFFFFFFFu }}, //
    {{ 0xFFFFFFFFFFFFFFFFu }}, //
    {{ 0xFFFFFFFFFFFFFFFFu }}, //
    {{ 0xFFFFFFFFFFFFFFFFu }}, //
    {{ 0xFFFFFFFFFFFFFFFFu }}, //
    {{ 0xFFFFFFFFFFFFFFFFu }}, //
    {{ 0xFFFFFFFFFFFFFFFFu }}, //
    {{ 0xFFFFFFFFFFFFFFFFu }}  // [72]
  };

  BL_INLINE const VecConst64<uint64_t>* loadstore16_lo8_msk8() const noexcept { return loadstore_msk8_data + 32; }
  BL_INLINE const VecConst64<uint64_t>* loadstore16_hi8_msk8() const noexcept { return loadstore_msk8_data + 24; }

#endif // BL_TARGET_ARCH_X86

/*
  VecConst128<uint64_t> swizu8_load4x32_tail_predicate[5] {
    {{ 0xffffffffffffffffu, 0xffffffffffffffffu }},
    {{ 0xffffffff0F0E0D0Cu, 0xffffffffffffffffu }},
    {{ 0x0F0E0D0C0B0A0908u, 0xffffffffffffffffu }},
    {{ 0x0B0A090807060504u, 0xffffffff0F0E0D0Cu }},
    {{ 0x0706050403020100u, 0x0F0E0D0C0B0A0908u }} // This should never be hit during 3-element load/store.
  };
*/
  VecConst128<uint64_t> swizu8_load_tail_0_to_16[17] {
    {{ 0xffffffffffffffffu, 0xffffffffffffffffu }}, // [00] The memory layout shown here describes how the 32-bit elements
    {{ 0xffffffffffffff00u, 0xffffffffffffffffu }}, // [01] were loaded to the vector register. We don't have to describe
    {{ 0xffffffffffff0100u, 0xffffffffffffffffu }}, // [02] 00-03 as that would be out of bounds access, which we don't do.
    {{ 0xffffffffff020100u, 0xffffffffffffffffu }}, // [03]
    {{ 0xffffffff03020100u, 0xffffffffffffffffu }}, // [04]
    {{ 0xffffff0703020100u, 0xffffffffffffffffu }}, // [05] [ __ __ __ __ | __ __ __ __ | 04 03 02 01 | 03 02 01 00 ]
    {{ 0xffff070603020100u, 0xffffffffffffffffu }}, // [06] [ __ __ __ __ | __ __ __ __ | 05 04 03 02 | 03 02 01 00 ]
    {{ 0xff07060503020100u, 0xffffffffffffffffu }}, // [07] [ __ __ __ __ | __ __ __ __ | 06 05 04 03 | 03 02 01 00 ]
    {{ 0x0706050403020100u, 0xffffffffffffffffu }}, // [08] [ __ __ __ __ | __ __ __ __ | 07 06 05 04 | 03 02 01 00 ]
    {{ 0x0706050403020100u, 0xffffffffffffff0Bu }}, // [09] [ __ __ __ __ | 08 07 06 05 | 07 06 05 04 | 03 02 01 00 ]
    {{ 0x0706050403020100u, 0xffffffffffff0B0Au }}, // [10] [ __ __ __ __ | 09 08 07 06 | 07 06 05 04 | 03 02 01 00 ]
    {{ 0x0706050403020100u, 0xffffffffff0B0A09u }}, // [11] [ __ __ __ __ | 10 09 08 07 | 07 06 05 04 | 03 02 01 00 ]
    {{ 0x0706050403020100u, 0xffffffff0B0A0908u }}, // [12] [ __ __ __ __ | 11 10 09 08 | 07 06 05 04 | 03 02 01 00 ]
    {{ 0x0706050403020100u, 0xffffff0F0B0A0908u }}, // [13] [ 12 11 10 09 | 11 10 09 08 | 07 06 05 04 | 03 02 01 00 ]
    {{ 0x0706050403020100u, 0xffff0F0E0B0A0908u }}, // [14] [ 13 12 11 10 | 11 10 09 08 | 07 06 05 04 | 03 02 01 00 ]
    {{ 0x0706050403020100u, 0xff0F0E0D0B0A0908u }}, // [15] [ 14 13 12 11 | 11 10 09 08 | 07 06 05 04 | 03 02 01 00 ]
    {{ 0x0706050403020100u, 0x0F0E0D0C0B0A0908u }}  // [16] [ 15 14 13 12 | 11 10 09 08 | 07 06 05 04 | 03 02 01 00 ]
  };

  //! \}

  //! \name Dithering Constants
  //! \{

  //! 16x16 Bayer dithering matrix repeated twice in X direction. The reason is that we want to use 16-byte loads
  //! regardless of where the X offset is (X offset is only using 4 bits). This matrix is also aligned to 1024 bytes,
  //! thus it's trivial to repeat it in Y direction by essentially just clearing one byt after every advance.
  uint8_t bayer_matrix_16x16[16u * 16u * 2u] = {
    #define BL_DM_ROW(...) __VA_ARGS__, __VA_ARGS__

    BL_DM_ROW(  0, 191,  48, 239,  12, 203,  60, 251,   3, 194,  51, 242,  15, 206,  63, 254),
    BL_DM_ROW(127,  64, 175, 112, 139,  76, 187, 124, 130,  67, 178, 115, 142,  79, 190, 127),
    BL_DM_ROW( 32, 223,  16, 207,  44, 235,  28, 219,  35, 226,  19, 210,  47, 238,  31, 222),
    BL_DM_ROW(159,  96, 143,  80, 171, 108, 155,  92, 162,  99, 146,  83, 174, 111, 158,  95),
    BL_DM_ROW(  8, 199,  56, 247,   4, 195,  52, 243,  11, 202,  59, 250,   7, 198,  55, 246),
    BL_DM_ROW(135,  72, 183, 120, 131,  68, 179, 116, 138,  75, 186, 123, 134,  71, 182, 119),
    BL_DM_ROW( 40, 231,  24, 215,  36, 227,  20, 211,  43, 234,  27, 218,  39, 230,  23, 214),
    BL_DM_ROW(167, 104, 151,  88, 163, 100, 147,  84, 170, 107, 154,  91, 166, 103, 150,  87),
    BL_DM_ROW(  2, 193,  50, 241,  14, 205,  62, 253,   1, 192,  49, 240,  13, 204,  61, 252),
    BL_DM_ROW(129,  66, 177, 114, 141,  78, 189, 126, 128,  65, 176, 113, 140,  77, 188, 125),
    BL_DM_ROW( 34, 225,  18, 209,  46, 237,  30, 221,  33, 224,  17, 208,  45, 236,  29, 220),
    BL_DM_ROW(161,  98, 145,  82, 173, 110, 157,  94, 160,  97, 144,  81, 172, 109, 156,  93),
    BL_DM_ROW( 10, 201,  58, 249,   6, 197,  54, 245,   9, 200,  57, 248,   5, 196,  53, 244),
    BL_DM_ROW(137,  74, 185, 122, 133,  70, 181, 118, 136,  73, 184, 121, 132,  69, 180, 117),
    BL_DM_ROW( 42, 233,  26, 217,  38, 229,  22, 213,  41, 232,  25, 216,  37, 228,  21, 212),
    BL_DM_ROW(169, 106, 153,  90, 165, 102, 149,  86, 168, 105, 152,  89, 164, 101, 148,  85)

    #undef BL_DM_ROW
  };

  //! \}

  //! \name Unpremultiply Tables
  //! \{

  //! Table, which can be used to turn integer division into multiplication and shift that is used by PRGB to ARGB
  //! (unpremultiply) pixel conversion. It supports division by 0 (multiplies by zero) up to 255 using 24 bits of
  //! precision. The multiplied product has to be shifted to the right by 16 bits to receive the final result.
  //!
  //! The unpremultiply function:
  //!
  //!   `if (b) ? (a * 255) / b : 0`
  //!
  //! can be rewritten as
  //!
  //!   `(a * unpremultiply_rcp[b] + 0x8000u) >> 16`
  uint32_t unpremultiply_rcp[256] = {
    // Could be also generated by:
    //
    // struct BLUnpremultiplyTableGen {
    //   //! Calculates the reciprocal for `CommonTable::unpremultiply` table.
    //   static constexpr uint32_t value(size_t i) noexcept {
    //     return i ? uint32_t(0xFF00FF / i) : uint32_t(0);
    //   }
    // };
    0x00000000u, 0x00FF00FFu, 0x007F807Fu, 0x00550055u, 0x003FC03Fu, 0x00330033u, 0x002A802Au, 0x00246DDBu,
    0x001FE01Fu, 0x001C5571u, 0x00198019u, 0x00172EA2u, 0x00154015u, 0x00139D9Du, 0x001236EDu, 0x00110011u,
    0x000FF00Fu, 0x000F000Fu, 0x000E2AB8u, 0x000D6BD7u, 0x000CC00Cu, 0x000C249Eu, 0x000B9751u, 0x000B164Du,
    0x000AA00Au, 0x000A333Du, 0x0009CECEu, 0x000971D0u, 0x00091B76u, 0x0008CB11u, 0x00088008u, 0x000839D6u,
    0x0007F807u, 0x0007BA36u, 0x00078007u, 0x0007492Bu, 0x0007155Cu, 0x0006E459u, 0x0006B5EBu, 0x000689DFu,
    0x00066006u, 0x00063838u, 0x0006124Fu, 0x0005EE29u, 0x0005CBA8u, 0x0005AAB0u, 0x00058B26u, 0x00056CF5u,
    0x00055005u, 0x00053443u, 0x0005199Eu, 0x00050005u, 0x0004E767u, 0x0004CFB7u, 0x0004B8E8u, 0x0004A2EDu,
    0x00048DBBu, 0x00047947u, 0x00046588u, 0x00045275u, 0x00044004u, 0x00042E2Eu, 0x00041CEBu, 0x00040C34u,
    0x0003FC03u, 0x0003EC52u, 0x0003DD1Bu, 0x0003CE57u, 0x0003C003u, 0x0003B219u, 0x0003A495u, 0x00039773u,
    0x00038AAEu, 0x00037E42u, 0x0003722Cu, 0x00036669u, 0x00035AF5u, 0x00034FCEu, 0x000344EFu, 0x00033A57u,
    0x00033003u, 0x000325F0u, 0x00031C1Cu, 0x00031284u, 0x00030927u, 0x00030003u, 0x0002F714u, 0x0002EE5Bu,
    0x0002E5D4u, 0x0002DD7Eu, 0x0002D558u, 0x0002CD5Fu, 0x0002C593u, 0x0002BDF2u, 0x0002B67Au, 0x0002AF2Bu,
    0x0002A802u, 0x0002A0FFu, 0x00029A21u, 0x00029367u, 0x00028CCFu, 0x00028658u, 0x00028002u, 0x000279CBu,
    0x000273B3u, 0x00026DB9u, 0x000267DBu, 0x0002621Au, 0x00025C74u, 0x000256E8u, 0x00025176u, 0x00024C1Du,
    0x000246DDu, 0x000241B5u, 0x00023CA3u, 0x000237A9u, 0x000232C4u, 0x00022DF5u, 0x0002293Au, 0x00022494u,
    0x00022002u, 0x00021B83u, 0x00021717u, 0x000212BDu, 0x00020E75u, 0x00020A3Fu, 0x0002061Au, 0x00020206u,
    0x0001FE01u, 0x0001FA0Du, 0x0001F629u, 0x0001F254u, 0x0001EE8Du, 0x0001EAD5u, 0x0001E72Bu, 0x0001E390u,
    0x0001E001u, 0x0001DC80u, 0x0001D90Cu, 0x0001D5A5u, 0x0001D24Au, 0x0001CEFCu, 0x0001CBB9u, 0x0001C882u,
    0x0001C557u, 0x0001C236u, 0x0001BF21u, 0x0001BC16u, 0x0001B916u, 0x0001B620u, 0x0001B334u, 0x0001B053u,
    0x0001AD7Au, 0x0001AAACu, 0x0001A7E7u, 0x0001A52Au, 0x0001A277u, 0x00019FCDu, 0x00019D2Bu, 0x00019A92u,
    0x00019801u, 0x00019578u, 0x000192F8u, 0x0001907Fu, 0x00018E0Eu, 0x00018BA4u, 0x00018942u, 0x000186E7u,
    0x00018493u, 0x00018247u, 0x00018001u, 0x00017DC2u, 0x00017B8Au, 0x00017958u, 0x0001772Du, 0x00017508u,
    0x000172EAu, 0x000170D1u, 0x00016EBFu, 0x00016CB2u, 0x00016AACu, 0x000168ABu, 0x000166AFu, 0x000164BAu,
    0x000162C9u, 0x000160DEu, 0x00015EF9u, 0x00015D18u, 0x00015B3Du, 0x00015966u, 0x00015795u, 0x000155C9u,
    0x00015401u, 0x0001523Eu, 0x0001507Fu, 0x00014EC6u, 0x00014D10u, 0x00014B60u, 0x000149B3u, 0x0001480Bu,
    0x00014667u, 0x000144C7u, 0x0001432Cu, 0x00014194u, 0x00014001u, 0x00013E71u, 0x00013CE5u, 0x00013B5Du,
    0x000139D9u, 0x00013859u, 0x000136DCu, 0x00013563u, 0x000133EDu, 0x0001327Bu, 0x0001310Du, 0x00012FA1u,
    0x00012E3Au, 0x00012CD5u, 0x00012B74u, 0x00012A16u, 0x000128BBu, 0x00012763u, 0x0001260Eu, 0x000124BDu,
    0x0001236Eu, 0x00012223u, 0x000120DAu, 0x00011F94u, 0x00011E51u, 0x00011D11u, 0x00011BD4u, 0x00011A9Au,
    0x00011962u, 0x0001182Du, 0x000116FAu, 0x000115CAu, 0x0001149Du, 0x00011372u, 0x0001124Au, 0x00011124u,
    0x00011001u, 0x00010EE0u, 0x00010DC1u, 0x00010CA5u, 0x00010B8Bu, 0x00010A73u, 0x0001095Eu, 0x0001084Bu,
    0x0001073Au, 0x0001062Cu, 0x0001051Fu, 0x00010415u, 0x0001030Du, 0x00010207u, 0x00010103u, 0x00010001u
  };

#if BL_TARGET_ARCH_X86

  // These tables are designed to take advantage of PMADDWD instruction (or its VPMADDWD AVX variant) so we only use
  // them on X86 targets. The approach with `unpremultiply_rcp[]` table is not possible in baseline SSE2 and it's not
  // so good even on SSE4.1 capable hardware that has PMULLD instruction, which has double the latency compared to
  // other multiplication instructions including PMADDWD.
  //
  // The downside of this approach is that we need two extra tables, as the multiplication is not precise enough.
  // The first table `unpremultiply_pmaddwd_rcp` is used with PMADDWD instruction, and the second table is used to
  // round the result correctly.
  uint32_t unpremultiply_pmaddwd_rcp[256] = {
    0x00000000u, 0x7E0067D0u, 0x3E0077D0u, 0x2A002555u, 0x1E007AAAu, 0x18006333u, 0x140052AAu, 0x12000FFFu,
    0x0E007CCCu, 0x0E000B6Du, 0x0C003199u, 0x0A0065FFu, 0x0A0028E3u, 0x080073FFu, 0x08004745u, 0x08002111u,
    0x06007E38u, 0x060060F0u, 0x060045B6u, 0x06002D89u, 0x0600186Bu, 0x060004B4u, 0x040072FFu, 0x040062D2u,
    0x0400542Cu, 0x0400468Bu, 0x040039FFu, 0x04002E50u, 0x0400237Au, 0x04001969u, 0x04001088u, 0x04000745u,
    0x02007F0Fu, 0x02007755u, 0x02007078u, 0x02006936u, 0x020062C2u, 0x02005C92u, 0x020056C4u, 0x02005143u,
    0x02004C1Fu, 0x0200470Au, 0x0200425Au, 0x02003DC7u, 0x0200397Bu, 0x02003574u, 0x02003169u, 0x02002DA1u,
    0x02002A0Bu, 0x0200268Bu, 0x02002345u, 0x02002050u, 0x02001CEFu, 0x020019FFu, 0x02001728u, 0x02001464u,
    0x020011BAu, 0x02000F2Du, 0x02000CB4u, 0x02000A4Fu, 0x02000823u, 0x020005C7u, 0x020003A2u, 0x02000189u,
    0x00007F83u, 0x00007D8Fu, 0x00007BAAu, 0x000079CBu, 0x0000781Eu, 0x00007646u, 0x0000749Bu, 0x000072EEu,
    0x0000715Au, 0x00006FFFu, 0x00006E49u, 0x00006CD8u, 0x00006B62u, 0x000069FFu, 0x000068A1u, 0x0000674Bu,
    0x00006606u, 0x000064BFu, 0x00006385u, 0x00006250u, 0x00006128u, 0x00006030u, 0x00005EE3u, 0x00005DCCu,
    0x00005CBDu, 0x00005BB1u, 0x00005ABAu, 0x000059ACu, 0x000058B4u, 0x000057BFu, 0x000056D0u, 0x000055E7u,
    0x00005503u, 0x00005421u, 0x00005345u, 0x0000526Eu, 0x0000519Du, 0x000050CCu, 0x00005028u, 0x00004F39u,
    0x00004E77u, 0x00004DBCu, 0x00004CFCu, 0x00004C44u, 0x00004B90u, 0x00004ADDu, 0x00004A32u, 0x00004984u,
    0x000048DDu, 0x00004837u, 0x00004796u, 0x000046F6u, 0x00004659u, 0x000045BFu, 0x00004529u, 0x00004497u,
    0x00004408u, 0x00004370u, 0x000042E3u, 0x00004258u, 0x000041CFu, 0x00004149u, 0x000040C4u, 0x00004041u,
    0x00003FC0u, 0x00003F42u, 0x00003EC7u, 0x00003E4Au, 0x00003DD2u, 0x00003D5Au, 0x00003CE2u, 0x00003C75u,
    0x00003C07u, 0x00003B90u, 0x00003B23u, 0x00003AB4u, 0x00003A4Bu, 0x000039DFu, 0x00003977u, 0x00003911u,
    0x000038ACu, 0x00003847u, 0x000037E5u, 0x00003783u, 0x00003723u, 0x000036C4u, 0x0000366Cu, 0x0000360Au,
    0x000035B0u, 0x0000355Eu, 0x000034FDu, 0x000034A6u, 0x0000344Fu, 0x000033FFu, 0x000033A6u, 0x00003352u,
    0x00003301u, 0x000032AFu, 0x0000325Fu, 0x00003210u, 0x000031C2u, 0x00003176u, 0x00003128u, 0x000030DDu,
    0x00003093u, 0x00003049u, 0x00003018u, 0x00002FB8u, 0x00002F72u, 0x00002F2Bu, 0x00002EE6u, 0x00002EA1u,
    0x00002E5Eu, 0x00002E1Au, 0x00002DD8u, 0x00002D96u, 0x00002D59u, 0x00002D17u, 0x00002CD6u, 0x00002C97u,
    0x00002C59u, 0x00002C1Cu, 0x00002BDFu, 0x00002BA4u, 0x00002B68u, 0x00002B2Du, 0x00002AF3u, 0x00002AB9u,
    0x00002A80u, 0x00002A49u, 0x00002A10u, 0x000029DAu, 0x000029A2u, 0x0000296Cu, 0x00002937u, 0x00002901u,
    0x000028CEu, 0x00002899u, 0x00002866u, 0x00002832u, 0x0000280Au, 0x000027CEu, 0x0000279Eu, 0x0000276Cu,
    0x0000273Cu, 0x0000270Bu, 0x000026DEu, 0x000026ACu, 0x0000267Eu, 0x0000264Fu, 0x00002622u, 0x000025F4u,
    0x000025C7u, 0x0000259Au, 0x0000256Fu, 0x00002543u, 0x00002518u, 0x000024EDu, 0x000024C2u, 0x0000249Au,
    0x0000246Eu, 0x00002445u, 0x0000241Cu, 0x000023F4u, 0x000023CAu, 0x000023A4u, 0x0000237Bu, 0x00002353u,
    0x00002333u, 0x00002306u, 0x000022DFu, 0x000022B9u, 0x00002294u, 0x0000226Eu, 0x0000224Bu, 0x00002227u,
    0x00002202u, 0x000021DCu, 0x000021B8u, 0x00002195u, 0x00002174u, 0x0000214Fu, 0x0000212Cu, 0x0000210Au,
    0x000020E7u, 0x000020C5u, 0x000020A4u, 0x00002083u, 0x00002062u, 0x00002041u, 0x00002040u, 0x00002010u
  };

  uint32_t unpremultiply_pmaddwd_rnd[256] = {
    0x0000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u,
    0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u,
    0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u,
    0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u,
    0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u,
    0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u,
    0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u,
    0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u,
    0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u,
    0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u,
    0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u,
    0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u,
    0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u,
    0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u,
    0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x0F8Du, 0x1000u,
    0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u,
    0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1100u, 0x1000u,
    0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u,
    0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x0FE2u, 0x1000u, 0x1000u, 0x1000u,
    0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x0FC6u, 0x1000u,
    0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1008u, 0x1000u,
    0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x0FB4u, 0x1000u, 0x1000u, 0x1000u,
    0x0FB0u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u,
    0x1004u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u,
    0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x1000u,
    0x1000u, 0x1000u, 0x0FC2u, 0x1000u, 0x1000u, 0x1000u, 0x0F6Eu, 0x0FB4u,
    0x0FA0u, 0x1000u, 0x1000u, 0x1000u, 0x0FD4u, 0x1000u, 0x1000u, 0x1000u,
    0x1014u, 0x1000u, 0x0FBDu, 0x1000u, 0x1000u, 0x1000u, 0x1000u, 0x0F00u,
    0x0FE0u, 0x1000u, 0x0FA4u, 0x0F3Cu, 0x1014u, 0x0F24u, 0x1000u, 0x1000u,
    0x0CE4u, 0x0FA6u, 0x1023u, 0x1000u, 0x0FC8u, 0x1000u, 0x1000u, 0x0EC8u,
    0x1000u, 0x1000u, 0x1008u, 0x0FADu, 0x0EB8u, 0x0F95u, 0x1000u, 0x0F92u,
    0x101Cu, 0x1019u, 0x1000u, 0x0FA9u, 0x0FECu, 0x1000u, 0x0040u, 0x1000u
  };

#endif // BL_TARGET_ARCH_X86

  //! \}

  //! \name Epilog
  //! \{

  //! Dummy constant to have something at the end.
  uint8_t epilog[32] {};

  //! \}
};

#undef REPEAT_128B
#undef REPEAT_64B
#undef REPEAT_32B

#undef REPEAT_16X
#undef REPEAT_8X
#undef REPEAT_4X
#undef REPEAT_2X

BL_HIDDEN extern const CommonTable common_table;

} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_TABLES_TABLES_P_H_INCLUDED
