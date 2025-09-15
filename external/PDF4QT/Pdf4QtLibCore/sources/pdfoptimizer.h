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

#ifndef PDFOPTIMIZER_H
#define PDFOPTIMIZER_H

#include "pdfdocument.h"

#include <QObject>

namespace pdf
{

/// Class for optimalizing documents. Can examine object structure and it's dependencies,
/// and remove unused objects, merge same objects, or even recompress some streams
/// to achieve better optimization ratio. Optimization is configurable, user can specify,
/// which optimization steps should be done and which not.
class PDF4QTLIBCORESHARED_EXPORT PDFOptimizer : public QObject
{
    Q_OBJECT

public:

    enum OptimizationFlag
    {
        None                        = 0x0000, ///< No optimization is performed
        DereferenceSimpleObjects    = 0x0001, ///< If simple objects are referenced (such as integers, bools, etc.), then they are dereferenced
        RemoveNullObjects           = 0x0002, ///< Remove null objects from dictionary entries
        RemoveUnusedObjects         = 0x0004, ///< Remove not referenced objects
        MergeIdenticalObjects       = 0x0008, ///< Merge identical objects
        ShrinkObjectStorage         = 0x0010, ///< Shrink object storage, so unused objects are filled with used (and generation number increased)
        RecompressFlateStreams      = 0x0020, ///< Flate streams are recompressed with maximal compression
        All                         = 0xFFFF, ///< All optimizations turned on
    };
    Q_DECLARE_FLAGS(OptimizationFlags, OptimizationFlag)

    explicit PDFOptimizer(OptimizationFlags flags, QObject* parent);

    /// Set document, which should be optimalized
    /// \param document Document to optimalize
    void setDocument(const PDFDocument* document) { setStorage(document->getStorage()); }

    /// Set storage directly (storage must be valid and filled with objects)
    /// \param storage Storage
    void setStorage(const PDFObjectStorage& storage) { m_storage = storage; }

    /// Perform document optimalization. During optimization process, various
    /// signals are emitted to view progress.
    void optimize();

    /// Returns object storage used for optimization
    const PDFObjectStorage& getStorage() const { return m_storage; }

    /// Returns object storage by move semantics, old object storage is destroyed
    PDFObjectStorage takeStorage() { return qMove(m_storage); }

    /// Returns optimized document. Object storage is cleared after
    /// this function call.
    PDFDocument takeOptimizedDocument() { return PDFDocument(qMove(m_storage), PDFVersion(2, 0), QByteArray()); }

    OptimizationFlags getFlags() const;
    void setFlags(OptimizationFlags flags);

signals:
    void optimizationStarted();
    void optimizationProgress(QString progressText);
    void optimizationFinished();

private:
    bool performDereferenceSimpleObjects();
    bool performRemoveNullObjects();
    bool performRemoveUnusedObjects();
    bool performMergeIdenticalObjects();
    bool performShrinkObjectStorage();
    bool performRecompressFlateStreams();

    OptimizationFlags m_flags;
    PDFObjectStorage m_storage;
};

}   // namespace pdf

Q_DECLARE_OPERATORS_FOR_FLAGS(pdf::PDFOptimizer::OptimizationFlags)

#endif // PDFOPTIMIZER_H
