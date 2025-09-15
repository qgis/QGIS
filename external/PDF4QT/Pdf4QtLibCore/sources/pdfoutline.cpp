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

#include "pdfoutline.h"
#include "pdfdocument.h"
#include "pdfexception.h"
#include "pdfencoding.h"
#include "pdfdbgheap.h"

namespace pdf
{

size_t PDFOutlineItem::getTotalCount() const
{
    size_t count = m_children.size();

    for (size_t i = 0; i < m_children.size(); ++i)
    {
        count += getChild(i)->getTotalCount();
    }

    return count;
}

QSharedPointer<PDFOutlineItem> PDFOutlineItem::parse(const PDFObjectStorage* storage, const PDFObject& root)
{
    const PDFObject& rootDereferenced = storage->getObject(root);
    if (rootDereferenced.isDictionary())
    {
        const PDFDictionary* dictionary = rootDereferenced.getDictionary();
        const PDFObject& first = dictionary->get("First");

        if (first.isReference())
        {
            QSharedPointer<PDFOutlineItem> result(new PDFOutlineItem());
            std::set<PDFObjectReference> visitedOutlineItems;
            parseImpl(storage, result.get(), first.getReference(), visitedOutlineItems);
            return result;
        }
    }

    return QSharedPointer<PDFOutlineItem>();
}

void PDFOutlineItem::parseImpl(const PDFObjectStorage* storage,
                               PDFOutlineItem* parent,
                               PDFObjectReference currentItem,
                               std::set<PDFObjectReference>& visitedOutlineItems)
{
    auto checkCyclicDependence = [&visitedOutlineItems](PDFObjectReference reference)
    {
        if (visitedOutlineItems.count(reference))
        {
            return false;
        }

        visitedOutlineItems.insert(reference);
        return true;
    };

    if (!checkCyclicDependence(currentItem))
    {
        return;
    }

    PDFObject dereferencedItem = storage->getObjectByReference(currentItem);
    while (dereferencedItem.isDictionary())
    {
        const PDFDictionary* dictionary = dereferencedItem.getDictionary();

        QSharedPointer<PDFOutlineItem> currentOutlineItem(new PDFOutlineItem());
        const PDFObject& titleObject = storage->getObject(dictionary->get("Title"));
        if (titleObject.isString())
        {
            currentOutlineItem->setTitle(PDFEncoding::convertTextString(titleObject.getString()));
        }
        currentOutlineItem->setAction(PDFAction::parse(storage, dictionary->get("A")));
        if (!currentOutlineItem->getAction() && dictionary->hasKey("Dest"))
        {
            currentOutlineItem->setAction(PDFActionPtr(new PDFActionGoTo(PDFDestination::parse(storage, dictionary->get("Dest")), PDFDestination())));
        }

        PDFDocumentDataLoaderDecorator loader(storage);
        std::vector<PDFReal> colors = loader.readNumberArrayFromDictionary(dictionary, "C", { 0.0, 0.0, 0.0 });
        colors.resize(3, 0.0);
        currentOutlineItem->setTextColor(QColor::fromRgbF(colors[0], colors[1], colors[2]));
        PDFInteger flag = loader.readIntegerFromDictionary(dictionary, "F", 0);
        currentOutlineItem->setFontItalics(flag & 0x1);
        currentOutlineItem->setFontBold(flag & 0x2);
        PDFObject structureElementObject = dictionary->get("SE");
        if (structureElementObject.isReference())
        {
            currentOutlineItem->setStructureElement(structureElementObject.getReference());
        }

        // Parse children of this item
        const PDFObject& firstItem = dictionary->get("First");
        if (firstItem.isReference())
        {
            std::set<PDFObjectReference> currentVisitedOutlineItems = visitedOutlineItems;
            parseImpl(storage, currentOutlineItem.get(), firstItem.getReference(), currentVisitedOutlineItems);
        }

        // Add new child to the parent
        parent->addChild(qMove(currentOutlineItem));

        // Parse next item
        const PDFObject& nextItem = dictionary->get("Next");
        if (nextItem.isReference())
        {
            if (!checkCyclicDependence(nextItem.getReference()))
            {
                break;
            }
            dereferencedItem = storage->getObject(nextItem);
        }
        else
        {
            // We are finished
            break;
        }
    }
}

PDFObjectReference PDFOutlineItem::getStructureElement() const
{
    return m_structureElement;
}

void PDFOutlineItem::setStructureElement(PDFObjectReference structureElement)
{
    m_structureElement = structureElement;
}

void PDFOutlineItem::apply(const std::function<void (PDFOutlineItem*)>& functor)
{
    functor(this);

    for (const auto& item : m_children)
    {
        item->apply(functor);
    }
}

QSharedPointer<PDFOutlineItem> PDFOutlineItem::clone() const
{
    QSharedPointer<PDFOutlineItem> result(new PDFOutlineItem());

    result->setTitle(getTitle());
    result->setTextColor(getTextColor());
    result->setStructureElement(getStructureElement());
    result->setFontItalics(isFontItalics());
    result->setFontBold(isFontBold());

    if (auto action = getAction())
    {
        result->setAction(action->clone());
    }

    for (size_t i = 0; i < getChildCount(); ++i)
    {
        result->addChild(getChild(i)->clone());
    }

    return result;
}

void PDFOutlineItem::insertChild(size_t index, QSharedPointer<PDFOutlineItem> item)
{
    m_children.insert(std::next(m_children.begin(), index), item);
}

void PDFOutlineItem::removeChild(size_t index)
{
    m_children.erase(std::next(m_children.begin(), index));
}

bool PDFOutlineItem::isFontBold() const
{
    return m_fontBold;
}

void PDFOutlineItem::setFontBold(bool fontBold)
{
    m_fontBold = fontBold;
}

bool PDFOutlineItem::isFontItalics() const
{
    return m_fontItalics;
}

void PDFOutlineItem::setFontItalics(bool fontItalics)
{
    m_fontItalics = fontItalics;
}

QColor PDFOutlineItem::getTextColor() const
{
    return m_textColor;
}

void PDFOutlineItem::setTextColor(const QColor& textColor)
{
    m_textColor = textColor;
}

const PDFAction* PDFOutlineItem::getAction() const
{
    return m_action.get();
}

PDFAction* PDFOutlineItem::getAction()
{
    return m_action.get();
}

void PDFOutlineItem::setAction(const PDFActionPtr& action)
{
    m_action = action;
}

}   // namespace pdf
