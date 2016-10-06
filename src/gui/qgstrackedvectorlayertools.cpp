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

QgsTrackedVectorLayerTools::QgsTrackedVectorLayerTools()
    : mBackend( nullptr )
{
}

bool QgsTrackedVectorLayerTools::addFeature( QgsVectorLayer* layer, const QgsAttributeMap& defaultValues, const QgsGeometry& defaultGeometry, QgsFeature* feature ) const
{
  QgsFeature* f = feature;
  if ( !feature )
    f = new QgsFeature();

  if ( mBackend->addFeature( layer, defaultValues, defaultGeometry, f ) )
  {
    mAddedFeatures[layer].insert( f->id() );
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

bool QgsTrackedVectorLayerTools::startEditing( QgsVectorLayer* layer ) const
{
  return mBackend->startEditing( layer );
}

bool QgsTrackedVectorLayerTools::stopEditing( QgsVectorLayer* layer, bool allowCancel ) const
{
  return mBackend->stopEditing( layer, allowCancel );
}

bool QgsTrackedVectorLayerTools::saveEdits( QgsVectorLayer* layer ) const
{
  return mBackend->saveEdits( layer );
}

void QgsTrackedVectorLayerTools::setVectorLayerTools( const QgsVectorLayerTools* tools )
{
  mBackend = tools;
}

void QgsTrackedVectorLayerTools::rollback()
{
  QMapIterator<QgsVectorLayer*, QgsFeatureIds> it( mAddedFeatures );
  while ( it.hasNext() )
  {
    it.next();
    it.key()->deleteFeatures( it.value() );
  }

  mAddedFeatures.clear();
}
