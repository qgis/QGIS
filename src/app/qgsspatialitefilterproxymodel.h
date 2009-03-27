/***************************************************************************
                         qgsspatialitefilterproxymodel.h  -  description
                         -----------------------
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

#ifndef QGSSPATIALITEFILTERPROXYMODEL_H
#define QGSSPATIALITEFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

/**A class that implements a custom filter and can be used
 as a proxy for QgsSpatiaLiteTableModel*/
class QgsSpatiaLiteFilterProxyModel: public QSortFilterProxyModel
{
  public:
    QgsSpatiaLiteFilterProxyModel( QObject * parent = 0 );
    ~QgsSpatiaLiteFilterProxyModel();
    /**Calls QSortFilterProxyModel::setFilterWildcard and triggers update*/
    void _setFilterWildcard( const QString & pattern );
    /**Calls QSortFilterProxyModel::setFilterRegExp and triggers update*/
    void _setFilterRegExp( const QString & pattern );

  protected:
    virtual bool filterAcceptsRow( int row, const QModelIndex & source_parent ) const;
};

#endif
