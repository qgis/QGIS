/***************************************************************************
                   qgsdamengtablemodel.h  -  description
                             ------------------
    begin                : 2025/01/14
    copyright            : ( C ) 2025 by Haiyang Zhao
    email                : zhaohaiyang@dameng.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   ( at your option ) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDAMENGTABLEMODEL_H
#define QGSDAMENGTABLEMODEL_H

#include "qgsabstractdbtablemodel.h"
#include "qgswkbtypes.h"
#include "qgsdamengconn.h"

class QIcon;

/**
 * A model that holds the tables of a database in a hierarchy where the
 * schemas are the root elements that contain the individual tables as children.
 *
 * The tables have the following columns: Type, Schema, Tablename, Geometry Column, Sql
*/
class QgsDamengTableModel : public QgsAbstractDbTableModel
{
    Q_OBJECT
  public:
    QgsDamengTableModel( QObject *parent = nullptr );

    //! Adds entry for one database table to the model
    void addTableEntry( const QgsDamengLayerProperty &property );

    //! Sets an sql statement that belongs to a cell specified by a model index
    void setSql( const QModelIndex &index, const QString &sql );

    //! Returns the number of tables in the model
    int tableCount() const { return mTableCount; }

    QStringList columns() const override;
    int defaultSearchColumn() const override;
    bool searchableColumn(int column) const override;

    enum Columns
    {
      DbtmSchema = 0,
      DbtmTable,
      DbtmComment,
      DbtmGeomCol,
      DbtmGeomType, // Data type ( geometry, geography, topogeometry, ...)
      DbtmType, // Spatial type ( point, line, polygon, ...)
      DbtmSrid,
      DbtmPkCol,
      DbtmSelectAtId,
      DbtmCheckPkUnicity,
      DbtmSql,
      DbtmColumns
    };

    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;

    QString layerURI( const QModelIndex &index, const QString &connInfo, bool useEstimatedMetadata );

    void setConnectionName( const QString &connName ) { mConnName = connName;  }

  private:
    //! Number of tables in the model
    int mTableCount = 0;
    //! connection name
    QString mConnName;
    QStringList mColumns;
};

#endif // QGSDAMENGTABLEMODEL_H
