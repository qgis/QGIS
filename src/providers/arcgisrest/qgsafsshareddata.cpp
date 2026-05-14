/***************************************************************************
    qgsafsshareddata.cpp
    ---------------------
    begin                : June 2017
    copyright            : (C) 2017 by Sandro Mani
    email                : manisandro at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsafsshareddata.h"

#include <nlohmann/json.hpp>

#include "qgsarcgisrestquery.h"
#include "qgsarcgisrestutils.h"
#include "qgsblockingnetworkrequest.h"
#include "qgsjsonutils.h"
#include "qgslogger.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsreadwritelocker.h"
#include "qgssetrequestinitiator_p.h"

#include <QFile>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QString>
#include <QUrlQuery>

using namespace Qt::StringLiterals;

long long QgsAfsSharedData::objectIdCount() const
{
  QgsReadWriteLocker locker( mReadWriteLock, QgsReadWriteLocker::Read );
  Q_ASSERT( mHasFetchedObjectIds );

  return mFeatureIdsToObjectIds.size();
}

long long QgsAfsSharedData::featureCount( QString &errorMessage )
{
  if ( !ensureObjectIdsFetched( errorMessage ) )
  {
    return -1;
  }

  QgsReadWriteLocker locker( mReadWriteLock, QgsReadWriteLocker::Read );
  return mFeatureIdsToObjectIds.size() - mDeletedFeatureIds.size();
}

QgsRectangle QgsAfsSharedData::extent() const
{
  if ( mDataSource.sql().isEmpty() )
    return mExtent;

  return QgsArcGisRestQueryUtils::getExtent( mDataSource.param( u"url"_s ), mDataSource.sql(), mDataSource.authConfigId(), mDataSource.httpHeaders() );
}

QgsAfsSharedData::QgsAfsSharedData( const QgsDataSourceUri &uri )
  : mDataSource( uri )
{}

bool QgsAfsSharedData::ensureObjectIdsFetched( QString &errorMessage )
{
  QgsReadWriteLocker locker( mReadWriteLock, QgsReadWriteLocker::Read );
  if ( mHasFetchedObjectIds )
    return true;

  locker.unlock();
  // Read OBJECTIDs of all features: these may not be a continuous sequence,
  // and we need to store these to iterate through the features. This query
  // also returns the name of the ObjectID field.
  return getObjectIds( errorMessage );
}

std::shared_ptr<QgsAfsSharedData> QgsAfsSharedData::clone() const
{
  QgsReadWriteLocker locker( mReadWriteLock, QgsReadWriteLocker::Read );
  auto copy = std::make_shared<QgsAfsSharedData>( mDataSource );
  copy->mHasFetchedObjectIds = mHasFetchedObjectIds;
  copy->mLimitBBox = mLimitBBox;
  copy->mExtent = mExtent;
  copy->mGeometryType = mGeometryType;
  copy->mFields = mFields;
  copy->mMaximumFetchObjectsCount = mMaximumFetchObjectsCount;
  copy->mObjectIdFieldName = mObjectIdFieldName;
  copy->mObjectIdFieldIdx = mObjectIdFieldIdx;
  copy->mFeatureIdsToObjectIds = mFeatureIdsToObjectIds;
  copy->mObjectIdToFeatureId = mObjectIdToFeatureId;
  copy->mDeletedFeatureIds = mDeletedFeatureIds;
  copy->mCache = mCache;
  copy->mSourceCRS = mSourceCRS;
  return copy;
}

QString QgsAfsSharedData::subsetString() const
{
  return mDataSource.sql();
}

bool QgsAfsSharedData::setSubsetString( const QString &subset )
{
  mDataSource.setSql( subset );

  clearCache();
  return true;
}

void QgsAfsSharedData::clearCache()
{
  QgsReadWriteLocker locker( mReadWriteLock, QgsReadWriteLocker::Write );

  mCache.clear();
  mFeatureIdsToObjectIds.clear();
  mObjectIdToFeatureId.clear();
  mDeletedFeatureIds.clear();
  mHasFetchedObjectIds = false;
}

bool QgsAfsSharedData::getObjectIds( QString &errorMessage )
{
  errorMessage.clear();

  // Read OBJECTIDs of all features: these may not be a continuous sequence,
  // and we need to store these to iterate through the features. This query
  // also returns the name of the ObjectID field.
  QString errorTitle;
  QString error;
  QVariantMap objectIdData = QgsArcGisRestQueryUtils::getObjectIds(
    mDataSource.param( u"url"_s ), mDataSource.authConfigId(), errorTitle, error, mDataSource.httpHeaders(), mDataSource.param( u"urlprefix"_s ), mLimitBBox ? mExtent : QgsRectangle(), mDataSource.sql()
  );
  if ( objectIdData.isEmpty() )
  {
    errorMessage = QObject::tr( "getObjectIds failed: %1 - %2" ).arg( errorTitle, error );
    return false;
  }
  if ( !objectIdData[u"objectIdFieldName"_s].isValid() || !objectIdData[u"objectIds"_s].isValid() )
  {
    errorMessage = QObject::tr( "Failed to determine objectIdFieldName and/or objectIds" );
    return false;
  }
  if ( objectIdData[u"objectIdFieldName"_s].toString() != mObjectIdFieldName )
  {
    QgsDebugError( u"Object ID field name mismatch: %1 vs %2"_s.arg( objectIdData[u"objectIdFieldName"_s].toString(), mObjectIdFieldName ) );
  }

  const QVariantList objectIds = objectIdData.value( u"objectIds"_s ).toList();
  mFeatureIdsToObjectIds.reserve( mFeatureIdsToObjectIds.size() + objectIds.size() );
  mObjectIdToFeatureId.reserve( mObjectIdToFeatureId.size() + objectIds.size() );
  for ( const QVariant &objectId : objectIds )
  {
    const int objectIdInt = objectId.toInt();
    mObjectIdToFeatureId.insert( objectIdInt, mFeatureIdsToObjectIds.size() );
    mFeatureIdsToObjectIds.append( objectIdInt );
  }
  mHasFetchedObjectIds = true;
  return true;
}

quint32 QgsAfsSharedData::featureIdToObjectId( QgsFeatureId id, QString &error )
{
  if ( !ensureObjectIdsFetched( error ) )
  {
    return 0;
  }
  QgsReadWriteLocker locker( mReadWriteLock, QgsReadWriteLocker::Read );
  return mFeatureIdsToObjectIds.value( id, -1 );
}

QgsFeatureId QgsAfsSharedData::objectIdToFeatureId( quint32 oid )
{
  Q_ASSERT( mHasFetchedObjectIds );

  // lock must already be obtained by caller!
  return mObjectIdToFeatureId.value( oid, -1 );
}

bool QgsAfsSharedData::getFeature( QgsFeatureId id, QgsFeature &f, const QList<QgsFeatureId> &pendingFeatureIds, QgsFeedback *feedback )
{
  QgsReadWriteLocker locker( mReadWriteLock, QgsReadWriteLocker::Read );
  Q_ASSERT( mHasFetchedObjectIds );

  // If cached, return cached feature
  QMap<QgsFeatureId, QgsFeature>::const_iterator it = mCache.constFind( id );
  if ( it != mCache.constEnd() )
  {
    f = it.value();
    return true;
  }

  const QString authcfg = mDataSource.authConfigId();
  bool featureFetched = false;
  QSet<quint32> objectIds;
  objectIds.reserve( mMaximumFetchObjectsCount );
  QVariantMap queryData;
  QList< QgsFeatureId > requestedFeatureIds;
  requestedFeatureIds.reserve( mMaximumFetchObjectsCount );

  if ( id < 0 || id >= mFeatureIdsToObjectIds.size() )
    return false;

  const quint32 targetObjectId = mFeatureIdsToObjectIds.at( id );
  while ( !featureFetched )
  {
    objectIds.clear();
    objectIds.insert( targetObjectId );
    // first priority is to fill the batch with pending feature IDs, because we know
    // these are desirable for the caller
    for ( QgsFeatureId pendingId : pendingFeatureIds )
    {
      if ( pendingId >= 0 && pendingId < mFeatureIdsToObjectIds.count() && !mDeletedFeatureIds.contains( pendingId ) && !mCache.contains( pendingId ) )
      {
        objectIds.insert( mFeatureIdsToObjectIds.at( pendingId ) );
        if ( objectIds.size() >= mMaximumFetchObjectsCount )
        {
          break;
        }
      }
    }

    if ( objectIds.size() < mMaximumFetchObjectsCount )
    {
      // if batch isn't yet full, then fill remainder with sequential IDs we haven't yet fetched
      const int startId = static_cast< int >( ( id / mMaximumFetchObjectsCount ) * mMaximumFetchObjectsCount );
      const int stopId = static_cast< int >( mFeatureIdsToObjectIds.length() );
      for ( int i = startId; i < stopId; ++i )
      {
        if ( i >= 0 && i < mFeatureIdsToObjectIds.count() && !mDeletedFeatureIds.contains( i ) && !mCache.contains( i ) )
        {
          objectIds.insert( mFeatureIdsToObjectIds.at( i ) );
          if ( objectIds.size() >= mMaximumFetchObjectsCount )
          {
            break;
          }
        }
      }
    }

    if ( objectIds.empty() )
    {
      QgsDebugMsgLevel( u"No valid features IDs to fetch"_s, 2 );
      return false;
    }

    // sort requested feature IDs, to make requests more cache friendly
    // (as opposed to unpredictable set ordering)
    QList< quint32 > requestedObjectIds = qgis::setToList( objectIds );
    std::sort( requestedObjectIds.begin(), requestedObjectIds.end() );
    requestedFeatureIds.clear();
    for ( quint32 objectId : requestedObjectIds )
    {
      requestedFeatureIds.append( mObjectIdToFeatureId[objectId] );
    }

    // don't lock while doing the fetch
    locker.unlock();

    // Query
    QString errorTitle, errorMessage;
    queryData = QgsArcGisRestQueryUtils::getObjects(
      mDataSource.param( u"url"_s ),
      authcfg,
      requestedObjectIds,
      mDataSource.param( u"crs"_s ),
      true,
      QStringList(),
      QgsWkbTypes::hasM( mGeometryType ),
      QgsWkbTypes::hasZ( mGeometryType ),
      errorTitle,
      errorMessage,
      mDataSource.httpHeaders(),
      feedback,
      mDataSource.param( u"urlprefix"_s )
    );

    if ( feedback && feedback->isCanceled() )
    {
      return false;
    }

    if ( queryData.isEmpty() )
    {
      if ( mMaximumFetchObjectsCount <= 1 || errorMessage.isEmpty() )
      {
        QgsDebugMsgLevel( u"Query returned empty result"_s, 2 );
        return false;
      }
      else
      {
        locker.changeMode( QgsReadWriteLocker::Read );
        mMaximumFetchObjectsCount = std::max( 1, mMaximumFetchObjectsCount / 5 );
      }
    }
    else
    {
      featureFetched = true;
    }
  }

  // but re-lock while updating cache
  locker.changeMode( QgsReadWriteLocker::Write );
  const QVariantList featuresData = queryData[u"features"_s].toList();
  if ( featuresData.isEmpty() )
  {
    QgsDebugMsgLevel( u"Query returned no features"_s, 3 );
    return false;
  }
  for ( int i = 0, n = featuresData.size(); i < n; ++i )
  {
    const QVariantMap featureData = featuresData[i].toMap();
    QgsFeature feature;
    if ( i >= requestedFeatureIds.size() )
    {
      QgsDebugError( u"Server responded with more features than expected -- results will be unpredictable!" );
      break;
    }
    QgsFeatureId featureId = requestedFeatureIds[i];

    // Set attributes
    const QVariantMap attributesData = featureData[u"attributes"_s].toMap();
    feature.setFields( mFields );
    QgsAttributes attributes( mFields.size() );
    for ( int idx = 0; idx < mFields.size(); ++idx )
    {
      QVariant attribute = attributesData[mFields.at( idx ).name()];
      if ( QgsVariantUtils::isNull( attribute ) )
      {
        // ensure that null values are mapped correctly for PyQGIS
        attribute = QgsVariantUtils::createNullVariant( QMetaType::Type::Int );
      }

      // date/datetime fields must be converted
      if ( mFields.at( idx ).type() == QMetaType::Type::QDateTime || mFields.at( idx ).type() == QMetaType::Type::QDate )
        attribute = QgsArcGisRestUtils::convertDateTime( attribute );

      if ( !mFields.at( idx ).convertCompatible( attribute ) )
      {
        QgsDebugError( u"Invalid value %1 for field %2 of type %3"_s.arg( attributesData[mFields.at( idx ).name()].toString(), mFields.at( idx ).name(), mFields.at( idx ).typeName() ) );
      }
      attributes[idx] = attribute;
      if ( mFields.at( idx ).name() == mObjectIdFieldName )
      {
        featureId = objectIdToFeatureId( attributesData[mFields.at( idx ).name()].toInt() );
      }
    }
    feature.setAttributes( attributes );

    // Set FID
    feature.setId( featureId );

    // Set geometry
    const QVariantMap geometryData = featureData[u"geometry"_s].toMap();
    std::unique_ptr<QgsAbstractGeometry> geometry(
      QgsArcGisRestUtils::convertGeometry( geometryData, queryData[u"geometryType"_s].toString(), QgsWkbTypes::hasM( mGeometryType ), QgsWkbTypes::hasZ( mGeometryType ), QgsWkbTypes::isCurvedType( mGeometryType ) )
    );
    // Above might return 0, which is OK since in theory empty geometries are allowed
    if ( geometry )
      feature.setGeometry( QgsGeometry( std::move( geometry ) ) );
    feature.setValid( true );
    mCache.insert( feature.id(), feature );
  }

  // If added to cache, return feature
  it = mCache.constFind( id );
  if ( it != mCache.constEnd() )
  {
    f = it.value();
    return true;
  }

  return false;
}

QgsFeatureIds QgsAfsSharedData::getFeatureIdsInExtent( const QgsRectangle &extent, QgsFeedback *feedback )
{
  QString errorTitle;
  QString errorText;

  const QString authcfg = mDataSource.authConfigId();
  const QList<quint32> objectIdsInRect = QgsArcGisRestQueryUtils::
    getObjectIdsByExtent( mDataSource.param( u"url"_s ), extent, errorTitle, errorText, authcfg, mDataSource.httpHeaders(), feedback, mDataSource.sql(), mDataSource.param( u"urlprefix"_s ) );

  QgsReadWriteLocker locker( mReadWriteLock, QgsReadWriteLocker::Read );
  Q_ASSERT( mHasFetchedObjectIds );

  QgsFeatureIds ids;
  for ( const quint32 objectId : objectIdsInRect )
  {
    const QgsFeatureId featureId = objectIdToFeatureId( objectId );
    if ( featureId >= 0 )
      ids.insert( featureId );
  }
  return ids;
}

bool QgsAfsSharedData::deleteFeatures( const QgsFeatureIds &ids, QString &error, QgsFeedback *feedback )
{
  error.clear();
  if ( !ensureObjectIdsFetched( error ) )
  {
    return false;
  }
  QgsReadWriteLocker locker( mReadWriteLock, QgsReadWriteLocker::Read );

  QStringList stringIds;
  for ( const QgsFeatureId id : ids )
  {
    stringIds.append( QString::number( mFeatureIdsToObjectIds[id] ) );
  }
  locker.unlock();

  QUrl queryUrl( mDataSource.param( u"url"_s ) + "/deleteFeatures" );

  QByteArray payload;
  payload.append( u"f=json&objectIds=%1"_s.arg( stringIds.join( ',' ) ).toUtf8() );

  bool ok = false;
  postData( queryUrl, payload, feedback, ok, error );
  if ( !ok )
  {
    return false;
  }

  locker.changeMode( QgsReadWriteLocker::Write );
  for ( QgsFeatureId id : ids )
  {
    mCache.remove( id );
    mDeletedFeatureIds.insert( id );
  }

  return true;
}

bool QgsAfsSharedData::addFeatures( QgsFeatureList &features, QString &errorMessage, QgsFeedback *feedback )
{
  errorMessage.clear();
  if ( !ensureObjectIdsFetched( errorMessage ) )
  {
    return false;
  }

  QUrl queryUrl( mDataSource.param( u"url"_s ) + "/addFeatures" );

  QgsArcGisRestContext context;

  QVariantList featuresJson;
  featuresJson.reserve( features.size() );
  for ( const QgsFeature &feature : features )
  {
    featuresJson.append(
      QgsArcGisRestUtils::
        featureToJson( feature, context, QgsCoordinateReferenceSystem(), QgsArcGisRestUtils::FeatureToJsonFlag::IncludeGeometry | QgsArcGisRestUtils::FeatureToJsonFlag::IncludeNonObjectIdAttributes | QgsArcGisRestUtils::FeatureToJsonFlag::SkipUnsetAttributes )
    );
  }

  const QString json = QString::fromStdString( QgsJsonUtils::jsonFromVariant( featuresJson ).dump( 2 ) );

  QByteArray payload;
  payload.append( u"f=json&features=%1"_s.arg( json ).toUtf8() );

  bool ok = false;
  const QVariantMap results = postData( queryUrl, payload, feedback, ok, errorMessage );
  if ( !ok )
  {
    return false;
  }

  const QVariantList addResults = results.value( u"addResults"_s ).toList();
  for ( const QVariant &result : addResults )
  {
    const QVariantMap resultMap = result.toMap();
    if ( !resultMap.value( u"success"_s ).toBool() )
    {
      errorMessage = resultMap.value( u"error"_s ).toMap().value( u"description"_s ).toString();
      return false;
    }
  }

  // All good!
  QgsReadWriteLocker locker( mReadWriteLock, QgsReadWriteLocker::Write );
  int i = 0;
  for ( const QVariant &result : addResults )
  {
    const QVariantMap resultMap = result.toMap();
    const long long objectId = resultMap.value( u"objectId"_s ).toLongLong();

    const QgsFeatureId newId = mFeatureIdsToObjectIds.size();
    features[i].setId( newId );
    mObjectIdToFeatureId.insert( objectId, newId );
    mFeatureIdsToObjectIds.append( objectId );

    i++;
  }
  return true;
}

bool QgsAfsSharedData::updateFeatures( const QgsFeatureList &features, bool includeGeometries, bool includeAttributes, QString &error, QgsFeedback *feedback )
{
  error.clear();
  if ( !ensureObjectIdsFetched( error ) )
  {
    return false;
  }
  QUrl queryUrl( mDataSource.param( u"url"_s ) + "/updateFeatures" );

  QgsArcGisRestContext context;
  context.setObjectIdFieldName( mObjectIdFieldName );

  QgsArcGisRestUtils::FeatureToJsonFlags flags;
  if ( includeGeometries )
    flags |= QgsArcGisRestUtils::FeatureToJsonFlag::IncludeGeometry;
  if ( includeAttributes )
    flags |= QgsArcGisRestUtils::FeatureToJsonFlag::IncludeNonObjectIdAttributes | QgsArcGisRestUtils::FeatureToJsonFlag::SkipUnsetAttributes;

  QVariantList featuresJson;
  featuresJson.reserve( features.size() );
  for ( const QgsFeature &feature : features )
  {
    featuresJson.append( QgsArcGisRestUtils::featureToJson( feature, context, QgsCoordinateReferenceSystem(), flags ) );
  }

  const QString json = QString::fromStdString( QgsJsonUtils::jsonFromVariant( featuresJson ).dump( 2 ) );

  QByteArray payload;
  payload.append( u"f=json&features=%1"_s.arg( json ).toUtf8() );

  bool ok = false;
  const QVariantMap results = postData( queryUrl, payload, feedback, ok, error );
  if ( !ok )
  {
    return false;
  }

  const QVariantList addResults = results.value( u"updateResults"_s ).toList();
  for ( const QVariant &result : addResults )
  {
    const QVariantMap resultMap = result.toMap();
    if ( !resultMap.value( u"success"_s ).toBool() )
    {
      error = resultMap.value( u"error"_s ).toMap().value( u"description"_s ).toString();
      return false;
    }
  }

  // All good. Now we remove the cached versions of features so that they'll get re-fetched from the service
  QgsReadWriteLocker locker( mReadWriteLock, QgsReadWriteLocker::Write );
  for ( const QgsFeature &feature : features )
  {
    mCache.remove( feature.id() );
  }
  return true;
}

bool QgsAfsSharedData::addFields( const QString &adminUrl, const QList<QgsField> &attributes, QString &error, QgsFeedback *feedback )
{
  error.clear();
  QUrl queryUrl( adminUrl + "/addToDefinition" );

  QVariantList fieldsJson;
  fieldsJson.reserve( attributes.size() );
  for ( const QgsField &field : attributes )
  {
    fieldsJson.append( QgsArcGisRestUtils::fieldDefinitionToJson( field ) );
  }

  const QVariantMap definition { { u"fields"_s, fieldsJson } };

  const QString json = QString::fromStdString( QgsJsonUtils::jsonFromVariant( definition ).dump( 2 ) );

  QByteArray payload;
  payload.append( u"f=json&addToDefinition=%1"_s.arg( json ).toUtf8() );

  bool ok = false;
  const QVariantMap results = postData( queryUrl, payload, feedback, ok, error );
  if ( !ok )
  {
    return false;
  }

  if ( !results.value( u"success"_s ).toBool() )
  {
    error = results.value( u"error"_s ).toMap().value( u"message"_s ).toString();
    return false;
  }

  // All good. Now we remove the cached versions of features so that they'll get re-fetched from the service
  QgsReadWriteLocker locker( mReadWriteLock, QgsReadWriteLocker::Write );
  mCache.clear();

  for ( const QgsField &field : attributes )
  {
    mFields.append( field );
  }

  return true;
}

bool QgsAfsSharedData::deleteFields( const QString &adminUrl, const QgsAttributeIds &attributes, QString &error, QgsFeedback *feedback )
{
  error.clear();
  if ( !ensureObjectIdsFetched( error ) )
  {
    return false;
  }
  QUrl queryUrl( adminUrl + "/deleteFromDefinition" );

  QVariantList fieldsJson;
  fieldsJson.reserve( attributes.size() );
  QStringList fieldNames;
  for ( int index : attributes )
  {
    if ( index >= 0 && index < mFields.count() )
    {
      fieldsJson.append( QVariantMap( { { u"name"_s, mFields.at( index ).name() } } ) );
      fieldNames << mFields.at( index ).name();
    }
  }

  const QVariantMap definition { { u"fields"_s, fieldsJson } };

  const QString json = QString::fromStdString( QgsJsonUtils::jsonFromVariant( definition ).dump( 2 ) );

  QByteArray payload;
  payload.append( u"f=json&deleteFromDefinition=%1"_s.arg( json ).toUtf8() );

  bool ok = false;
  const QVariantMap results = postData( queryUrl, payload, feedback, ok, error );
  if ( !ok )
  {
    return false;
  }

  if ( !results.value( u"success"_s ).toBool() )
  {
    error = results.value( u"error"_s ).toMap().value( u"message"_s ).toString();
    return false;
  }

  // All good. Now we remove the cached versions of features so that they'll get re-fetched from the service
  QgsReadWriteLocker locker( mReadWriteLock, QgsReadWriteLocker::Write );
  mCache.clear();

  for ( const QString &name : std::as_const( fieldNames ) )
  {
    mFields.remove( mFields.lookupField( name ) );
  }

  return true;
}

bool QgsAfsSharedData::addAttributeIndex( const QString &adminUrl, int attribute, QString &error, QgsFeedback *feedback )
{
  error.clear();
  if ( !ensureObjectIdsFetched( error ) )
  {
    return false;
  }
  QUrl queryUrl( adminUrl + "/addToDefinition" );

  const QString name = mFields.field( attribute ).name();


  QVariantList indexJson;
  indexJson << QVariantMap( { { u"name"_s, u"%1_index"_s.arg( name ) }, { u"fields"_s, name }, { u"description"_s, name } } );


  const QVariantMap definition { { u"indexes"_s, indexJson } };

  const QString json = QString::fromStdString( QgsJsonUtils::jsonFromVariant( definition ).dump( 2 ) );

  QByteArray payload;
  payload.append( u"f=json&addToDefinition=%1"_s.arg( json ).toUtf8() );

  bool ok = false;
  const QVariantMap results = postData( queryUrl, payload, feedback, ok, error );
  if ( !ok )
  {
    return false;
  }

  if ( !results.value( u"success"_s ).toBool() )
  {
    error = results.value( u"error"_s ).toMap().value( u"message"_s ).toString();
    return false;
  }

  return true;
}

bool QgsAfsSharedData::hasCachedAllFeatures()
{
  QgsReadWriteLocker locker( mReadWriteLock, QgsReadWriteLocker::Read );
  if ( !mHasFetchedObjectIds )
    return false;

  QString error;
  const long long count = featureCount( error );
  if ( count < 0 )
  {
    return false;
  }

  return mCache.count() == count;
}

int QgsAfsSharedData::objectIdFieldIndex() const
{
  QgsReadWriteLocker locker( mReadWriteLock, QgsReadWriteLocker::Read );
  return mObjectIdFieldIdx;
}

QVariantMap QgsAfsSharedData::postData( const QUrl &url, const QByteArray &payload, QgsFeedback *feedback, bool &ok, QString &errorText ) const
{
  errorText.clear();
  ok = false;

  bool isTestEndpoint = false;
  const QUrl modifiedUrl = QgsArcGisRestQueryUtils::parseUrl( url, &isTestEndpoint );
  if ( isTestEndpoint )
  {
    const QString localFile = modifiedUrl.toLocalFile() + "_payload";
    QgsDebugMsgLevel( u"payload file is %1"_s.arg( localFile ), 2 );
    {
      QFile file( localFile );
      if ( file.open( QFile::WriteOnly | QIODevice::Truncate ) )
      {
        file.write( payload );
        file.close();
      }
    }

    ok = true;

    QVariantMap res;
    {
      QFile file( modifiedUrl.toLocalFile() );
      if ( file.open( QFile::ReadOnly ) )
      {
        res = QgsJsonUtils::parseJson( file.readAll() ).toMap();
      }
    }

    return res;
  }

  QNetworkRequest request( modifiedUrl );
  request.setHeader( QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded"_L1 );
  QgsSetRequestInitiatorClass( request, u"QgsArcGisRestUtils"_s );

  QgsBlockingNetworkRequest networkRequest;
  networkRequest.setAuthCfg( mDataSource.authConfigId() );

  const QgsBlockingNetworkRequest::ErrorCode error = networkRequest.post( request, payload, false, feedback );

  // Handle network errors
  if ( error != QgsBlockingNetworkRequest::NoError )
  {
    QgsDebugError( u"Network error: %1"_s.arg( networkRequest.errorMessage() ) );
    errorText = networkRequest.errorMessage();

    // try to get detailed error message from reply
    const QString content = networkRequest.reply().content();
    const thread_local QRegularExpression errorRx( u"Error: <.*?>(.*?)<"_s );
    const QRegularExpressionMatch match = errorRx.match( content );
    if ( match.hasMatch() )
    {
      errorText = match.captured( 1 );
    }

    return QVariantMap();
  }

  ok = true;
  return QgsJsonUtils::parseJson( networkRequest.reply().content() ).toMap();
}
