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
#include <QHash>

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

    /** Construct a connection. Name can be either stored connection name or a path to the database file
     * - not used outside the QgsSpatiaLiteProvider classes
     * \since QGIS 1.8
     */
    explicit QgsSpatiaLiteConnection( const QString &name );

    /** Absolute Path of the Connection
     * - 'SpatiaLite/connections'
     * \note
     *  - extrcted from ConnectionString
     * \returns name of connection
     * \see connectionPath
     * \since QGIS 1.8
     */
    QString dbPath() { return mDbPath; }

    /** Key of the Connection
     * - 'SpatiaLite/connections'
     * \note
     *  - extrcted from ConnectionString
     * \returns name of connection
     * \see connectionPath
     * \since QGIS 1.8
     */
    QString getSubKey() { return mSubKey; }

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

    /** Create a SpatialiteDbInfo based Connection
     *  -> containing all needed Information about a Spatial Sqlite3 Container
     * \note
     *  - check result with spatialiteDbInfo->isDbSqlite3()
     *  -> if File exists and is a Sqlite3 Container.
     *  - check result with spatialiteDbInfo->isDbGdalOgr()
     *  -> if File only supported by QgsOgrProvider or QgsGdalProvider
     *  -> otherwise supported by QgsSpatiaLiteProvider
    * \returns true if file is a sqlite3 Database
    * \since QGIS 3.0
    */
    SpatialiteDbInfo *CreateSpatialiteConnection( QString sLayerName = QString::null, bool bLoadLayers = true, bool bShared = true, SpatialiteDbInfo::SpatialMetadata dbCreateOption = SpatialiteDbInfo::SpatialUnknown );

  protected:

    /** Key of the Connection
     * - 'SpatiaLite/connections'
     * \note
     *  - extrcted from ConnectionString
     * \returns name of connection
     * \see connectionPath
     * \since QGIS 1.8
     */
    QString mSubKey;

    /** Absolute Path of the Connection
     * - 'SpatiaLite/connections'
     * \note
     *  - extrcted from ConnectionString
     * \returns name of connection
     * \see connectionPath
     * \since QGIS 1.8
     */
    QString mDbPath;
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
      , mIsRasterLite2( false )
      , mIsRasterLite2Active( false )
      , mDbValid( false )
    {
    }

    /** The sqlite handler
     *  - used in SpatialiteDbInfo
     * \note
     *  - closing done through QgsSqliteHandle
     * \see QgsSqliteHandle::openDb
     * \see sqliteClose()
    * \since QGIS 1.8
    */
    sqlite3 *handle()
    {
      return sqlite_handle;
    }

    /** Absolute Path of the Connection
     * - 'SpatiaLite/connections'
     * \note
     *  - extracted from ConnectionString
     * \returns name of connection
     * \see connectionPath
     * \since QGIS 1.8
     */
    QString dbPath() const
    {
      return mDbPath;
    }

    /** Is the Database Connection valid
     * - not if the Database it is connected is
     * \see sDbValid()
     * \since QGIS 1.8
     */
    bool isValid() const
    {
      return mIsValid;
    }

    /** Set status of Database after GetSpatialiteDbInfo has run
     * - setting certin capabilities
     * \see SpatialiteDbInfo::GetSpatialiteDbInfo
     * \since QGIS 3.0
     */
    bool setStatus()
    {
      if ( mSpatialiteDbInfo )
      {
        mDbValid = getSpatialiteDbInfo()->isDbValid();
        mIsGdalOgr = getSpatialiteDbInfo()->isDbGdalOgr();
        mIsRasterLite2 = getSpatialiteDbInfo()->isDbRasterLite2();
      }
      return mDbValid;
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
        mIsValid = true;
      }
      else
      {
        mDbValid = false;
        mIsGdalOgr = false;
        mIsRasterLite2 = false;
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

    /** The read Database only supported by the QgsRasterLite2Provider
     * \note
     *  - QgsRasterLite2Provider: RasterLite2Raster
     * \since QGIS 3.0
     */
    bool isDbRasterLite2() const { return mIsRasterLite2; }


    /** Has rl2_init been called for the QgsRasterLite2Provider
     * \note
     *  - QgsRasterLite2Provider: RasterLite2Raster
     * \since QGIS 3.0
     */
    bool isDbRasterLite2Active() const { return mIsRasterLite2Active; }

    /** Invalidate the Database Connection valid
     * \since QGIS 1.8
     */
    void invalidate()
    {
      mIsValid = false;
    }

    /** If  RasterLite2 is to be used
     *  -  Memory must be  allocated
     * \note
     *  The spatialite-Library contains placeholders for RasterLite2 function
     *  - during 'rl2_init' these will be replaced with the real functions
     *  -> which will be called from SpatialiteDbInfo when needed
     * \returns true if rl2PrivateData if not nullptr (i.e. 'rl2_init' has been called)
     * \see SpatialiteDbInfo
     * \since QGIS 3.0
     */
    bool initRasterlite2();

    /** Close Spatialite Database
     *  - created with openDb
     *  -> called from closeDb [should nver be called directly]
     * \note
     *  - will close the Database connection
     *  - if the reserved memory for RasterLite2 has been allocated
     *  -> will be deallocated
     *  - if SpatialiteDbInfo is being used
     *  -> will be deleted
     * \see openDb
     * \see closeDb
     * \since QGIS 1.8
     */
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
     * \since QGIS 1.8
     */
    static QgsSqliteHandle *openDb( const QString &dbPath, bool shared = true, QString sLayerName = QString::null, bool bLoadLayers = true, SpatialiteDbInfo::SpatialMetadata dbCreateOption = SpatialiteDbInfo::SpatialUnknown );

    /** Close a (possibly cached) Spatialite Database
     * - checking will be none if the Connection is cached
     * \note
     *  - sqliteClose will be called for each Connection
     *  -> performing any needed tasks
     *  - remove a cached Connection, when no longer being referenced
     * \see sqliteClose
     * \since QGIS 1.8
     */
    static void closeDb( QgsSqliteHandle *&handle );

    /**
     * Will close any cached connection
     * To be called on application exit
     * \since QGIS 1.8
     */
    static void closeAll();

    /** Opening A New Database Connection
     * -  without two additional parameters for additional control over the new database connection
     * \note
     * - corresponds to flags=SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE
     * \param filename Database filename (UTF-8) [':memory:']
     * \param ppDb OUT: SQLite db handle
     * \since QGIS 3.0
     */
    static int sqlite3_open( const char *filename, sqlite3 **ppDb );

    /** Closing A Database Connection
     * -  will not return SQLITE_OK until all open tasks are completed
     * \note
     * - spatialite_cleanup_ex and spatialite_shutdown will be called when SPATIALITE_HAS_INIT_EX
     * \param filename Database filename (UTF-8) [':memory:']
     * \param sqlite3* : SQLite db handle [not previously closed]
     * \returns SQLITE_BUSY when unfinalized prepared statements or unfinished sqlite3_backup objects, otherwise SQLITE_OK (0)
     * \since QGIS 3.0
     */
    static int sqlite3_close( sqlite3 * );

    /** Opening A New Database Connection
     * -  with two additional parameters for additional control over the new database connection
     * \note
     * - flages=bShared ? SQLITE_OPEN_READWRITE : SQLITE_OPEN_READONLY | SQLITE_OPEN_NOMUTEX;
     * \param filename Database filename (UTF-8) [':memory:']
     * \param ppDb OUT: SQLite db handle
     * \param flags Flags
     * \param zVfs Name of VFS module to use
     * \since QGIS 3.0
     */
    static int sqlite3_open_v2( const char *filename, sqlite3 **ppDb, int flags, const char *zVfs = nullptr );

    /** Closing A Database Connection
     * -  will complete unfinalized prepared statements or unfinished sqlite3_backup objects
     * \note
     * - spatialite_cleanup_ex and spatialite_shutdown will be called when SPATIALITE_HAS_INIT_EX
     * \param filename Database filename (UTF-8) [':memory:']
     * \param sqlite3* : SQLite db handle [not previously closed]
     * \returns SQLITE_OK (0), will be destroyed only after completing tasks
     * \since QGIS 3.0
     */
    static int sqlite3_close_v2( sqlite3 * );
  private:

    /** Count on how often this Connection is being used when shared
     * \note
     *  -1 not being shared
     * \since QGIS 1.8
     */
    int ref;

    /** The sqlite handler
     *  - used in SpatialiteDbInfo
     * \see QgsSqliteHandle::openDb
     * \see sqliteClose()
    * \since QGIS 1.8
    */
    sqlite3 *sqlite_handle = nullptr;

    /** Absolute Path of the Connection
     * - 'SpatiaLite/connections'
     * \note
     *  - extracted from ConnectionString
     * \returns name of connection
     * \see connectionPath
     * \since QGIS 1.8
     */
    QString mDbPath;

    /** Is the Database Connection valid
     * - not if the Database it is connected is
     * \see sDbValid()
     * \since QGIS 1.8
     */
    bool mIsValid;

    /** The read Database only supported by the QgsOgrProvider or QgsGdalProvider Drivers
     * \note
     *  - QgsOgrProvider: GeoPackage-Vector
     *  - QgsGdalProvider: GeoPackage-Raster, MbTiles
     *  - QgsGdalProvider: RasterLite1 [when Gdal-RasterLite Driver is active]
     * \since QGIS 3.0
     */
    bool mIsGdalOgr;

    /** The read Database only supported by the QgsRasterLite2Provider
     * \note
     *  - QgsRasterLite2Provider: RasterLite2Raster
     * \since QGIS 3.0
     */
    bool mIsRasterLite2;

    /** Has rl2_init been called for the QgsRasterLite2Provider
     * \note
     *  - QgsRasterLite2Provider: RasterLite2Raster
     * \since QGIS 3.0
     */
    bool mIsRasterLite2Active;

    /** Is the read Database supported by QgsSpatiaLiteProvider or
     * a format only supported by the QgsOgrProvider or QgsGdalProvider
     * \note
     *  when false: the file is either a non-supported sqlite3 container
     *  or not a sqlite3 file (a fossil file would be a sqlite3 container not supported)
     * \since QGIS 3.0
     */
    bool mDbValid;

    /** Retrieve SpatialiteDbInfo
     * - containing all Information about Database file
     * \note
     * - isDbValid() return if the connection contains layers that are supported by
     * -- QgsSpatiaLiteProvider, QgsGdalProvider and QgsOgrProvider
     * \see SpatialiteDbInfo::isDbValid()
     * \since QGIS 3.0
     */
    SpatialiteDbInfo *mSpatialiteDbInfo = nullptr;

    /** Pointer to RL2 Private Data
     * If  RasterLite2 ist to be used
     *  -  Memory must be  allocated
     * \note
     *  - planned feature for the implementation of a RasterLite2Provider
     *  -> which will be called from SpatialiteDbInfo when needed
     * \see initRasterlite2
     * \since QGIS 3.0
     */
#ifdef RASTERLITE2_VERSION_GE_1_1_0
    void *rl2PrivateData = nullptr;  // pointer to RL2 Private Data
#endif

    /**
     * Map of cached of QgsSqliteHandle connections
     * \see openDb
     * \see closeDb
     * \since QGIS 1.8
     */
    static QMap < QString, QgsSqliteHandle * > sHandles;
#if defined(SPATIALITE_HAS_INIT_EX)

    /** Hash to contains the sqlite3 handle and the spatialite InternalCache
     *    which supports the DB connection
     *  -  Memory must be  allocated and freed [SPATIALITE_HAS_INIT_EX]
     * \note
     *  - setting up the internal cache: spatialite_alloc_connection();
     *  -> spatialite_init_ex(SqliteHandle, SpliteInternalCache, 0);
     *  -> spatialite_cleanup_ex(SpliteInternalCache);
     *  -> spatialite_shutdown [free memory used by this connection]
     * \see sqlite3_open
     * \see sqlite3_close
     * \see sqlite3_open_v2
     * \see sqlite3_close_v2
     * \since QGIS 3.0
     */
    static QHash<sqlite3 *, void *> sSpatialiteConnections;
#endif
};
#endif // QGSSPATIALITECONNECTION_H
