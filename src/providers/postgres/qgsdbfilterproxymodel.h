/***************************************************************************
                         qgsdbfilterproxymodel.h  -  description
                         -----------------------
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

#ifndef QGSDBFILTERPROXYMODEL_H
#define QGSDBFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

/**A class that implements a custom filter and can be used
 as a proxy for QgsDbTableModel*/
class CORE_EXPORT QgsDbFilterProxyModel: public QSortFilterProxyModel
{
  public:
    QgsDbFilterProxyModel( QObject* parent = 0 );
    ~QgsDbFilterProxyModel();
    /**Calls QSortFilterProxyModel::setFilterWildcard and triggers update*/
    void _setFilterWildcard( const QString& pattern );
    /**Calls QSortFilterProxyModel::setFilterRegExp and triggers update*/
    void _setFilterRegExp( const QString& pattern );

  protected:
    virtual bool filterAcceptsRow( int row, const QModelIndex & source_parent ) const;
};

#endif
