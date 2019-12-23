/***************************************************************************
    qgsgpsbearingitem.cpp
    ---------------------
    begin                : December 2019
    copyright            : (C) 2019 Nyall Dawson
    email                : nyall dot dawson at gmail dot com

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

#include "qgsgpsbearingitem.h"
#include "qgscoordinatetransform.h"
#include "qgsmapcanvas.h"
#include "qgsexception.h"
#include "qgsproject.h"
#include "qgsmessagelog.h"
#include "qgssymbol.h"


QgsGpsBearingItem::QgsGpsBearingItem( QgsMapCanvas *mapCanvas )
  : QgsMapCanvasLineSymbolItem( mapCanvas )
{
  mSymbol->setColor( QColor( 255, 0, 0 ) );
  mWgs84CRS = QgsCoordinateReferenceSystem::fromOgcWmsCrs( QStringLiteral( "EPSG:4326" ) );

  setZValue( 199 );

  connect( mMapCanvas, &QgsMapCanvas::rotationChanged, this, &QgsGpsBearingItem::updateLine );
  connect( mMapCanvas, &QgsMapCanvas::extentsChanged, this, &QgsGpsBearingItem::updateLine );
}

void QgsGpsBearingItem::setGpsPosition( const QgsPointXY &point )
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
  updateLine();
}

void QgsGpsBearingItem::setGpsBearing( double bearing )
{
  mBearing = bearing;
  updateLine();
}

void QgsGpsBearingItem::updatePosition()
{
  setGpsPosition( mCenter );
}

void QgsGpsBearingItem::updateLine()
{
  QLineF bearingLine;
  bearingLine.setP1( toCanvasCoordinates( mCenter ) );
  bearingLine.setLength( 5 * std::max( mMapCanvas->width(), mMapCanvas->height() ) );
  bearingLine.setAngle( 90 - mBearing  - mMapCanvas->rotation() );
  setLine( bearingLine );
}
