// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_FONTDEFS_H_INCLUDED
#define BLEND2D_FONTDEFS_H_INCLUDED

#include <blend2d/core/geometry.h>

//! \addtogroup bl_text
//! \{

//! \name BLFont Related Constants
//! \{

//! Orientation.
BL_DEFINE_ENUM(BLOrientation) {
  //! Horizontal orientation.
  BL_ORIENTATION_HORIZONTAL = 0,
  //! Vertical orientation.
  BL_ORIENTATION_VERTICAL = 1,

  //! Maximum value of `BLOrientation`.
  BL_ORIENTATION_MAX_VALUE = 1

  BL_FORCE_ENUM_UINT32(BL_ORIENTATION)
};

//! Type of a font or font face, see \ref BLFontFace (or \ref BLFontFaceCore).
BL_DEFINE_ENUM(BLFontFaceType) {
  //! None or unknown font type.
  BL_FONT_FACE_TYPE_NONE = 0,
  //! TrueType/OpenType font type (.ttf/.otf files and font collections).
  BL_FONT_FACE_TYPE_OPENTYPE = 1,

  //! Maximum value of `BLFontFaceType`.
  BL_FONT_FACE_TYPE_MAX_VALUE = 1

  BL_FORCE_ENUM_UINT32(BL_FONT_FACE_TYPE)
};

//! Font stretch.
BL_DEFINE_ENUM(BLFontStretch) {
  //! Ultra condensed stretch.
  BL_FONT_STRETCH_ULTRA_CONDENSED = 1,
  //! Extra condensed stretch.
  BL_FONT_STRETCH_EXTRA_CONDENSED = 2,
  //! Condensed stretch.
  BL_FONT_STRETCH_CONDENSED = 3,
  //! Semi condensed stretch.
  BL_FONT_STRETCH_SEMI_CONDENSED = 4,
  //! Normal stretch.
  BL_FONT_STRETCH_NORMAL = 5,
  //! Semi expanded stretch.
  BL_FONT_STRETCH_SEMI_EXPANDED = 6,
  //! Expanded stretch.
  BL_FONT_STRETCH_EXPANDED = 7,
  //! Extra expanded stretch.
  BL_FONT_STRETCH_EXTRA_EXPANDED = 8,
  //! Ultra expanded stretch.
  BL_FONT_STRETCH_ULTRA_EXPANDED = 9,

  //! Maximum value of `BLFontStretch`.
  BL_FONT_STRETCH_MAX_VALUE = 9

  BL_FORCE_ENUM_UINT32(BL_FONT_STRETCH)
};

//! Font style.
BL_DEFINE_ENUM(BLFontStyle) {
  //! Normal style.
  BL_FONT_STYLE_NORMAL = 0,
  //! Oblique.
  BL_FONT_STYLE_OBLIQUE = 1,
  //! Italic.
  BL_FONT_STYLE_ITALIC = 2,

  //! Maximum value of `BLFontStyle`.
  BL_FONT_STYLE_MAX_VALUE = 2

  BL_FORCE_ENUM_UINT32(BL_FONT_STYLE)
};

//! Font weight.
BL_DEFINE_ENUM(BLFontWeight) {
  //! Thin weight (100).
  BL_FONT_WEIGHT_THIN = 100,
  //! Extra light weight (200).
  BL_FONT_WEIGHT_EXTRA_LIGHT = 200,
  //! Light weight (300).
  BL_FONT_WEIGHT_LIGHT = 300,
  //! Semi light weight (350).
  BL_FONT_WEIGHT_SEMI_LIGHT = 350,
  //! Normal weight (400).
  BL_FONT_WEIGHT_NORMAL = 400,
  //! Medium weight (500).
  BL_FONT_WEIGHT_MEDIUM = 500,
  //! Semi bold weight (600).
  BL_FONT_WEIGHT_SEMI_BOLD = 600,
  //! Bold weight (700).
  BL_FONT_WEIGHT_BOLD = 700,
  //! Extra bold weight (800).
  BL_FONT_WEIGHT_EXTRA_BOLD = 800,
  //! Black weight (900).
  BL_FONT_WEIGHT_BLACK = 900,
  //! Extra black weight (950).
  BL_FONT_WEIGHT_EXTRA_BLACK = 950

  BL_FORCE_ENUM_UINT32(BL_FONT_WEIGHT)
};

