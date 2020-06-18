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
#include "qgsmessagebar.h"

#include "qgsgui.h"

#include <QMenu>
#include <QContextMenuEvent>
#include <QHeaderView>
#include <QScrollBar>

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

  // Ensure legend graphics are scrollable
  header()->setStretchLastSection( false );
  header()->setSectionResizeMode( QHeaderView::ResizeToContents );

  // If vertically scrolling by item, legend graphics can get clipped
  setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );

  setSelectionMode( ExtendedSelection );
  setDefaultDropAction( Qt::MoveAction );

  // we need a custom item delegate in order to draw indicators
  setItemDelegate( new QgsLayerTreeViewItemDelegate( this ) );
  setStyle( new QgsLayerTreeViewProxyStyle( this ) );

  setLayerMarkWidth( static_cast< int >( QFontMetricsF( font() ).width( 'l' ) * Qgis::UI_SCALE_FACTOR ) );

  connect( this, &QTreeView::collapsed, this, &QgsLayerTreeView::updateExpandedStateToNode );
  connect( this, &QTreeView::expanded, this, &QgsLayerTreeView::updateExpandedStateToNode );

  connect( horizontalScrollBar(), &QScrollBar::valueChanged, this, &QgsLayerTreeView::onHorizontalScroll );
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

  if ( mMessageBar )
    connect( layerTreeModel(), &QgsLayerTreeModel::messageEmitted,
             [ = ]( const QString & message, Qgis::MessageLevel level = Qgis::Info, int duration = 5 )
  {mMessageBar->pushMessage( message, level, duration );}
         );

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

void QgsLayerTreeView::setLayerVisible( QgsMapLayer *layer, bool visible )
{
  if ( !layer )
    return;
  QgsLayerTreeLayer *nodeLayer = layerTreeModel()->rootGroup()->findLayer( layer->id() );
  if ( !nodeLayer )
    return;
  nodeLayer->setItemVisibilityChecked( visible );
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
          QWidget *wdgt = provider->createWidget( layer, i );
          // Since column is resized to contents, limit the expanded width of embedded
          //  widgets, if they are not already limited, e.g. have the default MAX value.
          // Else, embedded widget may grow very wide due to large legend graphics.
          // NOTE: This approach DOES NOT work right. It causes horizontal scroll
          //       bar to disappear if the embedded widget is expanded and part
          //       of the last layer in the panel, even if much wider legend items
          //       are expanded above it. The correct width-limiting method should
          //       be setting fixed-width, hidpi-aware embedded widget items in a
          //       layout and appending an expanding QSpacerItem to end. This ensures
          //       full width is always created in the column by the embedded widget.
          //       See QgsLayerTreeOpacityWidget
          //if ( wdgt->maximumWidth() == QWIDGETSIZE_MAX )
          //{
          //  wdgt->setMaximumWidth( 250 );
          //}

          setIndexWidget( index, wdgt );
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

    const auto constLayerLegendNodes = layerTreeModel()->layerLegendNodes( QgsLayerTree::toLayer( parentNode ), true );
    for ( QgsLayerTreeModelLegendNode *legendNode : constLayerLegendNodes )
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

  QSet<QString> expandedLegendNodes = qgis::listToSet( node->customProperty( QStringLiteral( "expandedLegendNodes" ) ).toStringList() );

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

  const auto constChildren = node->children();
  for ( QgsLayerTreeNode *child : constChildren )
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
  const auto constSelectedNodes = selectedNodes();
  for ( QgsLayerTreeNode *node : constSelectedNodes )
  {
    if ( QgsLayerTree::isLayer( node ) )
      layerNodes << QgsLayerTree::toLayer( node );
  }
  return layerNodes;
}

QList<QgsMapLayer *> QgsLayerTreeView::selectedLayers() const
{
  QList<QgsMapLayer *> list;
  const auto constSelectedLayerNodes = selectedLayerNodes();
  for ( QgsLayerTreeLayer *node : constSelectedLayerNodes )
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
  return qgis::setToList( layersSet );
}

void QgsLayerTreeView::addIndicator( QgsLayerTreeNode *node, QgsLayerTreeViewIndicator *indicator )
{
  if ( !mIndicators[node].contains( indicator ) )
  {
    mIndicators[node].append( indicator );
    connect( indicator, &QgsLayerTreeViewIndicator::changed, this, [ = ]
    {
      update();
    } );
    update();
  }
}

void QgsLayerTreeView::removeIndicator( QgsLayerTreeNode *node, QgsLayerTreeViewIndicator *indicator )
{
  mIndicators[node].removeOne( indicator );
  update();
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
    const auto constLayerLegendNodes = model->layerLegendNodes( nodeLayer, true );
    for ( QgsLayerTreeModelLegendNode *legendNode : constLayerLegendNodes )
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
  const auto constChildren = parent->children();
  for ( QgsLayerTreeNode *node : constChildren )
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

void QgsLayerTreeView::setMessageBar( QgsMessageBar *messageBar )
{
  if ( mMessageBar == messageBar )
    return;

  mMessageBar = messageBar;

  if ( mMessageBar )
    connect( layerTreeModel(), &QgsLayerTreeModel::messageEmitted,
             [ = ]( const QString & message, Qgis::MessageLevel level = Qgis::Info, int duration = 5 )
  {mMessageBar->pushMessage( message, level, duration );}
         );
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
  if ( event->key() == Qt::Key_Space )
  {
    const auto constSelectedNodes = selectedNodes();

    if ( ! constSelectedNodes.isEmpty() )
    {
      bool isFirstNodeChecked = constSelectedNodes[0]->itemVisibilityChecked();
      for ( QgsLayerTreeNode *node : constSelectedNodes )
      {
        node->setItemVisibilityChecked( ! isFirstNodeChecked );
      }

      // if we call the original keyPress handler, the current item will be checked to the original state yet again
      return;
    }
  }

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

void QgsLayerTreeView::resizeEvent( QResizeEvent *event )
{
  // Since last column is resized to content (instead of stretched), the active
  // selection rectangle ends at width of widest visible item in tree,
  // regardless of which item is selected. This causes layer indicators to
  // become 'inactive' (not clickable and no tool tip) unless their rectangle
  // enters the view item's selection (active) rectangle.
  // Always resetting the minimum section size relative to the viewport ensures
  // the view item's selection rectangle extends to the right edge of the
  // viewport, which allows indicators to become active again.
  header()->setMinimumSectionSize( viewport()->width() );
  QTreeView::resizeEvent( event );
}

void QgsLayerTreeView::onHorizontalScroll( int value )
{
  Q_UNUSED( value )
  viewport()->update();
}
