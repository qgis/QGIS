// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_ARM_A64GLOBALS_H_INCLUDED
#define ASMJIT_ARM_A64GLOBALS_H_INCLUDED

#include <asmjit/arm/armglobals.h>

//! \namespace asmjit::a64
//! \ingroup asmjit_a64
//!
//! AArch64 backend.

ASMJIT_BEGIN_SUB_NAMESPACE(a64)

//! \addtogroup asmjit_a64
//! \{

//! AArch64 instruction.
//!
//! \note Only used to hold ARM-specific enumerations and static functions.
namespace Inst {
  //! Instruction id.
  enum Id : uint32_t {
    // ${InstId:Begin}
    kIdNone = 0,                         //!< Instruction ''.
    kIdAbs,                              //!< Instruction 'abs'.
    kIdAdc,                              //!< Instruction 'adc'.
    kIdAdcs,                             //!< Instruction 'adcs'.
    kIdAdd,                              //!< Instruction 'add'.
    kIdAddg,                             //!< Instruction 'addg'.
    kIdAdds,                             //!< Instruction 'adds'.
    kIdAdr,                              //!< Instruction 'adr'.
    kIdAdrp,                             //!< Instruction 'adrp'.
    kIdAnd,                              //!< Instruction 'and'.
    kIdAnds,                             //!< Instruction 'ands'.
    kIdAsr,                              //!< Instruction 'asr'.
    kIdAsrv,                             //!< Instruction 'asrv'.
    kIdAt,                               //!< Instruction 'at'.
    kIdAutda,                            //!< Instruction 'autda'.
    kIdAutdza,                           //!< Instruction 'autdza'.
    kIdAutdb,                            //!< Instruction 'autdb'.
    kIdAutdzb,                           //!< Instruction 'autdzb'.
    kIdAutia,                            //!< Instruction 'autia'.
    kIdAutia1716,                        //!< Instruction 'autia1716'.
    kIdAutiasp,                          //!< Instruction 'autiasp'.
    kIdAutiaz,                           //!< Instruction 'autiaz'.
    kIdAutib,                            //!< Instruction 'autib'.
    kIdAutib1716,                        //!< Instruction 'autib1716'.
    kIdAutibsp,                          //!< Instruction 'autibsp'.
    kIdAutibz,                           //!< Instruction 'autibz'.
    kIdAutiza,                           //!< Instruction 'autiza'.
    kIdAutizb,                           //!< Instruction 'autizb'.
    kIdAxflag,                           //!< Instruction 'axflag'.
    kIdB,                                //!< Instruction 'b'.
    kIdBc,                               //!< Instruction 'bc'.
    kIdBfc,                              //!< Instruction 'bfc'.
    kIdBfi,                              //!< Instruction 'bfi'.
    kIdBfm,                              //!< Instruction 'bfm'.
    kIdBfxil,                            //!< Instruction 'bfxil'.
    kIdBic,                              //!< Instruction 'bic'.
    kIdBics,                             //!< Instruction 'bics'.
    kIdBl,                               //!< Instruction 'bl'.
    kIdBlr,                              //!< Instruction 'blr'.
    kIdBr,                               //!< Instruction 'br'.
    kIdBrk,                              //!< Instruction 'brk'.
    kIdBti,                              //!< Instruction 'bti'.
    kIdCas,                              //!< Instruction 'cas'.
    kIdCasa,                             //!< Instruction 'casa'.
    kIdCasab,                            //!< Instruction 'casab'.
    kIdCasah,                            //!< Instruction 'casah'.
    kIdCasal,                            //!< Instruction 'casal'.
    kIdCasalb,                           //!< Instruction 'casalb'.
    kIdCasalh,                           //!< Instruction 'casalh'.
    kIdCasb,                             //!< Instruction 'casb'.
    kIdCash,                             //!< Instruction 'cash'.
    kIdCasl,                             //!< Instruction 'casl'.
    kIdCaslb,                            //!< Instruction 'caslb'.
    kIdCaslh,                            //!< Instruction 'caslh'.
    kIdCasp,                             //!< Instruction 'casp'.
    kIdCaspa,                            //!< Instruction 'caspa'.
    kIdCaspal,                           //!< Instruction 'caspal'.
    kIdCaspl,                            //!< Instruction 'caspl'.
    kIdCbnz,                             //!< Instruction 'cbnz'.
    kIdCbz,                              //!< Instruction 'cbz'.
    kIdCcmn,                             //!< Instruction 'ccmn'.
    kIdCcmp,                             //!< Instruction 'ccmp'.
    kIdCfinv,                            //!< Instruction 'cfinv'.
    kIdChkfeat,                          //!< Instruction 'chkfeat'.
    kIdCinc,                             //!< Instruction 'cinc'.
    kIdCinv,                             //!< Instruction 'cinv'.
    kIdClrbhb,                           //!< Instruction 'clrbhb'.
    kIdClrex,                            //!< Instruction 'clrex'.
    kIdCls,                              //!< Instruction 'cls'.
    kIdClz,                              //!< Instruction 'clz'.
    kIdCmn,                              //!< Instruction 'cmn'.
    kIdCmp,                              //!< Instruction 'cmp'.
    kIdCmpp,                             //!< Instruction 'cmpp'.
    kIdCneg,                             //!< Instruction 'cneg'.
    kIdCnt,                              //!< Instruction 'cnt'.
    kIdCrc32b,                           //!< Instruction 'crc32b'.
    kIdCrc32cb,                          //!< Instruction 'crc32cb'.
    kIdCrc32ch,                          //!< Instruction 'crc32ch'.
    kIdCrc32cw,                          //!< Instruction 'crc32cw'.
    kIdCrc32cx,                          //!< Instruction 'crc32cx'.
    kIdCrc32h,                           //!< Instruction 'crc32h'.
    kIdCrc32w,                           //!< Instruction 'crc32w'.
    kIdCrc32x,                           //!< Instruction 'crc32x'.
    kIdCsdb,                             //!< Instruction 'csdb'.
    kIdCsel,                             //!< Instruction 'csel'.
    kIdCset,                             //!< Instruction 'cset'.
    kIdCsetm,                            //!< Instruction 'csetm'.
    kIdCsinc,                            //!< Instruction 'csinc'.
    kIdCsinv,                            //!< Instruction 'csinv'.
    kIdCsneg,                            //!< Instruction 'csneg'.
    kIdCtz,                              //!< Instruction 'ctz'.
    kIdDc,                               //!< Instruction 'dc'.
    kIdDcps1,                            //!< Instruction 'dcps1'.
    kIdDcps2,                            //!< Instruction 'dcps2'.
    kIdDcps3,                            //!< Instruction 'dcps3'.
    kIdDgh,                              //!< Instruction 'dgh'.
    kIdDmb,                              //!< Instruction 'dmb'.
    kIdDrps,                             //!< Instruction 'drps'.
    kIdDsb,                              //!< Instruction 'dsb'.
    kIdEon,                              //!< Instruction 'eon'.
    kIdEor,                              //!< Instruction 'eor'.
    kIdEsb,                              //!< Instruction 'esb'.
    kIdExtr,                             //!< Instruction 'extr'.
    kIdEret,                             //!< Instruction 'eret'.
    kIdGmi,                              //!< Instruction 'gmi'.
    kIdHint,                             //!< Instruction 'hint'.
    kIdHlt,                              //!< Instruction 'hlt'.
    kIdHvc,                              //!< Instruction 'hvc'.
    kIdIc,                               //!< Instruction 'ic'.
    kIdIsb,                              //!< Instruction 'isb'.
    kIdLdadd,                            //!< Instruction 'ldadd'.
    kIdLdadda,                           //!< Instruction 'ldadda'.
    kIdLdaddab,                          //!< Instruction 'ldaddab'.
    kIdLdaddah,                          //!< Instruction 'ldaddah'.
    kIdLdaddal,                          //!< Instruction 'ldaddal'.
    kIdLdaddalb,                         //!< Instruction 'ldaddalb'.
    kIdLdaddalh,                         //!< Instruction 'ldaddalh'.
    kIdLdaddb,                           //!< Instruction 'ldaddb'.
    kIdLdaddh,                           //!< Instruction 'ldaddh'.
    kIdLdaddl,                           //!< Instruction 'ldaddl'.
    kIdLdaddlb,                          //!< Instruction 'ldaddlb'.
    kIdLdaddlh,                          //!< Instruction 'ldaddlh'.
    kIdLdar,                             //!< Instruction 'ldar'.
    kIdLdarb,                            //!< Instruction 'ldarb'.
    kIdLdarh,                            //!< Instruction 'ldarh'.
    kIdLdaxp,                            //!< Instruction 'ldaxp'.
    kIdLdaxr,                            //!< Instruction 'ldaxr'.
    kIdLdaxrb,                           //!< Instruction 'ldaxrb'.
    kIdLdaxrh,                           //!< Instruction 'ldaxrh'.
    kIdLdclr,                            //!< Instruction 'ldclr'.
    kIdLdclra,                           //!< Instruction 'ldclra'.
    kIdLdclrab,                          //!< Instruction 'ldclrab'.
    kIdLdclrah,                          //!< Instruction 'ldclrah'.
    kIdLdclral,                          //!< Instruction 'ldclral'.
    kIdLdclralb,                         //!< Instruction 'ldclralb'.
    kIdLdclralh,                         //!< Instruction 'ldclralh'.
    kIdLdclrb,                           //!< Instruction 'ldclrb'.
    kIdLdclrh,                           //!< Instruction 'ldclrh'.
    kIdLdclrl,                           //!< Instruction 'ldclrl'.
    kIdLdclrlb,                          //!< Instruction 'ldclrlb'.
    kIdLdclrlh,                          //!< Instruction 'ldclrlh'.
    kIdLdeor,                            //!< Instruction 'ldeor'.
    kIdLdeora,                           //!< Instruction 'ldeora'.
    kIdLdeorab,                          //!< Instruction 'ldeorab'.
    kIdLdeorah,                          //!< Instruction 'ldeorah'.
    kIdLdeoral,                          //!< Instruction 'ldeoral'.
    kIdLdeoralb,                         //!< Instruction 'ldeoralb'.
    kIdLdeoralh,                         //!< Instruction 'ldeoralh'.
    kIdLdeorb,                           //!< Instruction 'ldeorb'.
    kIdLdeorh,                           //!< Instruction 'ldeorh'.
    kIdLdeorl,                           //!< Instruction 'ldeorl'.
    kIdLdeorlb,                          //!< Instruction 'ldeorlb'.
    kIdLdeorlh,                          //!< Instruction 'ldeorlh'.
    kIdLdg,                              //!< Instruction 'ldg'.
    kIdLdgm,                             //!< Instruction 'ldgm'.
    kIdLdlar,                            //!< Instruction 'ldlar'.
    kIdLdlarb,                           //!< Instruction 'ldlarb'.
    kIdLdlarh,                           //!< Instruction 'ldlarh'.
    kIdLdnp,                             //!< Instruction 'ldnp'.
    kIdLdp,                              //!< Instruction 'ldp'.
    kIdLdpsw,                            //!< Instruction 'ldpsw'.
    kIdLdr,                              //!< Instruction 'ldr'.
    kIdLdraa,                            //!< Instruction 'ldraa'.
    kIdLdrab,                            //!< Instruction 'ldrab'.
    kIdLdrb,                             //!< Instruction 'ldrb'.
    kIdLdrh,                             //!< Instruction 'ldrh'.
    kIdLdrsb,                            //!< Instruction 'ldrsb'.
    kIdLdrsh,                            //!< Instruction 'ldrsh'.
    kIdLdrsw,                            //!< Instruction 'ldrsw'.
    kIdLdset,                            //!< Instruction 'ldset'.
    kIdLdseta,                           //!< Instruction 'ldseta'.
    kIdLdsetab,                          //!< Instruction 'ldsetab'.
    kIdLdsetah,                          //!< Instruction 'ldsetah'.
    kIdLdsetal,                          //!< Instruction 'ldsetal'.
    kIdLdsetalb,                         //!< Instruction 'ldsetalb'.
    kIdLdsetalh,                         //!< Instruction 'ldsetalh'.
    kIdLdsetb,                           //!< Instruction 'ldsetb'.
    kIdLdseth,                           //!< Instruction 'ldseth'.
    kIdLdsetl,                           //!< Instruction 'ldsetl'.
    kIdLdsetlb,                          //!< Instruction 'ldsetlb'.
    kIdLdsetlh,                          //!< Instruction 'ldsetlh'.
    kIdLdsmax,                           //!< Instruction 'ldsmax'.
    kIdLdsmaxa,                          //!< Instruction 'ldsmaxa'.
    kIdLdsmaxab,                         //!< Instruction 'ldsmaxab'.
    kIdLdsmaxah,                         //!< Instruction 'ldsmaxah'.
    kIdLdsmaxal,                         //!< Instruction 'ldsmaxal'.
    kIdLdsmaxalb,                        //!< Instruction 'ldsmaxalb'.
    kIdLdsmaxalh,                        //!< Instruction 'ldsmaxalh'.
    kIdLdsmaxb,                          //!< Instruction 'ldsmaxb'.
    kIdLdsmaxh,                          //!< Instruction 'ldsmaxh'.
    kIdLdsmaxl,                          //!< Instruction 'ldsmaxl'.
    kIdLdsmaxlb,                         //!< Instruction 'ldsmaxlb'.
    kIdLdsmaxlh,                         //!< Instruction 'ldsmaxlh'.
    kIdLdsmin,                           //!< Instruction 'ldsmin'.
    kIdLdsmina,                          //!< Instruction 'ldsmina'.
    kIdLdsminab,                         //!< Instruction 'ldsminab'.
    kIdLdsminah,                         //!< Instruction 'ldsminah'.
    kIdLdsminal,                         //!< Instruction 'ldsminal'.
    kIdLdsminalb,                        //!< Instruction 'ldsminalb'.
    kIdLdsminalh,                        //!< Instruction 'ldsminalh'.
    kIdLdsminb,                          //!< Instruction 'ldsminb'.
    kIdLdsminh,                          //!< Instruction 'ldsminh'.
    kIdLdsminl,                          //!< Instruction 'ldsminl'.
    kIdLdsminlb,                         //!< Instruction 'ldsminlb'.
    kIdLdsminlh,                         //!< Instruction 'ldsminlh'.
    kIdLdtr,                             //!< Instruction 'ldtr'.
    kIdLdtrb,                            //!< Instruction 'ldtrb'.
    kIdLdtrh,                            //!< Instruction 'ldtrh'.
    kIdLdtrsb,                           //!< Instruction 'ldtrsb'.
    kIdLdtrsh,                           //!< Instruction 'ldtrsh'.
    kIdLdtrsw,                           //!< Instruction 'ldtrsw'.
    kIdLdumax,                           //!< Instruction 'ldumax'.
    kIdLdumaxa,                          //!< Instruction 'ldumaxa'.
    kIdLdumaxab,                         //!< Instruction 'ldumaxab'.
    kIdLdumaxah,                         //!< Instruction 'ldumaxah'.
    kIdLdumaxal,                         //!< Instruction 'ldumaxal'.
    kIdLdumaxalb,                        //!< Instruction 'ldumaxalb'.
    kIdLdumaxalh,                        //!< Instruction 'ldumaxalh'.
    kIdLdumaxb,                          //!< Instruction 'ldumaxb'.
    kIdLdumaxh,                          //!< Instruction 'ldumaxh'.
    kIdLdumaxl,                          //!< Instruction 'ldumaxl'.
    kIdLdumaxlb,                         //!< Instruction 'ldumaxlb'.
    kIdLdumaxlh,                         //!< Instruction 'ldumaxlh'.
    kIdLdumin,                           //!< Instruction 'ldumin'.
    kIdLdumina,                          //!< Instruction 'ldumina'.
    kIdLduminab,                         //!< Instruction 'lduminab'.
    kIdLduminah,                         //!< Instruction 'lduminah'.
    kIdLduminal,                         //!< Instruction 'lduminal'.
    kIdLduminalb,                        //!< Instruction 'lduminalb'.
    kIdLduminalh,                        //!< Instruction 'lduminalh'.
    kIdLduminb,                          //!< Instruction 'lduminb'.
    kIdLduminh,                          //!< Instruction 'lduminh'.
    kIdLduminl,                          //!< Instruction 'lduminl'.
    kIdLduminlb,                         //!< Instruction 'lduminlb'.
    kIdLduminlh,                         //!< Instruction 'lduminlh'.
    kIdLdur,                             //!< Instruction 'ldur'.
    kIdLdurb,                            //!< Instruction 'ldurb'.
    kIdLdurh,                            //!< Instruction 'ldurh'.
    kIdLdursb,                           //!< Instruction 'ldursb'.
    kIdLdursh,                           //!< Instruction 'ldursh'.
    kIdLdursw,                           //!< Instruction 'ldursw'.
    kIdLdxp,                             //!< Instruction 'ldxp'.
    kIdLdxr,                             //!< Instruction 'ldxr'.
    kIdLdxrb,                            //!< Instruction 'ldxrb'.
    kIdLdxrh,                            //!< Instruction 'ldxrh'.
    kIdLsl,                              //!< Instruction 'lsl'.
    kIdLslv,                             //!< Instruction 'lslv'.
    kIdLsr,                              //!< Instruction 'lsr'.
    kIdLsrv,                             //!< Instruction 'lsrv'.
    kIdMadd,                             //!< Instruction 'madd'.
    kIdMneg,                             //!< Instruction 'mneg'.
    kIdMov,                              //!< Instruction 'mov'.
    kIdMovk,                             //!< Instruction 'movk'.
    kIdMovn,                             //!< Instruction 'movn'.
    kIdMovz,                             //!< Instruction 'movz'.
    kIdMrs,                              //!< Instruction 'mrs'.
    kIdMsr,                              //!< Instruction 'msr'.
    kIdMsub,                             //!< Instruction 'msub'.
    kIdMul,                              //!< Instruction 'mul'.
    kIdMvn,                              //!< Instruction 'mvn'.
    kIdNeg,                              //!< Instruction 'neg'.
    kIdNegs,                             //!< Instruction 'negs'.
    kIdNgc,                              //!< Instruction 'ngc'.
    kIdNgcs,                             //!< Instruction 'ngcs'.
    kIdNop,                              //!< Instruction 'nop'.
    kIdOrn,                              //!< Instruction 'orn'.
    kIdOrr,                              //!< Instruction 'orr'.
    kIdPacda,                            //!< Instruction 'pacda'.
    kIdPacdb,                            //!< Instruction 'pacdb'.
    kIdPacdza,                           //!< Instruction 'pacdza'.
    kIdPacdzb,                           //!< Instruction 'pacdzb'.
    kIdPacga,                            //!< Instruction 'pacga'.
    kIdPrfm,                             //!< Instruction 'prfm'.
    kIdPssbb,                            //!< Instruction 'pssbb'.
    kIdRbit,                             //!< Instruction 'rbit'.
    kIdRet,                              //!< Instruction 'ret'.
    kIdRev,                              //!< Instruction 'rev'.
    kIdRev16,                            //!< Instruction 'rev16'.
    kIdRev32,                            //!< Instruction 'rev32'.
    kIdRev64,                            //!< Instruction 'rev64'.
    kIdRor,                              //!< Instruction 'ror'.
    kIdRorv,                             //!< Instruction 'rorv'.
    kIdSbc,                              //!< Instruction 'sbc'.
    kIdSbcs,                             //!< Instruction 'sbcs'.
    kIdSbfiz,                            //!< Instruction 'sbfiz'.
    kIdSbfm,                             //!< Instruction 'sbfm'.
    kIdSbfx,                             //!< Instruction 'sbfx'.
    kIdSdiv,                             //!< Instruction 'sdiv'.
    kIdSetf8,                            //!< Instruction 'setf8'.
    kIdSetf16,                           //!< Instruction 'setf16'.
    kIdSev,                              //!< Instruction 'sev'.
    kIdSevl,                             //!< Instruction 'sevl'.
    kIdSmaddl,                           //!< Instruction 'smaddl'.
    kIdSmax,                             //!< Instruction 'smax'.
    kIdSmc,                              //!< Instruction 'smc'.
    kIdSmin,                             //!< Instruction 'smin'.
    kIdSmnegl,                           //!< Instruction 'smnegl'.
    kIdSmsubl,                           //!< Instruction 'smsubl'.
    kIdSmulh,                            //!< Instruction 'smulh'.
    kIdSmull,                            //!< Instruction 'smull'.
    kIdSsbb,                             //!< Instruction 'ssbb'.
    kIdSt2g,                             //!< Instruction 'st2g'.
    kIdStadd,                            //!< Instruction 'stadd'.
    kIdStaddl,                           //!< Instruction 'staddl'.
    kIdStaddb,                           //!< Instruction 'staddb'.
    kIdStaddlb,                          //!< Instruction 'staddlb'.
    kIdStaddh,                           //!< Instruction 'staddh'.
    kIdStaddlh,                          //!< Instruction 'staddlh'.
    kIdStclr,                            //!< Instruction 'stclr'.
    kIdStclrl,                           //!< Instruction 'stclrl'.
    kIdStclrb,                           //!< Instruction 'stclrb'.
    kIdStclrlb,                          //!< Instruction 'stclrlb'.
    kIdStclrh,                           //!< Instruction 'stclrh'.
    kIdStclrlh,                          //!< Instruction 'stclrlh'.
    kIdSteor,                            //!< Instruction 'steor'.
    kIdSteorl,                           //!< Instruction 'steorl'.
    kIdSteorb,                           //!< Instruction 'steorb'.
    kIdSteorlb,                          //!< Instruction 'steorlb'.
    kIdSteorh,                           //!< Instruction 'steorh'.
    kIdSteorlh,                          //!< Instruction 'steorlh'.
    kIdStg,                              //!< Instruction 'stg'.
    kIdStgm,                             //!< Instruction 'stgm'.
    kIdStgp,                             //!< Instruction 'stgp'.
    kIdStllr,                            //!< Instruction 'stllr'.
    kIdStllrb,                           //!< Instruction 'stllrb'.
    kIdStllrh,                           //!< Instruction 'stllrh'.
    kIdStlr,                             //!< Instruction 'stlr'.
    kIdStlrb,                            //!< Instruction 'stlrb'.
    kIdStlrh,                            //!< Instruction 'stlrh'.
    kIdStlxp,                            //!< Instruction 'stlxp'.
    kIdStlxr,                            //!< Instruction 'stlxr'.
    kIdStlxrb,                           //!< Instruction 'stlxrb'.
    kIdStlxrh,                           //!< Instruction 'stlxrh'.
    kIdStnp,                             //!< Instruction 'stnp'.
    kIdStp,                              //!< Instruction 'stp'.
    kIdStr,                              //!< Instruction 'str'.
    kIdStrb,                             //!< Instruction 'strb'.
    kIdStrh,                             //!< Instruction 'strh'.
    kIdStset,                            //!< Instruction 'stset'.
    kIdStsetl,                           //!< Instruction 'stsetl'.
    kIdStsetb,                           //!< Instruction 'stsetb'.
    kIdStsetlb,                          //!< Instruction 'stsetlb'.
    kIdStseth,                           //!< Instruction 'stseth'.
    kIdStsetlh,                          //!< Instruction 'stsetlh'.
    kIdStsmax,                           //!< Instruction 'stsmax'.
    kIdStsmaxl,                          //!< Instruction 'stsmaxl'.
    kIdStsmaxb,                          //!< Instruction 'stsmaxb'.
    kIdStsmaxlb,                         //!< Instruction 'stsmaxlb'.
    kIdStsmaxh,                          //!< Instruction 'stsmaxh'.
    kIdStsmaxlh,                         //!< Instruction 'stsmaxlh'.
    kIdStsmin,                           //!< Instruction 'stsmin'.
    kIdStsminl,                          //!< Instruction 'stsminl'.
    kIdStsminb,                          //!< Instruction 'stsminb'.
    kIdStsminlb,                         //!< Instruction 'stsminlb'.
    kIdStsminh,                          //!< Instruction 'stsminh'.
    kIdStsminlh,                         //!< Instruction 'stsminlh'.
    kIdSttr,                             //!< Instruction 'sttr'.
    kIdSttrb,                            //!< Instruction 'sttrb'.
    kIdSttrh,                            //!< Instruction 'sttrh'.
    kIdStumax,                           //!< Instruction 'stumax'.
    kIdStumaxl,                          //!< Instruction 'stumaxl'.
    kIdStumaxb,                          //!< Instruction 'stumaxb'.
    kIdStumaxlb,                         //!< Instruction 'stumaxlb'.
    kIdStumaxh,                          //!< Instruction 'stumaxh'.
    kIdStumaxlh,                         //!< Instruction 'stumaxlh'.
    kIdStumin,                           //!< Instruction 'stumin'.
    kIdStuminl,                          //!< Instruction 'stuminl'.
    kIdStuminb,                          //!< Instruction 'stuminb'.
    kIdStuminlb,                         //!< Instruction 'stuminlb'.
    kIdStuminh,                          //!< Instruction 'stuminh'.
    kIdStuminlh,                         //!< Instruction 'stuminlh'.
    kIdStur,                             //!< Instruction 'stur'.
    kIdSturb,                            //!< Instruction 'sturb'.
    kIdSturh,                            //!< Instruction 'sturh'.
    kIdStxp,                             //!< Instruction 'stxp'.
    kIdStxr,                             //!< Instruction 'stxr'.
    kIdStxrb,                            //!< Instruction 'stxrb'.
    kIdStxrh,                            //!< Instruction 'stxrh'.
    kIdStz2g,                            //!< Instruction 'stz2g'.
    kIdStzg,                             //!< Instruction 'stzg'.
    kIdStzgm,                            //!< Instruction 'stzgm'.
    kIdSub,                              //!< Instruction 'sub'.
    kIdSubg,                             //!< Instruction 'subg'.
    kIdSubp,                             //!< Instruction 'subp'.
    kIdSubps,                            //!< Instruction 'subps'.
    kIdSubs,                             //!< Instruction 'subs'.
    kIdSvc,                              //!< Instruction 'svc'.
    kIdSwp,                              //!< Instruction 'swp'.
    kIdSwpa,                             //!< Instruction 'swpa'.
    kIdSwpab,                            //!< Instruction 'swpab'.
    kIdSwpah,                            //!< Instruction 'swpah'.
    kIdSwpal,                            //!< Instruction 'swpal'.
    kIdSwpalb,                           //!< Instruction 'swpalb'.
    kIdSwpalh,                           //!< Instruction 'swpalh'.
    kIdSwpb,                             //!< Instruction 'swpb'.
    kIdSwph,                             //!< Instruction 'swph'.
    kIdSwpl,                             //!< Instruction 'swpl'.
    kIdSwplb,                            //!< Instruction 'swplb'.
    kIdSwplh,                            //!< Instruction 'swplh'.
    kIdSxtb,                             //!< Instruction 'sxtb'.
    kIdSxth,                             //!< Instruction 'sxth'.
    kIdSxtw,                             //!< Instruction 'sxtw'.
    kIdSys,                              //!< Instruction 'sys'.
    kIdTlbi,                             //!< Instruction 'tlbi'.
    kIdTst,                              //!< Instruction 'tst'.
    kIdTbnz,                             //!< Instruction 'tbnz'.
    kIdTbz,                              //!< Instruction 'tbz'.
    kIdUbfiz,                            //!< Instruction 'ubfiz'.
    kIdUbfm,                             //!< Instruction 'ubfm'.
    kIdUbfx,                             //!< Instruction 'ubfx'.
    kIdUdf,                              //!< Instruction 'udf'.
    kIdUdiv,                             //!< Instruction 'udiv'.
    kIdUmaddl,                           //!< Instruction 'umaddl'.
    kIdUmax,                             //!< Instruction 'umax'.
    kIdUmin,                             //!< Instruction 'umin'.
    kIdUmnegl,                           //!< Instruction 'umnegl'.
    kIdUmull,                            //!< Instruction 'umull'.
    kIdUmulh,                            //!< Instruction 'umulh'.
    kIdUmsubl,                           //!< Instruction 'umsubl'.
    kIdUxtb,                             //!< Instruction 'uxtb'.
    kIdUxth,                             //!< Instruction 'uxth'.
    kIdWfe,                              //!< Instruction 'wfe'.
    kIdWfi,                              //!< Instruction 'wfi'.
    kIdXaflag,                           //!< Instruction 'xaflag'.
    kIdXpacd,                            //!< Instruction 'xpacd'.
    kIdXpaci,                            //!< Instruction 'xpaci'.
    kIdXpaclri,                          //!< Instruction 'xpaclri'.
    kIdYield,                            //!< Instruction 'yield'.
    kIdAbs_v,                            //!< Instruction 'abs' {ASIMD}.
    kIdAdd_v,                            //!< Instruction 'add' {ASIMD}.
    kIdAddhn_v,                          //!< Instruction 'addhn' {ASIMD}.
    kIdAddhn2_v,                         //!< Instruction 'addhn2' {ASIMD}.
    kIdAddp_v,                           //!< Instruction 'addp' {ASIMD}.
    kIdAddv_v,                           //!< Instruction 'addv' {ASIMD}.
    kIdAesd_v,                           //!< Instruction 'aesd' {ASIMD}.
    kIdAese_v,                           //!< Instruction 'aese' {ASIMD}.
    kIdAesimc_v,                         //!< Instruction 'aesimc' {ASIMD}.
    kIdAesmc_v,                          //!< Instruction 'aesmc' {ASIMD}.
    kIdAnd_v,                            //!< Instruction 'and' {ASIMD}.
    kIdBcax_v,                           //!< Instruction 'bcax' {ASIMD}.
    kIdBfcvt_v,                          //!< Instruction 'bfcvt' {ASIMD}.
    kIdBfcvtn_v,                         //!< Instruction 'bfcvtn' {ASIMD}.
    kIdBfcvtn2_v,                        //!< Instruction 'bfcvtn2' {ASIMD}.
    kIdBfdot_v,                          //!< Instruction 'bfdot' {ASIMD}.
    kIdBfmlalb_v,                        //!< Instruction 'bfmlalb' {ASIMD}.
    kIdBfmlalt_v,                        //!< Instruction 'bfmlalt' {ASIMD}.
    kIdBfmmla_v,                         //!< Instruction 'bfmmla' {ASIMD}.
    kIdBic_v,                            //!< Instruction 'bic' {ASIMD}.
    kIdBif_v,                            //!< Instruction 'bif' {ASIMD}.
    kIdBit_v,                            //!< Instruction 'bit' {ASIMD}.
    kIdBsl_v,                            //!< Instruction 'bsl' {ASIMD}.
    kIdCls_v,                            //!< Instruction 'cls' {ASIMD}.
    kIdClz_v,                            //!< Instruction 'clz' {ASIMD}.
    kIdCmeq_v,                           //!< Instruction 'cmeq' {ASIMD}.
    kIdCmge_v,                           //!< Instruction 'cmge' {ASIMD}.
    kIdCmgt_v,                           //!< Instruction 'cmgt' {ASIMD}.
    kIdCmhi_v,                           //!< Instruction 'cmhi' {ASIMD}.
    kIdCmhs_v,                           //!< Instruction 'cmhs' {ASIMD}.
    kIdCmle_v,                           //!< Instruction 'cmle' {ASIMD}.
    kIdCmlt_v,                           //!< Instruction 'cmlt' {ASIMD}.
    kIdCmtst_v,                          //!< Instruction 'cmtst' {ASIMD}.
    kIdCnt_v,                            //!< Instruction 'cnt' {ASIMD}.
    kIdDup_v,                            //!< Instruction 'dup' {ASIMD}.
    kIdEor_v,                            //!< Instruction 'eor' {ASIMD}.
    kIdEor3_v,                           //!< Instruction 'eor3' {ASIMD}.
    kIdExt_v,                            //!< Instruction 'ext' {ASIMD}.
    kIdFabd_v,                           //!< Instruction 'fabd' {ASIMD}.
    kIdFabs_v,                           //!< Instruction 'fabs' {ASIMD}.
    kIdFacge_v,                          //!< Instruction 'facge' {ASIMD}.
    kIdFacgt_v,                          //!< Instruction 'facgt' {ASIMD}.
    kIdFadd_v,                           //!< Instruction 'fadd' {ASIMD}.
    kIdFaddp_v,                          //!< Instruction 'faddp' {ASIMD}.
    kIdFcadd_v,                          //!< Instruction 'fcadd' {ASIMD}.
    kIdFccmp_v,                          //!< Instruction 'fccmp' {ASIMD}.
    kIdFccmpe_v,                         //!< Instruction 'fccmpe' {ASIMD}.
    kIdFcmeq_v,                          //!< Instruction 'fcmeq' {ASIMD}.
    kIdFcmge_v,                          //!< Instruction 'fcmge' {ASIMD}.
    kIdFcmgt_v,                          //!< Instruction 'fcmgt' {ASIMD}.
    kIdFcmla_v,                          //!< Instruction 'fcmla' {ASIMD}.
    kIdFcmle_v,                          //!< Instruction 'fcmle' {ASIMD}.
    kIdFcmlt_v,                          //!< Instruction 'fcmlt' {ASIMD}.
    kIdFcmp_v,                           //!< Instruction 'fcmp' {ASIMD}.
    kIdFcmpe_v,                          //!< Instruction 'fcmpe' {ASIMD}.
    kIdFcsel_v,                          //!< Instruction 'fcsel' {ASIMD}.
    kIdFcvt_v,                           //!< Instruction 'fcvt' {ASIMD}.
    kIdFcvtas_v,                         //!< Instruction 'fcvtas' {ASIMD}.
    kIdFcvtau_v,                         //!< Instruction 'fcvtau' {ASIMD}.
    kIdFcvtl_v,                          //!< Instruction 'fcvtl' {ASIMD}.
    kIdFcvtl2_v,                         //!< Instruction 'fcvtl2' {ASIMD}.
    kIdFcvtms_v,                         //!< Instruction 'fcvtms' {ASIMD}.
    kIdFcvtmu_v,                         //!< Instruction 'fcvtmu' {ASIMD}.
    kIdFcvtn_v,                          //!< Instruction 'fcvtn' {ASIMD}.
    kIdFcvtn2_v,                         //!< Instruction 'fcvtn2' {ASIMD}.
    kIdFcvtns_v,                         //!< Instruction 'fcvtns' {ASIMD}.
    kIdFcvtnu_v,                         //!< Instruction 'fcvtnu' {ASIMD}.
    kIdFcvtps_v,                         //!< Instruction 'fcvtps' {ASIMD}.
    kIdFcvtpu_v,                         //!< Instruction 'fcvtpu' {ASIMD}.
    kIdFcvtxn_v,                         //!< Instruction 'fcvtxn' {ASIMD}.
    kIdFcvtxn2_v,                        //!< Instruction 'fcvtxn2' {ASIMD}.
    kIdFcvtzs_v,                         //!< Instruction 'fcvtzs' {ASIMD}.
    kIdFcvtzu_v,                         //!< Instruction 'fcvtzu' {ASIMD}.
    kIdFdiv_v,                           //!< Instruction 'fdiv' {ASIMD}.
    kIdFjcvtzs_v,                        //!< Instruction 'fjcvtzs' {ASIMD}.
    kIdFmadd_v,                          //!< Instruction 'fmadd' {ASIMD}.
    kIdFmax_v,                           //!< Instruction 'fmax' {ASIMD}.
    kIdFmaxnm_v,                         //!< Instruction 'fmaxnm' {ASIMD}.
    kIdFmaxnmp_v,                        //!< Instruction 'fmaxnmp' {ASIMD}.
    kIdFmaxnmv_v,                        //!< Instruction 'fmaxnmv' {ASIMD}.
    kIdFmaxp_v,                          //!< Instruction 'fmaxp' {ASIMD}.
    kIdFmaxv_v,                          //!< Instruction 'fmaxv' {ASIMD}.
    kIdFmin_v,                           //!< Instruction 'fmin' {ASIMD}.
    kIdFminnm_v,                         //!< Instruction 'fminnm' {ASIMD}.
    kIdFminnmp_v,                        //!< Instruction 'fminnmp' {ASIMD}.
    kIdFminnmv_v,                        //!< Instruction 'fminnmv' {ASIMD}.
    kIdFminp_v,                          //!< Instruction 'fminp' {ASIMD}.
    kIdFminv_v,                          //!< Instruction 'fminv' {ASIMD}.
    kIdFmla_v,                           //!< Instruction 'fmla' {ASIMD}.
    kIdFmlal_v,                          //!< Instruction 'fmlal' {ASIMD}.
    kIdFmlal2_v,                         //!< Instruction 'fmlal2' {ASIMD}.
    kIdFmls_v,                           //!< Instruction 'fmls' {ASIMD}.
    kIdFmlsl_v,                          //!< Instruction 'fmlsl' {ASIMD}.
    kIdFmlsl2_v,                         //!< Instruction 'fmlsl2' {ASIMD}.
    kIdFmov_v,                           //!< Instruction 'fmov' {ASIMD}.
    kIdFmsub_v,                          //!< Instruction 'fmsub' {ASIMD}.
    kIdFmul_v,                           //!< Instruction 'fmul' {ASIMD}.
    kIdFmulx_v,                          //!< Instruction 'fmulx' {ASIMD}.
    kIdFneg_v,                           //!< Instruction 'fneg' {ASIMD}.
    kIdFnmadd_v,                         //!< Instruction 'fnmadd' {ASIMD}.
    kIdFnmsub_v,                         //!< Instruction 'fnmsub' {ASIMD}.
    kIdFnmul_v,                          //!< Instruction 'fnmul' {ASIMD}.
    kIdFrecpe_v,                         //!< Instruction 'frecpe' {ASIMD}.
    kIdFrecps_v,                         //!< Instruction 'frecps' {ASIMD}.
    kIdFrecpx_v,                         //!< Instruction 'frecpx' {ASIMD}.
    kIdFrint32x_v,                       //!< Instruction 'frint32x' {ASIMD}.
    kIdFrint32z_v,                       //!< Instruction 'frint32z' {ASIMD}.
    kIdFrint64x_v,                       //!< Instruction 'frint64x' {ASIMD}.
    kIdFrint64z_v,                       //!< Instruction 'frint64z' {ASIMD}.
    kIdFrinta_v,                         //!< Instruction 'frinta' {ASIMD}.
    kIdFrinti_v,                         //!< Instruction 'frinti' {ASIMD}.
    kIdFrintm_v,                         //!< Instruction 'frintm' {ASIMD}.
    kIdFrintn_v,                         //!< Instruction 'frintn' {ASIMD}.
    kIdFrintp_v,                         //!< Instruction 'frintp' {ASIMD}.
    kIdFrintx_v,                         //!< Instruction 'frintx' {ASIMD}.
    kIdFrintz_v,                         //!< Instruction 'frintz' {ASIMD}.
    kIdFrsqrte_v,                        //!< Instruction 'frsqrte' {ASIMD}.
    kIdFrsqrts_v,                        //!< Instruction 'frsqrts' {ASIMD}.
    kIdFsqrt_v,                          //!< Instruction 'fsqrt' {ASIMD}.
    kIdFsub_v,                           //!< Instruction 'fsub' {ASIMD}.
    kIdIns_v,                            //!< Instruction 'ins' {ASIMD}.
    kIdLd1_v,                            //!< Instruction 'ld1' {ASIMD}.
    kIdLd1r_v,                           //!< Instruction 'ld1r' {ASIMD}.
    kIdLd2_v,                            //!< Instruction 'ld2' {ASIMD}.
    kIdLd2r_v,                           //!< Instruction 'ld2r' {ASIMD}.
    kIdLd3_v,                            //!< Instruction 'ld3' {ASIMD}.
    kIdLd3r_v,                           //!< Instruction 'ld3r' {ASIMD}.
    kIdLd4_v,                            //!< Instruction 'ld4' {ASIMD}.
    kIdLd4r_v,                           //!< Instruction 'ld4r' {ASIMD}.
    kIdLdnp_v,                           //!< Instruction 'ldnp' {ASIMD}.
    kIdLdp_v,                            //!< Instruction 'ldp' {ASIMD}.
    kIdLdr_v,                            //!< Instruction 'ldr' {ASIMD}.
    kIdLdur_v,                           //!< Instruction 'ldur' {ASIMD}.
    kIdMla_v,                            //!< Instruction 'mla' {ASIMD}.
    kIdMls_v,                            //!< Instruction 'mls' {ASIMD}.
    kIdMov_v,                            //!< Instruction 'mov' {ASIMD}.
    kIdMovi_v,                           //!< Instruction 'movi' {ASIMD}.
    kIdMul_v,                            //!< Instruction 'mul' {ASIMD}.
    kIdMvn_v,                            //!< Instruction 'mvn' {ASIMD}.
    kIdMvni_v,                           //!< Instruction 'mvni' {ASIMD}.
    kIdNeg_v,                            //!< Instruction 'neg' {ASIMD}.
    kIdNot_v,                            //!< Instruction 'not' {ASIMD}.
    kIdOrn_v,                            //!< Instruction 'orn' {ASIMD}.
    kIdOrr_v,                            //!< Instruction 'orr' {ASIMD}.
    kIdPmul_v,                           //!< Instruction 'pmul' {ASIMD}.
    kIdPmull_v,                          //!< Instruction 'pmull' {ASIMD}.
    kIdPmull2_v,                         //!< Instruction 'pmull2' {ASIMD}.
    kIdRaddhn_v,                         //!< Instruction 'raddhn' {ASIMD}.
    kIdRaddhn2_v,                        //!< Instruction 'raddhn2' {ASIMD}.
    kIdRax1_v,                           //!< Instruction 'rax1' {ASIMD}.
    kIdRbit_v,                           //!< Instruction 'rbit' {ASIMD}.
    kIdRev16_v,                          //!< Instruction 'rev16' {ASIMD}.
    kIdRev32_v,                          //!< Instruction 'rev32' {ASIMD}.
    kIdRev64_v,                          //!< Instruction 'rev64' {ASIMD}.
    kIdRshrn_v,                          //!< Instruction 'rshrn' {ASIMD}.
    kIdRshrn2_v,                         //!< Instruction 'rshrn2' {ASIMD}.
    kIdRsubhn_v,                         //!< Instruction 'rsubhn' {ASIMD}.
    kIdRsubhn2_v,                        //!< Instruction 'rsubhn2' {ASIMD}.
    kIdSaba_v,                           //!< Instruction 'saba' {ASIMD}.
    kIdSabal_v,                          //!< Instruction 'sabal' {ASIMD}.
    kIdSabal2_v,                         //!< Instruction 'sabal2' {ASIMD}.
    kIdSabd_v,                           //!< Instruction 'sabd' {ASIMD}.
    kIdSabdl_v,                          //!< Instruction 'sabdl' {ASIMD}.
    kIdSabdl2_v,                         //!< Instruction 'sabdl2' {ASIMD}.
    kIdSadalp_v,                         //!< Instruction 'sadalp' {ASIMD}.
    kIdSaddl_v,                          //!< Instruction 'saddl' {ASIMD}.
    kIdSaddl2_v,                         //!< Instruction 'saddl2' {ASIMD}.
    kIdSaddlp_v,                         //!< Instruction 'saddlp' {ASIMD}.
    kIdSaddlv_v,                         //!< Instruction 'saddlv' {ASIMD}.
    kIdSaddw_v,                          //!< Instruction 'saddw' {ASIMD}.
    kIdSaddw2_v,                         //!< Instruction 'saddw2' {ASIMD}.
    kIdScvtf_v,                          //!< Instruction 'scvtf' {ASIMD}.
    kIdSdot_v,                           //!< Instruction 'sdot' {ASIMD}.
    kIdSha1c_v,                          //!< Instruction 'sha1c' {ASIMD}.
    kIdSha1h_v,                          //!< Instruction 'sha1h' {ASIMD}.
    kIdSha1m_v,                          //!< Instruction 'sha1m' {ASIMD}.
    kIdSha1p_v,                          //!< Instruction 'sha1p' {ASIMD}.
    kIdSha1su0_v,                        //!< Instruction 'sha1su0' {ASIMD}.
    kIdSha1su1_v,                        //!< Instruction 'sha1su1' {ASIMD}.
    kIdSha256h_v,                        //!< Instruction 'sha256h' {ASIMD}.
    kIdSha256h2_v,                       //!< Instruction 'sha256h2' {ASIMD}.
    kIdSha256su0_v,                      //!< Instruction 'sha256su0' {ASIMD}.
    kIdSha256su1_v,                      //!< Instruction 'sha256su1' {ASIMD}.
    kIdSha512h_v,                        //!< Instruction 'sha512h' {ASIMD}.
    kIdSha512h2_v,                       //!< Instruction 'sha512h2' {ASIMD}.
    kIdSha512su0_v,                      //!< Instruction 'sha512su0' {ASIMD}.
    kIdSha512su1_v,                      //!< Instruction 'sha512su1' {ASIMD}.
    kIdShadd_v,                          //!< Instruction 'shadd' {ASIMD}.
    kIdShl_v,                            //!< Instruction 'shl' {ASIMD}.
    kIdShll_v,                           //!< Instruction 'shll' {ASIMD}.
    kIdShll2_v,                          //!< Instruction 'shll2' {ASIMD}.
    kIdShrn_v,                           //!< Instruction 'shrn' {ASIMD}.
    kIdShrn2_v,                          //!< Instruction 'shrn2' {ASIMD}.
    kIdShsub_v,                          //!< Instruction 'shsub' {ASIMD}.
    kIdSli_v,                            //!< Instruction 'sli' {ASIMD}.
    kIdSm3partw1_v,                      //!< Instruction 'sm3partw1' {ASIMD}.
    kIdSm3partw2_v,                      //!< Instruction 'sm3partw2' {ASIMD}.
    kIdSm3ss1_v,                         //!< Instruction 'sm3ss1' {ASIMD}.
    kIdSm3tt1a_v,                        //!< Instruction 'sm3tt1a' {ASIMD}.
    kIdSm3tt1b_v,                        //!< Instruction 'sm3tt1b' {ASIMD}.
    kIdSm3tt2a_v,                        //!< Instruction 'sm3tt2a' {ASIMD}.
    kIdSm3tt2b_v,                        //!< Instruction 'sm3tt2b' {ASIMD}.
    kIdSm4e_v,                           //!< Instruction 'sm4e' {ASIMD}.
    kIdSm4ekey_v,                        //!< Instruction 'sm4ekey' {ASIMD}.
    kIdSmax_v,                           //!< Instruction 'smax' {ASIMD}.
    kIdSmaxp_v,                          //!< Instruction 'smaxp' {ASIMD}.
    kIdSmaxv_v,                          //!< Instruction 'smaxv' {ASIMD}.
    kIdSmin_v,                           //!< Instruction 'smin' {ASIMD}.
    kIdSminp_v,                          //!< Instruction 'sminp' {ASIMD}.
    kIdSminv_v,                          //!< Instruction 'sminv' {ASIMD}.
    kIdSmlal_v,                          //!< Instruction 'smlal' {ASIMD}.
    kIdSmlal2_v,                         //!< Instruction 'smlal2' {ASIMD}.
    kIdSmlsl_v,                          //!< Instruction 'smlsl' {ASIMD}.
    kIdSmlsl2_v,                         //!< Instruction 'smlsl2' {ASIMD}.
    kIdSmmla_v,                          //!< Instruction 'smmla' {ASIMD}.
    kIdSmov_v,                           //!< Instruction 'smov' {ASIMD}.
    kIdSmull_v,                          //!< Instruction 'smull' {ASIMD}.
    kIdSmull2_v,                         //!< Instruction 'smull2' {ASIMD}.
    kIdSqabs_v,                          //!< Instruction 'sqabs' {ASIMD}.
    kIdSqadd_v,                          //!< Instruction 'sqadd' {ASIMD}.
    kIdSqdmlal_v,                        //!< Instruction 'sqdmlal' {ASIMD}.
    kIdSqdmlal2_v,                       //!< Instruction 'sqdmlal2' {ASIMD}.
    kIdSqdmlsl_v,                        //!< Instruction 'sqdmlsl' {ASIMD}.
    kIdSqdmlsl2_v,                       //!< Instruction 'sqdmlsl2' {ASIMD}.
    kIdSqdmulh_v,                        //!< Instruction 'sqdmulh' {ASIMD}.
    kIdSqdmull_v,                        //!< Instruction 'sqdmull' {ASIMD}.
    kIdSqdmull2_v,                       //!< Instruction 'sqdmull2' {ASIMD}.
    kIdSqneg_v,                          //!< Instruction 'sqneg' {ASIMD}.
    kIdSqrdmlah_v,                       //!< Instruction 'sqrdmlah' {ASIMD}.
    kIdSqrdmlsh_v,                       //!< Instruction 'sqrdmlsh' {ASIMD}.
    kIdSqrdmulh_v,                       //!< Instruction 'sqrdmulh' {ASIMD}.
    kIdSqrshl_v,                         //!< Instruction 'sqrshl' {ASIMD}.
    kIdSqrshrn_v,                        //!< Instruction 'sqrshrn' {ASIMD}.
    kIdSqrshrn2_v,                       //!< Instruction 'sqrshrn2' {ASIMD}.
    kIdSqrshrun_v,                       //!< Instruction 'sqrshrun' {ASIMD}.
    kIdSqrshrun2_v,                      //!< Instruction 'sqrshrun2' {ASIMD}.
    kIdSqshl_v,                          //!< Instruction 'sqshl' {ASIMD}.
    kIdSqshlu_v,                         //!< Instruction 'sqshlu' {ASIMD}.
    kIdSqshrn_v,                         //!< Instruction 'sqshrn' {ASIMD}.
    kIdSqshrn2_v,                        //!< Instruction 'sqshrn2' {ASIMD}.
    kIdSqshrun_v,                        //!< Instruction 'sqshrun' {ASIMD}.
    kIdSqshrun2_v,                       //!< Instruction 'sqshrun2' {ASIMD}.
    kIdSqsub_v,                          //!< Instruction 'sqsub' {ASIMD}.
    kIdSqxtn_v,                          //!< Instruction 'sqxtn' {ASIMD}.
    kIdSqxtn2_v,                         //!< Instruction 'sqxtn2' {ASIMD}.
    kIdSqxtun_v,                         //!< Instruction 'sqxtun' {ASIMD}.
    kIdSqxtun2_v,                        //!< Instruction 'sqxtun2' {ASIMD}.
    kIdSrhadd_v,                         //!< Instruction 'srhadd' {ASIMD}.
    kIdSri_v,                            //!< Instruction 'sri' {ASIMD}.
    kIdSrshl_v,                          //!< Instruction 'srshl' {ASIMD}.
    kIdSrshr_v,                          //!< Instruction 'srshr' {ASIMD}.
    kIdSrsra_v,                          //!< Instruction 'srsra' {ASIMD}.
    kIdSshl_v,                           //!< Instruction 'sshl' {ASIMD}.
    kIdSshll_v,                          //!< Instruction 'sshll' {ASIMD}.
    kIdSshll2_v,                         //!< Instruction 'sshll2' {ASIMD}.
    kIdSshr_v,                           //!< Instruction 'sshr' {ASIMD}.
    kIdSsra_v,                           //!< Instruction 'ssra' {ASIMD}.
    kIdSsubl_v,                          //!< Instruction 'ssubl' {ASIMD}.
    kIdSsubl2_v,                         //!< Instruction 'ssubl2' {ASIMD}.
    kIdSsubw_v,                          //!< Instruction 'ssubw' {ASIMD}.
    kIdSsubw2_v,                         //!< Instruction 'ssubw2' {ASIMD}.
    kIdSt1_v,                            //!< Instruction 'st1' {ASIMD}.
    kIdSt2_v,                            //!< Instruction 'st2' {ASIMD}.
    kIdSt3_v,                            //!< Instruction 'st3' {ASIMD}.
    kIdSt4_v,                            //!< Instruction 'st4' {ASIMD}.
    kIdStnp_v,                           //!< Instruction 'stnp' {ASIMD}.
    kIdStp_v,                            //!< Instruction 'stp' {ASIMD}.
    kIdStr_v,                            //!< Instruction 'str' {ASIMD}.
    kIdStur_v,                           //!< Instruction 'stur' {ASIMD}.
    kIdSub_v,                            //!< Instruction 'sub' {ASIMD}.
    kIdSubhn_v,                          //!< Instruction 'subhn' {ASIMD}.
    kIdSubhn2_v,                         //!< Instruction 'subhn2' {ASIMD}.
    kIdSudot_v,                          //!< Instruction 'sudot' {ASIMD}.
    kIdSuqadd_v,                         //!< Instruction 'suqadd' {ASIMD}.
    kIdSxtl_v,                           //!< Instruction 'sxtl' {ASIMD}.
    kIdSxtl2_v,                          //!< Instruction 'sxtl2' {ASIMD}.
    kIdTbl_v,                            //!< Instruction 'tbl' {ASIMD}.
    kIdTbx_v,                            //!< Instruction 'tbx' {ASIMD}.
    kIdTrn1_v,                           //!< Instruction 'trn1' {ASIMD}.
    kIdTrn2_v,                           //!< Instruction 'trn2' {ASIMD}.
    kIdUaba_v,                           //!< Instruction 'uaba' {ASIMD}.
    kIdUabal_v,                          //!< Instruction 'uabal' {ASIMD}.
    kIdUabal2_v,                         //!< Instruction 'uabal2' {ASIMD}.
    kIdUabd_v,                           //!< Instruction 'uabd' {ASIMD}.
    kIdUabdl_v,                          //!< Instruction 'uabdl' {ASIMD}.
    kIdUabdl2_v,                         //!< Instruction 'uabdl2' {ASIMD}.
    kIdUadalp_v,                         //!< Instruction 'uadalp' {ASIMD}.
    kIdUaddl_v,                          //!< Instruction 'uaddl' {ASIMD}.
    kIdUaddl2_v,                         //!< Instruction 'uaddl2' {ASIMD}.
    kIdUaddlp_v,                         //!< Instruction 'uaddlp' {ASIMD}.
    kIdUaddlv_v,                         //!< Instruction 'uaddlv' {ASIMD}.
    kIdUaddw_v,                          //!< Instruction 'uaddw' {ASIMD}.
    kIdUaddw2_v,                         //!< Instruction 'uaddw2' {ASIMD}.
    kIdUcvtf_v,                          //!< Instruction 'ucvtf' {ASIMD}.
    kIdUdot_v,                           //!< Instruction 'udot' {ASIMD}.
    kIdUhadd_v,                          //!< Instruction 'uhadd' {ASIMD}.
    kIdUhsub_v,                          //!< Instruction 'uhsub' {ASIMD}.
    kIdUmax_v,                           //!< Instruction 'umax' {ASIMD}.
    kIdUmaxp_v,                          //!< Instruction 'umaxp' {ASIMD}.
    kIdUmaxv_v,                          //!< Instruction 'umaxv' {ASIMD}.
    kIdUmin_v,                           //!< Instruction 'umin' {ASIMD}.
    kIdUminp_v,                          //!< Instruction 'uminp' {ASIMD}.
    kIdUminv_v,                          //!< Instruction 'uminv' {ASIMD}.
    kIdUmlal_v,                          //!< Instruction 'umlal' {ASIMD}.
    kIdUmlal2_v,                         //!< Instruction 'umlal2' {ASIMD}.
    kIdUmlsl_v,                          //!< Instruction 'umlsl' {ASIMD}.
    kIdUmlsl2_v,                         //!< Instruction 'umlsl2' {ASIMD}.
    kIdUmmla_v,                          //!< Instruction 'ummla' {ASIMD}.
    kIdUmov_v,                           //!< Instruction 'umov' {ASIMD}.
    kIdUmull_v,                          //!< Instruction 'umull' {ASIMD}.
    kIdUmull2_v,                         //!< Instruction 'umull2' {ASIMD}.
    kIdUqadd_v,                          //!< Instruction 'uqadd' {ASIMD}.
    kIdUqrshl_v,                         //!< Instruction 'uqrshl' {ASIMD}.
    kIdUqrshrn_v,                        //!< Instruction 'uqrshrn' {ASIMD}.
    kIdUqrshrn2_v,                       //!< Instruction 'uqrshrn2' {ASIMD}.
    kIdUqshl_v,                          //!< Instruction 'uqshl' {ASIMD}.
    kIdUqshrn_v,                         //!< Instruction 'uqshrn' {ASIMD}.
    kIdUqshrn2_v,                        //!< Instruction 'uqshrn2' {ASIMD}.
    kIdUqsub_v,                          //!< Instruction 'uqsub' {ASIMD}.
    kIdUqxtn_v,                          //!< Instruction 'uqxtn' {ASIMD}.
    kIdUqxtn2_v,                         //!< Instruction 'uqxtn2' {ASIMD}.
    kIdUrecpe_v,                         //!< Instruction 'urecpe' {ASIMD}.
    kIdUrhadd_v,                         //!< Instruction 'urhadd' {ASIMD}.
    kIdUrshl_v,                          //!< Instruction 'urshl' {ASIMD}.
    kIdUrshr_v,                          //!< Instruction 'urshr' {ASIMD}.
    kIdUrsqrte_v,                        //!< Instruction 'ursqrte' {ASIMD}.
    kIdUrsra_v,                          //!< Instruction 'ursra' {ASIMD}.
    kIdUsdot_v,                          //!< Instruction 'usdot' {ASIMD}.
    kIdUshl_v,                           //!< Instruction 'ushl' {ASIMD}.
    kIdUshll_v,                          //!< Instruction 'ushll' {ASIMD}.
    kIdUshll2_v,                         //!< Instruction 'ushll2' {ASIMD}.
    kIdUshr_v,                           //!< Instruction 'ushr' {ASIMD}.
    kIdUsmmla_v,                         //!< Instruction 'usmmla' {ASIMD}.
    kIdUsqadd_v,                         //!< Instruction 'usqadd' {ASIMD}.
    kIdUsra_v,                           //!< Instruction 'usra' {ASIMD}.
    kIdUsubl_v,                          //!< Instruction 'usubl' {ASIMD}.
    kIdUsubl2_v,                         //!< Instruction 'usubl2' {ASIMD}.
    kIdUsubw_v,                          //!< Instruction 'usubw' {ASIMD}.
    kIdUsubw2_v,                         //!< Instruction 'usubw2' {ASIMD}.
    kIdUxtl_v,                           //!< Instruction 'uxtl' {ASIMD}.
    kIdUxtl2_v,                          //!< Instruction 'uxtl2' {ASIMD}.
    kIdUzp1_v,                           //!< Instruction 'uzp1' {ASIMD}.
    kIdUzp2_v,                           //!< Instruction 'uzp2' {ASIMD}.
    kIdXar_v,                            //!< Instruction 'xar' {ASIMD}.
    kIdXtn_v,                            //!< Instruction 'xtn' {ASIMD}.
    kIdXtn2_v,                           //!< Instruction 'xtn2' {ASIMD}.
    kIdZip1_v,                           //!< Instruction 'zip1' {ASIMD}.
    kIdZip2_v,                           //!< Instruction 'zip2' {ASIMD}.
    _kIdCount
    // ${InstId:End}
  };

