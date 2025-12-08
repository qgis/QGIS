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

#include "pdfpagenavigation.h"
#include "pdfdbgheap.h"

namespace pdf
{

PDFPageNavigation::PDFPageNavigation(const PDFDocument* document, QObject* parent) :
    QObject(parent),
    m_currentPageIndex(0),
    m_document(document)
{
    if (m_document && m_document->getCatalog()->getPageCount() > 0)
    {
        navigateToPage(0);
    }
}

void PDFPageNavigation::navigateToPage(size_t pageIndex)
{
    if (m_currentPageIndex == pageIndex)
    {
        // Jakub Melka: We are at the same page! Do nothing.
        return;
    }

    const bool isForward = pageIndex > m_currentPageIndex;
    const PDFObjectStorage* const storage = &m_document->getStorage();
    const PDFPage* page = m_document->getCatalog()->getPage(pageIndex);

    // We will perform steps as in PDF 2.0 specification, chapter 12.4.4.2
    // Step a) navigate to page and set current navigation node
    m_currentPageIndex = pageIndex;
    m_transition = PDFPageTransition::parse(storage, page->getTransition(storage));
    m_currentNavigationNode = PDFNavigationNode::parse(storage, page->getFirstSubpageNavigationNode(storage));

    // Step b) Execute initially page navigation node. If we are navigating forward,
    //         then "next" action should be executed and current node should be changed to next.
    //         If we are navigating backward, then "previous" action should be executed
    //         and current node should be changed to previous.
    executeNavigationNode(isForward ? Direction::Forward : Direction::Backward);

    // Step c) Navigate to new page - send navigation request.
    Q_EMIT pageChangeRequest(m_currentPageIndex, &m_transition);
}

void PDFPageNavigation::navigateForward()
{
    if (m_currentNavigationNode)
    {
        executeNavigationNode(Direction::Forward);
    }
    else if (m_currentPageIndex + 1 < m_document->getCatalog()->getPageCount())
    {
        navigateToPage(m_currentPageIndex + 1);
    }
}

void PDFPageNavigation::navigateBackward()
{
    if (m_currentNavigationNode)
    {
        executeNavigationNode(Direction::Backward);
    }
    else if (m_currentPageIndex > 0)
    {
        navigateToPage(m_currentPageIndex - 1);
    }
}

void PDFPageNavigation::executeNavigationNode(Direction direction)
{
    if (!m_currentNavigationNode)
    {
        return;
    }

    // We will make a copy of object, because this object can change,
    // when actions are executed.
    PDFNavigationNode copy = *m_currentNavigationNode;
    m_currentNavigationNode = PDFNavigationNode::parse(&m_document->getStorage(), m_document->getObjectByReference((direction == Direction::Forward) ? copy.getNextNode() : copy.getPreviousNode()));

    switch (direction)
    {
        case Direction::Forward:
        {
            if (const PDFAction* action = copy.getNextAction().get())
            {
                for (const PDFAction* currentAction : action->getActionList())
                {
                    Q_EMIT actionTriggered(currentAction);
                }
            }
            break;
        }

        case Direction::Backward:
        {
            if (const PDFAction* action = copy.getPreviousAction().get())
            {
                for (const PDFAction* currentAction : action->getActionList())
                {
                    Q_EMIT actionTriggered(currentAction);
                }
            }
            break;
        }

        default:
            Q_ASSERT(false);
            break;
    }
}

PDFActionPtr PDFNavigationNode::getPreviousAction() const
{
    return m_previousAction;
}

PDFActionPtr PDFNavigationNode::getNextAction() const
{
    return m_nextAction;
}

PDFObjectReference PDFNavigationNode::getPreviousNode() const
{
    return m_previousNode;
}

PDFObjectReference PDFNavigationNode::getNextNode() const
{
    return m_nextNode;
}

PDFReal PDFNavigationNode::getDuration() const
{
    return m_duration;
}

std::optional<PDFNavigationNode> PDFNavigationNode::parse(const PDFObjectStorage* storage, PDFObject object)
{
    std::optional<PDFNavigationNode> result;

    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        PDFDocumentDataLoaderDecorator loader(storage);

        result = PDFNavigationNode();
        result->m_nextAction = PDFAction::parse(storage, dictionary->get("NA"));
        result->m_previousAction = PDFAction::parse(storage, dictionary->get("PA"));
        result->m_previousNode = loader.readReferenceFromDictionary(dictionary, "Prev");
        result->m_nextNode = loader.readReferenceFromDictionary(dictionary, "Next");
        result->m_duration = loader.readNumberFromDictionary(dictionary, "Dur", 0.0);
    }

    return result;
}

}   // namespace pdf
