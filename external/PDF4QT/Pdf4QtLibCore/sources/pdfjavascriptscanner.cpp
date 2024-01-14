//    Copyright (C) 2020-2022 Jakub Melka
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
//    along with PDF4QT. If not, see <https://www.gnu.org/licenses/>.

#include "pdfjavascriptscanner.h"
#include "pdfaction.h"
#include "pdfform.h"
#include "pdfdbgheap.h"

namespace pdf
{

PDFJavaScriptScanner::PDFJavaScriptScanner(const PDFDocument* document) :
    m_document(document)
{

}

PDFJavaScriptScanner::Entries PDFJavaScriptScanner::scan(const std::vector<PDFInteger>& pages, Options options) const
{
    Entries result;

    auto scanAction = [options, &result](PDFJavaScriptEntry::Type type, PDFInteger pageIndex, const PDFAction* action)
    {
        if (!result.empty() && options.testFlag(FindFirstOnly))
        {
            return;
        }

        if (action)
        {
            std::vector<const PDFAction*> actions = action->getActionList();
            for (const PDFAction* a : actions)
            {
                switch (a->getType())
                {
                    case ActionType::JavaScript:
                    {
                        const PDFActionJavaScript* javascriptAction = dynamic_cast<const PDFActionJavaScript*>(a);
                        Q_ASSERT(javascriptAction);

                        result.emplace_back(type, pageIndex, javascriptAction->getJavaScript());
                        break;
                    }

                    case ActionType::Rendition:
                    {
                        const PDFActionRendition* renditionAction = dynamic_cast<const PDFActionRendition*>(a);
                        Q_ASSERT(renditionAction);

                        if (!renditionAction->getJavaScript().isEmpty())
                        {
                            result.emplace_back(type, pageIndex, renditionAction->getJavaScript());
                        }
                        break;
                    }

                    default:
                        break;
                }

                if (!result.empty() && options.testFlag(FindFirstOnly))
                {
                    break;
                }
            }
        }
    };

    auto scanContainer = [&scanAction](PDFJavaScriptEntry::Type type, PDFInteger pageIndex, const auto& container)
    {
        for (const PDFActionPtr& action : container)
        {
            scanAction(type, pageIndex, action.get());
        }
    };

    const PDFCatalog* catalog = m_document->getCatalog();

    if (options.testFlag(ScanDocument) && (result.empty() || !options.testFlag(FindFirstOnly)))
    {
        scanContainer(PDFJavaScriptEntry::Type::Document, -1, catalog->getDocumentActions());
    }

    if (options.testFlag(ScanNamed) && (result.empty() || !options.testFlag(FindFirstOnly)))
    {
        for (const auto& actionItem : catalog->getNamedJavaScriptActions())
        {
            scanAction(PDFJavaScriptEntry::Type::Named, -1, actionItem.second.get());
        }
    }

    if (options.testFlag(ScanForm) && (result.empty() || !options.testFlag(FindFirstOnly)))
    {
        PDFForm form = PDFForm::parse(m_document, catalog->getFormObject());
        if (form.isAcroForm() || form.isXFAForm())
        {
            auto fillActions = [&scanContainer](const PDFFormField* formField)
            {
                scanContainer(PDFJavaScriptEntry::Type::Form, -1, formField->getActions().getActions());
            };
            form.apply(fillActions);
        }
    }

    if (options.testFlag(ScanPage) && (result.empty() || !options.testFlag(FindFirstOnly)))
    {
        std::vector<PDFInteger> scannedPages;
        if (options.testFlag(AllPages))
        {
            scannedPages.resize(m_document->getCatalog()->getPageCount(), 0);
            std::iota(scannedPages.begin(), scannedPages.end(), 0);
        }
        else
        {
            scannedPages = pages;
        }

        for (const PDFInteger pageIndex : scannedPages)
        {
            if (pageIndex < 0 || pageIndex >= PDFInteger(catalog->getPageCount()))
            {
                continue;
            }

            if (!result.empty() && options.testFlag(FindFirstOnly))
            {
                break;
            }

            PDFPageAdditionalActions pageActions = PDFPageAdditionalActions::parse(&m_document->getStorage(), catalog->getPage(pageIndex)->getAdditionalActions(&m_document->getStorage()));
            scanContainer(PDFJavaScriptEntry::Type::Page, pageIndex, pageActions.getActions());

            const std::vector<PDFObjectReference>& pageAnnotations = catalog->getPage(pageIndex)->getAnnotations();
            for (PDFObjectReference annotationReference : pageAnnotations)
            {
                PDFAnnotationPtr annotationPtr = PDFAnnotation::parse(&m_document->getStorage(), annotationReference);
                if (annotationPtr)
                {
                    switch (annotationPtr->getType())
                    {
                        case AnnotationType::Link:
                        {
                            const PDFLinkAnnotation* linkAnnotation = dynamic_cast<const PDFLinkAnnotation*>(annotationPtr.get());
                            Q_ASSERT(linkAnnotation);

                            scanAction(PDFJavaScriptEntry::Type::Annotation, pageIndex, linkAnnotation->getAction());
                            break;
                        }

                        case AnnotationType::Screen:
                        {
                            const PDFScreenAnnotation* screenAnnotation = dynamic_cast<const PDFScreenAnnotation*>(annotationPtr.get());
                            Q_ASSERT(screenAnnotation);

                            scanAction(PDFJavaScriptEntry::Type::Annotation, pageIndex, screenAnnotation->getAction());
                            scanContainer(PDFJavaScriptEntry::Type::Annotation, pageIndex, screenAnnotation->getAdditionalActions().getActions());
                            break;
                        }

                        case AnnotationType::Widget:
                        {
                            const PDFWidgetAnnotation* widgetAnnotation = dynamic_cast<const PDFWidgetAnnotation*>(annotationPtr.get());
                            Q_ASSERT(widgetAnnotation);

                            scanAction(PDFJavaScriptEntry::Type::Annotation, pageIndex, widgetAnnotation->getAction());
                            scanContainer(PDFJavaScriptEntry::Type::Annotation, pageIndex, widgetAnnotation->getAdditionalActions().getActions());
                            break;
                        }

                        default:
                            break;
                    }
                }
            }
        }
    }

    std::sort(result.begin(), result.end());
    if (options.testFlag(Optimize))
    {
        result.erase(std::unique(result.begin(), result.end()), result.end());
    }

    return result;
}

bool PDFJavaScriptScanner::hasJavaScript() const
{
    return !scan({ }, Options(AllPages | FindFirstOnly | ScanDocument | ScanNamed | ScanForm | ScanPage)).empty();
}

}   // namespace pdf
