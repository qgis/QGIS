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

#ifndef PDFVISITOR_H
#define PDFVISITOR_H

#include "pdfglobal.h"
#include "pdfobject.h"
#include "pdfdocument.h"

#include <QMutex>

#include <map>
#include <atomic>
#include <execution>

namespace pdf
{

/// Abstract visitor, can iterate trough object tree
class PDF4QTLIBCORESHARED_EXPORT PDFAbstractVisitor
{
public:

    /// This enum identifies how the tree of objects should be processed.
    /// Some visitors allow parallel (concurrent) run, some visitors are not
    /// thread safe, so they run in single thread. Please select strategy
    /// of your visitor carefully to avoid race conditions.
    enum class Strategy
    {
        /// Visitor with this strategy allows fully parallel run across multiple threads.
        /// So single instance of visitor is used and it is assumed, that all functions
        /// are thread safe. This means full parallelization.
        Parallel,

        /// Merging paraller strategy. For each thread, instance of visitor is created, and
        /// run on object tree. Then they are merged by the \p merge function. Strategy assumes,
        /// that \p merge function is not reentrant, and it is not run by multiple threads
        /// on same object. Locking is not needed in this function.
        Merging,

        /// Do not allow parallel run, use single instance and single thread run. This can
        /// be slower than previous two strategies, but allows visitor not to be thread
        /// safe. Use this strategy on visitors, which can't be thread safe from principle.
        Sequential
    };

    explicit constexpr PDFAbstractVisitor() = default;
    virtual ~PDFAbstractVisitor() = default;

    virtual void visitNull() { }
    virtual void visitBool(bool value) { Q_UNUSED(value); }
    virtual void visitInt(PDFInteger value) { Q_UNUSED(value); }
    virtual void visitReal(PDFReal value) { Q_UNUSED(value); }
    virtual void visitString(PDFStringRef string) { Q_UNUSED(string); }
    virtual void visitName(PDFStringRef name) { Q_UNUSED(name); }
    virtual void visitArray(const PDFArray* array) { Q_UNUSED(array); }
    virtual void visitDictionary(const PDFDictionary* dictionary) { Q_UNUSED(dictionary); }
    virtual void visitStream(const PDFStream* stream) { Q_UNUSED(stream); }
    virtual void visitReference(const PDFObjectReference reference) { Q_UNUSED(reference); }

protected:
    void acceptArray(const PDFArray* array);
    void acceptDictionary(const PDFDictionary* dictionary);
    void acceptStream(const PDFStream* stream);
};

/// Statistics visitor, collects statistics about PDF object, can be
/// invoked from multiple threads.
class PDF4QTLIBCORESHARED_EXPORT PDFStatisticsCollector : public PDFAbstractVisitor
{
public:
    explicit PDFStatisticsCollector();
    virtual ~PDFStatisticsCollector() override = default;

    /// We implement this visitor as fully parallel
    static constexpr const Strategy VisitorStrategy = Strategy::Parallel;

    struct Statistics
    {
        inline constexpr Statistics() :
            count(0),
            memoryConsumptionEstimate(0),
            memoryOverheadEstimate(0)
        {

        }

        /// This constructor must not be used while \p other is write-accessed from another thread.
        /// We use relaxed memory order, because we assume this constructor is used only while
        /// copying data, not when statistics is collected.
        /// \param other Object, from which data should be copied
        inline Statistics(const Statistics& other) :
            count(other.count.load(std::memory_order_relaxed)),
            memoryConsumptionEstimate(other.memoryConsumptionEstimate.load(std::memory_order_relaxed)),
            memoryOverheadEstimate(other.memoryOverheadEstimate.load(std::memory_order_relaxed))
        {

        }

        std::atomic<qint64> count = 0;
        std::atomic<qint64> memoryConsumptionEstimate = 0;
        std::atomic<qint64> memoryOverheadEstimate = 0;
    };

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

    qint64 getObjectCount(PDFObject::Type type) const { return m_statistics[size_t(type)].count; }

private:
    void collectStatisticsOfSimpleObject(PDFObject::Type type);
    void collectStatisticsOfString(const PDFString* string, Statistics& statistics);
    void collectStatisticsOfDictionary(Statistics& statistics, const PDFDictionary* dictionary);

    std::array<Statistics, size_t(PDFObject::Type::LastType)> m_statistics = { };
};

template<typename Visitor, PDFAbstractVisitor::Strategy strategy>
struct PDFApplyVisitorImpl;

template<typename Visitor>
void PDFApplyVisitor(const PDFDocument& document, Visitor* visitor)
{
    PDFApplyVisitorImpl<Visitor, Visitor::VisitorStrategy>::apply(document, visitor);
}

template<typename Visitor, PDFAbstractVisitor::Strategy strategy>
struct PDFApplyVisitorImpl
{
    // Do nothing, no strategy defined
};

template<typename Visitor>
struct PDFApplyVisitorImpl<Visitor, PDFAbstractVisitor::Strategy::Parallel>
{
    static void apply(const PDFDocument& document, Visitor* visitor)
    {
        const PDFObjectStorage& storage = document.getStorage();
        const PDFObjectStorage::PDFObjects& objects = storage.getObjects();
        const PDFObject& trailerDictionary = storage.getTrailerDictionary();

        std::for_each(std::execution::par, objects.cbegin(), objects.cend(), [visitor](const PDFObjectStorage::Entry& entry) { entry.object.accept(visitor); });
        trailerDictionary.accept(visitor);
    }
};

template<typename Visitor>
struct PDFApplyVisitorImpl<Visitor, PDFAbstractVisitor::Strategy::Merging>
{
    static void apply(const PDFDocument& document, Visitor* visitor)
    {
        const PDFObjectStorage& storage = document.getStorage();
        const PDFObjectStorage::PDFObjects& objects = storage.getObjects();
        const PDFObject& trailerDictionary = storage.getTrailerDictionary();

        QMutex mutex;

        auto process = [&mutex, visitor](const PDFObjectStorage::Entry& entry)
        {
            Visitor localVisitor;
            entry.object.accept(&localVisitor);

            QMutexLocker lock(&mutex);
            visitor->merge(&localVisitor);
        };

        std::for_each(std::execution::par, objects.cbegin(), objects.cend(), process);
        trailerDictionary.accept(visitor);
    }
};

template<typename Visitor>
struct PDFApplyVisitorImpl<Visitor, PDFAbstractVisitor::Strategy::Sequential>
{
    static void apply(const PDFDocument& document, Visitor* visitor)
    {
        const PDFObjectStorage& storage = document.getStorage();
        const PDFObjectStorage::PDFObjects& objects = storage.getObjects();
        const PDFObject& trailerDictionary = storage.getTrailerDictionary();

        std::for_each(std::execution::seq, objects.cbegin(), objects.cend(), [visitor](const PDFObjectStorage::Entry& entry) { entry.object.accept(visitor); });
        trailerDictionary.accept(visitor);
    }
};

class PDFUpdateObjectVisitor : public PDFAbstractVisitor
{
public:
    explicit inline PDFUpdateObjectVisitor(const PDFObjectStorage* storage) :
        m_storage(storage)
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

protected:
    const PDFObjectStorage* m_storage;
    std::vector<PDFObject> m_objectStack;
};

}   // namespace pdf

#endif // PDFVISITOR_H
