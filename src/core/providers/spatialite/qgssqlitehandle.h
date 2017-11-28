/***************************************************************************
    qgssqlitehandle.h
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
#ifndef QGSSQLITEHANDLE_H
#define QGSSQLITEHANDLE_H

#include <QStringList>
#include <QObject>
#include <QHash>
#include <QUuid>

#include "qgis_core.h"

#include "qgsspatialitedbinfo.h"

/**
 * Class to connect to Spatialite/Rasterlite2 Database
  * - replaces previous QgsSLConnect class used for static connections
  * - contains and SpatialiteDbInfo object when used as a instance class
  * \note
  *  SpatialiteDbInfo is the backbone of the QgisSpatialite/RasterLite2 Providers
  *  When shared, the QgsSqliteHandle class will used the same connection of each Layer in the same Database
  *  QgsDebugMsgLevel = 3: for connection messages that deal with QgsSpatiaLiteProvider,QgsOgrProvider or QgsGdalProvider
  *  QgsDebugMsgLevel = 4: for connection messages that deal with connections not supporting QgsSpatiaLiteProvider,QgsOgrProvider or QgsGdalProvider
  *  QgsDebugMsgLevel = 5: experimental code under development
  *  Loading of Spatialite and RasterLite2 with '  * \see '
  *  Spatialite: 'mod_spatialite'
  *  RasterLite2: 'mod_rasterlite'
  * \param handle set when using QgsSqliteHandle::sqlite3_open_v2
  * \param dbPath the absolute file path used to open the Database
  * \param shared was created with share option
  * \param bInitSpatialite was QgsSqliteHandle::sqlite3_open_v2 called with spatialite  [default=false]
  * \see SpatialiteDbLayer
  * \see loadExtension
  * \since QGIS 3.0
 */
class CORE_EXPORT QgsSqliteHandle
{
  public:
    QgsSqliteHandle( sqlite3 *handle, const QString &dbPath, bool shared = false, bool bInitSpatialite = false )
      : ref( shared ? 0 : -1 )
      , mSqliteHandle( handle )
      , mDbPath( dbPath )
      , mIsValid( true )
      , mIsSpatialite( false )
      , mIsSpatialiteActive( bInitSpatialite )
      , mIsGdalOgr( false )
      , mIsRasterLite2( false )
      , mIsRasterLite2Active( false )
      , mDbValid( false )
    {
    }

    /**
     * The sqlite handler
     *  - used in SpatialiteDbInfo
     * \note
     *  - closing done through QgsSqliteHandle
     * \see QgsSqliteHandle::openDb
     * \see sqliteClose()
    * \since QGIS 1.8
    */
    sqlite3 *handle()
    {
      return mSqliteHandle;
    }

    /**
     * Absolute Path of the Connection
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

    /**
     * The Database filename without path
    * \returns mFileName  name of the file, excluding the path
    * \since QGIS 3.0
    */
    QString getFileName() const { return mFileName; }

    /**
     * The Database Directory without file
    * \returns mDirectoryName  name of the complete  path,  (without without symbolic links), excluding the file-name
    * \since QGIS 3.0
    */
    QString getDirectoryName() const { return mDirectoryName; }

    /**
     * Connection info (DB-path) without table and geometry
     * - this will be called from classes using SpatialiteDbInfo
     * \note
     *  - to call for Database and Table/Geometry portion use: SpatialiteDbLayer::getLayerDataSourceUri()
    * \returns uri with Database only
    * \see SpatialiteDbLayer::getLayerDataSourceUri()
    * \since QGIS 3.0
    */
    QString getDatabaseUri() const { return mDatabaseUri; }

    /**
     * Is the Database Connection valid
     * - not if the Database it is connected to is
     * \see sDbValid()
     * \since QGIS 1.8
     */
    bool isValid() const
    {
      return mIsValid;
    }

