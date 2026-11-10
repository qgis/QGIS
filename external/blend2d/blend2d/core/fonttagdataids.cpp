// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/fonttagdataids_p.h>

namespace bl {
namespace FontTagData {

// bl::FontTagData - Table IDs
// ===========================

const BLTag table_id_to_tag_table[] = {
  BL_MAKE_TAG('B', 'A', 'S', 'E'), // kBASE - Baseline data                      (OpenType advanced typography)
  BL_MAKE_TAG('C', 'B', 'D', 'T'), // kCBDT - Color bitmap data                  (Color fonts)
  BL_MAKE_TAG('C', 'B', 'L', 'C'), // kCBLC - Color bitmap location data         (Color fonts)
  BL_MAKE_TAG('C', 'F', 'F', ' '), // kCFF  - Compact Font Format 1.0            (CFF outlines)
  BL_MAKE_TAG('C', 'F', 'F', '2'), // kCFF2 - Compact Font Format 2.0            (CFF outlines)
  BL_MAKE_TAG('C', 'O', 'L', 'R'), // kCOLR - Color table                        (Color fonts)
  BL_MAKE_TAG('C', 'P', 'A', 'L'), // kCPAL - Color palette table                (Color fonts)
  BL_MAKE_TAG('D', 'S', 'I', 'G'), // kDSIG - Digital signature                  (Optional)
  BL_MAKE_TAG('E', 'B', 'D', 'T'), // kEBDT - Embedded bitmap data               (Bitmap glyphs)
  BL_MAKE_TAG('E', 'B', 'L', 'C'), // kEBLC - Embedded bitmap location data      (Bitmap glyphs)
  BL_MAKE_TAG('E', 'B', 'S', 'C'), // kEBSC - Embedded bitmap scaling data       (Bitmap glyphs)
  BL_MAKE_TAG('G', 'D', 'E', 'F'), // kGDEF - Glyph definition data              (OpenType advanced typography)
  BL_MAKE_TAG('G', 'P', 'O', 'S'), // kGPOS - Glyph positioning data             (OpenType advanced typography)
  BL_MAKE_TAG('G', 'S', 'U', 'B'), // kGSUB - Glyph substitution data            (OpenType advanced typography)
  BL_MAKE_TAG('H', 'V', 'A', 'R'), // kHVAR - Horizontal metrics variations      (OpenType font variations)
  BL_MAKE_TAG('J', 'S', 'T', 'F'), // kJSTF - Justification data                 (OpenType advanced typography)
  BL_MAKE_TAG('L', 'T', 'S', 'H'), // kLTSH - Linear threshold data              (Other)
  BL_MAKE_TAG('M', 'A', 'T', 'H'), // kMATH - Math layout data                   (OpenType advanced typography)
  BL_MAKE_TAG('M', 'E', 'R', 'G'), // kMERG - Merge                              (Other)
  BL_MAKE_TAG('M', 'V', 'A', 'R'), // kMVAR - Metrics variations                 (OpenType font variations)
  BL_MAKE_TAG('O', 'S', '/', '2'), // kOS_2 - OS/2 and Windows specific metrics  (Required)
  BL_MAKE_TAG('P', 'C', 'L', 'T'), // kPCLT - PCL 5 data                         (Other)
  BL_MAKE_TAG('S', 'T', 'A', 'T'), // kSTAT - Style attributes                   (OpenType font variations)
  BL_MAKE_TAG('S', 'V', 'G', ' '), // kSVG  - SVG table                          (SVG outlines)
  BL_MAKE_TAG('V', 'D', 'M', 'X'), // kVDMX - Vertical device metrics            (Metrics)
  BL_MAKE_TAG('V', 'O', 'R', 'G'), // kVORG - Vertical origin                    (CFF outlines)
  BL_MAKE_TAG('V', 'V', 'A', 'R'), // kVVAR - Vertical metrics variations        (OpenType font variations)
  BL_MAKE_TAG('Z', 'a', 'p', 'f'), // kZAPF - ZAPF table                         (Apple advanced typography)
  BL_MAKE_TAG('a', 'c', 'n', 't'), // kACNT - Accent attachment table            (Apple advanced typography)
  BL_MAKE_TAG('a', 'n', 'k', 'r'), // kANKR - Anchor point table                 (Apple advanced typography)
  BL_MAKE_TAG('a', 'v', 'a', 'r'), // kAVAR - Axis variations                    (OpenType font variations)
  BL_MAKE_TAG('b', 'd', 'a', 't'), // kBDAT - Bitmap data table                  (Apple bitmap data)
  BL_MAKE_TAG('b', 'h', 'e', 'd'), // kBHED - Bitmap header table                (Apple bitmap data)
  BL_MAKE_TAG('b', 'l', 'o', 'c'), // kBLOC - Bitmap location table              (Apple bitmap data)
  BL_MAKE_TAG('b', 's', 'l', 'n'), // kBSLN - Baseline table                     (Apple advanced typography)
  BL_MAKE_TAG('c', 'm', 'a', 'p'), // kCMAP - Character to glyph mapping         (Required)
  BL_MAKE_TAG('c', 'v', 'a', 'r'), // kCVAR - CVT variations                     (OpenType font variations & TrueType outlines)
  BL_MAKE_TAG('c', 'v', 't', ' '), // kCVT  - Control value table                (TrueType outlines)
  BL_MAKE_TAG('f', 'd', 's', 'c'), // kFDSC - Font descriptors table             (Apple advanced typography)
  BL_MAKE_TAG('f', 'e', 'a', 't'), // kFEAT - Feature name table                 (Apple advanced typography)
  BL_MAKE_TAG('f', 'm', 't', 'x'), // kFMTX - Font metrics table                 (Apple advanced typography)
  BL_MAKE_TAG('f', 'o', 'n', 'd'), // kFOND - FOND table                         (Apple advanced typography)
  BL_MAKE_TAG('f', 'p', 'g', 'm'), // kFPGM - Font program                       (TrueType outlines)
  BL_MAKE_TAG('f', 'v', 'a', 'r'), // kFVAR - Font variations                    (OpenType font variations)
  BL_MAKE_TAG('g', 'a', 's', 'p'), // kGASP - Grid-fitting/Scan-conversion       (TrueType outlines)
  BL_MAKE_TAG('g', 'c', 'i', 'd'), // kGCID - Characters to CIDs mapping         (Apple advanced typography)
  BL_MAKE_TAG('g', 'l', 'y', 'f'), // kGLYF - Glyph data                         (TrueType outlines)
  BL_MAKE_TAG('g', 'v', 'a', 'r'), // kGVAR - Glyph variations                   (OpenType font variations & TrueType outlines)
  BL_MAKE_TAG('h', 'd', 'm', 'x'), // kHDMX - Horizontal device metrics          (Metrics)
  BL_MAKE_TAG('h', 'e', 'a', 'd'), // kHEAD - Font header                        (Required)
  BL_MAKE_TAG('h', 'h', 'e', 'a'), // kHHEA - Horizontal header                  (Required)
  BL_MAKE_TAG('h', 'm', 't', 'x'), // kHMTX - Horizontal metrics                 (Required)
  BL_MAKE_TAG('j', 'u', 's', 't'), // kJUST - Justification data                 (Apple advanced typography)
  BL_MAKE_TAG('k', 'e', 'r', 'n'), // kKERN - Legacy Kerning (not OpenType)      (Kerning)
  BL_MAKE_TAG('k', 'e', 'r', 'x'), // kKERX - Extended kerning table             (Apple advanced typography)
  BL_MAKE_TAG('l', 'c', 'a', 'r'), // kLCAR - Ligature caret table               (Apple advanced typography)
  BL_MAKE_TAG('l', 'o', 'c', 'a'), // kLOCA - Index to location                  (TrueType outlines)
  BL_MAKE_TAG('l', 't', 'a', 'g'), // kLTAG - Maps numeric codes and IETF tags   (Apple advanced typography)
  BL_MAKE_TAG('m', 'a', 'x', 'p'), // kMAXP - Maximum profile                    (Required)
  BL_MAKE_TAG('m', 'e', 't', 'a'), // kMETA - Metadata                           (Other)
  BL_MAKE_TAG('m', 'o', 'r', 't'), // kMORT - Glyph metamorphosis (Deprecated!)  (Apple advanced typography)
  BL_MAKE_TAG('m', 'o', 'r', 'x'), // kMORX - Extended glyph metamorphosis table (Apple advanced typography)
  BL_MAKE_TAG('n', 'a', 'm', 'e'), // kNAME - Naming table                       (Required)
  BL_MAKE_TAG('o', 'p', 'b', 'd'), // kOPBD - Optical bounds table               (Apple advanced typography)
  BL_MAKE_TAG('p', 'o', 's', 't'), // kPOST - PostScript information             (Required)
  BL_MAKE_TAG('p', 'r', 'e', 'p'), // kPREP - CVT program                        (TrueType outlines)
  BL_MAKE_TAG('p', 'r', 'o', 'p'), // kPROP - Glyph properties table             (Apple advanced typography)
  BL_MAKE_TAG('s', 'b', 'i', 'x'), // kSBIX - Standard bitmap graphics           (Bitmap glyphs)
  BL_MAKE_TAG('t', 'r', 'a', 'k'), // kTRAK - Tracking table                     (Apple advanced typography)
  BL_MAKE_TAG('v', 'h', 'e', 'a'), // kVHEA - Vertical Metrics header            (Metrics)
  BL_MAKE_TAG('v', 'm', 't', 'x'), // kVMTX - Vertical Metrics                   (Metrics)
  BL_MAKE_TAG('x', 'r', 'e', 'f')  // kXREF - Cross-reference table              (Apple advanced typography)
};

// bl::FontTagData - Script IDs
// ============================

const BLTag script_id_to_tag_table[] = {
  BL_MAKE_TAG('D', 'F', 'L', 'T'), // kDFLT - Default.

  BL_MAKE_TAG('a', 'd', 'l', 'm'), // kADLM - Adlam.
  BL_MAKE_TAG('a', 'g', 'h', 'b'), // kAGHB - Caucasian Albanian.
  BL_MAKE_TAG('a', 'h', 'o', 'm'), // kAHOM - Ahom.
  BL_MAKE_TAG('a', 'r', 'a', 'b'), // kARAB - Arabic.
  BL_MAKE_TAG('a', 'r', 'm', 'i'), // kARMI - Imperial Aramaic.
  BL_MAKE_TAG('a', 'r', 'm', 'n'), // kARMN - Armenian.
  BL_MAKE_TAG('a', 'v', 's', 't'), // kAVST - Avestan.
  BL_MAKE_TAG('b', 'a', 'l', 'i'), // kBALI - Balinese.
  BL_MAKE_TAG('b', 'a', 'm', 'u'), // kBAMU - Bamum.
  BL_MAKE_TAG('b', 'a', 's', 's'), // kBASS - Bassa Vah.
  BL_MAKE_TAG('b', 'a', 't', 'k'), // kBATK - Batak.
  BL_MAKE_TAG('b', 'e', 'n', 'g'), // kBENG - Bengali.
  BL_MAKE_TAG('b', 'h', 'k', 's'), // kBHKS - Bhaiksuki.
  BL_MAKE_TAG('b', 'n', 'g', '2'), // kBNG2 - Bengali v2.
  BL_MAKE_TAG('b', 'o', 'p', 'o'), // kBOPO - Bopomofo.
  BL_MAKE_TAG('b', 'r', 'a', 'h'), // kBRAH - Brahmi.
  BL_MAKE_TAG('b', 'r', 'a', 'i'), // kBRAI - Braille.
  BL_MAKE_TAG('b', 'u', 'g', 'i'), // kBUGI - Buginese.
  BL_MAKE_TAG('b', 'u', 'h', 'd'), // kBUHD - Buhid.
  BL_MAKE_TAG('b', 'y', 'z', 'm'), // kBYZM - Byzantine Music.
  BL_MAKE_TAG('c', 'a', 'k', 'm'), // kCAKM - Chakma.
  BL_MAKE_TAG('c', 'a', 'n', 's'), // kCANS - Canadian Syllabics.
  BL_MAKE_TAG('c', 'a', 'r', 'i'), // kCARI - Carian.
  BL_MAKE_TAG('c', 'h', 'a', 'm'), // kCHAM - Cham.
  BL_MAKE_TAG('c', 'h', 'e', 'r'), // kCHER - Cherokee.
  BL_MAKE_TAG('c', 'h', 'r', 's'), // kCHRS - Chorasmian.
  BL_MAKE_TAG('c', 'o', 'p', 't'), // kCOPT - Coptic.
  BL_MAKE_TAG('c', 'p', 'm', 'n'), // kCPMN - Cypro-Minoan.
  BL_MAKE_TAG('c', 'p', 'r', 't'), // kCPRT - Cypriot Syllabary.
  BL_MAKE_TAG('c', 'y', 'r', 'l'), // kCYRL - Cyrillic.
  BL_MAKE_TAG('d', 'e', 'v', '2'), // kDEV2 - Devanagari v2.
  BL_MAKE_TAG('d', 'e', 'v', 'a'), // kDEVA - Devanagari.
  BL_MAKE_TAG('d', 'i', 'a', 'k'), // kDIAK - Dives Akuru.
  BL_MAKE_TAG('d', 'o', 'g', 'r'), // kDOGR - Dogra.
  BL_MAKE_TAG('d', 's', 'r', 't'), // kDSRT - Deseret.
  BL_MAKE_TAG('d', 'u', 'p', 'l'), // kDUPL - Duployan.
  BL_MAKE_TAG('e', 'g', 'y', 'p'), // kEGYP - Egyptian Hieroglyphs.
  BL_MAKE_TAG('e', 'l', 'b', 'a'), // kELBA - Elbasan.
  BL_MAKE_TAG('e', 'l', 'y', 'm'), // kELYM - Elymaic.
  BL_MAKE_TAG('e', 't', 'h', 'i'), // kETHI - Ethiopic.
  BL_MAKE_TAG('g', 'e', 'o', 'r'), // kGEOR - Georgian.
  BL_MAKE_TAG('g', 'j', 'r', '2'), // kGJR2 - Gujarati v2.
  BL_MAKE_TAG('g', 'l', 'a', 'g'), // kGLAG - Glagolitic.
  BL_MAKE_TAG('g', 'o', 'n', 'g'), // kGONG - Gunjala Gondi.
  BL_MAKE_TAG('g', 'o', 'n', 'm'), // kGONM - Masaram Gondi.
  BL_MAKE_TAG('g', 'o', 't', 'h'), // kGOTH - Gothic.
  BL_MAKE_TAG('g', 'r', 'a', 'n'), // kGRAN - Grantha.
  BL_MAKE_TAG('g', 'r', 'e', 'k'), // kGREK - Greek.
  BL_MAKE_TAG('g', 'u', 'j', 'r'), // kGUJR - Gujarati.
  BL_MAKE_TAG('g', 'u', 'r', '2'), // kGUR2 - Gurmukhi v2.
  BL_MAKE_TAG('g', 'u', 'r', 'u'), // kGURU - Gurmukhi.
  BL_MAKE_TAG('h', 'a', 'n', 'g'), // kHANG - Hangul.
  BL_MAKE_TAG('h', 'a', 'n', 'i'), // kHANI - CJK Ideographic.
  BL_MAKE_TAG('h', 'a', 'n', 'o'), // kHANO - Hanunoo.
  BL_MAKE_TAG('h', 'a', 't', 'r'), // kHATR - Hatran.
  BL_MAKE_TAG('h', 'e', 'b', 'r'), // kHEBR - Hebrew.
  BL_MAKE_TAG('h', 'l', 'u', 'w'), // kHLUW - Anatolian Hieroglyphs.
  BL_MAKE_TAG('h', 'm', 'n', 'g'), // kHMNG - Pahawh Hmong.
  BL_MAKE_TAG('h', 'm', 'n', 'p'), // kHMNP - Nyiakeng Puachue Hmong.
  BL_MAKE_TAG('h', 'u', 'n', 'g'), // kHUNG - Old Hungarian.
  BL_MAKE_TAG('i', 't', 'a', 'l'), // kITAL - Old Italic.
  BL_MAKE_TAG('j', 'a', 'm', 'o'), // kJAMO - Hangul Jamo.
  BL_MAKE_TAG('j', 'a', 'v', 'a'), // kJAVA - Javanese.
  BL_MAKE_TAG('k', 'a', 'l', 'i'), // kKALI - Kayah Li.
  BL_MAKE_TAG('k', 'a', 'n', 'a'), // kKANA - Katakana, Hiragana.
  BL_MAKE_TAG('k', 'h', 'a', 'r'), // kKHAR - Kharosthi.
  BL_MAKE_TAG('k', 'h', 'm', 'r'), // kKHMR - Khmer.
  BL_MAKE_TAG('k', 'h', 'o', 'j'), // kKHOJ - Khojki.
  BL_MAKE_TAG('k', 'i', 't', 's'), // kKITS - Khitan Small Script.
  BL_MAKE_TAG('k', 'n', 'd', '2'), // kKND2 - Kannada v2.
  BL_MAKE_TAG('k', 'n', 'd', 'a'), // kKNDA - Kannada.
  BL_MAKE_TAG('k', 't', 'h', 'i'), // kKTHI - Kaithi.
  BL_MAKE_TAG('l', 'a', 'n', 'a'), // kLANA - Tai Tham (Lanna)
  BL_MAKE_TAG('l', 'a', 'o', ' '), // kLAO  - Lao.
  BL_MAKE_TAG('l', 'a', 't', 'n'), // kLATN - Latin.
  BL_MAKE_TAG('l', 'e', 'p', 'c'), // kLEPC - Lepcha.
  BL_MAKE_TAG('l', 'i', 'm', 'b'), // kLIMB - Limbu.
  BL_MAKE_TAG('l', 'i', 'n', 'a'), // kLINA - Linear A.
  BL_MAKE_TAG('l', 'i', 'n', 'b'), // kLINB - Linear B.
  BL_MAKE_TAG('l', 'i', 's', 'u'), // kLISU - Lisu (Fraser)
  BL_MAKE_TAG('l', 'y', 'c', 'i'), // kLYCI - Lycian.
  BL_MAKE_TAG('l', 'y', 'd', 'i'), // kLYDI - Lydian.
  BL_MAKE_TAG('m', 'a', 'h', 'j'), // kMAHJ - Mahajani.
  BL_MAKE_TAG('m', 'a', 'k', 'a'), // kMAKA - Makasar.
  BL_MAKE_TAG('m', 'a', 'n', 'd'), // kMAND - Mandaic, Mandaean.
  BL_MAKE_TAG('m', 'a', 'n', 'i'), // kMANI - Manichaean.
  BL_MAKE_TAG('m', 'a', 'r', 'c'), // kMARC - Marchen.
  BL_MAKE_TAG('m', 'a', 't', 'h'), // kMATH - Mathematical Alphanumeric Symbols.
  BL_MAKE_TAG('m', 'e', 'd', 'f'), // kMEDF - Medefaidrin.
  BL_MAKE_TAG('m', 'e', 'n', 'd'), // kMEND - Mende Kikakui.
  BL_MAKE_TAG('m', 'e', 'r', 'c'), // kMERC - Meroitic Cursive.
  BL_MAKE_TAG('m', 'e', 'r', 'o'), // kMERO - Meroitic Hieroglyphs.
  BL_MAKE_TAG('m', 'l', 'm', '2'), // kMLM2 - Malayalam v2.
  BL_MAKE_TAG('m', 'l', 'y', 'm'), // kMLYM - Malayalam.
  BL_MAKE_TAG('m', 'o', 'd', 'i'), // kMODI - Modi.
  BL_MAKE_TAG('m', 'o', 'n', 'g'), // kMONG - Mongolian.
  BL_MAKE_TAG('m', 'r', 'o', 'o'), // kMROO - Mro.
  BL_MAKE_TAG('m', 't', 'e', 'i'), // kMTEI - Meitei Mayek (Meithei, Meetei)
  BL_MAKE_TAG('m', 'u', 'l', 't'), // kMULT - Multani.
  BL_MAKE_TAG('m', 'u', 's', 'c'), // kMUSC - Musical Symbols.
  BL_MAKE_TAG('m', 'y', 'm', '2'), // kMYM2 - Myanmar v2.
  BL_MAKE_TAG('m', 'y', 'm', 'r'), // kMYMR - Myanmar.
  BL_MAKE_TAG('n', 'a', 'n', 'd'), // kNAND - Nandinagari.
  BL_MAKE_TAG('n', 'a', 'r', 'b'), // kNARB - Old North Arabian.
  BL_MAKE_TAG('n', 'b', 'a', 't'), // kNBAT - Nabataean.
  BL_MAKE_TAG('n', 'e', 'w', 'a'), // kNEWA - Newa.
  BL_MAKE_TAG('n', 'k', 'o', ' '), // kNKO  - N'Ko.
  BL_MAKE_TAG('n', 's', 'h', 'u'), // kNSHU - Nushu.
  BL_MAKE_TAG('o', 'g', 'a', 'm'), // kOGAM - Ogham.
  BL_MAKE_TAG('o', 'l', 'c', 'k'), // kOLCK - Ol Chiki.
  BL_MAKE_TAG('o', 'r', 'k', 'h'), // kORKH - Old Turkic, Orkhon Runic.
  BL_MAKE_TAG('o', 'r', 'y', '2'), // kORY2 - Odia v2.
  BL_MAKE_TAG('o', 'r', 'y', 'a'), // kORYA - Odia.
  BL_MAKE_TAG('o', 's', 'g', 'e'), // kOSGE - Osage.
  BL_MAKE_TAG('o', 's', 'm', 'a'), // kOSMA - Osmanya.
  BL_MAKE_TAG('o', 'u', 'g', 'r'), // kOUGR - Old Uyghur.
  BL_MAKE_TAG('p', 'a', 'l', 'm'), // kPALM - Palmyrene.
  BL_MAKE_TAG('p', 'a', 'u', 'c'), // kPAUC - Pau Cin Hau.
  BL_MAKE_TAG('p', 'e', 'r', 'm'), // kPERM - Old Permic.
  BL_MAKE_TAG('p', 'h', 'a', 'g'), // kPHAG - Phags-pa.
  BL_MAKE_TAG('p', 'h', 'l', 'i'), // kPHLI - Inscriptional Pahlavi.
  BL_MAKE_TAG('p', 'h', 'l', 'p'), // kPHLP - Psalter Pahlavi.
  BL_MAKE_TAG('p', 'h', 'n', 'x'), // kPHNX - Phoenician.
  BL_MAKE_TAG('p', 'l', 'r', 'd'), // kPLRD - Miao.
  BL_MAKE_TAG('p', 'r', 't', 'i'), // kPRTI - Inscriptional Parthian.
  BL_MAKE_TAG('r', 'j', 'n', 'g'), // kRJNG - Rejang.
  BL_MAKE_TAG('r', 'o', 'h', 'g'), // kROHG - Hanifi Rohingya.
  BL_MAKE_TAG('r', 'u', 'n', 'r'), // kRUNR - Runic.
  BL_MAKE_TAG('s', 'a', 'm', 'r'), // kSAMR - Samaritan.
  BL_MAKE_TAG('s', 'a', 'r', 'b'), // kSARB - Old South Arabian.
  BL_MAKE_TAG('s', 'a', 'u', 'r'), // kSAUR - Saurashtra.
  BL_MAKE_TAG('s', 'g', 'n', 'w'), // kSGNW - Sign Writing.
  BL_MAKE_TAG('s', 'h', 'a', 'w'), // kSHAW - Shavian.
  BL_MAKE_TAG('s', 'h', 'r', 'd'), // kSHRD - Sharada.
  BL_MAKE_TAG('s', 'i', 'd', 'd'), // kSIDD - Siddham.
  BL_MAKE_TAG('s', 'i', 'n', 'd'), // kSIND - Khudawadi.
  BL_MAKE_TAG('s', 'i', 'n', 'h'), // kSINH - Sinhala.
  BL_MAKE_TAG('s', 'o', 'g', 'd'), // kSOGD - Sogdian.
  BL_MAKE_TAG('s', 'o', 'g', 'o'), // kSOGO - Old Sogdian.
  BL_MAKE_TAG('s', 'o', 'r', 'a'), // kSORA - Sora Sompeng.
  BL_MAKE_TAG('s', 'o', 'y', 'o'), // kSOYO - Soyombo.
  BL_MAKE_TAG('s', 'u', 'n', 'd'), // kSUND - Sundanese.
  BL_MAKE_TAG('s', 'y', 'l', 'o'), // kSYLO - Syloti Nagri.
  BL_MAKE_TAG('s', 'y', 'r', 'c'), // kSYRC - Syriac.
  BL_MAKE_TAG('t', 'a', 'g', 'b'), // kTAGB - Tagbanwa.
  BL_MAKE_TAG('t', 'a', 'k', 'r'), // kTAKR - Takri.
  BL_MAKE_TAG('t', 'a', 'l', 'e'), // kTALE - Tai Le.
  BL_MAKE_TAG('t', 'a', 'l', 'u'), // kTALU - New Tai Lue.
  BL_MAKE_TAG('t', 'a', 'm', 'l'), // kTAML - Tamil.
  BL_MAKE_TAG('t', 'a', 'n', 'g'), // kTANG - Tangut.
  BL_MAKE_TAG('t', 'a', 'v', 't'), // kTAVT - Tai Viet.
  BL_MAKE_TAG('t', 'e', 'l', '2'), // kTEL2 - Telugu v2.
  BL_MAKE_TAG('t', 'e', 'l', 'u'), // kTELU - Telugu.
  BL_MAKE_TAG('t', 'f', 'n', 'g'), // kTFNG - Tifinagh.
  BL_MAKE_TAG('t', 'g', 'l', 'g'), // kTGLG - Tagalog.
  BL_MAKE_TAG('t', 'h', 'a', 'a'), // kTHAA - Thaana.
  BL_MAKE_TAG('t', 'h', 'a', 'i'), // kTHAI - Thai.
  BL_MAKE_TAG('t', 'i', 'b', 't'), // kTIBT - Tibetan.
  BL_MAKE_TAG('t', 'i', 'r', 'h'), // kTIRH - Tirhuta.
  BL_MAKE_TAG('t', 'm', 'l', '2'), // kTML2 - Tamil v2.
  BL_MAKE_TAG('t', 'n', 's', 'a'), // kTNSA - Tangsa.
  BL_MAKE_TAG('t', 'o', 't', 'o'), // kTOTO - Toto.
  BL_MAKE_TAG('u', 'g', 'a', 'r'), // kUGAR - Ugaritic Cuneiform.
  BL_MAKE_TAG('v', 'a', 'i', ' '), // kVAI  - Vai.
  BL_MAKE_TAG('v', 'i', 't', 'h'), // kVITH - Vithkuqi.
  BL_MAKE_TAG('w', 'a', 'r', 'a'), // kWARA - Warang Citi.
  BL_MAKE_TAG('w', 'c', 'h', 'o'), // kWCHO - Wancho.
  BL_MAKE_TAG('x', 'p', 'e', 'o'), // kXPEO - Old Persian Cuneiform.
  BL_MAKE_TAG('x', 's', 'u', 'x'), // kXSUX - Sumero-Akkadian Cuneiform.
  BL_MAKE_TAG('y', 'e', 'z', 'i'), // kYEZI - Yezidi.
  BL_MAKE_TAG('y', 'i', ' ', ' '), // kYI   - Yi.
  BL_MAKE_TAG('z', 'a', 'n', 'b')  // kZANB - Zanabazar Square.
};

// bl::FontTagData - Language IDs
// ==============================

const BLTag language_id_to_tag_table[] = {
  BL_MAKE_TAG('A', 'B', 'A', ' '), // kABA  - Abaza.
  BL_MAKE_TAG('A', 'B', 'K', ' '), // kABK  - Abkhazian.
  BL_MAKE_TAG('A', 'C', 'H', ' '), // kACH  - Acholi.
  BL_MAKE_TAG('A', 'C', 'R', ' '), // kACR  - Achi.
  BL_MAKE_TAG('A', 'D', 'Y', ' '), // kADY  - Adyghe.
  BL_MAKE_TAG('A', 'F', 'K', ' '), // kAFK  - Afrikaans.
  BL_MAKE_TAG('A', 'F', 'R', ' '), // kAFR  - Afar.
  BL_MAKE_TAG('A', 'G', 'W', ' '), // kAGW  - Agaw.
  BL_MAKE_TAG('A', 'I', 'O', ' '), // kAIO  - Aiton.
  BL_MAKE_TAG('A', 'K', 'A', ' '), // kAKA  - Akan.
  BL_MAKE_TAG('A', 'K', 'B', ' '), // kAKB  - Batak Angkola.
  BL_MAKE_TAG('A', 'L', 'S', ' '), // kALS  - Alsatian.
  BL_MAKE_TAG('A', 'L', 'T', ' '), // kALT  - Altai.
  BL_MAKE_TAG('A', 'M', 'H', ' '), // kAMH  - Amharic.
  BL_MAKE_TAG('A', 'N', 'G', ' '), // kANG  - Anglo-Saxon.
  BL_MAKE_TAG('A', 'P', 'P', 'H'), // kAPPH - Phonetic transcription—Americanist conventions.
  BL_MAKE_TAG('A', 'R', 'A', ' '), // kARA  - Arabic.
  BL_MAKE_TAG('A', 'R', 'G', ' '), // kARG  - Aragonese.
  BL_MAKE_TAG('A', 'R', 'I', ' '), // kARI  - Aari.
  BL_MAKE_TAG('A', 'R', 'K', ' '), // kARK  - Rakhine.
  BL_MAKE_TAG('A', 'S', 'M', ' '), // kASM  - Assamese.
  BL_MAKE_TAG('A', 'S', 'T', ' '), // kAST  - Asturian.
  BL_MAKE_TAG('A', 'T', 'H', ' '), // kATH  - Athapaskan.
  BL_MAKE_TAG('A', 'V', 'N', ' '), // kAVN  - Avatime.
  BL_MAKE_TAG('A', 'V', 'R', ' '), // kAVR  - Avar.
  BL_MAKE_TAG('A', 'W', 'A', ' '), // kAWA  - Awadhi.
  BL_MAKE_TAG('A', 'Y', 'M', ' '), // kAYM  - Aymara.
  BL_MAKE_TAG('A', 'Z', 'B', ' '), // kAZB  - Torki.
  BL_MAKE_TAG('A', 'Z', 'E', ' '), // kAZE  - Azerbaijani.
  BL_MAKE_TAG('B', 'A', 'D', ' '), // kBAD  - Badaga.
  BL_MAKE_TAG('B', 'A', 'D', '0'), // kBAD0 - Banda.
  BL_MAKE_TAG('B', 'A', 'G', ' '), // kBAG  - Baghelkhandi.
  BL_MAKE_TAG('B', 'A', 'L', ' '), // kBAL  - Balkar.
  BL_MAKE_TAG('B', 'A', 'N', ' '), // kBAN  - Balinese.
  BL_MAKE_TAG('B', 'A', 'R', ' '), // kBAR  - Bavarian.
  BL_MAKE_TAG('B', 'A', 'U', ' '), // kBAU  - Baulé.
  BL_MAKE_TAG('B', 'B', 'C', ' '), // kBBC  - Batak Toba.
  BL_MAKE_TAG('B', 'B', 'R', ' '), // kBBR  - Berber.
  BL_MAKE_TAG('B', 'C', 'H', ' '), // kBCH  - Bench.
  BL_MAKE_TAG('B', 'C', 'R', ' '), // kBCR  - Bible Cree.
  BL_MAKE_TAG('B', 'D', 'Y', ' '), // kBDY  - Bandjalang.
  BL_MAKE_TAG('B', 'E', 'L', ' '), // kBEL  - Belarussian.
  BL_MAKE_TAG('B', 'E', 'M', ' '), // kBEM  - Bemba.
  BL_MAKE_TAG('B', 'E', 'N', ' '), // kBEN  - Bengali.
  BL_MAKE_TAG('B', 'G', 'C', ' '), // kBGC  - Haryanvi.
  BL_MAKE_TAG('B', 'G', 'Q', ' '), // kBGQ  - Bagri.
  BL_MAKE_TAG('B', 'G', 'R', ' '), // kBGR  - Bulgarian.
  BL_MAKE_TAG('B', 'H', 'I', ' '), // kBHI  - Bhili.
  BL_MAKE_TAG('B', 'H', 'O', ' '), // kBHO  - Bhojpuri.
  BL_MAKE_TAG('B', 'I', 'K', ' '), // kBIK  - Bikol.
  BL_MAKE_TAG('B', 'I', 'L', ' '), // kBIL  - Bilen.
  BL_MAKE_TAG('B', 'I', 'S', ' '), // kBIS  - Bislama.
  BL_MAKE_TAG('B', 'J', 'J', ' '), // kBJJ  - Kanauji.
  BL_MAKE_TAG('B', 'K', 'F', ' '), // kBKF  - Blackfoot.
  BL_MAKE_TAG('B', 'L', 'I', ' '), // kBLI  - Baluchi.
  BL_MAKE_TAG('B', 'L', 'K', ' '), // kBLK  - Pa’o Karen.
  BL_MAKE_TAG('B', 'L', 'N', ' '), // kBLN  - Balante.
  BL_MAKE_TAG('B', 'L', 'T', ' '), // kBLT  - Balti.
  BL_MAKE_TAG('B', 'M', 'B', ' '), // kBMB  - Bambara (Bamanankan).
  BL_MAKE_TAG('B', 'M', 'L', ' '), // kBML  - Bamileke.
  BL_MAKE_TAG('B', 'O', 'S', ' '), // kBOS  - Bosnian.
  BL_MAKE_TAG('B', 'P', 'Y', ' '), // kBPY  - Bishnupriya Manipuri.
  BL_MAKE_TAG('B', 'R', 'E', ' '), // kBRE  - Breton.
  BL_MAKE_TAG('B', 'R', 'H', ' '), // kBRH  - Brahui.
  BL_MAKE_TAG('B', 'R', 'I', ' '), // kBRI  - Braj Bhasha.
  BL_MAKE_TAG('B', 'R', 'M', ' '), // kBRM  - Burmese.
  BL_MAKE_TAG('B', 'R', 'X', ' '), // kBRX  - Bodo.
  BL_MAKE_TAG('B', 'S', 'H', ' '), // kBSH  - Bashkir.
  BL_MAKE_TAG('B', 'S', 'K', ' '), // kBSK  - Burushaski.
  BL_MAKE_TAG('B', 'T', 'D', ' '), // kBTD  - Batak Dairi (Pakpak).
  BL_MAKE_TAG('B', 'T', 'I', ' '), // kBTI  - Beti.
  BL_MAKE_TAG('B', 'T', 'K', ' '), // kBTK  - Batak.
  BL_MAKE_TAG('B', 'T', 'M', ' '), // kBTM  - Batak Mandailing.
  BL_MAKE_TAG('B', 'T', 'S', ' '), // kBTS  - Batak Simalungun.
  BL_MAKE_TAG('B', 'T', 'X', ' '), // kBTX  - Batak Karo.
  BL_MAKE_TAG('B', 'T', 'Z', ' '), // kBTZ  - Batak Alas-Kluet.
  BL_MAKE_TAG('B', 'U', 'G', ' '), // kBUG  - Bugis.
  BL_MAKE_TAG('B', 'Y', 'V', ' '), // kBYV  - Medumba.
  BL_MAKE_TAG('C', 'A', 'K', ' '), // kCAK  - Kaqchikel.
  BL_MAKE_TAG('C', 'A', 'T', ' '), // kCAT  - Catalan.
  BL_MAKE_TAG('C', 'B', 'K', ' '), // kCBK  - Zamboanga Chavacano.
  BL_MAKE_TAG('C', 'C', 'H', 'N'), // kCCHN - Chinantec.
  BL_MAKE_TAG('C', 'E', 'B', ' '), // kCEB  - Cebuano.
  BL_MAKE_TAG('C', 'G', 'G', ' '), // kCGG  - Chiga.
  BL_MAKE_TAG('C', 'H', 'A', ' '), // kCHA  - Chamorro.
  BL_MAKE_TAG('C', 'H', 'E', ' '), // kCHE  - Chechen.
  BL_MAKE_TAG('C', 'H', 'G', ' '), // kCHG  - Chaha Gurage.
  BL_MAKE_TAG('C', 'H', 'H', ' '), // kCHH  - Chattisgarhi.
  BL_MAKE_TAG('C', 'H', 'I', ' '), // kCHI  - Chichewa (Chewa, Nyanja).
  BL_MAKE_TAG('C', 'H', 'K', ' '), // kCHK  - Chukchi.
  BL_MAKE_TAG('C', 'H', 'K', '0'), // kCHK0 - Chuukese.
  BL_MAKE_TAG('C', 'H', 'O', ' '), // kCHO  - Choctaw.
  BL_MAKE_TAG('C', 'H', 'P', ' '), // kCHP  - Chipewyan.
  BL_MAKE_TAG('C', 'H', 'R', ' '), // kCHR  - Cherokee.
  BL_MAKE_TAG('C', 'H', 'U', ' '), // kCHU  - Chuvash.
  BL_MAKE_TAG('C', 'H', 'Y', ' '), // kCHY  - Cheyenne.
  BL_MAKE_TAG('C', 'J', 'A', ' '), // kCJA  - Western Cham.
  BL_MAKE_TAG('C', 'J', 'M', ' '), // kCJM  - Eastern Cham.
  BL_MAKE_TAG('C', 'M', 'R', ' '), // kCMR  - Comorian.
  BL_MAKE_TAG('C', 'O', 'P', ' '), // kCOP  - Coptic.
  BL_MAKE_TAG('C', 'O', 'R', ' '), // kCOR  - Cornish.
  BL_MAKE_TAG('C', 'O', 'S', ' '), // kCOS  - Corsican.
  BL_MAKE_TAG('C', 'P', 'P', ' '), // kCPP  - Creoles.
  BL_MAKE_TAG('C', 'R', 'E', ' '), // kCRE  - Cree.
  BL_MAKE_TAG('C', 'R', 'R', ' '), // kCRR  - Carrier.
  BL_MAKE_TAG('C', 'R', 'T', ' '), // kCRT  - Crimean Tatar.
  BL_MAKE_TAG('C', 'S', 'B', ' '), // kCSB  - Kashubian.
  BL_MAKE_TAG('C', 'S', 'L', ' '), // kCSL  - Church Slavonic.
  BL_MAKE_TAG('C', 'S', 'Y', ' '), // kCSY  - Czech.
  BL_MAKE_TAG('C', 'T', 'G', ' '), // kCTG  - Chittagonian.
  BL_MAKE_TAG('C', 'T', 'T', ' '), // kCTT  - Wayanad Chetti.
  BL_MAKE_TAG('C', 'U', 'K', ' '), // kCUK  - San Blas Kuna.
  BL_MAKE_TAG('D', 'A', 'G', ' '), // kDAG  - Dagbani.
  BL_MAKE_TAG('D', 'A', 'N', ' '), // kDAN  - Danish.
  BL_MAKE_TAG('D', 'A', 'R', ' '), // kDAR  - Dargwa.
  BL_MAKE_TAG('D', 'A', 'X', ' '), // kDAX  - Dayi.
  BL_MAKE_TAG('D', 'C', 'R', ' '), // kDCR  - Woods Cree.
  BL_MAKE_TAG('D', 'E', 'U', ' '), // kDEU  - German.
  BL_MAKE_TAG('D', 'G', 'O', ' '), // kDGO  - Dogri (individual language).
  BL_MAKE_TAG('D', 'G', 'R', ' '), // kDGR  - Dogri (macrolanguage).
  BL_MAKE_TAG('D', 'H', 'G', ' '), // kDHG  - Dhangu.
  BL_MAKE_TAG('D', 'H', 'V', ' '), // kDHV  - Divehi (Dhivehi, Maldivian).   (deprecated).
  BL_MAKE_TAG('D', 'I', 'Q', ' '), // kDIQ  - Dimli.
  BL_MAKE_TAG('D', 'I', 'V', ' '), // kDIV  - Divehi (Dhivehi, Maldivian).
  BL_MAKE_TAG('D', 'J', 'R', ' '), // kDJR  - Zarma.
  BL_MAKE_TAG('D', 'J', 'R', '0'), // kDJR0 - Djambarrpuyngu.
  BL_MAKE_TAG('D', 'N', 'G', ' '), // kDNG  - Dangme.
  BL_MAKE_TAG('D', 'N', 'J', ' '), // kDNJ  - Dan.
  BL_MAKE_TAG('D', 'N', 'K', ' '), // kDNK  - Dinka.
  BL_MAKE_TAG('D', 'R', 'I', ' '), // kDRI  - Dari.
  BL_MAKE_TAG('D', 'U', 'J', ' '), // kDUJ  - Dhuwal.
  BL_MAKE_TAG('D', 'U', 'N', ' '), // kDUN  - Dungan.
  BL_MAKE_TAG('D', 'Z', 'N', ' '), // kDZN  - Dzongkha.
  BL_MAKE_TAG('E', 'B', 'I', ' '), // kEBI  - Ebira.
  BL_MAKE_TAG('E', 'C', 'R', ' '), // kECR  - Eastern Cree.
  BL_MAKE_TAG('E', 'D', 'O', ' '), // kEDO  - Edo.
  BL_MAKE_TAG('E', 'F', 'I', ' '), // kEFI  - Efik.
  BL_MAKE_TAG('E', 'L', 'L', ' '), // kELL  - Greek.
  BL_MAKE_TAG('E', 'M', 'K', ' '), // kEMK  - Eastern Maninkakan.
  BL_MAKE_TAG('E', 'N', 'G', ' '), // kENG  - English.
  BL_MAKE_TAG('E', 'R', 'Z', ' '), // kERZ  - Erzya.
  BL_MAKE_TAG('E', 'S', 'P', ' '), // kESP  - Spanish.
  BL_MAKE_TAG('E', 'S', 'U', ' '), // kESU  - Central Yupik.
  BL_MAKE_TAG('E', 'T', 'I', ' '), // kETI  - Estonian.
  BL_MAKE_TAG('E', 'U', 'Q', ' '), // kEUQ  - Basque.
  BL_MAKE_TAG('E', 'V', 'K', ' '), // kEVK  - Evenki.
  BL_MAKE_TAG('E', 'V', 'N', ' '), // kEVN  - Even.
  BL_MAKE_TAG('E', 'W', 'E', ' '), // kEWE  - Ewe.
  BL_MAKE_TAG('F', 'A', 'N', ' '), // kFAN  - French Antillean.
  BL_MAKE_TAG('F', 'A', 'N', '0'), // kFAN0 - Fang.
  BL_MAKE_TAG('F', 'A', 'R', ' '), // kFAR  - Persian.
  BL_MAKE_TAG('F', 'A', 'T', ' '), // kFAT  - Fanti.
  BL_MAKE_TAG('F', 'I', 'N', ' '), // kFIN  - Finnish.
  BL_MAKE_TAG('F', 'J', 'I', ' '), // kFJI  - Fijian.
  BL_MAKE_TAG('F', 'L', 'E', ' '), // kFLE  - Dutch (Flemish).
  BL_MAKE_TAG('F', 'M', 'P', ' '), // kFMP  - Fe’fe’.
  BL_MAKE_TAG('F', 'N', 'E', ' '), // kFNE  - Forest Enets.
  BL_MAKE_TAG('F', 'O', 'N', ' '), // kFON  - Fon.
  BL_MAKE_TAG('F', 'O', 'S', ' '), // kFOS  - Faroese.
  BL_MAKE_TAG('F', 'R', 'A', ' '), // kFRA  - French.
  BL_MAKE_TAG('F', 'R', 'C', ' '), // kFRC  - Cajun French.
  BL_MAKE_TAG('F', 'R', 'I', ' '), // kFRI  - Frisian.
  BL_MAKE_TAG('F', 'R', 'L', ' '), // kFRL  - Friulian.
  BL_MAKE_TAG('F', 'R', 'P', ' '), // kFRP  - Arpitan.
  BL_MAKE_TAG('F', 'T', 'A', ' '), // kFTA  - Futa.
  BL_MAKE_TAG('F', 'U', 'L', ' '), // kFUL  - Fulah.
  BL_MAKE_TAG('F', 'U', 'V', ' '), // kFUV  - Nigerian Fulfulde.
  BL_MAKE_TAG('G', 'A', 'D', ' '), // kGAD  - Ga.
  BL_MAKE_TAG('G', 'A', 'E', ' '), // kGAE  - Scottish Gaelic (Gaelic).
  BL_MAKE_TAG('G', 'A', 'G', ' '), // kGAG  - Gagauz.
  BL_MAKE_TAG('G', 'A', 'L', ' '), // kGAL  - Galician.
  BL_MAKE_TAG('G', 'A', 'R', ' '), // kGAR  - Garshuni.
  BL_MAKE_TAG('G', 'A', 'W', ' '), // kGAW  - Garhwali.
  BL_MAKE_TAG('G', 'E', 'Z', ' '), // kGEZ  - Geez.
  BL_MAKE_TAG('G', 'I', 'H', ' '), // kGIH  - Githabul.
  BL_MAKE_TAG('G', 'I', 'L', ' '), // kGIL  - Gilyak.
  BL_MAKE_TAG('G', 'I', 'L', '0'), // kGIL0 - Kiribati (Gilbertese).
  BL_MAKE_TAG('G', 'K', 'P', ' '), // kGKP  - Kpelle (Guinea).
  BL_MAKE_TAG('G', 'L', 'K', ' '), // kGLK  - Gilaki.
  BL_MAKE_TAG('G', 'M', 'Z', ' '), // kGMZ  - Gumuz.
  BL_MAKE_TAG('G', 'N', 'N', ' '), // kGNN  - Gumatj.
  BL_MAKE_TAG('G', 'O', 'G', ' '), // kGOG  - Gogo.
  BL_MAKE_TAG('G', 'O', 'N', ' '), // kGON  - Gondi.
  BL_MAKE_TAG('G', 'R', 'N', ' '), // kGRN  - Greenlandic.
  BL_MAKE_TAG('G', 'R', 'O', ' '), // kGRO  - Garo.
  BL_MAKE_TAG('G', 'U', 'A', ' '), // kGUA  - Guarani.
  BL_MAKE_TAG('G', 'U', 'C', ' '), // kGUC  - Wayuu.
  BL_MAKE_TAG('G', 'U', 'F', ' '), // kGUF  - Gupapuyngu.
  BL_MAKE_TAG('G', 'U', 'J', ' '), // kGUJ  - Gujarati.
  BL_MAKE_TAG('G', 'U', 'Z', ' '), // kGUZ  - Gusii.
  BL_MAKE_TAG('H', 'A', 'I', ' '), // kHAI  - Haitian (Haitian Creole).
  BL_MAKE_TAG('H', 'A', 'I', '0'), // kHAI0 - Haida.
  BL_MAKE_TAG('H', 'A', 'L', ' '), // kHAL  - Halam (Falam Chin).
  BL_MAKE_TAG('H', 'A', 'R', ' '), // kHAR  - Harauti.
  BL_MAKE_TAG('H', 'A', 'U', ' '), // kHAU  - Hausa.
  BL_MAKE_TAG('H', 'A', 'W', ' '), // kHAW  - Hawaiian.
  BL_MAKE_TAG('H', 'A', 'Y', ' '), // kHAY  - Haya.
  BL_MAKE_TAG('H', 'A', 'Z', ' '), // kHAZ  - Hazaragi.
  BL_MAKE_TAG('H', 'B', 'N', ' '), // kHBN  - Hammer-Banna.
  BL_MAKE_TAG('H', 'E', 'I', ' '), // kHEI  - Heiltsuk.
  BL_MAKE_TAG('H', 'E', 'R', ' '), // kHER  - Herero.
  BL_MAKE_TAG('H', 'I', 'L', ' '), // kHIL  - Hiligaynon.
  BL_MAKE_TAG('H', 'I', 'N', ' '), // kHIN  - Hindi.
  BL_MAKE_TAG('H', 'M', 'A', ' '), // kHMA  - High Mari.
  BL_MAKE_TAG('H', 'M', 'D', ' '), // kHMD  - A-Hmao.
  BL_MAKE_TAG('H', 'M', 'N', ' '), // kHMN  - Hmong.
  BL_MAKE_TAG('H', 'M', 'O', ' '), // kHMO  - Hiri Motu.
  BL_MAKE_TAG('H', 'M', 'Z', ' '), // kHMZ  - Hmong Shuat.
  BL_MAKE_TAG('H', 'N', 'D', ' '), // kHND  - Hindko.
  BL_MAKE_TAG('H', 'O', ' ', ' '), // kHO   - Ho.
  BL_MAKE_TAG('H', 'R', 'I', ' '), // kHRI  - Harari.
  BL_MAKE_TAG('H', 'R', 'V', ' '), // kHRV  - Croatian.
  BL_MAKE_TAG('H', 'U', 'N', ' '), // kHUN  - Hungarian.
  BL_MAKE_TAG('H', 'Y', 'E', ' '), // kHYE  - Armenian.
  BL_MAKE_TAG('H', 'Y', 'E', '0'), // kHYE0 - Armenian East.
  BL_MAKE_TAG('I', 'B', 'A', ' '), // kIBA  - Iban.
  BL_MAKE_TAG('I', 'B', 'B', ' '), // kIBB  - Ibibio.
  BL_MAKE_TAG('I', 'B', 'O', ' '), // kIBO  - Igbo.
  BL_MAKE_TAG('I', 'D', 'O', ' '), // kIDO  - Ido.
  BL_MAKE_TAG('I', 'J', 'O', ' '), // kIJO  - Ijo.
  BL_MAKE_TAG('I', 'L', 'E', ' '), // kILE  - Interlingue.
  BL_MAKE_TAG('I', 'L', 'O', ' '), // kILO  - Ilokano.
  BL_MAKE_TAG('I', 'N', 'A', ' '), // kINA  - Interlingua.
  BL_MAKE_TAG('I', 'N', 'D', ' '), // kIND  - Indonesian.
  BL_MAKE_TAG('I', 'N', 'G', ' '), // kING  - Ingush.
  BL_MAKE_TAG('I', 'N', 'U', ' '), // kINU  - Inuktitut.
  BL_MAKE_TAG('I', 'N', 'U', 'K'), // kINUK - Nunavik Inuktitut.
  BL_MAKE_TAG('I', 'P', 'K', ' '), // kIPK  - Inupiat.
  BL_MAKE_TAG('I', 'P', 'P', 'H'), // kIPPH - Phonetic transcription—IPA.
  BL_MAKE_TAG('I', 'R', 'I', ' '), // kIRI  - Irish.
  BL_MAKE_TAG('I', 'R', 'T', ' '), // kIRT  - Irish Traditional.
  BL_MAKE_TAG('I', 'R', 'U', ' '), // kIRU  - Irula.
  BL_MAKE_TAG('I', 'S', 'L', ' '), // kISL  - Icelandic.
  BL_MAKE_TAG('I', 'S', 'M', ' '), // kISM  - Inari Sami.
  BL_MAKE_TAG('I', 'T', 'A', ' '), // kITA  - Italian.
  BL_MAKE_TAG('I', 'W', 'R', ' '), // kIWR  - Hebrew.
  BL_MAKE_TAG('J', 'A', 'M', ' '), // kJAM  - Jamaican Creole.
  BL_MAKE_TAG('J', 'A', 'N', ' '), // kJAN  - Japanese.
  BL_MAKE_TAG('J', 'A', 'V', ' '), // kJAV  - Javanese.
  BL_MAKE_TAG('J', 'B', 'O', ' '), // kJBO  - Lojban.
  BL_MAKE_TAG('J', 'C', 'T', ' '), // kJCT  - Krymchak.
  BL_MAKE_TAG('J', 'I', 'I', ' '), // kJII  - Yiddish.
  BL_MAKE_TAG('J', 'U', 'D', ' '), // kJUD  - Ladino.
  BL_MAKE_TAG('J', 'U', 'L', ' '), // kJUL  - Jula.
  BL_MAKE_TAG('K', 'A', 'B', ' '), // kKAB  - Kabardian.
  BL_MAKE_TAG('K', 'A', 'B', '0'), // kKAB0 - Kabyle.
  BL_MAKE_TAG('K', 'A', 'C', ' '), // kKAC  - Kachchi.
  BL_MAKE_TAG('K', 'A', 'L', ' '), // kKAL  - Kalenjin.
  BL_MAKE_TAG('K', 'A', 'N', ' '), // kKAN  - Kannada.
  BL_MAKE_TAG('K', 'A', 'R', ' '), // kKAR  - Karachay.
  BL_MAKE_TAG('K', 'A', 'T', ' '), // kKAT  - Georgian.
  BL_MAKE_TAG('K', 'A', 'W', ' '), // kKAW  - Kawi (Old Javanese).
  BL_MAKE_TAG('K', 'A', 'Z', ' '), // kKAZ  - Kazakh.
  BL_MAKE_TAG('K', 'D', 'E', ' '), // kKDE  - Makonde.
  BL_MAKE_TAG('K', 'E', 'A', ' '), // kKEA  - Kabuverdianu (Crioulo).
  BL_MAKE_TAG('K', 'E', 'B', ' '), // kKEB  - Kebena.
  BL_MAKE_TAG('K', 'E', 'K', ' '), // kKEK  - Kekchi.
  BL_MAKE_TAG('K', 'G', 'E', ' '), // kKGE  - Khutsuri Georgian.
  BL_MAKE_TAG('K', 'H', 'A', ' '), // kKHA  - Khakass.
  BL_MAKE_TAG('K', 'H', 'K', ' '), // kKHK  - Khanty-Kazim.
  BL_MAKE_TAG('K', 'H', 'M', ' '), // kKHM  - Khmer.
  BL_MAKE_TAG('K', 'H', 'S', ' '), // kKHS  - Khanty-Shurishkar.
  BL_MAKE_TAG('K', 'H', 'T', ' '), // kKHT  - Khamti Shan.
  BL_MAKE_TAG('K', 'H', 'V', ' '), // kKHV  - Khanty-Vakhi.
  BL_MAKE_TAG('K', 'H', 'W', ' '), // kKHW  - Khowar.
  BL_MAKE_TAG('K', 'I', 'K', ' '), // kKIK  - Kikuyu (Gikuyu).
  BL_MAKE_TAG('K', 'I', 'R', ' '), // kKIR  - Kirghiz (Kyrgyz).
  BL_MAKE_TAG('K', 'I', 'S', ' '), // kKIS  - Kisii.
  BL_MAKE_TAG('K', 'I', 'U', ' '), // kKIU  - Kirmanjki.
  BL_MAKE_TAG('K', 'J', 'D', ' '), // kKJD  - Southern Kiwai.
  BL_MAKE_TAG('K', 'J', 'P', ' '), // kKJP  - Eastern Pwo Karen.
  BL_MAKE_TAG('K', 'J', 'Z', ' '), // kKJZ  - Bumthangkha.
  BL_MAKE_TAG('K', 'K', 'N', ' '), // kKKN  - Kokni.
  BL_MAKE_TAG('K', 'L', 'M', ' '), // kKLM  - Kalmyk.
  BL_MAKE_TAG('K', 'M', 'B', ' '), // kKMB  - Kamba.
  BL_MAKE_TAG('K', 'M', 'N', ' '), // kKMN  - Kumaoni.
  BL_MAKE_TAG('K', 'M', 'O', ' '), // kKMO  - Komo.
  BL_MAKE_TAG('K', 'M', 'S', ' '), // kKMS  - Komso.
  BL_MAKE_TAG('K', 'M', 'Z', ' '), // kKMZ  - Khorasani Turkic.
  BL_MAKE_TAG('K', 'N', 'R', ' '), // kKNR  - Kanuri.
  BL_MAKE_TAG('K', 'O', 'D', ' '), // kKOD  - Kodagu.
  BL_MAKE_TAG('K', 'O', 'H', ' '), // kKOH  - Korean Old Hangul.
  BL_MAKE_TAG('K', 'O', 'K', ' '), // kKOK  - Konkani.
  BL_MAKE_TAG('K', 'O', 'M', ' '), // kKOM  - Komi.
  BL_MAKE_TAG('K', 'O', 'N', ' '), // kKON  - Kikongo.
  BL_MAKE_TAG('K', 'O', 'N', '0'), // kKON0 - Kongo.
  BL_MAKE_TAG('K', 'O', 'P', ' '), // kKOP  - Komi-Permyak.
  BL_MAKE_TAG('K', 'O', 'R', ' '), // kKOR  - Korean.
  BL_MAKE_TAG('K', 'O', 'S', ' '), // kKOS  - Kosraean.
  BL_MAKE_TAG('K', 'O', 'Z', ' '), // kKOZ  - Komi-Zyrian.
  BL_MAKE_TAG('K', 'P', 'L', ' '), // kKPL  - Kpelle.
  BL_MAKE_TAG('K', 'R', 'I', ' '), // kKRI  - Krio.
  BL_MAKE_TAG('K', 'R', 'K', ' '), // kKRK  - Karakalpak.
  BL_MAKE_TAG('K', 'R', 'L', ' '), // kKRL  - Karelian.
  BL_MAKE_TAG('K', 'R', 'M', ' '), // kKRM  - Karaim.
  BL_MAKE_TAG('K', 'R', 'N', ' '), // kKRN  - Karen.
  BL_MAKE_TAG('K', 'R', 'T', ' '), // kKRT  - Koorete.
  BL_MAKE_TAG('K', 'S', 'H', ' '), // kKSH  - Kashmiri.
  BL_MAKE_TAG('K', 'S', 'H', '0'), // kKSH0 - Ripuarian.
  BL_MAKE_TAG('K', 'S', 'I', ' '), // kKSI  - Khasi.
  BL_MAKE_TAG('K', 'S', 'M', ' '), // kKSM  - Kildin Sami.
  BL_MAKE_TAG('K', 'S', 'W', ' '), // kKSW  - S’gaw Karen.
  BL_MAKE_TAG('K', 'U', 'A', ' '), // kKUA  - Kuanyama.
  BL_MAKE_TAG('K', 'U', 'I', ' '), // kKUI  - Kui.
  BL_MAKE_TAG('K', 'U', 'L', ' '), // kKUL  - Kulvi.
  BL_MAKE_TAG('K', 'U', 'M', ' '), // kKUM  - Kumyk.
  BL_MAKE_TAG('K', 'U', 'R', ' '), // kKUR  - Kurdish.
  BL_MAKE_TAG('K', 'U', 'U', ' '), // kKUU  - Kurukh.
  BL_MAKE_TAG('K', 'U', 'Y', ' '), // kKUY  - Kuy.
  BL_MAKE_TAG('K', 'W', 'K', ' '), // kKWK  - Kwakʼ.
  BL_MAKE_TAG('K', 'Y', 'K', ' '), // kKYK  - Koryak.
  BL_MAKE_TAG('K', 'Y', 'U', ' '), // kKYU  - Western Kayah.
  BL_MAKE_TAG('L', 'A', 'D', ' '), // kLAD  - Ladin.
  BL_MAKE_TAG('L', 'A', 'H', ' '), // kLAH  - Lahuli.
  BL_MAKE_TAG('L', 'A', 'K', ' '), // kLAK  - Lak.
  BL_MAKE_TAG('L', 'A', 'M', ' '), // kLAM  - Lambani.
  BL_MAKE_TAG('L', 'A', 'O', ' '), // kLAO  - Lao.
  BL_MAKE_TAG('L', 'A', 'T', ' '), // kLAT  - Latin.
  BL_MAKE_TAG('L', 'A', 'Z', ' '), // kLAZ  - Laz.
  BL_MAKE_TAG('L', 'C', 'R', ' '), // kLCR  - L-Cree.
  BL_MAKE_TAG('L', 'D', 'K', ' '), // kLDK  - Ladakhi.
  BL_MAKE_TAG('L', 'E', 'F', ' '), // kLEF  - Lelemi.
  BL_MAKE_TAG('L', 'E', 'Z', ' '), // kLEZ  - Lezgi.
  BL_MAKE_TAG('L', 'I', 'J', ' '), // kLIJ  - Ligurian.
  BL_MAKE_TAG('L', 'I', 'M', ' '), // kLIM  - Limburgish.
  BL_MAKE_TAG('L', 'I', 'N', ' '), // kLIN  - Lingala.
  BL_MAKE_TAG('L', 'I', 'S', ' '), // kLIS  - Lisu.
  BL_MAKE_TAG('L', 'J', 'P', ' '), // kLJP  - Lampung.
  BL_MAKE_TAG('L', 'K', 'I', ' '), // kLKI  - Laki.
  BL_MAKE_TAG('L', 'M', 'A', ' '), // kLMA  - Low Mari.
  BL_MAKE_TAG('L', 'M', 'B', ' '), // kLMB  - Limbu.
  BL_MAKE_TAG('L', 'M', 'O', ' '), // kLMO  - Lombard.
  BL_MAKE_TAG('L', 'M', 'W', ' '), // kLMW  - Lomwe.
  BL_MAKE_TAG('L', 'O', 'M', ' '), // kLOM  - Loma.
  BL_MAKE_TAG('L', 'P', 'O', ' '), // kLPO  - Lipo.
  BL_MAKE_TAG('L', 'R', 'C', ' '), // kLRC  - Luri.
  BL_MAKE_TAG('L', 'S', 'B', ' '), // kLSB  - Lower Sorbian.
  BL_MAKE_TAG('L', 'S', 'M', ' '), // kLSM  - Lule Sami.
  BL_MAKE_TAG('L', 'T', 'H', ' '), // kLTH  - Lithuanian.
  BL_MAKE_TAG('L', 'T', 'Z', ' '), // kLTZ  - Luxembourgish.
  BL_MAKE_TAG('L', 'U', 'A', ' '), // kLUA  - Luba-Lulua.
  BL_MAKE_TAG('L', 'U', 'B', ' '), // kLUB  - Luba-Katanga.
  BL_MAKE_TAG('L', 'U', 'G', ' '), // kLUG  - Ganda.
  BL_MAKE_TAG('L', 'U', 'H', ' '), // kLUH  - Luyia.
  BL_MAKE_TAG('L', 'U', 'O', ' '), // kLUO  - Luo.
  BL_MAKE_TAG('L', 'V', 'I', ' '), // kLVI  - Latvian.
  BL_MAKE_TAG('M', 'A', 'D', ' '), // kMAD  - Madura.
  BL_MAKE_TAG('M', 'A', 'G', ' '), // kMAG  - Magahi.
  BL_MAKE_TAG('M', 'A', 'H', ' '), // kMAH  - Marshallese.
  BL_MAKE_TAG('M', 'A', 'J', ' '), // kMAJ  - Majang.
  BL_MAKE_TAG('M', 'A', 'K', ' '), // kMAK  - Makhuwa.
  BL_MAKE_TAG('M', 'A', 'L', ' '), // kMAL  - Malayalam.
  BL_MAKE_TAG('M', 'A', 'M', ' '), // kMAM  - Mam.
  BL_MAKE_TAG('M', 'A', 'N', ' '), // kMAN  - Mansi.
  BL_MAKE_TAG('M', 'A', 'P', ' '), // kMAP  - Mapudungun.
  BL_MAKE_TAG('M', 'A', 'R', ' '), // kMAR  - Marathi.
  BL_MAKE_TAG('M', 'A', 'W', ' '), // kMAW  - Marwari.
  BL_MAKE_TAG('M', 'B', 'N', ' '), // kMBN  - Mbundu.
  BL_MAKE_TAG('M', 'B', 'O', ' '), // kMBO  - Mbo.
  BL_MAKE_TAG('M', 'C', 'H', ' '), // kMCH  - Manchu.
  BL_MAKE_TAG('M', 'C', 'R', ' '), // kMCR  - Moose Cree.
  BL_MAKE_TAG('M', 'D', 'E', ' '), // kMDE  - Mende.
  BL_MAKE_TAG('M', 'D', 'R', ' '), // kMDR  - Mandar.
  BL_MAKE_TAG('M', 'E', 'N', ' '), // kMEN  - Me’en.
  BL_MAKE_TAG('M', 'E', 'R', ' '), // kMER  - Meru.
  BL_MAKE_TAG('M', 'F', 'A', ' '), // kMFA  - Pattani Malay.
  BL_MAKE_TAG('M', 'F', 'E', ' '), // kMFE  - Morisyen.
  BL_MAKE_TAG('M', 'I', 'N', ' '), // kMIN  - Minangkabau.
  BL_MAKE_TAG('M', 'I', 'Z', ' '), // kMIZ  - Mizo.
  BL_MAKE_TAG('M', 'K', 'D', ' '), // kMKD  - Macedonian.
  BL_MAKE_TAG('M', 'K', 'R', ' '), // kMKR  - Makasar.
  BL_MAKE_TAG('M', 'K', 'W', ' '), // kMKW  - Kituba.
  BL_MAKE_TAG('M', 'L', 'E', ' '), // kMLE  - Male.
  BL_MAKE_TAG('M', 'L', 'G', ' '), // kMLG  - Malagasy.
  BL_MAKE_TAG('M', 'L', 'N', ' '), // kMLN  - Malinke.
  BL_MAKE_TAG('M', 'L', 'R', ' '), // kMLR  - Malayalam Reformed.
  BL_MAKE_TAG('M', 'L', 'Y', ' '), // kMLY  - Malay.
  BL_MAKE_TAG('M', 'N', 'D', ' '), // kMND  - Mandinka.
  BL_MAKE_TAG('M', 'N', 'G', ' '), // kMNG  - Mongolian.
  BL_MAKE_TAG('M', 'N', 'I', ' '), // kMNI  - Manipuri.
  BL_MAKE_TAG('M', 'N', 'K', ' '), // kMNK  - Maninka.
  BL_MAKE_TAG('M', 'N', 'X', ' '), // kMNX  - Manx.
  BL_MAKE_TAG('M', 'O', 'H', ' '), // kMOH  - Mohawk.
  BL_MAKE_TAG('M', 'O', 'K', ' '), // kMOK  - Moksha.
  BL_MAKE_TAG('M', 'O', 'L', ' '), // kMOL  - Moldavian.
  BL_MAKE_TAG('M', 'O', 'N', ' '), // kMON  - Mon.
  BL_MAKE_TAG('M', 'O', 'N', 'T'), // kMONT - Thailand Mon.
  BL_MAKE_TAG('M', 'O', 'R', ' '), // kMOR  - Moroccan.
  BL_MAKE_TAG('M', 'O', 'S', ' '), // kMOS  - Mossi.
  BL_MAKE_TAG('M', 'R', 'I', ' '), // kMRI  - Maori.
  BL_MAKE_TAG('M', 'T', 'H', ' '), // kMTH  - Maithili.
  BL_MAKE_TAG('M', 'T', 'S', ' '), // kMTS  - Maltese.
  BL_MAKE_TAG('M', 'U', 'N', ' '), // kMUN  - Mundari.
  BL_MAKE_TAG('M', 'U', 'S', ' '), // kMUS  - Muscogee.
  BL_MAKE_TAG('M', 'W', 'L', ' '), // kMWL  - Mirandese.
  BL_MAKE_TAG('M', 'W', 'W', ' '), // kMWW  - Hmong Daw.
  BL_MAKE_TAG('M', 'Y', 'N', ' '), // kMYN  - Mayan.
  BL_MAKE_TAG('M', 'Z', 'N', ' '), // kMZN  - Mazanderani.
  BL_MAKE_TAG('N', 'A', 'G', ' '), // kNAG  - Naga-Assamese.
  BL_MAKE_TAG('N', 'A', 'H', ' '), // kNAH  - Nahuatl.
  BL_MAKE_TAG('N', 'A', 'N', ' '), // kNAN  - Nanai.
  BL_MAKE_TAG('N', 'A', 'P', ' '), // kNAP  - Neapolitan.
  BL_MAKE_TAG('N', 'A', 'S', ' '), // kNAS  - Naskapi.
  BL_MAKE_TAG('N', 'A', 'U', ' '), // kNAU  - Nauruan.
  BL_MAKE_TAG('N', 'A', 'V', ' '), // kNAV  - Navajo.
  BL_MAKE_TAG('N', 'C', 'R', ' '), // kNCR  - N-Cree.
  BL_MAKE_TAG('N', 'D', 'B', ' '), // kNDB  - Ndebele.
  BL_MAKE_TAG('N', 'D', 'C', ' '), // kNDC  - Ndau.
  BL_MAKE_TAG('N', 'D', 'G', ' '), // kNDG  - Ndonga.
  BL_MAKE_TAG('N', 'D', 'S', ' '), // kNDS  - Low Saxon.
  BL_MAKE_TAG('N', 'E', 'P', ' '), // kNEP  - Nepali.
  BL_MAKE_TAG('N', 'E', 'W', ' '), // kNEW  - Newari.
  BL_MAKE_TAG('N', 'G', 'A', ' '), // kNGA  - Ngbaka.
  BL_MAKE_TAG('N', 'G', 'R', ' '), // kNGR  - Nagari.
  BL_MAKE_TAG('N', 'H', 'C', ' '), // kNHC  - Norway House Cree.
  BL_MAKE_TAG('N', 'I', 'S', ' '), // kNIS  - Nisi.
  BL_MAKE_TAG('N', 'I', 'U', ' '), // kNIU  - Niuean.
  BL_MAKE_TAG('N', 'K', 'L', ' '), // kNKL  - Nyankole.
  BL_MAKE_TAG('N', 'K', 'O', ' '), // kNKO  - N’Ko.
  BL_MAKE_TAG('N', 'L', 'D', ' '), // kNLD  - Dutch.
  BL_MAKE_TAG('N', 'O', 'E', ' '), // kNOE  - Nimadi.
  BL_MAKE_TAG('N', 'O', 'G', ' '), // kNOG  - Nogai.
  BL_MAKE_TAG('N', 'O', 'R', ' '), // kNOR  - Norwegian.
  BL_MAKE_TAG('N', 'O', 'V', ' '), // kNOV  - Novial.
  BL_MAKE_TAG('N', 'S', 'M', ' '), // kNSM  - Northern Sami.
  BL_MAKE_TAG('N', 'S', 'O', ' '), // kNSO  - Northern Sotho.
  BL_MAKE_TAG('N', 'T', 'A', ' '), // kNTA  - Northern Tai.
  BL_MAKE_TAG('N', 'T', 'O', ' '), // kNTO  - Esperanto.
  BL_MAKE_TAG('N', 'Y', 'M', ' '), // kNYM  - Nyamwezi.
  BL_MAKE_TAG('N', 'Y', 'N', ' '), // kNYN  - Norwegian Nynorsk (Nynorsk, Norwegian).
  BL_MAKE_TAG('N', 'Z', 'A', ' '), // kNZA  - Mbembe Tigon.
  BL_MAKE_TAG('O', 'C', 'I', ' '), // kOCI  - Occitan.
  BL_MAKE_TAG('O', 'C', 'R', ' '), // kOCR  - Oji-Cree.
  BL_MAKE_TAG('O', 'J', 'B', ' '), // kOJB  - Ojibway.
  BL_MAKE_TAG('O', 'R', 'I', ' '), // kORI  - Odia (formerly Oriya).
  BL_MAKE_TAG('O', 'R', 'O', ' '), // kORO  - Oromo.
  BL_MAKE_TAG('O', 'S', 'S', ' '), // kOSS  - Ossetian.
  BL_MAKE_TAG('P', 'A', 'A', ' '), // kPAA  - Palestinian Aramaic.
  BL_MAKE_TAG('P', 'A', 'G', ' '), // kPAG  - Pangasinan.
  BL_MAKE_TAG('P', 'A', 'L', ' '), // kPAL  - Pali.
  BL_MAKE_TAG('P', 'A', 'M', ' '), // kPAM  - Pampangan.
  BL_MAKE_TAG('P', 'A', 'N', ' '), // kPAN  - Punjabi.
  BL_MAKE_TAG('P', 'A', 'P', ' '), // kPAP  - Palpa.
  BL_MAKE_TAG('P', 'A', 'P', '0'), // kPAP0 - Papiamentu.
  BL_MAKE_TAG('P', 'A', 'S', ' '), // kPAS  - Pashto.
  BL_MAKE_TAG('P', 'A', 'U', ' '), // kPAU  - Palauan.
  BL_MAKE_TAG('P', 'C', 'C', ' '), // kPCC  - Bouyei.
  BL_MAKE_TAG('P', 'C', 'D', ' '), // kPCD  - Picard.
  BL_MAKE_TAG('P', 'D', 'C', ' '), // kPDC  - Pennsylvania German.
  BL_MAKE_TAG('P', 'G', 'R', ' '), // kPGR  - Polytonic Greek.
  BL_MAKE_TAG('P', 'H', 'K', ' '), // kPHK  - Phake.
  BL_MAKE_TAG('P', 'I', 'H', ' '), // kPIH  - Norfolk.
  BL_MAKE_TAG('P', 'I', 'L', ' '), // kPIL  - Filipino.
  BL_MAKE_TAG('P', 'L', 'G', ' '), // kPLG  - Palaung.
  BL_MAKE_TAG('P', 'L', 'K', ' '), // kPLK  - Polish.
  BL_MAKE_TAG('P', 'M', 'S', ' '), // kPMS  - Piemontese.
  BL_MAKE_TAG('P', 'N', 'B', ' '), // kPNB  - Western Panjabi.
  BL_MAKE_TAG('P', 'O', 'H', ' '), // kPOH  - Pocomchi.
  BL_MAKE_TAG('P', 'O', 'N', ' '), // kPON  - Pohnpeian.
  BL_MAKE_TAG('P', 'R', 'O', ' '), // kPRO  - Provençal / Old Provençal.
  BL_MAKE_TAG('P', 'T', 'G', ' '), // kPTG  - Portuguese.
  BL_MAKE_TAG('P', 'W', 'O', ' '), // kPWO  - Western Pwo Karen.
  BL_MAKE_TAG('Q', 'I', 'N', ' '), // kQIN  - Chin.
  BL_MAKE_TAG('Q', 'U', 'C', ' '), // kQUC  - K’iche’.
  BL_MAKE_TAG('Q', 'U', 'H', ' '), // kQUH  - Quechua (Bolivia).
  BL_MAKE_TAG('Q', 'U', 'Z', ' '), // kQUZ  - Quechua.
  BL_MAKE_TAG('Q', 'V', 'I', ' '), // kQVI  - Quechua (Ecuador).
  BL_MAKE_TAG('Q', 'W', 'H', ' '), // kQWH  - Quechua (Peru).
  BL_MAKE_TAG('R', 'A', 'J', ' '), // kRAJ  - Rajasthani.
  BL_MAKE_TAG('R', 'A', 'R', ' '), // kRAR  - Rarotongan.
  BL_MAKE_TAG('R', 'B', 'U', ' '), // kRBU  - Russian Buriat.
  BL_MAKE_TAG('R', 'C', 'R', ' '), // kRCR  - R-Cree.
  BL_MAKE_TAG('R', 'E', 'J', ' '), // kREJ  - Rejang.
  BL_MAKE_TAG('R', 'H', 'G', ' '), // kRHG  - Rohingya.
  BL_MAKE_TAG('R', 'I', 'A', ' '), // kRIA  - Riang.
  BL_MAKE_TAG('R', 'I', 'F', ' '), // kRIF  - Tarifit.
  BL_MAKE_TAG('R', 'I', 'T', ' '), // kRIT  - Ritarungo.
  BL_MAKE_TAG('R', 'K', 'W', ' '), // kRKW  - Arakwal.
  BL_MAKE_TAG('R', 'M', 'S', ' '), // kRMS  - Romansh.
  BL_MAKE_TAG('R', 'M', 'Y', ' '), // kRMY  - Vlax Romani.
  BL_MAKE_TAG('R', 'O', 'M', ' '), // kROM  - Romanian.
  BL_MAKE_TAG('R', 'O', 'Y', ' '), // kROY  - Romany.
  BL_MAKE_TAG('R', 'S', 'Y', ' '), // kRSY  - Rusyn.
  BL_MAKE_TAG('R', 'T', 'M', ' '), // kRTM  - Rotuman.
  BL_MAKE_TAG('R', 'U', 'A', ' '), // kRUA  - Kinyarwanda.
  BL_MAKE_TAG('R', 'U', 'N', ' '), // kRUN  - Rundi.
  BL_MAKE_TAG('R', 'U', 'P', ' '), // kRUP  - Aromanian.
  BL_MAKE_TAG('R', 'U', 'S', ' '), // kRUS  - Russian.
  BL_MAKE_TAG('S', 'A', 'D', ' '), // kSAD  - Sadri.
  BL_MAKE_TAG('S', 'A', 'N', ' '), // kSAN  - Sanskrit.
  BL_MAKE_TAG('S', 'A', 'S', ' '), // kSAS  - Sasak.
  BL_MAKE_TAG('S', 'A', 'T', ' '), // kSAT  - Santali.
  BL_MAKE_TAG('S', 'A', 'Y', ' '), // kSAY  - Sayisi.
  BL_MAKE_TAG('S', 'C', 'N', ' '), // kSCN  - Sicilian.
  BL_MAKE_TAG('S', 'C', 'O', ' '), // kSCO  - Scots.
  BL_MAKE_TAG('S', 'C', 'S', ' '), // kSCS  - North Slavey.
  BL_MAKE_TAG('S', 'E', 'K', ' '), // kSEK  - Sekota.
  BL_MAKE_TAG('S', 'E', 'L', ' '), // kSEL  - Selkup.
  BL_MAKE_TAG('S', 'F', 'M', ' '), // kSFM  - Small Flowery Miao.
  BL_MAKE_TAG('S', 'G', 'A', ' '), // kSGA  - Old Irish.
  BL_MAKE_TAG('S', 'G', 'O', ' '), // kSGO  - Sango.
  BL_MAKE_TAG('S', 'G', 'S', ' '), // kSGS  - Samogitian.
  BL_MAKE_TAG('S', 'H', 'I', ' '), // kSHI  - Tachelhit.
  BL_MAKE_TAG('S', 'H', 'N', ' '), // kSHN  - Shan.
  BL_MAKE_TAG('S', 'I', 'B', ' '), // kSIB  - Sibe.
  BL_MAKE_TAG('S', 'I', 'D', ' '), // kSID  - Sidamo.
  BL_MAKE_TAG('S', 'I', 'G', ' '), // kSIG  - Silte Gurage.
  BL_MAKE_TAG('S', 'K', 'S', ' '), // kSKS  - Skolt Sami.
  BL_MAKE_TAG('S', 'K', 'Y', ' '), // kSKY  - Slovak.
  BL_MAKE_TAG('S', 'L', 'A', ' '), // kSLA  - Slavey.
  BL_MAKE_TAG('S', 'L', 'V', ' '), // kSLV  - Slovenian.
  BL_MAKE_TAG('S', 'M', 'L', ' '), // kSML  - Somali.
  BL_MAKE_TAG('S', 'M', 'O', ' '), // kSMO  - Samoan.
  BL_MAKE_TAG('S', 'N', 'A', ' '), // kSNA  - Sena.
  BL_MAKE_TAG('S', 'N', 'A', '0'), // kSNA0 - Shona.
  BL_MAKE_TAG('S', 'N', 'D', ' '), // kSND  - Sindhi.
  BL_MAKE_TAG('S', 'N', 'H', ' '), // kSNH  - Sinhala (Sinhalese).
  BL_MAKE_TAG('S', 'N', 'K', ' '), // kSNK  - Soninke.
  BL_MAKE_TAG('S', 'O', 'G', ' '), // kSOG  - Sodo Gurage.
  BL_MAKE_TAG('S', 'O', 'P', ' '), // kSOP  - Songe.
  BL_MAKE_TAG('S', 'O', 'T', ' '), // kSOT  - Southern Sotho.
  BL_MAKE_TAG('S', 'Q', 'I', ' '), // kSQI  - Albanian.
  BL_MAKE_TAG('S', 'R', 'B', ' '), // kSRB  - Serbian.
  BL_MAKE_TAG('S', 'R', 'D', ' '), // kSRD  - Sardinian.
  BL_MAKE_TAG('S', 'R', 'K', ' '), // kSRK  - Saraiki.
  BL_MAKE_TAG('S', 'R', 'R', ' '), // kSRR  - Serer.
  BL_MAKE_TAG('S', 'S', 'L', ' '), // kSSL  - South Slavey.
  BL_MAKE_TAG('S', 'S', 'M', ' '), // kSSM  - Southern Sami.
  BL_MAKE_TAG('S', 'T', 'Q', ' '), // kSTQ  - Saterland Frisian.
  BL_MAKE_TAG('S', 'U', 'K', ' '), // kSUK  - Sukuma.
  BL_MAKE_TAG('S', 'U', 'N', ' '), // kSUN  - Sundanese.
  BL_MAKE_TAG('S', 'U', 'R', ' '), // kSUR  - Suri.
  BL_MAKE_TAG('S', 'V', 'A', ' '), // kSVA  - Svan.
  BL_MAKE_TAG('S', 'V', 'E', ' '), // kSVE  - Swedish.
  BL_MAKE_TAG('S', 'W', 'A', ' '), // kSWA  - Swadaya Aramaic.
  BL_MAKE_TAG('S', 'W', 'K', ' '), // kSWK  - Swahili.
  BL_MAKE_TAG('S', 'W', 'Z', ' '), // kSWZ  - Swati.
  BL_MAKE_TAG('S', 'X', 'T', ' '), // kSXT  - Sutu.
  BL_MAKE_TAG('S', 'X', 'U', ' '), // kSXU  - Upper Saxon.
  BL_MAKE_TAG('S', 'Y', 'L', ' '), // kSYL  - Sylheti.
  BL_MAKE_TAG('S', 'Y', 'R', ' '), // kSYR  - Syriac.
  BL_MAKE_TAG('S', 'Y', 'R', 'E'), // kSYRE - Syriac, Estrangela script-variant.
  BL_MAKE_TAG('S', 'Y', 'R', 'J'), // kSYRJ - Syriac, Western script-variant.
  BL_MAKE_TAG('S', 'Y', 'R', 'N'), // kSYRN - Syriac, Eastern script-variant.
  BL_MAKE_TAG('S', 'Z', 'L', ' '), // kSZL  - Silesian.
  BL_MAKE_TAG('T', 'A', 'B', ' '), // kTAB  - Tabasaran.
  BL_MAKE_TAG('T', 'A', 'J', ' '), // kTAJ  - Tajiki.
  BL_MAKE_TAG('T', 'A', 'M', ' '), // kTAM  - Tamil.
  BL_MAKE_TAG('T', 'A', 'T', ' '), // kTAT  - Tatar.
  BL_MAKE_TAG('T', 'C', 'R', ' '), // kTCR  - TH-Cree.
  BL_MAKE_TAG('T', 'D', 'D', ' '), // kTDD  - Dehong Dai.
  BL_MAKE_TAG('T', 'E', 'L', ' '), // kTEL  - Telugu.
  BL_MAKE_TAG('T', 'E', 'T', ' '), // kTET  - Tetum.
  BL_MAKE_TAG('T', 'G', 'L', ' '), // kTGL  - Tagalog.
  BL_MAKE_TAG('T', 'G', 'N', ' '), // kTGN  - Tongan.
  BL_MAKE_TAG('T', 'G', 'R', ' '), // kTGR  - Tigre.
  BL_MAKE_TAG('T', 'G', 'Y', ' '), // kTGY  - Tigrinya.
  BL_MAKE_TAG('T', 'H', 'A', ' '), // kTHA  - Thai.
  BL_MAKE_TAG('T', 'H', 'T', ' '), // kTHT  - Tahitian.
  BL_MAKE_TAG('T', 'I', 'B', ' '), // kTIB  - Tibetan.
  BL_MAKE_TAG('T', 'I', 'V', ' '), // kTIV  - Tiv.
  BL_MAKE_TAG('T', 'J', 'L', ' '), // kTJL  - Tai Laing.
  BL_MAKE_TAG('T', 'K', 'M', ' '), // kTKM  - Turkmen.
  BL_MAKE_TAG('T', 'L', 'I', ' '), // kTLI  - Tlingit.
  BL_MAKE_TAG('T', 'M', 'H', ' '), // kTMH  - Tamashek.
  BL_MAKE_TAG('T', 'M', 'N', ' '), // kTMN  - Temne.
  BL_MAKE_TAG('T', 'N', 'A', ' '), // kTNA  - Tswana.
  BL_MAKE_TAG('T', 'N', 'E', ' '), // kTNE  - Tundra Enets.
  BL_MAKE_TAG('T', 'N', 'G', ' '), // kTNG  - Tonga.
  BL_MAKE_TAG('T', 'O', 'D', ' '), // kTOD  - Todo.
  BL_MAKE_TAG('T' ,'O', 'D', '0'), // kTOD0 - Toma.
  BL_MAKE_TAG('T', 'P', 'I', ' '), // kTPI  - Tok Pisin.
  BL_MAKE_TAG('T', 'R', 'K', ' '), // kTRK  - Turkish.
  BL_MAKE_TAG('T', 'S', 'G', ' '), // kTSG  - Tsonga.
  BL_MAKE_TAG('T', 'S', 'J', ' '), // kTSJ  - Tshangla.
  BL_MAKE_TAG('T', 'U', 'A', ' '), // kTUA  - Turoyo Aramaic.
  BL_MAKE_TAG('T', 'U', 'L', ' '), // kTUL  - Tumbuka.
  BL_MAKE_TAG('T', 'U', 'M', ' '), // kTUM  - Tulu.
  BL_MAKE_TAG('T', 'U', 'V', ' '), // kTUV  - Tuvin.
  BL_MAKE_TAG('T', 'V', 'L', ' '), // kTVL  - Tuvalu.
  BL_MAKE_TAG('T', 'W', 'I', ' '), // kTWI  - Twi.
  BL_MAKE_TAG('T', 'Y', 'Z', ' '), // kTYZ  - Tày.
  BL_MAKE_TAG('T', 'Z', 'M', ' '), // kTZM  - Tamazight.
  BL_MAKE_TAG('T', 'Z', 'O', ' '), // kTZO  - Tzotzil.
  BL_MAKE_TAG('U', 'D', 'M', ' '), // kUDM  - Udmurt.
  BL_MAKE_TAG('U', 'K', 'R', ' '), // kUKR  - Ukrainian.
  BL_MAKE_TAG('U', 'M', 'B', ' '), // kUMB  - Umbundu.
  BL_MAKE_TAG('U', 'R', 'D', ' '), // kURD  - Urdu.
  BL_MAKE_TAG('U', 'S', 'B', ' '), // kUSB  - Upper Sorbian.
  BL_MAKE_TAG('U', 'Y', 'G', ' '), // kUYG  - Uyghur.
  BL_MAKE_TAG('U', 'Z', 'B', ' '), // kUZB  - Uzbek.
  BL_MAKE_TAG('V', 'E', 'C', ' '), // kVEC  - Venetian.
  BL_MAKE_TAG('V', 'E', 'N', ' '), // kVEN  - Venda.
  BL_MAKE_TAG('V', 'I', 'T', ' '), // kVIT  - Vietnamese.
  BL_MAKE_TAG('V', 'O', 'L', ' '), // kVOL  - Volapük.
  BL_MAKE_TAG('V', 'R', 'O', ' '), // kVRO  - Võro.
  BL_MAKE_TAG('W', 'A', ' ', ' '), // kWA   - Wa.
  BL_MAKE_TAG('W', 'A', 'G', ' '), // kWAG  - Wagdi.
  BL_MAKE_TAG('W', 'A', 'R', ' '), // kWAR  - Waray-Waray.
  BL_MAKE_TAG('W', 'C', 'I', ' '), // kWCI  - Waci Gbe.
  BL_MAKE_TAG('W', 'C', 'R', ' '), // kWCR  - West-Cree.
  BL_MAKE_TAG('W', 'E', 'L', ' '), // kWEL  - Welsh.
  BL_MAKE_TAG('W', 'L', 'F', ' '), // kWLF  - Wolof.
  BL_MAKE_TAG('W', 'L', 'N', ' '), // kWLN  - Walloon.
  BL_MAKE_TAG('W', 'T', 'M', ' '), // kWTM  - Mewati.
  BL_MAKE_TAG('X', 'B', 'D', ' '), // kXBD  - Lü.
  BL_MAKE_TAG('X', 'H', 'S', ' '), // kXHS  - Xhosa.
  BL_MAKE_TAG('X', 'J', 'B', ' '), // kXJB  - Minjangbal.
  BL_MAKE_TAG('X', 'K', 'F', ' '), // kXKF  - Khengkha.
  BL_MAKE_TAG('X', 'O', 'G', ' '), // kXOG  - Soga.
  BL_MAKE_TAG('X', 'P', 'E', ' '), // kXPE  - Kpelle (Liberia).
  BL_MAKE_TAG('X', 'U', 'B', ' '), // kXUB  - Bette Kuruma.
  BL_MAKE_TAG('X', 'U', 'J', ' '), // kXUJ  - Jennu Kuruma.
  BL_MAKE_TAG('Y', 'A', 'K', ' '), // kYAK  - Sakha.
  BL_MAKE_TAG('Y', 'A', 'O', ' '), // kYAO  - Yao.
  BL_MAKE_TAG('Y', 'A', 'P', ' '), // kYAP  - Yapese.
  BL_MAKE_TAG('Y', 'B', 'A', ' '), // kYBA  - Yoruba.
  BL_MAKE_TAG('Y', 'C', 'R', ' '), // kYCR  - Y-Cree.
  BL_MAKE_TAG('Y', 'G', 'P', ' '), // kYGP  - Gepo.
  BL_MAKE_TAG('Y', 'I', 'C', ' '), // kYIC  - Yi Classic.
  BL_MAKE_TAG('Y', 'I', 'M', ' '), // kYIM  - Yi Modern.
  BL_MAKE_TAG('Y', 'N', 'A', ' '), // kYNA  - Aluo.
  BL_MAKE_TAG('Y', 'W', 'Q', ' '), // kYWQ  - Wuding-Luquan Yi.
  BL_MAKE_TAG('Z', 'E', 'A', ' '), // kZEA  - Zealandic.
  BL_MAKE_TAG('Z', 'G', 'H', ' '), // kZGH  - Standard Moroccan Tamazight.
  BL_MAKE_TAG('Z', 'H', 'A', ' '), // kZHA  - Zhuang.
  BL_MAKE_TAG('Z', 'H', 'H', ' '), // kZHH  - Chinese, Traditional, Hong Kong SAR.
  BL_MAKE_TAG('Z', 'H', 'P', ' '), // kZHP  - Chinese, Phonetic.
  BL_MAKE_TAG('Z', 'H', 'S', ' '), // kZHS  - Chinese, Simplified.
  BL_MAKE_TAG('Z', 'H', 'T', ' '), // kZHT  - Chinese, Traditional.
  BL_MAKE_TAG('Z', 'H', 'T', 'M'), // kZHTM - Chinese, Traditional, Macao SAR.
  BL_MAKE_TAG('Z', 'N', 'D', ' '), // kZND  - Zande.
  BL_MAKE_TAG('Z', 'U', 'L', ' '), // kZUL  - Zulu.
  BL_MAKE_TAG('Z', 'Z', 'A', ' ')  // kZZA  - Zazaki.
};

// bl::FontTagData - Feature IDs
// =============================

const BLTag feature_id_to_tag_table[] = {
  BL_MAKE_TAG('a', 'a', 'l', 't'), // kAALT - Access All Alternates.
  BL_MAKE_TAG('a', 'b', 'v', 'f'), // kABVF - Above-base Forms.
  BL_MAKE_TAG('a', 'b', 'v', 'm'), // kABVM - Above-base Mark Positioning.
  BL_MAKE_TAG('a', 'b', 'v', 's'), // kABVS - Above-base Substitutions.
  BL_MAKE_TAG('a', 'f', 'r', 'c'), // kAFRC - Alternative Fractions.
  BL_MAKE_TAG('a', 'k', 'h', 'n'), // kAKHN - Akhand.
  BL_MAKE_TAG('b', 'l', 'w', 'f'), // kBLWF - Below-base Forms.
  BL_MAKE_TAG('b', 'l', 'w', 'm'), // kBLWM - Below-base Mark Positioning.
  BL_MAKE_TAG('b', 'l', 'w', 's'), // kBLWS - Below-base Substitutions.
  BL_MAKE_TAG('c', '2', 'p', 'c'), // kC2PC - Petite Capitals From Capitals.
  BL_MAKE_TAG('c', '2', 's', 'c'), // kC2SC - Small Capitals From Capitals.
  BL_MAKE_TAG('c', 'a', 'l', 't'), // kCALT - Contextual Alternates.
  BL_MAKE_TAG('c', 'a', 's', 'e'), // kCASE - Case-Sensitive Forms.
  BL_MAKE_TAG('c', 'c', 'm', 'p'), // kCCMP - Glyph Composition / Decomposition.
  BL_MAKE_TAG('c', 'f', 'a', 'r'), // kCFAR - Conjunct Form After Ro.
  BL_MAKE_TAG('c', 'h', 'w', 's'), // kCHWS - Contextual Half-width Spacing.
  BL_MAKE_TAG('c', 'j', 'c', 't'), // kCJCT - Conjunct Forms.
  BL_MAKE_TAG('c', 'l', 'i', 'g'), // kCLIG - Contextual Ligatures.
  BL_MAKE_TAG('c', 'p', 'c', 't'), // kCPCT - Centered CJK Punctuation.
  BL_MAKE_TAG('c', 'p', 's', 'p'), // kCPSP - Capital Spacing.
  BL_MAKE_TAG('c', 's', 'w', 'h'), // kCSWH - Contextual Swash.
  BL_MAKE_TAG('c', 'u', 'r', 's'), // kCURS - Cursive Positioning.
  BL_MAKE_TAG('c', 'v', '0', '1'), // kCV01 - Character Variant 1.
  BL_MAKE_TAG('c', 'v', '0', '2'), // kCV02 - Character Variant 2.
  BL_MAKE_TAG('c', 'v', '0', '3'), // kCV03 - Character Variant 3.
  BL_MAKE_TAG('c', 'v', '0', '4'), // kCV04 - Character Variant 4.
  BL_MAKE_TAG('c', 'v', '0', '5'), // kCV05 - Character Variant 5.
  BL_MAKE_TAG('c', 'v', '0', '6'), // kCV06 - Character Variant 6.
  BL_MAKE_TAG('c', 'v', '0', '7'), // kCV07 - Character Variant 7.
  BL_MAKE_TAG('c', 'v', '0', '8'), // kCV08 - Character Variant 8.
  BL_MAKE_TAG('c', 'v', '0', '9'), // kCV09 - Character Variant 9.
  BL_MAKE_TAG('c', 'v', '1', '0'), // kCV10 - Character Variant 10.
  BL_MAKE_TAG('c', 'v', '1', '1'), // kCV11 - Character Variant 11.
  BL_MAKE_TAG('c', 'v', '1', '2'), // kCV12 - Character Variant 12.
  BL_MAKE_TAG('c', 'v', '1', '3'), // kCV13 - Character Variant 13.
  BL_MAKE_TAG('c', 'v', '1', '4'), // kCV14 - Character Variant 14.
  BL_MAKE_TAG('c', 'v', '1', '5'), // kCV15 - Character Variant 15.
  BL_MAKE_TAG('c', 'v', '1', '6'), // kCV16 - Character Variant 16.
  BL_MAKE_TAG('c', 'v', '1', '7'), // kCV17 - Character Variant 17.
  BL_MAKE_TAG('c', 'v', '1', '8'), // kCV18 - Character Variant 18.
  BL_MAKE_TAG('c', 'v', '1', '9'), // kCV19 - Character Variant 19.
  BL_MAKE_TAG('c', 'v', '2', '0'), // kCV20 - Character Variant 20.
  BL_MAKE_TAG('c', 'v', '2', '1'), // kCV21 - Character Variant 21.
  BL_MAKE_TAG('c', 'v', '2', '2'), // kCV22 - Character Variant 22.
  BL_MAKE_TAG('c', 'v', '2', '3'), // kCV23 - Character Variant 23.
  BL_MAKE_TAG('c', 'v', '2', '4'), // kCV24 - Character Variant 24.
  BL_MAKE_TAG('c', 'v', '2', '5'), // kCV25 - Character Variant 25.
  BL_MAKE_TAG('c', 'v', '2', '6'), // kCV26 - Character Variant 26.
  BL_MAKE_TAG('c', 'v', '2', '7'), // kCV27 - Character Variant 27.
  BL_MAKE_TAG('c', 'v', '2', '8'), // kCV28 - Character Variant 28.
  BL_MAKE_TAG('c', 'v', '2', '9'), // kCV29 - Character Variant 29.
  BL_MAKE_TAG('c', 'v', '3', '0'), // kCV30 - Character Variant 30.
  BL_MAKE_TAG('c', 'v', '3', '1'), // kCV31 - Character Variant 31.
  BL_MAKE_TAG('c', 'v', '3', '2'), // kCV32 - Character Variant 32.
  BL_MAKE_TAG('c', 'v', '3', '3'), // kCV33 - Character Variant 33.
  BL_MAKE_TAG('c', 'v', '3', '4'), // kCV34 - Character Variant 34.
  BL_MAKE_TAG('c', 'v', '3', '5'), // kCV35 - Character Variant 35.
  BL_MAKE_TAG('c', 'v', '3', '6'), // kCV36 - Character Variant 36.
  BL_MAKE_TAG('c', 'v', '3', '7'), // kCV37 - Character Variant 37.
  BL_MAKE_TAG('c', 'v', '3', '8'), // kCV38 - Character Variant 38.
  BL_MAKE_TAG('c', 'v', '3', '9'), // kCV39 - Character Variant 39.
  BL_MAKE_TAG('c', 'v', '4', '0'), // kCV40 - Character Variant 40.
  BL_MAKE_TAG('c', 'v', '4', '1'), // kCV41 - Character Variant 41.
  BL_MAKE_TAG('c', 'v', '4', '2'), // kCV42 - Character Variant 42.
  BL_MAKE_TAG('c', 'v', '4', '3'), // kCV43 - Character Variant 43.
  BL_MAKE_TAG('c', 'v', '4', '4'), // kCV44 - Character Variant 44.
  BL_MAKE_TAG('c', 'v', '4', '5'), // kCV45 - Character Variant 45.
  BL_MAKE_TAG('c', 'v', '4', '6'), // kCV46 - Character Variant 46.
  BL_MAKE_TAG('c', 'v', '4', '7'), // kCV47 - Character Variant 47.
  BL_MAKE_TAG('c', 'v', '4', '8'), // kCV48 - Character Variant 48.
  BL_MAKE_TAG('c', 'v', '4', '9'), // kCV49 - Character Variant 49.
  BL_MAKE_TAG('c', 'v', '5', '0'), // kCV50 - Character Variant 50.
  BL_MAKE_TAG('c', 'v', '5', '1'), // kCV51 - Character Variant 51.
  BL_MAKE_TAG('c', 'v', '5', '2'), // kCV52 - Character Variant 52.
  BL_MAKE_TAG('c', 'v', '5', '3'), // kCV53 - Character Variant 53.
  BL_MAKE_TAG('c', 'v', '5', '4'), // kCV54 - Character Variant 54.
  BL_MAKE_TAG('c', 'v', '5', '5'), // kCV55 - Character Variant 55.
  BL_MAKE_TAG('c', 'v', '5', '6'), // kCV56 - Character Variant 56.
  BL_MAKE_TAG('c', 'v', '5', '7'), // kCV57 - Character Variant 57.
  BL_MAKE_TAG('c', 'v', '5', '8'), // kCV58 - Character Variant 58.
  BL_MAKE_TAG('c', 'v', '5', '9'), // kCV59 - Character Variant 59.
  BL_MAKE_TAG('c', 'v', '6', '0'), // kCV60 - Character Variant 60.
  BL_MAKE_TAG('c', 'v', '6', '1'), // kCV61 - Character Variant 61.
  BL_MAKE_TAG('c', 'v', '6', '2'), // kCV62 - Character Variant 62.
  BL_MAKE_TAG('c', 'v', '6', '3'), // kCV63 - Character Variant 63.
  BL_MAKE_TAG('c', 'v', '6', '4'), // kCV64 - Character Variant 64.
  BL_MAKE_TAG('c', 'v', '6', '5'), // kCV65 - Character Variant 65.
  BL_MAKE_TAG('c', 'v', '6', '6'), // kCV66 - Character Variant 66.
  BL_MAKE_TAG('c', 'v', '6', '7'), // kCV67 - Character Variant 67.
  BL_MAKE_TAG('c', 'v', '6', '8'), // kCV68 - Character Variant 68.
  BL_MAKE_TAG('c', 'v', '6', '9'), // kCV69 - Character Variant 69.
  BL_MAKE_TAG('c', 'v', '7', '0'), // kCV70 - Character Variant 70.
  BL_MAKE_TAG('c', 'v', '7', '1'), // kCV71 - Character Variant 71.
  BL_MAKE_TAG('c', 'v', '7', '2'), // kCV72 - Character Variant 72.
  BL_MAKE_TAG('c', 'v', '7', '3'), // kCV73 - Character Variant 73.
  BL_MAKE_TAG('c', 'v', '7', '4'), // kCV74 - Character Variant 74.
  BL_MAKE_TAG('c', 'v', '7', '5'), // kCV75 - Character Variant 75.
  BL_MAKE_TAG('c', 'v', '7', '6'), // kCV76 - Character Variant 76.
  BL_MAKE_TAG('c', 'v', '7', '7'), // kCV77 - Character Variant 77.
  BL_MAKE_TAG('c', 'v', '7', '8'), // kCV78 - Character Variant 78.
  BL_MAKE_TAG('c', 'v', '7', '9'), // kCV79 - Character Variant 79.
  BL_MAKE_TAG('c', 'v', '8', '0'), // kCV80 - Character Variant 80.
  BL_MAKE_TAG('c', 'v', '8', '1'), // kCV81 - Character Variant 81.
  BL_MAKE_TAG('c', 'v', '8', '2'), // kCV82 - Character Variant 82.
  BL_MAKE_TAG('c', 'v', '8', '3'), // kCV83 - Character Variant 83.
  BL_MAKE_TAG('c', 'v', '8', '4'), // kCV84 - Character Variant 84.
  BL_MAKE_TAG('c', 'v', '8', '5'), // kCV85 - Character Variant 85.
  BL_MAKE_TAG('c', 'v', '8', '6'), // kCV86 - Character Variant 86.
  BL_MAKE_TAG('c', 'v', '8', '7'), // kCV87 - Character Variant 87.
  BL_MAKE_TAG('c', 'v', '8', '8'), // kCV88 - Character Variant 88.
  BL_MAKE_TAG('c', 'v', '8', '9'), // kCV89 - Character Variant 89.
  BL_MAKE_TAG('c', 'v', '9', '0'), // kCV90 - Character Variant 90.
  BL_MAKE_TAG('c', 'v', '9', '1'), // kCV91 - Character Variant 91.
  BL_MAKE_TAG('c', 'v', '9', '2'), // kCV92 - Character Variant 92.
  BL_MAKE_TAG('c', 'v', '9', '3'), // kCV93 - Character Variant 93.
  BL_MAKE_TAG('c', 'v', '9', '4'), // kCV94 - Character Variant 94.
  BL_MAKE_TAG('c', 'v', '9', '5'), // kCV95 - Character Variant 95.
  BL_MAKE_TAG('c', 'v', '9', '6'), // kCV96 - Character Variant 96.
  BL_MAKE_TAG('c', 'v', '9', '7'), // kCV97 - Character Variant 97.
  BL_MAKE_TAG('c', 'v', '9', '8'), // kCV98 - Character Variant 98.
  BL_MAKE_TAG('c', 'v', '9', '9'), // kCV99 - Character Variant 99.
  BL_MAKE_TAG('d', 'i', 's', 't'), // kDIST - Distances.
  BL_MAKE_TAG('d', 'l', 'i', 'g'), // kDLIG - Discretionary Ligatures.
  BL_MAKE_TAG('d', 'n', 'o', 'm'), // kDNOM - Denominators.
  BL_MAKE_TAG('d', 't', 'l', 's'), // kDTLS - Dotless Forms.
  BL_MAKE_TAG('e', 'x', 'p', 't'), // kEXPT - Expert Forms.
  BL_MAKE_TAG('f', 'a', 'l', 't'), // kFALT - Final Glyph on Line Alternates.
  BL_MAKE_TAG('f', 'i', 'n', '2'), // kFIN2 - Terminal Forms #2.
  BL_MAKE_TAG('f', 'i', 'n', '3'), // kFIN3 - Terminal Forms #3.
  BL_MAKE_TAG('f', 'i', 'n', 'a'), // kFINA - Terminal Forms.
  BL_MAKE_TAG('f', 'l', 'a', 'c'), // kFLAC - Flattened accent forms.
  BL_MAKE_TAG('f', 'r', 'a', 'c'), // kFRAC - Fractions.
  BL_MAKE_TAG('f', 'w', 'i', 'd'), // kFWID - Full Widths.
  BL_MAKE_TAG('h', 'a', 'l', 'f'), // kHALF - Half Forms.
  BL_MAKE_TAG('h', 'a', 'l', 'n'), // kHALN - Halant Forms.
  BL_MAKE_TAG('h', 'a', 'l', 't'), // kHALT - Alternate Half Widths.
  BL_MAKE_TAG('h', 'i', 's', 't'), // kHIST - Historical Forms.
  BL_MAKE_TAG('h', 'k', 'n', 'a'), // kHKNA - Horizontal Kana Alternates.
  BL_MAKE_TAG('h', 'l', 'i', 'g'), // kHLIG - Historical Ligatures.
  BL_MAKE_TAG('h', 'n', 'g', 'l'), // kHNGL - Hangul.
  BL_MAKE_TAG('h', 'o', 'j', 'o'), // kHOJO - Hojo Kanji Forms (JIS X 0212-1990 Kanji Forms).
  BL_MAKE_TAG('h', 'w', 'i', 'd'), // kHWID - Half Widths.
  BL_MAKE_TAG('i', 'n', 'i', 't'), // kINIT - Initial Forms.
  BL_MAKE_TAG('i', 's', 'o', 'l'), // kISOL - Isolated Forms.
  BL_MAKE_TAG('i', 't', 'a', 'l'), // kITAL - Italics.
  BL_MAKE_TAG('j', 'a', 'l', 't'), // kJALT - Justification Alternates.
  BL_MAKE_TAG('j', 'p', '0', '4'), // kJP04 - JIS2004 Forms.
  BL_MAKE_TAG('j', 'p', '7', '8'), // kJP78 - JIS78 Forms.
  BL_MAKE_TAG('j', 'p', '8', '3'), // kJP83 - JIS83 Forms.
  BL_MAKE_TAG('j', 'p', '9', '0'), // kJP90 - JIS90 Forms.
  BL_MAKE_TAG('k', 'e', 'r', 'n'), // kKERN - Kerning.
  BL_MAKE_TAG('l', 'f', 'b', 'd'), // kLFBD - Left Bounds.
  BL_MAKE_TAG('l', 'i', 'g', 'a'), // kLIGA - Standard Ligatures.
  BL_MAKE_TAG('l', 'j', 'm', 'o'), // kLJMO - Leading Jamo Forms.
  BL_MAKE_TAG('l', 'n', 'u', 'm'), // kLNUM - Lining Figures.
  BL_MAKE_TAG('l', 'o', 'c', 'l'), // kLOCL - Localized Forms.
  BL_MAKE_TAG('l', 't', 'r', 'a'), // kLTRA - Left-to-right alternates.
  BL_MAKE_TAG('l', 't', 'r', 'm'), // kLTRM - Left-to-right mirrored forms.
  BL_MAKE_TAG('m', 'a', 'r', 'k'), // kMARK - Mark Positioning.
  BL_MAKE_TAG('m', 'e', 'd', '2'), // kMED2 - Medial Forms #2.
  BL_MAKE_TAG('m', 'e', 'd', 'i'), // kMEDI - Medial Forms.
  BL_MAKE_TAG('m', 'g', 'r', 'k'), // kMGRK - Mathematical Greek.
  BL_MAKE_TAG('m', 'k', 'm', 'k'), // kMKMK - Mark to Mark Positioning.
  BL_MAKE_TAG('m', 's', 'e', 't'), // kMSET - Mark Positioning via Substitution.
  BL_MAKE_TAG('n', 'a', 'l', 't'), // kNALT - Alternate Annotation Forms.
  BL_MAKE_TAG('n', 'l', 'c', 'k'), // kNLCK - NLC Kanji Forms.
  BL_MAKE_TAG('n', 'u', 'k', 't'), // kNUKT - Nukta Forms.
  BL_MAKE_TAG('n', 'u', 'm', 'r'), // kNUMR - Numerators.
  BL_MAKE_TAG('o', 'n', 'u', 'm'), // kONUM - Oldstyle Figures.
  BL_MAKE_TAG('o', 'p', 'b', 'd'), // kOPBD - Optical Bounds.
  BL_MAKE_TAG('o', 'r', 'd', 'n'), // kORDN - Ordinals.
  BL_MAKE_TAG('o', 'r', 'n', 'm'), // kORNM - Ornaments.
  BL_MAKE_TAG('p', 'a', 'l', 't'), // kPALT - Proportional Alternate Widths.
  BL_MAKE_TAG('p', 'c', 'a', 'p'), // kPCAP - Petite Capitals.
  BL_MAKE_TAG('p', 'k', 'n', 'a'), // kPKNA - Proportional Kana.
  BL_MAKE_TAG('p', 'n', 'u', 'm'), // kPNUM - Proportional Figures.
  BL_MAKE_TAG('p', 'r', 'e', 'f'), // kPREF - Pre-Base Forms.
  BL_MAKE_TAG('p', 'r', 'e', 's'), // kPRES - Pre-base Substitutions.
  BL_MAKE_TAG('p', 's', 't', 'f'), // kPSTF - Post-base Forms.
  BL_MAKE_TAG('p', 's', 't', 's'), // kPSTS - Post-base Substitutions.
  BL_MAKE_TAG('p', 'w', 'i', 'd'), // kPWID - Proportional Widths.
  BL_MAKE_TAG('q', 'w', 'i', 'd'), // kQWID - Quarter Widths.
  BL_MAKE_TAG('r', 'a', 'n', 'd'), // kRAND - Randomize.
  BL_MAKE_TAG('r', 'c', 'l', 't'), // kRCLT - Required Contextual Alternates.
  BL_MAKE_TAG('r', 'k', 'r', 'f'), // kRKRF - Rakar Forms.
  BL_MAKE_TAG('r', 'l', 'i', 'g'), // kRLIG - Required Ligatures.
  BL_MAKE_TAG('r', 'p', 'h', 'f'), // kRPHF - Reph Forms.
  BL_MAKE_TAG('r', 't', 'b', 'd'), // kRTBD - Right Bounds.
  BL_MAKE_TAG('r', 't', 'l', 'a'), // kRTLA - Right-to-left alternates.
  BL_MAKE_TAG('r', 't', 'l', 'm'), // kRTLM - Right-to-left mirrored forms.
  BL_MAKE_TAG('r', 'u', 'b', 'y'), // kRUBY - Ruby Notation Forms.
  BL_MAKE_TAG('r', 'v', 'r', 'n'), // kRVRN - Required Variation Alternates.
  BL_MAKE_TAG('s', 'a', 'l', 't'), // kSALT - Stylistic Alternates.
  BL_MAKE_TAG('s', 'i', 'n', 'f'), // kSINF - Scientific Inferiors.
  BL_MAKE_TAG('s', 'i', 'z', 'e'), // kSIZE - Optical size.
  BL_MAKE_TAG('s', 'm', 'c', 'p'), // kSMCP - Small Capitals.
  BL_MAKE_TAG('s', 'm', 'p', 'l'), // kSMPL - Simplified Forms.
  BL_MAKE_TAG('s', 's', '0', '1'), // kSS01 - Stylistic Set 1.
  BL_MAKE_TAG('s', 's', '0', '2'), // kSS02 - Stylistic Set 2.
  BL_MAKE_TAG('s', 's', '0', '3'), // kSS03 - Stylistic Set 3.
  BL_MAKE_TAG('s', 's', '0', '4'), // kSS04 - Stylistic Set 4.
  BL_MAKE_TAG('s', 's', '0', '5'), // kSS05 - Stylistic Set 5.
  BL_MAKE_TAG('s', 's', '0', '6'), // kSS06 - Stylistic Set 6.
  BL_MAKE_TAG('s', 's', '0', '7'), // kSS07 - Stylistic Set 7.
  BL_MAKE_TAG('s', 's', '0', '8'), // kSS08 - Stylistic Set 8.
  BL_MAKE_TAG('s', 's', '0', '9'), // kSS09 - Stylistic Set 9.
  BL_MAKE_TAG('s', 's', '1', '0'), // kSS10 - Stylistic Set 10.
  BL_MAKE_TAG('s', 's', '1', '1'), // kSS11 - Stylistic Set 11.
  BL_MAKE_TAG('s', 's', '1', '2'), // kSS12 - Stylistic Set 12.
  BL_MAKE_TAG('s', 's', '1', '3'), // kSS13 - Stylistic Set 13.
  BL_MAKE_TAG('s', 's', '1', '4'), // kSS14 - Stylistic Set 14.
  BL_MAKE_TAG('s', 's', '1', '5'), // kSS15 - Stylistic Set 15.
  BL_MAKE_TAG('s', 's', '1', '6'), // kSS16 - Stylistic Set 16.
  BL_MAKE_TAG('s', 's', '1', '7'), // kSS17 - Stylistic Set 17.
  BL_MAKE_TAG('s', 's', '1', '8'), // kSS18 - Stylistic Set 18.
  BL_MAKE_TAG('s', 's', '1', '9'), // kSS19 - Stylistic Set 19.
  BL_MAKE_TAG('s', 's', '2', '0'), // kSS20 - Stylistic Set 20.
  BL_MAKE_TAG('s', 's', 't', 'y'), // kSSTY - Math script style alternates.
  BL_MAKE_TAG('s', 't', 'c', 'h'), // kSTCH - Stretching Glyph Decomposition.
  BL_MAKE_TAG('s', 'u', 'b', 's'), // kSUBS - Subscript.
  BL_MAKE_TAG('s', 'u', 'p', 's'), // kSUPS - Superscript.
  BL_MAKE_TAG('s', 'w', 's', 'h'), // kSWSH - Swash.
  BL_MAKE_TAG('t', 'i', 't', 'l'), // kTITL - Titling.
  BL_MAKE_TAG('t', 'j', 'm', 'o'), // kTJMO - Trailing Jamo Forms.
  BL_MAKE_TAG('t', 'n', 'a', 'm'), // kTNAM - Traditional Name Forms.
  BL_MAKE_TAG('t', 'n', 'u', 'm'), // kTNUM - Tabular Figures.
  BL_MAKE_TAG('t', 'r', 'a', 'd'), // kTRAD - Traditional Forms.
  BL_MAKE_TAG('t', 'w', 'i', 'd'), // kTWID - Third Widths.
  BL_MAKE_TAG('u', 'n', 'i', 'c'), // kUNIC - Unicase.
  BL_MAKE_TAG('v', 'a', 'l', 't'), // kVALT - Alternate Vertical Metrics.
  BL_MAKE_TAG('v', 'a', 't', 'u'), // kVATU - Vattu Variants.
  BL_MAKE_TAG('v', 'c', 'h', 'w'), // kVCHW - Vertical Contextual Half-width Spacing.
  BL_MAKE_TAG('v', 'e', 'r', 't'), // kVERT - Vertical Writing.
  BL_MAKE_TAG('v', 'h', 'a', 'l'), // kVHAL - Alternate Vertical Half Metrics.
  BL_MAKE_TAG('v', 'j', 'm', 'o'), // kVJMO - Vowel Jamo Forms.
  BL_MAKE_TAG('v', 'k', 'n', 'a'), // kVKNA - Vertical Kana Alternates.
  BL_MAKE_TAG('v', 'k', 'r', 'n'), // kVKRN - Vertical Kerning.
  BL_MAKE_TAG('v', 'p', 'a', 'l'), // kVPAL - Proportional Alternate Vertical Metrics.
  BL_MAKE_TAG('v', 'r', 't', '2'), // kVRT2 - Vertical Alternates and Rotation.
  BL_MAKE_TAG('v', 'r', 't', 'r'), // kVRTR - Vertical Alternates for Rotation.
  BL_MAKE_TAG('z', 'e', 'r', 'o')  // kZERO - Slashed Zero.
};

// bl::FontTagData - Baseline IDs
// ==============================

const BLTag baseline_id_to_tag_table[] = {
  BL_MAKE_TAG('I', 'c', 'f', 'c'), // kICFC - Ideographic face center.
  BL_MAKE_TAG('I', 'd', 'c', 'e'), // kIDCE - Ideographics em-box center.
  BL_MAKE_TAG('h', 'a', 'n', 'g'), // kHANG - The hanging baseline.
  BL_MAKE_TAG('i', 'c', 'f', 'b'), // kICFB - Ideographic face bottom edge.
  BL_MAKE_TAG('i', 'c', 'f', 't'), // kICFT - Ideographic face top edge.
  BL_MAKE_TAG('i', 'd', 'e', 'o'), // kIDEO - Ideographic em-box bottom edge.
  BL_MAKE_TAG('i', 'd', 't', 'p'), // kIDTP - Ideographic em-box top edge baseline.
  BL_MAKE_TAG('m', 'a', 't', 'h'), // kMATH - Math characters baseline.
  BL_MAKE_TAG('r', 'o', 'm', 'n')  // kROMN - Alphabetic scripts baseline (Latin, Cyrillic, Greek, ...).
};

// bl::FontTagData - Variation IDs
// ===============================

const BLTag variation_id_to_tag_table[] = {
  BL_MAKE_TAG('i', 't', 'a', 'l'), // kITAL - Italic.
  BL_MAKE_TAG('o', 'p', 's', 'z'), // kOPSZ - Optical size.
  BL_MAKE_TAG('s', 'l', 'n', 't'), // kSLNT - Slant.
  BL_MAKE_TAG('w', 'd', 't', 'h'), // kWDTH - Width.
  BL_MAKE_TAG('w', 'g', 'h', 't')  // kWGHT - Weight.
};

} // {FontTagData}
} // {bl}
