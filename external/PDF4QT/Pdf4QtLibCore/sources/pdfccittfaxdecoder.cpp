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


#include "pdfccittfaxdecoder.h"
#include "pdfexception.h"
#include "pdfdbgheap.h"

namespace pdf
{

template<char... Digits>
constexpr uint8_t operator "" _bitlength()
{
    return sizeof...(Digits);
}

struct PDFCCITT2DModeInfo
{
    CCITT_2D_Code_Mode mode;
    uint16_t code;
    uint8_t bits;
};

static constexpr uint8_t MAX_2D_MODE_BIT_LENGTH = 7;

static constexpr PDFCCITT2DModeInfo CCITT_2D_CODE_MODES[] =
{
    { Pass,         0b0001,       0001_bitlength },
    { Horizontal,   0b001,        001_bitlength },
    { Vertical_3L,  0b0000010,    0000010_bitlength },
    { Vertical_2L,  0b000010,     000010_bitlength },
    { Vertical_1L,  0b010,        010_bitlength },
    { Vertical_0,   0b1,          1_bitlength },
    { Vertical_1R,  0b011,        011_bitlength },
    { Vertical_2R,  0b000011,     000011_bitlength },
    { Vertical_3R,  0b0000011,    0000011_bitlength }
};

struct PDFCCITTCode
{
    uint16_t length;
    uint16_t code;
    uint8_t bits;
};

static constexpr uint8_t MAX_CODE_BIT_LENGTH = 12;

static constexpr PDFCCITTCode CCITT_WHITE_CODES[] = {

// Terminating white codes

    { 0,       0b00110101,   00110101_bitlength },
    { 1,       0b000111,     000111_bitlength },
    { 2,       0b0111,       0111_bitlength },
    { 3,       0b1000,       1000_bitlength },
    { 4,       0b1011,       1011_bitlength },
    { 5,       0b1100,       1100_bitlength },
    { 6,       0b1110,       1110_bitlength },
    { 7,       0b1111,       1111_bitlength },
    { 8,       0b10011,      10011_bitlength },
    { 9,       0b10100,      10100_bitlength },
    { 10,      0b00111,      00111_bitlength },
    { 11,      0b01000,      01000_bitlength },
    { 12,      0b001000,     001000_bitlength },
    { 13,      0b000011,     000011_bitlength },
    { 14,      0b110100,     110100_bitlength },
    { 15,      0b110101,     110101_bitlength },
    { 16,      0b101010,     101010_bitlength },
    { 17,      0b101011,     101011_bitlength },
    { 18,      0b0100111,    0100111_bitlength },
    { 19,      0b0001100,    0001100_bitlength },
    { 20,      0b0001000,    0001000_bitlength },
    { 21,      0b0010111,    0010111_bitlength },
    { 22,      0b0000011,    0000011_bitlength },
    { 23,      0b0000100,    0000100_bitlength },
    { 24,      0b0101000,    0101000_bitlength },
    { 25,      0b0101011,    0101011_bitlength },
    { 26,      0b0010011,    0010011_bitlength },
    { 27,      0b0100100,    0100100_bitlength },
    { 28,      0b0011000,    0011000_bitlength },
    { 29,      0b00000010,   00000010_bitlength },
    { 30,      0b00000011,   00000011_bitlength },
    { 31,      0b00011010,   00011010_bitlength },
    { 32,      0b00011011,   00011011_bitlength },
    { 33,      0b00010010,   00010010_bitlength },
    { 34,      0b00010011,   00010011_bitlength },
    { 35,      0b00010100,   00010100_bitlength },
    { 36,      0b00010101,   00010101_bitlength },
    { 37,      0b00010110,   00010110_bitlength },
    { 38,      0b00010111,   00010111_bitlength },
    { 39,      0b00101000,   00101000_bitlength },
    { 40,      0b00101001,   00101001_bitlength },
    { 41,      0b00101010,   00101010_bitlength },
    { 42,      0b00101011,   00101011_bitlength },
    { 43,      0b00101100,   00101100_bitlength },
    { 44,      0b00101101,   00101101_bitlength },
    { 45,      0b00000100,   00000100_bitlength },
    { 46,      0b00000101,   00000101_bitlength },
    { 47,      0b00001010,   00001010_bitlength },
    { 48,      0b00001011,   00001011_bitlength },
    { 49,      0b01010010,   01010010_bitlength },
    { 50,      0b01010011,   01010011_bitlength },
    { 51,      0b01010100,   01010100_bitlength },
    { 52,      0b01010101,   01010101_bitlength },
    { 53,      0b00100100,   00100100_bitlength },
    { 54,      0b00100101,   00100101_bitlength },
    { 55,      0b01011000,   01011000_bitlength },
    { 56,      0b01011001,   01011001_bitlength },
    { 57,      0b01011010,   01011010_bitlength },
    { 58,      0b01011011,   01011011_bitlength },
    { 59,      0b01001010,   01001010_bitlength },
    { 60,      0b01001011,   01001011_bitlength },
    { 61,      0b00110010,   00110010_bitlength },
    { 62,      0b00110011,   00110011_bitlength },
    { 63,      0b00110100,   00110100_bitlength },

// Make up white codes - doesn't terminate
    { 64,     0b11011,       11011_bitlength },
    { 128,    0b10010,       10010_bitlength },
    { 192,    0b010111,      010111_bitlength },
    { 256,    0b0110111,     0110111_bitlength },
    { 320,    0b00110110,    00110110_bitlength },
    { 384,    0b00110111,    00110111_bitlength },
    { 448,    0b01100100,    01100100_bitlength },
    { 512,    0b01100101,    01100101_bitlength },
    { 576,    0b01101000,    01101000_bitlength },
    { 640,    0b01100111,    01100111_bitlength },
    { 704,    0b011001100,   011001100_bitlength },
    { 768,    0b011001101,   011001101_bitlength },
    { 832,    0b011010010,   011010010_bitlength },
    { 896,    0b011010011,   011010011_bitlength },
    { 960,    0b011010100,   011010100_bitlength },
    { 1024,   0b011010101,   011010101_bitlength },
    { 1088,   0b011010110,   011010110_bitlength },
    { 1152,   0b011010111,   011010111_bitlength },
    { 1216,   0b011011000,   011011000_bitlength },
    { 1280,   0b011011001,   011011001_bitlength },
    { 1344,   0b011011010,   011011010_bitlength },
    { 1408,   0b011011011,   011011011_bitlength },
    { 1472,   0b010011000,   010011000_bitlength },
    { 1536,   0b010011001,   010011001_bitlength },
    { 1600,   0b010011010,   010011010_bitlength },
    { 1664,   0b011000,      011000_bitlength },
    { 1728,   0b010011011,   010011011_bitlength },
    { 1792,   0b00000001000,     00000001000_bitlength },
    { 1856,   0b00000001100,     00000001100_bitlength },
    { 1920,   0b00000001101,     00000001101_bitlength },
    { 1984,   0b000000010010,    000000010010_bitlength },
    { 2048,   0b000000010011,    000000010011_bitlength },
    { 2112,   0b000000010100,    000000010100_bitlength },
    { 2176,   0b000000010101,    000000010101_bitlength },
    { 2240,   0b000000010110,    000000010110_bitlength },
    { 2304,   0b000000010111,    000000010111_bitlength },
    { 2368,   0b000000011100,    000000011100_bitlength },
    { 2432,   0b000000011101,    000000011101_bitlength },
    { 2496,   0b000000011110,    000000011110_bitlength },
    { 2560,   0b000000011111,    000000011111_bitlength }
};

static constexpr PDFCCITTCode CCITT_BLACK_CODES[] = {
// Terminating black codes

    { 0,     0b0000110111,      0000110111_bitlength },
    { 1,     0b010,             010_bitlength },
    { 2,     0b11,              11_bitlength },
    { 3,     0b10,              10_bitlength },
    { 4,     0b011,             011_bitlength },
    { 5,     0b0011,            0011_bitlength },
    { 6,     0b0010,            0010_bitlength },
    { 7,     0b00011,           00011_bitlength },
    { 8,     0b000101,          000101_bitlength },
    { 9,     0b000100,          000100_bitlength },
    { 10,    0b0000100,         0000100_bitlength },
    { 11,    0b0000101,         0000101_bitlength },
    { 12,    0b0000111,         0000111_bitlength },
    { 13,    0b00000100,        00000100_bitlength },
    { 14,    0b00000111,        00000111_bitlength },
    { 15,    0b000011000,       000011000_bitlength },
    { 16,    0b0000010111,      0000010111_bitlength },
    { 17,    0b0000011000,      0000011000_bitlength },
    { 18,    0b0000001000,      0000001000_bitlength },
    { 19,    0b00001100111,     00001100111_bitlength },
    { 20,    0b00001101000,     00001101000_bitlength },
    { 21,    0b00001101100,     00001101100_bitlength },
    { 22,    0b00000110111,     00000110111_bitlength },
    { 23,    0b00000101000,     00000101000_bitlength },
    { 24,    0b00000010111,     00000010111_bitlength },
    { 25,    0b00000011000,     00000011000_bitlength },
    { 26,    0b000011001010,    000011001010_bitlength },
    { 27,    0b000011001011,    000011001011_bitlength },
    { 28,    0b000011001100,    000011001100_bitlength },
    { 29,    0b000011001101,    000011001101_bitlength },
    { 30,    0b000001101000,    000001101000_bitlength },
    { 31,    0b000001101001,    000001101001_bitlength },
    { 32,    0b000001101010,    000001101010_bitlength },
    { 33,    0b000001101011,    000001101011_bitlength },
    { 34,    0b000011010010,    000011010010_bitlength },
    { 35,    0b000011010011,    000011010011_bitlength },
    { 36,    0b000011010100,    000011010100_bitlength },
    { 37,    0b000011010101,    000011010101_bitlength },
    { 38,    0b000011010110,    000011010110_bitlength },
    { 39,    0b000011010111,    000011010111_bitlength },
    { 40,    0b000001101100,    000001101100_bitlength },
    { 41,    0b000001101101,    000001101101_bitlength },
    { 42,    0b000011011010,    000011011010_bitlength },
    { 43,    0b000011011011,    000011011011_bitlength },
    { 44,    0b000001010100,    000001010100_bitlength },
    { 45,    0b000001010101,    000001010101_bitlength },
    { 46,    0b000001010110,    000001010110_bitlength },
    { 47,    0b000001010111,    000001010111_bitlength },
    { 48,    0b000001100100,    000001100100_bitlength },
    { 49,    0b000001100101,    000001100101_bitlength },
    { 50,    0b000001010010,    000001010010_bitlength },
    { 51,    0b000001010011,    000001010011_bitlength },
    { 52,    0b000000100100,    000000100100_bitlength },
    { 53,    0b000000110111,    000000110111_bitlength },
    { 54,    0b000000111000,    000000111000_bitlength },
    { 55,    0b000000100111,    000000100111_bitlength },
    { 56,    0b000000101000,    000000101000_bitlength },
    { 57,    0b000001011000,    000001011000_bitlength },
    { 58,    0b000001011001,    000001011001_bitlength },
    { 59,    0b000000101011,    000000101011_bitlength },
    { 60,    0b000000101100,    000000101100_bitlength },
    { 61,    0b000001011010,    000001011010_bitlength },
    { 62,    0b000001100110,    000001100110_bitlength },
    { 63,    0b000001100111,    000001100111_bitlength },

// Make up white codes - doesn't terminate

    { 64,      0b0000001111,       0000001111_bitlength },
    { 128,     0b000011001000,     000011001000_bitlength },
    { 192,     0b000011001001,     000011001001_bitlength },
    { 256,     0b000001011011,     000001011011_bitlength },
    { 320,     0b000000110011,     000000110011_bitlength },
    { 384,     0b000000110100,     000000110100_bitlength },
    { 448,     0b000000110101,     000000110101_bitlength },
    { 512,     0b0000001101100,    0000001101100_bitlength },
    { 576,     0b0000001101101,    0000001101101_bitlength },
    { 640,     0b0000001001010,    0000001001010_bitlength },
    { 704,     0b0000001001011,    0000001001011_bitlength },
    { 768,     0b0000001001100,    0000001001100_bitlength },
    { 832,     0b0000001001101,    0000001001101_bitlength },
    { 896,     0b0000001110010,    0000001110010_bitlength },
    { 960,     0b0000001110011,    0000001110011_bitlength },
    { 1024,    0b0000001110100,    0000001110100_bitlength },
    { 1088,    0b0000001110101,    0000001110101_bitlength },
    { 1152,    0b0000001110110,    0000001110110_bitlength },
    { 1216,    0b0000001110111,    0000001110111_bitlength },
    { 1280,    0b0000001010010,    0000001010010_bitlength },
    { 1344,    0b0000001010011,    0000001010011_bitlength },
    { 1408,    0b0000001010100,    0000001010100_bitlength },
    { 1472,    0b0000001010101,    0000001010101_bitlength },
    { 1536,    0b0000001011010,    0000001011010_bitlength },
    { 1600,    0b0000001011011,    0000001011011_bitlength },
    { 1664,    0b0000001100100,    0000001100100_bitlength },
    { 1728,    0b0000001100101,    0000001100101_bitlength },
    { 1792,    0b00000001000,      00000001000_bitlength },
    { 1856,    0b00000001100,      00000001100_bitlength },
    { 1920,    0b00000001101,      00000001101_bitlength },
    { 1984,    0b000000010010,     000000010010_bitlength },
    { 2048,    0b000000010011,     000000010011_bitlength },
    { 2112,    0b000000010100,     000000010100_bitlength },
    { 2176,    0b000000010101,     000000010101_bitlength },
    { 2240,    0b000000010110,     000000010110_bitlength },
    { 2304,    0b000000010111,     000000010111_bitlength },
    { 2368,    0b000000011100,     000000011100_bitlength },
    { 2432,    0b000000011101,     000000011101_bitlength },
    { 2496,    0b000000011110,     000000011110_bitlength },
    { 2560,    0b000000011111,     000000011111_bitlength }
};

PDFCCITTFaxDecoder::PDFCCITTFaxDecoder(const QByteArray* stream, const PDFCCITTFaxDecoderParameters& parameters) :
    m_reader(stream, 1),
    m_parameters(parameters)
{

}

PDFImageData PDFCCITTFaxDecoder::decode()
{
    PDFBitWriter writer(1);
    std::vector<int> codingLine;
    std::vector<int> referenceLine;

    int row = 0;
    const size_t lineSize = m_parameters.columns + 2;
    codingLine.resize(lineSize, m_parameters.columns);
    referenceLine.resize(lineSize, m_parameters.columns);
    bool isUsing2DEncoding = m_parameters.K < 0;
    bool isEndOfLineOccured = m_parameters.hasEndOfLine;
    codingLine[0] = 0;

    auto updateIsUsing2DEncoding = [this, &isUsing2DEncoding]()
    {
        if (m_parameters.K > 0)
        {
            // Mixed encoding
            isUsing2DEncoding = !m_reader.read(1);
        }
    };

    isEndOfLineOccured = skipFillAndEOL() || isEndOfLineOccured;
    updateIsUsing2DEncoding();

    while (!m_reader.isAtEnd())
    {
        int a0_index = 0;
        bool isCurrentPixelBlack = false;

        if (isUsing2DEncoding)
        {
            size_t b1_index = 0;

            // 2D encoding
            while (codingLine[a0_index] < m_parameters.columns)
            {
                CCITT_2D_Code_Mode mode = get2DMode();
                switch (mode)
                {
                    case Pass:
                    {
                        // In this mode, we set a0 to the b2 (from reference line). In pass mode,
                        // we do not change pixel color. Why we are adding 2 to the b1_index?
                        // We want to skip both b1, b2, because they will be left of new a0.
                        const size_t b2_index = b1_index + 1;
                        if (b2_index < referenceLine.size())
                        {
                            addPixels(codingLine, a0_index, referenceLine[b2_index], isCurrentPixelBlack, false);

                            if (referenceLine[b2_index] < m_parameters.columns)
                            {
                                b1_index += 2;

                                if (b1_index >= referenceLine.size())
                                {
                                    throw PDFException(PDFTranslationContext::tr("Invalid pass encoding data in CCITT stream."));
                                }
                            }
                        }
                        else
                        {
                            throw PDFException(PDFTranslationContext::tr("CCITT b2 index out of range."));
                        }

                        break;
                    }

                    case Horizontal:
                    {
                        // We scan two sequence length.
                        const int a0a1 = getRunLength(!isCurrentPixelBlack);
                        const int a1a2 = getRunLength(isCurrentPixelBlack);

                        addPixels(codingLine, a0_index, codingLine[a0_index] + a0a1, isCurrentPixelBlack, false);
                        addPixels(codingLine, a0_index, codingLine[a0_index] + a1a2, !isCurrentPixelBlack, false);

                        while (referenceLine[b1_index] <= codingLine[a0_index] && referenceLine[b1_index] < m_parameters.columns)
                        {
                            // We do not want to change the color (b1 should have opposite color of a0,
                            // should be first changing element of reference line right of a0).
                            b1_index += 2;

                            if (b1_index >= referenceLine.size())
                            {
                                throw PDFException(PDFTranslationContext::tr("Invalid horizontal encoding data in CCITT stream."));
                            }
                        }

                        break;
                    }

                    case Vertical_3L:
                    case Vertical_2L:
                    case Vertical_1L:
                    case Vertical_0:
                    case Vertical_1R:
                    case Vertical_2R:
                    case Vertical_3R:
                    {
                        const int32_t a1 = static_cast<int32_t>(referenceLine[b1_index]) + mode - static_cast<int32_t>(Vertical_0);

                        if (a1 < 0 || a1 > m_parameters.columns)
                        {
                            throw PDFException(PDFTranslationContext::tr("Invalid vertical encoding data in CCITT stream."));
                        }

                        const bool isNegativeOffset = mode < Vertical_0;
                        addPixels(codingLine, a0_index, static_cast<uint32_t>(a1), isCurrentPixelBlack, isNegativeOffset);
                        isCurrentPixelBlack = !isCurrentPixelBlack;

                        if (codingLine[a0_index] < m_parameters.columns)
                        {
                            // We must upgrade b1 index in such a way that it is first index
                            // of opposite color, than a0 index. If we are using negative offsets, then
                            // current position can move backward, and so we must look for first b1 index,
                            // which is of opposite color, than a0. So we decrease index by 1. But what to do,
                            // if we have b1 index equal to zero? In this case, we add -1 + 2 = 1 index, so we do it in
                            // same way, as positive/zero shift.
                            b1_index += (isNegativeOffset && b1_index > 0) ? -1 : 1;

                            // Why we have this check, if same check is in while cycle? Because if we are adding
                            // to the b1_index, we can go outside of reference line range.
                            if (b1_index >= referenceLine.size())
                            {
                                throw PDFException(PDFTranslationContext::tr("Invalid vertical encoding data in CCITT stream."));
                            }

                            while (referenceLine[b1_index] <= codingLine[a0_index] && referenceLine[b1_index] < m_parameters.columns)
                            {
                                // We do not want to change the color (b1 should have opposite color of a0,
                                // should be first changing element of reference line right of a0).
                                b1_index += 2;

                                if (b1_index >= referenceLine.size())
                                {
                                    throw PDFException(PDFTranslationContext::tr("Invalid vertical encoding data in CCITT stream."));
                                }
                            }
                        }

                        break;
                    }

                    default:
                        Q_ASSERT(false);
                        break;
                }
            }
        }
        else
        {
            // Simple 1D encoding
            while (codingLine[a0_index] < m_parameters.columns)
            {
                const uint32_t sequenceLength = getRunLength(!isCurrentPixelBlack);
                addPixels(codingLine, a0_index, codingLine[a0_index] + sequenceLength, isCurrentPixelBlack, false);
                isCurrentPixelBlack = !isCurrentPixelBlack;
            }
        }

        // Write the line to the output buffer
        isCurrentPixelBlack = false;
        int index = 0;
        for (int i = 0; i < m_parameters.columns; ++i)
        {
            if (i == codingLine[index])
            {
                isCurrentPixelBlack = !isCurrentPixelBlack;
                ++index;
            }

            writer.write(isCurrentPixelBlack ? 0 : 1);
        }
        writer.finishLine();

        ++row;

        // Check if we have reached desired number of rows (and end-of-block mode
        // is not set). If yes, then break the reading.
        if (!m_parameters.hasEndOfBlock && row == m_parameters.rows)
        {
            // We have reached number of rows, stop reading the data
            break;
        }

        bool foundEndOfLine = false;
        if (m_parameters.hasEndOfLine)
        {
            // End of line is required, try to scan it (until end of stream is reached).
            while (!m_reader.isAtEnd())
            {
                if (m_reader.look(12) == 1)
                {
                    m_reader.read(12);
                    foundEndOfLine = true;
                    break;
                }
                else
                {
                    m_reader.read(1);
                }
            }
        }
        else if (!m_parameters.hasEncodedByteAlign)
        {
            // Skip fill zeros and possibly find EOL
            foundEndOfLine = skipFillAndEOL();
        }

        // If end of line is found, be do not perform align to bytes (end of line
        // has perference against byte align)
        if (m_parameters.hasEncodedByteAlign && !foundEndOfLine)
        {
            m_reader.alignToBytes();
        }

        if (m_reader.isAtEnd())
        {
            // Have we finished reading?
            break;
        }

        updateIsUsing2DEncoding();

        if (m_parameters.hasEndOfBlock)
        {
            if (!m_parameters.hasEndOfLine && m_parameters.hasEncodedByteAlign)
            {
                // We do not have look for EOL above. We check, if we get end-of-facsimile-block signal.
                // It consists of two consecutive EOLs (see specification).
                if (m_reader.look(24) == 0x1001)
                {
                    // End of block found, stop reading the data
                    break;
                }
            }
            else if (foundEndOfLine)
            {
                if (m_reader.look(12) == 1)
                {
                    // End of block found, stop reading the data
                    break;
                }
            }
        }

        std::swap(codingLine, referenceLine);
        std::fill(codingLine.begin(), codingLine.end(), m_parameters.columns);
        std::fill(std::next(referenceLine.begin(), a0_index + 1), referenceLine.end(), m_parameters.columns);
        codingLine[0] = 0;
    }

    Q_ASSERT(m_parameters.decode.size() == 2);
    std::vector<PDFReal> decode;
    if (m_parameters.hasBlackIsOne)
    {
        decode = { m_parameters.decode[1], m_parameters.decode[0] };
    }
    else
    {
        decode = { m_parameters.decode[0], m_parameters.decode[1] };
    }

    return PDFImageData(1, 1, m_parameters.columns, row, (m_parameters.columns + 7) / 8, m_parameters.maskingType, writer.takeByteArray(), { }, qMove(decode), { });
}

void PDFCCITTFaxDecoder::skipFill()
{
    // This functions skips zero bits (because codewords have at most 12 bits,
    // we use 12 bit lookahead to ensure, that we do not broke data sequence).

    while (!m_reader.isAtEnd() && m_reader.look(12) == 0)
    {
        m_reader.read(1);
    }
}

bool PDFCCITTFaxDecoder::skipEOL()
{
    if (m_reader.look(12) == 1)
    {
        m_reader.read(12);
        return true;
    }

    return false;
}

void PDFCCITTFaxDecoder::addPixels(std::vector<int>& line, int& a0_index, int a1, bool isCurrentPixelBlack, bool isA1LeftOfA0Allowed)
{
    if (a1 > line[a0_index])
    {
        if (a1 > m_parameters.columns)
        {
            throw PDFException(PDFTranslationContext::tr("Invalid index of CCITT changing element a1: a1 = %1, columns = %2.").arg(a1).arg(m_parameters.columns));
        }

        // If we are changing the color, increment a0_index. a0_index == 0 is white, a0_index == 1 is black, etc.,
        // sequence of white and black runs alternates.
        if ((a0_index & 1) != isCurrentPixelBlack)
        {
            ++a0_index;
        }

        line[a0_index] = a1;
    }
    else if (isA1LeftOfA0Allowed && a1 < line[a0_index])
    {
        // We want to find first index, for which it holds:
        //  a1 > line[a0_index - 1], so if we set line[a0_index] = a1,
        // then we get a valid increasing sequence.
        while (a0_index > 0 && a1 <= line[a0_index - 1])
        {
            --a0_index;
        }
        line[a0_index] = a1;
    }
}

uint32_t PDFCCITTFaxDecoder::getRunLength(bool white)
{
    uint32_t value = 0;

    while (true)
    {
        uint32_t currentValue = 0;
        if (white)
        {
            currentValue = getWhiteCode();
        }
        else
        {
            currentValue = getBlackCode();
        }
        value += currentValue;

        if (currentValue < 64)
        {
            break;
        }
    }

    return value;
}

uint32_t PDFCCITTFaxDecoder::getWhiteCode()
{
    return getCode(CCITT_WHITE_CODES, std::size(CCITT_WHITE_CODES));
}

uint32_t PDFCCITTFaxDecoder::getBlackCode()
{
    return getCode(CCITT_BLACK_CODES, std::size(CCITT_BLACK_CODES));
}

uint32_t PDFCCITTFaxDecoder::getCode(const PDFCCITTCode* codes, size_t codeCount)
{
    uint32_t code = 0;
    uint8_t bits = 0;

    while (bits <= MAX_CODE_BIT_LENGTH)
    {
        code = (code << 1) + m_reader.read(1);
        ++bits;

        for (size_t i = 0; i < codeCount; ++i)
        {
            const PDFCCITTCode& currentCode = codes[i];
            if (currentCode.bits == bits && currentCode.code == code)
            {
                return currentCode.length;
            }
        }
    }

    throw PDFException(PDFTranslationContext::tr("Invalid CCITT run length code word."));
}

CCITT_2D_Code_Mode PDFCCITTFaxDecoder::get2DMode()
{
    uint32_t code = 0;
    uint8_t bits = 0;

    while (bits <= MAX_2D_MODE_BIT_LENGTH)
    {
        code = (code << 1) + m_reader.read(1);
        ++bits;

        for (const PDFCCITT2DModeInfo& info : CCITT_2D_CODE_MODES)
        {
            if (info.bits == bits && info.code == code)
            {
                return info.mode;
            }
        }
    }

    throw PDFException(PDFTranslationContext::tr("Invalid CCITT 2D mode."));
}

}   // namespace pdf