    /**
     * Set status of Database after getSpatialiteLayerInfo has run
     * - setting certin capabilities and File-Path infoormation
     * \note
     *  Making the information available during a shutdown
     * \see mDbValid
     * \see mIsSpatialite
     * \see mIsRasterLite2
     * \see mIsGdalOgr
     * \see mFileName
     * \see mDirectoryName
     * \see mDatabaseUri
     * \see mUuid
     * \since QGIS 3.0
     */
    bool setStatus()
    {
      if ( mSpatialiteDbInfo )
      {
        mDbValid = getSpatialiteDbInfo()->isDbValid();
        mIsSpatialite = getSpatialiteDbInfo()->isDbSpatialite();
        mIsRasterLite2 = getSpatialiteDbInfo()->isDbRasterLite2();
        mIsGdalOgr = getSpatialiteDbInfo()->isDbGdalOgr();
        mFileName = getSpatialiteDbInfo()->getFileName();
        mDirectoryName = getSpatialiteDbInfo()->getDirectoryName();
        mDatabaseUri = getSpatialiteDbInfo()->getDatabaseUri();
        if ( mUuid.isEmpty() )
        {
          mUuid = QUuid::createUuid().toString();
        }
      }
      else
      {
        mDbValid = false;
        mIsGdalOgr = false;
        mIsSpatialite = false;
        mIsRasterLite2 = false;
        invalidate(); // shutting down
      }
      return mDbValid;
    }

    /**
     * Unique number for this Connection
     * - mainly used to check if unique
     * \note
     *  - created with QUuid::createUuid().toString()
     * \see setStatus
     * \since QGIS 3.0
     */
    QString getUuid() const { return mUuid; }

    /**
     * Count on how often this Connection is being used when shared
     * \note
     *  -1 not being shared
     * \since QGIS 3.0
     */
    int getRef() const { return ref; }

    /**
     * Removes reference to this connection
     * \note
     *  -1 not being shared
    * \returns Count on how often this Connection is being used
     * \see SpatialiteDbInfo::getQSqliteHandle()
     * \since QGIS 3.0
     */
    int removeRef()
    {
      ref--;
      if ( ref <= 0 )
      {
        ref = -1;
      }
      return ref;
    }

    /**
     * Count on how many shared Spatialite Connections are active
     * \note
     *  Connections started with the static functions 'sqlite3_open' and 'sqlite3_open_v2' are not included
     * \since QGIS 3.0
     */
    int getSharedSpatialiteConnectionsCount() const
    {
      // Only count of shared connections is returned
      return sHandles.size();
    }

    /**
     * Set SpatialiteDbInfo pointer when valid
     * \see setStatus
     * \see SpatialiteDbInfo::attachQSqliteHandle
     * \since QGIS 1.8
     */
    void setSpatialiteDbInfo( SpatialiteDbInfo *spatialiteDbInfo )
    {
      mSpatialiteDbInfo = spatialiteDbInfo;
      setStatus();
    }

    /**
     * Retrieve SpatialiteDbInfo
     * - containing all Information about Database file
     * \note
     * - isDbValid() return if the connection contains layers that are supported by
     * -- QgsSpatiaLiteProvider, QgsGdalProvider and QgsOgrProvider
     * \see SpatialiteDbInfo::isDbValid()
     * \since QGIS 3.0
     */
    SpatialiteDbInfo *getSpatialiteDbInfo() const { return mSpatialiteDbInfo; }

    /**
     * Is the read Database supported by QgsSpatiaLiteProvider or
     * a format only supported by the QgsOgrProvider or QgsGdalProvider
     * \note
     *  when false: the file is either a non-supported sqlite3 container
     *  or not a sqlite3 file (a fossil file would be a sqlite3 container not supported)
     * \since QGIS 3.0
     */
    bool isDbValid() const { return mDbValid; }

    /**
     * The read Database only supported by the QgsRasterLite2Provider
     * \note
     *  - QgsRasterLite2Provider: RasterLite2Raster
     * \since QGIS 3.0
     */
    bool isDbSpatialite() const { return mIsSpatialite; }

    /**
     * Has 'mod_spatialite' been called for the QgsSpatialiteProvider and QgsRasterLite2Provider
     * \note
     *  - QgsSpatialiteProvider and QgsRasterLite2Provider
     * \since QGIS 3.0
     */
    bool isDbSpatialiteActive() const { return mIsSpatialiteActive; }

    /**
     * The read Database only supported by the QgsOgrProvider or QgsGdalProvider Drivers
     * \note
     *  - QgsOgrProvider: GeoPackage-Vector
     *  - QgsGdalProvider: GeoPackage-Raster, MbTiles
     *  - QgsGdalProvider: RasterLite1 [when Gdal-RasterLite Driver is active]
     * \since QGIS 3.0
     */
    bool isDbGdalOgr() const { return mIsGdalOgr; }

    /**
     * The read Database only supported by the QgsRasterLite2Provider
     * \note
     *  - QgsRasterLite2Provider: RasterLite2Raster
     * \since QGIS 3.0
     */
    bool isDbRasterLite2() const { return mIsRasterLite2; }

