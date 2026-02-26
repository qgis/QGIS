// MIT License
//
// Copyright (c) 2018-2025 Jakub Melka and Contributors
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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
        PageLabels          = 0x0080, ///< Remove page labels
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
    void performSanitizePageLabels();

    void removeAnnotations(const std::function<bool(const PDFAnnotation*)>& filter, QString message);

    SanitizationFlags m_flags;
    PDFObjectStorage m_storage;
};

}   // namespace pdf

#endif // PDFDOCUMENTSANITIZER_H