//! Font string identifiers used by OpenType 'name' table.
BL_DEFINE_ENUM(BLFontStringId) {
  //! Copyright notice.
  BL_FONT_STRING_ID_COPYRIGHT_NOTICE = 0,
  //! Font family name.
  BL_FONT_STRING_ID_FAMILY_NAME = 1,
  //! Font subfamily name.
  BL_FONT_STRING_ID_SUBFAMILY_NAME = 2,
  //! Unique font identifier.
  BL_FONT_STRING_ID_UNIQUE_IDENTIFIER = 3,
  //! Full font name that reflects all family and relevant subfamily descriptors.
  BL_FONT_STRING_ID_FULL_NAME = 4,
  //! Version string. Should begin with the synta `Version <number>.<number>`.
  BL_FONT_STRING_ID_VERSION_STRING = 5,
  //! PostScript name for the font.
  BL_FONT_STRING_ID_POST_SCRIPT_NAME = 6,
  //! Trademark notice/information for this font.
  BL_FONT_STRING_ID_TRADEMARK = 7,
  //! Manufacturer name.
  BL_FONT_STRING_ID_MANUFACTURER_NAME = 8,
  //! Name of the designer of the typeface.
  BL_FONT_STRING_ID_DESIGNER_NAME = 9,
  //! Description of the typeface.
  BL_FONT_STRING_ID_DESCRIPTION = 10,
  //! URL of font vendor.
  BL_FONT_STRING_ID_VENDOR_URL = 11,
  //! URL of typeface designer.
  BL_FONT_STRING_ID_DESIGNER_URL = 12,
  //! Description of how the font may be legally used.
  BL_FONT_STRING_ID_LICENSE_DESCRIPTION = 13,
  //! URL where additional licensing information can be found.
  BL_FONT_STRING_ID_LICENSE_INFO_URL = 14,
  //! Reserved.
  BL_FONT_STRING_ID_RESERVED = 15,
  //! Typographic family name.
  BL_FONT_STRING_ID_TYPOGRAPHIC_FAMILY_NAME = 16,
  //! Typographic subfamily name.
  BL_FONT_STRING_ID_TYPOGRAPHIC_SUBFAMILY_NAME = 17,
  //! Compatible full name (MAC only).
  BL_FONT_STRING_ID_COMPATIBLE_FULL_NAME = 18,
  //! Sample text - font name or any other text from the designer.
  BL_FONT_STRING_ID_SAMPLE_TEXT = 19,
  //! PostScript CID findfont name.
  BL_FONT_STRING_ID_POST_SCRIPT_CID_NAME = 20,
  //! WWS family name.
  BL_FONT_STRING_ID_WWS_FAMILY_NAME = 21,
  //! WWS subfamily name.
  BL_FONT_STRING_ID_WWS_SUBFAMILY_NAME = 22,
  //! Light background palette.
  BL_FONT_STRING_ID_LIGHT_BACKGROUND_PALETTE = 23,
  //! Dark background palette.
  BL_FONT_STRING_ID_DARK_BACKGROUND_PALETTE = 24,
  //! Variations PostScript name prefix.
  BL_FONT_STRING_ID_VARIATIONS_POST_SCRIPT_PREFIX = 25,

  //! Count of common font string ids.
  BL_FONT_STRING_ID_COMMON_MAX_VALUE = 26,
  //! Start of custom font string ids.
  BL_FONT_STRING_ID_CUSTOM_START_INDEX = 255

  BL_FORCE_ENUM_UINT32(BL_FONT_STRING_ID)
};

