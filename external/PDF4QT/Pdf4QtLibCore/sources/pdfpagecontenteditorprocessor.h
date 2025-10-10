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

#ifndef PDFPAGECONTENTEDITORPROCESSOR_H
#define PDFPAGECONTENTEDITORPROCESSOR_H

#include "pdfpagecontentprocessor.h"

#include <memory>

class QXmlStreamReader;

namespace pdf
{

class PDFEditedPageContentElementPath;
class PDFEditedPageContentElementText;
class PDFEditedPageContentElementImage;

class PDF4QTLIBCORESHARED_EXPORT PDFEditedPageContentElement
{
public:
    PDFEditedPageContentElement() = default;
    PDFEditedPageContentElement(PDFPageContentProcessorState state, QTransform transform);
    virtual ~PDFEditedPageContentElement() = default;

    enum class Type
    {
        Path,
        Text,
        Image
    };

    virtual Type getType() const = 0;
    virtual PDFEditedPageContentElement* clone() const = 0;

    virtual PDFEditedPageContentElementPath* asPath() { return nullptr; }
    virtual const PDFEditedPageContentElementPath* asPath() const { return nullptr; }

    virtual PDFEditedPageContentElementText* asText() { return nullptr; }
    virtual const PDFEditedPageContentElementText* asText() const { return nullptr; }

    virtual PDFEditedPageContentElementImage* asImage() { return nullptr; }
    virtual const PDFEditedPageContentElementImage* asImage() const { return nullptr; }

    const PDFPageContentProcessorState& getState() const;
    void setState(const PDFPageContentProcessorState& newState);

    virtual QRectF getBoundingBox() const = 0;

    QTransform getTransform() const;
    void setTransform(const QTransform& newTransform);

protected:
    PDFPageContentProcessorState m_state;
    QTransform m_transform;
};

class PDF4QTLIBCORESHARED_EXPORT PDFEditedPageContentElementPath : public PDFEditedPageContentElement
{
public:
    PDFEditedPageContentElementPath(PDFPageContentProcessorState state,
                                    QPainterPath path,
                                    bool strokePath,
                                    bool fillPath,
                                    QTransform transform);
    virtual ~PDFEditedPageContentElementPath() = default;

    virtual Type getType() const override;
    virtual PDFEditedPageContentElementPath* clone() const override;
    virtual PDFEditedPageContentElementPath* asPath() override { return this; }
    virtual const PDFEditedPageContentElementPath* asPath() const override { return this; }
    virtual QRectF getBoundingBox() const override;

    QPainterPath getPath() const;
    void setPath(QPainterPath newPath);

    bool getStrokePath() const;
    void setStrokePath(bool newStrokePath);

    bool getFillPath() const;
    void setFillPath(bool newFillPath);

private:
    QPainterPath m_path;
    bool m_strokePath;
    bool m_fillPath;
};

class PDF4QTLIBCORESHARED_EXPORT PDFEditedPageContentElementImage : public PDFEditedPageContentElement
{
public:
    PDFEditedPageContentElementImage(PDFPageContentProcessorState state,
                                     PDFObject imageObject,
                                     QImage image,
                                     QTransform transform);
    virtual ~PDFEditedPageContentElementImage() = default;

    virtual Type getType() const override;
    virtual PDFEditedPageContentElementImage* clone() const override;
    virtual PDFEditedPageContentElementImage* asImage() override { return this; }
    virtual const PDFEditedPageContentElementImage* asImage() const override { return this; }
    virtual QRectF getBoundingBox() const override;

    PDFObject getImageObject() const;
    void setImageObject(const PDFObject& newImageObject);

    QImage getImage() const;
    void setImage(const QImage& newImage);

private:
    PDFObject m_imageObject;
    QImage m_image;
};

class PDF4QTLIBCORESHARED_EXPORT PDFEditedPageContentElementText : public PDFEditedPageContentElement
{
public:

    struct Item
    {
        bool isUpdateGraphicState = false;
        bool isText = false;
        TextSequence textSequence;

        PDFPageContentProcessorState state;
    };

    PDFEditedPageContentElementText(PDFPageContentProcessorState state, QTransform transform);
    PDFEditedPageContentElementText(PDFPageContentProcessorState state,
                                    std::vector<Item> items,
                                    QPainterPath textPath,
                                    QTransform transform,
                                    QString itemsAsText);
    virtual ~PDFEditedPageContentElementText() = default;

