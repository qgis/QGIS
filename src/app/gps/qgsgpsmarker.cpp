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
#include <QSvgRenderer>

#include "qgsgpsmarker.h"
#include "qgscoordinatetransform.h"
#include "qgsmapcanvas.h"
#include "qgsmaprenderer.h"


QgsGpsMarker::QgsGpsMarker( QgsMapCanvas* mapCanvas )
    : QgsMapCanvasItem( mapCanvas )
{
  mSize = 16;
  mWgs84CRS.createFromOgcWmsCrs( "EPSG:4326" );
}

void QgsGpsMarker::setSize( int theSize )
{
  mSize = theSize;
}

void QgsGpsMarker::setCenter( const QgsPoint& point )
{
  //transform to map crs
  if ( mMapCanvas && mMapCanvas->mapRenderer() )
  {
    QgsCoordinateTransform t( mWgs84CRS, mMapCanvas->mapRenderer()->destinationCrs() );
    try
    {
      mCenter = t.transform( point );
    }
    catch ( QgsCsException e ) //silently ignore transformation exceptions
    {
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

void QgsGpsMarker::paint( QPainter* p )
{
  QSvgRenderer mySVG;
  if ( !mySVG.load( QString( ":/images/north_arrows/gpsarrow2.svg" ) ) )
  {
    qDebug( "GPS marker not found!" );
    return;
  }
  float myHalfSize = mSize / 2.0;
  mySVG.render( p, QRectF( 0 - myHalfSize , 0 - myHalfSize, mSize, mSize ) );
}


QRectF QgsGpsMarker::boundingRect() const
{
  float myHalfSize = mSize / 2.0;
  return QRectF( -myHalfSize, -myHalfSize, 2.0 * myHalfSize, 2.0 * myHalfSize );
}

void QgsGpsMarker::updatePosition()
{
  setCenter( mCenter );
}
