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

const int ResultWorker::ROWS_TO_FETCH = 200;

QgsQueryResultModel::QgsQueryResultModel(const QgsAbstractDatabaseProviderConnection::QueryResult& queryResult, QObject *parent )
  : QAbstractListModel( parent )
  , mQueryResult( queryResult )
  , mColumns( queryResult.columns() )
  , mRowCount( mQueryResult.fetchedRowCount() )
{
  if ( mQueryResult.hasNextRow() )
  {
    ResultWorker *worker = new ResultWorker( &mQueryResult );
    worker->moveToThread(&mWorkerThread);
    //connect(&mWorkerThread, &QThread::finished, worker, &ResultWorker::stopFetching );
    connect(&mWorkerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(&mWorkerThread, &QThread::started, worker, &ResultWorker::fetchRows);
    connect(worker, &ResultWorker::rowsReady, this, &QgsQueryResultModel::newRowsReady );
    mWorkerThread.start();
  }
}

void QgsQueryResultModel::newRowsReady( int newRowCount )
{
  beginInsertRows( QModelIndex(), mRowCount, mRowCount + newRowCount - 1 );
  mRowCount += newRowCount;
  endInsertRows();
}

QgsQueryResultModel::~QgsQueryResultModel()
{
  mWorkerThread.quit();
  mWorkerThread.wait();
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
  return mColumns.count();
}

QVariant QgsQueryResultModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() || index.row() < 0 || index.column() > mColumns.count() - 1 ||
       index.row() >= mRowCount )
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

void ResultWorker::fetchRows()
{
  int rowCount = 0;
  while ( mQueryResult->hasNextRow() && mStopFetching == 0 )
  {
     mQueryResult->nextRow();
     ++rowCount;
     if ( rowCount == ROWS_TO_FETCH && mStopFetching == 0 )
     {
       emit rowsReady( rowCount );
       rowCount = 0;
     }
  }

  if ( rowCount  > 0 && mStopFetching == 0 )
  {
    emit rowsReady( rowCount );
  }
}

void ResultWorker::stopFetching()
{
  qDebug() << "Stop fetching" << mQueryResult->fetchedRowCount();
  mStopFetching = 1;
}
