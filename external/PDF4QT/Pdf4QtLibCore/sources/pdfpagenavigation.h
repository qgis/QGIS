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

#ifndef PDFPAGENAVIGATION_H
#define PDFPAGENAVIGATION_H

#include "pdfpage.h"
#include "pdfcatalog.h"
#include "pdfdocument.h"
#include "pdfpagetransition.h"
#include "pdfaction.h"

#include <QObject>

namespace pdf
{

/// Page navigation node for subpage navigation.
class PDFNavigationNode
{
public:
    explicit PDFNavigationNode() = default;

    PDFActionPtr getPreviousAction() const;
    PDFActionPtr getNextAction() const;

    PDFObjectReference getPreviousNode() const;
    PDFObjectReference getNextNode() const;

    PDFReal getDuration() const;

    /// Parses navigation node. If error occurs, then empty optional
    /// item is returned.
    /// \param storage Storage
    /// \param object Object
    static std::optional<PDFNavigationNode> parse(const PDFObjectStorage* storage, PDFObject object);

private:
    PDFActionPtr m_previousAction;
    PDFActionPtr m_nextAction;
    PDFObjectReference m_previousNode;
    PDFObjectReference m_nextNode;
    PDFReal m_duration = 0.0;
};

/// Navigation object, which helps to navigate trough document.
/// It also handles subpage navigation, if subpage navigation
/// steps are present.
class PDF4QTLIBCORESHARED_EXPORT PDFPageNavigation : public QObject
{
    Q_OBJECT

public:
    explicit PDFPageNavigation(const PDFDocument* document, QObject* parent);

    /// Navigates to specified page. Page index must be valid.
    /// \param pageIndex Page index
    void navigateToPage(size_t pageIndex);

    /// Navigates forward. If subpage navigation is present,
    /// navigates to next page navigation node. Otherwise
    /// navigates to next page.
    void navigateForward();

    /// Navigates backward. If subpage navigation is present,
    /// navigates to previous page navigation node. Otherwise
    /// navigates to previous page.
    void navigateBackward();

signals:
    /// This signal is emitted, if subpage navigation is requested.
    void actionTriggered(const PDFAction* action);

    /// This signal is emitted, if navigation to another page is required.
    /// Page index is always valid. If document contains no pages, then
    /// this signal is never emitted. Also, transition is always a valid
    /// object (not null pointer).
    void pageChangeRequest(const size_t pageIndex, const PDFPageTransition* transition);

private:

    enum class Direction
    {
        Forward,  ///< We navigate forward
        Backward  ///< We navigate backward
    };

    void executeNavigationNode(Direction direction);

    size_t m_currentPageIndex;
    std::optional<PDFNavigationNode> m_currentNavigationNode;
    PDFPageTransition m_transition;
    const PDFDocument* m_document;
};

}   // namespace pdf

#endif // PDFPAGENAVIGATION_H
