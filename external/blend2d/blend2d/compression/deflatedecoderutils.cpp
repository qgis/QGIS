// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/compression/deflatedecoderutils_p.h>

namespace bl::Compression::Deflate {

#if BL_TARGET_ARCH_BITS < 64
const BLBitWord kScalarRepeatMultiply[sizeof(BLBitWord)] = {
  //_3_2_1_0
  0u,
  0x01010101u,
  0x00010001u,
  0x01000001u
};

// Offset 1 and 2 are simple rotates, offset 3 needs special care:
//   offset=3: [BBAACCBB|AACCBBAA] (L=08 R=16) => [AACCBBAA|CCBBAACC]
const uint8_t kScalarRotatePredicateL[sizeof(BLBitWord)] = { 0, 0, 0,  8 };
const uint8_t kScalarRotatePredicateR[sizeof(BLBitWord)] = { 0, 0, 0, 16 };
#else
const BLBitWord kScalarRepeatMultiply[sizeof(BLBitWord)] = {
  //_7_6_5_4_3_2_1_0
  0u,
  0x0101010101010101u,
  0x0001000100010001u,
  0x0001000001000001u,
  0x0000000100000001u,
  0x0000010000000001u,
  0x0001000000000001u,
  0x0100000000000001u
};

// Offset 1, 2, and 4 are simple rotates, offsets 3, 5, 6, 7 need special care:
//   offset=3: [BBAACCBB|AACCBBAA] (L=08 R=16) => [AACCBBAA|CCBBAACC]
//   offset=5: [CCBBAAEE|DDCCBBAA] (L=16 R=24) => [AAEEDDCC|BBAAEEDD]
//   offset=6: [BBAAFFEE|DDCCBBAA] (L=32 R=16) => [DDCCBBAA|FFEEDDCC]
//   offset=7: [AAGGFFEE|DDCCBBAA] (L=48 R=08) => [BBAAGGFF|EEDDCCBB]
const uint8_t kScalarRotatePredicateL[sizeof(BLBitWord)] = { 0, 0, 0,  8, 0, 16, 32, 48 };
const uint8_t kScalarRotatePredicateR[sizeof(BLBitWord)] = { 0, 0, 0, 16, 0, 24, 16,  8 };
#endif

const SimdRepeatTable16 kSimdRepeatTable16[16] = {
  {{0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 }}, // #00 (impossible)
  {{0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 }}, // #01
  {{0 , 1 , 0 , 1 , 0 , 1 , 0 , 1 , 0 , 1 , 0 , 1 , 0 , 1 , 0 , 1 }}, // #02
  {{0 , 1 , 2 , 0 , 1 , 2 , 0 , 1 , 2 , 0 , 1 , 2 , 0 , 1 , 2 , 0 }}, // #03
  {{0 , 1 , 2 , 3 , 0 , 1 , 2 , 3 , 0 , 1 , 2 , 3 , 0 , 1 , 2 , 3 }}, // #04
  {{0 , 1 , 2 , 3 , 4 , 0 , 1 , 2 , 3 , 4 , 0 , 1 , 2 , 3 , 4 , 0 }}, // #05
  {{0 , 1 , 2 , 3 , 4 , 5 , 0 , 1 , 2 , 3 , 4 , 5 , 0 , 1 , 2 , 3 }}, // #06
  {{0 , 1 , 2 , 3 , 4 , 5 , 6 , 0 , 1 , 2 , 3 , 4 , 5 , 6 , 0 , 1 }}, // #07
  {{0 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 0 , 1 , 2 , 3 , 4 , 5 , 6 , 7 }}, // #08
  {{0 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 , 0 , 1 , 2 , 3 , 4 , 5 , 6 }}, // #09
  {{0 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 , 9 , 0 , 1 , 2 , 3 , 4 , 5 }}, // #10
  {{0 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 , 9 , 10, 0 , 1 , 2 , 3 , 4 }}, // #11
  {{0 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 , 9 , 10, 11, 0 , 1 , 2 , 3 }}, // #12
  {{0 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 , 9 , 10, 11, 12, 0 , 1 , 2 }}, // #13
  {{0 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 , 9 , 10, 11, 12, 13, 0 , 1 }}, // #14
  {{0 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 , 9 , 10, 11, 12, 13, 14, 0 }}  // #15
};

const SimdRepeatTable16 kSimdRotateTable16[16] = {
  {{0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 }}, // #00 (impossible)
  {{0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 }}, // #01
  {{0 , 1 , 0 , 1 , 0 , 1 , 0 , 1 , 0 , 1 , 0 , 1 , 0 , 1 , 0 , 1 }}, // #02
  {{1 , 2 , 0 , 1 , 2 , 0 , 1 , 2 , 0 , 1 , 2 , 0 , 1 , 2 , 0 , 1 }}, // #03
  {{0 , 1 , 2 , 3 , 0 , 1 , 2 , 3 , 0 , 1 , 2 , 3 , 0 , 1 , 2 , 3 }}, // #04
  {{1 , 2 , 3 , 4 , 0 , 1 , 2 , 3 , 4 , 0 , 1 , 2 , 3 , 4 , 0 , 1 }}, // #05
  {{4 , 5 , 0 , 1 , 2 , 3 , 4 , 5 , 0 , 1 , 2 , 3 , 4 , 5 , 0 , 1 }}, // #06
  {{2 , 3 , 4 , 5 , 6 , 0 , 1 , 2 , 3 , 4 , 5 , 6 , 0 , 1 , 2 , 3 }}, // #07
  {{0 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 0 , 1 , 2 , 3 , 4 , 5 , 6 , 7 }}, // #08
  {{7 , 8 , 0 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 , 0 , 1 , 2 , 3 , 4 }}, // #09
  {{6 , 7 , 8 , 9 , 0 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 , 9 , 0 , 1 }}, // #10
  {{5 , 6 , 7 , 8 , 9 , 10, 0 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 , 9 }}, // #11
  {{4 , 5 , 6 , 7 , 8 , 9 , 10, 11, 0 , 1 , 2 , 3 , 4 , 5 , 6 , 7 }}, // #12
  {{3 , 4 , 5 , 6 , 7 , 8 , 9 , 10, 11, 12, 0 , 1 , 2 , 3 , 4 , 5 }}, // #13
  {{2 , 3 , 4 , 5 , 6 , 7 , 8 , 9 , 10, 11, 12, 13, 0 , 1 , 2 , 3 }}, // #14
  {{1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 , 9 , 10, 11, 12, 13, 14, 0 , 1 }}  // #15
};

} // {bl::Compression::Deflate}