//! Bit positions used by \ref BLFontCoverageInfo structure.
//!
//! Each bit represents a range (or multiple ranges) of unicode characters.
BL_DEFINE_ENUM(BLFontCoverageGroup) {
  BL_FONT_COVERAGE_GROUP_BASIC_LATIN,                              //!< [000000-00007F] Basic Latin.
  BL_FONT_COVERAGE_GROUP_LATIN1_SUPPLEMENT,                        //!< [000080-0000FF] Latin-1 Supplement.
  BL_FONT_COVERAGE_GROUP_LATIN_EXTENDED_A,                         //!< [000100-00017F] Latin Extended-A.
  BL_FONT_COVERAGE_GROUP_LATIN_EXTENDED_B,                         //!< [000180-00024F] Latin Extended-B.
  BL_FONT_COVERAGE_GROUP_IPA_EXTENSIONS,                           //!< [000250-0002AF] IPA Extensions.
                                                                   //!< [001D00-001D7F] Phonetic Extensions.
                                                                   //!< [001D80-001DBF] Phonetic Extensions Supplement.
  BL_FONT_COVERAGE_GROUP_SPACING_MODIFIER_LETTERS,                 //!< [0002B0-0002FF] Spacing Modifier Letters.
                                                                   //!< [00A700-00A71F] Modifier Tone Letters.
                                                                   //!< [001DC0-001DFF] Combining Diacritical Marks Supplement.
  BL_FONT_COVERAGE_GROUP_COMBINING_DIACRITICAL_MARKS,              //!< [000300-00036F] Combining Diacritical Marks.
  BL_FONT_COVERAGE_GROUP_GREEK_AND_COPTIC,                         //!< [000370-0003FF] Greek and Coptic.
  BL_FONT_COVERAGE_GROUP_COPTIC,                                   //!< [002C80-002CFF] Coptic.
  BL_FONT_COVERAGE_GROUP_CYRILLIC,                                 //!< [000400-0004FF] Cyrillic.
                                                                   //!< [000500-00052F] Cyrillic Supplement.
                                                                   //!< [002DE0-002DFF] Cyrillic Extended-A.
                                                                   //!< [00A640-00A69F] Cyrillic Extended-B.
  BL_FONT_COVERAGE_GROUP_ARMENIAN,                                 //!< [000530-00058F] Armenian.
  BL_FONT_COVERAGE_GROUP_HEBREW,                                   //!< [000590-0005FF] Hebrew.
  BL_FONT_COVERAGE_GROUP_VAI,                                      //!< [00A500-00A63F] Vai.
  BL_FONT_COVERAGE_GROUP_ARABIC,                                   //!< [000600-0006FF] Arabic.
                                                                   //!< [000750-00077F] Arabic Supplement.
  BL_FONT_COVERAGE_GROUP_NKO,                                      //!< [0007C0-0007FF] NKo.
  BL_FONT_COVERAGE_GROUP_DEVANAGARI,                               //!< [000900-00097F] Devanagari.
  BL_FONT_COVERAGE_GROUP_BENGALI,                                  //!< [000980-0009FF] Bengali.
  BL_FONT_COVERAGE_GROUP_GURMUKHI,                                 //!< [000A00-000A7F] Gurmukhi.
  BL_FONT_COVERAGE_GROUP_GUJARATI,                                 //!< [000A80-000AFF] Gujarati.
  BL_FONT_COVERAGE_GROUP_ORIYA,                                    //!< [000B00-000B7F] Oriya.
  BL_FONT_COVERAGE_GROUP_TAMIL,                                    //!< [000B80-000BFF] Tamil.
  BL_FONT_COVERAGE_GROUP_TELUGU,                                   //!< [000C00-000C7F] Telugu.
  BL_FONT_COVERAGE_GROUP_KANNADA,                                  //!< [000C80-000CFF] Kannada.
  BL_FONT_COVERAGE_GROUP_MALAYALAM,                                //!< [000D00-000D7F] Malayalam.
  BL_FONT_COVERAGE_GROUP_THAI,                                     //!< [000E00-000E7F] Thai.
  BL_FONT_COVERAGE_GROUP_LAO,                                      //!< [000E80-000EFF] Lao.
  BL_FONT_COVERAGE_GROUP_GEORGIAN,                                 //!< [0010A0-0010FF] Georgian.
                                                                   //!< [002D00-002D2F] Georgian Supplement.
  BL_FONT_COVERAGE_GROUP_BALINESE,                                 //!< [001B00-001B7F] Balinese.
  BL_FONT_COVERAGE_GROUP_HANGUL_JAMO,                              //!< [001100-0011FF] Hangul Jamo.
  BL_FONT_COVERAGE_GROUP_LATIN_EXTENDED_ADDITIONAL,                //!< [001E00-001EFF] Latin Extended Additional.
                                                                   //!< [002C60-002C7F] Latin Extended-C.
                                                                   //!< [00A720-00A7FF] Latin Extended-D.
  BL_FONT_COVERAGE_GROUP_GREEK_EXTENDED,                           //!< [001F00-001FFF] Greek Extended.
  BL_FONT_COVERAGE_GROUP_GENERAL_PUNCTUATION,                      //!< [002000-00206F] General Punctuation.
                                                                   //!< [002E00-002E7F] Supplemental Punctuation.
  BL_FONT_COVERAGE_GROUP_SUPERSCRIPTS_AND_SUBSCRIPTS,              //!< [002070-00209F] Superscripts And Subscripts.
  BL_FONT_COVERAGE_GROUP_CURRENCY_SYMBOLS,                         //!< [0020A0-0020CF] Currency Symbols.
  BL_FONT_COVERAGE_GROUP_COMBINING_DIACRITICAL_MARKS_FOR_SYMBOLS,  //!< [0020D0-0020FF] Combining Diacritical Marks For Symbols.
  BL_FONT_COVERAGE_GROUP_LETTERLIKE_SYMBOLS,                       //!< [002100-00214F] Letterlike Symbols.
  BL_FONT_COVERAGE_GROUP_NUMBER_FORMS,                             //!< [002150-00218F] Number Forms.
  BL_FONT_COVERAGE_GROUP_ARROWS,                                   //!< [002190-0021FF] Arrows.
                                                                   //!< [0027F0-0027FF] Supplemental Arrows-A.
                                                                   //!< [002900-00297F] Supplemental Arrows-B.
                                                                   //!< [002B00-002BFF] Miscellaneous Symbols and Arrows.
  BL_FONT_COVERAGE_GROUP_MATHEMATICAL_OPERATORS,                   //!< [002200-0022FF] Mathematical Operators.
                                                                   //!< [002A00-002AFF] Supplemental Mathematical Operators.
                                                                   //!< [0027C0-0027EF] Miscellaneous Mathematical Symbols-A.
                                                                   //!< [002980-0029FF] Miscellaneous Mathematical Symbols-B.
  BL_FONT_COVERAGE_GROUP_MISCELLANEOUS_TECHNICAL,                  //!< [002300-0023FF] Miscellaneous Technical.
  BL_FONT_COVERAGE_GROUP_CONTROL_PICTURES,                         //!< [002400-00243F] Control Pictures.
  BL_FONT_COVERAGE_GROUP_OPTICAL_CHARACTER_RECOGNITION,            //!< [002440-00245F] Optical Character Recognition.
  BL_FONT_COVERAGE_GROUP_ENCLOSED_ALPHANUMERICS,                   //!< [002460-0024FF] Enclosed Alphanumerics.
  BL_FONT_COVERAGE_GROUP_BOX_DRAWING,                              //!< [002500-00257F] Box Drawing.
  BL_FONT_COVERAGE_GROUP_BLOCK_ELEMENTS,                           //!< [002580-00259F] Block Elements.
  BL_FONT_COVERAGE_GROUP_GEOMETRIC_SHAPES,                         //!< [0025A0-0025FF] Geometric Shapes.
  BL_FONT_COVERAGE_GROUP_MISCELLANEOUS_SYMBOLS,                    //!< [002600-0026FF] Miscellaneous Symbols.
  BL_FONT_COVERAGE_GROUP_DINGBATS,                                 //!< [002700-0027BF] Dingbats.
  BL_FONT_COVERAGE_GROUP_CJK_SYMBOLS_AND_PUNCTUATION,              //!< [003000-00303F] CJK Symbols And Punctuation.
  BL_FONT_COVERAGE_GROUP_HIRAGANA,                                 //!< [003040-00309F] Hiragana.
  BL_FONT_COVERAGE_GROUP_KATAKANA,                                 //!< [0030A0-0030FF] Katakana.
                                                                   //!< [0031F0-0031FF] Katakana Phonetic Extensions.
  BL_FONT_COVERAGE_GROUP_BOPOMOFO,                                 //!< [003100-00312F] Bopomofo.
                                                                   //!< [0031A0-0031BF] Bopomofo Extended.
  BL_FONT_COVERAGE_GROUP_HANGUL_COMPATIBILITY_JAMO,                //!< [003130-00318F] Hangul Compatibility Jamo.
  BL_FONT_COVERAGE_GROUP_PHAGS_PA,                                 //!< [00A840-00A87F] Phags-pa.
  BL_FONT_COVERAGE_GROUP_ENCLOSED_CJK_LETTERS_AND_MONTHS,          //!< [003200-0032FF] Enclosed CJK Letters And Months.
  BL_FONT_COVERAGE_GROUP_CJK_COMPATIBILITY,                        //!< [003300-0033FF] CJK Compatibility.
  BL_FONT_COVERAGE_GROUP_HANGUL_SYLLABLES,                         //!< [00AC00-00D7AF] Hangul Syllables.
  BL_FONT_COVERAGE_GROUP_NON_PLANE,                                //!< [00D800-00DFFF] Non-Plane 0 *.
  BL_FONT_COVERAGE_GROUP_PHOENICIAN,                               //!< [010900-01091F] Phoenician.
  BL_FONT_COVERAGE_GROUP_CJK_UNIFIED_IDEOGRAPHS,                   //!< [004E00-009FFF] CJK Unified Ideographs.
                                                                   //!< [002E80-002EFF] CJK Radicals Supplement.
                                                                   //!< [002F00-002FDF] Kangxi Radicals.
                                                                   //!< [002FF0-002FFF] Ideographic Description Characters.
                                                                   //!< [003400-004DBF] CJK Unified Ideographs Extension A.
                                                                   //!< [020000-02A6DF] CJK Unified Ideographs Extension B.
                                                                   //!< [003190-00319F] Kanbun.
  BL_FONT_COVERAGE_GROUP_PRIVATE_USE_PLANE0,                       //!< [00E000-00F8FF] Private Use (Plane 0).
  BL_FONT_COVERAGE_GROUP_CJK_STROKES,                              //!< [0031C0-0031EF] CJK Strokes.
                                                                   //!< [00F900-00FAFF] CJK Compatibility Ideographs.
                                                                   //!< [02F800-02FA1F] CJK Compatibility Ideographs Supplement.
  BL_FONT_COVERAGE_GROUP_ALPHABETIC_PRESENTATION_FORMS,            //!< [00FB00-00FB4F] Alphabetic Presentation Forms.
  BL_FONT_COVERAGE_GROUP_ARABIC_PRESENTATION_FORMS_A,              //!< [00FB50-00FDFF] Arabic Presentation Forms-A.
  BL_FONT_COVERAGE_GROUP_COMBINING_HALF_MARKS,                     //!< [00FE20-00FE2F] Combining Half Marks.
  BL_FONT_COVERAGE_GROUP_VERTICAL_FORMS,                           //!< [00FE10-00FE1F] Vertical Forms.
                                                                   //!< [00FE30-00FE4F] CJK Compatibility Forms.
  BL_FONT_COVERAGE_GROUP_SMALL_FORM_VARIANTS,                      //!< [00FE50-00FE6F] Small Form Variants.
  BL_FONT_COVERAGE_GROUP_ARABIC_PRESENTATION_FORMS_B,              //!< [00FE70-00FEFF] Arabic Presentation Forms-B.
  BL_FONT_COVERAGE_GROUP_HALFWIDTH_AND_FULLWIDTH_FORMS,            //!< [00FF00-00FFEF] Halfwidth And Fullwidth Forms.
  BL_FONT_COVERAGE_GROUP_SPECIALS,                                 //!< [00FFF0-00FFFF] Specials.
  BL_FONT_COVERAGE_GROUP_TIBETAN,                                  //!< [000F00-000FFF] Tibetan.
  BL_FONT_COVERAGE_GROUP_SYRIAC,                                   //!< [000700-00074F] Syriac.
  BL_FONT_COVERAGE_GROUP_THAANA,                                   //!< [000780-0007BF] Thaana.
  BL_FONT_COVERAGE_GROUP_SINHALA,                                  //!< [000D80-000DFF] Sinhala.
  BL_FONT_COVERAGE_GROUP_MYANMAR,                                  //!< [001000-00109F] Myanmar.
  BL_FONT_COVERAGE_GROUP_ETHIOPIC,                                 //!< [001200-00137F] Ethiopic.
                                                                   //!< [001380-00139F] Ethiopic Supplement.
                                                                   //!< [002D80-002DDF] Ethiopic Extended.
  BL_FONT_COVERAGE_GROUP_CHEROKEE,                                 //!< [0013A0-0013FF] Cherokee.
  BL_FONT_COVERAGE_GROUP_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS,    //!< [001400-00167F] Unified Canadian Aboriginal Syllabics.
  BL_FONT_COVERAGE_GROUP_OGHAM,                                    //!< [001680-00169F] Ogham.
  BL_FONT_COVERAGE_GROUP_RUNIC,                                    //!< [0016A0-0016FF] Runic.
  BL_FONT_COVERAGE_GROUP_KHMER,                                    //!< [001780-0017FF] Khmer.
                                                                   //!< [0019E0-0019FF] Khmer Symbols.
  BL_FONT_COVERAGE_GROUP_MONGOLIAN,                                //!< [001800-0018AF] Mongolian.
  BL_FONT_COVERAGE_GROUP_BRAILLE_PATTERNS,                         //!< [002800-0028FF] Braille Patterns.
  BL_FONT_COVERAGE_GROUP_YI_SYLLABLES_AND_RADICALS,                //!< [00A000-00A48F] Yi Syllables.
                                                                   //!< [00A490-00A4CF] Yi Radicals.
  BL_FONT_COVERAGE_GROUP_TAGALOG_HANUNOO_BUHID_TAGBANWA,           //!< [001700-00171F] Tagalog.
                                                                   //!< [001720-00173F] Hanunoo.
                                                                   //!< [001740-00175F] Buhid.
                                                                   //!< [001760-00177F] Tagbanwa.
  BL_FONT_COVERAGE_GROUP_OLD_ITALIC,                               //!< [010300-01032F] Old Italic.
  BL_FONT_COVERAGE_GROUP_GOTHIC,                                   //!< [010330-01034F] Gothic.
  BL_FONT_COVERAGE_GROUP_DESERET,                                  //!< [010400-01044F] Deseret.
  BL_FONT_COVERAGE_GROUP_MUSICAL_SYMBOLS,                          //!< [01D000-01D0FF] Byzantine Musical Symbols.
                                                                   //!< [01D100-01D1FF] Musical Symbols.
                                                                   //!< [01D200-01D24F] Ancient Greek Musical Notation.
  BL_FONT_COVERAGE_GROUP_MATHEMATICAL_ALPHANUMERIC_SYMBOLS,        //!< [01D400-01D7FF] Mathematical Alphanumeric Symbols.
  BL_FONT_COVERAGE_GROUP_PRIVATE_USE_PLANE_15_16,                  //!< [0F0000-0FFFFD] Private Use (Plane 15).
                                                                   //!< [100000-10FFFD] Private Use (Plane 16).
  BL_FONT_COVERAGE_GROUP_VARIATION_SELECTORS,                      //!< [00FE00-00FE0F] Variation Selectors.
                                                                   //!< [0E0100-0E01EF] Variation Selectors Supplement.
  BL_FONT_COVERAGE_GROUP_TAGS,                                     //!< [0E0000-0E007F] Tags.
  BL_FONT_COVERAGE_GROUP_LIMBU,                                    //!< [001900-00194F] Limbu.
  BL_FONT_COVERAGE_GROUP_TAI_LE,                                   //!< [001950-00197F] Tai Le.
  BL_FONT_COVERAGE_GROUP_NEW_TAI_LUE,                              //!< [001980-0019DF] New Tai Lue.
  BL_FONT_COVERAGE_GROUP_BUGINESE,                                 //!< [001A00-001A1F] Buginese.
  BL_FONT_COVERAGE_GROUP_GLAGOLITIC,                               //!< [002C00-002C5F] Glagolitic.
  BL_FONT_COVERAGE_GROUP_TIFINAGH,                                 //!< [002D30-002D7F] Tifinagh.
  BL_FONT_COVERAGE_GROUP_YIJING_HEXAGRAM_SYMBOLS,                  //!< [004DC0-004DFF] Yijing Hexagram Symbols.
  BL_FONT_COVERAGE_GROUP_SYLOTI_NAGRI,                             //!< [00A800-00A82F] Syloti Nagri.
  BL_FONT_COVERAGE_GROUP_LINEAR_B_SYLLABARY_AND_IDEOGRAMS,         //!< [010000-01007F] Linear B Syllabary.
                                                                   //!< [010080-0100FF] Linear B Ideograms.
                                                                   //!< [010100-01013F] Aegean Numbers.
  BL_FONT_COVERAGE_GROUP_ANCIENT_GREEK_NUMBERS,                    //!< [010140-01018F] Ancient Greek Numbers.
  BL_FONT_COVERAGE_GROUP_UGARITIC,                                 //!< [010380-01039F] Ugaritic.
  BL_FONT_COVERAGE_GROUP_OLD_PERSIAN,                              //!< [0103A0-0103DF] Old Persian.
  BL_FONT_COVERAGE_GROUP_SHAVIAN,                                  //!< [010450-01047F] Shavian.
  BL_FONT_COVERAGE_GROUP_OSMANYA,                                  //!< [010480-0104AF] Osmanya.
  BL_FONT_COVERAGE_GROUP_CYPRIOT_SYLLABARY,                        //!< [010800-01083F] Cypriot Syllabary.
  BL_FONT_COVERAGE_GROUP_KHAROSHTHI,                               //!< [010A00-010A5F] Kharoshthi.
  BL_FONT_COVERAGE_GROUP_TAI_XUAN_JING_SYMBOLS,                    //!< [01D300-01D35F] Tai Xuan Jing Symbols.
  BL_FONT_COVERAGE_GROUP_CUNEIFORM,                                //!< [012000-0123FF] Cuneiform.
                                                                   //!< [012400-01247F] Cuneiform Numbers and Punctuation.
  BL_FONT_COVERAGE_GROUP_COUNTING_ROD_NUMERALS,                    //!< [01D360-01D37F] Counting Rod Numerals.
  BL_FONT_COVERAGE_GROUP_SUNDANESE,                                //!< [001B80-001BBF] Sundanese.
  BL_FONT_COVERAGE_GROUP_LEPCHA,                                   //!< [001C00-001C4F] Lepcha.
  BL_FONT_COVERAGE_GROUP_OL_CHIKI,                                 //!< [001C50-001C7F] Ol Chiki.
  BL_FONT_COVERAGE_GROUP_SAURASHTRA,                               //!< [00A880-00A8DF] Saurashtra.
  BL_FONT_COVERAGE_GROUP_KAYAH_LI,                                 //!< [00A900-00A92F] Kayah Li.
  BL_FONT_COVERAGE_GROUP_REJANG,                                   //!< [00A930-00A95F] Rejang.
  BL_FONT_COVERAGE_GROUP_CHAM,                                     //!< [00AA00-00AA5F] Cham.
  BL_FONT_COVERAGE_GROUP_ANCIENT_SYMBOLS,                          //!< [010190-0101CF] Ancient Symbols.
  BL_FONT_COVERAGE_GROUP_PHAISTOS_DISC,                            //!< [0101D0-0101FF] Phaistos Disc.
  BL_FONT_COVERAGE_GROUP_CARIAN_LYCIAN_LYDIAN,                     //!< [0102A0-0102DF] Carian.
                                                                   //!< [010280-01029F] Lycian.
                                                                   //!< [010920-01093F] Lydian.
  BL_FONT_COVERAGE_GROUP_DOMINO_AND_MAHJONG_TILES,                 //!< [01F030-01F09F] Domino Tiles.
                                                                   //!< [01F000-01F02F] Mahjong Tiles.
  BL_FONT_COVERAGE_GROUP_INTERNAL_USAGE_123,                       //!< Reserved for internal usage (123).
  BL_FONT_COVERAGE_GROUP_INTERNAL_USAGE_124,                       //!< Reserved for internal usage (124).
  BL_FONT_COVERAGE_GROUP_INTERNAL_USAGE_125,                       //!< Reserved for internal usage (125).
  BL_FONT_COVERAGE_GROUP_INTERNAL_USAGE_126,                       //!< Reserved for internal usage (126).
  BL_FONT_COVERAGE_GROUP_INTERNAL_USAGE_127,                       //!< Reserved for internal usage (127).

  //! Maximum value of `BLFontCoverageGroup`.
  BL_FONT_COVERAGE_GROUP_MAX_VALUE = BL_FONT_COVERAGE_GROUP_INTERNAL_USAGE_127

  BL_FORCE_ENUM_UINT32(BL_FONT_COVERAGE_GROUP)
};

