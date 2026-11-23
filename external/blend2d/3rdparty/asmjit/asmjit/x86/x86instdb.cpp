// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#if !defined(ASMJIT_NO_X86)

#include <asmjit/core/cpuinfo.h>
#include <asmjit/core/misc_p.h>
#include <asmjit/support/support.h>
#include <asmjit/x86/x86instdb_p.h>
#include <asmjit/x86/x86opcode_p.h>
#include <asmjit/x86/x86operand.h>

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

// x86::InstDB - InstInfo
// ======================

// Instruction opcode definitions:
//   - `O` encodes X86|MMX|SSE instructions.
//   - `V` encodes VEX|XOP|EVEX instructions.
//   - `E` encodes EVEX instructions only.
#define O_ENCODE(PREFIX, OPCODE, O, L, W, EvexW, N, TT) ((PREFIX) | (OPCODE) | (O) | (L) | (W) | (EvexW) | (N) | (TT))

#define O(PREFIX, OPCODE, ModO, LL, W, EvexW, N, ModRM) (O_ENCODE(Opcode::k##PREFIX, 0x##OPCODE, Opcode::kModO_##ModO, Opcode::kLL_##LL, Opcode::kW_##W, Opcode::kEvex_W_##EvexW, Opcode::kCDSHL_##N, Opcode::kModRM_##ModRM))
#define V(PREFIX, OPCODE, ModO, LL, W, EvexW, N, TT) (O_ENCODE(Opcode::k##PREFIX, 0x##OPCODE, Opcode::kModO_##ModO, Opcode::kLL_##LL, Opcode::kW_##W, Opcode::kEvex_W_##EvexW, Opcode::kCDSHL_##N, Opcode::kCDTT_##TT))
#define E(PREFIX, OPCODE, ModO, LL, W, EvexW, N, TT) (O_ENCODE(Opcode::k##PREFIX, 0x##OPCODE, Opcode::kModO_##ModO, Opcode::kLL_##LL, Opcode::kW_##W, Opcode::kEvex_W_##EvexW, Opcode::kCDSHL_##N, Opcode::kCDTT_##TT) | Opcode::kMM_ForceEvex)
#define O_FPU(PREFIX, OPCODE, ModO) (Opcode::kFPU_##PREFIX | (0x##OPCODE & 0xFFu) | ((0x##OPCODE >> 8) << Opcode::kFPU_2B_Shift) | Opcode::kModO_##ModO)

// Defines an X86 instruction.
#define INST(id, encoding, opcode0, opcode1, main_opcode_index, alt_opcode_index, common_info_index, additional_info_index) { \
  uint32_t(0),                              \
  uint32_t(common_info_index),              \
  uint32_t(additional_info_index),          \
  uint8_t(InstDB::kEncoding##encoding),     \
  uint8_t((opcode0) & 0xFFu),               \
  uint8_t(main_opcode_index),               \
  uint8_t(alt_opcode_index)                 \
}

const InstDB::InstInfo InstDB::_inst_info_table[] = {
  /*--------------------+--------------------+------------------+--------+------------------+--------+----+----+----+----+
  |    Instruction      |    Instruction     |    Main Opcode   |  EVEX  |Alternative Opcode|  EVEX  |Op0X|Op1X|IdxA|IdxB|
  |     Id & Name       |      Encoding      |  (pp+mmm|op/o|L|w|W|N|TT.)|--(pp+mmm|op/o|L|w|W|N|TT.)| (auto-generated)  |
  +---------------------+--------------------+---------+----+-+-+-+-+----+---------+----+-+-+-+-+----+----+----+----+---*/
  // ${InstInfo:Begin}
  INST(None             , None               , 0                         , 0                         , 0  , 0  , 0  , 0  ), // #0
  INST(Aaa              , X86Op_xAX          , O(000000,37,_,_,_,_,_,_  ), 0                         , 0  , 0  , 1  , 1  ), // #1
  INST(Aad              , X86I_xAX           , O(000000,D5,_,_,_,_,_,_  ), 0                         , 0  , 0  , 2  , 1  ), // #2
  INST(Aadd             , X86Mr              , O(000F38,FC,_,_,_,_,_,_  ), 0                         , 1  , 0  , 3  , 2  ), // #3
  INST(Aam              , X86I_xAX           , O(000000,D4,_,_,_,_,_,_  ), 0                         , 0  , 0  , 2  , 1  ), // #4
  INST(Aand             , X86Mr              , O(660F38,FC,_,_,_,_,_,_  ), 0                         , 2  , 0  , 3  , 2  ), // #5
  INST(Aas              , X86Op_xAX          , O(000000,3F,_,_,_,_,_,_  ), 0                         , 0  , 0  , 1  , 1  ), // #6
  INST(Adc              , X86Arith           , O(000000,10,2,_,x,_,_,_  ), 0                         , 3  , 0  , 4  , 3  ), // #7
  INST(Adcx             , X86Rm              , O(660F38,F6,_,_,x,_,_,_  ), 0                         , 2  , 0  , 5  , 4  ), // #8
  INST(Add              , X86Arith           , O(000000,00,0,_,x,_,_,_  ), 0                         , 0  , 0  , 4  , 1  ), // #9
  INST(Addpd            , ExtRm              , O(660F00,58,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6  , 5  ), // #10
  INST(Addps            , ExtRm              , O(000F00,58,_,_,_,_,_,_  ), 0                         , 5  , 0  , 6  , 6  ), // #11
  INST(Addsd            , ExtRm              , O(F20F00,58,_,_,_,_,_,_  ), 0                         , 6  , 0  , 7  , 5  ), // #12
  INST(Addss            , ExtRm              , O(F30F00,58,_,_,_,_,_,_  ), 0                         , 7  , 0  , 8  , 6  ), // #13
  INST(Addsubpd         , ExtRm              , O(660F00,D0,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6  , 7  ), // #14
  INST(Addsubps         , ExtRm              , O(F20F00,D0,_,_,_,_,_,_  ), 0                         , 6  , 0  , 6  , 7  ), // #15
  INST(Adox             , X86Rm              , O(F30F38,F6,_,_,x,_,_,_  ), 0                         , 8  , 0  , 5  , 8  ), // #16
  INST(Aesdec           , ExtRm              , O(660F38,DE,_,_,_,_,_,_  ), 0                         , 2  , 0  , 6  , 9  ), // #17
  INST(Aesdeclast       , ExtRm              , O(660F38,DF,_,_,_,_,_,_  ), 0                         , 2  , 0  , 6  , 9  ), // #18
  INST(Aesenc           , ExtRm              , O(660F38,DC,_,_,_,_,_,_  ), 0                         , 2  , 0  , 6  , 9  ), // #19
  INST(Aesenclast       , ExtRm              , O(660F38,DD,_,_,_,_,_,_  ), 0                         , 2  , 0  , 6  , 9  ), // #20
  INST(Aesimc           , ExtRm              , O(660F38,DB,_,_,_,_,_,_  ), 0                         , 2  , 0  , 6  , 9  ), // #21
  INST(Aeskeygenassist  , ExtRmi             , O(660F3A,DF,_,_,_,_,_,_  ), 0                         , 9  , 0  , 9  , 9  ), // #22
  INST(And              , X86Arith           , O(000000,20,4,_,x,_,_,_  ), 0                         , 10 , 0  , 10 , 1  ), // #23
  INST(Andn             , VexRvm_Wx          , V(000F38,F2,_,0,x,_,_,_  ), 0                         , 11 , 0  , 11 , 10 ), // #24
  INST(Andnpd           , ExtRm              , O(660F00,55,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6  , 5  ), // #25
  INST(Andnps           , ExtRm              , O(000F00,55,_,_,_,_,_,_  ), 0                         , 5  , 0  , 6  , 6  ), // #26
  INST(Andpd            , ExtRm              , O(660F00,54,_,_,_,_,_,_  ), 0                         , 4  , 0  , 12 , 5  ), // #27
  INST(Andps            , ExtRm              , O(000F00,54,_,_,_,_,_,_  ), 0                         , 5  , 0  , 12 , 6  ), // #28
  INST(Aor              , X86Mr              , O(F20F38,FC,_,_,_,_,_,_  ), 0                         , 12 , 0  , 3  , 2  ), // #29
  INST(Arpl             , X86Mr_NoSize       , O(000000,63,_,_,_,_,_,_  ), 0                         , 0  , 0  , 13 , 11 ), // #30
  INST(Axor             , X86Mr              , O(F30F38,FC,_,_,_,_,_,_  ), 0                         , 8  , 0  , 3  , 2  ), // #31
  INST(Bextr            , VexRmv_Wx          , V(000F38,F7,_,0,x,_,_,_  ), 0                         , 11 , 0  , 14 , 10 ), // #32
  INST(Blcfill          , VexVm_Wx           , V(XOP_M9,01,1,0,x,_,_,_  ), 0                         , 13 , 0  , 15 , 12 ), // #33
  INST(Blci             , VexVm_Wx           , V(XOP_M9,02,6,0,x,_,_,_  ), 0                         , 14 , 0  , 15 , 12 ), // #34
  INST(Blcic            , VexVm_Wx           , V(XOP_M9,01,5,0,x,_,_,_  ), 0                         , 15 , 0  , 15 , 12 ), // #35
  INST(Blcmsk           , VexVm_Wx           , V(XOP_M9,02,1,0,x,_,_,_  ), 0                         , 13 , 0  , 15 , 12 ), // #36
  INST(Blcs             , VexVm_Wx           , V(XOP_M9,01,3,0,x,_,_,_  ), 0                         , 16 , 0  , 15 , 12 ), // #37
  INST(Blendpd          , ExtRmi             , O(660F3A,0D,_,_,_,_,_,_  ), 0                         , 9  , 0  , 9  , 13 ), // #38
  INST(Blendps          , ExtRmi             , O(660F3A,0C,_,_,_,_,_,_  ), 0                         , 9  , 0  , 9  , 13 ), // #39
  INST(Blendvpd         , ExtRm_XMM0         , O(660F38,15,_,_,_,_,_,_  ), 0                         , 2  , 0  , 16 , 13 ), // #40
  INST(Blendvps         , ExtRm_XMM0         , O(660F38,14,_,_,_,_,_,_  ), 0                         , 2  , 0  , 16 , 13 ), // #41
  INST(Blsfill          , VexVm_Wx           , V(XOP_M9,01,2,0,x,_,_,_  ), 0                         , 17 , 0  , 15 , 12 ), // #42
  INST(Blsi             , VexVm_Wx           , V(000F38,F3,3,0,x,_,_,_  ), 0                         , 18 , 0  , 15 , 10 ), // #43
  INST(Blsic            , VexVm_Wx           , V(XOP_M9,01,6,0,x,_,_,_  ), 0                         , 14 , 0  , 15 , 12 ), // #44
  INST(Blsmsk           , VexVm_Wx           , V(000F38,F3,2,0,x,_,_,_  ), 0                         , 19 , 0  , 15 , 10 ), // #45
  INST(Blsr             , VexVm_Wx           , V(000F38,F3,1,0,x,_,_,_  ), 0                         , 20 , 0  , 15 , 10 ), // #46
  INST(Bndcl            , X86Rm              , O(F30F00,1A,_,_,_,_,_,_  ), 0                         , 7  , 0  , 17 , 14 ), // #47
  INST(Bndcn            , X86Rm              , O(F20F00,1B,_,_,_,_,_,_  ), 0                         , 6  , 0  , 17 , 14 ), // #48
  INST(Bndcu            , X86Rm              , O(F20F00,1A,_,_,_,_,_,_  ), 0                         , 6  , 0  , 17 , 14 ), // #49
  INST(Bndldx           , X86Rm              , O(000F00,1A,_,_,_,_,_,_  ), 0                         , 5  , 0  , 18 , 14 ), // #50
  INST(Bndmk            , X86Rm              , O(F30F00,1B,_,_,_,_,_,_  ), 0                         , 7  , 0  , 19 , 14 ), // #51
  INST(Bndmov           , X86Bndmov          , O(660F00,1A,_,_,_,_,_,_  ), O(660F00,1B,_,_,_,_,_,_  ), 4  , 1  , 20 , 14 ), // #52
  INST(Bndstx           , X86Mr              , O(000F00,1B,_,_,_,_,_,_  ), 0                         , 5  , 0  , 21 , 14 ), // #53
  INST(Bound            , X86Rm              , O(000000,62,_,_,_,_,_,_  ), 0                         , 0  , 0  , 22 , 0  ), // #54
  INST(Bsf              , X86Rm              , O(000F00,BC,_,_,x,_,_,_  ), 0                         , 5  , 0  , 23 , 1  ), // #55
  INST(Bsr              , X86Rm              , O(000F00,BD,_,_,x,_,_,_  ), 0                         , 5  , 0  , 23 , 1  ), // #56
  INST(Bswap            , X86Bswap           , O(000F00,C8,_,_,x,_,_,_  ), 0                         , 5  , 0  , 24 , 0  ), // #57
  INST(Bt               , X86Bt              , O(000F00,A3,_,_,x,_,_,_  ), O(000F00,BA,4,_,x,_,_,_  ), 5  , 2  , 25 , 15 ), // #58
  INST(Btc              , X86Bt              , O(000F00,BB,_,_,x,_,_,_  ), O(000F00,BA,7,_,x,_,_,_  ), 5  , 3  , 26 , 15 ), // #59
  INST(Btr              , X86Bt              , O(000F00,B3,_,_,x,_,_,_  ), O(000F00,BA,6,_,x,_,_,_  ), 5  , 4  , 26 , 15 ), // #60
  INST(Bts              , X86Bt              , O(000F00,AB,_,_,x,_,_,_  ), O(000F00,BA,5,_,x,_,_,_  ), 5  , 5  , 26 , 15 ), // #61
  INST(Bzhi             , VexRmv_Wx          , V(000F38,F5,_,0,x,_,_,_  ), 0                         , 11 , 0  , 14 , 16 ), // #62
  INST(Call             , X86Call            , O(000000,FF,2,_,_,_,_,_  ), 0                         , 3  , 0  , 27 , 1  ), // #63
  INST(Cbw              , X86Op_xAX          , O(660000,98,_,_,_,_,_,_  ), 0                         , 21 , 0  , 28 , 0  ), // #64
  INST(Cdq              , X86Op_xDX_xAX      , O(000000,99,_,_,_,_,_,_  ), 0                         , 0  , 0  , 29 , 0  ), // #65
  INST(Cdqe             , X86Op_xAX          , O(000000,98,_,_,1,_,_,_  ), 0                         , 22 , 0  , 30 , 0  ), // #66
  INST(Clac             , X86Op              , O(000F01,CA,_,_,_,_,_,_  ), 0                         , 23 , 0  , 31 , 17 ), // #67
  INST(Clc              , X86Op              , O(000000,F8,_,_,_,_,_,_  ), 0                         , 0  , 0  , 31 , 18 ), // #68
  INST(Cld              , X86Op              , O(000000,FC,_,_,_,_,_,_  ), 0                         , 0  , 0  , 31 , 19 ), // #69
  INST(Cldemote         , X86M_Only          , O(000F00,1C,0,_,_,_,_,_  ), 0                         , 5  , 0  , 32 , 20 ), // #70
  INST(Clflush          , X86M_Only          , O(000F00,AE,7,_,_,_,_,_  ), 0                         , 24 , 0  , 32 , 21 ), // #71
  INST(Clflushopt       , X86M_Only          , O(660F00,AE,7,_,_,_,_,_  ), 0                         , 25 , 0  , 32 , 22 ), // #72
  INST(Clgi             , X86Op              , O(000F01,DD,_,_,_,_,_,_  ), 0                         , 23 , 0  , 31 , 23 ), // #73
  INST(Cli              , X86Op              , O(000000,FA,_,_,_,_,_,_  ), 0                         , 0  , 0  , 31 , 24 ), // #74
  INST(Clrssbsy         , X86M_Only          , O(F30F00,AE,6,_,_,_,_,_  ), 0                         , 26 , 0  , 33 , 25 ), // #75
  INST(Clts             , X86Op              , O(000F00,06,_,_,_,_,_,_  ), 0                         , 5  , 0  , 31 , 0  ), // #76
  INST(Clui             , X86Op              , O(F30F01,EE,_,_,_,_,_,_  ), 0                         , 27 , 0  , 34 , 26 ), // #77
  INST(Clwb             , X86M_Only          , O(660F00,AE,6,_,_,_,_,_  ), 0                         , 28 , 0  , 32 , 27 ), // #78
  INST(Clzero           , X86Op_MemZAX       , O(000F01,FC,_,_,_,_,_,_  ), 0                         , 23 , 0  , 35 , 28 ), // #79
  INST(Cmc              , X86Op              , O(000000,F5,_,_,_,_,_,_  ), 0                         , 0  , 0  , 31 , 29 ), // #80
  INST(Cmovb            , X86Rm              , O(000F00,42,_,_,x,_,_,_  ), 0                         , 5  , 0  , 23 , 30 ), // #81
  INST(Cmovbe           , X86Rm              , O(000F00,46,_,_,x,_,_,_  ), 0                         , 5  , 0  , 23 , 31 ), // #82
  INST(Cmovl            , X86Rm              , O(000F00,4C,_,_,x,_,_,_  ), 0                         , 5  , 0  , 23 , 32 ), // #83
  INST(Cmovle           , X86Rm              , O(000F00,4E,_,_,x,_,_,_  ), 0                         , 5  , 0  , 23 , 33 ), // #84
  INST(Cmovnb           , X86Rm              , O(000F00,43,_,_,x,_,_,_  ), 0                         , 5  , 0  , 23 , 30 ), // #85
  INST(Cmovnbe          , X86Rm              , O(000F00,47,_,_,x,_,_,_  ), 0                         , 5  , 0  , 23 , 31 ), // #86
  INST(Cmovnl           , X86Rm              , O(000F00,4D,_,_,x,_,_,_  ), 0                         , 5  , 0  , 23 , 32 ), // #87
  INST(Cmovnle          , X86Rm              , O(000F00,4F,_,_,x,_,_,_  ), 0                         , 5  , 0  , 23 , 33 ), // #88
  INST(Cmovno           , X86Rm              , O(000F00,41,_,_,x,_,_,_  ), 0                         , 5  , 0  , 23 , 34 ), // #89
  INST(Cmovnp           , X86Rm              , O(000F00,4B,_,_,x,_,_,_  ), 0                         , 5  , 0  , 23 , 35 ), // #90
  INST(Cmovns           , X86Rm              , O(000F00,49,_,_,x,_,_,_  ), 0                         , 5  , 0  , 23 , 36 ), // #91
  INST(Cmovnz           , X86Rm              , O(000F00,45,_,_,x,_,_,_  ), 0                         , 5  , 0  , 23 , 37 ), // #92
  INST(Cmovo            , X86Rm              , O(000F00,40,_,_,x,_,_,_  ), 0                         , 5  , 0  , 23 , 34 ), // #93
  INST(Cmovp            , X86Rm              , O(000F00,4A,_,_,x,_,_,_  ), 0                         , 5  , 0  , 23 , 35 ), // #94
  INST(Cmovs            , X86Rm              , O(000F00,48,_,_,x,_,_,_  ), 0                         , 5  , 0  , 23 , 36 ), // #95
  INST(Cmovz            , X86Rm              , O(000F00,44,_,_,x,_,_,_  ), 0                         , 5  , 0  , 23 , 37 ), // #96
  INST(Cmp              , X86Arith           , O(000000,38,7,_,x,_,_,_  ), 0                         , 29 , 0  , 36 , 1  ), // #97
  INST(Cmpbexadd        , VexMvr_Wx          , V(660F38,E6,_,0,x,_,_,_  ), 0                         , 30 , 0  , 37 , 38 ), // #98
  INST(Cmpbxadd         , VexMvr_Wx          , V(660F38,E2,_,0,x,_,_,_  ), 0                         , 30 , 0  , 37 , 38 ), // #99
  INST(Cmplexadd        , VexMvr_Wx          , V(660F38,EE,_,0,x,_,_,_  ), 0                         , 30 , 0  , 37 , 38 ), // #100
  INST(Cmplxadd         , VexMvr_Wx          , V(660F38,EC,_,0,x,_,_,_  ), 0                         , 30 , 0  , 37 , 38 ), // #101
  INST(Cmpnbexadd       , VexMvr_Wx          , V(660F38,E7,_,0,x,_,_,_  ), 0                         , 30 , 0  , 37 , 38 ), // #102
  INST(Cmpnbxadd        , VexMvr_Wx          , V(660F38,E3,_,0,x,_,_,_  ), 0                         , 30 , 0  , 37 , 38 ), // #103
  INST(Cmpnlexadd       , VexMvr_Wx          , V(660F38,EF,_,0,x,_,_,_  ), 0                         , 30 , 0  , 37 , 38 ), // #104
  INST(Cmpnlxadd        , VexMvr_Wx          , V(660F38,ED,_,0,x,_,_,_  ), 0                         , 30 , 0  , 37 , 38 ), // #105
  INST(Cmpnoxadd        , VexMvr_Wx          , V(660F38,E1,_,0,x,_,_,_  ), 0                         , 30 , 0  , 37 , 38 ), // #106
  INST(Cmpnpxadd        , VexMvr_Wx          , V(660F38,EB,_,0,x,_,_,_  ), 0                         , 30 , 0  , 37 , 38 ), // #107
  INST(Cmpnsxadd        , VexMvr_Wx          , V(660F38,E9,_,0,x,_,_,_  ), 0                         , 30 , 0  , 37 , 38 ), // #108
  INST(Cmpnzxadd        , VexMvr_Wx          , V(660F38,E5,_,0,x,_,_,_  ), 0                         , 30 , 0  , 37 , 38 ), // #109
  INST(Cmpoxadd         , VexMvr_Wx          , V(660F38,E0,_,0,x,_,_,_  ), 0                         , 30 , 0  , 37 , 38 ), // #110
  INST(Cmppd            , ExtRmi             , O(660F00,C2,_,_,_,_,_,_  ), 0                         , 4  , 0  , 9  , 5  ), // #111
  INST(Cmpps            , ExtRmi             , O(000F00,C2,_,_,_,_,_,_  ), 0                         , 5  , 0  , 9  , 6  ), // #112
  INST(Cmppxadd         , VexMvr_Wx          , V(660F38,EA,_,0,x,_,_,_  ), 0                         , 30 , 0  , 37 , 38 ), // #113
  INST(Cmps             , X86StrMm           , O(000000,A6,_,_,_,_,_,_  ), 0                         , 0  , 0  , 38 , 39 ), // #114
  INST(Cmpsd            , ExtRmi             , O(F20F00,C2,_,_,_,_,_,_  ), 0                         , 6  , 0  , 39 , 5  ), // #115
  INST(Cmpss            , ExtRmi             , O(F30F00,C2,_,_,_,_,_,_  ), 0                         , 7  , 0  , 40 , 6  ), // #116
  INST(Cmpsxadd         , VexMvr_Wx          , V(660F38,E8,_,0,x,_,_,_  ), 0                         , 30 , 0  , 37 , 38 ), // #117
  INST(Cmpxchg          , X86Cmpxchg         , O(000F00,B0,_,_,x,_,_,_  ), 0                         , 5  , 0  , 41 , 40 ), // #118
  INST(Cmpxchg16b       , X86Cmpxchg8b_16b   , O(000F00,C7,1,_,1,_,_,_  ), 0                         , 31 , 0  , 42 , 41 ), // #119
  INST(Cmpxchg8b        , X86Cmpxchg8b_16b   , O(000F00,C7,1,_,_,_,_,_  ), 0                         , 32 , 0  , 43 , 42 ), // #120
  INST(Cmpzxadd         , VexMvr_Wx          , V(660F38,E4,_,0,x,_,_,_  ), 0                         , 30 , 0  , 37 , 38 ), // #121
  INST(Comisd           , ExtRm              , O(660F00,2F,_,_,_,_,_,_  ), 0                         , 4  , 0  , 7  , 43 ), // #122
  INST(Comiss           , ExtRm              , O(000F00,2F,_,_,_,_,_,_  ), 0                         , 5  , 0  , 8  , 44 ), // #123
  INST(Cpuid            , X86Op              , O(000F00,A2,_,_,_,_,_,_  ), 0                         , 5  , 0  , 44 , 45 ), // #124
  INST(Cqo              , X86Op_xDX_xAX      , O(000000,99,_,_,1,_,_,_  ), 0                         , 22 , 0  , 45 , 0  ), // #125
  INST(Crc32            , X86Crc             , O(F20F38,F0,_,_,x,_,_,_  ), 0                         , 12 , 0  , 46 , 46 ), // #126
  INST(Cvtdq2pd         , ExtRm              , O(F30F00,E6,_,_,_,_,_,_  ), 0                         , 7  , 0  , 7  , 5  ), // #127
  INST(Cvtdq2ps         , ExtRm              , O(000F00,5B,_,_,_,_,_,_  ), 0                         , 5  , 0  , 6  , 5  ), // #128
  INST(Cvtpd2dq         , ExtRm              , O(F20F00,E6,_,_,_,_,_,_  ), 0                         , 6  , 0  , 6  , 5  ), // #129
  INST(Cvtpd2pi         , ExtRm              , O(660F00,2D,_,_,_,_,_,_  ), 0                         , 4  , 0  , 47 , 5  ), // #130
  INST(Cvtpd2ps         , ExtRm              , O(660F00,5A,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6  , 5  ), // #131
  INST(Cvtpi2pd         , ExtRm              , O(660F00,2A,_,_,_,_,_,_  ), 0                         , 4  , 0  , 48 , 5  ), // #132
  INST(Cvtpi2ps         , ExtRm              , O(000F00,2A,_,_,_,_,_,_  ), 0                         , 5  , 0  , 48 , 6  ), // #133
  INST(Cvtps2dq         , ExtRm              , O(660F00,5B,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6  , 5  ), // #134
  INST(Cvtps2pd         , ExtRm              , O(000F00,5A,_,_,_,_,_,_  ), 0                         , 5  , 0  , 7  , 5  ), // #135
  INST(Cvtps2pi         , ExtRm              , O(000F00,2D,_,_,_,_,_,_  ), 0                         , 5  , 0  , 49 , 6  ), // #136
  INST(Cvtsd2si         , ExtRm_Wx_GpqOnly   , O(F20F00,2D,_,_,x,_,_,_  ), 0                         , 6  , 0  , 50 , 5  ), // #137
  INST(Cvtsd2ss         , ExtRm              , O(F20F00,5A,_,_,_,_,_,_  ), 0                         , 6  , 0  , 7  , 5  ), // #138
  INST(Cvtsi2sd         , ExtRm_Wx           , O(F20F00,2A,_,_,x,_,_,_  ), 0                         , 6  , 0  , 51 , 5  ), // #139
  INST(Cvtsi2ss         , ExtRm_Wx           , O(F30F00,2A,_,_,x,_,_,_  ), 0                         , 7  , 0  , 51 , 6  ), // #140
  INST(Cvtss2sd         , ExtRm              , O(F30F00,5A,_,_,_,_,_,_  ), 0                         , 7  , 0  , 8  , 5  ), // #141
  INST(Cvtss2si         , ExtRm_Wx_GpqOnly   , O(F30F00,2D,_,_,x,_,_,_  ), 0                         , 7  , 0  , 52 , 6  ), // #142
  INST(Cvttpd2dq        , ExtRm              , O(660F00,E6,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6  , 5  ), // #143
  INST(Cvttpd2pi        , ExtRm              , O(660F00,2C,_,_,_,_,_,_  ), 0                         , 4  , 0  , 47 , 5  ), // #144
  INST(Cvttps2dq        , ExtRm              , O(F30F00,5B,_,_,_,_,_,_  ), 0                         , 7  , 0  , 6  , 5  ), // #145
  INST(Cvttps2pi        , ExtRm              , O(000F00,2C,_,_,_,_,_,_  ), 0                         , 5  , 0  , 49 , 6  ), // #146
  INST(Cvttsd2si        , ExtRm_Wx_GpqOnly   , O(F20F00,2C,_,_,x,_,_,_  ), 0                         , 6  , 0  , 50 , 5  ), // #147
  INST(Cvttss2si        , ExtRm_Wx_GpqOnly   , O(F30F00,2C,_,_,x,_,_,_  ), 0                         , 7  , 0  , 52 , 6  ), // #148
  INST(Cwd              , X86Op_xDX_xAX      , O(660000,99,_,_,_,_,_,_  ), 0                         , 21 , 0  , 53 , 0  ), // #149
  INST(Cwde             , X86Op_xAX          , O(000000,98,_,_,_,_,_,_  ), 0                         , 0  , 0  , 54 , 0  ), // #150
  INST(Daa              , X86Op              , O(000000,27,_,_,_,_,_,_  ), 0                         , 0  , 0  , 1  , 1  ), // #151
  INST(Das              , X86Op              , O(000000,2F,_,_,_,_,_,_  ), 0                         , 0  , 0  , 1  , 1  ), // #152
  INST(Dec              , X86IncDec          , O(000000,FE,1,_,x,_,_,_  ), O(000000,48,_,_,x,_,_,_  ), 33 , 6  , 55 , 47 ), // #153
  INST(Div              , X86M_GPB_MulDiv    , O(000000,F6,6,_,x,_,_,_  ), 0                         , 34 , 0  , 56 , 1  ), // #154
  INST(Divpd            , ExtRm              , O(660F00,5E,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6  , 5  ), // #155
  INST(Divps            , ExtRm              , O(000F00,5E,_,_,_,_,_,_  ), 0                         , 5  , 0  , 6  , 6  ), // #156
  INST(Divsd            , ExtRm              , O(F20F00,5E,_,_,_,_,_,_  ), 0                         , 6  , 0  , 7  , 5  ), // #157
  INST(Divss            , ExtRm              , O(F30F00,5E,_,_,_,_,_,_  ), 0                         , 7  , 0  , 8  , 6  ), // #158
  INST(Dppd             , ExtRmi             , O(660F3A,41,_,_,_,_,_,_  ), 0                         , 9  , 0  , 9  , 13 ), // #159
  INST(Dpps             , ExtRmi             , O(660F3A,40,_,_,_,_,_,_  ), 0                         , 9  , 0  , 9  , 13 ), // #160
  INST(Emms             , X86Op              , O(000F00,77,_,_,_,_,_,_  ), 0                         , 5  , 0  , 57 , 48 ), // #161
  INST(Endbr32          , X86Op_Mod11RM      , O(F30F00,1E,7,_,_,_,_,3  ), 0                         , 35 , 0  , 31 , 49 ), // #162
  INST(Endbr64          , X86Op_Mod11RM      , O(F30F00,1E,7,_,_,_,_,2  ), 0                         , 36 , 0  , 31 , 49 ), // #163
  INST(Enqcmd           , X86EnqcmdMovdir64b , O(F20F38,F8,_,_,_,_,_,_  ), 0                         , 12 , 0  , 58 , 50 ), // #164
  INST(Enqcmds          , X86EnqcmdMovdir64b , O(F30F38,F8,_,_,_,_,_,_  ), 0                         , 8  , 0  , 58 , 50 ), // #165
  INST(Enter            , X86Enter           , O(000000,C8,_,_,_,_,_,_  ), 0                         , 0  , 0  , 59 , 0  ), // #166
  INST(Extractps        , ExtExtract         , O(660F3A,17,_,_,_,_,_,_  ), 0                         , 9  , 0  , 60 , 13 ), // #167
  INST(Extrq            , ExtExtrq           , O(660F00,79,_,_,_,_,_,_  ), O(660F00,78,0,_,_,_,_,_  ), 4  , 7  , 61 , 51 ), // #168
  INST(F2xm1            , FpuOp              , O_FPU(00,D9F0,_)          , 0                         , 37 , 0  , 31 , 52 ), // #169
  INST(Fabs             , FpuOp              , O_FPU(00,D9E1,_)          , 0                         , 37 , 0  , 31 , 52 ), // #170
  INST(Fadd             , FpuArith           , O_FPU(00,C0C0,0)          , 0                         , 38 , 0  , 62 , 52 ), // #171
  INST(Faddp            , FpuRDef            , O_FPU(00,DEC0,_)          , 0                         , 39 , 0  , 63 , 52 ), // #172
  INST(Fbld             , X86M_Only          , O_FPU(00,00DF,4)          , 0                         , 40 , 0  , 64 , 52 ), // #173
  INST(Fbstp            , X86M_Only          , O_FPU(00,00DF,6)          , 0                         , 41 , 0  , 64 , 52 ), // #174
  INST(Fchs             , FpuOp              , O_FPU(00,D9E0,_)          , 0                         , 37 , 0  , 31 , 52 ), // #175
  INST(Fclex            , FpuOp              , O_FPU(9B,DBE2,_)          , 0                         , 42 , 0  , 31 , 52 ), // #176
  INST(Fcmovb           , FpuR               , O_FPU(00,DAC0,_)          , 0                         , 43 , 0  , 65 , 53 ), // #177
  INST(Fcmovbe          , FpuR               , O_FPU(00,DAD0,_)          , 0                         , 43 , 0  , 65 , 54 ), // #178
  INST(Fcmove           , FpuR               , O_FPU(00,DAC8,_)          , 0                         , 43 , 0  , 65 , 55 ), // #179
  INST(Fcmovnb          , FpuR               , O_FPU(00,DBC0,_)          , 0                         , 44 , 0  , 65 , 53 ), // #180
  INST(Fcmovnbe         , FpuR               , O_FPU(00,DBD0,_)          , 0                         , 44 , 0  , 65 , 54 ), // #181
  INST(Fcmovne          , FpuR               , O_FPU(00,DBC8,_)          , 0                         , 44 , 0  , 65 , 55 ), // #182
  INST(Fcmovnu          , FpuR               , O_FPU(00,DBD8,_)          , 0                         , 44 , 0  , 65 , 56 ), // #183
  INST(Fcmovu           , FpuR               , O_FPU(00,DAD8,_)          , 0                         , 43 , 0  , 65 , 56 ), // #184
  INST(Fcom             , FpuCom             , O_FPU(00,D0D0,2)          , 0                         , 45 , 0  , 66 , 52 ), // #185
  INST(Fcomi            , FpuR               , O_FPU(00,DBF0,_)          , 0                         , 44 , 0  , 65 , 57 ), // #186
  INST(Fcomip           , FpuR               , O_FPU(00,DFF0,_)          , 0                         , 46 , 0  , 65 , 57 ), // #187
  INST(Fcomp            , FpuCom             , O_FPU(00,D8D8,3)          , 0                         , 47 , 0  , 66 , 52 ), // #188
  INST(Fcompp           , FpuOp              , O_FPU(00,DED9,_)          , 0                         , 39 , 0  , 31 , 52 ), // #189
  INST(Fcos             , FpuOp              , O_FPU(00,D9FF,_)          , 0                         , 37 , 0  , 31 , 52 ), // #190
  INST(Fdecstp          , FpuOp              , O_FPU(00,D9F6,_)          , 0                         , 37 , 0  , 31 , 52 ), // #191
  INST(Fdiv             , FpuArith           , O_FPU(00,F0F8,6)          , 0                         , 48 , 0  , 62 , 52 ), // #192
  INST(Fdivp            , FpuRDef            , O_FPU(00,DEF8,_)          , 0                         , 39 , 0  , 63 , 52 ), // #193
  INST(Fdivr            , FpuArith           , O_FPU(00,F8F0,7)          , 0                         , 49 , 0  , 62 , 52 ), // #194
  INST(Fdivrp           , FpuRDef            , O_FPU(00,DEF0,_)          , 0                         , 39 , 0  , 63 , 52 ), // #195
  INST(Femms            , X86Op              , O(000F00,0E,_,_,_,_,_,_  ), 0                         , 5  , 0  , 31 , 58 ), // #196
  INST(Ffree            , FpuR               , O_FPU(00,DDC0,_)          , 0                         , 50 , 0  , 65 , 52 ), // #197
  INST(Fiadd            , FpuM               , O_FPU(00,00DA,0)          , 0                         , 51 , 0  , 67 , 52 ), // #198
  INST(Ficom            , FpuM               , O_FPU(00,00DA,2)          , 0                         , 52 , 0  , 67 , 52 ), // #199
  INST(Ficomp           , FpuM               , O_FPU(00,00DA,3)          , 0                         , 53 , 0  , 67 , 52 ), // #200
  INST(Fidiv            , FpuM               , O_FPU(00,00DA,6)          , 0                         , 41 , 0  , 67 , 52 ), // #201
  INST(Fidivr           , FpuM               , O_FPU(00,00DA,7)          , 0                         , 54 , 0  , 67 , 52 ), // #202
  INST(Fild             , FpuM               , O_FPU(00,00DB,0)          , O_FPU(00,00DF,5)          , 51 , 8  , 68 , 52 ), // #203
  INST(Fimul            , FpuM               , O_FPU(00,00DA,1)          , 0                         , 55 , 0  , 67 , 52 ), // #204
  INST(Fincstp          , FpuOp              , O_FPU(00,D9F7,_)          , 0                         , 37 , 0  , 31 , 52 ), // #205
  INST(Finit            , FpuOp              , O_FPU(9B,DBE3,_)          , 0                         , 42 , 0  , 31 , 52 ), // #206
  INST(Fist             , FpuM               , O_FPU(00,00DB,2)          , 0                         , 52 , 0  , 67 , 52 ), // #207
  INST(Fistp            , FpuM               , O_FPU(00,00DB,3)          , O_FPU(00,00DF,7)          , 53 , 9  , 68 , 52 ), // #208
  INST(Fisttp           , FpuM               , O_FPU(00,00DB,1)          , O_FPU(00,00DD,1)          , 55 , 10 , 68 , 59 ), // #209
  INST(Fisub            , FpuM               , O_FPU(00,00DA,4)          , 0                         , 40 , 0  , 67 , 52 ), // #210
  INST(Fisubr           , FpuM               , O_FPU(00,00DA,5)          , 0                         , 56 , 0  , 67 , 52 ), // #211
  INST(Fld              , FpuFldFst          , O_FPU(00,00D9,0)          , O_FPU(00,00DB,5)          , 51 , 11 , 69 , 52 ), // #212
  INST(Fld1             , FpuOp              , O_FPU(00,D9E8,_)          , 0                         , 37 , 0  , 31 , 52 ), // #213
  INST(Fldcw            , X86M_Only          , O_FPU(00,00D9,5)          , 0                         , 56 , 0  , 70 , 52 ), // #214
  INST(Fldenv           , X86M_Only          , O_FPU(00,00D9,4)          , 0                         , 40 , 0  , 32 , 52 ), // #215
  INST(Fldl2e           , FpuOp              , O_FPU(00,D9EA,_)          , 0                         , 37 , 0  , 31 , 52 ), // #216
  INST(Fldl2t           , FpuOp              , O_FPU(00,D9E9,_)          , 0                         , 37 , 0  , 31 , 52 ), // #217
  INST(Fldlg2           , FpuOp              , O_FPU(00,D9EC,_)          , 0                         , 37 , 0  , 31 , 52 ), // #218
  INST(Fldln2           , FpuOp              , O_FPU(00,D9ED,_)          , 0                         , 37 , 0  , 31 , 52 ), // #219
  INST(Fldpi            , FpuOp              , O_FPU(00,D9EB,_)          , 0                         , 37 , 0  , 31 , 52 ), // #220
  INST(Fldz             , FpuOp              , O_FPU(00,D9EE,_)          , 0                         , 37 , 0  , 31 , 52 ), // #221
  INST(Fmul             , FpuArith           , O_FPU(00,C8C8,1)          , 0                         , 57 , 0  , 62 , 52 ), // #222
  INST(Fmulp            , FpuRDef            , O_FPU(00,DEC8,_)          , 0                         , 39 , 0  , 63 , 52 ), // #223
  INST(Fnclex           , FpuOp              , O_FPU(00,DBE2,_)          , 0                         , 44 , 0  , 31 , 52 ), // #224
  INST(Fninit           , FpuOp              , O_FPU(00,DBE3,_)          , 0                         , 44 , 0  , 31 , 52 ), // #225
  INST(Fnop             , FpuOp              , O_FPU(00,D9D0,_)          , 0                         , 37 , 0  , 31 , 52 ), // #226
  INST(Fnsave           , X86M_Only          , O_FPU(00,00DD,6)          , 0                         , 41 , 0  , 32 , 52 ), // #227
  INST(Fnstcw           , X86M_Only          , O_FPU(00,00D9,7)          , 0                         , 54 , 0  , 70 , 52 ), // #228
  INST(Fnstenv          , X86M_Only          , O_FPU(00,00D9,6)          , 0                         , 41 , 0  , 32 , 52 ), // #229
  INST(Fnstsw           , FpuStsw            , O_FPU(00,00DD,7)          , O_FPU(00,DFE0,_)          , 54 , 12 , 71 , 52 ), // #230
  INST(Fpatan           , FpuOp              , O_FPU(00,D9F3,_)          , 0                         , 37 , 0  , 31 , 52 ), // #231
  INST(Fprem            , FpuOp              , O_FPU(00,D9F8,_)          , 0                         , 37 , 0  , 31 , 52 ), // #232
  INST(Fprem1           , FpuOp              , O_FPU(00,D9F5,_)          , 0                         , 37 , 0  , 31 , 52 ), // #233
  INST(Fptan            , FpuOp              , O_FPU(00,D9F2,_)          , 0                         , 37 , 0  , 31 , 52 ), // #234
  INST(Frndint          , FpuOp              , O_FPU(00,D9FC,_)          , 0                         , 37 , 0  , 31 , 52 ), // #235
  INST(Frstor           , X86M_Only          , O_FPU(00,00DD,4)          , 0                         , 40 , 0  , 32 , 52 ), // #236
  INST(Fsave            , X86M_Only          , O_FPU(9B,00DD,6)          , 0                         , 58 , 0  , 32 , 52 ), // #237
  INST(Fscale           , FpuOp              , O_FPU(00,D9FD,_)          , 0                         , 37 , 0  , 31 , 52 ), // #238
  INST(Fsin             , FpuOp              , O_FPU(00,D9FE,_)          , 0                         , 37 , 0  , 31 , 52 ), // #239
  INST(Fsincos          , FpuOp              , O_FPU(00,D9FB,_)          , 0                         , 37 , 0  , 31 , 52 ), // #240
  INST(Fsqrt            , FpuOp              , O_FPU(00,D9FA,_)          , 0                         , 37 , 0  , 31 , 52 ), // #241
  INST(Fst              , FpuFldFst          , O_FPU(00,00D9,2)          , 0                         , 52 , 0  , 72 , 52 ), // #242
  INST(Fstcw            , X86M_Only          , O_FPU(9B,00D9,7)          , 0                         , 59 , 0  , 70 , 52 ), // #243
  INST(Fstenv           , X86M_Only          , O_FPU(9B,00D9,6)          , 0                         , 58 , 0  , 32 , 52 ), // #244
  INST(Fstp             , FpuFldFst          , O_FPU(00,00D9,3)          , O(000000,DB,7,_,_,_,_,_  ), 53 , 13 , 69 , 52 ), // #245
  INST(Fstsw            , FpuStsw            , O_FPU(9B,00DD,7)          , O_FPU(9B,DFE0,_)          , 59 , 14 , 71 , 52 ), // #246
  INST(Fsub             , FpuArith           , O_FPU(00,E0E8,4)          , 0                         , 60 , 0  , 62 , 52 ), // #247
  INST(Fsubp            , FpuRDef            , O_FPU(00,DEE8,_)          , 0                         , 39 , 0  , 63 , 52 ), // #248
  INST(Fsubr            , FpuArith           , O_FPU(00,E8E0,5)          , 0                         , 61 , 0  , 62 , 52 ), // #249
  INST(Fsubrp           , FpuRDef            , O_FPU(00,DEE0,_)          , 0                         , 39 , 0  , 63 , 52 ), // #250
  INST(Ftst             , FpuOp              , O_FPU(00,D9E4,_)          , 0                         , 37 , 0  , 31 , 52 ), // #251
  INST(Fucom            , FpuRDef            , O_FPU(00,DDE0,_)          , 0                         , 50 , 0  , 63 , 52 ), // #252
  INST(Fucomi           , FpuR               , O_FPU(00,DBE8,_)          , 0                         , 44 , 0  , 65 , 57 ), // #253
  INST(Fucomip          , FpuR               , O_FPU(00,DFE8,_)          , 0                         , 46 , 0  , 65 , 57 ), // #254
  INST(Fucomp           , FpuRDef            , O_FPU(00,DDE8,_)          , 0                         , 50 , 0  , 63 , 52 ), // #255
  INST(Fucompp          , FpuOp              , O_FPU(00,DAE9,_)          , 0                         , 43 , 0  , 31 , 52 ), // #256
  INST(Fwait            , X86Op              , O_FPU(00,009B,_)          , 0                         , 51 , 0  , 31 , 52 ), // #257
  INST(Fxam             , FpuOp              , O_FPU(00,D9E5,_)          , 0                         , 37 , 0  , 31 , 52 ), // #258
  INST(Fxch             , FpuR               , O_FPU(00,D9C8,_)          , 0                         , 37 , 0  , 63 , 52 ), // #259
  INST(Fxrstor          , X86M_Only          , O(000F00,AE,1,_,_,_,_,_  ), 0                         , 32 , 0  , 32 , 60 ), // #260
  INST(Fxrstor64        , X86M_Only          , O(000F00,AE,1,_,1,_,_,_  ), 0                         , 31 , 0  , 73 , 60 ), // #261
  INST(Fxsave           , X86M_Only          , O(000F00,AE,0,_,_,_,_,_  ), 0                         , 5  , 0  , 32 , 61 ), // #262
  INST(Fxsave64         , X86M_Only          , O(000F00,AE,0,_,1,_,_,_  ), 0                         , 62 , 0  , 73 , 61 ), // #263
  INST(Fxtract          , FpuOp              , O_FPU(00,D9F4,_)          , 0                         , 37 , 0  , 31 , 52 ), // #264
  INST(Fyl2x            , FpuOp              , O_FPU(00,D9F1,_)          , 0                         , 37 , 0  , 31 , 52 ), // #265
  INST(Fyl2xp1          , FpuOp              , O_FPU(00,D9F9,_)          , 0                         , 37 , 0  , 31 , 52 ), // #266
  INST(Getsec           , X86Op              , O(000F00,37,_,_,_,_,_,_  ), 0                         , 5  , 0  , 74 , 62 ), // #267
  INST(Gf2p8affineinvqb , ExtRmi             , O(660F3A,CF,_,_,_,_,_,_  ), 0                         , 9  , 0  , 9  , 63 ), // #268
  INST(Gf2p8affineqb    , ExtRmi             , O(660F3A,CE,_,_,_,_,_,_  ), 0                         , 9  , 0  , 9  , 63 ), // #269
  INST(Gf2p8mulb        , ExtRm              , O(660F38,CF,_,_,_,_,_,_  ), 0                         , 2  , 0  , 6  , 63 ), // #270
  INST(Haddpd           , ExtRm              , O(660F00,7C,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6  , 7  ), // #271
  INST(Haddps           , ExtRm              , O(F20F00,7C,_,_,_,_,_,_  ), 0                         , 6  , 0  , 6  , 7  ), // #272
  INST(Hlt              , X86Op              , O(000000,F4,_,_,_,_,_,_  ), 0                         , 0  , 0  , 31 , 0  ), // #273
  INST(Hreset           , X86Op_Mod11RM_I8   , O(F30F3A,F0,0,_,_,_,_,_  ), 0                         , 63 , 0  , 75 , 64 ), // #274
  INST(Hsubpd           , ExtRm              , O(660F00,7D,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6  , 7  ), // #275
  INST(Hsubps           , ExtRm              , O(F20F00,7D,_,_,_,_,_,_  ), 0                         , 6  , 0  , 6  , 7  ), // #276
  INST(Idiv             , X86M_GPB_MulDiv    , O(000000,F6,7,_,x,_,_,_  ), 0                         , 29 , 0  , 56 , 1  ), // #277
  INST(Imul             , X86Imul            , O(000000,F6,5,_,x,_,_,_  ), 0                         , 64 , 0  , 76 , 1  ), // #278
  INST(In               , X86In              , O(000000,EC,_,_,_,_,_,_  ), O(000000,E4,_,_,_,_,_,_  ), 0  , 15 , 77 , 0  ), // #279
  INST(Inc              , X86IncDec          , O(000000,FE,0,_,x,_,_,_  ), O(000000,40,_,_,x,_,_,_  ), 0  , 16 , 78 , 47 ), // #280
  INST(Incsspd          , X86M               , O(F30F00,AE,5,_,0,_,_,_  ), 0                         , 65 , 0  , 79 , 65 ), // #281
  INST(Incsspq          , X86M               , O(F30F00,AE,5,_,1,_,_,_  ), 0                         , 66 , 0  , 80 , 65 ), // #282
  INST(Ins              , X86Ins             , O(000000,6C,_,_,_,_,_,_  ), 0                         , 0  , 0  , 81 , 0  ), // #283
  INST(Insertps         , ExtRmi             , O(660F3A,21,_,_,_,_,_,_  ), 0                         , 9  , 0  , 40 , 13 ), // #284
  INST(Insertq          , ExtInsertq         , O(F20F00,79,_,_,_,_,_,_  ), O(F20F00,78,_,_,_,_,_,_  ), 6  , 17 , 82 , 51 ), // #285
  INST(Int              , X86Int             , O(000000,CD,_,_,_,_,_,_  ), 0                         , 0  , 0  , 83 , 0  ), // #286
  INST(Int3             , X86Op              , O(000000,CC,_,_,_,_,_,_  ), 0                         , 0  , 0  , 31 , 0  ), // #287
  INST(Into             , X86Op              , O(000000,CE,_,_,_,_,_,_  ), 0                         , 0  , 0  , 84 , 66 ), // #288
  INST(Invd             , X86Op              , O(000F00,08,_,_,_,_,_,_  ), 0                         , 5  , 0  , 31 , 45 ), // #289
  INST(Invept           , X86Rm_NoSize       , O(660F38,80,_,_,_,_,_,_  ), 0                         , 2  , 0  , 85 , 67 ), // #290
  INST(Invlpg           , X86M_Only          , O(000F00,01,7,_,_,_,_,_  ), 0                         , 24 , 0  , 32 , 45 ), // #291
  INST(Invlpga          , X86Op_xAddr        , O(000F01,DF,_,_,_,_,_,_  ), 0                         , 23 , 0  , 86 , 23 ), // #292
  INST(Invlpgb          , X86Op              , O(000F01,FE,_,_,_,_,_,_  ), 0                         , 23 , 0  , 87 , 68 ), // #293
  INST(Invpcid          , X86Rm_NoSize       , O(660F38,82,_,_,_,_,_,_  ), 0                         , 2  , 0  , 85 , 45 ), // #294
  INST(Invvpid          , X86Rm_NoSize       , O(660F38,81,_,_,_,_,_,_  ), 0                         , 2  , 0  , 85 , 67 ), // #295
  INST(Iret             , X86Op              , O(660000,CF,_,_,_,_,_,_  ), 0                         , 21 , 0  , 88 , 1  ), // #296
  INST(Iretd            , X86Op              , O(000000,CF,_,_,_,_,_,_  ), 0                         , 0  , 0  , 88 , 1  ), // #297
  INST(Iretq            , X86Op              , O(000000,CF,_,_,1,_,_,_  ), 0                         , 22 , 0  , 89 , 1  ), // #298
  INST(Jb               , X86Jcc             , O(000F00,82,_,_,_,_,_,_  ), O(000000,72,_,_,_,_,_,_  ), 5  , 18 , 90 , 69 ), // #299
  INST(Jbe              , X86Jcc             , O(000F00,86,_,_,_,_,_,_  ), O(000000,76,_,_,_,_,_,_  ), 5  , 19 , 90 , 70 ), // #300
  INST(Jecxz            , X86JecxzLoop       , 0                         , O(000000,E3,_,_,_,_,_,_  ), 0  , 20 , 91 , 0  ), // #301
  INST(Jl               , X86Jcc             , O(000F00,8C,_,_,_,_,_,_  ), O(000000,7C,_,_,_,_,_,_  ), 5  , 21 , 90 , 71 ), // #302
  INST(Jle              , X86Jcc             , O(000F00,8E,_,_,_,_,_,_  ), O(000000,7E,_,_,_,_,_,_  ), 5  , 22 , 90 , 72 ), // #303
  INST(Jmp              , X86Jmp             , O(000000,FF,4,_,_,_,_,_  ), O(000000,EB,_,_,_,_,_,_  ), 10 , 23 , 92 , 0  ), // #304
  INST(Jnb              , X86Jcc             , O(000F00,83,_,_,_,_,_,_  ), O(000000,73,_,_,_,_,_,_  ), 5  , 24 , 90 , 69 ), // #305
  INST(Jnbe             , X86Jcc             , O(000F00,87,_,_,_,_,_,_  ), O(000000,77,_,_,_,_,_,_  ), 5  , 25 , 90 , 70 ), // #306
  INST(Jnl              , X86Jcc             , O(000F00,8D,_,_,_,_,_,_  ), O(000000,7D,_,_,_,_,_,_  ), 5  , 26 , 90 , 71 ), // #307
  INST(Jnle             , X86Jcc             , O(000F00,8F,_,_,_,_,_,_  ), O(000000,7F,_,_,_,_,_,_  ), 5  , 27 , 90 , 72 ), // #308
  INST(Jno              , X86Jcc             , O(000F00,81,_,_,_,_,_,_  ), O(000000,71,_,_,_,_,_,_  ), 5  , 28 , 90 , 66 ), // #309
  INST(Jnp              , X86Jcc             , O(000F00,8B,_,_,_,_,_,_  ), O(000000,7B,_,_,_,_,_,_  ), 5  , 29 , 90 , 73 ), // #310
  INST(Jns              , X86Jcc             , O(000F00,89,_,_,_,_,_,_  ), O(000000,79,_,_,_,_,_,_  ), 5  , 30 , 90 , 74 ), // #311
  INST(Jnz              , X86Jcc             , O(000F00,85,_,_,_,_,_,_  ), O(000000,75,_,_,_,_,_,_  ), 5  , 31 , 90 , 75 ), // #312
  INST(Jo               , X86Jcc             , O(000F00,80,_,_,_,_,_,_  ), O(000000,70,_,_,_,_,_,_  ), 5  , 32 , 90 , 66 ), // #313
  INST(Jp               , X86Jcc             , O(000F00,8A,_,_,_,_,_,_  ), O(000000,7A,_,_,_,_,_,_  ), 5  , 33 , 90 , 73 ), // #314
  INST(Js               , X86Jcc             , O(000F00,88,_,_,_,_,_,_  ), O(000000,78,_,_,_,_,_,_  ), 5  , 34 , 90 , 74 ), // #315
  INST(Jz               , X86Jcc             , O(000F00,84,_,_,_,_,_,_  ), O(000000,74,_,_,_,_,_,_  ), 5  , 35 , 90 , 75 ), // #316
  INST(Kaddb            , VexRvm             , V(660F00,4A,_,1,0,_,_,_  ), 0                         , 67 , 0  , 93 , 76 ), // #317
  INST(Kaddd            , VexRvm             , V(660F00,4A,_,1,1,_,_,_  ), 0                         , 68 , 0  , 93 , 77 ), // #318
  INST(Kaddq            , VexRvm             , V(000F00,4A,_,1,1,_,_,_  ), 0                         , 69 , 0  , 93 , 77 ), // #319
  INST(Kaddw            , VexRvm             , V(000F00,4A,_,1,0,_,_,_  ), 0                         , 70 , 0  , 93 , 76 ), // #320
  INST(Kandb            , VexRvm             , V(660F00,41,_,1,0,_,_,_  ), 0                         , 67 , 0  , 93 , 76 ), // #321
  INST(Kandd            , VexRvm             , V(660F00,41,_,1,1,_,_,_  ), 0                         , 68 , 0  , 93 , 77 ), // #322
  INST(Kandnb           , VexRvm             , V(660F00,42,_,1,0,_,_,_  ), 0                         , 67 , 0  , 93 , 76 ), // #323
  INST(Kandnd           , VexRvm             , V(660F00,42,_,1,1,_,_,_  ), 0                         , 68 , 0  , 93 , 77 ), // #324
  INST(Kandnq           , VexRvm             , V(000F00,42,_,1,1,_,_,_  ), 0                         , 69 , 0  , 93 , 77 ), // #325
  INST(Kandnw           , VexRvm             , V(000F00,42,_,1,0,_,_,_  ), 0                         , 70 , 0  , 93 , 78 ), // #326
  INST(Kandq            , VexRvm             , V(000F00,41,_,1,1,_,_,_  ), 0                         , 69 , 0  , 93 , 77 ), // #327
  INST(Kandw            , VexRvm             , V(000F00,41,_,1,0,_,_,_  ), 0                         , 70 , 0  , 93 , 78 ), // #328
  INST(Kmovb            , VexKmov            , V(660F00,90,_,0,0,_,_,_  ), V(660F00,92,_,0,0,_,_,_  ), 71 , 36 , 94 , 79 ), // #329
  INST(Kmovd            , VexKmov            , V(660F00,90,_,0,1,_,_,_  ), V(F20F00,92,_,0,0,_,_,_  ), 72 , 37 , 95 , 80 ), // #330
  INST(Kmovq            , VexKmov            , V(000F00,90,_,0,1,_,_,_  ), V(F20F00,92,_,0,1,_,_,_  ), 73 , 38 , 96 , 80 ), // #331
  INST(Kmovw            , VexKmov            , V(000F00,90,_,0,0,_,_,_  ), V(000F00,92,_,0,0,_,_,_  ), 74 , 39 , 97 , 81 ), // #332
  INST(Knotb            , VexRm              , V(660F00,44,_,0,0,_,_,_  ), 0                         , 71 , 0  , 98 , 76 ), // #333
  INST(Knotd            , VexRm              , V(660F00,44,_,0,1,_,_,_  ), 0                         , 72 , 0  , 98 , 77 ), // #334
  INST(Knotq            , VexRm              , V(000F00,44,_,0,1,_,_,_  ), 0                         , 73 , 0  , 98 , 77 ), // #335
  INST(Knotw            , VexRm              , V(000F00,44,_,0,0,_,_,_  ), 0                         , 74 , 0  , 98 , 78 ), // #336
  INST(Korb             , VexRvm             , V(660F00,45,_,1,0,_,_,_  ), 0                         , 67 , 0  , 93 , 76 ), // #337
  INST(Kord             , VexRvm             , V(660F00,45,_,1,1,_,_,_  ), 0                         , 68 , 0  , 93 , 77 ), // #338
  INST(Korq             , VexRvm             , V(000F00,45,_,1,1,_,_,_  ), 0                         , 69 , 0  , 93 , 77 ), // #339
  INST(Kortestb         , VexRm              , V(660F00,98,_,0,0,_,_,_  ), 0                         , 71 , 0  , 98 , 82 ), // #340
  INST(Kortestd         , VexRm              , V(660F00,98,_,0,1,_,_,_  ), 0                         , 72 , 0  , 98 , 83 ), // #341
  INST(Kortestq         , VexRm              , V(000F00,98,_,0,1,_,_,_  ), 0                         , 73 , 0  , 98 , 83 ), // #342
  INST(Kortestw         , VexRm              , V(000F00,98,_,0,0,_,_,_  ), 0                         , 74 , 0  , 98 , 84 ), // #343
  INST(Korw             , VexRvm             , V(000F00,45,_,1,0,_,_,_  ), 0                         , 70 , 0  , 93 , 78 ), // #344
  INST(Kshiftlb         , VexRmi             , V(660F3A,32,_,0,0,_,_,_  ), 0                         , 75 , 0  , 99 , 76 ), // #345
  INST(Kshiftld         , VexRmi             , V(660F3A,33,_,0,0,_,_,_  ), 0                         , 75 , 0  , 99 , 77 ), // #346
  INST(Kshiftlq         , VexRmi             , V(660F3A,33,_,0,1,_,_,_  ), 0                         , 76 , 0  , 99 , 77 ), // #347
  INST(Kshiftlw         , VexRmi             , V(660F3A,32,_,0,1,_,_,_  ), 0                         , 76 , 0  , 99 , 78 ), // #348
  INST(Kshiftrb         , VexRmi             , V(660F3A,30,_,0,0,_,_,_  ), 0                         , 75 , 0  , 99 , 76 ), // #349
  INST(Kshiftrd         , VexRmi             , V(660F3A,31,_,0,0,_,_,_  ), 0                         , 75 , 0  , 99 , 77 ), // #350
  INST(Kshiftrq         , VexRmi             , V(660F3A,31,_,0,1,_,_,_  ), 0                         , 76 , 0  , 99 , 77 ), // #351
  INST(Kshiftrw         , VexRmi             , V(660F3A,30,_,0,1,_,_,_  ), 0                         , 76 , 0  , 99 , 78 ), // #352
  INST(Ktestb           , VexRm              , V(660F00,99,_,0,0,_,_,_  ), 0                         , 71 , 0  , 98 , 82 ), // #353
  INST(Ktestd           , VexRm              , V(660F00,99,_,0,1,_,_,_  ), 0                         , 72 , 0  , 98 , 83 ), // #354
  INST(Ktestq           , VexRm              , V(000F00,99,_,0,1,_,_,_  ), 0                         , 73 , 0  , 98 , 83 ), // #355
  INST(Ktestw           , VexRm              , V(000F00,99,_,0,0,_,_,_  ), 0                         , 74 , 0  , 98 , 82 ), // #356
  INST(Kunpckbw         , VexRvm             , V(660F00,4B,_,1,0,_,_,_  ), 0                         , 67 , 0  , 93 , 78 ), // #357
  INST(Kunpckdq         , VexRvm             , V(000F00,4B,_,1,1,_,_,_  ), 0                         , 69 , 0  , 93 , 77 ), // #358
  INST(Kunpckwd         , VexRvm             , V(000F00,4B,_,1,0,_,_,_  ), 0                         , 70 , 0  , 93 , 77 ), // #359
  INST(Kxnorb           , VexRvm             , V(660F00,46,_,1,0,_,_,_  ), 0                         , 67 , 0  , 100, 76 ), // #360
  INST(Kxnord           , VexRvm             , V(660F00,46,_,1,1,_,_,_  ), 0                         , 68 , 0  , 100, 77 ), // #361
  INST(Kxnorq           , VexRvm             , V(000F00,46,_,1,1,_,_,_  ), 0                         , 69 , 0  , 100, 77 ), // #362
  INST(Kxnorw           , VexRvm             , V(000F00,46,_,1,0,_,_,_  ), 0                         , 70 , 0  , 100, 78 ), // #363
  INST(Kxorb            , VexRvm             , V(660F00,47,_,1,0,_,_,_  ), 0                         , 67 , 0  , 100, 76 ), // #364
  INST(Kxord            , VexRvm             , V(660F00,47,_,1,1,_,_,_  ), 0                         , 68 , 0  , 100, 77 ), // #365
  INST(Kxorq            , VexRvm             , V(000F00,47,_,1,1,_,_,_  ), 0                         , 69 , 0  , 100, 77 ), // #366
  INST(Kxorw            , VexRvm             , V(000F00,47,_,1,0,_,_,_  ), 0                         , 70 , 0  , 100, 78 ), // #367
  INST(Lahf             , X86Op              , O(000000,9F,_,_,_,_,_,_  ), 0                         , 0  , 0  , 101, 85 ), // #368
  INST(Lar              , X86Rm              , O(000F00,02,_,_,_,_,_,_  ), 0                         , 5  , 0  , 102, 11 ), // #369
  INST(Lcall            , X86LcallLjmp       , O(000000,FF,3,_,_,_,_,_  ), O(000000,9A,_,_,_,_,_,_  ), 77 , 40 , 103, 1  ), // #370
  INST(Lddqu            , ExtRm              , O(F20F00,F0,_,_,_,_,_,_  ), 0                         , 6  , 0  , 104, 7  ), // #371
  INST(Ldmxcsr          , X86M_Only          , O(000F00,AE,2,_,_,_,_,_  ), 0                         , 78 , 0  , 105, 6  ), // #372
  INST(Lds              , X86Rm              , O(000000,C5,_,_,_,_,_,_  ), 0                         , 0  , 0  , 106, 0  ), // #373
  INST(Ldtilecfg        , AmxCfg             , V(000F38,49,_,0,0,_,_,_  ), 0                         , 11 , 0  , 107, 86 ), // #374
  INST(Lea              , X86Lea             , O(000000,8D,_,_,x,_,_,_  ), 0                         , 0  , 0  , 108, 0  ), // #375
  INST(Leave            , X86Op              , O(000000,C9,_,_,_,_,_,_  ), 0                         , 0  , 0  , 31 , 0  ), // #376
  INST(Les              , X86Rm              , O(000000,C4,_,_,_,_,_,_  ), 0                         , 0  , 0  , 106, 0  ), // #377
  INST(Lfence           , X86Fence           , O(000F00,AE,5,_,_,_,_,_  ), 0                         , 79 , 0  , 31 , 5  ), // #378
  INST(Lfs              , X86Rm              , O(000F00,B4,_,_,_,_,_,_  ), 0                         , 5  , 0  , 109, 0  ), // #379
  INST(Lgdt             , X86M_Only          , O(000F00,01,2,_,_,_,_,_  ), 0                         , 78 , 0  , 32 , 0  ), // #380
  INST(Lgs              , X86Rm              , O(000F00,B5,_,_,_,_,_,_  ), 0                         , 5  , 0  , 109, 0  ), // #381
  INST(Lidt             , X86M_Only          , O(000F00,01,3,_,_,_,_,_  ), 0                         , 80 , 0  , 32 , 0  ), // #382
  INST(Ljmp             , X86LcallLjmp       , O(000000,FF,5,_,_,_,_,_  ), O(000000,EA,_,_,_,_,_,_  ), 64 , 41 , 110, 0  ), // #383
  INST(Lldt             , X86M_NoSize        , O(000F00,00,2,_,_,_,_,_  ), 0                         , 78 , 0  , 111, 0  ), // #384
  INST(Llwpcb           , VexR_Wx            , V(XOP_M9,12,0,0,x,_,_,_  ), 0                         , 81 , 0  , 112, 87 ), // #385
  INST(Lmsw             , X86M_NoSize        , O(000F00,01,6,_,_,_,_,_  ), 0                         , 82 , 0  , 111, 0  ), // #386
  INST(Lods             , X86StrRm           , O(000000,AC,_,_,_,_,_,_  ), 0                         , 0  , 0  , 113, 88 ), // #387
  INST(Loop             , X86JecxzLoop       , 0                         , O(000000,E2,_,_,_,_,_,_  ), 0  , 42 , 114, 0  ), // #388
  INST(Loope            , X86JecxzLoop       , 0                         , O(000000,E1,_,_,_,_,_,_  ), 0  , 43 , 114, 75 ), // #389
  INST(Loopne           , X86JecxzLoop       , 0                         , O(000000,E0,_,_,_,_,_,_  ), 0  , 44 , 114, 75 ), // #390
  INST(Lsl              , X86Rm              , O(000F00,03,_,_,_,_,_,_  ), 0                         , 5  , 0  , 115, 11 ), // #391
  INST(Lss              , X86Rm              , O(000F00,B2,_,_,_,_,_,_  ), 0                         , 5  , 0  , 109, 0  ), // #392
  INST(Ltr              , X86M_NoSize        , O(000F00,00,3,_,_,_,_,_  ), 0                         , 80 , 0  , 111, 0  ), // #393
  INST(Lwpins           , VexVmi4_Wx         , V(XOP_MA,12,0,0,x,_,_,_  ), 0                         , 83 , 0  , 116, 87 ), // #394
  INST(Lwpval           , VexVmi4_Wx         , V(XOP_MA,12,1,0,x,_,_,_  ), 0                         , 84 , 0  , 116, 87 ), // #395
  INST(Lzcnt            , X86Rm_Raw66H       , O(F30F00,BD,_,_,x,_,_,_  ), 0                         , 7  , 0  , 23 , 89 ), // #396
  INST(Maskmovdqu       , ExtRm_ZDI          , O(660F00,F7,_,_,_,_,_,_  ), 0                         , 4  , 0  , 117, 5  ), // #397
  INST(Maskmovq         , ExtRm_ZDI          , O(000F00,F7,_,_,_,_,_,_  ), 0                         , 5  , 0  , 118, 90 ), // #398
  INST(Maxpd            , ExtRm              , O(660F00,5F,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6  , 5  ), // #399
  INST(Maxps            , ExtRm              , O(000F00,5F,_,_,_,_,_,_  ), 0                         , 5  , 0  , 6  , 6  ), // #400
  INST(Maxsd            , ExtRm              , O(F20F00,5F,_,_,_,_,_,_  ), 0                         , 6  , 0  , 7  , 5  ), // #401
  INST(Maxss            , ExtRm              , O(F30F00,5F,_,_,_,_,_,_  ), 0                         , 7  , 0  , 8  , 6  ), // #402
  INST(Mcommit          , X86Op              , O(F30F01,FA,_,_,_,_,_,_  ), 0                         , 27 , 0  , 31 , 91 ), // #403
  INST(Mfence           , X86Fence           , O(000F00,AE,6,_,_,_,_,_  ), 0                         , 82 , 0  , 31 , 5  ), // #404
  INST(Minpd            , ExtRm              , O(660F00,5D,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6  , 5  ), // #405
  INST(Minps            , ExtRm              , O(000F00,5D,_,_,_,_,_,_  ), 0                         , 5  , 0  , 6  , 6  ), // #406
  INST(Minsd            , ExtRm              , O(F20F00,5D,_,_,_,_,_,_  ), 0                         , 6  , 0  , 7  , 5  ), // #407
  INST(Minss            , ExtRm              , O(F30F00,5D,_,_,_,_,_,_  ), 0                         , 7  , 0  , 8  , 6  ), // #408
  INST(Monitor          , X86Op              , O(000F01,C8,_,_,_,_,_,_  ), 0                         , 23 , 0  , 119, 92 ), // #409
  INST(Monitorx         , X86Op              , O(000F01,FA,_,_,_,_,_,_  ), 0                         , 23 , 0  , 119, 93 ), // #410
  INST(Mov              , X86Mov             , 0                         , 0                         , 0  , 0  , 120, 94 ), // #411
  INST(Movabs           , X86Movabs          , 0                         , 0                         , 0  , 0  , 121, 0  ), // #412
  INST(Movapd           , ExtMov             , O(660F00,28,_,_,_,_,_,_  ), O(660F00,29,_,_,_,_,_,_  ), 4  , 45 , 122, 95 ), // #413
  INST(Movaps           , ExtMov             , O(000F00,28,_,_,_,_,_,_  ), O(000F00,29,_,_,_,_,_,_  ), 5  , 46 , 122, 96 ), // #414
  INST(Movbe            , ExtMovbe           , O(000F38,F0,_,_,x,_,_,_  ), O(000F38,F1,_,_,x,_,_,_  ), 1  , 47 , 123, 97 ), // #415
  INST(Movd             , ExtMovd            , O(000F00,6E,_,_,_,_,_,_  ), O(000F00,7E,_,_,_,_,_,_  ), 5  , 48 , 124, 98 ), // #416
  INST(Movddup          , ExtMov             , O(F20F00,12,_,_,_,_,_,_  ), 0                         , 6  , 0  , 7  , 7  ), // #417
  INST(Movdir64b        , X86EnqcmdMovdir64b , O(660F38,F8,_,_,_,_,_,_  ), 0                         , 2  , 0  , 125, 99 ), // #418
  INST(Movdiri          , X86MovntiMovdiri   , O(000F38,F9,_,_,_,_,_,_  ), 0                         , 1  , 0  , 3  , 100), // #419
  INST(Movdq2q          , ExtMov             , O(F20F00,D6,_,_,_,_,_,_  ), 0                         , 6  , 0  , 126, 5  ), // #420
  INST(Movdqa           , ExtMov             , O(660F00,6F,_,_,_,_,_,_  ), O(660F00,7F,_,_,_,_,_,_  ), 4  , 49 , 122, 95 ), // #421
  INST(Movdqu           , ExtMov             , O(F30F00,6F,_,_,_,_,_,_  ), O(F30F00,7F,_,_,_,_,_,_  ), 7  , 50 , 122, 95 ), // #422
  INST(Movhlps          , ExtMov             , O(000F00,12,_,_,_,_,_,_  ), 0                         , 5  , 0  , 127, 6  ), // #423
  INST(Movhpd           , ExtMov             , O(660F00,16,_,_,_,_,_,_  ), O(660F00,17,_,_,_,_,_,_  ), 4  , 51 , 128, 5  ), // #424
  INST(Movhps           , ExtMov             , O(000F00,16,_,_,_,_,_,_  ), O(000F00,17,_,_,_,_,_,_  ), 5  , 52 , 128, 6  ), // #425
  INST(Movlhps          , ExtMov             , O(000F00,16,_,_,_,_,_,_  ), 0                         , 5  , 0  , 127, 6  ), // #426
  INST(Movlpd           , ExtMov             , O(660F00,12,_,_,_,_,_,_  ), O(660F00,13,_,_,_,_,_,_  ), 4  , 53 , 128, 5  ), // #427
  INST(Movlps           , ExtMov             , O(000F00,12,_,_,_,_,_,_  ), O(000F00,13,_,_,_,_,_,_  ), 5  , 54 , 128, 6  ), // #428
  INST(Movmskpd         , ExtMov             , O(660F00,50,_,_,_,_,_,_  ), 0                         , 4  , 0  , 129, 5  ), // #429
  INST(Movmskps         , ExtMov             , O(000F00,50,_,_,_,_,_,_  ), 0                         , 5  , 0  , 129, 6  ), // #430
  INST(Movntdq          , ExtMov             , 0                         , O(660F00,E7,_,_,_,_,_,_  ), 0  , 55 , 130, 5  ), // #431
  INST(Movntdqa         , ExtMov             , O(660F38,2A,_,_,_,_,_,_  ), 0                         , 2  , 0  , 104, 13 ), // #432
  INST(Movnti           , X86MovntiMovdiri   , O(000F00,C3,_,_,x,_,_,_  ), 0                         , 5  , 0  , 3  , 5  ), // #433
  INST(Movntpd          , ExtMov             , 0                         , O(660F00,2B,_,_,_,_,_,_  ), 0  , 56 , 130, 5  ), // #434
  INST(Movntps          , ExtMov             , 0                         , O(000F00,2B,_,_,_,_,_,_  ), 0  , 57 , 130, 6  ), // #435
  INST(Movntq           , ExtMov             , 0                         , O(000F00,E7,_,_,_,_,_,_  ), 0  , 58 , 131, 90 ), // #436
  INST(Movntsd          , ExtMov             , 0                         , O(F20F00,2B,_,_,_,_,_,_  ), 0  , 59 , 132, 51 ), // #437
  INST(Movntss          , ExtMov             , 0                         , O(F30F00,2B,_,_,_,_,_,_  ), 0  , 60 , 133, 51 ), // #438
  INST(Movq             , ExtMovq            , O(000F00,6E,_,_,x,_,_,_  ), O(000F00,7E,_,_,x,_,_,_  ), 5  , 48 , 134, 101), // #439
  INST(Movq2dq          , ExtRm              , O(F30F00,D6,_,_,_,_,_,_  ), 0                         , 7  , 0  , 135, 5  ), // #440
  INST(Movs             , X86StrMm           , O(000000,A4,_,_,_,_,_,_  ), 0                         , 0  , 0  , 136, 88 ), // #441
  INST(Movsd            , ExtMov             , O(F20F00,10,_,_,_,_,_,_  ), O(F20F00,11,_,_,_,_,_,_  ), 6  , 61 , 137, 95 ), // #442
  INST(Movshdup         , ExtRm              , O(F30F00,16,_,_,_,_,_,_  ), 0                         , 7  , 0  , 6  , 7  ), // #443
  INST(Movsldup         , ExtRm              , O(F30F00,12,_,_,_,_,_,_  ), 0                         , 7  , 0  , 6  , 7  ), // #444
  INST(Movss            , ExtMov             , O(F30F00,10,_,_,_,_,_,_  ), O(F30F00,11,_,_,_,_,_,_  ), 7  , 62 , 138, 96 ), // #445
  INST(Movsx            , X86MovsxMovzx      , O(000F00,BE,_,_,x,_,_,_  ), 0                         , 5  , 0  , 139, 0  ), // #446
  INST(Movsxd           , X86Rm              , O(000000,63,_,_,x,_,_,_  ), 0                         , 0  , 0  , 140, 0  ), // #447
  INST(Movupd           , ExtMov             , O(660F00,10,_,_,_,_,_,_  ), O(660F00,11,_,_,_,_,_,_  ), 4  , 63 , 122, 95 ), // #448
  INST(Movups           , ExtMov             , O(000F00,10,_,_,_,_,_,_  ), O(000F00,11,_,_,_,_,_,_  ), 5  , 64 , 122, 96 ), // #449
  INST(Movzx            , X86MovsxMovzx      , O(000F00,B6,_,_,x,_,_,_  ), 0                         , 5  , 0  , 139, 0  ), // #450
  INST(Mpsadbw          , ExtRmi             , O(660F3A,42,_,_,_,_,_,_  ), 0                         , 9  , 0  , 9  , 13 ), // #451
  INST(Mul              , X86M_GPB_MulDiv    , O(000000,F6,4,_,x,_,_,_  ), 0                         , 10 , 0  , 56 , 1  ), // #452
  INST(Mulpd            , ExtRm              , O(660F00,59,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6  , 5  ), // #453
  INST(Mulps            , ExtRm              , O(000F00,59,_,_,_,_,_,_  ), 0                         , 5  , 0  , 6  , 6  ), // #454
  INST(Mulsd            , ExtRm              , O(F20F00,59,_,_,_,_,_,_  ), 0                         , 6  , 0  , 7  , 5  ), // #455
  INST(Mulss            , ExtRm              , O(F30F00,59,_,_,_,_,_,_  ), 0                         , 7  , 0  , 8  , 6  ), // #456
  INST(Mulx             , VexRvm_ZDX_Wx      , V(F20F38,F6,_,0,x,_,_,_  ), 0                         , 85 , 0  , 141, 102), // #457
  INST(Mwait            , X86Op              , O(000F01,C9,_,_,_,_,_,_  ), 0                         , 23 , 0  , 142, 92 ), // #458
  INST(Mwaitx           , X86Op              , O(000F01,FB,_,_,_,_,_,_  ), 0                         , 23 , 0  , 143, 93 ), // #459
  INST(Neg              , X86M_GPB           , O(000000,F6,3,_,x,_,_,_  ), 0                         , 77 , 0  , 144, 1  ), // #460
  INST(Nop              , X86M_Nop           , O(000000,90,_,_,_,_,_,_  ), 0                         , 0  , 0  , 145, 0  ), // #461
  INST(Not              , X86M_GPB           , O(000000,F6,2,_,x,_,_,_  ), 0                         , 3  , 0  , 144, 0  ), // #462
  INST(Or               , X86Arith           , O(000000,08,1,_,x,_,_,_  ), 0                         , 33 , 0  , 146, 1  ), // #463
  INST(Orpd             , ExtRm              , O(660F00,56,_,_,_,_,_,_  ), 0                         , 4  , 0  , 12 , 5  ), // #464
  INST(Orps             , ExtRm              , O(000F00,56,_,_,_,_,_,_  ), 0                         , 5  , 0  , 12 , 6  ), // #465
  INST(Out              , X86Out             , O(000000,EE,_,_,_,_,_,_  ), O(000000,E6,_,_,_,_,_,_  ), 0  , 65 , 147, 0  ), // #466
  INST(Outs             , X86Outs            , O(000000,6E,_,_,_,_,_,_  ), 0                         , 0  , 0  , 148, 0  ), // #467
  INST(Pabsb            , ExtRm_P            , O(000F38,1C,_,_,_,_,_,_  ), 0                         , 1  , 0  , 149, 103), // #468
  INST(Pabsd            , ExtRm_P            , O(000F38,1E,_,_,_,_,_,_  ), 0                         , 1  , 0  , 149, 103), // #469
  INST(Pabsw            , ExtRm_P            , O(000F38,1D,_,_,_,_,_,_  ), 0                         , 1  , 0  , 149, 103), // #470
  INST(Packssdw         , ExtRm_P            , O(000F00,6B,_,_,_,_,_,_  ), 0                         , 5  , 0  , 149, 98 ), // #471
  INST(Packsswb         , ExtRm_P            , O(000F00,63,_,_,_,_,_,_  ), 0                         , 5  , 0  , 149, 98 ), // #472
  INST(Packusdw         , ExtRm              , O(660F38,2B,_,_,_,_,_,_  ), 0                         , 2  , 0  , 6  , 13 ), // #473
  INST(Packuswb         , ExtRm_P            , O(000F00,67,_,_,_,_,_,_  ), 0                         , 5  , 0  , 149, 98 ), // #474
  INST(Paddb            , ExtRm_P            , O(000F00,FC,_,_,_,_,_,_  ), 0                         , 5  , 0  , 149, 98 ), // #475
  INST(Paddd            , ExtRm_P            , O(000F00,FE,_,_,_,_,_,_  ), 0                         , 5  , 0  , 149, 98 ), // #476
  INST(Paddq            , ExtRm_P            , O(000F00,D4,_,_,_,_,_,_  ), 0                         , 5  , 0  , 149, 5  ), // #477
  INST(Paddsb           , ExtRm_P            , O(000F00,EC,_,_,_,_,_,_  ), 0                         , 5  , 0  , 149, 98 ), // #478
  INST(Paddsw           , ExtRm_P            , O(000F00,ED,_,_,_,_,_,_  ), 0                         , 5  , 0  , 149, 98 ), // #479
  INST(Paddusb          , ExtRm_P            , O(000F00,DC,_,_,_,_,_,_  ), 0                         , 5  , 0  , 149, 98 ), // #480
  INST(Paddusw          , ExtRm_P            , O(000F00,DD,_,_,_,_,_,_  ), 0                         , 5  , 0  , 149, 98 ), // #481
  INST(Paddw            , ExtRm_P            , O(000F00,FD,_,_,_,_,_,_  ), 0                         , 5  , 0  , 149, 98 ), // #482
  INST(Palignr          , ExtRmi_P           , O(000F3A,0F,_,_,_,_,_,_  ), 0                         , 86 , 0  , 150, 103), // #483
  INST(Pand             , ExtRm_P            , O(000F00,DB,_,_,_,_,_,_  ), 0                         , 5  , 0  , 151, 98 ), // #484
  INST(Pandn            , ExtRm_P            , O(000F00,DF,_,_,_,_,_,_  ), 0                         , 5  , 0  , 152, 98 ), // #485
  INST(Pause            , X86Op              , O(F30000,90,_,_,_,_,_,_  ), 0                         , 87 , 0  , 31 , 0  ), // #486
  INST(Pavgb            , ExtRm_P            , O(000F00,E0,_,_,_,_,_,_  ), 0                         , 5  , 0  , 149, 104), // #487
  INST(Pavgusb          , Ext3dNow           , O(000F0F,BF,_,_,_,_,_,_  ), 0                         , 88 , 0  , 153, 58 ), // #488
  INST(Pavgw            , ExtRm_P            , O(000F00,E3,_,_,_,_,_,_  ), 0                         , 5  , 0  , 149, 104), // #489
  INST(Pblendvb         , ExtRm_XMM0         , O(660F38,10,_,_,_,_,_,_  ), 0                         , 2  , 0  , 16 , 13 ), // #490
  INST(Pblendw          , ExtRmi             , O(660F3A,0E,_,_,_,_,_,_  ), 0                         , 9  , 0  , 9  , 13 ), // #491
  INST(Pclmulqdq        , ExtRmi             , O(660F3A,44,_,_,_,_,_,_  ), 0                         , 9  , 0  , 9  , 105), // #492
  INST(Pcmpeqb          , ExtRm_P            , O(000F00,74,_,_,_,_,_,_  ), 0                         , 5  , 0  , 152, 98 ), // #493
  INST(Pcmpeqd          , ExtRm_P            , O(000F00,76,_,_,_,_,_,_  ), 0                         , 5  , 0  , 152, 98 ), // #494
  INST(Pcmpeqq          , ExtRm              , O(660F38,29,_,_,_,_,_,_  ), 0                         , 2  , 0  , 154, 13 ), // #495
  INST(Pcmpeqw          , ExtRm_P            , O(000F00,75,_,_,_,_,_,_  ), 0                         , 5  , 0  , 152, 98 ), // #496
  INST(Pcmpestri        , ExtRmi             , O(660F3A,61,_,_,_,_,_,_  ), 0                         , 9  , 0  , 155, 106), // #497
  INST(Pcmpestrm        , ExtRmi             , O(660F3A,60,_,_,_,_,_,_  ), 0                         , 9  , 0  , 156, 106), // #498
  INST(Pcmpgtb          , ExtRm_P            , O(000F00,64,_,_,_,_,_,_  ), 0                         , 5  , 0  , 152, 98 ), // #499
  INST(Pcmpgtd          , ExtRm_P            , O(000F00,66,_,_,_,_,_,_  ), 0                         , 5  , 0  , 152, 98 ), // #500
  INST(Pcmpgtq          , ExtRm              , O(660F38,37,_,_,_,_,_,_  ), 0                         , 2  , 0  , 154, 46 ), // #501
  INST(Pcmpgtw          , ExtRm_P            , O(000F00,65,_,_,_,_,_,_  ), 0                         , 5  , 0  , 152, 98 ), // #502
  INST(Pcmpistri        , ExtRmi             , O(660F3A,63,_,_,_,_,_,_  ), 0                         , 9  , 0  , 157, 106), // #503
  INST(Pcmpistrm        , ExtRmi             , O(660F3A,62,_,_,_,_,_,_  ), 0                         , 9  , 0  , 158, 106), // #504
  INST(Pconfig          , X86Op              , O(000F01,C5,_,_,_,_,_,_  ), 0                         , 23 , 0  , 31 , 107), // #505
  INST(Pdep             , VexRvm_Wx          , V(F20F38,F5,_,0,x,_,_,_  ), 0                         , 85 , 0  , 11 , 102), // #506
  INST(Pext             , VexRvm_Wx          , V(F30F38,F5,_,0,x,_,_,_  ), 0                         , 89 , 0  , 11 , 102), // #507
  INST(Pextrb           , ExtExtract         , O(000F3A,14,_,_,_,_,_,_  ), 0                         , 86 , 0  , 159, 13 ), // #508
  INST(Pextrd           , ExtExtract         , O(000F3A,16,_,_,_,_,_,_  ), 0                         , 86 , 0  , 60 , 13 ), // #509
  INST(Pextrq           , ExtExtract         , O(000F3A,16,_,_,1,_,_,_  ), 0                         , 90 , 0  , 160, 13 ), // #510
  INST(Pextrw           , ExtPextrw          , O(000F00,C5,_,_,_,_,_,_  ), O(000F3A,15,_,_,_,_,_,_  ), 5  , 66 , 161, 108), // #511
  INST(Pf2id            , Ext3dNow           , O(000F0F,1D,_,_,_,_,_,_  ), 0                         , 88 , 0  , 153, 58 ), // #512
  INST(Pf2iw            , Ext3dNow           , O(000F0F,1C,_,_,_,_,_,_  ), 0                         , 88 , 0  , 153, 109), // #513
  INST(Pfacc            , Ext3dNow           , O(000F0F,AE,_,_,_,_,_,_  ), 0                         , 88 , 0  , 153, 58 ), // #514
  INST(Pfadd            , Ext3dNow           , O(000F0F,9E,_,_,_,_,_,_  ), 0                         , 88 , 0  , 153, 58 ), // #515
  INST(Pfcmpeq          , Ext3dNow           , O(000F0F,B0,_,_,_,_,_,_  ), 0                         , 88 , 0  , 153, 58 ), // #516
  INST(Pfcmpge          , Ext3dNow           , O(000F0F,90,_,_,_,_,_,_  ), 0                         , 88 , 0  , 153, 58 ), // #517
  INST(Pfcmpgt          , Ext3dNow           , O(000F0F,A0,_,_,_,_,_,_  ), 0                         , 88 , 0  , 153, 58 ), // #518
  INST(Pfmax            , Ext3dNow           , O(000F0F,A4,_,_,_,_,_,_  ), 0                         , 88 , 0  , 153, 58 ), // #519
  INST(Pfmin            , Ext3dNow           , O(000F0F,94,_,_,_,_,_,_  ), 0                         , 88 , 0  , 153, 58 ), // #520
  INST(Pfmul            , Ext3dNow           , O(000F0F,B4,_,_,_,_,_,_  ), 0                         , 88 , 0  , 153, 58 ), // #521
  INST(Pfnacc           , Ext3dNow           , O(000F0F,8A,_,_,_,_,_,_  ), 0                         , 88 , 0  , 153, 109), // #522
  INST(Pfpnacc          , Ext3dNow           , O(000F0F,8E,_,_,_,_,_,_  ), 0                         , 88 , 0  , 153, 109), // #523
  INST(Pfrcp            , Ext3dNow           , O(000F0F,96,_,_,_,_,_,_  ), 0                         , 88 , 0  , 153, 58 ), // #524
  INST(Pfrcpit1         , Ext3dNow           , O(000F0F,A6,_,_,_,_,_,_  ), 0                         , 88 , 0  , 153, 58 ), // #525
  INST(Pfrcpit2         , Ext3dNow           , O(000F0F,B6,_,_,_,_,_,_  ), 0                         , 88 , 0  , 153, 58 ), // #526
  INST(Pfrcpv           , Ext3dNow           , O(000F0F,86,_,_,_,_,_,_  ), 0                         , 88 , 0  , 153, 110), // #527
  INST(Pfrsqit1         , Ext3dNow           , O(000F0F,A7,_,_,_,_,_,_  ), 0                         , 88 , 0  , 153, 58 ), // #528
  INST(Pfrsqrt          , Ext3dNow           , O(000F0F,97,_,_,_,_,_,_  ), 0                         , 88 , 0  , 153, 58 ), // #529
  INST(Pfrsqrtv         , Ext3dNow           , O(000F0F,87,_,_,_,_,_,_  ), 0                         , 88 , 0  , 153, 110), // #530
  INST(Pfsub            , Ext3dNow           , O(000F0F,9A,_,_,_,_,_,_  ), 0                         , 88 , 0  , 153, 58 ), // #531
  INST(Pfsubr           , Ext3dNow           , O(000F0F,AA,_,_,_,_,_,_  ), 0                         , 88 , 0  , 153, 58 ), // #532
  INST(Phaddd           , ExtRm_P            , O(000F38,02,_,_,_,_,_,_  ), 0                         , 1  , 0  , 149, 103), // #533
  INST(Phaddsw          , ExtRm_P            , O(000F38,03,_,_,_,_,_,_  ), 0                         , 1  , 0  , 149, 103), // #534
  INST(Phaddw           , ExtRm_P            , O(000F38,01,_,_,_,_,_,_  ), 0                         , 1  , 0  , 149, 103), // #535
  INST(Phminposuw       , ExtRm              , O(660F38,41,_,_,_,_,_,_  ), 0                         , 2  , 0  , 6  , 13 ), // #536
  INST(Phsubd           , ExtRm_P            , O(000F38,06,_,_,_,_,_,_  ), 0                         , 1  , 0  , 149, 103), // #537
  INST(Phsubsw          , ExtRm_P            , O(000F38,07,_,_,_,_,_,_  ), 0                         , 1  , 0  , 149, 103), // #538
  INST(Phsubw           , ExtRm_P            , O(000F38,05,_,_,_,_,_,_  ), 0                         , 1  , 0  , 149, 103), // #539
  INST(Pi2fd            , Ext3dNow           , O(000F0F,0D,_,_,_,_,_,_  ), 0                         , 88 , 0  , 153, 58 ), // #540
  INST(Pi2fw            , Ext3dNow           , O(000F0F,0C,_,_,_,_,_,_  ), 0                         , 88 , 0  , 153, 109), // #541
  INST(Pinsrb           , ExtRmi             , O(660F3A,20,_,_,_,_,_,_  ), 0                         , 9  , 0  , 162, 13 ), // #542
  INST(Pinsrd           , ExtRmi             , O(660F3A,22,_,_,_,_,_,_  ), 0                         , 9  , 0  , 163, 13 ), // #543
  INST(Pinsrq           , ExtRmi             , O(660F3A,22,_,_,1,_,_,_  ), 0                         , 91 , 0  , 164, 13 ), // #544
  INST(Pinsrw           , ExtRmi_P           , O(000F00,C4,_,_,_,_,_,_  ), 0                         , 5  , 0  , 165, 104), // #545
  INST(Pmaddubsw        , ExtRm_P            , O(000F38,04,_,_,_,_,_,_  ), 0                         , 1  , 0  , 149, 103), // #546
  INST(Pmaddwd          , ExtRm_P            , O(000F00,F5,_,_,_,_,_,_  ), 0                         , 5  , 0  , 149, 98 ), // #547
  INST(Pmaxsb           , ExtRm              , O(660F38,3C,_,_,_,_,_,_  ), 0                         , 2  , 0  , 12 , 13 ), // #548
  INST(Pmaxsd           , ExtRm              , O(660F38,3D,_,_,_,_,_,_  ), 0                         , 2  , 0  , 12 , 13 ), // #549
  INST(Pmaxsw           , ExtRm_P            , O(000F00,EE,_,_,_,_,_,_  ), 0                         , 5  , 0  , 151, 104), // #550
  INST(Pmaxub           , ExtRm_P            , O(000F00,DE,_,_,_,_,_,_  ), 0                         , 5  , 0  , 151, 104), // #551
  INST(Pmaxud           , ExtRm              , O(660F38,3F,_,_,_,_,_,_  ), 0                         , 2  , 0  , 12 , 13 ), // #552
  INST(Pmaxuw           , ExtRm              , O(660F38,3E,_,_,_,_,_,_  ), 0                         , 2  , 0  , 12 , 13 ), // #553
  INST(Pminsb           , ExtRm              , O(660F38,38,_,_,_,_,_,_  ), 0                         , 2  , 0  , 12 , 13 ), // #554
  INST(Pminsd           , ExtRm              , O(660F38,39,_,_,_,_,_,_  ), 0                         , 2  , 0  , 12 , 13 ), // #555
  INST(Pminsw           , ExtRm_P            , O(000F00,EA,_,_,_,_,_,_  ), 0                         , 5  , 0  , 151, 104), // #556
  INST(Pminub           , ExtRm_P            , O(000F00,DA,_,_,_,_,_,_  ), 0                         , 5  , 0  , 151, 104), // #557
  INST(Pminud           , ExtRm              , O(660F38,3B,_,_,_,_,_,_  ), 0                         , 2  , 0  , 12 , 13 ), // #558
  INST(Pminuw           , ExtRm              , O(660F38,3A,_,_,_,_,_,_  ), 0                         , 2  , 0  , 12 , 13 ), // #559
  INST(Pmovmskb         , ExtRm_P            , O(000F00,D7,_,_,_,_,_,_  ), 0                         , 5  , 0  , 166, 104), // #560
  INST(Pmovsxbd         , ExtRm              , O(660F38,21,_,_,_,_,_,_  ), 0                         , 2  , 0  , 8  , 13 ), // #561
  INST(Pmovsxbq         , ExtRm              , O(660F38,22,_,_,_,_,_,_  ), 0                         , 2  , 0  , 167, 13 ), // #562
  INST(Pmovsxbw         , ExtRm              , O(660F38,20,_,_,_,_,_,_  ), 0                         , 2  , 0  , 7  , 13 ), // #563
  INST(Pmovsxdq         , ExtRm              , O(660F38,25,_,_,_,_,_,_  ), 0                         , 2  , 0  , 7  , 13 ), // #564
  INST(Pmovsxwd         , ExtRm              , O(660F38,23,_,_,_,_,_,_  ), 0                         , 2  , 0  , 7  , 13 ), // #565
  INST(Pmovsxwq         , ExtRm              , O(660F38,24,_,_,_,_,_,_  ), 0                         , 2  , 0  , 8  , 13 ), // #566
  INST(Pmovzxbd         , ExtRm              , O(660F38,31,_,_,_,_,_,_  ), 0                         , 2  , 0  , 8  , 13 ), // #567
  INST(Pmovzxbq         , ExtRm              , O(660F38,32,_,_,_,_,_,_  ), 0                         , 2  , 0  , 167, 13 ), // #568
  INST(Pmovzxbw         , ExtRm              , O(660F38,30,_,_,_,_,_,_  ), 0                         , 2  , 0  , 7  , 13 ), // #569
  INST(Pmovzxdq         , ExtRm              , O(660F38,35,_,_,_,_,_,_  ), 0                         , 2  , 0  , 7  , 13 ), // #570
  INST(Pmovzxwd         , ExtRm              , O(660F38,33,_,_,_,_,_,_  ), 0                         , 2  , 0  , 7  , 13 ), // #571
  INST(Pmovzxwq         , ExtRm              , O(660F38,34,_,_,_,_,_,_  ), 0                         , 2  , 0  , 8  , 13 ), // #572
  INST(Pmuldq           , ExtRm              , O(660F38,28,_,_,_,_,_,_  ), 0                         , 2  , 0  , 6  , 13 ), // #573
  INST(Pmulhrsw         , ExtRm_P            , O(000F38,0B,_,_,_,_,_,_  ), 0                         , 1  , 0  , 149, 103), // #574
  INST(Pmulhrw          , Ext3dNow           , O(000F0F,B7,_,_,_,_,_,_  ), 0                         , 88 , 0  , 153, 58 ), // #575
  INST(Pmulhuw          , ExtRm_P            , O(000F00,E4,_,_,_,_,_,_  ), 0                         , 5  , 0  , 149, 104), // #576
  INST(Pmulhw           , ExtRm_P            , O(000F00,E5,_,_,_,_,_,_  ), 0                         , 5  , 0  , 149, 98 ), // #577
  INST(Pmulld           , ExtRm              , O(660F38,40,_,_,_,_,_,_  ), 0                         , 2  , 0  , 6  , 13 ), // #578
  INST(Pmullw           , ExtRm_P            , O(000F00,D5,_,_,_,_,_,_  ), 0                         , 5  , 0  , 149, 98 ), // #579
  INST(Pmuludq          , ExtRm_P            , O(000F00,F4,_,_,_,_,_,_  ), 0                         , 5  , 0  , 149, 5  ), // #580
  INST(Pop              , X86Pop             , O(000000,8F,0,_,_,_,_,_  ), O(000000,58,_,_,_,_,_,_  ), 0  , 67 , 168, 0  ), // #581
  INST(Popa             , X86Op              , O(660000,61,_,_,_,_,_,_  ), 0                         , 21 , 0  , 84 , 0  ), // #582
  INST(Popad            , X86Op              , O(000000,61,_,_,_,_,_,_  ), 0                         , 0  , 0  , 84 , 0  ), // #583
  INST(Popcnt           , X86Rm_Raw66H       , O(F30F00,B8,_,_,x,_,_,_  ), 0                         , 7  , 0  , 23 , 111), // #584
  INST(Popf             , X86Op              , O(660000,9D,_,_,_,_,_,_  ), 0                         , 21 , 0  , 31 , 112), // #585
  INST(Popfd            , X86Op              , O(000000,9D,_,_,_,_,_,_  ), 0                         , 0  , 0  , 84 , 112), // #586
  INST(Popfq            , X86Op              , O(000000,9D,_,_,_,_,_,_  ), 0                         , 0  , 0  , 34 , 112), // #587
  INST(Por              , ExtRm_P            , O(000F00,EB,_,_,_,_,_,_  ), 0                         , 5  , 0  , 151, 98 ), // #588
  INST(Prefetch         , X86M_Only          , O(000F00,0D,0,_,_,_,_,_  ), 0                         , 5  , 0  , 32 , 58 ), // #589
  INST(Prefetchit0      , X86M_Only          , O(000F00,18,7,_,_,_,_,_  ), 0                         , 24 , 0  , 73 , 113), // #590
  INST(Prefetchit1      , X86M_Only          , O(000F00,18,6,_,_,_,_,_  ), 0                         , 82 , 0  , 73 , 113), // #591
  INST(Prefetchnta      , X86M_Only          , O(000F00,18,0,_,_,_,_,_  ), 0                         , 5  , 0  , 32 , 6  ), // #592
  INST(Prefetcht0       , X86M_Only          , O(000F00,18,1,_,_,_,_,_  ), 0                         , 32 , 0  , 32 , 6  ), // #593
  INST(Prefetcht1       , X86M_Only          , O(000F00,18,2,_,_,_,_,_  ), 0                         , 78 , 0  , 32 , 6  ), // #594
  INST(Prefetcht2       , X86M_Only          , O(000F00,18,3,_,_,_,_,_  ), 0                         , 80 , 0  , 32 , 6  ), // #595
  INST(Prefetchw        , X86M_Only          , O(000F00,0D,1,_,_,_,_,_  ), 0                         , 32 , 0  , 32 , 114), // #596
  INST(Prefetchwt1      , X86M_Only          , O(000F00,0D,2,_,_,_,_,_  ), 0                         , 78 , 0  , 32 , 115), // #597
  INST(Psadbw           , ExtRm_P            , O(000F00,F6,_,_,_,_,_,_  ), 0                         , 5  , 0  , 149, 104), // #598
  INST(Pshufb           , ExtRm_P            , O(000F38,00,_,_,_,_,_,_  ), 0                         , 1  , 0  , 149, 103), // #599
  INST(Pshufd           , ExtRmi             , O(660F00,70,_,_,_,_,_,_  ), 0                         , 4  , 0  , 9  , 5  ), // #600
  INST(Pshufhw          , ExtRmi             , O(F30F00,70,_,_,_,_,_,_  ), 0                         , 7  , 0  , 9  , 5  ), // #601
  INST(Pshuflw          , ExtRmi             , O(F20F00,70,_,_,_,_,_,_  ), 0                         , 6  , 0  , 9  , 5  ), // #602
  INST(Pshufw           , ExtRmi_P           , O(000F00,70,_,_,_,_,_,_  ), 0                         , 5  , 0  , 169, 90 ), // #603
  INST(Psignb           , ExtRm_P            , O(000F38,08,_,_,_,_,_,_  ), 0                         , 1  , 0  , 149, 103), // #604
  INST(Psignd           , ExtRm_P            , O(000F38,0A,_,_,_,_,_,_  ), 0                         , 1  , 0  , 149, 103), // #605
  INST(Psignw           , ExtRm_P            , O(000F38,09,_,_,_,_,_,_  ), 0                         , 1  , 0  , 149, 103), // #606
  INST(Pslld            , ExtRmRi_P          , O(000F00,F2,_,_,_,_,_,_  ), O(000F00,72,6,_,_,_,_,_  ), 5  , 68 , 170, 98 ), // #607
  INST(Pslldq           , ExtRmRi            , 0                         , O(660F00,73,7,_,_,_,_,_  ), 0  , 69 , 171, 5  ), // #608
  INST(Psllq            , ExtRmRi_P          , O(000F00,F3,_,_,_,_,_,_  ), O(000F00,73,6,_,_,_,_,_  ), 5  , 70 , 170, 98 ), // #609
  INST(Psllw            , ExtRmRi_P          , O(000F00,F1,_,_,_,_,_,_  ), O(000F00,71,6,_,_,_,_,_  ), 5  , 71 , 170, 98 ), // #610
  INST(Psmash           , X86Op              , O(F30F01,FF,_,_,_,_,_,_  ), 0                         , 27 , 0  , 34 , 116), // #611
  INST(Psrad            , ExtRmRi_P          , O(000F00,E2,_,_,_,_,_,_  ), O(000F00,72,4,_,_,_,_,_  ), 5  , 72 , 170, 98 ), // #612
  INST(Psraw            , ExtRmRi_P          , O(000F00,E1,_,_,_,_,_,_  ), O(000F00,71,4,_,_,_,_,_  ), 5  , 73 , 170, 98 ), // #613
  INST(Psrld            , ExtRmRi_P          , O(000F00,D2,_,_,_,_,_,_  ), O(000F00,72,2,_,_,_,_,_  ), 5  , 74 , 170, 98 ), // #614
  INST(Psrldq           , ExtRmRi            , 0                         , O(660F00,73,3,_,_,_,_,_  ), 0  , 75 , 171, 5  ), // #615
  INST(Psrlq            , ExtRmRi_P          , O(000F00,D3,_,_,_,_,_,_  ), O(000F00,73,2,_,_,_,_,_  ), 5  , 76 , 170, 98 ), // #616
  INST(Psrlw            , ExtRmRi_P          , O(000F00,D1,_,_,_,_,_,_  ), O(000F00,71,2,_,_,_,_,_  ), 5  , 77 , 170, 98 ), // #617
  INST(Psubb            , ExtRm_P            , O(000F00,F8,_,_,_,_,_,_  ), 0                         , 5  , 0  , 152, 98 ), // #618
  INST(Psubd            , ExtRm_P            , O(000F00,FA,_,_,_,_,_,_  ), 0                         , 5  , 0  , 152, 98 ), // #619
  INST(Psubq            , ExtRm_P            , O(000F00,FB,_,_,_,_,_,_  ), 0                         , 5  , 0  , 152, 5  ), // #620
  INST(Psubsb           , ExtRm_P            , O(000F00,E8,_,_,_,_,_,_  ), 0                         , 5  , 0  , 152, 98 ), // #621
  INST(Psubsw           , ExtRm_P            , O(000F00,E9,_,_,_,_,_,_  ), 0                         , 5  , 0  , 152, 98 ), // #622
  INST(Psubusb          , ExtRm_P            , O(000F00,D8,_,_,_,_,_,_  ), 0                         , 5  , 0  , 152, 98 ), // #623
  INST(Psubusw          , ExtRm_P            , O(000F00,D9,_,_,_,_,_,_  ), 0                         , 5  , 0  , 152, 98 ), // #624
  INST(Psubw            , ExtRm_P            , O(000F00,F9,_,_,_,_,_,_  ), 0                         , 5  , 0  , 152, 98 ), // #625
  INST(Pswapd           , Ext3dNow           , O(000F0F,BB,_,_,_,_,_,_  ), 0                         , 88 , 0  , 153, 109), // #626
  INST(Ptest            , ExtRm              , O(660F38,17,_,_,_,_,_,_  ), 0                         , 2  , 0  , 6  , 117), // #627
  INST(Ptwrite          , X86M               , O(F30F00,AE,4,_,_,_,_,_  ), 0                         , 92 , 0  , 172, 118), // #628
  INST(Punpckhbw        , ExtRm_P            , O(000F00,68,_,_,_,_,_,_  ), 0                         , 5  , 0  , 149, 98 ), // #629
  INST(Punpckhdq        , ExtRm_P            , O(000F00,6A,_,_,_,_,_,_  ), 0                         , 5  , 0  , 149, 98 ), // #630
  INST(Punpckhqdq       , ExtRm              , O(660F00,6D,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6  , 5  ), // #631
  INST(Punpckhwd        , ExtRm_P            , O(000F00,69,_,_,_,_,_,_  ), 0                         , 5  , 0  , 149, 98 ), // #632
  INST(Punpcklbw        , ExtRm_P            , O(000F00,60,_,_,_,_,_,_  ), 0                         , 5  , 0  , 173, 98 ), // #633
  INST(Punpckldq        , ExtRm_P            , O(000F00,62,_,_,_,_,_,_  ), 0                         , 5  , 0  , 173, 98 ), // #634
  INST(Punpcklqdq       , ExtRm              , O(660F00,6C,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6  , 5  ), // #635
  INST(Punpcklwd        , ExtRm_P            , O(000F00,61,_,_,_,_,_,_  ), 0                         , 5  , 0  , 173, 98 ), // #636
  INST(Push             , X86Push            , O(000000,FF,6,_,_,_,_,_  ), O(000000,50,_,_,_,_,_,_  ), 34 , 78 , 174, 0  ), // #637
  INST(Pusha            , X86Op              , O(660000,60,_,_,_,_,_,_  ), 0                         , 21 , 0  , 84 , 0  ), // #638
  INST(Pushad           , X86Op              , O(000000,60,_,_,_,_,_,_  ), 0                         , 0  , 0  , 84 , 0  ), // #639
  INST(Pushf            , X86Op              , O(660000,9C,_,_,_,_,_,_  ), 0                         , 21 , 0  , 31 , 119), // #640
  INST(Pushfd           , X86Op              , O(000000,9C,_,_,_,_,_,_  ), 0                         , 0  , 0  , 84 , 119), // #641
  INST(Pushfq           , X86Op              , O(000000,9C,_,_,_,_,_,_  ), 0                         , 0  , 0  , 34 , 119), // #642
  INST(Pushw            , X86Pushw           , O(000000,FF,6,_,_,_,_,_  ), O(000000,50,_,_,_,_,_,_  ), 34 , 78 , 175, 0  ), // #643
  INST(Pvalidate        , X86Op              , O(F20F01,FF,_,_,_,_,_,_  ), 0                         , 93 , 0  , 31 , 120), // #644
  INST(Pxor             , ExtRm_P            , O(000F00,EF,_,_,_,_,_,_  ), 0                         , 5  , 0  , 152, 98 ), // #645
  INST(Rcl              , X86Rot             , O(000000,D0,2,_,x,_,_,_  ), 0                         , 3  , 0  , 176, 121), // #646
  INST(Rcpps            , ExtRm              , O(000F00,53,_,_,_,_,_,_  ), 0                         , 5  , 0  , 6  , 6  ), // #647
  INST(Rcpss            , ExtRm              , O(F30F00,53,_,_,_,_,_,_  ), 0                         , 7  , 0  , 8  , 6  ), // #648
  INST(Rcr              , X86Rot             , O(000000,D0,3,_,x,_,_,_  ), 0                         , 77 , 0  , 176, 121), // #649
  INST(Rdfsbase         , X86M               , O(F30F00,AE,0,_,x,_,_,_  ), 0                         , 7  , 0  , 177, 122), // #650
  INST(Rdgsbase         , X86M               , O(F30F00,AE,1,_,x,_,_,_  ), 0                         , 94 , 0  , 177, 122), // #651
  INST(Rdmsr            , X86Op              , O(000F00,32,_,_,_,_,_,_  ), 0                         , 5  , 0  , 178, 123), // #652
  INST(Rdpid            , X86R_Native        , O(F30F00,C7,7,_,_,_,_,_  ), 0                         , 95 , 0  , 179, 124), // #653
  INST(Rdpkru           , X86Op              , O(000F01,EE,_,_,_,_,_,_  ), 0                         , 23 , 0  , 180, 125), // #654
  INST(Rdpmc            , X86Op              , O(000F00,33,_,_,_,_,_,_  ), 0                         , 5  , 0  , 180, 0  ), // #655
  INST(Rdpru            , X86Op              , O(000F01,FD,_,_,_,_,_,_  ), 0                         , 23 , 0  , 180, 126), // #656
  INST(Rdrand           , X86M               , O(000F00,C7,6,_,x,_,_,_  ), 0                         , 82 , 0  , 24 , 127), // #657
  INST(Rdseed           , X86M               , O(000F00,C7,7,_,x,_,_,_  ), 0                         , 24 , 0  , 24 , 128), // #658
  INST(Rdsspd           , X86M               , O(F30F00,1E,1,_,_,_,_,_  ), 0                         , 94 , 0  , 79 , 65 ), // #659
  INST(Rdsspq           , X86M               , O(F30F00,1E,1,_,_,_,_,_  ), 0                         , 94 , 0  , 80 , 65 ), // #660
  INST(Rdtsc            , X86Op              , O(000F00,31,_,_,_,_,_,_  ), 0                         , 5  , 0  , 29 , 129), // #661
  INST(Rdtscp           , X86Op              , O(000F01,F9,_,_,_,_,_,_  ), 0                         , 23 , 0  , 180, 130), // #662
  INST(Ret              , X86Ret             , O(000000,C2,_,_,_,_,_,_  ), 0                         , 0  , 0  , 181, 0  ), // #663
  INST(Retf             , X86Ret             , O(000000,CA,_,_,x,_,_,_  ), 0                         , 0  , 0  , 182, 0  ), // #664
  INST(Rmpadjust        , X86Op              , O(F30F01,FE,_,_,_,_,_,_  ), 0                         , 27 , 0  , 34 , 116), // #665
  INST(Rmpupdate        , X86Op              , O(F20F01,FE,_,_,_,_,_,_  ), 0                         , 93 , 0  , 34 , 116), // #666
  INST(Rol              , X86Rot             , O(000000,D0,0,_,x,_,_,_  ), 0                         , 0  , 0  , 176, 131), // #667
  INST(Ror              , X86Rot             , O(000000,D0,1,_,x,_,_,_  ), 0                         , 33 , 0  , 176, 131), // #668
  INST(Rorx             , VexRmi_Wx          , V(F20F3A,F0,_,0,x,_,_,_  ), 0                         , 96 , 0  , 183, 102), // #669
  INST(Roundpd          , ExtRmi             , O(660F3A,09,_,_,_,_,_,_  ), 0                         , 9  , 0  , 9  , 13 ), // #670
  INST(Roundps          , ExtRmi             , O(660F3A,08,_,_,_,_,_,_  ), 0                         , 9  , 0  , 9  , 13 ), // #671
  INST(Roundsd          , ExtRmi             , O(660F3A,0B,_,_,_,_,_,_  ), 0                         , 9  , 0  , 39 , 13 ), // #672
  INST(Roundss          , ExtRmi             , O(660F3A,0A,_,_,_,_,_,_  ), 0                         , 9  , 0  , 40 , 13 ), // #673
  INST(Rsm              , X86Op              , O(000F00,AA,_,_,_,_,_,_  ), 0                         , 5  , 0  , 84 , 1  ), // #674
  INST(Rsqrtps          , ExtRm              , O(000F00,52,_,_,_,_,_,_  ), 0                         , 5  , 0  , 6  , 6  ), // #675
  INST(Rsqrtss          , ExtRm              , O(F30F00,52,_,_,_,_,_,_  ), 0                         , 7  , 0  , 8  , 6  ), // #676
  INST(Rstorssp         , X86M_Only          , O(F30F00,01,5,_,_,_,_,_  ), 0                         , 65 , 0  , 33 , 25 ), // #677
  INST(Sahf             , X86Op              , O(000000,9E,_,_,_,_,_,_  ), 0                         , 0  , 0  , 101, 132), // #678
  INST(Sar              , X86Rot             , O(000000,D0,7,_,x,_,_,_  ), 0                         , 29 , 0  , 176, 1  ), // #679
  INST(Sarx             , VexRmv_Wx          , V(F30F38,F7,_,0,x,_,_,_  ), 0                         , 89 , 0  , 14 , 102), // #680
  INST(Saveprevssp      , X86Op              , O(F30F01,EA,_,_,_,_,_,_  ), 0                         , 27 , 0  , 31 , 25 ), // #681
  INST(Sbb              , X86Arith           , O(000000,18,3,_,x,_,_,_  ), 0                         , 77 , 0  , 184, 3  ), // #682
  INST(Scas             , X86StrRm           , O(000000,AE,_,_,_,_,_,_  ), 0                         , 0  , 0  , 185, 39 ), // #683
  INST(Seamcall         , X86Op              , O(660F01,CF,_,_,_,_,_,_  ), 0                         , 97 , 0  , 31 , 133), // #684
  INST(Seamops          , X86Op              , O(660F01,CE,_,_,_,_,_,_  ), 0                         , 97 , 0  , 31 , 133), // #685
  INST(Seamret          , X86Op              , O(660F01,CD,_,_,_,_,_,_  ), 0                         , 97 , 0  , 31 , 133), // #686
  INST(Senduipi         , X86M_NoSize        , O(F30F00,C7,6,_,_,_,_,_  ), 0                         , 26 , 0  , 80 , 26 ), // #687
  INST(Serialize        , X86Op              , O(000F01,E8,_,_,_,_,_,_  ), 0                         , 23 , 0  , 31 , 134), // #688
  INST(Setb             , X86Set             , O(000F00,92,_,_,_,_,_,_  ), 0                         , 5  , 0  , 186, 69 ), // #689
  INST(Setbe            , X86Set             , O(000F00,96,_,_,_,_,_,_  ), 0                         , 5  , 0  , 186, 70 ), // #690
  INST(Setl             , X86Set             , O(000F00,9C,_,_,_,_,_,_  ), 0                         , 5  , 0  , 186, 71 ), // #691
  INST(Setle            , X86Set             , O(000F00,9E,_,_,_,_,_,_  ), 0                         , 5  , 0  , 186, 72 ), // #692
  INST(Setnb            , X86Set             , O(000F00,93,_,_,_,_,_,_  ), 0                         , 5  , 0  , 186, 69 ), // #693
  INST(Setnbe           , X86Set             , O(000F00,97,_,_,_,_,_,_  ), 0                         , 5  , 0  , 186, 70 ), // #694
  INST(Setnl            , X86Set             , O(000F00,9D,_,_,_,_,_,_  ), 0                         , 5  , 0  , 186, 71 ), // #695
  INST(Setnle           , X86Set             , O(000F00,9F,_,_,_,_,_,_  ), 0                         , 5  , 0  , 186, 72 ), // #696
  INST(Setno            , X86Set             , O(000F00,91,_,_,_,_,_,_  ), 0                         , 5  , 0  , 186, 66 ), // #697
  INST(Setnp            , X86Set             , O(000F00,9B,_,_,_,_,_,_  ), 0                         , 5  , 0  , 186, 73 ), // #698
  INST(Setns            , X86Set             , O(000F00,99,_,_,_,_,_,_  ), 0                         , 5  , 0  , 186, 74 ), // #699
  INST(Setnz            , X86Set             , O(000F00,95,_,_,_,_,_,_  ), 0                         , 5  , 0  , 186, 75 ), // #700
  INST(Seto             , X86Set             , O(000F00,90,_,_,_,_,_,_  ), 0                         , 5  , 0  , 186, 66 ), // #701
  INST(Setp             , X86Set             , O(000F00,9A,_,_,_,_,_,_  ), 0                         , 5  , 0  , 186, 73 ), // #702
  INST(Sets             , X86Set             , O(000F00,98,_,_,_,_,_,_  ), 0                         , 5  , 0  , 186, 74 ), // #703
  INST(Setssbsy         , X86Op              , O(F30F01,E8,_,_,_,_,_,_  ), 0                         , 27 , 0  , 31 , 65 ), // #704
  INST(Setz             , X86Set             , O(000F00,94,_,_,_,_,_,_  ), 0                         , 5  , 0  , 186, 75 ), // #705
  INST(Sfence           , X86Fence           , O(000F00,AE,7,_,_,_,_,_  ), 0                         , 24 , 0  , 31 , 6  ), // #706
  INST(Sgdt             , X86M_Only          , O(000F00,01,0,_,_,_,_,_  ), 0                         , 5  , 0  , 32 , 0  ), // #707
  INST(Sha1msg1         , ExtRm              , O(000F38,C9,_,_,_,_,_,_  ), 0                         , 1  , 0  , 6  , 135), // #708
  INST(Sha1msg2         , ExtRm              , O(000F38,CA,_,_,_,_,_,_  ), 0                         , 1  , 0  , 6  , 135), // #709
  INST(Sha1nexte        , ExtRm              , O(000F38,C8,_,_,_,_,_,_  ), 0                         , 1  , 0  , 6  , 135), // #710
  INST(Sha1rnds4        , ExtRmi             , O(000F3A,CC,_,_,_,_,_,_  ), 0                         , 86 , 0  , 9  , 135), // #711
  INST(Sha256msg1       , ExtRm              , O(000F38,CC,_,_,_,_,_,_  ), 0                         , 1  , 0  , 6  , 135), // #712
  INST(Sha256msg2       , ExtRm              , O(000F38,CD,_,_,_,_,_,_  ), 0                         , 1  , 0  , 6  , 135), // #713
  INST(Sha256rnds2      , ExtRm_XMM0         , O(000F38,CB,_,_,_,_,_,_  ), 0                         , 1  , 0  , 16 , 135), // #714
  INST(Shl              , X86Rot             , O(000000,D0,4,_,x,_,_,_  ), 0                         , 10 , 0  , 176, 1  ), // #715
  INST(Shld             , X86ShldShrd        , O(000F00,A4,_,_,x,_,_,_  ), 0                         , 5  , 0  , 187, 1  ), // #716
  INST(Shlx             , VexRmv_Wx          , V(660F38,F7,_,0,x,_,_,_  ), 0                         , 30 , 0  , 14 , 102), // #717
  INST(Shr              , X86Rot             , O(000000,D0,5,_,x,_,_,_  ), 0                         , 64 , 0  , 176, 1  ), // #718
  INST(Shrd             , X86ShldShrd        , O(000F00,AC,_,_,x,_,_,_  ), 0                         , 5  , 0  , 187, 1  ), // #719
  INST(Shrx             , VexRmv_Wx          , V(F20F38,F7,_,0,x,_,_,_  ), 0                         , 85 , 0  , 14 , 102), // #720
  INST(Shufpd           , ExtRmi             , O(660F00,C6,_,_,_,_,_,_  ), 0                         , 4  , 0  , 9  , 5  ), // #721
  INST(Shufps           , ExtRmi             , O(000F00,C6,_,_,_,_,_,_  ), 0                         , 5  , 0  , 9  , 6  ), // #722
  INST(Sidt             , X86M_Only          , O(000F00,01,1,_,_,_,_,_  ), 0                         , 32 , 0  , 32 , 0  ), // #723
  INST(Skinit           , X86Op_xAX          , O(000F01,DE,_,_,_,_,_,_  ), 0                         , 23 , 0  , 54 , 136), // #724
  INST(Sldt             , X86M_NoMemSize     , O(000F00,00,0,_,_,_,_,_  ), 0                         , 5  , 0  , 188, 0  ), // #725
  INST(Slwpcb           , VexR_Wx            , V(XOP_M9,12,1,0,x,_,_,_  ), 0                         , 13 , 0  , 112, 87 ), // #726
  INST(Smsw             , X86M_NoMemSize     , O(000F00,01,4,_,_,_,_,_  ), 0                         , 98 , 0  , 188, 0  ), // #727
  INST(Sqrtpd           , ExtRm              , O(660F00,51,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6  , 5  ), // #728
  INST(Sqrtps           , ExtRm              , O(000F00,51,_,_,_,_,_,_  ), 0                         , 5  , 0  , 6  , 6  ), // #729
  INST(Sqrtsd           , ExtRm              , O(F20F00,51,_,_,_,_,_,_  ), 0                         , 6  , 0  , 7  , 5  ), // #730
  INST(Sqrtss           , ExtRm              , O(F30F00,51,_,_,_,_,_,_  ), 0                         , 7  , 0  , 8  , 6  ), // #731
  INST(Stac             , X86Op              , O(000F01,CB,_,_,_,_,_,_  ), 0                         , 23 , 0  , 31 , 17 ), // #732
  INST(Stc              , X86Op              , O(000000,F9,_,_,_,_,_,_  ), 0                         , 0  , 0  , 31 , 18 ), // #733
  INST(Std              , X86Op              , O(000000,FD,_,_,_,_,_,_  ), 0                         , 0  , 0  , 31 , 19 ), // #734
  INST(Stgi             , X86Op              , O(000F01,DC,_,_,_,_,_,_  ), 0                         , 23 , 0  , 31 , 136), // #735
  INST(Sti              , X86Op              , O(000000,FB,_,_,_,_,_,_  ), 0                         , 0  , 0  , 31 , 24 ), // #736
  INST(Stmxcsr          , X86M_Only          , O(000F00,AE,3,_,_,_,_,_  ), 0                         , 80 , 0  , 105, 6  ), // #737
  INST(Stos             , X86StrMr           , O(000000,AA,_,_,_,_,_,_  ), 0                         , 0  , 0  , 189, 88 ), // #738
  INST(Str              , X86M_NoMemSize     , O(000F00,00,1,_,_,_,_,_  ), 0                         , 32 , 0  , 188, 0  ), // #739
  INST(Sttilecfg        , AmxCfg             , V(660F38,49,_,0,0,_,_,_  ), 0                         , 30 , 0  , 107, 86 ), // #740
  INST(Stui             , X86Op              , O(F30F01,EF,_,_,_,_,_,_  ), 0                         , 27 , 0  , 34 , 26 ), // #741
  INST(Sub              , X86Arith           , O(000000,28,5,_,x,_,_,_  ), 0                         , 64 , 0  , 184, 1  ), // #742
  INST(Subpd            , ExtRm              , O(660F00,5C,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6  , 5  ), // #743
  INST(Subps            , ExtRm              , O(000F00,5C,_,_,_,_,_,_  ), 0                         , 5  , 0  , 6  , 6  ), // #744
  INST(Subsd            , ExtRm              , O(F20F00,5C,_,_,_,_,_,_  ), 0                         , 6  , 0  , 7  , 5  ), // #745
  INST(Subss            , ExtRm              , O(F30F00,5C,_,_,_,_,_,_  ), 0                         , 7  , 0  , 8  , 6  ), // #746
  INST(Swapgs           , X86Op              , O(000F01,F8,_,_,_,_,_,_  ), 0                         , 23 , 0  , 34 , 0  ), // #747
  INST(Syscall          , X86Op              , O(000F00,05,_,_,_,_,_,_  ), 0                         , 5  , 0  , 34 , 0  ), // #748
  INST(Sysenter         , X86Op              , O(000F00,34,_,_,_,_,_,_  ), 0                         , 5  , 0  , 31 , 0  ), // #749
  INST(Sysexit          , X86Op              , O(000F00,35,_,_,_,_,_,_  ), 0                         , 5  , 0  , 31 , 0  ), // #750
  INST(Sysexitq         , X86Op              , O(000F00,35,_,_,1,_,_,_  ), 0                         , 62 , 0  , 34 , 0  ), // #751
  INST(Sysret           , X86Op              , O(000F00,07,_,_,_,_,_,_  ), 0                         , 5  , 0  , 34 , 0  ), // #752
  INST(Sysretq          , X86Op              , O(000F00,07,_,_,1,_,_,_  ), 0                         , 62 , 0  , 34 , 0  ), // #753
  INST(T1mskc           , VexVm_Wx           , V(XOP_M9,01,7,0,x,_,_,_  ), 0                         , 99 , 0  , 15 , 12 ), // #754
  INST(Tcmmimfp16ps     , AmxRmv             , V(660F38,6C,_,0,0,_,_,_  ), 0                         , 30 , 0  , 190, 137), // #755
  INST(Tcmmrlfp16ps     , AmxRmv             , V(000F38,6C,_,0,0,_,_,_  ), 0                         , 11 , 0  , 190, 137), // #756
  INST(Tdcall           , X86Op              , O(660F01,CC,_,_,_,_,_,_  ), 0                         , 97 , 0  , 31 , 133), // #757
  INST(Tdpbf16ps        , AmxRmv             , V(F30F38,5C,_,0,0,_,_,_  ), 0                         , 89 , 0  , 190, 138), // #758
  INST(Tdpbssd          , AmxRmv             , V(F20F38,5E,_,0,0,_,_,_  ), 0                         , 85 , 0  , 190, 139), // #759
  INST(Tdpbsud          , AmxRmv             , V(F30F38,5E,_,0,0,_,_,_  ), 0                         , 89 , 0  , 190, 139), // #760
  INST(Tdpbusd          , AmxRmv             , V(660F38,5E,_,0,0,_,_,_  ), 0                         , 30 , 0  , 190, 139), // #761
  INST(Tdpbuud          , AmxRmv             , V(000F38,5E,_,0,0,_,_,_  ), 0                         , 11 , 0  , 190, 139), // #762
  INST(Tdpfp16ps        , AmxRmv             , V(F20F38,5C,_,0,0,_,_,_  ), 0                         , 85 , 0  , 190, 140), // #763
  INST(Test             , X86Test            , O(000000,84,_,_,x,_,_,_  ), O(000000,F6,_,_,x,_,_,_  ), 0  , 79 , 191, 1  ), // #764
  INST(Testui           , X86Op              , O(F30F01,ED,_,_,_,_,_,_  ), 0                         , 27 , 0  , 34 , 141), // #765
  INST(Tileloadd        , AmxRm              , V(F20F38,4B,_,0,0,_,_,_  ), 0                         , 85 , 0  , 192, 86 ), // #766
  INST(Tileloaddt1      , AmxRm              , V(660F38,4B,_,0,0,_,_,_  ), 0                         , 30 , 0  , 192, 86 ), // #767
  INST(Tilerelease      , VexOpMod           , V(000F38,49,0,0,0,_,_,_  ), 0                         , 11 , 0  , 193, 86 ), // #768
  INST(Tilestored       , AmxMr              , V(F30F38,4B,_,0,0,_,_,_  ), 0                         , 89 , 0  , 194, 86 ), // #769
  INST(Tilezero         , AmxR               , V(F20F38,49,_,0,0,_,_,_  ), 0                         , 85 , 0  , 195, 86 ), // #770
  INST(Tlbsync          , X86Op              , O(000F01,FF,_,_,_,_,_,_  ), 0                         , 23 , 0  , 31 , 68 ), // #771
  INST(Tpause           , X86R32_EDX_EAX     , O(660F00,AE,6,_,_,_,_,_  ), 0                         , 28 , 0  , 196, 142), // #772
  INST(Tzcnt            , X86Rm_Raw66H       , O(F30F00,BC,_,_,x,_,_,_  ), 0                         , 7  , 0  , 23 , 10 ), // #773
  INST(Tzmsk            , VexVm_Wx           , V(XOP_M9,01,4,0,x,_,_,_  ), 0                         , 100, 0  , 15 , 12 ), // #774
  INST(Ucomisd          , ExtRm              , O(660F00,2E,_,_,_,_,_,_  ), 0                         , 4  , 0  , 7  , 43 ), // #775
  INST(Ucomiss          , ExtRm              , O(000F00,2E,_,_,_,_,_,_  ), 0                         , 5  , 0  , 8  , 44 ), // #776
  INST(Ud0              , X86Rm              , O(000F00,FF,_,_,_,_,_,_  ), 0                         , 5  , 0  , 197, 0  ), // #777
  INST(Ud1              , X86Rm              , O(000F00,B9,_,_,_,_,_,_  ), 0                         , 5  , 0  , 197, 0  ), // #778
  INST(Ud2              , X86Op              , O(000F00,0B,_,_,_,_,_,_  ), 0                         , 5  , 0  , 31 , 0  ), // #779
  INST(Uiret            , X86Op              , O(F30F01,EC,_,_,_,_,_,_  ), 0                         , 27 , 0  , 34 , 26 ), // #780
  INST(Umonitor         , X86R_FromM         , O(F30F00,AE,6,_,_,_,_,_  ), 0                         , 26 , 0  , 198, 143), // #781
  INST(Umwait           , X86R32_EDX_EAX     , O(F20F00,AE,6,_,_,_,_,_  ), 0                         , 101, 0  , 196, 142), // #782
  INST(Unpckhpd         , ExtRm              , O(660F00,15,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6  , 5  ), // #783
  INST(Unpckhps         , ExtRm              , O(000F00,15,_,_,_,_,_,_  ), 0                         , 5  , 0  , 6  , 6  ), // #784
  INST(Unpcklpd         , ExtRm              , O(660F00,14,_,_,_,_,_,_  ), 0                         , 4  , 0  , 6  , 5  ), // #785
  INST(Unpcklps         , ExtRm              , O(000F00,14,_,_,_,_,_,_  ), 0                         , 5  , 0  , 6  , 6  ), // #786
  INST(Vaddpd           , VexRvm_Lx          , V(660F00,58,_,x,I,1,4,FV ), 0                         , 102, 0  , 199, 144), // #787
  INST(Vaddph           , VexRvm_Lx          , E(00MAP5,58,_,_,_,0,4,FV ), 0                         , 103, 0  , 200, 145), // #788
  INST(Vaddps           , VexRvm_Lx          , V(000F00,58,_,x,I,0,4,FV ), 0                         , 104, 0  , 201, 144), // #789
  INST(Vaddsd           , VexRvm             , V(F20F00,58,_,I,I,1,3,T1S), 0                         , 105, 0  , 202, 144), // #790
  INST(Vaddsh           , VexRvm             , E(F3MAP5,58,_,_,_,0,1,T1S), 0                         , 106, 0  , 203, 145), // #791
  INST(Vaddss           , VexRvm             , V(F30F00,58,_,I,I,0,2,T1S), 0                         , 107, 0  , 204, 144), // #792
  INST(Vaddsubpd        , VexRvm_Lx          , V(660F00,D0,_,x,I,_,_,_  ), 0                         , 71 , 0  , 205, 146), // #793
  INST(Vaddsubps        , VexRvm_Lx          , V(F20F00,D0,_,x,I,_,_,_  ), 0                         , 108, 0  , 205, 146), // #794
  INST(Vaesdec          , VexRvm_Lx          , V(660F38,DE,_,x,I,_,4,FVM), 0                         , 109, 0  , 206, 147), // #795
  INST(Vaesdeclast      , VexRvm_Lx          , V(660F38,DF,_,x,I,_,4,FVM), 0                         , 109, 0  , 206, 147), // #796
  INST(Vaesenc          , VexRvm_Lx          , V(660F38,DC,_,x,I,_,4,FVM), 0                         , 109, 0  , 206, 147), // #797
  INST(Vaesenclast      , VexRvm_Lx          , V(660F38,DD,_,x,I,_,4,FVM), 0                         , 109, 0  , 206, 147), // #798
  INST(Vaesimc          , VexRm              , V(660F38,DB,_,0,I,_,_,_  ), 0                         , 30 , 0  , 207, 148), // #799
  INST(Vaeskeygenassist , VexRmi             , V(660F3A,DF,_,0,I,_,_,_  ), 0                         , 75 , 0  , 208, 148), // #800
  INST(Valignd          , VexRvmi_Lx         , E(660F3A,03,_,x,_,0,4,FV ), 0                         , 110, 0  , 209, 149), // #801
  INST(Valignq          , VexRvmi_Lx         , E(660F3A,03,_,x,_,1,4,FV ), 0                         , 111, 0  , 210, 149), // #802
  INST(Vandnpd          , VexRvm_Lx          , V(660F00,55,_,x,I,1,4,FV ), 0                         , 102, 0  , 211, 150), // #803
  INST(Vandnps          , VexRvm_Lx          , V(000F00,55,_,x,I,0,4,FV ), 0                         , 104, 0  , 212, 150), // #804
  INST(Vandpd           , VexRvm_Lx          , V(660F00,54,_,x,I,1,4,FV ), 0                         , 102, 0  , 213, 150), // #805
  INST(Vandps           , VexRvm_Lx          , V(000F00,54,_,x,I,0,4,FV ), 0                         , 104, 0  , 214, 150), // #806
  INST(Vbcstnebf162ps   , VexRm_Lx           , V(F30F38,B1,_,x,0,_,_,_  ), 0                         , 89 , 0  , 215, 151), // #807
  INST(Vbcstnesh2ps     , VexRm_Lx           , V(660F38,B1,_,x,0,_,_,_  ), 0                         , 30 , 0  , 215, 151), // #808
  INST(Vblendmpd        , VexRvm_Lx          , E(660F38,65,_,x,_,1,4,FV ), 0                         , 112, 0  , 216, 149), // #809
  INST(Vblendmps        , VexRvm_Lx          , E(660F38,65,_,x,_,0,4,FV ), 0                         , 113, 0  , 217, 149), // #810
  INST(Vblendpd         , VexRvmi_Lx         , V(660F3A,0D,_,x,I,_,_,_  ), 0                         , 75 , 0  , 218, 146), // #811
  INST(Vblendps         , VexRvmi_Lx         , V(660F3A,0C,_,x,I,_,_,_  ), 0                         , 75 , 0  , 218, 146), // #812
  INST(Vblendvpd        , VexRvmr_Lx         , V(660F3A,4B,_,x,0,_,_,_  ), 0                         , 75 , 0  , 219, 146), // #813
  INST(Vblendvps        , VexRvmr_Lx         , V(660F3A,4A,_,x,0,_,_,_  ), 0                         , 75 , 0  , 219, 146), // #814
  INST(Vbroadcastf128   , VexRm              , V(660F38,1A,_,1,0,_,_,_  ), 0                         , 114, 0  , 220, 146), // #815
  INST(Vbroadcastf32x2  , VexRm_Lx           , E(660F38,19,_,x,_,0,3,T2 ), 0                         , 115, 0  , 221, 152), // #816
  INST(Vbroadcastf32x4  , VexRm_Lx           , E(660F38,1A,_,x,_,0,4,T4 ), 0                         , 116, 0  , 222, 149), // #817
  INST(Vbroadcastf32x8  , VexRm              , E(660F38,1B,_,2,_,0,5,T8 ), 0                         , 117, 0  , 223, 152), // #818
  INST(Vbroadcastf64x2  , VexRm_Lx           , E(660F38,1A,_,x,_,1,4,T2 ), 0                         , 118, 0  , 222, 152), // #819
  INST(Vbroadcastf64x4  , VexRm              , E(660F38,1B,_,2,_,1,5,T4 ), 0                         , 119, 0  , 223, 149), // #820
  INST(Vbroadcasti128   , VexRm              , V(660F38,5A,_,1,0,_,_,_  ), 0                         , 114, 0  , 220, 153), // #821
  INST(Vbroadcasti32x2  , VexRm_Lx           , E(660F38,59,_,x,_,0,3,T2 ), 0                         , 115, 0  , 224, 152), // #822
  INST(Vbroadcasti32x4  , VexRm_Lx           , E(660F38,5A,_,x,_,0,4,T4 ), 0                         , 116, 0  , 222, 149), // #823
  INST(Vbroadcasti32x8  , VexRm              , E(660F38,5B,_,2,_,0,5,T8 ), 0                         , 117, 0  , 223, 152), // #824
  INST(Vbroadcasti64x2  , VexRm_Lx           , E(660F38,5A,_,x,_,1,4,T2 ), 0                         , 118, 0  , 222, 152), // #825
  INST(Vbroadcasti64x4  , VexRm              , E(660F38,5B,_,2,_,1,5,T4 ), 0                         , 119, 0  , 223, 149), // #826
  INST(Vbroadcastsd     , VexRm_Lx           , V(660F38,19,_,x,0,1,3,T1S), 0                         , 120, 0  , 225, 154), // #827
  INST(Vbroadcastss     , VexRm_Lx           , V(660F38,18,_,x,0,0,2,T1S), 0                         , 121, 0  , 226, 154), // #828
  INST(Vcmppd           , VexRvmi_Lx_KEvex   , V(660F00,C2,_,x,I,1,4,FV ), 0                         , 102, 0  , 227, 144), // #829
  INST(Vcmpph           , VexRvmi_Lx_KEvex   , E(000F3A,C2,_,_,_,0,4,FV ), 0                         , 122, 0  , 228, 145), // #830
  INST(Vcmpps           , VexRvmi_Lx_KEvex   , V(000F00,C2,_,x,I,0,4,FV ), 0                         , 104, 0  , 229, 144), // #831
  INST(Vcmpsd           , VexRvmi_KEvex      , V(F20F00,C2,_,I,I,1,3,T1S), 0                         , 105, 0  , 230, 144), // #832
  INST(Vcmpsh           , VexRvmi_KEvex      , E(F30F3A,C2,_,_,_,0,1,T1S), 0                         , 123, 0  , 231, 145), // #833
  INST(Vcmpss           , VexRvmi_KEvex      , V(F30F00,C2,_,I,I,0,2,T1S), 0                         , 107, 0  , 232, 144), // #834
  INST(Vcomisd          , VexRm              , V(660F00,2F,_,I,I,1,3,T1S), 0                         , 124, 0  , 233, 155), // #835
  INST(Vcomish          , VexRm              , E(00MAP5,2F,_,_,_,0,1,T1S), 0                         , 125, 0  , 234, 156), // #836
  INST(Vcomiss          , VexRm              , V(000F00,2F,_,I,I,0,2,T1S), 0                         , 126, 0  , 235, 155), // #837
  INST(Vcompresspd      , VexMr_Lx           , E(660F38,8A,_,x,_,1,3,T1S), 0                         , 127, 0  , 236, 149), // #838
  INST(Vcompressps      , VexMr_Lx           , E(660F38,8A,_,x,_,0,2,T1S), 0                         , 128, 0  , 236, 149), // #839
  INST(Vcvtdq2pd        , VexRm_Lx           , V(F30F00,E6,_,x,I,0,3,HV ), 0                         , 129, 0  , 237, 144), // #840
  INST(Vcvtdq2ph        , VexRm_Lx_Narrow    , E(00MAP5,5B,_,x,0,0,4,FV ), 0                         , 103, 0  , 238, 145), // #841
  INST(Vcvtdq2ps        , VexRm_Lx           , V(000F00,5B,_,x,I,0,4,FV ), 0                         , 104, 0  , 239, 144), // #842
  INST(Vcvtne2ps2bf16   , VexRvm_Lx          , E(F20F38,72,_,_,_,0,4,FV ), 0                         , 130, 0  , 217, 157), // #843
  INST(Vcvtneebf162ps   , VexRm_Lx           , V(F30F38,B0,_,x,0,_,_,_  ), 0                         , 89 , 0  , 240, 151), // #844
  INST(Vcvtneeph2ps     , VexRm_Lx           , V(660F38,B0,_,x,0,_,_,_  ), 0                         , 30 , 0  , 240, 151), // #845
  INST(Vcvtneobf162ps   , VexRm_Lx           , V(F20F38,B0,_,x,0,_,_,_  ), 0                         , 85 , 0  , 240, 151), // #846
  INST(Vcvtneoph2ps     , VexRm_Lx           , V(000F38,B0,_,x,0,_,_,_  ), 0                         , 11 , 0  , 240, 151), // #847
  INST(Vcvtneps2bf16    , VexRm_Lx_Narrow    , V(F30F38,72,_,_,_,0,4,FV ), 0                         , 131, 0  , 241, 158), // #848
  INST(Vcvtpd2dq        , VexRm_Lx_Narrow    , V(F20F00,E6,_,x,I,1,4,FV ), 0                         , 132, 0  , 242, 144), // #849
  INST(Vcvtpd2ph        , VexRm_Lx           , E(66MAP5,5A,_,_,_,1,4,FV ), 0                         , 133, 0  , 243, 145), // #850
  INST(Vcvtpd2ps        , VexRm_Lx_Narrow    , V(660F00,5A,_,x,I,1,4,FV ), 0                         , 102, 0  , 242, 144), // #851
  INST(Vcvtpd2qq        , VexRm_Lx           , E(660F00,7B,_,x,_,1,4,FV ), 0                         , 134, 0  , 244, 152), // #852
  INST(Vcvtpd2udq       , VexRm_Lx_Narrow    , E(000F00,79,_,x,_,1,4,FV ), 0                         , 135, 0  , 245, 149), // #853
  INST(Vcvtpd2uqq       , VexRm_Lx           , E(660F00,79,_,x,_,1,4,FV ), 0                         , 134, 0  , 244, 152), // #854
  INST(Vcvtph2dq        , VexRm_Lx           , E(66MAP5,5B,_,_,_,0,3,HV ), 0                         , 136, 0  , 246, 145), // #855
  INST(Vcvtph2pd        , VexRm_Lx           , E(00MAP5,5A,_,_,_,0,2,QV ), 0                         , 137, 0  , 247, 145), // #856
  INST(Vcvtph2ps        , VexRm_Lx           , V(660F38,13,_,x,0,0,3,HVM), 0                         , 138, 0  , 248, 159), // #857
  INST(Vcvtph2psx       , VexRm_Lx           , E(66MAP6,13,_,_,_,0,3,HV ), 0                         , 139, 0  , 249, 145), // #858
  INST(Vcvtph2qq        , VexRm_Lx           , E(66MAP5,7B,_,_,_,0,2,QV ), 0                         , 140, 0  , 250, 145), // #859
  INST(Vcvtph2udq       , VexRm_Lx           , E(00MAP5,79,_,_,_,0,3,HV ), 0                         , 141, 0  , 246, 145), // #860
  INST(Vcvtph2uqq       , VexRm_Lx           , E(66MAP5,79,_,_,_,0,2,QV ), 0                         , 140, 0  , 250, 145), // #861
  INST(Vcvtph2uw        , VexRm_Lx           , E(00MAP5,7D,_,_,_,0,4,FV ), 0                         , 103, 0  , 251, 145), // #862
  INST(Vcvtph2w         , VexRm_Lx           , E(66MAP5,7D,_,_,_,0,4,FV ), 0                         , 142, 0  , 251, 145), // #863
  INST(Vcvtps2dq        , VexRm_Lx           , V(660F00,5B,_,x,I,0,4,FV ), 0                         , 143, 0  , 239, 144), // #864
  INST(Vcvtps2pd        , VexRm_Lx           , V(000F00,5A,_,x,I,0,3,HV ), 0                         , 144, 0  , 252, 144), // #865
  INST(Vcvtps2ph        , VexMri_Lx          , V(660F3A,1D,_,x,0,0,3,HVM), 0                         , 145, 0  , 253, 159), // #866
  INST(Vcvtps2phx       , VexRm_Lx_Narrow    , E(66MAP5,1D,_,_,_,0,4,FV ), 0                         , 142, 0  , 238, 145), // #867
  INST(Vcvtps2qq        , VexRm_Lx           , E(660F00,7B,_,x,_,0,3,HV ), 0                         , 146, 0  , 254, 152), // #868
  INST(Vcvtps2udq       , VexRm_Lx           , E(000F00,79,_,x,_,0,4,FV ), 0                         , 147, 0  , 255, 149), // #869
  INST(Vcvtps2uqq       , VexRm_Lx           , E(660F00,79,_,x,_,0,3,HV ), 0                         , 146, 0  , 254, 152), // #870
  INST(Vcvtqq2pd        , VexRm_Lx           , E(F30F00,E6,_,x,_,1,4,FV ), 0                         , 148, 0  , 244, 152), // #871
  INST(Vcvtqq2ph        , VexRm_Lx           , E(00MAP5,5B,_,_,_,1,4,FV ), 0                         , 149, 0  , 243, 145), // #872
  INST(Vcvtqq2ps        , VexRm_Lx_Narrow    , E(000F00,5B,_,x,_,1,4,FV ), 0                         , 135, 0  , 245, 152), // #873
  INST(Vcvtsd2sh        , VexRvm             , E(F2MAP5,5A,_,_,_,1,3,T1S), 0                         , 150, 0  , 256, 145), // #874
  INST(Vcvtsd2si        , VexRm_Wx           , V(F20F00,2D,_,I,x,x,3,T1F), 0                         , 151, 0  , 257, 144), // #875
  INST(Vcvtsd2ss        , VexRvm             , V(F20F00,5A,_,I,I,1,3,T1S), 0                         , 105, 0  , 202, 144), // #876
  INST(Vcvtsd2usi       , VexRm_Wx           , E(F20F00,79,_,I,_,x,3,T1F), 0                         , 152, 0  , 258, 149), // #877
  INST(Vcvtsh2sd        , VexRvm             , E(F3MAP5,5A,_,_,_,0,1,T1S), 0                         , 106, 0  , 259, 145), // #878
  INST(Vcvtsh2si        , VexRm_Wx           , E(F3MAP5,2D,_,_,_,x,1,T1S), 0                         , 106, 0  , 260, 145), // #879
  INST(Vcvtsh2ss        , VexRvm             , E(00MAP6,13,_,_,_,0,1,T1S), 0                         , 153, 0  , 259, 145), // #880
  INST(Vcvtsh2usi       , VexRm_Wx           , E(F3MAP5,79,_,_,_,x,1,T1S), 0                         , 106, 0  , 260, 145), // #881
  INST(Vcvtsi2sd        , VexRvm_Wx          , V(F20F00,2A,_,I,x,x,2,T1W), 0                         , 154, 0  , 261, 144), // #882
  INST(Vcvtsi2sh        , VexRvm_Wx          , E(F3MAP5,2A,_,_,_,x,2,T1W), 0                         , 155, 0  , 262, 145), // #883
  INST(Vcvtsi2ss        , VexRvm_Wx          , V(F30F00,2A,_,I,x,x,2,T1W), 0                         , 156, 0  , 261, 144), // #884
  INST(Vcvtss2sd        , VexRvm             , V(F30F00,5A,_,I,I,0,2,T1S), 0                         , 107, 0  , 263, 144), // #885
  INST(Vcvtss2sh        , VexRvm             , E(00MAP5,1D,_,_,_,0,2,T1S), 0                         , 157, 0  , 264, 145), // #886
  INST(Vcvtss2si        , VexRm_Wx           , V(F30F00,2D,_,I,x,x,2,T1F), 0                         , 107, 0  , 265, 144), // #887
  INST(Vcvtss2usi       , VexRm_Wx           , E(F30F00,79,_,I,_,x,2,T1F), 0                         , 158, 0  , 266, 149), // #888
  INST(Vcvttpd2dq       , VexRm_Lx_Narrow    , V(660F00,E6,_,x,I,1,4,FV ), 0                         , 102, 0  , 267, 144), // #889
  INST(Vcvttpd2qq       , VexRm_Lx           , E(660F00,7A,_,x,_,1,4,FV ), 0                         , 134, 0  , 268, 149), // #890
  INST(Vcvttpd2udq      , VexRm_Lx_Narrow    , E(000F00,78,_,x,_,1,4,FV ), 0                         , 135, 0  , 269, 149), // #891
  INST(Vcvttpd2uqq      , VexRm_Lx           , E(660F00,78,_,x,_,1,4,FV ), 0                         , 134, 0  , 268, 152), // #892
  INST(Vcvttph2dq       , VexRm_Lx           , E(F3MAP5,5B,_,_,_,0,3,HV ), 0                         , 159, 0  , 249, 145), // #893
  INST(Vcvttph2qq       , VexRm_Lx           , E(66MAP5,7A,_,_,_,0,2,QV ), 0                         , 140, 0  , 247, 145), // #894
  INST(Vcvttph2udq      , VexRm_Lx           , E(00MAP5,78,_,_,_,0,3,HV ), 0                         , 141, 0  , 249, 145), // #895
  INST(Vcvttph2uqq      , VexRm_Lx           , E(66MAP5,78,_,_,_,0,2,QV ), 0                         , 140, 0  , 247, 145), // #896
  INST(Vcvttph2uw       , VexRm_Lx           , E(00MAP5,7C,_,_,_,0,4,FV ), 0                         , 103, 0  , 270, 145), // #897
  INST(Vcvttph2w        , VexRm_Lx           , E(66MAP5,7C,_,_,_,0,4,FV ), 0                         , 142, 0  , 270, 145), // #898
  INST(Vcvttps2dq       , VexRm_Lx           , V(F30F00,5B,_,x,I,0,4,FV ), 0                         , 160, 0  , 271, 144), // #899
  INST(Vcvttps2qq       , VexRm_Lx           , E(660F00,7A,_,x,_,0,3,HV ), 0                         , 146, 0  , 272, 152), // #900
  INST(Vcvttps2udq      , VexRm_Lx           , E(000F00,78,_,x,_,0,4,FV ), 0                         , 147, 0  , 273, 149), // #901
  INST(Vcvttps2uqq      , VexRm_Lx           , E(660F00,78,_,x,_,0,3,HV ), 0                         , 146, 0  , 272, 152), // #902
  INST(Vcvttsd2si       , VexRm_Wx           , V(F20F00,2C,_,I,x,x,3,T1F), 0                         , 151, 0  , 274, 144), // #903
  INST(Vcvttsd2usi      , VexRm_Wx           , E(F20F00,78,_,I,_,x,3,T1F), 0                         , 152, 0  , 275, 149), // #904
  INST(Vcvttsh2si       , VexRm_Wx           , E(F3MAP5,2C,_,_,_,x,1,T1S), 0                         , 106, 0  , 276, 145), // #905
  INST(Vcvttsh2usi      , VexRm_Wx           , E(F3MAP5,78,_,_,_,x,1,T1S), 0                         , 106, 0  , 276, 145), // #906
  INST(Vcvttss2si       , VexRm_Wx           , V(F30F00,2C,_,I,x,x,2,T1F), 0                         , 107, 0  , 277, 144), // #907
  INST(Vcvttss2usi      , VexRm_Wx           , E(F30F00,78,_,I,_,x,2,T1F), 0                         , 158, 0  , 278, 149), // #908
  INST(Vcvtudq2pd       , VexRm_Lx           , E(F30F00,7A,_,x,_,0,3,HV ), 0                         , 161, 0  , 254, 149), // #909
  INST(Vcvtudq2ph       , VexRm_Lx_Narrow    , E(F2MAP5,7A,_,_,_,0,4,FV ), 0                         , 162, 0  , 238, 145), // #910
  INST(Vcvtudq2ps       , VexRm_Lx           , E(F20F00,7A,_,x,_,0,4,FV ), 0                         , 163, 0  , 255, 149), // #911
  INST(Vcvtuqq2pd       , VexRm_Lx           , E(F30F00,7A,_,x,_,1,4,FV ), 0                         , 148, 0  , 244, 152), // #912
  INST(Vcvtuqq2ph       , VexRm_Lx           , E(F2MAP5,7A,_,_,_,1,4,FV ), 0                         , 164, 0  , 243, 145), // #913
  INST(Vcvtuqq2ps       , VexRm_Lx_Narrow    , E(F20F00,7A,_,x,_,1,4,FV ), 0                         , 165, 0  , 245, 152), // #914
  INST(Vcvtusi2sd       , VexRvm_Wx          , E(F20F00,7B,_,I,_,x,2,T1W), 0                         , 166, 0  , 279, 149), // #915
  INST(Vcvtusi2sh       , VexRvm_Wx          , E(F3MAP5,7B,_,_,_,x,2,T1W), 0                         , 155, 0  , 262, 145), // #916
  INST(Vcvtusi2ss       , VexRvm_Wx          , E(F30F00,7B,_,I,_,x,2,T1W), 0                         , 167, 0  , 279, 149), // #917
  INST(Vcvtuw2ph        , VexRm_Lx           , E(F2MAP5,7D,_,_,_,0,4,FV ), 0                         , 162, 0  , 251, 145), // #918
  INST(Vcvtw2ph         , VexRm_Lx           , E(F3MAP5,7D,_,_,_,0,4,FV ), 0                         , 168, 0  , 251, 145), // #919
  INST(Vdbpsadbw        , VexRvmi_Lx         , E(660F3A,42,_,x,_,0,4,FVM), 0                         , 110, 0  , 280, 160), // #920
  INST(Vdivpd           , VexRvm_Lx          , V(660F00,5E,_,x,I,1,4,FV ), 0                         , 102, 0  , 199, 144), // #921
  INST(Vdivph           , VexRvm_Lx          , E(00MAP5,5E,_,_,_,0,4,FV ), 0                         , 103, 0  , 200, 145), // #922
  INST(Vdivps           , VexRvm_Lx          , V(000F00,5E,_,x,I,0,4,FV ), 0                         , 104, 0  , 201, 144), // #923
  INST(Vdivsd           , VexRvm             , V(F20F00,5E,_,I,I,1,3,T1S), 0                         , 105, 0  , 202, 144), // #924
  INST(Vdivsh           , VexRvm             , E(F3MAP5,5E,_,_,_,0,1,T1S), 0                         , 106, 0  , 203, 145), // #925
  INST(Vdivss           , VexRvm             , V(F30F00,5E,_,I,I,0,2,T1S), 0                         , 107, 0  , 204, 144), // #926
  INST(Vdpbf16ps        , VexRvm_Lx          , E(F30F38,52,_,_,_,0,4,FV ), 0                         , 169, 0  , 217, 157), // #927
  INST(Vdppd            , VexRvmi_Lx         , V(660F3A,41,_,x,I,_,_,_  ), 0                         , 75 , 0  , 281, 146), // #928
  INST(Vdpps            , VexRvmi_Lx         , V(660F3A,40,_,x,I,_,_,_  ), 0                         , 75 , 0  , 218, 146), // #929
  INST(Verr             , X86M_NoSize        , O(000F00,00,4,_,_,_,_,_  ), 0                         , 98 , 0  , 111, 11 ), // #930
  INST(Verw             , X86M_NoSize        , O(000F00,00,5,_,_,_,_,_  ), 0                         , 79 , 0  , 111, 11 ), // #931
  INST(Vexpandpd        , VexRm_Lx           , E(660F38,88,_,x,_,1,3,T1S), 0                         , 127, 0  , 282, 149), // #932
  INST(Vexpandps        , VexRm_Lx           , E(660F38,88,_,x,_,0,2,T1S), 0                         , 128, 0  , 282, 149), // #933
  INST(Vextractf128     , VexMri             , V(660F3A,19,_,1,0,_,_,_  ), 0                         , 170, 0  , 283, 146), // #934
  INST(Vextractf32x4    , VexMri_Lx          , E(660F3A,19,_,x,_,0,4,T4 ), 0                         , 171, 0  , 284, 149), // #935
  INST(Vextractf32x8    , VexMri             , E(660F3A,1B,_,2,_,0,5,T8 ), 0                         , 172, 0  , 285, 152), // #936
  INST(Vextractf64x2    , VexMri_Lx          , E(660F3A,19,_,x,_,1,4,T2 ), 0                         , 173, 0  , 284, 152), // #937
  INST(Vextractf64x4    , VexMri             , E(660F3A,1B,_,2,_,1,5,T4 ), 0                         , 174, 0  , 285, 149), // #938
  INST(Vextracti128     , VexMri             , V(660F3A,39,_,1,0,_,_,_  ), 0                         , 170, 0  , 283, 153), // #939
  INST(Vextracti32x4    , VexMri_Lx          , E(660F3A,39,_,x,_,0,4,T4 ), 0                         , 171, 0  , 284, 149), // #940
  INST(Vextracti32x8    , VexMri             , E(660F3A,3B,_,2,_,0,5,T8 ), 0                         , 172, 0  , 285, 152), // #941
  INST(Vextracti64x2    , VexMri_Lx          , E(660F3A,39,_,x,_,1,4,T2 ), 0                         , 173, 0  , 284, 152), // #942
  INST(Vextracti64x4    , VexMri             , E(660F3A,3B,_,2,_,1,5,T4 ), 0                         , 174, 0  , 285, 149), // #943
  INST(Vextractps       , VexMri             , V(660F3A,17,_,0,I,I,2,T1S), 0                         , 175, 0  , 286, 144), // #944
  INST(Vfcmaddcph       , VexRvm_Lx          , E(F2MAP6,56,_,_,_,0,4,FV ), 0                         , 176, 0  , 287, 145), // #945
  INST(Vfcmaddcsh       , VexRvm             , E(F2MAP6,57,_,_,_,0,2,T1S), 0                         , 177, 0  , 264, 145), // #946
  INST(Vfcmulcph        , VexRvm_Lx          , E(F2MAP6,D6,_,_,_,0,4,FV ), 0                         , 176, 0  , 287, 145), // #947
  INST(Vfcmulcsh        , VexRvm             , E(F2MAP6,D7,_,_,_,0,2,T1S), 0                         , 177, 0  , 264, 145), // #948
  INST(Vfixupimmpd      , VexRvmi_Lx         , E(660F3A,54,_,x,_,1,4,FV ), 0                         , 111, 0  , 288, 149), // #949
  INST(Vfixupimmps      , VexRvmi_Lx         , E(660F3A,54,_,x,_,0,4,FV ), 0                         , 110, 0  , 289, 149), // #950
  INST(Vfixupimmsd      , VexRvmi            , E(660F3A,55,_,I,_,1,3,T1S), 0                         , 178, 0  , 290, 149), // #951
  INST(Vfixupimmss      , VexRvmi            , E(660F3A,55,_,I,_,0,2,T1S), 0                         , 179, 0  , 291, 149), // #952
  INST(Vfmadd132pd      , VexRvm_Lx          , V(660F38,98,_,x,1,1,4,FV ), 0                         , 180, 0  , 199, 161), // #953
  INST(Vfmadd132ph      , VexRvm_Lx          , E(66MAP6,98,_,_,_,0,4,FV ), 0                         , 181, 0  , 200, 145), // #954
  INST(Vfmadd132ps      , VexRvm_Lx          , V(660F38,98,_,x,0,0,4,FV ), 0                         , 109, 0  , 201, 161), // #955
  INST(Vfmadd132sd      , VexRvm             , V(660F38,99,_,I,1,1,3,T1S), 0                         , 182, 0  , 202, 161), // #956
  INST(Vfmadd132sh      , VexRvm             , E(66MAP6,99,_,_,_,0,1,T1S), 0                         , 183, 0  , 203, 145), // #957
  INST(Vfmadd132ss      , VexRvm             , V(660F38,99,_,I,0,0,2,T1S), 0                         , 121, 0  , 204, 161), // #958
  INST(Vfmadd213pd      , VexRvm_Lx          , V(660F38,A8,_,x,1,1,4,FV ), 0                         , 180, 0  , 199, 161), // #959
  INST(Vfmadd213ph      , VexRvm_Lx          , E(66MAP6,A8,_,_,_,0,4,FV ), 0                         , 181, 0  , 200, 145), // #960
  INST(Vfmadd213ps      , VexRvm_Lx          , V(660F38,A8,_,x,0,0,4,FV ), 0                         , 109, 0  , 201, 161), // #961
  INST(Vfmadd213sd      , VexRvm             , V(660F38,A9,_,I,1,1,3,T1S), 0                         , 182, 0  , 202, 161), // #962
  INST(Vfmadd213sh      , VexRvm             , E(66MAP6,A9,_,_,_,0,1,T1S), 0                         , 183, 0  , 203, 145), // #963
  INST(Vfmadd213ss      , VexRvm             , V(660F38,A9,_,I,0,0,2,T1S), 0                         , 121, 0  , 204, 161), // #964
  INST(Vfmadd231pd      , VexRvm_Lx          , V(660F38,B8,_,x,1,1,4,FV ), 0                         , 180, 0  , 199, 161), // #965
  INST(Vfmadd231ph      , VexRvm_Lx          , E(66MAP6,B8,_,_,_,0,4,FV ), 0                         , 181, 0  , 200, 145), // #966
  INST(Vfmadd231ps      , VexRvm_Lx          , V(660F38,B8,_,x,0,0,4,FV ), 0                         , 109, 0  , 201, 161), // #967
  INST(Vfmadd231sd      , VexRvm             , V(660F38,B9,_,I,1,1,3,T1S), 0                         , 182, 0  , 202, 161), // #968
  INST(Vfmadd231sh      , VexRvm             , E(66MAP6,B9,_,_,_,0,1,T1S), 0                         , 183, 0  , 203, 145), // #969
  INST(Vfmadd231ss      , VexRvm             , V(660F38,B9,_,I,0,0,2,T1S), 0                         , 121, 0  , 204, 161), // #970
  INST(Vfmaddcph        , VexRvm_Lx          , E(F3MAP6,56,_,_,_,0,4,FV ), 0                         , 184, 0  , 287, 145), // #971
  INST(Vfmaddcsh        , VexRvm             , E(F3MAP6,57,_,_,_,0,2,T1S), 0                         , 185, 0  , 264, 145), // #972
  INST(Vfmaddpd         , Fma4_Lx            , V(660F3A,69,_,x,x,_,_,_  ), 0                         , 75 , 0  , 292, 162), // #973
  INST(Vfmaddps         , Fma4_Lx            , V(660F3A,68,_,x,x,_,_,_  ), 0                         , 75 , 0  , 292, 162), // #974
  INST(Vfmaddsd         , Fma4               , V(660F3A,6B,_,0,x,_,_,_  ), 0                         , 75 , 0  , 293, 162), // #975
  INST(Vfmaddss         , Fma4               , V(660F3A,6A,_,0,x,_,_,_  ), 0                         , 75 , 0  , 294, 162), // #976
  INST(Vfmaddsub132pd   , VexRvm_Lx          , V(660F38,96,_,x,1,1,4,FV ), 0                         , 180, 0  , 199, 161), // #977
  INST(Vfmaddsub132ph   , VexRvm_Lx          , E(66MAP6,96,_,_,_,0,4,FV ), 0                         , 181, 0  , 200, 145), // #978
  INST(Vfmaddsub132ps   , VexRvm_Lx          , V(660F38,96,_,x,0,0,4,FV ), 0                         , 109, 0  , 201, 161), // #979
  INST(Vfmaddsub213pd   , VexRvm_Lx          , V(660F38,A6,_,x,1,1,4,FV ), 0                         , 180, 0  , 199, 161), // #980
  INST(Vfmaddsub213ph   , VexRvm_Lx          , E(66MAP6,A6,_,_,_,0,4,FV ), 0                         , 181, 0  , 200, 145), // #981
  INST(Vfmaddsub213ps   , VexRvm_Lx          , V(660F38,A6,_,x,0,0,4,FV ), 0                         , 109, 0  , 201, 161), // #982
  INST(Vfmaddsub231pd   , VexRvm_Lx          , V(660F38,B6,_,x,1,1,4,FV ), 0                         , 180, 0  , 199, 161), // #983
  INST(Vfmaddsub231ph   , VexRvm_Lx          , E(66MAP6,B6,_,_,_,0,4,FV ), 0                         , 181, 0  , 200, 145), // #984
  INST(Vfmaddsub231ps   , VexRvm_Lx          , V(660F38,B6,_,x,0,0,4,FV ), 0                         , 109, 0  , 201, 161), // #985
  INST(Vfmaddsubpd      , Fma4_Lx            , V(660F3A,5D,_,x,x,_,_,_  ), 0                         , 75 , 0  , 292, 162), // #986
  INST(Vfmaddsubps      , Fma4_Lx            , V(660F3A,5C,_,x,x,_,_,_  ), 0                         , 75 , 0  , 292, 162), // #987
  INST(Vfmsub132pd      , VexRvm_Lx          , V(660F38,9A,_,x,1,1,4,FV ), 0                         , 180, 0  , 199, 161), // #988
  INST(Vfmsub132ph      , VexRvm_Lx          , E(66MAP6,9A,_,_,_,0,4,FV ), 0                         , 181, 0  , 200, 145), // #989
  INST(Vfmsub132ps      , VexRvm_Lx          , V(660F38,9A,_,x,0,0,4,FV ), 0                         , 109, 0  , 201, 161), // #990
  INST(Vfmsub132sd      , VexRvm             , V(660F38,9B,_,I,1,1,3,T1S), 0                         , 182, 0  , 202, 161), // #991
  INST(Vfmsub132sh      , VexRvm             , E(66MAP6,9B,_,_,_,0,1,T1S), 0                         , 183, 0  , 203, 145), // #992
  INST(Vfmsub132ss      , VexRvm             , V(660F38,9B,_,I,0,0,2,T1S), 0                         , 121, 0  , 204, 161), // #993
  INST(Vfmsub213pd      , VexRvm_Lx          , V(660F38,AA,_,x,1,1,4,FV ), 0                         , 180, 0  , 199, 161), // #994
  INST(Vfmsub213ph      , VexRvm_Lx          , E(66MAP6,AA,_,_,_,0,4,FV ), 0                         , 181, 0  , 200, 145), // #995
  INST(Vfmsub213ps      , VexRvm_Lx          , V(660F38,AA,_,x,0,0,4,FV ), 0                         , 109, 0  , 201, 161), // #996
  INST(Vfmsub213sd      , VexRvm             , V(660F38,AB,_,I,1,1,3,T1S), 0                         , 182, 0  , 202, 161), // #997
  INST(Vfmsub213sh      , VexRvm             , E(66MAP6,AB,_,_,_,0,1,T1S), 0                         , 183, 0  , 203, 145), // #998
  INST(Vfmsub213ss      , VexRvm             , V(660F38,AB,_,I,0,0,2,T1S), 0                         , 121, 0  , 204, 161), // #999
  INST(Vfmsub231pd      , VexRvm_Lx          , V(660F38,BA,_,x,1,1,4,FV ), 0                         , 180, 0  , 199, 161), // #1000
  INST(Vfmsub231ph      , VexRvm_Lx          , E(66MAP6,BA,_,_,_,0,4,FV ), 0                         , 181, 0  , 200, 145), // #1001
  INST(Vfmsub231ps      , VexRvm_Lx          , V(660F38,BA,_,x,0,0,4,FV ), 0                         , 109, 0  , 201, 161), // #1002
  INST(Vfmsub231sd      , VexRvm             , V(660F38,BB,_,I,1,1,3,T1S), 0                         , 182, 0  , 202, 161), // #1003
  INST(Vfmsub231sh      , VexRvm             , E(66MAP6,BB,_,_,_,0,1,T1S), 0                         , 183, 0  , 203, 145), // #1004
  INST(Vfmsub231ss      , VexRvm             , V(660F38,BB,_,I,0,0,2,T1S), 0                         , 121, 0  , 204, 161), // #1005
  INST(Vfmsubadd132pd   , VexRvm_Lx          , V(660F38,97,_,x,1,1,4,FV ), 0                         , 180, 0  , 199, 161), // #1006
  INST(Vfmsubadd132ph   , VexRvm_Lx          , E(66MAP6,97,_,_,_,0,4,FV ), 0                         , 181, 0  , 200, 145), // #1007
  INST(Vfmsubadd132ps   , VexRvm_Lx          , V(660F38,97,_,x,0,0,4,FV ), 0                         , 109, 0  , 201, 161), // #1008
  INST(Vfmsubadd213pd   , VexRvm_Lx          , V(660F38,A7,_,x,1,1,4,FV ), 0                         , 180, 0  , 199, 161), // #1009
  INST(Vfmsubadd213ph   , VexRvm_Lx          , E(66MAP6,A7,_,_,_,0,4,FV ), 0                         , 181, 0  , 200, 145), // #1010
  INST(Vfmsubadd213ps   , VexRvm_Lx          , V(660F38,A7,_,x,0,0,4,FV ), 0                         , 109, 0  , 201, 161), // #1011
  INST(Vfmsubadd231pd   , VexRvm_Lx          , V(660F38,B7,_,x,1,1,4,FV ), 0                         , 180, 0  , 199, 161), // #1012
  INST(Vfmsubadd231ph   , VexRvm_Lx          , E(66MAP6,B7,_,_,_,0,4,FV ), 0                         , 181, 0  , 200, 145), // #1013
  INST(Vfmsubadd231ps   , VexRvm_Lx          , V(660F38,B7,_,x,0,0,4,FV ), 0                         , 109, 0  , 201, 161), // #1014
  INST(Vfmsubaddpd      , Fma4_Lx            , V(660F3A,5F,_,x,x,_,_,_  ), 0                         , 75 , 0  , 292, 162), // #1015
  INST(Vfmsubaddps      , Fma4_Lx            , V(660F3A,5E,_,x,x,_,_,_  ), 0                         , 75 , 0  , 292, 162), // #1016
  INST(Vfmsubpd         , Fma4_Lx            , V(660F3A,6D,_,x,x,_,_,_  ), 0                         , 75 , 0  , 292, 162), // #1017
  INST(Vfmsubps         , Fma4_Lx            , V(660F3A,6C,_,x,x,_,_,_  ), 0                         , 75 , 0  , 292, 162), // #1018
  INST(Vfmsubsd         , Fma4               , V(660F3A,6F,_,0,x,_,_,_  ), 0                         , 75 , 0  , 293, 162), // #1019
  INST(Vfmsubss         , Fma4               , V(660F3A,6E,_,0,x,_,_,_  ), 0                         , 75 , 0  , 294, 162), // #1020
  INST(Vfmulcph         , VexRvm_Lx          , E(F3MAP6,D6,_,_,_,0,4,FV ), 0                         , 184, 0  , 287, 145), // #1021
  INST(Vfmulcsh         , VexRvm             , E(F3MAP6,D7,_,_,_,0,2,T1S), 0                         , 185, 0  , 264, 145), // #1022
  INST(Vfnmadd132pd     , VexRvm_Lx          , V(660F38,9C,_,x,1,1,4,FV ), 0                         , 180, 0  , 199, 161), // #1023
  INST(Vfnmadd132ph     , VexRvm_Lx          , E(66MAP6,9C,_,_,_,0,4,FV ), 0                         , 181, 0  , 200, 145), // #1024
  INST(Vfnmadd132ps     , VexRvm_Lx          , V(660F38,9C,_,x,0,0,4,FV ), 0                         , 109, 0  , 201, 161), // #1025
  INST(Vfnmadd132sd     , VexRvm             , V(660F38,9D,_,I,1,1,3,T1S), 0                         , 182, 0  , 202, 161), // #1026
  INST(Vfnmadd132sh     , VexRvm             , E(66MAP6,9D,_,_,_,0,1,T1S), 0                         , 183, 0  , 203, 145), // #1027
  INST(Vfnmadd132ss     , VexRvm             , V(660F38,9D,_,I,0,0,2,T1S), 0                         , 121, 0  , 204, 161), // #1028
  INST(Vfnmadd213pd     , VexRvm_Lx          , V(660F38,AC,_,x,1,1,4,FV ), 0                         , 180, 0  , 199, 161), // #1029
  INST(Vfnmadd213ph     , VexRvm_Lx          , E(66MAP6,AC,_,_,_,0,4,FV ), 0                         , 181, 0  , 200, 145), // #1030
  INST(Vfnmadd213ps     , VexRvm_Lx          , V(660F38,AC,_,x,0,0,4,FV ), 0                         , 109, 0  , 201, 161), // #1031
  INST(Vfnmadd213sd     , VexRvm             , V(660F38,AD,_,I,1,1,3,T1S), 0                         , 182, 0  , 202, 161), // #1032
  INST(Vfnmadd213sh     , VexRvm             , E(66MAP6,AD,_,_,_,0,1,T1S), 0                         , 183, 0  , 203, 145), // #1033
  INST(Vfnmadd213ss     , VexRvm             , V(660F38,AD,_,I,0,0,2,T1S), 0                         , 121, 0  , 204, 161), // #1034
  INST(Vfnmadd231pd     , VexRvm_Lx          , V(660F38,BC,_,x,1,1,4,FV ), 0                         , 180, 0  , 199, 161), // #1035
  INST(Vfnmadd231ph     , VexRvm_Lx          , E(66MAP6,BC,_,_,_,0,4,FV ), 0                         , 181, 0  , 200, 145), // #1036
  INST(Vfnmadd231ps     , VexRvm_Lx          , V(660F38,BC,_,x,0,0,4,FV ), 0                         , 109, 0  , 201, 161), // #1037
  INST(Vfnmadd231sd     , VexRvm             , V(660F38,BD,_,I,1,1,3,T1S), 0                         , 182, 0  , 202, 161), // #1038
  INST(Vfnmadd231sh     , VexRvm             , E(66MAP6,BD,_,_,_,0,1,T1S), 0                         , 183, 0  , 203, 145), // #1039
  INST(Vfnmadd231ss     , VexRvm             , V(660F38,BD,_,I,0,0,2,T1S), 0                         , 121, 0  , 204, 161), // #1040
  INST(Vfnmaddpd        , Fma4_Lx            , V(660F3A,79,_,x,x,_,_,_  ), 0                         , 75 , 0  , 292, 162), // #1041
  INST(Vfnmaddps        , Fma4_Lx            , V(660F3A,78,_,x,x,_,_,_  ), 0                         , 75 , 0  , 292, 162), // #1042
  INST(Vfnmaddsd        , Fma4               , V(660F3A,7B,_,0,x,_,_,_  ), 0                         , 75 , 0  , 293, 162), // #1043
  INST(Vfnmaddss        , Fma4               , V(660F3A,7A,_,0,x,_,_,_  ), 0                         , 75 , 0  , 294, 162), // #1044
  INST(Vfnmsub132pd     , VexRvm_Lx          , V(660F38,9E,_,x,1,1,4,FV ), 0                         , 180, 0  , 199, 161), // #1045
  INST(Vfnmsub132ph     , VexRvm_Lx          , E(66MAP6,9E,_,_,_,0,4,FV ), 0                         , 181, 0  , 200, 145), // #1046
  INST(Vfnmsub132ps     , VexRvm_Lx          , V(660F38,9E,_,x,0,0,4,FV ), 0                         , 109, 0  , 201, 161), // #1047
  INST(Vfnmsub132sd     , VexRvm             , V(660F38,9F,_,I,1,1,3,T1S), 0                         , 182, 0  , 202, 161), // #1048
  INST(Vfnmsub132sh     , VexRvm             , E(66MAP6,9F,_,_,_,0,1,T1S), 0                         , 183, 0  , 203, 145), // #1049
  INST(Vfnmsub132ss     , VexRvm             , V(660F38,9F,_,I,0,0,2,T1S), 0                         , 121, 0  , 204, 161), // #1050
  INST(Vfnmsub213pd     , VexRvm_Lx          , V(660F38,AE,_,x,1,1,4,FV ), 0                         , 180, 0  , 199, 161), // #1051
  INST(Vfnmsub213ph     , VexRvm_Lx          , E(66MAP6,AE,_,_,_,0,4,FV ), 0                         , 181, 0  , 200, 145), // #1052
  INST(Vfnmsub213ps     , VexRvm_Lx          , V(660F38,AE,_,x,0,0,4,FV ), 0                         , 109, 0  , 201, 161), // #1053
  INST(Vfnmsub213sd     , VexRvm             , V(660F38,AF,_,I,1,1,3,T1S), 0                         , 182, 0  , 202, 161), // #1054
  INST(Vfnmsub213sh     , VexRvm             , E(66MAP6,AF,_,_,_,0,1,T1S), 0                         , 183, 0  , 203, 145), // #1055
  INST(Vfnmsub213ss     , VexRvm             , V(660F38,AF,_,I,0,0,2,T1S), 0                         , 121, 0  , 204, 161), // #1056
  INST(Vfnmsub231pd     , VexRvm_Lx          , V(660F38,BE,_,x,1,1,4,FV ), 0                         , 180, 0  , 199, 161), // #1057
  INST(Vfnmsub231ph     , VexRvm_Lx          , E(66MAP6,BE,_,_,_,0,4,FV ), 0                         , 181, 0  , 200, 145), // #1058
  INST(Vfnmsub231ps     , VexRvm_Lx          , V(660F38,BE,_,x,0,0,4,FV ), 0                         , 109, 0  , 201, 161), // #1059
  INST(Vfnmsub231sd     , VexRvm             , V(660F38,BF,_,I,1,1,3,T1S), 0                         , 182, 0  , 202, 161), // #1060
  INST(Vfnmsub231sh     , VexRvm             , E(66MAP6,BF,_,_,_,0,1,T1S), 0                         , 183, 0  , 203, 145), // #1061
  INST(Vfnmsub231ss     , VexRvm             , V(660F38,BF,_,I,0,0,2,T1S), 0                         , 121, 0  , 204, 161), // #1062
  INST(Vfnmsubpd        , Fma4_Lx            , V(660F3A,7D,_,x,x,_,_,_  ), 0                         , 75 , 0  , 292, 162), // #1063
  INST(Vfnmsubps        , Fma4_Lx            , V(660F3A,7C,_,x,x,_,_,_  ), 0                         , 75 , 0  , 292, 162), // #1064
  INST(Vfnmsubsd        , Fma4               , V(660F3A,7F,_,0,x,_,_,_  ), 0                         , 75 , 0  , 293, 162), // #1065
  INST(Vfnmsubss        , Fma4               , V(660F3A,7E,_,0,x,_,_,_  ), 0                         , 75 , 0  , 294, 162), // #1066
  INST(Vfpclasspd       , VexRmi_Lx          , E(660F3A,66,_,x,_,1,4,FV ), 0                         , 111, 0  , 295, 152), // #1067
  INST(Vfpclassph       , VexRmi_Lx          , E(000F3A,66,_,_,_,0,4,FV ), 0                         , 122, 0  , 296, 145), // #1068
  INST(Vfpclassps       , VexRmi_Lx          , E(660F3A,66,_,x,_,0,4,FV ), 0                         , 110, 0  , 297, 152), // #1069
  INST(Vfpclasssd       , VexRmi             , E(660F3A,67,_,I,_,1,3,T1S), 0                         , 178, 0  , 298, 152), // #1070
  INST(Vfpclasssh       , VexRmi             , E(000F3A,67,_,_,_,0,1,T1S), 0                         , 186, 0  , 299, 145), // #1071
  INST(Vfpclassss       , VexRmi             , E(660F3A,67,_,I,_,0,2,T1S), 0                         , 179, 0  , 300, 152), // #1072
  INST(Vfrczpd          , VexRm_Lx           , V(XOP_M9,81,_,x,0,_,_,_  ), 0                         , 81 , 0  , 301, 163), // #1073
  INST(Vfrczps          , VexRm_Lx           , V(XOP_M9,80,_,x,0,_,_,_  ), 0                         , 81 , 0  , 301, 163), // #1074
  INST(Vfrczsd          , VexRm              , V(XOP_M9,83,_,0,0,_,_,_  ), 0                         , 81 , 0  , 302, 163), // #1075
  INST(Vfrczss          , VexRm              , V(XOP_M9,82,_,0,0,_,_,_  ), 0                         , 81 , 0  , 303, 163), // #1076
  INST(Vgatherdpd       , VexRmvRm_VM        , V(660F38,92,_,x,1,_,_,_  ), E(660F38,92,_,x,_,1,3,T1S), 187, 80 , 304, 164), // #1077
  INST(Vgatherdps       , VexRmvRm_VM        , V(660F38,92,_,x,0,_,_,_  ), E(660F38,92,_,x,_,0,2,T1S), 30 , 81 , 305, 164), // #1078
  INST(Vgatherqpd       , VexRmvRm_VM        , V(660F38,93,_,x,1,_,_,_  ), E(660F38,93,_,x,_,1,3,T1S), 187, 82 , 306, 164), // #1079
  INST(Vgatherqps       , VexRmvRm_VM        , V(660F38,93,_,x,0,_,_,_  ), E(660F38,93,_,x,_,0,2,T1S), 30 , 83 , 307, 164), // #1080
  INST(Vgetexppd        , VexRm_Lx           , E(660F38,42,_,x,_,1,4,FV ), 0                         , 112, 0  , 268, 149), // #1081
  INST(Vgetexpph        , VexRm_Lx           , E(66MAP6,42,_,_,_,0,4,FV ), 0                         , 181, 0  , 270, 145), // #1082
  INST(Vgetexpps        , VexRm_Lx           , E(660F38,42,_,x,_,0,4,FV ), 0                         , 113, 0  , 273, 149), // #1083
  INST(Vgetexpsd        , VexRvm             , E(660F38,43,_,I,_,1,3,T1S), 0                         , 127, 0  , 308, 149), // #1084
  INST(Vgetexpsh        , VexRvm             , E(66MAP6,43,_,_,_,0,1,T1S), 0                         , 183, 0  , 259, 145), // #1085
  INST(Vgetexpss        , VexRvm             , E(660F38,43,_,I,_,0,2,T1S), 0                         , 128, 0  , 309, 149), // #1086
  INST(Vgetmantpd       , VexRmi_Lx          , E(660F3A,26,_,x,_,1,4,FV ), 0                         , 111, 0  , 310, 149), // #1087
  INST(Vgetmantph       , VexRmi_Lx          , E(000F3A,26,_,_,_,0,4,FV ), 0                         , 122, 0  , 311, 145), // #1088
  INST(Vgetmantps       , VexRmi_Lx          , E(660F3A,26,_,x,_,0,4,FV ), 0                         , 110, 0  , 312, 149), // #1089
  INST(Vgetmantsd       , VexRvmi            , E(660F3A,27,_,I,_,1,3,T1S), 0                         , 178, 0  , 290, 149), // #1090
  INST(Vgetmantsh       , VexRvmi            , E(000F3A,27,_,_,_,0,1,T1S), 0                         , 186, 0  , 313, 145), // #1091
  INST(Vgetmantss       , VexRvmi            , E(660F3A,27,_,I,_,0,2,T1S), 0                         , 179, 0  , 291, 149), // #1092
  INST(Vgf2p8affineinvqb, VexRvmi_Lx         , V(660F3A,CF,_,x,1,1,4,FV ), 0                         , 188, 0  , 314, 165), // #1093
  INST(Vgf2p8affineqb   , VexRvmi_Lx         , V(660F3A,CE,_,x,1,1,4,FV ), 0                         , 188, 0  , 314, 165), // #1094
  INST(Vgf2p8mulb       , VexRvm_Lx          , V(660F38,CF,_,x,0,0,4,FV ), 0                         , 109, 0  , 315, 165), // #1095
  INST(Vhaddpd          , VexRvm_Lx          , V(660F00,7C,_,x,I,_,_,_  ), 0                         , 71 , 0  , 205, 146), // #1096
  INST(Vhaddps          , VexRvm_Lx          , V(F20F00,7C,_,x,I,_,_,_  ), 0                         , 108, 0  , 205, 146), // #1097
  INST(Vhsubpd          , VexRvm_Lx          , V(660F00,7D,_,x,I,_,_,_  ), 0                         , 71 , 0  , 205, 146), // #1098
  INST(Vhsubps          , VexRvm_Lx          , V(F20F00,7D,_,x,I,_,_,_  ), 0                         , 108, 0  , 205, 146), // #1099
  INST(Vinsertf128      , VexRvmi            , V(660F3A,18,_,1,0,_,_,_  ), 0                         , 170, 0  , 316, 146), // #1100
  INST(Vinsertf32x4     , VexRvmi_Lx         , E(660F3A,18,_,x,_,0,4,T4 ), 0                         , 171, 0  , 317, 149), // #1101
  INST(Vinsertf32x8     , VexRvmi            , E(660F3A,1A,_,2,_,0,5,T8 ), 0                         , 172, 0  , 318, 152), // #1102
  INST(Vinsertf64x2     , VexRvmi_Lx         , E(660F3A,18,_,x,_,1,4,T2 ), 0                         , 173, 0  , 317, 152), // #1103
  INST(Vinsertf64x4     , VexRvmi            , E(660F3A,1A,_,2,_,1,5,T4 ), 0                         , 174, 0  , 318, 149), // #1104
  INST(Vinserti128      , VexRvmi            , V(660F3A,38,_,1,0,_,_,_  ), 0                         , 170, 0  , 316, 153), // #1105
  INST(Vinserti32x4     , VexRvmi_Lx         , E(660F3A,38,_,x,_,0,4,T4 ), 0                         , 171, 0  , 317, 149), // #1106
  INST(Vinserti32x8     , VexRvmi            , E(660F3A,3A,_,2,_,0,5,T8 ), 0                         , 172, 0  , 318, 152), // #1107
  INST(Vinserti64x2     , VexRvmi_Lx         , E(660F3A,38,_,x,_,1,4,T2 ), 0                         , 173, 0  , 317, 152), // #1108
  INST(Vinserti64x4     , VexRvmi            , E(660F3A,3A,_,2,_,1,5,T4 ), 0                         , 174, 0  , 318, 149), // #1109
  INST(Vinsertps        , VexRvmi            , V(660F3A,21,_,0,I,0,2,T1S), 0                         , 175, 0  , 319, 144), // #1110
  INST(Vlddqu           , VexRm_Lx           , V(F20F00,F0,_,x,I,_,_,_  ), 0                         , 108, 0  , 240, 146), // #1111
  INST(Vldmxcsr         , VexM               , V(000F00,AE,2,0,I,_,_,_  ), 0                         , 189, 0  , 320, 146), // #1112
  INST(Vmaskmovdqu      , VexRm_ZDI          , V(660F00,F7,_,0,I,_,_,_  ), 0                         , 71 , 0  , 321, 146), // #1113
  INST(Vmaskmovpd       , VexRvmMvr_Lx       , V(660F38,2D,_,x,0,_,_,_  ), V(660F38,2F,_,x,0,_,_,_  ), 30 , 84 , 322, 146), // #1114
  INST(Vmaskmovps       , VexRvmMvr_Lx       , V(660F38,2C,_,x,0,_,_,_  ), V(660F38,2E,_,x,0,_,_,_  ), 30 , 85 , 322, 146), // #1115
  INST(Vmaxpd           , VexRvm_Lx          , V(660F00,5F,_,x,I,1,4,FV ), 0                         , 102, 0  , 323, 144), // #1116
  INST(Vmaxph           , VexRvm_Lx          , E(00MAP5,5F,_,_,_,0,4,FV ), 0                         , 103, 0  , 324, 145), // #1117
  INST(Vmaxps           , VexRvm_Lx          , V(000F00,5F,_,x,I,0,4,FV ), 0                         , 104, 0  , 325, 144), // #1118
  INST(Vmaxsd           , VexRvm             , V(F20F00,5F,_,I,I,1,3,T1S), 0                         , 105, 0  , 326, 144), // #1119
  INST(Vmaxsh           , VexRvm             , E(F3MAP5,5F,_,_,_,0,1,T1S), 0                         , 106, 0  , 259, 145), // #1120
  INST(Vmaxss           , VexRvm             , V(F30F00,5F,_,I,I,0,2,T1S), 0                         , 107, 0  , 263, 144), // #1121
  INST(Vmcall           , X86Op              , O(000F01,C1,_,_,_,_,_,_  ), 0                         , 23 , 0  , 31 , 67 ), // #1122
  INST(Vmclear          , X86M_Only          , O(660F00,C7,6,_,_,_,_,_  ), 0                         , 28 , 0  , 33 , 67 ), // #1123
  INST(Vmfunc           , X86Op              , O(000F01,D4,_,_,_,_,_,_  ), 0                         , 23 , 0  , 31 , 67 ), // #1124
  INST(Vmgexit          , X86Op              , O(F20F01,D9,_,_,_,_,_,_  ), 0                         , 93 , 0  , 31 , 166), // #1125
  INST(Vminpd           , VexRvm_Lx          , V(660F00,5D,_,x,I,1,4,FV ), 0                         , 102, 0  , 323, 144), // #1126
  INST(Vminph           , VexRvm_Lx          , E(00MAP5,5D,_,_,_,0,4,FV ), 0                         , 103, 0  , 324, 145), // #1127
  INST(Vminps           , VexRvm_Lx          , V(000F00,5D,_,x,I,0,4,FV ), 0                         , 104, 0  , 325, 144), // #1128
  INST(Vminsd           , VexRvm             , V(F20F00,5D,_,I,I,1,3,T1S), 0                         , 105, 0  , 326, 144), // #1129
  INST(Vminsh           , VexRvm             , E(F3MAP5,5D,_,_,_,0,1,T1S), 0                         , 106, 0  , 259, 145), // #1130
  INST(Vminss           , VexRvm             , V(F30F00,5D,_,I,I,0,2,T1S), 0                         , 107, 0  , 263, 144), // #1131
  INST(Vmlaunch         , X86Op              , O(000F01,C2,_,_,_,_,_,_  ), 0                         , 23 , 0  , 31 , 67 ), // #1132
  INST(Vmload           , X86Op_xAX          , O(000F01,DA,_,_,_,_,_,_  ), 0                         , 23 , 0  , 327, 23 ), // #1133
  INST(Vmmcall          , X86Op              , O(000F01,D9,_,_,_,_,_,_  ), 0                         , 23 , 0  , 31 , 23 ), // #1134
  INST(Vmovapd          , VexRmMr_Lx         , V(660F00,28,_,x,I,1,4,FVM), V(660F00,29,_,x,I,1,4,FVM), 102, 86 , 328, 167), // #1135
  INST(Vmovaps          , VexRmMr_Lx         , V(000F00,28,_,x,I,0,4,FVM), V(000F00,29,_,x,I,0,4,FVM), 104, 87 , 328, 167), // #1136
  INST(Vmovd            , VexMovdMovq        , V(660F00,6E,_,0,0,0,2,T1S), V(660F00,7E,_,0,0,0,2,T1S), 190, 88 , 329, 144), // #1137
  INST(Vmovddup         , VexRm_Lx           , V(F20F00,12,_,x,I,1,3,DUP), 0                         , 191, 0  , 330, 144), // #1138
  INST(Vmovdqa          , VexRmMr_Lx         , V(660F00,6F,_,x,I,_,_,_  ), V(660F00,7F,_,x,I,_,_,_  ), 71 , 89 , 331, 168), // #1139
  INST(Vmovdqa32        , VexRmMr_Lx         , E(660F00,6F,_,x,_,0,4,FVM), E(660F00,7F,_,x,_,0,4,FVM), 192, 90 , 332, 169), // #1140
  INST(Vmovdqa64        , VexRmMr_Lx         , E(660F00,6F,_,x,_,1,4,FVM), E(660F00,7F,_,x,_,1,4,FVM), 134, 91 , 332, 169), // #1141
  INST(Vmovdqu          , VexRmMr_Lx         , V(F30F00,6F,_,x,I,_,_,_  ), V(F30F00,7F,_,x,I,_,_,_  ), 193, 92 , 331, 168), // #1142
  INST(Vmovdqu16        , VexRmMr_Lx         , E(F20F00,6F,_,x,_,1,4,FVM), E(F20F00,7F,_,x,_,1,4,FVM), 165, 93 , 332, 170), // #1143
  INST(Vmovdqu32        , VexRmMr_Lx         , E(F30F00,6F,_,x,_,0,4,FVM), E(F30F00,7F,_,x,_,0,4,FVM), 194, 94 , 332, 169), // #1144
  INST(Vmovdqu64        , VexRmMr_Lx         , E(F30F00,6F,_,x,_,1,4,FVM), E(F30F00,7F,_,x,_,1,4,FVM), 148, 95 , 332, 169), // #1145
  INST(Vmovdqu8         , VexRmMr_Lx         , E(F20F00,6F,_,x,_,0,4,FVM), E(F20F00,7F,_,x,_,0,4,FVM), 163, 96 , 332, 170), // #1146
  INST(Vmovhlps         , VexRvm             , V(000F00,12,_,0,I,0,_,_  ), 0                         , 74 , 0  , 333, 144), // #1147
  INST(Vmovhpd          , VexRvmMr           , V(660F00,16,_,0,I,1,3,T1S), V(660F00,17,_,0,I,1,3,T1S), 124, 97 , 334, 144), // #1148
  INST(Vmovhps          , VexRvmMr           , V(000F00,16,_,0,I,0,3,T2 ), V(000F00,17,_,0,I,0,3,T2 ), 195, 98 , 334, 144), // #1149
  INST(Vmovlhps         , VexRvm             , V(000F00,16,_,0,I,0,_,_  ), 0                         , 74 , 0  , 333, 144), // #1150
  INST(Vmovlpd          , VexRvmMr           , V(660F00,12,_,0,I,1,3,T1S), V(660F00,13,_,0,I,1,3,T1S), 124, 99 , 334, 144), // #1151
  INST(Vmovlps          , VexRvmMr           , V(000F00,12,_,0,I,0,3,T2 ), V(000F00,13,_,0,I,0,3,T2 ), 195, 100, 334, 144), // #1152
  INST(Vmovmskpd        , VexRm_Lx           , V(660F00,50,_,x,I,_,_,_  ), 0                         , 71 , 0  , 335, 146), // #1153
  INST(Vmovmskps        , VexRm_Lx           , V(000F00,50,_,x,I,_,_,_  ), 0                         , 74 , 0  , 335, 146), // #1154
  INST(Vmovntdq         , VexMr_Lx           , V(660F00,E7,_,x,I,0,4,FVM), 0                         , 143, 0  , 336, 144), // #1155
  INST(Vmovntdqa        , VexRm_Lx           , V(660F38,2A,_,x,I,0,4,FVM), 0                         , 109, 0  , 337, 154), // #1156
  INST(Vmovntpd         , VexMr_Lx           , V(660F00,2B,_,x,I,1,4,FVM), 0                         , 102, 0  , 336, 144), // #1157
  INST(Vmovntps         , VexMr_Lx           , V(000F00,2B,_,x,I,0,4,FVM), 0                         , 104, 0  , 336, 144), // #1158
  INST(Vmovq            , VexMovdMovq        , V(660F00,6E,_,0,I,1,3,T1S), V(660F00,7E,_,0,I,1,3,T1S), 124, 101, 338, 167), // #1159
  INST(Vmovsd           , VexMovssMovsd      , V(F20F00,10,_,I,I,1,3,T1S), V(F20F00,11,_,I,I,1,3,T1S), 105, 102, 339, 167), // #1160
  INST(Vmovsh           , VexMovssMovsd      , E(F3MAP5,10,_,I,_,0,1,T1S), E(F3MAP5,11,_,I,_,0,1,T1S), 106, 103, 340, 145), // #1161
  INST(Vmovshdup        , VexRm_Lx           , V(F30F00,16,_,x,I,0,4,FVM), 0                         , 160, 0  , 341, 144), // #1162
  INST(Vmovsldup        , VexRm_Lx           , V(F30F00,12,_,x,I,0,4,FVM), 0                         , 160, 0  , 341, 144), // #1163
  INST(Vmovss           , VexMovssMovsd      , V(F30F00,10,_,I,I,0,2,T1S), V(F30F00,11,_,I,I,0,2,T1S), 107, 104, 342, 167), // #1164
  INST(Vmovupd          , VexRmMr_Lx         , V(660F00,10,_,x,I,1,4,FVM), V(660F00,11,_,x,I,1,4,FVM), 102, 105, 328, 167), // #1165
  INST(Vmovups          , VexRmMr_Lx         , V(000F00,10,_,x,I,0,4,FVM), V(000F00,11,_,x,I,0,4,FVM), 104, 106, 328, 167), // #1166
  INST(Vmovw            , VexMovdMovq        , E(66MAP5,6E,_,0,_,I,1,T1S), E(66MAP5,7E,_,0,_,I,1,T1S), 196, 107, 343, 145), // #1167
  INST(Vmpsadbw         , VexRvmi_Lx         , V(660F3A,42,_,x,I,_,_,_  ), 0                         , 75 , 0  , 218, 171), // #1168
  INST(Vmptrld          , X86M_Only          , O(000F00,C7,6,_,_,_,_,_  ), 0                         , 82 , 0  , 33 , 67 ), // #1169
  INST(Vmptrst          , X86M_Only          , O(000F00,C7,7,_,_,_,_,_  ), 0                         , 24 , 0  , 33 , 67 ), // #1170
  INST(Vmread           , X86Mr_NoSize       , O(000F00,78,_,_,_,_,_,_  ), 0                         , 5  , 0  , 344, 67 ), // #1171
  INST(Vmresume         , X86Op              , O(000F01,C3,_,_,_,_,_,_  ), 0                         , 23 , 0  , 31 , 67 ), // #1172
  INST(Vmrun            , X86Op_xAX          , O(000F01,D8,_,_,_,_,_,_  ), 0                         , 23 , 0  , 327, 23 ), // #1173
  INST(Vmsave           , X86Op_xAX          , O(000F01,DB,_,_,_,_,_,_  ), 0                         , 23 , 0  , 327, 23 ), // #1174
  INST(Vmulpd           , VexRvm_Lx          , V(660F00,59,_,x,I,1,4,FV ), 0                         , 102, 0  , 199, 144), // #1175
  INST(Vmulph           , VexRvm_Lx          , E(00MAP5,59,_,_,_,0,4,FV ), 0                         , 103, 0  , 200, 145), // #1176
  INST(Vmulps           , VexRvm_Lx          , V(000F00,59,_,x,I,0,4,FV ), 0                         , 104, 0  , 201, 144), // #1177
  INST(Vmulsd           , VexRvm             , V(F20F00,59,_,I,I,1,3,T1S), 0                         , 105, 0  , 202, 144), // #1178
  INST(Vmulsh           , VexRvm             , E(F3MAP5,59,_,_,_,0,1,T1S), 0                         , 106, 0  , 203, 145), // #1179
  INST(Vmulss           , VexRvm             , V(F30F00,59,_,I,I,0,2,T1S), 0                         , 107, 0  , 204, 144), // #1180
  INST(Vmwrite          , X86Rm_NoSize       , O(000F00,79,_,_,_,_,_,_  ), 0                         , 5  , 0  , 345, 67 ), // #1181
  INST(Vmxoff           , X86Op              , O(000F01,C4,_,_,_,_,_,_  ), 0                         , 23 , 0  , 31 , 67 ), // #1182
  INST(Vmxon            , X86M_Only          , O(F30F00,C7,6,_,_,_,_,_  ), 0                         , 26 , 0  , 33 , 67 ), // #1183
  INST(Vorpd            , VexRvm_Lx          , V(660F00,56,_,x,I,1,4,FV ), 0                         , 102, 0  , 213, 150), // #1184
  INST(Vorps            , VexRvm_Lx          , V(000F00,56,_,x,I,0,4,FV ), 0                         , 104, 0  , 214, 150), // #1185
  INST(Vp2intersectd    , VexRvm_Lx_2xK      , E(F20F38,68,_,_,_,0,4,FV ), 0                         , 130, 0  , 346, 172), // #1186
  INST(Vp2intersectq    , VexRvm_Lx_2xK      , E(F20F38,68,_,_,_,1,4,FV ), 0                         , 197, 0  , 347, 172), // #1187
  INST(Vpabsb           , VexRm_Lx           , V(660F38,1C,_,x,I,_,4,FVM), 0                         , 109, 0  , 341, 173), // #1188
  INST(Vpabsd           , VexRm_Lx           , V(660F38,1E,_,x,I,0,4,FV ), 0                         , 109, 0  , 348, 154), // #1189
  INST(Vpabsq           , VexRm_Lx           , E(660F38,1F,_,x,_,1,4,FV ), 0                         , 112, 0  , 349, 149), // #1190
  INST(Vpabsw           , VexRm_Lx           , V(660F38,1D,_,x,I,_,4,FVM), 0                         , 109, 0  , 341, 173), // #1191
  INST(Vpackssdw        , VexRvm_Lx          , V(660F00,6B,_,x,I,0,4,FV ), 0                         , 143, 0  , 212, 173), // #1192
  INST(Vpacksswb        , VexRvm_Lx          , V(660F00,63,_,x,I,I,4,FVM), 0                         , 143, 0  , 315, 173), // #1193
  INST(Vpackusdw        , VexRvm_Lx          , V(660F38,2B,_,x,I,0,4,FV ), 0                         , 109, 0  , 212, 173), // #1194
  INST(Vpackuswb        , VexRvm_Lx          , V(660F00,67,_,x,I,I,4,FVM), 0                         , 143, 0  , 315, 173), // #1195
  INST(Vpaddb           , VexRvm_Lx          , V(660F00,FC,_,x,I,I,4,FVM), 0                         , 143, 0  , 315, 173), // #1196
  INST(Vpaddd           , VexRvm_Lx          , V(660F00,FE,_,x,I,0,4,FV ), 0                         , 143, 0  , 212, 154), // #1197
  INST(Vpaddq           , VexRvm_Lx          , V(660F00,D4,_,x,I,1,4,FV ), 0                         , 102, 0  , 211, 154), // #1198
  INST(Vpaddsb          , VexRvm_Lx          , V(660F00,EC,_,x,I,I,4,FVM), 0                         , 143, 0  , 315, 173), // #1199
  INST(Vpaddsw          , VexRvm_Lx          , V(660F00,ED,_,x,I,I,4,FVM), 0                         , 143, 0  , 315, 173), // #1200
  INST(Vpaddusb         , VexRvm_Lx          , V(660F00,DC,_,x,I,I,4,FVM), 0                         , 143, 0  , 315, 173), // #1201
  INST(Vpaddusw         , VexRvm_Lx          , V(660F00,DD,_,x,I,I,4,FVM), 0                         , 143, 0  , 315, 173), // #1202
  INST(Vpaddw           , VexRvm_Lx          , V(660F00,FD,_,x,I,I,4,FVM), 0                         , 143, 0  , 315, 173), // #1203
  INST(Vpalignr         , VexRvmi_Lx         , V(660F3A,0F,_,x,I,I,4,FVM), 0                         , 198, 0  , 314, 173), // #1204
  INST(Vpand            , VexRvm_Lx          , V(660F00,DB,_,x,I,_,_,_  ), 0                         , 71 , 0  , 350, 171), // #1205
  INST(Vpandd           , VexRvm_Lx          , E(660F00,DB,_,x,_,0,4,FV ), 0                         , 192, 0  , 351, 149), // #1206
  INST(Vpandn           , VexRvm_Lx          , V(660F00,DF,_,x,I,_,_,_  ), 0                         , 71 , 0  , 352, 171), // #1207
  INST(Vpandnd          , VexRvm_Lx          , E(660F00,DF,_,x,_,0,4,FV ), 0                         , 192, 0  , 353, 149), // #1208
  INST(Vpandnq          , VexRvm_Lx          , E(660F00,DF,_,x,_,1,4,FV ), 0                         , 134, 0  , 354, 149), // #1209
  INST(Vpandq           , VexRvm_Lx          , E(660F00,DB,_,x,_,1,4,FV ), 0                         , 134, 0  , 355, 149), // #1210
  INST(Vpavgb           , VexRvm_Lx          , V(660F00,E0,_,x,I,I,4,FVM), 0                         , 143, 0  , 315, 173), // #1211
  INST(Vpavgw           , VexRvm_Lx          , V(660F00,E3,_,x,I,I,4,FVM), 0                         , 143, 0  , 315, 173), // #1212
  INST(Vpblendd         , VexRvmi_Lx         , V(660F3A,02,_,x,0,_,_,_  ), 0                         , 75 , 0  , 218, 153), // #1213
  INST(Vpblendmb        , VexRvm_Lx          , E(660F38,66,_,x,_,0,4,FVM), 0                         , 113, 0  , 356, 160), // #1214
  INST(Vpblendmd        , VexRvm_Lx          , E(660F38,64,_,x,_,0,4,FV ), 0                         , 113, 0  , 217, 149), // #1215
  INST(Vpblendmq        , VexRvm_Lx          , E(660F38,64,_,x,_,1,4,FV ), 0                         , 112, 0  , 216, 149), // #1216
  INST(Vpblendmw        , VexRvm_Lx          , E(660F38,66,_,x,_,1,4,FVM), 0                         , 112, 0  , 356, 160), // #1217
  INST(Vpblendvb        , VexRvmr_Lx         , V(660F3A,4C,_,x,0,_,_,_  ), 0                         , 75 , 0  , 219, 171), // #1218
  INST(Vpblendw         , VexRvmi_Lx         , V(660F3A,0E,_,x,I,_,_,_  ), 0                         , 75 , 0  , 218, 171), // #1219
  INST(Vpbroadcastb     , VexRm_Lx_Bcst      , V(660F38,78,_,x,0,0,0,T1S), E(660F38,7A,_,x,0,0,0,T1S), 30 , 108, 357, 174), // #1220
  INST(Vpbroadcastd     , VexRm_Lx_Bcst      , V(660F38,58,_,x,0,0,2,T1S), E(660F38,7C,_,x,0,0,0,T1S), 121, 109, 358, 164), // #1221
  INST(Vpbroadcastmb2q  , VexRm_Lx           , E(F30F38,2A,_,x,_,1,_,_  ), 0                         , 199, 0  , 359, 175), // #1222
  INST(Vpbroadcastmw2d  , VexRm_Lx           , E(F30F38,3A,_,x,_,0,_,_  ), 0                         , 200, 0  , 359, 175), // #1223
  INST(Vpbroadcastq     , VexRm_Lx_Bcst      , V(660F38,59,_,x,0,1,3,T1S), E(660F38,7C,_,x,0,1,0,T1S), 120, 110, 360, 164), // #1224
  INST(Vpbroadcastw     , VexRm_Lx_Bcst      , V(660F38,79,_,x,0,0,1,T1S), E(660F38,7B,_,x,0,0,0,T1S), 201, 111, 361, 174), // #1225
  INST(Vpclmulqdq       , VexRvmi_Lx         , V(660F3A,44,_,x,I,_,4,FVM), 0                         , 198, 0  , 362, 176), // #1226
  INST(Vpcmov           , VexRvrmRvmr_Lx     , V(XOP_M8,A2,_,x,x,_,_,_  ), 0                         , 202, 0  , 363, 163), // #1227
  INST(Vpcmpb           , VexRvmi_Lx         , E(660F3A,3F,_,x,_,0,4,FVM), 0                         , 110, 0  , 364, 160), // #1228
  INST(Vpcmpd           , VexRvmi_Lx         , E(660F3A,1F,_,x,_,0,4,FV ), 0                         , 110, 0  , 365, 149), // #1229
  INST(Vpcmpeqb         , VexRvm_Lx_KEvex    , V(660F00,74,_,x,I,I,4,FV ), 0                         , 143, 0  , 366, 173), // #1230
  INST(Vpcmpeqd         , VexRvm_Lx_KEvex    , V(660F00,76,_,x,I,0,4,FVM), 0                         , 143, 0  , 367, 154), // #1231
  INST(Vpcmpeqq         , VexRvm_Lx_KEvex    , V(660F38,29,_,x,I,1,4,FVM), 0                         , 203, 0  , 368, 154), // #1232
  INST(Vpcmpeqw         , VexRvm_Lx_KEvex    , V(660F00,75,_,x,I,I,4,FV ), 0                         , 143, 0  , 366, 173), // #1233
  INST(Vpcmpestri       , VexRmi             , V(660F3A,61,_,0,I,_,_,_  ), 0                         , 75 , 0  , 369, 177), // #1234
  INST(Vpcmpestrm       , VexRmi             , V(660F3A,60,_,0,I,_,_,_  ), 0                         , 75 , 0  , 370, 177), // #1235
  INST(Vpcmpgtb         , VexRvm_Lx_KEvex    , V(660F00,64,_,x,I,I,4,FV ), 0                         , 143, 0  , 366, 173), // #1236
  INST(Vpcmpgtd         , VexRvm_Lx_KEvex    , V(660F00,66,_,x,I,0,4,FVM), 0                         , 143, 0  , 367, 154), // #1237
  INST(Vpcmpgtq         , VexRvm_Lx_KEvex    , V(660F38,37,_,x,I,1,4,FVM), 0                         , 203, 0  , 368, 154), // #1238
  INST(Vpcmpgtw         , VexRvm_Lx_KEvex    , V(660F00,65,_,x,I,I,4,FV ), 0                         , 143, 0  , 366, 173), // #1239
  INST(Vpcmpistri       , VexRmi             , V(660F3A,63,_,0,I,_,_,_  ), 0                         , 75 , 0  , 371, 177), // #1240
  INST(Vpcmpistrm       , VexRmi             , V(660F3A,62,_,0,I,_,_,_  ), 0                         , 75 , 0  , 372, 177), // #1241
  INST(Vpcmpq           , VexRvmi_Lx         , E(660F3A,1F,_,x,_,1,4,FV ), 0                         , 111, 0  , 373, 149), // #1242
  INST(Vpcmpub          , VexRvmi_Lx         , E(660F3A,3E,_,x,_,0,4,FVM), 0                         , 110, 0  , 364, 160), // #1243
  INST(Vpcmpud          , VexRvmi_Lx         , E(660F3A,1E,_,x,_,0,4,FV ), 0                         , 110, 0  , 365, 149), // #1244
  INST(Vpcmpuq          , VexRvmi_Lx         , E(660F3A,1E,_,x,_,1,4,FV ), 0                         , 111, 0  , 373, 149), // #1245
  INST(Vpcmpuw          , VexRvmi_Lx         , E(660F3A,3E,_,x,_,1,4,FVM), 0                         , 111, 0  , 364, 160), // #1246
  INST(Vpcmpw           , VexRvmi_Lx         , E(660F3A,3F,_,x,_,1,4,FVM), 0                         , 111, 0  , 364, 160), // #1247
  INST(Vpcomb           , VexRvmi            , V(XOP_M8,CC,_,0,0,_,_,_  ), 0                         , 202, 0  , 281, 163), // #1248
  INST(Vpcomd           , VexRvmi            , V(XOP_M8,CE,_,0,0,_,_,_  ), 0                         , 202, 0  , 281, 163), // #1249
  INST(Vpcompressb      , VexMr_Lx           , E(660F38,63,_,x,_,0,0,T1S), 0                         , 204, 0  , 236, 178), // #1250
  INST(Vpcompressd      , VexMr_Lx           , E(660F38,8B,_,x,_,0,2,T1S), 0                         , 128, 0  , 236, 149), // #1251
  INST(Vpcompressq      , VexMr_Lx           , E(660F38,8B,_,x,_,1,3,T1S), 0                         , 127, 0  , 236, 149), // #1252
  INST(Vpcompressw      , VexMr_Lx           , E(660F38,63,_,x,_,1,1,T1S), 0                         , 205, 0  , 236, 178), // #1253
  INST(Vpcomq           , VexRvmi            , V(XOP_M8,CF,_,0,0,_,_,_  ), 0                         , 202, 0  , 281, 163), // #1254
  INST(Vpcomub          , VexRvmi            , V(XOP_M8,EC,_,0,0,_,_,_  ), 0                         , 202, 0  , 281, 163), // #1255
  INST(Vpcomud          , VexRvmi            , V(XOP_M8,EE,_,0,0,_,_,_  ), 0                         , 202, 0  , 281, 163), // #1256
  INST(Vpcomuq          , VexRvmi            , V(XOP_M8,EF,_,0,0,_,_,_  ), 0                         , 202, 0  , 281, 163), // #1257
  INST(Vpcomuw          , VexRvmi            , V(XOP_M8,ED,_,0,0,_,_,_  ), 0                         , 202, 0  , 281, 163), // #1258
  INST(Vpcomw           , VexRvmi            , V(XOP_M8,CD,_,0,0,_,_,_  ), 0                         , 202, 0  , 281, 163), // #1259
  INST(Vpconflictd      , VexRm_Lx           , E(660F38,C4,_,x,_,0,4,FV ), 0                         , 113, 0  , 374, 175), // #1260
  INST(Vpconflictq      , VexRm_Lx           , E(660F38,C4,_,x,_,1,4,FV ), 0                         , 112, 0  , 374, 175), // #1261
  INST(Vpdpbssd         , VexRvm_Lx          , V(F20F38,50,_,x,0,_,_,_  ), 0                         , 85 , 0  , 205, 179), // #1262
  INST(Vpdpbssds        , VexRvm_Lx          , V(F20F38,51,_,x,0,_,_,_  ), 0                         , 85 , 0  , 205, 179), // #1263
  INST(Vpdpbsud         , VexRvm_Lx          , V(F30F38,50,_,x,0,_,_,_  ), 0                         , 89 , 0  , 205, 179), // #1264
  INST(Vpdpbsuds        , VexRvm_Lx          , V(F30F38,51,_,x,0,_,_,_  ), 0                         , 89 , 0  , 205, 179), // #1265
  INST(Vpdpbusd         , VexRvm_Lx          , V(660F38,50,_,x,_,0,4,FV ), 0                         , 109, 0  , 375, 180), // #1266
  INST(Vpdpbusds        , VexRvm_Lx          , V(660F38,51,_,x,_,0,4,FV ), 0                         , 109, 0  , 375, 180), // #1267
  INST(Vpdpbuud         , VexRvm_Lx          , V(000F38,50,_,x,0,_,_,_  ), 0                         , 11 , 0  , 205, 179), // #1268
  INST(Vpdpbuuds        , VexRvm_Lx          , V(000F38,51,_,x,0,_,_,_  ), 0                         , 11 , 0  , 205, 179), // #1269
  INST(Vpdpwssd         , VexRvm_Lx          , V(660F38,52,_,x,_,0,4,FV ), 0                         , 109, 0  , 375, 180), // #1270
  INST(Vpdpwssds        , VexRvm_Lx          , V(660F38,53,_,x,_,0,4,FV ), 0                         , 109, 0  , 375, 180), // #1271
  INST(Vpdpwsud         , VexRvm_Lx          , V(F30F38,D2,_,x,0,_,_,_  ), 0                         , 89 , 0  , 205, 181), // #1272
  INST(Vpdpwsuds        , VexRvm_Lx          , V(F30F38,D3,_,x,0,_,_,_  ), 0                         , 89 , 0  , 205, 181), // #1273
  INST(Vpdpwusd         , VexRvm_Lx          , V(660F38,D2,_,x,0,_,_,_  ), 0                         , 30 , 0  , 205, 181), // #1274
  INST(Vpdpwusds        , VexRvm_Lx          , V(660F38,D3,_,x,0,_,_,_  ), 0                         , 30 , 0  , 205, 181), // #1275
  INST(Vpdpwuud         , VexRvm_Lx          , V(000F38,D2,_,x,0,_,_,_  ), 0                         , 11 , 0  , 205, 181), // #1276
  INST(Vpdpwuuds        , VexRvm_Lx          , V(000F38,D3,_,x,0,_,_,_  ), 0                         , 11 , 0  , 205, 181), // #1277
  INST(Vperm2f128       , VexRvmi            , V(660F3A,06,_,1,0,_,_,_  ), 0                         , 170, 0  , 376, 146), // #1278
  INST(Vperm2i128       , VexRvmi            , V(660F3A,46,_,1,0,_,_,_  ), 0                         , 170, 0  , 376, 153), // #1279
  INST(Vpermb           , VexRvm_Lx          , E(660F38,8D,_,x,_,0,4,FVM), 0                         , 113, 0  , 356, 182), // #1280
  INST(Vpermd           , VexRvm_Lx          , V(660F38,36,_,x,0,0,4,FV ), 0                         , 109, 0  , 377, 164), // #1281
  INST(Vpermi2b         , VexRvm_Lx          , E(660F38,75,_,x,_,0,4,FVM), 0                         , 113, 0  , 356, 182), // #1282
  INST(Vpermi2d         , VexRvm_Lx          , E(660F38,76,_,x,_,0,4,FV ), 0                         , 113, 0  , 217, 149), // #1283
  INST(Vpermi2pd        , VexRvm_Lx          , E(660F38,77,_,x,_,1,4,FV ), 0                         , 112, 0  , 216, 149), // #1284
  INST(Vpermi2ps        , VexRvm_Lx          , E(660F38,77,_,x,_,0,4,FV ), 0                         , 113, 0  , 217, 149), // #1285
  INST(Vpermi2q         , VexRvm_Lx          , E(660F38,76,_,x,_,1,4,FV ), 0                         , 112, 0  , 216, 149), // #1286
  INST(Vpermi2w         , VexRvm_Lx          , E(660F38,75,_,x,_,1,4,FVM), 0                         , 112, 0  , 356, 160), // #1287
  INST(Vpermil2pd       , VexRvrmiRvmri_Lx   , V(660F3A,49,_,x,x,_,_,_  ), 0                         , 75 , 0  , 378, 163), // #1288
  INST(Vpermil2ps       , VexRvrmiRvmri_Lx   , V(660F3A,48,_,x,x,_,_,_  ), 0                         , 75 , 0  , 378, 163), // #1289
  INST(Vpermilpd        , VexRvmRmi_Lx       , V(660F38,0D,_,x,0,1,4,FV ), V(660F3A,05,_,x,0,1,4,FV ), 203, 112, 379, 144), // #1290
  INST(Vpermilps        , VexRvmRmi_Lx       , V(660F38,0C,_,x,0,0,4,FV ), V(660F3A,04,_,x,0,0,4,FV ), 109, 113, 380, 144), // #1291
  INST(Vpermpd          , VexRvmRmi_Lx       , E(660F38,16,_,x,1,1,4,FV ), V(660F3A,01,_,x,1,1,4,FV ), 206, 114, 381, 164), // #1292
  INST(Vpermps          , VexRvm_Lx          , V(660F38,16,_,x,0,0,4,FV ), 0                         , 109, 0  , 377, 164), // #1293
  INST(Vpermq           , VexRvmRmi_Lx       , E(660F38,36,_,x,_,1,4,FV ), V(660F3A,00,_,x,1,1,4,FV ), 112, 115, 381, 164), // #1294
  INST(Vpermt2b         , VexRvm_Lx          , E(660F38,7D,_,x,_,0,4,FVM), 0                         , 113, 0  , 356, 182), // #1295
  INST(Vpermt2d         , VexRvm_Lx          , E(660F38,7E,_,x,_,0,4,FV ), 0                         , 113, 0  , 217, 149), // #1296
  INST(Vpermt2pd        , VexRvm_Lx          , E(660F38,7F,_,x,_,1,4,FV ), 0                         , 112, 0  , 216, 149), // #1297
  INST(Vpermt2ps        , VexRvm_Lx          , E(660F38,7F,_,x,_,0,4,FV ), 0                         , 113, 0  , 217, 149), // #1298
  INST(Vpermt2q         , VexRvm_Lx          , E(660F38,7E,_,x,_,1,4,FV ), 0                         , 112, 0  , 216, 149), // #1299
  INST(Vpermt2w         , VexRvm_Lx          , E(660F38,7D,_,x,_,1,4,FVM), 0                         , 112, 0  , 356, 160), // #1300
  INST(Vpermw           , VexRvm_Lx          , E(660F38,8D,_,x,_,1,4,FVM), 0                         , 112, 0  , 356, 160), // #1301
  INST(Vpexpandb        , VexRm_Lx           , E(660F38,62,_,x,_,0,0,T1S), 0                         , 204, 0  , 282, 178), // #1302
  INST(Vpexpandd        , VexRm_Lx           , E(660F38,89,_,x,_,0,2,T1S), 0                         , 128, 0  , 282, 149), // #1303
  INST(Vpexpandq        , VexRm_Lx           , E(660F38,89,_,x,_,1,3,T1S), 0                         , 127, 0  , 282, 149), // #1304
  INST(Vpexpandw        , VexRm_Lx           , E(660F38,62,_,x,_,1,1,T1S), 0                         , 205, 0  , 282, 178), // #1305
  INST(Vpextrb          , VexMri             , V(660F3A,14,_,0,0,I,0,T1S), 0                         , 75 , 0  , 382, 183), // #1306
  INST(Vpextrd          , VexMri             , V(660F3A,16,_,0,0,0,2,T1S), 0                         , 175, 0  , 286, 150), // #1307
  INST(Vpextrq          , VexMri             , V(660F3A,16,_,0,1,1,3,T1S), 0                         , 207, 0  , 383, 150), // #1308
  INST(Vpextrw          , VexMri_Vpextrw     , V(660F3A,15,_,0,0,I,1,T1S), 0                         , 208, 0  , 384, 183), // #1309
  INST(Vpgatherdd       , VexRmvRm_VM        , V(660F38,90,_,x,0,_,_,_  ), E(660F38,90,_,x,_,0,2,T1S), 30 , 116, 305, 164), // #1310
  INST(Vpgatherdq       , VexRmvRm_VM        , V(660F38,90,_,x,1,_,_,_  ), E(660F38,90,_,x,_,1,3,T1S), 187, 117, 304, 164), // #1311
  INST(Vpgatherqd       , VexRmvRm_VM        , V(660F38,91,_,x,0,_,_,_  ), E(660F38,91,_,x,_,0,2,T1S), 30 , 118, 307, 164), // #1312
  INST(Vpgatherqq       , VexRmvRm_VM        , V(660F38,91,_,x,1,_,_,_  ), E(660F38,91,_,x,_,1,3,T1S), 187, 119, 306, 164), // #1313
  INST(Vphaddbd         , VexRm              , V(XOP_M9,C2,_,0,0,_,_,_  ), 0                         , 81 , 0  , 207, 163), // #1314
  INST(Vphaddbq         , VexRm              , V(XOP_M9,C3,_,0,0,_,_,_  ), 0                         , 81 , 0  , 207, 163), // #1315
  INST(Vphaddbw         , VexRm              , V(XOP_M9,C1,_,0,0,_,_,_  ), 0                         , 81 , 0  , 207, 163), // #1316
  INST(Vphaddd          , VexRvm_Lx          , V(660F38,02,_,x,I,_,_,_  ), 0                         , 30 , 0  , 205, 171), // #1317
  INST(Vphadddq         , VexRm              , V(XOP_M9,CB,_,0,0,_,_,_  ), 0                         , 81 , 0  , 207, 163), // #1318
  INST(Vphaddsw         , VexRvm_Lx          , V(660F38,03,_,x,I,_,_,_  ), 0                         , 30 , 0  , 205, 171), // #1319
  INST(Vphaddubd        , VexRm              , V(XOP_M9,D2,_,0,0,_,_,_  ), 0                         , 81 , 0  , 207, 163), // #1320
  INST(Vphaddubq        , VexRm              , V(XOP_M9,D3,_,0,0,_,_,_  ), 0                         , 81 , 0  , 207, 163), // #1321
  INST(Vphaddubw        , VexRm              , V(XOP_M9,D1,_,0,0,_,_,_  ), 0                         , 81 , 0  , 207, 163), // #1322
  INST(Vphaddudq        , VexRm              , V(XOP_M9,DB,_,0,0,_,_,_  ), 0                         , 81 , 0  , 207, 163), // #1323
  INST(Vphadduwd        , VexRm              , V(XOP_M9,D6,_,0,0,_,_,_  ), 0                         , 81 , 0  , 207, 163), // #1324
  INST(Vphadduwq        , VexRm              , V(XOP_M9,D7,_,0,0,_,_,_  ), 0                         , 81 , 0  , 207, 163), // #1325
  INST(Vphaddw          , VexRvm_Lx          , V(660F38,01,_,x,I,_,_,_  ), 0                         , 30 , 0  , 205, 171), // #1326
  INST(Vphaddwd         , VexRm              , V(XOP_M9,C6,_,0,0,_,_,_  ), 0                         , 81 , 0  , 207, 163), // #1327
  INST(Vphaddwq         , VexRm              , V(XOP_M9,C7,_,0,0,_,_,_  ), 0                         , 81 , 0  , 207, 163), // #1328
  INST(Vphminposuw      , VexRm              , V(660F38,41,_,0,I,_,_,_  ), 0                         , 30 , 0  , 207, 146), // #1329
  INST(Vphsubbw         , VexRm              , V(XOP_M9,E1,_,0,0,_,_,_  ), 0                         , 81 , 0  , 207, 163), // #1330
  INST(Vphsubd          , VexRvm_Lx          , V(660F38,06,_,x,I,_,_,_  ), 0                         , 30 , 0  , 205, 171), // #1331
  INST(Vphsubdq         , VexRm              , V(XOP_M9,E3,_,0,0,_,_,_  ), 0                         , 81 , 0  , 207, 163), // #1332
  INST(Vphsubsw         , VexRvm_Lx          , V(660F38,07,_,x,I,_,_,_  ), 0                         , 30 , 0  , 205, 171), // #1333
  INST(Vphsubw          , VexRvm_Lx          , V(660F38,05,_,x,I,_,_,_  ), 0                         , 30 , 0  , 205, 171), // #1334
  INST(Vphsubwd         , VexRm              , V(XOP_M9,E2,_,0,0,_,_,_  ), 0                         , 81 , 0  , 207, 163), // #1335
  INST(Vpinsrb          , VexRvmi            , V(660F3A,20,_,0,0,I,0,T1S), 0                         , 75 , 0  , 385, 183), // #1336
  INST(Vpinsrd          , VexRvmi            , V(660F3A,22,_,0,0,0,2,T1S), 0                         , 175, 0  , 386, 150), // #1337
  INST(Vpinsrq          , VexRvmi            , V(660F3A,22,_,0,1,1,3,T1S), 0                         , 207, 0  , 387, 150), // #1338
  INST(Vpinsrw          , VexRvmi            , V(660F00,C4,_,0,0,I,1,T1S), 0                         , 209, 0  , 388, 183), // #1339
  INST(Vplzcntd         , VexRm_Lx           , E(660F38,44,_,x,_,0,4,FV ), 0                         , 113, 0  , 374, 175), // #1340
  INST(Vplzcntq         , VexRm_Lx           , E(660F38,44,_,x,_,1,4,FV ), 0                         , 112, 0  , 349, 175), // #1341
  INST(Vpmacsdd         , VexRvmr            , V(XOP_M8,9E,_,0,0,_,_,_  ), 0                         , 202, 0  , 389, 163), // #1342
  INST(Vpmacsdqh        , VexRvmr            , V(XOP_M8,9F,_,0,0,_,_,_  ), 0                         , 202, 0  , 389, 163), // #1343
  INST(Vpmacsdql        , VexRvmr            , V(XOP_M8,97,_,0,0,_,_,_  ), 0                         , 202, 0  , 389, 163), // #1344
  INST(Vpmacssdd        , VexRvmr            , V(XOP_M8,8E,_,0,0,_,_,_  ), 0                         , 202, 0  , 389, 163), // #1345
  INST(Vpmacssdqh       , VexRvmr            , V(XOP_M8,8F,_,0,0,_,_,_  ), 0                         , 202, 0  , 389, 163), // #1346
  INST(Vpmacssdql       , VexRvmr            , V(XOP_M8,87,_,0,0,_,_,_  ), 0                         , 202, 0  , 389, 163), // #1347
  INST(Vpmacsswd        , VexRvmr            , V(XOP_M8,86,_,0,0,_,_,_  ), 0                         , 202, 0  , 389, 163), // #1348
  INST(Vpmacssww        , VexRvmr            , V(XOP_M8,85,_,0,0,_,_,_  ), 0                         , 202, 0  , 389, 163), // #1349
  INST(Vpmacswd         , VexRvmr            , V(XOP_M8,96,_,0,0,_,_,_  ), 0                         , 202, 0  , 389, 163), // #1350
  INST(Vpmacsww         , VexRvmr            , V(XOP_M8,95,_,0,0,_,_,_  ), 0                         , 202, 0  , 389, 163), // #1351
  INST(Vpmadcsswd       , VexRvmr            , V(XOP_M8,A6,_,0,0,_,_,_  ), 0                         , 202, 0  , 389, 163), // #1352
  INST(Vpmadcswd        , VexRvmr            , V(XOP_M8,B6,_,0,0,_,_,_  ), 0                         , 202, 0  , 389, 163), // #1353
  INST(Vpmadd52huq      , VexRvm_Lx          , V(660F38,B5,_,x,1,1,4,FV ), 0                         , 180, 0  , 390, 184), // #1354
  INST(Vpmadd52luq      , VexRvm_Lx          , V(660F38,B4,_,x,1,1,4,FV ), 0                         , 180, 0  , 390, 184), // #1355
  INST(Vpmaddubsw       , VexRvm_Lx          , V(660F38,04,_,x,I,I,4,FVM), 0                         , 109, 0  , 315, 173), // #1356
  INST(Vpmaddwd         , VexRvm_Lx          , V(660F00,F5,_,x,I,I,4,FVM), 0                         , 143, 0  , 315, 173), // #1357
  INST(Vpmaskmovd       , VexRvmMvr_Lx       , V(660F38,8C,_,x,0,_,_,_  ), V(660F38,8E,_,x,0,_,_,_  ), 30 , 120, 322, 153), // #1358
  INST(Vpmaskmovq       , VexRvmMvr_Lx       , V(660F38,8C,_,x,1,_,_,_  ), V(660F38,8E,_,x,1,_,_,_  ), 187, 121, 322, 153), // #1359
  INST(Vpmaxsb          , VexRvm_Lx          , V(660F38,3C,_,x,I,I,4,FVM), 0                         , 109, 0  , 391, 173), // #1360
  INST(Vpmaxsd          , VexRvm_Lx          , V(660F38,3D,_,x,I,0,4,FV ), 0                         , 109, 0  , 214, 154), // #1361
  INST(Vpmaxsq          , VexRvm_Lx          , E(660F38,3D,_,x,_,1,4,FV ), 0                         , 112, 0  , 216, 149), // #1362
  INST(Vpmaxsw          , VexRvm_Lx          , V(660F00,EE,_,x,I,I,4,FVM), 0                         , 143, 0  , 391, 173), // #1363
  INST(Vpmaxub          , VexRvm_Lx          , V(660F00,DE,_,x,I,I,4,FVM), 0                         , 143, 0  , 391, 173), // #1364
  INST(Vpmaxud          , VexRvm_Lx          , V(660F38,3F,_,x,I,0,4,FV ), 0                         , 109, 0  , 214, 154), // #1365
  INST(Vpmaxuq          , VexRvm_Lx          , E(660F38,3F,_,x,_,1,4,FV ), 0                         , 112, 0  , 216, 149), // #1366
  INST(Vpmaxuw          , VexRvm_Lx          , V(660F38,3E,_,x,I,I,4,FVM), 0                         , 109, 0  , 391, 173), // #1367
  INST(Vpminsb          , VexRvm_Lx          , V(660F38,38,_,x,I,I,4,FVM), 0                         , 109, 0  , 391, 173), // #1368
  INST(Vpminsd          , VexRvm_Lx          , V(660F38,39,_,x,I,0,4,FV ), 0                         , 109, 0  , 214, 154), // #1369
  INST(Vpminsq          , VexRvm_Lx          , E(660F38,39,_,x,_,1,4,FV ), 0                         , 112, 0  , 216, 149), // #1370
  INST(Vpminsw          , VexRvm_Lx          , V(660F00,EA,_,x,I,I,4,FVM), 0                         , 143, 0  , 391, 173), // #1371
  INST(Vpminub          , VexRvm_Lx          , V(660F00,DA,_,x,I,_,4,FVM), 0                         , 143, 0  , 391, 173), // #1372
  INST(Vpminud          , VexRvm_Lx          , V(660F38,3B,_,x,I,0,4,FV ), 0                         , 109, 0  , 214, 154), // #1373
  INST(Vpminuq          , VexRvm_Lx          , E(660F38,3B,_,x,_,1,4,FV ), 0                         , 112, 0  , 216, 149), // #1374
  INST(Vpminuw          , VexRvm_Lx          , V(660F38,3A,_,x,I,_,4,FVM), 0                         , 109, 0  , 391, 173), // #1375
  INST(Vpmovb2m         , VexRm_Lx           , E(F30F38,29,_,x,_,0,_,_  ), 0                         , 200, 0  , 392, 160), // #1376
  INST(Vpmovd2m         , VexRm_Lx           , E(F30F38,39,_,x,_,0,_,_  ), 0                         , 200, 0  , 392, 152), // #1377
  INST(Vpmovdb          , VexMr_Lx           , E(F30F38,31,_,x,_,0,2,QVM), 0                         , 210, 0  , 393, 149), // #1378
  INST(Vpmovdw          , VexMr_Lx           , E(F30F38,33,_,x,_,0,3,HVM), 0                         , 211, 0  , 394, 149), // #1379
  INST(Vpmovm2b         , VexRm_Lx           , E(F30F38,28,_,x,_,0,_,_  ), 0                         , 200, 0  , 359, 160), // #1380
  INST(Vpmovm2d         , VexRm_Lx           , E(F30F38,38,_,x,_,0,_,_  ), 0                         , 200, 0  , 359, 152), // #1381
  INST(Vpmovm2q         , VexRm_Lx           , E(F30F38,38,_,x,_,1,_,_  ), 0                         , 199, 0  , 359, 152), // #1382
  INST(Vpmovm2w         , VexRm_Lx           , E(F30F38,28,_,x,_,1,_,_  ), 0                         , 199, 0  , 359, 160), // #1383
  INST(Vpmovmskb        , VexRm_Lx           , V(660F00,D7,_,x,I,_,_,_  ), 0                         , 71 , 0  , 335, 171), // #1384
  INST(Vpmovq2m         , VexRm_Lx           , E(F30F38,39,_,x,_,1,_,_  ), 0                         , 199, 0  , 392, 152), // #1385
  INST(Vpmovqb          , VexMr_Lx           , E(F30F38,32,_,x,_,0,1,OVM), 0                         , 212, 0  , 395, 149), // #1386
  INST(Vpmovqd          , VexMr_Lx           , E(F30F38,35,_,x,_,0,3,HVM), 0                         , 211, 0  , 394, 149), // #1387
  INST(Vpmovqw          , VexMr_Lx           , E(F30F38,34,_,x,_,0,2,QVM), 0                         , 210, 0  , 393, 149), // #1388
  INST(Vpmovsdb         , VexMr_Lx           , E(F30F38,21,_,x,_,0,2,QVM), 0                         , 210, 0  , 393, 149), // #1389
  INST(Vpmovsdw         , VexMr_Lx           , E(F30F38,23,_,x,_,0,3,HVM), 0                         , 211, 0  , 394, 149), // #1390
  INST(Vpmovsqb         , VexMr_Lx           , E(F30F38,22,_,x,_,0,1,OVM), 0                         , 212, 0  , 395, 149), // #1391
  INST(Vpmovsqd         , VexMr_Lx           , E(F30F38,25,_,x,_,0,3,HVM), 0                         , 211, 0  , 394, 149), // #1392
  INST(Vpmovsqw         , VexMr_Lx           , E(F30F38,24,_,x,_,0,2,QVM), 0                         , 210, 0  , 393, 149), // #1393
  INST(Vpmovswb         , VexMr_Lx           , E(F30F38,20,_,x,_,0,3,HVM), 0                         , 211, 0  , 394, 160), // #1394
  INST(Vpmovsxbd        , VexRm_Lx           , V(660F38,21,_,x,I,I,2,QVM), 0                         , 213, 0  , 396, 154), // #1395
  INST(Vpmovsxbq        , VexRm_Lx           , V(660F38,22,_,x,I,I,1,OVM), 0                         , 214, 0  , 397, 154), // #1396
  INST(Vpmovsxbw        , VexRm_Lx           , V(660F38,20,_,x,I,I,3,HVM), 0                         , 138, 0  , 398, 173), // #1397
  INST(Vpmovsxdq        , VexRm_Lx           , V(660F38,25,_,x,I,0,3,HVM), 0                         , 138, 0  , 398, 154), // #1398
  INST(Vpmovsxwd        , VexRm_Lx           , V(660F38,23,_,x,I,I,3,HVM), 0                         , 138, 0  , 398, 154), // #1399
  INST(Vpmovsxwq        , VexRm_Lx           , V(660F38,24,_,x,I,I,2,QVM), 0                         , 213, 0  , 396, 154), // #1400
  INST(Vpmovusdb        , VexMr_Lx           , E(F30F38,11,_,x,_,0,2,QVM), 0                         , 210, 0  , 393, 149), // #1401
  INST(Vpmovusdw        , VexMr_Lx           , E(F30F38,13,_,x,_,0,3,HVM), 0                         , 211, 0  , 394, 149), // #1402
  INST(Vpmovusqb        , VexMr_Lx           , E(F30F38,12,_,x,_,0,1,OVM), 0                         , 212, 0  , 395, 149), // #1403
  INST(Vpmovusqd        , VexMr_Lx           , E(F30F38,15,_,x,_,0,3,HVM), 0                         , 211, 0  , 394, 149), // #1404
  INST(Vpmovusqw        , VexMr_Lx           , E(F30F38,14,_,x,_,0,2,QVM), 0                         , 210, 0  , 393, 149), // #1405
  INST(Vpmovuswb        , VexMr_Lx           , E(F30F38,10,_,x,_,0,3,HVM), 0                         , 211, 0  , 394, 160), // #1406
  INST(Vpmovw2m         , VexRm_Lx           , E(F30F38,29,_,x,_,1,_,_  ), 0                         , 199, 0  , 392, 160), // #1407
  INST(Vpmovwb          , VexMr_Lx           , E(F30F38,30,_,x,_,0,3,HVM), 0                         , 211, 0  , 394, 160), // #1408
  INST(Vpmovzxbd        , VexRm_Lx           , V(660F38,31,_,x,I,I,2,QVM), 0                         , 213, 0  , 396, 154), // #1409
  INST(Vpmovzxbq        , VexRm_Lx           , V(660F38,32,_,x,I,I,1,OVM), 0                         , 214, 0  , 397, 154), // #1410
  INST(Vpmovzxbw        , VexRm_Lx           , V(660F38,30,_,x,I,I,3,HVM), 0                         , 138, 0  , 398, 173), // #1411
  INST(Vpmovzxdq        , VexRm_Lx           , V(660F38,35,_,x,I,0,3,HVM), 0                         , 138, 0  , 398, 154), // #1412
  INST(Vpmovzxwd        , VexRm_Lx           , V(660F38,33,_,x,I,I,3,HVM), 0                         , 138, 0  , 398, 154), // #1413
  INST(Vpmovzxwq        , VexRm_Lx           , V(660F38,34,_,x,I,I,2,QVM), 0                         , 213, 0  , 396, 154), // #1414
  INST(Vpmuldq          , VexRvm_Lx          , V(660F38,28,_,x,I,1,4,FV ), 0                         , 203, 0  , 211, 154), // #1415
  INST(Vpmulhrsw        , VexRvm_Lx          , V(660F38,0B,_,x,I,I,4,FVM), 0                         , 109, 0  , 315, 173), // #1416
  INST(Vpmulhuw         , VexRvm_Lx          , V(660F00,E4,_,x,I,I,4,FVM), 0                         , 143, 0  , 315, 173), // #1417
  INST(Vpmulhw          , VexRvm_Lx          , V(660F00,E5,_,x,I,I,4,FVM), 0                         , 143, 0  , 315, 173), // #1418
  INST(Vpmulld          , VexRvm_Lx          , V(660F38,40,_,x,I,0,4,FV ), 0                         , 109, 0  , 212, 154), // #1419
  INST(Vpmullq          , VexRvm_Lx          , E(660F38,40,_,x,_,1,4,FV ), 0                         , 112, 0  , 216, 152), // #1420
  INST(Vpmullw          , VexRvm_Lx          , V(660F00,D5,_,x,I,I,4,FVM), 0                         , 143, 0  , 315, 173), // #1421
  INST(Vpmultishiftqb   , VexRvm_Lx          , E(660F38,83,_,x,_,1,4,FV ), 0                         , 112, 0  , 216, 182), // #1422
  INST(Vpmuludq         , VexRvm_Lx          , V(660F00,F4,_,x,I,1,4,FV ), 0                         , 102, 0  , 211, 154), // #1423
  INST(Vpopcntb         , VexRm_Lx           , E(660F38,54,_,x,_,0,4,FV ), 0                         , 113, 0  , 282, 185), // #1424
  INST(Vpopcntd         , VexRm_Lx           , E(660F38,55,_,x,_,0,4,FVM), 0                         , 113, 0  , 374, 186), // #1425
  INST(Vpopcntq         , VexRm_Lx           , E(660F38,55,_,x,_,1,4,FVM), 0                         , 112, 0  , 349, 186), // #1426
  INST(Vpopcntw         , VexRm_Lx           , E(660F38,54,_,x,_,1,4,FV ), 0                         , 112, 0  , 282, 185), // #1427
  INST(Vpor             , VexRvm_Lx          , V(660F00,EB,_,x,I,_,_,_  ), 0                         , 71 , 0  , 350, 171), // #1428
  INST(Vpord            , VexRvm_Lx          , E(660F00,EB,_,x,_,0,4,FV ), 0                         , 192, 0  , 351, 149), // #1429
  INST(Vporq            , VexRvm_Lx          , E(660F00,EB,_,x,_,1,4,FV ), 0                         , 134, 0  , 355, 149), // #1430
  INST(Vpperm           , VexRvrmRvmr        , V(XOP_M8,A3,_,0,x,_,_,_  ), 0                         , 202, 0  , 399, 163), // #1431
  INST(Vprold           , VexVmi_Lx          , E(660F00,72,1,x,_,0,4,FV ), 0                         , 215, 0  , 400, 149), // #1432
  INST(Vprolq           , VexVmi_Lx          , E(660F00,72,1,x,_,1,4,FV ), 0                         , 216, 0  , 401, 149), // #1433
  INST(Vprolvd          , VexRvm_Lx          , E(660F38,15,_,x,_,0,4,FV ), 0                         , 113, 0  , 217, 149), // #1434
  INST(Vprolvq          , VexRvm_Lx          , E(660F38,15,_,x,_,1,4,FV ), 0                         , 112, 0  , 216, 149), // #1435
  INST(Vprord           , VexVmi_Lx          , E(660F00,72,0,x,_,0,4,FV ), 0                         , 192, 0  , 400, 149), // #1436
  INST(Vprorq           , VexVmi_Lx          , E(660F00,72,0,x,_,1,4,FV ), 0                         , 134, 0  , 401, 149), // #1437
  INST(Vprorvd          , VexRvm_Lx          , E(660F38,14,_,x,_,0,4,FV ), 0                         , 113, 0  , 217, 149), // #1438
  INST(Vprorvq          , VexRvm_Lx          , E(660F38,14,_,x,_,1,4,FV ), 0                         , 112, 0  , 216, 149), // #1439
  INST(Vprotb           , VexRvmRmvRmi       , V(XOP_M9,90,_,0,x,_,_,_  ), V(XOP_M8,C0,_,0,x,_,_,_  ), 81 , 122, 402, 163), // #1440
  INST(Vprotd           , VexRvmRmvRmi       , V(XOP_M9,92,_,0,x,_,_,_  ), V(XOP_M8,C2,_,0,x,_,_,_  ), 81 , 123, 402, 163), // #1441
  INST(Vprotq           , VexRvmRmvRmi       , V(XOP_M9,93,_,0,x,_,_,_  ), V(XOP_M8,C3,_,0,x,_,_,_  ), 81 , 124, 402, 163), // #1442
  INST(Vprotw           , VexRvmRmvRmi       , V(XOP_M9,91,_,0,x,_,_,_  ), V(XOP_M8,C1,_,0,x,_,_,_  ), 81 , 125, 402, 163), // #1443
  INST(Vpsadbw          , VexRvm_Lx          , V(660F00,F6,_,x,I,I,4,FVM), 0                         , 143, 0  , 206, 173), // #1444
  INST(Vpscatterdd      , VexMr_VM           , E(660F38,A0,_,x,_,0,2,T1S), 0                         , 128, 0  , 403, 149), // #1445
  INST(Vpscatterdq      , VexMr_VM           , E(660F38,A0,_,x,_,1,3,T1S), 0                         , 127, 0  , 404, 149), // #1446
  INST(Vpscatterqd      , VexMr_VM           , E(660F38,A1,_,x,_,0,2,T1S), 0                         , 128, 0  , 405, 149), // #1447
  INST(Vpscatterqq      , VexMr_VM           , E(660F38,A1,_,x,_,1,3,T1S), 0                         , 127, 0  , 406, 149), // #1448
  INST(Vpshab           , VexRvmRmv          , V(XOP_M9,98,_,0,x,_,_,_  ), 0                         , 81 , 0  , 407, 163), // #1449
  INST(Vpshad           , VexRvmRmv          , V(XOP_M9,9A,_,0,x,_,_,_  ), 0                         , 81 , 0  , 407, 163), // #1450
  INST(Vpshaq           , VexRvmRmv          , V(XOP_M9,9B,_,0,x,_,_,_  ), 0                         , 81 , 0  , 407, 163), // #1451
  INST(Vpshaw           , VexRvmRmv          , V(XOP_M9,99,_,0,x,_,_,_  ), 0                         , 81 , 0  , 407, 163), // #1452
  INST(Vpshlb           , VexRvmRmv          , V(XOP_M9,94,_,0,x,_,_,_  ), 0                         , 81 , 0  , 407, 163), // #1453
  INST(Vpshld           , VexRvmRmv          , V(XOP_M9,96,_,0,x,_,_,_  ), 0                         , 81 , 0  , 407, 163), // #1454
  INST(Vpshldd          , VexRvmi_Lx         , E(660F3A,71,_,x,_,0,4,FV ), 0                         , 110, 0  , 209, 178), // #1455
  INST(Vpshldq          , VexRvmi_Lx         , E(660F3A,71,_,x,_,1,4,FV ), 0                         , 111, 0  , 210, 178), // #1456
  INST(Vpshldvd         , VexRvm_Lx          , E(660F38,71,_,x,_,0,4,FV ), 0                         , 113, 0  , 217, 178), // #1457
  INST(Vpshldvq         , VexRvm_Lx          , E(660F38,71,_,x,_,1,4,FV ), 0                         , 112, 0  , 216, 178), // #1458
  INST(Vpshldvw         , VexRvm_Lx          , E(660F38,70,_,x,_,1,4,FVM), 0                         , 112, 0  , 356, 178), // #1459
  INST(Vpshldw          , VexRvmi_Lx         , E(660F3A,70,_,x,_,1,4,FVM), 0                         , 111, 0  , 280, 178), // #1460
  INST(Vpshlq           , VexRvmRmv          , V(XOP_M9,97,_,0,x,_,_,_  ), 0                         , 81 , 0  , 407, 163), // #1461
  INST(Vpshlw           , VexRvmRmv          , V(XOP_M9,95,_,0,x,_,_,_  ), 0                         , 81 , 0  , 407, 163), // #1462
  INST(Vpshrdd          , VexRvmi_Lx         , E(660F3A,73,_,x,_,0,4,FV ), 0                         , 110, 0  , 209, 178), // #1463
  INST(Vpshrdq          , VexRvmi_Lx         , E(660F3A,73,_,x,_,1,4,FV ), 0                         , 111, 0  , 210, 178), // #1464
  INST(Vpshrdvd         , VexRvm_Lx          , E(660F38,73,_,x,_,0,4,FV ), 0                         , 113, 0  , 217, 178), // #1465
  INST(Vpshrdvq         , VexRvm_Lx          , E(660F38,73,_,x,_,1,4,FV ), 0                         , 112, 0  , 216, 178), // #1466
  INST(Vpshrdvw         , VexRvm_Lx          , E(660F38,72,_,x,_,1,4,FVM), 0                         , 112, 0  , 356, 178), // #1467
  INST(Vpshrdw          , VexRvmi_Lx         , E(660F3A,72,_,x,_,1,4,FVM), 0                         , 111, 0  , 280, 178), // #1468
  INST(Vpshufb          , VexRvm_Lx          , V(660F38,00,_,x,I,I,4,FVM), 0                         , 109, 0  , 315, 173), // #1469
  INST(Vpshufbitqmb     , VexRvm_Lx          , E(660F38,8F,_,x,0,0,4,FVM), 0                         , 113, 0  , 408, 185), // #1470
  INST(Vpshufd          , VexRmi_Lx          , V(660F00,70,_,x,I,0,4,FV ), 0                         , 143, 0  , 409, 154), // #1471
  INST(Vpshufhw         , VexRmi_Lx          , V(F30F00,70,_,x,I,I,4,FVM), 0                         , 160, 0  , 410, 173), // #1472
  INST(Vpshuflw         , VexRmi_Lx          , V(F20F00,70,_,x,I,I,4,FVM), 0                         , 217, 0  , 410, 173), // #1473
  INST(Vpsignb          , VexRvm_Lx          , V(660F38,08,_,x,I,_,_,_  ), 0                         , 30 , 0  , 205, 171), // #1474
  INST(Vpsignd          , VexRvm_Lx          , V(660F38,0A,_,x,I,_,_,_  ), 0                         , 30 , 0  , 205, 171), // #1475
  INST(Vpsignw          , VexRvm_Lx          , V(660F38,09,_,x,I,_,_,_  ), 0                         , 30 , 0  , 205, 171), // #1476
  INST(Vpslld           , VexRvmVmi_Lx_MEvex , V(660F00,F2,_,x,I,0,4,128), V(660F00,72,6,x,I,0,4,FV ), 218, 126, 411, 154), // #1477
  INST(Vpslldq          , VexVmi_Lx_MEvex    , V(660F00,73,7,x,I,I,4,FVM), 0                         , 219, 0  , 412, 173), // #1478
  INST(Vpsllq           , VexRvmVmi_Lx_MEvex , V(660F00,F3,_,x,I,1,4,128), V(660F00,73,6,x,I,1,4,FV ), 220, 127, 413, 154), // #1479
  INST(Vpsllvd          , VexRvm_Lx          , V(660F38,47,_,x,0,0,4,FV ), 0                         , 109, 0  , 212, 164), // #1480
  INST(Vpsllvq          , VexRvm_Lx          , V(660F38,47,_,x,1,1,4,FV ), 0                         , 180, 0  , 211, 164), // #1481
  INST(Vpsllvw          , VexRvm_Lx          , E(660F38,12,_,x,_,1,4,FVM), 0                         , 112, 0  , 356, 160), // #1482
  INST(Vpsllw           , VexRvmVmi_Lx_MEvex , V(660F00,F1,_,x,I,I,4,128), V(660F00,71,6,x,I,I,4,FVM), 218, 128, 414, 173), // #1483
  INST(Vpsrad           , VexRvmVmi_Lx_MEvex , V(660F00,E2,_,x,I,0,4,128), V(660F00,72,4,x,I,0,4,FV ), 218, 129, 411, 154), // #1484
  INST(Vpsraq           , VexRvmVmi_Lx_MEvex , E(660F00,E2,_,x,_,1,4,128), E(660F00,72,4,x,_,1,4,FV ), 221, 130, 415, 149), // #1485
  INST(Vpsravd          , VexRvm_Lx          , V(660F38,46,_,x,0,0,4,FV ), 0                         , 109, 0  , 212, 164), // #1486
  INST(Vpsravq          , VexRvm_Lx          , E(660F38,46,_,x,_,1,4,FV ), 0                         , 112, 0  , 216, 149), // #1487
  INST(Vpsravw          , VexRvm_Lx          , E(660F38,11,_,x,_,1,4,FVM), 0                         , 112, 0  , 356, 160), // #1488
  INST(Vpsraw           , VexRvmVmi_Lx_MEvex , V(660F00,E1,_,x,I,I,4,128), V(660F00,71,4,x,I,I,4,FVM), 218, 131, 414, 173), // #1489
  INST(Vpsrld           , VexRvmVmi_Lx_MEvex , V(660F00,D2,_,x,I,0,4,128), V(660F00,72,2,x,I,0,4,FV ), 218, 132, 411, 154), // #1490
  INST(Vpsrldq          , VexVmi_Lx_MEvex    , V(660F00,73,3,x,I,I,4,FVM), 0                         , 222, 0  , 412, 173), // #1491
  INST(Vpsrlq           , VexRvmVmi_Lx_MEvex , V(660F00,D3,_,x,I,1,4,128), V(660F00,73,2,x,I,1,4,FV ), 220, 133, 413, 154), // #1492
  INST(Vpsrlvd          , VexRvm_Lx          , V(660F38,45,_,x,0,0,4,FV ), 0                         , 109, 0  , 212, 164), // #1493
  INST(Vpsrlvq          , VexRvm_Lx          , V(660F38,45,_,x,1,1,4,FV ), 0                         , 180, 0  , 211, 164), // #1494
  INST(Vpsrlvw          , VexRvm_Lx          , E(660F38,10,_,x,_,1,4,FVM), 0                         , 112, 0  , 356, 160), // #1495
  INST(Vpsrlw           , VexRvmVmi_Lx_MEvex , V(660F00,D1,_,x,I,I,4,128), V(660F00,71,2,x,I,I,4,FVM), 218, 134, 414, 173), // #1496
  INST(Vpsubb           , VexRvm_Lx          , V(660F00,F8,_,x,I,I,4,FVM), 0                         , 143, 0  , 416, 173), // #1497
  INST(Vpsubd           , VexRvm_Lx          , V(660F00,FA,_,x,I,0,4,FV ), 0                         , 143, 0  , 417, 154), // #1498
  INST(Vpsubq           , VexRvm_Lx          , V(660F00,FB,_,x,I,1,4,FV ), 0                         , 102, 0  , 418, 154), // #1499
  INST(Vpsubsb          , VexRvm_Lx          , V(660F00,E8,_,x,I,I,4,FVM), 0                         , 143, 0  , 416, 173), // #1500
  INST(Vpsubsw          , VexRvm_Lx          , V(660F00,E9,_,x,I,I,4,FVM), 0                         , 143, 0  , 416, 173), // #1501
  INST(Vpsubusb         , VexRvm_Lx          , V(660F00,D8,_,x,I,I,4,FVM), 0                         , 143, 0  , 416, 173), // #1502
  INST(Vpsubusw         , VexRvm_Lx          , V(660F00,D9,_,x,I,I,4,FVM), 0                         , 143, 0  , 416, 173), // #1503
  INST(Vpsubw           , VexRvm_Lx          , V(660F00,F9,_,x,I,I,4,FVM), 0                         , 143, 0  , 416, 173), // #1504
  INST(Vpternlogd       , VexRvmi_Lx         , E(660F3A,25,_,x,_,0,4,FV ), 0                         , 110, 0  , 209, 149), // #1505
  INST(Vpternlogq       , VexRvmi_Lx         , E(660F3A,25,_,x,_,1,4,FV ), 0                         , 111, 0  , 210, 149), // #1506
  INST(Vptest           , VexRm_Lx           , V(660F38,17,_,x,I,_,_,_  ), 0                         , 30 , 0  , 301, 177), // #1507
  INST(Vptestmb         , VexRvm_Lx          , E(660F38,26,_,x,_,0,4,FVM), 0                         , 113, 0  , 408, 160), // #1508
  INST(Vptestmd         , VexRvm_Lx          , E(660F38,27,_,x,_,0,4,FV ), 0                         , 113, 0  , 419, 149), // #1509
  INST(Vptestmq         , VexRvm_Lx          , E(660F38,27,_,x,_,1,4,FV ), 0                         , 112, 0  , 420, 149), // #1510
  INST(Vptestmw         , VexRvm_Lx          , E(660F38,26,_,x,_,1,4,FVM), 0                         , 112, 0  , 408, 160), // #1511
  INST(Vptestnmb        , VexRvm_Lx          , E(F30F38,26,_,x,_,0,4,FVM), 0                         , 169, 0  , 408, 160), // #1512
  INST(Vptestnmd        , VexRvm_Lx          , E(F30F38,27,_,x,_,0,4,FV ), 0                         , 169, 0  , 419, 149), // #1513
  INST(Vptestnmq        , VexRvm_Lx          , E(F30F38,27,_,x,_,1,4,FV ), 0                         , 223, 0  , 420, 149), // #1514
  INST(Vptestnmw        , VexRvm_Lx          , E(F30F38,26,_,x,_,1,4,FVM), 0                         , 223, 0  , 408, 160), // #1515
  INST(Vpunpckhbw       , VexRvm_Lx          , V(660F00,68,_,x,I,I,4,FVM), 0                         , 143, 0  , 315, 173), // #1516
  INST(Vpunpckhdq       , VexRvm_Lx          , V(660F00,6A,_,x,I,0,4,FV ), 0                         , 143, 0  , 212, 154), // #1517
  INST(Vpunpckhqdq      , VexRvm_Lx          , V(660F00,6D,_,x,I,1,4,FV ), 0                         , 102, 0  , 211, 154), // #1518
  INST(Vpunpckhwd       , VexRvm_Lx          , V(660F00,69,_,x,I,I,4,FVM), 0                         , 143, 0  , 315, 173), // #1519
  INST(Vpunpcklbw       , VexRvm_Lx          , V(660F00,60,_,x,I,I,4,FVM), 0                         , 143, 0  , 315, 173), // #1520
  INST(Vpunpckldq       , VexRvm_Lx          , V(660F00,62,_,x,I,0,4,FV ), 0                         , 143, 0  , 212, 154), // #1521
  INST(Vpunpcklqdq      , VexRvm_Lx          , V(660F00,6C,_,x,I,1,4,FV ), 0                         , 102, 0  , 211, 154), // #1522
  INST(Vpunpcklwd       , VexRvm_Lx          , V(660F00,61,_,x,I,I,4,FVM), 0                         , 143, 0  , 315, 173), // #1523
  INST(Vpxor            , VexRvm_Lx          , V(660F00,EF,_,x,I,_,_,_  ), 0                         , 71 , 0  , 352, 171), // #1524
  INST(Vpxord           , VexRvm_Lx          , E(660F00,EF,_,x,_,0,4,FV ), 0                         , 192, 0  , 353, 149), // #1525
  INST(Vpxorq           , VexRvm_Lx          , E(660F00,EF,_,x,_,1,4,FV ), 0                         , 134, 0  , 354, 149), // #1526
  INST(Vrangepd         , VexRvmi_Lx         , E(660F3A,50,_,x,_,1,4,FV ), 0                         , 111, 0  , 288, 152), // #1527
  INST(Vrangeps         , VexRvmi_Lx         , E(660F3A,50,_,x,_,0,4,FV ), 0                         , 110, 0  , 289, 152), // #1528
  INST(Vrangesd         , VexRvmi            , E(660F3A,51,_,I,_,1,3,T1S), 0                         , 178, 0  , 290, 152), // #1529
  INST(Vrangess         , VexRvmi            , E(660F3A,51,_,I,_,0,2,T1S), 0                         , 179, 0  , 291, 152), // #1530
  INST(Vrcp14pd         , VexRm_Lx           , E(660F38,4C,_,x,_,1,4,FV ), 0                         , 112, 0  , 349, 149), // #1531
  INST(Vrcp14ps         , VexRm_Lx           , E(660F38,4C,_,x,_,0,4,FV ), 0                         , 113, 0  , 374, 149), // #1532
  INST(Vrcp14sd         , VexRvm             , E(660F38,4D,_,I,_,1,3,T1S), 0                         , 127, 0  , 421, 149), // #1533
  INST(Vrcp14ss         , VexRvm             , E(660F38,4D,_,I,_,0,2,T1S), 0                         , 128, 0  , 422, 149), // #1534
  INST(Vrcpph           , VexRm_Lx           , E(66MAP6,4C,_,_,_,0,4,FV ), 0                         , 181, 0  , 423, 145), // #1535
  INST(Vrcpps           , VexRm_Lx           , V(000F00,53,_,x,I,_,_,_  ), 0                         , 74 , 0  , 301, 146), // #1536
  INST(Vrcpsh           , VexRvm             , E(66MAP6,4D,_,_,_,0,1,T1S), 0                         , 183, 0  , 424, 145), // #1537
  INST(Vrcpss           , VexRvm             , V(F30F00,53,_,I,I,_,_,_  ), 0                         , 193, 0  , 425, 146), // #1538
  INST(Vreducepd        , VexRmi_Lx          , E(660F3A,56,_,x,_,1,4,FV ), 0                         , 111, 0  , 401, 152), // #1539
  INST(Vreduceph        , VexRmi_Lx          , E(000F3A,56,_,_,_,0,4,FV ), 0                         , 122, 0  , 311, 145), // #1540
  INST(Vreduceps        , VexRmi_Lx          , E(660F3A,56,_,x,_,0,4,FV ), 0                         , 110, 0  , 400, 152), // #1541
  INST(Vreducesd        , VexRvmi            , E(660F3A,57,_,I,_,1,3,T1S), 0                         , 178, 0  , 426, 152), // #1542
  INST(Vreducesh        , VexRvmi            , E(000F3A,57,_,_,_,0,1,T1S), 0                         , 186, 0  , 313, 145), // #1543
  INST(Vreducess        , VexRvmi            , E(660F3A,57,_,I,_,0,2,T1S), 0                         , 179, 0  , 427, 152), // #1544
  INST(Vrndscalepd      , VexRmi_Lx          , E(660F3A,09,_,x,_,1,4,FV ), 0                         , 111, 0  , 310, 149), // #1545
  INST(Vrndscaleph      , VexRmi_Lx          , E(000F3A,08,_,_,_,0,4,FV ), 0                         , 122, 0  , 311, 145), // #1546
  INST(Vrndscaleps      , VexRmi_Lx          , E(660F3A,08,_,x,_,0,4,FV ), 0                         , 110, 0  , 312, 149), // #1547
  INST(Vrndscalesd      , VexRvmi            , E(660F3A,0B,_,I,_,1,3,T1S), 0                         , 178, 0  , 290, 149), // #1548
  INST(Vrndscalesh      , VexRvmi            , E(000F3A,0A,_,_,_,0,1,T1S), 0                         , 186, 0  , 313, 145), // #1549
  INST(Vrndscaless      , VexRvmi            , E(660F3A,0A,_,I,_,0,2,T1S), 0                         , 179, 0  , 291, 149), // #1550
  INST(Vroundpd         , VexRmi_Lx          , V(660F3A,09,_,x,I,_,_,_  ), 0                         , 75 , 0  , 428, 146), // #1551
  INST(Vroundps         , VexRmi_Lx          , V(660F3A,08,_,x,I,_,_,_  ), 0                         , 75 , 0  , 428, 146), // #1552
  INST(Vroundsd         , VexRvmi            , V(660F3A,0B,_,I,I,_,_,_  ), 0                         , 75 , 0  , 429, 146), // #1553
  INST(Vroundss         , VexRvmi            , V(660F3A,0A,_,I,I,_,_,_  ), 0                         , 75 , 0  , 430, 146), // #1554
  INST(Vrsqrt14pd       , VexRm_Lx           , E(660F38,4E,_,x,_,1,4,FV ), 0                         , 112, 0  , 349, 149), // #1555
  INST(Vrsqrt14ps       , VexRm_Lx           , E(660F38,4E,_,x,_,0,4,FV ), 0                         , 113, 0  , 374, 149), // #1556
  INST(Vrsqrt14sd       , VexRvm             , E(660F38,4F,_,I,_,1,3,T1S), 0                         , 127, 0  , 421, 149), // #1557
  INST(Vrsqrt14ss       , VexRvm             , E(660F38,4F,_,I,_,0,2,T1S), 0                         , 128, 0  , 422, 149), // #1558
  INST(Vrsqrtph         , VexRm_Lx           , E(66MAP6,4E,_,_,_,0,4,FV ), 0                         , 181, 0  , 423, 145), // #1559
  INST(Vrsqrtps         , VexRm_Lx           , V(000F00,52,_,x,I,_,_,_  ), 0                         , 74 , 0  , 301, 146), // #1560
  INST(Vrsqrtsh         , VexRvm             , E(66MAP6,4F,_,_,_,0,1,T1S), 0                         , 183, 0  , 424, 145), // #1561
  INST(Vrsqrtss         , VexRvm             , V(F30F00,52,_,I,I,_,_,_  ), 0                         , 193, 0  , 425, 146), // #1562
  INST(Vscalefpd        , VexRvm_Lx          , E(660F38,2C,_,x,_,1,4,FV ), 0                         , 112, 0  , 431, 149), // #1563
  INST(Vscalefph        , VexRvm_Lx          , E(66MAP6,2C,_,_,_,0,4,FV ), 0                         , 181, 0  , 200, 145), // #1564
  INST(Vscalefps        , VexRvm_Lx          , E(660F38,2C,_,x,_,0,4,FV ), 0                         , 113, 0  , 287, 149), // #1565
  INST(Vscalefsd        , VexRvm             , E(660F38,2D,_,I,_,1,3,T1S), 0                         , 127, 0  , 256, 149), // #1566
  INST(Vscalefsh        , VexRvm             , E(66MAP6,2D,_,_,_,0,1,T1S), 0                         , 183, 0  , 203, 145), // #1567
  INST(Vscalefss        , VexRvm             , E(660F38,2D,_,I,_,0,2,T1S), 0                         , 128, 0  , 264, 149), // #1568
  INST(Vscatterdpd      , VexMr_VM           , E(660F38,A2,_,x,_,1,3,T1S), 0                         , 127, 0  , 404, 149), // #1569
  INST(Vscatterdps      , VexMr_VM           , E(660F38,A2,_,x,_,0,2,T1S), 0                         , 128, 0  , 403, 149), // #1570
  INST(Vscatterqpd      , VexMr_VM           , E(660F38,A3,_,x,_,1,3,T1S), 0                         , 127, 0  , 406, 149), // #1571
  INST(Vscatterqps      , VexMr_VM           , E(660F38,A3,_,x,_,0,2,T1S), 0                         , 128, 0  , 405, 149), // #1572
  INST(Vsha512msg1      , VexRm              , V(F20F38,CC,_,1,0,_,_,_  ), 0                         , 224, 0  , 432, 187), // #1573
  INST(Vsha512msg2      , VexRm              , V(F20F38,CD,_,1,0,_,_,_  ), 0                         , 224, 0  , 433, 187), // #1574
  INST(Vsha512rnds2     , VexRvm             , V(F20F38,CB,_,1,0,_,_,_  ), 0                         , 224, 0  , 434, 187), // #1575
  INST(Vshuff32x4       , VexRvmi_Lx         , E(660F3A,23,_,x,_,0,4,FV ), 0                         , 110, 0  , 435, 149), // #1576
  INST(Vshuff64x2       , VexRvmi_Lx         , E(660F3A,23,_,x,_,1,4,FV ), 0                         , 111, 0  , 436, 149), // #1577
  INST(Vshufi32x4       , VexRvmi_Lx         , E(660F3A,43,_,x,_,0,4,FV ), 0                         , 110, 0  , 435, 149), // #1578
  INST(Vshufi64x2       , VexRvmi_Lx         , E(660F3A,43,_,x,_,1,4,FV ), 0                         , 111, 0  , 436, 149), // #1579
  INST(Vshufpd          , VexRvmi_Lx         , V(660F00,C6,_,x,I,1,4,FV ), 0                         , 102, 0  , 437, 144), // #1580
  INST(Vshufps          , VexRvmi_Lx         , V(000F00,C6,_,x,I,0,4,FV ), 0                         , 104, 0  , 438, 144), // #1581
  INST(Vsm3msg1         , VexRvm             , V(000F38,DA,_,0,0,_,_,_  ), 0                         , 11 , 0  , 439, 188), // #1582
  INST(Vsm3msg2         , VexRvm             , V(660F38,DA,_,0,0,_,_,_  ), 0                         , 30 , 0  , 439, 188), // #1583
  INST(Vsm3rnds2        , VexRvmi            , V(660F3A,DE,_,0,0,_,_,_  ), 0                         , 75 , 0  , 281, 188), // #1584
  INST(Vsm4key4         , VexRvm_Lx          , V(F30F38,DA,_,x,0,_,_,_  ), 0                         , 89 , 0  , 205, 189), // #1585
  INST(Vsm4rnds4        , VexRvm_Lx          , V(F20F38,DA,_,x,0,_,_,_  ), 0                         , 85 , 0  , 205, 189), // #1586
  INST(Vsqrtpd          , VexRm_Lx           , V(660F00,51,_,x,I,1,4,FV ), 0                         , 102, 0  , 440, 144), // #1587
  INST(Vsqrtph          , VexRm_Lx           , E(00MAP5,51,_,_,_,0,4,FV ), 0                         , 103, 0  , 251, 145), // #1588
  INST(Vsqrtps          , VexRm_Lx           , V(000F00,51,_,x,I,0,4,FV ), 0                         , 104, 0  , 239, 144), // #1589
  INST(Vsqrtsd          , VexRvm             , V(F20F00,51,_,I,I,1,3,T1S), 0                         , 105, 0  , 202, 144), // #1590
  INST(Vsqrtsh          , VexRvm             , E(F3MAP5,51,_,_,_,0,1,T1S), 0                         , 106, 0  , 203, 145), // #1591
  INST(Vsqrtss          , VexRvm             , V(F30F00,51,_,I,I,0,2,T1S), 0                         , 107, 0  , 204, 144), // #1592
  INST(Vstmxcsr         , VexM               , V(000F00,AE,3,0,I,_,_,_  ), 0                         , 225, 0  , 320, 146), // #1593
  INST(Vsubpd           , VexRvm_Lx          , V(660F00,5C,_,x,I,1,4,FV ), 0                         , 102, 0  , 199, 144), // #1594
  INST(Vsubph           , VexRvm_Lx          , E(00MAP5,5C,_,_,_,0,4,FV ), 0                         , 103, 0  , 200, 145), // #1595
  INST(Vsubps           , VexRvm_Lx          , V(000F00,5C,_,x,I,0,4,FV ), 0                         , 104, 0  , 201, 144), // #1596
  INST(Vsubsd           , VexRvm             , V(F20F00,5C,_,I,I,1,3,T1S), 0                         , 105, 0  , 202, 144), // #1597
  INST(Vsubsh           , VexRvm             , E(F3MAP5,5C,_,_,_,0,1,T1S), 0                         , 106, 0  , 203, 145), // #1598
  INST(Vsubss           , VexRvm             , V(F30F00,5C,_,I,I,0,2,T1S), 0                         , 107, 0  , 204, 144), // #1599
  INST(Vtestpd          , VexRm_Lx           , V(660F38,0F,_,x,0,_,_,_  ), 0                         , 30 , 0  , 301, 177), // #1600
  INST(Vtestps          , VexRm_Lx           , V(660F38,0E,_,x,0,_,_,_  ), 0                         , 30 , 0  , 301, 177), // #1601
  INST(Vucomisd         , VexRm              , V(660F00,2E,_,I,I,1,3,T1S), 0                         , 124, 0  , 233, 155), // #1602
  INST(Vucomish         , VexRm              , E(00MAP5,2E,_,_,_,0,1,T1S), 0                         , 125, 0  , 234, 156), // #1603
  INST(Vucomiss         , VexRm              , V(000F00,2E,_,I,I,0,2,T1S), 0                         , 126, 0  , 235, 155), // #1604
  INST(Vunpckhpd        , VexRvm_Lx          , V(660F00,15,_,x,I,1,4,FV ), 0                         , 102, 0  , 211, 144), // #1605
  INST(Vunpckhps        , VexRvm_Lx          , V(000F00,15,_,x,I,0,4,FV ), 0                         , 104, 0  , 212, 144), // #1606
  INST(Vunpcklpd        , VexRvm_Lx          , V(660F00,14,_,x,I,1,4,FV ), 0                         , 102, 0  , 211, 144), // #1607
  INST(Vunpcklps        , VexRvm_Lx          , V(000F00,14,_,x,I,0,4,FV ), 0                         , 104, 0  , 212, 144), // #1608
  INST(Vxorpd           , VexRvm_Lx          , V(660F00,57,_,x,I,1,4,FV ), 0                         , 102, 0  , 418, 150), // #1609
  INST(Vxorps           , VexRvm_Lx          , V(000F00,57,_,x,I,0,4,FV ), 0                         , 104, 0  , 417, 150), // #1610
  INST(Vzeroall         , VexOp              , V(000F00,77,_,1,I,_,_,_  ), 0                         , 70 , 0  , 441, 146), // #1611
  INST(Vzeroupper       , VexOp              , V(000F00,77,_,0,I,_,_,_  ), 0                         , 74 , 0  , 441, 146), // #1612
  INST(Wbinvd           , X86Op              , O(000F00,09,_,_,_,_,_,_  ), 0                         , 5  , 0  , 31 , 45 ), // #1613
  INST(Wbnoinvd         , X86Op              , O(F30F00,09,_,_,_,_,_,_  ), 0                         , 7  , 0  , 31 , 190), // #1614
  INST(Wrfsbase         , X86M               , O(F30F00,AE,2,_,x,_,_,_  ), 0                         , 226, 0  , 177, 122), // #1615
  INST(Wrgsbase         , X86M               , O(F30F00,AE,3,_,x,_,_,_  ), 0                         , 227, 0  , 177, 122), // #1616
  INST(Wrmsr            , X86Op              , O(000F00,30,_,_,_,_,_,_  ), 0                         , 5  , 0  , 180, 191), // #1617
  INST(Wrssd            , X86Mr              , O(000F38,F6,_,_,_,_,_,_  ), 0                         , 1  , 0  , 442, 65 ), // #1618
  INST(Wrssq            , X86Mr              , O(000F38,F6,_,_,1,_,_,_  ), 0                         , 228, 0  , 443, 65 ), // #1619
  INST(Wrussd           , X86Mr              , O(660F38,F5,_,_,_,_,_,_  ), 0                         , 2  , 0  , 442, 65 ), // #1620
  INST(Wrussq           , X86Mr              , O(660F38,F5,_,_,1,_,_,_  ), 0                         , 229, 0  , 443, 65 ), // #1621
  INST(Xabort           , X86Op_Mod11RM_I8   , O(000000,C6,7,_,_,_,_,_  ), 0                         , 29 , 0  , 83 , 192), // #1622
  INST(Xadd             , X86Xadd            , O(000F00,C0,_,_,x,_,_,_  ), 0                         , 5  , 0  , 444, 40 ), // #1623
  INST(Xbegin           , X86JmpRel          , O(000000,C7,7,_,_,_,_,_  ), 0                         , 29 , 0  , 445, 192), // #1624
  INST(Xchg             , X86Xchg            , O(000000,86,_,_,x,_,_,_  ), 0                         , 0  , 0  , 446, 0  ), // #1625
  INST(Xend             , X86Op              , O(000F01,D5,_,_,_,_,_,_  ), 0                         , 23 , 0  , 31 , 192), // #1626
  INST(Xgetbv           , X86Op              , O(000F01,D0,_,_,_,_,_,_  ), 0                         , 23 , 0  , 180, 193), // #1627
  INST(Xlatb            , X86Op              , O(000000,D7,_,_,_,_,_,_  ), 0                         , 0  , 0  , 31 , 0  ), // #1628
  INST(Xor              , X86Arith           , O(000000,30,6,_,x,_,_,_  ), 0                         , 34 , 0  , 184, 1  ), // #1629
  INST(Xorpd            , ExtRm              , O(660F00,57,_,_,_,_,_,_  ), 0                         , 4  , 0  , 154, 5  ), // #1630
  INST(Xorps            , ExtRm              , O(000F00,57,_,_,_,_,_,_  ), 0                         , 5  , 0  , 154, 6  ), // #1631
  INST(Xresldtrk        , X86Op              , O(F20F01,E9,_,_,_,_,_,_  ), 0                         , 93 , 0  , 31 , 194), // #1632
  INST(Xrstor           , X86M_Only_EDX_EAX  , O(000F00,AE,5,_,_,_,_,_  ), 0                         , 79 , 0  , 447, 193), // #1633
  INST(Xrstor64         , X86M_Only_EDX_EAX  , O(000F00,AE,5,_,1,_,_,_  ), 0                         , 230, 0  , 448, 193), // #1634
  INST(Xrstors          , X86M_Only_EDX_EAX  , O(000F00,C7,3,_,_,_,_,_  ), 0                         , 80 , 0  , 447, 195), // #1635
  INST(Xrstors64        , X86M_Only_EDX_EAX  , O(000F00,C7,3,_,1,_,_,_  ), 0                         , 231, 0  , 448, 195), // #1636
  INST(Xsave            , X86M_Only_EDX_EAX  , O(000F00,AE,4,_,_,_,_,_  ), 0                         , 98 , 0  , 447, 193), // #1637
  INST(Xsave64          , X86M_Only_EDX_EAX  , O(000F00,AE,4,_,1,_,_,_  ), 0                         , 232, 0  , 448, 193), // #1638
  INST(Xsavec           , X86M_Only_EDX_EAX  , O(000F00,C7,4,_,_,_,_,_  ), 0                         , 98 , 0  , 447, 196), // #1639
  INST(Xsavec64         , X86M_Only_EDX_EAX  , O(000F00,C7,4,_,1,_,_,_  ), 0                         , 232, 0  , 448, 196), // #1640
  INST(Xsaveopt         , X86M_Only_EDX_EAX  , O(000F00,AE,6,_,_,_,_,_  ), 0                         , 82 , 0  , 447, 197), // #1641
  INST(Xsaveopt64       , X86M_Only_EDX_EAX  , O(000F00,AE,6,_,1,_,_,_  ), 0                         , 233, 0  , 448, 197), // #1642
  INST(Xsaves           , X86M_Only_EDX_EAX  , O(000F00,C7,5,_,_,_,_,_  ), 0                         , 79 , 0  , 447, 195), // #1643
  INST(Xsaves64         , X86M_Only_EDX_EAX  , O(000F00,C7,5,_,1,_,_,_  ), 0                         , 230, 0  , 448, 195), // #1644
  INST(Xsetbv           , X86Op              , O(000F01,D1,_,_,_,_,_,_  ), 0                         , 23 , 0  , 180, 193), // #1645
  INST(Xsusldtrk        , X86Op              , O(F20F01,E8,_,_,_,_,_,_  ), 0                         , 93 , 0  , 31 , 194), // #1646
  INST(Xtest            , X86Op              , O(000F01,D6,_,_,_,_,_,_  ), 0                         , 23 , 0  , 31 , 198)  // #1647
  // ${InstInfo:End}
};
#undef NAME_DATA_INDEX
#undef INST

// x86::InstDB - Opcode Tables
// ===========================

// ${MainOpcodeTable:Begin}
// ------------------- Automatically generated, do not edit -------------------
const uint32_t InstDB::main_opcode_table[] = {
  O(000000,00,0,0,0,0,0,0   ), // #0 [ref=56x]
  O(000F38,00,0,0,0,0,0,0   ), // #1 [ref=25x]
  O(660F38,00,0,0,0,0,0,0   ), // #2 [ref=44x]
  O(000000,00,2,0,0,0,0,0   ), // #3 [ref=4x]
  O(660F00,00,0,0,0,0,0,0   ), // #4 [ref=38x]
  O(000F00,00,0,0,0,0,0,0   ), // #5 [ref=189x]
  O(F20F00,00,0,0,0,0,0,0   ), // #6 [ref=24x]
  O(F30F00,00,0,0,0,0,0,0   ), // #7 [ref=29x]
  O(F30F38,00,0,0,0,0,0,0   ), // #8 [ref=3x]
  O(660F3A,00,0,0,0,0,0,0   ), // #9 [ref=22x]
  O(000000,00,4,0,0,0,0,0   ), // #10 [ref=4x]
  V(000F38,00,0,0,0,0,0,None), // #11 [ref=13x]
  O(F20F38,00,0,0,0,0,0,0   ), // #12 [ref=3x]
  V(XOP_M9,00,1,0,0,0,0,None), // #13 [ref=3x]
  V(XOP_M9,00,6,0,0,0,0,None), // #14 [ref=2x]
  V(XOP_M9,00,5,0,0,0,0,None), // #15 [ref=1x]
  V(XOP_M9,00,3,0,0,0,0,None), // #16 [ref=1x]
  V(XOP_M9,00,2,0,0,0,0,None), // #17 [ref=1x]
  V(000F38,00,3,0,0,0,0,None), // #18 [ref=1x]
  V(000F38,00,2,0,0,0,0,None), // #19 [ref=1x]
  V(000F38,00,1,0,0,0,0,None), // #20 [ref=1x]
  O(660000,00,0,0,0,0,0,0   ), // #21 [ref=7x]
  O(000000,00,0,0,1,0,0,0   ), // #22 [ref=3x]
  O(000F01,00,0,0,0,0,0,0   ), // #23 [ref=32x]
  O(000F00,00,7,0,0,0,0,0   ), // #24 [ref=6x]
  O(660F00,00,7,0,0,0,0,0   ), // #25 [ref=1x]
  O(F30F00,00,6,0,0,0,0,0   ), // #26 [ref=4x]
  O(F30F01,00,0,0,0,0,0,0   ), // #27 [ref=9x]
  O(660F00,00,6,0,0,0,0,0   ), // #28 [ref=3x]
  O(000000,00,7,0,0,0,0,0   ), // #29 [ref=5x]
  V(660F38,00,0,0,0,0,0,None), // #30 [ref=48x]
  O(000F00,00,1,0,1,0,0,0   ), // #31 [ref=2x]
  O(000F00,00,1,0,0,0,0,0   ), // #32 [ref=6x]
  O(000000,00,1,0,0,0,0,0   ), // #33 [ref=3x]
  O(000000,00,6,0,0,0,0,0   ), // #34 [ref=4x]
  O(F30F00,00,7,0,0,0,0,3   ), // #35 [ref=1x]
  O(F30F00,00,7,0,0,0,0,2   ), // #36 [ref=1x]
  O_FPU(00,D900,0)           , // #37 [ref=29x]
  O_FPU(00,C000,0)           , // #38 [ref=1x]
  O_FPU(00,DE00,0)           , // #39 [ref=7x]
  O_FPU(00,0000,4)           , // #40 [ref=4x]
  O_FPU(00,0000,6)           , // #41 [ref=4x]
  O_FPU(9B,DB00,0)           , // #42 [ref=2x]
  O_FPU(00,DA00,0)           , // #43 [ref=5x]
  O_FPU(00,DB00,0)           , // #44 [ref=8x]
  O_FPU(00,D000,2)           , // #45 [ref=1x]
  O_FPU(00,DF00,0)           , // #46 [ref=2x]
  O_FPU(00,D800,3)           , // #47 [ref=1x]
  O_FPU(00,F000,6)           , // #48 [ref=1x]
  O_FPU(00,F800,7)           , // #49 [ref=1x]
  O_FPU(00,DD00,0)           , // #50 [ref=3x]
  O_FPU(00,0000,0)           , // #51 [ref=4x]
  O_FPU(00,0000,2)           , // #52 [ref=3x]
  O_FPU(00,0000,3)           , // #53 [ref=3x]
  O_FPU(00,0000,7)           , // #54 [ref=3x]
  O_FPU(00,0000,1)           , // #55 [ref=2x]
  O_FPU(00,0000,5)           , // #56 [ref=2x]
  O_FPU(00,C800,1)           , // #57 [ref=1x]
  O_FPU(9B,0000,6)           , // #58 [ref=2x]
  O_FPU(9B,0000,7)           , // #59 [ref=2x]
  O_FPU(00,E000,4)           , // #60 [ref=1x]
  O_FPU(00,E800,5)           , // #61 [ref=1x]
  O(000F00,00,0,0,1,0,0,0   ), // #62 [ref=3x]
  O(F30F3A,00,0,0,0,0,0,0   ), // #63 [ref=1x]
  O(000000,00,5,0,0,0,0,0   ), // #64 [ref=4x]
  O(F30F00,00,5,0,0,0,0,0   ), // #65 [ref=2x]
  O(F30F00,00,5,0,1,0,0,0   ), // #66 [ref=1x]
  V(660F00,00,0,1,0,0,0,None), // #67 [ref=7x]
  V(660F00,00,0,1,1,0,0,None), // #68 [ref=6x]
  V(000F00,00,0,1,1,0,0,None), // #69 [ref=7x]
  V(000F00,00,0,1,0,0,0,None), // #70 [ref=8x]
  V(660F00,00,0,0,0,0,0,None), // #71 [ref=15x]
  V(660F00,00,0,0,1,0,0,None), // #72 [ref=4x]
  V(000F00,00,0,0,1,0,0,None), // #73 [ref=4x]
  V(000F00,00,0,0,0,0,0,None), // #74 [ref=10x]
  V(660F3A,00,0,0,0,0,0,None), // #75 [ref=48x]
  V(660F3A,00,0,0,1,0,0,None), // #76 [ref=4x]
  O(000000,00,3,0,0,0,0,0   ), // #77 [ref=4x]
  O(000F00,00,2,0,0,0,0,0   ), // #78 [ref=5x]
  O(000F00,00,5,0,0,0,0,0   ), // #79 [ref=4x]
  O(000F00,00,3,0,0,0,0,0   ), // #80 [ref=5x]
  V(XOP_M9,00,0,0,0,0,0,None), // #81 [ref=32x]
  O(000F00,00,6,0,0,0,0,0   ), // #82 [ref=6x]
  V(XOP_MA,00,0,0,0,0,0,None), // #83 [ref=1x]
  V(XOP_MA,00,1,0,0,0,0,None), // #84 [ref=1x]
  V(F20F38,00,0,0,0,0,0,None), // #85 [ref=11x]
  O(000F3A,00,0,0,0,0,0,0   ), // #86 [ref=4x]
  O(F30000,00,0,0,0,0,0,0   ), // #87 [ref=1x]
  O(000F0F,00,0,0,0,0,0,0   ), // #88 [ref=26x]
  V(F30F38,00,0,0,0,0,0,None), // #89 [ref=12x]
  O(000F3A,00,0,0,1,0,0,0   ), // #90 [ref=1x]
  O(660F3A,00,0,0,1,0,0,0   ), // #91 [ref=1x]
  O(F30F00,00,4,0,0,0,0,0   ), // #92 [ref=1x]
  O(F20F01,00,0,0,0,0,0,0   ), // #93 [ref=5x]
  O(F30F00,00,1,0,0,0,0,0   ), // #94 [ref=3x]
  O(F30F00,00,7,0,0,0,0,0   ), // #95 [ref=1x]
  V(F20F3A,00,0,0,0,0,0,None), // #96 [ref=1x]
  O(660F01,00,0,0,0,0,0,0   ), // #97 [ref=4x]
  O(000F00,00,4,0,0,0,0,0   ), // #98 [ref=4x]
  V(XOP_M9,00,7,0,0,0,0,None), // #99 [ref=1x]
  V(XOP_M9,00,4,0,0,0,0,None), // #100 [ref=1x]
  O(F20F00,00,6,0,0,0,0,0   ), // #101 [ref=1x]
  V(660F00,00,0,0,0,1,4,ByLL), // #102 [ref=25x]
  E(00MAP5,00,0,0,0,0,4,ByLL), // #103 [ref=10x]
  V(000F00,00,0,0,0,0,4,ByLL), // #104 [ref=19x]
  V(F20F00,00,0,0,0,1,3,None), // #105 [ref=10x]
  E(F3MAP5,00,0,0,0,0,1,None), // #106 [ref=13x]
  V(F30F00,00,0,0,0,0,2,None), // #107 [ref=12x]
  V(F20F00,00,0,0,0,0,0,None), // #108 [ref=4x]
  V(660F38,00,0,0,0,0,4,ByLL), // #109 [ref=50x]
  E(660F3A,00,0,0,0,0,4,ByLL), // #110 [ref=17x]
  E(660F3A,00,0,0,0,1,4,ByLL), // #111 [ref=18x]
  E(660F38,00,0,0,0,1,4,ByLL), // #112 [ref=38x]
  E(660F38,00,0,0,0,0,4,ByLL), // #113 [ref=25x]
  V(660F38,00,0,1,0,0,0,None), // #114 [ref=2x]
  E(660F38,00,0,0,0,0,3,None), // #115 [ref=2x]
  E(660F38,00,0,0,0,0,4,None), // #116 [ref=2x]
  E(660F38,00,0,2,0,0,5,None), // #117 [ref=2x]
  E(660F38,00,0,0,0,1,4,None), // #118 [ref=2x]
  E(660F38,00,0,2,0,1,5,None), // #119 [ref=2x]
  V(660F38,00,0,0,0,1,3,None), // #120 [ref=2x]
  V(660F38,00,0,0,0,0,2,None), // #121 [ref=14x]
  E(000F3A,00,0,0,0,0,4,ByLL), // #122 [ref=5x]
  E(F30F3A,00,0,0,0,0,1,None), // #123 [ref=1x]
  V(660F00,00,0,0,0,1,3,None), // #124 [ref=5x]
  E(00MAP5,00,0,0,0,0,1,None), // #125 [ref=2x]
  V(000F00,00,0,0,0,0,2,None), // #126 [ref=2x]
  E(660F38,00,0,0,0,1,3,None), // #127 [ref=12x]
  E(660F38,00,0,0,0,0,2,None), // #128 [ref=12x]
  V(F30F00,00,0,0,0,0,3,ByLL), // #129 [ref=1x]
  E(F20F38,00,0,0,0,0,4,ByLL), // #130 [ref=2x]
  V(F30F38,00,0,0,0,0,4,ByLL), // #131 [ref=1x]
  V(F20F00,00,0,0,0,1,4,ByLL), // #132 [ref=1x]
  E(66MAP5,00,0,0,0,1,4,ByLL), // #133 [ref=1x]
  E(660F00,00,0,0,0,1,4,ByLL), // #134 [ref=10x]
  E(000F00,00,0,0,0,1,4,ByLL), // #135 [ref=3x]
  E(66MAP5,00,0,0,0,0,3,ByLL), // #136 [ref=1x]
  E(00MAP5,00,0,0,0,0,2,ByLL), // #137 [ref=1x]
  V(660F38,00,0,0,0,0,3,ByLL), // #138 [ref=7x]
  E(66MAP6,00,0,0,0,0,3,ByLL), // #139 [ref=1x]
  E(66MAP5,00,0,0,0,0,2,ByLL), // #140 [ref=4x]
  E(00MAP5,00,0,0,0,0,3,ByLL), // #141 [ref=2x]
  E(66MAP5,00,0,0,0,0,4,ByLL), // #142 [ref=3x]
  V(660F00,00,0,0,0,0,4,ByLL), // #143 [ref=43x]
  V(000F00,00,0,0,0,0,3,ByLL), // #144 [ref=1x]
  V(660F3A,00,0,0,0,0,3,ByLL), // #145 [ref=1x]
  E(660F00,00,0,0,0,0,3,ByLL), // #146 [ref=4x]
  E(000F00,00,0,0,0,0,4,ByLL), // #147 [ref=2x]
  E(F30F00,00,0,0,0,1,4,ByLL), // #148 [ref=3x]
  E(00MAP5,00,0,0,0,1,4,ByLL), // #149 [ref=1x]
  E(F2MAP5,00,0,0,0,1,3,None), // #150 [ref=1x]
  V(F20F00,00,0,0,0,0,3,None), // #151 [ref=2x]
  E(F20F00,00,0,0,0,0,3,None), // #152 [ref=2x]
  E(00MAP6,00,0,0,0,0,1,None), // #153 [ref=1x]
  V(F20F00,00,0,0,0,0,2,T1W ), // #154 [ref=1x]
  E(F3MAP5,00,0,0,0,0,2,T1W ), // #155 [ref=2x]
  V(F30F00,00,0,0,0,0,2,T1W ), // #156 [ref=1x]
  E(00MAP5,00,0,0,0,0,2,None), // #157 [ref=1x]
  E(F30F00,00,0,0,0,0,2,None), // #158 [ref=2x]
  E(F3MAP5,00,0,0,0,0,3,ByLL), // #159 [ref=1x]
  V(F30F00,00,0,0,0,0,4,ByLL), // #160 [ref=4x]
  E(F30F00,00,0,0,0,0,3,ByLL), // #161 [ref=1x]
  E(F2MAP5,00,0,0,0,0,4,ByLL), // #162 [ref=2x]
  E(F20F00,00,0,0,0,0,4,ByLL), // #163 [ref=2x]
  E(F2MAP5,00,0,0,0,1,4,ByLL), // #164 [ref=1x]
  E(F20F00,00,0,0,0,1,4,ByLL), // #165 [ref=2x]
  E(F20F00,00,0,0,0,0,2,T1W ), // #166 [ref=1x]
  E(F30F00,00,0,0,0,0,2,T1W ), // #167 [ref=1x]
  E(F3MAP5,00,0,0,0,0,4,ByLL), // #168 [ref=1x]
  E(F30F38,00,0,0,0,0,4,ByLL), // #169 [ref=3x]
  V(660F3A,00,0,1,0,0,0,None), // #170 [ref=6x]
  E(660F3A,00,0,0,0,0,4,None), // #171 [ref=4x]
  E(660F3A,00,0,2,0,0,5,None), // #172 [ref=4x]
  E(660F3A,00,0,0,0,1,4,None), // #173 [ref=4x]
  E(660F3A,00,0,2,0,1,5,None), // #174 [ref=4x]
  V(660F3A,00,0,0,0,0,2,None), // #175 [ref=4x]
  E(F2MAP6,00,0,0,0,0,4,ByLL), // #176 [ref=2x]
  E(F2MAP6,00,0,0,0,0,2,None), // #177 [ref=2x]
  E(660F3A,00,0,0,0,1,3,None), // #178 [ref=6x]
  E(660F3A,00,0,0,0,0,2,None), // #179 [ref=6x]
  V(660F38,00,0,0,1,1,4,ByLL), // #180 [ref=22x]
  E(66MAP6,00,0,0,0,0,4,ByLL), // #181 [ref=22x]
  V(660F38,00,0,0,1,1,3,None), // #182 [ref=12x]
  E(66MAP6,00,0,0,0,0,1,None), // #183 [ref=16x]
  E(F3MAP6,00,0,0,0,0,4,ByLL), // #184 [ref=2x]
  E(F3MAP6,00,0,0,0,0,2,None), // #185 [ref=2x]
  E(000F3A,00,0,0,0,0,1,None), // #186 [ref=4x]
  V(660F38,00,0,0,1,0,0,None), // #187 [ref=5x]
  V(660F3A,00,0,0,1,1,4,ByLL), // #188 [ref=2x]
  V(000F00,00,2,0,0,0,0,None), // #189 [ref=1x]
  V(660F00,00,0,0,0,0,2,None), // #190 [ref=1x]
  V(F20F00,00,0,0,0,1,3,DUP ), // #191 [ref=1x]
  E(660F00,00,0,0,0,0,4,ByLL), // #192 [ref=6x]
  V(F30F00,00,0,0,0,0,0,None), // #193 [ref=3x]
  E(F30F00,00,0,0,0,0,4,ByLL), // #194 [ref=1x]
  V(000F00,00,0,0,0,0,3,None), // #195 [ref=2x]
  E(66MAP5,00,0,0,0,0,1,None), // #196 [ref=1x]
  E(F20F38,00,0,0,0,1,4,ByLL), // #197 [ref=1x]
  V(660F3A,00,0,0,0,0,4,ByLL), // #198 [ref=2x]
  E(F30F38,00,0,0,0,1,0,None), // #199 [ref=5x]
  E(F30F38,00,0,0,0,0,0,None), // #200 [ref=5x]
  V(660F38,00,0,0,0,0,1,None), // #201 [ref=1x]
  V(XOP_M8,00,0,0,0,0,0,None), // #202 [ref=22x]
  V(660F38,00,0,0,0,1,4,ByLL), // #203 [ref=4x]
  E(660F38,00,0,0,0,0,0,None), // #204 [ref=2x]
  E(660F38,00,0,0,0,1,1,None), // #205 [ref=2x]
  E(660F38,00,0,0,1,1,4,ByLL), // #206 [ref=1x]
  V(660F3A,00,0,0,1,1,3,None), // #207 [ref=2x]
  V(660F3A,00,0,0,0,0,1,None), // #208 [ref=1x]
  V(660F00,00,0,0,0,0,1,None), // #209 [ref=1x]
  E(F30F38,00,0,0,0,0,2,ByLL), // #210 [ref=6x]
  E(F30F38,00,0,0,0,0,3,ByLL), // #211 [ref=9x]
  E(F30F38,00,0,0,0,0,1,ByLL), // #212 [ref=3x]
  V(660F38,00,0,0,0,0,2,ByLL), // #213 [ref=4x]
  V(660F38,00,0,0,0,0,1,ByLL), // #214 [ref=2x]
  E(660F00,00,1,0,0,0,4,ByLL), // #215 [ref=1x]
  E(660F00,00,1,0,0,1,4,ByLL), // #216 [ref=1x]
  V(F20F00,00,0,0,0,0,4,ByLL), // #217 [ref=1x]
  V(660F00,00,0,0,0,0,4,None), // #218 [ref=6x]
  V(660F00,00,7,0,0,0,4,ByLL), // #219 [ref=1x]
  V(660F00,00,0,0,0,1,4,None), // #220 [ref=2x]
  E(660F00,00,0,0,0,1,4,None), // #221 [ref=1x]
  V(660F00,00,3,0,0,0,4,ByLL), // #222 [ref=1x]
  E(F30F38,00,0,0,0,1,4,ByLL), // #223 [ref=2x]
  V(F20F38,00,0,1,0,0,0,None), // #224 [ref=3x]
  V(000F00,00,3,0,0,0,0,None), // #225 [ref=1x]
  O(F30F00,00,2,0,0,0,0,0   ), // #226 [ref=1x]
  O(F30F00,00,3,0,0,0,0,0   ), // #227 [ref=1x]
  O(000F38,00,0,0,1,0,0,0   ), // #228 [ref=1x]
  O(660F38,00,0,0,1,0,0,0   ), // #229 [ref=1x]
  O(000F00,00,5,0,1,0,0,0   ), // #230 [ref=2x]
  O(000F00,00,3,0,1,0,0,0   ), // #231 [ref=1x]
  O(000F00,00,4,0,1,0,0,0   ), // #232 [ref=2x]
  O(000F00,00,6,0,1,0,0,0   )  // #233 [ref=1x]
};
// ----------------------------------------------------------------------------
// ${MainOpcodeTable:End}

// ${AltOpcodeTable:Begin}
// ------------------- Automatically generated, do not edit -------------------
const uint32_t InstDB::alt_opcode_table[] = {
  O(000000,00,0,0,0,0,0,0   ), // #0 [ref=1512x]
  O(660F00,1B,0,0,0,0,0,0   ), // #1 [ref=1x]
  O(000F00,BA,4,0,0,0,0,0   ), // #2 [ref=1x]
  O(000F00,BA,7,0,0,0,0,0   ), // #3 [ref=1x]
  O(000F00,BA,6,0,0,0,0,0   ), // #4 [ref=1x]
  O(000F00,BA,5,0,0,0,0,0   ), // #5 [ref=1x]
  O(000000,48,0,0,0,0,0,0   ), // #6 [ref=1x]
  O(660F00,78,0,0,0,0,0,0   ), // #7 [ref=1x]
  O_FPU(00,00DF,5)           , // #8 [ref=1x]
  O_FPU(00,00DF,7)           , // #9 [ref=1x]
  O_FPU(00,00DD,1)           , // #10 [ref=1x]
  O_FPU(00,00DB,5)           , // #11 [ref=1x]
  O_FPU(00,DFE0,0)           , // #12 [ref=1x]
  O(000000,DB,7,0,0,0,0,0   ), // #13 [ref=1x]
  O_FPU(9B,DFE0,0)           , // #14 [ref=1x]
  O(000000,E4,0,0,0,0,0,0   ), // #15 [ref=1x]
  O(000000,40,0,0,0,0,0,0   ), // #16 [ref=1x]
  O(F20F00,78,0,0,0,0,0,0   ), // #17 [ref=1x]
  O(000000,72,0,0,0,0,0,0   ), // #18 [ref=1x]
  O(000000,76,0,0,0,0,0,0   ), // #19 [ref=1x]
  O(000000,E3,0,0,0,0,0,0   ), // #20 [ref=1x]
  O(000000,7C,0,0,0,0,0,0   ), // #21 [ref=1x]
  O(000000,7E,0,0,0,0,0,0   ), // #22 [ref=1x]
  O(000000,EB,0,0,0,0,0,0   ), // #23 [ref=1x]
  O(000000,73,0,0,0,0,0,0   ), // #24 [ref=1x]
  O(000000,77,0,0,0,0,0,0   ), // #25 [ref=1x]
  O(000000,7D,0,0,0,0,0,0   ), // #26 [ref=1x]
  O(000000,7F,0,0,0,0,0,0   ), // #27 [ref=1x]
  O(000000,71,0,0,0,0,0,0   ), // #28 [ref=1x]
  O(000000,7B,0,0,0,0,0,0   ), // #29 [ref=1x]
  O(000000,79,0,0,0,0,0,0   ), // #30 [ref=1x]
  O(000000,75,0,0,0,0,0,0   ), // #31 [ref=1x]
  O(000000,70,0,0,0,0,0,0   ), // #32 [ref=1x]
  O(000000,7A,0,0,0,0,0,0   ), // #33 [ref=1x]
  O(000000,78,0,0,0,0,0,0   ), // #34 [ref=1x]
  O(000000,74,0,0,0,0,0,0   ), // #35 [ref=1x]
  V(660F00,92,0,0,0,0,0,None), // #36 [ref=1x]
  V(F20F00,92,0,0,0,0,0,None), // #37 [ref=1x]
  V(F20F00,92,0,0,1,0,0,None), // #38 [ref=1x]
  V(000F00,92,0,0,0,0,0,None), // #39 [ref=1x]
  O(000000,9A,0,0,0,0,0,0   ), // #40 [ref=1x]
  O(000000,EA,0,0,0,0,0,0   ), // #41 [ref=1x]
  O(000000,E2,0,0,0,0,0,0   ), // #42 [ref=1x]
  O(000000,E1,0,0,0,0,0,0   ), // #43 [ref=1x]
  O(000000,E0,0,0,0,0,0,0   ), // #44 [ref=1x]
  O(660F00,29,0,0,0,0,0,0   ), // #45 [ref=1x]
  O(000F00,29,0,0,0,0,0,0   ), // #46 [ref=1x]
  O(000F38,F1,0,0,0,0,0,0   ), // #47 [ref=1x]
  O(000F00,7E,0,0,0,0,0,0   ), // #48 [ref=2x]
  O(660F00,7F,0,0,0,0,0,0   ), // #49 [ref=1x]
  O(F30F00,7F,0,0,0,0,0,0   ), // #50 [ref=1x]
  O(660F00,17,0,0,0,0,0,0   ), // #51 [ref=1x]
  O(000F00,17,0,0,0,0,0,0   ), // #52 [ref=1x]
  O(660F00,13,0,0,0,0,0,0   ), // #53 [ref=1x]
  O(000F00,13,0,0,0,0,0,0   ), // #54 [ref=1x]
  O(660F00,E7,0,0,0,0,0,0   ), // #55 [ref=1x]
  O(660F00,2B,0,0,0,0,0,0   ), // #56 [ref=1x]
  O(000F00,2B,0,0,0,0,0,0   ), // #57 [ref=1x]
  O(000F00,E7,0,0,0,0,0,0   ), // #58 [ref=1x]
  O(F20F00,2B,0,0,0,0,0,0   ), // #59 [ref=1x]
  O(F30F00,2B,0,0,0,0,0,0   ), // #60 [ref=1x]
  O(F20F00,11,0,0,0,0,0,0   ), // #61 [ref=1x]
  O(F30F00,11,0,0,0,0,0,0   ), // #62 [ref=1x]
  O(660F00,11,0,0,0,0,0,0   ), // #63 [ref=1x]
  O(000F00,11,0,0,0,0,0,0   ), // #64 [ref=1x]
  O(000000,E6,0,0,0,0,0,0   ), // #65 [ref=1x]
  O(000F3A,15,0,0,0,0,0,0   ), // #66 [ref=1x]
  O(000000,58,0,0,0,0,0,0   ), // #67 [ref=1x]
  O(000F00,72,6,0,0,0,0,0   ), // #68 [ref=1x]
  O(660F00,73,7,0,0,0,0,0   ), // #69 [ref=1x]
  O(000F00,73,6,0,0,0,0,0   ), // #70 [ref=1x]
  O(000F00,71,6,0,0,0,0,0   ), // #71 [ref=1x]
  O(000F00,72,4,0,0,0,0,0   ), // #72 [ref=1x]
  O(000F00,71,4,0,0,0,0,0   ), // #73 [ref=1x]
  O(000F00,72,2,0,0,0,0,0   ), // #74 [ref=1x]
  O(660F00,73,3,0,0,0,0,0   ), // #75 [ref=1x]
  O(000F00,73,2,0,0,0,0,0   ), // #76 [ref=1x]
  O(000F00,71,2,0,0,0,0,0   ), // #77 [ref=1x]
  O(000000,50,0,0,0,0,0,0   ), // #78 [ref=2x]
  O(000000,F6,0,0,0,0,0,0   ), // #79 [ref=1x]
  E(660F38,92,0,0,0,1,3,None), // #80 [ref=1x]
  E(660F38,92,0,0,0,0,2,None), // #81 [ref=1x]
  E(660F38,93,0,0,0,1,3,None), // #82 [ref=1x]
  E(660F38,93,0,0,0,0,2,None), // #83 [ref=1x]
  V(660F38,2F,0,0,0,0,0,None), // #84 [ref=1x]
  V(660F38,2E,0,0,0,0,0,None), // #85 [ref=1x]
  V(660F00,29,0,0,0,1,4,ByLL), // #86 [ref=1x]
  V(000F00,29,0,0,0,0,4,ByLL), // #87 [ref=1x]
  V(660F00,7E,0,0,0,0,2,None), // #88 [ref=1x]
  V(660F00,7F,0,0,0,0,0,None), // #89 [ref=1x]
  E(660F00,7F,0,0,0,0,4,ByLL), // #90 [ref=1x]
  E(660F00,7F,0,0,0,1,4,ByLL), // #91 [ref=1x]
  V(F30F00,7F,0,0,0,0,0,None), // #92 [ref=1x]
  E(F20F00,7F,0,0,0,1,4,ByLL), // #93 [ref=1x]
  E(F30F00,7F,0,0,0,0,4,ByLL), // #94 [ref=1x]
  E(F30F00,7F,0,0,0,1,4,ByLL), // #95 [ref=1x]
  E(F20F00,7F,0,0,0,0,4,ByLL), // #96 [ref=1x]
  V(660F00,17,0,0,0,1,3,None), // #97 [ref=1x]
  V(000F00,17,0,0,0,0,3,None), // #98 [ref=1x]
  V(660F00,13,0,0,0,1,3,None), // #99 [ref=1x]
  V(000F00,13,0,0,0,0,3,None), // #100 [ref=1x]
  V(660F00,7E,0,0,0,1,3,None), // #101 [ref=1x]
  V(F20F00,11,0,0,0,1,3,None), // #102 [ref=1x]
  E(F3MAP5,11,0,0,0,0,1,None), // #103 [ref=1x]
  V(F30F00,11,0,0,0,0,2,None), // #104 [ref=1x]
  V(660F00,11,0,0,0,1,4,ByLL), // #105 [ref=1x]
  V(000F00,11,0,0,0,0,4,ByLL), // #106 [ref=1x]
  E(66MAP5,7E,0,0,0,0,1,None), // #107 [ref=1x]
  E(660F38,7A,0,0,0,0,0,None), // #108 [ref=1x]
  E(660F38,7C,0,0,0,0,0,None), // #109 [ref=1x]
  E(660F38,7C,0,0,0,1,0,None), // #110 [ref=1x]
  E(660F38,7B,0,0,0,0,0,None), // #111 [ref=1x]
  V(660F3A,05,0,0,0,1,4,ByLL), // #112 [ref=1x]
  V(660F3A,04,0,0,0,0,4,ByLL), // #113 [ref=1x]
  V(660F3A,01,0,0,1,1,4,ByLL), // #114 [ref=1x]
  V(660F3A,00,0,0,1,1,4,ByLL), // #115 [ref=1x]
  E(660F38,90,0,0,0,0,2,None), // #116 [ref=1x]
  E(660F38,90,0,0,0,1,3,None), // #117 [ref=1x]
  E(660F38,91,0,0,0,0,2,None), // #118 [ref=1x]
  E(660F38,91,0,0,0,1,3,None), // #119 [ref=1x]
  V(660F38,8E,0,0,0,0,0,None), // #120 [ref=1x]
  V(660F38,8E,0,0,1,0,0,None), // #121 [ref=1x]
  V(XOP_M8,C0,0,0,0,0,0,None), // #122 [ref=1x]
  V(XOP_M8,C2,0,0,0,0,0,None), // #123 [ref=1x]
  V(XOP_M8,C3,0,0,0,0,0,None), // #124 [ref=1x]
  V(XOP_M8,C1,0,0,0,0,0,None), // #125 [ref=1x]
  V(660F00,72,6,0,0,0,4,ByLL), // #126 [ref=1x]
  V(660F00,73,6,0,0,1,4,ByLL), // #127 [ref=1x]
  V(660F00,71,6,0,0,0,4,ByLL), // #128 [ref=1x]
  V(660F00,72,4,0,0,0,4,ByLL), // #129 [ref=1x]
  E(660F00,72,4,0,0,1,4,ByLL), // #130 [ref=1x]
  V(660F00,71,4,0,0,0,4,ByLL), // #131 [ref=1x]
  V(660F00,72,2,0,0,0,4,ByLL), // #132 [ref=1x]
  V(660F00,73,2,0,0,1,4,ByLL), // #133 [ref=1x]
  V(660F00,71,2,0,0,0,4,ByLL)  // #134 [ref=1x]
};
// ----------------------------------------------------------------------------
// ${AltOpcodeTable:End}

#undef O
#undef V
#undef E
#undef O_FPU

// x86::InstDB - CommonInfoTable
// =============================

// ${InstCommonTable:Begin}
// ------------------- Automatically generated, do not edit -------------------
#define F(VAL) uint32_t(InstDB::InstFlags::k##VAL)
#define X(VAL) uint32_t(InstDB::Avx512Flags::k##VAL)
#define CONTROL_FLOW(VAL) uint8_t(InstControlFlow::k##VAL)
#define SAME_REG_HINT(VAL) uint8_t(InstSameRegHint::k##VAL)
const InstDB::CommonInfo InstDB::_inst_common_info_table[] = {
  { 0                                                 , 0                             , 0  , 0 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #0 [ref=1x]
  { 0                                                 , 0                             , 487, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #1 [ref=4x]
  { 0                                                 , 0                             , 488, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #2 [ref=2x]
  { 0                                                 , 0                             , 143, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #3 [ref=6x]
  { F(Lock)|F(XAcquire)|F(XRelease)                   , 0                             , 20 , 13, CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #4 [ref=2x]
  { 0                                                 , 0                             , 77 , 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #5 [ref=2x]
  { F(Vec)                                            , 0                             , 99 , 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #6 [ref=54x]
  { F(Vec)                                            , 0                             , 172, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #7 [ref=19x]
  { F(Vec)                                            , 0                             , 313, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #8 [ref=16x]
  { F(Vec)                                            , 0                             , 322, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #9 [ref=20x]
  { F(Lock)|F(XAcquire)|F(XRelease)                   , 0                             , 33 , 13, CONTROL_FLOW(Regular), SAME_REG_HINT(RO)}, // #10 [ref=1x]
  { F(Vex)                                            , 0                             , 355, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #11 [ref=3x]
  { F(Vec)                                            , 0                             , 99 , 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(RO)}, // #12 [ref=12x]
  { 0                                                 , 0                             , 489, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #13 [ref=1x]
  { F(Vex)                                            , 0                             , 357, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #14 [ref=5x]
  { F(Vex)                                            , 0                             , 77 , 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #15 [ref=12x]
  { F(Vec)                                            , 0                             , 490, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #16 [ref=4x]
  { 0                                                 , 0                             , 359, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #17 [ref=3x]
  { F(Mib)                                            , 0                             , 491, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #18 [ref=1x]
  { 0                                                 , 0                             , 492, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #19 [ref=1x]
  { 0                                                 , 0                             , 361, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #20 [ref=1x]
  { F(Mib)                                            , 0                             , 493, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #21 [ref=1x]
  { 0                                                 , 0                             , 363, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #22 [ref=1x]
  { 0                                                 , 0                             , 76 , 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #23 [ref=21x]
  { 0                                                 , 0                             , 365, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #24 [ref=3x]
  { 0                                                 , 0                             , 163, 5 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #25 [ref=1x]
  { F(Lock)|F(XAcquire)|F(XRelease)                   , 0                             , 163, 5 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #26 [ref=3x]
  { F(Rep)|F(RepIgnored)                              , 0                             , 268, 3 , CONTROL_FLOW(Call), SAME_REG_HINT(None)}, // #27 [ref=1x]
  { 0                                                 , 0                             , 494, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #28 [ref=1x]
  { 0                                                 , 0                             , 495, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #29 [ref=2x]
  { 0                                                 , 0                             , 468, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #30 [ref=1x]
  { 0                                                 , 0                             , 145, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #31 [ref=88x]
  { 0                                                 , 0                             , 496, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #32 [ref=24x]
  { 0                                                 , 0                             , 497, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #33 [ref=6x]
  { 0                                                 , 0                             , 498, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #34 [ref=14x]
  { 0                                                 , 0                             , 499, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #35 [ref=1x]
  { 0                                                 , 0                             , 46 , 13, CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #36 [ref=1x]
  { F(Vex)                                            , 0                             , 367, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #37 [ref=16x]
  { F(Rep)                                            , 0                             , 208, 4 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #38 [ref=1x]
  { F(Vec)                                            , 0                             , 500, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #39 [ref=2x]
  { F(Vec)                                            , 0                             , 501, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #40 [ref=3x]
  { F(Lock)|F(XAcquire)|F(XRelease)                   , 0                             , 212, 4 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #41 [ref=1x]
  { F(Lock)|F(XAcquire)|F(XRelease)                   , 0                             , 502, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #42 [ref=1x]
  { F(Lock)|F(XAcquire)|F(XRelease)                   , 0                             , 503, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #43 [ref=1x]
  { 0                                                 , 0                             , 504, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #44 [ref=1x]
  { 0                                                 , 0                             , 505, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #45 [ref=1x]
  { 0                                                 , 0                             , 369, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #46 [ref=1x]
  { F(Mmx)|F(Vec)                                     , 0                             , 506, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #47 [ref=2x]
  { F(Mmx)|F(Vec)                                     , 0                             , 507, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #48 [ref=2x]
  { F(Mmx)|F(Vec)                                     , 0                             , 508, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #49 [ref=2x]
  { F(Vec)                                            , 0                             , 371, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #50 [ref=2x]
  { F(Vec)                                            , 0                             , 373, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #51 [ref=2x]
  { F(Vec)                                            , 0                             , 375, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #52 [ref=2x]
  { 0                                                 , 0                             , 509, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #53 [ref=1x]
  { 0                                                 , 0                             , 510, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #54 [ref=2x]
  { F(Lock)|F(XAcquire)|F(XRelease)                   , 0                             , 271, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #55 [ref=1x]
  { 0                                                 , 0                             , 72 , 4 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #56 [ref=3x]
  { F(Mmx)                                            , 0                             , 145, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #57 [ref=1x]
  { 0                                                 , 0                             , 377, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #58 [ref=2x]
  { 0                                                 , 0                             , 511, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #59 [ref=1x]
  { F(Vec)                                            , 0                             , 512, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #60 [ref=2x]
  { F(Vec)                                            , 0                             , 379, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #61 [ref=1x]
  { F(FpuM32)|F(FpuM64)                               , 0                             , 274, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #62 [ref=6x]
  { 0                                                 , 0                             , 381, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #63 [ref=9x]
  { F(FpuM80)                                         , 0                             , 513, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #64 [ref=2x]
  { 0                                                 , 0                             , 382, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #65 [ref=13x]
  { F(FpuM32)|F(FpuM64)                               , 0                             , 383, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #66 [ref=2x]
  { F(FpuM16)|F(FpuM32)                               , 0                             , 514, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #67 [ref=9x]
  { F(FpuM16)|F(FpuM32)|F(FpuM64)                     , 0                             , 515, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #68 [ref=3x]
  { F(FpuM32)|F(FpuM64)|F(FpuM80)                     , 0                             , 516, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #69 [ref=2x]
  { F(FpuM16)                                         , 0                             , 517, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #70 [ref=3x]
  { F(FpuM16)                                         , 0                             , 518, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #71 [ref=2x]
  { F(FpuM32)|F(FpuM64)                               , 0                             , 384, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #72 [ref=1x]
  { 0                                                 , 0                             , 519, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #73 [ref=4x]
  { 0                                                 , 0                             , 520, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #74 [ref=1x]
  { 0                                                 , 0                             , 521, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #75 [ref=1x]
  { 0                                                 , 0                             , 72 , 10, CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #76 [ref=1x]
  { 0                                                 , 0                             , 522, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #77 [ref=1x]
  { F(Lock)                                           , 0                             , 271, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #78 [ref=1x]
  { 0                                                 , 0                             , 407, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #79 [ref=2x]
  { 0                                                 , 0                             , 366, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #80 [ref=3x]
  { F(Rep)                                            , 0                             , 523, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #81 [ref=1x]
  { F(Vec)                                            , 0                             , 385, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #82 [ref=1x]
  { 0                                                 , 0                             , 524, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #83 [ref=2x]
  { 0                                                 , 0                             , 525, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #84 [ref=8x]
  { 0                                                 , 0                             , 387, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #85 [ref=3x]
  { 0                                                 , 0                             , 389, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #86 [ref=1x]
  { 0                                                 , 0                             , 391, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #87 [ref=1x]
  { 0                                                 , 0                             , 145, 1 , CONTROL_FLOW(Return), SAME_REG_HINT(None)}, // #88 [ref=2x]
  { 0                                                 , 0                             , 498, 1 , CONTROL_FLOW(Return), SAME_REG_HINT(None)}, // #89 [ref=1x]
  { F(Rep)                                            , 0                             , 393, 2 , CONTROL_FLOW(Branch), SAME_REG_HINT(None)}, // #90 [ref=16x]
  { F(Rep)                                            , 0                             , 395, 2 , CONTROL_FLOW(Branch), SAME_REG_HINT(None)}, // #91 [ref=1x]
  { F(Rep)                                            , 0                             , 277, 3 , CONTROL_FLOW(Jump), SAME_REG_HINT(None)}, // #92 [ref=1x]
  { F(Vex)                                            , 0                             , 526, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #93 [ref=19x]
  { F(Vex)                                            , 0                             , 397, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #94 [ref=1x]
  { F(Vex)                                            , 0                             , 399, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #95 [ref=1x]
  { F(Vex)                                            , 0                             , 216, 4 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #96 [ref=1x]
  { F(Vex)                                            , 0                             , 401, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #97 [ref=1x]
  { F(Vex)                                            , 0                             , 527, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #98 [ref=12x]
  { F(Vex)                                            , 0                             , 528, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #99 [ref=8x]
  { F(Vex)                                            , 0                             , 526, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(WO)}, // #100 [ref=8x]
  { 0                                                 , 0                             , 529, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #101 [ref=2x]
  { 0                                                 , 0                             , 286, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #102 [ref=1x]
  { 0                                                 , 0                             , 280, 3 , CONTROL_FLOW(Call), SAME_REG_HINT(None)}, // #103 [ref=1x]
  { F(Vec)                                            , 0                             , 198, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #104 [ref=2x]
  { 0                                                 , 0                             , 530, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #105 [ref=2x]
  { 0                                                 , 0                             , 403, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #106 [ref=2x]
  { F(Vex)                                            , 0                             , 531, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #107 [ref=2x]
  { 0                                                 , 0                             , 405, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #108 [ref=1x]
  { 0                                                 , 0                             , 283, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #109 [ref=3x]
  { 0                                                 , 0                             , 280, 3 , CONTROL_FLOW(Jump), SAME_REG_HINT(None)}, // #110 [ref=1x]
  { 0                                                 , 0                             , 532, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #111 [ref=5x]
  { F(Vex)                                            , 0                             , 407, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #112 [ref=2x]
  { F(Rep)                                            , 0                             , 220, 4 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #113 [ref=1x]
  { 0                                                 , 0                             , 395, 2 , CONTROL_FLOW(Branch), SAME_REG_HINT(None)}, // #114 [ref=3x]
  { 0                                                 , 0                             , 286, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #115 [ref=1x]
  { F(Vex)                                            , 0                             , 409, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #116 [ref=2x]
  { F(Vec)                                            , 0                             , 533, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #117 [ref=1x]
  { F(Mmx)                                            , 0                             , 534, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #118 [ref=1x]
  { 0                                                 , 0                             , 535, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #119 [ref=2x]
  { F(XRelease)                                       , 0                             , 0  , 20, CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #120 [ref=1x]
  { 0                                                 , 0                             , 82 , 9 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #121 [ref=1x]
  { F(Vec)                                            , 0                             , 411, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #122 [ref=6x]
  { 0                                                 , 0                             , 139, 6 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #123 [ref=1x]
  { F(Mmx)|F(Vec)                                     , 0                             , 413, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #124 [ref=1x]
  { 0                                                 , 0                             , 415, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #125 [ref=1x]
  { F(Mmx)|F(Vec)                                     , 0                             , 536, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #126 [ref=1x]
  { F(Vec)                                            , 0                             , 380, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #127 [ref=2x]
  { F(Vec)                                            , 0                             , 107, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #128 [ref=4x]
  { F(Vec)                                            , 0                             , 537, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #129 [ref=2x]
  { F(Vec)                                            , 0                             , 101, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #130 [ref=3x]
  { F(Mmx)                                            , 0                             , 538, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #131 [ref=1x]
  { F(Vec)                                            , 0                             , 107, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #132 [ref=1x]
  { F(Vec)                                            , 0                             , 115, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #133 [ref=1x]
  { F(Mmx)|F(Vec)                                     , 0                             , 168, 5 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #134 [ref=1x]
  { F(Mmx)|F(Vec)                                     , 0                             , 539, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #135 [ref=1x]
  { F(Rep)                                            , 0                             , 224, 4 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #136 [ref=1x]
  { F(Vec)                                            , 0                             , 417, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #137 [ref=1x]
  { F(Vec)                                            , 0                             , 419, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #138 [ref=1x]
  { 0                                                 , 0                             , 289, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #139 [ref=2x]
  { 0                                                 , 0                             , 421, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #140 [ref=1x]
  { F(Vex)                                            , 0                             , 423, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #141 [ref=1x]
  { 0                                                 , 0                             , 540, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #142 [ref=1x]
  { 0                                                 , 0                             , 541, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #143 [ref=1x]
  { F(Lock)|F(XAcquire)|F(XRelease)                   , 0                             , 272, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #144 [ref=2x]
  { 0                                                 , 0                             , 145, 6 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #145 [ref=1x]
  { F(Lock)|F(XAcquire)|F(XRelease)                   , 0                             , 59 , 13, CONTROL_FLOW(Regular), SAME_REG_HINT(RO)}, // #146 [ref=1x]
  { 0                                                 , 0                             , 542, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #147 [ref=1x]
  { F(Rep)                                            , 0                             , 543, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #148 [ref=1x]
  { F(Mmx)|F(Vec)                                     , 0                             , 425, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #149 [ref=37x]
  { F(Mmx)|F(Vec)                                     , 0                             , 427, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #150 [ref=1x]
  { F(Mmx)|F(Vec)                                     , 0                             , 425, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(RO)}, // #151 [ref=6x]
  { F(Mmx)|F(Vec)                                     , 0                             , 425, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(WO)}, // #152 [ref=16x]
  { F(Mmx)                                            , 0                             , 168, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #153 [ref=26x]
  { F(Vec)                                            , 0                             , 99 , 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(WO)}, // #154 [ref=4x]
  { F(Vec)                                            , 0                             , 544, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #155 [ref=1x]
  { F(Vec)                                            , 0                             , 545, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #156 [ref=1x]
  { F(Vec)                                            , 0                             , 546, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #157 [ref=1x]
  { F(Vec)                                            , 0                             , 547, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #158 [ref=1x]
  { F(Vec)                                            , 0                             , 548, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #159 [ref=1x]
  { F(Vec)                                            , 0                             , 549, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #160 [ref=1x]
  { F(Mmx)|F(Vec)                                     , 0                             , 429, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #161 [ref=1x]
  { F(Vec)                                            , 0                             , 550, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #162 [ref=1x]
  { F(Vec)                                            , 0                             , 551, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #163 [ref=1x]
  { F(Vec)                                            , 0                             , 552, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #164 [ref=1x]
  { F(Mmx)|F(Vec)                                     , 0                             , 553, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #165 [ref=1x]
  { F(Mmx)|F(Vec)                                     , 0                             , 554, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #166 [ref=1x]
  { F(Vec)                                            , 0                             , 343, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #167 [ref=2x]
  { 0                                                 , 0                             , 173, 5 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #168 [ref=1x]
  { F(Mmx)                                            , 0                             , 427, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #169 [ref=1x]
  { F(Mmx)|F(Vec)                                     , 0                             , 431, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #170 [ref=8x]
  { F(Vec)                                            , 0                             , 555, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #171 [ref=2x]
  { 0                                                 , 0                             , 433, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #172 [ref=1x]
  { F(Mmx)|F(Vec)                                     , 0                             , 435, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #173 [ref=3x]
  { 0                                                 , 0                             , 178, 5 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #174 [ref=1x]
  { 0                                                 , 0                             , 556, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #175 [ref=1x]
  { 0                                                 , 0                             , 437, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #176 [ref=7x]
  { 0                                                 , 0                             , 557, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #177 [ref=4x]
  { F(Vex)                                            , 0                             , 439, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #178 [ref=1x]
  { 0                                                 , 0                             , 441, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #179 [ref=1x]
  { 0                                                 , 0                             , 439, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #180 [ref=7x]
  { F(Rep)|F(RepIgnored)                              , 0                             , 443, 2 , CONTROL_FLOW(Return), SAME_REG_HINT(None)}, // #181 [ref=1x]
  { 0                                                 , 0                             , 443, 2 , CONTROL_FLOW(Return), SAME_REG_HINT(None)}, // #182 [ref=1x]
  { F(Vex)                                            , 0                             , 445, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #183 [ref=1x]
  { F(Lock)|F(XAcquire)|F(XRelease)                   , 0                             , 20 , 13, CONTROL_FLOW(Regular), SAME_REG_HINT(WO)}, // #184 [ref=3x]
  { F(Rep)                                            , 0                             , 228, 4 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #185 [ref=1x]
  { 0                                                 , 0                             , 558, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #186 [ref=16x]
  { 0                                                 , 0                             , 292, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #187 [ref=2x]
  { 0                                                 , 0                             , 447, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #188 [ref=3x]
  { F(Rep)                                            , 0                             , 232, 4 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #189 [ref=1x]
  { F(Vex)                                            , 0                             , 559, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #190 [ref=8x]
  { 0                                                 , 0                             , 91 , 8 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #191 [ref=1x]
  { F(Tsib)|F(Vex)                                    , 0                             , 560, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #192 [ref=2x]
  { F(Vex)                                            , 0                             , 498, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #193 [ref=1x]
  { F(Tsib)|F(Vex)                                    , 0                             , 561, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #194 [ref=1x]
  { F(Vex)                                            , 0                             , 562, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #195 [ref=1x]
  { 0                                                 , 0                             , 563, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #196 [ref=2x]
  { 0                                                 , 0                             , 77 , 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #197 [ref=2x]
  { 0                                                 , 0                             , 449, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #198 [ref=1x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(B64)|X(ER)|X(K)|X(SAE)|X(Z) , 295, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #199 [ref=22x]
  { F(Evex)|F(Vec)                                    , X(B16)|X(ER)|X(K)|X(SAE)|X(Z) , 295, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #200 [ref=23x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(B32)|X(ER)|X(K)|X(SAE)|X(Z) , 295, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #201 [ref=22x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(ER)|X(K)|X(SAE)|X(Z)        , 564, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #202 [ref=18x]
  { F(Evex)|F(Vec)                                    , X(ER)|X(K)|X(SAE)|X(Z)        , 565, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #203 [ref=18x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(ER)|X(K)|X(SAE)|X(Z)        , 566, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #204 [ref=17x]
  { F(Vec)|F(Vex)                                     , 0                             , 295, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #205 [ref=29x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , 0                             , 295, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #206 [ref=5x]
  { F(Vec)|F(Vex)                                     , 0                             , 99 , 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #207 [ref=17x]
  { F(Vec)|F(Vex)                                     , 0                             , 322, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #208 [ref=1x]
  { F(Evex)|F(Vec)                                    , X(B32)|X(K)|X(Z)              , 298, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #209 [ref=4x]
  { F(Evex)|F(Vec)                                    , X(B64)|X(K)|X(Z)              , 298, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #210 [ref=4x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(B64)|X(K)|X(Z)              , 295, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #211 [ref=10x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(B32)|X(K)|X(Z)              , 295, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #212 [ref=12x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(B64)|X(K)|X(Z)              , 295, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(RO)}, // #213 [ref=2x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(B32)|X(K)|X(Z)              , 295, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(RO)}, // #214 [ref=6x]
  { F(Vec)|F(Vex)                                     , 0                             , 567, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #215 [ref=2x]
  { F(Evex)|F(Vec)                                    , X(B64)|X(K)|X(Z)              , 295, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #216 [ref=17x]
  { F(Evex)|F(Vec)                                    , X(B32)|X(K)|X(Z)              , 295, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #217 [ref=12x]
  { F(Vec)|F(Vex)                                     , 0                             , 298, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #218 [ref=6x]
  { F(Vec)|F(Vex)                                     , 0                             , 451, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #219 [ref=3x]
  { F(EvexTransformable)|F(Vec)|F(Vex)                , 0                             , 568, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #220 [ref=2x]
  { F(Evex)|F(Vec)                                    , X(K)|X(Z)                     , 569, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #221 [ref=1x]
  { F(Evex)|F(Vec)                                    , X(K)|X(Z)                     , 570, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #222 [ref=4x]
  { F(Evex)|F(Vec)                                    , X(K)|X(Z)                     , 571, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #223 [ref=4x]
  { F(Evex)|F(Vec)                                    , X(K)|X(Z)                     , 477, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #224 [ref=1x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(K)|X(Z)                     , 569, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #225 [ref=1x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(K)|X(Z)                     , 572, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #226 [ref=1x]
  { F(Evex)|F(EvexKReg)|F(Vec)|F(Vex)                 , X(B64)|X(ImplicitZ)|X(K)|X(SAE), 301, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #227 [ref=1x]
  { F(Evex)|F(Vec)                                    , X(B16)|X(ImplicitZ)|X(K)|X(SAE), 304, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #228 [ref=1x]
  { F(Evex)|F(EvexKReg)|F(Vec)|F(Vex)                 , X(B32)|X(ImplicitZ)|X(K)|X(SAE), 301, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #229 [ref=1x]
  { F(Evex)|F(EvexKReg)|F(Vec)|F(Vex)                 , X(ImplicitZ)|X(K)|X(SAE)      , 573, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #230 [ref=1x]
  { F(Evex)|F(Vec)                                    , X(ImplicitZ)|X(K)|X(SAE)      , 574, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #231 [ref=1x]
  { F(Evex)|F(EvexKReg)|F(Vec)|F(Vex)                 , X(ImplicitZ)|X(K)|X(SAE)      , 575, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #232 [ref=1x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(SAE)                        , 172, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #233 [ref=2x]
  { F(Evex)|F(Vec)                                    , X(SAE)                        , 343, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #234 [ref=2x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(SAE)                        , 313, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #235 [ref=2x]
  { F(Evex)|F(Vec)                                    , X(K)|X(Z)                     , 307, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #236 [ref=6x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(B32)|X(K)|X(Z)              , 310, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #237 [ref=1x]
  { F(Evex)|F(Vec)                                    , X(B32)|X(ER)|X(K)|X(SAE)|X(Z) , 453, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #238 [ref=3x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(B32)|X(ER)|X(K)|X(SAE)|X(Z) , 151, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #239 [ref=3x]
  { F(Vec)|F(Vex)                                     , 0                             , 198, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #240 [ref=5x]
  { F(Evex)|F(EvexCompat)|F(PreferEvex)|F(Vec)|F(Vex) , X(B32)|X(K)|X(Z)              , 453, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #241 [ref=1x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(B64)|X(ER)|X(K)|X(SAE)|X(Z) , 453, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #242 [ref=2x]
  { F(Evex)|F(Vec)                                    , X(B64)|X(ER)|X(K)|X(SAE)|X(Z) , 576, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #243 [ref=3x]
  { F(Evex)|F(Vec)                                    , X(B64)|X(ER)|X(K)|X(SAE)|X(Z) , 151, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #244 [ref=4x]
  { F(Evex)|F(Vec)                                    , X(B64)|X(ER)|X(K)|X(SAE)|X(Z) , 453, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #245 [ref=3x]
  { F(Evex)|F(Vec)                                    , X(B16)|X(ER)|X(K)|X(SAE)|X(Z) , 310, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #246 [ref=2x]
  { F(Evex)|F(Vec)                                    , X(B16)|X(K)|X(SAE)|X(Z)       , 313, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #247 [ref=3x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(K)|X(SAE)|X(Z)              , 310, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #248 [ref=1x]
  { F(Evex)|F(Vec)                                    , X(B16)|X(K)|X(SAE)|X(Z)       , 310, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #249 [ref=3x]
  { F(Evex)|F(Vec)                                    , X(B16)|X(ER)|X(K)|X(SAE)|X(Z) , 313, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #250 [ref=2x]
  { F(Evex)|F(Vec)                                    , X(B16)|X(ER)|X(K)|X(SAE)|X(Z) , 151, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #251 [ref=5x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(B32)|X(ER)|X(K)|X(SAE)|X(Z) , 310, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #252 [ref=1x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(K)|X(SAE)|X(Z)              , 316, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #253 [ref=1x]
  { F(Evex)|F(Vec)                                    , X(B32)|X(ER)|X(K)|X(SAE)|X(Z) , 310, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #254 [ref=3x]
  { F(Evex)|F(Vec)                                    , X(B32)|X(ER)|X(K)|X(SAE)|X(Z) , 151, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #255 [ref=2x]
  { F(Evex)|F(Vec)                                    , X(ER)|X(K)|X(SAE)|X(Z)        , 564, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #256 [ref=2x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(ER)|X(SAE)                  , 371, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #257 [ref=1x]
  { F(Evex)|F(Vec)                                    , X(ER)|X(SAE)                  , 371, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #258 [ref=1x]
  { F(Evex)|F(Vec)                                    , X(K)|X(SAE)|X(Z)              , 565, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #259 [ref=5x]
  { F(Evex)|F(Vec)                                    , X(ER)|X(SAE)                  , 455, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #260 [ref=2x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(ER)|X(SAE)                  , 457, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #261 [ref=2x]
  { F(Evex)|F(Vec)                                    , X(ER)|X(SAE)                  , 459, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #262 [ref=2x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(K)|X(SAE)|X(Z)              , 566, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #263 [ref=3x]
  { F(Evex)|F(Vec)                                    , X(ER)|X(K)|X(SAE)|X(Z)        , 566, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #264 [ref=6x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(ER)|X(SAE)                  , 375, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #265 [ref=1x]
  { F(Evex)|F(Vec)                                    , X(ER)|X(SAE)                  , 375, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #266 [ref=1x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(B64)|X(K)|X(SAE)|X(Z)       , 453, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #267 [ref=1x]
  { F(Evex)|F(Vec)                                    , X(B64)|X(K)|X(SAE)|X(Z)       , 151, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #268 [ref=3x]
  { F(Evex)|F(Vec)                                    , X(B64)|X(K)|X(SAE)|X(Z)       , 453, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #269 [ref=1x]
  { F(Evex)|F(Vec)                                    , X(B16)|X(K)|X(SAE)|X(Z)       , 151, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #270 [ref=3x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(B32)|X(K)|X(SAE)|X(Z)       , 151, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #271 [ref=1x]
  { F(Evex)|F(Vec)                                    , X(B32)|X(K)|X(SAE)|X(Z)       , 310, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #272 [ref=2x]
  { F(Evex)|F(Vec)                                    , X(B32)|X(K)|X(SAE)|X(Z)       , 151, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #273 [ref=2x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(SAE)                        , 371, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #274 [ref=1x]
  { F(Evex)|F(Vec)                                    , X(SAE)                        , 371, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #275 [ref=1x]
  { F(Evex)|F(Vec)                                    , X(SAE)                        , 455, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #276 [ref=2x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(SAE)                        , 375, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #277 [ref=1x]
  { F(Evex)|F(Vec)                                    , X(SAE)                        , 375, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #278 [ref=1x]
  { F(Evex)|F(Vec)                                    , X(ER)|X(SAE)                  , 457, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #279 [ref=2x]
  { F(Evex)|F(Vec)                                    , X(K)|X(Z)                     , 298, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #280 [ref=3x]
  { F(Vec)|F(Vex)                                     , 0                             , 298, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #281 [ref=10x]
  { F(Evex)|F(Vec)                                    , X(K)|X(Z)                     , 151, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #282 [ref=8x]
  { F(EvexTransformable)|F(Vec)|F(Vex)                , 0                             , 317, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #283 [ref=2x]
  { F(Evex)|F(Vec)                                    , X(K)|X(Z)                     , 577, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #284 [ref=4x]
  { F(Evex)|F(Vec)                                    , X(K)|X(Z)                     , 318, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #285 [ref=4x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , 0                             , 512, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #286 [ref=2x]
  { F(Evex)|F(Vec)                                    , X(B32)|X(ER)|X(K)|X(SAE)|X(Z) , 295, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #287 [ref=5x]
  { F(Evex)|F(Vec)                                    , X(B64)|X(K)|X(SAE)|X(Z)       , 298, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #288 [ref=2x]
  { F(Evex)|F(Vec)                                    , X(B32)|X(K)|X(SAE)|X(Z)       , 298, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #289 [ref=2x]
  { F(Evex)|F(Vec)                                    , X(K)|X(SAE)|X(Z)              , 578, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #290 [ref=4x]
  { F(Evex)|F(Vec)                                    , X(K)|X(SAE)|X(Z)              , 579, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #291 [ref=4x]
  { F(Vec)|F(Vex)                                     , 0                             , 236, 4 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #292 [ref=12x]
  { F(Vec)|F(Vex)                                     , 0                             , 461, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #293 [ref=4x]
  { F(Vec)|F(Vex)                                     , 0                             , 463, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #294 [ref=4x]
  { F(Evex)|F(Vec)                                    , X(B64)|X(ImplicitZ)|X(K)      , 580, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #295 [ref=1x]
  { F(Evex)|F(Vec)                                    , X(B16)|X(ImplicitZ)|X(K)      , 580, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #296 [ref=1x]
  { F(Evex)|F(Vec)                                    , X(B32)|X(ImplicitZ)|X(K)      , 580, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #297 [ref=1x]
  { F(Evex)|F(Vec)                                    , X(ImplicitZ)|X(K)             , 581, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #298 [ref=1x]
  { F(Evex)|F(Vec)                                    , X(ImplicitZ)|X(K)             , 582, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #299 [ref=1x]
  { F(Evex)|F(Vec)                                    , X(ImplicitZ)|X(K)             , 583, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #300 [ref=1x]
  { F(Vec)|F(Vex)                                     , 0                             , 99 , 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #301 [ref=7x]
  { F(Vec)|F(Vex)                                     , 0                             , 172, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #302 [ref=1x]
  { F(Vec)|F(Vex)                                     , 0                             , 313, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #303 [ref=1x]
  { F(Evex)|F(EvexTwoOp)|F(Vec)|F(Vex)|F(Vsib)        , X(K)                          , 240, 4 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #304 [ref=2x]
  { F(Evex)|F(EvexTwoOp)|F(Vec)|F(Vex)|F(Vsib)        , X(K)                          , 183, 5 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #305 [ref=2x]
  { F(Evex)|F(EvexTwoOp)|F(Vec)|F(Vex)|F(Vsib)        , X(K)                          , 188, 5 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #306 [ref=2x]
  { F(Evex)|F(EvexTwoOp)|F(Vec)|F(Vex)|F(Vsib)        , X(K)                          , 319, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #307 [ref=2x]
  { F(Evex)|F(Vec)                                    , X(K)|X(SAE)|X(Z)              , 564, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #308 [ref=1x]
  { F(Evex)|F(Vec)                                    , X(K)|X(SAE)|X(Z)              , 566, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #309 [ref=1x]
  { F(Evex)|F(Vec)                                    , X(B64)|X(K)|X(SAE)|X(Z)       , 322, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #310 [ref=2x]
  { F(Evex)|F(Vec)                                    , X(B16)|X(K)|X(SAE)|X(Z)       , 322, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #311 [ref=3x]
  { F(Evex)|F(Vec)                                    , X(B32)|X(K)|X(SAE)|X(Z)       , 322, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #312 [ref=2x]
  { F(Evex)|F(Vec)                                    , X(K)|X(SAE)|X(Z)              , 584, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #313 [ref=3x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(K)|X(Z)                     , 298, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #314 [ref=3x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(K)|X(Z)                     , 295, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #315 [ref=22x]
  { F(EvexTransformable)|F(Vec)|F(Vex)                , 0                             , 465, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #316 [ref=2x]
  { F(Evex)|F(Vec)                                    , X(K)|X(Z)                     , 465, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #317 [ref=4x]
  { F(Evex)|F(Vec)                                    , X(K)|X(Z)                     , 585, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #318 [ref=4x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , 0                             , 579, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #319 [ref=1x]
  { F(Vex)                                            , 0                             , 530, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #320 [ref=2x]
  { F(Vec)|F(Vex)                                     , 0                             , 533, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #321 [ref=1x]
  { F(Vec)|F(Vex)                                     , 0                             , 244, 4 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #322 [ref=4x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(B64)|X(K)|X(SAE)|X(Z)       , 295, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #323 [ref=2x]
  { F(Evex)|F(Vec)                                    , X(B16)|X(K)|X(SAE)|X(Z)       , 295, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #324 [ref=2x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(B32)|X(K)|X(SAE)|X(Z)       , 295, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #325 [ref=2x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(K)|X(SAE)|X(Z)              , 564, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #326 [ref=2x]
  { 0                                                 , 0                             , 467, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #327 [ref=3x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(K)|X(Z)                     , 99 , 8 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #328 [ref=4x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , 0                             , 469, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #329 [ref=1x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(K)|X(Z)                     , 325, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #330 [ref=1x]
  { F(EvexTransformable)|F(Vec)|F(Vex)                , 0                             , 99 , 4 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #331 [ref=2x]
  { F(Evex)|F(Vec)                                    , X(K)|X(Z)                     , 151, 6 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #332 [ref=6x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , 0                             , 109, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #333 [ref=2x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , 0                             , 248, 4 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #334 [ref=4x]
  { F(Vec)|F(Vex)                                     , 0                             , 586, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #335 [ref=3x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , 0                             , 193, 5 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #336 [ref=3x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , 0                             , 198, 5 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #337 [ref=1x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , 0                             , 203, 5 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #338 [ref=1x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(K)|X(Z)                     , 107, 8 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #339 [ref=1x]
  { F(Evex)|F(Vec)                                    , X(K)|X(Z)                     , 252, 4 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #340 [ref=1x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(K)|X(Z)                     , 151, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #341 [ref=4x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(K)|X(Z)                     , 115, 8 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #342 [ref=1x]
  { F(Evex)|F(Vec)                                    , X(K)|X(Z)                     , 471, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #343 [ref=1x]
  { 0                                                 , 0                             , 473, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #344 [ref=1x]
  { 0                                                 , 0                             , 475, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #345 [ref=1x]
  { F(Evex)|F(Vec)                                    , X(B32)                        , 328, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #346 [ref=1x]
  { F(Evex)|F(Vec)                                    , X(B64)                        , 328, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #347 [ref=1x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(B32)|X(K)|X(Z)              , 151, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #348 [ref=1x]
  { F(Evex)|F(Vec)                                    , X(B64)|X(K)|X(Z)              , 151, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #349 [ref=5x]
  { F(EvexTransformable)|F(Vec)|F(Vex)                , 0                             , 295, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(RO)}, // #350 [ref=2x]
  { F(Evex)|F(Vec)                                    , X(B32)|X(K)|X(Z)              , 295, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(RO)}, // #351 [ref=2x]
  { F(EvexTransformable)|F(Vec)|F(Vex)                , 0                             , 295, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(WO)}, // #352 [ref=2x]
  { F(Evex)|F(Vec)                                    , X(B32)|X(K)|X(Z)              , 295, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(WO)}, // #353 [ref=2x]
  { F(Evex)|F(Vec)                                    , X(B64)|X(K)|X(Z)              , 295, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(WO)}, // #354 [ref=2x]
  { F(Evex)|F(Vec)                                    , X(B64)|X(K)|X(Z)              , 295, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(RO)}, // #355 [ref=2x]
  { F(Evex)|F(Vec)                                    , X(K)|X(Z)                     , 295, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #356 [ref=13x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(K)|X(Z)                     , 587, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #357 [ref=1x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(K)|X(Z)                     , 588, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #358 [ref=1x]
  { F(Evex)|F(Vec)                                    , 0                             , 589, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #359 [ref=6x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(K)|X(Z)                     , 477, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #360 [ref=1x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(K)|X(Z)                     , 590, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #361 [ref=1x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , 0                             , 298, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #362 [ref=1x]
  { F(Vec)|F(Vex)                                     , 0                             , 256, 4 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #363 [ref=1x]
  { F(Evex)|F(Vec)                                    , X(ImplicitZ)|X(K)             , 304, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(WO)}, // #364 [ref=4x]
  { F(Evex)|F(Vec)                                    , X(B32)|X(ImplicitZ)|X(K)      , 304, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(WO)}, // #365 [ref=2x]
  { F(Evex)|F(EvexKReg)|F(Vec)|F(Vex)                 , X(ImplicitZ)|X(K)             , 331, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(WO)}, // #366 [ref=4x]
  { F(Evex)|F(EvexKReg)|F(Vec)|F(Vex)                 , X(B32)|X(ImplicitZ)|X(K)      , 331, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(WO)}, // #367 [ref=2x]
  { F(Evex)|F(EvexKReg)|F(Vec)|F(Vex)                 , X(B64)|X(ImplicitZ)|X(K)      , 331, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(WO)}, // #368 [ref=2x]
  { F(Vec)|F(Vex)                                     , 0                             , 544, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #369 [ref=1x]
  { F(Vec)|F(Vex)                                     , 0                             , 545, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #370 [ref=1x]
  { F(Vec)|F(Vex)                                     , 0                             , 546, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #371 [ref=1x]
  { F(Vec)|F(Vex)                                     , 0                             , 547, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #372 [ref=1x]
  { F(Evex)|F(Vec)                                    , X(B64)|X(ImplicitZ)|X(K)      , 304, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(WO)}, // #373 [ref=2x]
  { F(Evex)|F(Vec)                                    , X(B32)|X(K)|X(Z)              , 151, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #374 [ref=6x]
  { F(Evex)|F(EvexCompat)|F(PreferEvex)|F(Vec)|F(Vex) , X(B32)|X(K)|X(Z)              , 295, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #375 [ref=4x]
  { F(Vec)|F(Vex)                                     , 0                             , 299, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #376 [ref=2x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(B32)|X(K)|X(Z)              , 296, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #377 [ref=2x]
  { F(Vec)|F(Vex)                                     , 0                             , 260, 4 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #378 [ref=2x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(B64)|X(K)|X(Z)              , 123, 8 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #379 [ref=1x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(B32)|X(K)|X(Z)              , 123, 8 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #380 [ref=1x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(B64)|X(K)|X(Z)              , 264, 4 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #381 [ref=2x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , 0                             , 548, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #382 [ref=1x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , 0                             , 549, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #383 [ref=1x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , 0                             , 591, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #384 [ref=1x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(K)|X(Z)                     , 592, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #385 [ref=1x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(K)|X(Z)                     , 593, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #386 [ref=1x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(K)|X(Z)                     , 594, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #387 [ref=1x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(K)|X(Z)                     , 595, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #388 [ref=1x]
  { F(Vec)|F(Vex)                                     , 0                             , 451, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #389 [ref=12x]
  { F(Evex)|F(EvexCompat)|F(PreferEvex)|F(Vec)|F(Vex) , X(B64)|X(K)|X(Z)              , 295, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #390 [ref=2x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(K)|X(Z)                     , 295, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(RO)}, // #391 [ref=8x]
  { F(Evex)|F(Vec)                                    , 0                             , 596, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #392 [ref=4x]
  { F(Evex)|F(Vec)                                    , X(K)|X(Z)                     , 334, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #393 [ref=6x]
  { F(Evex)|F(Vec)                                    , X(K)|X(Z)                     , 337, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #394 [ref=9x]
  { F(Evex)|F(Vec)                                    , X(K)|X(Z)                     , 340, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #395 [ref=3x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(K)|X(Z)                     , 313, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #396 [ref=4x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(K)|X(Z)                     , 343, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #397 [ref=2x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(K)|X(Z)                     , 310, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #398 [ref=6x]
  { F(Vec)|F(Vex)                                     , 0                             , 256, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #399 [ref=1x]
  { F(Evex)|F(Vec)                                    , X(B32)|X(K)|X(Z)              , 322, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #400 [ref=3x]
  { F(Evex)|F(Vec)                                    , X(B64)|X(K)|X(Z)              , 322, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #401 [ref=3x]
  { F(Vec)|F(Vex)                                     , 0                             , 479, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #402 [ref=4x]
  { F(Evex)|F(Vec)|F(Vsib)                            , X(K)                          , 346, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #403 [ref=2x]
  { F(Evex)|F(Vec)|F(Vsib)                            , X(K)                          , 481, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #404 [ref=2x]
  { F(Evex)|F(Vec)|F(Vsib)                            , X(K)                          , 483, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #405 [ref=2x]
  { F(Evex)|F(Vec)|F(Vsib)                            , X(K)                          , 349, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #406 [ref=2x]
  { F(Vec)|F(Vex)                                     , 0                             , 485, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #407 [ref=8x]
  { F(Evex)|F(Vec)                                    , X(ImplicitZ)|X(K)             , 352, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #408 [ref=5x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(B32)|X(K)|X(Z)              , 322, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #409 [ref=1x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(K)|X(Z)                     , 322, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #410 [ref=2x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(B32)|X(K)|X(Z)              , 157, 6 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #411 [ref=3x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , 0                             , 322, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #412 [ref=2x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(B64)|X(K)|X(Z)              , 157, 6 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #413 [ref=2x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(K)|X(Z)                     , 157, 6 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #414 [ref=3x]
  { F(Evex)|F(Vec)                                    , X(B64)|X(K)|X(Z)              , 157, 6 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #415 [ref=1x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(K)|X(Z)                     , 295, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(WO)}, // #416 [ref=6x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(B32)|X(K)|X(Z)              , 295, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(WO)}, // #417 [ref=2x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(B64)|X(K)|X(Z)              , 295, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(WO)}, // #418 [ref=2x]
  { F(Evex)|F(Vec)                                    , X(B32)|X(ImplicitZ)|X(K)      , 352, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #419 [ref=2x]
  { F(Evex)|F(Vec)                                    , X(B64)|X(ImplicitZ)|X(K)      , 352, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #420 [ref=2x]
  { F(Evex)|F(Vec)                                    , X(K)|X(Z)                     , 564, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #421 [ref=2x]
  { F(Evex)|F(Vec)                                    , X(K)|X(Z)                     , 566, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #422 [ref=2x]
  { F(Evex)|F(Vec)                                    , X(B16)|X(K)|X(Z)              , 151, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #423 [ref=2x]
  { F(Evex)|F(Vec)                                    , X(K)|X(Z)                     , 565, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #424 [ref=2x]
  { F(Vec)|F(Vex)                                     , 0                             , 566, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #425 [ref=2x]
  { F(Evex)|F(Vec)                                    , X(K)|X(Z)                     , 578, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #426 [ref=1x]
  { F(Evex)|F(Vec)                                    , X(K)|X(Z)                     , 579, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #427 [ref=1x]
  { F(EvexTransformable)|F(Vec)|F(Vex)                , 0                             , 322, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #428 [ref=2x]
  { F(EvexTransformable)|F(Vec)|F(Vex)                , 0                             , 578, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #429 [ref=1x]
  { F(EvexTransformable)|F(Vec)|F(Vex)                , 0                             , 579, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #430 [ref=1x]
  { F(Evex)|F(Vec)                                    , X(B64)|X(ER)|X(K)|X(SAE)|X(Z) , 295, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #431 [ref=1x]
  { F(Vec)|F(Vex)                                     , 0                             , 597, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #432 [ref=1x]
  { F(Vec)|F(Vex)                                     , 0                             , 598, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #433 [ref=1x]
  { F(Vec)|F(Vex)                                     , 0                             , 599, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #434 [ref=1x]
  { F(Evex)|F(Vec)                                    , X(B32)|X(K)|X(Z)              , 299, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #435 [ref=2x]
  { F(Evex)|F(Vec)                                    , X(B64)|X(K)|X(Z)              , 299, 2 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #436 [ref=2x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(B64)|X(K)|X(Z)              , 298, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #437 [ref=1x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(B32)|X(K)|X(Z)              , 298, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #438 [ref=1x]
  { F(Vec)|F(Vex)                                     , 0                             , 295, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #439 [ref=2x]
  { F(Evex)|F(EvexCompat)|F(Vec)|F(Vex)               , X(B64)|X(ER)|X(K)|X(SAE)|X(Z) , 151, 3 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #440 [ref=1x]
  { F(Vec)|F(Vex)                                     , 0                             , 145, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #441 [ref=2x]
  { 0                                                 , 0                             , 143, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #442 [ref=2x]
  { 0                                                 , 0                             , 42 , 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #443 [ref=2x]
  { F(Lock)|F(XAcquire)|F(XRelease)                   , 0                             , 20 , 4 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #444 [ref=1x]
  { 0                                                 , 0                             , 269, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #445 [ref=1x]
  { F(XAcquire)                                       , 0                             , 131, 8 , CONTROL_FLOW(Regular), SAME_REG_HINT(RO)}, // #446 [ref=1x]
  { 0                                                 , 0                             , 600, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}, // #447 [ref=6x]
  { 0                                                 , 0                             , 601, 1 , CONTROL_FLOW(Regular), SAME_REG_HINT(None)}  // #448 [ref=6x]
};
#undef SAME_REG_HINT
#undef CONTROL_FLOW
#undef X
#undef F
// ----------------------------------------------------------------------------
// ${InstCommonTable:End}

// x86::InstDB - AdditionalInfoTable
// =================================

// ${AdditionalInfoTable:Begin}
// ------------------- Automatically generated, do not edit -------------------
#define EXT(VAL) uint32_t(CpuFeatures::X86::k##VAL)
const InstDB::AdditionalInfo InstDB::additional_info_table[] = {
  { 0, 0, { 0 } }, // #0 [ref=68x]
  { 0, 1, { 0 } }, // #1 [ref=31x]
  { 0, 0, { EXT(RAO_INT) } }, // #2 [ref=4x]
  { 0, 2, { 0 } }, // #3 [ref=2x]
  { 0, 3, { EXT(ADX) } }, // #4 [ref=1x]
  { 0, 0, { EXT(SSE2) } }, // #5 [ref=60x]
  { 0, 0, { EXT(SSE) } }, // #6 [ref=46x]
  { 0, 0, { EXT(SSE3) } }, // #7 [ref=10x]
  { 0, 4, { EXT(ADX) } }, // #8 [ref=1x]
  { 0, 0, { EXT(AESNI) } }, // #9 [ref=6x]
  { 0, 1, { EXT(BMI) } }, // #10 [ref=6x]
  { 0, 5, { 0 } }, // #11 [ref=5x]
  { 0, 0, { EXT(TBM) } }, // #12 [ref=9x]
  { 0, 0, { EXT(SSE4_1) } }, // #13 [ref=47x]
  { 0, 0, { EXT(MPX) } }, // #14 [ref=7x]
  { 0, 6, { 0 } }, // #15 [ref=4x]
  { 0, 1, { EXT(BMI2) } }, // #16 [ref=1x]
  { 0, 7, { EXT(SMAP) } }, // #17 [ref=2x]
  { 0, 8, { 0 } }, // #18 [ref=2x]
  { 0, 9, { 0 } }, // #19 [ref=2x]
  { 0, 0, { EXT(CLDEMOTE) } }, // #20 [ref=1x]
  { 0, 0, { EXT(CLFLUSH) } }, // #21 [ref=1x]
  { 0, 0, { EXT(CLFLUSHOPT) } }, // #22 [ref=1x]
  { 0, 0, { EXT(SVM) } }, // #23 [ref=6x]
  { 0, 10, { 0 } }, // #24 [ref=2x]
  { 0, 1, { EXT(CET_SS) } }, // #25 [ref=3x]
  { 0, 0, { EXT(UINTR) } }, // #26 [ref=4x]
  { 0, 0, { EXT(CLWB) } }, // #27 [ref=1x]
  { 0, 0, { EXT(CLZERO) } }, // #28 [ref=1x]
  { 0, 3, { 0 } }, // #29 [ref=1x]
  { 0, 11, { EXT(CMOV) } }, // #30 [ref=2x]
  { 0, 12, { EXT(CMOV) } }, // #31 [ref=2x]
  { 0, 13, { EXT(CMOV) } }, // #32 [ref=2x]
  { 0, 14, { EXT(CMOV) } }, // #33 [ref=2x]
  { 0, 15, { EXT(CMOV) } }, // #34 [ref=2x]
  { 0, 16, { EXT(CMOV) } }, // #35 [ref=2x]
  { 0, 17, { EXT(CMOV) } }, // #36 [ref=2x]
  { 0, 18, { EXT(CMOV) } }, // #37 [ref=2x]
  { 0, 1, { EXT(CMPCCXADD) } }, // #38 [ref=16x]
  { 0, 19, { 0 } }, // #39 [ref=2x]
  { 0, 1, { EXT(I486) } }, // #40 [ref=2x]
  { 0, 5, { EXT(CMPXCHG16B) } }, // #41 [ref=1x]
  { 0, 5, { EXT(CMPXCHG8B) } }, // #42 [ref=1x]
  { 0, 1, { EXT(SSE2) } }, // #43 [ref=2x]
  { 0, 1, { EXT(SSE) } }, // #44 [ref=2x]
  { 0, 0, { EXT(I486) } }, // #45 [ref=5x]
  { 0, 0, { EXT(SSE4_2) } }, // #46 [ref=2x]
  { 0, 20, { 0 } }, // #47 [ref=2x]
  { 0, 0, { EXT(MMX) } }, // #48 [ref=1x]
  { 0, 0, { EXT(CET_IBT) } }, // #49 [ref=2x]
  { 0, 1, { EXT(ENQCMD) } }, // #50 [ref=2x]
  { 0, 0, { EXT(SSE4A) } }, // #51 [ref=4x]
  { 0, 21, { EXT(FPU) } }, // #52 [ref=80x]
  { 0, 22, { EXT(CMOV), EXT(FPU) } }, // #53 [ref=2x]
  { 0, 23, { EXT(CMOV), EXT(FPU) } }, // #54 [ref=2x]
  { 0, 24, { EXT(CMOV), EXT(FPU) } }, // #55 [ref=2x]
  { 0, 25, { EXT(CMOV), EXT(FPU) } }, // #56 [ref=2x]
  { 0, 26, { EXT(FPU) } }, // #57 [ref=4x]
  { 0, 0, { EXT(3DNOW) } }, // #58 [ref=21x]
  { 0, 21, { EXT(SSE3), EXT(FPU) } }, // #59 [ref=1x]
  { 0, 21, { EXT(FXSR) } }, // #60 [ref=2x]
  { 0, 27, { EXT(FXSR) } }, // #61 [ref=2x]
  { 0, 0, { EXT(SMX) } }, // #62 [ref=1x]
  { 0, 0, { EXT(GFNI) } }, // #63 [ref=3x]
  { 0, 0, { EXT(HRESET) } }, // #64 [ref=1x]
  { 0, 0, { EXT(CET_SS) } }, // #65 [ref=9x]
  { 0, 15, { 0 } }, // #66 [ref=5x]
  { 0, 0, { EXT(VMX) } }, // #67 [ref=13x]
  { 0, 0, { EXT(INVLPGB) } }, // #68 [ref=2x]
  { 0, 11, { 0 } }, // #69 [ref=4x]
  { 0, 12, { 0 } }, // #70 [ref=4x]
  { 0, 13, { 0 } }, // #71 [ref=4x]
  { 0, 14, { 0 } }, // #72 [ref=4x]
  { 0, 16, { 0 } }, // #73 [ref=4x]
  { 0, 17, { 0 } }, // #74 [ref=4x]
  { 0, 18, { 0 } }, // #75 [ref=6x]
  { 0, 0, { EXT(AVX512_DQ) } }, // #76 [ref=10x]
  { 0, 0, { EXT(AVX512_BW) } }, // #77 [ref=20x]
  { 0, 0, { EXT(AVX512_F) } }, // #78 [ref=9x]
  { 1, 0, { EXT(AVX512_DQ) } }, // #79 [ref=1x]
  { 1, 0, { EXT(AVX512_BW) } }, // #80 [ref=2x]
  { 1, 0, { EXT(AVX512_F) } }, // #81 [ref=1x]
  { 0, 1, { EXT(AVX512_DQ) } }, // #82 [ref=3x]
  { 0, 1, { EXT(AVX512_BW) } }, // #83 [ref=4x]
  { 0, 1, { EXT(AVX512_F) } }, // #84 [ref=1x]
  { 0, 28, { EXT(LAHFSAHF) } }, // #85 [ref=1x]
  { 0, 0, { EXT(AMX_TILE) } }, // #86 [ref=7x]
  { 0, 0, { EXT(LWP) } }, // #87 [ref=4x]
  { 0, 29, { 0 } }, // #88 [ref=3x]
  { 0, 1, { EXT(LZCNT) } }, // #89 [ref=1x]
  { 0, 0, { EXT(MMX2) } }, // #90 [ref=3x]
  { 0, 1, { EXT(MCOMMIT) } }, // #91 [ref=1x]
  { 0, 0, { EXT(MONITOR) } }, // #92 [ref=2x]
  { 0, 0, { EXT(MONITORX) } }, // #93 [ref=2x]
  { 1, 0, { 0 } }, // #94 [ref=1x]
  { 1, 0, { EXT(SSE2) } }, // #95 [ref=5x]
  { 1, 0, { EXT(SSE) } }, // #96 [ref=3x]
  { 0, 0, { EXT(MOVBE) } }, // #97 [ref=1x]
  { 0, 0, { EXT(MMX), EXT(SSE2) } }, // #98 [ref=45x]
  { 0, 0, { EXT(MOVDIR64B) } }, // #99 [ref=1x]
  { 0, 0, { EXT(MOVDIRI) } }, // #100 [ref=1x]
  { 1, 0, { EXT(MMX), EXT(SSE2) } }, // #101 [ref=1x]
  { 0, 0, { EXT(BMI2) } }, // #102 [ref=7x]
  { 0, 0, { EXT(SSSE3) } }, // #103 [ref=16x]
  { 0, 0, { EXT(MMX2), EXT(SSE2) } }, // #104 [ref=10x]
  { 0, 0, { EXT(PCLMULQDQ) } }, // #105 [ref=1x]
  { 0, 1, { EXT(SSE4_2) } }, // #106 [ref=4x]
  { 0, 0, { EXT(PCONFIG) } }, // #107 [ref=1x]
  { 0, 0, { EXT(MMX2), EXT(SSE2), EXT(SSE4_1) } }, // #108 [ref=1x]
  { 0, 0, { EXT(3DNOW2) } }, // #109 [ref=5x]
  { 0, 0, { EXT(GEODE) } }, // #110 [ref=2x]
  { 0, 1, { EXT(POPCNT) } }, // #111 [ref=1x]
  { 0, 30, { 0 } }, // #112 [ref=3x]
  { 0, 0, { EXT(PREFETCHI) } }, // #113 [ref=2x]
  { 0, 1, { EXT(PREFETCHW) } }, // #114 [ref=1x]
  { 0, 1, { EXT(PREFETCHWT1) } }, // #115 [ref=1x]
  { 0, 20, { EXT(SEV_SNP) } }, // #116 [ref=3x]
  { 0, 1, { EXT(SSE4_1) } }, // #117 [ref=1x]
  { 0, 0, { EXT(PTWRITE) } }, // #118 [ref=1x]
  { 0, 31, { 0 } }, // #119 [ref=3x]
  { 0, 1, { EXT(SEV_SNP) } }, // #120 [ref=1x]
  { 0, 32, { 0 } }, // #121 [ref=2x]
  { 0, 0, { EXT(FSGSBASE) } }, // #122 [ref=4x]
  { 0, 0, { EXT(MSR), EXT(MSR_IMM) } }, // #123 [ref=1x]
  { 0, 0, { EXT(RDPID) } }, // #124 [ref=1x]
  { 0, 0, { EXT(OSPKE) } }, // #125 [ref=1x]
  { 0, 0, { EXT(RDPRU) } }, // #126 [ref=1x]
  { 0, 1, { EXT(RDRAND) } }, // #127 [ref=1x]
  { 0, 1, { EXT(RDSEED) } }, // #128 [ref=1x]
  { 0, 0, { EXT(RDTSC) } }, // #129 [ref=1x]
  { 0, 0, { EXT(RDTSCP) } }, // #130 [ref=1x]
  { 0, 33, { 0 } }, // #131 [ref=2x]
  { 0, 34, { EXT(LAHFSAHF) } }, // #132 [ref=1x]
  { 0, 0, { EXT(SEAM) } }, // #133 [ref=4x]
  { 0, 0, { EXT(SERIALIZE) } }, // #134 [ref=1x]
  { 0, 0, { EXT(SHA) } }, // #135 [ref=7x]
  { 0, 0, { EXT(SKINIT) } }, // #136 [ref=2x]
  { 0, 0, { EXT(AMX_COMPLEX) } }, // #137 [ref=2x]
  { 0, 0, { EXT(AMX_BF16) } }, // #138 [ref=1x]
  { 0, 0, { EXT(AMX_INT8) } }, // #139 [ref=4x]
  { 0, 0, { EXT(AMX_FP16) } }, // #140 [ref=1x]
  { 0, 1, { EXT(UINTR) } }, // #141 [ref=1x]
  { 0, 1, { EXT(WAITPKG) } }, // #142 [ref=2x]
  { 0, 0, { EXT(WAITPKG) } }, // #143 [ref=1x]
  { 0, 0, { EXT(AVX), EXT(AVX512_F), EXT(AVX512_VL) } }, // #144 [ref=71x]
  { 0, 0, { EXT(AVX512_FP16), EXT(AVX512_VL) } }, // #145 [ref=104x]
  { 0, 0, { EXT(AVX) } }, // #146 [ref=35x]
  { 0, 0, { EXT(AESNI), EXT(VAES), EXT(AVX), EXT(AVX512_F), EXT(AVX512_VL) } }, // #147 [ref=4x]
  { 0, 0, { EXT(AESNI), EXT(AVX) } }, // #148 [ref=2x]
  { 0, 0, { EXT(AVX512_F), EXT(AVX512_VL) } }, // #149 [ref=135x]
  { 0, 0, { EXT(AVX), EXT(AVX512_DQ), EXT(AVX512_VL) } }, // #150 [ref=12x]
  { 0, 0, { EXT(AVX_NE_CONVERT) } }, // #151 [ref=6x]
  { 0, 0, { EXT(AVX512_DQ), EXT(AVX512_VL) } }, // #152 [ref=42x]
  { 0, 0, { EXT(AVX2) } }, // #153 [ref=7x]
  { 0, 0, { EXT(AVX), EXT(AVX2), EXT(AVX512_F), EXT(AVX512_VL) } }, // #154 [ref=39x]
  { 0, 1, { EXT(AVX), EXT(AVX512_F), EXT(AVX512_VL) } }, // #155 [ref=4x]
  { 0, 1, { EXT(AVX512_FP16), EXT(AVX512_VL) } }, // #156 [ref=2x]
  { 0, 0, { EXT(AVX512_BF16), EXT(AVX512_VL) } }, // #157 [ref=2x]
  { 0, 0, { EXT(AVX_NE_CONVERT), EXT(AVX512_BF16), EXT(AVX512_VL) } }, // #158 [ref=1x]
  { 0, 0, { EXT(F16C), EXT(AVX512_F), EXT(AVX512_VL) } }, // #159 [ref=2x]
  { 0, 0, { EXT(AVX512_BW), EXT(AVX512_VL) } }, // #160 [ref=24x]
  { 0, 0, { EXT(FMA), EXT(AVX512_F), EXT(AVX512_VL) } }, // #161 [ref=60x]
  { 0, 0, { EXT(FMA4) } }, // #162 [ref=20x]
  { 0, 0, { EXT(XOP) } }, // #163 [ref=55x]
  { 0, 0, { EXT(AVX2), EXT(AVX512_F), EXT(AVX512_VL) } }, // #164 [ref=19x]
  { 0, 0, { EXT(GFNI), EXT(AVX), EXT(AVX512_F), EXT(AVX512_VL) } }, // #165 [ref=3x]
  { 0, 0, { EXT(SEV_ES) } }, // #166 [ref=1x]
  { 1, 0, { EXT(AVX), EXT(AVX512_F), EXT(AVX512_VL) } }, // #167 [ref=7x]
  { 1, 0, { EXT(AVX) } }, // #168 [ref=2x]
  { 1, 0, { EXT(AVX512_F), EXT(AVX512_VL) } }, // #169 [ref=4x]
  { 1, 0, { EXT(AVX512_BW), EXT(AVX512_VL) } }, // #170 [ref=2x]
  { 0, 0, { EXT(AVX), EXT(AVX2) } }, // #171 [ref=17x]
  { 0, 0, { EXT(AVX512_VL), EXT(AVX512_VP2INTERSECT) } }, // #172 [ref=2x]
  { 0, 0, { EXT(AVX), EXT(AVX2), EXT(AVX512_BW), EXT(AVX512_VL) } }, // #173 [ref=54x]
  { 0, 0, { EXT(AVX2), EXT(AVX512_BW), EXT(AVX512_VL) } }, // #174 [ref=2x]
  { 0, 0, { EXT(AVX512_CD), EXT(AVX512_VL) } }, // #175 [ref=6x]
  { 0, 0, { EXT(PCLMULQDQ), EXT(VPCLMULQDQ), EXT(AVX), EXT(AVX512_F), EXT(AVX512_VL) } }, // #176 [ref=1x]
  { 0, 1, { EXT(AVX) } }, // #177 [ref=7x]
  { 0, 0, { EXT(AVX512_VBMI2), EXT(AVX512_VL) } }, // #178 [ref=16x]
  { 0, 0, { EXT(AVX_VNNI_INT8) } }, // #179 [ref=6x]
  { 0, 0, { EXT(AVX_VNNI), EXT(AVX512_VL), EXT(AVX512_VNNI) } }, // #180 [ref=4x]
  { 0, 0, { EXT(AVX_VNNI_INT16) } }, // #181 [ref=6x]
  { 0, 0, { EXT(AVX512_VBMI), EXT(AVX512_VL) } }, // #182 [ref=4x]
  { 0, 0, { EXT(AVX), EXT(AVX512_BW), EXT(AVX512_VL) } }, // #183 [ref=4x]
  { 0, 0, { EXT(AVX_IFMA), EXT(AVX512_IFMA), EXT(AVX512_VL) } }, // #184 [ref=2x]
  { 0, 0, { EXT(AVX512_BITALG), EXT(AVX512_VL) } }, // #185 [ref=3x]
  { 0, 0, { EXT(AVX512_VL), EXT(AVX512_VPOPCNTDQ) } }, // #186 [ref=2x]
  { 0, 0, { EXT(SHA512), EXT(AVX) } }, // #187 [ref=3x]
  { 0, 0, { EXT(SM3), EXT(AVX) } }, // #188 [ref=3x]
  { 0, 0, { EXT(SM4), EXT(AVX) } }, // #189 [ref=2x]
  { 0, 0, { EXT(WBNOINVD) } }, // #190 [ref=1x]
  { 0, 0, { EXT(MSR) } }, // #191 [ref=1x]
  { 0, 0, { EXT(RTM) } }, // #192 [ref=3x]
  { 0, 0, { EXT(XSAVE) } }, // #193 [ref=6x]
  { 0, 0, { EXT(TSXLDTRK) } }, // #194 [ref=2x]
  { 0, 0, { EXT(XSAVES) } }, // #195 [ref=4x]
  { 0, 0, { EXT(XSAVEC) } }, // #196 [ref=2x]
  { 0, 0, { EXT(XSAVEOPT) } }, // #197 [ref=2x]
  { 0, 1, { EXT(RTM) } }  // #198 [ref=1x]
};
#undef EXT

#define FLAG(VAL) uint32_t(CpuRWFlags::kX86_##VAL)
const InstDB::RWFlagsInfoTable InstDB::rw_flags_info_table[] = {
  { 0, 0 }, // #0 [ref=1352x]
  { 0, FLAG(AF) | FLAG(CF) | FLAG(OF) | FLAG(PF) | FLAG(SF) | FLAG(ZF) }, // #1 [ref=103x]
  { FLAG(CF), FLAG(AF) | FLAG(CF) | FLAG(OF) | FLAG(PF) | FLAG(SF) | FLAG(ZF) }, // #2 [ref=2x]
  { FLAG(CF), FLAG(CF) }, // #3 [ref=2x]
  { FLAG(OF), FLAG(OF) }, // #4 [ref=1x]
  { 0, FLAG(ZF) }, // #5 [ref=7x]
  { 0, FLAG(AF) | FLAG(CF) | FLAG(OF) | FLAG(PF) | FLAG(SF) }, // #6 [ref=4x]
  { 0, FLAG(AC) }, // #7 [ref=2x]
  { 0, FLAG(CF) }, // #8 [ref=2x]
  { 0, FLAG(DF) }, // #9 [ref=2x]
  { 0, FLAG(IF) }, // #10 [ref=2x]
  { FLAG(CF), 0 }, // #11 [ref=6x]
  { FLAG(CF) | FLAG(ZF), 0 }, // #12 [ref=6x]
  { FLAG(OF) | FLAG(SF), 0 }, // #13 [ref=6x]
  { FLAG(OF) | FLAG(SF) | FLAG(ZF), 0 }, // #14 [ref=6x]
  { FLAG(OF), 0 }, // #15 [ref=7x]
  { FLAG(PF), 0 }, // #16 [ref=6x]
  { FLAG(SF), 0 }, // #17 [ref=6x]
  { FLAG(ZF), 0 }, // #18 [ref=8x]
  { FLAG(DF), FLAG(AF) | FLAG(CF) | FLAG(OF) | FLAG(PF) | FLAG(SF) | FLAG(ZF) }, // #19 [ref=2x]
  { 0, FLAG(AF) | FLAG(OF) | FLAG(PF) | FLAG(SF) | FLAG(ZF) }, // #20 [ref=5x]
  { 0, FLAG(C0) | FLAG(C1) | FLAG(C2) | FLAG(C3) }, // #21 [ref=83x]
  { FLAG(CF), FLAG(C0) | FLAG(C1) | FLAG(C2) | FLAG(C3) }, // #22 [ref=2x]
  { FLAG(CF) | FLAG(ZF), FLAG(C0) | FLAG(C1) | FLAG(C2) | FLAG(C3) }, // #23 [ref=2x]
  { FLAG(ZF), FLAG(C0) | FLAG(C1) | FLAG(C2) | FLAG(C3) }, // #24 [ref=2x]
  { FLAG(PF), FLAG(C0) | FLAG(C1) | FLAG(C2) | FLAG(C3) }, // #25 [ref=2x]
  { 0, FLAG(C1) | FLAG(CF) | FLAG(PF) | FLAG(ZF) }, // #26 [ref=4x]
  { FLAG(C0) | FLAG(C1) | FLAG(C2) | FLAG(C3), 0 }, // #27 [ref=2x]
  { FLAG(AF) | FLAG(CF) | FLAG(PF) | FLAG(SF) | FLAG(ZF), 0 }, // #28 [ref=1x]
  { FLAG(DF), 0 }, // #29 [ref=3x]
  { 0, FLAG(AF) | FLAG(CF) | FLAG(DF) | FLAG(IF) | FLAG(OF) | FLAG(PF) | FLAG(SF) | FLAG(ZF) }, // #30 [ref=3x]
  { FLAG(AF) | FLAG(CF) | FLAG(DF) | FLAG(IF) | FLAG(OF) | FLAG(PF) | FLAG(SF) | FLAG(ZF), 0 }, // #31 [ref=3x]
  { FLAG(CF) | FLAG(OF), FLAG(CF) | FLAG(OF) }, // #32 [ref=2x]
  { 0, FLAG(CF) | FLAG(OF) }, // #33 [ref=2x]
  { 0, FLAG(AF) | FLAG(CF) | FLAG(PF) | FLAG(SF) | FLAG(ZF) }  // #34 [ref=1x]
};
#undef FLAG

#define FLAG(VAL) uint32_t(InstRWFlags::k##VAL)
const InstRWFlags InstDB::inst_flags_table[] = {
  InstRWFlags(FLAG(None)), // #0 [ref=1619x]
  InstRWFlags(FLAG(MovOp))  // #1 [ref=29x]
};
#undef FLAG
// ----------------------------------------------------------------------------
// ${AdditionalInfoTable:End}

// Inst - NameData
// ===============

#ifndef ASMJIT_NO_TEXT
// ${NameData:Begin}
// ------------------- Automatically generated, do not edit -------------------
const InstNameIndex InstDB::_inst_name_index = {{
  { Inst::kIdAaa          , Inst::kIdAxor          + 1 },
  { Inst::kIdBextr        , Inst::kIdBzhi          + 1 },
  { Inst::kIdCall         , Inst::kIdCwde          + 1 },
  { Inst::kIdDaa          , Inst::kIdDpps          + 1 },
  { Inst::kIdEmms         , Inst::kIdExtrq         + 1 },
  { Inst::kIdF2xm1        , Inst::kIdFyl2xp1       + 1 },
  { Inst::kIdGetsec       , Inst::kIdGf2p8mulb     + 1 },
  { Inst::kIdHaddpd       , Inst::kIdHsubps        + 1 },
  { Inst::kIdIdiv         , Inst::kIdIretq         + 1 },
  { Inst::kIdJb           , Inst::kIdJz            + 1 },
  { Inst::kIdKaddb        , Inst::kIdKxorw         + 1 },
  { Inst::kIdLahf         , Inst::kIdLzcnt         + 1 },
  { Inst::kIdMaskmovdqu   , Inst::kIdMwaitx        + 1 },
  { Inst::kIdNeg          , Inst::kIdNot           + 1 },
  { Inst::kIdOr           , Inst::kIdOuts          + 1 },
  { Inst::kIdPabsb        , Inst::kIdPxor          + 1 },
  { Inst::kIdNone         , Inst::kIdNone          + 1 },
  { Inst::kIdRcl          , Inst::kIdRstorssp      + 1 },
  { Inst::kIdSahf         , Inst::kIdSysretq       + 1 },
  { Inst::kIdT1mskc       , Inst::kIdTzmsk         + 1 },
  { Inst::kIdUcomisd      , Inst::kIdUnpcklps      + 1 },
  { Inst::kIdVaddpd       , Inst::kIdVzeroupper    + 1 },
  { Inst::kIdWbinvd       , Inst::kIdWrussq        + 1 },
  { Inst::kIdXabort       , Inst::kIdXtest         + 1 },
  { Inst::kIdNone         , Inst::kIdNone          + 1 },
  { Inst::kIdNone         , Inst::kIdNone          + 1 }
}, uint16_t(17)};

const char InstDB::_inst_name_string_table[] =
  "\x63\x6D\x6F\x76\x62\x0C\x63\x6D\x6F\x76\x2E\x62\x7C\x6E\x61\x65\x7C\x63\x63\x6D\x6F\x76\x62\x65\x0A\x63\x6D\x6F\x76"
  "\x2E\x62\x65\x7C\x6E\x61\x63\x6D\x6F\x76\x6C\x0A\x63\x6D\x6F\x76\x2E\x6C\x7C\x6E\x67\x65\x63\x6D\x6F\x76\x6C\x65\x0A"
  "\x63\x6D\x6F\x76\x2E\x6C\x65\x7C\x6E\x67\x63\x6D\x6F\x76\x6E\x62\x0D\x63\x6D\x6F\x76\x2E\x6E\x62\x7C\x61\x65\x7C\x6E"
  "\x63\x63\x6D\x6F\x76\x6E\x62\x65\x0A\x63\x6D\x6F\x76\x2E\x6E\x62\x65\x7C\x61\x63\x6D\x6F\x76\x6E\x6C\x0A\x63\x6D\x6F"
  "\x76\x2E\x6E\x6C\x7C\x67\x65\x63\x6D\x6F\x76\x6E\x6C\x65\x0A\x63\x6D\x6F\x76\x2E\x6E\x6C\x65\x7C\x67\x63\x6D\x6F\x76"
  "\x6E\x70\x0A\x63\x6D\x6F\x76\x2E\x6E\x70\x7C\x70\x6F\x63\x6D\x6F\x76\x6E\x7A\x0A\x63\x6D\x6F\x76\x2E\x6E\x7A\x7C\x6E"
  "\x65\x63\x6D\x6F\x76\x70\x09\x63\x6D\x6F\x76\x2E\x70\x7C\x70\x65\x63\x6D\x6F\x76\x7A\x08\x63\x6D\x6F\x76\x2E\x7A\x7C"
  "\x65\x6A\x62\x0A\x6A\x62\x7C\x6A\x6E\x61\x65\x7C\x6A\x63\x6A\x62\x65\x07\x6A\x62\x65\x7C\x6A\x6E\x61\x6A\x6C\x07\x6A"
  "\x6C\x7C\x6A\x6E\x67\x65\x6A\x6C\x65\x07\x6A\x6C\x65\x7C\x6A\x6E\x67\x6A\x6E\x62\x0B\x6A\x6E\x62\x7C\x6A\x61\x65\x7C"
  "\x6A\x6E\x63\x6A\x6E\x62\x65\x07\x6A\x6E\x62\x65\x7C\x6A\x61\x6A\x6E\x6C\x07\x6A\x6E\x6C\x7C\x6A\x67\x65\x6A\x6E\x6C"
  "\x65\x07\x6A\x6E\x6C\x65\x7C\x6A\x67\x6A\x6E\x70\x07\x6A\x6E\x70\x7C\x6A\x70\x6F\x6A\x6E\x7A\x07\x6A\x6E\x7A\x7C\x6A"
  "\x6E\x65\x6A\x70\x06\x6A\x70\x7C\x6A\x70\x65\x6A\x7A\x05\x6A\x7A\x7C\x6A\x65\x73\x65\x74\x62\x0B\x73\x65\x74\x2E\x62"
  "\x7C\x6E\x61\x65\x7C\x63\x73\x65\x74\x62\x65\x09\x73\x65\x74\x2E\x62\x65\x7C\x6E\x61\x73\x65\x74\x6C\x09\x73\x65\x74"
  "\x2E\x6C\x7C\x6E\x67\x65\x73\x65\x74\x6C\x65\x09\x73\x65\x74\x2E\x6C\x65\x7C\x6E\x67\x73\x65\x74\x6E\x62\x0C\x73\x65"
  "\x74\x2E\x6E\x62\x7C\x61\x65\x7C\x6E\x63\x73\x65\x74\x6E\x62\x65\x09\x73\x65\x74\x2E\x6E\x62\x65\x7C\x61\x73\x65\x74"
  "\x6E\x6C\x09\x73\x65\x74\x2E\x6E\x6C\x7C\x67\x65\x73\x65\x74\x6E\x6C\x65\x09\x73\x65\x74\x2E\x6E\x6C\x65\x7C\x67\x73"
  "\x65\x74\x6E\x70\x09\x73\x65\x74\x2E\x6E\x70\x7C\x70\x6F\x73\x65\x74\x6E\x7A\x09\x73\x65\x74\x2E\x6E\x7A\x7C\x6E\x65"
  "\x73\x65\x74\x70\x08\x73\x65\x74\x2E\x70\x7C\x70\x65\x73\x65\x74\x7A\x07\x73\x65\x74\x2E\x7A\x7C\x65\x76\x67\x66\x32"
  "\x70\x38\x61\x66\x66\x69\x6E\x65\x69\x6E\x76\x71\x62\x76\x61\x65\x73\x6B\x65\x79\x67\x65\x6E\x61\x73\x73\x69\x73\x76"
  "\x62\x72\x6F\x61\x64\x63\x61\x73\x74\x66\x33\x32\x78\x34\x36\x34\x78\x32\x36\x34\x78\x34\x69\x33\x32\x78\x32\x69\x33"
  "\x32\x78\x34\x69\x33\x32\x78\x38\x69\x36\x34\x78\x32\x69\x36\x34\x78\x34\x76\x70\x62\x72\x6F\x61\x64\x63\x61\x73\x74"
  "\x6D\x62\x32\x77\x32\x64\x76\x62\x63\x73\x74\x6E\x65\x62\x66\x31\x36\x32\x70\x31\x32\x38\x69\x31\x32\x38\x76\x63\x76"
  "\x74\x6E\x65\x32\x70\x73\x32\x76\x63\x76\x74\x6E\x65\x65\x62\x66\x31\x36\x76\x63\x76\x74\x6E\x65\x6F\x62\x66\x31\x36"
  "\x76\x66\x6D\x61\x64\x64\x73\x75\x62\x31\x33\x32\x70\x68\x32\x31\x33\x70\x64\x32\x31\x33\x70\x68\x32\x31\x33\x70\x73"
  "\x32\x33\x31\x70\x64\x32\x33\x31\x70\x68\x32\x33\x31\x70\x73\x76\x66\x6D\x73\x75\x62\x61\x64\x64\x31\x33\x32\x76\x70"
  "\x6D\x75\x6C\x74\x69\x73\x68\x69\x66\x74\x76\x63\x76\x74\x6E\x65\x70\x73\x32\x76\x65\x78\x74\x72\x61\x63\x76\x65\x78"
  "\x74\x72\x61\x63\x74\x66\x76\x70\x32\x69\x6E\x74\x65\x72\x73\x65\x63\x74\x74\x63\x6D\x6D\x69\x6D\x66\x70\x31\x36\x74"
  "\x63\x6D\x6D\x72\x6C\x66\x70\x31\x36\x73\x68\x32\x70\x73\x73\x64\x70\x68\x32\x70\x73\x76\x66\x6E\x6D\x61\x64\x64\x31"
  "\x33\x32\x32\x31\x33\x73\x64\x32\x31\x33\x73\x68\x32\x31\x33\x73\x73\x32\x33\x31\x73\x64\x32\x33\x31\x73\x68\x32\x33"
  "\x31\x73\x73\x76\x66\x6E\x6D\x73\x75\x62\x31\x33\x32\x76\x69\x6E\x73\x65\x72\x76\x69\x6E\x73\x65\x72\x74\x66\x76\x70"
  "\x73\x68\x75\x66\x62\x69\x74\x71\x76\x73\x68\x61\x35\x31\x32\x72\x6E\x64\x70\x72\x65\x66\x65\x74\x63\x68\x69\x74\x30"
  "\x6E\x74\x61\x77\x74\x31\x73\x61\x76\x65\x70\x72\x65\x76\x73\x73\x73\x68\x61\x32\x35\x36\x72\x6E\x64\x74\x69\x6C\x65"
  "\x6C\x6F\x61\x64\x64\x74\x69\x6C\x65\x72\x65\x6C\x65\x76\x61\x65\x73\x64\x65\x63\x6C\x76\x61\x65\x73\x65\x6E\x63\x6C"
  "\x76\x63\x6F\x6D\x70\x72\x65\x73\x73\x76\x63\x76\x74\x74\x70\x64\x32\x75\x64\x71\x71\x76\x63\x76\x74\x74\x70\x68\x32"
  "\x75\x71\x71\x76\x63\x76\x74\x74\x70\x73\x76\x63\x76\x74\x74\x73\x64\x32\x75\x76\x63\x76\x74\x74\x73\x68\x32\x75\x76"
  "\x63\x76\x74\x74\x73\x73\x32\x75\x76\x66\x69\x78\x75\x70\x69\x6D\x6D\x76\x66\x6D\x61\x64\x64\x31\x33\x32\x76\x66\x6D"
  "\x73\x75\x62\x31\x33\x32\x76\x6D\x61\x73\x6B\x6D\x6F\x76\x64\x71\x76\x70\x63\x6F\x6D\x70\x72\x65\x73\x73\x76\x70\x63"
  "\x6F\x6E\x66\x6C\x69\x63\x74\x76\x70\x68\x6D\x69\x6E\x70\x6F\x73\x75\x76\x70\x6D\x61\x64\x64\x35\x32\x68\x6C\x75\x71"
  "\x76\x70\x73\x63\x61\x74\x74\x65\x72\x71\x64\x76\x70\x75\x6E\x70\x63\x6B\x68\x71\x6C\x71\x64\x71\x76\x72\x6E\x64\x73"
  "\x63\x61\x6C\x65\x76\x73\x63\x61\x74\x74\x65\x72\x64\x71\x70\x64\x71\x70\x73\x6D\x73\x67\x31\x6D\x73\x67\x32\x63\x6C"
  "\x66\x6C\x75\x73\x68\x6F\x70\x63\x6D\x70\x6E\x62\x65\x78\x63\x6D\x70\x6E\x6C\x65\x78\x63\x6D\x70\x78\x63\x68\x67\x31"
  "\x36\x74\x32\x74\x69\x6C\x65\x73\x74\x6F\x72\x65\x76\x63\x76\x74\x70\x64\x76\x63\x76\x74\x70\x68\x32\x70\x73\x76\x63"
  "\x76\x74\x70\x73\x32\x70\x68\x76\x63\x76\x74\x73\x64\x32\x75\x76\x63\x76\x74\x73\x68\x32\x75\x76\x63\x76\x74\x73\x73"
  "\x32\x75\x32\x64\x71\x32\x71\x71\x76\x63\x76\x74\x75\x64\x71\x32\x76\x63\x76\x74\x75\x71\x71\x32\x76\x63\x76\x74\x75"
  "\x73\x69\x32\x76\x66\x63\x6D\x61\x64\x64\x63\x76\x66\x70\x63\x6C\x61\x73\x73\x76\x67\x61\x74\x68\x65\x72\x64\x76\x67"
  "\x65\x74\x6D\x61\x6E\x6D\x75\x6C\x62\x76\x70\x63\x6C\x6D\x75\x76\x70\x63\x6D\x70\x65\x73\x74\x72\x76\x70\x63\x6D\x70"
  "\x69\x73\x74\x72\x76\x70\x65\x72\x6D\x32\x66\x76\x70\x65\x72\x6D\x69\x6C\x32\x76\x70\x67\x61\x74\x68\x65\x72\x76\x70"
  "\x6D\x61\x63\x73\x73\x64\x71\x76\x70\x6D\x61\x64\x63\x73\x73\x77\x75\x62\x73\x77\x76\x70\x6D\x61\x73\x6B\x6D\x6F\x76"
  "\x70\x74\x65\x72\x6E\x6C\x6F\x67\x62\x77\x77\x64\x6C\x62\x77\x6C\x64\x71\x6C\x77\x64\x76\x72\x73\x71\x72\x74\x31\x34"
  "\x76\x73\x68\x75\x66\x76\x73\x68\x75\x66\x66\x76\x7A\x65\x72\x6F\x75\x70\x78\x73\x61\x76\x65\x6F\x70\x74\x63\x6D\x70"
  "\x62\x65\x78\x63\x6D\x70\x6C\x65\x78\x63\x6D\x70\x6E\x62\x78\x63\x6D\x70\x6E\x6C\x78\x63\x6D\x70\x6E\x6F\x78\x63\x6D"
  "\x70\x6E\x70\x78\x63\x6D\x70\x6E\x73\x78\x63\x6D\x70\x6E\x7A\x78\x38\x62\x32\x70\x69\x66\x78\x72\x73\x74\x6F\x72\x6C"
  "\x64\x74\x69\x6C\x65\x63\x66\x6D\x6F\x76\x64\x69\x72\x36\x34\x70\x76\x61\x6C\x69\x64\x61\x72\x6D\x70\x61\x64\x6A\x75"
  "\x72\x6D\x70\x75\x70\x64\x61\x73\x65\x72\x69\x61\x6C\x69\x73\x68\x61\x31\x6E\x65\x78\x73\x68\x61\x31\x72\x6E\x64\x73"
  "\x73\x74\x74\x69\x6C\x65\x63\x66\x74\x64\x70\x62\x66\x31\x36\x74\x64\x70\x66\x70\x31\x36\x76\x61\x64\x64\x73\x75\x62"
  "\x76\x62\x6C\x65\x6E\x64\x6D\x76\x70\x64\x76\x63\x76\x74\x64\x71\x32\x75\x77\x76\x63\x76\x74\x71\x71\x32\x76\x63\x76"
  "\x74\x73\x69\x32\x76\x63\x76\x74\x75\x77\x76\x64\x62\x70\x73\x61\x64\x76\x64\x70\x62\x66\x31\x36\x76\x65\x78\x70\x61"
  "\x6E\x64\x76\x66\x63\x6D\x75\x6C\x63\x63\x70\x68\x63\x73\x68\x76\x67\x65\x74\x65\x78\x70\x76\x6D\x6F\x76\x64\x71\x61"
  "\x75\x31\x36\x75\x33\x32\x75\x36\x34\x76\x6D\x6F\x76\x6D\x73\x6B\x76\x6D\x6F\x76\x6E\x74\x76\x6D\x6F\x76\x73\x68\x64"
  "\x76\x6D\x6F\x76\x73\x6C\x64\x76\x70\x61\x63\x6B\x73\x73\x64\x77\x62\x76\x70\x61\x63\x6B\x75\x73\x77\x62\x76\x70\x62"
  "\x6C\x65\x6E\x64\x6D\x64\x76\x70\x64\x70\x62\x73\x73\x75\x64\x73\x76\x70\x64\x70\x62\x75\x73\x76\x70\x64\x70\x77\x73"
  "\x73\x76\x70\x64\x70\x77\x75\x73\x32\x70\x64\x76\x70\x65\x72\x6D\x74\x76\x70\x65\x78\x70\x61\x6E\x76\x70\x68\x61\x64"
  "\x64\x75\x62\x77\x71\x64\x71\x68\x76\x70\x6D\x6F\x76\x6D\x73\x6B\x76\x70\x6D\x6F\x76\x73\x78\x62\x76\x70\x6D\x6F\x76"
  "\x75\x73\x71\x77\x76\x70\x6D\x6F\x76\x7A\x78\x62\x76\x70\x6D\x75\x6C\x68\x72\x76\x70\x74\x65\x73\x74\x6E\x6D\x71\x76"
  "\x72\x65\x64\x75\x63\x65\x76\x73\x63\x61\x6C\x65\x66\x76\x73\x6D\x33\x72\x6E\x64\x76\x73\x6D\x34\x72\x6E\x64\x73\x76"
  "\x75\x6E\x70\x63\x6B\x68\x6C\x70\x64\x6C\x70\x73\x78\x72\x65\x73\x6C\x64\x74\x72\x73\x36\x34\x78\x73\x75\x73\x6C\x64"
  "\x74\x72\x63\x6C\x64\x65\x6D\x6F\x63\x6C\x72\x73\x73\x62\x73\x63\x6D\x70\x62\x78\x63\x6D\x70\x6C\x78\x63\x6D\x70\x6F"
  "\x78\x63\x6D\x70\x70\x78\x63\x6D\x70\x73\x78\x63\x6D\x70\x7A\x78\x63\x76\x74\x70\x69\x66\x78\x73\x61\x76\x65\x6B\x6F"
  "\x72\x74\x65\x73\x74\x77\x6B\x73\x68\x69\x66\x74\x72\x62\x6B\x75\x6E\x70\x63\x6B\x6D\x6F\x6E\x69\x74\x6F\x72\x70\x66"
  "\x72\x63\x70\x69\x70\x66\x72\x73\x71\x69\x72\x74\x76\x72\x64\x66\x73\x62\x72\x64\x67\x73\x62\x73\x73\x70\x73\x65\x61"
  "\x6D\x63\x61\x6C\x73\x65\x6E\x64\x75\x69\x73\x65\x74\x73\x73\x62\x73\x73\x79\x73\x65\x73\x79\x73\x65\x78\x75\x6D\x76"
  "\x63\x76\x74\x77\x76\x66\x6D\x75\x6C\x76\x6C\x64\x6D\x78\x63\x73\x76\x6D\x6C\x61\x75\x6E\x64\x75\x70\x75\x38\x76\x6D"
  "\x6F\x76\x68\x76\x6D\x6F\x76\x6C\x68\x76\x6D\x70\x73\x61\x64\x76\x6D\x72\x65\x73\x75\x6D\x76\x70\x61\x64\x64\x75\x76"
  "\x70\x61\x6C\x69\x67\x6E\x67\x74\x62\x67\x74\x64\x67\x74\x71\x67\x74\x77\x32\x62\x62\x64\x62\x71\x76\x70\x68\x73\x75"
  "\x62\x76\x70\x6C\x7A\x63\x6E\x62\x32\x6D\x64\x32\x6D\x71\x32\x6D\x77\x32\x6D\x76\x70\x6F\x70\x63\x6E\x76\x70\x73\x68"
  "\x6C\x64\x76\x71\x76\x70\x73\x68\x72\x64\x76\x77\x68\x77\x76\x70\x73\x75\x62\x75\x76\x72\x61\x6E\x67\x65\x76\x72\x63"
  "\x70\x31\x34\x76\x72\x6F\x75\x6E\x64\x76\x73\x6D\x34\x6B\x65\x79\x76\x73\x74\x6D\x78\x63\x73\x76\x75\x63\x6F\x6D\x69"
  "\x61\x6C\x6C\x77\x62\x6E\x6F\x69\x6E\x77\x72\x66\x73\x62\x77\x72\x67\x73\x62\x63\x36\x34\x62\x6C\x63\x66\x69\x62\x6C"
  "\x73\x66\x69\x65\x6E\x64\x62\x72\x65\x6E\x71\x63\x6D\x66\x63\x6D\x6F\x76\x6E\x75\x66\x64\x65\x63\x73\x66\x69\x6E\x63"
  "\x73\x66\x6E\x73\x74\x65\x66\x72\x6E\x64\x66\x73\x69\x6E\x63\x66\x75\x63\x6F\x6D\x66\x79\x6C\x32\x78\x69\x6E\x63\x73"
  "\x73\x70\x71\x69\x6E\x76\x6C\x69\x6E\x76\x6C\x70\x69\x6E\x76\x70\x63\x69\x6E\x76\x76\x70\x6D\x63\x6F\x6D\x6D\x6D\x6F"
  "\x76\x71\x70\x61\x76\x67\x75\x70\x66\x63\x6D\x70\x65\x70\x66\x70\x6E\x61\x70\x74\x77\x72\x69\x73\x65\x61\x6D\x6F\x73"
  "\x65\x61\x6D\x72\x73\x79\x73\x63\x73\x79\x73\x72\x65\x74\x64\x70\x62\x75\x74\x6C\x62\x73\x79\x76\x61\x65\x73\x69\x76"
  "\x61\x6C\x69\x67\x76\x61\x6E\x64\x6E\x76\x63\x6F\x6D\x69\x76\x66\x72\x63\x7A\x76\x68\x61\x64\x64\x76\x68\x73\x75\x62"
  "\x76\x6D\x63\x6C\x65\x76\x6D\x67\x65\x78\x76\x6D\x6D\x63\x76\x6D\x6F\x76\x61\x76\x6D\x6F\x76\x75\x76\x6D\x70\x74\x76"
  "\x6D\x77\x72\x69\x76\x70\x61\x6E\x64\x76\x70\x65\x78\x74\x72\x77\x76\x70\x69\x6E\x73\x76\x70\x6D\x61\x78\x76\x70\x6D"
  "\x69\x6E\x76\x70\x72\x6F\x6C\x76\x70\x72\x6F\x72\x76\x70\x73\x61\x64\x76\x70\x73\x69\x67\x76\x70\x73\x6C\x76\x70\x73"
  "\x6C\x6C\x76\x70\x73\x72\x61\x76\x70\x73\x72\x6C\x76\x73\x71\x72\x76\x74\x65\x73";


const uint32_t InstDB::_inst_name_index_table[] = {
  0x80000000, // Small ''.
  0x80000421, // Small 'aaa'.
  0x80001021, // Small 'aad'.
  0x80021021, // Small 'aadd'.
  0x80003421, // Small 'aam'.
  0x80023821, // Small 'aand'.
  0x80004C21, // Small 'aas'.
  0x80000C81, // Small 'adc'.
  0x800C0C81, // Small 'adcx'.
  0x80001081, // Small 'add'.
  0x80481081, // Small 'addpd'.
  0x81381081, // Small 'addps'.
  0x80499081, // Small 'addsd'.
  0x81399081, // Small 'addss'.
  0x22AC629E, // Large 'addsub|pd'.
  0x2282629E, // Large 'addsub|ps'.
  0x800C3C81, // Small 'adox'.
  0x86524CA1, // Small 'aesdec'.
  0x322D73AE, // Large 'aesdecl|ast'.
  0x86E2CCA1, // Small 'aesenc'.
  0x322D73B6, // Large 'aesencl|ast'.
  0x86D4CCA1, // Small 'aesimc'.
  0x1154E218, // Large 'aeskeygenassis|t'.
  0x800011C1, // Small 'and'.
  0x800711C1, // Small 'andn'.
  0x890711C1, // Small 'andnpd'.
  0xA70711C1, // Small 'andnps'.
  0x804811C1, // Small 'andpd'.
  0x813811C1, // Small 'andps'.
  0x800049E1, // Small 'aor'.
  0x80064241, // Small 'arpl'.
  0x80093F01, // Small 'axor'.
  0x812A60A2, // Small 'bextr'.
  0x28BA58CF, // Large 'blcfi|ll'.
  0x80048D82, // Small 'blci'.
  0x80348D82, // Small 'blcic'.
  0x97368D82, // Small 'blcmsk'.
  0x80098D82, // Small 'blcs'.
  0x22AC563C, // Large 'blend|pd'.
  0x2282563C, // Large 'blend|ps'.
  0x3642563C, // Large 'blend|vpd'.
  0x3364563C, // Large 'blend|vps'.
  0x28BA58D4, // Large 'blsfi|ll'.
  0x8004CD82, // Small 'blsi'.
  0x8034CD82, // Small 'blsic'.
  0x9736CD82, // Small 'blsmsk'.
  0x80094D82, // Small 'blsr'.
  0x80C191C2, // Small 'bndcl'.
  0x80E191C2, // Small 'bndcn'.
  0x815191C2, // Small 'bndcu'.
  0xB04611C2, // Small 'bndldx'.
  0x80B691C2, // Small 'bndmk'.
  0xACF691C2, // Small 'bndmov'.
  0xB14991C2, // Small 'bndstx'.
  0x804755E2, // Small 'bound'.
  0x80001A62, // Small 'bsf'.
  0x80004A62, // Small 'bsr'.
  0x8100DE62, // Small 'bswap'.
  0x80000282, // Small 'bt'.
  0x80000E82, // Small 'btc'.
  0x80004A82, // Small 'btr'.
  0x80004E82, // Small 'bts'.
  0x8004A342, // Small 'bzhi'.
  0x80063023, // Small 'call'.
  0x80005C43, // Small 'cbw'.
  0x80004483, // Small 'cdq'.
  0x8002C483, // Small 'cdqe'.
  0x80018583, // Small 'clac'.
  0x80000D83, // Small 'clc'.
  0x80001183, // Small 'cld'.
  0x22FD677C, // Large 'cldemo|te'.
  0x00007486, // Large 'clflush'.
  0x11549486, // Large 'clflushop|t'.
  0x80049D83, // Small 'clgi'.
  0x80002583, // Small 'cli'.
  0x121D7782, // Large 'clrssbs|y'.
  0x8009D183, // Small 'clts'.
  0x8004D583, // Small 'clui'.
  0x80015D83, // Small 'clwb'.
  0x9F22E983, // Small 'clzero'.
  0x80000DA3, // Small 'cmc'.
  0x0FFF5000, // Large 'cmovb' + 'cmov.b|nae|c'
  0x0FFF6012, // Large 'cmovbe' + 'cmov.be|na'
  0x0FFF5023, // Large 'cmovl' + 'cmov.l|nge'
  0x0FFF6033, // Large 'cmovle' + 'cmov.le|ng'
  0x0FFF6044, // Large 'cmovnb' + 'cmov.nb|ae|nc'
  0x0FFF7058, // Large 'cmovnbe' + 'cmov.nbe|a'
  0x0FFF606A, // Large 'cmovnl' + 'cmov.nl|ge'
  0x0FFF707B, // Large 'cmovnle' + 'cmov.nle|g'
  0x9EEB3DA3, // Small 'cmovno'.
  0x0FFF608D, // Large 'cmovnp' + 'cmov.np|po'
  0xA6EB3DA3, // Small 'cmovns'.
  0x0FFF609E, // Large 'cmovnz' + 'cmov.nz|ne'
  0x80FB3DA3, // Small 'cmovo'.
  0x0FFF50AF, // Large 'cmovp' + 'cmov.p|pe'
  0x813B3DA3, // Small 'cmovs'.
  0x0FFF50BE, // Large 'cmovz' + 'cmov.z|e'
  0x800041A3, // Small 'cmp'.
  0x329E65A7, // Large 'cmpbex|add'.
  0x329E5789, // Large 'cmpbx|add'.
  0x329E65AD, // Large 'cmplex|add'.
  0x329E578E, // Large 'cmplx|add'.
  0x329E748F, // Large 'cmpnbex|add'.
  0x329E65B3, // Large 'cmpnbx|add'.
  0x329E7496, // Large 'cmpnlex|add'.
  0x329E65B9, // Large 'cmpnlx|add'.
  0x329E65BF, // Large 'cmpnox|add'.
  0x329E65C5, // Large 'cmpnpx|add'.
  0x329E65CB, // Large 'cmpnsx|add'.
  0x329E65D1, // Large 'cmpnzx|add'.
  0x329E5793, // Large 'cmpox|add'.
  0x804841A3, // Small 'cmppd'.
  0x813841A3, // Small 'cmpps'.
  0x329E5798, // Large 'cmppx|add'.
  0x8009C1A3, // Small 'cmps'.
  0x8049C1A3, // Small 'cmpsd'.
  0x8139C1A3, // Small 'cmpss'.
  0x329E579D, // Large 'cmpsx|add'.
  0x0000749D, // Large 'cmpxchg'.
  0x1004949D, // Large 'cmpxchg16|b'.
  0x25D7749D, // Large 'cmpxchg|8b'.
  0x329E57A2, // Large 'cmpzx|add'.
  0x8934B5E3, // Small 'comisd'.
  0xA734B5E3, // Small 'comiss'.
  0x8044D603, // Small 'cpuid'.
  0x80003E23, // Small 'cqo'.
  0x81DF0E43, // Small 'crc32'.
  0x22AC6646, // Large 'cvtdq2|pd'.
  0x22826646, // Large 'cvtdq2|ps'.
  0x34E154B2, // Large 'cvtpd|2dq'.
  0x35D954B2, // Large 'cvtpd|2pi'.
  0x328154B2, // Large 'cvtpd|2ps'.
  0x36F157A7, // Large 'cvtpi|2pd'.
  0x328157A7, // Large 'cvtpi|2ps'.
  0x23CF64C1, // Large 'cvtps2|dq'.
  0x122B74C1, // Large 'cvtps2p|d'.
  0x120F74C1, // Large 'cvtps2p|i'.
  0x222364CA, // Large 'cvtsd2|si'.
  0x222264CA, // Large 'cvtsd2|ss'.
  0x231D6656, // Large 'cvtsi2|sd'.
  0x22226656, // Large 'cvtsi2|ss'.
  0x231D64DA, // Large 'cvtss2|sd'.
  0x222364DA, // Large 'cvtss2|si'.
  0x23CF73C7, // Large 'cvttpd2|dq'.
  0x240473C7, // Large 'cvttpd2|pi'.
  0x34E163DE, // Large 'cvttps|2dq'.
  0x35D963DE, // Large 'cvttps|2pi'.
  0x222373E5, // Large 'cvttsd2|si'.
  0x222373F7, // Large 'cvttss2|si'.
  0x800012E3, // Small 'cwd'.
  0x800292E3, // Small 'cwde'.
  0x80000424, // Small 'daa'.
  0x80004C24, // Small 'das'.
  0x80000CA4, // Small 'dec'.
  0x80005924, // Small 'div'.
  0x80485924, // Small 'divpd'.
  0x81385924, // Small 'divps'.
  0x8049D924, // Small 'divsd'.
  0x8139D924, // Small 'divss'.
  0x80024204, // Small 'dppd'.
  0x8009C204, // Small 'dpps'.
  0x8009B5A5, // Small 'emms'.
  0x223158D9, // Large 'endbr|32'.
  0x223558D9, // Large 'endbr|64'.
  0x88D1C5C5, // Small 'enqcmd'.
  0x22A058DE, // Large 'enqcm|ds'.
  0x8122D1C5, // Small 'enter'.
  0x228272F0, // Large 'extract|ps'.
  0x81195305, // Small 'extrq'.
  0x81C6E3A6, // Small 'f2xm1'.
  0x80098826, // Small 'fabs'.
  0x80021026, // Small 'fadd'.
  0x81021026, // Small 'faddp'.
  0x80023046, // Small 'fbld'.
  0x810A4C46, // Small 'fbstp'.
  0x8009A066, // Small 'fchs'.
  0x8182B066, // Small 'fclex'.
  0x8567B466, // Small 'fcmovb'.
  0x40143500, // Large 'fcm|ovbe'.
  0x8B67B466, // Small 'fcmove'.
  0x40463500, // Large 'fcm|ovnb'.
  0x505A3500, // Large 'fcm|ovnbe'.
  0x20AD58E3, // Large 'fcmov|ne'.
  0x28E858E3, // Large 'fcmov|nu'.
  0xAB67B466, // Small 'fcmovu'.
  0x8006BC66, // Small 'fcom'.
  0x8096BC66, // Small 'fcomi'.
  0xA096BC66, // Small 'fcomip'.
  0x8106BC66, // Small 'fcomp'.
  0xA106BC66, // Small 'fcompp'.
  0x8009BC66, // Small 'fcos'.
  0x21EF58EA, // Large 'fdecs|tp'.
  0x800B2486, // Small 'fdiv'.
  0x810B2486, // Small 'fdivp'.
  0x812B2486, // Small 'fdivr'.
  0xA12B2486, // Small 'fdivrp'.
  0x8136B4A6, // Small 'femms'.
  0x8052C8C6, // Small 'ffree'.
  0x80420526, // Small 'fiadd'.
  0x80D78D26, // Small 'ficom'.
  0xA0D78D26, // Small 'ficomp'.
  0x81649126, // Small 'fidiv'.
  0xA5649126, // Small 'fidivr'.
  0x80023126, // Small 'fild'.
  0x80CAB526, // Small 'fimul'.
  0x21EF58EF, // Large 'fincs|tp'.
  0x8144B926, // Small 'finit'.
  0x800A4D26, // Small 'fist'.
  0x810A4D26, // Small 'fistp'.
  0xA14A4D26, // Small 'fisttp'.
  0x802ACD26, // Small 'fisub'.
  0xA42ACD26, // Small 'fisubr'.
  0x80001186, // Small 'fld'.
  0x800E1186, // Small 'fld1'.
  0x81719186, // Small 'fldcw'.
  0xACE29186, // Small 'fldenv'.
  0x8BD61186, // Small 'fldl2e'.
  0xA9D61186, // Small 'fldl2t'.
  0xBA761186, // Small 'fldlg2'.
  0xBAE61186, // Small 'fldln2'.
  0x80981186, // Small 'fldpi'.
  0x800D1186, // Small 'fldz'.
  0x800655A6, // Small 'fmul'.
  0x810655A6, // Small 'fmulp'.
  0xB0560DC6, // Small 'fnclex'.
  0xA89725C6, // Small 'fninit'.
  0x80083DC6, // Small 'fnop'.
  0x8B60CDC6, // Small 'fnsave'.
  0xAE3A4DC6, // Small 'fnstcw'.
  0x221358F4, // Large 'fnste|nv'.
  0xAF3A4DC6, // Small 'fnstsw'.
  0x9C1A0606, // Small 'fpatan'.
  0x80D2CA06, // Small 'fprem'.
  0xB8D2CA06, // Small 'fprem1'.
  0x80E0D206, // Small 'fptan'.
  0x32FB48F9, // Large 'frnd|int'.
  0xA4FA4E46, // Small 'frstor'.
  0x805B0666, // Small 'fsave'.
  0x8AC08E66, // Small 'fscale'.
  0x80072666, // Small 'fsin'.
  0x21DD58FD, // Large 'fsinc|os'.
  0x81494666, // Small 'fsqrt'.
  0x80005266, // Small 'fst'.
  0x8171D266, // Small 'fstcw'.
  0xACE2D266, // Small 'fstenv'.
  0x80085266, // Small 'fstp'.
  0x8179D266, // Small 'fstsw'.
  0x80015666, // Small 'fsub'.
  0x81015666, // Small 'fsubp'.
  0x81215666, // Small 'fsubr'.
  0xA1215666, // Small 'fsubrp'.
  0x800A4E86, // Small 'ftst'.
  0x80D78EA6, // Small 'fucom'.
  0x92D78EA6, // Small 'fucomi'.
  0x27D45902, // Large 'fucom|ip'.
  0xA0D78EA6, // Small 'fucomp'.
  0x279A5902, // Large 'fucom|pp'.
  0x814486E6, // Small 'fwait'.
  0x80068706, // Small 'fxam'.
  0x80040F06, // Small 'fxch'.
  0x000075DC, // Large 'fxrstor'.
  0x223575DC, // Large 'fxrstor|64'.
  0x8B60CF06, // Small 'fxsave'.
  0x223567AC, // Large 'fxsave|64'.
  0x52F225DC, // Large 'fx|tract'.
  0x818EB326, // Small 'fyl2x'.
  0x22735907, // Large 'fyl2x|p1'.
  0x8659D0A7, // Small 'getsec'.
  0x1004F207, // Large 'gf2p8affineinvq|b'.
  0x2215B207, // Large 'gf2p8affine|qb'.
  0x451E5207, // Large 'gf2p8|mulb'.
  0x89021028, // Small 'haddpd'.
  0xA7021028, // Small 'haddps'.
  0x80005188, // Small 'hlt'.
  0xA8599648, // Small 'hreset'.
  0x89015668, // Small 'hsubpd'.
  0xA7015668, // Small 'hsubps'.
  0x800B2489, // Small 'idiv'.
  0x800655A9, // Small 'imul'.
  0x800001C9, // Small 'in'.
  0x80000DC9, // Small 'inc'.
  0x22AC590C, // Large 'incss|pd'.
  0x2911590C, // Large 'incss|pq'.
  0x80004DC9, // Small 'ins'.
  0x2282635D, // Large 'insert|ps'.
  0x1215635D, // Large 'insert|q'.
  0x800051C9, // Small 'int'.
  0x800F51C9, // Small 'int3'.
  0x8007D1C9, // Small 'into'.
  0x800259C9, // Small 'invd'.
  0xA902D9C9, // Small 'invept'.
  0x8F0659C9, // Small 'invlpg'.
  0x354A4913, // Large 'invl|pga'.
  0x25775917, // Large 'invlp|gb'.
  0x25F7591C, // Large 'invpc|id'.
  0x25F75921, // Large 'invvp|id'.
  0x800A1649, // Small 'iret'.
  0x804A1649, // Small 'iretd'.
  0x811A1649, // Small 'iretq'.
  0x0FFF20CC, // Large 'jb' + 'jb|jnae|jc'
  0x0FFF30D9, // Large 'jbe' + 'jbe|jna'
  0x81AC0CAA, // Small 'jecxz'.
  0x0FFF20E4, // Large 'jl' + 'jl|jnge'
  0x0FFF30EE, // Large 'jle' + 'jle|jng'
  0x800041AA, // Small 'jmp'.
  0x0FFF30F9, // Large 'jnb' + 'jnb|jae|jnc'
  0x0FFF4108, // Large 'jnbe' + 'jnbe|ja'
  0x0FFF3114, // Large 'jnl' + 'jnl|jge'
  0x0FFF411F, // Large 'jnle' + 'jnle|jg'
  0x80003DCA, // Small 'jno'.
  0x0FFF312B, // Large 'jnp' + 'jnp|jpo'
  0x80004DCA, // Small 'jns'.
  0x0FFF3136, // Large 'jnz' + 'jnz|jne'
  0x800001EA, // Small 'jo'.
  0x0FFF2141, // Large 'jp' + 'jp|jpe'
  0x8000026A, // Small 'js'.
  0x0FFF214A, // Large 'jz' + 'jz|je'
  0x8022102B, // Small 'kaddb'.
  0x8042102B, // Small 'kaddd'.
  0x8112102B, // Small 'kaddq'.
  0x8172102B, // Small 'kaddw'.
  0x8022382B, // Small 'kandb'.
  0x8042382B, // Small 'kandd'.
  0x84E2382B, // Small 'kandnb'.
  0x88E2382B, // Small 'kandnd'.
  0xA2E2382B, // Small 'kandnq'.
  0xAEE2382B, // Small 'kandnw'.
  0x8112382B, // Small 'kandq'.
  0x8172382B, // Small 'kandw'.
  0x802B3DAB, // Small 'kmovb'.
  0x804B3DAB, // Small 'kmovd'.
  0x811B3DAB, // Small 'kmovq'.
  0x817B3DAB, // Small 'kmovw'.
  0x802A3DCB, // Small 'knotb'.
  0x804A3DCB, // Small 'knotd'.
  0x811A3DCB, // Small 'knotq'.
  0x817A3DCB, // Small 'knotw'.
  0x800149EB, // Small 'korb'.
  0x800249EB, // Small 'kord'.
  0x8008C9EB, // Small 'korq'.
  0x215467B2, // Large 'kortes|tb'.
  0x262667B2, // Large 'kortes|td'.
  0x236C67B2, // Large 'kortes|tq'.
  0x27B867B2, // Large 'kortes|tw'.
  0x800BC9EB, // Small 'korw'.
  0x252067BA, // Large 'kshift|lb'.
  0x257F67BA, // Large 'kshift|ld'.
  0x246267BA, // Large 'kshift|lq'.
  0x258267BA, // Large 'kshift|lw'.
  0x27C067BA, // Large 'kshift|rb'.
  0x122B77BA, // Large 'kshiftr|d'.
  0x121577BA, // Large 'kshiftr|q'.
  0x126477BA, // Large 'kshiftr|w'.
  0x8549968B, // Small 'ktestb'.
  0x8949968B, // Small 'ktestd'.
  0xA349968B, // Small 'ktestq'.
  0xAF49968B, // Small 'ktestw'.
  0x257867C2, // Large 'kunpck|bw'.
  0x23CF67C2, // Large 'kunpck|dq'.
  0x257A67C2, // Large 'kunpck|wd'.
  0x8527BB0B, // Small 'kxnorb'.
  0x8927BB0B, // Small 'kxnord'.
  0xA327BB0B, // Small 'kxnorq'.
  0xAF27BB0B, // Small 'kxnorw'.
  0x80293F0B, // Small 'kxorb'.
  0x80493F0B, // Small 'kxord'.
  0x81193F0B, // Small 'kxorq'.
  0x81793F0B, // Small 'kxorw'.
  0x8003202C, // Small 'lahf'.
  0x8000482C, // Small 'lar'.
  0x80C6046C, // Small 'lcall'.
  0x8158908C, // Small 'lddqu'.
  0x12286815, // Large 'ldmxcs|r'.
  0x80004C8C, // Small 'lds'.
  0x103185E3, // Large 'ldtilecf|g'.
  0x800004AC, // Small 'lea'.
  0x805B04AC, // Small 'leave'.
  0x80004CAC, // Small 'les'.
  0x8A3714CC, // Small 'lfence'.
  0x80004CCC, // Small 'lfs'.
  0x800A10EC, // Small 'lgdt'.
  0x80004CEC, // Small 'lgs'.
  0x800A112C, // Small 'lidt'.
  0x8008354C, // Small 'ljmp'.
  0x800A118C, // Small 'lldt'.
  0x84385D8C, // Small 'llwpcb'.
  0x800BCDAC, // Small 'lmsw'.
  0x800991EC, // Small 'lods'.
  0x80083DEC, // Small 'loop'.
  0x80583DEC, // Small 'loope'.
  0x8AE83DEC, // Small 'loopne'.
  0x8000326C, // Small 'lsl'.
  0x80004E6C, // Small 'lss'.
  0x80004A8C, // Small 'ltr'.
  0xA6E4C2EC, // Small 'lwpins'.
  0x981B42EC, // Small 'lwpval'.
  0x81470F4C, // Small 'lzcnt'.
  0x12A2941B, // Large 'maskmovdq|u'.
  0x1215741B, // Large 'maskmov|q'.
  0x8048602D, // Small 'maxpd'.
  0x8138602D, // Small 'maxps'.
  0x8049E02D, // Small 'maxsd'.
  0x8139E02D, // Small 'maxss'.
  0x236B5926, // Large 'mcomm|it'.
  0x8A3714CD, // Small 'mfence'.
  0x8048392D, // Small 'minpd'.
  0x8138392D, // Small 'minps'.
  0x8049B92D, // Small 'minsd'.
  0x8139B92D, // Small 'minss'.
  0x000077C8, // Large 'monitor'.
  0x123377C8, // Large 'monitor|x'.
  0x800059ED, // Small 'mov'.
  0xA620D9ED, // Small 'movabs'.
  0x8900D9ED, // Small 'movapd'.
  0xA700D9ED, // Small 'movaps'.
  0x805159ED, // Small 'movbe'.
  0x800259ED, // Small 'movd'.
  0x3821441F, // Large 'movd|dup'.
  0x100485EB, // Large 'movdir64|b'.
  0x120F65EB, // Large 'movdir|i'.
  0x24E4541F, // Large 'movdq|2q'.
  0x831259ED, // Small 'movdqa'.
  0xAB1259ED, // Small 'movdqu'.
  0x37664827, // Large 'movh|lps'.
  0x890459ED, // Small 'movhpd'.
  0xA70459ED, // Small 'movhps'.
  0x2282582C, // Large 'movlh|ps'.
  0x890659ED, // Small 'movlpd'.
  0xA70659ED, // Small 'movlps'.
  0x22AC669C, // Large 'movmsk|pd'.
  0x2282669C, // Large 'movmsk|ps'.
  0x23CF56A3, // Large 'movnt|dq'.
  0x368F56A3, // Large 'movnt|dqa'.
  0x934759ED, // Small 'movnti'.
  0x22AC56A3, // Large 'movnt|pd'.
  0x228256A3, // Large 'movnt|ps'.
  0xA34759ED, // Small 'movntq'.
  0x231D56A3, // Large 'movnt|sd'.
  0x222256A3, // Large 'movnt|ss'.
  0x8008D9ED, // Small 'movq'.
  0x34E1492B, // Large 'movq|2dq'.
  0x8009D9ED, // Small 'movs'.
  0x8049D9ED, // Small 'movsd'.
  0x240366A9, // Large 'movshd|up'.
  0x240366B0, // Large 'movsld|up'.
  0x8139D9ED, // Small 'movss'.
  0x8189D9ED, // Small 'movsx'.
  0x8989D9ED, // Small 'movsxd'.
  0x890AD9ED, // Small 'movupd'.
  0xA70AD9ED, // Small 'movups'.
  0x818D59ED, // Small 'movzx'.
  0x25785832, // Large 'mpsad|bw'.
  0x800032AD, // Small 'mul'.
  0x804832AD, // Small 'mulpd'.
  0x813832AD, // Small 'mulps'.
  0x8049B2AD, // Small 'mulsd'.
  0x8139B2AD, // Small 'mulss'.
  0x800C32AD, // Small 'mulx'.
  0x814486ED, // Small 'mwait'.
  0xB14486ED, // Small 'mwaitx'.
  0x80001CAE, // Small 'neg'.
  0x800041EE, // Small 'nop'.
  0x800051EE, // Small 'not'.
  0x8000024F, // Small 'or'.
  0x8002424F, // Small 'orpd'.
  0x8009C24F, // Small 'orps'.
  0x800052AF, // Small 'out'.
  0x8009D2AF, // Small 'outs'.
  0x80298830, // Small 'pabsb'.
  0x80498830, // Small 'pabsd'.
  0x81798830, // Small 'pabsw'.
  0x000086B7, // Large 'packssdw'.
  0x26BE66B7, // Large 'packss|wb'.
  0x26BD66C1, // Large 'packus|dw'.
  0x000086C1, // Large 'packuswb'.
  0x80221030, // Small 'paddb'.
  0x80421030, // Small 'paddd'.
  0x81121030, // Small 'paddq'.
  0x85321030, // Small 'paddsb'.
  0xAF321030, // Small 'paddsw'.
  0x2786583F, // Large 'paddu|sb'.
  0x2561583F, // Large 'paddu|sw'.
  0x81721030, // Small 'paddw'.
  0x12286845, // Large 'palign|r'.
  0x80023830, // Small 'pand'.
  0x80E23830, // Small 'pandn'.
  0x8059D430, // Small 'pause'.
  0x8023D830, // Small 'pavgb'.
  0x2786592F, // Large 'pavgu|sb'.
  0x8173D830, // Small 'pavgw'.
  0x200366CA, // Large 'pblend|vb'.
  0x126466CA, // Large 'pblend|w'.
  0x44625523, // Large 'pclmu|lqdq'.
  0x22155529, // Large 'pcmpe|qb'.
  0x24575529, // Large 'pcmpe|qd'.
  0x23D05529, // Large 'pcmpe|qq'.
  0x27255529, // Large 'pcmpe|qw'.
  0x120F8529, // Large 'pcmpestr|i'.
  0x10018529, // Large 'pcmpestr|m'.
  0x384B448E, // Large 'pcmp|gtb'.
  0x384E448E, // Large 'pcmp|gtd'.
  0x3851448E, // Large 'pcmp|gtq'.
  0x3854448E, // Large 'pcmp|gtw'.
  0x120F8532, // Large 'pcmpistr|i'.
  0x10018532, // Large 'pcmpistr|m'.
  0x2848542F, // Large 'pconf|ig'.
  0x80081490, // Small 'pdep'.
  0x800A60B0, // Small 'pext'.
  0x852A60B0, // Small 'pextrb'.
  0x892A60B0, // Small 'pextrd'.
  0xA32A60B0, // Small 'pextrq'.
  0xAF2A60B0, // Small 'pextrw'.
  0x8044F4D0, // Small 'pf2id'.
  0x8174F4D0, // Small 'pf2iw'.
  0x803184D0, // Small 'pfacc'.
  0x804204D0, // Small 'pfadd'.
  0x12156934, // Large 'pfcmpe|q'.
  0x20315934, // Large 'pfcmp|ge'.
  0x284B5934, // Large 'pfcmp|gt'.
  0x8180B4D0, // Small 'pfmax'.
  0x80E4B4D0, // Small 'pfmin'.
  0x80CAB4D0, // Small 'pfmul'.
  0x8630B8D0, // Small 'pfnacc'.
  0x2011593A, // Large 'pfpna|cc'.
  0x8101C8D0, // Small 'pfrcp'.
  0x238767CF, // Large 'pfrcpi|t1'.
  0x24A667CF, // Large 'pfrcpi|t2'.
  0xAD01C8D0, // Small 'pfrcpv'.
  0x238767D5, // Large 'pfrsqi|t1'.
  0x236157D5, // Large 'pfrsq|rt'.
  0x37DB57D5, // Large 'pfrsq|rtv'.
  0x802ACCD0, // Small 'pfsub'.
  0xA42ACCD0, // Small 'pfsubr'.
  0x88420510, // Small 'phaddd'.
  0x25615702, // Large 'phadd|sw'.
  0xAE420510, // Small 'phaddw'.
  0x12649439, // Large 'phminposu|w'.
  0x882ACD10, // Small 'phsubd'.
  0x2561585E, // Large 'phsub|sw'.
  0xAE2ACD10, // Small 'phsubw'.
  0x80437530, // Small 'pi2fd'.
  0x81737530, // Small 'pi2fw'.
  0x8529B930, // Small 'pinsrb'.
  0x8929B930, // Small 'pinsrd'.
  0xA329B930, // Small 'pinsrq'.
  0xAF29B930, // Small 'pinsrw'.
  0x45635443, // Large 'pmadd|ubsw'.
  0x257A5443, // Large 'pmadd|wd'.
  0x853C05B0, // Small 'pmaxsb'.
  0x893C05B0, // Small 'pmaxsd'.
  0xAF3C05B0, // Small 'pmaxsw'.
  0x855C05B0, // Small 'pmaxub'.
  0x895C05B0, // Small 'pmaxud'.
  0xAF5C05B0, // Small 'pmaxuw'.
  0x853725B0, // Small 'pminsb'.
  0x893725B0, // Small 'pminsd'.
  0xAF3725B0, // Small 'pminsw'.
  0x855725B0, // Small 'pminub'.
  0x895725B0, // Small 'pminud'.
  0xAF5725B0, // Small 'pminuw'.
  0x1004770F, // Large 'pmovmsk|b'.
  0x122B7717, // Large 'pmovsxb|d'.
  0x12157717, // Large 'pmovsxb|q'.
  0x12647717, // Large 'pmovsxb|w'.
  0x23CF6717, // Large 'pmovsx|dq'.
  0x257A6717, // Large 'pmovsx|wd'.
  0x27096717, // Large 'pmovsx|wq'.
  0x122B7728, // Large 'pmovzxb|d'.
  0x12157728, // Large 'pmovzxb|q'.
  0x12647728, // Large 'pmovzxb|w'.
  0x23CF6728, // Large 'pmovzx|dq'.
  0x257A6728, // Large 'pmovzx|wd'.
  0x27096728, // Large 'pmovzx|wq'.
  0xA24655B0, // Small 'pmuldq'.
  0x25616730, // Large 'pmulhr|sw'.
  0x12646730, // Large 'pmulhr|w'.
  0x264C5730, // Large 'pmulh|uw'.
  0xAE8655B0, // Small 'pmulhw'.
  0x88C655B0, // Small 'pmulld'.
  0xAEC655B0, // Small 'pmullw'.
  0x33CE42D4, // Large 'pmul|udq'.
  0x800041F0, // Small 'pop'.
  0x8000C1F0, // Small 'popa'.
  0x8040C1F0, // Small 'popad'.
  0xA8E1C1F0, // Small 'popcnt'.
  0x800341F0, // Small 'popf'.
  0x804341F0, // Small 'popfd'.
  0x811341F0, // Small 'popfq'.
  0x800049F0, // Small 'por'.
  0x00008378, // Large 'prefetch'.
  0x0000B378, // Large 'prefetchit0'.
  0x1270A378, // Large 'prefetchit|1'.
  0x33838378, // Large 'prefetch|nta'.
  0x23818378, // Large 'prefetch|t0'.
  0x23878378, // Large 'prefetch|t1'.
  0x24A68378, // Large 'prefetch|t2'.
  0x12648378, // Large 'prefetch|w'.
  0x33868378, // Large 'prefetch|wt1'.
  0xAE220670, // Small 'psadbw'.
  0x846AA270, // Small 'pshufb'.
  0x886AA270, // Small 'pshufd'.
  0x288B5365, // Large 'pshuf|hw'.
  0x25825365, // Large 'pshuf|lw'.
  0xAE6AA270, // Small 'pshufw'.
  0x84E3A670, // Small 'psignb'.
  0x88E3A670, // Small 'psignd'.
  0xAEE3A670, // Small 'psignw'.
  0x80463270, // Small 'pslld'.
  0xA2463270, // Small 'pslldq'.
  0x81163270, // Small 'psllq'.
  0x81763270, // Small 'psllw'.
  0x9130B670, // Small 'psmash'.
  0x8040CA70, // Small 'psrad'.
  0x8170CA70, // Small 'psraw'.
  0x80464A70, // Small 'psrld'.
  0xA2464A70, // Small 'psrldq'.
  0x81164A70, // Small 'psrlq'.
  0x81764A70, // Small 'psrlw'.
  0x80215670, // Small 'psubb'.
  0x80415670, // Small 'psubd'.
  0x81115670, // Small 'psubq'.
  0x85315670, // Small 'psubsb'.
  0xAF315670, // Small 'psubsw'.
  0x2786588E, // Large 'psubu|sb'.
  0x2561588E, // Large 'psubu|sw'.
  0x81715670, // Small 'psubw'.
  0x8900DE70, // Small 'pswapd'.
  0x81499690, // Small 'ptest'.
  0x22FD593F, // Large 'ptwri|te'.
  0x2578745A, // Large 'punpckh|bw'.
  0x23CF745A, // Large 'punpckh|dq'.
  0x23CF845A, // Large 'punpckhq|dq'.
  0x257A745A, // Large 'punpckh|wd'.
  0x357C645A, // Large 'punpck|lbw'.
  0x357F645A, // Large 'punpck|ldq'.
  0x4462645A, // Large 'punpck|lqdq'.
  0x3582645A, // Large 'punpck|lwd'.
  0x80044EB0, // Small 'push'.
  0x80144EB0, // Small 'pusha'.
  0x88144EB0, // Small 'pushad'.
  0x80644EB0, // Small 'pushf'.
  0x88644EB0, // Small 'pushfd'.
  0xA2644EB0, // Small 'pushfq'.
  0x81744EB0, // Small 'pushw'.
  0x22FD75F3, // Large 'pvalida|te'.
  0x80093F10, // Small 'pxor'.
  0x80003072, // Small 'rcl'.
  0x81384072, // Small 'rcpps'.
  0x8139C072, // Small 'rcpss'.
  0x80004872, // Small 'rcr'.
  0x317057DE, // Large 'rdfsb|ase'.
  0x317057E3, // Large 'rdgsb|ase'.
  0x8129B492, // Small 'rdmsr'.
  0x8044C092, // Small 'rdpid'.
  0xAB25C092, // Small 'rdpkru'.
  0x8036C092, // Small 'rdpmc'.
  0x81594092, // Small 'rdpru'.
  0x88E0C892, // Small 'rdrand'.
  0x8852CC92, // Small 'rdseed'.
  0x8909CC92, // Small 'rdsspd'.
  0xA309CC92, // Small 'rdsspq'.
  0x8039D092, // Small 'rdtsc'.
  0xA039D092, // Small 'rdtscp'.
  0x800050B2, // Small 'ret'.
  0x800350B2, // Small 'retf'.
  0x222E75FA, // Large 'rmpadju|st'.
  0x22FD7601, // Large 'rmpupda|te'.
  0x800031F2, // Small 'rol'.
  0x800049F2, // Small 'ror'.
  0x800C49F2, // Small 'rorx'.
  0x22AC58A0, // Large 'round|pd'.
  0x228258A0, // Large 'round|ps'.
  0x231D58A0, // Large 'round|sd'.
  0x222258A0, // Large 'round|ss'.
  0x80003672, // Small 'rsm'.
  0x22825586, // Large 'rsqrt|ps'.
  0x22225586, // Large 'rsqrt|ss'.
  0x37E855DE, // Large 'rstor|ssp'.
  0x80032033, // Small 'sahf'.
  0x80004833, // Small 'sar'.
  0x800C4833, // Small 'sarx'.
  0x1092A389, // Large 'saveprevss|p'.
  0x80000853, // Small 'sbb'.
  0x80098473, // Small 'scas'.
  0x102777EB, // Large 'seamcal|l'.
  0x22825944, // Large 'seamo|ps'.
  0x21535949, // Large 'seamr|et'.
  0x240467F2, // Large 'sendui|pi'.
  0x25997608, // Large 'seriali|ze'.
  0x0FFF4152, // Large 'setb' + 'set.b|nae|c'
  0x0FFF5162, // Large 'setbe' + 'set.be|na'
  0x0FFF4171, // Large 'setl' + 'set.l|nge'
  0x0FFF517F, // Large 'setle' + 'set.le|ng'
  0x0FFF518E, // Large 'setnb' + 'set.nb|ae|nc'
  0x0FFF61A0, // Large 'setnbe' + 'set.nbe|a'
  0x0FFF51B0, // Large 'setnl' + 'set.nl|ge'
  0x0FFF61BF, // Large 'setnle' + 'set.nle|g'
  0x80F750B3, // Small 'setno'.
  0x0FFF51CF, // Large 'setnp' + 'set.np|po'
  0x813750B3, // Small 'setns'.
  0x0FFF51DE, // Large 'setnz' + 'set.nz|ne'
  0x8007D0B3, // Small 'seto'.
  0x0FFF41ED, // Large 'setp' + 'set.p|pe'
  0x8009D0B3, // Small 'sets'.
  0x121D77F8, // Large 'setssbs|y'.
  0x0FFF41FA, // Large 'setz' + 'set.z|e'
  0x8A3714D3, // Small 'sfence'.
  0x800A10F3, // Small 'sgdt'.
  0x447E460F, // Large 'sha1|msg1'.
  0x4482460F, // Large 'sha1|msg2'.
  0x22FD760F, // Large 'sha1nex|te'.
  0x12348616, // Large 'sha1rnds|4'.
  0x447E6393, // Large 'sha256|msg1'.
  0x44826393, // Large 'sha256|msg2'.
  0x22839393, // Large 'sha256rnd|s2'.
  0x80003113, // Small 'shl'.
  0x80023113, // Small 'shld'.
  0x800C3113, // Small 'shlx'.
  0x80004913, // Small 'shr'.
  0x80024913, // Small 'shrd'.
  0x800C4913, // Small 'shrx'.
  0x89035513, // Small 'shufpd'.
  0xA7035513, // Small 'shufps'.
  0x800A1133, // Small 'sidt'.
  0xA8972573, // Small 'skinit'.
  0x800A1193, // Small 'sldt'.
  0x84385D93, // Small 'slwpcb'.
  0x800BCDB3, // Small 'smsw'.
  0x890A4A33, // Small 'sqrtpd'.
  0xA70A4A33, // Small 'sqrtps'.
  0x893A4A33, // Small 'sqrtsd'.
  0xA73A4A33, // Small 'sqrtss'.
  0x80018693, // Small 'stac'.
  0x80000E93, // Small 'stc'.
  0x80001293, // Small 'std'.
  0x80049E93, // Small 'stgi'.
  0x80002693, // Small 'sti'.
  0x122868AD, // Large 'stmxcs|r'.
  0x8009BE93, // Small 'stos'.
  0x80004A93, // Small 'str'.
  0x1031861E, // Large 'sttilecf|g'.
  0x8004D693, // Small 'stui'.
  0x80000AB3, // Small 'sub'.
  0x80480AB3, // Small 'subpd'.
  0x81380AB3, // Small 'subps'.
  0x80498AB3, // Small 'subsd'.
  0x81398AB3, // Small 'subss'.
  0xA67806F3, // Small 'swapgs'.
  0x38B9494E, // Large 'sysc|all'.
  0x42FC47FF, // Large 'syse|nter'.
  0x236B5803, // Large 'sysex|it'.
  0x336B5803, // Large 'sysex|itq'.
  0xA8594F33, // Small 'sysret'.
  0x236C5952, // Large 'sysre|tq'.
  0x86B9B794, // Small 't1mskc'.
  0x2282A304, // Large 'tcmmimfp16|ps'.
  0x2282A30E, // Large 'tcmmrlfp16|ps'.
  0x98C08C94, // Small 'tdcall'.
  0x22827626, // Large 'tdpbf16|ps'.
  0x331C4626, // Large 'tdpb|ssd'.
  0x36D84626, // Large 'tdpb|sud'.
  0x231D5957, // Large 'tdpbu|sd'.
  0x23CE5957, // Large 'tdpbu|ud'.
  0x2282762D, // Large 'tdpfp16|ps'.
  0x800A4CB4, // Small 'test'.
  0x935A4CB4, // Small 'testui'.
  0x0000939C, // Large 'tileloadd'.
  0x2387939C, // Large 'tileloadd|t1'.
  0x317083A5, // Large 'tilerele|ase'.
  0x122B94A8, // Large 'tilestore|d'.
  0x4599439C, // Large 'tile|zero'.
  0x2056595C, // Large 'tlbsy|nc'.
  0x8B3A8614, // Small 'tpause'.
  0x81470F54, // Small 'tzcnt'.
  0x80B9B754, // Small 'tzmsk'.
  0x231D58B4, // Large 'ucomi|sd'.
  0x222258B4, // Large 'ucomi|ss'.
  0x80006C95, // Small 'ud0'.
  0x80007095, // Small 'ud1'.
  0x80007495, // Small 'ud2'.
  0x8142C935, // Small 'uiret'.
  0x67C92808, // Large 'um|onitor'.
  0xA890DDB5, // Small 'umwait'.
  0x22AC645B, // Large 'unpckh|pd'.
  0x2282645B, // Large 'unpckh|ps'.
  0x3763545B, // Large 'unpck|lpd'.
  0x3766545B, // Large 'unpck|lps'.
  0x89021036, // Small 'vaddpd'.
  0x91021036, // Small 'vaddph'.
  0xA7021036, // Small 'vaddps'.
  0x89321036, // Small 'vaddsd'.
  0x91321036, // Small 'vaddsh'.
  0xA7321036, // Small 'vaddss'.
  0x22AC7634, // Large 'vaddsub|pd'.
  0x22827634, // Large 'vaddsub|ps'.
  0x000073AD, // Large 'vaesdec'.
  0x322D83AD, // Large 'vaesdecl|ast'.
  0x000073B5, // Large 'vaesenc'.
  0x322D83B5, // Large 'vaesencl|ast'.
  0x27EE5961, // Large 'vaesi|mc'.
  0x1154F217, // Large 'vaeskeygenassis|t'.
  0x23765966, // Large 'valig|nd'.
  0x28DF5966, // Large 'valig|nq'.
  0x22AC596B, // Large 'vandn|pd'.
  0x2282596B, // Large 'vandn|ps'.
  0x89023836, // Small 'vandpd'.
  0xA7023836, // Small 'vandps'.
  0x1152D267, // Large 'vbcstnebf162p|s'.
  0x53187267, // Large 'vbcstne|sh2ps'.
  0x22AC763B, // Large 'vblendm|pd'.
  0x2282763B, // Large 'vblendm|ps'.
  0x22AC663B, // Large 'vblend|pd'.
  0x2282663B, // Large 'vblend|ps'.
  0x3642663B, // Large 'vblend|vpd'.
  0x3364663B, // Large 'vblend|vps'.
  0x3274B226, // Large 'vbroadcastf|128'.
  0x1209E226, // Large 'vbroadcastf32x|2'.
  0x1234E226, // Large 'vbroadcastf32x|4'.
  0x120BE226, // Large 'vbroadcastf32x|8'.
  0x4235B226, // Large 'vbroadcastf|64x2'.
  0x4239B226, // Large 'vbroadcastf|64x4'.
  0x4277A226, // Large 'vbroadcast|i128'.
  0x523DA226, // Large 'vbroadcast|i32x2'.
  0x5242A226, // Large 'vbroadcast|i32x4'.
  0x5247A226, // Large 'vbroadcast|i32x8'.
  0x524CA226, // Large 'vbroadcast|i64x2'.
  0x5251A226, // Large 'vbroadcast|i64x4'.
  0x231DA226, // Large 'vbroadcast|sd'.
  0x2222A226, // Large 'vbroadcast|ss'.
  0x89083476, // Small 'vcmppd'.
  0x91083476, // Small 'vcmpph'.
  0xA7083476, // Small 'vcmpps'.
  0x89383476, // Small 'vcmpsd'.
  0x91383476, // Small 'vcmpsh'.
  0xA7383476, // Small 'vcmpss'.
  0x231D5970, // Large 'vcomi|sd'.
  0x22DA5970, // Large 'vcomi|sh'.
  0x22225970, // Large 'vcomi|ss'.
  0x22AC93BD, // Large 'vcompress|pd'.
  0x228293BD, // Large 'vcompress|ps'.
  0x22AC7645, // Large 'vcvtdq2|pd'.
  0x22A77645, // Large 'vcvtdq2|ph'.
  0x22827645, // Large 'vcvtdq2|ps'.
  0x426EA27B, // Large 'vcvtne2ps2|bf16'.
  0x3281B285, // Large 'vcvtneebf16|2ps'.
  0x531F7285, // Large 'vcvtnee|ph2ps'.
  0x3281B290, // Large 'vcvtneobf16|2ps'.
  0x531F7290, // Large 'vcvtneo|ph2ps'.
  0x426E92DF, // Large 'vcvtneps2|bf16'.
  0x34E164B1, // Large 'vcvtpd|2dq'.
  0x32A664B1, // Large 'vcvtpd|2ph'.
  0x328164B1, // Large 'vcvtpd|2ps'.
  0x34E464B1, // Large 'vcvtpd|2qq'.
  0x63CB427B, // Large 'vcvt|pd2udq'.
  0x43D964B1, // Large 'vcvtpd|2uqq'.
  0x23CF74B7, // Large 'vcvtph2|dq'.
  0x122B84B7, // Large 'vcvtph2p|d'.
  0x000094B7, // Large 'vcvtph2ps'.
  0x123394B7, // Large 'vcvtph2ps|x'.
  0x23D074B7, // Large 'vcvtph2|qq'.
  0x33CE74B7, // Large 'vcvtph2|udq'.
  0x33DA74B7, // Large 'vcvtph2|uqq'.
  0x264C74B7, // Large 'vcvtph2|uw'.
  0x126474B7, // Large 'vcvtph2|w'.
  0x23CF74C0, // Large 'vcvtps2|dq'.
  0x122B84C0, // Large 'vcvtps2p|d'.
  0x000094C0, // Large 'vcvtps2ph'.
  0x123394C0, // Large 'vcvtps2ph|x'.
  0x23D074C0, // Large 'vcvtps2|qq'.
  0x33CE74C0, // Large 'vcvtps2|udq'.
  0x33DA74C0, // Large 'vcvtps2|uqq'.
  0x22AC764E, // Large 'vcvtqq2|pd'.
  0x22A7764E, // Large 'vcvtqq2|ph'.
  0x2282764E, // Large 'vcvtqq2|ps'.
  0x22DA74C9, // Large 'vcvtsd2|sh'.
  0x222374C9, // Large 'vcvtsd2|si'.
  0x222274C9, // Large 'vcvtsd2|ss'.
  0x222384C9, // Large 'vcvtsd2u|si'.
  0x231D74D1, // Large 'vcvtsh2|sd'.
  0x222374D1, // Large 'vcvtsh2|si'.
  0x222274D1, // Large 'vcvtsh2|ss'.
  0x222384D1, // Large 'vcvtsh2u|si'.
  0x231D7655, // Large 'vcvtsi2|sd'.
  0x22DA7655, // Large 'vcvtsi2|sh'.
  0x22227655, // Large 'vcvtsi2|ss'.
  0x231D74D9, // Large 'vcvtss2|sd'.
  0x22DA74D9, // Large 'vcvtss2|sh'.
  0x222374D9, // Large 'vcvtss2|si'.
  0x222384D9, // Large 'vcvtss2u|si'.
  0x23CF83C6, // Large 'vcvttpd2|dq'.
  0x23D083C6, // Large 'vcvttpd2|qq'.
  0x1215A3C6, // Large 'vcvttpd2ud|q'.
  0x23D093C6, // Large 'vcvttpd2u|qq'.
  0x23CF83D2, // Large 'vcvttph2|dq'.
  0x23D083D2, // Large 'vcvttph2|qq'.
  0x43CD73D2, // Large 'vcvttph|2udq'.
  0x43D973D2, // Large 'vcvttph|2uqq'.
  0x126493D2, // Large 'vcvttph2u|w'.
  0x126483D2, // Large 'vcvttph2|w'.
  0x34E173DD, // Large 'vcvttps|2dq'.
  0x34E473DD, // Large 'vcvttps|2qq'.
  0x43CD73DD, // Large 'vcvttps|2udq'.
  0x43D973DD, // Large 'vcvttps|2uqq'.
  0x222383E4, // Large 'vcvttsd2|si'.
  0x222393E4, // Large 'vcvttsd2u|si'.
  0x222383ED, // Large 'vcvttsh2|si'.
  0x222393ED, // Large 'vcvttsh2u|si'.
  0x222383F6, // Large 'vcvttss2|si'.
  0x222393F6, // Large 'vcvttss2u|si'.
  0x22AC84E7, // Large 'vcvtudq2|pd'.
  0x22A784E7, // Large 'vcvtudq2|ph'.
  0x228284E7, // Large 'vcvtudq2|ps'.
  0x22AC84EF, // Large 'vcvtuqq2|pd'.
  0x22A784EF, // Large 'vcvtuqq2|ph'.
  0x228284EF, // Large 'vcvtuqq2|ps'.
  0x231D84F7, // Large 'vcvtusi2|sd'.
  0x22DA84F7, // Large 'vcvtusi2|sh'.
  0x222284F7, // Large 'vcvtusi2|ss'.
  0x32A6665C, // Large 'vcvtuw|2ph'.
  0x32A6580A, // Large 'vcvtw|2ph'.
  0x25787662, // Large 'vdbpsad|bw'.
  0x890B2496, // Small 'vdivpd'.
  0x910B2496, // Small 'vdivph'.
  0xA70B2496, // Small 'vdivps'.
  0x893B2496, // Small 'vdivsd'.
  0x913B2496, // Small 'vdivsh'.
  0xA73B2496, // Small 'vdivss'.
  0x22827669, // Large 'vdpbf16|ps'.
  0x80484096, // Small 'vdppd'.
  0x81384096, // Small 'vdpps'.
  0x800948B6, // Small 'verr'.
  0x800BC8B6, // Small 'verw'.
  0x22AC7670, // Large 'vexpand|pd'.
  0x22827670, // Large 'vexpand|ps'.
  0x327492EF, // Large 'vextractf|128'.
  0x622F72E8, // Large 'vextrac|tf32x4'.
  0x424892EF, // Large 'vextractf|32x8'.
  0x423592EF, // Large 'vextractf|64x2'.
  0x423992EF, // Large 'vextractf|64x4'.
  0x427782EF, // Large 'vextract|i128'.
  0x524282EF, // Large 'vextract|i32x4'.
  0x524782EF, // Large 'vextract|i32x8'.
  0x524C82EF, // Large 'vextract|i64x2'.
  0x525182EF, // Large 'vextract|i64x4'.
  0x228282EF, // Large 'vextract|ps'.
  0x22A784FF, // Large 'vfcmaddc|ph'.
  0x22DA84FF, // Large 'vfcmaddc|sh'.
  0x22A77677, // Large 'vfcmulc|ph'.
  0x22DA7677, // Large 'vfcmulc|sh'.
  0x22AC93FF, // Large 'vfixupimm|pd'.
  0x228293FF, // Large 'vfixupimm|ps'.
  0x231D93FF, // Large 'vfixupimm|sd'.
  0x222293FF, // Large 'vfixupimm|ss'.
  0x22AC9408, // Large 'vfmadd132|pd'.
  0x22A79408, // Large 'vfmadd132|ph'.
  0x22829408, // Large 'vfmadd132|ps'.
  0x231D9408, // Large 'vfmadd132|sd'.
  0x22DA9408, // Large 'vfmadd132|sh'.
  0x22229408, // Large 'vfmadd132|ss'.
  0x52A9629B, // Large 'vfmadd|213pd'.
  0x52AE629B, // Large 'vfmadd|213ph'.
  0x52B3629B, // Large 'vfmadd|213ps'.
  0x532E629B, // Large 'vfmadd|213sd'.
  0x5333629B, // Large 'vfmadd|213sh'.
  0x5338629B, // Large 'vfmadd|213ss'.
  0x52B8629B, // Large 'vfmadd|231pd'.
  0x52BD629B, // Large 'vfmadd|231ph'.
  0x52C2629B, // Large 'vfmadd|231ps'.
  0x533D629B, // Large 'vfmadd|231sd'.
  0x5342629B, // Large 'vfmadd|231sh'.
  0x5347629B, // Large 'vfmadd|231ss'.
  0x367E629B, // Large 'vfmadd|cph'.
  0x3681629B, // Large 'vfmadd|csh'.
  0x22AC629B, // Large 'vfmadd|pd'.
  0x2282629B, // Large 'vfmadd|ps'.
  0x122B729B, // Large 'vfmadds|d'.
  0x1152729B, // Large 'vfmadds|s'.
  0x122BD29B, // Large 'vfmaddsub132p|d'.
  0x12A8D29B, // Large 'vfmaddsub132p|h'.
  0x1152D29B, // Large 'vfmaddsub132p|s'.
  0x52A9929B, // Large 'vfmaddsub|213pd'.
  0x52AE929B, // Large 'vfmaddsub|213ph'.
  0x52B3929B, // Large 'vfmaddsub|213ps'.
  0x52B8929B, // Large 'vfmaddsub|231pd'.
  0x52BD929B, // Large 'vfmaddsub|231ph'.
  0x52C2929B, // Large 'vfmaddsub|231ps'.
  0x22AC929B, // Large 'vfmaddsub|pd'.
  0x2282929B, // Large 'vfmaddsub|ps'.
  0x22AC9411, // Large 'vfmsub132|pd'.
  0x22A79411, // Large 'vfmsub132|ph'.
  0x22829411, // Large 'vfmsub132|ps'.
  0x231D9411, // Large 'vfmsub132|sd'.
  0x22DA9411, // Large 'vfmsub132|sh'.
  0x22229411, // Large 'vfmsub132|ss'.
  0x52A962C7, // Large 'vfmsub|213pd'.
  0x52AE62C7, // Large 'vfmsub|213ph'.
  0x52B362C7, // Large 'vfmsub|213ps'.
  0x532E62C7, // Large 'vfmsub|213sd'.
  0x533362C7, // Large 'vfmsub|213sh'.
  0x533862C7, // Large 'vfmsub|213ss'.
  0x52B862C7, // Large 'vfmsub|231pd'.
  0x52BD62C7, // Large 'vfmsub|231ph'.
  0x52C262C7, // Large 'vfmsub|231ps'.
  0x533D62C7, // Large 'vfmsub|231sd'.
  0x534262C7, // Large 'vfmsub|231sh'.
  0x534762C7, // Large 'vfmsub|231ss'.
  0x22ACC2C7, // Large 'vfmsubadd132|pd'.
  0x22A7C2C7, // Large 'vfmsubadd132|ph'.
  0x2282C2C7, // Large 'vfmsubadd132|ps'.
  0x52A992C7, // Large 'vfmsubadd|213pd'.
  0x52AE92C7, // Large 'vfmsubadd|213ph'.
  0x52B392C7, // Large 'vfmsubadd|213ps'.
  0x52B892C7, // Large 'vfmsubadd|231pd'.
  0x52BD92C7, // Large 'vfmsubadd|231ph'.
  0x52C292C7, // Large 'vfmsubadd|231ps'.
  0x22AC92C7, // Large 'vfmsubadd|pd'.
  0x228292C7, // Large 'vfmsubadd|ps'.
  0x22AC62C7, // Large 'vfmsub|pd'.
  0x228262C7, // Large 'vfmsub|ps'.
  0x231D62C7, // Large 'vfmsub|sd'.
  0x222262C7, // Large 'vfmsub|ss'.
  0x367E580F, // Large 'vfmul|cph'.
  0x3681580F, // Large 'vfmul|csh'.
  0x22ACA324, // Large 'vfnmadd132|pd'.
  0x22A7A324, // Large 'vfnmadd132|ph'.
  0x2282A324, // Large 'vfnmadd132|ps'.
  0x231DA324, // Large 'vfnmadd132|sd'.
  0x22DAA324, // Large 'vfnmadd132|sh'.
  0x2222A324, // Large 'vfnmadd132|ss'.
  0x52A97324, // Large 'vfnmadd|213pd'.
  0x52AE7324, // Large 'vfnmadd|213ph'.
  0x52B37324, // Large 'vfnmadd|213ps'.
  0x532E7324, // Large 'vfnmadd|213sd'.
  0x53337324, // Large 'vfnmadd|213sh'.
  0x53387324, // Large 'vfnmadd|213ss'.
  0x52B87324, // Large 'vfnmadd|231pd'.
  0x52BD7324, // Large 'vfnmadd|231ph'.
  0x52C27324, // Large 'vfnmadd|231ps'.
  0x533D7324, // Large 'vfnmadd|231sd'.
  0x53427324, // Large 'vfnmadd|231sh'.
  0x53477324, // Large 'vfnmadd|231ss'.
  0x22AC7324, // Large 'vfnmadd|pd'.
  0x22827324, // Large 'vfnmadd|ps'.
  0x231D7324, // Large 'vfnmadd|sd'.
  0x22227324, // Large 'vfnmadd|ss'.
  0x22ACA34C, // Large 'vfnmsub132|pd'.
  0x22A7A34C, // Large 'vfnmsub132|ph'.
  0x2282A34C, // Large 'vfnmsub132|ps'.
  0x231DA34C, // Large 'vfnmsub132|sd'.
  0x22DAA34C, // Large 'vfnmsub132|sh'.
  0x2222A34C, // Large 'vfnmsub132|ss'.
  0x52A9734C, // Large 'vfnmsub|213pd'.
  0x52AE734C, // Large 'vfnmsub|213ph'.
  0x52B3734C, // Large 'vfnmsub|213ps'.
  0x532E734C, // Large 'vfnmsub|213sd'.
  0x5333734C, // Large 'vfnmsub|213sh'.
  0x5338734C, // Large 'vfnmsub|213ss'.
  0x52B8734C, // Large 'vfnmsub|231pd'.
  0x52BD734C, // Large 'vfnmsub|231ph'.
  0x52C2734C, // Large 'vfnmsub|231ps'.
  0x533D734C, // Large 'vfnmsub|231sd'.
  0x5342734C, // Large 'vfnmsub|231sh'.
  0x5347734C, // Large 'vfnmsub|231ss'.
  0x22AC734C, // Large 'vfnmsub|pd'.
  0x2282734C, // Large 'vfnmsub|ps'.
  0x231D734C, // Large 'vfnmsub|sd'.
  0x2222734C, // Large 'vfnmsub|ss'.
  0x22AC8507, // Large 'vfpclass|pd'.
  0x22A78507, // Large 'vfpclass|ph'.
  0x22828507, // Large 'vfpclass|ps'.
  0x231D8507, // Large 'vfpclass|sd'.
  0x22DA8507, // Large 'vfpclass|sh'.
  0x22228507, // Large 'vfpclass|ss'.
  0x22AC5975, // Large 'vfrcz|pd'.
  0x22825975, // Large 'vfrcz|ps'.
  0x231D5975, // Large 'vfrcz|sd'.
  0x22225975, // Large 'vfrcz|ss'.
  0x22AC850F, // Large 'vgatherd|pd'.
  0x2282850F, // Large 'vgatherd|ps'.
  0x3478750F, // Large 'vgather|qpd'.
  0x347B750F, // Large 'vgather|qps'.
  0x22AC7684, // Large 'vgetexp|pd'.
  0x22A77684, // Large 'vgetexp|ph'.
  0x22827684, // Large 'vgetexp|ps'.
  0x231D7684, // Large 'vgetexp|sd'.
  0x22DA7684, // Large 'vgetexp|sh'.
  0x22227684, // Large 'vgetexp|ss'.
  0x33CA7517, // Large 'vgetman|tpd'.
  0x33D67517, // Large 'vgetman|tph'.
  0x33E17517, // Large 'vgetman|tps'.
  0x33E87517, // Large 'vgetman|tsd'.
  0x33F17517, // Large 'vgetman|tsh'.
  0x33FA7517, // Large 'vgetman|tss'.
  0x2215F206, // Large 'vgf2p8affineinv|qb'.
  0x2215C206, // Large 'vgf2p8affine|qb'.
  0x451E6206, // Large 'vgf2p8|mulb'.
  0x22AC597A, // Large 'vhadd|pd'.
  0x2282597A, // Large 'vhadd|ps'.
  0x22AC597F, // Large 'vhsub|pd'.
  0x2282597F, // Large 'vhsub|ps'.
  0x3274835C, // Large 'vinsertf|128'.
  0x622F6356, // Large 'vinser|tf32x4'.
  0x4248835C, // Large 'vinsertf|32x8'.
  0x4235835C, // Large 'vinsertf|64x2'.
  0x4239835C, // Large 'vinsertf|64x4'.
  0x4277735C, // Large 'vinsert|i128'.
  0x5242735C, // Large 'vinsert|i32x4'.
  0x5247735C, // Large 'vinsert|i32x8'.
  0x524C735C, // Large 'vinsert|i64x2'.
  0x5251735C, // Large 'vinsert|i64x4'.
  0x2282735C, // Large 'vinsert|ps'.
  0xAB121196, // Small 'vlddqu'.
  0x12287814, // Large 'vldmxcs|r'.
  0x12A2A41A, // Large 'vmaskmovdq|u'.
  0x22AC841A, // Large 'vmaskmov|pd'.
  0x2282841A, // Large 'vmaskmov|ps'.
  0x890C05B6, // Small 'vmaxpd'.
  0x910C05B6, // Small 'vmaxph'.
  0xA70C05B6, // Small 'vmaxps'.
  0x893C05B6, // Small 'vmaxsd'.
  0x913C05B6, // Small 'vmaxsh'.
  0xA73C05B6, // Small 'vmaxss'.
  0x98C08DB6, // Small 'vmcall'.
  0x25F95984, // Large 'vmcle|ar'.
  0x86EA99B6, // Small 'vmfunc'.
  0x236B5989, // Large 'vmgex|it'.
  0x890725B6, // Small 'vminpd'.
  0x910725B6, // Small 'vminph'.
  0xA70725B6, // Small 'vminps'.
  0x893725B6, // Small 'vminsd'.
  0x913725B6, // Small 'vminsh'.
  0xA73725B6, // Small 'vminss'.
  0x237E681B, // Large 'vmlaun|ch'.
  0x8817B1B6, // Small 'vmload'.
  0x38B9498E, // Large 'vmmc|all'.
  0x22AC5992, // Large 'vmova|pd'.
  0x22825992, // Large 'vmova|ps'.
  0x804B3DB6, // Small 'vmovd'.
  0x3821568B, // Large 'vmovd|dup'.
  0x0000768B, // Large 'vmovdqa'.
  0x2231768B, // Large 'vmovdqa|32'.
  0x2235768B, // Large 'vmovdqa|64'.
  0x12A2668B, // Large 'vmovdq|u'.
  0x3692668B, // Large 'vmovdq|u16'.
  0x3695668B, // Large 'vmovdq|u32'.
  0x3698668B, // Large 'vmovdq|u64'.
  0x2824668B, // Large 'vmovdq|u8'.
  0x37665826, // Large 'vmovh|lps'.
  0x22AC5826, // Large 'vmovh|pd'.
  0x22825826, // Large 'vmovh|ps'.
  0x2282682B, // Large 'vmovlh|ps'.
  0x22AC582B, // Large 'vmovl|pd'.
  0x2282582B, // Large 'vmovl|ps'.
  0x22AC769B, // Large 'vmovmsk|pd'.
  0x2282769B, // Large 'vmovmsk|ps'.
  0x23CF66A2, // Large 'vmovnt|dq'.
  0x368F66A2, // Large 'vmovnt|dqa'.
  0x22AC66A2, // Large 'vmovnt|pd'.
  0x228266A2, // Large 'vmovnt|ps'.
  0x811B3DB6, // Small 'vmovq'.
  0x893B3DB6, // Small 'vmovsd'.
  0x913B3DB6, // Small 'vmovsh'.
  0x240376A8, // Large 'vmovshd|up'.
  0x240376AF, // Large 'vmovsld|up'.
  0xA73B3DB6, // Small 'vmovss'.
  0x3604468B, // Large 'vmov|upd'.
  0x22825997, // Large 'vmovu|ps'.
  0x817B3DB6, // Small 'vmovw'.
  0x25786831, // Large 'vmpsad|bw'.
  0x35E2499C, // Large 'vmpt|rld'.
  0x35DE499C, // Large 'vmpt|rst'.
  0x8812C9B6, // Small 'vmread'.
  0x100F7837, // Large 'vmresum|e'.
  0x80EAC9B6, // Small 'vmrun'.
  0x8B60CDB6, // Small 'vmsave'.
  0x890655B6, // Small 'vmulpd'.
  0x910655B6, // Small 'vmulph'.
  0xA70655B6, // Small 'vmulps'.
  0x893655B6, // Small 'vmulsd'.
  0x913655B6, // Small 'vmulsh'.
  0xA73655B6, // Small 'vmulss'.
  0x22FD59A0, // Large 'vmwri|te'.
  0x8C67E1B6, // Small 'vmxoff'.
  0x80E7E1B6, // Small 'vmxon'.
  0x804849F6, // Small 'vorpd'.
  0x813849F6, // Small 'vorps'.
  0x122BC2F8, // Large 'vp2intersect|d'.
  0x1215C2F8, // Large 'vp2intersect|q'.
  0x85310616, // Small 'vpabsb'.
  0x89310616, // Small 'vpabsd'.
  0xA3310616, // Small 'vpabsq'.
  0xAF310616, // Small 'vpabsw'.
  0x126486B6, // Large 'vpackssd|w'.
  0x26BE76B6, // Large 'vpackss|wb'.
  0x36BC66C0, // Large 'vpacku|sdw'.
  0x36C666C0, // Large 'vpacku|swb'.
  0x84420616, // Small 'vpaddb'.
  0x88420616, // Small 'vpaddd'.
  0xA2420616, // Small 'vpaddq'.
  0x2786583E, // Large 'vpadd|sb'.
  0x2561583E, // Large 'vpadd|sw'.
  0x2786683E, // Large 'vpaddu|sb'.
  0x2561683E, // Large 'vpaddu|sw'.
  0xAE420616, // Small 'vpaddw'.
  0x12287844, // Large 'vpalign|r'.
  0x80470616, // Small 'vpand'.
  0x88470616, // Small 'vpandd'.
  0x9C470616, // Small 'vpandn'.
  0x237659A5, // Large 'vpand|nd'.
  0x28DF59A5, // Large 'vpand|nq'.
  0xA2470616, // Small 'vpandq'.
  0x847B0616, // Small 'vpavgb'.
  0xAE7B0616, // Small 'vpavgw'.
  0x122B76C9, // Large 'vpblend|d'.
  0x226176C9, // Large 'vpblend|mb'.
  0x26D076C9, // Large 'vpblend|md'.
  0x121586C9, // Large 'vpblendm|q'.
  0x126486C9, // Large 'vpblendm|w'.
  0x200376C9, // Large 'vpblend|vb'.
  0x126476C9, // Large 'vpblend|w'.
  0x1004B256, // Large 'vpbroadcast|b'.
  0x122BB256, // Large 'vpbroadcast|d'.
  0x1215E256, // Large 'vpbroadcastmb2|q'.
  0x3264C256, // Large 'vpbroadcastm|w2d'.
  0x1215B256, // Large 'vpbroadcast|q'.
  0x1264B256, // Large 'vpbroadcast|w'.
  0x44626522, // Large 'vpclmu|lqdq'.
  0xACF68E16, // Small 'vpcmov'.
  0x85068E16, // Small 'vpcmpb'.
  0x89068E16, // Small 'vpcmpd'.
  0x22156528, // Large 'vpcmpe|qb'.
  0x24576528, // Large 'vpcmpe|qd'.
  0x23D06528, // Large 'vpcmpe|qq'.
  0x27256528, // Large 'vpcmpe|qw'.
  0x120F9528, // Large 'vpcmpestr|i'.
  0x10019528, // Large 'vpcmpestr|m'.
  0x384B5528, // Large 'vpcmp|gtb'.
  0x384E5528, // Large 'vpcmp|gtd'.
  0x38515528, // Large 'vpcmp|gtq'.
  0x38545528, // Large 'vpcmp|gtw'.
  0x120F9531, // Large 'vpcmpistr|i'.
  0x10019531, // Large 'vpcmpistr|m'.
  0xA3068E16, // Small 'vpcmpq'.
  0x22A25528, // Large 'vpcmp|ub'.
  0x23CE5528, // Large 'vpcmp|ud'.
  0x23DA5528, // Large 'vpcmp|uq'.
  0x264C5528, // Large 'vpcmp|uw'.
  0xAF068E16, // Small 'vpcmpw'.
  0x84D78E16, // Small 'vpcomb'.
  0x88D78E16, // Small 'vpcomd'.
  0x1004A424, // Large 'vpcompress|b'.
  0x122BA424, // Large 'vpcompress|d'.
  0x1215A424, // Large 'vpcompress|q'.
  0x1264A424, // Large 'vpcompress|w'.
  0xA2D78E16, // Small 'vpcomq'.
  0x22A25424, // Large 'vpcom|ub'.
  0x23CE5424, // Large 'vpcom|ud'.
  0x23DA5424, // Large 'vpcom|uq'.
  0x264C5424, // Large 'vpcom|uw'.
  0xAED78E16, // Small 'vpcomw'.
  0x122BA42E, // Large 'vpconflict|d'.
  0x1215A42E, // Large 'vpconflict|q'.
  0x122B76D2, // Large 'vpdpbss|d'.
  0x22A076D2, // Large 'vpdpbss|ds'.
  0x23CE66D2, // Large 'vpdpbs|ud'.
  0x36D966D2, // Large 'vpdpbs|uds'.
  0x122B76DC, // Large 'vpdpbus|d'.
  0x22A076DC, // Large 'vpdpbus|ds'.
  0x23CE66DC, // Large 'vpdpbu|ud'.
  0x36D966DC, // Large 'vpdpbu|uds'.
  0x122B76E3, // Large 'vpdpwss|d'.
  0x22A076E3, // Large 'vpdpwss|ds'.
  0x23CE66E3, // Large 'vpdpws|ud'.
  0x36D966E3, // Large 'vpdpws|uds'.
  0x122B76EA, // Large 'vpdpwus|d'.
  0x22A076EA, // Large 'vpdpwus|ds'.
  0x23CE66EA, // Large 'vpdpwu|ud'.
  0x36D966EA, // Large 'vpdpwu|uds'.
  0x3274753A, // Large 'vperm2f|128'.
  0x4277653A, // Large 'vperm2|i128'.
  0x84D91616, // Small 'vpermb'.
  0x88D91616, // Small 'vpermd'.
  0x28576541, // Large 'vpermi|2b'.
  0x22656541, // Large 'vpermi|2d'.
  0x36F16541, // Large 'vpermi|2pd'.
  0x32816541, // Large 'vpermi|2ps'.
  0x24E46541, // Large 'vpermi|2q'.
  0x22636541, // Large 'vpermi|2w'.
  0x22AC8541, // Large 'vpermil2|pd'.
  0x22828541, // Large 'vpermil2|ps'.
  0x22AC7541, // Large 'vpermil|pd'.
  0x22827541, // Large 'vpermil|ps'.
  0x22AC553A, // Large 'vperm|pd'.
  0x2282553A, // Large 'vperm|ps'.
  0xA2D91616, // Small 'vpermq'.
  0x285766F4, // Large 'vpermt|2b'.
  0x226566F4, // Large 'vpermt|2d'.
  0x36F166F4, // Large 'vpermt|2pd'.
  0x328166F4, // Large 'vpermt|2ps'.
  0x24E466F4, // Large 'vpermt|2q'.
  0x226366F4, // Large 'vpermt|2w'.
  0xAED91616, // Small 'vpermw'.
  0x266376FA, // Large 'vpexpan|db'.
  0x229F76FA, // Large 'vpexpan|dd'.
  0x23CF76FA, // Large 'vpexpan|dq'.
  0x26BD76FA, // Large 'vpexpan|dw'.
  0x37BF46FA, // Large 'vpex|trb'.
  0x247659AA, // Large 'vpext|rd'.
  0x245659AA, // Large 'vpext|rq'.
  0x29AF59AA, // Large 'vpext|rw'.
  0x229F8549, // Large 'vpgather|dd'.
  0x23CF8549, // Large 'vpgather|dq'.
  0x24578549, // Large 'vpgather|qd'.
  0x23D08549, // Large 'vpgather|qq'.
  0x28596701, // Large 'vphadd|bd'.
  0x285B6701, // Large 'vphadd|bq'.
  0x25786701, // Large 'vphadd|bw'.
  0x122B6701, // Large 'vphadd|d'.
  0x23CF6701, // Large 'vphadd|dq'.
  0x25616701, // Large 'vphadd|sw'.
  0x122B8701, // Large 'vphaddub|d'.
  0x12158701, // Large 'vphaddub|q'.
  0x12648701, // Large 'vphaddub|w'.
  0x23CF7701, // Large 'vphaddu|dq'.
  0x257A7701, // Large 'vphaddu|wd'.
  0x27097701, // Large 'vphaddu|wq'.
  0x12646701, // Large 'vphadd|w'.
  0x257A6701, // Large 'vphadd|wd'.
  0x27096701, // Large 'vphadd|wq'.
  0x1264A438, // Large 'vphminposu|w'.
  0x2578685D, // Large 'vphsub|bw'.
  0x122B685D, // Large 'vphsub|d'.
  0x23CF685D, // Large 'vphsub|dq'.
  0x2561685D, // Large 'vphsub|sw'.
  0x1264685D, // Large 'vphsub|w'.
  0x257A685D, // Large 'vphsub|wd'.
  0x27C059B1, // Large 'vpins|rb'.
  0x247659B1, // Large 'vpins|rd'.
  0x245659B1, // Large 'vpins|rq'.
  0x29AF59B1, // Large 'vpins|rw'.
  0x26266863, // Large 'vplzcn|td'.
  0x236C6863, // Large 'vplzcn|tq'.
  0x229F6551, // Large 'vpmacs|dd'.
  0x370B6551, // Large 'vpmacs|dqh'.
  0x35806551, // Large 'vpmacs|dql'.
  0x122B8551, // Large 'vpmacssd|d'.
  0x12A89551, // Large 'vpmacssdq|h'.
  0x10279551, // Large 'vpmacssdq|l'.
  0x257A7551, // Large 'vpmacss|wd'.
  0x25797551, // Large 'vpmacss|ww'.
  0x257A6551, // Large 'vpmacs|wd'.
  0x25796551, // Large 'vpmacs|ww'.
  0x122B955A, // Large 'vpmadcssw|d'.
  0x257A755A, // Large 'vpmadcs|wd'.
  0x23DA9442, // Large 'vpmadd52h|uq'.
  0x344B8442, // Large 'vpmadd52|luq'.
  0x45636442, // Large 'vpmadd|ubsw'.
  0x257A6442, // Large 'vpmadd|wd'.
  0x641D4442, // Large 'vpma|skmovd'.
  0x22148567, // Large 'vpmaskmo|vq'.
  0x278659B6, // Large 'vpmax|sb'.
  0x231D59B6, // Large 'vpmax|sd'.
  0x258759B6, // Large 'vpmax|sq'.
  0x256159B6, // Large 'vpmax|sw'.
  0x22A259B6, // Large 'vpmax|ub'.
  0x23CE59B6, // Large 'vpmax|ud'.
  0x23DA59B6, // Large 'vpmax|uq'.
  0x264C59B6, // Large 'vpmax|uw'.
  0x278659BB, // Large 'vpmin|sb'.
  0x231D59BB, // Large 'vpmin|sd'.
  0x258759BB, // Large 'vpmin|sq'.
  0x256159BB, // Large 'vpmin|sw'.
  0x22A259BB, // Large 'vpmin|ub'.
  0x23CE59BB, // Large 'vpmin|ud'.
  0x23DA59BB, // Large 'vpmin|uq'.
  0x264C59BB, // Large 'vpmin|uw'.
  0x3869570E, // Large 'vpmov|b2m'.
  0x386C570E, // Large 'vpmov|d2m'.
  0x2663570E, // Large 'vpmov|db'.
  0x26BD570E, // Large 'vpmov|dw'.
  0x2857670E, // Large 'vpmovm|2b'.
  0x2265670E, // Large 'vpmovm|2d'.
  0x24E4670E, // Large 'vpmovm|2q'.
  0x2263670E, // Large 'vpmovm|2w'.
  0x1004870E, // Large 'vpmovmsk|b'.
  0x386F570E, // Large 'vpmov|q2m'.
  0x2215570E, // Large 'vpmov|qb'.
  0x2457570E, // Large 'vpmov|qd'.
  0x2725570E, // Large 'vpmov|qw'.
  0x26636716, // Large 'vpmovs|db'.
  0x26BD6716, // Large 'vpmovs|dw'.
  0x22156716, // Large 'vpmovs|qb'.
  0x24576716, // Large 'vpmovs|qd'.
  0x27256716, // Large 'vpmovs|qw'.
  0x26BE6716, // Large 'vpmovs|wb'.
  0x122B8716, // Large 'vpmovsxb|d'.
  0x12158716, // Large 'vpmovsxb|q'.
  0x12648716, // Large 'vpmovsxb|w'.
  0x23CF7716, // Large 'vpmovsx|dq'.
  0x257A7716, // Large 'vpmovsx|wd'.
  0x27097716, // Large 'vpmovsx|wq'.
  0x2663771E, // Large 'vpmovus|db'.
  0x26BD771E, // Large 'vpmovus|dw'.
  0x2215771E, // Large 'vpmovus|qb'.
  0x2457771E, // Large 'vpmovus|qd'.
  0x2725771E, // Large 'vpmovus|qw'.
  0x26BE771E, // Large 'vpmovus|wb'.
  0x3872570E, // Large 'vpmov|w2m'.
  0x26BE570E, // Large 'vpmov|wb'.
  0x122B8727, // Large 'vpmovzxb|d'.
  0x12158727, // Large 'vpmovzxb|q'.
  0x12648727, // Large 'vpmovzxb|w'.
  0x23CF7727, // Large 'vpmovzx|dq'.
  0x257A7727, // Large 'vpmovzx|wd'.
  0x27097727, // Large 'vpmovzx|wq'.
  0x23CF52D3, // Large 'vpmul|dq'.
  0x2561772F, // Large 'vpmulhr|sw'.
  0x264C672F, // Large 'vpmulh|uw'.
  0x1264672F, // Large 'vpmulh|w'.
  0x257F52D3, // Large 'vpmul|ld'.
  0x246252D3, // Large 'vpmul|lq'.
  0x258252D3, // Large 'vpmul|lw'.
  0x2215C2D3, // Large 'vpmultishift|qb'.
  0x33CE52D3, // Large 'vpmul|udq'.
  0x21546875, // Large 'vpopcn|tb'.
  0x26266875, // Large 'vpopcn|td'.
  0x236C6875, // Large 'vpopcn|tq'.
  0x27B86875, // Large 'vpopcn|tw'.
  0x80093E16, // Small 'vpor'.
  0x80493E16, // Small 'vpord'.
  0x81193E16, // Small 'vporq'.
  0x9B22C216, // Small 'vpperm'.
  0x88C7CA16, // Small 'vprold'.
  0xA2C7CA16, // Small 'vprolq'.
  0x242159C0, // Large 'vprol|vd'.
  0x221459C0, // Large 'vprol|vq'.
  0x8927CA16, // Small 'vprord'.
  0xA327CA16, // Small 'vprorq'.
  0x242159C5, // Large 'vpror|vd'.
  0x221459C5, // Large 'vpror|vq'.
  0x8547CA16, // Small 'vprotb'.
  0x8947CA16, // Small 'vprotd'.
  0xA347CA16, // Small 'vprotq'.
  0xAF47CA16, // Small 'vprotw'.
  0x257859CA, // Large 'vpsad|bw'.
  0x229F944E, // Large 'vpscatter|dd'.
  0x23CF944E, // Large 'vpscatter|dq'.
  0x2457944E, // Large 'vpscatter|qd'.
  0x1215A44E, // Large 'vpscatterq|q'.
  0x84144E16, // Small 'vpshab'.
  0x88144E16, // Small 'vpshad'.
  0xA2144E16, // Small 'vpshaq'.
  0xAE144E16, // Small 'vpshaw'.
  0x84C44E16, // Small 'vpshlb'.
  0x88C44E16, // Small 'vpshld'.
  0x122B687B, // Large 'vpshld|d'.
  0x1215687B, // Large 'vpshld|q'.
  0x3668587B, // Large 'vpshl|dvd'.
  0x3880587B, // Large 'vpshl|dvq'.
  0x1264787B, // Large 'vpshldv|w'.
  0x1264687B, // Large 'vpshld|w'.
  0xA2C44E16, // Small 'vpshlq'.
  0xAEC44E16, // Small 'vpshlw'.
  0x122B6883, // Large 'vpshrd|d'.
  0x12156883, // Large 'vpshrd|q'.
  0x36685883, // Large 'vpshr|dvd'.
  0x38805883, // Large 'vpshr|dvq'.
  0x38885883, // Large 'vpshr|dvw'.
  0x12646883, // Large 'vpshrd|w'.
  0x00007364, // Large 'vpshufb'.
  0x2261A364, // Large 'vpshufbitq|mb'.
  0x122B6364, // Large 'vpshuf|d'.
  0x288B6364, // Large 'vpshuf|hw'.
  0x25826364, // Large 'vpshuf|lw'.
  0x204859CF, // Large 'vpsig|nb'.
  0x237659CF, // Large 'vpsig|nd'.
  0x28C159CF, // Large 'vpsig|nw'.
  0x88C64E16, // Small 'vpslld'.
  0x357F49D4, // Large 'vpsl|ldq'.
  0xA2C64E16, // Small 'vpsllq'.
  0x242159D8, // Large 'vpsll|vd'.
  0x221459D8, // Large 'vpsll|vq'.
  0x288959D8, // Large 'vpsll|vw'.
  0xAEC64E16, // Small 'vpsllw'.
  0x88194E16, // Small 'vpsrad'.
  0xA2194E16, // Small 'vpsraq'.
  0x242159DD, // Large 'vpsra|vd'.
  0x221459DD, // Large 'vpsra|vq'.
  0x288959DD, // Large 'vpsra|vw'.
  0xAE194E16, // Small 'vpsraw'.
  0x88C94E16, // Small 'vpsrld'.
  0x357F49DD, // Large 'vpsr|ldq'.
  0xA2C94E16, // Small 'vpsrlq'.
  0x242159E2, // Large 'vpsrl|vd'.
  0x221459E2, // Large 'vpsrl|vq'.
  0x288959E2, // Large 'vpsrl|vw'.
  0xAEC94E16, // Small 'vpsrlw'.
  0x842ACE16, // Small 'vpsubb'.
  0x882ACE16, // Small 'vpsubd'.
  0xA22ACE16, // Small 'vpsubq'.
  0x2786588D, // Large 'vpsub|sb'.
  0x2561588D, // Large 'vpsub|sw'.
  0x2786688D, // Large 'vpsubu|sb'.
  0x2561688D, // Large 'vpsubu|sw'.
  0xAE2ACE16, // Small 'vpsubw'.
  0x122B956F, // Large 'vpternlog|d'.
  0x1215956F, // Large 'vpternlog|q'.
  0xA932D216, // Small 'vptest'.
  0x22616736, // Large 'vptest|mb'.
  0x26D06736, // Large 'vptest|md'.
  0x273D6736, // Large 'vptest|mq'.
  0x28716736, // Large 'vptest|mw'.
  0x22617736, // Large 'vptestn|mb'.
  0x26D07736, // Large 'vptestn|md'.
  0x273D7736, // Large 'vptestn|mq'.
  0x12648736, // Large 'vptestnm|w'.
  0x25788459, // Large 'vpunpckh|bw'.
  0x23CF8459, // Large 'vpunpckh|dq'.
  0x23CF9459, // Large 'vpunpckhq|dq'.
  0x257A8459, // Large 'vpunpckh|wd'.
  0x357C7459, // Large 'vpunpck|lbw'.
  0x357F7459, // Large 'vpunpck|ldq'.
  0x44627459, // Large 'vpunpck|lqdq'.
  0x35827459, // Large 'vpunpck|lwd'.
  0x8127E216, // Small 'vpxor'.
  0x8927E216, // Small 'vpxord'.
  0xA327E216, // Small 'vpxorq'.
  0x22AC6893, // Large 'vrange|pd'.
  0x22826893, // Large 'vrange|ps'.
  0x231D6893, // Large 'vrange|sd'.
  0x22226893, // Large 'vrange|ss'.
  0x22AC6899, // Large 'vrcp14|pd'.
  0x22826899, // Large 'vrcp14|ps'.
  0x231D6899, // Large 'vrcp14|sd'.
  0x22226899, // Large 'vrcp14|ss'.
  0x91080E56, // Small 'vrcpph'.
  0xA7080E56, // Small 'vrcpps'.
  0x91380E56, // Small 'vrcpsh'.
  0xA7380E56, // Small 'vrcpss'.
  0x22AC773F, // Large 'vreduce|pd'.
  0x22A7773F, // Large 'vreduce|ph'.
  0x2282773F, // Large 'vreduce|ps'.
  0x231D773F, // Large 'vreduce|sd'.
  0x22DA773F, // Large 'vreduce|sh'.
  0x2222773F, // Large 'vreduce|ss'.
  0x22AC9466, // Large 'vrndscale|pd'.
  0x22A79466, // Large 'vrndscale|ph'.
  0x22829466, // Large 'vrndscale|ps'.
  0x231D9466, // Large 'vrndscale|sd'.
  0x22DA9466, // Large 'vrndscale|sh'.
  0x22229466, // Large 'vrndscale|ss'.
  0x22AC689F, // Large 'vround|pd'.
  0x2282689F, // Large 'vround|ps'.
  0x231D689F, // Large 'vround|sd'.
  0x2222689F, // Large 'vround|ss'.
  0x22AC8585, // Large 'vrsqrt14|pd'.
  0x22828585, // Large 'vrsqrt14|ps'.
  0x231D8585, // Large 'vrsqrt14|sd'.
  0x22228585, // Large 'vrsqrt14|ss'.
  0x22A76585, // Large 'vrsqrt|ph'.
  0x22826585, // Large 'vrsqrt|ps'.
  0x22DA6585, // Large 'vrsqrt|sh'.
  0x22226585, // Large 'vrsqrt|ss'.
  0x22AC7746, // Large 'vscalef|pd'.
  0x22A77746, // Large 'vscalef|ph'.
  0x22827746, // Large 'vscalef|ps'.
  0x231D7746, // Large 'vscalef|sd'.
  0x22DA7746, // Large 'vscalef|sh'.
  0x22227746, // Large 'vscalef|ss'.
  0x22AC946F, // Large 'vscatterd|pd'.
  0x2282946F, // Large 'vscatterd|ps'.
  0x3478846F, // Large 'vscatter|qpd'.
  0x347B846F, // Large 'vscatter|qps'.
  0x447E736E, // Large 'vsha512|msg1'.
  0x4482736E, // Large 'vsha512|msg2'.
  0x2283A36E, // Large 'vsha512rnd|s2'.
  0x5230558D, // Large 'vshuf|f32x4'.
  0x42356592, // Large 'vshuff|64x2'.
  0x5242558D, // Large 'vshuf|i32x4'.
  0x524C558D, // Large 'vshuf|i64x2'.
  0x22AC558D, // Large 'vshuf|pd'.
  0x2282558D, // Large 'vshuf|ps'.
  0x447E474D, // Large 'vsm3|msg1'.
  0x4482474D, // Large 'vsm3|msg2'.
  0x2283774D, // Large 'vsm3rnd|s2'.
  0x123478A5, // Large 'vsm4key|4'.
  0x12348754, // Large 'vsm4rnds|4'.
  0x33CA49E7, // Large 'vsqr|tpd'.
  0x33D649E7, // Large 'vsqr|tph'.
  0x33E149E7, // Large 'vsqr|tps'.
  0x33E849E7, // Large 'vsqr|tsd'.
  0x33F149E7, // Large 'vsqr|tsh'.
  0x33FA49E7, // Large 'vsqr|tss'.
  0x122878AC, // Large 'vstmxcs|r'.
  0x89015676, // Small 'vsubpd'.
  0x91015676, // Small 'vsubph'.
  0xA7015676, // Small 'vsubps'.
  0x89315676, // Small 'vsubsd'.
  0x91315676, // Small 'vsubsh'.
  0xA7315676, // Small 'vsubss'.
  0x33CA49EB, // Large 'vtes|tpd'.
  0x33E149EB, // Large 'vtes|tps'.
  0x231D68B3, // Large 'vucomi|sd'.
  0x22DA68B3, // Large 'vucomi|sh'.
  0x222268B3, // Large 'vucomi|ss'.
  0x22AC775C, // Large 'vunpckh|pd'.
  0x2282775C, // Large 'vunpckh|ps'.
  0x3763675C, // Large 'vunpck|lpd'.
  0x3766675C, // Large 'vunpck|lps'.
  0x89093F16, // Small 'vxorpd'.
  0xA7093F16, // Small 'vxorps'.
  0x38B95598, // Large 'vzero|all'.
  0x353B7598, // Large 'vzeroup|per'.
  0x89672457, // Small 'wbinvd'.
  0x242168BC, // Large 'wbnoin|vd'.
  0x317058C2, // Large 'wrfsb|ase'.
  0x317058C7, // Large 'wrgsb|ase'.
  0x8129B657, // Small 'wrmsr'.
  0x8049CE57, // Small 'wrssd'.
  0x8119CE57, // Small 'wrssq'.
  0x8939D657, // Small 'wrussd'.
  0xA339D657, // Small 'wrussq'.
  0xA9278838, // Small 'xabort'.
  0x80021038, // Small 'xadd'.
  0x9C939458, // Small 'xbegin'.
  0x8003A078, // Small 'xchg'.
  0x800238B8, // Small 'xend'.
  0xAC2A14F8, // Small 'xgetbv'.
  0x802A0598, // Small 'xlatb'.
  0x800049F8, // Small 'xor'.
  0x804849F8, // Small 'xorpd'.
  0x813849F8, // Small 'xorps'.
  0x121B8769, // Large 'xresldtr|k'.
  0xA4FA4E58, // Small 'xrstor'.
  0x223565DD, // Large 'xrstor|64'.
  0x115265DD, // Large 'xrstor|s'.
  0x377165DD, // Large 'xrstor|s64'.
  0x805B0678, // Small 'xsave'.
  0x2235559F, // Large 'xsave|64'.
  0x865B0678, // Small 'xsavec'.
  0x38CC559F, // Large 'xsave|c64'.
  0x0000859F, // Large 'xsaveopt'.
  0x2235859F, // Large 'xsaveopt|64'.
  0xA65B0678, // Small 'xsaves'.
  0x3771559F, // Large 'xsave|s64'.
  0xAC2A1678, // Small 'xsetbv'.
  0x121B8774, // Large 'xsusldtr|k'.
  0x81499698  // Small 'xtest'.
};

const char InstDB::alias_name_string_table[] =
  "\x63\x6D\x6F\x76\x6E\x61\x65\x67\x65";


const uint32_t InstDB::alias_name_index_table[] = {
  0x801B3DA3, // Small 'cmova'.
  0x8A1B3DA3, // Small 'cmovae'.
  0x803B3DA3, // Small 'cmovc'.
  0x805B3DA3, // Small 'cmove'.
  0x807B3DA3, // Small 'cmovg'.
  0x8A7B3DA3, // Small 'cmovge'.
  0x82EB3DA3, // Small 'cmovna'.
  0x00007000, // Large 'cmovnae'.
  0x86EB3DA3, // Small 'cmovnc'.
  0x8AEB3DA3, // Small 'cmovne'.
  0x8EEB3DA3, // Small 'cmovng'.
  0x20075000, // Large 'cmovn|ge'.
  0x8B0B3DA3, // Small 'cmovpe'.
  0x9F0B3DA3, // Small 'cmovpo'.
  0x8000002A, // Small 'ja'.
  0x8000142A, // Small 'jae'.
  0x8000006A, // Small 'jc'.
  0x800000AA, // Small 'je'.
  0x800000EA, // Small 'jg'.
  0x800014EA, // Small 'jge'.
  0x800005CA, // Small 'jna'.
  0x800285CA, // Small 'jnae'.
  0x80000DCA, // Small 'jnc'.
  0x800015CA, // Small 'jne'.
  0x80001DCA, // Small 'jng'.
  0x80029DCA, // Small 'jnge'.
  0x8000160A, // Small 'jpe'.
  0x80003E0A, // Small 'jpo'.
  0x80003033, // Small 'sal'.
  0x8000D0B3, // Small 'seta'.
  0x8050D0B3, // Small 'setae'.
  0x8001D0B3, // Small 'setc'.
  0x8002D0B3, // Small 'sete'.
  0x8003D0B3, // Small 'setg'.
  0x8053D0B3, // Small 'setge'.
  0x801750B3, // Small 'setna'.
  0x8A1750B3, // Small 'setnae'.
  0x803750B3, // Small 'setnc'.
  0x805750B3, // Small 'setne'.
  0x807750B3, // Small 'setng'.
  0x8A7750B3, // Small 'setnge'.
  0x805850B3, // Small 'setpe'.
  0x80F850B3, // Small 'setpo'.
  0x800A2437  // Small 'wait'.
};

const uint32_t InstDB::alias_index_to_inst_id_table[] = {
  Inst::kIdCmovnbe, // #0
  Inst::kIdCmovnb, // #1
  Inst::kIdCmovb, // #2
  Inst::kIdCmovz, // #3
  Inst::kIdCmovnle, // #4
  Inst::kIdCmovnl, // #5
  Inst::kIdCmovbe, // #6
  Inst::kIdCmovb, // #7
  Inst::kIdCmovnb, // #8
  Inst::kIdCmovnz, // #9
  Inst::kIdCmovle, // #10
  Inst::kIdCmovl, // #11
  Inst::kIdCmovp, // #12
  Inst::kIdCmovnp, // #13
  Inst::kIdJnbe, // #14
  Inst::kIdJnb, // #15
  Inst::kIdJb, // #16
  Inst::kIdJz, // #17
  Inst::kIdJnle, // #18
  Inst::kIdJnl, // #19
  Inst::kIdJbe, // #20
  Inst::kIdJb, // #21
  Inst::kIdJnb, // #22
  Inst::kIdJnz, // #23
  Inst::kIdJle, // #24
  Inst::kIdJl, // #25
  Inst::kIdJp, // #26
  Inst::kIdJnp, // #27
  Inst::kIdShl, // #28
  Inst::kIdSetnbe, // #29
  Inst::kIdSetnb, // #30
  Inst::kIdSetb, // #31
  Inst::kIdSetz, // #32
  Inst::kIdSetnle, // #33
  Inst::kIdSetnl, // #34
  Inst::kIdSetbe, // #35
  Inst::kIdSetb, // #36
  Inst::kIdSetnb, // #37
  Inst::kIdSetnz, // #38
  Inst::kIdSetle, // #39
  Inst::kIdSetl, // #40
  Inst::kIdSetp, // #41
  Inst::kIdSetnp, // #42
  Inst::kIdFwait  // #43
};
// ----------------------------------------------------------------------------
// ${NameData:End}
#endif // !ASMJIT_NO_TEXT

// x86::InstDB - InstSignature & OpSignature
// =========================================

#ifndef ASMJIT_NO_INTROSPECTION
// ${InstSignatureTable:Begin}
// ------------------- Automatically generated, do not edit -------------------
#define ROW(count, x86, x64, implicit, o0, o1, o2, o3, o4, o5)       \
  { count, uint8_t(x86 ? uint8_t(InstDB::Mode::kX86) : uint8_t(0)) | \
                  (x64 ? uint8_t(InstDB::Mode::kX64) : uint8_t(0)) , \
    implicit,                                                        \
    0,                                                               \
    { o0, o1, o2, o3, o4, o5 }                                       \
  }
const InstDB::InstSignature InstDB::_inst_signature_table[] = {
  ROW(2, 1, 1, 0, 1  , 2  , 0  , 0  , 0  , 0  ), // #0   {r8lo|r8hi|m8|mem, r8lo|r8hi}
  ROW(2, 1, 1, 0, 3  , 4  , 0  , 0  , 0  , 0  ), //      {r16|m16|mem|sreg, r16}
  ROW(2, 1, 1, 0, 5  , 6  , 0  , 0  , 0  , 0  ), //      {r32|m32|mem|sreg, r32}
  ROW(2, 0, 1, 0, 7  , 8  , 0  , 0  , 0  , 0  ), //      {r64|m64|mem|sreg|creg|dreg, r64}
  ROW(2, 1, 1, 0, 9  , 10 , 0  , 0  , 0  , 0  ), //      {r8lo|r8hi|m8, i8|u8}
  ROW(2, 1, 1, 0, 11 , 12 , 0  , 0  , 0  , 0  ), //      {r16|m16, i16|u16}
  ROW(2, 1, 1, 0, 13 , 14 , 0  , 0  , 0  , 0  ), //      {r32|m32, i32|u32}
  ROW(2, 0, 1, 0, 15 , 16 , 0  , 0  , 0  , 0  ), //      {r64|m64, i32}
  ROW(2, 1, 1, 0, 2  , 17 , 0  , 0  , 0  , 0  ), //      {r8lo|r8hi, m8|mem}
  ROW(2, 1, 1, 0, 4  , 18 , 0  , 0  , 0  , 0  ), //      {r16, m16|mem|sreg}
  ROW(2, 1, 1, 0, 6  , 19 , 0  , 0  , 0  , 0  ), //      {r32, m32|mem|sreg}
  ROW(2, 0, 1, 0, 8  , 20 , 0  , 0  , 0  , 0  ), //      {r64, m64|mem|i64|u64|sreg|creg|dreg}
  ROW(2, 1, 1, 0, 21 , 22 , 0  , 0  , 0  , 0  ), //      {m16|mem, sreg}
  ROW(2, 1, 1, 0, 21 , 22 , 0  , 0  , 0  , 0  ), //      {m16|mem, sreg}
  ROW(2, 0, 1, 0, 21 , 22 , 0  , 0  , 0  , 0  ), //      {m16|mem, sreg}
  ROW(2, 1, 1, 0, 22 , 21 , 0  , 0  , 0  , 0  ), //      {sreg, m16|mem}
  ROW(2, 1, 1, 0, 22 , 21 , 0  , 0  , 0  , 0  ), //      {sreg, m16|mem}
  ROW(2, 0, 1, 0, 22 , 21 , 0  , 0  , 0  , 0  ), //      {sreg, m16|mem}
  ROW(2, 1, 0, 0, 6  , 23 , 0  , 0  , 0  , 0  ), //      {r32, creg|dreg}
  ROW(2, 1, 0, 0, 23 , 6  , 0  , 0  , 0  , 0  ), //      {creg|dreg, r32}
  ROW(2, 1, 1, 0, 1  , 2  , 0  , 0  , 0  , 0  ), // #20  {r8lo|r8hi|m8|mem, r8lo|r8hi}
  ROW(2, 1, 1, 0, 24 , 4  , 0  , 0  , 0  , 0  ), //      {r16|m16|mem, r16}
  ROW(2, 1, 1, 0, 25 , 6  , 0  , 0  , 0  , 0  ), //      {r32|m32|mem, r32}
  ROW(2, 0, 1, 0, 26 , 8  , 0  , 0  , 0  , 0  ), //      {r64|m64|mem, r64}
  ROW(2, 1, 1, 0, 9  , 10 , 0  , 0  , 0  , 0  ), //      {r8lo|r8hi|m8, i8|u8}
  ROW(2, 1, 1, 0, 27 , 28 , 0  , 0  , 0  , 0  ), //      {r16|m16|r32|m32, i8}
  ROW(2, 0, 1, 0, 15 , 29 , 0  , 0  , 0  , 0  ), //      {r64|m64, i8|i32}
  ROW(2, 1, 1, 0, 11 , 12 , 0  , 0  , 0  , 0  ), //      {r16|m16, i16|u16}
  ROW(2, 1, 1, 0, 13 , 14 , 0  , 0  , 0  , 0  ), //      {r32|m32, i32|u32}
  ROW(2, 1, 1, 0, 2  , 17 , 0  , 0  , 0  , 0  ), //      {r8lo|r8hi, m8|mem}
  ROW(2, 1, 1, 0, 4  , 21 , 0  , 0  , 0  , 0  ), //      {r16, m16|mem}
  ROW(2, 1, 1, 0, 6  , 30 , 0  , 0  , 0  , 0  ), //      {r32, m32|mem}
  ROW(2, 0, 1, 0, 8  , 31 , 0  , 0  , 0  , 0  ), //      {r64, m64|mem}
  ROW(2, 1, 1, 0, 9  , 10 , 0  , 0  , 0  , 0  ), // #33  {r8lo|r8hi|m8, i8|u8}
  ROW(2, 1, 1, 0, 11 , 12 , 0  , 0  , 0  , 0  ), //      {r16|m16, i16|u16}
  ROW(2, 1, 1, 0, 13 , 14 , 0  , 0  , 0  , 0  ), //      {r32|m32, i32|u32}
  ROW(2, 0, 1, 0, 8  , 32 , 0  , 0  , 0  , 0  ), //      {r64, u32|i32|i8|r64|m64|mem}
  ROW(2, 0, 1, 0, 33 , 29 , 0  , 0  , 0  , 0  ), //      {m64, i32|i8}
  ROW(2, 1, 1, 0, 27 , 28 , 0  , 0  , 0  , 0  ), //      {r16|m16|r32|m32, i8}
  ROW(2, 1, 1, 0, 1  , 2  , 0  , 0  , 0  , 0  ), //      {r8lo|r8hi|m8|mem, r8lo|r8hi}
  ROW(2, 1, 1, 0, 24 , 4  , 0  , 0  , 0  , 0  ), //      {r16|m16|mem, r16}
  ROW(2, 1, 1, 0, 25 , 6  , 0  , 0  , 0  , 0  ), //      {r32|m32|mem, r32}
  ROW(2, 0, 1, 0, 31 , 8  , 0  , 0  , 0  , 0  ), // #42  {m64|mem, r64}
  ROW(2, 1, 1, 0, 2  , 17 , 0  , 0  , 0  , 0  ), //      {r8lo|r8hi, m8|mem}
  ROW(2, 1, 1, 0, 4  , 21 , 0  , 0  , 0  , 0  ), //      {r16, m16|mem}
  ROW(2, 1, 1, 0, 6  , 30 , 0  , 0  , 0  , 0  ), //      {r32, m32|mem}
  ROW(2, 1, 1, 0, 1  , 2  , 0  , 0  , 0  , 0  ), // #46  {r8lo|r8hi|m8|mem, r8lo|r8hi}
  ROW(2, 1, 1, 0, 24 , 4  , 0  , 0  , 0  , 0  ), //      {r16|m16|mem, r16}
  ROW(2, 1, 1, 0, 25 , 6  , 0  , 0  , 0  , 0  ), //      {r32|m32|mem, r32}
  ROW(2, 0, 1, 0, 26 , 8  , 0  , 0  , 0  , 0  ), //      {r64|m64|mem, r64}
  ROW(2, 1, 1, 0, 2  , 34 , 0  , 0  , 0  , 0  ), //      {r8lo|r8hi, m8|mem|i8|u8}
  ROW(2, 1, 1, 0, 4  , 35 , 0  , 0  , 0  , 0  ), //      {r16, m16|mem|i8|i16|u16}
  ROW(2, 1, 1, 0, 6  , 36 , 0  , 0  , 0  , 0  ), //      {r32, m32|mem|i8|i32|u32}
  ROW(2, 0, 1, 0, 8  , 37 , 0  , 0  , 0  , 0  ), //      {r64, m64|mem|i8|i32}
  ROW(2, 1, 1, 0, 38 , 10 , 0  , 0  , 0  , 0  ), //      {m8, i8|u8}
  ROW(2, 1, 1, 0, 39 , 28 , 0  , 0  , 0  , 0  ), //      {m16|m32, i8}
  ROW(2, 0, 1, 0, 33 , 29 , 0  , 0  , 0  , 0  ), //      {m64, i8|i32}
  ROW(2, 1, 1, 0, 40 , 12 , 0  , 0  , 0  , 0  ), //      {m16, i16|u16}
  ROW(2, 1, 1, 0, 41 , 14 , 0  , 0  , 0  , 0  ), //      {m32, i32|u32}
  ROW(2, 1, 1, 0, 1  , 2  , 0  , 0  , 0  , 0  ), // #59  {r8lo|r8hi|m8|mem, r8lo|r8hi}
  ROW(2, 1, 1, 0, 24 , 4  , 0  , 0  , 0  , 0  ), //      {r16|m16|mem, r16}
  ROW(2, 1, 1, 0, 25 , 6  , 0  , 0  , 0  , 0  ), //      {r32|m32|mem, r32}
  ROW(2, 0, 1, 0, 26 , 8  , 0  , 0  , 0  , 0  ), //      {r64|m64|mem, r64}
  ROW(2, 1, 1, 0, 9  , 10 , 0  , 0  , 0  , 0  ), //      {r8lo|r8hi|m8, i8|u8}
  ROW(2, 1, 1, 0, 11 , 12 , 0  , 0  , 0  , 0  ), //      {r16|m16, i16|u16}
  ROW(2, 1, 1, 0, 13 , 14 , 0  , 0  , 0  , 0  ), //      {r32|m32, i32|u32}
  ROW(2, 0, 1, 0, 15 , 29 , 0  , 0  , 0  , 0  ), //      {r64|m64, i32|i8}
  ROW(2, 1, 1, 0, 27 , 28 , 0  , 0  , 0  , 0  ), //      {r16|m16|r32|m32, i8}
  ROW(2, 1, 1, 0, 2  , 17 , 0  , 0  , 0  , 0  ), //      {r8lo|r8hi, m8|mem}
  ROW(2, 1, 1, 0, 4  , 21 , 0  , 0  , 0  , 0  ), //      {r16, m16|mem}
  ROW(2, 1, 1, 0, 6  , 30 , 0  , 0  , 0  , 0  ), //      {r32, m32|mem}
  ROW(2, 0, 1, 0, 8  , 31 , 0  , 0  , 0  , 0  ), //      {r64, m64|mem}
  ROW(2, 1, 1, 1, 42 , 1  , 0  , 0  , 0  , 0  ), // #72  {<ax>, r8lo|r8hi|m8|mem}
  ROW(3, 1, 1, 2, 43 , 42 , 24 , 0  , 0  , 0  ), //      {<dx>, <ax>, r16|m16|mem}
  ROW(3, 1, 1, 2, 44 , 45 , 25 , 0  , 0  , 0  ), //      {<edx>, <eax>, r32|m32|mem}
  ROW(3, 0, 1, 2, 46 , 47 , 26 , 0  , 0  , 0  ), //      {<rdx>, <rax>, r64|m64|mem}
  ROW(2, 1, 1, 0, 4  , 24 , 0  , 0  , 0  , 0  ), // #76  {r16, r16|m16|mem}
  ROW(2, 1, 1, 0, 6  , 25 , 0  , 0  , 0  , 0  ), // #77  {r32, r32|m32|mem}
  ROW(2, 0, 1, 0, 8  , 26 , 0  , 0  , 0  , 0  ), //      {r64, r64|m64|mem}
  ROW(3, 1, 1, 0, 4  , 24 , 48 , 0  , 0  , 0  ), //      {r16, r16|m16|mem, i8|i16|u16}
  ROW(3, 1, 1, 0, 6  , 25 , 49 , 0  , 0  , 0  ), //      {r32, r32|m32|mem, i8|i32|u32}
  ROW(3, 0, 1, 0, 8  , 26 , 29 , 0  , 0  , 0  ), //      {r64, r64|m64|mem, i8|i32}
  ROW(2, 0, 1, 0, 8  , 50 , 0  , 0  , 0  , 0  ), // #82  {r64, i64|u64}
  ROW(2, 1, 1, 0, 51 , 17 , 0  , 0  , 0  , 0  ), //      {al, m8|mem}
  ROW(2, 1, 1, 0, 52 , 21 , 0  , 0  , 0  , 0  ), //      {ax, m16|mem}
  ROW(2, 1, 1, 0, 53 , 30 , 0  , 0  , 0  , 0  ), //      {eax, m32|mem}
  ROW(2, 0, 1, 0, 54 , 31 , 0  , 0  , 0  , 0  ), //      {rax, m64|mem}
  ROW(2, 1, 1, 0, 17 , 51 , 0  , 0  , 0  , 0  ), //      {m8|mem, al}
  ROW(2, 1, 1, 0, 21 , 52 , 0  , 0  , 0  , 0  ), //      {m16|mem, ax}
  ROW(2, 1, 1, 0, 30 , 53 , 0  , 0  , 0  , 0  ), //      {m32|mem, eax}
  ROW(2, 0, 1, 0, 31 , 54 , 0  , 0  , 0  , 0  ), //      {m64|mem, rax}
  ROW(2, 1, 1, 0, 1  , 2  , 0  , 0  , 0  , 0  ), // #91  {r8lo|r8hi|m8|mem, r8lo|r8hi}
  ROW(2, 1, 1, 0, 24 , 4  , 0  , 0  , 0  , 0  ), //      {r16|m16|mem, r16}
  ROW(2, 1, 1, 0, 25 , 6  , 0  , 0  , 0  , 0  ), //      {r32|m32|mem, r32}
  ROW(2, 0, 1, 0, 26 , 8  , 0  , 0  , 0  , 0  ), //      {r64|m64|mem, r64}
  ROW(2, 1, 1, 0, 9  , 10 , 0  , 0  , 0  , 0  ), //      {r8lo|r8hi|m8, i8|u8}
  ROW(2, 1, 1, 0, 11 , 12 , 0  , 0  , 0  , 0  ), //      {r16|m16, i16|u16}
  ROW(2, 1, 1, 0, 13 , 14 , 0  , 0  , 0  , 0  ), //      {r32|m32, i32|u32}
  ROW(2, 0, 1, 0, 15 , 16 , 0  , 0  , 0  , 0  ), //      {r64|m64, i32}
  ROW(2, 1, 1, 0, 55 , 56 , 0  , 0  , 0  , 0  ), // #99  {xmm, xmm|m128|mem}
  ROW(2, 1, 1, 0, 57 , 58 , 0  , 0  , 0  , 0  ), //      {ymm, ymm|m256|mem}
  ROW(2, 1, 1, 0, 59 , 55 , 0  , 0  , 0  , 0  ), // #101 {m128|mem, xmm}
  ROW(2, 1, 1, 0, 60 , 57 , 0  , 0  , 0  , 0  ), //      {m256|mem, ymm}
  ROW(2, 1, 1, 0, 61 , 62 , 0  , 0  , 0  , 0  ), //      {zmm, zmm|m512|mem}
  ROW(2, 1, 1, 0, 59 , 55 , 0  , 0  , 0  , 0  ), //      {m128|mem, xmm}
  ROW(2, 1, 1, 0, 60 , 57 , 0  , 0  , 0  , 0  ), //      {m256|mem, ymm}
  ROW(2, 1, 1, 0, 63 , 61 , 0  , 0  , 0  , 0  ), //      {m512|mem, zmm}
  ROW(2, 1, 1, 0, 31 , 55 , 0  , 0  , 0  , 0  ), // #107 {m64|mem, xmm}
  ROW(2, 1, 1, 0, 55 , 31 , 0  , 0  , 0  , 0  ), //      {xmm, m64|mem}
  ROW(3, 1, 1, 0, 55 , 55 , 55 , 0  , 0  , 0  ), // #109 {xmm, xmm, xmm}
  ROW(3, 1, 1, 0, 55 , 55 , 55 , 0  , 0  , 0  ), //      {xmm, xmm, xmm}
  ROW(2, 1, 1, 0, 31 , 55 , 0  , 0  , 0  , 0  ), //      {m64|mem, xmm}
  ROW(2, 1, 1, 0, 55 , 31 , 0  , 0  , 0  , 0  ), //      {xmm, m64|mem}
  ROW(3, 1, 1, 0, 55 , 55 , 55 , 0  , 0  , 0  ), //      {xmm, xmm, xmm}
  ROW(3, 1, 1, 0, 55 , 55 , 55 , 0  , 0  , 0  ), //      {xmm, xmm, xmm}
  ROW(2, 1, 1, 0, 30 , 55 , 0  , 0  , 0  , 0  ), // #115 {m32|mem, xmm}
  ROW(2, 1, 1, 0, 55 , 30 , 0  , 0  , 0  , 0  ), //      {xmm, m32|mem}
  ROW(3, 1, 1, 0, 55 , 55 , 55 , 0  , 0  , 0  ), //      {xmm, xmm, xmm}
  ROW(3, 1, 1, 0, 55 , 55 , 55 , 0  , 0  , 0  ), //      {xmm, xmm, xmm}
  ROW(2, 1, 1, 0, 30 , 55 , 0  , 0  , 0  , 0  ), //      {m32|mem, xmm}
  ROW(2, 1, 1, 0, 55 , 30 , 0  , 0  , 0  , 0  ), //      {xmm, m32|mem}
  ROW(3, 1, 1, 0, 55 , 55 , 55 , 0  , 0  , 0  ), //      {xmm, xmm, xmm}
  ROW(3, 1, 1, 0, 55 , 55 , 55 , 0  , 0  , 0  ), //      {xmm, xmm, xmm}
  ROW(3, 1, 1, 0, 55 , 55 , 64 , 0  , 0  , 0  ), // #123 {xmm, xmm, xmm|m128|mem|i8|u8}
  ROW(3, 1, 1, 0, 55 , 59 , 10 , 0  , 0  , 0  ), //      {xmm, m128|mem, i8|u8}
  ROW(3, 1, 1, 0, 57 , 57 , 65 , 0  , 0  , 0  ), //      {ymm, ymm, ymm|m256|mem|i8|u8}
  ROW(3, 1, 1, 0, 57 , 60 , 10 , 0  , 0  , 0  ), //      {ymm, m256|mem, i8|u8}
  ROW(3, 1, 1, 0, 61 , 61 , 66 , 0  , 0  , 0  ), //      {zmm, zmm, zmm|m512|mem|i8|u8}
  ROW(3, 1, 1, 0, 55 , 59 , 10 , 0  , 0  , 0  ), //      {xmm, m128|mem, i8|u8}
  ROW(3, 1, 1, 0, 57 , 60 , 10 , 0  , 0  , 0  ), //      {ymm, m256|mem, i8|u8}
  ROW(3, 1, 1, 0, 61 , 63 , 10 , 0  , 0  , 0  ), //      {zmm, m512|mem, i8|u8}
  ROW(2, 1, 1, 0, 1  , 2  , 0  , 0  , 0  , 0  ), // #131 {r8lo|r8hi|m8|mem, r8lo|r8hi}
  ROW(2, 1, 1, 0, 24 , 4  , 0  , 0  , 0  , 0  ), //      {r16|m16|mem, r16}
  ROW(2, 1, 1, 0, 25 , 6  , 0  , 0  , 0  , 0  ), //      {r32|m32|mem, r32}
  ROW(2, 0, 1, 0, 26 , 8  , 0  , 0  , 0  , 0  ), //      {r64|m64|mem, r64}
  ROW(2, 1, 1, 0, 2  , 17 , 0  , 0  , 0  , 0  ), //      {r8lo|r8hi, m8|mem}
  ROW(2, 1, 1, 0, 4  , 21 , 0  , 0  , 0  , 0  ), //      {r16, m16|mem}
  ROW(2, 1, 1, 0, 6  , 30 , 0  , 0  , 0  , 0  ), //      {r32, m32|mem}
  ROW(2, 0, 1, 0, 8  , 31 , 0  , 0  , 0  , 0  ), //      {r64, m64|mem}
  ROW(2, 1, 1, 0, 4  , 21 , 0  , 0  , 0  , 0  ), // #139 {r16, m16|mem}
  ROW(2, 1, 1, 0, 6  , 30 , 0  , 0  , 0  , 0  ), //      {r32, m32|mem}
  ROW(2, 0, 1, 0, 8  , 31 , 0  , 0  , 0  , 0  ), //      {r64, m64|mem}
  ROW(2, 1, 1, 0, 21 , 4  , 0  , 0  , 0  , 0  ), //      {m16|mem, r16}
  ROW(2, 1, 1, 0, 30 , 6  , 0  , 0  , 0  , 0  ), // #143 {m32|mem, r32}
  ROW(2, 0, 1, 0, 31 , 8  , 0  , 0  , 0  , 0  ), //      {m64|mem, r64}
  ROW(0, 1, 1, 0, 0  , 0  , 0  , 0  , 0  , 0  ), // #145 {}
  ROW(1, 1, 1, 0, 27 , 0  , 0  , 0  , 0  , 0  ), //      {r16|m16|r32|m32}
  ROW(1, 0, 1, 0, 15 , 0  , 0  , 0  , 0  , 0  ), //      {r64|m64}
  ROW(2, 1, 1, 0, 24 , 4  , 0  , 0  , 0  , 0  ), //      {r16|m16|mem, r16}
  ROW(2, 1, 1, 0, 25 , 6  , 0  , 0  , 0  , 0  ), //      {r32|m32|mem, r32}
  ROW(2, 0, 1, 0, 26 , 8  , 0  , 0  , 0  , 0  ), //      {r64|m64|mem, r64}
  ROW(2, 1, 1, 0, 55 , 56 , 0  , 0  , 0  , 0  ), // #151 {xmm, xmm|m128|mem}
  ROW(2, 1, 1, 0, 57 , 58 , 0  , 0  , 0  , 0  ), //      {ymm, ymm|m256|mem}
  ROW(2, 1, 1, 0, 61 , 62 , 0  , 0  , 0  , 0  ), //      {zmm, zmm|m512|mem}
  ROW(2, 1, 1, 0, 59 , 55 , 0  , 0  , 0  , 0  ), //      {m128|mem, xmm}
  ROW(2, 1, 1, 0, 60 , 57 , 0  , 0  , 0  , 0  ), //      {m256|mem, ymm}
  ROW(2, 1, 1, 0, 63 , 61 , 0  , 0  , 0  , 0  ), //      {m512|mem, zmm}
  ROW(3, 1, 1, 0, 55 , 55 , 64 , 0  , 0  , 0  ), // #157 {xmm, xmm, xmm|m128|mem|i8|u8}
  ROW(3, 1, 1, 0, 57 , 57 , 64 , 0  , 0  , 0  ), //      {ymm, ymm, xmm|m128|mem|i8|u8}
  ROW(3, 1, 1, 0, 61 , 61 , 64 , 0  , 0  , 0  ), //      {zmm, zmm, xmm|m128|mem|i8|u8}
  ROW(3, 1, 1, 0, 55 , 59 , 10 , 0  , 0  , 0  ), //      {xmm, m128|mem, i8|u8}
  ROW(3, 1, 1, 0, 57 , 60 , 10 , 0  , 0  , 0  ), //      {ymm, m256|mem, i8|u8}
  ROW(3, 1, 1, 0, 61 , 63 , 10 , 0  , 0  , 0  ), //      {zmm, m512|mem, i8|u8}
  ROW(2, 1, 1, 0, 24 , 4  , 0  , 0  , 0  , 0  ), // #163 {r16|m16|mem, r16}
  ROW(2, 1, 1, 0, 25 , 6  , 0  , 0  , 0  , 0  ), //      {r32|m32|mem, r32}
  ROW(2, 0, 1, 0, 26 , 8  , 0  , 0  , 0  , 0  ), //      {r64|m64|mem, r64}
  ROW(2, 1, 1, 0, 27 , 10 , 0  , 0  , 0  , 0  ), //      {r16|m16|r32|m32, i8|u8}
  ROW(2, 0, 1, 0, 15 , 10 , 0  , 0  , 0  , 0  ), //      {r64|m64, i8|u8}
  ROW(2, 1, 1, 0, 67 , 68 , 0  , 0  , 0  , 0  ), // #168 {mm, mm|m64|mem}
  ROW(2, 0, 1, 0, 69 , 26 , 0  , 0  , 0  , 0  ), //      {mm|xmm, r64|m64|mem}
  ROW(2, 1, 1, 0, 31 , 69 , 0  , 0  , 0  , 0  ), //      {m64|mem, mm|xmm}
  ROW(2, 0, 1, 0, 26 , 69 , 0  , 0  , 0  , 0  ), //      {r64|m64|mem, mm|xmm}
  ROW(2, 1, 1, 0, 55 , 70 , 0  , 0  , 0  , 0  ), // #172 {xmm, xmm|m64|mem}
  ROW(1, 1, 1, 0, 11 , 0  , 0  , 0  , 0  , 0  ), // #173 {r16|m16}
  ROW(1, 1, 0, 0, 13 , 0  , 0  , 0  , 0  , 0  ), //      {r32|m32}
  ROW(1, 0, 1, 0, 15 , 0  , 0  , 0  , 0  , 0  ), //      {r64|m64}
  ROW(1, 1, 0, 0, 71 , 0  , 0  , 0  , 0  , 0  ), //      {ds|es|ss}
  ROW(1, 1, 1, 0, 72 , 0  , 0  , 0  , 0  , 0  ), //      {fs|gs}
  ROW(1, 1, 1, 0, 73 , 0  , 0  , 0  , 0  , 0  ), // #178 {r16|m16|i8|u8|i16|u16}
  ROW(1, 1, 0, 0, 74 , 0  , 0  , 0  , 0  , 0  ), //      {r32|m32|i32|u32}
  ROW(1, 0, 1, 0, 75 , 0  , 0  , 0  , 0  , 0  ), //      {r64|m64|i32}
  ROW(1, 1, 0, 0, 76 , 0  , 0  , 0  , 0  , 0  ), //      {cs|ss|ds|es}
  ROW(1, 1, 1, 0, 72 , 0  , 0  , 0  , 0  , 0  ), //      {fs|gs}
  ROW(3, 1, 1, 0, 55 , 77 , 55 , 0  , 0  , 0  ), // #183 {xmm, vm32x, xmm}
  ROW(3, 1, 1, 0, 57 , 78 , 57 , 0  , 0  , 0  ), //      {ymm, vm32y, ymm}
  ROW(2, 1, 1, 0, 55 , 77 , 0  , 0  , 0  , 0  ), //      {xmm, vm32x}
  ROW(2, 1, 1, 0, 57 , 78 , 0  , 0  , 0  , 0  ), //      {ymm, vm32y}
  ROW(2, 1, 1, 0, 61 , 79 , 0  , 0  , 0  , 0  ), //      {zmm, vm32z}
  ROW(3, 1, 1, 0, 55 , 80 , 55 , 0  , 0  , 0  ), // #188 {xmm, vm64x, xmm}
  ROW(3, 1, 1, 0, 57 , 81 , 57 , 0  , 0  , 0  ), //      {ymm, vm64y, ymm}
  ROW(2, 1, 1, 0, 55 , 80 , 0  , 0  , 0  , 0  ), //      {xmm, vm64x}
  ROW(2, 1, 1, 0, 57 , 81 , 0  , 0  , 0  , 0  ), //      {ymm, vm64y}
  ROW(2, 1, 1, 0, 61 , 82 , 0  , 0  , 0  , 0  ), //      {zmm, vm64z}
  ROW(2, 1, 1, 0, 59 , 55 , 0  , 0  , 0  , 0  ), // #193 {m128|mem, xmm}
  ROW(2, 1, 1, 0, 60 , 57 , 0  , 0  , 0  , 0  ), //      {m256|mem, ymm}
  ROW(2, 1, 1, 0, 59 , 55 , 0  , 0  , 0  , 0  ), //      {m128|mem, xmm}
  ROW(2, 1, 1, 0, 60 , 57 , 0  , 0  , 0  , 0  ), //      {m256|mem, ymm}
  ROW(2, 1, 1, 0, 63 , 61 , 0  , 0  , 0  , 0  ), //      {m512|mem, zmm}
  ROW(2, 1, 1, 0, 55 , 59 , 0  , 0  , 0  , 0  ), // #198 {xmm, m128|mem}
  ROW(2, 1, 1, 0, 57 , 60 , 0  , 0  , 0  , 0  ), //      {ymm, m256|mem}
  ROW(2, 1, 1, 0, 55 , 59 , 0  , 0  , 0  , 0  ), //      {xmm, m128|mem}
  ROW(2, 1, 1, 0, 57 , 60 , 0  , 0  , 0  , 0  ), //      {ymm, m256|mem}
  ROW(2, 1, 1, 0, 61 , 63 , 0  , 0  , 0  , 0  ), //      {zmm, m512|mem}
  ROW(2, 0, 1, 0, 26 , 55 , 0  , 0  , 0  , 0  ), // #203 {r64|m64|mem, xmm}
  ROW(2, 1, 1, 0, 55 , 70 , 0  , 0  , 0  , 0  ), //      {xmm, xmm|m64|mem}
  ROW(2, 0, 1, 0, 55 , 26 , 0  , 0  , 0  , 0  ), //      {xmm, r64|m64|mem}
  ROW(2, 1, 1, 0, 31 , 55 , 0  , 0  , 0  , 0  ), //      {m64|mem, xmm}
  ROW(2, 1, 1, 0, 31 , 55 , 0  , 0  , 0  , 0  ), //      {m64|mem, xmm}
  ROW(2, 1, 1, 0, 83 , 84 , 0  , 0  , 0  , 0  ), // #208 {ds:[memBase|zsi|m8], es:[memBase|zdi|m8]}
  ROW(2, 1, 1, 0, 85 , 86 , 0  , 0  , 0  , 0  ), //      {ds:[memBase|zsi|m16], es:[memBase|zdi|m16]}
  ROW(2, 1, 1, 0, 87 , 88 , 0  , 0  , 0  , 0  ), //      {ds:[memBase|zsi|m32], es:[memBase|zdi|m32]}
  ROW(2, 0, 1, 0, 89 , 90 , 0  , 0  , 0  , 0  ), //      {ds:[memBase|zsi|m64], es:[memBase|zdi|m64]}
  ROW(3, 1, 1, 1, 1  , 2  , 91 , 0  , 0  , 0  ), // #212 {r8lo|r8hi|m8|mem, r8lo|r8hi, <al>}
  ROW(3, 1, 1, 1, 24 , 4  , 42 , 0  , 0  , 0  ), //      {r16|m16|mem, r16, <ax>}
  ROW(3, 1, 1, 1, 25 , 6  , 45 , 0  , 0  , 0  ), //      {r32|m32|mem, r32, <eax>}
  ROW(3, 0, 1, 1, 26 , 8  , 47 , 0  , 0  , 0  ), //      {r64|m64|mem, r64, <rax>}
  ROW(2, 1, 1, 0, 92 , 93 , 0  , 0  , 0  , 0  ), // #216 {k, k|m64|mem}
  ROW(2, 0, 1, 0, 92 , 8  , 0  , 0  , 0  , 0  ), //      {k, r64}
  ROW(2, 1, 1, 0, 31 , 92 , 0  , 0  , 0  , 0  ), //      {m64|mem, k}
  ROW(2, 0, 1, 0, 8  , 92 , 0  , 0  , 0  , 0  ), //      {r64, k}
  ROW(2, 1, 1, 0, 51 , 94 , 0  , 0  , 0  , 0  ), // #220 {al, ds:[memBase|zsi|m8|mem]}
  ROW(2, 1, 1, 0, 52 , 95 , 0  , 0  , 0  , 0  ), //      {ax, ds:[memBase|zsi|m16|mem]}
  ROW(2, 1, 1, 0, 53 , 96 , 0  , 0  , 0  , 0  ), //      {eax, ds:[memBase|zsi|m32|mem]}
  ROW(2, 0, 1, 0, 54 , 97 , 0  , 0  , 0  , 0  ), //      {rax, ds:[memBase|zsi|m64|mem]}
  ROW(2, 1, 1, 0, 84 , 83 , 0  , 0  , 0  , 0  ), // #224 {es:[memBase|zdi|m8], ds:[memBase|zsi|m8]}
  ROW(2, 1, 1, 0, 86 , 85 , 0  , 0  , 0  , 0  ), //      {es:[memBase|zdi|m16], ds:[memBase|zsi|m16]}
  ROW(2, 1, 1, 0, 88 , 87 , 0  , 0  , 0  , 0  ), //      {es:[memBase|zdi|m32], ds:[memBase|zsi|m32]}
  ROW(2, 0, 1, 0, 90 , 89 , 0  , 0  , 0  , 0  ), //      {es:[memBase|zdi|m64], ds:[memBase|zsi|m64]}
  ROW(2, 1, 1, 0, 51 , 98 , 0  , 0  , 0  , 0  ), // #228 {al, es:[memBase|zdi|m8|mem]}
  ROW(2, 1, 1, 0, 52 , 99 , 0  , 0  , 0  , 0  ), //      {ax, es:[memBase|zdi|m16|mem]}
  ROW(2, 1, 1, 0, 53 , 100, 0  , 0  , 0  , 0  ), //      {eax, es:[memBase|zdi|m32|mem]}
  ROW(2, 0, 1, 0, 54 , 101, 0  , 0  , 0  , 0  ), //      {rax, es:[memBase|zdi|m64|mem]}
  ROW(2, 1, 1, 0, 98 , 51 , 0  , 0  , 0  , 0  ), // #232 {es:[memBase|zdi|m8|mem], al}
  ROW(2, 1, 1, 0, 99 , 52 , 0  , 0  , 0  , 0  ), //      {es:[memBase|zdi|m16|mem], ax}
  ROW(2, 1, 1, 0, 100, 53 , 0  , 0  , 0  , 0  ), //      {es:[memBase|zdi|m32|mem], eax}
  ROW(2, 0, 1, 0, 101, 54 , 0  , 0  , 0  , 0  ), //      {es:[memBase|zdi|m64|mem], rax}
  ROW(4, 1, 1, 0, 55 , 55 , 55 , 56 , 0  , 0  ), // #236 {xmm, xmm, xmm, xmm|m128|mem}
  ROW(4, 1, 1, 0, 57 , 57 , 57 , 58 , 0  , 0  ), //      {ymm, ymm, ymm, ymm|m256|mem}
  ROW(4, 1, 1, 0, 55 , 55 , 59 , 55 , 0  , 0  ), //      {xmm, xmm, m128|mem, xmm}
  ROW(4, 1, 1, 0, 57 , 57 , 60 , 57 , 0  , 0  ), //      {ymm, ymm, m256|mem, ymm}
  ROW(3, 1, 1, 0, 55 , 77 , 55 , 0  , 0  , 0  ), // #240 {xmm, vm32x, xmm}
  ROW(3, 1, 1, 0, 57 , 77 , 57 , 0  , 0  , 0  ), //      {ymm, vm32x, ymm}
  ROW(2, 1, 1, 0, 102, 77 , 0  , 0  , 0  , 0  ), //      {xmm|ymm, vm32x}
  ROW(2, 1, 1, 0, 61 , 78 , 0  , 0  , 0  , 0  ), //      {zmm, vm32y}
  ROW(3, 1, 1, 0, 59 , 55 , 55 , 0  , 0  , 0  ), // #244 {m128|mem, xmm, xmm}
  ROW(3, 1, 1, 0, 60 , 57 , 57 , 0  , 0  , 0  ), //      {m256|mem, ymm, ymm}
  ROW(3, 1, 1, 0, 55 , 55 , 59 , 0  , 0  , 0  ), //      {xmm, xmm, m128|mem}
  ROW(3, 1, 1, 0, 57 , 57 , 60 , 0  , 0  , 0  ), //      {ymm, ymm, m256|mem}
  ROW(2, 1, 1, 0, 31 , 55 , 0  , 0  , 0  , 0  ), // #248 {m64|mem, xmm}
  ROW(3, 1, 1, 0, 55 , 55 , 31 , 0  , 0  , 0  ), //      {xmm, xmm, m64|mem}
  ROW(2, 1, 1, 0, 31 , 55 , 0  , 0  , 0  , 0  ), //      {m64|mem, xmm}
  ROW(3, 1, 1, 0, 55 , 55 , 31 , 0  , 0  , 0  ), //      {xmm, xmm, m64|mem}
  ROW(2, 1, 1, 0, 21 , 55 , 0  , 0  , 0  , 0  ), // #252 {m16|mem, xmm}
  ROW(2, 1, 1, 0, 55 , 21 , 0  , 0  , 0  , 0  ), //      {xmm, m16|mem}
  ROW(3, 1, 1, 0, 55 , 55 , 55 , 0  , 0  , 0  ), //      {xmm, xmm, xmm}
  ROW(3, 1, 1, 0, 55 , 55 , 55 , 0  , 0  , 0  ), //      {xmm, xmm, xmm}
  ROW(4, 1, 1, 0, 55 , 55 , 55 , 56 , 0  , 0  ), // #256 {xmm, xmm, xmm, xmm|m128|mem}
  ROW(4, 1, 1, 0, 55 , 55 , 59 , 55 , 0  , 0  ), //      {xmm, xmm, m128|mem, xmm}
  ROW(4, 1, 1, 0, 57 , 57 , 57 , 58 , 0  , 0  ), //      {ymm, ymm, ymm, ymm|m256|mem}
  ROW(4, 1, 1, 0, 57 , 57 , 60 , 57 , 0  , 0  ), //      {ymm, ymm, m256|mem, ymm}
  ROW(5, 1, 1, 0, 55 , 55 , 56 , 55 , 103, 0  ), // #260 {xmm, xmm, xmm|m128|mem, xmm, i4|u4}
  ROW(5, 1, 1, 0, 55 , 55 , 55 , 59 , 103, 0  ), //      {xmm, xmm, xmm, m128|mem, i4|u4}
  ROW(5, 1, 1, 0, 57 , 57 , 58 , 57 , 103, 0  ), //      {ymm, ymm, ymm|m256|mem, ymm, i4|u4}
  ROW(5, 1, 1, 0, 57 , 57 , 57 , 60 , 103, 0  ), //      {ymm, ymm, ymm, m256|mem, i4|u4}
  ROW(3, 1, 1, 0, 57 , 58 , 10 , 0  , 0  , 0  ), // #264 {ymm, ymm|m256|mem, i8|u8}
  ROW(3, 1, 1, 0, 57 , 57 , 58 , 0  , 0  , 0  ), //      {ymm, ymm, ymm|m256|mem}
  ROW(3, 1, 1, 0, 61 , 61 , 66 , 0  , 0  , 0  ), //      {zmm, zmm, zmm|m512|mem|i8|u8}
  ROW(3, 1, 1, 0, 61 , 63 , 10 , 0  , 0  , 0  ), //      {zmm, m512|mem, i8|u8}
  ROW(1, 1, 0, 0, 104, 0  , 0  , 0  , 0  , 0  ), // #268 {rel16|r16|m16|mem|r32|m32}
  ROW(1, 1, 1, 0, 105, 0  , 0  , 0  , 0  , 0  ), // #269 {rel32}
  ROW(1, 0, 1, 0, 26 , 0  , 0  , 0  , 0  , 0  ), //      {r64|m64|mem}
  ROW(1, 1, 0, 0, 106, 0  , 0  , 0  , 0  , 0  ), // #271 {r16|r32}
  ROW(1, 1, 1, 0, 107, 0  , 0  , 0  , 0  , 0  ), // #272 {r8lo|r8hi|m8|r16|m16|r32|m32}
  ROW(1, 0, 1, 0, 15 , 0  , 0  , 0  , 0  , 0  ), //      {r64|m64}
  ROW(1, 1, 1, 0, 108, 0  , 0  , 0  , 0  , 0  ), // #274 {m32|m64}
  ROW(2, 1, 1, 0, 109, 110, 0  , 0  , 0  , 0  ), //      {st0, st}
  ROW(2, 1, 1, 0, 110, 109, 0  , 0  , 0  , 0  ), //      {st, st0}
  ROW(1, 1, 1, 0, 111, 0  , 0  , 0  , 0  , 0  ), // #277 {rel8|rel32}
  ROW(1, 1, 0, 0, 112, 0  , 0  , 0  , 0  , 0  ), //      {rel16|r32|m32}
  ROW(1, 0, 1, 0, 15 , 0  , 0  , 0  , 0  , 0  ), //      {r64|m64}
  ROW(2, 1, 0, 0, 12 , 113, 0  , 0  , 0  , 0  ), // #280 {i16|u16, i16|u16|i32|u32}
  ROW(1, 1, 1, 0, 114, 0  , 0  , 0  , 0  , 0  ), //      {m32|mem|m48}
  ROW(1, 0, 1, 0, 115, 0  , 0  , 0  , 0  , 0  ), //      {m80|mem}
  ROW(2, 1, 1, 0, 4  , 30 , 0  , 0  , 0  , 0  ), // #283 {r16, m32|mem}
  ROW(2, 1, 1, 0, 6  , 116, 0  , 0  , 0  , 0  ), //      {r32, m48|mem}
  ROW(2, 0, 1, 0, 8  , 115, 0  , 0  , 0  , 0  ), //      {r64, m80|mem}
  ROW(2, 1, 1, 0, 4  , 24 , 0  , 0  , 0  , 0  ), // #286 {r16, r16|m16|mem}
  ROW(2, 1, 1, 0, 6  , 117, 0  , 0  , 0  , 0  ), //      {r32, r32|m16|mem}
  ROW(2, 0, 1, 0, 8  , 117, 0  , 0  , 0  , 0  ), //      {r64, r32|m16|mem}
  ROW(2, 1, 1, 0, 4  , 9  , 0  , 0  , 0  , 0  ), // #289 {r16, r8lo|r8hi|m8}
  ROW(2, 1, 1, 0, 6  , 118, 0  , 0  , 0  , 0  ), //      {r32, r8lo|r8hi|m8|r16|m16}
  ROW(2, 0, 1, 0, 8  , 118, 0  , 0  , 0  , 0  ), //      {r64, r8lo|r8hi|m8|r16|m16}
  ROW(3, 1, 1, 0, 24 , 4  , 119, 0  , 0  , 0  ), // #292 {r16|m16|mem, r16, cl|i8|u8}
  ROW(3, 1, 1, 0, 25 , 6  , 119, 0  , 0  , 0  ), //      {r32|m32|mem, r32, cl|i8|u8}
  ROW(3, 0, 1, 0, 26 , 8  , 119, 0  , 0  , 0  ), //      {r64|m64|mem, r64, cl|i8|u8}
  ROW(3, 1, 1, 0, 55 , 55 , 56 , 0  , 0  , 0  ), // #295 {xmm, xmm, xmm|m128|mem}
  ROW(3, 1, 1, 0, 57 , 57 , 58 , 0  , 0  , 0  ), // #296 {ymm, ymm, ymm|m256|mem}
  ROW(3, 1, 1, 0, 61 , 61 , 62 , 0  , 0  , 0  ), //      {zmm, zmm, zmm|m512|mem}
  ROW(4, 1, 1, 0, 55 , 55 , 56 , 10 , 0  , 0  ), // #298 {xmm, xmm, xmm|m128|mem, i8|u8}
  ROW(4, 1, 1, 0, 57 , 57 , 58 , 10 , 0  , 0  ), // #299 {ymm, ymm, ymm|m256|mem, i8|u8}
  ROW(4, 1, 1, 0, 61 , 61 , 62 , 10 , 0  , 0  ), //      {zmm, zmm, zmm|m512|mem, i8|u8}
  ROW(4, 1, 1, 0, 120, 55 , 56 , 10 , 0  , 0  ), // #301 {xmm|k, xmm, xmm|m128|mem, i8|u8}
  ROW(4, 1, 1, 0, 121, 57 , 58 , 10 , 0  , 0  ), //      {ymm|k, ymm, ymm|m256|mem, i8|u8}
  ROW(4, 1, 1, 0, 92 , 61 , 62 , 10 , 0  , 0  ), //      {k, zmm, zmm|m512|mem, i8|u8}
  ROW(4, 1, 1, 0, 92 , 55 , 56 , 10 , 0  , 0  ), // #304 {k, xmm, xmm|m128|mem, i8|u8}
  ROW(4, 1, 1, 0, 92 , 57 , 58 , 10 , 0  , 0  ), //      {k, ymm, ymm|m256|mem, i8|u8}
  ROW(4, 1, 1, 0, 92 , 61 , 62 , 10 , 0  , 0  ), //      {k, zmm, zmm|m512|mem, i8|u8}
  ROW(2, 1, 1, 0, 56 , 55 , 0  , 0  , 0  , 0  ), // #307 {xmm|m128|mem, xmm}
  ROW(2, 1, 1, 0, 58 , 57 , 0  , 0  , 0  , 0  ), //      {ymm|m256|mem, ymm}
  ROW(2, 1, 1, 0, 62 , 61 , 0  , 0  , 0  , 0  ), //      {zmm|m512|mem, zmm}
  ROW(2, 1, 1, 0, 55 , 70 , 0  , 0  , 0  , 0  ), // #310 {xmm, xmm|m64|mem}
  ROW(2, 1, 1, 0, 57 , 56 , 0  , 0  , 0  , 0  ), //      {ymm, xmm|m128|mem}
  ROW(2, 1, 1, 0, 61 , 58 , 0  , 0  , 0  , 0  ), //      {zmm, ymm|m256|mem}
  ROW(2, 1, 1, 0, 55 , 122, 0  , 0  , 0  , 0  ), // #313 {xmm, xmm|m32|mem}
  ROW(2, 1, 1, 0, 57 , 70 , 0  , 0  , 0  , 0  ), //      {ymm, xmm|m64|mem}
  ROW(2, 1, 1, 0, 61 , 56 , 0  , 0  , 0  , 0  ), //      {zmm, xmm|m128|mem}
  ROW(3, 1, 1, 0, 70 , 55 , 10 , 0  , 0  , 0  ), // #316 {xmm|m64|mem, xmm, i8|u8}
  ROW(3, 1, 1, 0, 56 , 57 , 10 , 0  , 0  , 0  ), // #317 {xmm|m128|mem, ymm, i8|u8}
  ROW(3, 1, 1, 0, 58 , 61 , 10 , 0  , 0  , 0  ), // #318 {ymm|m256|mem, zmm, i8|u8}
  ROW(3, 1, 1, 0, 55 , 123, 55 , 0  , 0  , 0  ), // #319 {xmm, vm64x|vm64y, xmm}
  ROW(2, 1, 1, 0, 55 , 123, 0  , 0  , 0  , 0  ), //      {xmm, vm64x|vm64y}
  ROW(2, 1, 1, 0, 57 , 82 , 0  , 0  , 0  , 0  ), //      {ymm, vm64z}
  ROW(3, 1, 1, 0, 55 , 56 , 10 , 0  , 0  , 0  ), // #322 {xmm, xmm|m128|mem, i8|u8}
  ROW(3, 1, 1, 0, 57 , 58 , 10 , 0  , 0  , 0  ), //      {ymm, ymm|m256|mem, i8|u8}
  ROW(3, 1, 1, 0, 61 , 62 , 10 , 0  , 0  , 0  ), //      {zmm, zmm|m512|mem, i8|u8}
  ROW(2, 1, 1, 0, 55 , 70 , 0  , 0  , 0  , 0  ), // #325 {xmm, xmm|m64|mem}
  ROW(2, 1, 1, 0, 57 , 58 , 0  , 0  , 0  , 0  ), //      {ymm, ymm|m256|mem}
  ROW(2, 1, 1, 0, 61 , 62 , 0  , 0  , 0  , 0  ), //      {zmm, zmm|m512|mem}
  ROW(4, 1, 1, 0, 92 , 92 , 55 , 56 , 0  , 0  ), // #328 {k, k, xmm, xmm|m128|mem}
  ROW(4, 1, 1, 0, 92 , 92 , 57 , 58 , 0  , 0  ), //      {k, k, ymm, ymm|m256|mem}
  ROW(4, 1, 1, 0, 92 , 92 , 61 , 62 , 0  , 0  ), //      {k, k, zmm, zmm|m512|mem}
  ROW(3, 1, 1, 0, 120, 55 , 56 , 0  , 0  , 0  ), // #331 {xmm|k, xmm, xmm|m128|mem}
  ROW(3, 1, 1, 0, 121, 57 , 58 , 0  , 0  , 0  ), //      {ymm|k, ymm, ymm|m256|mem}
  ROW(3, 1, 1, 0, 92 , 61 , 62 , 0  , 0  , 0  ), //      {k, zmm, zmm|m512|mem}
  ROW(2, 1, 1, 0, 122, 55 , 0  , 0  , 0  , 0  ), // #334 {xmm|m32|mem, xmm}
  ROW(2, 1, 1, 0, 70 , 57 , 0  , 0  , 0  , 0  ), //      {xmm|m64|mem, ymm}
  ROW(2, 1, 1, 0, 56 , 61 , 0  , 0  , 0  , 0  ), //      {xmm|m128|mem, zmm}
  ROW(2, 1, 1, 0, 70 , 55 , 0  , 0  , 0  , 0  ), // #337 {xmm|m64|mem, xmm}
  ROW(2, 1, 1, 0, 56 , 57 , 0  , 0  , 0  , 0  ), //      {xmm|m128|mem, ymm}
  ROW(2, 1, 1, 0, 58 , 61 , 0  , 0  , 0  , 0  ), //      {ymm|m256|mem, zmm}
  ROW(2, 1, 1, 0, 124, 55 , 0  , 0  , 0  , 0  ), // #340 {xmm|m16|mem, xmm}
  ROW(2, 1, 1, 0, 122, 57 , 0  , 0  , 0  , 0  ), //      {xmm|m32|mem, ymm}
  ROW(2, 1, 1, 0, 70 , 61 , 0  , 0  , 0  , 0  ), //      {xmm|m64|mem, zmm}
  ROW(2, 1, 1, 0, 55 , 124, 0  , 0  , 0  , 0  ), // #343 {xmm, xmm|m16|mem}
  ROW(2, 1, 1, 0, 57 , 122, 0  , 0  , 0  , 0  ), //      {ymm, xmm|m32|mem}
  ROW(2, 1, 1, 0, 61 , 70 , 0  , 0  , 0  , 0  ), //      {zmm, xmm|m64|mem}
  ROW(2, 1, 1, 0, 77 , 55 , 0  , 0  , 0  , 0  ), // #346 {vm32x, xmm}
  ROW(2, 1, 1, 0, 78 , 57 , 0  , 0  , 0  , 0  ), //      {vm32y, ymm}
  ROW(2, 1, 1, 0, 79 , 61 , 0  , 0  , 0  , 0  ), //      {vm32z, zmm}
  ROW(2, 1, 1, 0, 80 , 55 , 0  , 0  , 0  , 0  ), // #349 {vm64x, xmm}
  ROW(2, 1, 1, 0, 81 , 57 , 0  , 0  , 0  , 0  ), //      {vm64y, ymm}
  ROW(2, 1, 1, 0, 82 , 61 , 0  , 0  , 0  , 0  ), //      {vm64z, zmm}
  ROW(3, 1, 1, 0, 92 , 55 , 56 , 0  , 0  , 0  ), // #352 {k, xmm, xmm|m128|mem}
  ROW(3, 1, 1, 0, 92 , 57 , 58 , 0  , 0  , 0  ), //      {k, ymm, ymm|m256|mem}
  ROW(3, 1, 1, 0, 92 , 61 , 62 , 0  , 0  , 0  ), //      {k, zmm, zmm|m512|mem}
  ROW(3, 1, 1, 0, 6  , 6  , 25 , 0  , 0  , 0  ), // #355 {r32, r32, r32|m32|mem}
  ROW(3, 0, 1, 0, 8  , 8  , 26 , 0  , 0  , 0  ), //      {r64, r64, r64|m64|mem}
  ROW(3, 1, 1, 0, 6  , 25 , 6  , 0  , 0  , 0  ), // #357 {r32, r32|m32|mem, r32}
  ROW(3, 0, 1, 0, 8  , 26 , 8  , 0  , 0  , 0  ), //      {r64, r64|m64|mem, r64}
  ROW(2, 1, 0, 0, 125, 25 , 0  , 0  , 0  , 0  ), // #359 {bnd, r32|m32|mem}
  ROW(2, 0, 1, 0, 125, 26 , 0  , 0  , 0  , 0  ), //      {bnd, r64|m64|mem}
  ROW(2, 1, 1, 0, 125, 126, 0  , 0  , 0  , 0  ), // #361 {bnd, bnd|mem}
  ROW(2, 1, 1, 0, 127, 125, 0  , 0  , 0  , 0  ), //      {mem, bnd}
  ROW(2, 1, 0, 0, 4  , 30 , 0  , 0  , 0  , 0  ), // #363 {r16, m32|mem}
  ROW(2, 1, 0, 0, 6  , 31 , 0  , 0  , 0  , 0  ), //      {r32, m64|mem}
  ROW(1, 1, 1, 0, 106, 0  , 0  , 0  , 0  , 0  ), // #365 {r16|r32}
  ROW(1, 0, 1, 0, 8  , 0  , 0  , 0  , 0  , 0  ), // #366 {r64}
  ROW(3, 1, 1, 0, 30 , 6  , 6  , 0  , 0  , 0  ), // #367 {m32|mem, r32, r32}
  ROW(3, 0, 1, 0, 31 , 8  , 8  , 0  , 0  , 0  ), //      {m64|mem, r64, r64}
  ROW(2, 1, 1, 0, 6  , 107, 0  , 0  , 0  , 0  ), // #369 {r32, r8lo|r8hi|m8|r16|m16|r32|m32}
  ROW(2, 0, 1, 0, 8  , 128, 0  , 0  , 0  , 0  ), //      {r64, r8lo|r8hi|m8|r64|m64}
  ROW(2, 1, 1, 0, 6  , 70 , 0  , 0  , 0  , 0  ), // #371 {r32, xmm|m64|mem}
  ROW(2, 0, 1, 0, 8  , 70 , 0  , 0  , 0  , 0  ), //      {r64, xmm|m64|mem}
  ROW(2, 1, 1, 0, 55 , 25 , 0  , 0  , 0  , 0  ), // #373 {xmm, r32|m32|mem}
  ROW(2, 0, 1, 0, 55 , 26 , 0  , 0  , 0  , 0  ), //      {xmm, r64|m64|mem}
  ROW(2, 1, 1, 0, 6  , 122, 0  , 0  , 0  , 0  ), // #375 {r32, xmm|m32|mem}
  ROW(2, 0, 1, 0, 8  , 122, 0  , 0  , 0  , 0  ), //      {r64, xmm|m32|mem}
  ROW(2, 1, 0, 0, 129, 63 , 0  , 0  , 0  , 0  ), // #377 {es:[mem|m512|memBase], m512|mem}
  ROW(2, 0, 1, 0, 129, 63 , 0  , 0  , 0  , 0  ), //      {es:[mem|m512|memBase], m512|mem}
  ROW(3, 1, 1, 0, 55 , 10 , 10 , 0  , 0  , 0  ), // #379 {xmm, i8|u8, i8|u8}
  ROW(2, 1, 1, 0, 55 , 55 , 0  , 0  , 0  , 0  ), // #380 {xmm, xmm}
  ROW(0, 1, 1, 0, 0  , 0  , 0  , 0  , 0  , 0  ), // #381 {}
  ROW(1, 1, 1, 0, 110, 0  , 0  , 0  , 0  , 0  ), // #382 {st}
  ROW(0, 1, 1, 0, 0  , 0  , 0  , 0  , 0  , 0  ), // #383 {}
  ROW(1, 1, 1, 0, 130, 0  , 0  , 0  , 0  , 0  ), // #384 {m32|m64|st}
  ROW(2, 1, 1, 0, 55 , 55 , 0  , 0  , 0  , 0  ), // #385 {xmm, xmm}
  ROW(4, 1, 1, 0, 55 , 55 , 10 , 10 , 0  , 0  ), //      {xmm, xmm, i8|u8, i8|u8}
  ROW(2, 1, 0, 0, 6  , 59 , 0  , 0  , 0  , 0  ), // #387 {r32, m128|mem}
  ROW(2, 0, 1, 0, 8  , 59 , 0  , 0  , 0  , 0  ), //      {r64, m128|mem}
  ROW(2, 1, 0, 2, 45 , 131, 0  , 0  , 0  , 0  ), // #389 {<eax>, <ecx>}
  ROW(2, 0, 1, 2, 132, 131, 0  , 0  , 0  , 0  ), //      {<eax|rax>, <ecx>}
  ROW(3, 1, 0, 3, 45 , 44 , 131, 0  , 0  , 0  ), // #391 {<eax>, <edx>, <ecx>}
  ROW(3, 0, 1, 3, 132, 44 , 131, 0  , 0  , 0  ), //      {<eax|rax>, <edx>, <ecx>}
  ROW(1, 1, 1, 0, 111, 0  , 0  , 0  , 0  , 0  ), // #393 {rel8|rel32}
  ROW(1, 1, 0, 0, 105, 0  , 0  , 0  , 0  , 0  ), //      {rel16}
  ROW(2, 1, 0, 1, 133, 134, 0  , 0  , 0  , 0  ), // #395 {<cx|ecx>, rel8}
  ROW(2, 0, 1, 1, 135, 134, 0  , 0  , 0  , 0  ), //      {<ecx|rcx>, rel8}
  ROW(2, 1, 1, 0, 92 , 136, 0  , 0  , 0  , 0  ), // #397 {k, k|m8|mem|r32}
  ROW(2, 1, 1, 0, 137, 92 , 0  , 0  , 0  , 0  ), //      {m8|mem|r32, k}
  ROW(2, 1, 1, 0, 92 , 138, 0  , 0  , 0  , 0  ), // #399 {k, k|m32|mem|r32}
  ROW(2, 1, 1, 0, 25 , 92 , 0  , 0  , 0  , 0  ), //      {m32|mem|r32, k}
  ROW(2, 1, 1, 0, 92 , 139, 0  , 0  , 0  , 0  ), // #401 {k, k|m16|mem|r32}
  ROW(2, 1, 1, 0, 117, 92 , 0  , 0  , 0  , 0  ), //      {m16|mem|r32, k}
  ROW(2, 1, 0, 0, 4  , 30 , 0  , 0  , 0  , 0  ), // #403 {r16, m32|mem}
  ROW(2, 1, 0, 0, 6  , 116, 0  , 0  , 0  , 0  ), //      {r32, m48|mem}
  ROW(2, 1, 1, 0, 106, 140, 0  , 0  , 0  , 0  ), // #405 {r16|r32, mem|m8|m16|m32|m48|m64|m80|m128|m256|m512|m1024}
  ROW(2, 0, 1, 0, 8  , 140, 0  , 0  , 0  , 0  ), //      {r64, mem|m8|m16|m32|m48|m64|m80|m128|m256|m512|m1024}
  ROW(1, 1, 1, 0, 6  , 0  , 0  , 0  , 0  , 0  ), // #407 {r32}
  ROW(1, 0, 1, 0, 8  , 0  , 0  , 0  , 0  , 0  ), //      {r64}
  ROW(3, 1, 1, 0, 6  , 25 , 14 , 0  , 0  , 0  ), // #409 {r32, r32|m32|mem, i32|u32}
  ROW(3, 0, 1, 0, 8  , 25 , 14 , 0  , 0  , 0  ), //      {r64, r32|m32|mem, i32|u32}
  ROW(2, 1, 1, 0, 55 , 56 , 0  , 0  , 0  , 0  ), // #411 {xmm, xmm|m128|mem}
  ROW(2, 1, 1, 0, 59 , 55 , 0  , 0  , 0  , 0  ), //      {m128|mem, xmm}
  ROW(2, 1, 1, 0, 69 , 25 , 0  , 0  , 0  , 0  ), // #413 {mm|xmm, r32|m32|mem}
  ROW(2, 1, 1, 0, 25 , 69 , 0  , 0  , 0  , 0  ), //      {r32|m32|mem, mm|xmm}
  ROW(2, 1, 1, 0, 129, 63 , 0  , 0  , 0  , 0  ), // #415 {es:[mem|m512|memBase], m512|mem}
  ROW(2, 1, 1, 0, 129, 63 , 0  , 0  , 0  , 0  ), //      {es:[mem|m512|memBase], m512|mem}
  ROW(2, 1, 1, 0, 55 , 70 , 0  , 0  , 0  , 0  ), // #417 {xmm, xmm|m64|mem}
  ROW(2, 1, 1, 0, 31 , 55 , 0  , 0  , 0  , 0  ), //      {m64|mem, xmm}
  ROW(2, 1, 1, 0, 55 , 122, 0  , 0  , 0  , 0  ), // #419 {xmm, xmm|m32|mem}
  ROW(2, 1, 1, 0, 30 , 55 , 0  , 0  , 0  , 0  ), //      {m32|mem, xmm}
  ROW(2, 0, 1, 0, 4  , 24 , 0  , 0  , 0  , 0  ), // #421 {r16, r16|m16|mem}
  ROW(2, 0, 1, 0, 141, 25 , 0  , 0  , 0  , 0  ), //      {r32|r64, r32|m32|mem}
  ROW(4, 1, 1, 1, 6  , 6  , 25 , 44 , 0  , 0  ), // #423 {r32, r32, r32|m32|mem, <edx>}
  ROW(4, 0, 1, 1, 8  , 8  , 26 , 46 , 0  , 0  ), //      {r64, r64, r64|m64|mem, <rdx>}
  ROW(2, 1, 1, 0, 67 , 68 , 0  , 0  , 0  , 0  ), // #425 {mm, mm|m64|mem}
  ROW(2, 1, 1, 0, 55 , 56 , 0  , 0  , 0  , 0  ), //      {xmm, xmm|m128|mem}
  ROW(3, 1, 1, 0, 67 , 68 , 10 , 0  , 0  , 0  ), // #427 {mm, mm|m64|mem, i8|u8}
  ROW(3, 1, 1, 0, 55 , 56 , 10 , 0  , 0  , 0  ), //      {xmm, xmm|m128|mem, i8|u8}
  ROW(3, 1, 1, 0, 6  , 69 , 10 , 0  , 0  , 0  ), // #429 {r32, mm|xmm, i8|u8}
  ROW(3, 1, 1, 0, 21 , 55 , 10 , 0  , 0  , 0  ), //      {m16|mem, xmm, i8|u8}
  ROW(2, 1, 1, 0, 67 , 142, 0  , 0  , 0  , 0  ), // #431 {mm, mm|m64|mem|i8|u8}
  ROW(2, 1, 1, 0, 55 , 64 , 0  , 0  , 0  , 0  ), //      {xmm, xmm|m128|mem|i8|u8}
  ROW(1, 1, 1, 0, 25 , 0  , 0  , 0  , 0  , 0  ), // #433 {r32|m32|mem}
  ROW(1, 0, 1, 0, 26 , 0  , 0  , 0  , 0  , 0  ), //      {r64|m64|mem}
  ROW(2, 1, 1, 0, 67 , 143, 0  , 0  , 0  , 0  ), // #435 {mm, mm|m32|mem}
  ROW(2, 1, 1, 0, 55 , 56 , 0  , 0  , 0  , 0  ), //      {xmm, xmm|m128|mem}
  ROW(2, 1, 1, 0, 107, 119, 0  , 0  , 0  , 0  ), // #437 {r8lo|r8hi|m8|r16|m16|r32|m32, cl|i8|u8}
  ROW(2, 0, 1, 0, 15 , 119, 0  , 0  , 0  , 0  ), //      {r64|m64, cl|i8|u8}
  ROW(3, 1, 1, 3, 44 , 45 , 131, 0  , 0  , 0  ), // #439 {<edx>, <eax>, <ecx>}
  ROW(2, 0, 1, 0, 8  , 14 , 0  , 0  , 0  , 0  ), //      {r64, i32|u32}
  ROW(1, 1, 0, 0, 6  , 0  , 0  , 0  , 0  , 0  ), // #441 {r32}
  ROW(1, 0, 1, 0, 8  , 0  , 0  , 0  , 0  , 0  ), //      {r64}
  ROW(0, 1, 1, 0, 0  , 0  , 0  , 0  , 0  , 0  ), // #443 {}
  ROW(1, 1, 1, 0, 144, 0  , 0  , 0  , 0  , 0  ), //      {u16}
  ROW(3, 1, 1, 0, 6  , 25 , 10 , 0  , 0  , 0  ), // #445 {r32, r32|m32|mem, i8|u8}
  ROW(3, 0, 1, 0, 8  , 26 , 10 , 0  , 0  , 0  ), //      {r64, r64|m64|mem, i8|u8}
  ROW(1, 1, 1, 0, 145, 0  , 0  , 0  , 0  , 0  ), // #447 {r16|m16|mem|r32}
  ROW(1, 0, 1, 0, 146, 0  , 0  , 0  , 0  , 0  ), //      {r64|m16|mem}
  ROW(1, 1, 0, 0, 147, 0  , 0  , 0  , 0  , 0  ), // #449 {ds:[mem|memBase]}
  ROW(1, 0, 1, 0, 147, 0  , 0  , 0  , 0  , 0  ), //      {ds:[mem|memBase]}
  ROW(4, 1, 1, 0, 55 , 55 , 56 , 55 , 0  , 0  ), // #451 {xmm, xmm, xmm|m128|mem, xmm}
  ROW(4, 1, 1, 0, 57 , 57 , 58 , 57 , 0  , 0  ), //      {ymm, ymm, ymm|m256|mem, ymm}
  ROW(2, 1, 1, 0, 55 , 148, 0  , 0  , 0  , 0  ), // #453 {xmm, xmm|m128|ymm|m256}
  ROW(2, 1, 1, 0, 57 , 62 , 0  , 0  , 0  , 0  ), //      {ymm, zmm|m512|mem}
  ROW(2, 1, 1, 0, 6  , 124, 0  , 0  , 0  , 0  ), // #455 {r32, xmm|m16|mem}
  ROW(2, 0, 1, 0, 8  , 124, 0  , 0  , 0  , 0  ), //      {r64, xmm|m16|mem}
  ROW(3, 1, 1, 0, 55 , 55 , 25 , 0  , 0  , 0  ), // #457 {xmm, xmm, r32|m32|mem}
  ROW(3, 0, 1, 0, 55 , 55 , 26 , 0  , 0  , 0  ), //      {xmm, xmm, r64|m64|mem}
  ROW(3, 1, 1, 0, 55 , 55 , 13 , 0  , 0  , 0  ), // #459 {xmm, xmm, r32|m32}
  ROW(3, 0, 1, 0, 55 , 55 , 15 , 0  , 0  , 0  ), //      {xmm, xmm, r64|m64}
  ROW(4, 1, 1, 0, 55 , 55 , 55 , 70 , 0  , 0  ), // #461 {xmm, xmm, xmm, xmm|m64|mem}
  ROW(4, 1, 1, 0, 55 , 55 , 31 , 55 , 0  , 0  ), //      {xmm, xmm, m64|mem, xmm}
  ROW(4, 1, 1, 0, 55 , 55 , 55 , 122, 0  , 0  ), // #463 {xmm, xmm, xmm, xmm|m32|mem}
  ROW(4, 1, 1, 0, 55 , 55 , 30 , 55 , 0  , 0  ), //      {xmm, xmm, m32|mem, xmm}
  ROW(4, 1, 1, 0, 57 , 57 , 56 , 10 , 0  , 0  ), // #465 {ymm, ymm, xmm|m128|mem, i8|u8}
  ROW(4, 1, 1, 0, 61 , 61 , 56 , 10 , 0  , 0  ), //      {zmm, zmm, xmm|m128|mem, i8|u8}
  ROW(1, 1, 0, 1, 45 , 0  , 0  , 0  , 0  , 0  ), // #467 {<eax>}
  ROW(1, 0, 1, 1, 47 , 0  , 0  , 0  , 0  , 0  ), // #468 {<rax>}
  ROW(2, 1, 1, 0, 25 , 55 , 0  , 0  , 0  , 0  ), // #469 {r32|m32|mem, xmm}
  ROW(2, 1, 1, 0, 55 , 25 , 0  , 0  , 0  , 0  ), //      {xmm, r32|m32|mem}
  ROW(2, 1, 1, 0, 117, 55 , 0  , 0  , 0  , 0  ), // #471 {r32|m16|mem, xmm}
  ROW(2, 1, 1, 0, 55 , 117, 0  , 0  , 0  , 0  ), //      {xmm, r32|m16|mem}
  ROW(2, 1, 0, 0, 25 , 6  , 0  , 0  , 0  , 0  ), // #473 {r32|m32|mem, r32}
  ROW(2, 0, 1, 0, 26 , 8  , 0  , 0  , 0  , 0  ), //      {r64|m64|mem, r64}
  ROW(2, 1, 0, 0, 6  , 25 , 0  , 0  , 0  , 0  ), // #475 {r32, r32|m32|mem}
  ROW(2, 0, 1, 0, 8  , 26 , 0  , 0  , 0  , 0  ), //      {r64, r64|m64|mem}
  ROW(2, 1, 1, 0, 149, 70 , 0  , 0  , 0  , 0  ), // #477 {xmm|ymm|zmm, xmm|m64|mem}
  ROW(2, 0, 1, 0, 149, 8  , 0  , 0  , 0  , 0  ), //      {xmm|ymm|zmm, r64}
  ROW(3, 1, 1, 0, 55 , 55 , 64 , 0  , 0  , 0  ), // #479 {xmm, xmm, xmm|m128|mem|i8|u8}
  ROW(3, 1, 1, 0, 55 , 59 , 150, 0  , 0  , 0  ), //      {xmm, m128|mem, i8|u8|xmm}
  ROW(2, 1, 1, 0, 77 , 102, 0  , 0  , 0  , 0  ), // #481 {vm32x, xmm|ymm}
  ROW(2, 1, 1, 0, 78 , 61 , 0  , 0  , 0  , 0  ), //      {vm32y, zmm}
  ROW(2, 1, 1, 0, 123, 55 , 0  , 0  , 0  , 0  ), // #483 {vm64x|vm64y, xmm}
  ROW(2, 1, 1, 0, 82 , 57 , 0  , 0  , 0  , 0  ), //      {vm64z, ymm}
  ROW(3, 1, 1, 0, 55 , 55 , 56 , 0  , 0  , 0  ), // #485 {xmm, xmm, xmm|m128|mem}
  ROW(3, 1, 1, 0, 55 , 59 , 55 , 0  , 0  , 0  ), //      {xmm, m128|mem, xmm}
  ROW(1, 1, 0, 1, 42 , 0  , 0  , 0  , 0  , 0  ), // #487 {<ax>}
  ROW(2, 1, 0, 1, 42 , 10 , 0  , 0  , 0  , 0  ), // #488 {<ax>, i8|u8}
  ROW(2, 1, 0, 0, 24 , 4  , 0  , 0  , 0  , 0  ), // #489 {r16|m16|mem, r16}
  ROW(3, 1, 1, 1, 55 , 56 , 151, 0  , 0  , 0  ), // #490 {xmm, xmm|m128|mem, <xmm0>}
  ROW(2, 1, 1, 0, 125, 152, 0  , 0  , 0  , 0  ), // #491 {bnd, mib}
  ROW(2, 1, 1, 0, 125, 127, 0  , 0  , 0  , 0  ), // #492 {bnd, mem}
  ROW(2, 1, 1, 0, 152, 125, 0  , 0  , 0  , 0  ), // #493 {mib, bnd}
  ROW(1, 1, 1, 1, 42 , 0  , 0  , 0  , 0  , 0  ), // #494 {<ax>}
  ROW(2, 1, 1, 2, 44 , 45 , 0  , 0  , 0  , 0  ), // #495 {<edx>, <eax>}
  ROW(1, 1, 1, 0, 127, 0  , 0  , 0  , 0  , 0  ), // #496 {mem}
  ROW(1, 1, 1, 0, 31 , 0  , 0  , 0  , 0  , 0  ), // #497 {m64|mem}
  ROW(0, 0, 1, 0, 0  , 0  , 0  , 0  , 0  , 0  ), // #498 {}
  ROW(1, 1, 1, 1, 153, 0  , 0  , 0  , 0  , 0  ), // #499 {<ds:[mem|m512|memBase|zax]>}
  ROW(3, 1, 1, 0, 55 , 70 , 10 , 0  , 0  , 0  ), // #500 {xmm, xmm|m64|mem, i8|u8}
  ROW(3, 1, 1, 0, 55 , 122, 10 , 0  , 0  , 0  ), // #501 {xmm, xmm|m32|mem, i8|u8}
  ROW(5, 0, 1, 4, 59 , 46 , 47 , 154, 155, 0  ), // #502 {m128|mem, <rdx>, <rax>, <rcx>, <rbx>}
  ROW(5, 1, 1, 4, 31 , 44 , 45 , 131, 156, 0  ), // #503 {m64|mem, <edx>, <eax>, <ecx>, <ebx>}
  ROW(4, 1, 1, 4, 45 , 156, 131, 44 , 0  , 0  ), // #504 {<eax>, <ebx>, <ecx>, <edx>}
  ROW(2, 0, 1, 2, 46 , 47 , 0  , 0  , 0  , 0  ), // #505 {<rdx>, <rax>}
  ROW(2, 1, 1, 0, 67 , 56 , 0  , 0  , 0  , 0  ), // #506 {mm, xmm|m128|mem}
  ROW(2, 1, 1, 0, 55 , 68 , 0  , 0  , 0  , 0  ), // #507 {xmm, mm|m64|mem}
  ROW(2, 1, 1, 0, 67 , 70 , 0  , 0  , 0  , 0  ), // #508 {mm, xmm|m64|mem}
  ROW(2, 1, 1, 2, 43 , 42 , 0  , 0  , 0  , 0  ), // #509 {<dx>, <ax>}
  ROW(1, 1, 1, 1, 45 , 0  , 0  , 0  , 0  , 0  ), // #510 {<eax>}
  ROW(2, 1, 1, 0, 12 , 10 , 0  , 0  , 0  , 0  ), // #511 {i16|u16, i8|u8}
  ROW(3, 1, 1, 0, 25 , 55 , 10 , 0  , 0  , 0  ), // #512 {r32|m32|mem, xmm, i8|u8}
  ROW(1, 1, 1, 0, 115, 0  , 0  , 0  , 0  , 0  ), // #513 {m80|mem}
  ROW(1, 1, 1, 0, 39 , 0  , 0  , 0  , 0  , 0  ), // #514 {m16|m32}
  ROW(1, 1, 1, 0, 157, 0  , 0  , 0  , 0  , 0  ), // #515 {m16|m32|m64}
  ROW(1, 1, 1, 0, 158, 0  , 0  , 0  , 0  , 0  ), // #516 {m32|m64|m80|st}
  ROW(1, 1, 1, 0, 21 , 0  , 0  , 0  , 0  , 0  ), // #517 {m16|mem}
  ROW(1, 1, 1, 0, 159, 0  , 0  , 0  , 0  , 0  ), // #518 {ax|m16|mem}
  ROW(1, 0, 1, 0, 127, 0  , 0  , 0  , 0  , 0  ), // #519 {mem}
  ROW(2, 1, 1, 2, 45 , 156, 0  , 0  , 0  , 0  ), // #520 {<eax>, <ebx>}
  ROW(2, 1, 1, 1, 10 , 45 , 0  , 0  , 0  , 0  ), // #521 {i8|u8, <eax>}
  ROW(2, 1, 1, 0, 160, 161, 0  , 0  , 0  , 0  ), // #522 {al|ax|eax, i8|u8|dx}
  ROW(2, 1, 1, 0, 162, 163, 0  , 0  , 0  , 0  ), // #523 {es:[memBase|zdi|m8|m16|m32], dx}
  ROW(1, 1, 1, 0, 10 , 0  , 0  , 0  , 0  , 0  ), // #524 {i8|u8}
  ROW(0, 1, 0, 0, 0  , 0  , 0  , 0  , 0  , 0  ), // #525 {}
  ROW(3, 1, 1, 0, 92 , 92 , 92 , 0  , 0  , 0  ), // #526 {k, k, k}
  ROW(2, 1, 1, 0, 92 , 92 , 0  , 0  , 0  , 0  ), // #527 {k, k}
  ROW(3, 1, 1, 0, 92 , 92 , 10 , 0  , 0  , 0  ), // #528 {k, k, i8|u8}
  ROW(1, 1, 1, 1, 164, 0  , 0  , 0  , 0  , 0  ), // #529 {<ah>}
  ROW(1, 1, 1, 0, 30 , 0  , 0  , 0  , 0  , 0  ), // #530 {m32|mem}
  ROW(1, 0, 1, 0, 63 , 0  , 0  , 0  , 0  , 0  ), // #531 {m512|mem}
  ROW(1, 1, 1, 0, 24 , 0  , 0  , 0  , 0  , 0  ), // #532 {r16|m16|mem}
  ROW(3, 1, 1, 1, 55 , 55 , 165, 0  , 0  , 0  ), // #533 {xmm, xmm, <ds:[mem|m128|memBase|zdi]>}
  ROW(3, 1, 1, 1, 67 , 67 , 166, 0  , 0  , 0  ), // #534 {mm, mm, <ds:[mem|m64|memBase|zdi]>}
  ROW(3, 1, 1, 3, 167, 131, 44 , 0  , 0  , 0  ), // #535 {<ds:[mem|memBase|zax]>, <ecx>, <edx>}
  ROW(2, 1, 1, 0, 67 , 55 , 0  , 0  , 0  , 0  ), // #536 {mm, xmm}
  ROW(2, 1, 1, 0, 6  , 55 , 0  , 0  , 0  , 0  ), // #537 {r32, xmm}
  ROW(2, 1, 1, 0, 31 , 67 , 0  , 0  , 0  , 0  ), // #538 {m64|mem, mm}
  ROW(2, 1, 1, 0, 55 , 67 , 0  , 0  , 0  , 0  ), // #539 {xmm, mm}
  ROW(2, 1, 1, 2, 45 , 131, 0  , 0  , 0  , 0  ), // #540 {<eax>, <ecx>}
  ROW(3, 1, 1, 3, 45 , 131, 156, 0  , 0  , 0  ), // #541 {<eax>, <ecx>, <ebx>}
  ROW(2, 1, 1, 0, 161, 160, 0  , 0  , 0  , 0  ), // #542 {i8|u8|dx, al|ax|eax}
  ROW(2, 1, 1, 0, 163, 168, 0  , 0  , 0  , 0  ), // #543 {dx, ds:[memBase|zsi|m8|m16|m32]}
  ROW(6, 1, 1, 3, 55 , 56 , 10 , 131, 45 , 44 ), // #544 {xmm, xmm|m128|mem, i8|u8, <ecx>, <eax>, <edx>}
  ROW(6, 1, 1, 3, 55 , 56 , 10 , 151, 45 , 44 ), // #545 {xmm, xmm|m128|mem, i8|u8, <xmm0>, <eax>, <edx>}
  ROW(4, 1, 1, 1, 55 , 56 , 10 , 131, 0  , 0  ), // #546 {xmm, xmm|m128|mem, i8|u8, <ecx>}
  ROW(4, 1, 1, 1, 55 , 56 , 10 , 151, 0  , 0  ), // #547 {xmm, xmm|m128|mem, i8|u8, <xmm0>}
  ROW(3, 1, 1, 0, 137, 55 , 10 , 0  , 0  , 0  ), // #548 {r32|m8|mem, xmm, i8|u8}
  ROW(3, 0, 1, 0, 26 , 55 , 10 , 0  , 0  , 0  ), // #549 {r64|m64|mem, xmm, i8|u8}
  ROW(3, 1, 1, 0, 55 , 137, 10 , 0  , 0  , 0  ), // #550 {xmm, r32|m8|mem, i8|u8}
  ROW(3, 1, 1, 0, 55 , 25 , 10 , 0  , 0  , 0  ), // #551 {xmm, r32|m32|mem, i8|u8}
  ROW(3, 0, 1, 0, 55 , 26 , 10 , 0  , 0  , 0  ), // #552 {xmm, r64|m64|mem, i8|u8}
  ROW(3, 1, 1, 0, 69 , 117, 10 , 0  , 0  , 0  ), // #553 {mm|xmm, r32|m16|mem, i8|u8}
  ROW(2, 1, 1, 0, 6  , 69 , 0  , 0  , 0  , 0  ), // #554 {r32, mm|xmm}
  ROW(2, 1, 1, 0, 55 , 10 , 0  , 0  , 0  , 0  ), // #555 {xmm, i8|u8}
  ROW(1, 1, 1, 0, 12 , 0  , 0  , 0  , 0  , 0  ), // #556 {i16|u16}
  ROW(1, 0, 1, 0, 141, 0  , 0  , 0  , 0  , 0  ), // #557 {r32|r64}
  ROW(1, 1, 1, 0, 1  , 0  , 0  , 0  , 0  , 0  ), // #558 {r8lo|r8hi|m8|mem}
  ROW(3, 0, 1, 0, 169, 169, 169, 0  , 0  , 0  ), // #559 {tmm, tmm, tmm}
  ROW(2, 0, 1, 0, 169, 170, 0  , 0  , 0  , 0  ), // #560 {tmm, tmem}
  ROW(2, 0, 1, 0, 170, 169, 0  , 0  , 0  , 0  ), // #561 {tmem, tmm}
  ROW(1, 0, 1, 0, 169, 0  , 0  , 0  , 0  , 0  ), // #562 {tmm}
  ROW(3, 1, 1, 2, 6  , 44 , 45 , 0  , 0  , 0  ), // #563 {r32, <edx>, <eax>}
  ROW(3, 1, 1, 0, 55 , 55 , 70 , 0  , 0  , 0  ), // #564 {xmm, xmm, xmm|m64|mem}
  ROW(3, 1, 1, 0, 55 , 55 , 124, 0  , 0  , 0  ), // #565 {xmm, xmm, xmm|m16|mem}
  ROW(3, 1, 1, 0, 55 , 55 , 122, 0  , 0  , 0  ), // #566 {xmm, xmm, xmm|m32|mem}
  ROW(2, 1, 1, 0, 102, 21 , 0  , 0  , 0  , 0  ), // #567 {xmm|ymm, m16|mem}
  ROW(2, 1, 1, 0, 57 , 59 , 0  , 0  , 0  , 0  ), // #568 {ymm, m128|mem}
  ROW(2, 1, 1, 0, 171, 70 , 0  , 0  , 0  , 0  ), // #569 {ymm|zmm, xmm|m64|mem}
  ROW(2, 1, 1, 0, 171, 59 , 0  , 0  , 0  , 0  ), // #570 {ymm|zmm, m128|mem}
  ROW(2, 1, 1, 0, 61 , 60 , 0  , 0  , 0  , 0  ), // #571 {zmm, m256|mem}
  ROW(2, 1, 1, 0, 149, 122, 0  , 0  , 0  , 0  ), // #572 {xmm|ymm|zmm, m32|mem|xmm}
  ROW(4, 1, 1, 0, 120, 55 , 70 , 10 , 0  , 0  ), // #573 {xmm|k, xmm, xmm|m64|mem, i8|u8}
  ROW(4, 1, 1, 0, 92 , 55 , 124, 10 , 0  , 0  ), // #574 {k, xmm, xmm|m16|mem, i8|u8}
  ROW(4, 1, 1, 0, 120, 55 , 122, 10 , 0  , 0  ), // #575 {xmm|k, xmm, xmm|m32|mem, i8|u8}
  ROW(2, 1, 1, 0, 55 , 172, 0  , 0  , 0  , 0  ), // #576 {xmm, xmm|m128|ymm|m256|zmm|m512}
  ROW(3, 1, 1, 0, 56 , 171, 10 , 0  , 0  , 0  ), // #577 {xmm|m128|mem, ymm|zmm, i8|u8}
  ROW(4, 1, 1, 0, 55 , 55 , 70 , 10 , 0  , 0  ), // #578 {xmm, xmm, xmm|m64|mem, i8|u8}
  ROW(4, 1, 1, 0, 55 , 55 , 122, 10 , 0  , 0  ), // #579 {xmm, xmm, xmm|m32|mem, i8|u8}
  ROW(3, 1, 1, 0, 92 , 172, 10 , 0  , 0  , 0  ), // #580 {k, xmm|m128|ymm|m256|zmm|m512, i8|u8}
  ROW(3, 1, 1, 0, 92 , 70 , 10 , 0  , 0  , 0  ), // #581 {k, xmm|m64|mem, i8|u8}
  ROW(3, 1, 1, 0, 92 , 124, 10 , 0  , 0  , 0  ), // #582 {k, xmm|m16|mem, i8|u8}
  ROW(3, 1, 1, 0, 92 , 122, 10 , 0  , 0  , 0  ), // #583 {k, xmm|m32|mem, i8|u8}
  ROW(4, 1, 1, 0, 55 , 55 , 124, 10 , 0  , 0  ), // #584 {xmm, xmm, xmm|m16|mem, i8|u8}
  ROW(4, 1, 1, 0, 61 , 61 , 58 , 10 , 0  , 0  ), // #585 {zmm, zmm, ymm|m256|mem, i8|u8}
  ROW(2, 1, 1, 0, 6  , 102, 0  , 0  , 0  , 0  ), // #586 {r32, xmm|ymm}
  ROW(2, 1, 1, 0, 149, 173, 0  , 0  , 0  , 0  ), // #587 {xmm|ymm|zmm, xmm|m8|mem|r32}
  ROW(2, 1, 1, 0, 149, 174, 0  , 0  , 0  , 0  ), // #588 {xmm|ymm|zmm, xmm|m32|mem|r32}
  ROW(2, 1, 1, 0, 149, 92 , 0  , 0  , 0  , 0  ), // #589 {xmm|ymm|zmm, k}
  ROW(2, 1, 1, 0, 149, 175, 0  , 0  , 0  , 0  ), // #590 {xmm|ymm|zmm, xmm|m16|mem|r32}
  ROW(3, 1, 1, 0, 117, 55 , 10 , 0  , 0  , 0  ), // #591 {r32|m16|mem, xmm, i8|u8}
  ROW(4, 1, 1, 0, 55 , 55 , 137, 10 , 0  , 0  ), // #592 {xmm, xmm, r32|m8|mem, i8|u8}
  ROW(4, 1, 1, 0, 55 , 55 , 25 , 10 , 0  , 0  ), // #593 {xmm, xmm, r32|m32|mem, i8|u8}
  ROW(4, 0, 1, 0, 55 , 55 , 26 , 10 , 0  , 0  ), // #594 {xmm, xmm, r64|m64|mem, i8|u8}
  ROW(4, 1, 1, 0, 55 , 55 , 117, 10 , 0  , 0  ), // #595 {xmm, xmm, r32|m16|mem, i8|u8}
  ROW(2, 1, 1, 0, 92 , 149, 0  , 0  , 0  , 0  ), // #596 {k, xmm|ymm|zmm}
  ROW(2, 1, 1, 0, 57 , 55 , 0  , 0  , 0  , 0  ), // #597 {ymm, xmm}
  ROW(2, 1, 1, 0, 57 , 57 , 0  , 0  , 0  , 0  ), // #598 {ymm, ymm}
  ROW(3, 1, 1, 0, 57 , 57 , 55 , 0  , 0  , 0  ), // #599 {ymm, ymm, xmm}
  ROW(3, 1, 1, 2, 127, 44 , 45 , 0  , 0  , 0  ), // #600 {mem, <edx>, <eax>}
  ROW(3, 0, 1, 2, 127, 44 , 45 , 0  , 0  , 0  )  // #601 {mem, <edx>, <eax>}
};
#undef ROW

#define ROW(op_flags, reg_id) { op_flags, uint8_t(reg_id) }
#define F(VAL) uint64_t(InstDB::OpFlags::k##VAL)
const InstDB::OpSignature InstDB::_op_signature_table[] = {
  ROW(0, 0xFF),
  ROW(F(RegGpbLo) | F(RegGpbHi) | F(MemUnspecified) | F(Mem8), 0x00),
  ROW(F(RegGpbLo) | F(RegGpbHi), 0x00),
  ROW(F(RegGpw) | F(RegSReg) | F(MemUnspecified) | F(Mem16), 0x00),
  ROW(F(RegGpw), 0x00),
  ROW(F(RegGpd) | F(RegSReg) | F(MemUnspecified) | F(Mem32), 0x00),
  ROW(F(RegGpd), 0x00),
  ROW(F(RegGpq) | F(RegSReg) | F(RegCReg) | F(RegDReg) | F(MemUnspecified) | F(Mem64), 0x00),
  ROW(F(RegGpq), 0x00),
  ROW(F(RegGpbLo) | F(RegGpbHi) | F(Mem8), 0x00),
  ROW(F(ImmI8) | F(ImmU8), 0x00),
  ROW(F(RegGpw) | F(Mem16), 0x00),
  ROW(F(ImmI16) | F(ImmU16), 0x00),
  ROW(F(RegGpd) | F(Mem32), 0x00),
  ROW(F(ImmI32) | F(ImmU32), 0x00),
  ROW(F(RegGpq) | F(Mem64), 0x00),
  ROW(F(ImmI32), 0x00),
  ROW(F(MemUnspecified) | F(Mem8), 0x00),
  ROW(F(RegSReg) | F(MemUnspecified) | F(Mem16), 0x00),
  ROW(F(RegSReg) | F(MemUnspecified) | F(Mem32), 0x00),
  ROW(F(RegSReg) | F(RegCReg) | F(RegDReg) | F(MemUnspecified) | F(Mem64) | F(ImmI64) | F(ImmU64), 0x00),
  ROW(F(MemUnspecified) | F(Mem16), 0x00),
  ROW(F(RegSReg), 0x00),
  ROW(F(RegCReg) | F(RegDReg), 0x00),
  ROW(F(RegGpw) | F(MemUnspecified) | F(Mem16), 0x00),
  ROW(F(RegGpd) | F(MemUnspecified) | F(Mem32), 0x00),
  ROW(F(RegGpq) | F(MemUnspecified) | F(Mem64), 0x00),
  ROW(F(RegGpw) | F(RegGpd) | F(Mem16) | F(Mem32), 0x00),
  ROW(F(ImmI8), 0x00),
  ROW(F(ImmI8) | F(ImmI32), 0x00),
  ROW(F(MemUnspecified) | F(Mem32), 0x00),
  ROW(F(MemUnspecified) | F(Mem64), 0x00),
  ROW(F(RegGpq) | F(MemUnspecified) | F(Mem64) | F(ImmI8) | F(ImmI32) | F(ImmU32), 0x00),
  ROW(F(Mem64), 0x00),
  ROW(F(MemUnspecified) | F(Mem8) | F(ImmI8) | F(ImmU8), 0x00),
  ROW(F(MemUnspecified) | F(Mem16) | F(ImmI8) | F(ImmI16) | F(ImmU16), 0x00),
  ROW(F(MemUnspecified) | F(Mem32) | F(ImmI8) | F(ImmI32) | F(ImmU32), 0x00),
  ROW(F(MemUnspecified) | F(Mem64) | F(ImmI8) | F(ImmI32), 0x00),
  ROW(F(Mem8), 0x00),
  ROW(F(Mem16) | F(Mem32), 0x00),
  ROW(F(Mem16), 0x00),
  ROW(F(Mem32), 0x00),
  ROW(F(RegGpw) | F(FlagImplicit), 0x01),
  ROW(F(RegGpw) | F(FlagImplicit), 0x04),
  ROW(F(RegGpd) | F(FlagImplicit), 0x04),
  ROW(F(RegGpd) | F(FlagImplicit), 0x01),
  ROW(F(RegGpq) | F(FlagImplicit), 0x04),
  ROW(F(RegGpq) | F(FlagImplicit), 0x01),
  ROW(F(ImmI8) | F(ImmI16) | F(ImmU16), 0x00),
  ROW(F(ImmI8) | F(ImmI32) | F(ImmU32), 0x00),
  ROW(F(ImmI64) | F(ImmU64), 0x00),
  ROW(F(RegGpbLo), 0x01),
  ROW(F(RegGpw), 0x01),
  ROW(F(RegGpd), 0x01),
  ROW(F(RegGpq), 0x01),
  ROW(F(RegXmm), 0x00),
  ROW(F(RegXmm) | F(MemUnspecified) | F(Mem128), 0x00),
  ROW(F(RegYmm), 0x00),
  ROW(F(RegYmm) | F(MemUnspecified) | F(Mem256), 0x00),
  ROW(F(MemUnspecified) | F(Mem128), 0x00),
  ROW(F(MemUnspecified) | F(Mem256), 0x00),
  ROW(F(RegZmm), 0x00),
  ROW(F(RegZmm) | F(MemUnspecified) | F(Mem512), 0x00),
  ROW(F(MemUnspecified) | F(Mem512), 0x00),
  ROW(F(RegXmm) | F(MemUnspecified) | F(Mem128) | F(ImmI8) | F(ImmU8), 0x00),
  ROW(F(RegYmm) | F(MemUnspecified) | F(Mem256) | F(ImmI8) | F(ImmU8), 0x00),
  ROW(F(RegZmm) | F(MemUnspecified) | F(Mem512) | F(ImmI8) | F(ImmU8), 0x00),
  ROW(F(RegMm), 0x00),
  ROW(F(RegMm) | F(MemUnspecified) | F(Mem64), 0x00),
  ROW(F(RegXmm) | F(RegMm), 0x00),
  ROW(F(RegXmm) | F(MemUnspecified) | F(Mem64), 0x00),
  ROW(F(RegSReg), 0x1A),
  ROW(F(RegSReg), 0x60),
  ROW(F(RegGpw) | F(Mem16) | F(ImmI8) | F(ImmU8) | F(ImmI16) | F(ImmU16), 0x00),
  ROW(F(RegGpd) | F(Mem32) | F(ImmI32) | F(ImmU32), 0x00),
  ROW(F(RegGpq) | F(Mem64) | F(ImmI32), 0x00),
  ROW(F(RegSReg), 0x1E),
  ROW(F(Vm32x), 0x00),
  ROW(F(Vm32y), 0x00),
  ROW(F(Vm32z), 0x00),
  ROW(F(Vm64x), 0x00),
  ROW(F(Vm64y), 0x00),
  ROW(F(Vm64z), 0x00),
  ROW(F(Mem8) | F(FlagMemBase) | F(FlagMemDs), 0x40),
  ROW(F(Mem8) | F(FlagMemBase) | F(FlagMemEs), 0x80),
  ROW(F(Mem16) | F(FlagMemBase) | F(FlagMemDs), 0x40),
  ROW(F(Mem16) | F(FlagMemBase) | F(FlagMemEs), 0x80),
  ROW(F(Mem32) | F(FlagMemBase) | F(FlagMemDs), 0x40),
  ROW(F(Mem32) | F(FlagMemBase) | F(FlagMemEs), 0x80),
  ROW(F(Mem64) | F(FlagMemBase) | F(FlagMemDs), 0x40),
  ROW(F(Mem64) | F(FlagMemBase) | F(FlagMemEs), 0x80),
  ROW(F(RegGpbLo) | F(FlagImplicit), 0x01),
  ROW(F(RegKReg), 0x00),
  ROW(F(RegKReg) | F(MemUnspecified) | F(Mem64), 0x00),
  ROW(F(MemUnspecified) | F(Mem8) | F(FlagMemBase) | F(FlagMemDs), 0x40),
  ROW(F(MemUnspecified) | F(Mem16) | F(FlagMemBase) | F(FlagMemDs), 0x40),
  ROW(F(MemUnspecified) | F(Mem32) | F(FlagMemBase) | F(FlagMemDs), 0x40),
  ROW(F(MemUnspecified) | F(Mem64) | F(FlagMemBase) | F(FlagMemDs), 0x40),
  ROW(F(MemUnspecified) | F(Mem8) | F(FlagMemBase) | F(FlagMemEs), 0x80),
  ROW(F(MemUnspecified) | F(Mem16) | F(FlagMemBase) | F(FlagMemEs), 0x80),
  ROW(F(MemUnspecified) | F(Mem32) | F(FlagMemBase) | F(FlagMemEs), 0x80),
  ROW(F(MemUnspecified) | F(Mem64) | F(FlagMemBase) | F(FlagMemEs), 0x80),
  ROW(F(RegXmm) | F(RegYmm), 0x00),
  ROW(F(ImmI4) | F(ImmU4), 0x00),
  ROW(F(RegGpw) | F(RegGpd) | F(MemUnspecified) | F(Mem16) | F(Mem32) | F(ImmI32) | F(ImmI64) | F(Rel32), 0x00),
  ROW(F(ImmI32) | F(ImmI64) | F(Rel32), 0x00),
  ROW(F(RegGpw) | F(RegGpd), 0x00),
  ROW(F(RegGpbLo) | F(RegGpbHi) | F(RegGpw) | F(RegGpd) | F(Mem8) | F(Mem16) | F(Mem32), 0x00),
  ROW(F(Mem32) | F(Mem64), 0x00),
  ROW(F(RegSt), 0x01),
  ROW(F(RegSt), 0x00),
  ROW(F(ImmI32) | F(ImmI64) | F(Rel8) | F(Rel32), 0x00),
  ROW(F(RegGpd) | F(Mem32) | F(ImmI32) | F(ImmI64) | F(Rel32), 0x00),
  ROW(F(ImmI16) | F(ImmU16) | F(ImmI32) | F(ImmU32), 0x00),
  ROW(F(MemUnspecified) | F(Mem32) | F(Mem48), 0x00),
  ROW(F(MemUnspecified) | F(Mem80), 0x00),
  ROW(F(MemUnspecified) | F(Mem48), 0x00),
  ROW(F(RegGpd) | F(MemUnspecified) | F(Mem16), 0x00),
  ROW(F(RegGpbLo) | F(RegGpbHi) | F(RegGpw) | F(Mem8) | F(Mem16), 0x00),
  ROW(F(RegGpbLo) | F(ImmI8) | F(ImmU8), 0x02),
  ROW(F(RegXmm) | F(RegKReg), 0x00),
  ROW(F(RegYmm) | F(RegKReg), 0x00),
  ROW(F(RegXmm) | F(MemUnspecified) | F(Mem32), 0x00),
  ROW(F(Vm64x) | F(Vm64y), 0x00),
  ROW(F(RegXmm) | F(MemUnspecified) | F(Mem16), 0x00),
  ROW(F(RegBnd), 0x00),
  ROW(F(RegBnd) | F(MemUnspecified), 0x00),
  ROW(F(MemUnspecified), 0x00),
  ROW(F(RegGpbLo) | F(RegGpbHi) | F(RegGpq) | F(Mem8) | F(Mem64), 0x00),
  ROW(F(MemUnspecified) | F(Mem512) | F(FlagMemBase) | F(FlagMemEs), 0x00),
  ROW(F(RegSt) | F(Mem32) | F(Mem64), 0x00),
  ROW(F(RegGpd) | F(FlagImplicit), 0x02),
  ROW(F(RegGpd) | F(RegGpq) | F(FlagImplicit), 0x01),
  ROW(F(RegGpw) | F(RegGpd) | F(FlagImplicit), 0x02),
  ROW(F(ImmI32) | F(ImmI64) | F(Rel8), 0x00),
  ROW(F(RegGpd) | F(RegGpq) | F(FlagImplicit), 0x02),
  ROW(F(RegGpd) | F(RegKReg) | F(MemUnspecified) | F(Mem8), 0x00),
  ROW(F(RegGpd) | F(MemUnspecified) | F(Mem8), 0x00),
  ROW(F(RegGpd) | F(RegKReg) | F(MemUnspecified) | F(Mem32), 0x00),
  ROW(F(RegGpd) | F(RegKReg) | F(MemUnspecified) | F(Mem16), 0x00),
  ROW(F(MemUnspecified) | F(Mem8) | F(Mem16) | F(Mem32) | F(Mem48) | F(Mem64) | F(Mem80) | F(Mem128) | F(Mem256) | F(Mem512) | F(Mem1024), 0x00),
  ROW(F(RegGpd) | F(RegGpq), 0x00),
  ROW(F(RegMm) | F(MemUnspecified) | F(Mem64) | F(ImmI8) | F(ImmU8), 0x00),
  ROW(F(RegMm) | F(MemUnspecified) | F(Mem32), 0x00),
  ROW(F(ImmU16), 0x00),
  ROW(F(RegGpw) | F(RegGpd) | F(MemUnspecified) | F(Mem16), 0x00),
  ROW(F(RegGpq) | F(MemUnspecified) | F(Mem16), 0x00),
  ROW(F(MemUnspecified) | F(FlagMemBase) | F(FlagMemDs), 0x00),
  ROW(F(RegXmm) | F(RegYmm) | F(Mem128) | F(Mem256), 0x00),
  ROW(F(RegXmm) | F(RegYmm) | F(RegZmm), 0x00),
  ROW(F(RegXmm) | F(ImmI8) | F(ImmU8), 0x00),
  ROW(F(RegXmm) | F(FlagImplicit), 0x01),
  ROW(F(MemUnspecified) | F(FlagMib), 0x00),
  ROW(F(MemUnspecified) | F(Mem512) | F(FlagMemBase) | F(FlagMemDs) | F(FlagImplicit), 0x01),
  ROW(F(RegGpq) | F(FlagImplicit), 0x02),
  ROW(F(RegGpq) | F(FlagImplicit), 0x08),
  ROW(F(RegGpd) | F(FlagImplicit), 0x08),
  ROW(F(Mem16) | F(Mem32) | F(Mem64), 0x00),
  ROW(F(RegSt) | F(Mem32) | F(Mem64) | F(Mem80), 0x00),
  ROW(F(RegGpw) | F(MemUnspecified) | F(Mem16), 0x01),
  ROW(F(RegGpbLo) | F(RegGpw) | F(RegGpd), 0x01),
  ROW(F(RegGpw) | F(ImmI8) | F(ImmU8), 0x04),
  ROW(F(Mem8) | F(Mem16) | F(Mem32) | F(FlagMemBase) | F(FlagMemEs), 0x80),
  ROW(F(RegGpw), 0x04),
  ROW(F(RegGpbHi) | F(FlagImplicit), 0x01),
  ROW(F(MemUnspecified) | F(Mem128) | F(FlagMemBase) | F(FlagMemDs) | F(FlagImplicit), 0x80),
  ROW(F(MemUnspecified) | F(Mem64) | F(FlagMemBase) | F(FlagMemDs) | F(FlagImplicit), 0x80),
  ROW(F(MemUnspecified) | F(FlagMemBase) | F(FlagMemDs) | F(FlagImplicit), 0x01),
  ROW(F(Mem8) | F(Mem16) | F(Mem32) | F(FlagMemBase) | F(FlagMemDs), 0x40),
  ROW(F(RegTmm), 0x00),
  ROW(F(MemUnspecified) | F(FlagTMem), 0x00),
  ROW(F(RegYmm) | F(RegZmm), 0x00),
  ROW(F(RegXmm) | F(RegYmm) | F(RegZmm) | F(Mem128) | F(Mem256) | F(Mem512), 0x00),
  ROW(F(RegGpd) | F(RegXmm) | F(MemUnspecified) | F(Mem8), 0x00),
  ROW(F(RegGpd) | F(RegXmm) | F(MemUnspecified) | F(Mem32), 0x00),
  ROW(F(RegGpd) | F(RegXmm) | F(MemUnspecified) | F(Mem16), 0x00)
};
#undef F
#undef ROW
// ----------------------------------------------------------------------------
// ${InstSignatureTable:End}
#endif // !ASMJIT_NO_INTROSPECTION

// x86::InstInternal - QueryRWInfo
// ===============================

// ${InstRWInfoTable:Begin}
// ------------------- Automatically generated, do not edit -------------------
const uint8_t InstDB::rw_info_index_a_table[Inst::_kIdCount] = {
  0, 0, 1, 2, 1, 2, 0, 3, 4, 3, 5, 5, 6, 7, 5, 5, 4, 5, 5, 5, 5, 8, 0, 3, 0, 5,
  5, 5, 5, 2, 9, 2, 0, 10, 10, 10, 10, 10, 0, 0, 0, 0, 10, 10, 10, 10, 10, 11, 11,
  11, 12, 12, 13, 14, 15, 10, 10, 0, 16, 17, 17, 17, 0, 0, 0, 18, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
  4, 19, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20, 0, 0, 0, 0, 0, 0,
  0, 21, 22, 0, 23, 24, 25, 8, 26, 26, 26, 25, 27, 8, 25, 28, 29, 30, 31, 32, 33,
  34, 26, 26, 8, 28, 29, 34, 35, 0, 0, 0, 0, 36, 5, 5, 6, 7, 0, 0, 0, 0, 0, 37,
  37, 0, 0, 38, 0, 0, 39, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 39, 0, 39, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 39, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 39, 0, 39, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 40, 0, 0, 5, 5, 5, 0, 41, 5, 5, 36, 42, 43, 0, 0, 0, 44, 0, 38, 0, 0, 0, 0,
  45, 0, 46, 0, 45, 45, 0, 0, 0, 0, 0, 47, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 48, 49, 50, 51, 52, 53, 54, 55,
  0, 0, 0, 56, 57, 58, 59, 0, 0, 0, 0, 0, 0, 0, 0, 0, 56, 57, 58, 59, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 60, 0, 61, 0, 2, 0, 62, 0, 2, 0, 2, 0, 2, 0, 0, 0, 0,
  0, 63, 64, 64, 64, 60, 2, 0, 0, 0, 10, 0, 0, 5, 5, 6, 7, 0, 0, 5, 5, 6, 7, 0,
  0, 65, 66, 67, 67, 68, 49, 25, 37, 68, 54, 67, 67, 69, 70, 70, 71, 72, 72, 73,
  73, 61, 61, 68, 61, 61, 72, 72, 74, 50, 54, 75, 76, 8, 8, 77, 78, 10, 67, 67,
  78, 0, 36, 5, 5, 6, 7, 0, 79, 0, 0, 80, 0, 3, 5, 5, 81, 82, 10, 10, 10, 4,
  4, 5, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 4, 4, 0, 4, 83, 4, 0, 0, 0, 4, 4, 5, 4, 0,
  0, 4, 4, 5, 4, 0, 0, 0, 0, 0, 0, 0, 0, 84, 28, 28, 83, 83, 83, 83, 83, 83, 83,
  83, 83, 83, 28, 83, 83, 83, 28, 28, 83, 83, 83, 4, 4, 4, 85, 4, 4, 4, 28, 28,
  0, 0, 0, 0, 4, 4, 5, 5, 4, 4, 5, 5, 5, 5, 4, 4, 5, 5, 86, 87, 88, 25, 25, 25,
  87, 87, 88, 25, 25, 25, 87, 5, 4, 83, 4, 4, 5, 4, 4, 0, 0, 0, 10, 0, 0, 0, 4,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 0, 0, 0, 0, 4, 4, 4, 4, 89, 4, 4, 0, 4, 4,
  4, 89, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 28, 90, 0, 4, 4, 5, 4, 91, 91, 5, 91, 0,
  0, 0, 0, 0, 0, 0, 0, 4, 92, 8, 93, 92, 0, 0, 94, 0, 0, 0, 0, 0, 0, 0, 0, 95, 0,
  0, 0, 0, 0, 92, 92, 0, 0, 0, 0, 0, 0, 8, 93, 0, 0, 92, 0, 0, 3, 96, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 5, 5, 0, 5,
  5, 0, 92, 0, 0, 92, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 8, 27, 93, 0, 0, 0, 0, 0, 0,
  97, 0, 0, 0, 3, 5, 5, 6, 7, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 16, 0, 98, 98, 0, 99, 0, 0, 0, 10, 10, 21, 22, 100, 100, 0, 0, 0, 0, 5, 5,
  5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 101, 101, 0,
  0, 0, 0, 0, 0, 102, 29, 103, 104, 103, 104, 102, 29, 103, 104, 103, 104, 105,
  106, 0, 0, 0, 0, 0, 0, 21, 107, 22, 108, 108, 109, 110, 10, 0, 68, 68, 68, 68,
  110, 110, 111, 110, 10, 110, 10, 109, 112, 109, 109, 112, 109, 112, 10, 10,
  10, 109, 0, 110, 109, 10, 109, 10, 113, 110, 0, 29, 0, 29, 0, 114, 0, 114, 0,
  0, 0, 0, 0, 34, 34, 110, 10, 110, 10, 109, 112, 109, 112, 10, 10, 10, 109, 10,
  109, 29, 29, 114, 114, 34, 34, 109, 110, 10, 10, 111, 110, 0, 0, 0, 10, 10,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 10,
  28, 115, 2, 2, 2, 116, 10, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 68, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 117, 117, 49, 118, 117, 117, 117, 117, 117,
  117, 117, 117, 0, 119, 119, 0, 72, 72, 120, 121, 68, 68, 68, 68, 122, 72, 123,
  10, 10, 74, 117, 117, 51, 0, 0, 0, 108, 0, 0, 0, 0, 0, 0, 0, 0, 0, 124, 0, 0,
  0, 0, 0, 0, 10, 10, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 125, 34, 126, 126, 29, 114, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 108, 108, 108, 108, 0,
  0, 0, 0, 0, 0, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 10, 10,
  10, 0, 0, 0, 0, 2, 2, 116, 2, 8, 8, 8, 0, 8, 0, 8, 8, 8, 8, 8, 8, 0, 8, 8, 85,
  8, 0, 8, 0, 0, 8, 0, 0, 0, 0, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 127, 128,
  129, 126, 126, 126, 126, 86, 127, 130, 129, 128, 128, 129, 130, 129, 128, 129,
  112, 131, 109, 109, 109, 112, 128, 129, 130, 129, 128, 129, 127, 129, 112, 131,
  109, 109, 109, 112, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 10, 10, 10, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 68, 68, 132, 68, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 124,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 10,
  0, 0, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 10, 0,
  0, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 68, 68, 68, 132, 133, 134, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 10, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 124, 124, 21,
  107, 22, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 74, 72, 74, 72, 0, 135, 0, 136,
  0, 0, 0, 3, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const uint8_t InstDB::rw_info_index_b_table[Inst::_kIdCount] = {
  0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 3, 0,
  0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 5, 5, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 4, 8, 1, 0, 9, 0, 0, 0, 10, 10, 10, 0, 0, 11, 0,
  0, 10, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 13, 13,
  13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 5, 5, 13, 0, 14, 15, 13, 16, 17, 18, 13,
  0, 0, 19, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 20, 1, 1, 21, 22, 0, 0, 0, 0, 5, 5, 0, 0, 0, 0, 0, 0, 23, 24, 0, 0,
  25, 26, 27, 28, 0, 0, 26, 26, 26, 26, 26, 26, 26, 26, 29, 30, 30, 29, 0, 0, 0,
  25, 26, 25, 26, 0, 26, 25, 25, 25, 25, 25, 25, 25, 0, 0, 31, 31, 31, 25, 25,
  29, 0, 32, 10, 0, 0, 0, 0, 0, 0, 25, 26, 0, 0, 0, 33, 34, 33, 35, 0, 0, 0, 0,
  0, 10, 33, 0, 0, 0, 0, 36, 34, 33, 36, 35, 25, 26, 25, 26, 0, 30, 30, 30, 30,
  0, 0, 0, 26, 10, 10, 33, 33, 0, 0, 0, 0, 5, 5, 0, 0, 0, 0, 0, 0, 0, 22, 37, 0,
  21, 38, 38, 0, 39, 40, 0, 0, 0, 0, 0, 10, 0, 41, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 42, 43, 44, 45, 42, 43, 42, 43, 44, 45,
  44, 45, 0, 0, 0, 0, 0, 0, 0, 0, 42, 43, 44, 0, 0, 0, 0, 45, 46, 47, 48, 49,
  46, 47, 48, 49, 0, 0, 0, 0, 50, 51, 52, 42, 43, 44, 45, 42, 43, 44, 45, 53, 0,
  25, 0, 54, 0, 55, 0, 0, 0, 0, 0, 10, 0, 10, 25, 56, 57, 56, 0, 0, 0, 0, 0, 0,
  56, 58, 58, 0, 59, 60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 61, 61, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  62, 0, 0, 62, 0, 0, 0, 0, 0, 5, 63, 0, 0, 0, 0, 64, 0, 65, 21, 66, 21, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 67, 0, 0, 0, 0, 0, 0, 6,
  5, 5, 0, 0, 0, 0, 68, 69, 0, 0, 0, 0, 70, 71, 0, 3, 3, 72, 23, 73, 74, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 75, 39, 76, 77, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 78, 0, 0, 0, 0, 0, 0, 0, 10, 10,
  10, 10, 10, 10, 10, 10, 10, 0, 0, 2, 2, 2, 79, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 66, 0, 0, 0, 0, 0, 0, 0, 0, 66, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 80, 80, 81, 80, 81, 81, 81, 80, 80, 82, 83,
  0, 84, 0, 0, 0, 0, 0, 0, 85, 2, 2, 86, 87, 0, 0, 0, 11, 88, 0, 4, 0, 0, 0, 0,
  0, 0, 89, 0, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 0, 90,
  0, 33, 0, 0, 0, 5, 0, 0, 6, 0, 91, 4, 0, 91, 4, 5, 5, 33, 20, 92, 80, 92, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 93, 0, 92, 94, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 95, 95, 0, 95, 95, 95, 95, 95, 95, 0, 0, 0, 0, 0, 0, 96, 0, 97, 0, 0, 0, 0,
  0, 0, 0, 0, 10, 97, 0, 0, 0, 0, 3, 3, 3, 98, 99, 100, 3, 3, 3, 3, 3, 3, 0, 2,
  3, 3, 3, 3, 3, 3, 0, 0, 3, 3, 3, 3, 101, 101, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 3, 102, 3, 103, 104, 105, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 106, 0, 0, 0, 0, 0, 0, 0,
  98, 0, 107, 0, 99, 0, 108, 0, 109, 110, 111, 112, 113, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 109, 110, 111, 0,
  0, 3, 3, 3, 3, 98, 99, 100, 3, 114, 3, 56, 56, 0, 0, 115, 116, 117, 116, 117,
  115, 116, 117, 116, 117, 23, 118, 119, 118, 119, 120, 120, 121, 122, 120, 120,
  120, 123, 124, 125, 120, 120, 120, 123, 124, 125, 120, 120, 120, 123, 124,
  125, 118, 119, 126, 126, 127, 128, 120, 120, 120, 120, 120, 120, 120, 120, 120,
  126, 126, 120, 120, 120, 123, 129, 125, 120, 120, 120, 123, 129, 125, 120, 120,
  120, 123, 129, 125, 120, 120, 120, 120, 120, 120, 120, 120, 120, 126, 126,
  126, 126, 127, 128, 118, 130, 120, 120, 120, 123, 124, 125, 120, 120, 120, 123,
  124, 125, 120, 120, 120, 123, 124, 125, 126, 126, 127, 128, 120, 120, 120,
  123, 129, 125, 120, 120, 120, 123, 129, 125, 120, 120, 120, 131, 129, 132, 126,
  126, 127, 128, 133, 133, 133, 79, 134, 135, 0, 0, 0, 0, 136, 137, 137, 138,
  0, 0, 0, 139, 140, 141, 85, 85, 85, 139, 140, 141, 3, 3, 3, 3, 3, 3, 3, 142, 143,
  144, 143, 144, 142, 143, 144, 143, 144, 100, 0, 54, 59, 145, 145, 3, 3, 3,
  98, 99, 100, 0, 11, 0, 0, 3, 3, 3, 98, 99, 100, 0, 146, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 147, 148, 148, 149, 150, 150, 0, 0, 0, 0, 0, 0, 0, 151, 152,
  0, 0, 153, 0, 0, 0, 3, 11, 154, 0, 0, 155, 146, 3, 3, 3, 98, 99, 100, 0, 0, 11,
  3, 3, 156, 156, 0, 0, 0, 0, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 101, 3, 0, 0, 0, 0, 0, 0, 3, 126, 102, 102, 3,
  3, 3, 3, 68, 69, 3, 3, 3, 3, 70, 71, 102, 102, 102, 102, 102, 102, 114, 114, 0,
  0, 0, 0, 114, 114, 114, 114, 114, 114, 0, 0, 120, 120, 120, 120, 120, 120, 120,
  120, 120, 120, 120, 120, 120, 120, 120, 120, 157, 157, 3, 3, 120, 120, 3,
  3, 120, 120, 126, 126, 158, 158, 158, 3, 158, 120, 120, 120, 120, 120, 120, 3,
  0, 0, 0, 0, 72, 23, 73, 159, 137, 136, 138, 137, 0, 0, 0, 3, 0, 3, 0, 0, 0, 0,
  0, 0, 3, 0, 0, 0, 0, 3, 0, 3, 3, 0, 160, 100, 98, 99, 0, 0, 161, 161, 161, 161,
  161, 161, 161, 161, 161, 161, 161, 161, 120, 120, 3, 3, 145, 145, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 3, 3, 3, 162, 85, 85, 3, 3, 85, 85, 3,
  3, 163, 163, 163, 163, 3, 0, 0, 0, 0, 163, 163, 163, 163, 163, 163, 3, 3, 120,
  120, 120, 3, 163, 163, 3, 3, 120, 120, 120, 3, 3, 102, 85, 85, 85, 3, 3, 3,
  164, 165, 164, 3, 3, 3, 166, 164, 167, 3, 3, 3, 166, 164, 165, 164, 3, 3, 3, 166,
  3, 3, 3, 3, 3, 3, 3, 3, 168, 168, 0, 102, 102, 102, 102, 102, 102, 102, 102,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 139, 141, 0, 0, 139, 141, 0, 0, 140,
  141, 85, 85, 85, 139, 140, 141, 85, 85, 85, 139, 140, 141, 85, 85, 139, 141,
  0, 0, 139, 141, 0, 0, 140, 141, 3, 3, 3, 98, 99, 100, 0, 0, 0, 0, 0, 0, 169, 3,
  3, 3, 3, 3, 3, 170, 170, 170, 3, 3, 0, 0, 0, 139, 140, 141, 93, 3, 3, 3, 98,
  99, 100, 0, 0, 0, 0, 0, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 57, 57, 171, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 81, 0, 0, 0, 0, 0, 172, 172, 172, 172, 173, 173, 173, 173, 173,
  173, 173, 173, 171, 0, 0
};

const InstDB::RWInfo InstDB::rw_info_a_table[] = {
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 0 , 0 , 0 , 0 , 0 , 0  } }, // #0 [ref=999x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 1 , 0 , 0 , 0 , 0 , 0  } }, // #1 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 2 , 3 , 0 , 0 , 0 , 0  } }, // #2 [ref=15x]
  { InstDB::RWInfo::kCategoryGeneric   , 1 , { 2 , 3 , 0 , 0 , 0 , 0  } }, // #3 [ref=7x]
  { InstDB::RWInfo::kCategoryGeneric   , 2 , { 2 , 3 , 0 , 0 , 0 , 0  } }, // #4 [ref=82x]
  { InstDB::RWInfo::kCategoryGeneric   , 3 , { 4 , 5 , 0 , 0 , 0 , 0  } }, // #5 [ref=55x]
  { InstDB::RWInfo::kCategoryGeneric   , 4 , { 6 , 7 , 0 , 0 , 0 , 0  } }, // #6 [ref=6x]
  { InstDB::RWInfo::kCategoryGeneric   , 5 , { 8 , 9 , 0 , 0 , 0 , 0  } }, // #7 [ref=6x]
  { InstDB::RWInfo::kCategoryGeneric   , 3 , { 10, 5 , 0 , 0 , 0 , 0  } }, // #8 [ref=26x]
  { InstDB::RWInfo::kCategoryGeneric   , 7 , { 12, 13, 0 , 0 , 0 , 0  } }, // #9 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 2 , { 11, 3 , 0 , 0 , 0 , 0  } }, // #10 [ref=75x]
  { InstDB::RWInfo::kCategoryGeneric   , 2 , { 5 , 3 , 0 , 0 , 0 , 0  } }, // #11 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 8 , { 10, 3 , 0 , 0 , 0 , 0  } }, // #12 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 9 , { 10, 5 , 0 , 0 , 0 , 0  } }, // #13 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 8 , { 15, 5 , 0 , 0 , 0 , 0  } }, // #14 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 3 , 3 , 0 , 0 , 0 , 0  } }, // #15 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 10, { 3 , 3 , 0 , 0 , 0 , 0  } }, // #16 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 10, { 2 , 3 , 0 , 0 , 0 , 0  } }, // #17 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 16, 17, 0 , 0 , 0 , 0  } }, // #18 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 1 , { 3 , 3 , 0 , 0 , 0 , 0  } }, // #19 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 14, { 20, 21, 0 , 0 , 0 , 0  } }, // #20 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 4 , { 7 , 7 , 0 , 0 , 0 , 0  } }, // #21 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 5 , { 9 , 9 , 0 , 0 , 0 , 0  } }, // #22 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 33, 34, 0 , 0 , 0 , 0  } }, // #23 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 16, { 2 , 3 , 0 , 0 , 0 , 0  } }, // #24 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 4 , { 10, 7 , 0 , 0 , 0 , 0  } }, // #25 [ref=10x]
  { InstDB::RWInfo::kCategoryGeneric   , 3 , { 35, 5 , 0 , 0 , 0 , 0  } }, // #26 [ref=5x]
  { InstDB::RWInfo::kCategoryGeneric   , 4 , { 36, 7 , 0 , 0 , 0 , 0  } }, // #27 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 4 , { 35, 7 , 0 , 0 , 0 , 0  } }, // #28 [ref=11x]
  { InstDB::RWInfo::kCategoryGeneric   , 4 , { 11, 7 , 0 , 0 , 0 , 0  } }, // #29 [ref=9x]
  { InstDB::RWInfo::kCategoryGeneric   , 4 , { 37, 7 , 0 , 0 , 0 , 0  } }, // #30 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 16, { 36, 3 , 0 , 0 , 0 , 0  } }, // #31 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 16, { 37, 3 , 0 , 0 , 0 , 0  } }, // #32 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 5 , { 36, 9 , 0 , 0 , 0 , 0  } }, // #33 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 5 , { 11, 9 , 0 , 0 , 0 , 0  } }, // #34 [ref=7x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 38, 39, 0 , 0 , 0 , 0  } }, // #35 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 17, { 1 , 40, 0 , 0 , 0 , 0  } }, // #36 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 13, { 43, 44, 0 , 0 , 0 , 0  } }, // #37 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 4 , 5 , 0 , 0 , 0 , 0  } }, // #38 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 46, 47, 0 , 0 , 0 , 0  } }, // #39 [ref=6x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 51, 30, 0 , 0 , 0 , 0  } }, // #40 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 0 , 51, 0 , 0 , 0 , 0  } }, // #41 [ref=1x]
  { InstDB::RWInfo::kCategoryImul      , 2 , { 0 , 0 , 0 , 0 , 0 , 0  } }, // #42 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 52, 53, 0 , 0 , 0 , 0  } }, // #43 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 14, { 54, 53, 0 , 0 , 0 , 0  } }, // #44 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 15, { 3 , 5 , 0 , 0 , 0 , 0  } }, // #45 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 22, 29, 0 , 0 , 0 , 0  } }, // #46 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 56, 0 , 0 , 0 , 0 , 0  } }, // #47 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 23, { 57, 40, 0 , 0 , 0 , 0  } }, // #48 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 24, { 45, 9 , 0 , 0 , 0 , 0  } }, // #49 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 25, { 35, 7 , 0 , 0 , 0 , 0  } }, // #50 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 26, { 49, 13, 0 , 0 , 0 , 0  } }, // #51 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 57, 40, 0 , 0 , 0 , 0  } }, // #52 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 45, 9 , 0 , 0 , 0 , 0  } }, // #53 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 35, 7 , 0 , 0 , 0 , 0  } }, // #54 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 49, 13, 0 , 0 , 0 , 0  } }, // #55 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 40, 40, 0 , 0 , 0 , 0  } }, // #56 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 9 , 9 , 0 , 0 , 0 , 0  } }, // #57 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 7 , 7 , 0 , 0 , 0 , 0  } }, // #58 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 13, 13, 0 , 0 , 0 , 0  } }, // #59 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 27, { 11, 3 , 0 , 0 , 0 , 0  } }, // #60 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 15, { 10, 5 , 0 , 0 , 0 , 0  } }, // #61 [ref=5x]
  { InstDB::RWInfo::kCategoryGeneric   , 8 , { 11, 3 , 0 , 0 , 0 , 0  } }, // #62 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 52, 20, 0 , 0 , 0 , 0  } }, // #63 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 59, 0 , 0 , 0 , 0 , 0  } }, // #64 [ref=3x]
  { InstDB::RWInfo::kCategoryMov       , 29, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #65 [ref=1x]
  { InstDB::RWInfo::kCategoryMovabs    , 0 , { 0 , 0 , 0 , 0 , 0 , 0  } }, // #66 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 30, { 10, 5 , 0 , 0 , 0 , 0  } }, // #67 [ref=6x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 11, 3 , 0 , 0 , 0 , 0  } }, // #68 [ref=18x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 36, 63, 0 , 0 , 0 , 0  } }, // #69 [ref=1x]
  { InstDB::RWInfo::kCategoryMovh64    , 12, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #70 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 64, 7 , 0 , 0 , 0 , 0  } }, // #71 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 12, { 35, 7 , 0 , 0 , 0 , 0  } }, // #72 [ref=9x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 57, 5 , 0 , 0 , 0 , 0  } }, // #73 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 28, { 45, 9 , 0 , 0 , 0 , 0  } }, // #74 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 14, { 65, 20, 0 , 0 , 0 , 0  } }, // #75 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 31, { 35, 7 , 0 , 0 , 0 , 0  } }, // #76 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 33, { 45, 9 , 0 , 0 , 0 , 0  } }, // #77 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 16, { 11, 3 , 0 , 0 , 0 , 0  } }, // #78 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 17, 29, 0 , 0 , 0 , 0  } }, // #79 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 11, { 3 , 3 , 0 , 0 , 0 , 0  } }, // #80 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 53, 22, 0 , 0 , 0 , 0  } }, // #81 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 14, { 53, 68, 0 , 0 , 0 , 0  } }, // #82 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 4 , { 26, 7 , 0 , 0 , 0 , 0  } }, // #83 [ref=18x]
  { InstDB::RWInfo::kCategoryGeneric   , 36, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #84 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 3 , { 71, 5 , 0 , 0 , 0 , 0  } }, // #85 [ref=2x]
  { InstDB::RWInfo::kCategoryVmov1_8   , 0 , { 0 , 0 , 0 , 0 , 0 , 0  } }, // #86 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 5 , { 10, 9 , 0 , 0 , 0 , 0  } }, // #87 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 27, { 10, 13, 0 , 0 , 0 , 0  } }, // #88 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 4 , 0 , 0 , 0 , 0 , 0  } }, // #89 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 3 , { 5 , 5 , 0 , 0 , 0 , 0  } }, // #90 [ref=1x]
  { InstDB::RWInfo::kCategoryPunpcklxx , 38, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #91 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 10, { 2 , 72, 0 , 0 , 0 , 0  } }, // #92 [ref=7x]
  { InstDB::RWInfo::kCategoryGeneric   , 5 , { 37, 9 , 0 , 0 , 0 , 0  } }, // #93 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 35, 0 , 0 , 0 , 0 , 0  } }, // #94 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 16, 51, 0 , 0 , 0 , 0  } }, // #95 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 22, 21, 0 , 0 , 0 , 0  } }, // #96 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 65, 22, 0 , 0 , 0 , 0  } }, // #97 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 8 , { 43, 3 , 0 , 0 , 0 , 0  } }, // #98 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 8 , { 11, 44, 0 , 0 , 0 , 0  } }, // #99 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 5 , { 76, 9 , 0 , 0 , 0 , 0  } }, // #100 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 21, { 11, 13, 0 , 0 , 0 , 0  } }, // #101 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 15, { 77, 5 , 0 , 0 , 0 , 0  } }, // #102 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 15, { 11, 5 , 0 , 0 , 0 , 0  } }, // #103 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 43, { 43, 78, 0 , 0 , 0 , 0  } }, // #104 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 44, { 11, 7 , 0 , 0 , 0 , 0  } }, // #105 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 45, { 11, 9 , 0 , 0 , 0 , 0  } }, // #106 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 27, { 13, 13, 0 , 0 , 0 , 0  } }, // #107 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 11, { 11, 3 , 0 , 0 , 0 , 0  } }, // #108 [ref=7x]
  { InstDB::RWInfo::kCategoryVmov2_1   , 46, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #109 [ref=19x]
  { InstDB::RWInfo::kCategoryVmov1_2   , 16, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #110 [ref=11x]
  { InstDB::RWInfo::kCategoryVmov1_4   , 16, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #111 [ref=2x]
  { InstDB::RWInfo::kCategoryVmov4_1   , 47, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #112 [ref=9x]
  { InstDB::RWInfo::kCategoryGeneric   , 16, { 10, 3 , 0 , 0 , 0 , 0  } }, // #113 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 27, { 11, 13, 0 , 0 , 0 , 0  } }, // #114 [ref=5x]
  { InstDB::RWInfo::kCategoryGeneric   , 5 , { 45, 9 , 0 , 0 , 0 , 0  } }, // #115 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 14, { 2 , 3 , 0 , 0 , 0 , 0  } }, // #116 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 57, { 11, 3 , 0 , 0 , 0 , 0  } }, // #117 [ref=12x]
  { InstDB::RWInfo::kCategoryVmovddup  , 38, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #118 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 12, { 35, 63, 0 , 0 , 0 , 0  } }, // #119 [ref=2x]
  { InstDB::RWInfo::kCategoryVmovmskpd , 0 , { 0 , 0 , 0 , 0 , 0 , 0  } }, // #120 [ref=1x]
  { InstDB::RWInfo::kCategoryVmovmskps , 0 , { 0 , 0 , 0 , 0 , 0 , 0  } }, // #121 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 58, { 35, 7 , 0 , 0 , 0 , 0  } }, // #122 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 21, { 49, 13, 0 , 0 , 0 , 0  } }, // #123 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 2 , { 3 , 3 , 0 , 0 , 0 , 0  } }, // #124 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 17, { 11, 40, 0 , 0 , 0 , 0  } }, // #125 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 11, 7 , 0 , 0 , 0 , 0  } }, // #126 [ref=6x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 35, 3 , 0 , 0 , 0 , 0  } }, // #127 [ref=4x]
  { InstDB::RWInfo::kCategoryVmov1_4   , 61, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #128 [ref=6x]
  { InstDB::RWInfo::kCategoryVmov1_2   , 48, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #129 [ref=9x]
  { InstDB::RWInfo::kCategoryVmov1_8   , 62, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #130 [ref=3x]
  { InstDB::RWInfo::kCategoryVmov8_1   , 63, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #131 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 14, { 11, 3 , 0 , 0 , 0 , 0  } }, // #132 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 87, 5 , 0 , 0 , 0 , 0  } }, // #133 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 87, 78, 0 , 0 , 0 , 0  } }, // #134 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 11, { 2 , 2 , 0 , 0 , 0 , 0  } }, // #135 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 57, { 2 , 2 , 0 , 0 , 0 , 0  } }  // #136 [ref=1x]
};

const InstDB::RWInfo InstDB::rw_info_b_table[] = {
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 0 , 0 , 0 , 0 , 0 , 0  } }, // #0 [ref=758x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 1 , 0 , 0 , 0 , 0 , 0  } }, // #1 [ref=5x]
  { InstDB::RWInfo::kCategoryGeneric   , 3 , { 10, 5 , 0 , 0 , 0 , 0  } }, // #2 [ref=7x]
  { InstDB::RWInfo::kCategoryGeneric   , 6 , { 11, 3 , 3 , 0 , 0 , 0  } }, // #3 [ref=193x]
  { InstDB::RWInfo::kCategoryGeneric   , 2 , { 11, 3 , 3 , 0 , 0 , 0  } }, // #4 [ref=5x]
  { InstDB::RWInfo::kCategoryGeneric   , 3 , { 4 , 5 , 0 , 0 , 0 , 0  } }, // #5 [ref=14x]
  { InstDB::RWInfo::kCategoryGeneric   , 3 , { 4 , 5 , 14, 0 , 0 , 0  } }, // #6 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 2 , 0 , 0 , 0 , 0 , 0  } }, // #7 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 11, { 3 , 0 , 0 , 0 , 0 , 0  } }, // #8 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 18, 0 , 0 , 0 , 0 , 0  } }, // #9 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 8 , { 3 , 0 , 0 , 0 , 0 , 0  } }, // #10 [ref=21x]
  { InstDB::RWInfo::kCategoryGeneric   , 12, { 7 , 0 , 0 , 0 , 0 , 0  } }, // #11 [ref=5x]
  { InstDB::RWInfo::kCategoryGeneric   , 13, { 19, 0 , 0 , 0 , 0 , 0  } }, // #12 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 2 , 2 , 3 , 0 , 0 , 0  } }, // #13 [ref=16x]
  { InstDB::RWInfo::kCategoryGeneric   , 4 , { 6 , 7 , 0 , 0 , 0 , 0  } }, // #14 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 5 , { 8 , 9 , 0 , 0 , 0 , 0  } }, // #15 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 11, { 2 , 3 , 22, 0 , 0 , 0  } }, // #16 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 15, { 4 , 23, 18, 24, 25, 0  } }, // #17 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 12, { 26, 27, 28, 29, 30, 0  } }, // #18 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 28, 31, 32, 16, 0 , 0  } }, // #19 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 28, 0 , 0 , 0 , 0 , 0  } }, // #20 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 10, { 2 , 0 , 0 , 0 , 0 , 0  } }, // #21 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 6 , { 41, 42, 3 , 0 , 0 , 0  } }, // #22 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 18, { 45, 5 , 0 , 0 , 0 , 0  } }, // #23 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 4 , 0 , 0 , 0 , 0 , 0  } }, // #24 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 14, { 3 , 0 , 0 , 0 , 0 , 0  } }, // #25 [ref=17x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 46, 0 , 0 , 0 , 0 , 0  } }, // #26 [ref=16x]
  { InstDB::RWInfo::kCategoryGeneric   , 19, { 47, 0 , 0 , 0 , 0 , 0  } }, // #27 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 19, { 48, 0 , 0 , 0 , 0 , 0  } }, // #28 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 20, { 3 , 0 , 0 , 0 , 0 , 0  } }, // #29 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 47, 0 , 0 , 0 , 0 , 0  } }, // #30 [ref=6x]
  { InstDB::RWInfo::kCategoryGeneric   , 14, { 11, 0 , 0 , 0 , 0 , 0  } }, // #31 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 21, { 13, 0 , 0 , 0 , 0 , 0  } }, // #32 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 8 , { 11, 0 , 0 , 0 , 0 , 0  } }, // #33 [ref=8x]
  { InstDB::RWInfo::kCategoryGeneric   , 21, { 49, 0 , 0 , 0 , 0 , 0  } }, // #34 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 7 , { 50, 0 , 0 , 0 , 0 , 0  } }, // #35 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 20, { 11, 0 , 0 , 0 , 0 , 0  } }, // #36 [ref=2x]
  { InstDB::RWInfo::kCategoryImul      , 22, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #37 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 40, 0 , 0 , 0 , 0 , 0  } }, // #38 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 5 , { 4 , 9 , 0 , 0 , 0 , 0  } }, // #39 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 4 , 5 , 0 , 0 , 0 , 0  } }, // #40 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 22, 55, 56, 0 , 0 , 0  } }, // #41 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 57, 40, 40, 0 , 0 , 0  } }, // #42 [ref=6x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 45, 9 , 9 , 0 , 0 , 0  } }, // #43 [ref=6x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 35, 7 , 7 , 0 , 0 , 0  } }, // #44 [ref=6x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 49, 13, 13, 0 , 0 , 0  } }, // #45 [ref=6x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 57, 40, 0 , 0 , 0 , 0  } }, // #46 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 45, 9 , 0 , 0 , 0 , 0  } }, // #47 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 35, 7 , 0 , 0 , 0 , 0  } }, // #48 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 49, 13, 0 , 0 , 0 , 0  } }, // #49 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 49, 40, 40, 0 , 0 , 0  } }, // #50 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 35, 9 , 9 , 0 , 0 , 0  } }, // #51 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 45, 13, 13, 0 , 0 , 0  } }, // #52 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 58, 0 , 0 , 0 , 0 , 0  } }, // #53 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 28, { 9 , 0 , 0 , 0 , 0 , 0  } }, // #54 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 13, { 44, 0 , 0 , 0 , 0 , 0  } }, // #55 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 7 , { 13, 0 , 0 , 0 , 0 , 0  } }, // #56 [ref=5x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 3 , 0 , 0 , 0 , 0 , 0  } }, // #57 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 5 , { 3 , 9 , 0 , 0 , 0 , 0  } }, // #58 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 15, { 5 , 5 , 60, 0 , 0 , 0  } }, // #59 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 12, { 7 , 7 , 61, 0 , 0 , 0  } }, // #60 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 8 , { 62, 29, 55, 0 , 0 , 0  } }, // #61 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 32, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #62 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 6 , { 66, 42, 3 , 0 , 0 , 0  } }, // #63 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 6 , { 11, 11, 3 , 67, 0 , 0  } }, // #64 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 17, 29, 30, 0 , 0 , 0  } }, // #65 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 10, { 3 , 0 , 0 , 0 , 0 , 0  } }, // #66 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 2 , { 2 , 3 , 0 , 0 , 0 , 0  } }, // #67 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 3 , { 5 , 5 , 0 , 69, 17, 55 } }, // #68 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 3 , { 5 , 5 , 0 , 70, 17, 55 } }, // #69 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 3 , { 5 , 5 , 0 , 69, 0 , 0  } }, // #70 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 3 , { 5 , 5 , 0 , 70, 0 , 0  } }, // #71 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 34, { 57, 5 , 0 , 0 , 0 , 0  } }, // #72 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 35, { 35, 5 , 0 , 0 , 0 , 0  } }, // #73 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 37, { 49, 3 , 0 , 0 , 0 , 0  } }, // #74 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 17, { 4 , 40, 0 , 0 , 0 , 0  } }, // #75 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 4 , { 4 , 7 , 0 , 0 , 0 , 0  } }, // #76 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 27, { 2 , 13, 0 , 0 , 0 , 0  } }, // #77 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 10, { 11, 0 , 0 , 0 , 0 , 0  } }, // #78 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 4 , { 35, 7 , 0 , 0 , 0 , 0  } }, // #79 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 11, 0 , 0 , 0 , 0 , 0  } }, // #80 [ref=6x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 16, 51, 29, 0 , 0 , 0  } }, // #81 [ref=5x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 45, 0 , 0 , 0 , 0 , 0  } }, // #82 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 35, 0 , 0 , 0 , 0 , 0  } }, // #83 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 16, 51, 69, 0 , 0 , 0  } }, // #84 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 2 , { 11, 3 , 0 , 0 , 0 , 0  } }, // #85 [ref=19x]
  { InstDB::RWInfo::kCategoryGeneric   , 4 , { 36, 7 , 0 , 0 , 0 , 0  } }, // #86 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 5 , { 37, 9 , 0 , 0 , 0 , 0  } }, // #87 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 73, 0 , 0 , 0 , 0 , 0  } }, // #88 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 7 , 0 , 0 , 0 , 0 , 0  } }, // #89 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 34, { 74, 0 , 0 , 0 , 0 , 0  } }, // #90 [ref=16x]
  { InstDB::RWInfo::kCategoryGeneric   , 11, { 2 , 3 , 72, 0 , 0 , 0  } }, // #91 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 39, { 11, 0 , 0 , 0 , 0 , 0  } }, // #92 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 28, { 45, 0 , 0 , 0 , 0 , 0  } }, // #93 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 13, { 43, 0 , 0 , 0 , 0 , 0  } }, // #94 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 75, 44, 44, 0 , 0 , 0  } }, // #95 [ref=8x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 43, 0 , 0 , 0 , 0 , 0  } }, // #96 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 9 , 55, 17, 0 , 0 , 0  } }, // #97 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 40, { 10, 5 , 7 , 0 , 0 , 0  } }, // #98 [ref=9x]
  { InstDB::RWInfo::kCategoryGeneric   , 41, { 10, 5 , 13, 0 , 0 , 0  } }, // #99 [ref=9x]
  { InstDB::RWInfo::kCategoryGeneric   , 42, { 10, 5 , 9 , 0 , 0 , 0  } }, // #100 [ref=9x]
  { InstDB::RWInfo::kCategoryGeneric   , 6 , { 11, 3 , 3 , 3 , 0 , 0  } }, // #101 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 6 , { 35, 3 , 3 , 0 , 0 , 0  } }, // #102 [ref=18x]
  { InstDB::RWInfo::kCategoryGeneric   , 40, { 11, 5 , 7 , 0 , 0 , 0  } }, // #103 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 41, { 35, 13, 13, 0 , 0 , 0  } }, // #104 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 42, { 11, 5 , 9 , 0 , 0 , 0  } }, // #105 [ref=1x]
  { InstDB::RWInfo::kCategoryVmov1_2   , 48, { 0 , 0 , 0 , 0 , 0 , 0  } }, // #106 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 40, { 10, 79, 7 , 0 , 0 , 0  } }, // #107 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 41, { 10, 5 , 5 , 0 , 0 , 0  } }, // #108 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 49, { 10, 63, 3 , 0 , 0 , 0  } }, // #109 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 49, { 10, 3 , 3 , 0 , 0 , 0  } }, // #110 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 49, { 10, 79, 3 , 0 , 0 , 0  } }, // #111 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 42, { 10, 63, 9 , 0 , 0 , 0  } }, // #112 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 42, { 10, 5 , 5 , 0 , 0 , 0  } }, // #113 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 50, { 10, 5 , 5 , 0 , 0 , 0  } }, // #114 [ref=9x]
  { InstDB::RWInfo::kCategoryGeneric   , 51, { 10, 78, 0 , 0 , 0 , 0  } }, // #115 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 51, { 10, 3 , 0 , 0 , 0 , 0  } }, // #116 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 52, { 77, 44, 0 , 0 , 0 , 0  } }, // #117 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 6 , { 80, 3 , 3 , 0 , 0 , 0  } }, // #118 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 42, { 81, 5 , 5 , 0 , 0 , 0  } }, // #119 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 6 , { 2 , 3 , 3 , 0 , 0 , 0  } }, // #120 [ref=90x]
  { InstDB::RWInfo::kCategoryGeneric   , 40, { 4 , 63, 7 , 0 , 0 , 0  } }, // #121 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 42, { 4 , 79, 9 , 0 , 0 , 0  } }, // #122 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 40, { 6 , 7 , 7 , 0 , 0 , 0  } }, // #123 [ref=11x]
  { InstDB::RWInfo::kCategoryGeneric   , 41, { 82, 5 , 5 , 0 , 0 , 0  } }, // #124 [ref=6x]
  { InstDB::RWInfo::kCategoryGeneric   , 42, { 8 , 9 , 9 , 0 , 0 , 0  } }, // #125 [ref=11x]
  { InstDB::RWInfo::kCategoryGeneric   , 53, { 11, 3 , 3 , 3 , 0 , 0  } }, // #126 [ref=15x]
  { InstDB::RWInfo::kCategoryGeneric   , 54, { 35, 7 , 7 , 7 , 0 , 0  } }, // #127 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 55, { 45, 9 , 9 , 9 , 0 , 0  } }, // #128 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 41, { 82, 5 , 13, 0 , 0 , 0  } }, // #129 [ref=6x]
  { InstDB::RWInfo::kCategoryGeneric   , 42, { 83, 5 , 5 , 0 , 0 , 0  } }, // #130 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 40, { 26, 7 , 7 , 0 , 0 , 0  } }, // #131 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 42, { 76, 9 , 9 , 0 , 0 , 0  } }, // #132 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 16, { 35, 3 , 0 , 0 , 0 , 0  } }, // #133 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 27, { 35, 13, 0 , 0 , 0 , 0  } }, // #134 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 5 , { 35, 9 , 0 , 0 , 0 , 0  } }, // #135 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 8 , { 2 , 3 , 2 , 0 , 0 , 0  } }, // #136 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 2 , 3 , 2 , 0 , 0 , 0  } }, // #137 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 14, { 4 , 3 , 4 , 0 , 0 , 0  } }, // #138 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 40, { 10, 63, 7 , 0 , 0 , 0  } }, // #139 [ref=9x]
  { InstDB::RWInfo::kCategoryGeneric   , 41, { 10, 84, 13, 0 , 0 , 0  } }, // #140 [ref=7x]
  { InstDB::RWInfo::kCategoryGeneric   , 42, { 10, 79, 9 , 0 , 0 , 0  } }, // #141 [ref=11x]
  { InstDB::RWInfo::kCategoryGeneric   , 50, { 77, 78, 5 , 0 , 0 , 0  } }, // #142 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 50, { 11, 3 , 5 , 0 , 0 , 0  } }, // #143 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 56, { 43, 44, 78, 0 , 0 , 0  } }, // #144 [ref=4x]
  { InstDB::RWInfo::kCategoryVmaskmov  , 0 , { 0 , 0 , 0 , 0 , 0 , 0  } }, // #145 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 22, 0 , 0 , 0 , 0 , 0  } }, // #146 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 10, 63, 63, 0 , 0 , 0  } }, // #147 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 12, { 10, 7 , 7 , 0 , 0 , 0  } }, // #148 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 10, 7 , 7 , 0 , 0 , 0  } }, // #149 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 12, { 10, 63, 7 , 0 , 0 , 0  } }, // #150 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 10, 63, 7 , 0 , 0 , 0  } }, // #151 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 10, 84, 13, 0 , 0 , 0  } }, // #152 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 10, 79, 9 , 0 , 0 , 0  } }, // #153 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 12, { 35, 0 , 0 , 0 , 0 , 0  } }, // #154 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 85, 0 , 0 , 0 , 0 , 0  } }, // #155 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 59, { 35, 86, 3 , 3 , 0 , 0  } }, // #156 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 56, { 77, 78, 78, 0 , 0 , 0  } }, // #157 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 22, { 11, 3 , 3 , 0 , 0 , 0  } }, // #158 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 7 , { 49, 5 , 0 , 0 , 0 , 0  } }, // #159 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 60, { 10, 5 , 40, 0 , 0 , 0  } }, // #160 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 50, { 10, 5 , 5 , 5 , 0 , 0  } }, // #161 [ref=12x]
  { InstDB::RWInfo::kCategoryGeneric   , 64, { 10, 5 , 5 , 5 , 0 , 0  } }, // #162 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 65, { 10, 5 , 5 , 0 , 0 , 0  } }, // #163 [ref=12x]
  { InstDB::RWInfo::kCategoryGeneric   , 66, { 11, 3 , 5 , 0 , 0 , 0  } }, // #164 [ref=5x]
  { InstDB::RWInfo::kCategoryGeneric   , 67, { 11, 3 , 0 , 0 , 0 , 0  } }, // #165 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 68, { 11, 3 , 5 , 0 , 0 , 0  } }, // #166 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 22, { 11, 3 , 5 , 0 , 0 , 0  } }, // #167 [ref=1x]
  { InstDB::RWInfo::kCategoryGenericEx , 6 , { 2 , 3 , 3 , 0 , 0 , 0  } }, // #168 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 87, 78, 5 , 0 , 0 , 0  } }, // #169 [ref=1x]
  { InstDB::RWInfo::kCategoryGeneric   , 50, { 4 , 5 , 5 , 0 , 0 , 0  } }, // #170 [ref=3x]
  { InstDB::RWInfo::kCategoryGeneric   , 0 , { 55, 17, 29, 0 , 0 , 0  } }, // #171 [ref=2x]
  { InstDB::RWInfo::kCategoryGeneric   , 8 , { 3 , 55, 17, 0 , 0 , 0  } }, // #172 [ref=4x]
  { InstDB::RWInfo::kCategoryGeneric   , 8 , { 11, 55, 17, 0 , 0 , 0  } }  // #173 [ref=8x]
};

const InstDB::RWInfoOp InstDB::rw_info_op_table[] = {
  { 0x0000000000000000u, 0x0000000000000000u, 0xFF, 0, { 0 }, OpRWFlags::kNone }, // #0 [ref=16348x]
  { 0x0000000000000003u, 0x0000000000000003u, 0x00, 0, { 0 }, OpRWFlags::kRW | OpRWFlags::kRegPhysId }, // #1 [ref=10x]
  { 0x0000000000000000u, 0x0000000000000000u, 0xFF, 0, { 0 }, OpRWFlags::kRW | OpRWFlags::kZExt }, // #2 [ref=267x]
  { 0x0000000000000000u, 0x0000000000000000u, 0xFF, 0, { 0 }, OpRWFlags::kRead }, // #3 [ref=1091x]
  { 0x000000000000FFFFu, 0x000000000000FFFFu, 0xFF, 0, { 0 }, OpRWFlags::kRW | OpRWFlags::kZExt }, // #4 [ref=93x]
  { 0x000000000000FFFFu, 0x0000000000000000u, 0xFF, 0, { 0 }, OpRWFlags::kRead }, // #5 [ref=338x]
  { 0x00000000000000FFu, 0x00000000000000FFu, 0xFF, 0, { 0 }, OpRWFlags::kRW }, // #6 [ref=18x]
  { 0x00000000000000FFu, 0x0000000000000000u, 0xFF, 0, { 0 }, OpRWFlags::kRead }, // #7 [ref=186x]
  { 0x000000000000000Fu, 0x000000000000000Fu, 0xFF, 0, { 0 }, OpRWFlags::kRW }, // #8 [ref=18x]
  { 0x000000000000000Fu, 0x0000000000000000u, 0xFF, 0, { 0 }, OpRWFlags::kRead }, // #9 [ref=133x]
  { 0x0000000000000000u, 0x000000000000FFFFu, 0xFF, 0, { 0 }, OpRWFlags::kWrite | OpRWFlags::kZExt }, // #10 [ref=178x]
  { 0x0000000000000000u, 0x0000000000000000u, 0xFF, 0, { 0 }, OpRWFlags::kWrite | OpRWFlags::kZExt }, // #11 [ref=445x]
  { 0x0000000000000003u, 0x0000000000000003u, 0xFF, 0, { 0 }, OpRWFlags::kRW }, // #12 [ref=1x]
  { 0x0000000000000003u, 0x0000000000000000u, 0xFF, 0, { 0 }, OpRWFlags::kRead }, // #13 [ref=71x]
  { 0x000000000000FFFFu, 0x0000000000000000u, 0x00, 0, { 0 }, OpRWFlags::kRead | OpRWFlags::kRegPhysId }, // #14 [ref=4x]
  { 0x0000000000000000u, 0x0000000000000000u, 0xFF, 0, { 0 }, OpRWFlags::kWrite | OpRWFlags::kZExt | OpRWFlags::kMemBaseWrite | OpRWFlags::kMemIndexWrite }, // #15 [ref=1x]
  { 0x0000000000000000u, 0x000000000000000Fu, 0x02, 0, { 0 }, OpRWFlags::kWrite | OpRWFlags::kZExt | OpRWFlags::kRegPhysId }, // #16 [ref=9x]
  { 0x000000000000000Fu, 0x0000000000000000u, 0x00, 0, { 0 }, OpRWFlags::kRead | OpRWFlags::kRegPhysId }, // #17 [ref=23x]
  { 0x00000000000000FFu, 0x00000000000000FFu, 0x00, 0, { 0 }, OpRWFlags::kRW | OpRWFlags::kZExt | OpRWFlags::kRegPhysId }, // #18 [ref=2x]
  { 0xFFFFFFFFFFFFFFFFu, 0x0000000000000000u, 0x00, 0, { 0 }, OpRWFlags::kRead | OpRWFlags::kMemPhysId }, // #19 [ref=1x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x06, 0, { 0 }, OpRWFlags::kRead | OpRWFlags::kMemBaseRW | OpRWFlags::kMemBasePostModify | OpRWFlags::kMemPhysId }, // #20 [ref=3x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x07, 0, { 0 }, OpRWFlags::kRead | OpRWFlags::kMemBaseRW | OpRWFlags::kMemBasePostModify | OpRWFlags::kMemPhysId }, // #21 [ref=2x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x00, 0, { 0 }, OpRWFlags::kRead | OpRWFlags::kRegPhysId }, // #22 [ref=8x]
  { 0x00000000000000FFu, 0x00000000000000FFu, 0x02, 0, { 0 }, OpRWFlags::kRW | OpRWFlags::kZExt | OpRWFlags::kRegPhysId }, // #23 [ref=1x]
  { 0x00000000000000FFu, 0x0000000000000000u, 0x01, 0, { 0 }, OpRWFlags::kRead | OpRWFlags::kRegPhysId }, // #24 [ref=1x]
  { 0x00000000000000FFu, 0x0000000000000000u, 0x03, 0, { 0 }, OpRWFlags::kRead | OpRWFlags::kRegPhysId }, // #25 [ref=1x]
  { 0x00000000000000FFu, 0x00000000000000FFu, 0xFF, 0, { 0 }, OpRWFlags::kRW | OpRWFlags::kZExt }, // #26 [ref=20x]
  { 0x000000000000000Fu, 0x000000000000000Fu, 0x02, 0, { 0 }, OpRWFlags::kRW | OpRWFlags::kZExt | OpRWFlags::kRegPhysId }, // #27 [ref=1x]
  { 0x000000000000000Fu, 0x000000000000000Fu, 0x00, 0, { 0 }, OpRWFlags::kRW | OpRWFlags::kZExt | OpRWFlags::kRegPhysId }, // #28 [ref=4x]
  { 0x000000000000000Fu, 0x0000000000000000u, 0x01, 0, { 0 }, OpRWFlags::kRead | OpRWFlags::kRegPhysId }, // #29 [ref=13x]
  { 0x000000000000000Fu, 0x0000000000000000u, 0x03, 0, { 0 }, OpRWFlags::kRead | OpRWFlags::kRegPhysId }, // #30 [ref=3x]
  { 0x0000000000000000u, 0x000000000000000Fu, 0x03, 0, { 0 }, OpRWFlags::kWrite | OpRWFlags::kZExt | OpRWFlags::kRegPhysId }, // #31 [ref=1x]
  { 0x000000000000000Fu, 0x000000000000000Fu, 0x01, 0, { 0 }, OpRWFlags::kRW | OpRWFlags::kZExt | OpRWFlags::kRegPhysId }, // #32 [ref=1x]
  { 0x0000000000000000u, 0x00000000000000FFu, 0x02, 0, { 0 }, OpRWFlags::kWrite | OpRWFlags::kZExt | OpRWFlags::kRegPhysId }, // #33 [ref=1x]
  { 0x00000000000000FFu, 0x0000000000000000u, 0x00, 0, { 0 }, OpRWFlags::kRead | OpRWFlags::kRegPhysId }, // #34 [ref=1x]
  { 0x0000000000000000u, 0x00000000000000FFu, 0xFF, 0, { 0 }, OpRWFlags::kWrite | OpRWFlags::kZExt }, // #35 [ref=84x]
  { 0x0000000000000000u, 0x00000000000000FFu, 0xFF, 0, { 0 }, OpRWFlags::kWrite }, // #36 [ref=6x]
  { 0x0000000000000000u, 0x000000000000000Fu, 0xFF, 0, { 0 }, OpRWFlags::kWrite }, // #37 [ref=6x]
  { 0x0000000000000000u, 0x0000000000000003u, 0x02, 0, { 0 }, OpRWFlags::kWrite | OpRWFlags::kRegPhysId }, // #38 [ref=1x]
  { 0x0000000000000003u, 0x0000000000000000u, 0x00, 0, { 0 }, OpRWFlags::kRead | OpRWFlags::kRegPhysId }, // #39 [ref=1x]
  { 0x0000000000000001u, 0x0000000000000000u, 0xFF, 0, { 0 }, OpRWFlags::kRead }, // #40 [ref=30x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x02, 0, { 0 }, OpRWFlags::kRW | OpRWFlags::kRegPhysId | OpRWFlags::kZExt }, // #41 [ref=2x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x00, 0, { 0 }, OpRWFlags::kRW | OpRWFlags::kRegPhysId | OpRWFlags::kZExt }, // #42 [ref=3x]
  { 0x0000000000000000u, 0xFFFFFFFFFFFFFFFFu, 0xFF, 0, { 0 }, OpRWFlags::kWrite | OpRWFlags::kZExt }, // #43 [ref=15x]
  { 0xFFFFFFFFFFFFFFFFu, 0x0000000000000000u, 0xFF, 0, { 0 }, OpRWFlags::kRead }, // #44 [ref=29x]
  { 0x0000000000000000u, 0x000000000000000Fu, 0xFF, 0, { 0 }, OpRWFlags::kWrite | OpRWFlags::kZExt }, // #45 [ref=30x]
  { 0x00000000000003FFu, 0x00000000000003FFu, 0xFF, 0, { 0 }, OpRWFlags::kRW | OpRWFlags::kZExt }, // #46 [ref=22x]
  { 0x00000000000003FFu, 0x0000000000000000u, 0xFF, 0, { 0 }, OpRWFlags::kRead }, // #47 [ref=13x]
  { 0x0000000000000000u, 0x00000000000003FFu, 0xFF, 0, { 0 }, OpRWFlags::kWrite | OpRWFlags::kZExt }, // #48 [ref=1x]
  { 0x0000000000000000u, 0x0000000000000003u, 0xFF, 0, { 0 }, OpRWFlags::kWrite | OpRWFlags::kZExt }, // #49 [ref=17x]
  { 0x0000000000000000u, 0x0000000000000003u, 0x00, 0, { 0 }, OpRWFlags::kWrite | OpRWFlags::kRegPhysId | OpRWFlags::kZExt }, // #50 [ref=2x]
  { 0x0000000000000000u, 0x000000000000000Fu, 0x00, 0, { 0 }, OpRWFlags::kWrite | OpRWFlags::kZExt | OpRWFlags::kRegPhysId }, // #51 [ref=9x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x00, 0, { 0 }, OpRWFlags::kWrite | OpRWFlags::kRegPhysId | OpRWFlags::kZExt }, // #52 [ref=2x]
  { 0x0000000000000003u, 0x0000000000000000u, 0x02, 0, { 0 }, OpRWFlags::kRead | OpRWFlags::kRegPhysId }, // #53 [ref=4x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x07, 0, { 0 }, OpRWFlags::kWrite | OpRWFlags::kZExt | OpRWFlags::kMemPhysId }, // #54 [ref=1x]
  { 0x000000000000000Fu, 0x0000000000000000u, 0x02, 0, { 0 }, OpRWFlags::kRead | OpRWFlags::kRegPhysId }, // #55 [ref=23x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x01, 0, { 0 }, OpRWFlags::kRead | OpRWFlags::kRegPhysId }, // #56 [ref=2x]
  { 0x0000000000000000u, 0x0000000000000001u, 0xFF, 0, { 0 }, OpRWFlags::kWrite | OpRWFlags::kZExt }, // #57 [ref=14x]
  { 0x0000000000000000u, 0x0000000000000001u, 0x00, 0, { 0 }, OpRWFlags::kWrite | OpRWFlags::kRegPhysId }, // #58 [ref=1x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x01, 0, { 0 }, OpRWFlags::kRW | OpRWFlags::kRegPhysId | OpRWFlags::kZExt }, // #59 [ref=3x]
  { 0x000000000000FFFFu, 0x000000000000FFFFu, 0x07, 0, { 0 }, OpRWFlags::kRW | OpRWFlags::kZExt | OpRWFlags::kMemPhysId }, // #60 [ref=2x]
  { 0x00000000000000FFu, 0x00000000000000FFu, 0x07, 0, { 0 }, OpRWFlags::kRW | OpRWFlags::kZExt | OpRWFlags::kMemPhysId }, // #61 [ref=1x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x00, 0, { 0 }, OpRWFlags::kRead | OpRWFlags::kMemPhysId }, // #62 [ref=2x]
  { 0x000000000000FF00u, 0x0000000000000000u, 0xFF, 0, { 0 }, OpRWFlags::kRead }, // #63 [ref=21x]
  { 0x0000000000000000u, 0x000000000000FF00u, 0xFF, 0, { 0 }, OpRWFlags::kWrite }, // #64 [ref=1x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x07, 0, { 0 }, OpRWFlags::kWrite | OpRWFlags::kZExt | OpRWFlags::kMemBaseRW | OpRWFlags::kMemBasePostModify | OpRWFlags::kMemPhysId }, // #65 [ref=2x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x02, 0, { 0 }, OpRWFlags::kWrite | OpRWFlags::kRegPhysId | OpRWFlags::kZExt }, // #66 [ref=1x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x02, 0, { 0 }, OpRWFlags::kRead | OpRWFlags::kRegPhysId }, // #67 [ref=1x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x06, 0, { 0 }, OpRWFlags::kRead | OpRWFlags::kMemPhysId }, // #68 [ref=1x]
  { 0x0000000000000000u, 0x000000000000000Fu, 0x01, 0, { 0 }, OpRWFlags::kWrite | OpRWFlags::kZExt | OpRWFlags::kRegPhysId }, // #69 [ref=5x]
  { 0x0000000000000000u, 0x000000000000FFFFu, 0x00, 0, { 0 }, OpRWFlags::kWrite | OpRWFlags::kZExt | OpRWFlags::kRegPhysId }, // #70 [ref=4x]
  { 0x0000000000000000u, 0x0000000000000007u, 0xFF, 0, { 0 }, OpRWFlags::kWrite | OpRWFlags::kZExt }, // #71 [ref=2x]
  { 0x0000000000000001u, 0x0000000000000000u, 0x01, 0, { 0 }, OpRWFlags::kRead | OpRWFlags::kRegPhysId }, // #72 [ref=9x]
  { 0x0000000000000001u, 0x0000000000000000u, 0x00, 0, { 0 }, OpRWFlags::kRead | OpRWFlags::kRegPhysId }, // #73 [ref=1x]
  { 0x0000000000000000u, 0x0000000000000001u, 0xFF, 0, { 0 }, OpRWFlags::kWrite }, // #74 [ref=16x]
  { 0xFFFFFFFFFFFFFFFFu, 0xFFFFFFFFFFFFFFFFu, 0xFF, 0, { 0 }, OpRWFlags::kRW | OpRWFlags::kZExt }, // #75 [ref=8x]
  { 0x000000000000000Fu, 0x000000000000000Fu, 0xFF, 0, { 0 }, OpRWFlags::kRW | OpRWFlags::kZExt }, // #76 [ref=3x]
  { 0x0000000000000000u, 0x00000000FFFFFFFFu, 0xFF, 0, { 0 }, OpRWFlags::kWrite | OpRWFlags::kZExt }, // #77 [ref=10x]
  { 0x00000000FFFFFFFFu, 0x0000000000000000u, 0xFF, 0, { 0 }, OpRWFlags::kRead }, // #78 [ref=18x]
  { 0x000000000000FFF0u, 0x0000000000000000u, 0xFF, 0, { 0 }, OpRWFlags::kRead }, // #79 [ref=16x]
  { 0x0000000000000000u, 0x0000000000000000u, 0xFF, 0, { 0 }, OpRWFlags::kRW | OpRWFlags::kUnique | OpRWFlags::kZExt }, // #80 [ref=4x]
  { 0x000000000000FFFFu, 0x000000000000FFFFu, 0xFF, 0, { 0 }, OpRWFlags::kRW | OpRWFlags::kUnique }, // #81 [ref=3x]
  { 0x000000000000FFFFu, 0x000000000000FFFFu, 0xFF, 0, { 0 }, OpRWFlags::kRW }, // #82 [ref=12x]
  { 0x000000000000FFFFu, 0x000000000000FFFFu, 0xFF, 0, { 0 }, OpRWFlags::kRW | OpRWFlags::kUnique | OpRWFlags::kZExt }, // #83 [ref=1x]
  { 0x000000000000FFFCu, 0x0000000000000000u, 0xFF, 0, { 0 }, OpRWFlags::kRead }, // #84 [ref=8x]
  { 0x0000000000000000u, 0x0000000000000000u, 0x00, 0, { 0 }, OpRWFlags::kRW | OpRWFlags::kZExt | OpRWFlags::kRegPhysId }, // #85 [ref=1x]
  { 0x0000000000000000u, 0x0000000000000000u, 0xFF, 0, { 0 }, OpRWFlags::kWrite | OpRWFlags::kZExt | OpRWFlags::kConsecutive }, // #86 [ref=2x]
  { 0x00000000FFFFFFFFu, 0x00000000FFFFFFFFu, 0xFF, 0, { 0 }, OpRWFlags::kRW | OpRWFlags::kZExt }  // #87 [ref=3x]
};

const InstDB::RWInfoRm InstDB::rw_info_rm_table[] = {
  { InstDB::RWInfoRm::kCategoryNone      , 0x00, 0 , 0, 0 }, // #0 [ref=1996x]
  { InstDB::RWInfoRm::kCategoryConsistent, 0x03, 0 , InstDB::RWInfoRm::kFlagAmbiguous, 0 }, // #1 [ref=8x]
  { InstDB::RWInfoRm::kCategoryConsistent, 0x02, 0 , 0, 0 }, // #2 [ref=190x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x02, 16, 0, 0 }, // #3 [ref=122x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x02, 8 , 0, 0 }, // #4 [ref=66x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x02, 4 , 0, 0 }, // #5 [ref=35x]
  { InstDB::RWInfoRm::kCategoryConsistent, 0x04, 0 , 0, 0 }, // #6 [ref=314x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x01, 2 , 0, 0 }, // #7 [ref=9x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x00, 0 , 0, 0 }, // #8 [ref=52x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x03, 0 , 0, 0 }, // #9 [ref=1x]
  { InstDB::RWInfoRm::kCategoryConsistent, 0x01, 0 , InstDB::RWInfoRm::kFlagAmbiguous, 0 }, // #10 [ref=20x]
  { InstDB::RWInfoRm::kCategoryConsistent, 0x01, 0 , 0, 0 }, // #11 [ref=14x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x00, 8 , 0, 0 }, // #12 [ref=25x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x00, 64, 0, 0 }, // #13 [ref=6x]
  { InstDB::RWInfoRm::kCategoryNone      , 0x00, 0 , InstDB::RWInfoRm::kFlagAmbiguous, 0 }, // #14 [ref=30x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x00, 16, 0, 0 }, // #15 [ref=17x]
  { InstDB::RWInfoRm::kCategoryConsistent, 0x02, 0 , InstDB::RWInfoRm::kFlagAmbiguous, 0 }, // #16 [ref=22x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x02, 1 , 0, 0 }, // #17 [ref=5x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x01, 4 , 0, 0 }, // #18 [ref=4x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x00, 10, 0, 0 }, // #19 [ref=2x]
  { InstDB::RWInfoRm::kCategoryNone      , 0x01, 0 , InstDB::RWInfoRm::kFlagAmbiguous, 0 }, // #20 [ref=5x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x00, 2 , 0, 0 }, // #21 [ref=6x]
  { InstDB::RWInfoRm::kCategoryConsistent, 0x06, 0 , 0, 0 }, // #22 [ref=6x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x03, 1 , 0, 0 }, // #23 [ref=1x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x03, 4 , 0, 0 }, // #24 [ref=3x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x03, 8 , 0, 0 }, // #25 [ref=2x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x03, 2 , 0, 0 }, // #26 [ref=2x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x02, 2 , 0, 0 }, // #27 [ref=13x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x00, 4 , 0, 0 }, // #28 [ref=8x]
  { InstDB::RWInfoRm::kCategoryNone      , 0x03, 0 , InstDB::RWInfoRm::kFlagAmbiguous, 0 }, // #29 [ref=1x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x03, 16, 0, 0 }, // #30 [ref=6x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x03, 8 , InstDB::RWInfoRm::kFlagMovssMovsd, 0 }, // #31 [ref=1x]
  { InstDB::RWInfoRm::kCategoryNone      , 0x00, 0 , InstDB::RWInfoRm::kFlagMovssMovsd, 0 }, // #32 [ref=2x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x03, 4 , InstDB::RWInfoRm::kFlagMovssMovsd, 0 }, // #33 [ref=1x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x01, 1 , 0, 0 }, // #34 [ref=18x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x01, 8 , 0, 0 }, // #35 [ref=2x]
  { InstDB::RWInfoRm::kCategoryNone      , 0x00, 0 , InstDB::RWInfoRm::kFlagPextrw, 0 }, // #36 [ref=1x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x01, 2 , InstDB::RWInfoRm::kFlagPextrw, uint32_t(CpuFeatures::X86::kSSE4_1) }, // #37 [ref=1x]
  { InstDB::RWInfoRm::kCategoryNone      , 0x02, 0 , 0, 0 }, // #38 [ref=4x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x01, 2 , InstDB::RWInfoRm::kFlagAmbiguous, 0 }, // #39 [ref=3x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x04, 8 , 0, 0 }, // #40 [ref=33x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x04, 2 , 0, 0 }, // #41 [ref=30x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x04, 4 , 0, 0 }, // #42 [ref=40x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x00, 32, 0, 0 }, // #43 [ref=4x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x02, 8 , InstDB::RWInfoRm::kFlagAmbiguous, 0 }, // #44 [ref=1x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x02, 4 , InstDB::RWInfoRm::kFlagAmbiguous, 0 }, // #45 [ref=1x]
  { InstDB::RWInfoRm::kCategoryHalf      , 0x02, 0 , 0, 0 }, // #46 [ref=19x]
  { InstDB::RWInfoRm::kCategoryQuarter   , 0x02, 0 , 0, 0 }, // #47 [ref=9x]
  { InstDB::RWInfoRm::kCategoryHalf      , 0x01, 0 , 0, 0 }, // #48 [ref=10x]
  { InstDB::RWInfoRm::kCategoryConsistent, 0x04, 0 , InstDB::RWInfoRm::kFlagAmbiguous, 0 }, // #49 [ref=6x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x04, 16, 0, 0 }, // #50 [ref=30x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x01, 16, 0, 0 }, // #51 [ref=6x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x01, 32, 0, 0 }, // #52 [ref=4x]
  { InstDB::RWInfoRm::kCategoryConsistent, 0x0C, 0 , 0, 0 }, // #53 [ref=15x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x0C, 8 , 0, 0 }, // #54 [ref=4x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x0C, 4 , 0, 0 }, // #55 [ref=4x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x04, 32, 0, 0 }, // #56 [ref=6x]
  { InstDB::RWInfoRm::kCategoryConsistent, 0x03, 0 , 0, 0 }, // #57 [ref=13x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x03, 8 , InstDB::RWInfoRm::kFlagAmbiguous, 0 }, // #58 [ref=1x]
  { InstDB::RWInfoRm::kCategoryConsistent, 0x08, 0 , 0, 0 }, // #59 [ref=2x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x04, 1 , 0, 0 }, // #60 [ref=1x]
  { InstDB::RWInfoRm::kCategoryQuarter   , 0x01, 0 , 0, 0 }, // #61 [ref=6x]
  { InstDB::RWInfoRm::kCategoryEighth    , 0x01, 0 , 0, 0 }, // #62 [ref=3x]
  { InstDB::RWInfoRm::kCategoryEighth    , 0x02, 0 , 0, 0 }, // #63 [ref=2x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x0C, 16, 0, 0 }, // #64 [ref=1x]
  { InstDB::RWInfoRm::kCategoryFixed     , 0x06, 16, 0, 0 }, // #65 [ref=12x]
  { InstDB::RWInfoRm::kCategoryConsistent, 0x06, 0 , InstDB::RWInfoRm::kFlagFeatureIfRMI, uint32_t(CpuFeatures::X86::kAVX512_F) }, // #66 [ref=5x]
  { InstDB::RWInfoRm::kCategoryConsistent, 0x02, 0 , InstDB::RWInfoRm::kFlagFeatureIfRMI, uint32_t(CpuFeatures::X86::kAVX512_BW) }, // #67 [ref=2x]
  { InstDB::RWInfoRm::kCategoryConsistent, 0x06, 0 , InstDB::RWInfoRm::kFlagFeatureIfRMI, uint32_t(CpuFeatures::X86::kAVX512_BW) }  // #68 [ref=3x]
};
// ----------------------------------------------------------------------------
// ${InstRWInfoTable:End}

// x86::InstDB - Tests
// ===================

#if defined(ASMJIT_TEST)
UNIT(x86_inst_db) {
  INFO("Checking validity of Inst enums");

  // Cross-validate prefixes.
  EXPECT_EQ(uint32_t(InstOptions::kX86_Rex ), 0x40000000u)
    .message("REX prefix must be at 0x40000000");

  EXPECT_EQ(uint32_t(InstOptions::kX86_Evex), 0x00001000u)
    .message("EVEX prefix must be at 0x00001000");

  // These could be combined together to form a valid REX prefix, they must match.
  EXPECT_EQ(uint32_t(InstOptions::kX86_OpCodeB), uint32_t(Opcode::kB))
    .message("Opcode::kB must match InstOptions::kX86_OpCodeB");

  EXPECT_EQ(uint32_t(InstOptions::kX86_OpCodeX), uint32_t(Opcode::kX))
    .message("Opcode::kX must match InstOptions::kX86_OpCodeX");

  EXPECT_EQ(uint32_t(InstOptions::kX86_OpCodeR), uint32_t(Opcode::kR))
    .message("Opcode::kR must match InstOptions::kX86_OpCodeR");

  EXPECT_EQ(uint32_t(InstOptions::kX86_OpCodeW), uint32_t(Opcode::kW))
    .message("Opcode::kW must match InstOptions::kX86_OpCodeW");

  uint32_t rex_rb = (Opcode::kR >> Opcode::kREX_Shift) | (Opcode::kB >> Opcode::kREX_Shift) | 0x40;
  uint32_t rex_rw = (Opcode::kR >> Opcode::kREX_Shift) | (Opcode::kW >> Opcode::kREX_Shift) | 0x40;

  EXPECT_EQ(rex_rb, 0x45u)
    .message("Opcode::kR|B must form a valid REX prefix (0x45) if combined with 0x40");

  EXPECT_EQ(rex_rw, 0x4Cu)
    .message("Opcode::kR|W must form a valid REX prefix (0x4C) if combined with 0x40");
}
#endif

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_X86
