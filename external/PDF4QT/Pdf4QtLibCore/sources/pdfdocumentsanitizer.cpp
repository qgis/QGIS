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

#include "pdfdocumentsanitizer.h"
#include "pdfvisitor.h"
#include "pdfexecutionpolicy.h"
#include "pdfoptimizer.h"
#include "pdfdocumentbuilder.h"
#include "pdfdbgheap.h"

namespace pdf
{

class PDFRemoveMetadataVisitor : public PDFUpdateObjectVisitor
{
public:
    explicit PDFRemoveMetadataVisitor(const PDFObjectStorage* storage, std::atomic<PDFInteger>* counter) :
        PDFUpdateObjectVisitor(storage),
        m_counter(counter)
    {

    }

    virtual void visitDictionary(const PDFDictionary* dictionary) override;

private:
    std::atomic<PDFInteger>* m_counter;
};

void PDFRemoveMetadataVisitor::visitDictionary(const PDFDictionary* dictionary)
{
    Q_ASSERT(dictionary);

    std::vector<PDFDictionary::DictionaryEntry> entries;
    entries.reserve(dictionary->getCount());

    for (size_t i = 0, count = dictionary->getCount(); i < count; ++i)
    {
        dictionary->getValue(i).accept(this);
        Q_ASSERT(!m_objectStack.empty());
        if (dictionary->getKey(i) != "Metadata")
        {
            entries.emplace_back(dictionary->getKey(i), m_objectStack.back());
        }
        else
        {
            ++*m_counter;
        }
        m_objectStack.pop_back();
    }

    m_objectStack.push_back(PDFObject::createDictionary(std::make_shared<PDFDictionary>(qMove(entries))));
}

PDFDocumentSanitizer::PDFDocumentSanitizer(SanitizationFlag flags, QObject* parent) :
    QObject(parent),
    m_flags(flags)
{

}

void PDFDocumentSanitizer::sanitize()
{
    Q_EMIT sanitizationStarted();

    if (m_flags.testFlag(DocumentInfo))
    {
        performSanitizeDocumentInfo();
    }

    if (m_flags.testFlag(Metadata))
    {
        performSanitizeMetadata();
    }

    if (m_flags.testFlag(Outline))
    {
        performSanitizeOutline();
    }

    if (m_flags.testFlag(FileAttachments))
    {
        performSanitizeFileAttachments();
    }

    if (m_flags.testFlag(EmbeddedSearchIndex))
    {
        performSanitizeEmbeddedSearchIndex();
    }

    if (m_flags.testFlag(MarkupAnnotations))
    {
        performSanitizeMarkupAnnotations();
    }

    if (m_flags.testFlag(PageThumbnails))
    {
        performSanitizePageThumbnails();
    }

    // Optimize - remove unused objects
    PDFOptimizer optimizer(PDFOptimizer::OptimizationFlags(PDFOptimizer::RemoveUnusedObjects | PDFOptimizer::ShrinkObjectStorage | PDFOptimizer::RemoveNullObjects), nullptr);
    optimizer.setStorage(m_storage);
    optimizer.optimize();
    m_storage = optimizer.takeStorage();

    Q_EMIT sanitizationFinished();
}

PDFDocumentSanitizer::SanitizationFlags PDFDocumentSanitizer::getFlags() const
{
    return m_flags;
}

void PDFDocumentSanitizer::setFlags(SanitizationFlags flags)
{
    m_flags = flags;
}

void PDFDocumentSanitizer::performSanitizeDocumentInfo()
{
    PDFObjectReference emptyDocumentInfoReference = m_storage.addObject(PDFObject());

    PDFDocumentBuilder builder(m_storage, PDFVersion(2, 0));
    const bool hasDocumentInfo = builder.getDocumentInfo().isValid();
    builder.setDocumentInfo(emptyDocumentInfoReference);
    PDFDocument document = builder.build();
    m_storage = document.getStorage();

    if (hasDocumentInfo)
    {
        Q_EMIT sanitizationProgress(tr("Document info was removed."));
    }
}

void PDFDocumentSanitizer::performSanitizeMetadata()
{
    std::atomic<PDFInteger> counter = 0;

    PDFObjectStorage::PDFObjects objects =  m_storage.getObjects();
    auto processEntry = [this, &counter](PDFObjectStorage::Entry& entry)
    {
        PDFRemoveMetadataVisitor visitor(&m_storage, &counter);
        entry.object.accept(&visitor);
        entry.object = visitor.getObject();
    };

    PDFExecutionPolicy::execute(PDFExecutionPolicy::Scope::Unknown, objects.begin(), objects.end(), processEntry);
    m_storage.setObjects(qMove(objects));
    Q_EMIT sanitizationProgress(tr("Metadata streams removed: %1").arg(counter));
}

void PDFDocumentSanitizer::performSanitizeOutline()
{
    PDFDocumentBuilder builder(m_storage, PDFVersion(2, 0));
    PDFObject catalogObject = builder.getObjectByReference(builder.getCatalogReference());
    const PDFDictionary* catalogDictionary = builder.getDictionaryFromObject(catalogObject);
    const bool hasOutline = catalogDictionary && catalogDictionary->hasKey("Outlines");

    if (hasOutline)
    {
        builder.removeOutline();
        PDFDocument document = builder.build();
        m_storage = document.getStorage();
        Q_EMIT sanitizationProgress(tr("Outline was removed."));
    }
}

void PDFDocumentSanitizer::performSanitizeFileAttachments()
{
    auto filter = [](const PDFAnnotation* annotation)
    {
        return annotation->getType() == AnnotationType::FileAttachment;
    };
    removeAnnotations(filter, tr("File attachments removed: %1."));

    // Remove files in name tree
    PDFDocumentBuilder builder(m_storage, PDFVersion(2, 0));
    PDFObject catalogObject = builder.getObjectByReference(builder.getCatalogReference());
    const PDFDictionary* catalogDictionary = builder.getDictionaryFromObject(catalogObject);
    const bool hasNames = catalogDictionary && catalogDictionary->hasKey("Names");

    if (hasNames)
    {
        PDFObject namesObject = builder.getObject(catalogDictionary->get("Names"));
        const PDFDictionary* namesDictionary = builder.getDictionaryFromObject(namesObject);
        if (namesDictionary->hasKey("EmbeddedFiles"))
        {
            PDFDictionary dictionaryCopy = *namesDictionary;
            dictionaryCopy.setEntry(PDFInplaceOrMemoryString("EmbeddedFiles"), PDFObject());
            namesObject = PDFObject::createDictionary(std::make_shared<PDFDictionary>(qMove(dictionaryCopy)));

            PDFObjectFactory factory;
            factory.beginDictionary();
            factory.beginDictionaryItem("Names");
            factory << namesObject;
            factory.endDictionaryItem();
            factory.endDictionary();
            PDFObject newCatalog = factory.takeObject();
            builder.mergeTo(builder.getCatalogReference(), std::move(newCatalog));
            PDFDocument document = builder.build();
            m_storage = document.getStorage();
            Q_EMIT sanitizationProgress(tr("Embedded files were removed."));
        }
    }
}

void PDFDocumentSanitizer::performSanitizeEmbeddedSearchIndex()
{
    PDFDocumentBuilder builder(m_storage, PDFVersion(2, 0));
    PDFObject catalogObject = builder.getObjectByReference(builder.getCatalogReference());
    const PDFDictionary* catalogDictionary = builder.getDictionaryFromObject(catalogObject);
    const bool hasPieceInfo = catalogDictionary && catalogDictionary->hasKey("PieceInfo");

    if (hasPieceInfo)
    {
        PDFObject pieceInfoObject = builder.getObject(catalogDictionary->get("PieceInfo"));
        const PDFDictionary* pieceInfoDictionary = builder.getDictionaryFromObject(pieceInfoObject);
        if (pieceInfoDictionary->hasKey("SearchIndex"))
        {
            PDFDictionary dictionaryCopy = *pieceInfoDictionary;
            dictionaryCopy.setEntry(PDFInplaceOrMemoryString("SearchIndex"), PDFObject());
            pieceInfoObject = PDFObject::createDictionary(std::make_shared<PDFDictionary>(qMove(dictionaryCopy)));

            PDFObjectFactory factory;
            factory.beginDictionary();
            factory.beginDictionaryItem("PieceInfo");
            factory << pieceInfoObject;
            factory.endDictionaryItem();
            factory.endDictionary();
            PDFObject newCatalog = factory.takeObject();
            builder.mergeTo(builder.getCatalogReference(), std::move(newCatalog));
            PDFDocument document = builder.build();
            m_storage = document.getStorage();
            Q_EMIT sanitizationProgress(tr("Search index was removed."));
        }
    }
}

void PDFDocumentSanitizer::performSanitizeMarkupAnnotations()
{
    auto filter = [](const PDFAnnotation* annotation)
    {
        return annotation->asMarkupAnnotation() != nullptr;
    };
    removeAnnotations(filter, tr("Markup annotations removed: %1."));
}

void PDFDocumentSanitizer::performSanitizePageThumbnails()
{
    PDFDocumentBuilder builder(m_storage, PDFVersion(2, 0));
    builder.flattenPageTree();
    std::vector<PDFObjectReference> pageReferences = builder.getPages();
    std::vector<PDFObjectReference> pagesWithThumbnail;

    for (const PDFObjectReference& pageReference : pageReferences)
    {
        const PDFDictionary* pageDictionary = builder.getDictionaryFromObject(builder.getObjectByReference(pageReference));
        if (pageDictionary && pageDictionary->hasKey("Thumb"))
        {
            pagesWithThumbnail.push_back(pageReference);
        }
    }

    if (!pagesWithThumbnail.empty())
    {
        for (const auto& pageReference : pagesWithThumbnail)
        {
            builder.removePageThumbnail(pageReference);
        }

        PDFDocument document = builder.build();
        m_storage = document.getStorage();
        Q_EMIT sanitizationProgress(tr("Page thumbnails removed: %1.").arg(pagesWithThumbnail.size()));
    }
}

void PDFDocumentSanitizer::removeAnnotations(const std::function<bool (const PDFAnnotation*)>& filter,
                                             QString message)
{
    PDFDocumentBuilder builder(m_storage, PDFVersion(2, 0));
    builder.flattenPageTree();
    std::vector<PDFObjectReference> pageReferences = builder.getPages();
    std::vector<std::pair<PDFObjectReference, PDFObjectReference>> annotationsToBeRemoved;

    PDFDocumentDataLoaderDecorator loader(&m_storage);
    for (const PDFObjectReference pageReference : pageReferences)
    {
        const PDFObject& pageObject = m_storage.getObjectByReference(pageReference);
        const PDFDictionary* pageDictionary = m_storage.getDictionaryFromObject(pageObject);

        if (!pageDictionary)
        {
            continue;
        }

        std::vector<PDFObjectReference> annotationReferences = loader.readReferenceArrayFromDictionary(pageDictionary, "Annots");
        for (const PDFObjectReference& annotationReference : annotationReferences)
        {
            PDFAnnotationPtr annotation = PDFAnnotation::parse(&m_storage, annotationReference);
            if (filter(annotation.get()))
            {
                annotationsToBeRemoved.emplace_back(pageReference, annotationReference);
            }
        }
    }

    if (!annotationsToBeRemoved.empty())
    {
        for (const auto& item : annotationsToBeRemoved)
        {
            const PDFObjectReference pageReference = item.first;
            const PDFObjectReference annotationReference = item.second;
            builder.removeAnnotation(pageReference, annotationReference);
        }

        PDFDocument document = builder.build();
        m_storage = document.getStorage();
        Q_EMIT sanitizationProgress(message.arg(annotationsToBeRemoved.size()));
    }
}

}   // namespace pdf
