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
#include "qgsarcgisrestutils.h"
#include "qgsarcgisrestquery.h"
#include "qgslogger.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsblockingnetworkrequest.h"
#include "qgsreadwritelocker.h"
#include "qgsjsonutils.h"

#include <QUrlQuery>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QFile>

#include <nlohmann/json.hpp>

long long QgsAfsSharedData::objectIdCount() const
{
  QgsReadWriteLocker locker( mReadWriteLock, QgsReadWriteLocker::Read );
  return mObjectIds.size();
}

long long QgsAfsSharedData::featureCount() const
{
  QgsReadWriteLocker locker( mReadWriteLock, QgsReadWriteLocker::Read );
  return mObjectIds.size() - mDeletedFeatureIds.size();
}

QgsRectangle QgsAfsSharedData::extent() const
{
  if ( mDataSource.sql().isEmpty() )
    return mExtent;

  return QgsArcGisRestQueryUtils::getExtent( mDataSource.param( QStringLiteral( "url" ) ), mDataSource.sql(), mDataSource.authConfigId(),
         mDataSource.httpHeaders() );
}

QgsAfsSharedData::QgsAfsSharedData( const QgsDataSourceUri &uri )
  : mDataSource( uri )
{
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
  mObjectIds.clear();
  mObjectIdToFeatureId.clear();
  mDeletedFeatureIds.clear();
  QString error;
  getObjectIds( error );
}

bool QgsAfsSharedData::getObjectIds( QString &errorMessage )
{
  errorMessage.clear();

  // Read OBJECTIDs of all features: these may not be a continuous sequence,
  // and we need to store these to iterate through the features. This query
  // also returns the name of the ObjectID field.
  QString errorTitle;
  QString error;
  QVariantMap objectIdData = QgsArcGisRestQueryUtils::getObjectIds( mDataSource.param( QStringLiteral( "url" ) ), mDataSource.authConfigId(),
                             errorTitle, error, mDataSource.httpHeaders(), mLimitBBox ? mExtent : QgsRectangle(), mDataSource.sql() );
  if ( objectIdData.isEmpty() )
  {
    errorMessage = tr( "getObjectIds failed: %1 - %2" ).arg( errorTitle, error );
    return false;
  }
  if ( !objectIdData[QStringLiteral( "objectIdFieldName" )].isValid() || !objectIdData[QStringLiteral( "objectIds" )].isValid() )
  {
    errorMessage = tr( "Failed to determine objectIdFieldName and/or objectIds" );
    return false;
  }
  mObjectIdFieldName = objectIdData[QStringLiteral( "objectIdFieldName" )].toString();
  for ( int idx = 0, nIdx = mFields.count(); idx < nIdx; ++idx )
  {
    if ( mFields.at( idx ).name() == mObjectIdFieldName )
    {
      mObjectIdFieldIdx = idx;

      // primary key is not null, unique
      QgsFieldConstraints constraints = mFields.at( idx ).constraints();
      constraints.setConstraint( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintOriginProvider );
      constraints.setConstraint( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintOriginProvider );
      mFields[ idx ].setConstraints( constraints );
      mFields[ idx ].setReadOnly( true );

      break;
    }
  }
  const QVariantList objectIds = objectIdData.value( QStringLiteral( "objectIds" ) ).toList();
  mObjectIds.reserve( mObjectIds.size() + objectIds.size() );
  mObjectIdToFeatureId.reserve( mObjectIdToFeatureId.size() + objectIds.size() );
  for ( const QVariant &objectId : objectIds )
  {
    const int objectIdInt = objectId.toInt();
    mObjectIdToFeatureId.insert( objectIdInt, mObjectIds.size() );
    mObjectIds.append( objectIdInt );
  }
  return true;
}

quint32 QgsAfsSharedData::featureIdToObjectId( QgsFeatureId id )
{
  QgsReadWriteLocker locker( mReadWriteLock, QgsReadWriteLocker::Read );
  return mObjectIds.value( id, -1 );
}

QgsFeatureId QgsAfsSharedData::objectIdToFeatureId( quint32 oid ) const
{
  // lock must already be obtained by caller!
  return mObjectIdToFeatureId.value( oid, -1 );
}

