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

#ifndef PDFJBIG2DECODER_H
#define PDFJBIG2DECODER_H

#include "pdfutils.h"
#include "pdfcolorspaces.h"

#include <optional>

namespace pdf
{
class PDFJBIG2Bitmap;
class PDFRenderErrorReporter;
class PDFJBIG2HuffmanCodeTable;
class PDFJBIG2SymbolDictionary;
class PDFJBIG2PatternDictionary;

struct PDFJBIG2HuffmanTableEntry;
struct PDFJBIG2BitmapDecodingParameters;
struct PDFJBIG2TextRegionDecodingParameters;
struct PDFJBIG2BitmapRefinementDecodingParameters;

enum class PDFJBIG2BitOperation
{
    Invalid,
    Or,
    And,
    Xor,
    NotXor,
    Replace
};

struct PDFJBIG2HuffmanTableEntry
{
    enum class Type : uint8_t
    {
        Standard,
        Negative,
        OutOfBand
    };

    /// Returns true, if current row represents interval (-∞, value),
    /// it means 32bit number must be read and
    bool isLowValue() const { return type == Type::Negative; }

    /// Returns true, if current row represents out-of-band value
    bool isOutOfBand() const { return type == Type::OutOfBand; }

    int32_t value = 0;              ///< Base value
    uint16_t prefixBitLength = 0;   ///< Bit length of prefix
    uint16_t rangeBitLength = 0;    ///< Bit length of additional value
    uint16_t prefix = 0;            ///< Bit prefix of the huffman code
    Type type = Type::Standard;     ///< Type of the value
};

/// Arithmetic decoder state for JBIG2 data streams. It contains state for context,
/// state is stored as 8-bit value, where only 7 bits are used. 6 bits are used
/// to store Qe value index (current row in the table, number 0-46), and lowest 1 bit
/// is used to store current MPS value (most probable symbol - 0/1).
class PDF4QTLIBCORESHARED_EXPORT PDFJBIG2ArithmeticDecoderState
{
public:
    explicit inline PDFJBIG2ArithmeticDecoderState() = default;
    explicit inline PDFJBIG2ArithmeticDecoderState(size_t size) :
        m_state(size, 0)
    {

    }

    /// Resets the context
    inline void reset(const uint8_t bits)
    {
        size_t size = (1ULL << bits);
        std::fill(m_state.begin(), m_state.end(), 0);
        if (m_state.size() != size)
        {
            m_state.resize(size, 0);
        }
    }

    /// Resets the context using another context
    inline void reset(const uint8_t bits, const PDFJBIG2ArithmeticDecoderState& other)
    {
        reset(bits);

        const size_t size = qMin(m_state.size(), other.m_state.size());
        std::copy(other.m_state.begin(), other.m_state.begin() + size, m_state.begin());
    }

    /// Returns row index to Qe value table, according to document ISO/IEC 14492:2001,
    /// annex E, table E.1 (Qe values and probability estimation process).
    inline uint8_t getQeRowIndex(size_t context) const
    {
        Q_ASSERT(context < m_state.size());
        return m_state[context] >> 1;
    }

    /// Returns Qe value for row index, according to document ISO/IEC 14492:2001,
    /// annex E, table E.1 (Qe values and probability estimation process).
    inline uint32_t getQe(size_t context) const;

    /// Returns current bit value of MPS (most probable symbol)
    inline uint8_t getMPS(size_t context) const
    {
        Q_ASSERT(context < m_state.size());
        return m_state[context] & 0x1;
    }

    /// Sets current row index to Qe value table, at given context, and also MPS bit value
    /// (most probable symbol).
    inline void setQeRowIndexAndMPS(size_t context, uint8_t QeRowIndex, uint8_t MPS)
    {
        Q_ASSERT(context < m_state.size());
        Q_ASSERT(MPS < 2);
        m_state[context] = (QeRowIndex << 1) + MPS;
    }

private:
    std::vector<uint8_t> m_state;
};

/// Arithmetic decoder for JBIG2 data streams. This arithmetic decoder is implementation
/// of decoder described in document ISO/IEC 14492:2001, T.88, annex G (arithmetic decoding
/// procedure). It uses 32-bit fixed point arithmetic instead of 16-bit fixed point
/// arithmetic described in the specification (it is much faster).
class PDF4QTLIBCORESHARED_EXPORT PDFJBIG2ArithmeticDecoder
{
public:
    explicit inline PDFJBIG2ArithmeticDecoder(PDFBitReader* reader) :
        m_c(0),
        m_a(0),
        m_ct(0),
        m_lastByte(0),
        m_reader(reader)
    {

    }

