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

QgsDbFilterProxyModel::QgsDbFilterProxyModel( QObject* parent ): QSortFilterProxyModel( parent )
{

}

QgsDbFilterProxyModel::~QgsDbFilterProxyModel()
{

}

bool QgsDbFilterProxyModel::filterAcceptsRow( int row, const QModelIndex & source_parent ) const
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

void QgsDbFilterProxyModel::_setFilterWildcard( const QString& pattern )
{
  QSortFilterProxyModel::setFilterWildcard( pattern );
  emit layoutChanged();
}

void QgsDbFilterProxyModel::_setFilterRegExp( const QString& pattern )
{
  QSortFilterProxyModel::setFilterRegExp( pattern );
  emit layoutChanged();
}
