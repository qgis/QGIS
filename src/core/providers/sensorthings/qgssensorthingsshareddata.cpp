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
  mMaximumPageSize = uriParts.value( QStringLiteral( "pageSize" ), 1000 ).toInt();

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
  return mHasCachedAllFeatures;
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

  if ( mHasCachedAllFeatures )
    return false; // all features are cached, and we didn't find a match

  const QString authcfg = mAuthCfg;
  bool featureFetched = false;

  basic_json rootContent;

  // copy some members before we unlock the read/write locker
  QString nextPage = mNextPage;
  const QString baseUri = mEntityBaseUri;
  int maximumPageSize = mMaximumPageSize;
  const QgsFields fields = mFields;
  const QgsHttpHeaders headers = mHeaders;

  while ( !featureFetched )
  {
    // don't lock while doing the fetch
    locker.unlock();

    // query next features
    if ( nextPage.isEmpty() )
      nextPage = QStringLiteral( "%1?$top=%2&$count=false" ).arg( baseUri ).arg( maximumPageSize );

    // from: https://docs.ogc.org/is/18-088/18-088.html#nextLink
    // "SensorThings clients SHALL treat the URL of the nextLink as opaque, and SHALL NOT append system query options to the URL of a next link"
    //
    // ie don't mess with this URL!!
    const QUrl url = parseUrl( nextPage );

    QNetworkRequest request( url );
    QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsArcGisRestUtils" ) );
    headers.updateNetworkRequest( request );

    QgsBlockingNetworkRequest networkRequest;
    networkRequest.setAuthCfg( authcfg );
    const QgsBlockingNetworkRequest::ErrorCode error = networkRequest.get( request, false, feedback );
    if ( feedback && feedback->isCanceled() )
    {
      return false;
    }

    if ( error != QgsBlockingNetworkRequest::NoError )
    {
      QgsDebugError( QStringLiteral( "Network error: %1" ).arg( networkRequest.errorMessage() ) );
      locker.changeMode( QgsReadWriteLocker::Write );
      mError = networkRequest.errorMessage();
      QgsDebugMsgLevel( QStringLiteral( "Query returned empty result" ), 2 );
      return false;
    }
    else
    {
      const QgsNetworkReplyContent content = networkRequest.reply();
      try
      {
        rootContent = json::parse( content.content().toStdString() );
        if ( !rootContent.contains( "value" ) )
        {
          locker.changeMode( QgsReadWriteLocker::Write );
          mError = QObject::tr( "No 'value' in response" );
          QgsDebugMsgLevel( QStringLiteral( "No 'value' in response" ), 2 );
          return false;
        }
        else
        {
          // all good, got a batch of features
          const auto &values = rootContent["value"];
          if ( values.empty() )
          {
            locker.changeMode( QgsReadWriteLocker::Write );

            mNextPage.clear();
            mHasCachedAllFeatures = true;

            return false;
          }
          else
          {
            locker.changeMode( QgsReadWriteLocker::Write );
            for ( const auto &featureData : values )
            {
              QgsFeature feature( fields );
              feature.setId( mNextFeatureId++ );

              auto getString = []( const basic_json<> &json, const char *tag ) -> QVariant
              {
                if ( !json.contains( tag ) )
                  return QVariant();

                const auto &jObj = json[tag];
                if ( jObj.is_number_integer() )
                {
                  return QString::number( jObj.get<int>() );
                }
                else if ( jObj.is_number_unsigned() )
                {
                  return QString::number( jObj.get<unsigned>() );
                }
                else if ( jObj.is_boolean() )
                {
                  return QString::number( jObj.get<bool>() );
                }
                else if ( jObj.is_number_float() )
                {
                  return QString::number( jObj.get<double>() );
                }

                return QString::fromStdString( json[tag].get<std::string >() );
              };

              // Set attributes
              const QString iotId = getString( featureData, "@iot.id" ).toString();
              const QString selfLink = getString( featureData, "@iot.selfLink" ).toString();
              // TODO!
              const QVariant properties;
              switch ( mEntityType )
              {
                case Qgis::SensorThingsEntity::Invalid:
                  break;

                case Qgis::SensorThingsEntity::Thing:
                  feature.setAttributes(
                    QgsAttributes()
                    << iotId
                    << selfLink
                    << getString( featureData, "name" )
                    << getString( featureData, "description" )
                    << properties
                  );
                  break;

                case Qgis::SensorThingsEntity::Location:
                  feature.setAttributes(
                    QgsAttributes()
                    << iotId
                    << selfLink
                    << getString( featureData, "name" )
                    << getString( featureData, "description" )
                    << properties
                  );
                  break;

                case Qgis::SensorThingsEntity::HistoricalLocation:
                  feature.setAttributes(
                    QgsAttributes()
                    << iotId
                    << selfLink
                    << QVariant() // TODO -- datetime parsing
                  );
                  break;

                case Qgis::SensorThingsEntity::Datastream:
                  feature.setAttributes(
                    QgsAttributes()
                    << iotId
                    << selfLink
                    << getString( featureData, "name" )
                    << getString( featureData, "description" )
                    << QVariant() // TODO unitOfMeasurement
                    << getString( featureData, "observationType" )
                    << properties
                    << QVariant() // TODO -- datetime parsing
                    << QVariant() // TODO -- datetime parsing
                    << QVariant() // TODO -- datetime parsing
                    << QVariant() // TODO -- datetime parsing
                  );
                  break;

                case Qgis::SensorThingsEntity::Sensor:
                  feature.setAttributes(
                    QgsAttributes()
                    << iotId
                    << selfLink
                    << getString( featureData, "name" )
                    << getString( featureData, "description" )
                    << getString( featureData, "metadata" )
                    << properties
                  );
                  break;

                case Qgis::SensorThingsEntity::ObservedProperty:
                  feature.setAttributes(
                    QgsAttributes()
                    << iotId
                    << selfLink
                    << getString( featureData, "name" )
                    << getString( featureData, "definition" )
                    << getString( featureData, "description" )
                    << properties
                  );
                  break;

                case Qgis::SensorThingsEntity::Observation:
                  feature.setAttributes(
                    QgsAttributes()
                    << iotId
                    << selfLink
                    << QVariant() // TODO -- datetime parsing
                    << QVariant() // TODO -- datetime parsing
                    << QVariant() // TODO -- result type handling!
                    << QVariant() // TODO -- datetime parsing
                    << QVariant() // TODO -- list parsing
                    << QVariant() // TODO -- datetime parsing
                    << QVariant() // TODO -- datetime parsing
                    << QVariant() // TODO -- parameters parsing
                  );
                  break;

                case Qgis::SensorThingsEntity::FeatureOfInterest:
                  feature.setAttributes(
                    QgsAttributes()
                    << iotId
                    << selfLink
                    << getString( featureData, "name" )
                    << getString( featureData, "description" )
                    << properties
                  );
                  break;
              }

              // Set geometry
              if ( mGeometryType != Qgis::WkbType::NoGeometry )
              {
#if 0
                const QVariantMap geometryData = featureData[QStringLiteral( "geometry" )].toMap();
                std::unique_ptr< QgsAbstractGeometry > geometry( QgsArcGisRestUtils::convertGeometry( geometryData, queryData[QStringLiteral( "geometryType" )].toString(),
                    QgsWkbTypes::hasM( mGeometryType ), QgsWkbTypes::hasZ( mGeometryType ) ) );
                // Above might return 0, which is OK since in theory empty geometries are allowed
                if ( geometry )
                  feature.setGeometry( QgsGeometry( std::move( geometry ) ) );
#endif
              }

              mCachedFeatures.insert( feature.id(), feature );
              mIotIdToFeatureId.insert( iotId, feature.id() );

              if ( feature.id() == id )
              {
                f = feature;
                featureFetched = true;
                // don't break here -- store all the features we retrieved in this page first!
              }
            }

            if ( rootContent.contains( "@iot.nextLink" ) )
            {
              mNextPage = QString::fromStdString( rootContent["@iot.nextLink"].get<std::string>() );
            }
            else
            {
              mNextPage.clear();
              mHasCachedAllFeatures = true;
            }

            // if target feature was added to cache, return it
            if ( featureFetched )
            {
              return true;
            }
            else if ( mHasCachedAllFeatures )
            {
              return false;
            }
          }
        }
      }
      catch ( const json::parse_error &ex )
      {
        locker.changeMode( QgsReadWriteLocker::Write );
        mError = QObject::tr( "Error parsing response: %1" ).arg( ex.what() );
        QgsDebugMsgLevel( QStringLiteral( "Error parsing response: %1" ).arg( ex.what() ), 2 );
        return false;
      }
    }
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