//! Text direction.
BL_DEFINE_ENUM(BLTextDirection) {
  //! Left-to-right direction.
  BL_TEXT_DIRECTION_LTR = 0,
  //! Right-to-left direction.
  BL_TEXT_DIRECTION_RTL = 1,

  //! Maximum value of `BLTextDirection`.
  BL_TEXT_DIRECTION_MAX_VALUE = 1

  BL_FORCE_ENUM_UINT32(BL_TEXT_DIRECTION)
};

//! \}

//! \name BLFont Related Types
//! \{

//! Glyph id - a 32-bit unsigned integer.
typedef uint32_t BLGlyphId;

//! \}

//! \name BLFont Related Structs
//! \{

//! Contains additional information associated with a glyph used by \ref BLGlyphBuffer.
struct BLGlyphInfo {
  //! \name Members
  //! \{

  uint32_t cluster;
  uint32_t reserved;

  //! \}

#ifdef __cplusplus
  //! \name Common Functionality
  //! \{

  BL_INLINE_NODEBUG void reset() noexcept { *this = BLGlyphInfo{}; }

  //! \}
#endif
};

//! Glyph placement.
//!
//! Provides information about glyph offset (x/y) and advance (x/y).
struct BLGlyphPlacement {
  //! \name Members
  //! \{