    /**
     * Has 'mod_rasterlite2' or 'rl2_init' been called for the QgsRasterLite2Provider
     * \note
     *  - QgsRasterLite2Provider: RasterLite2Raster
     * \since QGIS 3.0
     */
    bool isDbRasterLite2Active() const { return mIsRasterLite2Active; }

    /**
     * Is the QgsSqliteHandle Connection being shared
     *  - true when absolute file name is contained in 'sHandles'
     * \note
     *  - if not shared: the reference value will be set to -1, if not allready
     * \see QgsSqliteHandle::openDb
     * \since QGIS 3.0
     */
    bool isShared()
    {
      if ( sHandles.contains( dbPath() ) )
      {
        return true;
      }
      if ( ref != 1 )
      {
        ref = -1;
      }
      return false;
    }

    /**
     * Set the QgsSqliteHandle Connection to be shared
     * \see QgsSqliteHandle::openDb
     * \since QGIS 3.0
     */
    bool setShared( bool bIsShared );

    /**
     * Invalidate the Database Connection valid
     * \note
     * When used in QgsDataItem's, many threads are running at once
     * - a reconnect may take place in one DataItem before the other has compleated
     * A shared connection will not return a connection that has been invalidated
     * \see SpatialiteDbInfo::~SpatialiteDbInfo
     * \see openDb
     * \since QGIS 1.8
     */
    void invalidate()
    {
      mIsValid = false;
    }

    /**
     * Load  Spatialite driver
     *  -  using 'mod_spatialite'
     * \note
     *  The spatialite-Library allows only a 64 connections at one time
     *  - connection should only be initalized when really needed
     *  -> which will be called from SpatialiteDbInfo when needed
     * \param bRasterLite2 load with RasterLite2 [default=false]
     * \returns true if 'mod_spatialite' succeeded
     * \see mIsSpatialiteActive
     * \see initSpatialite
     * \see initRasterlite2
     * \since QGIS 3.0
     */
    bool loadSpatialite( bool bRasterLite2 = false );

    /**
     * If  RasterLite2 is to be used
     *  -  'mod_rasterlite2' must be called
     * \note
     *  The Spatialite-Library contains placeholders for RasterLite2 function
     *  - during 'mod_rasterlite2' these will be replaced with the real functions
     *  -> which will be called from SpatialiteDbInfo when needed
     * \returns true if 'mod_rasterlite2' succeeded
     * \see mIsRasterLite2Active
     * \see mIsSpatialiteActive
     * \since QGIS 3.0
     */
    bool initRasterlite2();

    /**
     * Close Spatialite Database
     *  - created with openDb
     *  -> called from closeDb [should never be called directly]
     * \note
     *  - will close the Database connection
     *  - if 'sqlite3_load_extension' has been used
     *  -> these extensions will be unloaded
     *  - if SpatialiteDbInfo is being used
     *  -> will be deleted
     * \see openDb
     * \see closeDb
     * \since QGIS 1.8
     */
    void sqliteClose();

    /**
     * Open Spatialite Database
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
     * \see QgsSqliteHandle
     * \since QGIS 1.8
     */
    static QgsSqliteHandle *openDb( const QString &dbPath, bool shared = true, QString sLayerName = QString::null, bool bLoadLayers = true, SpatialiteDbInfo::SpatialMetadata dbCreateOption = SpatialiteDbInfo::SpatialUnknown );

    /**
     * Close a (possibly cached) Spatialite Database
     * - checking will be none if the Connection is cached
     * \note
     *  - sqliteClose will be called for each Connection
     *  -> performing any needed tasks
     *  - remove a cached Connection, when no longer being referenced
     * \see sqliteClose
     * \since QGIS 1.8
     */
    static void closeDb( QgsSqliteHandle *&qSqliteHandle );

    /**
     * Will close any cached connection
     * To be called on application exit
     * \since QGIS 1.8
     */
    static void closeAll();

    /**
     * If  Spatialite is to be used
     *  -  'mod_spatialite' must be used
     * \note
     *  The spatialite-Library allows only a 64 connections at one time
     *  - connection should only be initalized when really needed
     *  -> which will be called from SpatialiteDbInfo when needed
     * \param sqlite_handle : SQLite db handle [must be open]
     * \param sFileName FileName, without path [for messages only]
     * \returns true if 'mod_spatialite' succeeded
     * \see SpatialiteDbInfo
     * \see sqlite3_open
     * \see sqlite3_open_v2
     * \see initRasterlite2
     * \see loadExtension
     * \since QGIS 3.0
     */
    static bool initSpatialite( sqlite3 *sqlite_handle, QString sFileName = QString() );

