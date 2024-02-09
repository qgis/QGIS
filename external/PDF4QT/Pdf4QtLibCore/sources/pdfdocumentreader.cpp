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


#include "pdfdocumentreader.h"
#include "pdfconstants.h"
#include "pdfxreftable.h"
#include "pdfexception.h"
#include "pdfparser.h"
#include "pdfstreamfilters.h"
#include "pdfexecutionpolicy.h"

#include <QFile>
#include <QCryptographicHash>

#include "pdfdbgheap.h"

#include <regex>
#include <cctype>
#include <algorithm>
#include <execution>

namespace pdf
{

PDFDocumentReader::PDFDocumentReader(PDFProgress* progress, const std::function<QString(bool*)>& getPasswordCallback, bool permissive, bool authorizeOwnerOnly) :
    m_result(Result::OK),
    m_getPasswordCallback(getPasswordCallback),
    m_progress(progress),
    m_permissive(permissive),
    m_authorizeOwnerOnly(authorizeOwnerOnly)
{

}

PDFDocument PDFDocumentReader::readFromFile(const QString& fileName)
{
    QFile file(fileName);

    reset();

    if (file.exists())
    {
        if (file.open(QFile::ReadOnly))
        {
            PDFDocument document = readFromDevice(&file);
            file.close();
            return document;
        }
        else
        {
            m_result = Result::Failed;
            m_errorMessage = tr("File '%1' cannot be opened for reading. %1").arg(file.errorString());
        }
    }
    else
    {
        m_result = Result::Failed;
        m_errorMessage = tr("File '%1' doesn't exist.").arg(fileName);
    }

    return PDFDocument();
}

PDFDocument PDFDocumentReader::readFromDevice(QIODevice* device)
{
    reset();

    if (device->isOpen())
    {
        if (device->isReadable())
        {
            // Do not close the device, it was not opened by us.
            return readFromBuffer(device->readAll());
        }
        else
        {
            m_result = Result::Failed;
            m_errorMessage = tr("Device is not opened for reading.");
        }
    }
    else if (device->open(QIODevice::ReadOnly))
    {
        QByteArray byteArray = device->readAll();
        device->close();
        return readFromBuffer(byteArray);
    }
    else
    {
        m_result = Result::Failed;
        m_errorMessage = tr("Can't open device for reading.");
    }

    return PDFDocument();
}

void PDFDocumentReader::checkFooter(const QByteArray& buffer)
{
    if (findFromEnd(PDF_END_OF_FILE_MARK, buffer, PDF_FOOTER_SCAN_LIMIT) == FIND_NOT_FOUND_RESULT)
    {
        QString message = tr("End of file marking was not found.");
        if (m_permissive)
        {
            QMutexLocker lock(&m_mutex);
            m_warnings << message;
        }
        else
        {
            throw PDFException(message);
        }
    }
}

void PDFDocumentReader::checkHeader(const QByteArray& buffer)
{
    // According to PDF Reference 1.7, Appendix H, file header can have two formats:
    //  - %PDF-x.x
    //  - %!PS-Adobe-y.y PDF-x.x
    // We will search for both of these formats.
    std::regex headerRegExp(PDF_FILE_HEADER_REGEXP);
    std::cmatch headerMatch;

    auto itBegin = buffer.cbegin();
    auto itEnd = std::next(buffer.cbegin(), qMin(buffer.size(), PDF_HEADER_SCAN_LIMIT));

    if (std::regex_search(itBegin, itEnd, headerMatch, headerRegExp))
    {
        // Size depends on regular expression, not on the text (if regular expresion is matched)
        Q_ASSERT(headerMatch.size() == 3);
        Q_ASSERT(headerMatch[1].matched != headerMatch[2].matched);

        for (int i : { 1, 2 })
        {
            if (headerMatch[i].matched)
            {
                Q_ASSERT(std::distance(headerMatch[i].first, headerMatch[i].second) == 3);
                m_version = PDFVersion(*headerMatch[i].first - '0', *std::prev(headerMatch[i].second) - '0');
                break;
            }
        }
    }
    else
    {
        throw PDFException(tr("Header of PDF file was not found."));
    }

    // Check, if version is valid
    if (!m_version.isValid())
    {
        throw PDFException(tr("Version of the PDF file is not valid."));
    }
}

PDFInteger PDFDocumentReader::findXrefTableOffset(const QByteArray& buffer)
{
    const int startXRefPosition = findFromEnd(PDF_START_OF_XREF_MARK, buffer, PDF_FOOTER_SCAN_LIMIT);
    if (startXRefPosition == FIND_NOT_FOUND_RESULT)
    {
        throw PDFException(tr("Start of object reference table not found."));
    }

    Q_ASSERT(startXRefPosition + std::strlen(PDF_START_OF_XREF_MARK) < static_cast<size_t>(buffer.size()));
    PDFLexicalAnalyzer analyzer(buffer.constData() + startXRefPosition + std::strlen(PDF_START_OF_XREF_MARK), buffer.constData() + buffer.size());
    const PDFLexicalAnalyzer::Token token = analyzer.fetch();
    if (token.type != PDFLexicalAnalyzer::TokenType::Integer)
    {
        throw PDFException(tr("Start of object reference table not found."));
    }

    const PDFInteger firstXrefTableOffset = token.data.toLongLong();
    return firstXrefTableOffset;
}

PDFObject PDFDocumentReader::getObject(PDFParsingContext* context, PDFInteger offset, PDFObjectReference reference) const
{
    PDFParsingContext::PDFParsingContextGuard guard(context, reference);

    PDFParser parser(m_source, context, PDFParser::AllowStreams);
    parser.seek(offset);

    PDFObject objectNumber = parser.getObject();
    PDFObject generation = parser.getObject();

    if (!objectNumber.isInt() || !generation.isInt())
    {
        throw PDFException(tr("Can't read object at position %1.").arg(offset));
    }

    if (!parser.fetchCommand(PDF_OBJECT_START_MARK))
    {
        throw PDFException(tr("Can't read object at position %1.").arg(offset));
    }

    PDFObject object = parser.getObject();

    if (!parser.fetchCommand(PDF_OBJECT_END_MARK))
    {
        throw PDFException(tr("Can't read object at position %1.").arg(offset));
    }

    PDFObjectReference scannedReference(objectNumber.getInteger(), generation.getInteger());
    if (scannedReference != reference)
    {
        throw PDFException(tr("Can't read object at position %1.").arg(offset));
    }

    return object;
}

PDFObject PDFDocumentReader::getObjectFromXrefTable(PDFXRefTable* xrefTable, PDFParsingContext* context, PDFObjectReference reference) const
{
    const PDFXRefTable::Entry& entry = xrefTable->getEntry(reference);
    switch (entry.type)
    {
        case PDFXRefTable::EntryType::Free:
            return PDFObject();

        case PDFXRefTable::EntryType::Occupied:
        {
            Q_ASSERT(entry.reference == reference);
            return getObject(context, entry.offset, reference);
        }

        default:
        {
            Q_ASSERT(false);
            break;
        }
    }

    return PDFObject();
}

PDFObject PDFDocumentReader::readDamagedTrailerDictionary() const
{
    PDFObject object = PDFObject::createDictionary(std::make_shared<PDFDictionary>(PDFDictionary()));
    PDFParsingContext context([](PDFParsingContext*, PDFObjectReference){ return PDFObject(); });

    int offset = 0;
    while (offset < m_source.size())
    {
        offset = m_source.indexOf(PDF_XREF_TRAILER, offset);

        if (offset == -1)
        {
            break;
        }

        offset += static_cast<int>(std::strlen(PDF_XREF_TRAILER));

        // Try to read trailer dictioanry
        try
        {
            PDFParser parser(m_source, &context, PDFParser::None);
            parser.seek(offset);

            PDFObject trailerDictionaryObject = parser.getObject();
            if (trailerDictionaryObject.isDictionary())
            {
                object = PDFObjectManipulator::merge(object, trailerDictionaryObject, PDFObjectManipulator::RemoveNullObjects);
            }
        }
        catch (const PDFException&)
        {
            // Do nothing...
        }
    }

    return object;
}

PDFDocumentReader::Result PDFDocumentReader::processReferenceTableEntries(PDFXRefTable* xrefTable, const std::vector<PDFXRefTable::Entry>& occupiedEntries, PDFObjectStorage::PDFObjects& objects)
{
    auto objectFetcher = [this, xrefTable](PDFParsingContext* context, PDFObjectReference reference) { return getObjectFromXrefTable(xrefTable, context, reference); };
    auto processEntry = [this, &objectFetcher, &objects](const PDFXRefTable::Entry& entry)
    {
        Q_ASSERT(entry.type == PDFXRefTable::EntryType::Occupied);

        if (m_result == Result::OK)
        {
            try
            {
                PDFParsingContext context(objectFetcher);
                PDFObject object = getObject(&context, entry.offset, entry.reference);

                progressStep();

                QMutexLocker lock(&m_mutex);
                objects[entry.reference.objectNumber] = PDFObjectStorage::Entry(entry.reference.generation, object);
            }
            catch (const PDFException& exception)
            {
                QMutexLocker lock(&m_mutex);

                if (m_permissive)
                {
                    m_warnings << exception.getMessage();
                }
                else
                {
                    m_result = Result::Failed;
                    m_errorMessage = exception.getMessage();
                }
            }
        }
    };

    // Now, we are ready to scan all objects
    if (!occupiedEntries.empty())
    {
        progressStart(occupiedEntries.size(), PDFTranslationContext::tr("Reading contents of document..."));
        PDFExecutionPolicy::execute(PDFExecutionPolicy::Scope::Unknown, occupiedEntries.cbegin(), occupiedEntries.cend(), processEntry);
        progressFinish();
    }

    return m_result;
}

PDFDocumentReader::Result PDFDocumentReader::processSecurityHandler(const PDFObject& trailerDictionaryObject,
                                                                    const std::vector<PDFXRefTable::Entry>& occupiedEntries,
                                                                    PDFObjectStorage::PDFObjects& objects)
{
    const PDFDictionary* trailerDictionary = nullptr;
    if (trailerDictionaryObject.isDictionary())
    {
        trailerDictionary = trailerDictionaryObject.getDictionary();
    }
    else if (trailerDictionaryObject.isStream())
    {
        const PDFStream* stream = trailerDictionaryObject.getStream();
        trailerDictionary = stream->getDictionary();
    }
    else
    {
        throw PDFException(tr("Invalid trailer dictionary."));
    }

    // Read the document ID
    QByteArray id;
    const PDFObject& idArrayObject = trailerDictionary->get("ID");
    if (idArrayObject.isArray())
    {
        const PDFArray* idArray = idArrayObject.getArray();
        if (idArray->getCount() > 0)
        {
            const PDFObject& idArrayItem = idArray->getItem(0);
            if (idArrayItem.isString())
            {
                id = idArrayItem.getString();
            }
        }
    }

    PDFObjectReference encryptObjectReference;
    PDFObject encryptObject = trailerDictionary->get("Encrypt");
    if (encryptObject.isReference())
    {
        encryptObjectReference = encryptObject.getReference();
        if (static_cast<size_t>(encryptObjectReference.objectNumber) < objects.size() && objects[encryptObjectReference.objectNumber].generation == encryptObjectReference.generation)
        {
            encryptObject = objects[encryptObjectReference.objectNumber].object;
        }
    }

    // Read the security handler
    m_securityHandler = PDFSecurityHandler::createSecurityHandler(encryptObject, id);
    PDFSecurityHandler::AuthorizationResult authorizationResult = m_securityHandler->authenticate(m_getPasswordCallback, m_authorizeOwnerOnly);

    if (authorizationResult == PDFSecurityHandler::AuthorizationResult::Cancelled)
    {
        // User cancelled the document reading
        m_result = Result::Cancelled;
        return m_result;
    }

    if (authorizationResult == PDFSecurityHandler::AuthorizationResult::Failed)
    {
        throw PDFException(PDFTranslationContext::tr("Authorization failed. Bad password provided."));
    }

    // Now, decrypt the document, if we are authorized. We must also check, if we have to decrypt the object.
    // According to the PDF specification, following items are ommited from encryption:
    //      1) Values for ID entry in the trailer dictionary
    //      2) Any strings in Encrypt dictionary
    //      3) String/streams in object streams (entire object streams are encrypted)
    //      4) Hexadecimal strings in Content key in signature dictionary
    //
    // Trailer dictionary is not decrypted, because PDF specification provides no algorithm to decrypt it,
    // because it needs object number and generation for generating the decrypt key. So 1) is handled
    // automatically. 2) is handled in the code below. 3) is handled also automatically, because we do not
    // decipher object streams here. 4) must be handled in the security handler.
    if (m_securityHandler->getMode() != EncryptionMode::None)
    {
        auto decryptEntry = [this, encryptObjectReference, &objects](const PDFXRefTable::Entry& entry)
        {
            progressStep();

            if (encryptObjectReference.objectNumber != 0 && encryptObjectReference == entry.reference)
            {
                // 2) - Encrypt dictionary
                return;
            }

            objects[entry.reference.objectNumber].object = m_securityHandler->decryptObject(objects[entry.reference.objectNumber].object, entry.reference);
        };

        progressStart(occupiedEntries.size(), PDFTranslationContext::tr("Decrypting encrypted contents of document..."));
        PDFExecutionPolicy::execute(PDFExecutionPolicy::Scope::Unknown, occupiedEntries.cbegin(), occupiedEntries.cend(), decryptEntry);
        progressFinish();
    }

    return m_result;
}

void PDFDocumentReader::processObjectStreams(PDFXRefTable* xrefTable, PDFObjectStorage::PDFObjects& objects)
{
    // Then process object streams
    std::vector<PDFXRefTable::Entry> objectStreamEntries = xrefTable->getObjectStreamEntries();
    std::set<PDFObjectReference> objectStreams;
    for (const PDFXRefTable::Entry& entry : objectStreamEntries)
    {
        Q_ASSERT(entry.type == PDFXRefTable::EntryType::InObjectStream);
        objectStreams.insert(entry.objectStream);
    }

    auto objectFetcher = [this, xrefTable](PDFParsingContext* context, PDFObjectReference reference) { return getObjectFromXrefTable(xrefTable, context, reference); };
    auto processObjectStream = [this, &objectFetcher, &objects, &objectStreamEntries] (const PDFObjectReference& objectStreamReference)
    {
        if (m_result != Result::OK)
        {
            return;
        }

        try
        {
            PDFParsingContext context(objectFetcher);
            if (objectStreamReference.objectNumber >= static_cast<PDFInteger>(objects.size()))
            {
                throw PDFException(PDFTranslationContext::tr("Object stream %1 not found.").arg(objectStreamReference.objectNumber));
            }

            const PDFObject& object = objects[objectStreamReference.objectNumber].object;
            if (!object.isStream())
            {
                throw PDFException(PDFTranslationContext::tr("Object stream %1 is invalid.").arg(objectStreamReference.objectNumber));
            }

            const PDFStream* objectStream = object.getStream();
            const PDFDictionary* objectStreamDictionary = objectStream->getDictionary();

            const PDFObject& objectStreamType = objectStreamDictionary->get("Type");
            if (!objectStreamType.isName() || objectStreamType.getString() != "ObjStm")
            {
                throw PDFException(PDFTranslationContext::tr("Object stream %1 is invalid.").arg(objectStreamReference.objectNumber));
            }

            const PDFObject& nObject = objectStreamDictionary->get("N");
            const PDFObject& firstObject = objectStreamDictionary->get("First");
            if (!nObject.isInt() || !firstObject.isInt())
            {
                throw PDFException(PDFTranslationContext::tr("Object stream %1 is invalid.").arg(objectStreamReference.objectNumber));
            }

            // Number of objects in object stream dictionary
            const PDFInteger n = nObject.getInteger();
            const PDFInteger first = firstObject.getInteger();

            QByteArray objectStreamData = PDFStreamFilterStorage::getDecodedStream(objectStream, m_securityHandler.data());

            PDFParsingContext::PDFParsingContextGuard guard(&context, objectStreamReference);
            PDFParser parser(objectStreamData, &context, PDFParser::AllowStreams);

            std::vector<std::pair<PDFInteger, PDFInteger>> objectNumberAndOffset;
            objectNumberAndOffset.reserve(n);
            for (PDFInteger i = 0; i < n; ++i)
            {
                PDFObject currentObjectNumber = parser.getObject();
                PDFObject currentOffset = parser.getObject();

                if (!currentObjectNumber.isInt() || !currentOffset.isInt())
                {
                    throw PDFException(PDFTranslationContext::tr("Object stream %1 is invalid.").arg(objectStreamReference.objectNumber));
                }

                const PDFInteger objectNumber = currentObjectNumber.getInteger();
                const PDFInteger offset = currentOffset.getInteger() + first;
                objectNumberAndOffset.emplace_back(objectNumber, offset);
            }

            for (size_t i = 0; i < objectNumberAndOffset.size(); ++i)
            {
                const PDFInteger objectNumber = objectNumberAndOffset[i].first;
                const PDFInteger offset = objectNumberAndOffset[i].second;
                parser.seek(offset);

                PDFObject currentObject = parser.getObject();
                auto predicate = [objectNumber, objectStreamReference](const PDFXRefTable::Entry& entry) -> bool { return entry.reference.objectNumber == objectNumber && entry.objectStream == objectStreamReference; };
                if (std::find_if(objectStreamEntries.cbegin(), objectStreamEntries.cend(), predicate) != objectStreamEntries.cend())
                {
                    QMutexLocker lock(&m_mutex);
                    objects[objectNumber].object = qMove(currentObject);
                }
                else
                {
                    // Silently ignore this error. It is not critical, so, maybe this object will be null.
                }
            }
        }
        catch (const PDFException& exception)
        {
            QMutexLocker lock(&m_mutex);
            m_result = Result::Failed;
            m_errorMessage = exception.getMessage();
        }
    };

    // Now, we are ready to scan all object streams
    PDFExecutionPolicy::execute(PDFExecutionPolicy::Scope::Unknown, objectStreams.cbegin(), objectStreams.cend(), processObjectStream);
}

PDFDocument PDFDocumentReader::readFromBuffer(const QByteArray& buffer)
{
    bool shouldTryPermissiveReading = true;

    try
    {
        m_source = buffer;

        // FOOTER CHECKING
        //  1) Check, if EOF marking is present
        //  2) Find start of cross reference table
        checkFooter(buffer);
        const PDFInteger firstXrefTableOffset = findXrefTableOffset(buffer);

        // HEADER CHECKING
        //  1) Check if header is present
        //  2) Scan header version
        checkHeader(buffer);

        // Now, we are ready to scan xref table
        PDFXRefTable xrefTable;
        xrefTable.readXRefTable(nullptr, buffer, firstXrefTableOffset);

        if (xrefTable.getSize() == 0)
        {
            throw PDFException(tr("Empty xref table."));
        }

        PDFObjectStorage::PDFObjects objects;
        objects.resize(xrefTable.getSize());

        std::vector<PDFXRefTable::Entry> occupiedEntries = xrefTable.getOccupiedEntries();

        // First, process regular objects
        if (processReferenceTableEntries(&xrefTable, occupiedEntries, objects) != Result::OK)
        {
            // Do not proceed further, if document loading failed
            return PDFDocument();
        }

        if (processSecurityHandler(xrefTable.getTrailerDictionary(), occupiedEntries, objects) == Result::Cancelled)
        {
            return PDFDocument();
        }

        // We are past security inicialization. Do not attempt to restore damaged document from
        // this point. After this point, security is decrypted. If something fails here,
        // then document can't be restored (user can't be asked multiple times for password).
        shouldTryPermissiveReading = !m_securityHandler || m_securityHandler->getMode() == EncryptionMode::None;
        processObjectStreams(&xrefTable, objects);

        PDFObjectStorage storage(std::move(objects), PDFObject(xrefTable.getTrailerDictionary()), qMove(m_securityHandler));
        return PDFDocument(std::move(storage), m_version, hash(buffer));
    }
    catch (const PDFException &parserException)
    {
        m_result = Result::Failed;
        m_errorMessage = parserException.getMessage();
        m_warnings << m_errorMessage;
    }

    if (m_result == Result::Failed && m_permissive && shouldTryPermissiveReading)
    {
        return readDamagedDocumentFromBuffer(buffer);
    }

    return PDFDocument();
}

QByteArray PDFDocumentReader::hash(const QByteArray& sourceData)
{
    return QCryptographicHash::hash(sourceData, QCryptographicHash::Sha256);
}

std::vector<std::pair<int, int>> PDFDocumentReader::findObjectByteOffsets(const QByteArray& buffer) const
{
    std::vector<std::pair<int, int>> offsets;
    int lastOffset = 0;
    const int shift = static_cast<int>(std::strlen(PDF_OBJECT_END_MARK));
    while (lastOffset < buffer.size())
    {
        int offset = buffer.indexOf(PDF_OBJECT_END_MARK, lastOffset);

        // Object end mark was not found
        if (offset == -1)
        {
            break;
        }

        offset += shift;

        int startOffset = buffer.indexOf(PDF_OBJECT_START_MARK, lastOffset);
        if (startOffset != -1 && startOffset < offset)
        {
            --startOffset;

            // Skip whitespace between obj and generation number
            while (startOffset >= 0 && PDFLexicalAnalyzer::isWhitespace(buffer[startOffset]))
            {
                --startOffset;
            }

            // Skip generation number
            while (startOffset >= 0 && std::isdigit(buffer[startOffset]))
            {
                --startOffset;
            }

            // Skip whitespace between generation number and object number
            while (startOffset >= 0 && PDFLexicalAnalyzer::isWhitespace(buffer[startOffset]))
            {
                --startOffset;
            }

            // Skip object number
            while (startOffset >= 0 && std::isdigit(buffer[startOffset]))
            {
                --startOffset;
            }

            ++startOffset;

            if (startOffset < offset)
            {
                offsets.emplace_back(startOffset, offset);
            }
        }

        lastOffset = offset;
    }

    return offsets;
}

bool PDFDocumentReader::restoreObjects(std::map<PDFObjectReference, PDFObject>& restoredObjects, const std::vector<std::pair<int, int>>& offsets)
{
    QMutex restoredObjectsMutex;
    std::atomic_bool succesfull = true;

    auto getObject = [&restoredObjects, &restoredObjectsMutex](PDFParsingContext*, PDFObjectReference reference)
    {
        QMutexLocker lock(&restoredObjectsMutex);

        auto it = restoredObjects.find(reference);
        if (it != restoredObjects.cend())
        {
            return it->second;
        }

        return PDFObject();
    };

    auto processOffsetEntry = [&, this](const std::pair<int, int>& offset)
    {
        PDFParsingContext context(getObject);
        const int startOffset = offset.first;
        const int endOffset = offset.second;

        Q_ASSERT(startOffset >= 0 && startOffset < m_source.size());
        Q_ASSERT(endOffset >= 0 && endOffset <= m_source.size());
        Q_ASSERT(startOffset <= endOffset);

        try
        {
            const char* begin = m_source.constData() + startOffset;
            const char* end = m_source.constData() + endOffset;

            PDFParser parser(begin, end, &context, PDFParser::AllowStreams);
            PDFObject objectNumberObject = parser.getObject();
            PDFObject objectGenerationObject = parser.getObject();
            parser.fetchCommand(PDF_OBJECT_START_MARK);
            PDFObject object = parser.getObject();

            if (objectNumberObject.isInt() && objectGenerationObject.isInt() && !object.isNull())
            {
                PDFObjectReference reference(objectNumberObject.getInteger(), objectGenerationObject.getInteger());
                if (reference.isValid())
                {
                    QMutexLocker lock(&restoredObjectsMutex);
                    if (!restoredObjects.count(reference))
                    {
                        restoredObjects[reference] = qMove(object);
                    }
                }
            }
        }
        catch (const PDFException&)
        {
            // Do nothing
            succesfull = false;
        }
    };
    PDFExecutionPolicy::execute(PDFExecutionPolicy::Scope::Unknown, offsets.cbegin(), offsets.cend(), processOffsetEntry);
    return succesfull;
}

PDFDocument PDFDocumentReader::readDamagedDocumentFromBuffer(const QByteArray& buffer)
{
    try
    {
        m_result = Result::OK;

        // Try to reconstruct trailer dictionary
        std::map<PDFObjectReference, PDFObject> restoredObjects;

        PDFObject trailerDictionaryObject = readDamagedTrailerDictionary();
        if (!trailerDictionaryObject.isDictionary())
        {
            throw PDFException(PDFTranslationContext::tr("Trailer dictionary is not valid."));
        }

        // Jakub Melka: Try to parse objects - read offsets of objects. We must probably
        // try second pass, if some streams have referenced objects.
        std::vector<std::pair<int, int>> offsets = findObjectByteOffsets(buffer);
        if (!restoreObjects(restoredObjects, offsets))
        {
            restoreObjects(restoredObjects, offsets);
        }

        // We will create security handler.
        PDFObjectStorage::PDFObjects objects;
        std::vector<PDFXRefTable::Entry> occupiedEntries;

        if (!restoredObjects.empty())
        {
            objects.resize(restoredObjects.rbegin()->first.objectNumber + 1);

            for (auto& objectItem : restoredObjects)
            {
                PDFObjectReference reference = objectItem.first;
                PDFObjectStorage::Entry& entry = objects[reference.objectNumber];
                entry.generation = reference.generation;
                entry.object = qMove(objectItem.second);
            }
        }

        if (processSecurityHandler(trailerDictionaryObject, occupiedEntries, objects) == Result::Cancelled)
        {
            return PDFDocument();
        }

        PDFObjectStorage storage(std::move(objects), PDFObject(trailerDictionaryObject), qMove(m_securityHandler));
        return PDFDocument(std::move(storage), m_version, QByteArray());
    }
    catch (const PDFException &parserException)
    {
        m_result = Result::Failed;
        m_warnings << parserException.getMessage();
    }

    return PDFDocument();
}

void PDFDocumentReader::reset()
{
    m_result = Result::OK;
    m_errorMessage = QString();
    m_version = PDFVersion();
    m_source = QByteArray();
    m_securityHandler = nullptr;
}

int PDFDocumentReader::findFromEnd(const char* what, const QByteArray& byteArray, int limit)
{
    if (byteArray.isEmpty())
    {
        // Byte array is empty, no value found
        return FIND_NOT_FOUND_RESULT;
    }

    const int size = byteArray.size();
    const int adjustedLimit = qMin(byteArray.size(), limit);
    const int whatLength = static_cast<int>(std::strlen(what));

    if (adjustedLimit < whatLength)
    {
        // Buffer is smaller than scan string
        return FIND_NOT_FOUND_RESULT;
    }

    auto itBegin = std::next(byteArray.cbegin(), size - adjustedLimit);
    auto itEnd = byteArray.cend();
    auto it = std::find_end(itBegin, itEnd, what, std::next(what, whatLength));

    if (it != byteArray.cend())
    {
        return std::distance(byteArray.cbegin(), it);
    }

    return FIND_NOT_FOUND_RESULT;
}

void PDFDocumentReader::progressStart(size_t stepCount, QString text)
{
    if (m_progress)
    {
        ProgressStartupInfo info;
        info.showDialog = !text.isEmpty();
        info.text = qMove(text);

        m_progress->start(stepCount, qMove(info));
    }
}

void PDFDocumentReader::progressStep()
{
    if (m_progress)
    {
        m_progress->step();
    }
}

void PDFDocumentReader::progressFinish()
{
    if (m_progress)
    {
        m_progress->finish();
    }
}

}   // namespace pdf
