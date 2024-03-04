/***************************************************************************
  qgsnominatimgeocoder.cpp
  ---------------
  Date                 : December 2020
  Copyright            : (C) 2020 by Mathieu Pellerin
  Email                : nirvn dot asia at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsnominatimgeocoder.h"
#include "qgsblockingnetworkrequest.h"
#include "qgsgeocodercontext.h"
#include "qgslogger.h"
#include "qgsnetworkaccessmanager.h"
#include "qgssetrequestinitiator_p.h"
#include "qgscoordinatetransform.h"
#include <QDateTime>
#include <QUrl>
#include <QUrlQuery>
#include <QMutex>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonArray>

QMutex QgsNominatimGeocoder::sMutex;
typedef QMap< QUrl, QList< QgsGeocoderResult > > CachedGeocodeResult;
Q_GLOBAL_STATIC( CachedGeocodeResult, sCachedResults )
qint64 QgsNominatimGeocoder::sLastRequestTimestamp = 0;

QgsNominatimGeocoder::QgsNominatimGeocoder( const QString &countryCodes, const QString &endpoint )
  : QgsGeocoderInterface()
  , mCountryCodes( countryCodes )
  , mEndpoint( QStringLiteral( "https://nominatim.qgis.org/search" ) )
{
  if ( !endpoint.isEmpty() )
    mEndpoint = endpoint;
}

QgsGeocoderInterface::Flags QgsNominatimGeocoder::flags() const
{
  return QgsGeocoderInterface::Flag::GeocodesStrings;
}

QgsFields QgsNominatimGeocoder::appendedFields() const
{
  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "osm_type" ), QVariant::String ) );
  fields.append( QgsField( QStringLiteral( "display_name" ), QVariant::String ) );
  fields.append( QgsField( QStringLiteral( "place_id" ), QVariant::String ) );
  fields.append( QgsField( QStringLiteral( "class" ), QVariant::String ) );
  fields.append( QgsField( QStringLiteral( "type" ), QVariant::String ) );
  fields.append( QgsField( QStringLiteral( "road" ), QVariant::String ) );
  fields.append( QgsField( QStringLiteral( "village" ), QVariant::String ) );
  fields.append( QgsField( QStringLiteral( "city_district" ), QVariant::String ) );
  fields.append( QgsField( QStringLiteral( "town" ), QVariant::String ) );
  fields.append( QgsField( QStringLiteral( "city" ), QVariant::String ) );
  fields.append( QgsField( QStringLiteral( "state" ), QVariant::String ) );
  fields.append( QgsField( QStringLiteral( "country" ), QVariant::String ) );
  fields.append( QgsField( QStringLiteral( "postcode" ), QVariant::String ) );
  return fields;
}

Qgis::WkbType QgsNominatimGeocoder::wkbType() const
{
  return Qgis::WkbType::Point;
}

QList<QgsGeocoderResult> QgsNominatimGeocoder::geocodeString( const QString &string, const QgsGeocoderContext &context, QgsFeedback *feedback ) const
{
  QgsRectangle bounds;
  if ( !context.areaOfInterest().isEmpty() )
  {
    QgsGeometry g = context.areaOfInterest();
    const QgsCoordinateTransform ct( context.areaOfInterestCrs(), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ), context.transformContext() );
    try
    {
      g.transform( ct );
      bounds = g.boundingBox();
    }
    catch ( QgsCsException & )
    {
      QgsDebugError( "Could not transform geocode bounds to WGS84" );
    }
  }

  const QUrl url = requestUrl( string, bounds );

  const QMutexLocker locker( &sMutex );
  const auto it = sCachedResults()->constFind( url );
  if ( it != sCachedResults()->constEnd() )
  {
    return *it;
  }

  while ( QDateTime::currentMSecsSinceEpoch() - sLastRequestTimestamp < 1000 / mRequestsPerSecond )
  {
    QThread::msleep( 50 );
    if ( feedback && feedback->isCanceled() )
      return QList<QgsGeocoderResult>();
  }

  QNetworkRequest request( url );
  QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsNominatimGeocoder" ) );

  QgsBlockingNetworkRequest newReq;
  const QgsBlockingNetworkRequest::ErrorCode errorCode = newReq.get( request, false, feedback );

  sLastRequestTimestamp = QDateTime::currentMSecsSinceEpoch();

  if ( errorCode != QgsBlockingNetworkRequest::NoError )
  {
    return QList<QgsGeocoderResult>() << QgsGeocoderResult::errorResult( newReq.errorMessage() );
  }

  // Parse data
  QJsonParseError err;
  const QJsonDocument doc = QJsonDocument::fromJson( newReq.reply().content(), &err );
  if ( doc.isNull() )
  {
    return QList<QgsGeocoderResult>() << QgsGeocoderResult::errorResult( err.errorString() );
  }

  const QVariantList results = doc.array().toVariantList();
  if ( results.isEmpty() )
  {
    sCachedResults()->insert( url, QList<QgsGeocoderResult>() );
    return QList<QgsGeocoderResult>();
  }

  QList< QgsGeocoderResult > matches;
  matches.reserve( results.size() );
  for ( const QVariant &result : results )
  {
    matches << jsonToResult( result.toMap() );
  }

  sCachedResults()->insert( url, matches );

  return matches;
}

QUrl QgsNominatimGeocoder::requestUrl( const QString &address, const QgsRectangle &bounds ) const
{
  QUrl res( mEndpoint );
  QUrlQuery query;
  query.addQueryItem( QStringLiteral( "format" ), QStringLiteral( "json" ) );
  query.addQueryItem( QStringLiteral( "addressdetails" ), QStringLiteral( "1" ) );
  if ( !bounds.isNull() && bounds.isFinite() )
  {
    query.addQueryItem( QStringLiteral( "viewbox" ), bounds.toString( 7 ).replace( QLatin1String( " : " ), QLatin1String( "," ) ) );
  }
  if ( !mCountryCodes.isEmpty() )
  {
    query.addQueryItem( QStringLiteral( "countrycodes" ), mCountryCodes.toLower() );
  }
  query.addQueryItem( QStringLiteral( "q" ), address );
  res.setQuery( query );

  return res;
}

QgsGeocoderResult QgsNominatimGeocoder::jsonToResult( const QVariantMap &json ) const
{
  const double latitude = json.value( QStringLiteral( "lat" ) ).toDouble();
  const double longitude = json.value( QStringLiteral( "lon" ) ).toDouble();

  const QgsGeometry geom = QgsGeometry::fromPointXY( QgsPointXY( longitude, latitude ) );

  QgsGeocoderResult res( json.value( QStringLiteral( "display_name" ) ).toString(),
                         geom,
                         QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );

  QVariantMap attributes;

  if ( json.contains( QStringLiteral( "display_name" ) ) )
    attributes.insert( QStringLiteral( "display_name" ), json.value( QStringLiteral( "display_name" ) ).toString() );
  if ( json.contains( QStringLiteral( "place_id" ) ) )
    attributes.insert( QStringLiteral( "place_id" ), json.value( QStringLiteral( "place_id" ) ).toString() );
  if ( json.contains( QStringLiteral( "osm_type" ) ) )
    attributes.insert( QStringLiteral( "osm_type" ), json.value( QStringLiteral( "osm_type" ) ).toString() );
  if ( json.contains( QStringLiteral( "class" ) ) )
    attributes.insert( QStringLiteral( "class" ), json.value( QStringLiteral( "class" ) ).toString() );
  if ( json.contains( QStringLiteral( "type" ) ) )
    attributes.insert( QStringLiteral( "type" ), json.value( QStringLiteral( "type" ) ).toString() );

  if ( json.contains( QStringLiteral( "address" ) ) )
  {
    const QVariantMap address_components = json.value( QStringLiteral( "address" ) ).toMap();
    if ( address_components.contains( QStringLiteral( "road" ) ) )
      attributes.insert( QStringLiteral( "road" ), address_components.value( QStringLiteral( "road" ) ).toString() );
    if ( address_components.contains( QStringLiteral( "village" ) ) )
      attributes.insert( QStringLiteral( "village" ), address_components.value( QStringLiteral( "village" ) ).toString() );
    if ( address_components.contains( QStringLiteral( "city_district" ) ) )
      attributes.insert( QStringLiteral( "city_district" ), address_components.value( QStringLiteral( "city_district" ) ).toString() );
    if ( address_components.contains( QStringLiteral( "town" ) ) )
      attributes.insert( QStringLiteral( "town" ), address_components.value( QStringLiteral( "town" ) ).toString() );
    if ( address_components.contains( QStringLiteral( "city" ) ) )
      attributes.insert( QStringLiteral( "city" ), address_components.value( QStringLiteral( "city" ) ).toString() );
    if ( address_components.contains( QStringLiteral( "state" ) ) )
    {
      attributes.insert( QStringLiteral( "state" ), address_components.value( QStringLiteral( "state" ) ).toString() );
      res.setGroup( address_components.value( QStringLiteral( "state" ) ).toString() );
    }
    if ( address_components.contains( QStringLiteral( "country" ) ) )
      attributes.insert( QStringLiteral( "country" ), address_components.value( QStringLiteral( "country" ) ).toString() );
    if ( address_components.contains( QStringLiteral( "postcode" ) ) )
      attributes.insert( QStringLiteral( "postcode" ), address_components.value( QStringLiteral( "postcode" ) ).toString() );
  }

  if ( json.contains( QStringLiteral( "boundingbox" ) ) )
  {
    const QVariantList boundingBox = json.value( QStringLiteral( "boundingbox" ) ).toList();
    if ( boundingBox.size() == 4 )
      res.setViewport( QgsRectangle( boundingBox.at( 2 ).toDouble(),
                                     boundingBox.at( 0 ).toDouble(),
                                     boundingBox.at( 3 ).toDouble(),
                                     boundingBox.at( 1 ).toDouble() ) );
  }

  res.setAdditionalAttributes( attributes );
  return res;
}

QString QgsNominatimGeocoder::endpoint() const
{
  return mEndpoint;
}

void QgsNominatimGeocoder::setEndpoint( const QString &endpoint )
{
  mEndpoint = endpoint;
}

QString QgsNominatimGeocoder::countryCodes() const
{
  return mCountryCodes;
}

void QgsNominatimGeocoder::setCountryCodes( const QString &countryCodes )
{
  mCountryCodes = countryCodes;
}
