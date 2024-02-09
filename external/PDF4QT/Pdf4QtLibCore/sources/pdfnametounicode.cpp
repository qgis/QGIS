//    Copyright (C) 2019-2022 Jakub Melka
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

#include "pdfnametounicode.h"
#include "pdfdbgheap.h"

#include <array>

namespace pdf
{

static constexpr const std::array<std::pair<QChar, const char*>, 4285>  glyphNameToUnicode = {
    std::pair<QChar, const char*>{ QChar(0x0041), "A" },                            // Character 'A' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x00C6), "AE" },                           // Character 'Æ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x01FC), "AEacute" },                      // Character 'Ǽ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x01E2), "AEmacron" },                     // Character 'Ǣ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF7E6), "AEsmall" },                      //
    std::pair<QChar, const char*>{ QChar(0x00C1), "Aacute" },                       // Character 'Á' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF7E1), "Aacutesmall" },                  //
    std::pair<QChar, const char*>{ QChar(0x0102), "Abreve" },                       // Character 'Ă' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1EAE), "Abreveacute" },                  // Character 'Ắ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x04D0), "Abrevecyrillic" },               // Character 'Ӑ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1EB6), "Abrevedotbelow" },               // Character 'Ặ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1EB0), "Abrevegrave" },                  // Character 'Ằ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1EB2), "Abrevehookabove" },              // Character 'Ẳ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1EB4), "Abrevetilde" },                  // Character 'Ẵ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x01CD), "Acaron" },                       // Character 'Ǎ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x24B6), "Acircle" },                      // Character 'Ⓐ' Symbol
    std::pair<QChar, const char*>{ QChar(0x00C2), "Acircumflex" },                  // Character 'Â' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1EA4), "Acircumflexacute" },             // Character 'Ấ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1EAC), "Acircumflexdotbelow" },          // Character 'Ậ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1EA6), "Acircumflexgrave" },             // Character 'Ầ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1EA8), "Acircumflexhookabove" },         // Character 'Ẩ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF7E2), "Acircumflexsmall" },             //
    std::pair<QChar, const char*>{ QChar(0x1EAA), "Acircumflextilde" },             // Character 'Ẫ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF6C9), "Acute" },                        //
    std::pair<QChar, const char*>{ QChar(0xF7B4), "Acutesmall" },                   //
    std::pair<QChar, const char*>{ QChar(0x0410), "Acyrillic" },                    // Character 'А' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0200), "Adblgrave" },                    // Character 'Ȁ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x00C4), "Adieresis" },                    // Character 'Ä' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x04D2), "Adieresiscyrillic" },            // Character 'Ӓ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x01DE), "Adieresismacron" },              // Character 'Ǟ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF7E4), "Adieresissmall" },               //
    std::pair<QChar, const char*>{ QChar(0x1EA0), "Adotbelow" },                    // Character 'Ạ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x01E0), "Adotmacron" },                   // Character 'Ǡ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x00C0), "Agrave" },                       // Character 'À' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF7E0), "Agravesmall" },                  //
    std::pair<QChar, const char*>{ QChar(0x1EA2), "Ahookabove" },                   // Character 'Ả' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x04D4), "Aiecyrillic" },                  // Character 'Ӕ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0202), "Ainvertedbreve" },               // Character 'Ȃ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0391), "Alpha" },                        // Character 'Α' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0386), "Alphatonos" },                   // Character 'Ά' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0100), "Amacron" },                      // Character 'Ā' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xFF21), "Amonospace" },                   // Character 'Ａ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0104), "Aogonek" },                      // Character 'Ą' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x00C5), "Aring" },                        // Character 'Å' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x01FA), "Aringacute" },                   // Character 'Ǻ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E00), "Aringbelow" },                   // Character 'Ḁ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF7E5), "Aringsmall" },                   //
    std::pair<QChar, const char*>{ QChar(0xF761), "Asmall" },                       //
    std::pair<QChar, const char*>{ QChar(0x00C3), "Atilde" },                       // Character 'Ã' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF7E3), "Atildesmall" },                  //
    std::pair<QChar, const char*>{ QChar(0x0531), "Aybarmenian" },                  // Character 'Ա' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0042), "B" },                            // Character 'B' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x24B7), "Bcircle" },                      // Character 'Ⓑ' Symbol
    std::pair<QChar, const char*>{ QChar(0x1E02), "Bdotaccent" },                   // Character 'Ḃ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E04), "Bdotbelow" },                    // Character 'Ḅ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0411), "Becyrillic" },                   // Character 'Б' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0532), "Benarmenian" },                  // Character 'Բ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0392), "Beta" },                         // Character 'Β' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0181), "Bhook" },                        // Character 'Ɓ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E06), "Blinebelow" },                   // Character 'Ḇ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xFF22), "Bmonospace" },                   // Character 'Ｂ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF6F4), "Brevesmall" },                   //
    std::pair<QChar, const char*>{ QChar(0xF762), "Bsmall" },                       //
    std::pair<QChar, const char*>{ QChar(0x0182), "Btopbar" },                      // Character 'Ƃ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0043), "C" },                            // Character 'C' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x053E), "Caarmenian" },                   // Character 'Ծ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0106), "Cacute" },                       // Character 'Ć' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF6CA), "Caron" },                        //
    std::pair<QChar, const char*>{ QChar(0xF6F5), "Caronsmall" },                   //
    std::pair<QChar, const char*>{ QChar(0x010C), "Ccaron" },                       // Character 'Č' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x00C7), "Ccedilla" },                     // Character 'Ç' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E08), "Ccedillaacute" },                // Character 'Ḉ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF7E7), "Ccedillasmall" },                //
    std::pair<QChar, const char*>{ QChar(0x24B8), "Ccircle" },                      // Character 'Ⓒ' Symbol
    std::pair<QChar, const char*>{ QChar(0x0108), "Ccircumflex" },                  // Character 'Ĉ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x010A), "Cdot" },                         // Character 'Ċ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x010A), "Cdotaccent" },                   // Character 'Ċ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF7B8), "Cedillasmall" },                 //
    std::pair<QChar, const char*>{ QChar(0x0549), "Chaarmenian" },                  // Character 'Չ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x04BC), "Cheabkhasiancyrillic" },         // Character 'Ҽ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0427), "Checyrillic" },                  // Character 'Ч' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x04BE), "Chedescenderabkhasiancyrillic" },// Character 'Ҿ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x04B6), "Chedescendercyrillic" },         // Character 'Ҷ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x04F4), "Chedieresiscyrillic" },          // Character 'Ӵ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0543), "Cheharmenian" },                 // Character 'Ճ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x04CB), "Chekhakassiancyrillic" },        // Character 'Ӌ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x04B8), "Cheverticalstrokecyrillic" },    // Character 'Ҹ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x03A7), "Chi" },                          // Character 'Χ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0187), "Chook" },                        // Character 'Ƈ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF6F6), "Circumflexsmall" },              //
    std::pair<QChar, const char*>{ QChar(0xFF23), "Cmonospace" },                   // Character 'Ｃ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0551), "Coarmenian" },                   // Character 'Ց' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF763), "Csmall" },                       //
    std::pair<QChar, const char*>{ QChar(0x0044), "D" },                            // Character 'D' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x01F1), "DZ" },                           // Character 'Ǳ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x01C4), "DZcaron" },                      // Character 'Ǆ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0534), "Daarmenian" },                   // Character 'Դ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0189), "Dafrican" },                     // Character 'Ɖ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x010E), "Dcaron" },                       // Character 'Ď' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E10), "Dcedilla" },                     // Character 'Ḑ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x24B9), "Dcircle" },                      // Character 'Ⓓ' Symbol
    std::pair<QChar, const char*>{ QChar(0x1E12), "Dcircumflexbelow" },             // Character 'Ḓ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0110), "Dcroat" },                       // Character 'Đ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E0A), "Ddotaccent" },                   // Character 'Ḋ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E0C), "Ddotbelow" },                    // Character 'Ḍ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0414), "Decyrillic" },                   // Character 'Д' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x03EE), "Deicoptic" },                    // Character 'Ϯ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x2206), "Delta" },                        // Character '∆' Symbol
    std::pair<QChar, const char*>{ QChar(0x0394), "Deltagreek" },                   // Character 'Δ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x018A), "Dhook" },                        // Character 'Ɗ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF6CB), "Dieresis" },                     //
    std::pair<QChar, const char*>{ QChar(0xF6CC), "DieresisAcute" },                //
    std::pair<QChar, const char*>{ QChar(0xF6CD), "DieresisGrave" },                //
    std::pair<QChar, const char*>{ QChar(0xF7A8), "Dieresissmall" },                //
    std::pair<QChar, const char*>{ QChar(0x03DC), "Digammagreek" },                 // Character 'Ϝ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0402), "Djecyrillic" },                  // Character 'Ђ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E0E), "Dlinebelow" },                   // Character 'Ḏ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xFF24), "Dmonospace" },                   // Character 'Ｄ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF6F7), "Dotaccentsmall" },               //
    std::pair<QChar, const char*>{ QChar(0x0110), "Dslash" },                       // Character 'Đ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF764), "Dsmall" },                       //
    std::pair<QChar, const char*>{ QChar(0x018B), "Dtopbar" },                      // Character 'Ƌ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x01F2), "Dz" },                           // Character 'ǲ' Letter, Title case
    std::pair<QChar, const char*>{ QChar(0x01C5), "Dzcaron" },                      // Character 'ǅ' Letter, Title case
    std::pair<QChar, const char*>{ QChar(0x04E0), "Dzeabkhasiancyrillic" },         // Character 'Ӡ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0405), "Dzecyrillic" },                  // Character 'Ѕ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x040F), "Dzhecyrillic" },                 // Character 'Џ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0045), "E" },                            // Character 'E' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x00C9), "Eacute" },                       // Character 'É' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF7E9), "Eacutesmall" },                  //
    std::pair<QChar, const char*>{ QChar(0x0114), "Ebreve" },                       // Character 'Ĕ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x011A), "Ecaron" },                       // Character 'Ě' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E1C), "Ecedillabreve" },                // Character 'Ḝ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0535), "Echarmenian" },                  // Character 'Ե' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x24BA), "Ecircle" },                      // Character 'Ⓔ' Symbol
    std::pair<QChar, const char*>{ QChar(0x00CA), "Ecircumflex" },                  // Character 'Ê' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1EBE), "Ecircumflexacute" },             // Character 'Ế' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E18), "Ecircumflexbelow" },             // Character 'Ḙ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1EC6), "Ecircumflexdotbelow" },          // Character 'Ệ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1EC0), "Ecircumflexgrave" },             // Character 'Ề' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1EC2), "Ecircumflexhookabove" },         // Character 'Ể' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF7EA), "Ecircumflexsmall" },             //
    std::pair<QChar, const char*>{ QChar(0x1EC4), "Ecircumflextilde" },             // Character 'Ễ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0404), "Ecyrillic" },                    // Character 'Є' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0204), "Edblgrave" },                    // Character 'Ȅ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x00CB), "Edieresis" },                    // Character 'Ë' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF7EB), "Edieresissmall" },               //
    std::pair<QChar, const char*>{ QChar(0x0116), "Edot" },                         // Character 'Ė' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0116), "Edotaccent" },                   // Character 'Ė' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1EB8), "Edotbelow" },                    // Character 'Ẹ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0424), "Efcyrillic" },                   // Character 'Ф' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x00C8), "Egrave" },                       // Character 'È' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF7E8), "Egravesmall" },                  //
    std::pair<QChar, const char*>{ QChar(0x0537), "Eharmenian" },                   // Character 'Է' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1EBA), "Ehookabove" },                   // Character 'Ẻ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x2167), "Eightroman" },                   // Character 'Ⅷ'
    std::pair<QChar, const char*>{ QChar(0x0206), "Einvertedbreve" },               // Character 'Ȇ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0464), "Eiotifiedcyrillic" },            // Character 'Ѥ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x041B), "Elcyrillic" },                   // Character 'Л' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x216A), "Elevenroman" },                  // Character 'Ⅺ'
    std::pair<QChar, const char*>{ QChar(0x0112), "Emacron" },                      // Character 'Ē' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E16), "Emacronacute" },                 // Character 'Ḗ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E14), "Emacrongrave" },                 // Character 'Ḕ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x041C), "Emcyrillic" },                   // Character 'М' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xFF25), "Emonospace" },                   // Character 'Ｅ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x041D), "Encyrillic" },                   // Character 'Н' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x04A2), "Endescendercyrillic" },          // Character 'Ң' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x014A), "Eng" },                          // Character 'Ŋ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x04A4), "Enghecyrillic" },                // Character 'Ҥ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x04C7), "Enhookcyrillic" },               // Character 'Ӈ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0118), "Eogonek" },                      // Character 'Ę' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0190), "Eopen" },                        // Character 'Ɛ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0395), "Epsilon" },                      // Character 'Ε' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0388), "Epsilontonos" },                 // Character 'Έ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0420), "Ercyrillic" },                   // Character 'Р' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x018E), "Ereversed" },                    // Character 'Ǝ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x042D), "Ereversedcyrillic" },            // Character 'Э' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0421), "Escyrillic" },                   // Character 'С' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x04AA), "Esdescendercyrillic" },          // Character 'Ҫ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x01A9), "Esh" },                          // Character 'Ʃ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF765), "Esmall" },                       //
    std::pair<QChar, const char*>{ QChar(0x0397), "Eta" },                          // Character 'Η' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0538), "Etarmenian" },                   // Character 'Ը' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0389), "Etatonos" },                     // Character 'Ή' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x00D0), "Eth" },                          // Character 'Ð' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF7F0), "Ethsmall" },                     //
    std::pair<QChar, const char*>{ QChar(0x1EBC), "Etilde" },                       // Character 'Ẽ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E1A), "Etildebelow" },                  // Character 'Ḛ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x20AC), "Euro" },                         // Character '€' Symbol
    std::pair<QChar, const char*>{ QChar(0x01B7), "Ezh" },                          // Character 'Ʒ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x01EE), "Ezhcaron" },                     // Character 'Ǯ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x01B8), "Ezhreversed" },                  // Character 'Ƹ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0046), "F" },                            // Character 'F' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x24BB), "Fcircle" },                      // Character 'Ⓕ' Symbol
    std::pair<QChar, const char*>{ QChar(0x1E1E), "Fdotaccent" },                   // Character 'Ḟ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0556), "Feharmenian" },                  // Character 'Ֆ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x03E4), "Feicoptic" },                    // Character 'Ϥ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0191), "Fhook" },                        // Character 'Ƒ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0472), "Fitacyrillic" },                 // Character 'Ѳ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x2164), "Fiveroman" },                    // Character 'Ⅴ'
    std::pair<QChar, const char*>{ QChar(0xFF26), "Fmonospace" },                   // Character 'Ｆ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x2163), "Fourroman" },                    // Character 'Ⅳ'
    std::pair<QChar, const char*>{ QChar(0xF766), "Fsmall" },                       //
    std::pair<QChar, const char*>{ QChar(0x0047), "G" },                            // Character 'G' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x3387), "GBsquare" },                     // Character '㎇' Symbol
    std::pair<QChar, const char*>{ QChar(0x01F4), "Gacute" },                       // Character 'Ǵ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0393), "Gamma" },                        // Character 'Γ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0194), "Gammaafrican" },                 // Character 'Ɣ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x03EA), "Gangiacoptic" },                 // Character 'Ϫ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x011E), "Gbreve" },                       // Character 'Ğ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x01E6), "Gcaron" },                       // Character 'Ǧ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0122), "Gcedilla" },                     // Character 'Ģ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x24BC), "Gcircle" },                      // Character 'Ⓖ' Symbol
    std::pair<QChar, const char*>{ QChar(0x011C), "Gcircumflex" },                  // Character 'Ĝ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0122), "Gcommaaccent" },                 // Character 'Ģ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0120), "Gdot" },                         // Character 'Ġ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0120), "Gdotaccent" },                   // Character 'Ġ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0413), "Gecyrillic" },                   // Character 'Г' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0542), "Ghadarmenian" },                 // Character 'Ղ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0494), "Ghemiddlehookcyrillic" },        // Character 'Ҕ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0492), "Ghestrokecyrillic" },            // Character 'Ғ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0490), "Gheupturncyrillic" },            // Character 'Ґ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0193), "Ghook" },                        // Character 'Ɠ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0533), "Gimarmenian" },                  // Character 'Գ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0403), "Gjecyrillic" },                  // Character 'Ѓ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E20), "Gmacron" },                      // Character 'Ḡ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xFF27), "Gmonospace" },                   // Character 'Ｇ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF6CE), "Grave" },                        //
    std::pair<QChar, const char*>{ QChar(0xF760), "Gravesmall" },                   //
    std::pair<QChar, const char*>{ QChar(0xF767), "Gsmall" },                       //
    std::pair<QChar, const char*>{ QChar(0x029B), "Gsmallhook" },                   // Character 'ʛ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x01E4), "Gstroke" },                      // Character 'Ǥ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0048), "H" },                            // Character 'H' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x25CF), "H18533" },                       // Character '●' Symbol
    std::pair<QChar, const char*>{ QChar(0x25AA), "H18543" },                       // Character '▪' Symbol
    std::pair<QChar, const char*>{ QChar(0x25AB), "H18551" },                       // Character '▫' Symbol
    std::pair<QChar, const char*>{ QChar(0x25A1), "H22073" },                       // Character '□' Symbol
    std::pair<QChar, const char*>{ QChar(0x33CB), "HPsquare" },                     // Character '㏋' Symbol
    std::pair<QChar, const char*>{ QChar(0x04A8), "Haabkhasiancyrillic" },          // Character 'Ҩ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x04B2), "Hadescendercyrillic" },          // Character 'Ҳ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x042A), "Hardsigncyrillic" },             // Character 'Ъ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0126), "Hbar" },                         // Character 'Ħ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E2A), "Hbrevebelow" },                  // Character 'Ḫ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E28), "Hcedilla" },                     // Character 'Ḩ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x24BD), "Hcircle" },                      // Character 'Ⓗ' Symbol
    std::pair<QChar, const char*>{ QChar(0x0124), "Hcircumflex" },                  // Character 'Ĥ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E26), "Hdieresis" },                    // Character 'Ḧ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E22), "Hdotaccent" },                   // Character 'Ḣ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E24), "Hdotbelow" },                    // Character 'Ḥ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xFF28), "Hmonospace" },                   // Character 'Ｈ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0540), "Hoarmenian" },                   // Character 'Հ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x03E8), "Horicoptic" },                   // Character 'Ϩ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF768), "Hsmall" },                       //
    std::pair<QChar, const char*>{ QChar(0xF6CF), "Hungarumlaut" },                 //
    std::pair<QChar, const char*>{ QChar(0xF6F8), "Hungarumlautsmall" },            //
    std::pair<QChar, const char*>{ QChar(0x3390), "Hzsquare" },                     // Character '㎐' Symbol
    std::pair<QChar, const char*>{ QChar(0x0049), "I" },                            // Character 'I' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x042F), "IAcyrillic" },                   // Character 'Я' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0132), "IJ" },                           // Character 'Ĳ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x042E), "IUcyrillic" },                   // Character 'Ю' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x00CD), "Iacute" },                       // Character 'Í' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF7ED), "Iacutesmall" },                  //
    std::pair<QChar, const char*>{ QChar(0x012C), "Ibreve" },                       // Character 'Ĭ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x01CF), "Icaron" },                       // Character 'Ǐ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x24BE), "Icircle" },                      // Character 'Ⓘ' Symbol
    std::pair<QChar, const char*>{ QChar(0x00CE), "Icircumflex" },                  // Character 'Î' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF7EE), "Icircumflexsmall" },             //
    std::pair<QChar, const char*>{ QChar(0x0406), "Icyrillic" },                    // Character 'І' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0208), "Idblgrave" },                    // Character 'Ȉ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x00CF), "Idieresis" },                    // Character 'Ï' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E2E), "Idieresisacute" },               // Character 'Ḯ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x04E4), "Idieresiscyrillic" },            // Character 'Ӥ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF7EF), "Idieresissmall" },               //
    std::pair<QChar, const char*>{ QChar(0x0130), "Idot" },                         // Character 'İ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0130), "Idotaccent" },                   // Character 'İ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1ECA), "Idotbelow" },                    // Character 'Ị' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x04D6), "Iebrevecyrillic" },              // Character 'Ӗ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0415), "Iecyrillic" },                   // Character 'Е' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x2111), "Ifraktur" },                     // Character 'ℑ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x00CC), "Igrave" },                       // Character 'Ì' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF7EC), "Igravesmall" },                  //
    std::pair<QChar, const char*>{ QChar(0x1EC8), "Ihookabove" },                   // Character 'Ỉ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0418), "Iicyrillic" },                   // Character 'И' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x020A), "Iinvertedbreve" },               // Character 'Ȋ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0419), "Iishortcyrillic" },              // Character 'Й' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x012A), "Imacron" },                      // Character 'Ī' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x04E2), "Imacroncyrillic" },              // Character 'Ӣ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xFF29), "Imonospace" },                   // Character 'Ｉ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x053B), "Iniarmenian" },                  // Character 'Ի' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0401), "Iocyrillic" },                   // Character 'Ё' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x012E), "Iogonek" },                      // Character 'Į' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0399), "Iota" },                         // Character 'Ι' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0196), "Iotaafrican" },                  // Character 'Ɩ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x03AA), "Iotadieresis" },                 // Character 'Ϊ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x038A), "Iotatonos" },                    // Character 'Ί' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF769), "Ismall" },                       //
    std::pair<QChar, const char*>{ QChar(0x0197), "Istroke" },                      // Character 'Ɨ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0128), "Itilde" },                       // Character 'Ĩ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E2C), "Itildebelow" },                  // Character 'Ḭ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0474), "Izhitsacyrillic" },              // Character 'Ѵ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0476), "Izhitsadblgravecyrillic" },      // Character 'Ѷ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x004A), "J" },                            // Character 'J' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0541), "Jaarmenian" },                   // Character 'Ձ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x24BF), "Jcircle" },                      // Character 'Ⓙ' Symbol
    std::pair<QChar, const char*>{ QChar(0x0134), "Jcircumflex" },                  // Character 'Ĵ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0408), "Jecyrillic" },                   // Character 'Ј' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x054B), "Jheharmenian" },                 // Character 'Ջ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xFF2A), "Jmonospace" },                   // Character 'Ｊ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF76A), "Jsmall" },                       //
    std::pair<QChar, const char*>{ QChar(0x004B), "K" },                            // Character 'K' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x3385), "KBsquare" },                     // Character '㎅' Symbol
    std::pair<QChar, const char*>{ QChar(0x33CD), "KKsquare" },                     // Character '㏍' Symbol
    std::pair<QChar, const char*>{ QChar(0x04A0), "Kabashkircyrillic" },            // Character 'Ҡ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E30), "Kacute" },                       // Character 'Ḱ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x041A), "Kacyrillic" },                   // Character 'К' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x049A), "Kadescendercyrillic" },          // Character 'Қ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x04C3), "Kahookcyrillic" },               // Character 'Ӄ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x039A), "Kappa" },                        // Character 'Κ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x049E), "Kastrokecyrillic" },             // Character 'Ҟ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x049C), "Kaverticalstrokecyrillic" },     // Character 'Ҝ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x01E8), "Kcaron" },                       // Character 'Ǩ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0136), "Kcedilla" },                     // Character 'Ķ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x24C0), "Kcircle" },                      // Character 'Ⓚ' Symbol
    std::pair<QChar, const char*>{ QChar(0x0136), "Kcommaaccent" },                 // Character 'Ķ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E32), "Kdotbelow" },                    // Character 'Ḳ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0554), "Keharmenian" },                  // Character 'Ք' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x053F), "Kenarmenian" },                  // Character 'Կ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0425), "Khacyrillic" },                  // Character 'Х' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x03E6), "Kheicoptic" },                   // Character 'Ϧ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0198), "Khook" },                        // Character 'Ƙ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x040C), "Kjecyrillic" },                  // Character 'Ќ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E34), "Klinebelow" },                   // Character 'Ḵ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xFF2B), "Kmonospace" },                   // Character 'Ｋ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0480), "Koppacyrillic" },                // Character 'Ҁ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x03DE), "Koppagreek" },                   // Character 'Ϟ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x046E), "Ksicyrillic" },                  // Character 'Ѯ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF76B), "Ksmall" },                       //
    std::pair<QChar, const char*>{ QChar(0x004C), "L" },                            // Character 'L' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x01C7), "LJ" },                           // Character 'Ǉ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF6BF), "LL" },                           //
    std::pair<QChar, const char*>{ QChar(0x0139), "Lacute" },                       // Character 'Ĺ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x039B), "Lambda" },                       // Character 'Λ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x013D), "Lcaron" },                       // Character 'Ľ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x013B), "Lcedilla" },                     // Character 'Ļ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x24C1), "Lcircle" },                      // Character 'Ⓛ' Symbol
    std::pair<QChar, const char*>{ QChar(0x1E3C), "Lcircumflexbelow" },             // Character 'Ḽ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x013B), "Lcommaaccent" },                 // Character 'Ļ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x013F), "Ldot" },                         // Character 'Ŀ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x013F), "Ldotaccent" },                   // Character 'Ŀ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E36), "Ldotbelow" },                    // Character 'Ḷ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E38), "Ldotbelowmacron" },              // Character 'Ḹ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x053C), "Liwnarmenian" },                 // Character 'Լ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x01C8), "Lj" },                           // Character 'ǈ' Letter, Title case
    std::pair<QChar, const char*>{ QChar(0x0409), "Ljecyrillic" },                  // Character 'Љ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E3A), "Llinebelow" },                   // Character 'Ḻ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xFF2C), "Lmonospace" },                   // Character 'Ｌ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0141), "Lslash" },                       // Character 'Ł' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF6F9), "Lslashsmall" },                  //
    std::pair<QChar, const char*>{ QChar(0xF76C), "Lsmall" },                       //
    std::pair<QChar, const char*>{ QChar(0x004D), "M" },                            // Character 'M' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x3386), "MBsquare" },                     // Character '㎆' Symbol
    std::pair<QChar, const char*>{ QChar(0xF6D0), "Macron" },                       //
    std::pair<QChar, const char*>{ QChar(0xF7AF), "Macronsmall" },                  //
    std::pair<QChar, const char*>{ QChar(0x1E3E), "Macute" },                       // Character 'Ḿ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x24C2), "Mcircle" },                      // Character 'Ⓜ' Symbol
    std::pair<QChar, const char*>{ QChar(0x1E40), "Mdotaccent" },                   // Character 'Ṁ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E42), "Mdotbelow" },                    // Character 'Ṃ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0544), "Menarmenian" },                  // Character 'Մ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xFF2D), "Mmonospace" },                   // Character 'Ｍ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF76D), "Msmall" },                       //
    std::pair<QChar, const char*>{ QChar(0x019C), "Mturned" },                      // Character 'Ɯ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x039C), "Mu" },                           // Character 'Μ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x004E), "N" },                            // Character 'N' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x01CA), "NJ" },                           // Character 'Ǌ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0143), "Nacute" },                       // Character 'Ń' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0147), "Ncaron" },                       // Character 'Ň' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0145), "Ncedilla" },                     // Character 'Ņ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x24C3), "Ncircle" },                      // Character 'Ⓝ' Symbol
    std::pair<QChar, const char*>{ QChar(0x1E4A), "Ncircumflexbelow" },             // Character 'Ṋ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0145), "Ncommaaccent" },                 // Character 'Ņ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E44), "Ndotaccent" },                   // Character 'Ṅ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E46), "Ndotbelow" },                    // Character 'Ṇ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x019D), "Nhookleft" },                    // Character 'Ɲ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x2168), "Nineroman" },                    // Character 'Ⅸ'
    std::pair<QChar, const char*>{ QChar(0x01CB), "Nj" },                           // Character 'ǋ' Letter, Title case
    std::pair<QChar, const char*>{ QChar(0x040A), "Njecyrillic" },                  // Character 'Њ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E48), "Nlinebelow" },                   // Character 'Ṉ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xFF2E), "Nmonospace" },                   // Character 'Ｎ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0546), "Nowarmenian" },                  // Character 'Ն' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF76E), "Nsmall" },                       //
    std::pair<QChar, const char*>{ QChar(0x00D1), "Ntilde" },                       // Character 'Ñ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF7F1), "Ntildesmall" },                  //
    std::pair<QChar, const char*>{ QChar(0x039D), "Nu" },                           // Character 'Ν' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x004F), "O" },                            // Character 'O' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0152), "OE" },                           // Character 'Œ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF6FA), "OEsmall" },                      //
    std::pair<QChar, const char*>{ QChar(0x00D3), "Oacute" },                       // Character 'Ó' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF7F3), "Oacutesmall" },                  //
    std::pair<QChar, const char*>{ QChar(0x04E8), "Obarredcyrillic" },              // Character 'Ө' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x04EA), "Obarreddieresiscyrillic" },      // Character 'Ӫ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x014E), "Obreve" },                       // Character 'Ŏ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x01D1), "Ocaron" },                       // Character 'Ǒ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x019F), "Ocenteredtilde" },               // Character 'Ɵ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x24C4), "Ocircle" },                      // Character 'Ⓞ' Symbol
    std::pair<QChar, const char*>{ QChar(0x00D4), "Ocircumflex" },                  // Character 'Ô' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1ED0), "Ocircumflexacute" },             // Character 'Ố' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1ED8), "Ocircumflexdotbelow" },          // Character 'Ộ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1ED2), "Ocircumflexgrave" },             // Character 'Ồ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1ED4), "Ocircumflexhookabove" },         // Character 'Ổ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF7F4), "Ocircumflexsmall" },             //
    std::pair<QChar, const char*>{ QChar(0x1ED6), "Ocircumflextilde" },             // Character 'Ỗ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x041E), "Ocyrillic" },                    // Character 'О' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0150), "Odblacute" },                    // Character 'Ő' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x020C), "Odblgrave" },                    // Character 'Ȍ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x00D6), "Odieresis" },                    // Character 'Ö' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x04E6), "Odieresiscyrillic" },            // Character 'Ӧ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF7F6), "Odieresissmall" },               //
    std::pair<QChar, const char*>{ QChar(0x1ECC), "Odotbelow" },                    // Character 'Ọ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF6FB), "Ogoneksmall" },                  //
    std::pair<QChar, const char*>{ QChar(0x00D2), "Ograve" },                       // Character 'Ò' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF7F2), "Ogravesmall" },                  //
    std::pair<QChar, const char*>{ QChar(0x0555), "Oharmenian" },                   // Character 'Օ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x2126), "Ohm" },                          // Character 'Ω' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1ECE), "Ohookabove" },                   // Character 'Ỏ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x01A0), "Ohorn" },                        // Character 'Ơ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1EDA), "Ohornacute" },                   // Character 'Ớ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1EE2), "Ohorndotbelow" },                // Character 'Ợ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1EDC), "Ohorngrave" },                   // Character 'Ờ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1EDE), "Ohornhookabove" },               // Character 'Ở' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1EE0), "Ohorntilde" },                   // Character 'Ỡ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0150), "Ohungarumlaut" },                // Character 'Ő' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x01A2), "Oi" },                           // Character 'Ƣ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x020E), "Oinvertedbreve" },               // Character 'Ȏ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x014C), "Omacron" },                      // Character 'Ō' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E52), "Omacronacute" },                 // Character 'Ṓ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E50), "Omacrongrave" },                 // Character 'Ṑ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x2126), "Omega" },                        // Character 'Ω' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0460), "Omegacyrillic" },                // Character 'Ѡ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x03A9), "Omegagreek" },                   // Character 'Ω' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x047A), "Omegaroundcyrillic" },           // Character 'Ѻ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x047C), "Omegatitlocyrillic" },           // Character 'Ѽ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x038F), "Omegatonos" },                   // Character 'Ώ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x039F), "Omicron" },                      // Character 'Ο' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x038C), "Omicrontonos" },                 // Character 'Ό' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xFF2F), "Omonospace" },                   // Character 'Ｏ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x2160), "Oneroman" },                     // Character 'Ⅰ'
    std::pair<QChar, const char*>{ QChar(0x01EA), "Oogonek" },                      // Character 'Ǫ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x01EC), "Oogonekmacron" },                // Character 'Ǭ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0186), "Oopen" },                        // Character 'Ɔ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x00D8), "Oslash" },                       // Character 'Ø' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x01FE), "Oslashacute" },                  // Character 'Ǿ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF7F8), "Oslashsmall" },                  //
    std::pair<QChar, const char*>{ QChar(0xF76F), "Osmall" },                       //
    std::pair<QChar, const char*>{ QChar(0x01FE), "Ostrokeacute" },                 // Character 'Ǿ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x047E), "Otcyrillic" },                   // Character 'Ѿ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x00D5), "Otilde" },                       // Character 'Õ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E4C), "Otildeacute" },                  // Character 'Ṍ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E4E), "Otildedieresis" },               // Character 'Ṏ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF7F5), "Otildesmall" },                  //
    std::pair<QChar, const char*>{ QChar(0x0050), "P" },                            // Character 'P' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E54), "Pacute" },                       // Character 'Ṕ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x24C5), "Pcircle" },                      // Character 'Ⓟ' Symbol
    std::pair<QChar, const char*>{ QChar(0x1E56), "Pdotaccent" },                   // Character 'Ṗ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x041F), "Pecyrillic" },                   // Character 'П' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x054A), "Peharmenian" },                  // Character 'Պ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x04A6), "Pemiddlehookcyrillic" },         // Character 'Ҧ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x03A6), "Phi" },                          // Character 'Φ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x01A4), "Phook" },                        // Character 'Ƥ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x03A0), "Pi" },                           // Character 'Π' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0553), "Piwrarmenian" },                 // Character 'Փ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xFF30), "Pmonospace" },                   // Character 'Ｐ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x03A8), "Psi" },                          // Character 'Ψ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0470), "Psicyrillic" },                  // Character 'Ѱ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF770), "Psmall" },                       //
    std::pair<QChar, const char*>{ QChar(0x0051), "Q" },                            // Character 'Q' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x24C6), "Qcircle" },                      // Character 'Ⓠ' Symbol
    std::pair<QChar, const char*>{ QChar(0xFF31), "Qmonospace" },                   // Character 'Ｑ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF771), "Qsmall" },                       //
    std::pair<QChar, const char*>{ QChar(0x0052), "R" },                            // Character 'R' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x054C), "Raarmenian" },                   // Character 'Ռ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0154), "Racute" },                       // Character 'Ŕ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0158), "Rcaron" },                       // Character 'Ř' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0156), "Rcedilla" },                     // Character 'Ŗ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x24C7), "Rcircle" },                      // Character 'Ⓡ' Symbol
    std::pair<QChar, const char*>{ QChar(0x0156), "Rcommaaccent" },                 // Character 'Ŗ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0210), "Rdblgrave" },                    // Character 'Ȑ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E58), "Rdotaccent" },                   // Character 'Ṙ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E5A), "Rdotbelow" },                    // Character 'Ṛ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E5C), "Rdotbelowmacron" },              // Character 'Ṝ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0550), "Reharmenian" },                  // Character 'Ր' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x211C), "Rfraktur" },                     // Character 'ℜ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x03A1), "Rho" },                          // Character 'Ρ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF6FC), "Ringsmall" },                    //
    std::pair<QChar, const char*>{ QChar(0x0212), "Rinvertedbreve" },               // Character 'Ȓ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E5E), "Rlinebelow" },                   // Character 'Ṟ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xFF32), "Rmonospace" },                   // Character 'Ｒ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF772), "Rsmall" },                       //
    std::pair<QChar, const char*>{ QChar(0x0281), "Rsmallinverted" },               // Character 'ʁ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x02B6), "Rsmallinvertedsuperior" },       // Character 'ʶ' Letter
    std::pair<QChar, const char*>{ QChar(0x0053), "S" },                            // Character 'S' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x250C), "SF010000" },                     // Character '┌' Symbol
    std::pair<QChar, const char*>{ QChar(0x2514), "SF020000" },                     // Character '└' Symbol
    std::pair<QChar, const char*>{ QChar(0x2510), "SF030000" },                     // Character '┐' Symbol
    std::pair<QChar, const char*>{ QChar(0x2518), "SF040000" },                     // Character '┘' Symbol
    std::pair<QChar, const char*>{ QChar(0x253C), "SF050000" },                     // Character '┼' Symbol
    std::pair<QChar, const char*>{ QChar(0x252C), "SF060000" },                     // Character '┬' Symbol
    std::pair<QChar, const char*>{ QChar(0x2534), "SF070000" },                     // Character '┴' Symbol
    std::pair<QChar, const char*>{ QChar(0x251C), "SF080000" },                     // Character '├' Symbol
    std::pair<QChar, const char*>{ QChar(0x2524), "SF090000" },                     // Character '┤' Symbol
    std::pair<QChar, const char*>{ QChar(0x2500), "SF100000" },                     // Character '─' Symbol
    std::pair<QChar, const char*>{ QChar(0x2502), "SF110000" },                     // Character '│' Symbol
    std::pair<QChar, const char*>{ QChar(0x2561), "SF190000" },                     // Character '╡' Symbol
    std::pair<QChar, const char*>{ QChar(0x2562), "SF200000" },                     // Character '╢' Symbol
    std::pair<QChar, const char*>{ QChar(0x2556), "SF210000" },                     // Character '╖' Symbol
    std::pair<QChar, const char*>{ QChar(0x2555), "SF220000" },                     // Character '╕' Symbol
    std::pair<QChar, const char*>{ QChar(0x2563), "SF230000" },                     // Character '╣' Symbol
    std::pair<QChar, const char*>{ QChar(0x2551), "SF240000" },                     // Character '║' Symbol
    std::pair<QChar, const char*>{ QChar(0x2557), "SF250000" },                     // Character '╗' Symbol
    std::pair<QChar, const char*>{ QChar(0x255D), "SF260000" },                     // Character '╝' Symbol
    std::pair<QChar, const char*>{ QChar(0x255C), "SF270000" },                     // Character '╜' Symbol
    std::pair<QChar, const char*>{ QChar(0x255B), "SF280000" },                     // Character '╛' Symbol
    std::pair<QChar, const char*>{ QChar(0x255E), "SF360000" },                     // Character '╞' Symbol
    std::pair<QChar, const char*>{ QChar(0x255F), "SF370000" },                     // Character '╟' Symbol
    std::pair<QChar, const char*>{ QChar(0x255A), "SF380000" },                     // Character '╚' Symbol
    std::pair<QChar, const char*>{ QChar(0x2554), "SF390000" },                     // Character '╔' Symbol
    std::pair<QChar, const char*>{ QChar(0x2569), "SF400000" },                     // Character '╩' Symbol
    std::pair<QChar, const char*>{ QChar(0x2566), "SF410000" },                     // Character '╦' Symbol
    std::pair<QChar, const char*>{ QChar(0x2560), "SF420000" },                     // Character '╠' Symbol
    std::pair<QChar, const char*>{ QChar(0x2550), "SF430000" },                     // Character '═' Symbol
    std::pair<QChar, const char*>{ QChar(0x256C), "SF440000" },                     // Character '╬' Symbol
    std::pair<QChar, const char*>{ QChar(0x2567), "SF450000" },                     // Character '╧' Symbol
    std::pair<QChar, const char*>{ QChar(0x2568), "SF460000" },                     // Character '╨' Symbol
    std::pair<QChar, const char*>{ QChar(0x2564), "SF470000" },                     // Character '╤' Symbol
    std::pair<QChar, const char*>{ QChar(0x2565), "SF480000" },                     // Character '╥' Symbol
    std::pair<QChar, const char*>{ QChar(0x2559), "SF490000" },                     // Character '╙' Symbol
    std::pair<QChar, const char*>{ QChar(0x2558), "SF500000" },                     // Character '╘' Symbol
    std::pair<QChar, const char*>{ QChar(0x2552), "SF510000" },                     // Character '╒' Symbol
    std::pair<QChar, const char*>{ QChar(0x2553), "SF520000" },                     // Character '╓' Symbol
    std::pair<QChar, const char*>{ QChar(0x256B), "SF530000" },                     // Character '╫' Symbol
    std::pair<QChar, const char*>{ QChar(0x256A), "SF540000" },                     // Character '╪' Symbol
    std::pair<QChar, const char*>{ QChar(0x015A), "Sacute" },                       // Character 'Ś' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E64), "Sacutedotaccent" },              // Character 'Ṥ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x03E0), "Sampigreek" },                   // Character 'Ϡ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0160), "Scaron" },                       // Character 'Š' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E66), "Scarondotaccent" },              // Character 'Ṧ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF6FD), "Scaronsmall" },                  //
    std::pair<QChar, const char*>{ QChar(0x015E), "Scedilla" },                     // Character 'Ş' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x018F), "Schwa" },                        // Character 'Ə' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x04D8), "Schwacyrillic" },                // Character 'Ә' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x04DA), "Schwadieresiscyrillic" },        // Character 'Ӛ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x24C8), "Scircle" },                      // Character 'Ⓢ' Symbol
    std::pair<QChar, const char*>{ QChar(0x015C), "Scircumflex" },                  // Character 'Ŝ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0218), "Scommaaccent" },                 // Character 'Ș' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E60), "Sdotaccent" },                   // Character 'Ṡ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E62), "Sdotbelow" },                    // Character 'Ṣ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E68), "Sdotbelowdotaccent" },           // Character 'Ṩ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x054D), "Seharmenian" },                  // Character 'Ս' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x2166), "Sevenroman" },                   // Character 'Ⅶ'
    std::pair<QChar, const char*>{ QChar(0x0547), "Shaarmenian" },                  // Character 'Շ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0428), "Shacyrillic" },                  // Character 'Ш' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0429), "Shchacyrillic" },                // Character 'Щ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x03E2), "Sheicoptic" },                   // Character 'Ϣ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x04BA), "Shhacyrillic" },                 // Character 'Һ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x03EC), "Shimacoptic" },                  // Character 'Ϭ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x03A3), "Sigma" },                        // Character 'Σ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x2165), "Sixroman" },                     // Character 'Ⅵ'
    std::pair<QChar, const char*>{ QChar(0xFF33), "Smonospace" },                   // Character 'Ｓ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x042C), "Softsigncyrillic" },             // Character 'Ь' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF773), "Ssmall" },                       //
    std::pair<QChar, const char*>{ QChar(0x03DA), "Stigmagreek" },                  // Character 'Ϛ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0054), "T" },                            // Character 'T' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x03A4), "Tau" },                          // Character 'Τ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0166), "Tbar" },                         // Character 'Ŧ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0164), "Tcaron" },                       // Character 'Ť' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0162), "Tcedilla" },                     // Character 'Ţ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x24C9), "Tcircle" },                      // Character 'Ⓣ' Symbol
    std::pair<QChar, const char*>{ QChar(0x1E70), "Tcircumflexbelow" },             // Character 'Ṱ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0162), "Tcommaaccent" },                 // Character 'Ţ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E6A), "Tdotaccent" },                   // Character 'Ṫ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E6C), "Tdotbelow" },                    // Character 'Ṭ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0422), "Tecyrillic" },                   // Character 'Т' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x04AC), "Tedescendercyrillic" },          // Character 'Ҭ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x2169), "Tenroman" },                     // Character 'Ⅹ'
    std::pair<QChar, const char*>{ QChar(0x04B4), "Tetsecyrillic" },                // Character 'Ҵ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0398), "Theta" },                        // Character 'Θ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x01AC), "Thook" },                        // Character 'Ƭ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x00DE), "Thorn" },                        // Character 'Þ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF7FE), "Thornsmall" },                   //
    std::pair<QChar, const char*>{ QChar(0x2162), "Threeroman" },                   // Character 'Ⅲ'
    std::pair<QChar, const char*>{ QChar(0xF6FE), "Tildesmall" },                   //
    std::pair<QChar, const char*>{ QChar(0x054F), "Tiwnarmenian" },                 // Character 'Տ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E6E), "Tlinebelow" },                   // Character 'Ṯ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xFF34), "Tmonospace" },                   // Character 'Ｔ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0539), "Toarmenian" },                   // Character 'Թ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x01BC), "Tonefive" },                     // Character 'Ƽ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0184), "Tonesix" },                      // Character 'Ƅ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x01A7), "Tonetwo" },                      // Character 'Ƨ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x01AE), "Tretroflexhook" },               // Character 'Ʈ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0426), "Tsecyrillic" },                  // Character 'Ц' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x040B), "Tshecyrillic" },                 // Character 'Ћ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF774), "Tsmall" },                       //
    std::pair<QChar, const char*>{ QChar(0x216B), "Twelveroman" },                  // Character 'Ⅻ'
    std::pair<QChar, const char*>{ QChar(0x2161), "Tworoman" },                     // Character 'Ⅱ'
    std::pair<QChar, const char*>{ QChar(0x0055), "U" },                            // Character 'U' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x00DA), "Uacute" },                       // Character 'Ú' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF7FA), "Uacutesmall" },                  //
    std::pair<QChar, const char*>{ QChar(0x016C), "Ubreve" },                       // Character 'Ŭ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x01D3), "Ucaron" },                       // Character 'Ǔ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x24CA), "Ucircle" },                      // Character 'Ⓤ' Symbol
    std::pair<QChar, const char*>{ QChar(0x00DB), "Ucircumflex" },                  // Character 'Û' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E76), "Ucircumflexbelow" },             // Character 'Ṷ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF7FB), "Ucircumflexsmall" },             //
    std::pair<QChar, const char*>{ QChar(0x0423), "Ucyrillic" },                    // Character 'У' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0170), "Udblacute" },                    // Character 'Ű' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0214), "Udblgrave" },                    // Character 'Ȕ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x00DC), "Udieresis" },                    // Character 'Ü' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x01D7), "Udieresisacute" },               // Character 'Ǘ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E72), "Udieresisbelow" },               // Character 'Ṳ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x01D9), "Udieresiscaron" },               // Character 'Ǚ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x04F0), "Udieresiscyrillic" },            // Character 'Ӱ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x01DB), "Udieresisgrave" },               // Character 'Ǜ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x01D5), "Udieresismacron" },              // Character 'Ǖ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF7FC), "Udieresissmall" },               //
    std::pair<QChar, const char*>{ QChar(0x1EE4), "Udotbelow" },                    // Character 'Ụ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x00D9), "Ugrave" },                       // Character 'Ù' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF7F9), "Ugravesmall" },                  //
    std::pair<QChar, const char*>{ QChar(0x1EE6), "Uhookabove" },                   // Character 'Ủ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x01AF), "Uhorn" },                        // Character 'Ư' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1EE8), "Uhornacute" },                   // Character 'Ứ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1EF0), "Uhorndotbelow" },                // Character 'Ự' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1EEA), "Uhorngrave" },                   // Character 'Ừ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1EEC), "Uhornhookabove" },               // Character 'Ử' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1EEE), "Uhorntilde" },                   // Character 'Ữ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0170), "Uhungarumlaut" },                // Character 'Ű' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x04F2), "Uhungarumlautcyrillic" },        // Character 'Ӳ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0216), "Uinvertedbreve" },               // Character 'Ȗ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0478), "Ukcyrillic" },                   // Character 'Ѹ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x016A), "Umacron" },                      // Character 'Ū' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x04EE), "Umacroncyrillic" },              // Character 'Ӯ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E7A), "Umacrondieresis" },              // Character 'Ṻ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xFF35), "Umonospace" },                   // Character 'Ｕ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0172), "Uogonek" },                      // Character 'Ų' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x03A5), "Upsilon" },                      // Character 'Υ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x03D2), "Upsilon1" },                     // Character 'ϒ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x03D3), "Upsilonacutehooksymbolgreek" },  // Character 'ϓ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x01B1), "Upsilonafrican" },               // Character 'Ʊ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x03AB), "Upsilondieresis" },              // Character 'Ϋ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x03D4), "Upsilondieresishooksymbolgreek" },// Character 'ϔ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x03D2), "Upsilonhooksymbol" },            // Character 'ϒ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x038E), "Upsilontonos" },                 // Character 'Ύ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x016E), "Uring" },                        // Character 'Ů' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x040E), "Ushortcyrillic" },               // Character 'Ў' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF775), "Usmall" },                       //
    std::pair<QChar, const char*>{ QChar(0x04AE), "Ustraightcyrillic" },            // Character 'Ү' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x04B0), "Ustraightstrokecyrillic" },      // Character 'Ұ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0168), "Utilde" },                       // Character 'Ũ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E78), "Utildeacute" },                  // Character 'Ṹ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E74), "Utildebelow" },                  // Character 'Ṵ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0056), "V" },                            // Character 'V' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x24CB), "Vcircle" },                      // Character 'Ⓥ' Symbol
    std::pair<QChar, const char*>{ QChar(0x1E7E), "Vdotbelow" },                    // Character 'Ṿ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0412), "Vecyrillic" },                   // Character 'В' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x054E), "Vewarmenian" },                  // Character 'Վ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x01B2), "Vhook" },                        // Character 'Ʋ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xFF36), "Vmonospace" },                   // Character 'Ｖ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0548), "Voarmenian" },                   // Character 'Ո' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF776), "Vsmall" },                       //
    std::pair<QChar, const char*>{ QChar(0x1E7C), "Vtilde" },                       // Character 'Ṽ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0057), "W" },                            // Character 'W' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E82), "Wacute" },                       // Character 'Ẃ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x24CC), "Wcircle" },                      // Character 'Ⓦ' Symbol
    std::pair<QChar, const char*>{ QChar(0x0174), "Wcircumflex" },                  // Character 'Ŵ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E84), "Wdieresis" },                    // Character 'Ẅ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E86), "Wdotaccent" },                   // Character 'Ẇ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E88), "Wdotbelow" },                    // Character 'Ẉ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E80), "Wgrave" },                       // Character 'Ẁ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xFF37), "Wmonospace" },                   // Character 'Ｗ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF777), "Wsmall" },                       //
    std::pair<QChar, const char*>{ QChar(0x0058), "X" },                            // Character 'X' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x24CD), "Xcircle" },                      // Character 'Ⓧ' Symbol
    std::pair<QChar, const char*>{ QChar(0x1E8C), "Xdieresis" },                    // Character 'Ẍ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E8A), "Xdotaccent" },                   // Character 'Ẋ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x053D), "Xeharmenian" },                  // Character 'Խ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x039E), "Xi" },                           // Character 'Ξ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xFF38), "Xmonospace" },                   // Character 'Ｘ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF778), "Xsmall" },                       //
    std::pair<QChar, const char*>{ QChar(0x0059), "Y" },                            // Character 'Y' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x00DD), "Yacute" },                       // Character 'Ý' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF7FD), "Yacutesmall" },                  //
    std::pair<QChar, const char*>{ QChar(0x0462), "Yatcyrillic" },                  // Character 'Ѣ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x24CE), "Ycircle" },                      // Character 'Ⓨ' Symbol
    std::pair<QChar, const char*>{ QChar(0x0176), "Ycircumflex" },                  // Character 'Ŷ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0178), "Ydieresis" },                    // Character 'Ÿ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF7FF), "Ydieresissmall" },               //
    std::pair<QChar, const char*>{ QChar(0x1E8E), "Ydotaccent" },                   // Character 'Ẏ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1EF4), "Ydotbelow" },                    // Character 'Ỵ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x042B), "Yericyrillic" },                 // Character 'Ы' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x04F8), "Yerudieresiscyrillic" },         // Character 'Ӹ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1EF2), "Ygrave" },                       // Character 'Ỳ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x01B3), "Yhook" },                        // Character 'Ƴ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1EF6), "Yhookabove" },                   // Character 'Ỷ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0545), "Yiarmenian" },                   // Character 'Յ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0407), "Yicyrillic" },                   // Character 'Ї' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0552), "Yiwnarmenian" },                 // Character 'Ւ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xFF39), "Ymonospace" },                   // Character 'Ｙ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF779), "Ysmall" },                       //
    std::pair<QChar, const char*>{ QChar(0x1EF8), "Ytilde" },                       // Character 'Ỹ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x046A), "Yusbigcyrillic" },               // Character 'Ѫ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x046C), "Yusbigiotifiedcyrillic" },       // Character 'Ѭ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0466), "Yuslittlecyrillic" },            // Character 'Ѧ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0468), "Yuslittleiotifiedcyrillic" },    // Character 'Ѩ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x005A), "Z" },                            // Character 'Z' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0536), "Zaarmenian" },                   // Character 'Զ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0179), "Zacute" },                       // Character 'Ź' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x017D), "Zcaron" },                       // Character 'Ž' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF6FF), "Zcaronsmall" },                  //
    std::pair<QChar, const char*>{ QChar(0x24CF), "Zcircle" },                      // Character 'Ⓩ' Symbol
    std::pair<QChar, const char*>{ QChar(0x1E90), "Zcircumflex" },                  // Character 'Ẑ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x017B), "Zdot" },                         // Character 'Ż' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x017B), "Zdotaccent" },                   // Character 'Ż' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E92), "Zdotbelow" },                    // Character 'Ẓ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0417), "Zecyrillic" },                   // Character 'З' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0498), "Zedescendercyrillic" },          // Character 'Ҙ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x04DE), "Zedieresiscyrillic" },           // Character 'Ӟ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0396), "Zeta" },                         // Character 'Ζ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x053A), "Zhearmenian" },                  // Character 'Ժ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x04C1), "Zhebrevecyrillic" },             // Character 'Ӂ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0416), "Zhecyrillic" },                  // Character 'Ж' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0496), "Zhedescendercyrillic" },         // Character 'Җ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x04DC), "Zhedieresiscyrillic" },          // Character 'Ӝ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E94), "Zlinebelow" },                   // Character 'Ẕ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xFF3A), "Zmonospace" },                   // Character 'Ｚ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF77A), "Zsmall" },                       //
    std::pair<QChar, const char*>{ QChar(0x01B5), "Zstroke" },                      // Character 'Ƶ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0061), "a" },                            // Character 'a' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0986), "aabengali" },                    // Character 'আ' Letter
    std::pair<QChar, const char*>{ QChar(0x00E1), "aacute" },                       // Character 'á' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0906), "aadeva" },                       // Character 'आ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A86), "aagujarati" },                   // Character 'આ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A06), "aagurmukhi" },                   // Character 'ਆ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A3E), "aamatragurmukhi" },              // Character 'ਾ' Mark
    std::pair<QChar, const char*>{ QChar(0x3303), "aarusquare" },                   // Character '㌃' Symbol
    std::pair<QChar, const char*>{ QChar(0x09BE), "aavowelsignbengali" },           // Character 'া' Mark
    std::pair<QChar, const char*>{ QChar(0x093E), "aavowelsigndeva" },              // Character 'ा' Mark
    std::pair<QChar, const char*>{ QChar(0x0ABE), "aavowelsigngujarati" },          // Character 'ા' Mark
    std::pair<QChar, const char*>{ QChar(0x055F), "abbreviationmarkarmenian" },     // Character '՟' Punctuation
    std::pair<QChar, const char*>{ QChar(0x0970), "abbreviationsigndeva" },         // Character '॰' Punctuation
    std::pair<QChar, const char*>{ QChar(0x0985), "abengali" },                     // Character 'অ' Letter
    std::pair<QChar, const char*>{ QChar(0x311A), "abopomofo" },                    // Character 'ㄚ' Letter
    std::pair<QChar, const char*>{ QChar(0x0103), "abreve" },                       // Character 'ă' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1EAF), "abreveacute" },                  // Character 'ắ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x04D1), "abrevecyrillic" },               // Character 'ӑ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1EB7), "abrevedotbelow" },               // Character 'ặ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1EB1), "abrevegrave" },                  // Character 'ằ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1EB3), "abrevehookabove" },              // Character 'ẳ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1EB5), "abrevetilde" },                  // Character 'ẵ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x01CE), "acaron" },                       // Character 'ǎ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x24D0), "acircle" },                      // Character 'ⓐ' Symbol
    std::pair<QChar, const char*>{ QChar(0x00E2), "acircumflex" },                  // Character 'â' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1EA5), "acircumflexacute" },             // Character 'ấ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1EAD), "acircumflexdotbelow" },          // Character 'ậ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1EA7), "acircumflexgrave" },             // Character 'ầ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1EA9), "acircumflexhookabove" },         // Character 'ẩ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1EAB), "acircumflextilde" },             // Character 'ẫ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x00B4), "acute" },                        // Character '´' Symbol
    std::pair<QChar, const char*>{ QChar(0x0317), "acutebelowcmb" },                // Character '̗' Mark
    std::pair<QChar, const char*>{ QChar(0x0301), "acutecmb" },                     // Character '́' Mark
    std::pair<QChar, const char*>{ QChar(0x0301), "acutecomb" },                    // Character '́' Mark
    std::pair<QChar, const char*>{ QChar(0x0954), "acutedeva" },                    // Character '॔' Mark
    std::pair<QChar, const char*>{ QChar(0x02CF), "acutelowmod" },                  // Character 'ˏ' Letter
    std::pair<QChar, const char*>{ QChar(0x0341), "acutetonecmb" },                 // Character '́' Mark
    std::pair<QChar, const char*>{ QChar(0x0430), "acyrillic" },                    // Character 'а' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0201), "adblgrave" },                    // Character 'ȁ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0A71), "addakgurmukhi" },                // Character 'ੱ' Mark
    std::pair<QChar, const char*>{ QChar(0x0905), "adeva" },                        // Character 'अ' Letter
    std::pair<QChar, const char*>{ QChar(0x00E4), "adieresis" },                    // Character 'ä' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x04D3), "adieresiscyrillic" },            // Character 'ӓ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x01DF), "adieresismacron" },              // Character 'ǟ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1EA1), "adotbelow" },                    // Character 'ạ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x01E1), "adotmacron" },                   // Character 'ǡ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x00E6), "ae" },                           // Character 'æ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x01FD), "aeacute" },                      // Character 'ǽ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3150), "aekorean" },                     // Character 'ㅐ' Letter
    std::pair<QChar, const char*>{ QChar(0x01E3), "aemacron" },                     // Character 'ǣ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x2015), "afii00208" },                    // Character '―' Punctuation
    std::pair<QChar, const char*>{ QChar(0x20A4), "afii08941" },                    // Character '₤' Symbol
    std::pair<QChar, const char*>{ QChar(0x0410), "afii10017" },                    // Character 'А' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0411), "afii10018" },                    // Character 'Б' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0412), "afii10019" },                    // Character 'В' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0413), "afii10020" },                    // Character 'Г' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0414), "afii10021" },                    // Character 'Д' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0415), "afii10022" },                    // Character 'Е' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0401), "afii10023" },                    // Character 'Ё' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0416), "afii10024" },                    // Character 'Ж' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0417), "afii10025" },                    // Character 'З' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0418), "afii10026" },                    // Character 'И' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0419), "afii10027" },                    // Character 'Й' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x041A), "afii10028" },                    // Character 'К' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x041B), "afii10029" },                    // Character 'Л' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x041C), "afii10030" },                    // Character 'М' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x041D), "afii10031" },                    // Character 'Н' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x041E), "afii10032" },                    // Character 'О' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x041F), "afii10033" },                    // Character 'П' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0420), "afii10034" },                    // Character 'Р' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0421), "afii10035" },                    // Character 'С' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0422), "afii10036" },                    // Character 'Т' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0423), "afii10037" },                    // Character 'У' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0424), "afii10038" },                    // Character 'Ф' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0425), "afii10039" },                    // Character 'Х' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0426), "afii10040" },                    // Character 'Ц' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0427), "afii10041" },                    // Character 'Ч' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0428), "afii10042" },                    // Character 'Ш' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0429), "afii10043" },                    // Character 'Щ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x042A), "afii10044" },                    // Character 'Ъ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x042B), "afii10045" },                    // Character 'Ы' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x042C), "afii10046" },                    // Character 'Ь' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x042D), "afii10047" },                    // Character 'Э' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x042E), "afii10048" },                    // Character 'Ю' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x042F), "afii10049" },                    // Character 'Я' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0490), "afii10050" },                    // Character 'Ґ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0402), "afii10051" },                    // Character 'Ђ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0403), "afii10052" },                    // Character 'Ѓ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0404), "afii10053" },                    // Character 'Є' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0405), "afii10054" },                    // Character 'Ѕ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0406), "afii10055" },                    // Character 'І' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0407), "afii10056" },                    // Character 'Ї' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0408), "afii10057" },                    // Character 'Ј' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0409), "afii10058" },                    // Character 'Љ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x040A), "afii10059" },                    // Character 'Њ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x040B), "afii10060" },                    // Character 'Ћ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x040C), "afii10061" },                    // Character 'Ќ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x040E), "afii10062" },                    // Character 'Ў' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF6C4), "afii10063" },                    //
    std::pair<QChar, const char*>{ QChar(0xF6C5), "afii10064" },                    //
    std::pair<QChar, const char*>{ QChar(0x0430), "afii10065" },                    // Character 'а' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0431), "afii10066" },                    // Character 'б' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0432), "afii10067" },                    // Character 'в' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0433), "afii10068" },                    // Character 'г' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0434), "afii10069" },                    // Character 'д' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0435), "afii10070" },                    // Character 'е' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0451), "afii10071" },                    // Character 'ё' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0436), "afii10072" },                    // Character 'ж' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0437), "afii10073" },                    // Character 'з' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0438), "afii10074" },                    // Character 'и' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0439), "afii10075" },                    // Character 'й' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x043A), "afii10076" },                    // Character 'к' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x043B), "afii10077" },                    // Character 'л' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x043C), "afii10078" },                    // Character 'м' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x043D), "afii10079" },                    // Character 'н' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x043E), "afii10080" },                    // Character 'о' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x043F), "afii10081" },                    // Character 'п' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0440), "afii10082" },                    // Character 'р' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0441), "afii10083" },                    // Character 'с' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0442), "afii10084" },                    // Character 'т' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0443), "afii10085" },                    // Character 'у' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0444), "afii10086" },                    // Character 'ф' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0445), "afii10087" },                    // Character 'х' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0446), "afii10088" },                    // Character 'ц' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0447), "afii10089" },                    // Character 'ч' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0448), "afii10090" },                    // Character 'ш' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0449), "afii10091" },                    // Character 'щ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x044A), "afii10092" },                    // Character 'ъ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x044B), "afii10093" },                    // Character 'ы' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x044C), "afii10094" },                    // Character 'ь' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x044D), "afii10095" },                    // Character 'э' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x044E), "afii10096" },                    // Character 'ю' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x044F), "afii10097" },                    // Character 'я' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0491), "afii10098" },                    // Character 'ґ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0452), "afii10099" },                    // Character 'ђ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0453), "afii10100" },                    // Character 'ѓ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0454), "afii10101" },                    // Character 'є' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0455), "afii10102" },                    // Character 'ѕ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0456), "afii10103" },                    // Character 'і' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0457), "afii10104" },                    // Character 'ї' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0458), "afii10105" },                    // Character 'ј' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0459), "afii10106" },                    // Character 'љ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x045A), "afii10107" },                    // Character 'њ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x045B), "afii10108" },                    // Character 'ћ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x045C), "afii10109" },                    // Character 'ќ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x045E), "afii10110" },                    // Character 'ў' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x040F), "afii10145" },                    // Character 'Џ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0462), "afii10146" },                    // Character 'Ѣ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0472), "afii10147" },                    // Character 'Ѳ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0474), "afii10148" },                    // Character 'Ѵ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0xF6C6), "afii10192" },                    //
    std::pair<QChar, const char*>{ QChar(0x045F), "afii10193" },                    // Character 'џ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0463), "afii10194" },                    // Character 'ѣ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0473), "afii10195" },                    // Character 'ѳ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0475), "afii10196" },                    // Character 'ѵ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0xF6C7), "afii10831" },                    //
    std::pair<QChar, const char*>{ QChar(0xF6C8), "afii10832" },                    //
    std::pair<QChar, const char*>{ QChar(0x04D9), "afii10846" },                    // Character 'ә' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x200E), "afii299" },                      //
    std::pair<QChar, const char*>{ QChar(0x200F), "afii300" },                      //
    std::pair<QChar, const char*>{ QChar(0x200D), "afii301" },                      //
    std::pair<QChar, const char*>{ QChar(0x066A), "afii57381" },                    // Character '٪' Punctuation
    std::pair<QChar, const char*>{ QChar(0x060C), "afii57388" },                    // Character '،' Punctuation
    std::pair<QChar, const char*>{ QChar(0x0660), "afii57392" },                    // Character '٠' Digit
    std::pair<QChar, const char*>{ QChar(0x0661), "afii57393" },                    // Character '١' Digit
    std::pair<QChar, const char*>{ QChar(0x0662), "afii57394" },                    // Character '٢' Digit
    std::pair<QChar, const char*>{ QChar(0x0663), "afii57395" },                    // Character '٣' Digit
    std::pair<QChar, const char*>{ QChar(0x0664), "afii57396" },                    // Character '٤' Digit
    std::pair<QChar, const char*>{ QChar(0x0665), "afii57397" },                    // Character '٥' Digit
    std::pair<QChar, const char*>{ QChar(0x0666), "afii57398" },                    // Character '٦' Digit
    std::pair<QChar, const char*>{ QChar(0x0667), "afii57399" },                    // Character '٧' Digit
    std::pair<QChar, const char*>{ QChar(0x0668), "afii57400" },                    // Character '٨' Digit
    std::pair<QChar, const char*>{ QChar(0x0669), "afii57401" },                    // Character '٩' Digit
    std::pair<QChar, const char*>{ QChar(0x061B), "afii57403" },                    // Character '؛' Punctuation
    std::pair<QChar, const char*>{ QChar(0x061F), "afii57407" },                    // Character '؟' Punctuation
    std::pair<QChar, const char*>{ QChar(0x0621), "afii57409" },                    // Character 'ء' Letter
    std::pair<QChar, const char*>{ QChar(0x0622), "afii57410" },                    // Character 'آ' Letter
    std::pair<QChar, const char*>{ QChar(0x0623), "afii57411" },                    // Character 'أ' Letter
    std::pair<QChar, const char*>{ QChar(0x0624), "afii57412" },                    // Character 'ؤ' Letter
    std::pair<QChar, const char*>{ QChar(0x0625), "afii57413" },                    // Character 'إ' Letter
    std::pair<QChar, const char*>{ QChar(0x0626), "afii57414" },                    // Character 'ئ' Letter
    std::pair<QChar, const char*>{ QChar(0x0627), "afii57415" },                    // Character 'ا' Letter
    std::pair<QChar, const char*>{ QChar(0x0628), "afii57416" },                    // Character 'ب' Letter
    std::pair<QChar, const char*>{ QChar(0x0629), "afii57417" },                    // Character 'ة' Letter
    std::pair<QChar, const char*>{ QChar(0x062A), "afii57418" },                    // Character 'ت' Letter
    std::pair<QChar, const char*>{ QChar(0x062B), "afii57419" },                    // Character 'ث' Letter
    std::pair<QChar, const char*>{ QChar(0x062C), "afii57420" },                    // Character 'ج' Letter
    std::pair<QChar, const char*>{ QChar(0x062D), "afii57421" },                    // Character 'ح' Letter
    std::pair<QChar, const char*>{ QChar(0x062E), "afii57422" },                    // Character 'خ' Letter
    std::pair<QChar, const char*>{ QChar(0x062F), "afii57423" },                    // Character 'د' Letter
    std::pair<QChar, const char*>{ QChar(0x0630), "afii57424" },                    // Character 'ذ' Letter
    std::pair<QChar, const char*>{ QChar(0x0631), "afii57425" },                    // Character 'ر' Letter
    std::pair<QChar, const char*>{ QChar(0x0632), "afii57426" },                    // Character 'ز' Letter
    std::pair<QChar, const char*>{ QChar(0x0633), "afii57427" },                    // Character 'س' Letter
    std::pair<QChar, const char*>{ QChar(0x0634), "afii57428" },                    // Character 'ش' Letter
    std::pair<QChar, const char*>{ QChar(0x0635), "afii57429" },                    // Character 'ص' Letter
    std::pair<QChar, const char*>{ QChar(0x0636), "afii57430" },                    // Character 'ض' Letter
    std::pair<QChar, const char*>{ QChar(0x0637), "afii57431" },                    // Character 'ط' Letter
    std::pair<QChar, const char*>{ QChar(0x0638), "afii57432" },                    // Character 'ظ' Letter
    std::pair<QChar, const char*>{ QChar(0x0639), "afii57433" },                    // Character 'ع' Letter
    std::pair<QChar, const char*>{ QChar(0x063A), "afii57434" },                    // Character 'غ' Letter
    std::pair<QChar, const char*>{ QChar(0x0640), "afii57440" },                    // Character 'ـ' Letter
    std::pair<QChar, const char*>{ QChar(0x0641), "afii57441" },                    // Character 'ف' Letter
    std::pair<QChar, const char*>{ QChar(0x0642), "afii57442" },                    // Character 'ق' Letter
    std::pair<QChar, const char*>{ QChar(0x0643), "afii57443" },                    // Character 'ك' Letter
    std::pair<QChar, const char*>{ QChar(0x0644), "afii57444" },                    // Character 'ل' Letter
    std::pair<QChar, const char*>{ QChar(0x0645), "afii57445" },                    // Character 'م' Letter
    std::pair<QChar, const char*>{ QChar(0x0646), "afii57446" },                    // Character 'ن' Letter
    std::pair<QChar, const char*>{ QChar(0x0648), "afii57448" },                    // Character 'و' Letter
    std::pair<QChar, const char*>{ QChar(0x0649), "afii57449" },                    // Character 'ى' Letter
    std::pair<QChar, const char*>{ QChar(0x064A), "afii57450" },                    // Character 'ي' Letter
    std::pair<QChar, const char*>{ QChar(0x064B), "afii57451" },                    // Character 'ً' Mark
    std::pair<QChar, const char*>{ QChar(0x064C), "afii57452" },                    // Character 'ٌ' Mark
    std::pair<QChar, const char*>{ QChar(0x064D), "afii57453" },                    // Character 'ٍ' Mark
    std::pair<QChar, const char*>{ QChar(0x064E), "afii57454" },                    // Character 'َ' Mark
    std::pair<QChar, const char*>{ QChar(0x064F), "afii57455" },                    // Character 'ُ' Mark
    std::pair<QChar, const char*>{ QChar(0x0650), "afii57456" },                    // Character 'ِ' Mark
    std::pair<QChar, const char*>{ QChar(0x0651), "afii57457" },                    // Character 'ّ' Mark
    std::pair<QChar, const char*>{ QChar(0x0652), "afii57458" },                    // Character 'ْ' Mark
    std::pair<QChar, const char*>{ QChar(0x0647), "afii57470" },                    // Character 'ه' Letter
    std::pair<QChar, const char*>{ QChar(0x06A4), "afii57505" },                    // Character 'ڤ' Letter
    std::pair<QChar, const char*>{ QChar(0x067E), "afii57506" },                    // Character 'پ' Letter
    std::pair<QChar, const char*>{ QChar(0x0686), "afii57507" },                    // Character 'چ' Letter
    std::pair<QChar, const char*>{ QChar(0x0698), "afii57508" },                    // Character 'ژ' Letter
    std::pair<QChar, const char*>{ QChar(0x06AF), "afii57509" },                    // Character 'گ' Letter
    std::pair<QChar, const char*>{ QChar(0x0679), "afii57511" },                    // Character 'ٹ' Letter
    std::pair<QChar, const char*>{ QChar(0x0688), "afii57512" },                    // Character 'ڈ' Letter
    std::pair<QChar, const char*>{ QChar(0x0691), "afii57513" },                    // Character 'ڑ' Letter
    std::pair<QChar, const char*>{ QChar(0x06BA), "afii57514" },                    // Character 'ں' Letter
    std::pair<QChar, const char*>{ QChar(0x06D2), "afii57519" },                    // Character 'ے' Letter
    std::pair<QChar, const char*>{ QChar(0x06D5), "afii57534" },                    // Character 'ە' Letter
    std::pair<QChar, const char*>{ QChar(0x20AA), "afii57636" },                    // Character '₪' Symbol
    std::pair<QChar, const char*>{ QChar(0x05BE), "afii57645" },                    // Character '־' Punctuation
    std::pair<QChar, const char*>{ QChar(0x05C3), "afii57658" },                    // Character '׃' Punctuation
    std::pair<QChar, const char*>{ QChar(0x05D0), "afii57664" },                    // Character 'א' Letter
    std::pair<QChar, const char*>{ QChar(0x05D1), "afii57665" },                    // Character 'ב' Letter
    std::pair<QChar, const char*>{ QChar(0x05D2), "afii57666" },                    // Character 'ג' Letter
    std::pair<QChar, const char*>{ QChar(0x05D3), "afii57667" },                    // Character 'ד' Letter
    std::pair<QChar, const char*>{ QChar(0x05D4), "afii57668" },                    // Character 'ה' Letter
    std::pair<QChar, const char*>{ QChar(0x05D5), "afii57669" },                    // Character 'ו' Letter
    std::pair<QChar, const char*>{ QChar(0x05D6), "afii57670" },                    // Character 'ז' Letter
    std::pair<QChar, const char*>{ QChar(0x05D7), "afii57671" },                    // Character 'ח' Letter
    std::pair<QChar, const char*>{ QChar(0x05D8), "afii57672" },                    // Character 'ט' Letter
    std::pair<QChar, const char*>{ QChar(0x05D9), "afii57673" },                    // Character 'י' Letter
    std::pair<QChar, const char*>{ QChar(0x05DA), "afii57674" },                    // Character 'ך' Letter
    std::pair<QChar, const char*>{ QChar(0x05DB), "afii57675" },                    // Character 'כ' Letter
    std::pair<QChar, const char*>{ QChar(0x05DC), "afii57676" },                    // Character 'ל' Letter
    std::pair<QChar, const char*>{ QChar(0x05DD), "afii57677" },                    // Character 'ם' Letter
    std::pair<QChar, const char*>{ QChar(0x05DE), "afii57678" },                    // Character 'מ' Letter
    std::pair<QChar, const char*>{ QChar(0x05DF), "afii57679" },                    // Character 'ן' Letter
    std::pair<QChar, const char*>{ QChar(0x05E0), "afii57680" },                    // Character 'נ' Letter
    std::pair<QChar, const char*>{ QChar(0x05E1), "afii57681" },                    // Character 'ס' Letter
    std::pair<QChar, const char*>{ QChar(0x05E2), "afii57682" },                    // Character 'ע' Letter
    std::pair<QChar, const char*>{ QChar(0x05E3), "afii57683" },                    // Character 'ף' Letter
    std::pair<QChar, const char*>{ QChar(0x05E4), "afii57684" },                    // Character 'פ' Letter
    std::pair<QChar, const char*>{ QChar(0x05E5), "afii57685" },                    // Character 'ץ' Letter
    std::pair<QChar, const char*>{ QChar(0x05E6), "afii57686" },                    // Character 'צ' Letter
    std::pair<QChar, const char*>{ QChar(0x05E7), "afii57687" },                    // Character 'ק' Letter
    std::pair<QChar, const char*>{ QChar(0x05E8), "afii57688" },                    // Character 'ר' Letter
    std::pair<QChar, const char*>{ QChar(0x05E9), "afii57689" },                    // Character 'ש' Letter
    std::pair<QChar, const char*>{ QChar(0x05EA), "afii57690" },                    // Character 'ת' Letter
    std::pair<QChar, const char*>{ QChar(0xFB2A), "afii57694" },                    // Character 'שׁ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB2B), "afii57695" },                    // Character 'שׂ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB4B), "afii57700" },                    // Character 'וֹ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB1F), "afii57705" },                    // Character 'ײַ' Letter
    std::pair<QChar, const char*>{ QChar(0x05F0), "afii57716" },                    // Character 'װ' Letter
    std::pair<QChar, const char*>{ QChar(0x05F1), "afii57717" },                    // Character 'ױ' Letter
    std::pair<QChar, const char*>{ QChar(0x05F2), "afii57718" },                    // Character 'ײ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB35), "afii57723" },                    // Character 'וּ' Letter
    std::pair<QChar, const char*>{ QChar(0x05B4), "afii57793" },                    // Character 'ִ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B5), "afii57794" },                    // Character 'ֵ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B6), "afii57795" },                    // Character 'ֶ' Mark
    std::pair<QChar, const char*>{ QChar(0x05BB), "afii57796" },                    // Character 'ֻ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B8), "afii57797" },                    // Character 'ָ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B7), "afii57798" },                    // Character 'ַ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B0), "afii57799" },                    // Character 'ְ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B2), "afii57800" },                    // Character 'ֲ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B1), "afii57801" },                    // Character 'ֱ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B3), "afii57802" },                    // Character 'ֳ' Mark
    std::pair<QChar, const char*>{ QChar(0x05C2), "afii57803" },                    // Character 'ׂ' Mark
    std::pair<QChar, const char*>{ QChar(0x05C1), "afii57804" },                    // Character 'ׁ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B9), "afii57806" },                    // Character 'ֹ' Mark
    std::pair<QChar, const char*>{ QChar(0x05BC), "afii57807" },                    // Character 'ּ' Mark
    std::pair<QChar, const char*>{ QChar(0x05BD), "afii57839" },                    // Character 'ֽ' Mark
    std::pair<QChar, const char*>{ QChar(0x05BF), "afii57841" },                    // Character 'ֿ' Mark
    std::pair<QChar, const char*>{ QChar(0x05C0), "afii57842" },                    // Character '׀' Punctuation
    std::pair<QChar, const char*>{ QChar(0x02BC), "afii57929" },                    // Character 'ʼ' Letter
    std::pair<QChar, const char*>{ QChar(0x2105), "afii61248" },                    // Character '℅' Symbol
    std::pair<QChar, const char*>{ QChar(0x2113), "afii61289" },                    // Character 'ℓ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x2116), "afii61352" },                    // Character '№' Symbol
    std::pair<QChar, const char*>{ QChar(0x202C), "afii61573" },                    //
    std::pair<QChar, const char*>{ QChar(0x202D), "afii61574" },                    //
    std::pair<QChar, const char*>{ QChar(0x202E), "afii61575" },                    //
    std::pair<QChar, const char*>{ QChar(0x200C), "afii61664" },                    //
    std::pair<QChar, const char*>{ QChar(0x066D), "afii63167" },                    // Character '٭' Punctuation
    std::pair<QChar, const char*>{ QChar(0x02BD), "afii64937" },                    // Character 'ʽ' Letter
    std::pair<QChar, const char*>{ QChar(0x00E0), "agrave" },                       // Character 'à' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0A85), "agujarati" },                    // Character 'અ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A05), "agurmukhi" },                    // Character 'ਅ' Letter
    std::pair<QChar, const char*>{ QChar(0x3042), "ahiragana" },                    // Character 'あ' Letter
    std::pair<QChar, const char*>{ QChar(0x1EA3), "ahookabove" },                   // Character 'ả' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0990), "aibengali" },                    // Character 'ঐ' Letter
    std::pair<QChar, const char*>{ QChar(0x311E), "aibopomofo" },                   // Character 'ㄞ' Letter
    std::pair<QChar, const char*>{ QChar(0x0910), "aideva" },                       // Character 'ऐ' Letter
    std::pair<QChar, const char*>{ QChar(0x04D5), "aiecyrillic" },                  // Character 'ӕ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0A90), "aigujarati" },                   // Character 'ઐ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A10), "aigurmukhi" },                   // Character 'ਐ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A48), "aimatragurmukhi" },              // Character 'ੈ' Mark
    std::pair<QChar, const char*>{ QChar(0x0639), "ainarabic" },                    // Character 'ع' Letter
    std::pair<QChar, const char*>{ QChar(0xFECA), "ainfinalarabic" },               // Character 'ﻊ' Letter
    std::pair<QChar, const char*>{ QChar(0xFECB), "aininitialarabic" },             // Character 'ﻋ' Letter
    std::pair<QChar, const char*>{ QChar(0xFECC), "ainmedialarabic" },              // Character 'ﻌ' Letter
    std::pair<QChar, const char*>{ QChar(0x0203), "ainvertedbreve" },               // Character 'ȃ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x09C8), "aivowelsignbengali" },           // Character 'ৈ' Mark
    std::pair<QChar, const char*>{ QChar(0x0948), "aivowelsigndeva" },              // Character 'ै' Mark
    std::pair<QChar, const char*>{ QChar(0x0AC8), "aivowelsigngujarati" },          // Character 'ૈ' Mark
    std::pair<QChar, const char*>{ QChar(0x30A2), "akatakana" },                    // Character 'ア' Letter
    std::pair<QChar, const char*>{ QChar(0xFF71), "akatakanahalfwidth" },           // Character 'ｱ' Letter
    std::pair<QChar, const char*>{ QChar(0x314F), "akorean" },                      // Character 'ㅏ' Letter
    std::pair<QChar, const char*>{ QChar(0x05D0), "alef" },                         // Character 'א' Letter
    std::pair<QChar, const char*>{ QChar(0x0627), "alefarabic" },                   // Character 'ا' Letter
    std::pair<QChar, const char*>{ QChar(0xFB30), "alefdageshhebrew" },             // Character 'אּ' Letter
    std::pair<QChar, const char*>{ QChar(0xFE8E), "aleffinalarabic" },              // Character 'ﺎ' Letter
    std::pair<QChar, const char*>{ QChar(0x0623), "alefhamzaabovearabic" },         // Character 'أ' Letter
    std::pair<QChar, const char*>{ QChar(0xFE84), "alefhamzaabovefinalarabic" },    // Character 'ﺄ' Letter
    std::pair<QChar, const char*>{ QChar(0x0625), "alefhamzabelowarabic" },         // Character 'إ' Letter
    std::pair<QChar, const char*>{ QChar(0xFE88), "alefhamzabelowfinalarabic" },    // Character 'ﺈ' Letter
    std::pair<QChar, const char*>{ QChar(0x05D0), "alefhebrew" },                   // Character 'א' Letter
    std::pair<QChar, const char*>{ QChar(0xFB4F), "aleflamedhebrew" },              // Character 'ﭏ' Letter
    std::pair<QChar, const char*>{ QChar(0x0622), "alefmaddaabovearabic" },         // Character 'آ' Letter
    std::pair<QChar, const char*>{ QChar(0xFE82), "alefmaddaabovefinalarabic" },    // Character 'ﺂ' Letter
    std::pair<QChar, const char*>{ QChar(0x0649), "alefmaksuraarabic" },            // Character 'ى' Letter
    std::pair<QChar, const char*>{ QChar(0xFEF0), "alefmaksurafinalarabic" },       // Character 'ﻰ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEF3), "alefmaksurainitialarabic" },     // Character 'ﻳ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEF4), "alefmaksuramedialarabic" },      // Character 'ﻴ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB2E), "alefpatahhebrew" },              // Character 'אַ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB2F), "alefqamatshebrew" },             // Character 'אָ' Letter
    std::pair<QChar, const char*>{ QChar(0x2135), "aleph" },                        // Character 'ℵ' Letter
    std::pair<QChar, const char*>{ QChar(0x224C), "allequal" },                     // Character '≌' Symbol
    std::pair<QChar, const char*>{ QChar(0x03B1), "alpha" },                        // Character 'α' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x03AC), "alphatonos" },                   // Character 'ά' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0101), "amacron" },                      // Character 'ā' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0xFF41), "amonospace" },                   // Character 'ａ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0026), "ampersand" },                    // Character '&' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFF06), "ampersandmonospace" },           // Character '＆' Punctuation
    std::pair<QChar, const char*>{ QChar(0xF726), "ampersandsmall" },               //
    std::pair<QChar, const char*>{ QChar(0x33C2), "amsquare" },                     // Character '㏂' Symbol
    std::pair<QChar, const char*>{ QChar(0x3122), "anbopomofo" },                   // Character 'ㄢ' Letter
    std::pair<QChar, const char*>{ QChar(0x3124), "angbopomofo" },                  // Character 'ㄤ' Letter
    std::pair<QChar, const char*>{ QChar(0x0E5A), "angkhankhuthai" },               // Character '๚' Punctuation
    std::pair<QChar, const char*>{ QChar(0x2220), "angle" },                        // Character '∠' Symbol
    std::pair<QChar, const char*>{ QChar(0x3008), "anglebracketleft" },             // Character '〈' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFE3F), "anglebracketleftvertical" },     // Character '︿' Punctuation
    std::pair<QChar, const char*>{ QChar(0x3009), "anglebracketright" },            // Character '〉' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFE40), "anglebracketrightvertical" },    // Character '﹀' Punctuation
    std::pair<QChar, const char*>{ QChar(0x2329), "angleleft" },                    // Character '〈' Punctuation
    std::pair<QChar, const char*>{ QChar(0x232A), "angleright" },                   // Character '〉' Punctuation
    std::pair<QChar, const char*>{ QChar(0x212B), "angstrom" },                     // Character 'Å' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x0387), "anoteleia" },                    // Character '·' Punctuation
    std::pair<QChar, const char*>{ QChar(0x0952), "anudattadeva" },                 // Character '॒' Mark
    std::pair<QChar, const char*>{ QChar(0x0982), "anusvarabengali" },              // Character 'ং' Mark
    std::pair<QChar, const char*>{ QChar(0x0902), "anusvaradeva" },                 // Character 'ं' Mark
    std::pair<QChar, const char*>{ QChar(0x0A82), "anusvaragujarati" },             // Character 'ં' Mark
    std::pair<QChar, const char*>{ QChar(0x0105), "aogonek" },                      // Character 'ą' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3300), "apaatosquare" },                 // Character '㌀' Symbol
    std::pair<QChar, const char*>{ QChar(0x249C), "aparen" },                       // Character '⒜' Symbol
    std::pair<QChar, const char*>{ QChar(0x055A), "apostrophearmenian" },           // Character '՚' Punctuation
    std::pair<QChar, const char*>{ QChar(0x02BC), "apostrophemod" },                // Character 'ʼ' Letter
    std::pair<QChar, const char*>{ QChar(0xF8FF), "apple" },                        //
    std::pair<QChar, const char*>{ QChar(0x2250), "approaches" },                   // Character '≐' Symbol
    std::pair<QChar, const char*>{ QChar(0x2248), "approxequal" },                  // Character '≈' Symbol
    std::pair<QChar, const char*>{ QChar(0x2252), "approxequalorimage" },           // Character '≒' Symbol
    std::pair<QChar, const char*>{ QChar(0x2245), "approximatelyequal" },           // Character '≅' Symbol
    std::pair<QChar, const char*>{ QChar(0x318E), "araeaekorean" },                 // Character 'ㆎ' Letter
    std::pair<QChar, const char*>{ QChar(0x318D), "araeakorean" },                  // Character 'ㆍ' Letter
    std::pair<QChar, const char*>{ QChar(0x2312), "arc" },                          // Character '⌒' Symbol
    std::pair<QChar, const char*>{ QChar(0x1E9A), "arighthalfring" },               // Character 'ẚ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x00E5), "aring" },                        // Character 'å' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x01FB), "aringacute" },                   // Character 'ǻ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E01), "aringbelow" },                   // Character 'ḁ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x2194), "arrowboth" },                    // Character '↔' Symbol
    std::pair<QChar, const char*>{ QChar(0x21E3), "arrowdashdown" },                // Character '⇣' Symbol
    std::pair<QChar, const char*>{ QChar(0x21E0), "arrowdashleft" },                // Character '⇠' Symbol
    std::pair<QChar, const char*>{ QChar(0x21E2), "arrowdashright" },               // Character '⇢' Symbol
    std::pair<QChar, const char*>{ QChar(0x21E1), "arrowdashup" },                  // Character '⇡' Symbol
    std::pair<QChar, const char*>{ QChar(0x21D4), "arrowdblboth" },                 // Character '⇔' Symbol
    std::pair<QChar, const char*>{ QChar(0x21D3), "arrowdbldown" },                 // Character '⇓' Symbol
    std::pair<QChar, const char*>{ QChar(0x21D0), "arrowdblleft" },                 // Character '⇐' Symbol
    std::pair<QChar, const char*>{ QChar(0x21D2), "arrowdblright" },                // Character '⇒' Symbol
    std::pair<QChar, const char*>{ QChar(0x21D1), "arrowdblup" },                   // Character '⇑' Symbol
    std::pair<QChar, const char*>{ QChar(0x2193), "arrowdown" },                    // Character '↓' Symbol
    std::pair<QChar, const char*>{ QChar(0x2199), "arrowdownleft" },                // Character '↙' Symbol
    std::pair<QChar, const char*>{ QChar(0x2198), "arrowdownright" },               // Character '↘' Symbol
    std::pair<QChar, const char*>{ QChar(0x21E9), "arrowdownwhite" },               // Character '⇩' Symbol
    std::pair<QChar, const char*>{ QChar(0x02C5), "arrowheaddownmod" },             // Character '˅' Symbol
    std::pair<QChar, const char*>{ QChar(0x02C2), "arrowheadleftmod" },             // Character '˂' Symbol
    std::pair<QChar, const char*>{ QChar(0x02C3), "arrowheadrightmod" },            // Character '˃' Symbol
    std::pair<QChar, const char*>{ QChar(0x02C4), "arrowheadupmod" },               // Character '˄' Symbol
    std::pair<QChar, const char*>{ QChar(0xF8E7), "arrowhorizex" },                 //
    std::pair<QChar, const char*>{ QChar(0x2190), "arrowleft" },                    // Character '←' Symbol
    std::pair<QChar, const char*>{ QChar(0x21D0), "arrowleftdbl" },                 // Character '⇐' Symbol
    std::pair<QChar, const char*>{ QChar(0x21CD), "arrowleftdblstroke" },           // Character '⇍' Symbol
    std::pair<QChar, const char*>{ QChar(0x21C6), "arrowleftoverright" },           // Character '⇆' Symbol
    std::pair<QChar, const char*>{ QChar(0x21E6), "arrowleftwhite" },               // Character '⇦' Symbol
    std::pair<QChar, const char*>{ QChar(0x2192), "arrowright" },                   // Character '→' Symbol
    std::pair<QChar, const char*>{ QChar(0x21CF), "arrowrightdblstroke" },          // Character '⇏' Symbol
    std::pair<QChar, const char*>{ QChar(0x279E), "arrowrightheavy" },              // Character '➞' Symbol
    std::pair<QChar, const char*>{ QChar(0x21C4), "arrowrightoverleft" },           // Character '⇄' Symbol
    std::pair<QChar, const char*>{ QChar(0x21E8), "arrowrightwhite" },              // Character '⇨' Symbol
    std::pair<QChar, const char*>{ QChar(0x21E4), "arrowtableft" },                 // Character '⇤' Symbol
    std::pair<QChar, const char*>{ QChar(0x21E5), "arrowtabright" },                // Character '⇥' Symbol
    std::pair<QChar, const char*>{ QChar(0x2191), "arrowup" },                      // Character '↑' Symbol
    std::pair<QChar, const char*>{ QChar(0x2195), "arrowupdn" },                    // Character '↕' Symbol
    std::pair<QChar, const char*>{ QChar(0x21A8), "arrowupdnbse" },                 // Character '↨' Symbol
    std::pair<QChar, const char*>{ QChar(0x21A8), "arrowupdownbase" },              // Character '↨' Symbol
    std::pair<QChar, const char*>{ QChar(0x2196), "arrowupleft" },                  // Character '↖' Symbol
    std::pair<QChar, const char*>{ QChar(0x21C5), "arrowupleftofdown" },            // Character '⇅' Symbol
    std::pair<QChar, const char*>{ QChar(0x2197), "arrowupright" },                 // Character '↗' Symbol
    std::pair<QChar, const char*>{ QChar(0x21E7), "arrowupwhite" },                 // Character '⇧' Symbol
    std::pair<QChar, const char*>{ QChar(0xF8E6), "arrowvertex" },                  //
    std::pair<QChar, const char*>{ QChar(0x005E), "asciicircum" },                  // Character '^' Symbol
    std::pair<QChar, const char*>{ QChar(0xFF3E), "asciicircummonospace" },         // Character '＾' Symbol
    std::pair<QChar, const char*>{ QChar(0x007E), "asciitilde" },                   // Character '~' Symbol
    std::pair<QChar, const char*>{ QChar(0xFF5E), "asciitildemonospace" },          // Character '～' Symbol
    std::pair<QChar, const char*>{ QChar(0x0251), "ascript" },                      // Character 'ɑ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0252), "ascriptturned" },                // Character 'ɒ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3041), "asmallhiragana" },               // Character 'ぁ' Letter
    std::pair<QChar, const char*>{ QChar(0x30A1), "asmallkatakana" },               // Character 'ァ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF67), "asmallkatakanahalfwidth" },      // Character 'ｧ' Letter
    std::pair<QChar, const char*>{ QChar(0x002A), "asterisk" },                     // Character '*' Punctuation
    std::pair<QChar, const char*>{ QChar(0x066D), "asteriskaltonearabic" },         // Character '٭' Punctuation
    std::pair<QChar, const char*>{ QChar(0x066D), "asteriskarabic" },               // Character '٭' Punctuation
    std::pair<QChar, const char*>{ QChar(0x2217), "asteriskmath" },                 // Character '∗' Symbol
    std::pair<QChar, const char*>{ QChar(0xFF0A), "asteriskmonospace" },            // Character '＊' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFE61), "asterisksmall" },                // Character '﹡' Punctuation
    std::pair<QChar, const char*>{ QChar(0x2042), "asterism" },                     // Character '⁂' Punctuation
    std::pair<QChar, const char*>{ QChar(0xF6E9), "asuperior" },                    //
    std::pair<QChar, const char*>{ QChar(0x2243), "asymptoticallyequal" },          // Character '≃' Symbol
    std::pair<QChar, const char*>{ QChar(0x0040), "at" },                           // Character '@' Punctuation
    std::pair<QChar, const char*>{ QChar(0x00E3), "atilde" },                       // Character 'ã' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0xFF20), "atmonospace" },                  // Character '＠' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFE6B), "atsmall" },                      // Character '﹫' Punctuation
    std::pair<QChar, const char*>{ QChar(0x0250), "aturned" },                      // Character 'ɐ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0994), "aubengali" },                    // Character 'ঔ' Letter
    std::pair<QChar, const char*>{ QChar(0x3120), "aubopomofo" },                   // Character 'ㄠ' Letter
    std::pair<QChar, const char*>{ QChar(0x0914), "audeva" },                       // Character 'औ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A94), "augujarati" },                   // Character 'ઔ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A14), "augurmukhi" },                   // Character 'ਔ' Letter
    std::pair<QChar, const char*>{ QChar(0x09D7), "aulengthmarkbengali" },          // Character 'ৗ' Mark
    std::pair<QChar, const char*>{ QChar(0x0A4C), "aumatragurmukhi" },              // Character 'ੌ' Mark
    std::pair<QChar, const char*>{ QChar(0x09CC), "auvowelsignbengali" },           // Character 'ৌ' Mark
    std::pair<QChar, const char*>{ QChar(0x094C), "auvowelsigndeva" },              // Character 'ौ' Mark
    std::pair<QChar, const char*>{ QChar(0x0ACC), "auvowelsigngujarati" },          // Character 'ૌ' Mark
    std::pair<QChar, const char*>{ QChar(0x093D), "avagrahadeva" },                 // Character 'ऽ' Letter
    std::pair<QChar, const char*>{ QChar(0x0561), "aybarmenian" },                  // Character 'ա' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x05E2), "ayin" },                         // Character 'ע' Letter
    std::pair<QChar, const char*>{ QChar(0xFB20), "ayinaltonehebrew" },             // Character 'ﬠ' Letter
    std::pair<QChar, const char*>{ QChar(0x05E2), "ayinhebrew" },                   // Character 'ע' Letter
    std::pair<QChar, const char*>{ QChar(0x0062), "b" },                            // Character 'b' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x09AC), "babengali" },                    // Character 'ব' Letter
    std::pair<QChar, const char*>{ QChar(0x005C), "backslash" },                    // Character '\' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFF3C), "backslashmonospace" },           // Character '＼' Punctuation
    std::pair<QChar, const char*>{ QChar(0x092C), "badeva" },                       // Character 'ब' Letter
    std::pair<QChar, const char*>{ QChar(0x0AAC), "bagujarati" },                   // Character 'બ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A2C), "bagurmukhi" },                   // Character 'ਬ' Letter
    std::pair<QChar, const char*>{ QChar(0x3070), "bahiragana" },                   // Character 'ば' Letter
    std::pair<QChar, const char*>{ QChar(0x0E3F), "bahtthai" },                     // Character '฿' Symbol
    std::pair<QChar, const char*>{ QChar(0x30D0), "bakatakana" },                   // Character 'バ' Letter
    std::pair<QChar, const char*>{ QChar(0x007C), "bar" },                          // Character '|' Symbol
    std::pair<QChar, const char*>{ QChar(0xFF5C), "barmonospace" },                 // Character '｜' Symbol
    std::pair<QChar, const char*>{ QChar(0x3105), "bbopomofo" },                    // Character 'ㄅ' Letter
    std::pair<QChar, const char*>{ QChar(0x24D1), "bcircle" },                      // Character 'ⓑ' Symbol
    std::pair<QChar, const char*>{ QChar(0x1E03), "bdotaccent" },                   // Character 'ḃ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E05), "bdotbelow" },                    // Character 'ḅ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x266C), "beamedsixteenthnotes" },         // Character '♬' Symbol
    std::pair<QChar, const char*>{ QChar(0x2235), "because" },                      // Character '∵' Symbol
    std::pair<QChar, const char*>{ QChar(0x0431), "becyrillic" },                   // Character 'б' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0628), "beharabic" },                    // Character 'ب' Letter
    std::pair<QChar, const char*>{ QChar(0xFE90), "behfinalarabic" },               // Character 'ﺐ' Letter
    std::pair<QChar, const char*>{ QChar(0xFE91), "behinitialarabic" },             // Character 'ﺑ' Letter
    std::pair<QChar, const char*>{ QChar(0x3079), "behiragana" },                   // Character 'べ' Letter
    std::pair<QChar, const char*>{ QChar(0xFE92), "behmedialarabic" },              // Character 'ﺒ' Letter
    std::pair<QChar, const char*>{ QChar(0xFC9F), "behmeeminitialarabic" },         // Character 'ﲟ' Letter
    std::pair<QChar, const char*>{ QChar(0xFC08), "behmeemisolatedarabic" },        // Character 'ﰈ' Letter
    std::pair<QChar, const char*>{ QChar(0xFC6D), "behnoonfinalarabic" },           // Character 'ﱭ' Letter
    std::pair<QChar, const char*>{ QChar(0x30D9), "bekatakana" },                   // Character 'ベ' Letter
    std::pair<QChar, const char*>{ QChar(0x0562), "benarmenian" },                  // Character 'բ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x05D1), "bet" },                          // Character 'ב' Letter
    std::pair<QChar, const char*>{ QChar(0x03B2), "beta" },                         // Character 'β' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x03D0), "betasymbolgreek" },              // Character 'ϐ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0xFB31), "betdagesh" },                    // Character 'בּ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB31), "betdageshhebrew" },              // Character 'בּ' Letter
    std::pair<QChar, const char*>{ QChar(0x05D1), "bethebrew" },                    // Character 'ב' Letter
    std::pair<QChar, const char*>{ QChar(0xFB4C), "betrafehebrew" },                // Character 'בֿ' Letter
    std::pair<QChar, const char*>{ QChar(0x09AD), "bhabengali" },                   // Character 'ভ' Letter
    std::pair<QChar, const char*>{ QChar(0x092D), "bhadeva" },                      // Character 'भ' Letter
    std::pair<QChar, const char*>{ QChar(0x0AAD), "bhagujarati" },                  // Character 'ભ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A2D), "bhagurmukhi" },                  // Character 'ਭ' Letter
    std::pair<QChar, const char*>{ QChar(0x0253), "bhook" },                        // Character 'ɓ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3073), "bihiragana" },                   // Character 'び' Letter
    std::pair<QChar, const char*>{ QChar(0x30D3), "bikatakana" },                   // Character 'ビ' Letter
    std::pair<QChar, const char*>{ QChar(0x0298), "bilabialclick" },                // Character 'ʘ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0A02), "bindigurmukhi" },                // Character 'ਂ' Mark
    std::pair<QChar, const char*>{ QChar(0x3331), "birusquare" },                   // Character '㌱' Symbol
    std::pair<QChar, const char*>{ QChar(0x25CF), "blackcircle" },                  // Character '●' Symbol
    std::pair<QChar, const char*>{ QChar(0x25C6), "blackdiamond" },                 // Character '◆' Symbol
    std::pair<QChar, const char*>{ QChar(0x25BC), "blackdownpointingtriangle" },    // Character '▼' Symbol
    std::pair<QChar, const char*>{ QChar(0x25C4), "blackleftpointingpointer" },     // Character '◄' Symbol
    std::pair<QChar, const char*>{ QChar(0x25C0), "blackleftpointingtriangle" },    // Character '◀' Symbol
    std::pair<QChar, const char*>{ QChar(0x3010), "blacklenticularbracketleft" },   // Character '【' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFE3B), "blacklenticularbracketleftvertical" },// Character '︻' Punctuation
    std::pair<QChar, const char*>{ QChar(0x3011), "blacklenticularbracketright" },  // Character '】' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFE3C), "blacklenticularbracketrightvertical" },// Character '︼' Punctuation
    std::pair<QChar, const char*>{ QChar(0x25E3), "blacklowerlefttriangle" },       // Character '◣' Symbol
    std::pair<QChar, const char*>{ QChar(0x25E2), "blacklowerrighttriangle" },      // Character '◢' Symbol
    std::pair<QChar, const char*>{ QChar(0x25AC), "blackrectangle" },               // Character '▬' Symbol
    std::pair<QChar, const char*>{ QChar(0x25BA), "blackrightpointingpointer" },    // Character '►' Symbol
    std::pair<QChar, const char*>{ QChar(0x25B6), "blackrightpointingtriangle" },   // Character '▶' Symbol
    std::pair<QChar, const char*>{ QChar(0x25AA), "blacksmallsquare" },             // Character '▪' Symbol
    std::pair<QChar, const char*>{ QChar(0x263B), "blacksmilingface" },             // Character '☻' Symbol
    std::pair<QChar, const char*>{ QChar(0x25A0), "blacksquare" },                  // Character '■' Symbol
    std::pair<QChar, const char*>{ QChar(0x2605), "blackstar" },                    // Character '★' Symbol
    std::pair<QChar, const char*>{ QChar(0x25E4), "blackupperlefttriangle" },       // Character '◤' Symbol
    std::pair<QChar, const char*>{ QChar(0x25E5), "blackupperrighttriangle" },      // Character '◥' Symbol
    std::pair<QChar, const char*>{ QChar(0x25B4), "blackuppointingsmalltriangle" }, // Character '▴' Symbol
    std::pair<QChar, const char*>{ QChar(0x25B2), "blackuppointingtriangle" },      // Character '▲' Symbol
    std::pair<QChar, const char*>{ QChar(0x2423), "blank" },                        // Character '␣' Symbol
    std::pair<QChar, const char*>{ QChar(0x1E07), "blinebelow" },                   // Character 'ḇ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x2588), "block" },                        // Character '█' Symbol
    std::pair<QChar, const char*>{ QChar(0xFF42), "bmonospace" },                   // Character 'ｂ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0E1A), "bobaimaithai" },                 // Character 'บ' Letter
    std::pair<QChar, const char*>{ QChar(0x307C), "bohiragana" },                   // Character 'ぼ' Letter
    std::pair<QChar, const char*>{ QChar(0x30DC), "bokatakana" },                   // Character 'ボ' Letter
    std::pair<QChar, const char*>{ QChar(0x249D), "bparen" },                       // Character '⒝' Symbol
    std::pair<QChar, const char*>{ QChar(0x33C3), "bqsquare" },                     // Character '㏃' Symbol
    std::pair<QChar, const char*>{ QChar(0xF8F4), "braceex" },                      //
    std::pair<QChar, const char*>{ QChar(0x007B), "braceleft" },                    // Character '{' Punctuation
    std::pair<QChar, const char*>{ QChar(0xF8F3), "braceleftbt" },                  //
    std::pair<QChar, const char*>{ QChar(0xF8F2), "braceleftmid" },                 //
    std::pair<QChar, const char*>{ QChar(0xFF5B), "braceleftmonospace" },           // Character '｛' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFE5B), "braceleftsmall" },               // Character '﹛' Punctuation
    std::pair<QChar, const char*>{ QChar(0xF8F1), "bracelefttp" },                  //
    std::pair<QChar, const char*>{ QChar(0xFE37), "braceleftvertical" },            // Character '︷' Punctuation
    std::pair<QChar, const char*>{ QChar(0x007D), "braceright" },                   // Character '}' Punctuation
    std::pair<QChar, const char*>{ QChar(0xF8FE), "bracerightbt" },                 //
    std::pair<QChar, const char*>{ QChar(0xF8FD), "bracerightmid" },                //
    std::pair<QChar, const char*>{ QChar(0xFF5D), "bracerightmonospace" },          // Character '｝' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFE5C), "bracerightsmall" },              // Character '﹜' Punctuation
    std::pair<QChar, const char*>{ QChar(0xF8FC), "bracerighttp" },                 //
    std::pair<QChar, const char*>{ QChar(0xFE38), "bracerightvertical" },           // Character '︸' Punctuation
    std::pair<QChar, const char*>{ QChar(0x005B), "bracketleft" },                  // Character '[' Punctuation
    std::pair<QChar, const char*>{ QChar(0xF8F0), "bracketleftbt" },                //
    std::pair<QChar, const char*>{ QChar(0xF8EF), "bracketleftex" },                //
    std::pair<QChar, const char*>{ QChar(0xFF3B), "bracketleftmonospace" },         // Character '［' Punctuation
    std::pair<QChar, const char*>{ QChar(0xF8EE), "bracketlefttp" },                //
    std::pair<QChar, const char*>{ QChar(0x005D), "bracketright" },                 // Character ']' Punctuation
    std::pair<QChar, const char*>{ QChar(0xF8FB), "bracketrightbt" },               //
    std::pair<QChar, const char*>{ QChar(0xF8FA), "bracketrightex" },               //
    std::pair<QChar, const char*>{ QChar(0xFF3D), "bracketrightmonospace" },        // Character '］' Punctuation
    std::pair<QChar, const char*>{ QChar(0xF8F9), "bracketrighttp" },               //
    std::pair<QChar, const char*>{ QChar(0x02D8), "breve" },                        // Character '˘' Symbol
    std::pair<QChar, const char*>{ QChar(0x032E), "brevebelowcmb" },                // Character '̮' Mark
    std::pair<QChar, const char*>{ QChar(0x0306), "brevecmb" },                     // Character '̆' Mark
    std::pair<QChar, const char*>{ QChar(0x032F), "breveinvertedbelowcmb" },        // Character '̯' Mark
    std::pair<QChar, const char*>{ QChar(0x0311), "breveinvertedcmb" },             // Character '̑' Mark
    std::pair<QChar, const char*>{ QChar(0x0361), "breveinverteddoublecmb" },       // Character '͡' Mark
    std::pair<QChar, const char*>{ QChar(0x032A), "bridgebelowcmb" },               // Character '̪' Mark
    std::pair<QChar, const char*>{ QChar(0x033A), "bridgeinvertedbelowcmb" },       // Character '̺' Mark
    std::pair<QChar, const char*>{ QChar(0x00A6), "brokenbar" },                    // Character '¦' Symbol
    std::pair<QChar, const char*>{ QChar(0x0180), "bstroke" },                      // Character 'ƀ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0xF6EA), "bsuperior" },                    //
    std::pair<QChar, const char*>{ QChar(0x0183), "btopbar" },                      // Character 'ƃ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3076), "buhiragana" },                   // Character 'ぶ' Letter
    std::pair<QChar, const char*>{ QChar(0x30D6), "bukatakana" },                   // Character 'ブ' Letter
    std::pair<QChar, const char*>{ QChar(0x2022), "bullet" },                       // Character '•' Punctuation
    std::pair<QChar, const char*>{ QChar(0x25D8), "bulletinverse" },                // Character '◘' Symbol
    std::pair<QChar, const char*>{ QChar(0x2219), "bulletoperator" },               // Character '∙' Symbol
    std::pair<QChar, const char*>{ QChar(0x25CE), "bullseye" },                     // Character '◎' Symbol
    std::pair<QChar, const char*>{ QChar(0x0063), "c" },                            // Character 'c' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x056E), "caarmenian" },                   // Character 'ծ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x099A), "cabengali" },                    // Character 'চ' Letter
    std::pair<QChar, const char*>{ QChar(0x0107), "cacute" },                       // Character 'ć' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x091A), "cadeva" },                       // Character 'च' Letter
    std::pair<QChar, const char*>{ QChar(0x0A9A), "cagujarati" },                   // Character 'ચ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A1A), "cagurmukhi" },                   // Character 'ਚ' Letter
    std::pair<QChar, const char*>{ QChar(0x3388), "calsquare" },                    // Character '㎈' Symbol
    std::pair<QChar, const char*>{ QChar(0x0981), "candrabindubengali" },           // Character 'ঁ' Mark
    std::pair<QChar, const char*>{ QChar(0x0310), "candrabinducmb" },               // Character '̐' Mark
    std::pair<QChar, const char*>{ QChar(0x0901), "candrabindudeva" },              // Character 'ँ' Mark
    std::pair<QChar, const char*>{ QChar(0x0A81), "candrabindugujarati" },          // Character 'ઁ' Mark
    std::pair<QChar, const char*>{ QChar(0x21EA), "capslock" },                     // Character '⇪' Symbol
    std::pair<QChar, const char*>{ QChar(0x2105), "careof" },                       // Character '℅' Symbol
    std::pair<QChar, const char*>{ QChar(0x02C7), "caron" },                        // Character 'ˇ' Letter
    std::pair<QChar, const char*>{ QChar(0x032C), "caronbelowcmb" },                // Character '̬' Mark
    std::pair<QChar, const char*>{ QChar(0x030C), "caroncmb" },                     // Character '̌' Mark
    std::pair<QChar, const char*>{ QChar(0x21B5), "carriagereturn" },               // Character '↵' Symbol
    std::pair<QChar, const char*>{ QChar(0x3118), "cbopomofo" },                    // Character 'ㄘ' Letter
    std::pair<QChar, const char*>{ QChar(0x010D), "ccaron" },                       // Character 'č' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x00E7), "ccedilla" },                     // Character 'ç' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E09), "ccedillaacute" },                // Character 'ḉ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x24D2), "ccircle" },                      // Character 'ⓒ' Symbol
    std::pair<QChar, const char*>{ QChar(0x0109), "ccircumflex" },                  // Character 'ĉ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0255), "ccurl" },                        // Character 'ɕ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x010B), "cdot" },                         // Character 'ċ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x010B), "cdotaccent" },                   // Character 'ċ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x33C5), "cdsquare" },                     // Character '㏅' Symbol
    std::pair<QChar, const char*>{ QChar(0x00B8), "cedilla" },                      // Character '¸' Symbol
    std::pair<QChar, const char*>{ QChar(0x0327), "cedillacmb" },                   // Character '̧' Mark
    std::pair<QChar, const char*>{ QChar(0x00A2), "cent" },                         // Character '¢' Symbol
    std::pair<QChar, const char*>{ QChar(0x2103), "centigrade" },                   // Character '℃' Symbol
    std::pair<QChar, const char*>{ QChar(0xF6DF), "centinferior" },                 //
    std::pair<QChar, const char*>{ QChar(0xFFE0), "centmonospace" },                // Character '￠' Symbol
    std::pair<QChar, const char*>{ QChar(0xF7A2), "centoldstyle" },                 //
    std::pair<QChar, const char*>{ QChar(0xF6E0), "centsuperior" },                 //
    std::pair<QChar, const char*>{ QChar(0x0579), "chaarmenian" },                  // Character 'չ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x099B), "chabengali" },                   // Character 'ছ' Letter
    std::pair<QChar, const char*>{ QChar(0x091B), "chadeva" },                      // Character 'छ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A9B), "chagujarati" },                  // Character 'છ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A1B), "chagurmukhi" },                  // Character 'ਛ' Letter
    std::pair<QChar, const char*>{ QChar(0x3114), "chbopomofo" },                   // Character 'ㄔ' Letter
    std::pair<QChar, const char*>{ QChar(0x04BD), "cheabkhasiancyrillic" },         // Character 'ҽ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x2713), "checkmark" },                    // Character '✓' Symbol
    std::pair<QChar, const char*>{ QChar(0x0447), "checyrillic" },                  // Character 'ч' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x04BF), "chedescenderabkhasiancyrillic" },// Character 'ҿ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x04B7), "chedescendercyrillic" },         // Character 'ҷ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x04F5), "chedieresiscyrillic" },          // Character 'ӵ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0573), "cheharmenian" },                 // Character 'ճ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x04CC), "chekhakassiancyrillic" },        // Character 'ӌ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x04B9), "cheverticalstrokecyrillic" },    // Character 'ҹ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x03C7), "chi" },                          // Character 'χ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3277), "chieuchacirclekorean" },         // Character '㉷' Symbol
    std::pair<QChar, const char*>{ QChar(0x3217), "chieuchaparenkorean" },          // Character '㈗' Symbol
    std::pair<QChar, const char*>{ QChar(0x3269), "chieuchcirclekorean" },          // Character '㉩' Symbol
    std::pair<QChar, const char*>{ QChar(0x314A), "chieuchkorean" },                // Character 'ㅊ' Letter
    std::pair<QChar, const char*>{ QChar(0x3209), "chieuchparenkorean" },           // Character '㈉' Symbol
    std::pair<QChar, const char*>{ QChar(0x0E0A), "chochangthai" },                 // Character 'ช' Letter
    std::pair<QChar, const char*>{ QChar(0x0E08), "chochanthai" },                  // Character 'จ' Letter
    std::pair<QChar, const char*>{ QChar(0x0E09), "chochingthai" },                 // Character 'ฉ' Letter
    std::pair<QChar, const char*>{ QChar(0x0E0C), "chochoethai" },                  // Character 'ฌ' Letter
    std::pair<QChar, const char*>{ QChar(0x0188), "chook" },                        // Character 'ƈ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3276), "cieucacirclekorean" },           // Character '㉶' Symbol
    std::pair<QChar, const char*>{ QChar(0x3216), "cieucaparenkorean" },            // Character '㈖' Symbol
    std::pair<QChar, const char*>{ QChar(0x3268), "cieuccirclekorean" },            // Character '㉨' Symbol
    std::pair<QChar, const char*>{ QChar(0x3148), "cieuckorean" },                  // Character 'ㅈ' Letter
    std::pair<QChar, const char*>{ QChar(0x3208), "cieucparenkorean" },             // Character '㈈' Symbol
    std::pair<QChar, const char*>{ QChar(0x321C), "cieucuparenkorean" },            // Character '㈜' Symbol
    std::pair<QChar, const char*>{ QChar(0x25CB), "circle" },                       // Character '○' Symbol
    std::pair<QChar, const char*>{ QChar(0x2297), "circlemultiply" },               // Character '⊗' Symbol
    std::pair<QChar, const char*>{ QChar(0x2299), "circleot" },                     // Character '⊙' Symbol
    std::pair<QChar, const char*>{ QChar(0x2295), "circleplus" },                   // Character '⊕' Symbol
    std::pair<QChar, const char*>{ QChar(0x3036), "circlepostalmark" },             // Character '〶' Symbol
    std::pair<QChar, const char*>{ QChar(0x25D0), "circlewithlefthalfblack" },      // Character '◐' Symbol
    std::pair<QChar, const char*>{ QChar(0x25D1), "circlewithrighthalfblack" },     // Character '◑' Symbol
    std::pair<QChar, const char*>{ QChar(0x02C6), "circumflex" },                   // Character 'ˆ' Letter
    std::pair<QChar, const char*>{ QChar(0x032D), "circumflexbelowcmb" },           // Character '̭' Mark
    std::pair<QChar, const char*>{ QChar(0x0302), "circumflexcmb" },                // Character '̂' Mark
    std::pair<QChar, const char*>{ QChar(0x2327), "clear" },                        // Character '⌧' Symbol
    std::pair<QChar, const char*>{ QChar(0x01C2), "clickalveolar" },                // Character 'ǂ' Letter
    std::pair<QChar, const char*>{ QChar(0x01C0), "clickdental" },                  // Character 'ǀ' Letter
    std::pair<QChar, const char*>{ QChar(0x01C1), "clicklateral" },                 // Character 'ǁ' Letter
    std::pair<QChar, const char*>{ QChar(0x01C3), "clickretroflex" },               // Character 'ǃ' Letter
    std::pair<QChar, const char*>{ QChar(0x2663), "club" },                         // Character '♣' Symbol
    std::pair<QChar, const char*>{ QChar(0x2663), "clubsuitblack" },                // Character '♣' Symbol
    std::pair<QChar, const char*>{ QChar(0x2667), "clubsuitwhite" },                // Character '♧' Symbol
    std::pair<QChar, const char*>{ QChar(0x33A4), "cmcubedsquare" },                // Character '㎤' Symbol
    std::pair<QChar, const char*>{ QChar(0xFF43), "cmonospace" },                   // Character 'ｃ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x33A0), "cmsquaredsquare" },              // Character '㎠' Symbol
    std::pair<QChar, const char*>{ QChar(0x0581), "coarmenian" },                   // Character 'ց' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x003A), "colon" },                        // Character ':' Punctuation
    std::pair<QChar, const char*>{ QChar(0x20A1), "colonmonetary" },                // Character '₡' Symbol
    std::pair<QChar, const char*>{ QChar(0xFF1A), "colonmonospace" },               // Character '：' Punctuation
    std::pair<QChar, const char*>{ QChar(0x20A1), "colonsign" },                    // Character '₡' Symbol
    std::pair<QChar, const char*>{ QChar(0xFE55), "colonsmall" },                   // Character '﹕' Punctuation
    std::pair<QChar, const char*>{ QChar(0x02D1), "colontriangularhalfmod" },       // Character 'ˑ' Letter
    std::pair<QChar, const char*>{ QChar(0x02D0), "colontriangularmod" },           // Character 'ː' Letter
    std::pair<QChar, const char*>{ QChar(0x002C), "comma" },                        // Character ',' Punctuation
    std::pair<QChar, const char*>{ QChar(0x0313), "commaabovecmb" },                // Character '̓' Mark
    std::pair<QChar, const char*>{ QChar(0x0315), "commaaboverightcmb" },           // Character '̕' Mark
    std::pair<QChar, const char*>{ QChar(0xF6C3), "commaaccent" },                  //
    std::pair<QChar, const char*>{ QChar(0x060C), "commaarabic" },                  // Character '،' Punctuation
    std::pair<QChar, const char*>{ QChar(0x055D), "commaarmenian" },                // Character '՝' Punctuation
    std::pair<QChar, const char*>{ QChar(0xF6E1), "commainferior" },                //
    std::pair<QChar, const char*>{ QChar(0xFF0C), "commamonospace" },               // Character '，' Punctuation
    std::pair<QChar, const char*>{ QChar(0x0314), "commareversedabovecmb" },        // Character '̔' Mark
    std::pair<QChar, const char*>{ QChar(0x02BD), "commareversedmod" },             // Character 'ʽ' Letter
    std::pair<QChar, const char*>{ QChar(0xFE50), "commasmall" },                   // Character '﹐' Punctuation
    std::pair<QChar, const char*>{ QChar(0xF6E2), "commasuperior" },                //
    std::pair<QChar, const char*>{ QChar(0x0312), "commaturnedabovecmb" },          // Character '̒' Mark
    std::pair<QChar, const char*>{ QChar(0x02BB), "commaturnedmod" },               // Character 'ʻ' Letter
    std::pair<QChar, const char*>{ QChar(0x263C), "compass" },                      // Character '☼' Symbol
    std::pair<QChar, const char*>{ QChar(0x2245), "congruent" },                    // Character '≅' Symbol
    std::pair<QChar, const char*>{ QChar(0x222E), "contourintegral" },              // Character '∮' Symbol
    std::pair<QChar, const char*>{ QChar(0x2303), "control" },                      // Character '⌃' Symbol
    std::pair<QChar, const char*>{ QChar(0x0006), "controlACK" },                   //
    std::pair<QChar, const char*>{ QChar(0x0007), "controlBEL" },                   //
    std::pair<QChar, const char*>{ QChar(0x0008), "controlBS" },                    //
    std::pair<QChar, const char*>{ QChar(0x0018), "controlCAN" },                   //
    std::pair<QChar, const char*>{ QChar(0x000D), "controlCR" },                    // Whitespace
    std::pair<QChar, const char*>{ QChar(0x0011), "controlDC1" },                   //
    std::pair<QChar, const char*>{ QChar(0x0012), "controlDC2" },                   //
    std::pair<QChar, const char*>{ QChar(0x0013), "controlDC3" },                   //
    std::pair<QChar, const char*>{ QChar(0x0014), "controlDC4" },                   //
    std::pair<QChar, const char*>{ QChar(0x007F), "controlDEL" },                   //
    std::pair<QChar, const char*>{ QChar(0x0010), "controlDLE" },                   //
    std::pair<QChar, const char*>{ QChar(0x0019), "controlEM" },                    //
    std::pair<QChar, const char*>{ QChar(0x0005), "controlENQ" },                   //
    std::pair<QChar, const char*>{ QChar(0x0004), "controlEOT" },                   //
    std::pair<QChar, const char*>{ QChar(0x001B), "controlESC" },                   //
    std::pair<QChar, const char*>{ QChar(0x0017), "controlETB" },                   //
    std::pair<QChar, const char*>{ QChar(0x0003), "controlETX" },                   //
    std::pair<QChar, const char*>{ QChar(0x000C), "controlFF" },                    // Whitespace
    std::pair<QChar, const char*>{ QChar(0x001C), "controlFS" },                    //
    std::pair<QChar, const char*>{ QChar(0x001D), "controlGS" },                    //
    std::pair<QChar, const char*>{ QChar(0x0009), "controlHT" },                    // Whitespace
    std::pair<QChar, const char*>{ QChar(0x000A), "controlLF" },                    // Whitespace
    std::pair<QChar, const char*>{ QChar(0x0015), "controlNAK" },                   //
    std::pair<QChar, const char*>{ QChar(0x001E), "controlRS" },                    //
    std::pair<QChar, const char*>{ QChar(0x000F), "controlSI" },                    //
    std::pair<QChar, const char*>{ QChar(0x000E), "controlSO" },                    //
    std::pair<QChar, const char*>{ QChar(0x0002), "controlSOT" },                   //
    std::pair<QChar, const char*>{ QChar(0x0001), "controlSTX" },                   //
    std::pair<QChar, const char*>{ QChar(0x001A), "controlSUB" },                   //
    std::pair<QChar, const char*>{ QChar(0x0016), "controlSYN" },                   //
    std::pair<QChar, const char*>{ QChar(0x001F), "controlUS" },                    //
    std::pair<QChar, const char*>{ QChar(0x000B), "controlVT" },                    // Whitespace
    std::pair<QChar, const char*>{ QChar(0x00A9), "copyright" },                    // Character '©' Symbol
    std::pair<QChar, const char*>{ QChar(0xF8E9), "copyrightsans" },                //
    std::pair<QChar, const char*>{ QChar(0xF6D9), "copyrightserif" },               //
    std::pair<QChar, const char*>{ QChar(0x300C), "cornerbracketleft" },            // Character '「' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFF62), "cornerbracketlefthalfwidth" },   // Character '｢' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFE41), "cornerbracketleftvertical" },    // Character '﹁' Punctuation
    std::pair<QChar, const char*>{ QChar(0x300D), "cornerbracketright" },           // Character '」' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFF63), "cornerbracketrighthalfwidth" },  // Character '｣' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFE42), "cornerbracketrightvertical" },   // Character '﹂' Punctuation
    std::pair<QChar, const char*>{ QChar(0x337F), "corporationsquare" },            // Character '㍿' Symbol
    std::pair<QChar, const char*>{ QChar(0x33C7), "cosquare" },                     // Character '㏇' Symbol
    std::pair<QChar, const char*>{ QChar(0x33C6), "coverkgsquare" },                // Character '㏆' Symbol
    std::pair<QChar, const char*>{ QChar(0x249E), "cparen" },                       // Character '⒞' Symbol
    std::pair<QChar, const char*>{ QChar(0x20A2), "cruzeiro" },                     // Character '₢' Symbol
    std::pair<QChar, const char*>{ QChar(0x0297), "cstretched" },                   // Character 'ʗ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x22CF), "curlyand" },                     // Character '⋏' Symbol
    std::pair<QChar, const char*>{ QChar(0x22CE), "curlyor" },                      // Character '⋎' Symbol
    std::pair<QChar, const char*>{ QChar(0x00A4), "currency" },                     // Character '¤' Symbol
    std::pair<QChar, const char*>{ QChar(0xF6D1), "cyrBreve" },                     //
    std::pair<QChar, const char*>{ QChar(0xF6D2), "cyrFlex" },                      //
    std::pair<QChar, const char*>{ QChar(0xF6D4), "cyrbreve" },                     //
    std::pair<QChar, const char*>{ QChar(0xF6D5), "cyrflex" },                      //
    std::pair<QChar, const char*>{ QChar(0x0064), "d" },                            // Character 'd' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0564), "daarmenian" },                   // Character 'դ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x09A6), "dabengali" },                    // Character 'দ' Letter
    std::pair<QChar, const char*>{ QChar(0x0636), "dadarabic" },                    // Character 'ض' Letter
    std::pair<QChar, const char*>{ QChar(0x0926), "dadeva" },                       // Character 'द' Letter
    std::pair<QChar, const char*>{ QChar(0xFEBE), "dadfinalarabic" },               // Character 'ﺾ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEBF), "dadinitialarabic" },             // Character 'ﺿ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEC0), "dadmedialarabic" },              // Character 'ﻀ' Letter
    std::pair<QChar, const char*>{ QChar(0x05BC), "dagesh" },                       // Character 'ּ' Mark
    std::pair<QChar, const char*>{ QChar(0x05BC), "dageshhebrew" },                 // Character 'ּ' Mark
    std::pair<QChar, const char*>{ QChar(0x2020), "dagger" },                       // Character '†' Punctuation
    std::pair<QChar, const char*>{ QChar(0x2021), "daggerdbl" },                    // Character '‡' Punctuation
    std::pair<QChar, const char*>{ QChar(0x0AA6), "dagujarati" },                   // Character 'દ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A26), "dagurmukhi" },                   // Character 'ਦ' Letter
    std::pair<QChar, const char*>{ QChar(0x3060), "dahiragana" },                   // Character 'だ' Letter
    std::pair<QChar, const char*>{ QChar(0x30C0), "dakatakana" },                   // Character 'ダ' Letter
    std::pair<QChar, const char*>{ QChar(0x062F), "dalarabic" },                    // Character 'د' Letter
    std::pair<QChar, const char*>{ QChar(0x05D3), "dalet" },                        // Character 'ד' Letter
    std::pair<QChar, const char*>{ QChar(0xFB33), "daletdagesh" },                  // Character 'דּ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB33), "daletdageshhebrew" },            // Character 'דּ' Letter
    std::pair<QChar, const char*>{ QChar(0x05B2), "dalethatafpatah" },              // Character 'ֲ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B2), "dalethatafpatahhebrew" },        // Character 'ֲ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B1), "dalethatafsegol" },              // Character 'ֱ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B1), "dalethatafsegolhebrew" },        // Character 'ֱ' Mark
    std::pair<QChar, const char*>{ QChar(0x05D3), "dalethebrew" },                  // Character 'ד' Letter
    std::pair<QChar, const char*>{ QChar(0x05B4), "dalethiriq" },                   // Character 'ִ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B4), "dalethiriqhebrew" },             // Character 'ִ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B9), "daletholam" },                   // Character 'ֹ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B9), "daletholamhebrew" },             // Character 'ֹ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B7), "daletpatah" },                   // Character 'ַ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B7), "daletpatahhebrew" },             // Character 'ַ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B8), "daletqamats" },                  // Character 'ָ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B8), "daletqamatshebrew" },            // Character 'ָ' Mark
    std::pair<QChar, const char*>{ QChar(0x05BB), "daletqubuts" },                  // Character 'ֻ' Mark
    std::pair<QChar, const char*>{ QChar(0x05BB), "daletqubutshebrew" },            // Character 'ֻ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B6), "daletsegol" },                   // Character 'ֶ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B6), "daletsegolhebrew" },             // Character 'ֶ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B0), "daletsheva" },                   // Character 'ְ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B0), "daletshevahebrew" },             // Character 'ְ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B5), "dalettsere" },                   // Character 'ֵ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B5), "dalettserehebrew" },             // Character 'ֵ' Mark
    std::pair<QChar, const char*>{ QChar(0xFEAA), "dalfinalarabic" },               // Character 'ﺪ' Letter
    std::pair<QChar, const char*>{ QChar(0x064F), "dammaarabic" },                  // Character 'ُ' Mark
    std::pair<QChar, const char*>{ QChar(0x064F), "dammalowarabic" },               // Character 'ُ' Mark
    std::pair<QChar, const char*>{ QChar(0x064C), "dammatanaltonearabic" },         // Character 'ٌ' Mark
    std::pair<QChar, const char*>{ QChar(0x064C), "dammatanarabic" },               // Character 'ٌ' Mark
    std::pair<QChar, const char*>{ QChar(0x0964), "danda" },                        // Character '।' Punctuation
    std::pair<QChar, const char*>{ QChar(0x05A7), "dargahebrew" },                  // Character '֧' Mark
    std::pair<QChar, const char*>{ QChar(0x05A7), "dargalefthebrew" },              // Character '֧' Mark
    std::pair<QChar, const char*>{ QChar(0x0485), "dasiapneumatacyrilliccmb" },     // Character '҅' Mark
    std::pair<QChar, const char*>{ QChar(0xF6D3), "dblGrave" },                     //
    std::pair<QChar, const char*>{ QChar(0x300A), "dblanglebracketleft" },          // Character '《' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFE3D), "dblanglebracketleftvertical" },  // Character '︽' Punctuation
    std::pair<QChar, const char*>{ QChar(0x300B), "dblanglebracketright" },         // Character '》' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFE3E), "dblanglebracketrightvertical" }, // Character '︾' Punctuation
    std::pair<QChar, const char*>{ QChar(0x032B), "dblarchinvertedbelowcmb" },      // Character '̫' Mark
    std::pair<QChar, const char*>{ QChar(0x21D4), "dblarrowleft" },                 // Character '⇔' Symbol
    std::pair<QChar, const char*>{ QChar(0x21D2), "dblarrowright" },                // Character '⇒' Symbol
    std::pair<QChar, const char*>{ QChar(0x0965), "dbldanda" },                     // Character '॥' Punctuation
    std::pair<QChar, const char*>{ QChar(0xF6D6), "dblgrave" },                     //
    std::pair<QChar, const char*>{ QChar(0x030F), "dblgravecmb" },                  // Character '̏' Mark
    std::pair<QChar, const char*>{ QChar(0x222C), "dblintegral" },                  // Character '∬' Symbol
    std::pair<QChar, const char*>{ QChar(0x2017), "dbllowline" },                   // Character '‗' Punctuation
    std::pair<QChar, const char*>{ QChar(0x0333), "dbllowlinecmb" },                // Character '̳' Mark
    std::pair<QChar, const char*>{ QChar(0x033F), "dbloverlinecmb" },               // Character '̿' Mark
    std::pair<QChar, const char*>{ QChar(0x02BA), "dblprimemod" },                  // Character 'ʺ' Letter
    std::pair<QChar, const char*>{ QChar(0x2016), "dblverticalbar" },               // Character '‖' Punctuation
    std::pair<QChar, const char*>{ QChar(0x030E), "dblverticallineabovecmb" },      // Character '̎' Mark
    std::pair<QChar, const char*>{ QChar(0x3109), "dbopomofo" },                    // Character 'ㄉ' Letter
    std::pair<QChar, const char*>{ QChar(0x33C8), "dbsquare" },                     // Character '㏈' Symbol
    std::pair<QChar, const char*>{ QChar(0x010F), "dcaron" },                       // Character 'ď' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E11), "dcedilla" },                     // Character 'ḑ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x24D3), "dcircle" },                      // Character 'ⓓ' Symbol
    std::pair<QChar, const char*>{ QChar(0x1E13), "dcircumflexbelow" },             // Character 'ḓ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0111), "dcroat" },                       // Character 'đ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x09A1), "ddabengali" },                   // Character 'ড' Letter
    std::pair<QChar, const char*>{ QChar(0x0921), "ddadeva" },                      // Character 'ड' Letter
    std::pair<QChar, const char*>{ QChar(0x0AA1), "ddagujarati" },                  // Character 'ડ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A21), "ddagurmukhi" },                  // Character 'ਡ' Letter
    std::pair<QChar, const char*>{ QChar(0x0688), "ddalarabic" },                   // Character 'ڈ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB89), "ddalfinalarabic" },              // Character 'ﮉ' Letter
    std::pair<QChar, const char*>{ QChar(0x095C), "dddhadeva" },                    // Character 'ड़' Letter
    std::pair<QChar, const char*>{ QChar(0x09A2), "ddhabengali" },                  // Character 'ঢ' Letter
    std::pair<QChar, const char*>{ QChar(0x0922), "ddhadeva" },                     // Character 'ढ' Letter
    std::pair<QChar, const char*>{ QChar(0x0AA2), "ddhagujarati" },                 // Character 'ઢ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A22), "ddhagurmukhi" },                 // Character 'ਢ' Letter
    std::pair<QChar, const char*>{ QChar(0x1E0B), "ddotaccent" },                   // Character 'ḋ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E0D), "ddotbelow" },                    // Character 'ḍ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x066B), "decimalseparatorarabic" },       // Character '٫' Punctuation
    std::pair<QChar, const char*>{ QChar(0x066B), "decimalseparatorpersian" },      // Character '٫' Punctuation
    std::pair<QChar, const char*>{ QChar(0x0434), "decyrillic" },                   // Character 'д' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x00B0), "degree" },                       // Character '°' Symbol
    std::pair<QChar, const char*>{ QChar(0x05AD), "dehihebrew" },                   // Character '֭' Mark
    std::pair<QChar, const char*>{ QChar(0x3067), "dehiragana" },                   // Character 'で' Letter
    std::pair<QChar, const char*>{ QChar(0x03EF), "deicoptic" },                    // Character 'ϯ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x30C7), "dekatakana" },                   // Character 'デ' Letter
    std::pair<QChar, const char*>{ QChar(0x232B), "deleteleft" },                   // Character '⌫' Symbol
    std::pair<QChar, const char*>{ QChar(0x2326), "deleteright" },                  // Character '⌦' Symbol
    std::pair<QChar, const char*>{ QChar(0x03B4), "delta" },                        // Character 'δ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x018D), "deltaturned" },                  // Character 'ƍ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x09F8), "denominatorminusonenumeratorbengali" },// Character '৸'
    std::pair<QChar, const char*>{ QChar(0x02A4), "dezh" },                         // Character 'ʤ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x09A7), "dhabengali" },                   // Character 'ধ' Letter
    std::pair<QChar, const char*>{ QChar(0x0927), "dhadeva" },                      // Character 'ध' Letter
    std::pair<QChar, const char*>{ QChar(0x0AA7), "dhagujarati" },                  // Character 'ધ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A27), "dhagurmukhi" },                  // Character 'ਧ' Letter
    std::pair<QChar, const char*>{ QChar(0x0257), "dhook" },                        // Character 'ɗ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0385), "dialytikatonos" },               // Character '΅' Symbol
    std::pair<QChar, const char*>{ QChar(0x0344), "dialytikatonoscmb" },            // Character '̈́' Mark
    std::pair<QChar, const char*>{ QChar(0x2666), "diamond" },                      // Character '♦' Symbol
    std::pair<QChar, const char*>{ QChar(0x2662), "diamondsuitwhite" },             // Character '♢' Symbol
    std::pair<QChar, const char*>{ QChar(0x00A8), "dieresis" },                     // Character '¨' Symbol
    std::pair<QChar, const char*>{ QChar(0xF6D7), "dieresisacute" },                //
    std::pair<QChar, const char*>{ QChar(0x0324), "dieresisbelowcmb" },             // Character '̤' Mark
    std::pair<QChar, const char*>{ QChar(0x0308), "dieresiscmb" },                  // Character '̈' Mark
    std::pair<QChar, const char*>{ QChar(0xF6D8), "dieresisgrave" },                //
    std::pair<QChar, const char*>{ QChar(0x0385), "dieresistonos" },                // Character '΅' Symbol
    std::pair<QChar, const char*>{ QChar(0x3062), "dihiragana" },                   // Character 'ぢ' Letter
    std::pair<QChar, const char*>{ QChar(0x30C2), "dikatakana" },                   // Character 'ヂ' Letter
    std::pair<QChar, const char*>{ QChar(0x3003), "dittomark" },                    // Character '〃' Punctuation
    std::pair<QChar, const char*>{ QChar(0x00F7), "divide" },                       // Character '÷' Symbol
    std::pair<QChar, const char*>{ QChar(0x2223), "divides" },                      // Character '∣' Symbol
    std::pair<QChar, const char*>{ QChar(0x2215), "divisionslash" },                // Character '∕' Symbol
    std::pair<QChar, const char*>{ QChar(0x0452), "djecyrillic" },                  // Character 'ђ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x2593), "dkshade" },                      // Character '▓' Symbol
    std::pair<QChar, const char*>{ QChar(0x1E0F), "dlinebelow" },                   // Character 'ḏ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3397), "dlsquare" },                     // Character '㎗' Symbol
    std::pair<QChar, const char*>{ QChar(0x0111), "dmacron" },                      // Character 'đ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0xFF44), "dmonospace" },                   // Character 'ｄ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x2584), "dnblock" },                      // Character '▄' Symbol
    std::pair<QChar, const char*>{ QChar(0x0E0E), "dochadathai" },                  // Character 'ฎ' Letter
    std::pair<QChar, const char*>{ QChar(0x0E14), "dodekthai" },                    // Character 'ด' Letter
    std::pair<QChar, const char*>{ QChar(0x3069), "dohiragana" },                   // Character 'ど' Letter
    std::pair<QChar, const char*>{ QChar(0x30C9), "dokatakana" },                   // Character 'ド' Letter
    std::pair<QChar, const char*>{ QChar(0x0024), "dollar" },                       // Character '$' Symbol
    std::pair<QChar, const char*>{ QChar(0xF6E3), "dollarinferior" },               //
    std::pair<QChar, const char*>{ QChar(0xFF04), "dollarmonospace" },              // Character '＄' Symbol
    std::pair<QChar, const char*>{ QChar(0xF724), "dollaroldstyle" },               //
    std::pair<QChar, const char*>{ QChar(0xFE69), "dollarsmall" },                  // Character '﹩' Symbol
    std::pair<QChar, const char*>{ QChar(0xF6E4), "dollarsuperior" },               //
    std::pair<QChar, const char*>{ QChar(0x20AB), "dong" },                         // Character '₫' Symbol
    std::pair<QChar, const char*>{ QChar(0x3326), "dorusquare" },                   // Character '㌦' Symbol
    std::pair<QChar, const char*>{ QChar(0x02D9), "dotaccent" },                    // Character '˙' Symbol
    std::pair<QChar, const char*>{ QChar(0x0307), "dotaccentcmb" },                 // Character '̇' Mark
    std::pair<QChar, const char*>{ QChar(0x0323), "dotbelowcmb" },                  // Character '̣' Mark
    std::pair<QChar, const char*>{ QChar(0x0323), "dotbelowcomb" },                 // Character '̣' Mark
    std::pair<QChar, const char*>{ QChar(0x30FB), "dotkatakana" },                  // Character '・' Punctuation
    std::pair<QChar, const char*>{ QChar(0x0131), "dotlessi" },                     // Character 'ı' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0xF6BE), "dotlessj" },                     //
    std::pair<QChar, const char*>{ QChar(0x0284), "dotlessjstrokehook" },           // Character 'ʄ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x22C5), "dotmath" },                      // Character '⋅' Symbol
    std::pair<QChar, const char*>{ QChar(0x25CC), "dottedcircle" },                 // Character '◌' Symbol
    std::pair<QChar, const char*>{ QChar(0xFB1F), "doubleyodpatah" },               // Character 'ײַ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB1F), "doubleyodpatahhebrew" },         // Character 'ײַ' Letter
    std::pair<QChar, const char*>{ QChar(0x031E), "downtackbelowcmb" },             // Character '̞' Mark
    std::pair<QChar, const char*>{ QChar(0x02D5), "downtackmod" },                  // Character '˕' Symbol
    std::pair<QChar, const char*>{ QChar(0x249F), "dparen" },                       // Character '⒟' Symbol
    std::pair<QChar, const char*>{ QChar(0xF6EB), "dsuperior" },                    //
    std::pair<QChar, const char*>{ QChar(0x0256), "dtail" },                        // Character 'ɖ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x018C), "dtopbar" },                      // Character 'ƌ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3065), "duhiragana" },                   // Character 'づ' Letter
    std::pair<QChar, const char*>{ QChar(0x30C5), "dukatakana" },                   // Character 'ヅ' Letter
    std::pair<QChar, const char*>{ QChar(0x01F3), "dz" },                           // Character 'ǳ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x02A3), "dzaltone" },                     // Character 'ʣ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x01C6), "dzcaron" },                      // Character 'ǆ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x02A5), "dzcurl" },                       // Character 'ʥ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x04E1), "dzeabkhasiancyrillic" },         // Character 'ӡ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0455), "dzecyrillic" },                  // Character 'ѕ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x045F), "dzhecyrillic" },                 // Character 'џ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0065), "e" },                            // Character 'e' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x00E9), "eacute" },                       // Character 'é' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x2641), "earth" },                        // Character '♁' Symbol
    std::pair<QChar, const char*>{ QChar(0x098F), "ebengali" },                     // Character 'এ' Letter
    std::pair<QChar, const char*>{ QChar(0x311C), "ebopomofo" },                    // Character 'ㄜ' Letter
    std::pair<QChar, const char*>{ QChar(0x0115), "ebreve" },                       // Character 'ĕ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x090D), "ecandradeva" },                  // Character 'ऍ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A8D), "ecandragujarati" },              // Character 'ઍ' Letter
    std::pair<QChar, const char*>{ QChar(0x0945), "ecandravowelsigndeva" },         // Character 'ॅ' Mark
    std::pair<QChar, const char*>{ QChar(0x0AC5), "ecandravowelsigngujarati" },     // Character 'ૅ' Mark
    std::pair<QChar, const char*>{ QChar(0x011B), "ecaron" },                       // Character 'ě' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E1D), "ecedillabreve" },                // Character 'ḝ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0565), "echarmenian" },                  // Character 'ե' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0587), "echyiwnarmenian" },              // Character 'և' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x24D4), "ecircle" },                      // Character 'ⓔ' Symbol
    std::pair<QChar, const char*>{ QChar(0x00EA), "ecircumflex" },                  // Character 'ê' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1EBF), "ecircumflexacute" },             // Character 'ế' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E19), "ecircumflexbelow" },             // Character 'ḙ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1EC7), "ecircumflexdotbelow" },          // Character 'ệ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1EC1), "ecircumflexgrave" },             // Character 'ề' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1EC3), "ecircumflexhookabove" },         // Character 'ể' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1EC5), "ecircumflextilde" },             // Character 'ễ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0454), "ecyrillic" },                    // Character 'є' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0205), "edblgrave" },                    // Character 'ȅ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x090F), "edeva" },                        // Character 'ए' Letter
    std::pair<QChar, const char*>{ QChar(0x00EB), "edieresis" },                    // Character 'ë' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0117), "edot" },                         // Character 'ė' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0117), "edotaccent" },                   // Character 'ė' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1EB9), "edotbelow" },                    // Character 'ẹ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0A0F), "eegurmukhi" },                   // Character 'ਏ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A47), "eematragurmukhi" },              // Character 'ੇ' Mark
    std::pair<QChar, const char*>{ QChar(0x0444), "efcyrillic" },                   // Character 'ф' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x00E8), "egrave" },                       // Character 'è' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0A8F), "egujarati" },                    // Character 'એ' Letter
    std::pair<QChar, const char*>{ QChar(0x0567), "eharmenian" },                   // Character 'է' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x311D), "ehbopomofo" },                   // Character 'ㄝ' Letter
    std::pair<QChar, const char*>{ QChar(0x3048), "ehiragana" },                    // Character 'え' Letter
    std::pair<QChar, const char*>{ QChar(0x1EBB), "ehookabove" },                   // Character 'ẻ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x311F), "eibopomofo" },                   // Character 'ㄟ' Letter
    std::pair<QChar, const char*>{ QChar(0x0038), "eight" },                        // Character '8' Digit
    std::pair<QChar, const char*>{ QChar(0x0668), "eightarabic" },                  // Character '٨' Digit
    std::pair<QChar, const char*>{ QChar(0x09EE), "eightbengali" },                 // Character '৮' Digit
    std::pair<QChar, const char*>{ QChar(0x2467), "eightcircle" },                  // Character '⑧'
    std::pair<QChar, const char*>{ QChar(0x2791), "eightcircleinversesansserif" },  // Character '➑'
    std::pair<QChar, const char*>{ QChar(0x096E), "eightdeva" },                    // Character '८' Digit
    std::pair<QChar, const char*>{ QChar(0x2471), "eighteencircle" },               // Character '⑱'
    std::pair<QChar, const char*>{ QChar(0x2485), "eighteenparen" },                // Character '⒅'
    std::pair<QChar, const char*>{ QChar(0x2499), "eighteenperiod" },               // Character '⒙'
    std::pair<QChar, const char*>{ QChar(0x0AEE), "eightgujarati" },                // Character '૮' Digit
    std::pair<QChar, const char*>{ QChar(0x0A6E), "eightgurmukhi" },                // Character '੮' Digit
    std::pair<QChar, const char*>{ QChar(0x0668), "eighthackarabic" },              // Character '٨' Digit
    std::pair<QChar, const char*>{ QChar(0x3028), "eighthangzhou" },                // Character '〨'
    std::pair<QChar, const char*>{ QChar(0x266B), "eighthnotebeamed" },             // Character '♫' Symbol
    std::pair<QChar, const char*>{ QChar(0x3227), "eightideographicparen" },        // Character '㈧'
    std::pair<QChar, const char*>{ QChar(0x2088), "eightinferior" },                // Character '₈'
    std::pair<QChar, const char*>{ QChar(0xFF18), "eightmonospace" },               // Character '８' Digit
    std::pair<QChar, const char*>{ QChar(0xF738), "eightoldstyle" },                //
    std::pair<QChar, const char*>{ QChar(0x247B), "eightparen" },                   // Character '⑻'
    std::pair<QChar, const char*>{ QChar(0x248F), "eightperiod" },                  // Character '⒏'
    std::pair<QChar, const char*>{ QChar(0x06F8), "eightpersian" },                 // Character '۸' Digit
    std::pair<QChar, const char*>{ QChar(0x2177), "eightroman" },                   // Character 'ⅷ'
    std::pair<QChar, const char*>{ QChar(0x2078), "eightsuperior" },                // Character '⁸'
    std::pair<QChar, const char*>{ QChar(0x0E58), "eightthai" },                    // Character '๘' Digit
    std::pair<QChar, const char*>{ QChar(0x0207), "einvertedbreve" },               // Character 'ȇ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0465), "eiotifiedcyrillic" },            // Character 'ѥ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x30A8), "ekatakana" },                    // Character 'エ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF74), "ekatakanahalfwidth" },           // Character 'ｴ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A74), "ekonkargurmukhi" },              // Character 'ੴ' Letter
    std::pair<QChar, const char*>{ QChar(0x3154), "ekorean" },                      // Character 'ㅔ' Letter
    std::pair<QChar, const char*>{ QChar(0x043B), "elcyrillic" },                   // Character 'л' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x2208), "element" },                      // Character '∈' Symbol
    std::pair<QChar, const char*>{ QChar(0x246A), "elevencircle" },                 // Character '⑪'
    std::pair<QChar, const char*>{ QChar(0x247E), "elevenparen" },                  // Character '⑾'
    std::pair<QChar, const char*>{ QChar(0x2492), "elevenperiod" },                 // Character '⒒'
    std::pair<QChar, const char*>{ QChar(0x217A), "elevenroman" },                  // Character 'ⅺ'
    std::pair<QChar, const char*>{ QChar(0x2026), "ellipsis" },                     // Character '…' Punctuation
    std::pair<QChar, const char*>{ QChar(0x22EE), "ellipsisvertical" },             // Character '⋮' Symbol
    std::pair<QChar, const char*>{ QChar(0x0113), "emacron" },                      // Character 'ē' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E17), "emacronacute" },                 // Character 'ḗ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E15), "emacrongrave" },                 // Character 'ḕ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x043C), "emcyrillic" },                   // Character 'м' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x2014), "emdash" },                       // Character '—' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFE31), "emdashvertical" },               // Character '︱' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFF45), "emonospace" },                   // Character 'ｅ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x055B), "emphasismarkarmenian" },         // Character '՛' Punctuation
    std::pair<QChar, const char*>{ QChar(0x2205), "emptyset" },                     // Character '∅' Symbol
    std::pair<QChar, const char*>{ QChar(0x3123), "enbopomofo" },                   // Character 'ㄣ' Letter
    std::pair<QChar, const char*>{ QChar(0x043D), "encyrillic" },                   // Character 'н' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x2013), "endash" },                       // Character '–' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFE32), "endashvertical" },               // Character '︲' Punctuation
    std::pair<QChar, const char*>{ QChar(0x04A3), "endescendercyrillic" },          // Character 'ң' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x014B), "eng" },                          // Character 'ŋ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3125), "engbopomofo" },                  // Character 'ㄥ' Letter
    std::pair<QChar, const char*>{ QChar(0x04A5), "enghecyrillic" },                // Character 'ҥ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x04C8), "enhookcyrillic" },               // Character 'ӈ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x2002), "enspace" },                      // Character ' ' Whitespace
    std::pair<QChar, const char*>{ QChar(0x0119), "eogonek" },                      // Character 'ę' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3153), "eokorean" },                     // Character 'ㅓ' Letter
    std::pair<QChar, const char*>{ QChar(0x025B), "eopen" },                        // Character 'ɛ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x029A), "eopenclosed" },                  // Character 'ʚ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x025C), "eopenreversed" },                // Character 'ɜ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x025E), "eopenreversedclosed" },          // Character 'ɞ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x025D), "eopenreversedhook" },            // Character 'ɝ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x24A0), "eparen" },                       // Character '⒠' Symbol
    std::pair<QChar, const char*>{ QChar(0x03B5), "epsilon" },                      // Character 'ε' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x03AD), "epsilontonos" },                 // Character 'έ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x003D), "equal" },                        // Character '=' Symbol
    std::pair<QChar, const char*>{ QChar(0xFF1D), "equalmonospace" },               // Character '＝' Symbol
    std::pair<QChar, const char*>{ QChar(0xFE66), "equalsmall" },                   // Character '﹦' Symbol
    std::pair<QChar, const char*>{ QChar(0x207C), "equalsuperior" },                // Character '⁼' Symbol
    std::pair<QChar, const char*>{ QChar(0x2261), "equivalence" },                  // Character '≡' Symbol
    std::pair<QChar, const char*>{ QChar(0x3126), "erbopomofo" },                   // Character 'ㄦ' Letter
    std::pair<QChar, const char*>{ QChar(0x0440), "ercyrillic" },                   // Character 'р' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0258), "ereversed" },                    // Character 'ɘ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x044D), "ereversedcyrillic" },            // Character 'э' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0441), "escyrillic" },                   // Character 'с' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x04AB), "esdescendercyrillic" },          // Character 'ҫ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0283), "esh" },                          // Character 'ʃ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0286), "eshcurl" },                      // Character 'ʆ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x090E), "eshortdeva" },                   // Character 'ऎ' Letter
    std::pair<QChar, const char*>{ QChar(0x0946), "eshortvowelsigndeva" },          // Character 'ॆ' Mark
    std::pair<QChar, const char*>{ QChar(0x01AA), "eshreversedloop" },              // Character 'ƪ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0285), "eshsquatreversed" },             // Character 'ʅ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3047), "esmallhiragana" },               // Character 'ぇ' Letter
    std::pair<QChar, const char*>{ QChar(0x30A7), "esmallkatakana" },               // Character 'ェ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF6A), "esmallkatakanahalfwidth" },      // Character 'ｪ' Letter
    std::pair<QChar, const char*>{ QChar(0x212E), "estimated" },                    // Character '℮' Symbol
    std::pair<QChar, const char*>{ QChar(0xF6EC), "esuperior" },                    //
    std::pair<QChar, const char*>{ QChar(0x03B7), "eta" },                          // Character 'η' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0568), "etarmenian" },                   // Character 'ը' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x03AE), "etatonos" },                     // Character 'ή' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x00F0), "eth" },                          // Character 'ð' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1EBD), "etilde" },                       // Character 'ẽ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E1B), "etildebelow" },                  // Character 'ḛ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0591), "etnahtafoukhhebrew" },           // Character '֑' Mark
    std::pair<QChar, const char*>{ QChar(0x0591), "etnahtafoukhlefthebrew" },       // Character '֑' Mark
    std::pair<QChar, const char*>{ QChar(0x0591), "etnahtahebrew" },                // Character '֑' Mark
    std::pair<QChar, const char*>{ QChar(0x0591), "etnahtalefthebrew" },            // Character '֑' Mark
    std::pair<QChar, const char*>{ QChar(0x01DD), "eturned" },                      // Character 'ǝ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3161), "eukorean" },                     // Character 'ㅡ' Letter
    std::pair<QChar, const char*>{ QChar(0x20AC), "euro" },                         // Character '€' Symbol
    std::pair<QChar, const char*>{ QChar(0x09C7), "evowelsignbengali" },            // Character 'ে' Mark
    std::pair<QChar, const char*>{ QChar(0x0947), "evowelsigndeva" },               // Character 'े' Mark
    std::pair<QChar, const char*>{ QChar(0x0AC7), "evowelsigngujarati" },           // Character 'ે' Mark
    std::pair<QChar, const char*>{ QChar(0x0021), "exclam" },                       // Character '!' Punctuation
    std::pair<QChar, const char*>{ QChar(0x055C), "exclamarmenian" },               // Character '՜' Punctuation
    std::pair<QChar, const char*>{ QChar(0x203C), "exclamdbl" },                    // Character '‼' Punctuation
    std::pair<QChar, const char*>{ QChar(0x00A1), "exclamdown" },                   // Character '¡' Punctuation
    std::pair<QChar, const char*>{ QChar(0xF7A1), "exclamdownsmall" },              //
    std::pair<QChar, const char*>{ QChar(0xFF01), "exclammonospace" },              // Character '！' Punctuation
    std::pair<QChar, const char*>{ QChar(0xF721), "exclamsmall" },                  //
    std::pair<QChar, const char*>{ QChar(0x2203), "existential" },                  // Character '∃' Symbol
    std::pair<QChar, const char*>{ QChar(0x0292), "ezh" },                          // Character 'ʒ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x01EF), "ezhcaron" },                     // Character 'ǯ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0293), "ezhcurl" },                      // Character 'ʓ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x01B9), "ezhreversed" },                  // Character 'ƹ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x01BA), "ezhtail" },                      // Character 'ƺ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0066), "f" },                            // Character 'f' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0xFB00), "f_f" },                          // Character 'ﬀ' Letter, Lowercase // NOT LISTED IN UNICODE CHARACTER LIST
    std::pair<QChar, const char*>{ QChar(0xFB03), "f_f_i" },                        // Character 'ﬃ' Letter, Lowercase // NOT LISTED IN UNICODE CHARACTER LIST
    std::pair<QChar, const char*>{ QChar(0xFB04), "f_f_l" },                        // Character 'ﬄ' Letter, Lowercase // NOT LISTED IN UNICODE CHARACTER LIST
    std::pair<QChar, const char*>{ QChar(0xFB01), "f_i" },                          // Character 'ﬁ' Letter, Lowercase // NOT LISTED IN UNICODE CHARACTER LIST
    std::pair<QChar, const char*>{ QChar(0x095E), "fadeva" },                       // Character 'फ़' Letter
    std::pair<QChar, const char*>{ QChar(0x0A5E), "fagurmukhi" },                   // Character 'ਫ਼' Letter
    std::pair<QChar, const char*>{ QChar(0x2109), "fahrenheit" },                   // Character '℉' Symbol
    std::pair<QChar, const char*>{ QChar(0x064E), "fathaarabic" },                  // Character 'َ' Mark
    std::pair<QChar, const char*>{ QChar(0x064E), "fathalowarabic" },               // Character 'َ' Mark
    std::pair<QChar, const char*>{ QChar(0x064B), "fathatanarabic" },               // Character 'ً' Mark
    std::pair<QChar, const char*>{ QChar(0x3108), "fbopomofo" },                    // Character 'ㄈ' Letter
    std::pair<QChar, const char*>{ QChar(0x24D5), "fcircle" },                      // Character 'ⓕ' Symbol
    std::pair<QChar, const char*>{ QChar(0x1E1F), "fdotaccent" },                   // Character 'ḟ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0641), "feharabic" },                    // Character 'ف' Letter
    std::pair<QChar, const char*>{ QChar(0x0586), "feharmenian" },                  // Character 'ֆ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0xFED2), "fehfinalarabic" },               // Character 'ﻒ' Letter
    std::pair<QChar, const char*>{ QChar(0xFED3), "fehinitialarabic" },             // Character 'ﻓ' Letter
    std::pair<QChar, const char*>{ QChar(0xFED4), "fehmedialarabic" },              // Character 'ﻔ' Letter
    std::pair<QChar, const char*>{ QChar(0x03E5), "feicoptic" },                    // Character 'ϥ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x2640), "female" },                       // Character '♀' Symbol
    std::pair<QChar, const char*>{ QChar(0xFB00), "ff" },                           // Character 'ﬀ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0xFB03), "ffi" },                          // Character 'ﬃ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0xFB04), "ffl" },                          // Character 'ﬄ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0xFB01), "fi" },                           // Character 'ﬁ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x246E), "fifteencircle" },                // Character '⑮'
    std::pair<QChar, const char*>{ QChar(0x2482), "fifteenparen" },                 // Character '⒂'
    std::pair<QChar, const char*>{ QChar(0x2496), "fifteenperiod" },                // Character '⒖'
    std::pair<QChar, const char*>{ QChar(0x2012), "figuredash" },                   // Character '‒' Punctuation
    std::pair<QChar, const char*>{ QChar(0x25A0), "filledbox" },                    // Character '■' Symbol
    std::pair<QChar, const char*>{ QChar(0x25AC), "filledrect" },                   // Character '▬' Symbol
    std::pair<QChar, const char*>{ QChar(0x05DA), "finalkaf" },                     // Character 'ך' Letter
    std::pair<QChar, const char*>{ QChar(0xFB3A), "finalkafdagesh" },               // Character 'ךּ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB3A), "finalkafdageshhebrew" },         // Character 'ךּ' Letter
    std::pair<QChar, const char*>{ QChar(0x05DA), "finalkafhebrew" },               // Character 'ך' Letter
    std::pair<QChar, const char*>{ QChar(0x05B8), "finalkafqamats" },               // Character 'ָ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B8), "finalkafqamatshebrew" },         // Character 'ָ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B0), "finalkafsheva" },                // Character 'ְ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B0), "finalkafshevahebrew" },          // Character 'ְ' Mark
    std::pair<QChar, const char*>{ QChar(0x05DD), "finalmem" },                     // Character 'ם' Letter
    std::pair<QChar, const char*>{ QChar(0x05DD), "finalmemhebrew" },               // Character 'ם' Letter
    std::pair<QChar, const char*>{ QChar(0x05DF), "finalnun" },                     // Character 'ן' Letter
    std::pair<QChar, const char*>{ QChar(0x05DF), "finalnunhebrew" },               // Character 'ן' Letter
    std::pair<QChar, const char*>{ QChar(0x05E3), "finalpe" },                      // Character 'ף' Letter
    std::pair<QChar, const char*>{ QChar(0x05E3), "finalpehebrew" },                // Character 'ף' Letter
    std::pair<QChar, const char*>{ QChar(0x05E5), "finaltsadi" },                   // Character 'ץ' Letter
    std::pair<QChar, const char*>{ QChar(0x05E5), "finaltsadihebrew" },             // Character 'ץ' Letter
    std::pair<QChar, const char*>{ QChar(0x02C9), "firsttonechinese" },             // Character 'ˉ' Letter
    std::pair<QChar, const char*>{ QChar(0x25C9), "fisheye" },                      // Character '◉' Symbol
    std::pair<QChar, const char*>{ QChar(0x0473), "fitacyrillic" },                 // Character 'ѳ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0035), "five" },                         // Character '5' Digit
    std::pair<QChar, const char*>{ QChar(0x0665), "fivearabic" },                   // Character '٥' Digit
    std::pair<QChar, const char*>{ QChar(0x09EB), "fivebengali" },                  // Character '৫' Digit
    std::pair<QChar, const char*>{ QChar(0x2464), "fivecircle" },                   // Character '⑤'
    std::pair<QChar, const char*>{ QChar(0x278E), "fivecircleinversesansserif" },   // Character '➎'
    std::pair<QChar, const char*>{ QChar(0x096B), "fivedeva" },                     // Character '५' Digit
    std::pair<QChar, const char*>{ QChar(0x215D), "fiveeighths" },                  // Character '⅝'
    std::pair<QChar, const char*>{ QChar(0x0AEB), "fivegujarati" },                 // Character '૫' Digit
    std::pair<QChar, const char*>{ QChar(0x0A6B), "fivegurmukhi" },                 // Character '੫' Digit
    std::pair<QChar, const char*>{ QChar(0x0665), "fivehackarabic" },               // Character '٥' Digit
    std::pair<QChar, const char*>{ QChar(0x3025), "fivehangzhou" },                 // Character '〥'
    std::pair<QChar, const char*>{ QChar(0x3224), "fiveideographicparen" },         // Character '㈤'
    std::pair<QChar, const char*>{ QChar(0x2085), "fiveinferior" },                 // Character '₅'
    std::pair<QChar, const char*>{ QChar(0xFF15), "fivemonospace" },                // Character '５' Digit
    std::pair<QChar, const char*>{ QChar(0xF735), "fiveoldstyle" },                 //
    std::pair<QChar, const char*>{ QChar(0x2478), "fiveparen" },                    // Character '⑸'
    std::pair<QChar, const char*>{ QChar(0x248C), "fiveperiod" },                   // Character '⒌'
    std::pair<QChar, const char*>{ QChar(0x06F5), "fivepersian" },                  // Character '۵' Digit
    std::pair<QChar, const char*>{ QChar(0x2174), "fiveroman" },                    // Character 'ⅴ'
    std::pair<QChar, const char*>{ QChar(0x2075), "fivesuperior" },                 // Character '⁵'
    std::pair<QChar, const char*>{ QChar(0x0E55), "fivethai" },                     // Character '๕' Digit
    std::pair<QChar, const char*>{ QChar(0xFB02), "fl" },                           // Character 'ﬂ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0192), "florin" },                       // Character 'ƒ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0xFF46), "fmonospace" },                   // Character 'ｆ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3399), "fmsquare" },                     // Character '㎙' Symbol
    std::pair<QChar, const char*>{ QChar(0x0E1F), "fofanthai" },                    // Character 'ฟ' Letter
    std::pair<QChar, const char*>{ QChar(0x0E1D), "fofathai" },                     // Character 'ฝ' Letter
    std::pair<QChar, const char*>{ QChar(0x0E4F), "fongmanthai" },                  // Character '๏' Punctuation
    std::pair<QChar, const char*>{ QChar(0x2200), "forall" },                       // Character '∀' Symbol
    std::pair<QChar, const char*>{ QChar(0x0034), "four" },                         // Character '4' Digit
    std::pair<QChar, const char*>{ QChar(0x0664), "fourarabic" },                   // Character '٤' Digit
    std::pair<QChar, const char*>{ QChar(0x09EA), "fourbengali" },                  // Character '৪' Digit
    std::pair<QChar, const char*>{ QChar(0x2463), "fourcircle" },                   // Character '④'
    std::pair<QChar, const char*>{ QChar(0x278D), "fourcircleinversesansserif" },   // Character '➍'
    std::pair<QChar, const char*>{ QChar(0x096A), "fourdeva" },                     // Character '४' Digit
    std::pair<QChar, const char*>{ QChar(0x0AEA), "fourgujarati" },                 // Character '૪' Digit
    std::pair<QChar, const char*>{ QChar(0x0A6A), "fourgurmukhi" },                 // Character '੪' Digit
    std::pair<QChar, const char*>{ QChar(0x0664), "fourhackarabic" },               // Character '٤' Digit
    std::pair<QChar, const char*>{ QChar(0x3024), "fourhangzhou" },                 // Character '〤'
    std::pair<QChar, const char*>{ QChar(0x3223), "fourideographicparen" },         // Character '㈣'
    std::pair<QChar, const char*>{ QChar(0x2084), "fourinferior" },                 // Character '₄'
    std::pair<QChar, const char*>{ QChar(0xFF14), "fourmonospace" },                // Character '４' Digit
    std::pair<QChar, const char*>{ QChar(0x09F7), "fournumeratorbengali" },         // Character '৷'
    std::pair<QChar, const char*>{ QChar(0xF734), "fouroldstyle" },                 //
    std::pair<QChar, const char*>{ QChar(0x2477), "fourparen" },                    // Character '⑷'
    std::pair<QChar, const char*>{ QChar(0x248B), "fourperiod" },                   // Character '⒋'
    std::pair<QChar, const char*>{ QChar(0x06F4), "fourpersian" },                  // Character '۴' Digit
    std::pair<QChar, const char*>{ QChar(0x2173), "fourroman" },                    // Character 'ⅳ'
    std::pair<QChar, const char*>{ QChar(0x2074), "foursuperior" },                 // Character '⁴'
    std::pair<QChar, const char*>{ QChar(0x246D), "fourteencircle" },               // Character '⑭'
    std::pair<QChar, const char*>{ QChar(0x2481), "fourteenparen" },                // Character '⒁'
    std::pair<QChar, const char*>{ QChar(0x2495), "fourteenperiod" },               // Character '⒕'
    std::pair<QChar, const char*>{ QChar(0x0E54), "fourthai" },                     // Character '๔' Digit
    std::pair<QChar, const char*>{ QChar(0x02CB), "fourthtonechinese" },            // Character 'ˋ' Letter
    std::pair<QChar, const char*>{ QChar(0x24A1), "fparen" },                       // Character '⒡' Symbol
    std::pair<QChar, const char*>{ QChar(0x2044), "fraction" },                     // Character '⁄' Symbol
    std::pair<QChar, const char*>{ QChar(0x20A3), "franc" },                        // Character '₣' Symbol
    std::pair<QChar, const char*>{ QChar(0x0067), "g" },                            // Character 'g' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0997), "gabengali" },                    // Character 'গ' Letter
    std::pair<QChar, const char*>{ QChar(0x01F5), "gacute" },                       // Character 'ǵ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0917), "gadeva" },                       // Character 'ग' Letter
    std::pair<QChar, const char*>{ QChar(0x06AF), "gafarabic" },                    // Character 'گ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB93), "gaffinalarabic" },               // Character 'ﮓ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB94), "gafinitialarabic" },             // Character 'ﮔ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB95), "gafmedialarabic" },              // Character 'ﮕ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A97), "gagujarati" },                   // Character 'ગ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A17), "gagurmukhi" },                   // Character 'ਗ' Letter
    std::pair<QChar, const char*>{ QChar(0x304C), "gahiragana" },                   // Character 'が' Letter
    std::pair<QChar, const char*>{ QChar(0x30AC), "gakatakana" },                   // Character 'ガ' Letter
    std::pair<QChar, const char*>{ QChar(0x03B3), "gamma" },                        // Character 'γ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0263), "gammalatinsmall" },              // Character 'ɣ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x02E0), "gammasuperior" },                // Character 'ˠ' Letter
    std::pair<QChar, const char*>{ QChar(0x03EB), "gangiacoptic" },                 // Character 'ϫ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x310D), "gbopomofo" },                    // Character 'ㄍ' Letter
    std::pair<QChar, const char*>{ QChar(0x011F), "gbreve" },                       // Character 'ğ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x01E7), "gcaron" },                       // Character 'ǧ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0123), "gcedilla" },                     // Character 'ģ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x24D6), "gcircle" },                      // Character 'ⓖ' Symbol
    std::pair<QChar, const char*>{ QChar(0x011D), "gcircumflex" },                  // Character 'ĝ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0123), "gcommaaccent" },                 // Character 'ģ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0121), "gdot" },                         // Character 'ġ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0121), "gdotaccent" },                   // Character 'ġ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0433), "gecyrillic" },                   // Character 'г' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3052), "gehiragana" },                   // Character 'げ' Letter
    std::pair<QChar, const char*>{ QChar(0x30B2), "gekatakana" },                   // Character 'ゲ' Letter
    std::pair<QChar, const char*>{ QChar(0x2251), "geometricallyequal" },           // Character '≑' Symbol
    std::pair<QChar, const char*>{ QChar(0x059C), "gereshaccenthebrew" },           // Character '֜' Mark
    std::pair<QChar, const char*>{ QChar(0x05F3), "gereshhebrew" },                 // Character '׳' Punctuation
    std::pair<QChar, const char*>{ QChar(0x059D), "gereshmuqdamhebrew" },           // Character '֝' Mark
    std::pair<QChar, const char*>{ QChar(0x00DF), "germandbls" },                   // Character 'ß' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x059E), "gershayimaccenthebrew" },        // Character '֞' Mark
    std::pair<QChar, const char*>{ QChar(0x05F4), "gershayimhebrew" },              // Character '״' Punctuation
    std::pair<QChar, const char*>{ QChar(0x3013), "getamark" },                     // Character '〓' Symbol
    std::pair<QChar, const char*>{ QChar(0x0998), "ghabengali" },                   // Character 'ঘ' Letter
    std::pair<QChar, const char*>{ QChar(0x0572), "ghadarmenian" },                 // Character 'ղ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0918), "ghadeva" },                      // Character 'घ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A98), "ghagujarati" },                  // Character 'ઘ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A18), "ghagurmukhi" },                  // Character 'ਘ' Letter
    std::pair<QChar, const char*>{ QChar(0x063A), "ghainarabic" },                  // Character 'غ' Letter
    std::pair<QChar, const char*>{ QChar(0xFECE), "ghainfinalarabic" },             // Character 'ﻎ' Letter
    std::pair<QChar, const char*>{ QChar(0xFECF), "ghaininitialarabic" },           // Character 'ﻏ' Letter
    std::pair<QChar, const char*>{ QChar(0xFED0), "ghainmedialarabic" },            // Character 'ﻐ' Letter
    std::pair<QChar, const char*>{ QChar(0x0495), "ghemiddlehookcyrillic" },        // Character 'ҕ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0493), "ghestrokecyrillic" },            // Character 'ғ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0491), "gheupturncyrillic" },            // Character 'ґ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x095A), "ghhadeva" },                     // Character 'ग़' Letter
    std::pair<QChar, const char*>{ QChar(0x0A5A), "ghhagurmukhi" },                 // Character 'ਗ਼' Letter
    std::pair<QChar, const char*>{ QChar(0x0260), "ghook" },                        // Character 'ɠ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3393), "ghzsquare" },                    // Character '㎓' Symbol
    std::pair<QChar, const char*>{ QChar(0x304E), "gihiragana" },                   // Character 'ぎ' Letter
    std::pair<QChar, const char*>{ QChar(0x30AE), "gikatakana" },                   // Character 'ギ' Letter
    std::pair<QChar, const char*>{ QChar(0x0563), "gimarmenian" },                  // Character 'գ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x05D2), "gimel" },                        // Character 'ג' Letter
    std::pair<QChar, const char*>{ QChar(0xFB32), "gimeldagesh" },                  // Character 'גּ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB32), "gimeldageshhebrew" },            // Character 'גּ' Letter
    std::pair<QChar, const char*>{ QChar(0x05D2), "gimelhebrew" },                  // Character 'ג' Letter
    std::pair<QChar, const char*>{ QChar(0x0453), "gjecyrillic" },                  // Character 'ѓ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x01BE), "glottalinvertedstroke" },        // Character 'ƾ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0294), "glottalstop" },                  // Character 'ʔ' Letter
    std::pair<QChar, const char*>{ QChar(0x0296), "glottalstopinverted" },          // Character 'ʖ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x02C0), "glottalstopmod" },               // Character 'ˀ' Letter
    std::pair<QChar, const char*>{ QChar(0x0295), "glottalstopreversed" },          // Character 'ʕ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x02C1), "glottalstopreversedmod" },       // Character 'ˁ' Letter
    std::pair<QChar, const char*>{ QChar(0x02E4), "glottalstopreversedsuperior" },  // Character 'ˤ' Letter
    std::pair<QChar, const char*>{ QChar(0x02A1), "glottalstopstroke" },            // Character 'ʡ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x02A2), "glottalstopstrokereversed" },    // Character 'ʢ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E21), "gmacron" },                      // Character 'ḡ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0xFF47), "gmonospace" },                   // Character 'ｇ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3054), "gohiragana" },                   // Character 'ご' Letter
    std::pair<QChar, const char*>{ QChar(0x30B4), "gokatakana" },                   // Character 'ゴ' Letter
    std::pair<QChar, const char*>{ QChar(0x24A2), "gparen" },                       // Character '⒢' Symbol
    std::pair<QChar, const char*>{ QChar(0x33AC), "gpasquare" },                    // Character '㎬' Symbol
    std::pair<QChar, const char*>{ QChar(0x2207), "gradient" },                     // Character '∇' Symbol
    std::pair<QChar, const char*>{ QChar(0x0060), "grave" },                        // Character '`' Symbol
    std::pair<QChar, const char*>{ QChar(0x0316), "gravebelowcmb" },                // Character '̖' Mark
    std::pair<QChar, const char*>{ QChar(0x0300), "gravecmb" },                     // Character '̀' Mark
    std::pair<QChar, const char*>{ QChar(0x0300), "gravecomb" },                    // Character '̀' Mark
    std::pair<QChar, const char*>{ QChar(0x0953), "gravedeva" },                    // Character '॓' Mark
    std::pair<QChar, const char*>{ QChar(0x02CE), "gravelowmod" },                  // Character 'ˎ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF40), "gravemonospace" },               // Character '｀' Symbol
    std::pair<QChar, const char*>{ QChar(0x0340), "gravetonecmb" },                 // Character '̀' Mark
    std::pair<QChar, const char*>{ QChar(0x003E), "greater" },                      // Character '>' Symbol
    std::pair<QChar, const char*>{ QChar(0x2265), "greaterequal" },                 // Character '≥' Symbol
    std::pair<QChar, const char*>{ QChar(0x22DB), "greaterequalorless" },           // Character '⋛' Symbol
    std::pair<QChar, const char*>{ QChar(0xFF1E), "greatermonospace" },             // Character '＞' Symbol
    std::pair<QChar, const char*>{ QChar(0x2273), "greaterorequivalent" },          // Character '≳' Symbol
    std::pair<QChar, const char*>{ QChar(0x2277), "greaterorless" },                // Character '≷' Symbol
    std::pair<QChar, const char*>{ QChar(0x2267), "greateroverequal" },             // Character '≧' Symbol
    std::pair<QChar, const char*>{ QChar(0xFE65), "greatersmall" },                 // Character '﹥' Symbol
    std::pair<QChar, const char*>{ QChar(0x0261), "gscript" },                      // Character 'ɡ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x01E5), "gstroke" },                      // Character 'ǥ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3050), "guhiragana" },                   // Character 'ぐ' Letter
    std::pair<QChar, const char*>{ QChar(0x00AB), "guillemotleft" },                // Character '«' Punctuation
    std::pair<QChar, const char*>{ QChar(0x00BB), "guillemotright" },               // Character '»' Punctuation
    std::pair<QChar, const char*>{ QChar(0x2039), "guilsinglleft" },                // Character '‹' Punctuation
    std::pair<QChar, const char*>{ QChar(0x203A), "guilsinglright" },               // Character '›' Punctuation
    std::pair<QChar, const char*>{ QChar(0x30B0), "gukatakana" },                   // Character 'グ' Letter
    std::pair<QChar, const char*>{ QChar(0x3318), "guramusquare" },                 // Character '㌘' Symbol
    std::pair<QChar, const char*>{ QChar(0x33C9), "gysquare" },                     // Character '㏉' Symbol
    std::pair<QChar, const char*>{ QChar(0x0068), "h" },                            // Character 'h' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x04A9), "haabkhasiancyrillic" },          // Character 'ҩ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x06C1), "haaltonearabic" },               // Character 'ہ' Letter
    std::pair<QChar, const char*>{ QChar(0x09B9), "habengali" },                    // Character 'হ' Letter
    std::pair<QChar, const char*>{ QChar(0x04B3), "hadescendercyrillic" },          // Character 'ҳ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0939), "hadeva" },                       // Character 'ह' Letter
    std::pair<QChar, const char*>{ QChar(0x0AB9), "hagujarati" },                   // Character 'હ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A39), "hagurmukhi" },                   // Character 'ਹ' Letter
    std::pair<QChar, const char*>{ QChar(0x062D), "haharabic" },                    // Character 'ح' Letter
    std::pair<QChar, const char*>{ QChar(0xFEA2), "hahfinalarabic" },               // Character 'ﺢ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEA3), "hahinitialarabic" },             // Character 'ﺣ' Letter
    std::pair<QChar, const char*>{ QChar(0x306F), "hahiragana" },                   // Character 'は' Letter
    std::pair<QChar, const char*>{ QChar(0xFEA4), "hahmedialarabic" },              // Character 'ﺤ' Letter
    std::pair<QChar, const char*>{ QChar(0x332A), "haitusquare" },                  // Character '㌪' Symbol
    std::pair<QChar, const char*>{ QChar(0x30CF), "hakatakana" },                   // Character 'ハ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF8A), "hakatakanahalfwidth" },          // Character 'ﾊ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A4D), "halantgurmukhi" },               // Character '੍' Mark
    std::pair<QChar, const char*>{ QChar(0x0621), "hamzaarabic" },                  // Character 'ء' Letter
    std::pair<QChar, const char*>{ QChar(0x064F), "hamzadammaarabic" },             // Character 'ُ' Mark
    std::pair<QChar, const char*>{ QChar(0x064C), "hamzadammatanarabic" },          // Character 'ٌ' Mark
    std::pair<QChar, const char*>{ QChar(0x064E), "hamzafathaarabic" },             // Character 'َ' Mark
    std::pair<QChar, const char*>{ QChar(0x064B), "hamzafathatanarabic" },          // Character 'ً' Mark
    std::pair<QChar, const char*>{ QChar(0x0621), "hamzalowarabic" },               // Character 'ء' Letter
    std::pair<QChar, const char*>{ QChar(0x0650), "hamzalowkasraarabic" },          // Character 'ِ' Mark
    std::pair<QChar, const char*>{ QChar(0x064D), "hamzalowkasratanarabic" },       // Character 'ٍ' Mark
    std::pair<QChar, const char*>{ QChar(0x0652), "hamzasukunarabic" },             // Character 'ْ' Mark
    std::pair<QChar, const char*>{ QChar(0x3164), "hangulfiller" },                 // Character 'ㅤ' Letter
    std::pair<QChar, const char*>{ QChar(0x044A), "hardsigncyrillic" },             // Character 'ъ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x21BC), "harpoonleftbarbup" },            // Character '↼' Symbol
    std::pair<QChar, const char*>{ QChar(0x21C0), "harpoonrightbarbup" },           // Character '⇀' Symbol
    std::pair<QChar, const char*>{ QChar(0x33CA), "hasquare" },                     // Character '㏊' Symbol
    std::pair<QChar, const char*>{ QChar(0x05B2), "hatafpatah" },                   // Character 'ֲ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B2), "hatafpatah16" },                 // Character 'ֲ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B2), "hatafpatah23" },                 // Character 'ֲ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B2), "hatafpatah2f" },                 // Character 'ֲ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B2), "hatafpatahhebrew" },             // Character 'ֲ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B2), "hatafpatahnarrowhebrew" },       // Character 'ֲ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B2), "hatafpatahquarterhebrew" },      // Character 'ֲ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B2), "hatafpatahwidehebrew" },         // Character 'ֲ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B3), "hatafqamats" },                  // Character 'ֳ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B3), "hatafqamats1b" },                // Character 'ֳ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B3), "hatafqamats28" },                // Character 'ֳ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B3), "hatafqamats34" },                // Character 'ֳ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B3), "hatafqamatshebrew" },            // Character 'ֳ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B3), "hatafqamatsnarrowhebrew" },      // Character 'ֳ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B3), "hatafqamatsquarterhebrew" },     // Character 'ֳ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B3), "hatafqamatswidehebrew" },        // Character 'ֳ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B1), "hatafsegol" },                   // Character 'ֱ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B1), "hatafsegol17" },                 // Character 'ֱ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B1), "hatafsegol24" },                 // Character 'ֱ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B1), "hatafsegol30" },                 // Character 'ֱ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B1), "hatafsegolhebrew" },             // Character 'ֱ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B1), "hatafsegolnarrowhebrew" },       // Character 'ֱ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B1), "hatafsegolquarterhebrew" },      // Character 'ֱ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B1), "hatafsegolwidehebrew" },         // Character 'ֱ' Mark
    std::pair<QChar, const char*>{ QChar(0x0127), "hbar" },                         // Character 'ħ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x310F), "hbopomofo" },                    // Character 'ㄏ' Letter
    std::pair<QChar, const char*>{ QChar(0x1E2B), "hbrevebelow" },                  // Character 'ḫ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E29), "hcedilla" },                     // Character 'ḩ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x24D7), "hcircle" },                      // Character 'ⓗ' Symbol
    std::pair<QChar, const char*>{ QChar(0x0125), "hcircumflex" },                  // Character 'ĥ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E27), "hdieresis" },                    // Character 'ḧ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E23), "hdotaccent" },                   // Character 'ḣ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E25), "hdotbelow" },                    // Character 'ḥ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x05D4), "he" },                           // Character 'ה' Letter
    std::pair<QChar, const char*>{ QChar(0x2665), "heart" },                        // Character '♥' Symbol
    std::pair<QChar, const char*>{ QChar(0x2665), "heartsuitblack" },               // Character '♥' Symbol
    std::pair<QChar, const char*>{ QChar(0x2661), "heartsuitwhite" },               // Character '♡' Symbol
    std::pair<QChar, const char*>{ QChar(0xFB34), "hedagesh" },                     // Character 'הּ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB34), "hedageshhebrew" },               // Character 'הּ' Letter
    std::pair<QChar, const char*>{ QChar(0x06C1), "hehaltonearabic" },              // Character 'ہ' Letter
    std::pair<QChar, const char*>{ QChar(0x0647), "heharabic" },                    // Character 'ه' Letter
    std::pair<QChar, const char*>{ QChar(0x05D4), "hehebrew" },                     // Character 'ה' Letter
    std::pair<QChar, const char*>{ QChar(0xFBA7), "hehfinalaltonearabic" },         // Character 'ﮧ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEEA), "hehfinalalttwoarabic" },         // Character 'ﻪ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEEA), "hehfinalarabic" },               // Character 'ﻪ' Letter
    std::pair<QChar, const char*>{ QChar(0xFBA5), "hehhamzaabovefinalarabic" },     // Character 'ﮥ' Letter
    std::pair<QChar, const char*>{ QChar(0xFBA4), "hehhamzaaboveisolatedarabic" },  // Character 'ﮤ' Letter
    std::pair<QChar, const char*>{ QChar(0xFBA8), "hehinitialaltonearabic" },       // Character 'ﮨ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEEB), "hehinitialarabic" },             // Character 'ﻫ' Letter
    std::pair<QChar, const char*>{ QChar(0x3078), "hehiragana" },                   // Character 'へ' Letter
    std::pair<QChar, const char*>{ QChar(0xFBA9), "hehmedialaltonearabic" },        // Character 'ﮩ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEEC), "hehmedialarabic" },              // Character 'ﻬ' Letter
    std::pair<QChar, const char*>{ QChar(0x337B), "heiseierasquare" },              // Character '㍻' Symbol
    std::pair<QChar, const char*>{ QChar(0x30D8), "hekatakana" },                   // Character 'ヘ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF8D), "hekatakanahalfwidth" },          // Character 'ﾍ' Letter
    std::pair<QChar, const char*>{ QChar(0x3336), "hekutaarusquare" },              // Character '㌶' Symbol
    std::pair<QChar, const char*>{ QChar(0x0267), "henghook" },                     // Character 'ɧ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3339), "herutusquare" },                 // Character '㌹' Symbol
    std::pair<QChar, const char*>{ QChar(0x05D7), "het" },                          // Character 'ח' Letter
    std::pair<QChar, const char*>{ QChar(0x05D7), "hethebrew" },                    // Character 'ח' Letter
    std::pair<QChar, const char*>{ QChar(0x0266), "hhook" },                        // Character 'ɦ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x02B1), "hhooksuperior" },                // Character 'ʱ' Letter
    std::pair<QChar, const char*>{ QChar(0x327B), "hieuhacirclekorean" },           // Character '㉻' Symbol
    std::pair<QChar, const char*>{ QChar(0x321B), "hieuhaparenkorean" },            // Character '㈛' Symbol
    std::pair<QChar, const char*>{ QChar(0x326D), "hieuhcirclekorean" },            // Character '㉭' Symbol
    std::pair<QChar, const char*>{ QChar(0x314E), "hieuhkorean" },                  // Character 'ㅎ' Letter
    std::pair<QChar, const char*>{ QChar(0x320D), "hieuhparenkorean" },             // Character '㈍' Symbol
    std::pair<QChar, const char*>{ QChar(0x3072), "hihiragana" },                   // Character 'ひ' Letter
    std::pair<QChar, const char*>{ QChar(0x30D2), "hikatakana" },                   // Character 'ヒ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF8B), "hikatakanahalfwidth" },          // Character 'ﾋ' Letter
    std::pair<QChar, const char*>{ QChar(0x05B4), "hiriq" },                        // Character 'ִ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B4), "hiriq14" },                      // Character 'ִ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B4), "hiriq21" },                      // Character 'ִ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B4), "hiriq2d" },                      // Character 'ִ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B4), "hiriqhebrew" },                  // Character 'ִ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B4), "hiriqnarrowhebrew" },            // Character 'ִ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B4), "hiriqquarterhebrew" },           // Character 'ִ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B4), "hiriqwidehebrew" },              // Character 'ִ' Mark
    std::pair<QChar, const char*>{ QChar(0x1E96), "hlinebelow" },                   // Character 'ẖ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0xFF48), "hmonospace" },                   // Character 'ｈ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0570), "hoarmenian" },                   // Character 'հ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0E2B), "hohipthai" },                    // Character 'ห' Letter
    std::pair<QChar, const char*>{ QChar(0x307B), "hohiragana" },                   // Character 'ほ' Letter
    std::pair<QChar, const char*>{ QChar(0x30DB), "hokatakana" },                   // Character 'ホ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF8E), "hokatakanahalfwidth" },          // Character 'ﾎ' Letter
    std::pair<QChar, const char*>{ QChar(0x05B9), "holam" },                        // Character 'ֹ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B9), "holam19" },                      // Character 'ֹ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B9), "holam26" },                      // Character 'ֹ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B9), "holam32" },                      // Character 'ֹ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B9), "holamhebrew" },                  // Character 'ֹ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B9), "holamnarrowhebrew" },            // Character 'ֹ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B9), "holamquarterhebrew" },           // Character 'ֹ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B9), "holamwidehebrew" },              // Character 'ֹ' Mark
    std::pair<QChar, const char*>{ QChar(0x0E2E), "honokhukthai" },                 // Character 'ฮ' Letter
    std::pair<QChar, const char*>{ QChar(0x0309), "hookabovecomb" },                // Character '̉' Mark
    std::pair<QChar, const char*>{ QChar(0x0309), "hookcmb" },                      // Character '̉' Mark
    std::pair<QChar, const char*>{ QChar(0x0321), "hookpalatalizedbelowcmb" },      // Character '̡' Mark
    std::pair<QChar, const char*>{ QChar(0x0322), "hookretroflexbelowcmb" },        // Character '̢' Mark
    std::pair<QChar, const char*>{ QChar(0x3342), "hoonsquare" },                   // Character '㍂' Symbol
    std::pair<QChar, const char*>{ QChar(0x03E9), "horicoptic" },                   // Character 'ϩ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x2015), "horizontalbar" },                // Character '―' Punctuation
    std::pair<QChar, const char*>{ QChar(0x031B), "horncmb" },                      // Character '̛' Mark
    std::pair<QChar, const char*>{ QChar(0x2668), "hotsprings" },                   // Character '♨' Symbol
    std::pair<QChar, const char*>{ QChar(0x2302), "house" },                        // Character '⌂' Symbol
    std::pair<QChar, const char*>{ QChar(0x24A3), "hparen" },                       // Character '⒣' Symbol
    std::pair<QChar, const char*>{ QChar(0x02B0), "hsuperior" },                    // Character 'ʰ' Letter
    std::pair<QChar, const char*>{ QChar(0x0265), "hturned" },                      // Character 'ɥ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3075), "huhiragana" },                   // Character 'ふ' Letter
    std::pair<QChar, const char*>{ QChar(0x3333), "huiitosquare" },                 // Character '㌳' Symbol
    std::pair<QChar, const char*>{ QChar(0x30D5), "hukatakana" },                   // Character 'フ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF8C), "hukatakanahalfwidth" },          // Character 'ﾌ' Letter
    std::pair<QChar, const char*>{ QChar(0x02DD), "hungarumlaut" },                 // Character '˝' Symbol
    std::pair<QChar, const char*>{ QChar(0x030B), "hungarumlautcmb" },              // Character '̋' Mark
    std::pair<QChar, const char*>{ QChar(0x0195), "hv" },                           // Character 'ƕ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x002D), "hyphen" },                       // Character '-' Punctuation
    std::pair<QChar, const char*>{ QChar(0xF6E5), "hypheninferior" },               //
    std::pair<QChar, const char*>{ QChar(0xFF0D), "hyphenmonospace" },              // Character '－' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFE63), "hyphensmall" },                  // Character '﹣' Punctuation
    std::pair<QChar, const char*>{ QChar(0xF6E6), "hyphensuperior" },               //
    std::pair<QChar, const char*>{ QChar(0x2010), "hyphentwo" },                    // Character '‐' Punctuation
    std::pair<QChar, const char*>{ QChar(0x0069), "i" },                            // Character 'i' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x00ED), "iacute" },                       // Character 'í' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x044F), "iacyrillic" },                   // Character 'я' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0987), "ibengali" },                     // Character 'ই' Letter
    std::pair<QChar, const char*>{ QChar(0x3127), "ibopomofo" },                    // Character 'ㄧ' Letter
    std::pair<QChar, const char*>{ QChar(0x012D), "ibreve" },                       // Character 'ĭ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x01D0), "icaron" },                       // Character 'ǐ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x24D8), "icircle" },                      // Character 'ⓘ' Symbol
    std::pair<QChar, const char*>{ QChar(0x00EE), "icircumflex" },                  // Character 'î' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0456), "icyrillic" },                    // Character 'і' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0209), "idblgrave" },                    // Character 'ȉ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x328F), "ideographearthcircle" },         // Character '㊏' Symbol
    std::pair<QChar, const char*>{ QChar(0x328B), "ideographfirecircle" },          // Character '㊋' Symbol
    std::pair<QChar, const char*>{ QChar(0x323F), "ideographicallianceparen" },     // Character '㈿' Symbol
    std::pair<QChar, const char*>{ QChar(0x323A), "ideographiccallparen" },         // Character '㈺' Symbol
    std::pair<QChar, const char*>{ QChar(0x32A5), "ideographiccentrecircle" },      // Character '㊥' Symbol
    std::pair<QChar, const char*>{ QChar(0x3006), "ideographicclose" },             // Character '〆' Letter
    std::pair<QChar, const char*>{ QChar(0x3001), "ideographiccomma" },             // Character '、' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFF64), "ideographiccommaleft" },         // Character '､' Punctuation
    std::pair<QChar, const char*>{ QChar(0x3237), "ideographiccongratulationparen" },// Character '㈷' Symbol
    std::pair<QChar, const char*>{ QChar(0x32A3), "ideographiccorrectcircle" },     // Character '㊣' Symbol
    std::pair<QChar, const char*>{ QChar(0x322F), "ideographicearthparen" },        // Character '㈯' Symbol
    std::pair<QChar, const char*>{ QChar(0x323D), "ideographicenterpriseparen" },   // Character '㈽' Symbol
    std::pair<QChar, const char*>{ QChar(0x329D), "ideographicexcellentcircle" },   // Character '㊝' Symbol
    std::pair<QChar, const char*>{ QChar(0x3240), "ideographicfestivalparen" },     // Character '㉀' Symbol
    std::pair<QChar, const char*>{ QChar(0x3296), "ideographicfinancialcircle" },   // Character '㊖' Symbol
    std::pair<QChar, const char*>{ QChar(0x3236), "ideographicfinancialparen" },    // Character '㈶' Symbol
    std::pair<QChar, const char*>{ QChar(0x322B), "ideographicfireparen" },         // Character '㈫' Symbol
    std::pair<QChar, const char*>{ QChar(0x3232), "ideographichaveparen" },         // Character '㈲' Symbol
    std::pair<QChar, const char*>{ QChar(0x32A4), "ideographichighcircle" },        // Character '㊤' Symbol
    std::pair<QChar, const char*>{ QChar(0x3005), "ideographiciterationmark" },     // Character '々' Letter
    std::pair<QChar, const char*>{ QChar(0x3298), "ideographiclaborcircle" },       // Character '㊘' Symbol
    std::pair<QChar, const char*>{ QChar(0x3238), "ideographiclaborparen" },        // Character '㈸' Symbol
    std::pair<QChar, const char*>{ QChar(0x32A7), "ideographicleftcircle" },        // Character '㊧' Symbol
    std::pair<QChar, const char*>{ QChar(0x32A6), "ideographiclowcircle" },         // Character '㊦' Symbol
    std::pair<QChar, const char*>{ QChar(0x32A9), "ideographicmedicinecircle" },    // Character '㊩' Symbol
    std::pair<QChar, const char*>{ QChar(0x322E), "ideographicmetalparen" },        // Character '㈮' Symbol
    std::pair<QChar, const char*>{ QChar(0x322A), "ideographicmoonparen" },         // Character '㈪' Symbol
    std::pair<QChar, const char*>{ QChar(0x3234), "ideographicnameparen" },         // Character '㈴' Symbol
    std::pair<QChar, const char*>{ QChar(0x3002), "ideographicperiod" },            // Character '。' Punctuation
    std::pair<QChar, const char*>{ QChar(0x329E), "ideographicprintcircle" },       // Character '㊞' Symbol
    std::pair<QChar, const char*>{ QChar(0x3243), "ideographicreachparen" },        // Character '㉃' Symbol
    std::pair<QChar, const char*>{ QChar(0x3239), "ideographicrepresentparen" },    // Character '㈹' Symbol
    std::pair<QChar, const char*>{ QChar(0x323E), "ideographicresourceparen" },     // Character '㈾' Symbol
    std::pair<QChar, const char*>{ QChar(0x32A8), "ideographicrightcircle" },       // Character '㊨' Symbol
    std::pair<QChar, const char*>{ QChar(0x3299), "ideographicsecretcircle" },      // Character '㊙' Symbol
    std::pair<QChar, const char*>{ QChar(0x3242), "ideographicselfparen" },         // Character '㉂' Symbol
    std::pair<QChar, const char*>{ QChar(0x3233), "ideographicsocietyparen" },      // Character '㈳' Symbol
    std::pair<QChar, const char*>{ QChar(0x3000), "ideographicspace" },             // Character '　' Whitespace
    std::pair<QChar, const char*>{ QChar(0x3235), "ideographicspecialparen" },      // Character '㈵' Symbol
    std::pair<QChar, const char*>{ QChar(0x3231), "ideographicstockparen" },        // Character '㈱' Symbol
    std::pair<QChar, const char*>{ QChar(0x323B), "ideographicstudyparen" },        // Character '㈻' Symbol
    std::pair<QChar, const char*>{ QChar(0x3230), "ideographicsunparen" },          // Character '㈰' Symbol
    std::pair<QChar, const char*>{ QChar(0x323C), "ideographicsuperviseparen" },    // Character '㈼' Symbol
    std::pair<QChar, const char*>{ QChar(0x322C), "ideographicwaterparen" },        // Character '㈬' Symbol
    std::pair<QChar, const char*>{ QChar(0x322D), "ideographicwoodparen" },         // Character '㈭' Symbol
    std::pair<QChar, const char*>{ QChar(0x3007), "ideographiczero" },              // Character '〇'
    std::pair<QChar, const char*>{ QChar(0x328E), "ideographmetalcircle" },         // Character '㊎' Symbol
    std::pair<QChar, const char*>{ QChar(0x328A), "ideographmooncircle" },          // Character '㊊' Symbol
    std::pair<QChar, const char*>{ QChar(0x3294), "ideographnamecircle" },          // Character '㊔' Symbol
    std::pair<QChar, const char*>{ QChar(0x3290), "ideographsuncircle" },           // Character '㊐' Symbol
    std::pair<QChar, const char*>{ QChar(0x328C), "ideographwatercircle" },         // Character '㊌' Symbol
    std::pair<QChar, const char*>{ QChar(0x328D), "ideographwoodcircle" },          // Character '㊍' Symbol
    std::pair<QChar, const char*>{ QChar(0x0907), "ideva" },                        // Character 'इ' Letter
    std::pair<QChar, const char*>{ QChar(0x00EF), "idieresis" },                    // Character 'ï' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E2F), "idieresisacute" },               // Character 'ḯ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x04E5), "idieresiscyrillic" },            // Character 'ӥ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1ECB), "idotbelow" },                    // Character 'ị' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x04D7), "iebrevecyrillic" },              // Character 'ӗ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0435), "iecyrillic" },                   // Character 'е' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3275), "ieungacirclekorean" },           // Character '㉵' Symbol
    std::pair<QChar, const char*>{ QChar(0x3215), "ieungaparenkorean" },            // Character '㈕' Symbol
    std::pair<QChar, const char*>{ QChar(0x3267), "ieungcirclekorean" },            // Character '㉧' Symbol
    std::pair<QChar, const char*>{ QChar(0x3147), "ieungkorean" },                  // Character 'ㅇ' Letter
    std::pair<QChar, const char*>{ QChar(0x3207), "ieungparenkorean" },             // Character '㈇' Symbol
    std::pair<QChar, const char*>{ QChar(0x00EC), "igrave" },                       // Character 'ì' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0A87), "igujarati" },                    // Character 'ઇ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A07), "igurmukhi" },                    // Character 'ਇ' Letter
    std::pair<QChar, const char*>{ QChar(0x3044), "ihiragana" },                    // Character 'い' Letter
    std::pair<QChar, const char*>{ QChar(0x1EC9), "ihookabove" },                   // Character 'ỉ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0988), "iibengali" },                    // Character 'ঈ' Letter
    std::pair<QChar, const char*>{ QChar(0x0438), "iicyrillic" },                   // Character 'и' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0908), "iideva" },                       // Character 'ई' Letter
    std::pair<QChar, const char*>{ QChar(0x0A88), "iigujarati" },                   // Character 'ઈ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A08), "iigurmukhi" },                   // Character 'ਈ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A40), "iimatragurmukhi" },              // Character 'ੀ' Mark
    std::pair<QChar, const char*>{ QChar(0x020B), "iinvertedbreve" },               // Character 'ȋ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0439), "iishortcyrillic" },              // Character 'й' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x09C0), "iivowelsignbengali" },           // Character 'ী' Mark
    std::pair<QChar, const char*>{ QChar(0x0940), "iivowelsigndeva" },              // Character 'ी' Mark
    std::pair<QChar, const char*>{ QChar(0x0AC0), "iivowelsigngujarati" },          // Character 'ી' Mark
    std::pair<QChar, const char*>{ QChar(0x0133), "ij" },                           // Character 'ĳ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x30A4), "ikatakana" },                    // Character 'イ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF72), "ikatakanahalfwidth" },           // Character 'ｲ' Letter
    std::pair<QChar, const char*>{ QChar(0x3163), "ikorean" },                      // Character 'ㅣ' Letter
    std::pair<QChar, const char*>{ QChar(0x02DC), "ilde" },                         // Character '˜' Symbol
    std::pair<QChar, const char*>{ QChar(0x05AC), "iluyhebrew" },                   // Character '֬' Mark
    std::pair<QChar, const char*>{ QChar(0x012B), "imacron" },                      // Character 'ī' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x04E3), "imacroncyrillic" },              // Character 'ӣ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x2253), "imageorapproximatelyequal" },    // Character '≓' Symbol
    std::pair<QChar, const char*>{ QChar(0x0A3F), "imatragurmukhi" },               // Character 'ਿ' Mark
    std::pair<QChar, const char*>{ QChar(0xFF49), "imonospace" },                   // Character 'ｉ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x2206), "increment" },                    // Character '∆' Symbol
    std::pair<QChar, const char*>{ QChar(0x221E), "infinity" },                     // Character '∞' Symbol
    std::pair<QChar, const char*>{ QChar(0x056B), "iniarmenian" },                  // Character 'ի' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x222B), "integral" },                     // Character '∫' Symbol
    std::pair<QChar, const char*>{ QChar(0x2321), "integralbottom" },               // Character '⌡' Symbol
    std::pair<QChar, const char*>{ QChar(0x2321), "integralbt" },                   // Character '⌡' Symbol
    std::pair<QChar, const char*>{ QChar(0xF8F5), "integralex" },                   //
    std::pair<QChar, const char*>{ QChar(0x2320), "integraltop" },                  // Character '⌠' Symbol
    std::pair<QChar, const char*>{ QChar(0x2320), "integraltp" },                   // Character '⌠' Symbol
    std::pair<QChar, const char*>{ QChar(0x2229), "intersection" },                 // Character '∩' Symbol
    std::pair<QChar, const char*>{ QChar(0x3305), "intisquare" },                   // Character '㌅' Symbol
    std::pair<QChar, const char*>{ QChar(0x25D8), "invbullet" },                    // Character '◘' Symbol
    std::pair<QChar, const char*>{ QChar(0x25D9), "invcircle" },                    // Character '◙' Symbol
    std::pair<QChar, const char*>{ QChar(0x263B), "invsmileface" },                 // Character '☻' Symbol
    std::pair<QChar, const char*>{ QChar(0x0451), "iocyrillic" },                   // Character 'ё' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x012F), "iogonek" },                      // Character 'į' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x03B9), "iota" },                         // Character 'ι' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x03CA), "iotadieresis" },                 // Character 'ϊ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0390), "iotadieresistonos" },            // Character 'ΐ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0269), "iotalatin" },                    // Character 'ɩ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x03AF), "iotatonos" },                    // Character 'ί' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x24A4), "iparen" },                       // Character '⒤' Symbol
    std::pair<QChar, const char*>{ QChar(0x0A72), "irigurmukhi" },                  // Character 'ੲ' Letter
    std::pair<QChar, const char*>{ QChar(0x3043), "ismallhiragana" },               // Character 'ぃ' Letter
    std::pair<QChar, const char*>{ QChar(0x30A3), "ismallkatakana" },               // Character 'ィ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF68), "ismallkatakanahalfwidth" },      // Character 'ｨ' Letter
    std::pair<QChar, const char*>{ QChar(0x09FA), "issharbengali" },                // Character '৺' Symbol
    std::pair<QChar, const char*>{ QChar(0x0268), "istroke" },                      // Character 'ɨ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0xF6ED), "isuperior" },                    //
    std::pair<QChar, const char*>{ QChar(0x309D), "iterationhiragana" },            // Character 'ゝ' Letter
    std::pair<QChar, const char*>{ QChar(0x30FD), "iterationkatakana" },            // Character 'ヽ' Letter
    std::pair<QChar, const char*>{ QChar(0x0129), "itilde" },                       // Character 'ĩ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E2D), "itildebelow" },                  // Character 'ḭ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3129), "iubopomofo" },                   // Character 'ㄩ' Letter
    std::pair<QChar, const char*>{ QChar(0x044E), "iucyrillic" },                   // Character 'ю' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x09BF), "ivowelsignbengali" },            // Character 'ি' Mark
    std::pair<QChar, const char*>{ QChar(0x093F), "ivowelsigndeva" },               // Character 'ि' Mark
    std::pair<QChar, const char*>{ QChar(0x0ABF), "ivowelsigngujarati" },           // Character 'િ' Mark
    std::pair<QChar, const char*>{ QChar(0x0475), "izhitsacyrillic" },              // Character 'ѵ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0477), "izhitsadblgravecyrillic" },      // Character 'ѷ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x006A), "j" },                            // Character 'j' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0571), "jaarmenian" },                   // Character 'ձ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x099C), "jabengali" },                    // Character 'জ' Letter
    std::pair<QChar, const char*>{ QChar(0x091C), "jadeva" },                       // Character 'ज' Letter
    std::pair<QChar, const char*>{ QChar(0x0A9C), "jagujarati" },                   // Character 'જ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A1C), "jagurmukhi" },                   // Character 'ਜ' Letter
    std::pair<QChar, const char*>{ QChar(0x3110), "jbopomofo" },                    // Character 'ㄐ' Letter
    std::pair<QChar, const char*>{ QChar(0x01F0), "jcaron" },                       // Character 'ǰ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x24D9), "jcircle" },                      // Character 'ⓙ' Symbol
    std::pair<QChar, const char*>{ QChar(0x0135), "jcircumflex" },                  // Character 'ĵ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x029D), "jcrossedtail" },                 // Character 'ʝ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x025F), "jdotlessstroke" },               // Character 'ɟ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0458), "jecyrillic" },                   // Character 'ј' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x062C), "jeemarabic" },                   // Character 'ج' Letter
    std::pair<QChar, const char*>{ QChar(0xFE9E), "jeemfinalarabic" },              // Character 'ﺞ' Letter
    std::pair<QChar, const char*>{ QChar(0xFE9F), "jeeminitialarabic" },            // Character 'ﺟ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEA0), "jeemmedialarabic" },             // Character 'ﺠ' Letter
    std::pair<QChar, const char*>{ QChar(0x0698), "jeharabic" },                    // Character 'ژ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB8B), "jehfinalarabic" },               // Character 'ﮋ' Letter
    std::pair<QChar, const char*>{ QChar(0x099D), "jhabengali" },                   // Character 'ঝ' Letter
    std::pair<QChar, const char*>{ QChar(0x091D), "jhadeva" },                      // Character 'झ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A9D), "jhagujarati" },                  // Character 'ઝ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A1D), "jhagurmukhi" },                  // Character 'ਝ' Letter
    std::pair<QChar, const char*>{ QChar(0x057B), "jheharmenian" },                 // Character 'ջ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3004), "jis" },                          // Character '〄' Symbol
    std::pair<QChar, const char*>{ QChar(0xFF4A), "jmonospace" },                   // Character 'ｊ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x24A5), "jparen" },                       // Character '⒥' Symbol
    std::pair<QChar, const char*>{ QChar(0x02B2), "jsuperior" },                    // Character 'ʲ' Letter
    std::pair<QChar, const char*>{ QChar(0x006B), "k" },                            // Character 'k' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x04A1), "kabashkircyrillic" },            // Character 'ҡ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0995), "kabengali" },                    // Character 'ক' Letter
    std::pair<QChar, const char*>{ QChar(0x1E31), "kacute" },                       // Character 'ḱ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x043A), "kacyrillic" },                   // Character 'к' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x049B), "kadescendercyrillic" },          // Character 'қ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0915), "kadeva" },                       // Character 'क' Letter
    std::pair<QChar, const char*>{ QChar(0x05DB), "kaf" },                          // Character 'כ' Letter
    std::pair<QChar, const char*>{ QChar(0x0643), "kafarabic" },                    // Character 'ك' Letter
    std::pair<QChar, const char*>{ QChar(0xFB3B), "kafdagesh" },                    // Character 'כּ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB3B), "kafdageshhebrew" },              // Character 'כּ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEDA), "kaffinalarabic" },               // Character 'ﻚ' Letter
    std::pair<QChar, const char*>{ QChar(0x05DB), "kafhebrew" },                    // Character 'כ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEDB), "kafinitialarabic" },             // Character 'ﻛ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEDC), "kafmedialarabic" },              // Character 'ﻜ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB4D), "kafrafehebrew" },                // Character 'כֿ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A95), "kagujarati" },                   // Character 'ક' Letter
    std::pair<QChar, const char*>{ QChar(0x0A15), "kagurmukhi" },                   // Character 'ਕ' Letter
    std::pair<QChar, const char*>{ QChar(0x304B), "kahiragana" },                   // Character 'か' Letter
    std::pair<QChar, const char*>{ QChar(0x04C4), "kahookcyrillic" },               // Character 'ӄ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x30AB), "kakatakana" },                   // Character 'カ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF76), "kakatakanahalfwidth" },          // Character 'ｶ' Letter
    std::pair<QChar, const char*>{ QChar(0x03BA), "kappa" },                        // Character 'κ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x03F0), "kappasymbolgreek" },             // Character 'ϰ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3171), "kapyeounmieumkorean" },          // Character 'ㅱ' Letter
    std::pair<QChar, const char*>{ QChar(0x3184), "kapyeounphieuphkorean" },        // Character 'ㆄ' Letter
    std::pair<QChar, const char*>{ QChar(0x3178), "kapyeounpieupkorean" },          // Character 'ㅸ' Letter
    std::pair<QChar, const char*>{ QChar(0x3179), "kapyeounssangpieupkorean" },     // Character 'ㅹ' Letter
    std::pair<QChar, const char*>{ QChar(0x330D), "karoriisquare" },                // Character '㌍' Symbol
    std::pair<QChar, const char*>{ QChar(0x0640), "kashidaautoarabic" },            // Character 'ـ' Letter
    std::pair<QChar, const char*>{ QChar(0x0640), "kashidaautonosidebearingarabic" },// Character 'ـ' Letter
    std::pair<QChar, const char*>{ QChar(0x30F5), "kasmallkatakana" },              // Character 'ヵ' Letter
    std::pair<QChar, const char*>{ QChar(0x3384), "kasquare" },                     // Character '㎄' Symbol
    std::pair<QChar, const char*>{ QChar(0x0650), "kasraarabic" },                  // Character 'ِ' Mark
    std::pair<QChar, const char*>{ QChar(0x064D), "kasratanarabic" },               // Character 'ٍ' Mark
    std::pair<QChar, const char*>{ QChar(0x049F), "kastrokecyrillic" },             // Character 'ҟ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0xFF70), "katahiraprolongmarkhalfwidth" }, // Character 'ｰ' Letter
    std::pair<QChar, const char*>{ QChar(0x049D), "kaverticalstrokecyrillic" },     // Character 'ҝ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x310E), "kbopomofo" },                    // Character 'ㄎ' Letter
    std::pair<QChar, const char*>{ QChar(0x3389), "kcalsquare" },                   // Character '㎉' Symbol
    std::pair<QChar, const char*>{ QChar(0x01E9), "kcaron" },                       // Character 'ǩ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0137), "kcedilla" },                     // Character 'ķ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x24DA), "kcircle" },                      // Character 'ⓚ' Symbol
    std::pair<QChar, const char*>{ QChar(0x0137), "kcommaaccent" },                 // Character 'ķ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E33), "kdotbelow" },                    // Character 'ḳ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0584), "keharmenian" },                  // Character 'ք' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3051), "kehiragana" },                   // Character 'け' Letter
    std::pair<QChar, const char*>{ QChar(0x30B1), "kekatakana" },                   // Character 'ケ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF79), "kekatakanahalfwidth" },          // Character 'ｹ' Letter
    std::pair<QChar, const char*>{ QChar(0x056F), "kenarmenian" },                  // Character 'կ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x30F6), "kesmallkatakana" },              // Character 'ヶ' Letter
    std::pair<QChar, const char*>{ QChar(0x0138), "kgreenlandic" },                 // Character 'ĸ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0996), "khabengali" },                   // Character 'খ' Letter
    std::pair<QChar, const char*>{ QChar(0x0445), "khacyrillic" },                  // Character 'х' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0916), "khadeva" },                      // Character 'ख' Letter
    std::pair<QChar, const char*>{ QChar(0x0A96), "khagujarati" },                  // Character 'ખ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A16), "khagurmukhi" },                  // Character 'ਖ' Letter
    std::pair<QChar, const char*>{ QChar(0x062E), "khaharabic" },                   // Character 'خ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEA6), "khahfinalarabic" },              // Character 'ﺦ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEA7), "khahinitialarabic" },            // Character 'ﺧ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEA8), "khahmedialarabic" },             // Character 'ﺨ' Letter
    std::pair<QChar, const char*>{ QChar(0x03E7), "kheicoptic" },                   // Character 'ϧ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0959), "khhadeva" },                     // Character 'ख़' Letter
    std::pair<QChar, const char*>{ QChar(0x0A59), "khhagurmukhi" },                 // Character 'ਖ਼' Letter
    std::pair<QChar, const char*>{ QChar(0x3278), "khieukhacirclekorean" },         // Character '㉸' Symbol
    std::pair<QChar, const char*>{ QChar(0x3218), "khieukhaparenkorean" },          // Character '㈘' Symbol
    std::pair<QChar, const char*>{ QChar(0x326A), "khieukhcirclekorean" },          // Character '㉪' Symbol
    std::pair<QChar, const char*>{ QChar(0x314B), "khieukhkorean" },                // Character 'ㅋ' Letter
    std::pair<QChar, const char*>{ QChar(0x320A), "khieukhparenkorean" },           // Character '㈊' Symbol
    std::pair<QChar, const char*>{ QChar(0x0E02), "khokhaithai" },                  // Character 'ข' Letter
    std::pair<QChar, const char*>{ QChar(0x0E05), "khokhonthai" },                  // Character 'ฅ' Letter
    std::pair<QChar, const char*>{ QChar(0x0E03), "khokhuatthai" },                 // Character 'ฃ' Letter
    std::pair<QChar, const char*>{ QChar(0x0E04), "khokhwaithai" },                 // Character 'ค' Letter
    std::pair<QChar, const char*>{ QChar(0x0E5B), "khomutthai" },                   // Character '๛' Punctuation
    std::pair<QChar, const char*>{ QChar(0x0199), "khook" },                        // Character 'ƙ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0E06), "khorakhangthai" },               // Character 'ฆ' Letter
    std::pair<QChar, const char*>{ QChar(0x3391), "khzsquare" },                    // Character '㎑' Symbol
    std::pair<QChar, const char*>{ QChar(0x304D), "kihiragana" },                   // Character 'き' Letter
    std::pair<QChar, const char*>{ QChar(0x30AD), "kikatakana" },                   // Character 'キ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF77), "kikatakanahalfwidth" },          // Character 'ｷ' Letter
    std::pair<QChar, const char*>{ QChar(0x3315), "kiroguramusquare" },             // Character '㌕' Symbol
    std::pair<QChar, const char*>{ QChar(0x3316), "kiromeetorusquare" },            // Character '㌖' Symbol
    std::pair<QChar, const char*>{ QChar(0x3314), "kirosquare" },                   // Character '㌔' Symbol
    std::pair<QChar, const char*>{ QChar(0x326E), "kiyeokacirclekorean" },          // Character '㉮' Symbol
    std::pair<QChar, const char*>{ QChar(0x320E), "kiyeokaparenkorean" },           // Character '㈎' Symbol
    std::pair<QChar, const char*>{ QChar(0x3260), "kiyeokcirclekorean" },           // Character '㉠' Symbol
    std::pair<QChar, const char*>{ QChar(0x3131), "kiyeokkorean" },                 // Character 'ㄱ' Letter
    std::pair<QChar, const char*>{ QChar(0x3200), "kiyeokparenkorean" },            // Character '㈀' Symbol
    std::pair<QChar, const char*>{ QChar(0x3133), "kiyeoksioskorean" },             // Character 'ㄳ' Letter
    std::pair<QChar, const char*>{ QChar(0x045C), "kjecyrillic" },                  // Character 'ќ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E35), "klinebelow" },                   // Character 'ḵ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3398), "klsquare" },                     // Character '㎘' Symbol
    std::pair<QChar, const char*>{ QChar(0x33A6), "kmcubedsquare" },                // Character '㎦' Symbol
    std::pair<QChar, const char*>{ QChar(0xFF4B), "kmonospace" },                   // Character 'ｋ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x33A2), "kmsquaredsquare" },              // Character '㎢' Symbol
    std::pair<QChar, const char*>{ QChar(0x3053), "kohiragana" },                   // Character 'こ' Letter
    std::pair<QChar, const char*>{ QChar(0x33C0), "kohmsquare" },                   // Character '㏀' Symbol
    std::pair<QChar, const char*>{ QChar(0x0E01), "kokaithai" },                    // Character 'ก' Letter
    std::pair<QChar, const char*>{ QChar(0x30B3), "kokatakana" },                   // Character 'コ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF7A), "kokatakanahalfwidth" },          // Character 'ｺ' Letter
    std::pair<QChar, const char*>{ QChar(0x331E), "kooposquare" },                  // Character '㌞' Symbol
    std::pair<QChar, const char*>{ QChar(0x0481), "koppacyrillic" },                // Character 'ҁ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x327F), "koreanstandardsymbol" },         // Character '㉿' Symbol
    std::pair<QChar, const char*>{ QChar(0x0343), "koroniscmb" },                   // Character '̓' Mark
    std::pair<QChar, const char*>{ QChar(0x24A6), "kparen" },                       // Character '⒦' Symbol
    std::pair<QChar, const char*>{ QChar(0x33AA), "kpasquare" },                    // Character '㎪' Symbol
    std::pair<QChar, const char*>{ QChar(0x046F), "ksicyrillic" },                  // Character 'ѯ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x33CF), "ktsquare" },                     // Character '㏏' Symbol
    std::pair<QChar, const char*>{ QChar(0x029E), "kturned" },                      // Character 'ʞ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x304F), "kuhiragana" },                   // Character 'く' Letter
    std::pair<QChar, const char*>{ QChar(0x30AF), "kukatakana" },                   // Character 'ク' Letter
    std::pair<QChar, const char*>{ QChar(0xFF78), "kukatakanahalfwidth" },          // Character 'ｸ' Letter
    std::pair<QChar, const char*>{ QChar(0x33B8), "kvsquare" },                     // Character '㎸' Symbol
    std::pair<QChar, const char*>{ QChar(0x33BE), "kwsquare" },                     // Character '㎾' Symbol
    std::pair<QChar, const char*>{ QChar(0x006C), "l" },                            // Character 'l' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x09B2), "labengali" },                    // Character 'ল' Letter
    std::pair<QChar, const char*>{ QChar(0x013A), "lacute" },                       // Character 'ĺ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0932), "ladeva" },                       // Character 'ल' Letter
    std::pair<QChar, const char*>{ QChar(0x0AB2), "lagujarati" },                   // Character 'લ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A32), "lagurmukhi" },                   // Character 'ਲ' Letter
    std::pair<QChar, const char*>{ QChar(0x0E45), "lakkhangyaothai" },              // Character 'ๅ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEFC), "lamaleffinalarabic" },           // Character 'ﻼ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEF8), "lamalefhamzaabovefinalarabic" }, // Character 'ﻸ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEF7), "lamalefhamzaaboveisolatedarabic" },// Character 'ﻷ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEFA), "lamalefhamzabelowfinalarabic" }, // Character 'ﻺ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEF9), "lamalefhamzabelowisolatedarabic" },// Character 'ﻹ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEFB), "lamalefisolatedarabic" },        // Character 'ﻻ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEF6), "lamalefmaddaabovefinalarabic" }, // Character 'ﻶ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEF5), "lamalefmaddaaboveisolatedarabic" },// Character 'ﻵ' Letter
    std::pair<QChar, const char*>{ QChar(0x0644), "lamarabic" },                    // Character 'ل' Letter
    std::pair<QChar, const char*>{ QChar(0x03BB), "lambda" },                       // Character 'λ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x019B), "lambdastroke" },                 // Character 'ƛ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x05DC), "lamed" },                        // Character 'ל' Letter
    std::pair<QChar, const char*>{ QChar(0xFB3C), "lameddagesh" },                  // Character 'לּ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB3C), "lameddageshhebrew" },            // Character 'לּ' Letter
    std::pair<QChar, const char*>{ QChar(0x05DC), "lamedhebrew" },                  // Character 'ל' Letter
    std::pair<QChar, const char*>{ QChar(0x05B9), "lamedholam" },                   // Character 'ֹ' Mark
    std::pair<QChar, const char*>{ QChar(0x05BC), "lamedholamdagesh" },             // Character 'ּ' Mark
    std::pair<QChar, const char*>{ QChar(0x05BC), "lamedholamdageshhebrew" },       // Character 'ּ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B9), "lamedholamhebrew" },             // Character 'ֹ' Mark
    std::pair<QChar, const char*>{ QChar(0xFEDE), "lamfinalarabic" },               // Character 'ﻞ' Letter
    std::pair<QChar, const char*>{ QChar(0xFCCA), "lamhahinitialarabic" },          // Character 'ﳊ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEDF), "laminitialarabic" },             // Character 'ﻟ' Letter
    std::pair<QChar, const char*>{ QChar(0xFCC9), "lamjeeminitialarabic" },         // Character 'ﳉ' Letter
    std::pair<QChar, const char*>{ QChar(0xFCCB), "lamkhahinitialarabic" },         // Character 'ﳋ' Letter
    std::pair<QChar, const char*>{ QChar(0xFDF2), "lamlamhehisolatedarabic" },      // Character 'ﷲ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEE0), "lammedialarabic" },              // Character 'ﻠ' Letter
    std::pair<QChar, const char*>{ QChar(0xFD88), "lammeemhahinitialarabic" },      // Character 'ﶈ' Letter
    std::pair<QChar, const char*>{ QChar(0xFCCC), "lammeeminitialarabic" },         // Character 'ﳌ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEA0), "lammeemjeeminitialarabic" },     // Character 'ﺠ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEA8), "lammeemkhahinitialarabic" },     // Character 'ﺨ' Letter
    std::pair<QChar, const char*>{ QChar(0x25EF), "largecircle" },                  // Character '◯' Symbol
    std::pair<QChar, const char*>{ QChar(0x019A), "lbar" },                         // Character 'ƚ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x026C), "lbelt" },                        // Character 'ɬ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x310C), "lbopomofo" },                    // Character 'ㄌ' Letter
    std::pair<QChar, const char*>{ QChar(0x013E), "lcaron" },                       // Character 'ľ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x013C), "lcedilla" },                     // Character 'ļ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x24DB), "lcircle" },                      // Character 'ⓛ' Symbol
    std::pair<QChar, const char*>{ QChar(0x1E3D), "lcircumflexbelow" },             // Character 'ḽ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x013C), "lcommaaccent" },                 // Character 'ļ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0140), "ldot" },                         // Character 'ŀ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0140), "ldotaccent" },                   // Character 'ŀ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E37), "ldotbelow" },                    // Character 'ḷ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E39), "ldotbelowmacron" },              // Character 'ḹ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x031A), "leftangleabovecmb" },            // Character '̚' Mark
    std::pair<QChar, const char*>{ QChar(0x0318), "lefttackbelowcmb" },             // Character '̘' Mark
    std::pair<QChar, const char*>{ QChar(0x003C), "less" },                         // Character '<' Symbol
    std::pair<QChar, const char*>{ QChar(0x2264), "lessequal" },                    // Character '≤' Symbol
    std::pair<QChar, const char*>{ QChar(0x22DA), "lessequalorgreater" },           // Character '⋚' Symbol
    std::pair<QChar, const char*>{ QChar(0xFF1C), "lessmonospace" },                // Character '＜' Symbol
    std::pair<QChar, const char*>{ QChar(0x2272), "lessorequivalent" },             // Character '≲' Symbol
    std::pair<QChar, const char*>{ QChar(0x2276), "lessorgreater" },                // Character '≶' Symbol
    std::pair<QChar, const char*>{ QChar(0x2266), "lessoverequal" },                // Character '≦' Symbol
    std::pair<QChar, const char*>{ QChar(0xFE64), "lesssmall" },                    // Character '﹤' Symbol
    std::pair<QChar, const char*>{ QChar(0x026E), "lezh" },                         // Character 'ɮ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x258C), "lfblock" },                      // Character '▌' Symbol
    std::pair<QChar, const char*>{ QChar(0x026D), "lhookretroflex" },               // Character 'ɭ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x20A4), "lira" },                         // Character '₤' Symbol
    std::pair<QChar, const char*>{ QChar(0x056C), "liwnarmenian" },                 // Character 'լ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x01C9), "lj" },                           // Character 'ǉ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0459), "ljecyrillic" },                  // Character 'љ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0xF6C0), "ll" },                           //
    std::pair<QChar, const char*>{ QChar(0x0933), "lladeva" },                      // Character 'ळ' Letter
    std::pair<QChar, const char*>{ QChar(0x0AB3), "llagujarati" },                  // Character 'ળ' Letter
    std::pair<QChar, const char*>{ QChar(0x1E3B), "llinebelow" },                   // Character 'ḻ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0934), "llladeva" },                     // Character 'ऴ' Letter
    std::pair<QChar, const char*>{ QChar(0x09E1), "llvocalicbengali" },             // Character 'ৡ' Letter
    std::pair<QChar, const char*>{ QChar(0x0961), "llvocalicdeva" },                // Character 'ॡ' Letter
    std::pair<QChar, const char*>{ QChar(0x09E3), "llvocalicvowelsignbengali" },    // Character 'ৣ' Mark
    std::pair<QChar, const char*>{ QChar(0x0963), "llvocalicvowelsigndeva" },       // Character 'ॣ' Mark
    std::pair<QChar, const char*>{ QChar(0x026B), "lmiddletilde" },                 // Character 'ɫ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0xFF4C), "lmonospace" },                   // Character 'ｌ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x33D0), "lmsquare" },                     // Character '㏐' Symbol
    std::pair<QChar, const char*>{ QChar(0x0E2C), "lochulathai" },                  // Character 'ฬ' Letter
    std::pair<QChar, const char*>{ QChar(0x2227), "logicaland" },                   // Character '∧' Symbol
    std::pair<QChar, const char*>{ QChar(0x00AC), "logicalnot" },                   // Character '¬' Symbol
    std::pair<QChar, const char*>{ QChar(0x2310), "logicalnotreversed" },           // Character '⌐' Symbol
    std::pair<QChar, const char*>{ QChar(0x2228), "logicalor" },                    // Character '∨' Symbol
    std::pair<QChar, const char*>{ QChar(0x0E25), "lolingthai" },                   // Character 'ล' Letter
    std::pair<QChar, const char*>{ QChar(0x017F), "longs" },                        // Character 'ſ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0xFE4E), "lowlinecenterline" },            // Character '﹎' Punctuation
    std::pair<QChar, const char*>{ QChar(0x0332), "lowlinecmb" },                   // Character '̲' Mark
    std::pair<QChar, const char*>{ QChar(0xFE4D), "lowlinedashed" },                // Character '﹍' Punctuation
    std::pair<QChar, const char*>{ QChar(0x25CA), "lozenge" },                      // Character '◊' Symbol
    std::pair<QChar, const char*>{ QChar(0x24A7), "lparen" },                       // Character '⒧' Symbol
    std::pair<QChar, const char*>{ QChar(0x0142), "lslash" },                       // Character 'ł' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x2113), "lsquare" },                      // Character 'ℓ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0xF6EE), "lsuperior" },                    //
    std::pair<QChar, const char*>{ QChar(0x2591), "ltshade" },                      // Character '░' Symbol
    std::pair<QChar, const char*>{ QChar(0x0E26), "luthai" },                       // Character 'ฦ' Letter
    std::pair<QChar, const char*>{ QChar(0x098C), "lvocalicbengali" },              // Character 'ঌ' Letter
    std::pair<QChar, const char*>{ QChar(0x090C), "lvocalicdeva" },                 // Character 'ऌ' Letter
    std::pair<QChar, const char*>{ QChar(0x09E2), "lvocalicvowelsignbengali" },     // Character 'ৢ' Mark
    std::pair<QChar, const char*>{ QChar(0x0962), "lvocalicvowelsigndeva" },        // Character 'ॢ' Mark
    std::pair<QChar, const char*>{ QChar(0x33D3), "lxsquare" },                     // Character '㏓' Symbol
    std::pair<QChar, const char*>{ QChar(0x006D), "m" },                            // Character 'm' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x09AE), "mabengali" },                    // Character 'ম' Letter
    std::pair<QChar, const char*>{ QChar(0x00AF), "macron" },                       // Character '¯' Symbol
    std::pair<QChar, const char*>{ QChar(0x0331), "macronbelowcmb" },               // Character '̱' Mark
    std::pair<QChar, const char*>{ QChar(0x0304), "macroncmb" },                    // Character '̄' Mark
    std::pair<QChar, const char*>{ QChar(0x02CD), "macronlowmod" },                 // Character 'ˍ' Letter
    std::pair<QChar, const char*>{ QChar(0xFFE3), "macronmonospace" },              // Character '￣' Symbol
    std::pair<QChar, const char*>{ QChar(0x1E3F), "macute" },                       // Character 'ḿ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x092E), "madeva" },                       // Character 'म' Letter
    std::pair<QChar, const char*>{ QChar(0x0AAE), "magujarati" },                   // Character 'મ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A2E), "magurmukhi" },                   // Character 'ਮ' Letter
    std::pair<QChar, const char*>{ QChar(0x05A4), "mahapakhhebrew" },               // Character '֤' Mark
    std::pair<QChar, const char*>{ QChar(0x05A4), "mahapakhlefthebrew" },           // Character '֤' Mark
    std::pair<QChar, const char*>{ QChar(0x307E), "mahiragana" },                   // Character 'ま' Letter
    std::pair<QChar, const char*>{ QChar(0xF895), "maichattawalowleftthai" },       //
    std::pair<QChar, const char*>{ QChar(0xF894), "maichattawalowrightthai" },      //
    std::pair<QChar, const char*>{ QChar(0x0E4B), "maichattawathai" },              // Character '๋' Mark
    std::pair<QChar, const char*>{ QChar(0xF893), "maichattawaupperleftthai" },     //
    std::pair<QChar, const char*>{ QChar(0xF88C), "maieklowleftthai" },             //
    std::pair<QChar, const char*>{ QChar(0xF88B), "maieklowrightthai" },            //
    std::pair<QChar, const char*>{ QChar(0x0E48), "maiekthai" },                    // Character '่' Mark
    std::pair<QChar, const char*>{ QChar(0xF88A), "maiekupperleftthai" },           //
    std::pair<QChar, const char*>{ QChar(0xF884), "maihanakatleftthai" },           //
    std::pair<QChar, const char*>{ QChar(0x0E31), "maihanakatthai" },               // Character 'ั' Mark
    std::pair<QChar, const char*>{ QChar(0xF889), "maitaikhuleftthai" },            //
    std::pair<QChar, const char*>{ QChar(0x0E47), "maitaikhuthai" },                // Character '็' Mark
    std::pair<QChar, const char*>{ QChar(0xF88F), "maitholowleftthai" },            //
    std::pair<QChar, const char*>{ QChar(0xF88E), "maitholowrightthai" },           //
    std::pair<QChar, const char*>{ QChar(0x0E49), "maithothai" },                   // Character '้' Mark
    std::pair<QChar, const char*>{ QChar(0xF88D), "maithoupperleftthai" },          //
    std::pair<QChar, const char*>{ QChar(0xF892), "maitrilowleftthai" },            //
    std::pair<QChar, const char*>{ QChar(0xF891), "maitrilowrightthai" },           //
    std::pair<QChar, const char*>{ QChar(0x0E4A), "maitrithai" },                   // Character '๊' Mark
    std::pair<QChar, const char*>{ QChar(0xF890), "maitriupperleftthai" },          //
    std::pair<QChar, const char*>{ QChar(0x0E46), "maiyamokthai" },                 // Character 'ๆ' Letter
    std::pair<QChar, const char*>{ QChar(0x30DE), "makatakana" },                   // Character 'マ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF8F), "makatakanahalfwidth" },          // Character 'ﾏ' Letter
    std::pair<QChar, const char*>{ QChar(0x2642), "male" },                         // Character '♂' Symbol
    std::pair<QChar, const char*>{ QChar(0x3347), "mansyonsquare" },                // Character '㍇' Symbol
    std::pair<QChar, const char*>{ QChar(0x05BE), "maqafhebrew" },                  // Character '־' Punctuation
    std::pair<QChar, const char*>{ QChar(0x2642), "mars" },                         // Character '♂' Symbol
    std::pair<QChar, const char*>{ QChar(0x05AF), "masoracirclehebrew" },           // Character '֯' Mark
    std::pair<QChar, const char*>{ QChar(0x3383), "masquare" },                     // Character '㎃' Symbol
    std::pair<QChar, const char*>{ QChar(0x3107), "mbopomofo" },                    // Character 'ㄇ' Letter
    std::pair<QChar, const char*>{ QChar(0x33D4), "mbsquare" },                     // Character '㏔' Symbol
    std::pair<QChar, const char*>{ QChar(0x24DC), "mcircle" },                      // Character 'ⓜ' Symbol
    std::pair<QChar, const char*>{ QChar(0x33A5), "mcubedsquare" },                 // Character '㎥' Symbol
    std::pair<QChar, const char*>{ QChar(0x1E41), "mdotaccent" },                   // Character 'ṁ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E43), "mdotbelow" },                    // Character 'ṃ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0645), "meemarabic" },                   // Character 'م' Letter
    std::pair<QChar, const char*>{ QChar(0xFEE2), "meemfinalarabic" },              // Character 'ﻢ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEE3), "meeminitialarabic" },            // Character 'ﻣ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEE4), "meemmedialarabic" },             // Character 'ﻤ' Letter
    std::pair<QChar, const char*>{ QChar(0xFCD1), "meemmeeminitialarabic" },        // Character 'ﳑ' Letter
    std::pair<QChar, const char*>{ QChar(0xFC48), "meemmeemisolatedarabic" },       // Character 'ﱈ' Letter
    std::pair<QChar, const char*>{ QChar(0x334D), "meetorusquare" },                // Character '㍍' Symbol
    std::pair<QChar, const char*>{ QChar(0x3081), "mehiragana" },                   // Character 'め' Letter
    std::pair<QChar, const char*>{ QChar(0x337E), "meizierasquare" },               // Character '㍾' Symbol
    std::pair<QChar, const char*>{ QChar(0x30E1), "mekatakana" },                   // Character 'メ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF92), "mekatakanahalfwidth" },          // Character 'ﾒ' Letter
    std::pair<QChar, const char*>{ QChar(0x05DE), "mem" },                          // Character 'מ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB3E), "memdagesh" },                    // Character 'מּ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB3E), "memdageshhebrew" },              // Character 'מּ' Letter
    std::pair<QChar, const char*>{ QChar(0x05DE), "memhebrew" },                    // Character 'מ' Letter
    std::pair<QChar, const char*>{ QChar(0x0574), "menarmenian" },                  // Character 'մ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x05A5), "merkhahebrew" },                 // Character '֥' Mark
    std::pair<QChar, const char*>{ QChar(0x05A6), "merkhakefulahebrew" },           // Character '֦' Mark
    std::pair<QChar, const char*>{ QChar(0x05A6), "merkhakefulalefthebrew" },       // Character '֦' Mark
    std::pair<QChar, const char*>{ QChar(0x05A5), "merkhalefthebrew" },             // Character '֥' Mark
    std::pair<QChar, const char*>{ QChar(0x0271), "mhook" },                        // Character 'ɱ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3392), "mhzsquare" },                    // Character '㎒' Symbol
    std::pair<QChar, const char*>{ QChar(0xFF65), "middledotkatakanahalfwidth" },   // Character '･' Punctuation
    std::pair<QChar, const char*>{ QChar(0x00B7), "middot" },                       // Character '·' Punctuation
    std::pair<QChar, const char*>{ QChar(0x3272), "mieumacirclekorean" },           // Character '㉲' Symbol
    std::pair<QChar, const char*>{ QChar(0x3212), "mieumaparenkorean" },            // Character '㈒' Symbol
    std::pair<QChar, const char*>{ QChar(0x3264), "mieumcirclekorean" },            // Character '㉤' Symbol
    std::pair<QChar, const char*>{ QChar(0x3141), "mieumkorean" },                  // Character 'ㅁ' Letter
    std::pair<QChar, const char*>{ QChar(0x3170), "mieumpansioskorean" },           // Character 'ㅰ' Letter
    std::pair<QChar, const char*>{ QChar(0x3204), "mieumparenkorean" },             // Character '㈄' Symbol
    std::pair<QChar, const char*>{ QChar(0x316E), "mieumpieupkorean" },             // Character 'ㅮ' Letter
    std::pair<QChar, const char*>{ QChar(0x316F), "mieumsioskorean" },              // Character 'ㅯ' Letter
    std::pair<QChar, const char*>{ QChar(0x307F), "mihiragana" },                   // Character 'み' Letter
    std::pair<QChar, const char*>{ QChar(0x30DF), "mikatakana" },                   // Character 'ミ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF90), "mikatakanahalfwidth" },          // Character 'ﾐ' Letter
    std::pair<QChar, const char*>{ QChar(0x2212), "minus" },                        // Character '−' Symbol
    std::pair<QChar, const char*>{ QChar(0x0320), "minusbelowcmb" },                // Character '̠' Mark
    std::pair<QChar, const char*>{ QChar(0x2296), "minuscircle" },                  // Character '⊖' Symbol
    std::pair<QChar, const char*>{ QChar(0x02D7), "minusmod" },                     // Character '˗' Symbol
    std::pair<QChar, const char*>{ QChar(0x2213), "minusplus" },                    // Character '∓' Symbol
    std::pair<QChar, const char*>{ QChar(0x2032), "minute" },                       // Character '′' Punctuation
    std::pair<QChar, const char*>{ QChar(0x334A), "miribaarusquare" },              // Character '㍊' Symbol
    std::pair<QChar, const char*>{ QChar(0x3349), "mirisquare" },                   // Character '㍉' Symbol
    std::pair<QChar, const char*>{ QChar(0x0270), "mlonglegturned" },               // Character 'ɰ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3396), "mlsquare" },                     // Character '㎖' Symbol
    std::pair<QChar, const char*>{ QChar(0x33A3), "mmcubedsquare" },                // Character '㎣' Symbol
    std::pair<QChar, const char*>{ QChar(0xFF4D), "mmonospace" },                   // Character 'ｍ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x339F), "mmsquaredsquare" },              // Character '㎟' Symbol
    std::pair<QChar, const char*>{ QChar(0x3082), "mohiragana" },                   // Character 'も' Letter
    std::pair<QChar, const char*>{ QChar(0x33C1), "mohmsquare" },                   // Character '㏁' Symbol
    std::pair<QChar, const char*>{ QChar(0x30E2), "mokatakana" },                   // Character 'モ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF93), "mokatakanahalfwidth" },          // Character 'ﾓ' Letter
    std::pair<QChar, const char*>{ QChar(0x33D6), "molsquare" },                    // Character '㏖' Symbol
    std::pair<QChar, const char*>{ QChar(0x0E21), "momathai" },                     // Character 'ม' Letter
    std::pair<QChar, const char*>{ QChar(0x33A7), "moverssquare" },                 // Character '㎧' Symbol
    std::pair<QChar, const char*>{ QChar(0x33A8), "moverssquaredsquare" },          // Character '㎨' Symbol
    std::pair<QChar, const char*>{ QChar(0x24A8), "mparen" },                       // Character '⒨' Symbol
    std::pair<QChar, const char*>{ QChar(0x33AB), "mpasquare" },                    // Character '㎫' Symbol
    std::pair<QChar, const char*>{ QChar(0x33B3), "mssquare" },                     // Character '㎳' Symbol
    std::pair<QChar, const char*>{ QChar(0xF6EF), "msuperior" },                    //
    std::pair<QChar, const char*>{ QChar(0x026F), "mturned" },                      // Character 'ɯ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x00B5), "mu" },                           // Character 'µ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x00B5), "mu1" },                          // Character 'µ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3382), "muasquare" },                    // Character '㎂' Symbol
    std::pair<QChar, const char*>{ QChar(0x226B), "muchgreater" },                  // Character '≫' Symbol
    std::pair<QChar, const char*>{ QChar(0x226A), "muchless" },                     // Character '≪' Symbol
    std::pair<QChar, const char*>{ QChar(0x338C), "mufsquare" },                    // Character '㎌' Symbol
    std::pair<QChar, const char*>{ QChar(0x03BC), "mugreek" },                      // Character 'μ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x338D), "mugsquare" },                    // Character '㎍' Symbol
    std::pair<QChar, const char*>{ QChar(0x3080), "muhiragana" },                   // Character 'む' Letter
    std::pair<QChar, const char*>{ QChar(0x30E0), "mukatakana" },                   // Character 'ム' Letter
    std::pair<QChar, const char*>{ QChar(0xFF91), "mukatakanahalfwidth" },          // Character 'ﾑ' Letter
    std::pair<QChar, const char*>{ QChar(0x3395), "mulsquare" },                    // Character '㎕' Symbol
    std::pair<QChar, const char*>{ QChar(0x00D7), "multiply" },                     // Character '×' Symbol
    std::pair<QChar, const char*>{ QChar(0x339B), "mumsquare" },                    // Character '㎛' Symbol
    std::pair<QChar, const char*>{ QChar(0x05A3), "munahhebrew" },                  // Character '֣' Mark
    std::pair<QChar, const char*>{ QChar(0x05A3), "munahlefthebrew" },              // Character '֣' Mark
    std::pair<QChar, const char*>{ QChar(0x266A), "musicalnote" },                  // Character '♪' Symbol
    std::pair<QChar, const char*>{ QChar(0x266B), "musicalnotedbl" },               // Character '♫' Symbol
    std::pair<QChar, const char*>{ QChar(0x266D), "musicflatsign" },                // Character '♭' Symbol
    std::pair<QChar, const char*>{ QChar(0x266F), "musicsharpsign" },               // Character '♯' Symbol
    std::pair<QChar, const char*>{ QChar(0x33B2), "mussquare" },                    // Character '㎲' Symbol
    std::pair<QChar, const char*>{ QChar(0x33B6), "muvsquare" },                    // Character '㎶' Symbol
    std::pair<QChar, const char*>{ QChar(0x33BC), "muwsquare" },                    // Character '㎼' Symbol
    std::pair<QChar, const char*>{ QChar(0x33B9), "mvmegasquare" },                 // Character '㎹' Symbol
    std::pair<QChar, const char*>{ QChar(0x33B7), "mvsquare" },                     // Character '㎷' Symbol
    std::pair<QChar, const char*>{ QChar(0x33BF), "mwmegasquare" },                 // Character '㎿' Symbol
    std::pair<QChar, const char*>{ QChar(0x33BD), "mwsquare" },                     // Character '㎽' Symbol
    std::pair<QChar, const char*>{ QChar(0x006E), "n" },                            // Character 'n' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x09A8), "nabengali" },                    // Character 'ন' Letter
    std::pair<QChar, const char*>{ QChar(0x2207), "nabla" },                        // Character '∇' Symbol
    std::pair<QChar, const char*>{ QChar(0x0144), "nacute" },                       // Character 'ń' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0928), "nadeva" },                       // Character 'न' Letter
    std::pair<QChar, const char*>{ QChar(0x0AA8), "nagujarati" },                   // Character 'ન' Letter
    std::pair<QChar, const char*>{ QChar(0x0A28), "nagurmukhi" },                   // Character 'ਨ' Letter
    std::pair<QChar, const char*>{ QChar(0x306A), "nahiragana" },                   // Character 'な' Letter
    std::pair<QChar, const char*>{ QChar(0x30CA), "nakatakana" },                   // Character 'ナ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF85), "nakatakanahalfwidth" },          // Character 'ﾅ' Letter
    std::pair<QChar, const char*>{ QChar(0x0149), "napostrophe" },                  // Character 'ŉ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3381), "nasquare" },                     // Character '㎁' Symbol
    std::pair<QChar, const char*>{ QChar(0x310B), "nbopomofo" },                    // Character 'ㄋ' Letter
    std::pair<QChar, const char*>{ QChar(0x00A0), "nbspace" },                      // Character ' ' Whitespace
    std::pair<QChar, const char*>{ QChar(0x0148), "ncaron" },                       // Character 'ň' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0146), "ncedilla" },                     // Character 'ņ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x24DD), "ncircle" },                      // Character 'ⓝ' Symbol
    std::pair<QChar, const char*>{ QChar(0x1E4B), "ncircumflexbelow" },             // Character 'ṋ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0146), "ncommaaccent" },                 // Character 'ņ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E45), "ndotaccent" },                   // Character 'ṅ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E47), "ndotbelow" },                    // Character 'ṇ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x306D), "nehiragana" },                   // Character 'ね' Letter
    std::pair<QChar, const char*>{ QChar(0x30CD), "nekatakana" },                   // Character 'ネ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF88), "nekatakanahalfwidth" },          // Character 'ﾈ' Letter
    std::pair<QChar, const char*>{ QChar(0x20AA), "newsheqelsign" },                // Character '₪' Symbol
    std::pair<QChar, const char*>{ QChar(0x338B), "nfsquare" },                     // Character '㎋' Symbol
    std::pair<QChar, const char*>{ QChar(0x0999), "ngabengali" },                   // Character 'ঙ' Letter
    std::pair<QChar, const char*>{ QChar(0x0919), "ngadeva" },                      // Character 'ङ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A99), "ngagujarati" },                  // Character 'ઙ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A19), "ngagurmukhi" },                  // Character 'ਙ' Letter
    std::pair<QChar, const char*>{ QChar(0x0E07), "ngonguthai" },                   // Character 'ง' Letter
    std::pair<QChar, const char*>{ QChar(0x3093), "nhiragana" },                    // Character 'ん' Letter
    std::pair<QChar, const char*>{ QChar(0x0272), "nhookleft" },                    // Character 'ɲ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0273), "nhookretroflex" },               // Character 'ɳ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x326F), "nieunacirclekorean" },           // Character '㉯' Symbol
    std::pair<QChar, const char*>{ QChar(0x320F), "nieunaparenkorean" },            // Character '㈏' Symbol
    std::pair<QChar, const char*>{ QChar(0x3135), "nieuncieuckorean" },             // Character 'ㄵ' Letter
    std::pair<QChar, const char*>{ QChar(0x3261), "nieuncirclekorean" },            // Character '㉡' Symbol
    std::pair<QChar, const char*>{ QChar(0x3136), "nieunhieuhkorean" },             // Character 'ㄶ' Letter
    std::pair<QChar, const char*>{ QChar(0x3134), "nieunkorean" },                  // Character 'ㄴ' Letter
    std::pair<QChar, const char*>{ QChar(0x3168), "nieunpansioskorean" },           // Character 'ㅨ' Letter
    std::pair<QChar, const char*>{ QChar(0x3201), "nieunparenkorean" },             // Character '㈁' Symbol
    std::pair<QChar, const char*>{ QChar(0x3167), "nieunsioskorean" },              // Character 'ㅧ' Letter
    std::pair<QChar, const char*>{ QChar(0x3166), "nieuntikeutkorean" },            // Character 'ㅦ' Letter
    std::pair<QChar, const char*>{ QChar(0x306B), "nihiragana" },                   // Character 'に' Letter
    std::pair<QChar, const char*>{ QChar(0x30CB), "nikatakana" },                   // Character 'ニ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF86), "nikatakanahalfwidth" },          // Character 'ﾆ' Letter
    std::pair<QChar, const char*>{ QChar(0xF899), "nikhahitleftthai" },             //
    std::pair<QChar, const char*>{ QChar(0x0E4D), "nikhahitthai" },                 // Character 'ํ' Mark
    std::pair<QChar, const char*>{ QChar(0x0039), "nine" },                         // Character '9' Digit
    std::pair<QChar, const char*>{ QChar(0x0669), "ninearabic" },                   // Character '٩' Digit
    std::pair<QChar, const char*>{ QChar(0x09EF), "ninebengali" },                  // Character '৯' Digit
    std::pair<QChar, const char*>{ QChar(0x2468), "ninecircle" },                   // Character '⑨'
    std::pair<QChar, const char*>{ QChar(0x2792), "ninecircleinversesansserif" },   // Character '➒'
    std::pair<QChar, const char*>{ QChar(0x096F), "ninedeva" },                     // Character '९' Digit
    std::pair<QChar, const char*>{ QChar(0x0AEF), "ninegujarati" },                 // Character '૯' Digit
    std::pair<QChar, const char*>{ QChar(0x0A6F), "ninegurmukhi" },                 // Character '੯' Digit
    std::pair<QChar, const char*>{ QChar(0x0669), "ninehackarabic" },               // Character '٩' Digit
    std::pair<QChar, const char*>{ QChar(0x3029), "ninehangzhou" },                 // Character '〩'
    std::pair<QChar, const char*>{ QChar(0x3228), "nineideographicparen" },         // Character '㈨'
    std::pair<QChar, const char*>{ QChar(0x2089), "nineinferior" },                 // Character '₉'
    std::pair<QChar, const char*>{ QChar(0xFF19), "ninemonospace" },                // Character '９' Digit
    std::pair<QChar, const char*>{ QChar(0xF739), "nineoldstyle" },                 //
    std::pair<QChar, const char*>{ QChar(0x247C), "nineparen" },                    // Character '⑼'
    std::pair<QChar, const char*>{ QChar(0x2490), "nineperiod" },                   // Character '⒐'
    std::pair<QChar, const char*>{ QChar(0x06F9), "ninepersian" },                  // Character '۹' Digit
    std::pair<QChar, const char*>{ QChar(0x2178), "nineroman" },                    // Character 'ⅸ'
    std::pair<QChar, const char*>{ QChar(0x2079), "ninesuperior" },                 // Character '⁹'
    std::pair<QChar, const char*>{ QChar(0x2472), "nineteencircle" },               // Character '⑲'
    std::pair<QChar, const char*>{ QChar(0x2486), "nineteenparen" },                // Character '⒆'
    std::pair<QChar, const char*>{ QChar(0x249A), "nineteenperiod" },               // Character '⒚'
    std::pair<QChar, const char*>{ QChar(0x0E59), "ninethai" },                     // Character '๙' Digit
    std::pair<QChar, const char*>{ QChar(0x01CC), "nj" },                           // Character 'ǌ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x045A), "njecyrillic" },                  // Character 'њ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x30F3), "nkatakana" },                    // Character 'ン' Letter
    std::pair<QChar, const char*>{ QChar(0xFF9D), "nkatakanahalfwidth" },           // Character 'ﾝ' Letter
    std::pair<QChar, const char*>{ QChar(0x019E), "nlegrightlong" },                // Character 'ƞ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E49), "nlinebelow" },                   // Character 'ṉ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0xFF4E), "nmonospace" },                   // Character 'ｎ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x339A), "nmsquare" },                     // Character '㎚' Symbol
    std::pair<QChar, const char*>{ QChar(0x09A3), "nnabengali" },                   // Character 'ণ' Letter
    std::pair<QChar, const char*>{ QChar(0x0923), "nnadeva" },                      // Character 'ण' Letter
    std::pair<QChar, const char*>{ QChar(0x0AA3), "nnagujarati" },                  // Character 'ણ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A23), "nnagurmukhi" },                  // Character 'ਣ' Letter
    std::pair<QChar, const char*>{ QChar(0x0929), "nnnadeva" },                     // Character 'ऩ' Letter
    std::pair<QChar, const char*>{ QChar(0x306E), "nohiragana" },                   // Character 'の' Letter
    std::pair<QChar, const char*>{ QChar(0x30CE), "nokatakana" },                   // Character 'ノ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF89), "nokatakanahalfwidth" },          // Character 'ﾉ' Letter
    std::pair<QChar, const char*>{ QChar(0x00A0), "nonbreakingspace" },             // Character ' ' Whitespace
    std::pair<QChar, const char*>{ QChar(0x0E13), "nonenthai" },                    // Character 'ณ' Letter
    std::pair<QChar, const char*>{ QChar(0x0E19), "nonuthai" },                     // Character 'น' Letter
    std::pair<QChar, const char*>{ QChar(0x0646), "noonarabic" },                   // Character 'ن' Letter
    std::pair<QChar, const char*>{ QChar(0xFEE6), "noonfinalarabic" },              // Character 'ﻦ' Letter
    std::pair<QChar, const char*>{ QChar(0x06BA), "noonghunnaarabic" },             // Character 'ں' Letter
    std::pair<QChar, const char*>{ QChar(0xFB9F), "noonghunnafinalarabic" },        // Character 'ﮟ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEEC), "noonhehinitialarabic" },         // Character 'ﻬ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEE7), "nooninitialarabic" },            // Character 'ﻧ' Letter
    std::pair<QChar, const char*>{ QChar(0xFCD2), "noonjeeminitialarabic" },        // Character 'ﳒ' Letter
    std::pair<QChar, const char*>{ QChar(0xFC4B), "noonjeemisolatedarabic" },       // Character 'ﱋ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEE8), "noonmedialarabic" },             // Character 'ﻨ' Letter
    std::pair<QChar, const char*>{ QChar(0xFCD5), "noonmeeminitialarabic" },        // Character 'ﳕ' Letter
    std::pair<QChar, const char*>{ QChar(0xFC4E), "noonmeemisolatedarabic" },       // Character 'ﱎ' Letter
    std::pair<QChar, const char*>{ QChar(0xFC8D), "noonnoonfinalarabic" },          // Character 'ﲍ' Letter
    std::pair<QChar, const char*>{ QChar(0x220C), "notcontains" },                  // Character '∌' Symbol
    std::pair<QChar, const char*>{ QChar(0x2209), "notelement" },                   // Character '∉' Symbol
    std::pair<QChar, const char*>{ QChar(0x2209), "notelementof" },                 // Character '∉' Symbol
    std::pair<QChar, const char*>{ QChar(0x2260), "notequal" },                     // Character '≠' Symbol
    std::pair<QChar, const char*>{ QChar(0x226F), "notgreater" },                   // Character '≯' Symbol
    std::pair<QChar, const char*>{ QChar(0x2271), "notgreaternorequal" },           // Character '≱' Symbol
    std::pair<QChar, const char*>{ QChar(0x2279), "notgreaternorless" },            // Character '≹' Symbol
    std::pair<QChar, const char*>{ QChar(0x2262), "notidentical" },                 // Character '≢' Symbol
    std::pair<QChar, const char*>{ QChar(0x226E), "notless" },                      // Character '≮' Symbol
    std::pair<QChar, const char*>{ QChar(0x2270), "notlessnorequal" },              // Character '≰' Symbol
    std::pair<QChar, const char*>{ QChar(0x2226), "notparallel" },                  // Character '∦' Symbol
    std::pair<QChar, const char*>{ QChar(0x2280), "notprecedes" },                  // Character '⊀' Symbol
    std::pair<QChar, const char*>{ QChar(0x2284), "notsubset" },                    // Character '⊄' Symbol
    std::pair<QChar, const char*>{ QChar(0x2281), "notsucceeds" },                  // Character '⊁' Symbol
    std::pair<QChar, const char*>{ QChar(0x2285), "notsuperset" },                  // Character '⊅' Symbol
    std::pair<QChar, const char*>{ QChar(0x0576), "nowarmenian" },                  // Character 'ն' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x24A9), "nparen" },                       // Character '⒩' Symbol
    std::pair<QChar, const char*>{ QChar(0x33B1), "nssquare" },                     // Character '㎱' Symbol
    std::pair<QChar, const char*>{ QChar(0x207F), "nsuperior" },                    // Character 'ⁿ' Letter
    std::pair<QChar, const char*>{ QChar(0x00F1), "ntilde" },                       // Character 'ñ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x03BD), "nu" },                           // Character 'ν' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x306C), "nuhiragana" },                   // Character 'ぬ' Letter
    std::pair<QChar, const char*>{ QChar(0x30CC), "nukatakana" },                   // Character 'ヌ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF87), "nukatakanahalfwidth" },          // Character 'ﾇ' Letter
    std::pair<QChar, const char*>{ QChar(0x09BC), "nuktabengali" },                 // Character '়' Mark
    std::pair<QChar, const char*>{ QChar(0x093C), "nuktadeva" },                    // Character '़' Mark
    std::pair<QChar, const char*>{ QChar(0x0ABC), "nuktagujarati" },                // Character '઼' Mark
    std::pair<QChar, const char*>{ QChar(0x0A3C), "nuktagurmukhi" },                // Character '਼' Mark
    std::pair<QChar, const char*>{ QChar(0x0023), "numbersign" },                   // Character '#' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFF03), "numbersignmonospace" },          // Character '＃' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFE5F), "numbersignsmall" },              // Character '﹟' Punctuation
    std::pair<QChar, const char*>{ QChar(0x0374), "numeralsigngreek" },             // Character 'ʹ' Letter
    std::pair<QChar, const char*>{ QChar(0x0375), "numeralsignlowergreek" },        // Character '͵' Symbol
    std::pair<QChar, const char*>{ QChar(0x2116), "numero" },                       // Character '№' Symbol
    std::pair<QChar, const char*>{ QChar(0x05E0), "nun" },                          // Character 'נ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB40), "nundagesh" },                    // Character 'נּ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB40), "nundageshhebrew" },              // Character 'נּ' Letter
    std::pair<QChar, const char*>{ QChar(0x05E0), "nunhebrew" },                    // Character 'נ' Letter
    std::pair<QChar, const char*>{ QChar(0x33B5), "nvsquare" },                     // Character '㎵' Symbol
    std::pair<QChar, const char*>{ QChar(0x33BB), "nwsquare" },                     // Character '㎻' Symbol
    std::pair<QChar, const char*>{ QChar(0x099E), "nyabengali" },                   // Character 'ঞ' Letter
    std::pair<QChar, const char*>{ QChar(0x091E), "nyadeva" },                      // Character 'ञ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A9E), "nyagujarati" },                  // Character 'ઞ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A1E), "nyagurmukhi" },                  // Character 'ਞ' Letter
    std::pair<QChar, const char*>{ QChar(0x006F), "o" },                            // Character 'o' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x00F3), "oacute" },                       // Character 'ó' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0E2D), "oangthai" },                     // Character 'อ' Letter
    std::pair<QChar, const char*>{ QChar(0x0275), "obarred" },                      // Character 'ɵ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x04E9), "obarredcyrillic" },              // Character 'ө' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x04EB), "obarreddieresiscyrillic" },      // Character 'ӫ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0993), "obengali" },                     // Character 'ও' Letter
    std::pair<QChar, const char*>{ QChar(0x311B), "obopomofo" },                    // Character 'ㄛ' Letter
    std::pair<QChar, const char*>{ QChar(0x014F), "obreve" },                       // Character 'ŏ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0911), "ocandradeva" },                  // Character 'ऑ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A91), "ocandragujarati" },              // Character 'ઑ' Letter
    std::pair<QChar, const char*>{ QChar(0x0949), "ocandravowelsigndeva" },         // Character 'ॉ' Mark
    std::pair<QChar, const char*>{ QChar(0x0AC9), "ocandravowelsigngujarati" },     // Character 'ૉ' Mark
    std::pair<QChar, const char*>{ QChar(0x01D2), "ocaron" },                       // Character 'ǒ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x24DE), "ocircle" },                      // Character 'ⓞ' Symbol
    std::pair<QChar, const char*>{ QChar(0x00F4), "ocircumflex" },                  // Character 'ô' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1ED1), "ocircumflexacute" },             // Character 'ố' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1ED9), "ocircumflexdotbelow" },          // Character 'ộ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1ED3), "ocircumflexgrave" },             // Character 'ồ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1ED5), "ocircumflexhookabove" },         // Character 'ổ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1ED7), "ocircumflextilde" },             // Character 'ỗ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x043E), "ocyrillic" },                    // Character 'о' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0151), "odblacute" },                    // Character 'ő' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x020D), "odblgrave" },                    // Character 'ȍ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0913), "odeva" },                        // Character 'ओ' Letter
    std::pair<QChar, const char*>{ QChar(0x00F6), "odieresis" },                    // Character 'ö' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x04E7), "odieresiscyrillic" },            // Character 'ӧ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1ECD), "odotbelow" },                    // Character 'ọ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0153), "oe" },                           // Character 'œ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x315A), "oekorean" },                     // Character 'ㅚ' Letter
    std::pair<QChar, const char*>{ QChar(0x02DB), "ogonek" },                       // Character '˛' Symbol
    std::pair<QChar, const char*>{ QChar(0x0328), "ogonekcmb" },                    // Character '̨' Mark
    std::pair<QChar, const char*>{ QChar(0x00F2), "ograve" },                       // Character 'ò' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0A93), "ogujarati" },                    // Character 'ઓ' Letter
    std::pair<QChar, const char*>{ QChar(0x0585), "oharmenian" },                   // Character 'օ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x304A), "ohiragana" },                    // Character 'お' Letter
    std::pair<QChar, const char*>{ QChar(0x1ECF), "ohookabove" },                   // Character 'ỏ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x01A1), "ohorn" },                        // Character 'ơ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1EDB), "ohornacute" },                   // Character 'ớ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1EE3), "ohorndotbelow" },                // Character 'ợ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1EDD), "ohorngrave" },                   // Character 'ờ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1EDF), "ohornhookabove" },               // Character 'ở' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1EE1), "ohorntilde" },                   // Character 'ỡ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0151), "ohungarumlaut" },                // Character 'ő' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x01A3), "oi" },                           // Character 'ƣ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x020F), "oinvertedbreve" },               // Character 'ȏ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x30AA), "okatakana" },                    // Character 'オ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF75), "okatakanahalfwidth" },           // Character 'ｵ' Letter
    std::pair<QChar, const char*>{ QChar(0x3157), "okorean" },                      // Character 'ㅗ' Letter
    std::pair<QChar, const char*>{ QChar(0x05AB), "olehebrew" },                    // Character '֫' Mark
    std::pair<QChar, const char*>{ QChar(0x014D), "omacron" },                      // Character 'ō' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E53), "omacronacute" },                 // Character 'ṓ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E51), "omacrongrave" },                 // Character 'ṑ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0950), "omdeva" },                       // Character 'ॐ' Letter
    std::pair<QChar, const char*>{ QChar(0x03C9), "omega" },                        // Character 'ω' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x03D6), "omega1" },                       // Character 'ϖ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0461), "omegacyrillic" },                // Character 'ѡ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0277), "omegalatinclosed" },             // Character 'ɷ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x047B), "omegaroundcyrillic" },           // Character 'ѻ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x047D), "omegatitlocyrillic" },           // Character 'ѽ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x03CE), "omegatonos" },                   // Character 'ώ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0AD0), "omgujarati" },                   // Character 'ૐ' Letter
    std::pair<QChar, const char*>{ QChar(0x03BF), "omicron" },                      // Character 'ο' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x03CC), "omicrontonos" },                 // Character 'ό' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0xFF4F), "omonospace" },                   // Character 'ｏ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0031), "one" },                          // Character '1' Digit
    std::pair<QChar, const char*>{ QChar(0x0661), "onearabic" },                    // Character '١' Digit
    std::pair<QChar, const char*>{ QChar(0x09E7), "onebengali" },                   // Character '১' Digit
    std::pair<QChar, const char*>{ QChar(0x2460), "onecircle" },                    // Character '①'
    std::pair<QChar, const char*>{ QChar(0x278A), "onecircleinversesansserif" },    // Character '➊'
    std::pair<QChar, const char*>{ QChar(0x0967), "onedeva" },                      // Character '१' Digit
    std::pair<QChar, const char*>{ QChar(0x2024), "onedotenleader" },               // Character '․' Punctuation
    std::pair<QChar, const char*>{ QChar(0x215B), "oneeighth" },                    // Character '⅛'
    std::pair<QChar, const char*>{ QChar(0xF6DC), "onefitted" },                    //
    std::pair<QChar, const char*>{ QChar(0x0AE7), "onegujarati" },                  // Character '૧' Digit
    std::pair<QChar, const char*>{ QChar(0x0A67), "onegurmukhi" },                  // Character '੧' Digit
    std::pair<QChar, const char*>{ QChar(0x0661), "onehackarabic" },                // Character '١' Digit
    std::pair<QChar, const char*>{ QChar(0x00BD), "onehalf" },                      // Character '½'
    std::pair<QChar, const char*>{ QChar(0x3021), "onehangzhou" },                  // Character '〡'
    std::pair<QChar, const char*>{ QChar(0x3220), "oneideographicparen" },          // Character '㈠'
    std::pair<QChar, const char*>{ QChar(0x2081), "oneinferior" },                  // Character '₁'
    std::pair<QChar, const char*>{ QChar(0xFF11), "onemonospace" },                 // Character '１' Digit
    std::pair<QChar, const char*>{ QChar(0x09F4), "onenumeratorbengali" },          // Character '৴'
    std::pair<QChar, const char*>{ QChar(0xF731), "oneoldstyle" },                  //
    std::pair<QChar, const char*>{ QChar(0x2474), "oneparen" },                     // Character '⑴'
    std::pair<QChar, const char*>{ QChar(0x2488), "oneperiod" },                    // Character '⒈'
    std::pair<QChar, const char*>{ QChar(0x06F1), "onepersian" },                   // Character '۱' Digit
    std::pair<QChar, const char*>{ QChar(0x00BC), "onequarter" },                   // Character '¼'
    std::pair<QChar, const char*>{ QChar(0x2170), "oneroman" },                     // Character 'ⅰ'
    std::pair<QChar, const char*>{ QChar(0x00B9), "onesuperior" },                  // Character '¹'
    std::pair<QChar, const char*>{ QChar(0x0E51), "onethai" },                      // Character '๑' Digit
    std::pair<QChar, const char*>{ QChar(0x2153), "onethird" },                     // Character '⅓'
    std::pair<QChar, const char*>{ QChar(0x01EB), "oogonek" },                      // Character 'ǫ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x01ED), "oogonekmacron" },                // Character 'ǭ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0A13), "oogurmukhi" },                   // Character 'ਓ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A4B), "oomatragurmukhi" },              // Character 'ੋ' Mark
    std::pair<QChar, const char*>{ QChar(0x0254), "oopen" },                        // Character 'ɔ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x24AA), "oparen" },                       // Character '⒪' Symbol
    std::pair<QChar, const char*>{ QChar(0x25E6), "openbullet" },                   // Character '◦' Symbol
    std::pair<QChar, const char*>{ QChar(0x2325), "option" },                       // Character '⌥' Symbol
    std::pair<QChar, const char*>{ QChar(0x00AA), "ordfeminine" },                  // Character 'ª' Letter
    std::pair<QChar, const char*>{ QChar(0x00BA), "ordmasculine" },                 // Character 'º' Letter
    std::pair<QChar, const char*>{ QChar(0x221F), "orthogonal" },                   // Character '∟' Symbol
    std::pair<QChar, const char*>{ QChar(0x0912), "oshortdeva" },                   // Character 'ऒ' Letter
    std::pair<QChar, const char*>{ QChar(0x094A), "oshortvowelsigndeva" },          // Character 'ॊ' Mark
    std::pair<QChar, const char*>{ QChar(0x00F8), "oslash" },                       // Character 'ø' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x01FF), "oslashacute" },                  // Character 'ǿ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3049), "osmallhiragana" },               // Character 'ぉ' Letter
    std::pair<QChar, const char*>{ QChar(0x30A9), "osmallkatakana" },               // Character 'ォ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF6B), "osmallkatakanahalfwidth" },      // Character 'ｫ' Letter
    std::pair<QChar, const char*>{ QChar(0x01FF), "ostrokeacute" },                 // Character 'ǿ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0xF6F0), "osuperior" },                    //
    std::pair<QChar, const char*>{ QChar(0x047F), "otcyrillic" },                   // Character 'ѿ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x00F5), "otilde" },                       // Character 'õ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E4D), "otildeacute" },                  // Character 'ṍ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E4F), "otildedieresis" },               // Character 'ṏ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3121), "oubopomofo" },                   // Character 'ㄡ' Letter
    std::pair<QChar, const char*>{ QChar(0x203E), "overline" },                     // Character '‾' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFE4A), "overlinecenterline" },           // Character '﹊' Punctuation
    std::pair<QChar, const char*>{ QChar(0x0305), "overlinecmb" },                  // Character '̅' Mark
    std::pair<QChar, const char*>{ QChar(0xFE49), "overlinedashed" },               // Character '﹉' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFE4C), "overlinedblwavy" },              // Character '﹌' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFE4B), "overlinewavy" },                 // Character '﹋' Punctuation
    std::pair<QChar, const char*>{ QChar(0x00AF), "overscore" },                    // Character '¯' Symbol
    std::pair<QChar, const char*>{ QChar(0x09CB), "ovowelsignbengali" },            // Character 'ো' Mark
    std::pair<QChar, const char*>{ QChar(0x094B), "ovowelsigndeva" },               // Character 'ो' Mark
    std::pair<QChar, const char*>{ QChar(0x0ACB), "ovowelsigngujarati" },           // Character 'ો' Mark
    std::pair<QChar, const char*>{ QChar(0x0070), "p" },                            // Character 'p' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3380), "paampssquare" },                 // Character '㎀' Symbol
    std::pair<QChar, const char*>{ QChar(0x332B), "paasentosquare" },               // Character '㌫' Symbol
    std::pair<QChar, const char*>{ QChar(0x09AA), "pabengali" },                    // Character 'প' Letter
    std::pair<QChar, const char*>{ QChar(0x1E55), "pacute" },                       // Character 'ṕ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x092A), "padeva" },                       // Character 'प' Letter
    std::pair<QChar, const char*>{ QChar(0x21DF), "pagedown" },                     // Character '⇟' Symbol
    std::pair<QChar, const char*>{ QChar(0x21DE), "pageup" },                       // Character '⇞' Symbol
    std::pair<QChar, const char*>{ QChar(0x0AAA), "pagujarati" },                   // Character 'પ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A2A), "pagurmukhi" },                   // Character 'ਪ' Letter
    std::pair<QChar, const char*>{ QChar(0x3071), "pahiragana" },                   // Character 'ぱ' Letter
    std::pair<QChar, const char*>{ QChar(0x0E2F), "paiyannoithai" },                // Character 'ฯ' Letter
    std::pair<QChar, const char*>{ QChar(0x30D1), "pakatakana" },                   // Character 'パ' Letter
    std::pair<QChar, const char*>{ QChar(0x0484), "palatalizationcyrilliccmb" },    // Character '҄' Mark
    std::pair<QChar, const char*>{ QChar(0x04C0), "palochkacyrillic" },             // Character 'Ӏ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x317F), "pansioskorean" },                // Character 'ㅿ' Letter
    std::pair<QChar, const char*>{ QChar(0x00B6), "paragraph" },                    // Character '¶' Punctuation
    std::pair<QChar, const char*>{ QChar(0x2225), "parallel" },                     // Character '∥' Symbol
    std::pair<QChar, const char*>{ QChar(0x0028), "parenleft" },                    // Character '(' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFD3E), "parenleftaltonearabic" },        // Character '﴾' Punctuation
    std::pair<QChar, const char*>{ QChar(0xF8ED), "parenleftbt" },                  //
    std::pair<QChar, const char*>{ QChar(0xF8EC), "parenleftex" },                  //
    std::pair<QChar, const char*>{ QChar(0x208D), "parenleftinferior" },            // Character '₍' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFF08), "parenleftmonospace" },           // Character '（' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFE59), "parenleftsmall" },               // Character '﹙' Punctuation
    std::pair<QChar, const char*>{ QChar(0x207D), "parenleftsuperior" },            // Character '⁽' Punctuation
    std::pair<QChar, const char*>{ QChar(0xF8EB), "parenlefttp" },                  //
    std::pair<QChar, const char*>{ QChar(0xFE35), "parenleftvertical" },            // Character '︵' Punctuation
    std::pair<QChar, const char*>{ QChar(0x0029), "parenright" },                   // Character ')' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFD3F), "parenrightaltonearabic" },       // Character '﴿' Punctuation
    std::pair<QChar, const char*>{ QChar(0xF8F8), "parenrightbt" },                 //
    std::pair<QChar, const char*>{ QChar(0xF8F7), "parenrightex" },                 //
    std::pair<QChar, const char*>{ QChar(0x208E), "parenrightinferior" },           // Character '₎' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFF09), "parenrightmonospace" },          // Character '）' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFE5A), "parenrightsmall" },              // Character '﹚' Punctuation
    std::pair<QChar, const char*>{ QChar(0x207E), "parenrightsuperior" },           // Character '⁾' Punctuation
    std::pair<QChar, const char*>{ QChar(0xF8F6), "parenrighttp" },                 //
    std::pair<QChar, const char*>{ QChar(0xFE36), "parenrightvertical" },           // Character '︶' Punctuation
    std::pair<QChar, const char*>{ QChar(0x2202), "partialdiff" },                  // Character '∂' Symbol
    std::pair<QChar, const char*>{ QChar(0x05C0), "paseqhebrew" },                  // Character '׀' Punctuation
    std::pair<QChar, const char*>{ QChar(0x0599), "pashtahebrew" },                 // Character '֙' Mark
    std::pair<QChar, const char*>{ QChar(0x33A9), "pasquare" },                     // Character '㎩' Symbol
    std::pair<QChar, const char*>{ QChar(0x05B7), "patah" },                        // Character 'ַ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B7), "patah11" },                      // Character 'ַ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B7), "patah1d" },                      // Character 'ַ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B7), "patah2a" },                      // Character 'ַ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B7), "patahhebrew" },                  // Character 'ַ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B7), "patahnarrowhebrew" },            // Character 'ַ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B7), "patahquarterhebrew" },           // Character 'ַ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B7), "patahwidehebrew" },              // Character 'ַ' Mark
    std::pair<QChar, const char*>{ QChar(0x05A1), "pazerhebrew" },                  // Character '֡' Mark
    std::pair<QChar, const char*>{ QChar(0x3106), "pbopomofo" },                    // Character 'ㄆ' Letter
    std::pair<QChar, const char*>{ QChar(0x24DF), "pcircle" },                      // Character 'ⓟ' Symbol
    std::pair<QChar, const char*>{ QChar(0x1E57), "pdotaccent" },                   // Character 'ṗ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x05E4), "pe" },                           // Character 'פ' Letter
    std::pair<QChar, const char*>{ QChar(0x043F), "pecyrillic" },                   // Character 'п' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0xFB44), "pedagesh" },                     // Character 'פּ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB44), "pedageshhebrew" },               // Character 'פּ' Letter
    std::pair<QChar, const char*>{ QChar(0x333B), "peezisquare" },                  // Character '㌻' Symbol
    std::pair<QChar, const char*>{ QChar(0xFB43), "pefinaldageshhebrew" },          // Character 'ףּ' Letter
    std::pair<QChar, const char*>{ QChar(0x067E), "peharabic" },                    // Character 'پ' Letter
    std::pair<QChar, const char*>{ QChar(0x057A), "peharmenian" },                  // Character 'պ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x05E4), "pehebrew" },                     // Character 'פ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB57), "pehfinalarabic" },               // Character 'ﭗ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB58), "pehinitialarabic" },             // Character 'ﭘ' Letter
    std::pair<QChar, const char*>{ QChar(0x307A), "pehiragana" },                   // Character 'ぺ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB59), "pehmedialarabic" },              // Character 'ﭙ' Letter
    std::pair<QChar, const char*>{ QChar(0x30DA), "pekatakana" },                   // Character 'ペ' Letter
    std::pair<QChar, const char*>{ QChar(0x04A7), "pemiddlehookcyrillic" },         // Character 'ҧ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0xFB4E), "perafehebrew" },                 // Character 'פֿ' Letter
    std::pair<QChar, const char*>{ QChar(0x0025), "percent" },                      // Character '%' Punctuation
    std::pair<QChar, const char*>{ QChar(0x066A), "percentarabic" },                // Character '٪' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFF05), "percentmonospace" },             // Character '％' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFE6A), "percentsmall" },                 // Character '﹪' Punctuation
    std::pair<QChar, const char*>{ QChar(0x002E), "period" },                       // Character '.' Punctuation
    std::pair<QChar, const char*>{ QChar(0x0589), "periodarmenian" },               // Character '։' Punctuation
    std::pair<QChar, const char*>{ QChar(0x00B7), "periodcentered" },               // Character '·' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFF61), "periodhalfwidth" },              // Character '｡' Punctuation
    std::pair<QChar, const char*>{ QChar(0xF6E7), "periodinferior" },               //
    std::pair<QChar, const char*>{ QChar(0xFF0E), "periodmonospace" },              // Character '．' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFE52), "periodsmall" },                  // Character '﹒' Punctuation
    std::pair<QChar, const char*>{ QChar(0xF6E8), "periodsuperior" },               //
    std::pair<QChar, const char*>{ QChar(0x0342), "perispomenigreekcmb" },          // Character '͂' Mark
    std::pair<QChar, const char*>{ QChar(0x22A5), "perpendicular" },                // Character '⊥' Symbol
    std::pair<QChar, const char*>{ QChar(0x2030), "perthousand" },                  // Character '‰' Punctuation
    std::pair<QChar, const char*>{ QChar(0x20A7), "peseta" },                       // Character '₧' Symbol
    std::pair<QChar, const char*>{ QChar(0x338A), "pfsquare" },                     // Character '㎊' Symbol
    std::pair<QChar, const char*>{ QChar(0x09AB), "phabengali" },                   // Character 'ফ' Letter
    std::pair<QChar, const char*>{ QChar(0x092B), "phadeva" },                      // Character 'फ' Letter
    std::pair<QChar, const char*>{ QChar(0x0AAB), "phagujarati" },                  // Character 'ફ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A2B), "phagurmukhi" },                  // Character 'ਫ' Letter
    std::pair<QChar, const char*>{ QChar(0x03C6), "phi" },                          // Character 'φ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x03D5), "phi1" },                         // Character 'ϕ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x327A), "phieuphacirclekorean" },         // Character '㉺' Symbol
    std::pair<QChar, const char*>{ QChar(0x321A), "phieuphaparenkorean" },          // Character '㈚' Symbol
    std::pair<QChar, const char*>{ QChar(0x326C), "phieuphcirclekorean" },          // Character '㉬' Symbol
    std::pair<QChar, const char*>{ QChar(0x314D), "phieuphkorean" },                // Character 'ㅍ' Letter
    std::pair<QChar, const char*>{ QChar(0x320C), "phieuphparenkorean" },           // Character '㈌' Symbol
    std::pair<QChar, const char*>{ QChar(0x0278), "philatin" },                     // Character 'ɸ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0E3A), "phinthuthai" },                  // Character 'ฺ' Mark
    std::pair<QChar, const char*>{ QChar(0x03D5), "phisymbolgreek" },               // Character 'ϕ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x01A5), "phook" },                        // Character 'ƥ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0E1E), "phophanthai" },                  // Character 'พ' Letter
    std::pair<QChar, const char*>{ QChar(0x0E1C), "phophungthai" },                 // Character 'ผ' Letter
    std::pair<QChar, const char*>{ QChar(0x0E20), "phosamphaothai" },               // Character 'ภ' Letter
    std::pair<QChar, const char*>{ QChar(0x03C0), "pi" },                           // Character 'π' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3273), "pieupacirclekorean" },           // Character '㉳' Symbol
    std::pair<QChar, const char*>{ QChar(0x3213), "pieupaparenkorean" },            // Character '㈓' Symbol
    std::pair<QChar, const char*>{ QChar(0x3176), "pieupcieuckorean" },             // Character 'ㅶ' Letter
    std::pair<QChar, const char*>{ QChar(0x3265), "pieupcirclekorean" },            // Character '㉥' Symbol
    std::pair<QChar, const char*>{ QChar(0x3172), "pieupkiyeokkorean" },            // Character 'ㅲ' Letter
    std::pair<QChar, const char*>{ QChar(0x3142), "pieupkorean" },                  // Character 'ㅂ' Letter
    std::pair<QChar, const char*>{ QChar(0x3205), "pieupparenkorean" },             // Character '㈅' Symbol
    std::pair<QChar, const char*>{ QChar(0x3174), "pieupsioskiyeokkorean" },        // Character 'ㅴ' Letter
    std::pair<QChar, const char*>{ QChar(0x3144), "pieupsioskorean" },              // Character 'ㅄ' Letter
    std::pair<QChar, const char*>{ QChar(0x3175), "pieupsiostikeutkorean" },        // Character 'ㅵ' Letter
    std::pair<QChar, const char*>{ QChar(0x3177), "pieupthieuthkorean" },           // Character 'ㅷ' Letter
    std::pair<QChar, const char*>{ QChar(0x3173), "pieuptikeutkorean" },            // Character 'ㅳ' Letter
    std::pair<QChar, const char*>{ QChar(0x3074), "pihiragana" },                   // Character 'ぴ' Letter
    std::pair<QChar, const char*>{ QChar(0x30D4), "pikatakana" },                   // Character 'ピ' Letter
    std::pair<QChar, const char*>{ QChar(0x03D6), "pisymbolgreek" },                // Character 'ϖ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0583), "piwrarmenian" },                 // Character 'փ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x002B), "plus" },                         // Character '+' Symbol
    std::pair<QChar, const char*>{ QChar(0x031F), "plusbelowcmb" },                 // Character '̟' Mark
    std::pair<QChar, const char*>{ QChar(0x2295), "pluscircle" },                   // Character '⊕' Symbol
    std::pair<QChar, const char*>{ QChar(0x00B1), "plusminus" },                    // Character '±' Symbol
    std::pair<QChar, const char*>{ QChar(0x02D6), "plusmod" },                      // Character '˖' Symbol
    std::pair<QChar, const char*>{ QChar(0xFF0B), "plusmonospace" },                // Character '＋' Symbol
    std::pair<QChar, const char*>{ QChar(0xFE62), "plussmall" },                    // Character '﹢' Symbol
    std::pair<QChar, const char*>{ QChar(0x207A), "plussuperior" },                 // Character '⁺' Symbol
    std::pair<QChar, const char*>{ QChar(0xFF50), "pmonospace" },                   // Character 'ｐ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x33D8), "pmsquare" },                     // Character '㏘' Symbol
    std::pair<QChar, const char*>{ QChar(0x307D), "pohiragana" },                   // Character 'ぽ' Letter
    std::pair<QChar, const char*>{ QChar(0x261F), "pointingindexdownwhite" },       // Character '☟' Symbol
    std::pair<QChar, const char*>{ QChar(0x261C), "pointingindexleftwhite" },       // Character '☜' Symbol
    std::pair<QChar, const char*>{ QChar(0x261E), "pointingindexrightwhite" },      // Character '☞' Symbol
    std::pair<QChar, const char*>{ QChar(0x261D), "pointingindexupwhite" },         // Character '☝' Symbol
    std::pair<QChar, const char*>{ QChar(0x30DD), "pokatakana" },                   // Character 'ポ' Letter
    std::pair<QChar, const char*>{ QChar(0x0E1B), "poplathai" },                    // Character 'ป' Letter
    std::pair<QChar, const char*>{ QChar(0x3012), "postalmark" },                   // Character '〒' Symbol
    std::pair<QChar, const char*>{ QChar(0x3020), "postalmarkface" },               // Character '〠' Symbol
    std::pair<QChar, const char*>{ QChar(0x24AB), "pparen" },                       // Character '⒫' Symbol
    std::pair<QChar, const char*>{ QChar(0x227A), "precedes" },                     // Character '≺' Symbol
    std::pair<QChar, const char*>{ QChar(0x211E), "prescription" },                 // Character '℞' Symbol
    std::pair<QChar, const char*>{ QChar(0x02B9), "primemod" },                     // Character 'ʹ' Letter
    std::pair<QChar, const char*>{ QChar(0x2035), "primereversed" },                // Character '‵' Punctuation
    std::pair<QChar, const char*>{ QChar(0x220F), "product" },                      // Character '∏' Symbol
    std::pair<QChar, const char*>{ QChar(0x2305), "projective" },                   // Character '⌅' Symbol
    std::pair<QChar, const char*>{ QChar(0x30FC), "prolongedkana" },                // Character 'ー' Letter
    std::pair<QChar, const char*>{ QChar(0x2318), "propellor" },                    // Character '⌘' Symbol
    std::pair<QChar, const char*>{ QChar(0x2282), "propersubset" },                 // Character '⊂' Symbol
    std::pair<QChar, const char*>{ QChar(0x2283), "propersuperset" },               // Character '⊃' Symbol
    std::pair<QChar, const char*>{ QChar(0x2237), "proportion" },                   // Character '∷' Symbol
    std::pair<QChar, const char*>{ QChar(0x221D), "proportional" },                 // Character '∝' Symbol
    std::pair<QChar, const char*>{ QChar(0x03C8), "psi" },                          // Character 'ψ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0471), "psicyrillic" },                  // Character 'ѱ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0486), "psilipneumatacyrilliccmb" },     // Character '҆' Mark
    std::pair<QChar, const char*>{ QChar(0x33B0), "pssquare" },                     // Character '㎰' Symbol
    std::pair<QChar, const char*>{ QChar(0x3077), "puhiragana" },                   // Character 'ぷ' Letter
    std::pair<QChar, const char*>{ QChar(0x30D7), "pukatakana" },                   // Character 'プ' Letter
    std::pair<QChar, const char*>{ QChar(0x33B4), "pvsquare" },                     // Character '㎴' Symbol
    std::pair<QChar, const char*>{ QChar(0x33BA), "pwsquare" },                     // Character '㎺' Symbol
    std::pair<QChar, const char*>{ QChar(0x0071), "q" },                            // Character 'q' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0958), "qadeva" },                       // Character 'क़' Letter
    std::pair<QChar, const char*>{ QChar(0x05A8), "qadmahebrew" },                  // Character '֨' Mark
    std::pair<QChar, const char*>{ QChar(0x0642), "qafarabic" },                    // Character 'ق' Letter
    std::pair<QChar, const char*>{ QChar(0xFED6), "qaffinalarabic" },               // Character 'ﻖ' Letter
    std::pair<QChar, const char*>{ QChar(0xFED7), "qafinitialarabic" },             // Character 'ﻗ' Letter
    std::pair<QChar, const char*>{ QChar(0xFED8), "qafmedialarabic" },              // Character 'ﻘ' Letter
    std::pair<QChar, const char*>{ QChar(0x05B8), "qamats" },                       // Character 'ָ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B8), "qamats10" },                     // Character 'ָ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B8), "qamats1a" },                     // Character 'ָ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B8), "qamats1c" },                     // Character 'ָ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B8), "qamats27" },                     // Character 'ָ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B8), "qamats29" },                     // Character 'ָ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B8), "qamats33" },                     // Character 'ָ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B8), "qamatsde" },                     // Character 'ָ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B8), "qamatshebrew" },                 // Character 'ָ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B8), "qamatsnarrowhebrew" },           // Character 'ָ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B8), "qamatsqatanhebrew" },            // Character 'ָ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B8), "qamatsqatannarrowhebrew" },      // Character 'ָ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B8), "qamatsqatanquarterhebrew" },     // Character 'ָ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B8), "qamatsqatanwidehebrew" },        // Character 'ָ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B8), "qamatsquarterhebrew" },          // Character 'ָ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B8), "qamatswidehebrew" },             // Character 'ָ' Mark
    std::pair<QChar, const char*>{ QChar(0x059F), "qarneyparahebrew" },             // Character '֟' Mark
    std::pair<QChar, const char*>{ QChar(0x3111), "qbopomofo" },                    // Character 'ㄑ' Letter
    std::pair<QChar, const char*>{ QChar(0x24E0), "qcircle" },                      // Character 'ⓠ' Symbol
    std::pair<QChar, const char*>{ QChar(0x02A0), "qhook" },                        // Character 'ʠ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0xFF51), "qmonospace" },                   // Character 'ｑ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x05E7), "qof" },                          // Character 'ק' Letter
    std::pair<QChar, const char*>{ QChar(0xFB47), "qofdagesh" },                    // Character 'קּ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB47), "qofdageshhebrew" },              // Character 'קּ' Letter
    std::pair<QChar, const char*>{ QChar(0x05B2), "qofhatafpatah" },                // Character 'ֲ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B2), "qofhatafpatahhebrew" },          // Character 'ֲ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B1), "qofhatafsegol" },                // Character 'ֱ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B1), "qofhatafsegolhebrew" },          // Character 'ֱ' Mark
    std::pair<QChar, const char*>{ QChar(0x05E7), "qofhebrew" },                    // Character 'ק' Letter
    std::pair<QChar, const char*>{ QChar(0x05B4), "qofhiriq" },                     // Character 'ִ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B4), "qofhiriqhebrew" },               // Character 'ִ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B9), "qofholam" },                     // Character 'ֹ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B9), "qofholamhebrew" },               // Character 'ֹ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B7), "qofpatah" },                     // Character 'ַ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B7), "qofpatahhebrew" },               // Character 'ַ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B8), "qofqamats" },                    // Character 'ָ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B8), "qofqamatshebrew" },              // Character 'ָ' Mark
    std::pair<QChar, const char*>{ QChar(0x05BB), "qofqubuts" },                    // Character 'ֻ' Mark
    std::pair<QChar, const char*>{ QChar(0x05BB), "qofqubutshebrew" },              // Character 'ֻ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B6), "qofsegol" },                     // Character 'ֶ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B6), "qofsegolhebrew" },               // Character 'ֶ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B0), "qofsheva" },                     // Character 'ְ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B0), "qofshevahebrew" },               // Character 'ְ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B5), "qoftsere" },                     // Character 'ֵ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B5), "qoftserehebrew" },               // Character 'ֵ' Mark
    std::pair<QChar, const char*>{ QChar(0x24AC), "qparen" },                       // Character '⒬' Symbol
    std::pair<QChar, const char*>{ QChar(0x2669), "quarternote" },                  // Character '♩' Symbol
    std::pair<QChar, const char*>{ QChar(0x05BB), "qubuts" },                       // Character 'ֻ' Mark
    std::pair<QChar, const char*>{ QChar(0x05BB), "qubuts18" },                     // Character 'ֻ' Mark
    std::pair<QChar, const char*>{ QChar(0x05BB), "qubuts25" },                     // Character 'ֻ' Mark
    std::pair<QChar, const char*>{ QChar(0x05BB), "qubuts31" },                     // Character 'ֻ' Mark
    std::pair<QChar, const char*>{ QChar(0x05BB), "qubutshebrew" },                 // Character 'ֻ' Mark
    std::pair<QChar, const char*>{ QChar(0x05BB), "qubutsnarrowhebrew" },           // Character 'ֻ' Mark
    std::pair<QChar, const char*>{ QChar(0x05BB), "qubutsquarterhebrew" },          // Character 'ֻ' Mark
    std::pair<QChar, const char*>{ QChar(0x05BB), "qubutswidehebrew" },             // Character 'ֻ' Mark
    std::pair<QChar, const char*>{ QChar(0x003F), "question" },                     // Character '?' Punctuation
    std::pair<QChar, const char*>{ QChar(0x061F), "questionarabic" },               // Character '؟' Punctuation
    std::pair<QChar, const char*>{ QChar(0x055E), "questionarmenian" },             // Character '՞' Punctuation
    std::pair<QChar, const char*>{ QChar(0x00BF), "questiondown" },                 // Character '¿' Punctuation
    std::pair<QChar, const char*>{ QChar(0xF7BF), "questiondownsmall" },            //
    std::pair<QChar, const char*>{ QChar(0x037E), "questiongreek" },                // Character ';' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFF1F), "questionmonospace" },            // Character '？' Punctuation
    std::pair<QChar, const char*>{ QChar(0xF73F), "questionsmall" },                //
    std::pair<QChar, const char*>{ QChar(0x0022), "quotedbl" },                     // Character '"' Punctuation
    std::pair<QChar, const char*>{ QChar(0x201E), "quotedblbase" },                 // Character '„' Punctuation
    std::pair<QChar, const char*>{ QChar(0x201C), "quotedblleft" },                 // Character '“' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFF02), "quotedblmonospace" },            // Character '＂' Punctuation
    std::pair<QChar, const char*>{ QChar(0x301E), "quotedblprime" },                // Character '〞' Punctuation
    std::pair<QChar, const char*>{ QChar(0x301D), "quotedblprimereversed" },        // Character '〝' Punctuation
    std::pair<QChar, const char*>{ QChar(0x201D), "quotedblright" },                // Character '”' Punctuation
    std::pair<QChar, const char*>{ QChar(0x2018), "quoteleft" },                    // Character '‘' Punctuation
    std::pair<QChar, const char*>{ QChar(0x201B), "quoteleftreversed" },            // Character '‛' Punctuation
    std::pair<QChar, const char*>{ QChar(0x201B), "quotereversed" },                // Character '‛' Punctuation
    std::pair<QChar, const char*>{ QChar(0x2019), "quoteright" },                   // Character '’' Punctuation
    std::pair<QChar, const char*>{ QChar(0x0149), "quoterightn" },                  // Character 'ŉ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x201A), "quotesinglbase" },               // Character '‚' Punctuation
    std::pair<QChar, const char*>{ QChar(0x0027), "quotesingle" },                  // Character ''' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFF07), "quotesinglemonospace" },         // Character '＇' Punctuation
    std::pair<QChar, const char*>{ QChar(0x0072), "r" },                            // Character 'r' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x057C), "raarmenian" },                   // Character 'ռ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x09B0), "rabengali" },                    // Character 'র' Letter
    std::pair<QChar, const char*>{ QChar(0x0155), "racute" },                       // Character 'ŕ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0930), "radeva" },                       // Character 'र' Letter
    std::pair<QChar, const char*>{ QChar(0x221A), "radical" },                      // Character '√' Symbol
    std::pair<QChar, const char*>{ QChar(0xF8E5), "radicalex" },                    //
    std::pair<QChar, const char*>{ QChar(0x33AE), "radoverssquare" },               // Character '㎮' Symbol
    std::pair<QChar, const char*>{ QChar(0x33AF), "radoverssquaredsquare" },        // Character '㎯' Symbol
    std::pair<QChar, const char*>{ QChar(0x33AD), "radsquare" },                    // Character '㎭' Symbol
    std::pair<QChar, const char*>{ QChar(0x05BF), "rafe" },                         // Character 'ֿ' Mark
    std::pair<QChar, const char*>{ QChar(0x05BF), "rafehebrew" },                   // Character 'ֿ' Mark
    std::pair<QChar, const char*>{ QChar(0x0AB0), "ragujarati" },                   // Character 'ર' Letter
    std::pair<QChar, const char*>{ QChar(0x0A30), "ragurmukhi" },                   // Character 'ਰ' Letter
    std::pair<QChar, const char*>{ QChar(0x3089), "rahiragana" },                   // Character 'ら' Letter
    std::pair<QChar, const char*>{ QChar(0x30E9), "rakatakana" },                   // Character 'ラ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF97), "rakatakanahalfwidth" },          // Character 'ﾗ' Letter
    std::pair<QChar, const char*>{ QChar(0x09F1), "ralowerdiagonalbengali" },       // Character 'ৱ' Letter
    std::pair<QChar, const char*>{ QChar(0x09F0), "ramiddlediagonalbengali" },      // Character 'ৰ' Letter
    std::pair<QChar, const char*>{ QChar(0x0264), "ramshorn" },                     // Character 'ɤ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x2236), "ratio" },                        // Character '∶' Symbol
    std::pair<QChar, const char*>{ QChar(0x3116), "rbopomofo" },                    // Character 'ㄖ' Letter
    std::pair<QChar, const char*>{ QChar(0x0159), "rcaron" },                       // Character 'ř' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0157), "rcedilla" },                     // Character 'ŗ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x24E1), "rcircle" },                      // Character 'ⓡ' Symbol
    std::pair<QChar, const char*>{ QChar(0x0157), "rcommaaccent" },                 // Character 'ŗ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0211), "rdblgrave" },                    // Character 'ȑ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E59), "rdotaccent" },                   // Character 'ṙ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E5B), "rdotbelow" },                    // Character 'ṛ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E5D), "rdotbelowmacron" },              // Character 'ṝ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x203B), "referencemark" },                // Character '※' Punctuation
    std::pair<QChar, const char*>{ QChar(0x2286), "reflexsubset" },                 // Character '⊆' Symbol
    std::pair<QChar, const char*>{ QChar(0x2287), "reflexsuperset" },               // Character '⊇' Symbol
    std::pair<QChar, const char*>{ QChar(0x00AE), "registered" },                   // Character '®' Symbol
    std::pair<QChar, const char*>{ QChar(0xF8E8), "registersans" },                 //
    std::pair<QChar, const char*>{ QChar(0xF6DA), "registerserif" },                //
    std::pair<QChar, const char*>{ QChar(0x0631), "reharabic" },                    // Character 'ر' Letter
    std::pair<QChar, const char*>{ QChar(0x0580), "reharmenian" },                  // Character 'ր' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0xFEAE), "rehfinalarabic" },               // Character 'ﺮ' Letter
    std::pair<QChar, const char*>{ QChar(0x308C), "rehiragana" },                   // Character 'れ' Letter
    std::pair<QChar, const char*>{ QChar(0x0644), "rehyehaleflamarabic" },          // Character 'ل' Letter
    std::pair<QChar, const char*>{ QChar(0x30EC), "rekatakana" },                   // Character 'レ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF9A), "rekatakanahalfwidth" },          // Character 'ﾚ' Letter
    std::pair<QChar, const char*>{ QChar(0x05E8), "resh" },                         // Character 'ר' Letter
    std::pair<QChar, const char*>{ QChar(0xFB48), "reshdageshhebrew" },             // Character 'רּ' Letter
    std::pair<QChar, const char*>{ QChar(0x05B2), "reshhatafpatah" },               // Character 'ֲ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B2), "reshhatafpatahhebrew" },         // Character 'ֲ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B1), "reshhatafsegol" },               // Character 'ֱ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B1), "reshhatafsegolhebrew" },         // Character 'ֱ' Mark
    std::pair<QChar, const char*>{ QChar(0x05E8), "reshhebrew" },                   // Character 'ר' Letter
    std::pair<QChar, const char*>{ QChar(0x05B4), "reshhiriq" },                    // Character 'ִ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B4), "reshhiriqhebrew" },              // Character 'ִ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B9), "reshholam" },                    // Character 'ֹ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B9), "reshholamhebrew" },              // Character 'ֹ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B7), "reshpatah" },                    // Character 'ַ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B7), "reshpatahhebrew" },              // Character 'ַ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B8), "reshqamats" },                   // Character 'ָ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B8), "reshqamatshebrew" },             // Character 'ָ' Mark
    std::pair<QChar, const char*>{ QChar(0x05BB), "reshqubuts" },                   // Character 'ֻ' Mark
    std::pair<QChar, const char*>{ QChar(0x05BB), "reshqubutshebrew" },             // Character 'ֻ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B6), "reshsegol" },                    // Character 'ֶ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B6), "reshsegolhebrew" },              // Character 'ֶ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B0), "reshsheva" },                    // Character 'ְ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B0), "reshshevahebrew" },              // Character 'ְ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B5), "reshtsere" },                    // Character 'ֵ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B5), "reshtserehebrew" },              // Character 'ֵ' Mark
    std::pair<QChar, const char*>{ QChar(0x223D), "reversedtilde" },                // Character '∽' Symbol
    std::pair<QChar, const char*>{ QChar(0x0597), "reviahebrew" },                  // Character '֗' Mark
    std::pair<QChar, const char*>{ QChar(0x0597), "reviamugrashhebrew" },           // Character '֗' Mark
    std::pair<QChar, const char*>{ QChar(0x2310), "revlogicalnot" },                // Character '⌐' Symbol
    std::pair<QChar, const char*>{ QChar(0x027E), "rfishhook" },                    // Character 'ɾ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x027F), "rfishhookreversed" },            // Character 'ɿ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x09DD), "rhabengali" },                   // Character 'ঢ়' Letter
    std::pair<QChar, const char*>{ QChar(0x095D), "rhadeva" },                      // Character 'ढ़' Letter
    std::pair<QChar, const char*>{ QChar(0x03C1), "rho" },                          // Character 'ρ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x027D), "rhook" },                        // Character 'ɽ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x027B), "rhookturned" },                  // Character 'ɻ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x02B5), "rhookturnedsuperior" },          // Character 'ʵ' Letter
    std::pair<QChar, const char*>{ QChar(0x03F1), "rhosymbolgreek" },               // Character 'ϱ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x02DE), "rhotichookmod" },                // Character '˞' Symbol
    std::pair<QChar, const char*>{ QChar(0x3271), "rieulacirclekorean" },           // Character '㉱' Symbol
    std::pair<QChar, const char*>{ QChar(0x3211), "rieulaparenkorean" },            // Character '㈑' Symbol
    std::pair<QChar, const char*>{ QChar(0x3263), "rieulcirclekorean" },            // Character '㉣' Symbol
    std::pair<QChar, const char*>{ QChar(0x3140), "rieulhieuhkorean" },             // Character 'ㅀ' Letter
    std::pair<QChar, const char*>{ QChar(0x313A), "rieulkiyeokkorean" },            // Character 'ㄺ' Letter
    std::pair<QChar, const char*>{ QChar(0x3169), "rieulkiyeoksioskorean" },        // Character 'ㅩ' Letter
    std::pair<QChar, const char*>{ QChar(0x3139), "rieulkorean" },                  // Character 'ㄹ' Letter
    std::pair<QChar, const char*>{ QChar(0x313B), "rieulmieumkorean" },             // Character 'ㄻ' Letter
    std::pair<QChar, const char*>{ QChar(0x316C), "rieulpansioskorean" },           // Character 'ㅬ' Letter
    std::pair<QChar, const char*>{ QChar(0x3203), "rieulparenkorean" },             // Character '㈃' Symbol
    std::pair<QChar, const char*>{ QChar(0x313F), "rieulphieuphkorean" },           // Character 'ㄿ' Letter
    std::pair<QChar, const char*>{ QChar(0x313C), "rieulpieupkorean" },             // Character 'ㄼ' Letter
    std::pair<QChar, const char*>{ QChar(0x316B), "rieulpieupsioskorean" },         // Character 'ㅫ' Letter
    std::pair<QChar, const char*>{ QChar(0x313D), "rieulsioskorean" },              // Character 'ㄽ' Letter
    std::pair<QChar, const char*>{ QChar(0x313E), "rieulthieuthkorean" },           // Character 'ㄾ' Letter
    std::pair<QChar, const char*>{ QChar(0x316A), "rieultikeutkorean" },            // Character 'ㅪ' Letter
    std::pair<QChar, const char*>{ QChar(0x316D), "rieulyeorinhieuhkorean" },       // Character 'ㅭ' Letter
    std::pair<QChar, const char*>{ QChar(0x221F), "rightangle" },                   // Character '∟' Symbol
    std::pair<QChar, const char*>{ QChar(0x0319), "righttackbelowcmb" },            // Character '̙' Mark
    std::pair<QChar, const char*>{ QChar(0x22BF), "righttriangle" },                // Character '⊿' Symbol
    std::pair<QChar, const char*>{ QChar(0x308A), "rihiragana" },                   // Character 'り' Letter
    std::pair<QChar, const char*>{ QChar(0x30EA), "rikatakana" },                   // Character 'リ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF98), "rikatakanahalfwidth" },          // Character 'ﾘ' Letter
    std::pair<QChar, const char*>{ QChar(0x02DA), "ring" },                         // Character '˚' Symbol
    std::pair<QChar, const char*>{ QChar(0x0325), "ringbelowcmb" },                 // Character '̥' Mark
    std::pair<QChar, const char*>{ QChar(0x030A), "ringcmb" },                      // Character '̊' Mark
    std::pair<QChar, const char*>{ QChar(0x02BF), "ringhalfleft" },                 // Character 'ʿ' Letter
    std::pair<QChar, const char*>{ QChar(0x0559), "ringhalfleftarmenian" },         // Character 'ՙ' Letter
    std::pair<QChar, const char*>{ QChar(0x031C), "ringhalfleftbelowcmb" },         // Character '̜' Mark
    std::pair<QChar, const char*>{ QChar(0x02D3), "ringhalfleftcentered" },         // Character '˓' Symbol
    std::pair<QChar, const char*>{ QChar(0x02BE), "ringhalfright" },                // Character 'ʾ' Letter
    std::pair<QChar, const char*>{ QChar(0x0339), "ringhalfrightbelowcmb" },        // Character '̹' Mark
    std::pair<QChar, const char*>{ QChar(0x02D2), "ringhalfrightcentered" },        // Character '˒' Symbol
    std::pair<QChar, const char*>{ QChar(0x0213), "rinvertedbreve" },               // Character 'ȓ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3351), "rittorusquare" },                // Character '㍑' Symbol
    std::pair<QChar, const char*>{ QChar(0x1E5F), "rlinebelow" },                   // Character 'ṟ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x027C), "rlongleg" },                     // Character 'ɼ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x027A), "rlonglegturned" },               // Character 'ɺ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0xFF52), "rmonospace" },                   // Character 'ｒ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x308D), "rohiragana" },                   // Character 'ろ' Letter
    std::pair<QChar, const char*>{ QChar(0x30ED), "rokatakana" },                   // Character 'ロ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF9B), "rokatakanahalfwidth" },          // Character 'ﾛ' Letter
    std::pair<QChar, const char*>{ QChar(0x0E23), "roruathai" },                    // Character 'ร' Letter
    std::pair<QChar, const char*>{ QChar(0x24AD), "rparen" },                       // Character '⒭' Symbol
    std::pair<QChar, const char*>{ QChar(0x09DC), "rrabengali" },                   // Character 'ড়' Letter
    std::pair<QChar, const char*>{ QChar(0x0931), "rradeva" },                      // Character 'ऱ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A5C), "rragurmukhi" },                  // Character 'ੜ' Letter
    std::pair<QChar, const char*>{ QChar(0x0691), "rreharabic" },                   // Character 'ڑ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB8D), "rrehfinalarabic" },              // Character 'ﮍ' Letter
    std::pair<QChar, const char*>{ QChar(0x09E0), "rrvocalicbengali" },             // Character 'ৠ' Letter
    std::pair<QChar, const char*>{ QChar(0x0960), "rrvocalicdeva" },                // Character 'ॠ' Letter
    std::pair<QChar, const char*>{ QChar(0x0AE0), "rrvocalicgujarati" },            // Character 'ૠ' Letter
    std::pair<QChar, const char*>{ QChar(0x09C4), "rrvocalicvowelsignbengali" },    // Character 'ৄ' Mark
    std::pair<QChar, const char*>{ QChar(0x0944), "rrvocalicvowelsigndeva" },       // Character 'ॄ' Mark
    std::pair<QChar, const char*>{ QChar(0x0AC4), "rrvocalicvowelsigngujarati" },   // Character 'ૄ' Mark
    std::pair<QChar, const char*>{ QChar(0xF6F1), "rsuperior" },                    //
    std::pair<QChar, const char*>{ QChar(0x2590), "rtblock" },                      // Character '▐' Symbol
    std::pair<QChar, const char*>{ QChar(0x0279), "rturned" },                      // Character 'ɹ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x02B4), "rturnedsuperior" },              // Character 'ʴ' Letter
    std::pair<QChar, const char*>{ QChar(0x308B), "ruhiragana" },                   // Character 'る' Letter
    std::pair<QChar, const char*>{ QChar(0x30EB), "rukatakana" },                   // Character 'ル' Letter
    std::pair<QChar, const char*>{ QChar(0xFF99), "rukatakanahalfwidth" },          // Character 'ﾙ' Letter
    std::pair<QChar, const char*>{ QChar(0x09F2), "rupeemarkbengali" },             // Character '৲' Symbol
    std::pair<QChar, const char*>{ QChar(0x09F3), "rupeesignbengali" },             // Character '৳' Symbol
    std::pair<QChar, const char*>{ QChar(0xF6DD), "rupiah" },                       //
    std::pair<QChar, const char*>{ QChar(0x0E24), "ruthai" },                       // Character 'ฤ' Letter
    std::pair<QChar, const char*>{ QChar(0x098B), "rvocalicbengali" },              // Character 'ঋ' Letter
    std::pair<QChar, const char*>{ QChar(0x090B), "rvocalicdeva" },                 // Character 'ऋ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A8B), "rvocalicgujarati" },             // Character 'ઋ' Letter
    std::pair<QChar, const char*>{ QChar(0x09C3), "rvocalicvowelsignbengali" },     // Character 'ৃ' Mark
    std::pair<QChar, const char*>{ QChar(0x0943), "rvocalicvowelsigndeva" },        // Character 'ृ' Mark
    std::pair<QChar, const char*>{ QChar(0x0AC3), "rvocalicvowelsigngujarati" },    // Character 'ૃ' Mark
    std::pair<QChar, const char*>{ QChar(0x0073), "s" },                            // Character 's' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x09B8), "sabengali" },                    // Character 'স' Letter
    std::pair<QChar, const char*>{ QChar(0x015B), "sacute" },                       // Character 'ś' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E65), "sacutedotaccent" },              // Character 'ṥ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0635), "sadarabic" },                    // Character 'ص' Letter
    std::pair<QChar, const char*>{ QChar(0x0938), "sadeva" },                       // Character 'स' Letter
    std::pair<QChar, const char*>{ QChar(0xFEBA), "sadfinalarabic" },               // Character 'ﺺ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEBB), "sadinitialarabic" },             // Character 'ﺻ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEBC), "sadmedialarabic" },              // Character 'ﺼ' Letter
    std::pair<QChar, const char*>{ QChar(0x0AB8), "sagujarati" },                   // Character 'સ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A38), "sagurmukhi" },                   // Character 'ਸ' Letter
    std::pair<QChar, const char*>{ QChar(0x3055), "sahiragana" },                   // Character 'さ' Letter
    std::pair<QChar, const char*>{ QChar(0x30B5), "sakatakana" },                   // Character 'サ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF7B), "sakatakanahalfwidth" },          // Character 'ｻ' Letter
    std::pair<QChar, const char*>{ QChar(0xFDFA), "sallallahoualayhewasallamarabic" },// Character 'ﷺ' Letter
    std::pair<QChar, const char*>{ QChar(0x05E1), "samekh" },                       // Character 'ס' Letter
    std::pair<QChar, const char*>{ QChar(0xFB41), "samekhdagesh" },                 // Character 'סּ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB41), "samekhdageshhebrew" },           // Character 'סּ' Letter
    std::pair<QChar, const char*>{ QChar(0x05E1), "samekhhebrew" },                 // Character 'ס' Letter
    std::pair<QChar, const char*>{ QChar(0x0E32), "saraaathai" },                   // Character 'า' Letter
    std::pair<QChar, const char*>{ QChar(0x0E41), "saraaethai" },                   // Character 'แ' Letter
    std::pair<QChar, const char*>{ QChar(0x0E44), "saraaimaimalaithai" },           // Character 'ไ' Letter
    std::pair<QChar, const char*>{ QChar(0x0E43), "saraaimaimuanthai" },            // Character 'ใ' Letter
    std::pair<QChar, const char*>{ QChar(0x0E33), "saraamthai" },                   // Character 'ำ' Letter
    std::pair<QChar, const char*>{ QChar(0x0E30), "saraathai" },                    // Character 'ะ' Letter
    std::pair<QChar, const char*>{ QChar(0x0E40), "saraethai" },                    // Character 'เ' Letter
    std::pair<QChar, const char*>{ QChar(0xF886), "saraiileftthai" },               //
    std::pair<QChar, const char*>{ QChar(0x0E35), "saraiithai" },                   // Character 'ี' Mark
    std::pair<QChar, const char*>{ QChar(0xF885), "saraileftthai" },                //
    std::pair<QChar, const char*>{ QChar(0x0E34), "saraithai" },                    // Character 'ิ' Mark
    std::pair<QChar, const char*>{ QChar(0x0E42), "saraothai" },                    // Character 'โ' Letter
    std::pair<QChar, const char*>{ QChar(0xF888), "saraueeleftthai" },              //
    std::pair<QChar, const char*>{ QChar(0x0E37), "saraueethai" },                  // Character 'ื' Mark
    std::pair<QChar, const char*>{ QChar(0xF887), "saraueleftthai" },               //
    std::pair<QChar, const char*>{ QChar(0x0E36), "sarauethai" },                   // Character 'ึ' Mark
    std::pair<QChar, const char*>{ QChar(0x0E38), "sarauthai" },                    // Character 'ุ' Mark
    std::pair<QChar, const char*>{ QChar(0x0E39), "sarauuthai" },                   // Character 'ู' Mark
    std::pair<QChar, const char*>{ QChar(0x3119), "sbopomofo" },                    // Character 'ㄙ' Letter
    std::pair<QChar, const char*>{ QChar(0x0161), "scaron" },                       // Character 'š' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E67), "scarondotaccent" },              // Character 'ṧ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x015F), "scedilla" },                     // Character 'ş' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0259), "schwa" },                        // Character 'ə' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x04D9), "schwacyrillic" },                // Character 'ә' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x04DB), "schwadieresiscyrillic" },        // Character 'ӛ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x025A), "schwahook" },                    // Character 'ɚ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x24E2), "scircle" },                      // Character 'ⓢ' Symbol
    std::pair<QChar, const char*>{ QChar(0x015D), "scircumflex" },                  // Character 'ŝ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0219), "scommaaccent" },                 // Character 'ș' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E61), "sdotaccent" },                   // Character 'ṡ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E63), "sdotbelow" },                    // Character 'ṣ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E69), "sdotbelowdotaccent" },           // Character 'ṩ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x033C), "seagullbelowcmb" },              // Character '̼' Mark
    std::pair<QChar, const char*>{ QChar(0x2033), "second" },                       // Character '″' Punctuation
    std::pair<QChar, const char*>{ QChar(0x02CA), "secondtonechinese" },            // Character 'ˊ' Letter
    std::pair<QChar, const char*>{ QChar(0x00A7), "section" },                      // Character '§' Punctuation
    std::pair<QChar, const char*>{ QChar(0x0633), "seenarabic" },                   // Character 'س' Letter
    std::pair<QChar, const char*>{ QChar(0xFEB2), "seenfinalarabic" },              // Character 'ﺲ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEB3), "seeninitialarabic" },            // Character 'ﺳ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEB4), "seenmedialarabic" },             // Character 'ﺴ' Letter
    std::pair<QChar, const char*>{ QChar(0x05B6), "segol" },                        // Character 'ֶ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B6), "segol13" },                      // Character 'ֶ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B6), "segol1f" },                      // Character 'ֶ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B6), "segol2c" },                      // Character 'ֶ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B6), "segolhebrew" },                  // Character 'ֶ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B6), "segolnarrowhebrew" },            // Character 'ֶ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B6), "segolquarterhebrew" },           // Character 'ֶ' Mark
    std::pair<QChar, const char*>{ QChar(0x0592), "segoltahebrew" },                // Character '֒' Mark
    std::pair<QChar, const char*>{ QChar(0x05B6), "segolwidehebrew" },              // Character 'ֶ' Mark
    std::pair<QChar, const char*>{ QChar(0x057D), "seharmenian" },                  // Character 'ս' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x305B), "sehiragana" },                   // Character 'せ' Letter
    std::pair<QChar, const char*>{ QChar(0x30BB), "sekatakana" },                   // Character 'セ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF7E), "sekatakanahalfwidth" },          // Character 'ｾ' Letter
    std::pair<QChar, const char*>{ QChar(0x003B), "semicolon" },                    // Character ';' Punctuation
    std::pair<QChar, const char*>{ QChar(0x061B), "semicolonarabic" },              // Character '؛' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFF1B), "semicolonmonospace" },           // Character '；' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFE54), "semicolonsmall" },               // Character '﹔' Punctuation
    std::pair<QChar, const char*>{ QChar(0x309C), "semivoicedmarkkana" },           // Character '゜' Symbol
    std::pair<QChar, const char*>{ QChar(0xFF9F), "semivoicedmarkkanahalfwidth" },  // Character 'ﾟ' Letter
    std::pair<QChar, const char*>{ QChar(0x3322), "sentisquare" },                  // Character '㌢' Symbol
    std::pair<QChar, const char*>{ QChar(0x3323), "sentosquare" },                  // Character '㌣' Symbol
    std::pair<QChar, const char*>{ QChar(0x0037), "seven" },                        // Character '7' Digit
    std::pair<QChar, const char*>{ QChar(0x0667), "sevenarabic" },                  // Character '٧' Digit
    std::pair<QChar, const char*>{ QChar(0x09ED), "sevenbengali" },                 // Character '৭' Digit
    std::pair<QChar, const char*>{ QChar(0x2466), "sevencircle" },                  // Character '⑦'
    std::pair<QChar, const char*>{ QChar(0x2790), "sevencircleinversesansserif" },  // Character '➐'
    std::pair<QChar, const char*>{ QChar(0x096D), "sevendeva" },                    // Character '७' Digit
    std::pair<QChar, const char*>{ QChar(0x215E), "seveneighths" },                 // Character '⅞'
    std::pair<QChar, const char*>{ QChar(0x0AED), "sevengujarati" },                // Character '૭' Digit
    std::pair<QChar, const char*>{ QChar(0x0A6D), "sevengurmukhi" },                // Character '੭' Digit
    std::pair<QChar, const char*>{ QChar(0x0667), "sevenhackarabic" },              // Character '٧' Digit
    std::pair<QChar, const char*>{ QChar(0x3027), "sevenhangzhou" },                // Character '〧'
    std::pair<QChar, const char*>{ QChar(0x3226), "sevenideographicparen" },        // Character '㈦'
    std::pair<QChar, const char*>{ QChar(0x2087), "seveninferior" },                // Character '₇'
    std::pair<QChar, const char*>{ QChar(0xFF17), "sevenmonospace" },               // Character '７' Digit
    std::pair<QChar, const char*>{ QChar(0xF737), "sevenoldstyle" },                //
    std::pair<QChar, const char*>{ QChar(0x247A), "sevenparen" },                   // Character '⑺'
    std::pair<QChar, const char*>{ QChar(0x248E), "sevenperiod" },                  // Character '⒎'
    std::pair<QChar, const char*>{ QChar(0x06F7), "sevenpersian" },                 // Character '۷' Digit
    std::pair<QChar, const char*>{ QChar(0x2176), "sevenroman" },                   // Character 'ⅶ'
    std::pair<QChar, const char*>{ QChar(0x2077), "sevensuperior" },                // Character '⁷'
    std::pair<QChar, const char*>{ QChar(0x2470), "seventeencircle" },              // Character '⑰'
    std::pair<QChar, const char*>{ QChar(0x2484), "seventeenparen" },               // Character '⒄'
    std::pair<QChar, const char*>{ QChar(0x2498), "seventeenperiod" },              // Character '⒘'
    std::pair<QChar, const char*>{ QChar(0x0E57), "seventhai" },                    // Character '๗' Digit
    std::pair<QChar, const char*>{ QChar(0x00AD), "sfthyphen" },                    //
    std::pair<QChar, const char*>{ QChar(0x0577), "shaarmenian" },                  // Character 'շ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x09B6), "shabengali" },                   // Character 'শ' Letter
    std::pair<QChar, const char*>{ QChar(0x0448), "shacyrillic" },                  // Character 'ш' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0651), "shaddaarabic" },                 // Character 'ّ' Mark
    std::pair<QChar, const char*>{ QChar(0xFC61), "shaddadammaarabic" },            // Character 'ﱡ' Letter
    std::pair<QChar, const char*>{ QChar(0xFC5E), "shaddadammatanarabic" },         // Character 'ﱞ' Letter
    std::pair<QChar, const char*>{ QChar(0xFC60), "shaddafathaarabic" },            // Character 'ﱠ' Letter
    std::pair<QChar, const char*>{ QChar(0x064B), "shaddafathatanarabic" },         // Character 'ً' Mark
    std::pair<QChar, const char*>{ QChar(0xFC62), "shaddakasraarabic" },            // Character 'ﱢ' Letter
    std::pair<QChar, const char*>{ QChar(0xFC5F), "shaddakasratanarabic" },         // Character 'ﱟ' Letter
    std::pair<QChar, const char*>{ QChar(0x2592), "shade" },                        // Character '▒' Symbol
    std::pair<QChar, const char*>{ QChar(0x2593), "shadedark" },                    // Character '▓' Symbol
    std::pair<QChar, const char*>{ QChar(0x2591), "shadelight" },                   // Character '░' Symbol
    std::pair<QChar, const char*>{ QChar(0x2592), "shademedium" },                  // Character '▒' Symbol
    std::pair<QChar, const char*>{ QChar(0x0936), "shadeva" },                      // Character 'श' Letter
    std::pair<QChar, const char*>{ QChar(0x0AB6), "shagujarati" },                  // Character 'શ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A36), "shagurmukhi" },                  // Character 'ਸ਼' Letter
    std::pair<QChar, const char*>{ QChar(0x0593), "shalshelethebrew" },             // Character '֓' Mark
    std::pair<QChar, const char*>{ QChar(0x3115), "shbopomofo" },                   // Character 'ㄕ' Letter
    std::pair<QChar, const char*>{ QChar(0x0449), "shchacyrillic" },                // Character 'щ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0634), "sheenarabic" },                  // Character 'ش' Letter
    std::pair<QChar, const char*>{ QChar(0xFEB6), "sheenfinalarabic" },             // Character 'ﺶ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEB7), "sheeninitialarabic" },           // Character 'ﺷ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEB8), "sheenmedialarabic" },            // Character 'ﺸ' Letter
    std::pair<QChar, const char*>{ QChar(0x03E3), "sheicoptic" },                   // Character 'ϣ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x20AA), "sheqel" },                       // Character '₪' Symbol
    std::pair<QChar, const char*>{ QChar(0x20AA), "sheqelhebrew" },                 // Character '₪' Symbol
    std::pair<QChar, const char*>{ QChar(0x05B0), "sheva" },                        // Character 'ְ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B0), "sheva115" },                     // Character 'ְ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B0), "sheva15" },                      // Character 'ְ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B0), "sheva22" },                      // Character 'ְ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B0), "sheva2e" },                      // Character 'ְ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B0), "shevahebrew" },                  // Character 'ְ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B0), "shevanarrowhebrew" },            // Character 'ְ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B0), "shevaquarterhebrew" },           // Character 'ְ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B0), "shevawidehebrew" },              // Character 'ְ' Mark
    std::pair<QChar, const char*>{ QChar(0x04BB), "shhacyrillic" },                 // Character 'һ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x03ED), "shimacoptic" },                  // Character 'ϭ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x05E9), "shin" },                         // Character 'ש' Letter
    std::pair<QChar, const char*>{ QChar(0xFB49), "shindagesh" },                   // Character 'שּ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB49), "shindageshhebrew" },             // Character 'שּ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB2C), "shindageshshindot" },            // Character 'שּׁ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB2C), "shindageshshindothebrew" },      // Character 'שּׁ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB2D), "shindageshsindot" },             // Character 'שּׂ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB2D), "shindageshsindothebrew" },       // Character 'שּׂ' Letter
    std::pair<QChar, const char*>{ QChar(0x05C1), "shindothebrew" },                // Character 'ׁ' Mark
    std::pair<QChar, const char*>{ QChar(0x05E9), "shinhebrew" },                   // Character 'ש' Letter
    std::pair<QChar, const char*>{ QChar(0xFB2A), "shinshindot" },                  // Character 'שׁ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB2A), "shinshindothebrew" },            // Character 'שׁ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB2B), "shinsindot" },                   // Character 'שׂ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB2B), "shinsindothebrew" },             // Character 'שׂ' Letter
    std::pair<QChar, const char*>{ QChar(0x0282), "shook" },                        // Character 'ʂ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x03C3), "sigma" },                        // Character 'σ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x03C2), "sigma1" },                       // Character 'ς' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x03C2), "sigmafinal" },                   // Character 'ς' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x03F2), "sigmalunatesymbolgreek" },       // Character 'ϲ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3057), "sihiragana" },                   // Character 'し' Letter
    std::pair<QChar, const char*>{ QChar(0x30B7), "sikatakana" },                   // Character 'シ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF7C), "sikatakanahalfwidth" },          // Character 'ｼ' Letter
    std::pair<QChar, const char*>{ QChar(0x05BD), "siluqhebrew" },                  // Character 'ֽ' Mark
    std::pair<QChar, const char*>{ QChar(0x05BD), "siluqlefthebrew" },              // Character 'ֽ' Mark
    std::pair<QChar, const char*>{ QChar(0x223C), "similar" },                      // Character '∼' Symbol
    std::pair<QChar, const char*>{ QChar(0x05C2), "sindothebrew" },                 // Character 'ׂ' Mark
    std::pair<QChar, const char*>{ QChar(0x3274), "siosacirclekorean" },            // Character '㉴' Symbol
    std::pair<QChar, const char*>{ QChar(0x3214), "siosaparenkorean" },             // Character '㈔' Symbol
    std::pair<QChar, const char*>{ QChar(0x317E), "sioscieuckorean" },              // Character 'ㅾ' Letter
    std::pair<QChar, const char*>{ QChar(0x3266), "sioscirclekorean" },             // Character '㉦' Symbol
    std::pair<QChar, const char*>{ QChar(0x317A), "sioskiyeokkorean" },             // Character 'ㅺ' Letter
    std::pair<QChar, const char*>{ QChar(0x3145), "sioskorean" },                   // Character 'ㅅ' Letter
    std::pair<QChar, const char*>{ QChar(0x317B), "siosnieunkorean" },              // Character 'ㅻ' Letter
    std::pair<QChar, const char*>{ QChar(0x3206), "siosparenkorean" },              // Character '㈆' Symbol
    std::pair<QChar, const char*>{ QChar(0x317D), "siospieupkorean" },              // Character 'ㅽ' Letter
    std::pair<QChar, const char*>{ QChar(0x317C), "siostikeutkorean" },             // Character 'ㅼ' Letter
    std::pair<QChar, const char*>{ QChar(0x0036), "six" },                          // Character '6' Digit
    std::pair<QChar, const char*>{ QChar(0x0666), "sixarabic" },                    // Character '٦' Digit
    std::pair<QChar, const char*>{ QChar(0x09EC), "sixbengali" },                   // Character '৬' Digit
    std::pair<QChar, const char*>{ QChar(0x2465), "sixcircle" },                    // Character '⑥'
    std::pair<QChar, const char*>{ QChar(0x278F), "sixcircleinversesansserif" },    // Character '➏'
    std::pair<QChar, const char*>{ QChar(0x096C), "sixdeva" },                      // Character '६' Digit
    std::pair<QChar, const char*>{ QChar(0x0AEC), "sixgujarati" },                  // Character '૬' Digit
    std::pair<QChar, const char*>{ QChar(0x0A6C), "sixgurmukhi" },                  // Character '੬' Digit
    std::pair<QChar, const char*>{ QChar(0x0666), "sixhackarabic" },                // Character '٦' Digit
    std::pair<QChar, const char*>{ QChar(0x3026), "sixhangzhou" },                  // Character '〦'
    std::pair<QChar, const char*>{ QChar(0x3225), "sixideographicparen" },          // Character '㈥'
    std::pair<QChar, const char*>{ QChar(0x2086), "sixinferior" },                  // Character '₆'
    std::pair<QChar, const char*>{ QChar(0xFF16), "sixmonospace" },                 // Character '６' Digit
    std::pair<QChar, const char*>{ QChar(0xF736), "sixoldstyle" },                  //
    std::pair<QChar, const char*>{ QChar(0x2479), "sixparen" },                     // Character '⑹'
    std::pair<QChar, const char*>{ QChar(0x248D), "sixperiod" },                    // Character '⒍'
    std::pair<QChar, const char*>{ QChar(0x06F6), "sixpersian" },                   // Character '۶' Digit
    std::pair<QChar, const char*>{ QChar(0x2175), "sixroman" },                     // Character 'ⅵ'
    std::pair<QChar, const char*>{ QChar(0x2076), "sixsuperior" },                  // Character '⁶'
    std::pair<QChar, const char*>{ QChar(0x246F), "sixteencircle" },                // Character '⑯'
    std::pair<QChar, const char*>{ QChar(0x09F9), "sixteencurrencydenominatorbengali" },// Character '৹'
    std::pair<QChar, const char*>{ QChar(0x2483), "sixteenparen" },                 // Character '⒃'
    std::pair<QChar, const char*>{ QChar(0x2497), "sixteenperiod" },                // Character '⒗'
    std::pair<QChar, const char*>{ QChar(0x0E56), "sixthai" },                      // Character '๖' Digit
    std::pair<QChar, const char*>{ QChar(0x002F), "slash" },                        // Character '/' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFF0F), "slashmonospace" },               // Character '／' Punctuation
    std::pair<QChar, const char*>{ QChar(0x017F), "slong" },                        // Character 'ſ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E9B), "slongdotaccent" },               // Character 'ẛ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x263A), "smileface" },                    // Character '☺' Symbol
    std::pair<QChar, const char*>{ QChar(0xFF53), "smonospace" },                   // Character 'ｓ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x05C3), "sofpasuqhebrew" },               // Character '׃' Punctuation
    std::pair<QChar, const char*>{ QChar(0x00AD), "softhyphen" },                   //
    std::pair<QChar, const char*>{ QChar(0x044C), "softsigncyrillic" },             // Character 'ь' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x305D), "sohiragana" },                   // Character 'そ' Letter
    std::pair<QChar, const char*>{ QChar(0x30BD), "sokatakana" },                   // Character 'ソ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF7F), "sokatakanahalfwidth" },          // Character 'ｿ' Letter
    std::pair<QChar, const char*>{ QChar(0x0338), "soliduslongoverlaycmb" },        // Character '̸' Mark
    std::pair<QChar, const char*>{ QChar(0x0337), "solidusshortoverlaycmb" },       // Character '̷' Mark
    std::pair<QChar, const char*>{ QChar(0x0E29), "sorusithai" },                   // Character 'ษ' Letter
    std::pair<QChar, const char*>{ QChar(0x0E28), "sosalathai" },                   // Character 'ศ' Letter
    std::pair<QChar, const char*>{ QChar(0x0E0B), "sosothai" },                     // Character 'ซ' Letter
    std::pair<QChar, const char*>{ QChar(0x0E2A), "sosuathai" },                    // Character 'ส' Letter
    std::pair<QChar, const char*>{ QChar(0x0020), "space" },                        // Character ' ' Whitespace
    std::pair<QChar, const char*>{ QChar(0x0020), "spacehackarabic" },              // Character ' ' Whitespace
    std::pair<QChar, const char*>{ QChar(0x2660), "spade" },                        // Character '♠' Symbol
    std::pair<QChar, const char*>{ QChar(0x2660), "spadesuitblack" },               // Character '♠' Symbol
    std::pair<QChar, const char*>{ QChar(0x2664), "spadesuitwhite" },               // Character '♤' Symbol
    std::pair<QChar, const char*>{ QChar(0x24AE), "sparen" },                       // Character '⒮' Symbol
    std::pair<QChar, const char*>{ QChar(0x033B), "squarebelowcmb" },               // Character '̻' Mark
    std::pair<QChar, const char*>{ QChar(0x33C4), "squarecc" },                     // Character '㏄' Symbol
    std::pair<QChar, const char*>{ QChar(0x339D), "squarecm" },                     // Character '㎝' Symbol
    std::pair<QChar, const char*>{ QChar(0x25A9), "squarediagonalcrosshatchfill" }, // Character '▩' Symbol
    std::pair<QChar, const char*>{ QChar(0x25A4), "squarehorizontalfill" },         // Character '▤' Symbol
    std::pair<QChar, const char*>{ QChar(0x338F), "squarekg" },                     // Character '㎏' Symbol
    std::pair<QChar, const char*>{ QChar(0x339E), "squarekm" },                     // Character '㎞' Symbol
    std::pair<QChar, const char*>{ QChar(0x33CE), "squarekmcapital" },              // Character '㏎' Symbol
    std::pair<QChar, const char*>{ QChar(0x33D1), "squareln" },                     // Character '㏑' Symbol
    std::pair<QChar, const char*>{ QChar(0x33D2), "squarelog" },                    // Character '㏒' Symbol
    std::pair<QChar, const char*>{ QChar(0x338E), "squaremg" },                     // Character '㎎' Symbol
    std::pair<QChar, const char*>{ QChar(0x33D5), "squaremil" },                    // Character '㏕' Symbol
    std::pair<QChar, const char*>{ QChar(0x339C), "squaremm" },                     // Character '㎜' Symbol
    std::pair<QChar, const char*>{ QChar(0x33A1), "squaremsquared" },               // Character '㎡' Symbol
    std::pair<QChar, const char*>{ QChar(0x25A6), "squareorthogonalcrosshatchfill" },// Character '▦' Symbol
    std::pair<QChar, const char*>{ QChar(0x25A7), "squareupperlefttolowerrightfill" },// Character '▧' Symbol
    std::pair<QChar, const char*>{ QChar(0x25A8), "squareupperrighttolowerleftfill" },// Character '▨' Symbol
    std::pair<QChar, const char*>{ QChar(0x25A5), "squareverticalfill" },           // Character '▥' Symbol
    std::pair<QChar, const char*>{ QChar(0x25A3), "squarewhitewithsmallblack" },    // Character '▣' Symbol
    std::pair<QChar, const char*>{ QChar(0x33DB), "srsquare" },                     // Character '㏛' Symbol
    std::pair<QChar, const char*>{ QChar(0x09B7), "ssabengali" },                   // Character 'ষ' Letter
    std::pair<QChar, const char*>{ QChar(0x0937), "ssadeva" },                      // Character 'ष' Letter
    std::pair<QChar, const char*>{ QChar(0x0AB7), "ssagujarati" },                  // Character 'ષ' Letter
    std::pair<QChar, const char*>{ QChar(0x3149), "ssangcieuckorean" },             // Character 'ㅉ' Letter
    std::pair<QChar, const char*>{ QChar(0x3185), "ssanghieuhkorean" },             // Character 'ㆅ' Letter
    std::pair<QChar, const char*>{ QChar(0x3180), "ssangieungkorean" },             // Character 'ㆀ' Letter
    std::pair<QChar, const char*>{ QChar(0x3132), "ssangkiyeokkorean" },            // Character 'ㄲ' Letter
    std::pair<QChar, const char*>{ QChar(0x3165), "ssangnieunkorean" },             // Character 'ㅥ' Letter
    std::pair<QChar, const char*>{ QChar(0x3143), "ssangpieupkorean" },             // Character 'ㅃ' Letter
    std::pair<QChar, const char*>{ QChar(0x3146), "ssangsioskorean" },              // Character 'ㅆ' Letter
    std::pair<QChar, const char*>{ QChar(0x3138), "ssangtikeutkorean" },            // Character 'ㄸ' Letter
    std::pair<QChar, const char*>{ QChar(0xF6F2), "ssuperior" },                    //
    std::pair<QChar, const char*>{ QChar(0x00A3), "sterling" },                     // Character '£' Symbol
    std::pair<QChar, const char*>{ QChar(0xFFE1), "sterlingmonospace" },            // Character '￡' Symbol
    std::pair<QChar, const char*>{ QChar(0x0336), "strokelongoverlaycmb" },         // Character '̶' Mark
    std::pair<QChar, const char*>{ QChar(0x0335), "strokeshortoverlaycmb" },        // Character '̵' Mark
    std::pair<QChar, const char*>{ QChar(0x2282), "subset" },                       // Character '⊂' Symbol
    std::pair<QChar, const char*>{ QChar(0x228A), "subsetnotequal" },               // Character '⊊' Symbol
    std::pair<QChar, const char*>{ QChar(0x2286), "subsetorequal" },                // Character '⊆' Symbol
    std::pair<QChar, const char*>{ QChar(0x227B), "succeeds" },                     // Character '≻' Symbol
    std::pair<QChar, const char*>{ QChar(0x220B), "suchthat" },                     // Character '∋' Symbol
    std::pair<QChar, const char*>{ QChar(0x3059), "suhiragana" },                   // Character 'す' Letter
    std::pair<QChar, const char*>{ QChar(0x30B9), "sukatakana" },                   // Character 'ス' Letter
    std::pair<QChar, const char*>{ QChar(0xFF7D), "sukatakanahalfwidth" },          // Character 'ｽ' Letter
    std::pair<QChar, const char*>{ QChar(0x0652), "sukunarabic" },                  // Character 'ْ' Mark
    std::pair<QChar, const char*>{ QChar(0x2211), "summation" },                    // Character '∑' Symbol
    std::pair<QChar, const char*>{ QChar(0x263C), "sun" },                          // Character '☼' Symbol
    std::pair<QChar, const char*>{ QChar(0x2283), "superset" },                     // Character '⊃' Symbol
    std::pair<QChar, const char*>{ QChar(0x228B), "supersetnotequal" },             // Character '⊋' Symbol
    std::pair<QChar, const char*>{ QChar(0x2287), "supersetorequal" },              // Character '⊇' Symbol
    std::pair<QChar, const char*>{ QChar(0x33DC), "svsquare" },                     // Character '㏜' Symbol
    std::pair<QChar, const char*>{ QChar(0x337C), "syouwaerasquare" },              // Character '㍼' Symbol
    std::pair<QChar, const char*>{ QChar(0x0074), "t" },                            // Character 't' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x09A4), "tabengali" },                    // Character 'ত' Letter
    std::pair<QChar, const char*>{ QChar(0x22A4), "tackdown" },                     // Character '⊤' Symbol
    std::pair<QChar, const char*>{ QChar(0x22A3), "tackleft" },                     // Character '⊣' Symbol
    std::pair<QChar, const char*>{ QChar(0x0924), "tadeva" },                       // Character 'त' Letter
    std::pair<QChar, const char*>{ QChar(0x0AA4), "tagujarati" },                   // Character 'ત' Letter
    std::pair<QChar, const char*>{ QChar(0x0A24), "tagurmukhi" },                   // Character 'ਤ' Letter
    std::pair<QChar, const char*>{ QChar(0x0637), "taharabic" },                    // Character 'ط' Letter
    std::pair<QChar, const char*>{ QChar(0xFEC2), "tahfinalarabic" },               // Character 'ﻂ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEC3), "tahinitialarabic" },             // Character 'ﻃ' Letter
    std::pair<QChar, const char*>{ QChar(0x305F), "tahiragana" },                   // Character 'た' Letter
    std::pair<QChar, const char*>{ QChar(0xFEC4), "tahmedialarabic" },              // Character 'ﻄ' Letter
    std::pair<QChar, const char*>{ QChar(0x337D), "taisyouerasquare" },             // Character '㍽' Symbol
    std::pair<QChar, const char*>{ QChar(0x30BF), "takatakana" },                   // Character 'タ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF80), "takatakanahalfwidth" },          // Character 'ﾀ' Letter
    std::pair<QChar, const char*>{ QChar(0x0640), "tatweelarabic" },                // Character 'ـ' Letter
    std::pair<QChar, const char*>{ QChar(0x03C4), "tau" },                          // Character 'τ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x05EA), "tav" },                          // Character 'ת' Letter
    std::pair<QChar, const char*>{ QChar(0xFB4A), "tavdages" },                     // Character 'תּ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB4A), "tavdagesh" },                    // Character 'תּ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB4A), "tavdageshhebrew" },              // Character 'תּ' Letter
    std::pair<QChar, const char*>{ QChar(0x05EA), "tavhebrew" },                    // Character 'ת' Letter
    std::pair<QChar, const char*>{ QChar(0x0167), "tbar" },                         // Character 'ŧ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x310A), "tbopomofo" },                    // Character 'ㄊ' Letter
    std::pair<QChar, const char*>{ QChar(0x0165), "tcaron" },                       // Character 'ť' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x02A8), "tccurl" },                       // Character 'ʨ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0163), "tcedilla" },                     // Character 'ţ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0686), "tcheharabic" },                  // Character 'چ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB7B), "tchehfinalarabic" },             // Character 'ﭻ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB7C), "tchehinitialarabic" },           // Character 'ﭼ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB7D), "tchehmedialarabic" },            // Character 'ﭽ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEE4), "tchehmeeminitialarabic" },       // Character 'ﻤ' Letter
    std::pair<QChar, const char*>{ QChar(0x24E3), "tcircle" },                      // Character 'ⓣ' Symbol
    std::pair<QChar, const char*>{ QChar(0x1E71), "tcircumflexbelow" },             // Character 'ṱ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0163), "tcommaaccent" },                 // Character 'ţ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E97), "tdieresis" },                    // Character 'ẗ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E6B), "tdotaccent" },                   // Character 'ṫ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E6D), "tdotbelow" },                    // Character 'ṭ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0442), "tecyrillic" },                   // Character 'т' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x04AD), "tedescendercyrillic" },          // Character 'ҭ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x062A), "teharabic" },                    // Character 'ت' Letter
    std::pair<QChar, const char*>{ QChar(0xFE96), "tehfinalarabic" },               // Character 'ﺖ' Letter
    std::pair<QChar, const char*>{ QChar(0xFCA2), "tehhahinitialarabic" },          // Character 'ﲢ' Letter
    std::pair<QChar, const char*>{ QChar(0xFC0C), "tehhahisolatedarabic" },         // Character 'ﰌ' Letter
    std::pair<QChar, const char*>{ QChar(0xFE97), "tehinitialarabic" },             // Character 'ﺗ' Letter
    std::pair<QChar, const char*>{ QChar(0x3066), "tehiragana" },                   // Character 'て' Letter
    std::pair<QChar, const char*>{ QChar(0xFCA1), "tehjeeminitialarabic" },         // Character 'ﲡ' Letter
    std::pair<QChar, const char*>{ QChar(0xFC0B), "tehjeemisolatedarabic" },        // Character 'ﰋ' Letter
    std::pair<QChar, const char*>{ QChar(0x0629), "tehmarbutaarabic" },             // Character 'ة' Letter
    std::pair<QChar, const char*>{ QChar(0xFE94), "tehmarbutafinalarabic" },        // Character 'ﺔ' Letter
    std::pair<QChar, const char*>{ QChar(0xFE98), "tehmedialarabic" },              // Character 'ﺘ' Letter
    std::pair<QChar, const char*>{ QChar(0xFCA4), "tehmeeminitialarabic" },         // Character 'ﲤ' Letter
    std::pair<QChar, const char*>{ QChar(0xFC0E), "tehmeemisolatedarabic" },        // Character 'ﰎ' Letter
    std::pair<QChar, const char*>{ QChar(0xFC73), "tehnoonfinalarabic" },           // Character 'ﱳ' Letter
    std::pair<QChar, const char*>{ QChar(0x30C6), "tekatakana" },                   // Character 'テ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF83), "tekatakanahalfwidth" },          // Character 'ﾃ' Letter
    std::pair<QChar, const char*>{ QChar(0x2121), "telephone" },                    // Character '℡' Symbol
    std::pair<QChar, const char*>{ QChar(0x260E), "telephoneblack" },               // Character '☎' Symbol
    std::pair<QChar, const char*>{ QChar(0x05A0), "telishagedolahebrew" },          // Character '֠' Mark
    std::pair<QChar, const char*>{ QChar(0x05A9), "telishaqetanahebrew" },          // Character '֩' Mark
    std::pair<QChar, const char*>{ QChar(0x2469), "tencircle" },                    // Character '⑩'
    std::pair<QChar, const char*>{ QChar(0x3229), "tenideographicparen" },          // Character '㈩'
    std::pair<QChar, const char*>{ QChar(0x247D), "tenparen" },                     // Character '⑽'
    std::pair<QChar, const char*>{ QChar(0x2491), "tenperiod" },                    // Character '⒑'
    std::pair<QChar, const char*>{ QChar(0x2179), "tenroman" },                     // Character 'ⅹ'
    std::pair<QChar, const char*>{ QChar(0x02A7), "tesh" },                         // Character 'ʧ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x05D8), "tet" },                          // Character 'ט' Letter
    std::pair<QChar, const char*>{ QChar(0xFB38), "tetdagesh" },                    // Character 'טּ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB38), "tetdageshhebrew" },              // Character 'טּ' Letter
    std::pair<QChar, const char*>{ QChar(0x05D8), "tethebrew" },                    // Character 'ט' Letter
    std::pair<QChar, const char*>{ QChar(0x04B5), "tetsecyrillic" },                // Character 'ҵ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x059B), "tevirhebrew" },                  // Character '֛' Mark
    std::pair<QChar, const char*>{ QChar(0x059B), "tevirlefthebrew" },              // Character '֛' Mark
    std::pair<QChar, const char*>{ QChar(0x09A5), "thabengali" },                   // Character 'থ' Letter
    std::pair<QChar, const char*>{ QChar(0x0925), "thadeva" },                      // Character 'थ' Letter
    std::pair<QChar, const char*>{ QChar(0x0AA5), "thagujarati" },                  // Character 'થ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A25), "thagurmukhi" },                  // Character 'ਥ' Letter
    std::pair<QChar, const char*>{ QChar(0x0630), "thalarabic" },                   // Character 'ذ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEAC), "thalfinalarabic" },              // Character 'ﺬ' Letter
    std::pair<QChar, const char*>{ QChar(0xF898), "thanthakhatlowleftthai" },       //
    std::pair<QChar, const char*>{ QChar(0xF897), "thanthakhatlowrightthai" },      //
    std::pair<QChar, const char*>{ QChar(0x0E4C), "thanthakhatthai" },              // Character '์' Mark
    std::pair<QChar, const char*>{ QChar(0xF896), "thanthakhatupperleftthai" },     //
    std::pair<QChar, const char*>{ QChar(0x062B), "theharabic" },                   // Character 'ث' Letter
    std::pair<QChar, const char*>{ QChar(0xFE9A), "thehfinalarabic" },              // Character 'ﺚ' Letter
    std::pair<QChar, const char*>{ QChar(0xFE9B), "thehinitialarabic" },            // Character 'ﺛ' Letter
    std::pair<QChar, const char*>{ QChar(0xFE9C), "thehmedialarabic" },             // Character 'ﺜ' Letter
    std::pair<QChar, const char*>{ QChar(0x2203), "thereexists" },                  // Character '∃' Symbol
    std::pair<QChar, const char*>{ QChar(0x2234), "therefore" },                    // Character '∴' Symbol
    std::pair<QChar, const char*>{ QChar(0x03B8), "theta" },                        // Character 'θ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x03D1), "theta1" },                       // Character 'ϑ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x03D1), "thetasymbolgreek" },             // Character 'ϑ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3279), "thieuthacirclekorean" },         // Character '㉹' Symbol
    std::pair<QChar, const char*>{ QChar(0x3219), "thieuthaparenkorean" },          // Character '㈙' Symbol
    std::pair<QChar, const char*>{ QChar(0x326B), "thieuthcirclekorean" },          // Character '㉫' Symbol
    std::pair<QChar, const char*>{ QChar(0x314C), "thieuthkorean" },                // Character 'ㅌ' Letter
    std::pair<QChar, const char*>{ QChar(0x320B), "thieuthparenkorean" },           // Character '㈋' Symbol
    std::pair<QChar, const char*>{ QChar(0x246C), "thirteencircle" },               // Character '⑬'
    std::pair<QChar, const char*>{ QChar(0x2480), "thirteenparen" },                // Character '⒀'
    std::pair<QChar, const char*>{ QChar(0x2494), "thirteenperiod" },               // Character '⒔'
    std::pair<QChar, const char*>{ QChar(0x0E11), "thonangmonthothai" },            // Character 'ฑ' Letter
    std::pair<QChar, const char*>{ QChar(0x01AD), "thook" },                        // Character 'ƭ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0E12), "thophuthaothai" },               // Character 'ฒ' Letter
    std::pair<QChar, const char*>{ QChar(0x00FE), "thorn" },                        // Character 'þ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0E17), "thothahanthai" },                // Character 'ท' Letter
    std::pair<QChar, const char*>{ QChar(0x0E10), "thothanthai" },                  // Character 'ฐ' Letter
    std::pair<QChar, const char*>{ QChar(0x0E18), "thothongthai" },                 // Character 'ธ' Letter
    std::pair<QChar, const char*>{ QChar(0x0E16), "thothungthai" },                 // Character 'ถ' Letter
    std::pair<QChar, const char*>{ QChar(0x0482), "thousandcyrillic" },             // Character '҂' Symbol
    std::pair<QChar, const char*>{ QChar(0x066C), "thousandsseparatorarabic" },     // Character '٬' Punctuation
    std::pair<QChar, const char*>{ QChar(0x066C), "thousandsseparatorpersian" },    // Character '٬' Punctuation
    std::pair<QChar, const char*>{ QChar(0x0033), "three" },                        // Character '3' Digit
    std::pair<QChar, const char*>{ QChar(0x0663), "threearabic" },                  // Character '٣' Digit
    std::pair<QChar, const char*>{ QChar(0x09E9), "threebengali" },                 // Character '৩' Digit
    std::pair<QChar, const char*>{ QChar(0x2462), "threecircle" },                  // Character '③'
    std::pair<QChar, const char*>{ QChar(0x278C), "threecircleinversesansserif" },  // Character '➌'
    std::pair<QChar, const char*>{ QChar(0x0969), "threedeva" },                    // Character '३' Digit
    std::pair<QChar, const char*>{ QChar(0x215C), "threeeighths" },                 // Character '⅜'
    std::pair<QChar, const char*>{ QChar(0x0AE9), "threegujarati" },                // Character '૩' Digit
    std::pair<QChar, const char*>{ QChar(0x0A69), "threegurmukhi" },                // Character '੩' Digit
    std::pair<QChar, const char*>{ QChar(0x0663), "threehackarabic" },              // Character '٣' Digit
    std::pair<QChar, const char*>{ QChar(0x3023), "threehangzhou" },                // Character '〣'
    std::pair<QChar, const char*>{ QChar(0x3222), "threeideographicparen" },        // Character '㈢'
    std::pair<QChar, const char*>{ QChar(0x2083), "threeinferior" },                // Character '₃'
    std::pair<QChar, const char*>{ QChar(0xFF13), "threemonospace" },               // Character '３' Digit
    std::pair<QChar, const char*>{ QChar(0x09F6), "threenumeratorbengali" },        // Character '৶'
    std::pair<QChar, const char*>{ QChar(0xF733), "threeoldstyle" },                //
    std::pair<QChar, const char*>{ QChar(0x2476), "threeparen" },                   // Character '⑶'
    std::pair<QChar, const char*>{ QChar(0x248A), "threeperiod" },                  // Character '⒊'
    std::pair<QChar, const char*>{ QChar(0x06F3), "threepersian" },                 // Character '۳' Digit
    std::pair<QChar, const char*>{ QChar(0x00BE), "threequarters" },                // Character '¾'
    std::pair<QChar, const char*>{ QChar(0xF6DE), "threequartersemdash" },          //
    std::pair<QChar, const char*>{ QChar(0x2172), "threeroman" },                   // Character 'ⅲ'
    std::pair<QChar, const char*>{ QChar(0x00B3), "threesuperior" },                // Character '³'
    std::pair<QChar, const char*>{ QChar(0x0E53), "threethai" },                    // Character '๓' Digit
    std::pair<QChar, const char*>{ QChar(0x3394), "thzsquare" },                    // Character '㎔' Symbol
    std::pair<QChar, const char*>{ QChar(0x3061), "tihiragana" },                   // Character 'ち' Letter
    std::pair<QChar, const char*>{ QChar(0x30C1), "tikatakana" },                   // Character 'チ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF81), "tikatakanahalfwidth" },          // Character 'ﾁ' Letter
    std::pair<QChar, const char*>{ QChar(0x3270), "tikeutacirclekorean" },          // Character '㉰' Symbol
    std::pair<QChar, const char*>{ QChar(0x3210), "tikeutaparenkorean" },           // Character '㈐' Symbol
    std::pair<QChar, const char*>{ QChar(0x3262), "tikeutcirclekorean" },           // Character '㉢' Symbol
    std::pair<QChar, const char*>{ QChar(0x3137), "tikeutkorean" },                 // Character 'ㄷ' Letter
    std::pair<QChar, const char*>{ QChar(0x3202), "tikeutparenkorean" },            // Character '㈂' Symbol
    std::pair<QChar, const char*>{ QChar(0x02DC), "tilde" },                        // Character '˜' Symbol
    std::pair<QChar, const char*>{ QChar(0x0330), "tildebelowcmb" },                // Character '̰' Mark
    std::pair<QChar, const char*>{ QChar(0x0303), "tildecmb" },                     // Character '̃' Mark
    std::pair<QChar, const char*>{ QChar(0x0303), "tildecomb" },                    // Character '̃' Mark
    std::pair<QChar, const char*>{ QChar(0x0360), "tildedoublecmb" },               // Character '͠' Mark
    std::pair<QChar, const char*>{ QChar(0x223C), "tildeoperator" },                // Character '∼' Symbol
    std::pair<QChar, const char*>{ QChar(0x0334), "tildeoverlaycmb" },              // Character '̴' Mark
    std::pair<QChar, const char*>{ QChar(0x033E), "tildeverticalcmb" },             // Character '̾' Mark
    std::pair<QChar, const char*>{ QChar(0x2297), "timescircle" },                  // Character '⊗' Symbol
    std::pair<QChar, const char*>{ QChar(0x0596), "tipehahebrew" },                 // Character '֖' Mark
    std::pair<QChar, const char*>{ QChar(0x0596), "tipehalefthebrew" },             // Character '֖' Mark
    std::pair<QChar, const char*>{ QChar(0x0A70), "tippigurmukhi" },                // Character 'ੰ' Mark
    std::pair<QChar, const char*>{ QChar(0x0483), "titlocyrilliccmb" },             // Character '҃' Mark
    std::pair<QChar, const char*>{ QChar(0x057F), "tiwnarmenian" },                 // Character 'տ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E6F), "tlinebelow" },                   // Character 'ṯ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0xFF54), "tmonospace" },                   // Character 'ｔ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0569), "toarmenian" },                   // Character 'թ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3068), "tohiragana" },                   // Character 'と' Letter
    std::pair<QChar, const char*>{ QChar(0x30C8), "tokatakana" },                   // Character 'ト' Letter
    std::pair<QChar, const char*>{ QChar(0xFF84), "tokatakanahalfwidth" },          // Character 'ﾄ' Letter
    std::pair<QChar, const char*>{ QChar(0x02E5), "tonebarextrahighmod" },          // Character '˥' Symbol
    std::pair<QChar, const char*>{ QChar(0x02E9), "tonebarextralowmod" },           // Character '˩' Symbol
    std::pair<QChar, const char*>{ QChar(0x02E6), "tonebarhighmod" },               // Character '˦' Symbol
    std::pair<QChar, const char*>{ QChar(0x02E8), "tonebarlowmod" },                // Character '˨' Symbol
    std::pair<QChar, const char*>{ QChar(0x02E7), "tonebarmidmod" },                // Character '˧' Symbol
    std::pair<QChar, const char*>{ QChar(0x01BD), "tonefive" },                     // Character 'ƽ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0185), "tonesix" },                      // Character 'ƅ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x01A8), "tonetwo" },                      // Character 'ƨ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0384), "tonos" },                        // Character '΄' Symbol
    std::pair<QChar, const char*>{ QChar(0x3327), "tonsquare" },                    // Character '㌧' Symbol
    std::pair<QChar, const char*>{ QChar(0x0E0F), "topatakthai" },                  // Character 'ฏ' Letter
    std::pair<QChar, const char*>{ QChar(0x3014), "tortoiseshellbracketleft" },     // Character '〔' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFE5D), "tortoiseshellbracketleftsmall" },// Character '﹝' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFE39), "tortoiseshellbracketleftvertical" },// Character '︹' Punctuation
    std::pair<QChar, const char*>{ QChar(0x3015), "tortoiseshellbracketright" },    // Character '〕' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFE5E), "tortoiseshellbracketrightsmall" },// Character '﹞' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFE3A), "tortoiseshellbracketrightvertical" },// Character '︺' Punctuation
    std::pair<QChar, const char*>{ QChar(0x0E15), "totaothai" },                    // Character 'ต' Letter
    std::pair<QChar, const char*>{ QChar(0x01AB), "tpalatalhook" },                 // Character 'ƫ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x24AF), "tparen" },                       // Character '⒯' Symbol
    std::pair<QChar, const char*>{ QChar(0x2122), "trademark" },                    // Character '™' Symbol
    std::pair<QChar, const char*>{ QChar(0xF8EA), "trademarksans" },                //
    std::pair<QChar, const char*>{ QChar(0xF6DB), "trademarkserif" },               //
    std::pair<QChar, const char*>{ QChar(0x0288), "tretroflexhook" },               // Character 'ʈ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x25BC), "triagdn" },                      // Character '▼' Symbol
    std::pair<QChar, const char*>{ QChar(0x25C4), "triaglf" },                      // Character '◄' Symbol
    std::pair<QChar, const char*>{ QChar(0x25BA), "triagrt" },                      // Character '►' Symbol
    std::pair<QChar, const char*>{ QChar(0x25B2), "triagup" },                      // Character '▲' Symbol
    std::pair<QChar, const char*>{ QChar(0x02A6), "ts" },                           // Character 'ʦ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x05E6), "tsadi" },                        // Character 'צ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB46), "tsadidagesh" },                  // Character 'צּ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB46), "tsadidageshhebrew" },            // Character 'צּ' Letter
    std::pair<QChar, const char*>{ QChar(0x05E6), "tsadihebrew" },                  // Character 'צ' Letter
    std::pair<QChar, const char*>{ QChar(0x0446), "tsecyrillic" },                  // Character 'ц' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x05B5), "tsere" },                        // Character 'ֵ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B5), "tsere12" },                      // Character 'ֵ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B5), "tsere1e" },                      // Character 'ֵ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B5), "tsere2b" },                      // Character 'ֵ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B5), "tserehebrew" },                  // Character 'ֵ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B5), "tserenarrowhebrew" },            // Character 'ֵ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B5), "tserequarterhebrew" },           // Character 'ֵ' Mark
    std::pair<QChar, const char*>{ QChar(0x05B5), "tserewidehebrew" },              // Character 'ֵ' Mark
    std::pair<QChar, const char*>{ QChar(0x045B), "tshecyrillic" },                 // Character 'ћ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0xF6F3), "tsuperior" },                    //
    std::pair<QChar, const char*>{ QChar(0x099F), "ttabengali" },                   // Character 'ট' Letter
    std::pair<QChar, const char*>{ QChar(0x091F), "ttadeva" },                      // Character 'ट' Letter
    std::pair<QChar, const char*>{ QChar(0x0A9F), "ttagujarati" },                  // Character 'ટ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A1F), "ttagurmukhi" },                  // Character 'ਟ' Letter
    std::pair<QChar, const char*>{ QChar(0x0679), "tteharabic" },                   // Character 'ٹ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB67), "ttehfinalarabic" },              // Character 'ﭧ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB68), "ttehinitialarabic" },            // Character 'ﭨ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB69), "ttehmedialarabic" },             // Character 'ﭩ' Letter
    std::pair<QChar, const char*>{ QChar(0x09A0), "tthabengali" },                  // Character 'ঠ' Letter
    std::pair<QChar, const char*>{ QChar(0x0920), "tthadeva" },                     // Character 'ठ' Letter
    std::pair<QChar, const char*>{ QChar(0x0AA0), "tthagujarati" },                 // Character 'ઠ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A20), "tthagurmukhi" },                 // Character 'ਠ' Letter
    std::pair<QChar, const char*>{ QChar(0x0287), "tturned" },                      // Character 'ʇ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3064), "tuhiragana" },                   // Character 'つ' Letter
    std::pair<QChar, const char*>{ QChar(0x30C4), "tukatakana" },                   // Character 'ツ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF82), "tukatakanahalfwidth" },          // Character 'ﾂ' Letter
    std::pair<QChar, const char*>{ QChar(0x3063), "tusmallhiragana" },              // Character 'っ' Letter
    std::pair<QChar, const char*>{ QChar(0x30C3), "tusmallkatakana" },              // Character 'ッ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF6F), "tusmallkatakanahalfwidth" },     // Character 'ｯ' Letter
    std::pair<QChar, const char*>{ QChar(0x246B), "twelvecircle" },                 // Character '⑫'
    std::pair<QChar, const char*>{ QChar(0x247F), "twelveparen" },                  // Character '⑿'
    std::pair<QChar, const char*>{ QChar(0x2493), "twelveperiod" },                 // Character '⒓'
    std::pair<QChar, const char*>{ QChar(0x217B), "twelveroman" },                  // Character 'ⅻ'
    std::pair<QChar, const char*>{ QChar(0x2473), "twentycircle" },                 // Character '⑳'
    std::pair<QChar, const char*>{ QChar(0x5344), "twentyhangzhou" },               // Character '卄' Letter
    std::pair<QChar, const char*>{ QChar(0x2487), "twentyparen" },                  // Character '⒇'
    std::pair<QChar, const char*>{ QChar(0x249B), "twentyperiod" },                 // Character '⒛'
    std::pair<QChar, const char*>{ QChar(0x0032), "two" },                          // Character '2' Digit
    std::pair<QChar, const char*>{ QChar(0x0662), "twoarabic" },                    // Character '٢' Digit
    std::pair<QChar, const char*>{ QChar(0x09E8), "twobengali" },                   // Character '২' Digit
    std::pair<QChar, const char*>{ QChar(0x2461), "twocircle" },                    // Character '②'
    std::pair<QChar, const char*>{ QChar(0x278B), "twocircleinversesansserif" },    // Character '➋'
    std::pair<QChar, const char*>{ QChar(0x0968), "twodeva" },                      // Character '२' Digit
    std::pair<QChar, const char*>{ QChar(0x2025), "twodotenleader" },               // Character '‥' Punctuation
    std::pair<QChar, const char*>{ QChar(0x2025), "twodotleader" },                 // Character '‥' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFE30), "twodotleadervertical" },         // Character '︰' Punctuation
    std::pair<QChar, const char*>{ QChar(0x0AE8), "twogujarati" },                  // Character '૨' Digit
    std::pair<QChar, const char*>{ QChar(0x0A68), "twogurmukhi" },                  // Character '੨' Digit
    std::pair<QChar, const char*>{ QChar(0x0662), "twohackarabic" },                // Character '٢' Digit
    std::pair<QChar, const char*>{ QChar(0x3022), "twohangzhou" },                  // Character '〢'
    std::pair<QChar, const char*>{ QChar(0x3221), "twoideographicparen" },          // Character '㈡'
    std::pair<QChar, const char*>{ QChar(0x2082), "twoinferior" },                  // Character '₂'
    std::pair<QChar, const char*>{ QChar(0xFF12), "twomonospace" },                 // Character '２' Digit
    std::pair<QChar, const char*>{ QChar(0x09F5), "twonumeratorbengali" },          // Character '৵'
    std::pair<QChar, const char*>{ QChar(0xF732), "twooldstyle" },                  //
    std::pair<QChar, const char*>{ QChar(0x2475), "twoparen" },                     // Character '⑵'
    std::pair<QChar, const char*>{ QChar(0x2489), "twoperiod" },                    // Character '⒉'
    std::pair<QChar, const char*>{ QChar(0x06F2), "twopersian" },                   // Character '۲' Digit
    std::pair<QChar, const char*>{ QChar(0x2171), "tworoman" },                     // Character 'ⅱ'
    std::pair<QChar, const char*>{ QChar(0x01BB), "twostroke" },                    // Character 'ƻ' Letter
    std::pair<QChar, const char*>{ QChar(0x00B2), "twosuperior" },                  // Character '²'
    std::pair<QChar, const char*>{ QChar(0x0E52), "twothai" },                      // Character '๒' Digit
    std::pair<QChar, const char*>{ QChar(0x2154), "twothirds" },                    // Character '⅔'
    std::pair<QChar, const char*>{ QChar(0x0075), "u" },                            // Character 'u' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x00FA), "uacute" },                       // Character 'ú' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0289), "ubar" },                         // Character 'ʉ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0989), "ubengali" },                     // Character 'উ' Letter
    std::pair<QChar, const char*>{ QChar(0x3128), "ubopomofo" },                    // Character 'ㄨ' Letter
    std::pair<QChar, const char*>{ QChar(0x016D), "ubreve" },                       // Character 'ŭ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x01D4), "ucaron" },                       // Character 'ǔ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x24E4), "ucircle" },                      // Character 'ⓤ' Symbol
    std::pair<QChar, const char*>{ QChar(0x00FB), "ucircumflex" },                  // Character 'û' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E77), "ucircumflexbelow" },             // Character 'ṷ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0443), "ucyrillic" },                    // Character 'у' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0951), "udattadeva" },                   // Character '॑' Mark
    std::pair<QChar, const char*>{ QChar(0x0171), "udblacute" },                    // Character 'ű' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0215), "udblgrave" },                    // Character 'ȕ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0909), "udeva" },                        // Character 'उ' Letter
    std::pair<QChar, const char*>{ QChar(0x00FC), "udieresis" },                    // Character 'ü' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x01D8), "udieresisacute" },               // Character 'ǘ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E73), "udieresisbelow" },               // Character 'ṳ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x01DA), "udieresiscaron" },               // Character 'ǚ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x04F1), "udieresiscyrillic" },            // Character 'ӱ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x01DC), "udieresisgrave" },               // Character 'ǜ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x01D6), "udieresismacron" },              // Character 'ǖ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1EE5), "udotbelow" },                    // Character 'ụ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x00F9), "ugrave" },                       // Character 'ù' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0A89), "ugujarati" },                    // Character 'ઉ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A09), "ugurmukhi" },                    // Character 'ਉ' Letter
    std::pair<QChar, const char*>{ QChar(0x3046), "uhiragana" },                    // Character 'う' Letter
    std::pair<QChar, const char*>{ QChar(0x1EE7), "uhookabove" },                   // Character 'ủ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x01B0), "uhorn" },                        // Character 'ư' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1EE9), "uhornacute" },                   // Character 'ứ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1EF1), "uhorndotbelow" },                // Character 'ự' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1EEB), "uhorngrave" },                   // Character 'ừ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1EED), "uhornhookabove" },               // Character 'ử' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1EEF), "uhorntilde" },                   // Character 'ữ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0171), "uhungarumlaut" },                // Character 'ű' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x04F3), "uhungarumlautcyrillic" },        // Character 'ӳ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0217), "uinvertedbreve" },               // Character 'ȗ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x30A6), "ukatakana" },                    // Character 'ウ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF73), "ukatakanahalfwidth" },           // Character 'ｳ' Letter
    std::pair<QChar, const char*>{ QChar(0x0479), "ukcyrillic" },                   // Character 'ѹ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x315C), "ukorean" },                      // Character 'ㅜ' Letter
    std::pair<QChar, const char*>{ QChar(0x016B), "umacron" },                      // Character 'ū' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x04EF), "umacroncyrillic" },              // Character 'ӯ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E7B), "umacrondieresis" },              // Character 'ṻ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0A41), "umatragurmukhi" },               // Character 'ੁ' Mark
    std::pair<QChar, const char*>{ QChar(0xFF55), "umonospace" },                   // Character 'ｕ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x005F), "underscore" },                   // Character '_' Punctuation
    std::pair<QChar, const char*>{ QChar(0x2017), "underscoredbl" },                // Character '‗' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFF3F), "underscoremonospace" },          // Character '＿' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFE33), "underscorevertical" },           // Character '︳' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFE4F), "underscorewavy" },               // Character '﹏' Punctuation
    std::pair<QChar, const char*>{ QChar(0x222A), "union" },                        // Character '∪' Symbol
    std::pair<QChar, const char*>{ QChar(0x2200), "universal" },                    // Character '∀' Symbol
    std::pair<QChar, const char*>{ QChar(0x0173), "uogonek" },                      // Character 'ų' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x24B0), "uparen" },                       // Character '⒰' Symbol
    std::pair<QChar, const char*>{ QChar(0x2580), "upblock" },                      // Character '▀' Symbol
    std::pair<QChar, const char*>{ QChar(0x05C4), "upperdothebrew" },               // Character 'ׄ' Mark
    std::pair<QChar, const char*>{ QChar(0x03C5), "upsilon" },                      // Character 'υ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x03CB), "upsilondieresis" },              // Character 'ϋ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x03B0), "upsilondieresistonos" },         // Character 'ΰ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x028A), "upsilonlatin" },                 // Character 'ʊ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x03CD), "upsilontonos" },                 // Character 'ύ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x031D), "uptackbelowcmb" },               // Character '̝' Mark
    std::pair<QChar, const char*>{ QChar(0x02D4), "uptackmod" },                    // Character '˔' Symbol
    std::pair<QChar, const char*>{ QChar(0x0A73), "uragurmukhi" },                  // Character 'ੳ' Letter
    std::pair<QChar, const char*>{ QChar(0x016F), "uring" },                        // Character 'ů' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x045E), "ushortcyrillic" },               // Character 'ў' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3045), "usmallhiragana" },               // Character 'ぅ' Letter
    std::pair<QChar, const char*>{ QChar(0x30A5), "usmallkatakana" },               // Character 'ゥ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF69), "usmallkatakanahalfwidth" },      // Character 'ｩ' Letter
    std::pair<QChar, const char*>{ QChar(0x04AF), "ustraightcyrillic" },            // Character 'ү' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x04B1), "ustraightstrokecyrillic" },      // Character 'ұ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0169), "utilde" },                       // Character 'ũ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E79), "utildeacute" },                  // Character 'ṹ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E75), "utildebelow" },                  // Character 'ṵ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x098A), "uubengali" },                    // Character 'ঊ' Letter
    std::pair<QChar, const char*>{ QChar(0x090A), "uudeva" },                       // Character 'ऊ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A8A), "uugujarati" },                   // Character 'ઊ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A0A), "uugurmukhi" },                   // Character 'ਊ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A42), "uumatragurmukhi" },              // Character 'ੂ' Mark
    std::pair<QChar, const char*>{ QChar(0x09C2), "uuvowelsignbengali" },           // Character 'ূ' Mark
    std::pair<QChar, const char*>{ QChar(0x0942), "uuvowelsigndeva" },              // Character 'ू' Mark
    std::pair<QChar, const char*>{ QChar(0x0AC2), "uuvowelsigngujarati" },          // Character 'ૂ' Mark
    std::pair<QChar, const char*>{ QChar(0x09C1), "uvowelsignbengali" },            // Character 'ু' Mark
    std::pair<QChar, const char*>{ QChar(0x0941), "uvowelsigndeva" },               // Character 'ु' Mark
    std::pair<QChar, const char*>{ QChar(0x0AC1), "uvowelsigngujarati" },           // Character 'ુ' Mark
    std::pair<QChar, const char*>{ QChar(0x0076), "v" },                            // Character 'v' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0935), "vadeva" },                       // Character 'व' Letter
    std::pair<QChar, const char*>{ QChar(0x0AB5), "vagujarati" },                   // Character 'વ' Letter
    std::pair<QChar, const char*>{ QChar(0x0A35), "vagurmukhi" },                   // Character 'ਵ' Letter
    std::pair<QChar, const char*>{ QChar(0x30F7), "vakatakana" },                   // Character 'ヷ' Letter
    std::pair<QChar, const char*>{ QChar(0x05D5), "vav" },                          // Character 'ו' Letter
    std::pair<QChar, const char*>{ QChar(0xFB35), "vavdagesh" },                    // Character 'וּ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB35), "vavdagesh65" },                  // Character 'וּ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB35), "vavdageshhebrew" },              // Character 'וּ' Letter
    std::pair<QChar, const char*>{ QChar(0x05D5), "vavhebrew" },                    // Character 'ו' Letter
    std::pair<QChar, const char*>{ QChar(0xFB4B), "vavholam" },                     // Character 'וֹ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB4B), "vavholamhebrew" },               // Character 'וֹ' Letter
    std::pair<QChar, const char*>{ QChar(0x05F0), "vavvavhebrew" },                 // Character 'װ' Letter
    std::pair<QChar, const char*>{ QChar(0x05F1), "vavyodhebrew" },                 // Character 'ױ' Letter
    std::pair<QChar, const char*>{ QChar(0x24E5), "vcircle" },                      // Character 'ⓥ' Symbol
    std::pair<QChar, const char*>{ QChar(0x1E7F), "vdotbelow" },                    // Character 'ṿ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0432), "vecyrillic" },                   // Character 'в' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x06A4), "veharabic" },                    // Character 'ڤ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB6B), "vehfinalarabic" },               // Character 'ﭫ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB6C), "vehinitialarabic" },             // Character 'ﭬ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB6D), "vehmedialarabic" },              // Character 'ﭭ' Letter
    std::pair<QChar, const char*>{ QChar(0x30F9), "vekatakana" },                   // Character 'ヹ' Letter
    std::pair<QChar, const char*>{ QChar(0x2640), "venus" },                        // Character '♀' Symbol
    std::pair<QChar, const char*>{ QChar(0x007C), "verticalbar" },                  // Character '|' Symbol
    std::pair<QChar, const char*>{ QChar(0x030D), "verticallineabovecmb" },         // Character '̍' Mark
    std::pair<QChar, const char*>{ QChar(0x0329), "verticallinebelowcmb" },         // Character '̩' Mark
    std::pair<QChar, const char*>{ QChar(0x02CC), "verticallinelowmod" },           // Character 'ˌ' Letter
    std::pair<QChar, const char*>{ QChar(0x02C8), "verticallinemod" },              // Character 'ˈ' Letter
    std::pair<QChar, const char*>{ QChar(0x057E), "vewarmenian" },                  // Character 'վ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x028B), "vhook" },                        // Character 'ʋ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x30F8), "vikatakana" },                   // Character 'ヸ' Letter
    std::pair<QChar, const char*>{ QChar(0x09CD), "viramabengali" },                // Character '্' Mark
    std::pair<QChar, const char*>{ QChar(0x094D), "viramadeva" },                   // Character '्' Mark
    std::pair<QChar, const char*>{ QChar(0x0ACD), "viramagujarati" },               // Character '્' Mark
    std::pair<QChar, const char*>{ QChar(0x0983), "visargabengali" },               // Character 'ঃ' Mark
    std::pair<QChar, const char*>{ QChar(0x0903), "visargadeva" },                  // Character 'ः' Mark
    std::pair<QChar, const char*>{ QChar(0x0A83), "visargagujarati" },              // Character 'ઃ' Mark
    std::pair<QChar, const char*>{ QChar(0xFF56), "vmonospace" },                   // Character 'ｖ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0578), "voarmenian" },                   // Character 'ո' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x309E), "voicediterationhiragana" },      // Character 'ゞ' Letter
    std::pair<QChar, const char*>{ QChar(0x30FE), "voicediterationkatakana" },      // Character 'ヾ' Letter
    std::pair<QChar, const char*>{ QChar(0x309B), "voicedmarkkana" },               // Character '゛' Symbol
    std::pair<QChar, const char*>{ QChar(0xFF9E), "voicedmarkkanahalfwidth" },      // Character 'ﾞ' Letter
    std::pair<QChar, const char*>{ QChar(0x30FA), "vokatakana" },                   // Character 'ヺ' Letter
    std::pair<QChar, const char*>{ QChar(0x24B1), "vparen" },                       // Character '⒱' Symbol
    std::pair<QChar, const char*>{ QChar(0x1E7D), "vtilde" },                       // Character 'ṽ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x028C), "vturned" },                      // Character 'ʌ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3094), "vuhiragana" },                   // Character 'ゔ' Letter
    std::pair<QChar, const char*>{ QChar(0x30F4), "vukatakana" },                   // Character 'ヴ' Letter
    std::pair<QChar, const char*>{ QChar(0x0077), "w" },                            // Character 'w' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E83), "wacute" },                       // Character 'ẃ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3159), "waekorean" },                    // Character 'ㅙ' Letter
    std::pair<QChar, const char*>{ QChar(0x308F), "wahiragana" },                   // Character 'わ' Letter
    std::pair<QChar, const char*>{ QChar(0x30EF), "wakatakana" },                   // Character 'ワ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF9C), "wakatakanahalfwidth" },          // Character 'ﾜ' Letter
    std::pair<QChar, const char*>{ QChar(0x3158), "wakorean" },                     // Character 'ㅘ' Letter
    std::pair<QChar, const char*>{ QChar(0x308E), "wasmallhiragana" },              // Character 'ゎ' Letter
    std::pair<QChar, const char*>{ QChar(0x30EE), "wasmallkatakana" },              // Character 'ヮ' Letter
    std::pair<QChar, const char*>{ QChar(0x3357), "wattosquare" },                  // Character '㍗' Symbol
    std::pair<QChar, const char*>{ QChar(0x301C), "wavedash" },                     // Character '〜' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFE34), "wavyunderscorevertical" },       // Character '︴' Punctuation
    std::pair<QChar, const char*>{ QChar(0x0648), "wawarabic" },                    // Character 'و' Letter
    std::pair<QChar, const char*>{ QChar(0xFEEE), "wawfinalarabic" },               // Character 'ﻮ' Letter
    std::pair<QChar, const char*>{ QChar(0x0624), "wawhamzaabovearabic" },          // Character 'ؤ' Letter
    std::pair<QChar, const char*>{ QChar(0xFE86), "wawhamzaabovefinalarabic" },     // Character 'ﺆ' Letter
    std::pair<QChar, const char*>{ QChar(0x33DD), "wbsquare" },                     // Character '㏝' Symbol
    std::pair<QChar, const char*>{ QChar(0x24E6), "wcircle" },                      // Character 'ⓦ' Symbol
    std::pair<QChar, const char*>{ QChar(0x0175), "wcircumflex" },                  // Character 'ŵ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E85), "wdieresis" },                    // Character 'ẅ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E87), "wdotaccent" },                   // Character 'ẇ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E89), "wdotbelow" },                    // Character 'ẉ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3091), "wehiragana" },                   // Character 'ゑ' Letter
    std::pair<QChar, const char*>{ QChar(0x2118), "weierstrass" },                  // Character '℘' Symbol
    std::pair<QChar, const char*>{ QChar(0x30F1), "wekatakana" },                   // Character 'ヱ' Letter
    std::pair<QChar, const char*>{ QChar(0x315E), "wekorean" },                     // Character 'ㅞ' Letter
    std::pair<QChar, const char*>{ QChar(0x315D), "weokorean" },                    // Character 'ㅝ' Letter
    std::pair<QChar, const char*>{ QChar(0x1E81), "wgrave" },                       // Character 'ẁ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x25E6), "whitebullet" },                  // Character '◦' Symbol
    std::pair<QChar, const char*>{ QChar(0x25CB), "whitecircle" },                  // Character '○' Symbol
    std::pair<QChar, const char*>{ QChar(0x25D9), "whitecircleinverse" },           // Character '◙' Symbol
    std::pair<QChar, const char*>{ QChar(0x300E), "whitecornerbracketleft" },       // Character '『' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFE43), "whitecornerbracketleftvertical" },// Character '﹃' Punctuation
    std::pair<QChar, const char*>{ QChar(0x300F), "whitecornerbracketright" },      // Character '』' Punctuation
    std::pair<QChar, const char*>{ QChar(0xFE44), "whitecornerbracketrightvertical" },// Character '﹄' Punctuation
    std::pair<QChar, const char*>{ QChar(0x25C7), "whitediamond" },                 // Character '◇' Symbol
    std::pair<QChar, const char*>{ QChar(0x25C8), "whitediamondcontainingblacksmalldiamond" },// Character '◈' Symbol
    std::pair<QChar, const char*>{ QChar(0x25BF), "whitedownpointingsmalltriangle" },// Character '▿' Symbol
    std::pair<QChar, const char*>{ QChar(0x25BD), "whitedownpointingtriangle" },    // Character '▽' Symbol
    std::pair<QChar, const char*>{ QChar(0x25C3), "whiteleftpointingsmalltriangle" },// Character '◃' Symbol
    std::pair<QChar, const char*>{ QChar(0x25C1), "whiteleftpointingtriangle" },    // Character '◁' Symbol
    std::pair<QChar, const char*>{ QChar(0x3016), "whitelenticularbracketleft" },   // Character '〖' Punctuation
    std::pair<QChar, const char*>{ QChar(0x3017), "whitelenticularbracketright" },  // Character '〗' Punctuation
    std::pair<QChar, const char*>{ QChar(0x25B9), "whiterightpointingsmalltriangle" },// Character '▹' Symbol
    std::pair<QChar, const char*>{ QChar(0x25B7), "whiterightpointingtriangle" },   // Character '▷' Symbol
    std::pair<QChar, const char*>{ QChar(0x25AB), "whitesmallsquare" },             // Character '▫' Symbol
    std::pair<QChar, const char*>{ QChar(0x263A), "whitesmilingface" },             // Character '☺' Symbol
    std::pair<QChar, const char*>{ QChar(0x25A1), "whitesquare" },                  // Character '□' Symbol
    std::pair<QChar, const char*>{ QChar(0x2606), "whitestar" },                    // Character '☆' Symbol
    std::pair<QChar, const char*>{ QChar(0x260F), "whitetelephone" },               // Character '☏' Symbol
    std::pair<QChar, const char*>{ QChar(0x3018), "whitetortoiseshellbracketleft" },// Character '〘' Punctuation
    std::pair<QChar, const char*>{ QChar(0x3019), "whitetortoiseshellbracketright" },// Character '〙' Punctuation
    std::pair<QChar, const char*>{ QChar(0x25B5), "whiteuppointingsmalltriangle" }, // Character '▵' Symbol
    std::pair<QChar, const char*>{ QChar(0x25B3), "whiteuppointingtriangle" },      // Character '△' Symbol
    std::pair<QChar, const char*>{ QChar(0x3090), "wihiragana" },                   // Character 'ゐ' Letter
    std::pair<QChar, const char*>{ QChar(0x30F0), "wikatakana" },                   // Character 'ヰ' Letter
    std::pair<QChar, const char*>{ QChar(0x315F), "wikorean" },                     // Character 'ㅟ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF57), "wmonospace" },                   // Character 'ｗ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3092), "wohiragana" },                   // Character 'を' Letter
    std::pair<QChar, const char*>{ QChar(0x30F2), "wokatakana" },                   // Character 'ヲ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF66), "wokatakanahalfwidth" },          // Character 'ｦ' Letter
    std::pair<QChar, const char*>{ QChar(0x20A9), "won" },                          // Character '₩' Symbol
    std::pair<QChar, const char*>{ QChar(0xFFE6), "wonmonospace" },                 // Character '￦' Symbol
    std::pair<QChar, const char*>{ QChar(0x0E27), "wowaenthai" },                   // Character 'ว' Letter
    std::pair<QChar, const char*>{ QChar(0x24B2), "wparen" },                       // Character '⒲' Symbol
    std::pair<QChar, const char*>{ QChar(0x1E98), "wring" },                        // Character 'ẘ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x02B7), "wsuperior" },                    // Character 'ʷ' Letter
    std::pair<QChar, const char*>{ QChar(0x028D), "wturned" },                      // Character 'ʍ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x01BF), "wynn" },                         // Character 'ƿ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0078), "x" },                            // Character 'x' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x033D), "xabovecmb" },                    // Character '̽' Mark
    std::pair<QChar, const char*>{ QChar(0x3112), "xbopomofo" },                    // Character 'ㄒ' Letter
    std::pair<QChar, const char*>{ QChar(0x24E7), "xcircle" },                      // Character 'ⓧ' Symbol
    std::pair<QChar, const char*>{ QChar(0x1E8D), "xdieresis" },                    // Character 'ẍ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E8B), "xdotaccent" },                   // Character 'ẋ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x056D), "xeharmenian" },                  // Character 'խ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x03BE), "xi" },                           // Character 'ξ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0xFF58), "xmonospace" },                   // Character 'ｘ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x24B3), "xparen" },                       // Character '⒳' Symbol
    std::pair<QChar, const char*>{ QChar(0x02E3), "xsuperior" },                    // Character 'ˣ' Letter
    std::pair<QChar, const char*>{ QChar(0x0079), "y" },                            // Character 'y' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x334E), "yaadosquare" },                  // Character '㍎' Symbol
    std::pair<QChar, const char*>{ QChar(0x09AF), "yabengali" },                    // Character 'য' Letter
    std::pair<QChar, const char*>{ QChar(0x00FD), "yacute" },                       // Character 'ý' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x092F), "yadeva" },                       // Character 'य' Letter
    std::pair<QChar, const char*>{ QChar(0x3152), "yaekorean" },                    // Character 'ㅒ' Letter
    std::pair<QChar, const char*>{ QChar(0x0AAF), "yagujarati" },                   // Character 'ય' Letter
    std::pair<QChar, const char*>{ QChar(0x0A2F), "yagurmukhi" },                   // Character 'ਯ' Letter
    std::pair<QChar, const char*>{ QChar(0x3084), "yahiragana" },                   // Character 'や' Letter
    std::pair<QChar, const char*>{ QChar(0x30E4), "yakatakana" },                   // Character 'ヤ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF94), "yakatakanahalfwidth" },          // Character 'ﾔ' Letter
    std::pair<QChar, const char*>{ QChar(0x3151), "yakorean" },                     // Character 'ㅑ' Letter
    std::pair<QChar, const char*>{ QChar(0x0E4E), "yamakkanthai" },                 // Character '๎' Mark
    std::pair<QChar, const char*>{ QChar(0x3083), "yasmallhiragana" },              // Character 'ゃ' Letter
    std::pair<QChar, const char*>{ QChar(0x30E3), "yasmallkatakana" },              // Character 'ャ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF6C), "yasmallkatakanahalfwidth" },     // Character 'ｬ' Letter
    std::pair<QChar, const char*>{ QChar(0x0463), "yatcyrillic" },                  // Character 'ѣ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x24E8), "ycircle" },                      // Character 'ⓨ' Symbol
    std::pair<QChar, const char*>{ QChar(0x0177), "ycircumflex" },                  // Character 'ŷ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x00FF), "ydieresis" },                    // Character 'ÿ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E8F), "ydotaccent" },                   // Character 'ẏ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1EF5), "ydotbelow" },                    // Character 'ỵ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x064A), "yeharabic" },                    // Character 'ي' Letter
    std::pair<QChar, const char*>{ QChar(0x06D2), "yehbarreearabic" },              // Character 'ے' Letter
    std::pair<QChar, const char*>{ QChar(0xFBAF), "yehbarreefinalarabic" },         // Character 'ﮯ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEF2), "yehfinalarabic" },               // Character 'ﻲ' Letter
    std::pair<QChar, const char*>{ QChar(0x0626), "yehhamzaabovearabic" },          // Character 'ئ' Letter
    std::pair<QChar, const char*>{ QChar(0xFE8A), "yehhamzaabovefinalarabic" },     // Character 'ﺊ' Letter
    std::pair<QChar, const char*>{ QChar(0xFE8B), "yehhamzaaboveinitialarabic" },   // Character 'ﺋ' Letter
    std::pair<QChar, const char*>{ QChar(0xFE8C), "yehhamzaabovemedialarabic" },    // Character 'ﺌ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEF3), "yehinitialarabic" },             // Character 'ﻳ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEF4), "yehmedialarabic" },              // Character 'ﻴ' Letter
    std::pair<QChar, const char*>{ QChar(0xFCDD), "yehmeeminitialarabic" },         // Character 'ﳝ' Letter
    std::pair<QChar, const char*>{ QChar(0xFC58), "yehmeemisolatedarabic" },        // Character 'ﱘ' Letter
    std::pair<QChar, const char*>{ QChar(0xFC94), "yehnoonfinalarabic" },           // Character 'ﲔ' Letter
    std::pair<QChar, const char*>{ QChar(0x06D1), "yehthreedotsbelowarabic" },      // Character 'ۑ' Letter
    std::pair<QChar, const char*>{ QChar(0x3156), "yekorean" },                     // Character 'ㅖ' Letter
    std::pair<QChar, const char*>{ QChar(0x00A5), "yen" },                          // Character '¥' Symbol
    std::pair<QChar, const char*>{ QChar(0xFFE5), "yenmonospace" },                 // Character '￥' Symbol
    std::pair<QChar, const char*>{ QChar(0x3155), "yeokorean" },                    // Character 'ㅕ' Letter
    std::pair<QChar, const char*>{ QChar(0x3186), "yeorinhieuhkorean" },            // Character 'ㆆ' Letter
    std::pair<QChar, const char*>{ QChar(0x05AA), "yerahbenyomohebrew" },           // Character '֪' Mark
    std::pair<QChar, const char*>{ QChar(0x05AA), "yerahbenyomolefthebrew" },       // Character '֪' Mark
    std::pair<QChar, const char*>{ QChar(0x044B), "yericyrillic" },                 // Character 'ы' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x04F9), "yerudieresiscyrillic" },         // Character 'ӹ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3181), "yesieungkorean" },               // Character 'ㆁ' Letter
    std::pair<QChar, const char*>{ QChar(0x3183), "yesieungpansioskorean" },        // Character 'ㆃ' Letter
    std::pair<QChar, const char*>{ QChar(0x3182), "yesieungsioskorean" },           // Character 'ㆂ' Letter
    std::pair<QChar, const char*>{ QChar(0x059A), "yetivhebrew" },                  // Character '֚' Mark
    std::pair<QChar, const char*>{ QChar(0x1EF3), "ygrave" },                       // Character 'ỳ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x01B4), "yhook" },                        // Character 'ƴ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1EF7), "yhookabove" },                   // Character 'ỷ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0575), "yiarmenian" },                   // Character 'յ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0457), "yicyrillic" },                   // Character 'ї' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3162), "yikorean" },                     // Character 'ㅢ' Letter
    std::pair<QChar, const char*>{ QChar(0x262F), "yinyang" },                      // Character '☯' Symbol
    std::pair<QChar, const char*>{ QChar(0x0582), "yiwnarmenian" },                 // Character 'ւ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0xFF59), "ymonospace" },                   // Character 'ｙ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x05D9), "yod" },                          // Character 'י' Letter
    std::pair<QChar, const char*>{ QChar(0xFB39), "yoddagesh" },                    // Character 'יּ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB39), "yoddageshhebrew" },              // Character 'יּ' Letter
    std::pair<QChar, const char*>{ QChar(0x05D9), "yodhebrew" },                    // Character 'י' Letter
    std::pair<QChar, const char*>{ QChar(0x05F2), "yodyodhebrew" },                 // Character 'ײ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB1F), "yodyodpatahhebrew" },            // Character 'ײַ' Letter
    std::pair<QChar, const char*>{ QChar(0x3088), "yohiragana" },                   // Character 'よ' Letter
    std::pair<QChar, const char*>{ QChar(0x3189), "yoikorean" },                    // Character 'ㆉ' Letter
    std::pair<QChar, const char*>{ QChar(0x30E8), "yokatakana" },                   // Character 'ヨ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF96), "yokatakanahalfwidth" },          // Character 'ﾖ' Letter
    std::pair<QChar, const char*>{ QChar(0x315B), "yokorean" },                     // Character 'ㅛ' Letter
    std::pair<QChar, const char*>{ QChar(0x3087), "yosmallhiragana" },              // Character 'ょ' Letter
    std::pair<QChar, const char*>{ QChar(0x30E7), "yosmallkatakana" },              // Character 'ョ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF6E), "yosmallkatakanahalfwidth" },     // Character 'ｮ' Letter
    std::pair<QChar, const char*>{ QChar(0x03F3), "yotgreek" },                     // Character 'ϳ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3188), "yoyaekorean" },                  // Character 'ㆈ' Letter
    std::pair<QChar, const char*>{ QChar(0x3187), "yoyakorean" },                   // Character 'ㆇ' Letter
    std::pair<QChar, const char*>{ QChar(0x0E22), "yoyakthai" },                    // Character 'ย' Letter
    std::pair<QChar, const char*>{ QChar(0x0E0D), "yoyingthai" },                   // Character 'ญ' Letter
    std::pair<QChar, const char*>{ QChar(0x24B4), "yparen" },                       // Character '⒴' Symbol
    std::pair<QChar, const char*>{ QChar(0x037A), "ypogegrammeni" },                // Character 'ͺ' Letter
    std::pair<QChar, const char*>{ QChar(0x0345), "ypogegrammenigreekcmb" },        // Character 'ͅ' Mark
    std::pair<QChar, const char*>{ QChar(0x01A6), "yr" },                           // Character 'Ʀ' Letter, Uppercase
    std::pair<QChar, const char*>{ QChar(0x1E99), "yring" },                        // Character 'ẙ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x02B8), "ysuperior" },                    // Character 'ʸ' Letter
    std::pair<QChar, const char*>{ QChar(0x1EF9), "ytilde" },                       // Character 'ỹ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x028E), "yturned" },                      // Character 'ʎ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3086), "yuhiragana" },                   // Character 'ゆ' Letter
    std::pair<QChar, const char*>{ QChar(0x318C), "yuikorean" },                    // Character 'ㆌ' Letter
    std::pair<QChar, const char*>{ QChar(0x30E6), "yukatakana" },                   // Character 'ユ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF95), "yukatakanahalfwidth" },          // Character 'ﾕ' Letter
    std::pair<QChar, const char*>{ QChar(0x3160), "yukorean" },                     // Character 'ㅠ' Letter
    std::pair<QChar, const char*>{ QChar(0x046B), "yusbigcyrillic" },               // Character 'ѫ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x046D), "yusbigiotifiedcyrillic" },       // Character 'ѭ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0467), "yuslittlecyrillic" },            // Character 'ѧ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0469), "yuslittleiotifiedcyrillic" },    // Character 'ѩ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3085), "yusmallhiragana" },              // Character 'ゅ' Letter
    std::pair<QChar, const char*>{ QChar(0x30E5), "yusmallkatakana" },              // Character 'ュ' Letter
    std::pair<QChar, const char*>{ QChar(0xFF6D), "yusmallkatakanahalfwidth" },     // Character 'ｭ' Letter
    std::pair<QChar, const char*>{ QChar(0x318B), "yuyekorean" },                   // Character 'ㆋ' Letter
    std::pair<QChar, const char*>{ QChar(0x318A), "yuyeokorean" },                  // Character 'ㆊ' Letter
    std::pair<QChar, const char*>{ QChar(0x09DF), "yyabengali" },                   // Character 'য়' Letter
    std::pair<QChar, const char*>{ QChar(0x095F), "yyadeva" },                      // Character 'य़' Letter
    std::pair<QChar, const char*>{ QChar(0x007A), "z" },                            // Character 'z' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0566), "zaarmenian" },                   // Character 'զ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x017A), "zacute" },                       // Character 'ź' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x095B), "zadeva" },                       // Character 'ज़' Letter
    std::pair<QChar, const char*>{ QChar(0x0A5B), "zagurmukhi" },                   // Character 'ਜ਼' Letter
    std::pair<QChar, const char*>{ QChar(0x0638), "zaharabic" },                    // Character 'ظ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEC6), "zahfinalarabic" },               // Character 'ﻆ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEC7), "zahinitialarabic" },             // Character 'ﻇ' Letter
    std::pair<QChar, const char*>{ QChar(0x3056), "zahiragana" },                   // Character 'ざ' Letter
    std::pair<QChar, const char*>{ QChar(0xFEC8), "zahmedialarabic" },              // Character 'ﻈ' Letter
    std::pair<QChar, const char*>{ QChar(0x0632), "zainarabic" },                   // Character 'ز' Letter
    std::pair<QChar, const char*>{ QChar(0xFEB0), "zainfinalarabic" },              // Character 'ﺰ' Letter
    std::pair<QChar, const char*>{ QChar(0x30B6), "zakatakana" },                   // Character 'ザ' Letter
    std::pair<QChar, const char*>{ QChar(0x0595), "zaqefgadolhebrew" },             // Character '֕' Mark
    std::pair<QChar, const char*>{ QChar(0x0594), "zaqefqatanhebrew" },             // Character '֔' Mark
    std::pair<QChar, const char*>{ QChar(0x0598), "zarqahebrew" },                  // Character '֘' Mark
    std::pair<QChar, const char*>{ QChar(0x05D6), "zayin" },                        // Character 'ז' Letter
    std::pair<QChar, const char*>{ QChar(0xFB36), "zayindagesh" },                  // Character 'זּ' Letter
    std::pair<QChar, const char*>{ QChar(0xFB36), "zayindageshhebrew" },            // Character 'זּ' Letter
    std::pair<QChar, const char*>{ QChar(0x05D6), "zayinhebrew" },                  // Character 'ז' Letter
    std::pair<QChar, const char*>{ QChar(0x3117), "zbopomofo" },                    // Character 'ㄗ' Letter
    std::pair<QChar, const char*>{ QChar(0x017E), "zcaron" },                       // Character 'ž' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x24E9), "zcircle" },                      // Character 'ⓩ' Symbol
    std::pair<QChar, const char*>{ QChar(0x1E91), "zcircumflex" },                  // Character 'ẑ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0291), "zcurl" },                        // Character 'ʑ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x017C), "zdot" },                         // Character 'ż' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x017C), "zdotaccent" },                   // Character 'ż' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x1E93), "zdotbelow" },                    // Character 'ẓ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0437), "zecyrillic" },                   // Character 'з' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0499), "zedescendercyrillic" },          // Character 'ҙ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x04DF), "zedieresiscyrillic" },           // Character 'ӟ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x305C), "zehiragana" },                   // Character 'ぜ' Letter
    std::pair<QChar, const char*>{ QChar(0x30BC), "zekatakana" },                   // Character 'ゼ' Letter
    std::pair<QChar, const char*>{ QChar(0x0030), "zero" },                         // Character '0' Digit
    std::pair<QChar, const char*>{ QChar(0x0660), "zeroarabic" },                   // Character '٠' Digit
    std::pair<QChar, const char*>{ QChar(0x09E6), "zerobengali" },                  // Character '০' Digit
    std::pair<QChar, const char*>{ QChar(0x0966), "zerodeva" },                     // Character '०' Digit
    std::pair<QChar, const char*>{ QChar(0x0AE6), "zerogujarati" },                 // Character '૦' Digit
    std::pair<QChar, const char*>{ QChar(0x0A66), "zerogurmukhi" },                 // Character '੦' Digit
    std::pair<QChar, const char*>{ QChar(0x0660), "zerohackarabic" },               // Character '٠' Digit
    std::pair<QChar, const char*>{ QChar(0x2080), "zeroinferior" },                 // Character '₀'
    std::pair<QChar, const char*>{ QChar(0xFF10), "zeromonospace" },                // Character '０' Digit
    std::pair<QChar, const char*>{ QChar(0xF730), "zerooldstyle" },                 //
    std::pair<QChar, const char*>{ QChar(0x06F0), "zeropersian" },                  // Character '۰' Digit
    std::pair<QChar, const char*>{ QChar(0x2070), "zerosuperior" },                 // Character '⁰'
    std::pair<QChar, const char*>{ QChar(0x0E50), "zerothai" },                     // Character '๐' Digit
    std::pair<QChar, const char*>{ QChar(0xFEFF), "zerowidthjoiner" },              //
    std::pair<QChar, const char*>{ QChar(0x200C), "zerowidthnonjoiner" },           //
    std::pair<QChar, const char*>{ QChar(0x200B), "zerowidthspace" },               //
    std::pair<QChar, const char*>{ QChar(0x03B6), "zeta" },                         // Character 'ζ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3113), "zhbopomofo" },                   // Character 'ㄓ' Letter
    std::pair<QChar, const char*>{ QChar(0x056A), "zhearmenian" },                  // Character 'ժ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x04C2), "zhebrevecyrillic" },             // Character 'ӂ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0436), "zhecyrillic" },                  // Character 'ж' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x0497), "zhedescendercyrillic" },         // Character 'җ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x04DD), "zhedieresiscyrillic" },          // Character 'ӝ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x3058), "zihiragana" },                   // Character 'じ' Letter
    std::pair<QChar, const char*>{ QChar(0x30B8), "zikatakana" },                   // Character 'ジ' Letter
    std::pair<QChar, const char*>{ QChar(0x05AE), "zinorhebrew" },                  // Character '֮' Mark
    std::pair<QChar, const char*>{ QChar(0x1E95), "zlinebelow" },                   // Character 'ẕ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0xFF5A), "zmonospace" },                   // Character 'ｚ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x305E), "zohiragana" },                   // Character 'ぞ' Letter
    std::pair<QChar, const char*>{ QChar(0x30BE), "zokatakana" },                   // Character 'ゾ' Letter
    std::pair<QChar, const char*>{ QChar(0x24B5), "zparen" },                       // Character '⒵' Symbol
    std::pair<QChar, const char*>{ QChar(0x0290), "zretroflexhook" },               // Character 'ʐ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x01B6), "zstroke" },                      // Character 'ƶ' Letter, Lowercase
    std::pair<QChar, const char*>{ QChar(0x305A), "zuhiragana" },                   // Character 'ず' Letter
    std::pair<QChar, const char*>{ QChar(0x30BA), "zukatakana" }                    // Character 'ズ' Letter
};

static constexpr const std::array<std::pair<QChar, const char*>, 201> glyphNameZapfDingbatsToUnicode = {
    std::pair<QChar, const char*>{ QChar(0x2701), "a1" },                           // Character '✁' Symbol
    std::pair<QChar, const char*>{ QChar(0x2721), "a10" },                          // Character '✡' Symbol
    std::pair<QChar, const char*>{ QChar(0x275E), "a100" },                         // Character '❞' Symbol
    std::pair<QChar, const char*>{ QChar(0x2761), "a101" },                         // Character '❡' Symbol
    std::pair<QChar, const char*>{ QChar(0x2762), "a102" },                         // Character '❢' Symbol
    std::pair<QChar, const char*>{ QChar(0x2763), "a103" },                         // Character '❣' Symbol
    std::pair<QChar, const char*>{ QChar(0x2764), "a104" },                         // Character '❤' Symbol
    std::pair<QChar, const char*>{ QChar(0x2710), "a105" },                         // Character '✐' Symbol
    std::pair<QChar, const char*>{ QChar(0x2765), "a106" },                         // Character '❥' Symbol
    std::pair<QChar, const char*>{ QChar(0x2766), "a107" },                         // Character '❦' Symbol
    std::pair<QChar, const char*>{ QChar(0x2767), "a108" },                         // Character '❧' Symbol
    std::pair<QChar, const char*>{ QChar(0x2660), "a109" },                         // Character '♠' Symbol
    std::pair<QChar, const char*>{ QChar(0x261B), "a11" },                          // Character '☛' Symbol
    std::pair<QChar, const char*>{ QChar(0x2665), "a110" },                         // Character '♥' Symbol
    std::pair<QChar, const char*>{ QChar(0x2666), "a111" },                         // Character '♦' Symbol
    std::pair<QChar, const char*>{ QChar(0x2663), "a112" },                         // Character '♣' Symbol
    std::pair<QChar, const char*>{ QChar(0x2709), "a117" },                         // Character '✉' Symbol
    std::pair<QChar, const char*>{ QChar(0x2708), "a118" },                         // Character '✈' Symbol
    std::pair<QChar, const char*>{ QChar(0x2707), "a119" },                         // Character '✇' Symbol
    std::pair<QChar, const char*>{ QChar(0x261E), "a12" },                          // Character '☞' Symbol
    std::pair<QChar, const char*>{ QChar(0x2460), "a120" },                         // Character '①'
    std::pair<QChar, const char*>{ QChar(0x2461), "a121" },                         // Character '②'
    std::pair<QChar, const char*>{ QChar(0x2462), "a122" },                         // Character '③'
    std::pair<QChar, const char*>{ QChar(0x2463), "a123" },                         // Character '④'
    std::pair<QChar, const char*>{ QChar(0x2464), "a124" },                         // Character '⑤'
    std::pair<QChar, const char*>{ QChar(0x2465), "a125" },                         // Character '⑥'
    std::pair<QChar, const char*>{ QChar(0x2466), "a126" },                         // Character '⑦'
    std::pair<QChar, const char*>{ QChar(0x2467), "a127" },                         // Character '⑧'
    std::pair<QChar, const char*>{ QChar(0x2468), "a128" },                         // Character '⑨'
    std::pair<QChar, const char*>{ QChar(0x2469), "a129" },                         // Character '⑩'
    std::pair<QChar, const char*>{ QChar(0x270C), "a13" },                          // Character '✌' Symbol
    std::pair<QChar, const char*>{ QChar(0x2776), "a130" },                         // Character '❶'
    std::pair<QChar, const char*>{ QChar(0x2777), "a131" },                         // Character '❷'
    std::pair<QChar, const char*>{ QChar(0x2778), "a132" },                         // Character '❸'
    std::pair<QChar, const char*>{ QChar(0x2779), "a133" },                         // Character '❹'
    std::pair<QChar, const char*>{ QChar(0x277A), "a134" },                         // Character '❺'
    std::pair<QChar, const char*>{ QChar(0x277B), "a135" },                         // Character '❻'
    std::pair<QChar, const char*>{ QChar(0x277C), "a136" },                         // Character '❼'
    std::pair<QChar, const char*>{ QChar(0x277D), "a137" },                         // Character '❽'
    std::pair<QChar, const char*>{ QChar(0x277E), "a138" },                         // Character '❾'
    std::pair<QChar, const char*>{ QChar(0x277F), "a139" },                         // Character '❿'
    std::pair<QChar, const char*>{ QChar(0x270D), "a14" },                          // Character '✍' Symbol
    std::pair<QChar, const char*>{ QChar(0x2780), "a140" },                         // Character '➀'
    std::pair<QChar, const char*>{ QChar(0x2781), "a141" },                         // Character '➁'
    std::pair<QChar, const char*>{ QChar(0x2782), "a142" },                         // Character '➂'
    std::pair<QChar, const char*>{ QChar(0x2783), "a143" },                         // Character '➃'
    std::pair<QChar, const char*>{ QChar(0x2784), "a144" },                         // Character '➄'
    std::pair<QChar, const char*>{ QChar(0x2785), "a145" },                         // Character '➅'
    std::pair<QChar, const char*>{ QChar(0x2786), "a146" },                         // Character '➆'
    std::pair<QChar, const char*>{ QChar(0x2787), "a147" },                         // Character '➇'
    std::pair<QChar, const char*>{ QChar(0x2788), "a148" },                         // Character '➈'
    std::pair<QChar, const char*>{ QChar(0x2789), "a149" },                         // Character '➉'
    std::pair<QChar, const char*>{ QChar(0x270E), "a15" },                          // Character '✎' Symbol
    std::pair<QChar, const char*>{ QChar(0x278A), "a150" },                         // Character '➊'
    std::pair<QChar, const char*>{ QChar(0x278B), "a151" },                         // Character '➋'
    std::pair<QChar, const char*>{ QChar(0x278C), "a152" },                         // Character '➌'
    std::pair<QChar, const char*>{ QChar(0x278D), "a153" },                         // Character '➍'
    std::pair<QChar, const char*>{ QChar(0x278E), "a154" },                         // Character '➎'
    std::pair<QChar, const char*>{ QChar(0x278F), "a155" },                         // Character '➏'
    std::pair<QChar, const char*>{ QChar(0x2790), "a156" },                         // Character '➐'
    std::pair<QChar, const char*>{ QChar(0x2791), "a157" },                         // Character '➑'
    std::pair<QChar, const char*>{ QChar(0x2792), "a158" },                         // Character '➒'
    std::pair<QChar, const char*>{ QChar(0x2793), "a159" },                         // Character '➓'
    std::pair<QChar, const char*>{ QChar(0x270F), "a16" },                          // Character '✏' Symbol
    std::pair<QChar, const char*>{ QChar(0x2794), "a160" },                         // Character '➔' Symbol
    std::pair<QChar, const char*>{ QChar(0x2192), "a161" },                         // Character '→' Symbol
    std::pair<QChar, const char*>{ QChar(0x27A3), "a162" },                         // Character '➣' Symbol
    std::pair<QChar, const char*>{ QChar(0x2194), "a163" },                         // Character '↔' Symbol
    std::pair<QChar, const char*>{ QChar(0x2195), "a164" },                         // Character '↕' Symbol
    std::pair<QChar, const char*>{ QChar(0x2799), "a165" },                         // Character '➙' Symbol
    std::pair<QChar, const char*>{ QChar(0x279B), "a166" },                         // Character '➛' Symbol
    std::pair<QChar, const char*>{ QChar(0x279C), "a167" },                         // Character '➜' Symbol
    std::pair<QChar, const char*>{ QChar(0x279D), "a168" },                         // Character '➝' Symbol
    std::pair<QChar, const char*>{ QChar(0x279E), "a169" },                         // Character '➞' Symbol
    std::pair<QChar, const char*>{ QChar(0x2711), "a17" },                          // Character '✑' Symbol
    std::pair<QChar, const char*>{ QChar(0x279F), "a170" },                         // Character '➟' Symbol
    std::pair<QChar, const char*>{ QChar(0x27A0), "a171" },                         // Character '➠' Symbol
    std::pair<QChar, const char*>{ QChar(0x27A1), "a172" },                         // Character '➡' Symbol
    std::pair<QChar, const char*>{ QChar(0x27A2), "a173" },                         // Character '➢' Symbol
    std::pair<QChar, const char*>{ QChar(0x27A4), "a174" },                         // Character '➤' Symbol
    std::pair<QChar, const char*>{ QChar(0x27A5), "a175" },                         // Character '➥' Symbol
    std::pair<QChar, const char*>{ QChar(0x27A6), "a176" },                         // Character '➦' Symbol
    std::pair<QChar, const char*>{ QChar(0x27A7), "a177" },                         // Character '➧' Symbol
    std::pair<QChar, const char*>{ QChar(0x27A8), "a178" },                         // Character '➨' Symbol
    std::pair<QChar, const char*>{ QChar(0x27A9), "a179" },                         // Character '➩' Symbol
    std::pair<QChar, const char*>{ QChar(0x2712), "a18" },                          // Character '✒' Symbol
    std::pair<QChar, const char*>{ QChar(0x27AB), "a180" },                         // Character '➫' Symbol
    std::pair<QChar, const char*>{ QChar(0x27AD), "a181" },                         // Character '➭' Symbol
    std::pair<QChar, const char*>{ QChar(0x27AF), "a182" },                         // Character '➯' Symbol
    std::pair<QChar, const char*>{ QChar(0x27B2), "a183" },                         // Character '➲' Symbol
    std::pair<QChar, const char*>{ QChar(0x27B3), "a184" },                         // Character '➳' Symbol
    std::pair<QChar, const char*>{ QChar(0x27B5), "a185" },                         // Character '➵' Symbol
    std::pair<QChar, const char*>{ QChar(0x27B8), "a186" },                         // Character '➸' Symbol
    std::pair<QChar, const char*>{ QChar(0x27BA), "a187" },                         // Character '➺' Symbol
    std::pair<QChar, const char*>{ QChar(0x27BB), "a188" },                         // Character '➻' Symbol
    std::pair<QChar, const char*>{ QChar(0x27BC), "a189" },                         // Character '➼' Symbol
    std::pair<QChar, const char*>{ QChar(0x2713), "a19" },                          // Character '✓' Symbol
    std::pair<QChar, const char*>{ QChar(0x27BD), "a190" },                         // Character '➽' Symbol
    std::pair<QChar, const char*>{ QChar(0x27BE), "a191" },                         // Character '➾' Symbol
    std::pair<QChar, const char*>{ QChar(0x279A), "a192" },                         // Character '➚' Symbol
    std::pair<QChar, const char*>{ QChar(0x27AA), "a193" },                         // Character '➪' Symbol
    std::pair<QChar, const char*>{ QChar(0x27B6), "a194" },                         // Character '➶' Symbol
    std::pair<QChar, const char*>{ QChar(0x27B9), "a195" },                         // Character '➹' Symbol
    std::pair<QChar, const char*>{ QChar(0x2798), "a196" },                         // Character '➘' Symbol
    std::pair<QChar, const char*>{ QChar(0x27B4), "a197" },                         // Character '➴' Symbol
    std::pair<QChar, const char*>{ QChar(0x27B7), "a198" },                         // Character '➷' Symbol
    std::pair<QChar, const char*>{ QChar(0x27AC), "a199" },                         // Character '➬' Symbol
    std::pair<QChar, const char*>{ QChar(0x2702), "a2" },                           // Character '✂' Symbol
    std::pair<QChar, const char*>{ QChar(0x2714), "a20" },                          // Character '✔' Symbol
    std::pair<QChar, const char*>{ QChar(0x27AE), "a200" },                         // Character '➮' Symbol
    std::pair<QChar, const char*>{ QChar(0x27B1), "a201" },                         // Character '➱' Symbol
    std::pair<QChar, const char*>{ QChar(0x2703), "a202" },                         // Character '✃' Symbol
    std::pair<QChar, const char*>{ QChar(0x2750), "a203" },                         // Character '❐' Symbol
    std::pair<QChar, const char*>{ QChar(0x2752), "a204" },                         // Character '❒' Symbol
    std::pair<QChar, const char*>{ QChar(0x276E), "a205" },                         // Character '❮' Punctuation
    std::pair<QChar, const char*>{ QChar(0x2770), "a206" },                         // Character '❰' Punctuation
    std::pair<QChar, const char*>{ QChar(0x2715), "a21" },                          // Character '✕' Symbol
    std::pair<QChar, const char*>{ QChar(0x2716), "a22" },                          // Character '✖' Symbol
    std::pair<QChar, const char*>{ QChar(0x2717), "a23" },                          // Character '✗' Symbol
    std::pair<QChar, const char*>{ QChar(0x2718), "a24" },                          // Character '✘' Symbol
    std::pair<QChar, const char*>{ QChar(0x2719), "a25" },                          // Character '✙' Symbol
    std::pair<QChar, const char*>{ QChar(0x271A), "a26" },                          // Character '✚' Symbol
    std::pair<QChar, const char*>{ QChar(0x271B), "a27" },                          // Character '✛' Symbol
    std::pair<QChar, const char*>{ QChar(0x271C), "a28" },                          // Character '✜' Symbol
    std::pair<QChar, const char*>{ QChar(0x2722), "a29" },                          // Character '✢' Symbol
    std::pair<QChar, const char*>{ QChar(0x2704), "a3" },                           // Character '✄' Symbol
    std::pair<QChar, const char*>{ QChar(0x2723), "a30" },                          // Character '✣' Symbol
    std::pair<QChar, const char*>{ QChar(0x2724), "a31" },                          // Character '✤' Symbol
    std::pair<QChar, const char*>{ QChar(0x2725), "a32" },                          // Character '✥' Symbol
    std::pair<QChar, const char*>{ QChar(0x2726), "a33" },                          // Character '✦' Symbol
    std::pair<QChar, const char*>{ QChar(0x2727), "a34" },                          // Character '✧' Symbol
    std::pair<QChar, const char*>{ QChar(0x2605), "a35" },                          // Character '★' Symbol
    std::pair<QChar, const char*>{ QChar(0x2729), "a36" },                          // Character '✩' Symbol
    std::pair<QChar, const char*>{ QChar(0x272A), "a37" },                          // Character '✪' Symbol
    std::pair<QChar, const char*>{ QChar(0x272B), "a38" },                          // Character '✫' Symbol
    std::pair<QChar, const char*>{ QChar(0x272C), "a39" },                          // Character '✬' Symbol
    std::pair<QChar, const char*>{ QChar(0x260E), "a4" },                           // Character '☎' Symbol
    std::pair<QChar, const char*>{ QChar(0x272D), "a40" },                          // Character '✭' Symbol
    std::pair<QChar, const char*>{ QChar(0x272E), "a41" },                          // Character '✮' Symbol
    std::pair<QChar, const char*>{ QChar(0x272F), "a42" },                          // Character '✯' Symbol
    std::pair<QChar, const char*>{ QChar(0x2730), "a43" },                          // Character '✰' Symbol
    std::pair<QChar, const char*>{ QChar(0x2731), "a44" },                          // Character '✱' Symbol
    std::pair<QChar, const char*>{ QChar(0x2732), "a45" },                          // Character '✲' Symbol
    std::pair<QChar, const char*>{ QChar(0x2733), "a46" },                          // Character '✳' Symbol
    std::pair<QChar, const char*>{ QChar(0x2734), "a47" },                          // Character '✴' Symbol
    std::pair<QChar, const char*>{ QChar(0x2735), "a48" },                          // Character '✵' Symbol
    std::pair<QChar, const char*>{ QChar(0x2736), "a49" },                          // Character '✶' Symbol
    std::pair<QChar, const char*>{ QChar(0x2706), "a5" },                           // Character '✆' Symbol
    std::pair<QChar, const char*>{ QChar(0x2737), "a50" },                          // Character '✷' Symbol
    std::pair<QChar, const char*>{ QChar(0x2738), "a51" },                          // Character '✸' Symbol
    std::pair<QChar, const char*>{ QChar(0x2739), "a52" },                          // Character '✹' Symbol
    std::pair<QChar, const char*>{ QChar(0x273A), "a53" },                          // Character '✺' Symbol
    std::pair<QChar, const char*>{ QChar(0x273B), "a54" },                          // Character '✻' Symbol
    std::pair<QChar, const char*>{ QChar(0x273C), "a55" },                          // Character '✼' Symbol
    std::pair<QChar, const char*>{ QChar(0x273D), "a56" },                          // Character '✽' Symbol
    std::pair<QChar, const char*>{ QChar(0x273E), "a57" },                          // Character '✾' Symbol
    std::pair<QChar, const char*>{ QChar(0x273F), "a58" },                          // Character '✿' Symbol
    std::pair<QChar, const char*>{ QChar(0x2740), "a59" },                          // Character '❀' Symbol
    std::pair<QChar, const char*>{ QChar(0x271D), "a6" },                           // Character '✝' Symbol
    std::pair<QChar, const char*>{ QChar(0x2741), "a60" },                          // Character '❁' Symbol
    std::pair<QChar, const char*>{ QChar(0x2742), "a61" },                          // Character '❂' Symbol
    std::pair<QChar, const char*>{ QChar(0x2743), "a62" },                          // Character '❃' Symbol
    std::pair<QChar, const char*>{ QChar(0x2744), "a63" },                          // Character '❄' Symbol
    std::pair<QChar, const char*>{ QChar(0x2745), "a64" },                          // Character '❅' Symbol
    std::pair<QChar, const char*>{ QChar(0x2746), "a65" },                          // Character '❆' Symbol
    std::pair<QChar, const char*>{ QChar(0x2747), "a66" },                          // Character '❇' Symbol
    std::pair<QChar, const char*>{ QChar(0x2748), "a67" },                          // Character '❈' Symbol
    std::pair<QChar, const char*>{ QChar(0x2749), "a68" },                          // Character '❉' Symbol
    std::pair<QChar, const char*>{ QChar(0x274A), "a69" },                          // Character '❊' Symbol
    std::pair<QChar, const char*>{ QChar(0x271E), "a7" },                           // Character '✞' Symbol
    std::pair<QChar, const char*>{ QChar(0x274B), "a70" },                          // Character '❋' Symbol
    std::pair<QChar, const char*>{ QChar(0x25CF), "a71" },                          // Character '●' Symbol
    std::pair<QChar, const char*>{ QChar(0x274D), "a72" },                          // Character '❍' Symbol
    std::pair<QChar, const char*>{ QChar(0x25A0), "a73" },                          // Character '■' Symbol
    std::pair<QChar, const char*>{ QChar(0x274F), "a74" },                          // Character '❏' Symbol
    std::pair<QChar, const char*>{ QChar(0x2751), "a75" },                          // Character '❑' Symbol
    std::pair<QChar, const char*>{ QChar(0x25B2), "a76" },                          // Character '▲' Symbol
    std::pair<QChar, const char*>{ QChar(0x25BC), "a77" },                          // Character '▼' Symbol
    std::pair<QChar, const char*>{ QChar(0x25C6), "a78" },                          // Character '◆' Symbol
    std::pair<QChar, const char*>{ QChar(0x2756), "a79" },                          // Character '❖' Symbol
    std::pair<QChar, const char*>{ QChar(0x271F), "a8" },                           // Character '✟' Symbol
    std::pair<QChar, const char*>{ QChar(0x25D7), "a81" },                          // Character '◗' Symbol
    std::pair<QChar, const char*>{ QChar(0x2758), "a82" },                          // Character '❘' Symbol
    std::pair<QChar, const char*>{ QChar(0x2759), "a83" },                          // Character '❙' Symbol
    std::pair<QChar, const char*>{ QChar(0x275A), "a84" },                          // Character '❚' Symbol
    std::pair<QChar, const char*>{ QChar(0x276F), "a85" },                          // Character '❯' Punctuation
    std::pair<QChar, const char*>{ QChar(0x2771), "a86" },                          // Character '❱' Punctuation
    std::pair<QChar, const char*>{ QChar(0x2772), "a87" },                          // Character '❲' Punctuation
    std::pair<QChar, const char*>{ QChar(0x2773), "a88" },                          // Character '❳' Punctuation
    std::pair<QChar, const char*>{ QChar(0x2768), "a89" },                          // Character '❨' Punctuation
    std::pair<QChar, const char*>{ QChar(0x2720), "a9" },                           // Character '✠' Symbol
    std::pair<QChar, const char*>{ QChar(0x2769), "a90" },                          // Character '❩' Punctuation
    std::pair<QChar, const char*>{ QChar(0x276C), "a91" },                          // Character '❬' Punctuation
    std::pair<QChar, const char*>{ QChar(0x276D), "a92" },                          // Character '❭' Punctuation
    std::pair<QChar, const char*>{ QChar(0x276A), "a93" },                          // Character '❪' Punctuation
    std::pair<QChar, const char*>{ QChar(0x276B), "a94" },                          // Character '❫' Punctuation
    std::pair<QChar, const char*>{ QChar(0x2774), "a95" },                          // Character '❴' Punctuation
    std::pair<QChar, const char*>{ QChar(0x2775), "a96" },                          // Character '❵' Punctuation
    std::pair<QChar, const char*>{ QChar(0x275B), "a97" },                          // Character '❛' Symbol
    std::pair<QChar, const char*>{ QChar(0x275C), "a98" },                          // Character '❜' Symbol
    std::pair<QChar, const char*>{ QChar(0x275D), "a99" }                           // Character '❝' Symbol
};

QChar PDFNameToUnicode::getUnicodeForName(const QByteArray& name)
{
    Q_ASSERT(std::is_sorted(glyphNameToUnicode.cbegin(), glyphNameToUnicode.cend(), Comparator()));

    auto [it, itEnd] = std::equal_range(glyphNameToUnicode.cbegin(), glyphNameToUnicode.cend(), name, Comparator());
    if (it != itEnd)
    {
        return it->first;
    }
    else
    {
        return QChar();
    }
}

QChar PDFNameToUnicode::getUnicodeForNameZapfDingbats(const QByteArray& name)
{
    Q_ASSERT(std::is_sorted(glyphNameZapfDingbatsToUnicode.cbegin(), glyphNameZapfDingbatsToUnicode.cend(), Comparator()));

    auto [it, itEnd] = std::equal_range(glyphNameZapfDingbatsToUnicode.cbegin(), glyphNameZapfDingbatsToUnicode.cend(), name, Comparator());
    if (it != itEnd)
    {
        return it->first;
    }
    else
    {
        return QChar();
    }
}

QChar PDFNameToUnicode::getUnicodeUsingResolvedName(const QByteArray& name)
{
    QChar character = getUnicodeForName(name);

    // Try ZapfDingbats, if this fails
    if (character.isNull())
    {
        character = getUnicodeForNameZapfDingbats(name);
    }

    if (character.isNull() && name.startsWith("uni"))
    {
        QByteArray hexValue = QByteArray::fromHex(name.mid(3, -1));
        if (hexValue.size() == 2)
        {
            unsigned short value = (static_cast<unsigned char>(hexValue[0]) << 8) + static_cast<unsigned char>(hexValue[1]);
            character = QChar(value);
        }
    }

    return character;
}

}   // namespace pdf
