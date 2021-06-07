/***************************************************************************
    qgsvectorlayerselectionmanager.cpp
     --------------------------------------
    Date                 : 6.6.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorlayerselectionmanager.h"

#include "qgsvectorlayer.h"

QgsVectorLayerSelectionManager::QgsVectorLayerSelectionManager( QgsVectorLayer *layer, QObject *parent )
  : QgsIFeatureSelectionManager( parent )
  , mLayer( layer )
{
  connect( mLayer, &QgsVectorLayer::selectionChanged, this, &QgsVectorLayerSelectionManager::onSelectionChanged );
}

int QgsVectorLayerSelectionManager::selectedFeatureCount()
{
  return mLayer->selectedFeatureCount();
}

void QgsVectorLayerSelectionManager::select( const QgsFeatureIds &ids )
{
  mLayer->select( ids );
}

void QgsVectorLayerSelectionManager::deselect( const QgsFeatureIds &ids )
{
  mLayer->deselect( ids );
}

void QgsVectorLayerSelectionManager::setSelectedFeatures( const QgsFeatureIds &ids )
{
  mLayer->selectByIds( ids );
}

const QgsFeatureIds &QgsVectorLayerSelectionManager::selectedFeatureIds() const
{
  return mLayer->selectedFeatureIds();
}

QgsVectorLayer *QgsVectorLayerSelectionManager::layer() const
{
  return mLayer;
}

void QgsVectorLayerSelectionManager::onSelectionChanged( const QgsFeatureIds &selected, const QgsFeatureIds &deselected, bool clearAndSelect )
{
  emit selectionChanged( selected, deselected, clearAndSelect );
}
