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
#include "qgsgeocodercontext.h"
#include "qgslogger.h"
#include "qgsnetworkaccessmanager.h"
#include "qgssetrequestinitiator_p.h"
#include "qgsblockingnetworkrequest.h"
#include "qgsreadwritelocker.h"
#include "qgscoordinatetransform.h"
#include <QUrl>
#include <QUrlQuery>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>

QReadWriteLock QgsGoogleMapsGeocoder::sMutex;

typedef QMap< QUrl, QList< QgsGeocoderResult > > CachedGeocodeResult;
Q_GLOBAL_STATIC( CachedGeocodeResult, sCachedResults )


QgsGoogleMapsGeocoder::QgsGoogleMapsGeocoder( const QString &apiKey, const QString &regionBias )
  : QgsGeocoderInterface()
  , mApiKey( apiKey )
  , mRegion( regionBias )
  , mEndpoint( QStringLiteral( "https://maps.googleapis.com/maps/api/geocode/json" ) )
{

}

QgsGeocoderInterface::Flags QgsGoogleMapsGeocoder::flags() const
{
  return QgsGeocoderInterface::Flag::GeocodesStrings;
}

QgsFields QgsGoogleMapsGeocoder::appendedFields() const
{
  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "location_type" ), QVariant::String ) );
  fields.append( QgsField( QStringLiteral( "formatted_address" ), QVariant::String ) );
  fields.append( QgsField( QStringLiteral( "place_id" ), QVariant::String ) );

  // add more?
  fields.append( QgsField( QStringLiteral( "street_number" ), QVariant::String ) );
  fields.append( QgsField( QStringLiteral( "route" ), QVariant::String ) );
  fields.append( QgsField( QStringLiteral( "locality" ), QVariant::String ) );
  fields.append( QgsField( QStringLiteral( "administrative_area_level_2" ), QVariant::String ) );
  fields.append( QgsField( QStringLiteral( "administrative_area_level_1" ), QVariant::String ) );
  fields.append( QgsField( QStringLiteral( "country" ), QVariant::String ) );
  fields.append( QgsField( QStringLiteral( "postal_code" ), QVariant::String ) );
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

  QgsReadWriteLocker locker( sMutex, QgsReadWriteLocker::Read );
  const auto it = sCachedResults()->constFind( url );
  if ( it != sCachedResults()->constEnd() )
  {
    return *it;
  }
  locker.unlock();

  QNetworkRequest request( url );
  QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsGoogleMapsGeocoder" ) );

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
  const QString status = res.value( QStringLiteral( "status" ) ).toString();
  if ( status.isEmpty() || !res.contains( QStringLiteral( "results" ) ) )
  {
    return QList<QgsGeocoderResult>();
  }

  if ( res.contains( QLatin1String( "error_message" ) ) )
  {
    return QList<QgsGeocoderResult>() << QgsGeocoderResult::errorResult( res.value( QStringLiteral( "error_message" ) ).toString() );
  }

  if ( status == QLatin1String( "REQUEST_DENIED" ) || status == QLatin1String( "OVER_QUERY_LIMIT" ) )
  {
    return QList<QgsGeocoderResult>() << QgsGeocoderResult::errorResult( QObject::tr( "Request denied -- the API key was rejected" ) );
  }
  if ( status != QLatin1String( "OK" ) && status != QLatin1String( "ZERO_RESULTS" ) )
  {
    return QList<QgsGeocoderResult>() << QgsGeocoderResult::errorResult( res.value( QStringLiteral( "status" ) ).toString() );
  }

  // all good!
  locker.changeMode( QgsReadWriteLocker::Write );

  const QVariantList results = res.value( QStringLiteral( "results" ) ).toList();
  if ( results.empty() )
  {
    sCachedResults()->insert( url, QList<QgsGeocoderResult>() );
    return QList<QgsGeocoderResult>();
  }

  QList< QgsGeocoderResult > matches;
  matches.reserve( results.size( ) );
  for ( const QVariant &result : results )
  {
    matches << jsonToResult( result.toMap() );
  }
  sCachedResults()->insert( url, matches );

  return matches;
}

