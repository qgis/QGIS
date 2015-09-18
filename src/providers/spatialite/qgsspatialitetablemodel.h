/***************************************************************************
                         qgsspatialitetablemodel.h  -  description
                         -------------------
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

#include <QStandardItemModel>
class QIcon;
#include "qgis.h"

/** A model that holds the tables of a database in a hierarchy where the
SQLite DB is the root elements that contain the individual tables as children.
The tables have the following columns: Type, Tablename, Geometry Column*/
class QgsSpatiaLiteTableModel: public QStandardItemModel
{
  Q_OBJECT public:

    QgsSpatiaLiteTableModel();
    ~QgsSpatiaLiteTableModel();
    /** Adds entry for one database table to the model*/
    void addTableEntry( QString type, QString tableName, QString geometryColName, QString sql );
    /** Sets an sql statement that belongs to a cell specified by a model index*/
    void setSql( const QModelIndex& index, const QString& sql );
    /** Sets one or more geometry types to a row. In case of several types, additional rows are inserted.
       This is for tables where the type is dectected later by thread*/
    void setGeometryTypesForTable( const QString & table, const QString & attribute, const QString & type );
    /** Returns the number of tables in the model*/
    int tableCount() const
    {
      return mTableCount;
    }
    /** Sets the SQLite DB full path*/
    void setSqliteDb( const QString & dbName )
    {
      mSqliteDb = dbName;
    }

  private:
    /** Number of tables in the model*/
    int mTableCount;
    QString mSqliteDb;

    QIcon iconForType( QGis::WkbType type ) const;
    QString displayStringForType( QGis::WkbType type ) const;
    /** Returns qgis wkbtype from database typename*/
    QGis::WkbType qgisTypeFromDbType( const QString & dbType ) const;
};
