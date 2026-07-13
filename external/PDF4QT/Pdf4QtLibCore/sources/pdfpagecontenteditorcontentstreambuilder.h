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

#ifndef PDFPAGECONTENTEDITORCONTENTSTREAMBUILDER_H
#define PDFPAGECONTENTEDITORCONTENTSTREAMBUILDER_H

#include "pdfpagecontenteditorprocessor.h"
#include "pdfeditorfallbackfont.h"

#include <QHash>
#include <QPaintDevice>

namespace pdf
{
class PDFPageContentElement;
class PDFContentEditorPaintEngine;
class PDFPageContentEditorContentStreamBuilder;

class PDF4QTLIBCORESHARED_EXPORT PDFContentEditorPaintDevice : public QPaintDevice
{
public:
    PDFContentEditorPaintDevice(PDFPageContentEditorContentStreamBuilder* builder, QRectF mediaRect, QRectF mediaRectMM);
    virtual ~PDFContentEditorPaintDevice() override;

    virtual int devType() const override;
    virtual QPaintEngine* paintEngine() const override;

protected:
    virtual int metric(PaintDeviceMetric metric) const override;

private:
    PDFContentEditorPaintEngine* m_paintEngine;
    QRectF m_mediaRect;
    QRectF m_mediaRectMM;
};

class PDF4QTLIBCORESHARED_EXPORT PDFPageContentEditorContentStreamBuilder
{
public:
    PDFPageContentEditorContentStreamBuilder(PDFDocument* document);

    void writeEditedElement(const PDFEditedPageContentElement* element);

    const QByteArray& getOutputContent() const;

    const PDFDictionary& getFontDictionary() const { return m_fontDictionary; }
    const PDFDictionary& getXObjectDictionary() const { return m_xobjectDictionary; }
    const PDFDictionary& getGraphicStateDictionary() const { return m_graphicStateDictionary; }

    void setFontDictionary(const PDFDictionary& newFontDictionary);
    void setXObjectDictionary(const PDFDictionary& newXObjectDictionary);
    void setGraphicStateDictionary(const PDFDictionary& newGraphicStateDictionary);

    const QStringList& getErrors() const { return m_errors; }
    void clearErrors() { m_errors.clear(); }

    void writeStyledPath(const QPainterPath& path,
                         const QPen& pen,
                         const QBrush& brush,
                         bool isStroking,
                         bool isFilling);

    /// Writes styled path with the given graphic state. Optional clip path
    /// is expressed in the page coordinate space (it is applied before
    /// the current transformation matrix is written). An empty clip path
    /// means no clipping.
    void writeStyledPath(const QPainterPath& path,
                         const PDFPageContentProcessorState& state,
                         bool isStroking,
                         bool isFilling,
                         const QPainterPath& clipPath = QPainterPath());

    void writeImage(const QImage& image, const QRectF& rectangle);

    /// Writes image placed by the painter transform. Optional clip path
    /// is expressed in the page coordinate space. An empty clip path
    /// means no clipping.
    void writeImage(const QImage& image, QTransform transform, const QRectF& rectangle, const QPainterPath& clipPath = QPainterPath());

    const PDFPageContentProcessorState& getCurrentState() { return m_currentState; }

    /// Writes the path construction operators (m, l, c, h) for the given path,
    /// without any painting operator.
    static void writePathGeometry(QTextStream& stream, const QPainterPath& path);

private:
    bool isNeededToWriteCurrentTransformationMatrix() const;

    void writeCurrentTransformationMatrix(QTextStream& stream);
    void writeStateDifference(QTextStream& stream, const PDFPageContentProcessorState& state);

    void writePainterPath(QTextStream& stream,
                          const QPainterPath& path,
                          bool isStroking,
                          bool isFilling);

    /// Writes the path as a clip path (path construction operators followed
    /// by "W n" or "W* n", according to the path fill rule).
    void writeClipPath(QTextStream& stream, const QPainterPath& clipPath);

    void writeText(QTextStream& stream, const QString& text);
    void writeTextCommand(QTextStream& stream, const QXmlStreamReader& reader);

    /// Writes text characters using the current text font. Characters, which
    /// cannot be encoded into the current font, are written using an on-the-fly
    /// generated Type 3 fallback font.
    void writeTextWithFallback(QTextStream& stream, const QString& characters);

    void writeTextHexString(QTextStream& stream, const QByteArray& encodedText);

    void writeImage(QTextStream& stream, const QImage& image);
    void writeImageObject(QTextStream& stream, const PDFObject& imageObject);

    QByteArray selectFont(const QByteArray& font);
    void addError(const QString& error);

    PDFDocument* m_document = nullptr;
    PDFDictionary m_fontDictionary;
    PDFDictionary m_xobjectDictionary;
    PDFDictionary m_graphicStateDictionary;
    QByteArray m_outputContent;
    PDFPageContentProcessorState m_currentState;
    PDFFontPointer m_textFont;
    QHash<QByteArray, PDFFontPointer> m_fontOverrides;
    QStringList m_errors;
    PDFEditorFallbackFontManager m_fallbackFontManager;
    QByteArray m_currentTextFontKey;    ///< Resource key of the last written Tf operator
    PDFReal m_currentTextFontSize = 0.0;
};

}   // namespace pdf

#endif // PDFPAGECONTENTEDITORCONTENTSTREAMBUILDER_H
