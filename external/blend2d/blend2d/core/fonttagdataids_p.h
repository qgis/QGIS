// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_FONTTAGDATAIDS_P_H_INCLUDED
#define BLEND2D_FONTTAGDATAIDS_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

namespace bl {
namespace FontTagData {

//! Invalid feature or variation id.
//!
//! Returned by tag to id mapping functions.
static constexpr uint32_t kInvalidId = 0xFFFFFFFFu;

// bl::FontTagData - Table IDs
// ===========================

//! Font table tag translated to a unique ID.
enum class TableId : uint32_t {
  kBASE, //!< Baseline data                      (OpenType advanced typography)
  kCBDT, //!< Color bitmap data                  (Color fonts)
  kCBLC, //!< Color bitmap location data         (Color fonts)
  kCFF , //!< Compact Font Format 1.0            (CFF outlines)
  kCFF2, //!< Compact Font Format 2.0            (CFF outlines)
  kCOLR, //!< Color table                        (Color fonts)
  kCPAL, //!< Color palette table                (Color fonts)
  kDSIG, //!< Digital signature                  (Optional)
  kEBDT, //!< Embedded bitmap data               (Bitmap glyphs)
  kEBLC, //!< Embedded bitmap location data      (Bitmap glyphs)
  kEBSC, //!< Embedded bitmap scaling data       (Bitmap glyphs)
  kGDEF, //!< Glyph definition data              (OpenType advanced typography)
  kGPOS, //!< Glyph positioning data             (OpenType advanced typography)
  kGSUB, //!< Glyph substitution data            (OpenType advanced typography)
  kHVAR, //!< Horizontal metrics variations      (OpenType font variations)
  kJSTF, //!< Justification data                 (OpenType advanced typography)
  kLTSH, //!< Linear threshold data              (Other)
  kMATH, //!< Math layout data                   (OpenType advanced typography)
  kMERG, //!< Merge                              (Other)
  kMVAR, //!< Metrics variations                 (OpenType font variations)
  kOS_2, //!< OS/2 and Windows specific metrics  (Required)
  kPCLT, //!< PCL 5 data                         (Other)
  kSTAT, //!< Style attributes                   (OpenType font variations)
  kSVG , //!< SVG table                          (SVG outlines)
  kVDMX, //!< Vertical device metrics            (Metrics)
  kVORG, //!< Vertical origin                    (CFF outlines)
  kVVAR, //!< Vertical metrics variations        (OpenType font variations)
  kZAPF, //!< ZAPF table                         (Apple advanced typography)

  kACNT, //!< Accent attachment table            (Apple advanced typography)
  kANKR, //!< Anchor point table                 (Apple advanced typography)
  kAVAR, //!< Axis variations                    (OpenType font variations)
  kBDAT, //!< Bitmap data table                  (Apple bitmap data)
  kBHED, //!< Bitmap header table                (Apple bitmap data)
  kBLOC, //!< Bitmap location table              (Apple bitmap data)
  kBSLN, //!< Baseline table                     (Apple advanced typography)
  kCMAP, //!< Character to glyph mapping         (Required)
  kCVAR, //!< CVT variations                     (OpenType font variations & TrueType outlines)
  kCVT , //!< Control value table                (TrueType outlines)
  kFDSC, //!< Font descriptors table             (Apple advanced typography)
  kFEAT, //!< Feature name table                 (Apple advanced typography)
  kFMTX, //!< Font metrics table                 (Apple advanced typography)
  kFOND, //!< FOND table                         (Apple advanced typography)
  kFPGM, //!< Font program                       (TrueType outlines)
  kFVAR, //!< Font variations                    (OpenType font variations)
  kGASP, //!< Grid-fitting/Scan-conversion       (TrueType outlines)
  kGCID, //!< Characters to CIDs mapping         (Apple advanced typography)
  kGLYF, //!< Glyph data                         (TrueType outlines)
  kGVAR, //!< Glyph variations                   (OpenType font variations & TrueType outlines)
  kHDMX, //!< Horizontal device metrics          (Metrics)
  kHEAD, //!< Font header                        (Required)
  kHHEA, //!< Horizontal header                  (Required)
  kHMTX, //!< Horizontal metrics                 (Required)
  kJUST, //!< Justification data                 (Apple advanced typography)
  kKERN, //!< Legacy Kerning (not OpenType)      (Kerning)
  kKERX, //!< Extended kerning table             (Apple advanced typography)
  kLCAR, //!< Ligature caret table               (Apple advanced typography)
  kLOCA, //!< Index to location                  (TrueType outlines)
  kLTAG, //!< Maps numeric codes and IETF tags   (Apple advanced typography)
  kMAXP, //!< Maximum profile                    (Required)
  kMETA, //!< Metadata                           (Other)
  kMORT, //!< Glyph metamorphosis (Deprecated!)  (Apple advanced typography)
  kMORX, //!< Extended glyph metamorphosis table (Apple advanced typography)
  kNAME, //!< Naming table                       (Required)
  kOPBD, //!< Optical bounds table               (Apple advanced typography)
  kPOST, //!< PostScript information             (Required)
  kPREP, //!< CVT program                        (TrueType outlines)
  kPROP, //!< Glyph properties table             (Apple advanced typography)
  kSBIX, //!< Standard bitmap graphics           (Bitmap glyphs)
  kTRAK, //!< Tracking table                     (Apple advanced typography)
  kVHEA, //!< Vertical Metrics header            (Metrics)
  kVMTX, //!< Vertical Metrics                   (Metrics)
  kXREF, //!< Cross-reference table              (Apple advanced typography)

  kMaxValue = kXREF
};

static constexpr uint32_t kTableIdCount = uint32_t(TableId::kMaxValue) + 1u;

BL_HIDDEN extern const BLTag table_id_to_tag_table[];

// bl::FontTagData - Script IDs
// ============================

//! Text script tag translated to a unique ID.
enum class ScriptId : uint32_t {
  kDFLT, //!< Default

  kADLM, //!< Adlam
  kAGHB, //!< Caucasian Albanian
  kAHOM, //!< Ahom
  kARAB, //!< Arabic
  kARMI, //!< Imperial Aramaic
  kARMN, //!< Armenian
  kAVST, //!< Avestan
  kBALI, //!< Balinese
  kBAMU, //!< Bamum
  kBASS, //!< Bassa Vah
  kBATK, //!< Batak
  kBENG, //!< Bengali
  kBHKS, //!< Bhaiksuki
  kBNG2, //!< Bengali v2
  kBOPO, //!< Bopomofo
  kBRAH, //!< Brahmi
  kBRAI, //!< Braille
  kBUGI, //!< Buginese
  kBUHD, //!< Buhid
  kBYZM, //!< Byzantine Music
  kCAKM, //!< Chakma
  kCANS, //!< Canadian Syllabics
  kCARI, //!< Carian
  kCHAM, //!< Cham
  kCHER, //!< Cherokee
  kCHRS, //!< Chorasmian
  kCOPT, //!< Coptic
  kCPMN, //!< Cypro-Minoan
  kCPRT, //!< Cypriot Syllabary
  kCYRL, //!< Cyrillic
  kDEV2, //!< Devanagari v2
  kDEVA, //!< Devanagari
  kDIAK, //!< Dives Akuru
  kDOGR, //!< Dogra
  kDSRT, //!< Deseret
  kDUPL, //!< Duployan
  kEGYP, //!< Egyptian Hieroglyphs
  kELBA, //!< Elbasan
  kELYM, //!< Elymaic
  kETHI, //!< Ethiopic
  kGEOR, //!< Georgian
  kGJR2, //!< Gujarati v2
  kGLAG, //!< Glagolitic
  kGONG, //!< Gunjala Gondi
  kGONM, //!< Masaram Gondi
  kGOTH, //!< Gothic
  kGRAN, //!< Grantha
  kGREK, //!< Greek
  kGUJR, //!< Gujarati
  kGUR2, //!< Gurmukhi v2
  kGURU, //!< Gurmukhi
  kHANG, //!< Hangul
  kHANI, //!< CJK Ideographic
  kHANO, //!< Hanunoo
  kHATR, //!< Hatran
  kHEBR, //!< Hebrew
  kHLUW, //!< Anatolian Hieroglyphs
  kHMNG, //!< Pahawh Hmong
  kHMNP, //!< Nyiakeng Puachue Hmong
  kHUNG, //!< Old Hungarian
  kITAL, //!< Old Italic
  kJAMO, //!< Hangul Jamo
  kJAVA, //!< Javanese
  kKALI, //!< Kayah Li
  kKANA, //!< Katakana, Hiragana
  kKHAR, //!< Kharosthi
  kKHMR, //!< Khmer
  kKHOJ, //!< Khojki
  kKITS, //!< Khitan Small Script
  kKND2, //!< Kannada v2
  kKNDA, //!< Kannada
  kKTHI, //!< Kaithi
  kLANA, //!< Tai Tham (Lanna)
  kLAO , //!< Lao
  kLATN, //!< Latin
  kLEPC, //!< Lepcha
  kLIMB, //!< Limbu
  kLINA, //!< Linear A
  kLINB, //!< Linear B
  kLISU, //!< Lisu (Fraser)
  kLYCI, //!< Lycian
  kLYDI, //!< Lydian
  kMAHJ, //!< Mahajani
  kMAKA, //!< Makasar
  kMAND, //!< Mandaic, Mandaean
  kMANI, //!< Manichaean
  kMARC, //!< Marchen
  kMATH, //!< Mathematical Alphanumeric Symbols
  kMEDF, //!< Medefaidrin
  kMEND, //!< Mende Kikakui
  kMERC, //!< Meroitic Cursive
  kMERO, //!< Meroitic Hieroglyphs
  kMLM2, //!< Malayalam v2
  kMLYM, //!< Malayalam
  kMODI, //!< Modi
  kMONG, //!< Mongolian
  kMROO, //!< Mro
  kMTEI, //!< Meitei Mayek (Meithei, Meetei)
  kMULT, //!< Multani
  kMUSC, //!< Musical Symbols
  kMYM2, //!< Myanmar v2
  kMYMR, //!< Myanmar
  kNAND, //!< Nandinagari
  kNARB, //!< Old North Arabian
  kNBAT, //!< Nabataean
  kNEWA, //!< Newa
  kNKO , //!< N'Ko
  kNSHU, //!< Nushu
  kOGAM, //!< Ogham
  kOLCK, //!< Ol Chiki
  kORKH, //!< Old Turkic, Orkhon Runic
  kORY2, //!< Odia v2
  kORYA, //!< Odia
  kOSGE, //!< Osage
  kOSMA, //!< Osmanya
  kOUGR, //!< Old Uyghur
  kPALM, //!< Palmyrene
  kPAUC, //!< Pau Cin Hau
  kPERM, //!< Old Permic
  kPHAG, //!< Phags-pa
  kPHLI, //!< Inscriptional Pahlavi
  kPHLP, //!< Psalter Pahlavi
  kPHNX, //!< Phoenician
  kPLRD, //!< Miao
  kPRTI, //!< Inscriptional Parthian
  kRJNG, //!< Rejang
  kROHG, //!< Hanifi Rohingya
  kRUNR, //!< Runic
  kSAMR, //!< Samaritan
  kSARB, //!< Old South Arabian
  kSAUR, //!< Saurashtra
  kSGNW, //!< Sign Writing
  kSHAW, //!< Shavian
  kSHRD, //!< Sharada
  kSIDD, //!< Siddham
  kSIND, //!< Khudawadi
  kSINH, //!< Sinhala
  kSOGD, //!< Sogdian
  kSOGO, //!< Old Sogdian
  kSORA, //!< Sora Sompeng
  kSOYO, //!< Soyombo
  kSUND, //!< Sundanese
  kSYLO, //!< Syloti Nagri
  kSYRC, //!< Syriac
  kTAGB, //!< Tagbanwa
  kTAKR, //!< Takri
  kTALE, //!< Tai Le
  kTALU, //!< New Tai Lue
  kTAML, //!< Tamil
  kTANG, //!< Tangut
  kTAVT, //!< Tai Viet
  kTEL2, //!< Telugu v2
  kTELU, //!< Telugu
  kTFNG, //!< Tifinagh
  kTGLG, //!< Tagalog
  kTHAA, //!< Thaana
  kTHAI, //!< Thai
  kTIBT, //!< Tibetan
  kTIRH, //!< Tirhuta
  kTML2, //!< Tamil v2
  kTNSA, //!< Tangsa
  kTOTO, //!< Toto
  kUGAR, //!< Ugaritic Cuneiform
  kVAI , //!< Vai
  kVITH, //!< Vithkuqi
  kWARA, //!< Warang Citi
  kWCHO, //!< Wancho
  kXPEO, //!< Old Persian Cuneiform
  kXSUX, //!< Sumero-Akkadian Cuneiform
  kYEZI, //!< Yezidi
  kYI  , //!< Yi
  kZANB, //!< Zanabazar Square

