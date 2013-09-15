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



QgsPluginSortFilterProxyModel::QgsPluginSortFilterProxyModel( QObject *parent ) : QSortFilterProxyModel( parent )
{
}



bool QgsPluginSortFilterProxyModel::filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const
{
  QModelIndex inx = sourceModel()->index( sourceRow, 0, sourceParent );

  if ( ! sourceModel()->data( inx, SPACER_ROLE ).toString().isEmpty() )
  {
    // it's a status spacer.
    // TODO: the condition below is only suitable for status spacers
    return ( filterByStatus( inx ) &&  mAcceptedStatuses.count() > 2 && sourceModel()->data( inx, SPACER_ROLE ).toString() == mAcceptedSpacers );
  }

  return ( filterByStatus( inx ) && sourceModel()->data( inx, filterRole() ).toString().contains( filterRegExp() ) );
}



void QgsPluginSortFilterProxyModel::setAcceptedStatuses( QStringList statuses )
{
  mAcceptedStatuses = statuses;
  invalidateFilter();
}



void QgsPluginSortFilterProxyModel::setAcceptedSpacers( QString spacers )
{
  mAcceptedSpacers = spacers;
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

  QString status = sourceModel()->data( index, PLUGIN_STATUS_ROLE ).toString();
  if ( status.endsWith( "Z" ) ) status.chop( 1 );
  if ( ! mAcceptedStatuses.isEmpty()
       && ! mAcceptedStatuses.contains( "invalid" )
       && ! mAcceptedStatuses.contains( status ) )
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
  for ( int i = 0; i < sourceModel()->rowCount(); ++i )
  {
    QModelIndex idx = sourceModel()->index( i, 0 );
    if ( filterByStatus( idx ) && sourceModel()->data( idx, SPACER_ROLE ).toString().isEmpty() )
    {
      result++ ;
    }
  }
  return result;
}



void QgsPluginSortFilterProxyModel::sortPluginsByName( )
{
  setAcceptedSpacers();
  sort( 0, Qt::AscendingOrder );
  setSortRole( Qt::DisplayRole );
}



void QgsPluginSortFilterProxyModel::sortPluginsByDownloads( )
{
  setAcceptedSpacers();
  sort( 0, Qt::DescendingOrder );
  setSortRole( PLUGIN_DOWNLOADS_ROLE );
}



void QgsPluginSortFilterProxyModel::sortPluginsByVote( )
{
  setAcceptedSpacers();
  sort( 0, Qt::DescendingOrder );
  setSortRole( PLUGIN_VOTE_ROLE );
}



void QgsPluginSortFilterProxyModel::sortPluginsByStatus( )
{
  setAcceptedSpacers( "status" );
  sort( 0, Qt::DescendingOrder );
  setSortRole( PLUGIN_STATUS_ROLE );
}
