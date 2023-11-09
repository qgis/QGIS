/***************************************************************************
                         qgsoracletablemodel.h  -  description
                         -------------------
    begin                : August 2012
    copyright            : (C) 2012 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSORACLETABLEMODEL_H
#define QGSORACLETABLEMODEL_H

#include "qgis.h"
#include "qgsoracleconn.h"
#include "qgsabstractdbtablemodel.h"


class QIcon;

/**
 * A model that holds the tables of a database in a hierarchy where the
 * schemas are the root elements that contain the individual tables as children.
 *
 * The tables have the following columns: Type, Owner, Tablename, Geometry Column, Sql
*/
class QgsOracleTableModel : public QgsAbstractDbTableModel
{
    Q_OBJECT
  public:
    QgsOracleTableModel( QObject *parent = nullptr );

    QStringList columns() const override;
    int defaultSearchColumn() const override;
    bool searchableColumn( int column ) const override;

    //! Adds entry for one database table to the model
    void addTableEntry( const QgsOracleLayerProperty &property );

    //! Sets an sql statement that belongs to a cell specified by a model index
    void setSql( const QModelIndex &index, const QString &sql ) override;

    //! Returns the number of tables in the model
    int tableCount() const { return mTableCount; }

    enum Columns
    {
      DbtmOwner = 0,
      DbtmTable,
      DbtmType,
      DbtmGeomCol,
      DbtmSrid,
      DbtmPkCol,
      DbtmSelectAtId,
      DbtmSql
    };

    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;

    QString layerURI( const QModelIndex &index, const QgsDataSourceUri &connInfo );

  private:
    //! Number of tables in the model
    int mTableCount = 0;
    QStringList mColumns;

};

#endif // QGSORACLETABLEMODEL_H
