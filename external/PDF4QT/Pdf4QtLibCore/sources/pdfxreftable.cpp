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

#include "pdfxreftable.h"
#include "pdfconstants.h"
#include "pdfexception.h"
#include "pdfparser.h"
#include "pdfstreamfilters.h"

#include <QIODevice>
#include <QDataStream>

#include "pdfdbgheap.h"

#include <stack>

namespace pdf
{

void PDFXRefTable::readXRefTable(PDFParsingContext* context, const QByteArray& byteArray, PDFInteger startTableOffset)
{
    PDFParser parser(byteArray, context, PDFParser::AllowStreams);

    m_entries.clear();

    std::set<PDFInteger> processedOffsets;
    std::stack<PDFInteger> workSet;
    workSet.push(startTableOffset);

    while (!workSet.empty())
    {
        PDFInteger currentOffset = workSet.top();
        workSet.pop();

        // Check, if we have cyclical references between tables
        if (processedOffsets.count(currentOffset))
        {
            // If cyclical reference occurs, do not report error, just ignore it.
            continue;
        }
        else
        {
            processedOffsets.insert(currentOffset);
        }

        // Now, we are ready to scan the table. Seek to the start of the reference table.
        parser.seek(currentOffset);

        if (parser.fetchCommand(PDF_XREF_HEADER))
        {
            while (!parser.fetchCommand(PDF_XREF_TRAILER))
            {
                // Now, first number is start offset, second number is count of table items
                PDFObject firstObject = parser.getObject();
                PDFObject countObject = parser.getObject();

                if (!firstObject.isInt() || !countObject.isInt())
                {
                    throw PDFException(tr("Invalid format of reference table."));
                }

                PDFInteger firstObjectNumber = firstObject.getInteger();
                PDFInteger count = countObject.getInteger();

                const PDFInteger lastObjectIndex = firstObjectNumber + count - 1;
                const PDFInteger desiredSize = lastObjectIndex + 1;

                if (static_cast<PDFInteger>(m_entries.size()) < desiredSize)
                {
                    m_entries.resize(desiredSize);
                }

                // Now, read the records
                for (PDFInteger i = 0; i < count; ++i)
                {
                    const PDFInteger objectNumber = firstObjectNumber + i;

                    PDFObject offset = parser.getObject();
                    PDFObject generation = parser.getObject();

                    bool occupied = parser.fetchCommand(PDF_XREF_OCCUPIED);
                    if (!occupied && !parser.fetchCommand(PDF_XREF_FREE))
                    {
                        throw PDFException(tr("Bad format of reference table entry."));
                    }

                    if (!offset.isInt() || !generation.isInt())
                    {
                        throw PDFException(tr("Bad format of reference table entry."));
                    }

                    if (static_cast<size_t>(objectNumber) >= m_entries.size())
                    {
                        throw PDFException(tr("Bad format of reference table entry."));
                    }

                    Entry entry;
                    if (occupied)
                    {
                        entry.reference = PDFObjectReference(objectNumber, generation.getInteger());
                        entry.offset = offset.getInteger();
                        entry.type = EntryType::Occupied;
                    }

                    if (m_entries[objectNumber].type == EntryType::Free)
                    {
                        m_entries[objectNumber] = std::move(entry);
                    }
                }
            }

            PDFObject trailerDictionary = parser.getObject();
            if (!trailerDictionary.isDictionary())
            {
                throw PDFException(tr("Trailer dictionary is invalid."));
            }

            // Now, we have scanned the table. If we didn't have a trailer dictionary yet, then
            // try to load it. We must also check, that trailer dictionary is OK.
            if (m_trailerDictionary.isNull())
            {
                m_trailerDictionary = trailerDictionary;
            }

            const PDFDictionary* dictionary = trailerDictionary.getDictionary();
            if (dictionary->hasKey(PDF_XREF_TRAILER_PREVIOUS))
            {
                PDFObject previousOffset = dictionary->get(PDF_XREF_TRAILER_PREVIOUS);

                if (!previousOffset.isInt())
                {
                    throw PDFException(tr("Offset of previous reference table is invalid."));
                }

                workSet.push(previousOffset.getInteger());
            }

            const PDFObject& xrefstmObject = dictionary->get(PDF_XREF_TRAILER_XREFSTM);
            if (xrefstmObject.isInt())
            {
                workSet.push(xrefstmObject.getInteger());
            }
        }
        else
        {
            // Try to read cross-reference stream
            PDFObject crossReferenceStreamObjectNumber = parser.getObject();
            PDFObject crossReferenceStreamGeneration = parser.getObject();

            if (!crossReferenceStreamObjectNumber.isInt() || !crossReferenceStreamGeneration.isInt())
            {
                throw PDFException(tr("Invalid format of reference table."));
            }

            if (!parser.fetchCommand(PDF_OBJECT_START_MARK))
            {
                throw PDFException(tr("Invalid format of reference table."));
            }

            PDFObject crossReferenceObject = parser.getObject();

            if (!parser.fetchCommand(PDF_OBJECT_END_MARK))
            {
                throw PDFException(tr("Invalid format of reference table."));
            }

            if (crossReferenceObject.isStream())
            {
                const PDFStream* crossReferenceStream = crossReferenceObject.getStream();
                const PDFDictionary* crossReferenceStreamDictionary = crossReferenceStream->getDictionary();
                const PDFObject typeObject = crossReferenceStreamDictionary->get("Type");
                if (typeObject.isName() && typeObject.getString() == "XRef")
                {
                    PDFObject sizeObject = crossReferenceStreamDictionary->get("Size");
                    if (!sizeObject.isInt() || sizeObject.getInteger() < 0)
                    {
                        throw PDFException(tr("Invalid format of cross-reference stream."));
                    }

                    const PDFInteger desiredSize = sizeObject.getInteger();
                    if (static_cast<PDFInteger>(m_entries.size()) < desiredSize)
                    {
                        m_entries.resize(desiredSize);
                    }

                    PDFObject prevObject = crossReferenceStreamDictionary->get("Prev");
                    if (prevObject.isInt())
                    {
                        workSet.push(prevObject.getInteger());
                    }

                    // Do not overwrite trailer dictionary, if it was already loaded.
                    if (m_trailerDictionary.isNull())
                    {
                        m_trailerDictionary = crossReferenceObject;
                    }

                    auto readIntegerArray = [crossReferenceStreamDictionary](const char* key, auto defaultValues) -> std::vector<PDFInteger>
                    {
                        std::vector<PDFInteger> result;

                        const PDFObject& object = crossReferenceStreamDictionary->get(key);
                        if (object.isArray())
                        {
                            const PDFArray* array = object.getArray();
                            result.reserve(array->getCount());

                            for (size_t i = 0, count = array->getCount(); i < count; ++i)
                            {
                                const PDFObject& itemObject = array->getItem(i);
                                if (itemObject.isInt())
                                {
                                    result.push_back(itemObject.getInteger());
                                }
                                else
                                {
                                    throw PDFException(tr("Invalid format of cross-reference stream."));
                                }
                            }
                        }
                        else
                        {
                            result = defaultValues;
                        }

                        return result;
                    };

                    std::vector<PDFInteger> indexArray = readIntegerArray("Index", std::initializer_list<PDFInteger>{ PDFInteger(0), PDFInteger(desiredSize) });
                    std::vector<PDFInteger> wArray = readIntegerArray("W", std::vector<PDFInteger>());

                    if (wArray.size() != 3 || indexArray.empty() || (indexArray.size() % 2 != 0))
                    {
                        throw PDFException(tr("Invalid format of cross-reference stream."));
                    }

                    const int columnTypeBytes = wArray[0];
                    const int columnObjectNumberOrByteOffsetBytes = wArray[1];
                    const int columnGenerationNumberOrObjectIndexBytes = wArray[2];
                    const size_t blockCount = indexArray.size() / 2;

                    QByteArray data = PDFStreamFilterStorage::getDecodedStream(crossReferenceStream, nullptr);
                    QDataStream dataStream(&data, QIODevice::ReadOnly);
                    dataStream.setByteOrder(QDataStream::BigEndian);

                    auto readNumber = [&dataStream](int bytes, PDFInteger defaultValue) -> PDFInteger
                    {
                        if (bytes)
                        {
                            uint64_t value = 0;

                            while (bytes--)
                            {
                                uint8_t byte = 0;
                                dataStream >> byte;
                                value = (value << 8) + byte;

                                // Check, if stream is OK (we doesn't read past the end of the stream,
                                // data aren't corrupted etc.)
                                if (dataStream.status() != QDataStream::Ok)
                                {
                                    throw PDFException(tr("Invalid format of cross-reference stream - not enough data in the stream."));
                                }
                            }

                            return static_cast<PDFInteger>(value);
                        }
                        return defaultValue;
                    };

                    for (size_t i = 0; i < blockCount; ++i)
                    {
                        PDFInteger firstObjectNumber = indexArray[2 * i];
                        PDFInteger count = indexArray[2 * i + 1];

                        const PDFInteger lastObjectIndex = firstObjectNumber + count - 1;
                        const PDFInteger currentDesiredSize = lastObjectIndex + 1;

                        if (static_cast<PDFInteger>(m_entries.size()) < currentDesiredSize)
                        {
                            m_entries.resize(currentDesiredSize);
                        }

                        for (PDFInteger objectNumber = firstObjectNumber; objectNumber <= lastObjectIndex; ++ objectNumber)
                        {
                            int itemType = readNumber(columnTypeBytes, 1);
                            int itemObjectNumberOfObjectStreamOrByteOffset = readNumber(columnObjectNumberOrByteOffsetBytes, 0);
                            int itemGenerationNumberOrObjectIndex = readNumber(columnGenerationNumberOrObjectIndexBytes, 0);

                            switch (itemType)
                            {
                                case 0:
                                    // Free object
                                    break;

                                case 1:
                                {
                                    Entry entry;
                                    entry.reference = PDFObjectReference(objectNumber, itemGenerationNumberOrObjectIndex);
                                    entry.offset = itemObjectNumberOfObjectStreamOrByteOffset;
                                    entry.type = EntryType::Occupied;

                                    if (m_entries[objectNumber].type == EntryType::Free)
                                    {
                                        m_entries[objectNumber] = std::move(entry);
                                    }
                                    break;
                                }

                                case 2:
                                {
                                    Entry entry;
                                    entry.reference = PDFObjectReference(objectNumber, 0);
                                    entry.objectStream = PDFObjectReference(itemObjectNumberOfObjectStreamOrByteOffset, 0);
                                    entry.indexInObjectStream = itemGenerationNumberOrObjectIndex;
                                    entry.type = EntryType::InObjectStream;

                                    if (m_entries[objectNumber].type == EntryType::Free)
                                    {
                                        m_entries[objectNumber] = std::move(entry);
                                    }

                                    break;
                                }

                                default:
                                    // According to the specification, treat this object as null object
                                    break;
                            }
                        }
                    }
                }

                continue;
            }

            throw PDFException(tr("Invalid format of reference table."));
        }
    }
}

std::vector<PDFXRefTable::Entry> PDFXRefTable::getOccupiedEntries() const
{
    std::vector<PDFXRefTable::Entry> result;

    // Suppose majority of items are occupied
    result.reserve(m_entries.size());
    std::copy_if(m_entries.cbegin(), m_entries.cend(), std::back_inserter(result), [](const Entry& entry) { return entry.type == EntryType::Occupied; });

    return result;
}

std::vector<PDFXRefTable::Entry> PDFXRefTable::getObjectStreamEntries() const
{
    std::vector<PDFXRefTable::Entry> result;

    // Suppose majority of items are occupied
    result.reserve(m_entries.size());
    std::copy_if(m_entries.cbegin(), m_entries.cend(), std::back_inserter(result), [](const Entry& entry) { return entry.type == EntryType::InObjectStream; });

    return result;
}

const PDFXRefTable::Entry& PDFXRefTable::getEntry(PDFObjectReference reference) const
{
    // We must also check generation number here. For this reason, we compare references of the entry at given position.
    if (reference.objectNumber >= 0 && reference.objectNumber < static_cast<PDFInteger>(m_entries.size()) && m_entries[reference.objectNumber].reference == reference)
    {
        return m_entries[reference.objectNumber];
    }
    else
    {
        static Entry dummy;
        return dummy;
    }
}

}   // namespace pdf
