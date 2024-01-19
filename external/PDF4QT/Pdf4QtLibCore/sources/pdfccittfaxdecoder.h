//    Copyright (C) 2019-2021 Jakub Melka
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

#ifndef PDFCCITTFAXDECODER_H
#define PDFCCITTFAXDECODER_H

#include "pdfutils.h"
#include "pdfimage.h"

namespace pdf
{

struct PDFCCITTCode;

struct PDFCCITTFaxDecoderParameters
{
    /// Type of encoding. Has this meaning:
    ///    K < 0 - pure two dimensional encoding (Group 4)
    ///    K = 0 - pure one dimensional encoding
    ///    K > 0 - mixed encoding; one dimensional encoded line can be followed by at most K - 1 two dimensional encoded lines
    PDFInteger K = 0;

    /// Pixel width of the image. Default value is 1728.
    PDFInteger columns = 1728;

    /// Pixel height of the image. This value can be zero or be absent, in this case,
    /// end of block pattern must be present and end the stream.
    PDFInteger rows = 0;

    /// This parameter is ignored in this library. If positive, and \p hasEndOfLine is true,
    /// and K is nonnegative, then if error occurs, end-of-line pattern is searched and
    /// data are copied from previous line, or are set to white, if previous line is also damaged.
    PDFInteger damagedRowsBeforeError = 0;

    /// Flag indicating, that end of line patterns are required in the encoded data.
    /// Stream filter must always accept end of line patterns, but require them only,
    /// if this flag is set to true.
    bool hasEndOfLine = false;

    /// Flag indicating that lines are byte aligned, i.e. 0 bits are inserted before each line
    /// to achieve byte alignment.
    bool hasEncodedByteAlign = false;

    /// Flag indicating, that filter expects the data be terminated by end of block bit pattern.
    /// In this case, \p rows parameter is ignored. Otherwise, rows parameter is used, or image
    /// is terminated by end of data stream, whichever occurs first. The end of block is marked
    /// as end-of-facsimile block (EOFB), or return to control (RTC), according the K parameter.
    bool hasEndOfBlock = true;

    /// If this flag is true, then 1 means black pixel, 0 white pixel. Otherwise, if false,
    /// then 0 means black pixel and 1 white pixel.
    bool hasBlackIsOne = false;

    /// Decode
    std::vector<PDFReal> decode;

    /// Masking type
    PDFImageData::MaskingType maskingType = PDFImageData::MaskingType::None;
};

enum CCITT_2D_Code_Mode
{
    Pass,
    Horizontal,
    Vertical_3L,
    Vertical_2L,
    Vertical_1L,
    Vertical_0,
    Vertical_1R,
    Vertical_2R,
    Vertical_3R,
    Invalid
};

class PDFCCITTFaxDecoder
{
public:
    explicit PDFCCITTFaxDecoder(const QByteArray* stream, const PDFCCITTFaxDecoderParameters& parameters);

    PDFImageData decode();

    const PDFBitReader* getReader() const { return &m_reader; }

private:
    /// Skip zero bits at the start
    void skipFill();

    /// Skip end-of-line, if occured. Returns true, if EOL was skipped.
    bool skipEOL();

    /// Skip fill bits and then try to skip EOL. If EOL is found, then
    /// true is returned, otherwise false is returned.
    bool skipFillAndEOL() { skipFill(); return skipEOL(); }

    /// Add pixels to the line.
    /// \param line Line with changing element indices
    /// \param a0_index Reference changing element index (index to the \p line array)
    /// \param a1 Current changing element index (column index, not index to the \p line array)
    /// \param isCurrentPixelBlack Are pixels black?
    /// \param isA1LeftOfA0Allowed Allow a1 to be left of a0 (not a0_index, but line[a0_index], which is a0)
    void addPixels(std::vector<int>& line, int& a0_index, int a1, bool isCurrentPixelBlack, bool isA1LeftOfA0Allowed);

    /// Get 2D mode from the stream
    CCITT_2D_Code_Mode get2DMode();

    uint32_t getRunLength(bool white);

    uint32_t getWhiteCode();
    uint32_t getBlackCode();

    uint32_t getCode(const PDFCCITTCode* codes, size_t codeCount);

    PDFBitReader m_reader;
    PDFCCITTFaxDecoderParameters m_parameters;
};

}   // namespace pdf

#endif // PDFCCITTFAXDECODER_H
