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

QgsQueryResultModel::QgsQueryResultModel( const QgsAbstractDatabaseProviderConnection::QueryResult &queryResult, QObject *parent )
  : QAbstractTableModel( parent )
  , mQueryResult( queryResult )
  , mColumns( queryResult.columns() )
{
  qRegisterMetaType< QList<QList<QVariant>>>( "QList<QList<QVariant>>" );
  if ( mQueryResult.hasNextRow() )
  {
    mWorker = new QgsQueryResultFetcher( &mQueryResult );
    mWorker->moveToThread( &mWorkerThread );
    connect( &mWorkerThread, &QThread::started, mWorker, &QgsQueryResultFetcher::fetchRows );
    connect( mWorker, &QgsQueryResultFetcher::rowsReady, this, &QgsQueryResultModel::rowsReady );
    mWorkerThread.start();
  }
}

void QgsQueryResultModel::rowsReady( const QList<QList<QVariant>> &rows )
{
  beginInsertRows( QModelIndex(), mRows.count( ), mRows.count( ) + rows.count() - 1 );
  mRows.append( rows );
  endInsertRows();
}

void QgsQueryResultModel::cancel()
{
  if ( mWorker )
  {
    mWorker->stopFetching();
  }
}

QgsQueryResultModel::~QgsQueryResultModel()
{
  if ( mWorker )
  {
    mWorker->stopFetching();
    mWorkerThread.quit();
    mWorkerThread.wait();
    mWorker->deleteLater();
  }
}

int QgsQueryResultModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;
  return mRows.count();
}

int QgsQueryResultModel::columnCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;
  return mColumns.count();
}

QVariant QgsQueryResultModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() || index.row() < 0 || index.column() >= mColumns.count() ||
       index.row() >= mRows.count( ) )
    return QVariant();

  switch ( role )
  {
    case  Qt::DisplayRole:
    {
      const QList<QVariant> result = mRows.at( index.row() );
      if ( index.column() < result.count( ) )
      {
        return result.at( index.column() );
      }
      break;
    }
  }
  return QVariant();
}

QVariant QgsQueryResultModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation == Qt::Orientation::Horizontal && role == Qt::ItemDataRole::DisplayRole && section < mColumns.count() )
  {
    return mColumns.at( section );
  }
  return QAbstractTableModel::headerData( section, orientation, role );
}

///@cond private

const int QgsQueryResultFetcher::ROWS_TO_FETCH = 200;

void QgsQueryResultFetcher::fetchRows()
{
  qlonglong rowCount { 0 };
  QList<QList<QVariant>> newRows;
  while ( mStopFetching == 0 && mQueryResult->hasNextRow() )
  {
    newRows.append( mQueryResult->nextRow() );
    ++rowCount;
    if ( rowCount % ROWS_TO_FETCH == 0 && mStopFetching == 0 )
    {
      emit rowsReady( newRows );
      newRows.clear();
    }
  }

  if ( rowCount % ROWS_TO_FETCH && mStopFetching == 0 )
  {
    emit rowsReady( newRows );
  }
}

void QgsQueryResultFetcher::stopFetching()
{
  mStopFetching = 1;
}


///@endcond private
