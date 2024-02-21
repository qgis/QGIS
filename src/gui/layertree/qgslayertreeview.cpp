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

#include <QApplication>
#include <QMenu>
#include <QContextMenuEvent>
#include <QHeaderView>
#include <QMimeData>
#include <QScrollBar>

#ifdef ENABLE_MODELTEST
#include "modeltest.h"
#endif

#include "qgslayertreeviewindicator.h"
#include "qgslayertreeviewitemdelegate.h"


QgsLayerTreeView::QgsLayerTreeView( QWidget *parent )
  : QTreeView( parent )
  , mBlockDoubleClickTimer( new QTimer( this ) )

{
  mBlockDoubleClickTimer->setSingleShot( true );
  mBlockDoubleClickTimer->setInterval( QApplication::doubleClickInterval() );
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

  setLayerMarkWidth( static_cast< int >( QFontMetricsF( font() ).horizontalAdvance( 'l' ) * Qgis::UI_SCALE_FACTOR ) );

  connect( this, &QTreeView::collapsed, this, &QgsLayerTreeView::updateExpandedStateToNode );
  connect( this, &QTreeView::expanded, this, &QgsLayerTreeView::updateExpandedStateToNode );

  connect( horizontalScrollBar(), &QScrollBar::valueChanged, this, &QgsLayerTreeView::onHorizontalScroll );
}

QgsLayerTreeView::~QgsLayerTreeView()
{
  delete mMenuProvider;
  delete mBlockDoubleClickTimer;
}

void QgsLayerTreeView::setModel( QAbstractItemModel *model )
{
  QgsLayerTreeModel *treeModel = qobject_cast<QgsLayerTreeModel *>( model );
  if ( !treeModel )
    return;

  if ( mMessageBar )
    connect( treeModel, &QgsLayerTreeModel::messageEmitted, this,
             [ = ]( const QString & message, Qgis::MessageLevel level = Qgis::MessageLevel::Info, int duration = 5 )
  {
    Q_UNUSED( duration )
    mMessageBar->pushMessage( message, level );
  }
         );

  treeModel->addTargetScreenProperties( QgsScreenProperties( screen() ) );

  mProxyModel = new QgsLayerTreeProxyModel( treeModel, this );

  connect( mProxyModel, &QAbstractItemModel::rowsInserted, this, &QgsLayerTreeView::modelRowsInserted );
  connect( mProxyModel, &QAbstractItemModel::rowsRemoved, this, &QgsLayerTreeView::modelRowsRemoved );

#ifdef ENABLE_MODELTEST
  new ModelTest( mProxyModel, this );
#endif

  mProxyModel->setShowPrivateLayers( mShowPrivateLayers );
  mProxyModel->setHideValidLayers( mHideValidLayers );
  QTreeView::setModel( mProxyModel );

  connect( treeModel->rootGroup(), &QgsLayerTreeNode::expandedChanged, this, &QgsLayerTreeView::onExpandedChanged );
  connect( treeModel->rootGroup(), &QgsLayerTreeNode::customPropertyChanged, this, &QgsLayerTreeView::onCustomPropertyChanged );

  connect( selectionModel(), &QItemSelectionModel::currentChanged, this, &QgsLayerTreeView::onCurrentChanged );

  connect( treeModel, &QAbstractItemModel::modelReset, this, &QgsLayerTreeView::onModelReset );

  connect( treeModel, &QAbstractItemModel::dataChanged, this, &QgsLayerTreeView::onDataChanged );

  updateExpandedStateFromNode( treeModel->rootGroup() );

  //checkModel();
}

QgsLayerTreeModel *QgsLayerTreeView::layerTreeModel() const
{
  return mProxyModel ? qobject_cast<QgsLayerTreeModel *>( mProxyModel->sourceModel() ) : nullptr;
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

  setCurrentIndex( node2index( nodeLayer ) );
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

  const QModelIndex idx = indexAt( event->pos() );
  if ( !idx.isValid() )
    setCurrentIndex( QModelIndex() );

  QMenu *menu = mMenuProvider->createContextMenu();
  if ( menu )
  {
    emit contextMenuAboutToShow( menu );

    if ( menu->actions().count() != 0 )
      menu->exec( mapToGlobal( event->pos() ) );
    delete menu;
  }
}


