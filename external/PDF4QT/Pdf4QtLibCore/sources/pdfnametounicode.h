//    Copyright (C) 2018-2021 Jakub Melka
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
