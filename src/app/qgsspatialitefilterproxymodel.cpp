/***************************************************************************
                         qgsspatialitefilterproxymodel.cpp  -  description
                         -------------------------
    begin                : Dec 2008
    copyright            : (C) 2008 by Sandro Furieri
    email                : a.furieri@lqt.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsspatialitefilterproxymodel.h"

QgsSpatiaLiteFilterProxyModel::QgsSpatiaLiteFilterProxyModel( QObject * parent ): QSortFilterProxyModel( parent )
{

}

QgsSpatiaLiteFilterProxyModel::~QgsSpatiaLiteFilterProxyModel()
{

}

bool QgsSpatiaLiteFilterProxyModel::filterAcceptsRow( int row, const QModelIndex & source_parent ) const
{
  //if parent is valid, we have a toplevel item that should be always shown
  if ( !source_parent.isValid() )
  {
    return true;
  }
  //else we have a row that describes a table and that
  //should be tested using the given wildcard/regexp
  return QSortFilterProxyModel::filterAcceptsRow( row, source_parent );
}

void QgsSpatiaLiteFilterProxyModel::_setFilterWildcard( const QString & pattern )
{
  QSortFilterProxyModel::setFilterWildcard( pattern );
  emit layoutChanged();
}

void QgsSpatiaLiteFilterProxyModel::_setFilterRegExp( const QString & pattern )
{
  QSortFilterProxyModel::setFilterRegExp( pattern );
  emit layoutChanged();
}
