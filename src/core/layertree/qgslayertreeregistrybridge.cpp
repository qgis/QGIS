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

#include "qgslayertree.h"
#include "qgslayertreeutils.h"
#include "qgsproject.h"
#include "qgslogger.h"

QgsLayerTreeRegistryBridge::QgsLayerTreeRegistryBridge( QgsLayerTreeGroup *root, QgsProject *project, QObject *parent )
  : QObject( parent )
  , mRoot( root )
  , mProject( project )
  , mRegistryRemovingLayers( false )
  , mEnabled( true )
  , mNewLayersVisible( true )
  , mInsertionPoint( root, 0 )
{
  connect( mProject, &QgsProject::legendLayersAdded, this, &QgsLayerTreeRegistryBridge::layersAdded );
  connect( mProject, qOverload<const QStringList &>( &QgsProject::layersWillBeRemoved ), this, &QgsLayerTreeRegistryBridge::layersWillBeRemoved );

  connect( mRoot, &QgsLayerTreeNode::willRemoveChildren, this, &QgsLayerTreeRegistryBridge::groupWillRemoveChildren );
  connect( mRoot, &QgsLayerTreeNode::removedChildren, this, &QgsLayerTreeRegistryBridge::groupRemovedChildren );
}

void QgsLayerTreeRegistryBridge::setLayerInsertionPoint( QgsLayerTreeGroup *parentGroup, int index )
{
  mInsertionPoint.group = parentGroup;
  mInsertionPoint.position = index;
}

void QgsLayerTreeRegistryBridge::setLayerInsertionPoint( const InsertionPoint &insertionPoint )
{
  mInsertionPoint = insertionPoint;
}

void QgsLayerTreeRegistryBridge::layersAdded( const QList<QgsMapLayer *> &layers )
{
  if ( !mEnabled )
    return;

  QList<QgsLayerTreeNode *> nodes;
  for ( QgsMapLayer *layer : layers )
  {
    QgsLayerTreeLayer *nodeLayer;
    if ( mInsertionMethod == Qgis::LayerTreeInsertionMethod::OptimalInInsertionGroup )
    {
      nodeLayer = QgsLayerTreeUtils::insertLayerAtOptimalPlacement( mInsertionPoint.group, layer );
    }
    else
    {
      nodeLayer = new QgsLayerTreeLayer( layer );
    }

    nodeLayer->setItemVisibilityChecked( mNewLayersVisible );

    nodes << nodeLayer;

    // check whether the layer is marked as embedded
    const QString projectFile = mProject->layerIsEmbedded( nodeLayer->layerId() );
    if ( !projectFile.isEmpty() )
    {
      nodeLayer->setCustomProperty( QStringLiteral( "embedded" ), 1 );
      nodeLayer->setCustomProperty( QStringLiteral( "embedded_project" ), projectFile );
    }
  }

  switch ( mInsertionMethod )
  {
    case Qgis::LayerTreeInsertionMethod::AboveInsertionPoint:
      mInsertionPoint.group->insertChildNodes( mInsertionPoint.position, nodes );
      break;
    case Qgis::LayerTreeInsertionMethod::TopOfTree:
      mRoot->insertChildNodes( 0, nodes );
      break;
    case Qgis::LayerTreeInsertionMethod::OptimalInInsertionGroup:
    default:
      break;
  }

  // tell other components that layers have been added - this signal is used in QGIS to auto-select the first layer
  emit addedLayersToLayerTree( layers );
}

void QgsLayerTreeRegistryBridge::layersWillBeRemoved( const QStringList &layerIds )
{
  QgsDebugMsgLevel( QStringLiteral( "%1 layers will be removed, enabled:%2" ).arg( layerIds.count() ).arg( mEnabled ), 4 );

  if ( !mEnabled )
    return;

  // when we start removing child nodes, the bridge would try to remove those layers from
  // the registry _again_ in groupRemovedChildren() - this prevents it
  mRegistryRemovingLayers = true;

  const auto constLayerIds = layerIds;
  for ( const QString &layerId : constLayerIds )
  {
    QgsLayerTreeLayer *nodeLayer = mRoot->findLayer( layerId );
    if ( nodeLayer )
      qobject_cast<QgsLayerTreeGroup *>( nodeLayer->parent() )->removeChildNode( nodeLayer );
  }

  mRegistryRemovingLayers = false;
}


static void _collectLayerIdsInGroup( QgsLayerTreeGroup *group, int indexFrom, int indexTo, QStringList &lst )
{
  for ( int i = indexFrom; i <= indexTo; ++i )
  {
    QgsLayerTreeNode *child = group->children().at( i );
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

void QgsLayerTreeRegistryBridge::groupWillRemoveChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo )
{
  if ( mRegistryRemovingLayers )
    return; // do not try to remove those layers again

  Q_ASSERT( mLayerIdsForRemoval.isEmpty() );

  Q_ASSERT( QgsLayerTree::isGroup( node ) );
  QgsLayerTreeGroup *group = QgsLayerTree::toGroup( node );

  _collectLayerIdsInGroup( group, indexFrom, indexTo, mLayerIdsForRemoval );
}

void QgsLayerTreeRegistryBridge::groupRemovedChildren()
{
  if ( mRegistryRemovingLayers )
    return; // do not try to remove those layers again

  // remove only those that really do not exist in the tree
  // (ignores layers that were dragged'n'dropped: 1. drop new 2. remove old)
  QStringList toRemove;
  const auto constMLayerIdsForRemoval = mLayerIdsForRemoval;
  for ( const QString &layerId : constMLayerIdsForRemoval )
    if ( !mRoot->findLayer( layerId ) )
      toRemove << layerId;
  mLayerIdsForRemoval.clear();

  QgsDebugMsgLevel( QStringLiteral( "%1 layers will be removed" ).arg( toRemove.count() ), 4 );

  // delay the removal of layers from the registry. There may be other slots connected to map layer registry's signals
  // that might disrupt the execution flow - e.g. a processEvents() call may force update of layer tree view with
  // semi-broken tree model
  QMetaObject::invokeMethod( this, "removeLayersFromRegistry", Qt::QueuedConnection, Q_ARG( QStringList, toRemove ) );
}

void QgsLayerTreeRegistryBridge::removeLayersFromRegistry( const QStringList &layerIds )
{
  mProject->removeMapLayers( layerIds );
}
