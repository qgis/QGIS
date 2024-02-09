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
//    along with PDF4QT.  If not, see <https://www.gnu.org/licenses/>.

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