  //! Tests whether the `inst_id` is defined (counts also Inst::kIdNone, which must be zero).
  static ASMJIT_INLINE_NODEBUG bool is_defined_id(InstId inst_id) noexcept { return (inst_id & uint32_t(InstIdParts::kRealId)) < _kIdCount; }
}

namespace Predicate {

//! Address translate options (AT).
namespace AT {
  static ASMJIT_INLINE_CONSTEXPR uint32_t encode(uint32_t op1, uint32_t crn, uint32_t crm, uint32_t op2) noexcept {
    return (op1 << 11) | (crn << 7) | (crm << 3) | (op2 << 0);
  }

  enum Value : uint32_t {
    kS1E1R  = encode(0b000, 0b0111, 0b1000, 0b000),
    kS1E2R  = encode(0b100, 0b0111, 0b1000, 0b000),
    kS1E3R  = encode(0b110, 0b0111, 0b1000, 0b000),
    kS1E1W  = encode(0b000, 0b0111, 0b1000, 0b001),
    kS1E2W  = encode(0b100, 0b0111, 0b1000, 0b001),
    kS1E3W  = encode(0b110, 0b0111, 0b1000, 0b001),
    kS1E0R  = encode(0b000, 0b0111, 0b1000, 0b010),
    kS1E0W  = encode(0b000, 0b0111, 0b1000, 0b011),
    kS12E1R = encode(0b100, 0b0111, 0b1000, 0b100),
    kS12E1W = encode(0b100, 0b0111, 0b1000, 0b101),
    kS12E0R = encode(0b100, 0b0111, 0b1000, 0b110),
    kS12E0W = encode(0b100, 0b0111, 0b1000, 0b111),
    kS1E1RP = encode(0b000, 0b0111, 0b1001, 0b000),
    kS1E1WP = encode(0b000, 0b0111, 0b1001, 0b001)
  };
}

//! Branch target identification targets (BTI).
namespace BTI {
  //! Branch target identification targets
  enum Value : uint32_t {
    kNone = 0u,
    kC = 1u,
    kJ = 2u,
    kJC = 3u
  };
}

//! Data barrier options (DMB/DSB).
namespace DB {
  //! Data barrier immediate values.
  enum Value : uint32_t {
    //! Waits only for loads to complete, and only applies to the outer shareable domain.
    kOSHLD = 0x01u,
    //! Waits only for stores to complete, and only applies to the outer shareable domain.
    kOSHST = 0x02u,
    //! Only applies to the outer shareable domain.
    kOSH = 0x03u,

