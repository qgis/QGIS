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

#include "pdfpainterutils.h"
#include "pdfpagecontentprocessor.h"

#include <QPainterPath>
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

QPen PDFPainterHelper::createPenFromState(const PDFPageContentProcessorState* graphicState, double alpha)
{
    QColor color = graphicState->getStrokeColor();
    if (color.isValid())
    {
        color.setAlphaF(alpha);
        const PDFReal lineWidth = graphicState->getLineWidth();
        Qt::PenCapStyle penCapStyle = graphicState->getLineCapStyle();
        Qt::PenJoinStyle penJoinStyle = graphicState->getLineJoinStyle();
        const PDFLineDashPattern& lineDashPattern = graphicState->getLineDashPattern();
        const PDFReal mitterLimit = graphicState->getMitterLimit();

        QPen pen(color);

        pen.setWidthF(lineWidth);
        pen.setCapStyle(penCapStyle);
        pen.setJoinStyle(penJoinStyle);
        pen.setMiterLimit(mitterLimit);

        if (lineDashPattern.isSolid())
        {
            pen.setStyle(Qt::SolidLine);
        }
        else
        {
            pen.setStyle(Qt::CustomDashLine);
            pen.setDashPattern(lineDashPattern.createForQPen(pen.widthF()));
            pen.setDashOffset(lineDashPattern.getDashOffset());
        }

        return pen;
    }
    else
    {
        return QPen(Qt::NoPen);
    }
}

QBrush PDFPainterHelper::createBrushFromState(const PDFPageContentProcessorState* graphicState, double alpha)
{
    QColor color = graphicState->getFillColor();
    if (color.isValid())
    {
        color.setAlphaF(alpha);
        return QBrush(color, Qt::SolidPattern);
    }
    else
    {
        return QBrush(Qt::NoBrush);
    }
}

void PDFPainterHelper::applyPenToGraphicState(PDFPageContentProcessorState* graphicState, const QPen& pen)
{
    if (pen.style() != Qt::NoPen)
    {
        graphicState->setLineWidth(pen.widthF());
        graphicState->setLineCapStyle(pen.capStyle());
        graphicState->setLineJoinStyle(pen.joinStyle());
        graphicState->setMitterLimit(pen.miterLimit());

        QColor color = pen.color();

        graphicState->setAlphaStroking(color.alphaF());

        const PDFAbstractColorSpace* strokeColorSpace = graphicState->getStrokeColorSpace();
        if (!strokeColorSpace || strokeColorSpace->getColorSpace() != PDFAbstractColorSpace::ColorSpace::DeviceRGB)
        {
            graphicState->setStrokeColorSpace(QSharedPointer<PDFAbstractColorSpace>(new PDFDeviceRGBColorSpace()));
        }
        graphicState->setStrokeColor(color, PDFColor(color.redF(), color.greenF(), color.blueF()));

        if (pen.style() == Qt::SolidLine)
        {
            graphicState->setLineDashPattern(PDFLineDashPattern());
        }
        else
        {
            PDFLineDashPattern lineDashPattern;
            QList<qreal> penPattern = pen.dashPattern();
            PDFReal penWidth = pen.widthF();

            std::vector<PDFReal> dashArray;
            for (qreal value : penPattern)
            {
                dashArray.push_back(value * penWidth);
            }

            lineDashPattern.setDashArray(std::move(dashArray));
            lineDashPattern.setDashOffset(pen.dashOffset());
            graphicState->setLineDashPattern(std::move(lineDashPattern));
        }
    }
}

void PDFPainterHelper::applyBrushToGraphicState(PDFPageContentProcessorState* graphicState, const QBrush& brush)
{
    if (brush.style() != Qt::NoBrush)
    {
        QColor color = brush.color();

        graphicState->setAlphaFilling(color.alphaF());

        const PDFAbstractColorSpace* fillColorSpace = graphicState->getFillColorSpace();
        if (!fillColorSpace || fillColorSpace->getColorSpace() != PDFAbstractColorSpace::ColorSpace::DeviceRGB)
        {
            graphicState->setFillColorSpace(QSharedPointer<PDFAbstractColorSpace>(new PDFDeviceRGBColorSpace()));
        }
        graphicState->setFillColor(color, PDFColor(color.redF(), color.greenF(), color.blueF()));
    }
}

PDFTransformationDecomposition PDFPainterHelper::decomposeTransform(const QTransform& transform)
{
    PDFTransformationDecomposition result;

    const qreal m11 = transform.m11();
    const qreal m12 = transform.m12();
    const qreal m21 = transform.m21();
    const qreal m22 = transform.m22();

    const qreal dx = transform.dx();
    const qreal dy = transform.dy();

    const qreal sx = std::sqrt(m11 * m11 + m21 * m21);
    const qreal phi = std::atan2(m21, m11);
    const qreal msy = m12 * std::cos(phi) + m22 * std::sin(phi);
    const qreal sy = -m12 * std::sin(phi) + m22 * std::cos(phi);

    result.rotationAngle = phi;
    result.scaleX = sx;
    result.scaleY = sy;

    if (!qFuzzyIsNull(sy))
    {
        result.shearFactor = msy / sy;
    }
    else
    {
        result.shearFactor = 0.0;
    }

    result.translateX = dx;
    result.translateY = dy;

    return result;
}

QTransform PDFPainterHelper::composeTransform(const PDFTransformationDecomposition& decomposition)
{
    const qreal s = std::sin(decomposition.rotationAngle);
    const qreal c = std::cos(decomposition.rotationAngle);

    const qreal m = decomposition.shearFactor;
    const qreal sx = decomposition.scaleX;
    const qreal sy = decomposition.scaleY;
    const qreal dx = decomposition.translateX;
    const qreal dy = decomposition.translateY;

    const qreal m11 = sx * c;
    const qreal m12 = sy * m * c - sy * s;
    const qreal m21 = sx * s;
    const qreal m22 = sy * m * s + sy * c;

    return QTransform(m11, m12, m21, m22, dx, dy);
}

}   // namespace pdf
