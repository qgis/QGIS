/***************************************************************************
  qgsabstractgeocoderlocatorfilter.cpp
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

#include "qgsabstractgeocoderlocatorfilter.h"

#include "qgsgeocoder.h"
#include "qgsgeocodercontext.h"

#include "moc_qgsabstractgeocoderlocatorfilter.cpp"

QgsAbstractGeocoderLocatorFilter::QgsAbstractGeocoderLocatorFilter( const QString &name, const QString &displayName, const QString &prefix, QgsGeocoderInterface *geocoder, const QgsRectangle &boundingBox )
  : mName( name )
  , mDisplayName( displayName )
  , mPrefix( prefix )
  , mGeocoder( geocoder )
  , mBoundingBox( boundingBox )
{

}

QString QgsAbstractGeocoderLocatorFilter::name() const
{
  return mName;
}

QString QgsAbstractGeocoderLocatorFilter::displayName() const
{
  return mDisplayName;
}

QString QgsAbstractGeocoderLocatorFilter::prefix() const
{
  return mPrefix;
}

void QgsAbstractGeocoderLocatorFilter::fetchResults( const QString &string, const QgsLocatorContext &context, QgsFeedback *feedback )
{
  QgsGeocoderContext geocodeContext( context.transformContext );
  geocodeContext.setAreaOfInterest( QgsGeometry::fromRect( context.targetExtent ) );
  geocodeContext.setAreaOfInterestCrs( context.targetExtentCrs );

  const QList< QgsGeocoderResult > results = mGeocoder->geocodeString( string, geocodeContext, feedback );
  for ( const QgsGeocoderResult &result : results )
  {
    if ( result.isValid() )
    {
      const QgsLocatorResult locatorRes = geocoderResultToLocatorResult( result );
      emit resultFetched( locatorRes );
    }
  }
}

void QgsAbstractGeocoderLocatorFilter::triggerResult( const QgsLocatorResult &result )
{
  const QgsGeocoderResult geocodeResult = locatorResultToGeocoderResult( result );
  handleGeocodeResult( geocodeResult );
}

QgsGeocoderInterface *QgsAbstractGeocoderLocatorFilter::geocoder() const
{
  return mGeocoder;
}

QgsGeocoderResult QgsAbstractGeocoderLocatorFilter::locatorResultToGeocoderResult( const QgsLocatorResult &result ) const
{
  const QVariantMap attrs = result.userData().toMap();
  QgsGeocoderResult geocodeResult( attrs.value( u"identifier"_s ).toString(),
                                   attrs.value( u"geom"_s ).value< QgsGeometry >(),
                                   attrs.value( u"crs"_s ).value< QgsCoordinateReferenceSystem >() );
  geocodeResult.setAdditionalAttributes( attrs.value( u"attributes"_s ).toMap() );
  geocodeResult.setViewport( attrs.value( u"viewport"_s ).value< QgsRectangle >() );
  geocodeResult.setDescription( result.description );
  geocodeResult.setGroup( result.group );
  return geocodeResult;
}

QgsLocatorResult QgsAbstractGeocoderLocatorFilter::geocoderResultToLocatorResult( const QgsGeocoderResult &result )
{
  QVariantMap attrs;
  attrs.insert( u"identifier"_s, result.identifier() );
  attrs.insert( u"geom"_s, result.geometry() );
  attrs.insert( u"viewport"_s, result.viewport() );
  attrs.insert( u"crs"_s, result.crs() );
  attrs.insert( u"attributes"_s, result.additionalAttributes() );
  QgsLocatorResult res( this, result.identifier(), attrs );
  res.description = result.description();
  res.group = result.group();
  return res;
}
