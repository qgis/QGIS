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

#include "pdfstreamfilters.h"
#include "pdfexception.h"
#include "pdfconstants.h"
#include "pdfparser.h"
#include "pdfsecurityhandler.h"
#include "pdfutils.h"

#include <zlib.h>

#include <QtEndian>

#include "pdfdbgheap.h"

namespace pdf
{

QByteArray PDFAsciiHexDecodeFilter::apply(const QByteArray& data,
                                          const PDFObjectFetcher& objectFetcher,
                                          const PDFObject& parameters,
                                          const PDFSecurityHandler* securityHandler) const
{
    Q_UNUSED(objectFetcher);
    Q_UNUSED(parameters);
    Q_UNUSED(securityHandler);

    const int indexOfEnd = data.indexOf('>');
    const int size = (indexOfEnd == -1) ? data.size() : indexOfEnd;

    if (size % 2 == 1)
    {
        // We must add trailing zero to the buffer
        QByteArray temporaryData(data.constData(), size);
        temporaryData.push_back('0');
        return QByteArray::fromHex(temporaryData);
    }
    else if (size == data.size())
    {
        // We do this, because we do not want to allocate unnecessary buffer for this case.
        // This case should be common.
        return QByteArray::fromHex(data);
    }

    return QByteArray::fromHex(QByteArray::fromRawData(data.constData(), size));
}

QByteArray PDFAscii85DecodeFilter::apply(const QByteArray& data,
                                         const PDFObjectFetcher& objectFetcher,
                                         const PDFObject& parameters,
                                         const PDFSecurityHandler* securityHandler) const
{
    Q_UNUSED(objectFetcher);
    Q_UNUSED(parameters);
    Q_UNUSED(securityHandler);

    const unsigned char* dataBegin = reinterpret_cast<const unsigned char*>(data.constData());
    const unsigned char* dataEnd = reinterpret_cast<const unsigned char*>(data.constData() + data.size());

    const unsigned char* it = dataBegin;
    const constexpr uint32_t STREAM_END = 0xFFFFFFFF;

    auto getChar = [&it, dataEnd]() -> uint32_t
    {
        // Skip whitespace characters
        while (it != dataEnd && PDFLexicalAnalyzer::isWhitespace(*it))
        {
            ++it;
        }

        if (it == dataEnd || (*it == '~'))
        {
            return STREAM_END;
        }

        return *it++;
    };

    QByteArray result;
    result.reserve(data.size() * 4 / 5);

    while (true)
    {
        const uint32_t scannedChar = getChar();
        if (scannedChar == STREAM_END)
        {
            break;
        }
        else if (scannedChar == 'z')
        {
            result.append(4, static_cast<char>(0));
        }
        else
        {
            // Scan all 5 characters, some of then can be equal to STREAM_END constant. We will
            // treat all these characters as last character.
            std::array<uint32_t, 5> scannedChars;
            scannedChars.fill(84);
            scannedChars[0] = scannedChar - 33;
            std::size_t validBytes = 0;
            for (auto it2 = std::next(scannedChars.begin()); it2 != scannedChars.end(); ++it2)
            {
                uint32_t character = getChar();
                if (character == STREAM_END)
                {
                    break;
                }
                *it2 = character - 33;
                ++validBytes;
            }

            // Decode bytes using 85 base
            uint32_t decodedBytesPacked = 0;
            for (const uint32_t value : scannedChars)
            {
                decodedBytesPacked = decodedBytesPacked * 85 + value;
            }

            // Decode bytes into byte array
            std::array<char, 4> decodedBytesUnpacked;
            decodedBytesUnpacked.fill(0);
            for (auto byteIt = decodedBytesUnpacked.rbegin(); byteIt != decodedBytesUnpacked.rend(); ++byteIt)
            {
                *byteIt = static_cast<char>(decodedBytesPacked & 0xFF);
                decodedBytesPacked = decodedBytesPacked >> 8;
            }

            Q_ASSERT(validBytes <= decodedBytesUnpacked.size());
            for (std::size_t i = 0; i < validBytes; ++i)
            {
                result.push_back(decodedBytesUnpacked[i]);
            }
        }
    }

    return result;
}

class PDFLzwStreamDecoder
{
public:
    explicit PDFLzwStreamDecoder(const QByteArray& inputByteArray, uint32_t early);

