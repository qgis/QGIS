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

void QgsAfsSharedData::clearCache()
{
  const QMutexLocker locker( &mMutex );
  mCache.clear();
}

bool QgsAfsSharedData::getFeature( QgsFeatureId id, QgsFeature &f, const QgsRectangle &filterRect, QgsFeedback *feedback )
{
  QMutexLocker locker( &mMutex );

  // If cached, return cached feature
  QMap<QgsFeatureId, QgsFeature>::const_iterator it = mCache.constFind( id );
  if ( it != mCache.constEnd() )
  {
    f = it.value();
    return filterRect.isNull() || ( f.hasGeometry() && f.geometry().intersects( filterRect ) );
  }

  // Fetch 100 features at the time
  const int startId = ( id / 100 ) * 100;
  const int stopId = std::min< size_t >( startId + 100, mObjectIds.length() );
  QList<quint32> objectIds;
  objectIds.reserve( stopId );
  for ( int i = startId; i < stopId; ++i )
  {
    if ( i >= 0 && i < mObjectIds.count() )
      objectIds.append( mObjectIds[i] );
  }

  if ( objectIds.empty() )
  {
    QgsDebugMsg( QStringLiteral( "No valid features IDs to fetch" ) );
    return false;
  }

  // don't lock while doing the fetch
  locker.unlock();

  // Query
  QString errorTitle, errorMessage;

  const QString authcfg = mDataSource.authConfigId();
  QgsHttpHeaders headers;
  const QString referer = mDataSource.param( QStringLiteral( "referer" ) );
  if ( !referer.isEmpty() )
    headers[ QStringLiteral( "Referer" )] = referer;

  const QVariantMap queryData = QgsArcGisRestQueryUtils::getObjects(
                                  mDataSource.param( QStringLiteral( "url" ) ), authcfg, objectIds, mDataSource.param( QStringLiteral( "crs" ) ), true,
                                  QStringList(), QgsWkbTypes::hasM( mGeometryType ), QgsWkbTypes::hasZ( mGeometryType ),
                                  filterRect, errorTitle, errorMessage, headers, feedback );

  if ( queryData.isEmpty() )
  {
//    const_cast<QgsAfsProvider *>( this )->pushError( errorTitle + ": " + errorMessage );
    QgsDebugMsg( QStringLiteral( "Query returned empty result" ) );
    return false;
  }

  // but re-lock while updating cache
  locker.relock();
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
    int featureId = startId + i;

    // Set attributes
    const QVariantMap attributesData = featureData[QStringLiteral( "attributes" )].toMap();
    feature.setFields( mFields );
    QgsAttributes attributes( mFields.size() );
    for ( int idx = 0; idx < mFields.size(); ++idx )
    {
      QVariant attribute = attributesData[mFields.at( idx ).name()];
      if ( attribute.isNull() )
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
        featureId = startId + objectIds.indexOf( attributesData[mFields.at( idx ).name()].toInt() );
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
  QgsHttpHeaders headers;
  const QString referer = mDataSource.param( QStringLiteral( "referer" ) );
  if ( !referer.isEmpty() )
    headers[ QStringLiteral( "Referer" )] = referer;
  const QList<quint32> featuresInRect = QgsArcGisRestQueryUtils::getObjectIdsByExtent( mDataSource.param( QStringLiteral( "url" ) ),
                                        extent, errorTitle, errorText, authcfg, headers, feedback );

  QgsFeatureIds ids;
  for ( const quint32 id : featuresInRect )
  {
    const int featureId = mObjectIds.indexOf( id );
    if ( featureId >= 0 )
      ids.insert( featureId );
  }
  return ids;
}

bool QgsAfsSharedData::hasCachedAllFeatures() const
{
  return mCache.count() == mObjectIds.count();
}
