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
#include "qgssetrequestinitiator_p.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsjsonutils.h"

#include <QCryptographicHash>
#include <QFile>
#include <nlohmann/json.hpp>

///@cond PRIVATE

QgsSensorThingsSharedData::QgsSensorThingsSharedData( const QString &uri )
{
  const QVariantMap uriParts = QgsSensorThingsProviderMetadata().decodeUri( uri );

  mEntityType = qgsEnumKeyToValue( uriParts.value( QStringLiteral( "entity" ) ).toString(), Qgis::SensorThingsEntity::Invalid );
  const QVariantList expandTo = uriParts.value( QStringLiteral( "expandTo" ) ).toList();
  QList< Qgis::SensorThingsEntity > expandedEntities;
  for ( const QVariant &expansionVariant : expandTo )
  {
    const QgsSensorThingsExpansionDefinition expansion = expansionVariant.value< QgsSensorThingsExpansionDefinition >();
    if ( expansion.isValid() )
    {
      mExpansions.append( expansion );
      expandedEntities.append( expansion.childEntity() );
    }

    mExpandQueryString = QgsSensorThingsUtils::asQueryString( mEntityType, mExpansions );
  }

  mFields = QgsSensorThingsUtils::fieldsForExpandedEntityType( mEntityType, expandedEntities );

  mGeometryField = QgsSensorThingsUtils::geometryFieldForEntityType( mEntityType );
  // use initial value of maximum page size as default
  mMaximumPageSize = uriParts.value( QStringLiteral( "pageSize" ), mMaximumPageSize ).toInt();
  // will default to 0 if not specified, i.e. no limit
  mFeatureLimit = uriParts.value( QStringLiteral( "featureLimit" ) ).toInt();
  mFilterExtent = uriParts.value( QStringLiteral( "bounds" ) ).value< QgsRectangle >();
  mSubsetString = uriParts.value( QStringLiteral( "sql" ) ).toString();

  if ( QgsSensorThingsUtils::entityTypeHasGeometry( mEntityType ) )
  {
    if ( uriParts.contains( QStringLiteral( "geometryType" ) ) )
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

      if ( mGeometryType != Qgis::WkbType::NoGeometry )
      {
        // geometry is always GeoJSON spec (for now, at least), so CRS will always be WGS84
        mSourceCRS = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) );
      }
    }
    else
    {
      mGeometryType = Qgis::WkbType::NoGeometry;
    }
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

QgsRectangle QgsSensorThingsSharedData::extent() const
{
  QgsReadWriteLocker locker( mReadWriteLock, QgsReadWriteLocker::Read );

  // Since we can't retrieve the actual layer extent via SensorThings API, we use a pessimistic
  // global extent until we've retrieved all the features from the layer
  return hasCachedAllFeatures() ? mFetchedFeatureExtent
         : ( !mFilterExtent.isNull() ? mFilterExtent : QgsRectangle( -180, -90, 180, 90 ) );
}

long long QgsSensorThingsSharedData::featureCount( QgsFeedback *feedback ) const
{
  QgsReadWriteLocker locker( mReadWriteLock, QgsReadWriteLocker::Read );
  if ( mFeatureCount >= 0 )
    return mFeatureCount;

  locker.changeMode( QgsReadWriteLocker::Write );
  mError.clear();

  // MISSING PART -- how to handle feature count when we are expanding features?
  // This situation is not handled by the SensorThings standard at all, so we'll just have
  // to return an unknown count whenever expansion is used
  if ( !mExpansions.isEmpty() )
  {
    return static_cast< long long >( Qgis::FeatureCountState::UnknownCount );
  }

  // return no features, just the total count
  QString countUri = QStringLiteral( "%1?$top=0&$count=true" ).arg( mEntityBaseUri );
  const QString typeFilter = QgsSensorThingsUtils::filterForWkbType( mEntityType, mGeometryType );
  const QString extentFilter = QgsSensorThingsUtils::filterForExtent( mGeometryField, mFilterExtent );
  QString filterString = QgsSensorThingsUtils::combineFilters( { typeFilter, extentFilter, mSubsetString } );
  if ( !filterString.isEmpty() )
    filterString = QStringLiteral( "&$filter=" ) + filterString;
  if ( !filterString.isEmpty() )
    countUri += filterString;

  const QUrl url = parseUrl( QUrl( countUri ) );

  QNetworkRequest request( url );
  QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsSensorThingsSharedData" ) );
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
      if ( mFeatureLimit > 0 && mFeatureCount > mFeatureLimit )
        mFeatureCount = mFeatureLimit;
    }
    catch ( const json::parse_error &ex )
    {
      mError = QObject::tr( "Error parsing response: %1" ).arg( ex.what() );
    }
  }

  return mFeatureCount;
}

