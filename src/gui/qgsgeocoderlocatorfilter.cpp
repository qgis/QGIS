/***************************************************************************
  qgsgeocoderlocatorfilter.cpp
  ---------------
  Date                 : August 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeocoderlocatorfilter.h"
#include "qgsmapcanvas.h"
#include "qgsmessagelog.h"
#include "qgsgeocoderresult.h"

QgsGeocoderLocatorFilter::QgsGeocoderLocatorFilter( const QString &name, const QString &displayName, const QString &prefix, QgsGeocoderInterface *geocoder, QgsMapCanvas *canvas, const QgsRectangle &boundingBox )
  : QgsAbstractGeocoderLocatorFilter( name, displayName, prefix, geocoder, boundingBox )
  , mCanvas( canvas )
{

}

QgsLocatorFilter *QgsGeocoderLocatorFilter::clone() const
{
  QgsLocatorFilter *filter = new QgsGeocoderLocatorFilter( name(), displayName(), prefix(), geocoder(), mCanvas );
  filter->setFetchResultsDelay( fetchResultsDelay() );
  return filter;
}

void QgsGeocoderLocatorFilter::handleGeocodeResult( const QgsGeocoderResult &result )
{
  const QgsCoordinateTransform ct( result.crs(), mCanvas->mapSettings().destinationCrs(), mCanvas->mapSettings().transformContext() );
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
      QgsCoordinateTransform extentTransform = ct;
      extentTransform.setBallparkTransformsAreAppropriate( true );
      bounds = extentTransform.transformBoundingBox( viewport );
    }
    mCanvas->zoomToFeatureExtent( bounds );

    mCanvas->flashGeometries( QList< QgsGeometry >() << g );
  }
  catch ( QgsCsException & )
  {
    QgsMessageLog::logMessage( tr( "Could not transform result to canvas CRS" ) );
  }
}