  BLPointI placement;
  BLPointI advance;

  //! \}

#ifdef __cplusplus
  //! \name Common Functionality
  //! \{

  BL_INLINE_NODEBUG void reset() noexcept { *this = BLGlyphPlacement{}; }

  //! \}
#endif
};

//! Character to glyph mapping state.
struct BLGlyphMappingState {
  //! \name Members
  //! \{

  //! Number of glyphs or glyph-items on output.
  size_t glyph_count;
  //! Index of the first undefined glyph (SIZE_MAX if none).
  size_t undefined_first;
  //! Undefined glyph count (chars that have no mapping).
  size_t undefined_count;

  //! \}

#ifdef __cplusplus
  //! \name Common Functionality
  //! \{

  BL_INLINE_NODEBUG void reset() noexcept { *this = BLGlyphMappingState{}; }

  //! \}
#endif
};

//! Information passed to a \ref BLPathSinkFunc sink by \ref BLFont::get_glyph_outlines().
struct BLGlyphOutlineSinkInfo {
  size_t glyph_index;
  size_t contour_count;
};

//! Font coverage information struct provides information about ranges of unicode characters supported by a font-face.
//!
//! \remarks The information is stored in "OS/2" table in TTF/OTF font. Blend2D reads this information and populates
//! `BLFontCoverageInfo` struct from such data, however, there is no guarantee that what font describes is actually
//! true.
struct BLFontCoverageInfo {
  //! \name Members
  //! \{

