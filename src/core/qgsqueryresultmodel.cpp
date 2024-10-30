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
#include "moc_qgsqueryresultmodel.cpp"
#include "qgsexpression.h"

const int QgsQueryResultModel::FETCH_MORE_ROWS_COUNT = 400;

QgsQueryResultModel::QgsQueryResultModel( const QgsAbstractDatabaseProviderConnection::QueryResult &queryResult, QObject *parent )
  : QAbstractTableModel( parent )
  , mQueryResult( queryResult )
  , mColumns( queryResult.columns() )
{
  qRegisterMetaType< QList<QList<QVariant>>>( "QList<QList<QVariant>>" );
  mWorker = std::make_unique<QgsQueryResultFetcher>( &mQueryResult );
  mWorker->moveToThread( &mWorkerThread );
  // Forward signals to the model
  connect( mWorker.get(), &QgsQueryResultFetcher::rowsReady, this, &QgsQueryResultModel::rowsReady );
  connect( mWorker.get(), &QgsQueryResultFetcher::fetchingComplete, this, &QgsQueryResultModel::fetchingComplete );
  connect( this, &QgsQueryResultModel::fetchMoreRows, mWorker.get(), &QgsQueryResultFetcher::fetchRows );
  mWorkerThread.start();
  if ( mQueryResult.rowCount() > 0 )
  {
    mRows.reserve( mQueryResult.rowCount() );
  }
}

void QgsQueryResultModel::rowsReady( const QList<QList<QVariant>> &rows )
{
  beginInsertRows( QModelIndex(), mRows.count( ), mRows.count( ) + rows.count() - 1 );
  mRows.append( rows );
  endInsertRows();
}


bool QgsQueryResultModel::canFetchMore( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return false;
  return mQueryResult.rowCount() < 0 || mRows.length() < mQueryResult.rowCount();
}


void QgsQueryResultModel::fetchMore( const QModelIndex &parent )
{
  if ( ! parent.isValid() )
  {
    emit fetchingStarted();
    emit fetchMoreRows( FETCH_MORE_ROWS_COUNT );
  }
}

void QgsQueryResultModel::cancel()
{
  if ( mWorker )
  {
    mWorker->stopFetching();
  }
}

QgsAbstractDatabaseProviderConnection::QueryResult QgsQueryResultModel::queryResult() const
{
  return mQueryResult;
}

QStringList QgsQueryResultModel::columns() const
{
  return mColumns;
}

QgsQueryResultModel::~QgsQueryResultModel()
{
  if ( mWorker )
  {
    mWorker->stopFetching();
    mWorkerThread.quit();
    mWorkerThread.wait();
  }
  else
  {
    emit fetchingComplete();
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
    case Qt::DisplayRole:
    {
      const QList<QVariant> result = mRows.at( index.row() );
      if ( index.column() < result.count( ) )
      {
        return result.at( index.column() );
      }
      break;
    }

    case Qt::ToolTipRole:
    {
      const QList<QVariant> result = mRows.at( index.row() );
      if ( index.column() < result.count( ) )
      {
        const QVariant value = result.at( index.column() );
        return QgsExpression::formatPreviewString( value, true, 255 );
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

const int QgsQueryResultFetcher::ROWS_BATCH_COUNT = 200;

void QgsQueryResultFetcher::fetchRows( long long maxRows )
{
  long long rowCount { 0 };
  QList<QList<QVariant>> newRows;
  newRows.reserve( ROWS_BATCH_COUNT );
  while ( mStopFetching == 0 && mQueryResult->hasNextRow() && ( maxRows < 0 || rowCount < maxRows ) )
  {
    newRows.append( mQueryResult->nextRow() );
    ++rowCount;
    if ( rowCount % ROWS_BATCH_COUNT == 0 && mStopFetching == 0 )
    {
      emit rowsReady( newRows );
      newRows.clear();
    }
  }

  if ( rowCount % ROWS_BATCH_COUNT && mStopFetching == 0 )
  {
    emit rowsReady( newRows );
  }

  emit fetchingComplete();
}

void QgsQueryResultFetcher::stopFetching()
{
  mStopFetching = 1;
}


///@endcond private