  kMaxValue = kZANB
};

static constexpr uint32_t kScriptIdCount = uint32_t(ScriptId::kMaxValue) + 1u;

BL_HIDDEN extern const BLTag script_id_to_tag_table[];

// bl::FontTagData - Language IDs
// ==============================

//! Font language tag translated to a unique ID.
enum class LanguageId : uint32_t {
  kABA , //!< Abaza (ISO: abq).
  kABK , //!< Abkhazian (ISO: abk).
  kACH , //!< Acholi (ISO: ach).
  kACR , //!< Achi (ISO: acr).
  kADY , //!< Adyghe (ISO: ady).
  kAFK , //!< Afrikaans (ISO: afr).
  kAFR , //!< Afar (ISO: aar).
  kAGW , //!< Agaw (ISO: ahg).
  kAIO , //!< Aiton (ISO: aio).
  kAKA , //!< Akan (ISO: aka, fat, twi).
  kAKB , //!< Batak Angkola (ISO: akb).
  kALS , //!< Alsatian (ISO: gsw).
  kALT , //!< Altai (ISO: atv, alt).
  kAMH , //!< Amharic (ISO: amh).
  kANG , //!< Anglo-Saxon (ISO: ang).
  kAPPH, //!< Phonetic transcription—Americanist conventions.
  kARA , //!< Arabic (ISO: ara).
  kARG , //!< Aragonese (ISO: arg).
  kARI , //!< Aari (ISO: aiw).
  kARK , //!< Rakhine (ISO: mhv, rmz, rki).
  kASM , //!< Assamese (ISO: asm).
  kAST , //!< Asturian (ISO: ast).
  kATH , //!< Athapaskan (ISO: aht, apa, apk, apj, apl, apm, apw, ath, bea, sek, bcr, caf, chp, clc, coq, crx, ctc,
         //!< den, dgr, gce, gwi, haa, hoi, hup, ing, kkz, koy, ktw, kuu, mvb, nav, qwt, scs, srs, taa, tau, tcb,
         //!< tce, tfn, tgx, tht, tol, ttm, tuu, txc, wlk, xup, xsl).
  kAVN , //!< Avatime (ISO: avn).
  kAVR , //!< Avar (ISO: ava).
  kAWA , //!< Awadhi (ISO: awa).
  kAYM , //!< Aymara (ISO: aym).
  kAZB , //!< Torki (ISO: azb).
  kAZE , //!< Azerbaijani (ISO: aze).
  kBAD , //!< Badaga (ISO: bfq).
  kBAD0, //!< Banda (ISO: bad, bbp, bfl, bjo, bpd, bqk, gox, kuw, liy, lna, lnl, mnh, nue, nuu, tor, yaj, zmz).
  kBAG , //!< Baghelkhandi (ISO: bfy).
  kBAL , //!< Balkar (ISO: krc).
  kBAN , //!< Balinese (ISO: ban).
  kBAR , //!< Bavarian (ISO: bar).
  kBAU , //!< Baulé (ISO: bci).
  kBBC , //!< Batak Toba (ISO: bbc).
  kBBR , //!< Berber (ISO: auj, ber, cnu, gha, gho, grr, jbe, jbn, kab, mzb, oua, rif, sds, shi, shy, siz, sjs, swn,
         //!< taq, tez, thv, thz, tia, tjo, tmh, ttq, tzm, zen, zgh).
  kBCH , //!< Bench (ISO: bcq).
  kBCR , //!< Bible Cree.
  kBDY , //!< Bandjalang (ISO: bdy).
  kBEL , //!< Belarussian (ISO: bel).
  kBEM , //!< Bemba (ISO: bem).
  kBEN , //!< Bengali (ISO: ben).
  kBGC , //!< Haryanvi (ISO: bgc).
  kBGQ , //!< Bagri (ISO: bgq).
  kBGR , //!< Bulgarian (ISO: bul).
  kBHI , //!< Bhili (ISO: bhi, bhb).
  kBHO , //!< Bhojpuri (ISO: bho).
  kBIK , //!< Bikol (ISO: bik, bhk, bcl, bto, cts, bln, fbl, lbl, rbl, ubl).
  kBIL , //!< Bilen (ISO: byn).
  kBIS , //!< Bislama (ISO: bis).
  kBJJ , //!< Kanauji (ISO: bjj).
  kBKF , //!< Blackfoot (ISO: bla).
  kBLI , //!< Baluchi (ISO: bal).
  kBLK , //!< Pa’o Karen (ISO: blk).
  kBLN , //!< Balante (ISO: bjt, ble).
  kBLT , //!< Balti (ISO: bft).
  kBMB , //!< Bambara (Bamanankan). (ISO: bam).
  kBML , //!< Bamileke (ISO: bai, bbj, bko, byv, fmp, jgo, nla, nnh, nnz, nwe, xmg, ybb).
  kBOS , //!< Bosnian (ISO: bos).
  kBPY , //!< Bishnupriya Manipuri (ISO: bpy).
  kBRE , //!< Breton (ISO: bre).
  kBRH , //!< Brahui (ISO: brh).
  kBRI , //!< Braj Bhasha (ISO: bra).
  kBRM , //!< Burmese (ISO: mya).
  kBRX , //!< Bodo (ISO: brx).
  kBSH , //!< Bashkir (ISO: bak).
  kBSK , //!< Burushaski (ISO: bsk).
  kBTD , //!< Batak Dairi (Pakpak). (ISO: btd).
  kBTI , //!< Beti (ISO: btb, beb, bum, bxp, eto, ewo, mct).
  kBTK , //!< Batak (ISO: languages   akb, bbc, btd, btk, btm, bts, btx, btz).
  kBTM , //!< Batak Mandailing (ISO: btm).
  kBTS , //!< Batak Simalungun (ISO: bts).
  kBTX , //!< Batak Karo (ISO: btx).
  kBTZ , //!< Batak Alas-Kluet (ISO: btz).
  kBUG , //!< Bugis (ISO: bug).
  kBYV , //!< Medumba (ISO: byv).
  kCAK , //!< Kaqchikel (ISO: cak).
  kCAT , //!< Catalan (ISO: cat).
  kCBK , //!< Zamboanga Chavacano (ISO: cbk).
  kCCHN, //!< Chinantec (ISO: cco, chj, chq, chz, cle, cnl, cnt, cpa, csa, cso, cte, ctl, cuc, cvn).
  kCEB , //!< Cebuano (ISO: ceb).
  kCGG , //!< Chiga (ISO: cgg).
  kCHA , //!< Chamorro (ISO: cha).
  kCHE , //!< Chechen (ISO: che).
  kCHG , //!< Chaha Gurage (ISO: sgw).
  kCHH , //!< Chattisgarhi (ISO: hne).
  kCHI , //!< Chichewa (Chewa, Nyanja). (ISO: nya).
  kCHK , //!< Chukchi (ISO: ckt).
  kCHK0, //!< Chuukese (ISO: chk).
  kCHO , //!< Choctaw (ISO: cho).
  kCHP , //!< Chipewyan (ISO: chp).
  kCHR , //!< Cherokee (ISO: chr).
  kCHU , //!< Chuvash (ISO: chv).
  kCHY , //!< Cheyenne (ISO: chy).
  kCJA , //!< Western Cham (ISO: cja).
  kCJM , //!< Eastern Cham (ISO: cjm).
  kCMR , //!< Comorian (ISO: swb, wlc, wni, zdj).
  kCOP , //!< Coptic (ISO: cop).
  kCOR , //!< Cornish (ISO: cor).
  kCOS , //!< Corsican (ISO: cos).
  kCPP , //!< Creoles (ISO: abs, acf, afs, aig, aoa, bah, bew, bis, bjs, bpl, bpq, brc, bxo, bzj, bzk, cbk, ccl, ccm,
         //!< chn, cks, cpe, cpf, cpi, cpp, cri, crp, crs, dcr, dep, djk, fab, fng, fpe, gac, gcf, gcl, gcr, gib, goq,
         //!< gpe, gul, gyn, hat, hca, hmo, hwc, icr, idb, ihb, jam, jvd, kcn, kea, kmv, kri, kww, lir, lou, lrt, max,
         //!< mbf, mcm, mfe, mfp, mkn, mod, msi, mud, mzs, nag, nef, ngm, njt, onx, oor, pap, pcm, pea, pey, pga, pih,
         //!< pis, pln, pml, pmy, pov, pre, rcf, rop, scf, sci, skw, srm, srn, sta, svc, tas, tch, tcs, tgh, tmg, tpi,
         //!< trf, tvy, uln, vic, vkp, wes, xmm).
  kCRE , //!< Cree (ISO: cre).
  kCRR , //!< Carrier (ISO: crx, caf).
  kCRT , //!< Crimean Tatar (ISO: crh).
  kCSB , //!< Kashubian (ISO: csb).
  kCSL , //!< Church Slavonic (ISO: chu).
  kCSY , //!< Czech (ISO: ces).
  kCTG , //!< Chittagonian (ISO: ctg).
  kCTT , //!< Wayanad Chetti (ISO: ctt).
  kCUK , //!< San Blas Kuna (ISO: cuk).
  kDAG , //!< Dagbani (ISO: dag).
  kDAN , //!< Danish (ISO: dan).
  kDAR , //!< Dargwa (ISO: dar).
  kDAX , //!< Dayi (ISO: dax).
  kDCR , //!< Woods Cree (ISO: cwd).
  kDEU , //!< German (ISO: deu).
  kDGO , //!< Dogri (individual language). (ISO: dgo).
  kDGR , //!< Dogri (macrolanguage). (ISO: doi).
  kDHG , //!< Dhangu (ISO: dhg).
  kDHV , //!< Divehi (Dhivehi, Maldivian).   (deprecated). (ISO: div).
  kDIQ , //!< Dimli (ISO: diq).
  kDIV , //!< Divehi (Dhivehi, Maldivian). (ISO: div).
  kDJR , //!< Zarma (ISO: dje).
  kDJR0, //!< Djambarrpuyngu (ISO: djr).
  kDNG , //!< Dangme (ISO: ada).
  kDNJ , //!< Dan (ISO: dnj).
  kDNK , //!< Dinka (ISO: din).
  kDRI , //!< Dari (ISO: prs).
  kDUJ , //!< Dhuwal (ISO: duj, dwu, dwy).
  kDUN , //!< Dungan (ISO: dng).
  kDZN , //!< Dzongkha (ISO: dzo).
  kEBI , //!< Ebira (ISO: igb).
  kECR , //!< Eastern Cree (ISO: crj, crl).
  kEDO , //!< Edo (ISO: bin).
  kEFI , //!< Efik (ISO: efi).
  kELL , //!< Greek (ISO: ell).
  kEMK , //!< Eastern Maninkakan (ISO: emk).
  kENG , //!< English (ISO: eng).
  kERZ , //!< Erzya (ISO: myv).
  kESP , //!< Spanish (ISO: spa).
  kESU , //!< Central Yupik (ISO: esu).
  kETI , //!< Estonian (ISO: est).
  kEUQ , //!< Basque (ISO: eus).
  kEVK , //!< Evenki (ISO: evn).
  kEVN , //!< Even (ISO: eve).
  kEWE , //!< Ewe (ISO: ewe).
  kFAN , //!< French Antillean (ISO: acf).
  kFAN0, //!< Fang (ISO: fan).
  kFAR , //!< Persian (ISO: fas).
  kFAT , //!< Fanti (ISO: fat).
  kFIN , //!< Finnish (ISO: fin).
  kFJI , //!< Fijian (ISO: fij).
  kFLE , //!< Dutch (Flemish). (ISO: vls).
  kFMP , //!< Fe’fe’ (ISO: fmp).
  kFNE , //!< Forest Enets (ISO: enf).
  kFON , //!< Fon (ISO: fon).
  kFOS , //!< Faroese (ISO: fao).
  kFRA , //!< French (ISO: fra).
  kFRC , //!< Cajun French (ISO: frc).
  kFRI , //!< Frisian (ISO: fry).
  kFRL , //!< Friulian (ISO: fur).
  kFRP , //!< Arpitan (ISO: frp).
  kFTA , //!< Futa (ISO: fuf).
  kFUL , //!< Fulah (ISO: ful).
  kFUV , //!< Nigerian Fulfulde (ISO: fuv).
  kGAD , //!< Ga (ISO: gaa).
  kGAE , //!< Scottish Gaelic (Gaelic). (ISO: gla).
  kGAG , //!< Gagauz (ISO: gag).
  kGAL , //!< Galician (ISO: glg).
  kGAR , //!< Garshuni.
  kGAW , //!< Garhwali (ISO: gbm).
  kGEZ , //!< Geez (ISO: gez).
  kGIH , //!< Githabul (ISO: gih).
  kGIL , //!< Gilyak (ISO: niv).
  kGIL0, //!< Kiribati (Gilbertese). (ISO: gil).
  kGKP , //!< Kpelle (Guinea). (ISO: gkp).
  kGLK , //!< Gilaki (ISO: glk).
  kGMZ , //!< Gumuz (ISO: guk).
  kGNN , //!< Gumatj (ISO: gnn).
  kGOG , //!< Gogo (ISO: gog).
  kGON , //!< Gondi (ISO: gon).
  kGRN , //!< Greenlandic (ISO: kal).
  kGRO , //!< Garo (ISO: grt).
  kGUA , //!< Guarani (ISO: grn).
  kGUC , //!< Wayuu (ISO: guc).
  kGUF , //!< Gupapuyngu (ISO: guf).
  kGUJ , //!< Gujarati (ISO: guj).
  kGUZ , //!< Gusii (ISO: guz).
  kHAI , //!< Haitian (Haitian Creole). (ISO: hat).
  kHAI0, //!< Haida (ISO: hai, hax, hdn).
  kHAL , //!< Halam (Falam Chin). (ISO: cfm).
  kHAR , //!< Harauti (ISO: hoj).
  kHAU , //!< Hausa (ISO: hau).
  kHAW , //!< Hawaiian (ISO: haw).
  kHAY , //!< Haya (ISO: hay).
  kHAZ , //!< Hazaragi (ISO: haz).
  kHBN , //!< Hammer-Banna (ISO: amf).
  kHEI , //!< Heiltsuk (ISO: hei).
  kHER , //!< Herero (ISO: her).
  kHIL , //!< Hiligaynon (ISO: hil).
  kHIN , //!< Hindi (ISO: hin).
  kHMA , //!< High Mari (ISO: mrj).
  kHMD , //!< A-Hmao (ISO: hmd).
  kHMN , //!< Hmong (ISO: hmn).
  kHMO , //!< Hiri Motu (ISO: hmo).
  kHMZ , //!< Hmong Shuat (ISO: hmz).
  kHND , //!< Hindko (ISO: hno, hnd).
  kHO  , //!< Ho (ISO: hoc).
  kHRI , //!< Harari (ISO: har).
  kHRV , //!< Croatian (ISO: hrv).
  kHUN , //!< Hungarian (ISO: hun).
  kHYE , //!< Armenian (ISO: hye, hyw).
  kHYE0, //!< Armenian East (ISO: hye).
  kIBA , //!< Iban (ISO: iba).
  kIBB , //!< Ibibio (ISO: ibb).
  kIBO , //!< Igbo (ISO: ibo).
  kIDO , //!< Ido (ISO: ido).
  kIJO , //!< Ijo (ISO: languages   iby, ijc, ije, ijn, ijo, ijs, nkx, okd, okr, orr).
  kILE , //!< Interlingue (ISO: ile).
  kILO , //!< Ilokano (ISO: ilo).
  kINA , //!< Interlingua (ISO: ina).
  kIND , //!< Indonesian (ISO: ind).
  kING , //!< Ingush (ISO: inh).
  kINU , //!< Inuktitut (ISO: iku).
  kINUK, //!< Nunavik Inuktitut (ISO: ike, iku).
  kIPK , //!< Inupiat (ISO: ipk).
  kIPPH, //!< Phonetic transcription—IPA (ISO: conventions).
  kIRI , //!< Irish (ISO: gle).
  kIRT , //!< Irish Traditional (ISO: gle).
  kIRU , //!< Irula (ISO: iru).
  kISL , //!< Icelandic (ISO: isl).
  kISM , //!< Inari Sami (ISO: smn).
  kITA , //!< Italian (ISO: ita).
  kIWR , //!< Hebrew (ISO: heb).
  kJAM , //!< Jamaican Creole (ISO: jam).
  kJAN , //!< Japanese (ISO: jpn).
  kJAV , //!< Javanese (ISO: jav).
  kJBO , //!< Lojban (ISO: jbo).
  kJCT , //!< Krymchak (ISO: jct).
  kJII , //!< Yiddish (ISO: yid).
  kJUD , //!< Ladino (ISO: lad).
  kJUL , //!< Jula (ISO: dyu).
  kKAB , //!< Kabardian (ISO: kbd).
  kKAB0, //!< Kabyle (ISO: kab).
  kKAC , //!< Kachchi (ISO: kfr).
  kKAL , //!< Kalenjin (ISO: kln).
  kKAN , //!< Kannada (ISO: kan).
  kKAR , //!< Karachay (ISO: krc).
  kKAT , //!< Georgian (ISO: kat).
  kKAW , //!< Kawi (Old Javanese). (ISO: kaw).
  kKAZ , //!< Kazakh (ISO: kaz).
  kKDE , //!< Makonde (ISO: kde).
  kKEA , //!< Kabuverdianu (Crioulo). (ISO: kea).
  kKEB , //!< Kebena (ISO: ktb).
  kKEK , //!< Kekchi (ISO: kek).
  kKGE , //!< Khutsuri Georgian (ISO: kat).
  kKHA , //!< Khakass (ISO: kjh).
  kKHK , //!< Khanty-Kazim (ISO: kca).
  kKHM , //!< Khmer (ISO: khm).
  kKHS , //!< Khanty-Shurishkar (ISO: kca).
  kKHT , //!< Khamti Shan (ISO: kht).
  kKHV , //!< Khanty-Vakhi (ISO: kca).
  kKHW , //!< Khowar (ISO: khw).
  kKIK , //!< Kikuyu (Gikuyu). (ISO: kik).
  kKIR , //!< Kirghiz (Kyrgyz). (ISO: kir).
  kKIS , //!< Kisii (ISO: kqs, kss).
  kKIU , //!< Kirmanjki (ISO: kiu).
  kKJD , //!< Southern Kiwai (ISO: kjd).
  kKJP , //!< Eastern Pwo Karen (ISO: kjp).
  kKJZ , //!< Bumthangkha (ISO: kjz).
  kKKN , //!< Kokni (ISO: kex).
  kKLM , //!< Kalmyk (ISO: xal).
  kKMB , //!< Kamba (ISO: kam).
  kKMN , //!< Kumaoni (ISO: kfy).
  kKMO , //!< Komo (ISO: kmw).
  kKMS , //!< Komso (ISO: kxc).
  kKMZ , //!< Khorasani Turkic (ISO: kmz).
  kKNR , //!< Kanuri (ISO: kau).
  kKOD , //!< Kodagu (ISO: kfa).
  kKOH , //!< Korean Old Hangul (ISO: kor, okm).
  kKOK , //!< Konkani (ISO: kok).
  kKOM , //!< Komi (ISO: kom).
  kKON , //!< Kikongo (ISO: ktu).
  kKON0, //!< Kongo (ISO: kon).
  kKOP , //!< Komi-Permyak (ISO: koi).
  kKOR , //!< Korean (ISO: kor).
  kKOS , //!< Kosraean (ISO: kos).
  kKOZ , //!< Komi-Zyrian (ISO: kpv).
  kKPL , //!< Kpelle (ISO: kpe).
  kKRI , //!< Krio (ISO: kri).
  kKRK , //!< Karakalpak (ISO: kaa).
  kKRL , //!< Karelian (ISO: krl).
  kKRM , //!< Karaim (ISO: kdr).
  kKRN , //!< Karen (ISO: blk, bwe, eky, ghk, jkm, jkp, kar, kjp, kjt, ksw, kvl, kvq, kvt, kvu, kvy, kxf, kxk, kyu,
         //!< pdu, pwo, pww, wea).
  kKRT , //!< Koorete (ISO: kqy).
  kKSH , //!< Kashmiri (ISO: kas).
  kKSH0, //!< Ripuarian (ISO: ksh).
  kKSI , //!< Khasi (ISO: kha).
  kKSM , //!< Kildin Sami (ISO: sjd).
  kKSW , //!< S’gaw Karen (ISO: ksw).
  kKUA , //!< Kuanyama (ISO: kua).
  kKUI , //!< Kui (ISO: kxu).
  kKUL , //!< Kulvi (ISO: kfx).
  kKUM , //!< Kumyk (ISO: kum).
  kKUR , //!< Kurdish (ISO: kur).
  kKUU , //!< Kurukh (ISO: kru).
  kKUY , //!< Kuy (ISO: kdt).
  kKWK , //!< Kwakʼ (ISO: wala   kwk).
  kKYK , //!< Koryak (ISO: kpy).
  kKYU , //!< Western Kayah (ISO: kyu).
  kLAD , //!< Ladin (ISO: lld).
  kLAH , //!< Lahuli (ISO: bfu).
  kLAK , //!< Lak (ISO: lbe).
  kLAM , //!< Lambani (ISO: lmn).
  kLAO , //!< Lao (ISO: lao).
  kLAT , //!< Latin (ISO: lat).
  kLAZ , //!< Laz (ISO: lzz).
  kLCR , //!< L-Cree (ISO: crm).
  kLDK , //!< Ladakhi (ISO: lbj).
  kLEF , //!< Lelemi (ISO: lef).
  kLEZ , //!< Lezgi (ISO: lez).
  kLIJ , //!< Ligurian (ISO: lij).
  kLIM , //!< Limburgish (ISO: lim).
  kLIN , //!< Lingala (ISO: lin).
  kLIS , //!< Lisu (ISO: lis).
  kLJP , //!< Lampung (ISO: ljp).
  kLKI , //!< Laki (ISO: lki).
  kLMA , //!< Low Mari (ISO: mhr).
  kLMB , //!< Limbu (ISO: lif).
  kLMO , //!< Lombard (ISO: lmo).
  kLMW , //!< Lomwe (ISO: ngl).
  kLOM , //!< Loma (ISO: lom).
  kLPO , //!< Lipo (ISO: lpo).
  kLRC , //!< Luri (ISO: lrc, luz, bqi, zum).
  kLSB , //!< Lower Sorbian (ISO: dsb).
  kLSM , //!< Lule Sami (ISO: smj).
  kLTH , //!< Lithuanian (ISO: lit).
  kLTZ , //!< Luxembourgish (ISO: ltz).
  kLUA , //!< Luba-Lulua (ISO: lua).
  kLUB , //!< Luba-Katanga (ISO: lub).
  kLUG , //!< Ganda (ISO: lug).
  kLUH , //!< Luyia (ISO: luy).
  kLUO , //!< Luo (ISO: luo).
  kLVI , //!< Latvian (ISO: lav).
  kMAD , //!< Madura (ISO: mad).
  kMAG , //!< Magahi (ISO: mag).
  kMAH , //!< Marshallese (ISO: mah).
  kMAJ , //!< Majang (ISO: mpe).
  kMAK , //!< Makhuwa (ISO: vmw).
  kMAL , //!< Malayalam (ISO: mal).
  kMAM , //!< Mam (ISO: mam).
  kMAN , //!< Mansi (ISO: mns).
  kMAP , //!< Mapudungun (ISO: arn).
  kMAR , //!< Marathi (ISO: mar).
  kMAW , //!< Marwari (ISO: mwr, dhd, rwr, mve, wry, mtr, swv).
  kMBN , //!< Mbundu (ISO: kmb).
  kMBO , //!< Mbo (ISO: mbo).
  kMCH , //!< Manchu (ISO: mnc).
  kMCR , //!< Moose Cree (ISO: crm).
  kMDE , //!< Mende (ISO: men).
  kMDR , //!< Mandar (ISO: mdr).
  kMEN , //!< Me’en (ISO: mym).
  kMER , //!< Meru (ISO: mer).
  kMFA , //!< Pattani Malay (ISO: mfa).
  kMFE , //!< Morisyen (ISO: mfe).
  kMIN , //!< Minangkabau (ISO: min).
  kMIZ , //!< Mizo (ISO: lus).
  kMKD , //!< Macedonian (ISO: mkd).
  kMKR , //!< Makasar (ISO: mak).
  kMKW , //!< Kituba (ISO: mkw).
  kMLE , //!< Male (ISO: mdy).
  kMLG , //!< Malagasy (ISO: mlg).
  kMLN , //!< Malinke (ISO: mlq).
  kMLR , //!< Malayalam Reformed (ISO: mal).
  kMLY , //!< Malay (ISO: msa).
  kMND , //!< Mandinka (ISO: mnk).
  kMNG , //!< Mongolian (ISO: mon).
  kMNI , //!< Manipuri (ISO: mni).
  kMNK , //!< Maninka (ISO: man, mnk, myq, mku, msc, emk, mwk, mlq).
  kMNX , //!< Manx (ISO: glv).
  kMOH , //!< Mohawk (ISO: moh).
  kMOK , //!< Moksha (ISO: mdf).
  kMOL , //!< Moldavian (ISO: mol).
  kMON , //!< Mon (ISO: mnw).
  kMONT, //!< Thailand Mon (ISO: mnw).
  kMOR , //!< Moroccan.
  kMOS , //!< Mossi (ISO: mos).
  kMRI , //!< Maori (ISO: mri).
  kMTH , //!< Maithili (ISO: mai).
  kMTS , //!< Maltese (ISO: mlt).
  kMUN , //!< Mundari (ISO: unr).
  kMUS , //!< Muscogee (ISO: mus).
  kMWL , //!< Mirandese (ISO: mwl).
  kMWW , //!< Hmong Daw (ISO: mww).
  kMYN , //!< Mayan (ISO: acr, agu, caa, cac, cak, chf, ckz, cob, ctu, emy, hus, itz, ixl, jac, kek, kjb, knj, lac,
         //!< mam, mhc, mop, myn, poc, poh, quc, qum, quv, toj, ttc, tzh, tzj, tzo, usp, yua).
  kMZN , //!< Mazanderani (ISO: mzn).
  kNAG , //!< Naga-Assamese (ISO: nag).
  kNAH , //!< Nahuatl (ISO: azd, azn, azz, nah, naz, nch, nci, ncj, ncl, ncx, ngu, nhc, nhe, nhg, nhi, nhk, nhm, nhn,
         //!< nhp, nhq, nht, nhv, nhw, nhx, nhy, nhz, nlv, npl, nsu, nuz).
  kNAN , //!< Nanai (ISO: gld).
  kNAP , //!< Neapolitan (ISO: nap).
  kNAS , //!< Naskapi (ISO: nsk).
  kNAU , //!< Nauruan (ISO: nau).
  kNAV , //!< Navajo (ISO: nav).
  kNCR , //!< N-Cree (ISO: csw).
  kNDB , //!< Ndebele (ISO: nbl, nde).
  kNDC , //!< Ndau (ISO: ndc).
  kNDG , //!< Ndonga (ISO: ndo).
  kNDS , //!< Low Saxon (ISO: nds).
  kNEP , //!< Nepali (ISO: nep).
  kNEW , //!< Newari (ISO: new).
  kNGA , //!< Ngbaka (ISO: nga).
  kNGR , //!< Nagari.
  kNHC , //!< Norway House Cree (ISO: csw).
  kNIS , //!< Nisi (ISO: dap, njz, tgj).
  kNIU , //!< Niuean (ISO: niu).
  kNKL , //!< Nyankole (ISO: nyn).
  kNKO , //!< N’Ko (ISO: nqo).
  kNLD , //!< Dutch (ISO: nld).
  kNOE , //!< Nimadi (ISO: noe).
  kNOG , //!< Nogai (ISO: nog).
  kNOR , //!< Norwegian (ISO: nob).
  kNOV , //!< Novial (ISO: nov).
  kNSM , //!< Northern Sami (ISO: sme).
  kNSO , //!< Northern Sotho (ISO: nso).
  kNTA , //!< Northern Tai (ISO: nod).
  kNTO , //!< Esperanto (ISO: epo).
  kNYM , //!< Nyamwezi (ISO: nym).
  kNYN , //!< Norwegian Nynorsk (Nynorsk, Norwegian). (ISO: nno).
  kNZA , //!< Mbembe Tigon (ISO: nza).
  kOCI , //!< Occitan (ISO: oci).
  kOCR , //!< Oji-Cree (ISO: ojs).
  kOJB , //!< Ojibway (ISO: oji).
  kORI , //!< Odia (formerly Oriya). (ISO: ori).
  kORO , //!< Oromo (ISO: orm).
  kOSS , //!< Ossetian (ISO: oss).
  kPAA , //!< Palestinian Aramaic (ISO: sam).
  kPAG , //!< Pangasinan (ISO: pag).
  kPAL , //!< Pali (ISO: pli).
  kPAM , //!< Pampangan (ISO: pam).
  kPAN , //!< Punjabi (ISO: pan).
  kPAP , //!< Palpa (ISO: plp).
  kPAP0, //!< Papiamentu (ISO: pap).
  kPAS , //!< Pashto (ISO: pus).
  kPAU , //!< Palauan (ISO: pau).
  kPCC , //!< Bouyei (ISO: pcc).
  kPCD , //!< Picard (ISO: pcd).
  kPDC , //!< Pennsylvania German (ISO: pdc).
  kPGR , //!< Polytonic Greek (ISO: ell).
  kPHK , //!< Phake (ISO: phk).
  kPIH , //!< Norfolk (ISO: pih).
  kPIL , //!< Filipino (ISO: fil).
  kPLG , //!< Palaung (ISO: pce, rbb, pll).
  kPLK , //!< Polish (ISO: pol).
  kPMS , //!< Piemontese (ISO: pms).
  kPNB , //!< Western Panjabi (ISO: pnb).
  kPOH , //!< Pocomchi (ISO: poh).
  kPON , //!< Pohnpeian (ISO: pon).
  kPRO , //!< Provençal / Old Provençal (ISO: pro).
  kPTG , //!< Portuguese (ISO: por).
  kPWO , //!< Western Pwo Karen (ISO: pwo).
  kQIN , //!< Chin (ISO: bgr, biu, cek, cey, cfm, cbl, cka, ckn, clj, clt, cmr, cnb, cnh, cnk, cnw, csh, csj, csv,
         //!< csy, ctd, cth, czt, dao, gnb, hlt, hmr, hra, lus, mrh, mwq, pck, pkh, pub, ral, rtc, sch, sez, shl,
         //!< smt, tcp, tcz, vap, weu, zom, zyp).
  kQUC , //!< K’iche’ (ISO: quc).
  kQUH , //!< Quechua (Bolivia). (ISO: quh).
  kQUZ , //!< Quechua (ISO: quz).
  kQVI , //!< Quechua (Ecuador). (ISO: qvi).
  kQWH , //!< Quechua (Peru). (ISO: qwh).
  kRAJ , //!< Rajasthani (ISO: raj).
  kRAR , //!< Rarotongan (ISO: rar).
  kRBU , //!< Russian Buriat (ISO: bxr).
  kRCR , //!< R-Cree (ISO: atj).
  kREJ , //!< Rejang (ISO: rej).
  kRHG , //!< Rohingya (ISO: rhg).
  kRIA , //!< Riang (ISO: ria).
  kRIF , //!< Tarifit (ISO: rif).
  kRIT , //!< Ritarungo (ISO: rit).
  kRKW , //!< Arakwal (ISO: rkw).
  kRMS , //!< Romansh (ISO: roh).
  kRMY , //!< Vlax Romani (ISO: rmy).
  kROM , //!< Romanian (ISO: ron).
  kROY , //!< Romany (ISO: rom).
  kRSY , //!< Rusyn (ISO: rue).
  kRTM , //!< Rotuman (ISO: rtm).
  kRUA , //!< Kinyarwanda (ISO: kin).
  kRUN , //!< Rundi (ISO: run).
  kRUP , //!< Aromanian (ISO: rup).
  kRUS , //!< Russian (ISO: rus).
  kSAD , //!< Sadri (ISO: sck).
  kSAN , //!< Sanskrit (ISO: san).
  kSAS , //!< Sasak (ISO: sas).
  kSAT , //!< Santali (ISO: sat).
  kSAY , //!< Sayisi (ISO: chp).
  kSCN , //!< Sicilian (ISO: scn).
  kSCO , //!< Scots (ISO: sco).
  kSCS , //!< North Slavey (ISO: scs).
  kSEK , //!< Sekota (ISO: xan).
  kSEL , //!< Selkup (ISO: sel).
  kSFM , //!< Small Flowery Miao (ISO: sfm).
  kSGA , //!< Old Irish (ISO: sga).
  kSGO , //!< Sango (ISO: sag).
  kSGS , //!< Samogitian (ISO: sgs).
  kSHI , //!< Tachelhit (ISO: shi).
  kSHN , //!< Shan (ISO: shn).
  kSIB , //!< Sibe (ISO: sjo).
  kSID , //!< Sidamo (ISO: sid).
  kSIG , //!< Silte Gurage (ISO: xst, stv, wle).
  kSKS , //!< Skolt Sami (ISO: sms).
  kSKY , //!< Slovak (ISO: slk).
  kSLA , //!< Slavey (ISO: den, scs, xsl).
  kSLV , //!< Slovenian (ISO: slv).
  kSML , //!< Somali (ISO: som).
  kSMO , //!< Samoan (ISO: smo).
  kSNA , //!< Sena (ISO: seh).
  kSNA0, //!< Shona (ISO: sna).
  kSND , //!< Sindhi (ISO: snd).
  kSNH , //!< Sinhala (Sinhalese). (ISO: sin).
  kSNK , //!< Soninke (ISO: snk).
  kSOG , //!< Sodo Gurage (ISO: gru).
  kSOP , //!< Songe (ISO: sop).
  kSOT , //!< Southern Sotho (ISO: sot).
  kSQI , //!< Albanian (ISO: sqi).
  kSRB , //!< Serbian (ISO: cnr, srp).
  kSRD , //!< Sardinian (ISO: srd).
  kSRK , //!< Saraiki (ISO: skr).
  kSRR , //!< Serer (ISO: srr).
  kSSL , //!< South Slavey (ISO: xsl).
  kSSM , //!< Southern Sami (ISO: sma).
  kSTQ , //!< Saterland Frisian (ISO: stq).
  kSUK , //!< Sukuma (ISO: suk).
  kSUN , //!< Sundanese (ISO: sun).
  kSUR , //!< Suri (ISO: suq).
  kSVA , //!< Svan (ISO: sva).
  kSVE , //!< Swedish (ISO: swe).
  kSWA , //!< Swadaya Aramaic (ISO: aii).
  kSWK , //!< Swahili (ISO: swa).
  kSWZ , //!< Swati (ISO: ssw).
  kSXT , //!< Sutu (ISO: ngo, xnj, xnq).
  kSXU , //!< Upper Saxon (ISO: sxu).
  kSYL , //!< Sylheti (ISO: syl).
  kSYR , //!< Syriac (ISO: aii, amw, cld, syc, syr, tru).
  kSYRE, //!< Syriac, Estrangela script-variant (ISO: syc, syr).
  kSYRJ, //!< Syriac, Western script-variant (ISO: syc, syr).
  kSYRN, //!< Syriac, Eastern script-variant (ISO: syc, syr).
  kSZL , //!< Silesian (ISO: szl).
  kTAB , //!< Tabasaran (ISO: tab).
  kTAJ , //!< Tajiki (ISO: tgk).
  kTAM , //!< Tamil (ISO: tam).
  kTAT , //!< Tatar (ISO: tat).
  kTCR , //!< TH-Cree (ISO: cwd).
  kTDD , //!< Dehong Dai (ISO: tdd).
  kTEL , //!< Telugu (ISO: tel).
  kTET , //!< Tetum (ISO: tet).
  kTGL , //!< Tagalog (ISO: tgl).
  kTGN , //!< Tongan (ISO: ton).
  kTGR , //!< Tigre (ISO: tig).
  kTGY , //!< Tigrinya (ISO: tir).
  kTHA , //!< Thai (ISO: tha).
  kTHT , //!< Tahitian (ISO: tah).
  kTIB , //!< Tibetan (ISO: bod).
  kTIV , //!< Tiv (ISO: tiv).
  kTJL , //!< Tai Laing (ISO: tjl).
  kTKM , //!< Turkmen (ISO: tuk).
  kTLI , //!< Tlingit (ISO: tli).
  kTMH , //!< Tamashek (ISO: taq, thv, thz, tmh, ttq).
  kTMN , //!< Temne (ISO: tem).
  kTNA , //!< Tswana (ISO: tsn).
  kTNE , //!< Tundra Enets (ISO: enh).
  kTNG , //!< Tonga (ISO: toi).
  kTOD , //!< Todo (ISO: xal).
  kTOD0, //!< Toma (ISO: tod).
  kTPI , //!< Tok Pisin (ISO: tpi).
  kTRK , //!< Turkish (ISO: tur).
  kTSG , //!< Tsonga (ISO: tso).
  kTSJ , //!< Tshangla (ISO: tsj).
  kTUA , //!< Turoyo Aramaic (ISO: tru).
  kTUL , //!< Tumbuka (ISO: tcy).
  kTUM , //!< Tulu (ISO: tum).
  kTUV , //!< Tuvin (ISO: tyv).
  kTVL , //!< Tuvalu (ISO: tvl).
  kTWI , //!< Twi (ISO: twi).
  kTYZ , //!< Tày (ISO: tyz).
  kTZM , //!< Tamazight (ISO: tzm).
  kTZO , //!< Tzotzil (ISO: tzo).
  kUDM , //!< Udmurt (ISO: udm).
  kUKR , //!< Ukrainian (ISO: ukr).
  kUMB , //!< Umbundu (ISO: umb).
  kURD , //!< Urdu (ISO: urd).
  kUSB , //!< Upper Sorbian (ISO: hsb).
  kUYG , //!< Uyghur (ISO: uig).
  kUZB , //!< Uzbek (ISO: uzb).
  kVEC , //!< Venetian (ISO: vec).
  kVEN , //!< Venda (ISO: ven).
  kVIT , //!< Vietnamese (ISO: vie).
  kVOL , //!< Volapük (ISO: vol).
  kVRO , //!< Võro (ISO: vro).
  kWA  , //!< Wa (ISO: wbm).
  kWAG , //!< Wagdi (ISO: wbr).
  kWAR , //!< Waray-Waray (ISO: war).
  kWCI , //!< Waci Gbe (ISO: wci).
  kWCR , //!< West-Cree (ISO: crk).
  kWEL , //!< Welsh (ISO: cym).
  kWLF , //!< Wolof (ISO: wol).
  kWLN , //!< Walloon (ISO: wln).
  kWTM , //!< Mewati (ISO: wtm).
  kXBD , //!< Lü (ISO: khb).
  kXHS , //!< Xhosa (ISO: xho).
  kXJB , //!< Minjangbal (ISO: xjb).
  kXKF , //!< Khengkha (ISO: xkf).
  kXOG , //!< Soga (ISO: xog).
  kXPE , //!< Kpelle (Liberia). (ISO: xpe).
  kXUB , //!< Bette Kuruma (ISO: xub).
  kXUJ , //!< Jennu Kuruma (ISO: xuj).
  kYAK , //!< Sakha (ISO: sah).
  kYAO , //!< Yao (ISO: yao).
  kYAP , //!< Yapese (ISO: yap).
  kYBA , //!< Yoruba (ISO: yor).
  kYCR , //!< Y-Cree (ISO: crj, crk, crl).
  kYGP , //!< Gepo (ISO: ygp).
  kYIC , //!< Yi Classic.
  kYIM , //!< Yi Modern (ISO: iii).
  kYNA , //!< Aluo (ISO: yna).
  kYWQ , //!< Wuding-Luquan Yi (ISO: ywq).
  kZEA , //!< Zealandic (ISO: zea).
  kZGH , //!< Standard Moroccan Tamazight (ISO: zgh).
  kZHA , //!< Zhuang (ISO: zha).
  kZHH , //!< Chinese, Traditional, Hong Kong SAR (ISO: zho).
  kZHP , //!< Chinese, Phonetic (ISO: zho).
  kZHS , //!< Chinese, Simplified (ISO: zho).
  kZHT , //!< Chinese, Traditional (ISO: zho).
  kZHTM, //!< Chinese, Traditional, Macao SAR (ISO: zho).
  kZND , //!< Zande (ISO: zne).
  kZUL , //!< Zulu (ISO: zul).
  kZZA , //!< Zazaki (ISO: zza).

