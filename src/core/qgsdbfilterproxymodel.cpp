/***************************************************************************
                         qgsdbfilterproxymodel.cpp  -  description
                         -------------------------
    begin                : Dec 2007
    copyright            : (C) 2007 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdbfilterproxymodel.h"

#include <QStandardItemModel>

QgsDatabaseFilterProxyModel::QgsDatabaseFilterProxyModel( QObject *parent ): QSortFilterProxyModel( parent )
{

}

bool QgsDatabaseFilterProxyModel::filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const
{
  if ( filterAcceptsRowItself( source_row, source_parent ) )
    return true;

//accept if any of the parents is accepted on it's own merits
  QModelIndex parent = source_parent;
  while ( parent.isValid() )
  {
    if ( filterAcceptsRowItself( parent.row(), parent.parent() ) )
      return true;
    parent = parent.parent();
  }

  //accept if any of the children is accepted on it's own merits
  if ( hasAcceptedChildren( source_row, source_parent ) )
  {
    return true;
  }

  return false;
}

bool QgsDatabaseFilterProxyModel::filterAcceptsRowItself( int source_row, const QModelIndex &source_parent ) const
{
  return QSortFilterProxyModel::filterAcceptsRow( source_row, source_parent );
}

bool QgsDatabaseFilterProxyModel::hasAcceptedChildren( int source_row, const QModelIndex &source_parent ) const
{
  QModelIndex item = sourceModel()->index( source_row, 0, source_parent );
  if ( !item.isValid() )
  {
    return false;
  }

//check if there are children
  int childCount = item.model()->rowCount( item );
  if ( childCount == 0 )
    return false;

  for ( int i = 0; i < childCount; ++i )
  {
    if ( filterAcceptsRowItself( i, item ) )
      return true;
    if ( hasAcceptedChildren( i, item ) )
      return true;
  }

  return false;
}




void QgsDatabaseFilterProxyModel::_setFilterWildcard( const QString &pattern )
{
  QSortFilterProxyModel::setFilterWildcard( pattern );
  emit layoutChanged();
}

void QgsDatabaseFilterProxyModel::_setFilterRegExp( const QString &pattern )
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  QSortFilterProxyModel::setFilterRegExp( pattern );
#else
  QSortFilterProxyModel::setFilterRegularExpression( pattern );
#endif
  emit layoutChanged();
}
