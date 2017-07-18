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
#ifdef RASTERLITE2_VERSION_GE_1_1_0
#include <rasterlite2.h>
#endif
}

#include "qgsspatialiteutils.h"


class QgsSpatiaLiteConnection : public QObject
{
    Q_OBJECT
  public:
    //! Construct a connection. Name can be either stored connection name or a path to the database file
    explicit QgsSpatiaLiteConnection( const QString &name );

    QString path() { return mPath; }

    /** Return List of all stored Connection-Namess in QgsSettings
     * - 'SpatiaLite/connections'
     * \note
     *  - use connectionPath to retrieve Absolute Path of the Connection
     * \returns name of connection
     * \see connectionPath
     * \since QGIS 1.8
     */
    static QStringList connectionList();

    /** Delete the given Connection in QgsSettings
     * \param name of connection as retuned from connectionList
     * \see connectionList
     * \since QGIS 1.8
     */
    static void deleteConnection( const QString &name );

    /** Remove the Connection Strings from the QgsSettings
     * - when the file no longer exists
     * \note
     *  - uses connectionPath to retrieve Absolute Path of the Connection
     *  - uses deleteConnection to delete the connection
     * \returns amount of removed files
     * \see connectionPath
     * \see deleteConnection
     * \since QGIS 3.0
     */
    static int deleteInvalidConnections( );

    /** Return the absolute Path of the given Connection
     * - when the file no longer exists
     * \param name of connection as retuned from connectionList
     * \returns path of Database file
     * \see connectionList
     * \since QGIS 1.8
     */
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
    // TODO: Remove after replacing with SpatialiteDbInfo
    Error fetchTables( bool loadGeometrylessTables );
    //! Return list of tables. fetchTables() function has to be called before
    QList<TableEntry> tables() { return mTables; }
    //! Return additional error message (if an error occurred before)
    QString errorMessage() { return mErrorMsg; }
    // TODO: Remove after replacing with SpatialiteDbInfo
    //! Updates the Internal Statistics
    bool updateStatistics();
  protected:
    // SpatiaLite DB open / close
    // TODO: Remove after replacing with SpatialiteDbInfo
    sqlite3 *openSpatiaLiteDb( const QString &path );
    // TODO: Remove after replacing with SpatialiteDbInfo
    void closeSpatiaLiteDb( sqlite3 *handle );
    // TODO: Remove after replacing with SpatialiteDbInfo
    //! Checks if geometry_columns and spatial_ref_sys exist and have expected layout
    int checkHasMetadataTables( sqlite3 *handle );
    // TODO: Remove after replacing with SpatialiteDbInfo

    /** Inserts information about the spatial tables into mTables
      \returns true if querying of tables was successful, false on error */
    bool getTableInfo( sqlite3 *handle, bool loadGeometrylessTables );
#ifdef SPATIALITE_VERSION_GE_4_0_0
    // TODO: Remove after replacing with SpatialiteDbInfo
    // only if libspatialite version is >= 4.0.0

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
#endif
    //! Cleaning well-formatted SQL strings
    QString quotedValue( QString value ) const;
    // TODO: Remove after replacing with SpatialiteDbInfo
    //! Checks if geometry_columns_auth table exists
    bool checkGeometryColumnsAuth( sqlite3 *handle );
    // TODO: Remove after replacing with SpatialiteDbInfo
    //! Checks if views_geometry_columns table exists
    bool checkViewsGeometryColumns( sqlite3 *handle );
    // TODO: Remove after replacing with SpatialiteDbInfo
    //! Checks if virts_geometry_columns table exists
    bool checkVirtsGeometryColumns( sqlite3 *handle );
    // TODO: Remove after replacing with SpatialiteDbInfo
    //! Checks if this layer has been declared HIDDEN
    bool isDeclaredHidden( sqlite3 *handle, const QString &table, const QString &geom );
    // TODO: Remove after replacing with SpatialiteDbInfo
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
    QgsSqliteHandle( sqlite3 *handle, const QString &dbPath, bool shared )
      : ref( shared ? 1 : -1 )
      , sqlite_handle( handle )
      , mDbPath( dbPath )
      , mIsValid( true )
      , mIsGdalOgr( false )
      , mDbValid( false )
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

