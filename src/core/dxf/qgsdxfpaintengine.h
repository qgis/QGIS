/***************************************************************************
                         qgsdxpaintengine.h
                         ------------------
    begin                : November 2013
    copyright            : (C) 2013 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDXFPAINTENGINE_H
#define QGSDXFPAINTENGINE_H

#include <QPaintEngine>

class QgsDxfExport;
class QgsDxfPaintDevice;
class QgsPoint;

class CORE_EXPORT QgsDxfPaintEngine: public QPaintEngine
{
  public:
    QgsDxfPaintEngine( const QgsDxfPaintDevice* dxfDevice, QgsDxfExport* dxf );
    ~QgsDxfPaintEngine();

    bool begin( QPaintDevice* pdev );
    bool end();
    QPaintEngine::Type type() const;
    void updateState( const QPaintEngineState& state );

    void drawPixmap( const QRectF& r, const QPixmap& pm, const QRectF& sr );

    void drawPolygon( const QPointF * points, int pointCount, PolygonDrawMode mode );
    void drawRects( const QRectF * rects, int rectCount );
    void drawEllipse( const QRectF& rect );
    void drawPath( const QPainterPath& path );
    void drawLines( const QLineF* lines, int lineCount );

    void setLayer( const QString& layer ) { mLayer = layer; }
    QString layer() const { return mLayer; }

    void setShift( const QPointF& shift ) { mShift = shift; }

  private:
    const QgsDxfPaintDevice* mPaintDevice;
    QgsDxfExport* mDxf;

    //painter state information
    QTransform mTransform;
    QPen mPen;
    QString mLayer;
    QPointF mShift;

    QgsPoint toDxfCoordinates( const QPointF& pt ) const;
    int currentPenColor() const;
    double currentWidth() const;
};

#endif // QGSDXFPAINTENGINE_H
