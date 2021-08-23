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
#include "qgslogger.h"

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
  mCenterWGS84 = point;
  //transform to map crs
  if ( mMapCanvas )
  {
    const QgsCoordinateTransform t( mWgs84CRS, mMapCanvas->mapSettings().destinationCrs(), QgsProject::instance() );
    try
    {
      mCenter = t.transform( mCenterWGS84 );
    }
    catch ( QgsCsException &e ) //silently ignore transformation exceptions
    {
      QgsMessageLog::logMessage( QObject::tr( "Error transforming the map center point: %1" ).arg( e.what() ), QStringLiteral( "GPS" ), Qgis::MessageLevel::Warning );
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
  setGpsPosition( mCenterWGS84 );
}

void QgsGpsBearingItem::updateLine()
{
  QPolygonF bearingLine;

  const QgsCoordinateTransform wgs84ToCanvas( mWgs84CRS, mMapCanvas->mapSettings().destinationCrs(), QgsProject::instance()->transformContext() );

  try
  {
    bearingLine << mMapCanvas->getCoordinateTransform()->transform( wgs84ToCanvas.transform( mCenterWGS84 ) ).toQPointF();

    // project out the bearing line by roughly twice the size of the canvas
    QgsDistanceArea da1;
    da1.setSourceCrs( mMapCanvas->mapSettings().destinationCrs(), QgsProject::instance()->transformContext() );
    da1.setEllipsoid( QgsProject::instance()->ellipsoid() );
    const double totalLength = 2 * da1.measureLine( mMapCanvas->mapSettings().extent().center(), QgsPointXY( mMapCanvas->mapSettings().extent().xMaximum(),
                               mMapCanvas->mapSettings().extent().yMaximum() ) );

    QgsDistanceArea da;
    da.setSourceCrs( mWgs84CRS, QgsProject::instance()->transformContext() );
    da.setEllipsoid( QgsProject::instance()->ellipsoid() );
    // use 20 segments, projected from GPS position, in order to render a nice ellipsoid aware bearing line
    for ( int i = 1; i < 20; i++ )
    {
      const double distance = totalLength * i / 20;
      const QgsPointXY res = da.computeSpheroidProject( mCenterWGS84, distance, mBearing * M_PI / 180.0 );
      bearingLine << mMapCanvas->getCoordinateTransform()->transform( wgs84ToCanvas.transform( res ) ).toQPointF();
    }
  }
  catch ( QgsCsException & )
  {
    QgsDebugMsg( QStringLiteral( "Coordinate exception encountered while drawing GPS bearing line" ) );
    bearingLine.clear();
  }

  setLine( bearingLine );
}
