//    Copyright (C) 2018-2023 Jakub Melka
//
//    This file is part of PDF4QT.
//
//    PDF4QT is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    with the written consent of the copyright owner, any later version.
//
//    PDF4QT is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public License
//    along with PDF4QT.  If not, see <https://www.gnu.org/licenses/>.

#include "pdfencoding.h"
#include "pdfdbgheap.h"

#include <QTimeZone>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringDecoder>
#else
#include <QTextCodec>
#endif

#include <cctype>
#include <cstring>

namespace pdf
{

namespace encoding
{

// PDF Reference 1.7, Appendix D, Section D.1, StandardEncoding
static const EncodingTable STANDARD_ENCODING_CONVERSION_TABLE = {
    QChar(0xfffd),  // Hex No. 00 (Dec 000) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 01 (Dec 001) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 02 (Dec 002) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 03 (Dec 003) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 04 (Dec 004) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 05 (Dec 005) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 06 (Dec 006) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 07 (Dec 007) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 08 (Dec 008) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 09 (Dec 009) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 0A (Dec 010) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 0B (Dec 011) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 0C (Dec 012) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 0D (Dec 013) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 0E (Dec 014) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 0F (Dec 015) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 10 (Dec 016) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 11 (Dec 017) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 12 (Dec 018) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 13 (Dec 019) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 14 (Dec 020) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 15 (Dec 021) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 16 (Dec 022) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 17 (Dec 023) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 18 (Dec 024) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 19 (Dec 025) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 1A (Dec 026) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 1B (Dec 027) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 1C (Dec 028) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 1D (Dec 029) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 1E (Dec 030) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 1F (Dec 031) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0x0020),  // Hex No. 20 (Dec 032) Character ' ' Whitespace
    QChar(0x0021),  // Hex No. 21 (Dec 033) Character '!' Punctuation
    QChar(0x0022),  // Hex No. 22 (Dec 034) Character '"' Punctuation
    QChar(0x0023),  // Hex No. 23 (Dec 035) Character '#' Punctuation
    QChar(0x0024),  // Hex No. 24 (Dec 036) Character '$' Symbol
    QChar(0x0025),  // Hex No. 25 (Dec 037) Character '%' Punctuation
    QChar(0x0026),  // Hex No. 26 (Dec 038) Character '&' Punctuation
    QChar(0x2019),  // Hex No. 27 (Dec 039) Character '’' Punctuation
    QChar(0x0028),  // Hex No. 28 (Dec 040) Character '(' Punctuation
    QChar(0x0029),  // Hex No. 29 (Dec 041) Character ')' Punctuation
    QChar(0x002a),  // Hex No. 2A (Dec 042) Character '*' Punctuation
    QChar(0x002b),  // Hex No. 2B (Dec 043) Character '+' Symbol
    QChar(0x002c),  // Hex No. 2C (Dec 044) Character ',' Punctuation
    QChar(0x002d),  // Hex No. 2D (Dec 045) Character '-' Punctuation
    QChar(0x002e),  // Hex No. 2E (Dec 046) Character '.' Punctuation
    QChar(0x002f),  // Hex No. 2F (Dec 047) Character '/' Punctuation
    QChar(0x0030),  // Hex No. 30 (Dec 048) Character '0' Digit
    QChar(0x0031),  // Hex No. 31 (Dec 049) Character '1' Digit
    QChar(0x0032),  // Hex No. 32 (Dec 050) Character '2' Digit
    QChar(0x0033),  // Hex No. 33 (Dec 051) Character '3' Digit
    QChar(0x0034),  // Hex No. 34 (Dec 052) Character '4' Digit
    QChar(0x0035),  // Hex No. 35 (Dec 053) Character '5' Digit
    QChar(0x0036),  // Hex No. 36 (Dec 054) Character '6' Digit
    QChar(0x0037),  // Hex No. 37 (Dec 055) Character '7' Digit
    QChar(0x0038),  // Hex No. 38 (Dec 056) Character '8' Digit
    QChar(0x0039),  // Hex No. 39 (Dec 057) Character '9' Digit
    QChar(0x003a),  // Hex No. 3A (Dec 058) Character ':' Punctuation
    QChar(0x003b),  // Hex No. 3B (Dec 059) Character ';' Punctuation
    QChar(0x003c),  // Hex No. 3C (Dec 060) Character '<' Symbol
    QChar(0x003d),  // Hex No. 3D (Dec 061) Character '=' Symbol
    QChar(0x003e),  // Hex No. 3E (Dec 062) Character '>' Symbol
    QChar(0x003f),  // Hex No. 3F (Dec 063) Character '?' Punctuation
    QChar(0x0040),  // Hex No. 40 (Dec 064) Character '@' Punctuation
    QChar(0x0041),  // Hex No. 41 (Dec 065) Character 'A' Letter, Uppercase
    QChar(0x0042),  // Hex No. 42 (Dec 066) Character 'B' Letter, Uppercase
    QChar(0x0043),  // Hex No. 43 (Dec 067) Character 'C' Letter, Uppercase
    QChar(0x0044),  // Hex No. 44 (Dec 068) Character 'D' Letter, Uppercase
    QChar(0x0045),  // Hex No. 45 (Dec 069) Character 'E' Letter, Uppercase
    QChar(0x0046),  // Hex No. 46 (Dec 070) Character 'F' Letter, Uppercase
    QChar(0x0047),  // Hex No. 47 (Dec 071) Character 'G' Letter, Uppercase
    QChar(0x0048),  // Hex No. 48 (Dec 072) Character 'H' Letter, Uppercase
    QChar(0x0049),  // Hex No. 49 (Dec 073) Character 'I' Letter, Uppercase
    QChar(0x004a),  // Hex No. 4A (Dec 074) Character 'J' Letter, Uppercase
    QChar(0x004b),  // Hex No. 4B (Dec 075) Character 'K' Letter, Uppercase
    QChar(0x004c),  // Hex No. 4C (Dec 076) Character 'L' Letter, Uppercase
    QChar(0x004d),  // Hex No. 4D (Dec 077) Character 'M' Letter, Uppercase
    QChar(0x004e),  // Hex No. 4E (Dec 078) Character 'N' Letter, Uppercase
    QChar(0x004f),  // Hex No. 4F (Dec 079) Character 'O' Letter, Uppercase
    QChar(0x0050),  // Hex No. 50 (Dec 080) Character 'P' Letter, Uppercase
    QChar(0x0051),  // Hex No. 51 (Dec 081) Character 'Q' Letter, Uppercase
    QChar(0x0052),  // Hex No. 52 (Dec 082) Character 'R' Letter, Uppercase
    QChar(0x0053),  // Hex No. 53 (Dec 083) Character 'S' Letter, Uppercase
    QChar(0x0054),  // Hex No. 54 (Dec 084) Character 'T' Letter, Uppercase
    QChar(0x0055),  // Hex No. 55 (Dec 085) Character 'U' Letter, Uppercase
    QChar(0x0056),  // Hex No. 56 (Dec 086) Character 'V' Letter, Uppercase
    QChar(0x0057),  // Hex No. 57 (Dec 087) Character 'W' Letter, Uppercase
    QChar(0x0058),  // Hex No. 58 (Dec 088) Character 'X' Letter, Uppercase
    QChar(0x0059),  // Hex No. 59 (Dec 089) Character 'Y' Letter, Uppercase
    QChar(0x005a),  // Hex No. 5A (Dec 090) Character 'Z' Letter, Uppercase
    QChar(0x005b),  // Hex No. 5B (Dec 091) Character '[' Punctuation
    QChar(0x005c),  // Hex No. 5C (Dec 092) Character '\' Punctuation
    QChar(0x005d),  // Hex No. 5D (Dec 093) Character ']' Punctuation
    QChar(0x005e),  // Hex No. 5E (Dec 094) Character '^' Symbol
    QChar(0x005f),  // Hex No. 5F (Dec 095) Character '_' Punctuation
    QChar(0x2018),  // Hex No. 60 (Dec 096) Character '‘' Punctuation
    QChar(0x0061),  // Hex No. 61 (Dec 097) Character 'a' Letter, Lowercase
    QChar(0x0062),  // Hex No. 62 (Dec 098) Character 'b' Letter, Lowercase
    QChar(0x0063),  // Hex No. 63 (Dec 099) Character 'c' Letter, Lowercase
    QChar(0x0064),  // Hex No. 64 (Dec 100) Character 'd' Letter, Lowercase
    QChar(0x0065),  // Hex No. 65 (Dec 101) Character 'e' Letter, Lowercase
    QChar(0x0066),  // Hex No. 66 (Dec 102) Character 'f' Letter, Lowercase
    QChar(0x0067),  // Hex No. 67 (Dec 103) Character 'g' Letter, Lowercase
    QChar(0x0068),  // Hex No. 68 (Dec 104) Character 'h' Letter, Lowercase
    QChar(0x0069),  // Hex No. 69 (Dec 105) Character 'i' Letter, Lowercase
    QChar(0x006a),  // Hex No. 6A (Dec 106) Character 'j' Letter, Lowercase
    QChar(0x006b),  // Hex No. 6B (Dec 107) Character 'k' Letter, Lowercase
    QChar(0x006c),  // Hex No. 6C (Dec 108) Character 'l' Letter, Lowercase
    QChar(0x006d),  // Hex No. 6D (Dec 109) Character 'm' Letter, Lowercase
    QChar(0x006e),  // Hex No. 6E (Dec 110) Character 'n' Letter, Lowercase
    QChar(0x006f),  // Hex No. 6F (Dec 111) Character 'o' Letter, Lowercase
    QChar(0x0070),  // Hex No. 70 (Dec 112) Character 'p' Letter, Lowercase
    QChar(0x0071),  // Hex No. 71 (Dec 113) Character 'q' Letter, Lowercase
    QChar(0x0072),  // Hex No. 72 (Dec 114) Character 'r' Letter, Lowercase
    QChar(0x0073),  // Hex No. 73 (Dec 115) Character 's' Letter, Lowercase
    QChar(0x0074),  // Hex No. 74 (Dec 116) Character 't' Letter, Lowercase
    QChar(0x0075),  // Hex No. 75 (Dec 117) Character 'u' Letter, Lowercase
    QChar(0x0076),  // Hex No. 76 (Dec 118) Character 'v' Letter, Lowercase
    QChar(0x0077),  // Hex No. 77 (Dec 119) Character 'w' Letter, Lowercase
    QChar(0x0078),  // Hex No. 78 (Dec 120) Character 'x' Letter, Lowercase
    QChar(0x0079),  // Hex No. 79 (Dec 121) Character 'y' Letter, Lowercase
    QChar(0x007a),  // Hex No. 7A (Dec 122) Character 'z' Letter, Lowercase
    QChar(0x007b),  // Hex No. 7B (Dec 123) Character '{' Punctuation
    QChar(0x007c),  // Hex No. 7C (Dec 124) Character '|' Symbol
    QChar(0x007d),  // Hex No. 7D (Dec 125) Character '}' Punctuation
    QChar(0x007e),  // Hex No. 7E (Dec 126) Character '~' Symbol
    QChar(0xfffd),  // Hex No. 7F (Dec 127) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 80 (Dec 128) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 81 (Dec 129) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 82 (Dec 130) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 83 (Dec 131) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 84 (Dec 132) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 85 (Dec 133) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 86 (Dec 134) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 87 (Dec 135) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 88 (Dec 136) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 89 (Dec 137) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 8A (Dec 138) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 8B (Dec 139) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 8C (Dec 140) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 8D (Dec 141) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 8E (Dec 142) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 8F (Dec 143) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 90 (Dec 144) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 91 (Dec 145) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 92 (Dec 146) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 93 (Dec 147) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 94 (Dec 148) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 95 (Dec 149) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 96 (Dec 150) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 97 (Dec 151) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 98 (Dec 152) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 99 (Dec 153) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 9A (Dec 154) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 9B (Dec 155) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 9C (Dec 156) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 9D (Dec 157) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 9E (Dec 158) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 9F (Dec 159) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. A0 (Dec 160) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0x00a1),  // Hex No. A1 (Dec 161) Character '¡' Punctuation
    QChar(0x00a2),  // Hex No. A2 (Dec 162) Character '¢' Symbol
    QChar(0x00a3),  // Hex No. A3 (Dec 163) Character '£' Symbol
    QChar(0x2044),  // Hex No. A4 (Dec 164) Character '⁄' Symbol
    QChar(0x00a5),  // Hex No. A5 (Dec 165) Character '¥' Symbol
    QChar(0x0192),  // Hex No. A6 (Dec 166) Character 'ƒ' Letter, Lowercase
    QChar(0x00a7),  // Hex No. A7 (Dec 167) Character '§' Punctuation
    QChar(0x00a4),  // Hex No. A8 (Dec 168) Character '¤' Symbol
    QChar(0x0027),  // Hex No. A9 (Dec 169) Character ''' Punctuation
    QChar(0x201c),  // Hex No. AA (Dec 170) Character '“' Punctuation
    QChar(0x00ab),  // Hex No. AB (Dec 171) Character '«' Punctuation
    QChar(0x2039),  // Hex No. AC (Dec 172) Character '‹' Punctuation
    QChar(0x203a),  // Hex No. AD (Dec 173) Character '›' Punctuation
    QChar(0xfb01),  // Hex No. AE (Dec 174) Character 'ﬁ' Letter, Lowercase
    QChar(0xfb02),  // Hex No. AF (Dec 175) Character 'ﬂ' Letter, Lowercase
    QChar(0xfffd),  // Hex No. B0 (Dec 176) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0x2013),  // Hex No. B1 (Dec 177) Character '–' Punctuation
    QChar(0x2020),  // Hex No. B2 (Dec 178) Character '†' Punctuation
    QChar(0x2021),  // Hex No. B3 (Dec 179) Character '‡' Punctuation
    QChar(0x00b7),  // Hex No. B4 (Dec 180) Character '·' Punctuation
    QChar(0xfffd),  // Hex No. B5 (Dec 181) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0x00b6),  // Hex No. B6 (Dec 182) Character '¶' Punctuation
    QChar(0x2022),  // Hex No. B7 (Dec 183) Character '•' Punctuation
    QChar(0x201a),  // Hex No. B8 (Dec 184) Character '‚' Punctuation
    QChar(0x201e),  // Hex No. B9 (Dec 185) Character '„' Punctuation
    QChar(0x201d),  // Hex No. BA (Dec 186) Character '”' Punctuation
    QChar(0x00bb),  // Hex No. BB (Dec 187) Character '»' Punctuation
    QChar(0x2026),  // Hex No. BC (Dec 188) Character '…' Punctuation
    QChar(0x2030),  // Hex No. BD (Dec 189) Character '‰' Punctuation
    QChar(0xfffd),  // Hex No. BE (Dec 190) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0x00bf),  // Hex No. BF (Dec 191) Character '¿' Punctuation
    QChar(0xfffd),  // Hex No. C0 (Dec 192) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0x0060),  // Hex No. C1 (Dec 193) Character '`' Symbol
    QChar(0x00b4),  // Hex No. C2 (Dec 194) Character '´' Symbol
    QChar(0x02c6),  // Hex No. C3 (Dec 195) Character 'ˆ' Letter
    QChar(0x02dc),  // Hex No. C4 (Dec 196) Character '˜' Symbol
    QChar(0x00af),  // Hex No. C5 (Dec 197) Character '¯' Symbol
    QChar(0x02d8),  // Hex No. C6 (Dec 198) Character '˘' Symbol
    QChar(0x02d9),  // Hex No. C7 (Dec 199) Character '˙' Symbol
    QChar(0x00a8),  // Hex No. C8 (Dec 200) Character '¨' Symbol
    QChar(0xfffd),  // Hex No. C9 (Dec 201) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0x02da),  // Hex No. CA (Dec 202) Character '˚' Symbol
    QChar(0x00b8),  // Hex No. CB (Dec 203) Character '¸' Symbol
    QChar(0xfffd),  // Hex No. CC (Dec 204) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0x02dd),  // Hex No. CD (Dec 205) Character '˝' Symbol
    QChar(0x02db),  // Hex No. CE (Dec 206) Character '˛' Symbol
    QChar(0x02c7),  // Hex No. CF (Dec 207) Character 'ˇ' Letter
    QChar(0x2014),  // Hex No. D0 (Dec 208) Character '—' Punctuation
    QChar(0xfffd),  // Hex No. D1 (Dec 209) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. D2 (Dec 210) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. D3 (Dec 211) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. D4 (Dec 212) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. D5 (Dec 213) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. D6 (Dec 214) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. D7 (Dec 215) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. D8 (Dec 216) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. D9 (Dec 217) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. DA (Dec 218) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. DB (Dec 219) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. DC (Dec 220) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. DD (Dec 221) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. DE (Dec 222) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. DF (Dec 223) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. E0 (Dec 224) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0x00c6),  // Hex No. E1 (Dec 225) Character 'Æ' Letter, Uppercase
    QChar(0xfffd),  // Hex No. E2 (Dec 226) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0x00aa),  // Hex No. E3 (Dec 227) Character 'ª' Letter
    QChar(0xfffd),  // Hex No. E4 (Dec 228) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. E5 (Dec 229) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. E6 (Dec 230) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. E7 (Dec 231) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0x0141),  // Hex No. E8 (Dec 232) Character 'Ł' Letter, Uppercase
    QChar(0x00d8),  // Hex No. E9 (Dec 233) Character 'Ø' Letter, Uppercase
    QChar(0x0152),  // Hex No. EA (Dec 234) Character 'Œ' Letter, Uppercase
    QChar(0x00ba),  // Hex No. EB (Dec 235) Character 'º' Letter
    QChar(0xfffd),  // Hex No. EC (Dec 236) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. ED (Dec 237) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. EE (Dec 238) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. EF (Dec 239) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. F0 (Dec 240) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0x00e6),  // Hex No. F1 (Dec 241) Character 'æ' Letter, Lowercase
    QChar(0xfffd),  // Hex No. F2 (Dec 242) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. F3 (Dec 243) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. F4 (Dec 244) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0x0131),  // Hex No. F5 (Dec 245) Character 'ı' Letter, Lowercase
    QChar(0xfffd),  // Hex No. F6 (Dec 246) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. F7 (Dec 247) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0x0142),  // Hex No. F8 (Dec 248) Character 'ł' Letter, Lowercase
    QChar(0x00f8),  // Hex No. F9 (Dec 249) Character 'ø' Letter, Lowercase
    QChar(0x0153),  // Hex No. FA (Dec 250) Character 'œ' Letter, Lowercase
    QChar(0x00df),  // Hex No. FB (Dec 251) Character 'ß' Letter, Lowercase
    QChar(0xfffd),  // Hex No. FC (Dec 252) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. FD (Dec 253) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. FE (Dec 254) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. FF (Dec 255) REPLACEMENT CHARACTER 0xFFFD - not present in character set
};

// PDF Reference 1.7, Appendix D, Section D.1, MacRomanEncoding
static const EncodingTable MAC_ROMAN_ENCODING_CONVERSION_TABLE = {
    QChar(0xfffd),  // Hex No. 00 (Dec 000) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 01 (Dec 001) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 02 (Dec 002) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 03 (Dec 003) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 04 (Dec 004) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 05 (Dec 005) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 06 (Dec 006) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 07 (Dec 007) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 08 (Dec 008) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 09 (Dec 009) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 0A (Dec 010) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 0B (Dec 011) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 0C (Dec 012) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 0D (Dec 013) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 0E (Dec 014) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 0F (Dec 015) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 10 (Dec 016) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 11 (Dec 017) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 12 (Dec 018) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 13 (Dec 019) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 14 (Dec 020) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 15 (Dec 021) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 16 (Dec 022) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 17 (Dec 023) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 18 (Dec 024) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 19 (Dec 025) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 1A (Dec 026) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 1B (Dec 027) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 1C (Dec 028) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 1D (Dec 029) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 1E (Dec 030) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 1F (Dec 031) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0x0020),  // Hex No. 20 (Dec 032) Character ' ' Whitespace
    QChar(0x0021),  // Hex No. 21 (Dec 033) Character '!' Punctuation
    QChar(0x0022),  // Hex No. 22 (Dec 034) Character '"' Punctuation
    QChar(0x0023),  // Hex No. 23 (Dec 035) Character '#' Punctuation
    QChar(0x0024),  // Hex No. 24 (Dec 036) Character '$' Symbol
    QChar(0x0025),  // Hex No. 25 (Dec 037) Character '%' Punctuation
    QChar(0x0026),  // Hex No. 26 (Dec 038) Character '&' Punctuation
    QChar(0x0027),  // Hex No. 27 (Dec 039) Character ''' Punctuation
    QChar(0x0028),  // Hex No. 28 (Dec 040) Character '(' Punctuation
    QChar(0x0029),  // Hex No. 29 (Dec 041) Character ')' Punctuation
    QChar(0x002a),  // Hex No. 2A (Dec 042) Character '*' Punctuation
    QChar(0x002b),  // Hex No. 2B (Dec 043) Character '+' Symbol
    QChar(0x002c),  // Hex No. 2C (Dec 044) Character ',' Punctuation
    QChar(0x002d),  // Hex No. 2D (Dec 045) Character '-' Punctuation
    QChar(0x002e),  // Hex No. 2E (Dec 046) Character '.' Punctuation
    QChar(0x002f),  // Hex No. 2F (Dec 047) Character '/' Punctuation
    QChar(0x0030),  // Hex No. 30 (Dec 048) Character '0' Digit
    QChar(0x0031),  // Hex No. 31 (Dec 049) Character '1' Digit
    QChar(0x0032),  // Hex No. 32 (Dec 050) Character '2' Digit
    QChar(0x0033),  // Hex No. 33 (Dec 051) Character '3' Digit
    QChar(0x0034),  // Hex No. 34 (Dec 052) Character '4' Digit
    QChar(0x0035),  // Hex No. 35 (Dec 053) Character '5' Digit
    QChar(0x0036),  // Hex No. 36 (Dec 054) Character '6' Digit
    QChar(0x0037),  // Hex No. 37 (Dec 055) Character '7' Digit
    QChar(0x0038),  // Hex No. 38 (Dec 056) Character '8' Digit
    QChar(0x0039),  // Hex No. 39 (Dec 057) Character '9' Digit
    QChar(0x003a),  // Hex No. 3A (Dec 058) Character ':' Punctuation
    QChar(0x003b),  // Hex No. 3B (Dec 059) Character ';' Punctuation
    QChar(0x003c),  // Hex No. 3C (Dec 060) Character '<' Symbol
    QChar(0x003d),  // Hex No. 3D (Dec 061) Character '=' Symbol
    QChar(0x003e),  // Hex No. 3E (Dec 062) Character '>' Symbol
    QChar(0x003f),  // Hex No. 3F (Dec 063) Character '?' Punctuation
    QChar(0x0040),  // Hex No. 40 (Dec 064) Character '@' Punctuation
    QChar(0x0041),  // Hex No. 41 (Dec 065) Character 'A' Letter, Uppercase
    QChar(0x0042),  // Hex No. 42 (Dec 066) Character 'B' Letter, Uppercase
    QChar(0x0043),  // Hex No. 43 (Dec 067) Character 'C' Letter, Uppercase
    QChar(0x0044),  // Hex No. 44 (Dec 068) Character 'D' Letter, Uppercase
    QChar(0x0045),  // Hex No. 45 (Dec 069) Character 'E' Letter, Uppercase
    QChar(0x0046),  // Hex No. 46 (Dec 070) Character 'F' Letter, Uppercase
    QChar(0x0047),  // Hex No. 47 (Dec 071) Character 'G' Letter, Uppercase
    QChar(0x0048),  // Hex No. 48 (Dec 072) Character 'H' Letter, Uppercase
    QChar(0x0049),  // Hex No. 49 (Dec 073) Character 'I' Letter, Uppercase
    QChar(0x004a),  // Hex No. 4A (Dec 074) Character 'J' Letter, Uppercase
    QChar(0x004b),  // Hex No. 4B (Dec 075) Character 'K' Letter, Uppercase
    QChar(0x004c),  // Hex No. 4C (Dec 076) Character 'L' Letter, Uppercase
    QChar(0x004d),  // Hex No. 4D (Dec 077) Character 'M' Letter, Uppercase
    QChar(0x004e),  // Hex No. 4E (Dec 078) Character 'N' Letter, Uppercase
    QChar(0x004f),  // Hex No. 4F (Dec 079) Character 'O' Letter, Uppercase
    QChar(0x0050),  // Hex No. 50 (Dec 080) Character 'P' Letter, Uppercase
    QChar(0x0051),  // Hex No. 51 (Dec 081) Character 'Q' Letter, Uppercase
    QChar(0x0052),  // Hex No. 52 (Dec 082) Character 'R' Letter, Uppercase
    QChar(0x0053),  // Hex No. 53 (Dec 083) Character 'S' Letter, Uppercase
    QChar(0x0054),  // Hex No. 54 (Dec 084) Character 'T' Letter, Uppercase
    QChar(0x0055),  // Hex No. 55 (Dec 085) Character 'U' Letter, Uppercase
    QChar(0x0056),  // Hex No. 56 (Dec 086) Character 'V' Letter, Uppercase
    QChar(0x0057),  // Hex No. 57 (Dec 087) Character 'W' Letter, Uppercase
    QChar(0x0058),  // Hex No. 58 (Dec 088) Character 'X' Letter, Uppercase
    QChar(0x0059),  // Hex No. 59 (Dec 089) Character 'Y' Letter, Uppercase
    QChar(0x005a),  // Hex No. 5A (Dec 090) Character 'Z' Letter, Uppercase
    QChar(0x005b),  // Hex No. 5B (Dec 091) Character '[' Punctuation
    QChar(0x005c),  // Hex No. 5C (Dec 092) Character '\' Punctuation
    QChar(0x005d),  // Hex No. 5D (Dec 093) Character ']' Punctuation
    QChar(0x005e),  // Hex No. 5E (Dec 094) Character '^' Symbol
    QChar(0x005f),  // Hex No. 5F (Dec 095) Character '_' Punctuation
    QChar(0x0060),  // Hex No. 60 (Dec 096) Character '`' Symbol
    QChar(0x0061),  // Hex No. 61 (Dec 097) Character 'a' Letter, Lowercase
    QChar(0x0062),  // Hex No. 62 (Dec 098) Character 'b' Letter, Lowercase
    QChar(0x0063),  // Hex No. 63 (Dec 099) Character 'c' Letter, Lowercase
    QChar(0x0064),  // Hex No. 64 (Dec 100) Character 'd' Letter, Lowercase
    QChar(0x0065),  // Hex No. 65 (Dec 101) Character 'e' Letter, Lowercase
    QChar(0x0066),  // Hex No. 66 (Dec 102) Character 'f' Letter, Lowercase
    QChar(0x0067),  // Hex No. 67 (Dec 103) Character 'g' Letter, Lowercase
    QChar(0x0068),  // Hex No. 68 (Dec 104) Character 'h' Letter, Lowercase
    QChar(0x0069),  // Hex No. 69 (Dec 105) Character 'i' Letter, Lowercase
    QChar(0x006a),  // Hex No. 6A (Dec 106) Character 'j' Letter, Lowercase
    QChar(0x006b),  // Hex No. 6B (Dec 107) Character 'k' Letter, Lowercase
    QChar(0x006c),  // Hex No. 6C (Dec 108) Character 'l' Letter, Lowercase
    QChar(0x006d),  // Hex No. 6D (Dec 109) Character 'm' Letter, Lowercase
    QChar(0x006e),  // Hex No. 6E (Dec 110) Character 'n' Letter, Lowercase
    QChar(0x006f),  // Hex No. 6F (Dec 111) Character 'o' Letter, Lowercase
    QChar(0x0070),  // Hex No. 70 (Dec 112) Character 'p' Letter, Lowercase
    QChar(0x0071),  // Hex No. 71 (Dec 113) Character 'q' Letter, Lowercase
    QChar(0x0072),  // Hex No. 72 (Dec 114) Character 'r' Letter, Lowercase
    QChar(0x0073),  // Hex No. 73 (Dec 115) Character 's' Letter, Lowercase
    QChar(0x0074),  // Hex No. 74 (Dec 116) Character 't' Letter, Lowercase
    QChar(0x0075),  // Hex No. 75 (Dec 117) Character 'u' Letter, Lowercase
    QChar(0x0076),  // Hex No. 76 (Dec 118) Character 'v' Letter, Lowercase
    QChar(0x0077),  // Hex No. 77 (Dec 119) Character 'w' Letter, Lowercase
    QChar(0x0078),  // Hex No. 78 (Dec 120) Character 'x' Letter, Lowercase
    QChar(0x0079),  // Hex No. 79 (Dec 121) Character 'y' Letter, Lowercase
    QChar(0x007a),  // Hex No. 7A (Dec 122) Character 'z' Letter, Lowercase
    QChar(0x007b),  // Hex No. 7B (Dec 123) Character '{' Punctuation
    QChar(0x007c),  // Hex No. 7C (Dec 124) Character '|' Symbol
    QChar(0x007d),  // Hex No. 7D (Dec 125) Character '}' Punctuation
    QChar(0x007e),  // Hex No. 7E (Dec 126) Character '~' Symbol
    QChar(0xfffd),  // Hex No. 7F (Dec 127) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0x00c4),  // Hex No. 80 (Dec 128) Character 'Ä' Letter, Uppercase
    QChar(0x00c5),  // Hex No. 81 (Dec 129) Character 'Å' Letter, Uppercase
    QChar(0x00c7),  // Hex No. 82 (Dec 130) Character 'Ç' Letter, Uppercase
    QChar(0x00c9),  // Hex No. 83 (Dec 131) Character 'É' Letter, Uppercase
    QChar(0x00d1),  // Hex No. 84 (Dec 132) Character 'Ñ' Letter, Uppercase
    QChar(0x00d6),  // Hex No. 85 (Dec 133) Character 'Ö' Letter, Uppercase
    QChar(0x00dc),  // Hex No. 86 (Dec 134) Character 'Ü' Letter, Uppercase
    QChar(0x00e1),  // Hex No. 87 (Dec 135) Character 'á' Letter, Lowercase
    QChar(0x00e0),  // Hex No. 88 (Dec 136) Character 'à' Letter, Lowercase
    QChar(0x00e2),  // Hex No. 89 (Dec 137) Character 'â' Letter, Lowercase
    QChar(0x00e4),  // Hex No. 8A (Dec 138) Character 'ä' Letter, Lowercase
    QChar(0x00e3),  // Hex No. 8B (Dec 139) Character 'ã' Letter, Lowercase
    QChar(0x00e5),  // Hex No. 8C (Dec 140) Character 'å' Letter, Lowercase
    QChar(0x00e7),  // Hex No. 8D (Dec 141) Character 'ç' Letter, Lowercase
    QChar(0x00e9),  // Hex No. 8E (Dec 142) Character 'é' Letter, Lowercase
    QChar(0x00e8),  // Hex No. 8F (Dec 143) Character 'è' Letter, Lowercase
    QChar(0x00ea),  // Hex No. 90 (Dec 144) Character 'ê' Letter, Lowercase
    QChar(0x00eb),  // Hex No. 91 (Dec 145) Character 'ë' Letter, Lowercase
    QChar(0x00ed),  // Hex No. 92 (Dec 146) Character 'í' Letter, Lowercase
    QChar(0x00ec),  // Hex No. 93 (Dec 147) Character 'ì' Letter, Lowercase
    QChar(0x00ee),  // Hex No. 94 (Dec 148) Character 'î' Letter, Lowercase
    QChar(0x00ef),  // Hex No. 95 (Dec 149) Character 'ï' Letter, Lowercase
    QChar(0x00f1),  // Hex No. 96 (Dec 150) Character 'ñ' Letter, Lowercase
    QChar(0x00f3),  // Hex No. 97 (Dec 151) Character 'ó' Letter, Lowercase
    QChar(0x00f2),  // Hex No. 98 (Dec 152) Character 'ò' Letter, Lowercase
    QChar(0x00f4),  // Hex No. 99 (Dec 153) Character 'ô' Letter, Lowercase
    QChar(0x00f6),  // Hex No. 9A (Dec 154) Character 'ö' Letter, Lowercase
    QChar(0x00f5),  // Hex No. 9B (Dec 155) Character 'õ' Letter, Lowercase
    QChar(0x00fa),  // Hex No. 9C (Dec 156) Character 'ú' Letter, Lowercase
    QChar(0x00f9),  // Hex No. 9D (Dec 157) Character 'ù' Letter, Lowercase
    QChar(0x00fb),  // Hex No. 9E (Dec 158) Character 'û' Letter, Lowercase
    QChar(0x00fc),  // Hex No. 9F (Dec 159) Character 'ü' Letter, Lowercase
    QChar(0x2020),  // Hex No. A0 (Dec 160) Character '†' Punctuation
    QChar(0x00b0),  // Hex No. A1 (Dec 161) Character '°' Symbol
    QChar(0x00a2),  // Hex No. A2 (Dec 162) Character '¢' Symbol
    QChar(0x00a3),  // Hex No. A3 (Dec 163) Character '£' Symbol
    QChar(0x00a7),  // Hex No. A4 (Dec 164) Character '§' Punctuation
    QChar(0x2022),  // Hex No. A5 (Dec 165) Character '•' Punctuation
    QChar(0x00b6),  // Hex No. A6 (Dec 166) Character '¶' Punctuation
    QChar(0x00df),  // Hex No. A7 (Dec 167) Character 'ß' Letter, Lowercase
    QChar(0x00ae),  // Hex No. A8 (Dec 168) Character '®' Symbol
    QChar(0x00a9),  // Hex No. A9 (Dec 169) Character '©' Symbol
    QChar(0x2122),  // Hex No. AA (Dec 170) Character '™' Symbol
    QChar(0x00b4),  // Hex No. AB (Dec 171) Character '´' Symbol
    QChar(0x00a8),  // Hex No. AC (Dec 172) Character '¨' Symbol
    QChar(0x2260),  // Hex No. AD (Dec 173) Character '≠' Symbol
    QChar(0x00c6),  // Hex No. AE (Dec 174) Character 'Æ' Letter, Uppercase
    QChar(0x00d8),  // Hex No. AF (Dec 175) Character 'Ø' Letter, Uppercase
    QChar(0x221e),  // Hex No. B0 (Dec 176) Character '∞' Symbol
    QChar(0x00b1),  // Hex No. B1 (Dec 177) Character '±' Symbol
    QChar(0x2264),  // Hex No. B2 (Dec 178) Character '≤' Symbol
    QChar(0x2265),  // Hex No. B3 (Dec 179) Character '≥' Symbol
    QChar(0x00a5),  // Hex No. B4 (Dec 180) Character '¥' Symbol
    QChar(0x00b5),  // Hex No. B5 (Dec 181) Character 'µ' Letter, Lowercase
    QChar(0x2202),  // Hex No. B6 (Dec 182) Character '∂' Symbol
    QChar(0x2211),  // Hex No. B7 (Dec 183) Character '∑' Symbol
    QChar(0x220f),  // Hex No. B8 (Dec 184) Character '∏' Symbol
    QChar(0x03c0),  // Hex No. B9 (Dec 185) Character 'π' Letter, Lowercase
    QChar(0x222b),  // Hex No. BA (Dec 186) Character '∫' Symbol
    QChar(0x00aa),  // Hex No. BB (Dec 187) Character 'ª' Letter
    QChar(0x00ba),  // Hex No. BC (Dec 188) Character 'º' Letter
    QChar(0x2126),  // Hex No. BD (Dec 189) Character 'Ω' Letter, Uppercase
    QChar(0x00e6),  // Hex No. BE (Dec 190) Character 'æ' Letter, Lowercase
    QChar(0x00f8),  // Hex No. BF (Dec 191) Character 'ø' Letter, Lowercase
    QChar(0x00bf),  // Hex No. C0 (Dec 192) Character '¿' Punctuation
    QChar(0x00a1),  // Hex No. C1 (Dec 193) Character '¡' Punctuation
    QChar(0x00ac),  // Hex No. C2 (Dec 194) Character '¬' Symbol
    QChar(0x221a),  // Hex No. C3 (Dec 195) Character '√' Symbol
    QChar(0x0192),  // Hex No. C4 (Dec 196) Character 'ƒ' Letter, Lowercase
    QChar(0x2248),  // Hex No. C5 (Dec 197) Character '≈' Symbol
    QChar(0x2206),  // Hex No. C6 (Dec 198) Character '∆' Symbol
    QChar(0x00ab),  // Hex No. C7 (Dec 199) Character '«' Punctuation
    QChar(0x00bb),  // Hex No. C8 (Dec 200) Character '»' Punctuation
    QChar(0x2026),  // Hex No. C9 (Dec 201) Character '…' Punctuation
    QChar(0x0020),  // Hex No. CA (Dec 202) Character ' ' Whitespace
    QChar(0x00c0),  // Hex No. CB (Dec 203) Character 'À' Letter, Uppercase
    QChar(0x00c3),  // Hex No. CC (Dec 204) Character 'Ã' Letter, Uppercase
    QChar(0x00d5),  // Hex No. CD (Dec 205) Character 'Õ' Letter, Uppercase
    QChar(0x0152),  // Hex No. CE (Dec 206) Character 'Œ' Letter, Uppercase
    QChar(0x0153),  // Hex No. CF (Dec 207) Character 'œ' Letter, Lowercase
    QChar(0x2013),  // Hex No. D0 (Dec 208) Character '–' Punctuation
    QChar(0x2014),  // Hex No. D1 (Dec 209) Character '—' Punctuation
    QChar(0x201c),  // Hex No. D2 (Dec 210) Character '“' Punctuation
    QChar(0x201d),  // Hex No. D3 (Dec 211) Character '”' Punctuation
    QChar(0x2018),  // Hex No. D4 (Dec 212) Character '‘' Punctuation
    QChar(0x2019),  // Hex No. D5 (Dec 213) Character '’' Punctuation
    QChar(0x00f7),  // Hex No. D6 (Dec 214) Character '÷' Symbol
    QChar(0x25ca),  // Hex No. D7 (Dec 215) Character '◊' Symbol
    QChar(0x00ff),  // Hex No. D8 (Dec 216) Character 'ÿ' Letter, Lowercase
    QChar(0x0178),  // Hex No. D9 (Dec 217) Character 'Ÿ' Letter, Uppercase
    QChar(0x2044),  // Hex No. DA (Dec 218) Character '⁄' Symbol
    QChar(0x00a4),  // Hex No. DB (Dec 219) Character '¤' Symbol
    QChar(0x2039),  // Hex No. DC (Dec 220) Character '‹' Punctuation
    QChar(0x203a),  // Hex No. DD (Dec 221) Character '›' Punctuation
    QChar(0xfb01),  // Hex No. DE (Dec 222) Character 'ﬁ' Letter, Lowercase
    QChar(0xfb02),  // Hex No. DF (Dec 223) Character 'ﬂ' Letter, Lowercase
    QChar(0x2021),  // Hex No. E0 (Dec 224) Character '‡' Punctuation
    QChar(0x00b7),  // Hex No. E1 (Dec 225) Character '·' Punctuation
    QChar(0x201a),  // Hex No. E2 (Dec 226) Character '‚' Punctuation
    QChar(0x201e),  // Hex No. E3 (Dec 227) Character '„' Punctuation
    QChar(0x2030),  // Hex No. E4 (Dec 228) Character '‰' Punctuation
    QChar(0x00c2),  // Hex No. E5 (Dec 229) Character 'Â' Letter, Uppercase
    QChar(0x00ca),  // Hex No. E6 (Dec 230) Character 'Ê' Letter, Uppercase
    QChar(0x00c1),  // Hex No. E7 (Dec 231) Character 'Á' Letter, Uppercase
    QChar(0x00cb),  // Hex No. E8 (Dec 232) Character 'Ë' Letter, Uppercase
    QChar(0x00c8),  // Hex No. E9 (Dec 233) Character 'È' Letter, Uppercase
    QChar(0x00cd),  // Hex No. EA (Dec 234) Character 'Í' Letter, Uppercase
    QChar(0x00ce),  // Hex No. EB (Dec 235) Character 'Î' Letter, Uppercase
    QChar(0x00cf),  // Hex No. EC (Dec 236) Character 'Ï' Letter, Uppercase
    QChar(0x00cc),  // Hex No. ED (Dec 237) Character 'Ì' Letter, Uppercase
    QChar(0x00d3),  // Hex No. EE (Dec 238) Character 'Ó' Letter, Uppercase
    QChar(0x00d4),  // Hex No. EF (Dec 239) Character 'Ô' Letter, Uppercase
    QChar(0xf8ff),  // Hex No. F0 (Dec 240)
    QChar(0x00d2),  // Hex No. F1 (Dec 241) Character 'Ò' Letter, Uppercase
    QChar(0x00da),  // Hex No. F2 (Dec 242) Character 'Ú' Letter, Uppercase
    QChar(0x00db),  // Hex No. F3 (Dec 243) Character 'Û' Letter, Uppercase
    QChar(0x00d9),  // Hex No. F4 (Dec 244) Character 'Ù' Letter, Uppercase
    QChar(0x0131),  // Hex No. F5 (Dec 245) Character 'ı' Letter, Lowercase
    QChar(0x02c6),  // Hex No. F6 (Dec 246) Character 'ˆ' Letter
    QChar(0x02dc),  // Hex No. F7 (Dec 247) Character '˜' Symbol
    QChar(0x00af),  // Hex No. F8 (Dec 248) Character '¯' Symbol
    QChar(0x02d8),  // Hex No. F9 (Dec 249) Character '˘' Symbol
    QChar(0x02d9),  // Hex No. FA (Dec 250) Character '˙' Symbol
    QChar(0x02da),  // Hex No. FB (Dec 251) Character '˚' Symbol
    QChar(0x00b8),  // Hex No. FC (Dec 252) Character '¸' Symbol
    QChar(0x02dd),  // Hex No. FD (Dec 253) Character '˝' Symbol
    QChar(0x02db),  // Hex No. FE (Dec 254) Character '˛' Symbol
    QChar(0x02c7),  // Hex No. FF (Dec 255) Character 'ˇ' Letter
};

// PDF Reference 1.7, Appendix D, Section D.1, WinAnsiEncoding
static const EncodingTable WIN_ANSI_ENCODING_CONVERSION_TABLE = {
    QChar(0xfffd),  // Hex No. 00 (Dec 000) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 01 (Dec 001) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 02 (Dec 002) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 03 (Dec 003) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 04 (Dec 004) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 05 (Dec 005) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 06 (Dec 006) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 07 (Dec 007) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 08 (Dec 008) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 09 (Dec 009) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 0A (Dec 010) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 0B (Dec 011) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 0C (Dec 012) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 0D (Dec 013) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 0E (Dec 014) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 0F (Dec 015) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 10 (Dec 016) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 11 (Dec 017) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 12 (Dec 018) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 13 (Dec 019) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 14 (Dec 020) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 15 (Dec 021) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 16 (Dec 022) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 17 (Dec 023) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 18 (Dec 024) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 19 (Dec 025) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 1A (Dec 026) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 1B (Dec 027) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 1C (Dec 028) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 1D (Dec 029) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 1E (Dec 030) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 1F (Dec 031) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0x0020),  // Hex No. 20 (Dec 032) Character ' ' Whitespace
    QChar(0x0021),  // Hex No. 21 (Dec 033) Character '!' Punctuation
    QChar(0x0022),  // Hex No. 22 (Dec 034) Character '"' Punctuation
    QChar(0x0023),  // Hex No. 23 (Dec 035) Character '#' Punctuation
    QChar(0x0024),  // Hex No. 24 (Dec 036) Character '$' Symbol
    QChar(0x0025),  // Hex No. 25 (Dec 037) Character '%' Punctuation
    QChar(0x0026),  // Hex No. 26 (Dec 038) Character '&' Punctuation
    QChar(0x0027),  // Hex No. 27 (Dec 039) Character ''' Punctuation
    QChar(0x0028),  // Hex No. 28 (Dec 040) Character '(' Punctuation
    QChar(0x0029),  // Hex No. 29 (Dec 041) Character ')' Punctuation
    QChar(0x002a),  // Hex No. 2A (Dec 042) Character '*' Punctuation
    QChar(0x002b),  // Hex No. 2B (Dec 043) Character '+' Symbol
    QChar(0x002c),  // Hex No. 2C (Dec 044) Character ',' Punctuation
    QChar(0x002d),  // Hex No. 2D (Dec 045) Character '-' Punctuation
    QChar(0x002e),  // Hex No. 2E (Dec 046) Character '.' Punctuation
    QChar(0x002f),  // Hex No. 2F (Dec 047) Character '/' Punctuation
    QChar(0x0030),  // Hex No. 30 (Dec 048) Character '0' Digit
    QChar(0x0031),  // Hex No. 31 (Dec 049) Character '1' Digit
    QChar(0x0032),  // Hex No. 32 (Dec 050) Character '2' Digit
    QChar(0x0033),  // Hex No. 33 (Dec 051) Character '3' Digit
    QChar(0x0034),  // Hex No. 34 (Dec 052) Character '4' Digit
    QChar(0x0035),  // Hex No. 35 (Dec 053) Character '5' Digit
    QChar(0x0036),  // Hex No. 36 (Dec 054) Character '6' Digit
    QChar(0x0037),  // Hex No. 37 (Dec 055) Character '7' Digit
    QChar(0x0038),  // Hex No. 38 (Dec 056) Character '8' Digit
    QChar(0x0039),  // Hex No. 39 (Dec 057) Character '9' Digit
    QChar(0x003a),  // Hex No. 3A (Dec 058) Character ':' Punctuation
    QChar(0x003b),  // Hex No. 3B (Dec 059) Character ';' Punctuation
    QChar(0x003c),  // Hex No. 3C (Dec 060) Character '<' Symbol
    QChar(0x003d),  // Hex No. 3D (Dec 061) Character '=' Symbol
    QChar(0x003e),  // Hex No. 3E (Dec 062) Character '>' Symbol
    QChar(0x003f),  // Hex No. 3F (Dec 063) Character '?' Punctuation
    QChar(0x0040),  // Hex No. 40 (Dec 064) Character '@' Punctuation
    QChar(0x0041),  // Hex No. 41 (Dec 065) Character 'A' Letter, Uppercase
    QChar(0x0042),  // Hex No. 42 (Dec 066) Character 'B' Letter, Uppercase
    QChar(0x0043),  // Hex No. 43 (Dec 067) Character 'C' Letter, Uppercase
    QChar(0x0044),  // Hex No. 44 (Dec 068) Character 'D' Letter, Uppercase
    QChar(0x0045),  // Hex No. 45 (Dec 069) Character 'E' Letter, Uppercase
    QChar(0x0046),  // Hex No. 46 (Dec 070) Character 'F' Letter, Uppercase
    QChar(0x0047),  // Hex No. 47 (Dec 071) Character 'G' Letter, Uppercase
    QChar(0x0048),  // Hex No. 48 (Dec 072) Character 'H' Letter, Uppercase
    QChar(0x0049),  // Hex No. 49 (Dec 073) Character 'I' Letter, Uppercase
    QChar(0x004a),  // Hex No. 4A (Dec 074) Character 'J' Letter, Uppercase
    QChar(0x004b),  // Hex No. 4B (Dec 075) Character 'K' Letter, Uppercase
    QChar(0x004c),  // Hex No. 4C (Dec 076) Character 'L' Letter, Uppercase
    QChar(0x004d),  // Hex No. 4D (Dec 077) Character 'M' Letter, Uppercase
    QChar(0x004e),  // Hex No. 4E (Dec 078) Character 'N' Letter, Uppercase
    QChar(0x004f),  // Hex No. 4F (Dec 079) Character 'O' Letter, Uppercase
    QChar(0x0050),  // Hex No. 50 (Dec 080) Character 'P' Letter, Uppercase
    QChar(0x0051),  // Hex No. 51 (Dec 081) Character 'Q' Letter, Uppercase
    QChar(0x0052),  // Hex No. 52 (Dec 082) Character 'R' Letter, Uppercase
    QChar(0x0053),  // Hex No. 53 (Dec 083) Character 'S' Letter, Uppercase
    QChar(0x0054),  // Hex No. 54 (Dec 084) Character 'T' Letter, Uppercase
    QChar(0x0055),  // Hex No. 55 (Dec 085) Character 'U' Letter, Uppercase
    QChar(0x0056),  // Hex No. 56 (Dec 086) Character 'V' Letter, Uppercase
    QChar(0x0057),  // Hex No. 57 (Dec 087) Character 'W' Letter, Uppercase
    QChar(0x0058),  // Hex No. 58 (Dec 088) Character 'X' Letter, Uppercase
    QChar(0x0059),  // Hex No. 59 (Dec 089) Character 'Y' Letter, Uppercase
    QChar(0x005a),  // Hex No. 5A (Dec 090) Character 'Z' Letter, Uppercase
    QChar(0x005b),  // Hex No. 5B (Dec 091) Character '[' Punctuation
    QChar(0x005c),  // Hex No. 5C (Dec 092) Character '\' Punctuation
    QChar(0x005d),  // Hex No. 5D (Dec 093) Character ']' Punctuation
    QChar(0x005e),  // Hex No. 5E (Dec 094) Character '^' Symbol
    QChar(0x005f),  // Hex No. 5F (Dec 095) Character '_' Punctuation
    QChar(0x0060),  // Hex No. 60 (Dec 096) Character '`' Symbol
    QChar(0x0061),  // Hex No. 61 (Dec 097) Character 'a' Letter, Lowercase
    QChar(0x0062),  // Hex No. 62 (Dec 098) Character 'b' Letter, Lowercase
    QChar(0x0063),  // Hex No. 63 (Dec 099) Character 'c' Letter, Lowercase
    QChar(0x0064),  // Hex No. 64 (Dec 100) Character 'd' Letter, Lowercase
    QChar(0x0065),  // Hex No. 65 (Dec 101) Character 'e' Letter, Lowercase
    QChar(0x0066),  // Hex No. 66 (Dec 102) Character 'f' Letter, Lowercase
    QChar(0x0067),  // Hex No. 67 (Dec 103) Character 'g' Letter, Lowercase
    QChar(0x0068),  // Hex No. 68 (Dec 104) Character 'h' Letter, Lowercase
    QChar(0x0069),  // Hex No. 69 (Dec 105) Character 'i' Letter, Lowercase
    QChar(0x006a),  // Hex No. 6A (Dec 106) Character 'j' Letter, Lowercase
    QChar(0x006b),  // Hex No. 6B (Dec 107) Character 'k' Letter, Lowercase
    QChar(0x006c),  // Hex No. 6C (Dec 108) Character 'l' Letter, Lowercase
    QChar(0x006d),  // Hex No. 6D (Dec 109) Character 'm' Letter, Lowercase
    QChar(0x006e),  // Hex No. 6E (Dec 110) Character 'n' Letter, Lowercase
    QChar(0x006f),  // Hex No. 6F (Dec 111) Character 'o' Letter, Lowercase
    QChar(0x0070),  // Hex No. 70 (Dec 112) Character 'p' Letter, Lowercase
    QChar(0x0071),  // Hex No. 71 (Dec 113) Character 'q' Letter, Lowercase
    QChar(0x0072),  // Hex No. 72 (Dec 114) Character 'r' Letter, Lowercase
    QChar(0x0073),  // Hex No. 73 (Dec 115) Character 's' Letter, Lowercase
    QChar(0x0074),  // Hex No. 74 (Dec 116) Character 't' Letter, Lowercase
    QChar(0x0075),  // Hex No. 75 (Dec 117) Character 'u' Letter, Lowercase
    QChar(0x0076),  // Hex No. 76 (Dec 118) Character 'v' Letter, Lowercase
    QChar(0x0077),  // Hex No. 77 (Dec 119) Character 'w' Letter, Lowercase
    QChar(0x0078),  // Hex No. 78 (Dec 120) Character 'x' Letter, Lowercase
    QChar(0x0079),  // Hex No. 79 (Dec 121) Character 'y' Letter, Lowercase
    QChar(0x007a),  // Hex No. 7A (Dec 122) Character 'z' Letter, Lowercase
    QChar(0x007b),  // Hex No. 7B (Dec 123) Character '{' Punctuation
    QChar(0x007c),  // Hex No. 7C (Dec 124) Character '|' Symbol
    QChar(0x007d),  // Hex No. 7D (Dec 125) Character '}' Punctuation
    QChar(0x007e),  // Hex No. 7E (Dec 126) Character '~' Symbol
    QChar(0x2022),  // Hex No. 7F (Dec 127) Character '•' Punctuation
    QChar(0x20ac),  // Hex No. 80 (Dec 128) Character '€' Symbol
    QChar(0x2022),  // Hex No. 81 (Dec 129) Character '•' Punctuation
    QChar(0x201a),  // Hex No. 82 (Dec 130) Character '‚' Punctuation
    QChar(0x0192),  // Hex No. 83 (Dec 131) Character 'ƒ' Letter, Lowercase
    QChar(0x201e),  // Hex No. 84 (Dec 132) Character '„' Punctuation
    QChar(0x2026),  // Hex No. 85 (Dec 133) Character '…' Punctuation
    QChar(0x2020),  // Hex No. 86 (Dec 134) Character '†' Punctuation
    QChar(0x2021),  // Hex No. 87 (Dec 135) Character '‡' Punctuation
    QChar(0x02c6),  // Hex No. 88 (Dec 136) Character 'ˆ' Letter
    QChar(0x2030),  // Hex No. 89 (Dec 137) Character '‰' Punctuation
    QChar(0x0160),  // Hex No. 8A (Dec 138) Character 'Š' Letter, Uppercase
    QChar(0x2039),  // Hex No. 8B (Dec 139) Character '‹' Punctuation
    QChar(0x0152),  // Hex No. 8C (Dec 140) Character 'Œ' Letter, Uppercase
    QChar(0x2022),  // Hex No. 8D (Dec 141) Character '•' Punctuation
    QChar(0x017d),  // Hex No. 8E (Dec 142) Character 'Ž' Letter, Uppercase
    QChar(0x2022),  // Hex No. 8F (Dec 143) Character '•' Punctuation
    QChar(0x2022),  // Hex No. 90 (Dec 144) Character '•' Punctuation
    QChar(0x2018),  // Hex No. 91 (Dec 145) Character '‘' Punctuation
    QChar(0x2019),  // Hex No. 92 (Dec 146) Character '’' Punctuation
    QChar(0x201c),  // Hex No. 93 (Dec 147) Character '“' Punctuation
    QChar(0x201d),  // Hex No. 94 (Dec 148) Character '”' Punctuation
    QChar(0x2022),  // Hex No. 95 (Dec 149) Character '•' Punctuation
    QChar(0x2013),  // Hex No. 96 (Dec 150) Character '–' Punctuation
    QChar(0x2014),  // Hex No. 97 (Dec 151) Character '—' Punctuation
    QChar(0x02dc),  // Hex No. 98 (Dec 152) Character '˜' Symbol
    QChar(0x2122),  // Hex No. 99 (Dec 153) Character '™' Symbol
    QChar(0x0161),  // Hex No. 9A (Dec 154) Character 'š' Letter, Lowercase
    QChar(0x203a),  // Hex No. 9B (Dec 155) Character '›' Punctuation
    QChar(0x0153),  // Hex No. 9C (Dec 156) Character 'œ' Letter, Lowercase
    QChar(0x2022),  // Hex No. 9D (Dec 157) Character '•' Punctuation
    QChar(0x017e),  // Hex No. 9E (Dec 158) Character 'ž' Letter, Lowercase
    QChar(0x0178),  // Hex No. 9F (Dec 159) Character 'Ÿ' Letter, Uppercase
    QChar(0x0020),  // Hex No. A0 (Dec 160) Character ' ' Whitespace
    QChar(0x00a1),  // Hex No. A1 (Dec 161) Character '¡' Punctuation
    QChar(0x00a2),  // Hex No. A2 (Dec 162) Character '¢' Symbol
    QChar(0x00a3),  // Hex No. A3 (Dec 163) Character '£' Symbol
    QChar(0x00a4),  // Hex No. A4 (Dec 164) Character '¤' Symbol
    QChar(0x00a5),  // Hex No. A5 (Dec 165) Character '¥' Symbol
    QChar(0x00a6),  // Hex No. A6 (Dec 166) Character '¦' Symbol
    QChar(0x00a7),  // Hex No. A7 (Dec 167) Character '§' Punctuation
    QChar(0x00a8),  // Hex No. A8 (Dec 168) Character '¨' Symbol
    QChar(0x00a9),  // Hex No. A9 (Dec 169) Character '©' Symbol
    QChar(0x00aa),  // Hex No. AA (Dec 170) Character 'ª' Letter
    QChar(0x00ab),  // Hex No. AB (Dec 171) Character '«' Punctuation
    QChar(0x00ac),  // Hex No. AC (Dec 172) Character '¬' Symbol
    QChar(0x002d),  // Hex No. AD (Dec 173) Character '-' Punctuation
    QChar(0x00ae),  // Hex No. AE (Dec 174) Character '®' Symbol
    QChar(0x00af),  // Hex No. AF (Dec 175) Character '¯' Symbol
    QChar(0x00b0),  // Hex No. B0 (Dec 176) Character '°' Symbol
    QChar(0x00b1),  // Hex No. B1 (Dec 177) Character '±' Symbol
    QChar(0x00b2),  // Hex No. B2 (Dec 178) Character '²'
    QChar(0x00b3),  // Hex No. B3 (Dec 179) Character '³'
    QChar(0x00b4),  // Hex No. B4 (Dec 180) Character '´' Symbol
    QChar(0x00b5),  // Hex No. B5 (Dec 181) Character 'µ' Letter, Lowercase
    QChar(0x00b6),  // Hex No. B6 (Dec 182) Character '¶' Punctuation
    QChar(0x00b7),  // Hex No. B7 (Dec 183) Character '·' Punctuation
    QChar(0x00b8),  // Hex No. B8 (Dec 184) Character '¸' Symbol
    QChar(0x00b9),  // Hex No. B9 (Dec 185) Character '¹'
    QChar(0x00ba),  // Hex No. BA (Dec 186) Character 'º' Letter
    QChar(0x00bb),  // Hex No. BB (Dec 187) Character '»' Punctuation
    QChar(0x00bc),  // Hex No. BC (Dec 188) Character '¼'
    QChar(0x00bd),  // Hex No. BD (Dec 189) Character '½'
    QChar(0x00be),  // Hex No. BE (Dec 190) Character '¾'
    QChar(0x00bf),  // Hex No. BF (Dec 191) Character '¿' Punctuation
    QChar(0x00c0),  // Hex No. C0 (Dec 192) Character 'À' Letter, Uppercase
    QChar(0x00c1),  // Hex No. C1 (Dec 193) Character 'Á' Letter, Uppercase
    QChar(0x00c2),  // Hex No. C2 (Dec 194) Character 'Â' Letter, Uppercase
    QChar(0x00c3),  // Hex No. C3 (Dec 195) Character 'Ã' Letter, Uppercase
    QChar(0x00c4),  // Hex No. C4 (Dec 196) Character 'Ä' Letter, Uppercase
    QChar(0x00c5),  // Hex No. C5 (Dec 197) Character 'Å' Letter, Uppercase
    QChar(0x00c6),  // Hex No. C6 (Dec 198) Character 'Æ' Letter, Uppercase
    QChar(0x00c7),  // Hex No. C7 (Dec 199) Character 'Ç' Letter, Uppercase
    QChar(0x00c8),  // Hex No. C8 (Dec 200) Character 'È' Letter, Uppercase
    QChar(0x00c9),  // Hex No. C9 (Dec 201) Character 'É' Letter, Uppercase
    QChar(0x00ca),  // Hex No. CA (Dec 202) Character 'Ê' Letter, Uppercase
    QChar(0x00cb),  // Hex No. CB (Dec 203) Character 'Ë' Letter, Uppercase
    QChar(0x00cc),  // Hex No. CC (Dec 204) Character 'Ì' Letter, Uppercase
    QChar(0x00cd),  // Hex No. CD (Dec 205) Character 'Í' Letter, Uppercase
    QChar(0x00ce),  // Hex No. CE (Dec 206) Character 'Î' Letter, Uppercase
    QChar(0x00cf),  // Hex No. CF (Dec 207) Character 'Ï' Letter, Uppercase
    QChar(0x00d0),  // Hex No. D0 (Dec 208) Character 'Ð' Letter, Uppercase
    QChar(0x00d1),  // Hex No. D1 (Dec 209) Character 'Ñ' Letter, Uppercase
    QChar(0x00d2),  // Hex No. D2 (Dec 210) Character 'Ò' Letter, Uppercase
    QChar(0x00d3),  // Hex No. D3 (Dec 211) Character 'Ó' Letter, Uppercase
    QChar(0x00d4),  // Hex No. D4 (Dec 212) Character 'Ô' Letter, Uppercase
    QChar(0x00d5),  // Hex No. D5 (Dec 213) Character 'Õ' Letter, Uppercase
    QChar(0x00d6),  // Hex No. D6 (Dec 214) Character 'Ö' Letter, Uppercase
    QChar(0x00d7),  // Hex No. D7 (Dec 215) Character '×' Symbol
    QChar(0x00d8),  // Hex No. D8 (Dec 216) Character 'Ø' Letter, Uppercase
    QChar(0x00d9),  // Hex No. D9 (Dec 217) Character 'Ù' Letter, Uppercase
    QChar(0x00da),  // Hex No. DA (Dec 218) Character 'Ú' Letter, Uppercase
    QChar(0x00db),  // Hex No. DB (Dec 219) Character 'Û' Letter, Uppercase
    QChar(0x00dc),  // Hex No. DC (Dec 220) Character 'Ü' Letter, Uppercase
    QChar(0x00dd),  // Hex No. DD (Dec 221) Character 'Ý' Letter, Uppercase
    QChar(0x00de),  // Hex No. DE (Dec 222) Character 'Þ' Letter, Uppercase
    QChar(0x00df),  // Hex No. DF (Dec 223) Character 'ß' Letter, Lowercase
    QChar(0x00e0),  // Hex No. E0 (Dec 224) Character 'à' Letter, Lowercase
    QChar(0x00e1),  // Hex No. E1 (Dec 225) Character 'á' Letter, Lowercase
    QChar(0x00e2),  // Hex No. E2 (Dec 226) Character 'â' Letter, Lowercase
    QChar(0x00e3),  // Hex No. E3 (Dec 227) Character 'ã' Letter, Lowercase
    QChar(0x00e4),  // Hex No. E4 (Dec 228) Character 'ä' Letter, Lowercase
    QChar(0x00e5),  // Hex No. E5 (Dec 229) Character 'å' Letter, Lowercase
    QChar(0x00e6),  // Hex No. E6 (Dec 230) Character 'æ' Letter, Lowercase
    QChar(0x00e7),  // Hex No. E7 (Dec 231) Character 'ç' Letter, Lowercase
    QChar(0x00e8),  // Hex No. E8 (Dec 232) Character 'è' Letter, Lowercase
    QChar(0x00e9),  // Hex No. E9 (Dec 233) Character 'é' Letter, Lowercase
    QChar(0x00ea),  // Hex No. EA (Dec 234) Character 'ê' Letter, Lowercase
    QChar(0x00eb),  // Hex No. EB (Dec 235) Character 'ë' Letter, Lowercase
    QChar(0x00ec),  // Hex No. EC (Dec 236) Character 'ì' Letter, Lowercase
    QChar(0x00ed),  // Hex No. ED (Dec 237) Character 'í' Letter, Lowercase
    QChar(0x00ee),  // Hex No. EE (Dec 238) Character 'î' Letter, Lowercase
    QChar(0x00ef),  // Hex No. EF (Dec 239) Character 'ï' Letter, Lowercase
    QChar(0x00f0),  // Hex No. F0 (Dec 240) Character 'ð' Letter, Lowercase
    QChar(0x00f1),  // Hex No. F1 (Dec 241) Character 'ñ' Letter, Lowercase
    QChar(0x00f2),  // Hex No. F2 (Dec 242) Character 'ò' Letter, Lowercase
    QChar(0x00f3),  // Hex No. F3 (Dec 243) Character 'ó' Letter, Lowercase
    QChar(0x00f4),  // Hex No. F4 (Dec 244) Character 'ô' Letter, Lowercase
    QChar(0x00f5),  // Hex No. F5 (Dec 245) Character 'õ' Letter, Lowercase
    QChar(0x00f6),  // Hex No. F6 (Dec 246) Character 'ö' Letter, Lowercase
    QChar(0x00f7),  // Hex No. F7 (Dec 247) Character '÷' Symbol
    QChar(0x00f8),  // Hex No. F8 (Dec 248) Character 'ø' Letter, Lowercase
    QChar(0x00f9),  // Hex No. F9 (Dec 249) Character 'ù' Letter, Lowercase
    QChar(0x00fa),  // Hex No. FA (Dec 250) Character 'ú' Letter, Lowercase
    QChar(0x00fb),  // Hex No. FB (Dec 251) Character 'û' Letter, Lowercase
    QChar(0x00fc),  // Hex No. FC (Dec 252) Character 'ü' Letter, Lowercase
    QChar(0x00fd),  // Hex No. FD (Dec 253) Character 'ý' Letter, Lowercase
    QChar(0x00fe),  // Hex No. FE (Dec 254) Character 'þ' Letter, Lowercase
    QChar(0x00ff),  // Hex No. FF (Dec 255) Character 'ÿ' Letter, Lowercase
};

// PDF Reference 1.7, Appendix D, Section D.1/D.2, PDFDocEncoding
static const EncodingTable PDF_DOC_ENCODING_CONVERSION_TABLE = {
    QChar(0x0000),  // Hex No. 00 (Dec 000) Null character
    QChar(0x0001),  // Hex No. 01 (Dec 001)
    QChar(0x0002),  // Hex No. 02 (Dec 002)
    QChar(0x0003),  // Hex No. 03 (Dec 003)
    QChar(0x0004),  // Hex No. 04 (Dec 004)
    QChar(0x0005),  // Hex No. 05 (Dec 005)
    QChar(0x0006),  // Hex No. 06 (Dec 006)
    QChar(0x0007),  // Hex No. 07 (Dec 007)
    QChar(0x0008),  // Hex No. 08 (Dec 008)
    QChar(0x0009),  // Hex No. 09 (Dec 009) Whitespace
    QChar(0x000a),  // Hex No. 0A (Dec 010) Whitespace
    QChar(0x000b),  // Hex No. 0B (Dec 011) Whitespace
    QChar(0x000c),  // Hex No. 0C (Dec 012) Whitespace
    QChar(0x000d),  // Hex No. 0D (Dec 013) Whitespace
    QChar(0x000e),  // Hex No. 0E (Dec 014)
    QChar(0x000f),  // Hex No. 0F (Dec 015)
    QChar(0x0010),  // Hex No. 10 (Dec 016)
    QChar(0x0011),  // Hex No. 11 (Dec 017)
    QChar(0x0012),  // Hex No. 12 (Dec 018)
    QChar(0x0013),  // Hex No. 13 (Dec 019)
    QChar(0x0014),  // Hex No. 14 (Dec 020)
    QChar(0x0015),  // Hex No. 15 (Dec 021)
    QChar(0x0016),  // Hex No. 16 (Dec 022)
    QChar(0x0017),  // Hex No. 17 (Dec 023)
    QChar(0x02d8),  // Hex No. 18 (Dec 024) Character '˘' Symbol
    QChar(0x02c7),  // Hex No. 19 (Dec 025) Character 'ˇ' Letter
    QChar(0x02c6),  // Hex No. 1A (Dec 026) Character 'ˆ' Letter
    QChar(0x02d9),  // Hex No. 1B (Dec 027) Character '˙' Symbol
    QChar(0x02dd),  // Hex No. 1C (Dec 028) Character '˝' Symbol
    QChar(0x02db),  // Hex No. 1D (Dec 029) Character '˛' Symbol
    QChar(0x02da),  // Hex No. 1E (Dec 030) Character '˚' Symbol
    QChar(0x02dc),  // Hex No. 1F (Dec 031) Character '˜' Symbol
    QChar(0x0020),  // Hex No. 20 (Dec 032) Character ' ' Whitespace
    QChar(0x0021),  // Hex No. 21 (Dec 033) Character '!' Punctuation
    QChar(0x0022),  // Hex No. 22 (Dec 034) Character '"' Punctuation
    QChar(0x0023),  // Hex No. 23 (Dec 035) Character '#' Punctuation
    QChar(0x0024),  // Hex No. 24 (Dec 036) Character '$' Symbol
    QChar(0x0025),  // Hex No. 25 (Dec 037) Character '%' Punctuation
    QChar(0x0026),  // Hex No. 26 (Dec 038) Character '&' Punctuation
    QChar(0x0027),  // Hex No. 27 (Dec 039) Character ''' Punctuation
    QChar(0x0028),  // Hex No. 28 (Dec 040) Character '(' Punctuation
    QChar(0x0029),  // Hex No. 29 (Dec 041) Character ')' Punctuation
    QChar(0x002a),  // Hex No. 2A (Dec 042) Character '*' Punctuation
    QChar(0x002b),  // Hex No. 2B (Dec 043) Character '+' Symbol
    QChar(0x002c),  // Hex No. 2C (Dec 044) Character ',' Punctuation
    QChar(0x002d),  // Hex No. 2D (Dec 045) Character '-' Punctuation
    QChar(0x002e),  // Hex No. 2E (Dec 046) Character '.' Punctuation
    QChar(0x002f),  // Hex No. 2F (Dec 047) Character '/' Punctuation
    QChar(0x0030),  // Hex No. 30 (Dec 048) Character '0' Digit
    QChar(0x0031),  // Hex No. 31 (Dec 049) Character '1' Digit
    QChar(0x0032),  // Hex No. 32 (Dec 050) Character '2' Digit
    QChar(0x0033),  // Hex No. 33 (Dec 051) Character '3' Digit
    QChar(0x0034),  // Hex No. 34 (Dec 052) Character '4' Digit
    QChar(0x0035),  // Hex No. 35 (Dec 053) Character '5' Digit
    QChar(0x0036),  // Hex No. 36 (Dec 054) Character '6' Digit
    QChar(0x0037),  // Hex No. 37 (Dec 055) Character '7' Digit
    QChar(0x0038),  // Hex No. 38 (Dec 056) Character '8' Digit
    QChar(0x0039),  // Hex No. 39 (Dec 057) Character '9' Digit
    QChar(0x003a),  // Hex No. 3A (Dec 058) Character ':' Punctuation
    QChar(0x003b),  // Hex No. 3B (Dec 059) Character ';' Punctuation
    QChar(0x003c),  // Hex No. 3C (Dec 060) Character '<' Symbol
    QChar(0x003d),  // Hex No. 3D (Dec 061) Character '=' Symbol
    QChar(0x003e),  // Hex No. 3E (Dec 062) Character '>' Symbol
    QChar(0x003f),  // Hex No. 3F (Dec 063) Character '?' Punctuation
    QChar(0x0040),  // Hex No. 40 (Dec 064) Character '@' Punctuation
    QChar(0x0041),  // Hex No. 41 (Dec 065) Character 'A' Letter, Uppercase
    QChar(0x0042),  // Hex No. 42 (Dec 066) Character 'B' Letter, Uppercase
    QChar(0x0043),  // Hex No. 43 (Dec 067) Character 'C' Letter, Uppercase
    QChar(0x0044),  // Hex No. 44 (Dec 068) Character 'D' Letter, Uppercase
    QChar(0x0045),  // Hex No. 45 (Dec 069) Character 'E' Letter, Uppercase
    QChar(0x0046),  // Hex No. 46 (Dec 070) Character 'F' Letter, Uppercase
    QChar(0x0047),  // Hex No. 47 (Dec 071) Character 'G' Letter, Uppercase
    QChar(0x0048),  // Hex No. 48 (Dec 072) Character 'H' Letter, Uppercase
    QChar(0x0049),  // Hex No. 49 (Dec 073) Character 'I' Letter, Uppercase
    QChar(0x004a),  // Hex No. 4A (Dec 074) Character 'J' Letter, Uppercase
    QChar(0x004b),  // Hex No. 4B (Dec 075) Character 'K' Letter, Uppercase
    QChar(0x004c),  // Hex No. 4C (Dec 076) Character 'L' Letter, Uppercase
    QChar(0x004d),  // Hex No. 4D (Dec 077) Character 'M' Letter, Uppercase
    QChar(0x004e),  // Hex No. 4E (Dec 078) Character 'N' Letter, Uppercase
    QChar(0x004f),  // Hex No. 4F (Dec 079) Character 'O' Letter, Uppercase
    QChar(0x0050),  // Hex No. 50 (Dec 080) Character 'P' Letter, Uppercase
    QChar(0x0051),  // Hex No. 51 (Dec 081) Character 'Q' Letter, Uppercase
    QChar(0x0052),  // Hex No. 52 (Dec 082) Character 'R' Letter, Uppercase
    QChar(0x0053),  // Hex No. 53 (Dec 083) Character 'S' Letter, Uppercase
    QChar(0x0054),  // Hex No. 54 (Dec 084) Character 'T' Letter, Uppercase
    QChar(0x0055),  // Hex No. 55 (Dec 085) Character 'U' Letter, Uppercase
    QChar(0x0056),  // Hex No. 56 (Dec 086) Character 'V' Letter, Uppercase
    QChar(0x0057),  // Hex No. 57 (Dec 087) Character 'W' Letter, Uppercase
    QChar(0x0058),  // Hex No. 58 (Dec 088) Character 'X' Letter, Uppercase
    QChar(0x0059),  // Hex No. 59 (Dec 089) Character 'Y' Letter, Uppercase
    QChar(0x005a),  // Hex No. 5A (Dec 090) Character 'Z' Letter, Uppercase
    QChar(0x005b),  // Hex No. 5B (Dec 091) Character '[' Punctuation
    QChar(0x005c),  // Hex No. 5C (Dec 092) Character '\' Punctuation
    QChar(0x005d),  // Hex No. 5D (Dec 093) Character ']' Punctuation
    QChar(0x005e),  // Hex No. 5E (Dec 094) Character '^' Symbol
    QChar(0x005f),  // Hex No. 5F (Dec 095) Character '_' Punctuation
    QChar(0x0060),  // Hex No. 60 (Dec 096) Character '`' Symbol
    QChar(0x0061),  // Hex No. 61 (Dec 097) Character 'a' Letter, Lowercase
    QChar(0x0062),  // Hex No. 62 (Dec 098) Character 'b' Letter, Lowercase
    QChar(0x0063),  // Hex No. 63 (Dec 099) Character 'c' Letter, Lowercase
    QChar(0x0064),  // Hex No. 64 (Dec 100) Character 'd' Letter, Lowercase
    QChar(0x0065),  // Hex No. 65 (Dec 101) Character 'e' Letter, Lowercase
    QChar(0x0066),  // Hex No. 66 (Dec 102) Character 'f' Letter, Lowercase
    QChar(0x0067),  // Hex No. 67 (Dec 103) Character 'g' Letter, Lowercase
    QChar(0x0068),  // Hex No. 68 (Dec 104) Character 'h' Letter, Lowercase
    QChar(0x0069),  // Hex No. 69 (Dec 105) Character 'i' Letter, Lowercase
    QChar(0x006a),  // Hex No. 6A (Dec 106) Character 'j' Letter, Lowercase
    QChar(0x006b),  // Hex No. 6B (Dec 107) Character 'k' Letter, Lowercase
    QChar(0x006c),  // Hex No. 6C (Dec 108) Character 'l' Letter, Lowercase
    QChar(0x006d),  // Hex No. 6D (Dec 109) Character 'm' Letter, Lowercase
    QChar(0x006e),  // Hex No. 6E (Dec 110) Character 'n' Letter, Lowercase
    QChar(0x006f),  // Hex No. 6F (Dec 111) Character 'o' Letter, Lowercase
    QChar(0x0070),  // Hex No. 70 (Dec 112) Character 'p' Letter, Lowercase
    QChar(0x0071),  // Hex No. 71 (Dec 113) Character 'q' Letter, Lowercase
    QChar(0x0072),  // Hex No. 72 (Dec 114) Character 'r' Letter, Lowercase
    QChar(0x0073),  // Hex No. 73 (Dec 115) Character 's' Letter, Lowercase
    QChar(0x0074),  // Hex No. 74 (Dec 116) Character 't' Letter, Lowercase
    QChar(0x0075),  // Hex No. 75 (Dec 117) Character 'u' Letter, Lowercase
    QChar(0x0076),  // Hex No. 76 (Dec 118) Character 'v' Letter, Lowercase
    QChar(0x0077),  // Hex No. 77 (Dec 119) Character 'w' Letter, Lowercase
    QChar(0x0078),  // Hex No. 78 (Dec 120) Character 'x' Letter, Lowercase
    QChar(0x0079),  // Hex No. 79 (Dec 121) Character 'y' Letter, Lowercase
    QChar(0x007a),  // Hex No. 7A (Dec 122) Character 'z' Letter, Lowercase
    QChar(0x007b),  // Hex No. 7B (Dec 123) Character '{' Punctuation
    QChar(0x007c),  // Hex No. 7C (Dec 124) Character '|' Symbol
    QChar(0x007d),  // Hex No. 7D (Dec 125) Character '}' Punctuation
    QChar(0x007e),  // Hex No. 7E (Dec 126) Character '~' Symbol
    QChar(0xfffd),  // Hex No. 7F (Dec 127) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0x2022),  // Hex No. 80 (Dec 128) Character '•' Punctuation
    QChar(0x2020),  // Hex No. 81 (Dec 129) Character '†' Punctuation
    QChar(0x2021),  // Hex No. 82 (Dec 130) Character '‡' Punctuation
    QChar(0x2026),  // Hex No. 83 (Dec 131) Character '…' Punctuation
    QChar(0x2014),  // Hex No. 84 (Dec 132) Character '—' Punctuation
    QChar(0x2013),  // Hex No. 85 (Dec 133) Character '–' Punctuation
    QChar(0x0192),  // Hex No. 86 (Dec 134) Character 'ƒ' Letter, Lowercase
    QChar(0x2044),  // Hex No. 87 (Dec 135) Character '⁄' Symbol
    QChar(0x2039),  // Hex No. 88 (Dec 136) Character '‹' Punctuation
    QChar(0x203a),  // Hex No. 89 (Dec 137) Character '›' Punctuation
    QChar(0x2212),  // Hex No. 8A (Dec 138) Character '−' Symbol
    QChar(0x2030),  // Hex No. 8B (Dec 139) Character '‰' Punctuation
    QChar(0x201e),  // Hex No. 8C (Dec 140) Character '„' Punctuation
    QChar(0x201c),  // Hex No. 8D (Dec 141) Character '“' Punctuation
    QChar(0x201d),  // Hex No. 8E (Dec 142) Character '”' Punctuation
    QChar(0x2018),  // Hex No. 8F (Dec 143) Character '‘' Punctuation
    QChar(0x2019),  // Hex No. 90 (Dec 144) Character '’' Punctuation
    QChar(0x201a),  // Hex No. 91 (Dec 145) Character '‚' Punctuation
    QChar(0x2122),  // Hex No. 92 (Dec 146) Character '™' Symbol
    QChar(0xfb01),  // Hex No. 93 (Dec 147) Character 'ﬁ' Letter, Lowercase
    QChar(0xfb02),  // Hex No. 94 (Dec 148) Character 'ﬂ' Letter, Lowercase
    QChar(0x0141),  // Hex No. 95 (Dec 149) Character 'Ł' Letter, Uppercase
    QChar(0x0152),  // Hex No. 96 (Dec 150) Character 'Œ' Letter, Uppercase
    QChar(0x0160),  // Hex No. 97 (Dec 151) Character 'Š' Letter, Uppercase
    QChar(0x0178),  // Hex No. 98 (Dec 152) Character 'Ÿ' Letter, Uppercase
    QChar(0x017d),  // Hex No. 99 (Dec 153) Character 'Ž' Letter, Uppercase
    QChar(0x0131),  // Hex No. 9A (Dec 154) Character 'ı' Letter, Lowercase
    QChar(0x0142),  // Hex No. 9B (Dec 155) Character 'ł' Letter, Lowercase
    QChar(0x0153),  // Hex No. 9C (Dec 156) Character 'œ' Letter, Lowercase
    QChar(0x0161),  // Hex No. 9D (Dec 157) Character 'š' Letter, Lowercase
    QChar(0x017e),  // Hex No. 9E (Dec 158) Character 'ž' Letter, Lowercase
    QChar(0xfffd),  // Hex No. 9F (Dec 159) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0x20ac),  // Hex No. A0 (Dec 160) Character '€' Symbol
    QChar(0x00a1),  // Hex No. A1 (Dec 161) Character '¡' Punctuation
    QChar(0x00a2),  // Hex No. A2 (Dec 162) Character '¢' Symbol
    QChar(0x00a3),  // Hex No. A3 (Dec 163) Character '£' Symbol
    QChar(0x00a4),  // Hex No. A4 (Dec 164) Character '¤' Symbol
    QChar(0x00a5),  // Hex No. A5 (Dec 165) Character '¥' Symbol
    QChar(0x00a6),  // Hex No. A6 (Dec 166) Character '¦' Symbol
    QChar(0x00a7),  // Hex No. A7 (Dec 167) Character '§' Punctuation
    QChar(0x00a8),  // Hex No. A8 (Dec 168) Character '¨' Symbol
    QChar(0x00a9),  // Hex No. A9 (Dec 169) Character '©' Symbol
    QChar(0x00aa),  // Hex No. AA (Dec 170) Character 'ª' Letter
    QChar(0x00ab),  // Hex No. AB (Dec 171) Character '«' Punctuation
    QChar(0x00ac),  // Hex No. AC (Dec 172) Character '¬' Symbol
    QChar(0xfffd),  // Hex No. AD (Dec 173) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0x00ae),  // Hex No. AE (Dec 174) Character '®' Symbol
    QChar(0x00af),  // Hex No. AF (Dec 175) Character '¯' Symbol
    QChar(0x00b0),  // Hex No. B0 (Dec 176) Character '°' Symbol
    QChar(0x00b1),  // Hex No. B1 (Dec 177) Character '±' Symbol
    QChar(0x00b2),  // Hex No. B2 (Dec 178) Character '²'
    QChar(0x00b3),  // Hex No. B3 (Dec 179) Character '³'
    QChar(0x00b4),  // Hex No. B4 (Dec 180) Character '´' Symbol
    QChar(0x00b5),  // Hex No. B5 (Dec 181) Character 'µ' Letter, Lowercase
    QChar(0x00b6),  // Hex No. B6 (Dec 182) Character '¶' Punctuation
    QChar(0x00b7),  // Hex No. B7 (Dec 183) Character '·' Punctuation
    QChar(0x00b8),  // Hex No. B8 (Dec 184) Character '¸' Symbol
    QChar(0x00b9),  // Hex No. B9 (Dec 185) Character '¹'
    QChar(0x00ba),  // Hex No. BA (Dec 186) Character 'º' Letter
    QChar(0x00bb),  // Hex No. BB (Dec 187) Character '»' Punctuation
    QChar(0x00bc),  // Hex No. BC (Dec 188) Character '¼'
    QChar(0x00bd),  // Hex No. BD (Dec 189) Character '½'
    QChar(0x00be),  // Hex No. BE (Dec 190) Character '¾'
    QChar(0x00bf),  // Hex No. BF (Dec 191) Character '¿' Punctuation
    QChar(0x00c0),  // Hex No. C0 (Dec 192) Character 'À' Letter, Uppercase
    QChar(0x00c1),  // Hex No. C1 (Dec 193) Character 'Á' Letter, Uppercase
    QChar(0x00c2),  // Hex No. C2 (Dec 194) Character 'Â' Letter, Uppercase
    QChar(0x00c3),  // Hex No. C3 (Dec 195) Character 'Ã' Letter, Uppercase
    QChar(0x00c4),  // Hex No. C4 (Dec 196) Character 'Ä' Letter, Uppercase
    QChar(0x00c5),  // Hex No. C5 (Dec 197) Character 'Å' Letter, Uppercase
    QChar(0x00c6),  // Hex No. C6 (Dec 198) Character 'Æ' Letter, Uppercase
    QChar(0x00c7),  // Hex No. C7 (Dec 199) Character 'Ç' Letter, Uppercase
    QChar(0x00c8),  // Hex No. C8 (Dec 200) Character 'È' Letter, Uppercase
    QChar(0x00c9),  // Hex No. C9 (Dec 201) Character 'É' Letter, Uppercase
    QChar(0x00ca),  // Hex No. CA (Dec 202) Character 'Ê' Letter, Uppercase
    QChar(0x00cb),  // Hex No. CB (Dec 203) Character 'Ë' Letter, Uppercase
    QChar(0x00cc),  // Hex No. CC (Dec 204) Character 'Ì' Letter, Uppercase
    QChar(0x00cd),  // Hex No. CD (Dec 205) Character 'Í' Letter, Uppercase
    QChar(0x00ce),  // Hex No. CE (Dec 206) Character 'Î' Letter, Uppercase
    QChar(0x00cf),  // Hex No. CF (Dec 207) Character 'Ï' Letter, Uppercase
    QChar(0x00d0),  // Hex No. D0 (Dec 208) Character 'Ð' Letter, Uppercase
    QChar(0x00d1),  // Hex No. D1 (Dec 209) Character 'Ñ' Letter, Uppercase
    QChar(0x00d2),  // Hex No. D2 (Dec 210) Character 'Ò' Letter, Uppercase
    QChar(0x00d3),  // Hex No. D3 (Dec 211) Character 'Ó' Letter, Uppercase
    QChar(0x00d4),  // Hex No. D4 (Dec 212) Character 'Ô' Letter, Uppercase
    QChar(0x00d5),  // Hex No. D5 (Dec 213) Character 'Õ' Letter, Uppercase
    QChar(0x00d6),  // Hex No. D6 (Dec 214) Character 'Ö' Letter, Uppercase
    QChar(0x00d7),  // Hex No. D7 (Dec 215) Character '×' Symbol
    QChar(0x00d8),  // Hex No. D8 (Dec 216) Character 'Ø' Letter, Uppercase
    QChar(0x00d9),  // Hex No. D9 (Dec 217) Character 'Ù' Letter, Uppercase
    QChar(0x00da),  // Hex No. DA (Dec 218) Character 'Ú' Letter, Uppercase
    QChar(0x00db),  // Hex No. DB (Dec 219) Character 'Û' Letter, Uppercase
    QChar(0x00dc),  // Hex No. DC (Dec 220) Character 'Ü' Letter, Uppercase
    QChar(0x00dd),  // Hex No. DD (Dec 221) Character 'Ý' Letter, Uppercase
    QChar(0x00de),  // Hex No. DE (Dec 222) Character 'Þ' Letter, Uppercase
    QChar(0x00df),  // Hex No. DF (Dec 223) Character 'ß' Letter, Lowercase
    QChar(0x00e0),  // Hex No. E0 (Dec 224) Character 'à' Letter, Lowercase
    QChar(0x00e1),  // Hex No. E1 (Dec 225) Character 'á' Letter, Lowercase
    QChar(0x00e2),  // Hex No. E2 (Dec 226) Character 'â' Letter, Lowercase
    QChar(0x00e3),  // Hex No. E3 (Dec 227) Character 'ã' Letter, Lowercase
    QChar(0x00e4),  // Hex No. E4 (Dec 228) Character 'ä' Letter, Lowercase
    QChar(0x00e5),  // Hex No. E5 (Dec 229) Character 'å' Letter, Lowercase
    QChar(0x00e6),  // Hex No. E6 (Dec 230) Character 'æ' Letter, Lowercase
    QChar(0x00e7),  // Hex No. E7 (Dec 231) Character 'ç' Letter, Lowercase
    QChar(0x00e8),  // Hex No. E8 (Dec 232) Character 'è' Letter, Lowercase
    QChar(0x00e9),  // Hex No. E9 (Dec 233) Character 'é' Letter, Lowercase
    QChar(0x00ea),  // Hex No. EA (Dec 234) Character 'ê' Letter, Lowercase
    QChar(0x00eb),  // Hex No. EB (Dec 235) Character 'ë' Letter, Lowercase
    QChar(0x00ec),  // Hex No. EC (Dec 236) Character 'ì' Letter, Lowercase
    QChar(0x00ed),  // Hex No. ED (Dec 237) Character 'í' Letter, Lowercase
    QChar(0x00ee),  // Hex No. EE (Dec 238) Character 'î' Letter, Lowercase
    QChar(0x00ef),  // Hex No. EF (Dec 239) Character 'ï' Letter, Lowercase
    QChar(0x00f0),  // Hex No. F0 (Dec 240) Character 'ð' Letter, Lowercase
    QChar(0x00f1),  // Hex No. F1 (Dec 241) Character 'ñ' Letter, Lowercase
    QChar(0x00f2),  // Hex No. F2 (Dec 242) Character 'ò' Letter, Lowercase
    QChar(0x00f3),  // Hex No. F3 (Dec 243) Character 'ó' Letter, Lowercase
    QChar(0x00f4),  // Hex No. F4 (Dec 244) Character 'ô' Letter, Lowercase
    QChar(0x00f5),  // Hex No. F5 (Dec 245) Character 'õ' Letter, Lowercase
    QChar(0x00f6),  // Hex No. F6 (Dec 246) Character 'ö' Letter, Lowercase
    QChar(0x00f7),  // Hex No. F7 (Dec 247) Character '÷' Symbol
    QChar(0x00f8),  // Hex No. F8 (Dec 248) Character 'ø' Letter, Lowercase
    QChar(0x00f9),  // Hex No. F9 (Dec 249) Character 'ù' Letter, Lowercase
    QChar(0x00fa),  // Hex No. FA (Dec 250) Character 'ú' Letter, Lowercase
    QChar(0x00fb),  // Hex No. FB (Dec 251) Character 'û' Letter, Lowercase
    QChar(0x00fc),  // Hex No. FC (Dec 252) Character 'ü' Letter, Lowercase
    QChar(0x00fd),  // Hex No. FD (Dec 253) Character 'ý' Letter, Lowercase
    QChar(0x00fe),  // Hex No. FE (Dec 254) Character 'þ' Letter, Lowercase
    QChar(0x00ff),  // Hex No. FF (Dec 255) Character 'ÿ' Letter, Lowercase
};

// PDF Reference 1.7, Appendix D, Section D.3, MacExpertEncoding
static const EncodingTable MAC_EXPERT_ENCODING_CONVERSION_TABLE = {
    QChar(0xfffd),  // Hex No. 00 (Dec 000) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 01 (Dec 001) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 02 (Dec 002) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 03 (Dec 003) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 04 (Dec 004) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 05 (Dec 005) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 06 (Dec 006) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 07 (Dec 007) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 08 (Dec 008) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 09 (Dec 009) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 0A (Dec 010) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 0B (Dec 011) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 0C (Dec 012) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 0D (Dec 013) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 0E (Dec 014) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 0F (Dec 015) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 10 (Dec 016) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 11 (Dec 017) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 12 (Dec 018) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 13 (Dec 019) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 14 (Dec 020) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 15 (Dec 021) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 16 (Dec 022) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 17 (Dec 023) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 18 (Dec 024) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 19 (Dec 025) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 1A (Dec 026) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 1B (Dec 027) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 1C (Dec 028) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 1D (Dec 029) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 1E (Dec 030) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 1F (Dec 031) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0x0020),  // Hex No. 20 (Dec 032) Character ' ' Whitespace
    QChar(0xf721),  // Hex No. 21 (Dec 033)
    QChar(0xf6f8),  // Hex No. 22 (Dec 034)
    QChar(0xf7a2),  // Hex No. 23 (Dec 035)
    QChar(0xf724),  // Hex No. 24 (Dec 036)
    QChar(0xf6e4),  // Hex No. 25 (Dec 037)
    QChar(0xf726),  // Hex No. 26 (Dec 038)
    QChar(0xf7b4),  // Hex No. 27 (Dec 039)
    QChar(0x207d),  // Hex No. 28 (Dec 040) Character '⁽' Punctuation
    QChar(0x207e),  // Hex No. 29 (Dec 041) Character '⁾' Punctuation
    QChar(0x2025),  // Hex No. 2A (Dec 042) Character '‥' Punctuation
    QChar(0x2024),  // Hex No. 2B (Dec 043) Character '․' Punctuation
    QChar(0x002c),  // Hex No. 2C (Dec 044) Character ',' Punctuation
    QChar(0x002d),  // Hex No. 2D (Dec 045) Character '-' Punctuation
    QChar(0x002e),  // Hex No. 2E (Dec 046) Character '.' Punctuation
    QChar(0x2044),  // Hex No. 2F (Dec 047) Character '⁄' Symbol
    QChar(0xf730),  // Hex No. 30 (Dec 048)
    QChar(0xf731),  // Hex No. 31 (Dec 049)
    QChar(0xf732),  // Hex No. 32 (Dec 050)
    QChar(0xf733),  // Hex No. 33 (Dec 051)
    QChar(0xf734),  // Hex No. 34 (Dec 052)
    QChar(0xf735),  // Hex No. 35 (Dec 053)
    QChar(0xf736),  // Hex No. 36 (Dec 054)
    QChar(0xf737),  // Hex No. 37 (Dec 055)
    QChar(0xf738),  // Hex No. 38 (Dec 056)
    QChar(0xf739),  // Hex No. 39 (Dec 057)
    QChar(0x003a),  // Hex No. 3A (Dec 058) Character ':' Punctuation
    QChar(0x003b),  // Hex No. 3B (Dec 059) Character ';' Punctuation
    QChar(0xfffd),  // Hex No. 3C (Dec 060) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xf6de),  // Hex No. 3D (Dec 061)
    QChar(0xfffd),  // Hex No. 3E (Dec 062) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xf73f),  // Hex No. 3F (Dec 063)
    QChar(0xfffd),  // Hex No. 40 (Dec 064) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 41 (Dec 065) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 42 (Dec 066) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 43 (Dec 067) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xf7f0),  // Hex No. 44 (Dec 068)
    QChar(0xfffd),  // Hex No. 45 (Dec 069) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 46 (Dec 070) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0x00bc),  // Hex No. 47 (Dec 071) Character '¼'
    QChar(0x00bd),  // Hex No. 48 (Dec 072) Character '½'
    QChar(0x00be),  // Hex No. 49 (Dec 073) Character '¾'
    QChar(0x215b),  // Hex No. 4A (Dec 074) Character '⅛'
    QChar(0x215c),  // Hex No. 4B (Dec 075) Character '⅜'
    QChar(0x215d),  // Hex No. 4C (Dec 076) Character '⅝'
    QChar(0x215e),  // Hex No. 4D (Dec 077) Character '⅞'
    QChar(0x2153),  // Hex No. 4E (Dec 078) Character '⅓'
    QChar(0x2154),  // Hex No. 4F (Dec 079) Character '⅔'
    QChar(0xfffd),  // Hex No. 50 (Dec 080) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 51 (Dec 081) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 52 (Dec 082) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 53 (Dec 083) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 54 (Dec 084) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 55 (Dec 085) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfb00),  // Hex No. 56 (Dec 086) Character 'ﬀ' Letter, Lowercase
    QChar(0xfb01),  // Hex No. 57 (Dec 087) Character 'ﬁ' Letter, Lowercase
    QChar(0xfb02),  // Hex No. 58 (Dec 088) Character 'ﬂ' Letter, Lowercase
    QChar(0xfb03),  // Hex No. 59 (Dec 089) Character 'ﬃ' Letter, Lowercase
    QChar(0xfb04),  // Hex No. 5A (Dec 090) Character 'ﬄ' Letter, Lowercase
    QChar(0x208d),  // Hex No. 5B (Dec 091) Character '₍' Punctuation
    QChar(0xfffd),  // Hex No. 5C (Dec 092) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0x208e),  // Hex No. 5D (Dec 093) Character '₎' Punctuation
    QChar(0xf6f6),  // Hex No. 5E (Dec 094)
    QChar(0xf6e5),  // Hex No. 5F (Dec 095)
    QChar(0xf760),  // Hex No. 60 (Dec 096)
    QChar(0xf761),  // Hex No. 61 (Dec 097)
    QChar(0xf762),  // Hex No. 62 (Dec 098)
    QChar(0xf763),  // Hex No. 63 (Dec 099)
    QChar(0xf764),  // Hex No. 64 (Dec 100)
    QChar(0xf765),  // Hex No. 65 (Dec 101)
    QChar(0xf766),  // Hex No. 66 (Dec 102)
    QChar(0xf767),  // Hex No. 67 (Dec 103)
    QChar(0xf768),  // Hex No. 68 (Dec 104)
    QChar(0xf769),  // Hex No. 69 (Dec 105)
    QChar(0xf76a),  // Hex No. 6A (Dec 106)
    QChar(0xf76b),  // Hex No. 6B (Dec 107)
    QChar(0xf76c),  // Hex No. 6C (Dec 108)
    QChar(0xf76d),  // Hex No. 6D (Dec 109)
    QChar(0xf76e),  // Hex No. 6E (Dec 110)
    QChar(0xf76f),  // Hex No. 6F (Dec 111)
    QChar(0xf770),  // Hex No. 70 (Dec 112)
    QChar(0xf771),  // Hex No. 71 (Dec 113)
    QChar(0xf772),  // Hex No. 72 (Dec 114)
    QChar(0xf773),  // Hex No. 73 (Dec 115)
    QChar(0xf774),  // Hex No. 74 (Dec 116)
    QChar(0xf775),  // Hex No. 75 (Dec 117)
    QChar(0xf776),  // Hex No. 76 (Dec 118)
    QChar(0xf777),  // Hex No. 77 (Dec 119)
    QChar(0xf778),  // Hex No. 78 (Dec 120)
    QChar(0xf779),  // Hex No. 79 (Dec 121)
    QChar(0xf77a),  // Hex No. 7A (Dec 122)
    QChar(0x20a1),  // Hex No. 7B (Dec 123) Character '₡' Symbol
    QChar(0xf6dc),  // Hex No. 7C (Dec 124)
    QChar(0xf6dd),  // Hex No. 7D (Dec 125)
    QChar(0xf6fe),  // Hex No. 7E (Dec 126)
    QChar(0xfffd),  // Hex No. 7F (Dec 127) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 80 (Dec 128) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xf6e9),  // Hex No. 81 (Dec 129)
    QChar(0xf6e0),  // Hex No. 82 (Dec 130)
    QChar(0xfffd),  // Hex No. 83 (Dec 131) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 84 (Dec 132) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 85 (Dec 133) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 86 (Dec 134) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xf7e1),  // Hex No. 87 (Dec 135)
    QChar(0xf7e0),  // Hex No. 88 (Dec 136)
    QChar(0xf7e2),  // Hex No. 89 (Dec 137)
    QChar(0xf7e4),  // Hex No. 8A (Dec 138)
    QChar(0xf7e3),  // Hex No. 8B (Dec 139)
    QChar(0xf7e5),  // Hex No. 8C (Dec 140)
    QChar(0xf7e7),  // Hex No. 8D (Dec 141)
    QChar(0xf7e9),  // Hex No. 8E (Dec 142)
    QChar(0xf7e8),  // Hex No. 8F (Dec 143)
    QChar(0xf7ea),  // Hex No. 90 (Dec 144)
    QChar(0xf7eb),  // Hex No. 91 (Dec 145)
    QChar(0xf7ed),  // Hex No. 92 (Dec 146)
    QChar(0xf7ec),  // Hex No. 93 (Dec 147)
    QChar(0xf7ee),  // Hex No. 94 (Dec 148)
    QChar(0xf7ef),  // Hex No. 95 (Dec 149)
    QChar(0xf7f1),  // Hex No. 96 (Dec 150)
    QChar(0xf7f3),  // Hex No. 97 (Dec 151)
    QChar(0xf7f2),  // Hex No. 98 (Dec 152)
    QChar(0xf7f4),  // Hex No. 99 (Dec 153)
    QChar(0xf7f6),  // Hex No. 9A (Dec 154)
    QChar(0xf7f5),  // Hex No. 9B (Dec 155)
    QChar(0xf7fa),  // Hex No. 9C (Dec 156)
    QChar(0xf7f9),  // Hex No. 9D (Dec 157)
    QChar(0xf7fb),  // Hex No. 9E (Dec 158)
    QChar(0xf7fc),  // Hex No. 9F (Dec 159)
    QChar(0xfffd),  // Hex No. A0 (Dec 160) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0x2078),  // Hex No. A1 (Dec 161) Character '⁸'
    QChar(0x2084),  // Hex No. A2 (Dec 162) Character '₄'
    QChar(0x2083),  // Hex No. A3 (Dec 163) Character '₃'
    QChar(0x2086),  // Hex No. A4 (Dec 164) Character '₆'
    QChar(0x2088),  // Hex No. A5 (Dec 165) Character '₈'
    QChar(0x2087),  // Hex No. A6 (Dec 166) Character '₇'
    QChar(0xf6fd),  // Hex No. A7 (Dec 167)
    QChar(0xfffd),  // Hex No. A8 (Dec 168) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xf6df),  // Hex No. A9 (Dec 169)
    QChar(0x2082),  // Hex No. AA (Dec 170) Character '₂'
    QChar(0xfffd),  // Hex No. AB (Dec 171) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xf7a8),  // Hex No. AC (Dec 172)
    QChar(0xfffd),  // Hex No. AD (Dec 173) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xf6f5),  // Hex No. AE (Dec 174)
    QChar(0xf6f0),  // Hex No. AF (Dec 175)
    QChar(0x2085),  // Hex No. B0 (Dec 176) Character '₅'
    QChar(0xfffd),  // Hex No. B1 (Dec 177) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xf6e1),  // Hex No. B2 (Dec 178)
    QChar(0xf6e7),  // Hex No. B3 (Dec 179)
    QChar(0xf7fd),  // Hex No. B4 (Dec 180)
    QChar(0xfffd),  // Hex No. B5 (Dec 181) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xf6e3),  // Hex No. B6 (Dec 182)
    QChar(0xfffd),  // Hex No. B7 (Dec 183) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. B8 (Dec 184) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xf7fe),  // Hex No. B9 (Dec 185)
    QChar(0xfffd),  // Hex No. BA (Dec 186) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0x2089),  // Hex No. BB (Dec 187) Character '₉'
    QChar(0x2080),  // Hex No. BC (Dec 188) Character '₀'
    QChar(0xf6ff),  // Hex No. BD (Dec 189)
    QChar(0xf7e6),  // Hex No. BE (Dec 190)
    QChar(0xf7f8),  // Hex No. BF (Dec 191)
    QChar(0xf7bf),  // Hex No. C0 (Dec 192)
    QChar(0x2081),  // Hex No. C1 (Dec 193) Character '₁'
    QChar(0xf6f9),  // Hex No. C2 (Dec 194)
    QChar(0xfffd),  // Hex No. C3 (Dec 195) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. C4 (Dec 196) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. C5 (Dec 197) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. C6 (Dec 198) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. C7 (Dec 199) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. C8 (Dec 200) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xf7b8),  // Hex No. C9 (Dec 201)
    QChar(0xfffd),  // Hex No. CA (Dec 202) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. CB (Dec 203) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. CC (Dec 204) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. CD (Dec 205) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. CE (Dec 206) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xf6fa),  // Hex No. CF (Dec 207)
    QChar(0x2012),  // Hex No. D0 (Dec 208) Character '‒' Punctuation
    QChar(0xf6e6),  // Hex No. D1 (Dec 209)
    QChar(0xfffd),  // Hex No. D2 (Dec 210) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. D3 (Dec 211) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. D4 (Dec 212) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. D5 (Dec 213) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xf7a1),  // Hex No. D6 (Dec 214)
    QChar(0xfffd),  // Hex No. D7 (Dec 215) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xf7ff),  // Hex No. D8 (Dec 216)
    QChar(0xfffd),  // Hex No. D9 (Dec 217) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0x00b9),  // Hex No. DA (Dec 218) Character '¹'
    QChar(0x00b2),  // Hex No. DB (Dec 219) Character '²'
    QChar(0x00b3),  // Hex No. DC (Dec 220) Character '³'
    QChar(0x2074),  // Hex No. DD (Dec 221) Character '⁴'
    QChar(0x2075),  // Hex No. DE (Dec 222) Character '⁵'
    QChar(0x2076),  // Hex No. DF (Dec 223) Character '⁶'
    QChar(0x2077),  // Hex No. E0 (Dec 224) Character '⁷'
    QChar(0x2079),  // Hex No. E1 (Dec 225) Character '⁹'
    QChar(0x2070),  // Hex No. E2 (Dec 226) Character '⁰'
    QChar(0xfffd),  // Hex No. E3 (Dec 227) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xf6ec),  // Hex No. E4 (Dec 228)
    QChar(0xf6f1),  // Hex No. E5 (Dec 229)
    QChar(0xf6f3),  // Hex No. E6 (Dec 230)
    QChar(0xfffd),  // Hex No. E7 (Dec 231) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. E8 (Dec 232) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xf6ed),  // Hex No. E9 (Dec 233)
    QChar(0xf6f2),  // Hex No. EA (Dec 234)
    QChar(0xf6eb),  // Hex No. EB (Dec 235)
    QChar(0xfffd),  // Hex No. EC (Dec 236) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. ED (Dec 237) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. EE (Dec 238) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. EF (Dec 239) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. F0 (Dec 240) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xf6ee),  // Hex No. F1 (Dec 241)
    QChar(0xf6fb),  // Hex No. F2 (Dec 242)
    QChar(0xf6f4),  // Hex No. F3 (Dec 243)
    QChar(0xf7af),  // Hex No. F4 (Dec 244)
    QChar(0xf6ea),  // Hex No. F5 (Dec 245)
    QChar(0x207f),  // Hex No. F6 (Dec 246) Character 'ⁿ' Letter
    QChar(0xf6ef),  // Hex No. F7 (Dec 247)
    QChar(0xf6e2),  // Hex No. F8 (Dec 248)
    QChar(0xf6e8),  // Hex No. F9 (Dec 249)
    QChar(0xf6f7),  // Hex No. FA (Dec 250)
    QChar(0xf6fc),  // Hex No. FB (Dec 251)
    QChar(0xfffd),  // Hex No. FC (Dec 252) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. FD (Dec 253) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. FE (Dec 254) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. FF (Dec 255) REPLACEMENT CHARACTER 0xFFFD - not present in character set
};

// PDF Reference 1.7, Appendix D, Section D.4, Symbol Set and Encoding
static const EncodingTable SYMBOL_SET_ENCODING_CONVERSION_TABLE = {
    QChar(0xfffd),  // Hex No. 00 (Dec 000) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 01 (Dec 001) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 02 (Dec 002) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 03 (Dec 003) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 04 (Dec 004) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 05 (Dec 005) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 06 (Dec 006) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 07 (Dec 007) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 08 (Dec 008) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 09 (Dec 009) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 0A (Dec 010) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 0B (Dec 011) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 0C (Dec 012) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 0D (Dec 013) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 0E (Dec 014) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 0F (Dec 015) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 10 (Dec 016) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 11 (Dec 017) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 12 (Dec 018) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 13 (Dec 019) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 14 (Dec 020) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 15 (Dec 021) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 16 (Dec 022) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 17 (Dec 023) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 18 (Dec 024) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 19 (Dec 025) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 1A (Dec 026) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 1B (Dec 027) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 1C (Dec 028) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 1D (Dec 029) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 1E (Dec 030) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 1F (Dec 031) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0x0020),  // Hex No. 20 (Dec 032) Character ' ' Whitespace
    QChar(0x0021),  // Hex No. 21 (Dec 033) Character '!' Punctuation
    QChar(0x2200),  // Hex No. 22 (Dec 034) Character '∀' Symbol
    QChar(0x0023),  // Hex No. 23 (Dec 035) Character '#' Punctuation
    QChar(0x2203),  // Hex No. 24 (Dec 036) Character '∃' Symbol
    QChar(0x0025),  // Hex No. 25 (Dec 037) Character '%' Punctuation
    QChar(0x0026),  // Hex No. 26 (Dec 038) Character '&' Punctuation
    QChar(0x220b),  // Hex No. 27 (Dec 039) Character '∋' Symbol
    QChar(0x0028),  // Hex No. 28 (Dec 040) Character '(' Punctuation
    QChar(0x0029),  // Hex No. 29 (Dec 041) Character ')' Punctuation
    QChar(0x2217),  // Hex No. 2A (Dec 042) Character '∗' Symbol
    QChar(0x002b),  // Hex No. 2B (Dec 043) Character '+' Symbol
    QChar(0x002c),  // Hex No. 2C (Dec 044) Character ',' Punctuation
    QChar(0x2212),  // Hex No. 2D (Dec 045) Character '−' Symbol
    QChar(0x002e),  // Hex No. 2E (Dec 046) Character '.' Punctuation
    QChar(0x002f),  // Hex No. 2F (Dec 047) Character '/' Punctuation
    QChar(0x0030),  // Hex No. 30 (Dec 048) Character '0' Digit
    QChar(0x0031),  // Hex No. 31 (Dec 049) Character '1' Digit
    QChar(0x0032),  // Hex No. 32 (Dec 050) Character '2' Digit
    QChar(0x0033),  // Hex No. 33 (Dec 051) Character '3' Digit
    QChar(0x0034),  // Hex No. 34 (Dec 052) Character '4' Digit
    QChar(0x0035),  // Hex No. 35 (Dec 053) Character '5' Digit
    QChar(0x0036),  // Hex No. 36 (Dec 054) Character '6' Digit
    QChar(0x0037),  // Hex No. 37 (Dec 055) Character '7' Digit
    QChar(0x0038),  // Hex No. 38 (Dec 056) Character '8' Digit
    QChar(0x0039),  // Hex No. 39 (Dec 057) Character '9' Digit
    QChar(0x003a),  // Hex No. 3A (Dec 058) Character ':' Punctuation
    QChar(0x003b),  // Hex No. 3B (Dec 059) Character ';' Punctuation
    QChar(0x003c),  // Hex No. 3C (Dec 060) Character '<' Symbol
    QChar(0x003d),  // Hex No. 3D (Dec 061) Character '=' Symbol
    QChar(0x003e),  // Hex No. 3E (Dec 062) Character '>' Symbol
    QChar(0x003f),  // Hex No. 3F (Dec 063) Character '?' Punctuation
    QChar(0x2245),  // Hex No. 40 (Dec 064) Character '≅' Symbol
    QChar(0x0391),  // Hex No. 41 (Dec 065) Character 'Α' Letter, Uppercase
    QChar(0x0392),  // Hex No. 42 (Dec 066) Character 'Β' Letter, Uppercase
    QChar(0x03a7),  // Hex No. 43 (Dec 067) Character 'Χ' Letter, Uppercase
    QChar(0x2206),  // Hex No. 44 (Dec 068) Character '∆' Symbol
    QChar(0x0395),  // Hex No. 45 (Dec 069) Character 'Ε' Letter, Uppercase
    QChar(0x03a6),  // Hex No. 46 (Dec 070) Character 'Φ' Letter, Uppercase
    QChar(0x0393),  // Hex No. 47 (Dec 071) Character 'Γ' Letter, Uppercase
    QChar(0x0397),  // Hex No. 48 (Dec 072) Character 'Η' Letter, Uppercase
    QChar(0x0399),  // Hex No. 49 (Dec 073) Character 'Ι' Letter, Uppercase
    QChar(0x03d1),  // Hex No. 4A (Dec 074) Character 'ϑ' Letter, Lowercase
    QChar(0x039a),  // Hex No. 4B (Dec 075) Character 'Κ' Letter, Uppercase
    QChar(0x039b),  // Hex No. 4C (Dec 076) Character 'Λ' Letter, Uppercase
    QChar(0x039c),  // Hex No. 4D (Dec 077) Character 'Μ' Letter, Uppercase
    QChar(0x039d),  // Hex No. 4E (Dec 078) Character 'Ν' Letter, Uppercase
    QChar(0x039f),  // Hex No. 4F (Dec 079) Character 'Ο' Letter, Uppercase
    QChar(0x03a0),  // Hex No. 50 (Dec 080) Character 'Π' Letter, Uppercase
    QChar(0x0398),  // Hex No. 51 (Dec 081) Character 'Θ' Letter, Uppercase
    QChar(0x03a1),  // Hex No. 52 (Dec 082) Character 'Ρ' Letter, Uppercase
    QChar(0x03a3),  // Hex No. 53 (Dec 083) Character 'Σ' Letter, Uppercase
    QChar(0x03a4),  // Hex No. 54 (Dec 084) Character 'Τ' Letter, Uppercase
    QChar(0x03a5),  // Hex No. 55 (Dec 085) Character 'Υ' Letter, Uppercase
    QChar(0x03c2),  // Hex No. 56 (Dec 086) Character 'ς' Letter, Lowercase
    QChar(0x2126),  // Hex No. 57 (Dec 087) Character 'Ω' Letter, Uppercase
    QChar(0x039e),  // Hex No. 58 (Dec 088) Character 'Ξ' Letter, Uppercase
    QChar(0x03a8),  // Hex No. 59 (Dec 089) Character 'Ψ' Letter, Uppercase
    QChar(0x0396),  // Hex No. 5A (Dec 090) Character 'Ζ' Letter, Uppercase
    QChar(0x005b),  // Hex No. 5B (Dec 091) Character '[' Punctuation
    QChar(0x2234),  // Hex No. 5C (Dec 092) Character '∴' Symbol
    QChar(0x005d),  // Hex No. 5D (Dec 093) Character ']' Punctuation
    QChar(0x22a5),  // Hex No. 5E (Dec 094) Character '⊥' Symbol
    QChar(0x005f),  // Hex No. 5F (Dec 095) Character '_' Punctuation
    QChar(0xf8e5),  // Hex No. 60 (Dec 096)
    QChar(0x03b1),  // Hex No. 61 (Dec 097) Character 'α' Letter, Lowercase
    QChar(0x03b2),  // Hex No. 62 (Dec 098) Character 'β' Letter, Lowercase
    QChar(0x03c7),  // Hex No. 63 (Dec 099) Character 'χ' Letter, Lowercase
    QChar(0x03b4),  // Hex No. 64 (Dec 100) Character 'δ' Letter, Lowercase
    QChar(0x03b5),  // Hex No. 65 (Dec 101) Character 'ε' Letter, Lowercase
    QChar(0x03c6),  // Hex No. 66 (Dec 102) Character 'φ' Letter, Lowercase
    QChar(0x03b3),  // Hex No. 67 (Dec 103) Character 'γ' Letter, Lowercase
    QChar(0x03b7),  // Hex No. 68 (Dec 104) Character 'η' Letter, Lowercase
    QChar(0x03b9),  // Hex No. 69 (Dec 105) Character 'ι' Letter, Lowercase
    QChar(0x03d5),  // Hex No. 6A (Dec 106) Character 'ϕ' Letter, Lowercase
    QChar(0x03ba),  // Hex No. 6B (Dec 107) Character 'κ' Letter, Lowercase
    QChar(0x03bb),  // Hex No. 6C (Dec 108) Character 'λ' Letter, Lowercase
    QChar(0x00b5),  // Hex No. 6D (Dec 109) Character 'µ' Letter, Lowercase
    QChar(0x03bd),  // Hex No. 6E (Dec 110) Character 'ν' Letter, Lowercase
    QChar(0x03bf),  // Hex No. 6F (Dec 111) Character 'ο' Letter, Lowercase
    QChar(0x03c0),  // Hex No. 70 (Dec 112) Character 'π' Letter, Lowercase
    QChar(0x03b8),  // Hex No. 71 (Dec 113) Character 'θ' Letter, Lowercase
    QChar(0x03c1),  // Hex No. 72 (Dec 114) Character 'ρ' Letter, Lowercase
    QChar(0x03c3),  // Hex No. 73 (Dec 115) Character 'σ' Letter, Lowercase
    QChar(0x03c4),  // Hex No. 74 (Dec 116) Character 'τ' Letter, Lowercase
    QChar(0x03c5),  // Hex No. 75 (Dec 117) Character 'υ' Letter, Lowercase
    QChar(0x03d6),  // Hex No. 76 (Dec 118) Character 'ϖ' Letter, Lowercase
    QChar(0x03c9),  // Hex No. 77 (Dec 119) Character 'ω' Letter, Lowercase
    QChar(0x03be),  // Hex No. 78 (Dec 120) Character 'ξ' Letter, Lowercase
    QChar(0x03c8),  // Hex No. 79 (Dec 121) Character 'ψ' Letter, Lowercase
    QChar(0x03b6),  // Hex No. 7A (Dec 122) Character 'ζ' Letter, Lowercase
    QChar(0x007b),  // Hex No. 7B (Dec 123) Character '{' Punctuation
    QChar(0x007c),  // Hex No. 7C (Dec 124) Character '|' Symbol
    QChar(0x007d),  // Hex No. 7D (Dec 125) Character '}' Punctuation
    QChar(0x223c),  // Hex No. 7E (Dec 126) Character '∼' Symbol
    QChar(0xfffd),  // Hex No. 7F (Dec 127) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 80 (Dec 128) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 81 (Dec 129) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 82 (Dec 130) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 83 (Dec 131) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 84 (Dec 132) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 85 (Dec 133) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 86 (Dec 134) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 87 (Dec 135) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 88 (Dec 136) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 89 (Dec 137) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 8A (Dec 138) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 8B (Dec 139) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 8C (Dec 140) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 8D (Dec 141) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 8E (Dec 142) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 8F (Dec 143) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 90 (Dec 144) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 91 (Dec 145) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 92 (Dec 146) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 93 (Dec 147) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 94 (Dec 148) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 95 (Dec 149) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 96 (Dec 150) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 97 (Dec 151) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 98 (Dec 152) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 99 (Dec 153) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 9A (Dec 154) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 9B (Dec 155) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 9C (Dec 156) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 9D (Dec 157) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 9E (Dec 158) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 9F (Dec 159) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. A0 (Dec 160) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0x03d2),  // Hex No. A1 (Dec 161) Character 'ϒ' Letter, Uppercase
    QChar(0x2032),  // Hex No. A2 (Dec 162) Character '′' Punctuation
    QChar(0x2264),  // Hex No. A3 (Dec 163) Character '≤' Symbol
    QChar(0x2044),  // Hex No. A4 (Dec 164) Character '⁄' Symbol
    QChar(0x221e),  // Hex No. A5 (Dec 165) Character '∞' Symbol
    QChar(0x0192),  // Hex No. A6 (Dec 166) Character 'ƒ' Letter, Lowercase
    QChar(0x2663),  // Hex No. A7 (Dec 167) Character '♣' Symbol
    QChar(0x2666),  // Hex No. A8 (Dec 168) Character '♦' Symbol
    QChar(0x2665),  // Hex No. A9 (Dec 169) Character '♥' Symbol
    QChar(0x2660),  // Hex No. AA (Dec 170) Character '♠' Symbol
    QChar(0x2194),  // Hex No. AB (Dec 171) Character '↔' Symbol
    QChar(0x2190),  // Hex No. AC (Dec 172) Character '←' Symbol
    QChar(0x2191),  // Hex No. AD (Dec 173) Character '↑' Symbol
    QChar(0x2192),  // Hex No. AE (Dec 174) Character '→' Symbol
    QChar(0x2193),  // Hex No. AF (Dec 175) Character '↓' Symbol
    QChar(0x00b0),  // Hex No. B0 (Dec 176) Character '°' Symbol
    QChar(0x00b1),  // Hex No. B1 (Dec 177) Character '±' Symbol
    QChar(0x2033),  // Hex No. B2 (Dec 178) Character '″' Punctuation
    QChar(0x2265),  // Hex No. B3 (Dec 179) Character '≥' Symbol
    QChar(0x00d7),  // Hex No. B4 (Dec 180) Character '×' Symbol
    QChar(0x221d),  // Hex No. B5 (Dec 181) Character '∝' Symbol
    QChar(0x2202),  // Hex No. B6 (Dec 182) Character '∂' Symbol
    QChar(0x2022),  // Hex No. B7 (Dec 183) Character '•' Punctuation
    QChar(0x00f7),  // Hex No. B8 (Dec 184) Character '÷' Symbol
    QChar(0x2260),  // Hex No. B9 (Dec 185) Character '≠' Symbol
    QChar(0x2261),  // Hex No. BA (Dec 186) Character '≡' Symbol
    QChar(0x2248),  // Hex No. BB (Dec 187) Character '≈' Symbol
    QChar(0x2026),  // Hex No. BC (Dec 188) Character '…' Punctuation
    QChar(0xf8e6),  // Hex No. BD (Dec 189)
    QChar(0xf8e7),  // Hex No. BE (Dec 190)
    QChar(0x21b5),  // Hex No. BF (Dec 191) Character '↵' Symbol
    QChar(0x2135),  // Hex No. C0 (Dec 192) Character 'ℵ' Letter
    QChar(0x2111),  // Hex No. C1 (Dec 193) Character 'ℑ' Letter, Uppercase
    QChar(0x211c),  // Hex No. C2 (Dec 194) Character 'ℜ' Letter, Uppercase
    QChar(0x2118),  // Hex No. C3 (Dec 195) Character '℘' Symbol
    QChar(0x2297),  // Hex No. C4 (Dec 196) Character '⊗' Symbol
    QChar(0x2295),  // Hex No. C5 (Dec 197) Character '⊕' Symbol
    QChar(0x2205),  // Hex No. C6 (Dec 198) Character '∅' Symbol
    QChar(0x2229),  // Hex No. C7 (Dec 199) Character '∩' Symbol
    QChar(0x222a),  // Hex No. C8 (Dec 200) Character '∪' Symbol
    QChar(0x2283),  // Hex No. C9 (Dec 201) Character '⊃' Symbol
    QChar(0x2287),  // Hex No. CA (Dec 202) Character '⊇' Symbol
    QChar(0x2284),  // Hex No. CB (Dec 203) Character '⊄' Symbol
    QChar(0x2282),  // Hex No. CC (Dec 204) Character '⊂' Symbol
    QChar(0x2286),  // Hex No. CD (Dec 205) Character '⊆' Symbol
    QChar(0x2208),  // Hex No. CE (Dec 206) Character '∈' Symbol
    QChar(0x2209),  // Hex No. CF (Dec 207) Character '∉' Symbol
    QChar(0x2220),  // Hex No. D0 (Dec 208) Character '∠' Symbol
    QChar(0x2207),  // Hex No. D1 (Dec 209) Character '∇' Symbol
    QChar(0xf6da),  // Hex No. D2 (Dec 210)
    QChar(0xf6d9),  // Hex No. D3 (Dec 211)
    QChar(0xf6db),  // Hex No. D4 (Dec 212)
    QChar(0x220f),  // Hex No. D5 (Dec 213) Character '∏' Symbol
    QChar(0x221a),  // Hex No. D6 (Dec 214) Character '√' Symbol
    QChar(0x22c5),  // Hex No. D7 (Dec 215) Character '⋅' Symbol
    QChar(0x00ac),  // Hex No. D8 (Dec 216) Character '¬' Symbol
    QChar(0x2227),  // Hex No. D9 (Dec 217) Character '∧' Symbol
    QChar(0x2228),  // Hex No. DA (Dec 218) Character '∨' Symbol
    QChar(0x21d4),  // Hex No. DB (Dec 219) Character '⇔' Symbol
    QChar(0x21d0),  // Hex No. DC (Dec 220) Character '⇐' Symbol
    QChar(0x21d1),  // Hex No. DD (Dec 221) Character '⇑' Symbol
    QChar(0x21d2),  // Hex No. DE (Dec 222) Character '⇒' Symbol
    QChar(0x21d3),  // Hex No. DF (Dec 223) Character '⇓' Symbol
    QChar(0x25ca),  // Hex No. E0 (Dec 224) Character '◊' Symbol
    QChar(0x2329),  // Hex No. E1 (Dec 225) Character '〈' Punctuation
    QChar(0xf8e8),  // Hex No. E2 (Dec 226)
    QChar(0xf8e9),  // Hex No. E3 (Dec 227)
    QChar(0xf8ea),  // Hex No. E4 (Dec 228)
    QChar(0x2211),  // Hex No. E5 (Dec 229) Character '∑' Symbol
    QChar(0xf8eb),  // Hex No. E6 (Dec 230)
    QChar(0xf8ec),  // Hex No. E7 (Dec 231)
    QChar(0xf8ed),  // Hex No. E8 (Dec 232)
    QChar(0xf8ee),  // Hex No. E9 (Dec 233)
    QChar(0xf8ef),  // Hex No. EA (Dec 234)
    QChar(0xf8f0),  // Hex No. EB (Dec 235)
    QChar(0xf8f1),  // Hex No. EC (Dec 236)
    QChar(0xf8f2),  // Hex No. ED (Dec 237)
    QChar(0xf8f3),  // Hex No. EE (Dec 238)
    QChar(0xf8f4),  // Hex No. EF (Dec 239)
    QChar(0xfffd),  // Hex No. F0 (Dec 240) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0x232a),  // Hex No. F1 (Dec 241) Character '〉' Punctuation
    QChar(0x222b),  // Hex No. F2 (Dec 242) Character '∫' Symbol
    QChar(0x2320),  // Hex No. F3 (Dec 243) Character '⌠' Symbol
    QChar(0xf8f5),  // Hex No. F4 (Dec 244)
    QChar(0x2321),  // Hex No. F5 (Dec 245) Character '⌡' Symbol
    QChar(0xf8f6),  // Hex No. F6 (Dec 246)
    QChar(0xf8f7),  // Hex No. F7 (Dec 247)
    QChar(0xf8f8),  // Hex No. F8 (Dec 248)
    QChar(0xf8f9),  // Hex No. F9 (Dec 249)
    QChar(0xf8fa),  // Hex No. FA (Dec 250)
    QChar(0xf8fb),  // Hex No. FB (Dec 251)
    QChar(0xf8fc),  // Hex No. FC (Dec 252)
    QChar(0xf8fd),  // Hex No. FD (Dec 253)
    QChar(0xf8fe),  // Hex No. FE (Dec 254)
    QChar(0xfffd),  // Hex No. FF (Dec 255) REPLACEMENT CHARACTER 0xFFFD - not present in character set
};

// PDF Reference 1.7, Appendix D, Section D.5, Zapf Dingbats Set and Encoding
static const EncodingTable ZAPF_DINGBATS_ENCODING_CONVERSION_TABLE = {
    QChar(0xfffd),  // Hex No. 00 (Dec 000) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 01 (Dec 001) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 02 (Dec 002) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 03 (Dec 003) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 04 (Dec 004) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 05 (Dec 005) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 06 (Dec 006) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 07 (Dec 007) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 08 (Dec 008) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 09 (Dec 009) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 0A (Dec 010) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 0B (Dec 011) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 0C (Dec 012) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 0D (Dec 013) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 0E (Dec 014) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 0F (Dec 015) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 10 (Dec 016) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 11 (Dec 017) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 12 (Dec 018) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 13 (Dec 019) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 14 (Dec 020) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 15 (Dec 021) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 16 (Dec 022) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 17 (Dec 023) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 18 (Dec 024) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 19 (Dec 025) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 1A (Dec 026) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 1B (Dec 027) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 1C (Dec 028) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 1D (Dec 029) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 1E (Dec 030) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 1F (Dec 031) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0x0020),  // Hex No. 20 (Dec 032) Character ' ' Whitespace
    QChar(0x2701),  // Hex No. 21 (Dec 033) Character '✁' Symbol
    QChar(0x2702),  // Hex No. 22 (Dec 034) Character '✂' Symbol
    QChar(0x2703),  // Hex No. 23 (Dec 035) Character '✃' Symbol
    QChar(0x2704),  // Hex No. 24 (Dec 036) Character '✄' Symbol
    QChar(0x260e),  // Hex No. 25 (Dec 037) Character '☎' Symbol
    QChar(0x2706),  // Hex No. 26 (Dec 038) Character '✆' Symbol
    QChar(0x2707),  // Hex No. 27 (Dec 039) Character '✇' Symbol
    QChar(0x2708),  // Hex No. 28 (Dec 040) Character '✈' Symbol
    QChar(0x2709),  // Hex No. 29 (Dec 041) Character '✉' Symbol
    QChar(0x261b),  // Hex No. 2A (Dec 042) Character '☛' Symbol
    QChar(0x261e),  // Hex No. 2B (Dec 043) Character '☞' Symbol
    QChar(0x270c),  // Hex No. 2C (Dec 044) Character '✌' Symbol
    QChar(0x270d),  // Hex No. 2D (Dec 045) Character '✍' Symbol
    QChar(0x270e),  // Hex No. 2E (Dec 046) Character '✎' Symbol
    QChar(0x270f),  // Hex No. 2F (Dec 047) Character '✏' Symbol
    QChar(0x2710),  // Hex No. 30 (Dec 048) Character '✐' Symbol
    QChar(0x2711),  // Hex No. 31 (Dec 049) Character '✑' Symbol
    QChar(0x2712),  // Hex No. 32 (Dec 050) Character '✒' Symbol
    QChar(0x2713),  // Hex No. 33 (Dec 051) Character '✓' Symbol
    QChar(0x2714),  // Hex No. 34 (Dec 052) Character '✔' Symbol
    QChar(0x2715),  // Hex No. 35 (Dec 053) Character '✕' Symbol
    QChar(0x2716),  // Hex No. 36 (Dec 054) Character '✖' Symbol
    QChar(0x2717),  // Hex No. 37 (Dec 055) Character '✗' Symbol
    QChar(0x2718),  // Hex No. 38 (Dec 056) Character '✘' Symbol
    QChar(0x2719),  // Hex No. 39 (Dec 057) Character '✙' Symbol
    QChar(0x271a),  // Hex No. 3A (Dec 058) Character '✚' Symbol
    QChar(0x271b),  // Hex No. 3B (Dec 059) Character '✛' Symbol
    QChar(0x271c),  // Hex No. 3C (Dec 060) Character '✜' Symbol
    QChar(0x271d),  // Hex No. 3D (Dec 061) Character '✝' Symbol
    QChar(0x271e),  // Hex No. 3E (Dec 062) Character '✞' Symbol
    QChar(0x271f),  // Hex No. 3F (Dec 063) Character '✟' Symbol
    QChar(0x2720),  // Hex No. 40 (Dec 064) Character '✠' Symbol
    QChar(0x2721),  // Hex No. 41 (Dec 065) Character '✡' Symbol
    QChar(0x2722),  // Hex No. 42 (Dec 066) Character '✢' Symbol
    QChar(0x2723),  // Hex No. 43 (Dec 067) Character '✣' Symbol
    QChar(0x2724),  // Hex No. 44 (Dec 068) Character '✤' Symbol
    QChar(0x2725),  // Hex No. 45 (Dec 069) Character '✥' Symbol
    QChar(0x2726),  // Hex No. 46 (Dec 070) Character '✦' Symbol
    QChar(0x2727),  // Hex No. 47 (Dec 071) Character '✧' Symbol
    QChar(0x2605),  // Hex No. 48 (Dec 072) Character '★' Symbol
    QChar(0x2729),  // Hex No. 49 (Dec 073) Character '✩' Symbol
    QChar(0x272a),  // Hex No. 4A (Dec 074) Character '✪' Symbol
    QChar(0x272b),  // Hex No. 4B (Dec 075) Character '✫' Symbol
    QChar(0x272c),  // Hex No. 4C (Dec 076) Character '✬' Symbol
    QChar(0x272d),  // Hex No. 4D (Dec 077) Character '✭' Symbol
    QChar(0x272e),  // Hex No. 4E (Dec 078) Character '✮' Symbol
    QChar(0x272f),  // Hex No. 4F (Dec 079) Character '✯' Symbol
    QChar(0x2730),  // Hex No. 50 (Dec 080) Character '✰' Symbol
    QChar(0x2731),  // Hex No. 51 (Dec 081) Character '✱' Symbol
    QChar(0x2732),  // Hex No. 52 (Dec 082) Character '✲' Symbol
    QChar(0x2733),  // Hex No. 53 (Dec 083) Character '✳' Symbol
    QChar(0x2734),  // Hex No. 54 (Dec 084) Character '✴' Symbol
    QChar(0x2735),  // Hex No. 55 (Dec 085) Character '✵' Symbol
    QChar(0x2736),  // Hex No. 56 (Dec 086) Character '✶' Symbol
    QChar(0x2737),  // Hex No. 57 (Dec 087) Character '✷' Symbol
    QChar(0x2738),  // Hex No. 58 (Dec 088) Character '✸' Symbol
    QChar(0x2739),  // Hex No. 59 (Dec 089) Character '✹' Symbol
    QChar(0x273a),  // Hex No. 5A (Dec 090) Character '✺' Symbol
    QChar(0x273b),  // Hex No. 5B (Dec 091) Character '✻' Symbol
    QChar(0x273c),  // Hex No. 5C (Dec 092) Character '✼' Symbol
    QChar(0x273d),  // Hex No. 5D (Dec 093) Character '✽' Symbol
    QChar(0x273e),  // Hex No. 5E (Dec 094) Character '✾' Symbol
    QChar(0x273f),  // Hex No. 5F (Dec 095) Character '✿' Symbol
    QChar(0x2740),  // Hex No. 60 (Dec 096) Character '❀' Symbol
    QChar(0x2741),  // Hex No. 61 (Dec 097) Character '❁' Symbol
    QChar(0x2742),  // Hex No. 62 (Dec 098) Character '❂' Symbol
    QChar(0x2743),  // Hex No. 63 (Dec 099) Character '❃' Symbol
    QChar(0x2744),  // Hex No. 64 (Dec 100) Character '❄' Symbol
    QChar(0x2745),  // Hex No. 65 (Dec 101) Character '❅' Symbol
    QChar(0x2746),  // Hex No. 66 (Dec 102) Character '❆' Symbol
    QChar(0x2747),  // Hex No. 67 (Dec 103) Character '❇' Symbol
    QChar(0x2748),  // Hex No. 68 (Dec 104) Character '❈' Symbol
    QChar(0x2749),  // Hex No. 69 (Dec 105) Character '❉' Symbol
    QChar(0x274a),  // Hex No. 6A (Dec 106) Character '❊' Symbol
    QChar(0x274b),  // Hex No. 6B (Dec 107) Character '❋' Symbol
    QChar(0x25cf),  // Hex No. 6C (Dec 108) Character '●' Symbol
    QChar(0x274d),  // Hex No. 6D (Dec 109) Character '❍' Symbol
    QChar(0x25a0),  // Hex No. 6E (Dec 110) Character '■' Symbol
    QChar(0x274f),  // Hex No. 6F (Dec 111) Character '❏' Symbol
    QChar(0x2750),  // Hex No. 70 (Dec 112) Character '❐' Symbol
    QChar(0x2751),  // Hex No. 71 (Dec 113) Character '❑' Symbol
    QChar(0x2752),  // Hex No. 72 (Dec 114) Character '❒' Symbol
    QChar(0x25b2),  // Hex No. 73 (Dec 115) Character '▲' Symbol
    QChar(0x25bc),  // Hex No. 74 (Dec 116) Character '▼' Symbol
    QChar(0x25c6),  // Hex No. 75 (Dec 117) Character '◆' Symbol
    QChar(0x2756),  // Hex No. 76 (Dec 118) Character '❖' Symbol
    QChar(0x25d7),  // Hex No. 77 (Dec 119) Character '◗' Symbol
    QChar(0x2758),  // Hex No. 78 (Dec 120) Character '❘' Symbol
    QChar(0x2759),  // Hex No. 79 (Dec 121) Character '❙' Symbol
    QChar(0x275a),  // Hex No. 7A (Dec 122) Character '❚' Symbol
    QChar(0x275b),  // Hex No. 7B (Dec 123) Character '❛' Symbol
    QChar(0x275c),  // Hex No. 7C (Dec 124) Character '❜' Symbol
    QChar(0x275d),  // Hex No. 7D (Dec 125) Character '❝' Symbol
    QChar(0x275e),  // Hex No. 7E (Dec 126) Character '❞' Symbol
    QChar(0xfffd),  // Hex No. 7F (Dec 127) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 80 (Dec 128) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 81 (Dec 129) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 82 (Dec 130) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 83 (Dec 131) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 84 (Dec 132) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 85 (Dec 133) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 86 (Dec 134) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 87 (Dec 135) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 88 (Dec 136) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 89 (Dec 137) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 8A (Dec 138) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 8B (Dec 139) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 8C (Dec 140) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 8D (Dec 141) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 8E (Dec 142) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 8F (Dec 143) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 90 (Dec 144) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 91 (Dec 145) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 92 (Dec 146) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 93 (Dec 147) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 94 (Dec 148) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 95 (Dec 149) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 96 (Dec 150) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 97 (Dec 151) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 98 (Dec 152) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 99 (Dec 153) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 9A (Dec 154) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 9B (Dec 155) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 9C (Dec 156) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 9D (Dec 157) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 9E (Dec 158) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 9F (Dec 159) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. A0 (Dec 160) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0x2761),  // Hex No. A1 (Dec 161) Character '❡' Symbol
    QChar(0x2762),  // Hex No. A2 (Dec 162) Character '❢' Symbol
    QChar(0x2763),  // Hex No. A3 (Dec 163) Character '❣' Symbol
    QChar(0x2764),  // Hex No. A4 (Dec 164) Character '❤' Symbol
    QChar(0x2765),  // Hex No. A5 (Dec 165) Character '❥' Symbol
    QChar(0x2766),  // Hex No. A6 (Dec 166) Character '❦' Symbol
    QChar(0x2767),  // Hex No. A7 (Dec 167) Character '❧' Symbol
    QChar(0x2663),  // Hex No. A8 (Dec 168) Character '♣' Symbol
    QChar(0x2666),  // Hex No. A9 (Dec 169) Character '♦' Symbol
    QChar(0x2665),  // Hex No. AA (Dec 170) Character '♥' Symbol
    QChar(0x2660),  // Hex No. AB (Dec 171) Character '♠' Symbol
    QChar(0x2460),  // Hex No. AC (Dec 172) Character '①'
    QChar(0x2461),  // Hex No. AD (Dec 173) Character '②'
    QChar(0x2462),  // Hex No. AE (Dec 174) Character '③'
    QChar(0x2463),  // Hex No. AF (Dec 175) Character '④'
    QChar(0x2464),  // Hex No. B0 (Dec 176) Character '⑤'
    QChar(0x2465),  // Hex No. B1 (Dec 177) Character '⑥'
    QChar(0x2466),  // Hex No. B2 (Dec 178) Character '⑦'
    QChar(0x2467),  // Hex No. B3 (Dec 179) Character '⑧'
    QChar(0x2468),  // Hex No. B4 (Dec 180) Character '⑨'
    QChar(0x2469),  // Hex No. B5 (Dec 181) Character '⑩'
    QChar(0x2776),  // Hex No. B6 (Dec 182) Character '❶'
    QChar(0x2777),  // Hex No. B7 (Dec 183) Character '❷'
    QChar(0x2778),  // Hex No. B8 (Dec 184) Character '❸'
    QChar(0x2779),  // Hex No. B9 (Dec 185) Character '❹'
    QChar(0x277a),  // Hex No. BA (Dec 186) Character '❺'
    QChar(0x277b),  // Hex No. BB (Dec 187) Character '❻'
    QChar(0x277c),  // Hex No. BC (Dec 188) Character '❼'
    QChar(0x277d),  // Hex No. BD (Dec 189) Character '❽'
    QChar(0x277e),  // Hex No. BE (Dec 190) Character '❾'
    QChar(0x277f),  // Hex No. BF (Dec 191) Character '❿'
    QChar(0x2780),  // Hex No. C0 (Dec 192) Character '➀'
    QChar(0x2781),  // Hex No. C1 (Dec 193) Character '➁'
    QChar(0x2782),  // Hex No. C2 (Dec 194) Character '➂'
    QChar(0x2783),  // Hex No. C3 (Dec 195) Character '➃'
    QChar(0x2784),  // Hex No. C4 (Dec 196) Character '➄'
    QChar(0x2785),  // Hex No. C5 (Dec 197) Character '➅'
    QChar(0x2786),  // Hex No. C6 (Dec 198) Character '➆'
    QChar(0x2787),  // Hex No. C7 (Dec 199) Character '➇'
    QChar(0x2788),  // Hex No. C8 (Dec 200) Character '➈'
    QChar(0x2789),  // Hex No. C9 (Dec 201) Character '➉'
    QChar(0x278a),  // Hex No. CA (Dec 202) Character '➊'
    QChar(0x278b),  // Hex No. CB (Dec 203) Character '➋'
    QChar(0x278c),  // Hex No. CC (Dec 204) Character '➌'
    QChar(0x278d),  // Hex No. CD (Dec 205) Character '➍'
    QChar(0x278e),  // Hex No. CE (Dec 206) Character '➎'
    QChar(0x278f),  // Hex No. CF (Dec 207) Character '➏'
    QChar(0x2790),  // Hex No. D0 (Dec 208) Character '➐'
    QChar(0x2791),  // Hex No. D1 (Dec 209) Character '➑'
    QChar(0x2792),  // Hex No. D2 (Dec 210) Character '➒'
    QChar(0x2793),  // Hex No. D3 (Dec 211) Character '➓'
    QChar(0x2794),  // Hex No. D4 (Dec 212) Character '➔' Symbol
    QChar(0x2192),  // Hex No. D5 (Dec 213) Character '→' Symbol
    QChar(0x2194),  // Hex No. D6 (Dec 214) Character '↔' Symbol
    QChar(0x2195),  // Hex No. D7 (Dec 215) Character '↕' Symbol
    QChar(0x2798),  // Hex No. D8 (Dec 216) Character '➘' Symbol
    QChar(0x2799),  // Hex No. D9 (Dec 217) Character '➙' Symbol
    QChar(0x279a),  // Hex No. DA (Dec 218) Character '➚' Symbol
    QChar(0x279b),  // Hex No. DB (Dec 219) Character '➛' Symbol
    QChar(0x279c),  // Hex No. DC (Dec 220) Character '➜' Symbol
    QChar(0x279d),  // Hex No. DD (Dec 221) Character '➝' Symbol
    QChar(0x279e),  // Hex No. DE (Dec 222) Character '➞' Symbol
    QChar(0x279f),  // Hex No. DF (Dec 223) Character '➟' Symbol
    QChar(0x27a0),  // Hex No. E0 (Dec 224) Character '➠' Symbol
    QChar(0x27a1),  // Hex No. E1 (Dec 225) Character '➡' Symbol
    QChar(0x27a2),  // Hex No. E2 (Dec 226) Character '➢' Symbol
    QChar(0x27a3),  // Hex No. E3 (Dec 227) Character '➣' Symbol
    QChar(0x27a4),  // Hex No. E4 (Dec 228) Character '➤' Symbol
    QChar(0x27a5),  // Hex No. E5 (Dec 229) Character '➥' Symbol
    QChar(0x27a6),  // Hex No. E6 (Dec 230) Character '➦' Symbol
    QChar(0x27a7),  // Hex No. E7 (Dec 231) Character '➧' Symbol
    QChar(0x27a8),  // Hex No. E8 (Dec 232) Character '➨' Symbol
    QChar(0x27a9),  // Hex No. E9 (Dec 233) Character '➩' Symbol
    QChar(0x27aa),  // Hex No. EA (Dec 234) Character '➪' Symbol
    QChar(0x27ab),  // Hex No. EB (Dec 235) Character '➫' Symbol
    QChar(0x27ac),  // Hex No. EC (Dec 236) Character '➬' Symbol
    QChar(0x27ad),  // Hex No. ED (Dec 237) Character '➭' Symbol
    QChar(0x27ae),  // Hex No. EE (Dec 238) Character '➮' Symbol
    QChar(0x27af),  // Hex No. EF (Dec 239) Character '➯' Symbol
    QChar(0xfffd),  // Hex No. F0 (Dec 240) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0x27b1),  // Hex No. F1 (Dec 241) Character '➱' Symbol
    QChar(0x27b2),  // Hex No. F2 (Dec 242) Character '➲' Symbol
    QChar(0x27b3),  // Hex No. F3 (Dec 243) Character '➳' Symbol
    QChar(0x27b4),  // Hex No. F4 (Dec 244) Character '➴' Symbol
    QChar(0x27b5),  // Hex No. F5 (Dec 245) Character '➵' Symbol
    QChar(0x27b6),  // Hex No. F6 (Dec 246) Character '➶' Symbol
    QChar(0x27b7),  // Hex No. F7 (Dec 247) Character '➷' Symbol
    QChar(0x27b8),  // Hex No. F8 (Dec 248) Character '➸' Symbol
    QChar(0x27b9),  // Hex No. F9 (Dec 249) Character '➹' Symbol
    QChar(0x27ba),  // Hex No. FA (Dec 250) Character '➺' Symbol
    QChar(0x27bb),  // Hex No. FB (Dec 251) Character '➻' Symbol
    QChar(0x27bc),  // Hex No. FC (Dec 252) Character '➼' Symbol
    QChar(0x27bd),  // Hex No. FD (Dec 253) Character '➽' Symbol
    QChar(0x27be),  // Hex No. FE (Dec 254) Character '➾' Symbol
    QChar(0xfffd),  // Hex No. FF (Dec 255) REPLACEMENT CHARACTER 0xFFFD - not present in character set
};

// Mac OS encoding
static const EncodingTable MAC_OS_ENCODING_CONVERSION_TABLE = {
    QChar(0xfffd),  // Hex No. 00 (Dec 000) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 01 (Dec 001) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 02 (Dec 002) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 03 (Dec 003) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 04 (Dec 004) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 05 (Dec 005) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 06 (Dec 006) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 07 (Dec 007) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 08 (Dec 008) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 09 (Dec 009) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 0A (Dec 010) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 0B (Dec 011) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 0C (Dec 012) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 0D (Dec 013) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 0E (Dec 014) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 0F (Dec 015) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 10 (Dec 016) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 11 (Dec 017) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 12 (Dec 018) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 13 (Dec 019) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 14 (Dec 020) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 15 (Dec 021) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 16 (Dec 022) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 17 (Dec 023) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 18 (Dec 024) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 19 (Dec 025) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 1A (Dec 026) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 1B (Dec 027) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 1C (Dec 028) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 1D (Dec 029) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 1E (Dec 030) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0xfffd),  // Hex No. 1F (Dec 031) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0x0020),  // Hex No. 20 (Dec 032) Character ' ' Whitespace
    QChar(0x0021),  // Hex No. 21 (Dec 033) Character '!' Punctuation
    QChar(0x0022),  // Hex No. 22 (Dec 034) Character '"' Punctuation
    QChar(0x0023),  // Hex No. 23 (Dec 035) Character '#' Punctuation
    QChar(0x0024),  // Hex No. 24 (Dec 036) Character '$' Symbol
    QChar(0x0025),  // Hex No. 25 (Dec 037) Character '%' Punctuation
    QChar(0x0026),  // Hex No. 26 (Dec 038) Character '&' Punctuation
    QChar(0x0027),  // Hex No. 27 (Dec 039) Character ''' Punctuation
    QChar(0x0028),  // Hex No. 28 (Dec 040) Character '(' Punctuation
    QChar(0x0029),  // Hex No. 29 (Dec 041) Character ')' Punctuation
    QChar(0x002a),  // Hex No. 2A (Dec 042) Character '*' Punctuation
    QChar(0x002b),  // Hex No. 2B (Dec 043) Character '+' Symbol
    QChar(0x002c),  // Hex No. 2C (Dec 044) Character ',' Punctuation
    QChar(0x002d),  // Hex No. 2D (Dec 045) Character '-' Punctuation
    QChar(0x002e),  // Hex No. 2E (Dec 046) Character '.' Punctuation
    QChar(0x002f),  // Hex No. 2F (Dec 047) Character '/' Punctuation
    QChar(0x0030),  // Hex No. 30 (Dec 048) Character '0' Digit
    QChar(0x0031),  // Hex No. 31 (Dec 049) Character '1' Digit
    QChar(0x0032),  // Hex No. 32 (Dec 050) Character '2' Digit
    QChar(0x0033),  // Hex No. 33 (Dec 051) Character '3' Digit
    QChar(0x0034),  // Hex No. 34 (Dec 052) Character '4' Digit
    QChar(0x0035),  // Hex No. 35 (Dec 053) Character '5' Digit
    QChar(0x0036),  // Hex No. 36 (Dec 054) Character '6' Digit
    QChar(0x0037),  // Hex No. 37 (Dec 055) Character '7' Digit
    QChar(0x0038),  // Hex No. 38 (Dec 056) Character '8' Digit
    QChar(0x0039),  // Hex No. 39 (Dec 057) Character '9' Digit
    QChar(0x003a),  // Hex No. 3A (Dec 058) Character ':' Punctuation
    QChar(0x003b),  // Hex No. 3B (Dec 059) Character ';' Punctuation
    QChar(0x003c),  // Hex No. 3C (Dec 060) Character '<' Symbol
    QChar(0x003d),  // Hex No. 3D (Dec 061) Character '=' Symbol
    QChar(0x003e),  // Hex No. 3E (Dec 062) Character '>' Symbol
    QChar(0x003f),  // Hex No. 3F (Dec 063) Character '?' Punctuation
    QChar(0x0040),  // Hex No. 40 (Dec 064) Character '@' Punctuation
    QChar(0x0041),  // Hex No. 41 (Dec 065) Character 'A' Letter, Uppercase
    QChar(0x0042),  // Hex No. 42 (Dec 066) Character 'B' Letter, Uppercase
    QChar(0x0043),  // Hex No. 43 (Dec 067) Character 'C' Letter, Uppercase
    QChar(0x0044),  // Hex No. 44 (Dec 068) Character 'D' Letter, Uppercase
    QChar(0x0045),  // Hex No. 45 (Dec 069) Character 'E' Letter, Uppercase
    QChar(0x0046),  // Hex No. 46 (Dec 070) Character 'F' Letter, Uppercase
    QChar(0x0047),  // Hex No. 47 (Dec 071) Character 'G' Letter, Uppercase
    QChar(0x0048),  // Hex No. 48 (Dec 072) Character 'H' Letter, Uppercase
    QChar(0x0049),  // Hex No. 49 (Dec 073) Character 'I' Letter, Uppercase
    QChar(0x004a),  // Hex No. 4A (Dec 074) Character 'J' Letter, Uppercase
    QChar(0x004b),  // Hex No. 4B (Dec 075) Character 'K' Letter, Uppercase
    QChar(0x004c),  // Hex No. 4C (Dec 076) Character 'L' Letter, Uppercase
    QChar(0x004d),  // Hex No. 4D (Dec 077) Character 'M' Letter, Uppercase
    QChar(0x004e),  // Hex No. 4E (Dec 078) Character 'N' Letter, Uppercase
    QChar(0x004f),  // Hex No. 4F (Dec 079) Character 'O' Letter, Uppercase
    QChar(0x0050),  // Hex No. 50 (Dec 080) Character 'P' Letter, Uppercase
    QChar(0x0051),  // Hex No. 51 (Dec 081) Character 'Q' Letter, Uppercase
    QChar(0x0052),  // Hex No. 52 (Dec 082) Character 'R' Letter, Uppercase
    QChar(0x0053),  // Hex No. 53 (Dec 083) Character 'S' Letter, Uppercase
    QChar(0x0054),  // Hex No. 54 (Dec 084) Character 'T' Letter, Uppercase
    QChar(0x0055),  // Hex No. 55 (Dec 085) Character 'U' Letter, Uppercase
    QChar(0x0056),  // Hex No. 56 (Dec 086) Character 'V' Letter, Uppercase
    QChar(0x0057),  // Hex No. 57 (Dec 087) Character 'W' Letter, Uppercase
    QChar(0x0058),  // Hex No. 58 (Dec 088) Character 'X' Letter, Uppercase
    QChar(0x0059),  // Hex No. 59 (Dec 089) Character 'Y' Letter, Uppercase
    QChar(0x005a),  // Hex No. 5A (Dec 090) Character 'Z' Letter, Uppercase
    QChar(0x005b),  // Hex No. 5B (Dec 091) Character '[' Punctuation
    QChar(0x005c),  // Hex No. 5C (Dec 092) Character '\' Punctuation
    QChar(0x005d),  // Hex No. 5D (Dec 093) Character ']' Punctuation
    QChar(0x005e),  // Hex No. 5E (Dec 094) Character '^' Symbol
    QChar(0x005f),  // Hex No. 5F (Dec 095) Character '_' Punctuation
    QChar(0x0060),  // Hex No. 60 (Dec 096) Character '`' Symbol
    QChar(0x0061),  // Hex No. 61 (Dec 097) Character 'a' Letter, Lowercase
    QChar(0x0062),  // Hex No. 62 (Dec 098) Character 'b' Letter, Lowercase
    QChar(0x0063),  // Hex No. 63 (Dec 099) Character 'c' Letter, Lowercase
    QChar(0x0064),  // Hex No. 64 (Dec 100) Character 'd' Letter, Lowercase
    QChar(0x0065),  // Hex No. 65 (Dec 101) Character 'e' Letter, Lowercase
    QChar(0x0066),  // Hex No. 66 (Dec 102) Character 'f' Letter, Lowercase
    QChar(0x0067),  // Hex No. 67 (Dec 103) Character 'g' Letter, Lowercase
    QChar(0x0068),  // Hex No. 68 (Dec 104) Character 'h' Letter, Lowercase
    QChar(0x0069),  // Hex No. 69 (Dec 105) Character 'i' Letter, Lowercase
    QChar(0x006a),  // Hex No. 6A (Dec 106) Character 'j' Letter, Lowercase
    QChar(0x006b),  // Hex No. 6B (Dec 107) Character 'k' Letter, Lowercase
    QChar(0x006c),  // Hex No. 6C (Dec 108) Character 'l' Letter, Lowercase
    QChar(0x006d),  // Hex No. 6D (Dec 109) Character 'm' Letter, Lowercase
    QChar(0x006e),  // Hex No. 6E (Dec 110) Character 'n' Letter, Lowercase
    QChar(0x006f),  // Hex No. 6F (Dec 111) Character 'o' Letter, Lowercase
    QChar(0x0070),  // Hex No. 70 (Dec 112) Character 'p' Letter, Lowercase
    QChar(0x0071),  // Hex No. 71 (Dec 113) Character 'q' Letter, Lowercase
    QChar(0x0072),  // Hex No. 72 (Dec 114) Character 'r' Letter, Lowercase
    QChar(0x0073),  // Hex No. 73 (Dec 115) Character 's' Letter, Lowercase
    QChar(0x0074),  // Hex No. 74 (Dec 116) Character 't' Letter, Lowercase
    QChar(0x0075),  // Hex No. 75 (Dec 117) Character 'u' Letter, Lowercase
    QChar(0x0076),  // Hex No. 76 (Dec 118) Character 'v' Letter, Lowercase
    QChar(0x0077),  // Hex No. 77 (Dec 119) Character 'w' Letter, Lowercase
    QChar(0x0078),  // Hex No. 78 (Dec 120) Character 'x' Letter, Lowercase
    QChar(0x0079),  // Hex No. 79 (Dec 121) Character 'y' Letter, Lowercase
    QChar(0x007a),  // Hex No. 7A (Dec 122) Character 'z' Letter, Lowercase
    QChar(0x007b),  // Hex No. 7B (Dec 123) Character '{' Punctuation
    QChar(0x007c),  // Hex No. 7C (Dec 124) Character '|' Symbol
    QChar(0x007d),  // Hex No. 7D (Dec 125) Character '}' Punctuation
    QChar(0x007e),  // Hex No. 7E (Dec 126) Character '~' Symbol
    QChar(0xfffd),  // Hex No. 7F (Dec 127) REPLACEMENT CHARACTER 0xFFFD - not present in character set
    QChar(0x00c4),  // Hex No. 80 (Dec 128) Character 'Ä' Letter, Uppercase
    QChar(0x00c5),  // Hex No. 81 (Dec 129) Character 'Å' Letter, Uppercase
    QChar(0x00c7),  // Hex No. 82 (Dec 130) Character 'Ç' Letter, Uppercase
    QChar(0x00c9),  // Hex No. 83 (Dec 131) Character 'É' Letter, Uppercase
    QChar(0x00d1),  // Hex No. 84 (Dec 132) Character 'Ñ' Letter, Uppercase
    QChar(0x00d6),  // Hex No. 85 (Dec 133) Character 'Ö' Letter, Uppercase
    QChar(0x00dc),  // Hex No. 86 (Dec 134) Character 'Ü' Letter, Uppercase
    QChar(0x00e1),  // Hex No. 87 (Dec 135) Character 'á' Letter, Lowercase
    QChar(0x00e0),  // Hex No. 88 (Dec 136) Character 'à' Letter, Lowercase
    QChar(0x00e2),  // Hex No. 89 (Dec 137) Character 'â' Letter, Lowercase
    QChar(0x00e4),  // Hex No. 8A (Dec 138) Character 'ä' Letter, Lowercase
    QChar(0x00e3),  // Hex No. 8B (Dec 139) Character 'ã' Letter, Lowercase
    QChar(0x00e5),  // Hex No. 8C (Dec 140) Character 'å' Letter, Lowercase
    QChar(0x00e7),  // Hex No. 8D (Dec 141) Character 'ç' Letter, Lowercase
    QChar(0x00e9),  // Hex No. 8E (Dec 142) Character 'é' Letter, Lowercase
    QChar(0x00e8),  // Hex No. 8F (Dec 143) Character 'è' Letter, Lowercase
    QChar(0x00ea),  // Hex No. 90 (Dec 144) Character 'ê' Letter, Lowercase
    QChar(0x00eb),  // Hex No. 91 (Dec 145) Character 'ë' Letter, Lowercase
    QChar(0x00ed),  // Hex No. 92 (Dec 146) Character 'í' Letter, Lowercase
    QChar(0x00ec),  // Hex No. 93 (Dec 147) Character 'ì' Letter, Lowercase
    QChar(0x00ee),  // Hex No. 94 (Dec 148) Character 'î' Letter, Lowercase
    QChar(0x00ef),  // Hex No. 95 (Dec 149) Character 'ï' Letter, Lowercase
    QChar(0x00f1),  // Hex No. 96 (Dec 150) Character 'ñ' Letter, Lowercase
    QChar(0x00f3),  // Hex No. 97 (Dec 151) Character 'ó' Letter, Lowercase
    QChar(0x00f2),  // Hex No. 98 (Dec 152) Character 'ò' Letter, Lowercase
    QChar(0x00f4),  // Hex No. 99 (Dec 153) Character 'ô' Letter, Lowercase
    QChar(0x00f6),  // Hex No. 9A (Dec 154) Character 'ö' Letter, Lowercase
    QChar(0x00f5),  // Hex No. 9B (Dec 155) Character 'õ' Letter, Lowercase
    QChar(0x00fa),  // Hex No. 9C (Dec 156) Character 'ú' Letter, Lowercase
    QChar(0x00f9),  // Hex No. 9D (Dec 157) Character 'ù' Letter, Lowercase
    QChar(0x00fb),  // Hex No. 9E (Dec 158) Character 'û' Letter, Lowercase
    QChar(0x00fc),  // Hex No. 9F (Dec 159) Character 'ü' Letter, Lowercase
    QChar(0x2020),  // Hex No. A0 (Dec 160) Character '†' Punctuation
    QChar(0x00b0),  // Hex No. A1 (Dec 161) Character '°' Symbol
    QChar(0x00a2),  // Hex No. A2 (Dec 162) Character '¢' Symbol
    QChar(0x00a3),  // Hex No. A3 (Dec 163) Character '£' Symbol
    QChar(0x00a7),  // Hex No. A4 (Dec 164) Character '§' Punctuation
    QChar(0x2022),  // Hex No. A5 (Dec 165) Character '•' Punctuation
    QChar(0x00b6),  // Hex No. A6 (Dec 166) Character '¶' Punctuation
    QChar(0x00df),  // Hex No. A7 (Dec 167) Character 'ß' Letter, Lowercase
    QChar(0x00ae),  // Hex No. A8 (Dec 168) Character '®' Symbol
    QChar(0x00a9),  // Hex No. A9 (Dec 169) Character '©' Symbol
    QChar(0x2122),  // Hex No. AA (Dec 170) Character '™' Symbol
    QChar(0x00b4),  // Hex No. AB (Dec 171) Character '´' Symbol
    QChar(0x00a8),  // Hex No. AC (Dec 172) Character '¨' Symbol
    QChar(0x2260),  // Hex No. AD (Dec 173) Character '≠' Symbol
    QChar(0x00c6),  // Hex No. AE (Dec 174) Character 'Æ' Letter, Uppercase
    QChar(0x00d8),  // Hex No. AF (Dec 175) Character 'Ø' Letter, Uppercase
    QChar(0x221e),  // Hex No. B0 (Dec 176) Character '∞' Symbol
    QChar(0x00b1),  // Hex No. B1 (Dec 177) Character '±' Symbol
    QChar(0x2264),  // Hex No. B2 (Dec 178) Character '≤' Symbol
    QChar(0x2265),  // Hex No. B3 (Dec 179) Character '≥' Symbol
    QChar(0x00a5),  // Hex No. B4 (Dec 180) Character '¥' Symbol
    QChar(0x00b5),  // Hex No. B5 (Dec 181) Character 'µ' Letter, Lowercase
    QChar(0x2202),  // Hex No. B6 (Dec 182) Character '∂' Symbol
    QChar(0x2211),  // Hex No. B7 (Dec 183) Character '∑' Symbol
    QChar(0x220f),  // Hex No. B8 (Dec 184) Character '∏' Symbol
    QChar(0x03c0),  // Hex No. B9 (Dec 185) Character 'π' Letter, Lowercase
    QChar(0x222b),  // Hex No. BA (Dec 186) Character '∫' Symbol
    QChar(0x00aa),  // Hex No. BB (Dec 187) Character 'ª' Letter
    QChar(0x00ba),  // Hex No. BC (Dec 188) Character 'º' Letter
    QChar(0x2126),  // Hex No. BD (Dec 189) Character 'Ω' Letter, Uppercase
    QChar(0x00e6),  // Hex No. BE (Dec 190) Character 'æ' Letter, Lowercase
    QChar(0x00f8),  // Hex No. BF (Dec 191) Character 'ø' Letter, Lowercase
    QChar(0x00bf),  // Hex No. C0 (Dec 192) Character '¿' Punctuation
    QChar(0x00a1),  // Hex No. C1 (Dec 193) Character '¡' Punctuation
    QChar(0x00ac),  // Hex No. C2 (Dec 194) Character '¬' Symbol
    QChar(0x221a),  // Hex No. C3 (Dec 195) Character '√' Symbol
    QChar(0x0192),  // Hex No. C4 (Dec 196) Character 'ƒ' Letter, Lowercase
    QChar(0x2248),  // Hex No. C5 (Dec 197) Character '≈' Symbol
    QChar(0x2206),  // Hex No. C6 (Dec 198) Character '∆' Symbol
    QChar(0x00ab),  // Hex No. C7 (Dec 199) Character '«' Punctuation
    QChar(0x00bb),  // Hex No. C8 (Dec 200) Character '»' Punctuation
    QChar(0x2026),  // Hex No. C9 (Dec 201) Character '…' Punctuation
    QChar(0x0020),  // Hex No. CA (Dec 202) Character ' ' Whitespace
    QChar(0x00c0),  // Hex No. CB (Dec 203) Character 'À' Letter, Uppercase
    QChar(0x00c3),  // Hex No. CC (Dec 204) Character 'Ã' Letter, Uppercase
    QChar(0x00d5),  // Hex No. CD (Dec 205) Character 'Õ' Letter, Uppercase
    QChar(0x0152),  // Hex No. CE (Dec 206) Character 'Œ' Letter, Uppercase
    QChar(0x0153),  // Hex No. CF (Dec 207) Character 'œ' Letter, Lowercase
    QChar(0x2013),  // Hex No. D0 (Dec 208) Character '–' Punctuation
    QChar(0x2014),  // Hex No. D1 (Dec 209) Character '—' Punctuation
    QChar(0x201c),  // Hex No. D2 (Dec 210) Character '“' Punctuation
    QChar(0x201d),  // Hex No. D3 (Dec 211) Character '”' Punctuation
    QChar(0x2018),  // Hex No. D4 (Dec 212) Character '‘' Punctuation
    QChar(0x2019),  // Hex No. D5 (Dec 213) Character '’' Punctuation
    QChar(0x00f7),  // Hex No. D6 (Dec 214) Character '÷' Symbol
    QChar(0x25ca),  // Hex No. D7 (Dec 215) Character '◊' Symbol
    QChar(0x00ff),  // Hex No. D8 (Dec 216) Character 'ÿ' Letter, Lowercase
    QChar(0x0178),  // Hex No. D9 (Dec 217) Character 'Ÿ' Letter, Uppercase
    QChar(0x2044),  // Hex No. DA (Dec 218) Character '⁄' Symbol
    QChar(0x20ac),  // Hex No. DB (Dec 219) Character '€' Symbol               !!! REPLACED FOR MAC OS
    QChar(0x2039),  // Hex No. DC (Dec 220) Character '‹' Punctuation
    QChar(0x203a),  // Hex No. DD (Dec 221) Character '›' Punctuation
    QChar(0xfb01),  // Hex No. DE (Dec 222) Character 'ﬁ' Letter, Lowercase
    QChar(0xfb02),  // Hex No. DF (Dec 223) Character 'ﬂ' Letter, Lowercase
    QChar(0x2021),  // Hex No. E0 (Dec 224) Character '‡' Punctuation
    QChar(0x00b7),  // Hex No. E1 (Dec 225) Character '·' Punctuation
    QChar(0x201a),  // Hex No. E2 (Dec 226) Character '‚' Punctuation
    QChar(0x201e),  // Hex No. E3 (Dec 227) Character '„' Punctuation
    QChar(0x2030),  // Hex No. E4 (Dec 228) Character '‰' Punctuation
    QChar(0x00c2),  // Hex No. E5 (Dec 229) Character 'Â' Letter, Uppercase
    QChar(0x00ca),  // Hex No. E6 (Dec 230) Character 'Ê' Letter, Uppercase
    QChar(0x00c1),  // Hex No. E7 (Dec 231) Character 'Á' Letter, Uppercase
    QChar(0x00cb),  // Hex No. E8 (Dec 232) Character 'Ë' Letter, Uppercase
    QChar(0x00c8),  // Hex No. E9 (Dec 233) Character 'È' Letter, Uppercase
    QChar(0x00cd),  // Hex No. EA (Dec 234) Character 'Í' Letter, Uppercase
    QChar(0x00ce),  // Hex No. EB (Dec 235) Character 'Î' Letter, Uppercase
    QChar(0x00cf),  // Hex No. EC (Dec 236) Character 'Ï' Letter, Uppercase
    QChar(0x00cc),  // Hex No. ED (Dec 237) Character 'Ì' Letter, Uppercase
    QChar(0x00d3),  // Hex No. EE (Dec 238) Character 'Ó' Letter, Uppercase
    QChar(0x00d4),  // Hex No. EF (Dec 239) Character 'Ô' Letter, Uppercase
    QChar(0xf8ff),  // Hex No. F0 (Dec 240)
    QChar(0x00d2),  // Hex No. F1 (Dec 241) Character 'Ò' Letter, Uppercase
    QChar(0x00da),  // Hex No. F2 (Dec 242) Character 'Ú' Letter, Uppercase
    QChar(0x00db),  // Hex No. F3 (Dec 243) Character 'Û' Letter, Uppercase
    QChar(0x00d9),  // Hex No. F4 (Dec 244) Character 'Ù' Letter, Uppercase
    QChar(0x0131),  // Hex No. F5 (Dec 245) Character 'ı' Letter, Lowercase
    QChar(0x02c6),  // Hex No. F6 (Dec 246) Character 'ˆ' Letter
    QChar(0x02dc),  // Hex No. F7 (Dec 247) Character '˜' Symbol
    QChar(0x00af),  // Hex No. F8 (Dec 248) Character '¯' Symbol
    QChar(0x02d8),  // Hex No. F9 (Dec 249) Character '˘' Symbol
    QChar(0x02d9),  // Hex No. FA (Dec 250) Character '˙' Symbol
    QChar(0x02da),  // Hex No. FB (Dec 251) Character '˚' Symbol
    QChar(0x00b8),  // Hex No. FC (Dec 252) Character '¸' Symbol
    QChar(0x02dd),  // Hex No. FD (Dec 253) Character '˝' Symbol
    QChar(0x02db),  // Hex No. FE (Dec 254) Character '˛' Symbol
    QChar(0x02c7),  // Hex No. FF (Dec 255) Character 'ˇ' Letter
};

} // namespace encoding

QString PDFEncoding::convert(const QByteArray& stream, PDFEncoding::Encoding encoding)
{
    const encoding::EncodingTable* table = getTableForEncoding(encoding);
    Q_ASSERT(table);

    // Test by assert, than table has enough items for encoded byte stream
    Q_ASSERT(table->size() == std::numeric_limits<unsigned char>::max() + 1);

    const int size = stream.size();
    const char* data = stream.constData();

    QString result;
    result.resize(size, QChar());

    for (int i = 0; i < size; ++i)
    {
        result[i] = (*table)[static_cast<unsigned char>(data[i])];
    }

    return result;
}

QByteArray PDFEncoding::convertToEncoding(const QString& string, Encoding encoding)
{
    QByteArray result;

    const encoding::EncodingTable* table = getTableForEncoding(encoding);
    Q_ASSERT(table);

    result.reserve(string.size());
    for (QChar character : string)
    {
        ushort unicode = character.unicode();
        unsigned char converted = 0;

        for (size_t i = 0; i < table->size(); ++i)
        {
            if (unicode == (*table)[static_cast<unsigned char>(i)])
            {
                converted = static_cast<unsigned char>(i);
                break;
            }
        }

        result.push_back(converted);
    }

    return result;
}

bool PDFEncoding::canConvertToEncoding(const QString& string, Encoding encoding, QString* invalidCharacters)
{
    const encoding::EncodingTable* table = getTableForEncoding(encoding);
    Q_ASSERT(table);

    bool isConvertible = true;
    for (QChar character : string)
    {
        ushort unicode = character.unicode();
        bool converted = false;

        for (int i = 0; i < static_cast< int >(table->size()); ++i)
        {
            if (unicode == (*table)[static_cast<unsigned char>(i)])
            {
                converted = true;
                break;
            }
        }

        if (!converted)
        {
            isConvertible = false;

            if (!invalidCharacters)
            {
                // We are not storing invalid characters - we can break on first not convertible
                // character.
                break;
            }

            *invalidCharacters += character;
        }
    }

    return isConvertible;
}

bool PDFEncoding::canConvertFromEncoding(const QByteArray& stream, Encoding encoding)
{
    const encoding::EncodingTable* table = getTableForEncoding(encoding);
    for (const unsigned char index : stream)
    {
        QChar character = (*table)[index];
        if (character == QChar(0xfffd))
        {
            return false;
        }
    }

    return true;
}

QString PDFEncoding::convertTextString(const QByteArray& stream)
{
    if (hasUnicodeLeadMarkings(stream))
    {
        return convertFromUnicode(stream);
    }
    else if (hasUTF8LeadMarkings(stream))
    {
        return QString::fromUtf8(stream);
    }
    else
    {
        return convert(stream, Encoding::PDFDoc);
    }
}

QString PDFEncoding::convertFromUnicode(const QByteArray& stream)
{
    const char16_t* bytes = reinterpret_cast<const char16_t*>(stream.data());
    const int sizeInChars = stream.size();
    const size_t sizeSizeInUShorts = sizeInChars / sizeof(const ushort) * sizeof(char);

    return QString::fromUtf16(bytes, sizeSizeInUShorts);
}

QDateTime PDFEncoding::convertToDateTime(const QByteArray& stream)
{
    // According to the specification, string has form:
    //      (D:YYYYMMDDHHmmSSOHH'mm'), where
    //  YYYY    - year (0000-9999)
    //  MM      - month (1-12)
    //  DD      - day (01-31)
    //  HH      - hour (00-23)
    //  mm      - minute (00-59)
    //  SS      - second (00-59)
    //  O       - 'Z' or '+' or '-' means zero offset / positive offset / negative offset
    //  HH'     - hour offset
    //  mm'     - minute offset

    auto it = stream.cbegin();
    auto itEnd = stream.cend();

    constexpr const char* PREFIX = "D:";
    if (stream.startsWith(PREFIX))
    {
        std::advance(it, std::strlen(PREFIX));
    }

    auto readInteger = [&it, &itEnd](int size, int defaultValue) -> int
    {
        const int remaining = std::distance(it, itEnd);
        if (size <= remaining)
        {
            int value = 0;

            for (int i = 0; i < size; ++i)
            {
                const char currentChar = *it++;
                if (std::isdigit(static_cast<unsigned char>(currentChar)))
                {
                    value = value * 10 + currentChar - '0';
                }
                else
                {
                    // This means error - digit is supposed to be here
                    return -1;
                }
            }

            return value;
        }

        // No remaining part, use default value
        return defaultValue;
    };

    int year = readInteger(4, 0);
    int month = readInteger(2, 1);
    int day = readInteger(2, 1);
    int hour = readInteger(2, 0);
    int minute = readInteger(2, 0);
    int second = readInteger(2, 0);
    bool negative = it != itEnd && *it++ == '-';
    int hourOffset = readInteger(2, 0);
    if (it != itEnd)
    {
        // Skip ' character
        ++it;
    }
    int minuteOffset = readInteger(2, 0);

    const int offset = hourOffset * 3600 + minuteOffset * 60;

    QDate parsedDate(year, month, day);
    QTime parsedTime(hour, minute, second);
    QTimeZone parsedTimeZone(negative ? -offset : offset);

    if (parsedDate.isValid() && parsedTime.isValid() && parsedTimeZone.isValid())
    {
        return QDateTime(parsedDate, parsedTime, parsedTimeZone);
    }

    return QDateTime();
}

QByteArray PDFEncoding::convertDateTimeToString(QDateTime dateTime)
{
    QDateTime utcDateTime = dateTime.toUTC();
    QString convertedDateTime = QString("D:%1").arg(utcDateTime.toString("yyyyMMddhhmmss"));
    return convertedDateTime.toLatin1();
}

const encoding::EncodingTable* PDFEncoding::getTableForEncoding(Encoding encoding)
{
    switch (encoding)
    {
        case Encoding::Standard:
            return &pdf::encoding::STANDARD_ENCODING_CONVERSION_TABLE;

        case Encoding::MacRoman:
            return &pdf::encoding::MAC_ROMAN_ENCODING_CONVERSION_TABLE;

        case Encoding::WinAnsi:
            return &pdf::encoding::WIN_ANSI_ENCODING_CONVERSION_TABLE;

        case Encoding::PDFDoc:
            return &pdf::encoding::PDF_DOC_ENCODING_CONVERSION_TABLE;

        case Encoding::MacExpert:
            return &pdf::encoding::MAC_EXPERT_ENCODING_CONVERSION_TABLE;

        case Encoding::Symbol:
            return &pdf::encoding::SYMBOL_SET_ENCODING_CONVERSION_TABLE;

        case Encoding::ZapfDingbats:
            return &pdf::encoding::ZAPF_DINGBATS_ENCODING_CONVERSION_TABLE;

        case Encoding::MacOsRoman:
            return &pdf::encoding::MAC_OS_ENCODING_CONVERSION_TABLE;

        default:
            break;
    }

    // Unknown encoding?
    Q_ASSERT(false);
    return nullptr;
}

QString PDFEncoding::convertSmartFromByteStringToUnicode(const QByteArray& stream, bool* isBinary)
{
    if (isBinary)
    {
        *isBinary = false;
    }

    if (hasUnicodeLeadMarkings(stream))
    {
        {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            QStringDecoder decoder(QStringDecoder::Utf16BE);
            QString text = decoder.decode(stream);

            if (!decoder.hasError())
            {
                return text;
            }
#else
            QTextCodec *codec = QTextCodec::codecForName("UTF-16BE");
            QString text = codec->toUnicode(stream);
            // TODO -- how to detect errors?
            return text;
#endif
        }

        {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            QStringDecoder decoder(QStringDecoder::Utf16LE);
            QString text = decoder.decode(stream);

            if (!decoder.hasError())
            {
                return text;
            }
#else
            QTextCodec *codec = QTextCodec::codecForName("UTF-16LE");
            QString text = codec->toUnicode(stream);
            // TODO -- how to detect errors?
            return text;
#endif
        }
    }

    if (hasUTF8LeadMarkings(stream))
    {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QStringDecoder decoder(QStringDecoder::Utf8);
        QString text = decoder.decode(stream);

        if (!decoder.hasError())
        {
            return text;
        }
#else
        QTextCodec *codec = QTextCodec::codecForName("UTF-8");
        QString text = codec->toUnicode(stream);
        // TODO -- how to detect errors?
        return text;
#endif
    }

    if (canConvertFromEncoding(stream, Encoding::PDFDoc))
    {
        return convert(stream, Encoding::PDFDoc);
    }

    if (isBinary)
    {
        *isBinary = true;
    }
    return QString::fromLatin1(stream.toHex()).toUpper();
}

QString PDFEncoding::convertSmartFromByteStringToRepresentableQString(const QByteArray& stream)
{
    if (stream.startsWith("D:"))
    {
        QDateTime dateTime = convertToDateTime(stream);
        if (dateTime.isValid())
        {
            return dateTime.toString(Qt::TextDate);
        }
    }

    bool isBinary = false;
    QString text = convertSmartFromByteStringToUnicode(stream, &isBinary);

    if (!isBinary)
    {
        return text;
    }

    return stream.toPercentEncoding(" ", QByteArray(), '%');
}

QString PDFEncoding::getEncodingCharacters(Encoding encoding)
{
    QString string;

    if (const encoding::EncodingTable* table = getTableForEncoding(encoding))
    {
        for (const QChar& character : *table)
        {
            if (character != QChar(0xFFFD))
            {
                string += character;
            }
        }
    }

    return string;
}

QByteArray PDFEncoding::getPrintableCharacters()
{
    QByteArray result;

    const char min = std::numeric_limits<char>::min();
    const char max = std::numeric_limits<char>::max();
    for (char i = min; i < max; ++i)
    {
        if (std::isprint(static_cast<unsigned char>(i)))
        {
            result.push_back(i);
        }
    }

    return result;
}

bool PDFEncoding::hasUnicodeLeadMarkings(const QByteArray& stream)
{
    if (stream.size() >= 2)
    {
        if (static_cast<unsigned char>(stream[0]) == 0xFE && static_cast<unsigned char>(stream[1]) == 0xFF)
        {
            // UTF 16-BE
            return true;
        }
        if (static_cast<unsigned char>(stream[0]) == 0xFF && static_cast<unsigned char>(stream[1]) == 0xFE)
        {
            // UTF 16-LE, forbidden in PDF 2.0 standard, but used in some PDF producers (wrongly)
            return true;
        }
    }

    return false;
}

bool PDFEncoding::hasUTF8LeadMarkings(const QByteArray& stream)
{
    if (stream.size() >= 3)
    {
        if (static_cast<unsigned char>(stream[0]) == 239 &&
            static_cast<unsigned char>(stream[1]) == 187 &&
            static_cast<unsigned char>(stream[2]) == 191)
        {
            // UTF-8
            return true;
        }
    }

    return false;
}

} // namespace pdf
