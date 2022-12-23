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
#include "qgssymbollayerutils.h"
#include "qgsmarkersymbol.h"
#include "qgsgui.h"


QgsGpsMarker::QgsGpsMarker( QgsMapCanvas *mapCanvas )
  : QgsMapCanvasMarkerSymbolItem( mapCanvas )
{
  mWgs84CRS = QgsCoordinateReferenceSystem::fromOgcWmsCrs( QStringLiteral( "EPSG:4326" ) );
  updateMarkerSymbol();

  QObject::connect( QgsGui::instance(), &QgsGui::optionsChanged, this, &QgsGpsMarker::updateMarkerSymbol );

  setZValue( 200 );
}

QgsGpsMarker::~QgsGpsMarker() = default;

void QgsGpsMarker::setGpsPosition( const QgsPointXY &point )
{
  //transform to map crs
  if ( mMapCanvas )
  {
    const QgsCoordinateTransform t( mWgs84CRS, mMapCanvas->mapSettings().destinationCrs(), QgsProject::instance() );
    try
    {
      mCenter = t.transform( point );
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

  setPointLocation( mCenter );
  updateSize();
}

void QgsGpsMarker::setMarkerRotation( double rotation )
{
  QgsMarkerSymbol *renderedMarker = qgis::down_cast< QgsMarkerSymbol *>( symbol() );
  if ( !settingRotateLocationMarker.value( ) )
  {
    renderedMarker->setAngle( mMarkerSymbol->angle() );
  }
  else
  {
    renderedMarker->setAngle( mMarkerSymbol->angle() + rotation + mMapCanvas->rotation() );
  }
  updateSize();
}

void QgsGpsMarker::updateMarkerSymbol()
{
  const QString defaultSymbol = QgsGpsMarker::settingLocationMarkerSymbol.value();
  QDomDocument symbolDoc;
  symbolDoc.setContent( defaultSymbol );
  const QDomElement markerElement = symbolDoc.documentElement();
  mMarkerSymbol.reset( QgsSymbolLayerUtils::loadSymbol<QgsMarkerSymbol>( markerElement, QgsReadWriteContext() ) );
  setSymbol( std::unique_ptr< QgsMarkerSymbol >( mMarkerSymbol->clone() ) );
  updateSize();
}
