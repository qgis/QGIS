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

#include "pdfoptimizer.h"
#include "pdfvisitor.h"
#include "pdfexecutionpolicy.h"
#include "pdfobjectutils.h"
#include "pdfutils.h"
#include "pdfconstants.h"
#include "pdfdocumentbuilder.h"
#include "pdfstreamfilters.h"
#include "pdfdbgheap.h"
#include "pdfdocumentwriter.h"

namespace pdf
{

class PDFRemoveSimpleObjectsVisitor : public PDFUpdateObjectVisitor
{
public:
    explicit inline PDFRemoveSimpleObjectsVisitor(const PDFObjectStorage* storage, std::atomic<PDFInteger>* counter) :
        PDFUpdateObjectVisitor(storage),
        m_counter(counter)
    {

    }

    virtual void visitReference(const PDFObjectReference reference) override;

private:
    std::atomic<PDFInteger>* m_counter;
};

void PDFRemoveSimpleObjectsVisitor::visitReference(const PDFObjectReference reference)
{
    PDFObject object = m_storage->getObjectByReference(reference);

    switch (object.getType())
    {
        case PDFObject::Type::Null:
        case PDFObject::Type::Bool:
        case PDFObject::Type::Int:
        case PDFObject::Type::Real:
        case PDFObject::Type::String:
        case PDFObject::Type::Name:
            ++*m_counter;
            m_objectStack.push_back(qMove(object));
            break;

        default:
            m_objectStack.push_back(PDFObject::createReference(reference));
            break;
    }
}

class PDFRemoveNullDictionaryEntriesVisitor : public PDFUpdateObjectVisitor
{
public:
    explicit PDFRemoveNullDictionaryEntriesVisitor(const PDFObjectStorage* storage, std::atomic<PDFInteger>* counter) :
        PDFUpdateObjectVisitor(storage),
        m_counter(counter)
    {

    }

