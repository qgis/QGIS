/***************************************************************************
      qgssensorthingsshareddata.h
      ----------------
    begin                : November 2023
    copyright            : (C) 2013 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssensorthingsshareddata.h"
#include "qgssensorthingsprovider.h"
#include "qgssensorthingsutils.h"
#include "qgslogger.h"
#include "qgsreadwritelocker.h"
#include "qgsblockingnetworkrequest.h"
#include "qgsnetworkaccessmanager.h"

#include <QCryptographicHash>
#include <QFile>
#include <nlohmann/json.hpp>

///@cond PRIVATE

QgsSensorThingsSharedData::QgsSensorThingsSharedData( const QString &uri )
{
  const QVariantMap uriParts = QgsSensorThingsProviderMetadata().decodeUri( uri );

  mEntityType = qgsEnumKeyToValue( uriParts.value( QStringLiteral( "entity" ) ).toString(), Qgis::SensorThingsEntity::Invalid );
  mFields = QgsSensorThingsUtils::fieldsForEntityType( mEntityType );

  if ( QgsSensorThingsUtils::entityTypeHasGeometry( mEntityType ) )
  {
    const QString geometryType = uriParts.value( QStringLiteral( "geometryType" ) ).toString();
    if ( geometryType.compare( QLatin1String( "point" ), Qt::CaseInsensitive ) == 0 )
    {
      mGeometryType = Qgis::WkbType::PointZ;
    }
    else if ( geometryType.compare( QLatin1String( "multipoint" ), Qt::CaseInsensitive ) == 0 )
    {
      mGeometryType = Qgis::WkbType::MultiPointZ;
    }
    else if ( geometryType.compare( QLatin1String( "line" ), Qt::CaseInsensitive ) == 0 )
    {
      mGeometryType = Qgis::WkbType::MultiLineStringZ;
    }
    else if ( geometryType.compare( QLatin1String( "polygon" ), Qt::CaseInsensitive ) == 0 )
    {
      mGeometryType = Qgis::WkbType::MultiPolygonZ;
    }
    // geometry is always GeoJSON spec (for now, at least), so CRS will always be WGS84
    mSourceCRS = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) );
  }
  else
  {
    mGeometryType = Qgis::WkbType::NoGeometry;
  }

  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );
  mAuthCfg = dsUri.authConfigId();
  mHeaders = dsUri.httpHeaders();

  mRootUri = uriParts.value( QStringLiteral( "url" ) ).toString();
}

QUrl QgsSensorThingsSharedData::parseUrl( const QUrl &url, bool *isTestEndpoint )
{
  if ( isTestEndpoint )
    *isTestEndpoint = false;

  QUrl modifiedUrl( url );
  if ( modifiedUrl.toString().contains( QLatin1String( "fake_qgis_http_endpoint" ) ) )
  {
    if ( isTestEndpoint )
      *isTestEndpoint = true;

    // Just for testing with local files instead of http:// resources
    QString modifiedUrlString = modifiedUrl.toString();
    // Qt5 does URL encoding from some reason (of the FILTER parameter for example)
    modifiedUrlString = QUrl::fromPercentEncoding( modifiedUrlString.toUtf8() );
    modifiedUrlString.replace( QLatin1String( "fake_qgis_http_endpoint/" ), QLatin1String( "fake_qgis_http_endpoint_" ) );
    QgsDebugMsgLevel( QStringLiteral( "Get %1" ).arg( modifiedUrlString ), 2 );
    modifiedUrlString = modifiedUrlString.mid( QStringLiteral( "http://" ).size() );
    QString args = modifiedUrlString.indexOf( '?' ) >= 0 ? modifiedUrlString.mid( modifiedUrlString.indexOf( '?' ) ) : QString();
    if ( modifiedUrlString.size() > 150 )
    {
      args = QCryptographicHash::hash( args.toUtf8(), QCryptographicHash::Md5 ).toHex();
    }
    else
    {
      args.replace( QLatin1String( "?" ), QLatin1String( "_" ) );
      args.replace( QLatin1String( "&" ), QLatin1String( "_" ) );
      args.replace( QLatin1String( "$" ), QLatin1String( "_" ) );
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
    modifiedUrl = QUrl::fromLocalFile( modifiedUrlString );
    if ( !QFile::exists( modifiedUrlString ) )
    {
      QgsDebugError( QStringLiteral( "Local test file %1 for URL %2 does not exist!!!" ).arg( modifiedUrlString, url.toString() ) );
    }
  }

  return modifiedUrl;
}

long long QgsSensorThingsSharedData::featureCount( QgsFeedback *feedback ) const
{
  QgsReadWriteLocker locker( mReadWriteLock, QgsReadWriteLocker::Read );
  if ( mFeatureCount >= 0 )
    return mFeatureCount;

  locker.changeMode( QgsReadWriteLocker::Write );
  mError.clear();

  // return no features, just the total count
  const QUrl url = parseUrl( QUrl( QStringLiteral( "%1?$top=0&$count=true" ).arg( mEntityBaseUri ) ) );

  QNetworkRequest request( url );
  QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsArcGisRestUtils" ) );
  mHeaders.updateNetworkRequest( request );

  QgsBlockingNetworkRequest networkRequest;
  networkRequest.setAuthCfg( mAuthCfg );
  const QgsBlockingNetworkRequest::ErrorCode error = networkRequest.get( request, false, feedback );

  if ( feedback && feedback->isCanceled() )
    return mFeatureCount;

  // Handle network errors
  if ( error != QgsBlockingNetworkRequest::NoError )
  {
    QgsDebugError( QStringLiteral( "Network error: %1" ).arg( networkRequest.errorMessage() ) );
    mError = networkRequest.errorMessage();
  }
  else
  {
    const QgsNetworkReplyContent content = networkRequest.reply();
    try
    {
      auto rootContent = json::parse( content.content().toStdString() );
      if ( !rootContent.contains( "@iot.count" ) )
      {
        mError = QObject::tr( "No '@iot.count' value in response" );
        return mFeatureCount;
      }

      mFeatureCount = rootContent["@iot.count"].get<long long>();
    }
    catch ( const json::parse_error &ex )
    {
      mError = QObject::tr( "Error parsing response: %1" ).arg( ex.what() );
    }
  }

  return mFeatureCount;
}

bool QgsSensorThingsSharedData::hasCachedAllFeatures() const
{
  QgsReadWriteLocker locker( mReadWriteLock, QgsReadWriteLocker::Read );
  return mCachedFeatures.count() == featureCount();
}

bool QgsSensorThingsSharedData::getFeature( QgsFeatureId id, QgsFeature &f, QgsFeedback *feedback )
{
  QgsReadWriteLocker locker( mReadWriteLock, QgsReadWriteLocker::Read );

  // If cached, return cached feature
  QMap<QgsFeatureId, QgsFeature>::const_iterator it = mCachedFeatures.constFind( id );
  if ( it != mCachedFeatures.constEnd() )
  {
    f = it.value();
    return true;
  }

  const QString authcfg = mAuthCfg;
  bool featureFetched = false;
  long long startId = 0;
  QList<quint32> objectIds;

  // a potential optimisation? We could defer retrieval of total count and include it just in the first request for values?
  const long long totalFeatures = featureCount( feedback );
  if ( feedback && feedback->isCanceled() )
    return false;

  basic_json rootContent;

  while ( !featureFetched )
  {
    startId = ( id / mMaximumPageSize ) * mMaximumPageSize;
    const long long stopId = std::min< long long >( startId + mMaximumPageSize, totalFeatures );
    objectIds.clear();
    objectIds.reserve( stopId - startId );
    for ( size_t i = startId; i < stopId; ++i )
    {
      if ( i >= 0 && i < mObjectIds.count() && !mCachedFeatures.contains( i ) )
        objectIds.append( mObjectIds.at( i ) );
    }

    if ( objectIds.empty() )
    {
      QgsDebugMsgLevel( QStringLiteral( "No valid features IDs to fetch" ), 2 );
      return false;
    }

    // don't lock while doing the fetch
    locker.unlock();

    // query next features
    QString errorMessage;

    // TODO
    const QUrl url = parseUrl( QUrl( QStringLiteral( "%1?$top=%2&$count=false" ).arg( mEntityBaseUri ).arg( mMaximumPageSize ) ) );

    QNetworkRequest request( url );
    QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsArcGisRestUtils" ) );
    mHeaders.updateNetworkRequest( request );

    QgsBlockingNetworkRequest networkRequest;
    networkRequest.setAuthCfg( mAuthCfg );
    const QgsBlockingNetworkRequest::ErrorCode error = networkRequest.get( request, false, feedback );
    if ( feedback && feedback->isCanceled() )
    {
      return false;
    }

    if ( error != QgsBlockingNetworkRequest::NoError )
    {
      QgsDebugError( QStringLiteral( "Network error: %1" ).arg( networkRequest.errorMessage() ) );
      mError = networkRequest.errorMessage();
      if ( mMaximumPageSize <= 1 || errorMessage.isEmpty() )
      {
        QgsDebugMsgLevel( QStringLiteral( "Query returned empty result" ), 2 );
        return false;
      }
      else
      {
        locker.changeMode( QgsReadWriteLocker::Read );
        mMaximumPageSize = std::max( 1, mMaximumPageSize / 5 );
      }
    }
    else
    {
      const QgsNetworkReplyContent content = networkRequest.reply();
      try
      {
        rootContent = json::parse( content.content().toStdString() );
        if ( !rootContent.contains( "value" ) )
        {
          if ( mMaximumPageSize <= 1 || errorMessage.isEmpty() )
          {
            QgsDebugMsgLevel( QStringLiteral( "Query returned empty result" ), 2 );
            return false;
          }
          else
          {
            locker.changeMode( QgsReadWriteLocker::Read );
            mMaximumPageSize = std::max( 1, mMaximumPageSize / 5 );
          }
        }
        else
        {
          featureFetched = true;
        }
      }
      catch ( const json::parse_error &ex )
      {
        mError = QObject::tr( "Error parsing response: %1" ).arg( ex.what() );
        if ( mMaximumPageSize <= 1 || errorMessage.isEmpty() )
        {
          QgsDebugMsgLevel( QStringLiteral( "Query returned empty result" ), 2 );
          return false;
        }
        else
        {
          locker.changeMode( QgsReadWriteLocker::Read );
          mMaximumPageSize = std::max( 1, mMaximumPageSize / 5 );
        }
      }
    }
  }

  // re-lock while updating cache
  locker.changeMode( QgsReadWriteLocker::Write );

  const auto &values = rootContent["value"];
  if ( values.empty() )
  {
    QgsDebugMsgLevel( QStringLiteral( "Query returned no features" ), 3 );
    return false;
  }

  int i = 0;
  int n = featuresData.size();

  for ( const auto &featureData : values )
  {
    if ( i >= n )
      break;

    QgsFeature feature( mFields );
    QgsFeatureId featureId = startId + i;

    // Set attributes
    QgsAttributes attributes( mFields.size() );
    for ( int idx = 0; idx < mFields.size(); ++idx )
    {
#if 0
      QVariant attribute = attributesData[mFields.at( idx ).name()];
      if ( QgsVariantUtils::isNull( attribute ) )
      {
        // ensure that null values are mapped correctly for PyQGIS
        attribute = QVariant( QVariant::Int );
      }

      // date/datetime fields must be converted
      if ( mFields.at( idx ).type() == QVariant::DateTime || mFields.at( idx ).type() == QVariant::Date )
        attribute = QgsArcGisRestUtils::convertDateTime( attribute );

      if ( !mFields.at( idx ).convertCompatible( attribute ) )
      {
        QgsDebugError( QStringLiteral( "Invalid value %1 for field %2 of type %3" ).arg( attributesData[mFields.at( idx ).name()].toString(), mFields.at( idx ).name(), mFields.at( idx ).typeName() ) );
      }
      attributes[idx] = attribute;
      if ( mFields.at( idx ).name() == mObjectIdFieldName )
      {
        featureId = objectIdToFeatureId( attributesData[mFields.at( idx ).name()].toInt() );
      }
#endif
    }
    feature.setAttributes( attributes );

    // Set FID
    feature.setId( featureId );

    // Set geometry
#if 0
    const QVariantMap geometryData = featureData[QStringLiteral( "geometry" )].toMap();
    std::unique_ptr< QgsAbstractGeometry > geometry( QgsArcGisRestUtils::convertGeometry( geometryData, queryData[QStringLiteral( "geometryType" )].toString(),
        QgsWkbTypes::hasM( mGeometryType ), QgsWkbTypes::hasZ( mGeometryType ) ) );
    // Above might return 0, which is OK since in theory empty geometries are allowed
    if ( geometry )
      feature.setGeometry( QgsGeometry( std::move( geometry ) ) );
#endif

    mCachedFeatures.insert( feature.id(), feature );

    ++i;
  }

  // If added to cache, return feature
  it = mCachedFeatures.constFind( id );
  if ( it != mCachedFeatures.constEnd() )
  {
    f = it.value();
    return true;
  }

  return false;
}

QgsFeatureIds QgsSensorThingsSharedData::getFeatureIdsInExtent( const QgsRectangle &extent, QgsFeedback *feedback )
{
  QgsFeatureIds ids;
#if 0
  QString errorTitle;
  QString errorText;

  const QString authcfg = mDataSource.authConfigId();
  const QList<quint32> objectIdsInRect = QgsArcGisRestQueryUtils::getObjectIdsByExtent( mDataSource.param( QStringLiteral( "url" ) ),
                                         extent, errorTitle, errorText, authcfg, mDataSource.httpHeaders(), feedback, mDataSource.sql() );

  QgsReadWriteLocker locker( mReadWriteLock, QgsReadWriteLocker::Read );

  for ( const quint32 objectId : objectIdsInRect )
  {
    const QgsFeatureId featureId = objectIdToFeatureId( objectId );
    if ( featureId >= 0 )
      ids.insert( featureId );
  }
#endif
  return ids;
}

void QgsSensorThingsSharedData::clearCache()
{
  QgsReadWriteLocker locker( mReadWriteLock, QgsReadWriteLocker::Write );

  mFeatureCount = static_cast< long long >( Qgis::FeatureCountState::Uncounted );
  mCachedFeatures.clear();
  mIotIdToFeatureId.clear();
}

///@endcond PRIVATE