QString QgsSensorThingsSharedData::subsetString() const
{
  return mSubsetString;
}

bool QgsSensorThingsSharedData::hasCachedAllFeatures() const
{
  QgsReadWriteLocker locker( mReadWriteLock, QgsReadWriteLocker::Read );
  return mHasCachedAllFeatures
         || ( mFeatureCount > 0 && mCachedFeatures.size() == mFeatureCount )
         || ( mFeatureLimit > 0 && mRetrievedBaseFeatureCount >= mFeatureLimit );
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

  if ( hasCachedAllFeatures() )
    return false; // all features are cached, and we didn't find a match

  bool featureFetched = false;

  if ( mNextPage.isEmpty() )
  {
    locker.changeMode( QgsReadWriteLocker::Write );

    int thisPageSize = mMaximumPageSize;
    if ( mFeatureLimit > 0 && ( mCachedFeatures.size() + thisPageSize ) > mFeatureLimit )
      thisPageSize = mFeatureLimit - mCachedFeatures.size();

    mNextPage = QStringLiteral( "%1?$top=%2&$count=false%3" ).arg( mEntityBaseUri ).arg( thisPageSize ).arg( !mExpandQueryString.isEmpty() ? ( QStringLiteral( "&" ) + mExpandQueryString ) : QString() );
    const QString typeFilter = QgsSensorThingsUtils::filterForWkbType( mEntityType, mGeometryType );
    const QString extentFilter = QgsSensorThingsUtils::filterForExtent( mGeometryField, mFilterExtent );
    const QString filterString = QgsSensorThingsUtils::combineFilters( { typeFilter, extentFilter, mSubsetString } );
    if ( !filterString.isEmpty() )
      mNextPage += QStringLiteral( "&$filter=" ) + filterString;
  }

  locker.unlock();

  processFeatureRequest( mNextPage, feedback, [id, &f, &featureFetched]( const QgsFeature & feature )
  {
    if ( feature.id() == id )
    {
      f = feature;
      featureFetched = true;
      // don't break here -- store all the features we retrieved in this page first!
    }
  }, [&featureFetched, this]
  {
    return !featureFetched && !hasCachedAllFeatures();
  }, [this]
  {
    mNextPage.clear();
    mHasCachedAllFeatures = true;
  } );

  return featureFetched;
}