    void initialize() { perform_INITDEC(); }
    uint32_t readBit(size_t context, PDFJBIG2ArithmeticDecoderState* state) { return perform_DECODE(context, state); }
    uint32_t readByte(size_t context, PDFJBIG2ArithmeticDecoderState* state);

    uint32_t getRegisterC() const { return m_c; }
    uint32_t getRegisterA() const { return m_a; }
    uint32_t getRegisterCT() const { return m_ct; }

    int32_t getIAID(uint32_t size, PDFJBIG2ArithmeticDecoderState* state);
    std::optional<int32_t> getSignedInteger(PDFJBIG2ArithmeticDecoderState* state);

    /// This function is used to read last byte of byte sequence { 0xFF, 0xAC },
    /// when finishing data stream.
    void finalize();

private:
    /// Performs INITDEC operation as described in the specification
    void perform_INITDEC();

    /// Performs BYTEIN operation as described in the specification
    void perform_BYTEIN();

    /// Performs DECODE operation as described in the specification
    /// \param context Context index
    /// \param state State of the arithmetic decoder
    /// \returns Single decoded bit (lowest bit, other bits are zero)
    uint32_t perform_DECODE(size_t context, PDFJBIG2ArithmeticDecoderState* state);

    /// This is 32 bit register consisting of two 16-bit subregisters - "c_high" and "c_low", as
    /// it is in specification. But we can work with it as 32 bit register (if we adjust some
    /// operations and fixed point arithmetic).
    uint32_t m_c;

    /// This is 32 bit register for interval range. In the specification, it is 16-bit register,
    /// but we use 32-bit fixed point arithmetic instead of 16-bit fixed point arithmetic.
    uint32_t m_a;

    /// Number of current unprocessed bits.
    uint32_t m_ct;

    /// Last processed byte
    uint8_t m_lastByte;

    /// Data source to read from
    PDFBitReader* m_reader;
};

enum class JBIG2SegmentType : uint32_t
{
    Invalid,
    SymbolDictionary,           ///< See chapter 7.4.2  in specification
    TextRegion,                 ///< See chapter 7.4.3  in specification
    PatternDictionary,          ///< See chapter 7.4.4  in specification
    HalftoneRegion,             ///< See chapter 7.4.5  in specification
    GenericRegion,              ///< See chapter 7.4.6  in specification
    GenericRefinementRegion,    ///< See chapter 7.4.7  in specification
    PageInformation,            ///< See chapter 7.4.8  in specification
    EndOfPage,                  ///< See chapter 7.4.9  in specification
    EndOfStripe,                ///< See chapter 7.4.10 in specification
    EndOfFile,                  ///< See chapter 7.4.11 in specification
    Profiles,                   ///< See chapter 7.4.12 in specification
    Tables,                     ///< See chapter 7.4.13 in specification
    Extension                   ///< See chapter 7.4.14 in specification
};

class PDFJBIG2SegmentHeader
{
public:
    explicit inline PDFJBIG2SegmentHeader() = default;

    /// Returns segment type
    inline JBIG2SegmentType getSegmentType() const { return m_segmentType; }

    /// Returns segment number
    inline uint32_t getSegmentNumber() const { return m_segmentNumber; }

    /// Returns segment data length (or 0xFFFFFFFF, if length is not defined)
    /// \sa isSegmentDataLengthDefined
    inline uint32_t getSegmentDataLength() const { return m_segmentDataLength; }

    /// Returns true, if segment is immediate (direct paint on page's bitmap)
    inline bool isImmediate() const { return m_immediate; }

    /// Returns true, if segment is lossless
    inline bool isLossless() const { return m_lossless; }

    /// Returns true, if segmend data length is defined
    inline bool isSegmentDataLengthDefined() const { return m_segmentDataLength != 0xFFFFFFFF; }

    /// Returns referred segments
    inline const std::vector<uint32_t>& getReferredSegments() const { return m_referredSegments; }

