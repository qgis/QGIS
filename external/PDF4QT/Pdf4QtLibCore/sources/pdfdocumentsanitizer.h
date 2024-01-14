//    Copyright (C) 2023 Jakub Melka
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
//    along with PDF4QT. If not, see <https://www.gnu.org/licenses/>.

#ifndef PDFDOCUMENTSANITIZER_H
#define PDFDOCUMENTSANITIZER_H

#include "pdfdocument.h"

namespace pdf
{
class PDFAnnotation;

/// Class for sanitizing documents. Can remove sensitive content from the document,
/// except the content streams. Sanitization is configurable, user can specify,
/// which content should be removed.
class PDF4QTLIBCORESHARED_EXPORT PDFDocumentSanitizer : public QObject
{
    Q_OBJECT

public:

    enum SanitizationFlag
    {
        None                = 0x0000, ///< No sanitization is performed
        DocumentInfo        = 0x0001, ///< Remove document information
        Metadata            = 0x0002, ///< Remove all metadata streams in all objects
        Outline             = 0x0004, ///< Remove outline
        FileAttachments     = 0x0008, ///< Remove file attachments
        EmbeddedSearchIndex = 0x0010, ///< Remove embedded search index
        MarkupAnnotations   = 0x0020, ///< Remove markup annotations from all pages
        PageThumbnails      = 0x0040, ///< Remove page thumbnails
        All                 = 0xFFFF, ///< All sanitization turned on
    };
    Q_DECLARE_FLAGS(SanitizationFlags, SanitizationFlag)

    explicit PDFDocumentSanitizer(SanitizationFlag flags, QObject* parent);

    /// Set document, which should be sanitized
    /// \param document Document to be sanitized
    void setDocument(const PDFDocument* document) { setStorage(document->getStorage()); }

    /// Set storage directly (storage must be valid and filled with objects)
    /// \param storage Storage
    void setStorage(const PDFObjectStorage& storage) { m_storage = storage; }

    /// Perform document sanitization. During optimization process, various
    /// signals are emitted to view progress.
    void sanitize();

    /// Returns object storage used for optimization
    const PDFObjectStorage& getStorage() const { return m_storage; }

    /// Returns object storage by move semantics, old object storage is destroyed
    PDFObjectStorage takeStorage() { return qMove(m_storage); }

    /// Returns sanitized document. Object storage is cleared after
    /// this function call.
    PDFDocument takeSanitizedDocument() { return PDFDocument(qMove(m_storage), PDFVersion(2, 0), QByteArray()); }

    SanitizationFlags getFlags() const;
    void setFlags(SanitizationFlags flags);

signals:
    void sanitizationStarted();
    void sanitizationProgress(QString progressText);
    void sanitizationFinished();

private:
    void performSanitizeDocumentInfo();
    void performSanitizeMetadata();
    void performSanitizeOutline();
    void performSanitizeFileAttachments();
    void performSanitizeEmbeddedSearchIndex();
    void performSanitizeMarkupAnnotations();
    void performSanitizePageThumbnails();

    void removeAnnotations(const std::function<bool(const PDFAnnotation*)>& filter, QString message);

    SanitizationFlags m_flags;
    PDFObjectStorage m_storage;
};

}   // namespace pdf

#endif // PDFDOCUMENTSANITIZER_H
