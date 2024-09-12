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
  return mResults.count();
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
      switch ( static_cast<Column>( index.column() ) )
      {
        case Name:
        {
          switch ( mResults.at( index.row() ).type )
          {
            case EntryType::Filter:
              return mResults.at( index.row() ).filterTitle;

            case EntryType::Group:
            {
              QString groupTitle = mResults.at( index.row() ).groupTitle;
              groupTitle.prepend( "  " );
              return groupTitle;
            }

            case EntryType::Result:
              return mResults.at( index.row() ).result.displayString;
          }
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
      switch ( static_cast<Column>( index.column() ) )
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

    case static_cast< int >( CustomRole::ResultData ):
      if ( !mResults.at( index.row() ).filter )
        return QVariant::fromValue( mResults.at( index.row() ).result );
      else
        return QVariant();

    case static_cast< int >( CustomRole::ResultType ):
      return static_cast<int>( mResults.at( index.row() ).type );

    case static_cast< int >( CustomRole::ResultScore ):
      if ( mResults.at( index.row() ).filter )
        return 0;
      else
        return ( mResults.at( index.row() ).result.score );

    case static_cast< int >( CustomRole::ResultFilterPriority ):
      if ( !mResults.at( index.row() ).filter )
        return mResults.at( index.row() ).result.filter->priority();
      else
        return mResults.at( index.row() ).filter->priority();

    case static_cast< int >( CustomRole::ResultFilterName ):
      if ( !mResults.at( index.row() ).filter )
        return mResults.at( index.row() ).result.filter->displayName();
      else
        return mResults.at( index.row() ).filterTitle;

    case static_cast< int >( CustomRole::ResultFilterGroupTitle ):
      return mResults.at( index.row() ).groupTitle;

    case static_cast< int >( CustomRole::ResultFilterGroupScore ):
      return mResults.at( index.row() ).groupScore;

    case static_cast< int >( CustomRole::ResultActions ):
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
  roles[static_cast< int >( CustomRole::ResultData )] = "ResultData";
  roles[static_cast< int >( CustomRole::ResultType )] = "ResultType";
  roles[static_cast< int >( CustomRole::ResultFilterPriority )] = "ResultFilterPriority";
  roles[static_cast< int >( CustomRole::ResultScore )] = "ResultScore";
  roles[static_cast< int >( CustomRole::ResultFilterName )] = "ResultFilterName";
  roles[static_cast< int >( CustomRole::ResultFilterGroupSorting )] = "ResultFilterGroupSorting"; // Deprecated
  roles[static_cast< int >( CustomRole::ResultFilterGroupTitle )] = "ResultFilterGroupTitle";
  roles[static_cast< int >( CustomRole::ResultFilterGroupScore )] = "ResultFilterGroupScore";
  roles[static_cast< int >( CustomRole::ResultActions )] = "ResultContextMenuActions";
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
                           || !mFoundResultsFilterGroups.value( result.filter ).contains( std::pair( result.group, result.groupScore ) ) );
  if ( addingGroup )
  {
    if ( !mFoundResultsFilterGroups.contains( result.filter ) )
      mFoundResultsFilterGroups[result.filter] = QList<std::pair<QString, double>>();

    mFoundResultsFilterGroups[result.filter] << std::pair( result.group, result.groupScore );
  }

  if ( mDeferredClear )
  {
    beginResetModel();
    mResults.clear();
  }
  else
  {
    beginInsertRows( QModelIndex(), pos, pos + ( static_cast<int>( addingFilter ) + static_cast<int>( addingGroup ) ) );
  }

  const double groupScore = result.group.isEmpty() ? NoGroup : result.groupScore;
  if ( addingFilter )
  {
    Entry entry;
    entry.type = EntryType::Filter;
    entry.filterTitle = result.filter->displayName();
    entry.filter = result.filter;
    mResults << entry;
  }
  if ( addingGroup )
  {
    Entry entry;
    entry.type = EntryType::Group;
    entry.filterTitle = result.filter->displayName();
    entry.groupTitle = result.group;
    entry.groupScore = groupScore;
    entry.filter = result.filter;
    mResults << entry;
  }
  Entry entry;
  entry.type = EntryType::Result;
  entry.filter = result.filter;
  entry.filterTitle = result.filter->displayName();
  entry.result = result;
  entry.groupTitle = result.group;
  entry.groupScore = groupScore;
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
  // sort by filter priority
  const int leftFilterPriority = sourceModel()->data( left, static_cast< int >( QgsLocatorModel::CustomRole::ResultFilterPriority ) ).toInt();
  const int rightFilterPriority  = sourceModel()->data( right, static_cast< int >( QgsLocatorModel::CustomRole::ResultFilterPriority ) ).toInt();
  if ( leftFilterPriority != rightFilterPriority )
    return leftFilterPriority < rightFilterPriority;

  // sort by filter name
  QString leftFilter = sourceModel()->data( left, static_cast< int >( QgsLocatorModel::CustomRole::ResultFilterName ) ).toString();
  QString rightFilter = sourceModel()->data( right, static_cast< int >( QgsLocatorModel::CustomRole::ResultFilterName ) ).toString();
  if ( leftFilter != rightFilter )
    return QString::localeAwareCompare( leftFilter, rightFilter ) < 0;

  // make sure filter title appears before
  const int leftTypeRole = sourceModel()->data( left, static_cast< int >( QgsLocatorModel::CustomRole::ResultType ) ).toInt();
  const int rightTypeRole = sourceModel()->data( right, static_cast< int >( QgsLocatorModel::CustomRole::ResultType ) ).toInt();
  if ( leftTypeRole != rightTypeRole && ( leftTypeRole == 0 || rightTypeRole == 0 ) )
    return leftTypeRole < rightTypeRole;

  // sort by group score
  const double leftGroupScoreRole = sourceModel()->data( left, static_cast< double >( QgsLocatorModel::CustomRole::ResultFilterGroupScore ) ).toDouble();
  const double rightGroupScoreRole = sourceModel()->data( right, static_cast< double >( QgsLocatorModel::CustomRole::ResultFilterGroupScore ) ).toDouble();
  if ( leftGroupScoreRole != rightGroupScoreRole )
    return leftGroupScoreRole > rightGroupScoreRole;

  // sort by group name alphabetically
  QString leftGroupTitle = sourceModel()->data( left, static_cast< int >( QgsLocatorModel::CustomRole::ResultFilterGroupTitle ) ).toString();
  QString rightGroupTitle = sourceModel()->data( right, static_cast< int >( QgsLocatorModel::CustomRole::ResultFilterGroupTitle ) ).toString();
  if ( leftGroupTitle != rightGroupTitle )
    return QString::localeAwareCompare( leftGroupTitle, rightGroupTitle ) < 0;

  // make sure group appears before filter's results
  if ( leftTypeRole != rightTypeRole )
    return leftTypeRole < rightTypeRole;

  // sort results by score
  const double leftScore = sourceModel()->data( left, static_cast< int >( QgsLocatorModel::CustomRole::ResultScore ) ).toDouble();
  const double rightScore = sourceModel()->data( right, static_cast< int >( QgsLocatorModel::CustomRole::ResultScore ) ).toDouble();
  if ( !qgsDoubleNear( leftScore, rightScore ) )
    return leftScore > rightScore;

  // sort results alphabetically
  leftFilter = sourceModel()->data( left, Qt::DisplayRole ).toString();
  rightFilter = sourceModel()->data( right, Qt::DisplayRole ).toString();
  return QString::localeAwareCompare( leftFilter, rightFilter ) < 0;
}