    /**
     * Opening A New Database Connection
     * -  without two additional parameters for additional control over the new database connection
     * \note
     * - corresponds to flags=SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE
     * 'PRAGMA foreign_keys = 1' will be called [needed for proper internal spatialite support for admin tables (CASCADE)]
     * \param filename Database filename (UTF-8) [':memory:']
     * \param psqlite_handle OUT: SQLite db handle
     * \param bInitSpatialite: true: start with Spatialite connection ('mod_spatialite')
     * \see initSpatialite
     * \since QGIS 3.0
     */
    static int sqlite3_open( const char *filename, sqlite3 **psqlite_handle, bool bInitSpatialite = true );

    /**
     * Opening A New Database Connection
     * -  with two additional parameters for additional control over the new database connection
     * \note
     * - flages=bShared ? SQLITE_OPEN_READWRITE : SQLITE_OPEN_READONLY | SQLITE_OPEN_NOMUTEX;
     * 'PRAGMA foreign_keys = 1' will be called [needed for proper internal spatialite support for admin tables (CASCADE)]
     * \param filename Database filename (UTF-8) [':memory:']
     * \param psqlite_handle OUT: SQLite db handle
     * \param flags Flags
     * \param zVfs Name of VFS module to use
     * \param bInitSpatialite true: start with Spatialite connection ('mod_spatialite')
     * \see initSpatialite
     * \since QGIS 3.0
     */
    static int sqlite3_open_v2( const char *filename, sqlite3 **psqlite_handle, int flags, const char *zVfs = nullptr, bool bInitSpatialite = true );

    /**
     * Closing A Database Connection
     * -  will not return SQLITE_OK until all open tasks are completed
     * \note
     *  - if 'sqlite3_load_extension' has been used
     *  -> these extensions will be unloaded
     * \param sqlite_handle : SQLite db handle [not previously closed]
     * \param sFileName FileName, without path [for messages only]
     * \returns SQLITE_BUSY when unfinalized prepared statements or unfinished sqlite3_backup objects, otherwise SQLITE_OK (0)
     * \since QGIS 3.0
     */
    static int sqlite3_close( sqlite3 *sqlite_handle, QString sFileName = QString() );

    /**
     * Closing A Database Connection
     * -  will complete unfinalized prepared statements or unfinished sqlite3_backup objects
     * \note
     *  - if 'sqlite3_load_extension' has been used
     *  -> these extensions will be unloaded
     * \param sqlite_handle : SQLite db handle [not previously closed]
     * \param sFileName FileName, without path [for messages only]
     * \returns SQLITE_OK (0), will be destroyed only after completing tasks
     * \since QGIS 3.0
     */
    static int sqlite3_close_v2( sqlite3 *sqlite_handle, QString sFileName = QString() );

    /**
     * Return String representation of sqlite3 return code
     * -  Extended Result Codes are not supported
     * \param i_rc from sqlite3 function
     * \see https://sqlite.org/c3ref/c_abort.html
     * \see https://sqlite.org/c3ref/c_abort_rollback.html
     * \returns Text representation of sqlite3 return code
     * \since QGIS 3.0
     */
    static QString get_sqlite3_result_code_string( int i_rc = 0 );
  private:

    /**
     * Count on how often this Connection is being used when shared
     * \note
     *  -1 not being shared
     * \since QGIS 1.8
     */
    int ref;

    /**
     * The sqlite handler
     *  - used in SpatialiteDbInfo
     * \see QgsSqliteHandle::openDb
     * \see sqliteClose()
    * \since QGIS 1.8
    */
    sqlite3 *mSqliteHandle = nullptr;

    /**
     * Absolute Path of the Connection
     * - 'SpatiaLite/connections'
     * \note
     *  - extracted from ConnectionString
     * \returns name of connection
     * \see connectionPath
     * \since QGIS 1.8
     */
    QString mDbPath;

    /**
     * The Database filename without path
    * \returns mFileName  name of the file, excluding the path
    * \since QGIS 3.0
    */
    QString mFileName;

    /**
     * The Database Directory without file
    * \returns mDirectoryName  name of the complete  path,  (without without symbolic links), excluding the file-name
    * \since QGIS 3.0
    */
    QString mDirectoryName;

