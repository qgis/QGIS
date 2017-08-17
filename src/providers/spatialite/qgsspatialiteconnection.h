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
    SpatialiteDbInfo *CreateSpatialiteConnection( QString sLayerName = QString::null, bool bLoadLayers = true, bool bShared = true );

#if 0
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
#endif
  protected:
#if 0
    QString mErrorMsg;
    QList<TableEntry> mTables;
#endif

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

    /** Invalidate the Database Connection valid
     * \since QGIS 1.8
     */
    void invalidate()
    {
      mIsValid = false;
    }

    /** If  RasterLite2 ist to be used
     *  -  Memory must be  allocated
     * \note
     *  - planned feature for the implementation of a RasterLite2Provider
     *  -> which will be called from SpatialiteDbInfo when needed
     * \see SpatialiteDbInfo
     * \since QGIS 3.0
     */
    void initRasterlite2();

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
    static QgsSqliteHandle *openDb( const QString &dbPath, bool shared = true, QString sLayerName = QString::null, bool bLoadLayers = true );

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
    //static void closeDb( QMap < QString, QgsSqliteHandle * >&handlesRO, QgsSqliteHandle * &handle );
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
     * Map of cached connections
     * \see openDb
     * \see closeDb
     * \since QGIS 1.8
     */
    static QMap < QString, QgsSqliteHandle * > sHandles;
};
#endif // QGSSPATIALITECONNECTION_H
