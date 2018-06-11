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

QgsMeshDatasetGroupTreeItem::QgsMeshDatasetGroupTreeItem( QgsMeshDatasetGroupTreeItem *parent )
  : mParent( parent )
{
}

QgsMeshDatasetGroupTreeItem::QgsMeshDatasetGroupTreeItem( const QString &name, QgsMeshDatasetGroupTreeItem *parent )
  : mParent( parent )
  , mName( name )
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

int QgsMeshDatasetGroupTreeItem::columnCount() const
{
  return 1;
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

QVariant QgsMeshDatasetGroupTreeItem::data( int column ) const
{
  Q_UNUSED( column );
  return mName;
}

/////////////////////////////////////////////////////////////////////////////////////////

QgsMeshDatasetGroupTreeModel::QgsMeshDatasetGroupTreeModel( QObject *parent )
  : QAbstractItemModel( parent )
  ,  mRootItem( new QgsMeshDatasetGroupTreeItem() )
{
}

QgsMeshDatasetGroupTreeModel::~QgsMeshDatasetGroupTreeModel() = default;

int QgsMeshDatasetGroupTreeModel::columnCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return static_cast<QgsMeshDatasetGroupTreeItem *>( parent.internalPointer() )->columnCount();
  else
    return mRootItem->columnCount();
}

QVariant QgsMeshDatasetGroupTreeModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  if ( role != Qt::DisplayRole )
    return QVariant();

  QgsMeshDatasetGroupTreeItem *item = static_cast<QgsMeshDatasetGroupTreeItem *>( index.internalPointer() );

  return item->data( index.column() );
}

Qt::ItemFlags QgsMeshDatasetGroupTreeModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return Qt::NoItemFlags;

  return QAbstractItemModel::flags( index );
}

QVariant QgsMeshDatasetGroupTreeModel::headerData( int section,
    Qt::Orientation orientation,
    int role ) const
{
  Q_UNUSED( section );

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

void QgsMeshDatasetGroupTreeModel::setupModelData( const QStringList &groups )
{
  beginResetModel();

  mRootItem.reset( new QgsMeshDatasetGroupTreeItem() );

  for ( const QString &groupName : groups )
  {
    QgsMeshDatasetGroupTreeItem *item = new QgsMeshDatasetGroupTreeItem( groupName, mRootItem.get() );
    mRootItem->appendChild( item );
  }

  endResetModel();
}

/////////////////////////////////////////////////////////////////////////////////////////

QgsMeshDatasetGroupTreeView::QgsMeshDatasetGroupTreeView( QWidget *parent )
  : QTreeView( parent )
{
  setModel( &mModel );

  setSelectionMode( QAbstractItemView::SingleSelection ) ;
  connect( selectionModel(),
           &QItemSelectionModel::selectionChanged,
           this,
           &QgsMeshDatasetGroupTreeView::onSelectionChanged
         );
}

void QgsMeshDatasetGroupTreeView::setLayer( QgsMeshLayer *layer )
{
  if ( layer != mMeshLayer )
  {
    mMeshLayer = layer;
    syncToLayer();
  }
}

QVector<int> QgsMeshDatasetGroupTreeView::datasetsInActiveGroup() const
{
  if ( mGroups.constFind( mActiveGroup ) == mGroups.constEnd() )
    return QVector<int>();
  else
    return mGroups[mActiveGroup];
}

void QgsMeshDatasetGroupTreeView::onSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected )
{
  Q_UNUSED( deselected );

  if ( selected.isEmpty() )
  {
    mActiveGroup = QString();
    return;
  }

  if ( selected.first().indexes().isEmpty() )
  {
    mActiveGroup = QString();
    return;
  }

  QModelIndex index = selected.first().indexes().first(); //single selection only
  QVariant name = mModel.data( index, 0 );
  mActiveGroup = name.toString();
  emit activeGroupChanged();
}


void QgsMeshDatasetGroupTreeView::extractGroups()
{
  // TODO replace with MDAL groups when introduced
  mGroups.clear();

  if ( !mMeshLayer || !mMeshLayer->dataProvider() )
    return;

  for ( int i = 0; i < mMeshLayer->dataProvider()->datasetCount(); ++i )
  {
    const QgsMeshDatasetMetadata meta = mMeshLayer->dataProvider()->datasetMetadata( i );
    QString name = meta.extraOptions()["name"];
    if ( mGroups.constFind( name ) == mGroups.constEnd() )
    {
      QVector<int> datasets;
      datasets.append( i );
      mGroups[name] = datasets;
    }
    else
    {
      mGroups[name].append( i );
    }
  }
}

void QgsMeshDatasetGroupTreeView::syncToLayer()
{
  mActiveGroup.clear();

  extractGroups();

  mModel.setupModelData( mGroups.keys() );

  if ( mGroups.size() > 0 )
    setCurrentIndex( mModel.index( 0, 0 ) );
}
