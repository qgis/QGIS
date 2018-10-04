/***************************************************************************
    qgsspatialiteconnection.h
    ---------------------
    begin                : October 2011
    copyright            : (C) 2011 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSSPATIALITECONNECTION_H
#define QGSSPATIALITECONNECTION_H

#include <QStringList>
#include <QObject>
#include <QMutex>

#include "qgsspatialiteutils.h"

extern "C"
{
#include <sqlite3.h>
#include <spatialite/gaiageo.h>
#include <spatialite.h>
}

class QgsSpatiaLiteConnection : public QObject
{
    Q_OBJECT
  public:
    //! Construct a connection. Name can be either stored connection name or a path to the database file
    explicit QgsSpatiaLiteConnection( const QString &name );

    QString path() { return mPath; }

    static QStringList connectionList();
    static void deleteConnection( const QString &name );
    static QString connectionPath( const QString &name );

    typedef struct TableEntry
    {
      TableEntry( const QString &_tableName, const QString &_column, const QString &_type )
        : tableName( _tableName )
        , column( _column )
        , type( _type )
      {}
      QString tableName;
      QString column;
      QString type;
    } TableEntry;

    enum Error
    {
      NoError,
      NotExists,
      FailedToOpen,
      FailedToCheckMetadata,
      FailedToGetTables,
    };

    enum DbLayoutVersion
    {
      LayoutUnknown,
      LayoutLegacy,
      LayoutCurrent,
    };

    Error fetchTables( bool loadGeometrylessTables );

    //! Returns list of tables. fetchTables() function has to be called before
    QList<TableEntry> tables() { return mTables; }

    //! Returns additional error message (if an error occurred before)
    QString errorMessage() { return mErrorMsg; }

    //! Updates the Internal Statistics
    bool updateStatistics();

  protected:
    // SpatiaLite DB open / close
    sqlite3 *openSpatiaLiteDb( const QString &path );
    void closeSpatiaLiteDb( sqlite3 *handle );

    //! Checks if geometry_columns and spatial_ref_sys exist and have expected layout
    int checkHasMetadataTables( sqlite3 *handle );

    /**
     * Inserts information about the spatial tables into mTables
      \returns true if querying of tables was successful, false on error */
    bool getTableInfo( sqlite3 *handle, bool loadGeometrylessTables );

    /**
     * Inserts information about the spatial tables into mTables
     * please note: this method is fully based on the Abstract Interface
     * implemented in libspatialite starting since v.4.0
     *
     * using the Abstract Interface is highly recommended, because all
     * version-dependent implementation details become completely transparent,
     * thus completely freeing the client application to take care of them.
     */
    bool getTableInfoAbstractInterface( sqlite3 *handle, bool loadGeometrylessTables );

    //! Checks if geometry_columns_auth table exists
    bool checkGeometryColumnsAuth( sqlite3 *handle );

    //! Checks if views_geometry_columns table exists
    bool checkViewsGeometryColumns( sqlite3 *handle );

    //! Checks if virts_geometry_columns table exists
    bool checkVirtsGeometryColumns( sqlite3 *handle );

    //! Checks if this layer has been declared HIDDEN
    bool isDeclaredHidden( sqlite3 *handle, const QString &table, const QString &geom );

    //! Checks if this layer is a RasterLite-1 datasource
    bool isRasterlite1Datasource( sqlite3 *handle, const char *table );

    QString mErrorMsg;
    QString mPath; // full path to the database

    QList<TableEntry> mTables;
};

class QgsSqliteHandle
{
    //
    // a class allowing to reuse the same sqlite handle for more layers
    //
  public:
    QgsSqliteHandle( spatialite_database_unique_ptr &&database, const QString &dbPath, bool shared )
      : ref( shared ? 1 : -1 )
      , mDbPath( dbPath )
      , mIsValid( true )
    {
      mDatabase = std::move( database );
    }

    sqlite3 *handle()
    {
      return mDatabase.get();
    }

    QString dbPath() const
    {
      return mDbPath;
    }

    bool isValid() const
    {
      return mIsValid;
    }

    void invalidate()
    {
      mIsValid = false;
    }
    static QgsSqliteHandle *openDb( const QString &dbPath, bool shared = true );
    static bool checkMetadata( sqlite3 *handle );
    static void closeDb( QgsSqliteHandle *&handle );

    /**
     * Will close any cached connection
     * To be called on application exit
     */
    static void closeAll();
    //static void closeDb( QMap < QString, QgsSqliteHandle * >&handlesRO, QgsSqliteHandle * &handle );

  private:
    int ref;
    spatialite_database_unique_ptr mDatabase;
    QString mDbPath;
    bool mIsValid;

    static QMap < QString, QgsSqliteHandle * > sHandles;
    static QMutex sHandleMutex;
};


#endif // QGSSPATIALITECONNECTION_H
