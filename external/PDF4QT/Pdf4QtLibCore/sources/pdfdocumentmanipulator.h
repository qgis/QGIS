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

#ifndef PDFDOCUMENTMANIPULATOR_H
#define PDFDOCUMENTMANIPULATOR_H

#include "pdfdocument.h"
#include "pdfutils.h"

#include <QImage>

namespace pdf
{

/// Document page assembler/manipulator. Can assemble document(s) pages
/// to a new document, where pages are inserted/removed/moved, or joined
/// from another documents, or blank pages/image pages inserted. Document
/// is also optimized.
class PDF4QTLIBCORESHARED_EXPORT PDFDocumentManipulator
{
    Q_DECLARE_TR_FUNCTIONS(pdf::PDFDocumentManipulator)

public:
    explicit PDFDocumentManipulator() = default;

    /// Selects outline creation mode, when multiple documents
    /// are merged into one. For single document manipulation,
    /// this has no meaning.
    enum class OutlineMode
    {
        NoOutline,
        Join,
        DocumentParts
    };

    struct AssembledPage
    {
        PDFInteger documentIndex = -1; ///< Source document index. If page is not from a document, value is -1.
        PDFInteger imageIndex = -1; ///< Source image index. If page is not from a image, value is -1.
        PDFInteger pageIndex = -1; ///< Source document page index. If page is not from a document, value is -1.
        QSizeF pageSize; ///< Unrotated page size
        PageRotation pageRotation = PageRotation::None; ///< Page rotation

        constexpr bool isDocumentPage() const { return documentIndex != -1; }
        constexpr bool isImagePage() const { return imageIndex != -1; }
        constexpr bool isBlankPage() const { return documentIndex == -1 && imageIndex == -1; }
    };

    using AssembledPages = std::vector<AssembledPage>;

    /// Adds document with given index to available document list
    /// \param documentIndex Document index
    /// \param document Document
    void addDocument(int documentIndex, const PDFDocument* document) { m_documents[documentIndex] = document; }

    /// Adds image with given index to available image list
    /// \param imageIndex Image index
    /// \param image Image
    void addImage(int imageIndex, QImage image) { m_images[imageIndex] = std::move(image); }

    /// Assembles pages into a new document. Returns true, if a new document
    /// was assembled, otherwise error message is being returned. Assebmled
    /// document can be accessed trough a given getters.
    /// \param pages Pages
    /// \returns True or error message
    PDFOperationResult assemble(const AssembledPages& pages);

    /// Returns reference to an assembled document. This function should
    /// be called only, if method \p assemble returns true, otherwise
    /// undefined document can be returned.
    /// \returns Assembled document
    const PDFDocument& getAssembledDocument() const { return m_assembledDocument; }

    /// Returns rvalue reference to an assembled document. This function should
    /// be called only, if method \p assemble returns true, otherwise
    /// undefined document can be returned.
    /// \returns Assembled document
    PDFDocument&& takeAssembledDocument() { return std::move(m_assembledDocument); }

    static AssembledPages createAllDocumentPages(int documentIndex, const PDFDocument* document);

    static constexpr AssembledPage createDocumentPage(int documentIndex, int pageIndex, QSizeF pageSize, PageRotation pageRotation) { return AssembledPage{ documentIndex, -1, pageIndex, pageSize, pageRotation}; }
    static constexpr AssembledPage createImagePage(int imageIndex, QSizeF pageSize, PageRotation pageRotation) { return AssembledPage{ -1, imageIndex, -1, pageSize, pageRotation}; }
    static constexpr AssembledPage createBlankPage(QSizeF pageSize, PageRotation pageRotation) { return AssembledPage{ -1, -1, -1, pageSize, pageRotation}; }

    OutlineMode getOutlineMode() const;
    void setOutlineMode(OutlineMode outlineMode);

private:

    struct ProcessedPage
    {
        AssembledPage assembledPage;
        PDFObjectReference targetPageReference;
    };

    using ProcessedPages = std::vector<ProcessedPage>;

    enum AssembleFlag
    {
        None            = 0x0000,
        SingleDocument  = 0x0001, ///< We are assembling a single page document (possibly with blank pages / image pages
        RemovedPages    = 0x0002, ///< Document contains removed pages
    };
    Q_DECLARE_FLAGS(AssembleFlags, AssembleFlag)

    enum MergedObjectType
    {
        MOT_OCProperties,
        MOT_Form,
        MOT_Names,
        MOT_Last
    };

    /// Processes pages given a page list and a document builder.
    /// \param documentBuilder Document builder
    /// \param pages Pages to be processed
    /// \returns Processed pages
    ProcessedPages processPages(PDFDocumentBuilder& documentBuilder, const AssembledPages& pages);

    /// Collects objects and copies them into the target document builder.
    /// \param documentBuilder Document builder
    /// \param pages Pages to be copied
    /// \returns Processed pages
    ProcessedPages collectObjectsAndCopyPages(PDFDocumentBuilder& documentBuilder, const AssembledPages& pages);

    void classify(const AssembledPages& pages);
    void initializeMergedObjects(PDFDocumentBuilder& documentBuilder);
    void finalizeMergedObjects(PDFDocumentBuilder& documentBuilder);
    void finalizeDocument(PDFDocument* document);
    void addOutlineAndDocumentParts(PDFDocumentBuilder& documentBuilder,
                                    const AssembledPages& pages,
                                    const std::vector<PDFObjectReference>& adjustedPages);
    void filterOutline(PDFDocumentBuilder& documentBuilder,
                       const PDFDocument* singleDocument,
                       const std::vector<PDFObjectReference>& adjustedPages);

    std::map<PDFInteger, const PDFDocument*> m_documents;
    std::map<PDFInteger, QImage> m_images;
    AssembleFlags m_flags = None;
    std::array<PDFObjectReference, MOT_Last> m_mergedObjects = { };
    PDFDocument m_assembledDocument;
    OutlineMode m_outlineMode = OutlineMode::DocumentParts;
    std::map<PDFInteger, PDFObjectReference> m_outlines;
};

}   // namespace pdf

#endif // PDFDOCUMENTMANIPULATOR_H
