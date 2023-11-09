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

#include <QFont>

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
  mFoundResultsFilterGroups.clear();
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
          else if ( mResults.at( index.row() ).filter && mResults.at( index.row() ).groupSorting == 0 )
            return mResults.at( index.row() ).filterTitle;
          else
          {
            QString groupTitle = mResults.at( index.row() ).groupTitle;
            groupTitle.prepend( "  " );
            return groupTitle;
          }
        case Description:
          if ( !mResults.at( index.row() ).filter )
            return mResults.at( index.row() ).result.description;
          else
            return QVariant();
      }
      break;
    }

    case Qt::FontRole:
      if ( index.column() == Name && !mResults.at( index.row() ).groupTitle.isEmpty() )
      {
        QFont font;
        font.setItalic( true );
        return font;
      }
      else
      {
        return QVariant();
      }
      break;

    case Qt::DecorationRole:
      switch ( index.column() )
      {
        case Name:
          if ( !mResults.at( index.row() ).filter )
          {
            const QIcon icon = mResults.at( index.row() ).result.icon;
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
      // 0 for filter title, the group otherwise, 9999 if no group
      return mResults.at( index.row() ).groupSorting;

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

    case ResultFilterGroupSortingRole:
      if ( mResults.at( index.row() ).groupTitle.isEmpty() )
        return 1;
      else
        return 0;

    case ResultActionsRole:
      return QVariant::fromValue( mResults.at( index.row() ).result.actions );
  }

  return QVariant();
}

Qt::ItemFlags QgsLocatorModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() || index.row() < 0 || index.column() < 0 ||
       index.row() >= rowCount( QModelIndex() ) || index.column() >= columnCount( QModelIndex() ) )
    return QAbstractTableModel::flags( index );

  Qt::ItemFlags flags = QAbstractTableModel::flags( index );
  if ( mResults.at( index.row() ).filter )
  {
    flags = flags & ~( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
  }
  return flags;
}

QHash<int, QByteArray> QgsLocatorModel::roleNames() const
{
  QHash<int, QByteArray> roles;
  roles[ResultDataRole] = "ResultData";
  roles[ResultTypeRole] = "ResultType";
  roles[ResultFilterPriorityRole] = "ResultFilterPriority";
  roles[ResultScoreRole] = "ResultScore";
  roles[ResultFilterNameRole] = "ResultFilterName";
  roles[ResultFilterGroupSortingRole] = "ResultFilterGroupSorting";
  roles[ResultActionsRole] = "ResultContextMenuActions";
  roles[Qt::DisplayRole] = "Text";
  return roles;
}

void QgsLocatorModel::addResult( const QgsLocatorResult &result )
{
  mDeferredClearTimer.stop();
  if ( mDeferredClear )
  {
    mFoundResultsFromFilterNames.clear();
    mFoundResultsFilterGroups.clear();
  }

  const int pos = mResults.size();
  const bool addingFilter = !result.filter->displayName().isEmpty() && !mFoundResultsFromFilterNames.contains( result.filter->name() );
  if ( addingFilter )
    mFoundResultsFromFilterNames << result.filter->name();

  const bool addingGroup = !result.group.isEmpty() && ( !mFoundResultsFilterGroups.contains( result.filter )
                           || !mFoundResultsFilterGroups.value( result.filter ).contains( result.group ) );
  if ( addingGroup )
  {
    if ( !mFoundResultsFilterGroups.contains( result.filter ) )
      mFoundResultsFilterGroups[result.filter] = QStringList();
    mFoundResultsFilterGroups[result.filter] << result.group ;
  }
  if ( mDeferredClear )
  {
    beginResetModel();
    mResults.clear();
  }
  else
    beginInsertRows( QModelIndex(), pos, pos + ( static_cast<int>( addingFilter ) + static_cast<int>( addingGroup ) ) );

  if ( addingFilter )
  {
    Entry entry;
    entry.filterTitle = result.filter->displayName();
    entry.filter = result.filter;
    mResults << entry;
  }
  if ( addingGroup )
  {
    Entry entry;
    entry.filterTitle = result.filter->displayName();
    entry.groupTitle = result.group;
    // the sorting of groups will be achieved by order of adding groups
    // this could be customized by adding the extra info to QgsLocatorResult
    entry.groupSorting = mFoundResultsFilterGroups[result.filter].count();
    entry.filter = result.filter;
    mResults << entry;
  }
  Entry entry;
  entry.result = result;
  // keep the group title empty to allow differecing group title from results
  entry.groupSorting = result.group.isEmpty() ? NoGroup : mFoundResultsFilterGroups[result.filter].indexOf( result.group ) + 1;
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
    const QString nextSearch = mNextRequestedString;
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
  const int leftFilterPriority = sourceModel()->data( left, QgsLocatorModel::ResultFilterPriorityRole ).toInt();
  const int rightFilterPriority  = sourceModel()->data( right, QgsLocatorModel::ResultFilterPriorityRole ).toInt();
  if ( leftFilterPriority != rightFilterPriority )
    return leftFilterPriority < rightFilterPriority;

  // then filter name
  QString leftFilter = sourceModel()->data( left, QgsLocatorModel::ResultFilterNameRole ).toString();
  QString rightFilter = sourceModel()->data( right, QgsLocatorModel::ResultFilterNameRole ).toString();
  if ( leftFilter != rightFilter )
    return QString::localeAwareCompare( leftFilter, rightFilter ) < 0;

  // then make sure filter title or group appears before filter's results
  const int leftTypeRole = sourceModel()->data( left, QgsLocatorModel::ResultTypeRole ).toInt();
  const int rightTypeRole = sourceModel()->data( right, QgsLocatorModel::ResultTypeRole ).toInt();
  if ( leftTypeRole != rightTypeRole )
    return leftTypeRole < rightTypeRole;

  // make sure group title are above
  const int leftGroupRole = sourceModel()->data( left, QgsLocatorModel::ResultFilterGroupSortingRole ).toInt();
  const int rightGroupRole = sourceModel()->data( right, QgsLocatorModel::ResultFilterGroupSortingRole ).toInt();
  if ( leftGroupRole != rightGroupRole )
    return leftGroupRole < rightGroupRole;

  // sort filter's results by score
  const double leftScore = sourceModel()->data( left, QgsLocatorModel::ResultScoreRole ).toDouble();
  const double rightScore = sourceModel()->data( right, QgsLocatorModel::ResultScoreRole ).toDouble();
  if ( !qgsDoubleNear( leftScore, rightScore ) )
    return leftScore > rightScore;

  // lastly sort filter's results by string
  leftFilter = sourceModel()->data( left, Qt::DisplayRole ).toString();
  rightFilter = sourceModel()->data( right, Qt::DisplayRole ).toString();
  return QString::localeAwareCompare( leftFilter, rightFilter ) < 0;
}