    /// Reads the segment header from the data stream. If error occurs, then
    /// exception is thrown.
    static PDFJBIG2SegmentHeader read(PDFBitReader* reader);

private:
    uint32_t m_segmentNumber = 0;
    uint32_t m_pageAssociation = 0;
    uint32_t m_segmentDataLength = 0;
    JBIG2SegmentType m_segmentType = JBIG2SegmentType::Invalid;
    bool m_immediate = false;
    bool m_lossless = false;
    std::vector<uint32_t> m_referredSegments;
};

class PDFJBIG2Segment
{
public:
    explicit inline PDFJBIG2Segment() = default;
    virtual ~PDFJBIG2Segment();

    virtual const PDFJBIG2Bitmap* asBitmap() const { return nullptr; }
    virtual PDFJBIG2Bitmap* asBitmap() { return nullptr; }

    virtual const PDFJBIG2HuffmanCodeTable* asHuffmanCodeTable() const { return nullptr; }
    virtual PDFJBIG2HuffmanCodeTable* asHuffmanCodeTable() { return nullptr; }

    virtual const PDFJBIG2SymbolDictionary* asSymbolDictionary() const { return nullptr; }
    virtual PDFJBIG2SymbolDictionary* asSymbolDictionary() { return nullptr; }

    virtual const PDFJBIG2PatternDictionary* asPatternDictionary() const { return nullptr; }
    virtual PDFJBIG2PatternDictionary* asPatternDictionary() { return nullptr; }
};

/// Huffman decoder - can decode integers / out of band values from huffman table.
class PDFJBIG2HuffmanDecoder
{
public:
    explicit inline PDFJBIG2HuffmanDecoder() = default;

    /// Constructs huffman decoder from static tables, so no memory are allocated (vector is empty)
    explicit inline PDFJBIG2HuffmanDecoder(PDFBitReader* reader, const PDFJBIG2HuffmanTableEntry* begin, const PDFJBIG2HuffmanTableEntry* end) :
        m_reader(reader),
        m_begin(begin),
        m_end(end)
    {

    }

    /// Constructs huffman decoder from huffman code table, in this case, memory is allocated
    explicit PDFJBIG2HuffmanDecoder(PDFBitReader* reader, const PDFJBIG2HuffmanCodeTable* table);

    /// Constructs huffman decoder from huffman code table, in this case, memory is allocated
    explicit PDFJBIG2HuffmanDecoder(PDFBitReader* reader, std::vector<PDFJBIG2HuffmanTableEntry>&& table);

    PDFJBIG2HuffmanDecoder(const PDFJBIG2HuffmanDecoder&) = delete;
    PDFJBIG2HuffmanDecoder(PDFJBIG2HuffmanDecoder&& other);

    PDFJBIG2HuffmanDecoder& operator=(const PDFJBIG2HuffmanDecoder&) = delete;
    PDFJBIG2HuffmanDecoder& operator=(PDFJBIG2HuffmanDecoder&& other);

    /// Returns true, if huffman table is valid (and usable)
    bool isValid() const { return m_begin != m_end; }

    /// Returns true, if huffman table has out-of-band value
    bool isOutOfBandSupported() const;

    /// Tries to read signed integer using the table and current reader.
    /// \returns Integer, or out-of-band value, using the std::optional semantics
    std::optional<int32_t> readSignedInteger();

private:
    /// Data source to read from
    PDFBitReader* m_reader = nullptr;

    const PDFJBIG2HuffmanTableEntry* m_begin = nullptr;
    const PDFJBIG2HuffmanTableEntry* m_end = nullptr;
    std::vector<PDFJBIG2HuffmanTableEntry> m_entries;
};

class PDF4QTLIBCORESHARED_EXPORT PDFJBIG2Bitmap : public PDFJBIG2Segment
{
public:
    explicit PDFJBIG2Bitmap();
    explicit PDFJBIG2Bitmap(int width, int height);
    explicit PDFJBIG2Bitmap(int width, int height, uint8_t fill);
    virtual ~PDFJBIG2Bitmap() override;

    virtual const PDFJBIG2Bitmap* asBitmap() const override { return this; }
    virtual PDFJBIG2Bitmap* asBitmap() override { return this; }

    inline int getWidth() const { return m_width; }
    inline int getHeight() const { return m_height; }
    inline int getPixelCount() const { return m_width * m_height; }
    inline uint8_t getPixel(int x, int y) const { return m_data[y * m_width + x]; }
    inline void setPixel(int x, int y, uint8_t value) { m_data[y * m_width + x] = value; }

