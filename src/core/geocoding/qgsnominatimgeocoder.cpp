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
#include "qgscoordinatetransform.h"
#include "qgsgeocodercontext.h"
#include "qgslogger.h"
#include "qgsnetworkaccessmanager.h"
#include "qgssetrequestinitiator_p.h"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMutex>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>

QMutex QgsNominatimGeocoder::sMutex;
typedef QMap< QUrl, QList< QgsGeocoderResult > > CachedGeocodeResult;
Q_GLOBAL_STATIC( CachedGeocodeResult, sCachedResultsNominatim )
qint64 QgsNominatimGeocoder::sLastRequestTimestamp = 0;

QgsNominatimGeocoder::QgsNominatimGeocoder( const QString &countryCodes, const QString &endpoint )
  : QgsGeocoderInterface()
  , mCountryCodes( countryCodes )
  , mEndpoint( u"https://nominatim.qgis.org/search"_s )
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
  fields.append( QgsField( u"osm_type"_s, QMetaType::Type::QString ) );
  fields.append( QgsField( u"display_name"_s, QMetaType::Type::QString ) );
  fields.append( QgsField( u"place_id"_s, QMetaType::Type::QString ) );
  fields.append( QgsField( u"class"_s, QMetaType::Type::QString ) );
  fields.append( QgsField( u"type"_s, QMetaType::Type::QString ) );
  fields.append( QgsField( u"road"_s, QMetaType::Type::QString ) );
  fields.append( QgsField( u"village"_s, QMetaType::Type::QString ) );
  fields.append( QgsField( u"city_district"_s, QMetaType::Type::QString ) );
  fields.append( QgsField( u"town"_s, QMetaType::Type::QString ) );
  fields.append( QgsField( u"city"_s, QMetaType::Type::QString ) );
  fields.append( QgsField( u"state"_s, QMetaType::Type::QString ) );
  fields.append( QgsField( u"country"_s, QMetaType::Type::QString ) );
  fields.append( QgsField( u"postcode"_s, QMetaType::Type::QString ) );
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
    const QgsCoordinateTransform ct( context.areaOfInterestCrs(), QgsCoordinateReferenceSystem( u"EPSG:4326"_s ), context.transformContext() );
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
  const auto it = sCachedResultsNominatim()->constFind( url );
  if ( it != sCachedResultsNominatim()->constEnd() )
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
  QgsSetRequestInitiatorClass( request, u"QgsNominatimGeocoder"_s );

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
    sCachedResultsNominatim()->insert( url, QList<QgsGeocoderResult>() );
    return QList<QgsGeocoderResult>();
  }

  QList< QgsGeocoderResult > matches;
  matches.reserve( results.size() );
  for ( const QVariant &result : results )
  {
    matches << jsonToResult( result.toMap() );
  }

  sCachedResultsNominatim()->insert( url, matches );

  return matches;
}

QUrl QgsNominatimGeocoder::requestUrl( const QString &address, const QgsRectangle &bounds ) const
{
  QUrl res( mEndpoint );
  QUrlQuery query;
  query.addQueryItem( u"format"_s, u"json"_s );
  query.addQueryItem( u"addressdetails"_s, u"1"_s );
  if ( !bounds.isNull() && bounds.isFinite() )
  {
    query.addQueryItem( u"viewbox"_s, bounds.toString( 7 ).replace( " : "_L1, ","_L1 ) );
  }
  if ( !mCountryCodes.isEmpty() )
  {
    query.addQueryItem( u"countrycodes"_s, mCountryCodes.toLower() );
  }
  query.addQueryItem( u"q"_s, address );
  res.setQuery( query );

  return res;
}

QgsGeocoderResult QgsNominatimGeocoder::jsonToResult( const QVariantMap &json ) const
{
  const double latitude = json.value( u"lat"_s ).toDouble();
  const double longitude = json.value( u"lon"_s ).toDouble();

  const QgsGeometry geom = QgsGeometry::fromPointXY( QgsPointXY( longitude, latitude ) );

  QgsGeocoderResult res( json.value( u"display_name"_s ).toString(),
                         geom,
                         QgsCoordinateReferenceSystem( u"EPSG:4326"_s ) );

  QVariantMap attributes;

  if ( json.contains( u"display_name"_s ) )
    attributes.insert( u"display_name"_s, json.value( u"display_name"_s ).toString() );
  if ( json.contains( u"place_id"_s ) )
    attributes.insert( u"place_id"_s, json.value( u"place_id"_s ).toString() );
  if ( json.contains( u"osm_type"_s ) )
    attributes.insert( u"osm_type"_s, json.value( u"osm_type"_s ).toString() );
  if ( json.contains( u"class"_s ) )
    attributes.insert( u"class"_s, json.value( u"class"_s ).toString() );
  if ( json.contains( u"type"_s ) )
    attributes.insert( u"type"_s, json.value( u"type"_s ).toString() );

  if ( json.contains( u"address"_s ) )
  {
    const QVariantMap address_components = json.value( u"address"_s ).toMap();
    if ( address_components.contains( u"road"_s ) )
      attributes.insert( u"road"_s, address_components.value( u"road"_s ).toString() );
    if ( address_components.contains( u"village"_s ) )
      attributes.insert( u"village"_s, address_components.value( u"village"_s ).toString() );
    if ( address_components.contains( u"city_district"_s ) )
      attributes.insert( u"city_district"_s, address_components.value( u"city_district"_s ).toString() );
    if ( address_components.contains( u"town"_s ) )
      attributes.insert( u"town"_s, address_components.value( u"town"_s ).toString() );
    if ( address_components.contains( u"city"_s ) )
      attributes.insert( u"city"_s, address_components.value( u"city"_s ).toString() );
    if ( address_components.contains( u"state"_s ) )
    {
      attributes.insert( u"state"_s, address_components.value( u"state"_s ).toString() );
      res.setGroup( address_components.value( u"state"_s ).toString() );
    }
    if ( address_components.contains( u"country"_s ) )
      attributes.insert( u"country"_s, address_components.value( u"country"_s ).toString() );
    if ( address_components.contains( u"postcode"_s ) )
      attributes.insert( u"postcode"_s, address_components.value( u"postcode"_s ).toString() );
  }

  if ( json.contains( u"boundingbox"_s ) )
  {
    const QVariantList boundingBox = json.value( u"boundingbox"_s ).toList();
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
