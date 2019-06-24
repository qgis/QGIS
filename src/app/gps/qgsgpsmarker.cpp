/***************************************************************************
    qgsgpsmarker.cpp  - canvas item which shows a gps position marker
    ---------------------
    begin                : December 2009
    copyright            : (C) 2009 by Tim Sutton
    email                : tim at linfiniti dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QPainter>
#include <QObject>

#include "qgsgpsmarker.h"
#include "qgscoordinatetransform.h"
#include "qgsmapcanvas.h"
#include "qgsexception.h"
#include "qgsproject.h"
#include "qgsmessagelog.h"


QgsGpsMarker::QgsGpsMarker( QgsMapCanvas *mapCanvas )
  : QgsMapCanvasItem( mapCanvas )
{
  mSize = 16;
  mWgs84CRS = QgsCoordinateReferenceSystem::fromOgcWmsCrs( QStringLiteral( "EPSG:4326" ) );
  mSvg.load( QStringLiteral( ":/images/north_arrows/gpsarrow2.svg" ) );
}

void QgsGpsMarker::setSize( int size )
{
  mSize = size;
}

void QgsGpsMarker::setCenter( const QgsPointXY &point )
{
  //transform to map crs
  if ( mMapCanvas )
  {
    QgsCoordinateTransform t( mWgs84CRS, mMapCanvas->mapSettings().destinationCrs(), QgsProject::instance() );
    try
    {
      mCenter = t.transform( point );
    }
    catch ( QgsCsException &e ) //silently ignore transformation exceptions
    {
      QgsMessageLog::logMessage( QObject::tr( "Error transforming the map center point: %1" ).arg( e.what() ), QStringLiteral( "GPS" ), Qgis::Warning );
      return;
    }
  }
  else
  {
    mCenter = point;
  }

  QPointF pt = toCanvasCoordinates( mCenter );
  setPos( pt );
}

void QgsGpsMarker::paint( QPainter *p )
{
  if ( ! mSvg.isValid() )
  {
    return;
  }

  // this needs to be done when the canvas is repainted to make for smoother map rendering
  // if not done the map could be panned, but the cursor position won't be updated until the next valid GPS fix is received
  QPointF pt = toCanvasCoordinates( mCenter );
  setPos( pt );

  double halfSize = mSize / 2.0;
  mSvg.render( p, QRectF( 0 - halfSize, 0 - halfSize, mSize, mSize ) );
}


QRectF QgsGpsMarker::boundingRect() const
{
  double halfSize = mSize / 2.0;
  return QRectF( -halfSize, -halfSize, 2.0 * halfSize, 2.0 * halfSize );
}

void QgsGpsMarker::updatePosition()
{
  setCenter( mCenter );
}
