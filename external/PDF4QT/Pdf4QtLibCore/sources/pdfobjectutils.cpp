//    Copyright (C) 2020-2022 Jakub Melka
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

#include "pdfobjectutils.h"
#include "pdfvisitor.h"
#include "pdfexecutionpolicy.h"
#include "pdfdocumentwriter.h"
#include "pdfdbgheap.h"

namespace pdf
{

class PDFCollectReferencesVisitor : public PDFAbstractVisitor
{
public:
    explicit PDFCollectReferencesVisitor(std::set<PDFObjectReference>& references) :
        m_references(references)
    {

    }

    virtual void visitArray(const PDFArray* array) override;
    virtual void visitDictionary(const PDFDictionary* dictionary) override;
    virtual void visitStream(const PDFStream* stream) override;
    virtual void visitReference(const PDFObjectReference reference) override;

private:
    std::set<PDFObjectReference>& m_references;
};

void PDFCollectReferencesVisitor::visitArray(const PDFArray* array)
{
    acceptArray(array);
}

void PDFCollectReferencesVisitor::visitDictionary(const PDFDictionary* dictionary)
{
    acceptDictionary(dictionary);
}

void PDFCollectReferencesVisitor::visitStream(const PDFStream* stream)
{
    acceptStream(stream);
}

void PDFCollectReferencesVisitor::visitReference(const PDFObjectReference reference)
{
    m_references.insert(reference);
}

class PDFReplaceReferencesVisitor : public PDFAbstractVisitor
{
public:
    explicit PDFReplaceReferencesVisitor(const std::map<PDFObjectReference, PDFObjectReference>& replacements) :
        m_replacements(replacements)
    {
        m_objectStack.reserve(32);
    }

    virtual void visitNull() override;
    virtual void visitBool(bool value) override;
    virtual void visitInt(PDFInteger value) override;
    virtual void visitReal(PDFReal value) override;
    virtual void visitString(PDFStringRef string) override;
    virtual void visitName(PDFStringRef name) override;
    virtual void visitArray(const PDFArray* array) override;
    virtual void visitDictionary(const PDFDictionary* dictionary) override;
    virtual void visitStream(const PDFStream* stream) override;
    virtual void visitReference(const PDFObjectReference reference) override;