  //! Unicode coverage data as a bit array of 4 32-bit unsigned integers.
  uint32_t data[4];

  //! \}

#ifdef __cplusplus
  BL_INLINE_NODEBUG void reset() noexcept { *this = BLFontCoverageInfo{}; }

  [[nodiscard]]
  BL_INLINE_NODEBUG bool is_empty() const noexcept {
    return (data[0] | data[1] | data[2] | data[3]) == 0;
  }

  [[nodiscard]]
  BL_INLINE_NODEBUG bool has_bit(uint32_t index) const noexcept {
    return ((data[index / 32u] >> (index % 32u)) & 0x1) != 0;
  }

  BL_INLINE_NODEBUG void set_bit(uint32_t index) noexcept {
    data[index / 32u] |= uint32_t(1) << (index % 32u);
  }

  BL_INLINE_NODEBUG void clear_bit(uint32_t index) noexcept {
    data[index / 32u] &= ~(uint32_t(1) << (index % 32u));
  }

  [[nodiscard]]
  BL_INLINE_NODEBUG bool equals(const BLFontCoverageInfo& other) const noexcept {
    return BLInternal::bool_and(bl_equals(data[0], other.data[0]),
                                bl_equals(data[1], other.data[1]),
                                bl_equals(data[2], other.data[2]),
                                bl_equals(data[3], other.data[3]));
  }