void QgsLayerTreeView::modelRowsInserted( const QModelIndex &index, int start, int end )
{
  QgsLayerTreeNode *parentNode = index2node( index );
  if ( !parentNode )
    return;

  // Embedded widgets - replace placeholders in the model by actual widgets
  if ( layerTreeModel()->testFlag( QgsLayerTreeModel::UseEmbeddedWidgets ) && QgsLayerTree::isLayer( parentNode ) )
  {
    QgsLayerTreeLayer *nodeLayer = QgsLayerTree::toLayer( parentNode );
    if ( QgsMapLayer *layer = nodeLayer->layer() )
    {
      const int widgetsCount = layer->customProperty( QStringLiteral( "embeddedWidgets/count" ), 0 ).toInt();
      QList<QgsLayerTreeModelLegendNode *> legendNodes = layerTreeModel()->layerLegendNodes( nodeLayer, true );
      for ( int i = 0; i < widgetsCount; ++i )
      {
        const QString providerId = layer->customProperty( QStringLiteral( "embeddedWidgets/%1/id" ).arg( i ) ).toString();
        if ( QgsLayerTreeEmbeddedWidgetProvider *provider = QgsGui::layerTreeEmbeddedWidgetRegistry()->provider( providerId ) )
        {
          const QModelIndex index = legendNode2index( legendNodes[i] );
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
    const QStringList expandedNodeKeys = parentNode->customProperty( QStringLiteral( "expandedLegendNodes" ) ).toStringList();
    if ( expandedNodeKeys.isEmpty() )
      return;

    const auto constLayerLegendNodes = layerTreeModel()->layerLegendNodes( QgsLayerTree::toLayer( parentNode ), true );
    for ( QgsLayerTreeModelLegendNode *legendNode : constLayerLegendNodes )
    {
      const QString ruleKey = legendNode->data( static_cast< int >( QgsLayerTreeModelLegendNode::CustomRole::RuleKey ) ).toString();
      if ( expandedNodeKeys.contains( ruleKey ) )
        setExpanded( legendNode2index( legendNode ), true );
    }
    return;
  }

  QList<QgsLayerTreeNode *> children = parentNode->children();
  for ( int i = start; i <= end && i < children.count(); ++i )
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
  if ( QgsLayerTreeNode *node = index2node( index ) )
  {
    node->setExpanded( isExpanded( index ) );
  }
  else if ( QgsLayerTreeModelLegendNode *node = index2legendNode( index ) )
  {
    const QString ruleKey = node->data( static_cast< int >( QgsLayerTreeModelLegendNode::CustomRole::RuleKey ) ).toString();
    QStringList lst = node->layerNode()->customProperty( QStringLiteral( "expandedLegendNodes" ) ).toStringList();
    const bool expanded = isExpanded( index );
    const bool isInList = lst.contains( ruleKey );
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
  const QString layerCurrentID = layerCurrent ? layerCurrent->id() : QString();
  if ( mCurrentLayerID == layerCurrentID )
    return;

  // update the current index in model (the item will be underlined)
  QModelIndex proxyModelNodeLayerIndex;
  if ( layerCurrent )
  {
    QgsLayerTreeLayer *nodeLayer = layerTreeModel()->rootGroup()->findLayer( layerCurrentID );
    if ( nodeLayer )
      proxyModelNodeLayerIndex = node2index( nodeLayer );
  }

  if ( ! proxyModelNodeLayerIndex.isValid() )
  {
    mCurrentLayerID = QString();
    layerTreeModel()->setCurrentIndex( QModelIndex() );
  }
  else
  {
    mCurrentLayerID = layerCurrentID;
    layerTreeModel()->setCurrentIndex( mProxyModel->mapToSource( proxyModelNodeLayerIndex ) );
  }

  //checkModel();

  emit currentLayerChanged( layerCurrent );
}

void QgsLayerTreeView::onExpandedChanged( QgsLayerTreeNode *node, bool expanded )
{
  const QModelIndex idx = node2index( node );
  if ( isExpanded( idx ) != expanded )
    setExpanded( idx, expanded );
}

void QgsLayerTreeView::onCustomPropertyChanged( QgsLayerTreeNode *node, const QString &key )
{
  if ( key != QLatin1String( "expandedLegendNodes" ) || !QgsLayerTree::isLayer( node ) )
    return;

  const QSet<QString> expandedLegendNodes = qgis::listToSet( node->customProperty( QStringLiteral( "expandedLegendNodes" ) ).toStringList() );

  const QList<QgsLayerTreeModelLegendNode *> legendNodes = layerTreeModel()->layerLegendNodes( QgsLayerTree::toLayer( node ), true );
  for ( QgsLayerTreeModelLegendNode *legendNode : legendNodes )
  {
    const QString key = legendNode->data( static_cast< int >( QgsLayerTreeModelLegendNode::CustomRole::RuleKey ) ).toString();
    if ( !key.isEmpty() )
      setExpanded( legendNode2index( legendNode ), expandedLegendNodes.contains( key ) );
  }
}

void QgsLayerTreeView::onModelReset()
{
  updateExpandedStateFromNode( layerTreeModel()->rootGroup() );
  //checkModel();
}

void QgsLayerTreeView::updateExpandedStateFromNode( QgsLayerTreeNode *node )
{
  const QModelIndex idx = node2index( node );
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
    QgsLayerTreeNode *node = index2node( index );
    if ( node )
    {
      if ( QgsLayerTree::isLayer( node ) )
        return QgsLayerTree::toLayer( node )->layer();
    }
    else
    {
      // possibly a legend node
      QgsLayerTreeModelLegendNode *legendNode = index2legendNode( index );
      if ( legendNode )
        return legendNode->layerNode()->layer();
    }
  }
  return nullptr;
}

QgsLayerTreeNode *QgsLayerTreeView::currentNode() const
{
  return index2node( selectionModel()->currentIndex() );
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

  if ( QgsLayerTreeModelLegendNode *legendNode = index2legendNode( selectionModel()->currentIndex() ) )
  {
    QgsLayerTreeLayer *parent = legendNode->layerNode();
    if ( QgsLayerTree::isGroup( parent->parent() ) )
      return QgsLayerTree::toGroup( parent->parent() );
  }

  return nullptr;
}

QgsLayerTreeModelLegendNode *QgsLayerTreeView::currentLegendNode() const
{
  return index2legendNode( selectionModel()->currentIndex() );
}

QList<QgsLayerTreeNode *> QgsLayerTreeView::selectedNodes( bool skipInternal ) const
{
  QModelIndexList mapped;
  const QModelIndexList selected = selectionModel()->selectedIndexes();
  mapped.reserve( selected.size() );
  for ( const QModelIndex &index : selected )
    mapped << mProxyModel->mapToSource( index );

  return layerTreeModel()->indexes2nodes( mapped, skipInternal );
}

QList<QgsLayerTreeLayer *> QgsLayerTreeView::selectedLayerNodes() const
{
  QList<QgsLayerTreeLayer *> layerNodes;
  const QList<QgsLayerTreeNode *> constSelectedNodes = selectedNodes();
  layerNodes.reserve( constSelectedNodes.size() );
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
  const QList<QgsLayerTreeLayer *> constSelectedLayerNodes = selectedLayerNodes();
  list.reserve( constSelectedLayerNodes.size() );
  for ( QgsLayerTreeLayer *node : constSelectedLayerNodes )
  {
    if ( node->layer() )
      list << node->layer();
  }
  return list;
}

QList<QgsLayerTreeModelLegendNode *> QgsLayerTreeView::selectedLegendNodes() const
{
  QList<QgsLayerTreeModelLegendNode *> res;
  const QModelIndexList selected = selectionModel()->selectedIndexes();
  res.reserve( selected.size() );
  for ( const QModelIndex &index : selected )
  {
    const QModelIndex &modelIndex = mProxyModel->mapToSource( index );
    if ( QgsLayerTreeModelLegendNode *node = layerTreeModel()->index2legendNode( modelIndex ) )
    {
      res.push_back( node );
    }
  }

  return res;
}

QList<QgsMapLayer *> QgsLayerTreeView::selectedLayersRecursive() const
{
  QModelIndexList mapped;
  const QModelIndexList selected = selectionModel()->selectedIndexes();
  mapped.reserve( selected.size() );
  for ( const QModelIndex &index : selected )
    mapped << mProxyModel->mapToSource( index );

  const QList<QgsLayerTreeNode *> nodes = layerTreeModel()->indexes2nodes( mapped, false );
  const QSet<QgsMapLayer *> layersSet = QgsLayerTreeUtils::collectMapLayersRecursive( nodes );
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
      viewport()->repaint();
    } );
    update();
    viewport()->repaint(); //update() does not automatically trigger a repaint()
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
      const QString parentKey = legendNode->data( static_cast< int >( QgsLayerTreeModelLegendNode::CustomRole::ParentRuleKey ) ).toString();
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
    connect( layerTreeModel(), &QgsLayerTreeModel::messageEmitted, this,
             [ = ]( const QString & message, Qgis::MessageLevel level = Qgis::MessageLevel::Info, int duration = 5 )
  {
    Q_UNUSED( duration )
    mMessageBar->pushMessage( message, level );
  }
         );
}

void QgsLayerTreeView::setShowPrivateLayers( bool showPrivate )
{
  mShowPrivateLayers = showPrivate;
  mProxyModel->setShowPrivateLayers( showPrivate );
}

void QgsLayerTreeView::setHideValidLayers( bool hideValid )
{
  mHideValidLayers = hideValid;
  mProxyModel->setHideValidLayers( mHideValidLayers );
}

bool QgsLayerTreeView::showPrivateLayers() const
{
  return mShowPrivateLayers;
}

bool QgsLayerTreeView::hideValidLayers() const
{
  return mHideValidLayers;
}

void QgsLayerTreeView::mouseDoubleClickEvent( QMouseEvent *event )
{
  if ( mBlockDoubleClickTimer->isActive() )
    event->accept();
  else
    QTreeView::mouseDoubleClickEvent( event );
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
    const QList<QgsLayerTreeNode *> constSelectedNodes = selectedNodes();

    if ( !constSelectedNodes.isEmpty() )
    {
      const bool isFirstNodeChecked = constSelectedNodes[0]->itemVisibilityChecked();
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

void QgsLayerTreeView::dragEnterEvent( QDragEnterEvent *event )
{
  if ( event->mimeData()->hasUrls() || event->mimeData()->hasFormat( QStringLiteral( "application/x-vnd.qgis.qgis.uri" ) ) )
  {
    // the mime data are coming from layer tree, so ignore that, do not import those layers again
    if ( !event->mimeData()->hasFormat( QStringLiteral( "application/qgis.layertreemodeldata" ) ) )
    {
      event->accept();
      return;
    }
  }
  QTreeView::dragEnterEvent( event );
}

void QgsLayerTreeView::dragMoveEvent( QDragMoveEvent *event )
{
  if ( event->mimeData()->hasUrls() || event->mimeData()->hasFormat( QStringLiteral( "application/x-vnd.qgis.qgis.uri" ) ) )
  {
    // the mime data are coming from layer tree, so ignore that, do not import those layers again
    if ( !event->mimeData()->hasFormat( QStringLiteral( "application/qgis.layertreemodeldata" ) ) )
    {
      event->accept();
      return;
    }
  }
  QTreeView::dragMoveEvent( event );
}

void QgsLayerTreeView::dropEvent( QDropEvent *event )
{
  if ( event->mimeData()->hasUrls() || event->mimeData()->hasFormat( QStringLiteral( "application/x-vnd.qgis.qgis.uri" ) ) )
  {
    // the mime data are coming from layer tree, so ignore that, do not import those layers again
    if ( !event->mimeData()->hasFormat( QStringLiteral( "application/qgis.layertreemodeldata" ) ) )
    {
      event->accept();

      QModelIndex index = indexAt( event->pos() );
      if ( index.isValid() )
      {
        setCurrentIndex( index );
      }

      emit datasetsDropped( event );
      return;
    }
  }
  if ( event->keyboardModifiers() & Qt::AltModifier || event->keyboardModifiers() & Qt::ControlModifier )
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

void QgsLayerTreeView::onDataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles )
{
  Q_UNUSED( topLeft )
  Q_UNUSED( bottomRight )

  // If an item is resized asynchronously (e.g. wms legend)
  // The items below will need to be shifted vertically.
  // This doesn't happen automatically, unless the viewport update is triggered.

  if ( roles.contains( Qt::SizeHintRole ) )
    viewport()->update();

  mBlockDoubleClickTimer->start();
  //checkModel();
}

#if 0
// for model debugging
void QgsLayerTreeView::checkModel()
{
  std::function<void( QgsLayerTreeNode *, int )> debug;
  debug = [ & ]( QgsLayerTreeNode * node, int depth )
  {
    if ( depth == 1 )
      qDebug() << "----------------------------------------------";

    qDebug() << depth << node->name() << node2index( node ) << layerTreeModel()->rowCount( node2sourceIndex( node ) ) << mProxyModel->rowCount( node2index( node ) );
    Q_ASSERT( node == index2node( node2index( node ) ) );
    Q_ASSERT( node == layerTreeModel()->index2node( node2sourceIndex( node ) ) );
    Q_ASSERT( layerTreeModel()->rowCount( node2sourceIndex( node ) ) == mProxyModel->rowCount( node2index( node ) ) );

    for ( int i = 0; i < mProxyModel->rowCount( node2index( node ) ); i++ )
    {
      QgsLayerTreeNode *childNode { index2node( mProxyModel->index( i, 0, node2index( node ) ) ) };
      if ( childNode )
        debug( childNode, depth + 1 );
      else
        qDebug() << "Warning no child node!";
    }
  };
  debug( layerTreeModel()->rootGroup(), 1 );
}
#endif

QgsLayerTreeProxyModel *QgsLayerTreeView::proxyModel() const
{
  return mProxyModel;
}

QgsLayerTreeNode *QgsLayerTreeView::index2node( const QModelIndex &index ) const
{
  return layerTreeModel()->index2node( mProxyModel->mapToSource( index ) );
}

QModelIndex QgsLayerTreeView::node2index( QgsLayerTreeNode *node ) const
{
  return mProxyModel->mapFromSource( node2sourceIndex( node ) );
}

QModelIndex QgsLayerTreeView::node2sourceIndex( QgsLayerTreeNode *node ) const
{
  return layerTreeModel()->node2index( node );
}

QgsLayerTreeModelLegendNode *QgsLayerTreeView::index2legendNode( const QModelIndex &index ) const
{
  return QgsLayerTreeModel::index2legendNode( mProxyModel->mapToSource( index ) );
}

QModelIndex QgsLayerTreeView::legendNode2index( QgsLayerTreeModelLegendNode *legendNode )
{
  return mProxyModel->mapFromSource( legendNode2sourceIndex( legendNode ) );
}

QModelIndex QgsLayerTreeView::legendNode2sourceIndex( QgsLayerTreeModelLegendNode *legendNode )
{
  return layerTreeModel()->legendNode2index( legendNode );
}

QgsLayerTreeProxyModel::QgsLayerTreeProxyModel( QgsLayerTreeModel *treeModel, QObject *parent )
  : QSortFilterProxyModel( parent )
  , mLayerTreeModel( treeModel )
{
  setSourceModel( treeModel );
}

void QgsLayerTreeProxyModel::setFilterText( const QString &filterText )
{
  if ( filterText == mFilterText )
    return;

  mFilterText = filterText;
  invalidateFilter();
}

bool QgsLayerTreeProxyModel::filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const
{
  QgsLayerTreeNode *node = mLayerTreeModel->index2node( mLayerTreeModel->index( sourceRow, 0, sourceParent ) );
  return nodeShown( node );
}

bool QgsLayerTreeProxyModel::nodeShown( QgsLayerTreeNode *node ) const
{
  if ( !node )
    return true;

  if ( node->nodeType() == QgsLayerTreeNode::NodeGroup )
  {
    return true;
  }
  else
  {
    QgsMapLayer *layer = QgsLayerTree::toLayer( node )->layer();
    if ( !layer )
      return true;
    if ( !mFilterText.isEmpty() && !layer->name().contains( mFilterText, Qt::CaseInsensitive ) )
      return false;
    if ( ! mShowPrivateLayers && layer->flags().testFlag( QgsMapLayer::LayerFlag::Private ) )
    {
      return false;
    }
    if ( mHideValidLayers && layer->isValid() )
      return false;

    return true;
  }
}

bool QgsLayerTreeProxyModel::showPrivateLayers() const
{
  return mShowPrivateLayers;
}

void QgsLayerTreeProxyModel::setShowPrivateLayers( bool showPrivate )
{
  if ( showPrivate == mShowPrivateLayers )
    return;

  mShowPrivateLayers = showPrivate;
  invalidateFilter();
}

bool QgsLayerTreeProxyModel::hideValidLayers() const
{
  return mHideValidLayers;
}

void QgsLayerTreeProxyModel::setHideValidLayers( bool hideValid )
{
  if ( hideValid == mHideValidLayers )
    return;

  mHideValidLayers = hideValid;
  invalidateFilter();
}
