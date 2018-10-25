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
#include "qgslayertreeembeddedwidgetregistry.h"
#include "qgslayertreemodel.h"
#include "qgslayertreemodellegendnode.h"
#include "qgslayertreeutils.h"
#include "qgslayertreeviewdefaultactions.h"
#include "qgsmaplayer.h"
#include "qgsgui.h"

#include <QMenu>
#include <QContextMenuEvent>

#include "qgslayertreeviewindicator.h"
#include "qgslayertreeviewitemdelegate.h"


QgsLayerTreeView::QgsLayerTreeView( QWidget *parent )
  : QTreeView( parent )

{
  setHeaderHidden( true );

  setDragEnabled( true );
  setAcceptDrops( true );
  setDropIndicatorShown( true );
  setEditTriggers( EditKeyPressed );
  setExpandsOnDoubleClick( false ); // normally used for other actions

  setSelectionMode( ExtendedSelection );
  setDefaultDropAction( Qt::MoveAction );

  // we need a custom item delegate in order to draw indicators
  setItemDelegate( new QgsLayerTreeViewItemDelegate( this ) );
  setStyle( new QgsLayerTreeViewProxyStyle( this ) );

  connect( this, &QTreeView::collapsed, this, &QgsLayerTreeView::updateExpandedStateToNode );
  connect( this, &QTreeView::expanded, this, &QgsLayerTreeView::updateExpandedStateToNode );
}

QgsLayerTreeView::~QgsLayerTreeView()
{
  delete mMenuProvider;
}

void QgsLayerTreeView::setModel( QAbstractItemModel *model )
{
  if ( !qobject_cast<QgsLayerTreeModel *>( model ) )
    return;

  connect( model, &QAbstractItemModel::rowsInserted, this, &QgsLayerTreeView::modelRowsInserted );
  connect( model, &QAbstractItemModel::rowsRemoved, this, &QgsLayerTreeView::modelRowsRemoved );

  QTreeView::setModel( model );

  connect( layerTreeModel()->rootGroup(), &QgsLayerTreeNode::expandedChanged, this, &QgsLayerTreeView::onExpandedChanged );
  connect( layerTreeModel()->rootGroup(), &QgsLayerTreeNode::customPropertyChanged, this, &QgsLayerTreeView::onCustomPropertyChanged );

  connect( selectionModel(), &QItemSelectionModel::currentChanged, this, &QgsLayerTreeView::onCurrentChanged );

  connect( layerTreeModel(), &QAbstractItemModel::modelReset, this, &QgsLayerTreeView::onModelReset );

  updateExpandedStateFromNode( layerTreeModel()->rootGroup() );
}

QgsLayerTreeModel *QgsLayerTreeView::layerTreeModel() const
{
  return qobject_cast<QgsLayerTreeModel *>( model() );
}

QgsLayerTreeViewDefaultActions *QgsLayerTreeView::defaultActions()
{
  if ( !mDefaultActions )
    mDefaultActions = new QgsLayerTreeViewDefaultActions( this );
  return mDefaultActions;
}

void QgsLayerTreeView::setMenuProvider( QgsLayerTreeViewMenuProvider *menuProvider )
{
  delete mMenuProvider;
  mMenuProvider = menuProvider;
}

QgsMapLayer *QgsLayerTreeView::currentLayer() const
{
  return layerForIndex( currentIndex() );
}

void QgsLayerTreeView::setCurrentLayer( QgsMapLayer *layer )
{
  if ( !layer )
  {
    setCurrentIndex( QModelIndex() );
    return;
  }

  QgsLayerTreeLayer *nodeLayer = layerTreeModel()->rootGroup()->findLayer( layer->id() );
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

  QMenu *menu = mMenuProvider->createContextMenu();
  if ( menu && menu->actions().count() != 0 )
    menu->exec( mapToGlobal( event->pos() ) );
  delete menu;
}


