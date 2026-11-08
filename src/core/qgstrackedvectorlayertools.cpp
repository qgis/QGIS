/***************************************************************************
  qgstrackedvectorlayertools.cpp - QgsTrackedVectorLayerTools

 ---------------------
 begin                : 16.5.2016
 copyright            : (C) 2016 by Matthias Kuhn, OPENGIS.ch
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstrackedvectorlayertools.h"

#include "qgsvectorlayer.h"
#include "qgsvectorlayertoolscontext.h"

#include "moc_qgstrackedvectorlayertools.cpp"

bool QgsTrackedVectorLayerTools::addFeatureV2(
  QgsVectorLayer *layer, const QgsAttributeMap &defaultValues, const QgsGeometry &defaultGeometry, QgsFeature *feature, const QgsVectorLayerToolsContext &context
) const
{
  QgsFeature *f = feature;
  if ( !feature )
    f = new QgsFeature();

  const_cast<QgsVectorLayerTools *>( mBackend )->setForceSuppressFormPopup( forceSuppressFormPopup() );
  if ( mBackend->addFeatureV2( layer, defaultValues, defaultGeometry, f, context ) )
  {
    connect( layer, &QgsVectorLayer::willBeDeleted, this, &QgsTrackedVectorLayerTools::layerWillBeDeleted, Qt::UniqueConnection );
    mAddedFeatures[layer->id()].insert( f->id() );
    if ( !feature )
      delete f;
    return true;
  }
  else
  {
    if ( !feature )
      delete f;
    return false;
  }
}

bool QgsTrackedVectorLayerTools::startEditing( QgsVectorLayer *layer ) const
{
  return mBackend->startEditing( layer );
}

bool QgsTrackedVectorLayerTools::stopEditing( QgsVectorLayer *layer, bool allowCancel ) const
{
  return mBackend->stopEditing( layer, allowCancel );
}

bool QgsTrackedVectorLayerTools::saveEdits( QgsVectorLayer *layer ) const
{
  return mBackend->saveEdits( layer );
}

bool QgsTrackedVectorLayerTools::copyMoveFeatures(
  QgsVectorLayer *layer, QgsFeatureRequest &request, double dx, double dy, QString *errorMsg, const bool topologicalEditing, QgsVectorLayer *topologicalLayer, QString *childrenInfoMsg
) const
{
  return mBackend->copyMoveFeatures( layer, request, dx, dy, errorMsg, topologicalEditing, topologicalLayer, childrenInfoMsg );
}

void QgsTrackedVectorLayerTools::setVectorLayerTools( const QgsVectorLayerTools *tools )
{
  mBackend = tools;
}

void QgsTrackedVectorLayerTools::rollback()
{
  QgsProject *p = project();
  if ( !p )
    return;

  for ( auto it = mAddedFeatures.constBegin(); it != mAddedFeatures.constEnd(); ++it )
  {
    if ( QgsVectorLayer *layer = qobject_cast< QgsVectorLayer * >( p->mapLayer( it.key() ) ) )
    {
      layer->deleteFeatures( it.value() );
    }
  }
  mAddedFeatures.clear();
}

void QgsTrackedVectorLayerTools::layerWillBeDeleted()
{
  if ( QgsVectorLayer *layer = qobject_cast< QgsVectorLayer * >( sender() ) )
  {
    // make sure there's no dangling references left to a layer which will be deleted
    mAddedFeatures.remove( layer->id() );
  }
}
