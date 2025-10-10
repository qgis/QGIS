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

#include "pdfglobal.h"

#include <QRectF>
#include <QTransform>

namespace pdf
{
class PDFPrecompiledPage;

/// Snapshot for current widget viewable items.
struct PDF4QTLIBCORESHARED_EXPORT PDFWidgetSnapshot
{
    struct SnapshotItem
    {
        PDFInteger pageIndex = -1;  ///< Index of page
        QRectF rect;                ///< Page rectangle on viewport
        QTransform pageToDeviceMatrix;             ///< Transforms page coordinates to widget coordinates
        const PDFPrecompiledPage* compiledPage = nullptr; ///< Compiled page (can be nullptr)
    };

    bool hasPage(PDFInteger pageIndex) const { return getPageSnapshot(pageIndex) != nullptr; }
    const SnapshotItem* getPageSnapshot(PDFInteger pageIndex) const;

    using SnapshotItems = std::vector<SnapshotItem>;
    SnapshotItems items;
};

}   // namespace pdf