void QgsLayerTreeView::modelRowsInserted( const QModelIndex &index, int start, int end )
{
  QgsLayerTreeNode *parentNode = layerTreeModel()->index2node( index );
  if ( !parentNode )
    return;

  // Embedded widgets - replace placeholders in the model by actual widgets
  if ( layerTreeModel()->testFlag( QgsLayerTreeModel::UseEmbeddedWidgets ) && QgsLayerTree::isLayer( parentNode ) )
  {
    QgsLayerTreeLayer *nodeLayer = QgsLayerTree::toLayer( parentNode );
    if ( QgsMapLayer *layer = nodeLayer->layer() )
    {
      int widgetsCount = layer->customProperty( QStringLiteral( "embeddedWidgets/count" ), 0 ).toInt();
      QList<QgsLayerTreeModelLegendNode *> legendNodes = layerTreeModel()->layerLegendNodes( nodeLayer, true );
      for ( int i = 0; i < widgetsCount; ++i )
      {
        QString providerId = layer->customProperty( QStringLiteral( "embeddedWidgets/%1/id" ).arg( i ) ).toString();
        if ( QgsLayerTreeEmbeddedWidgetProvider *provider = QgsGui::layerTreeEmbeddedWidgetRegistry()->provider( providerId ) )
        {
          QModelIndex index = layerTreeModel()->legendNode2index( legendNodes[i] );
          setIndexWidget( index, provider->createWidget( layer, i ) );
        }
      }
    }
  }


  if ( QgsLayerTree::isLayer( parentNode ) )
  {
    // if ShowLegendAsTree flag is enabled in model, we may need to expand some legend nodes
    QStringList expandedNodeKeys = parentNode->customProperty( QStringLiteral( "expandedLegendNodes" ) ).toStringList();
    if ( expandedNodeKeys.isEmpty() )
      return;

    Q_FOREACH ( QgsLayerTreeModelLegendNode *legendNode, layerTreeModel()->layerLegendNodes( QgsLayerTree::toLayer( parentNode ), true ) )
    {
      QString ruleKey = legendNode->data( QgsLayerTreeModelLegendNode::RuleKeyRole ).toString();
      if ( expandedNodeKeys.contains( ruleKey ) )
        setExpanded( layerTreeModel()->legendNode2index( legendNode ), true );
    }
    return;
  }

  QList<QgsLayerTreeNode *> children = parentNode->children();
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

void QgsLayerTreeView::updateExpandedStateToNode( const QModelIndex &index )
{
  if ( QgsLayerTreeNode *node = layerTreeModel()->index2node( index ) )
  {
    node->setExpanded( isExpanded( index ) );
  }
  else if ( QgsLayerTreeModelLegendNode *node = layerTreeModel()->index2legendNode( index ) )
  {
    QString ruleKey = node->data( QgsLayerTreeModelLegendNode::RuleKeyRole ).toString();
    QStringList lst = node->layerNode()->customProperty( QStringLiteral( "expandedLegendNodes" ) ).toStringList();
    bool expanded = isExpanded( index );
    bool isInList = lst.contains( ruleKey );
    if ( expanded && !isInList )
    {
      lst.append( ruleKey );
      node->layerNode()->setCustomProperty( QStringLiteral( "expandedLegendNodes" ), lst );
    }
    else if ( !expanded && isInList )
    {
      lst.removeAll( ruleKey );
      node->layerNode()->setCustomProperty( QStringLiteral( "expandedLegendNodes" ), lst );
    }
  }
}

void QgsLayerTreeView::onCurrentChanged()
{
  QgsMapLayer *layerCurrent = layerForIndex( currentIndex() );
  QString layerCurrentID = layerCurrent ? layerCurrent->id() : QString();
  if ( mCurrentLayerID == layerCurrentID )
    return;

  // update the current index in model (the item will be underlined)
  QModelIndex nodeLayerIndex;
  if ( layerCurrent )
  {
    QgsLayerTreeLayer *nodeLayer = layerTreeModel()->rootGroup()->findLayer( layerCurrentID );
    if ( nodeLayer )
      nodeLayerIndex = layerTreeModel()->node2index( nodeLayer );
  }
  layerTreeModel()->setCurrentIndex( nodeLayerIndex );

  mCurrentLayerID = layerCurrentID;
  emit currentLayerChanged( layerCurrent );
}

void QgsLayerTreeView::onExpandedChanged( QgsLayerTreeNode *node, bool expanded )
{
  QModelIndex idx = layerTreeModel()->node2index( node );
  if ( isExpanded( idx ) != expanded )
    setExpanded( idx, expanded );
}

void QgsLayerTreeView::onCustomPropertyChanged( QgsLayerTreeNode *node, const QString &key )
{
  if ( key != QStringLiteral( "expandedLegendNodes" ) || !QgsLayerTree::isLayer( node ) )
    return;

  QSet<QString> expandedLegendNodes = node->customProperty( QStringLiteral( "expandedLegendNodes" ) ).toStringList().toSet();

  const QList<QgsLayerTreeModelLegendNode *> legendNodes = layerTreeModel()->layerLegendNodes( QgsLayerTree::toLayer( node ), true );
  for ( QgsLayerTreeModelLegendNode *legendNode : legendNodes )
  {
    QString key = legendNode->data( QgsLayerTreeModelLegendNode::RuleKeyRole ).toString();
    if ( !key.isEmpty() )
      setExpanded( layerTreeModel()->legendNode2index( legendNode ), expandedLegendNodes.contains( key ) );
  }
}

void QgsLayerTreeView::onModelReset()
{
  updateExpandedStateFromNode( layerTreeModel()->rootGroup() );
}

void QgsLayerTreeView::updateExpandedStateFromNode( QgsLayerTreeNode *node )
{
  QModelIndex idx = layerTreeModel()->node2index( node );
  setExpanded( idx, node->isExpanded() );

  Q_FOREACH ( QgsLayerTreeNode *child, node->children() )
    updateExpandedStateFromNode( child );
}

QgsMapLayer *QgsLayerTreeView::layerForIndex( const QModelIndex &index ) const
{
  // Check if model has been set and index is valid
  if ( layerTreeModel() && index.isValid() )
  {
    QgsLayerTreeNode *node = layerTreeModel()->index2node( index );
    if ( node )
    {
      if ( QgsLayerTree::isLayer( node ) )
        return QgsLayerTree::toLayer( node )->layer();
    }
    else
    {
      // possibly a legend node
      QgsLayerTreeModelLegendNode *legendNode = layerTreeModel()->index2legendNode( index );
      if ( legendNode )
        return legendNode->layerNode()->layer();
    }
  }
  return nullptr;
}

QgsLayerTreeNode *QgsLayerTreeView::currentNode() const
{
  return layerTreeModel()->index2node( selectionModel()->currentIndex() );
}

QgsLayerTreeGroup *QgsLayerTreeView::currentGroupNode() const
{
  QgsLayerTreeNode *node = currentNode();
  if ( QgsLayerTree::isGroup( node ) )
    return QgsLayerTree::toGroup( node );
  else if ( QgsLayerTree::isLayer( node ) )
  {
    QgsLayerTreeNode *parent = node->parent();
    if ( QgsLayerTree::isGroup( parent ) )
      return QgsLayerTree::toGroup( parent );
  }

  if ( QgsLayerTreeModelLegendNode *legendNode = layerTreeModel()->index2legendNode( selectionModel()->currentIndex() ) )
  {
    QgsLayerTreeLayer *parent = legendNode->layerNode();
    if ( QgsLayerTree::isGroup( parent->parent() ) )
      return QgsLayerTree::toGroup( parent->parent() );
  }

  return nullptr;
}

QgsLayerTreeModelLegendNode *QgsLayerTreeView::currentLegendNode() const
{
  return layerTreeModel()->index2legendNode( selectionModel()->currentIndex() );
}

QList<QgsLayerTreeNode *> QgsLayerTreeView::selectedNodes( bool skipInternal ) const
{
  return layerTreeModel()->indexes2nodes( selectionModel()->selectedIndexes(), skipInternal );
}

QList<QgsLayerTreeLayer *> QgsLayerTreeView::selectedLayerNodes() const
{
  QList<QgsLayerTreeLayer *> layerNodes;
  Q_FOREACH ( QgsLayerTreeNode *node, selectedNodes() )
  {
    if ( QgsLayerTree::isLayer( node ) )
      layerNodes << QgsLayerTree::toLayer( node );
  }
  return layerNodes;
}

QList<QgsMapLayer *> QgsLayerTreeView::selectedLayers() const
{
  QList<QgsMapLayer *> list;
  Q_FOREACH ( QgsLayerTreeLayer *node, selectedLayerNodes() )
  {
    if ( node->layer() )
      list << node->layer();
  }
  return list;
}

QList<QgsMapLayer *> QgsLayerTreeView::selectedLayersRecursive() const
{
  const QList<QgsLayerTreeNode *> nodes = layerTreeModel()->indexes2nodes( selectionModel()->selectedIndexes(), false );
  QSet<QgsMapLayer *> layersSet = QgsLayerTreeUtils::collectMapLayersRecursive( nodes );
  return layersSet.toList();
}

void QgsLayerTreeView::addIndicator( QgsLayerTreeNode *node, QgsLayerTreeViewIndicator *indicator )
{
  if ( !mIndicators[node].contains( indicator ) )
    mIndicators[node].append( indicator );
}

void QgsLayerTreeView::removeIndicator( QgsLayerTreeNode *node, QgsLayerTreeViewIndicator *indicator )
{
  mIndicators[node].removeOne( indicator );
}

QList<QgsLayerTreeViewIndicator *> QgsLayerTreeView::indicators( QgsLayerTreeNode *node ) const
{
  return mIndicators.value( node );
}

///@cond PRIVATE
QStringList QgsLayerTreeView::viewOnlyCustomProperties()
{
  return QStringList() << QStringLiteral( "expandedLegendNodes" );
}
///@endcond

void QgsLayerTreeView::refreshLayerSymbology( const QString &layerId )
{
  QgsLayerTreeLayer *nodeLayer = layerTreeModel()->rootGroup()->findLayer( layerId );
  if ( nodeLayer )
    layerTreeModel()->refreshLayerLegend( nodeLayer );
}


static void _expandAllLegendNodes( QgsLayerTreeLayer *nodeLayer, bool expanded, QgsLayerTreeModel *model )
{
  // for layers we also need to find out with legend nodes contain some children and make them expanded/collapsed
  // if we are collapsing, we just write out an empty list
  QStringList lst;
  if ( expanded )
  {
    Q_FOREACH ( QgsLayerTreeModelLegendNode *legendNode, model->layerLegendNodes( nodeLayer, true ) )
    {
      QString parentKey = legendNode->data( QgsLayerTreeModelLegendNode::ParentRuleKeyRole ).toString();
      if ( !parentKey.isEmpty() && !lst.contains( parentKey ) )
        lst << parentKey;
    }
  }
  nodeLayer->setCustomProperty( QStringLiteral( "expandedLegendNodes" ), lst );
}


static void _expandAllNodes( QgsLayerTreeGroup *parent, bool expanded, QgsLayerTreeModel *model )
{
  Q_FOREACH ( QgsLayerTreeNode *node, parent->children() )
  {
    node->setExpanded( expanded );
    if ( QgsLayerTree::isGroup( node ) )
      _expandAllNodes( QgsLayerTree::toGroup( node ), expanded, model );
    else if ( QgsLayerTree::isLayer( node ) )
      _expandAllLegendNodes( QgsLayerTree::toLayer( node ), expanded, model );
  }
}


void QgsLayerTreeView::expandAllNodes()
{
  // unfortunately expandAll() does not emit expanded() signals
  _expandAllNodes( layerTreeModel()->rootGroup(), true, layerTreeModel() );
  expandAll();
}

void QgsLayerTreeView::collapseAllNodes()
{
  // unfortunately collapseAll() does not emit collapsed() signals
  _expandAllNodes( layerTreeModel()->rootGroup(), false, layerTreeModel() );
  collapseAll();
}

void QgsLayerTreeView::mouseReleaseEvent( QMouseEvent *event )
{
  // we need to keep last mouse position in order to know whether to emit an indicator's clicked() signal
  // (the item delegate needs to know which indicator has been clicked)
  mLastReleaseMousePos = event->pos();

  const QgsLayerTreeModel::Flags oldFlags = layerTreeModel()->flags();
  if ( event->modifiers() & Qt::ControlModifier )
    layerTreeModel()->setFlags( oldFlags | QgsLayerTreeModel::ActionHierarchical );
  else
    layerTreeModel()->setFlags( oldFlags & ~QgsLayerTreeModel::ActionHierarchical );
  QTreeView::mouseReleaseEvent( event );
  layerTreeModel()->setFlags( oldFlags );
}

void QgsLayerTreeView::keyPressEvent( QKeyEvent *event )
{
  const QgsLayerTreeModel::Flags oldFlags = layerTreeModel()->flags();
  if ( event->modifiers() & Qt::ControlModifier )
    layerTreeModel()->setFlags( oldFlags | QgsLayerTreeModel::ActionHierarchical );
  else
    layerTreeModel()->setFlags( oldFlags & ~QgsLayerTreeModel::ActionHierarchical );
  QTreeView::keyPressEvent( event );
  layerTreeModel()->setFlags( oldFlags );
}

void QgsLayerTreeView::dropEvent( QDropEvent *event )
{
  if ( event->keyboardModifiers() & Qt::AltModifier )
  {
    event->accept();
  }
  QTreeView::dropEvent( event );
}
