/***************************************************************************
    qgssymbollayermodel.h
    ---------------------
    begin                : June 2026
    copyright            : (C) 2026 by Valentin Buira
    email                : valentin dot buira at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssymbollayermodel.h"

#include "qgsapplication.h"
#include "qgsexpressioncontextutils.h"
#include "qgsguiutils.h"
#include "qgslinesymbol.h"
#include "qgsmarkersymbol.h"
#include "qgsstyle.h"
#include "qgssymbol.h"
#include "qgssymbollayer.h"
#include "qgssymbollayerregistry.h"
#include "qgssymbollayerutils.h"
#include "qgsvectorlayer.h"

#include <qscreen.h>

#include "moc_qgssymbollayermodel.cpp"

using namespace Qt::StringLiterals;

QgsSymbolLayerModelNode::QgsSymbolLayerModelNode()
{}

QgsSymbolLayerModelNode::QgsSymbolLayerModelNode( QgsSymbolLayer *layer, Qgis::SymbolType symbolType, QgsVectorLayer *vectorLayer, QScreen *screen )
  : mVectorLayer( vectorLayer )
  , mScreen( screen )
{
  setLayer( layer, symbolType );
}

QgsSymbolLayerModelNode::QgsSymbolLayerModelNode( QgsSymbol *symbol, QgsVectorLayer *vectorLayer, QScreen *screen )
  : mVectorLayer( vectorLayer )
  , mScreen( screen )
{
  setSymbol( symbol );
}

QgsSymbolLayerModelNode::~QgsSymbolLayerModelNode()
{
  deleteChildren();
}

void QgsSymbolLayerModelNode::setLayer( QgsSymbolLayer *layer, Qgis::SymbolType symbolType )
{
  mLayer = layer;
  mIsLayer = true;
  mSymbol = nullptr;
  mSymbolType = symbolType;
}

void QgsSymbolLayerModelNode::setScreen( QScreen *screen )
{
  mScreen = screen;
}

void QgsSymbolLayerModelNode::setSymbol( QgsSymbol *symbol )
{
  mSymbol = symbol;
  mIsLayer = false;
  mLayer = nullptr;
}

QIcon QgsSymbolLayerModelNode::icon() const
{
  const int size = QgsGuiUtils::scaleIconSize( 16 );
  const QSize iconSize = QSize( size, size );

  QIcon icon;
  if ( mIsLayer )
    icon = QgsSymbolLayerUtils::
      symbolLayerPreviewIcon( mLayer, Qgis::RenderUnit::Millimeters, iconSize, QgsMapUnitScale(), mSymbol ? mSymbol->type() : mSymbolType, mVectorLayer, QgsScreenProperties( mScreen.data() ) );
  else
  {
    QgsExpressionContext expContext;
    // TODO -- this model should have a way to register an explicit expression context, so that the preview icons
    // correctly reflect atlas features/map scales/etc
    expContext.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( mVectorLayer ) );
    icon = QIcon( QgsSymbolLayerUtils::symbolPreviewPixmap( mSymbol, iconSize, 0, nullptr, false, &expContext, nullptr, QgsScreenProperties( mScreen.data() ) ) );
  }
  return icon;
}

QVariant QgsSymbolLayerModelNode::data( int role ) const
{
  if ( role == Qt::DisplayRole || role == Qt::EditRole )
  {
    if ( mIsLayer )
    {
      QgsSymbolLayerAbstractMetadata *m = QgsApplication::symbolLayerRegistry()->symbolLayerMetadata( mLayer->layerType() );
      if ( m )
        return m->visibleName();
      else
        return QString();
    }
    else
    {
      switch ( mSymbol->type() )
      {
        case Qgis::SymbolType::Marker:
          return QCoreApplication::translate( "QgsSymbolLayerModelNode", "Marker" );
        case Qgis::SymbolType::Fill:
          return QCoreApplication::translate( "QgsSymbolLayerModelNode", "Fill" );
        case Qgis::SymbolType::Line:
          return QCoreApplication::translate( "QgsSymbolLayerModelNode", "Line" );
        case Qgis::SymbolType::Hybrid:
          return "Symbol";
        default:
          QgsDebugError( "Unhandled Symbol Type" );
      }
    }
  }
  else if ( role == Qt::ForegroundRole && mIsLayer )
  {
    if ( !mLayer->enabled() )
    {
      QPalette pal = qApp->palette();
      QBrush brush = QBrush();
      brush.setColor( pal.color( QPalette::Disabled, QPalette::WindowText ) );
      return brush;
    }
    else
    {
      return QVariant();
    }
  }
  else if ( role == Qt::DecorationRole )
  {
    return icon();
  }
  else if ( role == Qt::FontRole && !mIsLayer )
  {
    QFont font;
    font.setBold( true );
    return font;
  }
  return QVariant();
}

void QgsSymbolLayerModelNode::deleteChildren()
{
  mChildren.clear();
}

QgsSymbolLayerModelNode *QgsSymbolLayerModelNode::addChildNode( std::unique_ptr<QgsSymbolLayerModelNode> node )
{
  if ( !node )
    return nullptr;

  Q_ASSERT( !node->mParent );
  node->mParent = this;

  return mChildren.emplace_back( std::move( node ) ).get();
}

QgsSymbolLayerModelNode *QgsSymbolLayerModelNode::insertChildNode( int index, std::unique_ptr<QgsSymbolLayerModelNode> node )
{
  if ( !node )
    return nullptr;

  Q_ASSERT( !node->mParent );
  node->mParent = this;

  return mChildren.insert( mChildren.begin() + index, std::move( node ) )->get();
}

void QgsSymbolLayerModelNode::moveChildNode( QgsSymbolLayerModelNode *node, int to )
{
  if ( !node )
    return;

  // Only allow moving a node that is a child of this current node
  Q_ASSERT( node->mParent == this );

  int idx = indexOf( node );
  if ( idx != -1 && idx != to )
  {
    std::unique_ptr<QgsSymbolLayerModelNode> n = std::move( mChildren.at( idx ) );
    mChildren.erase( mChildren.begin() + idx );
    mChildren.insert( mChildren.begin() + to, std::move( n ) );
  }
}

void QgsSymbolLayerModelNode::removeChildNode( QgsSymbolLayerModelNode *node )
{
  if ( !node )
    return;

  int idx = indexOf( node );
  if ( idx != -1 )
  {
    mChildren.erase( mChildren.begin() + idx );
  }
}

QgsSymbolLayerModelNode *QgsSymbolLayerModelNode::childAt( int index ) const
{
  return mChildren.at( index ).get();
}

int QgsSymbolLayerModelNode::indexOf( const QgsSymbolLayerModelNode *node )
{
  Q_ASSERT( node->mParent == this );

  auto it = std::find_if( mChildren.begin(), mChildren.end(), [&]( const std::unique_ptr<QgsSymbolLayerModelNode> &p ) { return p.get() == node; } );

  if ( it != mChildren.end() )
    return std::distance( mChildren.begin(), it );
  return -1;
}

int QgsSymbolLayerModelNode::rowCount() const
{
  return mChildren.size();
}

int QgsSymbolLayerModelNode::rowIndex() const
{
  if ( !mParent )
    return -1;

  return mParent->indexOf( this );
}

QgsSymbolLayerModel::QgsSymbolLayerModel( QgsVectorLayer *vl, QObject *parent, QScreen *screen )
  : QAbstractItemModel( parent )
  , mRootNode( std::make_unique<QgsSymbolLayerModelNode>() )
  , mVectorLayer( vl )
  , mScreen( screen )
{}


QVariant QgsSymbolLayerModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  QgsSymbolLayerModelNode *node = index2node( index );
  if ( !node )
    return QVariant();

  return node->data( role );
};

int QgsSymbolLayerModel::rowCount( const QModelIndex &parent ) const
{
  QgsSymbolLayerModelNode *n = index2node( parent );
  if ( !n )
    return 0;

  return n->rowCount();
};

int QgsSymbolLayerModel::columnCount( const QModelIndex & ) const
{
  return 1;
};

QModelIndex QgsSymbolLayerModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( !hasIndex( row, column, parent ) )
    return QModelIndex();

  QgsSymbolLayerModelNode *node = index2node( parent );
  if ( !node )
    return QModelIndex(); // have no children

  return createIndex( row, column, node->childAt( row ) );
};

QModelIndex QgsSymbolLayerModel::parent( const QModelIndex &child ) const
{
  if ( !child.isValid() )
    return QModelIndex();

  if ( QgsSymbolLayerModelNode *node = index2node( child ) )
  {
    return indexOfParentTreeNode( node->parent() ); // must not be null
  }
  else
  {
    Q_ASSERT( false ); // no other node types!
    return QModelIndex();
  }
};

QModelIndex QgsSymbolLayerModel::indexOfParentTreeNode( QgsSymbolLayerModelNode *parentNode ) const
{
  Q_ASSERT( parentNode );

  QgsSymbolLayerModelNode *grandParentNode = parentNode->parent();
  if ( !grandParentNode )
    return QModelIndex(); // root node -> invalid index

  int row = grandParentNode->indexOf( parentNode );
  Q_ASSERT( row >= 0 );

  return createIndex( row, 0, parentNode );
};

QgsSymbolLayerModelNode *QgsSymbolLayerModel::index2node( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return mRootNode.get();

  return reinterpret_cast<QgsSymbolLayerModelNode *>( index.internalPointer() );
};

QModelIndex QgsSymbolLayerModel::node2index( QgsSymbolLayerModelNode *node ) const
{
  if ( !node || !node->parent() )
    return QModelIndex(); // this is the only root item -> invalid index

  QModelIndex parentIndex = node2index( node->parent() );

  int row = node->parent()->indexOf( node );
  Q_ASSERT( row >= 0 );
  return index( row, 0, parentIndex );
};

void QgsSymbolLayerModel::rebuild()
{
  beginResetModel();
  mRootNode->deleteChildren();
  loadSymbol( mSymbol, mRootNode.get() );
  endResetModel();
}

void QgsSymbolLayerModel::updateNode( QgsSymbol *symbol, QgsSymbolLayerModelNode *parent )
{
  const QModelIndex parentIndex = node2index( parent );
  beginRemoveRows( parentIndex, 0, rowCount( parentIndex ) - 1 );
  parent->deleteChildren();
  endRemoveRows();

  beginInsertRows( parentIndex, 0, symbol->symbolLayerCount() - 1 );

  loadSymbol( symbol, parent );

  endInsertRows();
}

void QgsSymbolLayerModel::moveLayerByOffset( QgsSymbolLayerModelNode *node, int offset )
{
  if ( !node->isLayer() )
    return;

  QgsSymbolLayerModelNode *parent = node->parent();
  QModelIndex parentIndex = node2index( parent );
  const int row = node->rowIndex();

  QgsSymbol *parentSymbol = parent->symbol();

  const int layerIdx = parent->rowCount() - row - 1;
  // switch layers
  QgsSymbolLayer *tmpLayer = parentSymbol->takeSymbolLayer( layerIdx );
  parentSymbol->insertSymbolLayer( layerIdx - offset, tmpLayer );

  beginMoveRows( parentIndex, row, row, parentIndex, row + offset + ( offset > 0 ? 1 : 0 ) );
  parent->moveChildNode( node, row + offset );
  endMoveRows();
}

QgsSymbolLayerModelNode *QgsSymbolLayerModel::duplicateLayer( QgsSymbolLayerModelNode *node )
{
  if ( !node->isLayer() )
    return nullptr;

  QgsSymbolLayer *source = node->layer();
  const int insertIdx = node->rowIndex();
  QgsSymbolLayerModelNode *parent = node->parent();
  QModelIndex parentIndex = node2index( parent );

  QgsSymbol *parentSymbol = parent->symbol();

  QgsSymbolLayer *newLayer = source->clone();
  QgsSymbolLayerUtils::resetSymbolLayerIds( newLayer );
  if ( insertIdx == -1 )
    parentSymbol->appendSymbolLayer( newLayer );
  else
    parentSymbol->insertSymbolLayer( parent->rowCount() - insertIdx, newLayer );


  beginInsertRows( parentIndex, insertIdx, insertIdx );

  auto newNode = std::make_unique<QgsSymbolLayerModelNode>( newLayer, parentSymbol->type(), mVectorLayer, mScreen );
  QgsSymbolLayerModelNode *newNodePtr;
  if ( insertIdx == -1 )
    newNodePtr = parent->addChildNode( std::move( newNode ) );
  else
    newNodePtr = parent->insertChildNode( insertIdx, std::move( newNode ) );

  if ( newLayer->subSymbol() )
  {
    loadSymbol( newLayer->subSymbol(), newNodePtr );
  }

  endInsertRows();

  return newNodePtr;
}

QgsSymbolLayerModelNode *QgsSymbolLayerModel::addLayer( QModelIndex index )
{
  if ( !index.isValid() )
    return nullptr;

  int insertIdx = -1;
  QgsSymbolLayerModelNode *node = index2node( index );
  if ( node->isLayer() )
  {
    insertIdx = node->rowIndex();
    node = node->parent();
  }


  QgsSymbol *parentSymbol = node->symbol();

  // save data-defined values from symbol, to apply to the new symbol layer
  const QgsProperty ddSize( parentSymbol->type() == Qgis::SymbolType::Marker ? static_cast<QgsMarkerSymbol *>( parentSymbol )->dataDefinedSize() : QgsProperty() );
  const QgsProperty ddAngle( parentSymbol->type() == Qgis::SymbolType::Marker ? static_cast<QgsMarkerSymbol *>( parentSymbol )->dataDefinedAngle() : QgsProperty() );
  const QgsProperty ddWidth( parentSymbol->type() == Qgis::SymbolType::Line ? static_cast<QgsLineSymbol *>( parentSymbol )->dataDefinedWidth() : QgsProperty() );

  QgsSymbolLayer *newLayer = QgsSymbolLayerRegistry::defaultSymbolLayer( parentSymbol->type() ).release();
  {
    // Transfer the ownership to the parent symbol, which takes ownership of the layer
    if ( insertIdx == -1 )
      parentSymbol->appendSymbolLayer( newLayer );
    else
      parentSymbol->insertSymbolLayer( node->rowCount() - insertIdx, newLayer );
  }

  // restore data-defined values from the symbol
  if ( ddSize )
    static_cast<QgsMarkerSymbol *>( parentSymbol )->setDataDefinedSize( ddSize );
  if ( ddAngle )
    static_cast<QgsMarkerSymbol *>( parentSymbol )->setDataDefinedAngle( ddAngle );
  if ( ddWidth )
    static_cast<QgsLineSymbol *>( parentSymbol )->setDataDefinedWidth( ddWidth );


  const int atRowIndex = ( insertIdx == -1 ) ? 0 : insertIdx;
  beginInsertRows( node2index( node ), atRowIndex, atRowIndex );

  auto newNode = std::make_unique<QgsSymbolLayerModelNode>( newLayer, parentSymbol->type(), mVectorLayer, mScreen );

  QgsSymbolLayerModelNode *newNodePtr = node->insertChildNode( atRowIndex, std::move( newNode ) );

  endInsertRows();

  return newNodePtr;
}

void QgsSymbolLayerModel::removeLayer( QgsSymbolLayerModelNode *node )
{
  if ( !node->isLayer() )
    return;

  const int row = node->rowIndex();
  QgsSymbolLayerModelNode *parent = node->parent();
  const int layerIdx = parent->rowCount() - row - 1; // The index in the model and the symbol are inverted
  QgsSymbol *parentSymbol = parent->symbol();

  beginRemoveRows( node2index( parent ), row, row );
  parent->removeChildNode( node );

  QgsSymbolLayer *tmpLayer = parentSymbol->takeSymbolLayer( layerIdx );
  //finally delete the removed layer pointer
  delete tmpLayer;
  endRemoveRows();
}

void QgsSymbolLayerModel::changeLayer( QgsSymbolLayerModelNode *node, QgsSymbolLayer *newLayer )
{
  QgsSymbolLayerModelNode *parentNode = node->parent();
  QgsSymbol *parentSymbol = parentNode->symbol();

  const int layerIdx = parentNode->rowCount() - node->rowIndex() - 1;
  parentSymbol->changeSymbolLayer( layerIdx, newLayer );

  if ( node->rowCount() > 0 )
  {
    beginRemoveRows( node2index( node ), 0, node->rowCount() - 1 );
    node->deleteChildren();
    endRemoveRows();
  }

  node->setLayer( newLayer, parentSymbol->type() );
  if ( newLayer->subSymbol() )
  {
    updateNode( newLayer->subSymbol(), node );
  }

  updatePreview( node );
}

void QgsSymbolLayerModel::updatePreview( QgsSymbolLayerModelNode *node )
{
  const QModelIndex index = node2index( node );
  emit dataChanged( index, index, QVector<int>() << Qt::DecorationRole );

  // Recursively update the parent's preview up to the root node.
  if ( QgsSymbolLayerModelNode *lParent = node->parent() )
    updatePreview( lParent );
}


void QgsSymbolLayerModel::setSymbol( QgsSymbol *symbol )
{
  if ( symbol )
  {
    mSymbol = symbol;
  }

  rebuild();
}

void QgsSymbolLayerModel::setScreen( QScreen *screen )
{
  mScreen = screen;

  // BFS traversal to set the screen for all nodes and update their preview icons
  QList<QgsSymbolLayerModelNode * > stack;
  stack.append( mRootNode.get() );

  while ( !stack.isEmpty() )
  {
    QgsSymbolLayerModelNode *node = stack.takeLast();
    node->setScreen( mScreen );
    updatePreview( node );

    for ( int i = 0; i < node->rowCount(); ++i )
    {
      stack.append( node->childAt( i ) );
    }
  }
}

void QgsSymbolLayerModel::loadSymbol( QgsSymbol *symbol, QgsSymbolLayerModelNode *parent )
{
  if ( !symbol )
    return;

  if ( !parent )
  {
    return;
  }

  auto symbolNode = std::make_unique<QgsSymbolLayerModelNode>( symbol, mVectorLayer, mScreen );
  const int count = symbol->symbolLayerCount();
  for ( int i = count - 1; i >= 0; i-- )
  {
    auto layerNode = std::make_unique<QgsSymbolLayerModelNode>( symbol->symbolLayer( i ), symbol->type(), mVectorLayer, mScreen );
    if ( symbol->symbolLayer( i )->subSymbol() )
    {
      loadSymbol( symbol->symbolLayer( i )->subSymbol(), layerNode.get() );
    }
    symbolNode->addChildNode( std::move( layerNode ) );
  }
  parent->addChildNode( std::move( symbolNode ) );
}