    inline uint8_t getPixelSafe(int x, int y) const
    {
        if (x < 0 || x >= m_width || y < 0 || y >= m_height)
        {
            return 0;
        }

        return getPixel(x, y);
    }

    inline void fill(uint8_t value) { std::fill(m_data.begin(), m_data.end(), value); }
    inline void fillZero() { fill(0); }
    inline void fillOne() { fill(0xFF); }

    inline bool isValid() const { return getPixelCount() > 0; }

    /// Returns subbitmap of this bitmap. If some pixels of subbitmap are outside
    /// of current bitmap, then they are reset to zero.
    /// \param offsetX Horizontal offset of subbitmap
    /// \param offsetY Vertical offset of subbitmap
    /// \param width Width of subbitmap
    /// \param height Height of subbitmap
    PDFJBIG2Bitmap getSubbitmap(int offsetX, int offsetY, int width, int height) const;

    /// Paints another bitmap onto this bitmap. If bitmap is invalid, nothing is done.
    /// If \p expandY is true, height of target bitmap is expanded to fit source draw area.
    /// \param bitmap Bitmap to be painted on this
    /// \param offsetX Horizontal offset of paint area
    /// \param offsetY Vertical offset of paint area
    /// \param operation Paint operation to be performed
    /// \param expandY Expand vertically, if painted bitmap exceeds current bitmap area
    /// \param expandPixel Initialize pixels by this value during expanding
    void paint(const PDFJBIG2Bitmap& bitmap, int offsetX, int offsetY, PDFJBIG2BitOperation operation, bool expandY, const uint8_t expandPixel);

    /// Copies data from source row to target row. If source or target row doesn't exists,
    /// then exception is thrown.
    /// \param target Target row
    /// \param source Source row
    void copyRow(int target, int source);

private:
    int m_width;
    int m_height;
    std::vector<uint8_t> m_data;
};

struct PDFJBIG2ReferencedSegments
{
    std::vector<const PDFJBIG2Bitmap*> bitmaps;
    std::vector<const PDFJBIG2HuffmanCodeTable*> codeTables;
    std::vector<const PDFJBIG2SymbolDictionary*> symbolDictionaries;
    std::vector<const PDFJBIG2PatternDictionary*> patternDictionaries;
    size_t currentUserCodeTableIndex = 0;

    /// Returns symbol bitmaps from all symbol dictionaries
    std::vector<const PDFJBIG2Bitmap*> getSymbolBitmaps() const;

    /// Returns pattern bitmaps from all pattern dictionaries
    std::vector<const PDFJBIG2Bitmap*> getPatternBitmaps() const;

    /// Returns current user huffman table according the index. If index
    /// is out of range, then exception is thrown.
    PDFJBIG2HuffmanDecoder getUserTable(PDFBitReader* reader);
};

/// Region segment information field, see chapter 7.4.1 in the specification
struct PDFJBIG2RegionSegmentInformationField
{
    uint32_t width = 0;
    uint32_t height = 0;
    int32_t offsetX = 0;
    int32_t offsetY = 0;
    PDFJBIG2BitOperation operation = PDFJBIG2BitOperation::Invalid;
};

/// Info structure for adaptative template
struct PDFJBIG2ATPosition
{
    int8_t x = 0;
    int8_t y = 0;
};

using PDFJBIG2ATPositions = std::array<PDFJBIG2ATPosition, 4>;

/// Decoder of JBIG2 data streams. Decodes the black/white monochrome image.
/// Handles also global segments. Decoder decodes data using the specification
/// ISO/IEC 14492:2001, T.88.
class PDF4QTLIBCORESHARED_EXPORT PDFJBIG2Decoder
{
public:
    explicit inline PDFJBIG2Decoder(QByteArray data, QByteArray globalData, PDFRenderErrorReporter* errorReporter) :
        m_data(qMove(data)),
        m_globalData(qMove(globalData)),
        m_errorReporter(errorReporter),
        m_reader(nullptr, 8),
        m_pageDefaultPixelValue(0),
        m_pageDefaultCompositionOperator(PDFJBIG2BitOperation::Invalid),
        m_pageDefaultCompositionOperatorOverriden(false),
        m_pageSizeUndefined(false)
    {

    }

    PDFJBIG2Decoder(const PDFJBIG2Decoder&) = delete;
    PDFJBIG2Decoder(PDFJBIG2Decoder&&) = default;

