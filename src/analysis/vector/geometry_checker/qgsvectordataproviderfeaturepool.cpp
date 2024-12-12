/***************************************************************************
                      qgsvectordataproviderfeaturepool.h
                     --------------------------------------
Date                 : 3.9.2018
Copyright            : (C) 2018 by Matthias Kuhn
email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectordataproviderfeaturepool.h"
#include "qgsthreadingutils.h"

#include "qgsfeaturerequest.h"

QgsVectorDataProviderFeaturePool::QgsVectorDataProviderFeaturePool( QgsVectorLayer *layer, bool selectedOnly )
  : QgsFeaturePool( layer )
  , mSelectedOnly( selectedOnly )
{
  // Build spatial index
  QgsFeature feature;
  QgsFeatureRequest req;
  QgsFeatureIds featureIds;
  if ( selectedOnly )
  {
    featureIds = layer->selectedFeatureIds();
    req.setFilterFids( featureIds );
  }

  QgsFeatureIterator it = layer->getFeatures( req );
  while ( it.nextFeature( feature ) )
  {
    if ( feature.hasGeometry() && !feature.geometry().isEmpty() )
    {
      insertFeature( feature );
      featureIds.insert( feature.id() );
    }
    else
    {
      featureIds.remove( feature.id() );
    }
  }
  setFeatureIds( featureIds );
}

bool QgsVectorDataProviderFeaturePool::addFeature( QgsFeature &feature, Flags flags )
{
  Q_UNUSED( flags )
  QgsFeatureList features;
  features.append( feature );

  bool res = false;

  auto addFeatureSynchronized = [this, &features, &res]() {
    QgsVectorLayer *lyr = layer();
    if ( lyr )
      res = lyr->dataProvider()->addFeatures( features );
  };

  QgsThreadingUtils::runOnMainThread( addFeatureSynchronized );

  if ( !res )
    return false;

  feature.setId( features.front().id() );
  if ( mSelectedOnly )
  {
    QgsThreadingUtils::runOnMainThread( [this, feature]() {
      QgsVectorLayer *lyr = layer();
      if ( lyr )
      {
        QgsFeatureIds selectedFeatureIds = lyr->selectedFeatureIds();
        selectedFeatureIds.insert( feature.id() );
        lyr->selectByIds( selectedFeatureIds );
      }
    } );
  }

  insertFeature( feature );

  return res;
}

bool QgsVectorDataProviderFeaturePool::addFeatures( QgsFeatureList &features, QgsFeatureSink::Flags flags )
{
  Q_UNUSED( flags )

  bool res = false;

  auto addFeatureSynchronized = [this, &features, &res]() {
    QgsVectorLayer *lyr = layer();
    if ( lyr )
      res = lyr->dataProvider()->addFeatures( features );
  };

  QgsThreadingUtils::runOnMainThread( addFeatureSynchronized );

  if ( !res )
    return false;

  if ( mSelectedOnly )
  {
    QgsThreadingUtils::runOnMainThread( [this, features]() {
      QgsVectorLayer *lyr = layer();
      if ( lyr )
      {
        QgsFeatureIds selectedFeatureIds = lyr->selectedFeatureIds();
        for ( const QgsFeature &feature : std::as_const( features ) )
          selectedFeatureIds.insert( feature.id() );
        lyr->selectByIds( selectedFeatureIds );
      }
    } );
  }

  for ( const QgsFeature &feature : std::as_const( features ) )
    insertFeature( feature );

  return res;
}

void QgsVectorDataProviderFeaturePool::updateFeature( QgsFeature &feature )
{
  QgsFeature origFeature;
  getFeature( feature.id(), origFeature );

  QgsGeometryMap geometryMap;
  geometryMap.insert( feature.id(), feature.geometry() );
  QgsChangedAttributesMap changedAttributesMap;
  QgsAttributeMap attribMap;
  const int attributeCount = feature.attributeCount();
  for ( int i = 0, n = attributeCount; i < n; ++i )
  {
    attribMap.insert( i, feature.attributes().at( i ) );
  }
  changedAttributesMap.insert( feature.id(), attribMap );

  QgsThreadingUtils::runOnMainThread( [this, geometryMap, changedAttributesMap]() {
    QgsVectorLayer *lyr = layer();
    if ( lyr )
    {
      lyr->dataProvider()->changeGeometryValues( geometryMap );
      lyr->dataProvider()->changeAttributeValues( changedAttributesMap );
    }
  } );

  refreshCache( feature, origFeature );
}

void QgsVectorDataProviderFeaturePool::deleteFeature( QgsFeatureId fid )
{
  removeFeature( fid );
  QgsThreadingUtils::runOnMainThread( [this, fid]() {
    QgsVectorLayer *lyr = layer();
    if ( lyr )
    {
      lyr->dataProvider()->deleteFeatures( QgsFeatureIds() << fid );
    }
  } );
}
