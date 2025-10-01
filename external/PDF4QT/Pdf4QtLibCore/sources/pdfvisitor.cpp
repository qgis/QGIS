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

#include "pdfvisitor.h"
#include "pdfdbgheap.h"

namespace pdf
{

void PDFAbstractVisitor::acceptArray(const PDFArray* array)
{
    Q_ASSERT(array);

    for (size_t i = 0, count = array->getCount(); i < count; ++i)
    {
        array->getItem(i).accept(this);
    }
}

void PDFAbstractVisitor::acceptDictionary(const PDFDictionary* dictionary)
{
    Q_ASSERT(dictionary);

    for (size_t i = 0, count = dictionary->getCount(); i < count; ++i)
    {
        dictionary->getValue(i).accept(this);
    }
}

void PDFAbstractVisitor::acceptStream(const PDFStream* stream)
{
    Q_ASSERT(stream);

    acceptDictionary(stream->getDictionary());
}

PDFStatisticsCollector::PDFStatisticsCollector()
{

}

void PDFStatisticsCollector::visitNull()
{
    collectStatisticsOfSimpleObject(PDFObject::Type::Null);
}

void PDFStatisticsCollector::visitBool(bool value)
{
    Q_UNUSED(value);
    collectStatisticsOfSimpleObject(PDFObject::Type::Bool);
}

void PDFStatisticsCollector::visitInt(PDFInteger value)
{
    Q_UNUSED(value);
     collectStatisticsOfSimpleObject(PDFObject::Type::Int);
}

void PDFStatisticsCollector::visitReal(PDFReal value)
{
    Q_UNUSED(value);
    collectStatisticsOfSimpleObject(PDFObject::Type::Real);
}

void PDFStatisticsCollector::visitString(PDFStringRef string)
{
    Statistics& statistics = m_statistics[size_t(PDFObject::Type::String)];
    if (string.inplaceString)
    {
        collectStatisticsOfSimpleObject(PDFObject::Type::String);
    }
    else
    {
        collectStatisticsOfString(string.memoryString, statistics);
    }
}

void PDFStatisticsCollector::visitName(PDFStringRef name)
{
    Statistics& statistics = m_statistics[size_t(PDFObject::Type::Name)];
    if (name.inplaceString)
    {
        collectStatisticsOfSimpleObject(PDFObject::Type::Name);
    }
    else
    {
        collectStatisticsOfString(name.memoryString, statistics);
    }
}

void PDFStatisticsCollector::visitArray(const PDFArray* array)
{
    Statistics& statistics = m_statistics[size_t(PDFObject::Type::Array)];
    statistics.count += 1;
    statistics.memoryConsumptionEstimate += sizeof(PDFObject) + sizeof(PDFArray);

    // We process elements of the array, together with memory consumption,
    // in the call of acceptArray function. No need to calculate memory consumption here.
    // Just calculate the overhead.
    statistics.memoryOverheadEstimate += (array->getCapacity() - array->getCount()) * sizeof(PDFObject);

    acceptArray(array);
}

void PDFStatisticsCollector::visitDictionary(const PDFDictionary* dictionary)
{
    Statistics& statistics = m_statistics[size_t(PDFObject::Type::Dictionary)];
    collectStatisticsOfDictionary(statistics, dictionary);

    acceptDictionary(dictionary);
}

void PDFStatisticsCollector::visitStream(const PDFStream* stream)
{
    Statistics& statistics = m_statistics[size_t(PDFObject::Type::Stream)];
    collectStatisticsOfDictionary(statistics, stream->getDictionary());

    const QByteArray& byteArray = *stream->getContent();
    const uint64_t memoryConsumption = byteArray.size() * sizeof(char);
    const uint64_t memoryOverhead = (byteArray.capacity() - byteArray.size()) * sizeof(char);

    statistics.memoryConsumptionEstimate += memoryConsumption;
    statistics.memoryOverheadEstimate += memoryOverhead;

    acceptStream(stream);
}

void PDFStatisticsCollector::visitReference(const PDFObjectReference reference)
{
    Q_UNUSED(reference);
    collectStatisticsOfSimpleObject(PDFObject::Type::Reference);
}

void PDFStatisticsCollector::collectStatisticsOfDictionary(Statistics& statistics, const PDFDictionary* dictionary)
{
    statistics.count += 1;
    statistics.memoryConsumptionEstimate += sizeof(PDFObject) + sizeof(PDFDictionary);

    constexpr uint64_t sizeOfItem = sizeof(std::pair<QByteArray, PDFObject>);
    constexpr uint64_t sizeOfItemWithoutObject = sizeOfItem - sizeof(PDFObject);

    uint64_t consumptionEstimate = sizeOfItemWithoutObject * dictionary->getCount();
    uint64_t overheadEstimate = sizeOfItem * (dictionary->getCapacity() - dictionary->getCount());

    for (size_t i = 0, count = dictionary->getCount(); i < count; ++i)
    {
        if (!dictionary->getKey(i).isInplace())
        {
            QByteArray key = dictionary->getKey(i).getString();

            consumptionEstimate += key.size() * sizeof(char);
            overheadEstimate += (key.capacity() - key.size()) * sizeof(char);
        }
    }

    statistics.memoryConsumptionEstimate += consumptionEstimate;
    statistics.memoryOverheadEstimate += overheadEstimate;
}

void PDFStatisticsCollector::collectStatisticsOfString(const PDFString* string, Statistics& statistics)
{
    statistics.count += 1;
    statistics.memoryConsumptionEstimate += sizeof(PDFObject) + sizeof(PDFString);

    const QByteArray& byteArray = string->getString();
    const uint64_t memoryConsumption = byteArray.size() * sizeof(char);
    const uint64_t memoryOverhead = (byteArray.capacity() - byteArray.size()) * sizeof(char);

    statistics.memoryConsumptionEstimate += memoryConsumption;
    statistics.memoryOverheadEstimate += memoryOverhead;
}

void PDFStatisticsCollector::collectStatisticsOfSimpleObject(PDFObject::Type type)
{
    Statistics& statistics = m_statistics[size_t(type)];
    statistics.count += 1;
    statistics.memoryConsumptionEstimate += sizeof(PDFObject);
}

void PDFUpdateObjectVisitor::visitNull()
{
    m_objectStack.push_back(PDFObject::createNull());
}

void PDFUpdateObjectVisitor::visitBool(bool value)
{
    m_objectStack.push_back(PDFObject::createBool(value));
}

void PDFUpdateObjectVisitor::visitInt(PDFInteger value)
{
    m_objectStack.push_back(PDFObject::createInteger(value));
}

void PDFUpdateObjectVisitor::visitReal(PDFReal value)
{
    m_objectStack.push_back(PDFObject::createReal(value));
}

void PDFUpdateObjectVisitor::visitString(PDFStringRef string)
{
    m_objectStack.push_back(PDFObject::createString(string));
}

void PDFUpdateObjectVisitor::visitName(PDFStringRef name)
{
    m_objectStack.push_back(PDFObject::createName(name));
}

void PDFUpdateObjectVisitor::visitArray(const PDFArray* array)
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

void PDFUpdateObjectVisitor::visitDictionary(const PDFDictionary* dictionary)
{
    Q_ASSERT(dictionary);

    std::vector<PDFDictionary::DictionaryEntry> entries;
    entries.reserve(dictionary->getCount());

    for (size_t i = 0, count = dictionary->getCount(); i < count; ++i)
    {
        dictionary->getValue(i).accept(this);
        Q_ASSERT(!m_objectStack.empty());
        entries.emplace_back(dictionary->getKey(i), m_objectStack.back());
        m_objectStack.pop_back();
    }

    m_objectStack.push_back(PDFObject::createDictionary(std::make_shared<PDFDictionary>(qMove(entries))));
}

void PDFUpdateObjectVisitor::visitStream(const PDFStream* stream)
{
    const PDFDictionary* dictionary = stream->getDictionary();

    visitDictionary(dictionary);

    Q_ASSERT(!m_objectStack.empty());
    PDFObject dictionaryObject = m_objectStack.back();
    m_objectStack.pop_back();

    PDFDictionary newDictionary(*dictionaryObject.getDictionary());
    m_objectStack.push_back(PDFObject::createStream(std::make_shared<PDFStream>(qMove(newDictionary), QByteArray(*stream->getContent()))));
}

void PDFUpdateObjectVisitor::visitReference(const PDFObjectReference reference)
{
    m_objectStack.push_back(PDFObject::createReference(reference));
}

PDFObject PDFUpdateObjectVisitor::getObject()
{
    Q_ASSERT(m_objectStack.size() == 1);
    return qMove(m_objectStack.back());
}

}   // namespace pdf
