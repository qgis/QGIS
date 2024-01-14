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

#ifndef PDFSNAPPER_H
#define PDFSNAPPER_H

#include "pdfglobal.h"

#include <QImage>
#include <QPainterPath>

#include <array>
#include <optional>

class QPainter;

namespace pdf
{
class PDFPrecompiledPage;
struct PDFWidgetSnapshot;

enum class SnapType
{
    Invalid,
    PageCorner,        ///< Corner of the page media box
    ImageCorner,       ///< Corner of image
    PageCenter,        ///< Center of page media box
    ImageCenter,       ///< Center of image
    LineCenter,        ///< Center of line
    GeneratedLineProjection,   ///< Generated point to line projections
    Custom  ///< Custom snap point
};

/// Contain informations for snap points in the pdf page. Snap points
/// can be for example image centers, rectangle corners, line start/end
/// points, page boundary boxes etc. All coordinates are in page coordinates.
class PDFSnapInfo
{
public:
    explicit inline PDFSnapInfo() = default;

    struct SnapPoint
    {
        explicit inline constexpr SnapPoint() = default;
        explicit inline constexpr SnapPoint(SnapType type, QPointF point) :
            type(type),
            point(point)
        {

        }

        SnapType type = SnapType::Invalid;
        QPointF point;
    };

    struct SnapImage
    {
        QPainterPath imagePath;
        QImage image;
    };

    /// Adds page media box. Media box must be in page coordinates.
    /// \param mediaBox Media box
    void addPageMediaBox(const QRectF& mediaBox);

    /// Adds image box. Because it is not guaranteed, that it will be rectangle, five
    /// points are defined - four corners and center.
    /// \param points Four corner points in clockwise order, fifth point is center,
    ///        all in page coordinates.
    /// \param image Image
    void addImage(const std::array<QPointF, 5>& points, const QImage& image);

    /// Adds line and line center points
    /// \param start Start point of line, in page coordinates
    /// \param end End point of line, in page coordinates
    void addLine(const QPointF& start, const QPointF& end);

    /// Returns snap points
    const std::vector<SnapPoint>& getSnapPoints() const { return m_snapPoints; }

    /// Returns lines
    const std::vector<QLineF>& getLines() const { return m_snapLines; }

    /// Returns snap images (together with painter path in page coordinates,
    /// in which image is painted).
    const std::vector<SnapImage>& getSnapImages() const { return m_snapImages; }

private:
    std::vector<SnapPoint> m_snapPoints;
    std::vector<QLineF> m_snapLines;
    std::vector<SnapImage> m_snapImages;
};

/// Snap engine, which handles snapping of points on the page.
class PDF4QTLIBCORESHARED_EXPORT PDFSnapper
{
public:
    PDFSnapper();

    struct ViewportSnapPoint : public PDFSnapInfo::SnapPoint
    {
        QPointF viewportPoint;
        PDFInteger pageIndex;
    };

    struct ViewportSnapImage : public PDFSnapInfo::SnapImage
    {
        PDFInteger pageIndex;
        QPainterPath viewportPath;
    };

    /// Sets snap point pixel size
    /// \param snapPointPixelSize Snap point diameter in pixels
    void setSnapPointPixelSize(int snapPointPixelSize) { m_snapPointPixelSize = snapPointPixelSize; }

    /// Returns snap point pixel size
    int getSnapPointPixelSize() const { return m_snapPointPixelSize; }

    /// Draws snapping points onto the painter. This function needs valid snap points,
    /// so \p m_snapPoints must be valid before this function is called.
    /// \param painter Painter
    void drawSnapPoints(QPainter* painter) const;

    /// Returns true, if snapping is allowed at current page
    /// \param pageIndex Page index to be tested for allowing snapping
    bool isSnappingAllowed(PDFInteger pageIndex) const;

    /// Updates snapped point/image using given information. If current page is set, it means, we are
    /// using snapping info from current page, and if we are hovering at different page,
    /// then nothing happens. Otherwise, other page snap info is used to update snapped point.
    /// If mouse point distance from some snap point is lesser than tolerance, then new snap is set.
    /// \param mousePoint Mouse point in widget coordinates
    void updateSnappedPoint(const QPointF& mousePoint);

    /// Returns true, if some point of interest is snapped
    bool isSnapped() const { return m_snappedPoint.has_value(); }

    /// Builds snap points from the widget snapshot. Updates current value
    /// of snapped point (from mouse position).
    /// \param snapshot Widget snapshot
    void buildSnapPoints(const PDFWidgetSnapshot& snapshot);

    /// Builds snap images from the widget snapshot.
    /// \param snapshot Widget snapshot
    void buildSnapImages(const PDFWidgetSnapshot& snapshot);

    /// Returns current snap point tolerance (while aiming with the mouse cursor,
    /// when mouse cursor is at most tolerance distance from some snap point,
    /// it is snapped.
    int getSnapPointTolerance() const;

    /// Sets snap point tolerance
    void setSnapPointTolerance(int snapPointTolerance);

    /// Returns snapped position. If point at mouse cursor is not snapped, then
    /// mouse cursor position is returned.
    QPointF getSnappedPoint() const;

    /// Returns snapped image. If no image is present at mouse cursor, then
    /// nullptr is returned.
    const ViewportSnapImage* getSnappedImage() const;

    /// Sets current page index and reference point
    /// \param pageIndex Page index
    /// \param pagePoint Page point
    void setReferencePoint(PDFInteger pageIndex, QPointF pagePoint);

    /// Resets reference point (and current page)
    void clearReferencePoint();

    /// Clears all snapped data, including snap points, images and referenced data.
    void clear();

    /// Returns a vector of custom snap points
    const std::vector<QPointF>& getCustomSnapPoints() const;

    /// Sets custom set of snap points. Snap points are always referred to current
    /// page index. If page index changes, then custom snap points are cleared.
    /// \param customSnapPoints Custom snap points
    void setCustomSnapPoints(const std::vector<QPointF>& customSnapPoints);

private:
    struct SnappedPoint
    {
        PDFInteger pageIndex = -1;
        QPointF snappedPoint;
    };

    std::vector<ViewportSnapPoint> m_snapPoints;
    std::vector<ViewportSnapImage> m_snapImages;
    std::vector<QPointF> m_customSnapPoints;
    std::optional<ViewportSnapPoint> m_snappedPoint;
    std::optional<ViewportSnapImage> m_snappedImage;
    QPointF m_mousePoint;
    std::optional<QPointF> m_referencePoint;
    PDFInteger m_currentPage = -1;
    int m_snapPointPixelSize = 0;
    int m_snapPointTolerance = 0;
};

}   // namespace pdf

#endif // PDFSNAPPER_H
