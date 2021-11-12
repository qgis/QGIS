/***************************************************************************
   qgshanatablemodel.h
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maxim Rylov
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#ifndef QGSHANATABLEMODEL_H
#define QGSHANATABLEMODEL_H

#include "qgis.h"
#include "qgswkbtypes.h"
#include "qgsabstractdbtablemodel.h"

//! Schema properties structure
struct QgsHanaSchemaProperty
{
  QString name;
  QString owner;
};

//! Layer Property structure
struct QgsHanaLayerProperty
{
  QString     schemaName;
  QString     tableName;
  QString     tableComment;
  QString     geometryColName;
  QgsWkbTypes::Type type;
  QStringList pkCols;
  int         srid;
  QString     sql;
  bool        isView = false;
  bool        isUnique = false;
  bool        isValid = false;
  QString     errorMessage;

  QString defaultName() const
  {
    QString ret = tableName;
    if ( !isUnique && !geometryColName.isEmpty() )
      ret += " [" + geometryColName + "]";
    return ret;
  }

  bool isGeometryValid() const { return type != QgsWkbTypes::Unknown && srid >= 0; }
};

class QIcon;

/**
 * A model that holds the tables of a database in a hierarchy where the
 * schemas are the root elements that contain the individual tables as children.
 *
 * The tables have the following columns: Type, Schema, Tablename, Geometry Column, Sql
*/
class QgsHanaTableModel : public QgsAbstractDbTableModel
{
    Q_OBJECT
  public:
    QgsHanaTableModel( QObject *parent = nullptr );

    QStringList columns() const override;
    int defaultSearchColumn() const override;
    bool searchableColumn( int column ) const override;

    //! Adds entry for one database table to the model
    void addTableEntry( const QString &connName, const QgsHanaLayerProperty &property );

    //! Sets an sql statement that belongs to a cell specified by a model index
    void setSql( const QModelIndex &index, const QString &sql ) override;

    //! Returns the number of tables in the model
    int tableCount() const { return mTableCount; }

    enum Columns
    {
      DbtmSchema = 0,
      DbtmTable,
      DbtmComment,
      DbtmGeomCol,
      DbtmGeomType,
      DbtmSrid,
      DbtmPkCol,
      DbtmSelectAtId,
      DbtmSql
    };

    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;

    QString layerURI( const QModelIndex &index, const QString &connName, const QString &connInfo );

    static QIcon iconForWkbType( QgsWkbTypes::Type type );

  private:
    //! Number of tables in the model
    int mTableCount = 0;
    QStringList mColumns;

};

#endif  // QGSHANATABLEMODEL_H