    //! Waits only for loads to complete and only applies out to the point of unification.
    kNSHLD = 0x05u,
    //! Waits only for stores to complete and only applies out to the point of unification.
    kNSHST = 0x06u,
    //! Only applies out to the point of unification.
    kNSH = 0x07u,

    //! Waits only for loads to complete, and only applies to the inner shareable domain.
    kISHLD = 0x09u,
    //! Waits only for stores to complete, and only applies to the inner shareable domain.
    kISHST = 0x0Au,
    //! Only applies to the inner shareable domain.
    kISH = 0x0Bu,

    //! Waits only for loads to complete.
    kLD = 0x0Du,
    //! Waits only for stores to complete.
    kST = 0x0Eu,
    //! Full system memory barrier operation.
    kSY = 0x0Fu
  };
}

//! Data cache maintenance options.
namespace DC {
  static ASMJIT_INLINE_CONSTEXPR uint32_t encode(uint32_t op1, uint32_t crn, uint32_t crm, uint32_t op2) noexcept {
    return (op1 << 11) | (crn << 7) | (crm << 3) | (op2 << 0);
  }

  //! Data cache maintenance immediate values.
  enum Value : uint32_t {
    kZVA     = encode(0b011, 0b0111, 0b0100, 0b001),
    kIVAC    = encode(0b000, 0b0111, 0b0110, 0b001),
    kISW     = encode(0b000, 0b0111, 0b0110, 0b010),
    kCVAC    = encode(0b011, 0b0111, 0b1010, 0b001),
    kCSW     = encode(0b000, 0b0111, 0b1010, 0b010),
    kCVAU    = encode(0b011, 0b0111, 0b1011, 0b001),
    kCIVAC   = encode(0b011, 0b0111, 0b1110, 0b001),
    kCISW    = encode(0b000, 0b0111, 0b1110, 0b010),
    kCVAP    = encode(0b011, 0b0111, 0b1100, 0b001),
    kCVADP   = encode(0b011, 0b0111, 0b1101, 0b001),
    kIGVAC   = encode(0b000, 0b0111, 0b0110, 0b011),
    kIGSW    = encode(0b000, 0b0111, 0b0110, 0b100),
    kCGSW    = encode(0b000, 0b0111, 0b1010, 0b100),
    kCIGSW   = encode(0b000, 0b0111, 0b1110, 0b100),
    kCGVAC   = encode(0b011, 0b0111, 0b1010, 0b011),
    kCGVAP   = encode(0b011, 0b0111, 0b1100, 0b011),
    kCGVADP  = encode(0b011, 0b0111, 0b1101, 0b011),
    kCIGVAC  = encode(0b011, 0b0111, 0b1110, 0b011),
    kGVA     = encode(0b011, 0b0111, 0b0100, 0b011),
    kIGDVAC  = encode(0b000, 0b0111, 0b0110, 0b101),
    kIGDSW   = encode(0b000, 0b0111, 0b0110, 0b110),
    kCGDSW   = encode(0b000, 0b0111, 0b1010, 0b110),
    kCIGDSW  = encode(0b000, 0b0111, 0b1110, 0b110),
    kCGDVAC  = encode(0b011, 0b0111, 0b1010, 0b101),
    kCGDVAP  = encode(0b011, 0b0111, 0b1100, 0b101),
    kCGDVADP = encode(0b011, 0b0111, 0b1101, 0b101),
    kCIGDVAC = encode(0b011, 0b0111, 0b1110, 0b101),
    kGZVA    = encode(0b011, 0b0111, 0b0100, 0b100)
  };
}

//! Instruction cache maintenance options.
namespace IC {
  static ASMJIT_INLINE_CONSTEXPR uint32_t encode(uint32_t op1, uint32_t crn, uint32_t crm, uint32_t op2) noexcept {
    return (op1 << 11) | (crn << 7) | (crm << 3) | (op2 << 0);
  }