  [[nodiscard]]
  BL_INLINE_NODEBUG bool operator==(const BLFontCoverageInfo& other) const noexcept { return  equals(other); }

  [[nodiscard]]
  BL_INLINE_NODEBUG bool operator!=(const BLFontCoverageInfo& other) const noexcept { return !equals(other); }
#endif
};

//! Font PANOSE classification.
struct BLFontPanoseInfo {
  union {
    uint8_t data[10];
    uint8_t family_kind;

    struct {
      uint8_t family_kind;
      uint8_t serif_style;
      uint8_t weight;
      uint8_t proportion;
      uint8_t contrast;
      uint8_t stroke_variation;
      uint8_t arm_style;
      uint8_t letterform;
      uint8_t midline;
      uint8_t x_height;
    } text;

    struct {
      uint8_t family_kind;
      uint8_t tool_kind;
      uint8_t weight;
      uint8_t spacing;
      uint8_t aspect_ratio;
      uint8_t contrast;
      uint8_t topology;
      uint8_t form;
      uint8_t finials;
      uint8_t x_ascent;
    } script;

    struct {
      uint8_t family_kind;
      uint8_t decorative_class;
      uint8_t weight;
      uint8_t aspect;
      uint8_t contrast;
      uint8_t serif_variant;
      uint8_t treatment;
      uint8_t lining;
      uint8_t topology;
      uint8_t character_range;
    } decorative;

    struct {
      uint8_t family_kind;
      uint8_t symbol_kind;
      uint8_t weight;
      uint8_t spacing;
      uint8_t aspect_ratio_and_contrast;
      uint8_t aspect_ratio_94;
      uint8_t aspect_ratio_119;
      uint8_t aspect_ratio_157;
      uint8_t aspect_ratio_163;
      uint8_t aspect_ratio_211;
    } symbol;
  };

#ifdef __cplusplus
  BL_INLINE_NODEBUG void reset() noexcept { *this = BLFontPanoseInfo{}; }

  BL_INLINE_NODEBUG bool is_empty() const noexcept {
    return (data[0] | data[1] | data[2] | data[3] | data[4] |
            data[5] | data[6] | data[7] | data[8] | data[9] ) == 0;
  }
#endif
};

//! 2x2 transformation matrix used by \ref BLFont. It's similar to \ref BLMatrix2D,
//! however, it doesn't provide a translation part as it's assumed to be zero.
struct BLFontMatrix {
  union {
    double m[4];
    struct {
      double m00;
      double m01;
      double m10;
      double m11;
    };
  };

#ifdef __cplusplus
  BL_INLINE_NODEBUG BLFontMatrix() noexcept = default;
  BL_INLINE_NODEBUG BLFontMatrix(const BLFontMatrix& other) noexcept = default;

