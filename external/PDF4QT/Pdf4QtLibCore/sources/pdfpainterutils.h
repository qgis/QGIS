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

#ifndef PDFPAINTERUTILS_H
#define PDFPAINTERUTILS_H

#include "pdfglobal.h"

#include <QPainter>

namespace pdf
{
class PDFPageContentProcessorState;

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

struct PDFTransformationDecomposition
{
    double rotationAngle = 0.0;
    double shearFactor = 0.0;
    double scaleX = 0.0;
    double scaleY = 0.0;
    double translateX = 0.0;
    double translateY = 0.0;
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

    /// Creates pen from painter graphicState
    static QPen createPenFromState(const PDFPageContentProcessorState* graphicState, double alpha);

    /// Creates brush from painter graphicState
    static QBrush createBrushFromState(const PDFPageContentProcessorState* graphicState, double alpha);

    static void applyPenToGraphicState(PDFPageContentProcessorState* graphicState, const QPen& pen);
    static void applyBrushToGraphicState(PDFPageContentProcessorState* graphicState, const QBrush& brush);

    /// Decompose transform
    static PDFTransformationDecomposition decomposeTransform(const QTransform& transform);

    /// Compose transform
    static QTransform composeTransform(const PDFTransformationDecomposition& decomposition);
};

}   // namespace pdf

#endif // PDFPAINTERUTILS_H
