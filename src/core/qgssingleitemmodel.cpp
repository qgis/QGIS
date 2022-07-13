/***************************************************************************
    qgssingleitemmodel.cpp
    ---------------
    begin                : May 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssingleitemmodel.h"


QgsSingleItemModel::QgsSingleItemModel( QObject *parent, const QString &text, const QMap< int, QVariant > &data, Qt::ItemFlags flags )
  : QAbstractItemModel( parent )
  , mText( text )
  , mData( data )
  , mFlags( flags )
{

}

QgsSingleItemModel::QgsSingleItemModel( QObject *parent, const QList<QMap<int, QVariant> > &columnData, Qt::ItemFlags flags )
  : QAbstractItemModel( parent )
  , mColumnData( columnData )
  , mFlags( flags )
{
}

QVariant QgsSingleItemModel::data( const QModelIndex &index, int role ) const
{
  if ( index.row() < 0 || index.row() >= rowCount( QModelIndex() ) )
    return QVariant();

  if ( index.column() < 0 || index.column() >= columnCount( QModelIndex() ) )
    return QVariant();

  if ( !mColumnData.isEmpty() )
  {
    return mColumnData.value( index.column() ).value( role );
  }
  else
  {
    switch ( role )
    {
      case Qt::DisplayRole:
        return mText;

      case Qt::ToolTipRole:
        return mData.value( Qt::ToolTipRole, mText );

      default:
        return mData.value( role );
    }
  }
}

Qt::ItemFlags QgsSingleItemModel::flags( const QModelIndex &index ) const
{
  if ( index.isValid() )
  {
    return mFlags;
  }
  else
  {
    return QAbstractItemModel::flags( index );
  }
}

QModelIndex QgsSingleItemModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( !hasIndex( row, column, parent ) )
    return QModelIndex();

  if ( !parent.isValid() )
  {
    return createIndex( row, column );
  }

  return QModelIndex();
}

QModelIndex QgsSingleItemModel::parent( const QModelIndex & ) const
{
  return QModelIndex();
}

int QgsSingleItemModel::rowCount( const QModelIndex &parent ) const
{
  if ( !parent.isValid() )
  {
    return 1;
  }
  return 0;
}

int QgsSingleItemModel::columnCount( const QModelIndex & ) const
{
  if ( !mColumnData.empty() )
    return mColumnData.size();

  return 1;
}
