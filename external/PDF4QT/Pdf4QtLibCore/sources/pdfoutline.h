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

#ifndef PDFOUTLINE_H
#define PDFOUTLINE_H

#include "pdfglobal.h"
#include "pdfobject.h"
#include "pdfaction.h"

#include <QSharedPointer>

#include <set>

namespace pdf
{
class PDFDocument;

/// Outline item
class PDF4QTLIBCORESHARED_EXPORT PDFOutlineItem
{
public:
    explicit PDFOutlineItem() = default;

    const QString& getTitle() const { return m_title; }
    void setTitle(const QString& title) { m_title = title; }

    size_t getChildCount() const { return m_children.size(); }
    size_t getTotalCount() const;
    const PDFOutlineItem* getChild(size_t index) const { return m_children[index].get(); }
    void addChild(QSharedPointer<PDFOutlineItem> child) { m_children.emplace_back(qMove(child)); }
    QSharedPointer<PDFOutlineItem> getChildPtr(size_t index) const { return m_children[index]; }

    static QSharedPointer<PDFOutlineItem> parse(const PDFObjectStorage* storage, const PDFObject& root);

    const PDFAction* getAction() const;
    PDFAction* getAction();
    void setAction(const PDFActionPtr& action);

    QColor getTextColor() const;
    void setTextColor(const QColor& textColor);

    bool isFontItalics() const;
    void setFontItalics(bool fontItalics);

    bool isFontBold() const;
    void setFontBold(bool fontBold);

    PDFObjectReference getStructureElement() const;
    void setStructureElement(PDFObjectReference structureElement);

    void apply(const std::function<void(PDFOutlineItem*)>& functor);

    QSharedPointer<PDFOutlineItem> clone() const;

    void insertChild(size_t index, QSharedPointer<PDFOutlineItem> item);
    void removeChild(size_t index);

private:
    static void parseImpl(const PDFObjectStorage* storage,
                          PDFOutlineItem* parent,
                          PDFObjectReference currentItem,
                          std::set<PDFObjectReference>& visitedOutlineItems);

    QString m_title;
    std::vector<QSharedPointer<PDFOutlineItem>> m_children;
    PDFActionPtr m_action;
    PDFObjectReference m_structureElement;
    QColor m_textColor;
    bool m_fontItalics = false;
    bool m_fontBold = false;
};

}   // namespace pdf

#endif // PDFOUTLINE_H
