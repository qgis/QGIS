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
#include "qgsgeometryutils.h"

#include <QMutexLocker>
#include <limits>

QgsFeaturePool::QgsFeaturePool( QgsVectorLayer *layer, double layerToMapUnits, const QgsCoordinateTransform &layerToMapTransform, bool selectedOnly )
  : mFeatureCache( CACHE_SIZE )
  , mLayer( layer )
  , mLayerToMapUnits( layerToMapUnits )
  , mLayerToMapTransform( layerToMapTransform )
  , mSelectedOnly( selectedOnly )
{
  // Build spatial index
  QgsFeature feature;
  QgsFeatureRequest req;
  req.setSubsetOfAttributes( QgsAttributeList() );
  if ( selectedOnly )
  {
    mFeatureIds = layer->selectedFeatureIds();
    req.setFilterFids( mFeatureIds );
  }

  QgsFeatureIterator it = layer->getFeatures( req );
  while ( it.nextFeature( feature ) )
  {
    if ( feature.geometry() )
    {
      mIndex.insertFeature( feature );
      mFeatureIds.insert( feature.id() );
    }
    else
    {
      mFeatureIds.remove( feature.id() );
    }
  }
}

bool QgsFeaturePool::get( QgsFeatureId id, QgsFeature &feature )
{
  QMutexLocker lock( &mLayerMutex );
  QgsFeature *pfeature = mFeatureCache.object( id );
  if ( pfeature )
  {
    //feature was cached
    feature = *pfeature;
  }

  // Feature not in cache, retrieve from layer
  pfeature = new QgsFeature();
  // TODO: avoid always querying all attributes (attribute values are needed when merging by attribute)
  if ( !mLayer->getFeatures( QgsFeatureRequest( id ) ).nextFeature( *pfeature ) )
  {
    delete pfeature;
    return false;
  }
  //make a copy of pfeature into feature parameter
  feature = QgsFeature( *pfeature );
  //ownership of pfeature is transferred to cache
  mFeatureCache.insert( id, pfeature );
  return true;
}

void QgsFeaturePool::addFeature( QgsFeature &feature )
{
  QgsFeatureList features;
  features.append( feature );
  mLayerMutex.lock();
  mLayer->dataProvider()->addFeatures( features );
  feature.setId( features.front().id() );
  if ( mSelectedOnly )
  {
    QgsFeatureIds selectedFeatureIds = mLayer->selectedFeatureIds();
    selectedFeatureIds.insert( feature.id() );
    mLayer->selectByIds( selectedFeatureIds );
  }
  mLayerMutex.unlock();
  mIndexMutex.lock();
  mIndex.insertFeature( feature );
  mIndexMutex.unlock();
}

void QgsFeaturePool::updateFeature( QgsFeature &feature )
{
  QgsFeature origFeature;
  get( feature.id(), origFeature );

  QgsGeometryMap geometryMap;
  geometryMap.insert( feature.id(), QgsGeometry( feature.geometry().constGet()->clone() ) );
  QgsChangedAttributesMap changedAttributesMap;
  QgsAttributeMap attribMap;
  for ( int i = 0, n = feature.attributes().size(); i < n; ++i )
  {
    attribMap.insert( i, feature.attributes().at( i ) );
  }
  changedAttributesMap.insert( feature.id(), attribMap );
  mLayerMutex.lock();
  mFeatureCache.remove( feature.id() ); // Remove to force reload on next get()
  mLayer->dataProvider()->changeGeometryValues( geometryMap );
  mLayer->dataProvider()->changeAttributeValues( changedAttributesMap );
  mLayerMutex.unlock();
  mIndexMutex.lock();
  mIndex.deleteFeature( origFeature );
  mIndex.insertFeature( feature );
  mIndexMutex.unlock();
}

void QgsFeaturePool::deleteFeature( const QgsFeatureId &fid )
{
  QgsFeature origFeature;
  if ( get( fid, origFeature ) )
  {
    mIndexMutex.lock();
    mIndex.deleteFeature( origFeature );
    mIndexMutex.unlock();
  }
  mLayerMutex.lock();
  mFeatureCache.remove( origFeature.id() );
  mLayer->dataProvider()->deleteFeatures( QgsFeatureIds() << fid );
  mLayerMutex.unlock();
}

QgsFeatureIds QgsFeaturePool::getIntersects( const QgsRectangle &rect ) const
{
  QMutexLocker lock( &mIndexMutex );
  return QgsFeatureIds::fromList( mIndex.intersects( rect ) );
}