    /** Is the Database Connection valid
     * \since QGIS 1.8
     */
    bool isValid() const
    {
      return mIsValid;
    }

    /** Count on how often this Connection is being used when shared
     * \note
     *  -1 not being shared
     * \since QGIS 3.0
     */
    bool getRef() const { return ref; }

    /** Set SpatialiteDbInfo pointer when valid
     * \see SpatialiteDbInfo::attachQSqliteHandle
     * \since QGIS 1.8
     */
    void setSpatialiteDbInfo( SpatialiteDbInfo *spatialiteDbInfo )
    {
      mSpatialiteDbInfo = spatialiteDbInfo;
      if ( mSpatialiteDbInfo )
      {
        mDbValid = getSpatialiteDbInfo()->isDbValid();
        mIsGdalOgr = getSpatialiteDbInfo()->isDbGdalOgr();
      }
      else
      {
        mDbValid = false;
        mIsGdalOgr = false;
      }
    }

    /** Retrieve SpatialiteDbInfo
     * - containing all Information about Database file
     * \note
     * - isDbValid() return if the connection contains layers that are supported by
     * -- QgsSpatiaLiteProvider, QgsGdalProvider and QgsOgrProvider
     * \see SpatialiteDbInfo::isDbValid()
     * \since QGIS 3.0
     */
    SpatialiteDbInfo *getSpatialiteDbInfo() const { return mSpatialiteDbInfo; }

    /** Is the read Database supported by QgsSpatiaLiteProvider or
     * a format only supported by the QgsOgrProvider or QgsGdalProvider
     * \note
     *  when false: the file is either a non-supported sqlite3 container
     *  or not a sqlite3 file (a fossil file would be a sqlite3 container not supported)
     * \since QGIS 3.0
     */
    bool isDbValid() const { return mDbValid; }

    /** The read Database only supported by the QgsOgrProvider or QgsGdalProvider Drivers
     * \note
     *  - QgsOgrProvider: GeoPackage-Vector
     *  - QgsGdalProvider: GeoPackage-Raster, MbTiles
     *  - QgsGdalProvider: RasterLite1 [when Gdal-RasterLite Driver is active]
     * \since QGIS 3.0
     */
    bool isDbGdalOgr() const { return mIsGdalOgr; }
    void invalidate()
    {
      mIsValid = false;
    }
    //
    // libsqlite3 wrapper
    //
    void initRasterlite2();
    void sqliteClose();

    /** Open Spatialite Database
     * - at this point we are 'sniffing' the Capabilities of the opened Database
     * \note
     *  - when only a specific table or table with geometry are being looked for
     *  -> format: 'table_name(geometry_name)' or only 'table_name', with all of its geometries
     *  - otherwise all tables with geometries
     * \param sDatabaseFileName Database Filename
     * \param shared share this connection with others [default]
     * \param sLayerName when used will load the Layer-Information of that Layer only
     * \param bLoadLayers Load all Layer-Information or only 'sniff' [default] the Database the Database
     * \returns SpatialiteDbInfo with collected results if supported by QgsSpatiaLiteProvider,QgsOgrProvider or QgsGdalProvider
     * \see QgsSLConnect
     * \since QGIS 3.0
     */
    static QgsSqliteHandle *openDb( const QString &dbPath, bool shared = true, QString sLayerName = QString::null, bool bLoadLayers = true );
#if 0
    static bool checkMetadata( sqlite3 *handle );
#endif
    static void closeDb( QgsSqliteHandle *&handle );

    /**
     * Will close any cached connection
     * To be called on application exit
     */
    static void closeAll();
    //static void closeDb( QMap < QString, QgsSqliteHandle * >&handlesRO, QgsSqliteHandle * &handle );
  private:
    int ref;
    sqlite3 *sqlite_handle = nullptr;
    QString mDbPath;
    bool mIsValid;
    bool mIsGdalOgr;
    bool mDbValid;
    SpatialiteDbInfo *mSpatialiteDbInfo = nullptr;
    void *rl2PrivateData = nullptr;  // pointer to RL2 Private Data
    static QMap < QString, QgsSqliteHandle * > sHandles;
};
#endif // QGSSPATIALITECONNECTION_H