  kMaxValue = kZZA
};

static constexpr uint32_t kLanguageIdCount = uint32_t(LanguageId::kMaxValue) + 1u;

BL_HIDDEN extern const BLTag language_id_to_tag_table[];

// bl::FontTagData - Feature IDs
// =============================

//! Font feature tag translated to a unique ID.
enum class FeatureId : uint32_t {
  kAALT, //!< Access All Alternates.
  kABVF, //!< Above-base Forms.
  kABVM, //!< Above-base Mark Positioning.
  kABVS, //!< Above-base Substitutions.
  kAFRC, //!< Alternative Fractions.
  kAKHN, //!< Akhand.
  kBLWF, //!< Below-base Forms.
  kBLWM, //!< Below-base Mark Positioning.
  kBLWS, //!< Below-base Substitutions.
  kC2PC, //!< Petite Capitals From Capitals.
  kC2SC, //!< Small Capitals From Capitals.
  kCALT, //!< Contextual Alternates.
  kCASE, //!< Case-Sensitive Forms.
  kCCMP, //!< Glyph Composition / Decomposition.
  kCFAR, //!< Conjunct Form After Ro.
  kCHWS, //!< Contextual Half-width Spacing.
  kCJCT, //!< Conjunct Forms.
  kCLIG, //!< Contextual Ligatures.
  kCPCT, //!< Centered CJK Punctuation.
  kCPSP, //!< Capital Spacing.
  kCSWH, //!< Contextual Swash.
  kCURS, //!< Cursive Positioning.
  kCV01, //!< Character Variant 1.
  kCV02, //!< Character Variant 2.
  kCV03, //!< Character Variant 3.
  kCV04, //!< Character Variant 4.
  kCV05, //!< Character Variant 5.
  kCV06, //!< Character Variant 6.
  kCV07, //!< Character Variant 7.
  kCV08, //!< Character Variant 8.
  kCV09, //!< Character Variant 9.
  kCV10, //!< Character Variant 10.
  kCV11, //!< Character Variant 11.
  kCV12, //!< Character Variant 12.
  kCV13, //!< Character Variant 13.
  kCV14, //!< Character Variant 14.
  kCV15, //!< Character Variant 15.
  kCV16, //!< Character Variant 16.
  kCV17, //!< Character Variant 17.
  kCV18, //!< Character Variant 18.
  kCV19, //!< Character Variant 19.
  kCV20, //!< Character Variant 20.
  kCV21, //!< Character Variant 21.
  kCV22, //!< Character Variant 22.
  kCV23, //!< Character Variant 23.
  kCV24, //!< Character Variant 24.
  kCV25, //!< Character Variant 25.
  kCV26, //!< Character Variant 26.
  kCV27, //!< Character Variant 27.
  kCV28, //!< Character Variant 28.
  kCV29, //!< Character Variant 29.
  kCV30, //!< Character Variant 30.
  kCV31, //!< Character Variant 31.
  kCV32, //!< Character Variant 32.
  kCV33, //!< Character Variant 33.
  kCV34, //!< Character Variant 34.
  kCV35, //!< Character Variant 35.
  kCV36, //!< Character Variant 36.
  kCV37, //!< Character Variant 37.
  kCV38, //!< Character Variant 38.
  kCV39, //!< Character Variant 39.
  kCV40, //!< Character Variant 40.
  kCV41, //!< Character Variant 41.
  kCV42, //!< Character Variant 42.
  kCV43, //!< Character Variant 43.
  kCV44, //!< Character Variant 44.
  kCV45, //!< Character Variant 45.
  kCV46, //!< Character Variant 46.
  kCV47, //!< Character Variant 47.
  kCV48, //!< Character Variant 48.
  kCV49, //!< Character Variant 49.
  kCV50, //!< Character Variant 50.
  kCV51, //!< Character Variant 51.
  kCV52, //!< Character Variant 52.
  kCV53, //!< Character Variant 53.
  kCV54, //!< Character Variant 54.
  kCV55, //!< Character Variant 55.
  kCV56, //!< Character Variant 56.
  kCV57, //!< Character Variant 57.
  kCV58, //!< Character Variant 58.
  kCV59, //!< Character Variant 59.
  kCV60, //!< Character Variant 60.
  kCV61, //!< Character Variant 61.
  kCV62, //!< Character Variant 62.
  kCV63, //!< Character Variant 63.
  kCV64, //!< Character Variant 64.
  kCV65, //!< Character Variant 65.
  kCV66, //!< Character Variant 66.
  kCV67, //!< Character Variant 67.
  kCV68, //!< Character Variant 68.
  kCV69, //!< Character Variant 69.
  kCV70, //!< Character Variant 70.
  kCV71, //!< Character Variant 71.
  kCV72, //!< Character Variant 72.
  kCV73, //!< Character Variant 73.
  kCV74, //!< Character Variant 74.
  kCV75, //!< Character Variant 75.
  kCV76, //!< Character Variant 76.
  kCV77, //!< Character Variant 77.
  kCV78, //!< Character Variant 78.
  kCV79, //!< Character Variant 79.
  kCV80, //!< Character Variant 80.
  kCV81, //!< Character Variant 81.
  kCV82, //!< Character Variant 82.
  kCV83, //!< Character Variant 83.
  kCV84, //!< Character Variant 84.
  kCV85, //!< Character Variant 85.
  kCV86, //!< Character Variant 86.
  kCV87, //!< Character Variant 87.
  kCV88, //!< Character Variant 88.
  kCV89, //!< Character Variant 89.
  kCV90, //!< Character Variant 90.
  kCV91, //!< Character Variant 91.
  kCV92, //!< Character Variant 92.
  kCV93, //!< Character Variant 93.
  kCV94, //!< Character Variant 94.
  kCV95, //!< Character Variant 95.
  kCV96, //!< Character Variant 96.
  kCV97, //!< Character Variant 97.
  kCV98, //!< Character Variant 98.
  kCV99, //!< Character Variant 99.
  kDIST, //!< Distances.
  kDLIG, //!< Discretionary Ligatures.
  kDNOM, //!< Denominators.
  kDTLS, //!< Dotless Forms.
  kEXPT, //!< Expert Forms.
  kFALT, //!< Final Glyph on Line Alternates.
  kFIN2, //!< Terminal Forms #2.
  kFIN3, //!< Terminal Forms #3.
  kFINA, //!< Terminal Forms.
  kFLAC, //!< Flattened accent forms.
  kFRAC, //!< Fractions.
  kFWID, //!< Full Widths.
  kHALF, //!< Half Forms.
  kHALN, //!< Halant Forms.
  kHALT, //!< Alternate Half Widths.
  kHIST, //!< Historical Forms.
  kHKNA, //!< Horizontal Kana Alternates.
  kHLIG, //!< Historical Ligatures.
  kHNGL, //!< Hangul.
  kHOJO, //!< Hojo Kanji Forms (JIS X 0212-1990 Kanji Forms).
  kHWID, //!< Half Widths.
  kINIT, //!< Initial Forms.
  kISOL, //!< Isolated Forms.
  kITAL, //!< Italics.
  kJALT, //!< Justification Alternates.
  kJP04, //!< JIS2004 Forms.
  kJP78, //!< JIS78 Forms.
  kJP83, //!< JIS83 Forms.
  kJP90, //!< JIS90 Forms.
  kKERN, //!< Kerning.
  kLFBD, //!< Left Bounds.
  kLIGA, //!< Standard Ligatures.
  kLJMO, //!< Leading Jamo Forms.
  kLNUM, //!< Lining Figures.
  kLOCL, //!< Localized Forms.
  kLTRA, //!< Left-to-right alternates.
  kLTRM, //!< Left-to-right mirrored forms.
  kMARK, //!< Mark Positioning.
  kMED2, //!< Medial Forms #2.
  kMEDI, //!< Medial Forms.
  kMGRK, //!< Mathematical Greek.
  kMKMK, //!< Mark to Mark Positioning.
  kMSET, //!< Mark Positioning via Substitution.
  kNALT, //!< Alternate Annotation Forms.
  kNLCK, //!< NLC Kanji Forms.
  kNUKT, //!< Nukta Forms.
  kNUMR, //!< Numerators.
  kONUM, //!< Oldstyle Figures.
  kOPBD, //!< Optical Bounds.
  kORDN, //!< Ordinals.
  kORNM, //!< Ornaments.
  kPALT, //!< Proportional Alternate Widths.
  kPCAP, //!< Petite Capitals.
  kPKNA, //!< Proportional Kana.
  kPNUM, //!< Proportional Figures.
  kPREF, //!< Pre-Base Forms.
  kPRES, //!< Pre-base Substitutions.
  kPSTF, //!< Post-base Forms.
  kPSTS, //!< Post-base Substitutions.
  kPWID, //!< Proportional Widths.
  kQWID, //!< Quarter Widths.
  kRAND, //!< Randomize.
  kRCLT, //!< Required Contextual Alternates.
  kRKRF, //!< Rakar Forms.
  kRLIG, //!< Required Ligatures.
  kRPHF, //!< Reph Forms.
  kRTBD, //!< Right Bounds.
  kRTLA, //!< Right-to-left alternates.
  kRTLM, //!< Right-to-left mirrored forms.
  kRUBY, //!< Ruby Notation Forms.
  kRVRN, //!< Required Variation Alternates.
  kSALT, //!< Stylistic Alternates.
  kSINF, //!< Scientific Inferiors.
  kSIZE, //!< Optical size.
  kSMCP, //!< Small Capitals.
  kSMPL, //!< Simplified Forms.
  kSS01, //!< Stylistic Set 1.
  kSS02, //!< Stylistic Set 2.
  kSS03, //!< Stylistic Set 3.
  kSS04, //!< Stylistic Set 4.
  kSS05, //!< Stylistic Set 5.
  kSS06, //!< Stylistic Set 6.
  kSS07, //!< Stylistic Set 7.
  kSS08, //!< Stylistic Set 8.
  kSS09, //!< Stylistic Set 9.
  kSS10, //!< Stylistic Set 10.
  kSS11, //!< Stylistic Set 11.
  kSS12, //!< Stylistic Set 12.
  kSS13, //!< Stylistic Set 13.
  kSS14, //!< Stylistic Set 14.
  kSS15, //!< Stylistic Set 15.
  kSS16, //!< Stylistic Set 16.
  kSS17, //!< Stylistic Set 17.
  kSS18, //!< Stylistic Set 18.
  kSS19, //!< Stylistic Set 19.
  kSS20, //!< Stylistic Set 20.
  kSSTY, //!< Math script style alternates.
  kSTCH, //!< Stretching Glyph Decomposition.
  kSUBS, //!< Subscript.
  kSUPS, //!< Superscript.
  kSWSH, //!< Swash.
  kTITL, //!< Titling.
  kTJMO, //!< Trailing Jamo Forms.
  kTNAM, //!< Traditional Name Forms.
  kTNUM, //!< Tabular Figures.
  kTRAD, //!< Traditional Forms.
  kTWID, //!< Third Widths.
  kUNIC, //!< Unicase.
  kVALT, //!< Alternate Vertical Metrics.
  kVATU, //!< Vattu Variants.
  kVCHW, //!< Vertical Contextual Half-width Spacing.
  kVERT, //!< Vertical Writing.
  kVHAL, //!< Alternate Vertical Half Metrics.
  kVJMO, //!< Vowel Jamo Forms.
  kVKNA, //!< Vertical Kana Alternates.
  kVKRN, //!< Vertical Kerning.
  kVPAL, //!< Proportional Alternate Vertical Metrics.
  kVRT2, //!< Vertical Alternates and Rotation.
  kVRTR, //!< Vertical Alternates for Rotation.
  kZERO, //!< Slashed Zero.

