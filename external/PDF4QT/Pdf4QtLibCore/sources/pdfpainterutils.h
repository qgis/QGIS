//    Copyright (C) 2020-2021 Jakub Melka
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

#ifndef PDFPAINTERUTILS_H
#define PDFPAINTERUTILS_H

#include "pdfglobal.h"

#include <QPainter>

namespace pdf
{

/// RAII wrapper for painter save/restore
class PDFPainterStateGuard
{
public:
    explicit inline PDFPainterStateGuard(QPainter* painter) :
        m_painter(painter)
    {
        m_painter->save();
    }

    inline ~PDFPainterStateGuard()
    {
        m_painter->restore();
    }

private:
    QPainter* m_painter;
};

class PDF4QTLIBCORESHARED_EXPORT PDFPainterHelper
{
public:
    /// Draws bubble using painter. Bubble is aligned to the point, colored with color
    /// and inside bubble, text is being paint. Bubble bounding box is being returned.
    /// \param painter Painter
    /// \param point Position of the bubble
    /// \param color Color of the bubble
    /// \param text Text inside the bubble
    /// \param alignment Bubble alignment relative to the bubble position point
    static QRect drawBubble(QPainter* painter, QPoint point, QColor color, QString text, Qt::Alignment alignment);
};

}   // namespace pdf

#endif // PDFPAINTERUTILS_H
