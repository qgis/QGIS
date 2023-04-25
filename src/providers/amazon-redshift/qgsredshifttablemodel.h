/***************************************************************************
   qgsredshifttablemodel.h
   --------------------------------------
   Date      : 16.02.2021
   Copyright : (C) 2021 Amazon Inc. or its affiliates
   Author    : Marcel Bezdrighin
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#ifndef QGSREDSHIFTTABLEMODEL_H
#define QGSREDSHIFTTABLEMODEL_H

#include "qgsabstractdbtablemodel.h"
#include "qgsredshiftconn.h"
#include "qgswkbtypes.h"

class QIcon;

/**
 * A model that holds the tables of a database in a hierarchy where the
 * schemas are the root elements that contain the individual tables as children.
 *
 * The tables have the following columns: Type, Schema, Tablename, Geometry Column, Sql
*/
class QgsRedshiftTableModel : public QgsAbstractDbTableModel
{
    Q_OBJECT
  public:
    QgsRedshiftTableModel( QObject *parent = nullptr );

    //! Adds entry for one database table to the model
    void addTableEntry( const QgsRedshiftLayerProperty &property );

    //! Sets an sql statement that belongs to a cell specified by a model index
    void setSql( const QModelIndex &index, const QString &sql ) override;

    //! Returns the number of tables in the model
    int tableCount() const
    {
      return mTableCount;
    }

    QStringList columns() const override;
    int defaultSearchColumn() const override;

    enum Columns
    {
      DbtmDatabase = 0,
      DbtmSchema,
      DbtmTable,
      DbtmComment,
      DbtmGeomCol,
      DbtmGeomType, // Data type (geometry, geography, ...)
      DbtmType,     // Spatial type (point, line, polygon, ...)
      DbtmSrid,
      DbtmPkCol,
      DbtmSelectAtId,
      DbtmCheckPkUnicity,
      DbtmSql
    };

    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;

    QString layerURI( const QModelIndex &index, const QString &connInfo, bool useEstimatedMetadata );

    static QIcon iconForWkbType( Qgis::WkbType type );

    void setConnectionName( const QString &connName )
    {
      mConnName = connName;
    }

  private:
    //! Number of tables in the model
    int mTableCount = 0;
    //! connection name
    QString mConnName;
    QStringList mColumns;
};

#endif // QGSREDSHIFTTABLEMODEL_H