    /**
     * Connection info (DB-path) without table and geometry
     * - this will be called from classes using SpatialiteDbInfo
     * \note
     *  - to call for Database and Table/Geometry portion use: SpatialiteDbLayer::getLayerDataSourceUri()
    * \returns uri with Database only
    * \see SpatialiteDbLayer::getLayerDataSourceUri()
    * \since QGIS 3.0
    */
    QString mDatabaseUri;

    /**
     * Is the Database Connection valid
     * - not if the Database it is connected is
     * \see sDbValid()
     * \since QGIS 1.8
     */
    bool mIsValid;

    /**
     * The read Database only supported by the QgsSpatialiteProvider
     * \note
     *  - QgsSpatialiteProvider:
     * \since QGIS 3.0
     */
    bool mIsSpatialite;

    /**
     * Has 'mod_spatialite' or 'spatialite_init' been called for the QgsSpatialiteProvider and QgsRasterLite2Provider
     * \note
     *  - QgsSpatialiteProvider and QgsRasterLite2Provider
     * \since QGIS 3.0
     */
    bool mIsSpatialiteActive;

    /**
     * The read Database only supported by the QgsOgrProvider or QgsGdalProvider Drivers
     * \note
     *  - QgsOgrProvider: GeoPackage-Vector
     *  - QgsGdalProvider: GeoPackage-Raster, MbTiles
     *  - QgsGdalProvider: RasterLite1 [when Gdal-RasterLite Driver is active]
     * \since QGIS 3.0
     */
    bool mIsGdalOgr;

    /**
     * The read Database only supported by the QgsRasterLite2Provider
     * \note
     *  - QgsRasterLite2Provider: RasterLite2Raster
     * \since QGIS 3.0
     */
    bool mIsRasterLite2;

    /**
     * Has 'mod_rasterlite2' been called for the QgsRasterLite2Provider
     * \note
     *  - QgsRasterLite2Provider: RasterLite2Raster
     * \see initRasterlite2
     * \since QGIS 3.0
     */
    bool mIsRasterLite2Active;

    /**
     * Is the read Database supported by QgsSpatiaLiteProvider or
     * a format only supported by the QgsOgrProvider or QgsGdalProvider
     * \note
     *  when false: the file is either a non-supported sqlite3 container
     *  or not a sqlite3 file (a fossil file would be a sqlite3 container not supported)
     * \since QGIS 3.0
     */
    bool mDbValid;

    /**
     * Retrieve SpatialiteDbInfo
     * - containing all Information about Database file
     * \note
     * - isDbValid() return if the connection contains layers that are supported by
     * -- QgsSpatiaLiteProvider, QgsGdalProvider and QgsOgrProvider
     * \see SpatialiteDbInfo::isDbValid()
     * \since QGIS 3.0
     */
    SpatialiteDbInfo *mSpatialiteDbInfo = nullptr;

    /**
     * Map of cached of QgsSqliteHandle connections
     * \note
     *  -  not used when the static functions 'sqlite3_open' and 'sqlite3_open_v2' are called directly
     * \see openDb
     * \see closeDb
     * \since QGIS 1.8
     */
    static QMap < QString, QgsSqliteHandle * > sHandles;

    /**
     * Load  Extension for Spatialite or RasterLite2 Extension
     *  -  using sqlite3_load_extension
     *  -  sqlite3_enable_load_extension(sqlite_handle,1)
     * \note
     * Spatialite must be called first, then RasterLite2
     * Spatialite: 'mod_spatialite'  [bRasterLite2=false]
     * RasterLite2: 'mod_rasterlite2' [bRasterLite2=true]
     * \param sqlite_handle : SQLite db handle [must be open]
     * \param bRasterLite2 load Spatialite[false, default] or RasterLite2 [true]
     * \param sFileName FileName, without path [for messages only]
     * \returns true sqlite3_load_extension returns SQLITE_OK [0]
     * \see initSpatialite
     * \see loadSpatialite
     * \see initRasterlite2
     * \since QGIS 3.0
     */
    static bool loadExtension( sqlite3 *sqlite_handle, bool bRasterLite2 = false, QString sFileName = QString() );

    /**
     * Unique number for this Connection
     * - mainly used to check if unique
     * \note
     *  - created with QUuid::createUuid().toString()
     * \see setStatus
     * \since QGIS 3.0
     */
    QString mUuid = QString();

};
#endif // QGSSQLITEHANDLE_H
