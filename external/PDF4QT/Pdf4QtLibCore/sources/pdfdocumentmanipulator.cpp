//    Copyright (C) 2021-2022 Jakub Melka
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

#include "pdfdocumentmanipulator.h"
#include "pdfdocumentbuilder.h"
#include "pdfoptimizer.h"
#include "pdfdbgheap.h"

namespace pdf
{

PDFOperationResult PDFDocumentManipulator::assemble(const AssembledPages& pages)
{
    if (pages.empty())
    {
        return tr("Empty page list.");
    }

    m_flags = None;
    m_mergedObjects = { };
    m_assembledDocument = PDFDocument();

    try
    {
        classify(pages);

        pdf::PDFDocumentBuilder documentBuilder;
        if (m_flags.testFlag(SingleDocument))
        {
            PDFInteger documentIndex = -1;

            for (const AssembledPage& assembledPage : pages)
            {
                if (assembledPage.isDocumentPage())
                {
                    documentIndex = assembledPage.documentIndex;
                }
            }

            if (documentIndex == -1 || !m_documents.count(documentIndex))
            {
                throw PDFException(tr("Invalid document."));
            }

            documentBuilder.setDocument(m_documents.at(documentIndex));
        }
        else
        {
            documentBuilder.createDocument();
        }

        initializeMergedObjects(documentBuilder);

        ProcessedPages processedPages = processPages(documentBuilder, pages);
        std::vector<PDFObjectReference> adjustedPages;
        std::transform(processedPages.cbegin(), processedPages.cend(), std::back_inserter(adjustedPages), [](const auto& page) { return page.targetPageReference; });
        documentBuilder.setPages(adjustedPages);

        // Correct page tree (invalid parents are present)
        documentBuilder.flattenPageTree();
        if (!m_flags.testFlag(SingleDocument) || m_flags.testFlag(RemovedPages))
        {
            documentBuilder.removeOutline();
            documentBuilder.removeThreads();
            documentBuilder.removeDocumentActions();
            documentBuilder.removeStructureTree();
        }

        // Jakub Melka: we also create document parts for each document part (if we aren't
        // manipulating a single document).
        if (!m_flags.testFlag(SingleDocument))
        {
            addOutlineAndDocumentParts(documentBuilder, pages, adjustedPages);
        }

        pdf::PDFDocument mergedDocument = documentBuilder.build();

        // Optimize document - remove unused objects and shrink object storage
        finalizeDocument(&mergedDocument);
    }
    catch (const PDFException& exception)
    {
        return exception.getMessage();
    }

    return true;
}

PDFDocumentManipulator::AssembledPages PDFDocumentManipulator::createAllDocumentPages(int documentIndex, const PDFDocument* document)
{
    AssembledPages assembledPages;
    size_t pageCount = document->getCatalog()->getPageCount();

    for (size_t i = 0; i < pageCount; ++i)
    {
        pdf::PDFDocumentManipulator::AssembledPage assembledPage;

        assembledPage.documentIndex = documentIndex;
        assembledPage.imageIndex = -1;
        assembledPage.pageIndex = i;

        const pdf::PDFPage* page = document->getCatalog()->getPage(i);
        const pdf::PageRotation originalPageRotation = page->getPageRotation();

        assembledPage.pageRotation = originalPageRotation;
        assembledPage.pageSize = page->getMediaBox().size();

        assembledPages.emplace_back(assembledPage);
    }

    return assembledPages;
}

PDFDocumentManipulator::ProcessedPages PDFDocumentManipulator::processPages(PDFDocumentBuilder& documentBuilder, const AssembledPages& pages)
{
    ProcessedPages processedPages;

    // First, decide, if we are manipulating a single document, or
    // an array of documents. If the former is the case, then we do not
    // want to copy objects, it is just unnecessary. If the latter is the case,
    // then we must

    if (m_flags.testFlag(SingleDocument))
    {
        documentBuilder.flattenPageTree();
        std::vector<PDFObjectReference> pageReferences = documentBuilder.getPages();
        std::set<PDFObjectReference> usedPages;

        processedPages.reserve(pageReferences.size());
        for (const AssembledPage& assembledPage : pages)
        {
            ProcessedPage processedPage;
            processedPage.assembledPage = assembledPage;

            if (assembledPage.isDocumentPage())
            {
                const PDFInteger pageIndex = assembledPage.pageIndex;

                if (pageIndex < 0 || pageIndex >= PDFInteger(pageReferences.size()))
                {
                    throw PDFException(tr("Missing page (%1) in a document.").arg(pageIndex));
                }

                PDFObjectReference pageReference = pageReferences[pageIndex];
                if (!usedPages.count(pageReference))
                {
                    processedPage.targetPageReference = pageReference;
                    usedPages.insert(pageReference);
                }
                else
                {
                    // Page is being cloned. So we must clone it...
                    std::vector<pdf::PDFObjectReference> references = pdf::PDFDocumentBuilder::createReferencesFromObjects(documentBuilder.copyFrom(pdf::PDFDocumentBuilder::createObjectsFromReferences({ pageReference }), *documentBuilder.getStorage(), true));
                    Q_ASSERT(references.size() == 1);
                    processedPage.targetPageReference = references.front();
                    usedPages.insert(references.front());
                }
            }

            processedPages.push_back(processedPage);
        }
    }
    else
    {
        processedPages = collectObjectsAndCopyPages(documentBuilder, pages);
    }

    // Now, create "special" pages, such as image pages or blank pages, and rotate
    // final pages (we must check, that page object exists).
    for (ProcessedPage& processedPage : processedPages)
    {
        if (processedPage.assembledPage.isBlankPage() || processedPage.assembledPage.isImagePage())
        {
            QImage image;

            if (processedPage.assembledPage.isImagePage())
            {
                const PDFInteger imageIndex = processedPage.assembledPage.imageIndex;

                if (!m_images.count(imageIndex))
                {
                    throw PDFException(tr("Missing image."));
                }

                image = m_images.at(imageIndex);

                if (image.isNull())
                {
                    throw PDFException(tr("Missing image."));
                }
            }

            QRectF pageRect = QRectF(QPointF(0, 0), processedPage.assembledPage.pageSize * PDF_MM_TO_POINT);
            processedPage.targetPageReference = documentBuilder.appendPage(pageRect);
            PDFPageContentStreamBuilder contentStreamBuilder(&documentBuilder);

            QPainter* painter = contentStreamBuilder.begin(processedPage.targetPageReference);

            if (processedPage.assembledPage.isImagePage())
            {
                // Just paint the image
                painter->drawImage(pageRect, image);
            }

            contentStreamBuilder.end(painter);
        }

        if (!processedPage.targetPageReference.isValid())
        {
            throw PDFException(tr("Error occured during page creation."));
        }

        documentBuilder.setPageRotation(processedPage.targetPageReference, processedPage.assembledPage.pageRotation);
    }

    return processedPages;
}

PDFDocumentManipulator::ProcessedPages PDFDocumentManipulator::collectObjectsAndCopyPages(PDFDocumentBuilder& documentBuilder, const AssembledPages& pages)
{
    ProcessedPages processedPages;
    processedPages.reserve(pages.size());

    std::map<std::pair<int, int>, PDFObjectReference> documentPages;

    for (const AssembledPage& assembledPage : pages)
    {
        ProcessedPage processedPage;
        processedPage.assembledPage = assembledPage;
        processedPages.push_back(processedPage);

        if (assembledPage.isDocumentPage())
        {
            documentPages[std::make_pair(assembledPage.documentIndex, assembledPage.pageIndex)] = PDFObjectReference();
        }
    }

    for (auto it = documentPages.begin(); it != documentPages.end();)
    {
        const int documentIndex = it->first.first;

        // Jakub Melka: we will find end of a single document page range
        auto itEnd = it;
        while (itEnd != documentPages.end() && itEnd->first.first == documentIndex)
        {
            ++itEnd;
        }

        if (documentIndex != -1)
        {
            // Check we have the document
            if (!m_documents.count(documentIndex))
            {
                throw PDFException(tr("Invalid document."));
            }
            const PDFDocument* document = m_documents.at(documentIndex);

            // Copy the pages into the target document builder
            std::vector<PDFInteger> pageIndices;
            for (auto currentIt = it; currentIt != itEnd; ++currentIt)
            {
                pageIndices.push_back(currentIt->first.second);
            }

            pdf::PDFDocumentBuilder temporaryBuilder(document);
            temporaryBuilder.flattenPageTree();

            std::vector<pdf::PDFObjectReference> currentPages = temporaryBuilder.getPages();
            std::vector<pdf::PDFObjectReference> objectsToMerge;
            objectsToMerge.reserve(std::distance(it, itEnd));

            for (PDFInteger pageIndex : pageIndices)
            {
                if (pageIndex < 0 || pageIndex >= static_cast< PDFInteger >(currentPages.size()))
                {
                    throw PDFException(tr("Missing page (%1) in a document.").arg(pageIndex));
                }

                objectsToMerge.push_back(currentPages[pageIndex]);
            }

            pdf::PDFObjectReference acroFormReference;
            pdf::PDFObjectReference namesReference;
            pdf::PDFObjectReference ocPropertiesReference;
            pdf::PDFObjectReference outlineReference;

            pdf::PDFObject formObject = document->getCatalog()->getFormObject();
            if (formObject.isReference())
            {
                acroFormReference = formObject.getReference();
            }
            else
            {
                acroFormReference = temporaryBuilder.addObject(formObject);
            }

            if (const pdf::PDFDictionary* catalogDictionary = temporaryBuilder.getDictionaryFromObject(temporaryBuilder.getObjectByReference(temporaryBuilder.getCatalogReference())))
            {
                pdf::PDFObject namesObject = catalogDictionary->get("Names");
                if (namesObject.isReference())
                {
                    namesReference = namesObject.getReference();
                }

                pdf::PDFObject ocPropertiesObject = catalogDictionary->get("OCProperties");
                if (ocPropertiesObject.isReference())
                {
                    ocPropertiesReference = ocPropertiesObject.getReference();
                }

                pdf::PDFObject outlineObject = catalogDictionary->get("Outlines");
                if (outlineObject.isReference())
                {
                    outlineReference = outlineObject.getReference();
                }
            }

            if (!namesReference.isValid())
            {
                namesReference = temporaryBuilder.addObject(pdf::PDFObject());
            }

            if (!ocPropertiesReference.isValid())
            {
                ocPropertiesReference = temporaryBuilder.addObject(pdf::PDFObject());
            }

            if (!outlineReference.isValid())
            {
                outlineReference = temporaryBuilder.addObject(pdf::PDFObject());
            }

            objectsToMerge.insert(objectsToMerge.end(), { acroFormReference, namesReference, ocPropertiesReference, outlineReference });

            // Now, we are ready to merge objects into target document builder
            std::vector<pdf::PDFObjectReference> references = pdf::PDFDocumentBuilder::createReferencesFromObjects(documentBuilder.copyFrom(pdf::PDFDocumentBuilder::createObjectsFromReferences(objectsToMerge), *temporaryBuilder.getStorage(), true));

            outlineReference = references.back();
            references.pop_back();
            ocPropertiesReference = references.back();
            references.pop_back();
            namesReference = references.back();
            references.pop_back();
            acroFormReference = references.back();
            references.pop_back();

            documentBuilder.appendTo(m_mergedObjects[MOT_OCProperties], documentBuilder.getObjectByReference(ocPropertiesReference));
            documentBuilder.appendTo(m_mergedObjects[MOT_Form], documentBuilder.getObjectByReference(acroFormReference));
            documentBuilder.mergeNames(m_mergedObjects[MOT_Names], namesReference);
            m_outlines[documentIndex] = outlineReference;

            Q_ASSERT(references.size() == size_t(std::distance(it, itEnd)));

            auto referenceIt = references.begin();
            for (auto currentIt = it; currentIt != itEnd; ++currentIt, ++referenceIt)
            {
                currentIt->second = *referenceIt;
            }
        }

        // Advance the index
        it = itEnd;
    }

    std::set<PDFObjectReference> usedReferences;
    for (ProcessedPage& processedPage : processedPages)
    {
        if (processedPage.assembledPage.isDocumentPage())
        {
            auto key = std::make_pair(processedPage.assembledPage.documentIndex, processedPage.assembledPage.pageIndex);
            Q_ASSERT(documentPages.count(key));

            PDFObjectReference pageReference = documentPages.at(key);
            if (!usedReferences.count(pageReference))
            {
                processedPage.targetPageReference = pageReference;
                usedReferences.insert(pageReference);
            }
            else
            {
                // Page is being cloned. So we must clone it...
                std::vector<pdf::PDFObjectReference> references = pdf::PDFDocumentBuilder::createReferencesFromObjects(documentBuilder.copyFrom(pdf::PDFDocumentBuilder::createObjectsFromReferences({ pageReference }), *documentBuilder.getStorage(), true));
                Q_ASSERT(references.size() == 1);
                processedPage.targetPageReference = references.front();
                usedReferences.insert(references.front());
            }
        }
    }

    return processedPages;
}

void PDFDocumentManipulator::classify(const AssembledPages& pages)
{
    m_flags = None;

    std::set<PDFInteger> documentIndices;
    std::set<PDFInteger> pageIndices;
    for (const AssembledPage& assembledPage : pages)
    {
        documentIndices.insert(assembledPage.documentIndex);
        pageIndices.insert(assembledPage.pageIndex);
    }

    documentIndices.erase(-1);
    pageIndices.erase(-1);

    m_flags.setFlag(SingleDocument, documentIndices.size() == 1);

    if (m_flags.testFlag(SingleDocument) && m_documents.count(*documentIndices.begin()))
    {
        const PDFDocument* document = m_documents.at(*documentIndices.begin());
        const bool pagesRemoved = pageIndices.size() < document->getCatalog()->getPageCount();
        m_flags.setFlag(RemovedPages, pagesRemoved);
    }
}

void PDFDocumentManipulator::initializeMergedObjects(PDFDocumentBuilder& documentBuilder)
{
    m_mergedObjects[MOT_OCProperties] = documentBuilder.addObject(PDFObject());
    m_mergedObjects[MOT_Form] = documentBuilder.addObject(PDFObject());
    m_mergedObjects[MOT_Names] = documentBuilder.addObject(PDFObject());
}

void PDFDocumentManipulator::finalizeMergedObjects(PDFDocumentBuilder& documentBuilder)
{
    if (!m_flags.testFlag(SingleDocument))
    {
        if (!documentBuilder.getObjectByReference(m_mergedObjects[MOT_OCProperties]).isNull())
        {
            documentBuilder.setCatalogOptionalContentProperties(m_mergedObjects[MOT_OCProperties]);
        }

        if (!documentBuilder.getObjectByReference(m_mergedObjects[MOT_Names]).isNull())
        {
            documentBuilder.setCatalogNames(m_mergedObjects[MOT_Names]);
        }

        if (!documentBuilder.getObjectByReference(m_mergedObjects[MOT_Form]).isNull())
        {
            documentBuilder.setCatalogAcroForm(m_mergedObjects[MOT_Form]);
        }
    }
}

void PDFDocumentManipulator::finalizeDocument(PDFDocument* document)
{
    auto optimizationFlags = pdf::PDFOptimizer::OptimizationFlags(PDFOptimizer::RemoveUnusedObjects |
                                                                  PDFOptimizer::ShrinkObjectStorage |
                                                                  PDFOptimizer::DereferenceSimpleObjects |
                                                                  PDFOptimizer::MergeIdenticalObjects);
    PDFOptimizer optimizer(optimizationFlags, nullptr);
    optimizer.setDocument(document);
    optimizer.optimize();
    PDFDocument mergedDocument = optimizer.takeOptimizedDocument();

    // We must adjust some objects - they can have merged objects
    pdf::PDFDocumentBuilder finalBuilder(&mergedDocument);
    if (const pdf::PDFDictionary* dictionary = finalBuilder.getDictionaryFromObject(finalBuilder.getObjectByReference(finalBuilder.getCatalogReference())))
    {
        pdf::PDFDocumentDataLoaderDecorator loader(finalBuilder.getStorage());
        pdf::PDFObjectReference ocPropertiesReference = loader.readReferenceFromDictionary(dictionary, "OCProperties");
        if (ocPropertiesReference.isValid())
        {
            finalBuilder.setObject(ocPropertiesReference, pdf::PDFObjectManipulator::removeDuplicitReferencesInArrays(finalBuilder.getObjectByReference(ocPropertiesReference)));
        }
        pdf::PDFObjectReference acroFormReference = loader.readReferenceFromDictionary(dictionary, "AcroForm");
        if (acroFormReference.isValid())
        {
            finalBuilder.setObject(acroFormReference, pdf::PDFObjectManipulator::removeDuplicitReferencesInArrays(finalBuilder.getObjectByReference(acroFormReference)));
        }
    }
    m_assembledDocument = finalBuilder.build();
}

void PDFDocumentManipulator::addOutlineAndDocumentParts(PDFDocumentBuilder& documentBuilder,
                                                        const AssembledPages& pages,
                                                        const std::vector<PDFObjectReference>& adjustedPages)
{
    PDFInteger lastDocumentIndex = pages.front().documentIndex;

    struct DocumentPartInfo
    {
        size_t pageCount = 0;
        PDFInteger documentIndex = 0;
        bool isWholeDocument = false;
        QString caption;
        PDFObjectReference firstPageReference;
    };
    std::vector<DocumentPartInfo> documentParts = { DocumentPartInfo() };

    PDFClosedIntervalSet pageNumbers;
    PDFInteger imageCount = 0;
    PDFInteger blankPageCount = 0;
    PDFInteger totalPageCount = 0;

    auto addDocumentPartCaption = [&](PDFInteger documentIndex)
    {
        DocumentPartInfo& info = documentParts.back();

        QString documentTitle;
        if (documentIndex != -1 && m_documents.count(documentIndex))
        {
            const PDFDocument* document = m_documents.at(documentIndex);
            documentTitle = document->getInfo()->title;
            if (documentTitle.isEmpty())
            {
                documentTitle = tr("Document %1").arg(documentIndex);
            }

            if (pageNumbers.getTotalLength() < PDFInteger(document->getCatalog()->getPageCount()))
            {
                documentTitle = tr("%1, p. %2").arg(documentTitle, pageNumbers.toText(true));
            }
            else
            {
                info.isWholeDocument = true;
            }
        }
        else if (imageCount > 0 && blankPageCount == 0)
        {
            documentTitle = tr("%1 Images").arg(imageCount);
        }
        else
        {
            documentTitle = tr("%1 Pages").arg(imageCount + blankPageCount);
        }

        info.caption = documentTitle;
        info.documentIndex = documentIndex;
        info.firstPageReference = (totalPageCount < PDFInteger(adjustedPages.size())) ? adjustedPages[totalPageCount] : PDFObjectReference();

        pageNumbers = PDFClosedIntervalSet();
        imageCount = 0;
        blankPageCount = 0;
        totalPageCount += info.pageCount;
    };

    for (const AssembledPage& page : pages)
    {
        if (page.documentIndex == lastDocumentIndex)
        {
            ++documentParts.back().pageCount;
        }
        else
        {
            addDocumentPartCaption(lastDocumentIndex);
            documentParts.push_back(DocumentPartInfo());
            ++documentParts.back().pageCount;
            lastDocumentIndex = page.documentIndex;
        }

        if (page.isDocumentPage())
        {
            pageNumbers.addValue(page.pageIndex + 1);
        }

        if (page.isImagePage())
        {
            ++imageCount;
        }

        if (page.isBlankPage())
        {
            ++blankPageCount;
        }
    }
    addDocumentPartCaption(lastDocumentIndex);

    std::vector<size_t> documentPartPageCounts;
    std::transform(documentParts.cbegin(), documentParts.cend(), std::back_inserter(documentPartPageCounts), [](const auto& part) { return part.pageCount; });
    documentBuilder.createDocumentParts(documentPartPageCounts);

    if (m_outlineMode != OutlineMode::NoOutline)
    {
        QSharedPointer<PDFOutlineItem> rootItem(new PDFOutlineItem());

        for (const DocumentPartInfo& info : documentParts)
        {
            QSharedPointer<PDFOutlineItem> documentPartItem(new PDFOutlineItem);
            documentPartItem->setAction(PDFActionPtr(new PDFActionGoTo(PDFDestination::createFit(info.firstPageReference), PDFDestination())));
            documentPartItem->setTitle(info.caption);
            documentPartItem->setFontBold(true);

            if (m_outlineMode == OutlineMode::Join && info.isWholeDocument)
            {
                const PDFInteger documentIndex = info.documentIndex;
                QSharedPointer<PDFOutlineItem> outline = PDFOutlineItem::parse(documentBuilder.getStorage(), PDFObject::createReference(m_outlines.at(documentIndex)));
                if (outline)
                {
                    for (size_t i = 0; i < outline->getChildCount(); ++i)
                    {
                        documentPartItem->addChild(outline->getChildPtr(i));
                    }
                }
            }

            rootItem->addChild(std::move(documentPartItem));
        }

        documentBuilder.setOutline(rootItem.data());
    }
}

PDFDocumentManipulator::OutlineMode PDFDocumentManipulator::getOutlineMode() const
{
    return m_outlineMode;
}

void PDFDocumentManipulator::setOutlineMode(OutlineMode outlineMode)
{
    m_outlineMode = outlineMode;
}

}   // namespace pdf
