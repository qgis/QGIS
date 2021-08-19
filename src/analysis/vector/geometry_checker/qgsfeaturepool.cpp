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
#include "qgsvectorlayerutils.h"
#include "qgsreadwritelocker.h"

#include <QMutexLocker>
#include <QThread>



QgsFeaturePool::QgsFeaturePool( QgsVectorLayer *layer )
  : mFeatureCache( CACHE_SIZE )
  , mLayer( layer )
  , mGeometryType( layer->geometryType() )
  , mFeatureSource( std::make_unique<QgsVectorLayerFeatureSource>( layer ) )
  , mLayerName( layer->name() )
{

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

  QgsReadWriteLocker locker( mCacheLock, QgsReadWriteLocker::Write );
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
  QgsReadWriteLocker( mCacheLock, QgsReadWriteLocker::Write );
  Q_UNUSED( feedback )
  Q_ASSERT( QThread::currentThread() == qApp->thread() );

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
  Q_ASSERT( QThread::currentThread() == qApp->thread() );

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

void QgsFeaturePool::refreshCache( const QgsFeature &feature )
{
  QgsReadWriteLocker locker( mCacheLock, QgsReadWriteLocker::Write );
  mFeatureCache.remove( feature.id() );
  mIndex.deleteFeature( feature );
  locker.unlock();

  QgsFeature tempFeature;
  getFeature( feature.id(), tempFeature );
}

void QgsFeaturePool::removeFeature( const QgsFeatureId featureId )
{
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
  QgsReadWriteLocker( mCacheLock, QgsReadWriteLocker::Read );
  return mFeatureSource->crs();
}

QgsWkbTypes::GeometryType QgsFeaturePool::geometryType() const
{
  return mGeometryType;
}

QString QgsFeaturePool::layerId() const
{
  QgsReadWriteLocker( mCacheLock, QgsReadWriteLocker::Read );
  return mFeatureSource->id();
}
