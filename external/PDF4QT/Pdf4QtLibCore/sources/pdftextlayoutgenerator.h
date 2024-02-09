//    Copyright (C) 2023 Jakub Melka
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

#include "pdfpagecontentprocessor.h"

namespace pdf
{

class PDF4QTLIBCORESHARED_EXPORT PDFTextLayoutGenerator : public PDFPageContentProcessor
{
    using BaseClass = PDFPageContentProcessor;

public:
    explicit PDFTextLayoutGenerator(PDFRenderer::Features features,
                                    const PDFPage* page,
                                    const PDFDocument* document,
                                    const PDFFontCache* fontCache,
                                    const PDFCMS* cms,
                                    const PDFOptionalContentActivity* optionalContentActivity,
                                    QTransform pagePointToDevicePointMatrix,
                                    const PDFMeshQualitySettings& meshQualitySettings) :
        BaseClass(page, document, fontCache, cms, optionalContentActivity, pagePointToDevicePointMatrix, meshQualitySettings),
        m_features(features)
    {

    }

    /// Creates text layout from the text
    PDFTextLayout createTextLayout();

protected:
    virtual bool isContentSuppressedByOC(PDFObjectReference ocgOrOcmd) override;
    virtual bool isContentKindSuppressed(ContentKind kind) const override;
    virtual void performOutputCharacter(const PDFTextCharacterInfo& info) override;

private:
    PDFRenderer::Features m_features;
    PDFTextLayout m_textLayout;
};

}   // namespace pdf
