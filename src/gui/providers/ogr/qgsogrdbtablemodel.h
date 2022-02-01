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

#include <type_traits>
#include "qgslayeritem.h"
#include "qgis_sip.h"
#include "qgsabstractdbtablemodel.h"


///@cond PRIVATE
#define SIP_NO_FILE

class QgsOgrDbTableModel : public QgsAbstractDbTableModel
{
    Q_OBJECT

  public:

    QgsOgrDbTableModel( QObject *parent = nullptr );

    QStringList columns() const override;
    int defaultSearchColumn() const override;
    bool searchableColumn( int column ) const override;

    //! Sets the geometry type for the table
    void setGeometryTypesForTable( const QString &table, const QString &attribute, const QString &type );

    //! Adds entry for one database table to the model
    void addTableEntry( Qgis::BrowserLayerType layerType, const QString &tableName, const QString &uri, const QString &geometryColName, const QString &geometryType, const QString &sql );

    //! Sets an sql statement that belongs to a cell specified by a model index
    void setSql( const QModelIndex &index, const QString &sql ) override;

    //! Returns the number of tables in the model
    int tableCount() const
    {
      return mTableCount;
    }

    enum Columns
    {
      DbtmTable = 0,
      DbtmType,
      DbtmGeomCol,
      DbtmSql,
    };

    //! Sets the DB full path
    void setPath( const QString &path )
    {
      mPath = path;
    }

  private:
    //! Number of tables in the model
    int mTableCount = 0;
    QString mPath;
    QStringList mColumns;

    QIcon iconForType( QgsWkbTypes::Type type ) const;
    QString displayStringForType( QgsWkbTypes::Type type ) const;
    //! Returns qgis wkbtype from database typename
    QgsWkbTypes::Type qgisTypeFromDbType( const QString &dbType ) const;
};

///@endcond
#endif // QGSOGRDBTABLEMODEL_H
