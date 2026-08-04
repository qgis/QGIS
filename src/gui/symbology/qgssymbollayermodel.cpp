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
        default:
          return "Symbol";
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
  else if ( role == Qt::CheckStateRole )
    return QVariant(); // could be true/false
  return QVariant();
}


void QgsSymbolLayerModelNode::deleteChildren()
{
  qDeleteAll( mChildren );
  mChildren.clear();
}

void QgsSymbolLayerModelNode::addChildNode( QgsSymbolLayerModelNode *node )
{
  if ( !node )
    return;

  Q_ASSERT( !node->mParent );
  node->mParent = this;

  mChildren.append( node );
}

void QgsSymbolLayerModelNode::insertChildNode( int index, QgsSymbolLayerModelNode *node )
{
  if ( !node )
    return;

  Q_ASSERT( !node->mParent );
  node->mParent = this;

  mChildren.insert( index, node );
}


void QgsSymbolLayerModelNode::moveChildNode( QgsSymbolLayerModelNode *node, int to )
{
  if ( !node )
    return;

  // Only allow moving a node that is a child of this current node
  Q_ASSERT( node->mParent == this );

  int idx = mChildren.indexOf( node );
  if ( idx != -1 )
  {
    mChildren.move( idx, to );
  }
}


void QgsSymbolLayerModelNode::removeChildNode( QgsSymbolLayerModelNode *node )
{
  if ( !node )
    return;

  int idx = mChildren.indexOf( node );
  if ( idx != -1 )
  {
    delete mChildren.takeAt( idx );
  }
}

int QgsSymbolLayerModelNode::rowCount() const
{
  return children().count();
}

int QgsSymbolLayerModelNode::rowIndex() const
{
  if ( !mParent )
    return -1;

  return mParent->children().indexOf( this );
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

  return n->children().count();
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

  return createIndex( row, column, static_cast<QObject *>( node->children().at( row ) ) );
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

  int row = grandParentNode->children().indexOf( parentNode );
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

  int row = node->parent()->children().indexOf( node );
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


  beginMoveRows( parentIndex, row, row, parentIndex, row + offset + ( offset == 1 ? 1 : 0 ) );
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

  QgsSymbolLayerModelNode *newNode = new QgsSymbolLayerModelNode( newLayer, parentSymbol->type(), mVectorLayer, mScreen );
  if ( insertIdx == -1 )
    parent->addChildNode( newNode );
  else
    parent->insertChildNode( insertIdx, newNode );

  if ( newLayer->subSymbol() )
  {
    loadSymbol( newLayer->subSymbol(), newNode );
  }

  endInsertRows();

  return newNode;
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

  // save data-defined values at marker level
  const QgsProperty ddSize( parentSymbol->type() == Qgis::SymbolType::Marker ? static_cast<QgsMarkerSymbol *>( parentSymbol )->dataDefinedSize() : QgsProperty() );
  const QgsProperty ddAngle( parentSymbol->type() == Qgis::SymbolType::Marker ? static_cast<QgsMarkerSymbol *>( parentSymbol )->dataDefinedAngle() : QgsProperty() );
  const QgsProperty ddWidth( parentSymbol->type() == Qgis::SymbolType::Line ? static_cast<QgsLineSymbol *>( parentSymbol )->dataDefinedWidth() : QgsProperty() );

  QgsSymbolLayer *newLayerPtr = nullptr;
  {
    std::unique_ptr< QgsSymbolLayer > newLayer = QgsSymbolLayerRegistry::defaultSymbolLayer( parentSymbol->type() );
    newLayerPtr = newLayer.get();
    if ( insertIdx == -1 )
      parentSymbol->appendSymbolLayer( newLayer.release() );
    else
      parentSymbol->insertSymbolLayer( node->rowCount() - insertIdx, newLayer.release() );
  }

  // restore data-defined values at marker level
  if ( ddSize )
    static_cast<QgsMarkerSymbol *>( parentSymbol )->setDataDefinedSize( ddSize );
  if ( ddAngle )
    static_cast<QgsMarkerSymbol *>( parentSymbol )->setDataDefinedAngle( ddAngle );
  if ( ddWidth )
    static_cast<QgsLineSymbol *>( parentSymbol )->setDataDefinedWidth( ddWidth );


  const int atRowIndex = ( insertIdx == -1 ) ? 0 : insertIdx;
  beginInsertRows( node2index( node ), atRowIndex, atRowIndex );

  // TODO -- using newLayerPtr is not safe in some circumstances here. This needs reworking so that QgsSymbolLayerModelNode does has
  // its own owned QgsSymbolLayer clone, and isn't reliant on a pointer to the object owned by parentSymbol.
  QgsSymbolLayerModelNode *newNode = new QgsSymbolLayerModelNode( newLayerPtr, parentSymbol->type(), mVectorLayer, mScreen );
  node->insertChildNode( atRowIndex, newNode );

  endInsertRows();

  return newNode;
}

void QgsSymbolLayerModel::removeLayer( QgsSymbolLayerModelNode *node )
{
  if ( !node->isLayer() )
    return;

  const int row = node->rowIndex();
  QgsSymbolLayerModelNode *parent = node->parent();
  const int layerIdx = parent->rowCount() - row - 1; // The index in the model and the symbol are inverted
  QgsSymbol *parentSymbol = parent->symbol();
  QgsSymbolLayer *tmpLayer = parentSymbol->takeSymbolLayer( layerIdx );
  //finally delete the removed layer pointer
  delete tmpLayer;


  beginRemoveRows( node2index( parent ), row, row );
  parent->removeChildNode( node );
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
  rebuild();
}

void QgsSymbolLayerModel::loadSymbol( QgsSymbol *symbol, QgsSymbolLayerModelNode *parent )
{
  if ( !symbol )
    return;

  if ( !parent )
  {
    return;
  }


  QgsSymbolLayerModelNode *symbolNode = new QgsSymbolLayerModelNode( symbol, mVectorLayer, mScreen );
  parent->addChildNode( symbolNode );

  const int count = symbol->symbolLayerCount();
  for ( int i = count - 1; i >= 0; i-- )
  {
    QgsSymbolLayerModelNode *layerNode = new QgsSymbolLayerModelNode( symbol->symbolLayer( i ), symbol->type(), mVectorLayer, mScreen );
    symbolNode->addChildNode( layerNode );
    if ( symbol->symbolLayer( i )->subSymbol() )
    {
      loadSymbol( symbol->symbolLayer( i )->subSymbol(), layerNode );
    }
  }
}
