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