  BL_INLINE_NODEBUG BLFontMatrix(double m00, double m01, double m10, double m11) noexcept
    : m00(m00), m01(m01), m10(m10), m11(m11) {}

  BL_INLINE_NODEBUG BLFontMatrix& operator=(const BLFontMatrix& other) noexcept = default;

  BL_INLINE_NODEBUG void reset() noexcept {
    m00 = 1.0;
    m01 = 0.0;
    m10 = 0.0;
    m11 = 1.0;
  }

  BL_INLINE_NODEBUG void reset(double m00_value, double m01_value, double m10_value, double m11_value) noexcept {
    m00 = m00_value;
    m01 = m01_value;
    m10 = m10_value;
    m11 = m11_value;
  }
#endif
};

//! Scaled \ref BLFontDesignMetrics based on font size and other properties.
struct BLFontMetrics {
  //! Font size.
  float size;

  union {
    struct {
      //! Font ascent (horizontal orientation).
      float ascent;
      //! Font ascent (vertical orientation).
      float v_ascent;
      //! Font descent (horizontal orientation).
      float descent;
      //! Font descent (vertical orientation).
      float v_descent;
    };

    struct {
      float ascent_by_orientation[2];
      float descent_by_orientation[2];
    };
  };

  //! Line gap.
  float line_gap;
  //! Distance between the baseline and the mean line of lower-case letters.
  float x_height;
  //! Maximum height of a capital letter above the baseline.
  float cap_height;

  //! Minimum x, reported by the font.
  float x_min;
  //! Minimum y, reported by the font.
  float y_min;
  //! Maximum x, reported by the font.
  float x_max;
  //! Maximum y, reported by the font.
  float y_max;

  //! Text underline position.
  float underline_position;
  //! Text underline thickness.
  float underline_thickness;
  //! Text strikethrough position.
  float strikethrough_position;
  //! Text strikethrough thickness.
  float strikethrough_thickness;

#ifdef __cplusplus
  BL_INLINE_NODEBUG void reset() noexcept { *this = BLFontMetrics{}; }
#endif
};

//! Design metrics of a font.
//!
//! Design metrics is information that \ref BLFontFace collected directly from the font data. It means that all
//! fields are measured in font design units.
//!
//! When a new \ref BLFont instance is created a scaled metrics \ref BLFontMetrics is automatically calculated
//! from \ref BLFontDesignMetrics including other members like transformation, etc...
struct BLFontDesignMetrics {
  //! Units per EM square.
  int units_per_em;
  //! Lowest readable size in pixels.
  int lowest_ppem;
  //! Line gap.
  int line_gap;
  //! Distance between the baseline and the mean line of lower-case letters.
  int x_height;
  //! Maximum height of a capital letter above the baseline.
  int cap_height;

  union {
    struct {
      //! Ascent (horizontal layout).
      int ascent;
      //! Ascent (vertical layout).
      int v_ascent;
      //! Descent (horizontal layout).
      int descent;
      //! Descent (vertical layout).
      int v_descent;
      //! Minimum leading-side bearing (horizontal layout).
      int h_min_lsb;
      //! Minimum leading-side bearing (vertical layout).
      int v_min_lsb;
      //! Minimum trailing-side bearing (horizontal layout).
      int h_min_tsb;
      //! Minimum trailing-side bearing (vertical layout).
      int v_min_tsb;
      //! Maximum advance (horizontal layout).
      int h_max_advance;
      //! Maximum advance (vertical layout).
      int v_max_advance;
    };

    struct {
      //! Horizontal & vertical ascents.
      int ascent_by_orientation[2];
      //! Horizontal & vertical descents.
      int descent_by_orientation[2];
      //! Minimum leading-side bearing (horizontal and vertical).
      int min_lsb_by_orientation[2];
      //! Minimum trailing-side bearing (horizontal and vertical)..
      int min_tsb_by_orientation[2];
      //! Maximum advance width (horizontal) and height (vertical).
      int max_advance_by_orientation[2];
    };
  };

  //! Aggregated bounding box of all glyphs in the font.
  //!
  //! \note This value is reported by the font data so it's not granted to be true.
  BLBoxI glyph_bounding_box;

  //! Text underline position.
  int underline_position;
  //! Text underline thickness.
  int underline_thickness;
  //! Text strikethrough position.
  int strikethrough_position;
  //! Text strikethrough thickness.
  int strikethrough_thickness;

#ifdef __cplusplus
  BL_INLINE_NODEBUG void reset() noexcept { *this = BLFontDesignMetrics{}; }
#endif
};

//! Text metrics.
struct BLTextMetrics {
  BLPoint advance;
  BLPoint leading_bearing;
  BLPoint trailing_bearing;
  BLBox bounding_box;

#ifdef __cplusplus
  BL_INLINE_NODEBUG void reset() noexcept { *this = BLTextMetrics{}; }
#endif
};

//! \}

//! \}

#endif // BLEND2D_FONTDEFS_H_INCLUDED
