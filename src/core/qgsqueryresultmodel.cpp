/***************************************************************************
  qgsqueryresultmodel.cpp - QgsQueryResultModel

 ---------------------
 begin                : 24.12.2020
 copyright            : (C) 2020 by Alessandro Pasotti
 email                : elpaso@itopen.it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsqueryresultmodel.h"

const int QgsQueryResultModel::ROWS_TO_FETCH = 200;

QgsQueryResultModel::QgsQueryResultModel( const QgsAbstractDatabaseProviderConnection::QueryResult &queryResult, QObject *parent )
  : QAbstractListModel( parent )
  , mQueryResult( queryResult )
{
}

int QgsQueryResultModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;
  return mRowCount;
}

int QgsQueryResultModel::columnCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;
  return mQueryResult.columns().count();
}

QVariant QgsQueryResultModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() || index.row() < 0 || index.column() > mQueryResult.columns().count() - 1 )
    return QVariant();

  if ( index.row() >= mRowCount || index.row() < 0 )
    return QVariant();

  if ( role == Qt::DisplayRole )
  {
    const QList<QVariant> result { mQueryResult.at( index.row() ) };
    if ( index.column() < result.count( ) )
    {
      return result.at( index.column() );
    }
  }
  return QVariant();
}

void QgsQueryResultModel::fetchMore( const QModelIndex &parent )
{
  if ( parent.isValid() )
    return;

  QVariantList newRows;
  for ( int i = 0; i < ROWS_TO_FETCH && mQueryResult.hasNextRow(); ++i )
  {
    newRows.push_back( mQueryResult.nextRow() );
  }

  if ( !newRows.isEmpty() )
  {
    const qlonglong newRowsCount { newRows.count() };
    beginInsertRows( QModelIndex(), newRowsCount, newRowsCount + newRows.count() - 1 );
    mRowCount += newRowsCount;
    endInsertRows();
  }
}

bool QgsQueryResultModel::canFetchMore( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return false;

  return mQueryResult.hasNextRow();
}
