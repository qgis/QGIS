/***************************************************************************
  qgsogrdbtablemodel.h - QgsOgrDbTableModel

 ---------------------
 begin                : 5.9.2017
 copyright            : (C) 2017 by Alessandro Pasotti
 email                : apasotti at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSOGRDBTABLEMODEL_H
#define QGSOGRDBTABLEMODEL_H

#include "qgis.h"
#include "qgsdataitem.h"

#include <QObject>
#include <QStandardItemModel>
#include <type_traits>

class QgsOgrDbTableModel : public QStandardItemModel
{
    Q_OBJECT

  public:

    QgsOgrDbTableModel();

    //! Set the geometry type for the table
    void setGeometryTypesForTable( const QString &table, const QString &attribute, const QString &type );

    //! Adds entry for one database table to the model
    void addTableEntry( const QgsLayerItem::LayerType &layerType, const QString &tableName, const QString &uri, const QString &geometryColName, const QString &geometryType, const QString &sql );

    //! Sets an sql statement that belongs to a cell specified by a model index
    void setSql( const QModelIndex &index, const QString &sql );

    //! Returns the number of tables in the model
    int tableCount() const
    {
      return mTableCount;
    }

    //! Sets the DB full path
    void setPath( const QString &path )
    {
      mPath = path;
    }

  private:
    //! Number of tables in the model
    int mTableCount = 0;
    QString mPath;

    QIcon iconForType( QgsWkbTypes::Type type ) const;
    QString displayStringForType( QgsWkbTypes::Type type ) const;
    //! Returns qgis wkbtype from database typename
    QgsWkbTypes::Type qgisTypeFromDbType( const QString &dbType ) const;
};
#endif // QGSOGRDBTABLEMODEL_H
