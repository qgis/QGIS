/***************************************************************************
 qgsquicksubmodel.cpp
  --------------------------------------
  Date                 : 16.9.2016
  Copyright            : (C) 2016 by Matthias Kuhn
  Email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsquicksubmodel.h"

QgsQuickSubModel::QgsQuickSubModel( QObject *parent )
  : QAbstractItemModel( parent )
{
}

QModelIndex QgsQuickSubModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( !mModel )
    return mRootIndex;

  QModelIndex sourceIndex = mModel->index( row, column, parent.isValid() ? mapToSource( parent ) : static_cast<QModelIndex>( mRootIndex ) );
  return mapFromSource( sourceIndex );
}

QModelIndex QgsQuickSubModel::parent( const QModelIndex &child ) const
{
  if ( !mModel )
    return mRootIndex;

  QModelIndex idx = mModel->parent( child );
  if ( idx == mRootIndex )
    return QModelIndex();
  else
    return mapFromSource( idx );
}

int QgsQuickSubModel::rowCount( const QModelIndex &parent ) const
{
  if ( !mModel )
    return 0;

  return mModel->rowCount( parent.isValid() ? mapToSource( parent ) : static_cast<QModelIndex>( mRootIndex ) );
}

int QgsQuickSubModel::columnCount( const QModelIndex &parent ) const
{
  if ( !mModel )
    return 0;

  return mModel->columnCount( parent.isValid() ? mapToSource( parent ) : static_cast<QModelIndex>( mRootIndex ) );
}

QVariant QgsQuickSubModel::data( const QModelIndex &index, int role ) const
{
  if ( !mModel )
    return QVariant();

  return mModel->data( mapToSource( index ), role );
}

bool QgsQuickSubModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !mModel )
    return false;

  return mModel->setData( mapToSource( index ), value, role );
}

QHash<int, QByteArray> QgsQuickSubModel::roleNames() const
{
  if ( !mModel )
    return QHash<int, QByteArray>();

  return mModel->roleNames();
}

QModelIndex QgsQuickSubModel::rootIndex() const
{
  return mRootIndex;
}

void QgsQuickSubModel::setRootIndex( const QModelIndex &rootIndex )
{
  if ( rootIndex == mRootIndex )
    return;

  beginResetModel();
  mRootIndex = rootIndex;
  endResetModel();
  emit rootIndexChanged();
}

QAbstractItemModel *QgsQuickSubModel::model() const
{
  return mModel;
}

void QgsQuickSubModel::setModel( QAbstractItemModel *model )
{
  if ( model == mModel )
    return;

  if ( model )
  {
    connect( model, &QAbstractItemModel::rowsAboutToBeInserted, this, &QgsQuickSubModel::onRowsAboutToBeInserted );
    connect( model, &QAbstractItemModel::rowsInserted, this, &QgsQuickSubModel::onRowsInserted );
    connect( model, &QAbstractItemModel::rowsAboutToBeRemoved, this, &QgsQuickSubModel::onRowsAboutToBeRemoved );
    connect( model, &QAbstractItemModel::rowsRemoved, this, &QgsQuickSubModel::onRowsRemoved );
    connect( model, &QAbstractItemModel::modelAboutToBeReset, this, &QgsQuickSubModel::onModelAboutToBeReset );
    connect( model, &QAbstractItemModel::modelReset, this, &QAbstractItemModel::modelReset );
    connect( model, &QAbstractItemModel::dataChanged, this, &QgsQuickSubModel::onDataChanged );
  }

  mModel = model;
  emit modelChanged();
}

void QgsQuickSubModel::onRowsAboutToBeInserted( const QModelIndex &parent, int first, int last )
{
  beginInsertRows( mapFromSource( parent ), first, last );
}

void QgsQuickSubModel::onRowsInserted( const QModelIndex &parent, int first, int last )
{
  Q_UNUSED( parent )
  Q_UNUSED( first )
  Q_UNUSED( last )
  endInsertRows();
}

void QgsQuickSubModel::onRowsAboutToBeRemoved( const QModelIndex &parent, int first, int last )
{
  beginRemoveRows( mapFromSource( parent ), first, last );
}

void QgsQuickSubModel::onRowsRemoved( const QModelIndex &parent, int first, int last )
{
  Q_UNUSED( parent )
  Q_UNUSED( first )
  Q_UNUSED( last )
  endRemoveRows();
}

void QgsQuickSubModel::onModelAboutToBeReset()
{
  mMappings.clear();
}

void QgsQuickSubModel::onDataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles )
{
  emit dataChanged( mapFromSource( topLeft ), mapFromSource( bottomRight ), roles );
}

QModelIndex QgsQuickSubModel::mapFromSource( const QModelIndex &sourceIndex ) const
{
  if ( sourceIndex == mRootIndex || !sourceIndex.isValid() )
    return QModelIndex();

  if ( !mMappings.contains( sourceIndex.internalId() ) )
  {
    mMappings.insert( sourceIndex.internalId(), sourceIndex.parent() );
  }

  return createIndex( sourceIndex.row(), sourceIndex.column(), sourceIndex.internalId() );
}

QModelIndex QgsQuickSubModel::mapToSource( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return mRootIndex;

  if ( !mModel )
    return mRootIndex;

  return mModel->index( index.row(), index.column(), mMappings.find( index.internalId() ).value() );
}
