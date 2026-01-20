/***************************************************************************
  qgsgooglemapsgeocoder.cpp
  ---------------
  Date                 : November 2020
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

#include "qgsgooglemapsgeocoder.h"

#include "qgsblockingnetworkrequest.h"
#include "qgscoordinatetransform.h"
#include "qgsgeocodercontext.h"
#include "qgslogger.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsreadwritelocker.h"
#include "qgssetrequestinitiator_p.h"

#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>

QReadWriteLock QgsGoogleMapsGeocoder::sMutex;

typedef QMap< QUrl, QList< QgsGeocoderResult > > CachedGeocodeResult;
Q_GLOBAL_STATIC( CachedGeocodeResult, sCachedResultsGM )


QgsGoogleMapsGeocoder::QgsGoogleMapsGeocoder( const QString &apiKey, const QString &regionBias )
  : QgsGeocoderInterface()
  , mApiKey( apiKey )
  , mRegion( regionBias )
  , mEndpoint( u"https://maps.googleapis.com/maps/api/geocode/json"_s )
{

}

QgsGeocoderInterface::Flags QgsGoogleMapsGeocoder::flags() const
{
  return QgsGeocoderInterface::Flag::GeocodesStrings;
}

QgsFields QgsGoogleMapsGeocoder::appendedFields() const
{
  QgsFields fields;
  fields.append( QgsField( u"location_type"_s, QMetaType::Type::QString ) );
  fields.append( QgsField( u"formatted_address"_s, QMetaType::Type::QString ) );
  fields.append( QgsField( u"place_id"_s, QMetaType::Type::QString ) );

  // add more?
  fields.append( QgsField( u"street_number"_s, QMetaType::Type::QString ) );
  fields.append( QgsField( u"route"_s, QMetaType::Type::QString ) );
  fields.append( QgsField( u"locality"_s, QMetaType::Type::QString ) );
  fields.append( QgsField( u"administrative_area_level_2"_s, QMetaType::Type::QString ) );
  fields.append( QgsField( u"administrative_area_level_1"_s, QMetaType::Type::QString ) );
  fields.append( QgsField( u"country"_s, QMetaType::Type::QString ) );
  fields.append( QgsField( u"postal_code"_s, QMetaType::Type::QString ) );
  return fields;
}

Qgis::WkbType QgsGoogleMapsGeocoder::wkbType() const
{
  return Qgis::WkbType::Point;
}

QList<QgsGeocoderResult> QgsGoogleMapsGeocoder::geocodeString( const QString &string, const QgsGeocoderContext &context, QgsFeedback *feedback ) const
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

  QgsReadWriteLocker locker( sMutex, QgsReadWriteLocker::Read );
  const auto it = sCachedResultsGM()->constFind( url );
  if ( it != sCachedResultsGM()->constEnd() )
  {
    return *it;
  }
  locker.unlock();

  QNetworkRequest request( url );
  QgsSetRequestInitiatorClass( request, u"QgsGoogleMapsGeocoder"_s );

  QgsBlockingNetworkRequest newReq;
  const QgsBlockingNetworkRequest::ErrorCode errorCode = newReq.get( request, false, feedback );
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
  const QVariantMap res = doc.object().toVariantMap();
  const QString status = res.value( u"status"_s ).toString();
  if ( status.isEmpty() || !res.contains( u"results"_s ) )
  {
    return QList<QgsGeocoderResult>();
  }

  if ( res.contains( "error_message"_L1 ) )
  {
    return QList<QgsGeocoderResult>() << QgsGeocoderResult::errorResult( res.value( u"error_message"_s ).toString() );
  }

  if ( status == "REQUEST_DENIED"_L1 || status == "OVER_QUERY_LIMIT"_L1 )
  {
    return QList<QgsGeocoderResult>() << QgsGeocoderResult::errorResult( QObject::tr( "Request denied -- the API key was rejected" ) );
  }
  if ( status != "OK"_L1 && status != "ZERO_RESULTS"_L1 )
  {
    return QList<QgsGeocoderResult>() << QgsGeocoderResult::errorResult( res.value( u"status"_s ).toString() );
  }

  // all good!
  locker.changeMode( QgsReadWriteLocker::Write );

  const QVariantList results = res.value( u"results"_s ).toList();
  if ( results.empty() )
  {
    sCachedResultsGM()->insert( url, QList<QgsGeocoderResult>() );
    return QList<QgsGeocoderResult>();
  }

  QList< QgsGeocoderResult > matches;
  matches.reserve( results.size( ) );
  for ( const QVariant &result : results )
  {
    matches << jsonToResult( result.toMap() );
  }
  sCachedResultsGM()->insert( url, matches );

  return matches;
}

QUrl QgsGoogleMapsGeocoder::requestUrl( const QString &address, const QgsRectangle &bounds ) const
{
  QUrl res( mEndpoint );
  QUrlQuery query;
  if ( !bounds.isNull() )
  {
    query.addQueryItem( u"bounds"_s, u"%1,%2|%3,%4"_s.arg( bounds.yMinimum() )
                        .arg( bounds.xMinimum() )
                        .arg( bounds.yMaximum() )
                        .arg( bounds.yMinimum() ) );
  }
  if ( !mRegion.isEmpty() )
  {
    query.addQueryItem( u"region"_s, mRegion.toLower() );
  }
  query.addQueryItem( u"sensor"_s, u"false"_s );
  query.addQueryItem( u"address"_s, address );
  query.addQueryItem( u"key"_s, mApiKey );
  res.setQuery( query );


  if ( res.toString().contains( "fake_qgis_http_endpoint"_L1 ) )
  {
    // Just for testing with local files instead of http:// resources
    QString modifiedUrlString = res.toString();
    // Qt5 does URL encoding from some reason (of the FILTER parameter for example)
    modifiedUrlString = QUrl::fromPercentEncoding( modifiedUrlString.toUtf8() );
    modifiedUrlString.replace( "fake_qgis_http_endpoint/"_L1, "fake_qgis_http_endpoint_"_L1 );
    QgsDebugMsgLevel( u"Get %1"_s.arg( modifiedUrlString ), 2 );
    modifiedUrlString = modifiedUrlString.mid( u"http://"_s.size() );
    QString args = modifiedUrlString.mid( modifiedUrlString.indexOf( '?' ) );
    if ( modifiedUrlString.size() > 150 )
    {
      args = QCryptographicHash::hash( args.toUtf8(), QCryptographicHash::Md5 ).toHex();
    }
    else
    {
      args.replace( "?"_L1, "_"_L1 );
      args.replace( "&"_L1, "_"_L1 );
      args.replace( "<"_L1, "_"_L1 );
      args.replace( ">"_L1, "_"_L1 );
      args.replace( "'"_L1, "_"_L1 );
      args.replace( "\""_L1, "_"_L1 );
      args.replace( " "_L1, "_"_L1 );
      args.replace( ":"_L1, "_"_L1 );
      args.replace( "/"_L1, "_"_L1 );
      args.replace( "\n"_L1, "_"_L1 );
    }
#ifdef Q_OS_WIN
    // Passing "urls" like "http://c:/path" to QUrl 'eats' the : after c,
    // so we must restore it
    if ( modifiedUrlString[1] == '/' )
    {
      modifiedUrlString = modifiedUrlString[0] + ":/" + modifiedUrlString.mid( 2 );
    }
#endif
    modifiedUrlString = modifiedUrlString.mid( 0, modifiedUrlString.indexOf( '?' ) ) + args;
    QgsDebugMsgLevel( u"Get %1 (after laundering)"_s.arg( modifiedUrlString ), 2 );
    res = QUrl::fromLocalFile( modifiedUrlString );
  }

  return res;
}

QgsGeocoderResult QgsGoogleMapsGeocoder::jsonToResult( const QVariantMap &json ) const
{
  const QVariantMap geometry = json.value( u"geometry"_s ).toMap();
  const QVariantMap location = geometry.value( u"location"_s ).toMap();
  const double latitude = location.value( u"lat"_s ).toDouble();
  const double longitude = location.value( u"lng"_s ).toDouble();

  const QgsGeometry geom = QgsGeometry::fromPointXY( QgsPointXY( longitude, latitude ) );

  QgsGeocoderResult res( json.value( u"formatted_address"_s ).toString(),
                         geom,
                         QgsCoordinateReferenceSystem( u"EPSG:4326"_s ) );

  QVariantMap attributes;

  if ( json.contains( u"formatted_address"_s ) )
    attributes.insert( u"formatted_address"_s, json.value( u"formatted_address"_s ).toString() );
  if ( json.contains( u"place_id"_s ) )
    attributes.insert( u"place_id"_s, json.value( u"place_id"_s ).toString() );
  if ( geometry.contains( u"location_type"_s ) )
    attributes.insert( u"location_type"_s, geometry.value( u"location_type"_s ).toString() );

  const QVariantList components = json.value( u"address_components"_s ).toList();
  for ( const QVariant &component : components )
  {
    const QVariantMap componentMap = component.toMap();
    const QStringList types = componentMap.value( u"types"_s ).toStringList();

    for ( const QString &t :
          {
            u"street_number"_s,
            u"route"_s,
            u"locality"_s,
            u"administrative_area_level_2"_s,
            u"administrative_area_level_1"_s,
            u"country"_s,
            u"postal_code"_s
          } )
    {
      if ( types.contains( t ) )
      {
        attributes.insert( t, componentMap.value( u"long_name"_s ).toString() );
        if ( t == "administrative_area_level_1"_L1 )
          res.setGroup( componentMap.value( u"long_name"_s ).toString() );
      }
    }
  }

  if ( geometry.contains( u"viewport"_s ) )
  {
    const QVariantMap viewport = geometry.value( u"viewport"_s ).toMap();
    const QVariantMap northEast = viewport.value( u"northeast"_s ).toMap();
    const QVariantMap southWest = viewport.value( u"southwest"_s ).toMap();
    res.setViewport( QgsRectangle( southWest.value( u"lng"_s ).toDouble(),
                                   southWest.value( u"lat"_s ).toDouble(),
                                   northEast.value( u"lng"_s ).toDouble(),
                                   northEast.value( u"lat"_s ).toDouble()
                                 ) );
  }

  res.setAdditionalAttributes( attributes );
  return res;
}

void QgsGoogleMapsGeocoder::setEndpoint( const QString &endpoint )
{
  mEndpoint = endpoint;
}

QString QgsGoogleMapsGeocoder::apiKey() const
{
  return mApiKey;
}

void QgsGoogleMapsGeocoder::setApiKey( const QString &apiKey )
{
  mApiKey = apiKey;
}

QString QgsGoogleMapsGeocoder::region() const
{
  return mRegion;
}

void QgsGoogleMapsGeocoder::setRegion( const QString &region )
{
  mRegion = region;
}