  //! Instruction cache maintenance immediate values.
  enum Value : uint32_t {
    kIALLUIS = encode(0b000, 0b0111, 0b0001, 0b000),
    kIALLU   = encode(0b000, 0b0111, 0b0101, 0b000),
    kIVAU    = encode(0b011, 0b0111, 0b0101, 0b001)
  };
}

//! Instruction-fetch barrier options.
namespace ISB {
  //! Instruction-fetch barrier immediate values.
  enum Value : uint32_t {
    kSY = 0xF
  };
}

//! Prefetch options.
namespace PRFOp {
  //! Prefetch immediate values.
  enum Value : uint32_t {
    kPLDL1KEEP = 0x00,
    kPLDL1STRM = 0x01,
    kPLDL2KEEP = 0x02,
    kPLDL2STRM = 0x03,
    kPLDL3KEEP = 0x04,
    kPLDL3STRM = 0x05,
    kPLIL1KEEP = 0x08,
    kPLIL1STRM = 0x09,
    kPLIL2KEEP = 0x0A,
    kPLIL2STRM = 0x0B,
    kPLIL3KEEP = 0x0C,
    kPLIL3STRM = 0x0D,
    kPSTL1KEEP = 0x10,
    kPSTL1STRM = 0x11,
    kPSTL2KEEP = 0x12,
    kPSTL2STRM = 0x13,
    kPSTL3KEEP = 0x14,
    kPSTL3STRM = 0x15
  };
}

//! PSB instruction options.
namespace PSB {
  //! PSB immediate values.
  enum Value : uint32_t {
    kCSYNC = 0x11u
  };
}

namespace TLBI {
  static ASMJIT_INLINE_CONSTEXPR uint32_t encode(uint32_t op1, uint32_t crn, uint32_t crm, uint32_t op2) noexcept {
    return (op1 << 11) | (crn << 7) | (crm << 3) | (op2 << 0);
  }

  enum Value : uint32_t {
    kIPAS2E1IS    = encode(0b100, 0b1000, 0b0000, 0b001),
    kIPAS2LE1IS   = encode(0b100, 0b1000, 0b0000, 0b101),
    kVMALLE1IS    = encode(0b000, 0b1000, 0b0011, 0b000),
    kALLE2IS      = encode(0b100, 0b1000, 0b0011, 0b000),
    kALLE3IS      = encode(0b110, 0b1000, 0b0011, 0b000),
    kVAE1IS       = encode(0b000, 0b1000, 0b0011, 0b001),
    kVAE2IS       = encode(0b100, 0b1000, 0b0011, 0b001),
    kVAE3IS       = encode(0b110, 0b1000, 0b0011, 0b001),
    kASIDE1IS     = encode(0b000, 0b1000, 0b0011, 0b010),
    kVAAE1IS      = encode(0b000, 0b1000, 0b0011, 0b011),
    kALLE1IS      = encode(0b100, 0b1000, 0b0011, 0b100),
    kVALE1IS      = encode(0b000, 0b1000, 0b0011, 0b101),
    kVALE2IS      = encode(0b100, 0b1000, 0b0011, 0b101),
    kVALE3IS      = encode(0b110, 0b1000, 0b0011, 0b101),
    kVMALLS12E1IS = encode(0b100, 0b1000, 0b0011, 0b110),
    kVAALE1IS     = encode(0b000, 0b1000, 0b0011, 0b111),
    kIPAS2E1      = encode(0b100, 0b1000, 0b0100, 0b001),
    kIPAS2LE1     = encode(0b100, 0b1000, 0b0100, 0b101),
    kVMALLE1      = encode(0b000, 0b1000, 0b0111, 0b000),
    kALLE2        = encode(0b100, 0b1000, 0b0111, 0b000),
    kALLE3        = encode(0b110, 0b1000, 0b0111, 0b000),
    kVAE1         = encode(0b000, 0b1000, 0b0111, 0b001),
    kVAE2         = encode(0b100, 0b1000, 0b0111, 0b001),
    kVAE3         = encode(0b110, 0b1000, 0b0111, 0b001),
    kASIDE1       = encode(0b000, 0b1000, 0b0111, 0b010),
    kVAAE1        = encode(0b000, 0b1000, 0b0111, 0b011),
    kALLE1        = encode(0b100, 0b1000, 0b0111, 0b100),
    kVALE1        = encode(0b000, 0b1000, 0b0111, 0b101),
    kVALE2        = encode(0b100, 0b1000, 0b0111, 0b101),
    kVALE3        = encode(0b110, 0b1000, 0b0111, 0b101),
    kVMALLS12E1   = encode(0b100, 0b1000, 0b0111, 0b110),
    kVAALE1       = encode(0b000, 0b1000, 0b0111, 0b111),

    kVMALLE1OS    = encode(0b000, 0b1000, 0b0001, 0b000),
    kVAE1OS       = encode(0b000, 0b1000, 0b0001, 0b001),
    kASIDE1OS     = encode(0b000, 0b1000, 0b0001, 0b010),
    kVAAE1OS      = encode(0b000, 0b1000, 0b0001, 0b011),
    kVALE1OS      = encode(0b000, 0b1000, 0b0001, 0b101),
    kVAALE1OS     = encode(0b000, 0b1000, 0b0001, 0b111),
    kIPAS2E1OS    = encode(0b100, 0b1000, 0b0100, 0b000),
    kIPAS2LE1OS   = encode(0b100, 0b1000, 0b0100, 0b100),
    kVAE2OS       = encode(0b100, 0b1000, 0b0001, 0b001),
    kVALE2OS      = encode(0b100, 0b1000, 0b0001, 0b101),
    kVMALLS12E1OS = encode(0b100, 0b1000, 0b0001, 0b110),
    kVAE3OS       = encode(0b110, 0b1000, 0b0001, 0b001),
    kVALE3OS      = encode(0b110, 0b1000, 0b0001, 0b101),
    kALLE2OS      = encode(0b100, 0b1000, 0b0001, 0b000),
    kALLE1OS      = encode(0b100, 0b1000, 0b0001, 0b100),
    kALLE3OS      = encode(0b110, 0b1000, 0b0001, 0b000),

    kRVAE1        = encode(0b000, 0b1000, 0b0110, 0b001),
    kRVAAE1       = encode(0b000, 0b1000, 0b0110, 0b011),
    kRVALE1       = encode(0b000, 0b1000, 0b0110, 0b101),
    kRVAALE1      = encode(0b000, 0b1000, 0b0110, 0b111),
    kRVAE1IS      = encode(0b000, 0b1000, 0b0010, 0b001),
    kRVAAE1IS     = encode(0b000, 0b1000, 0b0010, 0b011),
    kRVALE1IS     = encode(0b000, 0b1000, 0b0010, 0b101),
    kRVAALE1IS    = encode(0b000, 0b1000, 0b0010, 0b111),
    kRVAE1OS      = encode(0b000, 0b1000, 0b0101, 0b001),
    kRVAAE1OS     = encode(0b000, 0b1000, 0b0101, 0b011),
    kRVALE1OS     = encode(0b000, 0b1000, 0b0101, 0b101),
    kRVAALE1OS    = encode(0b000, 0b1000, 0b0101, 0b111),
    kRIPAS2E1IS   = encode(0b100, 0b1000, 0b0000, 0b010),
    kRIPAS2LE1IS  = encode(0b100, 0b1000, 0b0000, 0b110),
    kRIPAS2E1     = encode(0b100, 0b1000, 0b0100, 0b010),
    kRIPAS2LE1    = encode(0b100, 0b1000, 0b0100, 0b110),
    kRIPAS2E1OS   = encode(0b100, 0b1000, 0b0100, 0b011),
    kRIPAS2LE1OS  = encode(0b100, 0b1000, 0b0100, 0b111),
    kRVAE2        = encode(0b100, 0b1000, 0b0110, 0b001),
    kRVALE2       = encode(0b100, 0b1000, 0b0110, 0b101),
    kRVAE2IS      = encode(0b100, 0b1000, 0b0010, 0b001),
    kRVALE2IS     = encode(0b100, 0b1000, 0b0010, 0b101),
    kRVAE2OS      = encode(0b100, 0b1000, 0b0101, 0b001),
    kRVALE2OS     = encode(0b100, 0b1000, 0b0101, 0b101),
    kRVAE3        = encode(0b110, 0b1000, 0b0110, 0b001),
    kRVALE3       = encode(0b110, 0b1000, 0b0110, 0b101),
    kRVAE3IS      = encode(0b110, 0b1000, 0b0010, 0b001),
    kRVALE3IS     = encode(0b110, 0b1000, 0b0010, 0b101),
    kRVAE3OS      = encode(0b110, 0b1000, 0b0101, 0b001),
    kRVALE3OS     = encode(0b110, 0b1000, 0b0101, 0b101),
  };
}

//! Trace synchronization barrier options.
namespace TSB {
  //! Trace synchronization immediate values.
  enum Value : uint32_t {
    kCSYNC = 0
  };
}

//! Processor state access through MSR.
namespace PState {
  //! Encodes a pstate from `op0` and `op1`.
  static ASMJIT_INLINE_CONSTEXPR uint32_t encode(uint32_t op0, uint32_t op1) noexcept {
    return (op0 << 3) | (op1 << 0);
  }

  //! Processor state access immediates.
  enum Value : uint32_t {
    kSPSel   = encode(0b000, 0b101),
    kDAIFSet = encode(0b011, 0b110),
    kDAIFClr = encode(0b011, 0b111),
    kPAN     = encode(0b000, 0b100),
    kUAO     = encode(0b000, 0b011),
    kDIT     = encode(0b011, 0b010),
    kSSBS    = encode(0b011, 0b001),
    kTCO     = encode(0b011, 0b100)
  };
}

//! System register identifiers and utilities (MSR/MRS).
namespace SysReg {
  //! System register fields.
  struct Fields {
    uint8_t op0;
    uint8_t op1;
    uint8_t crn;
    uint8_t crm;
    uint8_t op2;
  };

  //! Encodes a system register from `op0`, `op1`, `crn`, `crm`, and `op2` fields.
  static ASMJIT_INLINE_CONSTEXPR uint32_t encode(uint32_t op0, uint32_t op1, uint32_t crn, uint32_t crm, uint32_t op2) noexcept {
    return (op0 << 14) | (op1 << 11) | (crn << 7) | (crm << 3) | (op2 << 0);
  }

  //! Encodes a system register from `fields`.
  static ASMJIT_INLINE_CONSTEXPR uint32_t encode(const Fields& fields) noexcept {
    return encode(fields.op0, fields.op1, fields.crn, fields.crm, fields.op2);
  }

  //! Decodes a system register to \ref Fields.
  static ASMJIT_INLINE_CONSTEXPR Fields decode(uint32_t id) noexcept {
    return Fields {
      uint8_t((id >> 14) & 0x3u),
      uint8_t((id >> 11) & 0x7u),
      uint8_t((id >>  7) & 0xFu),
      uint8_t((id >>  3) & 0xFu),
      uint8_t((id >>  0) & 0x7u)
    };
  }

