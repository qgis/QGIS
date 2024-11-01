/***************************************************************************
                         qgsmssqltablemodel.h  -  description
                         -------------------
    begin                : 2011-10-08
    copyright            : (C) 2011 by Tamas Szekeres
    email                : szekerest at gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMSSQLTABLEMODEL_H
#define QGSMSSQLTABLEMODEL_H

#include "qgsabstractdbtablemodel.h"
#include "qgswkbtypes.h"

//! Layer Property structure
struct QgsMssqlLayerProperty
{
    // MSSQL layer properties
    QString type;
    QString schemaName;
    QString tableName;
    QString geometryColName;
    QStringList pkCols;
    QString srid;
    bool isGeography;
    QString sql;
    bool isView;
};


class QIcon;

/**
 * A model that holds the tables of a database in a hierarchy where the
 * schemas are the root elements that contain the individual tables as children.
 *
 * The tables have the following columns: Type, Schema, Tablename, Geometry Column, Sql
*/
class QgsMssqlTableModel : public QgsAbstractDbTableModel
{
    Q_OBJECT
  public:
    QgsMssqlTableModel( QObject *parent = nullptr );

    QStringList columns() const override;
    int defaultSearchColumn() const override;
    bool searchableColumn( int column ) const override;

    //! Adds entry for one database table to the model
    void addTableEntry( const QgsMssqlLayerProperty &property );

    //! Sets an sql statement that belongs to a cell specified by a model index
    void setSql( const QModelIndex &index, const QString &sql ) override;

    /**
     * Sets one or more geometry types to a row. In case of several types, additional rows are inserted.
     * This is for tables where the type is detected later by thread.
    */
    void setGeometryTypesForTable( QgsMssqlLayerProperty layerProperty );

    //! Returns the number of tables in the model
    int tableCount() const { return mTableCount; }

    enum Columns
    {
      DbtmSchema = 0,
      DbtmTable,
      DbtmType,
      DbtmGeomCol,
      DbtmSrid,
      DbtmPkCol,
      DbtmSelectAtId,
      DbtmSql,
      DbtmView
    };

    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;

    QString layerURI( const QModelIndex &index, const QString &connInfo, bool useEstimatedMetadata, bool disableInvalidGeometryHandling );

    static Qgis::WkbType wkbTypeFromMssql( QString dbType );

    void setConnectionName( const QString &connectionName );

  private:
    //! Number of tables in the model
    int mTableCount = 0;
    QString mConnectionName;
    QStringList mColumns;
};

#endif
