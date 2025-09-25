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

#ifndef PDFJAVASCRIPTSCANNER_H
#define PDFJAVASCRIPTSCANNER_H

#include "pdfdocument.h"

namespace pdf
{

struct PDFJavaScriptEntry
{
    enum class Type
    {
        Invalid,
        Document,
        Named,
        Form,
        Page,
        Annotation
    };

    explicit PDFJavaScriptEntry() = default;
    explicit PDFJavaScriptEntry(Type type, PDFInteger pageIndex, QString javaScript) :
        type(type), pageIndex(pageIndex), javaScript(javaScript)
    {

    }

    bool operator <(const PDFJavaScriptEntry& other) const
    {
        return std::tie(type, pageIndex, javaScript) < std::tie(other.type, other.pageIndex, other.javaScript);
    }

    bool operator==(const PDFJavaScriptEntry&) const = default;

    Type type = Type::Invalid;
    PDFInteger pageIndex = -1;
    QString javaScript;
};

/// Scans document for all javascript presence (in actions). Several option
/// can be set, for example, scan only document actions, or stop scanning,
/// when first javascript is found.
class PDF4QTLIBCORESHARED_EXPORT PDFJavaScriptScanner
{
public:
    explicit PDFJavaScriptScanner(const PDFDocument* document);

    enum Option
    {
        AllPages        = 0x0001,   ///< Scan all pages
        FindFirstOnly   = 0x0002,   ///< Return only first javascript found
        ScanDocument    = 0x0004,   ///< Scan document related actions for javascript
        ScanNamed       = 0x0008,   ///< Scan named javascript in catalog
        ScanForm        = 0x0010,   ///< Scan javascript in form actions
        ScanPage        = 0x0020,   ///< Scan javascript in page annotations
        Optimize        = 0x0040,   ///< Remove redundant entries

        ScanMask = ScanDocument | ScanNamed | ScanForm | ScanPage
    };
    Q_DECLARE_FLAGS(Options, Option)

    using Entries = std::vector<PDFJavaScriptEntry>;

    /// Scans document for javascript actions using flags
    Entries scan(const std::vector<PDFInteger>& pages, Options options)  const;

    /// Returns true, if document has any java script action. Calling
    /// this function can be slow.
    bool hasJavaScript() const;

private:
    const PDFDocument* m_document;
};

}   // namespace pdf

Q_DECLARE_OPERATORS_FOR_FLAGS(pdf::PDFJavaScriptScanner::Options)

#endif // PDFJAVASCRIPTSCANNER_H
