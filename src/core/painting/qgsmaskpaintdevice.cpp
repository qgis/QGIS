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
#include "qgspainting.h"

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
  QPainterPath realPath = path;
  if ( mUsePathStroker )
  {
    QPen pen = painter()->pen();
    QPainterPathStroker stroker( pen );
    QPainterPath strokedPath = stroker.createStroke( path );
    realPath = strokedPath;
  }

  const QTransform transform = painter()->combinedTransform();
  mMaskPainterPath.addPath( transform.map( realPath ) );
}

void QgsMaskPaintEngine::drawPolygon( const QPointF *points, int numPoints, QPaintEngine::PolygonDrawMode mode )
{
  Q_UNUSED( mode );

  QPolygonF polygon;
  polygon.reserve( numPoints );
  for ( int i = 0; i < numPoints; ++i )
    polygon << points[i];

  const QTransform transform = painter()->transform();
  mMaskPainterPath.addPolygon( transform.map( polygon ) );
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
  switch ( m )
  {
    case PdmWidth:
      val = static_cast< int >( mPaintEngine->maskPainterPath().boundingRect().width() );
      break;
    case PdmHeight:
      val = static_cast< int >( mPaintEngine->maskPainterPath().boundingRect().height() );
      break;
    case PdmWidthMM:
      val = static_cast< int >( 25.4 / QgsPainting::qtDefaultDpiX() * mPaintEngine->maskPainterPath().boundingRect().width() );
      break;
    case PdmHeightMM:
      val = static_cast< int >( 25.4 / QgsPainting::qtDefaultDpiY() * mPaintEngine->maskPainterPath().boundingRect().height() );
      break;
    case PdmDpiX:
    case PdmPhysicalDpiX:
      val = QgsPainting::qtDefaultDpiX();
      break;
    case PdmDpiY:
    case PdmPhysicalDpiY:
      val = QgsPainting::qtDefaultDpiY();
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
      val = static_cast< int >( 1 * QPaintDevice::devicePixelRatioFScale() );
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
