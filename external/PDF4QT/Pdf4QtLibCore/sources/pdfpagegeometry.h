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

#ifndef PDFPAGEGEOMETRY_H
#define PDFPAGEGEOMETRY_H

#include "pdfglobal.h"
#include "pdfdocument.h"
#include "pdfutils.h"

#include <QMarginsF>
#include <QSizeF>
#include <QString>
#include <QPointF>

namespace pdf
{

/**
 * \brief Configuration for page geometry adjustments performed by PDFPageGeometry.
 *
 * This structure describes how selected pages should be resized, repositioned,
 * and optionally scaled. The operation can target one or more page boxes,
 * derive the placement from an existing reference box, restrict the affected
 * pages by range or subset, and optionally transform annotation geometry
 * together with page content.
 *
 * All user-facing dimensions are expressed in millimeters. They are converted
 * internally to PDF points before the modification is written back to the
 * document.
 */
struct PDF4QTLIBCORESHARED_EXPORT PDFPageGeometrySettings
{
    /**
     * \brief Restricts the operation to a subset of pages.
     */
    enum class PageSubset
    {
        AllPages,          ///< Apply the operation to all selected pages.
        OddPages,          ///< Apply the operation only to odd-numbered pages.
        EvenPages,         ///< Apply the operation only to even-numbered pages.
        PortraitPages,     ///< Apply the operation only to portrait pages.
        LandscapePages     ///< Apply the operation only to landscape pages.
    };

    /**
     * \brief Selects which page box is used as the source geometry reference.
     */
    enum class ReferenceBox
    {
        MediaBox,   ///< Use the page media box.
        CropBox,    ///< Use the page crop box.
        BleedBox,   ///< Use the page bleed box.
        TrimBox,    ///< Use the page trim box.
        ArtBox      ///< Use the page art box.
    };

    /**
     * \brief Defines the alignment anchor inside the target inner rectangle.
     *
     * The anchor is used when the referenced source box is placed into the
     * target area after margins are applied.
     */
    enum class Anchor
    {
        TopLeft,        ///< Align to the top-left corner.
        TopCenter,      ///< Align to the top edge center.
        TopRight,       ///< Align to the top-right corner.
        MiddleLeft,     ///< Align to the left edge center.
        MiddleCenter,   ///< Align to the center.
        MiddleRight,    ///< Align to the right edge center.
        BottomLeft,     ///< Align to the bottom-left corner.
        BottomCenter,   ///< Align to the bottom edge center.
        BottomRight     ///< Align to the bottom-right corner.
    };

    QString pageRange = "-";                          ///< Page range in the standard PDF4QT interval syntax.
    PageSubset pageSubset = PageSubset::AllPages;    ///< Additional subset filter applied after the page range.
    ReferenceBox referenceBox = ReferenceBox::CropBox; ///< Source page box used to compute placement and scaling.

    bool applyMediaBox = true;   ///< Rewrite the media box.
    bool applyCropBox = true;    ///< Rewrite the crop box.
    bool applyBleedBox = false;  ///< Rewrite the bleed box.
    bool applyTrimBox = false;   ///< Rewrite the trim box.
    bool applyArtBox = false;    ///< Rewrite the art box.

    bool useTargetPageSize = false;                      ///< Use targetPageSizeMM instead of deriving the target size from the source box and margins.
    QSizeF targetPageSizeMM = QSizeF(210.0, 297.0);     ///< Explicit output page size in millimeters.
    QMarginsF marginsMM = QMarginsF(0.0, 0.0, 0.0, 0.0); ///< Inner margins in millimeters, ordered as left, top, right, bottom.

    Anchor anchor = Anchor::MiddleCenter;             ///< Anchor used to place the transformed source box into the target inner rectangle.
    QPointF offsetMM = QPointF(0.0, 0.0);             ///< Additional placement offset in millimeters.

    bool scaleContent = false;                        ///< Scale page content to fit the target inner rectangle.
    bool preserveAspectRatio = true;                  ///< Preserve the original aspect ratio when scaleContent is enabled.
    bool scaleAnnotationsAndFormFields = true;        ///< Transform annotation and widget geometry together with scaled page content.

    /**
     * \brief Returns true if at least one output page box is selected.
     */
    bool hasAnyTargetBoxSelected() const
    {
        return applyMediaBox || applyCropBox || applyBleedBox || applyTrimBox || applyArtBox;
    }
};

/**
 * \brief Applies high-level page geometry transformations to a PDF document.
 *
 * The operation can:
 * - select pages by range and subset,
 * - use a chosen page box as the source reference,
 * - derive a new page size or use an explicit target size,
 * - add margins and align the content using an anchor,
 * - optionally scale page content,
 * - optionally transform annotations and form fields together with content,
 * - rewrite one or more destination page boxes.
 *
 * The document is modified in place only when the operation finalizes
 * successfully.
 */
class PDF4QTLIBCORESHARED_EXPORT PDFPageGeometry
{
public:
    /**
     * \brief Applies page geometry settings to a document.
     * \param document Document to modify.
     * \param settings Geometry settings describing the requested transformation.
     * \param modificationFlags Optional output flags describing what parts of the
     *        document were modified.
     * \return Operation result. On failure, the result contains a translated
     *         error message suitable for presentation to the user.
     */
    static PDFOperationResult apply(PDFDocument* document,
                                    const PDFPageGeometrySettings& settings,
                                    PDFModifiedDocument::ModificationFlags* modificationFlags = nullptr);
};

}   // namespace pdf

#endif // PDFPAGEGEOMETRY_H
