/***************************************************************************
  qgsgeometrypaintdevice.h
  --------------------------------------
  Date                 : May 2024
  Copyright            : (C) 2024 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGEOMETRYPAINTDEVICE_H
#define QGSGEOMETRYPAINTDEVICE_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsgeometrycollection.h"
#include "qgsgeometry.h"

#include <QPainterPath>
#include <QPaintDevice>
#include <QPaintEngine>
#include <memory>

class QgsLineString;

#ifndef SIP_RUN

/**
 * \ingroup core
 * \brief A paint engine which converts everything renderer to a QgsGeometry representation of the rendered shapes.
 * \since QGIS 3.38
 */
class QgsGeometryPaintEngine: public QPaintEngine
{

  public:

    /**
     * Constructor for QgsGeometryPaintEngine.
     *
     * If \a usePathStroker is TRUE, rendered paths will be converted using a stroke respecting the QPainter
     * pen configuration.
     */
    QgsGeometryPaintEngine( bool usePathStroker = false );

    /**
     * Sets the number of \a segments to use when drawing stroked paths with a rounded pen.
     *
     * The default is 8 segments, a smaller number will result in simpler paths.
     */
    void setStrokedPathSegments( int segments );

    /**
     * Sets a simplification tolerance (in painter units) to use for on-the-fly simplification of geometries while rendering.
     *
     * This will result in simpler, generalised paths.
     *
     * Set \a tolerance to 0 to disable simplification. (No simplification is the default behavior).
     */
    void setSimplificationTolerance( double tolerance );

    bool begin( QPaintDevice * ) final;
    bool end() final;
    QPaintEngine::Type type() const final;
    void updateState( const QPaintEngineState & ) final;

    // no-op drawing methods. We don't want the base class methods to be called and unnecessary work performed
    void drawImage( const QRectF &rectangle, const QImage &image, const QRectF &sr, Qt::ImageConversionFlags flags = Qt::AutoColor ) final;
    void drawPixmap( const QRectF &, const QPixmap &, const QRectF & ) final;
    void drawTiledPixmap( const QRectF &rect, const QPixmap &pixmap, const QPointF &p ) final;

    // supported drawing operations. While we could rely on the base class methods to convert these operations
    // to paths and then only implement drawPath, we instead want this class to be as FAST as possible
    // and accordingly we implement the most optimised geometry conversion for each drawing primitive
    void drawLines( const QLineF *lines, int lineCount ) final;
    void drawLines( const QLine *lines, int lineCount ) final;
    void drawPoints( const QPointF *points, int pointCount ) final;
    void drawPoints( const QPoint *points, int pointCount ) final;
    void drawPolygon( const QPointF *points, int pointCount, QPaintEngine::PolygonDrawMode mode ) final;
    void drawPolygon( const QPoint *points, int pointCount, QPaintEngine::PolygonDrawMode mode ) final;
    void drawRects( const QRectF *rects, int rectCount ) final;
    void drawRects( const QRect *rects, int rectCount ) final;
    void drawPath( const QPainterPath &path ) final;

    // We can't represent an ellipse as a curved geometry, so let QPaintEngine base class convert this to a segmentized
    // shape instead
    // void drawEllipse(const QRectF &rect) final;

    // We want Qt to handle text -> path conversion
    // void drawTextItem(const QPointF &p, const QTextItem &textItem) final;

    /**
     * Returns the rendered geometry.
     */
    const QgsAbstractGeometry &geometry() const { return mGeometry; }

  private:

    void addSubpathGeometries( const QPainterPath &path, const QTransform &matrix );
    void addStrokedLine( const QgsLineString *line, double penWidth, Qgis::EndCapStyle endCapStyle, Qgis::JoinStyle joinStyle, double miterLimit, const QTransform *matrix );
    static Qgis::EndCapStyle penStyleToCapStyle( Qt::PenCapStyle style );
    static Qgis::JoinStyle penStyleToJoinStyle( Qt::PenJoinStyle style );

    bool mUsePathStroker = false;
    QPen mPen;
    QgsGeometryCollection mGeometry;
    int mStrokedPathsSegments = 8;
    double mSimplifyTolerance = 0;
};

#endif


/**
 * \ingroup core
 * \brief A paint device which converts everything renderer to a QgsGeometry representation of the rendered shapes.
 * \since QGIS 3.38
 */
class CORE_EXPORT QgsGeometryPaintDevice: public QPaintDevice
{

  public:

    /**
     * Constructor for QgsGeometryPaintDevice.
     *
     * If \a usePathStroker is TRUE, rendered paths will be converted using a stroke respecting the QPainter
     * pen configuration.
     */
    QgsGeometryPaintDevice( bool usePathStroker = false );

    /**
     * Sets the number of \a segments to use when drawing stroked paths with a rounded pen.
     *
     * The default is 8 segments, a smaller number will result in simpler paths.
     */
    void setStrokedPathSegments( int segments );

    /**
     * Sets a simplification tolerance (in painter units) to use for on-the-fly simplification of geometries while rendering.
     *
     * This will result in simpler, generalised paths.
     *
     * Set \a tolerance to 0 to disable simplification. (No simplification is the default behavior).
     */
    void setSimplificationTolerance( double tolerance );

    QPaintEngine *paintEngine() const override;

    int metric( PaintDeviceMetric metric ) const override;

    /**
     * Returns the rendered geometry.
     */
    const QgsAbstractGeometry &geometry() const;

    /**
     * Converts a painter \a path to a QgsGeometry.
     *
     * \since QGIS 3.38.1
     */
    static QgsGeometry painterPathToGeometry( const QPainterPath &path );

  private:

    std::unique_ptr<QgsGeometryPaintEngine> mPaintEngine;

    QSize mSize;

};


#endif // QGSGEOMETRYPAINTDEVICE_H
