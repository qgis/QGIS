//    Copyright (C) 2021-2022 Jakub Melka
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

#include "pdfpainterutils.h"

#include <QFontMetrics>

#include "pdfdbgheap.h"

namespace pdf
{

QRect PDFPainterHelper::drawBubble(QPainter* painter, QPoint point, QColor color, QString text, Qt::Alignment alignment)
{
    QFontMetrics fontMetrics = painter->fontMetrics();

    const int lineSpacing = fontMetrics.lineSpacing();
    const int bubbleHeight = lineSpacing* 2;
    const int bubbleWidth = lineSpacing + fontMetrics.horizontalAdvance(text);

    QRect rectangle(point, QSize(bubbleWidth, bubbleHeight));

    if (alignment.testFlag(Qt::AlignVCenter))
    {
        rectangle.translate(0, -rectangle.height() / 2);
    }
    else if (alignment.testFlag(Qt::AlignTop))
    {
        rectangle.translate(0, -rectangle.height());
    }

    if (alignment.testFlag(Qt::AlignHCenter))
    {
        rectangle.translate(-rectangle.width() / 2, 0);
    }
    else if (alignment.testFlag(Qt::AlignLeft))
    {
        rectangle.translate(-rectangle.width(), 0);
    }

    PDFPainterStateGuard guard(painter);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(color));
    painter->drawRoundedRect(rectangle, rectangle.height() / 2, rectangle.height() / 2, Qt::AbsoluteSize);
    painter->setPen(Qt::black);
    painter->drawText(rectangle, Qt::AlignCenter, text);

    return rectangle;
}

}   // namespace pdf