  kMaxValue = kZERO
};

static constexpr uint32_t kFeatureIdCount = uint32_t(FeatureId::kMaxValue) + 1u;

BL_HIDDEN extern const BLTag feature_id_to_tag_table[];

// bl::FontTagData - Baseline IDs
// ==============================

//! Font baseline tag translated to a unique ID.
enum class BaselineId : uint32_t {
  kICFC, //!< Ideographic face center.
  kIDCE, //!< Ideographic em-box center.
  kHANG, //!< The hanging baseline.
  kICFB, //!< Ideographic face bottom edge.
  kICFT, //!< Ideographic face top edge.
  kIDEO, //!< Ideographic em-box bottom edge.
  kIDTP, //!< Ideographic em-box top edge baseline.
  kMATH, //!< Math characters baseline.
  kROMN, //!< Alphabetic scripts baseline (Latin, Cyrillic, Greek, ...).

  kMaxValue = kROMN
};

static constexpr uint32_t kBaselineIdCount = uint32_t(BaselineId::kMaxValue) + 1u;

BL_HIDDEN extern const BLTag baseline_id_to_tag_table[];

// bl::FontTagData - Variation IDs
// ===============================

//! Font variation tag translated to a unique ID.
enum class VariationId : uint32_t {
  kITAL, //!< Italic.
  kOPSZ, //!< Optical size.
  kSLNT, //!< Slant.
  kWDTH, //!< Width.
  kWGHT, //!< Weight.

