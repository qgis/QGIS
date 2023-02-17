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

#include "qgsmaskpaintdevice.h"


Q_GUI_EXPORT extern int qt_defaultDpiX();
Q_GUI_EXPORT extern int qt_defaultDpiY();

///@cond PRIVATE

QgsMaskPaintEngine::QgsMaskPaintEngine( bool usePathStroker )
  : QPaintEngine( QPaintEngine::AllFeatures )
  , mUsePathStroker( usePathStroker )
{
}

QPainterPath QgsMaskPaintEngine::maskPainterPath() const
{
  return mMaskPainterPath;
}

void QgsMaskPaintEngine::drawPath( const QPainterPath &path )
{
  if ( mUsePathStroker )
  {
    QPen pen = painter()->pen();
    QPainterPathStroker stroker( pen );
    QPainterPath strokedPath = stroker.createStroke( path );
    strokedPath = painter()->combinedTransform().map( strokedPath );
    mMaskPainterPath.addPath( strokedPath );
  }
  else
  {
    mMaskPainterPath.addPath( path );
  }
}

void QgsMaskPaintEngine::drawPolygon( const QPointF *points, int numPoints, QPaintEngine::PolygonDrawMode mode )
{
  Q_UNUSED( mode );

  QPolygonF polygon;
  polygon.reserve( numPoints );
  for ( int i = 0; i < numPoints; ++i )
    polygon << points[i];
  mMaskPainterPath.addPolygon( polygon );
}

///@endcond

QgsMaskPaintDevice::QgsMaskPaintDevice( bool usePathStroker )
{
  mPaintEngine = std::make_unique<QgsMaskPaintEngine>( usePathStroker );
}

QPaintEngine *QgsMaskPaintDevice::paintEngine() const
{
  return mPaintEngine.get();
}

int QgsMaskPaintDevice::metric( PaintDeviceMetric m ) const
{
  // copy/paste from qpicture.cpp
  int val;
  QRectF brect = mPaintEngine->maskPainterPath().boundingRect();
  switch ( m )
  {
    case PdmWidth:
      val = brect.width();
      break;
    case PdmHeight:
      val = brect.height();
      break;
    case PdmWidthMM:
      val = int( 25.4 / qt_defaultDpiX() * brect.width() );
      break;
    case PdmHeightMM:
      val = int( 25.4 / qt_defaultDpiY() * brect.height() );
      break;
    case PdmDpiX:
    case PdmPhysicalDpiX:
      val = qt_defaultDpiX();
      break;
    case PdmDpiY:
    case PdmPhysicalDpiY:
      val = qt_defaultDpiY();
      break;
    case PdmNumColors:
      val = 16777216;
      break;
    case PdmDepth:
      val = 24;
      break;
    case PdmDevicePixelRatio:
      val = 1;
      break;
    case PdmDevicePixelRatioScaled:
      val = 1 * QPaintDevice::devicePixelRatioFScale();
      break;
    default:
      val = 0;
      qWarning( "QPicture::metric: Invalid metric command" );
  }
  return val;
}

QPainterPath QgsMaskPaintDevice::maskPainterPath() const
{
  return mPaintEngine->maskPainterPath();
}