QgsFeatureIds QgsSensorThingsSharedData::getFeatureIdsInExtent( const QgsRectangle &extent, QgsFeedback *feedback, const QString &thisPage, QString &nextPage, const QgsFeatureIds &alreadyFetchedIds )
{
  const QgsRectangle requestExtent = mFilterExtent.isNull() ? extent : extent.intersect( mFilterExtent );
  const QgsGeometry extentGeom = QgsGeometry::fromRect( requestExtent );
  QgsReadWriteLocker locker( mReadWriteLock, QgsReadWriteLocker::Read );

  if ( hasCachedAllFeatures() || mCachedExtent.contains( extentGeom ) )
  {
    // all features cached locally, rely on local spatial index
    nextPage.clear();
    return qgis::listToSet( mSpatialIndex.intersects( requestExtent ) );
  }

  const QString typeFilter = QgsSensorThingsUtils::filterForWkbType( mEntityType, mGeometryType );
  const QString extentFilter = QgsSensorThingsUtils::filterForExtent( mGeometryField, requestExtent );
  QString filterString = QgsSensorThingsUtils::combineFilters( { typeFilter, extentFilter, mSubsetString } );
  if ( !filterString.isEmpty() )
    filterString = QStringLiteral( "&$filter=" ) + filterString;
  int thisPageSize = mMaximumPageSize;
  QString queryUrl;
  if ( !thisPage.isEmpty() )
  {
    queryUrl = thisPage;
    const thread_local QRegularExpression topRe( QStringLiteral( "\\$top=\\d+" ) );
    const QRegularExpressionMatch match = topRe.match( queryUrl );
    if ( match.hasMatch() )
    {
      if ( mFeatureLimit > 0 && ( mCachedFeatures.size() + thisPageSize ) > mFeatureLimit )
        thisPageSize = mFeatureLimit - mCachedFeatures.size();
      queryUrl = queryUrl.left( match.capturedStart( 0 ) ) + QStringLiteral( "$top=%1" ).arg( thisPageSize ) + queryUrl.mid( match.capturedEnd( 0 ) );
    }
  }
  else
  {
    queryUrl = QStringLiteral( "%1?$top=%2&$count=false%3%4" ).arg( mEntityBaseUri ).arg( thisPageSize ).arg( filterString, !mExpandQueryString.isEmpty() ? ( QStringLiteral( "&" ) + mExpandQueryString ) : QString() );
  }

  if ( thisPage.isEmpty() && mCachedExtent.intersects( extentGeom ) )
  {
    // we have SOME of the results from this extent cached. Let's return those first.
    // This is slightly nicer from a rendering point of view, because panning the map won't see features
    // previously visible disappear temporarily while we wait for them to be included in the service's result set...
    nextPage = queryUrl;
    return qgis::listToSet( mSpatialIndex.intersects( requestExtent ) );
  }

  locker.unlock();

  QgsFeatureIds ids;

  bool noMoreFeatures = false;
  bool hasFirstPage = false;
  const bool res = processFeatureRequest( queryUrl, feedback, [&ids, &alreadyFetchedIds]( const QgsFeature & feature )
  {
    if ( !alreadyFetchedIds.contains( feature.id() ) )
      ids.insert( feature.id() );
  }, [&hasFirstPage]
  {
    if ( !hasFirstPage )
    {
      hasFirstPage = true;
      return true;
    }

    return false;
  }, [&noMoreFeatures]
  {
    noMoreFeatures = true;
  } );
  if ( noMoreFeatures && res && ( !feedback || !feedback->isCanceled() ) )
  {
    locker.changeMode( QgsReadWriteLocker::Write );
    mCachedExtent = QgsGeometry::unaryUnion( { mCachedExtent, extentGeom } );
  }
  nextPage = noMoreFeatures || !res ? QString() : queryUrl;

  return ids;
}

void QgsSensorThingsSharedData::clearCache()
{
  QgsReadWriteLocker locker( mReadWriteLock, QgsReadWriteLocker::Write );

  mFeatureCount = static_cast< long long >( Qgis::FeatureCountState::Uncounted );
  mCachedFeatures.clear();
  mIotIdToFeatureId.clear();
  mSpatialIndex = QgsSpatialIndex();
  mFetchedFeatureExtent = QgsRectangle();
}