QUrl QgsGoogleMapsGeocoder::requestUrl( const QString &address, const QgsRectangle &bounds ) const
{
  QUrl res( mEndpoint );
  QUrlQuery query;
  if ( !bounds.isNull() )
  {
    query.addQueryItem( QStringLiteral( "bounds" ), QStringLiteral( "%1,%2|%3,%4" ).arg( bounds.yMinimum() )
                        .arg( bounds.xMinimum() )
                        .arg( bounds.yMaximum() )
                        .arg( bounds.yMinimum() ) );
  }
  if ( !mRegion.isEmpty() )
  {
    query.addQueryItem( QStringLiteral( "region" ), mRegion.toLower() );
  }
  query.addQueryItem( QStringLiteral( "sensor" ), QStringLiteral( "false" ) );
  query.addQueryItem( QStringLiteral( "address" ), address );
  query.addQueryItem( QStringLiteral( "key" ), mApiKey );
  res.setQuery( query );


  if ( res.toString().contains( QLatin1String( "fake_qgis_http_endpoint" ) ) )
  {
    // Just for testing with local files instead of http:// resources
    QString modifiedUrlString = res.toString();
    // Qt5 does URL encoding from some reason (of the FILTER parameter for example)
    modifiedUrlString = QUrl::fromPercentEncoding( modifiedUrlString.toUtf8() );
    modifiedUrlString.replace( QLatin1String( "fake_qgis_http_endpoint/" ), QLatin1String( "fake_qgis_http_endpoint_" ) );
    QgsDebugMsgLevel( QStringLiteral( "Get %1" ).arg( modifiedUrlString ), 2 );
    modifiedUrlString = modifiedUrlString.mid( QStringLiteral( "http://" ).size() );
    QString args = modifiedUrlString.mid( modifiedUrlString.indexOf( '?' ) );
    if ( modifiedUrlString.size() > 150 )
    {
      args = QCryptographicHash::hash( args.toUtf8(), QCryptographicHash::Md5 ).toHex();
    }
    else
    {
      args.replace( QLatin1String( "?" ), QLatin1String( "_" ) );
      args.replace( QLatin1String( "&" ), QLatin1String( "_" ) );
      args.replace( QLatin1String( "<" ), QLatin1String( "_" ) );
      args.replace( QLatin1String( ">" ), QLatin1String( "_" ) );
      args.replace( QLatin1String( "'" ), QLatin1String( "_" ) );
      args.replace( QLatin1String( "\"" ), QLatin1String( "_" ) );
      args.replace( QLatin1String( " " ), QLatin1String( "_" ) );
      args.replace( QLatin1String( ":" ), QLatin1String( "_" ) );
      args.replace( QLatin1String( "/" ), QLatin1String( "_" ) );
      args.replace( QLatin1String( "\n" ), QLatin1String( "_" ) );
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
    QgsDebugMsgLevel( QStringLiteral( "Get %1 (after laundering)" ).arg( modifiedUrlString ), 2 );
    res = QUrl::fromLocalFile( modifiedUrlString );
  }

  return res;
}

QgsGeocoderResult QgsGoogleMapsGeocoder::jsonToResult( const QVariantMap &json ) const
{
  const QVariantMap geometry = json.value( QStringLiteral( "geometry" ) ).toMap();
  const QVariantMap location = geometry.value( QStringLiteral( "location" ) ).toMap();
  const double latitude = location.value( QStringLiteral( "lat" ) ).toDouble();
  const double longitude = location.value( QStringLiteral( "lng" ) ).toDouble();

  const QgsGeometry geom = QgsGeometry::fromPointXY( QgsPointXY( longitude, latitude ) );

  QgsGeocoderResult res( json.value( QStringLiteral( "formatted_address" ) ).toString(),
                         geom,
                         QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );

  QVariantMap attributes;

  if ( json.contains( QStringLiteral( "formatted_address" ) ) )
    attributes.insert( QStringLiteral( "formatted_address" ), json.value( QStringLiteral( "formatted_address" ) ).toString() );
  if ( json.contains( QStringLiteral( "place_id" ) ) )
    attributes.insert( QStringLiteral( "place_id" ), json.value( QStringLiteral( "place_id" ) ).toString() );
  if ( geometry.contains( QStringLiteral( "location_type" ) ) )
    attributes.insert( QStringLiteral( "location_type" ), geometry.value( QStringLiteral( "location_type" ) ).toString() );

  const QVariantList components = json.value( QStringLiteral( "address_components" ) ).toList();
  for ( const QVariant &component : components )
  {
    const QVariantMap componentMap = component.toMap();
    const QStringList types = componentMap.value( QStringLiteral( "types" ) ).toStringList();

    for ( const QString &t :
          {
            QStringLiteral( "street_number" ),
            QStringLiteral( "route" ),
            QStringLiteral( "locality" ),
            QStringLiteral( "administrative_area_level_2" ),
            QStringLiteral( "administrative_area_level_1" ),
            QStringLiteral( "country" ),
            QStringLiteral( "postal_code" )
          } )
    {
      if ( types.contains( t ) )
      {
        attributes.insert( t, componentMap.value( QStringLiteral( "long_name" ) ).toString() );
        if ( t == QLatin1String( "administrative_area_level_1" ) )
          res.setGroup( componentMap.value( QStringLiteral( "long_name" ) ).toString() );
      }
    }
  }

  if ( geometry.contains( QStringLiteral( "viewport" ) ) )
  {
    const QVariantMap viewport = geometry.value( QStringLiteral( "viewport" ) ).toMap();
    const QVariantMap northEast = viewport.value( QStringLiteral( "northeast" ) ).toMap();
    const QVariantMap southWest = viewport.value( QStringLiteral( "southwest" ) ).toMap();
    res.setViewport( QgsRectangle( southWest.value( QStringLiteral( "lng" ) ).toDouble(),
                                   southWest.value( QStringLiteral( "lat" ) ).toDouble(),
                                   northEast.value( QStringLiteral( "lng" ) ).toDouble(),
                                   northEast.value( QStringLiteral( "lat" ) ).toDouble()
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
