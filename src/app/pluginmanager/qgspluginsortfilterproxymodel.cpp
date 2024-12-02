/***************************************************************************
    qgspluginsortfilterproxymodel.cpp
     --------------------------------------
    Date                 : 20-May-2013
    Copyright            : (C) 2013 by Borys Jurgiel
    Email                : info at borysjurgiel dot pl
****************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspluginsortfilterproxymodel.h"
#include "moc_qgspluginsortfilterproxymodel.cpp"


QgsPluginSortFilterProxyModel::QgsPluginSortFilterProxyModel( QObject *parent )
  : QSortFilterProxyModel( parent )
{
}


bool QgsPluginSortFilterProxyModel::filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const
{
  QModelIndex inx = sourceModel()->index( sourceRow, 0, sourceParent );

  if ( !sourceModel()->data( inx, SPACER_ROLE ).toString().isEmpty() )
  {
    // it's a status spacer.
    // TODO: the condition below is only suitable for status spacers
    return ( filterByStatus( inx ) && mAcceptedStatuses.count() > 2 && sourceModel()->data( inx, SPACER_ROLE ).toString() == mAcceptedSpacers );
  }

  return ( filterByStatus( inx ) && filterByPhrase( inx ) );
}


void QgsPluginSortFilterProxyModel::setAcceptedStatuses( const QStringList &statuses )
{
  mAcceptedStatuses = statuses;
  invalidateFilter();
}


void QgsPluginSortFilterProxyModel::setAcceptedSpacers( const QString &spacers )
{
  mAcceptedSpacers = spacers;
  invalidateFilter();
}


bool QgsPluginSortFilterProxyModel::filterByStatus( QModelIndex &index ) const
{
  if ( mAcceptedStatuses.contains( QStringLiteral( "invalid" ) )
       && sourceModel()->data( index, PLUGIN_ERROR_ROLE ).toString().isEmpty() )
  {
    // Don't accept if the "invalid" filter is set and the plugin is OK
    return false;
  }

  QString status = sourceModel()->data( index, PLUGIN_STATUS_ROLE ).toString();
  const QString statusexp = sourceModel()->data( index, PLUGIN_STATUSEXP_ROLE ).toString();
  if ( status.endsWith( 'Z' ) )
    status.chop( 1 );
  if ( !mAcceptedStatuses.isEmpty()
       && !mAcceptedStatuses.contains( QStringLiteral( "invalid" ) )
       && !( mAcceptedStatuses.contains( status ) || mAcceptedStatuses.contains( statusexp ) ) )
  {
    // Don't accept if the status doesn't match
    return false;
  }

  // Otherwise, let the item go.
  return true;
}


bool QgsPluginSortFilterProxyModel::filterByPhrase( QModelIndex &index ) const
{
  switch ( filterRole() )
  {
    case PLUGIN_TAGS_ROLE:
      // search in tags only
      return sourceModel()->data( index, PLUGIN_TAGS_ROLE ).toString().contains( filterRegularExpression() );
    case 0:
    {
      const QRegularExpression regEx = filterRegularExpression();
      // full search: name + description + tags + author
      return sourceModel()->data( index, PLUGIN_DESCRIPTION_ROLE ).toString().contains( regEx )
             || sourceModel()->data( index, PLUGIN_AUTHOR_ROLE ).toString().contains( regEx )
             || sourceModel()->data( index, Qt::DisplayRole ).toString().contains( regEx )
             || sourceModel()->data( index, PLUGIN_TAGS_ROLE ).toString().contains( regEx );
    }
    default:
      // unknown filter mode, return nothing
      return false;
  }
}


int QgsPluginSortFilterProxyModel::countWithCurrentStatus()
{
  int result = 0;
  for ( int i = 0; i < sourceModel()->rowCount(); ++i )
  {
    QModelIndex idx = sourceModel()->index( i, 0 );
    if ( filterByStatus( idx ) && sourceModel()->data( idx, SPACER_ROLE ).toString().isEmpty() )
    {
      result++;
    }
  }
  return result;
}


void QgsPluginSortFilterProxyModel::sortPluginsByName()
{
  setAcceptedSpacers();
  sort( 0, Qt::AscendingOrder );
  setSortRole( Qt::DisplayRole );
}


void QgsPluginSortFilterProxyModel::sortPluginsByDownloads()
{
  setAcceptedSpacers();
  sort( 0, Qt::DescendingOrder );
  setSortRole( PLUGIN_DOWNLOADS_ROLE );
}


void QgsPluginSortFilterProxyModel::sortPluginsByVote()
{
  setAcceptedSpacers();
  sort( 0, Qt::DescendingOrder );
  setSortRole( PLUGIN_VOTE_ROLE );
}


void QgsPluginSortFilterProxyModel::sortPluginsByStatus()
{
  setAcceptedSpacers( QStringLiteral( "status" ) );
  sort( 0, Qt::DescendingOrder );
  setSortRole( PLUGIN_STATUS_ROLE );
}


void QgsPluginSortFilterProxyModel::sortPluginsByDateCreated()
{
  setAcceptedSpacers();
  sort( 0, Qt::DescendingOrder );
  setSortRole( PLUGIN_CREATE_DATE );
}


void QgsPluginSortFilterProxyModel::sortPluginsByDateUpdated()
{
  setAcceptedSpacers();
  sort( 0, Qt::DescendingOrder );
  setSortRole( PLUGIN_UPDATE_DATE );
}


bool QgsPluginSortFilterProxyModel::lessThan( const QModelIndex &source_left, const QModelIndex &source_right ) const
{
  // Always move deprecated plugins to bottom, regardless of the sort order.
  const bool isLeftDepreciated = sourceModel()->data( source_left, PLUGIN_ISDEPRECATED_ROLE ).toString() == QLatin1String( "true" );
  const bool isRightDepreciated = sourceModel()->data( source_right, PLUGIN_ISDEPRECATED_ROLE ).toString() == QLatin1String( "true" );
  if ( isRightDepreciated && !isLeftDepreciated )
  {
    return sortOrder() == Qt::AscendingOrder ? true : false;
  }
  else if ( isLeftDepreciated && !isRightDepreciated )
  {
    return sortOrder() == Qt::AscendingOrder ? false : true;
  }
  else
  {
    return QSortFilterProxyModel::lessThan( source_left, source_right );
  }
}