    virtual void visitDictionary(const PDFDictionary* dictionary) override;

private:
    std::atomic<PDFInteger>* m_counter;
};

void PDFRemoveNullDictionaryEntriesVisitor::visitDictionary(const PDFDictionary* dictionary)
{
    Q_ASSERT(dictionary);

    std::vector<PDFDictionary::DictionaryEntry> entries;
    entries.reserve(dictionary->getCount());

    for (size_t i = 0, count = dictionary->getCount(); i < count; ++i)
    {
        dictionary->getValue(i).accept(this);
        Q_ASSERT(!m_objectStack.empty());
        if (!m_objectStack.back().isNull())
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

PDFOptimizer::PDFOptimizer(OptimizationFlags flags, QObject* parent) :
    QObject(parent),
    m_flags(flags)
{

}

void PDFOptimizer::optimize()
{
    // Jakub Melka: We divide optimization into stages, each
    // stage can consist from multiple passes.
    constexpr OptimizationFlags stages[] = { OptimizationFlags(DereferenceSimpleObjects),
                                             OptimizationFlags(RemoveNullObjects),
                                             OptimizationFlags(RemoveUnusedObjects | MergeIdenticalObjects),
                                             OptimizationFlags(ShrinkObjectStorage),
                                             OptimizationFlags(RecompressFlateStreams) };

    int stage = 1;

    Q_EMIT optimizationStarted();
    for (OptimizationFlags flags : stages)
    {
        Q_EMIT optimizationProgress(tr("Stage %1").arg(stage++));
        OptimizationFlags currentSteps = flags & m_flags;

        int passIndex = 1;

        bool pass = true;
        while (pass)
        {
            Q_EMIT optimizationProgress(tr("Pass %1").arg(passIndex++));
            pass = false;

            if (currentSteps.testFlag(DereferenceSimpleObjects))
            {
                pass = performDereferenceSimpleObjects() || pass;
            }
            if (currentSteps.testFlag(RemoveNullObjects))
            {
                pass = performRemoveNullObjects() || pass;
            }
            if (currentSteps.testFlag(RemoveUnusedObjects))
            {
                pass = performRemoveUnusedObjects() || pass;
            }
            if (currentSteps.testFlag(MergeIdenticalObjects))
            {
                pass = performMergeIdenticalObjects() || pass;
            }
            if (currentSteps.testFlag(ShrinkObjectStorage))
            {
                pass = performShrinkObjectStorage() || pass;
            }
            if (currentSteps.testFlag(RecompressFlateStreams))
            {
                pass = performRecompressFlateStreams() || pass;
            }
        }
    }
    Q_EMIT optimizationFinished();
}

PDFOptimizer::OptimizationFlags PDFOptimizer::getFlags() const
{
    return m_flags;
}

void PDFOptimizer::setFlags(OptimizationFlags flags)
{
    m_flags = flags;
}

bool PDFOptimizer::performDereferenceSimpleObjects()
{
    std::atomic<PDFInteger> counter = 0;

    PDFObjectStorage::PDFObjects objects =  m_storage.getObjects();
    auto processEntry = [this, &counter](PDFObjectStorage::Entry& entry)
    {
        PDFRemoveSimpleObjectsVisitor visitor(&m_storage, &counter);
        entry.object.accept(&visitor);
        entry.object = visitor.getObject();
    };

    PDFExecutionPolicy::execute(PDFExecutionPolicy::Scope::Unknown, objects.begin(), objects.end(), processEntry);
    m_storage.setObjects(qMove(objects));
    Q_EMIT optimizationProgress(tr("Simple objects dereferenced and embedded: %1").arg(counter));

    return false;
}

bool PDFOptimizer::performRemoveNullObjects()
{
    std::atomic<PDFInteger> counter = 0;

    PDFObjectStorage::PDFObjects objects =  m_storage.getObjects();
    auto processEntry = [this, &counter](PDFObjectStorage::Entry& entry)
    {
        PDFRemoveNullDictionaryEntriesVisitor visitor(&m_storage, &counter);
        entry.object.accept(&visitor);
        entry.object = visitor.getObject();
    };

    PDFExecutionPolicy::execute(PDFExecutionPolicy::Scope::Unknown, objects.begin(), objects.end(), processEntry);
    m_storage.setObjects(qMove(objects));
    Q_EMIT optimizationProgress(tr("Null objects entries from dictionaries removed: %1").arg(counter));

    return false;
}

bool PDFOptimizer::performRemoveUnusedObjects()
{
    std::atomic<PDFInteger> counter = 0;
    PDFObjectStorage::PDFObjects objects =  m_storage.getObjects();
    std::set<PDFObjectReference> references = PDFObjectUtils::getReferences({ m_storage.getTrailerDictionary() }, m_storage);

    PDFIntegerRange<size_t> range(0, objects.size());
    auto processEntry = [&counter, &objects, &references](size_t index)
    {
        PDFObjectStorage::Entry& entry = objects[index];
        PDFObjectReference reference(PDFInteger(index), entry.generation);
        if (!references.count(reference) && !entry.object.isNull())
        {
            entry.object = PDFObject();
            ++counter;
        }
    };

    PDFExecutionPolicy::execute(PDFExecutionPolicy::Scope::Unknown, range.begin(), range.end(), processEntry);
    m_storage.setObjects(qMove(objects));
    Q_EMIT optimizationProgress(tr("Unused objects removed: %1").arg(counter));

    return counter > 0;
}

bool PDFOptimizer::performMergeIdenticalObjects()
{
    std::atomic<PDFInteger> counter = 0;
    std::map<PDFObjectReference, PDFObjectReference> replacementMap;
    std::map<QByteArray, PDFObjectReference> serializedObjectToReference;
    PDFObjectStorage::PDFObjects objects =  m_storage.getObjects();
    std::vector<QByteArray> serializedObjects(objects.size());

    PDFIntegerRange<size_t> range(0, objects.size());
    auto serializeEntry = [&objects, &serializedObjects](size_t index)
    {
        const PDFObjectStorage::Entry& entry = objects[index];

        if (!entry.object.isNull())
        {
            serializedObjects[index] = PDFDocumentWriter::getSerializedObject(entry.object);
        }
    };
    PDFExecutionPolicy::execute(PDFExecutionPolicy::Scope::Unknown, range.begin(), range.end(), serializeEntry);

    // Find same object
    for (PDFInteger index : range)
    {
        const PDFObjectStorage::Entry& entry = objects[index];

        if (!entry.object.isNull())
        {
            // We do not merge special objects, such as pages
            if (const PDFDictionary* dictionary = m_storage.getDictionaryFromObject(entry.object))
            {
                PDFObject nameObject = m_storage.getObject(dictionary->get("Type"));
                if (nameObject.isName())
                {
                    if (nameObject.getString() == "Page")
                    {
                        continue;
                    }
                }
            }

            PDFObjectReference currentReference(PDFInteger(index), objects[index].generation);
            const QByteArray& serializedObject = serializedObjects[index];
            if (!serializedObjectToReference.count(serializedObject))
            {
                serializedObjectToReference[serializedObject] = currentReference;
            }
            else
            {
                PDFObjectReference oldReference = currentReference;
                PDFObjectReference newReference = serializedObjectToReference.at(serializedObject);
                replacementMap[oldReference] = newReference;
                ++counter;
            }
        }
    }

    // Replace objects
    if (!replacementMap.empty())
    {
        for (size_t i = 0; i < objects.size(); ++i)
        {
            objects[i].object = PDFObjectUtils::replaceReferences(objects[i].object, replacementMap);
        }
        PDFObject trailerDictionary = PDFObjectUtils::replaceReferences(m_storage.getTrailerDictionary(), replacementMap);
        m_storage.setTrailerDictionary(trailerDictionary);
    }

    m_storage.setObjects(qMove(objects));
    Q_EMIT optimizationProgress(tr("Identical objects merged: %1").arg(counter));

    return counter > 0;
}

bool PDFOptimizer::performShrinkObjectStorage()
{
    std::map<PDFObjectReference, PDFObjectReference> replacementMap;
    PDFObjectStorage::PDFObjects objects =  m_storage.getObjects();

    auto isFree = [](const PDFObjectStorage::Entry& entry)
    {
        return entry.object.isNull() && entry.generation < PDF_MAX_OBJECT_GENERATION;
    };
    auto isOccupied = [](const PDFObjectStorage::Entry& entry)
    {
        return !entry.object.isNull();
    };

    // Make list of free usable indices
    std::vector<size_t> freeIndices;
    freeIndices.reserve(objects.size() / 8);

    const size_t objectCount = objects.size();
    for (size_t sourceIndex = 1; sourceIndex < objectCount; ++sourceIndex)
    {
        if (isFree(objects[sourceIndex]))
        {
            freeIndices.push_back(sourceIndex);
        }
    }
    std::reverse(freeIndices.begin(), freeIndices.end());

    // Move objects to free entries
    for (size_t sourceIndex = objectCount - 1; sourceIndex > 0; --sourceIndex)
    {
        if (freeIndices.empty())
        {
            // Jakub Melka: We have run out of free indices
            break;
        }

        PDFObjectStorage::Entry& sourceEntry = objects[sourceIndex];
        if (isOccupied(sourceEntry))
        {
            size_t targetIndex = freeIndices.back();
            freeIndices.pop_back();

            if (targetIndex >= sourceIndex)
            {
                break;
            }

            PDFObjectStorage::Entry& targetEntry = objects[targetIndex];
            Q_ASSERT(isFree(targetEntry));

            ++targetEntry.generation;
            targetEntry.object = qMove(sourceEntry.object);
            sourceEntry.object = PDFObject();

            replacementMap[PDFObjectReference(PDFInteger(sourceIndex), sourceEntry.generation)] = PDFObjectReference(PDFInteger(targetIndex), targetEntry.generation);
        }
    }

    // Shrink objects array
    for (size_t sourceIndex = objectCount - 1; sourceIndex > 0; --sourceIndex)
    {
        if (isOccupied(objects[sourceIndex]))
        {
            objects.resize(sourceIndex + 1);
            break;
        }
    }

    // Update objects
    if (!replacementMap.empty())
    {
        for (size_t i = 0; i < objects.size(); ++i)
        {
            objects[i].object = PDFObjectUtils::replaceReferences(objects[i].object, replacementMap);
        }
        PDFObject trailerDictionary = PDFObjectUtils::replaceReferences(m_storage.getTrailerDictionary(), replacementMap);

        PDFObjectFactory factory;
        factory.beginDictionary();
        factory.beginDictionaryItem("Size");
        factory << PDFInteger(objects.size());
        factory.endDictionaryItem();
        factory.endDictionary();

        trailerDictionary = PDFObjectManipulator::merge(trailerDictionary, factory.takeObject(), PDFObjectManipulator::NoFlag);
        m_storage.setTrailerDictionary(trailerDictionary);
    }

    const size_t newObjectCount = objects.size();
    m_storage.setObjects(qMove(objects));
    Q_EMIT optimizationProgress(tr("Object list shrinked by: %1").arg(objectCount - newObjectCount));

    return false;
}

bool PDFOptimizer::performRecompressFlateStreams()
{
    std::atomic<PDFInteger> bytesSaved = 0;

    PDFObjectStorage::PDFObjects objects =  m_storage.getObjects();
    auto processEntry = [this, &bytesSaved](PDFObjectStorage::Entry& entry)
    {
        if (entry.object.isStream())
        {
            const PDFStream* stream = entry.object.getStream();
            const PDFDictionary* dictionary = stream->getDictionary();

            if (dictionary->hasKey("F"))
            {
                // External file stream, we do not recompress it
                return;
            }

            PDFStreamFilterStorage::StreamFilters streamFilters = PDFStreamFilterStorage::getStreamFilters(stream, std::bind(QOverload<const PDFObject&>::of(&PDFObjectStorage::getObject), &m_storage, std::placeholders::_1));

            if (streamFilters.filterObjects.empty())
            {
                // No filters
                return;
            }

            const PDFStreamFilter* streamFilter = streamFilters.filterObjects.back();
            if (dynamic_cast<const PDFFlateDecodeFilter*>(streamFilter))
            {
                // Try to recompress. If we end with less data, then we use recompressed stream
                QByteArray recompressedData = PDFFlateDecodeFilter::recompress(*stream->getContent());
                const PDFInteger currentBytesSaved = stream->getContent()->size() - recompressedData.size();
                if (currentBytesSaved > 0)
                {
                    bytesSaved += currentBytesSaved;
                    PDFDictionary updatedDictionary = *dictionary;
                    updatedDictionary.setEntry(PDFInplaceOrMemoryString("Length"), PDFObject::createInteger(recompressedData.size()));
                    entry.object = PDFObject::createStream(std::make_shared<PDFStream>(qMove(updatedDictionary), qMove(recompressedData)));
                }
            }
        }
    };

    PDFExecutionPolicy::execute(PDFExecutionPolicy::Scope::Unknown, objects.begin(), objects.end(), processEntry);
    m_storage.setObjects(qMove(objects));
    Q_EMIT optimizationProgress(tr("Bytes saved by recompressing stream: %1").arg(bytesSaved));

    return false;
}

}   // namespace pdf