  kMaxValue = kWGHT
};

static constexpr uint32_t kVariationIdCount = uint32_t(VariationId::kMaxValue) + 1u;

BL_HIDDEN extern const BLTag variation_id_to_tag_table[];

// bl::FontTagData - Generated
// ===========================

// The following code was generated by `bl_generate`.

// {bl::FontTagData::Generated::Begin}
static BL_INLINE uint32_t table_tag_to_id(uint32_t tag) noexcept {
  static const uint8_t hash_table[128] = {
    44, 0, 0, 26, 0, 0, 0, 0, 0, 0, 71, 67, 0, 0, 24, 6, 46, 0, 43, 30, 29, 39, 1, 49,
    0, 0, 0, 69, 31, 19, 14, 63, 0, 0, 52, 0, 0, 0, 40, 0, 17, 0, 34, 0, 47, 33, 28, 0,
    37, 62, 20, 0, 11, 0, 50, 0, 18, 0, 0, 0, 0, 38, 70, 2, 5, 13, 0, 0, 0, 23, 36, 0,
    22, 8, 0, 0, 7, 45, 0, 64, 48, 0, 25, 55, 0, 0, 41, 16, 10, 51, 59, 0, 0, 57, 0, 66,
    61, 0, 58, 0, 4, 0, 0, 0, 21, 53, 0, 15, 0, 35, 32, 0, 0, 68, 9, 0, 60, 0, 42, 0,
    54, 0, 27, 0, 0, 3, 56, 12
  };

  uint32_t h1 = (tag * 907879475u) >> 25u;
  uint32_t i1 = hash_table[h1];
  uint32_t index = 0xFFFFFFFFu;

  if (table_id_to_tag_table[i1] == tag)
    index = i1;

  if (tag == 1886545264)
    index = 65;

  return index;
}

static BL_INLINE uint32_t script_tag_to_id(uint32_t tag) noexcept {
  static const uint8_t hash_table[256] = {
    45, 0, 162, 127, 94, 21, 140, 113, 122, 0, 0, 50, 65, 4, 126, 32, 124, 107, 78, 59, 0, 133, 128, 20,
    101, 120, 0, 51, 54, 55, 83, 0, 156, 154, 97, 144, 0, 109, 52, 6, 74, 151, 145, 23, 90, 73, 0, 42,
    121, 79, 149, 115, 158, 164, 0, 112, 30, 85, 46, 129, 0, 63, 0, 136, 40, 160, 0, 93, 138, 91, 71, 44,
    26, 68, 170, 0, 29, 0, 49, 146, 0, 33, 87, 0, 130, 11, 98, 0, 141, 36, 110, 0, 152, 105, 89, 18,
    72, 69, 0, 53, 116, 0, 150, 96, 31, 1, 15, 0, 153, 171, 0, 142, 0, 75, 111, 84, 157, 0, 16, 0,
    0, 148, 134, 48, 0, 10, 0, 125, 80, 0, 0, 0, 39, 0, 0, 0, 88, 41, 0, 0, 147, 5, 28, 132,
    57, 38, 167, 19, 0, 17, 0, 0, 169, 95, 123, 0, 0, 0, 70, 47, 99, 0, 159, 67, 0, 0, 7, 8,
    0, 0, 131, 0, 0, 168, 62, 0, 161, 165, 0, 25, 27, 0, 155, 92, 0, 0, 137, 61, 0, 106, 0, 0,
    0, 143, 117, 56, 119, 3, 0, 0, 82, 0, 0, 102, 118, 0, 0, 77, 0, 0, 0, 86, 2, 0, 0, 64,
    14, 135, 114, 43, 139, 0, 100, 163, 0, 24, 0, 60, 104, 108, 0, 9, 34, 103, 0, 0, 172, 0, 0, 0,
    0, 37, 35, 22, 0, 166, 13, 0, 12, 76, 81, 66, 0, 0, 58, 0
  };

  uint32_t h1 = (tag * 516816048u) >> 24u;
  uint32_t h2 = (tag * 670046653u) >> 24u;

  uint32_t i1 = hash_table[h1];
  uint32_t i2 = hash_table[h2];

  uint32_t index = 0xFFFFFFFFu;

  if (script_id_to_tag_table[i1] == tag)
    index = i1;

  if (script_id_to_tag_table[i2] == tag)
    index = i2;

  return index;
}

static BL_INLINE uint32_t language_tag_to_id(uint32_t tag) noexcept {
  static const uint16_t hashTable1[1024] = {
    0, 0, 0, 448, 0, 438, 549, 146, 0, 332, 0, 0, 0, 498, 0, 572, 478, 0, 190, 410, 387, 271, 0, 0,
    265, 430, 46, 0, 0, 596, 0, 585, 0, 0, 346, 232, 480, 293, 390, 286, 0, 0, 82, 0, 0, 472, 0, 50,
    0, 0, 0, 216, 100, 0, 244, 0, 151, 0, 0, 89, 65, 0, 0, 30, 135, 263, 210, 53, 0, 217, 0, 43,
    0, 12, 579, 114, 132, 0, 0, 198, 536, 0, 237, 563, 136, 211, 0, 0, 0, 0, 0, 0, 142, 423, 0, 0,
    0, 0, 0, 0, 0, 192, 0, 466, 0, 376, 164, 0, 399, 447, 122, 40, 364, 226, 0, 0, 586, 347, 328, 0,
    196, 0, 131, 84, 0, 0, 403, 371, 55, 126, 0, 0, 629, 582, 507, 0, 0, 0, 0, 458, 0, 6, 0, 544,
    565, 0, 434, 0, 274, 298, 0, 0, 0, 0, 154, 249, 0, 0, 396, 504, 0, 0, 0, 0, 0, 0, 276, 383,
    79, 474, 0, 157, 440, 341, 518, 0, 0, 0, 0, 577, 623, 0, 479, 302, 0, 67, 449, 0, 0, 273, 149, 0,
    344, 520, 0, 427, 0, 0, 0, 0, 349, 0, 7, 386, 0, 0, 61, 452, 0, 0, 0, 56, 127, 21, 0, 0,
    105, 424, 0, 202, 0, 395, 0, 0, 0, 456, 490, 0, 0, 0, 405, 556, 614, 339, 372, 469, 607, 0, 208, 0,
    188, 14, 115, 0, 0, 0, 72, 0, 0, 0, 0, 385, 375, 0, 0, 52, 218, 0, 167, 107, 102, 358, 116, 484,
    0, 75, 0, 193, 68, 166, 0, 562, 0, 0, 400, 0, 0, 108, 206, 516, 546, 0, 485, 0, 352, 0, 499, 0,
    454, 5, 64, 0, 0, 0, 272, 0, 0, 0, 0, 0, 508, 0, 0, 0, 0, 309, 113, 9, 0, 0, 96, 0,
    435, 0, 212, 0, 160, 0, 191, 0, 0, 0, 0, 106, 422, 32, 0, 0, 0, 300, 225, 0, 0, 165, 560, 163,
    0, 0, 444, 343, 45, 169, 189, 363, 0, 0, 13, 0, 241, 0, 194, 292, 130, 0, 245, 531, 463, 0, 0, 90,
    49, 20, 517, 147, 215, 143, 0, 354, 0, 481, 229, 322, 0, 239, 3, 0, 316, 57, 555, 0, 0, 155, 47, 19,
    42, 591, 248, 11, 186, 327, 0, 199, 492, 0, 0, 569, 236, 0, 0, 541, 0, 25, 124, 15, 0, 0, 0, 0,
    0, 0, 0, 325, 73, 0, 0, 0, 0, 0, 0, 59, 0, 398, 446, 519, 0, 0, 255, 311, 0, 578, 502, 0,
    455, 139, 240, 584, 287, 213, 2, 0, 0, 0, 0, 172, 0, 0, 628, 16, 0, 77, 0, 603, 330, 112, 0, 0,
    571, 0, 60, 222, 0, 0, 429, 559, 0, 0, 587, 511, 0, 0, 331, 187, 0, 0, 0, 182, 0, 70, 618, 0,
    275, 0, 161, 374, 58, 439, 22, 340, 0, 0, 426, 34, 542, 576, 0, 0, 0, 0, 177, 0, 281, 0, 617, 529,
    0, 0, 0, 258, 0, 144, 515, 0, 361, 0, 0, 305, 0, 0, 453, 537, 0, 62, 0, 0, 80, 243, 0, 0,
    0, 0, 583, 117, 0, 0, 432, 0, 0, 0, 0, 92, 0, 0, 223, 175, 0, 0, 593, 0, 0, 48, 512, 118,
    250, 0, 0, 443, 505, 0, 0, 299, 0, 0, 389, 0, 0, 162, 134, 543, 0, 0, 461, 581, 433, 203, 35, 357,
    0, 0, 0, 0, 0, 227, 0, 282, 0, 0, 380, 267, 525, 552, 123, 0, 205, 0, 120, 0, 580, 351, 0, 394,
    296, 633, 413, 0, 63, 381, 0, 314, 24, 554, 0, 608, 0, 0, 17, 0, 459, 0, 0, 0, 179, 138, 181, 477,
    178, 224, 0, 0, 0, 0, 373, 345, 0, 337, 119, 44, 141, 0, 0, 506, 200, 0, 97, 494, 0, 238, 319, 0,
    475, 0, 158, 0, 0, 342, 0, 0, 204, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 284, 599, 0, 609, 370,
    0, 0, 0, 588, 256, 0, 39, 99, 0, 36, 0, 150, 391, 76, 88, 436, 0, 535, 589, 0, 262, 0, 0, 570,
    257, 0, 41, 366, 0, 0, 125, 231, 0, 0, 66, 0, 83, 0, 0, 0, 0, 527, 0, 367, 0, 0, 252, 0,
    0, 0, 33, 0, 0, 324, 137, 514, 540, 0, 388, 280, 0, 406, 377, 133, 0, 214, 0, 0, 254, 482, 38, 0,
    0, 233, 303, 195, 294, 611, 0, 0, 0, 534, 312, 54, 0, 521, 338, 111, 0, 153, 101, 355, 246, 329, 0, 0,
    0, 0, 489, 0, 0, 174, 0, 264, 428, 269, 0, 0, 0, 0, 0, 0, 0, 501, 29, 228, 95, 0, 0, 0,
    414, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 417, 392, 0, 278, 0, 412, 219,
    0, 378, 0, 0, 0, 170, 510, 0, 626, 103, 0, 0, 348, 304, 197, 4, 0, 0, 488, 464, 0, 404, 0, 23,
    260, 0, 0, 176, 104, 0, 0, 201, 308, 0, 8, 419, 0, 91, 0, 279, 317, 0, 0, 0, 0, 0, 606, 437,
    0, 0, 0, 0, 0, 31, 209, 575, 416, 71, 0, 290, 0, 0, 384, 27, 0, 0, 441, 0, 0, 0, 81, 0,
    0, 0, 98, 326, 74, 180, 0, 0, 411, 568, 129, 173, 0, 266, 0, 0, 0, 0, 550, 462, 0, 0, 503, 350,
    602, 0, 148, 0, 0, 85, 600, 0, 26, 0, 128, 51, 207, 0, 0, 0, 253, 365, 0, 0, 0, 0, 574, 0,
    93, 0, 601, 450, 1, 0, 533, 78, 0, 0, 0, 0, 145, 0, 360, 0, 184, 0, 0, 0, 493, 0, 0, 0,
    277, 0, 28, 0, 0, 0, 369, 285, 0, 168, 0, 625, 0, 0, 0, 0, 0, 630, 291, 0, 283, 0, 0, 0,
    268, 0, 242, 171, 0, 0, 140, 0, 0, 0, 353, 306, 457, 0, 0, 289, 87, 221, 564, 0, 0, 0, 261, 0,
    0, 121, 18, 336, 0, 247, 359, 185, 230, 10, 234, 94, 0, 69, 318, 0
  };

  static const uint16_t hashTable2[512] = {
    0, 0, 627, 313, 0, 335, 0, 408, 0, 595, 0, 0, 0, 0, 321, 0, 415, 0, 0, 613, 471, 0, 483, 0,
    0, 0, 532, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 526, 0, 0, 0, 0, 0, 0, 270,
    0, 0, 86, 0, 451, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 597, 0, 0, 0,
    0, 0, 624, 0, 0, 0, 0, 0, 530, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 539, 497, 323, 0, 159,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 598, 620, 183, 523, 0, 0, 0, 0, 0, 0, 402, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 513, 0, 470, 0, 407, 0, 251, 0, 0, 0, 612, 465, 0, 0, 592,
    409, 0, 0, 0, 0, 491, 0, 0, 109, 0, 0, 0, 0, 0, 0, 0, 0, 558, 379, 0, 0, 0, 619, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 528,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 310, 0, 0, 594, 0, 0, 288, 0, 0, 356, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 500, 259, 0, 0, 553, 0, 0, 0, 397, 0, 545, 0,
    0, 0, 0, 0, 0, 382, 0, 0, 0, 420, 0, 605, 152, 0, 0, 616, 0, 37, 561, 0, 0, 0, 295, 0,
    476, 362, 0, 0, 0, 487, 538, 0, 0, 0, 0, 0, 524, 156, 547, 0, 0, 0, 509, 0, 0, 0, 0, 0,
    301, 220, 0, 0, 0, 0, 467, 334, 610, 567, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 431, 0, 320, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 393,
    0, 0, 0, 0, 401, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 621, 0, 0,
    0, 0, 0, 573, 473, 0, 0, 0, 0, 0, 0, 0, 557, 297, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 110, 551, 0, 445, 0, 460, 0, 0, 0, 0, 0, 425, 0, 368, 0, 0, 0, 0, 590, 0, 0, 0, 0,
    632, 0, 0, 495, 0, 0, 0, 0, 442, 0, 0, 0, 0, 0, 0, 566, 468, 0, 0, 0, 0, 631, 522, 0,
    0, 0, 333, 0, 0, 0, 0, 0, 0, 0, 421, 548, 622, 235, 0, 0, 0, 0, 0, 0, 0, 307, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 604, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 315, 0, 0, 0, 0, 486, 0, 0, 0, 0, 0, 0, 0, 418, 0, 0, 615, 0,
    0, 0, 0, 496, 0, 0, 0, 0
  };

  uint32_t h1 = (tag * 78747468u) >> 22u;
  uint32_t h2 = (tag * 3168659u) >> 23u;

  uint32_t i1 = hashTable1[h1];
  uint32_t i2 = hashTable2[h2];

  uint32_t index = 0xFFFFFFFFu;

  if (language_id_to_tag_table[i1] == tag)
    index = i1;

  if (language_id_to_tag_table[i2] == tag)
    index = i2;

  return index;
}

static BL_INLINE uint32_t feature_tag_to_id(uint32_t tag) noexcept {
  static const uint8_t hashTable1[256] = {
    30, 0, 40, 160, 50, 0, 60, 0, 70, 225, 80, 0, 90, 0, 100, 0, 110, 193, 120, 0, 204, 144, 23, 156,
    33, 0, 43, 0, 53, 186, 63, 139, 73, 0, 83, 163, 93, 137, 103, 184, 113, 166, 197, 145, 207, 0, 27, 143,
    37, 175, 47, 0, 57, 129, 67, 0, 77, 0, 87, 168, 97, 0, 107, 135, 117, 240, 201, 0, 211, 0, 0, 124,
    0, 142, 149, 0, 0, 190, 0, 15, 146, 0, 0, 126, 0, 3, 0, 0, 0, 154, 205, 127, 24, 136, 34, 13,
    44, 8, 54, 20, 64, 12, 74, 170, 84, 234, 94, 133, 104, 191, 114, 0, 198, 11, 208, 226, 28, 0, 38, 9,
    48, 0, 58, 233, 68, 10, 78, 182, 88, 231, 98, 0, 108, 0, 118, 0, 202, 0, 212, 237, 31, 187, 41, 0,
    51, 150, 61, 125, 71, 0, 81, 0, 91, 235, 101, 134, 111, 123, 0, 219, 0, 128, 25, 158, 35, 0, 45, 0,
    55, 0, 65, 0, 75, 130, 85, 161, 95, 2, 105, 229, 115, 0, 199, 222, 209, 131, 29, 220, 39, 7, 49, 195,
    59, 138, 69, 221, 79, 173, 89, 1, 99, 0, 109, 0, 119, 153, 203, 188, 22, 192, 32, 6, 42, 174, 52, 0,
    62, 14, 72, 0, 82, 179, 92, 183, 102, 224, 112, 194, 0, 0, 206, 0, 26, 122, 36, 152, 46, 5, 56, 172,
    66, 155, 76, 17, 86, 4, 96, 0, 106, 140, 116, 0, 200, 0, 210, 0
  };

  static const uint8_t hashTable2[64] = {
    230, 0, 213, 214, 215, 16, 0, 147, 0, 0, 0, 162, 178, 0, 167, 238, 169, 0, 0, 0, 217, 0, 0, 227,
    159, 132, 232, 157, 181, 0, 223, 18, 165, 19, 218, 185, 121, 0, 0, 0, 0, 176, 0, 141, 0, 0, 216, 189,
    236, 196, 177, 180, 148, 164, 0, 171, 0, 228, 0, 0, 239, 21, 0, 151
  };

  uint32_t h1 = (tag * 1174536694u) >> 24u;
  uint32_t h2 = (tag * 46218148u) >> 26u;

  uint32_t i1 = hashTable1[h1];
  uint32_t i2 = hashTable2[h2];

  uint32_t index = 0xFFFFFFFFu;

  if (feature_id_to_tag_table[i1] == tag)
    index = i1;

  if (feature_id_to_tag_table[i2] == tag)
    index = i2;

  return index;
}

static BL_INLINE uint32_t baseline_tag_to_id(uint32_t tag) noexcept {
  static const uint8_t hash_table[16] = {
    0, 1, 5, 0, 3, 4, 0, 0, 0, 2, 0, 0, 8, 0, 7, 6
  };

  uint32_t h1 = (tag * 3145789u) >> 28u;
  uint32_t i1 = hash_table[h1];
  uint32_t index = 0xFFFFFFFFu;

  if (baseline_id_to_tag_table[i1] == tag)
    index = i1;

  return index;
}

static BL_INLINE uint32_t variation_tag_to_id(uint32_t tag) noexcept {
  static const uint8_t hash_table[8] = {
    0, 0, 0, 3, 4, 2, 1, 0
  };

  uint32_t h1 = (tag * 119u) >> 29u;
  uint32_t i1 = hash_table[h1];
  uint32_t index = 0xFFFFFFFFu;

  if (variation_id_to_tag_table[i1] == tag)
    index = i1;

  return index;
}
// {bl::FontTagData::Generated::End}

} // {FontTagData}
} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_FONTTAGDATAIDS_P_H_INCLUDED
