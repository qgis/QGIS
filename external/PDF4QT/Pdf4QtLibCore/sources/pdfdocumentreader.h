//    Copyright (C) 2018-2021 Jakub Melka
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


#ifndef PDFDOCUMENTREADER_H
#define PDFDOCUMENTREADER_H

#include "pdfglobal.h"
#include "pdfdocument.h"
#include "pdfprogress.h"
#include "pdfxreftable.h"

#include <QMutex>
#include <QIODevice>

namespace pdf
{
class PDFXRefTable;
class PDFParsingContext;

/// This class is a reader of PDF document from various devices (file, io device,
/// byte buffer). This class doesn't throw exceptions, to check errors, use
/// appropriate functions.
class PDF4QTLIBCORESHARED_EXPORT PDFDocumentReader
{
    Q_DECLARE_TR_FUNCTIONS(pdf::PDFDocumentReader)

public:
    explicit PDFDocumentReader(PDFProgress* progress, const std::function<QString(bool*)>& getPasswordCallback, bool permissive, bool authorizeOwnerOnly);

    constexpr inline PDFDocumentReader(const PDFDocumentReader&) = delete;
    constexpr inline PDFDocumentReader(PDFDocumentReader&&) = delete;
    constexpr inline PDFDocumentReader& operator=(const PDFDocumentReader&) = delete;
    constexpr inline PDFDocumentReader& operator=(PDFDocumentReader&&) = delete;

    enum class Result
    {
        OK,         ///< Document was successfully loaded
        Failed,     ///< Error occured during document reading
        Cancelled   ///< User cancelled document reading
    };

    /// Reads a PDF document from the specified file. If file doesn't exist,
    /// cannot be opened or contain invalid pdf, empty PDF file is returned.
    /// No exception is thrown.
    PDFDocument readFromFile(const QString& fileName);

    /// Reads a PDF document from the specified device. If device is not opened
    /// for reading, then function tries it to open for reading. If it is opened,
    /// but not for reading, empty PDF document is returned. This also occurs
    /// when incorrect PDF is read. No exception is thrown.
    PDFDocument readFromDevice(QIODevice* device);

    /// Reads a PDF document from the specified buffer (byte array). If incorrect
    /// PDF is read, then empty PDF document is returned. No exception is thrown.
    PDFDocument readFromBuffer(const QByteArray& buffer);

    /// Returns result code for reading document from the device
    Result getReadingResult() const { return m_result; }

    /// Returns error message, if document reading was unsuccessfull
    const QString& getErrorMessage() const { return m_errorMessage; }

    /// Get source data of the document
    const QByteArray& getSource() const { return m_source; }

    /// Returns warning messages
    const QStringList& getWarnings() const { return m_warnings; }

    static QByteArray hash(const QByteArray& sourceData);

private:
    static constexpr const int FIND_NOT_FOUND_RESULT = -1;

    /// Resets the internal state and prepares it for new reading cycle
    void reset();

    /// Find a last string in the byte array, scan only \p limit bytes. If string
    /// is not found, then FIND_NOT_FOUND_RESULT is returned, if it is found, then
    /// it position from the beginning of byte array is returned.
    /// \param what String to be found
    /// \param byteArray Byte array to be scanned from the end
    /// \param limit Scan up to this value bytes from the end
    /// \returns Position of string, or FIND_NOT_FOUND_RESULT
    int findFromEnd(const char* what, const QByteArray& byteArray, int limit);

    void checkFooter(const QByteArray& buffer);
    void checkHeader(const QByteArray& buffer);
    PDFInteger findXrefTableOffset(const QByteArray& buffer);
    Result processReferenceTableEntries(PDFXRefTable* xrefTable, const std::vector<PDFXRefTable::Entry>& occupiedEntries, PDFObjectStorage::PDFObjects& objects);
    Result processSecurityHandler(const PDFObject& trailerDictionaryObject, const std::vector<PDFXRefTable::Entry>& occupiedEntries, PDFObjectStorage::PDFObjects& objects);
    void processObjectStreams(PDFXRefTable* xrefTable, PDFObjectStorage::PDFObjects& objects);

    /// This function fetches object from the buffer from the specified offset.
    /// Can throw exception, returns a pair of scanned reference and object content.
    /// \param context Context
    /// \param offset Offset
    /// \param reference Reference to parsed object
    PDFObject getObject(PDFParsingContext* context, PDFInteger offset, PDFObjectReference reference) const;

    /// Tries to restore objects from object list. This function can be used in multiple pass, because
    /// for example streams, can have length defined in referred object. If such is the case, then
    /// second pass is needed. Returns true, if all object were correctly read.
    /// \param restoredObjects Map of restored objects
    /// \param offsets Offsets, from which are objects being read
    bool restoreObjects(std::map<PDFObjectReference, PDFObject>& restoredObjects, const std::vector<std::pair<int, int>>& offsets);

    /// Fetch object from reference table
    PDFObject getObjectFromXrefTable(PDFXRefTable* xrefTable, PDFParsingContext* context, PDFObjectReference reference) const;

    /// Tries to read damaged trailer dictionary
    PDFObject readDamagedTrailerDictionary() const;

    /// Attempts to read a damaged PDF document from the specified buffer (byte array). If incorrect
    /// PDF is read, then empty PDF document is returned. No exception is thrown.
    PDFDocument readDamagedDocumentFromBuffer(const QByteArray& buffer);

    /// This function is used, when damaged pdf document is being restored. It returns
    /// array of hints, where objects should appear. It constists of pair of start offset,
    /// and end offset. Start offset is always a valid index to the buffer, end offset
    /// can be one index after the buffers end (it is end iterator).
    /// \param buffer Buffer
    std::vector<std::pair<int, int>> findObjectByteOffsets(const QByteArray& buffer) const;

    void progressStart(size_t stepCount, QString text);
    void progressStep();
    void progressFinish();

    /// Mutex for access to variables of this reader from more threads
    /// (providing thread safety)
    QMutex m_mutex;

    /// Result of document reading from the device
    std::atomic<Result> m_result;

    /// In case if error occurs, it is stored here
    QString m_errorMessage;

    /// Version of the scanned file
    PDFVersion m_version;

    /// Callback to obtain password from the user
    std::function<QString(bool*)> m_getPasswordCallback;

    /// Progress indicator
    PDFProgress* m_progress;

    /// Raw document data (byte array containing source data for created document)
    QByteArray m_source;

    /// Security handler
    PDFSecurityHandlerPointer m_securityHandler;

    /// Be permissive when reading, tolerate errors and try to fix broken document
    bool m_permissive;

    /// Authorize as owner only (if owner authorization fails, then whole document
    /// reading fails)
    bool m_authorizeOwnerOnly;

    /// Warnings
    QStringList m_warnings;
};

}   // namespace pdf

#endif // PDFDOCUMENTREADER_H