    PDFObject getObject();

private:
    const std::map<PDFObjectReference, PDFObjectReference>& m_replacements;
    std::vector<PDFObject> m_objectStack;
};

void PDFReplaceReferencesVisitor::visitNull()
{
    m_objectStack.push_back(PDFObject::createNull());
}

void PDFReplaceReferencesVisitor::visitBool(bool value)
{
    m_objectStack.push_back(PDFObject::createBool(value));
}

void PDFReplaceReferencesVisitor::visitInt(PDFInteger value)
{
    m_objectStack.push_back(PDFObject::createInteger(value));
}

void PDFReplaceReferencesVisitor::visitReal(PDFReal value)
{
    m_objectStack.push_back(PDFObject::createReal(value));
}

void PDFReplaceReferencesVisitor::visitString(PDFStringRef string)
{
    m_objectStack.push_back(PDFObject::createString(string));
}

void PDFReplaceReferencesVisitor::visitName(PDFStringRef name)
{
    m_objectStack.push_back(PDFObject::createName(name));
}

void PDFReplaceReferencesVisitor::visitArray(const PDFArray* array)
{
    acceptArray(array);

    // We have all objects on the stack
    Q_ASSERT(array->getCount() <= m_objectStack.size());

    auto it = std::next(m_objectStack.cbegin(), m_objectStack.size() - array->getCount());
    std::vector<PDFObject> objects(it, m_objectStack.cend());
    PDFObject object = PDFObject::createArray(std::make_shared<PDFArray>(qMove(objects)));
    m_objectStack.erase(it, m_objectStack.cend());
    m_objectStack.push_back(object);
}

void PDFReplaceReferencesVisitor::visitDictionary(const PDFDictionary* dictionary)
{
    Q_ASSERT(dictionary);

    std::vector<PDFDictionary::DictionaryEntry> entries;
    entries.reserve(dictionary->getCount());

    for (size_t i = 0, count = dictionary->getCount(); i < count; ++i)
    {
        dictionary->getValue(i).accept(this);
        entries.emplace_back(dictionary->getKey(i), m_objectStack.back());
        m_objectStack.pop_back();
    }

    m_objectStack.push_back(PDFObject::createDictionary(std::make_shared<PDFDictionary>(qMove(entries))));
}

void PDFReplaceReferencesVisitor::visitStream(const PDFStream* stream)
{
    // Replace references in the dictionary
    visitDictionary(stream->getDictionary());
    PDFObject dictionaryObject = m_objectStack.back();
    m_objectStack.pop_back();
    m_objectStack.push_back(PDFObject::createStream(std::make_shared<PDFStream>(PDFDictionary(*dictionaryObject.getDictionary()), QByteArray(*stream->getContent()))));
}

void PDFReplaceReferencesVisitor::visitReference(const PDFObjectReference reference)
{
    auto it = m_replacements.find(reference);
    if (it != m_replacements.cend())
    {
        // Replace the reference
        m_objectStack.push_back(PDFObject::createReference(it->second));
    }
    else
    {
        // Preserve old reference
        m_objectStack.push_back(PDFObject::createReference(reference));
    }
}

PDFObject PDFReplaceReferencesVisitor::getObject()
{
    Q_ASSERT(m_objectStack.size() == 1);
    return qMove(m_objectStack.back());
}

std::set<PDFObjectReference> PDFObjectUtils::getReferences(const std::vector<PDFObject>& objects, const PDFObjectStorage& storage)
{
    std::set<PDFObjectReference> references;
    {
        PDFCollectReferencesVisitor collectReferencesVisitor(references);
        for (const PDFObject& object : objects)
        {
            object.accept(&collectReferencesVisitor);
        }
    }

    // Iterative algorihm, which adds additional references from referenced objects.
    // If new reference is added, then we must also check, that all referenced objects
    // from this object are added.
    std::set<PDFObjectReference> workSet = references;
    while (!workSet.empty())
    {
        std::set<PDFObjectReference> addedReferences;
        PDFCollectReferencesVisitor collectReferencesVisitor(addedReferences);
        for (const PDFObjectReference& objectReference : workSet)
        {
            storage.getObject(objectReference).accept(&collectReferencesVisitor);
        }

        workSet.clear();
        std::set_difference(addedReferences.cbegin(), addedReferences.cend(), references.cbegin(), references.cend(), std::inserter(workSet, workSet.cend()));
        references.merge(addedReferences);
    }

    return references;
}

std::set<PDFObjectReference> PDFObjectUtils::getDirectReferences(const PDFObject& object)
{
    std::set<PDFObjectReference> references;

    PDFCollectReferencesVisitor collectReferencesVisitor(references);
    object.accept(&collectReferencesVisitor);

    return references;
}

PDFObject PDFObjectUtils::replaceReferences(const PDFObject& object, const std::map<PDFObjectReference, PDFObjectReference>& referenceMapping)
{
    PDFReplaceReferencesVisitor replaceReferencesVisitor(referenceMapping);
    object.accept(&replaceReferencesVisitor);
    return replaceReferencesVisitor.getObject();
}

QString PDFObjectUtils::getObjectTypeName(PDFObject::Type type)
{
    switch (type)
    {
        case pdf::PDFObject::Type::Null:
            return PDFTranslationContext::tr("Null");

        case pdf::PDFObject::Type::Bool:
            return PDFTranslationContext::tr("Boolean");

        case pdf::PDFObject::Type::Int:
            return PDFTranslationContext::tr("Integer");

        case pdf::PDFObject::Type::Real:
            return PDFTranslationContext::tr("Real");

        case pdf::PDFObject::Type::String:
            return PDFTranslationContext::tr("String");

        case pdf::PDFObject::Type::Name:
            return PDFTranslationContext::tr("Name");

        case pdf::PDFObject::Type::Array:
            return PDFTranslationContext::tr("Array");

        case pdf::PDFObject::Type::Dictionary:
            return PDFTranslationContext::tr("Dictionary");

        case pdf::PDFObject::Type::Stream:
            return PDFTranslationContext::tr("Stream");

        case pdf::PDFObject::Type::Reference:
            return PDFTranslationContext::tr("Reference");

        default:
            Q_ASSERT(false);
            break;
    }

    return QString();
}

void PDFObjectClassifier::classify(const PDFDocument* document)
{
    // Clear old classification, if it exist
    m_classification.clear();
    m_allTypesUsed = None;

    if (!document)
    {
        return;
    }

    PDFDocumentDataLoaderDecorator loader(document);
    const PDFObjectStorage& storage = document->getStorage();
    const PDFObjectStorage::PDFObjects& objects = storage.getObjects();

    m_classification.resize(objects.size(), Classification());
    for (size_t i = 0; i < objects.size(); ++i)
    {
        PDFObjectReference reference(i, objects[i].generation);
        m_classification[i].reference = reference;
    }

    // First, iterate trough pages of the document
    const PDFCatalog* catalog = document->getCatalog();
    const size_t pageCount = catalog->getPageCount();
    for (size_t i = 0; i < pageCount; ++i)
    {
        const PDFPage* page = catalog->getPage(i);

        if (!page)
        {
            continue;
        }

        // Handle page itself
        if (hasObject(page->getPageReference()))
        {
            mark(page->getPageReference(), Page);
        }

        // Handle annotations
        for (const PDFObjectReference& reference : page->getAnnotations())
        {
            if (hasObject(reference))
            {
                mark(reference, Annotation);
            }
        }

        // Handle contents
        PDFObject pageObject = document->getObjectByReference(page->getPageReference());
        Q_ASSERT(pageObject.isDictionary());

        const PDFDictionary* dictionary = pageObject.getDictionary();
        const PDFObject& contentsObject = dictionary->get("Contents");
        if (contentsObject.isReference())
        {
            mark(contentsObject.getReference(), ContentStream);
        }

        // Handle resources
        if (const PDFDictionary* resourcesDictionary = document->getDictionaryFromObject(dictionary->get("Resources")))
        {
            markDictionary(document, resourcesDictionary->get("ExtGState"), GraphicState);
            markDictionary(document, resourcesDictionary->get("ColorSpace"), ColorSpace);
            markDictionary(document, resourcesDictionary->get("Pattern"), Pattern);
            markDictionary(document, resourcesDictionary->get("Shading"), Shading);
            markDictionary(document, resourcesDictionary->get("Font"), Font);

            if (const PDFDictionary* xobjectDictionary = document->getDictionaryFromObject(resourcesDictionary->get("XObject")))
            {
                const size_t count = xobjectDictionary->getCount();
                for (size_t xObjectIndex = 0; xObjectIndex < count; ++xObjectIndex)
                {
                    const PDFObject& item = xobjectDictionary->getValue(xObjectIndex);
                    if (item.isReference() && hasObject(item.getReference()))
                    {
                        if (const PDFDictionary* xobjectItemDictionary = document->getDictionaryFromObject(item))
                        {
                            QByteArray subtype = loader.readNameFromDictionary(xobjectItemDictionary, "Subtype");

                            if (subtype == "Image")
                            {
                                mark(item.getReference(), Image);
                            }
                            else if (subtype == "Form")
                            {
                                mark(item.getReference(), Form);
                            }
                        }
                    }
                }
            }
        }
    }

    for (Classification& classification : m_classification)
    {
        if (const PDFDictionary* dictionary = document->getDictionaryFromObject(document->getObjectByReference(classification.reference)))
        {
            QByteArray typeName = loader.readNameFromDictionary(dictionary, "Type");
            if (typeName == "Action")
            {
                classification.types.setFlag(Action);
            }
        }
    }

    for (const Classification& classification : m_classification)
    {
        m_allTypesUsed |= classification.types;
    }
}

bool PDFObjectClassifier::hasObject(PDFObjectReference reference) const
{
    return reference.isValid() &&
           reference.objectNumber < PDFInteger(m_classification.size()) &&
            m_classification[reference.objectNumber].reference == reference;
}

std::vector<PDFObjectReference> PDFObjectClassifier::getObjectsByType(Type type) const
{
    std::vector<PDFObjectReference> result;

    for (const Classification& classification : m_classification)
    {
        if (classification.types.testFlag(type))
        {
            result.push_back(classification.reference);
        }
    }

    return result;
}

PDFObjectClassifier::Statistics PDFObjectClassifier::calculateStatistics(const PDFDocument* document) const
{
    Statistics result;

    // Jakub Melka: prepare statistics map
    result.statistics[None];

    for (uint i = 0; i < 32; ++i)
    {
        uint32_t mask = 1 << i;
        if (m_allTypesUsed & mask)
        {
            result.statistics[Type(mask)];
        }
    }

    auto processEntry = [document, &result](const Classification& entry)
    {
        const PDFObject& object = document->getObjectByReference(entry.reference);

        if (object.isNull())
        {
            return;
        }

        Type type = Type(uint32_t(entry.types));
        if (!result.statistics.count(type))
        {
            type = None;
        }

        Q_ASSERT(result.statistics.count(type));

        const qint64 objectSize = PDFDocumentWriter::getObjectSize(document, entry.reference);

        StatisticsItem& statisticsItem = result.statistics.at(type);
        statisticsItem.count.fetch_add(1);
        statisticsItem.bytes.fetch_add(objectSize);
    };

    PDFExecutionPolicy::execute(PDFExecutionPolicy::Scope::Unknown, m_classification.cbegin(), m_classification.cend(), processEntry);

    PDFStatisticsCollector collector;
    PDFApplyVisitor(*document, &collector);

    for (PDFObject::Type objectType : PDFObject::getTypes())
    {
        result.objectCountByType[size_t(objectType)] = collector.getObjectCount(objectType);
    }

    return result;
}

void PDFObjectClassifier::mark(PDFObjectReference reference, Type type)
{
    Q_ASSERT(hasObject(reference));
    m_classification[reference.objectNumber].types.setFlag(type, true);
}

void PDFObjectClassifier::markDictionary(const PDFDocument* document, PDFObject object, Type type)
{
    if (const PDFDictionary* dictionary = document->getDictionaryFromObject(object))
    {
        const size_t count = dictionary->getCount();
        for (size_t i = 0; i < count; ++i)
        {
            const PDFObject& item = dictionary->getValue(i);
            if (item.isReference() && hasObject(item.getReference()))
            {
                mark(item.getReference(), type);
            }
        }
    }
}

}   // namespace pdf
