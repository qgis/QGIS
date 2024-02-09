//    Copyright (C) 2021 Jakub Melka
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

#ifndef PDFXFAENGINE_H
#define PDFXFAENGINE_H

#include "pdfglobal.h"
#include "pdfdocument.h"
#include "pdfexception.h"

#include <memory>

namespace pdf
{

class PDFForm;
class PDFXFAEngineImpl;

class PDFXFAEngine
{
public:
    PDFXFAEngine();
    ~PDFXFAEngine();

    void setDocument(const PDFModifiedDocument& document, PDFForm* form);

    /// Returns list of page sizes (after paging is complete)
    std::vector<QSizeF> getPageSizes() const;

    /// Draws XFA form
    /// \param pagePointToDevicePointMatrix Page point to device point matrix
    /// \param page Page
    /// \param errors Error list (for reporting rendering errors)
    /// \param painter Painter
    void draw(const QTransform& pagePointToDevicePointMatrix,
              const PDFPage* page,
              QList<PDFRenderError>& errors,
              QPainter* painter);

private:
    std::unique_ptr<PDFXFAEngineImpl> m_impl;
};

/* START GENERATED CODE */

namespace xfa
{
} // namespace xfa


/* END GENERATED CODE */

}   // namespace pdf

#endif // PDFXFAENGINE_H
