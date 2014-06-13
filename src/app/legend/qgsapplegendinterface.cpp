/***************************************************************************
    qgsapplegendinterface.cpp
     --------------------------------------
    Date                 : 19-Nov-2009
    Copyright            : (C) 2009 by Andres Manz
    Email                : manz dot andres at gmail dot com
****************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplegendinterface.h"

#include "qgsapplayertreeviewmenuprovider.h"
#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgslayertreeview.h"
#include "qgsmaplayer.h"
#include "qgsproject.h"
#include "qgslayertreeregistrybridge.h"


QgsAppLegendInterface::QgsAppLegendInterface( QgsLayerTreeView * layerTreeView )
    : mLayerTreeView( layerTreeView )
{
  connect( layerTreeView->layerTreeModel()->rootGroup(), SIGNAL( addedChildren( QgsLayerTreeNode*, int, int ) ), this, SLOT( onAddedChildren( QgsLayerTreeNode*, int, int ) ) );
  connect( layerTreeView->layerTreeModel()->rootGroup(), SIGNAL( removedChildren( QgsLayerTreeNode*, int, int ) ), this, SLOT( onRemovedChildren() ) );
  connect( layerTreeView, SIGNAL( currentLayerChanged( QgsMapLayer * ) ), this, SIGNAL( currentLayerChanged( QgsMapLayer * ) ) );
}

QgsAppLegendInterface::~QgsAppLegendInterface()
{
}

int QgsAppLegendInterface::addGroup( QString name, bool expand, QTreeWidgetItem* parent )
{
  if ( parent )
    return -1;

  return addGroup( name, expand, -1 );
}

void QgsAppLegendInterface::setExpanded( QgsLayerTreeNode *node, bool expand )
{
  QModelIndex idx = mLayerTreeView->layerTreeModel()->node2index( node );
  if ( expand )
    mLayerTreeView->expand( idx );
  else
    mLayerTreeView->collapse( idx );
}

int QgsAppLegendInterface::addGroup( QString name, bool expand, int parentIndex )
{
  QgsLayerTreeGroup* parentGroup = parentIndex == -1 ? mLayerTreeView->layerTreeModel()->rootGroup() : groupIndexToNode( parentIndex );
  if ( !parentGroup )
    return -1;

  QgsLayerTreeGroup* group = parentGroup->addGroup( name );
  setExpanded( group, expand );
  return groupNodeToIndex( group );
}

void QgsAppLegendInterface::removeGroup( int groupIndex )
{
  QgsLayerTreeGroup* group = groupIndexToNode( groupIndex );
  if ( !group || !QgsLayerTree::isGroup( group->parent() ) )
    return;

  QgsLayerTreeGroup* parentGroup = QgsLayerTree::toGroup( group->parent() );
  parentGroup->removeChildNode( group );
}

void QgsAppLegendInterface::moveLayer( QgsMapLayer * ml, int groupIndex )
{
  QgsLayerTreeGroup* group = groupIndexToNode( groupIndex );
  if ( !group )
    return;

  QgsLayerTreeLayer* nodeLayer = mLayerTreeView->layerTreeModel()->rootGroup()->findLayer( ml->id() );
  if ( !nodeLayer || !QgsLayerTree::isGroup( nodeLayer->parent() ) )
    return;

  group->insertLayer( 0, ml );

  QgsLayerTreeGroup* nodeLayerParentGroup = QgsLayerTree::toGroup( nodeLayer->parent() );
  nodeLayerParentGroup->removeChildNode( nodeLayer );
}

void QgsAppLegendInterface::setGroupExpanded( int groupIndex, bool expand )
{
  if ( QgsLayerTreeGroup* group = groupIndexToNode( groupIndex ) )
    setExpanded( group, expand );
}

void QgsAppLegendInterface::setGroupVisible( int groupIndex, bool visible )
{
  if ( QgsLayerTreeGroup* group = groupIndexToNode( groupIndex ) )
    group->setVisible( visible ? Qt::Checked : Qt::Unchecked );
}


static QgsLayerTreeGroup* _groupIndexToNode( int groupIndex, QgsLayerTreeGroup* parentGroup, int& currentIndex )
{
  ++currentIndex;
  foreach ( QgsLayerTreeNode* child, parentGroup->children() )
  {
    if ( QgsLayerTree::isGroup( child ) )
    {
      if ( currentIndex == groupIndex )
        return QgsLayerTree::toGroup( child );

      if ( QgsLayerTreeGroup* res = _groupIndexToNode( groupIndex, QgsLayerTree::toGroup( child ), currentIndex ) )
        return res;
    }
  }

  return 0;
}

QgsLayerTreeGroup* QgsAppLegendInterface::groupIndexToNode( int itemIndex )
{
  int currentIndex = -1;
  return _groupIndexToNode( itemIndex, mLayerTreeView->layerTreeModel()->rootGroup(), currentIndex );
}


static int _groupNodeToIndex( QgsLayerTreeGroup* group, QgsLayerTreeGroup* parentGroup, int& currentIndex )
{
  ++currentIndex;
  foreach ( QgsLayerTreeNode* child, parentGroup->children() )
  {
    if ( QgsLayerTree::isGroup( child ) )
    {
      QgsLayerTreeGroup* childGroup = QgsLayerTree::toGroup( child );
      if ( childGroup == group )
        return currentIndex;

      int res = _groupNodeToIndex( group, childGroup, currentIndex );
      if ( res != -1 )
        return res;
    }
  }

  return -1;
}

int QgsAppLegendInterface::groupNodeToIndex( QgsLayerTreeGroup* group )
{
  int currentIndex = -1;
  return _groupNodeToIndex( group, mLayerTreeView->layerTreeModel()->rootGroup(), currentIndex );
}

void QgsAppLegendInterface::setLayerVisible( QgsMapLayer * ml, bool visible )
{
  if ( QgsLayerTreeLayer* nodeLayer = mLayerTreeView->layerTreeModel()->rootGroup()->findLayer( ml->id() ) )
    nodeLayer->setVisible( visible ? Qt::Checked : Qt::Unchecked );
}

void QgsAppLegendInterface::setLayerExpanded( QgsMapLayer * ml, bool expand )
{
  if ( QgsLayerTreeLayer* nodeLayer = mLayerTreeView->layerTreeModel()->rootGroup()->findLayer( ml->id() ) )
    setExpanded( nodeLayer, expand );
}

static void _collectGroups( QgsLayerTreeGroup* parentGroup, QStringList& list )
{
  foreach ( QgsLayerTreeNode* child, parentGroup->children() )
  {
    if ( QgsLayerTree::isGroup( child ) )
    {
      QgsLayerTreeGroup* childGroup = QgsLayerTree::toGroup( child );
      list << childGroup->name();

      _collectGroups( childGroup, list );
    }
  }
}

QStringList QgsAppLegendInterface::groups()
{
  QStringList list;
  _collectGroups( mLayerTreeView->layerTreeModel()->rootGroup(), list );
  return list;
}

QList< GroupLayerInfo > QgsAppLegendInterface::groupLayerRelationship()
{
  QList< GroupLayerInfo > groupLayerList;
  QList< QgsLayerTreeNode* > nodes = mLayerTreeView->layerTreeModel()->rootGroup()->children();

  while ( !nodes.isEmpty() )
  {
    QgsLayerTreeNode* currentNode = nodes.takeFirst();

    if ( QgsLayerTree::isLayer( currentNode ) )
    {
      QList<QString> layerList;
      layerList.push_back( QgsLayerTree::toLayer( currentNode )->layerId() );
      groupLayerList.push_back( qMakePair( QString(), layerList ) );
    }
    else if ( QgsLayerTree::isGroup( currentNode ) )
    {
      QList<QString> layerList;
      foreach ( QgsLayerTreeNode* gNode, QgsLayerTree::toGroup( currentNode )->children() )
      {
        if ( QgsLayerTree::isLayer( gNode ) )
        {
          layerList.push_back( QgsLayerTree::toLayer( gNode )->layerId() );
        }
        else if ( QgsLayerTree::isGroup( gNode ) )
        {
          layerList << QgsLayerTree::toGroup( gNode )->name();
          nodes << gNode;
        }
      }

      groupLayerList.push_back( qMakePair( QgsLayerTree::toGroup( currentNode )->name(), layerList ) );
    }
  }

  return groupLayerList;
}

bool QgsAppLegendInterface::groupExists( int groupIndex )
{
  return groupIndexToNode( groupIndex ) != 0;
}

bool QgsAppLegendInterface::isGroupExpanded( int groupIndex )
{
  if ( QgsLayerTreeGroup* group = groupIndexToNode( groupIndex ) )
    return group->isExpanded();

  return false;
}

bool QgsAppLegendInterface::isGroupVisible( int groupIndex )
{
  if ( QgsLayerTreeGroup* group = groupIndexToNode( groupIndex ) )
    return group->isVisible() == Qt::Checked;

  return false;
}

bool QgsAppLegendInterface::isLayerExpanded( QgsMapLayer * ml )
{
  if ( QgsLayerTreeLayer* nodeLayer = mLayerTreeView->layerTreeModel()->rootGroup()->findLayer( ml->id() ) )
    return nodeLayer->isExpanded();

  return false;
}


bool QgsAppLegendInterface::isLayerVisible( QgsMapLayer * ml )
{
  if ( QgsLayerTreeLayer* nodeLayer = mLayerTreeView->layerTreeModel()->rootGroup()->findLayer( ml->id() ) )
    return nodeLayer->isVisible() == Qt::Checked;

  return false;
}

QList<QgsMapLayer *> QgsAppLegendInterface::selectedLayers( bool inDrawOrder ) const
{
  Q_UNUSED( inDrawOrder ); // TODO[MD]
  return mLayerTreeView->selectedLayers();
}

QList< QgsMapLayer * > QgsAppLegendInterface::layers() const
{
  QList<QgsMapLayer*> lst;
  foreach ( QgsLayerTreeLayer* node, mLayerTreeView->layerTreeModel()->rootGroup()->findLayers() )
  {
    if ( node->layer() )
      lst << node->layer();
  }
  return lst;
}

void QgsAppLegendInterface::refreshLayerSymbology( QgsMapLayer *ml )
{
  mLayerTreeView->refreshLayerSymbology( ml->id() );
}

void QgsAppLegendInterface::onAddedChildren( QgsLayerTreeNode* node, int indexFrom, int indexTo )
{
  emit groupRelationsChanged();

  for ( int i = indexFrom; i <= indexTo; ++i )
  {
    QgsLayerTreeNode* child = node->children().at( i );
    emit itemAdded( mLayerTreeView->layerTreeModel()->node2index( child ) );

    // also notify about all children
    if ( QgsLayerTree::isGroup( child ) && child->children().count() )
      onAddedChildren( child, 0, child->children().count() - 1 );
  }
}

void QgsAppLegendInterface::onRemovedChildren()
{
  emit groupRelationsChanged();

  emit itemRemoved();
}

void QgsAppLegendInterface::addLegendLayerAction( QAction* action,
    QString menu, QString id, QgsMapLayer::LayerType type, bool allLayers )
{
  QgsAppLayerTreeViewMenuProvider* menuProvider = dynamic_cast<QgsAppLayerTreeViewMenuProvider*>( mLayerTreeView->menuProvider() );
  if ( !menuProvider )
    return;

  menuProvider->addLegendLayerAction( action, menu, id, type, allLayers );
}

void QgsAppLegendInterface::addLegendLayerActionForLayer( QAction* action, QgsMapLayer* layer )
{
  QgsAppLayerTreeViewMenuProvider* menuProvider = dynamic_cast<QgsAppLayerTreeViewMenuProvider*>( mLayerTreeView->menuProvider() );
  if ( !menuProvider )
    return;

  menuProvider->addLegendLayerActionForLayer( action, layer );
}

bool QgsAppLegendInterface::removeLegendLayerAction( QAction* action )
{
  QgsAppLayerTreeViewMenuProvider* menuProvider = dynamic_cast<QgsAppLayerTreeViewMenuProvider*>( mLayerTreeView->menuProvider() );
  if ( !menuProvider )
    return false;

  return menuProvider->removeLegendLayerAction( action );
}

QgsMapLayer* QgsAppLegendInterface::currentLayer()
{
  return mLayerTreeView->currentLayer();
}

bool QgsAppLegendInterface::setCurrentLayer( QgsMapLayer *layer )
{
  if ( !mLayerTreeView->layerTreeModel()->rootGroup()->findLayer( layer->id() ) )
    return false;

  mLayerTreeView->setCurrentLayer( layer );
  return true;
}