    QByteArray decompress();

private:
    static constexpr const uint32_t CODE_TABLE_RESET = 256;
    static constexpr const uint32_t CODE_END_OF_STREAM = 257;

    // Maximal code size is 12 bits. so we can have 2^12 = 4096 items
    // in the table (some items are unused, for example 256, 257). We also
    // need to initialize items under code 256, because we treat them specially,
    // they are not initialized in the decompress.
    static constexpr const uint32_t TABLE_SIZE = 4096;

    /// Clears the input data table
    void clearTable();

    /// Returns a newly scanned code
    uint32_t getCode();

    struct TableItem
    {
        uint32_t previous = TABLE_SIZE;
        char character = 0;
    };

    std::array<TableItem, TABLE_SIZE> m_table;
    std::array<char, TABLE_SIZE> m_sequence;

    uint32_t m_nextCode;        ///< Next code value (to be written into the table)
    uint32_t m_nextBits;        ///< Number of bits of the next code
    uint32_t m_early;           ///< Early (see PDF 1.7 Specification, this constant is 0 or 1, based on the dictionary value)
    uint32_t m_inputBuffer;     ///< Input buffer, containing bits, which were read from the input byte array
    uint32_t m_inputBits;       ///< Number of bits in the input buffer.
    std::array<char, TABLE_SIZE>::iterator m_currentSequenceEnd;
    bool m_first;               ///< Are we reading from stream for first time after the reset
    char m_newCharacter;        ///< New character to be written
    int m_position;             ///< Position in the input array
    const QByteArray& m_inputByteArray;
};

PDFLzwStreamDecoder::PDFLzwStreamDecoder(const QByteArray& inputByteArray, uint32_t early) :
    m_table(),
    m_sequence(),
    m_nextCode(0),
    m_nextBits(0),
    m_early(early),
    m_inputBuffer(0),
    m_inputBits(0),
    m_currentSequenceEnd(m_sequence.begin()),
    m_first(false),
    m_newCharacter(0),
    m_position(0),
    m_inputByteArray(inputByteArray)
{
    for (size_t i = 0; i < 256; ++i)
    {
        m_table[i].character = static_cast<char>(i);
        m_table[i].previous = TABLE_SIZE;
    }

    clearTable();
}

QByteArray PDFLzwStreamDecoder::decompress()
{
    QByteArray result;

    // Guess output byte array size - assume compress ratio is 2:1
    result.reserve(m_inputByteArray.size() * 2);

    uint32_t previousCode = TABLE_SIZE;
    while (true)
    {
        const uint32_t code = getCode();

        if (code == CODE_END_OF_STREAM)
        {
            // We are at end of stream
            break;
        }
        else if (code == CODE_TABLE_RESET)
        {
            // Just reset the table
            clearTable();
            continue;
        }

        // Normal operation code
        if (code < m_nextCode)
        {
            m_currentSequenceEnd = m_sequence.begin();

            for (uint32_t currentCode = code; currentCode != TABLE_SIZE; currentCode = m_table[currentCode].previous)
            {
                *m_currentSequenceEnd++ = m_table[currentCode].character;
            }

            // We must reverse the sequence, because we stored it in the
            // linked list, which we traversed from last to first item.
            std::reverse(m_sequence.begin(), m_currentSequenceEnd);
        }
        else if (code == m_nextCode)
        {
            // We use the buffer from previous run, just add a new
            // character to the end.
            *m_currentSequenceEnd++ = m_newCharacter;
        }
        else
        {
            // Unknown code
            throw PDFException(PDFTranslationContext::tr("Invalid code in the LZW stream."));
        }
        m_newCharacter = m_sequence.front();

        if (m_first)
        {
            m_first = false;
        }
        else
        {
            // Add a new word in the dictionary, if we have it
            if (m_nextCode < TABLE_SIZE)
            {
                m_table[m_nextCode].character = m_newCharacter;
                m_table[m_nextCode].previous = previousCode;
                ++m_nextCode;
            }

            // Change bit size of the code, if it is neccessary
            switch (m_nextCode + m_early)
            {
                case 512:
                    m_nextBits = 10;
                    break;

                case 1024:
                    m_nextBits = 11;
                    break;

                case 2048:
                    m_nextBits = 12;
                    break;

                default:
                    break;
            }
        }

        previousCode = code;

        // Copy the input array to the buffer
        std::copy(m_sequence.begin(), m_currentSequenceEnd, std::back_inserter(result));
    }

    result.shrink_to_fit();
    return result;
}

void PDFLzwStreamDecoder::clearTable()
{
    // We do not clear the m_table array here. It is for performance reasons, we assume
    // the input is correct. We also do not clear the sequence buffer here.

    m_nextCode = 258;
    m_nextBits = 9;
    m_first = true;
    m_newCharacter = 0;
}

uint32_t PDFLzwStreamDecoder::getCode()
{
    while (m_inputBits < m_nextBits)
    {
        // Did we reach end of array?
        if (m_position == m_inputByteArray.size())
        {
            return CODE_END_OF_STREAM;
        }

        m_inputBuffer = (m_inputBuffer << 8) | static_cast<unsigned char>(m_inputByteArray[m_position++]);
        m_inputBits += 8;
    }

    // We must omit bits from left (old ones) and right (newly scanned ones) and
    // read just m_nextBits bits. Mask should omit the old ones and shift (m_inputBits - m_nextBits)
    // should omit the new ones.
    const uint32_t mask = ((1 << m_nextBits) - 1);
    const uint32_t code = (m_inputBuffer >> (m_inputBits - m_nextBits)) & mask;
    m_inputBits -= m_nextBits;
    return code;
}

QByteArray PDFLzwDecodeFilter::apply(const QByteArray& data,
                                     const PDFObjectFetcher& objectFetcher,
                                     const PDFObject& parameters,
                                     const PDFSecurityHandler* securityHandler) const
{
    Q_UNUSED(securityHandler);

    uint32_t early = 1;

    const PDFObject& dereferencedParameters = objectFetcher(parameters);
    if (dereferencedParameters.isDictionary())
    {
        const PDFDictionary* dictionary = dereferencedParameters.getDictionary();

        const PDFObject& earlyChangeObject = objectFetcher(dictionary->get("EarlyChange"));
        if (earlyChangeObject.isInt())
        {
            early = earlyChangeObject.getInteger();
        }
    }

    PDFStreamPredictor predictor = PDFStreamPredictor::createPredictor(objectFetcher, parameters);
    PDFLzwStreamDecoder decoder(data, early);
    return predictor.apply(decoder.decompress());
}

QByteArray PDFFlateDecodeFilter::apply(const QByteArray& data,
                                       const PDFObjectFetcher& objectFetcher,
                                       const PDFObject& parameters,
                                       const PDFSecurityHandler* securityHandler) const
{
    Q_UNUSED(securityHandler);

    PDFStreamPredictor predictor = PDFStreamPredictor::createPredictor(objectFetcher, parameters);
    return predictor.apply(uncompress(data));
}

QByteArray PDFFlateDecodeFilter::compress(const QByteArray& decompressedData)
{
    QByteArray result;

    z_stream stream = { };
    stream.next_in = const_cast<Bytef*>(convertByteArrayToUcharPtr(decompressedData));
    stream.avail_in = decompressedData.size();

    std::array<Bytef, 1024> outputBuffer = { };

    int error = deflateInit(&stream, Z_BEST_COMPRESSION);
    if (error != Z_OK)
    {
        throw PDFException(PDFTranslationContext::tr("Failed to initialize flate compression stream."));
    }

    do
    {
        stream.next_out = outputBuffer.data();
        stream.avail_out = static_cast<uInt>(outputBuffer.size());

        error = deflate(&stream, Z_FINISH);

        int bytesWritten = int(outputBuffer.size()) - stream.avail_out;
        result.append(reinterpret_cast<const char*>(outputBuffer.data()), bytesWritten);
    } while (error == Z_OK);

    QString errorMessage;
    if (stream.msg)
    {
        errorMessage = QString::fromLatin1(stream.msg);
    }

    deflateEnd(&stream);

    switch (error)
    {
        case Z_STREAM_END:
            break; // No error, normal behaviour

        default:
        {
            if (errorMessage.isEmpty())
            {
                errorMessage = PDFTranslationContext::tr("zlib code: %1").arg(error);
            }

            throw PDFException(PDFTranslationContext::tr("Error compressing by flate method: %1").arg(errorMessage));
        }
    }

    return result;
}

QByteArray PDFFlateDecodeFilter::recompress(const QByteArray& data)
{
    QByteArray decompressedData = uncompress(data);
    return compress(decompressedData);
}

PDFInteger PDFFlateDecodeFilter::getStreamDataLength(const QByteArray& data, PDFInteger offset) const
{
    if (offset < 0 || offset >= data.size())
    {
        return -1;
    }

    z_stream stream = { };
    stream.next_in = const_cast<Bytef*>(convertByteArrayToUcharPtr(data) + offset);
    stream.avail_in = data.size() - offset;

    std::array<Bytef, 1024> outputBuffer = { };

    int error = inflateInit(&stream);
    if (error != Z_OK)
    {
        return -1;
    }

    do
    {
        stream.next_out = outputBuffer.data();
        stream.avail_out = static_cast<uInt>(outputBuffer.size());

        error = inflate(&stream, Z_NO_FLUSH);
    } while (error == Z_OK);

    PDFInteger dataLength = stream.total_in;
    inflateEnd(&stream);

    if (error == Z_STREAM_END)
    {
        return dataLength;
    }

    return -1;
}

QByteArray PDFFlateDecodeFilter::uncompress(const QByteArray& data)
{
    QByteArray result;

    z_stream stream = { };
    stream.next_in = const_cast<Bytef*>(convertByteArrayToUcharPtr(data));
    stream.avail_in = data.size();

    std::array<Bytef, 1024> outputBuffer = { };

    int error = inflateInit(&stream);
    if (error != Z_OK)
    {
        throw PDFException(PDFTranslationContext::tr("Failed to initialize flate decompression stream."));
    }

    do
    {
        stream.next_out = outputBuffer.data();
        stream.avail_out = static_cast<uInt>(outputBuffer.size());

        error = inflate(&stream, Z_NO_FLUSH);

        int bytesWritten = int(outputBuffer.size()) - stream.avail_out;
        result.append(reinterpret_cast<const char*>(outputBuffer.data()), bytesWritten);
    } while (error == Z_OK);

    QString errorMessage;
    if (stream.msg)
    {
        errorMessage = QString::fromLatin1(stream.msg);
    }

    inflateEnd(&stream);

    switch (error)
    {
        case Z_STREAM_END:
            break; // No error, normal behaviour

        default:
        {
            const bool ignoreError = error == Z_DATA_ERROR && errorMessage == "incorrect data check";

            if (!ignoreError)
            {
                if (errorMessage.isEmpty())
                {
                    errorMessage = PDFTranslationContext::tr("zlib code: %1").arg(error);
                }

                throw PDFException(PDFTranslationContext::tr("Error decompressing by flate method: %1").arg(errorMessage));
            }
        }
    }

    return result;
}

QByteArray PDFRunLengthDecodeFilter::apply(const QByteArray& data,
                                           const PDFObjectFetcher& objectFetcher,
                                           const PDFObject& parameters,
                                           const PDFSecurityHandler* securityHandler) const
{
    Q_UNUSED(objectFetcher);
    Q_UNUSED(parameters);
    Q_UNUSED(securityHandler);

    QByteArray result;
    result.reserve(data.size() * 2);

    auto itEnd = data.cend();
    for (auto it = data.cbegin(); it != itEnd;)
    {
        const unsigned char current = *it++;
        if (current == 128)
        {
            // End of stream marker
            break;
        }
        else if (current < 128)
        {
            // Copy n + 1 characters from the input array literally (and advance iterators)
            const int count = static_cast<int>(current) + 1;
            std::copy(it, std::next(it, count), std::back_inserter(result));
            std::advance(it, count);
        }
        else if (current > 128)
        {
            // Copy 257 - n copies of single character
            const int count = 257 - current;
            const char toBeCopied = *it++;
            std::fill_n(std::back_inserter(result), count, toBeCopied);
        }
    }

    return result;
}

const PDFStreamFilter* PDFStreamFilterStorage::getFilter(const QByteArray& filterName)
{
    const PDFStreamFilterStorage* instance = getInstance();
    auto it = instance->m_filters.find(filterName);
    if (it != instance->m_filters.cend())
    {
        return it->second.get();
    }

    auto itNameDecoded = instance->m_abbreviations.find(filterName);
    if (itNameDecoded != instance->m_abbreviations.cend())
    {
        return getFilter(itNameDecoded->second);
    }

    return nullptr;
}

PDFStreamFilterStorage::StreamFilters PDFStreamFilterStorage::getStreamFilters(const PDFStream* stream, const PDFObjectFetcher& objectFetcher)
{
    StreamFilters result;
    const PDFDictionary* dictionary = stream->getDictionary();

    // Retrieve filters
    PDFObject filters;
    if (dictionary->hasKey(PDF_STREAM_DICT_FILTER))
    {
        filters = objectFetcher(dictionary->get(PDF_STREAM_DICT_FILTER));
    }
    else if (dictionary->hasKey(PDF_STREAM_DICT_FILE_FILTER))
    {
        filters = objectFetcher(dictionary->get(PDF_STREAM_DICT_FILE_FILTER));
    }

    // Retrieve filter parameters
    PDFObject filterParameters;
    if (dictionary->hasKey(PDF_STREAM_DICT_DECODE_PARMS))
    {
        filterParameters = objectFetcher(dictionary->get(PDF_STREAM_DICT_DECODE_PARMS));
    }
    else if (dictionary->hasKey(PDF_STREAM_DICT_FDECODE_PARMS))
    {
        filterParameters = objectFetcher(dictionary->get(PDF_STREAM_DICT_FDECODE_PARMS));
    }

    if (filters.isName())
    {
        result.filterObjects.push_back(PDFStreamFilterStorage::getFilter(filters.getString()));
    }
    else if (filters.isArray())
    {
        const PDFArray* filterArray = filters.getArray();
        const size_t filterCount = filterArray->getCount();
        for (size_t i = 0; i < filterCount; ++i)
        {
            const PDFObject& object = objectFetcher(filterArray->getItem(i));
            if (object.isName())
            {
                result.filterObjects.push_back(PDFStreamFilterStorage::getFilter(object.getString()));
            }
            else
            {
                result.valid = false;
                return result;
            }
        }
    }
    else if (!filters.isNull())
    {
        result.valid = false;
        return result;
    }

    if (filterParameters.isArray())
    {
        const PDFArray* filterParameterArray = filterParameters.getArray();
        const size_t filterParameterCount = filterParameterArray->getCount();
        for (size_t i = 0; i < filterParameterCount; ++i)
        {
            const PDFObject& object = objectFetcher(filterParameterArray->getItem(i));
            result.filterParameterObjects.push_back(object);
        }
    }
    else
    {
        result.filterParameterObjects.push_back(filterParameters);
    }

    result.filterParameterObjects.resize(result.filterObjects.size());
    return result;
}

QByteArray PDFStreamFilterStorage::getDecodedStream(const PDFStream* stream, const PDFObjectFetcher& objectFetcher, const PDFSecurityHandler* securityHandler)
{
    StreamFilters streamFilters = getStreamFilters(stream, objectFetcher);
    QByteArray result = *stream->getContent();

    if (!streamFilters.valid)
    {
        // Stream filters are invalid
        return QByteArray();
    }

    for (size_t i = 0, count = streamFilters.filterObjects.size(); i < count; ++i)
    {
        const PDFStreamFilter* streamFilter = streamFilters.filterObjects[i];
        const PDFObject& streamFilterParameters = streamFilters.filterParameterObjects[i];

        if (streamFilter)
        {
            result = streamFilter->apply(result, objectFetcher, streamFilterParameters, securityHandler);
        }
    }

    return result;
}

QByteArray PDFStreamFilterStorage::getDecodedStream(const PDFStream* stream, const PDFSecurityHandler* securityHandler)
{
    return getDecodedStream(stream, [](const PDFObject& object) -> const PDFObject& { return object; }, securityHandler);
}

PDFInteger PDFStreamFilterStorage::getStreamDataLength(const QByteArray& data, const QByteArray& filterName, PDFInteger offset)
{
    if (const PDFStreamFilter* filter = getFilter(filterName))
    {
        return filter->getStreamDataLength(data, offset);
    }

    return -1;
}

PDFStreamFilterStorage::PDFStreamFilterStorage()
{
    // Initialize map with the filters
    m_filters["ASCIIHexDecode"] = std::make_unique<PDFAsciiHexDecodeFilter>();
    m_filters["ASCII85Decode"] = std::make_unique<PDFAscii85DecodeFilter>();
    m_filters["LZWDecode"] = std::make_unique<PDFLzwDecodeFilter>();
    m_filters["FlateDecode"] = std::make_unique<PDFFlateDecodeFilter>();
    m_filters["RunLengthDecode"] = std::make_unique<PDFRunLengthDecodeFilter>();
    m_filters["Crypt"] = std::make_unique<PDFCryptFilter>();

    m_abbreviations["AHx"] = "ASCIIHexDecode";
    m_abbreviations["A85"] = "ASCII85Decode";
    m_abbreviations["LZW"] = "LZWDecode";
    m_abbreviations["Fl"] = "FlateDecode";
    m_abbreviations["RL"] = "RunLengthDecode";
    m_abbreviations["CCF"] = "CCITFaxDecode";
    m_abbreviations["DCT"] = "DCTDecode";
}

const PDFStreamFilterStorage* PDFStreamFilterStorage::getInstance()
{
    static PDFStreamFilterStorage instance;
    return &instance;
}

PDFStreamPredictor PDFStreamPredictor::createPredictor(const PDFObjectFetcher& objectFetcher, const PDFObject& parameters)
{
    const PDFObject& dereferencedParameters = objectFetcher(parameters);
    if (dereferencedParameters.isDictionary())
    {
        const PDFDictionary* dictionary = dereferencedParameters.getDictionary();

        auto getInteger = [dictionary, &objectFetcher](const char* key, int min, int max, int defaultValue) -> int
        {
            const PDFObject& object = objectFetcher(dictionary->get(key));

            if (object.isInt())
            {
                PDFInteger value = object.getInteger();
                if (value < min || value > max)
                {
                    throw PDFException(PDFTranslationContext::tr("Property '%1' should be in range from %2 to %3.").arg(QString::fromLatin1(key)).arg(min).arg(max));
                }

                return value;
            }
            else if (object.isNull())
            {
                return defaultValue;
            }

            throw PDFException(PDFTranslationContext::tr("Invalid property '%1' of the stream predictor parameters.").arg(QString::fromLatin1(key)));
        };

        int predictor = getInteger("Predictor", 1, 15, 1);
        int components = getInteger("Colors", 1, PDF_MAX_COLOR_COMPONENTS, 1);
        int bitsPerComponent = getInteger("BitsPerComponent", 1, 16, 8);
        int columns = getInteger("Columns", 1, std::numeric_limits<int>::max(), 1);

        return PDFStreamPredictor(static_cast<Predictor>(predictor), components, bitsPerComponent, columns);
    }

    return PDFStreamPredictor();
}

QByteArray PDFStreamPredictor::apply(const QByteArray& data) const
{
    switch (m_predictor)
    {
        case NoPredictor:
            return data;

        case TIFF:
            return applyTIFFPredictor(data);

        default:
        {
            if (m_predictor >= 10)
            {
                return applyPNGPredictor(data);
            }
            break;
        }
    }

    throw PDFException(PDFTranslationContext::tr("Invalid predictor algorithm."));
}

QByteArray PDFStreamPredictor::applyPNGPredictor(const QByteArray& data) const
{
    QByteArray outputData;
    outputData.reserve(data.size());

    auto it = data.cbegin();
    auto itEnd = data.cend();

    int pixelBytes = (m_components * m_bitsPerComponent + 7) / 8;

    auto readByte = [&it, &itEnd]() -> uint8_t
    {
        if (it != itEnd)
        {
            return static_cast<uint8_t>(*it++);
        }

        // According to the PDF specification, incomplete line is completed. For this
        // reason, we behave as we have zero data in the buffer.
        return 0;
    };

    // Idea: to avoid using if for many cases, we use larger buffer filled with zeros
    const int totalBytes = m_stride + pixelBytes;
    std::vector<uint8_t> line(totalBytes, 0);
    std::vector<uint8_t> lineOld(totalBytes, 0);

    Predictor currentPredictor = m_predictor;
    while (it != itEnd)
    {
        // First, read the predictor data for current line
        currentPredictor = static_cast<Predictor>(readByte() + 10);

        for (int i = 0; i < m_stride; ++i)
        {
            uint8_t currentByte = readByte();

            int lineIndex = i + pixelBytes;
            switch (currentPredictor)
            {
                case PNG_Sub:
                {
                    line[lineIndex] = line[i] + currentByte;
                    break;
                }

                case PNG_Up:
                {
                    line[lineIndex] = lineOld[lineIndex] + currentByte;
                    break;
                }

                case PNG_Average:
                {
                    line[lineIndex] = (lineOld[lineIndex] + line[i]) / 2 + currentByte;
                    break;
                }

                case PNG_Paeth:
                {
                    // a = left,
                    // b = upper,
                    // c = upper left
                    const int a = line[i];
                    const int b = lineOld[lineIndex];
                    const int c = lineOld[i];
                    const int p = a + b - c;
                    const int pa = std::abs(p - a);
                    const int pb = std::abs(p - b);
                    const int pc = std::abs(p - c);
                    if (pa <= pb && pa <= pc)
                    {
                        line[lineIndex] = a + currentByte;
                    }
                    else if (pb <= pc)
                    {
                        line[lineIndex] = b + currentByte;
                    }
                    else
                    {
                        line[lineIndex] = c + currentByte;
                    }
                    break;
                }

                case PNG_None:
                default:
                {
                    line[lineIndex] = currentByte;
                    break;
                }
            }

            // Fill the output buffer
            outputData.push_back(static_cast<char>(line[lineIndex]));
        }

        // Swap the buffers
        std::swap(line, lineOld);
    }

    return outputData;
}

QByteArray PDFStreamPredictor::applyTIFFPredictor(const QByteArray& data) const
{
    Q_UNUSED(data);

    PDFBitWriter writer(m_bitsPerComponent);
    PDFBitReader reader(&data, m_bitsPerComponent);

    writer.reserve(data.size());
    std::vector<uint32_t> leftValues(m_components, 0);

    while (!reader.isAtEnd())
    {
        for (int i = 0; i < m_columns; ++i)
        {
            for (int componentIndex = 0; componentIndex < m_components; ++componentIndex)
            {
                leftValues[componentIndex] = (leftValues[componentIndex] + reader.read()) & reader.max();
                writer.write(leftValues[componentIndex]);
            }
        }

        std::fill(leftValues.begin(), leftValues.end(), 0);
        reader.alignToBytes();
        writer.finishLine();
    }

    return writer.takeByteArray();
}

QByteArray PDFCryptFilter::apply(const QByteArray& data,
                                 const PDFObjectFetcher& objectFetcher,
                                 const PDFObject& parameters,
                                 const PDFSecurityHandler* securityHandler) const
{
    if (!securityHandler)
    {
        throw PDFException(PDFTranslationContext::tr("Security handler required, but not provided."));
    }

    PDFObjectReference objectReference;
    QByteArray cryptFilterName = PDFSecurityHandler::IDENTITY_FILTER_NAME;
    const PDFObject& dereferencedParameters = objectFetcher(parameters);
    if (dereferencedParameters.isDictionary())
    {
        const PDFDictionary* dictionary = dereferencedParameters.getDictionary();
        const PDFObject& cryptFilterNameObject = objectFetcher(dictionary->get("Name"));
        if (cryptFilterNameObject.isName())
        {
            cryptFilterName = cryptFilterNameObject.getString();
        }

        const PDFObject& objectReferenceObject = dictionary->get(PDFSecurityHandler::OBJECT_REFERENCE_DICTIONARY_NAME);
        if (objectReferenceObject.isReference())
        {
            objectReference = objectReferenceObject.getReference();
        }
    }

    return securityHandler->decryptByFilter(data, cryptFilterName, objectReference);
}

PDFInteger PDFStreamFilter::getStreamDataLength(const QByteArray& data, PDFInteger offset) const
{
    Q_UNUSED(data);
    Q_UNUSED(offset);

    return -1;
}

}   // namespace pdf
