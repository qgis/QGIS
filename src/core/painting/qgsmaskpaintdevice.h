/***************************************************************************
  qgsmaskpaintdevice.h
  --------------------------------------
  Date                 : February 2022
  Copyright            : (C) 2022 by Julien Cabieces
  Email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMASKPAINTDEVICE_H
#define QGSMASKPAINTDEVICE_H

#include "qgis_core.h"
#include "qgis_sip.h"

#include <QPainterPath>
#include <QPaintDevice>
#include <QPaintEngine>
#include <memory>

#ifndef SIP_RUN
///@cond PRIVATE
class QgsMaskPaintEngine: public QPaintEngine
{

  public:

    QgsMaskPaintEngine( bool usePathStroker = false );

    bool begin( QPaintDevice * ) override { return true; };
    bool end() override { return true; };
    QPaintEngine::Type type() const override { return QPaintEngine::User; };
    void updateState( const QPaintEngineState & ) override { return; };

    void drawPath( const QPainterPath & ) override;
    void drawPolygon( const QPointF *, int, PolygonDrawMode ) override;
    void drawPixmap( const QRectF &, const QPixmap &, const QRectF & ) override { return; };

    QPainterPath maskPainterPath() const;

  private:

    bool mUsePathStroker = false;
    QPainterPath mMaskPainterPath;

};
///@endcond
#endif


/**
 * \ingroup core
 * \brief Mask painter device that can be used to register everything painted into a QPainterPath
 * used later as clip path
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsMaskPaintDevice: public QPaintDevice
{

  public:

    /*!
     * Constructor
     * If \a usePathStroker is TRUE, path will be considered with a stroke regarding QPainter
     * pen configuration
     */
    QgsMaskPaintDevice( bool usePathStroker = false );

    QPaintEngine *paintEngine() const override;

    int metric( PaintDeviceMetric metric ) const override;

    /**
     * Returns the mask painter path painted on this paint device
     */
    QPainterPath maskPainterPath() const;

  private:

    std::unique_ptr<QgsMaskPaintEngine> mPaintEngine;

    QSize mSize;

};


#endif // QGSMASKPAINTDEVICE_H
