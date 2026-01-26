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

#ifndef PDFNAMETOUNICODE_H
#define PDFNAMETOUNICODE_H

#include "pdfglobal.h"

#include <QChar>
#include <QByteArray>

namespace pdf
{

class PDF4QTLIBCORESHARED_EXPORT PDFNameToUnicode
{
public:
    explicit PDFNameToUnicode() = delete;

    /// Returns unicode character for name. If name is not found, then null character is returned.
    static QChar getUnicodeForName(const QByteArray& name);

    /// Returns unicode character for name (for ZapfDingbats). If name is not found, then null character is returned.
    static QChar getUnicodeForNameZapfDingbats(const QByteArray& name);

    /// Tries to resolve unicode name
    static QChar getUnicodeUsingResolvedName(const QByteArray& name);

private:
    struct Comparator
    {
        inline bool operator()(const QByteArray& left, const std::pair<QChar, const char*>& right)
        {
            return left < right.second;
        }
        inline bool operator()(const std::pair<QChar, const char*>& left, const QByteArray& right)
        {
            return left.second < right;
        }
        inline bool operator()(const std::pair<QChar, const char*>& left, const std::pair<QChar, const char*>& right)
        {
            return QLatin1String(left.second) < QLatin1String(right.second);
        }
    };
};

}   // namespace pdf

#endif // PDFNAMETOUNICODE_H
