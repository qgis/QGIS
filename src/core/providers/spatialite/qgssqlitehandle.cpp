/***************************************************************************
    qgssqlitehandle.cpp
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
#include "qgssqlitehandle.h"
#include "qgssettings.h"
#include "qgslogger.h"
#include <QFileInfo>
#include <cstdlib> // atoi

#ifdef _MSC_VER
#define strcasecmp(a,b) stricmp(a,b)
#endif

//-----------------------------------------------------------------
// QgsSqliteHandle
//-----------------------------------------------------------------
QMap < QString, QgsSqliteHandle * > QgsSqliteHandle::sHandles;
//-----------------------------------------------------------------
// QgsSqliteHandle::openDb [static]
//-----------------------------------------------------------------
// [will be created if a valid create-option is given and the file does not exist]
// If 'shared' == 1 and isDbValid():
// - then it has been added to 'sHandles' as ConnectionRef[1] when not contained in 'sHandles', which (at this point) should never be
// -> will contain a filled Uuid
// If NOT 'shared' == 1 and/or NOT isDbValid(): ConnectionRef[-1]
// -> will contain an empty Uuid
//-----------------------------------------------------------------
// Note: for extrnal functions, such as
// - QgsSpatiaLiteProvider::deleteLayer [SpatialiteDbInfo::dropGeoTable]
// -> the fetched coonection Uuid should be the same as
// a possibly active Layer in QgsSpatiaLiteProvider
// -> The QgsSpatiaLiteProvider should invalidate itsself
//-----------------------------------------------------------------
QgsSqliteHandle *QgsSqliteHandle::openDb( const QString &dbPath, bool shared,  QString sLayerName, bool bLoadLayers, SpatialiteDbInfo::SpatialMetadata dbCreateOption )
{
  QString sDbPath = dbPath;
  QFileInfo file_info( dbPath );
  // for canonicalFilePath, the file must exist
  if ( file_info.exists() )
  {
    // SpatialiteDbInfo uses the actual absolute path [with resolved soft-links]
    sDbPath = file_info.canonicalFilePath();
  }
  SpatialiteDbInfo::SpatialSniff sniffType = SpatialiteDbInfo::SniffUnknown;
  // if allready loaded, use it [shared or not]
  if ( sHandles.contains( sDbPath ) )
  {
    // The absolute file name is contained in 'sHandles'
    QgsSqliteHandle *static_handle = sHandles.value( sDbPath );
    if ( static_handle )
    {
      if ( static_handle->isValid() )
      {
        if ( static_handle->handle() )
        {
          if ( static_handle->ref <= 0 )
          {
            static_handle->ref = 1;
          }
          static_handle->ref++;
          QgsDebugMsgLevel( QString( "Cached sqlite3 connection ConnectionRef[%1] Uuid[%2] shared[%3] for sDbPath[%4] " ).arg( static_handle->ref ).arg( static_handle->getUuid() ).arg( shared ).arg( sDbPath ), 3 );
          return static_handle;
        }
      }
      else
      {
        // Has been shutdown, but not yet removed
        sHandles.remove( sDbPath );
      }
    }
  }
  //-----------------------------------------------------------------
  // [will be created if a valid create-option is given and the file does not exist]
  // If 'shared' == 1 and isDbValid():
  // - then it has been added to 'sHandles' as ConnectionRef[1] when not contained in 'sHandles', which (at this point) should never be
  // -> will contain a filled Uuid
  // If NOT 'shared' == 1 and/or NOT isDbValid(): ConnectionRef[-1]
  // -> will contain an empty Uuid
  //-----------------------------------------------------------------
  SpatialiteDbInfo *spatialiteDbInfo = SpatialiteDbInfo::FetchSpatialiteDbInfo( sDbPath, shared, sLayerName, bLoadLayers, dbCreateOption, sniffType );
  if ( spatialiteDbInfo )
  {
    if ( spatialiteDbInfo->getQSqliteHandle() )
    {
      // The file exists, is a Sqlite3-File and a connection has been made.
      if ( spatialiteDbInfo->isDbValid() )
      {
        QgsDebugMsgLevel( QString( "New sqlite3 connection ConnectionRef[%1] Uuid[%2] share[%3] for LayerName['%4',%5]  path[%6] " ).arg( spatialiteDbInfo->getConnectionRef() ).arg( spatialiteDbInfo->getConnectionUuid() ).arg( shared ).arg( sLayerName ).arg( bLoadLayers ).arg( sDbPath ), 3 );
        // The file Sqlite3-Container is supported by QgsSpatiaLiteProvider,QgsOgrProvider or QgsGdalProvider
        if ( !spatialiteDbInfo->isDbGdalOgr() )
        {
          // The file Sqlite3-Container is supported by QgsSpatiaLiteProvider.
        }
        if ( spatialiteDbInfo->isDbSpatialite() )
        {
          // The file Sqlite3-Container is supported by QgsOgrProvider or QgsGdalProvider.
        }
      }
      return spatialiteDbInfo->getQSqliteHandle();
    }
  }
  return nullptr;
}
//-----------------------------------------------------------------
// QgsSqliteHandle::setShared
//-----------------------------------------------------------------
bool QgsSqliteHandle::setShared( bool bIsShared )
{
  if ( ( bIsShared ) && ( !sHandles.contains( dbPath() ) ) )
  {
    if ( ref <= 0 )
    {
      ref = 1;
    }
    else
    {
      ref++;
    }
    if ( !isShared() )
    {
      ref = 1;
      sHandles.insert( dbPath(), this );
    }
    return isShared();
  }
  if ( ( !bIsShared ) && ( sHandles.contains( dbPath() ) ) )
  {
    ref--;
    if ( getRef() < 1 )
    {
      // Only when not used by others
      sHandles.remove( dbPath() );
      ref = -1;
    }
    return isShared();
  }
  return isShared();
}
//-----------------------------------------------------------------
// QgsSqliteHandle::closeDb [static]
//-----------------------------------------------------------------
void QgsSqliteHandle::closeDb( QgsSqliteHandle *&qSqliteHandle )
{
  if ( !qSqliteHandle )
    return;
  if ( qSqliteHandle->isShared() )
  {
    // The absolute file name is contained in 'sHandles'
    QgsSqliteHandle *static_handle = sHandles.value( qSqliteHandle->dbPath() );
    if ( ( static_handle ) && ( static_handle->ref <= 1 ) )
    {
      QString sDbPath = qSqliteHandle->dbPath();
      QgsDebugMsgLevel( QString( "Closing sqlite3 connection ConnectionRef[%1] Uuid[%2] shared[%3] path[%4] " ).arg( qSqliteHandle->getSharedSpatialiteConnectionsCount() ).arg( qSqliteHandle->getUuid() ).arg( static_handle->isShared() ).arg( qSqliteHandle->dbPath() ), 7 );
      qSqliteHandle->sqliteClose();
      delete static_handle;
      sHandles.remove( sDbPath );
    }
  }
  else
  {
    // not shared
    QgsDebugMsgLevel( QString( "Closing sqlite3 connection ConnectionRef[%1] Uuid[%2] not shared[%3] path[%4] " ).arg( qSqliteHandle->getSharedSpatialiteConnectionsCount() ).arg( qSqliteHandle->getUuid() ).arg( qSqliteHandle->isShared() ).arg( qSqliteHandle->dbPath() ), 7 );
    qSqliteHandle->sqliteClose();
    delete qSqliteHandle;
  }
  qSqliteHandle = nullptr;
}
//-----------------------------------------------------------------
// QgsSqliteHandle::closeAll [static]
//-----------------------------------------------------------------
void QgsSqliteHandle::closeAll()
{
  QMap < QString, QgsSqliteHandle * >::iterator i;
  for ( i = sHandles.begin(); i != sHandles.end(); ++i )
  {
    i.value()->sqliteClose();
    delete i.value();
  }
  sHandles.clear();
}
//-----------------------------------------------------------------
// QgsSqliteHandle::loadExtension [static]
//-----------------------------------------------------------------
// Since Spatialite 4.2.0
// - for RasterLite2, Apatialite must be loaded first
// sFileName only needed for debuging for what files this is being called for
// 'mod_spatialite' calls:
// -> 'sqlite3_modspatialite_init'   [calls 'init_spatialite_extension', 'spatialite_alloc_connection' and 'spatialite_init_ex']
// 'mod_rasterlite2' calls:
// -> 'sqlite3_modrasterlite_init' [calls 'init_rl2_extension' and 'register_rl2_sql_functions']
//-----------------------------------------------------------------
bool QgsSqliteHandle::loadExtension( sqlite3 *sqlite_handle,  bool bRasterLite2, QString sFileName )
{
  bool bRc = false;
  char *errMsg = nullptr;
  int i_rc = SQLITE_ERROR;
  QString sExtension = QStringLiteral( "mod_spatialite" );
  if ( bRasterLite2 )
  {
    sExtension = QStringLiteral( "mod_rasterlite2" );
  }
  else
  {
    // avoid 'not authorized' error
    sqlite3_enable_load_extension( sqlite_handle, 1 ); // allways returns SQLITE_OK
  }
  i_rc = sqlite3_load_extension( sqlite_handle, sExtension.toUtf8().constData(), nullptr, &errMsg );
  if ( i_rc == SQLITE_OK )
  {
    QgsDebugMsgLevel( QString( "Extension ['%1'] has been loaded. Filename[%2]" ).arg( sExtension ).arg( sFileName ), 7 );
    bRc = true;
  }
  else
  {
    QgsDebugMsgLevel( QString( "Error loading Extension ['%1'] Filename[%2] rc=[%3,%4] error[%5]" ).arg( sExtension ).arg( sFileName ).arg( i_rc ).arg( QgsSqliteHandle::get_sqlite3_result_code_string( i_rc ) ).arg( errMsg ), 7 );
    sqlite3_free( errMsg );
  }
  return bRc;
}
//-----------------------------------------------------------------
// QgsSqliteHandle::initSpatialite [static]
//-----------------------------------------------------------------
bool QgsSqliteHandle::initSpatialite( sqlite3 *sqlite_handle, QString sFileName )
{
  return loadExtension( sqlite_handle, false, sFileName ); // spatialite
}
//-----------------------------------------------------------------
// QgsSqliteHandle::loadSpatialite
//-----------------------------------------------------------------
bool QgsSqliteHandle::loadSpatialite( bool bRasterLite2 )
{
  if ( mSqliteHandle )
  {
    if ( bRasterLite2 )
    {
      return initRasterlite2();
    }
    if ( QgsSqliteHandle::initSpatialite( mSqliteHandle, getFileName() ) )
    {
      mIsSpatialiteActive = true;
    }
  }
  return mIsSpatialiteActive;
}
//-----------------------------------------------------------------
// QgsSqliteHandle::initRasterlite2
//-----------------------------------------------------------------
// Since Spatialite is a pre-condition for RasterLite2
// - Spatialite will be started first, if not allready started
// LoadExtension  will  be called for both
//-----------------------------------------------------------------
bool QgsSqliteHandle::initRasterlite2()
{
  if ( mSqliteHandle )
  {
    if ( !mIsSpatialiteActive )
    {
      // Spatialite must be active to start RasterLite2
      if ( QgsSqliteHandle::initSpatialite( mSqliteHandle, getFileName() ) )
      {
        mIsSpatialiteActive = true;
      }
    }
    if ( ( mIsSpatialiteActive ) && ( !mIsRasterLite2Active ) )
    {
      // register the rl2-functions as sql-commands using 'mod_rasterlite2'
      mIsRasterLite2Active = loadExtension( mSqliteHandle, true, getFileName() ); // rasterlite2
    }
  }
  return mIsRasterLite2Active;
}
//-----------------------------------------------------------------
// QgsSqliteHandle::sqliteClose
//-----------------------------------------------------------------
void QgsSqliteHandle::sqliteClose()
{
  invalidate(); // shutting down
  if ( mSqliteHandle )
  {
    QFileInfo file_info( getFileName() );
    if ( file_info.exists() )
    {
      // avoid SQLITE_MISUSE error
      QgsSqliteHandle::sqlite3_close( mSqliteHandle, getFileName() );
    }
    mSqliteHandle = nullptr;
    if ( mSpatialiteDbInfo )
    {
      delete mSpatialiteDbInfo;
      mSpatialiteDbInfo = nullptr;
    }
  }
}
//-----------------------------------------------------------------
// QgsSqliteHandle::sqlite3_open [static]
//-----------------------------------------------------------------
int QgsSqliteHandle::sqlite3_open( const char *filename, sqlite3 **psqlite_handle, bool bInitSpatialite )
{
  QString sFileName = QString( filename );
  QFileInfo file_info( sFileName );
  int i_rc = ::sqlite3_open( filename, psqlite_handle );
  if ( i_rc == SQLITE_OK )
  {
    // activating Foreign Key constraints [needed for proper internal spatialite support for admin tables (CASCADE)]
    ( void ) sqlite3_exec( *psqlite_handle, "PRAGMA foreign_keys = 1", nullptr, 0, nullptr );
    if ( bInitSpatialite )
    {
      if ( !QgsSqliteHandle::initSpatialite( *psqlite_handle, file_info.fileName() ) )
      {
        i_rc = SQLITE_ABORT;
      }
    }
  }
  if ( i_rc != SQLITE_OK )
  {
    *psqlite_handle = nullptr; // return a set pointer only when SQLITE_OK
    // rc=5, The database file is locked : [former crash, database file with journal file open]
    QgsDebugMsgLevel( QString( "sqlite3_open failed: rc=[%1,%2] FileName[%3]" ).arg( i_rc ).arg( QgsSqliteHandle::get_sqlite3_result_code_string( i_rc ) ).arg( file_info.fileName() ), 7 );
  }
  return i_rc;
}
//-----------------------------------------------------------------
// QgsSqliteHandle::sqlite3_open_v2 [static]
//-----------------------------------------------------------------
int QgsSqliteHandle::sqlite3_open_v2( const char *filename, sqlite3 **psqlite_handle, int flags, const char *zVfs, bool bInitSpatialite )
{
  QString sFileName = QString( filename );
  QFileInfo file_info( sFileName );
  int i_rc = ::sqlite3_open_v2( filename, psqlite_handle, flags, zVfs );
  if ( i_rc == SQLITE_OK )
  {
    // activating Foreign Key constraints [needed for proper internal spatialite support for admin tables (CASCADE)]
    ( void )sqlite3_exec( *psqlite_handle, "PRAGMA foreign_keys = 1", nullptr, 0, nullptr );
    if ( bInitSpatialite )
    {
      if ( !QgsSqliteHandle::initSpatialite( *psqlite_handle, file_info.fileName() ) )
      {
        i_rc = SQLITE_ABORT;
      }
    }
  }
  if ( i_rc != SQLITE_OK )
  {
    *psqlite_handle = nullptr; // return a set pointer only when SQLITE_OK
    // rc=5, The database file is locked : [former crash, database file with journal file open]
    QgsDebugMsgLevel( QString( "sqlite3_open_v2 failed: rc=[%1,%2] FileName[%3]" ).arg( i_rc ).arg( QgsSqliteHandle::get_sqlite3_result_code_string( i_rc ) ).arg( file_info.fileName() ), 7 );
  }
  return i_rc;
}
//-----------------------------------------------------------------
// QgsSqliteHandle::sqlite3_close [static]
//-----------------------------------------------------------------
int QgsSqliteHandle::sqlite3_close( sqlite3 *sqlite_handle, QString sFileName )
{
  int i_rc = SQLITE_OK;
  // avoid SQLITE_MISUSE error
  if ( sqlite_handle )
  {
    ::sqlite3_close( sqlite_handle );
    if ( i_rc != SQLITE_OK )
    {
      QgsDebugMsgLevel( QString( "sqlite3_close failed: rc=[%1,%2]  FileName[%3]" ).arg( i_rc ).arg( QgsSqliteHandle::get_sqlite3_result_code_string( i_rc ) ).arg( sFileName ), 7 );
    }
  }
  return i_rc;
}
//-----------------------------------------------------------------
// QgsSqliteHandle::sqlite3_close_v2 [static]
//-----------------------------------------------------------------
int QgsSqliteHandle::sqlite3_close_v2( sqlite3 *sqlite_handle, QString sFileName )
{
  int i_rc = SQLITE_OK;
  // avoid SQLITE_MISUSE error
  if ( sqlite_handle )
  {
    i_rc = ::sqlite3_close( sqlite_handle );
    if ( i_rc != SQLITE_OK )
    {
      QgsDebugMsgLevel( QString( "sqlite3_close_v2 failed: rc=[%1,%2]  FileName[%3]" ).arg( i_rc ).arg( QgsSqliteHandle::get_sqlite3_result_code_string( i_rc ) ).arg( sFileName ), 7 );
    }
  }
  return i_rc;
}
//-----------------------------------------------------------------
// QgsSqliteHandle::get_sqlite3_result_code_string [static]
//-----------------------------------------------------------------
QString QgsSqliteHandle::get_sqlite3_result_code_string( int i_rc )
{
  QString sResultCode = QString();
  switch ( i_rc )
  {
    case SQLITE_OK: // 0
      sResultCode = QStringLiteral( "Successful result" );
      break;
    case SQLITE_ERROR: // 1
      sResultCode = QStringLiteral( "Generic error" );
      break;
    case SQLITE_INTERNAL: // 2
      sResultCode = QStringLiteral( "Internal logic error in SQLite" );
      break;
    case SQLITE_PERM: // 3
      sResultCode = QStringLiteral( "Access permission denied" );
      break;
    case SQLITE_ABORT: // 4
      sResultCode = QStringLiteral( "Callback routine requested an abort" );
      break;
    case SQLITE_BUSY: // 5
      sResultCode = QStringLiteral( "The database file is locked" );
      break;
    case SQLITE_LOCKED: // 6
      sResultCode = QStringLiteral( "The database file is locked" );
      break;
    case SQLITE_NOMEM: // 7
      sResultCode = QStringLiteral( "A malloc() failed" );
      break;
    case SQLITE_READONLY: // 8
      sResultCode = QStringLiteral( "Attempt to write a readonly database" );
      break;
    case SQLITE_INTERRUPT: // 9
      sResultCode = QStringLiteral( "Operation terminated by sqlite3_interrupt()" );
      break;
    case SQLITE_IOERR: // 10
      sResultCode = QStringLiteral( "Some kind of disk I/O error occurred" );
      break;
    case SQLITE_CORRUPT: // 11
      sResultCode = QStringLiteral( "The database disk image is malformed" );
      break;
    case SQLITE_NOTFOUND: // 12
      sResultCode = QStringLiteral( "Unknown opcode in sqlite3_file_control() " );
      break;
    case SQLITE_FULL: // 13
      sResultCode = QStringLiteral( "Insertion failed because database is full" );
      break;
    case SQLITE_CANTOPEN: // 14
      sResultCode = QStringLiteral( "Unable to open the database file" );
      break;
    case SQLITE_PROTOCOL: // 15
      sResultCode = QStringLiteral( "Database lock protocol error" );
      break;
    case SQLITE_EMPTY: // 16
      sResultCode = QStringLiteral( "Internal use only" );
      break;
    case SQLITE_SCHEMA: // 17
      sResultCode = QStringLiteral( "The database schema changed" );
      break;
    case SQLITE_TOOBIG: // 18
      sResultCode = QStringLiteral( "String or BLOB exceeds size limit" );
      break;
    case SQLITE_CONSTRAINT: // 19
      sResultCode = QStringLiteral( "Abort due to constraint violation" );
      break;
    case SQLITE_MISMATCH: // 20
      sResultCode = QStringLiteral( "Data type mismatch" );
      break;
    case SQLITE_MISUSE: // 21
      sResultCode = QStringLiteral( "Library used incorrectly" );
      break;
    case SQLITE_NOLFS: // 22
      sResultCode = QStringLiteral( "Uses OS features not supported on host" );
      break;
    case SQLITE_AUTH: // 23
      sResultCode = QStringLiteral( "Authorization denied" );
      break;
    case SQLITE_FORMAT: // 24
      sResultCode = QStringLiteral( "Not used" );
      break;
    case SQLITE_RANGE: // 25
      sResultCode = QStringLiteral( "2nd parameter to sqlite3_bind out of range" );
      break;
    case SQLITE_NOTADB: // 26
      sResultCode = QStringLiteral( "File opened that is not a database file" );
      break;
    case SQLITE_NOTICE : // 27
      sResultCode = QStringLiteral( "Notifications from sqlite3_log()" );
      break;
    case SQLITE_WARNING: // 28
      sResultCode = QStringLiteral( "Warnings from sqlite3_log()" );
      break;
    case SQLITE_ROW: // 100
      sResultCode = QStringLiteral( "sqlite3_step() has another row ready" );
      break;
    case SQLITE_DONE: // 101
      sResultCode = QStringLiteral( "sqlite3_step() has finished executing" );
      break;
    default:
      sResultCode = QStringLiteral( "Unknown return code[%1]" ).arg( i_rc );
      break;
  }
  return sResultCode;
}
