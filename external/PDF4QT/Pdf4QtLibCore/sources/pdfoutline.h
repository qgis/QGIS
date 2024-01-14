//    Copyright (C) 2019-2021 Jakub Melka
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