  //! System register identifiers.
  enum Id : uint32_t {
    kACTLR_EL1            = encode(0b11, 0b000, 0b0001, 0b0000, 0b001), // RW
    kACTLR_EL2            = encode(0b11, 0b100, 0b0001, 0b0000, 0b001), // RW
    kACTLR_EL3            = encode(0b11, 0b110, 0b0001, 0b0000, 0b001), // RW
    kAFSR0_EL1            = encode(0b11, 0b000, 0b0101, 0b0001, 0b000), // RW
    kAFSR0_EL12           = encode(0b11, 0b101, 0b0101, 0b0001, 0b000), // RW
    kAFSR0_EL2            = encode(0b11, 0b100, 0b0101, 0b0001, 0b000), // RW
    kAFSR0_EL3            = encode(0b11, 0b110, 0b0101, 0b0001, 0b000), // RW
    kAFSR1_EL1            = encode(0b11, 0b000, 0b0101, 0b0001, 0b001), // RW
    kAFSR1_EL12           = encode(0b11, 0b101, 0b0101, 0b0001, 0b001), // RW
    kAFSR1_EL2            = encode(0b11, 0b100, 0b0101, 0b0001, 0b001), // RW
    kAFSR1_EL3            = encode(0b11, 0b110, 0b0101, 0b0001, 0b001), // RW
    kAIDR_EL1             = encode(0b11, 0b001, 0b0000, 0b0000, 0b111), // RO
    kAMAIR_EL1            = encode(0b11, 0b000, 0b1010, 0b0011, 0b000), // RW
    kAMAIR_EL12           = encode(0b11, 0b101, 0b1010, 0b0011, 0b000), // RW
    kAMAIR_EL2            = encode(0b11, 0b100, 0b1010, 0b0011, 0b000), // RW
    kAMAIR_EL3            = encode(0b11, 0b110, 0b1010, 0b0011, 0b000), // RW
    kAMCFGR_EL0           = encode(0b11, 0b011, 0b1101, 0b0010, 0b001), // RO
    kAMCGCR_EL0           = encode(0b11, 0b011, 0b1101, 0b0010, 0b010), // RO
    kAMCNTENCLR0_EL0      = encode(0b11, 0b011, 0b1101, 0b0010, 0b100), // RW
    kAMCNTENCLR1_EL0      = encode(0b11, 0b011, 0b1101, 0b0011, 0b000), // RW
    kAMCNTENSET0_EL0      = encode(0b11, 0b011, 0b1101, 0b0010, 0b101), // RW
    kAMCNTENSET1_EL0      = encode(0b11, 0b011, 0b1101, 0b0011, 0b001), // RW
    kAMCR_EL0             = encode(0b11, 0b011, 0b1101, 0b0010, 0b000), // RW
    kAMEVCNTR00_EL0       = encode(0b11, 0b011, 0b1101, 0b0100, 0b000), // RW
    kAMEVCNTR01_EL0       = encode(0b11, 0b011, 0b1101, 0b0100, 0b001), // RW
    kAMEVCNTR02_EL0       = encode(0b11, 0b011, 0b1101, 0b0100, 0b010), // RW
    kAMEVCNTR03_EL0       = encode(0b11, 0b011, 0b1101, 0b0100, 0b011), // RW
    kAMEVCNTR10_EL0       = encode(0b11, 0b011, 0b1101, 0b1100, 0b000), // RW
    kAMEVCNTR110_EL0      = encode(0b11, 0b011, 0b1101, 0b1101, 0b010), // RW
    kAMEVCNTR111_EL0      = encode(0b11, 0b011, 0b1101, 0b1101, 0b011), // RW
    kAMEVCNTR112_EL0      = encode(0b11, 0b011, 0b1101, 0b1101, 0b100), // RW
    kAMEVCNTR113_EL0      = encode(0b11, 0b011, 0b1101, 0b1101, 0b101), // RW
    kAMEVCNTR114_EL0      = encode(0b11, 0b011, 0b1101, 0b1101, 0b110), // RW
    kAMEVCNTR115_EL0      = encode(0b11, 0b011, 0b1101, 0b1101, 0b111), // RW
    kAMEVCNTR11_EL0       = encode(0b11, 0b011, 0b1101, 0b1100, 0b001), // RW
    kAMEVCNTR12_EL0       = encode(0b11, 0b011, 0b1101, 0b1100, 0b010), // RW
    kAMEVCNTR13_EL0       = encode(0b11, 0b011, 0b1101, 0b1100, 0b011), // RW
    kAMEVCNTR14_EL0       = encode(0b11, 0b011, 0b1101, 0b1100, 0b100), // RW
    kAMEVCNTR15_EL0       = encode(0b11, 0b011, 0b1101, 0b1100, 0b101), // RW
    kAMEVCNTR16_EL0       = encode(0b11, 0b011, 0b1101, 0b1100, 0b110), // RW
    kAMEVCNTR17_EL0       = encode(0b11, 0b011, 0b1101, 0b1100, 0b111), // RW
    kAMEVCNTR18_EL0       = encode(0b11, 0b011, 0b1101, 0b1101, 0b000), // RW
    kAMEVCNTR19_EL0       = encode(0b11, 0b011, 0b1101, 0b1101, 0b001), // RW
    kAMEVTYPER00_EL0      = encode(0b11, 0b011, 0b1101, 0b0110, 0b000), // RO
    kAMEVTYPER01_EL0      = encode(0b11, 0b011, 0b1101, 0b0110, 0b001), // RO
    kAMEVTYPER02_EL0      = encode(0b11, 0b011, 0b1101, 0b0110, 0b010), // RO
    kAMEVTYPER03_EL0      = encode(0b11, 0b011, 0b1101, 0b0110, 0b011), // RO
    kAMEVTYPER10_EL0      = encode(0b11, 0b011, 0b1101, 0b1110, 0b000), // RW
    kAMEVTYPER110_EL0     = encode(0b11, 0b011, 0b1101, 0b1111, 0b010), // RW
    kAMEVTYPER111_EL0     = encode(0b11, 0b011, 0b1101, 0b1111, 0b011), // RW
    kAMEVTYPER112_EL0     = encode(0b11, 0b011, 0b1101, 0b1111, 0b100), // RW
    kAMEVTYPER113_EL0     = encode(0b11, 0b011, 0b1101, 0b1111, 0b101), // RW
    kAMEVTYPER114_EL0     = encode(0b11, 0b011, 0b1101, 0b1111, 0b110), // RW
    kAMEVTYPER115_EL0     = encode(0b11, 0b011, 0b1101, 0b1111, 0b111), // RW
    kAMEVTYPER11_EL0      = encode(0b11, 0b011, 0b1101, 0b1110, 0b001), // RW
    kAMEVTYPER12_EL0      = encode(0b11, 0b011, 0b1101, 0b1110, 0b010), // RW
    kAMEVTYPER13_EL0      = encode(0b11, 0b011, 0b1101, 0b1110, 0b011), // RW
    kAMEVTYPER14_EL0      = encode(0b11, 0b011, 0b1101, 0b1110, 0b100), // RW
    kAMEVTYPER15_EL0      = encode(0b11, 0b011, 0b1101, 0b1110, 0b101), // RW
    kAMEVTYPER16_EL0      = encode(0b11, 0b011, 0b1101, 0b1110, 0b110), // RW
    kAMEVTYPER17_EL0      = encode(0b11, 0b011, 0b1101, 0b1110, 0b111), // RW
    kAMEVTYPER18_EL0      = encode(0b11, 0b011, 0b1101, 0b1111, 0b000), // RW
    kAMEVTYPER19_EL0      = encode(0b11, 0b011, 0b1101, 0b1111, 0b001), // RW
    kAMUSERENR_EL0        = encode(0b11, 0b011, 0b1101, 0b0010, 0b011), // RW
    kAPDAKeyHi_EL1        = encode(0b11, 0b000, 0b0010, 0b0010, 0b001), // RW
    kAPDAKeyLo_EL1        = encode(0b11, 0b000, 0b0010, 0b0010, 0b000), // RW
    kAPDBKeyHi_EL1        = encode(0b11, 0b000, 0b0010, 0b0010, 0b011), // RW
    kAPDBKeyLo_EL1        = encode(0b11, 0b000, 0b0010, 0b0010, 0b010), // RW
    kAPGAKeyHi_EL1        = encode(0b11, 0b000, 0b0010, 0b0011, 0b001), // RW
    kAPGAKeyLo_EL1        = encode(0b11, 0b000, 0b0010, 0b0011, 0b000), // RW
    kAPIAKeyHi_EL1        = encode(0b11, 0b000, 0b0010, 0b0001, 0b001), // RW
    kAPIAKeyLo_EL1        = encode(0b11, 0b000, 0b0010, 0b0001, 0b000), // RW
    kAPIBKeyHi_EL1        = encode(0b11, 0b000, 0b0010, 0b0001, 0b011), // RW
    kAPIBKeyLo_EL1        = encode(0b11, 0b000, 0b0010, 0b0001, 0b010), // RW
    kCCSIDR2_EL1          = encode(0b11, 0b001, 0b0000, 0b0000, 0b010), // RO
    kCCSIDR_EL1           = encode(0b11, 0b001, 0b0000, 0b0000, 0b000), // RO
    kCLIDR_EL1            = encode(0b11, 0b001, 0b0000, 0b0000, 0b001), // RO
    kCNTFRQ_EL0           = encode(0b11, 0b011, 0b1110, 0b0000, 0b000), // RW
    kCNTHCTL_EL2          = encode(0b11, 0b100, 0b1110, 0b0001, 0b000), // RW
    kCNTHPS_CTL_EL2       = encode(0b11, 0b100, 0b1110, 0b0101, 0b001), // RW
    kCNTHPS_CVAL_EL2      = encode(0b11, 0b100, 0b1110, 0b0101, 0b010), // RW
    kCNTHPS_TVAL_EL2      = encode(0b11, 0b100, 0b1110, 0b0101, 0b000), // RW
    kCNTHP_CTL_EL2        = encode(0b11, 0b100, 0b1110, 0b0010, 0b001), // RW
    kCNTHP_CVAL_EL2       = encode(0b11, 0b100, 0b1110, 0b0010, 0b010), // RW
    kCNTHP_TVAL_EL2       = encode(0b11, 0b100, 0b1110, 0b0010, 0b000), // RW
    kCNTHVS_CTL_EL2       = encode(0b11, 0b100, 0b1110, 0b0100, 0b001), // RW
    kCNTHVS_CVAL_EL2      = encode(0b11, 0b100, 0b1110, 0b0100, 0b010), // RW
    kCNTHVS_TVAL_EL2      = encode(0b11, 0b100, 0b1110, 0b0100, 0b000), // RW
    kCNTHV_CTL_EL2        = encode(0b11, 0b100, 0b1110, 0b0011, 0b001), // RW
    kCNTHV_CVAL_EL2       = encode(0b11, 0b100, 0b1110, 0b0011, 0b010), // RW
    kCNTHV_TVAL_EL2       = encode(0b11, 0b100, 0b1110, 0b0011, 0b000), // RW
    kCNTISCALE_EL2        = encode(0b11, 0b100, 0b1110, 0b0000, 0b101), // RW
    kCNTKCTL_EL1          = encode(0b11, 0b000, 0b1110, 0b0001, 0b000), // RW
    kCNTKCTL_EL12         = encode(0b11, 0b101, 0b1110, 0b0001, 0b000), // RW
    kCNTPCTSS_EL0         = encode(0b11, 0b011, 0b1110, 0b0000, 0b101), // RW
    kCNTPCT_EL0           = encode(0b11, 0b011, 0b1110, 0b0000, 0b001), // RO
    kCNTPOFF_EL2          = encode(0b11, 0b100, 0b1110, 0b0000, 0b110), // RW
    kCNTPS_CTL_EL1        = encode(0b11, 0b111, 0b1110, 0b0010, 0b001), // RW
    kCNTPS_CVAL_EL1       = encode(0b11, 0b111, 0b1110, 0b0010, 0b010), // RW
    kCNTPS_TVAL_EL1       = encode(0b11, 0b111, 0b1110, 0b0010, 0b000), // RW
    kCNTP_CTL_EL0         = encode(0b11, 0b011, 0b1110, 0b0010, 0b001), // RW
    kCNTP_CTL_EL02        = encode(0b11, 0b101, 0b1110, 0b0010, 0b001), // RW
    kCNTP_CVAL_EL0        = encode(0b11, 0b011, 0b1110, 0b0010, 0b010), // RW
    kCNTP_CVAL_EL02       = encode(0b11, 0b101, 0b1110, 0b0010, 0b010), // RW
    kCNTP_TVAL_EL0        = encode(0b11, 0b011, 0b1110, 0b0010, 0b000), // RW
    kCNTP_TVAL_EL02       = encode(0b11, 0b101, 0b1110, 0b0010, 0b000), // RW
    kCNTSCALE_EL2         = encode(0b11, 0b100, 0b1110, 0b0000, 0b100), // RW
    kCNTVCTSS_EL0         = encode(0b11, 0b011, 0b1110, 0b0000, 0b110), // RW
    kCNTVCT_EL0           = encode(0b11, 0b011, 0b1110, 0b0000, 0b010), // RO
    kCNTVFRQ_EL2          = encode(0b11, 0b100, 0b1110, 0b0000, 0b111), // RW
    kCNTVOFF_EL2          = encode(0b11, 0b100, 0b1110, 0b0000, 0b011), // RW
    kCNTV_CTL_EL0         = encode(0b11, 0b011, 0b1110, 0b0011, 0b001), // RW
    kCNTV_CTL_EL02        = encode(0b11, 0b101, 0b1110, 0b0011, 0b001), // RW
    kCNTV_CVAL_EL0        = encode(0b11, 0b011, 0b1110, 0b0011, 0b010), // RW
    kCNTV_CVAL_EL02       = encode(0b11, 0b101, 0b1110, 0b0011, 0b010), // RW
    kCNTV_TVAL_EL0        = encode(0b11, 0b011, 0b1110, 0b0011, 0b000), // RW
    kCNTV_TVAL_EL02       = encode(0b11, 0b101, 0b1110, 0b0011, 0b000), // RW
    kCONTEXTIDR_EL1       = encode(0b11, 0b000, 0b1101, 0b0000, 0b001), // RW
    kCONTEXTIDR_EL12      = encode(0b11, 0b101, 0b1101, 0b0000, 0b001), // RW
    kCONTEXTIDR_EL2       = encode(0b11, 0b100, 0b1101, 0b0000, 0b001), // RW
    kCPACR_EL1            = encode(0b11, 0b000, 0b0001, 0b0000, 0b010), // RW
    kCPACR_EL12           = encode(0b11, 0b101, 0b0001, 0b0000, 0b010), // RW
    kCPM_IOACC_CTL_EL3    = encode(0b11, 0b111, 0b1111, 0b0010, 0b000), // RW
    kCPTR_EL2             = encode(0b11, 0b100, 0b0001, 0b0001, 0b010), // RW
    kCPTR_EL3             = encode(0b11, 0b110, 0b0001, 0b0001, 0b010), // RW
    kCSSELR_EL1           = encode(0b11, 0b010, 0b0000, 0b0000, 0b000), // RW
    kCTR_EL0              = encode(0b11, 0b011, 0b0000, 0b0000, 0b001), // RO
    kCurrentEL            = encode(0b11, 0b000, 0b0100, 0b0010, 0b010), // RO
    kDACR32_EL2           = encode(0b11, 0b100, 0b0011, 0b0000, 0b000), // RW
    kDAIF                 = encode(0b11, 0b011, 0b0100, 0b0010, 0b001), // RW
    kDBGAUTHSTATUS_EL1    = encode(0b10, 0b000, 0b0111, 0b1110, 0b110), // RO
    kDBGBCR0_EL1          = encode(0b10, 0b000, 0b0000, 0b0000, 0b101), // RW
    kDBGBCR10_EL1         = encode(0b10, 0b000, 0b0000, 0b1010, 0b101), // RW
    kDBGBCR11_EL1         = encode(0b10, 0b000, 0b0000, 0b1011, 0b101), // RW
    kDBGBCR12_EL1         = encode(0b10, 0b000, 0b0000, 0b1100, 0b101), // RW
    kDBGBCR13_EL1         = encode(0b10, 0b000, 0b0000, 0b1101, 0b101), // RW
    kDBGBCR14_EL1         = encode(0b10, 0b000, 0b0000, 0b1110, 0b101), // RW
    kDBGBCR15_EL1         = encode(0b10, 0b000, 0b0000, 0b1111, 0b101), // RW
    kDBGBCR1_EL1          = encode(0b10, 0b000, 0b0000, 0b0001, 0b101), // RW
    kDBGBCR2_EL1          = encode(0b10, 0b000, 0b0000, 0b0010, 0b101), // RW
    kDBGBCR3_EL1          = encode(0b10, 0b000, 0b0000, 0b0011, 0b101), // RW
    kDBGBCR4_EL1          = encode(0b10, 0b000, 0b0000, 0b0100, 0b101), // RW
    kDBGBCR5_EL1          = encode(0b10, 0b000, 0b0000, 0b0101, 0b101), // RW
    kDBGBCR6_EL1          = encode(0b10, 0b000, 0b0000, 0b0110, 0b101), // RW
    kDBGBCR7_EL1          = encode(0b10, 0b000, 0b0000, 0b0111, 0b101), // RW
    kDBGBCR8_EL1          = encode(0b10, 0b000, 0b0000, 0b1000, 0b101), // RW
    kDBGBCR9_EL1          = encode(0b10, 0b000, 0b0000, 0b1001, 0b101), // RW
    kDBGBVR0_EL1          = encode(0b10, 0b000, 0b0000, 0b0000, 0b100), // RW
    kDBGBVR10_EL1         = encode(0b10, 0b000, 0b0000, 0b1010, 0b100), // RW
    kDBGBVR11_EL1         = encode(0b10, 0b000, 0b0000, 0b1011, 0b100), // RW
    kDBGBVR12_EL1         = encode(0b10, 0b000, 0b0000, 0b1100, 0b100), // RW
    kDBGBVR13_EL1         = encode(0b10, 0b000, 0b0000, 0b1101, 0b100), // RW
    kDBGBVR14_EL1         = encode(0b10, 0b000, 0b0000, 0b1110, 0b100), // RW
    kDBGBVR15_EL1         = encode(0b10, 0b000, 0b0000, 0b1111, 0b100), // RW
    kDBGBVR1_EL1          = encode(0b10, 0b000, 0b0000, 0b0001, 0b100), // RW
    kDBGBVR2_EL1          = encode(0b10, 0b000, 0b0000, 0b0010, 0b100), // RW
    kDBGBVR3_EL1          = encode(0b10, 0b000, 0b0000, 0b0011, 0b100), // RW
    kDBGBVR4_EL1          = encode(0b10, 0b000, 0b0000, 0b0100, 0b100), // RW
    kDBGBVR5_EL1          = encode(0b10, 0b000, 0b0000, 0b0101, 0b100), // RW
    kDBGBVR6_EL1          = encode(0b10, 0b000, 0b0000, 0b0110, 0b100), // RW
    kDBGBVR7_EL1          = encode(0b10, 0b000, 0b0000, 0b0111, 0b100), // RW
    kDBGBVR8_EL1          = encode(0b10, 0b000, 0b0000, 0b1000, 0b100), // RW
    kDBGBVR9_EL1          = encode(0b10, 0b000, 0b0000, 0b1001, 0b100), // RW
    kDBGCLAIMCLR_EL1      = encode(0b10, 0b000, 0b0111, 0b1001, 0b110), // RW
    kDBGCLAIMSET_EL1      = encode(0b10, 0b000, 0b0111, 0b1000, 0b110), // RW
    kDBGDTRRX_EL0         = encode(0b10, 0b011, 0b0000, 0b0101, 0b000), // RO
    kDBGDTRTX_EL0         = encode(0b10, 0b011, 0b0000, 0b0101, 0b000), // WO
    kDBGDTR_EL0           = encode(0b10, 0b011, 0b0000, 0b0100, 0b000), // RW
    kDBGPRCR_EL1          = encode(0b10, 0b000, 0b0001, 0b0100, 0b100), // RW
    kDBGVCR32_EL2         = encode(0b10, 0b100, 0b0000, 0b0111, 0b000), // RW
    kDBGWCR0_EL1          = encode(0b10, 0b000, 0b0000, 0b0000, 0b111), // RW
    kDBGWCR10_EL1         = encode(0b10, 0b000, 0b0000, 0b1010, 0b111), // RW
    kDBGWCR11_EL1         = encode(0b10, 0b000, 0b0000, 0b1011, 0b111), // RW
    kDBGWCR12_EL1         = encode(0b10, 0b000, 0b0000, 0b1100, 0b111), // RW
    kDBGWCR13_EL1         = encode(0b10, 0b000, 0b0000, 0b1101, 0b111), // RW
    kDBGWCR14_EL1         = encode(0b10, 0b000, 0b0000, 0b1110, 0b111), // RW
    kDBGWCR15_EL1         = encode(0b10, 0b000, 0b0000, 0b1111, 0b111), // RW
    kDBGWCR1_EL1          = encode(0b10, 0b000, 0b0000, 0b0001, 0b111), // RW
    kDBGWCR2_EL1          = encode(0b10, 0b000, 0b0000, 0b0010, 0b111), // RW
    kDBGWCR3_EL1          = encode(0b10, 0b000, 0b0000, 0b0011, 0b111), // RW
    kDBGWCR4_EL1          = encode(0b10, 0b000, 0b0000, 0b0100, 0b111), // RW
    kDBGWCR5_EL1          = encode(0b10, 0b000, 0b0000, 0b0101, 0b111), // RW
    kDBGWCR6_EL1          = encode(0b10, 0b000, 0b0000, 0b0110, 0b111), // RW
    kDBGWCR7_EL1          = encode(0b10, 0b000, 0b0000, 0b0111, 0b111), // RW
    kDBGWCR8_EL1          = encode(0b10, 0b000, 0b0000, 0b1000, 0b111), // RW
    kDBGWCR9_EL1          = encode(0b10, 0b000, 0b0000, 0b1001, 0b111), // RW
    kDBGWVR0_EL1          = encode(0b10, 0b000, 0b0000, 0b0000, 0b110), // RW
    kDBGWVR10_EL1         = encode(0b10, 0b000, 0b0000, 0b1010, 0b110), // RW
    kDBGWVR11_EL1         = encode(0b10, 0b000, 0b0000, 0b1011, 0b110), // RW
    kDBGWVR12_EL1         = encode(0b10, 0b000, 0b0000, 0b1100, 0b110), // RW
    kDBGWVR13_EL1         = encode(0b10, 0b000, 0b0000, 0b1101, 0b110), // RW
    kDBGWVR14_EL1         = encode(0b10, 0b000, 0b0000, 0b1110, 0b110), // RW
    kDBGWVR15_EL1         = encode(0b10, 0b000, 0b0000, 0b1111, 0b110), // RW
    kDBGWVR1_EL1          = encode(0b10, 0b000, 0b0000, 0b0001, 0b110), // RW
    kDBGWVR2_EL1          = encode(0b10, 0b000, 0b0000, 0b0010, 0b110), // RW
    kDBGWVR3_EL1          = encode(0b10, 0b000, 0b0000, 0b0011, 0b110), // RW
    kDBGWVR4_EL1          = encode(0b10, 0b000, 0b0000, 0b0100, 0b110), // RW
    kDBGWVR5_EL1          = encode(0b10, 0b000, 0b0000, 0b0101, 0b110), // RW
    kDBGWVR6_EL1          = encode(0b10, 0b000, 0b0000, 0b0110, 0b110), // RW
    kDBGWVR7_EL1          = encode(0b10, 0b000, 0b0000, 0b0111, 0b110), // RW
    kDBGWVR8_EL1          = encode(0b10, 0b000, 0b0000, 0b1000, 0b110), // RW
    kDBGWVR9_EL1          = encode(0b10, 0b000, 0b0000, 0b1001, 0b110), // RW
    kDCZID_EL0            = encode(0b11, 0b011, 0b0000, 0b0000, 0b111), // RO
    kDISR_EL1             = encode(0b11, 0b000, 0b1100, 0b0001, 0b001), // RW
    kDIT                  = encode(0b11, 0b011, 0b0100, 0b0010, 0b101), // RW
    kDLR_EL0              = encode(0b11, 0b011, 0b0100, 0b0101, 0b001), // RW
    kDSPSR_EL0            = encode(0b11, 0b011, 0b0100, 0b0101, 0b000), // RW
    kELR_EL1              = encode(0b11, 0b000, 0b0100, 0b0000, 0b001), // RW
    kELR_EL12             = encode(0b11, 0b101, 0b0100, 0b0000, 0b001), // RW
    kELR_EL2              = encode(0b11, 0b100, 0b0100, 0b0000, 0b001), // RW
    kELR_EL3              = encode(0b11, 0b110, 0b0100, 0b0000, 0b001), // RW
    kERRIDR_EL1           = encode(0b11, 0b000, 0b0101, 0b0011, 0b000), // RO
    kERRSELR_EL1          = encode(0b11, 0b000, 0b0101, 0b0011, 0b001), // RW
    kERXADDR_EL1          = encode(0b11, 0b000, 0b0101, 0b0100, 0b011), // RW
    kERXCTLR_EL1          = encode(0b11, 0b000, 0b0101, 0b0100, 0b001), // RW
    kERXFR_EL1            = encode(0b11, 0b000, 0b0101, 0b0100, 0b000), // RO
    kERXMISC0_EL1         = encode(0b11, 0b000, 0b0101, 0b0101, 0b000), // RW
    kERXMISC1_EL1         = encode(0b11, 0b000, 0b0101, 0b0101, 0b001), // RW
    kERXMISC2_EL1         = encode(0b11, 0b000, 0b0101, 0b0101, 0b010), // RW
    kERXMISC3_EL1         = encode(0b11, 0b000, 0b0101, 0b0101, 0b011), // RW
    kERXPFGCDN_EL1        = encode(0b11, 0b000, 0b0101, 0b0100, 0b110), // RW
    kERXPFGCTL_EL1        = encode(0b11, 0b000, 0b0101, 0b0100, 0b101), // RW
    kERXPFGF_EL1          = encode(0b11, 0b000, 0b0101, 0b0100, 0b100), // RO
    kERXSTATUS_EL1        = encode(0b11, 0b000, 0b0101, 0b0100, 0b010), // RW
    kESR_EL1              = encode(0b11, 0b000, 0b0101, 0b0010, 0b000), // RW
    kESR_EL12             = encode(0b11, 0b101, 0b0101, 0b0010, 0b000), // RW
    kESR_EL2              = encode(0b11, 0b100, 0b0101, 0b0010, 0b000), // RW
    kESR_EL3              = encode(0b11, 0b110, 0b0101, 0b0010, 0b000), // RW
    kFAR_EL1              = encode(0b11, 0b000, 0b0110, 0b0000, 0b000), // RW
    kFAR_EL12             = encode(0b11, 0b101, 0b0110, 0b0000, 0b000), // RW
    kFAR_EL2              = encode(0b11, 0b100, 0b0110, 0b0000, 0b000), // RW
    kFAR_EL3              = encode(0b11, 0b110, 0b0110, 0b0000, 0b000), // RW
    kFPCR                 = encode(0b11, 0b011, 0b0100, 0b0100, 0b000), // RW
    kFPEXC32_EL2          = encode(0b11, 0b100, 0b0101, 0b0011, 0b000), // RW
    kFPSR                 = encode(0b11, 0b011, 0b0100, 0b0100, 0b001), // RW
    kGCR_EL1              = encode(0b11, 0b000, 0b0001, 0b0000, 0b110), // RW
    kGMID_EL1             = encode(0b11, 0b001, 0b0000, 0b0000, 0b100), // RO
    kHACR_EL2             = encode(0b11, 0b100, 0b0001, 0b0001, 0b111), // RW
    kHCR_EL2              = encode(0b11, 0b100, 0b0001, 0b0001, 0b000), // RW
    kHDFGRTR_EL2          = encode(0b11, 0b100, 0b0011, 0b0001, 0b100), // RW
    kHDFGWTR_EL2          = encode(0b11, 0b100, 0b0011, 0b0001, 0b101), // RW
    kHFGITR_EL2           = encode(0b11, 0b100, 0b0001, 0b0001, 0b110), // RW
    kHFGRTR_EL2           = encode(0b11, 0b100, 0b0001, 0b0001, 0b100), // RW
    kHFGWTR_EL2           = encode(0b11, 0b100, 0b0001, 0b0001, 0b101), // RW
    kHPFAR_EL2            = encode(0b11, 0b100, 0b0110, 0b0000, 0b100), // RW
    kHSTR_EL2             = encode(0b11, 0b100, 0b0001, 0b0001, 0b011), // RW
    kICC_AP0R0_EL1        = encode(0b11, 0b000, 0b1100, 0b1000, 0b100), // RW
    kICC_AP0R1_EL1        = encode(0b11, 0b000, 0b1100, 0b1000, 0b101), // RW
    kICC_AP0R2_EL1        = encode(0b11, 0b000, 0b1100, 0b1000, 0b110), // RW
    kICC_AP0R3_EL1        = encode(0b11, 0b000, 0b1100, 0b1000, 0b111), // RW
    kICC_AP1R0_EL1        = encode(0b11, 0b000, 0b1100, 0b1001, 0b000), // RW
    kICC_AP1R1_EL1        = encode(0b11, 0b000, 0b1100, 0b1001, 0b001), // RW
    kICC_AP1R2_EL1        = encode(0b11, 0b000, 0b1100, 0b1001, 0b010), // RW
    kICC_AP1R3_EL1        = encode(0b11, 0b000, 0b1100, 0b1001, 0b011), // RW
    kICC_ASGI1R_EL1       = encode(0b11, 0b000, 0b1100, 0b1011, 0b110), // WO
    kICC_BPR0_EL1         = encode(0b11, 0b000, 0b1100, 0b1000, 0b011), // RW
    kICC_BPR1_EL1         = encode(0b11, 0b000, 0b1100, 0b1100, 0b011), // RW
    kICC_CTLR_EL1         = encode(0b11, 0b000, 0b1100, 0b1100, 0b100), // RW
    kICC_CTLR_EL3         = encode(0b11, 0b110, 0b1100, 0b1100, 0b100), // RW
    kICC_DIR_EL1          = encode(0b11, 0b000, 0b1100, 0b1011, 0b001), // WO
    kICC_EOIR0_EL1        = encode(0b11, 0b000, 0b1100, 0b1000, 0b001), // WO
    kICC_EOIR1_EL1        = encode(0b11, 0b000, 0b1100, 0b1100, 0b001), // WO
    kICC_HPPIR0_EL1       = encode(0b11, 0b000, 0b1100, 0b1000, 0b010), // RO
    kICC_HPPIR1_EL1       = encode(0b11, 0b000, 0b1100, 0b1100, 0b010), // RO
    kICC_IAR0_EL1         = encode(0b11, 0b000, 0b1100, 0b1000, 0b000), // RO
    kICC_IAR1_EL1         = encode(0b11, 0b000, 0b1100, 0b1100, 0b000), // RO
    kICC_IGRPEN0_EL1      = encode(0b11, 0b000, 0b1100, 0b1100, 0b110), // RW
    kICC_IGRPEN1_EL1      = encode(0b11, 0b000, 0b1100, 0b1100, 0b111), // RW
    kICC_IGRPEN1_EL3      = encode(0b11, 0b110, 0b1100, 0b1100, 0b111), // RW
    kICC_PMR_EL1          = encode(0b11, 0b000, 0b0100, 0b0110, 0b000), // RW
    kICC_RPR_EL1          = encode(0b11, 0b000, 0b1100, 0b1011, 0b011), // RO
    kICC_SGI0R_EL1        = encode(0b11, 0b000, 0b1100, 0b1011, 0b111), // WO
    kICC_SGI1R_EL1        = encode(0b11, 0b000, 0b1100, 0b1011, 0b101), // WO
    kICC_SRE_EL1          = encode(0b11, 0b000, 0b1100, 0b1100, 0b101), // RW
    kICC_SRE_EL2          = encode(0b11, 0b100, 0b1100, 0b1001, 0b101), // RW
    kICC_SRE_EL3          = encode(0b11, 0b110, 0b1100, 0b1100, 0b101), // RW
    kICH_AP0R0_EL2        = encode(0b11, 0b100, 0b1100, 0b1000, 0b000), // RW
    kICH_AP0R1_EL2        = encode(0b11, 0b100, 0b1100, 0b1000, 0b001), // RW
    kICH_AP0R2_EL2        = encode(0b11, 0b100, 0b1100, 0b1000, 0b010), // RW
    kICH_AP0R3_EL2        = encode(0b11, 0b100, 0b1100, 0b1000, 0b011), // RW
    kICH_AP1R0_EL2        = encode(0b11, 0b100, 0b1100, 0b1001, 0b000), // RW
    kICH_AP1R1_EL2        = encode(0b11, 0b100, 0b1100, 0b1001, 0b001), // RW
    kICH_AP1R2_EL2        = encode(0b11, 0b100, 0b1100, 0b1001, 0b010), // RW
    kICH_AP1R3_EL2        = encode(0b11, 0b100, 0b1100, 0b1001, 0b011), // RW
    kICH_EISR_EL2         = encode(0b11, 0b100, 0b1100, 0b1011, 0b011), // RO
    kICH_ELRSR_EL2        = encode(0b11, 0b100, 0b1100, 0b1011, 0b101), // RO
    kICH_HCR_EL2          = encode(0b11, 0b100, 0b1100, 0b1011, 0b000), // RW
    kICH_LR0_EL2          = encode(0b11, 0b100, 0b1100, 0b1100, 0b000), // RW
    kICH_LR10_EL2         = encode(0b11, 0b100, 0b1100, 0b1101, 0b010), // RW
    kICH_LR11_EL2         = encode(0b11, 0b100, 0b1100, 0b1101, 0b011), // RW
    kICH_LR12_EL2         = encode(0b11, 0b100, 0b1100, 0b1101, 0b100), // RW
    kICH_LR13_EL2         = encode(0b11, 0b100, 0b1100, 0b1101, 0b101), // RW
    kICH_LR14_EL2         = encode(0b11, 0b100, 0b1100, 0b1101, 0b110), // RW
    kICH_LR15_EL2         = encode(0b11, 0b100, 0b1100, 0b1101, 0b111), // RW
    kICH_LR1_EL2          = encode(0b11, 0b100, 0b1100, 0b1100, 0b001), // RW
    kICH_LR2_EL2          = encode(0b11, 0b100, 0b1100, 0b1100, 0b010), // RW
    kICH_LR3_EL2          = encode(0b11, 0b100, 0b1100, 0b1100, 0b011), // RW
    kICH_LR4_EL2          = encode(0b11, 0b100, 0b1100, 0b1100, 0b100), // RW
    kICH_LR5_EL2          = encode(0b11, 0b100, 0b1100, 0b1100, 0b101), // RW
    kICH_LR6_EL2          = encode(0b11, 0b100, 0b1100, 0b1100, 0b110), // RW
    kICH_LR7_EL2          = encode(0b11, 0b100, 0b1100, 0b1100, 0b111), // RW
    kICH_LR8_EL2          = encode(0b11, 0b100, 0b1100, 0b1101, 0b000), // RW
    kICH_LR9_EL2          = encode(0b11, 0b100, 0b1100, 0b1101, 0b001), // RW
    kICH_MISR_EL2         = encode(0b11, 0b100, 0b1100, 0b1011, 0b010), // RO
    kICH_VMCR_EL2         = encode(0b11, 0b100, 0b1100, 0b1011, 0b111), // RW
    kICH_VTR_EL2          = encode(0b11, 0b100, 0b1100, 0b1011, 0b001), // RO
    kID_AA64AFR0_EL1      = encode(0b11, 0b000, 0b0000, 0b0101, 0b100), // RO
    kID_AA64AFR1_EL1      = encode(0b11, 0b000, 0b0000, 0b0101, 0b101), // RO
    kID_AA64DFR0_EL1      = encode(0b11, 0b000, 0b0000, 0b0101, 0b000), // RO
    kID_AA64DFR1_EL1      = encode(0b11, 0b000, 0b0000, 0b0101, 0b001), // RO
    kID_AA64ISAR0_EL1     = encode(0b11, 0b000, 0b0000, 0b0110, 0b000), // RO
    kID_AA64ISAR1_EL1     = encode(0b11, 0b000, 0b0000, 0b0110, 0b001), // RO
    kID_AA64ISAR2_EL1     = encode(0b11, 0b000, 0b0000, 0b0110, 0b010), // RO
    kID_AA64MMFR0_EL1     = encode(0b11, 0b000, 0b0000, 0b0111, 0b000), // RO
    kID_AA64MMFR1_EL1     = encode(0b11, 0b000, 0b0000, 0b0111, 0b001), // RO
    kID_AA64MMFR2_EL1     = encode(0b11, 0b000, 0b0000, 0b0111, 0b010), // RO
    kID_AA64MMFR3_EL1     = encode(0b11, 0b000, 0b0000, 0b0111, 0b011), // RO
    kID_AA64MMFR4_EL1     = encode(0b11, 0b000, 0b0000, 0b0111, 0b100), // RO
    kID_AA64PFR0_EL1      = encode(0b11, 0b000, 0b0000, 0b0100, 0b000), // RO
    kID_AA64PFR1_EL1      = encode(0b11, 0b000, 0b0000, 0b0100, 0b001), // RO
    kID_AA64ZFR0_EL1      = encode(0b11, 0b000, 0b0000, 0b0100, 0b100), // RO
    kID_AFR0_EL1          = encode(0b11, 0b000, 0b0000, 0b0001, 0b011), // RO
    kID_DFR0_EL1          = encode(0b11, 0b000, 0b0000, 0b0001, 0b010), // RO
    kID_ISAR0_EL1         = encode(0b11, 0b000, 0b0000, 0b0010, 0b000), // RO
    kID_ISAR1_EL1         = encode(0b11, 0b000, 0b0000, 0b0010, 0b001), // RO
    kID_ISAR2_EL1         = encode(0b11, 0b000, 0b0000, 0b0010, 0b010), // RO
    kID_ISAR3_EL1         = encode(0b11, 0b000, 0b0000, 0b0010, 0b011), // RO
    kID_ISAR4_EL1         = encode(0b11, 0b000, 0b0000, 0b0010, 0b100), // RO
    kID_ISAR5_EL1         = encode(0b11, 0b000, 0b0000, 0b0010, 0b101), // RO
    kID_ISAR6_EL1         = encode(0b11, 0b000, 0b0000, 0b0010, 0b111), // RO
    kID_MMFR0_EL1         = encode(0b11, 0b000, 0b0000, 0b0001, 0b100), // RO
    kID_MMFR1_EL1         = encode(0b11, 0b000, 0b0000, 0b0001, 0b101), // RO
    kID_MMFR2_EL1         = encode(0b11, 0b000, 0b0000, 0b0001, 0b110), // RO
    kID_MMFR3_EL1         = encode(0b11, 0b000, 0b0000, 0b0001, 0b111), // RO
    kID_MMFR4_EL1         = encode(0b11, 0b000, 0b0000, 0b0010, 0b110), // RO
    kID_MMFR5_EL1         = encode(0b11, 0b000, 0b0000, 0b0011, 0b110), // RO
    kID_PFR0_EL1          = encode(0b11, 0b000, 0b0000, 0b0001, 0b000), // RO
    kID_PFR1_EL1          = encode(0b11, 0b000, 0b0000, 0b0001, 0b001), // RO
    kID_PFR2_EL1          = encode(0b11, 0b000, 0b0000, 0b0011, 0b100), // RO
    kIFSR32_EL2           = encode(0b11, 0b100, 0b0101, 0b0000, 0b001), // RW
    kISR_EL1              = encode(0b11, 0b000, 0b1100, 0b0001, 0b000), // RO
    kLORC_EL1             = encode(0b11, 0b000, 0b1010, 0b0100, 0b011), // RW
    kLOREA_EL1            = encode(0b11, 0b000, 0b1010, 0b0100, 0b001), // RW
    kLORID_EL1            = encode(0b11, 0b000, 0b1010, 0b0100, 0b111), // RO
    kLORN_EL1             = encode(0b11, 0b000, 0b1010, 0b0100, 0b010), // RW
    kLORSA_EL1            = encode(0b11, 0b000, 0b1010, 0b0100, 0b000), // RW
    kMAIR_EL1             = encode(0b11, 0b000, 0b1010, 0b0010, 0b000), // RW
    kMAIR_EL12            = encode(0b11, 0b101, 0b1010, 0b0010, 0b000), // RW
    kMAIR_EL2             = encode(0b11, 0b100, 0b1010, 0b0010, 0b000), // RW
    kMAIR_EL3             = encode(0b11, 0b110, 0b1010, 0b0010, 0b000), // RW
    kMDCCINT_EL1          = encode(0b10, 0b000, 0b0000, 0b0010, 0b000), // RW
    kMDCCSR_EL0           = encode(0b10, 0b011, 0b0000, 0b0001, 0b000), // RO
    kMDCR_EL2             = encode(0b11, 0b100, 0b0001, 0b0001, 0b001), // RW
    kMDCR_EL3             = encode(0b11, 0b110, 0b0001, 0b0011, 0b001), // RW
    kMDRAR_EL1            = encode(0b10, 0b000, 0b0001, 0b0000, 0b000), // RO
    kMDSCR_EL1            = encode(0b10, 0b000, 0b0000, 0b0010, 0b010), // RW
    kMIDR_EL1             = encode(0b11, 0b000, 0b0000, 0b0000, 0b000), // RO
    kMPAM0_EL1            = encode(0b11, 0b000, 0b1010, 0b0101, 0b001), // RW
    kMPAM1_EL1            = encode(0b11, 0b000, 0b1010, 0b0101, 0b000), // RW
    kMPAM1_EL12           = encode(0b11, 0b101, 0b1010, 0b0101, 0b000), // RW
    kMPAM2_EL2            = encode(0b11, 0b100, 0b1010, 0b0101, 0b000), // RW
    kMPAM3_EL3            = encode(0b11, 0b110, 0b1010, 0b0101, 0b000), // RW
    kMPAMHCR_EL2          = encode(0b11, 0b100, 0b1010, 0b0100, 0b000), // RW
    kMPAMIDR_EL1          = encode(0b11, 0b000, 0b1010, 0b0100, 0b100), // RO
    kMPAMVPM0_EL2         = encode(0b11, 0b100, 0b1010, 0b0110, 0b000), // RW
    kMPAMVPM1_EL2         = encode(0b11, 0b100, 0b1010, 0b0110, 0b001), // RW
    kMPAMVPM2_EL2         = encode(0b11, 0b100, 0b1010, 0b0110, 0b010), // RW
    kMPAMVPM3_EL2         = encode(0b11, 0b100, 0b1010, 0b0110, 0b011), // RW
    kMPAMVPM4_EL2         = encode(0b11, 0b100, 0b1010, 0b0110, 0b100), // RW
    kMPAMVPM5_EL2         = encode(0b11, 0b100, 0b1010, 0b0110, 0b101), // RW
    kMPAMVPM6_EL2         = encode(0b11, 0b100, 0b1010, 0b0110, 0b110), // RW
    kMPAMVPM7_EL2         = encode(0b11, 0b100, 0b1010, 0b0110, 0b111), // RW
    kMPAMVPMV_EL2         = encode(0b11, 0b100, 0b1010, 0b0100, 0b001), // RW
    kMPIDR_EL1            = encode(0b11, 0b000, 0b0000, 0b0000, 0b101), // RO
    kMVFR0_EL1            = encode(0b11, 0b000, 0b0000, 0b0011, 0b000), // RO
    kMVFR1_EL1            = encode(0b11, 0b000, 0b0000, 0b0011, 0b001), // RO
    kMVFR2_EL1            = encode(0b11, 0b000, 0b0000, 0b0011, 0b010), // RO
    kNZCV                 = encode(0b11, 0b011, 0b0100, 0b0010, 0b000), // RW
    kOSDLR_EL1            = encode(0b10, 0b000, 0b0001, 0b0011, 0b100), // RW
    kOSDTRRX_EL1          = encode(0b10, 0b000, 0b0000, 0b0000, 0b010), // RW
    kOSDTRTX_EL1          = encode(0b10, 0b000, 0b0000, 0b0011, 0b010), // RW
    kOSECCR_EL1           = encode(0b10, 0b000, 0b0000, 0b0110, 0b010), // RW
    kOSLAR_EL1            = encode(0b10, 0b000, 0b0001, 0b0000, 0b100), // WO
    kOSLSR_EL1            = encode(0b10, 0b000, 0b0001, 0b0001, 0b100), // RO
    kPAN                  = encode(0b11, 0b000, 0b0100, 0b0010, 0b011), // RW
    kPAR_EL1              = encode(0b11, 0b000, 0b0111, 0b0100, 0b000), // RW
    kPMBIDR_EL1           = encode(0b11, 0b000, 0b1001, 0b1010, 0b111), // RO
    kPMBLIMITR_EL1        = encode(0b11, 0b000, 0b1001, 0b1010, 0b000), // RW
    kPMBPTR_EL1           = encode(0b11, 0b000, 0b1001, 0b1010, 0b001), // RW
    kPMBSR_EL1            = encode(0b11, 0b000, 0b1001, 0b1010, 0b011), // RW
    kPMCCFILTR_EL0        = encode(0b11, 0b011, 0b1110, 0b1111, 0b111), // RW
    kPMCCNTR_EL0          = encode(0b11, 0b011, 0b1001, 0b1101, 0b000), // RW
    kPMCEID0_EL0          = encode(0b11, 0b011, 0b1001, 0b1100, 0b110), // RO
    kPMCEID1_EL0          = encode(0b11, 0b011, 0b1001, 0b1100, 0b111), // RO
    kPMCNTENCLR_EL0       = encode(0b11, 0b011, 0b1001, 0b1100, 0b010), // RW
    kPMCNTENSET_EL0       = encode(0b11, 0b011, 0b1001, 0b1100, 0b001), // RW
    kPMCR_EL0             = encode(0b11, 0b011, 0b1001, 0b1100, 0b000), // RW
    kPMEVCNTR0_EL0        = encode(0b11, 0b011, 0b1110, 0b1000, 0b000), // RW
    kPMEVCNTR10_EL0       = encode(0b11, 0b011, 0b1110, 0b1001, 0b010), // RW
    kPMEVCNTR11_EL0       = encode(0b11, 0b011, 0b1110, 0b1001, 0b011), // RW
    kPMEVCNTR12_EL0       = encode(0b11, 0b011, 0b1110, 0b1001, 0b100), // RW
    kPMEVCNTR13_EL0       = encode(0b11, 0b011, 0b1110, 0b1001, 0b101), // RW
    kPMEVCNTR14_EL0       = encode(0b11, 0b011, 0b1110, 0b1001, 0b110), // RW
    kPMEVCNTR15_EL0       = encode(0b11, 0b011, 0b1110, 0b1001, 0b111), // RW
    kPMEVCNTR16_EL0       = encode(0b11, 0b011, 0b1110, 0b1010, 0b000), // RW
    kPMEVCNTR17_EL0       = encode(0b11, 0b011, 0b1110, 0b1010, 0b001), // RW
    kPMEVCNTR18_EL0       = encode(0b11, 0b011, 0b1110, 0b1010, 0b010), // RW
    kPMEVCNTR19_EL0       = encode(0b11, 0b011, 0b1110, 0b1010, 0b011), // RW
    kPMEVCNTR1_EL0        = encode(0b11, 0b011, 0b1110, 0b1000, 0b001), // RW
    kPMEVCNTR20_EL0       = encode(0b11, 0b011, 0b1110, 0b1010, 0b100), // RW
    kPMEVCNTR21_EL0       = encode(0b11, 0b011, 0b1110, 0b1010, 0b101), // RW
    kPMEVCNTR22_EL0       = encode(0b11, 0b011, 0b1110, 0b1010, 0b110), // RW
    kPMEVCNTR23_EL0       = encode(0b11, 0b011, 0b1110, 0b1010, 0b111), // RW
    kPMEVCNTR24_EL0       = encode(0b11, 0b011, 0b1110, 0b1011, 0b000), // RW
    kPMEVCNTR25_EL0       = encode(0b11, 0b011, 0b1110, 0b1011, 0b001), // RW
    kPMEVCNTR26_EL0       = encode(0b11, 0b011, 0b1110, 0b1011, 0b010), // RW
    kPMEVCNTR27_EL0       = encode(0b11, 0b011, 0b1110, 0b1011, 0b011), // RW
    kPMEVCNTR28_EL0       = encode(0b11, 0b011, 0b1110, 0b1011, 0b100), // RW
    kPMEVCNTR29_EL0       = encode(0b11, 0b011, 0b1110, 0b1011, 0b101), // RW
    kPMEVCNTR2_EL0        = encode(0b11, 0b011, 0b1110, 0b1000, 0b010), // RW
    kPMEVCNTR30_EL0       = encode(0b11, 0b011, 0b1110, 0b1011, 0b110), // RW
    kPMEVCNTR3_EL0        = encode(0b11, 0b011, 0b1110, 0b1000, 0b011), // RW
    kPMEVCNTR4_EL0        = encode(0b11, 0b011, 0b1110, 0b1000, 0b100), // RW
    kPMEVCNTR5_EL0        = encode(0b11, 0b011, 0b1110, 0b1000, 0b101), // RW
    kPMEVCNTR6_EL0        = encode(0b11, 0b011, 0b1110, 0b1000, 0b110), // RW
    kPMEVCNTR7_EL0        = encode(0b11, 0b011, 0b1110, 0b1000, 0b111), // RW
    kPMEVCNTR8_EL0        = encode(0b11, 0b011, 0b1110, 0b1001, 0b000), // RW
    kPMEVCNTR9_EL0        = encode(0b11, 0b011, 0b1110, 0b1001, 0b001), // RW
    kPMEVTYPER0_EL0       = encode(0b11, 0b011, 0b1110, 0b1100, 0b000), // RW
    kPMEVTYPER10_EL0      = encode(0b11, 0b011, 0b1110, 0b1101, 0b010), // RW
    kPMEVTYPER11_EL0      = encode(0b11, 0b011, 0b1110, 0b1101, 0b011), // RW
    kPMEVTYPER12_EL0      = encode(0b11, 0b011, 0b1110, 0b1101, 0b100), // RW
    kPMEVTYPER13_EL0      = encode(0b11, 0b011, 0b1110, 0b1101, 0b101), // RW
    kPMEVTYPER14_EL0      = encode(0b11, 0b011, 0b1110, 0b1101, 0b110), // RW
    kPMEVTYPER15_EL0      = encode(0b11, 0b011, 0b1110, 0b1101, 0b111), // RW
    kPMEVTYPER16_EL0      = encode(0b11, 0b011, 0b1110, 0b1110, 0b000), // RW
    kPMEVTYPER17_EL0      = encode(0b11, 0b011, 0b1110, 0b1110, 0b001), // RW
    kPMEVTYPER18_EL0      = encode(0b11, 0b011, 0b1110, 0b1110, 0b010), // RW
    kPMEVTYPER19_EL0      = encode(0b11, 0b011, 0b1110, 0b1110, 0b011), // RW
    kPMEVTYPER1_EL0       = encode(0b11, 0b011, 0b1110, 0b1100, 0b001), // RW
    kPMEVTYPER20_EL0      = encode(0b11, 0b011, 0b1110, 0b1110, 0b100), // RW
    kPMEVTYPER21_EL0      = encode(0b11, 0b011, 0b1110, 0b1110, 0b101), // RW
    kPMEVTYPER22_EL0      = encode(0b11, 0b011, 0b1110, 0b1110, 0b110), // RW
    kPMEVTYPER23_EL0      = encode(0b11, 0b011, 0b1110, 0b1110, 0b111), // RW
    kPMEVTYPER24_EL0      = encode(0b11, 0b011, 0b1110, 0b1111, 0b000), // RW
    kPMEVTYPER25_EL0      = encode(0b11, 0b011, 0b1110, 0b1111, 0b001), // RW
    kPMEVTYPER26_EL0      = encode(0b11, 0b011, 0b1110, 0b1111, 0b010), // RW
    kPMEVTYPER27_EL0      = encode(0b11, 0b011, 0b1110, 0b1111, 0b011), // RW
    kPMEVTYPER28_EL0      = encode(0b11, 0b011, 0b1110, 0b1111, 0b100), // RW
    kPMEVTYPER29_EL0      = encode(0b11, 0b011, 0b1110, 0b1111, 0b101), // RW
    kPMEVTYPER2_EL0       = encode(0b11, 0b011, 0b1110, 0b1100, 0b010), // RW
    kPMEVTYPER30_EL0      = encode(0b11, 0b011, 0b1110, 0b1111, 0b110), // RW
    kPMEVTYPER3_EL0       = encode(0b11, 0b011, 0b1110, 0b1100, 0b011), // RW
    kPMEVTYPER4_EL0       = encode(0b11, 0b011, 0b1110, 0b1100, 0b100), // RW
    kPMEVTYPER5_EL0       = encode(0b11, 0b011, 0b1110, 0b1100, 0b101), // RW
    kPMEVTYPER6_EL0       = encode(0b11, 0b011, 0b1110, 0b1100, 0b110), // RW
    kPMEVTYPER7_EL0       = encode(0b11, 0b011, 0b1110, 0b1100, 0b111), // RW
    kPMEVTYPER8_EL0       = encode(0b11, 0b011, 0b1110, 0b1101, 0b000), // RW
    kPMEVTYPER9_EL0       = encode(0b11, 0b011, 0b1110, 0b1101, 0b001), // RW
    kPMINTENCLR_EL1       = encode(0b11, 0b000, 0b1001, 0b1110, 0b010), // RW
    kPMINTENSET_EL1       = encode(0b11, 0b000, 0b1001, 0b1110, 0b001), // RW
    kPMMIR_EL1            = encode(0b11, 0b000, 0b1001, 0b1110, 0b110), // RW
    kPMOVSCLR_EL0         = encode(0b11, 0b011, 0b1001, 0b1100, 0b011), // RW
    kPMOVSSET_EL0         = encode(0b11, 0b011, 0b1001, 0b1110, 0b011), // RW
    kPMSCR_EL1            = encode(0b11, 0b000, 0b1001, 0b1001, 0b000), // RW
    kPMSCR_EL12           = encode(0b11, 0b101, 0b1001, 0b1001, 0b000), // RW
    kPMSCR_EL2            = encode(0b11, 0b100, 0b1001, 0b1001, 0b000), // RW
    kPMSELR_EL0           = encode(0b11, 0b011, 0b1001, 0b1100, 0b101), // RW
    kPMSEVFR_EL1          = encode(0b11, 0b000, 0b1001, 0b1001, 0b101), // RW
    kPMSFCR_EL1           = encode(0b11, 0b000, 0b1001, 0b1001, 0b100), // RW
    kPMSICR_EL1           = encode(0b11, 0b000, 0b1001, 0b1001, 0b010), // RW
    kPMSIDR_EL1           = encode(0b11, 0b000, 0b1001, 0b1001, 0b111), // RO
    kPMSIRR_EL1           = encode(0b11, 0b000, 0b1001, 0b1001, 0b011), // RW
    kPMSLATFR_EL1         = encode(0b11, 0b000, 0b1001, 0b1001, 0b110), // RW
    kPMSWINC_EL0          = encode(0b11, 0b011, 0b1001, 0b1100, 0b100), // WO
    kPMUSERENR_EL0        = encode(0b11, 0b011, 0b1001, 0b1110, 0b000), // RW
    kPMXEVCNTR_EL0        = encode(0b11, 0b011, 0b1001, 0b1101, 0b010), // RW
    kPMXEVTYPER_EL0       = encode(0b11, 0b011, 0b1001, 0b1101, 0b001), // RW
    kREVIDR_EL1           = encode(0b11, 0b000, 0b0000, 0b0000, 0b110), // RO
    kRGSR_EL1             = encode(0b11, 0b000, 0b0001, 0b0000, 0b101), // RW
    kRMR_EL1              = encode(0b11, 0b000, 0b1100, 0b0000, 0b010), // RW
    kRMR_EL2              = encode(0b11, 0b100, 0b1100, 0b0000, 0b010), // RW
    kRMR_EL3              = encode(0b11, 0b110, 0b1100, 0b0000, 0b010), // RW
    kRNDR                 = encode(0b11, 0b011, 0b0010, 0b0100, 0b000), // RO
    kRNDRRS               = encode(0b11, 0b011, 0b0010, 0b0100, 0b001), // RO
    kRVBAR_EL1            = encode(0b11, 0b000, 0b1100, 0b0000, 0b001), // RO
    kRVBAR_EL2            = encode(0b11, 0b100, 0b1100, 0b0000, 0b001), // RO
    kRVBAR_EL3            = encode(0b11, 0b110, 0b1100, 0b0000, 0b001), // RO
    kSCR_EL3              = encode(0b11, 0b110, 0b0001, 0b0001, 0b000), // RW
    kSCTLR_EL1            = encode(0b11, 0b000, 0b0001, 0b0000, 0b000), // RW
    kSCTLR_EL12           = encode(0b11, 0b101, 0b0001, 0b0000, 0b000), // RW
    kSCTLR_EL2            = encode(0b11, 0b100, 0b0001, 0b0000, 0b000), // RW
    kSCTLR_EL3            = encode(0b11, 0b110, 0b0001, 0b0000, 0b000), // RW
    kSCXTNUM_EL0          = encode(0b11, 0b011, 0b1101, 0b0000, 0b111), // RW
    kSCXTNUM_EL1          = encode(0b11, 0b000, 0b1101, 0b0000, 0b111), // RW
    kSCXTNUM_EL12         = encode(0b11, 0b101, 0b1101, 0b0000, 0b111), // RW
    kSCXTNUM_EL2          = encode(0b11, 0b100, 0b1101, 0b0000, 0b111), // RW
    kSCXTNUM_EL3          = encode(0b11, 0b110, 0b1101, 0b0000, 0b111), // RW
    kSDER32_EL2           = encode(0b11, 0b100, 0b0001, 0b0011, 0b001), // RW
    kSDER32_EL3           = encode(0b11, 0b110, 0b0001, 0b0001, 0b001), // RW
    kSPSR_EL1             = encode(0b11, 0b000, 0b0100, 0b0000, 0b000), // RW
    kSPSR_EL12            = encode(0b11, 0b101, 0b0100, 0b0000, 0b000), // RW
    kSPSR_EL2             = encode(0b11, 0b100, 0b0100, 0b0000, 0b000), // RW
    kSPSR_EL3             = encode(0b11, 0b110, 0b0100, 0b0000, 0b000), // RW
    kSPSR_abt             = encode(0b11, 0b100, 0b0100, 0b0011, 0b001), // RW
    kSPSR_fiq             = encode(0b11, 0b100, 0b0100, 0b0011, 0b011), // RW
    kSPSR_irq             = encode(0b11, 0b100, 0b0100, 0b0011, 0b000), // RW
    kSPSR_und             = encode(0b11, 0b100, 0b0100, 0b0011, 0b010), // RW
    kSPSel                = encode(0b11, 0b000, 0b0100, 0b0010, 0b000), // RW
    kSP_EL0               = encode(0b11, 0b000, 0b0100, 0b0001, 0b000), // RW
    kSP_EL1               = encode(0b11, 0b100, 0b0100, 0b0001, 0b000), // RW
    kSP_EL2               = encode(0b11, 0b110, 0b0100, 0b0001, 0b000), // RW
    kSSBS                 = encode(0b11, 0b011, 0b0100, 0b0010, 0b110), // RW
    kTCO                  = encode(0b11, 0b011, 0b0100, 0b0010, 0b111), // RW
    kTCR_EL1              = encode(0b11, 0b000, 0b0010, 0b0000, 0b010), // RW
    kTCR_EL12             = encode(0b11, 0b101, 0b0010, 0b0000, 0b010), // RW
    kTCR_EL2              = encode(0b11, 0b100, 0b0010, 0b0000, 0b010), // RW
    kTCR_EL3              = encode(0b11, 0b110, 0b0010, 0b0000, 0b010), // RW
    kTEECR32_EL1          = encode(0b10, 0b010, 0b0000, 0b0000, 0b000), // RW
    kTEEHBR32_EL1         = encode(0b10, 0b010, 0b0001, 0b0000, 0b000), // RW
    kTFSRE0_EL1           = encode(0b11, 0b000, 0b0101, 0b0110, 0b001), // RW
    kTFSR_EL1             = encode(0b11, 0b000, 0b0101, 0b0110, 0b000), // RW
    kTFSR_EL12            = encode(0b11, 0b101, 0b0101, 0b0110, 0b000), // RW
    kTFSR_EL2             = encode(0b11, 0b100, 0b0101, 0b0110, 0b000), // RW
    kTFSR_EL3             = encode(0b11, 0b110, 0b0101, 0b0110, 0b000), // RW
    kTPIDRRO_EL0          = encode(0b11, 0b011, 0b1101, 0b0000, 0b011), // RW
    kTPIDR_EL0            = encode(0b11, 0b011, 0b1101, 0b0000, 0b010), // RW
    kTPIDR_EL1            = encode(0b11, 0b000, 0b1101, 0b0000, 0b100), // RW
    kTPIDR_EL2            = encode(0b11, 0b100, 0b1101, 0b0000, 0b010), // RW
    kTPIDR_EL3            = encode(0b11, 0b110, 0b1101, 0b0000, 0b010), // RW
    kTRBBASER_EL1         = encode(0b11, 0b000, 0b1001, 0b1011, 0b010), // RW
    kTRBIDR_EL1           = encode(0b11, 0b000, 0b1001, 0b1011, 0b111), // RO
    kTRBLIMITR_EL1        = encode(0b11, 0b000, 0b1001, 0b1011, 0b000), // RW
    kTRBMAR_EL1           = encode(0b11, 0b000, 0b1001, 0b1011, 0b100), // RW
    kTRBPTR_EL1           = encode(0b11, 0b000, 0b1001, 0b1011, 0b001), // RW
    kTRBSR_EL1            = encode(0b11, 0b000, 0b1001, 0b1011, 0b011), // RW
    kTRBTRG_EL1           = encode(0b11, 0b000, 0b1001, 0b1011, 0b110), // RW
    kTRCACATR0            = encode(0b10, 0b001, 0b0010, 0b0000, 0b010), // RW
    kTRCACATR1            = encode(0b10, 0b001, 0b0010, 0b0010, 0b010), // RW
    kTRCACATR10           = encode(0b10, 0b001, 0b0010, 0b0100, 0b011), // RW
    kTRCACATR11           = encode(0b10, 0b001, 0b0010, 0b0110, 0b011), // RW
    kTRCACATR12           = encode(0b10, 0b001, 0b0010, 0b1000, 0b011), // RW
    kTRCACATR13           = encode(0b10, 0b001, 0b0010, 0b1010, 0b011), // RW
    kTRCACATR14           = encode(0b10, 0b001, 0b0010, 0b1100, 0b011), // RW
    kTRCACATR15           = encode(0b10, 0b001, 0b0010, 0b1110, 0b011), // RW
    kTRCACATR2            = encode(0b10, 0b001, 0b0010, 0b0100, 0b010), // RW
    kTRCACATR3            = encode(0b10, 0b001, 0b0010, 0b0110, 0b010), // RW
    kTRCACATR4            = encode(0b10, 0b001, 0b0010, 0b1000, 0b010), // RW
    kTRCACATR5            = encode(0b10, 0b001, 0b0010, 0b1010, 0b010), // RW
    kTRCACATR6            = encode(0b10, 0b001, 0b0010, 0b1100, 0b010), // RW
    kTRCACATR7            = encode(0b10, 0b001, 0b0010, 0b1110, 0b010), // RW
    kTRCACATR8            = encode(0b10, 0b001, 0b0010, 0b0000, 0b011), // RW
    kTRCACATR9            = encode(0b10, 0b001, 0b0010, 0b0010, 0b011), // RW
    kTRCACVR0             = encode(0b10, 0b001, 0b0010, 0b0000, 0b000), // RW
    kTRCACVR1             = encode(0b10, 0b001, 0b0010, 0b0010, 0b000), // RW
    kTRCACVR10            = encode(0b10, 0b001, 0b0010, 0b0100, 0b001), // RW
    kTRCACVR11            = encode(0b10, 0b001, 0b0010, 0b0110, 0b001), // RW
    kTRCACVR12            = encode(0b10, 0b001, 0b0010, 0b1000, 0b001), // RW
    kTRCACVR13            = encode(0b10, 0b001, 0b0010, 0b1010, 0b001), // RW
    kTRCACVR14            = encode(0b10, 0b001, 0b0010, 0b1100, 0b001), // RW
    kTRCACVR15            = encode(0b10, 0b001, 0b0010, 0b1110, 0b001), // RW
    kTRCACVR2             = encode(0b10, 0b001, 0b0010, 0b0100, 0b000), // RW
    kTRCACVR3             = encode(0b10, 0b001, 0b0010, 0b0110, 0b000), // RW
    kTRCACVR4             = encode(0b10, 0b001, 0b0010, 0b1000, 0b000), // RW
    kTRCACVR5             = encode(0b10, 0b001, 0b0010, 0b1010, 0b000), // RW
    kTRCACVR6             = encode(0b10, 0b001, 0b0010, 0b1100, 0b000), // RW
    kTRCACVR7             = encode(0b10, 0b001, 0b0010, 0b1110, 0b000), // RW
    kTRCACVR8             = encode(0b10, 0b001, 0b0010, 0b0000, 0b001), // RW
    kTRCACVR9             = encode(0b10, 0b001, 0b0010, 0b0010, 0b001), // RW
    kTRCAUTHSTATUS        = encode(0b10, 0b001, 0b0111, 0b1110, 0b110), // RO
    kTRCAUXCTLR           = encode(0b10, 0b001, 0b0000, 0b0110, 0b000), // RW
    kTRCBBCTLR            = encode(0b10, 0b001, 0b0000, 0b1111, 0b000), // RW
    kTRCCCCTLR            = encode(0b10, 0b001, 0b0000, 0b1110, 0b000), // RW
    kTRCCIDCCTLR0         = encode(0b10, 0b001, 0b0011, 0b0000, 0b010), // RW
    kTRCCIDCCTLR1         = encode(0b10, 0b001, 0b0011, 0b0001, 0b010), // RW
    kTRCCIDCVR0           = encode(0b10, 0b001, 0b0011, 0b0000, 0b000), // RW
    kTRCCIDCVR1           = encode(0b10, 0b001, 0b0011, 0b0010, 0b000), // RW
    kTRCCIDCVR2           = encode(0b10, 0b001, 0b0011, 0b0100, 0b000), // RW
    kTRCCIDCVR3           = encode(0b10, 0b001, 0b0011, 0b0110, 0b000), // RW
    kTRCCIDCVR4           = encode(0b10, 0b001, 0b0011, 0b1000, 0b000), // RW
    kTRCCIDCVR5           = encode(0b10, 0b001, 0b0011, 0b1010, 0b000), // RW
    kTRCCIDCVR6           = encode(0b10, 0b001, 0b0011, 0b1100, 0b000), // RW
    kTRCCIDCVR7           = encode(0b10, 0b001, 0b0011, 0b1110, 0b000), // RW
    kTRCCIDR0             = encode(0b10, 0b001, 0b0111, 0b1100, 0b111), // RO
    kTRCCIDR1             = encode(0b10, 0b001, 0b0111, 0b1101, 0b111), // RO
    kTRCCIDR2             = encode(0b10, 0b001, 0b0111, 0b1110, 0b111), // RO
    kTRCCIDR3             = encode(0b10, 0b001, 0b0111, 0b1111, 0b111), // RO
    kTRCCLAIMCLR          = encode(0b10, 0b001, 0b0111, 0b1001, 0b110), // RW
    kTRCCLAIMSET          = encode(0b10, 0b001, 0b0111, 0b1000, 0b110), // RW
    kTRCCNTCTLR0          = encode(0b10, 0b001, 0b0000, 0b0100, 0b101), // RW
    kTRCCNTCTLR1          = encode(0b10, 0b001, 0b0000, 0b0101, 0b101), // RW
    kTRCCNTCTLR2          = encode(0b10, 0b001, 0b0000, 0b0110, 0b101), // RW
    kTRCCNTCTLR3          = encode(0b10, 0b001, 0b0000, 0b0111, 0b101), // RW
    kTRCCNTRLDVR0         = encode(0b10, 0b001, 0b0000, 0b0000, 0b101), // RW
    kTRCCNTRLDVR1         = encode(0b10, 0b001, 0b0000, 0b0001, 0b101), // RW
    kTRCCNTRLDVR2         = encode(0b10, 0b001, 0b0000, 0b0010, 0b101), // RW
    kTRCCNTRLDVR3         = encode(0b10, 0b001, 0b0000, 0b0011, 0b101), // RW
    kTRCCNTVR0            = encode(0b10, 0b001, 0b0000, 0b1000, 0b101), // RW
    kTRCCNTVR1            = encode(0b10, 0b001, 0b0000, 0b1001, 0b101), // RW
    kTRCCNTVR2            = encode(0b10, 0b001, 0b0000, 0b1010, 0b101), // RW
    kTRCCNTVR3            = encode(0b10, 0b001, 0b0000, 0b1011, 0b101), // RW
    kTRCCONFIGR           = encode(0b10, 0b001, 0b0000, 0b0100, 0b000), // RW
    kTRCDEVAFF0           = encode(0b10, 0b001, 0b0111, 0b1010, 0b110), // RO
    kTRCDEVAFF1           = encode(0b10, 0b001, 0b0111, 0b1011, 0b110), // RO
    kTRCDEVARCH           = encode(0b10, 0b001, 0b0111, 0b1111, 0b110), // RO
    kTRCDEVID             = encode(0b10, 0b001, 0b0111, 0b0010, 0b111), // RO
    kTRCDEVTYPE           = encode(0b10, 0b001, 0b0111, 0b0011, 0b111), // RO
    kTRCDVCMR0            = encode(0b10, 0b001, 0b0010, 0b0000, 0b110), // RW
    kTRCDVCMR1            = encode(0b10, 0b001, 0b0010, 0b0100, 0b110), // RW
    kTRCDVCMR2            = encode(0b10, 0b001, 0b0010, 0b1000, 0b110), // RW
    kTRCDVCMR3            = encode(0b10, 0b001, 0b0010, 0b1100, 0b110), // RW
    kTRCDVCMR4            = encode(0b10, 0b001, 0b0010, 0b0000, 0b111), // RW
    kTRCDVCMR5            = encode(0b10, 0b001, 0b0010, 0b0100, 0b111), // RW
    kTRCDVCMR6            = encode(0b10, 0b001, 0b0010, 0b1000, 0b111), // RW
    kTRCDVCMR7            = encode(0b10, 0b001, 0b0010, 0b1100, 0b111), // RW
    kTRCDVCVR0            = encode(0b10, 0b001, 0b0010, 0b0000, 0b100), // RW
    kTRCDVCVR1            = encode(0b10, 0b001, 0b0010, 0b0100, 0b100), // RW
    kTRCDVCVR2            = encode(0b10, 0b001, 0b0010, 0b1000, 0b100), // RW
    kTRCDVCVR3            = encode(0b10, 0b001, 0b0010, 0b1100, 0b100), // RW
    kTRCDVCVR4            = encode(0b10, 0b001, 0b0010, 0b0000, 0b101), // RW
    kTRCDVCVR5            = encode(0b10, 0b001, 0b0010, 0b0100, 0b101), // RW
    kTRCDVCVR6            = encode(0b10, 0b001, 0b0010, 0b1000, 0b101), // RW
    kTRCDVCVR7            = encode(0b10, 0b001, 0b0010, 0b1100, 0b101), // RW
    kTRCEVENTCTL0R        = encode(0b10, 0b001, 0b0000, 0b1000, 0b000), // RW
    kTRCEVENTCTL1R        = encode(0b10, 0b001, 0b0000, 0b1001, 0b000), // RW
    kTRCEXTINSELR         = encode(0b10, 0b001, 0b0000, 0b1000, 0b100), // RW
    kTRCEXTINSELR0        = encode(0b10, 0b001, 0b0000, 0b1000, 0b100), // RW
    kTRCEXTINSELR1        = encode(0b10, 0b001, 0b0000, 0b1001, 0b100), // RW
    kTRCEXTINSELR2        = encode(0b10, 0b001, 0b0000, 0b1010, 0b100), // RW
    kTRCEXTINSELR3        = encode(0b10, 0b001, 0b0000, 0b1011, 0b100), // RW
    kTRCIDR0              = encode(0b10, 0b001, 0b0000, 0b1000, 0b111), // RO
    kTRCIDR1              = encode(0b10, 0b001, 0b0000, 0b1001, 0b111), // RO
    kTRCIDR10             = encode(0b10, 0b001, 0b0000, 0b0010, 0b110), // RO
    kTRCIDR11             = encode(0b10, 0b001, 0b0000, 0b0011, 0b110), // RO
    kTRCIDR12             = encode(0b10, 0b001, 0b0000, 0b0100, 0b110), // RO
    kTRCIDR13             = encode(0b10, 0b001, 0b0000, 0b0101, 0b110), // RO
    kTRCIDR2              = encode(0b10, 0b001, 0b0000, 0b1010, 0b111), // RO
    kTRCIDR3              = encode(0b10, 0b001, 0b0000, 0b1011, 0b111), // RO
    kTRCIDR4              = encode(0b10, 0b001, 0b0000, 0b1100, 0b111), // RO
    kTRCIDR5              = encode(0b10, 0b001, 0b0000, 0b1101, 0b111), // RO
    kTRCIDR6              = encode(0b10, 0b001, 0b0000, 0b1110, 0b111), // RO
    kTRCIDR7              = encode(0b10, 0b001, 0b0000, 0b1111, 0b111), // RO
    kTRCIDR8              = encode(0b10, 0b001, 0b0000, 0b0000, 0b110), // RO
    kTRCIDR9              = encode(0b10, 0b001, 0b0000, 0b0001, 0b110), // RO
    kTRCIMSPEC0           = encode(0b10, 0b001, 0b0000, 0b0000, 0b111), // RW
    kTRCIMSPEC1           = encode(0b10, 0b001, 0b0000, 0b0001, 0b111), // RW
    kTRCIMSPEC2           = encode(0b10, 0b001, 0b0000, 0b0010, 0b111), // RW
    kTRCIMSPEC3           = encode(0b10, 0b001, 0b0000, 0b0011, 0b111), // RW
    kTRCIMSPEC4           = encode(0b10, 0b001, 0b0000, 0b0100, 0b111), // RW
    kTRCIMSPEC5           = encode(0b10, 0b001, 0b0000, 0b0101, 0b111), // RW
    kTRCIMSPEC6           = encode(0b10, 0b001, 0b0000, 0b0110, 0b111), // RW
    kTRCIMSPEC7           = encode(0b10, 0b001, 0b0000, 0b0111, 0b111), // RW
    kTRCITCTRL            = encode(0b10, 0b001, 0b0111, 0b0000, 0b100), // RW
    kTRCLAR               = encode(0b10, 0b001, 0b0111, 0b1100, 0b110), // WO
    kTRCLSR               = encode(0b10, 0b001, 0b0111, 0b1101, 0b110), // RO
    kTRCOSLAR             = encode(0b10, 0b001, 0b0001, 0b0000, 0b100), // WO
    kTRCOSLSR             = encode(0b10, 0b001, 0b0001, 0b0001, 0b100), // RO
    kTRCPDCR              = encode(0b10, 0b001, 0b0001, 0b0100, 0b100), // RW
    kTRCPDSR              = encode(0b10, 0b001, 0b0001, 0b0101, 0b100), // RO
    kTRCPIDR0             = encode(0b10, 0b001, 0b0111, 0b1000, 0b111), // RO
    kTRCPIDR1             = encode(0b10, 0b001, 0b0111, 0b1001, 0b111), // RO
    kTRCPIDR2             = encode(0b10, 0b001, 0b0111, 0b1010, 0b111), // RO
    kTRCPIDR3             = encode(0b10, 0b001, 0b0111, 0b1011, 0b111), // RO
    kTRCPIDR4             = encode(0b10, 0b001, 0b0111, 0b0100, 0b111), // RO
    kTRCPIDR5             = encode(0b10, 0b001, 0b0111, 0b0101, 0b111), // RO
    kTRCPIDR6             = encode(0b10, 0b001, 0b0111, 0b0110, 0b111), // RO
    kTRCPIDR7             = encode(0b10, 0b001, 0b0111, 0b0111, 0b111), // RO
    kTRCPRGCTLR           = encode(0b10, 0b001, 0b0000, 0b0001, 0b000), // RW
    kTRCPROCSELR          = encode(0b10, 0b001, 0b0000, 0b0010, 0b000), // RW
    kTRCQCTLR             = encode(0b10, 0b001, 0b0000, 0b0001, 0b001), // RW
    kTRCRSCTLR10          = encode(0b10, 0b001, 0b0001, 0b1010, 0b000), // RW
    kTRCRSCTLR11          = encode(0b10, 0b001, 0b0001, 0b1011, 0b000), // RW
    kTRCRSCTLR12          = encode(0b10, 0b001, 0b0001, 0b1100, 0b000), // RW
    kTRCRSCTLR13          = encode(0b10, 0b001, 0b0001, 0b1101, 0b000), // RW
    kTRCRSCTLR14          = encode(0b10, 0b001, 0b0001, 0b1110, 0b000), // RW
    kTRCRSCTLR15          = encode(0b10, 0b001, 0b0001, 0b1111, 0b000), // RW
    kTRCRSCTLR16          = encode(0b10, 0b001, 0b0001, 0b0000, 0b001), // RW
    kTRCRSCTLR17          = encode(0b10, 0b001, 0b0001, 0b0001, 0b001), // RW
    kTRCRSCTLR18          = encode(0b10, 0b001, 0b0001, 0b0010, 0b001), // RW
    kTRCRSCTLR19          = encode(0b10, 0b001, 0b0001, 0b0011, 0b001), // RW
    kTRCRSCTLR2           = encode(0b10, 0b001, 0b0001, 0b0010, 0b000), // RW
    kTRCRSCTLR20          = encode(0b10, 0b001, 0b0001, 0b0100, 0b001), // RW
    kTRCRSCTLR21          = encode(0b10, 0b001, 0b0001, 0b0101, 0b001), // RW
    kTRCRSCTLR22          = encode(0b10, 0b001, 0b0001, 0b0110, 0b001), // RW
    kTRCRSCTLR23          = encode(0b10, 0b001, 0b0001, 0b0111, 0b001), // RW
    kTRCRSCTLR24          = encode(0b10, 0b001, 0b0001, 0b1000, 0b001), // RW
    kTRCRSCTLR25          = encode(0b10, 0b001, 0b0001, 0b1001, 0b001), // RW
    kTRCRSCTLR26          = encode(0b10, 0b001, 0b0001, 0b1010, 0b001), // RW
    kTRCRSCTLR27          = encode(0b10, 0b001, 0b0001, 0b1011, 0b001), // RW
    kTRCRSCTLR28          = encode(0b10, 0b001, 0b0001, 0b1100, 0b001), // RW
    kTRCRSCTLR29          = encode(0b10, 0b001, 0b0001, 0b1101, 0b001), // RW
    kTRCRSCTLR3           = encode(0b10, 0b001, 0b0001, 0b0011, 0b000), // RW
    kTRCRSCTLR30          = encode(0b10, 0b001, 0b0001, 0b1110, 0b001), // RW
    kTRCRSCTLR31          = encode(0b10, 0b001, 0b0001, 0b1111, 0b001), // RW
    kTRCRSCTLR4           = encode(0b10, 0b001, 0b0001, 0b0100, 0b000), // RW
    kTRCRSCTLR5           = encode(0b10, 0b001, 0b0001, 0b0101, 0b000), // RW
    kTRCRSCTLR6           = encode(0b10, 0b001, 0b0001, 0b0110, 0b000), // RW
    kTRCRSCTLR7           = encode(0b10, 0b001, 0b0001, 0b0111, 0b000), // RW
    kTRCRSCTLR8           = encode(0b10, 0b001, 0b0001, 0b1000, 0b000), // RW
    kTRCRSCTLR9           = encode(0b10, 0b001, 0b0001, 0b1001, 0b000), // RW
    kTRCRSR               = encode(0b10, 0b001, 0b0000, 0b1010, 0b000), // RW
    kTRCSEQEVR0           = encode(0b10, 0b001, 0b0000, 0b0000, 0b100), // RW
    kTRCSEQEVR1           = encode(0b10, 0b001, 0b0000, 0b0001, 0b100), // RW
    kTRCSEQEVR2           = encode(0b10, 0b001, 0b0000, 0b0010, 0b100), // RW
    kTRCSEQRSTEVR         = encode(0b10, 0b001, 0b0000, 0b0110, 0b100), // RW
    kTRCSEQSTR            = encode(0b10, 0b001, 0b0000, 0b0111, 0b100), // RW
    kTRCSSCCR0            = encode(0b10, 0b001, 0b0001, 0b0000, 0b010), // RW
    kTRCSSCCR1            = encode(0b10, 0b001, 0b0001, 0b0001, 0b010), // RW
    kTRCSSCCR2            = encode(0b10, 0b001, 0b0001, 0b0010, 0b010), // RW
    kTRCSSCCR3            = encode(0b10, 0b001, 0b0001, 0b0011, 0b010), // RW
    kTRCSSCCR4            = encode(0b10, 0b001, 0b0001, 0b0100, 0b010), // RW
    kTRCSSCCR5            = encode(0b10, 0b001, 0b0001, 0b0101, 0b010), // RW
    kTRCSSCCR6            = encode(0b10, 0b001, 0b0001, 0b0110, 0b010), // RW
    kTRCSSCCR7            = encode(0b10, 0b001, 0b0001, 0b0111, 0b010), // RW
    kTRCSSCSR0            = encode(0b10, 0b001, 0b0001, 0b1000, 0b010), // RW
    kTRCSSCSR1            = encode(0b10, 0b001, 0b0001, 0b1001, 0b010), // RW
    kTRCSSCSR2            = encode(0b10, 0b001, 0b0001, 0b1010, 0b010), // RW
    kTRCSSCSR3            = encode(0b10, 0b001, 0b0001, 0b1011, 0b010), // RW
    kTRCSSCSR4            = encode(0b10, 0b001, 0b0001, 0b1100, 0b010), // RW
    kTRCSSCSR5            = encode(0b10, 0b001, 0b0001, 0b1101, 0b010), // RW
    kTRCSSCSR6            = encode(0b10, 0b001, 0b0001, 0b1110, 0b010), // RW
    kTRCSSCSR7            = encode(0b10, 0b001, 0b0001, 0b1111, 0b010), // RW
    kTRCSSPCICR0          = encode(0b10, 0b001, 0b0001, 0b0000, 0b011), // RW
    kTRCSSPCICR1          = encode(0b10, 0b001, 0b0001, 0b0001, 0b011), // RW
    kTRCSSPCICR2          = encode(0b10, 0b001, 0b0001, 0b0010, 0b011), // RW
    kTRCSSPCICR3          = encode(0b10, 0b001, 0b0001, 0b0011, 0b011), // RW
    kTRCSSPCICR4          = encode(0b10, 0b001, 0b0001, 0b0100, 0b011), // RW
    kTRCSSPCICR5          = encode(0b10, 0b001, 0b0001, 0b0101, 0b011), // RW
    kTRCSSPCICR6          = encode(0b10, 0b001, 0b0001, 0b0110, 0b011), // RW
    kTRCSSPCICR7          = encode(0b10, 0b001, 0b0001, 0b0111, 0b011), // RW
    kTRCSTALLCTLR         = encode(0b10, 0b001, 0b0000, 0b1011, 0b000), // RW
    kTRCSTATR             = encode(0b10, 0b001, 0b0000, 0b0011, 0b000), // RO
    kTRCSYNCPR            = encode(0b10, 0b001, 0b0000, 0b1101, 0b000), // RW
    kTRCTRACEIDR          = encode(0b10, 0b001, 0b0000, 0b0000, 0b001), // RW
    kTRCTSCTLR            = encode(0b10, 0b001, 0b0000, 0b1100, 0b000), // RW
    kTRCVDARCCTLR         = encode(0b10, 0b001, 0b0000, 0b1010, 0b010), // RW
    kTRCVDCTLR            = encode(0b10, 0b001, 0b0000, 0b1000, 0b010), // RW
    kTRCVDSACCTLR         = encode(0b10, 0b001, 0b0000, 0b1001, 0b010), // RW
    kTRCVICTLR            = encode(0b10, 0b001, 0b0000, 0b0000, 0b010), // RW
    kTRCVIIECTLR          = encode(0b10, 0b001, 0b0000, 0b0001, 0b010), // RW
    kTRCVIPCSSCTLR        = encode(0b10, 0b001, 0b0000, 0b0011, 0b010), // RW
    kTRCVISSCTLR          = encode(0b10, 0b001, 0b0000, 0b0010, 0b010), // RW
    kTRCVMIDCCTLR0        = encode(0b10, 0b001, 0b0011, 0b0010, 0b010), // RW
    kTRCVMIDCCTLR1        = encode(0b10, 0b001, 0b0011, 0b0011, 0b010), // RW
    kTRCVMIDCVR0          = encode(0b10, 0b001, 0b0011, 0b0000, 0b001), // RW
    kTRCVMIDCVR1          = encode(0b10, 0b001, 0b0011, 0b0010, 0b001), // RW
    kTRCVMIDCVR2          = encode(0b10, 0b001, 0b0011, 0b0100, 0b001), // RW
    kTRCVMIDCVR3          = encode(0b10, 0b001, 0b0011, 0b0110, 0b001), // RW
    kTRCVMIDCVR4          = encode(0b10, 0b001, 0b0011, 0b1000, 0b001), // RW
    kTRCVMIDCVR5          = encode(0b10, 0b001, 0b0011, 0b1010, 0b001), // RW
    kTRCVMIDCVR6          = encode(0b10, 0b001, 0b0011, 0b1100, 0b001), // RW
    kTRCVMIDCVR7          = encode(0b10, 0b001, 0b0011, 0b1110, 0b001), // RW
    kTRFCR_EL1            = encode(0b11, 0b000, 0b0001, 0b0010, 0b001), // RW
    kTRFCR_EL12           = encode(0b11, 0b101, 0b0001, 0b0010, 0b001), // RW
    kTRFCR_EL2            = encode(0b11, 0b100, 0b0001, 0b0010, 0b001), // RW
    kTTBR0_EL1            = encode(0b11, 0b000, 0b0010, 0b0000, 0b000), // RW
    kTTBR0_EL12           = encode(0b11, 0b101, 0b0010, 0b0000, 0b000), // RW
    kTTBR0_EL2            = encode(0b11, 0b100, 0b0010, 0b0000, 0b000), // RW
    kTTBR0_EL3            = encode(0b11, 0b110, 0b0010, 0b0000, 0b000), // RW
    kTTBR1_EL1            = encode(0b11, 0b000, 0b0010, 0b0000, 0b001), // RW
    kTTBR1_EL12           = encode(0b11, 0b101, 0b0010, 0b0000, 0b001), // RW
    kTTBR1_EL2            = encode(0b11, 0b100, 0b0010, 0b0000, 0b001), // RW
    kUAO                  = encode(0b11, 0b000, 0b0100, 0b0010, 0b100), // RW
    kVBAR_EL1             = encode(0b11, 0b000, 0b1100, 0b0000, 0b000), // RW
    kVBAR_EL12            = encode(0b11, 0b101, 0b1100, 0b0000, 0b000), // RW
    kVBAR_EL2             = encode(0b11, 0b100, 0b1100, 0b0000, 0b000), // RW
    kVBAR_EL3             = encode(0b11, 0b110, 0b1100, 0b0000, 0b000), // RW
    kVDISR_EL2            = encode(0b11, 0b100, 0b1100, 0b0001, 0b001), // RW
    kVMPIDR_EL2           = encode(0b11, 0b100, 0b0000, 0b0000, 0b101), // RW
    kVNCR_EL2             = encode(0b11, 0b100, 0b0010, 0b0010, 0b000), // RW
    kVPIDR_EL2            = encode(0b11, 0b100, 0b0000, 0b0000, 0b000), // RW
    kVSESR_EL2            = encode(0b11, 0b100, 0b0101, 0b0010, 0b011), // RW
    kVSTCR_EL2            = encode(0b11, 0b100, 0b0010, 0b0110, 0b010), // RW
    kVSTTBR_EL2           = encode(0b11, 0b100, 0b0010, 0b0110, 0b000), // RW
    kVTCR_EL2             = encode(0b11, 0b100, 0b0010, 0b0001, 0b010), // RW
    kVTTBR_EL2            = encode(0b11, 0b100, 0b0010, 0b0001, 0b000), // RW
    kZCR_EL1              = encode(0b11, 0b000, 0b0001, 0b0010, 0b000), // RW
    kZCR_EL12             = encode(0b11, 0b101, 0b0001, 0b0010, 0b000), // RW
    kZCR_EL2              = encode(0b11, 0b100, 0b0001, 0b0010, 0b000), // RW
    kZCR_EL3              = encode(0b11, 0b110, 0b0001, 0b0010, 0b000)  // RW
  };
}

} // {Predicate}

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_ARM_A64GLOBALS_H_INCLUDED
