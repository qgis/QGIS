//    Copyright (C) 2020-2021 Jakub Melka
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