    PDFJBIG2Decoder& operator=(const PDFJBIG2Decoder&) = delete;
    PDFJBIG2Decoder& operator=(PDFJBIG2Decoder&&) = default;

    ~PDFJBIG2Decoder();

    /// Decodes image interpreting the data as JBIG2 data stream. If image cannot
    /// be decoded, exception is thrown (or invalid PDFImageData is returned).
    /// \param maskingType Image masking type
    PDFImageData decode(PDFImageData::MaskingType maskingType);

    /// Decodes image interpreting the data as JBIG2 file stream (not data stream).
    /// Decoding procedure also handles file header/file flags and number of pages.
    /// If number of pages is invalid, then exception is thrown.
    PDFImageData decodeFileStream();

private:
    static constexpr const uint32_t MAX_BITMAP_SIZE = 65536;

    /// Processes current data stream (reads all data from the stream, interprets
    /// them as segments and processes the segments).
    void processStream();

    void processSymbolDictionary(const PDFJBIG2SegmentHeader& header);
    void processTextRegion(const PDFJBIG2SegmentHeader& header);
    void processPatternDictionary(const PDFJBIG2SegmentHeader& header);
    void processHalftoneRegion(const PDFJBIG2SegmentHeader& header);
    void processGenericRegion(const PDFJBIG2SegmentHeader& header);
    void processGenericRefinementRegion(const PDFJBIG2SegmentHeader& header);
    void processPageInformation(const PDFJBIG2SegmentHeader& header);
    void processEndOfPage(const PDFJBIG2SegmentHeader& header);
    void processEndOfStripe(const PDFJBIG2SegmentHeader& header);
    void processEndOfFile(const PDFJBIG2SegmentHeader& header);
    void processProfiles(const PDFJBIG2SegmentHeader& header);
    void processCodeTables(const PDFJBIG2SegmentHeader& header);
    void processExtension(const PDFJBIG2SegmentHeader& header);

    /// Returns bitmap for given segment index. If bitmap is not found, or segment
    /// is of different type, then exception is thrown.
    /// \param segmentIndex Segment index with bitmap
    /// \param remove Remove the segment?
    PDFJBIG2Bitmap getBitmap(const uint32_t segmentIndex, bool remove);

    /// Reads bitmap using decoding parameters
    /// \param parameters Decoding parameters
    PDFJBIG2Bitmap readBitmap(PDFJBIG2BitmapDecodingParameters& parameters);

    /// Reads refined bitmap using decoding parameters
    /// \param parameters Decoding parameters
    PDFJBIG2Bitmap readRefinementBitmap(PDFJBIG2BitmapRefinementDecodingParameters& parameters);

    /// Reads text bitmap using decoding parameters
    /// \param parameters Decoding parameters
    PDFJBIG2Bitmap readTextBitmap(PDFJBIG2TextRegionDecodingParameters& parameters);

    /// Reads the region segment information field (see chapter 7.4.1)
    PDFJBIG2RegionSegmentInformationField readRegionSegmentInformationField();

    /// Read adaptative pixel template positions, positions, which are not read, are filled with 0
    PDFJBIG2ATPositions readATTemplatePixelPositions(int count);

    /// Skip segment data
    void skipSegment(const PDFJBIG2SegmentHeader& header);

    /// Returns structure containing referenced segments. If segment numbers
    /// are wrong, or invalid segments appears, then exception is thrown.
    /// \param header Header, from which referred segments are read
    PDFJBIG2ReferencedSegments getReferencedSegments(const PDFJBIG2SegmentHeader& header) const;

    static void checkBitmapSize(const uint32_t size);
    static void checkRegionSegmentInformationField(const PDFJBIG2RegionSegmentInformationField& field);
    static int32_t checkInteger(std::optional<int32_t> value);

    QByteArray m_data;
    QByteArray m_globalData;
    PDFRenderErrorReporter* m_errorReporter;
    PDFBitReader m_reader;
    std::map<uint32_t, std::unique_ptr<PDFJBIG2Segment>> m_segments;
    uint8_t m_pageDefaultPixelValue;
    PDFJBIG2BitOperation m_pageDefaultCompositionOperator;
    bool m_pageDefaultCompositionOperatorOverriden;
    bool m_pageSizeUndefined;
    PDFJBIG2Bitmap m_pageBitmap;
};

}   // namespace pdf

#endif // PDFJBIG2DECODER_H
