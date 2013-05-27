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
#include "qgslogger.h"

QgsPluginSortFilterProxyModel::QgsPluginSortFilterProxyModel( QObject *parent ) : QSortFilterProxyModel( parent )
{
}



bool QgsPluginSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
  QModelIndex inx = sourceModel()->index(sourceRow, 0, sourceParent);

  return ( filterByStatus( inx ) && sourceModel()->data( inx, filterRole() ).toString().contains( filterRegExp() ) );
}



void QgsPluginSortFilterProxyModel::setAcceptedStatuses( QStringList statuses )
{
  mAcceptedStatuses = statuses;
  invalidateFilter();
}



bool QgsPluginSortFilterProxyModel::filterByStatus( QModelIndex &index ) const
{
  if ( mAcceptedStatuses.contains( "invalid" )
    && sourceModel()->data( index, PLUGIN_ERROR_ROLE ).toString().isEmpty() )
  {
    // Don't accept if the "invalid" filter is set and the plugin is ok
    return false;
  }

  if ( ! mAcceptedStatuses.isEmpty()
    && ! mAcceptedStatuses.contains( "invalid" )
    && ! mAcceptedStatuses.contains( sourceModel()->data(index, PLUGIN_STATUS_ROLE).toString() ) )
  {
    // Don't accept if the status doesn't match
    return false;
  }

  // Otherwise, let the item go.
  return true;
}



int QgsPluginSortFilterProxyModel::countWithCurrentStatus( )
{
  int result = 0;
  for ( int i=0; i < sourceModel()->rowCount(); ++i )
  {
    QModelIndex idx = sourceModel()->index( i, 0 );
    if ( filterByStatus( idx ) )
    {
      result++ ;
    }
  }
  return result;
}



void QgsPluginSortFilterProxyModel::sortPluginsByName( )
{
  sort( 0, Qt::AscendingOrder );
  setSortRole( Qt::DisplayRole );
}



void QgsPluginSortFilterProxyModel::sortPluginsByDownloads( )
{
  sort( 0, Qt::DescendingOrder );
  setSortRole( PLUGIN_DOWNLOADS_ROLE );
}



void QgsPluginSortFilterProxyModel::sortPluginsByVote( )
{
  sort( 0, Qt::DescendingOrder );
  setSortRole( PLUGIN_VOTE_ROLE );
}



void QgsPluginSortFilterProxyModel::sortPluginsByStatus( )
{
  sort( 0, Qt::DescendingOrder );
  setSortRole( PLUGIN_STATUS_ROLE );
}



void QgsPluginSortFilterProxyModel::sortPluginsByRepository( )
{
  sort( 0, Qt::DescendingOrder );
  setSortRole( PLUGIN_REPOSITORY_ROLE );
}
