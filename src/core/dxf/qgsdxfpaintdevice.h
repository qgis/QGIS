/***************************************************************************
                         qgsdxpaintdevice.h
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

#ifndef QGSDXFPAINTDEVICE_H
#define QGSDXFPAINTDEVICE_H

#include <QPaintDevice>
#include "qgsdxfpaintengine.h"

class QgsDxfExport;
class QPaintEngine;

/**A paint device for drawing into dxf files*/

class CORE_EXPORT QgsDxfPaintDevice: public QPaintDevice
{
  public:
    QgsDxfPaintDevice( QgsDxfExport* dxf );
    ~QgsDxfPaintDevice();

    QPaintEngine* paintEngine() const;

    void setDrawingSize( const QSizeF& size ) { mDrawingSize = size; }
    void setOutputSize( const QRectF& r ) { mRectangle = r; }

    /**Returns scale factor for line width*/
    double widthScaleFactor() const;

    /**Converts a point from device coordinates to dxf coordinates*/
    QPointF dxfCoordinates( const QPointF& pt ) const;

    /*int height() const { return mDrawingSize.height(); }
    int width() const { return mDrawingSize.width(); }*/

    int metric( PaintDeviceMetric metric ) const;

    void setLayer( const QString& layer );

    void setShift( const QPointF& shift );


  private:
    QgsDxfPaintEngine* mPaintEngine;

    QSizeF mDrawingSize; //size (in source coordinates)
    QRectF mRectangle; //size (in dxf coordinates)
};

#endif // QGSDXFPAINTDEVICE_H
