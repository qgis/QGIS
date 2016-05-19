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
#include "qgsgeometry.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsgeomutils.h"

#include <QMutexLocker>
#include <qmath.h>
#include <limits>

QgsFeaturePool::QgsFeaturePool( QgsVectorLayer *layer, bool selectedOnly )
    : mFeatureCache( sCacheSize )
    , mLayer( layer )
    , mSelectedOnly( selectedOnly )
{
  if ( selectedOnly )
  {
    mFeatureIds = layer->selectedFeaturesIds();
  }
  else
  {
    mFeatureIds = layer->allFeatureIds();
  }

  // Build spatial index
  QgsFeature feature;
  QgsFeatureRequest req;
  req.setSubsetOfAttributes( QgsAttributeList() );
  QgsFeatureIterator it = layer->getFeatures( req );
  while ( it.nextFeature( feature ) )
  {
    mIndex.insertFeature( feature );
  }
}

bool QgsFeaturePool::get( QgsFeatureId id , QgsFeature& feature )
{
  QMutexLocker lock( &mLayerMutex );
  QgsFeature* pfeature = mFeatureCache.object( id );
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

void QgsFeaturePool::addFeature( QgsFeature& feature )
{
  QgsFeatureList features;
  features.append( feature );
  mLayerMutex.lock();
  mLayer->dataProvider()->addFeatures( features );
  feature.setFeatureId( features.front().id() );
  if ( mSelectedOnly )
  {
    QgsFeatureIds selectedFeatureIds = mLayer->selectedFeaturesIds();
    selectedFeatureIds.insert( feature.id() );
    mLayer->selectByIds( selectedFeatureIds );
  }
  mLayerMutex.unlock();
  mIndexMutex.lock();
  mIndex.insertFeature( feature );
  mIndexMutex.unlock();
}

void QgsFeaturePool::updateFeature( QgsFeature& feature )
{
  QgsGeometryMap geometryMap;
  geometryMap.insert( feature.id(), QgsGeometry( feature.geometry()->geometry()->clone() ) );
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
  mIndex.deleteFeature( feature );
  mIndex.insertFeature( feature );
  mIndexMutex.unlock();
}

void QgsFeaturePool::deleteFeature( QgsFeature& feature )
{
  mIndexMutex.lock();
  mIndex.deleteFeature( feature );
  mIndexMutex.unlock();
  mLayerMutex.lock();
  mFeatureCache.remove( feature.id() );
  mLayer->dataProvider()->deleteFeatures( QgsFeatureIds() << feature.id() );
  mLayerMutex.unlock();
}

QgsFeatureIds QgsFeaturePool::getIntersects( const QgsRectangle &rect )
{
  QMutexLocker lock( &mIndexMutex );
  return QgsFeatureIds::fromList( mIndex.intersects( rect ) );
}
