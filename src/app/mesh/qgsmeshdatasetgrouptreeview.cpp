/***************************************************************************
    qgsmeshdatasetgrouptreeview.cpp
    -------------------------------
    begin                : June 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmeshdatasetgrouptreeview.h"

#include "qgis.h"
#include "qgsmeshlayer.h"

#include <QList>
#include <QItemSelectionModel>
#include <QMouseEvent>

QgsMeshDatasetGroupTreeItem::QgsMeshDatasetGroupTreeItem( QgsMeshDatasetGroupTreeItem *parent )
  : mParent( parent )
{
}

QgsMeshDatasetGroupTreeItem::QgsMeshDatasetGroupTreeItem( const QString &name,
    bool isVector,
    int index,
    QgsMeshDatasetGroupTreeItem *parent )
  : mParent( parent )
  , mName( name )
  , mIsVector( isVector )
  , mDatasetGroupIndex( index )
{
}

QgsMeshDatasetGroupTreeItem::~QgsMeshDatasetGroupTreeItem()
{
  qDeleteAll( mChildren );
}

void QgsMeshDatasetGroupTreeItem::appendChild( QgsMeshDatasetGroupTreeItem *node )
{
  mChildren.append( node );
}

QgsMeshDatasetGroupTreeItem *QgsMeshDatasetGroupTreeItem::child( int row ) const
{
  if ( row < mChildren.count() )
    return mChildren.at( row );
  else
    return nullptr;
}

int QgsMeshDatasetGroupTreeItem::childCount() const
{
  return mChildren.count();
}

QgsMeshDatasetGroupTreeItem *QgsMeshDatasetGroupTreeItem::parentItem() const
{
  return mParent;
}

int QgsMeshDatasetGroupTreeItem::row() const
{
  if ( mParent )
    return mParent->mChildren.indexOf( const_cast<QgsMeshDatasetGroupTreeItem *>( this ) );

  return 0;
}

QString QgsMeshDatasetGroupTreeItem::name() const
{
  return mName;
}

bool QgsMeshDatasetGroupTreeItem::isVector() const
{
  return mIsVector;
}

int QgsMeshDatasetGroupTreeItem::datasetGroupIndex() const
{
  return mDatasetGroupIndex;
}


/////////////////////////////////////////////////////////////////////////////////////////

QgsMeshDatasetGroupTreeModel::QgsMeshDatasetGroupTreeModel( QObject *parent )
  : QAbstractItemModel( parent )
  ,  mRootItem( new QgsMeshDatasetGroupTreeItem() )
{
}

int QgsMeshDatasetGroupTreeModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return 1;
}

int QgsMeshDatasetGroupTreeModel::activeScalarGroup() const
{
  return mActiveScalarGroupIndex;
}

void QgsMeshDatasetGroupTreeModel::setActiveScalarGroup( int group )
{
  if ( mActiveScalarGroupIndex == group )
    return;

  int oldGroupIndex = mActiveScalarGroupIndex;
  mActiveScalarGroupIndex = group;

  if ( oldGroupIndex > -1 )
  {
    const auto index = groupIndexToModelIndex( oldGroupIndex );
    emit dataChanged( index, index );
  }

  if ( group > -1 )
  {
    const auto index = groupIndexToModelIndex( group );
    emit dataChanged( index, index );
  }
}

int QgsMeshDatasetGroupTreeModel::activeVectorGroup() const
{
  return mActiveVectorGroupIndex;
}

void QgsMeshDatasetGroupTreeModel::setActiveVectorGroup( int group )
{
  if ( mActiveVectorGroupIndex == group )
    return;

  int oldGroupIndex = mActiveVectorGroupIndex;
  mActiveVectorGroupIndex = group;

  if ( oldGroupIndex > -1 )
  {
    const auto index = groupIndexToModelIndex( oldGroupIndex );
    emit dataChanged( index, index );
  }

  if ( group > -1 )
  {
    const auto index = groupIndexToModelIndex( group );
    emit dataChanged( index, index );
  }
}

QVariant QgsMeshDatasetGroupTreeModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  QgsMeshDatasetGroupTreeItem *item = static_cast<QgsMeshDatasetGroupTreeItem *>( index.internalPointer() );

  switch ( role )
  {
    case Qt::DisplayRole:
    case Name:
      return item->name();
    case IsVector:
      return item->isVector();
    case IsActiveScalarDatasetGroup:
      return item->datasetGroupIndex() == mActiveScalarGroupIndex;
    case IsActiveVectorDatasetGroup:
      return item->datasetGroupIndex() == mActiveVectorGroupIndex;
    case DatasetGroupIndex:
      return item->datasetGroupIndex();
  }

  return QVariant();
}

Qt::ItemFlags QgsMeshDatasetGroupTreeModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return Qt::NoItemFlags;

  return Qt::ItemIsEnabled;
}

QVariant QgsMeshDatasetGroupTreeModel::headerData( int section,
    Qt::Orientation orientation,
    int role ) const
{
  Q_UNUSED( section )

  if ( orientation == Qt::Horizontal && role == Qt::DisplayRole )
    return tr( "Groups" );

  return QVariant();
}

QModelIndex QgsMeshDatasetGroupTreeModel::index( int row, int column, const QModelIndex &parent )
const
{
  if ( !hasIndex( row, column, parent ) )
    return QModelIndex();

  QgsMeshDatasetGroupTreeItem *parentItem;

  if ( !parent.isValid() )
    parentItem = mRootItem.get();
  else
    parentItem = static_cast<QgsMeshDatasetGroupTreeItem *>( parent.internalPointer() );

  QgsMeshDatasetGroupTreeItem *childItem = parentItem->child( row );
  if ( childItem )
    return createIndex( row, column, childItem );
  else
    return QModelIndex();
}

QModelIndex QgsMeshDatasetGroupTreeModel::parent( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return QModelIndex();

  QgsMeshDatasetGroupTreeItem *childItem = static_cast<QgsMeshDatasetGroupTreeItem *>( index.internalPointer() );
  QgsMeshDatasetGroupTreeItem *parentItem = childItem->parentItem();

  if ( parentItem == mRootItem.get() )
    return QModelIndex();

  return createIndex( parentItem->row(), 0, parentItem );
}

int QgsMeshDatasetGroupTreeModel::rowCount( const QModelIndex &parent ) const
{
  QgsMeshDatasetGroupTreeItem *parentItem;
  if ( parent.column() > 0 )
    return 0;

  if ( !parent.isValid() )
    parentItem = mRootItem.get();
  else
    parentItem = static_cast<QgsMeshDatasetGroupTreeItem *>( parent.internalPointer() );

  return parentItem->childCount();
}

void QgsMeshDatasetGroupTreeModel::syncToLayer( QgsMeshLayer *layer )
{
  beginResetModel();

  mRootItem.reset( new QgsMeshDatasetGroupTreeItem() );
  mNameToItem.clear();
  mDatasetGroupIndexToItem.clear();

  if ( layer && layer->dataProvider() )
  {
    const QgsMeshDataProvider *dp = layer->dataProvider();
    for ( int groupIndex = 0; groupIndex < dp->datasetGroupCount(); ++groupIndex )
    {
      const QgsMeshDatasetGroupMetadata meta = dp->datasetGroupMetadata( groupIndex );
      const QString name = meta.name();
      const bool isVector = meta.isVector();
      const QStringList subdatasets = name.split( '/' );


      if ( subdatasets.size() == 1 )
      {
        addTreeItem( name, isVector, groupIndex, mRootItem.get() );
      }
      else if ( subdatasets.size() == 2 )
      {
        auto i = mNameToItem.find( subdatasets[0] );
        if ( i == mNameToItem.end() )
        {
          QgsDebugMsg( QStringLiteral( "Unable to find parent group for %1." ).arg( name ) );
          addTreeItem( name, isVector, groupIndex, mRootItem.get() );
        }
        else
        {
          addTreeItem( subdatasets[1], isVector, groupIndex, i.value() );
        }
      }
      else
      {
        QgsDebugMsg( QStringLiteral( "Ignoring too deep child group name %1." ).arg( name ) );
        addTreeItem( name, isVector, groupIndex, mRootItem.get() );
      }
    }
  }

  endResetModel();
}

void QgsMeshDatasetGroupTreeModel::addTreeItem( const QString &groupName, bool isVector, int groupIndex, QgsMeshDatasetGroupTreeItem *parent )
{
  Q_ASSERT( parent );
  QgsMeshDatasetGroupTreeItem *item = new QgsMeshDatasetGroupTreeItem( groupName, isVector, groupIndex, parent );
  parent->appendChild( item );

  if ( mNameToItem.contains( groupName ) )
  {
    QgsDebugMsg( QStringLiteral( "Group %1 is not unique" ).arg( groupName ) );
  }
  mNameToItem[groupName] = item;

  if ( mDatasetGroupIndexToItem.contains( groupIndex ) )
  {
    QgsDebugMsg( QStringLiteral( "Group index %1 is not unique" ).arg( groupIndex ) );
  }
  mDatasetGroupIndexToItem[groupIndex] = item;
}

QModelIndex QgsMeshDatasetGroupTreeModel::groupIndexToModelIndex( int groupIndex )
{
  if ( groupIndex < 0 || !mDatasetGroupIndexToItem.contains( groupIndex ) )
    return QModelIndex();

  const auto item = mDatasetGroupIndexToItem[groupIndex];
  auto parentItem = item->parentItem();
  if ( parentItem )
  {
    const auto parentIndex = index( parentItem->row(), 0, QModelIndex() );
    return index( item->row(), 0, parentIndex );
  }
  else
    return QModelIndex();

}

/////////////////////////////////////////////////////////////////////////////////////////


QgsMeshDatasetGroupTreeItemDelagate::QgsMeshDatasetGroupTreeItemDelagate( QObject *parent )
  : QStyledItemDelegate( parent )
  , mScalarSelectedPixmap( QStringLiteral( ":/images/themes/default/propertyicons/meshcontours.svg" ) )
  , mScalarDeselectedPixmap( QStringLiteral( ":/images/themes/default/propertyicons/meshcontoursoff.svg" ) )
  , mVectorSelectedPixmap( QStringLiteral( ":/images/themes/default/propertyicons/meshvectors.svg" ) )
  , mVectorDeselectedPixmap( QStringLiteral( ":/images/themes/default/propertyicons/meshvectorsoff.svg" ) )
{
}

void QgsMeshDatasetGroupTreeItemDelagate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  if ( !painter )
    return;

  QStyledItemDelegate::paint( painter, option, index );
  bool isVector = index.data( QgsMeshDatasetGroupTreeModel::IsVector ).toBool();
  if ( isVector )
  {
    bool isActive = index.data( QgsMeshDatasetGroupTreeModel::IsActiveVectorDatasetGroup ).toBool();
    painter->drawPixmap( iconRect( option.rect, true ), isActive ? mVectorSelectedPixmap : mVectorDeselectedPixmap );
  }
  bool isActive = index.data( QgsMeshDatasetGroupTreeModel::IsActiveScalarDatasetGroup ).toBool();
  painter->drawPixmap( iconRect( option.rect, false ), isActive ? mScalarSelectedPixmap : mScalarDeselectedPixmap );
}

QRect QgsMeshDatasetGroupTreeItemDelagate::iconRect( const QRect rect, bool isVector ) const
{
  int iw = mScalarSelectedPixmap.width();
  int ih = mScalarSelectedPixmap.height();
  int margin = ( rect.height() - ih ) / 2;
  int i = isVector ? 1 : 2;
  return QRect( rect.right() - i * ( iw + margin ), rect.top() + margin, iw, ih );
}

QSize QgsMeshDatasetGroupTreeItemDelagate::sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  QSize hint = QStyledItemDelegate::sizeHint( option, index );
  if ( hint.height() < 16 )
    hint.setHeight( 16 );
  return hint;
}

/////////////////////////////////////////////////////////////////////////////////////////

QgsMeshDatasetGroupTreeView::QgsMeshDatasetGroupTreeView( QWidget *parent )
  : QTreeView( parent )
{
  setModel( &mModel );
  setItemDelegate( &mDelegate );
  setSelectionMode( QAbstractItemView::SingleSelection );
}

void QgsMeshDatasetGroupTreeView::setLayer( QgsMeshLayer *layer )
{
  if ( layer != mMeshLayer )
  {
    mMeshLayer = layer;
    syncToLayer();
  }
}

int QgsMeshDatasetGroupTreeView::activeScalarGroup() const
{
  return mModel.activeScalarGroup();
}

void QgsMeshDatasetGroupTreeView::setActiveScalarGroup( int group )
{
  if ( mModel.activeScalarGroup() != group )
  {
    mModel.setActiveScalarGroup( group );
    emit activeScalarGroupChanged( group );
  }
}

int QgsMeshDatasetGroupTreeView::activeVectorGroup() const
{
  return mModel.activeVectorGroup();
}

void QgsMeshDatasetGroupTreeView::setActiveVectorGroup( int group )
{
  if ( mModel.activeVectorGroup() != group )
  {
    mModel.setActiveVectorGroup( group );
    emit activeVectorGroupChanged( group );
  }
}

void QgsMeshDatasetGroupTreeView::syncToLayer()
{
  mModel.syncToLayer( mMeshLayer );
  setActiveGroupFromActiveDataset();
}

void QgsMeshDatasetGroupTreeView::mousePressEvent( QMouseEvent *event )
{
  if ( !event )
    return;

  bool processed = false;
  const QModelIndex idx = indexAt( event->pos() );
  if ( idx.isValid() )
  {
    const QRect vr = visualRect( idx );
    if ( mDelegate.iconRect( vr, true ).contains( event->pos() ) )
    {
      bool isVector = idx.data( QgsMeshDatasetGroupTreeModel::IsVector ).toBool();
      if ( isVector )
      {
        setActiveVectorGroup( idx.data( QgsMeshDatasetGroupTreeModel::DatasetGroupIndex ).toInt() );
        processed = true;
      }
    }
    else if ( mDelegate.iconRect( vr, false ).contains( event->pos() ) )
    {
      setActiveScalarGroup( idx.data( QgsMeshDatasetGroupTreeModel::DatasetGroupIndex ).toInt() );
      processed = true;
    }
  }

  // only if the user did not click one of the icons do usual handling
  if ( !processed )
    QTreeView::mousePressEvent( event );
}

void QgsMeshDatasetGroupTreeView::setActiveGroupFromActiveDataset()
{
  int scalarGroup = -1;
  int vectorGroup = -1;

  // find active dataset
  if ( mMeshLayer )
  {
    const QgsMeshRendererSettings rendererSettings = mMeshLayer->rendererSettings();
    scalarGroup = rendererSettings.activeScalarDataset().group();
    vectorGroup = rendererSettings.activeVectorDataset().group();
  }

  setActiveScalarGroup( scalarGroup );
  setActiveVectorGroup( vectorGroup );
}
