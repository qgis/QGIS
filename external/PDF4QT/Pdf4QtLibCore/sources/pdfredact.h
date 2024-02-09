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

#ifndef PDFREDACT_H
#define PDFREDACT_H

#include "pdfdocument.h"
#include "pdfrenderer.h"

namespace pdf
{

/// Create redacted document from the document, which have redact annotations.
/// Redacted document has removed content marked by these annotations, and
/// annotations themselfs are removed.
class PDF4QTLIBCORESHARED_EXPORT PDFRedact
{
public:
    explicit PDFRedact(const PDFDocument* document,
                       const PDFFontCache* fontCache,
                       const PDFCMS* cms,
                       const PDFOptionalContentActivity* optionalContentActivity,
                       const PDFMeshQualitySettings* meshQualitySettings,
                       QColor redactFillColor);

    enum Option
    {
        None            = 0x0000,
        CopyTitle       = 0x0001,
        CopyMetadata    = 0x0002,
        CopyOutline     = 0x0004
    };
    Q_DECLARE_FLAGS(Options, Option)


    pdf::PDFDocument perform(Options options);

private:
    const PDFDocument* m_document;
    const PDFFontCache* m_fontCache;
    const PDFCMS* m_cms;
    const PDFOptionalContentActivity* m_optionalContentActivity;
    const PDFMeshQualitySettings* m_meshQualitySettings;
    QColor m_redactFillColor;
};

}   // namespace pdf

#endif // PDFREDACT_H
