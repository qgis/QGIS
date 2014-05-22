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

#include <QMimeData>
#include <QTextStream>

#include "qgsdataitem.h"
#include "qgspluginlayer.h"
#include "qgsrasterlayer.h"
#include "qgsrendererv2.h"
#include "qgssymbollayerv2utils.h"
#include "qgsvectorlayer.h"


QgsLayerTreeModel::QgsLayerTreeModel( QgsLayerTreeGroup* rootNode, QObject *parent )
    : QAbstractItemModel( parent )
    , mRootNode( rootNode )
    , mFlags( ShowSymbology )
    , mCurrentNode( 0 )
{
  Q_ASSERT( mRootNode );

  connect( mRootNode, SIGNAL( willAddChildren( QgsLayerTreeNode*, int, int ) ), this, SLOT( nodeWillAddChildren( QgsLayerTreeNode*, int, int ) ) );
  connect( mRootNode, SIGNAL( addedChildren( QgsLayerTreeNode*, int, int ) ), this, SLOT( nodeAddedChildren( QgsLayerTreeNode*, int, int ) ) );
  connect( mRootNode, SIGNAL( willRemoveChildren( QgsLayerTreeNode*, int, int ) ), this, SLOT( nodeWillRemoveChildren( QgsLayerTreeNode*, int, int ) ) );
  connect( mRootNode, SIGNAL( removedChildren( QgsLayerTreeNode*, int, int ) ), this, SLOT( nodeRemovedChildren() ) );
  connect( mRootNode, SIGNAL( visibilityChanged( QgsLayerTreeNode*, Qt::CheckState ) ), this, SLOT( nodeVisibilityChanged( QgsLayerTreeNode* ) ) );
}

QgsLayerTreeModel::~QgsLayerTreeModel()
{
  foreach ( QList<QgsLayerTreeModelSymbologyNode*> nodeL, mSymbologyNodes )
    qDeleteAll( nodeL );
  mSymbologyNodes.clear();
}

QgsLayerTreeNode* QgsLayerTreeModel::index2node( const QModelIndex& index ) const
{
  if ( !index.isValid() )
    return mRootNode;

  QObject* obj = reinterpret_cast<QObject*>( index.internalPointer() );
  return qobject_cast<QgsLayerTreeNode*>( obj );
}

QgsLayerTreeModelSymbologyNode* QgsLayerTreeModel::index2symnode( const QModelIndex& index )
{
  return qobject_cast<QgsLayerTreeModelSymbologyNode*>( reinterpret_cast<QObject*>( index.internalPointer() ) );
}