bool QgsSensorThingsSharedData::processFeatureRequest( QString &nextPage, QgsFeedback *feedback, const std::function< void( const QgsFeature & ) > &fetchedFeatureCallback, const std::function<bool ()> &continueFetchingCallback, const std::function<void ()> &onNoMoreFeaturesCallback )
{
  // copy some members before we unlock the read/write locker

  QgsReadWriteLocker locker( mReadWriteLock, QgsReadWriteLocker::Read );
  const QString authcfg = mAuthCfg;
  const QgsHttpHeaders headers = mHeaders;
  const QgsFields fields = mFields;
  const QList< QgsSensorThingsExpansionDefinition > expansions = mExpansions;

  while ( continueFetchingCallback() )
  {
    // don't lock while doing the fetch
    locker.unlock();

    // from: https://docs.ogc.org/is/18-088/18-088.html#nextLink
    // "SensorThings clients SHALL treat the URL of the nextLink as opaque, and SHALL NOT append system query options to the URL of a next link"
    //
    // ie don't mess with this URL!!
    const QUrl url = parseUrl( nextPage );

    QNetworkRequest request( url );
    QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsSensorThingsSharedData" ) );
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
        const auto rootContent = json::parse( content.content().toStdString() );
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

            onNoMoreFeaturesCallback();

            return true;
          }
          else
          {
            locker.changeMode( QgsReadWriteLocker::Write );
            for ( const auto &featureData : values )
            {
              auto getString = []( const basic_json<> &json, const char *tag ) -> QVariant
              {
                if ( !json.contains( tag ) )
                  return QVariant();

                std::function< QString( const basic_json<> &obj, bool &ok ) > objToString;
                objToString = [&objToString]( const basic_json<> &obj, bool & ok ) -> QString
                {
                  ok = true;
                  if ( obj.is_number_integer() )
                  {
                    return QString::number( obj.get<int>() );
                  }
                  else if ( obj.is_number_unsigned() )
                  {
                    return QString::number( obj.get<unsigned>() );
                  }
                  else if ( obj.is_boolean() )
                  {
                    return QString::number( obj.get<bool>() );
                  }
                  else if ( obj.is_number_float() )
                  {
                    return QString::number( obj.get<double>() );
                  }
                  else if ( obj.is_array() )
                  {
                    QStringList results;
                    results.reserve( obj.size() );
                    for ( const auto &item : obj )
                    {
                      bool itemOk = false;
                      const QString itemString = objToString( item, itemOk );
                      if ( itemOk )
                        results.push_back( itemString );
                    }
                    return results.join( ',' );
                  }
                  else if ( obj.is_string() )
                  {
                    return QString::fromStdString( obj.get<std::string >() );
                  }

                  ok = false;
                  return QString();
                };

                const auto &jObj = json[tag];
                bool ok = false;
                const QString r = objToString( jObj, ok );
                if ( ok )
                  return r;
                return QVariant();
              };

              auto getDateTime = []( const basic_json<> &json, const char *tag ) -> QVariant
              {
                if ( !json.contains( tag ) )
                  return QVariant();

                const auto &jObj = json[tag];
                if ( jObj.is_string() )
                {
                  const QString dateTimeString = QString::fromStdString( json[tag].get<std::string >() );
                  return QDateTime::fromString( dateTimeString, Qt::ISODateWithMs );
                }

                return QVariant();
              };

              auto getVariantMap = []( const basic_json<> &json, const char *tag ) -> QVariant
              {
                if ( !json.contains( tag ) )
                  return QVariant();

                return QgsJsonUtils::jsonToVariant( json[tag] );
              };

              auto getVariantList = []( const basic_json<> &json, const char *tag ) -> QVariant
              {
                if ( !json.contains( tag ) )
                  return QVariant();

                return QgsJsonUtils::jsonToVariant( json[tag] );
              };

              auto getStringList = []( const basic_json<> &json, const char *tag ) -> QVariant
              {
                if ( !json.contains( tag ) )
                  return QVariant();

                const auto &jObj = json[tag];
                if ( jObj.is_string() )
                {
                  return QStringList{ QString::fromStdString( json[tag].get<std::string >() ) };
                }
                else if ( jObj.is_array() )
                {
                  QStringList res;
                  for ( const auto &element : jObj )
                  {
                    if ( element.is_string() )
                      res.append( QString::fromStdString( element.get<std::string >() ) );
                  }
                  return res;
                }

                return QVariant();
              };

              auto getDateTimeRange = []( const basic_json<> &json, const char *tag ) -> std::pair< QVariant, QVariant >
              {
                if ( !json.contains( tag ) )
                  return { QVariant(), QVariant() };

                const auto &jObj = json[tag];
                if ( jObj.is_string() )
                {
                  const QString rangeString = QString::fromStdString( json[tag].get<std::string >() );
                  const QStringList rangeParts = rangeString.split( '/' );
                  if ( rangeParts.size() == 2 )
                  {
                    return
                    {
                      QDateTime::fromString( rangeParts.at( 0 ), Qt::ISODateWithMs ),
                      QDateTime::fromString( rangeParts.at( 1 ), Qt::ISODateWithMs )
                    };
                  }
                  else
                  {
                    const QDateTime instant = QDateTime::fromString( rangeString, Qt::ISODateWithMs );
                    if ( instant.isValid() )
                      return { instant, instant };
                  }
                }

                return { QVariant(), QVariant() };
              };

              const QString iotId = getString( featureData, "@iot.id" ).toString();
              if ( expansions.isEmpty() )
              {
                auto existingFeatureIdIt = mIotIdToFeatureId.constFind( iotId );
                if ( existingFeatureIdIt != mIotIdToFeatureId.constEnd() )
                {
                  // we've previously fetched and cached this feature, skip it
                  fetchedFeatureCallback( *mCachedFeatures.find( *existingFeatureIdIt ) );
                  continue;
                }
              }

              QgsFeature feature( fields );

              // Set geometry
              if ( mGeometryType != Qgis::WkbType::NoGeometry )
              {
                if ( featureData.contains( mGeometryField.toLocal8Bit().constData() ) )
                {
                  const auto &geometryPart = featureData[mGeometryField.toLocal8Bit().constData()];
                  if ( geometryPart.contains( "geometry" ) )
                    feature.setGeometry( QgsJsonUtils::geometryFromGeoJson( geometryPart["geometry"] ) );
                  else
                    feature.setGeometry( QgsJsonUtils::geometryFromGeoJson( geometryPart ) );
                }
              }

              auto extendAttributes = [&getString, &getVariantMap, &getDateTimeRange, &getDateTime, &getStringList, &getVariantList]( Qgis::SensorThingsEntity entityType, const auto & entityData, QgsAttributes & attributes )
              {
                const QString iotId = getString( entityData, "@iot.id" ).toString();
                const QString selfLink = getString( entityData, "@iot.selfLink" ).toString();

                const QVariant properties = getVariantMap( entityData, "properties" );

                // NOLINTBEGIN(bugprone-branch-clone)
                switch ( entityType )
                {
                  case Qgis::SensorThingsEntity::Invalid:
                    break;

                  case Qgis::SensorThingsEntity::Thing:
                    attributes
                        << iotId
                        << selfLink
                        << getString( entityData, "name" )
                        << getString( entityData, "description" )
                        << properties;
                    break;

                  case Qgis::SensorThingsEntity::Location:
                    attributes
                        << iotId
                        << selfLink
                        << getString( entityData, "name" )
                        << getString( entityData, "description" )
                        << properties;
                    break;

                  case Qgis::SensorThingsEntity::HistoricalLocation:
                    attributes
                        << iotId
                        << selfLink
                        << getDateTime( entityData, "time" );
                    break;

                  case Qgis::SensorThingsEntity::Datastream:
                  {
                    std::pair< QVariant, QVariant > phenomenonTime = getDateTimeRange( entityData, "phenomenonTime" );
                    std::pair< QVariant, QVariant > resultTime = getDateTimeRange( entityData, "resultTime" );
                    attributes
                        << iotId
                        << selfLink
                        << getString( entityData, "name" )
                        << getString( entityData, "description" )
                        << getVariantMap( entityData, "unitOfMeasurement" )
                        << getString( entityData, "observationType" )
                        << properties
                        << phenomenonTime.first
                        << phenomenonTime.second
                        << resultTime.first
                        << resultTime.second;
                    break;
                  }

                  case Qgis::SensorThingsEntity::Sensor:
                    attributes
                        << iotId
                        << selfLink
                        << getString( entityData, "name" )
                        << getString( entityData, "description" )
                        << getString( entityData, "metadata" )
                        << properties;
                    break;

                  case Qgis::SensorThingsEntity::ObservedProperty:
                    attributes
                        << iotId
                        << selfLink
                        << getString( entityData, "name" )
                        << getString( entityData, "definition" )
                        << getString( entityData, "description" )
                        << properties;
                    break;

                  case Qgis::SensorThingsEntity::Observation:
                  {
                    std::pair< QVariant, QVariant > phenomenonTime = getDateTimeRange( entityData, "phenomenonTime" );
                    std::pair< QVariant, QVariant > validTime = getDateTimeRange( entityData, "validTime" );
                    attributes
                        << iotId
                        << selfLink
                        << phenomenonTime.first
                        << phenomenonTime.second
                        << getString( entityData, "result" ) // TODO -- result type handling!
                        << getDateTime( entityData, "resultTime" )
                        << getStringList( entityData, "resultQuality" )
                        << validTime.first
                        << validTime.second
                        << getVariantMap( entityData, "parameters" );
                    break;
                  }

                  case Qgis::SensorThingsEntity::FeatureOfInterest:
                    attributes
                        << iotId
                        << selfLink
                        << getString( entityData, "name" )
                        << getString( entityData, "description" )
                        << properties;
                    break;

                  case Qgis::SensorThingsEntity::MultiDatastream:
                  {
                    std::pair< QVariant, QVariant > phenomenonTime = getDateTimeRange( entityData, "phenomenonTime" );
                    std::pair< QVariant, QVariant > resultTime = getDateTimeRange( entityData, "resultTime" );
                    attributes
                        << iotId
                        << selfLink
                        << getString( entityData, "name" )
                        << getString( entityData, "description" )
                        << getVariantList( entityData, "unitOfMeasurements" )
                        << getString( entityData, "observationType" )
                        << getStringList( entityData, "multiObservationDataTypes" )
                        << properties
                        << phenomenonTime.first
                        << phenomenonTime.second
                        << resultTime.first
                        << resultTime.second;
                    break;
                  }
                }
                // NOLINTEND(bugprone-branch-clone)
              };

              QgsAttributes attributes;
              attributes.reserve( fields.size() );
              extendAttributes( mEntityType, featureData, attributes );

              auto processFeature = [this, &fetchedFeatureCallback]( QgsFeature & feature, const QString & rawFeatureId )
              {
                feature.setId( mNextFeatureId++ );

                mCachedFeatures.insert( feature.id(), feature );
                mIotIdToFeatureId.insert( rawFeatureId, feature.id() );
                mSpatialIndex.addFeature( feature );
                mFetchedFeatureExtent.combineExtentWith( feature.geometry().boundingBox() );

                fetchedFeatureCallback( feature );
              };

              const QString baseFeatureId = getString( featureData, "@iot.id" ).toString();
              if ( !expansions.empty() )
              {
                mRetrievedBaseFeatureCount++;

                std::function< void( const nlohmann::json &, Qgis::SensorThingsEntity, const QList<QgsSensorThingsExpansionDefinition > &, const QString &, const QgsAttributes & ) > traverseExpansion;
                traverseExpansion = [this, &feature, &getString, &traverseExpansion, &fetchedFeatureCallback, &extendAttributes, &processFeature]( const nlohmann::json & currentLevelData, Qgis::SensorThingsEntity parentEntityType, const QList<QgsSensorThingsExpansionDefinition > &expansionTargets, const QString & lowerLevelId, const QgsAttributes & lowerLevelAttributes )
                {
                  const QgsSensorThingsExpansionDefinition currentExpansionTarget = expansionTargets.at( 0 );
                  const QList< QgsSensorThingsExpansionDefinition > remainingExpansionTargets = expansionTargets.mid( 1 );

                  bool ok = false;
                  const Qgis::RelationshipCardinality cardinality = QgsSensorThingsUtils::relationshipCardinality( parentEntityType, currentExpansionTarget.childEntity(), ok );
                  QString currentExpansionPropertyString;
                  switch ( cardinality )
                  {
                    case Qgis::RelationshipCardinality::OneToOne:
                    case Qgis::RelationshipCardinality::ManyToOne:
                      currentExpansionPropertyString = qgsEnumValueToKey( currentExpansionTarget.childEntity() );
                      break;

                    case Qgis::RelationshipCardinality::OneToMany:
                    case Qgis::RelationshipCardinality::ManyToMany:
                      currentExpansionPropertyString = QgsSensorThingsUtils::entityToSetString( currentExpansionTarget.childEntity() );
                      break;
                  }

                  if ( currentLevelData.contains( currentExpansionPropertyString.toLocal8Bit().constData() ) )
                  {
                    auto parseExpandedEntity = [lowerLevelAttributes, &feature, &processFeature, &lowerLevelId, &getString, &remainingExpansionTargets, &fetchedFeatureCallback, &extendAttributes, &traverseExpansion, &currentExpansionTarget, this]( const json & expandedEntityElement )
                    {
                      QgsAttributes expandedAttributes = lowerLevelAttributes;
                      const QString expandedEntityIotId = getString( expandedEntityElement, "@iot.id" ).toString();
                      const QString expandedFeatureId = lowerLevelId + '_' + expandedEntityIotId;

                      if ( remainingExpansionTargets.empty() )
                      {
                        auto existingFeatureIdIt = mIotIdToFeatureId.constFind( expandedFeatureId );
                        if ( existingFeatureIdIt != mIotIdToFeatureId.constEnd() )
                        {
                          // we've previously fetched and cached this feature, skip it
                          fetchedFeatureCallback( *mCachedFeatures.find( *existingFeatureIdIt ) );
                          return;
                        }
                      }

                      extendAttributes( currentExpansionTarget.childEntity(), expandedEntityElement, expandedAttributes );
                      if ( !remainingExpansionTargets.empty() )
                      {
                        // traverse deeper
                        traverseExpansion( expandedEntityElement, currentExpansionTarget.childEntity(), remainingExpansionTargets, expandedFeatureId, expandedAttributes );
                      }
                      else
                      {
                        feature.setAttributes( expandedAttributes );
                        processFeature( feature, expandedFeatureId );
                      }
                    };
                    const auto &expandedEntity = currentLevelData[currentExpansionPropertyString.toLocal8Bit().constData()];
                    if ( expandedEntity.is_array() )
                    {
                      for ( const auto &expandedEntityElement : expandedEntity )
                      {
                        parseExpandedEntity( expandedEntityElement );
                      }
                      // NOTE: What do we do when the expanded entity has a next link? Does this situation ever arise?
                      // The specification doesn't explicitly state whether pagination is supported for expansion, so we assume
                      // it's not possible.
                    }
                    else if ( expandedEntity.is_object() )
                    {
                      parseExpandedEntity( expandedEntity );
                    }
                  }
                  else
                  {
                    // No expansion for this parent feature.
                    // Maybe we should NULL out the attributes and return the parent feature? Right now we just
                    // skip it if there's no child features...
                  }
                };

                traverseExpansion( featureData, mEntityType, expansions, baseFeatureId, attributes );

                if ( mFeatureLimit > 0 && mFeatureLimit <= mRetrievedBaseFeatureCount )
                  break;
              }
              else
              {
                feature.setAttributes( attributes );
                processFeature( feature, baseFeatureId );
                mRetrievedBaseFeatureCount++;
                if ( mFeatureLimit > 0 && mFeatureLimit <= mRetrievedBaseFeatureCount )
                  break;
              }
            }
          }
          locker.unlock();

          if ( rootContent.contains( "@iot.nextLink" ) && ( mFeatureLimit == 0 || mFeatureLimit > mCachedFeatures.size() ) )
          {
            nextPage = QString::fromStdString( rootContent["@iot.nextLink"].get<std::string>() );
          }
          else
          {
            onNoMoreFeaturesCallback();
          }

          // if target feature was added to cache, return it
          if ( !continueFetchingCallback() )
          {
            return true;
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

///@endcond PRIVATE
