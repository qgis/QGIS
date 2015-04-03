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

extern "C"
{
#include <sqlite3.h>
#include <spatialite/gaiageo.h>
#include <spatialite.h>
}

class QgsSpatiaLiteConnection : public QObject
{
    Q_OBJECT;
  public:
    /** construct a connection. Name can be either stored connection name or a path to the database file */
    QgsSpatiaLiteConnection( QString name );

    QString path() { return mPath; }

    static QStringList connectionList();
    static void deleteConnection( QString name );
    static QString connectionPath( QString name );

    typedef struct TableEntry
    {
      TableEntry( QString _tableName, QString _column, QString _type )
          : tableName( _tableName ), column( _column ), type( _type ) {}
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

    /** return list of tables. fetchTables() function has to be called before */
    QList<TableEntry> tables() { return mTables; }

    /** return additional error message (if an error occurred before) */
    QString errorMessage() { return mErrorMsg; }

    /**Updates the Internal Statistics*/
    bool updateStatistics();

  protected:
    // SpatiaLite DB open / close
    sqlite3 *openSpatiaLiteDb( QString path );
    void closeSpatiaLiteDb( sqlite3 * handle );

    /**Checks if geometry_columns and spatial_ref_sys exist and have expected layout*/
    int checkHasMetadataTables( sqlite3* handle );

    /**Inserts information about the spatial tables into mTables
      @return true if querying of tables was successful, false on error */
    bool getTableInfo( sqlite3 * handle, bool loadGeometrylessTables );

#ifdef SPATIALITE_VERSION_GE_4_0_0
    // only if libspatialite version is >= 4.0.0
    /**
       Inserts information about the spatial tables into mTables
       please note: this method is fully based on the Abstract Interface
       implemented in libspatialite starting since v.4.0

       using the Abstract Interface is highly reccommended, because all
       version-dependent implementation details become completly transparent,
       thus completely freeing the client application to take care of them.
    */
    bool getTableInfoAbstractInterface( sqlite3 * handle, bool loadGeometrylessTables );
#endif

    /**cleaning well-formatted SQL strings*/
    QString quotedValue( QString value ) const;

    /**Checks if geometry_columns_auth table exists*/
    bool checkGeometryColumnsAuth( sqlite3 * handle );

    /**Checks if views_geometry_columns table exists*/
    bool checkViewsGeometryColumns( sqlite3 * handle );

    /**Checks if virts_geometry_columns table exists*/
    bool checkVirtsGeometryColumns( sqlite3 * handle );

    /**Checks if this layer has been declared HIDDEN*/
    bool isDeclaredHidden( sqlite3 * handle, QString table, QString geom );

    /**Checks if this layer is a RasterLite-1 datasource*/
    bool isRasterlite1Datasource( sqlite3 * handle, const char * table );

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
    QgsSqliteHandle( sqlite3 * handle, const QString& dbPath, bool shared )
        : ref( shared ? 1 : -1 ), sqlite_handle( handle ), mDbPath( dbPath )
    {
    }

    sqlite3 *handle()
    {
      return sqlite_handle;
    }

    QString dbPath() const
    {
      return mDbPath;
    }

    //
    // libsqlite3 wrapper
    //
    void sqliteClose();

    static QgsSqliteHandle *openDb( const QString & dbPath, bool shared = true );
    static bool checkMetadata( sqlite3 * handle );
    static void closeDb( QgsSqliteHandle * &handle );

    /**
     * Will close any cached connection
     * To be called on application exit
     */
    static void closeAll();
    //static void closeDb( QMap < QString, QgsSqliteHandle * >&handlesRO, QgsSqliteHandle * &handle );

  private:
    int ref;
    sqlite3 *sqlite_handle;
    QString mDbPath;

    static QMap < QString, QgsSqliteHandle * > handles;
};


#endif // QGSSPATIALITECONNECTION_H
