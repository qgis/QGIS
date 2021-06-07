/***************************************************************************
  qgsgeocoderlocatorfilter.cpp
  ---------------
  Date                 : August 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeocoderlocatorfilter.h"
#include "qgsmapcanvas.h"
#include "qgsmessagelog.h"
#include "qgsgeocoderresult.h"

QgsGeocoderLocatorFilter::QgsGeocoderLocatorFilter( const QString &name, const QString &displayName, const QString &prefix, QgsGeocoderInterface *geocoder, QgsMapCanvas *canvas )
  : QgsAbstractGeocoderLocatorFilter( name, displayName, prefix, geocoder )
  , mCanvas( canvas )
{

}

QgsLocatorFilter *QgsGeocoderLocatorFilter::clone() const
{
  return new QgsGeocoderLocatorFilter( name(), displayName(), prefix(), geocoder(), mCanvas );
}

void QgsGeocoderLocatorFilter::handleGeocodeResult( const QgsGeocoderResult &result )
{
  QgsCoordinateTransform ct( result.crs(), mCanvas->mapSettings().destinationCrs(), mCanvas->mapSettings().transformContext() );
  QgsGeometry g = result.geometry();
  const QgsRectangle viewport = result.viewport();
  try
  {
    QgsRectangle bounds;
    g.transform( ct );
    if ( viewport.isNull() )
    {
      bounds = g.boundingBox();
    }
    else
    {
      bounds = ct.transformBoundingBox( viewport );
    }
    mCanvas->zoomToFeatureExtent( bounds );

    mCanvas->flashGeometries( QList< QgsGeometry >() << g );
  }
  catch ( QgsCsException & )
  {
    QgsMessageLog::logMessage( tr( "Could not transform result to canvas CRS" ) );
  }
}

