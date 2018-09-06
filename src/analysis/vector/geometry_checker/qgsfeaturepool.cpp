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


QgsFeaturePool::QgsFeaturePool( QgsVectorLayer *layer, double layerToMapUnits, const QgsCoordinateTransform &layerToMapTransform )
  : mFeatureCache( CACHE_SIZE )
  , mLayer( layer )
  , mLayerToMapUnits( layerToMapUnits )
  , mLayerToMapTransform( layerToMapTransform )
  , mLayerId( layer->id() )
  , mGeometryType( layer->geometryType() )
{

}

bool QgsFeaturePool::get( QgsFeatureId id, QgsFeature &feature )
{
  QgsReadWriteLocker locker( mCacheLock, QgsReadWriteLocker::Read );
  QgsFeature *cachedFeature = mFeatureCache.object( id );
  if ( cachedFeature )
  {
    //feature was cached
    feature = *cachedFeature;
  }
  else
  {
    std::unique_ptr<QgsVectorLayerFeatureSource> source = QgsVectorLayerUtils::getFeatureSource( mLayer );

    // Feature not in cache, retrieve from layer
    // TODO: avoid always querying all attributes (attribute values are needed when merging by attribute)
    if ( !source->getFeatures( QgsFeatureRequest( id ) ).nextFeature( feature ) )
    {
      return false;
    }
    locker.changeMode( QgsReadWriteLocker::Write );
    mFeatureCache.insert( id, new QgsFeature( feature ) );
    mIndex.insertFeature( feature );
  }
  return true;
}

QgsFeatureIds QgsFeaturePool::getFeatureIds() const
{
  return mFeatureIds;
}

QgsFeatureIds QgsFeaturePool::getIntersects( const QgsRectangle &rect ) const
{
  QgsReadWriteLocker locker( mIndexLock, QgsReadWriteLocker::Read );
  QgsFeatureIds ids = QgsFeatureIds::fromList( mIndex.intersects( rect ) );
  return ids;
}

QgsVectorLayer *QgsFeaturePool::layer() const
{
  Q_ASSERT( QThread::currentThread() == qApp->thread() );

  return mLayer.data();
}

void QgsFeaturePool::insertFeature( const QgsFeature &feature )
{
  QgsReadWriteLocker locker( mIndexLock, QgsReadWriteLocker::Write );
  mFeatureCache.insert( feature.id(), new QgsFeature( feature ) );
  mIndex.insertFeature( feature );
}

void QgsFeaturePool::refreshCache( const QgsFeature &feature )
{
  QgsReadWriteLocker locker( mIndexLock, QgsReadWriteLocker::Write );
  mFeatureCache.remove( feature.id() );
  mIndex.deleteFeature( feature );
  locker.unlock();

  QgsFeature tempFeature;
  get( feature.id(), tempFeature );
}

void QgsFeaturePool::removeFeature( const QgsFeatureId featureId )
{
  QgsFeature origFeature;
  QgsReadWriteLocker locker( mIndexLock, QgsReadWriteLocker::Unlocked );
  if ( get( featureId, origFeature ) )
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

QgsWkbTypes::GeometryType QgsFeaturePool::geometryType() const
{
  return mGeometryType;
}

QString QgsFeaturePool::layerId() const
{
  return mLayerId;
}
