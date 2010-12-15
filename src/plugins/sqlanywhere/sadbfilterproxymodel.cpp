/***************************************************************************
  sadbfilterproxymodel.cpp
  A class that implements a custom filter and can be used as a proxy for SaDbTableModel
  -------------------
    begin                : Dec 2010
    copyright            : (C) 2010 by iAnywhere Solutions, Inc.
    author		 : David DeHaan
    email                : ddehaan at sybase dot com

 This class was copied and modified from QgsDbFilterProxyModel because that 
 class is not accessible to QGIS plugins.  Therefore, the author gratefully
 acknowledges the following copyright on the original content:
			 qgsdbfilterproxymodel.cpp
    begin                : Dec 2007
    copyright            : (C) 2007 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*  $Id$ */


#include "sadbfilterproxymodel.h"

SaDbFilterProxyModel::SaDbFilterProxyModel( QObject* parent ): QSortFilterProxyModel( parent )
{

}

SaDbFilterProxyModel::~SaDbFilterProxyModel()
{

}

bool SaDbFilterProxyModel::filterAcceptsRow( int row, const QModelIndex & source_parent ) const
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

void SaDbFilterProxyModel::_setFilterWildcard( const QString& pattern )
{
  QSortFilterProxyModel::setFilterWildcard( pattern );
  emit layoutChanged();
}

void SaDbFilterProxyModel::_setFilterRegExp( const QString& pattern )
{
  QSortFilterProxyModel::setFilterRegExp( pattern );
  emit layoutChanged();
}
