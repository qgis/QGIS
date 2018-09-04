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

#include "qgsfeaturerequest.h"

template <typename Func>
void runOnMainThread( const Func &func )
{
#if QT_VERSION >= QT_VERSION_CHECK( 5, 10, 0 )
  // Make sure we only deal with the vector layer on the main thread where it lives.
  // Anything else risks a crash.
  if ( QThread::currentThread() == qApp->thread() )
    func();
  else
    QMetaObject::invokeMethod( qApp, func, Qt::BlockingQueuedConnection );
#else
  func();
#endif
}

QgsVectorDataProviderFeaturePool::QgsVectorDataProviderFeaturePool( QgsVectorLayer *layer, double layerToMapUnits, const QgsCoordinateTransform &layerToMapTransform, bool selectedOnly )
  : QgsFeaturePool( layer, layerToMapUnits, layerToMapTransform )
  , mSelectedOnly( selectedOnly )
{
  // Build spatial index
  QgsFeature feature;
  QgsFeatureRequest req;
  req.setSubsetOfAttributes( QgsAttributeList() );
  QgsFeatureIds featureIds;
  if ( selectedOnly )
  {
    featureIds = layer->selectedFeatureIds();
    req.setFilterFids( featureIds );
  }

  QgsFeatureIterator it = layer->getFeatures( req );
  while ( it.nextFeature( feature ) )
  {
    if ( feature.geometry() )
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

void QgsVectorDataProviderFeaturePool::addFeature( QgsFeature &feature )
{
  QgsFeatureList features;
  features.append( feature );

  auto addFeatureSynchronized = [ this, &features ]()
  {
    QgsVectorLayer *lyr = layer();
    if ( lyr )
      lyr->dataProvider()->addFeatures( features );
  };

  runOnMainThread( addFeatureSynchronized );

  feature.setId( features.front().id() );
  if ( mSelectedOnly )
  {
    runOnMainThread( [ this, feature ]()
    {
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
}

void QgsVectorDataProviderFeaturePool::updateFeature( QgsFeature &feature )
{
  QgsFeature origFeature;
  get( feature.id(), origFeature );

  QgsGeometryMap geometryMap;
  geometryMap.insert( feature.id(), feature.geometry() );
  QgsChangedAttributesMap changedAttributesMap;
  QgsAttributeMap attribMap;
  for ( int i = 0, n = feature.attributes().size(); i < n; ++i )
  {
    attribMap.insert( i, feature.attributes().at( i ) );
  }
  changedAttributesMap.insert( feature.id(), attribMap );

  removeFeature( origFeature.id() );
  runOnMainThread( [this, geometryMap, changedAttributesMap]()
  {
    QgsVectorLayer *lyr = layer();
    if ( lyr )
    {
      lyr->dataProvider()->changeGeometryValues( geometryMap );
      lyr->dataProvider()->changeAttributeValues( changedAttributesMap );
    }
  } );

  insertFeature( feature );
}

void QgsVectorDataProviderFeaturePool::deleteFeature( QgsFeatureId fid )
{
  removeFeature( fid );
  runOnMainThread( [this, fid]()
  {
    QgsVectorLayer *lyr = layer();
    if ( lyr )
    {
      lyr->dataProvider()->deleteFeatures( QgsFeatureIds() << fid );
    }
  } );
}