bool QgsAfsSharedData::getFeature( QgsFeatureId id, QgsFeature &f, const QgsRectangle &filterRect, QgsFeedback *feedback )
{
  QgsReadWriteLocker locker( mReadWriteLock, QgsReadWriteLocker::Read );

  // If cached, return cached feature
  QMap<QgsFeatureId, QgsFeature>::const_iterator it = mCache.constFind( id );
  if ( it != mCache.constEnd() )
  {
    f = it.value();
    return filterRect.isNull() || ( f.hasGeometry() && f.geometry().intersects( filterRect ) );
  }

  const QString authcfg = mDataSource.authConfigId();
  bool featureFetched = false;
  int startId;
  QList<quint32> objectIds;
  QVariantMap queryData;
  while ( !featureFetched )
  {
    startId = ( id / mMaximumFetchObjectsCount ) * mMaximumFetchObjectsCount;
    const int stopId = std::min< size_t >( startId + mMaximumFetchObjectsCount, mObjectIds.length() );
    objectIds.clear();
    objectIds.reserve( stopId - startId );
    for ( int i = startId; i < stopId; ++i )
    {
      if ( i >= 0 && i < mObjectIds.count() && !mDeletedFeatureIds.contains( i ) && !mCache.contains( i ) )
        objectIds.append( mObjectIds.at( i ) );
    }

    if ( objectIds.empty() )
    {
      QgsDebugMsgLevel( QStringLiteral( "No valid features IDs to fetch" ), 2 );
      return false;
    }

    // don't lock while doing the fetch
    locker.unlock();

    // Query
    QString errorTitle, errorMessage;
    queryData = QgsArcGisRestQueryUtils::getObjects(
                  mDataSource.param( QStringLiteral( "url" ) ), authcfg, objectIds, mDataSource.param( QStringLiteral( "crs" ) ), true,
                  QStringList(), QgsWkbTypes::hasM( mGeometryType ), QgsWkbTypes::hasZ( mGeometryType ),
                  filterRect, errorTitle, errorMessage, mDataSource.httpHeaders(), feedback );

    if ( feedback && feedback->isCanceled() )
    {
      return false;
    }

    if ( queryData.isEmpty() )
    {
      if ( mMaximumFetchObjectsCount <= 1 || errorMessage.isEmpty() )
      {
        QgsDebugMsgLevel( QStringLiteral( "Query returned empty result" ), 2 );
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
  const QVariantList featuresData = queryData[QStringLiteral( "features" )].toList();
  if ( featuresData.isEmpty() )
  {
    QgsDebugMsgLevel( QStringLiteral( "Query returned no features" ), 3 );
    return false;
  }
  for ( int i = 0, n = featuresData.size(); i < n; ++i )
  {
    const QVariantMap featureData = featuresData[i].toMap();
    QgsFeature feature;
    QgsFeatureId featureId = startId + i;

    // Set attributes
    const QVariantMap attributesData = featureData[QStringLiteral( "attributes" )].toMap();
    feature.setFields( mFields );
    QgsAttributes attributes( mFields.size() );
    for ( int idx = 0; idx < mFields.size(); ++idx )
    {
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
        QgsDebugMsg( QStringLiteral( "Invalid value %1 for field %2 of type %3" ).arg( attributesData[mFields.at( idx ).name()].toString(), mFields.at( idx ).name(), mFields.at( idx ).typeName() ) );
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
    const QVariantMap geometryData = featureData[QStringLiteral( "geometry" )].toMap();
    std::unique_ptr< QgsAbstractGeometry > geometry( QgsArcGisRestUtils::convertGeometry( geometryData, queryData[QStringLiteral( "geometryType" )].toString(),
        QgsWkbTypes::hasM( mGeometryType ), QgsWkbTypes::hasZ( mGeometryType ) ) );
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
    return filterRect.isNull() || ( f.hasGeometry() && f.geometry().intersects( filterRect ) );
  }

  return false;
}

QgsFeatureIds QgsAfsSharedData::getFeatureIdsInExtent( const QgsRectangle &extent, QgsFeedback *feedback )
{
  QString errorTitle;
  QString errorText;

  const QString authcfg = mDataSource.authConfigId();
  const QList<quint32> objectIdsInRect = QgsArcGisRestQueryUtils::getObjectIdsByExtent( mDataSource.param( QStringLiteral( "url" ) ),
                                         extent, errorTitle, errorText, authcfg, mDataSource.httpHeaders(), feedback, mDataSource.sql() );

  QgsReadWriteLocker locker( mReadWriteLock, QgsReadWriteLocker::Read );
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
  QgsReadWriteLocker locker( mReadWriteLock, QgsReadWriteLocker::Read );

  QStringList stringIds;
  for ( const QgsFeatureId id : ids )
  {
    stringIds.append( QString::number( mObjectIds[id] ) );
  }
  locker.unlock();

  QUrl queryUrl( mDataSource.param( QStringLiteral( "url" ) ) + "/deleteFeatures" );

  QByteArray payload;
  payload.append( QStringLiteral( "f=json&objectIds=%1" ).arg( stringIds.join( ',' ) ).toUtf8() );

  bool ok = false;
  postData( queryUrl, payload, feedback, ok, error );
  if ( ! ok )
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
  QUrl queryUrl( mDataSource.param( QStringLiteral( "url" ) ) + "/addFeatures" );

  QgsArcGisRestContext context;

  QVariantList featuresJson;
  featuresJson.reserve( features.size() );
  for ( const QgsFeature &feature : features )
  {
    featuresJson.append( QgsArcGisRestUtils::featureToJson( feature, context ) );
  }

  const QString json = QString::fromStdString( QgsJsonUtils::jsonFromVariant( featuresJson ).dump( 2 ) );

  QByteArray payload;
  payload.append( QStringLiteral( "f=json&features=%1" ).arg( json ).toUtf8() );

  bool ok = false;
  const QVariantMap results = postData( queryUrl, payload, feedback, ok, errorMessage );
  if ( !ok )
  {
    return false;
  }

  const QVariantList addResults = results.value( QStringLiteral( "addResults" ) ).toList();
  for ( const QVariant &result : addResults )
  {
    const QVariantMap resultMap = result.toMap();
    if ( !resultMap.value( QStringLiteral( "success" ) ).toBool() )
    {
      errorMessage = resultMap.value( QStringLiteral( "error" ) ).toMap().value( QStringLiteral( "description" ) ).toString();
      return false;
    }
  }

  // All good!
  QgsReadWriteLocker locker( mReadWriteLock, QgsReadWriteLocker::Write );
  int i = 0;
  for ( const QVariant &result : addResults )
  {
    const QVariantMap resultMap = result.toMap();
    const long long objectId = resultMap.value( QStringLiteral( "objectId" ) ).toLongLong();

    const QgsFeatureId newId = mObjectIds.size();
    features[i].setId( newId );
    mObjectIdToFeatureId.insert( objectId, newId );
    mObjectIds.append( objectId );

    i++;
  }
  return true;
}

bool QgsAfsSharedData::updateFeatures( const QgsFeatureList &features, bool includeGeometries, bool includeAttributes, QString &error, QgsFeedback *feedback )
{
  error.clear();
  QUrl queryUrl( mDataSource.param( QStringLiteral( "url" ) ) + "/updateFeatures" );

  QgsArcGisRestContext context;
  context.setObjectIdFieldName( mObjectIdFieldName );

  QgsArcGisRestUtils::FeatureToJsonFlags flags;
  if ( includeGeometries )
    flags |= QgsArcGisRestUtils::FeatureToJsonFlag::IncludeGeometry;
  if ( includeAttributes )
    flags |= QgsArcGisRestUtils::FeatureToJsonFlag::IncludeNonObjectIdAttributes;

  QVariantList featuresJson;
  featuresJson.reserve( features.size() );
  for ( const QgsFeature &feature : features )
  {
    featuresJson.append( QgsArcGisRestUtils::featureToJson( feature, context, QgsCoordinateReferenceSystem(), flags ) );
  }

  const QString json = QString::fromStdString( QgsJsonUtils::jsonFromVariant( featuresJson ).dump( 2 ) );

  QByteArray payload;
  payload.append( QStringLiteral( "f=json&features=%1" ).arg( json ).toUtf8() );

  bool ok = false;
  const QVariantMap results = postData( queryUrl, payload, feedback, ok, error );
  if ( !ok )
  {
    return false;
  }

  const QVariantList addResults = results.value( QStringLiteral( "updateResults" ) ).toList();
  for ( const QVariant &result : addResults )
  {
    const QVariantMap resultMap = result.toMap();
    if ( !resultMap.value( QStringLiteral( "success" ) ).toBool() )
    {
      error = resultMap.value( QStringLiteral( "error" ) ).toMap().value( QStringLiteral( "description" ) ).toString();
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

  const QVariantMap definition {{ QStringLiteral( "fields" ), fieldsJson }};

  const QString json = QString::fromStdString( QgsJsonUtils::jsonFromVariant( definition ).dump( 2 ) );

  QByteArray payload;
  payload.append( QStringLiteral( "f=json&addToDefinition=%1" ).arg( json ).toUtf8() );

  bool ok = false;
  const QVariantMap results = postData( queryUrl, payload, feedback, ok, error );
  if ( !ok )
  {
    return false;
  }

  if ( !results.value( QStringLiteral( "success" ) ).toBool() )
  {
    error = results.value( QStringLiteral( "error" ) ).toMap().value( QStringLiteral( "message" ) ).toString();
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
  QUrl queryUrl( adminUrl + "/deleteFromDefinition" );

  QVariantList fieldsJson;
  fieldsJson.reserve( attributes.size() );
  QStringList fieldNames;
  for ( int index : attributes )
  {
    if ( index >= 0 && index < mFields.count() )
    {
      fieldsJson.append( QVariantMap( {{QStringLiteral( "name" ), mFields.at( index ).name() }} ) );
      fieldNames << mFields.at( index ).name();
    }
  }

  const QVariantMap definition {{ QStringLiteral( "fields" ), fieldsJson }};

  const QString json = QString::fromStdString( QgsJsonUtils::jsonFromVariant( definition ).dump( 2 ) );

  QByteArray payload;
  payload.append( QStringLiteral( "f=json&deleteFromDefinition=%1" ).arg( json ).toUtf8() );

  bool ok = false;
  const QVariantMap results = postData( queryUrl, payload, feedback, ok, error );
  if ( !ok )
  {
    return false;
  }

  if ( !results.value( QStringLiteral( "success" ) ).toBool() )
  {
    error = results.value( QStringLiteral( "error" ) ).toMap().value( QStringLiteral( "message" ) ).toString();
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
  QUrl queryUrl( adminUrl + "/addToDefinition" );

  const QString name = mFields.field( attribute ).name();


  QVariantList indexJson;
  indexJson << QVariantMap(
  {
    {QStringLiteral( "name" ), QStringLiteral( "%1_index" ).arg( name )},
    {QStringLiteral( "fields" ), name},
    {QStringLiteral( "description" ), name}
  } );


  const QVariantMap definition {{ QStringLiteral( "indexes" ), indexJson }};

  const QString json = QString::fromStdString( QgsJsonUtils::jsonFromVariant( definition ).dump( 2 ) );

  QByteArray payload;
  payload.append( QStringLiteral( "f=json&addToDefinition=%1" ).arg( json ).toUtf8() );

  bool ok = false;
  const QVariantMap results = postData( queryUrl, payload, feedback, ok, error );
  if ( !ok )
  {
    return false;
  }

  if ( !results.value( QStringLiteral( "success" ) ).toBool() )
  {
    error = results.value( QStringLiteral( "error" ) ).toMap().value( QStringLiteral( "message" ) ).toString();
    return false;
  }

  return true;
}

bool QgsAfsSharedData::hasCachedAllFeatures() const
{
  QgsReadWriteLocker locker( mReadWriteLock, QgsReadWriteLocker::Read );
  return mCache.count() == featureCount();
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
    QgsDebugMsg( QStringLiteral( "payload file is %1" ).arg( localFile ) );
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
  request.setHeader( QNetworkRequest::ContentTypeHeader, QLatin1String( "application/x-www-form-urlencoded" ) );
  QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsArcGisRestUtils" ) );

  QgsBlockingNetworkRequest networkRequest;
  networkRequest.setAuthCfg( mDataSource.authConfigId() );

  const QgsBlockingNetworkRequest::ErrorCode error = networkRequest.post( request, payload, false, feedback );

// Handle network errors
  if ( error != QgsBlockingNetworkRequest::NoError )
  {
    QgsDebugMsg( QStringLiteral( "Network error: %1" ).arg( networkRequest.errorMessage() ) );
    errorText = networkRequest.errorMessage();

    // try to get detailed error message from reply
    const QString content = networkRequest.reply().content();
    const thread_local QRegularExpression errorRx( QStringLiteral( "Error: <.*?>(.*?)<" ) );
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
