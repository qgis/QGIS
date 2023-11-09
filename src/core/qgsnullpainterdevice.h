/***************************************************************************
  qgsnullpainterdevice.h
  --------------------------------------
  Date                 : December 2021
  Copyright            : (C) 2013 by Mathieu Pellerin
  Email                : nirvn dot asia at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSNULLPAINTERDEVICE_H
#define QGSNULLPAINTERDEVICE_H

#include "qgis_core.h"
#include "qgis_sip.h"

#include <QPaintDevice>
#include <QPaintEngine>
#include <memory>


#ifndef SIP_RUN
///@cond PRIVATE
class QgsNullPaintEngine: public QPaintEngine
{

  public:

    QgsNullPaintEngine() : QPaintEngine( QPaintEngine::AllFeatures ) {};

    bool begin( QPaintDevice * ) override { return true; };
    bool end() override { return true; };
    QPaintEngine::Type type() const override { return QPaintEngine::User; };
    void updateState( const QPaintEngineState & ) override { return; };

    void drawRects( const QRect *, int ) override { return; };
    void drawRects( const QRectF *, int ) override { return; };
    void drawLines( const QLine *, int ) override { return; };
    void drawLines( const QLineF *, int ) override { return; };
    void drawEllipse( const QRectF & ) override { return; };
    void drawEllipse( const QRect & ) override { return; };
    void drawPath( const QPainterPath & ) override { return; };
    void drawPoints( const QPointF *, int ) override { return; };
    void drawPoints( const QPoint *, int ) override { return; };
    void drawPolygon( const QPointF *, int, PolygonDrawMode ) override { return; };
    void drawPolygon( const QPoint *, int, PolygonDrawMode ) override { return; };
    void drawPixmap( const QRectF &, const QPixmap &, const QRectF & ) override { return; };
    void drawTextItem( const QPointF &, const QTextItem & ) override { return; };
    void drawTiledPixmap( const QRectF &, const QPixmap &, const QPointF & ) override { return; };
    void drawImage( const QRectF &, const QImage &, const QRectF &, Qt::ImageConversionFlags ) override { return; };

};
///@endcond
#endif


/**
 * \ingroup core
 * \brief Null painter device that can be used for map renderer jobs which use custom painters.
 * \since QGIS 3.24
 */
class CORE_EXPORT QgsNullPaintDevice: public QPaintDevice
{

  public:

    QgsNullPaintDevice();

    QPaintEngine *paintEngine() const override;

    int metric( PaintDeviceMetric metric ) const override;

    /**
     * Sets the \a size of the device in pixels.
     */
    void setOutputSize( const QSize &size ) { mSize = size; };

    /**
     * Sets the \a dpi of the device.
     */
    void setOutputDpi( const int dpi ) { mDpi = dpi; };

  private:

    std::unique_ptr<QgsNullPaintEngine> mPaintEngine;

    QSize mSize;
    int mDpi = 96;

};


#endif // QGSNULLPAINTERDEVICE_H
