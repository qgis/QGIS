/***************************************************************************
  qgslayertreeregistrybridge.cpp
  --------------------------------------
  Date                 : May 2014
  Copyright            : (C) 2014 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayertreeregistrybridge.h"

#include "qgsmaplayerregistry.h"

#include "qgslayertree.h"

#include "qgsproject.h"
#include "qgslogger.h"

QgsLayerTreeRegistryBridge::QgsLayerTreeRegistryBridge( QgsLayerTreeGroup *root, QObject *parent )
    : QObject( parent )
    , mRoot( root )
    , mRegistryRemovingLayers( false )
    , mEnabled( true )
    , mNewLayersVisible( true )
    , mInsertionPointGroup( root )
    , mInsertionPointIndex( 0 )
{
  connect( QgsMapLayerRegistry::instance(), SIGNAL( legendLayersAdded( QList<QgsMapLayer*> ) ), this, SLOT( layersAdded( QList<QgsMapLayer*> ) ) );
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layersWillBeRemoved( QStringList ) ), this, SLOT( layersWillBeRemoved( QStringList ) ) );

  connect( mRoot, SIGNAL( willRemoveChildren( QgsLayerTreeNode*, int, int ) ), this, SLOT( groupWillRemoveChildren( QgsLayerTreeNode*, int, int ) ) );
  connect( mRoot, SIGNAL( removedChildren( QgsLayerTreeNode*, int, int ) ), this, SLOT( groupRemovedChildren() ) );
}

void QgsLayerTreeRegistryBridge::setLayerInsertionPoint( QgsLayerTreeGroup* parentGroup, int index )
{
  mInsertionPointGroup = parentGroup;
  mInsertionPointIndex = index;
}

void QgsLayerTreeRegistryBridge::layersAdded( QList<QgsMapLayer*> layers )
{
  if ( !mEnabled )
    return;

  QList<QgsLayerTreeNode*> nodes;
  foreach ( QgsMapLayer* layer, layers )
  {
    QgsLayerTreeLayer* nodeLayer = new QgsLayerTreeLayer( layer );
    nodeLayer->setVisible( mNewLayersVisible ? Qt::Checked : Qt::Unchecked );

    nodes << nodeLayer;

    // check whether the layer is marked as embedded
    QString projectFile = QgsProject::instance()->layerIsEmbedded( nodeLayer->layerId() );
    if ( !projectFile.isEmpty() )
    {
      nodeLayer->setCustomProperty( "embedded", 1 );
      nodeLayer->setCustomProperty( "embedded_project", projectFile );
    }
  }

  // add new layers to the right place
  mInsertionPointGroup->insertChildNodes( mInsertionPointIndex, nodes );

  // tell other components that layers have been added - this signal is used in QGIS to auto-select the first layer
  emit addedLayersToLayerTree( layers );
}

void QgsLayerTreeRegistryBridge::layersWillBeRemoved( QStringList layerIds )
{
  QgsDebugMsg( QString( "%1 layers will be removed, enabled:%2" ).arg( layerIds.count() ).arg( mEnabled ) );

  if ( !mEnabled )
    return;

  // when we start removing child nodes, the bridge would try to remove those layers from
  // the registry _again_ in groupRemovedChildren() - this prevents it
  mRegistryRemovingLayers = true;

  foreach ( QString layerId, layerIds )
  {
    QgsLayerTreeLayer* nodeLayer = mRoot->findLayer( layerId );
    if ( nodeLayer )
      qobject_cast<QgsLayerTreeGroup*>( nodeLayer->parent() )->removeChildNode( nodeLayer );
  }

  mRegistryRemovingLayers = false;
}


static void _collectLayerIdsInGroup( QgsLayerTreeGroup* group, int indexFrom, int indexTo, QStringList& lst )
{
  for ( int i = indexFrom; i <= indexTo; ++i )
  {
    QgsLayerTreeNode* child = group->children()[i];
    if ( QgsLayerTree::isLayer( child ) )
    {
      lst << QgsLayerTree::toLayer( child )->layerId();
    }
    else if ( QgsLayerTree::isGroup( child ) )
    {
      _collectLayerIdsInGroup( QgsLayerTree::toGroup( child ), 0, child->children().count() - 1, lst );
    }
  }
}

void QgsLayerTreeRegistryBridge::groupWillRemoveChildren( QgsLayerTreeNode* node, int indexFrom, int indexTo )
{
  if ( mRegistryRemovingLayers )
    return; // do not try to remove those layers again

  Q_ASSERT( mLayerIdsForRemoval.isEmpty() );

  Q_ASSERT( QgsLayerTree::isGroup( node ) );
  QgsLayerTreeGroup* group = QgsLayerTree::toGroup( node );

  _collectLayerIdsInGroup( group, indexFrom, indexTo, mLayerIdsForRemoval );
}

void QgsLayerTreeRegistryBridge::groupRemovedChildren()
{
  if ( mRegistryRemovingLayers )
    return; // do not try to remove those layers again

  // remove only those that really do not exist in the tree
  // (ignores layers that were dragged'n'dropped: 1. drop new 2. remove old)
  QStringList toRemove;
  foreach ( QString layerId, mLayerIdsForRemoval )
    if ( !mRoot->findLayer( layerId ) )
      toRemove << layerId;
  mLayerIdsForRemoval.clear();

  QgsDebugMsg( QString( "%1 layers will be removed" ).arg( toRemove.count() ) );

  // delay the removal of layers from the registry. There may be other slots connected to map layer registry's signals
  // that might disrupt the execution flow - e.g. a processEvents() call may force update of layer tree view with
  // semi-broken tree model
  QMetaObject::invokeMethod( this, "removeLayersFromRegistry", Qt::QueuedConnection, Q_ARG( QStringList, toRemove ) );
}

void QgsLayerTreeRegistryBridge::removeLayersFromRegistry( QStringList layerIds )
{
  QgsMapLayerRegistry::instance()->removeMapLayers( layerIds );
}