int QgsLayerTreeModel::rowCount( const QModelIndex &parent ) const
{
  if ( index2symnode( parent ) )
  {
    return 0; // they are leaves
  }

  QgsLayerTreeNode* n = index2node( parent );

  if ( parent.isValid() && parent.column() != 0 )
    return 0;

  if ( QgsLayerTree::isLayer( n ) )
  {
    QgsLayerTreeLayer* nL = QgsLayerTree::toLayer( n );

    return mSymbologyNodes[nL].count();
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
    QgsLayerTreeModelSymbologyNode* sym = index2symnode( parent );
    Q_ASSERT( sym );
    return QModelIndex(); // have no children
  }

  if ( !n || column != 0 || row >= rowCount( parent ) )
    return QModelIndex();

  if ( testFlag( ShowSymbology ) && QgsLayerTree::isLayer( n ) )
  {
    QgsLayerTreeLayer* nL = QgsLayerTree::toLayer( n );
    Q_ASSERT( mSymbologyNodes.contains( nL ) );
    return createIndex( row, column, static_cast<QObject*>( mSymbologyNodes[nL].at( row ) ) );
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
    QgsLayerTreeModelSymbologyNode* sym = index2symnode( child );
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

  if ( QgsLayerTreeModelSymbologyNode* sym = index2symnode( index ) )
  {
    if ( role == Qt::DisplayRole )
      return sym->name();
    else if ( role == Qt::DecorationRole )
      return sym->icon();
    return QVariant();
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
      return QgsDataCollectionItem::iconDir();
    else if ( QgsLayerTree::isLayer( node ) )
    {
      QgsMapLayer* layer = QgsLayerTree::toLayer( node )->layer();
      if ( !layer )
        return QVariant();
      if ( layer->type() == QgsMapLayer::RasterLayer )
        return QgsLayerItem::iconRaster();
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
    QFont f;
    if ( node->customProperty( "embedded" ).toInt() )
      f.setItalic( true );
    if ( QgsLayerTree::isLayer( node ) )
      f.setBold( true );
    if ( node == mCurrentNode )
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

  if ( index2symnode( index ) )
    return Qt::ItemIsEnabled; // | Qt::ItemIsSelectable;

  Qt::ItemFlags f = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  if ( testFlag( AllowNodeRename ) )
    f |= Qt::ItemIsEditable;
  if ( testFlag( AllowNodeReorder ) )
    f |= Qt::ItemIsDragEnabled;
  QgsLayerTreeNode* node = index2node( index );
  if ( testFlag( AllowNodeChangeVisibility ) && ( QgsLayerTree::isLayer( node ) || QgsLayerTree::isGroup( node ) ) )
    f |= Qt::ItemIsUserCheckable;
  if ( testFlag( AllowNodeReorder ) && QgsLayerTree::isGroup( node ) )
    f |= Qt::ItemIsDropEnabled;
  return f;
}

bool QgsLayerTreeModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
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
    }
    else if ( QgsLayerTree::isGroup( node ) )
    {
      QgsLayerTreeGroup* group = QgsLayerTree::toGroup( node );
      group->setVisible(( Qt::CheckState )value.toInt() );
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

void QgsLayerTreeModel::refreshLayerSymbology( QgsLayerTreeLayer* nodeLayer )
{
  // update title
  QModelIndex idx = node2index( nodeLayer );
  emit dataChanged( idx, idx );

  // update children
  beginRemoveRows( idx, 0, rowCount( idx ) - 1 );
  removeSymbologyFromLayer( nodeLayer );
  endRemoveRows();

  addSymbologyToLayer( nodeLayer );
}

void QgsLayerTreeModel::setCurrentNode( QgsLayerTreeNode* currentNode )
{
  if ( mCurrentNode )
  {
    QModelIndex idx = node2index( mCurrentNode );
    emit dataChanged( idx, idx );
  }

  mCurrentNode = currentNode;

  if ( mCurrentNode )
  {
    QModelIndex idx = node2index( mCurrentNode );
    emit dataChanged( idx, idx );
  }
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

  // disconnect from layers and remove their symbology
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

void QgsLayerTreeModel::nodeLayerLoaded()
{
  QgsLayerTreeLayer* nodeLayer = qobject_cast<QgsLayerTreeLayer*>( sender() );
  if ( !nodeLayer )
    return;

  // deffered connection to the layer
  connectToLayer( nodeLayer );
}

void QgsLayerTreeModel::layerRendererChanged()
{
  QgsMapLayer* layer = qobject_cast<QgsMapLayer*>( sender() );
  if ( !layer )
    return;

  QgsLayerTreeLayer* nodeLayer = mRootNode->findLayer( layer->id() );
  if ( !nodeLayer )
    return;

  refreshLayerSymbology( nodeLayer );
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
}


void QgsLayerTreeModel::removeSymbologyFromLayer( QgsLayerTreeLayer* nodeLayer )
{
  if ( mSymbologyNodes.contains( nodeLayer ) )
  {
    qDeleteAll( mSymbologyNodes[nodeLayer] );
    mSymbologyNodes.remove( nodeLayer );

    disconnect( nodeLayer->layer(), SIGNAL( rendererChanged() ), this, SLOT( layerRendererChanged() ) );
  }
}


void QgsLayerTreeModel::addSymbologyToLayer( QgsLayerTreeLayer* nodeL )
{
  if ( !nodeL->layer() )
    return;

  if ( nodeL->layer()->type() == QgsMapLayer::VectorLayer )
  {
    addSymbologyToVectorLayer( nodeL );
  }
  else if ( nodeL->layer()->type() == QgsMapLayer::RasterLayer )
  {
    addSymbologyToRasterLayer( nodeL );
  }
  else if ( nodeL->layer()->type() == QgsMapLayer::PluginLayer )
  {
    addSymbologyToPluginLayer( nodeL );
  }

  // be ready for any subsequent changes of the renderer
  connect( nodeL->layer(), SIGNAL( rendererChanged() ), this, SLOT( layerRendererChanged() ) );
}


void QgsLayerTreeModel::addSymbologyToVectorLayer( QgsLayerTreeLayer* nodeL )
{
  QgsVectorLayer* vlayer = static_cast<QgsVectorLayer*>( nodeL->layer() );
  QgsFeatureRendererV2* r = vlayer->rendererV2();
  if ( !r )
    return;

  QList<QgsLayerTreeModelSymbologyNode*>& lst = mSymbologyNodes[nodeL];

  bool showFeatureCount = nodeL->customProperty( "showFeatureCount", 0 ).toBool();
  if ( showFeatureCount )
  {
    vlayer->countSymbolFeatures();
  }
  QSize iconSize( 16, 16 );
  QgsLegendSymbolList items = r->legendSymbolItems();

  beginInsertRows( node2index( nodeL ), 0, items.count() - 1 );

  typedef QPair<QString, QgsSymbolV2*> XY;
  foreach ( XY item, items )
  {
    QString label = item.first;
    QIcon icon;
    if ( item.second )
      icon = QgsSymbolLayerV2Utils::symbolPreviewPixmap( item.second, iconSize );
    if ( showFeatureCount && item.second )
      label += QString( " [%1]" ).arg( vlayer->featureCount( item.second ) );
    lst << new QgsLayerTreeModelSymbologyNode( nodeL, label, icon );
  }

  endInsertRows();
}


void QgsLayerTreeModel::addSymbologyToRasterLayer( QgsLayerTreeLayer* nodeL )
{
  QgsRasterLayer* rlayer = static_cast<QgsRasterLayer*>( nodeL->layer() );

  QgsLegendColorList rasterItemList = rlayer->legendSymbologyItems();

  QList<QgsLayerTreeModelSymbologyNode*>& lst = mSymbologyNodes[nodeL];

  // temporary solution for WMS. Ideally should be done with a delegate.
  if ( rlayer->providerType() == "wms" )
  {
    QImage img = rlayer->dataProvider()->getLegendGraphic( 1000 ); // dummy scale - should not be required!
    if ( !img.isNull() )
      lst << new QgsLayerTreeModelSymbologyNode( nodeL, tr( "Double-click to view legend" ) );
  }

  // Paletted raster may have many colors, for example UInt16 may have 65536 colors
  // and it is very slow, so we limit max count
  QSize iconSize( 16, 16 );
  int count = 0;
  int max_count = 1000;
  int total_count = qMin( max_count + 1, rasterItemList.count() );

  beginInsertRows( node2index( nodeL ), 0, total_count - 1 );

  for ( QgsLegendColorList::const_iterator itemIt = rasterItemList.constBegin();
        itemIt != rasterItemList.constEnd(); ++itemIt, ++count )
  {
    QPixmap pix( iconSize );
    pix.fill( itemIt->second );
    lst << new QgsLayerTreeModelSymbologyNode( nodeL, itemIt->first, QIcon( pix ) );

    if ( count == max_count )
    {
      pix.fill( Qt::transparent );
      QString label = tr( "following %1 items\nnot displayed" ).arg( rasterItemList.size() - max_count );
      lst << new QgsLayerTreeModelSymbologyNode( nodeL, label, QIcon( pix ) );
      break;
    }
  }

  endInsertRows();
}


void QgsLayerTreeModel::addSymbologyToPluginLayer( QgsLayerTreeLayer* nodeL )
{
  QgsPluginLayer* player = static_cast<QgsPluginLayer*>( nodeL->layer() );

  QList<QgsLayerTreeModelSymbologyNode*>& lst = mSymbologyNodes[nodeL];

  QSize iconSize( 16, 16 );
  QgsLegendSymbologyList symbologyList = player->legendSymbologyItems( iconSize );

  beginInsertRows( node2index( nodeL ), 0, symbologyList.count() - 1 );

  typedef QPair<QString, QPixmap> XY;
  foreach ( XY item, symbologyList )
  {
    lst << new QgsLayerTreeModelSymbologyNode( nodeL, item.first, QIcon( item.second ) );
  }

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

  if ( testFlag( ShowSymbology ) )
  {
    addSymbologyToLayer( nodeLayer );
  }

  QgsMapLayer* layer = nodeLayer->layer();
  if ( layer->type() == QgsMapLayer::VectorLayer )
  {
    // using unique connection because there may be temporarily more nodes for a layer than just one
    // which would create multiple connections, however disconnect() would disconnect all multiple connections
    // even if we wanted to disconnect just one connection in each call.
    connect( layer, SIGNAL( editingStarted() ), this, SLOT( layerNeedsUpdate() ), Qt::UniqueConnection );
    connect( layer, SIGNAL( editingStopped() ), this, SLOT( layerNeedsUpdate() ), Qt::UniqueConnection );
    connect( layer, SIGNAL( layerModified() ), this, SLOT( layerNeedsUpdate() ), Qt::UniqueConnection );
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
  if ( !nodeLayer->layer() )
    return; // we were never connected

  if ( testFlag( ShowSymbology ) )
  {
    removeSymbologyFromLayer( nodeLayer );
  }

  if ( _numLayerCount( mRootNode, nodeLayer->layerId() ) == 1 )
  {
    // last instance of the layer in the tree: disconnect from all signals from layer!
    disconnect( nodeLayer->layer(), 0, this, 0 );
  }
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
