/***************************************************************************
  sadbfilterproxymodel.h
  A class that implements a custom filter and can be used as a proxy for SaDbTableModel
  -------------------
    begin                : Dec 2010
    copyright            : (C) 2010 by iAnywhere Solutions, Inc.
    author		 : David DeHaan
    email                : ddehaan at sybase dot com

 This class was copied and modified from QgsDbFilterProxyModel because that 
 class is not accessible to QGIS plugins.  Therefore, the author gratefully
 acknowledges the following copyright on the original content:
			 qgsdbfilterproxymodel.h
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

#ifndef SADBFILTERPROXYMODEL_H
#define SADBFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

/**A class that implements a custom filter and can be used
 as a proxy for SaDbTableModel*/
class SaDbFilterProxyModel: public QSortFilterProxyModel
{
  public:
    SaDbFilterProxyModel( QObject* parent = 0 );
    ~SaDbFilterProxyModel();
    /**Calls QSortFilterProxyModel::setFilterWildcard and triggers update*/
    void _setFilterWildcard( const QString& pattern );
    /**Calls QSortFilterProxyModel::setFilterRegExp and triggers update*/
    void _setFilterRegExp( const QString& pattern );

  protected:
    virtual bool filterAcceptsRow( int row, const QModelIndex & source_parent ) const;
};

#endif // SADBFILTERPROXYMODEL_H
