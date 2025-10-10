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

#include "pdfredact.h"
#include "pdfpainter.h"
#include "pdfdocumentbuilder.h"
#include "pdfoptimizer.h"
#include "pdfdbgheap.h"

namespace pdf
{

PDFRedact::PDFRedact(const PDFDocument* document,
                     const PDFFontCache* fontCache,
                     const PDFCMS* cms,
                     const PDFOptionalContentActivity* optionalContentActivity,
                     const PDFMeshQualitySettings* meshQualitySettings,
                     QColor redactFillColor) :
    m_document(document),
    m_fontCache(fontCache),
    m_cms(cms),
    m_optionalContentActivity(optionalContentActivity),
    m_meshQualitySettings(meshQualitySettings),
    m_redactFillColor(redactFillColor)
{

}

PDFDocument PDFRedact::perform(Options options)
{
    PDFDocumentBuilder builder;
    builder.createDocument();

    PDFRenderer renderer(m_document,
                         m_fontCache,
                         m_cms,
                         m_optionalContentActivity,
                         PDFRenderer::None,
                         *m_meshQualitySettings);

    std::map<PDFObjectReference, PDFObjectReference> mapOldPageRefToNewPageRef;

    for (size_t i = 0; i < m_document->getCatalog()->getPageCount(); ++i)
    {
        const PDFPage* page = m_document->getCatalog()->getPage(i);

        PDFPrecompiledPage compiledPage;
        renderer.compile(&compiledPage, i);

        PDFObjectReference newPageReference = builder.appendPage(page->getMediaBox());
        mapOldPageRefToNewPageRef[page->getPageReference()] = newPageReference;

        if (!page->getCropBox().isEmpty())
        {
            builder.setPageCropBox(newPageReference, page->getCropBox());
        }
        if (!page->getBleedBox().isEmpty())
        {
            builder.setPageBleedBox(newPageReference, page->getBleedBox());
        }
        if (!page->getTrimBox().isEmpty())
        {
            builder.setPageTrimBox(newPageReference, page->getTrimBox());
        }
        if (!page->getArtBox().isEmpty())
        {
            builder.setPageArtBox(newPageReference, page->getArtBox());
        }
        builder.setPageRotation(newPageReference, page->getPageRotation());

        PDFPageContentStreamBuilder contentStreamBuilder(&builder);

        QPainterPath redactPath;

        for (const PDFObjectReference& annotationReference : page->getAnnotations())
        {
            PDFAnnotationPtr annotation = PDFAnnotation::parse(&m_document->getStorage(), annotationReference);
            if (!annotation || annotation->getType() != AnnotationType::Redact)
            {
                continue;
            }

            // We have redact annotation here
            const PDFRedactAnnotation* redactAnnotation = dynamic_cast<const PDFRedactAnnotation*>(annotation.get());
            Q_ASSERT(redactAnnotation);

            redactPath = redactPath.united(redactAnnotation->getRedactionRegion().getPath());
        }

        QTransform matrix;
        matrix.translate(0, page->getMediaBox().height());
        matrix.scale(1.0, -1.0);

        QPainter* painter = contentStreamBuilder.begin(newPageReference);
        compiledPage.redact(redactPath, matrix, m_redactFillColor);
        compiledPage.draw(painter, QRectF(), matrix, PDFRenderer::None, 1.0);
        contentStreamBuilder.end(painter);
    }

    if (options.testFlag(CopyTitle))
    {
        builder.setDocumentTitle(m_document->getInfo()->title);
    }

    if (options.testFlag(CopyMetadata))
    {
        PDFObject info = m_document->getTrailerDictionary()->get("Info");
        if (!info.isNull())
        {
            std::vector<PDFObject> copiedObjects = builder.copyFrom({ info }, m_document->getStorage(), true);
            if (copiedObjects.size() == 1 && copiedObjects.front().isReference())
            {
                builder.setDocumentInfo(copiedObjects.front().getReference());
            }
        }
    }

    if (options.testFlag(CopyOutline))
    {
        PDFObject catalog = m_document->getObject(m_document->getTrailerDictionary()->get("Root"));
        if (const PDFDictionary* catalogDictionary = m_document->getDictionaryFromObject(catalog))
        {
            if (catalogDictionary->hasKey("Outlines"))
            {
                QSharedPointer<PDFOutlineItem> outlineRoot = PDFOutlineItem::parse(&m_document->getStorage(), catalogDictionary->get("Outlines"));

                if (outlineRoot)
                {
                    auto resolveNamedDestination = [this, &mapOldPageRefToNewPageRef](PDFOutlineItem* item)
                    {
                        PDFActionGoTo* action = dynamic_cast<PDFActionGoTo*>(item->getAction());
                        if (action)
                        {
                            if (action->getDestination().isNamedDestination())
                            {
                                const PDFDestination* destination = m_document->getCatalog()->getNamedDestination(action->getDestination().getName());
                                if (destination)
                                {
                                    action->setDestination(*destination);
                                }
                            }

                            PDFDestination destination = action->getDestination();
                            auto it = mapOldPageRefToNewPageRef.find(destination.getPageReference());
                            if (it != mapOldPageRefToNewPageRef.cend())
                            {
                                destination.setPageReference(it->second);
                                action->setDestination(destination);
                            }
                        }
                    };
                    outlineRoot->apply(resolveNamedDestination);

                    builder.setOutline(outlineRoot.data());
                }
            }
        }
    }

    PDFDocument redactedDocument = builder.build();
    PDFOptimizer optimizer(PDFOptimizer::All, nullptr);
    optimizer.setDocument(&redactedDocument);
    optimizer.optimize();
    return optimizer.takeOptimizedDocument();
}

}   // namespace pdf