    virtual Type getType() const override;
    virtual PDFEditedPageContentElementText* clone() const override;
    virtual PDFEditedPageContentElementText* asText() override { return this; }
    virtual const PDFEditedPageContentElementText* asText() const override { return this; }
    virtual QRectF getBoundingBox() const override;

    void addItem(Item item);
    const std::vector<Item>& getItems() const;
    void setItems(const std::vector<Item>& newItems);

    bool isEmpty() const { return m_items.empty(); }

    QPainterPath getTextPath() const;
    void setTextPath(QPainterPath newTextPath);

    static QString createItemsAsText(const PDFPageContentProcessorState& initialState,
                                     const std::vector<Item>& items);

    QString getItemsAsText() const;
    void setItemsAsText(const QString& newItemsAsText);

    void optimize();

private:
    std::vector<Item> m_items;
    QPainterPath m_textPath;
    QString m_itemsAsText;
};

class PDF4QTLIBCORESHARED_EXPORT PDFEditedPageContent
{
public:
    PDFEditedPageContent() = default;
    PDFEditedPageContent(const PDFEditedPageContent&) = delete;
    PDFEditedPageContent(PDFEditedPageContent&&) = default;

    PDFEditedPageContent& operator=(const PDFEditedPageContent&) = delete;
    PDFEditedPageContent& operator=(PDFEditedPageContent&&) = default;

    static QString getOperatorToString(PDFPageContentProcessor::Operator operatorValue);
    static QString getOperandName(PDFPageContentProcessor::Operator operatorValue, int operandIndex);

    void addContentPath(PDFPageContentProcessorState state, QPainterPath path, bool strokePath, bool fillPath);
    void addContentImage(PDFPageContentProcessorState state, PDFObject imageObject, QImage image);
    void addContentClipping(PDFPageContentProcessorState state, QPainterPath path);
    void addContentElement(std::unique_ptr<PDFEditedPageContentElement> element);

    std::size_t getElementCount() const { return m_contentElements.size(); }
    PDFEditedPageContentElement* getElement(size_t index) const { return m_contentElements.at(index).get(); }

    PDFEditedPageContentElement* getBackElement() const;

    PDFDictionary getFontDictionary() const;
    void setFontDictionary(const PDFDictionary& newFontDictionary);

    PDFDictionary getXObjectDictionary() const;
    void setXObjectDictionary(const PDFDictionary& newXobjectDictionary);

private:
    std::vector<std::unique_ptr<PDFEditedPageContentElement>> m_contentElements;
    PDFDictionary m_fontDictionary;
    PDFDictionary m_xobjectDictionary;
};

class PDF4QTLIBCORESHARED_EXPORT PDFPageContentEditorProcessor : public PDFPageContentProcessor
{
    using BaseClass = PDFPageContentProcessor;

public:
    PDFPageContentEditorProcessor(const PDFPage* page,
                                  const PDFDocument* document,
                                  const PDFFontCache* fontCache,
                                  const PDFCMS* CMS,
                                  const PDFOptionalContentActivity* optionalContentActivity,
                                  QTransform pagePointToDevicePointMatrix,
                                  const PDFMeshQualitySettings& meshQualitySettings);

    const PDFEditedPageContent& getEditedPageContent() const;
    PDFEditedPageContent takeEditedPageContent();

protected:
    virtual void performInterceptInstruction(Operator currentOperator, ProcessOrder processOrder, const QByteArray& operatorAsText) override;
    virtual void performPathPainting(const QPainterPath& path, bool stroke, bool fill, bool text, Qt::FillRule fillRule) override;
    virtual bool isContentKindSuppressed(ContentKind kind) const override;
    virtual bool performOriginalImagePainting(const PDFImage& image, const PDFStream* stream) override;
    virtual void performImagePainting(const QImage& image) override;
    virtual void performClipping(const QPainterPath& path, Qt::FillRule fillRule) override;
    virtual void performSaveGraphicState(ProcessOrder order) override;
    virtual void performRestoreGraphicState(ProcessOrder order) override;
    virtual void performUpdateGraphicsState(const PDFPageContentProcessorState& state) override;
    virtual void performProcessTextSequence(const TextSequence& textSequence, ProcessOrder order) override;

private:
    PDFEditedPageContent m_content;
    std::stack<QPainterPath> m_clippingPaths;
    std::unique_ptr<PDFEditedPageContentElementText> m_contentElementText;
    QPainterPath m_textPath;
};

}   // namespace pdf

#endif // PDFPAGECONTENTEDITORPROCESSOR_H
