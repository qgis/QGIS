/***************************************************************************
  qgslayertreeview.cpp
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

#include "qgslayertreeview.h"

#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgslayertreemodellegendnode.h"
#include "qgslayertreeviewdefaultactions.h"
#include "qgsmaplayer.h"

#include <QMenu>
#include <QContextMenuEvent>

QgsLayerTreeView::QgsLayerTreeView( QWidget *parent )
    : QTreeView( parent )
    , mDefaultActions( 0 )
    , mMenuProvider( 0 )
{
  setHeaderHidden( true );

  setDragEnabled( true );
  setAcceptDrops( true );
  setDropIndicatorShown( true );
  setEditTriggers( EditKeyPressed );
  setExpandsOnDoubleClick( false ); // normally used for other actions

  setSelectionMode( ExtendedSelection );

  connect( this, SIGNAL( collapsed( QModelIndex ) ), this, SLOT( updateExpandedStateToNode( QModelIndex ) ) );
  connect( this, SIGNAL( expanded( QModelIndex ) ), this, SLOT( updateExpandedStateToNode( QModelIndex ) ) );
}

QgsLayerTreeView::~QgsLayerTreeView()
{
  delete mMenuProvider;
}

void QgsLayerTreeView::setModel( QAbstractItemModel* model )
{
  if ( !qobject_cast<QgsLayerTreeModel*>( model ) )
    return;

  connect( model, SIGNAL( rowsInserted( QModelIndex, int, int ) ), this, SLOT( modelRowsInserted( QModelIndex, int, int ) ) );
  connect( model, SIGNAL( rowsRemoved( QModelIndex, int, int ) ), this, SLOT( modelRowsRemoved() ) );

  QTreeView::setModel( model );

  connect( layerTreeModel()->rootGroup(), SIGNAL( expandedChanged( QgsLayerTreeNode*, bool ) ), this, SLOT( onExpandedChanged( QgsLayerTreeNode*, bool ) ) );

  connect( selectionModel(), SIGNAL( currentChanged( QModelIndex, QModelIndex ) ), this, SLOT( onCurrentChanged() ) );

  connect( layerTreeModel(), SIGNAL( modelReset() ), this, SLOT( onModelReset() ) );

  updateExpandedStateFromNode( layerTreeModel()->rootGroup() );
}

QgsLayerTreeModel *QgsLayerTreeView::layerTreeModel() const
{
  return qobject_cast<QgsLayerTreeModel*>( model() );
}

QgsLayerTreeViewDefaultActions* QgsLayerTreeView::defaultActions()
{
  if ( !mDefaultActions )
    mDefaultActions = new QgsLayerTreeViewDefaultActions( this );
  return mDefaultActions;
}

void QgsLayerTreeView::setMenuProvider( QgsLayerTreeViewMenuProvider* menuProvider )
{
  delete mMenuProvider;
  mMenuProvider = menuProvider;
}

QgsMapLayer* QgsLayerTreeView::currentLayer() const
{
  return layerForIndex( currentIndex() );
}

void QgsLayerTreeView::setCurrentLayer( QgsMapLayer* layer )
{
  if ( !layer )
  {
    setCurrentIndex( QModelIndex() );
    return;
  }

  QgsLayerTreeLayer* nodeLayer = layerTreeModel()->rootGroup()->findLayer( layer->id() );
  if ( !nodeLayer )
    return;

  setCurrentIndex( layerTreeModel()->node2index( nodeLayer ) );
}


void QgsLayerTreeView::contextMenuEvent( QContextMenuEvent *event )
{
  if ( !mMenuProvider )
    return;

  QModelIndex idx = indexAt( event->pos() );
  if ( !idx.isValid() )
    setCurrentIndex( QModelIndex() );

  QMenu* menu = mMenuProvider->createContextMenu();
  if ( menu && menu->actions().count() != 0 )
    menu->exec( mapToGlobal( event->pos() ) );
  delete menu;
}


void QgsLayerTreeView::modelRowsInserted( QModelIndex index, int start, int end )
{
  QgsLayerTreeNode* parentNode = layerTreeModel()->index2node( index );
  if ( !parentNode )
    return;

  if ( QgsLayerTree::isLayer( parentNode ) )
  {
    // if ShowLegendAsTree flag is enabled in model, we may need to expand some legend nodes
    QStringList expandedNodeKeys = parentNode->customProperty( "expandedLegendNodes" ).toStringList();
    if ( expandedNodeKeys.isEmpty() )
      return;

    Q_FOREACH ( QgsLayerTreeModelLegendNode* legendNode, layerTreeModel()->layerLegendNodes( QgsLayerTree::toLayer( parentNode ) ) )
    {
      QString ruleKey = legendNode->data( QgsLayerTreeModelLegendNode::RuleKeyRole ).toString();
      if ( expandedNodeKeys.contains( ruleKey ) )
        setExpanded( layerTreeModel()->legendNode2index( legendNode ), true );
    }
    return;
  }

  QList<QgsLayerTreeNode*> children = parentNode->children();
  for ( int i = start; i <= end; ++i )
  {
    updateExpandedStateFromNode( children[i] );
  }

  // make sure we still have correct current layer
  onCurrentChanged();
}

void QgsLayerTreeView::modelRowsRemoved()
{
  // make sure we still have correct current layer
  onCurrentChanged();
}

void QgsLayerTreeView::updateExpandedStateToNode( QModelIndex index )
{
  if ( QgsLayerTreeNode* node = layerTreeModel()->index2node( index ) )
  {
    node->setExpanded( isExpanded( index ) );
  }
  else if ( QgsLayerTreeModelLegendNode* node = layerTreeModel()->index2legendNode( index ) )
  {
    QString ruleKey = node->data( QgsLayerTreeModelLegendNode::RuleKeyRole ).toString();
    QStringList lst = node->layerNode()->customProperty( "expandedLegendNodes" ).toStringList();
    bool expanded = isExpanded( index );
    bool isInList = lst.contains( ruleKey );
    if ( expanded && !isInList )
    {
      lst.append( ruleKey );
      node->layerNode()->setCustomProperty( "expandedLegendNodes", lst );
    }
    else if ( !expanded && isInList )
    {
      lst.removeAll( ruleKey );
      node->layerNode()->setCustomProperty( "expandedLegendNodes", lst );
    }
  }
}

void QgsLayerTreeView::onCurrentChanged()
{
  QgsMapLayer* layerCurrent = layerForIndex( currentIndex() );
  QString layerCurrentID = layerCurrent ? layerCurrent->id() : QString();
  if ( mCurrentLayerID == layerCurrentID )
    return;

  // update the current index in model (the item will be underlined)
  QModelIndex nodeLayerIndex;
  if ( layerCurrent )
  {
    QgsLayerTreeLayer* nodeLayer = layerTreeModel()->rootGroup()->findLayer( layerCurrentID );
    if ( nodeLayer )
      nodeLayerIndex = layerTreeModel()->node2index( nodeLayer );
  }
  layerTreeModel()->setCurrentIndex( nodeLayerIndex );

  mCurrentLayerID = layerCurrentID;
  emit currentLayerChanged( layerCurrent );
}

void QgsLayerTreeView::onExpandedChanged( QgsLayerTreeNode* node, bool expanded )
{
  QModelIndex idx = layerTreeModel()->node2index( node );
  if ( isExpanded( idx ) != expanded )
    setExpanded( idx, expanded );
}

void QgsLayerTreeView::onModelReset()
{
  updateExpandedStateFromNode( layerTreeModel()->rootGroup() );
}

void QgsLayerTreeView::updateExpandedStateFromNode( QgsLayerTreeNode* node )
{
  QModelIndex idx = layerTreeModel()->node2index( node );
  setExpanded( idx, node->isExpanded() );

  Q_FOREACH ( QgsLayerTreeNode* child, node->children() )
    updateExpandedStateFromNode( child );
}

QgsMapLayer* QgsLayerTreeView::layerForIndex( const QModelIndex& index ) const
{
  QgsLayerTreeNode* node = layerTreeModel()->index2node( index );
  if ( node )
  {
    if ( QgsLayerTree::isLayer( node ) )
      return QgsLayerTree::toLayer( node )->layer();
  }
  else
  {
    // possibly a legend node
    QgsLayerTreeModelLegendNode* legendNode = layerTreeModel()->index2legendNode( index );
    if ( legendNode )
      return legendNode->layerNode()->layer();
  }

  return 0;
}

QgsLayerTreeNode* QgsLayerTreeView::currentNode() const
{
  return layerTreeModel()->index2node( selectionModel()->currentIndex() );
}

QgsLayerTreeGroup* QgsLayerTreeView::currentGroupNode() const
{
  QgsLayerTreeNode* node = currentNode();
  if ( QgsLayerTree::isGroup( node ) )
    return QgsLayerTree::toGroup( node );
  else if ( QgsLayerTree::isLayer( node ) )
  {
    QgsLayerTreeNode* parent = node->parent();
    if ( QgsLayerTree::isGroup( parent ) )
      return QgsLayerTree::toGroup( parent );
  }

  if ( QgsLayerTreeModelLegendNode* legendNode = layerTreeModel()->index2legendNode( selectionModel()->currentIndex() ) )
  {
    QgsLayerTreeLayer* parent = legendNode->layerNode();
    if ( QgsLayerTree::isGroup( parent->parent() ) )
      return QgsLayerTree::toGroup( parent->parent() );
  }

  return 0;
}

QList<QgsLayerTreeNode*> QgsLayerTreeView::selectedNodes( bool skipInternal ) const
{
  return layerTreeModel()->indexes2nodes( selectionModel()->selectedIndexes(), skipInternal );
}

QList<QgsLayerTreeLayer*> QgsLayerTreeView::selectedLayerNodes() const
{
  QList<QgsLayerTreeLayer*> layerNodes;
  Q_FOREACH ( QgsLayerTreeNode* node, selectedNodes() )
  {
    if ( QgsLayerTree::isLayer( node ) )
      layerNodes << QgsLayerTree::toLayer( node );
  }
  return layerNodes;
}

QList<QgsMapLayer*> QgsLayerTreeView::selectedLayers() const
{
  QList<QgsMapLayer*> list;
  Q_FOREACH ( QgsLayerTreeLayer* node, selectedLayerNodes() )
  {
    if ( node->layer() )
      list << node->layer();
  }
  return list;
}


void QgsLayerTreeView::refreshLayerSymbology( const QString& layerId )
{
  QgsLayerTreeLayer* nodeLayer = layerTreeModel()->rootGroup()->findLayer( layerId );
  if ( nodeLayer )
    layerTreeModel()->refreshLayerLegend( nodeLayer );
}
