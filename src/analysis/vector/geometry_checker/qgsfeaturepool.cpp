/***************************************************************************
 *  qgsfeaturepool.cpp                                                     *
 *  -------------------                                                    *
 *  copyright            : (C) 2014 by Sandro Mani / Sourcepole AG         *
 *  email                : smani@sourcepole.ch                             *
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfeaturepool.h"
#include "qgsfeature.h"
#include "qgsfeatureiterator.h"
#include "qgsgeometry.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsreadwritelocker.h"

#include <QMutexLocker>
#include <QThread>


QgsFeaturePool::QgsFeaturePool( QgsVectorLayer *layer )
  : mFeatureCache( CACHE_SIZE )
  , mLayer( layer )
  , mGeometryType( layer->geometryType() )
  , mFeatureSource( std::make_unique<QgsVectorLayerFeatureSource>( layer ) )
  , mLayerId( layer->id() )
  , mLayerName( layer->name() )
  , mCrs( layer->crs() )
{
  Q_ASSERT( QThread::currentThread() == mLayer->thread() );
}

bool QgsFeaturePool::getFeature( QgsFeatureId id, QgsFeature &feature )
{
  // Why is there a write lock acquired here? Weird, we only want to read a feature from the cache, right?
  // A method like `QCache::object(const Key &key) const` certainly would not modify its internals.
  // Mmmh. What if reality was different?
  // If one reads the docs very, very carefully one will find the term "reentrant" in the
  // small print of the QCache docs. This is the hint that reality is different.
  //
  // https://bugreports.qt.io/browse/QTBUG-19794

  // If the feature we want is amongst the features that have been updated,
  // then get it from the dedicated hash.
  // It would not be thread-safe to get it directly from the layer,
  // and it could be outdated in the feature source (in case of a memory layer),
  // and it could have been cleared from the cache due to a cache overflow.
  if ( mUpdatedFeatures.contains( id ) )
  {
    feature = mUpdatedFeatures[id];
    return true;
  }

  QgsReadWriteLocker locker( mCacheLock, QgsReadWriteLocker::Read );
  QgsFeature *cachedFeature = mFeatureCache.object( id );
  if ( cachedFeature )
  {
    //feature was cached
    feature = *cachedFeature;
  }
  else
  {
    // Feature not in cache, retrieve from layer
    // TODO: avoid always querying all attributes (attribute values are needed when merging by attribute)
    if ( !mFeatureSource->getFeatures( QgsFeatureRequest( id ) ).nextFeature( feature ) )
    {
      return false;
    }
    locker.changeMode( QgsReadWriteLocker::Write );
    mFeatureCache.insert( id, new QgsFeature( feature ) );
    mIndex.addFeature( feature );
  }
  return true;
}

QgsFeatureIds QgsFeaturePool::getFeatures( const QgsFeatureRequest &request, QgsFeedback *feedback )
{
  QgsReadWriteLocker locker( mCacheLock, QgsReadWriteLocker::Write );
  Q_UNUSED( feedback )
  Q_ASSERT( mLayer );
  Q_ASSERT( QThread::currentThread() == mLayer->thread() );

  mUpdatedFeatures.clear();
  mFeatureCache.clear();
  mIndex = QgsSpatialIndex();

  QgsFeatureIds fids;

  mFeatureSource = std::make_unique<QgsVectorLayerFeatureSource>( mLayer );

  QgsFeatureIterator it = mFeatureSource->getFeatures( request );
  QgsFeature feature;
  while ( it.nextFeature( feature ) )
  {
    insertFeature( feature, true );
    fids << feature.id();
  }

  return fids;
}

QgsFeatureIds QgsFeaturePool::allFeatureIds() const
{
  return mFeatureIds;
}

QgsFeatureIds QgsFeaturePool::getIntersects( const QgsRectangle &rect ) const
{
  const QgsReadWriteLocker locker( mCacheLock, QgsReadWriteLocker::Read );
  QgsFeatureIds ids = qgis::listToSet( mIndex.intersects( rect ) );
  return ids;
}

QgsVectorLayer *QgsFeaturePool::layer() const
{
  if ( mLayer )
    Q_ASSERT( QThread::currentThread() == mLayer->thread() );

  return mLayer.data();
}

QPointer<QgsVectorLayer> QgsFeaturePool::layerPtr() const
{
  return mLayer;
}

void QgsFeaturePool::insertFeature( const QgsFeature &feature, bool skipLock )
{
  QgsReadWriteLocker locker( mCacheLock, QgsReadWriteLocker::Unlocked );
  if ( !skipLock )
    locker.changeMode( QgsReadWriteLocker::Write );
  mFeatureCache.insert( feature.id(), new QgsFeature( feature ) );
  QgsFeature indexFeature( feature );
  mIndex.addFeature( indexFeature );
}

void QgsFeaturePool::refreshCache( QgsFeature feature, const QgsFeature &origFeature )
{
  // insert/refresh the updated features as well
  mUpdatedFeatures.insert( feature.id(), feature );

  QgsReadWriteLocker locker( mCacheLock, QgsReadWriteLocker::Write );
  mFeatureCache.insert( feature.id(), new QgsFeature( feature ) );
  mIndex.deleteFeature( origFeature );
  mIndex.addFeature( feature );
  locker.unlock();
}

void QgsFeaturePool::removeFeature( const QgsFeatureId featureId )
{
  mUpdatedFeatures.remove( featureId );
  QgsFeature origFeature;
  QgsReadWriteLocker locker( mCacheLock, QgsReadWriteLocker::Unlocked );
  if ( getFeature( featureId, origFeature ) )
  {
    locker.changeMode( QgsReadWriteLocker::Write );
    mIndex.deleteFeature( origFeature );
  }
  locker.changeMode( QgsReadWriteLocker::Write );
  mFeatureCache.remove( origFeature.id() );
}

void QgsFeaturePool::setFeatureIds( const QgsFeatureIds &ids )
{
  mFeatureIds = ids;
}

bool QgsFeaturePool::isFeatureCached( QgsFeatureId fid )
{
  const QgsReadWriteLocker locker( mCacheLock, QgsReadWriteLocker::Read );
  return mFeatureCache.contains( fid );
}

QString QgsFeaturePool::layerName() const
{
  return mLayerName;
}

QgsCoordinateReferenceSystem QgsFeaturePool::crs() const
{
  return mCrs;
}

Qgis::GeometryType QgsFeaturePool::geometryType() const
{
  return mGeometryType;
}

QString QgsFeaturePool::layerId() const
{
  return mLayerId;
}
