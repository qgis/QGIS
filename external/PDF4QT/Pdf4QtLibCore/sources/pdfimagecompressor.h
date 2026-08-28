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

#ifndef PDFIMAGECOMPRESSOR_H
#define PDFIMAGECOMPRESSOR_H

#include "pdfglobal.h"
#include "pdfobject.h"

#include <QImage>
#include <QPointF>

#include <limits>
#include <vector>

namespace pdf
{
class PDFDocument;

/// Helper responsible for gathering image statistics needed for compression.
class PDF4QTLIBCORESHARED_EXPORT PDFImageCompressor
{
public:
    struct ImageStatistics
    {
        PDFObjectReference reference;
        QImage image;
        QPointF minimalDpi = QPointF(std::numeric_limits<double>::infinity(),
                                     std::numeric_limits<double>::infinity());
    };

    using ImageStatisticsList = std::vector<ImageStatistics>;

    /// Collects all image XObjects from the document and computes their
    /// analysis data required for compression decisions.
    /// \param document Processed document
    /// \return Collected statistics for every unique image reference
    ImageStatisticsList collectImages(const PDFDocument* document) const;
};

}   // namespace pdf

#endif // PDFIMAGECOMPRESSOR_H
