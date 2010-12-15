/***************************************************************************
  sadbtablemodel.h
  A model that holds the tables of a database in a hierarchy where the
  schemas are the root elements that contain the individual tables as children.
  -------------------
    begin                : Dec 2010
    copyright            : (C) 2010 by iAnywhere Solutions, Inc.
    author		 : David DeHaan
    email                : ddehaan at sybase dot com

 This class was copied and modified from QgsDbTableModel because that 
 class is not accessible to QGIS plugins.  Therefore, the author gratefully
 acknowledges the following copyright on the original content:
			 qgsdbtablemodel.cpp
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

#ifndef SADBTABLEMODEL_H
#define SADBTABLEMODEL_H

#include <QStandardItemModel>
class QIcon;
#include "qgis.h"

/**A model that holds the tables of a database in a hierarchy where the
schemas are the root elements that contain the individual tables as children.
The tables have the following columns: Type, Schema, Tablename, Geometry Column, Sql*/
class SaDbTableModel : public QStandardItemModel
{
    Q_OBJECT
  public:
    SaDbTableModel();
    ~SaDbTableModel();
    /**Adds entry for one database table to the model*/
    void addTableEntry( QString type, QString schemaName, QString tableName, QString srid, QString lineInterp, QString geometryColName, QString Sql );
    /**Sets an sql statement that belongs to a cell specified by a model index*/
    void setSql( const QModelIndex& index, const QString& sql );
    /**Sets one or more geometry types to a row. In case of several types, additional rows are inserted.
       This is for tables where the type is dectected later by thread*/
    void setGeometryTypesForTable( const QString& schema, const QString& table, const QString& attribute, const QString& type, const QString& srid, const QString& lineInterp );
    /**Returns the number of tables in the model*/
    int tableCount() const {return mTableCount;}

    enum columns
    {
      dbtmSchema = 0,
      dbtmTable,
      dbtmType,
      dbtmSrid,
      dbtmLineInterp,
      dbtmGeomCol,
      dbtmSql,
      dbtmColumns
    };

  private:
    /**Number of tables in the model*/
    int mTableCount;

    QIcon iconForType( QGis::WkbType type ) const;
    /**Returns qgis wkbtype from database typename*/
    QGis::WkbType qgisTypeFromDbType( const QString& dbType ) const;
};

#endif //SADBTABLEMODEL_H
