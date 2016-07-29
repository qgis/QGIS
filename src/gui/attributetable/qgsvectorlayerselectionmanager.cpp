/***************************************************************************
    qgsvectorlayerselectionmanager.cpp
     --------------------------------------
    Date                 : 6.6.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorlayerselectionmanager.h"

#include "qgsvectorlayer.h"

QgsVectorLayerSelectionManager::QgsVectorLayerSelectionManager( QgsVectorLayer* layer, QObject* parent )
    : QgsIFeatureSelectionManager( parent )
    , mLayer( layer )
{
  connect( mLayer, SIGNAL( selectionChanged( QgsFeatureIds, QgsFeatureIds, bool ) ), this, SIGNAL( selectionChanged( QgsFeatureIds, QgsFeatureIds, bool ) ) );
}

int QgsVectorLayerSelectionManager::selectedFeatureCount()
{
  return mLayer->selectedFeatureCount();
}

void QgsVectorLayerSelectionManager::select( const QgsFeatureIds& ids )
{
  mLayer->select( ids );
}

void QgsVectorLayerSelectionManager::deselect( const QgsFeatureIds& ids )
{
  mLayer->deselect( ids );
}

void QgsVectorLayerSelectionManager::setSelectedFeatures( const QgsFeatureIds& ids )
{
  mLayer->selectByIds( ids );
}

const QgsFeatureIds& QgsVectorLayerSelectionManager::selectedFeaturesIds() const
{
  return mLayer->selectedFeaturesIds();
}
