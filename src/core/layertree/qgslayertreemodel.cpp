/***************************************************************************
  qgslayertreemodel.cpp
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

#include "qgslayertreemodel.h"

#include "qgslayertree.h"
#include "qgslayertreemodellegendnode.h"

#include <QMimeData>
#include <QTextStream>

#include "qgsdataitem.h"
#include "qgsmaplayerlegend.h"
#include "qgspluginlayer.h"
#include "qgsrasterlayer.h"
#include "qgsrendererv2.h"
#include "qgssymbollayerv2utils.h"
#include "qgsvectorlayer.h"


QgsLayerTreeModel::QgsLayerTreeModel( QgsLayerTreeGroup* rootNode, QObject *parent )
    : QAbstractItemModel( parent )
    , mRootNode( rootNode )
    , mFlags( ShowLegend | AllowLegendChangeState )
    , mAutoCollapseLegendNodesCount( -1 )
    , mLegendFilterByScale( 0 )
{
  connectToRootNode();

  mFontLayer.setBold( true );
}

QgsLayerTreeModel::~QgsLayerTreeModel()
{
  foreach ( QList<QgsLayerTreeModelLegendNode*> nodeL, mOriginalLegendNodes )
    qDeleteAll( nodeL );
  mOriginalLegendNodes.clear();
  mLegendNodes.clear(); // does not own the nodes
}

QgsLayerTreeNode* QgsLayerTreeModel::index2node( const QModelIndex& index ) const
{
  if ( !index.isValid() )
    return mRootNode;

  QObject* obj = reinterpret_cast<QObject*>( index.internalPointer() );
  return qobject_cast<QgsLayerTreeNode*>( obj );
}

QgsLayerTreeModelLegendNode* QgsLayerTreeModel::index2legendNode( const QModelIndex& index )
{
  return qobject_cast<QgsLayerTreeModelLegendNode*>( reinterpret_cast<QObject*>( index.internalPointer() ) );
}

QModelIndex QgsLayerTreeModel::legendNode2index( QgsLayerTreeModelLegendNode* legendNode )
{
  QModelIndex parentIndex = node2index( legendNode->parent() );
  Q_ASSERT( parentIndex.isValid() );
  int row = mLegendNodes[legendNode->parent()].indexOf( legendNode );
  Q_ASSERT( row >= 0 );
  return index( row, 0, parentIndex );
}


int QgsLayerTreeModel::rowCount( const QModelIndex &parent ) const
{
  if ( index2legendNode( parent ) )
  {
    return 0; // they are leaves
  }

  QgsLayerTreeNode* n = index2node( parent );

  if ( parent.isValid() && parent.column() != 0 )
    return 0;

  if ( QgsLayerTree::isLayer( n ) )
  {
    QgsLayerTreeLayer* nL = QgsLayerTree::toLayer( n );

    if ( mLegendNodes[nL].count() == 1 && mLegendNodes[nL][0]->isEmbeddedInParent() )
      return 0;

    return mLegendNodes[nL].count();
  }

  return n->children().count();
}

int QgsLayerTreeModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent );
  return 1;
}

QModelIndex QgsLayerTreeModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( column != 0 || row < 0 || row >= rowCount( parent ) )
    return QModelIndex();

  QgsLayerTreeNode* n = index2node( parent );

  if ( !n )
  {
    QgsLayerTreeModelLegendNode* sym = index2legendNode( parent );
    Q_ASSERT( sym );
    return QModelIndex(); // have no children
  }

  if ( !n || column != 0 || row >= rowCount( parent ) )
    return QModelIndex();

  if ( testFlag( ShowLegend ) && QgsLayerTree::isLayer( n ) )
  {
    QgsLayerTreeLayer* nL = QgsLayerTree::toLayer( n );
    Q_ASSERT( mLegendNodes.contains( nL ) );
    return createIndex( row, column, static_cast<QObject*>( mLegendNodes[nL].at( row ) ) );
  }

  return createIndex( row, column, static_cast<QObject*>( n->children().at( row ) ) );
}

QModelIndex QgsLayerTreeModel::parent( const QModelIndex &child ) const
{
  if ( !child.isValid() )
    return QModelIndex();

  QgsLayerTreeNode* n = index2node( child );

  QgsLayerTreeNode* parentNode = 0;

  if ( !n )
  {
    QgsLayerTreeModelLegendNode* sym = index2legendNode( child );
    Q_ASSERT( sym );
    parentNode = sym->parent();
  }
  else
    parentNode = n->parent(); // must not be null
  Q_ASSERT( parentNode );

  QgsLayerTreeNode* grandParentNode = parentNode->parent();
  if ( !grandParentNode )
    return QModelIndex();  // root node -> invalid index

  int row = grandParentNode->children().indexOf( parentNode );
  Q_ASSERT( row >= 0 );
  return createIndex( row, 0, static_cast<QObject*>( parentNode ) );
}

QVariant QgsLayerTreeModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  if ( QgsLayerTreeModelLegendNode* sym = index2legendNode( index ) )
  {
    if ( role == Qt::CheckStateRole && !testFlag( AllowLegendChangeState ) )
      return QVariant();
    return sym->data( role );
  }

  QgsLayerTreeNode* node = index2node( index );
  if ( role == Qt::DisplayRole || role == Qt::EditRole )
  {
    if ( QgsLayerTree::isGroup( node ) )
      return QgsLayerTree::toGroup( node )->name();
    else if ( QgsLayerTree::isLayer( node ) )
    {
      QgsLayerTreeLayer* nodeLayer = QgsLayerTree::toLayer( node );
      QString name = nodeLayer->layerName();
      if ( nodeLayer->customProperty( "showFeatureCount", 0 ).toInt() && role == Qt::DisplayRole )
      {
        QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer*>( nodeLayer->layer() );
        if ( vlayer && vlayer->pendingFeatureCount() >= 0 )
          name += QString( " [%1]" ).arg( vlayer->pendingFeatureCount() );
      }
      return name;
    }
  }
  else if ( role == Qt::DecorationRole && index.column() == 0 )
  {
    if ( QgsLayerTree::isGroup( node ) )
      return iconGroup();
    else if ( QgsLayerTree::isLayer( node ) )
    {
      QgsLayerTreeLayer* nodeLayer = QgsLayerTree::toLayer( node );

      QgsMapLayer* layer = nodeLayer->layer();
      if ( !layer )
        return QVariant();

      // icons possibly overriding default icon
      if ( layer->type() == QgsMapLayer::RasterLayer )
      {
        if ( testFlag( ShowRasterPreviewIcon ) )
        {
          QgsRasterLayer* rlayer = qobject_cast<QgsRasterLayer *>( layer );
          return QIcon( rlayer->previewAsPixmap( QSize( 32, 32 ) ) );
        }
      }
      else if ( layer->type() == QgsMapLayer::VectorLayer )
      {
        QgsVectorLayer* vlayer = static_cast<QgsVectorLayer*>( layer );
        if ( vlayer->isEditable() )
        {
          if ( vlayer->isModified() )
            return QIcon( QgsApplication::getThemePixmap( "/mIconEditableEdits.png" ) );
          else
            return QIcon( QgsApplication::getThemePixmap( "/mIconEditable.png" ) );
        }
      }

      // if there's just on legend entry that should be embedded in layer - do that!
      if ( testFlag( ShowLegend ) && mLegendNodes[nodeLayer].count() == 1 && mLegendNodes[nodeLayer][0]->isEmbeddedInParent() )
        return mLegendNodes[nodeLayer][0]->data( Qt::DecorationRole );

      if ( layer->type() == QgsMapLayer::RasterLayer )
      {
        return QgsLayerItem::iconRaster();
      }
      else if ( layer->type() == QgsMapLayer::VectorLayer )
      {
        QgsVectorLayer* vlayer = static_cast<QgsVectorLayer*>( layer );
        if ( vlayer->geometryType() == QGis::Point )
          return QgsLayerItem::iconPoint();
        else if ( vlayer->geometryType() == QGis::Line )
          return QgsLayerItem::iconLine();
        else if ( vlayer->geometryType() == QGis::Polygon )
          return QgsLayerItem::iconPolygon();
        else if ( vlayer->geometryType() == QGis::NoGeometry )
          return QgsLayerItem::iconTable();
      }
      return QgsLayerItem::iconDefault();
    }
  }
  else if ( role == Qt::CheckStateRole )
  {
    if ( !testFlag( AllowNodeChangeVisibility ) )
      return QVariant();

    if ( QgsLayerTree::isLayer( node ) )
    {
      QgsLayerTreeLayer* nodeLayer = QgsLayerTree::toLayer( node );
      if ( nodeLayer->layer() && nodeLayer->layer()->type() == QgsMapLayer::VectorLayer )
      {
        if ( qobject_cast<QgsVectorLayer*>( nodeLayer->layer() )->geometryType() == QGis::NoGeometry )
          return QVariant(); // do not show checkbox for non-spatial tables
      }
      return nodeLayer->isVisible();
    }
    else if ( QgsLayerTree::isGroup( node ) )
    {
      QgsLayerTreeGroup* nodeGroup = QgsLayerTree::toGroup( node );
      return nodeGroup->isVisible();
    }
  }
  else if ( role == Qt::FontRole )
  {
    QFont f( QgsLayerTree::isLayer( node ) ? mFontLayer : ( QgsLayerTree::isGroup( node ) ? mFontGroup : QFont() ) );
    if ( node->customProperty( "embedded" ).toInt() )
      f.setItalic( true );
    if ( index == mCurrentIndex )
      f.setUnderline( true );
    return f;
  }
  else if ( role == Qt::ToolTipRole )
  {
    if ( QgsLayerTree::isLayer( node ) )
    {
      if ( QgsMapLayer* layer = QgsLayerTree::toLayer( node )->layer() )
        return layer->publicSource();
    }
  }

  return QVariant();
}

Qt::ItemFlags QgsLayerTreeModel::flags( const QModelIndex& index ) const
{
  if ( !index.isValid() )
  {
    Qt::ItemFlags rootFlags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if ( testFlag( AllowNodeReorder ) )
      rootFlags |= Qt::ItemIsDropEnabled;
    return rootFlags;
  }

  if ( QgsLayerTreeModelLegendNode* symn = index2legendNode( index ) )
  {
    Qt::ItemFlags f = symn->flags();
    if ( !testFlag( AllowLegendChangeState ) )
      f &= ~Qt::ItemIsUserCheckable;
    return f;
  }

  Qt::ItemFlags f = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

  if ( testFlag( AllowNodeRename ) )
    f |= Qt::ItemIsEditable;

  QgsLayerTreeNode* node = index2node( index );
  bool isEmbedded = node->customProperty( "embedded" ).toInt();

  if ( testFlag( AllowNodeReorder ) )
  {
    // only root embedded nodes can be reordered
    if ( !isEmbedded || ( isEmbedded && node->parent() && !node->parent()->customProperty( "embedded" ).toInt() ) )
      f |= Qt::ItemIsDragEnabled;
  }

  if ( testFlag( AllowNodeChangeVisibility ) && ( QgsLayerTree::isLayer( node ) || QgsLayerTree::isGroup( node ) ) )
    f |= Qt::ItemIsUserCheckable;

  if ( testFlag( AllowNodeReorder ) && QgsLayerTree::isGroup( node ) && !isEmbedded )
    f |= Qt::ItemIsDropEnabled;

  return f;
}

bool QgsLayerTreeModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
  QgsLayerTreeModelLegendNode *sym = index2legendNode( index );
  if ( sym )
  {
    if ( role == Qt::CheckStateRole && !testFlag( AllowLegendChangeState ) )
      return false;
    bool res = sym->setData( value, role );
    if ( res )
      emit dataChanged( index, index );
    return res;
  }

  QgsLayerTreeNode* node = index2node( index );
  if ( !node )
    return QgsLayerTreeModel::setData( index, value, role );

  if ( role == Qt::CheckStateRole )
  {
    if ( !testFlag( AllowNodeChangeVisibility ) )
      return false;

    if ( QgsLayerTree::isLayer( node ) )
    {
      QgsLayerTreeLayer* layer = QgsLayerTree::toLayer( node );
      layer->setVisible(( Qt::CheckState )value.toInt() );
      return true;
    }

    if ( QgsLayerTree::isGroup( node ) )
    {
      QgsLayerTreeGroup* group = QgsLayerTree::toGroup( node );
      group->setVisible(( Qt::CheckState )value.toInt() );
      return true;
    }

    return true;
  }
  else if ( role == Qt::EditRole )
  {
    if ( !testFlag( AllowNodeRename ) )
      return false;

    if ( QgsLayerTree::isLayer( node ) )
    {
      QgsLayerTreeLayer* layer = QgsLayerTree::toLayer( node );
      layer->setLayerName( value.toString() );
      emit dataChanged( index, index );
    }
    else if ( QgsLayerTree::isGroup( node ) )
    {
      QgsLayerTree::toGroup( node )->setName( value.toString() );
      emit dataChanged( index, index );
    }
  }

  return QAbstractItemModel::setData( index, value, role );
}

QModelIndex QgsLayerTreeModel::node2index( QgsLayerTreeNode* node ) const
{
  if ( !node->parent() )
    return QModelIndex(); // this is the only root item -> invalid index
  QModelIndex parentIndex = node2index( node->parent() );

  int row = node->parent()->children().indexOf( node );
  Q_ASSERT( row >= 0 );
  return index( row, 0, parentIndex );
}


static bool _isChildOfNode( QgsLayerTreeNode* child, QgsLayerTreeNode* node )
{
  if ( !child->parent() )
    return false;

  if ( child->parent() == node )
    return true;

  return _isChildOfNode( child->parent(), node );
}

static bool _isChildOfNodes( QgsLayerTreeNode* child, QList<QgsLayerTreeNode*> nodes )
{
  foreach ( QgsLayerTreeNode* n, nodes )
  {
    if ( _isChildOfNode( child, n ) )
      return true;
  }
  return false;
}


QList<QgsLayerTreeNode*> QgsLayerTreeModel::indexes2nodes( const QModelIndexList& list, bool skipInternal ) const
{
  QList<QgsLayerTreeNode*> nodes;
  foreach ( QModelIndex index, list )
  {
    QgsLayerTreeNode* node = index2node( index );
    if ( !node )
      continue;

    nodes << node;
  }

  if ( !skipInternal )
    return nodes;

  // remove any children of nodes if both parent node and children are selected
  QList<QgsLayerTreeNode*> nodesFinal;
  foreach ( QgsLayerTreeNode* node, nodes )
  {
    if ( !_isChildOfNodes( node, nodes ) )
      nodesFinal << node;
  }

  return nodesFinal;
}

bool QgsLayerTreeModel::isIndexSymbologyNode( const QModelIndex& index ) const
{
  return index2legendNode( index ) != 0;
}

QgsLayerTreeLayer* QgsLayerTreeModel::layerNodeForSymbologyNode( const QModelIndex& index ) const
{
  QgsLayerTreeModelLegendNode* symNode = index2legendNode( index );
  return symNode ? symNode->parent() : 0;
}

QList<QgsLayerTreeModelLegendNode*> QgsLayerTreeModel::layerLegendNodes( QgsLayerTreeLayer* nodeLayer )
{
  return mLegendNodes.value( nodeLayer );
}

QgsLayerTreeGroup*QgsLayerTreeModel::rootGroup()
{
  return mRootNode;
}

void QgsLayerTreeModel::setRootGroup( QgsLayerTreeGroup* newRootGroup )
{
  beginResetModel();

  disconnectFromRootNode();

  Q_ASSERT( mLegendNodes.isEmpty() );
  Q_ASSERT( mOriginalLegendNodes.isEmpty() );

  mRootNode = newRootGroup;

  endResetModel();

  connectToRootNode();
}

void QgsLayerTreeModel::refreshLayerLegend( QgsLayerTreeLayer* nodeLayer )
{
  // update title
  QModelIndex idx = node2index( nodeLayer );
  emit dataChanged( idx, idx );

  // update children
  int oldNodeCount = rowCount( idx );
  beginRemoveRows( idx, 0, oldNodeCount - 1 );
  removeLegendFromLayer( nodeLayer );
  endRemoveRows();

  addLegendToLayer( nodeLayer );
  int newNodeCount = rowCount( idx );

  // automatic collapse of legend nodes - useful if a layer has many legend nodes
  if ( mAutoCollapseLegendNodesCount != -1 && oldNodeCount != newNodeCount && newNodeCount >= mAutoCollapseLegendNodesCount )
    nodeLayer->setExpanded( false );
}

QModelIndex QgsLayerTreeModel::currentIndex() const
{
  return mCurrentIndex;
}

void QgsLayerTreeModel::setCurrentIndex( const QModelIndex& currentIndex )
{
  QModelIndex oldIndex = mCurrentIndex;
  mCurrentIndex = currentIndex;

  if ( oldIndex.isValid() )
    emit dataChanged( oldIndex, oldIndex );
  if ( currentIndex.isValid() )
    emit dataChanged( currentIndex, currentIndex );
}


void QgsLayerTreeModel::setLayerTreeNodeFont( int nodeType, const QFont& font )
{
  if ( nodeType == QgsLayerTreeNode::NodeGroup )
  {
    if ( mFontGroup != font )
    {
      mFontGroup = font;
      recursivelyEmitDataChanged();
    }
  }
  else if ( nodeType == QgsLayerTreeNode::NodeLayer )
  {
    if ( mFontLayer != font )
    {
      mFontLayer = font;
      recursivelyEmitDataChanged();
    }
  }
  else
  {
    QgsDebugMsg( "invalid node type" );
  }
}


QFont QgsLayerTreeModel::layerTreeNodeFont( int nodeType ) const
{
  if ( nodeType == QgsLayerTreeNode::NodeGroup )
    return mFontGroup;
  else if ( nodeType == QgsLayerTreeNode::NodeLayer )
    return mFontLayer;
  else
  {
    QgsDebugMsg( "invalid node type" );
    return QFont();
  }
}

void QgsLayerTreeModel::setLegendFilterByScale( double scaleDenominator )
{
  mLegendFilterByScale = scaleDenominator;

  // this could be later done in more efficient way
  // by just updating active legend nodes, without refreshing original legend nodes
  foreach ( QgsLayerTreeLayer* nodeLayer, mRootNode->findLayers() )
    refreshLayerLegend( nodeLayer );
}

void QgsLayerTreeModel::nodeWillAddChildren( QgsLayerTreeNode* node, int indexFrom, int indexTo )
{
  Q_ASSERT( node );
  beginInsertRows( node2index( node ), indexFrom, indexTo );
}

static QList<QgsLayerTreeLayer*> _layerNodesInSubtree( QgsLayerTreeNode* node, int indexFrom, int indexTo )
{
  QList<QgsLayerTreeNode*> children = node->children();
  QList<QgsLayerTreeLayer*> newLayerNodes;
  for ( int i = indexFrom; i <= indexTo; ++i )
  {
    QgsLayerTreeNode* child = children.at( i );
    if ( QgsLayerTree::isLayer( child ) )
      newLayerNodes << QgsLayerTree::toLayer( child );
    else if ( QgsLayerTree::isGroup( child ) )
      newLayerNodes << QgsLayerTree::toGroup( child )->findLayers();
  }
  return newLayerNodes;
}

void QgsLayerTreeModel::nodeAddedChildren( QgsLayerTreeNode* node, int indexFrom, int indexTo )
{
  Q_ASSERT( node );

  endInsertRows();

  foreach ( QgsLayerTreeLayer* newLayerNode, _layerNodesInSubtree( node, indexFrom, indexTo ) )
    connectToLayer( newLayerNode );
}

void QgsLayerTreeModel::nodeWillRemoveChildren( QgsLayerTreeNode* node, int indexFrom, int indexTo )
{
  Q_ASSERT( node );

  beginRemoveRows( node2index( node ), indexFrom, indexTo );

  // disconnect from layers and remove their legend
  foreach ( QgsLayerTreeLayer* nodeLayer, _layerNodesInSubtree( node, indexFrom, indexTo ) )
    disconnectFromLayer( nodeLayer );
}

void QgsLayerTreeModel::nodeRemovedChildren()
{
  endRemoveRows();
}

void QgsLayerTreeModel::nodeVisibilityChanged( QgsLayerTreeNode* node )
{
  Q_ASSERT( node );

  QModelIndex index = node2index( node );
  emit dataChanged( index, index );
}


void QgsLayerTreeModel::nodeCustomPropertyChanged( QgsLayerTreeNode* node, const QString& key )
{
  if ( QgsLayerTree::isLayer( node ) && key == "showFeatureCount" )
    refreshLayerLegend( QgsLayerTree::toLayer( node ) );
}


void QgsLayerTreeModel::nodeLayerLoaded()
{
  QgsLayerTreeLayer* nodeLayer = qobject_cast<QgsLayerTreeLayer*>( sender() );
  if ( !nodeLayer )
    return;

  // deffered connection to the layer
  connectToLayer( nodeLayer );
}

void QgsLayerTreeModel::nodeLayerWillBeUnloaded()
{
  QgsLayerTreeLayer* nodeLayer = qobject_cast<QgsLayerTreeLayer*>( sender() );
  if ( !nodeLayer )
    return;

  disconnectFromLayer( nodeLayer );

  // wait for the layer to appear again
  connect( nodeLayer, SIGNAL( layerLoaded() ), this, SLOT( nodeLayerLoaded() ) );
}

void QgsLayerTreeModel::layerLegendChanged()
{
  if ( !testFlag( ShowLegend ) )
    return;

  QgsMapLayer* layer = qobject_cast<QgsMapLayer*>( sender() );
  if ( !layer )
    return;

  QgsLayerTreeLayer* nodeLayer = mRootNode->findLayer( layer->id() );
  if ( !nodeLayer )
    return;

  refreshLayerLegend( nodeLayer );
}

void QgsLayerTreeModel::layerNeedsUpdate()
{
  QgsMapLayer* layer = qobject_cast<QgsMapLayer*>( sender() );
  if ( !layer )
    return;

  QgsLayerTreeLayer* nodeLayer = mRootNode->findLayer( layer->id() );
  if ( !nodeLayer )
    return;

  QModelIndex index = node2index( nodeLayer );
  emit dataChanged( index, index );

  if ( nodeLayer->customProperty( "showFeatureCount" ).toInt() )
    refreshLayerLegend( nodeLayer );
}


void QgsLayerTreeModel::legendNodeDataChanged()
{
  QgsLayerTreeModelLegendNode* legendNode = qobject_cast<QgsLayerTreeModelLegendNode*>( sender() );
  if ( !legendNode )
    return;

  QModelIndex index = legendNode2index( legendNode );
  emit dataChanged( index, index );
}


void QgsLayerTreeModel::removeLegendFromLayer( QgsLayerTreeLayer* nodeLayer )
{
  if ( mLegendNodes.contains( nodeLayer ) )
  {
    qDeleteAll( mOriginalLegendNodes[nodeLayer] );
    mOriginalLegendNodes.remove( nodeLayer );
    mLegendNodes.remove( nodeLayer );
  }
}


void QgsLayerTreeModel::addLegendToLayer( QgsLayerTreeLayer* nodeL )
{
  if ( !nodeL->layer() )
    return;

  QgsMapLayerLegend* layerLegend = nodeL->layer()->legend();
  if ( !layerLegend )
    return;

  QList<QgsLayerTreeModelLegendNode*> lstNew = layerLegend->createLayerTreeModelLegendNodes( nodeL );

  // apply filtering defined in layer node's custom properties (reordering, filtering, custom labels)
  QgsMapLayerLegendUtils::applyLayerNodeProperties( nodeL, lstNew );

  QList<QgsLayerTreeModelLegendNode*> filteredLstNew = filterLegendNodes( lstNew );

  beginInsertRows( node2index( nodeL ), 0, filteredLstNew.count() - 1 );

  foreach ( QgsLayerTreeModelLegendNode* n, lstNew )
  {
    n->setParent( this );
    connect( n, SIGNAL( dataChanged() ), this, SLOT( legendNodeDataChanged() ) );
  }

  mOriginalLegendNodes[nodeL] = lstNew;
  mLegendNodes[nodeL] = filteredLstNew;

  endInsertRows();
}


void QgsLayerTreeModel::connectToLayer( QgsLayerTreeLayer* nodeLayer )
{
  if ( !nodeLayer->layer() )
  {
    // in order to connect to layer, we need to have it loaded.
    // keep an eye on the layer ID: once loaded, we will use it
    connect( nodeLayer, SIGNAL( layerLoaded() ), this, SLOT( nodeLayerLoaded() ) );
    return;
  }

  // watch if the layer is getting removed
  connect( nodeLayer, SIGNAL( layerWillBeUnloaded() ), this, SLOT( nodeLayerWillBeUnloaded() ) );

  if ( testFlag( ShowLegend ) )
  {
    addLegendToLayer( nodeLayer );

    // automatic collapse of legend nodes - useful if a layer has many legend nodes
    if ( !mRootNode->customProperty( "loading" ).toBool() )
    {
      if ( mAutoCollapseLegendNodesCount != -1 && rowCount( node2index( nodeLayer ) )  >= mAutoCollapseLegendNodesCount )
        nodeLayer->setExpanded( false );
    }
  }

  QgsMapLayer* layer = nodeLayer->layer();
  connect( layer, SIGNAL( legendChanged() ), this, SLOT( layerLegendChanged() ), Qt::UniqueConnection );

  if ( layer->type() == QgsMapLayer::VectorLayer )
  {
    // using unique connection because there may be temporarily more nodes for a layer than just one
    // which would create multiple connections, however disconnect() would disconnect all multiple connections
    // even if we wanted to disconnect just one connection in each call.
    connect( layer, SIGNAL( editingStarted() ), this, SLOT( layerNeedsUpdate() ), Qt::UniqueConnection );
    connect( layer, SIGNAL( editingStopped() ), this, SLOT( layerNeedsUpdate() ), Qt::UniqueConnection );
    connect( layer, SIGNAL( layerModified() ), this, SLOT( layerNeedsUpdate() ), Qt::UniqueConnection );
    connect( layer, SIGNAL( layerNameChanged() ), this, SLOT( layerNeedsUpdate() ), Qt::UniqueConnection );
  }
}

// try to find out if the layer ID is present in the tree multiple times
static int _numLayerCount( QgsLayerTreeGroup* group, const QString& layerId )
{
  int count = 0;
  foreach ( QgsLayerTreeNode* child, group->children() )
  {
    if ( QgsLayerTree::isLayer( child ) )
    {
      if ( QgsLayerTree::toLayer( child )->layerId() == layerId )
        count++;
    }
    else if ( QgsLayerTree::isGroup( child ) )
    {
      count += _numLayerCount( QgsLayerTree::toGroup( child ), layerId );
    }
  }
  return count;
}

void QgsLayerTreeModel::disconnectFromLayer( QgsLayerTreeLayer* nodeLayer )
{
  disconnect( nodeLayer, 0, this, 0 ); // disconnect from delayed load of layer

  if ( !nodeLayer->layer() )
    return; // we were never connected

  if ( testFlag( ShowLegend ) )
  {
    removeLegendFromLayer( nodeLayer );
  }

  if ( _numLayerCount( mRootNode, nodeLayer->layerId() ) == 1 )
  {
    // last instance of the layer in the tree: disconnect from all signals from layer!
    disconnect( nodeLayer->layer(), 0, this, 0 );
  }
}

void QgsLayerTreeModel::connectToLayers( QgsLayerTreeGroup* parentGroup )
{
  foreach ( QgsLayerTreeNode* node, parentGroup->children() )
  {
    if ( QgsLayerTree::isGroup( node ) )
      connectToLayers( QgsLayerTree::toGroup( node ) );
    else if ( QgsLayerTree::isLayer( node ) )
      connectToLayer( QgsLayerTree::toLayer( node ) );
  }
}

void QgsLayerTreeModel::disconnectFromLayers( QgsLayerTreeGroup* parentGroup )
{
  foreach ( QgsLayerTreeNode* node, parentGroup->children() )
  {
    if ( QgsLayerTree::isGroup( node ) )
      disconnectFromLayers( QgsLayerTree::toGroup( node ) );
    else if ( QgsLayerTree::isLayer( node ) )
      disconnectFromLayer( QgsLayerTree::toLayer( node ) );
  }
}

void QgsLayerTreeModel::connectToRootNode()
{
  Q_ASSERT( mRootNode );

  connect( mRootNode, SIGNAL( willAddChildren( QgsLayerTreeNode*, int, int ) ), this, SLOT( nodeWillAddChildren( QgsLayerTreeNode*, int, int ) ) );
  connect( mRootNode, SIGNAL( addedChildren( QgsLayerTreeNode*, int, int ) ), this, SLOT( nodeAddedChildren( QgsLayerTreeNode*, int, int ) ) );
  connect( mRootNode, SIGNAL( willRemoveChildren( QgsLayerTreeNode*, int, int ) ), this, SLOT( nodeWillRemoveChildren( QgsLayerTreeNode*, int, int ) ) );
  connect( mRootNode, SIGNAL( removedChildren( QgsLayerTreeNode*, int, int ) ), this, SLOT( nodeRemovedChildren() ) );
  connect( mRootNode, SIGNAL( visibilityChanged( QgsLayerTreeNode*, Qt::CheckState ) ), this, SLOT( nodeVisibilityChanged( QgsLayerTreeNode* ) ) );

  connect( mRootNode, SIGNAL( customPropertyChanged( QgsLayerTreeNode*, QString ) ), this, SLOT( nodeCustomPropertyChanged( QgsLayerTreeNode*, QString ) ) );

  connectToLayers( mRootNode );
}

void QgsLayerTreeModel::disconnectFromRootNode()
{
  disconnect( mRootNode, 0, this, 0 );

  disconnectFromLayers( mRootNode );
}

void QgsLayerTreeModel::recursivelyEmitDataChanged( const QModelIndex& idx )
{
  QgsLayerTreeNode* node = index2node( idx );
  if ( !node )
    return;

  int count = node->children().count();
  if ( count == 0 )
    return;
  emit dataChanged( index( 0, 0, idx ), index( count - 1, 0, idx ) );
  for ( int i = 0; i < count; ++i )
    recursivelyEmitDataChanged( index( i, 0, idx ) );
}


Qt::DropActions QgsLayerTreeModel::supportedDropActions() const
{
  return Qt::MoveAction;
}

QStringList QgsLayerTreeModel::mimeTypes() const
{
  QStringList types;
  types << "application/qgis.layertreemodeldata";
  return types;
}


QMimeData* QgsLayerTreeModel::mimeData( const QModelIndexList& indexes ) const
{
  QList<QgsLayerTreeNode*> nodesFinal = indexes2nodes( indexes, true );

  if ( nodesFinal.count() == 0 )
    return 0;

  QMimeData *mimeData = new QMimeData();

  QDomDocument doc;
  QDomElement rootElem = doc.createElement( "layer_tree_model_data" );
  foreach ( QgsLayerTreeNode* node, nodesFinal )
    node->writeXML( rootElem );
  doc.appendChild( rootElem );
  QString txt = doc.toString();

  mimeData->setData( "application/qgis.layertreemodeldata", txt.toUtf8() );
  return mimeData;
}

bool QgsLayerTreeModel::dropMimeData( const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent )
{
  if ( action == Qt::IgnoreAction )
    return true;

  if ( !data->hasFormat( "application/qgis.layertreemodeldata" ) )
    return false;

  if ( column > 0 )
    return false;

  QgsLayerTreeNode* nodeParent = index2node( parent );
  if ( !QgsLayerTree::isGroup( nodeParent ) )
    return false;

  QByteArray encodedData = data->data( "application/qgis.layertreemodeldata" );

  QDomDocument doc;
  if ( !doc.setContent( QString::fromUtf8( encodedData ) ) )
    return false;

  QDomElement rootElem = doc.documentElement();
  if ( rootElem.tagName() != "layer_tree_model_data" )
    return false;

  QList<QgsLayerTreeNode*> nodes;

  QDomElement elem = rootElem.firstChildElement();
  while ( !elem.isNull() )
  {
    QgsLayerTreeNode* node = QgsLayerTreeNode::readXML( elem );
    if ( node )
      nodes << node;

    elem = elem.nextSiblingElement();
  }

  if ( nodes.count() == 0 )
    return false;

  if ( parent.isValid() && row == -1 )
    row = 0; // if dropped directly onto group item, insert at first position

  QgsLayerTree::toGroup( nodeParent )->insertChildNodes( row, nodes );

  return true;
}

bool QgsLayerTreeModel::removeRows( int row, int count, const QModelIndex& parent )
{
  QgsLayerTreeNode* parentNode = index2node( parent );
  if ( QgsLayerTree::isGroup( parentNode ) )
  {
    QgsLayerTree::toGroup( parentNode )->removeChildren( row, count );
    return true;
  }
  return false;
}

void QgsLayerTreeModel::setFlags( QgsLayerTreeModel::Flags f )
{
  mFlags = f;
}

void QgsLayerTreeModel::setFlag( QgsLayerTreeModel::Flag f, bool on )
{
  if ( on )
    mFlags |= f;
  else
    mFlags &= ~f;
}

QgsLayerTreeModel::Flags QgsLayerTreeModel::flags() const
{
  return mFlags;
}

bool QgsLayerTreeModel::testFlag( QgsLayerTreeModel::Flag f ) const
{
  return mFlags.testFlag( f );
}


const QIcon& QgsLayerTreeModel::iconGroup()
{
  static QIcon icon;

  if ( icon.isNull() )
    icon = QgsApplication::getThemeIcon( "/mActionFolder.png" );

  return icon;
}

QList<QgsLayerTreeModelLegendNode*> QgsLayerTreeModel::filterLegendNodes( const QList<QgsLayerTreeModelLegendNode*>& nodes )
{
  QList<QgsLayerTreeModelLegendNode*> filtered;
  foreach ( QgsLayerTreeModelLegendNode* node, nodes )
  {
    if ( mLegendFilterByScale > 0 && !node->isScaleOK( mLegendFilterByScale ) )
      continue;

    filtered << node;
  }
  return filtered;
}
