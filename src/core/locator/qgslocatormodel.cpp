/***************************************************************************
                         qgslocatormodel.cpp
                         --------------------
    begin                : May 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgslocatormodel.h"
#include "qgslocator.h"
#include "qgsapplication.h"
#include "qgslogger.h"

//
// QgsLocatorModel
//

QgsLocatorModel::QgsLocatorModel( QObject *parent )
  : QAbstractTableModel( parent )
{
  mDeferredClearTimer.setInterval( 100 );
  mDeferredClearTimer.setSingleShot( true );
  connect( &mDeferredClearTimer, &QTimer::timeout, this, &QgsLocatorModel::clear );
}

void QgsLocatorModel::clear()
{
  mDeferredClearTimer.stop();
  mDeferredClear = false;

  beginResetModel();
  mResults.clear();
  mFoundResultsFromFilterNames.clear();
  endResetModel();
}

void QgsLocatorModel::deferredClear()
{
  mDeferredClear = true;
  mDeferredClearTimer.start();
}

int QgsLocatorModel::rowCount( const QModelIndex & ) const
{
  return mResults.size();
}

int QgsLocatorModel::columnCount( const QModelIndex & ) const
{
  return 2;
}

QVariant QgsLocatorModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() || index.row() < 0 || index.column() < 0 ||
       index.row() >= rowCount( QModelIndex() ) || index.column() >= columnCount( QModelIndex() ) )
    return QVariant();

  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::EditRole:
    {
      switch ( index.column() )
      {
        case Name:
          if ( !mResults.at( index.row() ).filter )
            return mResults.at( index.row() ).result.displayString;
          else
            return mResults.at( index.row() ).filterTitle;
        case Description:
          if ( !mResults.at( index.row() ).filter )
            return mResults.at( index.row() ).result.description;
          else
            return QVariant();
      }
      break;
    }

    case Qt::DecorationRole:
      switch ( index.column() )
      {
        case Name:
          if ( !mResults.at( index.row() ).filter )
          {
            QIcon icon = mResults.at( index.row() ).result.icon;
            if ( !icon.isNull() )
              return icon;
            return QgsApplication::getThemeIcon( QStringLiteral( "/search.svg" ) );
          }
          else
            return QVariant();
        case Description:
          return QVariant();
      }
      break;

    case ResultDataRole:
      if ( !mResults.at( index.row() ).filter )
        return QVariant::fromValue( mResults.at( index.row() ).result );
      else
        return QVariant();

    case ResultTypeRole:
      if ( mResults.at( index.row() ).filter )
        return 0;
      else
        return 1;

    case ResultScoreRole:
      if ( mResults.at( index.row() ).filter )
        return 0;
      else
        return ( mResults.at( index.row() ).result.score );

    case ResultFilterPriorityRole:
      if ( !mResults.at( index.row() ).filter )
        return mResults.at( index.row() ).result.filter->priority();
      else
        return mResults.at( index.row() ).filter->priority();

    case ResultFilterNameRole:
      if ( !mResults.at( index.row() ).filter )
        return mResults.at( index.row() ).result.filter->displayName();
      else
        return mResults.at( index.row() ).filterTitle;
  }

  return QVariant();
}

Qt::ItemFlags QgsLocatorModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() || index.row() < 0 || index.column() < 0 ||
       index.row() >= rowCount( QModelIndex() ) || index.column() >= columnCount( QModelIndex() ) )
    return QAbstractTableModel::flags( index );

  Qt::ItemFlags flags = QAbstractTableModel::flags( index );
  if ( !mResults.at( index.row() ).filterTitle.isEmpty() )
  {
    flags = flags & ~( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
  }
  return flags;
}

void QgsLocatorModel::addResult( const QgsLocatorResult &result )
{
  mDeferredClearTimer.stop();
  if ( mDeferredClear )
  {
    mFoundResultsFromFilterNames.clear();
  }

  int pos = mResults.size();
  bool addingFilter = !result.filter->displayName().isEmpty() && !mFoundResultsFromFilterNames.contains( result.filter->name() );
  if ( addingFilter )
    mFoundResultsFromFilterNames << result.filter->name();

  if ( mDeferredClear )
  {
    beginResetModel();
    mResults.clear();
  }
  else
    beginInsertRows( QModelIndex(), pos, pos + ( addingFilter ? 1 : 0 ) );

  if ( addingFilter )
  {
    Entry entry;
    entry.filterTitle = result.filter->displayName();
    entry.filter = result.filter;
    mResults << entry;
  }
  Entry entry;
  entry.result = result;
  mResults << entry;

  if ( mDeferredClear )
    endResetModel();
  else
    endInsertRows();

  mDeferredClear = false;
}


//
// QgsLocatorAutomaticModel
//

QgsLocatorAutomaticModel::QgsLocatorAutomaticModel( QgsLocator *locator )
  : QgsLocatorModel( locator )
  , mLocator( locator )
{
  Q_ASSERT( mLocator );
  connect( mLocator, &QgsLocator::foundResult, this, &QgsLocatorAutomaticModel::addResult );
  connect( mLocator, &QgsLocator::finished, this, &QgsLocatorAutomaticModel::searchFinished );
}

QgsLocator *QgsLocatorAutomaticModel::locator()
{
  return mLocator;
}

void QgsLocatorAutomaticModel::search( const QString &string )
{
  if ( mLocator->isRunning() )
  {
    // can't do anything while a query is running, and can't block
    // here waiting for the current query to cancel
    // so we queue up this string until cancel has happened
    mLocator->cancelWithoutBlocking();
    mNextRequestedString = string;
    mHasQueuedRequest = true;
    return;
  }
  else
  {
    deferredClear();
    mLocator->fetchResults( string, createContext() );
  }
}

QgsLocatorContext QgsLocatorAutomaticModel::createContext()
{
  return QgsLocatorContext();
}

void QgsLocatorAutomaticModel::searchFinished()
{
  if ( mHasQueuedRequest )
  {
    // a queued request was waiting for this - run the queued search now
    QString nextSearch = mNextRequestedString;
    mNextRequestedString.clear();
    mHasQueuedRequest = false;
    search( nextSearch );
  }
}





//
// QgsLocatorProxyModel
//

QgsLocatorProxyModel::QgsLocatorProxyModel( QObject *parent )
  : QSortFilterProxyModel( parent )
{
  setDynamicSortFilter( true );
  setSortLocaleAware( true );
  setFilterCaseSensitivity( Qt::CaseInsensitive );
  sort( 0 );
}

bool QgsLocatorProxyModel::lessThan( const QModelIndex &left, const QModelIndex &right ) const
{
  // first go by filter priority
  int leftFilterPriority = sourceModel()->data( left, QgsLocatorModel::ResultFilterPriorityRole ).toInt();
  int rightFilterPriority  = sourceModel()->data( right, QgsLocatorModel::ResultFilterPriorityRole ).toInt();
  if ( leftFilterPriority != rightFilterPriority )
    return leftFilterPriority < rightFilterPriority;

  // then filter name
  QString leftFilter = sourceModel()->data( left, QgsLocatorModel::ResultFilterNameRole ).toString();
  QString rightFilter = sourceModel()->data( right, QgsLocatorModel::ResultFilterNameRole ).toString();
  if ( leftFilter != rightFilter )
    return QString::localeAwareCompare( leftFilter, rightFilter ) < 0;

  // then make sure filter title appears before filter's results
  int leftTypeRole = sourceModel()->data( left, QgsLocatorModel::ResultTypeRole ).toInt();
  int rightTypeRole = sourceModel()->data( right, QgsLocatorModel::ResultTypeRole ).toInt();
  if ( leftTypeRole != rightTypeRole )
    return leftTypeRole < rightTypeRole;

  // sort filter's results by score
  double leftScore = sourceModel()->data( left, QgsLocatorModel::ResultScoreRole ).toDouble();
  double rightScore = sourceModel()->data( right, QgsLocatorModel::ResultScoreRole ).toDouble();
  if ( !qgsDoubleNear( leftScore, rightScore ) )
    return leftScore > rightScore;

  // lastly sort filter's results by string
  leftFilter = sourceModel()->data( left, Qt::DisplayRole ).toString();
  rightFilter = sourceModel()->data( right, Qt::DisplayRole ).toString();
  return QString::localeAwareCompare( leftFilter, rightFilter ) < 0;
}


